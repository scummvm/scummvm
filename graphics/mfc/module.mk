MODULE := graphics/mfc

ifdef USE_MFC
MODULE_OBJS := \
	afx.o \
	bitmap.o \
	bitmap_button.o \
	brush.o \
	button.o \
	cmd_target.o \
	dialog.o \
	dc.o \
	doc_manager.o \
	doc_template.o \
	document.o \
	edit.o \
	file.o \
	font.o \
	frame_wnd.o \
	gdi_object.o \
	global_functions.o \
	ifstream.o \
	joystickapi.o \
	listbox.o \
	menu.o \
	object.o \
	paint_dc.o \
	palette.o \
	pen.o \
	rgn.o \
	scroll_view.o \
	scrollbar.o \
	single_doc_template.o \
	static.o \
	string.o \
	view.o \
	win_app.o \
	win_thread.o \
	wingdi.o \
	wnd.o \
	gfx/blitter.o \
	gfx/surface.o \
	gfx/cursor.o \
	gfx/dialog_template.o \
	gfx/fonts.o \
	gfx/palette_map.o \
	gfx/text_render.o \
	libs/event_loop.o \
	libs/events.o \
	libs/resources.o \
	libs/settings.o
endif

# Include common rules
include $(srcdir)/rules.mk
