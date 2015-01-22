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
#include "xeen/party.h"
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

	int var18 = 0;
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
		int varA = 0;
		_animCounter = 0;
		int var4E = 0;
		const Common::Point pt = party._mazePosition;
		int varC = 1;
		_v2 = 1;
		_nEdamageType = 0;
		int var40 = -1;

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

				if (event._position == pt && party._mazeDirection != (pt.x | pt.y)
						&& event._line == _lineNum) {
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
		&Scripts::cmdDoTownEvent, &Scripts::cmdExit, &Scripts::cmdAfterMap,
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
		&Scripts::cmdDisplayMain, &Scripts::cmdGoto, &Scripts::cmdConfirmWord2,
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

void Scripts::cmdPlayFX(Common::Array<byte> &params) {
	_vm->_sound->playFX(params[0]);

	_var4F = true;
	cmdNoAction(params);
}

void Scripts::cmdTeleport(Common::Array<byte> &params) {
}

void Scripts::cmdIf(Common::Array<byte> &params) {
	switch (params[0]) {
	case 16:
	case 34:
	case 100:
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		break;
	default:
		break;
	}
}

void Scripts::cmdMoveObj(Common::Array<byte> &params) {}
void Scripts::cmdTakeOrGive(Common::Array<byte> &params) {}

void Scripts::cmdNoAction(Common::Array<byte> &params) {
	// Move to next line
	_lineNum = _vm->_party->_partyDead ? -1 : _lineNum + 1;
}

void Scripts::cmdRemove(Common::Array<byte> &params) {}
void Scripts::cmdSetChar(Common::Array<byte> &params) {}
void Scripts::cmdSpawn(Common::Array<byte> &params) {}
void Scripts::cmdDoTownEvent(Common::Array<byte> &params) {}
void Scripts::cmdExit(Common::Array<byte> &params) {}
void Scripts::cmdAfterMap(Common::Array<byte> &params) {}
void Scripts::cmdGiveExtended(Common::Array<byte> &params) {}
void Scripts::cmdConfirmWord(Common::Array<byte> &params) {}
void Scripts::cmdDamage(Common::Array<byte> &params) {}
void Scripts::cmdJumpRnd(Common::Array<byte> &params) {}
void Scripts::cmdAfterEvent(Common::Array<byte> &params) {}
void Scripts::cmdCallEvent(Common::Array<byte> &params) {}
void Scripts::cmdReturn(Common::Array<byte> &params) {}
void Scripts::cmdSetVar(Common::Array<byte> &params) {}
void Scripts::cmdCutsceneEndClouds(Common::Array<byte> &params) {}
void Scripts::cmdWhoWill(Common::Array<byte> &params) {}
void Scripts::cmdRndDamage(Common::Array<byte> &params) {}
void Scripts::cmdMoveWallObj(Common::Array<byte> &params) {}
void Scripts::cmdAlterCellFlag(Common::Array<byte> &params) {}
void Scripts::cmdAlterHed(Common::Array<byte> &params) {}
void Scripts::cmdDisplayStat(Common::Array<byte> &params) {}
void Scripts::cmdSeatTextSml(Common::Array<byte> &params) {}
void Scripts::cmdPlayEventVoc(Common::Array<byte> &params) {}
void Scripts::cmdDisplayBottom(Common::Array<byte> &params) {}
void Scripts::cmdIfMapFlag(Common::Array<byte> &params) {}
void Scripts::cmdSelRndChar(Common::Array<byte> &params) {}
void Scripts::cmdGiveEnchanted(Common::Array<byte> &params) {}
void Scripts::cmdItemType(Common::Array<byte> &params) {}
void Scripts::cmdMakeNothingHere(Common::Array<byte> &params) {}
void Scripts::cmdNoAction2(Common::Array<byte> &params) {}
void Scripts::cmdChooseNumeric(Common::Array<byte> &params) {}
void Scripts::cmdDisplayBottomTwoLines(Common::Array<byte> &params) {}
void Scripts::cmdDisplayLarge(Common::Array<byte> &params) {}
void Scripts::cmdExchObj(Common::Array<byte> &params) {}
void Scripts::cmdFallToMap(Common::Array<byte> &params) {}
void Scripts::cmdDisplayMain(Common::Array<byte> &params) {}
void Scripts::cmdGoto(Common::Array<byte> &params) {}
void Scripts::cmdConfirmWord2(Common::Array<byte> &params) {}
void Scripts::cmdGotoRandom(Common::Array<byte> &params) {}
void Scripts::cmdCutsceneEndDarkside(Common::Array<byte> &params) {}
void Scripts::cmdCutsceneEdWorld(Common::Array<byte> &params) {}
void Scripts::cmdFlipWorld(Common::Array<byte> &params) {}
void Scripts::cmdPlayCD(Common::Array<byte> &params) {}

} // End of namespace Xeen
