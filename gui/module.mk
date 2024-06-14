MODULE := gui

MODULE_OBJS := \
	about.o \
	browser.o \
	chooser.o \
	console.o \
	debugger.o \
	dialog.o \
	dump-all-dialogs.o \
	editgamedialog.o \
	error.o \
	EventRecorder.o \
	filebrowser-dialog.o \
	gui-manager.o \
	helpdialog.o \
	imagealbum-dialog.o \
	launcher.o \
	massadd.o \
	message.o \
	MetadataParser.o \
	object.o \
	options.o \
	predictivedialog.o \
	saveload.o \
	saveload-dialog.o \
	shaderbrowser-dialog.o \
	textviewer.o \
	themebrowser.o \
	ThemeEngine.o \
	ThemeEval.o \
	ThemeLayout.o \
	ThemeParser.o \
	Tooltip.o \
	unknown-game-dialog.o \
	widget.o \
	animation/Animation.o \
	animation/RepeatAnimationWrapper.o \
	animation/SequenceAnimationComposite.o \
	widgets/editable.o \
	widgets/edittext.o \
	widgets/grid.o \
	widgets/groupedlist.o \
	widgets/list.o \
	widgets/popup.o \
	widgets/richtext.o \
	widgets/scrollbar.o \
	widgets/scrollcontainer.o \
	widgets/tab.o

ifdef USE_LIBCURL
ifdef USE_CLOUD
MODULE_OBJS += \
	cloudconnectionwizard.o \
	downloaddialog.o \
	remotebrowser.o
endif

MODULE_OBJS += \
	downloadpacksdialog.o \
	integrity-dialog.o
endif

ifdef USE_DLC
MODULE_OBJS += \
	dlcsdialog.o \
	downloaddlcsdialog.o
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
