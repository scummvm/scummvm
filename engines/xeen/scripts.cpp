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

#include "xeen/scripts.h"
#include "xeen/dialogs_input.h"
#include "xeen/dialogs_whowill.h"
#include "xeen/dialogs_query.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

MazeEvent::MazeEvent() : _direction(DIR_ALL), _line(-1), _opcode(OP_None) {
}

void MazeEvent::synchronize(Common::Serializer &s) {
	int len = 5 + _parameters.size();
	s.syncAsByte(len);

	s.syncAsByte(_position.x);
	s.syncAsByte(_position.y);
	s.syncAsByte(_direction);
	s.syncAsByte(_line);
	s.syncAsByte(_opcode);

	len -= 5;
	if (s.isLoading())
		_parameters.resize(len);
	for (int i = 0; i < len; ++i)
		s.syncAsByte(_parameters[i]);
}

/*------------------------------------------------------------------------*/

void MazeEvents::synchronize(XeenSerializer &s) {
	MazeEvent e;

	if (s.isLoading()) {
		clear();
		while (!s.finished()) {
			e.synchronize(s);
			push_back(e);
		}
	} else {
		for (uint i = 0; i < size(); ++i)
			(*this).operator[](i).synchronize(s);
	}
}

/*------------------------------------------------------------------------*/

bool MirrorEntry::synchronize(Common::SeekableReadStream &s) {
	if (s.pos() >= s.size())
		return false;

	char buffer[28];
	s.read(buffer, 28);
	buffer[27] = '\0';

	_name = Common::String(buffer);
	_mapId = s.readByte();
	_position.x = s.readSByte();
	_position.y = s.readSByte();
	_direction = s.readSByte();
	return true;
}

/*------------------------------------------------------------------------*/

Scripts::Scripts(XeenEngine *vm) : _vm(vm) {
	Common::fill(&_charFX[0], &_charFX[MAX_ACTIVE_PARTY], 0);
	_whoWill = 0;
	_itemType = 0;
	_treasureItems = 0;
	_treasureGold = 0;
	_treasureGems = 0;
	_lineNum = 0;
	_charIndex = 0;
	_v2 = 0;
	_nEdamageType = 0;
	_animCounter = 0;
	_eventSkipped = false;
	_mirrorId = -1;
	_refreshIcons = false;
	_scriptResult = false;
	_scriptExecuted = false;
	_var50 = false;
}

int Scripts::checkEvents() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;

	_refreshIcons = false;
	_itemType = 0;
	_scriptExecuted = false;
	_var50 = false;
	_whoWill = 0;
	Mode oldMode = _vm->_mode;
	Common::fill(&_charFX[0], &_charFX[MAX_ACTIVE_PARTY], 0);
	//int items = _treasureItems;
	
	if (_treasureGold & _treasureItems) {
		// TODO
	} else {
		// TODO
	}

	do {
		_lineNum = 0;
		_scriptResult = false;
		_animCounter = 0;
//		int var4E = 0;
		_currentPos = party._mazePosition;
		_charIndex = 1;
		_v2 = 1;
		_nEdamageType = 0;
//		int var40 = -1;

		while (!_vm->shouldQuit() && _lineNum >= 0) {
			// Break out of the events if there's an attacking monster
			if (combat._attackMonsters[0] != -1) {
				_eventSkipped = true;
				break;
			}

			_eventSkipped = false;
			uint eventIndex;
			for (eventIndex = 0; eventIndex < map._events.size(); ++eventIndex) {
				MazeEvent &event = map._events[eventIndex];

				if (event._position == _currentPos && party._mazeDirection != 
						(_currentPos.x | _currentPos.y) && event._line == _lineNum) {
					if (event._direction == party._mazeDirection || event._direction == DIR_ALL) {
						_vm->_mode = MODE_9;
						_scriptExecuted = true;
						doOpcode(event);
						break;
					} else {
						_var50 = true;
					}
				}
			}
			if (eventIndex == map._events.size())
				_lineNum = -1;
		}
	} while (!_vm->shouldQuit() && _lineNum != -1);

	intf._face1State = intf._face2State = 2;
	if (_refreshIcons) {
		screen.closeWindows();
		intf.drawParty(true);
	}

	// TODO

	_animCounter = 0;
	_vm->_mode = oldMode;
	screen.closeWindows();

	if (_scriptExecuted || !intf._objNumber || _var50) {
		if (_var50 && !_scriptExecuted && intf._objNumber && !map._currentIsEvent) {
			sound.playFX(21);
		}
	} else {
		Window &w = screen._windows[38];
		w.open();
		w.writeString(NOTHING_HERE);
		w.update();

		do {
			intf.draw3d(true);
			events.updateGameCounter();
			events.wait(1, true);
		} while (!events.isKeyMousePressed());
		events.clearEvents();

		w.close();
	}

	_v2 = 1;
	Common::fill(&_charFX[0], &_charFX[6], 0);

	return _scriptResult;
}

void Scripts::giveTreasure() {
	// TODO
}

void Scripts::openGrate(int v1, int v2) {
	// TODO
}

typedef void(Scripts::*ScriptMethodPtr)(Common::Array<byte> &);

/**
 * Handles executing a given script command
 */
void Scripts::doOpcode(MazeEvent &event) {
	static const ScriptMethodPtr COMMAND_LIST[] = {
		nullptr, &Scripts::cmdDisplay1, &Scripts::cmdDoorTextSml,
		&Scripts::cmdDoorTextLrg, &Scripts::cmdSignText,
		&Scripts::cmdNPC, &Scripts::cmdPlayFX, &Scripts::cmdTeleport,
		&Scripts::cmdIf, &Scripts::cmdIf, &Scripts::cmdIf,
		&Scripts::cmdMoveObj, &Scripts::cmdTakeOrGive, &Scripts::cmdNoAction,
		&Scripts::cmdRemove, &Scripts::cmdSetChar, &Scripts::cmdSpawn,
		&Scripts::cmdDoTownEvent, &Scripts::cmdExit, &Scripts::cmdAlterMap,
		&Scripts::cmdGiveExtended, &Scripts::cmdConfirmWord, &Scripts::cmdDamage,
		&Scripts::cmdJumpRnd, &Scripts::cmdAlterEvent, &Scripts::cmdCallEvent,
		&Scripts::cmdReturn, &Scripts::cmdSetVar, &Scripts::cmdTakeOrGive,
		&Scripts::cmdTakeOrGive, &Scripts::cmdCutsceneEndClouds,
		&Scripts::cmdTeleport, &Scripts::cmdWhoWill,
		&Scripts::cmdRndDamage, &Scripts::cmdMoveWallObj, &Scripts::cmdAlterCellFlag,
		&Scripts::cmdAlterHed, &Scripts::cmdDisplayStat, &Scripts::cmdTakeOrGive,
		&Scripts::cmdSeatTextSml, &Scripts::cmdPlayEventVoc, &Scripts::cmdDisplayBottom,
		&Scripts::cmdIfMapFlag, &Scripts::cmdSelRndChar, &Scripts::cmdGiveEnchanted,
		&Scripts::cmdItemType, &Scripts::cmdMakeNothingHere, &Scripts::cmdCheckProtection,
		&Scripts::cmdChooseNumeric, &Scripts::cmdDisplayBottomTwoLines,
		&Scripts::cmdDisplayLarge, &Scripts::cmdExchObj, &Scripts::cmdFallToMap,
		&Scripts::cmdDisplayMain, &Scripts::cmdGoto, &Scripts::cmdConfirmWord,
		&Scripts::cmdGotoRandom, &Scripts::cmdCutsceneEndDarkside,
		&Scripts::cmdCutsceneEdWorld, &Scripts::cmdFlipWorld, &Scripts::cmdPlayCD
	};

	_event = &event;
	(this->*COMMAND_LIST[event._opcode])(event._parameters);
}

/**
 * Display a msesage on-screen
 */
void Scripts::cmdDisplay1(Common::Array<byte> &params) {
	Screen &screen = *_vm->_screen;
	Common::String paramText = _vm->_map->_events._text[_event->_parameters[0]];
	Common::String msg = Common::String::format("\r\x03""c%s", paramText.c_str());

	screen._windows[12].close();
	if (screen._windows[38]._enabled)
		screen._windows[38].open();
	screen._windows[38].writeString(msg);
	screen._windows[38].update();

	cmdNoAction(params);
}

/**
 * Displays a door text message using the small font
 */
void Scripts::cmdDoorTextSml(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[_event->_parameters[0]];
	intf._screenText = Common::String::format("\x02\f""08\x03""c\t116\v025%s\x03""l\fd""\x01", 
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	cmdNoAction(params);
}

/**
 * Displays a door text message using the large font
 */
void Scripts::cmdDoorTextLrg(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[_event->_parameters[0]];
	intf._screenText = Common::String::format("\f04\x03""c\t116\v030%s\x03""l\fd",
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	cmdNoAction(params);
}

/**
 * Show a sign text on-screen
 */
void Scripts::cmdSignText(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[_event->_parameters[0]];
	intf._screenText = Common::String::format("\f08\x03""c\t120\v088%s\x03""l\fd",
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	cmdNoAction(params);
}

void Scripts::cmdNPC(Common::Array<byte> &params) {
	warning("TODO: cmdNPC");
}

/**
 * Play a sound FX
 */
void Scripts::cmdPlayFX(Common::Array<byte> &params) {
	_vm->_sound->playFX(params[0]);

	cmdNoAction(params);
}

void Scripts::cmdTeleport(Common::Array<byte> &params) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;

	screen.closeWindows();

	int mapId;
	Common::Point pt;

	if (params[0]) {
		mapId = params[0];
		pt = Common::Point((int8)params[1], (int8)params[2]);
	} else {
		assert(_mirrorId > 0);
		MirrorEntry &me = _mirror[_mirrorId - 1];
		mapId = me._mapId;
		pt = me._position;
		if (me._direction != -1)
			party._mazeDirection = (Direction)me._direction;

		if (pt.x == 0 && pt.y == 0)
			pt.x = 999;

		sound.playFX(51);
	}

	party._stepped = true;
	if (mapId != party._mazeId) {
		int spriteId = (intf._objNumber == 0) ? -1 :
			map._mobData._objects[intf._objNumber - 1]._spriteId;

		switch (spriteId) {
		case 47:
			sound.playFX(45);
			break;
		case 48:
			sound.playFX(44);
			break;
		default:
			break;
		}

		// Load the new map
		map.load(mapId);
	}

	if (pt.x == 999) {
		party._mazePosition = map.mazeData()._runPosition;
	} else {
		party._mazePosition = pt;
	}

	events.clearEvents();

	if (_event->_opcode == OP_TeleportAndContinue) {
		intf.draw3d(true);
		_lineNum = 0;
	} else {
		cmdExit(params);
	}
}

/**
 * Do a conditional check
 */
void Scripts::cmdIf(Common::Array<byte> &params) {
	Party &party = *_vm->_party;
	uint32 mask;
	int newLineNum;

	switch (params[0]) {
	case 16:
	case 34:
	case 100:
		mask = (params[4] << 24) | (params[3] << 16) | (params[2] << 8) | params[1];
		newLineNum = params[5];
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		mask = (params[2] << 8) | params[1];
		newLineNum = params[3];
		break;
	default:
		mask = params[1];
		newLineNum = params[2];
		break;
	}

	bool result;
	if ((_charIndex != 0 && _charIndex != 8) || params[0] == 44) {
		result = ifProc(params[0], mask, _event->_opcode - 8, _charIndex - 1);
	} else {
		result = false;
		for (int idx = 0; idx < party._partyCount && !result; ++idx) {
			if (_charIndex == 0 || (_charIndex == 8 && idx != _v2)) {
				result = ifProc(params[0], mask, _event->_opcode - 8, idx);
			}
		}
	}

	if (result)
		_lineNum = newLineNum - 1;

	cmdNoAction(params);
}

/**
 * Moves the position of an object
 */
void Scripts::cmdMoveObj(Common::Array<byte> &params) { 
	MazeObject &mazeObj = _vm->_map->_mobData._objects[params[0]];

	if (mazeObj._position.x == params[1] && mazeObj._position.y == params[2]) {
		// Already in position, so simply flip it
		mazeObj._flipped = !mazeObj._flipped;
	} else {
		mazeObj._position.x = params[1];
		mazeObj._position.y = params[2];
	}
}

void Scripts::cmdTakeOrGive(Common::Array<byte> &params) { error("TODO"); }

/**
 * Move to the next line of the script
 */
void Scripts::cmdNoAction(Common::Array<byte> &params) {
	// Move to next line
	_lineNum = _vm->_party->_partyDead ? -1 : _lineNum + 1;
}

void Scripts::cmdRemove(Common::Array<byte> &params) { 
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;

	if (intf._objNumber) {
		// Give the active object a completely way out of bounds position
		MazeObject &obj = map._mobData._objects[intf._objNumber - 1];
		obj._position = Common::Point(128, 128);
	}

	cmdMakeNothingHere(params);
}

/**
 * Set the currently active character for other script operations
 */
void Scripts::cmdSetChar(Common::Array<byte> &params) { 
	if (params[0] != 7) {
		_charIndex = WhoWill::show(_vm, 22, 3, false);
		if (_charIndex == 0) {
			cmdExit(params);
			return;
		}
	} else {
		_charIndex = _vm->getRandomNumber(1, _vm->_party->_partyCount);
	}

	_v2 = 1;
	cmdNoAction(params);
}

/**
 * Spawn a monster
 */
void Scripts::cmdSpawn(Common::Array<byte> &params) {
	Map &map = *_vm->_map;
	if (params[0] >= map._mobData._monsters.size())
		map._mobData._monsters.resize(params[0] + 1);

	MazeMonster &monster = _vm->_map->_mobData._monsters[params[0]];
	MonsterStruct &monsterData = _vm->_map->_monsterData[monster._spriteId];
	monster._position.x = params[1];
	monster._position.y = params[2];
	monster._frame = _vm->getRandomNumber(7);
	monster._field7 = 0;
	monster._isAttacking = params[1] != 0;
	monster._hp = monsterData._hp;

	cmdNoAction(params);
}

void Scripts::cmdDoTownEvent(Common::Array<byte> &params) {
	_scriptResult = _vm->_town->townAction(params[0]);
	_vm->_party->_stepped = true;
	_refreshIcons = true;

	cmdExit(params);
}

/**
 * Stop executing the script
 */
void Scripts::cmdExit(Common::Array<byte> &params) { 
	_lineNum = -1;
}

/**
 * Changes the value for the wall on a given cell
 */
void Scripts::cmdAlterMap(Common::Array<byte> &params) { 
	Map &map = *_vm->_map;

	if (params[2] == DIR_ALL) {
		for (int dir = DIR_NORTH; dir <= DIR_WEST; ++dir)
			map.setWall(Common::Point(params[0], params[1]), (Direction)dir, params[3]);
	} else {
		map.setWall(Common::Point(params[0], params[1]), (Direction)params[2], params[3]);
	}

	cmdNoAction(params);
}

void Scripts::cmdGiveExtended(Common::Array<byte> &params) { 
	Party &party = *_vm->_party;
	uint32 mask;
	int newLineNum;
	bool result;

	switch (params[0]) {
	case 16:
	case 34:
	case 100:
		mask = (params[4] << 24) | params[3] | (params[2] << 8) | (params[1] << 16);
		newLineNum = params[5];
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		mask = (params[2] << 8) | params[1];
		newLineNum = params[3];
		break;
	default:
		mask = params[1];
		newLineNum = params[2];
		break;
	}

	if ((_charIndex != 0 && _charIndex != 8) || params[0] == 44) {
		result = ifProc(params[0], mask, _event->_opcode - OP_If1, _charIndex - 1);
	} else {
		result = false;
		for (int idx = 0; idx < party._partyCount && !result; ++idx) {
			if (_charIndex == 0 || (_charIndex == 8 && _v2 != idx)) {
				result = ifProc(params[0], mask, _event->_opcode - OP_If1, idx);
			}
		}
	}

	if (result)
		_lineNum = newLineNum - 1;

	cmdNoAction(params);
}

void Scripts::cmdConfirmWord(Common::Array<byte> &params) { 
	Map &map = *_vm->_map;
	Common::String msg1 = params[2] ? map._events._text[params[2]] :
		_vm->_interface->_interfaceText;
	Common::String msg2;

	if (_event->_opcode == OP_ConfirmWord_2) {
		msg2 = map._events._text[params[3]];
	} else if (params[3]) {
		msg2 = "";
	} else {
		msg2 = WHATS_THE_PASSWORD;
	}

	int result = StringInput::show(_vm, params[0], msg1, msg2,_event->_opcode);
	if (result) {
		if (result == 33 && _vm->_files->_isDarkCc) {
			doEndGame2();
		} else if (result == 34 && _vm->_files->_isDarkCc) {
			doWorldEnd();
		} else if (result == 35 && _vm->_files->_isDarkCc &&
				_vm->getGameID() == GType_WorldOfXeen) {
			doEndGame();
		} else if (result == 40 && !_vm->_files->_isDarkCc) {
			doEndGame();
		} else if (result == 60 && !_vm->_files->_isDarkCc) {
			doEndGame2();
		}
		else if (result == 61 && !_vm->_files->_isDarkCc) {
			doWorldEnd();
		} else {		
			if (result == 59 && !_vm->_files->_isDarkCc) {
				for (int idx = 0; idx < TOTAL_ITEMS; ++idx) {
					XeenItem &item = _vm->_treasure._weapons[idx];
					if (!item._id) {
						item._id = 34;
						item._material = 0;
						item._bonusFlags = 0;
						_vm->_treasure._hasItems = true;
						
						cmdExit(params);
						return;
					}
				}
			}

			_lineNum = result == -1 ? params[3] : params[1];
			
			return;
		}
	}

	cmdNoAction(params);
}

void Scripts::cmdDamage(Common::Array<byte> &params) { error("TODO"); }

/**
 * Jump if a random number matches a given value
 */
void Scripts::cmdJumpRnd(Common::Array<byte> &params) {
	int v = _vm->getRandomNumber(1, params[0]);
	if (v == params[1])
		_lineNum = params[2] - 1;

	cmdNoAction(params);
}

/**
 * Alter an existing event
 */
void Scripts::cmdAlterEvent(Common::Array<byte> &params) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	for (uint idx = 0; idx < map._events.size(); ++idx) {
		MazeEvent &evt = map._events[idx];
		if (evt._position == party._mazePosition &&
				(evt._direction == DIR_ALL || evt._direction == party._mazeDirection) &&
				evt._line == params[0]) {
			evt._opcode = (Opcode)params[1];
		}
	}

	cmdNoAction(params);
}

/**
 * Stores the current location and line for later resuming, and set up to execute
 * a script at a given location
 */
void Scripts::cmdCallEvent(Common::Array<byte> &params) { 
	_stack.push(StackEntry(_currentPos, _lineNum));
	_currentPos = Common::Point(params[0], params[1]);
	_lineNum = params[2] - 1;

	cmdNoAction(params);
}

/**
 * Return from executing a script to the script location that previously 
 * called the script
 */
void Scripts::cmdReturn(Common::Array<byte> &params) {
	StackEntry &se = _stack.top();
	_currentPos = se;
	_lineNum = se.line;

	cmdNoAction(params);
}

void Scripts::cmdSetVar(Common::Array<byte> &params) { 
	Party &party = *_vm->_party;
	uint val;
	_refreshIcons = true;

	switch (params[0]) {
	case 25:
	case 35:
	case 101:
	case 106:
		val = (params[2] << 8) | params[1];
		break;
	case 16:
	case 34:
	case 100:
		val = (params[4] << 24) | (params[3] << 16) | (params[2] << 8) | params[3];
		break;
	default:
		val = params[1];
		break;
	}

	if (_charIndex != 0 && _charIndex != 8) {
		party._activeParty[_charIndex - 1].setValue(params[0], val);
	} else {
		// Set value for entire party
		for (int idx = 0; idx < party._partyCount; ++idx) {
			if (_charIndex == 0 || (_charIndex == 8 && _v2 != idx)) {
				party._activeParty[idx].setValue(params[0], val);
			}
		}
	}

	cmdNoAction(params);
}

void Scripts::cmdCutsceneEndClouds(Common::Array<byte> &params) { error("TODO"); }

void Scripts::cmdWhoWill(Common::Array<byte> &params) { 
	_charIndex = WhoWill::show(_vm, params[0], params[1], true);

	if (_charIndex == 0)
		cmdExit(params);
	else
		cmdNoAction(params);
}

void Scripts::cmdRndDamage(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdMoveWallObj(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdAlterCellFlag(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdAlterHed(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdDisplayStat(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdSeatTextSml(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdPlayEventVoc(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdDisplayBottom(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdIfMapFlag(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdSelRndChar(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdGiveEnchanted(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdItemType(Common::Array<byte> &params) { error("TODO"); }

/**
 * Disable all the scripts at the party's current position
 */
void Scripts::cmdMakeNothingHere(Common::Array<byte> &params) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	// Scan through the event list and mark the opcodes for all the lines of any scripts
	// on the party's current cell as having no operation, effectively disabling them
	for (uint idx = 0; idx < map._events.size(); ++idx) {
		MazeEvent &evt = map._events[idx];
		if (evt._position == party._mazePosition)
			evt._opcode = OP_None;
	}

	cmdExit(params);
}

void Scripts::cmdCheckProtection(Common::Array<byte> &params) {
	if (copyProtectionCheck())
		cmdNoAction(params);
	else
		cmdExit(params);
}

void Scripts::cmdChooseNumeric(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdDisplayBottomTwoLines(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdDisplayLarge(Common::Array<byte> &params) { error("TODO"); }

/**
 * Exchange the positions of two objects in the maze
 */
void Scripts::cmdExchObj(Common::Array<byte> &params) {
	MazeObject &obj1 = _vm->_map->_mobData._objects[params[0]];
	MazeObject &obj2 = _vm->_map->_mobData._objects[params[1]];

	Common::Point pt = obj1._position;
	obj1._position = obj2._position;
	obj2._position = pt;

	cmdNoAction(params);
}

void Scripts::cmdFallToMap(Common::Array<byte> &params) {
	Party &party = *_vm->_party;
	party._fallMaze = params[0];
	party._fallPosition = Common::Point(params[1], params[2]);
	party._fallDamage = params[3];

	_lineNum = -1;
}

void Scripts::cmdDisplayMain(Common::Array<byte> &params) { 
	error("TODO"); 
}

/**
 * Jumps to a given line number if the surface at relative cell position 1 matches
 * a specified surface.
 * @remarks		This opcode is apparently never actually used
 */
void Scripts::cmdGoto(Common::Array<byte> &params) { 
	Map &map = *_vm->_map;
	map.getCell(1);
	if (params[0] == map._currentSurfaceId)
		_lineNum = params[1] - 1;

	cmdNoAction(params);
}

/**
 * Pick a random value from the parameter list and jump to that line number
 */
void Scripts::cmdGotoRandom(Common::Array<byte> &params) { 
	_lineNum = params[_vm->getRandomNumber(1, params[0])] - 1;
	cmdNoAction(params);
}

void Scripts::cmdCutsceneEndDarkside(Common::Array<byte> &params) { 
	Party &party = *_vm->_party;
	_vm->_saves->_wonDarkSide = true;
	party._questItems[53] = 1;
	party._darkSideEnd = true;
	party._mazeId = 29;
	party._mazeDirection = DIR_NORTH;
	party._mazePosition = Common::Point(25, 21);

	doEndGame2();
}

void Scripts::cmdCutsceneEdWorld(Common::Array<byte> &params) { 
	_vm->_saves->_wonWorld = true;
	_vm->_party->_worldEnd = true;
	doWorldEnd();
}

void Scripts::cmdFlipWorld(Common::Array<byte> &params) { 
	_vm->_map->_loadDarkSide = params[0] != 0;
}

void Scripts::cmdPlayCD(Common::Array<byte> &params) { error("TODO"); }

void Scripts::doEndGame() {
	doEnding("ENDGAME", 0);
}

void Scripts::doEndGame2() {
	Party &party = *_vm->_party;
	int v2 = 0;

	for (int idx = 0; idx < party._partyCount; ++idx) {
		Character &player = party._activeParty[idx];
		if (player.hasAward(77)) {
			v2 = 2;
			break;
		}
		else if (player.hasAward(76)) {
			v2 = 1;
			break;
		}
	}

	doEnding("ENDGAME2", v2);
}

void Scripts::doWorldEnd() {

}

void Scripts::doEnding(const Common::String &endStr, int v2) {
	_vm->_saves->saveChars();
	
	warning("TODO: doEnding");
}

/**
 * This monstrosity handles doing the various types of If checks on various data
 */
bool Scripts::ifProc(int action, uint32 mask, int mode, int charIndex) {
	Party &party = *_vm->_party;
	Character &ps = party._activeParty[charIndex];
	uint v = 0;

	switch (action) {
	case 3:
		// Player sex
		v = (uint)ps._sex;
		break;
	case 4:
		// Player race
		v = (uint)ps._race;
		break;
	case 5:
		// Player class
		v = (uint)ps._class;
		break;
	case 8:
		// Current health points
		v = (uint)ps._currentHp;
		break;
	case 9:
		// Current spell points
		v = (uint)ps._currentSp;
		break;
	case 10:
		// Get armor class
		v = (uint)ps.getArmorClass(false);
		break;
	case 11:
		// Level bonus (extra beyond base)
		v = ps._level._temporary;
		break;
	case 12:
		// Current age, including unnatural aging
		v = ps.getAge(false);
		break;
	case 13:
		assert(mask < 18);
		if (ps._skills[mask])
			v = mask;
		break;
	case 15:
		// Award
		assert(mask < 128);
		if (ps.hasAward(mask))
			v = mask;
		break;
	case 16:
		// Experience
		v = ps._experience;
		break;
	case 17:
		// Party poison resistence
		v = party._poisonResistence;
		break;
	case 18:
		// Condition
		assert(mask < 16);
		if (!ps._conditions[mask] && !(mask & 0x10))
			v = mask;
		break;
	case 19: {
		// Can player cast a given spell

		// Get the type of character
		int category;
		switch (ps._class) {
		case CLASS_KNIGHT:
		case CLASS_ARCHER:
			category = 0;
			break;
		case CLASS_PALADIN:
		case CLASS_CLERIC:
			category = 1;
			break;
		case CLASS_BARBARIAN:
		case CLASS_DRUID:
			category = 2;
			break;
		default:
			category = 0;
			break;
		}

		// Check if the character class can cast the particular spell
		for (int idx = 0; idx < 39; ++idx) {
			if (SPELLS_ALLOWED[mode][idx] == mask) {
				// Can cast it. Check if the player has it in their spellbook
				if (ps._spells[idx])
					v = mask;
				break;
			}
		}
		break;
	}
	case 20:
		if (_vm->_files->_isDarkCc)
			mask += 0x100;
		assert(mask < 0x200);
		v = party._gameFlags[mask] ? mask : 0xffffffff;
		break;
	case 21:
		// Scans inventories for given item number
		v = 0xFFFFFFFF;
		if (mask < 82) {
			for (int idx = 0; idx < 9; ++idx) {
				if (mask == 35) {
					if (ps._weapons[idx]._id == mask) {
						v = mask;
						break;
					}
				} else if (mask < 49) {
					if (ps._armor[idx]._id == (mask - 35)) {
						v = mask;
						break;
					}
				} else if (mask < 60) {
					if (ps._accessories[idx]._id == (mask - 49)) {
						v = mask;
						break;
					}
				} else {
					if (ps._misc[idx]._id == (mask - 60)) {
						v = mask;
						break;
					}
				}
			}
		} else {
			int baseFlag = 8 * (6 + mask);
			for (int idx = 0; idx < 8; ++idx) {
				if (party._gameFlags[baseFlag + idx]) {
					v = mask;
					break;
				}
			}
		}
		break;
	case 25:
		// Returns number of minutes elapsed in the day (0-1440)
		v = party._minutes;
		break;
	case 34:
		// Current party gold
		v = party._gold;
		break;
	case 35:
		// Current party gems
		v = party._gems;
		break;
	case 37:
		// Might bonus (extra beond base)
		v = ps._might._temporary;
		break;
	case 38:
		// Intellect bonus (extra beyond base)
		v = ps._intellect._temporary;
		break;
	case 39:
		// Personality bonus (extra beyond base)
		v = ps._personality._temporary;
		break;
	case 40:
		// Endurance bonus (extra beyond base)
		v = ps._endurance._temporary;
		break;
	case 41:
		// Speed bonus (extra beyond base)
		v = ps._speed._temporary;
		break;
	case 42:
		// Accuracy bonus (extra beyond base)
		v = ps._accuracy._temporary;
		break;
	case 43:
		// Luck bonus (extra beyond base)
		v = ps._luck._temporary;
		break;
	case 44:
		v = YesNo::show(_vm, mask);
		v = (!v && !mask) ? 2 : mask;
		break;
	case 45:
		// Might base (before bonus)
		v = ps._might._permanent;
		break;
	case 46:
		// Intellect base (before bonus)
		v = ps._intellect._permanent;
		break;
	case 47:
		// Personality base (before bonus)
		v = ps._personality._permanent;
		break;
	case 48:
		// Endurance base (before bonus)
		v = ps._endurance._permanent;
		break;
	case 49:
		// Speed base (before bonus)
		v = ps._speed._permanent;
		break;
	case 50:
		// Accuracy base (before bonus)
		v = ps._accuracy._permanent;
		break;
	case 51:
		// Luck base (before bonus)
		v = ps._luck._permanent;
		break;
	case 52:
		// Fire resistence (before bonus)
		v = ps._fireResistence._permanent;
		break;
	case 53:
		// Elecricity resistence (before bonus)
		v = ps._electricityResistence._permanent;
		break;
	case 54:
		// Cold resistence (before bonus)
		v = ps._coldResistence._permanent;
		break;
	case 55:
		// Poison resistence (before bonus)
		v = ps._poisonResistence._permanent;
		break;
	case 56:
		// Energy reistence (before bonus)
		v = ps._energyResistence._permanent;
		break;
	case 57:
		// Energy resistence (before bonus)
		v = ps._magicResistence._permanent;
		break;
	case 58:
		// Fire resistence (extra beyond base)
		v = ps._fireResistence._temporary;
		break;
	case 59:
		// Electricity resistence (extra beyond base)
		v = ps._electricityResistence._temporary;
		break;
	case 60:
		// Cold resistence (extra beyond base)
		v = ps._coldResistence._temporary;
		break;
	case 61:
		// Poison resistence (extra beyod base)
		v = ps._poisonResistence._temporary;
		break;
	case 62:
		// Energy resistence (extra beyond base)
		v = ps._energyResistence._temporary;
		break;
	case 63:
		// Magic resistence (extra beyond base)
		v = ps._magicResistence._temporary;
		break;
	case 64:
		// Level (before bonus)
		v = ps._level._permanent;
		break;
	case 65:
		// Total party food
		v = party._food;
		break;
	case 69:
		// Test for Levitate being active
		v = party._levitateActive ? 1 : 0;
		break;
	case 70:
		// Amount of light
		v = party._lightCount;
		break;
	case 71:
		// Party magical fire resistence
		v = party._fireResistence;
		break;
	case 72:
		// Party magical electricity resistence
		v = party._electricityResistence;
		break;
	case 73:
		// Party magical cold resistence
		v = party._coldResistence;
		break;
	case 76:
		// Day of the year (100 per year)
		v = party._day;
		break;
	case 77:
		// Armor class (extra beyond base)
		v = ps._ACTemp;
		break;
	case 78:
		// Test whether current Hp is equal to or exceeds the max HP
		v = ps._currentHp >= ps.getMaxHP() ? 1 : 0;
		break;
	case 79:
		// Test for Wizard Eye being active
		v = party._wizardEyeActive ? 1 : 0;
		break;
	case 81:
		// Test whether current Sp is equal to or exceeds the max SP
		v = ps._currentSp >= ps.getMaxSP() ? 1 : 0;
		break;
	case 84:
		// Current facing direction
		v = (uint)party._mazeDirection;
		break;
	case 85:
		// Current game year since start
		v = party._year;
		break;
	case 86:
	case 87:
	case 88:
	case 89:
	case 90:
	case 91:
	case 92:
		// Get a player stat
		v = ps.getStat((Attribute)(action - 86), 0);
		break;
	case 93:
		// Current day of the week (10 days per week)
		v = party._day / 10;
		break;
	case 94:
		// Test whether Walk on Water is currently active
		v = party._walkOnWaterActive ? 1 : 0;
		break;
	case 99:
		// Party skills check
		v = party.checkSkill((Skill)mask) ? mask : 0xffffffff;
		break;
	case 102:
		// Thievery skill
		v = ps.getThievery();
		break;
	case 103:
		// Get value of world flag
		v = party._worldFlags[mask] ? mask : 0xffffffff;
		break;
	case 104:
		// Get value of quest flag
		v = party._quests[mask + (_vm->_files->_isDarkCc ? 30 : 0)] ?
			mask : 0xffffffff;
		break;
	case 105:
		// Test number of Megacredits in party. Only used by King's Engineer in Castle Burlock
		v = party._questItems[26];
		break;
	case 107:
		// Get value of character flag
		error("Unused");
		break;
	default:
		break;
	}

	switch (mode) {
	case 0:
		return mask >= v;
	case 1:
		return mask == v;
	case 2:
		return mask <= v;
	default:
		return false;
	}
}

bool Scripts::copyProtectionCheck() {
	// Currentl not implemented
	return true;
}

} // End of namespace Xeen
