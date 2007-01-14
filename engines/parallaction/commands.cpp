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

extern char *_callableNames[];


Command *parseCommands(ArchivedFile *file) {
//	printf("parseCommands()\n");

	Node root;
	memset(&root, 0, sizeof(root));

	parseFillBuffers();

	while (scumm_stricmp(_tokens[0], "ENDCOMMANDS") && scumm_stricmp(_tokens[0], "ENDZONE")) {
//		printf("token[0] = %s\n", _tokens[0]);

		Command *cmd = (Command*)memAlloc(sizeof(Command));
		memset(cmd, 0, sizeof(Command));

		cmd->_id = _vm->searchTable(_tokens[0], commands_names);
		uint16 _si = 1;

//		printf("cmd id = %i\n", cmd->_id);

		switch (cmd->_id) {
		case CMD_SET:	// set
		case CMD_CLEAR: // clear
		case CMD_TOGGLE:	// toggle
			if (_vm->searchTable(_tokens[1], _globalTable) == -1) {
				do {
					char _al = _vm->searchTable(_tokens[_si], _localFlagNames);
					_si++;
					cmd->u._flags |= 1 << (_al - 1);
				} while (!scumm_stricmp(_tokens[_si++], "|"));
				_si--;
			} else {
				cmd->u._flags |= kFlagsGlobal;
				do {
					char _al = _vm->searchTable(_tokens[1], _globalTable);
					_si++;
					cmd->u._flags |= 1 << (_al - 1);
				} while (!scumm_stricmp(_tokens[_si++], "|"));
				_si--;
			}
			break;

		case CMD_START: // start
		case CMD_STOP:	// stop
			cmd->u._animation = findAnimation(_tokens[_si]);
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
			cmd->u._zone = findZone(_tokens[_si]);
			_si++;
			break;

		case CMD_LOCATION:	// location
			cmd->u._string = (char*)memAlloc(strlen(_tokens[_si])+1);
			strcpy(cmd->u._string, _tokens[_si]);
			_si++;
			break;

		case CMD_CALL:	// call
			cmd->u._callable = _vm->searchTable(_tokens[_si], _callableNames) - 1;
			_si++;
			break;

		case CMD_DROP:	// drop
			cmd->u._object = _vm->searchTable(_tokens[_si], _objectsNames);
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
					byte _al = _vm->searchTable(&_tokens[_si][2], _localFlagNames);
					cmd->_flagsOff |= 1 << (_al - 1);
				} else {
					byte _al = _vm->searchTable(_tokens[_si], _localFlagNames);
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
					byte _al = _vm->searchTable(&_tokens[_si][2], _globalTable);
					cmd->_flagsOff |= 1 << (_al - 1);
				} else {
					byte _al = _vm->searchTable(_tokens[_si], _localFlagNames);
					cmd->_flagsOn |= 1 << (_al - 1);
				}

				_si++;

			} while (!scumm_stricmp(_tokens[_si++], "|"));

		}

		addNode(&root, &cmd->_node);
		parseFillBuffers();

	}

	return (Command*)root._next;
}


void freeCommands(Command *list) {

	Command *cmd = list;

	while (cmd) {
		Command *v4 = (Command*)cmd->_node._next;

		if (cmd->_id == 6) memFree(cmd->u._zone);	// open
		memFree(cmd);

		cmd = v4;
	}

	return;
}



void runCommands(Command *list, Zone *z) {
//	printf("runCommands()\n");

	Command *cmd = list;
	for ( ; cmd; cmd = (Command*)cmd->_node._next) {
		CommandData *u = &cmd->u;
		uint32 v8 = _localFlags[_vm->_currentLocationIndex];

		if (_engineFlags & kEngineQuit)
			break;

		if (cmd->_flagsOn & kFlagsGlobal) {
			v8 = _commandFlags | kFlagsGlobal;
		}

		if ((cmd->_flagsOn & v8) != cmd->_flagsOn) continue;
		if ((cmd->_flagsOff & ~v8) != cmd->_flagsOff) continue;

//		printf("localflags: %x\n", v8);
//		printf("%s (on flags: %x, off flags: %x)\n", commands_names[cmd->_id-1], cmd->_flagsOn, cmd->_flagsOff);

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
			if (u->_zone->u.door->_cnv._count != 0) {
				addJob(&jobToggleDoor, (void*)u->_zone, JOBPRIORITY_TOGGLEDOOR );
			}
			break;

		case CMD_CLOSE: // close
			u->_zone->_flags |= kFlagsClosed;
			break;

		case CMD_ON:	// on
			u->_zone->_flags &= ~kFlagsRemove;
			u->_zone->_flags |= kFlagsActive;
			if ((u->_zone->_type & 0xFFFF) == kZoneGet) {
				addJob(&jobDisplayDroppedItem, u->_zone, JOBPRIORITY_ADDREMOVEITEMS );
			}
			break;

		case CMD_OFF:	// off
			u->_zone->_flags |= kFlagsRemove;
			break;

		case CMD_LOCATION:	// location
			strcpy(_location, u->_string);
			_engineFlags |= kEngineChangeLocation;
			break;

		case CMD_CALL:	// call
			_callables[u->_callable](z);
			break;

		case CMD_QUIT:	// quit
			_engineFlags |= kEngineQuit;
			break;

		case CMD_MOVE: {	// move
			if ((_yourself._zone._flags & kFlagsRemove) || (_yourself._zone._flags & kFlagsActive) == 0) {
				continue;
			}

			WalkNode *vC = buildWalkPath(u->_move._x, u->_move._y);

			addJob(&jobWalk, vC, JOBPRIORITY_WALK );
			_engineFlags |= kEngineWalking;
			}
			break;

		}
	}

	return;

}

} // namespace Parallaction

