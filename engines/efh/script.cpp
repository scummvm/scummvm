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

#include "common/system.h"
#include "efh/efh.h"

namespace Efh {

uint8 *EfhEngine::script_readNumberArray(uint8 *srcBuffer, int16 destArraySize, int16 *destArray) {
	debugC(6, kDebugScript, "script_readNumberArray");

	uint8 *buffer = srcBuffer;
	for (int i = 0; i < destArraySize; ++i) {
		buffer++;
		buffer = script_getNumber(buffer, &destArray[i]);
	}

	return buffer;
}

uint8 *EfhEngine::script_getNumber(uint8 *srcBuffer, int16 *retBuf) {
	debugC(6, kDebugScript, "script_getNumber");

	uint8 *buffer = srcBuffer;
	int16 retVal = 0;
	for (;;) {
		uint8 curChar = *buffer;
		if (curChar < 0x30 || curChar > 0x39) {
			break;
		}
		retVal = retVal * 10 + curChar - 0x30;
		buffer++;
	}

	*retBuf = retVal;
	return buffer;
}

int16 EfhEngine::script_parse(Common::String stringBuffer, int16 posX, int16 posY, int16 maxX, int16 maxY, bool scriptExecuteFlag) {
	debugC(3, kDebugScript, "script_parse stringBuffer %d-%d %d-%d %s", posX, posY, maxX, maxY, scriptExecuteFlag ? "True" : "False");
	debugC(6, kDebugScript, "%s", stringBuffer.c_str());

	bool doneFlag = false;
	bool noTextFlag = true;
	int16 retVal = 0xFF;
	int16 joiningNpcId = 0xFF;
	uint16 curLineNb = 0;
	int16 numbLines = (1 + maxY - posY) / 9;
	int16 width = maxX - posX;
	int16 spaceWidth = getStringWidth(" ");
	uint8 *buffer = (uint8 *)stringBuffer.c_str();
	char nextWord[80];
	Common::String curLine = "";
	memset(nextWord, 0, sizeof(nextWord));
	int16 curWordPos = 0;
	setTextPos(posX, curLineNb * 9 + posY);

	while (!doneFlag) {
		uint8 curChar = *buffer;
		if (curChar != 0x5E && curChar != 0x20 && curChar != 0 && curChar != 0x7C) { // '^', ' ', NUL, '|'
			noTextFlag = false;
			nextWord[curWordPos++] = curChar;
			++buffer;
			continue;
		}

		if (curChar != 0x5E) { // '^'
			if (curChar == 0)
				doneFlag = true;
			else if (curChar == 0x7C) // '|'
				noTextFlag = false;

			nextWord[curWordPos] = 0;
			int16 widthNextWord = getStringWidth(nextWord);
			int16 widthCurrentLine = spaceWidth + getStringWidth(curLine.c_str());

			if (widthCurrentLine + widthNextWord > width || curChar == 0x7C) { // '|'
				if (curLineNb >= numbLines) {
					doneFlag = true;
				} else {
					if (!noTextFlag)
						displayStringAtTextPos(curLine);

					curLine = Common::String(nextWord) + " ";
					++curLineNb;
					setTextPos(posX, posY + curLineNb * 9);
					curWordPos = 0;
				}
			} else {
				curLine += Common::String(nextWord) + " ";
				curWordPos = 0;
			}
			++buffer;
			continue;
		}

		// At this point, curChar == 0x5E '^'
		++buffer;
		int16 opCode = 0;
		buffer = script_getNumber(buffer, &opCode);
		int16 scriptNumberArray[10];
		memset(scriptNumberArray, 0, sizeof(scriptNumberArray));

		switch (opCode) {
		case 0x00:
			// Enter room { full Place Id, posX, posY }
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				if (_largeMapFlag) {
					_largeMapFlag = false;
					_techDataId_MapPosX = _mapPosX;
					_techDataId_MapPosY = _mapPosY;
				}
				_oldMapPosX = _mapPosX = scriptNumberArray[1];
				_oldMapPosY = _mapPosY = scriptNumberArray[2];
				loadPlacesFile(scriptNumberArray[0], false);
				_checkTileDisabledByScriptFl = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x01:
			// Exit room { }
			if (scriptExecuteFlag) {
				_largeMapFlag = true;
				_oldMapPosX = _mapPosX = _techDataId_MapPosX;
				_oldMapPosY = _mapPosY = _techDataId_MapPosY;
				_checkTileDisabledByScriptFl = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x02:
			// Change map. { map number, posX, posY }
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				writeTechAndMapFiles();
				_oldMapPosX = _mapPosX = scriptNumberArray[1];
				_oldMapPosY = _mapPosY = scriptNumberArray[2];
				loadTechMapImp(scriptNumberArray[0]);
				_largeMapFlag = true;
				_checkTileDisabledByScriptFl = true;
				_redrawNeededFl = true;
				doneFlag = true;
			}
			break;
		case 0x03:
			buffer = script_readNumberArray(buffer, 4, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 rangeX = scriptNumberArray[2] - scriptNumberArray[0];
				int16 rangeY = scriptNumberArray[3] - scriptNumberArray[1];

				_mapPosX = getRandom(rangeX) + scriptNumberArray[0] - 1;
				_mapPosY = getRandom(rangeY) + scriptNumberArray[1] - 1;
				_checkTileDisabledByScriptFl = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x04:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				_mapPosX = scriptNumberArray[0];
				_mapPosY = scriptNumberArray[1];
				_checkTileDisabledByScriptFl = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x05:
			buffer = script_readNumberArray(buffer, 4, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 npcId = _teamChar[scriptNumberArray[0]]._id;
				if (npcId != -1) {
					int16 scoreId = scriptNumberArray[1];
					_npcBuf[npcId]._activeScore[scoreId] += scriptNumberArray[2] & 0xFF;
					_npcBuf[npcId]._activeScore[scoreId] -= scriptNumberArray[3] & 0xFF;
				}
			}
			break;
		case 0x06:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 npcId = _teamChar[scriptNumberArray[0]]._id;
				if (npcId != -1) {
					int16 scoreId = scriptNumberArray[1];
					_npcBuf[npcId]._activeScore[scoreId] = scriptNumberArray[2] & 0xFF;
				}
			}
			break;
		case 0x07:
			if (scriptExecuteFlag) {
				totalPartyKill();
			}
			break;
		case 0x08:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag && scriptNumberArray[0] != -1) {
				_npcBuf[_teamChar[scriptNumberArray[0]]._id]._hitPoints = 0;
			}
			break;
		case 0x09:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 npcId = _teamChar[scriptNumberArray[0]]._id;
				if (npcId != -1) {
					_npcBuf[npcId]._hitPoints += getRandom(scriptNumberArray[1]);
					if (_npcBuf[npcId]._hitPoints > _npcBuf[npcId]._maxHP)
						_npcBuf[npcId]._hitPoints = _npcBuf[npcId]._maxHP;
				}
			}
			break;
		case 0x0A:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 npcId = _teamChar[scriptNumberArray[0]]._id;
				if (npcId != -1) {
					_npcBuf[npcId]._hitPoints = _npcBuf[npcId]._maxHP;
				}
			}
			break;
		case 0x0B:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 npcId = _teamChar[scriptNumberArray[0]]._id;
				if (npcId != -1) {
					_npcBuf[npcId]._hitPoints -= getRandom(scriptNumberArray[1]);
					if (_npcBuf[npcId]._hitPoints < 0)
						_npcBuf[npcId]._hitPoints = 0;
				}
			}
			break;
		case 0x0C:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 scriptItemId = scriptNumberArray[0];
				bool found = false;
				for (int counter = 0; counter < _teamSize && !found; ++counter) {
					for (uint objectId = 0; objectId < 10; ++objectId) {
						if (_npcBuf[_teamChar[counter]._id]._inventory[objectId]._ref == scriptItemId) {
							removeObject(_teamChar[counter]._id, objectId);
							found = true;
							break;
						}
					}
				}
			}
			break;
		case 0x0D:
			// Put item in inventory { objectId }
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 scriptObjectId = scriptNumberArray[0];
				for (int counter = 0; counter < _teamSize; ++counter) {
					if (giveItemTo(_teamChar[counter]._id, scriptObjectId, 0xFF))
						break;
				}
			}
			break;
		case 0x0E:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 scriptItemId = scriptNumberArray[0];
				bool found = false;
				for (int counter = 0; counter < _teamSize && !found; ++counter) {
					for (uint objectId = 0; objectId < 10; ++objectId) {
						if (_npcBuf[_teamChar[counter]._id]._inventory[objectId]._ref == scriptItemId) {
							found = true;
							break;
						}
					}
				}

				if (found)
					retVal = scriptNumberArray[1];
				else
					retVal = scriptNumberArray[2];
			}
			break;
		case 0x0F:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				if (isNpcATeamMember(scriptNumberArray[0]))
					retVal = scriptNumberArray[1];
				else
					retVal = scriptNumberArray[2];
			}
			break;
		case 0x10:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag)
				retVal = scriptNumberArray[0];

			break;
		case 0x11:
			if (scriptExecuteFlag)
				_alertDelay = 0;
			break;
		case 0x12:
			// Disable special tile
			if (scriptExecuteFlag) {
				int16 tileId = findMapSpecialTileIndex(_mapPosX, _mapPosY);
				if (tileId != -1)
					_mapSpecialTiles[_techId][tileId]._posX = 0xFF;
			}
			break;
		case 0x13:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag && _largeMapFlag) {
				_textBoxDisabledByScriptFl = true;
				loadPlacesFile(scriptNumberArray[0], false);
				transitionMap(scriptNumberArray[1], scriptNumberArray[2]);
				setSpecialTechZone(scriptNumberArray[0], scriptNumberArray[1], scriptNumberArray[2]);
				retVal = -1;
			}
			break;
		case 0x14:
			// Add character to team { charId }
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 scriptNpcId = scriptNumberArray[0];
				if (!isNpcATeamMember(scriptNpcId))
					joiningNpcId = scriptNpcId;
				retVal = -1;
			}
			break;
		case 0x15:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				_oldMapPosX = _mapPosX = scriptNumberArray[0];
				_oldMapPosY = _mapPosY = scriptNumberArray[1];
				_largeMapFlag = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x16:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 scriptNpcId = scriptNumberArray[0];
				// TODO: This "if" is useless, it's doing just the same loop and if statement. Consider removing it.
				if (isNpcATeamMember(scriptNpcId)) {
					for (uint counter = 0; counter < 3; ++counter) {
						if (_teamChar[counter]._id == scriptNpcId) {
							removeCharacterFromTeam(counter);
							break;
						}
					}
				}
			}
			break;
		case 0x17:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 animId = scriptNumberArray[0];
				displayAnimFrames(animId, true);
			}
			break;
		case 0x18:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				bool found = false;
				int16 scriptRandomItemId = getRandom(scriptNumberArray[1] - scriptNumberArray[0] + 1) + scriptNumberArray[0] - 1;
				int16 counter;
				for (counter = 0; counter < _teamSize; ++counter) {
					if (giveItemTo(_teamChar[counter]._id, scriptRandomItemId, 0xFF)) {
						found = true;
						break;
					}
				}

				if (!found) {
					drawMapWindow();
					displayFctFullScreen();
					drawMapWindow();
					scriptRandomItemId = displayBoxWithText("Nothing...", 1, 2, true);
					displayFctFullScreen();
				} else {
					_enemyNamePt2 = _npcBuf[_teamChar[counter]._id]._name;
					_nameBuffer = _items[scriptRandomItemId]._name;
					curLine = Common::String::format("%s finds a %s!", _enemyNamePt2.c_str(), _nameBuffer.c_str());
					drawMapWindow();
					displayFctFullScreen();
					drawMapWindow();
					scriptRandomItemId = displayBoxWithText(curLine, 1, 2, true);
					displayFctFullScreen();
				}

				int16 tileId = findMapSpecialTileIndex(_mapPosX, _mapPosY);
				if (tileId != -1) {
					// Disable special tile
					_mapSpecialTiles[_techId][tileId]._posX = 0xFF;
				}
				_redrawNeededFl = true;
			}
			break;
		case 0x19:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				if (_largeMapFlag) {
					_mapGameMaps[_techId][scriptNumberArray[0]][scriptNumberArray[1]] = scriptNumberArray[2] & 0xFF;
				} else {
					_curPlace[scriptNumberArray[0]][scriptNumberArray[1]] = scriptNumberArray[2] & 0xFF;
				}
			}
			break;
		case 0x1A:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 tileId = findMapSpecialTileIndex(scriptNumberArray[0], scriptNumberArray[1]);
				if (tileId != -1) {
					// Disable tile
					_mapSpecialTiles[_techId][tileId]._posX = 0xFF;
				}
			}
			break;
		case 0x1B:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				int16 tileId = findMapSpecialTileIndex(scriptNumberArray[0], scriptNumberArray[1]);
				if (tileId != -1) {
					// Disable tile
					_mapSpecialTiles[_techId][tileId]._posX = 0xFF;
				}
				_mapSpecialTiles[_techId][scriptNumberArray[2]]._posX = scriptNumberArray[0];
				_mapSpecialTiles[_techId][scriptNumberArray[2]]._posY = scriptNumberArray[1];
			}
			break;
		case 0x1C:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				_history[scriptNumberArray[0]] = 0xFF;
			}
			break;
		case 0x1D:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag) {
				_history[scriptNumberArray[0]] = 0;
			}
			break;
		case 0x1E:
			// Dialog with condition { historyId, dialogId1, dialogId2 }
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (scriptExecuteFlag) {
				if (_history[scriptNumberArray[0]] == 0)
					retVal = scriptNumberArray[2];
				else
					retVal = scriptNumberArray[1];
			}
			break;
		case 0x1F:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (scriptExecuteFlag)
				_alertDelay = scriptNumberArray[0];

			break;
		case 0x20:
			if (scriptExecuteFlag) {
				handleWinSequence();
				_system->quit();
			}
		default:
			break;
		}
	}

	if (curLineNb < numbLines && !noTextFlag)
		displayStringAtTextPos(curLine);

	if (joiningNpcId != 0xFF) {
		displayLowStatusScreen(true);
		int16 teamSlot = handleCharacterJoining();
		if (teamSlot > -1) {
			_teamChar[teamSlot]._id = joiningNpcId;
		}
		refreshTeamSize();
	}

	return retVal;
}

} // End of namespace Efh

