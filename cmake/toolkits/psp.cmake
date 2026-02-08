# PSP toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT PSPSDK)
  message(FATAL_ERROR "Please set PSPSDK in your environment.")
endif()

set(HOST_OS "psp" CACHE INTERNAL "")
set(HOST_CPU "mipsallegrexel" CACHE INTERNAL "")

add_compile_definitions(PSP NONSTANDARD_PORT)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_ELF_LOADER ON)

set(PSP_FLAGS "-I${PSPSDK}/include -D_PSP_FW_VERSION=150 -mno-gpopt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PSP_FLAGS}")

# Linker flags might need more tuning based on PSPDEV
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${PSPSDK}/lib")

set(DETECT_HOST TRUE CACHE INTERNAL "")
