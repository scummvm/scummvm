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

#include "ags/plugins/core/textbox.h"
#include "ags/engine/ac/textbox.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Textbox::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(TextBox::GetText^1, Textbox::GetText);
	SCRIPT_METHOD(TextBox::SetText^1, Textbox::SetText);
	SCRIPT_METHOD(TextBox::get_Font, Textbox::GetFont);
	SCRIPT_METHOD(TextBox::set_Font, Textbox::SetFont);
	SCRIPT_METHOD(TextBox::get_Text, Textbox::GetText_New);
	SCRIPT_METHOD(TextBox::set_Text, Textbox::SetText);
	SCRIPT_METHOD(TextBox::get_TextColor, Textbox::GetTextColor);
	SCRIPT_METHOD(TextBox::set_TextColor, Textbox::SetTextColor);
}

void Textbox::GetText(ScriptMethodParams &params) {
	PARAMS2(GUITextBox *, texbox, char *, buffer);
	AGS3::TextBox_GetText(texbox, buffer);
}

void Textbox::SetText(ScriptMethodParams &params) {
	PARAMS2(GUITextBox *, texbox, const char *, newtex);
	AGS3::TextBox_SetText(texbox, newtex);
}

void Textbox::GetText_New(ScriptMethodParams &params) {
	PARAMS1(GUITextBox *, texbox);
	params._result = AGS3::TextBox_GetText_New(texbox);
}

void Textbox::GetFont(ScriptMethodParams &params) {
	PARAMS1(GUITextBox *, texbox);
	params._result = AGS3::TextBox_GetFont(texbox);
}

void Textbox::SetFont(ScriptMethodParams &params) {
	PARAMS2(GUITextBox *, guit, int, fontnum);
	AGS3::TextBox_SetFont(guit, fontnum);
}

void Textbox::GetTextColor(ScriptMethodParams &params) {
	PARAMS1(GUITextBox *, texbox);
	params._result = AGS3::TextBox_GetTextColor(texbox);
}

void Textbox::SetTextColor(ScriptMethodParams &params) {
	PARAMS2(GUITextBox *, guit, int, colr);
	AGS3::TextBox_SetTextColor(guit, colr);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
