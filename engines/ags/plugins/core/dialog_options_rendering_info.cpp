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

#include "ags/plugins/core/dialog_options_rendering_info.h"
#include "ags/engine/ac/dialog_options_rendering.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void DialogOptionsRenderingInfo::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_ActiveOptionID, DialogOptionsRenderingInfo::GetActiveOptionID);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_ActiveOptionID, DialogOptionsRenderingInfo::SetActiveOptionID);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_DialogToRender, DialogOptionsRenderingInfo::GetDialogToRender);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_Height, DialogOptionsRenderingInfo::GetHeight);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_Height, DialogOptionsRenderingInfo::SetHeight);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_ParserTextBoxX, DialogOptionsRenderingInfo::GetParserTextboxX);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_ParserTextBoxX, DialogOptionsRenderingInfo::SetParserTextboxX);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_ParserTextBoxY, DialogOptionsRenderingInfo::GetParserTextboxY);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_ParserTextBoxY, DialogOptionsRenderingInfo::SetParserTextboxY);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_ParserTextBoxWidth, DialogOptionsRenderingInfo::GetParserTextboxWidth);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_ParserTextBoxWidth, DialogOptionsRenderingInfo::SetParserTextboxWidth);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_Surface, DialogOptionsRenderingInfo::GetSurface);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_Width, DialogOptionsRenderingInfo::GetWidth);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_Width, DialogOptionsRenderingInfo::SetWidth);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_X, DialogOptionsRenderingInfo::GetX);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_X, DialogOptionsRenderingInfo::SetX);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::get_Y, DialogOptionsRenderingInfo::GetY);
	SCRIPT_METHOD(DialogOptionsRenderingInfo::set_Y, DialogOptionsRenderingInfo::SetY);
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
