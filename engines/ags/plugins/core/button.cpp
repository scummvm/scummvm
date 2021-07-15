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

#include "ags/plugins/core/button.h"
#include "ags/engine/ac/button.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Button::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Button::Animate^4, Animate);
	SCRIPT_METHOD_EXT(Button::GetText^1, GetText);
	SCRIPT_METHOD_EXT(Button::SetText^1, SetText);
	SCRIPT_METHOD_EXT(Button::get_ClipImage, GetClipImage);
	SCRIPT_METHOD_EXT(Button::set_ClipImage, SetClipImage);
	SCRIPT_METHOD_EXT(Button::get_Font, GetFont);
	SCRIPT_METHOD_EXT(Button::set_Font, SetFont);
	SCRIPT_METHOD_EXT(Button::get_Graphic, GetGraphic);
	SCRIPT_METHOD_EXT(Button::get_MouseOverGraphic, GetMouseOverGraphic);
	SCRIPT_METHOD_EXT(Button::set_MouseOverGraphic, SetMouseOverGraphic);
	SCRIPT_METHOD_EXT(Button::get_NormalGraphic, GetNormalGraphic);
	SCRIPT_METHOD_EXT(Button::set_NormalGraphic, SetNormalGraphic);
	SCRIPT_METHOD_EXT(Button::get_PushedGraphic, GetPushedGraphic);
	SCRIPT_METHOD_EXT(Button::set_PushedGraphic, SetPushedGraphic);
	SCRIPT_METHOD_EXT(Button::get_Text, GetText_New);
	SCRIPT_METHOD_EXT(Button::set_Text, SetText);
	SCRIPT_METHOD_EXT(Button::get_TextColor, GetTextColor);
	SCRIPT_METHOD_EXT(Button::set_TextColor, SetTextColor);
}

void Button::Animate(ScriptMethodParams &params) {
	PARAMS5(GUIButton *, butt, int, view, int, loop, int, speed, int, repeat);
	AGS3::Button_Animate(butt, view, loop, speed, repeat);
}

void Button::GetText(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, butt, char *, buffer);
	AGS3::Button_GetText(butt, buffer);
}

void Button::SetText(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, butt, const char *, newtx);
	AGS3::Button_SetText(butt, newtx);
}

void Button::GetClipImage(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetClipImage(butt);
}

void Button::SetClipImage(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, butt, int, newval);
	AGS3::Button_SetClipImage(butt, newval);
}

void Button::GetFont(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetFont(butt);
}

void Button::SetFont(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, butt, int, newFont);
	AGS3::Button_SetFont(butt, newFont);
}

void Button::GetGraphic(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetGraphic(butt);
}

void Button::GetMouseOverGraphic(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetMouseOverGraphic(butt);
}

void Button::SetMouseOverGraphic(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, guil, int, slotn);
	AGS3::Button_SetMouseOverGraphic(guil, slotn);
}

void Button::GetNormalGraphic(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetNormalGraphic(butt);
}

void Button::SetNormalGraphic(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, guil, int, slotn);
	AGS3::Button_SetNormalGraphic(guil, slotn);
}

void Button::GetPushedGraphic(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetPushedGraphic(butt);
}

void Button::SetPushedGraphic(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, guil, int, slotn);
	AGS3::Button_SetPushedGraphic(guil, slotn);
}

void Button::GetText_New(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetText_New(butt);
}

void Button::GetTextColor(ScriptMethodParams &params) {
	PARAMS1(GUIButton *, butt);
	params._result = AGS3::Button_GetTextColor(butt);
}

void Button::SetTextColor(ScriptMethodParams &params) {
	PARAMS2(GUIButton *, butt, int, newcol);
	AGS3::Button_SetTextColor(butt, newcol);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
