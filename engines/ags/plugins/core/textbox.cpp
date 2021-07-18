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

#include "ags/plugins/core/textbox.h"
#include "ags/engine/ac/textbox.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Textbox::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(TextBox::GetText^1, GetText);
	SCRIPT_METHOD_EXT(TextBox::SetText^1, SetText);
	SCRIPT_METHOD_EXT(TextBox::get_Font, GetFont);
	SCRIPT_METHOD_EXT(TextBox::set_Font, SetFont);
	SCRIPT_METHOD_EXT(TextBox::get_Text, GetText_New);
	SCRIPT_METHOD_EXT(TextBox::set_Text, SetText);
	SCRIPT_METHOD_EXT(TextBox::get_TextColor, GetTextColor);
	SCRIPT_METHOD_EXT(TextBox::set_TextColor, SetTextColor);
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
