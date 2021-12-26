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

#ifndef AGS_ENGINE_AC_LABEL_H
#define AGS_ENGINE_AC_LABEL_H

#include "ags/shared/gui/gui_label.h"

namespace AGS3 {

using AGS::Shared::GUILabel;

const char *Label_GetText_New(GUILabel *labl);
void        Label_GetText(GUILabel *labl, char *buffer);
void        Label_SetText(GUILabel *labl, const char *newtx);
int         Label_GetColor(GUILabel *labl);
void        Label_SetColor(GUILabel *labl, int colr);
int         Label_GetFont(GUILabel *labl);
void        Label_SetFont(GUILabel *guil, int fontnum);

} // namespace AGS3

#endif
