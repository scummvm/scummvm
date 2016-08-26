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
		::warning("%s", output.c_str());
	} else {
		static Common::Array<Common::String> stringsPrinted;

		if (Common::find(stringsPrinted.begin(), stringsPrinted.end(), s) == stringsPrinted.end()) {
			stringsPrinted.push_back(output);
			::warning("%s", output.c_str());
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

uint16 bitmapByteCount(uint16 pixelWidth, uint16 height) {
	return pixelWidth / 2 * height;
}

uint16 normalizeModulo4(uint16 val) {
	return val & 3;
}

int32 filterTime(int32 mapTime) {
	return mapTime & 0x00FFFFFF;
}

int32 setMapAndTime(int32 &mapTime, uint32 map, uint32 time) {
	return (mapTime) = ((time) | (((long)(map)) << 24));
}

uint16 getMap(int32 mapTime) {
	return ((uint16)((mapTime) >> 24));
}

Thing thingWithNewCell(Thing thing, int16 cell) {
	return Thing(((thing.toUint16()) & 0x3FFF) | ((cell) << 14));
}

int16 getDistance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2) {
	return ABS(mapx1 - mapx2) + ABS(mapy1 - mapy2);
}

DMEngine::DMEngine(OSystem *syst, const DMADGameDescription *desc) : Engine(syst), _console(nullptr), _gameVersion(desc) {
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
	_dungeonId = 0;

	_newGameFl = 0;
	_restartGameRequest = false;
	_stopWaitingForPlayerInput = true;
	_gameTimeTicking = false;
	_restartGameAllowed = false;
	_gameId = 0;
	_pressingEye = false;
	_stopPressingEye = false;
	_pressingMouth = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;
	_projectileDisableMovementTicks = 0;
	_lastProjectileDisabledMovementDirection = 0;
	_gameWon = false;
	_newPartyMapIndex = kM1_mapIndexNone;
	_setMousePointerToObjectInMainLoop = false;
	_disabledMovementTicks = 0;
	_gameTime = 0;
	_stringBuildBuffer[0] = '\0';
	_waitForInputMaxVerticalBlankCount = 0;
	_savedScreenForOpenEntranceDoors = nullptr;
	for (uint16 i = 0; i < 10; ++i)
		_entranceDoorAnimSteps[i] = nullptr;
	_interfaceCredits = nullptr;
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
	if (loadgame(slot) != kM1_LoadgameFailure) {
		_displayMan->fillScreen(k0_ColorBlack);
		_displayMan->startEndFadeToPalette(_displayMan->_palDungeonView[0]);
		_newGameFl = k0_modeLoadSavedGame;

		startGame();
		_restartGameRequest = false;
		_eventMan->hideMouse();
		_eventMan->discardAllInput();
		return Common::kNoError;
	}

	return Common::kNoGameDataFoundError;
}

bool DMEngine::canLoadGameStateCurrently() {
	return _canLoadFromGMM;
}

void DMEngine::delay(uint16 verticalBlank) {
	for (uint16 i = 0; i < verticalBlank * 2; ++i) {
		_eventMan->processInput();
		_displayMan->updateScreen();
		_system->delayMillis(10); // Google says most Amiga games had a refreshrate of 50 hz
	}
}

uint16 DMEngine::getScaledProduct(uint16 val, uint16 scale, uint16 vale2) {
	return ((uint32)val * vale2) >> scale;
}

void DMEngine::initializeGame() {
	initMemoryManager();
	_displayMan->loadGraphics();
	_displayMan->initializeGraphicData();
	_displayMan->loadFloorSet(k0_FloorSetStone);
	_displayMan->loadWallSet(k0_WallSetStone);

	_sound->loadSounds(); // @ F0506_AMIGA_AllocateData

	if (!ConfMan.hasKey("save_slot")) // skip drawing title if loading from launcher
		drawTittle();

	_textMan->initialize();
	_objectMan->loadObjectNames();
	_eventMan->initMouse();

	int16 saveSlot = -1;
	do {
		// if loading from the launcher
		if (ConfMan.hasKey("save_slot")) {
			saveSlot = ConfMan.getInt("save_slot");
		} else { // else show the entrance
			processEntrance();
			if (_engineShouldQuit)
				return;

			if (_newGameFl == k0_modeLoadSavedGame) { // if resume was clicked, bring up ScummVM load screen
				GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
				saveSlot = dialog->runModalWithCurrentTarget();
				delete dialog;
			}
		}
	} while (loadgame(saveSlot) != k1_LoadgameSuccess);

	_displayMan->loadIntoBitmap(k11_MenuSpellAreLinesIndice, _menuMan->_bitmapSpellAreaLines); // @ F0396_MENUS_LoadSpellAreaLinesBitmap

	// There was some memory wizardy for the Amiga platform, I skipped that part
	_displayMan->allocateFlippedWallBitmaps();

	startGame();
	if (_newGameFl)
		_moveSens->getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
	_eventMan->showMouse();
	_eventMan->discardAllInput();
}

void DMEngine::initMemoryManager() {
	static uint16 palSwoosh[16] = {0x000, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0xFFF, 0x000, 0xFFF, 0xAAA, 0xFFF, 0xAAA, 0x444, 0xFF0, 0xFF0}; // @ K0057_aui_Palette_Swoosh

	_displayMan->buildPaletteChangeCopperList(palSwoosh, palSwoosh);
	for (uint16 i = 0; i < 16; ++i) {
		_displayMan->_paletteTopAndBottomScreen[i] = _displayMan->_palDungeonView[0][i];
		_displayMan->_paletteMiddleScreen[i] = _displayMan->_palDungeonView[0][i];
	}
}

void DMEngine::startGame() {
	static Box boxScreenTop(0, 319, 0, 32); // @ G0061_s_Graphic562_Box_ScreenTop
	static Box boxScreenRight(224, 319, 33, 169); // @ G0062_s_Graphic562_Box_ScreenRight
	static Box boxScreenBottom(0, 319, 169, 199); // @ G0063_s_Graphic562_Box_ScreenBottom

	_pressingEye = false;
	_stopPressingEye = false;
	_pressingMouth = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;
	_eventMan->_highlightBoxEnabled = false;
	_championMan->_partyIsSleeping = false;
	_championMan->_actingChampionOrdinal = indexToOrdinal(kM1_ChampionNone);
	_menuMan->_actionAreaContainsIcons = true;
	_eventMan->_useChampionIconOrdinalAsMousePointerBitmap = indexToOrdinal(kM1_ChampionNone);

	_eventMan->_primaryMouseInput = _eventMan->_primaryMouseInputInterface;
	_eventMan->_secondaryMouseInput = _eventMan->_secondaryMouseInputMovement;
	_eventMan->_primaryKeyboardInput = _eventMan->_primaryKeyboardInputInterface;
	_eventMan->_secondaryKeyboardInput = _eventMan->_secondaryKeyboardInputMovement;

	processNewPartyMap(_dungeonMan->_partyMapIndex);

	if (!_newGameFl) {
		_displayMan->startEndFadeToPalette(_displayMan->_paletteTopAndBottomScreen);
		_displayMan->_useByteBoxCoordinates = false;
		delay(1);
		_displayMan->fillScreenBox(boxScreenTop, k0_ColorBlack);
		_displayMan->fillScreenBox(boxScreenRight, k0_ColorBlack);
		_displayMan->fillScreenBox(boxScreenBottom, k0_ColorBlack);
	} else {
		_displayMan->_useByteBoxCoordinates = false;
		_displayMan->fillScreenBox(boxScreenTop, k0_ColorBlack);
		_displayMan->fillScreenBox(boxScreenRight, k0_ColorBlack);
		_displayMan->fillScreenBox(boxScreenBottom, k0_ColorBlack);
	}

	_displayMan->buildPaletteChangeCopperList(_displayMan->_palDungeonView[0], _displayMan->_paletteTopAndBottomScreen);
	_menuMan->drawMovementArrows();
	_championMan->resetDataToStartGame();
	_gameTimeTicking = true;
}

void DMEngine::processNewPartyMap(uint16 mapIndex) {
	_groupMan->removeAllActiveGroups();
	_dungeonMan->setCurrentMapAndPartyMap(mapIndex);
	_displayMan->loadCurrentMapGraphics();
	_groupMan->addAllActiveGroups();
	_inventoryMan->setDungeonViewPalette();
}

Common::Error DMEngine::run() {
	initConstants();

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

	initializeGame();
	while (true) {
		gameloop();

		if (_engineShouldQuit)
			return Common::kNoError;

		if (_loadSaveSlotAtRuntime == -1)
			endGame(_championMan->_partyDead);
		else {
			loadGameState(_loadSaveSlotAtRuntime);
			_menuMan->drawEnabledMenus();
			_displayMan->updateScreen();
			_loadSaveSlotAtRuntime = -1;
		}
	}

	return Common::kNoError;
}

void DMEngine::gameloop() {
	_canLoadFromGMM = true;
	_waitForInputMaxVerticalBlankCount = 10;
	while (true) {
		if (_engineShouldQuit) {
			_canLoadFromGMM = false;
			return;
		}

		// DEBUG CODE
		for (int16 i = 0; i < _championMan->_partyChampionCount; ++i) {
			Champion &champ = _championMan->_champions[i];
			if (_console->_debugGodmodeHP)
				champ._currHealth = champ._maxHealth;
			if (_console->_debugGodmodeMana)
				champ._currMana = champ._maxMana;
			if (_console->_debugGodmodeStamina)
				champ._currStamina = champ._maxStamina;
		}

		for (;;) {


			if (_newPartyMapIndex != kM1_mapIndexNone) {
				processNewPartyMap(_newPartyMapIndex);
				_moveSens->getMoveResult(Thing::_party, kM1_MapXNotOnASquare, 0, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
				_newPartyMapIndex = kM1_mapIndexNone;
				_eventMan->discardAllInput();
			}
			_timeline->processTimeline();

			if (_newPartyMapIndex == kM1_mapIndexNone)
				break;
		}

		if (!_inventoryMan->_inventoryChampionOrdinal && !_championMan->_partyIsSleeping) {
			Box box(0, 223, 0, 135);
			_displayMan->fillBoxBitmap(_displayMan->_bitmapViewport, box, k0_ColorBlack, k112_byteWidthViewport, k136_heightViewport); // (possibly dummy code)
			_displayMan->drawDungeon(_dungeonMan->_partyDir, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
			if (_setMousePointerToObjectInMainLoop) {
				_setMousePointerToObjectInMainLoop = false;
				_eventMan->showMouse();
				_eventMan->setPointerToObject(_objectMan->_objectIconForMousePointer);
				_eventMan->hideMouse();

			}
			if (_eventMan->_refreshMousePointerInMainLoop) {
				_eventMan->_refreshMousePointerInMainLoop = false;
				_eventMan->_mousePointerBitmapUpdated = true;
				_eventMan->showMouse();
				_eventMan->hideMouse();
			}
		}
		_eventMan->highlightBoxDisable();
		_sound->playPendingSound();
		_championMan->applyAndDrawPendingDamageAndWounds();
		if (_championMan->_partyDead)
			break;

		_gameTime++;

		if (!(_gameTime & 511))
			_inventoryMan->decreaseTorchesLightPower();

		if (_championMan->_party._freezeLifeTicks)
			_championMan->_party._freezeLifeTicks -= 1;

		_menuMan->refreshActionAreaAndSetChampDirMaxDamageReceived();

		if (!(_gameTime & (_championMan->_partyIsSleeping ? 15 : 63)))
			_championMan->applyTimeEffects();

		if (_disabledMovementTicks)
			_disabledMovementTicks--;

		if (_projectileDisableMovementTicks)
			_projectileDisableMovementTicks--;

		_textMan->clearExpiredRows();
		_stopWaitingForPlayerInput = false;
		uint16 vblankCounter = 0;
		do {
			_eventMan->processInput();

			if (_stopPressingEye) {
				_pressingEye = false;
				_stopPressingEye = false;
				_inventoryMan->drawStopPressingEye();
			} else if (_stopPressingMouth) {
				_pressingMouth = false;
				_stopPressingMouth = false;
				_inventoryMan->drawStopPressingMouth();
			}

			_eventMan->processCommandQueue();
			if (_engineShouldQuit || _loadSaveSlotAtRuntime != -1) {
				_canLoadFromGMM = false;
				return;
			}
			_displayMan->updateScreen();
			if (!_stopWaitingForPlayerInput) {
				_eventMan->highlightBoxDisable();
			}

			_system->delayMillis(2);
			if (++vblankCounter >= _waitForInputMaxVerticalBlankCount * 4)
				_stopWaitingForPlayerInput = true;

		} while (!_stopWaitingForPlayerInput || !_gameTimeTicking);
	}
	_canLoadFromGMM = false;
}

int16 DMEngine::ordinalToIndex(int16 val) {
	return val - 1;
}

int16 DMEngine::indexToOrdinal(int16 val) {
	return val + 1;
}

void DMEngine::processEntrance() {
	_eventMan->_primaryMouseInput = _eventMan->_primaryMouseInputEntrance;
	_eventMan->_secondaryMouseInput = nullptr;
	_eventMan->_primaryKeyboardInput = nullptr;
	_eventMan->_secondaryKeyboardInput = nullptr;
	_entranceDoorAnimSteps[0] = new byte[128 * 161 * 12];
	for (uint16 idx = 1; idx < 8; idx++)
		_entranceDoorAnimSteps[idx] = _entranceDoorAnimSteps[idx - 1] + 128 * 161;

	_entranceDoorAnimSteps[8] = _entranceDoorAnimSteps[7] + 128 * 161;
	_entranceDoorAnimSteps[9] = _entranceDoorAnimSteps[8] + 128 * 161 * 2;

	_displayMan->loadIntoBitmap(k3_entranceRightDoorGraphicIndice, _entranceDoorAnimSteps[4]);
	_displayMan->loadIntoBitmap(k2_entranceLeftDoorGraphicIndice, _entranceDoorAnimSteps[0]);
	_interfaceCredits = _displayMan->getNativeBitmapOrGraphic(k5_creditsGraphicIndice);
	_displayMan->_useByteBoxCoordinates = false;
	Box displayBox(0, 100, 0, 160);
	for (uint16 idx = 1; idx < 4; idx++) {
		_displayMan->blitToBitmap(_entranceDoorAnimSteps[0], _entranceDoorAnimSteps[idx], displayBox, idx << 2, 0, k64_byteWidth, k64_byteWidth, kM1_ColorNoTransparency, 161, 161);
		displayBox._x2 -= 4;
	}
	displayBox._x2 = 127;
	for (uint16 idx = 5; idx < 8; idx++) {
		displayBox._x1 += 4;
		_displayMan->blitToBitmap(_entranceDoorAnimSteps[4], _entranceDoorAnimSteps[idx], displayBox, 0, 0, k64_byteWidth, k64_byteWidth, kM1_ColorNoTransparency, 161, 161);
	}

	do {
		drawEntrance();
		_eventMan->showMouse();
		_eventMan->discardAllInput();
		_newGameFl = k99_modeWaitingOnEntrance;
		do {
			_eventMan->processInput();
			if (_engineShouldQuit)
				return;
			_eventMan->processCommandQueue();
			_displayMan->updateScreen();
		} while (_newGameFl == k99_modeWaitingOnEntrance);
	} while (_newGameFl == k202_CommandEntranceDrawCredits);

	//Strangerke: CHECKME: Earlier versions were using G0566_puc_Graphic534_Sound01Switch
	_sound->play(k01_soundSWITCH, 112, 0x40, 0x40);
	delay(20);
	_eventMan->showMouse();
	if (_newGameFl)
		openEntranceDoors();

	delete[] _entranceDoorAnimSteps[0];
	for (uint16 i = 0; i < 10; ++i)
		_entranceDoorAnimSteps[i] = nullptr;
}

void DMEngine::endGame(bool doNotDrawCreditsOnly) {
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

	_eventMan->setMousePointerToNormal(k0_pointerArrow);
	_eventMan->showMouse();
	_eventMan->_primaryMouseInput = nullptr;
	_eventMan->_secondaryMouseInput = nullptr;
	_eventMan->_primaryKeyboardInput = nullptr;
	_eventMan->_secondaryKeyboardInput = nullptr;
	if (doNotDrawCreditsOnly && !_gameWon) {
		_sound->requestPlay(k06_soundSCREAM, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY, k0_soundModePlayImmediately);
		delay(240);
	}

	if (_displayMan->_paletteSwitchingEnabled) {
		uint16 oldPalTopAndBottomScreen[16];
		for (uint16 i = 0; i < 16; ++i)
			oldPalTopAndBottomScreen[i] = _displayMan->_paletteTopAndBottomScreen[i];
		for (int i = 0; i <= 7; i++) {
			delay(1);
			for (int colIdx = 0; colIdx < 16; colIdx++) {
				_displayMan->_paletteMiddleScreen[colIdx] = _displayMan->getDarkenedColor(_displayMan->_paletteMiddleScreen[colIdx]);
				_displayMan->_paletteTopAndBottomScreen[colIdx] = _displayMan->getDarkenedColor(_displayMan->_paletteTopAndBottomScreen[colIdx]);
			}
		}
		_displayMan->_paletteSwitchingEnabled = false;
		delay(1);
		for (uint16 i = 0; i < 16; ++i)
			_displayMan->_paletteTopAndBottomScreen[i] = oldPalTopAndBottomScreen[i];
	} else
		_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);

	uint16 darkBluePalette[16];
	if (doNotDrawCreditsOnly) {
		if (_gameWon) {
			// Strangerke: Related to portraits. Game data could be missing for earlier versions of the game.
			_displayMan->fillScreen(k12_ColorDarkestGray);
			for (int16 championIndex = k0_ChampionFirst; championIndex < _championMan->_partyChampionCount; championIndex++) {
				int16 textPosY = championIndex * 48;
				Champion *curChampion = &_championMan->_champions[championIndex];
				_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(k208_wallOrn_43_champMirror), &championMirrorBox, k32_byteWidth, k10_ColorFlesh, 43);
				_displayMan->blitToScreen(curChampion->_portrait, &championPortraitBox, k16_byteWidth, k1_ColorDarkGary, 29);
				_textMan->printEndGameString(87, textPosY += 14, k9_ColorGold, curChampion->_name);
				int textPosX = (6 * strlen(curChampion->_name)) + 87;
				char championTitleFirstCharacter = curChampion->_title[0];
				if ((championTitleFirstCharacter != ',') && (championTitleFirstCharacter != ';') && (championTitleFirstCharacter != '-'))
					textPosX += 6;

				_textMan->printEndGameString(textPosX, textPosY++, k9_ColorGold, curChampion->_title);
				for (int16 idx = k0_ChampionSkillFighter; idx <= k3_ChampionSkillWizard; idx++) {
					uint16 skillLevel = MIN<uint16>(16, _championMan->getSkillLevel(championIndex, idx | (k0x4000_IgnoreObjectModifiers | k0x8000_IgnoreTemporaryExperience)));
					if (skillLevel == 1)
						continue;

					char displStr[20];
					strcpy(displStr, _inventoryMan->_skillLevelNames[skillLevel - 2]);
					strcat(displStr, " ");
					strcat(displStr, _championMan->_baseSkillName[idx]);
					_textMan->printEndGameString(105, textPosY = textPosY + 8, k13_ColorLightestGray, displStr);
				}
				championMirrorBox._y1 += 48;
				championMirrorBox._y2 += 48;
				championPortraitBox._y1 += 48;
				championPortraitBox._y1 += 48;
			}
			_displayMan->startEndFadeToPalette(_displayMan->_paletteTopAndBottomScreen);
			_engineShouldQuit = true;
			return;
		}
T0444017:
		_displayMan->fillScreen(k0_ColorBlack);
		_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(k6_theEndIndice), &theEndBox, k40_byteWidth, kM1_ColorNoTransparency, 14);
		for (uint16 i = 0; i < 16; ++i)
			darkBluePalette[i] = D01_RGB_DARK_BLUE;
		uint16 curPalette[16];
		for (uint16 i = 0; i < 15; ++i)
			curPalette[i] = darkBluePalette[i];
		curPalette[15] = D09_RGB_WHITE;
		_displayMan->startEndFadeToPalette(curPalette);
		_displayMan->updateScreen();
		if (waitBeforeDrawingRestart)
			delay(300);

		if (_restartGameAllowed) {
			_displayMan->_useByteBoxCoordinates = false;
			_displayMan->fillScreenBox(restartOuterBox, k12_ColorDarkestGray);
			_displayMan->fillScreenBox(restartInnerBox, k0_ColorBlack);

			switch (getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
				_textMan->printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "RESTART THIS GAME");
				break;
			case Common::DE_DEU:
				_textMan->printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "DIESES SPIEL NEU STARTEN");
				break;
			case Common::FR_FRA:
				_textMan->printToLogicalScreen(110, 154, k4_ColorCyan, k0_ColorBlack, "RECOMMENCER CE JEU");
				break;
			}

			curPalette[1] = D03_RGB_PINK;
			curPalette[4] = D09_RGB_WHITE;
			_eventMan->_primaryMouseInput = _eventMan->_primaryMouseInputRestartGame;
			_eventMan->discardAllInput();
			_eventMan->hideMouse();
			_displayMan->startEndFadeToPalette(curPalette);
			for (int16 verticalBlankCount = 900; --verticalBlankCount && !_restartGameRequest; delay(1))
				_eventMan->processCommandQueue();

			_eventMan->showMouse();
			if (_restartGameRequest) {
				_displayMan->startEndFadeToPalette(darkBluePalette);
				_displayMan->fillScreen(k0_ColorBlack);
				_displayMan->startEndFadeToPalette(_displayMan->_palDungeonView[0]);
				_newGameFl = k0_modeLoadSavedGame;
				if (loadgame(1) != kM1_LoadgameFailure) {
					startGame();
					_restartGameRequest = false;
					_eventMan->hideMouse();
					_eventMan->discardAllInput();
					return;
				}
			}
		}

		_displayMan->startEndFadeToPalette(darkBluePalette);
	}
	Box box(0, 319, 0, 199);
	_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(k5_creditsGraphicIndice), &box, k160_byteWidthScreen, kM1_ColorNoTransparency, k200_heightScreen);

	_displayMan->startEndFadeToPalette(_displayMan->_palCredits);
	_eventMan->waitForMouseOrKeyActivity();
	if (_engineShouldQuit)
		return;

	if (_restartGameAllowed && doNotDrawCreditsOnly) {
		waitBeforeDrawingRestart = false;
		_displayMan->startEndFadeToPalette(darkBluePalette);
		goto T0444017;
	}

	_engineShouldQuit = true;
	return;
}


void DMEngine::drawEntrance() {
	static Box doorsUpperHalfBox = Box(0, 231, 0, 80);
	static Box doorsLowerHalfBox = Box(0, 231, 81, 160);
	static Box closedDoorLeftBox = Box(0, 104, 30, 190);
	static Box closedDoorRightBox = Box(105, 231, 30, 190);
	/* Atari ST: { 0x000, 0x333, 0x444, 0x420, 0x654, 0x210, 0x040, 0x050, 0x432, 0x700, 0x543, 0x321, 0x222, 0x555, 0x310, 0x777 }, RGB colors are different */
	static uint16 palEntrance[16] = {0x000, 0x666, 0x888, 0x840, 0xCA8, 0x0C0, 0x080, 0x0A0, 0x864, 0xF00, 0xA86, 0x642, 0x444, 0xAAA, 0x620, 0xFFF}; // @ G0020_aui_Graphic562_Palette_Entrance

	byte *microDungeonCurrentMapData[32];

	_dungeonMan->_partyMapIndex = k255_mapIndexEntrance;
	_displayMan->_drawFloorAndCeilingRequested = true;
	_dungeonMan->_currMapWidth = 5;
	_dungeonMan->_currMapHeight = 5;
	_dungeonMan->_currMapData = microDungeonCurrentMapData;

	Map map; // uninitialized, won't be used
	_dungeonMan->_currMap = &map;
	Square microDungeonSquares[25];
	for (uint16 i = 0; i < 25; ++i)
		microDungeonSquares[i] = Square(k0_ElementTypeWall, 0);

	for (int16 idx = 0; idx < 5; idx++) {
		microDungeonCurrentMapData[idx] = (byte*)&microDungeonSquares[idx * 5];
		microDungeonSquares[idx + 10] = Square(k1_CorridorElemType, 0);
	}
	microDungeonSquares[7] = Square(k1_CorridorElemType, 0);
	_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);

	// note, a global variable is used here in the original
	_displayMan->loadIntoBitmap(k4_entranceGraphicIndice, _displayMan->_bitmapScreen);
	_displayMan->drawDungeon(kDirSouth, 2, 0);

	if (!_savedScreenForOpenEntranceDoors)
		_savedScreenForOpenEntranceDoors = new byte[k200_heightScreen * k160_byteWidthScreen * 2];
	memcpy(_savedScreenForOpenEntranceDoors, _displayMan->_bitmapScreen, 320 * 200);

	_displayMan->_useByteBoxCoordinates = false, _displayMan->blitToBitmap(_displayMan->_bitmapScreen, _entranceDoorAnimSteps[8], doorsUpperHalfBox, 0, 30, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);
	_displayMan->_useByteBoxCoordinates = false, _displayMan->blitToBitmap(_displayMan->_bitmapScreen, _entranceDoorAnimSteps[8], doorsLowerHalfBox, 0, 111, k160_byteWidthScreen, k128_byteWidth, kM1_ColorNoTransparency, 200, 161);

	_displayMan->blitToScreen(_entranceDoorAnimSteps[0], &closedDoorLeftBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->blitToScreen(_entranceDoorAnimSteps[4], &closedDoorRightBox, k64_byteWidth, kM1_ColorNoTransparency, 161);
	_displayMan->startEndFadeToPalette(palEntrance);
}

void DMEngine::openEntranceDoors() {
	Box rightDoorBox(109, 231, 30, 193);
	byte *rightDoorBitmap = _displayMan->getNativeBitmapOrGraphic(k3_entranceRightDoorGraphicIndice);
	Box leftDoorBox(0, 100, 30, 193);
	uint16 leftDoorBlitFrom = 0;
	byte *leftDoorBitmap = _displayMan->getNativeBitmapOrGraphic(k2_entranceLeftDoorGraphicIndice);

	Box screenBox(0, 319, 0, 199);

	for (uint16 animStep = 1; animStep < 32; ++animStep) {
		if ((animStep % 3) == 1) {
			// Strangerke: CHECKME: Earlier versions of the game were using G0565_puc_Graphic535_Sound02DoorRattle instead of k02_soundDOOR_RATTLE 2
			_sound->play(k02_soundDOOR_RATTLE, 145, 0x40, 0x40);
		}

		_displayMan->blitToScreen(_savedScreenForOpenEntranceDoors, &screenBox, 160, kM1_ColorNoTransparency, 200);
		_displayMan->blitToBitmap(leftDoorBitmap, _displayMan->_bitmapScreen, leftDoorBox, leftDoorBlitFrom, 0, 64, k160_byteWidthScreen,
									   kM1_ColorNoTransparency, 161, k200_heightScreen);
		_displayMan->blitToBitmap(rightDoorBitmap, _displayMan->_bitmapScreen, rightDoorBox, 0, 0, 64, k160_byteWidthScreen,
									   kM1_ColorNoTransparency, 161, k200_heightScreen);
		_eventMan->discardAllInput();
		_displayMan->updateScreen();

		leftDoorBox._x2 -= 4;
		leftDoorBlitFrom += 4;
		rightDoorBox._x1 += 4;

		delay(3);
	}
	delete[] _savedScreenForOpenEntranceDoors;
	_savedScreenForOpenEntranceDoors = nullptr;
}

void DMEngine::drawTittle() {
	static Box boxTitleStrikesBackDestination(0, 319, 118, 174);
	static Box boxTitleStrikesBackSource(0, 319, 0, 56);
	static Box boxTitlePresents(0, 319, 90, 105);
	static Box boxTitleDungeonChaos(0, 319, 0, 79);

	_displayMan->_useByteBoxCoordinates = false;

	byte *allocatedMem = new byte[145600 * 2];
	byte *titleSteps = allocatedMem;
	byte *bitmapTitle = titleSteps;
	_displayMan->loadIntoBitmap(k1_titleGraphicsIndice, titleSteps);

	titleSteps += 320 * 200;
	uint16 blitPalette[16];
	for (uint16 i = 0; i < 16; ++i)
		blitPalette[i] = D01_RGB_DARK_BLUE;

	_displayMan->startEndFadeToPalette(blitPalette);
	_displayMan->fillScreen(k0_ColorBlack);
	// uncomment this to draw 'Presents'
	//_displayMan->f132_blitToBitmap(L1384_puc_Bitmap_Title, _displayMan->_g348_bitmapScreen, G0005_s_Graphic562_Box_Title_Presents, 0, 137, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, k200_heightScreen, k200_heightScreen);
	blitPalette[15] = D09_RGB_WHITE;
	_displayMan->startEndFadeToPalette(blitPalette);
	byte *masterStrikesBack = titleSteps;
	_displayMan->blitToBitmap(bitmapTitle, masterStrikesBack, boxTitleStrikesBackSource, 0, 80, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 57);
	titleSteps += 320 * 57;
	byte *bitmapDungeonChaos = titleSteps; /* Unreferenced on Atari ST */
	_displayMan->blitToBitmap(bitmapTitle, bitmapDungeonChaos, boxTitleDungeonChaos, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kM1_ColorNoTransparency, 200, 80);
	titleSteps += 320 * 80;
	bitmapTitle = bitmapDungeonChaos;
	uint16 destinationHeight = 12;
	int16 destinationPixelWidth = 48;
	byte *shrinkedTitle[20]; /* Only the first 18 entries are actually used */
	int16 blitCoordinates[20][5]; /* Only the first 18 entries are actually used */
	for (int16 i = 0; i < 18; i++) {
		shrinkedTitle[i] = titleSteps;
		_displayMan->blitToBitmapShrinkWithPalChange(bitmapTitle, titleSteps, 320, 80, destinationPixelWidth, destinationHeight, _displayMan->_palChangesNoChanges);
		blitCoordinates[i][0] = (320 - destinationPixelWidth) / 2;
		blitCoordinates[i][1] = blitCoordinates[i][0] + destinationPixelWidth - 1;
		blitCoordinates[i][2] = (160 - destinationHeight) / 2;
		blitCoordinates[i][3] = blitCoordinates[i][2] + destinationHeight - 1;
		titleSteps += (blitCoordinates[i][4] = ((destinationPixelWidth + 15) / 16) * 8) * destinationHeight * 2;
		destinationHeight += 4;
		destinationPixelWidth += 16;
	}
	blitPalette[15] = D01_RGB_DARK_BLUE;
	_displayMan->startEndFadeToPalette(blitPalette);
	_displayMan->fillScreen(k0_ColorBlack);
	blitPalette[3] = D05_RGB_DARK_GOLD;
	blitPalette[4] = D02_RGB_LIGHT_BROWN;
	blitPalette[5] = D06_RGB_GOLD;
	blitPalette[6] = D04_RGB_LIGHTER_BROWN;
	blitPalette[8] = D08_RGB_YELLOW;
	blitPalette[15] = D07_RGB_RED;
	blitPalette[10] = D01_RGB_DARK_BLUE;
	blitPalette[12] = D01_RGB_DARK_BLUE;
	_displayMan->startEndFadeToPalette(blitPalette);
	delay(1);
	for (int16 i = 0; i < 18; i++) {
		delay(2);
		Box box(blitCoordinates[i]);
		_displayMan->blitToBitmap(shrinkedTitle[i], _displayMan->_bitmapScreen, box, 0, 0, blitCoordinates[i][4], k160_byteWidthScreen, kM1_ColorNoTransparency, blitCoordinates[i][3] - blitCoordinates[i][2] + 1, k200_heightScreen);
	}
	delay(25);
	_displayMan->blitToBitmap(masterStrikesBack, _displayMan->_bitmapScreen, boxTitleStrikesBackDestination, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, k0_ColorBlack, 57, k200_heightScreen);
	blitPalette[10] = D00_RGB_BLACK;
	blitPalette[12] = D07_RGB_RED;
	_displayMan->startEndFadeToPalette(blitPalette);
	delete[] allocatedMem;
	delay(75);
}

void DMEngine::entranceDrawCredits() {
	_eventMan->showMouse();
	_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);
	_displayMan->loadIntoBitmap(k5_creditsGraphicIndice, _displayMan->_bitmapScreen);
	_displayMan->startEndFadeToPalette(_displayMan->_palCredits);
	delay(50);
	_eventMan->waitForMouseOrKeyActivity();
	_newGameFl = k202_modeEntranceDrawCredits;
}

void DMEngine::fuseSequnce() {
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


	_gameWon = true;
	if (_inventoryMan->_inventoryChampionOrdinal) {
		_inventoryMan->toggleInventory(k4_ChampionCloseInventory);
	}
	_eventMan->highlightBoxDisable();
	_championMan->_party._magicalLightAmount = 200;
	_inventoryMan->setDungeonViewPalette();
	_championMan->_party._fireShieldDefense = _championMan->_party._spellShieldDefense = _championMan->_party._shieldDefense = 100;
	_timeline->refreshAllChampionStatusBoxes();
	fuseSequenceUpdate();
	L1431_i_LordChaosMapX = _dungeonMan->_partyMapX;
	L1432_i_LordChaosMapY = _dungeonMan->_partyMapY;
	L1431_i_LordChaosMapX += _dirIntoStepCountEast[_dungeonMan->_partyDir], L1432_i_LordChaosMapY += _dirIntoStepCountNorth[_dungeonMan->_partyDir];
	L1428_ps_Group = (Group*)_dungeonMan->getThingData(L1433_T_LordChaosThing = _groupMan->groupGetThing(L1431_i_LordChaosMapX, L1432_i_LordChaosMapY));
	L1428_ps_Group->_health[0] = 10000;
	_dungeonMan->setGroupCells(L1428_ps_Group, k255_CreatureTypeSingleCenteredCreature, _dungeonMan->_partyMapIndex);
	_dungeonMan->setGroupDirections(L1428_ps_Group, returnOppositeDir(_dungeonMan->_partyDir), _dungeonMan->_partyMapIndex);

	AL1424_B_RemoveFluxcagesFromLoadChaosSquare = true;
	AL1425_i_FluxcageMapX = _dungeonMan->_partyMapX;
	AL1426_i_FluxcageMapY = _dungeonMan->_partyMapY;
T0446002:
	L1427_T_Thing = _dungeonMan->getSquareFirstObject(AL1425_i_FluxcageMapX, AL1426_i_FluxcageMapY);
	while (L1427_T_Thing != Thing::_endOfList) {
		if (L1427_T_Thing.getType() == k15_ExplosionThingType) {
			L1429_ps_Explosion = (Explosion*)_dungeonMan->getThingData(L1427_T_Thing);
			if (L1429_ps_Explosion->getType() == k50_ExplosionType_Fluxcage) {
				_dungeonMan->unlinkThingFromList(L1427_T_Thing, Thing(0), AL1425_i_FluxcageMapX, AL1426_i_FluxcageMapY);
				L1429_ps_Explosion->setNextThing(Thing::_none);
				goto T0446002;
			}
		}
		L1427_T_Thing = _dungeonMan->getNextThing(L1427_T_Thing);
	}
	if (AL1424_B_RemoveFluxcagesFromLoadChaosSquare) {
		AL1424_B_RemoveFluxcagesFromLoadChaosSquare = false;
		AL1425_i_FluxcageMapX = L1431_i_LordChaosMapX;
		AL1426_i_FluxcageMapY = L1432_i_LordChaosMapY;
		goto T0446002;
	}

	fuseSequenceUpdate();
	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->createExplosion(Thing::_explFireBall, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		fuseSequenceUpdate();
	}
	_sound->requestPlay(k17_soundBUZZ, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k1_soundModePlayIfPrioritized);
	L1428_ps_Group->_type = k25_CreatureTypeLordOrder;
	fuseSequenceUpdate();
	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->createExplosion(Thing::_explHarmNonMaterial, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		fuseSequenceUpdate();
	}
	for (AL1425_i_CycleCount = 4; --AL1425_i_CycleCount; ) {
		for (AL1424_i_CreatureTypeSwitchCount = 5; --AL1424_i_CreatureTypeSwitchCount; ) {
			_sound->requestPlay(k17_soundBUZZ, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k1_soundModePlayIfPrioritized);
			L1428_ps_Group->_type = (AL1424_i_CreatureTypeSwitchCount & 0x0001) ? k25_CreatureTypeLordOrder : k23_CreatureTypeLordChaos;
			for (AL1426_i_FuseSequenceUpdateCount = AL1425_i_CycleCount; AL1426_i_FuseSequenceUpdateCount--; fuseSequenceUpdate());
		}
	}
	_projexpl->createExplosion(Thing::_explFireBall, 255, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
	_projexpl->createExplosion(Thing::_explHarmNonMaterial, 255, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
	fuseSequenceUpdate();
	L1428_ps_Group->_type = k26_CreatureTypeGreyLord;
	fuseSequenceUpdate();
	_displayMan->_doNotDrawFluxcagesDuringEndgame = true;
	fuseSequenceUpdate();
	for (AL1424_i_MapX = 0; AL1424_i_MapX < _dungeonMan->_currMapWidth; AL1424_i_MapX++) {
		for (AL1425_i_MapY = 0; AL1425_i_MapY < _dungeonMan->_currMapHeight; AL1425_i_MapY++) {
			if (((L1427_T_Thing = _groupMan->groupGetThing(AL1424_i_MapX, AL1425_i_MapY)) != Thing::_endOfList) && ((AL1424_i_MapX != L1431_i_LordChaosMapX) || (AL1425_i_MapY != L1432_i_LordChaosMapY))) {
				_groupMan->groupDelete(AL1424_i_MapX, AL1425_i_MapY);
			}
		}
	}
	fuseSequenceUpdate();
	/* Count and get list of text things located at 0, 0 in the current map. Their text is then printed as messages in the order specified by their first letter (which is not printed) */
	L1427_T_Thing = _dungeonMan->getSquareFirstThing(0, 0);
	AL1424_i_TextStringThingCount = 0;
	while (L1427_T_Thing != Thing::_endOfList) {
		if (L1427_T_Thing.getType() == k2_TextstringType) {
			L1435_aT_TextStringThings[AL1424_i_TextStringThingCount++] = L1427_T_Thing;
		}
		L1427_T_Thing = _dungeonMan->getNextThing(L1427_T_Thing);
	}
	L1434_c_TextFirstCharacter = 'A';
	AL1426_i_TextStringThingCount = AL1424_i_TextStringThingCount;
	while (AL1424_i_TextStringThingCount--) {
		for (AL1425_i_TextStringThingIndex = 0; AL1425_i_TextStringThingIndex < AL1426_i_TextStringThingCount; AL1425_i_TextStringThingIndex++) {
			_dungeonMan->decodeText(L1436_ac_String, L1435_aT_TextStringThings[AL1425_i_TextStringThingIndex], (TextType)(k1_TextTypeMessage | k0x8000_DecodeEvenIfInvisible));
			if (L1436_ac_String[1] == L1434_c_TextFirstCharacter) {
				_textMan->clearAllRows();
				L1436_ac_String[1] = '\n'; /* New line */
				_textMan->printMessage(k15_ColorWhite, &L1436_ac_String[1]);
				fuseSequenceUpdate();
				delay(780);
				L1434_c_TextFirstCharacter++;
				break;
			}
		}
	}

	for (AL1424_i_Attack = 55; AL1424_i_Attack <= 255; AL1424_i_Attack += 40) {
		_projexpl->createExplosion(Thing::_explHarmNonMaterial, AL1424_i_Attack, L1431_i_LordChaosMapX, L1432_i_LordChaosMapY, k255_CreatureTypeSingleCenteredCreature);
		fuseSequenceUpdate();
	}

	delay(600);
	_restartGameAllowed = false;
	endGame(true);
}

void DMEngine::fuseSequenceUpdate() {
	_timeline->processTimeline();
	_displayMan->drawDungeon(_dungeonMan->_partyDir, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
	_sound->playPendingSound();
	_eventMan->discardAllInput();
	_displayMan->updateScreen();
	delay(2);
	_gameTime++; /* BUG0_71 Some timings are too short on fast computers.
					  The ending animation when Lord Chaos is fused plays too quickly because the execution speed is not limited */
}

Common::Language DMEngine::getGameLanguage() { return _gameVersion->_desc.language; }
} // End of namespace DM
