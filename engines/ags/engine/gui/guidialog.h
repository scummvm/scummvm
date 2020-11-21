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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef AGS_ENGINE_GUI_GUIDIALOG_H
#define AGS_ENGINE_GUI_GUIDIALOG_H

namespace AGS { namespace Common { class Bitmap; } }

// Functions for handling hard-coded GUIs
// Prepares GUI bitmaps which will be passed to the renderer's draw chain
AGS::Common::Bitmap *prepare_gui_screen(int x, int y, int width, int height, bool opaque);
AGS::Common::Bitmap *get_gui_screen();
// Deletes GUI bitmaps
void clear_gui_screen();
// Draws virtual screen contents on the GUI bitmaps and assignes them to
// the renderer's draw chain
void refresh_gui_screen();
int  loadgamedialog();
int  savegamedialog();
void preparesavegamelist(int ctrllist);
void enterstringwindow(const char *prompttext, char *stouse);
int  enternumberwindow(char *prompttext);
int  roomSelectorWindow(int currentRoom, int numRooms, int*roomNumbers, char**roomNames);
int  myscimessagebox(const char *lpprompt, char *btn1, char *btn2);
int  quitdialog();

// last string value in gui dialog.
char *get_gui_dialog_buffer();

#endif
