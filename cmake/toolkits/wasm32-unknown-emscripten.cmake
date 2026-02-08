# Emscripten toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".html" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".html")

if(NOT EMSDK)
  message(FATAL_ERROR "Please set EMSDK in your environment.")
endif()

set(HOST_OS "emscripten" CACHE INTERNAL "")
set(HOST_CPU "wasm32" CACHE INTERNAL "")

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_ENET OFF)
scummvm_toolkit_set_option(USE_SDL_NET OFF)
scummvm_toolkit_set_option(USE_CURL OFF)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_SNDIO OFF)
scummvm_toolkit_set_option(USE_CLOUD ON)

add_compile_definitions(EMSCRIPTEN)

set(EMS_LD_FLAGS
    "-s INITIAL_MEMORY=128MB -s STACK_SIZE=32MB -s ALLOW_MEMORY_GROWTH=1 -s FORCE_FILESYSTEM -lidbfs.js -s ASYNCIFY=1 -s ASYNCIFY_STACK_SIZE=1048576 -s EXPORTED_FUNCTIONS=[_main,_malloc,_free,_raise] -s EXPORTED_RUNTIME_METHODS=[ccall,lengthBytesUTF8,setValue,writeArrayToMemory] -O2"
)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMS_LD_FLAGS}")

set(DETECT_HOST TRUE CACHE INTERNAL "")
