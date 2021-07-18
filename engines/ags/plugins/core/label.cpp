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

#include "ags/plugins/core/label.h"
#include "ags/engine/ac/label.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Label::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Label::GetText^1, GetText);
	SCRIPT_METHOD_EXT(Label::SetText^1, SetText);
	SCRIPT_METHOD_EXT(Label::get_Font, GetFont);
	SCRIPT_METHOD_EXT(Label::set_Font, SetFont);
	SCRIPT_METHOD_EXT(Label::get_Text, GetText_New);
	SCRIPT_METHOD_EXT(Label::set_Text, SetText);
	SCRIPT_METHOD_EXT(Label::get_TextColor, GetColor);
	SCRIPT_METHOD_EXT(Label::set_TextColor, SetColor);
}

void Label::GetText(ScriptMethodParams &params) {
	PARAMS2(GUILabel *, labl, char *, buffer);
	AGS3::Label_GetText(labl, buffer);
}

void Label::SetText(ScriptMethodParams &params) {
	PARAMS2(GUILabel *, labl, const char *, newtx);
	AGS3::Label_SetText(labl, newtx);
}

void Label::GetFont(ScriptMethodParams &params) {
	PARAMS1(GUILabel *, labl);
	params._result = AGS3::Label_GetFont(labl);
}

void Label::SetFont(ScriptMethodParams &params) {
	PARAMS2(GUILabel *, guil, int, fontnum);
	AGS3::Label_SetFont(guil, fontnum);
}

void Label::GetText_New(ScriptMethodParams &params) {
	PARAMS1(GUILabel *, labl);
	params._result = AGS3::Label_GetText_New(labl);
}

void Label::GetColor(ScriptMethodParams &params) {
	PARAMS1(GUILabel *, labl);
	params._result = AGS3::Label_GetColor(labl);
}

void Label::SetColor(ScriptMethodParams &params) {
	PARAMS2(GUILabel *, labl, int, colr);
	AGS3::Label_SetColor(labl, colr);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
