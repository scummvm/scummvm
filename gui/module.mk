MODULE := gui

MODULE_OBJS := \
	about.o \
	browser.o \
	chooser.o \
	console.o \
	dialog.o \
	editable.o \
	EditTextWidget.o \
	launcher.o \
	ListWidget.o \
	message.o \
	newgui.o \
	options.o \
	PopUpWidget.o \
	ScrollBarWidget.o \
	TabWidget.o \
	widget.o \
	theme.o \
	ThemeNew.o

MODULE_DIRS += \
	gui

# Include common rules 
include $(srcdir)/common.rules
