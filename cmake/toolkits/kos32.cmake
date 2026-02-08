# Kolibri OS (kos32) toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".dll" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".dll")

if(NOT KOS32_SDK_DIR OR NOT KOS32_AUTOBUILD)
  message(FATAL_ERROR "Please set KOS32_SDK_DIR and KOS32_AUTOBUILD in your environment.")
endif()

set(HOST_OS "kolibrios" CACHE INTERNAL "")
set(HOST_CPU "i686" CACHE INTERNAL "")

add_compile_definitions(KOLIBRIOS _POSIX_C_SOURCE=1 _XOPEN_SOURCE=1)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
# Special defines from configure
add_compile_definitions(
  -U__WIN32__
  -U_Win32
  -U_WIN32
  -U__MINGW32__
  -UWIN32
)

set(KOS_CXX_FLAGS
    "-I${KOS32_SDK_DIR}/sources/newlib/libc/include -I${KOS32_SDK_DIR}/sources/libstdc++-v3/include -fno-ident -fomit-frame-pointer"
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KOS_CXX_FLAGS}")

# Linker flags from configure
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -specs=${CMAKE_SOURCE_DIR}/backends/platform/sdl/kolibrios/kolibrios.spec"
)

# Library paths
set(ZLIB_CFLAGS "-I${KOS32_SDK_DIR}/sources/zlib")
set(PNG_CFLAGS "-I${KOS32_SDK_DIR}/sources/libpng")
set(PNG_LIBS "-lpng16 -lz")
set(JPEG_CFLAGS "-I${KOS32_SDK_DIR}/sources/libjpeg")
set(FREETYPE2_CFLAGS "-I${KOS32_SDK_DIR}/sources/freetype/include")
set(SDL_CFLAGS "-I${KOS32_SDK_DIR}/sources/SDL-1.2.2_newlib/include")
set(SDL_LIBS "-lSDLn -lsound")

set(DETECT_HOST TRUE CACHE INTERNAL "")
