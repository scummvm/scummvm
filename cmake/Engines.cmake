# This file contains engine related functions and targets

option(ENABLE_ALL_ENGINES "Enable all engines" OFF)
option(DISABLE_ALL_ENGINES "Disable all engines" OFF)
if(ENABLE_ALL_ENGINES AND DISABLE_ALL_ENGINES)
  message(
    WARNING
      "ENABLE_ALL_ENGINES and DISABLE_ALL_ENGINES should not be set at the same time. ENABLE_ALL_ENGINES will take precedence."
  )
  set(DISABLE_ALL_ENGINES OFF CACHE BOOL "Disable all engines" FORCE)
endif()

option(ENABLE_ALL_UNSTABLE_ENGINES "Enable all unstable engines" OFF)
option(DISABLE_ALL_UNSTABLE_ENGINES "Disable all unstable engines" OFF)
if(ENABLE_ALL_UNSTABLE_ENGINES AND DISABLE_ALL_UNSTABLE_ENGINES)
  message(
    WARNING
      "ENABLE_ALL_UNSTABLE_ENGINES and DISABLE_ALL_UNSTABLE_ENGINES should not be set at the same time. ENABLE_ALL_UNSTABLE_ENGINES will take precedence."
  )
  set(DISABLE_ALL_UNSTABLE_ENGINES OFF CACHE BOOL "Disable all unstable engines" FORCE)
endif()

set(ENABLED_ENGINES "" CACHE STRING "Enabled engines")
set(DISABLED_ENGINES "" CACHE STRING "Disabled engines")

# Detection related options
option(DETECTION_FULL "Build detection features for all engines" ON)
option(DETECTION_STATIC "Build detection features into executable" ON)
add_compile_definitions(DETECTION_STATIC)

# Internal variable to store all engine names
set(_engines "")

#[[
# parse_engine_file(<file_path>)
#
# Description:
#   Parses sh-like add_engine function calls. Most of this logic will be unnecessary if/when we add
#   parentheses to the function calls - which would make them valid CMake functions that can just be called via include()
#
# Arguments:
#   <file_path>  The path to the configure.engine file to be parsed
#]]
macro(read_engine_configuration file_path)
  file(STRINGS "${file_path}" ENGINE_CONFIG REGEX "^add_engine")
  foreach(LINE IN ITEMS ${ENGINE_CONFIG})
    separate_arguments(ENGINE_ARGS UNIX_COMMAND "${LINE}")

    # Pad the list to 8 elements to ensure list(GET) doesn't error
    list(LENGTH ENGINE_ARGS len)
    while(len LESS 8)
      list(APPEND ENGINE_ARGS "")
      list(LENGTH ENGINE_ARGS len)
    endwhile()

    list(GET ENGINE_ARGS 1 name)
    list(APPEND _engines ${name})
    set("_engine_${name}_name" ${name})
    list(GET ENGINE_ARGS 2 desc)
    set("_engine_${name}_desc" ${desc})
    list(GET ENGINE_ARGS 3 build_default)
    if(build_default STREQUAL "yes")
      set("_engine_${name}_build_default" ON)
      set("_engine_${name}_build" ON)
    else()
      set("_engine_${name}_build_default" OFF)
      set("_engine_${name}_build" OFF)
    endif()

    list(GET ENGINE_ARGS 4 subengines)
    separate_arguments(subengines UNIX_COMMAND ${subengines})
    set("_engine_${name}_subengines" ${subengines})
    list(GET ENGINE_ARGS 5 base)
    set("_engine_${name}_base" ${base})
    list(GET ENGINE_ARGS 6 deps)
    separate_arguments(deps UNIX_COMMAND ${deps})
    set("_engine_${name}_deps" ${deps})
    list(GET ENGINE_ARGS 7 components)
    separate_arguments(components UNIX_COMMAND ${components})
    set("_engine_${name}_components" ${components})
    cmake_path(SET normalized_path NORMALIZE "${file_path}")
    cmake_path(GET normalized_path PARENT_PATH result)
    set("_engine_${name}_path" "${result}")
    cmake_path(GET result FILENAME folder_name)
    set("_engine_${name}_folder" "${folder_name}")
    unset(components)
    unset(deps)
    unset(normalized_path)
    unset(result)
  endforeach()
endmacro()

#[[
# read_engines(<engine_name> <link_engine>)
#
# Description:
#   Reads all matching engine names and add all of the necessary dependencies to our build.
#   Optionally add them to the enabled_engines target
#
# Arguments:
#   <engine_name>  Name of the engines to read - takes a glob like format
#]]
macro(read_all_engines)
  # set(LINK_MODE "${link_engine}" PARENT_SCOPE)
  set(ENGINES_DIR "${PROJECT_SOURCE_DIR}/engines")
  file(GLOB ENGINES RELATIVE "${ENGINES_DIR}" "${ENGINES_DIR}/*/configure.engine")
  foreach(engine ${ENGINES})
    read_engine_configuration("${ENGINES_DIR}/${engine}")
  endforeach()
endmacro()

# Let's start by reading all engines
read_all_engines()

# Process all engine enable/disable flags
if(DISABLE_ALL_ENGINES)
  foreach(engine IN LISTS _engines)
    set("_engine_${engine}_build" OFF)
  endforeach()
endif()

if(ENABLE_ALL_ENGINES)
  foreach(engine IN LISTS _engines)
    set("_engine_${engine}_build" ON)
  endforeach()
endif()

if(ENABLE_ALL_UNSTABLE_ENGINES)
  foreach(engine IN LISTS _engines)
    if(NOT "${_engine_${engine}_build_default}")
      set("_engine_${engine}_build" ON)
    endif()
  endforeach()
endif()

if(DISABLE_ALL_UNSTABLE_ENGINES)
  foreach(engine IN LISTS _engines)
    if(NOT "${_engine_${engine}_build_default}")
      set("_engine_${engine}_build" OFF)
    endif()
  endforeach()
endif()

# Only after handling the enable/disable flags we should process user entered values
foreach(engine IN LISTS DISABLED_ENGINES)
  set("_engine_${engine}_build" OFF)
endforeach()

foreach(engine IN LISTS ENABLED_ENGINES)
  set("_engine_${engine}_build" ON)
endforeach()

# Let's print the engines we are building
set(_enabled_engines "")
set(_disabled_engines "")
foreach(engine IN LISTS _engines)
  if("${_engine_${engine}_build}")
    list(APPEND _enabled_engines ${engine})
    string(TOUPPER "${engine}" engine_upper)
    add_compile_definitions("ENABLE_${engine_upper}")
    option("ENABLE_${engine_upper}" "Enable ${engine}" ON)
    enable_components("${_engine_${engine}_components}")
  else()
    list(APPEND _disabled_engines ${engine})
  endif()
endforeach()

set(ENABLED_ENGINES_FINAL ${_enabled_engines} CACHE INTERNAL "Engines enabled for this build")
set(DISABLED_ENGINES_FINAL ${_disabled_engines} CACHE INTERNAL "Engines disabled for this build")

string(REPLACE ";" "\n\t" _enabled_engines_output "${_enabled_engines}")
string(REPLACE ";" "\n\t" _disabled_engines_output "${_disabled_engines}")
message(STATUS "Enabled engines: \n\t${_enabled_engines_output}")
message(STATUS "Disabled engines: \n\t${_disabled_engines_output}")
