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

#include "common/stdafx.h"

#include "parallaction/parallaction.h"


namespace Parallaction {

#define CMD_SET 			1
#define CMD_CLEAR			2
#define CMD_START			3
#define CMD_SPEAK			4
#define CMD_GET 			5
#define CMD_LOCATION		6
#define CMD_OPEN			7
#define CMD_CLOSE			8
#define CMD_ON				9
#define CMD_OFF 			10
#define CMD_CALL			11
#define CMD_TOGGLE			12
#define CMD_DROP			13
#define CMD_QUIT			14
#define CMD_MOVE			15
#define CMD_STOP			16

DECLARE_COMMAND_PARSER(Flags) {
	if (_globalTable->lookup(_tokens[1]) == -1) {
		do {
			char _al = _localFlagNames->lookup(_tokens[_cmdParseCtxt.nextToken]);
			_cmdParseCtxt.nextToken++;
			_cmdParseCtxt.cmd->u._flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[_cmdParseCtxt.nextToken++], "|"));
		_cmdParseCtxt.nextToken--;
	} else {
		_cmdParseCtxt.cmd->u._flags |= kFlagsGlobal;
		do {
			char _al = _globalTable->lookup(_tokens[1]);
			_cmdParseCtxt.nextToken++;
			_cmdParseCtxt.cmd->u._flags |= 1 << (_al - 1);
		} while (!scumm_stricmp(_tokens[_cmdParseCtxt.nextToken++], "|"));
		_cmdParseCtxt.nextToken--;
	}
}


DECLARE_COMMAND_PARSER(Animation) {
	_cmdParseCtxt.cmd->u._animation = findAnimation(_tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
	if (_cmdParseCtxt.cmd->u._animation == NULL) {
		strcpy(_forwardedAnimationNames[_numForwards], _tokens[_cmdParseCtxt.nextToken-1]);
		_forwardedCommands[_numForwards] = _cmdParseCtxt.cmd;
		_numForwards++;
	}
}


DECLARE_COMMAND_PARSER(Zone) {
	_cmdParseCtxt.cmd->u._zone = findZone(_tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
}


DECLARE_COMMAND_PARSER(Location) {
	_cmdParseCtxt.cmd->u._string = (char*)malloc(strlen(_tokens[_cmdParseCtxt.nextToken])+1);
	strcpy(_cmdParseCtxt.cmd->u._string, _tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
}


DECLARE_COMMAND_PARSER(Drop) {
	_cmdParseCtxt.cmd->u._object = _objectsNames->lookup(_tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
}


DECLARE_COMMAND_PARSER(Call) {
	_cmdParseCtxt.cmd->u._callable = _callableNames->lookup(_tokens[_cmdParseCtxt.nextToken]) - 1;
	_cmdParseCtxt.nextToken++;
}


DECLARE_COMMAND_PARSER(Null) {
}


DECLARE_COMMAND_PARSER(Move) {
	_cmdParseCtxt.cmd->u._move._x = atoi(_tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
	_cmdParseCtxt.cmd->u._move._y = atoi(_tokens[_cmdParseCtxt.nextToken]);
	_cmdParseCtxt.nextToken++;
}




void Parallaction::parseCommands(Script &script, CommandList& list) {

	static const Opcode parsers[] = {
		COMMAND_PARSER(Flags),			// set
		COMMAND_PARSER(Flags),			// clear
		COMMAND_PARSER(Animation),		// start
		COMMAND_PARSER(Zone),			// speak
		COMMAND_PARSER(Zone),			// get
		COMMAND_PARSER(Location),		// location
		COMMAND_PARSER(Zone),			// open
		COMMAND_PARSER(Zone),			// close
		COMMAND_PARSER(Zone),			// on
		COMMAND_PARSER(Zone),			// off
		COMMAND_PARSER(Call),			// call
		COMMAND_PARSER(Flags),			// toggle
		COMMAND_PARSER(Drop),			// drop
		COMMAND_PARSER(Null),			// quit
		COMMAND_PARSER(Move),			// move
		COMMAND_PARSER(Animation)		// stop
	};

	_commandParsers = parsers;


	fillBuffers(script, true);

	while (scumm_stricmp(_tokens[0], "ENDCOMMANDS") && scumm_stricmp(_tokens[0], "ENDZONE")) {

		Command *cmd = new Command;

		cmd->_id = _commandsNames->lookup(_tokens[0]);

		_cmdParseCtxt.nextToken = 1;
		_cmdParseCtxt.cmd = cmd;

		(this->*_commandParsers[cmd->_id - 1])();

		int _si = _cmdParseCtxt.nextToken;

		if (!scumm_stricmp(_tokens[_si], "flags")) {
			_si++;

			do {
				if (!scumm_stricmp(_tokens[_si], "exit") || !scumm_stricmp(_tokens[_si], "exittrap")) {
					cmd->_flagsOn |= kFlagsExit;
				} else
				if (!scumm_stricmp(_tokens[_si], "enter") || !scumm_stricmp(_tokens[_si], "entertrap")) {
					cmd->_flagsOn |= kFlagsEnter;
				} else
				if (!scumm_strnicmp(_tokens[_si], "no", 2)) {
					byte _al = _localFlagNames->lookup(&_tokens[_si][2]);
					cmd->_flagsOff |= 1 << (_al - 1);
				} else {
					byte _al = _localFlagNames->lookup(_tokens[_si]);
					cmd->_flagsOn |= 1 << (_al - 1);
				}

				_si++;

			} while (!scumm_stricmp(_tokens[_si++], "|"));

		}

		if (!scumm_stricmp(_tokens[_si], "gflags")) {
			_si++;
			cmd->_flagsOn |= kFlagsGlobal;

			do {
				if (!scumm_stricmp(_tokens[_si], "exit")) {
					cmd->_flagsOn |= kFlagsExit;
				} else
				if (!scumm_stricmp(_tokens[_si], "enter")) {
					cmd->_flagsOn |= kFlagsEnter;
				} else
				if (!scumm_strnicmp(_tokens[_si], "no", 2)) {
					byte _al = _globalTable->lookup(&_tokens[_si][2]);
					cmd->_flagsOff |= 1 << (_al - 1);
				} else {
					byte _al = _globalTable->lookup(_tokens[_si]);
					cmd->_flagsOn |= 1 << (_al - 1);
				}

				_si++;

			} while (!scumm_stricmp(_tokens[_si++], "|"));

		}

		// FIXME: implement a proper parseCommands for BRA
		if (getGameType() == GType_BRA)
			delete cmd;
		else
			list.push_front(cmd);	// NOTE: command lists are written backwards in scripts

		fillBuffers(script, true);

	}
}


DECLARE_COMMAND_OPCODE(set) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags |= _cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] |= _cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(clear) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags &= ~_cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] &= ~_cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(start) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsActing;
}


DECLARE_COMMAND_OPCODE(speak) {
	_activeZone = _cmdRunCtxt.cmd->u._zone;
}


DECLARE_COMMAND_OPCODE(get) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsFixed;
	if (!runZone(_cmdRunCtxt.cmd->u._zone)) {
		runCommands(_cmdRunCtxt.cmd->u._zone->_commands);
	}
}


DECLARE_COMMAND_OPCODE(location) {
	strcpy(_location._name, _cmdRunCtxt.cmd->u._string);
	_engineFlags |= kEngineChangeLocation;
}


DECLARE_COMMAND_OPCODE(open) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsClosed;
	if (_cmdRunCtxt.cmd->u._zone->u.door->_cnv) {
		addJob(&jobToggleDoor, (void*)_cmdRunCtxt.cmd->u._zone, kPriority18 );
	}
}


DECLARE_COMMAND_OPCODE(close) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsClosed;
	if (_cmdRunCtxt.cmd->u._zone->u.door->_cnv) {
		addJob(&jobToggleDoor, (void*)_cmdRunCtxt.cmd->u._zone, kPriority18 );
	}
}


DECLARE_COMMAND_OPCODE(on) {
	// WORKAROUND: the original DOS-based engine didn't check u->_zone before dereferencing
	// the pointer to get structure members, thus leading to crashes in systems with memory
	// protection.
	// As a side note, the overwritten address is the 5th entry in the DOS interrupt table
	// (print screen handler): this suggests that a system would hang when the print screen
	// key is pressed after playing Nippon Safes, provided that this code path is taken.
	if (_cmdRunCtxt.cmd->u._zone != NULL) {
		_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsRemove;
		_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsActive;
		if ((_cmdRunCtxt.cmd->u._zone->_type & 0xFFFF) == kZoneGet) {
			addJob(&jobDisplayDroppedItem, _cmdRunCtxt.cmd->u._zone, kPriority17 );
		}
	}
}


DECLARE_COMMAND_OPCODE(off) {
	_cmdRunCtxt.cmd->u._zone->_flags |= kFlagsRemove;
}


DECLARE_COMMAND_OPCODE(call) {
	callFunction(_cmdRunCtxt.cmd->u._callable, _cmdRunCtxt.z);
}


DECLARE_COMMAND_OPCODE(toggle) {
	if (_cmdRunCtxt.cmd->u._flags & kFlagsGlobal) {
		_cmdRunCtxt.cmd->u._flags &= ~kFlagsGlobal;
		_commandFlags ^= _cmdRunCtxt.cmd->u._flags;
	} else {
		_localFlags[_currentLocationIndex] ^= _cmdRunCtxt.cmd->u._flags;
	}
}


DECLARE_COMMAND_OPCODE(drop){
	dropItem( _cmdRunCtxt.cmd->u._object );
}


DECLARE_COMMAND_OPCODE(quit) {
	_engineFlags |= kEngineQuit;
}


DECLARE_COMMAND_OPCODE(move) {
	if ((_char._ani._flags & kFlagsRemove) || (_char._ani._flags & kFlagsActive) == 0) {
		return;
	}

	WalkNodeList *vC = _char._builder.buildPath(_cmdRunCtxt.cmd->u._move._x, _cmdRunCtxt.cmd->u._move._y);

	addJob(&jobWalk, vC, kPriority19 );
	_engineFlags |= kEngineWalking;
}


DECLARE_COMMAND_OPCODE(stop) {
	_cmdRunCtxt.cmd->u._zone->_flags &= ~kFlagsActing;
}




void Parallaction::runCommands(CommandList& list, Zone *z) {
	debugC(1, kDebugLocation, "runCommands");

	static const Opcode opcodes[] = {
		COMMAND_OPCODE(set),
		COMMAND_OPCODE(clear),
		COMMAND_OPCODE(start),
		COMMAND_OPCODE(speak),
		COMMAND_OPCODE(get),
		COMMAND_OPCODE(location),
		COMMAND_OPCODE(open),
		COMMAND_OPCODE(close),
		COMMAND_OPCODE(on),
		COMMAND_OPCODE(off),
		COMMAND_OPCODE(call),
		COMMAND_OPCODE(toggle),
		COMMAND_OPCODE(drop),
		COMMAND_OPCODE(quit),
		COMMAND_OPCODE(move),
		COMMAND_OPCODE(stop)
	};

	_commandOpcodes = opcodes;

	CommandList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {

		Command *cmd = *it;
		uint32 v8 = _localFlags[_currentLocationIndex];

		if (_engineFlags & kEngineQuit)
			break;

		if (cmd->_flagsOn & kFlagsGlobal) {
			v8 = _commandFlags | kFlagsGlobal;
		}

		if ((cmd->_flagsOn & v8) != cmd->_flagsOn) continue;
		if ((cmd->_flagsOff & ~v8) != cmd->_flagsOff) continue;

		debugC(1, kDebugLocation, "runCommands: %s (on: %x, off: %x)", _commandsNamesRes[cmd->_id-1], cmd->_flagsOn, cmd->_flagsOff);

		_cmdRunCtxt.z = z;
		_cmdRunCtxt.cmd = cmd;

		(this->*_commandOpcodes[cmd->_id - 1])();
	}

	debugC(1, kDebugLocation, "runCommands completed");

	return;

}

Command::Command() {
	_id = 0;
	_flagsOn = 0;
	_flagsOff = 0;
}

Command::~Command() {

	if (_id == CMD_LOCATION) free(u._string);

}

} // namespace Parallaction



