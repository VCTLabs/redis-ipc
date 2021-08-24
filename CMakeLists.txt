cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

project(redis_ipc C CXX)
include(CTest)
include(GNUInstallDirs)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

option(BUILD_SHARED_LIBS "build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)

option(BUILD_TESTING "buiuld and run tests" ON)

#set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Install directory for pkgconfig (.pc) files")
set(EXTRA_TARGET_LINK_LIBRARIES)

find_package(json-c)
if (json-c_FOUND)
  list(APPEND THIRD_PARTY_LIBS json-c)
endif()

find_package(PkgConfig)
pkg_check_modules(HIREDIS IMPORTED_TARGET hiredis)
if (HIREDIS_FOUND)
  list(APPEND THIRD_PARTY_LIBS PkgConfig::HIREDIS)
else()
  find_package(hiredis)
  list(APPEND THIRD_PARTY_LIBS hiredis)
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
  add_definitions(-DUNICODE -D_UNICODE -DSTRICT -DNOMINMAX)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_SCL_SECURE_NO_WARNINGS)
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

# SCM_VERSION_INFO is defined by cmake args and passed into the code as a
# define (VERSION_INFO) here.
target_compile_definitions(redis_ipc PRIVATE VERSION_INFO=${SCM_VERSION_INFO})

# this looks weird, but needed for correct SOVERSION links
set_target_properties(redis_ipc PROPERTIES VERSION 0.0.3)
set_target_properties(redis_ipc PROPERTIES SOVERSION 0)

if(BUILD_TESTING)
  set(TEST_TARGETS
      command_result_test
      pub_sub_test
      settings_status_test
      multithread_test
      )

  add_executable(json_test test/json_test.cpp)
  target_link_libraries(json_test redis_ipc ${EXTRA_TARGET_LINK_LIBRARIES})
  add_test(NAME json_test COMMAND json_test)

  foreach(target ${TEST_TARGETS})
    add_executable(${target} test/${target}.c)
    target_link_libraries(${target} redis_ipc ${EXTRA_TARGET_LINK_LIBRARIES})
    add_test(NAME ${target} COMMAND ${target})
  endforeach(target)
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

#set(RIPC_PC ${CMAKE_CURRENT_BINARY_DIR}/redis_ipc.pc)
#configure_file( ${CMAKE_CURRENT_SOURCE_DIR}/redis_ipc.pc.in ${RIPC_PC} @ONLY)
#install(FILES ${RIPC_PC} DESTINATION "${INSTALL_PKGCONFIG_DIR}")

