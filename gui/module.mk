MODULE := gui

MODULE_OBJS := \
	gui/about.o \
	gui/browser.o \
	gui/chooser.o \
	gui/console.o \
	gui/dialog.o \
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

MODULE_DIRS += \
	gui

# Include common rules 
include common.rules
