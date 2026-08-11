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

#include "ags/plugins/core/button.h"
#include "ags/engine/ac/button.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Button::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Button::Animate^4, Button::Animate);
	SCRIPT_METHOD(Button::GetText^1, Button::GetText);
	SCRIPT_METHOD(Button::SetText^1, Button::SetText);
	SCRIPT_METHOD(Button::get_ClipImage, Button::GetClipImage);
	SCRIPT_METHOD(Button::set_ClipImage, Button::SetClipImage);
	SCRIPT_METHOD(Button::get_Font, Button::GetFont);
	SCRIPT_METHOD(Button::set_Font, Button::SetFont);
	SCRIPT_METHOD(Button::get_Graphic, Button::GetGraphic);
	SCRIPT_METHOD(Button::get_MouseOverGraphic, Button::GetMouseOverGraphic);
	SCRIPT_METHOD(Button::set_MouseOverGraphic, Button::SetMouseOverGraphic);
	SCRIPT_METHOD(Button::get_NormalGraphic, Button::GetNormalGraphic);
	SCRIPT_METHOD(Button::set_NormalGraphic, Button::SetNormalGraphic);
	SCRIPT_METHOD(Button::get_PushedGraphic, Button::GetPushedGraphic);
	SCRIPT_METHOD(Button::set_PushedGraphic, Button::SetPushedGraphic);
	SCRIPT_METHOD(Button::get_Text, Button::GetText_New);
	SCRIPT_METHOD(Button::set_Text, Button::SetText);
	SCRIPT_METHOD(Button::get_TextColor, Button::GetTextColor);
	SCRIPT_METHOD(Button::set_TextColor, Button::SetTextColor);
}

void Button::Animate(ScriptMethodParams &params) {
	PARAMS5(GUIButton *, butt, int, view, int, loop, int, speed, int, repeat);
	AGS3::Button_Animate4(butt, view, loop, speed, repeat);
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
