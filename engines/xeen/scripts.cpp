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
#include "xeen/dialogs_string_input.h"
#include "xeen/dialogs_whowill.h"
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
}

void Scripts::checkEvents() {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

//	int var18 = 0;
	_itemType = 0;
	_var4F = 0;
	bool var50 = false;
	_whoWill = 0;
	Mode oldMode = _vm->_mode;
	Common::fill(&_charFX[0], &_charFX[MAX_ACTIVE_PARTY], 0);
	int items = _treasureItems;
	
	if (_treasureGold & _treasureItems) {
		// TODO
	} else {
		// TODO
	}

	do {
		_lineNum = 0;
//		int varA = 0;
		_animCounter = 0;
//		int var4E = 0;
		_currentPos = party._mazePosition;
		_charIndex = 1;
		_v2 = 1;
		_nEdamageType = 0;
//		int var40 = -1;

		while (_lineNum >= 0) {
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
						_paramText = event._parameters.size() == 0 ? "" :
							map._events._text[event._parameters[0]];
						doOpcode(event);
						break;
					} else {
						var50 = true;
					}
				}
			}
			if (eventIndex == map._events.size())
				break;
		}
	} while (0);

	// TODO
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
		&Scripts::cmdJumpRnd, &Scripts::cmdAfterEvent, &Scripts::cmdCallEvent,
		&Scripts::cmdReturn, &Scripts::cmdSetVar, &Scripts::cmdTakeOrGive,
		&Scripts::cmdTakeOrGive, &Scripts::cmdCutsceneEndClouds,
		&Scripts::cmdTeleport, &Scripts::cmdWhoWill,
		&Scripts::cmdRndDamage, &Scripts::cmdMoveWallObj, &Scripts::cmdAlterCellFlag,
		&Scripts::cmdAlterHed, &Scripts::cmdDisplayStat, &Scripts::cmdTakeOrGive,
		&Scripts::cmdSeatTextSml, &Scripts::cmdPlayEventVoc, &Scripts::cmdDisplayBottom,
		&Scripts::cmdIfMapFlag, &Scripts::cmdSelRndChar, &Scripts::cmdGiveEnchanted,
		&Scripts::cmdItemType, &Scripts::cmdMakeNothingHere, &Scripts::cmdNoAction2,
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
	Common::String msg = Common::String::format("\r\x03""c%s", _paramText.c_str());

	screen._windows[12].close();
	if (screen._windows[38]._enabled)
		screen._windows[38].open();
	screen._windows[38].writeString(msg);
	screen._windows[38].update();

	_var4F = true;
	cmdNoAction(params);
}

/**
 * Displays a door text message using the small font
 */
void Scripts::cmdDoorTextSml(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;
	intf._screenText = Common::String::format("\x02\f""08\x03""c\t116\v025%s\x03""l\fd""\x01", 
		_paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	_var4F = true;
	cmdNoAction(params);
}

/**
 * Displays a door text message using the large font
 */
void Scripts::cmdDoorTextLrg(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;
	intf._screenText = Common::String::format("\f04\x03""c\t116\v030%s\x03""l\fd",
		_paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	_var4F = true;
	cmdNoAction(params);
}

/**
 * Show a sign text on-screen
 */
void Scripts::cmdSignText(Common::Array<byte> &params) {
	Interface &intf = *_vm->_interface;
	intf._screenText = Common::String::format("\f08\x03""c\t120\v088%s\x03""l\fd",
		_paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	_var4F = true;
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

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdTeleport(Common::Array<byte> &params) {
	error("TODO");
}

/**
 * Do a conditional check
 */
void Scripts::cmdIf(Common::Array<byte> &params) {
	switch (params[0]) {
	case 16:
	case 34:
	case 100:
		// TODO
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		// TODO
		break;
	default:
		break;
	}
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

void Scripts::cmdRemove(Common::Array<byte> &params) { error("TODO"); }

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
	MazeMonster &monster = _vm->_map->_mobData._monsters[params[0]];
	MonsterStruct &monsterData = _vm->_map->_monsterData[monster._spriteId];
	monster._position.x = params[1];
	monster._position.y = params[2];
	monster._frame = _vm->getRandomNumber(7);
	monster._field7 = 0;
	monster._isAttacking = params[1] != 0;
	monster._hp = monsterData._hp;

	_var4F = 1;
	cmdNoAction(params);
}

void Scripts::cmdDoTownEvent(Common::Array<byte> &params) { error("TODO"); }

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

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdGiveExtended(Common::Array<byte> &params) { 
	switch (params[0]) {
	case 16:
	case 34:
	case 100:
		// TODO
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		// TODO
		break;
	default:
		break;
	}
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
					if (!item._name) {
						item._name = 34;
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

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdDamage(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdJumpRnd(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdAfterEvent(Common::Array<byte> &params) { error("TODO"); }

/**
 * Stores the current location and line for later resuming, and set up to execute
 * a script at a given location
 */
void Scripts::cmdCallEvent(Common::Array<byte> &params) { 
	_stack.push(StackEntry(_currentPos, _lineNum));
	_currentPos = Common::Point(params[0], params[1]);
	_lineNum = params[2] - 1;

	_var4F = true;
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

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdSetVar(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdCutsceneEndClouds(Common::Array<byte> &params) { error("TODO"); }

void Scripts::cmdWhoWill(Common::Array<byte> &params) { 
	_charIndex = WhoWill::show(_vm, params[0], params[1], true);

	_var4F = true;
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
void Scripts::cmdMakeNothingHere(Common::Array<byte> &params) { error("TODO"); }
void Scripts::cmdNoAction2(Common::Array<byte> &params) { error("TODO"); }
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

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdFallToMap(Common::Array<byte> &params) {
	Party &party = *_vm->_party;
	party._fallMaze = params[0];
	party._fallPosition = Common::Point(params[1], params[2]);
	party._fallDamage = params[3];

	_var4F = true;
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

	_var4F = true;
	cmdNoAction(params);
}

/**
 * Pick a random value from the parameter list and jump to that line number
 */
void Scripts::cmdGotoRandom(Common::Array<byte> &params) { 
	_lineNum = params[_vm->getRandomNumber(1, params[0])] - 1;
	_var4F = true;
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
		PlayerStruct &player = party._activeParty[idx];
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


} // End of namespace Xeen
