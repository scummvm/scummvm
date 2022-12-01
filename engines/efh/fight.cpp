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

bool EfhEngine::handleFight(int16 monsterId) {
	debug("handleFight %d", monsterId);

	int16 var8C = 0;
	_ongoingFightFl = true;

	sub1BE89(monsterId);

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

		sub1CDFA();
		sub1C219(nullptr, 2, 1, false);

		for (uint counter = 0; counter < 8; ++counter) {
			int16 monsterGroupIdOrMonsterId = _stru3244C[counter]._field0;
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
						handleFight_lastAction_U(monsterGroupIdOrMonsterId);
						break;
					default:
						break;
					}
				}
			} else if (unkFct_checkMonsterField8(monsterGroupIdOrMonsterId, true)) {
				// handleFight - Loop on var86 - Start
				for (uint var86 = 0; var86 < 9; ++var86) {
					if (isMonsterActive(monsterGroupIdOrMonsterId, var86)) {
						int16 unk_monsterField5_itemId = _mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._itemId_Weapon;
						if (unk_monsterField5_itemId == 0xFF)
							unk_monsterField5_itemId = 0x3F;
						int16 teamMemberId = -1;
						int16 var54;
						if (_items[unk_monsterField5_itemId]._range < 3) {
							for (uint var84 = 0; var84 < 10; ++var84) {
								teamMemberId = getRandom(_teamSize) - 1;
								if (checkWeaponRange(_teamMonsterIdArray[monsterGroupIdOrMonsterId], unk_monsterField5_itemId) && isTeamMemberStatusNormal(teamMemberId) && getRandom(100) < _teamPctVisible[teamMemberId]) {
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
								int16 var70 = kEncounters[_mapMonsters[varInt]._monsterRef]._nameArticle;
								int16 var5E = _npcBuf[_teamCharId[var7E]]._possessivePronounSHL6 >> 6;
								varInt = _items[unk_monsterField5_itemId].field_13;
								_word32482[var7E] += (varInt * 5);
								int16 var62 = 0;
								int16 hitPoints = 0;
								int16 originalDamage = 0;
								int16 damagePointsAbsorbed = 0;
								int16 var64 = _mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._field_1 * _items[unk_monsterField5_itemId]._attacks;
								for (int var84 = 0; var84 < var64; ++var84) {
									// handleFight - Loop var84 on var64 (objectId) - Start
									if (getRandom(100) > _word32482[var7E])
										continue;

									++var62;

									if (hasAdequateDefense_2(_teamCharId[var7E], _items[unk_monsterField5_itemId]._attackType))
										continue;

									int16 var7C = getRandom(_items[unk_monsterField5_itemId]._damage);
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
								if (!checkSpecialItemsOnCurrentPlace(unk_monsterField5_itemId))
									var62 = 0;

								if (var62 > 0) {
									_npcBuf[_teamCharId[var7E]]._hitPoints -= originalDamage;
									if (var62 > 1)
										snprintf(_attackBuffer, 20, "%d times ", var62);
									else
										*_attackBuffer = 0;
								}

								int16 var68 = _items[unk_monsterField5_itemId]._attackType + 1;
								int16 var6A = getRandom(3);
								if (var5E == 2)
									snprintf(_characterNamePt1, 5, "The ");
								else
									*_characterNamePt1 = 0;

								if (var7E == 2)
									_enemyNamePt1 = "The ";
								else
									_enemyNamePt1 = "";

								_enemyNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._name;
								_characterNamePt2 = _npcBuf[_teamCharId[var7E]]._name;
								_nameBuffer = _items[unk_monsterField5_itemId]._name;
								if (checkSpecialItemsOnCurrentPlace(unk_monsterField5_itemId)) {
									// handleFight - check damages - Start
									if (var62 == 0) {
										snprintf((char *)_messageToBePrinted, 400, "%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1, _characterNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
									} else if (hitPoints <= 0) {
										snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str());
									} else if (hitPoints == 1) {
										snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str());
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											strncat((char *)_messageToBePrinted, "!", 2);
									} else {
										snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str(), hitPoints);
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											strncat((char *)_messageToBePrinted, "!", 2);
									}
									// handleFight - check damages - End

									// handleFight - Add reaction text - start
									if (var62 != 0 && originalDamage > 0 && getRandom(100) <= 35 && _npcBuf[_teamCharId[var7E]]._hitPoints > 0) {
										if (_npcBuf[_teamCharId[var7E]]._hitPoints - 5 <= originalDamage) {
											addReactionText(0);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 8) {
											addReactionText(1);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 4) {
											addReactionText(2);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 2) {
											addReactionText(3);
										} else if (_npcBuf[_teamCharId[var7E]]._hitPoints < _npcBuf[_teamCharId[var7E]]._maxHP / 3) {
											// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it
											addReactionText(4);
										} else if (_npcBuf[_teamCharId[var7E]]._maxHP / 8 >= originalDamage) {
											addReactionText(5);
										} else if (originalDamage == 0 && getRandom(100) < 35) {
											addReactionText(6);
										}
									}
									// handleFight - Add reaction text - end

									// handleFight - Check armor - start
									if (var76 != 0 && var62 != 0 && _npcBuf[_teamCharId[var7E]]._hitPoints > 0) {
										char buffer[80];
										memset(buffer, 0, 80);
										if (damagePointsAbsorbed <= 1)
											snprintf(buffer, 80, "  %s%s's armor absorbs 1 point!", _characterNamePt1, _characterNamePt2.c_str());
										else
											snprintf(buffer, 80, "  %s%s's armor absorbs %d points!", _characterNamePt1, _characterNamePt2.c_str(), damagePointsAbsorbed);

										strncat((char *)_messageToBePrinted, buffer, 80);
										varInt = (originalDamage + damagePointsAbsorbed) / 10;
										sub1D8C2(_teamCharId[var7E], varInt);
									}
									// handleFight - Check armor - end

									// handleFight - Check effect - start
									char buffer[80];
									memset(buffer, 0, 80);
									switch (_items[unk_monsterField5_itemId].field_16) {
									case 1:
										if (getRandom(100) < 20) {
											_teamCharStatus[var7E]._status = 1;
											_teamCharStatus[var7E]._duration = getRandom(10);
											snprintf(buffer, 80, "  %s%s falls asleep!", _characterNamePt1, _characterNamePt2.c_str());
											strncat((char *)_messageToBePrinted, buffer, 80);
										}
										break;
									case 2:
										if (getRandom(100) < 20) {
											_teamCharStatus[var7E]._status = 2;
											_teamCharStatus[var7E]._duration = getRandom(10);
											snprintf(buffer, 80, "  %s%s is frozen!", _characterNamePt1, _characterNamePt2.c_str());
											strncat((char *)_messageToBePrinted, buffer, 80);
										}
										break;
									case 5:
									case 6:
										if (getRandom(100) < 20) {
											snprintf(buffer, 80, "  %s%s's life energy is gone!", _characterNamePt1, _characterNamePt2.c_str());
											strncat((char *)_messageToBePrinted, buffer, 80);
											_npcBuf[_teamCharId[var7E]]._hitPoints = 0;
										}
										break;
									default:
										break;
									}
									// handleFight - Check effect - end
								} else {
									snprintf((char *)_messageToBePrinted, 400, "%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
								}
								genericGenerateSound(_items[unk_monsterField5_itemId]._attackType, var62);
								sub1C219(_messageToBePrinted, 1, 2, true);
							}
							// handleFight - Loop on var7E - End
						}
					} else if (_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._pictureRef[var86] > 0 && _stru32686[monsterGroupIdOrMonsterId]._field0[var86]) {
						--_stru32686[monsterGroupIdOrMonsterId]._field2[var86];
						if (_stru32686[monsterGroupIdOrMonsterId]._field2[var86] <= 0) {
							_enemyNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._name;
							int16 var70 = kEncounters[_mapMonsters[_teamMonsterIdArray[monsterGroupIdOrMonsterId]]._monsterRef]._nameArticle;
							if (var70 == 2)
								_enemyNamePt1 = "The ";
							else
								_enemyNamePt1 = "";

							switch (_stru32686[monsterGroupIdOrMonsterId]._field0[var86]) {
							case 1:
								snprintf((char *)_messageToBePrinted, 400, "%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							case 2:
								snprintf((char *)_messageToBePrinted, 400, "%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							default:
								snprintf((char *)_messageToBePrinted, 400, "%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							}
							_stru32686[monsterGroupIdOrMonsterId]._field0[var86] = 0;
							sub1C219(_messageToBePrinted, 1, 2, true);
						}
					}
				}
				// handleFight - Loop on var86 - End
			}
		}

		sub174A0();
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
	if ((_npcBuf[_teamCharId[charId]]._possessivePronounSHL6 >> 6) == 2) {
		_enemyNamePt1 = "The ";
	} else {
		_enemyNamePt1 = "";
	}

	// End of effect message depends on the type of effect
	switch (_teamCharStatus[charId]._status) {
	case 1:
		snprintf((char *)_messageToBePrinted, 400, "%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	case 2:
		snprintf((char *)_messageToBePrinted, 400, "%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	default:
		snprintf((char *)_messageToBePrinted, 400, "%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
		break;
	}

	// The character status is back to normal
	_teamCharStatus[charId]._status = 0;

	// Finally, display the message
	sub1C219(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_A(int16 teamCharId) {
	debug("handleFight_lastAction_A %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.

	int16 unk_monsterField5_itemId = sub1C80A(_teamCharId[teamCharId], 9, true);
	if (unk_monsterField5_itemId == 0x7FFF)
		unk_monsterField5_itemId = 0x3F;
	int16 monsterGroupNumber = _teamNextAttack[teamCharId];
	if (monsterGroupNumber == 0x64)
		monsterGroupNumber = 0;

	if (monsterGroupNumber == -1)
		return;
	int16 var58;
	if (_items[unk_monsterField5_itemId]._range == 4)
		var58 = 5;
	else
		var58 = monsterGroupNumber + 1;

	int16 var54;
	int16 teamMemberId;
	if (_items[unk_monsterField5_itemId]._range < 3) {
		teamMemberId = sub1DEC8(monsterGroupNumber);
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

			for (int16 var7E = teamMemberId; var7E < var54; ++var7E) {
				if (isMonsterActive(groupId, var7E) && var6E) {
					int16 var5C;
					if (unkFct_checkMonsterField8(groupId, true)) {
						sub1E028(groupId, 9, true);
						_unkArray2C8AA[0] += 500;
						var5C = -1;
					} else
						var5C = 0;

					int16 var76 = getRandom(_mapMonsters[_teamMonsterIdArray[groupId]]._field_6);
					int16 varInt = _teamCharId[teamCharId];
					int16 var51 = _npcBuf[varInt]._possessivePronounSHL6;
					var51 >>= 6;
					int16 var70 = var51;
					varInt = _teamMonsterIdArray[groupId];
					int16 var5E = kEncounters[_mapMonsters[varInt]._monsterRef]._nameArticle;
					int16 charScore = getCharacterScore(_teamCharId[teamCharId], unk_monsterField5_itemId);
					int16 var80 = _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E];
					int16 var62 = 0;
					int16 hitPoints = 0;
					int16 originalDamage = 0;
					int16 damagePointsAbsorbed = 0;
					int16 var64 = _items[unk_monsterField5_itemId]._attacks * _npcBuf[_teamCharId[teamCharId]]._speed;

					// Action A - Loop var84 - Start
					for (int var84 = 0; var84 < var64; ++var84) {
						if (getRandom(100) < charScore) {
							++var62;
							if (!hasAdequateDefense(_teamMonsterIdArray[groupId], _items[unk_monsterField5_itemId]._attackType)) {
								int16 var7C = getRandom(_items[unk_monsterField5_itemId]._damage);
								varInt = var7C - var76;
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

					hitPoints = originalDamage + damagePointsAbsorbed;

					if (!checkSpecialItemsOnCurrentPlace(unk_monsterField5_itemId))
						var62 = 0;

					if (var62 > 0) {
						_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] -= originalDamage;
						if (var62 > 1) {
							snprintf(_attackBuffer, 20, "%d times ", var62);
						} else {
							*_attackBuffer = 0;
						}
					}
					int16 var68 = _items[unk_monsterField5_itemId]._attackType + 1;
					int16 var6A = getRandom(3) - 1;
					if (var5E == 2) {
						snprintf(_characterNamePt1, 5, "The ");
					} else {
						*_characterNamePt1 = 0;
					}

					if (var70 == 2) {
						_enemyNamePt1 = "The ";
					} else {
						_enemyNamePt1 = "";
					}

					_characterNamePt2 = kEncounters[_mapMonsters[_teamMonsterIdArray[groupId]]._monsterRef]._name;
					_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
					_nameBuffer = _items[unk_monsterField5_itemId]._name;
					if (checkSpecialItemsOnCurrentPlace(unk_monsterField5_itemId)) {
						// Action A - Check damages - Start
						if (var62 == 0) {
							snprintf((char *)_messageToBePrinted, 400, "%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1, _characterNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
						} else if (hitPoints <= 0) {
							snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str());
						} else if (hitPoints == 1) {
							snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str());
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								strncat((char *)_messageToBePrinted, "!", 2);
							}
						} else {
							snprintf((char *)_messageToBePrinted, 400, "%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1, _characterNamePt2.c_str(), _attackBuffer, kPossessive[var70], _nameBuffer.c_str(), hitPoints);
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								strncat((char *)_messageToBePrinted, "!", 2);
							}
						}
						// Action A - Check damages - End

						// Action A - Add reaction text - Start
						if (var62 != 0 && originalDamage > 0 && getRandom(100) <= 35 && _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] > 0) {
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] - 5 <= originalDamage) {
								addReactionText(0);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] < var80 / 8) {
								addReactionText(1);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] < var80 / 4) {
								addReactionText(2);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] < var80 / 2) {
								addReactionText(3);
							} else if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] < var80 / 3) {
								// CHECKME: Doesn't make any sense to check /3 after /2... I don't get it
								addReactionText(4);
							} else if (var80 / 8 >= originalDamage) {
								addReactionText(5);
							} else if (originalDamage == 0 && getRandom(100) < 35) {
								addReactionText(6);
							}
						}
						// Action A - Add reaction text - End

						// Action A - Add armor absorb text - Start
						if (var76 && var62 && _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] > 0) {
							char buffer[80];
							memset(buffer, 0, 80);
							if (damagePointsAbsorbed <= 1)
								snprintf(buffer, 80, "  %s%s's armor absorbs 1 point!", _characterNamePt1, _characterNamePt2.c_str());
							else
								snprintf(buffer, 80, "  %s%s's armor absorbs %d points!", _characterNamePt1, _characterNamePt2.c_str(), damagePointsAbsorbed);

							strncat((char *)_messageToBePrinted, buffer, 80);
						}
						// Action A - Add armor absorb text - End

						if (var5C)
							strncat((char *)_messageToBePrinted, "  Your actions do not go un-noticed...", 400);

						// Action A - Check item durability - Start
						varInt = _teamCharId[teamCharId];
						var64 = sub1C80A(varInt, 9, false);
						if (var64 != 0x7FFF && (_npcBuf[varInt]._inventory[var64]._stat1 & 0x7F) != 0x7F) {
							var51 = _npcBuf[varInt]._inventory[var64]._stat1 & 0x7F;
							--var51;
							if (var51 <= 0) {
								char buffer[80];
								memset(buffer, 0, 80);
								snprintf(buffer, 80, "  * %s%s's %s breaks!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), _nameBuffer.c_str());
								strncat((char *)_messageToBePrinted, buffer, 80);
								setCharacterObjectToBroken(varInt, var64);
								var6E = false;
							} else {
								_npcBuf[varInt]._inventory[var64]._stat1 = (_npcBuf[varInt]._inventory[var64]._stat1 & 80) + var51;
							}
						}
						// Action A - Check item durability - End

						// Action A - Check effect - Start
						if (_items[unk_monsterField5_itemId].field_16 == 1 && _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] > 0) {
							if (getRandom(100) < 35) {
								_stru32686[var7E]._field0[groupId] = 1;
								_stru32686[var7E]._field2[groupId] = getRandom(10);
								char buffer[80];
								memset(buffer, 0, 80);
								snprintf(buffer, 80, "  %s%s falls asleep!", _characterNamePt1, _characterNamePt2.c_str());
								strncat((char *)_messageToBePrinted, buffer, 80);
							}
						} else if (_items[unk_monsterField5_itemId].field_16 == 2 && _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] > 0) {
							_stru32686[var7E]._field0[groupId] = 2;
							_stru32686[var7E]._field2[groupId] = getRandom(10);
							char buffer[80];
							memset(buffer, 0, 80);
							snprintf(buffer, 80, "  %s%s is frozen!", _characterNamePt1, _characterNamePt2.c_str());
							strncat((char *)_messageToBePrinted, buffer, 80);
						}
						// Action A - Check effect - End
					} else {
						snprintf((char *)_messageToBePrinted, 400, "%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
					}

					genericGenerateSound(_items[unk_monsterField5_itemId]._attackType, var62);
					sub1C219(_messageToBePrinted, 1, 2, true);
				}
			}
		}
	}
}

void EfhEngine::handleFight_lastAction_D(int16 teamCharId) {
	debug("handleFight_lastAction_D %d", teamCharId);

	_word32482[teamCharId] -= 40;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	int16 var70 = _npcBuf[_teamCharId[teamCharId]]._possessivePronounSHL6 >> 6;

	if (var70 == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	snprintf((char *)_messageToBePrinted, 400, "%s%s prepares to defend %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[var70]);
	sub1C219(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_H(int16 teamCharId) {
	debug("handleFight_lastAction_H %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.

	_teamPctVisible[teamCharId] -= 50;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	int16 var70 = _npcBuf[_teamCharId[teamCharId]]._possessivePronounSHL6 >> 6;

	if (var70 == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	snprintf((char *)_messageToBePrinted, 400, "%s%s attempts to hide %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[var70]);
	sub1C219(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_U(int16 teamCharId) {
	debug("handleFight_lastAction_U %d", teamCharId);

	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	int16 unk_monsterField5_itemId = _npcBuf[_teamCharId[teamCharId]]._inventory[_word31780[teamCharId]]._ref;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	_nameBuffer = _items[unk_monsterField5_itemId]._name;
	int16 var70 = _npcBuf[_teamCharId[teamCharId]]._possessivePronounSHL6 >> 6;
	if (var70 == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	snprintf((char *)_messageToBePrinted, 400, "%s%s uses %s %s!  ", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
	sub1C219(_messageToBePrinted, 1, 2, true);
}

bool EfhEngine::sub1BC74(int16 monsterId, int16 teamMonsterId) {
	debug("sub1BC74 %d %d", monsterId, teamMonsterId);

	for (int counter = 0; counter < teamMonsterId; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId)
			return true;
	}
	return false;
}

void EfhEngine::sub1BCA7(int16 monsterTeamId) {
	debug("sub1BCA7 %d", monsterTeamId);

	int16 counter = 0;
	if (monsterTeamId != -1 && countPictureRef(monsterTeamId, false) > 0) {
		counter = 1;
		_teamMonsterIdArray[0] = monsterTeamId;
	}

	for (int counter2 = 1; counter2 <= 3; ++counter2) {
		if (counter >= 5)
			break;

		for (uint monsterId = 0; monsterId < 64; ++monsterId) {
			if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
				continue;

			if (((_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) != 0x3F || isCharacterATeamMember(_mapMonsters[monsterId]._field_1)) && (_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) > 0x3D)
				continue;

			if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
				continue;

			bool var6 = false;
			for (uint counter3 = 0; counter3 < 9; ++counter3) {
				if (_mapMonsters[monsterId]._pictureRef[counter3] > 0) {
					var6 = true;
					break;
				}
			}

			if (var6) {
				if (computeMonsterGroupDistance(monsterId) <= counter2 && !sub1BC74(monsterId, counter)) {
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

void EfhEngine::reset_stru32686() {
	debug("reset_stru32686");
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		for (uint counter2 = 0; counter2 < 9; ++counter2) {
			_stru32686[counter1]._field0[counter2] = 0;
			_stru32686[counter1]._field2[counter2] = 0;
		}
	}
}

void EfhEngine::sub1BE89(int16 monsterId) {
	debug("sub1BE89 %d", monsterId);
	sub1BCA7(monsterId);
	reset_stru32686();
}

void EfhEngine::resetTeamMonsterIdArray() {
	debug("resetTeamMonsterIdArray");

	for (int i = 0; i < 5; ++i) {
		_teamMonsterIdArray[i] = -1;
	}
}

} // End of namespace Efh
