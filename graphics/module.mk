MODULE := graphics

MODULE_OBJS := \
	big5.o \
	blit/blit.o \
	blit/blit-alpha.o \
	blit/blit-fast.o \
	blit/blit-generic.o \
	blit/blit-scale.o \
	color_quantizer.o \
	cursorman.o \
	font.o \
	fontman.o \
	fonts/amigafont.o \
	fonts/bdf.o \
	fonts/bgifont.o \
	fonts/consolefont.o \
	fonts/dosfont.o \
	fonts/freetype.o \
	fonts/macfont.o \
	fonts/newfont_big.o \
	fonts/newfont.o \
	fonts/ttf.o \
	fonts/winfont.o \
	framelimiter.o \
	image-archive.o \
	korfont.o \
	larryScale.o \
	maccursor.o \
	macgui/datafiles.o \
	macgui/macbutton.o \
	macgui/macdialog.o \
	macgui/macfontmanager.o \
	macgui/macmenu.o \
	macgui/macpopupmenu.o \
	macgui/mactext.o \
	macgui/mactext-canvas.o \
	macgui/mactext-md.o \
	macgui/mactextwindow.o \
	macgui/macwidget.o \
	macgui/macwindow.o \
	macgui/macwindowborder.o \
	macgui/macwindowmanager.o \
	managed_surface.o \
	nine_patch.o \
	opengl/context.o \
	opengl/debug.o \
	opengl/shader.o \
	opengl/texture.o \
	palette.o \
	pixelformat.o \
	pm5544.o \
	primitives.o \
	renderer.o \
	scalerplugin.o \
	scaler/downscaler.o \
	scaler/thumbnail_intern.o \
	screen.o \
	scaler/normal.o \
	sjis.o \
	surface.o \
	svg.o \
	transform_struct.o \
	transform_tools.o \
	thumbnail.o \
	VectorRenderer.o \
	VectorRendererSpec.o \
	wincursor.o \
	yuv_to_rgb.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/downscalerARM.o
endif

ifdef USE_MFC
MODULE_OBJS += \
	mfc/afx.o \
	mfc/bitmap.o \
	mfc/bitmap_button.o \
	mfc/brush.o \
	mfc/button.o \
	mfc/cmd_target.o \
	mfc/dialog.o \
	mfc/dc.o \
	mfc/doc_manager.o \
	mfc/doc_template.o \
	mfc/document.o \
	mfc/edit.o \
	mfc/file.o \
	mfc/font.o \
	mfc/frame_wnd.o \
	mfc/gdi_object.o \
	mfc/global_functions.o \
	mfc/ifstream.o \
	mfc/joystickapi.o \
	mfc/listbox.o \
	mfc/menu.o \
	mfc/object.o \
	mfc/paint_dc.o \
	mfc/palette.o \
	mfc/pen.o \
	mfc/rgn.o \
	mfc/scroll_view.o \
	mfc/scrollbar.o \
	mfc/single_doc_template.o \
	mfc/static.o \
	mfc/string.o \
	mfc/view.o \
	mfc/win_app.o \
	mfc/win_thread.o \
	mfc/wingdi.o \
	mfc/wnd.o \
	mfc/gfx/blitter.o \
	mfc/gfx/surface.o \
	mfc/gfx/cursor.o \
	mfc/gfx/dialog_template.o \
	mfc/gfx/fonts.o \
	mfc/gfx/palette_map.o \
	mfc/gfx/text_render.o \
	mfc/libs/event_loop.o \
	mfc/libs/events.o \
	mfc/libs/resources.o \
	mfc/libs/settings.o
endif

ifdef USE_TINYGL
MODULE_OBJS += \
	tinygl/api.o \
	tinygl/arrays.o \
	tinygl/clear.o \
	tinygl/clip.o \
	tinygl/fog.o \
	tinygl/get.o \
	tinygl/init.o \
	tinygl/light.o \
	tinygl/list.o \
	tinygl/matrix.o \
	tinygl/memory.o \
	tinygl/misc.o \
	tinygl/pixelbuffer.o \
	tinygl/select.o \
	tinygl/specbuf.o \
	tinygl/texture.o \
	tinygl/texelbuffer.o \
	tinygl/vertex.o \
	tinygl/zbuffer.o \
	tinygl/zline.o \
	tinygl/zmath.o \
	tinygl/ztriangle.o \
	tinygl/zblit.o \
	tinygl/zdirtyrect.o
endif

ifdef USE_ASPECT
MODULE_OBJS += \
	scaler/aspect.o
endif

ifdef USE_SCALERS
MODULE_OBJS += \
	scaler/dotmatrix.o \
	scaler/sai.o \
	scaler/pm.o \
	scaler/scale2x.o \
	scaler/scale3x.o \
	scaler/scalebit.o \
	scaler/tv.o

ifdef USE_ARM_SCALER_ASM
MODULE_OBJS += \
	scaler/scale2xARM.o \
	scaler/Normal2xARM.o
endif

ifdef USE_HQ_SCALERS
MODULE_OBJS += \
	scaler/hq.o

ifdef USE_NASM
MODULE_OBJS += \
	scaler/hq2x_i386.o \
	scaler/hq3x_i386.o
endif

endif

ifdef USE_EDGE_SCALERS
MODULE_OBJS += \
	scaler/edge.o
endif

endif

ifdef ATARI
MODULE_OBJS += \
	blit/blit-atari.o
endif

ifdef SCUMMVM_NEON
MODULE_OBJS += \
	blit/blit-neon.o
endif
ifdef SCUMMVM_SSE2
MODULE_OBJS += \
	blit/blit-sse2.o
endif
ifdef SCUMMVM_AVX2
MODULE_OBJS += \
	blit/blit-avx2.o
endif

# Include common rules
include $(srcdir)/rules.mk
