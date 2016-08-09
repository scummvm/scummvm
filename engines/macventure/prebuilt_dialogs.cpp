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

#include "macventure/prebuilt_dialogs.h"

namespace MacVenture {

PrebuiltDialog prebuiltDialogs[kPrebuiltDialogCount] = {

  {/* kSaveAsDialog */
    Common::Rect(0, 146, 456, 254),
    {
      {kDEButton, "YES", kDASaveAs, Common::Point(24, 68), 120, 22},
      {kDEButton, "NO", kDACloseDialog, Common::Point(168, 68), 120, 22},
      {kDEButton, "CANCEL", kDACloseDialog, Common::Point(312, 68), 120, 22},
      {kDEPlainText, "Save As...", kDANone, Common::Point(100, 10), 340, 38},
      {kDETextInput, "", kDANone, Common::Point(100, 30), 340, 20},
      {kDEEnd, "", kDANone, Common::Point(0, 0), 0, 0}
    }
  },

  { /* kSpeakDialog */
    Common::Rect(20, 92, 400, 200),
    {
      {kDEButton, "OK", kDASubmit, Common::Point(10, 70), 50, 20},
      {kDEButton, "CANCEL", kDACloseDialog, Common::Point(96, 70), 50, 20},
      {kDEPlainText, "What would you like to say?", kDANone, Common::Point(10, 10), 400, 20},
      {kDETextInput, "", kDANone, Common::Point(10, 25), 350, 40},
      {kDEEnd, "", kDANone, Common::Point(0, 0), 0, 0}
    }
},

  { /* kWinGameDialog */
	  Common::Rect(20, 100, 400, 200),
	  {
		  {kDEPlainText, "You Won!", kDANone, Common::Point(10, 10), 400, 20},
		  {kDEPlainText, "What do you want to do?", kDANone, Common::Point(10, 30), 400, 20},
		  {kDEButton, "New Game", kDANewGame, Common::Point(40, 30), 50, 20},
		  {kDEButton, "Load", kDALoadGame, Common::Point(100, 30), 50, 20},
		  {kDEButton, "Quit", kDAQuit, Common::Point(160, 30), 50, 20},
		  {kDEEnd, "", kDANone, Common::Point(0, 0), 0, 0}
	  }
  }

};
} // End of namespace MacVenture
