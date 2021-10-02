# ~~~
# This module finds headers and libjson-c library.
# Results are reported in variables:
#  JSONC_FOUND           - True if headers and library were found
#  JSONC_INCLUDE_DIRS    - libjson-c include directories
#  JSONC_LIBRARIES       - libjson-c library to be linked
# ~~~

find_path(
  JSONC_INCLUDE_DIR
  NAMES json-c/json.h
  HINTS ENV VCPKG_ROOT ENV CONDA_PREFIX
  PATH_SUFFIXES include include/json-c
  PATHS ~/Library/Frameworks /Library/Frameworks /opt/local /opt /usr
        /usr/local/)

find_library(
  JSONC_LIBRARY
  NAMES json-c libjson-c
  HINTS ENV VCPKG_ROOT ENV CONDA_PREFIX
  PATH_SUFFIXES lib lib64 lib32
  PATHS ~/Library/Frameworks /Library/Frameworks /opt/local /opt /usr
        /usr/local/)

mark_as_advanced(JSONC_INCLUDE_DIR JSONC_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(JSONC REQUIRED_VARS JSONC_LIBRARY
                                  JSONC_INCLUDE_DIR)

if(JSONC_FOUND)
  # need if _FOUND guard to allow project to autobuild; can't overwrite imported
  # target even if bad
  set(JSONC_INCLUDE_DIRS ${JSONC_INCLUDE_DIR})
  set(JSONC_LIBRARIES ${JSONC_LIBRARY})

  if(NOT TARGET json-c::json-c)
    add_library(json-c::json-c INTERFACE IMPORTED)
    set_target_properties(
      json-c::json-c
      PROPERTIES INTERFACE_LINK_LIBRARIES "${JSONC_LIBRARIES}"
                 INTERFACE_INCLUDE_DIRECTORIES "${JSONC_INCLUDE_DIR}")
  endif()
endif(JSONC_FOUND)
