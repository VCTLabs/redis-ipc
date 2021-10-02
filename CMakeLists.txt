cmake_minimum_required(VERSION 3.15...3.18)

# Add cmake modules of this project to the module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# used for both library and pkgconfig file
set(LIBRARY_VERSION 0.0.3)
set(LIBRARY_SOVERSION 0)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

if(SCM_VERSION_INFO)
  set(LIBRARY_VERSION ${SCM_VERSION_INFO})
endif()

set(RIPC_SONAME ${LIBRARY_VERSION})

project(redis_ipc VERSION ${LIBRARY_VERSION} LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include(CTest)
include(GNUInstallDirs)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

SET(CMAKE_C_STANDARD 99)
SET(CMAKE_DEBUG_POSTFIX d)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
            "Default build type: RelWithDebInfo" FORCE)
endif()

# Ensure proper configuration if in a conda environment
#include(CondaAware)
# find the active python site-packages path
#include(FindPython)

option(BUILD_SHARED_LIBS "build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)

option(RIPC_BUILD_TESTING "build and run tests" ON)
option(RIPC_DISABLE_SOCK_TESTS "disable tests requiring redis socket" OFF)

option(RIPC_RUNTIME_DIR "Default path to directory containing redis server socket")

set(WITH_COVERAGE "" CACHE PATH "build with test coverage enabled")

set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Install directory for pkgconfig (.pc) files")
set(EXTRA_TARGET_LINK_LIBRARIES)

if(DEFINED RIPC_RUNTIME_DIR)
    add_compile_options(-DRIPC_RUNTIME_DIR=\"${RIPC_RUNTIME_DIR}\")
endif()

if(WITH_COVERAGE)
    add_compile_options(--coverage)
    add_link_options(--coverage)
endif()

#include(FindJSONC)
#include_directories(${JSONC_INCLUDE_DIRS})
#list(APPEND THIRD_PARTY_LIBS ${JSONC_LIBRARIES})

find_package(PkgConfig)

if(NOT PkgConfig_FOUND)
  find_package(json-c)
  find_package(hiredis)
  list(APPEND THIRD_PARTY_LIBS json-c hiredis)
else()
  pkg_check_modules(HIREDIS IMPORTED_TARGET hiredis)
  if (HIREDIS_FOUND)
    list(APPEND THIRD_PARTY_LIBS PkgConfig::HIREDIS)
  endif()
  pkg_check_modules(JSONC IMPORTED_TARGET json-c)
  if (JSONC_FOUND)
    list(APPEND THIRD_PARTY_LIBS PkgConfig::JSONC)
  endif()
endif()

list(APPEND EXTRA_TARGET_LINK_LIBRARIES ${THIRD_PARTY_LIBS})

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
  if(MSVC_VERSION LESS 1900)
    message(FATAL_ERROR "you need Visual Studio 2015 or later")
  endif()
  if(BUILD_SHARED_LIBS)
    # See http://www.kitware.com/blog/home/post/939 for details.
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
  endif()
  # CMake defaults to /W3, but some users like /W4 (or /Wall) and /WX,
  # so we disable various warnings that aren't particularly helpful.
  add_compile_options(/wd4100 /wd4201 /wd4456 /wd4457 /wd4702 /wd4815)
  # Without a byte order mark (BOM), Visual Studio assumes that the source
  # file is encoded using the current user code page, so we specify UTF-8.
  add_compile_options(/utf-8)
endif()

if(WIN32)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
  add_compile_definitions(WIN32_LEAN_AND_MEAN UNICODE _UNICODE STRICT NOMINMAX)
elseif(UNIX)
  add_compile_options(-pthread)
  list(APPEND EXTRA_TARGET_LINK_LIBRARIES -pthread)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc
                    ${CMAKE_CURRENT_SOURCE_DIR}/src
                    )

set(RIPC_SOURCES src/redis_ipc.c)

if(BUILD_STATIC_LIBS)
  add_library(ripcstatic STATIC ${RIPC_SOURCES})
  set_target_properties(ripcstatic PROPERTIES OUTPUT_NAME redis_ipc)
endif()

add_library(redis_ipc ${RIPC_SOURCES})
add_library(redis_ipc::redis_ipc ALIAS redis_ipc)
target_link_libraries(redis_ipc ${EXTRA_TARGET_LINK_LIBRARIES})

# SCM_VERSION_INFO can be defined by cmake args and passed into the code as a
# define here (see LIBRARY_VERSION near the top of this file)
target_compile_definitions(redis_ipc PRIVATE VERSION_INFO=${LIBRARY_VERSION})

# this looks weird, but needed for correct SOVERSION links
set_target_properties(redis_ipc
    PROPERTIES
    VERSION ${RIPC_SONAME}
    SOVERSION ${LIBRARY_SOVERSION}
    )

if(RIPC_BUILD_TESTING)
  if (NOT RIPC_DISABLE_SOCK_TESTS)
    set(TEST_TARGETS
        command_result_test
        pub_sub_test
        settings_status_test
        multithread_test
        )

    foreach(target ${TEST_TARGETS})
      add_executable(${target} test/${target}.c)
      target_link_libraries(${target}
        redis_ipc
        ${EXTRA_TARGET_LINK_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
        )
      add_test(NAME ${target} COMMAND ${target})
    endforeach(target)
  endif()

  add_executable(json_test test/json_test.cpp)
  target_link_libraries(json_test
    redis_ipc
    ${EXTRA_TARGET_LINK_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    )
  add_test(NAME json_test COMMAND json_test)
endif()

set(RIPC_HEADERS
    inc/json.hh
    src/redis_ipc.h
    )

install(FILES ${RIPC_HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_SHARED_LIBS)
  list(APPEND RIPC_LIBS redis_ipc)
endif()

if(BUILD_STATIC_LIBS)
  list(APPEND RIPC_LIBS ripcstatic)
endif()

install(TARGETS ${RIPC_LIBS} EXPORT redis_ipcConfig
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(EXPORT redis_ipcConfig
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/redis_ipc NAMESPACE redis_ipc::)


set(RIPC_PC ${CMAKE_CURRENT_BINARY_DIR}/redis-ipc.pc)
configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/redis-ipc.pc.in ${RIPC_PC} @ONLY)
install(FILES ${RIPC_PC} DESTINATION ${INSTALL_PKGCONFIG_DIR})
