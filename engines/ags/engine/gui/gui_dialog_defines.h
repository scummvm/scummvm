/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

//=============================================================================
//
// Constants for built-in GUI dialogs.
//
//=============================================================================

#ifndef AGS_ENGINE_GUI_GUI_DIALOG_DEFINES_H
#define AGS_ENGINE_GUI_GUI_DIALOG_DEFINES_H

#include "ags/engine/ac/game_setup.h"
#include "ags/globals.h"

namespace AGS3 {

#define MSG_RESTORE      984
#define MSG_CANCEL       985    // "Cancel"
#define MSG_SELECTLOAD   986    // "Select game to restore"
#define MSG_SAVEBUTTON   987    // "Save"
#define MSG_SAVEDIALOG   988    // "Save game name:"
#define MSG_REPLACE      989    // "Replace"
#define MSG_MUSTREPLACE  990    // "The folder is full. you must replace"
#define MSG_REPLACEWITH1 991    // "Replace:"
#define MSG_REPLACEWITH2 992    // "With:"
#define MSG_QUITBUTTON   993    // "Quit"
#define MSG_PLAYBUTTON   994    // "Play"
#define MSG_QUITDIALOG   995    // "Do you want to quit?"

#define TEXT_HT _GP(usetup).textheight

/*#define COL251 26
#define COL252 28
#define COL253 29
#define COL254 27
#define COL255 24*/
#define COL253 15
#define COL254 7
#define COL255 8

//  =========  DEFINES  ========
// Control types
#define CNT_PUSHBUTTON 0x001
#define CNT_LISTBOX    0x002
#define CNT_LABEL      0x003
#define CNT_TEXTBOX    0x004
// Control properties
#define CNF_DEFAULT    0x100
#define CNF_CANCEL     0x200

// Dialog messages
#define CM_COMMAND   1
#define CM_KEYPRESS  2
#define CM_SELCHANGE 3
// System messages
#define SM_SAVEGAME  100
#define SM_LOADGAME  101
#define SM_QUIT      102
// System messages (to ADVEN)
#define SM_SETTRANSFERMEM 120
#define SM_GETINIVALUE    121
// System messages (to driver)
#define SM_QUERYQUIT 110
#define SM_KEYPRESS  111
#define SM_TIMER     112
// ListBox messages
#define CLB_ADDITEM   1
#define CLB_CLEAR     2
#define CLB_GETCURSEL 3
#define CLB_GETTEXT   4
#define CLB_SETTEXT   5
#define CLB_SETCURSEL 6
// TextBox messages
#define CTB_GETTEXT   1
#define CTB_SETTEXT   2

#define CTB_KEYPRESS 91

namespace AGS {
namespace Shared {
class Bitmap;
}
}
using namespace AGS; // FIXME later

//  =========  STRUCTS  ========
#ifdef OBSOLETE
struct DisplayProperties {
	int width;
	int height;
	int colors;
	int textheight;
};
#endif // OBSOLETE

struct CSCIMessage {
	int code;
	int id;
	int wParam;
};

struct OnScreenWindow {
	int x, y;
	int handle;
	int oldtop;

	OnScreenWindow();
};

} // namespace AGS3

#endif
