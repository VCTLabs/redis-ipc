find_package(PythonInterp)
find_package(PythonLibs)

if(PYTHONINTERP_FOUND)
  execute_process(
    COMMAND ${PYTHON_EXECUTABLE} --version
    ERROR_VARIABLE PYTHON_VERSION_FULL
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  string(REGEX MATCH "[0-9]+.[0-9]+" PYTHON_VERSION_MAJOR_MINOR
               "${PYTHON_VERSION_FULL}")
  if(UNIX)
    set(PYTHON_PACKAGES_PATH
        lib/python${PYTHON_VERSION_MAJOR_MINOR}/site-packages
        CACHE PATH "Where to install the python packages.")
  endif()
  if(WIN32)
    get_filename_component(
      PYTHON_PATH
      "[HKEY_LOCAL_MACHINE\\SOFTWARE\\Python\\PythonCore\\${PYTHON_VERSION_MAJOR_MINOR}\\InstallPath]"
      ABSOLUTE
      CACHE)
    set(PYTHON_PACKAGES_PATH "${PYTHON_PATH}/Lib/site-packages")
  endif()

  if("${PYTHON_VERSION_MAJOR_MINOR}" VERSION_GREATER 2.5)
    set(PYTHON_EXECUTABLE ${PYTHON_EXECUTABLE} -B)
  endif()
endif(PYTHONINTERP_FOUND)
