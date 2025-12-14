cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_library(gui)

target_link_libraries(gui PRIVATE engines backends)

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
          widgets/tab.cpp)
