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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/system.h"
#include "common/random.h"
#include "common/error.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "engines/util.h"
#include "graphics/palette.h"

#include "efh/efh.h"
#include "efh/constants.h"

namespace Efh {

EfhEngine *EfhEngine::s_Engine = nullptr;

EfhGraphicsStruct::EfhGraphicsStruct() {
	_vgaLineBuffer = nullptr;
	_shiftValue = 0;
	_width = 0;
	_height = 0;
	_area = Common::Rect(0, 0, 0, 0);
}
EfhGraphicsStruct::EfhGraphicsStruct(int8 **lineBuf, int16 x, int16 y, int16 width, int16 height) {
	_vgaLineBuffer = lineBuf;
	_shiftValue = 0;
	_width = width;
	_height = height;
	_area = Common::Rect(x, y, x + width - 1, y + height - 1);
}

void EfhGraphicsStruct::copy(EfhGraphicsStruct *src) {
	// Same buffer address
	_vgaLineBuffer = src->_vgaLineBuffer;
	_shiftValue = src->_shiftValue;
	_width = src->_width;
	_height = src->_height;
	_area = src->_area;
}

void InvObject::init() {
	_ref = 0;
	_stat1 = 0;
	_stat2 = 0;
}

void UnkAnimStruct::init() {
	field0 = field1 = field2 = field3 = 0;
}

void AnimInfo::init() {
	for (int i = 0; i < 15; ++i)
		_unkAnimArray[i].init();

	for (int i = 0; i < 10; ++i) {
		_field3C_startY[i] = 0;
		_field46_startX[i] = 0;
	}
}

void ItemStruct::init() {
	for (int16 idx = 0; idx < 15; ++idx)
		_name[idx] = 0;

	_damage = 0;
	_defense = 0;
	_attacks = 0;
	_uses = 0;
	field_13 = 0;
	_range = 0;
	_attackType = 0;
	field_16 = 0;
	field17_attackTypeDefense = 0;
	field_18 = 0;
	field_19 = 0;
	field_1A = 0;
}

void NPCStruct::init() {
	for (int i = 0; i < 9; ++i)
		_name[i] =  0;
	field_9 = 0;
	field_A = 0;
	field_B = 0;
	field_C = 0;
	field_D = 0;
	field_E = 0;
	field_F = 0;
	field_10 = 0;
	field_11 = 0;
	field_12 = 0;
	field_14 = 0;
	_xp = 0;

	for (int i = 0; i < 15; ++i)
		_activeScore[i] = 0;

	for (int i = 0; i < 11; ++i) {
		_passiveScore[i] = 0;
		_infoScore[i] = 0;
	}

	field_3F = 0;
	field_40 = 0;

	for (int i = 0; i < 10; ++i)
		_inventory[i].init();
	
	_possessivePronounSHL6 = 0;
	_speed = 0;
	field_6B = 0;
	field_6C = 0;
	field_6D = 0;
	_unkItemId = 0;
	field_6F = 0;
	field_70 = 0;
	field_71 = 0;
	field_72 = 0;
	field_73 = 0;
	_hitPoints = 0;
	_maxHP = 0;
	field_78 = 0;
	field_79 = 0;
	field_7B = 0;
	field_7D = 0;
	field_7E = 0;
	field_7F = 0;
	field_80 = 0;
	field_81 = 0;
	field_82 = 0;
	field_83 = 0;
	field_84 = 0;
	field_85 = 0;
}

EfhEngine::EfhEngine(OSystem *syst, const EfhGameDescription *gd) : Engine(syst), _gameDescription(gd) {
	const Common::FSNode gameDataDir(ConfMan.get("path"));

	SearchMan.addSubDirectoryMatching(gameDataDir, "gendata");
	SearchMan.addSubDirectoryMatching(gameDataDir, "images");
	SearchMan.addSubDirectoryMatching(gameDataDir, "imp");
	SearchMan.addSubDirectoryMatching(gameDataDir, "maps");

	_system = syst;
	_rnd = nullptr;

	_shouldQuit = false;
	_eventMan = nullptr;
	_lastTime = 0;
	_gameType = kGameTypeNone;
	_platform = Common::kPlatformUnknown;
	_mainSurface = nullptr;

	_vgaGraphicsStruct1 = new EfhGraphicsStruct(_vgaLineBuffer, 0, 0, 320, 200);
	_vgaGraphicsStruct2 = new EfhGraphicsStruct();

	_videoMode = 0;
	_graphicsStruct = nullptr;
	_mapBitmapRef = nullptr;
	_mapUnknownPtr = nullptr;
	_mapMonstersPtr = nullptr;
	_mapGameMapPtr = nullptr;

	_defaultBoxColor = 0;

	_fontDescr._widthArray = nullptr;
	_fontDescr._extraLines = nullptr;
	_fontDescr._fontData = nullptr;
	_fontDescr._charHeight = 0;
	_fontDescr._extraHorizontalSpace = _fontDescr._extraVerticalSpace = 0;

	_word31E9E = 0;
	_oldAnimImageSetId = -1;
	_animImageSetId = 254;
	_paletteTransformationConstant = 10;

	for (int i = 0; i < 12; ++i)
		_circleImageSubFileArray[i] = nullptr;

	_imageDataPtr._dataPtr = nullptr;
	_imageDataPtr._width = 0;
	_imageDataPtr._startX = _imageDataPtr._startY = 0;
	_imageDataPtr._height = 0;
	_imageDataPtr._lineDataSize = 0;
	_imageDataPtr._paletteTransformation = 0;
	_imageDataPtr._fieldD = 0;

	for (int i = 0; i < 3; ++i)
		_currentTileBankImageSetId[i] = -1;

	_unkRelatedToAnimImageSetId = 0;
	_techId = 0;
	_currentAnimImageSetId = 0xFF;

	for (int i = 0; i < 20; ++i) {
		_portraitSubFilesArray[i] = nullptr;
		_ennemyNamePt2[i] = 0;
		_nameBuffer[i] = 0;
	}

	for (int i = 0; i < 100; ++i)
		_imp1PtrArray[i] = nullptr;

	for (int i = 0; i < 432; ++i)
		_imp2PtrArray[i] = nullptr;

	_unkAnimRelatedIndex = -1;

	_initRect = Common::Rect(0, 0, 0, 0);
	_engineInitPending = true;
	_unkVideoRelatedWord1 = 0x0E;
	_protectionPassed = false;
	_fullPlaceId = 0xFF;
	_guessAnimationAmount = 9;
	_largeMapFlag = 0xFFFF;
	_teamCharId[0] = 0;
	_teamCharId[1] = _teamCharId[2] = -1;

	for (int i = 0; i < 3; ++i) {
		_teamCharStatus[i]._status = 0;
		_teamCharStatus[i]._duration = 0;
		_unkArray2C8AA[i] = 0;
	}

	_unkArray2C8AA[2] = 1;
	_teamSize = 1;
	_word2C872 = 0;
	_imageSetSubFilesIdx = 144;

	_mapPosX = _mapPosY = 31;
	_oldMapPosX = _oldMapPosY = 31;
	_techDataId_MapPosX = _techDataId_MapPosY = 31;

	_textPosX = 0;
	_textPosY = 0;

	_lastMainPlaceId = 0;
	_word2C86E = 0;
	_dword2C856 = nullptr;
	_word2C880 = 0;
	_word2C894 = 0;
	_word2C8D7 = -1;
	_word2C876 = true;
	_word2C878 = true;
	_word2C87A = false;
	_unk_sub26437_flag = 0;
	_word2C8D9 = 0;

	memset(_messageToBePrinted, 0, 400);
}

EfhEngine::~EfhEngine() {
	delete _rnd;
	delete _graphicsStruct;
	delete _vgaGraphicsStruct1;
	delete _vgaGraphicsStruct2;
}

bool EfhEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher) || (f == kSupportsLoadingDuringRuntime) || (f == kSupportsSavingDuringRuntime);
}

const char *EfhEngine::getCopyrightString() const {
	return "Escape From Hell (C) Electronic Arts, 1990";
}

GameType EfhEngine::getGameType() const {
	return _gameType;
}

Common::Platform EfhEngine::getPlatform() const {
	return _platform;
}

void EfhEngine::initPalette() {
	const uint8 pal[3 * 16] = {
		0, 0, 0,
		0, 0, 170,
		0, 170, 0,
		0, 170, 170,
		170, 0, 0,
		170, 0, 170,
		170, 85, 0,
		170, 170, 170,
		85, 85, 85,
		85, 85, 255,
		1, 1, 1,
		85, 255, 255,
		255, 85, 85,
		255, 85, 255,
		255, 255, 85,
		255, 255, 255
	};
	
	_system->getPaletteManager()->setPalette(pal, 0, 16);
	_system->updateScreen();
}

Common::Error EfhEngine::run() {
	s_Engine = this;
	initialize();
	initGraphics(320, 200);

	_mainSurface = new Graphics::Surface();
	_mainSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());

	initPalette();
/*
	// Setup mixer
	syncSoundSettings();
	_soundHandler->init();
*/
	initEngine();
	sub15150(true);
	sub12A7F();
	displayLowStatusScreen(true);

	if (!_protectionPassed)
		return Common::kNoError;

	warning("STUB - Main loop");
	for (;;) {
		displayFctFullScreen();
	}
	return Common::kNoError;
}

void EfhEngine::initialize() {
	_rnd = new Common::RandomSource("Hell");
	_rnd->setSeed(666);                              // Kick random number generator
	_shouldQuit = false;
}

int32 EfhEngine::readFileToBuffer(Common::String &filename, uint8 *destBuffer) {
	Common::File f;
	if (!f.open(filename))
		error("Unable to find file %s", filename.c_str());

	int size = f.size();
	
	return f.read(destBuffer, size);
}

void EfhEngine::readAnimInfo() {
	Common::String fileName = "animinfo";
	uint8 animInfoBuf[9000];
	memset(animInfoBuf, 0, 9000);
	uint8 *curPtr = animInfoBuf;
	
	readFileToBuffer(fileName, animInfoBuf);
	for (int i = 0; i < 100; ++i) {
		for (int id = 0; id < 15; ++id) {
			_animInfo[i]._unkAnimArray[id].field0 = *curPtr++;
			_animInfo[i]._unkAnimArray[id].field1 = *curPtr++;
			_animInfo[i]._unkAnimArray[id].field2 = *curPtr++;
			_animInfo[i]._unkAnimArray[id].field3 = *curPtr++;
		}

		for (int id = 0; id < 10; ++id)
			_animInfo[i]._field3C_startY[id] = *curPtr++;

		for (int id = 0; id < 10; ++id) {
			_animInfo[i]._field46_startX[id] = READ_LE_INT16(curPtr);
			curPtr += 2;
		}
	}
}

void EfhEngine::findMapFile(int16 mapId) {
	if (_word31E9E == 0)
		return;

	Common::String fileName = Common::String::format("map.%d", mapId);
	Common::File f;
	// The original was checking for the file and eventually asking to change floppies
	if (!f.open(fileName))
		error("File not found: %s", fileName.c_str());

	f.close();
}

void EfhEngine::loadNewPortrait() {
	static int16 unkConstRelatedToAnimImageSetId[19] = {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2};
	_unkRelatedToAnimImageSetId = unkConstRelatedToAnimImageSetId[_techId];

	if (_currentAnimImageSetId == 200 + _unkRelatedToAnimImageSetId)
		return;

	findMapFile(_techId);
	_currentAnimImageSetId = 200 + _unkRelatedToAnimImageSetId;
	int imageSetId = _unkRelatedToAnimImageSetId + 13;
	loadImageSet(imageSetId, _portraitBuf, _portraitSubFilesArray, _hiResImageBuf);
}

void EfhEngine::loadAnimImageSet() {
	warning("STUB - loadAnimImageSet");
	if (_currentAnimImageSetId == _animImageSetId || _animImageSetId == 0xFF)
		return;

	findMapFile(_techId);

	_unkAnimRelatedIndex = 0;
	_currentAnimImageSetId = _animImageSetId;

	int16 animSetId = _animImageSetId + 17;
	loadImageSet(animSetId, _portraitBuf, _portraitSubFilesArray, _hiResImageBuf);
}

void EfhEngine::loadHistory() {
	Common::String fileName = "history";
	readFileToBuffer(fileName, _history);
}

void EfhEngine::loadTechMapImp(int16 fileId) {
	if (fileId == 0xFF)
		return;

	_techId = fileId;
	findMapFile(_techId);

	Common::String fileName = Common::String::format("tech.%d", _techId);
	readFileToBuffer(fileName, _hiResImageBuf);
	uncompressBuffer(_hiResImageBuf, _techData);

	fileName = Common::String::format("map.%d", _techId);
	readFileToBuffer(fileName, _hiResImageBuf);
	uncompressBuffer(_hiResImageBuf, _map);
	// This is not present in the original.
	// The purpose is to properly load the mapMonster data in an array of struct in order to use it without being a pain afterwards
	loadMapMonsters();

	loadImageSetToTileBank(1, _mapBitmapRef[0] + 1);
	loadImageSetToTileBank(2, _mapBitmapRef[1] + 1);

	initMapMonsters();
	readImpFile(_techId, true);
	displayAnimFrames(0xFE, false);
	
}

void EfhEngine::loadPlacesFile(uint16 fullPlaceId, bool forceReloadFl) {
	//TODO : Remove unused parameter when all the calls are implemented
	if (fullPlaceId == 0xFF)
		return;

	findMapFile(_techId);
	_fullPlaceId = fullPlaceId;
	uint16 minPlace = _lastMainPlaceId * 20;
	uint16 maxPlace = minPlace + 19;

	if (_fullPlaceId < minPlace || _fullPlaceId > maxPlace || forceReloadFl) {
		_lastMainPlaceId = _fullPlaceId / 20;
		Common::String fileName = Common::String::format("places.%d", _lastMainPlaceId);
		readFileToBuffer(fileName, _hiResImageBuf);
		uncompressBuffer(_hiResImageBuf, _places);
	}
	copyCurrentPlaceToBuffer(_fullPlaceId / 20);
}

void EfhEngine::drawUnknownMenuBox() {
	warning("STUB - drawUnknownMenuBox");
}

void EfhEngine::displayAnimFrame() {
	// The original had a parameter. As it was always equal to zero, it was removed in ScummVM

	if (_animImageSetId == 0xFF)
		return;

	if (_animImageSetId == 0xFE) {
		sub10B77_unkDisplayFct1(_portraitSubFilesArray[0], 16, 8);
		return;
	}

	sub10B77_unkDisplayFct1(_portraitSubFilesArray[0], 16, 8);
	for (int i = 0; i < 4; ++i) {
		int16 var2 = _animInfo[_animImageSetId]._unkAnimArray[_unkAnimRelatedIndex].field0;
		if (var2 == 0xFF)
			continue;
		sub10B77_unkDisplayFct1(_portraitSubFilesArray[var2 + 1], _animInfo[_animImageSetId]._field46_startX[var2] + 16, _animInfo[_animImageSetId]._field3C_startY[var2] + 8);
	}
}

void EfhEngine::displayAnimFrames(int16 animId, bool displayMenuBoxFl) {
	if (animId == 0xFF)
		return;

	_animImageSetId = animId;
	if (_animImageSetId == 0xFE)
		loadNewPortrait();
	else
		loadAnimImageSet();

	if (!displayMenuBoxFl)
		return;
	
	for (int i = 0; i < 2; ++i) {
		drawUnknownMenuBox();
		displayAnimFrame();

		if (i == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::readTileFact() {
	Common::String fileName = "tilefact";
	readFileToBuffer(fileName, _tileFact);
}

void EfhEngine::readItems() {
	Common::String fileName = "items";
	uint8 itemBuff[8100];
	readFileToBuffer(fileName, itemBuff);
	uint8 *curPtr = itemBuff;

	for (int i = 0; i < 300; ++i) {
		for (int16 idx = 0; idx < 15; ++idx)
			_items[i]._name[idx] = *curPtr++;

		
		_items[i]._damage = *curPtr++;
		_items[i]._defense = *curPtr++;
		_items[i]._attacks = *curPtr++;
		_items[i]._uses = *curPtr++;
		_items[i].field_13 = *curPtr++;
		_items[i]._range = *curPtr++;
		_items[i]._attackType = *curPtr++;
		_items[i].field_16 = *curPtr++;
		_items[i].field17_attackTypeDefense = *curPtr++;
		_items[i].field_18 = *curPtr++;
		_items[i].field_19 = *curPtr++;
		_items[i].field_1A = *curPtr++;
	}
}

void EfhEngine::loadNPCS() {
	Common::String fileName = "npcs";
	uint8 npcLoading[13400];
	readFileToBuffer(fileName, npcLoading);
	uint8 *curPtr = npcLoading;

	for (int i = 0; i < 99; ++i) {
		for (int idx = 0; idx < 9; ++idx)
			_npcBuf[i]._name[idx] = *curPtr++;
		_npcBuf[i].field_9 = *curPtr++;
		_npcBuf[i].field_A = *curPtr++;
		_npcBuf[i].field_B = *curPtr++;
		_npcBuf[i].field_C = *curPtr++;
		_npcBuf[i].field_D = *curPtr++;
		_npcBuf[i].field_E = *curPtr++;
		_npcBuf[i].field_F = *curPtr++;
		_npcBuf[i].field_10 = *curPtr++;
		_npcBuf[i].field_11 = *curPtr++;
		_npcBuf[i].field_12 = READ_LE_INT16(curPtr);
		_npcBuf[i].field_14 = READ_LE_INT16(curPtr + 2);
		curPtr += 4;
		_npcBuf[i]._xp = READ_LE_INT32(curPtr);
		curPtr += 4;
		for (int idx = 0; idx < 15; ++idx) {
			_npcBuf[i]._activeScore[idx] = *curPtr++;
		}
		for (int idx = 0; idx < 11; ++idx) {
			_npcBuf[i]._passiveScore[idx] = *curPtr++;
		}
		for (int idx = 0; idx < 11; ++idx) {
			_npcBuf[i]._infoScore[idx] = *curPtr++;
		}
		_npcBuf[i].field_3F = *curPtr++;
		_npcBuf[i].field_40 = *curPtr++;
		for (int idx = 0; idx < 10; ++idx) {
			_npcBuf[i]._inventory[idx]._ref = READ_LE_INT16(curPtr);
			curPtr += 2;
			_npcBuf[i]._inventory[idx]._stat1 = *curPtr++;
			_npcBuf[i]._inventory[idx]._stat2 = *curPtr++;
		}
		_npcBuf[i]._possessivePronounSHL6 = *curPtr++;
		_npcBuf[i]._speed = *curPtr++;
		_npcBuf[i].field_6B = *curPtr++;
		_npcBuf[i].field_6C = *curPtr++;
		_npcBuf[i].field_6D = *curPtr++;
		_npcBuf[i]._unkItemId = *curPtr++;
		_npcBuf[i].field_6F = *curPtr++;
		_npcBuf[i].field_70 = *curPtr++;
		_npcBuf[i].field_71 = *curPtr++;
		_npcBuf[i].field_72 = *curPtr++;
		_npcBuf[i].field_73 = *curPtr++;
		_npcBuf[i]._hitPoints = READ_LE_INT16(curPtr);
		_npcBuf[i]._maxHP = READ_LE_INT16(curPtr + 2);
		curPtr += 4;
		_npcBuf[i].field_78 = *curPtr++;
		_npcBuf[i].field_79 = READ_LE_INT16(curPtr);
		_npcBuf[i].field_7B = READ_LE_INT16(curPtr + 2);
		curPtr += 4;
		_npcBuf[i].field_7D = *curPtr++;
		_npcBuf[i].field_7E = *curPtr++;
		_npcBuf[i].field_7F = *curPtr++;
		_npcBuf[i].field_80 = *curPtr++;
		_npcBuf[i].field_81 = *curPtr++;
		_npcBuf[i].field_82 = *curPtr++;
		_npcBuf[i].field_83 = *curPtr++;
		_npcBuf[i].field_84 = *curPtr++;
		_npcBuf[i].field_85 = *curPtr++;
	}
}

void EfhEngine::setDefaultNoteDuration() {
	// Original implementation is based on the int1C, which is triggered at 18.2065Hz.
	// Every 4 times, it sets a flag (thus, approx every 220ms)
	// The function was checking the keyboard in a loop, incrementing a counter and setting the last character read
	// The "_defaultNoteDuration" was then set to 7 times this counter
	//
	// No implementation required in ScummVM
}

Common::KeyCode EfhEngine::playSong(uint8 *buffer) {
	warning("STUB: playSong");
	_system->delayMillis(1000);
	
	return Common::KEYCODE_INVALID;
}

void EfhEngine::decryptImpFile(bool techMapFl) {
	uint16 counter = 0;
	uint16 target;
	uint8 *curPtr;

	if (!techMapFl) {
		_imp2PtrArray[counter++] = curPtr = _imp2;
		target = 431;
	} else {
		_imp2PtrArray[counter++] = curPtr = _imp1;
		target = 99;
	}

	do {
		*curPtr = (*curPtr - 3) ^ 0xD7;
		if (*curPtr == 0x40) {
			curPtr += 3;
			if (!techMapFl)
				_imp2PtrArray[counter++] = curPtr;
			else
				_imp1PtrArray[counter++] = curPtr;
		} else
			++curPtr;
	} while (*curPtr != 0x60 && counter <= target);

	Common::DumpFile dump;
	if (!techMapFl) {
		dump.open("imp2_unc.dump");
		dump.write(_imp2, curPtr - _imp2);
	} else {
		dump.open("imp1_unc.dump");
		dump.write(_imp1, curPtr - _imp1);
	}
	dump.close();
}

void EfhEngine::readImpFile(int16 id, bool techMapFl) {
	Common::String fileName = Common::String::format("imp.%d", id);

	if (techMapFl)
		readFileToBuffer(fileName, _imp1);
	else
		readFileToBuffer(fileName, _imp2);

	decryptImpFile(techMapFl);
}

Common::KeyCode EfhEngine::getLastCharAfterAnimCount(int16 delay) {
	if (delay == 0)
		return Common::KEYCODE_INVALID;

	Common::Event event;
	do {
		_system->getEventManager()->pollEvent(event);
	} while (event.kbd.keycode != Common::KEYCODE_INVALID);

	Common::KeyCode lastChar = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (delay > 0 && lastChar == Common::KEYCODE_INVALID) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			--delay;
			unkFct_anim();
		}

		lastChar = handleAndMapInput(false);
	} 
	
	return lastChar;
}

void EfhEngine::playIntro() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0);

	// Load animations on previous picture with GF
	loadImageSet(63, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	readImpFile(100, false);
	Common::KeyCode lastInput = getLastCharAfterAnimCount(8);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// With GF on the bed
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// Poof
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 110, 16);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 110, 16);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// On the phone
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 110, 16);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 110, 16);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[5], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[5], 6, 150, 268, 186, 0);
	getLastCharAfterAnimCount(80);
}

void EfhEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

//	_sound->syncVolume();
}

Common::String EfhEngine::getSavegameFilename(int slot) {
	return _targetName + Common::String::format("-%02d.SAV", slot);
}

void EfhEngine::initEngine() {
	_videoMode = 2; // In the original, 2 = VGA/MCGA, EGA = 4, Tandy = 6, cga = 8.
	memset(_bufferCharBM, 0, sizeof(_bufferCharBM));
	_graphicsStruct = new EfhGraphicsStruct;
	_graphicsStruct->copy(_vgaGraphicsStruct1);

	for (int i = 0; i < 3; ++i) {
		memset(_tileBank[i], 0, sizeof(_tileBank[i]));
	}

	memset(_circleImageBuf, 0, sizeof(_circleImageBuf));
	memset(_portraitBuf, 0, sizeof(_portraitBuf));
	memset(_hiResImageBuf, 0, sizeof(_hiResImageBuf));
	memset(_loResImageBuf, 0, sizeof(_loResImageBuf));
	memset(_menuBuf, 0, sizeof(_menuBuf));
	memset(_windowWithBorderBuf, 0, sizeof(_windowWithBorderBuf));
	memset(_map, 0, sizeof(_map));
	memset(_places, 0, sizeof(_places));
	memset(_curPlace, 0, sizeof(_curPlace));
	memset(_npcBuf, 0, sizeof(_npcBuf));
	memset(_imp1, 0, sizeof(_imp1));
	memset(_imp2, 0, sizeof(_imp2));
	memset(_titleSong, 0, sizeof(_titleSong));
	for (int i = 0; i < 300; ++i)
		_items[i].init();
	memset(_tileFact, 0, sizeof(_tileFact));

	for (int i = 0; i < 100; ++i)
		_animInfo[i].init();

	memset(_history, 0, sizeof(_history));
	memset(_techData, 0, sizeof(_techData));

	_mapBitmapRef = &_map[0];
	_mapUnknownPtr = &_map[2];

	// Replaces _mapMonstersPtr which was equal to &_map[902];
	for (int i = 0; i < 64; ++i) {
		_mapMonsters[i]._possessivePronounSHL6 = 0;
		_mapMonsters[i]._field_1 = 0;
		_mapMonsters[i]._guess_fullPlaceId = 0xFF;
		_mapMonsters[i]._posX = 0;
		_mapMonsters[i]._posY = 0;
		_mapMonsters[i]._itemId_Weapon = 0;
		_mapMonsters[i]._field_6 = 0;
		_mapMonsters[i]._MonsterRef = 0;
		_mapMonsters[i]._field_8 = 0;
		_mapMonsters[i]._field_9 = 0;
		_mapMonsters[i]._groupSize = 0;
		for (int j = 0; j < 9; ++j)
			_mapMonsters[i]._pictureRef[j] = 0;
	}
	
	_mapGameMapPtr = &_map[2758];

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
	_word31E9E = 0;

	saveAnimImageSetId();

	// Save int 1C
	// Set new int 1C:
	// TODO: Implement that in the main loop
	// static uint8 counter = 0;
	// ++counter;
	// if (counter == 4) {
	//    counter = 0;
	//    tick220Fl = 1;
	// }

	// Load Title Screen
	loadImageSet(11, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0);

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

	if (lastInput != Common::KEYCODE_ESCAPE) {
		playIntro();
	}

	loadImageSet(6, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	readImpFile(99, false);
	_word31E9E = 0xFFFF;
	restoreAnimImageSetId();

	// Note: The original at this point saves int 24h and sets a new int24 to handle fatal failure

	checkProtection();
	loadGame();
	_engineInitPending = false;
}

void EfhEngine::initMapMonsters() {
	for (uint8 monsterId = 0; monsterId < 64; ++monsterId) {
		if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
			continue;

		for (uint8 counter = 0; counter < 9; ++counter)
			_mapMonsters[monsterId]._pictureRef[counter] = 0;

		uint8 groupSize = _mapMonsters[monsterId]._groupSize;
		if (groupSize == 0)
			groupSize = getRandom(10);

		for (uint8 counter = 0; counter < groupSize; ++counter) {
			uint rand100 = getRandom(100);
			uint16 pictureRef = kEncounters[_mapMonsters[monsterId]._MonsterRef]._pictureRef;

			if (rand100 <= 25) {
				uint16 delta = getRandom(pictureRef / 2);
				_mapMonsters[monsterId]._pictureRef[counter] = pictureRef - delta;
			} else if (rand100 <= 75) {
				_mapMonsters[monsterId]._pictureRef[counter] = pictureRef;
			} else {
				uint16 delta = getRandom(pictureRef / 2);
				_mapMonsters[monsterId]._pictureRef[counter] = pictureRef + delta;
			}
		}		
	}
}

void EfhEngine::loadMapMonsters() {
	_mapMonstersPtr = &_map[902];

	for (int i = 0; i < 64; ++i) {
		_mapMonsters[i]._possessivePronounSHL6 = _mapMonstersPtr[29 * i];
		_mapMonsters[i]._field_1 = _mapMonstersPtr[29 * i + 1];
		_mapMonsters[i]._guess_fullPlaceId = _mapMonstersPtr[29 * i + 2];
		_mapMonsters[i]._posX = _mapMonstersPtr[29 * i + 3];
		_mapMonsters[i]._posY = _mapMonstersPtr[29 * i + 4];
		_mapMonsters[i]._itemId_Weapon = _mapMonstersPtr[29 * i + 5];
		_mapMonsters[i]._field_6 = _mapMonstersPtr[29 * i + 6];
		_mapMonsters[i]._MonsterRef = _mapMonstersPtr[29 * i + 7];
		_mapMonsters[i]._field_8 = _mapMonstersPtr[29 * i + 8];
		_mapMonsters[i]._field_9 = _mapMonstersPtr[29 * i + 9];
		_mapMonsters[i]._groupSize = _mapMonstersPtr[29 * i + 10];
		for (int j = 0; j < 9; ++j)
			_mapMonsters[i]._pictureRef[j] = READ_LE_UINT16(&_mapMonstersPtr[29 * i + 11 + j * 2]);
	}
}

void EfhEngine::saveAnimImageSetId() {
	_oldAnimImageSetId = _animImageSetId;
	_animImageSetId = 255;
}

int16 EfhEngine::getEquipmentDefense(int16 charId, bool flag) {
	int16 altDef = 0;
	int16 totalDef = 0;
	for (int i = 0; i < 10; ++i) {
		if (_npcBuf[charId]._inventory[i]._ref == 0x7FFF)
			continue;

		if ((_npcBuf[charId]._inventory[i]._stat1 & 0x80) == 0)
			continue;

		int16 curDef = _npcBuf[charId]._inventory[i]._stat2;
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

uint16 EfhEngine::sub1C80A(int16 charId, int field18, bool flag) {
	for (int i = 0; i < 10; ++i) {
		if ((_npcBuf[charId]._inventory[i]._stat1 & 0x80) == 0)
			continue;

		int16 itemId = _npcBuf[charId]._inventory[i]._ref;
		
		if (_items[itemId].field_18 != field18)
			continue;

		if (!flag)
			return i;

		return itemId;
	}

	return 0x7FFF;
}

void EfhEngine::sub15150(bool flag) {
	uint8 mapTileInfo = getMapTileInfo(_mapPosX, _mapPosY);
	int16 imageSetId = _currentTileBankImageSetId[mapTileInfo / 72];

	int16 mapImageSetId = (imageSetId * 72) + (mapTileInfo % 72);
	// CHECKME : Why do we compute this Id if we don't use it?
	
	for (int counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			sub1512B();
			// TODO: _word2C86E is some kind of counter
			if (_word2C86E != 0) {
				// TODO: _dword2C856 is most likely an "Imp" Array
				// Note: the original was doing the check in the opposite order, which looks really suspicious
				if ((_dword2C856 != nullptr) && (_dword2C856[0] != 0x30)) {
					sub221FA(_dword2C856, false);
				}
			}
		}

		if (counter == 0 && flag)
			displayFctFullScreen();
	}
}

void EfhEngine::sub1258F(bool largeMapFl, int16 posX, int16 posY, int imapSize, bool unkFl1, bool unkFl2) {
	warning("STUB : sub1258F");
}

void EfhEngine::sub1256E(int16 posX, int16 posY) {
	sub1258F(false, posX, posY, 23, _word2C876, _word2C878);
}

void EfhEngine::sub1254C(int16 posX, int16 posY) {
	sub1258F(true, posX, posY, 63, _word2C876, _word2C878);
}

void EfhEngine::sub12A7F() {
	for (int16 counter = 0; counter < 2; ++counter) {
		_word2C894 = 0;
		if (!_largeMapFlag) {
			if (_fullPlaceId != 0xFF)
				sub1256E(_mapPosX, _mapPosY);

			if (_word2C8D9 != 0)
				sub150EE();
		} else {
			if (_techId != 0xFF)
				sub1254C(_mapPosX, _mapPosY);
			
			if (_word2C8D9 != 0)
				sub150EE();
		}
		if (counter == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::displayLowStatusScreen(bool flag) {
	static char strName[5] = "Name";
	static char strDef[4] = "DEF";
	static char strHp[3] = "HP";
	static char strMaxHp[7] = "Max HP";
	static char strWeapon[7] = "Weapon";
	static char strDead[9] = "* DEAD *";

	char buffer[80];
	memset(buffer, 0, 80);
	
	for (int counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			unkFct_displayMenuBox_2(0);
			set_unkVideoRelatedWord1_to_0Fh();
			displayCenteredString(strName, 16, 88, 152);
			displayCenteredString(strDef, 104, 128, 152);
			displayCenteredString(strHp, 144, 176, 152);
			displayCenteredString(strMaxHp, 192, 224, 152);
			displayCenteredString(strWeapon, 225, 302, 152);
			set_unkVideoRelatedWord1_to_0Ch();

			for (int i = 0; i < 3; ++i) {
				if (_teamCharId[i] == -1)
					continue;
				int16 charId = _teamCharId[i];
				int16 textPosY = 161 + 9 * i;
				copyString(_npcBuf[charId]._name, buffer);
				setTextPos(16, textPosY);
				unkFct_displayString_2(buffer);
				sprintf(buffer, "%d", getEquipmentDefense(charId, false));
				displayCenteredString(buffer, 104, 128, textPosY);
				sprintf(buffer, "%d", _npcBuf[charId]._hitPoints);
				displayCenteredString(buffer, 144, 176, textPosY);
				sprintf(buffer, "%d", _npcBuf[charId]._maxHP);
				displayCenteredString(buffer, 192, 224, textPosY);

				if (_npcBuf[charId]._hitPoints <= 0) {
					displayCenteredString(strDead, 225, 302, textPosY);
					continue;
				}

				switch (_teamCharStatus[i]._status) {
				case 0: {
					uint16 var4 = sub1C80A(charId, 9, true);
					if (var4 == 0x7FFF)
						strcpy(_nameBuffer, "(NONE)");
					else
						copyString(_items[var4]._name, _nameBuffer);
					}
					break;				
				case 1:
					strcpy(_nameBuffer, "* ASLEEP *");
					break;
				case 2:
					strcpy(_nameBuffer, "* FROZEN *");
					break;
				default:
					strcpy(_nameBuffer, "* DISABLED *");
					break;
				}

				displayCenteredString(_nameBuffer, 225, 302, textPosY);
			}
		}

		if (counter == 0 && flag)
			displayFctFullScreen();
	}
}

void EfhEngine::loadImageSet(int imageSetId, uint8 *buffer, uint8 **subFilesArray, uint8 *destBuffer) {
	Common::String fileName = Common::String::format("imageset.%d", imageSetId);
	rImageFile(fileName, buffer, subFilesArray, destBuffer);
}

void EfhEngine::rImageFile(Common::String filename, uint8 *targetBuffer, uint8 **subFilesArray, uint8 *packedBuffer) {
	readFileToBuffer(filename, packedBuffer);
	uint32 size = uncompressBuffer(packedBuffer, targetBuffer);
	// TODO: Keep this dump for debug purposes only
	Common::DumpFile dump;
	dump.open(filename + ".dump");
	dump.write(targetBuffer, size);
	// End of dump	
	
	// TODO: Refactoring: once uncompressed, the container contains for each image its width, its height, and raw data (4 Bpp)
	// => Write a class to handle that more properly
	uint8 *ptr = targetBuffer;
	uint16 counter = 0;
	while (READ_LE_INT16(ptr) != 0) {
		subFilesArray[counter] = ptr;
		++counter;
		int16 imageWidth = READ_LE_INT16(ptr);
		ptr += 2;
		int16 imageHeight = READ_LE_INT16(ptr);
		ptr += 2;
		ptr += (imageWidth * imageHeight);
	}
}

void EfhEngine::displayFctFullScreen() {
	// CHECKME: 319 is in the original but looks suspicious.
	copyDirtyRect(0, 0, 319, 200);

	
	_system->copyRectToScreen((uint8 *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
	_system->updateScreen();
}

void EfhEngine::copyDirtyRect(int16 minX, int16 minY, int16 maxX, int16 maxY) {
	_graphicsStruct->copy(_vgaGraphicsStruct2);
	_initRect = Common::Rect(minX, minY, maxX, maxY);
	copyGraphicBufferFromTo(_vgaGraphicsStruct2, _vgaGraphicsStruct1, _initRect, minX, minY);
}

void EfhEngine::copyGraphicBufferFromTo(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y) {
	warning("STUB - copyGraphicBufferFromTo");
}

void EfhEngine::sub24D92(BufferBM *bufferBM, int16 posX, int16 posY) {
	// TODO: Quick code to display stuff, may require to really reverse the actual function
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);
	// warning("%d %d - startX %d startY %d width %d height %d lineDataSize %d fieldD %d", posX, posY, bufferBM->_startX, bufferBM->_startY, bufferBM->_width, bufferBM->_height, bufferBM->_lineDataSize, bufferBM->_fieldD);
	int counter = 0;
	for (int line = 0; line < bufferBM->_height; ++line) {
		for (int col = 0; col < bufferBM->_lineDataSize; ++col) { // _lineDataSize = _width / 2
			destPtr[320 * line + 2 * col] = bufferBM->_dataPtr[counter] >> 4;
			destPtr[320 * line + 2 * col + 1] = bufferBM->_dataPtr[counter] & 0xF;
			++counter;
		}
	}

//	_system->copyRectToScreen((uint8 *)_mainSurface->getPixels(), 320, 0, 0, 320, 200);
//	_system->updateScreen();
}

uint8 *EfhEngine::script_readNumberArray(uint8 *srcBuffer, int16 destArraySize, int16 *destArray) {
	uint8 *buffer = srcBuffer;

	for (int16 i = 0; i < destArraySize; ++i) {
		buffer = script_getNumber(buffer, &destArray[i]);
	}

	return buffer;
}

uint8 *EfhEngine::script_getNumber(uint8 *srcBuffer, int16 *retval) {
	uint8 *buffer = srcBuffer; 
	int16 var2 = 0;
	for (;;) {
		uint8 curChar = *buffer;
		if (curChar < 0x30 || curChar > 0x39) {
			*retval = var2;
			return buffer;
		}
		var2 = var2 * 10 + curChar - 0x30;
	}
}

void EfhEngine::removeObject(int16 charId, int16 objectId) {
	_npcBuf[charId]._inventory[objectId]._ref = 0x7FFF;
	_npcBuf[charId]._inventory[objectId]._stat1 = 0;
	_npcBuf[charId]._inventory[objectId]._stat2 = 0;
}

void EfhEngine::totalPartyKill() {
	for (int16 counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1)
			_npcBuf[counter]._hitPoints = 0;
	}
}

int16 EfhEngine::getRandom(int16 maxVal) {
	if (maxVal == 0)
		return 0;

	return 1 + _rnd->getRandomNumber(maxVal - 1);
}

void EfhEngine::removeCharacterFromTeam(int16 teamMemberId) {
	warning("STUB - removeCharacterFromTeam");
}

void EfhEngine::emptyFunction(int i) {
	// TODO: Eventually remove this useless function
}

void EfhEngine::refreshTeamSize() {
	_teamSize = 0;
	for (int16 counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1)
			++_teamSize;
	}
}

bool EfhEngine::isCharacterATeamMember(int16 id) {
	for (int16 counter = 0; counter < _teamSize; ++counter) {
		if (_teamCharId[counter] == id)
			return true;
	}

	return false;
}

void EfhEngine::handleWinSequence() {
	warning("STUB - handleWinSequence");
}

bool EfhEngine::giveItemTo(int16 charId, int16 objectId, int altCharId) {
	warning("STUB - giveItemTo");

	return false;
}

void EfhEngine::sub26437(char *str, int16 startX, int16 startY, uint16 unkFl) {
	uint8 *curPtr = (uint8 *)str;
	uint16 lineHeight = _fontDescr._charHeight + _fontDescr._extraVerticalSpace;
	_unk_sub26437_flag = unkFl & 0x3FFF;
	int16 minX = startX;
	int16 minY = startY;                                 // Used in case 0x8000
	int16 var6 = _fontDescr._extraLines[0] + startY - 1; // Used in case 0x8000

	if (unkFl & 0x8000) {
		warning("STUB - sub26437 - 0x8000");
	}

	for (uint8 curChar = *curPtr++; curChar != 0; curChar = *curPtr++) {
		if (curChar == 0x0A) {
			startX = minX;
			startY += lineHeight;
			continue;
		}

		if (curChar < 0x20)
			continue;

		uint16 characterId = (curChar + 0xE0) & 0xFF;
		uint8 charWidth = _fontDescr._widthArray[characterId];

		if (startX + charWidth >= 319) {
			startX = minX;
			startY += lineHeight;
		}

		uint8 varC = _fontDescr._extraLines[characterId];
		sub252CE(curChar, startX, startY + varC);
		startX += charWidth + _fontDescr._extraHorizontalSpace;
	}
	
}

void EfhEngine::displayCenteredString(char *str, int16 minX, int16 maxX, int16 posY) {
	uint16 length = getStringWidth(str);
	int16 startCenteredDisplayX = minX + (maxX - minX - length) / 2;
	sub26437(str, startCenteredDisplayX, posY, _unkVideoRelatedWord1);
}

int16 EfhEngine::chooseCharacterToReplace() {
	warning("STUB - chooseCharacterToReplace");
	return 0x1B;
}

int16 EfhEngine::handleCharacterJoining() {
	static char strReplaceWho[13] = "Replace Who?";
	for (int16 counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] == -1) {
			return counter;
		}
	}

	for (int16 counter = 0; counter < 2; ++counter) {
		drawMenuBox(200, 112, 278, 132, 0);
		displayCenteredString(strReplaceWho, 200, 278, 117);
		if (counter == 0)
			displayFctFullScreen();
	}

	int16 charId = chooseCharacterToReplace();
	for (int16 counter = 0; counter < 2; ++counter) {
		drawMenuBox(200, 112, 278, 132, 0);
		if (counter == 0)
			displayFctFullScreen();
	}

	if (charId == 0x1B) // Escape Keycode
		return -1;

	removeCharacterFromTeam(charId);
	return 2;
}

void EfhEngine::drawMapWindow() {
	drawMenuBox(128, 8, 303, 135, 0);
}

void EfhEngine::copyString(char *srcStr, char *destStr) {
	char lastChar = 1;
	int16 idx = 0;

	while (lastChar != 0) {
		lastChar = destStr[idx] = srcStr[idx];
		++idx;
	}
}

int16 EfhEngine::script_parse(uint8 *stringBuffer, int posX, int posY, int maxX, int maxY, int argC) {	
	bool doneFlag = false;
	int16 var_F2 = -1;
	int16 var_F0 = 0xFF;
	int16 var_EE = 0xFF;
	const char *stringToDisplay = " ";
	uint16 curLine = 0;
	int16 numbLines = (1 + maxY - posY) / 9;
	int16 width = maxX - posX;
	int16 var_114 = getStringWidth((char *)stringToDisplay);
	uint8 *buffer = stringBuffer;
	char var_EC[80];
	char dest[150];
	memset(var_EC, 0, sizeof(var_EC));
	memset(dest, 0, sizeof(dest));
	int16 var_116 = 0;
	setTextPos(posX, curLine * 9 + posY);

	while (!doneFlag) {
		uint8 curChar = *buffer;
		if (curChar != 0x5E && curChar != 0x20 && curChar != 0 && curChar != 0x7C) {
			var_F2 = 0;
			var_EC[var_116++] = curChar;
			++buffer;
			continue;
		}

		if (curChar != 0x5E) {
			if (curChar == 0)
				doneFlag = true;
			else if (curChar == 0x7C)
				var_F2 = 0;

			var_EC[var_116] = 0;
			int16 var_11A = getStringWidth(var_EC);
			int16 var_118 = var_114 + getStringWidth(dest);

			if (var_118 + var_11A > width || curChar == 0x7C) {
				if (curLine >= numbLines) {
					doneFlag = true;
				} else {
					if (var_F2 == 0)
						unkFct_displayString_2(dest);

					*dest = 0;
					strcpy(dest, var_EC);
					strcat(dest, " ");
					++curLine;
					setTextPos(posX, posY + curLine * 9);
					var_116 = 0;
				}
			} else {
				strcat(dest, var_EC);
				strcat(dest, " ");
				var_116 = 0;
			}
			++buffer;
			continue;
		}

		// At this point, curChar == 0x5E
		++buffer;
		int16 var_108 = 0;
		buffer = script_getNumber(buffer, &var_108);
		int16 scriptNumberArray[10];
		memset(scriptNumberArray, 0, ARRAYSIZE(scriptNumberArray));

		switch (var_108) {
		case 0x00:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				if (_largeMapFlag) {
					_largeMapFlag = false;
					_techDataId_MapPosX = _mapPosX;
					_techDataId_MapPosY = _mapPosY;
				}
				_oldMapPosX = _mapPosX = scriptNumberArray[1];
				_oldMapPosY = _mapPosY = scriptNumberArray[2];
				loadPlacesFile(scriptNumberArray[0], false);
				_word2C880 = -1;
				_word2C894 = -1;
			}
			break;
		case 0x01:
			if (argC != 0) {
				_largeMapFlag = true;
				_oldMapPosX = _mapPosX = _techDataId_MapPosX;
				_oldMapPosY = _mapPosY = _techDataId_MapPosY;
				_word2C880 = -1;
				_word2C894 = -1;
			}
			break;
		case 0x02:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				if (_word2C8D7)
					writeTechAndMapFiles();
				_oldMapPosX = _mapPosX = scriptNumberArray[1];
				_oldMapPosY = _mapPosY = scriptNumberArray[2];
				loadTechMapImp(scriptNumberArray[0]);
				_largeMapFlag = true;
				_word2C880 = -1;
				_word2C894 = -1;
				doneFlag = true;
			}
			break;
		case 0x03:
			buffer = script_readNumberArray(buffer, 4, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[2] - scriptNumberArray[0];
				int16 var10E = scriptNumberArray[3] - scriptNumberArray[1];

				_mapPosX = getRandom(var110) + scriptNumberArray[0] - 1;
				_mapPosY = getRandom(var10E) + scriptNumberArray[1] - 1;
				_word2C880 = -1;
				_word2C894 = -1;
			}
			break;
		case 0x04:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				_mapPosX = scriptNumberArray[0];
				_mapPosY = scriptNumberArray[1];
				_word2C880 = -1;
				_word2C894 = -1;
			}
			break;
		case 0x05:
			buffer = script_readNumberArray(buffer, 4, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				if (var110 != -1) {
					int16 var10E = scriptNumberArray[1];
					_npcBuf[var110]._activeScore[var10E] += scriptNumberArray[2] & 0xFF;
					_npcBuf[var110]._activeScore[var10E] -= scriptNumberArray[3] & 0xFF;
				}
			}
			break;
		case 0x06:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				if (var110 != -1) {
					int16 var10E = scriptNumberArray[1];
					_npcBuf[var110]._activeScore[var10E] = scriptNumberArray[1];
				}
			}
			break;
		case 0x07:
			if (argC != 0) {
				totalPartyKill();
				emptyFunction(2);
			}
			break;
		case 0x08:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0 && scriptNumberArray[0] != -1) {
				_npcBuf[_teamCharId[scriptNumberArray[0]]]._hitPoints = 0;
			}
			break;		
		case 0x09:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				if (var110 != -1) {
					int16 var10E = getRandom(scriptNumberArray[1]);
					_npcBuf[var110]._hitPoints += var10E;
					if (_npcBuf[var110]._hitPoints > _npcBuf[var110]._maxHP)
						_npcBuf[var110]._hitPoints = _npcBuf[var110]._maxHP;
				}
			}
			break;
		case 0x0A:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				if (var110 != -1) {
					_npcBuf[var110]._hitPoints = _npcBuf[var110]._maxHP;
				}
			}
			break;
		case 0x0B:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				if (var110 != -1) {
					int16 var10E = getRandom(scriptNumberArray[1]);
					_npcBuf[var110]._hitPoints -= var10E;
					if (_npcBuf[var110]._hitPoints < 0)
						_npcBuf[var110]._hitPoints = 0;
				}
			}
			break;
		case 0x0C:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				bool found = false;
				for (int16 counter = 0; counter < _teamSize && !found; ++counter) {
					for (int16 objectId = 0; objectId < 10; ++objectId) {
						if (_npcBuf[_teamCharId[counter]]._inventory[objectId]._ref == var110) {
							removeObject(_teamCharId[counter], objectId);
							found = true;
							break;
						}						
					}
				}
			}
			break;
		case 0x0D:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = _teamCharId[scriptNumberArray[0]];
				for (int16 counter = 0; counter < _teamSize; ++counter) {
					if (giveItemTo(_teamCharId[counter], var110, 0xFF))
						break;
				}
			}
			break;
		case 0x0E:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[0];
				bool found = false;
				for (int16 counter = 0; counter < _teamSize && !found; ++counter) {
					for (int16 objectId = 0; objectId < 10; ++objectId) {
						if (_npcBuf[_teamCharId[counter]]._inventory[objectId]._ref == var110) {
							found = true;
							break;
						}
					}
				}

				if (found)
					var_F0 = scriptNumberArray[1];
				else
					var_F0 = scriptNumberArray[2];
			}
			break;
		case 0x0F:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[0];
				if (isCharacterATeamMember(var110))
					var_F0 = scriptNumberArray[1];
				else
					var_F0 = scriptNumberArray[2];	
			}
			break;
		case 0x10:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0)
				var_F0 = scriptNumberArray[0];

			break;
		case 0x11:
			if (argC != 0)
				_unkArray2C8AA[0] = 0;
			break;
		case 0x12:
			if (argC != 0) {
				int16 var110 = sub151FD(_mapPosX, _mapPosY);
				if (var110 != -1)
					_mapUnknownPtr[9 * var110 + 1] = 0xFF;
			}
			break;
		case 0x13:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0 && _largeMapFlag) {
				_word2C87A = true;
				loadPlacesFile(scriptNumberArray[0], false);
				sub15A28(scriptNumberArray[1], scriptNumberArray[2]);
				sub2455E(scriptNumberArray[0], scriptNumberArray[1], scriptNumberArray[2]);
				var_F0 = -1;
			}
			break;
		case 0x14:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[0];
				if (!isCharacterATeamMember(var110))
					var_EE = var110;
				var_F0 = -1;
			}
			break;
		case 0x15:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				_oldMapPosX = _mapPosX = scriptNumberArray[0];
				_oldMapPosY = _mapPosY = scriptNumberArray[1];
				_largeMapFlag = true;
				_word2C894 = -1;
			}
			break;
		case 0x16:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[0];
				// TODO: This "if" is useless, it's doing just the same loop and if statement. Consider removing it.
				if (isCharacterATeamMember(var110)) {
					for (int16 counter = 0; counter < 3; ++counter) {
						if (_teamCharId[counter] == var110) {
							removeCharacterFromTeam(counter);
							break;
						}
					}
				}				
			}
			break;
		case 0x17:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[0];
				displayAnimFrames(var110, true);
			}
			break;
		case 0x18:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = scriptNumberArray[1] - scriptNumberArray[0] + 1;
				bool found = false;
				var110 = getRandom(var110) + scriptNumberArray[0] - 1;
				int16 counter;
				for (counter = 0; counter < _teamSize; ++counter) {
					if (giveItemTo(_teamCharId[counter], var110, 0xFF)) {
						found = true;
						break;
					}
				}

				if (!found) {
					drawMapWindow();
					displayFctFullScreen();
					drawMapWindow();
					var110 = sub1C219("Nothing...", 1, 2, 0xFFFF);
					displayFctFullScreen();
				} else {
					copyString(_npcBuf[_teamCharId[counter]]._name, _ennemyNamePt2);
					copyString(_items[var110]._name, _nameBuffer);
					sprintf(dest, "%s finds a %s!", _ennemyNamePt2, _nameBuffer);
					drawMapWindow();
					displayFctFullScreen();
					drawMapWindow();
					var110 = sub1C219(dest, 1, 2, 0xFFFF);
					displayFctFullScreen();
				}

				var110 = sub151FD(_mapPosX, _mapPosY);
				if (var110 != -1) {
					_mapUnknownPtr[var110 * 9 + 1] = 0xFF;
				}
				_word2C894 = -1;
			}
			break;
		case 0x19:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				if (_largeMapFlag) {
					_mapGameMapPtr[scriptNumberArray[0] * 6 + scriptNumberArray[1]] = scriptNumberArray[2] & 0xFF;
				} else {
					_curPlace[scriptNumberArray[0] * 24 + scriptNumberArray[1]] = scriptNumberArray[2] & 0xFF;
				}
			}
			break;
		case 0x1A:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = sub151FD(scriptNumberArray[0], scriptNumberArray[1]);
				if (var110 != -1) {
					_mapUnknownPtr[9 * var110 + 1] = 0xFF;
				}
			}
			break;
		case 0x1B:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				int16 var110 = sub151FD(scriptNumberArray[0], scriptNumberArray[1]);
				if (var110 != -1) {
					_mapUnknownPtr[9 * var110 + 1] = 0xFF;
				}
				_mapUnknownPtr[9 * scriptNumberArray[2] + 1] = scriptNumberArray[0];
				_mapUnknownPtr[9 * scriptNumberArray[2] + 2] = scriptNumberArray[1];
			}
			break;
		case 0x1C:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				_history[scriptNumberArray[0]] = 0xFF;
			}
			break;
		case 0x1D:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0) {
				_history[scriptNumberArray[0]] = 0;
			}
			break;
		case 0x1E:
			buffer = script_readNumberArray(buffer, 3, scriptNumberArray);
			if (argC != 0) {
				if (_history[scriptNumberArray[0]] == 0)
					var_F0 = scriptNumberArray[2];
				else
					var_F0 = scriptNumberArray[1];
			}
			break;
		case 0x1F:
			buffer = script_readNumberArray(buffer, 1, scriptNumberArray);
			if (argC != 0)
				_unkArray2C8AA[0] = scriptNumberArray[0];

			break;
		case 0x20:
			if (argC != 0) {
				handleWinSequence();
				_system->quit();
			}
		default:
			break;
		}
	}

	if (*dest != 0 && curLine < numbLines && var_F2 == 0)
		unkFct_displayString_2(dest);

	if (var_EE != 0xFF) {
		displayLowStatusScreen(true);
		int16 teamSlot = handleCharacterJoining();
		if (teamSlot > -1) {
			_teamCharId[teamSlot] = var_EE;
		}
		refreshTeamSize();
	}

	return var_F0;
}

void EfhEngine::sub133E5(uint8 *srcPtr, int posX, int posY, int maxX, int maxY, int argC) {
	uint16 stringIdx = 0;
	uint8 *impPtr = srcPtr;
	memset(_messageToBePrinted, 0, 200);
	
	for (;;) {
		uint8 curChar = *impPtr;
		
		if (curChar == 0 || curChar == 0x40 || curChar == 0x60)
			break;

		if (curChar == 0x0D) {
			_messageToBePrinted[stringIdx++] = ' ';
			++impPtr;
		} else if (curChar == 0x0A) {
			++impPtr;
		} else {
			_messageToBePrinted[stringIdx++] = curChar;
			++impPtr;
		}
	}

	script_parse(_messageToBePrinted, posX, posY, maxX, maxY, argC);
}

void EfhEngine::sub1512B() {
	displayFullScreenColoredMenuBox(0);
	sub15094();
	sub150EE();
	sub15018();
	displayAnimFrame();
	displayLowStatusScreen(false);
}

void EfhEngine::sub221FA(uint8 *impArray, bool flag) {
	for (uint8 counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			drawMenuBox(16, 115, 111, 133, 0);
			if (impArray != nullptr) {
				_word2C86E = 4;
				_dword2C856 = impArray;
				sub133E5(impArray, 17, 115, 110, 133, 0);
			}
		}
	}
}

void EfhEngine::sub15094() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 112, 0);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[3], 16, 0);
}

void EfhEngine::sub150EE() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 304, 0);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[4], 128, 0);
}

void EfhEngine::sub15018() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[7], 16, 136);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[8], 16, 192);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[5], 0, 136);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[6], 304, 136);
}

void EfhEngine::sub15A28(int16 arg0, int16 arg2) {
	warning("STUB: sub15A28");
}

void EfhEngine::sub2455E(int16 arg0, int16 arg1, int16 arg2) {
	warning("STUB: sub2455E");
}

int16 EfhEngine::sub1C219(const char *str, int menuType, int arg4, int displayTeamWindowFl) {
	warning("STUB: sub1C219");
	return -1;
}

int16 EfhEngine::sub151FD(int16 posX, int16 posY) {
	warning("STUB: sub151FD");
	return -1;
}

void EfhEngine::sub252CE(uint8 curChar, int16 posX, int posY) {
	// Quick hacked display, may require rework
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);

	int16 charId = curChar - 0x20;
	uint8 width = _fontDescr._widthArray[charId];

	for (int16 line = 0; line < 8; ++line) {
		int16 x = 0;
		for (int i = 7; i >= 7 - width; --i) {
			if (_fontDescr._fontData[charId]._lines[line] & (1 << i))
				destPtr[320 * line + x] = 14;
			++x;
		}
	}	
}

void EfhEngine::set_unkVideoRelatedWord1_to_0Fh() {
	if (_videoMode == 8) // CGA
		_unkVideoRelatedWord1 = 0x3;
	else
		_unkVideoRelatedWord1 = 0xF;
}

void EfhEngine::set_unkVideoRelatedWord1_to_0Ch() {
	if (_videoMode == 8) // CGA
		_unkVideoRelatedWord1 = 0x2;
	else
		_unkVideoRelatedWord1 = 0xC;
}

void EfhEngine::setNumLock() {
	// No implementation in ScummVM
}

void EfhEngine::unkfct_mapFunction() {
	warning("STUB - unkfct_mapFunction");
}

void EfhEngine::unkFct_anim() {
	setNumLock();

	if (_engineInitPending)
		return;

	if (_animImageSetId != 0xFF) {
		displayNextAnimFrame();
		displayFctFullScreen();
		displayAnimFrame();
	}

	unkfct_mapFunction();
}

void EfhEngine::setNextCharacterPos() {
	if (_textPosX <= 311)
		return;

	_textPosX = 0;
	_textPosY += 8;

	if (_textPosY > 191)
		_textPosY = 0;
}

void EfhEngine::unkFct_displayString_2(char *message) {
	sub26437(message, _textPosX, _textPosY, _unkVideoRelatedWord1);
	_textPosX += getStringWidth(message) + 1;
	setNextCharacterPos();
}

void EfhEngine::unkFct_displayMenuBox_2(int16 color) {
	drawMenuBox(16, 152, 302, 189, color);
}

void EfhEngine::loadImageSetToTileBank(int16 tileBankId, int16 imageSetId) {
	// TODO: all the values of titleBankId and imageSetId are hardcoded. When all the calls are implemented, fix the values to avoid to have to decrease them
	int16 bankId = tileBankId - 1;
	int16 setId = imageSetId - 1;

	if (_currentTileBankImageSetId[bankId] == setId)
		return;

	_currentTileBankImageSetId[bankId] = setId;

	if (bankId == 0 || bankId == 1)
		_mapBitmapRef[bankId] = setId;

	int16 ptrIndex = bankId * 72;
	loadImageSet(setId, _tileBank[bankId], &_imageSetSubFilesArray[ptrIndex], _hiResImageBuf);
}

void EfhEngine::restoreAnimImageSetId() {
	_animImageSetId = _oldAnimImageSetId;
}

void EfhEngine::checkProtection() {
	_unkVideoRelatedWord1 = 0xE;

	//CHECKME : Well, yeah, some code may be missing there. Who knows.
	
	_protectionPassed = true;
	sub15150(true);	
}

void EfhEngine::loadGame() {
	// The original used a loop to check for the presence of the savegame on the current floppy.
	// When the savegame wasn't found, it was displaying a screen asking for Disk 1 and was setting a flag used
	// to call a function after loading right before returning.
	//
	// The savegame is used to initialize the engine, so this part is reimplemented.
	// The check for existence is replaced by an error.

	Common::String fileName = "savegame";
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

	for (int i = 0; i < 3; ++i) {
		_unkArray2C8AA[i] = f.readSint16LE();		
	}

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

uint32 EfhEngine::uncompressBuffer(uint8 *compressedBuf, uint8 *destBuf) {
	if (compressedBuf == nullptr || destBuf == nullptr)
		error("uncompressBuffer - Invalid pointer used in parameter list");

	uint8 *curPtrDest = destBuf;

	uint16 compSize = READ_LE_UINT16(compressedBuf) + 1;	
	uint8 *curPtrCompressed = compressedBuf + 2;

	// Not in the original. This has been added for debug purposes (the original function doesn't return a value)
	uint32 decompSize = 0;

	for (;;) {
		uint8 next = *curPtrCompressed++;
		if (--compSize <= 0)
			break;

		if (next != 0xC3) {
			*curPtrDest++ = next;
			++decompSize;
			continue;
		}

		next = *curPtrCompressed++;
		if (--compSize <= 0)
			break;

		if (next == 0) {
			*curPtrDest++ = 0xC3;
			++decompSize;
			continue;
		}
			
		uint8 loopVal = next;
		next = *curPtrCompressed++;

		for (int i = 0; i < loopVal; ++i) {
			*curPtrDest++ = next;
			++decompSize;
		}
		
		--compSize;
		if (compSize == 0)
			break;
	}

	curPtrDest[0] = curPtrDest[1] = 0;
	decompSize += 2;

	return decompSize;
}

uint8 EfhEngine::getMapTileInfo(int16 mapPosX, int16 mapPosY) {
	int size = _largeMapFlag ? 32 : 24;

	return _mapGameMapPtr[mapPosX * size + mapPosY];
}

void EfhEngine::drawRect(int minX, int minY, int maxX, int maxY) {
	if (minY > maxY)
		SWAP(minY, maxY);

	if (minX > maxX)
		SWAP(minX, maxX);
	
	// warning("drawRect - _graphicsStruct x %d -> %d, y %d -> %d", _graphicsStruct->_area.left, _graphicsStruct->_area.right, _graphicsStruct->_area.top, _graphicsStruct->_area.bottom);

	minX = CLIP(minX, 0, 319);
	maxX = CLIP(maxX, 0, 319);
	minY = CLIP(minY, 0, 199);
	maxY = CLIP(maxY, 0, 199);
	
	int deltaY = 1 + maxY - minY;
	int deltaX = 1 + maxX - minX;

	uint8 color = _defaultBoxColor & 0xF;
	bool xorColor = (_defaultBoxColor & 0x40) != 0;
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(minX, minY);
	
	for (int line = 0; line < deltaY; ++line) {
		for (int col = 0; col < deltaX; ++col) {
			if (xorColor)
				destPtr[320 * line + col] ^= color;
			else
				destPtr[320 * line + col] = color;
		}
	}
	
}

void EfhEngine::drawMenuBox(int minX, int minY, int maxX, int maxY, int color) {
	uint8 oldValue = _defaultBoxColor;
	_defaultBoxColor = color;
	drawRect(minX, minY, maxX, maxY);
	_defaultBoxColor = oldValue;
}

void EfhEngine::displayFullScreenColoredMenuBox(int color) {
	drawMenuBox(0, 0, 320, 200, color);
}

Common::KeyCode EfhEngine::handleAndMapInput(bool animFl) {
	// The original checks for the joystick input
	Common::Event event;
	_system->getEventManager()->pollEvent(event);
	Common::KeyCode retVal = Common::KEYCODE_INVALID;
	if (event.type == Common::EVENT_KEYUP) {
		retVal = event.kbd.keycode;
	} 	

	if (animFl) {
		warning("STUB - handleAndMapInput - animFl");
	}
	return retVal;
}

void EfhEngine::displayNextAnimFrame() {
	if (++_unkAnimRelatedIndex >= 15)
		_unkAnimRelatedIndex = 0;

	displayAnimFrame();
}

void EfhEngine::writeTechAndMapFiles() {
	warning("STUB - writeTechAndMapFiles");
}

uint16 EfhEngine::getStringWidth(char *buffer) {
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

	return retVal;
}

void EfhEngine::setTextPos(int16 textPosX, int16 textPosY) {
	_textPosX = textPosX;
	_textPosY = textPosY;
}

void EfhEngine::copyCurrentPlaceToBuffer(int id) {
	uint8 *placesPtr = &_places[576 * id];

	// Note that 576 = 24 * 24
	memcpy(_curPlace, placesPtr, 24 * 24);
}

void EfhEngine::sub10B77_unkDisplayFct1(uint8 *imagePtr, int16 posX, int16 posY) {
	uint16 height = READ_LE_INT16(imagePtr);
	uint16 width = READ_LE_INT16(imagePtr + 2);
	uint8 *imageData = imagePtr + 4;

	_imageDataPtr._lineDataSize = width;
	_imageDataPtr._dataPtr = imageData;
	_imageDataPtr._height = height;
	_imageDataPtr._width = width * 2; // 2 pixels per byte
	_imageDataPtr._startX = _imageDataPtr._startY = 0;
	
	sub24D92(&_imageDataPtr, posX, posY);
}
} // End of namespace Efh
