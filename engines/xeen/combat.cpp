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

static const int ATTACK_TYPE_FX[23] = {
	49, 18, 13, 14, 15, 17, 16, 0, 6, 1, 2, 3,
	4, 5, 4, 9, 27, 29, 44, 51, 53, 61, 71
};

static const PowType MONSTER_SHOOT_POW[7] = { 
	POW_MAGIC_ARROW, POW_SPARKLES, POW_FIREBALL,
	POW_MEGAVOLTS, POW_COLD_RAY, POW_SPRAY, POW_ENERGY_BLAST
};

static const int COMBAT_SHOOTING[4] = { 1, 1, 2, 3 };

static const int DAMAGE_TYPE_EFFECTS[19] = {
	3, 10, 4, 11, 1, 2, 5, 9, 5, 14, 5, 14, 10, 8, 3, 9, 2, 2, 3
};

static const int POW_WEAPON_VOCS[35] = {
	0, 5, 4, 5, 5, 5, 5, 2, 4, 5, 3, 5, 4, 2, 3, 2, 2, 4, 5, 5,
	5, 5, 5, 1, 3, 2, 5, 1, 1, 1, 0, 0, 0, 2, 2
};

static const int MONSTER_ITEM_RANGES[6] = { 10, 20, 50, 100, 100, 100 };

#define monsterSavingThrow(MONINDEX) (_vm->getRandomNumber(1, 50 + (MONINDEX)) <= (MONINDEX))

/*------------------------------------------------------------------------*/

Combat::Combat(XeenEngine *vm): _vm(vm), _missVoc("miss.voc") {
	Common::fill(&_attackMonsters[0], &_attackMonsters[26], 0);
	Common::fill(&_shootingRow[0], &_shootingRow[MAX_PARTY_COUNT], 0);
	Common::fill(&_monsterMap[0][0], &_monsterMap[32][32], 0);
	Common::fill(&_monsterMoved[0], &_monsterMoved[MAX_NUM_MONSTERS], false);
	Common::fill(&_rangeAttacking[0], &_rangeAttacking[MAX_NUM_MONSTERS], false);
	Common::fill(&_gmonHit[0], &_gmonHit[36], 0);
	Common::fill(&_missedShot[0], &_missedShot[MAX_PARTY_COUNT], 0);
	_globalCombat = 0;
	_whosTurn = -1;
	_itemFlag = false;
	_monstersAttacking = false;
	_combatMode = COMBATMODE_STARTUP;
	_attackDurationCtr = 0;
	_partyRan = false;
	_monster2Attack = -1;
	_whosSpeed = 0;
	_damageType = DT_PHYSICAL;
	_oldCharacter = nullptr;
	_shootType = ST_0;
	_monsterDamage = 0;
	_weaponDamage = 0;
	_weaponDie = _weaponDice = 0;
	_weaponElemMaterial = 0;
	_attackWeapon = nullptr;
	_attackWeaponId = 0;
	_hitChanceBonus = 0;
	_dangerPresent = false;
	_moveMonsters = false;
	_rangeType = RT_SINGLE;
	_combatTarget = 0;
}

void Combat::clearAttackers() {
	Common::fill(&_attackMonsters[0], &_attackMonsters[ATTACK_MONSTERS_COUNT], -1);
}

void Combat::clearBlocked() {
	Common::fill(_charsBlocked, _charsBlocked + PARTY_AND_MONSTERS, false);
}

void Combat::clearShooting() {
	Common::fill(_shootingRow, _shootingRow + MAX_PARTY_COUNT, 0);
}

void Combat::giveCharDamage(int damage, DamageType attackType, int charIndex) {
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	int endIndex = charIndex + 1;
	int selectedIndex = 0;
	bool breakFlag = false;

	windows.closeAll();

	int idx = (int)party._activeParty.size();
	if (_combatTarget == 2) {
		for (idx = 0; idx < (int)party._activeParty.size(); ++idx) {
			Character &c = party._activeParty[idx];
			Condition condition = c.worstCondition();

			if (!(condition >= UNCONSCIOUS && condition <= ERADICATED)) {
				if (!charIndex) {
					charIndex = idx + 1;
				} else {
					selectedIndex = idx + 1;
					--charIndex;
					break;
				}
			}
		}
	}
	if (idx == (int)party._activeParty.size()) {
		if (!_combatTarget)
			charIndex = 0;
	}

	for (;;) {
		for (; charIndex < (_combatTarget ? endIndex : (int)party._activeParty.size()); ++charIndex) {
			Character &c = party._activeParty[charIndex];
			c._conditions[ASLEEP] = 0;	// Force attacked character to be awake

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
			intf._charPowSprites.draw(0, frame, Common::Point(Res.CHAR_FACES_X[charIndex], 150));
			windows[33].update();

			// Reduce damage if power shield active, and set it zero
			// if the damage amount has become negative.. you wouldn't
			// want attacks healing the characters
			if (party._powerShield)
				damage -= party._powerShield;
			if (damage < 0)
				damage = 0;

			if (attackType == DT_SLEEP) {
				damage = c._currentHp;
				c._conditions[DEAD] = 1;
			}

			// Subtract the hit points from the character
			c.subtractHitPoints(damage);
			if (selectedIndex)
				break;
		}

		// Break check and if not, move to other index
		if (!selectedIndex || breakFlag)
			break;

		charIndex = selectedIndex - 1;
		breakFlag = true;
	}

	// WORKAROUND: Flag a script in progress when pausing to prevent any pending combat starting prematurely
	Mode oldMode = _vm->_mode;
	_vm->_mode = MODE_SCRIPT_IN_PROGRESS;
	events.ipause(5);
	_vm->_mode = oldMode;

	intf.drawParty(true);
	party.checkPartyDead();
}

void Combat::doCharDamage(Character &c, int charNum, int monsterDataIndex) {
	Debugger &debugger = *g_vm->_debugger;
	EventsManager &events = *_vm->_events;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	Windows &windows = *_vm->_windows;
	MonsterStruct &monsterData = map._monsterData[monsterDataIndex];

	// Attacked characters are automatically woken up
	c._conditions[ASLEEP] = 0;

	// Figure out the damage amount
	int damage = 0;
	for (int idx = 0; idx < monsterData._strikes; ++idx)
		damage += _vm->getRandomNumber(1, monsterData._dmgPerStrike);


	int fx = 29, frame = 0;
	if (monsterData._attackType != DT_PHYSICAL) {
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
	intf._charPowSprites.draw(0, frame, Common::Point(Res.CHAR_FACES_X[charNum], 150));
	windows[33].update();

	damage = MAX(damage - party._powerShield, 0);
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
			c._items.curseUncurse(true);
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
				if (weapon._id < XEEN_SLAYER_SWORD && weapon._id != 0 && weapon._frame != 0) {
					weapon._state._broken = true;
					// WORKAROUND: For consistency, we don't de-equip broken items
					//weapon._frame = 0;
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

		default:
			break;
		}
	}

	if (debugger._invincible)
		// Invincibility mode is on, so reset conditions that were set
		c.clearConditions();
	else
		// Standard gameplay, deal out the damage
		c.subtractHitPoints(damage);

	events.ipause(2);
	intf.drawParty(true);
}

void Combat::moveMonsters() {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;

	if (!_moveMonsters)
		return;

	intf._tillMove = 0;
	if (intf._charsShooting)
		return;

	Common::fill(&_monsterMap[0][0], &_monsterMap[32][32], 0);
	Common::fill(&_monsterMoved[0], &_monsterMoved[MAX_NUM_MONSTERS], false);
	Common::fill(&_rangeAttacking[0], &_rangeAttacking[MAX_NUM_MONSTERS], false);
	Common::fill(&_gmonHit[0], &_gmonHit[36], -1);
	_dangerPresent = false;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];

		// WORKAROUND: Original only checked on y, but some monsters have an invalid X instead
		if ((uint)monster._position.x < 32 && (uint)monster._position.y < 32) {
			assert((uint)monster._position.x < 32);
			_monsterMap[monster._position.y][monster._position.x]++;
		}
	}

	for (int loopNum = 0; loopNum < 2; ++loopNum) {
		int arrIndex = -1;
		for (int yDiff = 3; yDiff >= -3; --yDiff) {
			for (int xDiff = -3; xDiff <= 3; ++xDiff) {
				Common::Point pt = party._mazePosition + Common::Point(xDiff, yDiff);
				++arrIndex;

				for (int idx = 0; idx < (int)map._mobData._monsters.size(); ++idx) {
					MazeMonster &monster = map._mobData._monsters[idx];
					MonsterStruct &monsterData = *monster._monsterData;

					if (pt == monster._position) {
						_dangerPresent = true;
						if ((monster._isAttacking || _vm->_mode == MODE_SLEEPING)
								&& !_monsterMoved[idx]) {
							if (party._mazePosition.x == pt.x || party._mazePosition.y == pt.y) {
								// Check for range attacks
								if (monsterData._rangeAttack && !_rangeAttacking[idx]
									&& _attackMonsters[0] != idx && _attackMonsters[1] != idx
									&& _attackMonsters[2] != idx && monster._damageType == DT_PHYSICAL) {
									// Setup monster for attacking
									setupMonsterAttack(monster._spriteId, pt);
									_rangeAttacking[idx] = true;
								}
							}

							switch (party._mazeDirection) {
							case DIR_NORTH:
							case DIR_SOUTH:
								if (canMonsterMove(pt, Res.MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX1[arrIndex]],
										MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex], idx)) {
									// Move the monster
									moveMonster(idx, Common::Point(MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex]));
								} else {
									if (canMonsterMove(pt, Res.MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX2[arrIndex]],
										arrIndex >= 21 && arrIndex <= 27 ? MONSTER_GRID3[arrIndex] : 0,
										arrIndex >= 21 && arrIndex <= 27 ? 0 : MONSTER_GRID3[arrIndex],
										idx)) {
										if (arrIndex >= 21 && arrIndex <= 27) {
											moveMonster(idx, Common::Point(MONSTER_GRID3[arrIndex], 0));
										} else {
											moveMonster(idx, Common::Point(0, MONSTER_GRID3[arrIndex]));
										}
									}
								}
								break;

							case DIR_EAST:
							case DIR_WEST:
								if (canMonsterMove(pt, Res.MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX2[arrIndex]],
									arrIndex >= 21 && arrIndex <= 27 ? MONSTER_GRID3[arrIndex] : 0,
									arrIndex >= 21 && arrIndex <= 27 ? 0 : MONSTER_GRID3[arrIndex],
									idx)) {
									if (arrIndex >= 21 && arrIndex <= 27) {
										moveMonster(idx, Common::Point(MONSTER_GRID3[arrIndex], 0));
									} else {
										moveMonster(idx, Common::Point(0, MONSTER_GRID3[arrIndex]));
									}
								} else if (canMonsterMove(pt, Res.MONSTER_GRID_BITMASK[MONSTER_GRID_BITINDEX1[arrIndex]],
										MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex], idx)) {
									moveMonster(idx, Common::Point(MONSTER_GRID_X[arrIndex], MONSTER_GRID_Y[arrIndex]));
								}

							default:
								break;
							}
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
	Sound &sound = *_vm->_sound;
	PowType powNum = POW_INVALID;
	MonsterStruct *monsterData = nullptr;
	OutdoorDrawList &outdoorList = intf._outdoorList;
	IndoorDrawList &indoorList = intf._indoorList;

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] != -1) {
			monsterData = &map._monsterData[_gmonHit[idx]];
			powNum = MONSTER_SHOOT_POW[monsterData->_attackType];
			if (powNum != POW_MAGIC_ARROW)
				break;
		}
	}

	_powSprites.load(Common::String::format("pow%d.icn", (int)powNum));
	sound.playFX(ATTACK_TYPE_FX[monsterData->_attackType]);

	for (int charNum = 0; charNum < MAX_PARTY_COUNT; ++charNum) {
		if (!_shootingRow[charNum])
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

			switch (_shootingRow[charNum]) {
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

			switch (_shootingRow[charNum]) {
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
	} while (!_vm->shouldExit() && intf._isAttacking);

	endAttack();

	if (_vm->_mode != MODE_COMBAT) {
		// Combat wasn't previously active, but it is now. Set up
		// the combat party from the currently active party
		setupCombatParty();
	}

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] != -1)
			doMonsterTurn(_gmonHit[idx]);
	}

	_monstersAttacking = false;

	if (_vm->_mode == MODE_SLEEPING) {
		for (uint charNum = 0; charNum < party._activeParty.size(); ++charNum) {
			Condition condition = party._activeParty[charNum].worstCondition();

			if (condition == DEPRESSED || condition == CONFUSED || condition == NO_CONDITION) {
				_vm->_mode = MODE_INTERACTIVE;
				break;
			}
		}
	}
}

void Combat::setupMonsterAttack(int monsterDataIndex, const Common::Point &pt) {
	Party &party = *_vm->_party;

	for (int idx = 0; idx < 36; ++idx) {
		if (_gmonHit[idx] == -1) {
			int result = stopAttack(pt - party._mazePosition);
			if (result) {
				_monstersAttacking = true;
				_gmonHit[idx] = monsterDataIndex;

				if (result != 1) {
					for (int charNum = 0; charNum < MAX_PARTY_COUNT; ++charNum) {
						if (!_shootingRow[charNum]) {
							_shootingRow[charNum] = COMBAT_SHOOTING[result - 1];
							break;
						}
					}
				}
			}

			break;
		}
	}
}

bool Combat::canMonsterMove(const Common::Point &pt, int wallShift, int xDiff, int yDiff, int monsterId) {
	Map &map = *_vm->_map;
	MazeMonster &monster = map._mobData._monsters[monsterId];
	MonsterStruct &monsterData = *monster._monsterData;

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
				return _vm->_files->_ccNum || monster._spriteId != 59;
			}
		default:
			return v <= map.mazeData()._difficulties._wallNoPass;
		}
	}
}

void Combat::moveMonster(int monsterId, const Common::Point &moveDelta) {
	Map &map = *_vm->_map;
	MazeMonster &monster = map._mobData._monsters[monsterId];
	Common::Point newPos = monster._position + moveDelta;

	// FIXME: Monster moved outside mapping area. Which shouldn't happen, so ignore the move if it does
	if ((uint)newPos.x >= 32 || (uint)newPos.y >= 32)
		return;

	if (_monsterMap[newPos.y][newPos.x] < 3 && monster._damageType == DT_PHYSICAL && _moveMonsters) {
		// Adjust monster's position
		++_monsterMap[newPos.y][newPos.x];
		--_monsterMap[monster._position.y][monster._position.x];
		monster._position = newPos;
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

	clearShooting();
}

void Combat::monsterOvercome() {
	Map &map = *_vm->_map;

	for (uint idx = 0; idx < map._mobData._monsters.size(); ++idx) {
		MazeMonster &monster = map._mobData._monsters[idx];
		int dataIndex = monster._spriteId;

		if (monster._damageType != DT_PHYSICAL && monster._damageType != DT_DRAGONSLEEP) {
			// Do a saving throw for monster
			if (dataIndex <= _vm->getRandomNumber(1, dataIndex + 50))
				monster._damageType = DT_PHYSICAL;
		}
	}
}

void Combat::doMonsterTurn(int monsterId) {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (!_monstersAttacking) {
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

		assert(monsterIndex != -1);
		MazeMonster &monster = map._mobData._monsters[monsterIndex];
		MonsterStruct &monsterData = *monster._monsterData;
		if (monster._damageType != DT_PHYSICAL)
			return;

		monster._frame = 8;
		monster._postAttackDelay = 3;
		monster._field9 = 0;
		intf.draw3d(true);
		intf.draw3d(true);

		sound.playSound(Common::String::format("%s.voc", monsterData._attackVoc.c_str()));
		monsterId = monster._spriteId;
	}

	MonsterStruct &monsterData = map._monsterData[monsterId];
	for (int attackNum = 0; attackNum < monsterData._numberOfAttacks; ++attackNum) {
		int charNum = -1;
		bool isHated = false;

		if (monsterData._hatesClass != CLASS_PALADIN) {
			if (monsterData._hatesClass == HATES_PARTY) {
				// Monster hates entire party, even the disabled/dead
				for (uint idx = 0; idx < _combatParty.size(); ++idx) {
					doMonsterTurn(monsterId, idx);
				}

				// Move onto monster's next attack (if any)
				continue;
			}

			for (uint charIndex = 0; charIndex < _combatParty.size(); ++charIndex) {
				Character &c = *_combatParty[charIndex];
				Condition cond = c.worstCondition();
				if (cond >= PARALYZED && cond <= ERADICATED)
					continue;

				switch (monsterData._hatesClass) {
				case CLASS_KNIGHT:
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
				case HATES_DWARF:
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
			// No particularly hated foe, so pick a random character to start with
			// Note: Original had a whole switch statement depending on party size, that boiled down to
			// picking a random character in all cases anyway
			charNum = _vm->getRandomNumber(0, _combatParty.size() - 1);
		}

		// If the chosen character is already disabled, we need to pick a still able body character
		// from the remainder of the combat party
		Condition cond = _combatParty[charNum]->worstCondition();
		if (cond >= PARALYZED && cond <= ERADICATED) {
			Common::Array<int> ableChars;

			for (uint idx = 0; idx < _combatParty.size(); ++idx) {
				switch (_combatParty[idx]->worstCondition()) {
				case PARALYZED:
				case UNCONSCIOUS:
				case DEAD:
				case STONED:
				case ERADICATED:
					break;
				default:
					ableChars.push_back(idx);
					break;
				}
			}

			if (ableChars.size() == 0) {
				party._dead = true;
				_vm->_mode = MODE_INTERACTIVE;
				return;
			}

			charNum = ableChars[_vm->getRandomNumber(0, ableChars.size() - 1)];
		}

		doMonsterTurn(monsterId, charNum);
	}

	intf.drawParty(true);
}

void Combat::doMonsterTurn(int monsterId, int charNum) {
	Map &map = *_vm->_map;
	Sound &sound = *_vm->_sound;
	MonsterStruct &monsterData = map._monsterData[monsterId];
	Character &c = *_combatParty[charNum];

	if (monsterData._attackType != DT_PHYSICAL || c._conditions[ASLEEP]) {
		doCharDamage(c, charNum, monsterId);
	} else {
		int v = _vm->getRandomNumber(1, 20);
		if (v == 1) {
			// Critical Save
			sound.playFX(6);
		} else {
			if (v == 20)
				// Critical failure
				doCharDamage(c, charNum, monsterId);
			v += monsterData._hitChance / 4 + _vm->getRandomNumber(1,
				monsterData._hitChance);

			int ac = c.getArmorClass() + (!_charsBlocked[charNum] ? 10 :
				c.getCurrentLevel() / 2 + 15);
			if (ac > v) {
				sound.playFX(6);
			} else {
				doCharDamage(c, charNum, monsterId);
			}
		}
	}
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

void Combat::setupCombatParty() {
	Party &party = *_vm->_party;

	_combatParty.clear();
	for (uint idx = 0; idx < party._activeParty.size(); ++idx)
		_combatParty.push_back(&party._activeParty[idx]);
}

void Combat::setSpeedTable() {
	Map &map = *_vm->_map;
	Common::Array<int> charSpeeds;
	bool hasSpeed = _whosSpeed != -1;
	int oldSpeed = hasSpeed && _whosSpeed < (int)_speedTable.size() ? _speedTable[_whosSpeed] : 0;

	// Set up speeds for party members
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
			MonsterStruct &monsterData = *monster._monsterData;
			charSpeeds.push_back(monsterData._speed);

			maxSpeed = MAX(maxSpeed, monsterData._speed);
		} else {
			charSpeeds.push_back(0);
		}
	}

	// Populate the _speedTable list with the character/monster indexes
	// in order of attacking speed
	_speedTable.clear();
	for (; maxSpeed > 0; --maxSpeed) {
		for (uint idx = 0; idx < charSpeeds.size(); ++idx) {
			if (charSpeeds[idx] == maxSpeed)
				_speedTable.push_back(idx);
		}
	}

	if (hasSpeed) {
		if (_speedTable.empty()) {
			_whosSpeed = 0;
		} else if (_whosSpeed >= (int)_speedTable.size() || _speedTable[_whosSpeed] != oldSpeed) {
			for (_whosSpeed = 0; _whosSpeed < (int)_speedTable.size(); ++_whosSpeed) {
				if (oldSpeed == _speedTable[_whosSpeed])
					break;
			}

			if (_whosSpeed == (int)charSpeeds.size())
				error("Could not reset next speedy character. Beep beep.");
		}
	}
}

bool Combat::allHaveGone() const {
	int monsCount = (_attackMonsters[0] != -1 ? 1 : 0)
		+ (_attackMonsters[1] != -1 ? 1 : 0)
		+ (_attackMonsters[2] != -1 ? 1 : 0);

	for (uint idx = 0; idx < (_combatParty.size() + monsCount); ++idx) {
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

bool Combat::charsCantAct() const {
	for (uint idx = 0; idx < _combatParty.size(); ++idx) {
		if (!_combatParty[idx]->isDisabledOrDead())
			return false;
	}

	return true;
}

Common::String Combat::getMonsterDescriptions() {
	Map &map = *_vm->_map;
	Common::String lines[3];

	// Get names of monsters attacking, if any
	for (int idx = 0; idx < 3; ++idx) {
		if (_attackMonsters[idx] != -1) {
			MazeMonster &monster = map._mobData._monsters[_attackMonsters[idx]];
			MonsterStruct &monsterData = *monster._monsterData;
			int textColor = monster.getTextColor();

			Common::String format = "\n\v020\f%2u%s\fd";
			format.setChar('2' + idx, 3);
			lines[idx] = Common::String::format(format.c_str(), textColor,
				monsterData._name.c_str());
		}
	}

	if (_attackDurationCtr == 2 && _attackMonsters[2] != -1) {
		_monster2Attack = _attackMonsters[2];
	} else if (_attackDurationCtr == 1 && _attackMonsters[1] != -1) {
		_monster2Attack = _attackMonsters[1];
	} else {
		_monster2Attack = _attackMonsters[0];
		_attackDurationCtr = 0;
	}

	return Common::String::format(Res.COMBAT_DETAILS, lines[0].c_str(),
		lines[1].c_str(), lines[2].c_str());
}

void Combat::attack(Character &c, RangeType rangeType) {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	int damage = _monsterDamage;

	if (_monster2Attack == -1)
		return;

	MazeMonster &monster = map._mobData._monsters[_monster2Attack];
	int monsterDataIndex = monster._spriteId;
	MonsterStruct &monsterData = map._monsterData[monsterDataIndex];

	if (rangeType != RT_SINGLE) {
		if (_shootType != ST_1 || _damageType == DT_MAGIC_ARROW) {
			if (!monsterData._magicResistence || monsterData._magicResistence <=
					_vm->getRandomNumber(1, 100 + _oldCharacter->getCurrentLevel())) {
				if (_monsterDamage != 0) {
					attack2(damage, rangeType);
				} else {
					switch (_damageType) {
					case DT_SLEEP:
						if (monsterData._monsterType == MONSTER_ANIMAL || monsterData._monsterType == MONSTER_HUMANOID) {
							if (_vm->getRandomNumber(1, 50 + monsterDataIndex) > monsterDataIndex)
								monster._damageType = DT_SLEEP;
						}
						break;
					case DT_FINGEROFDEATH:
						if ((monsterData._monsterType == MONSTER_ANIMAL || monsterData._monsterType == MONSTER_HUMANOID)
							&& !monsterSavingThrow(monsterDataIndex)) {
							damage = MIN(monster._hp, 50);
							attack2(damage, RT_ALL);
						}
						break;
					case DT_HOLYWORD:
						if (monsterData._monsterType == MONSTER_UNDEAD) {
							attack2(monster._hp, RT_ALL);
						}
						break;
					case DT_MASS_DISTORTION:
						attack2(MAX(monster._hp / 2, 1), RT_ALL);
						break;
					case DT_UNDEAD:
						if (monsterData._monsterType == MONSTER_UNDEAD)
							damage = 25;
						else
							rangeType = RT_ALL;
						attack2(damage, rangeType);
						break;
					case DT_BEASTMASTER:
						if ((monsterData._monsterType == MONSTER_ANIMAL || monsterData._monsterType == MONSTER_HUMANOID)
							&& !monsterSavingThrow(monsterDataIndex)) {
							monster._damageType = DT_BEASTMASTER;
						}
						break;
					case DT_DRAGONSLEEP:
						if (monsterData._monsterType == MONSTER_DRAGON && !monsterSavingThrow(monsterDataIndex))
							monster._damageType = DT_DRAGONSLEEP;
						break;
					case DT_GOLEMSTOPPER:
						if (monsterData._monsterType == MONSTER_GOLEM) {
							attack2(100, rangeType);
						}
						break;
					case DT_HYPNOTIZE:
						if ((monsterData._monsterType == MONSTER_ANIMAL || monsterData._monsterType == MONSTER_HUMANOID)
							&& !monsterSavingThrow(monsterDataIndex)) {
							monster._damageType = _damageType;
						}
						break;
					case DT_INSECT_SPRAY:
						if (monsterData._monsterType == MONSTER_INSECT) {
							attack2(25, rangeType);
						}
						break;
					case DT_MAGIC_ARROW:
						attack2(8, rangeType);
						break;
					default:
						break;
					}
				}
			}
		} else {
			_pow.resetElementals();
			damage = 0;

			for (uint charIndex = 0; charIndex < party._activeParty.size(); ++charIndex) {
				Character &ch = party._activeParty[charIndex];

				if (_shootingRow[charIndex] && !_missedShot[charIndex]) {
					if (!hitMonster(ch, rangeType)) {
						++_missedShot[charIndex];
					} else {
						damage = _monsterDamage ? _monsterDamage : _weaponDamage;
						_shootingRow[charIndex] = 0;
						attack2(damage, RT_HIT);

						if (map._isOutdoors) {
							intf._outdoorList._attackImgs1[charIndex]._scale = 0;
							intf._outdoorList._attackImgs1[charIndex]._sprites = nullptr;
							intf._outdoorList._attackImgs2[charIndex]._scale = 0;
							intf._outdoorList._attackImgs2[charIndex]._sprites = nullptr;
							intf._outdoorList._attackImgs3[charIndex]._scale = 0;
							intf._outdoorList._attackImgs3[charIndex]._sprites = nullptr;
							intf._outdoorList._attackImgs4[charIndex]._scale = 0;
							intf._outdoorList._attackImgs4[charIndex]._sprites = nullptr;
						} else {
							intf._indoorList._attackImgs1[charIndex]._scale = 0;
							intf._indoorList._attackImgs1[charIndex]._sprites = nullptr;
							intf._indoorList._attackImgs2[charIndex]._scale = 0;
							intf._indoorList._attackImgs2[charIndex]._sprites = nullptr;
							intf._indoorList._attackImgs3[charIndex]._scale = 0;
							intf._indoorList._attackImgs3[charIndex]._sprites = nullptr;
							intf._indoorList._attackImgs4[charIndex]._scale = 0;
							intf._indoorList._attackImgs4[charIndex]._sprites = nullptr;
						}

						if (_monster2Attack == -1)
							return;
					}
				}
			}
		}
	} else {
		_damageType = DT_PHYSICAL;
		int divisor = 0;
		switch (c._class) {
		case CLASS_BARBARIAN:
			divisor = 4;
			break;
		case CLASS_KNIGHT:
		case CLASS_NINJA:
			divisor = 5;
			break;
		case CLASS_PALADIN:
		case CLASS_ARCHER:
		case CLASS_ROBBER:
		case CLASS_RANGER:
			divisor = 6;
			break;
		case CLASS_CLERIC:
		case CLASS_DRUID:
			divisor = 7;
			break;
		case CLASS_SORCERER:
			divisor = 8;
			break;
		default:
			error("Invalid class");
		}

		int numberOfAttacks = c.getCurrentLevel() / divisor + 1;
		damage = 0;

		while (numberOfAttacks-- > 0) {
			if (hitMonster(c, RT_SINGLE))
				damage += getMonsterDamage(c);
		}

		for (int itemIndex = 0; itemIndex < INV_ITEMS_TOTAL; ++itemIndex) {
			XeenItem &weapon = c._weapons[itemIndex];
			if (weapon.isEquipped()) {
				switch (weapon._state._counter) {
				case EFFECTIVE_DRAGON:
					if (monsterData._monsterType == MONSTER_DRAGON)
						damage *= 3;
					break;
				case EFFECTIVE_UNDEAD	:
					if (monsterData._monsterType == MONSTER_UNDEAD)
						damage *= 3;
					break;
				case EFFECTIVE_GOLEM:
					if (monsterData._monsterType == MONSTER_GOLEM)
						damage *= 3;
					break;
				case EFFECTIVE_INSECT:
					if (monsterData._monsterType == MONSTER_INSECT)
						damage *= 3;
					break;
				case EFFEctIVE_MONSTERS:
					if (monsterData._monsterType == MONSTER_MONSTERS)
						damage *= 3;
					break;
				case EFFECTIVE_ANIMAL:
					if (monsterData._monsterType == MONSTER_ANIMAL)
						damage *= 3;
					break;
				default:
					break;
				}
			}
		}

		attack2(damage, rangeType);
	}

	setSpeedTable();
}

void Combat::attack2(int damage, RangeType rangeType) {
	Debugger &debugger = *_vm->_debugger;
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;
	int ccNum = _vm->_files->_ccNum;
	MazeMonster &monster = map._mobData._monsters[_monster2Attack];
	MonsterStruct &monsterData = *monster._monsterData;
	bool monsterDied = false;

	if (!ccNum && damage && rangeType != RT_SINGLE && monster._spriteId == 89)
		damage = 0;
	if (debugger._superStrength)
		damage = 10000;

	if (!damage) {
		sound.playSound(_missVoc, 1);
		sound.playFX(6);
	} else {
		if (!ccNum && monster._spriteId == 89)
			damage += 100;
		if (monster._damageType == DT_SLEEP || monster._damageType == DT_DRAGONSLEEP)
			monster._damageType = DT_PHYSICAL;

		if ((rangeType == RT_SINGLE || _damageType == DT_PHYSICAL) && _attackWeaponId < XEEN_SLAYER_SWORD) {
			if (monsterData._phsyicalResistence != 0) {
				if (monsterData._phsyicalResistence == 100) {
					// Completely immune to the damage
					damage = 0;
				} else {
					// Reduce the damage based on physical resistance
					damage = damage * (100 - monsterData._phsyicalResistence) / 100;
				}
			}
		}

		if (damage) {
			_pow[_attackDurationCtr]._duration = 3;
			_pow[_attackDurationCtr]._active = _damageType == DT_PHYSICAL && (rangeType == RT_HIT || rangeType == RT_SINGLE);
			monster._frame = 11;
			monster._postAttackDelay = 5;
		}

		int monsterResist = getMonsterResistence(rangeType);
		damage += monsterResist;
		if (monsterResist > 0) {
			_pow[_attackDurationCtr]._elemFrame = XeenItem::getElementalCategory(_weaponElemMaterial);
			_pow[_attackDurationCtr]._elemScale = getDamageScale(monsterResist);
		} else if (rangeType != RT_HIT) {
			_pow[_attackDurationCtr]._elemFrame = 0;
		}

		if (rangeType != RT_SINGLE && rangeType != RT_HIT) {
			monster._effect2 = DAMAGE_TYPE_EFFECTS[_damageType];
			monster._effect1 = 0;
		}

		if (rangeType != RT_SINGLE && monsterSavingThrow(monster._spriteId)) {
			switch (_damageType) {
			case DT_FINGEROFDEATH:
			case DT_MASS_DISTORTION:
				damage = 5;
				break;
			case DT_SLEEP:
			case DT_HOLYWORD:
			case DT_UNDEAD:
			case DT_BEASTMASTER:
			case DT_DRAGONSLEEP:
			case DT_GOLEMSTOPPER:
			case DT_HYPNOTIZE:
			case DT_INSECT_SPRAY:
			case DT_MAGIC_ARROW:
				break;
			default:
				damage /= 2;
				break;
			}
		}

		if (damage < 1) {
			sound.playSound(_missVoc, 1);
			sound.playFX(6);
		} else {
			_pow[_attackDurationCtr]._scale = getDamageScale(damage);
			intf.draw3d(true);

			sound.stopSound();
			int powNum = (_attackWeaponId > XEEN_SLAYER_SWORD) ? 0 : POW_WEAPON_VOCS[_attackWeaponId];
			File powVoc(Common::String::format("pow%d.voc", powNum));
			sound.playFX(60 + powNum);
			sound.playSound(powVoc, 1);

			if (monster._hp > damage) {
				monster._hp -= damage;
			} else {
				monster._hp = 0;
				monsterDied = true;
			}
		}
	}

	intf.draw3d(true);

	if (monsterDied) {
		if (!ccNum) {
			if (_monster2Attack == 20 && party._mazeId == 41)
				party._gameFlags[0][11] = true;
			if (_monster2Attack == 8 && party._mazeId == 78) {
				party._gameFlags[0][60] = true;
				party._questFlags[23] = false;

				for (uint idx = 0; idx < party._activeParty.size(); ++idx)
					party._activeParty[idx].setAward(42, true);
			}
			if (_monster2Attack == 27 && party._mazeId == 29)
				party._gameFlags[0][104] = true;
		}

		giveExperience(monsterData._experience);

		if (party._mazeId != 85) {
			party._treasure._gold += monsterData._gold;
			party._treasure._gems += monsterData._gems;

			if (!ccNum && monster._spriteId == 89) {
				// Xeen's Scepter of Temporal Distortion
				party._treasure._weapons[0]._id = 90;
				party._treasure._weapons[0]._material = 0;
				party._treasure._weapons[0]._state.clear();
				party._treasure._hasItems = true;
				party._questItems[8]++;
			}

			int itemDrop = monsterData._itemDrop;
			if (itemDrop) {
				if (MONSTER_ITEM_RANGES[itemDrop] >= _vm->getRandomNumber(1, 100)) {
					Character tempChar;
					int category = tempChar.makeItem(itemDrop, 0, 0);

					switch (category) {
					case CATEGORY_WEAPON:
						for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
							if (party._treasure._weapons[idx].empty()) {
								party._treasure._weapons[idx] = tempChar._weapons[0];
								party._treasure._hasItems = true;
								break;
							}
						}
						break;
					case CATEGORY_ARMOR:
						for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
							if (party._treasure._armor[idx].empty()) {
								party._treasure._armor[idx] = tempChar._armor[0];
								party._treasure._hasItems = true;
								break;
							}
						}
						break;
					case CATEGORY_ACCESSORY:
						for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
							if (party._treasure._accessories[idx].empty()) {
								party._treasure._accessories[idx] = tempChar._accessories[0];
								party._treasure._hasItems = true;
								break;
							}
						}
						break;
					case CATEGORY_MISC:
						for (int idx = 0; idx < MAX_TREASURE_ITEMS; ++idx) {
							if (party._treasure._accessories[idx].empty()) {
								party._treasure._accessories[idx] = tempChar._accessories[0];
								party._treasure._hasItems = true;
								break;
							}
						}
						break;
					default:
						break;
					}
				}
			}
		}

		monster._position = Common::Point(0x80, 0x80);
		_pow[_attackDurationCtr]._duration = 0;
		_monster2Attack = -1;
		intf.draw3d(true);

		if (_attackMonsters[0] != -1) {
			_monster2Attack = _attackMonsters[0];
			_attackDurationCtr = 0;
		}
	}
}

void Combat::block() {
	_charsBlocked[_whosTurn] = true;
}

void Combat::quickFight() {
	Spells &spells = *_vm->_spells;
	Character *c = _combatParty[_whosTurn];

	switch (c->_quickOption) {
	case QUICK_ATTACK:
		attack(*c, RT_SINGLE);
		break;
	case QUICK_SPELL:
		if (c->_currentSpell != -1) {
			spells.castSpell(c, (MagicSpell)Res.SPELLS_ALLOWED[c->getSpellsCategory()][c->_currentSpell]);
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

void Combat::run() {
	Map &map = *_vm->_map;
	Sound &sound = *_vm->_sound;

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

bool Combat::hitMonster(Character &c, RangeType rangeType) {
	Map &map = *_vm->_map;
	getWeaponDamage(c, rangeType);
	int chance = c.statBonus(c.getStat(ACCURACY)) + _hitChanceBonus;
	int divisor = 0;

	switch (c._class) {
	case CLASS_PALADIN :
	case CLASS_ARCHER:
	case CLASS_ROBBER:
	case CLASS_NINJA:
	case CLASS_RANGER:
		divisor = 2;
		break;
	case CLASS_CLERIC:
	case CLASS_DRUID:
		divisor = 3;
		break;
	case CLASS_SORCERER:
		divisor = 4;
		break;
	case CLASS_KNIGHT:
	case CLASS_BARBARIAN:
	default:
		divisor = 1;
		break;
	}

	chance += c.getCurrentLevel() / divisor;
	chance -= c._conditions[CURSED];

	// Add on a random amount
	int v;
	do {
		v = _vm->getRandomNumber(1, 20);
		chance += v;
	} while (v == 20);

	assert(_monster2Attack != -1);
	MazeMonster &monster = map._mobData._monsters[_monster2Attack];
	MonsterStruct &monsterData = *monster._monsterData;

	if (monster._damageType != DT_PHYSICAL)
		chance += 20;

	return chance >= (monsterData._armorClass + 10);
}

void Combat::getWeaponDamage(Character &c, RangeType rangeType) {
	Party &party = *_vm->_party;
	_attackWeapon = nullptr;
	_weaponDie = _weaponDice = 0;
	_weaponDamage = 0;
	_hitChanceBonus = 0;
	_weaponElemMaterial = 0;

	for (int idx = 0; idx < INV_ITEMS_TOTAL; ++idx) {
		XeenItem &weapon = c._weapons[idx];
		bool flag;
		if (rangeType != RT_SINGLE) {
			flag = weapon._frame == 4;
		} else {
			flag = weapon._frame == 1 || weapon._frame == 13;
		}

		if (flag) {
			if (!weapon.isBad()) {
				_attackWeapon = &weapon;

				if (weapon._material < 37) {
					_weaponElemMaterial = weapon._material;
				} else if (weapon._material < 59) {
					_hitChanceBonus = Res.METAL_DAMAGE_PERCENT[weapon._material - 37];
					_weaponDamage = Res.METAL_DAMAGE[weapon._material - 37];
				}
			}

			_hitChanceBonus += party._heroism;
			_attackWeaponId = weapon._id;
			_weaponDice = Res.WEAPON_DAMAGE_BASE[_attackWeaponId];
			_weaponDie = Res.WEAPON_DAMAGE_MULTIPLIER[_attackWeaponId];

			for (int diceIdx = 0; diceIdx < _weaponDice; ++diceIdx)
				_weaponDamage += _vm->getRandomNumber(1, _weaponDie);
		}
	}

	if (_weaponDamage < 1)
		_weaponDamage = 0;
	if (party._difficulty == ADVENTURER) {
		_hitChanceBonus += 5;
		_weaponDamage *= 3;
	}
}

int Combat::getMonsterDamage(Character &c) {
	return MAX(c.statBonus(c.getStat(MIGHT)) + _weaponDamage, 1);
}

int Combat::getDamageScale(int v) {
	if (v < 10)
		return 5;
	else if (v < 100)
		return 0;
	else
		return 0x8000;
}

int Combat::getMonsterResistence(RangeType rangeType) {
	Map &map = *_vm->_map;
	assert(_monster2Attack != -1);
	MazeMonster &monster = map._mobData._monsters[_monster2Attack];
	MonsterStruct &monsterData = *monster._monsterData;
	int resistence = 0, damage = 0;

	if (rangeType != RT_SINGLE && rangeType != RT_HIT) {
		switch (_damageType) {
		case DT_PHYSICAL:
			resistence = monsterData._phsyicalResistence;
			break;
		case DT_MAGICAL:
			resistence = monsterData._magicResistence;
			break;
		case DT_FIRE:
			resistence = monsterData._fireResistence;
			break;
		case DT_ELECTRICAL:
			resistence = monsterData._electricityResistence;
			break;
		case DT_COLD:
			resistence = monsterData._coldResistence;
			break;
		case DT_POISON:
			resistence = monsterData._poisonResistence;
			break;
		case DT_ENERGY:
			resistence = monsterData._energyResistence;
			break;
		default:
			break;
		}
	} else {
		int material = _weaponElemMaterial;
		damage = Res.ELEMENTAL_DAMAGE[material];

		if (material != 0) {
			if (material < 9)
				resistence = monsterData._fireResistence;
			else if (material < 16)
				resistence = monsterData._electricityResistence;
			else if (material < 21)
				resistence = monsterData._coldResistence;
			else if (material < 26)
				resistence = monsterData._poisonResistence;
			else if (material < 34)
				resistence = monsterData._energyResistence;
			else
				resistence = monsterData._magicResistence;
		}
	}

	if (resistence != 0) {
		if (resistence == 100)
			return 0;
		else
			return ((100 - resistence) * damage) / 100;
	}

	return damage;
}

void Combat::giveExperience(int experience) {
	Party &party = *_vm->_party;
	bool inCombat = _vm->_mode == MODE_COMBAT;
	int count = 0;

	// Two loops: first to figure out how many active characters there are,
	// and the second to distribute the experience between them
	for (int loopNum = 0; loopNum < 2; ++loopNum) {
		for (uint charIndex = 0; charIndex < (inCombat ? _combatParty.size() :
				party._activeParty.size()); ++charIndex) {
			Character &c = inCombat ? *_combatParty[charIndex] : party._activeParty[charIndex];
			Condition condition = c.worstCondition();

			if (condition != DEAD && condition != STONED && condition != ERADICATED) {
				if (loopNum == 0) {
					++count;
				} else {
					int exp = experience / count;
					if (c._level._permanent < 15 && _vm->getGameID() != GType_Clouds)
						exp *= 2;
					c._experience += exp;
				}
			}
		}
	}
}

void Combat::rangedAttack(PowType powNum) {
	Interface &intf = *_vm->_interface;
	Map &map = *_vm->_map;
	Party &party = *_vm->_party;
	Sound &sound = *_vm->_sound;

	if (_damageType == DT_POISON_VOLLEY) {
		_damageType = DT_POISON;
		_shootType = ST_1;
		Common::fill(&_shootingRow[0], &_shootingRow[MAX_ACTIVE_PARTY], 1);
	} else if (powNum == POW_ARROW) {
		_shootType = ST_1;
		bool flag = false;

		if (_damageType == DT_PHYSICAL) {
			for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
				Character &c = party._activeParty[idx];
				if (c.hasMissileWeapon()) {
					_shootingRow[idx] = 1;
					flag = true;
				}
			}
		} else {
			_shootingRow[0] = 1;
			flag = true;
		}

		if (!flag) {
			sound.playFX(21);
			return;
		}

		sound.playFX(49);
	} else {
		_shootingRow[0] = 1;
		_shootType = ST_0;
	}

	intf._charsShooting = true;
	_powSprites.load(Common::String::format("pow%d.icn", (int)powNum));
	int attackDurationCtr = _attackDurationCtr;
	int monster2Attack = _monster2Attack;
	bool attackedFlag = false;

	Common::Array<int> attackMonsters;
	for (int idx = 0; idx < 3; ++idx) {
		if (_attackMonsters[idx] != -1)
			attackMonsters.push_back(_attackMonsters[idx]);
	}

	_attackDurationCtr = -1;
	if (_monster2Attack != -1) {
		_attackDurationCtr = attackDurationCtr - 1;
		if (attackMonsters.empty())
			attackMonsters.resize(1);
		attackMonsters[0] = monster2Attack;
	}

	for (uint idx = 0; idx < party._activeParty.size(); ++idx) {
		if (_shootingRow[idx]) {
			if (map._isOutdoors) {
				intf._outdoorList._attackImgs1[idx]._scale = 0;
				intf._outdoorList._attackImgs2[idx]._scale = 4;
				intf._outdoorList._attackImgs3[idx]._scale = 8;
				intf._outdoorList._attackImgs4[idx]._scale = 12;
				intf._outdoorList._attackImgs1[idx]._sprites = &_powSprites;
				intf._outdoorList._attackImgs2[idx]._sprites = nullptr;
				intf._outdoorList._attackImgs3[idx]._sprites = nullptr;
				intf._outdoorList._attackImgs4[idx]._sprites = nullptr;
			} else {
				intf._indoorList._attackImgs1[idx]._scale = 0;
				intf._indoorList._attackImgs2[idx]._scale = 4;
				intf._indoorList._attackImgs3[idx]._scale = 8;
				intf._indoorList._attackImgs4[idx]._scale = 12;
				intf._indoorList._attackImgs1[idx]._sprites = &_powSprites;
				intf._indoorList._attackImgs2[idx]._sprites = nullptr;
				intf._indoorList._attackImgs3[idx]._sprites = nullptr;
				intf._indoorList._attackImgs4[idx]._sprites = nullptr;
			}
		}
	}

	intf.draw3d(true);

	// Iterate through the three possible monster positions in the first row
	for (uint monIdx = 0; monIdx < 3; ++monIdx) {
		++_attackDurationCtr;

		if (monIdx < attackMonsters.size()) {
			Common::fill(&_missedShot[0], &_missedShot[MAX_PARTY_COUNT], false);
			_monster2Attack = attackMonsters[monIdx];
			attack(*_oldCharacter, RT_GROUP);
			attackedFlag = true;

			if (_rangeType == RT_SINGLE)
				// Only single shot, so exit now that the attack is done
				goto finished;
		}
	}

	if (attackedFlag && _rangeType == RT_GROUP)
		// Finished group attack, so exit
		goto finished;

	if (map._isOutdoors) {
		map.getCell(7);
		switch (map._currentWall) {
		case 1:
		case 3:
		case 6:
		case 7:
		case 9:
		case 10:
		case 12:
			sound.playFX(46);
			goto finished;
		default:
			break;
		}
	} else {
		int cell = map.getCell(2);
		if (cell >= map.mazeData()._difficulties._wallNoPass) {
			sound.playFX(46);
			goto finished;
		}
	}
	if (!intf._isAttacking)
		goto finished;

	intf.draw3d(true);

	// Start handling second teir of monsters in the back
	attackMonsters.clear();
	for (uint idx = 3; idx < 6; ++idx) {
		if (_attackMonsters[idx] != -1)
			attackMonsters.push_back(_attackMonsters[idx]);
	}

	// Iterate through the three possible monster positions in the second row
	for (uint monIdx = 0; monIdx < 3; ++monIdx) {
		++_attackDurationCtr;

		if (monIdx < attackMonsters.size()) {
			Common::fill(&_missedShot[0], &_missedShot[MAX_PARTY_COUNT], false);
			_monster2Attack = attackMonsters[monIdx];
			attack(*_oldCharacter, RT_GROUP);
			attackedFlag = true;

			if (_rangeType == RT_SINGLE)
				// Only single shot, so exit now that the attack is done
				goto finished;
		}
	}

	if (attackedFlag && _rangeType == RT_GROUP)
		// Finished group attack, so exit
		goto finished;

	if (map._isOutdoors) {
		map.getCell(14);
		switch (map._currentWall) {
		case 1:
		case 3:
		case 6:
		case 7:
		case 9:
		case 10:
		case 12:
			sound.playFX(46);
			goto finished;
		default:
			break;
		}
	} else {
		int cell = map.getCell(7);
		if (cell >= map.mazeData()._difficulties._wallNoPass) {
			sound.playFX(46);
			goto finished;
		}
	}
	if (!intf._isAttacking)
		goto finished;

	intf.draw3d(true);

	// Start handling third teir of monsters in the back
	attackMonsters.clear();
	for (uint idx = 6; idx < 9; ++idx) {
		if (_attackMonsters[idx] != -1)
			attackMonsters.push_back(_attackMonsters[idx]);
	}

	// Iterate through the three possible monster positions in the third row
	for (uint monIdx = 0; monIdx < 3; ++monIdx) {
		++_attackDurationCtr;

		if (monIdx < attackMonsters.size()) {
			Common::fill(&_missedShot[0], &_missedShot[MAX_PARTY_COUNT], false);
			_monster2Attack = attackMonsters[monIdx];
			attack(*_oldCharacter, RT_GROUP);
			attackedFlag = true;

			if (_rangeType == RT_SINGLE)
				// Only single shot, so exit now that the attack is done
				goto finished;
		}
	}

	if (attackedFlag && _rangeType == RT_GROUP)
		// Finished group attack, so exit
		goto finished;

	if (map._isOutdoors) {
		map.getCell(27);
		switch (map._currentWall) {
		case 1:
		case 3:
		case 6:
		case 7:
		case 9:
		case 10:
		case 12:
			sound.playFX(46);
			goto finished;
		default:
			break;
		}
	} else {
		int cell = map.getCell(14);
		if (cell >= map.mazeData()._difficulties._wallNoPass) {
			sound.playFX(46);
			goto finished;
		}
	}
	if (!intf._isAttacking)
		goto finished;

	intf.draw3d(true);

	// Fourth tier
	attackMonsters.clear();
	for (uint idx = 9; idx < 12; ++idx) {
		if (_attackMonsters[idx] != -1)
			attackMonsters.push_back(_attackMonsters[idx]);
	}

	// Iterate through the three possible monster positions in the fourth row
	for (uint monIdx = 0; monIdx < 3; ++monIdx) {
		++_attackDurationCtr;

		if (monIdx < attackMonsters.size()) {
			Common::fill(&_missedShot[0], &_missedShot[MAX_PARTY_COUNT], false);
			_monster2Attack = attackMonsters[monIdx];
			attack(*_oldCharacter, RT_GROUP);
			attackedFlag = true;

			if (_rangeType == RT_SINGLE)
				// Only single shot, so exit now that the attack is done
				goto finished;
		}
	}

	if (!(attackedFlag && _rangeType == RT_GROUP))
		goto done;

finished:
	endAttack();

done:
	clearShooting();
	_monster2Attack = monster2Attack;
	_attackDurationCtr = attackDurationCtr;
	party.giveTreasure();
}

void Combat::shootRangedWeapon() {
	_rangeType = RT_ALL;
	_damageType = DT_PHYSICAL;
	rangedAttack(POW_ARROW);
}

bool Combat::areMonstersPresent() const {
	for (int idx = 0; idx < 26; ++idx) {
		if (_attackMonsters[idx] != -1)
			return true;
	}

	return false;
}

void Combat::reset() {
	clearShooting();
	setupCombatParty();

	_combatMode = COMBATMODE_INTERACTIVE;
	_monster2Attack = -1;
}

} // End of namespace Xeen
