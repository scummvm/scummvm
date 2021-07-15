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

#include "ags/plugins/core/dialog_options_rendering_info.h"
#include "ags/engine/ac/dialog_options_rendering.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DialogOptionsRenderingInfo::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_ActiveOptionID, GetActiveOptionID);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_ActiveOptionID, SetActiveOptionID);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_DialogToRender, GetDialogToRender);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_Height, GetHeight);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_Height, SetHeight);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_ParserTextBoxX, GetParserTextboxX);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_ParserTextBoxX, SetParserTextboxX);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_ParserTextBoxY, GetParserTextboxY);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_ParserTextBoxY, SetParserTextboxY);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_ParserTextBoxWidth, GetParserTextboxWidth);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_ParserTextBoxWidth, SetParserTextboxWidth);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_Surface, GetSurface);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_Width, GetWidth);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_Width, SetWidth);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_X, GetX);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_X, SetX);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::get_Y, GetY);
	SCRIPT_METHOD_EXT(DialogOptionsRenderingInfo::set_Y, SetY);
}

void DialogOptionsRenderingInfo::GetActiveOptionID(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetActiveOptionID(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetActiveOptionID(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, activeOptionID);
	AGS3::DialogOptionsRendering_SetActiveOptionID(dlgOptRender, activeOptionID);
}

void DialogOptionsRenderingInfo::GetDialogToRender(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetDialogToRender(dlgOptRender);
}

void DialogOptionsRenderingInfo::GetHeight(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetHeight(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetHeight(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newHeight);
	AGS3::DialogOptionsRendering_SetHeight(dlgOptRender, newHeight);
}

void DialogOptionsRenderingInfo::GetParserTextboxX(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetParserTextboxX(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetParserTextboxX(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newX);
	AGS3::DialogOptionsRendering_SetParserTextboxX(dlgOptRender, newX);
}

void DialogOptionsRenderingInfo::GetParserTextboxY(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetParserTextboxY(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetParserTextboxY(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newY);
	AGS3::DialogOptionsRendering_SetParserTextboxY(dlgOptRender, newY);
}

void DialogOptionsRenderingInfo::GetParserTextboxWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetParserTextboxWidth(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetParserTextboxWidth(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newWidth);
	AGS3::DialogOptionsRendering_SetParserTextboxWidth(dlgOptRender, newWidth);
}

void DialogOptionsRenderingInfo::GetSurface(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetSurface(dlgOptRender);
}

void DialogOptionsRenderingInfo::GetWidth(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetWidth(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetWidth(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newWidth);
	AGS3::DialogOptionsRendering_SetWidth(dlgOptRender, newWidth);
}

void DialogOptionsRenderingInfo::GetX(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetX(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetX(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newX);
	AGS3::DialogOptionsRendering_SetX(dlgOptRender, newX);
}

void DialogOptionsRenderingInfo::GetY(ScriptMethodParams &params) {
	PARAMS1(ScriptDialogOptionsRendering *, dlgOptRender);
	params._result = AGS3::DialogOptionsRendering_GetY(dlgOptRender);
}

void DialogOptionsRenderingInfo::SetY(ScriptMethodParams &params) {
	PARAMS2(ScriptDialogOptionsRendering *, dlgOptRender, int, newY);
	AGS3::DialogOptionsRendering_SetY(dlgOptRender, newY);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
