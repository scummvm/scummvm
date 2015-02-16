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

#include "common/algorithm.h"
#include "common/rect.h"
#include "xeen/character.h"
#include "xeen/combat.h"
#include "xeen/interface.h"
#include "xeen/xeen.h"

namespace Xeen {

static const int MONSTER_GRID_X[48] = {
	1, 1, 1, 0, -1, -1, -1, 1, 1, 1, 0, -1,
	-1, -1, 1, 1, 1, 0, -1, -1, -1, 1, 1, 1,
	0, -1, -1, -1, 1, 1, 1, 0, -1, -1, -1, 1,
	1, 1, 0, -1, -1, -1, 1, 1, 1, 0, -1, -1
};

static const int MONSTER_GRID_Y[48] = {
	0, 0, 0, -1, 0, 0, 0, 0, 0, 0, -1, 0,
	0, 0, 0, 0, 0, -1, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
	0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0
};

static const int MONSTER_GRID3[48] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
	- 1, -1, -1, -1, -1, -1, -1, -1, -1, 1, 1, 1,
	0, -1, -1, -1, 1, 1, 1, 1, 1, 1, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
};

static const int MONSTER_GRID_BITINDEX1[48] = {
	1, 1, 1, 2, 3, 3, 3, 1, 1, 1, 2, 3,
	3, 3, 1, 1, 1, 2, 3, 3, 3, 1, 1, 1,
	0, 3, 3, 3, 1, 1, 1, 0, 3, 3, 3, 1,
	1, 1, 0, 3, 3, 3, 1, 1, 1, 0, 3, 3
};

static const int MONSTER_GRID_BITINDEX2[48] = {
	2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
	2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1,
	0, 3, 3, 3, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static const int MONSTER_GRID_BITMASK[12] = {
	0xC, 8, 4, 0, 0xF, 0xF000, 0xF00, 0xF0, 0xF00, 0xF0, 0x0F, 0xF000
};

static const int ATTACK_TYPE_FX[23] = {
	49, 18, 13, 14, 15, 17, 16, 0, 6, 1, 2, 3,
	4, 5, 4, 9, 27, 29, 44, 51, 53, 61, 71
};

static const int MONSTER_SHOOT_POW[7] = { 12, 14, 0, 4, 8, 10, 13 };

static const int COMBAT_SHOOTING[4] = { 1, 1, 2, 3 };

/*------------------------------------------------------------------------*/

Combat::Combat(XeenEngine *vm): _vm(vm) {
	Common::fill(&_attackMonsters[0], &_attackMonsters[26], 0);
	Common::fill(&_charsArray1[0], &_charsArray1[12], 0);
	Common::fill(&_monPow[0], &_monPow[12], 0);
	Common::fill(&_monsterScale[0], &_monsterScale[12], 0);
	Common::fill(&_elemPow[0], &_elemPow[12], 0);
	Common::fill(&_elemScale[0], &_elemScale[12], 0);
	Common::fill(&_shooting[0], &_shooting[8], 0);
	Common::fill(&_monsterMap[0][0], &_monsterMap[32][32], 0);
	Common::fill(&_monsterMoved[0], &_monsterMoved[MAX_NUM_MONSTERS], false);
	Common::fill(&_rangeAttacking[0], &_rangeAttacking[MAX_NUM_MONSTERS], false);
	Common::fill(&_gmonHit[0], &_gmonHit[36], 0);
	_globalCombat = 0;
	_whosTurn = -1;
	_itemFlag = false;
	_monstersAttacking = false;
	_combatMode = 0;
}

void Combat::clear() {
	Common::fill(&_attackMonsters[0], &_attackMonsters[26], -1);
}

void Combat::giveCharDamage(int damage, DamageType attackType, int charIndex) {
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	Scripts &scripts = *_vm->_scripts;
	SoundManager &sound = *_vm->_sound;
	int charIndex1 = charIndex + 1;
	int selectedIndex1 = 0;
	int selectedIndex2 = 0;
	bool breakFlag = false;

	screen.closeWindows();
	
	int idx = (int)party._activeParty.size();
	if (!scripts._v2) {
		for (idx = 0; idx < (int)party._activeParty.size(); ++idx) {
			Character &c = party._activeParty[idx];
			Condition condition = c.worstCondition();

			if (!(condition >= UNCONSCIOUS && condition <= ERADICATED)) {
				if (!selectedIndex1) {
					selectedIndex1 = idx + 1;
				} else {
					selectedIndex2 = idx + 1;
					break;
				}
			}
		}
	}
	if (idx == (int)party._activeParty.size()) {
		selectedIndex1 = scripts._v2 ? charIndex : 0;
		goto loop;
	}

	for (;;) {
		// The if below is to get around errors due to the
		// goto I was forced to use when reimplementing this method
		if (true) {
			Character &c = party._activeParty[selectedIndex1];
			c._conditions[ASLEEP] = 0;	// Force character to be awake

			int frame = 0, fx = 0;
			switch (attackType) {
			case DT_PHYSICAL:
				fx = 29;
				break;
			case DT_MAGICAL:
				frame = 6;
				fx = 27;
				break;
			case DT_FIRE:
				damage -= party._fireResistence;
				frame = 1;
				fx = 22;
				break;
			case DT_ELECTRICAL:
				damage -= party._electricityResistence;
				frame = 2;
				fx = 23;
				break;
			case DT_COLD:
				damage -= party._coldResistence;
				frame = 3;
				fx = 24;
				break;
			case DT_POISON:
				damage -= party._poisonResistence;
				frame = 4;
				fx = 26;
				break;
			case DT_ENERGY:
				frame = 5;
				fx = 25;
				break;
			case DT_SLEEP:
				fx = 38;
				break;
			default:
				break;
			}

			// All attack types other than physical allow for saving
			// throws to reduce the damage
			if (attackType != DT_PHYSICAL) {
				while (c.charSavingThrow(attackType) && damage > 0)
					damage /= 2;
			}

			// Draw the attack effect on the character sprite
			sound.playFX(fx);
			_powSprites.draw(screen, frame,
				Common::Point(CHAR_FACES_X[selectedIndex1], 150));
			screen._windows[33].update();

			// Reduce damage if power shield active, and set it zero
			// if the damage amount has become negative.. you wouldn't
			// want attacks healing the characters
			if (party._powerShield)
				damage -= party._powerShield;
			if (damage < 0)
				damage = 0;

			// TODO: This seems weird.. maybe I've got attack types wrong..
			// why should attack type 7 (DT_SLEEP) set the dead condition?
			if (attackType == DT_SLEEP) {
				damage = c._currentHp;
				c._conditions[DEAD] = 1;
			}

			// Subtract the hit points from the character
			c.subtractHitPoints(damage);
		}

		if (selectedIndex2) {
			++selectedIndex1;
loop:
			if ((scripts._v2 ? charIndex1 : (int)party._activeParty.size()) > selectedIndex1)
				break;
		}

		// Break check and if not, move to other index
		if (!selectedIndex2 || breakFlag)
			break;

		selectedIndex1 = selectedIndex2 - 1;
		breakFlag = true;
	}
}

void Combat::moveMonsters() {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	if (!_vm->_moveMonsters)
		return;

	intf._tillMove = 0;
	if (intf._charsShooting)
		return;

	Common::fill(&_monsterMap[0][0], &_monsterMap[32][32], 0);
	Common::fill(&_monsterMoved[0], &_monsterMoved[MAX_NUM_MONSTERS], false);
	Common::fill(&_rangeAttacking[0], &_rangeAttacking[MAX_NUM_MONSTERS], false);
	Common::fill(&_gmonHit[0], &_gmonHit[36], -1);
	_vm->_dangerSenseAllowed = false;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];
		if (monster._position.y < 32) {
			_monsterMap[monster._position.y][monster._position.x]++;
		}
	}

	for (int loopNum = 0; loopNum < 2; ++loopNum) {
		int arrIndex = -1;
		for (int yDiff = 3; yDiff >= -3; --yDiff) {
			for (int xDiff = 3; xDiff >= -3; --xDiff) {
				Common::Point pt = party._mazePosition + Common::Point(xDiff, yDiff);
				++arrIndex;

				for (int idx = 0; idx < (int)map._mobData._monsters.size(); ++idx) {
					MazeMonster &monster = map._mobData._monsters[idx];
					MonsterStruct &monsterData = map._monsterData[monster._spriteId];

					if (pt == monster._position) {
						_vm->_dangerSenseAllowed = true;
						if ((monster._isAttacking || _vm->_mode == MODE_SLEEPING)
								&& !_monsterMoved[idx]) {
							if (party._mazePosition.x == pt.x || party._mazePosition.y == pt.y) {
								// Check for range attacks
								if (monsterData._rangeAttack && !_rangeAttacking[idx]
									&& _attackMonsters[0] != idx && _attackMonsters[1] != idx
									&& _attackMonsters[2] != idx && !monster._field7) {
									// Setup monster for attacking
									setupMonsterAttack(monster._spriteId, pt);
									_rangeAttacking[idx] = true;
								}
							}
						}
					}

					switch (party._mazeDirection) {
					case DIR_NORTH:
					case DIR_SOUTH:
						if (monsterCanMove(pt, MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX1[arrIndex]],
								MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex], idx)) {
							// Move the monster
							moveMonster(idx, Common::Point(MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex]));
						} else {
							if (monsterCanMove(pt, MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX2[arrIndex]],
								arrIndex >= 21 && arrIndex <= 27 ? MONSTER_GRID3[arrIndex] : 0,
								arrIndex >= 21 && arrIndex <= 27 ? 0 : MONSTER_GRID3[arrIndex],
								idx))
							if (arrIndex >= 21 && arrIndex <= 27) {
								moveMonster(idx, Common::Point(MONSTER_GRID3[arrIndex], 0));
							} else {
								moveMonster(idx, Common::Point(0, MONSTER_GRID3[arrIndex]));
							}
						}
						break;

					case DIR_EAST:
					case DIR_WEST:
						if (monsterCanMove(pt, MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX2[arrIndex]],
							arrIndex >= 21 && arrIndex <= 27 ? MONSTER_GRID3[arrIndex] : 0,
							arrIndex >= 21 && arrIndex <= 27 ? 0 : MONSTER_GRID3[arrIndex],
							idx)) {
							if (arrIndex >= 21 && arrIndex <= 27) {
								moveMonster(idx, Common::Point(MONSTER_GRID3[arrIndex], 0));
							} else {
								moveMonster(idx, Common::Point(0, MONSTER_GRID3[arrIndex]));
							}
						} else if (monsterCanMove(pt, MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX1[arrIndex]],
								MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex], idx)) {
							moveMonster(idx, Common::Point(MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex]));
						}
					}
				}
			}
		}
	}

	monsterOvercome();
	if (_monstersAttacking)
		monstersAttack();
}

void Combat::monstersAttack() {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;
	int powNum = -1;
	MonsterStruct *monsterData = nullptr;
	OutdoorDrawList &outdoorList = intf._outdoorList;
	IndoorDrawList &indoorList = intf._indoorList;

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] != -1) {
			monsterData = &map._monsterData[_gmonHit[idx]];
			powNum = MONSTER_SHOOT_POW[monsterData->_attackType];
			if (powNum != 12)
				break;
		}
	}

	_powSprites.load(Common::String::format("pow%d.icn", powNum));
	sound.playFX(ATTACK_TYPE_FX[monsterData->_attackType]);

	for (int charNum = 0; charNum < MAX_PARTY_COUNT; ++charNum) {
		if (!_shooting[charNum])
			continue;

		if (map._isOutdoors) {
			outdoorList._attackImgs1[charNum]._scale = 3;
			outdoorList._attackImgs2[charNum]._scale = 7;
			outdoorList._attackImgs3[charNum]._scale = 11;
			outdoorList._attackImgs4[charNum]._scale = 15;
			outdoorList._attackImgs1[charNum]._sprites = nullptr;
			outdoorList._attackImgs2[charNum]._sprites = nullptr;
			outdoorList._attackImgs3[charNum]._sprites = nullptr;
			outdoorList._attackImgs4[charNum]._sprites = nullptr;

			switch (_shooting[charNum]) {
			case 1:
				outdoorList._attackImgs1[charNum]._sprites = &_powSprites;
				break;
			case 2:
				outdoorList._attackImgs2[charNum]._sprites = &_powSprites;
				break;
			default:
				outdoorList._attackImgs3[charNum]._sprites = &_powSprites;
				break;
			}
		} else {
			indoorList._attackImgs1[charNum]._scale = 3;
			indoorList._attackImgs2[charNum]._scale = 7;
			indoorList._attackImgs3[charNum]._scale = 11;
			indoorList._attackImgs4[charNum]._scale = 15;
			indoorList._attackImgs1[charNum]._sprites = nullptr;
			indoorList._attackImgs2[charNum]._sprites = nullptr;
			indoorList._attackImgs3[charNum]._sprites = nullptr;
			indoorList._attackImgs4[charNum]._sprites = nullptr;

			switch (_shooting[charNum]) {
			case 1:
				indoorList._attackImgs1[charNum]._sprites = &_powSprites;
				break;
			case 2:
				indoorList._attackImgs2[charNum]._sprites = &_powSprites;
				break;
			default:
				indoorList._attackImgs3[charNum]._sprites = &_powSprites;
				break;
			}
		}
	}

	// Wait whilst the attacking effect is done
	do {
		intf.draw3d(true);
		events.pollEventsAndWait();
	} while (!_vm->shouldQuit() && intf._isAttacking);
	
	endAttack();

	if (_vm->_mode != MODE_COMBAT) {
		// Combat wasn't previously active, but it is now. Set up
		// the combat party from the currently active party
		_combatParty.clear();
		for (uint idx = 0; idx < party._activeParty.size(); ++idx)
			_combatParty.push_back(&party._activeParty[idx]);
	}

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] != -1)
			attackMonster(_gmonHit[idx]);
	}

	_monstersAttacking = false;

	if (_vm->_mode != MODE_SLEEPING) {
		for (uint charNum = 0; charNum < party._activeParty.size(); ++charNum) {
			Condition condition = party._activeParty[charNum].worstCondition();

			if (condition != ASLEEP && (condition < PARALYZED || condition == NO_CONDITION)) {
				_vm->_mode = MODE_1;
				break;
			}
		}
	}
}

void Combat::setupMonsterAttack(int monsterDataIndex, const Common::Point &pt) {
	Party &party = *_vm->_party;

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] != -1) {
			int result = stopAttack(pt - party._mazePosition);
			if (result) {	
				_monstersAttacking = true;
				_gmonHit[idx] = monsterDataIndex;

				if (result != 1) {
					for (int charNum = 0; charNum < MAX_PARTY_COUNT; ++charNum) {
						if (!_shooting[charNum]) {
							_shooting[charNum] = COMBAT_SHOOTING[result - 1];
						}
					}
				}
			}
		}
	}
}

bool Combat::monsterCanMove(const Common::Point &pt, int wallShift,
		int xDiff, int yDiff, int monsterId) {
	Map &map = *_vm->_map;
	MazeMonster &monster = map._mobData._monsters[monsterId];
	MonsterStruct &monsterData = map._monsterData[monster._spriteId];

	Common::Point tempPos = pt;
	if (map._isOutdoors) {
		tempPos += Common::Point(xDiff, yDiff);
		wallShift = 4;
	}
	int v = map.mazeLookup(tempPos, wallShift);

	if (!map._isOutdoors) {
		return v <= map.mazeData()._difficulties._wallNoPass;
	} else {
		SurfaceType surfaceType;
		switch (v) {
		case 0:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 8:
		case 11:
		case 13:
		case 14:
			surfaceType = (SurfaceType)map.mazeData()._surfaceTypes[map._currentSurfaceId];
			if (surfaceType == SURFTYPE_WATER || surfaceType == SURFTYPE_DWATER) {
				return monsterData._flying || monster._spriteId == 59;
			} else if (surfaceType == SURFTYPE_SPACE) {
				return monsterData._flying;
			} else {
				return _vm->_files->_isDarkCc || monster._spriteId != 59;
			}
		default:
			return v <= map.mazeData()._difficulties._wallNoPass;
		}
	}
}

void Combat::moveMonster(int monsterId, const Common::Point &pt) {
	Map &map = *_vm->_map;
	MazeMonster &monster = map._mobData._monsters[monsterId];

	if (_monsterMap[pt.y][pt.x] < 3 && !monster._field7 && _vm->_moveMonsters) {
		++_monsterMap[pt.y][pt.x];
		--_monsterMap[monster._position.y][monster._position.x];
		monster._position = pt;
		_monsterMoved[monsterId] = true;
	}
}

void Combat::endAttack() {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	intf._isAttacking = false;
	IndoorDrawList &indoorList = intf._indoorList;
	OutdoorDrawList &outdoorList = intf._outdoorList;

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		if (map._isOutdoors) {
			outdoorList._attackImgs1[idx]._scale = 0;
			outdoorList._attackImgs2[idx]._scale = 0;
			outdoorList._attackImgs3[idx]._scale = 0;
			outdoorList._attackImgs4[idx]._scale = 0;
			outdoorList._attackImgs1[idx]._sprites = nullptr;
			outdoorList._attackImgs2[idx]._sprites = nullptr;
			outdoorList._attackImgs3[idx]._sprites = nullptr;
			outdoorList._attackImgs4[idx]._sprites = nullptr;
		} else {
			indoorList._attackImgs1[idx]._scale = 0;
			indoorList._attackImgs2[idx]._scale = 0;
			indoorList._attackImgs3[idx]._scale = 0;
			indoorList._attackImgs4[idx]._scale = 0;
			indoorList._attackImgs1[idx]._sprites = nullptr;
			indoorList._attackImgs2[idx]._sprites = nullptr;
			indoorList._attackImgs3[idx]._sprites = nullptr;
			indoorList._attackImgs4[idx]._sprites = nullptr;
		}
	}

	Common::fill(&_shooting[0], &_shooting[MAX_PARTY_COUNT], false);
}

void Combat::monsterOvercome() {
	Map &map = *_vm->_map;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];
		int dataIndex = monster._spriteId;

		if (monster._field7 != 0 && monster._field7 != 13) {
			// Do a saving throw for monster
			if (dataIndex <= _vm->getRandomNumber(1, dataIndex + 50))
				monster._field7 = 0;
		}
	}
}

void Combat::attackMonster(int monsterId) {

}

bool Combat::stopAttack(const Common::Point &diffPt) {
	Map &map = *_vm->_map;

	if (map._isOutdoors) {

	}

	// TODO
	return false;
}

} // End of namespace Xeen
