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
		_teamMonsterIdArray[0] = monsterTeamId;
	}

	for (int counter2 = 1; counter2 <= 3; ++counter2) {
		if (counter >= 5)
			break;

		for (uint monsterId = 0; monsterId < 64; ++monsterId) {
			if (_mapMonsters[monsterId]._fullPlaceId == 0xFF)
				continue;

			if (((_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) != 0x3F || isNpcATeamMember(_mapMonsters[monsterId]._npcId)) && (_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) > 0x3D)
				continue;

			if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
				continue;

			bool found = false;
			for (uint subId = 0; subId < 9; ++subId) {
				if (_mapMonsters[monsterId]._hitPoints[subId] > 0) {
					found = true;
					break;
				}
			}

			if (found) {
				if (computeMonsterGroupDistance(monsterId) <= counter2 && !isMonsterAlreadyFighting(monsterId, counter)) {
					_teamMonsterIdArray[counter] = monsterId;
					if (++counter >= 5)
						break;
				}
			}
		}
	}

	if (counter > 4)
		return;

	for (uint id = counter; id < 5; ++id)
		_teamMonsterIdArray[id] = -1;
}

void EfhEngine::initFight(int16 monsterId) {
	debugC(3, kDebugFight, "initFight %d", monsterId);
	createOpponentList(monsterId);
	resetTeamMonsterEffects();
}

bool EfhEngine::handleFight(int16 monsterId) {
	debug("handleFight %d", monsterId);

	_ongoingFightFl = true;

	initFight(monsterId);

	if (_teamMonsterIdArray[0] == -1) {
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

		if (_teamMonsterIdArray[0] == -1) {
			resetTeamMonsterIdArray();
			_ongoingFightFl = false;
			displayAnimFrames(0xFE, true);
			return true;
		}

		int16 varInt = getTeamMonsterAnimId();
		displayAnimFrames(varInt, true);
		for (int counter = 0; counter < _teamSize; ++counter) {
			_teamPctVisible[counter] = 100;
			_word32482[counter] = 65;
		}

		if (!sub1CB27()) {
			resetTeamMonsterIdArray();
			_ongoingFightFl = false;
			totalPartyKill();
			displayAnimFrames(0xFE, true);
			return false;
		}

		for (int counter = 0; counter < _teamSize; ++counter) {
			if (_teamLastAction[counter] == 0x52) // 'R'
				mainLoopCond = true;
		}

		computeInitiatives();
		displayBoxWithText("", 2, 1, false);

		for (uint counter = 0; counter < 8; ++counter) {
			int16 monsterGroupIdOrMonsterId = _initiatives[counter]._id;
			if (monsterGroupIdOrMonsterId == -1)
				continue;
			if (monsterGroupIdOrMonsterId > 999) { // Team Member
				monsterGroupIdOrMonsterId -= 1000;
				if (!isTeamMemberStatusNormal(monsterGroupIdOrMonsterId)) {
					handleFight_checkEndEffect(monsterGroupIdOrMonsterId);
				} else {
					switch (_teamLastAction[monsterGroupIdOrMonsterId]) {
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
				// handleFight - Loop on var86 - Start
				for (uint var86 = 0; var86 < 9; ++var86) {
					if (isMonsterActive(monsterGroupIdOrMonsterId, var86)) {
						int16 monsterWeaponItemId = _mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._weaponItemId;
						if (monsterWeaponItemId == 0xFF)
							monsterWeaponItemId = 0x3F;
						int16 teamMemberId = -1;
						int16 var54;
						if (_items[monsterWeaponItemId]._range < 3) {
							for (uint var84 = 0; var84 < 10; ++var84) {
								teamMemberId = getRandom(_teamSize) - 1;
								if (checkWeaponRange(_teamMonsterIdArray[monsterGroupIdOrMonsterId], monsterWeaponItemId) && isTeamMemberStatusNormal(teamMemberId) && getRandom(100) < _teamPctVisible[teamMemberId]) {
									break;
								}
								teamMemberId = -1;
							}
							var54 = teamMemberId + 1;
						} else {
							teamMemberId = 0;
							var54 = _teamSize;
						}
						if (teamMemberId != -1) {
							// handleFight - Loop on var7E - Start
							for (int16 var7E = teamMemberId; var7E < var54; ++var7E) {
								if (_teamCharId[var7E] == -1 || !isTeamMemberStatusNormal(var7E))
									continue;

								int16 var76 = getRandom(getEquipmentDefense(_teamCharId[var7E], false));
								varInt = _teamMonsterIdArray[monsterGroupIdOrMonsterId];
								int16 ennemyPronoun = kEncounters[_mapMonsters[varInt]._monsterRef]._nameArticle;
								int16 characterPronoun = _npcBuf[_teamCharId[var7E]].getPronoun();
								varInt = _items[monsterWeaponItemId].field_13;
								_word32482[var7E] += (varInt * 5);
								int16 var62 = 0;
								int16 hitPoints = 0;
								int16 originalDamage = 0;
								int16 damagePointsAbsorbed = 0;
								int16 var64 = _mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._npcId * _items[monsterWeaponItemId]._attacks;
								for (int var84 = 0; var84 < var64; ++var84) {
									// handleFight - Loop var84 on var64 (objectId) - Start
									if (getRandom(100) > _word32482[var7E])
										continue;

									++var62;

									if (hasAdequateDefenseNPC(_teamCharId[var7E], _items[monsterWeaponItemId]._attackType))
										continue;

									int16 var7C = getRandom(_items[monsterWeaponItemId]._damage);
									varInt = var7C - var76;

									if (varInt > 0) {
										damagePointsAbsorbed += var76;
										originalDamage += varInt;
									} else {
										damagePointsAbsorbed += var7C;
									}
									// handleFight - Loop var84 on var64 (objectId) - End
								}

								if (originalDamage < 0)
									originalDamage = 0;

								hitPoints = originalDamage + damagePointsAbsorbed;
								if (!checkSpecialItemsOnCurrentPlace(monsterWeaponItemId))
									var62 = 0;

								if (var62 > 0) {
									_npcBuf[_teamCharId[var7E]]._hitPoints -= originalDamage;
									if (var62 > 1)
										_attackBuffer = Common::String::format("%d times ", var62);
									else
										_attackBuffer = "";
								}

								int16 var68 = _items[monsterWeaponItemId]._attackType + 1;
								int16 var6A = getRandom(3);
								if (characterPronoun == 2)
									_characterNamePt1 = "The ";
								else
									_characterNamePt1 = "";

								if (ennemyPronoun == 2)
									_enemyNamePt1 = "The ";
								else
									_enemyNamePt1 = "";

								_enemyNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._name;
								_characterNamePt2 = _npcBuf[_teamCharId[var7E]]._name;
								_nameBuffer = _items[monsterWeaponItemId]._name;
								if (checkSpecialItemsOnCurrentPlace(monsterWeaponItemId)) {
									// handleFight - check damages - Start
									if (var62 == 0) {
										_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
									} else if (hitPoints <= 0) {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
									} else if (hitPoints == 1) {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											_messageToBePrinted += "!";
									} else {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str(), hitPoints);
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											_messageToBePrinted += "!";
									}
									// handleFight - check damages - End

									// handleFight - Add reaction text - start
									if (var62 != 0 && originalDamage > 0 && getRandom(100) <= 35 && _npcBuf[_teamCharId[var7E]]._hitPoints > 0) {
										if (_npcBuf[_teamCharId[var7E]]._hitPoints - 5 <= originalDamage) {
											addReactionText(kEfhReactionReels);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 8) {
											addReactionText(kEfhReactionCriesOut);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 4) {
											addReactionText(kEfhReactionFalters);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 2) {
											addReactionText(kEfhReactionWinces);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 3) {
											// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it
											addReactionText(kEfhReactionScreams);
										} else if (_npcBuf[_teamCharId[var7E]]._maxHP / 8 >= originalDamage) {
											addReactionText(kEfhReactionChortles);
										} else if (originalDamage == 0 && getRandom(100) < 35) {
											addReactionText(kEfhReactionLaughs);
										}
									}
									// handleFight - Add reaction text - end

									// handleFight - Check armor - start
									if (var76 != 0 && var62 != 0 && _npcBuf[_teamCharId[var7E]]._hitPoints > 0) {
										if (damagePointsAbsorbed <= 1)
											_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
										else
											_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);

										varInt = (originalDamage + damagePointsAbsorbed) / 10;
										sub1D8C2(_teamCharId[var7E], varInt);
									}
									// handleFight - Check armor - end

									// handleFight - Check effect - start
									switch (_items[monsterWeaponItemId]._specialEffect) {
									case 1:
										if (getRandom(100) < 20) {
											_teamCharStatus[var7E]._status = 1;
											_teamCharStatus[var7E]._duration = getRandom(10);
											_messageToBePrinted += Common::String::format("  %s%s falls asleep!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
										}
										break;
									case 2:
										if (getRandom(100) < 20) {
											_teamCharStatus[var7E]._status = 2;
											_teamCharStatus[var7E]._duration = getRandom(10);
											_messageToBePrinted += Common::String::format("  %s%s is frozen!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
										}
										break;
									case 5:
									case 6:
										if (getRandom(100) < 20) {
											_messageToBePrinted += Common::String::format("  %s%s's life energy is gone!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
											_npcBuf[_teamCharId[var7E]]._hitPoints = 0;
										}
										break;
									default:
										break;
									}
									// handleFight - Check effect - end
								} else {
									_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
								}
								genericGenerateSound(_items[monsterWeaponItemId]._attackType, var62);
								displayBoxWithText(_messageToBePrinted, 1, 2, true);
							}
							// handleFight - Loop on var7E - End
						}
					} else if (_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._hitPoints[var86] > 0 && _teamMonsterEffects[monsterGroupIdOrMonsterId]._effect[var86]) {
						--_teamMonsterEffects[monsterGroupIdOrMonsterId]._duration[var86];
						if (_teamMonsterEffects[monsterGroupIdOrMonsterId]._duration[var86] <= 0) {
							_enemyNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._name;
							int16 var70 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._nameArticle;
							if (var70 == 2)
								_enemyNamePt1 = "The ";
							else
								_enemyNamePt1 = "";

							switch (_teamMonsterEffects[monsterGroupIdOrMonsterId]._effect[var86]) {
							case 1:
								_messageToBePrinted = Common::String::format("%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							case 2:
								_messageToBePrinted = Common::String::format("%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							default:
								_messageToBePrinted = Common::String::format("%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							}
							_teamMonsterEffects[monsterGroupIdOrMonsterId]._effect[var86] = 0;
							displayBoxWithText(_messageToBePrinted, 1, 2, true);
						}
					}
				}
				// handleFight - Loop on var86 - End
			}
		}

		handleMapMonsterMoves();
		sub1BE9A(monsterId);
	}

	resetTeamMonsterIdArray();
	_ongoingFightFl = false;
	displayAnimFrames(0xFE, true);
	return true;
}

void EfhEngine::handleFight_checkEndEffect(int16 charId) {
	debug("handleFight_checkEndEffect %d", charId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	if (_teamCharStatus[charId]._status == 0)
		return;
	if (--_teamCharStatus[charId]._duration != 0)
		return;

	// At this point : The status is different to 0 (normal) and the effect duration is finally 0 (end of effect)
	_enemyNamePt2 = _npcBuf[_teamCharId[charId]]._name;
	if (_npcBuf[_teamCharId[charId]].getPronoun() == 2) {
		_enemyNamePt1 = "The ";
	} else {
		_enemyNamePt1 = "";
	}

	// End of effect message depends on the type of effect
	switch (_teamCharStatus[charId]._status) {
	case 1:
		_messageToBePrinted = Common::String::format("%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	case 2:
		_messageToBePrinted = Common::String::format("%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	default:
		_messageToBePrinted = Common::String::format("%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	}

	// The character status is back to normal
	_teamCharStatus[charId]._status = 0;

	// Finally, display the message
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_A(int16 teamCharId) {
	debug("handleFight_lastAction_A %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.

	int16 teamCharItemId = getEquippedExclusiveType(_teamCharId[teamCharId], 9, true);
	if (teamCharItemId == 0x7FFF)
		teamCharItemId = 0x3F;
	int16 monsterGroupNumber = _teamNextAttack[teamCharId];
	if (monsterGroupNumber == 0x64)
		monsterGroupNumber = 0;

	if (monsterGroupNumber == -1)
		return;
	int16 var58;
	if (_items[teamCharItemId]._range == 4)
		var58 = 5;
	else
		var58 = monsterGroupNumber + 1;

	int16 var54;
	int16 teamMemberId;
	if (_items[teamCharItemId]._range < 3) {
		teamMemberId = getWeakestMobster(monsterGroupNumber);
		var54 = teamMemberId + 1;
	} else {
		teamMemberId = 0;
		var54 = 9;
	}

	if (teamMemberId != -1) {
		bool var6E = true;
		for (int16 groupId = monsterGroupNumber; groupId < var58; ++groupId) {
			if (_teamMonsterIdArray[groupId] == -1)
				continue;

			for (int16 mobsterCounter = teamMemberId; mobsterCounter < var54; ++mobsterCounter) {
				if (isMonsterActive(groupId, mobsterCounter) && var6E) {
					bool noticedFl;
					if (!checkMonsterMovementType(groupId, true)) {
						setMapMonsterAggressivenessAndMovementType(groupId, 9, true);
						_unk2C8AA += 500;
						noticedFl = true;
					} else
						noticedFl = false;

					int16 var76 = getRandom(_mapMonsters[_teamMonsterIdArray[groupId]]._maxDamageAbsorption);
					int16 ennemyPronoun = _npcBuf[_teamCharId[teamCharId]].getPronoun();
					int16 monsterId = _teamMonsterIdArray[groupId];
					int16 characterPronoun = kEncounters[_mapMonsters[monsterId]._monsterRef]._nameArticle;
					int16 charScore = getCharacterScore(_teamCharId[teamCharId], teamCharItemId);
					int16 hitPointsBefore = _mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter];
					int16 hitCount = 0;
					int16 originalDamage = 0;
					int16 damagePointsAbsorbed = 0;
					int16 attackSpeed = _items[teamCharItemId]._attacks * _npcBuf[_teamCharId[teamCharId]]._speed;

					// Action A - Loop var84 - Start
					for (int var84 = 0; var84 < attackSpeed; ++var84) {
						if (getRandom(100) < charScore) {
							++hitCount;
							if (!hasAdequateDefense(_teamMonsterIdArray[groupId], _items[teamCharItemId]._attackType)) {
								int16 var7C = getRandom(_items[teamCharItemId]._damage);
								int16 varInt = var7C - var76;
								if (varInt > 0) {
									originalDamage += varInt;
									damagePointsAbsorbed += var76;
								} else {
									damagePointsAbsorbed += var7C;
								}
							}
						}
					}
					// Action A - Loop var84 - End

					if (originalDamage < 0)
						originalDamage = 0;

					int16 hitPoints = originalDamage + damagePointsAbsorbed;

					if (!checkSpecialItemsOnCurrentPlace(teamCharItemId))
						hitCount = 0;

					if (hitCount > 0) {
						_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] -= originalDamage;
						if (hitCount > 1) {
							_attackBuffer = Common::String::format("%d times ", hitCount);
						} else {
							_attackBuffer = "";
						}
					}
					int16 verbId = (3 * _items[teamCharItemId]._attackType + 1) + getRandom(3) - 1;
					if (characterPronoun == 2) {
						_characterNamePt1 = "The ";
					} else {
						_characterNamePt1 = "";
					}

					if (ennemyPronoun == 2) {
						_enemyNamePt1 = "The ";
					} else {
						_enemyNamePt1 = "";
					}

					_characterNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[groupId]]._monsterRef]._name;
					_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
					_nameBuffer = _items[teamCharItemId]._name;
					if (checkSpecialItemsOnCurrentPlace(teamCharItemId)) {
						// Action A - Check damages - Start
						if (hitCount == 0) {
							_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
						} else if (hitPoints <= 0) {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
						} else if (hitPoints == 1) {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								_messageToBePrinted += "!";
							}
						} else {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[verbId], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str(), hitPoints);
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								_messageToBePrinted += "!";
							}
						}
						// Action A - Check damages - End

						// Action A - Add reaction text - Start
						if (hitCount != 0 && originalDamage > 0 && getRandom(100) <= 35 && _mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] > 0) {
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] - 5 <= originalDamage) {
								addReactionText(kEfhReactionReels);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] < hitPointsBefore / 8) {
								addReactionText(kEfhReactionCriesOut);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] < hitPointsBefore / 4) {
								addReactionText(kEfhReactionFalters);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] < hitPointsBefore / 2) {
								addReactionText(kEfhReactionWinces);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] < hitPointsBefore / 3) {
								// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it
								addReactionText(kEfhReactionScreams);
							} else if (hitPointsBefore / 8 >= originalDamage) {
								addReactionText(kEfhReactionChortles);
							} else if (originalDamage == 0 && getRandom(100) < 35) {
								addReactionText(kEfhReactionLaughs);
							}
						}
						// Action A - Add reaction text - End

						// Action A - Add armor absorb text - Start
						if (var76 && hitCount && _mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] > 0) {
							if (damagePointsAbsorbed <= 1)
								_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
							else
								_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);
						}
						// Action A - Add armor absorb text - End

						if (noticedFl)
							_messageToBePrinted += Common::String("  Your actions do not go un-noticed...");

						// Action A - Check item durability - Start
						int16 npcId = _teamCharId[teamCharId];

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
						if (_items[teamCharItemId]._specialEffect == 1 && _mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] > 0) {
							if (getRandom(100) < 35) {
								_teamMonsterEffects[groupId]._effect[mobsterCounter] = 1;
								_teamMonsterEffects[groupId]._duration[mobsterCounter] = getRandom(10);
								_messageToBePrinted += Common::String::format("  %s%s falls asleep!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
							}
						} else if (_items[teamCharItemId]._specialEffect == 2 && _mapMonsters[_teamMonsterIdArray[groupId]]._hitPoints[mobsterCounter] > 0) {
							_teamMonsterEffects[groupId]._effect[mobsterCounter] = 2;
							_teamMonsterEffects[groupId]._duration[mobsterCounter] = getRandom(10);
							_messageToBePrinted += Common::String::format("  %s%s is frozen!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
						}
						// Action A - Check effect - End
					} else {
						_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[ennemyPronoun], _nameBuffer.c_str());
					}

					genericGenerateSound(_items[teamCharItemId]._attackType, hitCount);
					displayBoxWithText(_messageToBePrinted, 1, 2, true);
				}
			}
		}
	}
}

void EfhEngine::handleFight_lastAction_D(int16 teamCharId) {
	debug("handleFight_lastAction_D %d", teamCharId);

	_word32482[teamCharId] -= 40;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;

	uint8 pronoun = _npcBuf[_teamCharId[teamCharId]].getPronoun();

	if (pronoun == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	_messageToBePrinted = Common::String::format("%s%s prepares to defend %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[pronoun]);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_H(int16 teamCharId) {
	debugC(3, kDebugFight, "handleFight_lastAction_H %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.

	_teamPctVisible[teamCharId] -= 50;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	int16 pronoun = _npcBuf[_teamCharId[teamCharId]].getPronoun();

	if (pronoun == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	_messageToBePrinted = Common::String::format("%s%s attempts to hide %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[pronoun]);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);
}

bool EfhEngine::handleFight_lastAction_U(int16 teamCharId) {
	debug("handleFight_lastAction_U %d", teamCharId);

	// Fight - Action 'U' - Use Item
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	int16 itemId = _npcBuf[_teamCharId[teamCharId]]._inventory[_word31780[teamCharId]]._ref;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	_nameBuffer = _items[itemId]._name;
	int16 pronoun = _npcBuf[_teamCharId[teamCharId]].getPronoun();
	if (pronoun == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	_messageToBePrinted = Common::String::format("%s%s uses %s %s!  ", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[pronoun], _nameBuffer.c_str());
	bool retVal = useObject(_teamCharId[teamCharId], _word31780[teamCharId], _teamNextAttack[teamCharId], teamCharId, 0, 3);
	displayBoxWithText(_messageToBePrinted, 1, 2, true);

	return retVal;
}

bool EfhEngine::isTPK() {
	debugC(6, kDebugFight, "isTPK");

	int16 zeroedChar = 0;
	for (int counter = 0; counter < _teamSize; ++counter) {
		if (_npcBuf[_teamCharId[counter]]._hitPoints <= 0)
			++zeroedChar;
	}

	return zeroedChar == _teamSize;
}

bool EfhEngine::isMonsterAlreadyFighting(int16 monsterId, int16 teamMonsterId) {
	debugC(6, kDebugFight, "isMonsterAlreadyFighting %d %d", monsterId, teamMonsterId);

	for (int counter = 0; counter < teamMonsterId; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId)
			return true;
	}
	return false;
}

void EfhEngine::resetTeamMonsterEffects() {
	debugC(6, kDebugFight, "resetTeamMonsterEffects");
	for (uint ctrMonsterId = 0; ctrMonsterId < 5; ++ctrMonsterId) {
		for (uint ctrEffectId = 0; ctrEffectId < 9; ++ctrEffectId) {
			_teamMonsterEffects[ctrMonsterId]._effect[ctrEffectId] = 0;
			_teamMonsterEffects[ctrMonsterId]._duration[ctrEffectId] = 0;
		}
	}
}

void EfhEngine::resetTeamMonsterIdArray() {
	debugC(6, kDebugFight, "resetTeamMonsterIdArray");

	for (int i = 0; i < 5; ++i) {
		_teamMonsterIdArray[i] = -1;
	}
}

bool EfhEngine::isTeamMemberStatusNormal(int16 teamMemberId) {
	debugC(6, kDebugFight, "isTeamMemberStatusNormal %d", teamMemberId);

	if (_npcBuf[_teamCharId[teamMemberId]]._hitPoints > 0 && _teamCharStatus[teamMemberId]._status == 0)
		return true;

	return false;
}

void EfhEngine::getDeathTypeDescription(int16 attackerId, int16 victimId) {
	debug("getDeathTypeDescription %d %d", attackerId, victimId);

	uint8 pronoun;

	if (attackerId > 999) {
		int16 charId = _teamCharId[attackerId - 1000];
		pronoun = _npcBuf[charId].getPronoun();
	} else {
		int16 charId = _teamMonsterIdArray[attackerId];
		pronoun = _mapMonsters[charId].getPronoun();
	}

	if (pronoun > 2)
		pronoun = 2;

	int16 deathType;
	if (getRandom(100) < 20) {
		deathType = 0;
	} else {
		if (victimId >= 1000) {
			int16 charId = _teamCharId[victimId - 1000];
			if (charId == -1)
				deathType = 0;
			else {
				int16 exclusiveItemId = getEquippedExclusiveType(charId, 9, true);
				if (exclusiveItemId == 0x7FFF)
					deathType = 0;
				else
					deathType = _items[exclusiveItemId]._attackType + 1;
			}
		} else if (_teamMonsterIdArray[victimId] == -1)
			deathType = 0;
		else {
			int16 itemId = _mapMonsters[_teamMonsterIdArray[victimId]]._weaponItemId;
			deathType = _items[itemId]._attackType;
		}
	}

	int16 rndDescrForDeathType = getRandom((3)) - 1;
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

int16 EfhEngine::sub1C956(int16 charId, int16 unkFied18Val, bool arg4) {
	debug("sub1C956 %d %d %d", charId, unkFied18Val, arg4);

	int16 varE = -1;

	int16 curItemId = getEquippedExclusiveType(charId, unkFied18Val, true);
	int16 range = 0;
	if (curItemId != 0x7FFF)
		range = _items[curItemId]._range;

	switch (range) {
	case 3:
	case 2:
		++range;
		// no break on purpose
	case 1:
		++range;
		// no break on purpose
	case 0:
		++range;
		break;
	case 4:
		return 100;
	default:
		return varE;
	}

	do {
		for (uint counter = 0; counter < 2; ++counter) {
			drawCombatScreen(charId, true, false);
			if (_teamMonsterIdArray[1] != -1)
				displayBoxWithText("Select Monster Group:", 3, 0, false);

			if (counter == 0)
				displayFctFullScreen();
		}

		if (_teamMonsterIdArray[1] == -1)
			varE = 0;
		else
			varE = selectMonsterGroup();

		if (!arg4) {
			if (varE == 27) // Esc
				varE = 0;
		} else if (varE != 27) {
			int16 monsterGroupDistance = computeMonsterGroupDistance(_teamMonsterIdArray[varE]);
			if (monsterGroupDistance > range) {
				varE = 27;
			}
		}
	} while (varE == -1);

	if (varE == 27)
		varE = -1;

	return varE;
}

bool EfhEngine::sub1CB27() {
	debug("sub1CB27");

	bool var4 = false;
	for (int counter1 = 0; counter1 < _teamSize; ++counter1) {
		_teamLastAction[counter1] = 0;
		if (!isTeamMemberStatusNormal(counter1))
			continue;

		var4 = true;
		do {
			drawCombatScreen(_teamCharId[counter1], false, true);
			Common::KeyCode var1 = handleAndMapInput(true);
			switch (var1) {
			case Common::KEYCODE_a: // Attack
				_teamLastAction[counter1] = 'A';
				_teamNextAttack[counter1] = sub1C956(_teamCharId[counter1], 9, true);
				if (_teamNextAttack[counter1] == -1)
					_teamLastAction[counter1] = 0;
				break;
			case Common::KEYCODE_d: // Defend
				_teamLastAction[counter1] = 'D';
				break;
			case Common::KEYCODE_h: // Hide
				_teamLastAction[counter1] = 'H';
				break;
			case Common::KEYCODE_r: // Run
				for (int counter2 = 0; counter2 < _teamSize; ++counter2) {
					_teamLastAction[counter2] = 'R';
				}
				return true;
			case Common::KEYCODE_s: { // Status
				int16 var8 = handleStatusMenu(2, _teamCharId[counter1]);
				sub1CAB6(_teamCharId[counter1]);
				if (var8 > 999) {
					if (var8 == 0x7D00)
						_teamLastAction[counter1] = 'S';
				} else {
					_teamLastAction[counter1] = 'U';
					_word31780[counter1] = var8;
					int16 var6 = _npcBuf[_teamCharId[counter1]]._inventory[var8]._ref;
					switch (var6 - 1) {
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
						_teamNextAttack[counter1] = sub1C956(_teamCharId[counter1], 9, false);
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
						_teamNextAttack[counter1] = selectOtherCharFromTeam();
						break;

					case 16:
					case 17:
					case 26:
						_teamNextAttack[counter1] = 0xC8;
						break;

					case 19:
					case 20:
					case 21:
					case 22:
					case 23:
					default:
						break;
					}
				}

			} break;
			case Common::KEYCODE_t: // Terrain
				redrawScreenForced();
				getInputBlocking();
				drawCombatScreen(_teamCharId[counter1], false, true);
				break;
			default:
				break;
			}
		} while (_teamLastAction[counter1] == 0);
	}

	return var4;
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
			sub1C4CA(whiteFl);
			displayCombatMenu(charId);
			displayLowStatusScreen(false);
		}

		if (counter == 0 && drawFl)
			displayFctFullScreen();
	}
}

void EfhEngine::getXPAndSearchCorpse(int16 charId, Common::String namePt1, Common::String namePt2, int16 monsterId) {
	debugC(3, kDebugFight, "getXPAndSearchCorpse %d %s%s %d", charId, namePt1.c_str(), namePt2.c_str(), monsterId);

	int16 oldXpLevel = getXPLevel(_npcBuf[charId]._xp);
	_npcBuf[charId]._xp += kEncounters[_mapMonsters[monsterId]._monsterRef]._xpGiven;

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

	_messageToBePrinted += Common::String::format("  %s%s gains %d experience", namePt1.c_str(), namePt2.c_str(), kEncounters[_mapMonsters[monsterId]._monsterRef]._xpGiven);
	if (!characterSearchesMonsterCorpse(charId, monsterId))
		_messageToBePrinted += "!";
}

bool EfhEngine::characterSearchesMonsterCorpse(int16 charId, int16 monsterId) {
	debug("characterSearchesMonsterCorpse %d %d", charId, monsterId);

	int16 rndVal = getRandom(100);
	if (kEncounters[_mapMonsters[monsterId]._monsterRef]._dropOccurrencePct < rndVal)
		return false;

	rndVal = getRandom(5) - 1;
	int16 itemId = kEncounters[_mapMonsters[monsterId]._monsterRef]._dropItemId[rndVal];
	if (itemId == -1)
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

void EfhEngine::sub1C4CA(bool whiteFl) {
	debugC(5, kDebugFight, "sub1C4CA %s", whiteFl ? "True" : "False");

	int16 textPosY = 20;
	for (uint counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == -1)
			continue;

		int16 monsterDistance = computeMonsterGroupDistance(_teamMonsterIdArray[counter]);
		int16 mobsterCount = countMonsterGroupMembers(counter);
		if (whiteFl)
			setTextColorWhite();
		else
			setTextColorGrey();

		setTextPos(129, textPosY);
		Common::String buffer = Common::String::format("%c)", 'A' + counter);
		displayStringAtTextPos(buffer);
		setTextColorRed();
		int16 var1 = _mapMonsters[_teamMonsterIdArray[counter]]._possessivePronounSHL6 & 0x3F;
		if (var1 <= 0x3D) {
			buffer = Common::String::format("%d %s", mobsterCount, kEncounters[_mapMonsters[_teamMonsterIdArray[counter]]._monsterRef]._name);
			displayStringAtTextPos(buffer);
			if (mobsterCount > 1)
				displayStringAtTextPos("s");
		} else if (var1 == 0x3E) {
			displayStringAtTextPos("(NOT DEFINED)");
		} else if (var1 == 0x3F) {
			Common::String stringToDisplay = _npcBuf[_mapMonsters[_teamMonsterIdArray[counter]]._npcId]._name;
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
	int16 monsterId = _teamMonsterIdArray[groupNumber];

	if (monsterId == -1)
		return -1;

	for (uint counter = 0; counter < 9; ++counter) {
		if (isMonsterActive(groupNumber, counter)) {
			weakestMobsterId = counter;
			break;
		}
	}

	for (int16 counter = weakestMobsterId + 1; counter < 9; ++counter) {
		if (!isMonsterActive(groupNumber, counter))
			continue;

		if (_mapMonsters[monsterId]._hitPoints[weakestMobsterId] > _mapMonsters[monsterId]._hitPoints[counter])
			weakestMobsterId = counter;
	}

	// Useless check, as the
	if (_mapMonsters[monsterId]._hitPoints[weakestMobsterId] <= 0)
		return -1;

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

	extraScore += _items[itemId].field_13;

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
	case 2:
		if ((itemId >= 0x61 && itemId <= 0x63) || (itemId >= 0x74 && itemId <= 0x76) || (itemId >= 0x86 && itemId <= 0x89) || itemId == 0x5B || itemId == 0x5E || itemId == 0x66 || itemId == 0x68 || itemId == 0x8C)
			retVal = false;
	default:
		break;
	}

	return retVal;
}

bool EfhEngine::hasAdequateDefense(int16 monsterId, uint8 attackType) {
	debugC(3, kDebugFight, "hasAdequateDefense %d %d", monsterId, attackType);

	int16 itemId = _mapMonsters[monsterId]._weaponItemId;

	if (_items[itemId]._specialEffect != 0)
		return false;

	return _items[itemId]._field17_attackTypeDefense == attackType;
}

bool EfhEngine::hasAdequateDefenseNPC(int16 charId, uint8 attackType) {
	debugC(3, kDebugFight, "hasAdequateDefenseNPC %d %d", charId, attackType);

	int16 itemId = _npcBuf[charId]._defaultDefenseItemId;

	if (_items[itemId]._specialEffect == 0 && _items[itemId]._field17_attackTypeDefense == attackType)
		return true;

	for (uint counter = 0; counter < 10; ++counter) {
		if (_npcBuf[charId]._inventory[counter]._ref == 0x7FFF || !_npcBuf[charId]._inventory[counter].isEquipped())
			continue;

		itemId = _npcBuf[charId]._inventory[counter]._ref;
		if (_items[itemId]._specialEffect == 0 && _items[itemId]._field17_attackTypeDefense == attackType)
			return true;
	}
	return false;
}

// The parameter isn't used in the original
void EfhEngine::sub1BE9A(int16 monsterId) {
	debug("sub1BE9A %d", monsterId);

	// sub1BE9A - 1rst loop counter1_monsterId - Start
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		if (countMonsterGroupMembers(counter1))
			continue;

		for (uint counter2 = 0; counter2 < 9; ++counter2) {
			_mapMonsters[_teamMonsterIdArray[counter1]]._hitPoints[counter2] = 0;
			_teamMonsterEffects[counter1]._effect[counter2] = 0;
			_teamMonsterEffects[counter1]._duration[counter2] = 0;
		}

		_teamMonsterIdArray[counter1] = -1;

		// CHECKME: counter1 is not incrementing, which is very, very suspicious as we are copying over and over to the same destination
		// if the purpose is compact the array, it should be handle differently
		for (uint counter2 = counter1 + 1; counter2 < 5; ++counter2) {
			for (uint var8 = 0; var8 < 9; ++var8) {
				_teamMonsterEffects[counter1]._effect[var8] = _teamMonsterEffects[counter2]._effect[var8];
				_teamMonsterEffects[counter1]._duration[var8] = _teamMonsterEffects[counter2]._duration[var8];
			}
			_teamMonsterIdArray[counter1] = _teamMonsterIdArray[counter2];
		}
	}
	// sub1BE9A - 1rst loop counter1_monsterId - End

	int16 teamMonsterId = -1;
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		if (_teamMonsterIdArray[counter1] == -1) {
			teamMonsterId = counter1;
			break;
		}
	}

	if (teamMonsterId != -1) {
		// sub1BE9A - loop var2 - Start
		for (int var2 = 1; var2 < 3; ++var2) {
			if (teamMonsterId >= 5)
				break;

			for (uint ctrMapMonsterId = 0; ctrMapMonsterId < 64; ++ctrMapMonsterId) {
				if (_mapMonsters[ctrMapMonsterId]._fullPlaceId == 0xFF)
					continue;

				if (((_mapMonsters[ctrMapMonsterId]._possessivePronounSHL6 & 0x3F) == 0x3F && !isNpcATeamMember(_mapMonsters[ctrMapMonsterId]._npcId)) || (_mapMonsters[ctrMapMonsterId]._possessivePronounSHL6 & 0x3F) <= 0x3D) {
					if (checkIfMonsterOnSameLargeMapPlace(ctrMapMonsterId)) {
						bool monsterActiveFound = false;
						for (uint ctrSubId = 0; ctrSubId < 9; ++ctrSubId) {
							if (_mapMonsters[ctrMapMonsterId]._hitPoints[ctrSubId] > 0) {
								monsterActiveFound = true;
								break;
							}
						}

						if (!monsterActiveFound)
							continue;

						if (computeMonsterGroupDistance(ctrMapMonsterId) > var2)
							continue;

						if (isMonsterAlreadyFighting(ctrMapMonsterId, teamMonsterId))
							continue;

						_teamMonsterIdArray[teamMonsterId] = ctrMapMonsterId;

						// The original at this point was doing a loop on counter1, which is not a good idea as
						// it was resetting the counter1 to 9 whatever its value before the loop.
						// I therefore decided to use another counter as it looks like an original misbehavior/bug.
						for (uint ctrEffectId = 0; ctrEffectId < 9; ++ctrEffectId) {
							_teamMonsterEffects[teamMonsterId]._effect[ctrEffectId] = 0;
						}

						if (++teamMonsterId >= 5)
							break;
					}
				}
			}
		}
		// sub1BE9A - loop var2 - End
	}

	if (teamMonsterId == -1 || teamMonsterId > 4)
		return;

	// sub1BE9A - last loop counter1_monsterId - Start
	for (int16 ctrTeamMonsterId = teamMonsterId; ctrTeamMonsterId < 5; ++ctrTeamMonsterId) {
		_teamMonsterIdArray[ctrTeamMonsterId] = -1;
		for (uint ctrEffectId = 0; ctrEffectId < 9; ++ctrEffectId) {
			_teamMonsterEffects[ctrTeamMonsterId]._effect[ctrEffectId] = (int16)0x8000;
		}
	}
	// sub1BE9A - last loop counter1_monsterId - End
}

int16 EfhEngine::getTeamMonsterAnimId() {
	debugC(6, kDebugFight, "getTeamMonsterAnimId");

	int16 retVal = 0xFF;
	for (uint counter = 0; counter < 5; ++counter) {
		int16 monsterId = _teamMonsterIdArray[counter];
		if (monsterId == -1)
			continue;

		if (!checkMonsterMovementType(monsterId, false))
			continue;

		retVal = kEncounters[_mapMonsters[monsterId]._monsterRef]._animId;
		break;
	}

	if (retVal == 0xFF)
		retVal = kEncounters[_mapMonsters[_teamMonsterIdArray[0]]._monsterRef]._animId;

	return retVal;
}

} // End of namespace Efh
