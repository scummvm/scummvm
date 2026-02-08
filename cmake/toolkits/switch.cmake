# Nintendo Switch toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT DEVKITPRO)
  message(FATAL_ERROR "Please set DEVKITPRO in your environment.")
endif()

set(HOST_OS "switch" CACHE INTERNAL "")
set(HOST_CPU "aarch64" CACHE INTERNAL "")

add_compile_definitions(SWITCH __SWITCH__ NINTENDO_SWITCH)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)

set(SWITCH_FLAGS
    "-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE -ftls-model=local-exec -ffunction-sections -fdata-sections"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${SWITCH_FLAGS} -I${DEVKITPRO}/libnx/include -I${DEVKITPRO}/portlibs/switch/include"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -L${DEVKITPRO}/libnx/lib -L${DEVKITPRO}/portlibs/switch/lib -specs=${DEVKITPRO}/libnx/switch.specs"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
