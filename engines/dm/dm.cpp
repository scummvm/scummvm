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
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*
*/

/*
* Based on the Reverse Engineering work of Christophe Fontanel,
* maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
*/

#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "engines/util.h"
#include "engines/engine.h"
#include "graphics/palette.h"
#include "common/file.h"
#include "common/events.h"
#include "common/array.h"
#include "common/algorithm.h"

#include "dm/dm.h"
#include "gfx.h"
#include "dungeonman.h"
#include "eventman.h"
#include "menus.h"
#include "champion.h"
#include "loadsave.h"
#include "objectman.h"
#include "inventory.h"
#include "text.h"
#include "movesens.h"
#include "group.h"
#include "timeline.h"
#include "projexpl.h"
#include "dialog.h"

namespace DM {
void warning(bool repeat, const char* s, ...) {
	va_list va;

	va_start(va, s);
	Common::String output = Common::String::vformat(s, va);
	va_end(va);

	if (repeat) {
		::warning(output.c_str());
	} else {
		static Common::Array<Common::String> stringsPrinted;

		if (Common::find(stringsPrinted.begin(), stringsPrinted.end(), s) == stringsPrinted.end()) {
			stringsPrinted.push_back(output);
			::warning(output.c_str());
		}
	}
}

void turnDirRight(Direction &dir) {
	dir = (Direction)((dir + 1) & 3);
}

void turnDirLeft(Direction &dir) {
	dir = (Direction)((dir - 1) & 3);
}

Direction returnOppositeDir(Direction dir) {
	return (Direction)((dir + 2) & 3);
}

uint16 returnPrevVal(uint16 val) {
	return (Direction)((val + 3) & 3);
}

uint16 returnNextVal(uint16 val) {
	return (val + 1) & 0x3;
}

bool isOrientedWestEast(Direction dir) {
	return dir & 1;
}

uint16 toggleFlag(uint16& val, uint16 mask) {
	return val ^= mask;
}

uint16 M75_bitmapByteCount(uint16 pixelWidth, uint16 height) {
	return pixelWidth / 2 * height;
}

uint16 M21_normalizeModulo4(uint16 val) {
	return val & 3;
}

int32 M30_time(int32 mapTime) {
	return mapTime & 0x00FFFFFF;
}

int32 M33_setMapAndTime(int32 &mapTime, uint32 map, uint32 time) {
	return (mapTime) = ((time) | (((long)(map)) << 24));
}

uint16 M29_map(int32 mapTime) {
	return ((uint16)((mapTime) >> 24));
}

Thing M15_thingWithNewCell(Thing thing, int16 cell) {
	return Thing(((thing.toUint16()) & 0x3FFF) | ((cell) << 14));
}

int16 M38_distance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2) {
	return ABS(mapx1 - mapx2) + ABS(mapy1 - mapy2);
}

DMEngine::DMEngine(OSystem *syst) : Engine(syst), _console(nullptr) {
// Do not load data files
// Do not initialize graphics here
// Do not initialize audio devices here
// Do these from run

//Specify all default directories
//const Common::FSNode gameDataDir(ConfMan.get("example"));
//SearchMan.addSubDirectoryMatching(gameDataDir, "example2");
	DebugMan.addDebugChannel(kDMDebugExample, "example", "example desc");

	// register random source
	_rnd = new Common::RandomSource("dm");

	_dungeonMan = nullptr;
	_displayMan = nullptr;
	_eventMan = nullptr;
	_menuMan = nullptr;
	_championMan = nullptr;
	_objectMan = nullptr;
	_inventoryMan = nullptr;
	_textMan = nullptr;
	_moveSens = nullptr;
	_groupMan = nullptr;
	_timeline = nullptr;
	_projexpl = nullptr;
	_displayMan = nullptr;

	_engineShouldQuit = false;
	_g528_saveFormat = 0;
	_g527_platform = 0;
	_g526_dungeonId = 0;

	_g298_newGame = false;
	_g523_restartGameRequest = false;
	_g321_stopWaitingForPlayerInput = true;
	_g301_gameTimeTicking = false;
	_g524_restartGameAllowed = false;
	_g525_gameId = 0;
	_g331_pressingEye = false;
	_g332_stopPressingEye = false;
	_g333_pressingMouth = false;
	_g334_stopPressingMouth = false;
	_g340_highlightBoxInversionRequested = false;
	_g311_projectileDisableMovementTicks = 0;
	_g312_lastProjectileDisabledMovementDirection = 0;
	_g302_gameWon = false;
	_g327_newPartyMapIndex = kM1_mapIndexNone;
	_g325_setMousePointerToObjectInMainLoop = false;
	_g310_disabledMovementTicks = 0;
	_g313_gameTime = 0;
	_g353_stringBuildBuffer[0] = '\0';
	_g318_waitForInputMaxVerticalBlankCount = 0;
	_savedScreenForOpenEntranceDoors = nullptr;
	for (uint16 i = 0; i < 10; ++i)
		_g562_entranceDoorAnimSteps[i] = nullptr;
	_g564_interfaceCredits = nullptr;
	debug("DMEngine::DMEngine");
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	delete _console;
	delete _displayMan;
	delete _dungeonMan;
	delete _eventMan;
	delete _menuMan;
	delete _championMan;
	delete _objectMan;
	delete _inventoryMan;
	delete _textMan;
	delete _moveSens;
	delete _groupMan;
	delete _timeline;
	delete _projexpl;
	delete _dialog;

	for (uint16 i = 0; i < k34_D13_soundCount; ++i)
		delete[] _gK24_soundData[i]._firstSample;

	delete[] _savedScreenForOpenEntranceDoors;
	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

bool DMEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsLoadingDuringRuntime);
}

void DMEngine::f22_delay(uint16 verticalBlank) {
	_system->delayMillis(verticalBlank * 20); // Google says most Amiga games had a refreshrate of 50 hz
}

uint16 DMEngine::f30_getScaledProduct(uint16 val, uint16 scale, uint16 vale2) {
	return ((uint32)val * vale2) >> scale;
}

void DMEngine::f463_initializeGame() {
	_displayMan->f479_loadGraphics();
	_displayMan->f460_initializeGraphicData();
	warning(false, "Dummy code in f463_initializeGame, setting palette");
	_displayMan->loadPalette(g21_PalDungeonView[0]);
	_displayMan->f94_loadFloorSet(k0_FloorSetStone);
	_displayMan->f95_loadWallSet(k0_WallSetStone);
	f503_loadSounds();
	f437_STARTEND_drawTittle();
	_textMan->f54_textInitialize();
	_objectMan->loadObjectNames();
	_eventMan->initMouse();
	do {
		f441_processEntrance();
		if (_engineShouldQuit)
			return;
	} while (f435_loadgame(1) != k1_LoadgameSuccess);
	//F0396_MENUS_LoadSpellAreaLinesBitmap() is not needed, every bitmap has been loaded

	// There was some memory wizardy for the Amiga platform, I skipped that part
	_displayMan->f461_allocateFlippedWallBitmaps();

	f462_startGame();
	if (_g298_newGame)
		_moveSens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
	_eventMan->f78_showMouse();
	_eventMan->f357_discardAllInput();
}

void DMEngine::f448_initMemoryManager() {
	warning(false, "STUB METHOD: f448_initMemoryManager");
	for (uint16 i = 0; i < 16; ++i)
		_displayMan->_g347_paletteTopAndBottomScreen[i] = g21_PalDungeonView[0][i];
}

void DMEngine::f462_startGame() {
	static Box boxScreenTop(0, 319, 0, 32); // @ G0061_s_Graphic562_Box_ScreenTop
	static Box boxScreenRight(224, 319, 33, 169); // @ G0062_s_Graphic562_Box_ScreenRight
	static Box boxScreenBottom(0, 319, 169, 199); // @ G0063_s_Graphic562_Box_ScreenBottom

	_g331_pressingEye = false;
	_g332_stopPressingEye = false;
	_g333_pressingMouth = false;
	_g334_stopPressingMouth = false;
	_g340_highlightBoxInversionRequested = false;
	_eventMan->_g341_highlightBoxEnabled = false;
	_championMan->_g300_partyIsSleeping = false;
	_championMan->_g506_actingChampionOrdinal = M0_indexToOrdinal(kM1_ChampionNone);
	_menuMan->_g509_actionAreaContainsIcons = true;
	_eventMan->_g599_useChampionIconOrdinalAsMousePointerBitmap = M0_indexToOrdinal(kM1_ChampionNone);

	_eventMan->_g441_primaryMouseInput = g447_PrimaryMouseInput_Interface;
	_eventMan->_g442_secondaryMouseInput = g448_SecondaryMouseInput_Movement;
	_eventMan->_g443_primaryKeyboardInput = g458_primaryKeyboardInput_interface;
	_eventMan->_g444_secondaryKeyboardInput = g459_secondaryKeyboardInput_movement;

	f3_processNewPartyMap(_dungeonMan->_g309_partyMapIndex);

	if (!_g298_newGame) {
		_displayMan->_g578_useByteBoxCoordinates = false;
		f22_delay(1);
		_displayMan->D24_fillScreenBox(boxScreenTop, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(boxScreenRight, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(boxScreenBottom, k0_ColorBlack);
	} else {
		_displayMan->_g578_useByteBoxCoordinates = false;
		_displayMan->D24_fillScreenBox(boxScreenTop, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(boxScreenRight, k0_ColorBlack);
		_displayMan->D24_fillScreenBox(boxScreenBottom, k0_ColorBlack);
	}

	warning(false, "TODO: build copper");
	_menuMan->f395_drawMovementArrows();
	_championMan->f278_resetDataToStartGame();
	_g301_gameTimeTicking = true;
}

void DMEngine::f3_processNewPartyMap(uint16 mapIndex) {
	_groupMan->f194_removeAllActiveGroups();
	_dungeonMan->f174_setCurrentMapAndPartyMap(mapIndex);
	_displayMan->f96_loadCurrentMapGraphics();
	_groupMan->f195_addAllActiveGroups();
	_inventoryMan->f337_setDungeonViewPalette();
}

Common::Error DMEngine::run() {
	initArrays();

	// scummvm/engine specific
	initGraphics(320, 200, false);
	_console = new Console(this);
	_displayMan = new DisplayMan(this);
	_dungeonMan = new DungeonMan(this);
	_eventMan = new EventManager(this);
	_menuMan = new MenuMan(this);
	_championMan = new ChampionMan(this);
	_objectMan = new ObjectMan(this);
	_inventoryMan = new InventoryMan(this);
	_textMan = new TextMan(this);
	_moveSens = new MovesensMan(this);
	_groupMan = new GroupMan(this);
	_timeline = new Timeline(this);
	_projexpl = new ProjExpl(this);
	_dialog = new DialogMan(this);
	_displayMan->setUpScreens(320, 200);

	f463_initializeGame();
	while (true) {
		f2_gameloop();
		if (_engineShouldQuit)
			return Common::kNoError;
		f444_endGame(_championMan->_g303_partyDead);
	}

	return Common::kNoError;
}

void DMEngine::f2_gameloop() {
	//HACK: Remove this block to get real starting position in the Hall of Champions
	if (_g298_newGame) {
		warning(false, "DUMMY CODE: SETTING PARTY POS AND DIRECTION");
		_dungeonMan->_g306_partyMapX = 9;
		_dungeonMan->_g307_partyMapY = 9;
		_dungeonMan->_g308_partyDir = kDirWest;
	}

	_g318_waitForInputMaxVerticalBlankCount = 10;
	while (true) {
		if (_engineShouldQuit)
			return;

		for (;;) {
			if (_g327_newPartyMapIndex != kM1_mapIndexNone) {
				f3_processNewPartyMap(_g327_newPartyMapIndex);
				_moveSens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
				_g327_newPartyMapIndex = kM1_mapIndexNone;
				_eventMan->f357_discardAllInput();
			}
			_timeline->f261_processTimeline();

			if (_g327_newPartyMapIndex == kM1_mapIndexNone)
				break;
		}

		if (!_inventoryMan->_g432_inventoryChampionOrdinal && !_championMan->_g300_partyIsSleeping) {
			Box box(0, 223, 0, 135);
			warning(false, "DUMMY CODE: clearing screen to black");
			_displayMan->f135_fillBoxBitmap(_displayMan->_g296_bitmapViewport, box, k0_ColorBlack, k112_byteWidthViewport, k136_heightViewport); // dummy code
			_displayMan->f128_drawDungeon(_dungeonMan->_g308_partyDir, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
			if (_g325_setMousePointerToObjectInMainLoop) {
				_g325_setMousePointerToObjectInMainLoop = false;
				_eventMan->f78_showMouse();
				_eventMan->f68_setPointerToObject(_objectMan->_g412_objectIconForMousePointer);
				_eventMan->f77_hideMouse();

			}
			if (_eventMan->_g326_refreshMousePointerInMainLoop) {
				_eventMan->_g326_refreshMousePointerInMainLoop = false;
				_eventMan->_g598_mousePointerBitmapUpdated = true;
				_eventMan->f78_showMouse();
				_eventMan->f77_hideMouse();
			}
		}
		_eventMan->f363_highlightBoxDisable();
		f65_playPendingSound();
		_championMan->f320_applyAndDrawPendingDamageAndWounds();
		if (_championMan->_g303_partyDead)
			break;

		_g313_gameTime++;

		if (!(_g313_gameTime & 511))
			_inventoryMan->f338_decreaseTorchesLightPower();

		if (_championMan->_g407_party._freezeLifeTicks)
			_championMan->_g407_party._freezeLifeTicks -= 1;

		_menuMan->f390_refreshActionAreaAndSetChampDirMaxDamageReceived();

		if (!(_g313_gameTime & (_championMan->_g300_partyIsSleeping ? 15 : 63)))
			_championMan->f331_applyTimeEffects();

		if (_g310_disabledMovementTicks)
			_g310_disabledMovementTicks--;

		if (_g311_projectileDisableMovementTicks)
			_g311_projectileDisableMovementTicks--;

		_textMan->f44_messageAreaClearExpiredRows();
		_g321_stopWaitingForPlayerInput = false;
		uint16 vblankCounter = 0;
		do {
			_eventMan->processInput();

			if (_g332_stopPressingEye) {
				_g331_pressingEye = false;
				_g332_stopPressingEye = false;
				_inventoryMan->f353_drawStopPressingEye();
			} else if (_g334_stopPressingMouth) {
				_g333_pressingMouth = false;
				_g334_stopPressingMouth = false;
				_inventoryMan->f350_drawStopPressingMouth();
			}

			_eventMan->f380_processCommandQueue();
			if (_engineShouldQuit)
				return;
			_displayMan->updateScreen();
			if (!_g321_stopWaitingForPlayerInput) {
				_eventMan->f363_highlightBoxDisable();
			}

			_system->delayMillis(2);
			if (++vblankCounter >= _g318_waitForInputMaxVerticalBlankCount * 5)
				_g321_stopWaitingForPlayerInput = true;

		} while (!_g321_stopWaitingForPlayerInput || !_g301_gameTimeTicking);
	}
}

int16 DMEngine::M1_ordinalToIndex(int16 val) {
	return val - 1;
}

int16 DMEngine::M0_indexToOrdinal(int16 val) {
	return val + 1;
}

void DMEngine::f441_processEntrance() {
	_eventMan->_g441_primaryMouseInput = g445_PrimaryMouseInput_Entrance;
	_eventMan->_g442_secondaryMouseInput = nullptr;
	_eventMan->_g443_primaryKeyboardInput = nullptr;
	_eventMan->_g444_secondaryKeyboardInput = nullptr;
	_g562_entranceDoorAnimSteps[0] = new byte[128 * 161 * 12];
	for (uint16 idx = 1; idx < 8; idx++)
		_g562_entranceDoorAnimSteps[idx] = _g562_entranceDoorAnimSteps[idx - 1] + 128 * 161;

	_g562_entranceDoorAnimSteps[8] = _g562_entranceDoorAnimSteps[7] + 128 * 161;
	_g562_entranceDoorAnimSteps[9] = _g562_entranceDoorAnimSteps[8] + 128 * 161 * 2;

	_displayMan->f466_loadIntoBitmap(k3_entranceRightDoorGraphicIndice, _g562_entranceDoorAnimSteps[4]);
	_displayMan->f466_loadIntoBitmap(k2_entranceLeftDoorGraphicIndice, _g562_entranceDoorAnimSteps[0]);
	_g564_interfaceCredits = _displayMan->f489_getNativeBitmapOrGraphic(k5_creditsGraphicIndice);
	_displayMan->_g578_useByteBoxCoordinates = false;
	Box displayBox;
	displayBox._x1 = 0;
	displayBox._x2 = 100;
	displayBox._y1 = 0;
	displayBox._y2 = 160;
	for (uint16 idx = 1; idx < 4; idx++) {
		_displayMan->f132_blitToBitmap(_g562_entranceDoorAnimSteps[0], _g562_entranceDoorAnimSteps[idx], displayBox, idx << 2, 0, k64_byteWidth, k64_byteWidth, kM1_ColorNoTransparency, 161, 161);
		displayBox._x2 -= 4;
	}
	displayBox._x2 = 127;
	for (uint16 idx = 5; idx < 8; idx++) {
		displayBox._x1 += 4;
		_displayMan->f132_blitToBitmap(_g562_entranceDoorAnimSteps[4], _g562_entranceDoorAnimSteps[idx], displayBox, 0, 0, k64_byteWidth, k64_byteWidth, kM1_ColorNoTransparency, 161, 161);
	}

	do {
		f439_drawEntrance();
		_eventMan->f78_showMouse();
		_eventMan->f357_discardAllInput();
		_g298_newGame = k99_modeWaitingOnEntrance;
		do {
			_eventMan->processInput();
			if (_engineShouldQuit)
				return;
			_eventMan->f380_processCommandQueue();
			_displayMan->updateScreen();
		} while (_g298_newGame == k99_modeWaitingOnEntrance);
	} while (_g298_newGame == k202_CommandEntranceDrawCredits);

	//Strangerke: CHECKME: Earlier versions were using G0566_puc_Graphic534_Sound01Switch
	f060_SOUND_Play(k01_soundSWITCH, 112, 0x40, 0x40);
	f22_delay(20);
	_eventMan->f78_showMouse();
	if (_g298_newGame)
		f438_STARTEND_OpenEntranceDoors();

	delete[] _g562_entranceDoorAnimSteps[0];
	for (uint16 i = 0; i < 10; ++i)
		_g562_entranceDoorAnimSteps[i] = nullptr;
}

void DMEngine::f444_endGame(bool doNotDrawCreditsOnly) {
	// TODO: localization
	static Box restartOuterBox = Box(103, 217, 145, 159);
	static Box restartInnerBox = Box(105, 215, 147, 157);
	static Box theEndBox = Box(120, 199, 95, 108);
	static Box championMirrorBox = Box(11, 74, 7, 49);
	static Box championPortraitBox = Box(27, 58, 13, 41);

	bool waitBeforeDrawingRestart = true;

	_eventMan->f67_setMousePointerToNormal(k0_pointerArrow);
	_eventMan->f78_showMouse();
	_eventMan->_g441_primaryMouseInput = nullptr;
	_eventMan->_g442_secondaryMouseInput = nullptr;
	_eventMan->_g443_primaryKeyboardInput = nullptr;
	_eventMan->_g444_secondaryKeyboardInput = nullptr;
	if (doNotDrawCreditsOnly && !_g302_gameWon) {
		f064_SOUND_RequestPlay_CPSD(k06_soundSCREAM, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY, k0_soundModePlayImmediately);
		f22_delay(240);
	}

	if (_displayMan->_g322_paletteSwitchingEnabled) {
		uint16 oldPalTopAndBottomScreen[16];
		for (uint16 i = 0; i < 16; ++i)
			oldPalTopAndBottomScreen[i] = _displayMan->_g347_paletteTopAndBottomScreen[i];
		for (int i = 0; i <= 7; i++) {
			f22_delay(1);
			for (int colIdx = 0; colIdx < 16; colIdx++) {
				_displayMan->_g346_paletteMiddleScreen[colIdx] = _displayMan->f431_getDarkenedColor(_displayMan->_g346_paletteMiddleScreen[colIdx]);
				_displayMan->_g347_paletteTopAndBottomScreen[colIdx] = _displayMan->f431_getDarkenedColor(_displayMan->_g347_paletteTopAndBottomScreen[colIdx]);
			}
		}
		_displayMan->_g322_paletteSwitchingEnabled = false;
		f22_delay(1);
		for (uint16 i = 0; i < 16; ++i)
			_displayMan->_g347_paletteTopAndBottomScreen[i] = oldPalTopAndBottomScreen[i];
	} else
		_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_g345_aui_BlankBuffer);

	uint16 darkBluePalette[16];
	if (doNotDrawCreditsOnly) {
		if (_g302_gameWon) {
			// Strangerke: Related to portraits. Game data could be missing for earlier versions of the game.
			_displayMan->fillScreen(k12_ColorDarkestGray);
			for (int16 championIndex = k0_ChampionFirst; championIndex < _championMan->_g305_partyChampionCount; championIndex++) {
				int16 textPosY = championIndex * 48;
				Champion *curChampion = &_championMan->_gK71_champions[championIndex];
				_displayMan->f21_blitToScreen(_displayMan->f489_getNativeBitmapOrGraphic(k208_wallOrn_43_champMirror), &championMirrorBox, k32_byteWidth, k10_ColorFlesh, 43);
				_displayMan->f21_blitToScreen(curChampion->_portrait, &championPortraitBox, k16_byteWidth, k1_ColorDarkGary, 29);
				_textMan->f443_endgamePrintString(87, textPosY += 14, k9_ColorGold, curChampion->_name);
				int textPosX = (6 * strlen(curChampion->_name)) + 87;
				char championTitleFirstCharacter = curChampion->_title[0];
				if ((championTitleFirstCharacter != ',') && (championTitleFirstCharacter != ';') && (championTitleFirstCharacter != '-'))
					textPosX += 6;

				_textMan->f443_endgamePrintString(textPosX, textPosY++, k9_ColorGold, curChampion->_title);
				for (int16 idx = k0_ChampionSkillFighter; idx <= k3_ChampionSkillWizard; idx++) {
					uint16 skillLevel = MIN<uint16>(16, _championMan->f303_getSkillLevel(championIndex, idx | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience)));
					if (skillLevel == 1)
						continue;

					char displStr[20];
					strcpy(displStr, G0428_apc_SkillLevelNames[skillLevel - 2]);
					strcat(displStr, " ");
					strcat(displStr, g417_baseSkillName[idx]);
					_textMan->f443_endgamePrintString(105, textPosY = textPosY + 8, k13_ColorLightestGray, displStr);
				}
				championMirrorBox._y1 += 48;
				championMirrorBox._y2 += 48;
				championPortraitBox._y1 += 48;
				championPortraitBox._y1 += 48;
			}
			_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_g347_paletteTopAndBottomScreen);
			_engineShouldQuit = true;
			return;
		}
T0444017:
		_displayMan->fillScreen(k0_ColorBlack);
		_displayMan->f21_blitToScreen(_displayMan->f489_getNativeBitmapOrGraphic(k6_theEndIndice), &theEndBox, k40_byteWidth, kM1_ColorNoTransparency, 14);
		for (uint16 i = 0; i < 16; ++i)
			darkBluePalette[i] = D01_RGB_DARK_BLUE;
		uint16 curPalette[16];
		for (uint16 i = 0; i < 15; ++i)
			curPalette[i] = darkBluePalette[i];
		curPalette[15] = D09_RGB_WHITE;
		_displayMan->f436_STARTEND_FadeToPalette(curPalette);
		if (waitBeforeDrawingRestart)
			f22_delay(300);

		if (_g524_restartGameAllowed) {
			_displayMan->_g578_useByteBoxCoordinates = false;
			_displayMan->D24_fillScreenBox(restartOuterBox, k12_ColorDarkestGray);
			_displayMan->D24_fillScreenBox(restartInnerBox, k0_ColorBlack);
			_textMan->f53_printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "RESTART THIS GAME");
			curPalette[1] = D03_RGB_PINK;
			curPalette[4] = D09_RGB_WHITE;
			_eventMan->_g441_primaryMouseInput = g446_PrimaryMouseInput_RestartGame;
			_eventMan->f357_discardAllInput();
			_eventMan->f77_hideMouse();
			_displayMan->f436_STARTEND_FadeToPalette(curPalette);
			for (int16 verticalBlankCount = 900; --verticalBlankCount && !_g523_restartGameRequest; f22_delay(1))
				_eventMan->f380_processCommandQueue();

			_eventMan->f78_showMouse();
			if (_g523_restartGameRequest) {
				_displayMan->f436_STARTEND_FadeToPalette(darkBluePalette);
				_displayMan->fillScreen(k0_ColorBlack);
				_displayMan->f436_STARTEND_FadeToPalette(g21_PalDungeonView[0]);
				_g298_newGame = k0_modeLoadSavedGame;
				if (f435_loadgame(1) != kM1_LoadgameFailure) {
					f462_startGame();
					_g523_restartGameRequest = false;
					_eventMan->f77_hideMouse();
					_eventMan->f357_discardAllInput();
					return;
				}
			}
		}

		_displayMan->f436_STARTEND_FadeToPalette(darkBluePalette);
	}
	Box box(0, 319, 0, 199);
	_displayMan->f132_blitToBitmap(_displayMan->f489_getNativeBitmapOrGraphic(k5_creditsGraphicIndice), _displayMan->_g348_bitmapScreen, box, 0, 0, 160, 160, kM1_ColorNoTransparency);

	_displayMan->f436_STARTEND_FadeToPalette(g19_PalCredits);
	_eventMan->f541_waitForMouseOrKeyActivity();
	if (_engineShouldQuit)
		return;

	if (_g524_restartGameAllowed && doNotDrawCreditsOnly) {
		waitBeforeDrawingRestart = false;
		_displayMan->f436_STARTEND_FadeToPalette(darkBluePalette);
		goto T0444017;
	}

	_engineShouldQuit = true;
	return;
}


void DMEngine::f439_drawEntrance() {
	static Box doorsUpperHalfBox = Box(0, 231, 0, 80);
	static Box doorsLowerHalfBox = Box(0, 231, 81, 160);
	static Box closedDoorLeftBox = Box(0, 104, 30, 190);
	static Box closedDoorRightBox = Box(105, 231, 30, 190);

	byte *microDungeonCurrentMapData[32];

	_dungeonMan->_g309_partyMapIndex = k255_mapIndexEntrance;
	_displayMan->_g297_drawFloorAndCeilingRequested = true;
	_dungeonMan->_g273_currMapWidth = 5;
	_dungeonMan->_g274_currMapHeight = 5;
	_dungeonMan->_g271_currMapData = microDungeonCurrentMapData;

	Map map; // uninitialized, won't be used
	_dungeonMan->_g269_currMap = &map;
	Square microDungeonSquares[25];
	for (uint16 i = 0; i < 25; ++i)
		microDungeonSquares[i] = Square(k0_ElementTypeWall, 0);

	for (int16 idx = 0; idx < 5; idx++) {
		microDungeonCurrentMapData[idx] = (byte*)&microDungeonSquares[idx * 5];
		microDungeonSquares[idx + 10] = Square(k1_CorridorElemType, 0);
	}
	microDungeonSquares[7] = Square(k1_CorridorElemType, 0);
	_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_g345_aui_BlankBuffer);

	// note, a global variable is used here in the original
	_displayMan->f466_loadIntoBitmap(k4_entranceGraphicIndice, _displayMan->_g348_bitmapScreen);
	_displayMan->f128_drawDungeon(kDirSouth, 2, 0);
	warning(false, "IGNORED CODE: G0324_B_DrawViewportRequested = false;");

	if (!_savedScreenForOpenEntranceDoors)
		_savedScreenForOpenEntranceDoors = new byte[k200_heightScreen * k160_byteWidthScreen * 2];
	memcpy(_savedScreenForOpenEntranceDoors, _displayMan->_g348_bitmapScreen, 320 * 200);

	_displayMan->_g578_useByteBoxCoordinates = false, _displayMan->f132_blitToBitmap(_displayMan->_g348_bitmapScreen, _g562_entranceDoorAnimSteps[8], doorsUpperHalfBox, 0, 30, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);
	_displayMan->_g578_useByteBoxCoordinates = false, _displayMan->f132_blitToBitmap(_displayMan->_g348_bitmapScreen, _g562_entranceDoorAnimSteps[8], doorsLowerHalfBox, 0, 111, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);

	_displayMan->f21_blitToScreen(_g562_entranceDoorAnimSteps[0], &closedDoorLeftBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->f21_blitToScreen(_g562_entranceDoorAnimSteps[4], &closedDoorRightBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->f436_STARTEND_FadeToPalette(g20_PalEntrance);
}

void DMEngine::f438_STARTEND_OpenEntranceDoors() {
	Box rightDoorBox(109, 231, 33, 193);
	byte *rightDoorBitmap = _displayMan->f489_getNativeBitmapOrGraphic(k3_entranceRightDoorGraphicIndice);
	Box leftDoorBox(0, 100, 33, 193);
	uint16 leftDoorBlitFrom = 0;
	byte *leftDoorBitmap = _displayMan->f489_getNativeBitmapOrGraphic(k2_entranceLeftDoorGraphicIndice);

	Box screenBox(0, 319, 0, 199);

	for (uint16 animStep = 1; animStep < 32; ++animStep) {
		if ((animStep % 3) == 1) {
			// Strangerke: CHECKME: Earlier versions of the game were using G0565_puc_Graphic535_Sound02DoorRattle instead of k02_soundDOOR_RATTLE 2
			f060_SOUND_Play(k02_soundDOOR_RATTLE, 145, 0x40, 0x40);
		}

		_displayMan->f21_blitToScreen(_savedScreenForOpenEntranceDoors, &screenBox, 160, kM1_ColorNoTransparency, 200);
		_displayMan->f132_blitToBitmap(leftDoorBitmap, _displayMan->_g348_bitmapScreen, leftDoorBox, leftDoorBlitFrom, 0, 64, k160_byteWidthScreen, kM1_ColorNoTransparency);
		_displayMan->f132_blitToBitmap(rightDoorBitmap, _displayMan->_g348_bitmapScreen, rightDoorBox, 0, 0, 64, k160_byteWidthScreen, kM1_ColorNoTransparency);
		_eventMan->f357_discardAllInput();
		_displayMan->updateScreen();

		leftDoorBox._x2 -= 4;
		leftDoorBlitFrom += 4;
		rightDoorBox._x1 += 4;

		f22_delay(3);
	}
	delete[] _savedScreenForOpenEntranceDoors;
}




} // End of namespace DM
