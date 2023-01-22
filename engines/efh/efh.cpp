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
#include "common/random.h"
#include "common/error.h"
#include "common/events.h"
#include "engines/util.h"

#include "efh/efh.h"
#include "efh/constants.h"

namespace Efh {

void EfhGraphicsStruct::copy(EfhGraphicsStruct *src) {
	// Same buffer address
	_vgaLineBuffer = src->_vgaLineBuffer;
	_shiftValue = src->_shiftValue;
	_width = src->_width;
	_height = src->_height;
	_area = src->_area;
}

bool InvObject::isEquipped() {
	return (_stat1 & 0x80) != 0;
}

int8 InvObject::getUsesLeft() {
	return _stat1 & 0x7F;
}

EfhEngine::~EfhEngine() {
	delete _rnd;
	delete _graphicsStruct;
	delete _vgaGraphicsStruct1;
	delete _vgaGraphicsStruct2;
}

Common::Error EfhEngine::run() {
	debug("run");

	initialize();
	initGraphics(320, 200);

	_mainSurface = new Graphics::Surface();
	_mainSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	initPalette();

	// Setup mixer
	syncSoundSettings();

	// Sometimes a ghost key event stops the intro, so we ass a short delay and purge the keyboard events
	_system->delayMillis(100);
	_system->getEventManager()->purgeKeyboardEvents();

	initEngine();
	drawGameScreenAndTempText(true);
	drawScreen();
	displayLowStatusScreen(true);

	if (!_protectionPassed)
		return Common::kNoError;

	uint32 lastMs = _system->getMillis();
	while (!_shouldQuit) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 220) {
			lastMs = newMs;
			handleAnimations();
		}

		Common::Event event;
		Common::KeyCode retVal = getLastCharAfterAnimCount(4);

		switch (retVal) {
		case Common::KEYCODE_DOWN:
		case Common::KEYCODE_KP2:
			goSouth();
			_imageSetSubFilesIdx = 144;
			break;
		case Common::KEYCODE_UP:
		case Common::KEYCODE_KP8:
			goNorth();
			_imageSetSubFilesIdx = 145;
			break;
		case Common::KEYCODE_RIGHT:
		case Common::KEYCODE_KP6:
			goEast();
			_imageSetSubFilesIdx = 146;
			break;
		case Common::KEYCODE_LEFT:
		case Common::KEYCODE_KP4:
			goWest();
			_imageSetSubFilesIdx = 147;
			break;
		case Common::KEYCODE_PAGEUP:
		case Common::KEYCODE_KP9:
			goNorthEast();
			_imageSetSubFilesIdx = 146;
			break;
		case Common::KEYCODE_PAGEDOWN:
		case Common::KEYCODE_KP3:
			goSouthEast();
			_imageSetSubFilesIdx = 146;
			break;
		case Common::KEYCODE_END:
		case Common::KEYCODE_KP1:
			goSouthWest();
			_imageSetSubFilesIdx = 147;
			break;
		case Common::KEYCODE_HOME:
		case Common::KEYCODE_KP7:
			goNorthWest();
			_imageSetSubFilesIdx = 147;
			break;
		case Common::KEYCODE_F1:
			if (_teamCharId[0] != -1) {
				handleStatusMenu(1, _teamCharId[0]);
				_tempTextPtr = nullptr;
				drawGameScreenAndTempText(true);
				_redrawNeededFl = true;
			}
			break;
		case Common::KEYCODE_F2:
			if (_teamCharId[1] != -1) {
				handleStatusMenu(1, _teamCharId[1]);
				_tempTextPtr = nullptr;
				drawGameScreenAndTempText(true);
				_redrawNeededFl = true;
			}
			break;
		case Common::KEYCODE_F3:
			if (_teamCharId[2] != -1) {
				handleStatusMenu(1, _teamCharId[2]);
				_tempTextPtr = nullptr;
				drawGameScreenAndTempText(true);
				_redrawNeededFl = true;
			}
			break;
		case Common::KEYCODE_F5: { // Original is using CTRL-S
			for (uint counter = 0; counter < 2; ++counter) {
				clearBottomTextZone(0);
				displayCenteredString("Are You Sure You Want To Save?", 24, 296, 160);
				if (counter == 0)
					displayFctFullScreen();
			}
			Common::KeyCode input = waitForKey();
			if (input == Common::KEYCODE_y) {
				displayMenuAnswerString("-> Yes <-", 24, 296, 169);
				getInput(2);
				saveGameDialog();
			} else {
				displayMenuAnswerString("-> No!!! <-", 24, 296, 169);
				getInput(2);
			}
			clearBottomTextZone_2(0);
			displayLowStatusScreen(true);

			}
			break;
		case Common::KEYCODE_F7: { // Original is using CTRL-L
			for (uint counter = 0; counter < 2; ++counter) {
				clearBottomTextZone(0);
				displayCenteredString("Are You Sure You Want To Load?", 24, 296, 160);
				if (counter == 0)
					displayFctFullScreen();
			}
			Common::KeyCode input = waitForKey();
			if (input == Common::KEYCODE_y) {
				displayMenuAnswerString("-> Yes <-", 24, 296, 169);
				getInput(2);
				loadGameDialog();
			} else {
				displayMenuAnswerString("-> No!!! <-", 24, 296, 169);
				getInput(2);
			}
			clearBottomTextZone_2(0);
			displayLowStatusScreen(true);

		} break;
		// debug cases to test sound
		case Common::KEYCODE_1:
			generateSound(13);
			break;
		case Common::KEYCODE_2:
			generateSound(14);
			break;
		case Common::KEYCODE_3:
			generateSound(15);
			break;
		default:
			if (retVal != Common::KEYCODE_INVALID)
				warning("Main Loop: Unhandled input %d", retVal);
			break;
		}

		if ((_mapPosX != _oldMapPosX || _mapPosY != _oldMapPosY) && !_shouldQuit) {
			bool collisionFl = checkMonsterCollision();
			if (_dbgForceMonsterBlock || collisionFl) {
				_oldMapPosX = _mapPosX;
				_oldMapPosY = _mapPosY;
				_oldImageSetSubFilesIdx = _imageSetSubFilesIdx;
				_redrawNeededFl = true;
			} else {
				_mapPosX = _oldMapPosX;
				_mapPosY = _oldMapPosY;
				if (_oldImageSetSubFilesIdx != _imageSetSubFilesIdx) {
					_redrawNeededFl = true;
					_oldImageSetSubFilesIdx = _imageSetSubFilesIdx;
				}
			}
			if (_largeMapFlag) {
				_techDataId_MapPosX = _mapPosX;
				_techDataId_MapPosY = _mapPosY;
			}
		}

		if (!_shouldQuit) {
			handleMapMonsterMoves();
		}

		if (_redrawNeededFl && !_shouldQuit) {
			drawScreen();
			displayLowStatusScreen(true);
		}

		if (!_shouldQuit) {
			handleNewRoundEffects();

			if (_tempTextDelay > 0) {
				if (--_tempTextDelay == 0) {
					displayMiddleLeftTempText(nullptr, true);
				}
			}
		}

		if (_unk2C8AA > 0)
			--_unk2C8AA;

		if (isTPK()) {
			if (handleDeathMenu())
				_shouldQuit = true;
		}

		displayFctFullScreen();
	}
	return Common::kNoError;
}

void EfhEngine::initialize() {
	_rnd = new Common::RandomSource("Hell");
	_rnd->setSeed(g_system->getMillis());   // Kick random number generator
	_shouldQuit = false;
}

void EfhEngine::songDelay(int delay) {
	debug("songDelay %ld", delay);

	int remainingDelay = delay / 2;
	while (remainingDelay > 0 && !shouldQuit()) {
		remainingDelay -= 3;
		_system->delayMillis(3);
	}
}

void EfhEngine::playNote(int frequencyIndex, int totalDelay) {
	debug("playNote %d %d", frequencyIndex, totalDelay);
	_speakerStream->play(Audio::PCSpeaker::kWaveFormSquare, 0x1234DD / kSoundFrequency [frequencyIndex], -1);
	songDelay(totalDelay);
	_speakerStream->stop();
}

Common::KeyCode EfhEngine::playSong(uint8 *buffer) {
	debug("playSong");

	_speakerStream = new Audio::PCSpeaker(_mixer->getOutputRate());
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_speakerHandle,
							_speakerStream, -1, Audio::Mixer::kMaxChannelVolume, 0, DisposeAfterUse::NO, true);

	Common::KeyCode inputChar = Common::KEYCODE_INVALID;
	int totalDelay = 0;

	int8 stopFl;
	uint8 varC = *buffer++;
	Common::Event event;
	do {
		stopFl = *buffer & 0x3F;
		if (stopFl != 0) {
			int delay = stopFl * varC * 0x2200 / 1000;

			if (*buffer > 0x7F)
				delay /= 2;

			if (*buffer & 0x40)
				delay = (delay * 2) / 3;

			++buffer;
			uint8 frequencyIndex = *buffer;
			++buffer;

			if (frequencyIndex > 0x7F)
				totalDelay += delay;
			else if (frequencyIndex == 0)
				songDelay(delay);
			else {
				playNote(frequencyIndex, totalDelay + delay);
				totalDelay = 0;
			}
		}

		songDelay(10);
		_system->getEventManager()->pollEvent(event);
		if (event.type == Common::EVENT_KEYUP) {
			inputChar = event.kbd.keycode;
			// Hack, sometimes there's a ghost event after the 2nd note
			if (inputChar == Common::KEYCODE_ESCAPE || inputChar == Common::KEYCODE_RETURN)
				stopFl = 0;
		}
	} while (stopFl != 0);
	
	_mixer->stopHandle(_speakerHandle);
	delete _speakerStream;
	_speakerStream = nullptr;

	return inputChar;
}

void EfhEngine::playIntro() {
	debugC(6, kDebugEngine, "playIntro");

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);

	// Load animations on previous picture with GF
	loadImageSet(63, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	readImpFile(100, false);
	Common::KeyCode lastInput = getLastCharAfterAnimCount(8);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// With GF on the bed
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[0], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[0], 6, 150, 268, 186, false);

	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// Poof
	displayRawDataAtPos(_circleImageSubFileArray[1], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[1], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[1], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[1], 6, 150, 268, 186, false);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// On the phone
	displayRawDataAtPos(_circleImageSubFileArray[2], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[2], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[2], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[2], 6, 150, 268, 186, false);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[3], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[3], 6, 150, 268, 186, false);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[4], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[4], 6, 150, 268, 186, false);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[3], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[5], 6, 150, 268, 186, false);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[3], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	drawText(_imp2PtrArray[5], 6, 150, 268, 186, false);
	getLastCharAfterAnimCount(80);
}

void EfhEngine::initEngine() {
	_videoMode = 2; // In the original, 2 = VGA/MCGA, EGA = 4, Tandy = 6, cga = 8.
	_graphicsStruct = new EfhGraphicsStruct;
	_graphicsStruct->copy(_vgaGraphicsStruct1);

	_vgaGraphicsStruct2->copy(_vgaGraphicsStruct1);
	_vgaGraphicsStruct2->_shiftValue = 0x2000;

	_graphicsStruct->copy(_vgaGraphicsStruct2);

	_defaultBoxColor = 7;

	// Init Font
	_fontDescr._widthArray = kFontWidthArray;
	_fontDescr._extraLines = kFontExtraLinesArray;
	_fontDescr._fontData = kFontData;
	_fontDescr._charHeight = 8;
	_fontDescr._extraVerticalSpace = 3;
	_fontDescr._extraHorizontalSpace = 1;
	_introDoneFl = false;

	// Pre-load stuff required for savegames
	preLoadMaps();

	saveAnimImageSetId();

	// Load Title Screen, skip if loading a savegame from launcher
	loadImageSet(11, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	if (_loadSaveSlot == -1) {
		displayFctFullScreen();
		displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
		displayFctFullScreen();
		displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	}

	// Load map tiles bitmaps
	loadImageSetToTileBank(1, 1);
	loadImageSetToTileBank(2, 2);

	// Load characters bitmaps
	loadImageSetToTileBank(3, 6);

	// Load 320*200 Menu screen
	Common::String fileName = Common::String::format("imageset.%d", 10);
	readFileToBuffer(fileName, _menuBuf);

	// Load 96*64 Window with pink border and yellow bottom
	fileName = Common::String::format("imageset.%d", 12);
	readFileToBuffer(fileName, _windowWithBorderBuf);

	readAnimInfo();

	displayAnimFrames(0xFE, false);
	saveAnimImageSetId();
	readTileFact();
	readItems();
	loadNPCS();

	// Load picture room with girlfriend
	loadImageSet(62, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	fileName = "titlsong";
	readFileToBuffer(fileName, _titleSong);
	setDefaultNoteDuration();
	Common::KeyCode lastInput = playSong(_titleSong);

	if (lastInput != Common::KEYCODE_ESCAPE && _loadSaveSlot == -1) {
		playIntro();
	}

	loadImageSet(6, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	readImpFile(99, false);
	_introDoneFl = true;
	restoreAnimImageSetId();

	// Note: The original at this point saves int 24h and sets a new int24 to handle fatal failure

	checkProtection();
	if (_loadSaveSlot == -1) {
		loadEfhGame();
		resetGame();
	} else {
		loadGameState(_loadSaveSlot);
		_loadSaveSlot = -1;
	}

	_saveAuthorized = true;
	_engineInitPending = false;
}

void EfhEngine::initMapMonsters() {
	debugC(3, kDebugEngine, "initMapMonsters");

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (_mapMonsters[_techId][monsterId]._fullPlaceId == 0xFF)
			continue;

		for (uint counter = 0; counter < 9; ++counter)
			_mapMonsters[_techId][monsterId]._hitPoints[counter] = 0;

		uint8 groupSize = _mapMonsters[_techId][monsterId]._groupSize;
		if (groupSize == 0)
			groupSize = getRandom(10) - 1;

		if (groupSize == 0)
			continue;
		
		for (uint counter = 0; counter < groupSize; ++counter) {
			uint rand100 = getRandom(100);
			uint16 pictureRef = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._pictureRef;

			if (rand100 <= 25) {
				uint16 delta = getRandom(pictureRef / 2);
				_mapMonsters[_techId][monsterId]._hitPoints[counter] = pictureRef - delta;
			} else if (rand100 <= 75) {
				_mapMonsters[_techId][monsterId]._hitPoints[counter] = pictureRef;
			} else {
				uint16 delta = getRandom(pictureRef / 2);
				_mapMonsters[_techId][monsterId]._hitPoints[counter] = pictureRef + delta;
			}
		}
	}
}

void EfhEngine::loadMapArrays(int idx) {
	// No longer required as everything is in memory. 
}

void EfhEngine::saveAnimImageSetId() {
	debugC(6, kDebugEngine, "saveAnimImageSetId");

	_oldAnimImageSetId = _animImageSetId;
	_animImageSetId = 0xFF;
}

int16 EfhEngine::getEquipmentDefense(int16 charId, bool flag) {
	debugC(2, kDebugGraphics, "getEquipmentDefense %d %s", charId, flag ? "True" : "False");
	// TODO: flag is always false, remove it when refactoring

	int16 altDef = 0;
	int16 totalDef = 0;
	for (int i = 0; i < 10; ++i) {
		if (_npcBuf[charId]._inventory[i]._ref == 0x7FFF)
			continue;

		if (!_npcBuf[charId]._inventory[i].isEquipped())
			continue;

		int16 curDef = _npcBuf[charId]._inventory[i]._curHitPoints;
		if (curDef == 0xFF)
			curDef = _items[_npcBuf[charId]._inventory[i]._ref]._defense;

		if (curDef <= 0)
			continue;

		totalDef += curDef;
		altDef += (curDef / 8) + 1;
	}

	if (flag)
		return totalDef;

	return altDef;
}

uint16 EfhEngine::getEquippedExclusiveType(int16 charId, int16 exclusiveType, bool flag) {
	debugC(2, kDebugEngine, "getEquippedExclusiveType %d %d %s", charId, exclusiveType, flag ? "True" : "False");

	for (int i = 0; i < 10; ++i) {
		if (!_npcBuf[charId]._inventory[i].isEquipped())
			continue;

		int16 curItemId = _npcBuf[charId]._inventory[i]._ref;

		if (_items[curItemId]._exclusiveType != exclusiveType)
			continue;

		// If flag is set, returns the ItemId, otherwise return the inventory slot number
		if (!flag)
			return i;

		return curItemId;
	}

	return 0x7FFF;
}

void EfhEngine::drawGameScreenAndTempText(bool flag) {
	debugC(2, kDebugEngine, "drawGameScreenAndTempText %s", flag ? "True" : "False");

#if 0
	// This code is present in the original, but looks strictly useless.
	uint8 mapTileInfo = getMapTileInfo(_mapPosX, _mapPosY);
	int16 imageSetId = _currentTileBankImageSetId[mapTileInfo / 72];

	int16 mapImageSetId = (imageSetId * 72) + (mapTileInfo % 72);
#endif

	for (int counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			displayGameScreen();
			// Redraw temp text if one is displayed currently
			// _tempTextDelay determines whether a temp text is displayed in the middle-left zone
			if (_tempTextDelay != 0) {
				// Note: the original was doing the check in the opposite order, which looks really suspicious
				if ((_tempTextPtr != nullptr) && (_tempTextPtr[0] != 0x30)) {
					displayMiddleLeftTempText(_tempTextPtr, false);
				}
			}
		}

		if (counter == 0 && flag)
			displayFctFullScreen();
	}
}

void EfhEngine::drawMap(bool largeMapFl, int16 mapPosX, int16 mapPosY, int16 mapSize, bool drawHeroFl, bool drawMonstersFl) {
	debugC(6, kDebugEngine, "drawMap %s %d-%d %d %s %s", largeMapFl ? "True" : "False", mapPosX, mapPosY, mapSize, drawHeroFl ? "True" : "False", drawMonstersFl ? "True" : "False");

	int16 shiftPosX = 5;
	int16 shiftPosY = 4;
	int16 minX = mapPosX - 5;
	int16 minY = mapPosY - 4;

	if (minX < 0) {
		shiftPosX += minX;
		minX = 0;
	}

	if (minY < 0) {
		shiftPosY += minY;
		minY = 0;
	}

	int16 maxX = minX + 10;
	int16 maxY = minY + 7;

	if (maxX > mapSize) {
		shiftPosX += (maxX - mapSize);
		maxX = mapSize;
		minX = mapSize - 10;
	}

	if (maxY > mapSize) {
		shiftPosY += (maxY - mapSize);
		maxY = mapSize;
		minY = mapSize - 7;
	}

	int16 drawPosY = 8;
	for (int16 counterY = minY; counterY <= maxY; ++counterY) {
		int16 drawPosX = 128;
		for (int16 counterX = minX; counterX <= maxX; ++counterX) {
			if (largeMapFl) {
				int16 curTile = _mapGameMaps[_techId][counterX][counterY];
				displayRawDataAtPos(_imageSetSubFilesArray[curTile], drawPosX, drawPosY);
			} else {
				int16 curTile = _curPlace[counterX][counterY];
				displayRawDataAtPos(_imageSetSubFilesArray[curTile], drawPosX, drawPosY);
			}
			drawPosX += 16;
		}
		drawPosY += 16;
	}

	if (drawHeroFl) {
		// Draw hero
		int16 drawPosX = 128 + shiftPosX * 16;
		drawPosY = 8 + shiftPosY * 16;
		displayRawDataAtPos(_imageSetSubFilesArray[_imageSetSubFilesIdx], drawPosX, drawPosY);
	}

	if (drawMonstersFl) {
		for (uint var16 = 0; var16 < 64; ++var16) {
			if ((_largeMapFlag && _mapMonsters[_techId][var16]._fullPlaceId == 0xFE) || (!_largeMapFlag && _mapMonsters[_techId][var16]._fullPlaceId == _fullPlaceId)){
				bool var4 = false;
				int16 posX = _mapMonsters[_techId][var16]._posX;
				int16 posY = _mapMonsters[_techId][var16]._posY;

				if (posX < minX || posX > maxX || posY < minY || posY > maxY)
					continue;

				for (uint counterY = 0; counterY < 9 && !var4; ++counterY) {
					if (_mapMonsters[_techId][var16]._hitPoints[counterY] > 0)
						var4 = true;
				}

				if (!var4)
					continue;

				int16 var6 = 148 + kEncounters[_mapMonsters[_techId][var16]._monsterRef]._animId;
				int16 var1 = _mapMonsters[_techId][var16]._possessivePronounSHL6 & 0x3F;

				if (var1 == 0x3F && isNpcATeamMember(_mapMonsters[_techId][var16]._npcId))
					continue;

				int16 drawPosX = 128 + (posX - minX) * 16;
				drawPosY = 8 + (posY - minY) * 16;
				displayRawDataAtPos(_imageSetSubFilesArray[var6], drawPosX, drawPosY);
			}
		}
	}
}

void EfhEngine::displaySmallMap(int16 posX, int16 posY) {
	debugC(6, kDebugEngine, "displaySmallMap %d %d", posX, posY);
	drawMap(false, posX, posY, 23, _drawHeroOnMapFl, _drawMonstersOnMapFl);
}

void EfhEngine::displayLargeMap(int16 posX, int16 posY) {
	debugC(6, kDebugEngine, "displayLargeMap %d %d", posX, posY);
	drawMap(true, posX, posY, 63, _drawHeroOnMapFl, _drawMonstersOnMapFl);
}

void EfhEngine::drawScreen() {
	debugC(2, kDebugEngine, "drawScreen");

	for (uint counter = 0; counter < 2; ++counter) {
		_redrawNeededFl = false;
		if (!_largeMapFlag) {
			if (_fullPlaceId != 0xFF)
				displaySmallMap(_mapPosX, _mapPosY);
		} else {
			if (_techId != 0xFF)
				displayLargeMap(_mapPosX, _mapPosY);
		}
		if (counter == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::displayLowStatusScreen(bool flag) {
	debugC(6, kDebugEngine, "displayLowStatusScreen %s", flag ? "True" : "False");

	for (int counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			clearBottomTextZone(0);
			setTextColorWhite();
			displayCenteredString("Name", 16, 88, 152);
			displayCenteredString("DEF", 104, 128, 152);
			displayCenteredString("HP", 144, 176, 152);
			displayCenteredString("Max HP", 192, 224, 152);
			displayCenteredString("Weapon", 225, 302, 152);
			setTextColorRed();

			for (int i = 0; i < 3; ++i) {
				if (_teamCharId[i] == -1)
					continue;
				int16 charId = _teamCharId[i];
				int16 textPosY = 161 + 9 * i;
				Common::String buffer = _npcBuf[charId]._name;
				setTextPos(16, textPosY);
				displayStringAtTextPos(buffer);
				buffer = Common::String::format("%d", getEquipmentDefense(charId, false));
				displayCenteredString(buffer, 104, 128, textPosY);
				buffer = Common::String::format("%d", _npcBuf[charId]._hitPoints);
				displayCenteredString(buffer, 144, 176, textPosY);
				buffer = Common::String::format("%d", _npcBuf[charId]._maxHP);
				displayCenteredString(buffer, 192, 224, textPosY);

				if (_npcBuf[charId]._hitPoints <= 0) {
					displayCenteredString("* DEAD *", 225, 302, textPosY);
					continue;
				}

				switch (_teamCharStatus[i]._status) {
				case 0: {
					uint16 exclusiveItemId = getEquippedExclusiveType(charId, 9, true);
					if (exclusiveItemId == 0x7FFF)
						_nameBuffer = "(NONE)";
					else
						_nameBuffer = _items[exclusiveItemId]._name;
					}
					break;
				case 1:
					_nameBuffer = "* ASLEEP *";
					break;
				case 2:
					_nameBuffer = "* FROZEN *";
					break;
				default:
					_nameBuffer = "* DISABLED *";
					break;
				}

				displayCenteredString(_nameBuffer, 225, 302, textPosY);
			}
		}

		if (counter == 0 && flag)
			displayFctFullScreen();
	}
}


void EfhEngine::removeObject(int16 charId, int16 objectId) {
	debugC(6, kDebugEngine, "removeObject %d %d", charId, objectId);
	_npcBuf[charId]._inventory[objectId]._ref = 0x7FFF;
	_npcBuf[charId]._inventory[objectId]._stat1 = 0;
	_npcBuf[charId]._inventory[objectId]._curHitPoints = 0;
}

void EfhEngine::totalPartyKill() {
	debugC(6, kDebugEngine, "totalPartyKill");

	for (uint counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1)
			_npcBuf[counter]._hitPoints = 0;
	}
}

void EfhEngine::removeCharacterFromTeam(int16 teamMemberId) {
	debugC(6, kDebugEngine, "removeCharacterFromTeam %d", teamMemberId);

	int16 charId = _teamCharId[teamMemberId];
	_npcBuf[charId].field12_textId = _npcBuf[charId].fieldB_textId;
	_npcBuf[charId].field14_textId = _npcBuf[charId].fieldE_textId;
	_npcBuf[charId].field_10 = _npcBuf[charId].field_C;
	_npcBuf[charId].field11_NpcId = _npcBuf[charId].field_D;

	_teamCharId[teamMemberId] = -1;
	_teamCharStatus[teamMemberId]._status = 0;
	_teamCharStatus[teamMemberId]._duration = 0;

	for (int var4 = teamMemberId; var4 < 2; ++var4) {
		_teamCharId[var4] = _teamCharId[var4 + 1];
		_teamCharId[var4 + 1] = -1;
	}

	refreshTeamSize();
}

void EfhEngine::refreshTeamSize() {
	debugC(6, kDebugEngine, "refreshTeamSize");

	_teamSize = 0;
	for (uint charId = 0; charId < 3; ++charId) {
		if (_teamCharId[charId] != -1)
			++_teamSize;
	}
}

bool EfhEngine::isNpcATeamMember(int16 id) {
	debugC(6, kDebugEngine,"isNpcATeamMember %d", id);

	for (int charId = 0; charId < _teamSize; ++charId) {
		if (_teamCharId[charId] == id)
			return true;
	}

	return false;
}

void EfhEngine::handleWinSequence() {
	debugC(1, kDebugEngine, "handleWinSequence");
	_saveAuthorized = false;

	saveAnimImageSetId();
	findMapFile(18);
	// clearMemory();
	uint8 *decompBuffer = (uint8 *)malloc(41000);
	uint8 *winSeqBuf3 = (uint8 *)malloc(40100);
	uint8 *winSeqBuf4 = (uint8 *)malloc(40100);

	uint8 *winSeqSubFilesArray1[10];
	uint8 *winSeqSubFilesArray2[20];
	loadImageSet(64, winSeqBuf3, winSeqSubFilesArray1, decompBuffer);
	loadImageSet(65, winSeqBuf4, winSeqSubFilesArray2, decompBuffer);

	for (uint counter = 0; counter < 2; ++counter) {
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		displayRawDataAtPos(winSeqSubFilesArray2[0], 136, 48);
		if (counter == 0)
			displayFctFullScreen();
	}

	getInput(12);
	for (uint animId = 1; animId < 8; ++animId) {
		for (uint counter = 0; counter < 2; ++counter) {
			displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
			displayRawDataAtPos(winSeqSubFilesArray2[animId], 136, 48);
			if (counter == 0)
				displayFctFullScreen();
		}
		getInput(1);
	}

	Common::KeyCode input = Common::KEYCODE_INVALID;

	while (input != Common::KEYCODE_ESCAPE) {
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		displayFctFullScreen();
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		input = getInput(32);
		if (input != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[10], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[10], 136, 72);
			input = getInput(1);
		}

		if (input != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[11], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[11], 136, 72);
			input = getInput(1);
		}

		if (input != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[12], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[12], 136, 72);
			input = getInput(1);
		}

		if (input != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[13], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[13], 136, 72);
			input = getInput(1);
		}

		if (input != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[14], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[14], 136, 72);
			input = getInput(1);
		}
	}

	free(decompBuffer);
	free(winSeqBuf3);
	free(winSeqBuf4);
}

bool EfhEngine::giveItemTo(int16 charId, int16 objectId, int16 fromCharId) {
	debugC(3, kDebugEngine, "giveItemTo %d %d %d", charId, objectId, fromCharId);

	for (uint newObjectId = 0; newObjectId < 10; ++newObjectId) {
		if (_npcBuf[charId]._inventory[newObjectId]._ref != 0x7FFF)
			continue;

		if (fromCharId == 0xFF) {
			_npcBuf[charId]._inventory[newObjectId]._ref = objectId;
			_npcBuf[charId]._inventory[newObjectId]._curHitPoints = _items[objectId]._defense;
			_npcBuf[charId]._inventory[newObjectId]._stat1 = _items[objectId]._uses;
		} else {
			_npcBuf[charId]._inventory[newObjectId]._ref = _npcBuf[fromCharId]._inventory[objectId]._ref;
			_npcBuf[charId]._inventory[newObjectId]._curHitPoints = _npcBuf[fromCharId]._inventory[objectId]._curHitPoints;
			_npcBuf[charId]._inventory[newObjectId]._stat1 = _npcBuf[fromCharId]._inventory[objectId].getUsesLeft(); // not equipped as the upper bit isn't set (0x80)
		}

		return true;
	}

	return false;
}

int16 EfhEngine::chooseCharacterToReplace() {
	debugC(3, kDebugEngine, "chooseCharacterToReplace");

	Common::KeyCode maxVal = (Common::KeyCode)(Common::KEYCODE_0 + _teamSize);
	Common::KeyCode input;
	for (;;) {
		input = waitForKey();
		if (input == Common::KEYCODE_ESCAPE || input == Common::KEYCODE_0 || (input > Common::KEYCODE_1 && input < maxVal))
			break;
	}

	if (input == Common::KEYCODE_ESCAPE || input == Common::KEYCODE_0)
		return 0x1B;

	return (int16)input - (int16)Common::KEYCODE_1;
}

int16 EfhEngine::handleCharacterJoining() {
	debugC(3, kDebugEngine, "handleCharacterJoining");

	for (uint counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] == -1) {
			return counter;
		}
	}

	for (uint counter = 0; counter < 2; ++counter) {
		drawColoredRect(200, 112, 278, 132, 0);
		displayCenteredString("Replace Who?", 200, 278, 117);
		if (counter == 0)
			displayFctFullScreen();
	}

	int16 charId = chooseCharacterToReplace();
	for (uint counter = 0; counter < 2; ++counter) {
		drawColoredRect(200, 112, 278, 132, 0);
		if (counter == 0)
			displayFctFullScreen();
	}

	if (charId == 0x1B) // Escape Keycode
		return -1;

	removeCharacterFromTeam(charId);
	return 2;
}

void EfhEngine::drawText(uint8 *srcPtr, int16 posX, int16 posY, int16 maxX, int16 maxY, bool flag) {
	debugC(7, kDebugEngine, "drawText %d-%d %d-%d %s", posX, posY, maxX, maxY, flag ? "True" : "False");

	uint16 stringIdx = 0;
	uint8 *impPtr = srcPtr;
	_messageToBePrinted = "";

	for (;;) {
		uint8 curChar = *impPtr;

		if (curChar == 0 || curChar == 0x40 || curChar == 0x60)
			break;

		if (curChar == 0x0D) {
			_messageToBePrinted += " ";
			stringIdx++;
			++impPtr;
		} else if (curChar == 0x0A) {
			++impPtr;
		} else {
			_messageToBePrinted += curChar;
			stringIdx++;
			++impPtr;
		}
	}

	script_parse(_messageToBePrinted, posX, posY, maxX, maxY, flag);
}

void EfhEngine::displayMiddleLeftTempText(uint8 *impArray, bool flag) {
	debugC(3, kDebugEngine, "displayMiddleLeftTempText %s %s", (char *)impArray, flag ? "True" : "False");

	for (uint counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			// clear middle-left text area
			drawColoredRect(16, 115, 111, 133, 0);
			if (impArray != nullptr) {
				_tempTextDelay = 4;
				_tempTextPtr = impArray;
				drawText(impArray, 17, 115, 110, 133, false);
			}
			if (counter == 0 && flag)
				displayFctFullScreen();
		}
	}
}

void EfhEngine::transitionMap(int16 centerX, int16 centerY) {
	debugC(2, kDebugEngine, "transitionMap %d %d", centerX, centerY);

	_drawHeroOnMapFl = false;
	int16 minX = centerX - 11;
	int16 minY = centerY - 11;

	if (minX < 0)
		minX = 0;
	if (minY < 0)
		minY = 0;

	for (uint counterX = 0; counterX <= 23; counterX += 2) {
		for (uint counterY = 0; counterY <= 23; ++counterY) {
			int16 curX = counterX + minX;
			int16 curY = counterY + minY;

			if (curX < 64 && curY < 64)
				_mapGameMaps[_techId][curX][curY] = _curPlace[counterX][counterY];
		}
		drawScreen();
	}

	for (uint counterX = 1; counterX <= 23; counterX += 2) {
		for (uint counterY = 0; counterY <= 23; ++counterY) {
			int16 curX = counterX + minX;
			int16 curY = counterY + minY;

			if (curX < 64 && curY < 64)
				_mapGameMaps[_techId][curX][curY] = _curPlace[counterX][counterY];
		}
		drawScreen();
	}

	getLastCharAfterAnimCount(3);
	_drawHeroOnMapFl = true;
}

void EfhEngine::setSpecialTechZone(int16 unkId, int16 centerX, int16 centerY) {
	debugC(2, kDebugEngine, "setSpecialTechZone %d %d %d", unkId, centerX, centerY);

	if (unkId < 0 || unkId >= 60)
		error("setSpecialTechZone - unexpected value for unkId: %d", unkId);

	uint8 zoneValue = kByte2C7D0[unkId];

	// Added a CLIP as a safeguard to avoid any value larger than 64
	int16 minX = CLIP(centerX - 11, 0, 64);
	int16 minY = CLIP(centerY - 11, 0, 64);


	int16 maxX = CLIP(minX + 23, 0, 64);
	int16 maxY = CLIP(minY + 23,0, 64);

	for (int16 counterX = minX; counterX <= maxX; ++counterX) {
		for (int16 counterY = minY; counterY <= maxY; ++counterY) {
			_techDataArr[_techId][counterY + counterX * 64] = zoneValue;
		}
	}
}

int16 EfhEngine::findMapSpecialTileIndex(int16 posX, int16 posY) {
	debugC(5, kDebugEngine, "findMapSpecialTileIndex %d %d", posX, posY);

	uint16 searchPlaceId = _largeMapFlag ? 0xFE : _fullPlaceId;
	
	for (uint counter = 0; counter < 100; ++counter) {
		if (_mapSpecialTiles[_techId][counter]._posX == posX && _mapSpecialTiles[_techId][counter]._posY == posY && _mapSpecialTiles[_techId][counter]._placeId == searchPlaceId)
			return counter;
	}

	return -1;
}

bool EfhEngine::isPosOutOfMap(int16 mapPosX, int16 mapPosY) {
	debugC(6, kDebugEngine, "isPosOutOfMap %d %d", mapPosX, mapPosY);

	int16 maxMapBlocks = _largeMapFlag ? 63 : 23;

	if (mapPosX == 0 && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	if (mapPosX == maxMapBlocks && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	return false;
}

void EfhEngine::goSouth() {
	debugC(6,kDebugEngine, "goSouth");

	if (_largeMapFlag) {
		if (++_mapPosY > 63)
			_mapPosY = 63;
	} else {
		if (++_mapPosY > 23)
			_mapPosY = 23;
	}

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goNorth() {
	debugC(6,kDebugEngine, "goNorth");

	if (--_mapPosY < 0)
		_mapPosY = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goEast() {
	debugC(6, kDebugEngine, "goEast");

	if (_largeMapFlag) {
		if (++_mapPosX > 63)
			_mapPosX = 63;
	} else {
		if (++_mapPosX > 23)
			_mapPosX = 23;
	}

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goWest() {
	debugC(6, kDebugEngine, "goWest");

	if (--_mapPosX < 0)
		_mapPosX = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goNorthEast() {
	debugC(6, kDebugEngine, "goNorthEast");

	if (--_mapPosY < 0)
		_mapPosY = 0;

	if (_largeMapFlag) {
		if (++_mapPosX > 63)
			_mapPosX = 63;
	} else {
		if (++_mapPosX > 23)
			_mapPosX = 23;
	}

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goSouthEast() {
	debugC(6, kDebugEngine, "goSouthEast");

	if (_largeMapFlag) {
		if (++_mapPosX > 63)
			_mapPosX = 63;
	} else if (++_mapPosX > 23)
		_mapPosX = 23;

	if (_largeMapFlag) {
		if (++_mapPosY > 63)
			_mapPosY = 63;
	} else if (++_mapPosY > 23)
		_mapPosY = 23;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goNorthWest() {
	debugC(6, kDebugEngine,"goNorthWest");

	if (--_mapPosY < 0)
		_mapPosY = 0;

	if (--_mapPosX < 0)
		_mapPosX = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goSouthWest() {
	debugC(6, kDebugEngine, "goSouthWest");

	if (--_mapPosX < 0)
		_mapPosX = 0;

	if (_largeMapFlag) {
		if (++_mapPosY > 63)
			_mapPosY = 63;
	} else if (++_mapPosY > 23)
		_mapPosY = 23;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::handleNewRoundEffects() {
	debugC(6, kDebugEngine, "handleNewRoundEffects");

	for (int counter = 0; counter < _teamSize; ++counter) {
		CharStatus *curStatus = &_teamCharStatus[counter];
		if (curStatus->_status == 0) // normal
			continue;

		if (--curStatus->_duration <= 0) {
			curStatus->_status = 0;
			curStatus->_duration = 0;
		}
	}

	if (++_regenCounter <= 8)
		return;

	for (int counter = 0; counter < _teamSize; ++counter) {
		NPCStruct *curNpc = &_npcBuf[_teamCharId[counter]];
		if (curNpc->_hitPoints < curNpc->_maxHP)
			++curNpc->_hitPoints = curNpc->_maxHP;
	}
	_regenCounter = 0;
}

void EfhEngine::resetGame() {
	loadTechMapImp(0);
	_largeMapFlag = true;
	_oldMapPosX = _mapPosX = 31;
	_oldMapPosY = _mapPosY = 31;
	_unkRelatedToAnimImageSetId = 0;
	_unk2C8AA = 0;
}

void EfhEngine::computeMapAnimation() {
	debugC(6, kDebugEngine, "computeMapAnimation");

	const int16 maxMapBlocks = _largeMapFlag ? 63 : 23;

	int16 minMapX = CLIP<int16>(_mapPosX - 5, 0, maxMapBlocks);
	int16 minMapY = CLIP<int16>(_mapPosY - 4, 0, maxMapBlocks);
	int16 maxMapX = CLIP<int16>(minMapX + 10, 0, maxMapBlocks);
	int16 maxMapY = CLIP<int16>(minMapY + 7, 0, maxMapBlocks);

	for (int16 counterY = minMapY; counterY < maxMapY; ++counterY) {
		for (int16 counterX = minMapX; counterX < maxMapX; ++counterX) {
			if (_currentTileBankImageSetId[0] != 0)
				continue;

			if (_largeMapFlag) {
				uint8 curTile = _mapGameMaps[_techId][counterX][counterY];
				if (curTile >= 1 && curTile <= 0xF) {
					if (getRandom(100) < 50)
						_mapGameMaps[_techId][counterX][counterY] += 0xC5;
				} else if (curTile >= 0xC6 && curTile <= 0xD5) {
					if (getRandom(100) < 50)
						_mapGameMaps[_techId][counterX][counterY] -= 0xC5;
				}
			} else {
				uint8 curTile = _curPlace[counterX][counterY];
				if (curTile >= 1 && curTile <= 0xF) {
					if (getRandom(100) < 50)
						_curPlace[counterX][counterY] += 0xC5;
				} else if (curTile >= 0xC6 && curTile <= 0xD5) {
					if (getRandom(100) < 50)
						_curPlace[counterX][counterY] -= 0xC5;
				}
			}
		}
	}
}

void EfhEngine::handleAnimations() {
	setNumLock();

	if (_engineInitPending)
		return;

	debugC(6, kDebugEngine, "handleAnimations");

	if (_animImageSetId != 0xFF) {
		displayNextAnimFrame();
		displayFctFullScreen();
		displayAnimFrame();
	}

	computeMapAnimation();
}

int8 EfhEngine::checkMonsterMoveCollisionAndTileTexture(int16 monsterId) {
	debugC(3, kDebugEngine,"checkMonsterMoveCollisionAndTileTexture %d", monsterId);

	int16 maxSize = _largeMapFlag ? 63 : 23;
	if (_mapMonsters[_techId][monsterId]._posX < 0 || _mapMonsters[_techId][monsterId]._posY < 0 || _mapMonsters[_techId][monsterId]._posX > maxSize || _mapMonsters[_techId][monsterId]._posY > maxSize)
		return 0;

	if (_mapMonsters[_techId][monsterId]._posX == _mapPosX && _mapMonsters[_techId][monsterId]._posY == _mapPosY)
		return 0;

	for (int counter = 0; counter < 64; ++counter) {
		if (counter == monsterId)
			continue;

		if (!checkMapMonsterAvailability(counter))
			continue;

		if (_mapMonsters[_techId][monsterId]._fullPlaceId == _mapMonsters[_techId][counter]._fullPlaceId
		 && _mapMonsters[_techId][monsterId]._posX == _mapMonsters[_techId][counter]._posX
		 && _mapMonsters[_techId][monsterId]._posY == _mapMonsters[_techId][counter]._posY)
			return 0;
	}

	return checkTileStatus(_mapMonsters[_techId][monsterId]._posX, _mapMonsters[_techId][monsterId]._posY, false);
}

bool EfhEngine::moveMonsterAwayFromTeam(int16 monsterId) {
	debugC(6, kDebugEngine, "moveMonsterAwayFromTeam %d", monsterId);

	if (_mapMonsters[_techId][monsterId]._posX < _mapPosX) {
		--_mapMonsters[_techId][monsterId]._posX;
		if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
			--_mapMonsters[_techId][monsterId]._posY;
		else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
			++_mapMonsters[_techId][monsterId]._posY;

		return true;
	}

	if (_mapMonsters[_techId][monsterId]._posX > _mapPosX) {
		++_mapMonsters[_techId][monsterId]._posX;
		if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
			--_mapMonsters[_techId][monsterId]._posY;
		else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
			++_mapMonsters[_techId][monsterId]._posY;

		return true;
	}

	// Original checks for posX equality, which is the only possible option at this point => skipped
	if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
		--_mapMonsters[_techId][monsterId]._posY;
	else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
		++_mapMonsters[_techId][monsterId]._posY;
	else
		return false;

	return true;
}

bool EfhEngine::moveMonsterTowardsTeam(int16 monsterId) {
	debugC(6, kDebugEngine, "moveMonsterTowardsTeam %d", monsterId);

	if (_mapMonsters[_techId][monsterId]._posX < _mapPosX) {
		++_mapMonsters[_techId][monsterId]._posX;
		if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
			++_mapMonsters[_techId][monsterId]._posY;
		else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
			--_mapMonsters[_techId][monsterId]._posY;

		return true;
	}

	if (_mapMonsters[_techId][monsterId]._posX > _mapPosX) {
		--_mapMonsters[_techId][monsterId]._posX;
		if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
			++_mapMonsters[_techId][monsterId]._posY;
		else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
			--_mapMonsters[_techId][monsterId]._posY;

		return true;
	}

	// Original checks for posX equality, which is the only possible option at this point => skipped
	if (_mapMonsters[_techId][monsterId]._posY < _mapPosY)
		++_mapMonsters[_techId][monsterId]._posY;
	else if (_mapMonsters[_techId][monsterId]._posY > _mapPosY)
		--_mapMonsters[_techId][monsterId]._posY;
	else
		return false;

	return true;
}

bool EfhEngine::moveMonsterGroupOther(int16 monsterId, int16 direction) {
	debugC(6, kDebugEngine, "moveMonsterGroupOther %d %d", monsterId, direction);

	bool retVal;

	switch (direction - 1) {
	case 0:
		--_mapMonsters[_techId][monsterId]._posY;
		retVal = true;
		break;
	case 1:
		--_mapMonsters[_techId][monsterId]._posY;
		++_mapMonsters[_techId][monsterId]._posX;
		retVal = true;
		break;
	case 2:
		++_mapMonsters[_techId][monsterId]._posX;
		retVal = true;
		break;
	case 3:
		++_mapMonsters[_techId][monsterId]._posX;
		++_mapMonsters[_techId][monsterId]._posY;
		retVal = true;
		break;
	case 4:
		++_mapMonsters[_techId][monsterId]._posY;
		retVal = true;
		break;
	case 5:
		++_mapMonsters[_techId][monsterId]._posY;
		--_mapMonsters[_techId][monsterId]._posX;
		retVal = true;
		break;
	case 6:
		--_mapMonsters[_techId][monsterId]._posX;
		retVal = true;
		break;
	case 7:
		--_mapMonsters[_techId][monsterId]._posX;
		--_mapMonsters[_techId][monsterId]._posY;
		retVal = true;
		break;
	default:
		retVal = false;
		break;
	}

	return retVal;
}

bool EfhEngine::moveMonsterGroupRandom(int16 monsterId) {
	debugC(2, kDebugEngine, "moveMonsterGroupRandom %d", monsterId);

	int16 rand100 = getRandom(100);

	if (rand100 < 30)
		return moveMonsterTowardsTeam(monsterId);

	if (rand100 >= 60)
		// CHECKME: the original seems to only use 1 param??
		return moveMonsterGroupOther(monsterId, getRandom(8));

	return moveMonsterAwayFromTeam(monsterId);
}

int16 EfhEngine::computeMonsterGroupDistance(int16 monsterId) {
	debugC(2, kDebugEngine, "computeMonsterGroupDistance %d", monsterId);

	int16 monsterPosX = _mapMonsters[_techId][monsterId]._posX;
	int16 monsterPosY = _mapMonsters[_techId][monsterId]._posY;

	int16 deltaX = monsterPosX - _mapPosX;
	int16 deltaY = monsterPosY - _mapPosY;

	return (int16)sqrt(deltaX * deltaX + deltaY * deltaY);
}

bool EfhEngine::checkWeaponRange(int16 monsterId, int16 weaponId) {
	debugC(6, kDebugEngine, "checkWeaponRange %d %d", monsterId, weaponId);

	static const int16 kRange[5] = {1, 2, 3, 3, 3};

	assert(_items[weaponId]._range < 5);
	if (computeMonsterGroupDistance(monsterId) > kRange[_items[weaponId]._range])
		return false;

	return true;
}

bool EfhEngine::checkMonsterMovementType(int16 id, bool teamFlag) {
	debugC(6, kDebugEngine, "checkMonsterMovementType %d %s", id, teamFlag ? "True" : "False");

	int16 monsterId = id;
	if (teamFlag)
		monsterId = _teamMonsterIdArray[id];

	if ((_mapMonsters[_techId][monsterId]._additionalInfo & 0xF) >= 8)
		return true;

	if (_unk2C8AA != 0 && (_mapMonsters[_techId][monsterId]._additionalInfo & 0x80) != 0)
		return true;

	return false;
}

bool EfhEngine::checkTeamWeaponRange(int16 monsterId) {
	debugC(6, kDebugEngine, "checkTeamWeaponRange %d", monsterId);

	if (!_ongoingFightFl)
		return true;

	for (uint counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId && checkMonsterMovementType(monsterId, false) && checkWeaponRange(monsterId, _mapMonsters[_techId][monsterId]._weaponItemId))
			return false;
	}

	return true;
}

bool EfhEngine::checkIfMonsterOnSameLargeMapPlace(int16 monsterId) {
	debugC(6, kDebugEngine, "checkIfMonsterOnSameLargeMapPlace %d", monsterId);

	if (_largeMapFlag && _mapMonsters[_techId][monsterId]._fullPlaceId == 0xFE)
		return true;

	if (!_largeMapFlag && _mapMonsters[_techId][monsterId]._fullPlaceId == _fullPlaceId)
		return true;

	return false;
}

bool EfhEngine::checkMonsterWeaponRange(int16 monsterId) {
	debugC(6, kDebugEngine, "checkMonsterWeaponRange %d", monsterId);

	return checkWeaponRange(monsterId, _mapMonsters[_techId][monsterId]._weaponItemId);
}

void EfhEngine::handleMapMonsterMoves() {
	debugC(3, kDebugEngine, "handleMapMonsterMoves");

	_redrawNeededFl = true;
	int16 attackMonsterId = -1;
	int16 mapSize = _largeMapFlag ? 63 : 23;
	int16 minDisplayedMapX = CLIP<int16>(_mapPosX - 10, 0, mapSize);
	int16 minDisplayedMapY = CLIP<int16>(_mapPosY - 9, 0, mapSize);
	int16 maxDisplayedMapX = CLIP<int16>(minDisplayedMapX + 20, 0, mapSize);
	int16 maxDisplayedMapY = CLIP<int16>(minDisplayedMapY + 17, 0, mapSize);

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkMapMonsterAvailability(monsterId))
			continue;

		if (!checkTeamWeaponRange(monsterId))
			continue;

		if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
			continue;

		int16 previousPosX = _mapMonsters[_techId][monsterId]._posX;
		int16 previousPosY = _mapMonsters[_techId][monsterId]._posY;

		if (previousPosX < minDisplayedMapX || previousPosX > maxDisplayedMapX || previousPosY < minDisplayedMapY || previousPosY > maxDisplayedMapY)
			continue;

		bool monsterMovedFl = false;
		int16 lastRangeCheck = 0;

		int8 monsterMoveType = _mapMonsters[_techId][monsterId]._additionalInfo & 0xF; // 0000 1111

		if (_unk2C8AA != 0 && (_mapMonsters[_techId][monsterId]._additionalInfo & 0x80)) // 1000 0000
			monsterMoveType = 9;

		int16 randomModPct = _mapMonsters[_techId][monsterId]._additionalInfo & 0x70; // 0111 0000
		randomModPct >>= 4; // Max 7 (0111)

		int16 retryCounter = randomModPct;
		do {
			switch (monsterMoveType - 1) {
			case 0:
				if (getRandom(100) >= 14 - randomModPct)
					monsterMovedFl = moveMonsterTowardsTeam(monsterId);
				else
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 1:
				if (getRandom(100) >= 14 - randomModPct)
					monsterMovedFl = moveMonsterAwayFromTeam(monsterId);
				else
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 2:
				monsterMovedFl = moveMonsterGroupOther(monsterId, getRandom(8));
				break;
			case 3:
				monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 4:
				if (getRandom(100) > 50 - randomModPct)
					monsterMovedFl = moveMonsterTowardsTeam(monsterId);
				else
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 5:
				if (getRandom(100) > 50 - randomModPct)
					monsterMovedFl = moveMonsterAwayFromTeam(monsterId);
				else
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 6:
				if (getRandom(100) >= 50 - randomModPct)
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			case 7:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				break;
			case 8:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					if (getRandom(100) >= 14 - randomModPct)
						monsterMovedFl = moveMonsterTowardsTeam(monsterId);
					else
						monsterMovedFl = moveMonsterGroupRandom(monsterId);
				}
				break;
			case 9:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					if (getRandom(100) >= 14 - randomModPct)
						monsterMovedFl = moveMonsterAwayFromTeam(monsterId);
					else
						monsterMovedFl = moveMonsterGroupRandom(monsterId);
				}
				break;
			case 10:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					monsterMovedFl = moveMonsterGroupOther(monsterId, getRandom(8));
				}
				break;
			case 11:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				}
				break;
			case 12:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					if (getRandom(100) >= 50 - randomModPct)
						monsterMovedFl = moveMonsterTowardsTeam(monsterId);
					else
						monsterMovedFl = moveMonsterGroupRandom(monsterId);
				}
				break;
			case 13:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0) {
					if (getRandom(100) >= 50 - randomModPct)
						monsterMovedFl = moveMonsterAwayFromTeam(monsterId);
					else
						monsterMovedFl = moveMonsterGroupRandom(monsterId);
				}
				break;
			case 14:
				lastRangeCheck = checkMonsterWeaponRange(monsterId);
				if (lastRangeCheck == 0 && getRandom(100) >= 50 - randomModPct)
					monsterMovedFl = moveMonsterGroupRandom(monsterId);
				break;
			default:
				break;
			}

			for (;;) {
				if (!monsterMovedFl) {
					if (lastRangeCheck == 0) {
						monsterMovedFl = true;
					} else {
						attackMonsterId = monsterId;
						monsterMovedFl = true;
					}
				} else {
					int8 checkMoveFl = checkMonsterMoveCollisionAndTileTexture(monsterId);

					if (checkMoveFl == 0) { // Blocked
						_mapMonsters[_techId][monsterId]._posX = previousPosX;
						_mapMonsters[_techId][monsterId]._posY = previousPosY;
						monsterMovedFl = false;
						--retryCounter;
					} else if (checkMoveFl == 2) { // Wall
						_mapMonsters[_techId][monsterId]._posX = previousPosX;
						_mapMonsters[_techId][monsterId]._posY = previousPosY;
					}
				}

				if (!monsterMovedFl && retryCounter == 1 && randomModPct > 1) {
					monsterMovedFl = moveMonsterGroupOther(monsterId, getRandom(8));
					continue;
				}

				break;
			}
		} while (!monsterMovedFl && retryCounter > 0);
	}

	if (attackMonsterId != -1)
		handleFight(attackMonsterId);
}

bool EfhEngine::checkMapMonsterAvailability(int16 monsterId) {
	debugC(6, kDebugEngine, "checkMapMonsterAvailability %d", monsterId);

	if (_mapMonsters[_techId][monsterId]._fullPlaceId == 0xFF)
		return false;

	for (uint counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[_techId][monsterId]._hitPoints[counter] > 0)
			return true;
	}

	return false;
}

void EfhEngine::displayMonsterAnim(int16 monsterId) {
	debugC(6, kDebugEngine, "displayMonsterAnim %d", monsterId);

	int16 animId = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._animId;
	displayAnimFrames(animId, true);
}

// The original is using an additional bool parameter which was only passed as 'False'.
// It has been removed as well as the associated code
int16 EfhEngine::countAliveMonsters(int16 id) {
	debugC(6, kDebugEngine, "countAliveMonsters %d", id);

	int16 count = 0;

	for (uint counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[_techId][id]._hitPoints[counter] > 0)
			++count;
	}

	return count;
}

bool EfhEngine::checkMonsterGroupDistance1OrLess(int16 monsterId) {
	debugC(6,kDebugEngine, "checkMonsterGroupDistance1OrLess %d", monsterId);

	if (computeMonsterGroupDistance(monsterId) > 1)
		return false;

	return true;
}

bool EfhEngine::handleTalk(int16 monsterId, int16 arg2, int16 itemId) {
	debugC(6, kDebugEngine, "handleTalk %d %d %d", monsterId, arg2, itemId);

	if (_mapMonsters[_techId][monsterId]._fullPlaceId == 0xFF)
		return false;

	if (countAliveMonsters(monsterId) < 1)
		return false;

	if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
		return false;

	if (!checkMonsterGroupDistance1OrLess(monsterId))
		return false;

	if ((_mapMonsters[_techId][monsterId]._possessivePronounSHL6 & 0x3F) != 0x3F) {
		if (_mapMonsters[_techId][monsterId]._talkTextId == 0xFF || arg2 != 5) {
			return false;
		}
		displayMonsterAnim(monsterId);
		displayImp1Text(_mapMonsters[_techId][monsterId]._talkTextId);
		displayAnimFrames(0xFE, true);
		return true;
	}

	if (isNpcATeamMember(_mapMonsters[_techId][monsterId]._npcId))
		return false;

	int16 npcId = _mapMonsters[_techId][monsterId]._npcId;
	switch (_npcBuf[npcId].field_10 - 0xEE) {
	case 0:
		if (arg2 == 4 && _npcBuf[npcId].field11_NpcId == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[npcId].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 1:
		if (arg2 == 2 && _npcBuf[npcId].field11_NpcId == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[npcId].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 2:
		if (arg2 == 1 && _npcBuf[npcId].field11_NpcId == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[npcId].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 3:
		if (_history[_npcBuf[npcId].field11_NpcId] != 0) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[npcId].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 4:
		for (int charId = 0; charId < _teamSize; ++charId) {
			for (uint inventoryId = 0; inventoryId < 10; ++inventoryId) {
				if (_npcBuf[_teamCharId[charId]]._inventory[inventoryId]._ref == _npcBuf[npcId].field11_NpcId) {
					removeObject(_teamCharId[charId], inventoryId);
					displayMonsterAnim(monsterId);
					displayImp1Text(_npcBuf[npcId].field14_textId);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 5:
		if (arg2 == 2 && _npcBuf[npcId].field11_NpcId == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[npcId].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 6:
		for (int charId = 0; charId < _teamSize; ++charId) {
			for (uint inventoryId = 0; inventoryId < 10; ++inventoryId) {
				if (_npcBuf[_teamCharId[charId]]._inventory[inventoryId]._ref == _npcBuf[npcId].field11_NpcId) {
					displayMonsterAnim(monsterId);
					displayImp1Text(_npcBuf[npcId].field14_textId);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 7:
		for (int charId = 0; charId < _teamSize; ++charId) {
			if (_npcBuf[npcId].field11_NpcId == _teamCharId[charId]) {
				removeCharacterFromTeam(charId);
				displayMonsterAnim(monsterId);
				displayImp1Text(_npcBuf[npcId].field14_textId);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 8:
		for (int charId = 0; charId < _teamSize; ++charId) {
			if (_npcBuf[npcId].field11_NpcId == _teamCharId[charId]) {
				displayMonsterAnim(monsterId);
				_enemyNamePt2 = _npcBuf[npcId]._name;
				_characterNamePt2 = _npcBuf[_teamCharId[charId]]._name;
				Common::String buffer = Common::String::format("%s asks that %s leave your party.", _enemyNamePt2.c_str(), _characterNamePt2.c_str());
				for (uint i = 0; i < 2; ++i) {
					clearBottomTextZone(0);
					_textColor = 0xE;
					displayCenteredString(buffer, 24, 296, 161);
					setTextPos(24, 169);
					displayStringAtTextPos("Will you do this?");
					if (i == 0)
						displayFctFullScreen();
				}
				setTextColorRed();
				Common::KeyCode input = mapInputCode(waitForKey());
				if (input == Common::KEYCODE_y) {
					removeCharacterFromTeam(charId);
					displayImp1Text(_npcBuf[npcId].field14_textId);
				}
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 9:
		for (int charId = 0; charId < _teamSize; ++charId) {
			if (_npcBuf[npcId].field11_NpcId == _teamCharId[charId]) {
				displayMonsterAnim(monsterId);
				displayImp1Text(_npcBuf[npcId].field14_textId);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 16:
		displayMonsterAnim(monsterId);
		displayImp1Text(_npcBuf[npcId].field14_textId);
		displayAnimFrames(0xFE, true);
		return true;
	default:

		break;
	}

	if (_npcBuf[npcId].field12_textId == 0x7FFF || arg2 != 5)
		return false;

	displayMonsterAnim(monsterId);
	displayImp1Text(_npcBuf[npcId].field12_textId);
	displayAnimFrames(0xFE, true);
	return true;
}

void EfhEngine::startTalkMenu(int16 monsterId) {
	debugC(6, kDebugEngine, "startTalkMenu %d", monsterId);

	if (monsterId != -1) {
		_tempTextPtr = nullptr;
		handleTalk(monsterId, 5, -1);
	}
}

void EfhEngine::displayImp1Text(int16 textId) {
	debugC(6, kDebugEngine,"displayImp1Text %d", textId);

	int16 charCounter = 0;
	int16 stringIdx = 0;

	if (textId <= 0xFE) {
		bool textComplete = false;
		bool maxReached = false;
		// Clear temp text on the lower left part of the screen
		if (_tempTextPtr) {
			_tempTextPtr = nullptr;
			displayMiddleLeftTempText(_tempTextPtr, true);
		}
		if (_statusMenuActive)
			drawGameScreenAndTempText(true);

		int16 curTextId = textId;

		for (;;) {
			uint8 *curString = nullptr;
			if (curTextId >= 0 && curTextId <= 0xFE) {
				curString = _imp1PtrArray[curTextId];
			}

			curTextId = 0xFF;
			if (curString == nullptr)
				break;

			do {
				if (stringIdx == 0)
					_messageToBePrinted = "";

				switch (*curString) {
				case 0x00:
				case 0x0A:
					break;
				case 0x0D:
				case 0x20:
					_messageToBePrinted += " ";
					stringIdx++;
					if (++charCounter >= 350) {
						maxReached = true;
					}
					break;
				case 0x40:
				case 0x60:
					textComplete = true;
					break;
				case 0x7C:
					_messageToBePrinted += Common::String(0x7C);
					stringIdx++;
					charCounter += 20;
					if (charCounter >= 350) {
						maxReached = true;
					}
					break;
				case 0x7E:
					maxReached = true;
					break;
				default:
					_messageToBePrinted += Common::String(*curString);
					stringIdx++;
					charCounter++;
				break;
				}
				curString += 1;
				if (maxReached || textComplete) {
					int16 nextTextId = 0xFF;
					maxReached = false;
					stringIdx = 0;
					charCounter = 0;
					uint8 firstChar = _messageToBePrinted.firstChar(); 
					if (firstChar == 0x5E || firstChar == 0) {
						if (firstChar == 0x5E) {
							nextTextId = script_parse(_messageToBePrinted, 0, 0, 319, 199, true);
							_word2C87A = false;
						}
					} else {
						for (uint counter = 0; counter < 2; ++counter) {
							drawMapWindow();
							if (counter == 0)
								displayFctFullScreen();
						}

						nextTextId = displayBoxWithText(_messageToBePrinted, 1, 1, true);
						if (nextTextId != 0xFF)
							curTextId = nextTextId;

						if (curTextId != -1) {
							for (uint counter = 0; counter < 2; ++counter) {
								if (textComplete) {
									displayCenteredString("[DONE]", 128, 303, 117);
								} else {
									displayCenteredString("[MORE]", 128, 303, 117);
								}
								if (counter == 0)
									displayFctFullScreen();
							}
							getInputBlocking();
						}
					}
					if (nextTextId != 0xFF)
						curTextId = nextTextId;
				}

			} while (!textComplete && curTextId != -1);

			textComplete = false;
			if (curTextId == 0xFF || curTextId == -1)
				break;
		}
	}

	displayAnimFrames(0xFE, true);
}

bool EfhEngine::handleInteractionText(int16 mapPosX, int16 mapPosY, int16 charId, int16 itemId, int16 arg8, int16 imageSetId) {
	debugC(3, kDebugEngine, "handleInteractionText %d-%d %d %d %d %d", mapPosX, mapPosY, charId, itemId, arg8, imageSetId);

	int16 tileId = findMapSpecialTileIndex(mapPosX, mapPosY);

	if (tileId == -1) {
		if (imageSetId != -1 && *_imp2PtrArray[imageSetId] != 0x30)
			displayMiddleLeftTempText(_imp2PtrArray[imageSetId], true);
	} else if (arg8 == 0) {
		if (_mapSpecialTiles[_techId][tileId]._field3 == 0xFF) {
			displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
			return true;
		}

		if (_mapSpecialTiles[_techId][tileId]._field3 == 0xFE) {
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;
				if (_teamCharId[counter] == _mapSpecialTiles[_techId][tileId]._triggerId) {
					displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
					return true;
				}
			}
		} else if (_mapSpecialTiles[_techId][tileId]._field3 == 0xFD) {
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (uint var2 = 0; var2 < 10; ++var2) {
					if (_npcBuf[_teamCharId[counter]]._inventory[var2]._ref == _mapSpecialTiles[_techId][tileId]._triggerId) {
						displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
						return true;
					}
				}
			}
		// original makes a useless check on (_mapSpecialTile[tileId]._field3 > 0x7F)
		} else if (_mapSpecialTiles[_techId][tileId]._field3 <= 0x77) {
			int16 var6 = _mapSpecialTiles[_techId][tileId]._field3;
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (uint var2 = 0; var2 < 39; ++var2) {
					// CHECKME : the whole loop doesn't make much sense as it's using var6 instead of var2, plus _activeScore is an array of 15 bytes, not 0x77...
					// Also, 39 correspond to the size of activeScore + passiveScore + infoScore + the 2 remaining bytes of the struct
					warning("handleInteractionText - _activeScore[%d]", var6);
					if (_npcBuf[_teamCharId[counter]]._activeScore[var6] >= _mapSpecialTiles[_techId][tileId]._triggerId) {
						displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
						return true;
					}
				}
			}
		}
	} else if ((_mapSpecialTiles[_techId][tileId]._field3 == 0xFA && arg8 == 1) || (_mapSpecialTiles[_techId][tileId]._field3 == 0xFC && arg8 == 2) || (_mapSpecialTiles[_techId][tileId]._field3 == 0xFB && arg8 == 3)) {
		if (_mapSpecialTiles[_techId][tileId]._triggerId == itemId) {
			displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
			return true;
		}
	} else if (arg8 == 4) {
		int16 var6 = _mapSpecialTiles[_techId][tileId]._field3;
		if (var6 >= 0x78 && var6 <= 0xEF) {
			var6 -= 0x78;
			warning("handleInteractionText - _activeScore[%d]", var6);
			// The 2 checks on var6 are useless, as [0x78..0xEF] - 0x78 => [0x00..0x77]
			if (var6 >= 0 && var6 <= 0x8B && var6 == itemId && _mapSpecialTiles[_techId][tileId]._triggerId <= _npcBuf[charId]._activeScore[itemId]) {
				displayImp1Text(_mapSpecialTiles[_techId][tileId]._field5_textId);
				return true;
			}
		}
	}

	for (uint counter = 0; counter < 64; ++counter) {
		if (handleTalk(counter, arg8, itemId))
			return true;
	}

	// CHECKME: there's suspiciously no check on tileId
	if ((arg8 == 4 && _mapSpecialTiles[_techId][tileId]._field3 < 0xFA) || arg8 != 4) {
		if (_mapSpecialTiles[_techId][tileId]._field7_textId > 0xFE)
			return false;
		displayImp1Text(_mapSpecialTiles[_techId][tileId]._field7_textId);
		return true;
	}

	return false;
}

int8 EfhEngine::checkTileStatus(int16 mapPosX, int16 mapPosY, bool arg4) {
	debugC(3, kDebugEngine, "checkTileStatus %d-%d %s", mapPosX, mapPosY, arg4 ? "true" : "false");

	int16 curTileInfo = getMapTileInfo(mapPosX, mapPosY);
	int16 tileFactId = _currentTileBankImageSetId[curTileInfo / 72] * 72;
	tileFactId += curTileInfo % 72;

	if (arg4) {
		handleInteractionText(mapPosX, mapPosY, -1, 0x7FFF, 0, tileFactId);
	}

	if (_word2C880) {
		_word2C880 = false;
		return -1;
	}
	if (_tileFact[tileFactId]._tileId != 0xFF && !_dbgForceMonsterBlock) {
		if ((arg4) || (!arg4 && tileFactId != 128 && tileFactId != 121)) {
			if (_largeMapFlag) {
				_mapGameMaps[_techId][mapPosX][mapPosY] = _tileFact[tileFactId]._tileId;
			} else {
				_curPlace[mapPosX][mapPosY] = _tileFact[tileFactId]._tileId;
			}

			_redrawNeededFl = true;
			if (_tileFact[tileFactId]._field0 == 0)
				return 2;
			return 1;
		}
	}

	return _tileFact[tileFactId]._field0;
}

void EfhEngine::computeInitiatives() {
	debugC(6, kDebugEngine, "computeInitiatives");

	for (int counter = 0; counter < 3; ++counter) {
		if (counter < _teamSize && _teamCharId[counter] != -1) {
			_initiatives[counter]._id = counter + 1000; // Magic value added to detect it's a member of the team
			_initiatives[counter]._initiative = _npcBuf[_teamCharId[counter]]._infoScore[3]; // "Agility"
		} else {
			_initiatives[counter]._id = -1;
			_initiatives[counter]._initiative = -1;
		}
	}

	for (int counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == -1) {
			_initiatives[counter + 3]._id = -1;
			_initiatives[counter + 3]._initiative = -1;
		} else {
			_initiatives[counter + 3]._id = counter;
			_initiatives[counter + 3]._initiative = _mapMonsters[_techId][_teamMonsterIdArray[counter]]._npcId + getRandom(20);
		}
	}

	for (uint counter = 0; counter < 8; ++counter) {
		for (uint counter2 = 0; counter2 < 8; ++counter2) {
			if (_initiatives[counter]._initiative >= _initiatives[counter2]._initiative)
				continue;

			SWAP(_initiatives[counter]._id, _initiatives[counter2]._id);
			SWAP(_initiatives[counter]._initiative, _initiatives[counter2]._initiative);
		}
	}
}

void EfhEngine::redrawScreenForced() {
	debugC(3, kDebugEngine,"redrawScreenForced");

	for (uint counter = 0; counter < 2; ++counter) {
		drawScreen();
		if (counter == 0)
			displayFctFullScreen();
	}
}

int16 EfhEngine::countMonsterGroupMembers(int16 monsterGroup) {
	debugC(9, kDebugEngine, "countMonsterGroupMembers %d", monsterGroup);

	int16 result = 0;
	for (uint counter = 0; counter < 9; ++counter) {
		if (isMonsterActive(monsterGroup, counter))
			++result;
	}

	return result;
}

uint16 EfhEngine::getXPLevel(uint32 xp) {
	debugC(6, kDebugEngine, "getXPLevel %u", xp);

	uint16 level = 0;
	int16 nextLevelXP = 1500;

	int32 wrkXp = xp;

	while (wrkXp > 0) {
		wrkXp -= nextLevelXP;
		if (wrkXp >= 0)
			++level;

		nextLevelXP += 1500;
		if (nextLevelXP > 15000)
			nextLevelXP = 15000;
	}

	return level;
}

bool EfhEngine::isItemCursed(int16 itemId) {
	debugC(6, kDebugEngine, "isItemCursed %d", itemId);

	if (_items[itemId]._specialEffect == 21 || _items[itemId]._specialEffect == 22 || _items[itemId]._specialEffect == 23)
		return true;

	return false;
}

bool EfhEngine::hasObjectEquipped(int16 charId, int16 objectId) {
	debugC(6, kDebugEngine, "hasObjectEquipped %d %d", charId, objectId);
	return _npcBuf[charId]._inventory[objectId].isEquipped();
}


void EfhEngine::setMapMonsterAggressivenessAndMovementType(int16 id, uint8 mask, bool groupFl) {
	debugC(2, kDebugEngine, "setMapMonsterAggressivenessAndMovementType %d 0x%X %s", id, mask, groupFl ? "True" : "False");

	int16 monsterId;
	if (groupFl) { // groupFl is always True
		monsterId = _teamMonsterIdArray[id];
	} else {
		monsterId = id;
	}

	mask &= 0x0F;
	_mapMonsters[_techId][monsterId]._additionalInfo &= 0xF0;
	_mapMonsters[_techId][monsterId]._additionalInfo |= mask;
}

bool EfhEngine::isMonsterActive(int16 groupId, int16 id) {
	debugC(5, kDebugEngine, "isMonsterActive %d %d", groupId, id);

	if (_mapMonsters[_techId][_teamMonsterIdArray[groupId]]._hitPoints[id] > 0 && _teamMonsterEffects[groupId]._effect[id] == 0)
		return true;
	return false;
}

int16 EfhEngine::getTileFactId(int16 mapPosX, int16 mapPosY) {
	debugC(3, kDebugEngine, "getTileFactId %d-%d", mapPosX, mapPosY);

	int16 curTileInfo = getMapTileInfo(mapPosX, mapPosY);
	int16 imageSetId = _currentTileBankImageSetId[curTileInfo / 72] * 72;
	imageSetId += curTileInfo % 72;

	return imageSetId;
}

void EfhEngine::setCharacterObjectToBroken(int16 charId, int16 objectId) {
	debugC(3, kDebugEngine, "setCharacterObjectToBroken %d %d", charId, objectId);

	_npcBuf[charId]._inventory[objectId]._ref = 0x7FFF;
}

int16 EfhEngine::selectOtherCharFromTeam() {
	debugC(3, kDebugEngine, "selectOtherCharFromTeam");

	Common::KeyCode maxVal = (Common::KeyCode) (Common::KEYCODE_0 + _teamSize);
	Common::KeyCode input = Common::KEYCODE_INVALID;
	for (;;) {
		input = waitForKey();
		if (input == Common::KEYCODE_ESCAPE || (input >= Common::KEYCODE_0 && input <= maxVal))
			break;
	}

	if (input == Common::KEYCODE_ESCAPE || input == Common::KEYCODE_0)
		return 0x1B;

	return (int16)input - (int16)Common::KEYCODE_1;
}

bool EfhEngine::checkMonsterCollision() {
	debugC(3, kDebugEngine, "checkMonsterCollision");

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkMapMonsterAvailability(monsterId))
			continue;

		if (!(_largeMapFlag && _mapMonsters[_techId][monsterId]._fullPlaceId == 0xFE)
		 && !(!_largeMapFlag && _mapMonsters[_techId][monsterId]._fullPlaceId == _fullPlaceId))
			continue;

		if ((_mapMonsters[_techId][monsterId]._possessivePronounSHL6 & 0x3F) > 0x3D
		 && ((_mapMonsters[_techId][monsterId]._possessivePronounSHL6 & 0x3F) != 0x3F || isNpcATeamMember(_mapMonsters[_techId][monsterId]._npcId)))
			continue;

		if (_mapMonsters[_techId][monsterId]._posX != _mapPosX || _mapMonsters[_techId][monsterId]._posY != _mapPosY)
			continue;

		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
		if (_imageSetSubFilesIdx != _oldImageSetSubFilesIdx)
			_oldImageSetSubFilesIdx = _imageSetSubFilesIdx;
		_redrawNeededFl = true;

		int16 mobsterCount = 0;
		for (uint mobsterCounter = 0; mobsterCounter < 9; ++mobsterCounter) {
			if (_mapMonsters[_techId][monsterId]._hitPoints[mobsterCounter])
				++mobsterCount;
		}

		bool endLoop = false;
		Common::String buffer;
		do {
			for (uint displayCounter = 0; displayCounter < 2; ++displayCounter) {
				Common::String dest;
				switch (_mapMonsters[_techId][monsterId]._possessivePronounSHL6 & 0x3F) {
				case 0x3E:
					buffer = "(NOT DEFINED)";
					dest = "(NOT DEFINED)";
					break;
				case 0x3F:
					// Special character name
					dest = _npcBuf[_mapMonsters[_techId][monsterId]._npcId]._name;
					buffer = Common::String("with ") + dest;
					break;
				default:
					dest = kEncounters[_mapMonsters[_techId][monsterId]._monsterRef]._name;
					if (mobsterCount > 1)
						dest += "s";

					buffer = Common::String::format("with %d ", mobsterCount) + dest;
					break;
				}

				clearBottomTextZone(0);
				_textColor = 0xE;
				displayCenteredString("Interaction", 24, 296, 152);
				displayCenteredString(buffer, 24, 296, 161);
				setTextPos(24, 169);
				setTextColorWhite();
				displayStringAtTextPos("T");
				setTextColorRed();
				buffer = Common::String("alk to the ") + dest;
				displayStringAtTextPos(buffer);
				setTextPos(24, 178);
				setTextColorWhite();
				displayStringAtTextPos("A");
				setTextColorRed();
				buffer = Common::String("ttack the ") + dest;
				displayStringAtTextPos(buffer);
				setTextPos(198, 169);
				setTextColorWhite();
				displayStringAtTextPos("S");
				setTextColorRed();
				displayStringAtTextPos("tatus");
				setTextPos(198, 178);
				setTextColorWhite();
				displayStringAtTextPos("L");
				setTextColorRed();
				displayStringAtTextPos("eave");
				if (displayCounter == 0)
					displayFctFullScreen();
			}

			Common::KeyCode input = mapInputCode(waitForKey());

			switch (input) {
			case Common::KEYCODE_a: // Attack
				handleFight(monsterId);
				endLoop = true;
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_l: // Leave
				endLoop = true;
				break;
			case Common::KEYCODE_s: // Status
				handleStatusMenu(1, _teamCharId[0]);
				endLoop = true;
				_tempTextPtr = nullptr;
				drawGameScreenAndTempText(true);
				break;
			case Common::KEYCODE_t: // Talk
				startTalkMenu(monsterId);
				endLoop = true;
				break;
			default:
				break;
			}
		} while (!endLoop);
		return false;
	}

	int8 check = checkTileStatus(_mapPosX, _mapPosY, true);
	if (check == 0 || check == 2)
		return false;

	return true;
}

void EfhEngine::loadImageSetToTileBank(int16 tileBankId, int16 imageSetId) {
	debugC(3, kDebugEngine, "loadImageSetToTileBank %d %d", tileBankId, imageSetId);

	// TODO: all the values of titleBankId and imageSetId are hardcoded. When all the calls are implemented, fix the values to avoid to have to decrease them
	int16 bankId = tileBankId - 1;
	int16 setId = imageSetId - 1;

	if (_currentTileBankImageSetId[bankId] == setId)
		return;

	_currentTileBankImageSetId[bankId] = setId;

	if (bankId == 0)
		_mapBitmapRefArr[_techId]._setId1 = setId;
	else if (bankId == 1)
		_mapBitmapRefArr[_techId]._setId2 = setId;

	int16 ptrIndex = bankId * 72;
	loadImageSet(setId, _tileBank[bankId], &_imageSetSubFilesArray[ptrIndex], _hiResImageBuf);
}

void EfhEngine::restoreAnimImageSetId() {
	_animImageSetId = _oldAnimImageSetId;
}

void EfhEngine::checkProtection() {
	_textColor = 0xE;

	//CHECKME : Well, yeah, some code may be missing there. Who knows.

	_protectionPassed = true;
	drawGameScreenAndTempText(true);
}

void EfhEngine::loadEfhGame() {
	debug("loadEfhGame");

	// The original used a loop to check for the presence of the savegame on the current floppy.
	// When the savegame wasn't found, it was displaying a screen asking for Disk 1 and was setting a flag used
	// to call a function after loading right before returning.
	//
	// The savegame is used to initialize the engine, so this part is reimplemented.
	// The check for existence is replaced by an error.

	Common::String fileName("savegame");
	Common::File f;

	if (!f.open(fileName))
		error("Missing file %s", fileName.c_str());

	_techId = f.readSint16LE();
	_fullPlaceId = f.readUint16LE();
	_guessAnimationAmount = f.readSint16LE();
	_largeMapFlag = f.readUint16LE();
	_teamCharId[0] = f.readSint16LE();
	_teamCharId[1] = f.readSint16LE();
	_teamCharId[2] = f.readSint16LE();

	for (int i = 0; i < 3; ++i) {
		_teamCharStatus[i]._status = f.readSint16LE();
		_teamCharStatus[i]._duration = f.readSint16LE();
	}

	_teamSize = f.readSint16LE();

	_unk2C8AA = f.readSint16LE();

	_word2C872 = f.readSint16LE();

	_imageSetSubFilesIdx = f.readSint16LE();
	_mapPosX = f.readSint16LE();
	_mapPosY = f.readSint16LE();
	_techDataId_MapPosX = f.readSint16LE();
	_techDataId_MapPosY = f.readSint16LE();

	f.close();

	_oldMapPosX = _mapPosX;
	_oldMapPosY = _mapPosY;
	_unkRelatedToAnimImageSetId = 0;
	loadNPCS();

	loadHistory();
	loadTechMapImp(_techId);

	_lastMainPlaceId = 0xFFFF;
	loadPlacesFile(_fullPlaceId, true);
}

uint8 EfhEngine::getMapTileInfo(int16 mapPosX, int16 mapPosY) {
	debugC(3, kDebugEngine, "getMapTileInfo %d-%d", mapPosX, mapPosY);

	if (_largeMapFlag)
		return _mapGameMaps[_techId][mapPosX][mapPosY];

	return _curPlace[mapPosX][mapPosY];
}

void EfhEngine::writeTechAndMapFiles() {
	// The original game overwrite game data files when switching map, keeping track of modified data.
	// In our implementation, we have everything in memory and save it in savegames only.
	// This function is therefore not useful and is not implemented.
}

uint16 EfhEngine::getStringWidth(const char *buffer) {
	debugC(6, kDebugEngine, "getStringWidth %s", buffer);

	uint16 retVal = 0;

	for (;;) {
		uint8 curChar = (uint8) *buffer++;
		if (curChar == 0) {
			--buffer;
			break;
		}

		if (curChar < 0x20)
			continue;

		retVal += _fontDescr._widthArray[curChar - 0x20] + 1;
	}

	if (retVal)
		retVal--;

	return retVal;
}

void EfhEngine::setTextPos(int16 textPosX, int16 textPosY) {
	debugC(6, kDebugEngine, "setTextPos %d-%d", textPosX, textPosY);

	_textPosX = textPosX;
	_textPosY = textPosY;
}

void EfhEngine::copyCurrentPlaceToBuffer(int16 id) {
	debugC(2, kDebugEngine, "copyCurrentPlaceToBuffer %d", id);

	// Note that 576 = 24 * 24
	uint8 *placesPtr = &_places[576 * id];

	for (uint i = 0; i < 24; ++i) {
		for (uint j = 0; j < 24; ++j) {
			_curPlace[i][j] = placesPtr[i * 24 + j];
		}
	}
}

} // End of namespace Efh
