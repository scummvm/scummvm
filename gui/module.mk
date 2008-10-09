MODULE := gui

MODULE_OBJS := \
	about.o \
	browser.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	editable.o \
	EditTextWidget.o \
	ThemeEngine.o \
	launcher.o \
	ListWidget.o \
	massadd.o \
	message.o \
	newgui.o \
	object.o \
	options.o \
	PopUpWidget.o \
	ScrollBarWidget.o \
	TabWidget.o \
	themebrowser.o \
	widget.o \
	theme.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o

# Include common rules
include $(srcdir)/rules.mk
