add_library(gui)

target_sources(
  gui
  PRIVATE about.cpp
          browser.cpp
          chooser.cpp
          console.cpp
          debugger.cpp
          dialog.cpp
          dump-all-dialogs.cpp
          editgamedialog.cpp
          error.cpp
          EventRecorder.cpp
          filebrowser-dialog.cpp
          gui-manager.cpp
          helpdialog.cpp
          imagealbum-dialog.cpp
          launcher.cpp
          massadd.cpp
          message.cpp
          MetadataParser.cpp
          object.cpp
          options.cpp
          predictivedialog.cpp
          saveload.cpp
          saveload-dialog.cpp
          shaderbrowser-dialog.cpp
          textviewer.cpp
          themebrowser.cpp
          ThemeEngine.cpp
          ThemeEval.cpp
          ThemeLayout.cpp
          ThemeParser.cpp
          Tooltip.cpp
          unknown-game-dialog.cpp
          widget.cpp
          animation/Animation.cpp
          animation/RepeatAnimationWrapper.cpp
          animation/SequenceAnimationComposite.cpp
          widgets/editable.cpp
          widgets/edittext.cpp
          widgets/grid.cpp
          widgets/groupedlist.cpp
          widgets/list.cpp
          widgets/popup.cpp
          widgets/richtext.cpp
          widgets/scrollbar.cpp
          widgets/scrollcontainer.cpp
          widgets/tab.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          about.h
          browser.h
          chooser.h
          console.h
          debugger.h
          dialog.h
          dump-all-dialogs.h
          editgamedialog.h
          error.h
          EventRecorder.h
          filebrowser-dialog.h
          gui-manager.h
          helpdialog.h
          imagealbum-dialog.h
          launcher.h
          massadd.h
          message.h
          MetadataParser.h
          object.h
          options.h
          predictivedialog.h
          saveload.h
          saveload-dialog.h
          shaderbrowser-dialog.h
          textviewer.h
          themebrowser.h
          ThemeEngine.h
          ThemeEval.h
          ThemeLayout.h
          ThemeParser.h
          Tooltip.h
          unknown-game-dialog.h
          widget.h
          animation/Animation.h
          animation/RepeatAnimationWrapper.h
          animation/SequenceAnimationComposite.h
          widgets/editable.h
          widgets/edittext.h
          widgets/grid.h
          widgets/groupedlist.h
          widgets/list.h
          widgets/popup.h
          widgets/richtext.h
          widgets/scrollbar.h
          widgets/scrollcontainer.h
          widgets/tab.h
)
if(USE_CLOUD)
  target_sources(
    gui
    PRIVATE cloudconnectionwizard.cpp downloaddialog.cpp remotebrowser.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            cloudconnectionwizard.h
            downloaddialog.h
            remotebrowser.h
  )
endif()

if(USE_HTTP)
  target_sources(
    gui
    PRIVATE downloadpacksdialog.cpp integrity-dialog.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            downloadpacksdialog.h
            integrity-dialog.h
  )
endif()

if(USE_DLC)
  target_sources(
    gui
    PRIVATE dlcsdialog.cpp downloaddlcsdialog.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            dlcsdialog.h
            downloaddlcsdialog.h
  )
endif()

if(ENABLE_EVENTRECORDER)
  target_sources(
    gui
    PRIVATE editrecorddialog.cpp onscreendialog.cpp recorderdialog.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            editrecorddialog.h
            onscreendialog.h
            recorderdialog.h
  )
endif()

if(USE_FLUIDSYNTH)
  target_sources(
    gui
    PRIVATE fluidsynth-dialog.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fluidsynth-dialog.h
  )
endif()

if(USE_UPDATES)
  target_sources(
    gui
    PRIVATE updates-dialog.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            updates-dialog.h
  )
endif()
