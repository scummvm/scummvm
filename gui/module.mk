MODULE := gui

MODULE_OBJS := \
	about.o \
	browser.o \
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
	unknown-game-dialog.o \
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

ifdef USE_CLOUD
ifdef USE_LIBCURL
MODULE_OBJS += \
	downloaddialog.o \
	remotebrowser.o
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
