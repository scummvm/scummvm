# Manage all of the dependencies and optional libraries used in ScummVM
include(CheckSymbolExists)
find_package(PkgConfig REQUIRED) # Required for pkg_check_modules

# SDL Detection
macro(scummvm_find_sdl)
  find_package(SDL2 CONFIG)
  if(SDL2_FOUND)
    add_library(SDL::Any ALIAS SDL2::SDL2)
    message(STATUS "Found SDL2")
    option(USE_SDL2 "Use SDL2 as backend" ON)
    add_compile_definitions(SDL_BACKEND USE_SDL2)
  else()
    find_package(SDL3 CONFIG)
    if(SDL3_FOUND)
      add_library(SDL::Any ALIAS SDL3::SDL3)
      message(STATUS "Found SDL3")
      option(USE_SDL3 "Use SDL3 as backend" ON)
      add_compile_definitions(SDL_BACKEND USE_SDL3)
      # Fallback to SDL 1.2 (TODO: Can we remove this?)
    else()
      find_package(SDL)
      if(SDL_FOUND)
        add_library(SDL::Any INTERFACE IMPORTED)
        set_target_properties(
          SDL::Any PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${SDL_INCLUDE_DIR}"
                              INTERFACE_LINK_LIBRARIES "${SDL_LIBRARY}"
        )
      else()
        message(FATAL_ERROR "Could not find SDL3, SDL2, or SDL 1.2")
      endif()
    endif()
  endif()
endmacro()

if(BACKEND STREQUAL "sdl")
  option(SDL_BACKEND "Use SDL as backend" ON)
  scummvm_find_sdl()
  link_libraries(SDL::Any)
  add_compile_definitions(SDL_BACKEND)
endif()

# SDLNet
if(USE_SDL2)
  find_package(SDL2_net CONFIG)
  option(USE_SDL2_NET "Enable SDL2 Net support" ${SDL2_net_FOUND})
  if(USE_SDL2_NET)
    link_libraries(SDL2_net::SDL2_net)
    add_compile_definitions(USE_SDL2_NET)
  endif()
elseif(USE_SDL3)
  find_package(SDL3_net CONFIG)
  option(USE_SDL3_NET "Enable SDL3 Net support" ${SDL3_net_FOUND})
  if(USE_SDL3_NET)
    link_libraries(SDL3_net::SDL3_net)
    add_compile_definitions(USE_SDL3_NET)
  endif()
elseif(USE_SDL)
  find_package(SDL_net CONFIG)
  option(USE_SDL_NET "Enable SDL Net support" ${SDL_net_FOUND})
  if(USE_SDL_NET)
    link_libraries(SDL_net::SDL_net)
    add_compile_definitions(USE_SDL_NET)
  endif()
endif()

return()

# OGG
find_package(Ogg CONFIG)
option(USE_OGG "Enable OGG support" ${Ogg_FOUND})
if(USE_OGG)
  link_libraries(Ogg::ogg)
  add_compile_definitions(USE_OGG)
endif()

# Vorbis
find_package(Vorbis CONFIG)
option(USE_VORBIS "Enable Vorbis support" ${Vorbis_FOUND})
if(USE_VORBIS)
  link_libraries(Vorbis::vorbisfile)
  add_compile_definitions(USE_VORBIS)
endif()

# curl
find_package(CURL)
option(USE_CURL "Enable cURL support" ${CURL_FOUND})
if(USE_CURL)
  link_libraries(CURL::libcurl)
  add_compile_definitions(USE_CURL)
endif()

# ENET
find_package(unofficial-enet CONFIG)
option(USE_ENET "Enable ENet support" ${unofficial-enet_FOUND})
if(USE_ENET)
  check_symbol_exists(sendmsg "sys/socket.h" HAVE_SENDMSG)
  check_symbol_exists(recvmsg "sys/socket.h" HAVE_RECVMSG)
  if(HAVE_SENDMSG AND HAVE_RECVMSG)
    link_libraries(unofficial-enet::enet)
    add_compile_definitions(USE_ENET)
  else()
    message(
      WARNING "ENet was disabled because sendmsg or recvmsg are not available on this platform."
    )
    set(USE_ENET OFF CACHE BOOL "Enable ENet" FORCE)
  endif()
endif()

# Tremor

# FLAC
find_package(FLAC CONFIG)
option(USE_FLAC "Enable FLAC support" ${FLAC_FOUND})
if(USE_FLAC)
  link_libraries(FLAC::FLAC)
  add_compile_definitions(USE_FLAC)
endif()

# MAD
find_package(mad CONFIG)
option(USE_MAD "Enable MAD support" ${mad_FOUND})
if(USE_MAD)
  link_libraries(mad::mad)
  add_compile_definitions(USE_MAD)
endif()

#ZLib
find_package(ZLIB)
option(USE_ZLIB "Enable ZLib support" ${ZLIB_FOUND})
if(USE_ZLIB)
  link_libraries(ZLIB::ZLIB)
  add_compile_definitions(USE_ZLIB)
endif()

# PNG >= 1.2.8
find_package(PNG)
option(USE_PNG "Enable PNG support" ${PNG_FOUND})
if(USE_PNG)
  link_libraries(PNG::PNG)
  add_compile_definitions(USE_PNG)
endif()

# GIF >= 5.x
find_package(GIF)
option(USE_GIF "Enable GIF support" ${GIF_FOUND})
if(USE_GIF)
  link_libraries(GIF::GIF)
  add_compile_definitions(USE_GIF)
endif()

# Theoradec >- 1.x
find_package(unofficial-theora CONFIG)
option(USE_THEORADEC "Enable Theoradec support" ${unofficial-theora_FOUND})
if(USE_THEORADEC)
  link_libraries(unofficial::theora::theoradec)
  add_compile_definitions(USE_THEORADEC)
endif()

# VPX >= 1.6
find_package(unofficial-libvpx CONFIG)
option(USE_VPX "Enable VPX support" ${unofficial-libvpx_FOUND})
if(USE_VPX)
  link_libraries(unofficial::libvpx::libvpx)
  add_compile_definitions(USE_VPX)
endif()

# OpenMPT
find_package(libopenmpt CONFIG)
option(USE_OPENMPT "Enable OpenMPT support" ${libopenmpt_FOUND})
if(USE_OPENMPT)
  link_libraries(libopenmpt::libopenmpt)
  add_compile_definitions(USE_OPENMPT)
endif()

# MikMod (Mutually exclusive with OpenMPT)
pkg_check_modules(MIKMOD IMPORTED_TARGET libmikmod)
option(USE_MIKMOD "Enable MikMod support" ${MIKMOD_FOUND})
if(USE_MIKMOD)
  link_libraries(PkgConfig::MIKMOD)
  add_compile_definitions(USE_MIKMOD)
endif()

# FluidSynth
find_package(FluidSynth CONFIG)
option(USE_FLUIDSYNTH "Enable FluidSynth support" ${FluidSynth_FOUND})
if(USE_FLUIDSYNTH)
  link_libraries(FluidSynth::libfluidsynth)
  add_compile_definitions(USE_FLUIDSYNTH)
endif()

# FluidLite
find_package(fluidlite CONFIG)
option(USE_FLUIDLITE "Enable FluidLite support" ${fluidlite_FOUND})
if(USE_FLUIDLITE)
  link_libraries(fluidlite::fluidlite)
  add_compile_definitions(USE_FLUIDLITE)
endif()

# JPEG
find_package(JPEG)
option(USE_JPEG "Enable JPEG support" ${JPEG_FOUND})
if(USE_JPEG)
  link_libraries(JPEG::JPEG)
  add_compile_definitions(USE_JPEG)
endif()

# MPEG2
find_package(MPEG2)
option(USE_MPEG2 "Enable MPEG2 support" ${MPEG2_FOUND})
if(USE_MPEG2)
  add_library(MPEG2::MPEG2 INTERFACE IMPORTED)
  set_target_properties(
    MPEG2::MPEG2 PROPERTIES INTERFACE_LINK_LIBRARIES "${MPEG2_LIBRARIES}"
                            INTERFACE_INCLUDE_DIRECTORIES "${MPEG2_INCLUDE_DIR}"
  )
  link_libraries(MPEG2::MPEG2)
  add_compile_definitions(USE_MPEG2)
endif()
message(STATUS "MPEG2 has an optional dependency on SDL, you can ignore the message above")

# Discord RPC
set(DISCORD_RPC_FOUND FALSE)
find_library(DISCORD_RPC_LIBRARY discord-rpc)
if(DISCORD_RPC_LIBRARY)
  set(DISCORD_RPC_FOUND TRUE)
endif()
option(USE_DISCORD_RPC "Enable Discord RPC support" ${DISCORD_RPC_FOUND})
if(USE_DISCORD_RPC)
  link_libraries(${DISCORD_RPC_LIBRARY})
  add_compile_definitions(USE_DISCORD_RPC)
endif()

# Freetype
find_package(Freetype)
option(USE_FREETYPE "Enable FreeType support" ${Freetype_FOUND})
if(USE_FREETYPE)
  link_libraries(Freetype::Freetype)
  add_compile_definitions(USE_FREETYPE)
endif()

# ImgUI
find_package(imgui CONFIG)
option(USE_IMGUI "Enable Dear ImGui support" ${imgui_FOUND})
if(USE_IMGUI)
  link_libraries(imgui::imgui)
  add_compile_definitions(USE_IMGUI)
  if(USE_SDL2)
    add_compile_definitions(USE_IMGUI_SDLRENDERER2)
  elseif(USE_SDL3)
    add_compile_definitions(USE_IMGUI_SDLRENDERER3)
  else()
    message(WARNING "Dear ImGui requires SDL2 or SDL3")
  endif()
endif()

# Fribidi
pkg_check_modules(FRIBIDI IMPORTED_TARGET fribidi)
option(USE_FRIBIDI "Enable Freebidi support" ${FRIBIDI_FOUND})
if(USE_FRIBIDI)
  link_libraries(PkgConfig::FRIBIDI)
  add_compile_definitions(USE_FRIBIDI)
endif()

# Faad
pkg_check_modules(FAAD2 IMPORTED_TARGET faad2)
option(USE_FAAD "Enable Faad support" ${FAAD2_FOUND})
if(USE_FAAD)
  link_libraries(PkgConfig::FAAD2)
  add_compile_definitions(USE_FAAD)
endif()
