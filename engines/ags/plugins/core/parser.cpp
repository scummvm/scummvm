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

#include "ags/plugins/core/parser.h"
#include "ags/engine/ac/parser.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void Parser::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(Parser::FindWordID^1, Parser::FindWordID);
	SCRIPT_METHOD(Parser::ParseText^1, Parser::ParseText);
	SCRIPT_METHOD(Parser::SaidUnknownWord^0, Parser::SaidUnknownWord);
	SCRIPT_METHOD(Parser::Said^1, Parser::Said);
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
