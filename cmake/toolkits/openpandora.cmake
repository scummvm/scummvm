# OpenPandora toolkit configuration

set(HOST_OS "linux" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(OPENPANDORA REDUCE_MEMORY_USAGE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_HQ_SCALERS ON)
scummvm_toolkit_set_option(USE_EDGE_SCALERS OFF)
scummvm_toolkit_set_option(USE_VKEYBD OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)

set(OP_FLAGS
    "-march=armv7-a -mtune=cortex-a8 -mfloat-abi=softfp -mfpu=neon -fsingle-precision-constant"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OP_FLAGS}")

set(DETECT_HOST TRUE CACHE INTERNAL "")
