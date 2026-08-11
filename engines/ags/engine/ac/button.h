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

#ifndef AGS_ENGINE_AC_BUTTON_H
#define AGS_ENGINE_AC_BUTTON_H

#include "ags/globals.h"
#include "ags/shared/gui/gui_button.h"

namespace AGS3 {

using AGS::Shared::GUIButton;
struct AnimatingGUIButton;

void        Button_Animate(GUIButton *butt, int view, int loop, int speed, int repeat, int blocking, int direction, int sframe = 0, int volume = 100);
void        Button_Animate4(GUIButton *butt, int view, int loop, int speed, int repeat);
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

// Update button's animation, returns whether the animation continues
bool        UpdateAnimatingButton(int bu);
size_t      GetAnimatingButtonCount();
AnimatingGUIButton *GetAnimatingButtonByIndex(int idxn);
void        AddButtonAnimation(const AnimatingGUIButton &abtn);
void		StopButtonAnimation(int idxn);
int         FindButtonAnimation(int guin, int objn);
void		FindAndRemoveButtonAnimation(int guin, int objn);
void        RemoveAllButtonAnimations();

} // namespace AGS3

#endif
