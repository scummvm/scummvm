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

			bool found = false;
			for (uint counter3 = 0; counter3 < 9; ++counter3) {
				if (_mapMonsters[monsterId]._pictureRef[counter3] > 0) {
					found = true;
					break;
				}
			}

			if (found) {
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

void EfhEngine::sub1BE89(int16 monsterId) {
	debug("sub1BE89 %d", monsterId);
	sub1BCA7(monsterId);
	reset_stru32686();
}

bool EfhEngine::handleFight(int16 monsterId) {
	debug("handleFight %d", monsterId);

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
		sub1C219("", 2, 1, false);

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
										_attackBuffer = Common::String::format("%d times ", var62);
									else
										_attackBuffer = "";
								}

								int16 var68 = _items[unk_monsterField5_itemId]._attackType + 1;
								int16 var6A = getRandom(3);
								if (var5E == 2)
									_characterNamePt1 = "The ";
								else
									_characterNamePt1 = "";

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
										_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
									} else if (hitPoints <= 0) {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str());
									} else if (hitPoints == 1) {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str());
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											_messageToBePrinted += "!";
									} else {
										_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[var68 * 3 + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str(), hitPoints);
										if (_npcBuf[_teamCharId[var7E]]._hitPoints <= 0)
											getDeathTypeDescription(var7E + 1000, monsterGroupIdOrMonsterId);
										else
											_messageToBePrinted += "!";
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
										if (damagePointsAbsorbed <= 1)
											_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
										else
											_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);

										varInt = (originalDamage + damagePointsAbsorbed) / 10;
										sub1D8C2(_teamCharId[var7E], varInt);
									}
									// handleFight - Check armor - end

									// handleFight - Check effect - start
									switch (_items[unk_monsterField5_itemId].field_16) {
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
									_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
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
								_messageToBePrinted = Common::String::format("%s%s wakes up!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							case 2:
								_messageToBePrinted = Common::String::format("%s%s thaws out!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
								break;
							default:
								_messageToBePrinted = Common::String::format("%s%s recovers!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str());
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
							_attackBuffer = Common::String::format("%d times ", var62);
						} else {
							_attackBuffer = "";
						}
					}
					int16 var68 = _items[unk_monsterField5_itemId]._attackType + 1;
					int16 var6A = getRandom(3) - 1;
					if (var5E == 2) {
						_characterNamePt1 = "The ";
					} else {
						_characterNamePt1 = "";
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
							_messageToBePrinted = Common::String::format("%s%s %s at %s%s with %s %s, but misses!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
						} else if (hitPoints <= 0) {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s, but does no damage!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str());
						} else if (hitPoints == 1) {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for 1 point", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str());
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								_messageToBePrinted += "!";
							}
						} else {
							_messageToBePrinted = Common::String::format("%s%s %s %s%s %swith %s %s for %d points", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kAttackVerbs[(var68 * 3) + var6A], _characterNamePt1.c_str(), _characterNamePt2.c_str(), _attackBuffer.c_str(), kPossessive[var70], _nameBuffer.c_str(), hitPoints);
							if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] <= 0) {
								getDeathTypeDescription(groupId, teamCharId + 1000);
								getXPAndSearchCorpse(_teamCharId[teamCharId], _enemyNamePt1, _enemyNamePt2, _teamMonsterIdArray[groupId]);
							} else {
								_messageToBePrinted += "!";
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
							if (damagePointsAbsorbed <= 1)
								_messageToBePrinted += Common::String::format("  %s%s's armor absorbs 1 point!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
							else
								_messageToBePrinted += Common::String::format("  %s%s's armor absorbs %d points!", _characterNamePt1.c_str(), _characterNamePt2.c_str(), damagePointsAbsorbed);
						}
						// Action A - Add armor absorb text - End

						if (var5C)
							_messageToBePrinted += Common::String("  Your actions do not go un-noticed...");

						// Action A - Check item durability - Start
						varInt = _teamCharId[teamCharId];
						var64 = sub1C80A(varInt, 9, false);
						if (var64 != 0x7FFF && (_npcBuf[varInt]._inventory[var64]._stat1 & 0x7F) != 0x7F) {
							var51 = _npcBuf[varInt]._inventory[var64]._stat1 & 0x7F;
							--var51;
							if (var51 <= 0) {
								_messageToBePrinted += Common::String::format("  * %s%s's %s breaks!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), _nameBuffer.c_str());
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
								_messageToBePrinted += Common::String::format("  %s%s falls asleep!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
							}
						} else if (_items[unk_monsterField5_itemId].field_16 == 2 && _mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[var7E] > 0) {
							_stru32686[var7E]._field0[groupId] = 2;
							_stru32686[var7E]._field2[groupId] = getRandom(10);
							_messageToBePrinted += Common::String::format("  %s%s is frozen!", _characterNamePt1.c_str(), _characterNamePt2.c_str());
						}
						// Action A - Check effect - End
					} else {
						_messageToBePrinted = Common::String::format("%s%s tries to use %s %s, but it doesn't work!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
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

	_messageToBePrinted = Common::String::format("%s%s prepares to defend %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[var70]);
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

	_messageToBePrinted = Common::String::format("%s%s attempts to hide %sself!", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPersonal[var70]);
	sub1C219(_messageToBePrinted, 1, 2, true);
}

void EfhEngine::handleFight_lastAction_U(int16 teamCharId) {
	debug("handleFight_lastAction_U %d", teamCharId);

	// Fight - Action 'U' - Use Item
	// In the original, this function is part of handleFight.
	// It has been split for readability purposes.
	int16 itemId = _npcBuf[_teamCharId[teamCharId]]._inventory[_word31780[teamCharId]]._ref;
	_enemyNamePt2 = _npcBuf[_teamCharId[teamCharId]]._name;
	_nameBuffer = _items[itemId]._name;
	int16 var70 = _npcBuf[_teamCharId[teamCharId]]._possessivePronounSHL6 >> 6;
	if (var70 == 2)
		_enemyNamePt1 = "The ";
	else
		_enemyNamePt1 = "";

	_messageToBePrinted = Common::String::format("%s%s uses %s %s!  ", _enemyNamePt1.c_str(), _enemyNamePt2.c_str(), kPossessive[var70], _nameBuffer.c_str());
	sub1C219(_messageToBePrinted, 1, 2, true);
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

bool EfhEngine::sub1BC74(int16 monsterId, int16 teamMonsterId) {
	debug("sub1BC74 %d %d", monsterId, teamMonsterId);

	for (int counter = 0; counter < teamMonsterId; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId)
			return true;
	}
	return false;
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

void EfhEngine::resetTeamMonsterIdArray() {
	debug("resetTeamMonsterIdArray");

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

	int16 possessivePronoun;

	if (attackerId > 999) {
		int16 charId = _teamCharId[attackerId - 1000];
		possessivePronoun = _npcBuf[charId]._possessivePronounSHL6 >> 6;
	} else {
		int16 charId = _teamMonsterIdArray[attackerId];
		possessivePronoun = _mapMonsters[charId]._possessivePronounSHL6 >> 6;
	}

	if (possessivePronoun > 2)
		possessivePronoun = 2;

	int16 deathType;
	if (getRandom(100) < 20) {
		deathType = 0;
	} else {
		if (victimId >= 1000) {
			int16 charId = _teamCharId[victimId - 1000];
			if (charId == -1)
				deathType = 0;
			else {
				int16 var6 = sub1C80A(charId, 9, true);
				if (var6 == 0x7FFF)
					deathType = 0;
				else
					deathType = _items[var6]._attackType + 1;
			}
		} else if (_teamMonsterIdArray[victimId] == -1)
			deathType = 0;
		else {
			int16 itemId = _mapMonsters[_teamMonsterIdArray[victimId]]._itemId_Weapon;
			deathType = _items[itemId]._attackType;
		}
	}

	int16 rndDescrForDeathType = getRandom((3)) - 1;
	Common::String tmpStr = "DUDE IS TOAST!";
	switch (deathType) {
	case 0:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", killing %s!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", slaughtering %s!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", annihilating %s!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 1:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", cutting %s in two!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", dicing %s into small cubes!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", butchering %s into lamb chops!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", piercing %s heart!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", leaving %s a spouting mass of blood!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", popping %s like a zit!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 3:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", pulping %s head over a wide area!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", smashing %s into a meat patty!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", squashing %s like a ripe tomato!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 4:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", totally incinerating %s!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", reducing %s to a pile of ash!", kPersonal[possessivePronoun]);
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
			tmpStr = Common::String::format(", turning %s into a popsicle!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", encasing %s in a block of ice!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", shattering %s into shards!", kPersonal[possessivePronoun]);
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
			tmpStr = Common::String::format(", bursting %s head like a bubble!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", turning %s into a mindless vegetable", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 7:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", reducing %s to an oozing pile of flesh!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", melting %s like an ice cube in hot coffee!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", vaporizing %s into a steaming cloud!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 8:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", engulfing %s in black smoke puffs!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", sucking %s into eternity!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", turning %s into a mindless zombie!", kPersonal[possessivePronoun]);
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
			tmpStr = Common::String::format(", completely disintegrating %s!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", spreading %s into a fine mist!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", leaving a smoking crater in %s place!", kPersonal[possessivePronoun]);
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
			tmpStr = Common::String::format(", tearing a chunk out of %s back!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", blowing %s brains out!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", exploding %s entire chest!", kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 15:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", choking %s to death!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", melting %s lungs!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", leaving %s gasping for air as %s collapses!", kPersonal[possessivePronoun], kPersonal[possessivePronoun]);
			break;
		default:
			break;
		}
		break;
	case 16:
		switch (rndDescrForDeathType) {
		case 0:
			tmpStr = Common::String::format(", tearing a chunk out of %s back!", kPersonal[possessivePronoun]);
			break;
		case 1:
			tmpStr = Common::String::format(", piercing %s heart!", kPersonal[possessivePronoun]);
			break;
		case 2:
			tmpStr = Common::String::format(", impaling %s brain!", kPersonal[possessivePronoun]);
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

	int16 var6 = sub1C80A(charId, unkFied18Val, true);
	int16 range = 0;
	if (var6 != 0x7FFF)
		range = _items[var6]._range;

	switch (range) {
	case 3:
	case 2:
		++range;
	case 1:
		++range;
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
				sub1C219("Select Monster Group:", 3, 0, false);

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
						sub1C219("Select Character:", 3, 1, false);
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

void EfhEngine::drawCombatScreen(int16 charId, bool whiteFl, bool forceDrawFl) {
	debug("drawCombatScreen %d %s %s", charId, whiteFl ? "True" : "False", forceDrawFl ? "True" : "False");

	for (uint counter = 0; counter < 2; ++counter) {
		if (counter == 0 || forceDrawFl) {
			drawMapWindow();
			displayCenteredString("Combat", 128, 303, 9);
			drawColoredRect(200, 112, 278, 132, 0);
			displayCenteredString("'T' for Terrain", 128, 303, 117);
			sub1C219("", 1, 0, false);
			sub1C4CA(whiteFl);
			displayCombatMenu(charId);
			displayLowStatusScreen(false);
		}

		if (counter == 0 && forceDrawFl)
			displayFctFullScreen();
	}
}

void EfhEngine::getXPAndSearchCorpse(int16 charId, Common::String namePt1, Common::String namePt2, int16 monsterId) {
	debug("getXPAndSearchCorpse %d %s%s %d", charId, namePt1.c_str(), namePt2.c_str(), monsterId);

	int16 xpLevel = getXPLevel(_npcBuf[charId]._xp);
	_npcBuf[charId]._xp += kEncounters[_mapMonsters[monsterId]._monsterRef]._xpGiven;

	if (getXPLevel(_npcBuf[charId]._xp) > xpLevel) {
		generateSound(15);
		int16 var2 = getRandom(20) + getRandom(_npcBuf[charId]._infoScore[4]);
		_npcBuf[charId]._hitPoints += var2;
		_npcBuf[charId]._maxHP += var2;
		_npcBuf[charId]._infoScore[0] += getRandom(3) - 1;
		_npcBuf[charId]._infoScore[1] += getRandom(3) - 1;
		_npcBuf[charId]._infoScore[2] += getRandom(3) - 1;
		_npcBuf[charId]._infoScore[3] += getRandom(3) - 1;
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
	debug("addReactionText %d", id);

	int16 rand3 = getRandom(3);

	switch (id) {
	case 0:
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
	case 1:
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
	case 2:
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
	case 3:
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
	case 4:
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
	case 5:
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
	case 6:
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
	debug("sub1C4CA %s", whiteFl ? "True" : "False");

	int16 textPosY = 20;
	for (uint counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == -1)
			continue;

		int16 var6C = computeMonsterGroupDistance(_teamMonsterIdArray[counter]);
		int16 var6E = countMonsterGroupMembers(counter);
		if (whiteFl)
			setTextColorWhite();
		else
			setTextColorGrey();

		setTextPos(129, textPosY);
		char buffer[80];
		snprintf(buffer, 80, "%c)", 'A' + counter);
		displayStringAtTextPos(buffer);
		setTextColorRed();
		int16 var1 = _mapMonsters[_teamMonsterIdArray[counter]]._possessivePronounSHL6 & 0x3F;
		if (var1 <= 0x3D) {
			snprintf(buffer, 80, "%d %s", var6E, kEncounters[_mapMonsters[_teamMonsterIdArray[counter]]._monsterRef]._name);
			displayStringAtTextPos(buffer);
			if (var6E > 1)
				displayStringAtTextPos("s");
		} else if (var1 == 0x3E) {
			displayStringAtTextPos("(NOT DEFINED)");
		} else if (var1 == 0x3F) {
			Common::String stringToDisplay = _npcBuf[_mapMonsters[_teamMonsterIdArray[counter]]._field_1]._name;
			displayStringAtTextPos(stringToDisplay);
		}

		setTextPos(228, textPosY);
		if (unkFct_checkMonsterField8(counter, true)) {
			_textColor = 0xE;
			displayStringAtTextPos("Hostile");
		} else {
			_textColor = 0x2;
			displayStringAtTextPos("Friendly");
		}

		setTextColorRed();
		switch (var6C) {
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

int16 EfhEngine::sub1DEC8(int16 groupNumber) {
	debug("sub1DEC8 %d", groupNumber);

	int16 var4 = -1;
	int16 monsterId = _teamMonsterIdArray[groupNumber];

	if (monsterId == -1)
		return -1;

	for (uint counter = 0; counter < 9; ++counter) {
		if (isMonsterActive(groupNumber, counter)) {
			var4 = counter;
			break;
		}
	}

	for (int16 counter = var4 + 1; counter < 9; ++counter) {
		if (!isMonsterActive(groupNumber, counter))
			continue;

		if (_mapMonsters[monsterId]._pictureRef[var4] > _mapMonsters[monsterId]._pictureRef[counter])
			var4 = counter;
	}

	if (_mapMonsters[monsterId]._pictureRef[var4] <= 0)
		return -1;

	return var4;
}

int16 EfhEngine::getCharacterScore(int16 charId, int16 itemId) {
	debug("getCharacterScore %d %d", charId, itemId);

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

	int16 grandTotalScore = totalScore + extraScore;
	if (grandTotalScore > 60)
		grandTotalScore = 60;

	int16 retVal = CLIP(grandTotalScore + 30, 5, 90);
	return retVal;
}

bool EfhEngine::checkSpecialItemsOnCurrentPlace(int16 itemId) {
	debug("checkSpecialItemsOnCurrentPlace %d", itemId);

	switch (_techDataArr[_techId][_techDataId_MapPosX * 64 + _techDataId_MapPosY]) {
	case 1:
		if ((itemId < 0x58 || itemId > 0x68) && (itemId < 0x86 || itemId > 0x89) && (itemId < 0x74 || itemId > 0x76) && (itemId != 0x8C))
			return true;
		return false;
	case 2:
		if ((itemId < 0x61 || itemId > 0x63) && (itemId < 0x74 || itemId > 0x76) && (itemId < 0x86 || itemId > 0x89) && (itemId < 0x5B || itemId > 0x5E) && (itemId < 0x66 || itemId > 0x68) && (itemId != 0x8C))
			return true;
		return false;
	default:
		return true;
	}
}

bool EfhEngine::hasAdequateDefense(int16 monsterId, uint8 attackType) {
	debug("hasAdequateDefense %d %d", monsterId, attackType);

	int16 itemId = _mapMonsters[monsterId]._itemId_Weapon;

	if (_items[itemId].field_16 != 0)
		return false;

	return _items[itemId].field17_attackTypeDefense == attackType;
}

bool EfhEngine::hasAdequateDefense_2(int16 charId, uint8 attackType) {
	debug("hasAdequateDefense_2 %d %d", charId, attackType);

	int16 itemId = _npcBuf[charId]._unkItemId;

	if (_items[itemId].field_16 == 0 && _items[itemId].field17_attackTypeDefense == attackType)
		return true;

	for (uint counter = 0; counter < 10; ++counter) {
		if (_npcBuf[charId]._inventory[counter]._ref == 0x7FFF || _npcBuf[charId]._inventory[counter]._stat1 == 0x80)
			continue;

		itemId = _npcBuf[charId]._inventory[counter]._ref;
		if (_items[itemId].field_16 == 0 && _items[itemId].field17_attackTypeDefense == attackType)
			return true;
	}
	return false;
}

} // End of namespace Efh
