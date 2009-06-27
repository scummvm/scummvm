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
 * $URL$
 * $Id$
 *
 */

#ifndef DRACI_SCRIPT_H
#define DRACI_SCRIPT_H

#include "common/str.h"
#include "common/stream.h"

namespace Draci {

/** The maximum number of parameters for a GPL command */
const int kMaxParams = 3; 

// FIXME: Add function handlers

/**
 *  Represents a single command in the GPL scripting language bytecode.
 *	Each command is represented in the bytecode by a command number and a 
 *	subnumber.
 */

struct GPL2Command { 
	byte _number; 
	byte _subNumber; 
	Common::String _name; 
	uint16 _numParams;
	int _paramTypes[kMaxParams];
};

class Script {

public:
	int run(byte *gplcode, uint16 len);
	
private:
	GPL2Command *findCommand(byte num, byte subnum);
	void handleMathExpression(Common::MemoryReadStream &reader);

};

}

#endif // DRACI_SCRIPT_H
