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

void UnkMapStruct::init() {
	_placeId = _posX = _posY = _field3 = _field4 = 0;
	_field5_textId = _field7_textId = 0;
}

void UnkAnimStruct::init() {
	memset(_field, 0, 4);
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
	for (uint idx = 0; idx < 15; ++idx)
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
	for (int i = 0; i < 11; ++i)
		_name[i] =  0;
	field_B = 0;
	field_C = 0;
	field_D = 0;
	field_E = 0;
	field_F = 0;
	field_10 = 0;
	field_11 = 0;
	field12_textId = 0;
	field14_textId = 0;
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

void Stru32686::init() {
	for (int i = 0; i < 9; ++i) {
		_field0[i] = 0;
		_field2[i] = 0;
	}
}

void Stru3244C::init() {
	_field0 = _field2 = 0;
}

void TileFactStruct::init() {
	_field0 = _field1 = 0;
}

EfhEngine::EfhEngine(OSystem *syst, const ADGameDescription *gd) : Engine(syst), _gameDescription(gd) {
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
	_platform = Common::kPlatformUnknown;
	_mainSurface = nullptr;

	_vgaGraphicsStruct1 = new EfhGraphicsStruct(_vgaLineBuffer, 0, 0, 320, 200);
	_vgaGraphicsStruct2 = new EfhGraphicsStruct();

	_videoMode = 0;
	_graphicsStruct = nullptr;

	for (int i = 0; i < 19; ++i)
		_mapBitmapRefArr[i] = nullptr;

	_defaultBoxColor = 0;

	_fontDescr._widthArray = nullptr;
	_fontDescr._extraLines = nullptr;
	_fontDescr._fontData = nullptr;
	_fontDescr._charHeight = 0;
	_fontDescr._extraHorizontalSpace = _fontDescr._extraVerticalSpace = 0;

	_introDoneFl = false;
	_oldAnimImageSetId = -1;
	_animImageSetId = 0xFE;
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
	}

	_characterNamePt1 = "";
	_characterNamePt2 = "";
	_enemyNamePt1 = "";
	_enemyNamePt2 = "";
	_nameBuffer = "";
	_attackBuffer = "";

	for (int i = 0; i < 100; ++i) {
		_imp1PtrArray[i] = nullptr;
		_mapUnknown[i].init();
	}

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
		_teamPctVisible[i] = 0;
		_word32482[i] = 0;
		_teamNextAttack[i] = -1;
		_word31780[i] = 0;
		_teamLastAction[i] = 0;
	}

	for (int i = 0; i < 5; ++i) {
		_teamMonsterIdArray[i] = -1;
		_stru32686[i].init();
	}

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
	_tempTextDelay = 0;
	_tempTextPtr = nullptr;
	_word2C880 = false;
	_redrawNeededFl = false;
	_drawHeroOnMapFl = true;
	_drawMonstersOnMapFl = true;
	_word2C87A = false;
	_dbgForceDisplayUpperRightBorder = false;
	_dbgForceMonsterBlock = false;
	_ongoingFightFl = false;
	_statusMenuActive = false;
	_menuDepth = 0;
	_menuItemCounter = 0;

	for (int i = 0; i < 15; ++i) {
		_word3273A[i] = 0;
	}

	_messageToBePrinted = "";
	for (int i = 0; i < 8; ++i)
		_stru3244C[i].init();

	memset(_bufferCharBM, 0, ARRAYSIZE(_bufferCharBM));
	for (int i = 0; i < 3; ++i)
		memset(_tileBank[i], 0, ARRAYSIZE(_tileBank[i]));
	memset(_circleImageBuf, 0, ARRAYSIZE(_circleImageBuf));
	memset(_portraitBuf, 0, ARRAYSIZE(_portraitBuf));
	memset(_hiResImageBuf, 0, ARRAYSIZE(_hiResImageBuf));
	memset(_loResImageBuf, 0, ARRAYSIZE(_loResImageBuf));
	memset(_menuBuf, 0, ARRAYSIZE(_menuBuf));
	memset(_windowWithBorderBuf, 0, ARRAYSIZE(_windowWithBorderBuf));
	memset(_places, 0, ARRAYSIZE(_places));
	for (int i = 0; i < 24; ++i)
		memset(_curPlace[i], 0, ARRAYSIZE(_curPlace[i]));
	memset(_npcBuf, 0, ARRAYSIZE(_npcBuf));
	memset(_imp1, 0, ARRAYSIZE(_imp1));
	memset(_imp2, 0, ARRAYSIZE(_imp2));
	memset(_titleSong, 0, ARRAYSIZE(_titleSong));
	memset(_items, 0, ARRAYSIZE(_items));
	memset(_tileFact, 0, ARRAYSIZE(_tileFact));
	memset(_animInfo, 0, ARRAYSIZE(_animInfo));
	memset(_history, 0, ARRAYSIZE(_history));
	for (int i = 0; i < 19; ++i) {
		memset(_techDataArr[i], 0, ARRAYSIZE(_techDataArr[i]));
		memset(_mapArr[i], 0, ARRAYSIZE(_mapArr[i]));
	}
	memset(_mapMonsters, 0, ARRAYSIZE(_mapMonsters));
	memset(_mapGameMap, 0, ARRAYSIZE(_mapGameMap));
	memset(_imageSetSubFilesArray, 0, ARRAYSIZE(_imageSetSubFilesArray));

	// If requested, load a savegame instead of showing the intro
	_loadSaveSlot = -1;
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadSaveSlot = saveSlot;
	}
}

EfhEngine::~EfhEngine() {
	delete _rnd;
	delete _graphicsStruct;
	delete _vgaGraphicsStruct1;
	delete _vgaGraphicsStruct2;
}

void EfhEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	warning("TODO: _sound->syncVolume();");
}

Common::Error EfhEngine::run() {
	debug("run");
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
				saveEfhGame();
				clearBottomTextZone_2(0);
				displayLowStatusScreen(true);
			} else {
				displayMenuAnswerString("-> No!!! <-", 24, 296, 169);
				getInput(2);
				clearBottomTextZone_2(0);
				displayLowStatusScreen(true);
			}

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
//				loadEfhGame();
				saveEfhGame();
				clearBottomTextZone_2(0);
				displayLowStatusScreen(true);
			} else {
				displayMenuAnswerString("-> No!!! <-", 24, 296, 169);
				getInput(2);
				clearBottomTextZone_2(0);
				displayLowStatusScreen(true);
			}

		} break;
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
			sub174A0();
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

		if (--_unkArray2C8AA[0] < 0 && !_shouldQuit)
			_unkArray2C8AA[0] = 0;

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

Common::KeyCode EfhEngine::playSong(uint8 *buffer) {
	warning("STUB: playSong");

	_system->delayMillis(1000);

	return Common::KEYCODE_INVALID;
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
	_engineInitPending = false;
}

void EfhEngine::initMapMonsters() {
	debug("initMapMonsters");

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
			continue;

		for (uint counter = 0; counter < 9; ++counter)
			_mapMonsters[monsterId]._pictureRef[counter] = 0;

		uint8 groupSize = _mapMonsters[monsterId]._groupSize;
		if (groupSize == 0)
			groupSize = getRandom(10);

		for (uint counter = 0; counter < groupSize; ++counter) {
			uint rand100 = getRandom(100);
			uint16 pictureRef = kEncounters[_mapMonsters[monsterId]._monsterRef]._pictureRef;

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

void EfhEngine::loadMapArrays(int idx) {
	debug("loadMapArrays %d", idx);

	uint8 *_mapUnknownPtr = &_mapArr[idx][2];

	for (int i = 0; i < 100; ++i) {
		_mapUnknown[i]._placeId = _mapUnknownPtr[9 * i];
		_mapUnknown[i]._posX = _mapUnknownPtr[9 * i + 1];
		_mapUnknown[i]._posY = _mapUnknownPtr[9 * i + 2];
		_mapUnknown[i]._field3 = _mapUnknownPtr[9 * i + 3];
		_mapUnknown[i]._field4 = _mapUnknownPtr[9 * i + 4];
		_mapUnknown[i]._field5_textId = READ_LE_UINT16(&_mapUnknownPtr[9 * i + 5]);
		_mapUnknown[i]._field7_textId = READ_LE_UINT16(&_mapUnknownPtr[9 * i + 7]);
	}

	uint8 *mapMonstersPtr = &_mapArr[idx][902];

	for (int i = 0; i < 64; ++i) {
		_mapMonsters[i]._possessivePronounSHL6 = mapMonstersPtr[29 * i];
		_mapMonsters[i]._field_1 = mapMonstersPtr[29 * i + 1];
		_mapMonsters[i]._guess_fullPlaceId = mapMonstersPtr[29 * i + 2];
		_mapMonsters[i]._posX = mapMonstersPtr[29 * i + 3];
		_mapMonsters[i]._posY = mapMonstersPtr[29 * i + 4];
		_mapMonsters[i]._itemId_Weapon = mapMonstersPtr[29 * i + 5];
		_mapMonsters[i]._field_6 = mapMonstersPtr[29 * i + 6];
		_mapMonsters[i]._monsterRef = mapMonstersPtr[29 * i + 7];
		_mapMonsters[i]._field_8 = mapMonstersPtr[29 * i + 8];
		_mapMonsters[i]._field9_textId = mapMonstersPtr[29 * i + 9];
		_mapMonsters[i]._groupSize = mapMonstersPtr[29 * i + 10];
		for (int j = 0; j < 9; ++j)
			_mapMonsters[i]._pictureRef[j] = READ_LE_INT16(&mapMonstersPtr[29 * i + 11 + j * 2]);
	}

	uint8 *mapPtr = &_mapArr[idx][2758];
	for (int i = 0; i < 64; ++i) {
		for (int j = 0; j < 64; ++j)
			_mapGameMap[i][j] = *mapPtr++;
	}
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

uint16 EfhEngine::sub1C80A(int16 charId, int16 field18, bool flag) {
	debugC(2, kDebugEngine, "sub1C80A %d %d %s", charId, field18, flag ? "True" : "False");

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
				int16 idx = _mapGameMap[counterX][counterY];
				displayRawDataAtPos(_imageSetSubFilesArray[idx], drawPosX, drawPosY);
			} else {
				int16 idx = _curPlace[counterX][counterY];
				displayRawDataAtPos(_imageSetSubFilesArray[idx], drawPosX, drawPosY);
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
			if ((_largeMapFlag && _mapMonsters[var16]._guess_fullPlaceId == 0xFE) || (!_largeMapFlag && _mapMonsters[var16]._guess_fullPlaceId == _fullPlaceId)){
				bool var4 = false;
				int16 posX = _mapMonsters[var16]._posX;
				int16 posY = _mapMonsters[var16]._posY;

				if (posX < minX || posX > maxX || posY < minY || posY > maxY)
					continue;

				for (uint counterY = 0; counterY < 9 && !var4; ++counterY) {
					if (_mapMonsters[var16]._pictureRef[counterY] > 0)
						var4 = true;
				}

				if (!var4)
					continue;

				int16 var6 = 148 + kEncounters[_mapMonsters[var16]._monsterRef]._animId;
				int16 var1 = _mapMonsters[var16]._possessivePronounSHL6 & 0x3F;

				if (var1 == 0x3F && isCharacterATeamMember(_mapMonsters[var16]._field_1))
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

			// TODO: When refactoring : Always false, to be removed
			if (_dbgForceDisplayUpperRightBorder)
				drawUpperRightBorders();
		} else {
			if (_techId != 0xFF)
				displayLargeMap(_mapPosX, _mapPosY);

			// TODO: When refactoring : Always false, to be removed
			if (_dbgForceDisplayUpperRightBorder)
				drawUpperRightBorders();
		}
		if (counter == 0)
			displayFctFullScreen();
	}
}

void EfhEngine::displayLowStatusScreen(bool flag) {
	debugC(6, kDebugEngine, "displayLowStatusScreen %s", flag ? "True" : "False");

	Common::String strName = "Name";
	Common::String strDef = "DEF";
	Common::String strHp = "HP";
	Common::String strMaxHp = "Max HP";
	Common::String strWeapon = "Weapon";
	Common::String strDead = "* DEAD *";

	for (int counter = 0; counter < 2; ++counter) {
		if (counter == 0 || flag) {
			clearBottomTextZone(0);
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
					displayCenteredString(strDead, 225, 302, textPosY);
					continue;
				}

				switch (_teamCharStatus[i]._status) {
				case 0: {
					uint16 var4 = sub1C80A(charId, 9, true);
					if (var4 == 0x7FFF)
						_nameBuffer = "(NONE)";
					else
						_nameBuffer = _items[var4]._name;
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
	debug("removeObject %d %d", charId, objectId);
	_npcBuf[charId]._inventory[objectId]._ref = 0x7FFF;
	_npcBuf[charId]._inventory[objectId]._stat1 = 0;
	_npcBuf[charId]._inventory[objectId]._stat2 = 0;
}

void EfhEngine::totalPartyKill() {
	debug("totalPartyKill");

	for (uint counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1)
			_npcBuf[counter]._hitPoints = 0;
	}
}

void EfhEngine::removeCharacterFromTeam(int16 teamMemberId) {
	debug("removeCharacterFromTeam %d", teamMemberId);

	int16 charId = _teamCharId[teamMemberId];
	_npcBuf[charId].field12_textId = _npcBuf[charId].field_B;
	_npcBuf[charId].field14_textId = _npcBuf[charId].field_E;
	_npcBuf[charId].field_10 = _npcBuf[charId].field_C;
	_npcBuf[charId].field_11 = _npcBuf[charId].field_D;

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
	debug("refreshTeamSize");

	_teamSize = 0;
	for (uint counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1)
			++_teamSize;
	}
}

bool EfhEngine::isCharacterATeamMember(int16 id) {
	debug("isCharacterATeamMember %d", id);

	for (int counter = 0; counter < _teamSize; ++counter) {
		if (_teamCharId[counter] == id)
			return true;
	}

	return false;
}

bool EfhEngine::isTPK() {
	debug("isTPK");

	int16 zeroedChar = 0;
	for (int counter = 0; counter < _teamSize; ++counter) {
		if (_npcBuf[_teamCharId[counter]]._hitPoints <= 0)
			++zeroedChar;
	}

	return zeroedChar == _teamSize;
}

void EfhEngine::handleWinSequence() {
	debugC(1, kDebugEngine, "handleWinSequence");

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

bool EfhEngine::giveItemTo(int16 charId, int16 objectId, int16 altCharId) {
	debug("giveItemTo %d %d %d", charId, objectId, altCharId);

	for (uint newObjectId = 0; newObjectId < 10; ++newObjectId) {
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

int16 EfhEngine::chooseCharacterToReplace() {
	debug("chooseCharacterToReplace");

	Common::KeyCode maxVal = (Common::KeyCode)(Common::KEYCODE_0 + _teamSize);
	Common::KeyCode input = Common::KEYCODE_INVALID;
	for (;;) {
		input = waitForKey();
		if (input == Common::KEYCODE_ESCAPE || input == Common::KEYCODE_0 || (input > Common::KEYCODE_1 && input <= maxVal))
			break;
	}

	if (input == Common::KEYCODE_ESCAPE || input == Common::KEYCODE_0)
		return 0x1B;

	return (int16)input - (int16)Common::KEYCODE_1;
}

int16 EfhEngine::handleCharacterJoining() {
	debug("handleCharacterJoining");

	const char strReplaceWho[13] = "Replace Who?";
	for (uint counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] == -1) {
			return counter;
		}
	}

	for (uint counter = 0; counter < 2; ++counter) {
		drawColoredRect(200, 112, 278, 132, 0);
		displayCenteredString(strReplaceWho, 200, 278, 117);
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

void EfhEngine::sub15A28(int16 arg0, int16 arg2) {
	debug("sub15A28 %d %d", arg0, arg2);

	_drawHeroOnMapFl = false;
	int16 varE = arg0 - 11;
	int16 varC = arg2 - 11;

	if (varE < 0)
		varE = 0;
	if (varC < 0)
		varC = 0;

	for (uint counter = 0; counter <= 23; counter += 2) {
		for (uint var8 = 0; var8 <= 23; ++var8) {
			int16 var4 = counter + varE;
			int16 var2 = var8 + varC;
			_mapGameMap[var4][var2] = _curPlace[counter][var8];
		}
		drawScreen();
	}

	for (uint counter = 1; counter <= 23; counter += 2) {
		for (uint var8 = 0; var8 <= 23; ++var8) {
			int16 var4 = counter + varE;
			int16 var2 = var8 + varC;
			_mapGameMap[var4][var2] = _curPlace[counter][var8];
		}
		drawScreen();
	}

	getLastCharAfterAnimCount(3);
	_drawHeroOnMapFl = true;
}

void EfhEngine::sub2455E(int16 arg0, int16 arg2, int16 arg4) {
	debug("sub2455E %d %d %d", arg0, arg2, arg4);

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
			_techDataArr[_techId][var2 + var4 * 64] = varD;
		}
	}
}

int16 EfhEngine::sub1C219(Common::String str, int16 menuType, int16 displayOption, bool displayTeamWindowFl) {
	debug("sub1C219 %s %d %d %s", str.c_str(), menuType, displayOption, displayTeamWindowFl ? "True" : "False");

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

	drawColoredRect(minX, minY, maxX, maxY, 0);
	if (str.size())
		varA = script_parse(str, minX, minY, maxX, maxY, true);

	if (displayTeamWindowFl)
		displayLowStatusScreen(false);

	if (displayOption != 0) {
		displayFctFullScreen();
		if (_word2C87A)
			_word2C87A = false;
		else {
			drawColoredRect(minX, minY, maxX, maxY, 0);
			if (str.size())
				script_parse(str, minX, minY, maxX, maxY, false);
		}

		if (displayTeamWindowFl)
			displayLowStatusScreen(false);

		if (displayOption >= 2)
			getLastCharAfterAnimCount(_guessAnimationAmount);

		if (displayOption == 3)
			drawColoredRect(minX, minY, maxX, maxY, 0);
	}

	return varA;
}

int16 EfhEngine::sub151FD(int16 posX, int16 posY) {
	debug("sub151FD %d %d", posX, posY);

	if (_largeMapFlag) {
		for (uint counter = 0; counter < 100; ++counter) {
			if (_mapUnknown[counter]._posX == posX && _mapUnknown[counter]._posY == posY && _mapUnknown[counter]._placeId == 0xFE)
				return counter;
		}
	} else {
		for (uint counter = 0; counter < 100; ++counter) {
			if (_mapUnknown[counter]._posX == posX && _mapUnknown[counter]._posY == posY && _mapUnknown[counter]._placeId == _fullPlaceId)
				return counter;
		}
	}
	return -1;
}

bool EfhEngine::isPosOutOfMap(int16 mapPosX, int16 mapPosY) {
	debug("isPosOutOfMap %d %d", mapPosX, mapPosY);

	int16 maxMapBlocks = _largeMapFlag ? 63 : 23;

	if (mapPosX == 0 && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	if (mapPosX == maxMapBlocks && (mapPosY == 0 || mapPosY == maxMapBlocks))
		return true;

	return false;
}

void EfhEngine::goSouth() {
	debug("goSouth");

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
	debug("goNorth");

	if (--_mapPosY < 0)
		_mapPosY = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goEast() {
	debug("goEast");

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
	debug("goWest");

	if (--_mapPosX < 0)
		_mapPosX = 0;

	if (isPosOutOfMap(_mapPosX, _mapPosY)) {
		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
	}
}

void EfhEngine::goNorthEast() {
	debug("goNorthEast");

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
	debug("goSouthEast");

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
	debug("goNorthWest");

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
	debug("goSouthWest");

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
	debug("handleNewRoundEffects");

	static int16 regenCounter = 0;

	for (int counter = 0; counter < _teamSize; ++counter) {
		if (_teamCharStatus[counter]._status == 0) // normal
			continue;
		if (--_teamCharStatus[counter]._duration <= 0) {
			_teamCharStatus[counter]._status = 0;
			_teamCharStatus[counter]._duration = 0;
		}
	}

	if (++regenCounter <= 8)
		return;

	for (int counter = 0; counter < _teamSize; ++counter) {
		if (++_npcBuf[_teamCharId[counter]]._hitPoints > _npcBuf[_teamCharId[counter]]._maxHP)
			_npcBuf[_teamCharId[counter]]._hitPoints = _npcBuf[_teamCharId[counter]]._maxHP;
	}
	regenCounter = 0;
}

void EfhEngine::resetGame() {
	loadTechMapImp(0);
	_largeMapFlag = true;
	_oldMapPosX = _mapPosX = 31;
	_oldMapPosY = _mapPosY = 31;
	_unkRelatedToAnimImageSetId = 0;
	_unkArray2C8AA[0] = 0;
}

bool EfhEngine::handleDeathMenu() {
	debug("handleDeathMenu");

	displayAnimFrames(20, true);
	_imageSetSubFilesIdx = 213;
	drawScreen();

	for (uint counter = 0; counter < 2; ++counter) {
		clearBottomTextZone(0);
		displayCenteredString("Darkness Prevails...Death Has Taken You!", 24, 296, 153);
		setTextPos(100, 162);
		setTextColorWhite();
		displayCharAtTextPos('L');
		setTextColorRed();
		displayStringAtTextPos("oad last saved game");
		setTextPos(100, 171);
		setTextColorWhite();
		displayCharAtTextPos('R');
		setTextColorRed();
		displayStringAtTextPos("estart from beginning");
		setTextPos(100, 180);
		setTextColorWhite();
		displayCharAtTextPos('Q');
		setTextColorRed();
		displayStringAtTextPos("uit for now");
		if (counter == 0)
			displayFctFullScreen();
	}

	for (bool found = false; !found;) {
		Common::KeyCode input = waitForKey();
		switch (input) {
		case Common::KEYCODE_l:
			//loadEfhGame();
			//TODO : saveEfhGame opens the GUI save/load screen. It shouldn't bepossible to save at this point
			saveEfhGame();
			found = true;
			break;
		case Common::KEYCODE_q:
			_shouldQuit = true;
			return true;
			break;
		case Common::KEYCODE_r:
			loadEfhGame();
			resetGame();
			found = true;
			break;
		case Common::KEYCODE_x:
			found = true;
			break;
		default:
			break;
		}
	}

	displayAnimFrames(0xFE, true);
	return false;
}

void EfhEngine::computeMapAnimation() {
	debug("computeMapAnimation");

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
				uint8 var4 = _mapGameMap[counterX][counterY];
				if (var4 >= 1 && var4 <= 0xF) {
					if (getRandom(100) < 50)
						_mapGameMap[counterX][counterY] += 0xC5;
				} else if (var4 >= 0xC6 && var4 <= 0xD5) {
					if (getRandom(100) < 50)
						_mapGameMap[counterX][counterY] -= 0xC5;
				}
			} else {
				if (_currentTileBankImageSetId[0] != 0)
					continue;
				uint8 var4 = _curPlace[counterX][counterY];
				if (var4 >= 1 && var4 <= 0xF) {
					if (getRandom(100) < 50)
						_curPlace[counterX][counterY] += 0xC5;
				} else if (var4 >= 0xC6 && var4 <= 0xD5) {
					if (getRandom(100) < 50)
						_curPlace[counterX][counterY] -= 0xC5;
				}
			}
		}
	}
}

void EfhEngine::unkFct_anim() {
	setNumLock();

	if (_engineInitPending)
		return;

	debug("unkFct_anim");

	if (_animImageSetId != 0xFF) {
		displayNextAnimFrame();
		displayFctFullScreen();
		displayAnimFrame();
	}

	computeMapAnimation();
}

int8 EfhEngine::sub16B08(int16 monsterId) {
	debug("sub16B08 %d", monsterId);

	// Simplified version compared to the original
	int16 maxSize = _largeMapFlag ? 63 : 23;
	if (_mapMonsters[monsterId]._posX < 0 || _mapMonsters[monsterId]._posY < 0 || _mapMonsters[monsterId]._posX > maxSize || _mapMonsters[monsterId]._posY > maxSize)
		return 0;

	if (_mapMonsters[monsterId]._posX == _mapPosX && _mapMonsters[monsterId]._posY == _mapPosY)
		return 0;

	for (int counter = 0; counter < 64; ++counter) {
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
	debug("moveMonsterAwayFromTeam %d", monsterId);

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
	debug("moveMonsterTowardsTeam %d", monsterId);

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
	debug("moveMonsterGroupOther %d %d", monsterId, direction);

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
	debugC(2, kDebugEngine, "moveMonsterGroup %d", monsterId);

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

	int16 monsterPosX = _mapMonsters[monsterId]._posX;
	int16 monsterPosY = _mapMonsters[monsterId]._posY;

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

bool EfhEngine::unkFct_checkMonsterField8(int16 id, bool teamFlag) {
	debugC(6, kDebugEngine, "unkFct_checkMonsterField8 %d %s", id, teamFlag ? "True" : "False");

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
	debugC(6, kDebugEngine, "checkTeamWeaponRange %d", monsterId);

	if (!_ongoingFightFl)
		return true;

	for (uint counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == monsterId && unkFct_checkMonsterField8(monsterId, false) && checkWeaponRange(monsterId, _mapMonsters[monsterId]._itemId_Weapon))
			return false;
	}

	return true;
}

bool EfhEngine::checkIfMonsterOnSameLargeMapPlace(int16 monsterId) {
	debugC(6, kDebugEngine, "checkIfMonsterOnSameLargeMapPlace %d", monsterId);

	if (_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == 0xFE)
		return true;

	if (!_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == _fullPlaceId)
		return true;

	return false;
}

bool EfhEngine::checkMonsterWeaponRange(int16 monsterId) {
	debugC(6, kDebugEngine, "checkMonsterWeaponRange %d", monsterId);

	return checkWeaponRange(monsterId, _mapMonsters[monsterId]._itemId_Weapon);
}

void EfhEngine::sub174A0() {
	debug("sub174A0");

	static int16 sub174A0_monsterPosX = -1;
	static int16 sub174A0_monsterPosY = -1;

	_redrawNeededFl = true;
	int16 unkMonsterId = -1;
	int16 mapSize = _largeMapFlag ? 63 : 23;
	int16 minDisplayedMapX = CLIP<int16>(_mapPosX - 10, 0, mapSize);
	int16 minDisplayedMapY = CLIP<int16>(_mapPosY - 9, 0, mapSize);
	int16 maxDisplayedMapX = CLIP<int16>(minDisplayedMapX + 20, 0, mapSize);
	int16 maxDisplayedMapY = CLIP<int16>(minDisplayedMapY + 17, 0, mapSize);

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkPictureRefAvailability(monsterId))
			continue;

		if (!checkTeamWeaponRange(monsterId))
			continue;

		if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
			continue;

		int16 var4 = _mapMonsters[monsterId]._posX;
		int16 var2 = _mapMonsters[monsterId]._posY;

		if (var4 < minDisplayedMapX || var4 > maxDisplayedMapX || var2 < minDisplayedMapY || var2 > maxDisplayedMapY)
			continue;

		bool var1A = false;
		int16 var14 = 0;

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
	debugC(6, kDebugEngine, "checkPictureRefAvailability %d", monsterId);

	if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
		return false;

	for (uint counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[monsterId]._pictureRef[counter] > 0)
			return true;
	}

	return false;
}

void EfhEngine::displayMonsterAnim(int16 monsterId) {
	debug("displayMonsterAnim %d", monsterId);

	int16 animId = kEncounters[_mapMonsters[monsterId]._monsterRef]._animId;
	displayAnimFrames(animId, true);
}

int16 EfhEngine::countPictureRef(int16 id, bool teamMemberFl) {
	debug("countPictureRef %d %s", id, teamMemberFl ? "True" : "False");

	int16 count = 0;
	int16 monsterId;

	if (teamMemberFl)
		monsterId = _teamMonsterIdArray[id];
	else
		monsterId = id;

	for (uint counter = 0; counter < 9; ++counter) {
		if (_mapMonsters[monsterId]._pictureRef[counter] > 0)
			++count;
	}

	return count;
}

bool EfhEngine::checkMonsterGroupDistance1OrLess(int16 monsterId) {
	debug("checkMonsterGroupDistance1OrLess %d", monsterId);

	if (computeMonsterGroupDistance(monsterId) > 1)
		return false;

	return true;
}

bool EfhEngine::sub21820(int16 monsterId, int16 arg2, int16 itemId) {
	debug("sub21820 %d %d %d", monsterId, arg2, itemId);

	uint8 var51 = _mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F;
	if (_mapMonsters[monsterId]._guess_fullPlaceId == 0xFF)
		return false;

	if (countPictureRef(monsterId, false) < 1)
		return false;

	if (!checkIfMonsterOnSameLargeMapPlace(monsterId))
		return false;

	if (!checkMonsterGroupDistance1OrLess(monsterId))
		return false;

	if (var51 != 0x3F) {
		if (_mapMonsters[monsterId]._field9_textId == 0xFF || arg2 != 5) {
			return false;
		}
		displayMonsterAnim(monsterId);
		displayImp1Text(_mapMonsters[monsterId]._field9_textId);
		displayAnimFrames(0xFE, true);
		return true;
	}

	if (isCharacterATeamMember(_mapMonsters[monsterId]._field_1))
		return false;

	int16 var58 = _mapMonsters[monsterId]._field_1;
	switch (_npcBuf[var58].field_10 - 0xEE) {
	case 0:
		if (arg2 == 4 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[var58].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 1:
		if (arg2 == 2 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[var58].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 2:
		if (arg2 == 1 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[var58].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 3:
		if (_history[_npcBuf[var58].field_11] != 0) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[var58].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 4:
		for (int counter = 0; counter < _teamSize; ++counter) {
			for (uint charId = 0; charId < 10; ++charId) {
				if (_npcBuf[_teamCharId[counter]]._inventory[charId]._ref == _npcBuf[var58].field_11) {
					removeObject(_teamCharId[counter], charId);
					displayMonsterAnim(monsterId);
					displayImp1Text(_npcBuf[var58].field14_textId);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 5:
		if (arg2 == 2 && _npcBuf[var58].field_11 == itemId) {
			displayMonsterAnim(monsterId);
			displayImp1Text(_npcBuf[var58].field14_textId);
			displayAnimFrames(0xFE, true);
			return true;
		}
		break;
	case 6:
		for (int counter = 0; counter < _teamSize; ++counter) {
			for (uint charId = 0; charId < 10; ++charId) {
				if (_npcBuf[_teamCharId[counter]]._inventory[charId]._ref == _npcBuf[var58].field_11) {
					displayMonsterAnim(monsterId);
					displayImp1Text(_npcBuf[var58].field14_textId);
					displayAnimFrames(0xFE, true);
					return true;
				}
			}
		}
		break;
	case 7:
		for (int counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				removeCharacterFromTeam(counter);
				displayMonsterAnim(monsterId);
				displayImp1Text(_npcBuf[var58].field14_textId);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 8:
		for (int counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				displayMonsterAnim(monsterId);
				_enemyNamePt2 = _npcBuf[var58]._name;
				_characterNamePt2 = _npcBuf[_teamCharId[counter]]._name;
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
					removeCharacterFromTeam(counter);
					displayImp1Text(_npcBuf[var58].field14_textId);
				}
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 9:
		for (int counter = 0; counter < _teamSize; ++counter) {
			if (_npcBuf[var58].field_11 == _teamCharId[counter]) {
				displayMonsterAnim(monsterId);
				displayImp1Text(_npcBuf[var58].field14_textId);
				displayAnimFrames(0xFE, true);
				return true;
			}
		}
		break;
	case 16:
		displayMonsterAnim(monsterId);
		displayImp1Text(_npcBuf[var58].field14_textId);
		displayAnimFrames(0xFE, true);
		return true;
	default:

		break;
	}

	if (_npcBuf[var58].field12_textId == 0x7FFF || arg2 != 5)
		return false;

	displayMonsterAnim(monsterId);
	displayImp1Text(_npcBuf[var58].field12_textId);
	displayAnimFrames(0xFE, true);
	return true;
}

void EfhEngine::sub221D2(int16 monsterId) {
	debug("sub221D2 %d", monsterId);

	if (monsterId != -1) {
		_tempTextPtr = nullptr;
		sub21820(monsterId, 5, -1);
	}
}

void EfhEngine::displayImp1Text(int16 textId) {
	debug("displayImp1Text %d", textId);

	int16 charCounter = 0;
	int16 stringIdx = 0;
	bool textComplete = false;
	bool maxReached = false;

	if (textId <= 0xFE) {
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

						nextTextId = sub1C219(_messageToBePrinted, 1, 1, true);
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

bool EfhEngine::sub22293(int16 mapPosX, int16 mapPosY, int16 charId, int16 itemId, int16 arg8, int16 imageSetId) {
	debug("sub22293 %d-%d %d %d %d %d", mapPosX, mapPosY, charId, itemId, arg8, imageSetId);

	int16 var8 = sub151FD(mapPosX, mapPosY);

	if (var8 == -1) {
		if (imageSetId != -1 && *_imp2PtrArray[imageSetId] != 0x30)
			displayMiddleLeftTempText(_imp2PtrArray[imageSetId], true);
	} else if (arg8 == 0) {
		if (_mapUnknown[var8]._field3 == 0xFF) {
			displayImp1Text(_mapUnknown[var8]._field5_textId); // word!
			return true;
		}

		if (_mapUnknown[var8]._field3 == 0xFE) {
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;
				if (_teamCharId[counter] == _mapUnknown[var8]._field4) {
					displayImp1Text(_mapUnknown[var8]._field5_textId);
					return true;
				}
			}
		} else if (_mapUnknown[var8]._field3 == 0xFD) {
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (uint var2 = 0; var2 < 10; ++var2) {
					if (_npcBuf[_teamCharId[counter]]._inventory[var2]._ref == _mapUnknown[var8]._field4) {
						displayImp1Text(_mapUnknown[var8]._field5_textId);
						return true;
					}
				}
			}
		// original makes a useless check on (_mapUnknownPtr[var8 * 9 + 3] > 0x7F)
		} else if (_mapUnknown[var8]._field3 <= 0x77) {
			int16 var6 = _mapUnknown[var8]._field3;
			for (int counter = 0; counter < _teamSize; ++counter) {
				if (_teamCharId[counter] == -1)
					continue;

				for (uint var2 = 0; var2 < 39; ++var2) {
					// CHECKME : the whole look doesn't make much sense as it's using var6 instead of var2, plus _activeScore is an array of 15 bytes, not 0x77...
					// Also, 39 correspond to the size of activeScore + passiveScore + infoScore + the 2 remaining bytes of the struct
					if (_npcBuf[_teamCharId[counter]]._activeScore[var6] >= _mapUnknown[var8]._field4) {
						displayImp1Text(_mapUnknown[var8]._field5_textId);
						return true;
					}
				}
			}
		}
	} else {
		if ((_mapUnknown[var8]._field3 == 0xFA && arg8 == 1) || (_mapUnknown[var8]._field3 == 0xFC && arg8 == 2) || (_mapUnknown[var8]._field3 == 0xFB && arg8 == 3)) {
			if (_mapUnknown[var8]._field4 == itemId) {
				displayImp1Text(_mapUnknown[var8]._field5_textId);
				return true;
			}
		} else if (arg8 == 4) {
			int16 var6 = _mapUnknown[var8]._field3;
			if (var6 >= 0x7B && var6 <= 0xEF) {
				var6 -= 0x78;
				if (var6 >= 0 && var6 <= 0x8B && var6 == itemId && _mapUnknown[var8]._field4 <= _npcBuf[charId]._activeScore[itemId]) {
					displayImp1Text(_mapUnknown[var8]._field5_textId);
					return true;
				}
			}
		}
	}

	for (uint counter = 0; counter < 64; ++counter) {
		if (!sub21820(counter, arg8, itemId))
			return true;
	}

	if ((arg8 == 4 && _mapUnknown[var8]._field3 < 0xFA) || arg8 != 4) {
		if (_mapUnknown[var8]._field7_textId > 0xFE)
			return false;
		displayImp1Text(_mapUnknown[var8]._field7_textId);
		return true;
	}

	return false;
}

int8 EfhEngine::sub15581(int16 mapPosX, int16 mapPosY, int16 arg4) {
	debug("sub15581 %d-%d %d", mapPosX, mapPosY, arg4);

	int16 curTileInfo = getMapTileInfo(mapPosX, mapPosY);
	int16 imageSetId = _currentTileBankImageSetId[curTileInfo / 72];
	imageSetId *= 72;
	imageSetId += curTileInfo % 72;

	if (arg4 == 1) {
		sub22293(mapPosX, mapPosY, -1, 0x7FFF, 0, imageSetId);
	}

	if (_word2C880) {
		_word2C880 = false;
		return -1;
	}
	if (_tileFact[imageSetId]._field1 != 0xFF && !_dbgForceMonsterBlock) {
		if ((arg4 == 1) || (arg4 == 0 && imageSetId != 128 && imageSetId != 121)) {
			if (_largeMapFlag) {
				_mapGameMap[mapPosX][mapPosY] = _tileFact[imageSetId]._field1;
			} else {
				_curPlace[mapPosX][mapPosY] = _tileFact[imageSetId]._field1;
			}

			_redrawNeededFl = true;
			if (_tileFact[imageSetId]._field0 == 0)
				return 2;
			return 1;
		}
	}

	return _tileFact[imageSetId]._field0;
}

bool EfhEngine::isTeamMemberStatusNormal(int16 teamMemberId) {
	debug("isTeamMemberStatusNormal %d", teamMemberId);

	if (_npcBuf[_teamCharId[teamMemberId]]._hitPoints > 0 && _teamCharStatus[teamMemberId]._status == 0)
		return true;

	return false;
}

void EfhEngine::sub1CDFA() {
	debug("sub1CDFA"); // Initiatives

	for (int counter = 0; counter < 3; ++counter) {
		if (_teamCharId[counter] != -1 && counter < _teamSize) {
			_stru3244C[counter]._field0 = counter + 1000;
			_stru3244C[counter]._field2 = _npcBuf[_teamCharId[counter]]._infoScore[3];
		} else {
			_stru3244C[counter]._field0 = -1;
			_stru3244C[counter]._field2 = -1;
		}
	}

	for (int counter = 0; counter < 5; ++counter) {
		if (_teamMonsterIdArray[counter] == -1) {
			_stru3244C[counter + 3]._field0 = -1;
			_stru3244C[counter + 3]._field2 = -1;
		} else {
			_stru3244C[counter + 3]._field0 = counter;
			_stru3244C[counter + 3]._field2 = _mapMonsters[_teamMonsterIdArray[counter]]._field_1 + getRandom(20);
		}
	}

	for (uint counter = 0; counter < 8; ++counter) {
		for (uint counter2 = 0; counter2 < 8; ++counter2) {
			if (_stru3244C[counter]._field2 >= _stru3244C[counter2]._field2)
				continue;

			SWAP(_stru3244C[counter]._field0, _stru3244C[counter2]._field0);
			SWAP(_stru3244C[counter]._field2, _stru3244C[counter2]._field2);
		}
	}
}

void EfhEngine::redrawScreenForced() {
	debug("redrawScreenForced");

	for (uint counter = 0; counter < 2; ++counter) {
		drawScreen();
		if (counter == 0)
			displayFctFullScreen();
	}
}

int16 EfhEngine::selectMonsterGroup() {
	debug("selectMonsterGroup");

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
			if (_teamMonsterIdArray[retVal] == -1)
				retVal = -1;
			break;
		default:
			break;
		}
	}

	return retVal;
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

void EfhEngine::sub1CAB6(int16 charId) {
	debug("sub1CAB6 %d", charId);

	for (uint counter = 0; counter < 2; ++counter) {
		drawGameScreenAndTempText(false);
		displayLowStatusScreen(false);
		drawCombatScreen(charId, false, false);
		if (counter == 0)
			displayFctFullScreen();
	}
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

				}
				break;
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

// The parameter isn't used in the original
void EfhEngine::sub1BE9A(int16 monsterId) {
	debug("sub1BE9A %d", monsterId);

	int16 var4 = 1;

	// sub1BE9A - 1rst loop counter1_monsterId - Start
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		if (countMonsterGroupMembers(counter1))
			continue;

		for (uint counter2 = 0; counter2 < 9; ++counter2) {
			_mapMonsters[_teamMonsterIdArray[counter1]]._pictureRef[counter2] = 0;
			_stru32686[counter1]._field0[counter2] = 0;
			_stru32686[counter1]._field2[counter2] = 0;
		}

		_teamMonsterIdArray[counter1] = -1;
		for (uint counter2 = counter1 + 1; counter2 < 5; ++counter2) {
			for (uint var8 = 0; var8 < 9; ++var8) {
				_stru32686[counter1]._field0[var8] = _stru32686[counter2]._field0[var8];
				_stru32686[counter1]._field2[var8] = _stru32686[counter2]._field2[var8];
			}
			_teamMonsterIdArray[counter1] = _teamMonsterIdArray[counter2];
		}

	}
	// sub1BE9A - 1rst loop counter1_monsterId - End

	var4 = -1;
	for (uint counter1 = 0; counter1 < 5; ++counter1) {
		if (_teamMonsterIdArray[counter1] == -1) {
			var4 = counter1;
			break;
		}
	}

	if (var4 != -1) {
		// sub1BE9A - loop var2 - Start
		for (int var2 = 1; var2 < 3; ++var2) {
			if (var4 >= 5)
				break;

			for (uint counter1 = 0; counter1 < 64; ++counter1) {
				if (_mapMonsters[counter1]._guess_fullPlaceId == 0xFF)
					continue;

				if (((_mapMonsters[counter1]._possessivePronounSHL6 & 0x3F) == 0x3F && !isCharacterATeamMember(_mapMonsters[counter1]._field_1)) || (_mapMonsters[counter1]._possessivePronounSHL6 & 0x3F) <= 0x3D) {
					if (checkIfMonsterOnSameLargeMapPlace(counter1)) {
						bool var6 = false;
						for (uint counter2 = 0; counter2 < 9; ++counter2) {
							if (_mapMonsters[counter1]._pictureRef[counter2] > 0) {
								var6 = true;
								break;
							}
						}

						if (!var6)
							continue;

						if (computeMonsterGroupDistance(counter1) > var2)
							continue;

						if (sub1BC74(counter1, var4))
							continue;

						_teamMonsterIdArray[var4] = counter1;

						// The original at this point was doing a loop on counter1, which is not a good idea as
						// it was resetting the counter1 to 9 whatever its value before the loop.
						// Furthermore, it was accessing _stru32686[counter1]._field0[counter1] which doesn't make
						// sense...
						// I therefore decided to use another counter as it looks like an original misbehavior/bug.
						for (uint counter2 = 0; counter2 < 9; ++counter2) {
							_stru32686[counter1]._field0[counter2] = 0;
						}

						if (++var4 >= 5)
							break;
					}
				}
			}
		}
		// sub1BE9A - loop var2 - End
	}

	if (var4 == -1 || var4 > 4)
		return;

	// sub1BE9A - last loop counter1_monsterId - Start
	for (int16 counter1 = var4; counter1 < 5; ++counter1) {
		_teamMonsterIdArray[counter1] = -1;
		for (uint counter2 = 0; counter2 < 9; ++counter2) {
			_stru32686[counter1]._field0[counter2] = (int16)0x8000;
		}
	}
	// sub1BE9A - last loop counter1_monsterId - End
}

int16 EfhEngine::getTeamMonsterAnimId() {
	debug("getTeamMonsterAnimId");

	int16 retVal = 0xFF;
	for (uint counter = 0; counter < 5; ++counter) {
		int16 monsterId = _teamMonsterIdArray[counter];
		if (monsterId == -1)
			continue;

		if (!unkFct_checkMonsterField8(monsterId, false))
			continue;

		retVal = kEncounters[_mapMonsters[monsterId]._monsterRef]._animId;
		break;
	}

	if (retVal == 0xFF)
		retVal = kEncounters[_mapMonsters[_teamMonsterIdArray[0]]._monsterRef]._animId;

	return retVal;
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

void EfhEngine::displayCombatMenu(int16 charId) {
	debug("displayCombatMenu %d", charId);

	Common::String buffer = _npcBuf[charId]._name;
	buffer += ":";
	setTextColorWhite();
	setTextPos(144, 7);
	displayStringAtTextPos(buffer);
	setTextPos(152, 79);
	displayStringAtTextPos("A");
	setTextColorRed();
	displayStringAtTextPos("ttack");
	setTextPos(195, 79);
	setTextColorWhite();
	displayStringAtTextPos("H");
	setTextColorRed();
	displayStringAtTextPos("ide");
	setTextPos(152, 88);
	setTextColorWhite();
	displayStringAtTextPos("D");
	setTextColorRed();
	displayStringAtTextPos("efend");
	setTextPos(195, 88);
	setTextColorWhite();
	displayStringAtTextPos("R");
	setTextColorRed();
	displayStringAtTextPos("un");
	setTextPos(152, 97);
	setTextColorWhite();
	displayStringAtTextPos("S");
	setTextColorRed();
	displayStringAtTextPos("tatus");
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

	switch(_techDataArr[_techId][_techDataId_MapPosX * 64 + _techDataId_MapPosY]) {
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

void EfhEngine::sub1D8C2(int16 charId, int16 damage) {
	debug("sub1D8C2 %d %d", charId, damage);

	int16 destroyCounter = 0;
	int16 var40 = _npcBuf[charId]._possessivePronounSHL6 / 64;

	if (var40 > 2) {
		var40 = 2;
	}

	if (damage > 50)
		damage = 50;

	for (uint objectId = 0; objectId < 10; ++objectId) {
		if (_npcBuf[charId]._inventory[objectId]._ref == 0x7FFF || (_npcBuf[charId]._inventory[objectId]._stat1 & 0x80) == 0 && _items[_npcBuf[charId]._inventory[objectId]._ref]._defense == 0)
			continue;

		int16 var44 = damage - _npcBuf[charId]._inventory[objectId]._stat2;
		_npcBuf[charId]._inventory[objectId]._stat2 -= damage;

		if (_npcBuf[charId]._inventory[objectId]._stat2 <= 0) {
			Common::String buffer2 = _items[_npcBuf[charId]._inventory[objectId]._ref]._name;
			removeObject(charId, objectId);

			if (destroyCounter == 0) {
				destroyCounter = 1;
				_messageToBePrinted += Common::String::format(", but %s ", kPossessive[var40]) + buffer2;
			} else {
				++destroyCounter;
				_messageToBePrinted += Common::String(", ") + buffer2;
			}
		}

		if (var44 > 0)
			damage = var44;
	}

	if (destroyCounter == 0) {
		_messageToBePrinted += "!";
	} else if (destroyCounter > 1 || _messageToBePrinted.lastChar() == 's' || _messageToBePrinted.lastChar() == 'S') {
		_messageToBePrinted += " are destroyed!";
	} else {
		_messageToBePrinted += " is destroyed!";
	}
}

void EfhEngine::displayMenuItemString(int16 menuBoxId, int16 thisBoxId, int16 minX, int16 maxX, int16 minY, const char *str) {
	debug("displayMenuItemString %d %d %d->%d %d %s", menuBoxId, thisBoxId, minX, maxX, minY, str);

	if (menuBoxId == thisBoxId) {
		if (_menuDepth == 0)
			setTextColorWhite();
		else
			setTextColorGrey();

		Common::String buffer = Common::String::format("> %s <", str);
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
	debug("displayStatusMenu %d", windowId);

	for (uint counter = 0; counter < 9; ++counter) {
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
	debug("countRightWindowItems %d %d", menuId, charId);

	int16 var2 = 0;
	int16 var4 = 0;
	_menuItemCounter = 0;

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
		_menuItemCounter = 0;
		break;
	}

	if (var4 == -1) {
		for (uint counter = 0; counter < 10; ++counter) {
			if (_npcBuf[charId]._inventory[counter]._ref != 0x7FFF) {
				_word3273A[_menuItemCounter++] = counter;
			}
		}
	} else {
		for (int16 counter = var4; counter < var2; ++counter) {
			if (_npcBuf[charId]._activeScore[counter] != 0) {
				_word3273A[_menuItemCounter++] = counter;
			}
		}
	}
}

int16 EfhEngine::getXPLevel(int32 xp) {
	debug("getXPLevel %ld", xp);

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

void EfhEngine::displayCharacterSummary(int16 curMenuLine, int16 npcId) {
	debug("displayCharacterSummary %d %d", curMenuLine, npcId);

	setTextColorRed();
	Common::String buffer1 = _npcBuf[npcId]._name;
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Level: %d", getXPLevel(_npcBuf[npcId]._xp));
	setTextPos(146, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("XP: %lu", _npcBuf[npcId]._xp);
	setTextPos(227, 36);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Speed: %d", _npcBuf[npcId]._speed);
	setTextPos(146, 45);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Defense: %d", getEquipmentDefense(npcId, false));
	setTextPos(146, 54);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Hit Points: %d", _npcBuf[npcId]._hitPoints);
	setTextPos(146, 63);
	displayStringAtTextPos(buffer1);
	buffer1 = Common::String::format("Max HP: %d", _npcBuf[npcId]._maxHP);
	setTextPos(227, 63);
	displayStringAtTextPos(buffer1);
	displayCenteredString("Inventory", 144, 310, 72);

	if (_menuItemCounter == 0) {
		if (curMenuLine != -1)
			setTextColorWhite();

		displayCenteredString("Nothing Carried", 144, 310, 117);
		setTextColorRed();
		return;
	}

	for (int counter = 0; counter < _menuItemCounter; ++counter) {
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
				displayCharAtTextPos('E');
			}
		}

		setTextPos(152, textPosY);
		if (counter == curMenuLine) {
			buffer1 = Common::String::format("%c>", 'A' + counter);
		} else {
			buffer1 = Common::String::format("%c)", 'A' + counter);
		}
		displayStringAtTextPos(buffer1);

		if (itemId != 0x7FFF) {
			setTextPos(168, textPosY);
			buffer1 = Common::String::format("  %s", _items[itemId]._name);
			displayStringAtTextPos(buffer1);
			setTextPos(262, textPosY);

			if (_items[itemId]._defense > 0) {
				int16 var54 = _npcBuf[npcId]._inventory[_word3273A[counter]]._stat2;
				if (var54 == 0xFF) {
					// useless?
					var54 = _items[_npcBuf[npcId]._inventory[_word3273A[counter]]._ref]._defense;
				} else {
					buffer1 = Common::String::format("%d", 1 + var54 / 8);
					displayStringAtTextPos(buffer1);
					setTextPos(286, textPosY);
					displayStringAtTextPos("Def");
				}
			} else if (_items[itemId]._uses != 0x7F) {
				int16 var52 = _npcBuf[npcId]._inventory[_word3273A[counter]]._stat1;
				if (var52 != 0x7F) {
					buffer1 = Common::String::format("%d", var52);
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
	debug("displayCharacterInformationOrSkills %d %d", curMenuLine, charId);

	setTextColorRed();
	Common::String buffer = _npcBuf[charId]._name;
	setTextPos(146, 27);
	displayStringAtTextPos("Name: ");
	displayStringAtTextPos(buffer);
	if (_menuItemCounter <= 0) {
		if (curMenuLine != -1)
			setTextColorWhite();
		displayCenteredString("No Skills To Select", 144, 310, 96);
		setTextColorRed();
		return;
	}

	for (int counter = 0; counter < _menuItemCounter; ++counter) {
		if (counter == curMenuLine)
			setTextColorWhite();
		int16 textPosY = 38 + counter * 9;
		setTextPos(146, textPosY);
		if (counter == curMenuLine) {
			buffer = Common::String::format("%c>", 'A' + counter);
		} else {
			buffer = Common::String::format("%c)", 'A' + counter);
		}

		displayStringAtTextPos(buffer);
		setTextPos(163, textPosY);
		displayStringAtTextPos(kSkillArray[_word3273A[counter]]);
		buffer = Common::String::format("%d", _npcBuf[charId]._activeScore[_word3273A[counter]]);
		setTextPos(278, textPosY);
		displayStringAtTextPos(buffer);
		setTextColorRed();
	}
}

void EfhEngine::displayStatusMenuActions(int16 menuId, int16 curMenuLine, int16 npcId) {
	debug("displayStatusMenuActions %d %d %d", menuId, curMenuLine, npcId);

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
	debug("unk_StatusMenu %d %d %d %d %s", windowId, menuId, curMenuLine, charId, refreshFl ? "True" : "False");

	displayStatusMenu(windowId);

	countRightWindowItems(menuId, charId);
	displayStatusMenuActions(menuId, curMenuLine, charId);

	if (refreshFl)
		displayFctFullScreen();
}

void EfhEngine::sub18E80(int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debug("sub18E80 %d %d %d %d", charId, windowId, menuId, curMenuLine);

	for (int counter = 0; counter < 2; ++counter) {
		displayWindow(_menuBuf, 0, 0, _hiResImageBuf);
		unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, false);

		if (counter == 0)
			displayFctFullScreen();
	}
}

int16 EfhEngine::displayString_3(Common::String str, bool animFl, int16 charId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debug("displayString_3 %s %s %d %d %d %d", str.c_str(), animFl ? "True" : "False", charId, windowId, menuId, curMenuLine);

	int16 retVal = 0;

	for (uint counter = 0; counter < 2; ++counter) {
		unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, false);
		displayWindow(_windowWithBorderBuf, 19, 113, _hiResImageBuf);

		if (counter == 0) {
			script_parse(str, 28, 122, 105, 166, false);
			displayFctFullScreen();
		} else {
			retVal = script_parse(str, 28, 122, 105, 166, true);
		}
	}

	if (animFl) {
		getLastCharAfterAnimCount(_guessAnimationAmount);
		sub18E80(charId, windowId, menuId, curMenuLine);
	}

	return retVal;
}

bool EfhEngine::isItemCursed(int16 itemId) {
	debugC(6, kDebugEngine, "isItemCursed %d", itemId);

	if (_items[itemId].field_16 == 21 || _items[itemId].field_16 == 22 || _items[itemId].field_16 == 23)
		return true;

	return false;
}

bool EfhEngine::hasObjectEquipped(int16 charId, int16 objectId) {
	debugC(6, kDebugEngine, "hasObjectEquipped %d %d", charId, objectId);
	if ((_npcBuf[charId]._inventory[objectId]._stat1 & 0x80) == 0)
		return false;

	return true;
}

void EfhEngine::equipCursedItem(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debug("equipCursedItem %d %d %d %d %d", charId, objectId, windowId, menuId, curMenuLine);

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (isItemCursed(itemId)) {
		_npcBuf[charId]._inventory[objectId]._stat1 &= 0x7F;
	} else {
		displayString_3("Cursed Item Already Equipped!", true, charId, windowId, menuId, curMenuLine);
	}

}

void EfhEngine::sub191FF(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine) {
	debug("sub191FF %d %d %d %d %d", charId, objectId, windowId, menuId, curMenuLine);

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;

	if (hasObjectEquipped(charId, objectId)) {
		equipCursedItem(charId, objectId, windowId, menuId, curMenuLine);
	} else {
		int16 var2 = _items[itemId].field_18;
		if (var2 != 4) {
			for (uint counter = 0; counter < 10; ++counter) {
				if (var2 == _items[_npcBuf[charId]._inventory[counter]._ref].field_18)
					equipCursedItem(charId, objectId, windowId, menuId, curMenuLine);
			}
		}

		_npcBuf[charId]._inventory[objectId]._stat1 |= 0x80;
	}
}

void EfhEngine::sub1E028(int16 id, uint8 mask, int16 groupFl) {
	debug("sub1E028 %d 0x%X %d", id, mask, groupFl);

	int16 monsterId;
	if (groupFl) {
		monsterId = _teamMonsterIdArray[id];
	} else {
		monsterId = id;
	}

	_mapMonsters[monsterId]._field_8 &= 0xF0;
	_mapMonsters[monsterId]._field_8 |= mask;
}

bool EfhEngine::isMonsterActive(int16 groupId, int16 id) {
	debugC(5, kDebugEngine, "isMonsterActive %d %d", groupId, id);

	if (_mapMonsters[_teamMonsterIdArray[groupId]]._pictureRef[id] > 0 && _stru32686[groupId]._field0[id] == 0)
		return true;
	return false;
}

int16 EfhEngine::sub15538(int16 mapPosX, int16 mapPosY) {
	debug("sub15538 %d-%d", mapPosX, mapPosY);

	int16 mapTileInfo = getMapTileInfo(mapPosX, mapPosY);
	int16 imageSetId = mapTileInfo / 72;

	return (_currentTileBankImageSetId[imageSetId] * 72) + (mapTileInfo % 72);
}

void EfhEngine::setCharacterObjectToBroken(int16 charId, int16 objectId) {
	debug("setCharacterObjectToBroken %d %d", charId, objectId);

	_npcBuf[charId]._inventory[objectId]._ref = 0x7FFF;
}

int16 EfhEngine::selectOtherCharFromTeam() {
	debug("selectOtherCharFromTeam");

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

int16 EfhEngine::sub19E2E(int16 charId, int16 objectId, int16 windowId, int16 menuId, int16 curMenuLine, int16 argA) {
	debug("sub19E2E %d %d %d %d %d %d", charId, objectId, windowId, menuId, curMenuLine, argA);

	Common::String buffer1 = "";

	bool varA6 = false;
	bool retVal = false;

	int16 itemId = _npcBuf[charId]._inventory[objectId]._ref;
	switch (_items[itemId].field_16 - 1) {
	case 0: // "Demonic Powers", "MindDomination", "Guilt Trip", "Sleep Grenade", "SleepGrenader"
		if (argA == 2) {
			displayString_3("The item emits a low droning hum...", false, charId, windowId, menuId, curMenuLine);
		} else {
			int16 victims = 0;
			_messageToBePrinted += "  The item emits a low droning hum...";
			if (getRandom(100) < 50) {
				for (uint counter = 0; counter < 9; ++counter) {
					if (isMonsterActive(windowId, counter)) {
						++victims;
						_stru32686[windowId]._field0[counter] = 1;
						_stru32686[windowId]._field2[counter] = getRandom(8);
					}
				}
			} else {
				int16 NumberOfTargets = getRandom(9);
				for (uint counter = 0; counter < 9; ++counter) {
					if (NumberOfTargets == 0)
						break;

					if (isMonsterActive(windowId, counter)) {
						++victims;
						--NumberOfTargets;
						_stru32686[windowId]._field0[counter] = 1;
						_stru32686[windowId]._field2[counter] = getRandom(8);
					}
				}
			}
			// The original was duplicating this code in each branch of the previous random check.
			if (victims > 1) {
				buffer1 = Common::String::format("%d %ss fall asleep!", victims, kEncounters[_mapMonsters[_teamMonsterIdArray[windowId]]._monsterRef]._name);
			} else {
				buffer1 = Common::String::format("%d %s falls asleep!", victims, kEncounters[_mapMonsters[_teamMonsterIdArray[windowId]]._monsterRef]._name);
			}
			_messageToBePrinted += buffer1;
		}

		varA6 = true;
		break;
	case 1: // "Chilling Touch", "Guilt", "Petrify Rod", "Elmer's Gun"
		if (argA == 2) {
			displayString_3("The item grows very cold for a moment...", false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The item emits a blue beam...";
			int16 victim = 0;
			if (getRandom(100) < 50) {
				for (uint varA8 = 0; varA8 < 9; ++varA8) {
					if (isMonsterActive(windowId, varA8)) {
						++victim;
						_stru32686[windowId]._field0[varA8] = 2;
						_stru32686[windowId]._field2[varA8] = getRandom(8);
					}
				}
			} else {
				int16 varAC = getRandom(9);
				for (uint varA8 = 0; varA8 < 9; ++varA8) {
					if (varAC == 0)
						break;

					if (isMonsterActive(windowId, varA8)) {
						++victim;
						--varAC;
						_stru32686[windowId]._field0[varA8] = 2;
						_stru32686[windowId]._field2[varA8] = getRandom(8);
					}
				}
			}
			// <CHECKME>: This part is only present in the original in the case < 50, but for me
			// it's missing in the other case as there's an effect (frozen enemies) but no feedback to the player
			if (victim > 1) {
				buffer1 = Common::String::format("%d %ss are frozen in place!", victim, kEncounters[_mapMonsters[_teamMonsterIdArray[windowId]]._monsterRef]._name);
			} else {
				buffer1 = Common::String::format("%d %s is frozen in place!", victim, kEncounters[_mapMonsters[_teamMonsterIdArray[windowId]]._monsterRef]._name);
			}
			_messageToBePrinted += buffer1;
			// </CHECKME>
		}

		varA6 = true;
		break;
	case 2:
		if (argA == 2) {
			displayString_3("A serene feeling passes through the air...", false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The combat pauses...as there is a moment of forgiveness...";
			_unkArray2C8AA[0] = 0;
		}

		varA6 = true;
		break;
	case 4: // "Unholy Sinwave", "Holy Water"
		if (argA == 2) {
			displayString_3("A dark sense fills your soul...then fades!", false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  A dark gray fiery whirlwind surrounds the poor victim...the power fades and death abounds!";
			if (getRandom(100) < 50) {
				for (uint counter = 0; counter < 9; ++counter) {
					if (getRandom(100) < 50) {
						_mapMonsters[_teamMonsterIdArray[windowId]]._pictureRef[counter] = 0;
					}
				}
			} else {
				for (uint counter = 0; counter < 9; ++counter) {
					if (isMonsterActive(windowId, counter)) {
						if (getRandom(100) < 50) {
							_mapMonsters[_teamMonsterIdArray[windowId]]._pictureRef[counter] = 0;
						}
						break;
					}
				}
			}
		}
		varA6 = true;
		break;
	case 5: // "Lucifer'sTouch", "Book of Death", "Holy Cross"
		if (argA == 2) {
			displayString_3("A dark sense fills your soul...then fades!", false, charId, windowId, menuId, curMenuLine);
		} else {
			if (getRandom(100) < 50) {
				_messageToBePrinted += "  A dark fiery whirlwind surrounds the poor victim...the power fades and all targeted die!";
				for (uint counter = 0; counter < 9; ++counter) {
					_mapMonsters[_teamMonsterIdArray[windowId]]._pictureRef[counter] = 0;
				}
			} else {
				_messageToBePrinted += "  A dark fiery whirlwind surrounds the poor victim...the power fades and one victim dies!";
				for (uint counter = 0; counter < 9; ++counter) {
					if (isMonsterActive(windowId, counter)) {
						_mapMonsters[_teamMonsterIdArray[windowId]]._pictureRef[counter] = 0;
					}
				}
			}
		}

		varA6 = true;
		break;
	case 12: // "Terror Gaze", "Servitude Rod", "Despair Ankh", "ConfusionPrism", "Pipe of Peace", "Red Cape", "Peace Symbol", "Hell Badge"
		if (argA == 2) {
			displayString_3("There is no apparent affect!", false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += "  The magic sparkles brilliant hues in the air!";
			sub1E028(windowId, _items[itemId].field17_attackTypeDefense, true);
		}
		varA6 = true;
		break;
	case 14: { // "Feathered Cap"
		int16 varAA;
		if (argA == 2) {
			displayString_3("Who will use the item?", false, charId, windowId, menuId, curMenuLine);
			varAA = selectOtherCharFromTeam();
		} else {
			varAA = windowId;
		}

		if (varAA != 0x1B) {
			buffer1 = "  The magic makes the user as quick and agile as a bird!";
			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
			}
			_word32482[varAA] -= 50;
			if (_word32482[varAA] < 0)
				_word32482[varAA] = 0;
		}

		varA6 = true;
		}
		break;
	case 15: { // "Regal Crown"
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use the item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = windowId;
		}

		if (teamCharId != 0x1B) {
			buffer1 = "  The magic makes the user invisible!";
			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
			}

			_teamPctVisible[teamCharId] -= 50;
			if (_teamPctVisible[teamCharId] < 0)
				_teamPctVisible[teamCharId] = 0;
		}

		varA6 = true;
		}
		break;
	case 16: { // Fairy Dust
		_mapPosX = getRandom(_largeMapFlag ? 63 : 23);
		_mapPosY = getRandom(_largeMapFlag ? 63 : 23);
		int16 varAE = sub15538(_mapPosX, _mapPosY);

		if (_tileFact[varAE]._field0 == 0) {
			totalPartyKill();
			buffer1 = "The entire party vanishes in a flash... only to appear in stone !";
			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
			// emptyFunction(2);
		} else {
			if (varAE == 0 || varAE == 0x48) {
				buffer1 = "The entire party vanishes in a flash...but re-appears, as if nothing happened!";
				if (argA == 2) {
					displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			} else {
				buffer1 = "The entire party vanishes in a flash...only to appear elsewhere!";
				if (argA == 2) {
					displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			}
		}

		varA6 = true;
		}
		break;
	case 17: { // "Devil Dust"
		_mapPosX = _items[itemId].field_19;
		_mapPosY = _items[itemId].field_1A;
		int16 varAE = sub15538(_mapPosX, _mapPosY);
		if (_tileFact[varAE]._field0 == 0) {
			totalPartyKill();
			buffer1 = "The entire party vanishes in a flash... only to appear in stone !";
			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
			// emptyFunction(2);
		} else {
			if (varAE == 0 || varAE == 0x48) {
				buffer1 = "The entire party vanishes in a flash...but re-appears, as if nothing happened!";
				if (argA == 2) {
					displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			} else {
				buffer1 = "The entire party vanishes in a flash...only to appear elsewhere!";
				if (argA == 2) {
					displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
					retVal = true;
				}
			}
		}

		varA6 = true;
		}
		break;
	case 18:
		if (argA == 2) {
			displayString_3("The item makes a loud noise!", false, charId, windowId, menuId, curMenuLine);
		} else {
			int16 teamCharId = windowId;
			if (teamCharId != 0x1B) {
				if (_teamCharStatus[teamCharId]._status == 2) { // frozen
					_messageToBePrinted += "  The item makes a loud noise, awakening the character!";
					_teamCharStatus[teamCharId]._status = 0;
					_teamCharStatus[teamCharId]._duration = 0;
				} else {
					_messageToBePrinted += "  The item makes a loud noise, but has no effect!";
				}
			}
		}

		varA6 = true;
		break;
	case 19: // "Junk"
		buffer1 = "  * The item breaks!";
		if (argA == 2) {
			displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
		}
		setCharacterObjectToBroken(charId, objectId);
		varA6 = true;
		break;
	case 23: // "Divining Rod"
		buffer1 = Common::String::format("The %s says, '", _items[itemId]._name);
		if (_items[itemId].field_19 < _mapPosX) {
			if (_items[itemId].field_1A < _mapPosY) {
				buffer1 += "North West!";
			} else if (_items[itemId].field_1A > _mapPosY) {
				buffer1 += "South West!";
			} else {
				buffer1 += "West!";
			}
		} else if (_items[itemId].field_19 > _mapPosX) {
			if (_items[itemId].field_1A < _mapPosY) {
				buffer1 += "North East!";
			} else if (_items[itemId].field_1A > _mapPosY) {
				buffer1 += "South East!";
			} else {
				buffer1 += "East!";
			}
		} else { // equals _mapPosX
			if (_items[itemId].field_1A < _mapPosY) {
				buffer1 += "North!";
			} else if (_items[itemId].field_1A > _mapPosY) {
				buffer1 += "South!";
			} else {
				buffer1 += "Here!!!";
			}
		}
		buffer1 += "'";
		if (argA == 2) {
			displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		varA6 = true;
		break;
	case 24: {
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use this item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else
			teamCharId = windowId;

		if (teamCharId != 0x1B) {
			uint8 varAE = _items[itemId].field17_attackTypeDefense;
			uint8 effectPoints = getRandom(_items[itemId].field_19);
			_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] += effectPoints;
			if (_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] > 20) {
				_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] = 20;
			}
			if (effectPoints > 1)
				buffer1 = Common::String::format("%s increased %d points!", kSkillArray[varAE], effectPoints);
			else
				buffer1 = Common::String::format("%s increased 1 point!", kSkillArray[varAE]);

			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		}

		varA6 = true;
		}
		break;
	case 25: {
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use this item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else
			teamCharId = windowId;

		if (teamCharId != 0x1B) {
			uint8 varAE = _items[itemId].field17_attackTypeDefense;
			uint8 effectPoints = getRandom(_items[itemId].field_19);
			_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] -= effectPoints;
			if (_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] > 20 || _npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] < 0) {
				_npcBuf[_teamCharId[teamCharId]]._activeScore[varAE] = 1;
			}
			if (effectPoints > 1)
				buffer1 = Common::String::format("%s lowered %d points!", kSkillArray[varAE], effectPoints);
			else
				buffer1 = Common::String::format("%s lowered 1 point!", kSkillArray[varAE]);

			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
		}

		varA6 = true;
		}
		break;
	case 26: // "Black Sphere"
		buffer1 = "The entire party collapses, dead!!!";
		if (argA == 2) {
			displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}
		totalPartyKill();
		// emptyFunction(2);
		varA6 = true;
		break;
	case 27: { // "Magic Pyramid", "Razor Blade"
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use the item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = windowId;
		}

		if (teamCharId != 0x1B) {
			_npcBuf[_teamCharId[teamCharId]]._hitPoints = 0;
			buffer1 = Common::String::format("%s collapses, dead!!!", _npcBuf[_teamCharId[teamCharId]]._name);
			if (argA == 2) {
				displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
			} else {
				_messageToBePrinted += buffer1;
				retVal = true;
			}
			// emptyFunction(2);
		}

		varA6 = true;
		}
		break;
	case 28: // "Bugle"
		if (argA == 2) {
			displayString_3("The item makes a loud noise!", false, charId, windowId, menuId, curMenuLine);
		} else {
			int16 teamCharId = windowId;
			if (teamCharId != 0x1B) {
				if (_teamCharStatus[teamCharId]._status == 0) {
					_messageToBePrinted += "  The item makes a loud noise, awakening the character!";
					_teamCharStatus[teamCharId]._status = 0;
					_teamCharStatus[teamCharId]._duration = 0;
				} else {
					_messageToBePrinted += "  The item makes a loud noise, but has no effect!";
				}
			}
		}

		varA6 = true;
		break;
	case 29: { // "Healing Spray", "Healing Elixir", "Curing Potion", "Magic Potion"
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use the item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = windowId;
		}

		if (teamCharId != 0x1B) {
			int16 effectPoints = getRandom(_items[itemId].field17_attackTypeDefense);
			_npcBuf[_teamCharId[teamCharId]]._hitPoints += effectPoints;
			if (_npcBuf[_teamCharId[teamCharId]]._hitPoints > _npcBuf[_teamCharId[teamCharId]]._maxHP)
				_npcBuf[_teamCharId[teamCharId]]._hitPoints = _npcBuf[_teamCharId[teamCharId]]._maxHP;

			if (effectPoints > 1)
				buffer1 = Common::String::format("%s is healed %d points!", _npcBuf[_teamCharId[teamCharId]]._name, effectPoints);
			else
				buffer1 = Common::String::format("%s is healed 1 point!", _npcBuf[_teamCharId[teamCharId]]._name);
		}

		if (argA == 2) {
			displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		varA6 = true;
		}
		break;
	case 30: {
		int16 teamCharId;
		if (argA == 2) {
			displayString_3("Who will use the item?", false, charId, windowId, menuId, curMenuLine);
			teamCharId = selectOtherCharFromTeam();
		} else {
			teamCharId = windowId;
		}

		if (teamCharId != 0x1B) {
			int16 effectPoints = getRandom(_items[itemId].field17_attackTypeDefense);
			_npcBuf[_teamCharId[teamCharId]]._hitPoints -= effectPoints;
			if (_npcBuf[_teamCharId[teamCharId]]._hitPoints < 0)
				_npcBuf[_teamCharId[teamCharId]]._hitPoints = 0;

			if (effectPoints > 1)
				buffer1 = Common::String::format("%s is harmed for %d points!", _npcBuf[_teamCharId[teamCharId]]._name, effectPoints);
			else
				buffer1 = Common::String::format("%s is harmed for 1 point!", _npcBuf[_teamCharId[teamCharId]]._name);
		}

		if (argA == 2) {
			displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
		} else {
			_messageToBePrinted += buffer1;
			retVal = true;
		}

		varA6 = true;

		}
		break;
	case 3:
	case 6:
	case 7:
	case 8:
	case 9:
	case 10:
	case 11:
	case 13:
	case 20:
	case 21:
	case 22:
	default:
		break;
	}

	if (varA6) {
		if ((_npcBuf[charId]._inventory[objectId]._stat1 & 0x7F) != 0x7F) {
			int8 varA1 = (_npcBuf[charId]._inventory[objectId]._stat1 & 0x7F) - 1;
			if (varA1 <= 0) {
				buffer1 = "  * The item breaks!";
				if (argA == 2) {
					getLastCharAfterAnimCount(_guessAnimationAmount);
					displayString_3(buffer1, false, charId, windowId, menuId, curMenuLine);
				} else {
					_messageToBePrinted += buffer1;
				}
				setCharacterObjectToBroken(charId, objectId);
			} else {
				_npcBuf[charId]._inventory[objectId]._stat1 &= 0x80;
				_npcBuf[charId]._inventory[objectId]._stat1 |= 0xA1;
			}
		}

		if (argA == 2) {
			getLastCharAfterAnimCount(_guessAnimationAmount);
			sub18E80(charId, windowId, menuId, curMenuLine);
		}
	}

	return retVal;
}

int16 EfhEngine::handleStatusMenu(int16 gameMode, int16 charId) {
	debug("handleStatusMenu %d %d", gameMode, charId);

	int16 menuId = 9;
	int16 selectedLine = -1;
	int16 windowId = -1;
	int16 curMenuLine = -1;
	bool var10 = false;
	bool var2 = false;

	saveAnimImageSetId();

	_statusMenuActive = true;
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
					windowId = 8;
					var19 = Common::KEYCODE_RETURN;
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
				// in the sub-menus, only a list of selectable items is displayed
				if (var19 >= Common::KEYCODE_a && var19 <= Common::KEYCODE_z) {
					int16 var8 = var19 - Common::KEYCODE_a;
					if (var8 < _menuItemCounter) {
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
						var10 = true;
					else {
						_menuDepth = 1;
						curMenuLine = 0;
					}
				} else if (_menuDepth == 1) {
					if (_menuItemCounter == 0) {
						_menuDepth = 0;
						curMenuLine = -1;
						menuId = 9;
						unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);
					} else {
						selectedLine = curMenuLine;
						var10 = true;
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
					if (_menuItemCounter != 0) {
						++curMenuLine;
						if (curMenuLine > _menuItemCounter - 1)
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
					if (_menuItemCounter != 0) {
						--curMenuLine;
						if (curMenuLine < 0)
							curMenuLine = _menuItemCounter - 1;
					}
				}
				break;
			default:
				break;
			}

			if (curMenuLine == -1)
				unk_StatusMenu(windowId, menuId, curMenuLine, charId, false, true);
			else
				unk_StatusMenu(windowId, menuId, curMenuLine, charId, true, true);

		} while (!var10);

		bool validationFl = true;

		int16 objectId;
		int16 itemId;
		switch (menuId) {
		case 0:
			objectId = _word3273A[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			sub191FF(charId, objectId, windowId, menuId, curMenuLine);
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_statusMenuActive = false;
				return 0x7D00;
			}
			break;
		case 1:
			objectId = _word3273A[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (gameMode == 2) {
				restoreAnimImageSetId();
				_statusMenuActive = false;
				return objectId;
			}

			if (sub22293(_mapPosX, _mapPosY, charId, itemId, 2, -1)) {
				_statusMenuActive = false;
				return -1;
			}

			sub19E2E(charId, objectId, windowId, menuId, curMenuLine, 2);
			break;
		case 2:
			objectId = _word3273A[selectedLine];
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
							_statusMenuActive = false;
							return -1;
						}
					}
				}
			}

			break;
		case 3:
			objectId = _word3273A[selectedLine];
			itemId = _npcBuf[charId]._inventory[objectId]._ref;
			if (hasObjectEquipped(charId, objectId) && isItemCursed(itemId)) {
				displayString_3("The item is cursed!  IT IS EVIL!!!!!!!!", true, charId, windowId, menuId, curMenuLine);
			} else if (hasObjectEquipped(charId, objectId)) {
				displayString_3("Item is Equipped!  Trade anyway?", false, charId, windowId, menuId, curMenuLine);
				if (!getValidationFromUser())
					validationFl = false;
				sub18E80(charId, windowId, menuId, curMenuLine);

				if (validationFl) {
					bool var6;
					int16 var8;
					do {
						if (_teamCharId[2] != -1) {
							var8 = displayString_3("Who will you give the item to?", false, charId, windowId, menuId, curMenuLine);
							var2 = false;
						} else if (_teamCharId[1]) {
							var8 = 0x1A;
							var2 = false;
						} else {
							var2 = true;
							if (_teamCharId[0] == charId)
								var8 = 1;
							else
								var8 = 0;
						}

						if (var8 != 0x1A && var8 != 0x1B) {
							var6 = giveItemTo(_teamCharId[var8], objectId, charId);
							if (!var6) {
								displayString_3("That character cannot carry anymore!", false, charId, windowId, menuId, curMenuLine);
								getLastCharAfterAnimCount(_guessAnimationAmount);
							}
						} else {
							if (var8 == 0x1A) {
								displayString_3("No one to trade with!", false, charId, windowId, menuId, curMenuLine);
								getLastCharAfterAnimCount(_guessAnimationAmount);
								var8 = 0x1B;
							}
							var6 = false;
						}
					} while (!var6 && !var2 && var8 != 0x1B);

					if (var6) {
						removeObject(charId, objectId);
						if (gameMode == 2) {
							restoreAnimImageSetId();
							_statusMenuActive = false;
							return 0x7D00;
						}
					}

					sub18E80(charId, windowId, menuId, curMenuLine);
				}
			}
			break;
		case 4:
			objectId = _word3273A[selectedLine];
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
						_statusMenuActive = false;
						return 0x7D00;
					}

					bool var8 = sub22293(_mapPosX, _mapPosY, charId, itemId, 1, -1);
					if (var8) {
						_statusMenuActive = false;
						return -1;
					}
				}
			}
			break;
		case 5:
			objectId = _word3273A[selectedLine];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_statusMenuActive = false;
					return -1;
				}
			}
			break;
		case 6: // Identical to case 5?
			objectId = _word3273A[selectedLine];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_statusMenuActive = false;
					return -1;
				}
			}
			break;
		case 7: // Identical to case 5?
			objectId = _word3273A[selectedLine];
			if (gameMode == 2) {
				displayString_3("Not a Combat Option!", true, charId, windowId, menuId, curMenuLine);
			} else {
				bool var8 = sub22293(_mapPosX, _mapPosY, charId, objectId, 4, -1);
				if (var8) {
					_statusMenuActive = false;
					return -1;
				}
			}
			break;
		default:
			break;
		}

		if (menuId != 8) {
			var10 = false;
			_menuDepth = 0;
			menuId = 9;
			selectedLine = -1;
			curMenuLine = -1;
		}

		if (menuId == 8) {
			restoreAnimImageSetId();
			_statusMenuActive = false;
			return 0x7FFF;
		}
	}

	return 0;
}

bool EfhEngine::checkMonsterCollision() {
	debug("checkMonsterCollision");

	int16 var68 = 0;

	for (uint monsterId = 0; monsterId < 64; ++monsterId) {
		if (!checkPictureRefAvailability(monsterId))
			continue;

		if (!(_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == 0xFE) && !(!_largeMapFlag && _mapMonsters[monsterId]._guess_fullPlaceId == _fullPlaceId))
			continue;

		if ((_mapMonsters[monsterId]._field_1 & 0x3F) > 0x3D
		&& (((_mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F) == 0x3F) || isCharacterATeamMember(_mapMonsters[monsterId]._field_1)))
			continue;

		if (_mapMonsters[monsterId]._posX != _mapPosX || _mapMonsters[monsterId]._posY != _mapPosY)
			continue;

		_mapPosX = _oldMapPosX;
		_mapPosY = _oldMapPosY;
		if (_imageSetSubFilesIdx != _oldImageSetSubFilesIdx)
			_oldImageSetSubFilesIdx = _imageSetSubFilesIdx;
		_redrawNeededFl = true;

		int16 var6A = 0;
		for (uint var6C = 0; var6C < 9; ++var6C) {
			if (_mapMonsters[monsterId]._pictureRef[var6C])
				++var6A;
		}

		Common::String buffer = "";
		do {
			for (uint var6C = 0; var6C < 2; ++var6C) {
				int16 var1 = _mapMonsters[monsterId]._possessivePronounSHL6 & 0x3F;
				Common::String dest;
				if (var1 <= 0x3D) {
					dest = kEncounters[_mapMonsters[monsterId]._monsterRef]._name;
					if (var6A > 1)
						dest += "s";

					buffer = Common::String::format("with %d ", var6A) + dest;
				} else if (var1 == 0x3E) {
					buffer = "(NOT DEFINED)";
					dest = "(NOT DEFINED)";
				} else if (var1 == 0x3F) { // Useless check, it's the last possible value
					// Special character name
					dest = _npcBuf[_mapMonsters[monsterId]._field_1]._name;
					buffer = Common::String("with ") + dest;
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
				_tempTextPtr = nullptr;
				drawGameScreenAndTempText(true);
				break;
			case Common::KEYCODE_t: // Talk
				sub221D2(monsterId);
				var68 = true;
				break;
			default:
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
	debugC(3, kDebugEngine, "loadImageSetToTileBank %d %d", tileBankId, imageSetId);

	// TODO: all the values of titleBankId and imageSetId are hardcoded. When all the calls are implemented, fix the values to avoid to have to decrease them
	int16 bankId = tileBankId - 1;
	int16 setId = imageSetId - 1;

	if (_currentTileBankImageSetId[bankId] == setId)
		return;

	_currentTileBankImageSetId[bankId] = setId;

	if (bankId == 0 || bankId == 1)
		_mapBitmapRefArr[_techId][bankId] = setId;

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

void EfhEngine::saveEfhGame() {
	warning("STUB - saveEfhGame");
	openMainMenuDialog();
}

uint8 EfhEngine::getMapTileInfo(int16 mapPosX, int16 mapPosY) {
	debugC(3, kDebugEngine, "getMapTileInfo %d-%d", mapPosX, mapPosY);

	if (_largeMapFlag)
		return _mapGameMap[mapPosX][mapPosY];

	return _curPlace[mapPosX][mapPosY];
}

void EfhEngine::displayNextAnimFrame() {
	debug("displayNextAnimFrame");

	if (++_unkAnimRelatedIndex >= 15)
		_unkAnimRelatedIndex = 0;

	displayAnimFrame();
}

void EfhEngine::writeTechAndMapFiles() {
	warning("STUB - writeTechAndMapFiles");
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
