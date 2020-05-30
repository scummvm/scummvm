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

#ifndef GLK_COMPREHEND_OPCODE_MAP_H
#define GLK_COMPREHEND_OPCODE_MAP_H

#include "common/scummsys.h"

namespace Glk {
namespace Comprehend {

/*
 * Version 2 of the Comprehend engine (OO-Topos) changes some of the opcode
 * numbers and adds new opcodes. This class encapsulates a table to translate
 * the opcodes used in the original games into a generic version used by the engine
 *
 * FIXME - unimplemented/unknown ocpodes:
 *
 * d5(obj): Make object visible. This will print a "you see: object" when the
 *          object is in the room.
 */
class OpcodeMap {
public:
	byte _opcodeMap[0x100];

private:
	void loadVersion1();
	void loadVersion2();

public:
	void loadOpcodes(int version);
};

} // namespace Comprehend
} // namespace Glk

#endif
