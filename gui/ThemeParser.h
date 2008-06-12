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
 * $URL$
 * $Id$
 *
 */

#ifndef THEME_PARSER_H
#define THEME_PARSER_H

#include "common/scummsys.h"
#include "graphics/surface.h"
#include "common/system.h"

namespace GUI {

class ThemeParser {

	ThemeParser() {}
	~ThemeParser() {}

	enum ParserState {
		kParserKeyNeedName,
		kParserKeyNeedToken,
		kParserKeyNeedSubkey,
		kParserNeedKey,
		kParserInComment,
		kParserError
	};

	bool parseDrawData();
	void parserError(const char *error_string);
	
protected:
	int _pos;
	char *_text;

	ParserState _state;

	Common::String _error;
	Common::String _token;
};

}

#endif