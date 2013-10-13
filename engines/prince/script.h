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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef PRINCE_SCRIPT_H
#define PRINCE_SCRIPT_H

#include "common/random.h"

namespace Common {
    class SeekableReadStream;
}

namespace Prince {

class PrinceEngine;

class Script
{
public:
    Script(PrinceEngine *vm);
    virtual ~Script();

    bool loadFromStream(Common::SeekableReadStream &stream);

    void step();

private:
    PrinceEngine *_vm;

	Common::RandomSource _random;

	byte *_code;
	uint16 _codeSize;
	uint16 _currentInstruction;

	// Stack
	uint16 _stack[0x20];
	uint8 _stacktop;
	uint8 _savedStacktop;

	// Helper functions
	uint8 getCodeByte(uint16 address);
	uint8 readScript8bits();
	uint16 readScript16bits();
	uint32 readScript32bits();
	uint16 readScript8or16bits();

	typedef void (Script::*OpcodeFunc)();
	static OpcodeFunc _opcodes[];
};

}

#endif
