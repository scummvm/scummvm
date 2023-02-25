/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "efh/efh.h"

namespace Efh {

void EfhEngine::createOpponentList(int16 monsterTeamId) {
	debugC(3, kDebugFight, "createOpponentList %d", monsterTeamId);

	int16 counter = 0;
	if (monsterTeamId != -1 && countAliveMonsters(monsterTeamId) > 0) {
		counter = 1;
		_teamMonster[0]._id = monsterTeamId;
	}

	for (int counter2 = 1; counter2 <= 3; ++counter2) {
		if (counter >= 5)
			break;

		for (uint monsterId = 0; monsterId < 64; ++monsterId) {
			MapMonster *curMapMonst = &_mapMonsters[_techId][monsterId];
			if (curMapMonst->_fullPlaceId == 0xFF)
				continue;

			if (((curMapMonst->_possessivePronounSHL6 & 0x3F) != 0x3F || isNpcATeamMember(curMapMonst->_npcId)) && (curMapMonst->_possessivePronounSHL6 & 0x3F) > 0x3D)
				continue;

			if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
				continue;

			bool found = false;
			for (uint subId = 0; subId < 9; ++subId) {
				if (curMapMonst->_hitPoints[subId] > 0) {
					found = true;
					break;
				}
			}

			if (found) {
				if (computeMonsterGroupDistance(monsterId) <= counter2 && !isMonsterAlreadyFighting(monsterId, counter)) {
					_teamMonster[counter]._id = monsterId;
					if (++counter >= 5)
						break;
				}
			}
		}
	}

	if (counter > 4)
		return;

	for (uint id = counter; id < 5; ++id)
		_teamMonster[id]._id = -1;
}

void EfhEngine::initFight(int16 monsterId) {
	debugC(3, kDebugFight, "initFight %d", monsterId);
	createOpponentList(monsterId);
	resetTeamMonsterEffects();
}

bool EfhEngine::handleFight(int16 monsterId) {
	debugC(3, kDebugFight, "handleFight %d", monsterId);

	_ongoingFightFl = true;

	initFight(monsterId);

	if (_teamMonster[0]._id == -1) {
		resetTeamMonsterIdArray();
		_ongoingFightFl = false;
		displayAnimFrames(0xFE, true);
		return true;
	}

	drawCombatScreen(0, false, true);

	for (bool mainLoopCond = false; !mainLoopCond;) {
		if (isTPK()) {
			resetTeamMonsterIdArray();
			_ongoingFightFl = false;
			displayAnimFrames(0xFE, true);
			return false;
		}

		if (_teamMonster[0]._id == -1) {
			resetTeamMonsterIdArray();
			_ongoingFightFl = false;
			displayAnimFrames(0xFE, true);
			return true;
		}

		displayAnimFrames(getTeamMonsterAnimId(), true);
		for (int counter = 0; counter < _teamSize; ++counter) {
			_teamChar[counter]._pctVisible = 100;
			_teamChar[counter]._pctDodgeMiss = 65;
		}

		if (!getTeamAttackRoundPlans()) {
			resetTeamMonsterIdArray();
			_ongoingFightFl = false;
			totalPartyKill();
			displayAnimFrames(0xFE, true);
			return false;
		}

		for (int counter = 0; counter < _teamSize; ++counter) {
			if (_teamChar[counter]._lastAction == 0x52) // 'R'
				mainLoopCond = true;
		}

		computeInitiatives();
		displayBoxWithText("", 2, 1, false);

		for (uint counter = 0; counter < 8; ++counter) {
			int16 monsterGroupIdOrMonsterId = _initiatives[counter]._id;
			if (monsterGroupIdOrMonsterId == -1)
				continue;
			if (monsterGroupIdOrMonsterId >= 1000) { // Magic number which determines if it's a Team Member
				monsterGroupIdOrMonsterId -= 1000;
				if (!isTeamMemberStatusNormal(monsterGroupIdOrMonsterId)) {
					handleFight_checkEndEffect(monsterGroupIdOrMonsterId);
				} else {
					switch (_teamChar[monsterGroupIdOrMonsterId]._lastAction) {
					case 0x41: // 'A'ttack
						handleFight_lastAction_A(monsterGroupIdOrMonsterId);
						break;
					case 0x44: // 'D'efend
						handleFight_lastAction_D(monsterGroupIdOrMonsterId);
						break;
					case 0x48: // 'H'ide
						handleFight_lastAction_H(monsterGroupIdOrMonsterId);
						break;
					case 0x55: // 'U'se
						mainLoopCond = handleFight_lastAction_U(monsterGroupIdOrMonsterId);
						break;
					default:
						break;
					}
				}
			} else if (checkMonsterMovementType(monsterGroupIdOrMonsterId, true)) {
				handleFight_MobstersAttack(monsterGroupIdOrMonsterId);
			}
		}

		handleMapMonsterMoves();
		addNewOpponents(monsterId);
	}

	resetTeamMonsterIdArray();
	_ongoingFightFl = false;
	displayAnimFrames(0xFE, true);
	return true;
}

void EfhEngine::handleFight_checkEndEffect(int16 charId) {
	debugC(3, kDebugFight, "handleFight_checkEndEffect %d", charId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	if (_teamChar[charId]._status._type == kEfhStatusNormal)
		return;
	if (--_teamChar[charId]._status._duration > 0)
		return;

	// At this point : The status is different to 0 (normal) and the effect duration is finally 0 (end of effect)
	_enemyNamePt2 = _npcBuf[_teamChar[charId]._id]._name;
	_enemyNamePt1 = getArticle(_npcBuf[_teamChar[charId]._id].getPronoun());

	// End of effect message depends on the type of effect
	switch (_teamChar[charId]._status._type) {
	case kEfhStatusSleeping:
		_messageToBePrinted = Common::String::format("%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	case kEfhStatusFrozen:
		_messageToBePrinted = Common::String::format("%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	default:
		_messageToBePrinted = Common::String::format("%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	}

	// The character status is back to normal
	_teamChar[charId]._status._type = kEfhStatusNormal;

	// Finally, display the message
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_A(int16 teamCharId) {
	debugC(3, kDebugFight, "handleFight_lastAction_A %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.

	int16 teamCharItemId = getEquippedExclusiveType(_teamChar[teamCharId]._id, 9, true);
	if (teamCharItemId == 0x7FFF)
		teamCharItemId = 0x3F;
	int16 minMonsterGroupId = _teamChar[teamCharId]._nextAttack;
	if (minMonsterGroupId == 0x64)
		minMonsterGroupId = 0;

	if (minMonsterGroupId == -1)
		return;

	int16 maxMonsterGroupId;
	if (_items[teamCharItemId]._range == 4)
		maxMonsterGroupId = 5;
	else
		maxMonsterGroupId = minMonsterGroupId + 1;

	int16 minTeamMemberId;
	int16 maxTeamMemberId;
	if (_items[teamCharItemId]._range < 3) {
		minTeamMemberId = getWeakestMobster(minMonsterGroupId);
		maxTeamMemberId = minTeamMemberId + 1;
	} else {
		minTeamMemberId = 0;
		maxTeamMemberId = 9;
	}

	if (minTeamMemberId == -1)
		return;

	bool var6E = true;
	for (int16 groupId = minMonsterGroupId; groupId < maxMonsterGroupId; ++groupId) {
		if (_teamMonster[groupId]._id == -1)
			continue;

		for (int16 ctrMobsterId = minTeamMemberId; ctrMobsterId < maxTeamMemberId; ++ctrMobsterId) {
			if (!isMonsterActive(groupId, ctrMobsterId) || !var6E)
				return;

			bool noticedFl;
			if (!checkMonsterMovementType(groupId, true)) {
				setMapMonsterAggressivenessAndMovementType(groupId, 9);
				_alertDelay += 500;
				noticedFl = true;
			} else
				noticedFl = false;

			int16 randomDamageAbsorbed = getRandom(_mapMonsters[_techId][_teamMonster[groupId]._id]._maxDamageAbsorption);
			int16 enemyPronoun = _npcBuf[_teamChar[teamCharId]._id].getPronoun();
			int16 monsterId = _teamMonster[groupId]._id;
			int16 characterPronoun = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._nameArticle;
			int16 charScore = getCharacterScore(_teamChar[teamCharId]._id, teamCharItemId);
			int16 hitPointsBefore = _mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId];
			int16 hitCount = 0;
			int16 originalDamage = 0;
			int16 damagePointsAbsorbed = 0;
			int16 attackSpeed = _items[teamCharItemId]._attacks * _npcBuf[_teamChar[teamCharId]._id]._speed;

			// Action A - Loop attackCounter - Start
			for (int attackCounter = 0; attackCounter < attackSpeed; ++attackCounter) {
				if (getRandom(100) < charScore) {
					++hitCount;
					if (!hasAdequateDefense(_teamMonster[groupId]._id, _items[teamCharItemId]._attackType)) {
						int16 randomDamage = getRandom(_items[teamCharItemId]._damage);
						int16 residualDamage = randomDamage - randomDamageAbsorbed;
						if (residualDamage > 0) {
							originalDamage += residualDamage;
							damagePointsAbsorbed += randomDamageAbsorbed;
						} else {
							damagePointsAbsorbed += randomDamage;
						}
					}
				}
			}
			// Action A - Loop attackCounter - End

			if (originalDamage < 0)
				originalDamage = 0;

			int16 hitPoints = originalDamage + damagePointsAbsorbed;

			if (!checkSpecialItemsOnCurrentPlace(teamCharItemId))
				hitCount = 0;

			if (hitCount > 0) {
				_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] -= originalDamage;
				if (hitCount > 1) {
					_attackBuffer = Common::String::format("%d times ", hitCount);
				} else {
					_attackBuffer = "";
				}
			}
			int16 verbId = (3 * _items[teamCharItemId]._attackType + 1) + getRandom(3) - 1;

			_characterNamePt1 = getArticle(characterPronoun);
			_characterNamePt2 = kEncounters[_mapMonsters[_techId][_teamMonster[groupId]._id]._monsterRef]._name;

			_enemyNamePt1 = getArticle(enemyPronoun);
			_enemyNamePt2 = _npcBuf[_teamChar[teamCharId]._id]._name;

			_nameBuffer = _items[teamCharItemId]._name;
			if (checkSpecialItemsOnCurrentPlace(teamCharItemId)) {
				// Action A - Check damages - Start
				if (hitCount == 0) {
					_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
				} else if (hitPoints <= 0) {
					_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
				} else if (hitPoints == 1) {
					_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
					if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] <= 0) {
						getDeathTypeDescription(groupId, teamCharId + 1000);
						getXPAndSearchCorpse(_teamChar[teamCharId]._id, _enemyNamePt1, _enemyNamePt2, _teamMonster[groupId]._id);
					} else {
						_messageToBePrinted += "!";
					}
				} else {
					_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str(), hitPoints);
					if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] <= 0) {
						getDeathTypeDescription(groupId, teamCharId + 1000);
						getXPAndSearchCorpse(_teamChar[teamCharId]._id, _enemyNamePt1, _enemyNamePt2, _teamMonster[groupId]._id);
					} else {
						_messageToBePrinted += "!";
					}
				}
				// Action A - Check damages - End

				// Action A - Add reaction text - Start
				if (hitCount != 0 && originalDamage > 0 && getRandom(100) <= 35 && _mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] > 0) {
					if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] - 5 <= originalDamage) {
						addReactionText(kEfhReactionReels);
					} else if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] < hitPointsBefore / 8) {
						addReactionText(kEfhReactionCriesOut);
					} else if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] < hitPointsBefore / 4) {
						addReactionText(kEfhReactionFalters);
					} else if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] < hitPointsBefore / 2) {
						addReactionText(kEfhReactionWinces);
					} else if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] < hitPointsBefore / 3) {
						// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it. Looks like an original bug
						addReactionText(kEfhReactionScreams);
					} else if (hitPointsBefore / 8 >= originalDamage) {
						addReactionText(kEfhReactionChortles);
					} else if (originalDamage == 0 && getRandom(100) < 35) {
						// CHECKME: "originalDamage == 0" is always false as it's checked beforehand. Looks like another original bug
						addReactionText(kEfhReactionLaughs);
					}
				}
				// Action A - Add reaction text - End

				// Action A - Add armor absorb text - Start
				if (randomDamageAbsorbed && hitCount && _mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] > 0) {
					if (damagePointsAbsorbed <= 1)
						_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
					else
						_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);
				}
				// Action A - Add armor absorb text - End

				if (noticedFl)
					_messageToBePrinted += Common::String("  Your actions do not go un-noticed...");

				// Action A - Check item durability - Start
				int16 npcId = _teamChar[teamCharId]._id;

				// get equipped inventory slot with exclusiveType == 9
				uint16 exclusiveInventoryId = getEquippedExclusiveType(npcId, 9, false);
				if (exclusiveInventoryId != 0x7FFF && _npcBuf[npcId]._inventory[exclusiveInventoryId].getUsesLeft() != 0x7F) {
					int16 usesLeft = _npcBuf[npcId]._inventory[exclusiveInventoryId].getUsesLeft();
					--usesLeft;
					if (usesLeft <= 0) {
						_messageToBePrinted += Common::String::format("  * %s%s's %s breaks!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), _nameBuffer.c_str());
						setCharacterObjectToBroken(npcId, exclusiveInventoryId);
						var6E = false;
					} else {
						_npcBuf[npcId]._inventory[exclusiveInventoryId]._stat1 = (_npcBuf[npcId]._inventory[exclusiveInventoryId]._stat1 & 80) + usesLeft;
					}
				}
				// Action A - Check item durability - End

				// Action A - Check effect - Start
				if (_items[teamCharItemId]._specialEffect == 1 && _mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] > 0) {
					if (getRandom(100) < 35) {
						_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusSleeping;
						_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._duration = getRandom(10);
						_messageToBePrinted += Common::String::format("  %s%s falls asleep!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
					}
				} else if (_items[teamCharItemId]._specialEffect == 2 && _mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] > 0) {
					_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusFrozen;
					_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._duration = getRandom(10);
					_messageToBePrinted += Common::String::format("  %s%s is frozen!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
				}
				// Action A - Check effect - End
			} else {
				_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
			}

			genericGenerateSound(_items[teamCharItemId]._attackType, hitCount);
			displayBoxWithText(_messageToBePrinted, 1, 2, true);
		}
	}
}

void EfhEngine::handleFight_lastAction_D(int16 teamCharId) {
	// Fight - Action 'D' - Defend
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	debugC(3, kDebugFight, "handleFight_lastAction_D %d", teamCharId);

	_teamChar[teamCharId]._pctDodgeMiss -= 40;

	uint8 pronoun = _npcBuf[_teamChar[teamCharId]._id].getPronoun();
	_enemyNamePt1 = getArticle(pronoun);
	_enemyNamePt2 = _npcBuf[_teamChar[teamCharId]._id]._name;

	_messageToBePrinted = Common::String::format("%s%s prepares to defend %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[pronoun]);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_H(int16 teamCharId) {
	// Fight - Action 'H' - Hide
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	debugC(3, kDebugFight, "handleFight_lastAction_H %d", teamCharId);

	_teamChar[teamCharId]._pctVisible -= 50;

	int16 pronoun = _npcBuf[_teamChar[teamCharId]._id].getPronoun();
	_enemyNamePt1 = getArticle(pronoun);
	_enemyNamePt2 = _npcBuf[_teamChar[teamCharId]._id]._name;

	_messageToBePrinted = Common::String::format("%s%s attempts to hide %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[pronoun]);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

bool EfhEngine::handleFight_lastAction_U(int16 teamCharId) {
	// Fight - Action 'U' - Use Item
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	debugC(3, kDebugFight, "handleFight_lastAction_U %d", teamCharId);

	int16 itemId = _npcBuf[_teamChar[teamCharId]._id]._inventory[_teamChar[teamCharId]._lastInventoryUsed]._ref;
	_nameBuffer = _items[itemId]._name;

	int16 pronoun = _npcBuf[_teamChar[teamCharId]._id].getPronoun();
	_enemyNamePt1 = getArticle(pronoun);
	_enemyNamePt2 = _npcBuf[_teamChar[teamCharId]._id]._name;

	_messageToBePrinted = Common::String::format("%s%s uses %s %s!  ", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[pronoun], _nameBuffer.c_str());
	bool retVal = useObject(_teamChar[teamCharId]._id, _teamChar[teamCharId]._lastInventoryUsed, _teamChar[teamCharId]._nextAttack, teamCharId, 0, 3);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);

	return retVal;
}

void EfhEngine::handleFight_MobstersAttack(int groupId) {
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	debugC(3, kDebugFight, "handleFight_MobstersAttack %d", groupId);

	// handleFight - Loop on mobsterId - Start
	for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
		if (isMonsterActive(groupId, ctrMobsterId)) {
			int16 monsterWeaponItemId = _mapMonsters[_techId][_teamMonster[groupId]._id]._weaponItemId;
			if (monsterWeaponItemId == 0xFF)
				monsterWeaponItemId = 0x3F;
			int16 minTeamMemberId = -1;
			int16 maxTeamMemberId;
			if (_items[monsterWeaponItemId]._range < 3) {
				for (uint attackTry = 0; attackTry < 10; ++attackTry) {
					minTeamMemberId = getRandom(_teamSize) - 1;
					if (checkWeaponRange(_teamMonster[groupId]._id, monsterWeaponItemId) && isTeamMemberStatusNormal(minTeamMemberId) && getRandom(100) < _teamChar[minTeamMemberId]._pctVisible) {
						break;
					}
					minTeamMemberId = -1;
				}
				maxTeamMemberId = minTeamMemberId + 1;
			} else {
				minTeamMemberId = 0;
				maxTeamMemberId = _teamSize;
			}

			if (minTeamMemberId <= -1)
				continue;

			// handleFight - Loop on targetId - Start
			for (int16 targetId = minTeamMemberId; targetId < maxTeamMemberId; ++targetId) {
				if (_teamChar[targetId]._id == -1 || !isTeamMemberStatusNormal(targetId))
					continue;

				int16 randomDefense = getRandom(getEquipmentDefense(_teamChar[targetId]._id));

				int16 enemyPronoun = kEncounters[_mapMonsters[_techId][_teamMonster[groupId]._id]._monsterRef]._nameArticle;
				int16 characterPronoun = _npcBuf[_teamChar[targetId]._id].getPronoun();

				_teamChar[targetId]._pctDodgeMiss += (_items[monsterWeaponItemId]._agilityModifier * 5);
				int16 hitCount = 0;
				int16 originalDamage = 0;
				int16 damagePointsAbsorbed = 0;

				int16 var64 = _mapMonsters[_techId][_teamMonster[groupId]._id]._npcId * _items[monsterWeaponItemId]._attacks;
				for (int var84 = 0; var84 < var64; ++var84) {
					// handleFight - Loop var84 on var64 (objectId) - Start
					if (getRandom(100) > _teamChar[targetId]._pctDodgeMiss)
						continue;

					++hitCount;

					if (hasAdequateDefenseNPC(_teamChar[targetId]._id, _items[monsterWeaponItemId]._attackType))
						continue;

					int16 baseDamage = getRandom(_items[monsterWeaponItemId]._damage);
					int deltaDamage = baseDamage - randomDefense;

					if (deltaDamage > 0) {
						damagePointsAbsorbed += randomDefense;
						originalDamage += deltaDamage;
					} else {
						damagePointsAbsorbed += baseDamage;
					}
					// handleFight - Loop var84 on var64 (objectId) - End
				}

				if (originalDamage < 0)
					originalDamage = 0;

				int16 hitPoints = originalDamage + damagePointsAbsorbed;
				if (!checkSpecialItemsOnCurrentPlace(monsterWeaponItemId))
					hitCount = 0;

				if (hitCount > 0) {
					_npcBuf[_teamChar[targetId]._id]._hitPoints -= originalDamage;
					if (hitCount > 1)
						_attackBuffer = Common::String::format("%d times ", hitCount);
					else
						_attackBuffer = "";
				}

				int16 var68 = _items[monsterWeaponItemId]._attackType + 1;
				int16 var6A = getRandom(3);

				_enemyNamePt1 = getArticle(enemyPronoun);
				_enemyNamePt2 = kEncounters[_mapMonsters[_techId][_teamMonster[groupId]._id]._monsterRef]._name;

				_characterNamePt1 = getArticle(characterPronoun);
				_characterNamePt2 = _npcBuf[_teamChar[targetId]._id]._name;

				_nameBuffer = _items[monsterWeaponItemId]._name;
				if (checkSpecialItemsOnCurrentPlace(monsterWeaponItemId)) {
					// handleFight - check damages - Start
					if (hitCount == 0) {
						_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
					} else if (hitPoints <= 0) {
						_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
					} else if (hitPoints == 1) {
						_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
						if (_npcBuf[_teamChar[targetId]._id]._hitPoints <= 0)
							getDeathTypeDescription(targetId + 1000, groupId);
						else
							_messageToBePrinted += "!";
					} else {
						_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str(), hitPoints);
						if (_npcBuf[_teamChar[targetId]._id]._hitPoints <= 0)
							getDeathTypeDescription(targetId + 1000, groupId);
						else
							_messageToBePrinted += "!";
					}
					// handleFight - check damages - End

					// handleFight - Add reaction text - start
					if (hitCount != 0 && originalDamage > 0 && getRandom(100) <= 35 && _npcBuf[_teamChar[targetId]._id]._hitPoints > 0) {
						if (_npcBuf[_teamChar[targetId]._id]._hitPoints - 5 <= originalDamage) {
							addReactionText(kEfhReactionReels);
						} else if (_npcBuf[_teamChar[targetId]._id]._hitPoints < _npcBuf[_teamChar[targetId]._id]._maxHP / 8) {
							addReactionText(kEfhReactionCriesOut);
						} else if (_npcBuf[_teamChar[targetId]._id]._hitPoints < _npcBuf[_teamChar[targetId]._id]._maxHP / 4) {
							addReactionText(kEfhReactionFalters);
						} else if (_npcBuf[_teamChar[targetId]._id]._hitPoints < _npcBuf[_teamChar[targetId]._id]._maxHP / 2) {
							addReactionText(kEfhReactionWinces);
						} else if (_npcBuf[_teamChar[targetId]._id]._hitPoints < _npcBuf[_teamChar[targetId]._id]._maxHP / 3) {
							// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it. Looks like an original bug
							addReactionText(kEfhReactionScreams);
						} else if (_npcBuf[_teamChar[targetId]._id]._maxHP / 8 >= originalDamage) {
							addReactionText(kEfhReactionChortles);
						} else if (originalDamage == 0 && getRandom(100) < 35) {
							// CHECKME: "originalDamage == 0" is always false as it's checked beforehand. Looks like another original bug
							addReactionText(kEfhReactionLaughs);
						}
					}
					// handleFight - Add reaction text - end

					// handleFight - Check armor - start
					if (randomDefense != 0 && hitCount != 0 && _npcBuf[_teamChar[targetId]._id]._hitPoints > 0) {
						if (damagePointsAbsorbed <= 1)
							_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
						else
							_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);

						int armorDamage = (originalDamage + damagePointsAbsorbed) / 10;
						handleDamageOnArmor(_teamChar[targetId]._id, armorDamage);
					}
					// handleFight - Check armor - end

					// handleFight - Check effect - start
					switch (_items[monsterWeaponItemId]._specialEffect) {
					case 1:
						if (getRandom(100) < 20) {
							_teamChar[targetId]._status._type = kEfhStatusSleeping;
							_teamChar[targetId]._status._duration = getRandom(10);
							_messageToBePrinted += Common::String::format("  %s%s falls asleep!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
						}
						break;
					case 2:
						if (getRandom(100) < 20) {
							_teamChar[targetId]._status._type = kEfhStatusFrozen;
							_teamChar[targetId]._status._duration = getRandom(10);
							_messageToBePrinted += Common::String::format("  %s%s is frozen!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
						}
						break;
					case 5:
					case 6:
						if (getRandom(100) < 20) {
							_messageToBePrinted += Common::String::format("  %s%s's life energy is gone!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
							_npcBuf[_teamChar[targetId]._id]._hitPoints = 0;
						}
						break;
					default:
						break;
					}
					// handleFight - Check effect - end
				} else {
					_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[enemyPronoun], _nameBuffer.c_str());
				}
				genericGenerateSound(_items[monsterWeaponItemId]._attackType, hitCount);
				displayBoxWithText(_messageToBePrinted, 1, 2, true);
			}
			// handleFight - Loop on targetId - End
		} else if (_mapMonsters[_techId][_teamMonster[groupId]._id]._hitPoints[ctrMobsterId] > 0 && _teamMonster[groupId]._mobsterStatus[ctrMobsterId]._type != kEfhStatusNormal) {
			--_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._duration;
			if (_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._duration <= 0) {
				_enemyNamePt1 = getArticle(kEncounters[_mapMonsters[_techId][_teamMonster[groupId]._id]._monsterRef]._nameArticle);
				_enemyNamePt2 = kEncounters[_mapMonsters[_techId][_teamMonster[groupId]._id]._monsterRef]._name;

				switch (_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._type) {
				case kEfhStatusSleeping:
					_messageToBePrinted = Common::String::format("%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
					break;
				case kEfhStatusFrozen:
					_messageToBePrinted = Common::String::format("%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
					break;
				default:
					_messageToBePrinted = Common::String::format("%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
					break;
				}
				_teamMonster[groupId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusNormal;
				displayBoxWithText(_messageToBePrinted, 1, 2, true);
			}
		}
	}
	// handleFight - Loop on mobsterId - End
}

bool EfhEngine::isTPK() {
	debugC(6, kDebugFight, "isTPK");

	int16 zeroedChar = 0;
	for (int counter = 0; counter < _teamSize; ++counter) {
		if (_npcBuf[_teamChar[counter]._id]._hitPoints <= 0)
			++zeroedChar;
	}

	return zeroedChar == _teamSize;
}

bool EfhEngine::isMonsterAlreadyFighting(int16 monsterId, int16 teamMonsterId) {
	debugC(6, kDebugFight, "isMonsterAlreadyFighting %d %d", monsterId, teamMonsterId);

	for (int counter = 0; counter < teamMonsterId; ++counter) {
		if (_teamMonster[counter]._id == monsterId)
			return true;
	}
	return false;
}

void EfhEngine::resetTeamMonsterEffects() {
	debugC(6, kDebugFight, "resetTeamMonsterEffects");
	for (uint ctrGroupId = 0; ctrGroupId < 5; ++ctrGroupId) {
		for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
			_teamMonster[ctrGroupId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusNormal;
			_teamMonster[ctrGroupId]._mobsterStatus[ctrMobsterId]._duration = 0;
		}
	}
}

void EfhEngine::resetTeamMonsterIdArray() {
	debugC(6, kDebugFight, "resetTeamMonsterIdArray");

	for (int i = 0; i < 5; ++i) {
		_teamMonster[i]._id = -1;
	}
}

bool EfhEngine::isTeamMemberStatusNormal(int16 teamMemberId) {
	debugC(6, kDebugFight, "isTeamMemberStatusNormal %d", teamMemberId);

	if (_npcBuf[_teamChar[teamMemberId]._id]._hitPoints > 0 && _teamChar[teamMemberId]._status._type == kEfhStatusNormal)
		return true;

	return false;
}

void EfhEngine::getDeathTypeDescription(int16 victimId, int16 attackerId) {
	debugC(3, kDebugFight, "getDeathTypeDescription %d %d", victimId, attackerId);

	uint8 pronoun = 0;

	if (victimId >= 1000) { // Magic value for team members
		int16 charId = _teamChar[victimId - 1000]._id;
		pronoun = _npcBuf[charId].getPronoun();
	} else if (victimId < 5) { // Safeguard added
		int16 charId = _teamMonster[victimId]._id;
		pronoun = _mapMonsters[_techId][charId].getPronoun();
	}

	if (pronoun > 2)
		pronoun = 2;

	int16 deathType;
	if (getRandom(100) < 20) {
		deathType = 0;
	} else if (attackerId >= 1000) {
		int16 charId = _teamChar[attackerId - 1000]._id;
		if (charId == -1)
			deathType = 0;
		else {
			int16 exclusiveItemId = getEquippedExclusiveType(charId, 9, true);
			if (exclusiveItemId == 0x7FFF)
				deathType = 0;
			else
				deathType = _items[exclusiveItemId]._attackType + 1;
		}
	// The check "attackerId >= 5" is a safeguard for a Coverity "OVERRUN" ticket, not present in the original
	} else if (attackerId >= 5 || _teamMonster[attackerId]._id == -1) {
		deathType = 0;
	} else {
		int16 itemId = _mapMonsters[_techId][_teamMonster[attackerId]._id]._weaponItemId;
		deathType = _items[itemId]._attackType + 1;
	}

	int16 rndDescrForDeathType = getRandom((3)) - 1; // [0..2]
	Common::String tmpStr = "DUDE IS TOAST!";
	switch (deathType) {
	case 0:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", killing %s!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", slaughtering %s!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", annihilating %s!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", cutting %s in two!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", dicing %s into small cubes!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", butchering %s into lamb chops!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", piercing %s heart!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", leaving %s a spouting mass of blood!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", popping %s like a zit!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 3:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", pulping %s head over a wide area!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", smashing %s into a meat patty!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", squashing %s like a ripe tomato!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 4:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", totally incinerating %s!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", reducing %s to a pile of ash!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", leaving a blistered mass of flesh behind!");
			break;
		default:
			break;
		}
		break;
	case 5:
		switch (rndDescrForDeathType) {
		case 0:
			// The original has a typo: popscicle
			tmpStr = Common::String::format(", turning %s into a popsicle!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", encasing %s in a block of ice!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", shattering %s into shards!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 6:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", leaving pudding for brains");
			break;
		case 1:
			tmpStr = Common::String::format(", bursting %s head like a bubble!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", turning %s into a mindless vegetable", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 7:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", reducing %s to an oozing pile of flesh!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", melting %s like an ice cube in hot coffee!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", vaporizing %s into a steaming cloud!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 8:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", engulfing %s in black smoke puffs!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", sucking %s into eternity!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", turning %s into a mindless zombie!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 9:
	case 10:
	case 11:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", completely disintegrating %s!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", spreading %s into a fine mist!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", leaving a smoking crater in %s place!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 12:
	case 13:
	case 14:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", tearing a chunk out of %s back!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", blowing %s brains out!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", exploding %s entire chest!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 15:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", choking %s to death!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", melting %s lungs!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", leaving %s gasping for air as %s collapses!", kPersonal[pronoun], kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	case 16:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", tearing a chunk out of %s back!", kPersonal[pronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", piercing %s heart!", kPersonal[pronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", impaling %s brain!", kPersonal[pronoun]);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	_messageToBePrinted += tmpStr;
}

int16 EfhEngine::determineTeamTarget(int16 charId, int16 unkFied18Val, bool checkDistanceFl) {
	debugC(3, kDebugFight, "determineTeamTarget %d %d %d", charId, unkFied18Val, checkDistanceFl);

	int16 retVal = -1;

	int16 curItemId = getEquippedExclusiveType(charId, unkFied18Val, true);
	int16 rangeType = 0;
	int16 realRange = 0;
	if (curItemId != 0x7FFF)
		rangeType = _items[curItemId]._range;

	switch (rangeType) {
	case 3:
	case 2:
		++realRange;
		// fall through
	case 1:
		++realRange;
		// fall through
	case 0:
		++realRange;
		break;
	case 4:
		return 100;
	default:
		return retVal;
	}

	do {
		for (uint counter = 0; counter < 2; ++counter) {
			drawCombatScreen(charId, true, false);
			if (_teamMonster[1]._id != -1)
				displayBoxWithText("Select Monster Group:", 3, 0, false);

			if (counter == 0)
				displayFctFullScreen();
		}

		retVal = (_teamMonster[1]._id == -1) ? 0 : selectMonsterGroup();

		if (!checkDistanceFl) {
			if (retVal == 27) // Esc
				retVal = 0;
		} else if (retVal != 27) {
			int16 monsterGroupDistance = computeMonsterGroupDistance(_teamMonster[retVal]._id);
			if (monsterGroupDistance > realRange) {
				retVal = 27;
				displayBoxWithText("That Group Is Out Of Range!", 3, 1, false);
				getLastCharAfterAnimCount(_guessAnimationAmount);
			}
		}
	} while (retVal == -1);

	if (retVal == 27)
		retVal = -1;

	return retVal;
}

bool EfhEngine::getTeamAttackRoundPlans() {
	debugC(3, kDebugFight, "getTeamAttackRoundPlans");

	bool retVal = false;
	for (int charId = 0; charId < _teamSize; ++charId) {
		_teamChar[charId]._lastAction = 0;
		if (!isTeamMemberStatusNormal(charId))
			continue;

		retVal = true;
		do {
			drawCombatScreen(_teamChar[charId]._id, false, true);
			switch (handleAndMapInput(true)) {
			case Common::KEYCODE_a: // Attack
				_teamChar[charId]._lastAction = 'A';
				_teamChar[charId]._nextAttack = determineTeamTarget(_teamChar[charId]._id, 9, true);
				if (_teamChar[charId]._nextAttack == -1)
					_teamChar[charId]._lastAction = 0;
				break;
			case Common::KEYCODE_d: // Defend
				_teamChar[charId]._lastAction = 'D';
				break;
			case Common::KEYCODE_h: // Hide
				_teamChar[charId]._lastAction = 'H';
				break;
			case Common::KEYCODE_r: // Run
				for (int counter2 = 0; counter2 < _teamSize; ++counter2) {
					_teamChar[counter2]._lastAction = 'R';
				}
				return true;
			case Common::KEYCODE_s: { // Status
				int16 lastInvId = handleStatusMenu(2, _teamChar[charId]._id);
				redrawCombatScreenWithTempText(_teamChar[charId]._id);
				if (lastInvId >= 999) {
					if (lastInvId == 0x7D00) // Result of Equip, Give and Drop in combat mode(2)
						_teamChar[charId]._lastAction = 'S';
				} else {
					_teamChar[charId]._lastAction = 'U';
					_teamChar[charId]._lastInventoryUsed = lastInvId;
					int16 invEffect = _items[_npcBuf[_teamChar[charId]._id]._inventory[lastInvId]._ref]._specialEffect;
					switch (invEffect - 1) {
					case 0:
					case 1:
					case 2:
					case 3:
					case 4:
					case 5:
					case 6:
					case 7:
					case 8:
					case 10:
					case 12:
					case 13:
						_teamChar[charId]._nextAttack = determineTeamTarget(_teamChar[charId]._id, 9, false);
						break;

					case 9:
					case 11:
					case 14:
					case 15:
					case 18:
					case 24:
					case 25:
					case 27:
					case 28:
					case 29:
					case 30:
						displayBoxWithText("Select Character:", 3, 1, false);
						_teamChar[charId]._nextAttack = selectOtherCharFromTeam();
						break;

					case 16:
					case 17:
					case 26:
						_teamChar[charId]._nextAttack = 0xC8;
						break;

					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					default:
						_teamChar[charId]._lastInventoryUsed = lastInvId;
						_teamChar[charId]._nextAttack = -1;
						break;
					}
				}

			} break;
			case Common::KEYCODE_t: // Terrain
				redrawScreenForced();
				getInputBlocking();
				drawCombatScreen(_teamChar[charId]._id, false, true);
				break;
			default:
				break;
			}
		} while (_teamChar[charId]._lastAction == 0);
	}

	return retVal;
}

void EfhEngine::drawCombatScreen(int16 charId, bool whiteFl, bool drawFl) {
	debugC(6, kDebugFight, "drawCombatScreen %d %s %s", charId, whiteFl ? "True" : "False", drawFl ? "True" : "False");

	for (uint counter = 0; counter < 2; ++counter) {
		if (counter == 0 || drawFl) {
			drawMapWindow();
			displayCenteredString("Combat", 128, 303, 9);
			drawColoredRect(200, 112, 278, 132, 0);
			displayCenteredString("'T' for Terrain", 128, 303, 117);
			displayBoxWithText("", 1, 0, false);
			displayEncounterInfo(whiteFl);
			displayCombatMenu(charId);
			displayLowStatusScreen(false);
		}

		if (counter == 0 && drawFl)
			displayFctFullScreen();
	}
}

void EfhEngine::getXPAndSearchCorpse(int16 charId, Common::String namePt1, Common::String namePt2, int16 monsterId) {
	debugC(3, kDebugFight, "getXPAndSearchCorpse %d %s%s %d", charId, namePt1.c_str(), namePt2.c_str(), monsterId);

	uint16 oldXpLevel = getXPLevel(_npcBuf[charId]._xp);
	_npcBuf[charId]._xp += kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._xpGiven;

	if (getXPLevel(_npcBuf[charId]._xp) > oldXpLevel) {
		generateSound(15);
		int16 hpGain = getRandom(20) + getRandom(_npcBuf[charId]._infoScore[4]); // "Stamina"
		_npcBuf[charId]._hitPoints += hpGain;
		_npcBuf[charId]._maxHP += hpGain;
		// "Strength",
		_npcBuf[charId]._infoScore[0] += getRandom(3) - 1;
		// "Intelligence",
		_npcBuf[charId]._infoScore[1] += getRandom(3) - 1;
		// "Piety",
		_npcBuf[charId]._infoScore[2] += getRandom(3) - 1;
		// "Agility",
		_npcBuf[charId]._infoScore[3] += getRandom(3) - 1;
		// "Stamina",
		_npcBuf[charId]._infoScore[4] += getRandom(3) - 1;
	}

	_messageToBePrinted += Common::String::format("  %s%s gains %d experience", namePt1.c_str(), namePt2.c_str(), kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._xpGiven);
	if (!characterSearchesMonsterCorpse(charId, monsterId))
		_messageToBePrinted += "!";
}

bool EfhEngine::characterSearchesMonsterCorpse(int16 charId, int16 monsterId) {
	debugC(3, kDebugFight, "characterSearchesMonsterCorpse %d %d", charId, monsterId);

	int16 rndVal = getRandom(100);
	if (kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._dropOccurrencePct < rndVal)
		return false;

	rndVal = getRandom(5) - 1;
	int16 itemId = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._dropItemId[rndVal];
	if (itemId == -1 || itemId == 0)
		return false;

	if (!giveItemTo(charId, itemId, 0xFF))
		return false;

	_messageToBePrinted += Common::String::format(" and finds a %s!", _items[itemId]._name);
	return true;
}

void EfhEngine::addReactionText(int16 id) {
	debugC(3, kDebugFight, "addReactionText %d", id);

	int16 rand3 = getRandom(3);

	switch (id) {
	case kEfhReactionReels:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s reels from the blow!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s sways from the attack!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s looks dazed!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionCriesOut:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s cries out in agony!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s screams from the abuse!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s wails terribly!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionFalters:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s is staggering!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s falters for a moment!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s is stumbling about!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionWinces:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s winces from the pain!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s cringes from the damage!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s shrinks from the wound!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionScreams:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s screams!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s bellows!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s shrills!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionChortles:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s chortles!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s seems amused!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s looks concerned!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	case kEfhReactionLaughs:
		switch (rand3) {
		case 1:
			_messageToBePrinted += Common::String::format("  %s%s laughs at the feeble attack!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 2:
			_messageToBePrinted += Common::String::format("  %s%s smiles at the pathetic attack!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		case 3:
			_messageToBePrinted += Common::String::format("  %s%s laughs at the ineffective assault!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void EfhEngine::displayEncounterInfo(bool whiteFl) {
	debugC(5, kDebugFight, "displayEncounterInfo %s", whiteFl ? "True" : "False");

	int16 textPosY = 20;
	for (uint counter = 0; counter < 5; ++counter) {
		if (_teamMonster[counter]._id == -1)
			continue;

		int16 monsterDistance = computeMonsterGroupDistance(_teamMonster[counter]._id);
		int16 mobsterCount = countMonsterGroupMembers(counter);
		if (whiteFl)
			setTextColorWhite();
		else
			setTextColorGrey();

		setTextPos(129, textPosY);
		Common::String buffer = Common::String::format("%c)", 'A' + counter);
		displayStringAtTextPos(buffer);
		setTextColorRed();
		int16 var1 = _mapMonsters[_techId][_teamMonster[counter]._id]._possessivePronounSHL6 & 0x3F;
		if (var1 <= 0x3D) {
			buffer = Common::String::format("%d %s", mobsterCount, kEncounters[_mapMonsters[_techId][_teamMonster[counter]._id]._monsterRef]._name);
			displayStringAtTextPos(buffer);
			if (mobsterCount > 1)
				displayStringAtTextPos("s");
		} else if (var1 == 0x3E) {
			displayStringAtTextPos("(NOT DEFINED)");
		} else if (var1 == 0x3F) {
			Common::String stringToDisplay = _npcBuf[_mapMonsters[_techId][_teamMonster[counter]._id]._npcId]._name;
			displayStringAtTextPos(stringToDisplay);
		}

		setTextPos(228, textPosY);
		if (checkMonsterMovementType(counter, true)) {
			_textColor = 0xE;
			displayStringAtTextPos("Hostile");
		} else {
			_textColor = 0x2;
			displayStringAtTextPos("Friendly");
		}

		setTextColorRed();
		switch (monsterDistance) {
		case 1:
			displayCenteredString("S", 290, 302, textPosY);
			break;
		case 2:
			displayCenteredString("M", 290, 302, textPosY);
			break;
		case 3:
			displayCenteredString("L", 290, 302, textPosY);
			break;
		default:
			displayCenteredString("?", 290, 302, textPosY);
			break;
		}

		textPosY += 9;
	}
}

int16 EfhEngine::getWeakestMobster(int16 groupNumber) {
	debugC(3, kDebugFight, "getWeakestMobster %d", groupNumber);

	int16 weakestMobsterId = -1;
	int16 monsterId = _teamMonster[groupNumber]._id;

	if (monsterId == -1)
		return -1;

	for (uint counter = 0; counter < 9; ++counter) {
		if (isMonsterActive(groupNumber, counter)) {
			weakestMobsterId = counter;
			break;
		}
	}

	//Safeguard added
	if (weakestMobsterId < 0)
		return -1;

	for (int16 counter = weakestMobsterId + 1; counter < 9; ++counter) {
		if (!isMonsterActive(groupNumber, counter))
			continue;

		if (_mapMonsters[_techId][monsterId]._hitPoints[weakestMobsterId] > _mapMonsters[_techId][monsterId]._hitPoints[counter])
			weakestMobsterId = counter;
	}

	// Useless check on _hitPoints > 0 removed. It's covered by isMonsterActive()

	return weakestMobsterId;
}

int16 EfhEngine::getCharacterScore(int16 charId, int16 itemId) {
	debugC(3, kDebugFight, "getCharacterScore %d %d", charId, itemId);

	int16 totalScore = 0;
	switch (_items[itemId]._range) {
	case 0:
		totalScore = _npcBuf[charId]._passiveScore[5] + _npcBuf[charId]._passiveScore[3] + _npcBuf[charId]._passiveScore[4];
		totalScore += _npcBuf[charId]._infoScore[0] / 5;
		totalScore += _npcBuf[charId]._infoScore[2] * 2,
		totalScore += _npcBuf[charId]._infoScore[6] / 5;
		totalScore += 2 * _npcBuf[charId]._infoScore[5] / 5;
		break;
	case 1:
		totalScore = _npcBuf[charId]._passiveScore[3] + _npcBuf[charId]._passiveScore[4];
		totalScore += _npcBuf[charId]._infoScore[2] * 2;
		totalScore += _npcBuf[charId]._infoScore[1] / 5;
		totalScore += _npcBuf[charId]._infoScore[3] / 5;
		break;
	case 2:
	case 3:
	case 4:
		totalScore = _npcBuf[charId]._passiveScore[1];
		totalScore += _npcBuf[charId]._infoScore[2] * 2;
		totalScore += _npcBuf[charId]._infoScore[1] / 5;
		totalScore += _npcBuf[charId]._infoScore[3] / 5;
		totalScore += _npcBuf[charId]._infoScore[8] / 5;
	default:
		break;
	}

	int16 extraScore = 0;
	switch (_items[itemId]._attackType) {
	case 0:
	case 1:
	case 2:
		if (itemId == 0x3F)
			extraScore = _npcBuf[charId]._passiveScore[2];
		else if (itemId == 0x41 || itemId == 0x42 || itemId == 0x6A || itemId == 0x6C || itemId == 0x6D)
			extraScore = _npcBuf[charId]._passiveScore[0];
		break;
	case 3:
	case 4:
	case 6:
		extraScore = _npcBuf[charId]._infoScore[7];
		break;
	case 5:
	case 7:
		extraScore = _npcBuf[charId]._infoScore[9];
		break;
	case 8:
	case 9:
		extraScore = _npcBuf[charId]._activeScore[12];
		break;
	case 10:
		extraScore = _npcBuf[charId]._passiveScore[10];
		break;
	case 11:
		extraScore = _npcBuf[charId]._passiveScore[6];
		break;
	case 12:
		extraScore = _npcBuf[charId]._passiveScore[7];
		break;
	case 13:
		extraScore = _npcBuf[charId]._passiveScore[8];
		break;
	case 14:
		extraScore = _npcBuf[charId]._activeScore[13];
		break;
	case 15:
		extraScore = _npcBuf[charId]._passiveScore[9];
		break;
	default:
		break;
	}

	extraScore += _items[itemId]._agilityModifier;

	int16 grandTotalScore = CLIP(totalScore + extraScore + 30, 5, 90);

	return grandTotalScore;
}

bool EfhEngine::checkSpecialItemsOnCurrentPlace(int16 itemId) {
	debugC(3, kDebugFight, "checkSpecialItemsOnCurrentPlace %d", itemId);

	bool retVal = true;
	switch (_techDataArr[_techId][_techDataId_MapPosX * 64 + _techDataId_MapPosY]) {
	case 1:
		if ((itemId >= 0x58 && itemId <= 0x68) || (itemId >= 0x86 && itemId <= 0x89) || (itemId >= 0x74 && itemId <= 0x76) || itemId == 0x8C)
			retVal = false;
		break;
	case 2:
		if ((itemId >= 0x61 && itemId <= 0x63) || (itemId >= 0x74 && itemId <= 0x76) || (itemId >= 0x86 && itemId <= 0x89) || itemId == 0x5B || itemId == 0x5E || itemId == 0x66 || itemId == 0x68 || itemId == 0x8C)
			retVal = false;
		break;
	default:
		break;
	}

	return retVal;
}

bool EfhEngine::hasAdequateDefense(int16 monsterId, uint8 attackType) {
	debugC(3, kDebugFight, "hasAdequateDefense %d %d", monsterId, attackType);

	int16 itemId = _mapMonsters[_techId][monsterId]._weaponItemId;

	if (_items[itemId]._specialEffect != 0)
		return false;

	return _items[itemId]._defenseType == attackType;
}

bool EfhEngine::hasAdequateDefenseNPC(int16 charId, uint8 attackType) {
	debugC(3, kDebugFight, "hasAdequateDefenseNPC %d %d", charId, attackType);

	int16 itemId = _npcBuf[charId]._defaultDefenseItemId;

	if (_items[itemId]._specialEffect == 0 && _items[itemId]._defenseType == attackType)
		return true;

	for (uint counter = 0; counter < 10; ++counter) {
		if (_npcBuf[charId]._inventory[counter]._ref == 0x7FFF || !_npcBuf[charId]._inventory[counter].isEquipped())
			continue;

		itemId = _npcBuf[charId]._inventory[counter]._ref;
		if (_items[itemId]._specialEffect == 0 && _items[itemId]._defenseType == attackType)
			return true;
	}
	return false;
}

// The parameter isn't used in the original
void EfhEngine::addNewOpponents(int16 monsterId) {
	debugC(3, kDebugFight, "addNewOpponents %d", monsterId);

	// addNewOpponents - 1rst loop counter1_monsterId - Start
	for (uint ctrGroupId = 0; ctrGroupId < 5; ++ctrGroupId) {
		if (countMonsterGroupMembers(ctrGroupId))
			continue;

		for (uint ctrMobster = 0; ctrMobster < 9; ++ctrMobster) {
			_mapMonsters[_techId][_teamMonster[ctrGroupId]._id]._hitPoints[ctrMobster] = 0;
			_teamMonster[ctrGroupId]._mobsterStatus[ctrMobster]._type = kEfhStatusNormal;
			_teamMonster[ctrGroupId]._mobsterStatus[ctrMobster]._duration = 0;
		}

		_teamMonster[ctrGroupId]._id = -1;

		// CHECKME: ctrGroupId is not incrementing, which is very, very suspicious as we are copying over and over to the same destination
		// if the purpose is compact the array, it should be handle differently
		for (uint counter2 = ctrGroupId + 1; counter2 < 5; ++counter2) {
			for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
				_teamMonster[ctrGroupId]._mobsterStatus[ctrMobsterId]._type = _teamMonster[counter2]._mobsterStatus[ctrMobsterId]._type;
				_teamMonster[ctrGroupId]._mobsterStatus[ctrMobsterId]._duration = _teamMonster[counter2]._mobsterStatus[ctrMobsterId]._duration;
			}
			_teamMonster[ctrGroupId]._id = _teamMonster[counter2]._id;
		}
	}
	// addNewOpponents - 1rst loop counter1_monsterId - End

	int16 teamMonsterId = -1;
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		if (_teamMonster[counter1]._id == -1) {
			teamMonsterId = counter1;
			break;
		}
	}

	if (teamMonsterId != -1) {
		// addNewOpponents - loop distCtr - Start
		for (int distCtr = 1; distCtr < 3; ++distCtr) {
			if (teamMonsterId >= 5)
				break;

			for (uint ctrMapMonsterId = 0; ctrMapMonsterId < 64; ++ctrMapMonsterId) {
				if (_mapMonsters[_techId][ctrMapMonsterId]._fullPlaceId == 0xFF)
					continue;

				if (((_mapMonsters[_techId][ctrMapMonsterId]._possessivePronounSHL6 & 0x3F) == 0x3F && !isNpcATeamMember(_mapMonsters[_techId][ctrMapMonsterId]._npcId)) || (_mapMonsters[_techId][ctrMapMonsterId]._possessivePronounSHL6 & 0x3F) <= 0x3D) {
					if (checkIfMonsterOnSameLargeMapPlace(ctrMapMonsterId)) {
						bool monsterActiveFound = false;
						for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
							if (_mapMonsters[_techId][ctrMapMonsterId]._hitPoints[ctrMobsterId] > 0) {
								monsterActiveFound = true;
								break;
							}
						}

						if (!monsterActiveFound)
							continue;

						if (computeMonsterGroupDistance(ctrMapMonsterId) > distCtr)
							continue;

						if (isMonsterAlreadyFighting(ctrMapMonsterId, teamMonsterId))
							continue;

						_teamMonster[teamMonsterId]._id = ctrMapMonsterId;

						// The original at this point was doing a loop on counter1, which is not a good idea as
						// it was resetting the counter1 to 9 whatever its value before the loop.
						// I therefore decided to use another counter as it looks like an original misbehavior/bug.
						for (uint ctrMobsterId = 0; ctrMobsterId < 9; ++ctrMobsterId) {
							_teamMonster[teamMonsterId]._mobsterStatus[ctrMobsterId]._type = kEfhStatusNormal;
						}

						if (++teamMonsterId >= 5)
							break;
					}
				}
			}
		}
		// addNewOpponents - loop distCtr - End
	}

	if (teamMonsterId == -1 || teamMonsterId > 4)
		return;

	// Reset the unused groups
	for (int16 ctrTeamMonsterId = teamMonsterId; ctrTeamMonsterId < 5; ++ctrTeamMonsterId)
		_teamMonster[ctrTeamMonsterId].init();
}

int16 EfhEngine::getTeamMonsterAnimId() {
	debugC(6, kDebugFight, "getTeamMonsterAnimId");

	int16 retVal = 0xFF;
	for (uint counter = 0; counter < 5; ++counter) {
		int16 monsterId = _teamMonster[counter]._id;
		if (monsterId == -1)
			continue;

		if (!checkMonsterMovementType(monsterId, false))
			continue;

		retVal = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._animId;
		break;
	}

	if (retVal == 0xFF)
		retVal = kEncounters[_mapMonsters[_techId][_teamMonster[0]._id]._monsterRef]._animId;

	return retVal;
}

int16 EfhEngine::selectMonsterGroup() {
	debugC(3, kDebugFight, "selectMonsterGroup");

	int16 retVal = -1;

	while (retVal == -1) {
		Common::KeyCode input = handleAndMapInput(true);
		switch (input) {
		case Common::KEYCODE_ESCAPE:
			retVal = 27;
			break;
		case Common::KEYCODE_a:
		case Common::KEYCODE_b:
		case Common::KEYCODE_c:
		case Common::KEYCODE_d:
		case Common::KEYCODE_e:
			retVal = input - Common::KEYCODE_a;
			if (_teamMonster[retVal]._id == -1)
				retVal = -1;
			break;
		default:
			break;
		}
	}

	return retVal;
}

void EfhEngine::redrawCombatScreenWithTempText(int16 charId) {
	debugC(3, kDebugFight, "redrawCombatScreenWithTempText %d", charId);

	for (uint counter = 0; counter < 2; ++counter) {
		drawGameScreenAndTempText(false);
		displayLowStatusScreen(false);
		drawCombatScreen(charId, false, false);
		if (counter == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::handleDamageOnArmor(int16 charId, int16 damage) {
	debugC(3, kDebugFight, "handleDamageOnArmor %d %d", charId, damage);

	int16 destroyCounter = 0;
	int16 pronoun = _npcBuf[charId].getPronoun();

	if (pronoun > 2) {
		pronoun = 2;
	}

	int16 curDamage = CLIP<int16>(damage, 0, 50);

	for (uint objectId = 0; objectId < 10; ++objectId) {
		if (_npcBuf[charId]._inventory[objectId]._ref == 0x7FFF || !_npcBuf[charId]._inventory[objectId].isEquipped() || _items[_npcBuf[charId]._inventory[objectId]._ref]._defense == 0)
			continue;

		int16 remainingDamage = curDamage - _npcBuf[charId]._inventory[objectId]._curHitPoints;
		// not in the original: this int16 is used to test if the result is negative. Otherwise _curHitPoints (uint8) turns it into a "large" positive value.
		int16 newDurability = _npcBuf[charId]._inventory[objectId]._curHitPoints - curDamage;
		_npcBuf[charId]._inventory[objectId]._curHitPoints = newDurability;

		if (newDurability <= 0) {
			Common::String buffer2 = _items[_npcBuf[charId]._inventory[objectId]._ref]._name;
			removeObject(charId, objectId);

			if (destroyCounter == 0) {
				_messageToBePrinted += Common::String::format(", but %s ", kPossessive[pronoun]) + buffer2;
			} else {
				_messageToBePrinted += Common::String(", ") + buffer2;
			}

			++destroyCounter;
		}

		if (remainingDamage > 0)
			curDamage = remainingDamage;
		// The original doesn't contain this Else clause. But logically, if the remainingDamage is less than 0, it doesn't make sense to keep damaging equipment with the previous damage.
		// As it looks like an original bug, I just added the code to stop damaging equipped protections.
		else
			break;
	}

	if (destroyCounter == 0) {
		_messageToBePrinted += "!";
	} else if (destroyCounter > 1 || _messageToBePrinted.lastChar() == 's' || _messageToBePrinted.lastChar() == 'S') {
		_messageToBePrinted += " are destroyed!";
	} else {
		_messageToBePrinted += " is destroyed!";
	}
}

} // End of namespace Efh
