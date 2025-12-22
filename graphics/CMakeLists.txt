add_library(graphics)

target_sources(
  graphics
  PRIVATE big5.cpp
          blit/blit.cpp
          blit/blit-alpha.cpp
          blit/blit-fast.cpp
          blit/blit-generic.cpp
          blit/blit-scale.cpp
          color_quantizer.cpp
          cursorman.cpp
          font.cpp
          fontman.cpp
          fonts/amigafont.cpp
          fonts/bdf.cpp
          fonts/bgifont.cpp
          fonts/consolefont.cpp
          fonts/dosfont.cpp
          fonts/freetype.cpp
          fonts/macfont.cpp
          fonts/newfont_big.cpp
          fonts/newfont.cpp
          fonts/ttf.cpp
          fonts/winfont.cpp
          framelimiter.cpp
          image-archive.cpp
          korfont.cpp
          larryScale.cpp
          maccursor.cpp
          macgui/datafiles.cpp
          macgui/macbutton.cpp
          macgui/macdialog.cpp
          macgui/macfontmanager.cpp
          macgui/macmenu.cpp
          macgui/macpopupmenu.cpp
          macgui/mactext.cpp
          macgui/mactext-canvas.cpp
          macgui/mactext-md.cpp
          macgui/mactextwindow.cpp
          macgui/macwidget.cpp
          macgui/macwindow.cpp
          macgui/macwindowborder.cpp
          macgui/macwindowmanager.cpp
          managed_surface.cpp
          nine_patch.cpp
          opengl/context.cpp
          opengl/debug.cpp
          opengl/shader.cpp
          opengl/texture.cpp
          palette.cpp
          pixelformat.cpp
          pm5544.cpp
          primitives.cpp
          renderer.cpp
          scalerplugin.cpp
          scaler/downscaler.cpp
          scaler/thumbnail_intern.cpp
          screen.cpp
          scaler/normal.cpp
          sjis.cpp
          surface.cpp
          svg.cpp
          transform_struct.cpp
          transform_tools.cpp
          thumbnail.cpp
          VectorRenderer.cpp
          VectorRendererSpec.cpp
          wincursor.cpp
          yuv_to_rgb.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          big5.h
          blit/blit-alpha.h
          color_quantizer.h
          cursorman.h
          font.h
          fontman.h
          fonts/amigafont.h
          fonts/bdf.h
          fonts/bgifont.h
          fonts/dosfont.h
          fonts/freetype.h
          fonts/macfont.h
          fonts/ttf.h
          fonts/winfont.h
          framelimiter.h
          image-archive.h
          korfont.h
          larryScale.h
          maccursor.h
          macgui/macbutton.h
          macgui/macdialog.h
          macgui/macfontmanager.h
          macgui/macmenu.h
          macgui/macpopupmenu.h
          macgui/mactext.h
          macgui/mactext-canvas.h
          macgui/mactextwindow.h
          macgui/macwidget.h
          macgui/macwindow.h
          macgui/macwindowborder.h
          macgui/macwindowmanager.h
          managed_surface.h
          nine_patch.h
          opengl/context.h
          opengl/debug.h
          opengl/shader.h
          opengl/texture.h
          palette.h
          pixelformat.h
          pm5544.h
          primitives.h
          renderer.h
          scalerplugin.h
          scaler/downscaler.h
          screen.h
          scaler/normal.h
          sjis.h
          surface.h
          svg.h
          transform_struct.h
          transform_tools.h
          thumbnail.h
          VectorRenderer.h
          VectorRendererSpec.h
          wincursor.h
          yuv_to_rgb.h
)
if(USE_ARM_SCALER_ASM)
  target_sources(graphics PRIVATE scaler/downscalerARM.cpp)
endif()

if(USE_TINYGL)
  target_sources(
    graphics
    PRIVATE tinygl/api.cpp
            tinygl/arrays.cpp
            tinygl/clear.cpp
            tinygl/clip.cpp
            tinygl/fog.cpp
            tinygl/get.cpp
            tinygl/init.cpp
            tinygl/light.cpp
            tinygl/list.cpp
            tinygl/matrix.cpp
            tinygl/memory.cpp
            tinygl/misc.cpp
            tinygl/pixelbuffer.cpp
            tinygl/select.cpp
            tinygl/specbuf.cpp
            tinygl/texture.cpp
            tinygl/texelbuffer.cpp
            tinygl/vertex.cpp
            tinygl/zbuffer.cpp
            tinygl/zline.cpp
            tinygl/zmath.cpp
            tinygl/ztriangle.cpp
            tinygl/zblit.cpp
            tinygl/zdirtyrect.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            tinygl/pixelbuffer.h
            tinygl/texelbuffer.h
            tinygl/zbuffer.h
            tinygl/zmath.h
            tinygl/zblit.h
            tinygl/zdirtyrect.h
  )
endif()

if(USE_ASPECT)
  target_sources(
    graphics
    PRIVATE scaler/aspect.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            scaler/aspect.h
  )
endif()

if(USE_SCALERS)
  target_sources(
    graphics
    PRIVATE scaler/dotmatrix.cpp
            scaler/sai.cpp
            scaler/pm.cpp
            scaler/scale2x.cpp
            scaler/scale3x.cpp
            scaler/scalebit.cpp
            scaler/tv.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            scaler/dotmatrix.h
            scaler/sai.h
            scaler/pm.h
            scaler/scale2x.h
            scaler/scale3x.h
            scaler/scalebit.h
            scaler/tv.h
  )
  if(USE_ARM_SCALER_ASM)
    target_sources(graphics PRIVATE scaler/scale2xARM.cpp scaler/Normal2xARM.cpp)
  endif()

  if(USE_HQ_SCALERS)
    target_sources(
      graphics
      PRIVATE scaler/hq.cpp
      PRIVATE FILE_SET
              HEADERS
              FILES
              scaler/hq.h
    )
    if(USE_NASM)
      target_sources(graphics PRIVATE scaler/hq2x_i386.cpp scaler/hq3x_i386.cpp)
    endif()

  endif()

  if(USE_EDGE_SCALERS)
    target_sources(
      graphics
      PRIVATE scaler/edge.cpp
      PRIVATE FILE_SET
              HEADERS
              FILES
              scaler/edge.h
    )
  endif()

endif()

if(ATARI)
  target_sources(graphics PRIVATE blit/blit-atari.cpp)
endif()

if(SCUMMVM_NEON)
  target_sources(graphics PRIVATE blit/blit-neon.cpp)
endif()

if(SCUMMVM_SSE2)
  target_sources(graphics PRIVATE blit/blit-sse2.cpp)
endif()

if(SCUMMVM_AVX2)
  target_sources(graphics PRIVATE blit/blit-avx2.cpp)
endif()
