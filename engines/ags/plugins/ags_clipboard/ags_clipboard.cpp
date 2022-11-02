/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * of the License, or(at your option) any later version.
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

#include "common/system.h"
#include "ags/plugins/ags_clipboard/ags_clipboard.h"

namespace AGS3 {
namespace Plugins {
namespace AGSClipboard {

const char *AGSClipboard::AGS_GetPluginName() {
	return "AGS Clipboard Plugin v0.4";
}

void AGSClipboard::AGS_EngineStartup(IAGSEngine *engine) {
	PluginBase::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Clipboard::PasteText, AGSClipboard::Clipboard_PasteText);
	SCRIPT_METHOD(Clipboard::CopyText^1, AGSClipboard::Clipboard_CopyText);
}

void AGSClipboard::Clipboard_PasteText(ScriptMethodParams &params) {
	Common::U32String text = g_system->getTextFromClipboard();
	_text = text;

	params._result = _text.c_str();
}

void AGSClipboard::Clipboard_CopyText(ScriptMethodParams &params) {
	PARAMS1(const char *, text);
	g_system->setTextInClipboard(Common::U32String(text));
}

} // namespace AGSClipboard
} // namespace Plugins
} // namespace AGS3
