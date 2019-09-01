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

#include "common/config-manager.h"
#include "common/textconsole.h"
#include "backends/audiocd/audiocd.h"
#include "xeen/scripts.h"
#include "xeen/dialogs/dialogs_copy_protection.h"
#include "xeen/dialogs/dialogs_input.h"
#include "xeen/dialogs/dialogs_whowill.h"
#include "xeen/dialogs/dialogs_query.h"
#include "xeen/party.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

byte EventParameters::Iterator::readByte() {
	byte result = (_index >= _data.size()) ? 0 : _data[_index];
	++_index;
	return result;
}

uint16 EventParameters::Iterator::readUint16LE() {
	uint16 result = ((_index + 1) >= _data.size()) ? 0 :
		READ_LE_UINT16(&_data[_index]);
	_index += 2;
	return result;
}

uint32 EventParameters::Iterator::readUint32LE() {
	uint32 result = ((_index + 3) >= _data.size()) ? 0 :
		READ_LE_UINT32(&_data[_index]);
	_index += 4;
	return result;
}

/*------------------------------------------------------------------------*/

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
	_whoWill = 0;
	_treasureItems = 0;
	_lineNum = 0;
	_charIndex = 0;
	_nEdamageType = DT_PHYSICAL;
	_animCounter = 0;
	_eventSkipped = false;
	_mirrorId = -1;
	_refreshIcons = false;
	_scriptResult = false;
	_scriptExecuted = false;
	_dirFlag = false;
	_redrawDone = false;
	_windowIndex = -1;
	_event = nullptr;
}

int Scripts::checkEvents() {
	Combat &combat = *_vm->_combat;
	EventsManager &events = *_vm->_events;
	FileManager &files = *_vm->_files;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	int ccNum = files._ccNum;

	_refreshIcons = false;
	_scriptExecuted = false;
	_dirFlag = false;
	_whoWill = 0;
	Mode oldMode = _vm->_mode;
	Common::fill(&intf._charFX[0], &intf._charFX[MAX_ACTIVE_PARTY], 0);

	if (party._treasure._gold & party._treasure._gems) {
		// Backup any current treasure data
		party._savedTreasure = party._treasure;
		party._treasure._hasItems = false;
		party._treasure._gold = 0;
		party._treasure._gems = 0;
	} else {
		party._savedTreasure._hasItems = false;
		party._savedTreasure._gold = 0;
		party._savedTreasure._gems = 0;
	}

	do {
		_lineNum = 0;
		_scriptResult = false;
		_animCounter = 0;
		_redrawDone = false;
		_currentPos = party._mazePosition;
		_charIndex = 1;
		combat._combatTarget = 1;
		_nEdamageType = DT_PHYSICAL;

		while (!_vm->shouldExit() && _lineNum >= 0) {
			// Stop processing events if there's an attacking monster
			if (combat._attackMonsters[0] != -1) {
				_eventSkipped = true;
				_lineNum = SCRIPT_ABORT;
				break;
			}

			uint eventIndex;
			for (eventIndex = 0; eventIndex < map._events.size() && !_vm->shouldExit(); ++eventIndex) {
				MazeEvent &event = map._events[eventIndex];

				if (event._position == _currentPos && event._line == _lineNum &&
						(party._mazeDirection | _currentPos.x | _currentPos.y)) {
					if (event._direction == party._mazeDirection || event._direction == DIR_ALL) {
						_vm->_mode = MODE_SCRIPT_IN_PROGRESS;
						_scriptExecuted = true;
						doOpcode(event);
						break;
					} else {
						_dirFlag = true;
					}
				}
			}
			if (eventIndex == map._events.size())
				_lineNum = SCRIPT_ABORT;
		}
	} while (!_vm->shouldExit() && _lineNum != SCRIPT_ABORT);

	intf._face1State = intf._face2State = 2;
	if (_refreshIcons) {
		windows.closeAll();
		intf.drawParty(true);
	}

	party.checkPartyDead();
	if (party._treasure._hasItems || party._treasure._gold || party._treasure._gems)
		party.giveTreasure();

	if (_animCounter > 0 && intf._objNumber != -1) {
		MazeObject &selectedObj = map._mobData._objects[intf._objNumber];

		if (selectedObj._spriteId == (ccNum ? 15 : 16)) {
			// Treasure chests that were opened will be set to be in an open, empty state
			for (uint idx = 0; idx < map._mobData._objectSprites.size(); ++idx) {
				MonsterObjectData::SpriteResourceEntry &e = map._mobData._objectSprites[idx];
				if (e._spriteId == (ccNum ? 57 : 62)) {
					selectedObj._id = idx;
					selectedObj._spriteId = ccNum ? 57 : 62;
					selectedObj._sprites = &e._sprites;
					break;
				}
			}
		} else if (selectedObj._spriteId == 73) {
			for (uint idx = 0; idx < map._mobData._objectSprites.size(); ++idx) {
				MonsterObjectData::SpriteResourceEntry &e = map._mobData._objectSprites[idx];
				if (e._spriteId == 119) {
					selectedObj._id = idx;
					selectedObj._spriteId = 119;
					selectedObj._sprites = &e._sprites;
					break;
				}
			}
		}
	}

	_animCounter = 0;
	_vm->_mode = oldMode;
	windows.closeAll();

	if (g_vm->getIsCD() && g_system->getAudioCDManager()->isPlaying())
		// Stop any playing voice
		g_system->getAudioCDManager()->stop();

	if (g_vm->shouldExit())
		return g_vm->_gameMode;

	if (_scriptExecuted)
		intf.clearEvents();
	if (_scriptExecuted || intf._objNumber == -1 || _dirFlag) {
		if (_dirFlag && !_scriptExecuted && intf._objNumber != -1 && !map._currentIsEvent) {
			sound.playFX(21);
		}
	} else {
		Window &w = windows[38];
		w.open();
		w.writeString(Res.NOTHING_HERE);
		w.update();

		do {
			intf.draw3d(true);
			events.updateGameCounter();
			events.wait(1);
		} while (!events.isKeyMousePressed() && !_vm->shouldExit());
		events.clearEvents();

		w.close();
	}

	// Restore saved treasure
	if (party._savedTreasure._hasItems || party._savedTreasure._gold ||
			party._savedTreasure._gems) {
		party._treasure = party._savedTreasure;
	}

	combat._combatTarget = 1;
	Common::fill(&intf._charFX[0], &intf._charFX[6], 0);

	return _scriptResult;
}

bool Scripts::openGrate(int wallVal, int action) {
	Combat &combat = *_vm->_combat;
	FileManager &files = *_vm->_files;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	int ccNum = files._ccNum;

	if (!((wallVal != 13 || map._currentGrateUnlocked) && (!ccNum || wallVal != 9 ||
			map.mazeData()._wallKind != 2)))
		return false;

	if (wallVal != 9 && !map._currentGrateUnlocked) {
		int charIndex = WhoWill::show(_vm, 13, action, false) - 1;
		if (charIndex < 0) {
			intf.draw3d(true);
			return true;
		}

		// There is a 1 in 4 chance the character will receive damage
		if (_vm->getRandomNumber(1, 4) == 1) {
			combat.giveCharDamage(map.mazeData()._trapDamage,
				(DamageType)_vm->getRandomNumber(0, 6), charIndex);
		}

		// Check whether character can unlock the door
		Character &c = party._activeParty[charIndex];
		if ((c.getThievery() + _vm->getRandomNumber(1, 20)) <
				map.mazeData()._difficulties._unlockDoor)
			return true;

		c._experience += map.mazeData()._difficulties._unlockDoor * c.getCurrentLevel();
	}

	// Flag the grate as unlocked, and the wall the grate is on
	map.setCellSurfaceFlags(party._mazePosition, 0x80);
	map.setWall(party._mazePosition, party._mazeDirection, wallVal);

	// Set the grate as opened and the wall on the other side of the grate
	Common::Point pt = party._mazePosition;
	Direction dir = (Direction)((int)party._mazeDirection ^ 2);
	switch (party._mazeDirection) {
	case DIR_NORTH:
		pt.y++;
		break;
	case DIR_EAST:
		pt.x++;
		break;
	case DIR_SOUTH:
		pt.y--;
		break;
	case DIR_WEST:
		pt.x--;
		break;
	default:
		break;
	}

	map.setCellSurfaceFlags(pt, 0x80);
	map.setWall(pt, dir, wallVal);

	sound.playFX(10);
	intf.draw3d(true);
	return true;
}

bool Scripts::doOpcode(MazeEvent &event) {
	Map &map = *_vm->_map;
	typedef bool(Scripts::*ScriptMethodPtr)(ParamsIterator &);
	static const ScriptMethodPtr COMMAND_LIST[] = {
		&Scripts::cmdDoNothing, &Scripts::cmdDisplay1, &Scripts::cmdDoorTextSml,
		&Scripts::cmdDoorTextLrg, &Scripts::cmdSignText,
		&Scripts::cmdNPC, &Scripts::cmdPlayFX, &Scripts::cmdTeleport,
		&Scripts::cmdIf, &Scripts::cmdIf, &Scripts::cmdIf,
		&Scripts::cmdMoveObj, &Scripts::cmdTakeOrGive, &Scripts::cmdDoNothing,
		&Scripts::cmdRemove, &Scripts::cmdSetChar, &Scripts::cmdSpawn,
		&Scripts::cmdDoTownEvent, &Scripts::cmdExit, &Scripts::cmdAlterMap,
		&Scripts::cmdGiveMulti, &Scripts::cmdConfirmWord, &Scripts::cmdDamage,
		&Scripts::cmdJumpRnd, &Scripts::cmdAlterEvent, &Scripts::cmdCallEvent,
		&Scripts::cmdReturn, &Scripts::cmdSetVar, &Scripts::cmdTakeOrGive,
		&Scripts::cmdTakeOrGive, &Scripts::cmdCutsceneEndClouds,
		&Scripts::cmdTeleport, &Scripts::cmdWhoWill,
		&Scripts::cmdRndDamage, &Scripts::cmdMoveWallObj, &Scripts::cmdAlterCellFlag,
		&Scripts::cmdAlterHed, &Scripts::cmdDisplayStat, &Scripts::cmdTakeOrGive,
		&Scripts::cmdSignTextSml, &Scripts::cmdPlayEventVoc, &Scripts::cmdDisplayBottom,
		&Scripts::cmdIfMapFlag, &Scripts::cmdSelectRandomChar, &Scripts::cmdGiveEnchanted,
		&Scripts::cmdItemType, &Scripts::cmdMakeNothingHere, &Scripts::cmdCheckProtection,
		&Scripts::cmdChooseNumeric, &Scripts::cmdDisplayBottomTwoLines,
		&Scripts::cmdDisplayLarge, &Scripts::cmdExchObj, &Scripts::cmdFallToMap,
		&Scripts::cmdDisplayMain, &Scripts::cmdGoto, &Scripts::cmdConfirmWord,
		&Scripts::cmdGotoRandom, &Scripts::cmdCutsceneEndDarkside,
		&Scripts::cmdCutsceneEndWorld, &Scripts::cmdFlipWorld, &Scripts::cmdPlayCD
	};

	_event = &event;

	// Some opcodes use the first parameter as a message
	uint msgId = event._parameters.empty() ? 0 : event._parameters[0];
	_message = msgId >= map._events._text.size() ? "" : map._events._text[msgId];

	// Execute the opcode
	ParamsIterator params = event._parameters.getIterator();
	bool result = (this->*COMMAND_LIST[event._opcode])(params);
	if (result)
		// Move to next line
		_lineNum = _vm->_party->_dead ? SCRIPT_ABORT : _lineNum + 1;

	return result;
}

bool Scripts::cmdDoNothing(ParamsIterator &params) {
	return true;
}

bool Scripts::cmdDisplay1(ParamsIterator &params) {
	Windows &windows = *_vm->_windows;
	Common::String paramText = _vm->_map->_events._text[params.readByte()];
	Common::String msg = Common::String::format("\r\x03""c%s", paramText.c_str());

	windows[12].close();
	if (!windows[38]._enabled)
		windows[38].open();
	windows[38].writeString(msg);
	windows[38].update();

	return true;
}

bool Scripts::cmdDoorTextSml(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[params.readByte()];
	intf._screenText = Common::String::format("\x02\f""08\x03""c\t116\v025%s\x03""l\fd""\x01",
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	return true;
}

bool Scripts::cmdDoorTextLrg(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[params.readByte()];
	intf._screenText = Common::String::format("\f04\x03""c\t116\v030%s\x03""l\fd",
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	return true;
}

bool Scripts::cmdSignText(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;

	Common::String paramText = _vm->_map->_events._text[params.readByte()];
	intf._screenText = Common::String::format("\f08\x03""c\t120\v088%s\x03""l\fd",
		paramText.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	return true;
}

bool Scripts::cmdNPC(ParamsIterator &params) {
	Map &map = *_vm->_map;

	params.readByte();					// _message already holds title
	int textNum = params.readByte();
	int portrait = params.readByte();
	int confirm = params.readByte();
	int lineNum = params.readByte();

	if (LocationMessage::show(portrait, _message, map._events._text[textNum],
			confirm)) {
		_lineNum = lineNum;
		return false;
	}

	return true;
}

bool Scripts::cmdPlayFX(ParamsIterator &params) {
	_vm->_sound->playFX(params.readByte());

	return true;
}

bool Scripts::cmdTeleport(ParamsIterator &params) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Sound &sound = *_vm->_sound;

	windows.closeAll();

	bool restartFlag = _event->_opcode == OP_TeleportAndContinue;
	int mapId = params.readByte();
	Common::Point pt;

	if (mapId) {
		// Specific map, x & y specified
		pt.x = params.readShort();
		pt.y = params.readShort();
	} else {
		// Mirror teleportation
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
		int spriteId = (intf._objNumber == -1) ? -1 : map._mobData._objects[intf._objNumber]._spriteId;

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
		party.moveToRunLocation();
	} else {
		party._mazePosition = pt;
	}

	events.clearEvents();

	if (restartFlag) {
		// Draw the new location and start any script at that location
		events.ipause(2);
		_lineNum = SCRIPT_RESET;
		return false;
	} else {
		// Stop executing the script
		return cmdExit(params);
	}
}

bool Scripts::cmdIf(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	uint32 val;
	int newLineNum;

	int mode = params.readByte();
	switch (mode) {
	case 16:
	case 34:
	case 100:
		val = params.readUint32LE();
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		val = params.readUint16LE();
		break;
	default:
		val = params.readByte();
		break;
	}
	newLineNum = params.readByte();

	bool result;
	if ((_charIndex != 0 && _charIndex != 8) || mode == 44) {
		result = ifProc(mode, val, _event->_opcode - 8, _charIndex - 1);
	} else {
		result = false;
		for (int idx = 0; idx < (int)party._activeParty.size() && !result; ++idx) {
			if (_charIndex == 0 || (_charIndex == 8 && (int)idx != combat._combatTarget)) {
				result = ifProc(mode, val, _event->_opcode - 8, idx);
			}
		}
	}

	if (result) {
		_lineNum = newLineNum;
		return false;
	}

	return true;
}

bool Scripts::cmdMoveObj(ParamsIterator &params) {
	MazeObject &mazeObj = _vm->_map->_mobData._objects[params.readByte()];
	int x = params.readShort(), y = params.readShort();

	if (mazeObj._position.x == x && mazeObj._position.y == y) {
		// Already in position, so simply flip it
		mazeObj._flipped = !mazeObj._flipped;
	} else {
		mazeObj._position.x = x;
		mazeObj._position.y = y;
	}

	return true;
}

bool Scripts::cmdTakeOrGive(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	int mode1, mode2, mode3;
	uint32 val1, val2, val3;

	_refreshIcons = true;
	mode1 = params.readByte();
	switch (mode1) {
	case 16:
	case 34:
	case 100:
		val1 = params.readUint32LE();
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		val1 = params.readUint16LE();
		break;
	default:
		val1 = params.readByte();
		break;
	}

	mode2 = params.readByte();
	switch (mode2) {
	case 16:
	case 34:
	case 100:
		val2 = params.readUint32LE();
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		val2 = params.readUint16LE();
		break;
	default:
		val2 = params.readByte();
		break;
	}

	mode3 = params.readByte();
	switch (mode3) {
	case 16:
	case 34:
	case 100:
		val3 = params.readUint32LE();
		break;
	case 25:
	case 35:
	case 101:
	case 106:
		val3 = params.readUint16LE();
		break;
	default:
		val3 = params.readByte();
		break;
	}

	if (mode2 == 67)
		windows.closeAll();

	switch (_event->_opcode) {
	case OP_TakeOrGive_2:
		if (_charIndex == 0 || _charIndex == 8) {
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				if (_charIndex == 0 || (_charIndex == 8 && (int)idx != combat._combatTarget)) {
					if (ifProc(mode1, val1, _event->_opcode == OP_TakeOrGive_4 ? 2 : 1, idx)) {
						party.giveTake(0, 0, mode2, val2, idx);
						if (mode2 == 82)
							break;
					}
				}
			}
		} else if (ifProc(mode1, val1, _event->_opcode == OP_TakeOrGive_4 ? 2 : 1, _charIndex - 1)) {
			party.giveTake(0, 0, mode2, val2, _charIndex - 1);
		}
		break;

	case OP_TakeOrGive_3:
		if (_charIndex == 0 || _charIndex == 8) {
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				if (_charIndex == 0 || (_charIndex == 8 && (int)idx != combat._combatTarget)) {
					if (ifProc(mode1, val1, 1, idx) && ifProc(mode2, val2, 1, idx)) {
						party.giveTake(0, 0, mode2, val3, idx);
						if (mode2 == 82)
							break;
					}
				}
			}
		} else if (ifProc(mode1, val1, 1, _charIndex - 1) &&
				ifProc(mode2, val2, 1, _charIndex - 1)) {
			party.giveTake(0, 0, mode2, val3, _charIndex - 1);
		}
		break;

	case OP_TakeOrGive_4:
		if (_charIndex == 0 || _charIndex == 8) {
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				if (_charIndex == 0 || (_charIndex == 8 && (int)idx != combat._combatTarget)) {
					if (ifProc(mode1, val1, _event->_opcode == OP_TakeOrGive_4 ? 2 : 1, idx)) {
						party.giveTake(0, 0, mode2, val2, idx);
						if (mode2 == 82)
							break;
					}
				}
			}
		} else if (ifProc(mode1, val1, 1, _charIndex - 1)) {
			party.giveTake(0, 0, mode2, val2, _charIndex - 1);
		}
		break;

	default:
		if (_charIndex == 0 || _charIndex == 8) {
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				if (_charIndex == 0 || (_charIndex == 8 && (int)idx != combat._combatTarget)) {
					bool flag = party.giveTake(mode1, val1, mode2, val2, idx);

					switch (mode1) {
					case 8:
						mode1 = 0;
						// fall through
					case 21:
					case 66:
						if (flag) {
							switch (mode2) {
							case 82:
								mode1 = 0;
								// fall through
							case 21:
							case 34:
							case 35:
							case 65:
							case 66:
							case 100:
							case 101:
							case 106:
								if (flag)
									continue;

								// Break out of character loop
								idx = party._activeParty.size();
								break;
							default:
								break;
							}
						} else {
							// Break out of character loop
							idx = party._activeParty.size();
						}
						break;

					case 34:
					case 35:
					case 65:
					case 100:
					case 101:
					case 106:
						if (flag) {
							_lineNum = -1;
							return false;
						}

						// Break out of character loop
						idx = party._activeParty.size();
						break;

					default:
						switch (mode2) {
						case 82:
							mode1 = 0;
							// fall through
						case 21:
						case 34:
						case 35:
						case 65:
						case 66:
						case 100:
						case 101:
						case 106:
							if (flag)
								continue;

							// Break out of character loop
							idx = party._activeParty.size();
							break;
						}
						break;
					}
				}
			}
		} else {
			if (!party.giveTake(mode1, val1, mode2, val2, _charIndex - 1)) {
				if (mode2 == 79)
					windows.closeAll();
			} else {
				return cmdExit(params);
			}
		}
		break;
	}

	return true;
}

bool Scripts::cmdRemove(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;

	if (intf._objNumber != -1) {
		// Give the active object a completely way out of bounds position
		MazeObject &obj = map._mobData._objects[intf._objNumber];
		obj._position = Common::Point(128, 128);
	}

	cmdMakeNothingHere(params);
	return true;
}

bool Scripts::cmdSetChar(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	int charId = params.readByte();

	if (charId == 0) {
		_charIndex = 0;
		combat._combatTarget = 0;
	} else if (charId < 7) {
		combat._combatTarget = charId;
	} else if (charId == 7) {
		_charIndex = _vm->getRandomNumber(1, _vm->_party->_activeParty.size());
		combat._combatTarget = 1;
	} else {
		_charIndex = WhoWill::show(_vm, 22, 3, false);
		if (_charIndex == 0)
			return cmdExit(params);
	}

	return true;
}

bool Scripts::cmdSpawn(ParamsIterator &params) {
	Map &map = *_vm->_map;
	uint index = params.readByte();

	if (index >= map._mobData._monsters.size())
		map._mobData._monsters.resize(index + 1);

	MazeMonster &monster = _vm->_map->_mobData._monsters[index];
	MonsterStruct &monsterData = _vm->_map->_monsterData[monster._spriteId];
	monster._monsterData = &monsterData;
	monster._position.x = params.readShort();
	monster._position.y = params.readShort();
	monster._frame = _vm->getRandomNumber(7);
	monster._damageType = DT_PHYSICAL;
	monster._isAttacking = false;
	monster._hp = monsterData._hp;

	return true;
}

bool Scripts::cmdDoTownEvent(ParamsIterator &params) {
	_scriptResult = _vm->_locations->doAction(params.readByte());
	_vm->_party->_stepped = true;
	_refreshIcons = true;

	return cmdExit(params);
}

bool Scripts::cmdExit(ParamsIterator &params) {
	_lineNum = SCRIPT_ABORT;
	return false;
}

bool Scripts::cmdAlterMap(ParamsIterator &params) {
	Map &map = *_vm->_map;
	int x = params.readShort();
	int y = params.readShort();
	Direction dir = (Direction)params.readByte();
	int val = params.readByte();

	if (dir == DIR_ALL) {
		for (dir = DIR_NORTH; dir <= DIR_WEST; dir = (Direction)((int)dir + 1))
			map.setWall(Common::Point(x, y), dir, val);
	} else {
		map.setWall(Common::Point(x, y), dir, val);
	}

	return true;
}

bool Scripts::cmdGiveMulti(ParamsIterator &params) {
	Party &party = *_vm->_party;
	int modes[3];
	uint32 vals[3];

	_refreshIcons = true;
	for (int idx = 0; idx < 3; ++idx) {
		modes[idx] = params.readByte();
		switch (modes[idx]) {
		case 16:
		case 34:
		case 100:
			vals[idx] = params.readUint32LE();
			break;
		case 25:
		case 35:
		case 101:
		case 106:
			vals[idx] = params.readUint16LE();
			break;
		default:
			vals[idx] = params.readByte();
			break;
		}
	}

	_scriptExecuted = true;
	bool result = party.giveExt(modes[0], vals[0], modes[1], vals[1], modes[2], vals[2],
		(_charIndex > 0) ? _charIndex - 1 : 0);

	if (result) {
		if (_animCounter == 255) {
			_animCounter = 0;
			return cmdExit(params);
		} else if (modes[0] == 67 || modes[1] == 67 || modes[2] == 67) {
			_animCounter = 1;
		} else {
			return cmdExit(params);
		}
	} else {
		if (modes[0] == 67 || modes[1] == 67 || modes[2] == 67)
			return cmdExit(params);
	}

	return true;
}

bool Scripts::cmdConfirmWord(ParamsIterator &params) {
	FileManager &files = *_vm->_files;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int inputType = params.readByte();
	int lineNum = params.readByte();
	int param2 = params.readByte();
	int param3 = params.readByte();

	Common::String expected2;
	Common::String title;

	if (_event->_opcode == OP_ConfirmWord_2) {
		title = "";
	} else if (param3) {
		title = map._events._text[param3];
	} else {
		title = Res.WHATS_THE_PASSWORD;
	}

	if (!param2) {
		expected2 = _message;
	} else if (param2 < (int)map._events._text.size()) {
		expected2 = map._events._text[param2];
	}

	_mirrorId = StringInput::show(_vm, inputType, expected2, title, _event->_opcode);
	if (_mirrorId) {
		if (_mirrorId == 33 && files._ccNum) {
			doDarkSideEnding();
		} else if (_mirrorId == 34 && files._ccNum) {
			doWorldEnding();
		} else if (_mirrorId == 35 && files._ccNum &&
				_vm->getGameID() == GType_WorldOfXeen) {
			doCloudsEnding();
		} else if (_mirrorId == 40 && !files._ccNum) {
			doCloudsEnding();
		} else if (_mirrorId == 60 && !files._ccNum) {
			doDarkSideEnding();
		} else if (_mirrorId == 61 && !files._ccNum) {
			doWorldEnding();
		} else {
			if (_mirrorId == 59 && !files._ccNum) {
				for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
					XeenItem &item = party._treasure._weapons[idx];
					if (!item._id) {
						item._id = XEEN_SLAYER_SWORD;
						item._material = 0;
						item._state.clear();
						party._treasure._hasItems = true;

						return cmdExit(params);
					}
				}
			}

			_lineNum = _mirrorId == -1 ? param3 : lineNum;
			return false;
		}
	}

	return true;
}

bool Scripts::cmdDamage(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;

	if (!_redrawDone) {
		intf.draw3d(true);
		_redrawDone = true;
	}

	int damage = params.readUint16LE();
	DamageType damageType = (DamageType)params.readByte();
	combat.giveCharDamage(damage, damageType, _charIndex - 1);

	return true;
}

bool Scripts::cmdJumpRnd(ParamsIterator &params) {
	int v = _vm->getRandomNumber(1, params.readByte());
	if (v == params.readByte()) {
		_lineNum = params.readByte();
		return false;
	}

	return true;
}

bool Scripts::cmdAlterEvent(ParamsIterator &params) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int lineNum = params.readByte();
	Opcode opcode = (Opcode)params.readByte();

	for (uint idx = 0; idx < map._events.size(); ++idx) {
		MazeEvent &evt = map._events[idx];
		if (evt._position == party._mazePosition &&
				(evt._direction == DIR_ALL || evt._direction == party._mazeDirection) &&
				evt._line == lineNum) {
			evt._opcode = opcode;
		}
	}

	return true;
}

bool Scripts::cmdCallEvent(ParamsIterator &params) {
	_stack.push(StackEntry(_currentPos, _lineNum));
	_currentPos.x = params.readShort();
	_currentPos.y = params.readShort();
	_lineNum = params.readByte();

	return false;
}

bool Scripts::cmdReturn(ParamsIterator &params) {
	if (_stack.empty()) {
		// WORKAROUND: Some scripts in Swords of Xeen use cmdReturn as a substitute for cmdExit
		return cmdExit(params);
	} else {
		StackEntry se = _stack.pop();
		_currentPos = se;
		_lineNum = se.line;

		return true;
	}
}

bool Scripts::cmdSetVar(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	Party &party = *_vm->_party;
	uint val;
	_refreshIcons = true;

	int mode = params.readByte();
	switch (mode) {
	case 25:
	case 35:
	case 101:
	case 106:
		val = params.readUint16LE();
		break;
	case 16:
	case 34:
	case 100:
		val = params.readUint32LE();
		break;
	default:
		val = params.readByte();
		break;
	}

	if (_charIndex != 0 && _charIndex != 8) {
		party._activeParty[_charIndex - 1].setValue(mode, val);
	} else {
		// Set value for entire party
		for (int idx = 0; idx < (int)party._activeParty.size(); ++idx) {
			if (_charIndex == 0 || (_charIndex == 8 && combat._combatTarget != idx)) {
				party._activeParty[idx].setValue(mode, val);
			}
		}
	}

	return true;
}

bool Scripts::cmdCutsceneEndClouds(ParamsIterator &params) {
	Party &party = *_vm->_party;
	party._gameFlags[0][75] = true;
	party._mazeId = 28;
	party._mazePosition = Common::Point(18, 4);

	g_vm->_gameWon[0] = true;
	g_vm->_finalScore = party.getScore();
	g_vm->saveSettings();

	doCloudsEnding();
	return false;
}

bool Scripts::cmdWhoWill(ParamsIterator &params) {
	int msg = params.readByte();
	int action = params.readByte();
	_charIndex = WhoWill::show(_vm, msg, action, true);

	if (_charIndex == 0)
		return cmdExit(params);
	else
		return true;
}

bool Scripts::cmdRndDamage(ParamsIterator &params) {
	Combat &combat = *_vm->_combat;
	Interface &intf = *_vm->_interface;

	if (!_redrawDone) {
		intf.draw3d(true);
		_redrawDone = true;
	}

	DamageType dmgType = (DamageType)params.readByte();
	int max = params.readByte();
	combat.giveCharDamage(_vm->getRandomNumber(1, max), dmgType, _charIndex - 1);
	return true;
}

bool Scripts::cmdMoveWallObj(ParamsIterator &params) {
	Map &map = *_vm->_map;
	int index = params.readByte();
	int x = params.readShort();
	int y = params.readShort();

	map._mobData._wallItems[index]._position = Common::Point(x, y);
	return true;
}

bool Scripts::cmdAlterCellFlag(ParamsIterator &params) {
	Map &map = *_vm->_map;
	Common::Point pt;
	pt.x = params.readShort();
	pt.y = params.readShort();
	int surfaceId = params.readByte();

	map.cellFlagLookup(pt);

	if (map._isOutdoors) {
		MazeWallLayers &wallData = map.mazeDataCurrent()._wallData[pt.y][pt.x];
		wallData._data = (wallData._data & 0xFFF0) | surfaceId;
	} else {
		pt.x &= 0xF;
		pt.y &= 0xF;
		MazeCell &cell = map.mazeDataCurrent()._cells[pt.y][pt.x];
		cell._surfaceId = surfaceId;
	}

	return true;
}

bool Scripts::cmdAlterHed(ParamsIterator &params) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	HeadData::HeadEntry &he = map._headData[party._mazePosition.y][party._mazePosition.x];
	he._left = params.readByte();
	he._right = params.readByte();

	return true;
}

bool Scripts::cmdDisplayStat(ParamsIterator &params) {
	Party &party = *_vm->_party;
	Windows &windows = *_vm->_windows;
	Window &w = windows[12];
	Character &c = party._activeParty[_charIndex - 1];

	if (!w._enabled)
		w.open();
	w.writeString(Common::String::format(_message.c_str(), c._name.c_str()));
	w.update();

	return true;
}

bool Scripts::cmdSignTextSml(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;

	intf._screenText = Common::String::format("\x2\f08\x3""c\t116\v090%s\x3l\fd\x1",
		_message.c_str());
	intf._upDoorText = true;
	intf.draw3d(true);

	return true;
}

bool Scripts::cmdPlayEventVoc(ParamsIterator &params) {
	Sound &sound = *_vm->_sound;
	sound.stopSound();
	sound.playSound(Res.EVENT_SAMPLES[params.readByte()], 1);

	return true;
}

bool Scripts::cmdDisplayBottom(ParamsIterator &params) {
	_windowIndex = 12;

	display(false, 0);
	return true;
}

bool Scripts::cmdIfMapFlag(ParamsIterator &params) {
	Map &map = *_vm->_map;
	int monsterNum = params.readByte();
	int lineNum = params.readByte();

	if (monsterNum == 0xff) {
		for (monsterNum = 0; monsterNum < (int)map._mobData._monsters.size(); ++monsterNum) {
			MazeMonster &monster = map._mobData._monsters[monsterNum];

			if ((uint)monster._position.x < 32 && (uint)monster._position.y < 32)
				return true;
		}
	} else {
		MazeMonster &monster = map._mobData._monsters[monsterNum];

		if ((uint)monster._position.x < 32 && (uint)monster._position.y < 32)
			return true;
	}

	_lineNum = lineNum;
	return false;
}

bool Scripts::cmdSelectRandomChar(ParamsIterator &params) {
	_charIndex = _vm->getRandomNumber(1, _vm->_party->_activeParty.size());
	return true;
}

bool Scripts::cmdGiveEnchanted(ParamsIterator &params) {
	Party &party = *_vm->_party;
	int itemOffset = _vm->getGameID() == GType_Swords ? 6 : 0;
	XeenItem *item;
	int invIndex;
	int id = params.readByte();

	// Get category of item to add
	ItemCategory cat = CATEGORY_WEAPON;
	if (id < (35 + itemOffset)) {
	} else if (id < (49 + itemOffset)) {
		cat = CATEGORY_ARMOR;
		id -= 35 + itemOffset;
	} else if (id < (60 + itemOffset)) {
		cat = CATEGORY_ACCESSORY;
		id -= 49 + itemOffset;
	} else if (id < (82 + itemOffset)) {
		cat = CATEGORY_MISC;
		id -= 60 + itemOffset;
	} else {
		party._questItems[id - (82 + itemOffset)]++;
	}

	// Check for an empty slot
	for (invIndex = 0, item = party._treasure[cat]; invIndex < MAX_TREASURE_ITEMS && !item->empty(); ++invIndex, ++item)
		;

	if (invIndex == MAX_TREASURE_ITEMS) {
		// Treasure category entirely full. Should never happen
		warning("Treasure category was completely filled up");
	} else {
		party._treasure._hasItems = true;

		if (cat == CATEGORY_MISC) {
			// Handling of misc items. Note that for them, id actually specifies the material field
			item->_material = id;
			item->_id = params.readByte();
			item->_state._counter = (item->_material == 10 || item->_material == 11) ? 1 : _vm->getRandomNumber(3, 10);
		} else {
			// Weapons, armor, and accessories
			item->_id = id;
			item->_material = params.readByte();
			item->_state = params.readByte();
		}
	}

	return true;
}

bool Scripts::cmdItemType(ParamsIterator &params) {
	Character::_itemType = params.readByte();

	return true;
}

bool Scripts::cmdMakeNothingHere(ParamsIterator &params) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	// Scan through the event list and mark the opcodes for all the lines of any scripts
	// on the party's current cell as having no operation, effectively disabling them
	for (uint idx = 0; idx < map._events.size(); ++idx) {
		MazeEvent &evt = map._events[idx];
		if (evt._position == party._mazePosition)
			evt._opcode = OP_None;
	}

	return cmdExit(params);
}

bool Scripts::cmdCheckProtection(ParamsIterator &params) {
	if (copyProtectionCheck())
		return true;
	else
		return cmdExit(params);
}

bool Scripts::cmdChooseNumeric(ParamsIterator &params) {
	int choice = Choose123::show(_vm, params.readByte());
	if (choice) {
		_lineNum = _event->_parameters[choice];
		return false;
	}

	return true;
}

bool Scripts::cmdDisplayBottomTwoLines(ParamsIterator &params) {
	Map &map = *_vm->_map;
	Windows &windows = *_vm->_windows;
	Window &w = windows[12];

	params.readByte();
	int textId = params.readByte();

	Common::String msg = Common::String::format("\r\x03""c\t000\v007%s\n\n%s",
		"",
		map._events._text[textId].c_str());
	w.close();
	w.open();
	w.writeString(msg);
	w.update();

	YesNo::show(_vm, true);
	_lineNum = -1;
	return false;
}

bool Scripts::cmdDisplayLarge(ParamsIterator &params) {
	Party &party = *g_vm->_party;
	Common::String filename = Common::String::format("aaze2%03u.txt", party._mazeId);
	uint lineNumber = params.readByte();

	// Get the text data for the current maze
	File f(filename);
	char *data = new char[f.size()];
	f.read(data, f.size());
	f.close();

	// Get the message at the specified line
	const char *lineP = data;
	for (uint idx = 0; idx < lineNumber; ++idx, lineP += strlen(lineP) + 1) {}

	_message = Common::String(lineP);
	delete[] data;

	// Display the message
	_windowIndex = 11;
	display(true, 0);
	return true;
}

bool Scripts::cmdExchObj(ParamsIterator &params) {
	int id1 = params.readByte(), id2 = params.readByte();

	MazeObject &obj1 = _vm->_map->_mobData._objects[id1];
	MazeObject &obj2 = _vm->_map->_mobData._objects[id2];

	Common::Point pt = obj1._position;
	obj1._position = obj2._position;
	obj2._position = pt;

	return true;
}

bool Scripts::cmdFallToMap(ParamsIterator &params) {
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	party._fallMaze = params.readByte();
	party._fallPosition.x = params.readShort();
	party._fallPosition.y = params.readShort();
	party._fallDamage = params.readByte();
	intf.startFalling(true);

	_lineNum = SCRIPT_RESET;
	return false;
}

bool Scripts::cmdDisplayMain(ParamsIterator &params) {
	_windowIndex = 11;

	display(false, 0);
	return true;
}

bool Scripts::cmdGoto(ParamsIterator &params) {
	Map &map = *_vm->_map;
	map.getCell(1);
	if (map._currentSurfaceId == params.readByte()) {
		_lineNum = params.readByte();
		return false;
	}

	return true;
}

bool Scripts::cmdGotoRandom(ParamsIterator &params) {
	_lineNum = _event->_parameters[_vm->getRandomNumber(1, params.readByte())];
	return false;
}

bool Scripts::cmdCutsceneEndDarkside(ParamsIterator &params) {
	Party &party = *_vm->_party;
	_vm->_saves->_wonDarkSide = true;
	party._questItems[53] = 1;
	party._darkSideCompleted = true;
	party._mazeId = 29;
	party._mazeDirection = DIR_NORTH;
	party._mazePosition = Common::Point(25, 21);

	g_vm->_gameWon[1] = true;
	g_vm->_finalScore = party.getScore();
	g_vm->saveSettings();

	doDarkSideEnding();
	return false;
}

bool Scripts::cmdCutsceneEndWorld(ParamsIterator &params) {
	Party &party = *g_vm->_party;

	g_vm->_gameWon[2] = true;
	g_vm->_finalScore = party.getScore();
	g_vm->saveSettings();

	_vm->_saves->_wonWorld = true;
	_vm->_party->_worldCompleted = true;

	doWorldEnding();
	return false;
}

bool Scripts::cmdFlipWorld(ParamsIterator &params) {
	_vm->_map->_loadCcNum = params.readByte();
	return true;
}

bool Scripts::cmdPlayCD(ParamsIterator &params) {
	int trackNum = params.readByte();
	int start = params.readUint16LE();
	int finish = params.readUint16LE();
	debugC(3, kDebugScripts, "cmdPlayCD Track=%d start=%d finish=%d", trackNum, start, finish);

	if (_vm->_files->_ccNum && trackNum < 31)
		trackNum += 30;
	assert(trackNum <= 60);

	start = convertCDTime(start);
	finish = convertCDTime(finish);

	g_system->getAudioCDManager()->play(trackNum, 1, start, finish - start, false, Audio::Mixer::kSpeechSoundType);
	return true;
}

#define CD_FRAME_RATE 75
uint Scripts::convertCDTime(uint srcTime) {
	// Times are encoded as MMSSCC - MM=Minutes, SS=Seconds, CC=Centiseconds (1/100th second)
	uint mins = srcTime / 10000;
	uint csec = srcTime % 10000;
	return (mins * 6000 + csec) * CD_FRAME_RATE / 100;
}

void Scripts::doCloudsEnding() {
	g_vm->_party->_cloudsCompleted = true;
	doEnding("ENDGAME");

	g_vm->_mode = MODE_INTERACTIVE;
	g_vm->_saves->saveGame();

	g_vm->_gameMode = GMODE_MENU;
	g_vm->_mode = MODE_STARTUP;
}

void Scripts::doDarkSideEnding() {
	g_vm->_party->_darkSideCompleted = true;
	doEnding("ENDGAME2");
}

void Scripts::doWorldEnding() {
	doEnding("WORLDEND");
}

void Scripts::doEnding(const Common::String &endStr) {
	Party &party = *_vm->_party;

	int state = 0;
	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		Character &player = party._activeParty[idx];
		if (player.hasAward(SUPER_GOOBER)) {
			state = 2;
			break;
		} else if (player.hasAward(GOOBER)) {
			state = 1;
			break;
		}
	}

	// Get the current total score
	uint finalScore = party.getScore();

	g_vm->_mode = MODE_STARTUP;
	g_vm->showCutscene(endStr, state, finalScore);
	g_vm->_gameMode = GMODE_MENU;
}

bool Scripts::ifProc(int action, uint32 val, int mode, int charIndex) {
	FileManager &files = *_vm->_files;
	Party &party = *_vm->_party;
	Character *ps = (charIndex == -1) ? nullptr : &party._activeParty[charIndex];
	uint v = 0;

	switch (action) {
	case 3:
		// Player sex
		v = (uint)ps->_sex;
		break;
	case 4:
		// Player race
		v = (uint)ps->_race;
		break;
	case 5:
		// Player class
		v = (uint)ps->_class;
		break;
	case 8:
		// Current health points
		v = (uint)ps->_currentHp;
		break;
	case 9:
		// Current spell points
		v = (uint)ps->_currentSp;
		break;
	case 10:
		// Get armor class
		v = (uint)ps->getArmorClass(false);
		break;
	case 11:
		// Level bonus (extra beyond base)
		v = ps->_level._temporary;
		break;
	case 12:
		// Current age, including unnatural aging
		v = ps->getAge(false);
		break;
	case 13:
		assert(val < 18);
		if (ps->_skills[val])
			v = val;
		break;
	case 15:
		// Award
		assert(val < AWARDS_TOTAL);
		if (ps->hasAward(val))
			v = val;
		break;
	case 16:
		// Experience
		v = ps->_experience;
		break;
	case 17:
		// Party poison resistence
		v = party._poisonResistence;
		break;
	case 18:
		// Condition
		assert(val <= NO_CONDITION);
		v = (ps->_conditions[val] || val == NO_CONDITION) ? val : 0xffffffff;
		break;
	case 19: {
		// Can player cast a given spell
		SpellsCategory category = ps->getSpellsCategory();
		assert(category != SPELLCAT_INVALID);

		// Check if the character class can cast the particular spell
		for (int idx = 0; idx < SPELLS_PER_CLASS; ++idx) {
			if (Res.SPELLS_ALLOWED[category][idx] == (int)val) {
				// Can cast it. Check if the player has it in their spellbook
				if (ps->_spells[idx])
					v = val;
				break;
			}
		}
		break;
	}
	case 20:
		assert(val < 256);
		if (files._ccNum && _vm->getGameID() != GType_Swords)
			val += 256;
		v = party._gameFlags[val / 256][val % 256] ? val : 0xffffffff;
		break;
	case 21: {
		// Scans inventories for given item number
		uint itemOffset = _vm->getGameID() == GType_Swords ? 6 : 0;
		v = 0xFFFFFFFF;
		if (val < (82 + itemOffset)) {
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				if (val < (35 + itemOffset)) {
					if (ps->_weapons[idx]._id == val) {
						v = val;
						break;
					}
				} else if (val < (49 + itemOffset)) {
					if (ps->_armor[idx]._id == (val - 35)) {
						v = val;
						break;
					}
				} else if (val < (60 + itemOffset)) {
					if (ps->_accessories[idx]._id == (val - (49 + itemOffset))) {
						v = val;
						break;
					}
				} else {
					if (ps->_misc[idx]._id == (val - (60 + itemOffset))) {
						v = val;
						break;
					}
				}
			}
		} else if (party._questItems[val - (82 + itemOffset)]) {
			v = val;
		}
		break;
	}
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
		// Might bonus (extra beyond base)
		if (party._mazeId == 82)
			// WORKAROUND: Strength test opening sarcophagus in Northern Sphinx should use full might
			v = ps->getStat(MIGHT);
		else
			v = ps->_might._temporary;
		break;
	case 38:
		// Intellect bonus (extra beyond base)
		v = ps->_intellect._temporary;
		break;
	case 39:
		// Personality bonus (extra beyond base)
		v = ps->_personality._temporary;
		break;
	case 40:
		// Endurance bonus (extra beyond base)
		v = ps->_endurance._temporary;
		break;
	case 41:
		// Speed bonus (extra beyond base)
		v = ps->_speed._temporary;
		break;
	case 42:
		// Accuracy bonus (extra beyond base)
		v = ps->_accuracy._temporary;
		break;
	case 43:
		// Luck bonus (extra beyond base)
		v = ps->_luck._temporary;
		break;
	case 44:
		v = YesNo::show(_vm, val);
		v = (!v && !val) ? 2 : val;
		break;
	case 45:
		// Might base (before bonus)
		v = ps->_might._permanent;
		break;
	case 46:
		// Intellect base (before bonus)
		v = ps->_intellect._permanent;
		break;
	case 47:
		// Personality base (before bonus)
		v = ps->_personality._permanent;
		break;
	case 48:
		// Endurance base (before bonus)
		v = ps->_endurance._permanent;
		break;
	case 49:
		// Speed base (before bonus)
		v = ps->_speed._permanent;
		break;
	case 50:
		// Accuracy base (before bonus)
		v = ps->_accuracy._permanent;
		break;
	case 51:
		// Luck base (before bonus)
		v = ps->_luck._permanent;
		break;
	case 52:
		// Fire resistence (before bonus)
		v = ps->_fireResistence._permanent;
		break;
	case 53:
		// Elecricity resistence (before bonus)
		v = ps->_electricityResistence._permanent;
		break;
	case 54:
		// Cold resistence (before bonus)
		v = ps->_coldResistence._permanent;
		break;
	case 55:
		// Poison resistence (before bonus)
		v = ps->_poisonResistence._permanent;
		break;
	case 56:
		// Energy reistence (before bonus)
		v = ps->_energyResistence._permanent;
		break;
	case 57:
		// Energy resistence (before bonus)
		v = ps->_magicResistence._permanent;
		break;
	case 58:
		// Fire resistence (extra beyond base)
		v = ps->_fireResistence._temporary;
		break;
	case 59:
		// Electricity resistence (extra beyond base)
		v = ps->_electricityResistence._temporary;
		break;
	case 60:
		// Cold resistence (extra beyond base)
		v = ps->_coldResistence._temporary;
		break;
	case 61:
		// Poison resistence (extra beyod base)
		v = ps->_poisonResistence._temporary;
		break;
	case 62:
		// Energy resistence (extra beyond base)
		v = ps->_energyResistence._temporary;
		break;
	case 63:
		// Magic resistence (extra beyond base)
		v = ps->_magicResistence._temporary;
		break;
	case 64:
		// Level (before bonus)
		v = ps->_level._permanent;
		break;
	case 65:
		// Total party food
		v = party._food;
		break;
	case 69:
		v = party._levitateCount;
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
		v = ps->_ACTemp;
		break;
	case 78:
		// Test whether current Hp exceeds max HP or not
		v = ps->_currentHp <= ps->getMaxHP() ? 1 : 0;
		break;
	case 79:
		// Test for Wizard Eye being active
		v = party._wizardEyeActive ? 1 : 0;
		break;
	case 81:
		// Test whether current Sp exceeds the max SP or not
		v = ps->_currentSp <= ps->getMaxSP() ? 1 : 0;
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
		v = ps->getStat((Attribute)(action - 86), 0);
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
		v = party.checkSkill((Skill)val) ? val : 0xffffffff;
		break;
	case 102:
		// Thievery skill
		v = ps->getThievery();
		break;
	case 103:
		// Get value of world flag
		v = party._worldFlags[val] ? val : 0xffffffff;
		break;
	case 104:
		// Get value of quest flag
		v = party._questFlags[(_vm->getGameID() == GType_Swords ? 0 : files._ccNum * 30) + val] ? val : 0xffffffff;
		break;
	case 105:
		// Test number of Megacredits in party. Only used by King's Engineer in Castle Burlock
		v = party._questItems[26];
		break;
	case 107:
		// Get value of character flag
		v = party._characterFlags[ps->_rosterId][val] ? val : 0xffffffff;
		break;
	default:
		break;
	}

	switch (mode) {
	case 0:
		return v >= val;
	case 1:
		return v == val;
	case 2:
		return v <= val;
	default:
		return false;
	}
}

bool Scripts::copyProtectionCheck() {
	// Only bother doing the protection check if it's been explicitly turned on
	if (!ConfMan.getBool("copy_protection"))
		return true;

	// Show the copy protection dialog
	return CopyProtection::show(_vm);
}

void Scripts::display(bool justifyFlag, int var46) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Windows &windows = *_vm->_windows;
	Window &w = windows[_windowIndex];

	if (!_redrawDone) {
		intf.draw3d(true);
		_redrawDone = true;
	}
	windows[38].close();

	if (!justifyFlag)
		_displayMessage = Common::String::format("\r\x3""c%s", _message.c_str());
	else
		_displayMessage = _message;

	if (!w._enabled)
		w.open();

	while (!_vm->shouldExit()) {
		const char *newPos = w.writeString(_displayMessage);
		w.update();

		// Check for end of message
		if (!newPos)
			break;
		_displayMessage = Common::String(newPos);
		if (_displayMessage.empty())
			break;

		// Wait for click
		events.clearEvents();

		do {
			events.updateGameCounter();
			intf.draw3d(true);

			events.wait(1);
		} while (!_vm->shouldExit() && !events.isKeyMousePressed());

		w.writeString(justifyFlag ? "\r" : "\r\x3""c");
	}
}

} // End of namespace Xeen
