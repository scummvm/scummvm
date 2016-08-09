/* ScummVM - Graphic Adventure Engine
*
* ScummVM is the legal property of its developers, whose names
* are too numerous to list here. Please refer to the COPYRIGHT
* file distributed with this source distribution.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

#ifndef MACVENTURE_PREBUIT_DIALOGS_H
#define MACVENTURE_PREBUIT_DIALOGS_H

#include "common/rect.h"

namespace MacVenture {

enum DialogAction {
  kDANone,
  kDACloseDialog,
  kDASubmit,
  kDASaveAs,
  kDALoadGame,
  kDAQuit,
  kDANewGame
};

enum PrebuiltDialogs {
  kSaveAsDialog = 0, //TODO: Currently unused, we are using ScummVM dialogs instead.
  kSpeakDialog = 1,
  kWinGameDialog = 2,
  kPrebuiltDialogCount
};

enum PrebuiltElementType {
  kDEPlainText,
  kDEButton,
  kDETextInput,
  kDEEnd
};

struct PrebuiltDialogElement {
  PrebuiltElementType type;
  Common::String title;
  DialogAction action;
  Common::Point position;
  uint width;
  uint height;
};

// Prebuilt dialogs
enum {
  // HACK
  kMaxPrebuiltDialogElements = 10
};

struct PrebuiltDialog {
  Common::Rect bounds;
  PrebuiltDialogElement elements[kMaxPrebuiltDialogElements];
};
} // End of namespace MacVenture

#endif
