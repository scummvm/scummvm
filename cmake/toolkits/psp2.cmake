# PS Vita (psp2) toolkit configuration

if(NOT VITASDK)
  message(FATAL_ERROR "Please set VITASDK in your environment.")
endif()

set(HOST_OS "psp2" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(PSP2 SYSTME_NOT_SUPPORTING_D_TYPE NONSTANDARD_PORT)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)

set(PSP2_FLAGS
    "--sysroot=${VITASDK}/arm-vita-eabi -Wl,-q -march=armv7-a -mtune=cortex-a9 -mfpu=neon -mfloat-abi=hard -fno-optimize-sibling-calls -mlong-calls -mword-relocations -fomit-frame-pointer"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PSP2_FLAGS} -I${VITASDK}/arm-vita-eabi/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${PSP2_FLAGS} -L${VITASDK}/arm-vita-eabi/lib")

add_compile_definitions(PSP2)

set(DETECT_HOST TRUE CACHE INTERNAL "")
