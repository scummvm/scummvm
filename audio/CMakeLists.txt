add_library(audio)

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
          softsynth/pcspk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          adlib_ctmidi.h
          adlib_ms.h
          audiostream.h
          casio.h
          chip.h
          cms.h
          fmopl.h
          mididrv.h
          mididrv_ms.h
          midiparser_qt.h
          midiparser_smf.h
          midiparser.h
          midiplayer.h
          mixer.h
          mpu401.h
          mt32gm.h
          musicplugin.h
          null.h
          rate.h
          timestamp.h
          decoders/3do.h
          decoders/aac.h
          decoders/adpcm.h
          decoders/aiff.h
          decoders/apc.h
          decoders/asf.h
          decoders/flac.h
          decoders/g711.h
          decoders/iff_sound.h
          decoders/mac_snd.h
          decoders/mp3.h
          decoders/qdm2.h
          decoders/quicktime.h
          decoders/raw.h
          decoders/voc.h
          decoders/vorbis.h
          decoders/wave.h
          decoders/wma.h
          decoders/xa.h
          decoders/xan_dpcm.h
          mods/universaltracker.h
          mods/infogrames.h
          mods/maxtrax.h
          mods/mod_xm_s3m.h
          mods/module.h
          mods/module_mod_xm_s3m.h
          mods/protracker.h
          mods/paula.h
          mods/rjp1.h
          mods/soundfx.h
          mods/tfmx.h
          softsynth/cms.h
          softsynth/opl/dbopl.h
          softsynth/opl/dosbox.h
          softsynth/opl/mame.h
          softsynth/pcspk.h)
if(NOT DISABLE_NUKED_OPL)
  target_sources(
    audio
    PRIVATE softsynth/opl/nuked.cpp
    PRIVATE FILE_SET HEADERS FILES softsynth/opl/nuked.h)
endif()

if(USE_A52)
  target_sources(
    audio
    PRIVATE decoders/ac3.cpp
    PRIVATE FILE_SET HEADERS FILES decoders/ac3.h)
endif()

if(USE_ALSA)
  target_sources(audio PRIVATE alsa_opl.cpp)
endif()

if(USE_FMTOWNS_PC98_AUDIO)
  target_sources(
    audio
    PRIVATE softsynth/fmtowns_pc98/pc98_audio.cpp
            softsynth/fmtowns_pc98/pcm_common.cpp
            softsynth/fmtowns_pc98/sega_audio.cpp
            softsynth/fmtowns_pc98/towns_audio.cpp
            softsynth/fmtowns_pc98/towns_euphony.cpp
            softsynth/fmtowns_pc98/towns_pc98_driver.cpp
            softsynth/fmtowns_pc98/towns_pc98_fmsynth.cpp
            softsynth/fmtowns_pc98/towns_pc98_plugins.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            softsynth/fmtowns_pc98/pc98_audio.h
            softsynth/fmtowns_pc98/pcm_common.h
            softsynth/fmtowns_pc98/sega_audio.h
            softsynth/fmtowns_pc98/towns_audio.h
            softsynth/fmtowns_pc98/towns_euphony.h
            softsynth/fmtowns_pc98/towns_pc98_driver.h
            softsynth/fmtowns_pc98/towns_pc98_fmsynth.h)
endif()

if(USE_MPCDEC)
  target_sources(
    audio
    PRIVATE decoders/mpc.cpp
    PRIVATE FILE_SET HEADERS FILES decoders/mpc.h)
endif()

if(USE_MT32EMU)
  target_sources(audio PRIVATE softsynth/mt32.cpp)
endif()

if(USE_SID_AUDIO)
  target_sources(
    audio
    PRIVATE softsynth/sid.cpp softsynth/wave6581.cpp
    PRIVATE FILE_SET HEADERS FILES softsynth/sid.h)
endif()

if(ENABLE_OPL2LPT)
  target_sources(audio PRIVATE opl2lpt.cpp)
endif()

if(USE_RETROWAVE)
  target_sources(
    audio
    PRIVATE rwopl3.cpp
    PRIVATE FILE_SET HEADERS FILES rwopl3.h)
endif()

if(USE_VGMTRANS_AUDIO)
  target_sources(
    audio
    PRIVATE soundfont/rawfile.cpp
            soundfont/rifffile.cpp
            soundfont/sf2file.cpp
            soundfont/synthfile.cpp
            soundfont/vgmcoll.cpp
            soundfont/vgminstrset.cpp
            soundfont/vgmitem.cpp
            soundfont/vgmsamp.cpp
            soundfont/vab/psxspu.cpp
            soundfont/vab/vab.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            soundfont/rawfile.h
            soundfont/rifffile.h
            soundfont/sf2file.h
            soundfont/synthfile.h
            soundfont/vgmcoll.h
            soundfont/vgminstrset.h
            soundfont/vgmitem.h
            soundfont/vgmsamp.h
            soundfont/vab/psxspu.h
            soundfont/vab/vab.h)
endif()
