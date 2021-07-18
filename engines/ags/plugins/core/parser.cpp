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

#include "ags/plugins/core/parser.h"
#include "ags/engine/ac/parser.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Parser::AGS_EngineStartup(IAGSEngine *engine) {
	SCRIPT_METHOD_EXT(Parser::FindWordID^1, FindWordID);
	SCRIPT_METHOD_EXT(Parser::ParseText^1, ParseText);
	SCRIPT_METHOD_EXT(Parser::SaidUnknownWord^0, SaidUnknownWord);
	SCRIPT_METHOD_EXT(Parser::Said^1, Said);
}

void Parser::FindWordID(ScriptMethodParams &params) {
	PARAMS1(const char *, wordToFind);
	params._result = AGS3::Parser_FindWordID(wordToFind);
}

void Parser::ParseText(ScriptMethodParams &params) {
	PARAMS1(const char *, text);
	AGS3::Parser_FindWordID(text);
}

void Parser::SaidUnknownWord(ScriptMethodParams &params) {
	AGS3::Parser_SaidUnknownWord();
}

void Parser::Said(ScriptMethodParams &params) {
	PARAMS1(const char *, checkWords);
	params._result = AGS3::Parser_FindWordID(checkWords);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
