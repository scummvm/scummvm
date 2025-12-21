add_library(audio)

target_link_libraries(audio PRIVATE common)

target_compile_definitions(audio PRIVATE DISABLE_NUKED_OPL)

if(PLATFORM_WINDOWS)
  target_link_libraries(audio PRIVATE winmm)
endif()

target_sources(
  audio
  PRIVATE adlib.cpp
          adlib_ctmidi.cpp
          adlib_ms.cpp
          audiostream.cpp
          casio.cpp
          chip.cpp
          cms.cpp
          fmopl.cpp
          mac_plugin.cpp
          mididrv.cpp
          mididrv_ms.cpp
          midiparser_qt.cpp
          midiparser_smf.cpp
          midiparser_xmidi.cpp
          midiparser.cpp
          midiplayer.cpp
          miles_adlib.cpp
          miles_midi.cpp
          mixer.cpp
          mpu401.cpp
          mt32gm.cpp
          musicplugin.cpp
          null.cpp
          rate.cpp
          timestamp.cpp
          decoders/3do.cpp
          decoders/aac.cpp
          decoders/adpcm.cpp
          decoders/aiff.cpp
          decoders/apc.cpp
          decoders/asf.cpp
          decoders/flac.cpp
          decoders/g711.cpp
          decoders/iff_sound.cpp
          decoders/mac_snd.cpp
          decoders/mp3.cpp
          decoders/qdm2.cpp
          decoders/quicktime.cpp
          decoders/raw.cpp
          decoders/voc.cpp
          decoders/vorbis.cpp
          decoders/wave.cpp
          decoders/wma.cpp
          decoders/xa.cpp
          decoders/xan_dpcm.cpp
          mods/universaltracker.cpp
          mods/infogrames.cpp
          mods/maxtrax.cpp
          mods/mod_xm_s3m.cpp
          mods/module.cpp
          mods/module_mod_xm_s3m.cpp
          mods/protracker.cpp
          mods/paula.cpp
          mods/rjp1.cpp
          mods/soundfx.cpp
          mods/tfmx.cpp
          softsynth/cms.cpp
          softsynth/opl/dbopl.cpp
          softsynth/opl/dosbox.cpp
          softsynth/opl/mame.cpp
          softsynth/appleiigs.cpp
          softsynth/fluidsynth.cpp
          softsynth/eas.cpp
          softsynth/pcspk.cpp)
