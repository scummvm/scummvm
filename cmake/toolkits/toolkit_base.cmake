# Base helper macros for ScummVM CMake toolkit files

macro(scummvm_toolkit_define flag)
  add_compile_definitions(${flag})
endmacro()

macro(scummvm_toolkit_add_cxx_flags flags)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flags}" PARENT_SCOPE)
endmacro()

macro(scummvm_toolkit_add_ld_flags flags)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${flags}" PARENT_SCOPE)
endmacro()

macro(scummvm_toolkit_add_libs libs)
  link_libraries(${libs})
endmacro()

macro(scummvm_toolkit_set_option option_name value)
  set(${option_name} ${value} CACHE BOOL "Force ${option_name} to ${value}" FORCE)
  if(${value})
    add_compile_definitions(${option_name})
  endif()
endmacro()
