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

#ifndef AGS_ENGINE_AC_DIALOG_OPTIONS_RENDERING_H
#define AGS_ENGINE_AC_DIALOG_OPTIONS_RENDERING_H

#include "ags/engine/ac/dynobj/script_dialog.h"
#include "ags/engine/ac/dynobj/script_dialog_options_rendering.h"

namespace AGS3 {

int  DialogOptionsRendering_GetX(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetX(ScriptDialogOptionsRendering *dlgOptRender, int newX);
int  DialogOptionsRendering_GetY(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetY(ScriptDialogOptionsRendering *dlgOptRender, int newY);
int  DialogOptionsRendering_GetWidth(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth);
int  DialogOptionsRendering_GetHeight(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetHeight(ScriptDialogOptionsRendering *dlgOptRender, int newHeight);
int  DialogOptionsRendering_GetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetHasAlphaChannel(ScriptDialogOptionsRendering *dlgOptRender, bool hasAlphaChannel);
int  DialogOptionsRendering_GetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxX(ScriptDialogOptionsRendering *dlgOptRender, int newX);
int  DialogOptionsRendering_GetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxY(ScriptDialogOptionsRendering *dlgOptRender, int newY);
int  DialogOptionsRendering_GetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetParserTextboxWidth(ScriptDialogOptionsRendering *dlgOptRender, int newWidth);
ScriptDialog *DialogOptionsRendering_GetDialogToRender(ScriptDialogOptionsRendering *dlgOptRender);
ScriptDrawingSurface *DialogOptionsRendering_GetSurface(ScriptDialogOptionsRendering *dlgOptRender);
int  DialogOptionsRendering_GetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender);
void DialogOptionsRendering_SetActiveOptionID(ScriptDialogOptionsRendering *dlgOptRender, int activeOptionID);

} // namespace AGS3

#endif
