# Samsung TV toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".so" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".so")

set(HOST_OS "linux" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(SAMSUNGTV DISABLE_COMMAND_LINE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_VKEYBD ON)

set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -shared")

set(DETECT_HOST TRUE CACHE INTERNAL "")
