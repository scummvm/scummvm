/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or(at your option) any later version.
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

#include "common/system.h"
#include "ags/plugins/ags_clipboard/ags_clipboard.h"

namespace AGS3 {
namespace Plugins {
namespace AGSClipboard {

IAGSEngine *AGSClipboard::_engine;
Common::String *AGSClipboard::_text;

AGSClipboard::AGSClipboard() : PluginBase() {
	_engine = nullptr;
	_text = new Common::String();

	DLL_METHOD(AGS_GetPluginName);
	DLL_METHOD(AGS_EngineStartup);
	DLL_METHOD(AGS_EngineShutdown);
}

const char *AGSClipboard::AGS_GetPluginName() {
	return "AGS Clipboard Plugin v0.4";
}

void AGSClipboard::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Clipboard::PasteText, Clipboard_PasteText);
	SCRIPT_METHOD_EXT(Clipboard::CopyText^1, Clipboard_CopyText);
}

void AGSClipboard::AGS_EngineShutdown() {
	delete _text;
}

void AGSClipboard::Clipboard_PasteText(ScriptMethodParams &params) {
	Common::U32String text = g_system->getTextFromClipboard();
	*_text = Common::String(text);

	params._result = _text->c_str();
}

void AGSClipboard::Clipboard_CopyText(ScriptMethodParams &params) {
	PARAMS1(const char *, text);
	g_system->setTextInClipboard(Common::U32String(text));
}

} // namespace AGSClipboard
} // namespace Plugins
} // namespace AGS3
