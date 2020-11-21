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

#ifndef __AGS_EE_AC__TEXTBOX_H
#define __AGS_EE_AC__TEXTBOX_H

#include "gui/guitextbox.h"

using AGS::Common::GUITextBox;

const char*	TextBox_GetText_New(GUITextBox *texbox);
void		TextBox_GetText(GUITextBox *texbox, char *buffer);
void		TextBox_SetText(GUITextBox *texbox, const char *newtex);
int			TextBox_GetTextColor(GUITextBox *guit);
void		TextBox_SetTextColor(GUITextBox *guit, int colr);
int			TextBox_GetFont(GUITextBox *guit);
void		TextBox_SetFont(GUITextBox *guit, int fontnum);

#endif // __AGS_EE_AC__TEXTBOX_H
