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
// Legacy built-in GUI dialogs and controls.
//
//=============================================================================

#ifndef AGS_ENGINE_GUI_CSCI_DIALOG_H
#define AGS_ENGINE_GUI_CSCI_DIALOG_H

#include "ags/shared/core/types.h"
#include "ags/engine/gui/gui_dialog_defines.h"

namespace AGS3 {

#define MAXCONTROLS 20
#define MAXSCREENWINDOWS 5

int  CSCIGetVersion();
int  CSCIDrawWindow(int xx, int yy, int wid, int hit);
void CSCIEraseWindow(int handl);
int  CSCIWaitMessage(CSCIMessage *cscim);
int  CSCICreateControl(int typeandflags, int xx, int yy, int wii, int hii, const char *title);
void CSCIDeleteControl(int haa);
int  CSCISendControlMessage(int haa, int mess, int wPar, NumberPtr lPar);
void multiply_up_to_game_res(int *x, int *y);
void multiply_up(int *x1, int *y1, int *x2, int *y2);
int  checkcontrols();
int  finddefaultcontrol(int flagmask);
int  GetBaseWidth();

} // namespace AGS3

#endif
