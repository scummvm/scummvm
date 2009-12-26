MODULE := gui

MODULE_OBJS := \
	about.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	editable.o \
	EditTextWidget.o \
	GuiManager.o \
	launcher.o \
	ListWidget.o \
	massadd.o \
	message.o \
	object.o \
	options.o \
	PopUpWidget.o \
	saveload.o \
	ScrollBarWidget.o \
	TabWidget.o \
	themebrowser.o \
	ThemeEngine.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o \
	widget.o

ifdef MACOSX
MODULE_OBJS += \
	browser_osx.o
else
MODULE_OBJS += \
	browser.o
endif

# Include common rules
include $(srcdir)/rules.mk
