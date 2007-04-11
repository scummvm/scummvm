/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
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

#ifndef PARALLACTION_COMMANDS_H
#define PARALLACTION_COMMANDS_H


#include "common/stdafx.h"
#include "common/scummsys.h"

#include "parallaction/defs.h"

namespace Parallaction {

enum CommandFlags {
	kFlagsVisited		= 1,
	kFlagsExit			= 0x10000000,
	kFlagsEnter 		= 0x20000000,
	kFlagsGlobal		= 0x40000000
};

struct Zone;
struct Animation;


// TODO: turn this into a struct
union CommandData {
	uint32			_flags;
	Animation * 	_animation;
	Zone*			_zone;
	char*			_string;
	uint16			_callable;
	uint16			_object;
	struct {
		int16		 _x;
		int16		_y;
	} _move;

	CommandData() {
		_flags = 0;
	}

	~CommandData() {
	}
};

struct Command {
	uint16			_id;
	CommandData 	u;
	uint32			_flagsOn;
	uint32			_flagsOff;

	Command();
	~Command();
};

//typedef Common::List<Command*> CommandList;
typedef ManagedList<Command*> CommandList;

} // namespace Parallaction

#endif
