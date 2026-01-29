add_library(agos STATIC)

target_sources(
  agos
  PRIVATE drivers/accolade/adlib.cpp
          drivers/accolade/casio.cpp
          drivers/accolade/cms.cpp
          drivers/accolade/driverfile.cpp
          drivers/accolade/pc98.cpp
          drivers/accolade/mt32.cpp
          drivers/simon1/adlib.cpp
          agos.cpp
          charset.cpp
          charset-fontdata.cpp
          contain.cpp
          cursor.cpp
          debug.cpp
          debugger.cpp
          dialogs.cpp
          draw.cpp
          event.cpp
          gfx.cpp
          icons.cpp
          input.cpp
          input_pn.cpp
          items.cpp
          menus.cpp
          metaengine.cpp
          midi.cpp
          midiparser_gmf.cpp
          midiparser_s1d.cpp
          midiparser_simonwin.cpp
          pn.cpp
          res.cpp
          res_ami.cpp
          res_snd.cpp
          rooms.cpp
          saveload.cpp
          script.cpp
          script_pn.cpp
          script_e1.cpp
          script_e2.cpp
          script_ww.cpp
          script_s1.cpp
          script_s2.cpp
          sfxparser_accolade.cpp
          sound.cpp
          string.cpp
          string_pn.cpp
          subroutine.cpp
          verb.cpp
          verb_pn.cpp
          vga.cpp
          vga_pn.cpp
          vga_e2.cpp
          vga_ww.cpp
          vga_s1.cpp
          vga_s2.cpp
          window.cpp
          zones.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          drivers/accolade/adlib.h
          drivers/accolade/casio.h
          drivers/accolade/cms.h
          drivers/accolade/mt32.h
          drivers/simon1/adlib.h
          agos.h
          debug.h
          debugger.h
          dialogs.h
          midi.h
          midiparser_gmf.h
          midiparser_simonwin.h
          sfxparser_accolade.h
          sound.h
          vga.h
)

if(ENABLE_AGOS2)
  target_sources(
    agos
    PRIVATE animation.cpp
            feeble.cpp
            oracle.cpp
            script_dp.cpp
            script_ff.cpp
            script_pp.cpp
            vga_ff.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            animation.h
  )

endif()

add_library(agos_detection)

target_sources(agos_detection PRIVATE detection.cpp)
