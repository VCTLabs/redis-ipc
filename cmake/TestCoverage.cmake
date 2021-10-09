set(CMAKE_C_FLAGS "-g -O0 --coverage")
set(CMAKE_CXX_FLAGS "-g -O0 --coverage")
set(CMAKE_EXE_LINKER_FLAGS "--coverage")
set(CMAKE_SHARED_LINKER_FLAGS "--coverage")

set(COVERAGE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/coverage")
set(TRACEFILE "${CMAKE_SOURCE_DIR}/coverage.info")
set(REPORT_DIR "${COVERAGE_OUTPUT_DIR}")

# cmake-format: off
add_custom_command(
  OUTPUT "${TRACEFILE}" always
  WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
  COMMAND ${CMAKE_COMMAND} -E remove "${TRACEFILE}"
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${COVERAGE_OUTPUT_DIR}"
  COMMAND ${CMAKE_CTEST_COMMAND}  # execute default test suite

  COMMAND lcov
    --config-file "${CMAKE_SOURCE_DIR}/.lcovrc"
    --capture
    --directory "${CMAKE_BINARY_DIR}"
    --include "${CMAKE_SOURCE_DIR}/src/redis_ipc.c"
    --include "${CMAKE_SOURCE_DIR}/src/redis_ipc.h"
    --include "${CMAKE_SOURCE_DIR}/inc/json.hh"
    --output-file "${TRACEFILE}"

  COMMAND genhtml ${TRACEFILE}
    --prefix "."
    --title "${CMAKE_PROJECT_NAME}"
    --legend --show-details
    --output-directory ${REPORT_DIR}

  VERBATIM  # for correct handling of wildcards in command line parameters
)
# cmake-format: on

add_custom_target(cov DEPENDS ${TRACEFILE} always)
