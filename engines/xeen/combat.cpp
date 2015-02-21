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
	_monsterIndex = 0;
	_partyRan = false;
	_monster2Attack = -1;
	_whosSpeed = 0;
	_damageType = DT_PHYSICAL;
	_oldCharacter = nullptr;
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

/**
 * Do damage to a specific character
 */
void Combat::doCharDamage(Character &c, int charNum, int monsterDataIndex) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Screen &screen = *_vm->_screen;
	SoundManager &sound = *_vm->_sound;
	MonsterStruct &monsterData = map._monsterData[monsterDataIndex];

	// Attacked characters are automatically woken up
	c._conditions[ASLEEP] = 0;

	// Figure out the damage amount
	int damage = 0;
	for (int idx = 0; idx < monsterData._strikes; ++idx)
		damage += _vm->getRandomNumber(1, monsterData._dmgPerStrike);


	int fx = 29, frame = 0;
	if (monsterData._attackType) {
		if (c.charSavingThrow(monsterData._attackType))
			damage /= 2;

		switch (monsterData._attackType) {
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
		default:
			break;
		}

		while (damage > 0 && c.charSavingThrow(monsterData._attackType))
			damage /= 2;
	}

	sound.playFX(fx);
	intf._charPowSprites.draw(screen, frame, Common::Point(CHAR_FACES_X[charNum], 150));
	screen._windows[33].update();

	damage -= party._powerShield;
	if (damage > 0 && monsterData._specialAttack && !c.charSavingThrow(DT_PHYSICAL)) {
		switch (monsterData._specialAttack) {
		case SA_POISON:
			if (!++c._conditions[POISONED])
				c._conditions[POISONED] = -1;
			sound.playFX(26);
			break;
		case SA_DISEASE:
			if (!++c._conditions[DISEASED])
				c._conditions[DISEASED] = -1;
			sound.playFX(26);
			break;
		case SA_INSANE:
			if (!++c._conditions[INSANE])
				c._conditions[INSANE] = -1;
			sound.playFX(28);
			break;
		case SA_SLEEP:
			if (!++c._conditions[ASLEEP])
				c._conditions[ASLEEP] = -1;
			sound.playFX(36);
			break;
		case SA_CURSEITEM:
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				if (c._weapons[idx]._id != 34)
					c._weapons[idx]._bonusFlags |= ITEMFLAG_CURSED;
				c._armor[idx]._bonusFlags |= ITEMFLAG_CURSED;
				c._accessories[idx]._bonusFlags |= ITEMFLAG_CURSED;
				c._misc[idx]._bonusFlags |= ITEMFLAG_CURSED;;
			}
			sound.playFX(37);
			break;
		case SA_DRAINSP:
			c._currentSp = 0;
			sound.playFX(37);
			break;
		case SA_CURSE:
			if (!++c._conditions[CURSED])
				c._conditions[CURSED] = -1;
			sound.playFX(37);
			break;
		case SA_PARALYZE:
			if (!++c._conditions[PARALYZED])
				c._conditions[PARALYZED] = -1;
			sound.playFX(37);
			break;
		case SA_UNCONSCIOUS:
			if (!++c._conditions[UNCONSCIOUS])
				c._conditions[UNCONSCIOUS] = -1;
			sound.playFX(37);
			break;
		case SA_CONFUSE:
			if (!++c._conditions[CONFUSED])
				c._conditions[CONFUSED] = -1;
			sound.playFX(28);
			break;
		case SA_BREAKWEAPON:
			for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
				XeenItem &weapon = c._weapons[idx];
				if (weapon._id != 34 && weapon._id != 0 && weapon._frame != 0) {
					weapon._bonusFlags |= ITEMFLAG_BROKEN;
					weapon._frame = 0;
				}
			}
			sound.playFX(37);
			break;
		case SA_WEAKEN:
			if (!++c._conditions[WEAK])
				c._conditions[WEAK] = -1;
			sound.playFX(36);
			break;
		case SA_ERADICATE:
			if (!++c._conditions[ERADICATED])
				c._conditions[ERADICATED] = -1;
			c._items.breakAllItems();
			sound.playFX(37);

			if (c._currentHp > 0)
				c._currentHp = 0;
			break;
		case SA_AGING:
			++c._tempAge;
			sound.playFX(37);
			break;
		case SA_DEATH:
			if (!++c._conditions[DEAD])
				c._conditions[DEAD] = -1;
			sound.playFX(38);
			if (c._currentHp > 0)
				c._currentHp = 0;
			break;
		case SA_STONE:
			if (!++c._conditions[STONED])
				c._conditions[STONED] = -1;
			sound.playFX(38);
			if (c._currentHp > 0)
				c._currentHp = 0;
			break;
		}

		c.subtractHitPoints(damage);
	}

	events.ipause(2);
	intf.drawParty(true);
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
		setupCombatParty();
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
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	SoundManager &sound = *_vm->_sound;

	if (_monstersAttacking) {
		warning("TODO: Original used uninitialized variables if flag was set");
		return;
	}

	int monsterIndex;
	switch (_whosTurn - _combatParty.size()) {
	case 0:
		monsterIndex = _attackMonsters[0];
		intf._indoorList[156]._scale = 0;
		break;
	case 1:
		monsterIndex = _attackMonsters[1];
		intf._indoorList[150]._scale = 0;
		break;
	case 2:
	default:
		monsterIndex = _attackMonsters[2];
		intf._indoorList[153]._scale = 0;
	} 

	MazeMonster &monster = map._mobData._monsters[monsterIndex];
	MonsterStruct &monsterData = map._monsterData[monster._spriteId];
	if (monster._field7)
		return;

	monster._frame = 8;
	monster._fieldA = 3;
	monster._field9 = 0;
	intf.draw3d(true);
	intf.draw3d(true);

	File f(monsterData._attackVoc);
	sound.playSample(&f, 0);
	bool flag = false;

	for (int attackNum = 0; attackNum < monsterData._numberOfAttacks; ++attackNum) {
		int charNum = -1;
		bool isHated = false;

		if (monsterData._hatesClass != -1) {
			if (monsterData._hatesClass == 15)
				// Monster hates all classes
				goto loop;

			for (uint charIndex = 0; charIndex < _combatParty.size(); ++charIndex) {
				Character &c = *_combatParty[charIndex];
				Condition cond = c.worstCondition();
				if (cond >= PARALYZED && cond <= ERADICATED)
					continue;

				bool isHated = false;
				switch (monsterData._hatesClass) {
				case CLASS_KNIGHT:
				case CLASS_PALADIN:
				case CLASS_ARCHER:
				case CLASS_CLERIC:
				case CLASS_SORCERER:
				case CLASS_ROBBER:
				case CLASS_NINJA:
				case CLASS_BARBARIAN:
				case CLASS_DRUID:
				case CLASS_RANGER:
					isHated = c._class == monsterData._hatesClass;
					break;
				case 12:
					isHated = c._race == DWARF;
					break;
				default:
					break;
				}

				if (isHated) {
					charNum = charIndex;
					break;
				}
			}
		}

		if (!isHated) {
			// No particularly hated foe, so decide which character to start with
			switch (_combatParty.size()) {
			case 1:
				charNum = 0;
				break;
			case 2:
			case 3:
			case 4:
			case 5:
				charNum = _vm->getRandomNumber(0, _combatParty.size() - 1);
				break;
			case 6:
				if (_vm->getRandomNumber(1, 6) == 6)
					charNum = 5;
				else
					charNum = _vm->getRandomNumber(0, 4);
				break;
			}
		}

		// Attacking loop
		do {
			if (!flag) {
				Condition cond = _combatParty[charNum]->worstCondition();

				if (cond >= PARALYZED && cond <= ERADICATED) {
					Common::Array<int> ableChars;
					bool skip = false;
					
					for (uint idx = 0; idx < _combatParty.size() && !skip; ++idx) {
						switch (_combatParty[idx]->worstCondition()) {
						case PARALYZED:
						case UNCONSCIOUS:
							if (flag)
								skip = true;
							break;
						case DEAD:
						case STONED:
						case ERADICATED:
							break;
						default:
							ableChars.push_back(idx);
							break;
						}
					}

					if (!skip) {
						if (ableChars.size() == 0) {
							party._dead = true;
							_vm->_mode = MODE_1;
							return;
						}

						charNum = ableChars[_vm->getRandomNumber(0, ableChars.size() - 1)];
					}
				}
			}

			// Unconditional if to get around goto initialization errors
			if (true) {
				Character &c = *_combatParty[charNum];
				if (monsterData._attackType != DT_PHYSICAL || c._conditions[ASLEEP]) {
					doCharDamage(c, charNum, monster._spriteId);
				} else {
					int v = _vm->getRandomNumber(1, 20);
					if (v == 1) {
						sound.playFX(6);
					} else {
						if (v == 20)
							doCharDamage(c, charNum, monster._spriteId);
						v += monsterData._hitChance / 4 + _vm->getRandomNumber(1,
							monsterData._hitChance);

						int ac = c.getArmorClass() + (!_charsBlocked[charNum] ? 10 :
							c.getCurrentLevel() / 2 + 15);
						if (ac > v) {
							sound.playFX(6);
						} else {
							doCharDamage(c, charNum, monster._spriteId);
						}
					}
				}

				if (flag)
					break;
			}
loop:
			flag = true;
		} while (++charNum < (int)_combatParty.size());
	}

	intf.drawParty(true);
}

int Combat::stopAttack(const Common::Point &diffPt) {
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Direction dir = party._mazeDirection;
	const Common::Point &mazePos = party._mazePosition;

	if (map._isOutdoors) {
		if (diffPt.x > 0) {
			for (int x = 1; x <= diffPt.x; ++x) {
				int v = map.mazeLookup(Common::Point(mazePos.x + x, mazePos.y), 0, 8);
				if (v)
					return 0;
			}
			return (dir == DIR_EAST) ? diffPt.x + 1 : 1;

		} else if (diffPt.x < 0) {
			for (int x = diffPt.x; x < 0; ++x) {
				int v = map.mazeLookup(Common::Point(mazePos.x + x, mazePos.y), 4);
				switch (v) {
				case 0:
				case 2:
				case 4:
				case 5:
				case 8:
				case 11:
				case 13:
				case 14:
					break;
				default:
					return 0;
				}
			}
			return dir == DIR_WEST ? diffPt.x * -1 + 1 : 1;
		
		} else if (diffPt.y <= 0) {
			for (int y = diffPt.y; y < 0; ++y) {
				int v = map.mazeLookup(Common::Point(mazePos.x, mazePos.y + y), 4);
				switch (v) {
				case 0:
				case 2:
				case 4:
				case 5:
				case 8:
				case 11:
				case 13:
				case 14:
					break;
				default:
					return 0;
				}
			}
			return party._mazeDirection == DIR_SOUTH ? diffPt.y * -1 + 1 : 1;

		} else {
			for (int y = 1; y <= diffPt.y; ++y) {
				int v = map.mazeLookup(Common::Point(mazePos.x, mazePos.y + y), 4);
				switch (v) {
				case 0:
				case 2:
				case 4:
				case 5:
				case 8:
				case 11:
				case 13:
				case 14:
					break;
				default:
					return 0;
				}
			}
			return dir == DIR_NORTH ? diffPt.y + 1 : 1;
		}
	} else {
		// Indoors
		if (diffPt.x > 0) {
			for (int x = 1; x <= diffPt.x; ++x) {
				int v = map.mazeLookup(Common::Point(mazePos.x + x, mazePos.y), 0, 8);
				if (v)
					return 0;
			}
			return dir == DIR_EAST ? diffPt.x + 1 : 1;

		} else if (diffPt.x < 0) {
			for (int x = diffPt.x; x < 0; ++x) {
				int v = map.mazeLookup(Common::Point(mazePos.x + x, mazePos.y), 0, 0x800);
				if (v)
					return 0;
			}
			return dir == DIR_WEST ? diffPt.x * -1 + 1 : 1;
		
		} else if (diffPt.y <= 0) {
			for (int y = diffPt.y; y < 0; ++y) {
				int v = map.mazeLookup(Common::Point(mazePos.x, mazePos.y + y), 0, 0x8000);
				if (v)
					return 0;
			}
			return dir == DIR_SOUTH ? diffPt.y * -1 + 1 : 1;

		} else {
			for (int y = 1; y <= diffPt.y; ++y) {
				int v = map.mazeLookup(Common::Point(mazePos.x, mazePos.y + y), 0, 0x80);
				if (v)
					return 0;
			}
			return dir == DIR_NORTH ? diffPt.y + 1 : 1;
		}
	}
}

/**
 * Setup the combat party with a copy of the currently active party
 */
void Combat::setupCombatParty() {
	Party &party = *_vm->_party;

	_combatParty.clear();
	for (uint idx = 0; idx < party._activeParty.size(); ++idx)
		_combatParty.push_back(&party._activeParty[idx]);
}

void Combat::setSpeedTable() {
	Map &map = *_vm->_map;
	Common::Array<int> charSpeeds;
	bool flag = _whosSpeed != -1;
	int oldSpeed = (_whosSpeed == -1) ? 0 : _speedTable[_whosSpeed];

	Common::fill(&_speedTable[0], &_speedTable[12], -1);
	Common::fill(&charSpeeds[0], &charSpeeds[12], -1);

	// Set up speeds for party membres
	int maxSpeed = 0;
	for (uint charNum = 0; charNum < _combatParty.size(); ++charNum) {
		Character &c = *_combatParty[charNum];
		charSpeeds.push_back(c.getStat(SPEED));

		maxSpeed = MAX(charSpeeds[charNum], maxSpeed);
	}

	// Add in speeds of attacking monsters
	for (int monsterNum = 0; monsterNum < 3; ++monsterNum) {
		if (_attackMonsters[monsterNum] != -1) {
			MazeMonster &monster = map._mobData._monsters[_attackMonsters[monsterNum]];
			MonsterStruct &monsterData = map._monsterData[monster._spriteId];
			charSpeeds.push_back(monsterData._speed);

			maxSpeed = MAX(maxSpeed, monsterData._speed);
		} else {
			charSpeeds.push_back(0);
		}
	}

	_speedTable.clear();
	for (; maxSpeed >= 0; --maxSpeed) {
		for (uint idx = 0; idx < charSpeeds.size(); ++idx) {
			if (charSpeeds[idx] == maxSpeed)
				_speedTable.push_back(idx);
		}
	}

	if (flag) {
		if (_speedTable[_whosSpeed] != oldSpeed) {
			for (uint idx = 0; idx < charSpeeds.size(); ++idx) {
				if (oldSpeed == _speedTable[idx]) {
					_whosSpeed = idx;
					break;
				}
			}
		}
	}
}

/**
 * Returns true if all participants in the combat are disabled
 */
bool Combat::allHaveGone() const {
	for (uint idx = 0; idx < _charsGone.size(); ++idx) {
		if (!_charsGone[idx]) {
			if (idx >= _combatParty.size()) {
				return false;
			} else {
				Condition condition = _combatParty[idx]->worstCondition();
				if (condition < PARALYZED || condition == NO_CONDITION)
					return false;
			}
		}
	}

	return true;
}

/**
 * Returns true if all the characters of the party are disabled
 */
bool Combat::charsCantAct() const {
	for (uint idx = 0; idx < _combatParty.size(); ++idx) {
		if (!_combatParty[idx]->isDisabledOrDead())
			return false;
	}

	return true;
}

/**
 * Return a description of the monsters being faced
 */
Common::String Combat::getMonsterDescriptions() {
	Map &map = *_vm->_map;
	Common::String lines[3];

	// Get names of monsters attacking, if any
	for (int idx = 0; idx < 3; ++idx) {
		if (_attackMonsters[idx] != -1) {
			MazeMonster &monster = map._mobData._monsters[_attackMonsters[idx]];
			MonsterStruct &monsterData = map._monsterData[monster._spriteId];

			Common::String format = "\n\v020\f%2u%s\fd";
			format.setChar('2' + idx, 3);
			lines[idx] = Common::String::format(format.c_str(), monsterData._name.c_str());
		}
	}

	if (_monsterIndex == 2 && _attackMonsters[2] != -1) {
		_monster2Attack = _attackMonsters[2];
	} if (_monsterIndex == 1 && _attackMonsters[1] != -1) {
		_monster2Attack = _attackMonsters[1];
	} else {
		_monster2Attack = _attackMonsters[0];
		_monsterIndex = 0;
	}

	return Common::String::format(COMBAT_DETAILS, lines[0].c_str(),
		lines[1].c_str(), lines[2].c_str());
}

void Combat::attack(Character &c, int v2) {
	error("TODO");
}

/**
 * Flag the currently active character as blocking/defending
 */
void Combat::block() {
	_charsBlocked[_whosTurn] = true;
}

/**
 * Perform whatever the current combat character's quick action is
 */
void Combat::quickFight() {
	Spells &spells = *_vm->_spells;
	Character *c = _combatParty[_whosTurn];

	switch (c->_quickOption) {
	case QUICK_ATTACK:
		attack(*c, 0);
		break;
	case QUICK_SPELL:
		if (c->_currentSpell != -1) {
			spells.castSpell(c, SPELLS_ALLOWED[c->getClassCategory()][c->_currentSpell]);
		}
		break;
	case QUICK_BLOCK:
		block();
		break;
	case QUICK_RUN:
		run();
		break;
	default:
		break;
	}
}

void Combat::giveTreasure() {
	error("TODO: giveTreasure");
}

/**
 * Current selected character is trying to run away
 */
void Combat::run() {
	Map &map = *_vm->_map;
	SoundManager &sound = *_vm->_sound;

	if (_vm->getRandomNumber(1, 100) < map.mazeData()._difficulties._chance2Run) {
		// Remove the character from the combat party
		_combatParty.remove_at(_whosTurn);
		setSpeedTable();
		--_whosSpeed;
		_whosTurn = -1;
		_partyRan = true;
		sound.playFX(51);
	}
}

} // End of namespace Xeen
