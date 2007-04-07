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

#include "parallaction/parallaction.h"
#include "parallaction/parser.h"
#include "parallaction/commands.h"
#include "parallaction/walk.h"
#include "parallaction/zone.h"

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


void Parallaction::parseCommands(Script &script, CommandList& list) {
//	printf("parseCommands()");

	fillBuffers(script, true);

	while (scumm_stricmp(_tokens[0], "ENDCOMMANDS") && scumm_stricmp(_tokens[0], "ENDZONE")) {
//		printf("token[0] = %s", _tokens[0]);

		Command *cmd = new Command;

		cmd->_id = _commandsNames->lookup(_tokens[0]);
		uint16 _si = 1;

//		printf("cmd id = %i", cmd->_id);

		switch (cmd->_id) {
		case CMD_SET:	// set
		case CMD_CLEAR: // clear
		case CMD_TOGGLE:	// toggle
			if (_globalTable->lookup(_tokens[1]) == -1) {
				do {
					char _al = _localFlagNames->lookup(_tokens[_si]);
					_si++;
					cmd->u._flags |= 1 << (_al - 1);
				} while (!scumm_stricmp(_tokens[_si++], "|"));
				_si--;
			} else {
				cmd->u._flags |= kFlagsGlobal;
				do {
					char _al = _globalTable->lookup(_tokens[1]);
					_si++;
					cmd->u._flags |= 1 << (_al - 1);
				} while (!scumm_stricmp(_tokens[_si++], "|"));
				_si--;
			}
			break;

		case CMD_START: // start
		case CMD_STOP:	// stop
			cmd->u._animation = _vm->findAnimation(_tokens[_si]);
			_si++;
			if (cmd->u._animation == NULL) {
				strcpy(_forwardedAnimationNames[_numForwards], _tokens[_si-1]);
				_forwardedCommands[_numForwards] = cmd;
				_numForwards++;
			}
			break;

		case CMD_SPEAK: // speak
		case CMD_GET:	// get
		case CMD_OPEN:	// open
		case CMD_CLOSE: // close
		case CMD_ON:	// on
		case CMD_OFF:	// off
			cmd->u._zone = _vm->findZone(_tokens[_si]);
			_si++;
			break;

		case CMD_LOCATION:	// location
			cmd->u._string = (char*)malloc(strlen(_tokens[_si])+1);
			strcpy(cmd->u._string, _tokens[_si]);
			_si++;
			break;

		case CMD_CALL:	// call
			cmd->u._callable = _callableNames->lookup(_tokens[_si]) - 1;
			_si++;
			break;

		case CMD_DROP:	// drop
			cmd->u._object = _objectsNames->lookup(_tokens[_si]);
			_si++;
			break;

		case CMD_QUIT:	// quit
			break;

		case CMD_MOVE:	// move
			cmd->u._move._x = atoi(_tokens[_si]);
			_si++;
			cmd->u._move._y = atoi(_tokens[_si]);
			_si++;
			break;

		}

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

		list.push_front(cmd);	// NOTE: command lists are written backwards in scripts
		fillBuffers(script, true);

	}

}


void Parallaction::freeCommands(CommandList &list) {

	CommandList::iterator it = list.begin();

	while ( it != list.end() ) {
		delete *it;
		it++;
	}

	list.clear();

	return;
}



void Parallaction::runCommands(CommandList& list, Zone *z) {
	debugC(1, kDebugLocation, "runCommands");

	CommandList::iterator it = list.begin();
	for ( ; it != list.end(); it++) {

		Command *cmd = *it;
		CommandData *u = &cmd->u;
		uint32 v8 = _localFlags[_vm->_currentLocationIndex];

		if (_engineFlags & kEngineQuit)
			break;

		if (cmd->_flagsOn & kFlagsGlobal) {
			v8 = _commandFlags | kFlagsGlobal;
		}

		if ((cmd->_flagsOn & v8) != cmd->_flagsOn) continue;
		if ((cmd->_flagsOff & ~v8) != cmd->_flagsOff) continue;

		debugC(1, kDebugLocation, "runCommands: %s (on: %x, off: %x)", _commandsNamesRes[cmd->_id-1], cmd->_flagsOn, cmd->_flagsOff);

		switch (cmd->_id) {

		case CMD_SET:	// set
			if (cmd->u._flags & kFlagsGlobal) {
				cmd->u._flags &= ~kFlagsGlobal;
				_commandFlags |= cmd->u._flags;
			} else {
				_localFlags[_vm->_currentLocationIndex] |= cmd->u._flags;
			}
			break;

		case CMD_CLEAR: // clear
			if (cmd->u._flags & kFlagsGlobal) {
				cmd->u._flags &= ~kFlagsGlobal;
				_commandFlags &= ~cmd->u._flags;
			} else {
				_localFlags[_vm->_currentLocationIndex] &= ~cmd->u._flags;
			}
			break;

		case CMD_TOGGLE:	// toggle
			if (cmd->u._flags & kFlagsGlobal) {
				cmd->u._flags &= ~kFlagsGlobal;
				_commandFlags ^= cmd->u._flags;
			} else {
				_localFlags[_vm->_currentLocationIndex] ^= cmd->u._flags;
			}
			break;

		case CMD_START: // start
			cmd->u._zone->_flags |= kFlagsActing;
			break;

		case CMD_STOP:	// stop
			cmd->u._zone->_flags &= ~kFlagsActing;
			break;

		case CMD_SPEAK: // speak
			_activeZone = u->_zone;
			break;

		case CMD_GET:	// get
			u->_zone->_flags &= ~kFlagsFixed;
			if (!runZone(u->_zone)) {
				runCommands(u->_zone->_commands);
			}
			break;

		case CMD_DROP:	// drop
			dropItem( u->_object );
			break;

		case CMD_OPEN:	// open
			u->_zone->_flags &= ~kFlagsClosed;
			if (u->_zone->u.door->_cnv) {
				_vm->addJob(&jobToggleDoor, (void*)u->_zone, kPriority18 );
			}
			break;

		case CMD_CLOSE: // close
			u->_zone->_flags |= kFlagsClosed;
			break;

		case CMD_ON:	// on
			// WORKAROUND: the original DOS-based engine didn't check u->_zone before dereferencing
			// the pointer to get structure members, thus leading to crashes in systems with memory
			// protection.
			// As a side note, the overwritten address is the 5th entry in the DOS interrupt table
			// (print screen handler): this suggests that a system would hang when the print screen
			// key is pressed after playing Nippon Safes, provided that this code path is taken.
			if (u->_zone != NULL) {
				u->_zone->_flags &= ~kFlagsRemove;
				u->_zone->_flags |= kFlagsActive;
				if ((u->_zone->_type & 0xFFFF) == kZoneGet) {
					_vm->addJob(&jobDisplayDroppedItem, u->_zone, kPriority17 );
				}
			}
			break;

		case CMD_OFF:	// off
			u->_zone->_flags |= kFlagsRemove;
			break;

		case CMD_LOCATION:	// location
			strcpy(_vm->_location._name, u->_string);
			_engineFlags |= kEngineChangeLocation;
			break;

		case CMD_CALL:	// call
			_callables[u->_callable](z);
			break;

		case CMD_QUIT:	// quit
			_engineFlags |= kEngineQuit;
			break;

		case CMD_MOVE: {	// move
			if ((_vm->_char._ani._flags & kFlagsRemove) || (_vm->_char._ani._flags & kFlagsActive) == 0) {
				continue;
			}

			WalkNode *vC = buildWalkPath(u->_move._x, u->_move._y);

			_vm->addJob(&jobWalk, vC, kPriority19 );
			_engineFlags |= kEngineWalking;
			}
			break;

		}
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



