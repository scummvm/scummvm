# Consolidated Wii/GameCube toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT DEVKITPRO)
  message(FATAL_ERROR "Please set DEVKITPRO in your environment.")
endif()

set(HOST_CPU "powerpc" CACHE INTERNAL "")
add_compile_definitions(SCUMM_BIG_ENDIAN)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_TINYGL OFF)
scummvm_toolkit_set_option(DISABLE_NUKED_OPL ON)
scummvm_toolkit_set_option(USE_ELF_LOADER ON)

if(NOT WII_TARGET)
  if(HOST STREQUAL "gamecube")
    set(WII_TARGET "gamecube")
  else()
    set(WII_TARGET "wii")
  endif()
endif()

set(WII_COMMON_FLAGS
    "-mcpu=750 -meabi -mhard-float -ffunction-sections -fdata-sections -fmodulo-sched -fuse-cxa-atexit"
)

if(WII_TARGET STREQUAL "gamecube")
  set(HOST_OS "gamecube" CACHE INTERNAL "")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${WII_COMMON_FLAGS} -mogc -I${DEVKITPRO}/libogc/include -I${DEVKITPRO}/portlibs/ppc/include"
  )
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} ${WII_COMMON_FLAGS} -mogc -L${DEVKITPRO}/libogc/lib/cube -L${DEVKITPRO}/portlibs/ppc/lib -logc"
  )
  add_compile_definitions(GAMECUBE AUDIO_REVERSE_STEREO USE_WII_DI)
else()
  set(HOST_OS "wii" CACHE INTERNAL "")
  set(CMAKE_CXX_FLAGS
      "${CMAKE_CXX_FLAGS} ${WII_COMMON_FLAGS} -mrvl -I${DEVKITPRO}/libogc/include -I${DEVKITPRO}/portlibs/ppc/include"
  )
  set(CMAKE_EXE_LINKER_FLAGS
      "${CMAKE_EXE_LINKER_FLAGS} ${WII_COMMON_FLAGS} -mrvl -L${DEVKITPRO}/libogc/lib/wii -L${DEVKITPRO}/portlibs/ppc/lib -logc"
  )
  add_compile_definitions(DEBUG_WII_USBGECKO USE_WII_DI USE_WII_SMB USE_WII_KBD)
endif()

set(DETECT_HOST TRUE CACHE INTERNAL "")
