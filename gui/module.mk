MODULE := gui

MODULE_OBJS = \
	gui/browser.o \
	gui/chooser.o \
	gui/dialog.o \
	gui/EditTextWidget.o \
	gui/launcher.o \
	gui/ListWidget.o \
	gui/message.o \
	gui/newgui.o \
	gui/options.o \
	gui/PopUpWidget.o \
	gui/ScrollBarWidget.o \
	gui/widget.o \

# Include common rules 
include common.rules
