MODULE := gui

MODULE_OBJS := \
	about.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	editgamedialog.o \
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
	animation/Animation.o \
	animation/RepeatAnimationWrapper.o \
	animation/SequenceAnimationComposite.o \
	widget.o \
	widgets/editable.o \
	widgets/edittext.o \
	widgets/list.o \
	widgets/popup.o \
	widgets/scrollbar.o \
	widgets/scrollcontainer.o \
	widgets/tab.o

# HACK: create_project's XCode generator relies on the following ifdef
# structure to pick up the right browser implementations for iOS and Mac OS X.
# Please keep it like this or XCode project generation will be broken.
# FIXME: This only works because of a bug in how we handle ifdef statements in
# create_project's module.mk parser. create_project will think that both
# browser.o and browser_osx.o is built when both IPHONE and MACOSX is set.
# When we do proper ifdef handling, only browser.o will be picked up, breaking
# XCode generation.
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

ifdef USE_CLOUD
ifdef USE_LIBCURL
MODULE_OBJS += \
	downloaddialog.o \
	remotebrowser.o \
	storagewizarddialog.o
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

ifdef USE_UPDATES
MODULE_OBJS += \
	updates-dialog.o
endif

# Include common rules
include $(srcdir)/rules.mk
