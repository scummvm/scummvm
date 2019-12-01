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

#ifndef DIRECTOR_LINGO_LINGO_BYTECODE_H
#define DIRECTOR_LINGO_LINGO_BYTECODE_H

namespace Director {

typedef void (*inst)(void);

struct LingoV4Bytecode {
	const uint8 opcode;
	const inst func;
	const char *proto;
};

enum TheEntityArgsType {
	kTEANOArgs = 0,
	kTEAItemId = 1,
	kTEAString,
	kTEAMenuIdItemId
};

struct LingoV4TheEntity {
	const uint8 bank;
	const uint8 firstArg;
	const int entity;
	const int field;
	const bool writable;
	const TheEntityArgsType type;
};

} // End of namespace Director

#endif
