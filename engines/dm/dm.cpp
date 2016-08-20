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

#include "common/config-manager.h"
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
#include <graphics/cursorman.h>
#include <advancedDetector.h>
#include "sounds.h"
#include <graphics/surface.h>
#include <common/translation.h>
#include <gui/saveload.h>

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

const char* debugGetDirectionName(Direction dir) {
	static const char* directionNames[] = {"North", "East", "South", "West"};
	if (dir < 0 || dir > 3)
		return "Invalid direction";
	return directionNames[dir];
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

DMEngine::DMEngine(OSystem *syst, const ADGameDescription *desc) : Engine(syst), _console(nullptr), _gameVersion(desc) {
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
	_sound = nullptr;

	_engineShouldQuit = false;
	_g528_saveFormat = 0;
	_g527_platform = 0;
	_g526_dungeonId = 0;

	_g298_newGame = 0;
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

	_saveThumbnail = nullptr;
	_canLoadFromGMM = false;
	_loadSaveSlotAtRuntime = -1;
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
	delete _sound;

	delete _saveThumbnail;

	delete[] _savedScreenForOpenEntranceDoors;
	// clear debug channels
	DebugMan.clearAllDebugChannels();
}

bool DMEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsLoadingDuringRuntime);
}

Common::Error DMEngine::loadGameState(int slot) {
	if (f435_loadgame(slot) != kM1_LoadgameFailure) {
		_displayMan->fillScreen(k0_ColorBlack);
		_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_palDungeonView[0]);
		_g298_newGame = k0_modeLoadSavedGame;

		f462_startGame();
		_g523_restartGameRequest = false;
		_eventMan->f77_hideMouse();
		_eventMan->f357_discardAllInput();
		return Common::kNoError;
	}

	return Common::kNoGameDataFoundError;
}

bool DMEngine::canLoadGameStateCurrently() {
	return _canLoadFromGMM;
}

void DMEngine::f22_delay(uint16 verticalBlank) {
	for (uint16 i = 0; i < verticalBlank * 2; ++i) {
		_eventMan->processInput();
		_displayMan->updateScreen();
		_system->delayMillis(10); // Google says most Amiga games had a refreshrate of 50 hz
	}
}

uint16 DMEngine::f30_getScaledProduct(uint16 val, uint16 scale, uint16 vale2) {
	return ((uint32)val * vale2) >> scale;
}

void DMEngine::f463_initializeGame() {
	f448_initMemoryManager();
	_displayMan->f479_loadGraphics();
	_displayMan->f460_initializeGraphicData();
	_displayMan->f94_loadFloorSet(k0_FloorSetStone);
	_displayMan->f95_loadWallSet(k0_WallSetStone);

	_sound->f503_loadSounds(); // @ F0506_AMIGA_AllocateData

	if (!ConfMan.hasKey("save_slot")) // skip drawing title if loading from launcher
		f437_STARTEND_drawTittle();

	_textMan->f54_textInitialize();
	_objectMan->loadObjectNames();
	_eventMan->initMouse();

	int16 saveSlot = -1;
	do {
		// if loading from the launcher
		if (ConfMan.hasKey("save_slot")) {
			saveSlot = ConfMan.getInt("save_slot");
		} else { // else show the entrance
			f441_processEntrance();
			if (_engineShouldQuit)
				return;

			if (_g298_newGame == k0_modeLoadSavedGame) { // if resume was clicked, bring up ScummVM load screen
				GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
				saveSlot = dialog->runModalWithCurrentTarget();
				delete dialog;
			}
		}
	} while (f435_loadgame(saveSlot) != k1_LoadgameSuccess);

	_displayMan->f466_loadIntoBitmap(k11_MenuSpellAreLinesIndice, _menuMan->_gK73_bitmapSpellAreaLines); // @ F0396_MENUS_LoadSpellAreaLinesBitmap

	// There was some memory wizardy for the Amiga platform, I skipped that part
	_displayMan->f461_allocateFlippedWallBitmaps();

	f462_startGame();
	if (_g298_newGame)
		_moveSens->f267_getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
	_eventMan->f78_showMouse();
	_eventMan->f357_discardAllInput();
}

void DMEngine::f448_initMemoryManager() {
	static uint16 palSwoosh[16] = {0x000, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0x000, 0xFFF, 0xAAA, 0xFFF, 0xAAA, 0x444, 0xFF0, 0xFF0}; // @ K0057_aui_Palette_Swoosh

	_displayMan->f508_buildPaletteChangeCopperList(palSwoosh, palSwoosh);
	for (uint16 i = 0; i < 16; ++i) {
		_displayMan->_g347_paletteTopAndBottomScreen[i] = _displayMan->_palDungeonView[0][i];
		_displayMan->_g346_paletteMiddleScreen[i] = _displayMan->_palDungeonView[0][i];
	}
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

	_eventMan->_g441_primaryMouseInput = _eventMan->_primaryMouseInputInterface;
	_eventMan->_g442_secondaryMouseInput = _eventMan->_secondaryMouseInputMovement;
	_eventMan->_g443_primaryKeyboardInput = _eventMan->_primaryKeyboardInputInterface;
	_eventMan->_g444_secondaryKeyboardInput = _eventMan->_secondaryKeyboardInputMovement;

	f3_processNewPartyMap(_dungeonMan->_g309_partyMapIndex);

	if (!_g298_newGame) {
		_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_g347_paletteTopAndBottomScreen);
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

	_displayMan->f508_buildPaletteChangeCopperList(_displayMan->_palDungeonView[0], _displayMan->_g347_paletteTopAndBottomScreen);
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
	_sound = SoundMan::getSoundMan(this, _gameVersion);
	_displayMan->setUpScreens(320, 200);

	f463_initializeGame();
	while (true) {
		f2_gameloop();

		if (_engineShouldQuit)
			return Common::kNoError;

		if (_loadSaveSlotAtRuntime == -1)
			f444_endGame(_championMan->_g303_partyDead);
		else {
			loadGameState(_loadSaveSlotAtRuntime);
			_menuMan->f457_drawEnabledMenus();
			_displayMan->updateScreen();
			_loadSaveSlotAtRuntime = -1;
		}
	}

	return Common::kNoError;
}

void DMEngine::f2_gameloop() {
	_canLoadFromGMM = true;
	_g318_waitForInputMaxVerticalBlankCount = 10;
	while (true) {
		if (_engineShouldQuit) {
			_canLoadFromGMM = false;
			return;
		}

		// DEBUG CODE
		for (int16 i = 0; i < _championMan->_g305_partyChampionCount; ++i) {
			Champion &champ = _championMan->_gK71_champions[i];
			if (_console->_debugGodmodeHP)
				champ._currHealth = champ._maxHealth;
			if (_console->_debugGodmodeMana)
				champ._currMana = champ._maxMana;
			if (_console->_debugGodmodeStamina)
				champ._currStamina = champ._maxStamina;
		}

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
			_displayMan->f135_fillBoxBitmap(_displayMan->_g296_bitmapViewport, box, k0_ColorBlack, k112_byteWidthViewport, k136_heightViewport); // (possibly dummy code)
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
		_sound->f65_playPendingSound();
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
			if (_engineShouldQuit || _loadSaveSlotAtRuntime != -1) {
				_canLoadFromGMM = false;
				return;
			}
			_displayMan->updateScreen();
			if (!_g321_stopWaitingForPlayerInput) {
				_eventMan->f363_highlightBoxDisable();
			}

			_system->delayMillis(2);
			if (++vblankCounter >= _g318_waitForInputMaxVerticalBlankCount * 5)
				_g321_stopWaitingForPlayerInput = true;

		} while (!_g321_stopWaitingForPlayerInput || !_g301_gameTimeTicking);
	}
	_canLoadFromGMM = false;
}

int16 DMEngine::M1_ordinalToIndex(int16 val) {
	return val - 1;
}

int16 DMEngine::M0_indexToOrdinal(int16 val) {
	return val + 1;
}

void DMEngine::f441_processEntrance() {
	_eventMan->_g441_primaryMouseInput = _eventMan->_primaryMouseInputEntrance;
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
	_sound->f060_SOUND_Play(k01_soundSWITCH, 112, 0x40, 0x40);
	f22_delay(20);
	_eventMan->f78_showMouse();
	if (_g298_newGame)
		f438_STARTEND_OpenEntranceDoors();

	delete[] _g562_entranceDoorAnimSteps[0];
	for (uint16 i = 0; i < 10; ++i)
		_g562_entranceDoorAnimSteps[i] = nullptr;
}

void DMEngine::f444_endGame(bool doNotDrawCreditsOnly) {
	static Box boxEndgameRestartOuterEN(103, 217, 145, 159);
	static Box boxEndgameRestartInnerEN(105, 215, 147, 157);

	static Box boxEndgameRestartOuterDE(82, 238, 145, 159);
	static Box boxEndgameRestartInnerDE(84, 236, 147, 157);

	static Box boxEndgameRestartOuterFR(100, 220, 145, 159);
	static Box boxEndgameRestartInnerFR(102, 218, 147, 157);

	Box restartOuterBox;
	Box restartInnerBox;

	switch (getGameLanguage()) { // localized
	default:
	case Common::EN_ANY:
		restartOuterBox = boxEndgameRestartOuterEN;
		restartInnerBox = boxEndgameRestartInnerEN;
		break;
	case Common::DE_DEU:
		restartOuterBox = boxEndgameRestartOuterDE;
		restartInnerBox = boxEndgameRestartInnerDE;
		break;
	case Common::FR_FRA:
		restartOuterBox = boxEndgameRestartOuterFR;
		restartInnerBox = boxEndgameRestartInnerFR;
		break;
	}

	static Box theEndBox(120, 199, 95, 108);
	static Box championMirrorBox(11, 74, 7, 49);
	static Box championPortraitBox(27, 58, 13, 41);

	bool waitBeforeDrawingRestart = true;

	_eventMan->f67_setMousePointerToNormal(k0_pointerArrow);
	_eventMan->f78_showMouse();
	_eventMan->_g441_primaryMouseInput = nullptr;
	_eventMan->_g442_secondaryMouseInput = nullptr;
	_eventMan->_g443_primaryKeyboardInput = nullptr;
	_eventMan->_g444_secondaryKeyboardInput = nullptr;
	if (doNotDrawCreditsOnly && !_g302_gameWon) {
		_sound->f064_SOUND_RequestPlay_CPSD(k06_soundSCREAM, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY, k0_soundModePlayImmediately);
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
		_displayMan->updateScreen();
		if (waitBeforeDrawingRestart)
			f22_delay(300);

		if (_g524_restartGameAllowed) {
			_displayMan->_g578_useByteBoxCoordinates = false;
			_displayMan->D24_fillScreenBox(restartOuterBox, k12_ColorDarkestGray);
			_displayMan->D24_fillScreenBox(restartInnerBox, k0_ColorBlack);

			switch (getGameLanguage()) { // localized
			default:
			case Common::EN_ANY: _textMan->f53_printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "RESTART THIS GAME"); break;
			case Common::DE_DEU: _textMan->f53_printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "DIESES SPIEL NEU STARTEN"); break;
			case Common::FR_FRA: _textMan->f53_printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "RECOMMENCER CE JEU"); break;
			}

			curPalette[1] = D03_RGB_PINK;
			curPalette[4] = D09_RGB_WHITE;
			_eventMan->_g441_primaryMouseInput = _eventMan->_primaryMouseInputRestartGame;
			_eventMan->f357_discardAllInput();
			_eventMan->f77_hideMouse();
			_displayMan->f436_STARTEND_FadeToPalette(curPalette);
			for (int16 verticalBlankCount = 900; --verticalBlankCount && !_g523_restartGameRequest; f22_delay(1))
				_eventMan->f380_processCommandQueue();

			_eventMan->f78_showMouse();
			if (_g523_restartGameRequest) {
				_displayMan->f436_STARTEND_FadeToPalette(darkBluePalette);
				_displayMan->fillScreen(k0_ColorBlack);
				_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_palDungeonView[0]);
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
	_displayMan->f21_blitToScreen(_displayMan->f489_getNativeBitmapOrGraphic(k5_creditsGraphicIndice), &box, k160_byteWidthScreen, kM1_ColorNoTransparency, k200_heightScreen);

	_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_palCredits);
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
	/* Atari ST: { 0x000, 0x333, 0x444, 0x420, 0x654, 0x210, 0x040, 0x050, 0x432, 0x700, 0x543, 0x321, 0x222, 0x555, 0x310, 0x777 }, RGB colors are different */
	static uint16 palEntrance[16] = {0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0, 0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF}; // @ G0020_aui_Graphic562_Palette_Entrance

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

	if (!_savedScreenForOpenEntranceDoors)
		_savedScreenForOpenEntranceDoors = new byte[k200_heightScreen * k160_byteWidthScreen * 2];
	memcpy(_savedScreenForOpenEntranceDoors, _displayMan->_g348_bitmapScreen, 320 * 200);

	_displayMan->_g578_useByteBoxCoordinates = false, _displayMan->f132_blitToBitmap(_displayMan->_g348_bitmapScreen, _g562_entranceDoorAnimSteps[8], doorsUpperHalfBox, 0, 30, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);
	_displayMan->_g578_useByteBoxCoordinates = false, _displayMan->f132_blitToBitmap(_displayMan->_g348_bitmapScreen, _g562_entranceDoorAnimSteps[8], doorsLowerHalfBox, 0, 111, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);

	_displayMan->f21_blitToScreen(_g562_entranceDoorAnimSteps[0], &closedDoorLeftBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->f21_blitToScreen(_g562_entranceDoorAnimSteps[4], &closedDoorRightBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->f436_STARTEND_FadeToPalette(palEntrance);
}

void DMEngine::f438_STARTEND_OpenEntranceDoors() {
	Box rightDoorBox(109, 231, 30, 193);
	byte *rightDoorBitmap = _displayMan->f489_getNativeBitmapOrGraphic(k3_entranceRightDoorGraphicIndice);
	Box leftDoorBox(0, 100, 30, 193);
	uint16 leftDoorBlitFrom = 0;
	byte *leftDoorBitmap = _displayMan->f489_getNativeBitmapOrGraphic(k2_entranceLeftDoorGraphicIndice);

	Box screenBox(0, 319, 0, 199);

	for (uint16 animStep = 1; animStep < 32; ++animStep) {
		if ((animStep % 3) == 1) {
			// Strangerke: CHECKME: Earlier versions of the game were using G0565_puc_Graphic535_Sound02DoorRattle instead of k02_soundDOOR_RATTLE 2
			_sound->f060_SOUND_Play(k02_soundDOOR_RATTLE, 145, 0x40, 0x40);
		}

		_displayMan->f21_blitToScreen(_savedScreenForOpenEntranceDoors, &screenBox, 160, kM1_ColorNoTransparency, 200);
		_displayMan->f132_blitToBitmap(leftDoorBitmap, _displayMan->_g348_bitmapScreen, leftDoorBox, leftDoorBlitFrom, 0, 64, k160_byteWidthScreen,
									   kM1_ColorNoTransparency, 161, k200_heightScreen);
		_displayMan->f132_blitToBitmap(rightDoorBitmap, _displayMan->_g348_bitmapScreen, rightDoorBox, 0, 0, 64, k160_byteWidthScreen,
									   kM1_ColorNoTransparency, 161, k200_heightScreen);
		_eventMan->f357_discardAllInput();
		_displayMan->updateScreen();

		leftDoorBox._x2 -= 4;
		leftDoorBlitFrom += 4;
		rightDoorBox._x1 += 4;

		f22_delay(3);
	}
	delete[] _savedScreenForOpenEntranceDoors;
	_savedScreenForOpenEntranceDoors = nullptr;
}

void DMEngine::f437_STARTEND_drawTittle() {
	static Box G0003_s_Graphic562_Box_Title_StrikesBack_Destination(0, 319, 118, 174);
	static Box G0004_s_Graphic562_Box_Title_StrikesBack_Source(0, 319, 0, 56);
	static Box G0005_s_Graphic562_Box_Title_Presents(0, 319, 90, 105);
	static Box K0078_s_Box_Title_Dungeon_Chaos(0, 319, 0, 79);

	int16 L1380_i_Counter;
	uint16 L1381_ui_DestinationHeight;
	int16 L1382_i_DestinationPixelWidth;
	byte *L1383_l_Bitmap_TitleSteps;
	byte* L1384_puc_Bitmap_Title;
	byte* L1387_apuc_Bitmap_ShrinkedTitle[20]; /* Only the first 18 entries are actually used */
	byte* L1389_puc_Bitmap_Master_StrikesBack;
	byte* L1761_puc_Bitmap_Dungeon_Chaos; /* Unreferenced on Atari ST */
	int16 L1391_aai_Coordinates[20][5]; /* Only the first 18 entries are actually used */
	uint16 L1392_aui_Palette[16];


	_displayMan->_g578_useByteBoxCoordinates = false;

	byte *alloactedMem = L1383_l_Bitmap_TitleSteps = new byte[145600 * 2];

	L1384_puc_Bitmap_Title = L1383_l_Bitmap_TitleSteps;
	_displayMan->f466_loadIntoBitmap(k1_titleGraphicsIndice, L1383_l_Bitmap_TitleSteps);

	L1383_l_Bitmap_TitleSteps += 320 * 200;
	for (uint16 i = 0; i < 16; ++i)
		L1392_aui_Palette[i] = D01_RGB_DARK_BLUE;

	_displayMan->f436_STARTEND_FadeToPalette(L1392_aui_Palette);
	_displayMan->fillScreen(k0_ColorBlack);
	_displayMan->f132_blitToBitmap(L1384_puc_Bitmap_Title, _displayMan->_g348_bitmapScreen, G0005_s_Graphic562_Box_Title_Presents, 0, 137, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, k200_heightScreen, k200_heightScreen);
	L1392_aui_Palette[15] = D09_RGB_WHITE;
	_displayMan->f436_STARTEND_FadeToPalette(L1392_aui_Palette);
	_displayMan->f132_blitToBitmap(L1384_puc_Bitmap_Title, L1389_puc_Bitmap_Master_StrikesBack = L1383_l_Bitmap_TitleSteps, G0004_s_Graphic562_Box_Title_StrikesBack_Source, 0, 80, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 57);
	L1383_l_Bitmap_TitleSteps += 320 * 57;
	_displayMan->f132_blitToBitmap(L1384_puc_Bitmap_Title, L1761_puc_Bitmap_Dungeon_Chaos = L1383_l_Bitmap_TitleSteps, K0078_s_Box_Title_Dungeon_Chaos, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 80);
	L1383_l_Bitmap_TitleSteps += 320 * 80;
	L1384_puc_Bitmap_Title = L1761_puc_Bitmap_Dungeon_Chaos;
	L1381_ui_DestinationHeight = 12;
	L1382_i_DestinationPixelWidth = 48;
	for (L1380_i_Counter = 0; L1380_i_Counter < 18; L1380_i_Counter++) {
		L1387_apuc_Bitmap_ShrinkedTitle[L1380_i_Counter] = L1383_l_Bitmap_TitleSteps;
		_displayMan->f129_blitToBitmapShrinkWithPalChange(L1384_puc_Bitmap_Title, L1383_l_Bitmap_TitleSteps, 320, 80, L1382_i_DestinationPixelWidth, L1381_ui_DestinationHeight, _displayMan->_palChangesNoChanges);
		L1391_aai_Coordinates[L1380_i_Counter][1] = (L1391_aai_Coordinates[L1380_i_Counter][0] = (320 - L1382_i_DestinationPixelWidth) / 2) + L1382_i_DestinationPixelWidth - 1;
		L1391_aai_Coordinates[L1380_i_Counter][3] = (L1391_aai_Coordinates[L1380_i_Counter][2] = ((int)(160 - L1381_ui_DestinationHeight)) / 2) + L1381_ui_DestinationHeight - 1;
		L1383_l_Bitmap_TitleSteps += (L1391_aai_Coordinates[L1380_i_Counter][4] = ((L1382_i_DestinationPixelWidth + 15) / 16) * 8) * L1381_ui_DestinationHeight;
		L1381_ui_DestinationHeight += 4;
		L1382_i_DestinationPixelWidth += 16;
	}
	L1392_aui_Palette[15] = D01_RGB_DARK_BLUE;
	_displayMan->f436_STARTEND_FadeToPalette(L1392_aui_Palette);
	_displayMan->fillScreen(k0_ColorBlack);
	L1392_aui_Palette[3] = D05_RGB_DARK_GOLD;
	L1392_aui_Palette[4] = D02_RGB_LIGHT_BROWN;
	L1392_aui_Palette[5] = D06_RGB_GOLD;
	L1392_aui_Palette[6] = D04_RGB_LIGHTER_BROWN;
	L1392_aui_Palette[8] = D08_RGB_YELLOW;
	L1392_aui_Palette[15] = D07_RGB_RED;
	L1392_aui_Palette[10] = D01_RGB_DARK_BLUE;
	L1392_aui_Palette[12] = D01_RGB_DARK_BLUE;
	_displayMan->f436_STARTEND_FadeToPalette(L1392_aui_Palette);
	f22_delay(1);
	for (L1380_i_Counter = 0; L1380_i_Counter < 18; L1380_i_Counter++) {
		f22_delay(2);
		Box box(L1391_aai_Coordinates[L1380_i_Counter]);
		_displayMan->f132_blitToBitmap(L1387_apuc_Bitmap_ShrinkedTitle[L1380_i_Counter], _displayMan->_g348_bitmapScreen, box, 0, 0, L1391_aai_Coordinates[L1380_i_Counter][4], k160_byteWidthScreen, kM1_ColorNoTransparency, L1391_aai_Coordinates[L1380_i_Counter][3] - L1391_aai_Coordinates[L1380_i_Counter][2] + 1, k200_heightScreen);
	}
	f22_delay(25L);
	_displayMan->f132_blitToBitmap(L1389_puc_Bitmap_Master_StrikesBack, _displayMan->_g348_bitmapScreen, G0003_s_Graphic562_Box_Title_StrikesBack_Destination, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, k0_ColorBlack, 57, k200_heightScreen);
	L1392_aui_Palette[10] = D00_RGB_BLACK;
	L1392_aui_Palette[12] = D07_RGB_RED;
	_displayMan->f436_STARTEND_FadeToPalette(L1392_aui_Palette);
	delete[] alloactedMem;
	f22_delay(75L);
}

void DMEngine::f442_SARTEND_processCommand202_entranceDrawCredits() {
	_eventMan->f78_showMouse();
	_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_g345_aui_BlankBuffer);
	_displayMan->f466_loadIntoBitmap(k5_creditsGraphicIndice, _displayMan->_g348_bitmapScreen);
	_displayMan->f436_STARTEND_FadeToPalette(_displayMan->_palCredits);
	f22_delay(50);
	_eventMan->f541_waitForMouseOrKeyActivity();
	_g298_newGame = k202_modeEntranceDrawCredits;
}

void DMEngine::f446_STARTEND_fuseSequnce() {
	int16 L1424_i_Multiple;
#define AL1424_B_RemoveFluxcagesFromLoadChaosSquare L1424_i_Multiple
#define AL1424_i_Attack                             L1424_i_Multiple
#define AL1424_i_CreatureTypeSwitchCount            L1424_i_Multiple
#define AL1424_i_MapX                               L1424_i_Multiple
#define AL1424_i_TextStringThingCount               L1424_i_Multiple
	int16 L1425_i_Multiple;
#define AL1425_i_FluxcageMapX         L1425_i_Multiple
#define AL1425_i_MapY                 L1425_i_Multiple
#define AL1425_i_CycleCount           L1425_i_Multiple
#define AL1425_i_TextStringThingIndex L1425_i_Multiple
	int16 L1426_i_Multiple;
#define AL1426_i_FluxcageMapY            L1426_i_Multiple
#define AL1426_i_FuseSequenceUpdateCount L1426_i_Multiple
#define AL1426_i_TextStringThingCount    L1426_i_Multiple
	Thing L1427_T_Thing;
	Group* L1428_ps_Group;
	Explosion* L1429_ps_Explosion;
	Thing L1430_T_NextThing;
	int16 L1431_i_LordChaosMapX;
	int16 L1432_i_LordChaosMapY;
	Thing L1433_T_LordChaosThing;
	char L1434_c_TextFirstCharacter;
	Thing L1435_aT_TextStringThings[8];
	char L1436_ac_String[200];


	_g302_gameWon = true;
	if (_inventoryMan->_g432_inventoryChampionOrdinal) {
		_inventoryMan->f355_toggleInventory(k4_ChampionCloseInventory);
	}
	_eventMan->f363_highlightBoxDisable();
	_championMan->_g407_party._magicalLightAmount = 200;
	_inventoryMan->f337_setDungeonViewPalette();
	_championMan->_g407_party._fireShieldDefense = _championMan->_g407_party._spellShieldDefense = _championMan->_g407_party._shieldDefense = 100;
	_timeline->f260_timelineRefreshAllChampionStatusBoxes();
	f445_STARTEND_fuseSequenceUpdate();
	L1431_i_LordChaosMapX = _dungeonMan->_g306_partyMapX;
	L1432_i_LordChaosMapY = _dungeonMan->_g307_partyMapY;
	L1431_i_LordChaosMapX += _dirIntoStepCountEast[_dungeonMan->_g308_partyDir], L1432_i_LordChaosMapY += _dirIntoStepCountNorth[_dungeonMan->_g308_partyDir];
	L1428_ps_Group = (Group*)_dungeonMan->f156_getThingData(L1433_T_LordChaosThing = _groupMan->f175_groupGetThing(L1431_i_LordChaosMapX, L1432_i_LordChaosMapY));
	L1428_ps_Group->_health[0] = 10000;
	_dungeonMan->f146_setGroupCells(L1428_ps_Group, k255_CreatureTypeSingleCenteredCreature, _dungeonMan->_g309_partyMapIndex);
	_dungeonMan->f148_setGroupDirections(L1428_ps_Group, returnOppositeDir(_dungeonMan->_g308_partyDir), _dungeonMan->_g309_partyMapIndex);

	AL1424_B_RemoveFluxcagesFromLoadChaosSquare = true;
	AL1425_i_FluxcageMapX = _dungeonMan->_g306_partyMapX;
	AL1426_i_FluxcageMapY = _dungeonMan->_g307_partyMapY;
T0446002:
	L1427_T_Thing = _dungeonMan->f162_getSquareFirstObject(AL1425_i_FluxcageMapX, AL1426_i_FluxcageMapY);
	while (L1427_T_Thing != Thing::_endOfList) {
		if (L1427_T_Thing.getType() == k15_ExplosionThingType) {
			L1429_ps_Explosion = (Explosion*)_dungeonMan->f156_getThingData(L1427_T_Thing);
			if (L1429_ps_Explosion->getType() == k50_ExplosionType_Fluxcage) {
				_dungeonMan->f164_unlinkThingFromList(L1427_T_Thing, Thing(0), AL1425_i_FluxcageMapX, AL1426_i_FluxcageMapY);
				L1429_ps_Explosion->setNextThing(Thing::_none);
				goto T0446002;
			}
		}
		L1427_T_Thing = _dungeonMan->f159_getNextThing(L1427_T_Thing);
	}
	if (AL1424_B_RemoveFluxcagesFromLoadChaosSquare) {
		AL1424_B_RemoveFluxcagesFromLoadChaosSquare = false;
		AL1425_i_FluxcageMapX = L1431_i_LordChaosMapX;
		AL1426_i_FluxcageMapY = L1432_i_LordChaosMapY;
		goto T0446002;
	}

	f445_STARTEND_fuseSequenceUpdate();
	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->f213_explosionCreate(Thing::_explFireBall, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		f445_STARTEND_fuseSequenceUpdate();
	}
	_sound->f064_SOUND_RequestPlay_CPSD(k17_soundBUZZ, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k1_soundModePlayIfPrioritized);
	L1428_ps_Group->_type = k25_CreatureTypeLordOrder;
	f445_STARTEND_fuseSequenceUpdate();
	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->f213_explosionCreate(Thing::_explHarmNonMaterial, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		f445_STARTEND_fuseSequenceUpdate();
	}
	for (AL1425_i_CycleCount = 4; --AL1425_i_CycleCount; ) {
		for (AL1424_i_CreatureTypeSwitchCount = 5; --AL1424_i_CreatureTypeSwitchCount; ) {
			_sound->f064_SOUND_RequestPlay_CPSD(k17_soundBUZZ, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k1_soundModePlayIfPrioritized);
			L1428_ps_Group->_type = (AL1424_i_CreatureTypeSwitchCount & 0x0001) ? k25_CreatureTypeLordOrder : k23_CreatureTypeLordChaos;
			for (AL1426_i_FuseSequenceUpdateCount = AL1425_i_CycleCount; AL1426_i_FuseSequenceUpdateCount--; f445_STARTEND_fuseSequenceUpdate());
		}
	}
	_projexpl->f213_explosionCreate(Thing::_explFireBall, 255, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
	_projexpl->f213_explosionCreate(Thing::_explHarmNonMaterial, 255, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
	f445_STARTEND_fuseSequenceUpdate();
	L1428_ps_Group->_type = k26_CreatureTypeGreyLord;
	f445_STARTEND_fuseSequenceUpdate();
	_displayMan->_g77_doNotDrawFluxcagesDuringEndgame = true;
	f445_STARTEND_fuseSequenceUpdate();
	for (AL1424_i_MapX = 0; AL1424_i_MapX < _dungeonMan->_g273_currMapWidth; AL1424_i_MapX++) {
		for (AL1425_i_MapY = 0; AL1425_i_MapY < _dungeonMan->_g274_currMapHeight; AL1425_i_MapY++) {
			if (((L1427_T_Thing = _groupMan->f175_groupGetThing(AL1424_i_MapX, AL1425_i_MapY)) != Thing::_endOfList) && ((AL1424_i_MapX != L1431_i_LordChaosMapX) || (AL1425_i_MapY != L1432_i_LordChaosMapY))) {
				_groupMan->f189_delete(AL1424_i_MapX, AL1425_i_MapY);
			}
		}
	}
	f445_STARTEND_fuseSequenceUpdate();
	/* Count and get list of text things located at 0, 0 in the current map. Their text is then printed as messages in the order specified by their first letter (which is not printed) */
	L1427_T_Thing = _dungeonMan->f161_getSquareFirstThing(0, 0);
	AL1424_i_TextStringThingCount = 0;
	while (L1427_T_Thing != Thing::_endOfList) {
		if (L1427_T_Thing.getType() == k2_TextstringType) {
			L1435_aT_TextStringThings[AL1424_i_TextStringThingCount++] = L1427_T_Thing;
		}
		L1427_T_Thing = _dungeonMan->f159_getNextThing(L1427_T_Thing);
	}
	L1434_c_TextFirstCharacter = 'A';
	AL1426_i_TextStringThingCount = AL1424_i_TextStringThingCount;
	while (AL1424_i_TextStringThingCount--) {
		for (AL1425_i_TextStringThingIndex = 0; AL1425_i_TextStringThingIndex < AL1426_i_TextStringThingCount; AL1425_i_TextStringThingIndex++) {
			_dungeonMan->f168_decodeText(L1436_ac_String, L1435_aT_TextStringThings[AL1425_i_TextStringThingIndex], (TextType)(k1_TextTypeMessage | k0x8000_DecodeEvenIfInvisible));
			if (L1436_ac_String[1] == L1434_c_TextFirstCharacter) {
				_textMan->f43_messageAreaClearAllRows();
				L1436_ac_String[1] = '\n'; /* New line */
				_textMan->f47_messageAreaPrintMessage(k15_ColorWhite, &L1436_ac_String[1]);
				f445_STARTEND_fuseSequenceUpdate();
				f22_delay(780);
				L1434_c_TextFirstCharacter++;
				break;
			}
		}
	}

	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->f213_explosionCreate(Thing::_explHarmNonMaterial, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		f445_STARTEND_fuseSequenceUpdate();
	}

	f22_delay(600);
	_g524_restartGameAllowed = false;
	f444_endGame(true);
}

void DMEngine::f445_STARTEND_fuseSequenceUpdate() {
	_timeline->f261_processTimeline();
	_displayMan->f128_drawDungeon(_dungeonMan->_g308_partyDir, _dungeonMan->_g306_partyMapX, _dungeonMan->_g307_partyMapY);
	_sound->f65_playPendingSound();
	_eventMan->f357_discardAllInput();
	_displayMan->updateScreen();
	f22_delay(2);
	_g313_gameTime++; /* BUG0_71 Some timings are too short on fast computers.
					  The ending animation when Lord Chaos is fused plays too quickly because the execution speed is not limited */
}

Common::Language DMEngine::getGameLanguage() { return _gameVersion->language; }
} // End of namespace DM
