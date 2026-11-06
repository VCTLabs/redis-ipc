cmake_minimum_required(VERSION 3.18)

include(ProcessorCount)
ProcessorCount(N)
if(NOT N EQUAL 0)
    set(CTEST_BUILD_FLAGS -j${N})
endif()

set(ENV{LANG} "C") # Only ascii output

# CTest Start Step
set(CTEST_SOURCE_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}")
set(CTEST_BINARY_DIRECTORY "${CTEST_SCRIPT_DIRECTORY}/build")

# CTest Update Step
set(CTEST_UPDATE_COMMAND "/usr/bin/git")
set(CTEST_UPDATE_OPTIONS "")
set(CTEST_UPDATE_VERSION_ONLY "")
set(CTEST_NIGHTLY_START_TIME "00:00:00")

# Git options
set(CTEST_GIT_COMMAND "/usr/bin/git")
set(CTEST_GIT_INIT_SUBMODULES "")
set(CTEST_GIT_UPDATE_CUSTOM "")
set(CTEST_GIT_UPDATE_OPTIONS "")

# CTest Configure Step
set(CTEST_CMAKE_GENERATOR "Unix Makefiles")
set(CTEST_LABELS_FOR_SUBPROJECTS "")

# CTest Build Step
set(CTEST_CONFIGURATION_TYPE "RelWithDebInfo")
set(CTEST_USE_LAUNCHERS TRUE)

# CTest Test Step
set(CTEST_RESOURCE_SPEC_FILE "")
set(CTEST_TEST_LOAD "")
set(CTEST_TEST_TIMEOUT "1500")
set(CTEST_TEST_COVERAGE_TOOL "")

# CTest Coverage Step
set(CTEST_COVERAGE_COMMAND "/usr/bin/gcov")
set(CTEST_COVERAGE_EXTRA_FLAGS "-l")

# CTest MemCheck Step
set(CTEST_MEMORYCHECK_COMMAND "MEMORYCHECK_COMMAND-NOTFOUND")
set(CTEST_MEMORYCHECK_COMMAND_OPTIONS "")
set(CTEST_MEMORYCHECK_TYPE "")
set(CTEST_MEMORYCHECK_SANITIZER_OPTIONS "")
set(CTEST_MEMORYCHECK_SUPPRESSIONS_FILE "")

# CTest Submit Step
set(CTEST_SITE "kepler")
set(CTEST_BUILD_NAME "Linux-g++")

ctest_empty_binary_directory(${CTEST_BINARY_DIRECTORY})

# ##############################################################################

if(NOT MODEL)
    set(MODEL "Experimental")
endif()

if(MODEL STREQUAL "NightlyMemoryCheck")
    set(MODEL "Nightly")
    set(ACTIONS "Start;Update;Configure;Build;MemCheck;Coverage;Submit")
endif()

if(NOT ACTIONS)
    if(MODEL STREQUAL "Experimental")
        set(ACTIONS "Start;Configure;Build;Test;Coverage;Submit")
    else()
        set(ACTIONS "Start;Update;Configure;Build;Test;Coverage;Submit")
    endif()
endif()

# ##############################################################################

set(_exit_code 0)

if("Start" IN_LIST ACTIONS OR NOT EXISTS
                              "${CTEST_BINARY_DIRECTORY}/Testing/TAG"
)
    ctest_start("${MODEL}")
else()
    ctest_start("${MODEL}" APPEND)
endif()

if("Update" IN_LIST ACTIONS)
    ctest_update(RETURN_VALUE update_count)
    if(update_count LESS 0)
        math(EXPR _exit_code "${_exit_code} | 0x01")
    endif()
    if(MODEL STREQUAL "Continuous" AND update_count EQUAL 0)
        return()
    endif()
endif()

if("Configure" IN_LIST ACTIONS)
    ctest_configure(
        SOURCE "${CTEST_SOURCE_DIRECTORY}"
        BUILD "${CTEST_BINARY_DIRECTORY}"
        OPTIONS "-DWITH_COVERAGE=ON"
        RETURN_VALUE success
    )
    if(success LESS 0)
        math(EXPR _exit_code "${_exit_code} | 0x02")
    endif()
endif()

if("Build" IN_LIST ACTIONS)
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_build(RETURN_VALUE success)
    if(NOT success EQUAL 0)
        math(EXPR _exit_code "${_exit_code} | 0x04")
    endif()
endif()

if("Test" IN_LIST ACTIONS)
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_test(RETURN_VALUE success)
    if(NOT success EQUAL 0)
        math(EXPR _exit_code "${_exit_code} | 0x08")
    endif()
endif()

if("Coverage" IN_LIST ACTIONS)
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_coverage(RETURN_VALUE success)
    if(NOT success EQUAL 0)
        math(EXPR _exit_code "${_exit_code} | 0x20")
    endif()
endif()

if("MemCheck" IN_LIST ACTIONS)
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_memcheck(RETURN_VALUE success)
    if(NOT success EQUAL 0)
        math(EXPR _exit_code "${_exit_code} | 0x10")
    endif()
endif()

file(
    GLOB notes_files
    LIST_DIRECTORIES OFF
    "${CTEST_BINARY_DIRECTORY}/Testing/Notes/*"
)
if(notes_files)
    list(APPEND CTEST_NOTES_FILES "${notes_files}")
endif()

if("Submit" IN_LIST ACTIONS)
    ctest_read_custom_files("${CTEST_BINARY_DIRECTORY}")
    ctest_submit(
        RETRY_COUNT "3"
        RETRY_DELAY "5"
        RETURN_VALUE success
    )
    if(NOT success EQUAL 0)
        math(EXPR _exit_code "${_exit_code} | 0x40")
    endif()
endif()

cmake_language(EXIT "${_exit_code}")
