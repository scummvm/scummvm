# This file is responsible for registering and turning on/off optional engine components and
# features

macro(scummvm_add_component id name define_name)
  option(${define_name} "Enable ${name} support" OFF)
  # set("_component_${id}" ${define_name} CACHE INTERNAL "Definition name for ${name}")
  set("_component_${id}_define" ${define_name})
  set("_component_${id}_name" ${name})
endmacro()

macro(scummvm_add_feature id name define_name)
  option(${define_name} "Enable ${name} support" OFF)
  set("_feature_${id}_define" ${define_name})
  set("_feature_${id}_name" ${name})
endmacro()

macro(enable_component id)
  if(DEFINED "_component_${id}_define")
    set("${_component_${id}_define}" CACHE BOOL ON FORCE)
  else()
    message(WARNING "Component ${id} is undefined.")
  endif()
endmacro()

macro(enable_components components)
  foreach(item ${components})
    enable_component(${item})
  endforeach()
endmacro()

# Components
scummvm_add_component("enet" "ENet" "USE_ENET")
scummvm_add_component("fmtowns_pc98_audio" "FM-TOWNS/PC98 audio" "USE_FMTOWNS_PC98_AUDIO")
scummvm_add_component("gif" "GIF" "USE_GIF")
scummvm_add_component("hnm" "HNM" "USE_HNM")
scummvm_add_component("imgui" "Dear ImGui based debugger" "USE_IMGUI")
scummvm_add_component("indeo3" "Indeo 3" "USE_INDEO3")
scummvm_add_component("indeo45" "Indeo 4&5" "USE_INDEO45")
scummvm_add_component("lua" "Lua" "USE_LUA")
scummvm_add_component("vpx" "libvpx" "USE_VPX")
scummvm_add_component("theoradec" "libtheoradec" "USE_THEORADEC")
scummvm_add_component("midi" "MIDI synthesis" "USE_MIDI")
scummvm_add_component("mpc" "MPC" "USE_MPCDEC")
scummvm_add_component("mpeg2" "mpeg2" "USE_MPEG2")
scummvm_add_component("qdm2" "QDM2" "USE_QDM2")
scummvm_add_component("sid_audio" "SID audio" "USE_SID_AUDIO")
scummvm_add_component("svq1" "Sorenson Video 1" "USE_SVQ1")
scummvm_add_component("tinygl" "TinyGL" "USE_TINYGL")
scummvm_add_component("universaltracker" "External Tracker Libraries" "USE_UNIVERSALTRACKER")
scummvm_add_component("vgmtrans_audio" "VGMTrans Soundfont audio" "USE_VGMTRANS_AUDIO")

scummvm_add_feature("16bit" "16bit color" "USE_RGB_COLOR")
scummvm_add_feature("3d" "3D rendering" "USE_3D")
scummvm_add_feature("bink" "Bink" "USE_BINK")
scummvm_add_feature("cloud" "cloud" "USE_CLOUD")
scummvm_add_feature("faad" "libfaad" "USE_FAAD")
scummvm_add_feature("flac" "FLAC" "USE_FLAC")
scummvm_add_feature("fribidi" "FriBidi" "USE_FRIBIDI")
scummvm_add_feature("freetype2" "FreeType2" "USE_FREETYPE2")
scummvm_add_feature("highres" "high resolution" "USE_HIGHRES")
scummvm_add_feature("jpeg" "JPEG" "USE_JPEG")
scummvm_add_feature("mad" "MAD" "USE_MAD")
scummvm_add_feature("opengl_game_classic" "OpenGL (classic)" "USE_OPENGL_GAME")
scummvm_add_feature("opengl_game_shaders" "OpenGL with shaders" "USE_OPENGL_SHADERS")
scummvm_add_feature("png" "PNG" "USE_PNG")
scummvm_add_feature("vorbis" "Vorbis file support" "USE_VORBIS")
scummvm_add_feature("zlib" "zlib" "USE_ZLIB")
scummvm_add_feature("test_cxx11" "Test C++11" "ENABLE_TEST_CPP_11")

# Features
if(NOT USE_MIDI)
  set(USE_FLUIDSYNTH OFF CACHE BOOL "Enable FluidSynth support" FORCE)
  set(USE_FLUIDLITE OFF CACHE BOOL "Enable FluidLite support" FORCE)
  set(USE_TIMIDITY OFF CACHE BOOL "Enable Timidity support" FORCE)
  set(USE_SONIVOX OFF CACHE BOOL "Enable Sonivox support" FORCE)
  message(
    STATUS
      "Disabling mt32emu, fluidsynth, fluidlite, timidity and sonivox because MIDI is unused by enabled engines"
  )
endif()

if(NOT USE_UNIVERSALTRACKER)
  set(USE_MIKMOD OFF CACHE BOOL "Enable MikMod support" FORCE)
  set(USE_OPENMPT OFF CACHE BOOL "Enable OpenMPT support" FORCE)
  message(
    STATUS
      "Disabling MikMod and OpenMPT because Universal Tracker playback is unused by enabled engines"
  )
endif()
