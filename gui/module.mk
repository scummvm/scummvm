MODULE := gui

MODULE_OBJS := \
	gui/about.o \
	gui/browser.o \
	gui/chooser.o \
	gui/console.o \
	gui/dialog.o \
	gui/editable.o \
	gui/EditTextWidget.o \
	gui/launcher.o \
	gui/ListWidget.o \
	gui/message.o \
	gui/newgui.o \
	gui/options.o \
	gui/PopUpWidget.o \
	gui/ScrollBarWidget.o \
	gui/TabWidget.o \
	gui/widget.o \
	gui/theme.o \
	gui/ThemeNew.o

MODULE_DIRS += \
	gui

# Include common rules 
include $(srcdir)/common.rules
