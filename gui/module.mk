MODULE := gui

MODULE_OBJS := \
	about.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	error.o \
	EventRecorder.o \
	filebrowser-dialog.o \
	gui-manager.o \
	launcher.o \
	massadd.o \
	message.o \
	object.o \
	options.o \
	predictivedialog.o \
	saveload.o \
	saveload-dialog.o \
	themebrowser.o \
	ThemeEngine.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o \
	Tooltip.o \
	widget.o \
	widgets/editable.o \
	widgets/edittext.o \
	widgets/list.o \
	widgets/popup.o \
	widgets/scrollbar.o \
	widgets/tab.o

# Even if it seems redundant, please keep
# these directives in that order!
# This is needed by the "create_project" tool, for the OS X / iOS Xcode project
ifdef IPHONE
MODULE_OBJS += \
	browser.o
else
ifdef MACOSX
MODULE_OBJS += \
	browser_osx.o
else
MODULE_OBJS += \
	browser.o
endif
endif

ifdef ENABLE_EVENTRECORDER
MODULE_OBJS += \
	editrecorddialog.o \
	onscreendialog.o \
	recorderdialog.o
endif

ifdef USE_FLUIDSYNTH
MODULE_OBJS += \
	fluidsynth-dialog.o
endif

# Include common rules
include $(srcdir)/rules.mk
