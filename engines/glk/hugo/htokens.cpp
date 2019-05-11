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

#include "glk/hugo/htokens.h"
#include "common/algorithm.h"

namespace Glk {
namespace Hugo {

const char *const HTokens::token[] = {
	/* 0x00 - 0x0f */
	"",  "(", ")", ".", ":", "=", "-", "+",
	"*", "/", "|", ";", "{", "}", "[", "]",

	/* 0x10 - 0x1f */
	"#", "~", ">=", "<=", "~=", "&", ">", "<",
	"if", ",", "else", "elseif", "while", "do", "select", "case",

	/* 0x20 - 0x2f */
	"for", "return", "break",  "and", "or", "jump", "run", "is",
	"not", "true", "false", "local", "verb", "xverb", "held", "multi",

	/* 0x30 - 0x3f */
	"multiheld", "newline", "anything", "print",
	"number", "capital", "text", "graphics",
	"color", "remove", "move", "to",
	"parent", "sibling", "child", "youngest",

	/* 0x40 - 0x4f */
	"eldest", "younger", "elder", "prop#",
	"attr#", "var#", "dictentry#", "textdata#",
	"routine#","debugdata#","objectnum#", "value#",
	"eol#", "system", "notheld", "multinotheld",

	/* 0x50 - 0x5f */
	"window", "random", "word", "locate",
	"parse$", "children", "in", "pause",
	"runevents", "arraydata#", "call", "stringdata#",
	"save", "restore", "quit", "input",

	/* 0x60 - 0x6f */
	"serial$", "cls", "scripton", "scriptoff",
	"restart", "hex", "object", "xobject",
	"string", "array", "printchar", "undo",
	"dict", "recordon", "recordoff", "writefile",

	/* 0x70 - */
	"readfile", "writeval", "readval", "playback",
	"colour", "picture", "label#", "sound",
	"music", "repeat", "addcontext", "video"
};

int HTokens::token_hash[TOKENS + 1];

HTokens::HTokens() {
	Common::fill(&token_hash[0], &token_hash[TOKENS + 1], 0);
}

} // End of namespace Hugo
} // End of namespace Glk
