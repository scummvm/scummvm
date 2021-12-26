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

#ifndef AGS_ENGINE_AC_TEXTBOX_H
#define AGS_ENGINE_AC_TEXTBOX_H

#include "ags/shared/gui/gui_textbox.h"

namespace AGS3 {

using AGS::Shared::GUITextBox;

const char *TextBox_GetText_New(GUITextBox *texbox);
void        TextBox_GetText(GUITextBox *texbox, char *buffer);
void        TextBox_SetText(GUITextBox *texbox, const char *newtex);
int         TextBox_GetTextColor(GUITextBox *guit);
void        TextBox_SetTextColor(GUITextBox *guit, int colr);
int         TextBox_GetFont(GUITextBox *guit);
void        TextBox_SetFont(GUITextBox *guit, int fontnum);

} // namespace AGS3

#endif
