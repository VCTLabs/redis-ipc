cmake_minimum_required(VERSION 3.10...3.15)

# Add cmake modules of this project to the module path
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# used for both library and pkgconfig file
set(PACKAGE_VERSION 0.2.1)
set(LIBRARY_SOVERSION 0)

if(POLICY CMP0048)
    cmake_policy(SET CMP0048 NEW)
endif()

if(SCM_VERSION_INFO)
    set(PACKAGE_VERSION ${SCM_VERSION_INFO})
endif()

set(RIPC_SONAME ${PACKAGE_VERSION})

project(
    redis-ipc
    VERSION ${PACKAGE_VERSION}
    LANGUAGES C CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_VERBOSE_MAKEFILE ON)

include(CTest)
include(GNUInstallDirs)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(CMAKE_C_STANDARD 99)
set(CMAKE_DEBUG_POSTFIX d)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE
        "RelWithDebInfo"
        CACHE STRING "Default build type: RelWithDebInfo" FORCE
    )
endif()

option(BUILD_SHARED_LIBS "build shared libraries" ON)
option(BUILD_STATIC_LIBS "Build static libraries" OFF)

option(RIPC_BUILD_TESTING "build and run tests" OFF)
option(RIPC_DISABLE_SOCK_TESTS "disable tests requiring redis socket" OFF)

set(RIPC_RUNTIME_DIR
    ""
    CACHE PATH "path to directory containing redis server socket"
)

set(WITH_COVERAGE
    ""
    CACHE PATH "build with test coverage enabled"
)

set(INSTALL_PKGCONFIG_DIR
    "${CMAKE_INSTALL_PREFIX}/share/pkgconfig"
    CACHE PATH "Install directory for pkgconfig (.pc) files"
)
set(EXTRA_TARGET_LINK_LIBRARIES)

# accept cmake option -or- environment variable
if(DEFINED ENV{RIPC_RUNTIME_DIR})
    message(STATUS "Found socket path in ENV: $ENV{RIPC_RUNTIME_DIR}")
    set(RUNTIME_DIR "$ENV{RIPC_RUNTIME_DIR}")
    add_compile_definitions(RIPC_RUNTIME_DIR="${RUNTIME_DIR}")
elseif(RIPC_RUNTIME_DIR)
    message(STATUS "Got cmake option RIPC_RUNTIME_DIR: ${RIPC_RUNTIME_DIR}")
    set(RUNTIME_DIR "${RIPC_RUNTIME_DIR}")
    add_compile_definitions(RIPC_RUNTIME_DIR="${RUNTIME_DIR}")
endif()

# ~~~
# Adds custom target for lcov report generation (does not build, only
# runs test cmd). Uses the same .lcovrc and command args as autotools.
# Note: "make cov" target must be run both with/without RIPC_SERVER_PATH
#        override in order to generate full coverage data.
# ~~~
if(WITH_COVERAGE)
    set(RIPC_BUILD_TESTING ON)
    include(TestCoverage)
endif()

find_package(PkgConfig)

if(NOT PkgConfig_FOUND)
    find_package(json-c)
    find_package(hiredis)
    list(APPEND THIRD_PARTY_LIBS json-c hiredis)
else()
    pkg_check_modules(HIREDIS IMPORTED_TARGET hiredis)
    if(HIREDIS_FOUND)
        list(APPEND THIRD_PARTY_LIBS PkgConfig::HIREDIS)
    endif()
    pkg_check_modules(JSONC IMPORTED_TARGET json-c)
    if(JSONC_FOUND)
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
    # CMake defaults to /W3, but some users like /W4 (or /Wall) and /WX, so we
    # disable various warnings that aren't particularly helpful.
    add_compile_options(/wd4100 /wd4201 /wd4456 /wd4457 /wd4702 /wd4815)
    # Without a byte order mark (BOM), Visual Studio assumes that the source
    # file is encoded using the current user code page, so we specify UTF-8.
    add_compile_options(/utf-8)
endif()

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS _SCL_SECURE_NO_WARNINGS)
    add_compile_definitions(
        WIN32_LEAN_AND_MEAN UNICODE _UNICODE STRICT NOMINMAX
    )
elseif(UNIX)
    add_compile_options(-pthread)
    list(APPEND EXTRA_TARGET_LINK_LIBRARIES -pthread)
endif()

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/inc ${CMAKE_CURRENT_SOURCE_DIR}/src
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
# define here (see PACKAGE_VERSION near the top of this file)
target_compile_definitions(redis_ipc PRIVATE VERSION_INFO=${PACKAGE_VERSION})

# this looks weird, but needed for correct SOVERSION links
set_target_properties(
    redis_ipc PROPERTIES VERSION ${RIPC_SONAME} SOVERSION ${LIBRARY_SOVERSION}
)

if(RIPC_BUILD_TESTING)
    enable_testing()
    if(NOT RIPC_DISABLE_SOCK_TESTS)
        set(TEST_TARGETS command_result_test pub_sub_test settings_status_test
                         multithread_test
        )

        foreach(target ${TEST_TARGETS})
            add_executable(${target} test/${target}.c)
            target_link_libraries(
                ${target} redis_ipc ${EXTRA_TARGET_LINK_LIBRARIES}
                ${CMAKE_THREAD_LIBS_INIT}
            )
            add_test(NAME ${target} COMMAND ${target})
        endforeach(target)
    endif()

    add_executable(json_test test/json_test.cpp)
    target_link_libraries(
        json_test redis_ipc ${EXTRA_TARGET_LINK_LIBRARIES}
        ${CMAKE_THREAD_LIBS_INIT}
    )
    add_test(NAME json_test COMMAND json_test)

    # Add source-based code coverage targets instead of using WITH_COVERAGE
    # above. Note this requires recent clang/llvm tooling.
    if(COVERAGE_BUILD AND NOT WITH_COVERAGE)
        include(coverage)
        add_coverage(redis_ipc)
        add_coverage(json_test)
    endif()
endif()

set(RIPC_HEADERS inc/json.hh src/redis_ipc.h)

install(FILES ${RIPC_HEADERS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_SHARED_LIBS)
    list(APPEND RIPC_LIBS redis_ipc)
endif()

if(BUILD_STATIC_LIBS)
    list(APPEND RIPC_LIBS ripcstatic)
endif()

install(
    TARGETS ${RIPC_LIBS}
    EXPORT redis_ipcConfig
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    EXPORT redis_ipcConfig
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/redis_ipc
    NAMESPACE redis_ipc::
)

set(RIPC_PC ${CMAKE_CURRENT_BINARY_DIR}/redis-ipc.pc)
set(prefix ${CMAKE_INSTALL_PREFIX})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/redis-ipc.pc.in ${RIPC_PC} @ONLY)
install(FILES ${RIPC_PC} DESTINATION ${INSTALL_PKGCONFIG_DIR})
