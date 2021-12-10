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

	_word31E9E = false;
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
		_characterNamePt2[i] = 0;
		_nameBuffer[i] = 0;
	}

	for (int i = 0; i < 100; ++i)
		_imp1PtrArray[i] = nullptr;

	for (int i = 0; i < 432; ++i)
		_imp2PtrArray[i] = nullptr;

	_unkAnimRelatedIndex = -1;

	_initRect = Common::Rect(0, 0, 0, 0);
	_engineInitPending = true;
	_textColor = 0x0E; // Yellow
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

	for (int i = 0; i < 5; ++i)
		_teamMonsterIdArray[i] = -1;

	_unkArray2C8AA[2] = 1;
	_teamSize = 1;
	_word2C872 = 0;
	_imageSetSubFilesIdx = 144;
	_oldImageSetSubFilesIdx = 143;

	_mapPosX = _mapPosY = 31;
	_oldMapPosX = _oldMapPosY = 31;
	_techDataId_MapPosX = _techDataId_MapPosY = 31;

	_textPosX = 0;
	_textPosY = 0;

	_lastMainPlaceId = 0;
	_word2C86E = 0;
	_dword2C856 = nullptr;
	_word2C880 = false;
	_redrawNeededFl = false;
	_word2C8D7 = true;
	_drawHeroOnMapFl = true;
	_drawMonstersOnMapFl = true;
	_word2C87A = false;
	_unk_sub26437_flag = 0;
	_word2C8D9 = false;
	_word2C8D5 = false;
	_word2D0BC = false;
	_word2C8D2 = false;
	_menuDepth = 0;
	_word2D0BA = 0;


	for (int i = 0; i < 15; ++i) {
		_word3273A[i] = 0;
	}

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
	redrawScreen();
	displayLowStatusScreen(true);

	if (!_protectionPassed)
		return Common::kNoError;

	uint32 lastMs = _system->getMillis();
	warning("STUB - Main loop");
	for (;;) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			unkFct_anim();
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

		default:
			if (retVal != Common::KEYCODE_INVALID)
				warning("Main Loop: Unhandled input %d", retVal);
			break;
		}

		if ((_mapPosX != _oldMapPosX || _mapPosY != _oldMapPosY) && !_shouldQuit) {
			int16 var4 = sub16E14();
			if (!_word2C8D5 || var4 != 0) {
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
			sub174A0();
		}

		if (_redrawNeededFl && !_shouldQuit) {
			redrawScreen();
			displayLowStatusScreen(true);
		}

		if (!_shouldQuit) {
			handleNewRoundEffects();

			if (_word2C86E > 0) {
				if (--_word2C86E == 0) {
					sub221FA(nullptr, true);
				}
			}
		}

		if (--_unkArray2C8AA[0] < 0 && !_shouldQuit)
			_unkArray2C8AA[0] = 0;

		if (isTPK()) {
			if (handleDeathMenu())
				_shouldQuit = true;
		}
		
		warning("Main loop - missing implementation");
		
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
	if (!_word31E9E)
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

void EfhEngine::drawLeftCenterBox() {
	drawColoredRect(16, 8, 111, 135, 0);
}

void EfhEngine::displayAnimFrame() {
	// The original had a parameter. As it was always equal to zero, it was removed in ScummVM

	if (_animImageSetId == 0xFF)
		return;

	if (_animImageSetId == 0xFE) {
		displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
		return;
	}

	displayRawDataAtPos(_portraitSubFilesArray[0], 16, 8);
	for (int i = 0; i < 4; ++i) {
		int16 var2 = _animInfo[_animImageSetId]._unkAnimArray[_unkAnimRelatedIndex].field0;
		if (var2 == 0xFF)
			continue;
		displayRawDataAtPos(_portraitSubFilesArray[var2 + 1], _animInfo[_animImageSetId]._field46_startX[var2] + 16, _animInfo[_animImageSetId]._field3C_startY[var2] + 8);
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
		drawLeftCenterBox();
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

void EfhEngine::playIntro() {
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
	sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[0], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// Poof
	displayRawDataAtPos(_circleImageSubFileArray[1], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[1], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[1], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	// On the phone
	displayRawDataAtPos(_circleImageSubFileArray[2], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[2], 110, 16);
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[2], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[3], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[4], 6, 150, 268, 186, 0);
	lastInput = getLastCharAfterAnimCount(80);
	if (lastInput == Common::KEYCODE_ESCAPE)
		return;

	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
	sub133E5(_imp2PtrArray[5], 6, 150, 268, 186, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 144);
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
	_word31E9E = false;

	saveAnimImageSetId();

	// Load Title Screen
	loadImageSet(11, _circleImageBuf, _circleImageSubFileArray, _hiResImageBuf);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	displayFctFullScreen();
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);

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
	_word31E9E = true;
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
			_mapMonsters[i]._pictureRef[j] = READ_LE_INT16(&_mapMonstersPtr[29 * i + 11 + j * 2]);
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
			displayGameScreen();
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

void EfhEngine::drawMap(bool largeMapFl, int16 mapPosX, int16 mapPosY, int mapSize, bool drawHeroFl, bool drawMonstersFl) {
	int16 unkPosX = 5;
	int16 unkPosY = 4;
	int16 posX = 0;
	int16 posY = 0;
	int16 var6 = 0;
	int16 minX = mapPosX - 5;
	int16 minY = mapPosY - 4;

	if (minX < 0) {
		unkPosX -= minX;
		minX = 0;
	}

	if (minY < 0) {
		unkPosY -= minY;
		minY = 0;
	}

	int16 maxX = minX + 10;
	int16 maxY = minY + 7;

	if (maxX > mapSize) {
		unkPosX += (maxX - mapSize);
		maxX = mapSize;
		minX = mapSize - 10;
	}

	if (maxY > mapSize) {
		unkPosY += (maxY - mapSize);
		maxY = mapSize;
		minY = mapSize - 7;
	}

	int16 var10 = 8;
	for (int16 counterY = minY; counterY <= maxY; ++counterY) {
		int16 var12 = 128;
		for (int16 var16 = minX; var16 <= maxX; ++var16) {
			if (largeMapFl) {
				int16 idx = _mapGameMapPtr[(var16 * 64) + counterY]; // 64 = large map size (square)
				displayRawDataAtPos(_imageSetSubFilesArray[idx], var12, var10);
			} else {
				int16 idx = _curPlace[(var16 * 24) + counterY]; // 24 = small map size (square)
				displayRawDataAtPos(_imageSetSubFilesArray[idx], var12, var10);
			}
			var12 += 16;
		}
		var10 += 16;
	}

	if (drawHeroFl) {
		int16 var12 = 128 + unkPosX * 16;
		var10 = 8 + unkPosY * 16;
		displayRawDataAtPos(_imageSetSubFilesArray[_imageSetSubFilesIdx], var12, var10);
	}

	if (drawMonstersFl) {
		for (int16 var16 = 0; var16 < 64; ++var16) {
			if ((_largeMapFlag && _mapMonsters[var16]._guess_fullPlaceId == 0xFE) || (!_largeMapFlag && _mapMonsters[var16]._guess_fullPlaceId == _fullPlaceId)){
				bool var4 = false;
				posX = _mapMonsters[var16]._posX;
				posY = _mapMonsters[var16]._posY;

				if (posX < minX || posX > maxX || posY < minY || posY > maxY)
					continue;

				for (int16 counterY = 0; counterY < 9 && !var4; ++counterY) {
					if (_mapMonsters[var16]._pictureRef[counterY] > 0)
						var4 = true;
				}

				if (!var4)
					continue;

				var6 = 148 + kEncounters[_mapMonsters[var16]._MonsterRef]._animId;
				int16 var1 = _mapMonsters[var16]._possessivePronounSHL6 & 0x3F;

				if (var1 == 0x3F && isCharacterATeamMember(_mapMonsters[var16]._field_1))
					continue;

				int16 var12 = 128 + (posX - minX) * 16;
				var10 = 8 + (posY - minY) * 16;
				displayRawDataAtPos(_imageSetSubFilesArray[var6], var12, var10);
			}
		}
	}

	if (_word2C8D7)
		return;

	warning("drawMap() - unexpected code reached, not implemented");
}

void EfhEngine::displaySmallMap(int16 posX, int16 posY) {
	drawMap(false, posX, posY, 23, _drawHeroOnMapFl, _drawMonstersOnMapFl);
}

void EfhEngine::displayLargeMap(int16 posX, int16 posY) {
	drawMap(true, posX, posY, 63, _drawHeroOnMapFl, _drawMonstersOnMapFl);
}

void EfhEngine::redrawScreen() {
	for (int16 counter = 0; counter < 2; ++counter) {
		_redrawNeededFl = false;
		if (!_largeMapFlag) {
			if (_fullPlaceId != 0xFF)
				displaySmallMap(_mapPosX, _mapPosY);

			if (_word2C8D9)
				drawUpperRightBorders();
		} else {
			if (_techId != 0xFF)
				displayLargeMap(_mapPosX, _mapPosY);
			
			if (_word2C8D9)
				drawUpperRightBorders();
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
			setTextColorWhite();
			displayCenteredString(strName, 16, 88, 152);
			displayCenteredString(strDef, 104, 128, 152);
			displayCenteredString(strHp, 144, 176, 152);
			displayCenteredString(strMaxHp, 192, 224, 152);
			displayCenteredString(strWeapon, 225, 302, 152);
			setTextColorRed();

			for (int i = 0; i < 3; ++i) {
				if (_teamCharId[i] == -1)
					continue;
				int16 charId = _teamCharId[i];
				int16 textPosY = 161 + 9 * i;
				copyString(_npcBuf[charId]._name, buffer);
				setTextPos(16, textPosY);
				displayStringAtTextPos(buffer);
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
	// copyDirtyRect(0, 0, 319, 200);

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

void EfhEngine::displayBufferBmAtPos(BufferBM *bufferBM, int16 posX, int16 posY) {
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
	int16 charId = _teamCharId[teamMemberId];
	_npcBuf[charId].field_12 = _npcBuf[charId].field_B;
	_npcBuf[charId].field_14 = _npcBuf[charId].field_E;
	_npcBuf[charId].field_10 = _npcBuf[charId].field_C;
	_npcBuf[charId].field_11 = _npcBuf[charId].field_D;

	_teamCharId[teamMemberId] = -1;
	_teamCharStatus[teamMemberId]._status = 0;
	_teamCharStatus[teamMemberId]._duration = 0;

	for (int16 var4 = teamMemberId; var4 < 2; ++var4) {
		_teamCharId[var4] = _teamCharId[var4 + 1];
		_teamCharId[var4 + 1] = -1;
	}

	refreshTeamSize();
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

bool EfhEngine::isTPK() {
	int16 zeroedChar = 0;
	for (int16 counter = 0; counter < _teamSize; ++counter) {
		if (_npcBuf[_teamCharId[counter]]._hitPoints <= 0)
			++zeroedChar;
	}

	return zeroedChar == _teamSize;
}

void EfhEngine::handleWinSequence() {
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

	for (int16 counter = 0; counter < 2; ++counter) {
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		displayRawDataAtPos(winSeqSubFilesArray2[0], 136, 48);
		if (counter == 0)
			displayFctFullScreen();
	}

	getInput(12);
	for (int16 counter2 = 1; counter2 < 8; ++counter2) {
		for (int16 counter = 0; counter < 2; ++counter) {
			displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
			displayRawDataAtPos(winSeqSubFilesArray2[counter2], 136, 48);
			if (counter == 0)
				displayFctFullScreen();
		}
		getInput(1);
	}

	Common::KeyCode var59 = Common::KEYCODE_INVALID;

	while(var59 != Common::KEYCODE_ESCAPE) {
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		displayFctFullScreen();
		displayRawDataAtPos(winSeqSubFilesArray1[0], 0, 0);
		var59 = getInput(32);
		if (var59 != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[10], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[10], 136, 72);
			var59 = getInput(1);
		}

		if (var59 != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[11], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[11], 136, 72);
			var59 = getInput(1);
		}

		if (var59 != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[12], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[12], 136, 72);
			var59 = getInput(1);
		}

		if (var59 != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[13], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[13], 136, 72);
			var59 = getInput(1);
		}

		if (var59 != Common::KEYCODE_ESCAPE) {
			displayRawDataAtPos(winSeqSubFilesArray2[14], 136, 72);
			displayFctFullScreen();
			displayRawDataAtPos(winSeqSubFilesArray2[14], 136, 72);
			var59 = getInput(1);
		}
	}
	
	free(decompBuffer);
	free(winSeqBuf3);
	free(winSeqBuf4);
}

bool EfhEngine::giveItemTo(int16 charId, int16 objectId, int altCharId) {
	for (int16 newObjectId = 0; newObjectId < 10; ++newObjectId) {
		if (_npcBuf[charId]._inventory[newObjectId]._ref != 0x7FFF)
			continue;

		if (altCharId == 0xFF) {
			_npcBuf[charId]._inventory[newObjectId]._ref = objectId;
			_npcBuf[charId]._inventory[newObjectId]._stat2 = _items[objectId]._defense;
			_npcBuf[charId]._inventory[newObjectId]._stat1 = _items[objectId]._uses;
		} else {
			_npcBuf[charId]._inventory[newObjectId]._ref = _npcBuf[altCharId]._inventory[newObjectId]._ref;
			_npcBuf[charId]._inventory[newObjectId]._stat2 = _npcBuf[altCharId]._inventory[newObjectId]._stat2;
			_npcBuf[charId]._inventory[newObjectId]._stat1 = _npcBuf[altCharId]._inventory[newObjectId]._stat1 & 0x7F;
		}

		return true;
	}

	return false;
}

void EfhEngine::drawString(const char *str, int16 startX, int16 startY, uint16 unkFl) {
	uint8 *curPtr = (uint8 *)str;
	uint16 lineHeight = _fontDescr._charHeight + _fontDescr._extraVerticalSpace;
	_unk_sub26437_flag = unkFl & 0x3FFF;
	int16 minX = startX;
	int16 minY = startY;                                 // Used in case 0x8000
	int16 var6 = _fontDescr._extraLines[0] + startY - 1; // Used in case 0x8000

	if (unkFl & 0x8000) {
		warning("STUB - drawString - 0x8000");
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
		drawChar(curChar, startX, startY + varC);
		startX += charWidth + _fontDescr._extraHorizontalSpace;
	}
	
}

void EfhEngine::displayCenteredString(const char *str, int16 minX, int16 maxX, int16 posY) {
	uint16 length = getStringWidth(str);
	int16 startCenteredDisplayX = minX + (maxX - minX - length) / 2;
	drawString(str, startCenteredDisplayX, posY, _textColor);
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
		drawColoredRect(200, 112, 278, 132, 0);
		displayCenteredString(strReplaceWho, 200, 278, 117);
		if (counter == 0)
			displayFctFullScreen();
	}

	int16 charId = chooseCharacterToReplace();
	for (int16 counter = 0; counter < 2; ++counter) {
		drawColoredRect(200, 112, 278, 132, 0);
		if (counter == 0)
			displayFctFullScreen();
	}

	if (charId == 0x1B) // Escape Keycode
		return -1;

	removeCharacterFromTeam(charId);
	return 2;
}

void EfhEngine::drawMapWindow() {
	drawColoredRect(128, 8, 303, 135, 0);
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
	int16 var_114 = getStringWidth(stringToDisplay);
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
						displayStringAtTextPos(dest);

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
				_word2C880 = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x01:
			if (argC != 0) {
				_largeMapFlag = true;
				_oldMapPosX = _mapPosX = _techDataId_MapPosX;
				_oldMapPosY = _mapPosY = _techDataId_MapPosY;
				_word2C880 = true;
				_redrawNeededFl = true;
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
				_word2C880 = true;
				_redrawNeededFl = true;
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
				_word2C880 = true;
				_redrawNeededFl = true;
			}
			break;
		case 0x04:
			buffer = script_readNumberArray(buffer, 2, scriptNumberArray);
			if (argC != 0) {
				_mapPosX = scriptNumberArray[0];
				_mapPosY = scriptNumberArray[1];
				_word2C880 = true;
				_redrawNeededFl = true;
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
				_redrawNeededFl = true;
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
					var110 = sub1C219((char *)"Nothing...", 1, 2, true);
					displayFctFullScreen();
				} else {
					copyString(_npcBuf[_teamCharId[counter]]._name, _ennemyNamePt2);
					copyString(_items[var110]._name, _nameBuffer);
					sprintf(dest, "%s finds a %s!", _ennemyNamePt2, _nameBuffer);
					drawMapWindow();
					displayFctFullScreen();
					drawMapWindow();
					var110 = sub1C219(dest, 1, 2, true);
					displayFctFullScreen();
				}

				var110 = sub151FD(_mapPosX, _mapPosY);
				if (var110 != -1) {
					_mapUnknownPtr[var110 * 9 + 1] = 0xFF;
				}
				_redrawNeededFl = true;
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
		displayStringAtTextPos(dest);

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

void EfhEngine::displayGameScreen() {
	clearScreen(0);
	drawUpperLeftBorders();
	drawUpperRightBorders();
	drawBottomBorders();
	displayAnimFrame();
	displayLowStatusScreen(false);
}

void EfhEngine::sub221FA(uint8 *impArray, bool flag) {
	for (uint8 counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			drawColoredRect(16, 115, 111, 133, 0);
			if (impArray != nullptr) {
				_word2C86E = 4;
				_dword2C856 = impArray;
				sub133E5(impArray, 17, 115, 110, 133, 0);
			}
		}
	}
}

void EfhEngine::drawUpperLeftBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[0], 0, 0);
	displayRawDataAtPos(_circleImageSubFileArray[1], 112, 0);
	displayRawDataAtPos(_circleImageSubFileArray[3], 16, 0);
}

void EfhEngine::drawUpperRightBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[2], 304, 0);
	displayRawDataAtPos(_circleImageSubFileArray[4], 128, 0);
}

void EfhEngine::drawBottomBorders() {
	displayRawDataAtPos(_circleImageSubFileArray[7], 16, 136);
	displayRawDataAtPos(_circleImageSubFileArray[8], 16, 192);
	displayRawDataAtPos(_circleImageSubFileArray[5], 0, 136);
	displayRawDataAtPos(_circleImageSubFileArray[6], 304, 136);
}

void EfhEngine::sub15A28(int16 arg0, int16 arg2) {
	warning("STUB: sub15A28");
}

void EfhEngine::sub2455E(int16 arg0, int16 arg2, int16 arg4) {
	warning("TODO: sub2455E - check behavior");
	uint8 varD = kByte2C7D0[arg0];
	int16 varC = arg2 - 11;
	int16 varA = arg4 - 11;

	if (varC < 0)
		varC = 0;

	if (varA < 0)
		varA = 0;

	int16 var8 = varC + 23;
	int16 var6 = varA + 23;

	for (int16 var4 = varC; var4 <= var8; ++var4) {
		for (int16 var2 = varA; var2 <= var6; ++var2) {
			_techData[var2 + var4 * 64] = varD;
		}
	}
}

int16 EfhEngine::sub1C219(char *str, int menuType, int arg4, bool displayTeamWindowFl) {
	int16 varA = 0xFF;
	int16 minX, maxX, minY, maxY;
	
	switch (menuType) {
	case 0:
		minX = 129;
		minY = 9;
		maxX = 302;
		maxY = 18;
		break;
	case 1:
		minX = 129;
		minY = 9;
		maxX = 302;
		maxY = 110;
		break;
	case 2:
		minX = 129;
		minY = 112;
		maxX = 302;
		maxY = 132;
		break;
	case 3:
		minX = 129;
		minY = 79;
		maxX = 303;
		maxY = 107;
		break;
	default:
		minX = minY = 0;
		maxX = 320;
		maxY = 200;
		break;
	}

	drawColoredRect(minX, maxX, minY, maxY, 0);
	if (str)
		varA = script_parse((uint8 *)str, minX, minY, maxX, maxY, -1);

	if (displayTeamWindowFl)
		displayLowStatusScreen(false);

	if (arg4 != 0) {
		displayFctFullScreen();
		if (_word2C87A != 0)
			_word2C87A = 0;
		else {
			drawColoredRect(minX, maxX, minY, maxY, 0);
			if (str)
				int16 varC = script_parse((uint8 *)str, minX, minY, maxX, maxY, -1);
		}

		if (displayTeamWindowFl)
			displayLowStatusScreen(false);

		if (arg4 >= 2)
			int16 varC = getLastCharAfterAnimCount(_guessAnimationAmount);

		if (arg4 == 3)
			drawColoredRect(minX, maxX, minY, maxY, 0);
	}

	return varA;
}

int16 EfhEngine::sub151FD(int16 posX, int16 posY) {
	if (_largeMapFlag) {
		for (int16 counter = 0; counter < 100; ++counter) {
			if (_mapUnknownPtr[counter * 9 + 1] == posX && _mapUnknownPtr[counter * 9 + 2] == posY && _mapUnknownPtr[counter * 9] == 0xFE)
				return counter;
		}
	} else {
		for (int16 counter = 0; counter < 100; ++counter) {
			if (_mapUnknownPtr[counter * 9 + 1] == posX && _mapUnknownPtr[counter * 9 + 2] == posY && _mapUnknownPtr[counter * 9] == _fullPlaceId)
				return counter;
		}
	}
	return -1;
}

void EfhEngine::drawChar(uint8 curChar, int16 posX, int posY) {
	// Quick hacked display, may require rework
	uint8 *destPtr = (uint8 *)_mainSurface->getBasePtr(posX, posY);

	int16 charId = curChar - 0x20;
	uint8 width = _fontDescr._widthArray[charId];

	for (int16 line = 0; line < 8; ++line) {
		int16 x = 0;
		for (int i = 7; i >= 7 - width; --i) {
			if (_fontDescr._fontData[charId]._lines[line] & (1 << i))
				destPtr[320 * line + x] = _textColor;
			++x;
		}
	}	
}

void EfhEngine::setTextColorWhite() {
	if (_videoMode == 8) // CGA
		_textColor = 0x3;
	else
		_textColor = 0xF;
}

void EfhEngine::setTextColorRed() {
	if (_videoMode == 8) // CGA
		_textColor = 0x2;
	else
		_textColor = 0xC;
}

void EfhEngine::setTextColorGrey() {
	if (_videoMode == 8) // CGA
		_textColor = 0x1;
	else
		_textColor = 0x8;
}

bool EfhEngine::isPosOutOfMap(int16 mapPosX, int16 mapPosY) {
	int16 maxMapBlocks = _largeMapFlag ? 63 : 23;

	if (mapPosX == 0 && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	if (mapPosX == maxMapBlocks && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	return false;
}

void EfhEngine::goSouth() {
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
	if (--_mapPosY < 0)
		_mapPosY = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goEast() {
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
	if (--_mapPosX < 0)
		_mapPosX = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goNorthEast() {
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
	if (_largeMapFlag) {
		if (++_mapPosX > 63)
			_mapPosX = 63;
	} else {
		if (++_mapPosX > 23)
			_mapPosX = 23;
	}

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

void EfhEngine::goNorthWest() {
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
	if (--_mapPosX < 0)
		_mapPosX = 0;

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

void EfhEngine::handleNewRoundEffects() {
	static int16 regenCounter = 0;

	if (!_word2C8D7)
		return;

	for (int16 counter = 0; counter < _teamSize; ++counter) {
		if (_teamCharStatus[counter]._status == 0) // normal
			continue;
		if (--_teamCharStatus[counter]._duration <= 0) {
			_teamCharStatus[counter]._status = 0;
			_teamCharStatus[counter]._duration = 0;
		}
	}

	if (++regenCounter <= 8)
		return;

	for (int16 counter = 0; counter < _teamSize; ++counter) {
		if (++_npcBuf[_teamCharId[counter]]._hitPoints > _npcBuf[_teamCharId[counter]]._maxHP)
			_npcBuf[_teamCharId[counter]]._hitPoints = _npcBuf[_teamCharId[counter]]._maxHP;
	}
	regenCounter = 0;
}

bool EfhEngine::handleDeathMenu() {
	warning("STUB: handleDeathMenu");
	return false;
}

void EfhEngine::setNumLock() {
	// No implementation in ScummVM
}

void EfhEngine::computeMapAnimation() {
	const int16 maxMapBlocks = _largeMapFlag ? 63 : 23;

	int16 minMapX = _mapPosX - 5;
	int16 minMapY = _mapPosY - 4;

	if (minMapX < 0)
		minMapX = 0;
	if (minMapY < 0)
		minMapY = 0;

	int16 maxMapX = minMapX + 10;
	int16 maxMapY = minMapY + 7;

	if (maxMapX > maxMapBlocks)
		maxMapX = maxMapBlocks;
	if (maxMapY > maxMapBlocks)
		maxMapY = maxMapBlocks;

	for (int16 counterY = minMapY; counterY < maxMapY; ++counterY) {
		for (int16 counterX = minMapX; counterX < maxMapX; ++counterX) {
			if (_largeMapFlag) {
				if (_currentTileBankImageSetId[0] != 0)
					continue;
				uint8 var4 = _mapGameMapPtr[counterX * 64 + counterY];
				if (var4 >= 1 && var4 <= 0xF) {
					if (getRandom(100) < 50)
						_mapGameMapPtr[counterX * 64 + counterY] += 0xC5;
				} else if (var4 >= 0xC6 && var4 <= 0xD5) {
					if (getRandom(100) < 50)
						_mapGameMapPtr[counterX * 64 + counterY] -= 0xC5;
				}
			} else {
				if (_currentTileBankImageSetId[0] != 0)
					continue;
				uint8 var4 = _curPlace[counterX * 24 + counterY];
				if (var4 >= 1 && var4 <= 0xF) {
					if (getRandom(100) < 50)
						_curPlace[counterX * 24 + counterY] += 0xC5;
				} else if (var4 >= 0xC6 && var4 <= 0xD5) {
					if (getRandom(100) < 50)
						_curPlace[counterX * 24 + counterY] -= 0xC5;
				}
			}
		}
	}
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

	computeMapAnimation();
}

void EfhEngine::setNextCharacterPos() {
	if (_textPosX <= 311)
		return;

	_textPosX = 0;
	_textPosY += 8;

	if (_textPosY > 191)
		_textPosY = 0;
}

void EfhEngine::displayStringAtTextPos(const char *message) {
	drawString(message, _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(message) + 1;
	setNextCharacterPos();
}

void EfhEngine::unkFct_displayMenuBox_2(int16 color) {
	drawColoredRect(16, 152, 302, 189, color);
}

int8 EfhEngine::sub16B08(int16 monsterId) {
	// Simplified version compared to the original
	int16 maxSize = _largeMapFlag ? 63 : 23;
	if (_mapMonsters[monsterId]._posX < 0 || _mapMonsters[monsterId]._posY < 0 || _mapMonsters[monsterId]._posX > maxSize || _mapMonsters[monsterId]._posY > maxSize)
		return 0;

	if (_mapMonsters[monsterId]._posX == _mapPosX && _mapMonsters[monsterId]._posY == _mapPosY)
		return 0;

	for (int16 counter = 0; counter < 64; ++counter) {
		if (counter == monsterId)
			continue;

		if (!checkPictureRefAvailability(counter))
			continue;

		if (_mapMonsters[monsterId]._guess_fullPlaceId == _mapMonsters[counter]._guess_fullPlaceId
		 && _mapMonsters[monsterId]._posX == _mapMonsters[counter]._posX
		 && _mapMonsters[monsterId]._posY == _mapMonsters[counter]._posY)
			return 0;
	}

	return sub15581(_mapMonsters[monsterId]._posX, _mapMonsters[monsterId]._posY, 0);
}

bool EfhEngine::moveMonsterAwayFromTeam(int16 monsterId) {
	if (_mapMonsters[monsterId]._posX < _mapPosX) {
		--_mapMonsters[monsterId]._posX;
		if (_mapMonsters[monsterId]._posY < _mapPosY)
			--_mapMonsters[monsterId]._posY;
		else if (_mapMonsters[monsterId]._posY > _mapPosY)
			++_mapMonsters[monsterId]._posY;

		return true;
	}

	if (_mapMonsters[monsterId]._posX > _mapPosX) {
		++_mapMonsters[monsterId]._posX;
		if (_mapMonsters[monsterId]._posY < _mapPosY)
			--_mapMonsters[monsterId]._posY;
		else if (_mapMonsters[monsterId]._posY > _mapPosY)
			++_mapMonsters[monsterId]._posY;

			return true;
	}

	// Original checks for posX equality, which is the only possible option at this point => skipped
	if (_mapMonsters[monsterId]._posY < _mapPosY)
		--_mapMonsters[monsterId]._posY;
	else if (_mapMonsters[monsterId]._posY > _mapPosY)
		++_mapMonsters[monsterId]._posY;
	else
		return false;

	return true;
}

bool EfhEngine::moveMonsterTowardsTeam(int16 monsterId) {
	if (_mapMonsters[monsterId]._posX < _mapPosX) {
		++_mapMonsters[monsterId]._posX;
		if (_mapMonsters[monsterId]._posY < _mapPosY)
			++_mapMonsters[monsterId]._posY;
		else if (_mapMonsters[monsterId]._posY > _mapPosY)
			--_mapMonsters[monsterId]._posY;

		return true;
	}

	if (_mapMonsters[monsterId]._posX > _mapPosX) {
		--_mapMonsters[monsterId]._posX;
		if (_mapMonsters[monsterId]._posY < _mapPosY)
			++_mapMonsters[monsterId]._posY;
		else if (_mapMonsters[monsterId]._posY > _mapPosY)
			--_mapMonsters[monsterId]._posY;

		return true;
	}

	// Original checks for posX equality, which is the only possible option at this point => skipped
	if (_mapMonsters[monsterId]._posY < _mapPosY)
		++_mapMonsters[monsterId]._posY;
	else if (_mapMonsters[monsterId]._posY > _mapPosY)
		--_mapMonsters[monsterId]._posY;
	else
		return false;

	return true;
}

bool EfhEngine::moveMonsterGroupOther(int16 monsterId, int16 direction) {

	switch (direction - 1) {
	case 0:
		--_mapMonsters[monsterId]._posY;
		return true;
	case 1:
		--_mapMonsters[monsterId]._posY;
		++_mapMonsters[monsterId]._posX;
		return true;
	case 2:
		++_mapMonsters[monsterId]._posX;
		return true;
	case 3:
		++_mapMonsters[monsterId]._posX;
		++_mapMonsters[monsterId]._posY;
		return true;
	case 4:
		++_mapMonsters[monsterId]._posY;
		return true;
	case 5:
		++_mapMonsters[monsterId]._posY;
		--_mapMonsters[monsterId]._posX;
		return true;
	case 6:
		--_mapMonsters[monsterId]._posX;
		return true;
	case 7:
		--_mapMonsters[monsterId]._posX;
		--_mapMonsters[monsterId]._posY;
		return true;
	default:
		return false;
	}
}

bool EfhEngine::moveMonsterGroup(int16 monsterId) {
	int16 rand100 = getRandom(100);

	if (rand100 < 30)
		return moveMonsterTowardsTeam(monsterId);

	if (rand100 >= 60)
		// CHECKME: the original seems to only use 1 param??
		return moveMonsterGroupOther(monsterId, getRandom(8));

	return moveMonsterAwayFromTeam(monsterId);
}

int16 EfhEngine::computeMonsterGroupDistance(int monsterId) {
	int16 monsterPosX = _mapMonsters[monsterId]._posX;
	int16 monsterPosY = _mapMonsters[monsterId]._posY;

	int16 deltaX = monsterPosX - _mapPosX;
	int16 deltaY = monsterPosY - _mapPosY;

	return (int16)sqrt(deltaX * deltaX + deltaY * deltaY);
}

bool EfhEngine::checkWeaponRange(int16 monsterId, int weaponId) {
	static const int16 kRange[5] = {1, 2, 3, 3, 3};

	assert(_items[weaponId]._range < 5);
	if (computeMonsterGroupDistance(monsterId) > kRange[_items[weaponId]._range])
		return false;

	return true;
}

bool EfhEngine::unkFct_checkMonsterField8(int id, bool teamFlag) {
	int16 monsterId = id;
	if (teamFlag)
		monsterId = _teamMonsterIdArray[id];

	if ((_mapMonsters[monsterId]._field_8 & 0xF) >= 8)
		return true;

	if (_unkArray2C8AA[0] == 0)
		return false;

	if ((_mapMonsters[monsterId]._field_8 & 0x80) != 0)
		return true;

	return false;
}

bool EfhEngine::checkTeamWeaponRange(int16 monsterId) {
	if (!_word2D0BC)
		return true;

	for (int16 counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId && unkFct_checkMonsterField8(monsterId, false) && checkWeaponRange(monsterId, _mapMonsters[monsterId]._itemId_Weapon))
			return false;
	}

	return true;
}

bool EfhEngine::checkIfMonsterOnSameLargelMapPlace(int16 monsterId) {
	if (_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == 0xFE)
		return true;

	if (!_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == _fullPlaceId)
		return true;

	return false;
}

bool EfhEngine::checkMonsterWeaponRange(int16 monsterId) {
	return checkWeaponRange(monsterId, _mapMonsters[monsterId]._itemId_Weapon);
}

void EfhEngine::sub174A0() {
	static int16 sub174A0_monsterPosX = -1;
	static int16 sub174A0_monsterPosY = -1;
	
	int16 var14 = 0;
	int16 var6 = 0;
	_redrawNeededFl = true;
	int16 unkMonsterId = -1;
	int16 mapSize = _largeMapFlag ? 63 : 23;
	int16 minDisplayedMapX = CLIP<int16>(_mapPosX - 10, 0, mapSize);
	int16 minDisplayedMapY = CLIP<int16>(_mapPosY - 9, 0, mapSize);
	int16 maxDisplayedMapX = CLIP<int16>(minDisplayedMapX + 20, 0, mapSize);
	int16 maxDisplayedMapY = CLIP<int16>(minDisplayedMapY + 17, 0, mapSize);
	
	for (int16 monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkPictureRefAvailability(monsterId))
			continue;

		if (!checkTeamWeaponRange(monsterId))
			continue;

		if (!checkIfMonsterOnSameLargelMapPlace(monsterId))
			continue;

		int16 var4 = _mapMonsters[monsterId]._posX;
		int16 var2 = _mapMonsters[monsterId]._posY;

		if (var4 < minDisplayedMapX || var4 > maxDisplayedMapX || var2 < minDisplayedMapY || var2 > maxDisplayedMapY)
			continue;

		bool var1A = false;
		var14 = 0;

		sub174A0_monsterPosX = _mapMonsters[monsterId]._posX;
		sub174A0_monsterPosY = _mapMonsters[monsterId]._posY;
		int8 var1C = _mapMonsters[monsterId]._field_8 & 0xF;

		if (_unkArray2C8AA[0] != 0 && (_mapMonsters[monsterId]._field_8 & 0x80))
			var1C = 9;

		int16 var1E = _mapMonsters[monsterId]._field_8 & 0x70;
		var1E >>= 4;

		int16 var16 = var1E;
		do {
			switch (var1C - 1) {
			case 0:
				if (getRandom(100) >= 0xE - var1E)
					var1A = moveMonsterTowardsTeam(monsterId);
				else
					var1A = moveMonsterGroup(monsterId);
				break;
			case 1:
				if (getRandom(100) >= 0xE - var1E)
					var1A = moveMonsterAwayFromTeam(monsterId);
				else
					var1A = moveMonsterGroup(monsterId);
				break;
			case 2:
				var1A = moveMonsterGroupOther(monsterId, getRandom(8));
				break;
			case 3:
				var1A = moveMonsterGroup(monsterId);
				break;
			case 4:
				if (getRandom(100) > 0x32 - var1E)
					var1A = moveMonsterTowardsTeam(monsterId);
				else
					var1A = moveMonsterGroup(monsterId);
				break;
			case 5:
				if (getRandom(100) > 0x32 - var1E)
					var1A = moveMonsterAwayFromTeam(monsterId);
				else
					var1A = moveMonsterGroup(monsterId);
				break;
			case 6:
				if (getRandom(100) >= 0x32 - var1E)
					var1A = moveMonsterGroup(monsterId);
				break;
			case 7:
				// var14 is not a typo.
				var14 = checkMonsterWeaponRange(monsterId);
				break;
			case 8:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					if (getRandom(100) >= 0xE - var1E)
						var1A = moveMonsterTowardsTeam(monsterId);
					else
						var1A = moveMonsterGroup(monsterId);
				}
				break;
			case 9:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					if (getRandom(100) >= 0xE - var1E)
						var1A = moveMonsterAwayFromTeam(monsterId);
					else
						var1A = moveMonsterGroup(monsterId);
				}
				break;
			case 10:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					var1A = moveMonsterGroupOther(monsterId, getRandom(8));
				}
				break;
			case 11:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					var1A = moveMonsterGroup(monsterId);
				}
				break;
			case 12:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					if (getRandom(100) >= 0x32 - var1E)
						var1A = moveMonsterTowardsTeam(monsterId);
					else
						var1A = moveMonsterGroup(monsterId);
				}
				break;
			case 13:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0) {
					if (getRandom(100) >= 0x32 - var1E)
						var1A = moveMonsterAwayFromTeam(monsterId);
					else
						var1A = moveMonsterGroup(monsterId);
				}
				break;
			case 14:
				var14 = checkMonsterWeaponRange(monsterId);
				if (var14 == 0 && getRandom(100) >= 0x32 - var1E)
					var1A = moveMonsterGroup(monsterId);
				break;
			default:
				break;
			}

			for (;;) {
				if (!var1A) {
					if (var14 == 0) {
						var1A = true;
					} else {
						unkMonsterId = monsterId;
						var1A = true;
					}
				} else {
					int8 var18 = sub16B08(monsterId);

					if (var18 == 0) {
						_mapMonsters[monsterId]._posX = sub174A0_monsterPosX;
						_mapMonsters[monsterId]._posY = sub174A0_monsterPosY;
						var1A = false;
						--var16;
					} else if (var18 == 2) {
						_mapMonsters[monsterId]._posX = sub174A0_monsterPosX;
						_mapMonsters[monsterId]._posY = sub174A0_monsterPosY;
					}
				}

				if (!var1A && var16 == 1 && var1E > 1) {
					var1A = moveMonsterGroupOther(monsterId, getRandom(8));
					continue;
				}
				
				break;
			}
		} while (!var1A && var16 > 0);
	}

	if (unkMonsterId != -1)
		handleFight(unkMonsterId);
}

bool EfhEngine::checkPictureRefAvailability(int16 monsterId) {
	if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
		return false;

	for (int16 counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[monsterId]._pictureRef[counter] > 0)
			return true;
	}

	return false;
}

void EfhEngine::displayMonsterAnim(int16 monsterId) {
	int16 animId = kEncounters[_mapMonsters[monsterId]._MonsterRef]._animId;
	displayAnimFrames(animId, true);
}

int16 EfhEngine::countPictureRef(int16 id, bool teamMemberFl) {
	int16 count = 0;
	int16 monsterId;

	if (teamMemberFl)
		monsterId = _teamMonsterIdArray[id];
	else
		monsterId = id;

	for (int16 counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[monsterId]._pictureRef[counter] > 0)
			++count;
	}

	return count;
}

bool EfhEngine::checkMonsterGroupDistance1OrLess(int16 monsterId) {
	if (computeMonsterGroupDistance(monsterId) > 1)
		return false;

	return true;
}

bool EfhEngine::sub21820(int16 monsterId, int16 arg2, int16 itemId) {
	char buffer[80];
	memset(buffer, 0, 80);

	int8 var51 = _mapMonsters[monsterId]._possessivePronounSHL6;
	if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
		return false;

	if (countPictureRef(monsterId, false) < 1)
		return false;

	if (!checkIfMonsterOnSameLargelMapPlace(monsterId))
		return false;

	if (!checkMonsterGroupDistance1OrLess(monsterId))
		return false;

	if (var51 != 0x3F) {
		if (_mapMonsters[monsterId]._field_9 == 0xFF || arg2 != 5) {
			return false;
		}
		displayMonsterAnim(monsterId);
		sub22AA8(_mapMonsters[monsterId]._field_9);
		displayAnimFrames(0xFE, true);
		return true;
	}

	if (isCharacterATeamMember(_mapMonsters[monsterId]._field_1))
		return false;

	int16 var58 = isCharacterATeamMember(_mapMonsters[monsterId]._field_1);
	switch (_npcBuf[var58].field_10 - 0xEE) {
	case 0:
		if (arg2 == 4 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			sub22AA8(_npcBuf[var58].field_14);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 1:
		if (arg2 == 2 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			sub22AA8(_npcBuf[var58].field_14);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 2:
		if (arg2 == 1 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			sub22AA8(_npcBuf[var58].field_14);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 3:
		if (_history[_npcBuf[var58].field_11] != 0) {
			displayMonsterAnim(monsterId);
			sub22AA8(_npcBuf[var58].field_14);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 4:
		for (int16 counter = 0; counter < _teamSize; ++counter) {
			for (int16 charId = 0; charId < 10; ++charId) {
				if (_npcBuf[_teamCharId[counter]]._inventory[charId]._ref == _npcBuf[var58].field_11) {
					removeObject(_teamCharId[counter], charId);
					displayMonsterAnim(monsterId);
					sub22AA8(_npcBuf[var58].field_14);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 5:
		if (arg2 == 2 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			sub22AA8(_npcBuf[var58].field_14);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 6:
		for (int16 counter = 0; counter < _teamSize; ++counter) {
			for (int16 charId = 0; charId < 10; ++charId) {
				if (_npcBuf[_teamCharId[counter]]._inventory[charId]._ref == _npcBuf[var58].field_11) {
					displayMonsterAnim(monsterId);
					sub22AA8(_npcBuf[var58].field_14);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 7:
		for (int16 counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				removeCharacterFromTeam(counter);
				displayMonsterAnim(monsterId);
				sub22AA8(_npcBuf[var58].field_14);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 8:
		for (int16 counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				displayMonsterAnim(monsterId);
				copyString(_npcBuf[var58]._name, _ennemyNamePt2);
				copyString(_npcBuf[_teamCharId[counter]]._name, _characterNamePt2);
				sprintf(buffer, "%s asks that %s leave your party.", _ennemyNamePt2, _characterNamePt2);
				for (int16 i = 0; i < 2; ++i) {
					unkFct_displayMenuBox_2(0);
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
					removeCharacterFromTeam(counter);
					sub22AA8(_npcBuf[var58].field_14);
				}
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 9:
		for (int16 counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				displayMonsterAnim(monsterId);
				sub22AA8(_npcBuf[var58].field_14);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 16:
		displayMonsterAnim(monsterId);
		sub22AA8(_npcBuf[var58].field_14);
		displayAnimFrames(0xFE, true);
		return true;
	default:
		break;
	}

	if (_npcBuf[var58].field_12 == 0x7FFF || arg2 != 5)
		return false;

	displayMonsterAnim(monsterId);
	sub22AA8(_npcBuf[var58].field_12);
	displayAnimFrames(0xFE, true);
	return true;
}

void EfhEngine::sub221D2(int16 monsterId) {
	if (monsterId != -1) {
		_dword2C856 = nullptr;
		sub21820(monsterId, 5, -1);
	}
}

void EfhEngine::sub22AA8(int16 arg0) {
	warning("STUB: sub22AA8");
}

bool EfhEngine::sub22293(int16 mapPosX, int16 mapPosY, int16 charId, int16 itemId, int16 arg8, int16 imageSetId) {
	int16 var8 = sub151FD(mapPosX, mapPosY);

	if (var8 == -1) {
		if (imageSetId != -1 && *_imp2PtrArray[imageSetId] != 0x30)
			sub221FA(_imp2PtrArray[imageSetId], true);
	} else if (var8 == 0) {
		if (_mapUnknownPtr[var8 * 9 + 3] == 0xFF) {
			sub22AA8(_mapUnknownPtr[var8 * 9 + 5]); // word!
			return true;
		} else if (_mapUnknownPtr[var8 * 9 + 3] == 0xFE) {
			for (int16 counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;
				if (_teamCharId[counter] == _mapUnknownPtr[var8 * 9 + 4]) {
					sub22AA8(_mapUnknownPtr[var8 * 9 + 5]);
					return true;
				}
			}
		} else if (_mapUnknownPtr[var8 * 9 + 3] == 0xFD) {
			for (int16 counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (int16 var2 = 0; var2 < 10; ++var2) {
					if (_npcBuf[_teamCharId[counter]]._inventory[var2]._ref == _mapUnknownPtr[var8 * 9 + 4]) {
						sub22AA8(_mapUnknownPtr[var8 * 9 + 5]);
						return true;
					}
				}
			}
		// original makes a useless check on (_mapUnknownPtr[var8 * 9 + 3] > 0x7F)
		} else if (_mapUnknownPtr[var8 * 9 + 3] <= 0x77) {
			int16 var6 = _mapUnknownPtr[var8 * 9 + 3];
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (int16 var2 = 0; var2 < 39; ++var2) {
					if (_npcBuf[_teamCharId[counter]]._activeScore[var2] >= _mapUnknownPtr[var8 * 9 + 4]) {
						sub22AA8(_mapUnknownPtr[var8 * 9 + 5]);
						return true;
					}
				}
			}
		}
	} else {
		if ((_mapUnknownPtr[var8 * 9 + 3] == 0xFA && arg8 == 1)
		||  (_mapUnknownPtr[var8 * 9 + 3] == 0xFC && arg8 == 2)
		||  (_mapUnknownPtr[var8 * 9 + 3] == 0xFB && arg8 == 3)) {
			if (_mapUnknownPtr[var8 * 9 + 4] == itemId) {
				sub22AA8(_mapUnknownPtr[var8 * 9 + 5]);
				return true;
			}
		} else if (arg8 == 4) {
			int16 var6 = _mapUnknownPtr[var8 * 9 + 3];
			if (var6 >= 0x7B && var6 <= 0xEF) {
				var6 -= 0x78;
				if (var6 >= 0 && var6 <= 0x8B && var6 == itemId && _mapUnknownPtr[var8 * 9 + 4] <= _npcBuf[charId]._activeScore[itemId]) {
					sub22AA8(_mapUnknownPtr[var8 * 9 + 5]);
					return true;
				}
			}		
		}
	}

	for (int16 counter = 0; counter < 64; ++counter) {
		if (!sub21820(counter, arg8, itemId))
			return true;
	}

	if ((arg8 == 4 && _mapUnknownPtr[var8 * 9 + 3] < 0xFA) || arg8 != 4) {
		if (_mapUnknownPtr[var8 * 9 + 7] > 0xFE) // word!!
			return false;
		sub22AA8(_mapUnknownPtr[var8 * 9 + 7]);
		return true;		
	} else
		return false;

	return false;
}

int8 EfhEngine::sub15581(int16 mapPosX, int16 mapPosY, int16 arg4) {
	int16 curTileInfo = getMapTileInfo(mapPosX, mapPosY);
	int16 imageSetId = _currentTileBankImageSetId[curTileInfo / 72];
	imageSetId *= 72;
	imageSetId += curTileInfo % 72;

	if (arg4 == 1 && _word2C8D7) {
		int16 var2 = sub22293(mapPosX, mapPosY, -1, 0x7FFF, 0, imageSetId);
	}

	if (_word2C880) {
		_word2C880 = false;
		return -1;
	}
	if (_tileFact[imageSetId * 2 + 1] != 0xFF && !_word2C8D5) {
		if ((arg4 == 1 && _word2C8D7) || (arg4 == 0 && _word2C8D7 && imageSetId != 128 && imageSetId != 121)) {
			if (_largeMapFlag) {
				_mapGameMapPtr[mapPosX * 64 + mapPosY] = _tileFact[imageSetId * 2 + 1];
			} else {
				_curPlace[mapPosX * 24 + mapPosY] = _tileFact[imageSetId * 2 + 1];
			}

			_redrawNeededFl = true;
			if (_tileFact[imageSetId * 2] == 0)
				return 2;
			return 1;
		}
	}

	return _tileFact[imageSetId * 2];
}

bool EfhEngine::handleFight(int16 monsterId) {
	warning("STUB - handleFight");
	return false;
}

void EfhEngine::displayMenuItemString(int16 menuBoxId, int thisBoxId, int minX, int maxX, int minY, const char *str) {
	char buffer[20];
	memset(buffer, 0, 20);

	if (menuBoxId == thisBoxId) {
		if (_menuDepth == 0)
			setTextColorWhite();
		else
			setTextColorGrey();

		sprintf(buffer, "> %s <", str);
		displayCenteredString(buffer, minX, maxX, minY);
		setTextColorRed();
	} else {
		if (_menuDepth == 0)
			setTextColorRed();
		else
			setTextColorGrey();

		displayCenteredString(str, minX, maxX, minY);
	}
}

void EfhEngine::displayStatusMenu(int16 windowId) {
	for (int16 counter = 0; counter < 9; ++counter) {
		drawColoredRect(80, 39 + 14 * counter, 134, 47 + 14 * counter, 0);
	}

	if (_menuDepth != 0)
		setTextColorGrey();

	displayMenuItemString(windowId, 0, 80, 134, 39, "EQUIP");
	displayMenuItemString(windowId, 1, 80, 134, 53, "USE");
	displayMenuItemString(windowId, 2, 80, 134, 67, "GIVE");
	displayMenuItemString(windowId, 3, 80, 134, 81, "TRADE");
	displayMenuItemString(windowId, 4, 80, 134, 95, "DROP");
	displayMenuItemString(windowId, 5, 80, 134, 109, "INFO.");
	displayMenuItemString(windowId, 6, 80, 134, 123, "PASSIVE");
	displayMenuItemString(windowId, 7, 80, 134, 137, "ACTIVE");
	displayMenuItemString(windowId, 8, 80, 134, 151, "LEAVE");

	setTextColorRed();
}

void EfhEngine::countRightWindowItems(int16 menuId, int16 charId) {
	int16 var2 = 0;
	int16 var4 = 0;
	_word2D0BA = 0;

	switch (menuId) {
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 9:
		var4 = -1;
		break;
	case 5:
		var4 = 26;
		var2 = 36;
		break;
	case 6:
		var4 = 15;
		var2 = 25;
		break;
	case 7:
		var4 = 0;
		var2 = 14;
		break;
	default: // Case 8 + Default
		var4 = -1;
		_word2D0BA = 0;
		break;
	}

	if (var4 == -1) {
		for (int16 counter = 0; counter < 10; ++counter) {
			if (_npcBuf[charId]._inventory[counter]._ref != 0x7FFF) {
				_word3273A[_word2D0BA++] = counter;
			}
		}
	} else {
		for (int16 counter = var4; counter < var2; ++counter) {
			if (_npcBuf[charId]._activeScore[counter] != 0) {
				_word3273A[_word2D0BA++] = counter;
			}
		}
	}
}

int16 EfhEngine::getXPLevel(int32 xp) {
	int16 level = 0;
	int16 var6 = 1500;

	int32 wrkXp = xp;

	while (wrkXp > 0) {
		wrkXp -= var6;
		if (wrkXp >= 0)
			++level;

		var6 += 1500;
		if (var6 > 15000)
			var6 = 15000;
	}

	return level;
}

void EfhEngine::displayChar(char character) {
	char buffer[2];
	buffer[0] = character;
	buffer[1] = 0;

	drawString(buffer, _textPosX, _textPosY, _textColor);
	_textPosX += getStringWidth(buffer) + 1;
	setNextCharacterPos();
}

void EfhEngine::displayCharacterSummary(int16 curMenuLine, int16 npcId) {
	char buffer1[40];
	char buffer2[40];
	memset(buffer1, 0, 40);
	memset(buffer2, 0, 40);
	
	setTextColorRed();
	copyString(_npcBuf[npcId]._name, buffer1);
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "Level: %d", getXPLevel(_npcBuf[npcId]._xp));
	setTextPos(146, 36);
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "XP: %lu", _npcBuf[npcId]._xp);
	setTextPos(227, 36);
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "Speed: %d", _npcBuf[npcId]._speed);
	setTextPos(146, 45);
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "Defense: %d", getEquipmentDefense(npcId, false));
	setTextPos(146, 54);
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "Hit Points: %d", _npcBuf[npcId]._hitPoints);
	setTextPos(146, 63);
	displayStringAtTextPos(buffer1);
	sprintf(buffer1, "Max HP: %d", _npcBuf[npcId]._maxHP);
	setTextPos(227, 63);
	displayStringAtTextPos(buffer1);
	displayCenteredString("Inventory", 144, 310, 72);

	if (_word2D0BA == 0) {
		if (curMenuLine != -1)
			setTextColorWhite();

		displayCenteredString("Nothing Carried", 144, 310, 117);
		setTextColorRed();
		return;
	}

	for (int16 counter = 0; counter < _word2D0BA; ++counter) {
		if (_menuDepth == 0)
			setTextColorGrey();
		else {
			if (counter == curMenuLine)
				setTextColorWhite();
		}
		int16 textPosY = 81 + counter * 9;
		int16 itemId = _npcBuf[npcId]._inventory[_word3273A[counter]]._ref;
		if (itemId != 0x7FFF) {
			if (_npcBuf[npcId]._inventory[_word3273A[counter]]._stat1 & 0x80) {
				setTextPos(146, textPosY);
				displayChar('E');
			}
		}

		setTextPos(152, textPosY);
		if (counter == curMenuLine) {
			sprintf(buffer1, "%c>", 'A' + counter);
		} else {
			sprintf(buffer1, "%c)", 'A' + counter);
		}
		displayStringAtTextPos(buffer1);

		if (itemId != 0x7FFF) {
			setTextPos(168, textPosY);
			copyString(_items[itemId]._name, buffer2);
			sprintf(buffer1, "  %s", buffer2);
			displayStringAtTextPos(buffer1);
			setTextPos(262, textPosY);

			if (_items[itemId]._defense > 0) {
				int16 var54 = _npcBuf[npcId]._inventory[_word3273A[counter]]._stat2;
				if (var54 == 0xFF) {
					// useless?
					var54 = _items[_npcBuf[npcId]._inventory[_word3273A[counter]]._ref]._defense;
				} else {
					sprintf(buffer1, "%d", 1 + var54 / 8);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					displayStringAtTextPos("Def");
				}
			} else if (_items[itemId]._uses != 0x7F) {
				int16 var52 = _npcBuf[npcId]._inventory[_word3273A[counter]]._stat1;
				if (var52 != 0x7F) {
					sprintf(buffer1, "%d", var52);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					if (var52 == 1)
						displayStringAtTextPos("Use");
					else
						displayStringAtTextPos("Uses");
				}
			}
		}
		setTextColorRed();
	}
}

void EfhEngine::displayCharacterInformationOrSkills(int16 curMenuLine, int16 charId) {
	char buffer[40];
	memset(buffer, 0, 40);

	setTextColorRed();
	copyString(_npcBuf[charId]._name, buffer);
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer);
	if (_word2D0BA <= 0) {
		if (curMenuLine != -1)
			setTextColorWhite();
		displayCenteredString("No Skills To Select", 144, 310, 96);
		setTextColorRed();
		return;
	}

	for (int16 counter = 0; counter < _word2D0BA; ++counter) {
		if (counter == curMenuLine)
			setTextColorWhite();
		int16 textPosY = 38 + counter * 9;
		setTextPos(146, textPosY);
		if (counter == curMenuLine) {
			sprintf(buffer, "%c>", 'A' + counter);
		} else {
			sprintf(buffer, "%c)", 'A' + counter);
		}

		displayStringAtTextPos(buffer);
		setTextPos(163, textPosY);
		displayStringAtTextPos(kSkillArray[_word3273A[counter]]);
		sprintf(buffer, "%d", _npcBuf[charId]._activeScore[_word3273A[counter]]);
		setTextPos(278, textPosY);
		displayStringAtTextPos(buffer);
		setTextColorRed();
	}
}

void EfhEngine::displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId) {
	drawColoredRect(144, 15, 310, 184, 0);
	displayCenteredString("(ESCape Aborts)", 144, 310, 175);
	_textColor = 0x0E;
	switch (menuId) {
	case 0:
		displayCenteredString("Select Item to Equip", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 1:
		displayCenteredString("Select Item to Use", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 2:
		displayCenteredString("Select Item to Give", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 3:
		displayCenteredString("Select Item to Trade", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 4:
		displayCenteredString("Select Item to Drop", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	case 5:
		displayCenteredString("Character Information", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 6:
		displayCenteredString("Passive Skills", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 7:
		displayCenteredString("Active Skills", 144, 310, 15);
		displayCharacterInformationOrSkills(curMenuLine, npcId);
		break;
	case 8:
	case 9:
		displayCenteredString("Character Summary", 144, 310, 15);
		displayCharacterSummary(curMenuLine, npcId);
		break;
	}
}

void EfhEngine::unk_StatusMenu(int16 windowId, int16 menuId, int16 curMenuLine, int16 charId, bool unusedFl, bool refreshFl) {
	displayStatusMenu(windowId);

	countRightWindowItems(menuId, charId);
	displayStatusMenuActions(menuId, curMenuLine, charId);

	if (refreshFl)
		displayFctFullScreen();
}

void EfhEngine::displayWindow(uint8 *buffer, int16 posX, int16 posY, uint8 *dest) {
	if (buffer == nullptr) {
		warning("Target Buffer Not Defined...DCImage!"); // That's the original message... And yes, it's wrong: it's checking the source buffer :)
		return;
	}

	// Only MCGA handled, the rest is skipped
	uncompressBuffer(buffer, dest);
	displayRawDataAtPos(dest, posX, posY);
	displayFctFullScreen();
	displayRawDataAtPos(dest, posX, posY);
}

void EfhEngine::sub18E80(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	for (int counter = 0; counter < 2; ++counter) {
		displayWindow(_menuBuf, 0, 0, _hiResImageBuf);
		unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, false);

		if (counter == 0)
			displayFctFullScreen();
	}
}

int16 EfhEngine::displayString_3(const char *str, bool animFl, int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	int16 var2 = 0;
	
	for (int16 counter = 0; counter < 2; ++counter) {
		unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, false);
		displayWindow(_windowWithBorderBuf, 19, 113, _hiResImageBuf);

		if (counter == 0) {
			script_parse((uint8 *)str, 28, 122, 105, 166, 0);
			displayFctFullScreen();
		} else {
			var2 = script_parse((uint8 *)str, 28, 122, 105, 166, -1);
		}
	}

	if (animFl) {
		getLastCharAfterAnimCount(_guessAnimationAmount);
		sub18E80(charId, windowId, menuId, curMenuLine);
	}
	
	return var2;
}

bool EfhEngine::isItemCursed(int16 itemId) {
	if (_items[itemId].field_16 == 21 || _items[itemId].field_16 == 22 || _items[itemId].field_16 == 23)
		return true;

	return false;
}

bool EfhEngine::hasObjectEquipped(int16 charId, int16 _objectId) {
	if ((_npcBuf[charId]._inventory[_objectId]._stat1 & 0x80) == 0)
		return false;

	return true;
}

void EfhEngine::equipCursedItem(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (isItemCursed(itemId)) {
		_npcBuf[charId]._inventory[objectId]._stat1 &= 0x7F;
	} else {
		displayString_3("Cursed Item Already Equipped!", true, charId, windowId, menuId, curMenuLine);
	}
	
}

void EfhEngine::sub191FF(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (hasObjectEquipped(charId, objectId)) {
		equipCursedItem(charId, objectId, windowId, menuId, curMenuLine);
	} else {
		int16 var2 = _items[itemId].field_18;
		if (var2 != 4) {
			for (int16 counter = 0; counter < 10; ++counter) {
				if (var2 != _items[_npcBuf[charId]._inventory[counter]._ref].field_18)
					equipCursedItem(charId, objectId, windowId, menuId, curMenuLine);
			}
		}

		_npcBuf[charId]._inventory[objectId]._stat1 |= 0x80;
	}
}

int16 EfhEngine::sub19E2E(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine, int16 argA) {
	warning("STUB: sub19E2E");

	return -1;
}

bool EfhEngine::getValidationFromUser() {
	Common::KeyCode input = handleAndMapInput(true);
	if (input == Common::KEYCODE_y) // or if joystick button 1
		return true;

	return false;
}

int16 EfhEngine::handleStatusMenu(int16 gameMode, int16 charId) {
	int16 menuId = 9;
	int16 var16 = -1;
	int16 windowId = -1;
	int16 curMenuLine = -1;
	int16 var10 = 0;
	int16 var2 = 0;

	saveAnimImageSetId();

	_word2C8D2 = true;
	_menuDepth = 0;

	sub18E80(charId, windowId, menuId, curMenuLine);

	for (;;) {
		if (windowId != -1)
			unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);
		else
			windowId = 0;

		do {
			Common::KeyCode var19 = handleAndMapInput(false);
			if (_menuDepth == 0) {
				switch (var19) {
				case Common::KEYCODE_ESCAPE:
					if (_menuDepth == 0) { // ?? Useless case ?
						windowId = 8;
						var19 = Common::KEYCODE_RETURN;
					}
					break;
				case Common::KEYCODE_a:
					windowId = 7;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_d:
					windowId = 4;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_e:
					windowId = 0;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_g:
					windowId = 2;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_i:
					windowId = 5;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_l:
					windowId = 8;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_p:
					windowId = 6;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_t:
					windowId = 3;
					var19 = Common::KEYCODE_RETURN;
					break;
				case Common::KEYCODE_u:
					windowId = 1;
					var19 = Common::KEYCODE_RETURN;
					break;
				// case 0xFB: Joystick button 2
				default:
				//	warning("handleStatusMenu - unhandled keys (or joystick event?) 0xBA, 0xBB, 0xBC");
					break;
				}
			} else if (_menuDepth == 1) {
				if (var19 >= Common::KEYCODE_a && var19 <= Common::KEYCODE_z) {
					int16 var8 = var19 - Common::KEYCODE_a;
					if (var8 < _word2D0BA) {
						curMenuLine = var8;
						var19 = Common::KEYCODE_RETURN;
					}
				}

			}

			switch (var19) {
			case Common::KEYCODE_RETURN:
			// case 0xFA: Joystick button 1
				if (_menuDepth == 0) {
					menuId = windowId;
					if (menuId > 7)
						var10 = -1;
					else {
						_menuDepth = 1;
						curMenuLine = 0;
					}
				} else if (_menuDepth == 1) {
					if (_word2D0BA == 0) {
						_menuDepth = 0;
						curMenuLine = -1;
						menuId = 9;
						unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);
					} else {
						var16 = curMenuLine;
						var10 = -1;
					}
				}
				break;
			case Common::KEYCODE_ESCAPE:
				_menuDepth = 0;
				curMenuLine = -1;
				menuId = 9;
				unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);
				break;
			case Common::KEYCODE_2:
			case Common::KEYCODE_6:
			// Added for ScummVM
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP2:
			case Common::KEYCODE_KP6:
				// Original checks joystick axis: case 0xCC, 0xCF
				if (_menuDepth == 0) {
					if (++windowId > 8)
						windowId = 0;
				} else if (_menuDepth == 1) {
					if (_word2D0BA != 0) {
						++curMenuLine;
						if (curMenuLine > _word2D0BA - 1)
							curMenuLine = 0;
					}
				}
				break;
			case Common::KEYCODE_4:
			case Common::KEYCODE_8:
			// Added for ScummVM
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP4:
			case Common::KEYCODE_KP8:
			// Original checks joystick axis: case 0xC7, 0xCA
				if (_menuDepth == 0) {
					if (--windowId < 0)
						windowId = 8;
				} else if (_menuDepth == 1) {
					if (_word2D0BA != 0) {
						--curMenuLine;
						if (curMenuLine < 0)
							curMenuLine = _word2D0BA - 1;
					}
				}
				break;
			}

			if (curMenuLine == -1)
				unk_StatusMenu(windowId, menuId, curMenuLine, charId, false, true);
			else
				unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);

		} while (var10 == 0);

		bool validationFl = true;

		int16 objectId;
		int16 itemId;
		switch (menuId) {
		case 0:
			objectId = _word3273A[var16];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sub191FF(charId, objectId, windowId, menuId, curMenuLine);
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_word2C8D2 = false;
				return 0x7D00;
			}
			break;
		case 1:
			objectId = _word3273A[var16];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_word2C8D2 = false;
				return objectId;
			} else {
				if (sub22293(_mapPosX, _mapPosY, charId, itemId, 2, -1)) {
					_word2C8D2 = false;
					return -1;
				} else {
					sub19E2E(charId, objectId, windowId, menuId, curMenuLine, 2);
				}
			}
			break;
		case 2:
			objectId = _word3273A[var16];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayString_3("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else if (hasObjectEquipped(charId, objectId)){
				displayString_3("Item is Equipped!  Give anyway?", false, charId, windowId, menuId, curMenuLine);
				if (!getValidationFromUser())
					validationFl = false;
				sub18E80(charId, windowId, menuId, curMenuLine);

				if (validationFl) {
					if (gameMode == 2) {
						displayString_3("Not a Combat Option !", true, charId, windowId, menuId, curMenuLine);
					} else {
						removeObject(charId, objectId);
						int16 var8 = sub22293(_mapPosX, _mapPosY, charId, itemId, 3, -1);
						if (var8 != 0) {
							_word2C8D2 = false;
							return -1;
						}
					}
				}
			}

			break;
		case 3:
			objectId = _word3273A[var16];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayString_3("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else if (hasObjectEquipped(charId, objectId)) {
				displayString_3("Item is Equipped!  Trade anyway?", false, charId, windowId, menuId, curMenuLine);
				if (!getValidationFromUser())
					validationFl = false;
				sub18E80(charId, windowId, menuId, curMenuLine);

				if (validationFl) {
					int16 var6;
					int16 var8;
					do {
						if (_teamCharId[2] != -1) {
							var8 = displayString_3("Who will you give the item to?", false, charId, windowId, menuId, curMenuLine);
							var2 = 0;
						} else if (_teamCharId[1]) {
							var8 = 0x1A;
							var2 = 0;
						} else {
							var2 = -1;
							if (_teamCharId[0] == charId)
								var8 = 1;
							else
								var8 = 0;
						}

						if (var8 != 0x1A && var8 != 0x1B) {
							var6 = giveItemTo(_teamCharId[var8], objectId, charId);
							if (var6 == 0) {
								displayString_3("That character cannot carry anymore!", false, charId, windowId, menuId, curMenuLine);
								getLastCharAfterAnimCount(_guessAnimationAmount);
							}
						} else {
							if (var8 == 0x1A) {
								displayString_3("No one to trade with!", false, charId, windowId, menuId, curMenuLine);
								getLastCharAfterAnimCount(_guessAnimationAmount);
								var8 = 0x1B;
							}
							var6 = 0;
						}
					} while (var6 == 0 && var2 == 0 && var8 != 0x1B);

					if (var6) {
						removeObject(charId, objectId);
						if (gameMode == 2) {
							restoreAnimImageSetId();
							_word2C8D2 = false;
							return 0x7D00;
						}
					}

					sub18E80(charId, windowId, menuId, curMenuLine);
				}
			}
			break;
		case 4:
			objectId = _word3273A[var16];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayString_3("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else if (hasObjectEquipped(charId, objectId)) {
				displayString_3("Item Is Equipped!  Drop Anyway?", false, charId, windowId, menuId, curMenuLine);
				if (!getValidationFromUser())
					validationFl = false;
				sub18E80(charId, windowId, menuId, curMenuLine);

				if (validationFl) {
					removeObject(charId, objectId);
					if (gameMode == 2) {
						restoreAnimImageSetId();
						_word2C8D2 = false;
						return 0x7D00;
					}

					bool var8 = sub22293(_mapPosX, _mapPosY, charId, itemId, 1, -1);
					if (var8) {
						_word2C8D2 = false;
						return -1;
					}
				}
			}
			break;
		case 5:
			objectId = _word3273A[var16];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_word2C8D2 = false;
					return -1;
				}
			}
			break;
		case 6: // Identical to case 5?
			objectId = _word3273A[var16];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_word2C8D2 = false;
					return -1;
				}
			}
			break;
		case 7: // Identical to case 5?
			objectId = _word3273A[var16];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_word2C8D2 = false;
					return -1;
				}
			}
			break;
		}

		if (menuId != 8) {
			var10 = 0;
			_menuDepth = 0;
			menuId = 9;
			var16 = -1;
			curMenuLine = -1;
		}

		if (menuId == 8) {
			restoreAnimImageSetId();
			_word2C8D2 = false;
			return 0x7FFF;
		}
	}

	return 0;
}

bool EfhEngine::sub16E14() {
	int16 var68 = 0;
	char dest[20];
	char buffer[80];
	
	int16 monsterId;
	for (monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkPictureRefAvailability(monsterId))
			continue;

		if (!(_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == 0xFE) && !(!_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == _fullPlaceId))
			continue;

		if ((_mapMonsters[monsterId]._field_1 & 0x3F) > 0x3D
		&& (((_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) == 0x3F) || isCharacterATeamMember(_mapMonsters[monsterId]._field_1)))
			continue;

		if (_mapMonsters[monsterId]._posX != _mapPosX || _mapMonsters[monsterId]._posY != _mapPosY)
			continue;

		if (!_word2C8D7)
			return false;

		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
		if (_imageSetSubFilesIdx != _oldImageSetSubFilesIdx)
			_oldImageSetSubFilesIdx = _imageSetSubFilesIdx;
		_redrawNeededFl = true;

		int16 var6A = 0;
		for (int16 var6C = 0; var6C < 9; ++var6C) {
			if (_mapMonsters[monsterId]._pictureRef[var6C])
				++var6A;
		}

		do {
			for (int16 var6C = 0; var6C < 2; ++var6C) {
				int16 var1 = _mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F;
				if (var1 <= 0x3D) {
					strcpy(dest, kEncounters[_mapMonsters[monsterId]._MonsterRef]._name);
					if (var6A > 1)
						strcat(dest, " ");

					sprintf(buffer, "with %d %s", var6A, dest);
				} else if (var1 == 0x3E) {
					strcpy(buffer, "(NOT DEFINED)");
				} else if (var1 == 0x3F) { // Useless if, it's the last possible value
					copyString(_npcBuf[_mapMonsters[monsterId]._MonsterRef]._name, dest);
					sprintf(buffer, "with %s", dest);
				}

				unkFct_displayMenuBox_2(0);
				_textColor = 0xE;
				displayCenteredString("Interaction", 24, 296, 152);
				displayCenteredString(buffer, 24, 296, 161);
				setTextPos(24, 169);
				setTextColorWhite();
				displayStringAtTextPos("T");
				setTextColorRed();
				sprintf(buffer, "alk to the %s", dest);
				displayStringAtTextPos(buffer);
				setTextPos(24, 178);
				setTextColorWhite();
				displayStringAtTextPos("A");
				setTextColorRed();
				sprintf(buffer, "ttack the %s", dest);
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
				if (var6C == 0)
					displayFctFullScreen();
			}

			Common::KeyCode input = mapInputCode(waitForKey());

			switch (input) {
			case Common::KEYCODE_a: // Attack
				var6A = handleFight(monsterId);
				var68 = true;
				break;
			case Common::KEYCODE_ESCAPE:
			case Common::KEYCODE_l: // Leave
				var68 = true;
				break;
			case Common::KEYCODE_s: // Status
				var6A = handleStatusMenu(1, _teamCharId[0]);
				var68 = true;
				_dword2C856 = nullptr;
				sub15150(true);
				break;
			case Common::KEYCODE_t: // Talk
				sub221D2(monsterId);
				var68 = true;
				break;
			default:
				warning("STUB: sub16E14 - Missing mapping ?");
				break;
			}
		} while (!var68);
		return true;
	}

	int8 check = sub15581(_mapPosX, _mapPosY, 1);
	if (check == 0 || check == 2)
		return false;

	return true;
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
	_textColor = 0xE;

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

	_unkArray2C8AA[0] = f.readSint16LE();		

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

void EfhEngine::drawColoredRect(int minX, int minY, int maxX, int maxY, int color) {
	uint8 oldValue = _defaultBoxColor;
	_defaultBoxColor = color;
	drawRect(minX, minY, maxX, maxY);
	_defaultBoxColor = oldValue;
}

void EfhEngine::clearScreen(int color) {
	drawColoredRect(0, 0, 320, 200, color);
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

Common::KeyCode EfhEngine::waitForKey() {
	Common::KeyCode retVal = Common::KEYCODE_INVALID;
	Common::Event event;

	uint32 lastMs = _system->getMillis();
	while (retVal == Common::KEYCODE_INVALID) { // TODO: Check shouldquit()
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			unkFct_anim();
		}

		_system->getEventManager()->pollEvent(event);
		if (event.type == Common::EVENT_KEYUP) {
			retVal = event.kbd.keycode;
		} 	
	}

	return retVal;
}

Common::KeyCode EfhEngine::mapInputCode(Common::KeyCode input) {
	// Original is doing:
	// if input < a or > z : return input
	// else return (input + 0xE0)
	// ex: 'a' = 0x61 + 0xE0 = 0x0141, but it's a uint8 so it's 0x41 which is 'A'.
	// So basically the original works with uppercase letters and do not alter the other inputs.
	// => no implementation needed.
	return input;
}

Common::KeyCode EfhEngine::getLastCharAfterAnimCount(int16 delay) {
	if (delay == 0)
		return Common::KEYCODE_INVALID;

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

Common::KeyCode EfhEngine::getInput(int16 delay) {
	if (delay == 0)
		return Common::KEYCODE_INVALID;

	Common::KeyCode lastChar = Common::KEYCODE_INVALID;
	Common::KeyCode retVal = Common::KEYCODE_INVALID;

	uint32 lastMs = _system->getMillis();
	while (delay > 0) {
		_system->delayMillis(20);
		uint32 newMs = _system->getMillis();

		if (newMs - lastMs >= 200) {
			lastMs = newMs;
			--delay;
			unkFct_anim();
		}

		lastChar = handleAndMapInput(false);
		if (lastChar != Common::KEYCODE_INVALID)
			retVal = lastChar;
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

uint16 EfhEngine::getStringWidth(const char *buffer) {
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

void EfhEngine::displayRawDataAtPos(uint8 *imagePtr, int16 posX, int16 posY) {
	uint16 height = READ_LE_INT16(imagePtr);
	uint16 width = READ_LE_INT16(imagePtr + 2);
	uint8 *imageData = imagePtr + 4;

	_imageDataPtr._lineDataSize = width;
	_imageDataPtr._dataPtr = imageData;
	_imageDataPtr._height = height;
	_imageDataPtr._width = width * 2; // 2 pixels per byte
	_imageDataPtr._startX = _imageDataPtr._startY = 0;
	
	displayBufferBmAtPos(&_imageDataPtr, posX, posY);
}
} // End of namespace Efh
