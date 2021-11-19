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
#include "graphics/cursorman.h"

#include "efh/efh.h"
#include "efh/constants.h"

#include "engines/util.h"

namespace Efh {

EfhEngine *EfhEngine::s_Engine = nullptr;

EfhGraphicsStruct::EfhGraphicsStruct() {
	_vgaLineBuffer = nullptr;
	_shiftValue = 0;
	_width = 0;
	_height = 0;
	_area = Common::Rect(0, 0, 0, 0);
}
EfhGraphicsStruct::EfhGraphicsStruct(int16 *lineBuf, int16 x, int16 y, int16 width, int16 height) {
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
	_imageDataPtr._fieldA = 0;
	_imageDataPtr._paletteTransformation = 0;
	_imageDataPtr._fieldD = 0;

	for (int i = 0; i < 3; ++i)
		_currentTileBankImageSetId[i] = -1;

	_unkRelatedToAnimImageSetId = 0;
	_techId = 0;
	_currentAnimImageSetId = 0xFF;

	for (int i = 0; i < 20; ++i)
		_portraitSubFilesArray[i] = nullptr;

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
	_teamCharIdArray = 0;
	_charId = -1;
	_word2C8B8 = -1;

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

	_lastMainPlaceId = 0;
	_word2C86E = 0;
	_dword2C856 = nullptr;
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

Common::Error EfhEngine::run() {
	s_Engine = this;
	initialize();
	initGraphics(320, 200);

	_mainSurface = new Graphics::Surface();
	_mainSurface->create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
/*
	// Setup mixer
	syncSoundSettings();
	_soundHandler->init();

	CursorMan.replaceCursor(_normalCursor, 16, 16, 0, 0, 0);
	CursorMan.showMouse(true);
*/
	initEngine();
	sub15150(true);
	sub12A7F();
	displayLowStatusScreen(-1);

	if (!_protectionPassed)
		return Common::kNoError;

	warning("STUB - Main loop");

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
	readFileToBuffer(fileName, _animInfo);
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
	loadImageSet(imageSetId, _portraitBuf, _portraitSubFilesArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
}

void EfhEngine::loadAnimImageSet() {
	warning("STUB - loadAnimImageSet");
	if (_currentAnimImageSetId == _animImageSetId || _animImageSetId == 0xFF)
		return;

	findMapFile(_techId);

	_unkAnimRelatedIndex = 0;
	_currentAnimImageSetId = _animImageSetId;

	int16 animSetId = _animImageSetId + 17;
	loadImageSet(animSetId, _portraitBuf, _portraitSubFilesArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
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

void EfhEngine::loadPlacesFile(uint16 fullPlaceId, int16 unused, bool forceReloadFl) {
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
	warning("STUB - displayAnimFrame");
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
	readFileToBuffer(fileName, _items);
}

void EfhEngine::loadNPCS() {
	Common::String fileName = "npcs";
	readFileToBuffer(fileName, _npcBuf);
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
	return Common::KEYCODE_INVALID;
}

void EfhEngine::decryptImpFile(bool techMapFl) {
	uint16 counter = 0;
	uint16 target;
	uint8 *curPtr;

	if (!techMapFl) {
		_imp2PtrArray[++counter] = curPtr = _imp2;
		target = 431;
	} else {
		_imp2PtrArray[++counter] = curPtr = _imp1;
		target = 99;
	}

	do {
		*curPtr = (*curPtr - 3) ^ 0xD7;
		if (*curPtr == 0x40) {
			curPtr += 3;
			if (!techMapFl)
				_imp2PtrArray[++counter] = curPtr;
			else
				_imp1PtrArray[++counter] = curPtr;
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
	warning("STUB - getLastCharAfterAnimCount");
	return Common::KEYCODE_INVALID;
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
	memset(_items, 0, sizeof(_items));
	memset(_tileFact, 0, sizeof(_tileFact));
	memset(_animInfo, 0, sizeof(_animInfo));
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
	static uint8 fontWidthArray[96] = {
		3, 2, 3, 5, 5, 5, 5, 2, 3, 3, 5, 5, 3, 3, 2, 7, 4, 3, 4, 4, 5, 4, 4, 4, 4, 4, 3, 4, 4, 5, 4, 5, 1, 4, 4, 4,
		4, 4, 4, 4, 4, 3, 4, 4, 4, 7, 5, 4, 4, 4, 4, 4, 5, 4, 5, 7, 5, 5, 5, 3, 7, 3, 5, 0, 2, 4, 4, 4, 4, 4, 4, 4,
		4, 1, 2, 4, 1, 7, 4, 4, 4, 4, 4, 4, 3, 4, 5, 7, 4, 4, 5, 3, 0, 3, 0, 0
	};

	static uint8 fontExtraLinesArray[96] = {
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 3,
		1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 0, 1, 0, 0
	};

	static Font fontData[96] = {
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x00},
		{0xA0, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x50, 0xF8, 0x50, 0xF8, 0x50, 0x00, 0x00},
		{0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00},
		{0xC8, 0xC8, 0x10, 0x20, 0x40, 0x98, 0x98, 0x00},
		{0x20, 0x50, 0x20, 0x40, 0xA8, 0x90, 0x68, 0x00},
		{0x40, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x40, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x40},
		{0x40, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x40},
		{0x00, 0xA8, 0x70, 0xF8, 0x70, 0xA8, 0x00, 0x00},
		{0x00, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x20, 0x40},
		{0x00, 0x00, 0x00, 0xE0, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x00},
		{0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00},
		{0x60, 0x90, 0x90, 0x90, 0x90, 0x90, 0x60, 0x00},
		{0x40, 0xC0, 0x40, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x60, 0x90, 0x10, 0x20, 0x40, 0x80, 0xF0, 0x00},
		{0x60, 0x90, 0x10, 0x20, 0x10, 0x90, 0x60, 0x00},
		{0x10, 0x30, 0x50, 0x90, 0xF8, 0x10, 0x10, 0x00},
		{0xF0, 0x80, 0xE0, 0x10, 0x10, 0x90, 0x60, 0x00},
		{0x60, 0x90, 0x80, 0xE0, 0x90, 0x90, 0x60, 0x00},
		{0xF0, 0x10, 0x20, 0x20, 0x40, 0x40, 0x40, 0x00},
		{0x60, 0x90, 0x90, 0x60, 0x90, 0x90, 0x60, 0x00},
		{0x60, 0x90, 0x90, 0x70, 0x10, 0x90, 0x60, 0x00},
		{0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00},
		{0x00, 0x00, 0x20, 0x00, 0x00, 0x20, 0x40, 0x00},
		{0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10, 0x00},
		{0x00, 0x00, 0xF8, 0x00, 0x00, 0xF8, 0x00, 0x00},
		{0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80, 0x00},
		{0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x60, 0x90, 0x90, 0xF0, 0x90, 0x90, 0x90, 0x00},
		{0xE0, 0x90, 0x90, 0xE0, 0x90, 0x90, 0xE0, 0x00},
		{0x60, 0x90, 0x80, 0x80, 0x80, 0x90, 0x60, 0x00},
		{0xE0, 0x90, 0x90, 0x90, 0x90, 0x90, 0xE0, 0x00},
		{0xF0, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF0, 0x00},
		{0xF0, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x80, 0x00},
		{0x60, 0x90, 0x80, 0xB0, 0x90, 0x90, 0x70, 0x00},
		{0x90, 0x90, 0x90, 0xF0, 0x90, 0x90, 0x90, 0x00},
		{0xE0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00},
		{0x10, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00},
		{0x90, 0x90, 0x90, 0xE0, 0x90, 0x90, 0x90, 0x00},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0x00},
		{0x82, 0xC6, 0xAA, 0x92, 0x82, 0x82, 0x82, 0x00},
		{0x88, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x00},
		{0x60, 0x90, 0x90, 0x90, 0x90, 0x90, 0x60, 0x00},
		{0xE0, 0x90, 0x90, 0xE0, 0x80, 0x80, 0x80, 0x00},
		{0x60, 0x90, 0x90, 0x90, 0x90, 0x90, 0x60, 0x10},
		{0xE0, 0x90, 0x90, 0xE0, 0x90, 0x90, 0x90, 0x00},
		{0x60, 0x90, 0x80, 0x60, 0x10, 0x90, 0x60, 0x00},
		{0xF8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00},
		{0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x60, 0x00},
		{0x88, 0x88, 0x88, 0x50, 0x50, 0x20, 0x20, 0x00},
		{0x82, 0x82, 0x82, 0x92, 0xAA, 0xC6, 0x82, 0x00},
		{0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00},
		{0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x20, 0x00},
		{0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00},
		{0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0},
		{0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x00},
		{0x60, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x60},
		{0x20, 0x50, 0x88, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x80, 0x80, 0x40, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x60, 0x10, 0x70, 0x90, 0x70, 0x00},
		{0x80, 0x80, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0x00},
		{0x00, 0x00, 0x60, 0x90, 0x80, 0x90, 0x60, 0x00},
		{0x10, 0x10, 0x70, 0x90, 0x90, 0x90, 0x70, 0x00},
		{0x00, 0x00, 0x60, 0x90, 0xF0, 0x80, 0x60, 0x00},
		{0x30, 0x40, 0xE0, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x70, 0x90, 0x90, 0x90, 0x70, 0x10, 0xE0, 0x00},
		{0x80, 0x80, 0xE0, 0x90, 0x90, 0x90, 0x90, 0x00},
		{0x80, 0x00, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00},
		{0x40, 0x00, 0x40, 0x40, 0x40, 0x40, 0x40, 0x80},
		{0x80, 0x80, 0x90, 0x90, 0xE0, 0x90, 0x90, 0x00},
		{0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00},
		{0x00, 0x00, 0xEC, 0x92, 0x92, 0x92, 0x92, 0x00},
		{0x00, 0x00, 0xE0, 0x90, 0x90, 0x90, 0x90, 0x00},
		{0x00, 0x00, 0x60, 0x90, 0x90, 0x90, 0x60, 0x00},
		{0x00, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0x80, 0x80},
		{0x00, 0x70, 0x90, 0x90, 0x90, 0x70, 0x10, 0x10},
		{0x00, 0x00, 0xB0, 0xC0, 0x80, 0x80, 0x80, 0x00},
		{0x00, 0x00, 0x70, 0x80, 0x60, 0x10, 0xE0, 0x00},
		{0x40, 0x40, 0xE0, 0x40, 0x40, 0x40, 0x40, 0x00},
		{0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0x70, 0x00},
		{0x00, 0x00, 0x88, 0x50, 0x50, 0x20, 0x20, 0x00},
		{0x00, 0x00, 0x92, 0x92, 0x92, 0x92, 0x6E, 0x00},
		{0x00, 0x00, 0x90, 0x90, 0x60, 0x90, 0x90, 0x00},
		{0x00, 0x90, 0x90, 0x90, 0x90, 0x70, 0x10, 0xE0},
		{0x00, 0x00, 0xF8, 0x10, 0x20, 0x40, 0xF8, 0x00},
		{0x20, 0x40, 0x40, 0x80, 0x40, 0x40, 0x20, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x80, 0x40, 0x40, 0x20, 0x40, 0x40, 0x80, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
		{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}
	};

	_fontDescr._widthArray = fontWidthArray;
	_fontDescr._extraLines = fontExtraLinesArray;
	_fontDescr._fontData = fontData;
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
	loadImageSet(11, _circleImageBuf, _circleImageSubFileArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0, _paletteTransformationConstant);
	displayFctFullScreen();
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0, _paletteTransformationConstant);

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
	loadImageSet(62, _circleImageBuf, _circleImageSubFileArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
	fileName = "titlsong"; 
	readFileToBuffer(fileName, _titleSong);
	setDefaultNoteDuration();
	Common::KeyCode lastInput = playSong(_titleSong);

	if (lastInput != Common::KEYCODE_ESCAPE) {
		sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0, _paletteTransformationConstant);
		displayFctFullScreen();
		sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0, _paletteTransformationConstant);

		// Load animations on previous picture with GF
		loadImageSet(63, _circleImageBuf, _circleImageSubFileArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
		readImpFile(100, 0);
		lastInput = getLastCharAfterAnimCount(8);

		if (lastInput != Common::KEYCODE_ESCAPE) {
			sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
			sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
			displayFctFullScreen();
			sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
			sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
			lastInput = getLastCharAfterAnimCount(80);
			if (lastInput != Common::KEYCODE_ESCAPE) {
				sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 110, 16, _paletteTransformationConstant);
				sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
				sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
				displayFctFullScreen();
				sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 110, 16, _paletteTransformationConstant);
				sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
				sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
				lastInput = getLastCharAfterAnimCount(80);
				if (lastInput != Common::KEYCODE_ESCAPE) {
					sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 110, 16, _paletteTransformationConstant);
					sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
					sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
					displayFctFullScreen();
					sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 110, 16, _paletteTransformationConstant);
					sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
					sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
					lastInput = getLastCharAfterAnimCount(80);
					if (lastInput != Common::KEYCODE_ESCAPE) {
						sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
						sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
						displayFctFullScreen();
						sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
						sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
						lastInput = getLastCharAfterAnimCount(80);
						if (lastInput != Common::KEYCODE_ESCAPE) {
							sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
							sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
							displayFctFullScreen();
							sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
							sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
							lastInput = getLastCharAfterAnimCount(80);
							if (lastInput != Common::KEYCODE_ESCAPE) {
								sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
								sub133E5(_imp2PtrArray[5], 6, 150, 268, 186, 0);
								displayFctFullScreen();
								sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 144, _paletteTransformationConstant);
								sub133E5(_imp2PtrArray[5], 6, 150, 268, 186, 0);
								lastInput = getLastCharAfterAnimCount(80);
							}
						}
					}
				}
			}
		}		
	}

	loadImageSet(6, _circleImageBuf, _circleImageSubFileArray, 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
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
			groupSize = _rnd->getRandomNumber(10);

		for (uint8 counter = 0; counter < groupSize; ++counter) {
			uint rand100 = _rnd->getRandomNumber(99) + 1;
			uint16 pictureRef = _encounters[_mapMonsters[monsterId]._MonsterRef]._pictureRef;

			if (rand100 <= 25) {
				uint16 delta = _rnd->getRandomNumber((pictureRef / 2) - 1) + 1;
				_mapMonsters[monsterId]._pictureRef[counter] = pictureRef - delta;
			} else if (rand100 <= 75) {
				_mapMonsters[monsterId]._pictureRef[counter] = pictureRef;
			} else {
				uint16 delta = _rnd->getRandomNumber((pictureRef / 2) - 1) + 1;
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

void EfhEngine::sub12A7F() {
	warning("STUB - sub12A7F");
}

void EfhEngine::displayLowStatusScreen(int i) {
	warning("STUB - displayLowStatusScreen");
}

void EfhEngine::loadImageSet(int imageSetId, uint8 *buffer, uint8 **subFilesArray, char CGAVal, char EGAVal, uint8 *destBuffer, uint8 *transfBuffer) {
	Common::String fileName = Common::String::format("imageset.%d", imageSetId);
	rImageFile(fileName, buffer, subFilesArray, CGAVal, EGAVal, destBuffer, transfBuffer);
}

void EfhEngine::rImageFile(Common::String filename, uint8 *buffer, uint8 **subFilesArray, char CGAVal, char EGAVal, uint8 *packedBuffer, uint8 *targetBuffer) {
	readFileToBuffer(filename, packedBuffer);
	uint32 size = uncompressBuffer(packedBuffer, targetBuffer);
	// TODO: Keep this dump for debug purposes only
	Common::DumpFile dump;
	dump.open(filename + ".dump");
	dump.write(targetBuffer, size);
	// End of dump	
	
	// TODO: Refactoring: once uncompressed, the container contains for each image its width, its height, and raw data (1 Bpp)
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
	displayBitmapAtPos(0, 0, 319, 200);
}

void EfhEngine::displayBitmapAtPos(int16 minX, int16 minY, int16 maxX, int16 maxY) {
	_graphicsStruct->copy(_vgaGraphicsStruct2);
	_initRect = Common::Rect(minX, minY, maxX, maxY);
	displayBitmap(_vgaGraphicsStruct2, _vgaGraphicsStruct1, _initRect, minX, minY);
}

void EfhEngine::displayBitmap(EfhGraphicsStruct *efh_graphics_struct, EfhGraphicsStruct *efh_graphics_struct1, const Common::Rect &rect, int16 min_x, int16 min_y) {
	warning("STUB - displayBitmap");
}

void EfhEngine::sub24D92(BufferBM *bufferBM, int16 posX, int16 posY) {
	static uint16 byte2C80C[72] = {
		   0,    1,    2,    3,    4,    5,    6,    7,
		   8,    9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
		0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
		0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
		0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
		0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
		0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
		   0,    0,    0,    0, 0x3F,    1, 0xC7,    0
	};

	warning("STUB - sub24D92");
	
#if 0
	if (bufferBM == nullptr)
		return;

	Common::Rect unkRect;
	unkRect.left = posX - bufferBM->_startX;
	unkRect.right = unkRect.left + bufferBM->_width - 1;
	unkRect.top = posY - bufferBM->_startY;
	unkRect.bottom = unkRect.top + bufferBM->_height - 1;

	Common::Rect destRect;
	if (!computeLargeRect(_graphicsStruct->_area, &unkRect, &destRect))
		return;
	
	uint16 bufferFieldA = bufferBM->_fieldA;
	int16 deltaMinY = (destRect.top - unkRect.top) * bufferFieldA;
	int16 destWidth = destRect.width() + 1;

	int16 deltaMinX = -1;
	int16 tmpVal = (destRect.left - unkRect.left) * 2;

	if (tmpVal < 0)
		deltaMinX = 0;

	int16 deltaWidth = (unkRect.right - unkRect.left) * 2 - (destRect.right - unkRect.left) * 2 + tmpVal;

	uint8 *si = &bufferBM->_dataPtr[deltaMinY + tmpVal];
	uint8 ch = bufferBM->_paletteTransformation;
	uint8 cl = 4;
	uint16 var3A = byte2C80C[bufferBM->_fieldD << 3];
	//incomplete
#endif
}

void EfhEngine::sub133E5(uint8 *impPtr, int posX, int posY, int maxX, int maxY, int argC) {
	warning("STUB - sub133E5");
}

void EfhEngine::sub1512B() {
	displayFullScreenColoredMenuBox(0);
	sub15094();
	sub150EE();
	sub15018();
	displayAnimFrame();
	displayLowStatusScreen(0);
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
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[0], 0, 0, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[1], 112, 0, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[3], 16, 0, _paletteTransformationConstant);
}

void EfhEngine::sub150EE() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[2], 304, 0, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[4], 128, 0, _paletteTransformationConstant);
}

void EfhEngine::sub15018() {
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[7], 16, 136, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[8], 16, 192, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[5], 0, 136, _paletteTransformationConstant);
	sub10B77_unkDisplayFct1(_circleImageSubFileArray[6], 304, 136, _paletteTransformationConstant);
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
	loadImageSet(setId, _tileBank[bankId], &_imageSetSubFilesArray[ptrIndex], 0, _paletteTransformationConstant, _hiResImageBuf, _loResImageBuf);
}

void EfhEngine::restoreAnimImageSetId() {
	_animImageSetId = _oldAnimImageSetId;
}

void EfhEngine::checkProtection() {
	// bool successfulCheck = false;
	// uint8 protectionItemId = _rnd->getRandomNumber(5);
	// uint8 ProtectionArrayId = _rnd->getRandomNumber(14);
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
	//
	// Fun fact : it was therefore expected to overwrite the original savegame on the floppy each time you saved. What could possibly go wrong?

	Common::String fileName = "savegame";
	Common::File f;

	if (!f.open(fileName))
		error("Missing file %s", fileName.c_str());

	_techId = f.readSint16LE();
	_fullPlaceId = f.readUint16LE();
	_guessAnimationAmount = f.readSint16LE();
	_largeMapFlag = f.readUint16LE();
	_teamCharIdArray = f.readSint16LE();
	_charId = f.readSint16LE();
	_word2C8B8 = f.readSint16LE();

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
	loadPlacesFile(_fullPlaceId, 0, true);
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

void EfhEngine::drawBox(int minX, int minY, int maxX, int maxY) {
	warning("STUB - drawBox");
}

void EfhEngine::drawMenuBox(int minX, int minY, int maxX, int maxY, int color) {
	uint8 oldValue = _defaultBoxColor;
	_defaultBoxColor = color;
	drawBox(minX, minY, maxX, maxY);
	_defaultBoxColor = oldValue;
}

void EfhEngine::displayFullScreenColoredMenuBox(int color) {
	drawMenuBox(0, 0, 320, 200, color);
}

void EfhEngine::copyCurrentPlaceToBuffer(int id) {
	warning("STUB - copyCurrentPlaceToBuffer");
}

void EfhEngine::sub10B77_unkDisplayFct1(uint8 *imagePtr, int16 posX, int16 posY, uint8 guess_paletteTransformation) {
	uint16 height = READ_LE_INT16(imagePtr);
	uint16 width = READ_LE_INT16(imagePtr + 2);
	uint8 *imageData = imagePtr + 4;

	_imageDataPtr._fieldA = width;
	_imageDataPtr._dataPtr = imageData;
	_imageDataPtr._height = height;
	_imageDataPtr._width = width * 2;
	_imageDataPtr._startX = _imageDataPtr._startY = 0;
	
	sub24D92(&_imageDataPtr, posX, posY);
}
} // End of namespace Efh
