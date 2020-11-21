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

#ifndef AGS_ENGINE_AC_BUTTON_H
#define AGS_ENGINE_AC_BUTTON_H

#include "gui/guibutton.h"

namespace AGS3 {

using AGS::Common::GUIButton;

void        Button_Animate(GUIButton *butt, int view, int loop, int speed, int repeat);
const char *Button_GetText_New(GUIButton *butt);
void        Button_GetText(GUIButton *butt, char *buffer);
void        Button_SetText(GUIButton *butt, const char *newtx);
void        Button_SetFont(GUIButton *butt, int newFont);
int         Button_GetFont(GUIButton *butt);
int         Button_GetClipImage(GUIButton *butt);
void        Button_SetClipImage(GUIButton *butt, int newval);
int         Button_GetGraphic(GUIButton *butt);
int         Button_GetMouseOverGraphic(GUIButton *butt);
void        Button_SetMouseOverGraphic(GUIButton *guil, int slotn);
int         Button_GetNormalGraphic(GUIButton *butt);
void        Button_SetNormalGraphic(GUIButton *guil, int slotn);
int         Button_GetPushedGraphic(GUIButton *butt);
void        Button_SetPushedGraphic(GUIButton *guil, int slotn);
int         Button_GetTextColor(GUIButton *butt);
void        Button_SetTextColor(GUIButton *butt, int newcol);

int         UpdateAnimatingButton(int bu);
void        StopButtonAnimation(int idxn);
void        FindAndRemoveButtonAnimation(int guin, int objn);

} // namespace AGS3

#endif
