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

/*
 * Based on the Reverse Engineering work of Christophe Fontanel,
 * maintainer of the Dungeon Master Encyclopaedia (http://dmweb.free.fr/)
 */

#include "advancedDetector.h"

#include "common/config-manager.h"
#include "common/scummsys.h"
#include "common/system.h"

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/error.h"

#include "common/file.h"
#include "common/events.h"
#include "common/array.h"
#include "common/algorithm.h"
#include "common/translation.h"

#include "engines/util.h"
#include "engines/engine.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"

#include "gui/saveload.h"

#include "dm/dm.h"
#include "dm/gfx.h"
#include "dm/dungeonman.h"
#include "dm/eventman.h"
#include "dm/menus.h"
#include "dm/champion.h"
#include "dm/loadsave.h"
#include "dm/objectman.h"
#include "dm/inventory.h"
#include "dm/text.h"
#include "dm/movesens.h"
#include "dm/group.h"
#include "dm/timeline.h"
#include "dm/projexpl.h"
#include "dm/dialog.h"
#include "dm/sounds.h"

namespace DM {

bool DMEngine::isDemo() const {
	return (bool)(_gameVersion->_desc.flags & ADGF_DEMO);
}

Direction DMEngine::turnDirRight(int16 dir) {
	Direction result = (Direction)((dir + 1) & 3);
	return result;
}

Direction DMEngine::returnOppositeDir(int16 dir) {
	Direction result = (Direction)((dir + 2) & 3);
	return result;
}

Direction DMEngine::turnDirLeft(int16 dir) {
	Direction result = (Direction)((dir + 3) & 3);
	return result;
}

bool DMEngine::isOrientedWestEast(int16 dir) {
	return dir & 1;
}

uint16 DMEngine::normalizeModulo4(int16 dir) {
	return dir & 3;
}

int32 DMEngine::filterTime(int32 mapTime) {
	return mapTime & 0x00FFFFFF;
}

int32 DMEngine::setMapAndTime(uint32 map, uint32 time) {
	return (time | (map << 24));
}

uint16 DMEngine::getMap(int32 mapTime) {
	return ((uint16)(mapTime >> 24));
}

int32 DMEngine::setMap(int32 mapTime, uint32 map) {
	return ((mapTime & 0x00FFFFFF) | (map << 24));
}

Thing DMEngine::thingWithNewCell(Thing thing, int16 cell) {
	return Thing(((thing.toUint16()) & 0x3FFF) | ((cell) << 14));
}

int16 DMEngine::getDistance(int16 mapx1, int16 mapy1, int16 mapx2, int16 mapy2) {
	return ABS(mapx1 - mapx2) + ABS(mapy1 - mapy2);
}

DMEngine::DMEngine(OSystem *syst, const DMADGameDescription *desc) :
			Engine(syst), _console(nullptr), _gameVersion(desc),
			_thingNone(0), _thingEndOfList(0xFFFE), _thingFirstExplosion(0xFF80),
			_thingExplFireBall(0xFF80), _thingExplSlime(0xFF81), _thingExplLightningBolt(0xFF82),
			_thingExplHarmNonMaterial(0xFF83), _thingExplOpenDoor(0xFF84), _thingExplPoisonBolt(0xFF86),
			_thingExplPoisonCloud(0xFF87), _thingExplSmoke(0xFFA8), _thingExplFluxcage(0xFFB2),
			_thingExplRebirthStep1(0xFFE4), _thingExplRebirthStep2(0xFFE5), _thingParty(0xFFFF)
	{
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
	_sound = nullptr;

	_engineShouldQuit = false;
	_dungeonId = 0;

	_gameMode = kDMModeLoadSavedGame;
	_restartGameRequest = false;
	_stopWaitingForPlayerInput = true;
	_gameTimeTicking = false;
	_restartGameAllowed = false;
	_pressingEye = false;
	_stopPressingEye = false;
	_pressingMouth = false;
	_stopPressingMouth = false;
	_highlightBoxInversionRequested = false;
	_projectileDisableMovementTicks = 0;
	_lastProjectileDisabledMovementDirection = 0;
	_gameWon = false;
	_newPartyMapIndex = kDMMapIndexNone;
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
	_dialog = nullptr;
}

DMEngine::~DMEngine() {
	debug("DMEngine::~DMEngine");

	// dispose of resources
	delete _rnd;
	//delete _console; Debugger is deleted by Engine
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
}

bool DMEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsSavingDuringRuntime) ||
		(f == kSupportsLoadingDuringRuntime);
}

Common::Error DMEngine::loadGameState(int slot) {
	if (loadgame(slot) != kDMLoadgameFailure) {
		_displayMan->fillScreen(kDMColorBlack);
		_displayMan->startEndFadeToPalette(_displayMan->_palDungeonView[0]);
		_gameMode = kDMModeLoadSavedGame;

		startGame();
		_restartGameRequest = false;
		_eventMan->hideMouse();
		_eventMan->discardAllInput();
		return Common::kNoError;
	}

	return Common::kNoGameDataFoundError;
}

bool DMEngine::canLoadGameStateCurrently(Common::U32String *msg) {
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
	_displayMan->loadFloorSet(kDMFloorSetStone);
	_displayMan->loadWallSet(kDMWallSetStone);

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

			if (_gameMode == kDMModeLoadSavedGame) { // if resume was clicked, bring up ScummVM load screen
				GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
				saveSlot = dialog->runModalWithCurrentTarget();
				delete dialog;
			}
		}
	} while (loadgame(saveSlot) != kDMLoadgameSuccess);

	_displayMan->loadIntoBitmap(kDMGraphicIdxMenuSpellAreLines, _menuMan->_bitmapSpellAreaLines); // @ F0396_MENUS_LoadSpellAreaLinesBitmap

	// There was some memory wizardy for the Amiga platform, I skipped that part
	_displayMan->allocateFlippedWallBitmaps();

	startGame();
	if (_gameMode != kDMModeLoadSavedGame)
		_moveSens->getMoveResult(_thingParty, kDMMapXNotOnASquare, 0, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
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
	_championMan->_actingChampionOrdinal = indexToOrdinal(kDMChampionNone);
	_menuMan->_actionAreaContainsIcons = true;
	_eventMan->_useChampionIconOrdinalAsMousePointerBitmap = indexToOrdinal(kDMChampionNone);

	_eventMan->_primaryMouseInput = _eventMan->_primaryMouseInputInterface;
	_eventMan->_secondaryMouseInput = _eventMan->_secondaryMouseInputMovement;
	_eventMan->_primaryKeyboardInput = _eventMan->_primaryKeyboardInputInterface;
	_eventMan->_secondaryKeyboardInput = _eventMan->_secondaryKeyboardInputMovement;

	processNewPartyMap(_dungeonMan->_partyMapIndex);

	if (_gameMode == kDMModeLoadSavedGame) {
		_displayMan->startEndFadeToPalette(_displayMan->_paletteTopAndBottomScreen);
		_displayMan->_useByteBoxCoordinates = false;
		delay(1);
		_displayMan->fillScreenBox(boxScreenTop, kDMColorBlack);
		_displayMan->fillScreenBox(boxScreenRight, kDMColorBlack);
		_displayMan->fillScreenBox(boxScreenBottom, kDMColorBlack);
	} else {
		_displayMan->_useByteBoxCoordinates = false;
		_displayMan->fillScreenBox(boxScreenTop, kDMColorBlack);
		_displayMan->fillScreenBox(boxScreenRight, kDMColorBlack);
		_displayMan->fillScreenBox(boxScreenBottom, kDMColorBlack);
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
	initGraphics(320, 200);
	_console = new Console(this);
	setDebugger(_console);
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
	_waitForInputMaxVerticalBlankCount = 15;
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


			if (_newPartyMapIndex != kDMMapIndexNone) {
				processNewPartyMap(_newPartyMapIndex);
				_moveSens->getMoveResult(_thingParty, kDMMapXNotOnASquare, 0, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY);
				_newPartyMapIndex = kDMMapIndexNone;
				_eventMan->discardAllInput();
			}
			_timeline->processTimeline();

			if (_newPartyMapIndex == kDMMapIndexNone)
				break;
		}

		if (!_inventoryMan->_inventoryChampionOrdinal && !_championMan->_partyIsSleeping) {
			Box box(0, 223, 0, 135);
			_displayMan->fillBoxBitmap(_displayMan->_bitmapViewport, box, kDMColorBlack, k112_byteWidthViewport, k136_heightViewport); // (possibly dummy code)
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

			if (++vblankCounter > _waitForInputMaxVerticalBlankCount)
				_stopWaitingForPlayerInput = true;
			else if (!_stopWaitingForPlayerInput)
				_system->delayMillis(10);

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

	_displayMan->loadIntoBitmap(kDMGraphicIdxEntranceRightDoor, _entranceDoorAnimSteps[4]);
	_displayMan->loadIntoBitmap(kDMGraphicIdxEntranceLeftDoor, _entranceDoorAnimSteps[0]);
	_interfaceCredits = _displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxCredits);
	_displayMan->_useByteBoxCoordinates = false;
	Box displayBox(0, 100, 0, 160);
	for (uint16 idx = 1; idx < 4; idx++) {
		_displayMan->blitToBitmap(_entranceDoorAnimSteps[0], _entranceDoorAnimSteps[idx], displayBox, idx << 2, 0, k64_byteWidth, k64_byteWidth, kDMColorNoTransparency, 161, 161);
		displayBox._rect.right -= 4;
	}
	displayBox._rect.right = 127;
	for (uint16 idx = 5; idx < 8; idx++) {
		displayBox._rect.left += 4;
		_displayMan->blitToBitmap(_entranceDoorAnimSteps[4], _entranceDoorAnimSteps[idx], displayBox, 0, 0, k64_byteWidth, k64_byteWidth, kDMColorNoTransparency, 161, 161);
	}

	do {
		drawEntrance();
		_eventMan->showMouse();
		_eventMan->discardAllInput();
		_gameMode = kDMModeWaitingOnEntrance;
		do {
			_eventMan->processInput();
			if (_engineShouldQuit)
				return;
			_eventMan->processCommandQueue();
			_displayMan->updateScreen();
		} while (_gameMode == kDMModeWaitingOnEntrance);
	} while (_gameMode == kDMModeEntranceDrawCredits);

	//Strangerke: CHECKME: Earlier versions were using G0566_puc_Graphic534_Sound01Switch
	_sound->play(kDMSoundIndexSwitch, 112, 0x40, 0x40);
	delay(20);
	_eventMan->showMouse();
	if (_gameMode != kDMModeLoadSavedGame)
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
		_sound->requestPlay(kDMSoundIndexScream, _dungeonMan->_partyMapX, _dungeonMan->_partyMapY, kDMSoundModePlayImmediately);
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
			_displayMan->fillScreen(kDMColorDarkestGray);
			for (int16 championIndex = kDMChampionFirst; championIndex < _championMan->_partyChampionCount; championIndex++) {
				int16 textPosY = championIndex * 48;
				Champion *curChampion = &_championMan->_champions[championIndex];
				_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxWallOrnChampMirror), &championMirrorBox, k32_byteWidth, kDMColorFlesh, 43);
				_displayMan->blitToScreen(curChampion->_portrait, &championPortraitBox, k16_byteWidth, kDMColorDarkGary, 29);
				_textMan->printEndGameString(87, textPosY += 14, kDMColorGold, curChampion->_name);
				int textPosX = (6 * strlen(curChampion->_name)) + 87;
				char championTitleFirstCharacter = curChampion->_title[0];
				if ((championTitleFirstCharacter != ',') && (championTitleFirstCharacter != ';') && (championTitleFirstCharacter != '-'))
					textPosX += 6;

				_textMan->printEndGameString(textPosX, textPosY++, kDMColorGold, curChampion->_title);
				for (int16 idx = kDMSkillFighter; idx <= kDMSkillWizard; idx++) {
					uint16 skillLevel = MIN<uint16>(16, _championMan->getSkillLevel(championIndex, idx | (kDMIgnoreObjectModifiers | kDMIgnoreTemporaryExperience)));
					if (skillLevel == 1)
						continue;

					Common::String displStr = Common::String::format("%s %s", _inventoryMan->_skillLevelNames[skillLevel - 2], _championMan->_baseSkillName[idx]);
					_textMan->printEndGameString(105, textPosY = textPosY + 8, kDMColorLightestGray, displStr.c_str());
				}
				championMirrorBox._rect.top += 48;
				championMirrorBox._rect.bottom += 48;
				championPortraitBox._rect.top += 48;
				championPortraitBox._rect.top += 48;
			}
			_displayMan->startEndFadeToPalette(_displayMan->_paletteTopAndBottomScreen);
			_engineShouldQuit = true;
			return;
		}
T0444017:
		_displayMan->fillScreen(kDMColorBlack);
		_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxTheEnd), &theEndBox, k40_byteWidth, kDMColorNoTransparency, 14);
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
			_displayMan->fillScreenBox(restartOuterBox, kDMColorDarkestGray);
			_displayMan->fillScreenBox(restartInnerBox, kDMColorBlack);

			switch (getGameLanguage()) { // localized
			default:
			case Common::EN_ANY:
				_textMan->printToLogicalScreen(110, 154, kDMColorCyan, kDMColorBlack, "RESTART THIS GAME");
				break;
			case Common::DE_DEU:
				_textMan->printToLogicalScreen(110, 154, kDMColorCyan, kDMColorBlack, "DIESES SPIEL NEU STARTEN");
				break;
			case Common::FR_FRA:
				_textMan->printToLogicalScreen(110, 154, kDMColorCyan, kDMColorBlack, "RECOMMENCER CE JEU");
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
				_displayMan->fillScreen(kDMColorBlack);
				_displayMan->startEndFadeToPalette(_displayMan->_palDungeonView[0]);
				_gameMode = kDMModeLoadSavedGame;
				if (loadgame(1) != kDMLoadgameFailure) {
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
	_displayMan->blitToScreen(_displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxCredits), &box, k160_byteWidthScreen, kDMColorNoTransparency, k200_heightScreen);

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

	byte **microDungeonCurrentMapData = new byte*[32];

	_dungeonMan->_partyMapIndex = kDMMapIndexEntrance;
	_displayMan->_drawFloorAndCeilingRequested = true;
	_dungeonMan->_currMapWidth = 5;
	_dungeonMan->_currMapHeight = 5;
	_dungeonMan->_currMapData = microDungeonCurrentMapData;

	Map map; // uninitialized, won't be used
	_dungeonMan->_currMap = &map;
	Square microDungeonSquares[25];
	for (uint16 i = 0; i < 25; ++i)
		microDungeonSquares[i] = Square(kDMElementTypeWall, 0);

	for (int16 idx = 0; idx < 5; idx++) {
		microDungeonCurrentMapData[idx] = (byte*)&microDungeonSquares[idx * 5];
		microDungeonSquares[idx + 10] = Square(kDMElementTypeCorridor, 0);
	}
	microDungeonSquares[7] = Square(kDMElementTypeCorridor, 0);
	_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);

	// note, a global variable is used here in the original
	_displayMan->loadIntoBitmap(kDMGraphicIdxEntrance, _displayMan->_bitmapScreen);
	_displayMan->drawDungeon(kDMDirSouth, 2, 0);

	if (!_savedScreenForOpenEntranceDoors)
		_savedScreenForOpenEntranceDoors = new byte[k200_heightScreen * k160_byteWidthScreen * 2];
	memcpy(_savedScreenForOpenEntranceDoors, _displayMan->_bitmapScreen, 320 * 200);

	_displayMan->_useByteBoxCoordinates = false;
	_displayMan->blitToBitmap(_displayMan->_bitmapScreen, _entranceDoorAnimSteps[8], doorsUpperHalfBox, 0, 30, k160_byteWidthScreen, k128_byteWidth, kDMColorNoTransparency, 200, 161);
	_displayMan->_useByteBoxCoordinates = false;
	_displayMan->blitToBitmap(_displayMan->_bitmapScreen, _entranceDoorAnimSteps[8], doorsLowerHalfBox, 0, 111, k160_byteWidthScreen, k128_byteWidth, kDMColorNoTransparency, 200, 161);

	_displayMan->blitToScreen(_entranceDoorAnimSteps[0], &closedDoorLeftBox, k64_byteWidth, kDMColorNoTransparency, 161);
	_displayMan->blitToScreen(_entranceDoorAnimSteps[4], &closedDoorRightBox, k64_byteWidth, kDMColorNoTransparency, 161);
	_displayMan->startEndFadeToPalette(palEntrance);
}

void DMEngine::openEntranceDoors() {
	Box rightDoorBox(109, 231, 30, 193);
	byte *rightDoorBitmap = _displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxEntranceRightDoor);
	Box leftDoorBox(0, 100, 30, 193);
	uint16 leftDoorBlitFrom = 0;
	byte *leftDoorBitmap = _displayMan->getNativeBitmapOrGraphic(kDMGraphicIdxEntranceLeftDoor);

	Box screenBox(0, 319, 0, 199);

	for (uint16 animStep = 1; animStep < 32; ++animStep) {
		if ((animStep % 3) == 1) {
			// Strangerke: CHECKME: Earlier versions of the game were using G0565_puc_Graphic535_Sound02DoorRattle instead of k02_soundDOOR_RATTLE 2
			_sound->play(kDMSoundIndexDoorRattle, 145, 0x40, 0x40);
		}

		_displayMan->blitToScreen(_savedScreenForOpenEntranceDoors, &screenBox, 160, kDMColorNoTransparency, 200);
		_displayMan->blitToBitmap(leftDoorBitmap, _displayMan->_bitmapScreen, leftDoorBox, leftDoorBlitFrom, 0, 64, k160_byteWidthScreen,
								  kDMColorNoTransparency, 161, k200_heightScreen);
		_displayMan->blitToBitmap(rightDoorBitmap, _displayMan->_bitmapScreen, rightDoorBox, 0, 0, 64, k160_byteWidthScreen,
								  kDMColorNoTransparency, 161, k200_heightScreen);
		_eventMan->discardAllInput();
		_displayMan->updateScreen();

		leftDoorBox._rect.right -= 4;
		leftDoorBlitFrom += 4;
		rightDoorBox._rect.left += 4;

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
	_displayMan->loadIntoBitmap(kDMGraphicIdxTitle, titleSteps);

	titleSteps += 320 * 200;
	uint16 blitPalette[16];
	for (uint16 i = 0; i < 16; ++i)
		blitPalette[i] = D01_RGB_DARK_BLUE;

	_displayMan->startEndFadeToPalette(blitPalette);
	_displayMan->fillScreen(kDMColorBlack);
	// uncomment this to draw 'Presents'
	_displayMan->blitToBitmap(bitmapTitle, _displayMan->_bitmapScreen, boxTitlePresents, 0, 137, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorNoTransparency, k200_heightScreen, k200_heightScreen);
	blitPalette[15] = D09_RGB_WHITE;
	_displayMan->startEndFadeToPalette(blitPalette);
	byte *masterStrikesBack = titleSteps;
	_displayMan->blitToBitmap(bitmapTitle, masterStrikesBack, boxTitleStrikesBackSource, 0, 80, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorNoTransparency, 200, 57);
	titleSteps += 320 * 57;
	byte *bitmapDungeonChaos = titleSteps; /* Unreferenced on Atari ST */
	_displayMan->blitToBitmap(bitmapTitle, bitmapDungeonChaos, boxTitleDungeonChaos, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorNoTransparency, 200, 80);
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
	_displayMan->fillScreen(kDMColorBlack);
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
		Box box(blitCoordinates[i][0], blitCoordinates[i][1], blitCoordinates[i][2], blitCoordinates[i][3]);
		_displayMan->blitToBitmap(shrinkedTitle[i], _displayMan->_bitmapScreen, box, 0, 0, blitCoordinates[i][4], k160_byteWidthScreen, kDMColorNoTransparency, blitCoordinates[i][3] - blitCoordinates[i][2] + 1, k200_heightScreen);
	}
	delay(25);
	_displayMan->blitToBitmap(masterStrikesBack, _displayMan->_bitmapScreen, boxTitleStrikesBackDestination, 0, 0, k160_byteWidthScreen, k160_byteWidthScreen, kDMColorBlack, 57, k200_heightScreen);
	blitPalette[10] = D00_RGB_BLACK;
	blitPalette[12] = D07_RGB_RED;
	_displayMan->startEndFadeToPalette(blitPalette);
	delete[] allocatedMem;
	delay(75);
}

void DMEngine::entranceDrawCredits() {
	_eventMan->showMouse();
	_displayMan->startEndFadeToPalette(_displayMan->_blankBuffer);
	_displayMan->loadIntoBitmap(kDMGraphicIdxCredits, _displayMan->_bitmapScreen);
	_displayMan->startEndFadeToPalette(_displayMan->_palCredits);
	delay(50);
	_eventMan->waitForMouseOrKeyActivity();
	_gameMode = kDMModeEntranceDrawCredits;
}

void DMEngine::fuseSequence() {
	_gameWon = true;
	if (_inventoryMan->_inventoryChampionOrdinal)
		_inventoryMan->toggleInventory(kDMChampionCloseInventory);

	_eventMan->highlightBoxDisable();
	_championMan->_party._magicalLightAmount = 200;
	_inventoryMan->setDungeonViewPalette();
	_championMan->_party._fireShieldDefense = _championMan->_party._spellShieldDefense = _championMan->_party._shieldDefense = 100;
	_timeline->refreshAllChampionStatusBoxes();
	fuseSequenceUpdate();
	int16 lordChaosMapX = _dungeonMan->_partyMapX;
	int16 lordChaosMapY = _dungeonMan->_partyMapY;
	lordChaosMapX += _dirIntoStepCountEast[_dungeonMan->_partyDir];
	lordChaosMapY += _dirIntoStepCountNorth[_dungeonMan->_partyDir];
	Thing lordChaosThing = _groupMan->groupGetThing(lordChaosMapX, lordChaosMapY);
	Group *lordGroup = (Group*)_dungeonMan->getThingData(lordChaosThing);
	lordGroup->_health[0] = 10000;
	_dungeonMan->setGroupCells(lordGroup, kDMCreatureTypeSingleCenteredCreature, _dungeonMan->_partyMapIndex);
	_dungeonMan->setGroupDirections(lordGroup, returnOppositeDir(_dungeonMan->_partyDir), _dungeonMan->_partyMapIndex);

	bool removeFluxcagesFromLordChaosSquare = true;
	int16 fluxCageMapX = _dungeonMan->_partyMapX;
	int16 fluxcageMapY = _dungeonMan->_partyMapY;

	for (;;) {
		Thing curThing = _dungeonMan->getSquareFirstObject(fluxCageMapX, fluxcageMapY);
		while (curThing != _thingEndOfList) {
			if (curThing.getType() == kDMThingTypeExplosion) {
				Explosion *curExplosion = (Explosion*)_dungeonMan->getThingData(curThing);
				if (curExplosion->getType() == kDMExplosionTypeFluxcage) {
					_dungeonMan->unlinkThingFromList(curThing, Thing(0), fluxCageMapX, fluxcageMapY);
					curExplosion->setNextThing(_thingNone);
					continue;
				}
			}
			curThing = _dungeonMan->getNextThing(curThing);
		}
		if (removeFluxcagesFromLordChaosSquare) {
			removeFluxcagesFromLordChaosSquare = false;
			fluxCageMapX = lordChaosMapX;
			fluxcageMapY = lordChaosMapY;
		} else
			break;
	}
	fuseSequenceUpdate();
	for (int16 attackId = 55; attackId <= 255; attackId += 40) {
		_projexpl->createExplosion(_thingExplFireBall, attackId, lordChaosMapX, lordChaosMapY, kDMCreatureTypeSingleCenteredCreature);
		fuseSequenceUpdate();
	}
	_sound->requestPlay(kDMSoundIndexBuzz, lordChaosMapX, lordChaosMapY, kDMSoundModePlayIfPrioritized);
	lordGroup->_type = kDMCreatureTypeLordOrder;
	fuseSequenceUpdate();
	for (int16 attackId = 55; attackId <= 255; attackId += 40) {
		_projexpl->createExplosion(_thingExplHarmNonMaterial, attackId, lordChaosMapX, lordChaosMapY, kDMCreatureTypeSingleCenteredCreature);
		fuseSequenceUpdate();
	}
	for (int16 cycleCount = 3; cycleCount > 0; cycleCount--) {
		for (int16 switchCount = 4; switchCount > 0; switchCount--) {
			_sound->requestPlay(kDMSoundIndexBuzz, lordChaosMapX, lordChaosMapY, kDMSoundModePlayIfPrioritized);
			lordGroup->_type = (switchCount & 0x0001) ? kDMCreatureTypeLordOrder : kDMCreatureTypeLordChaos;
			for (int16 fuseSequenceUpdateCount = cycleCount - 1; fuseSequenceUpdateCount >= 0; fuseSequenceUpdateCount--)
				fuseSequenceUpdate();
		}
	}
	_projexpl->createExplosion(_thingExplFireBall, 255, lordChaosMapX, lordChaosMapY, kDMCreatureTypeSingleCenteredCreature);
	_projexpl->createExplosion(_thingExplHarmNonMaterial, 255, lordChaosMapX, lordChaosMapY, kDMCreatureTypeSingleCenteredCreature);
	fuseSequenceUpdate();
	lordGroup->_type = kDMCreatureTypeGreyLord;
	fuseSequenceUpdate();
	_displayMan->_doNotDrawFluxcagesDuringEndgame = true;
	fuseSequenceUpdate();
	for (int16 curMapX = 0; curMapX < _dungeonMan->_currMapWidth; curMapX++) {
		for (int curMapY = 0; curMapY < _dungeonMan->_currMapHeight; curMapY++) {
			Thing curThing = _groupMan->groupGetThing(curMapX, curMapY);
			if ((curThing != _thingEndOfList) && ((curMapX != lordChaosMapX) || (curMapY != lordChaosMapY))) {
				_groupMan->groupDelete(curMapX, curMapY);
			}
		}
	}
	fuseSequenceUpdate();
	/* Count and get list of text things located at 0, 0 in the current map. Their text is then printed as messages in the order specified by their first letter (which is not printed) */
	Thing curThing = _dungeonMan->getSquareFirstThing(0, 0);
	int16 textStringThingCount = 0;
	Thing textStringThings[8];
	while (curThing != _thingEndOfList) {
		if (curThing.getType() == kDMstringTypeText)
			textStringThings[textStringThingCount++] = curThing;

		curThing = _dungeonMan->getNextThing(curThing);
	}
	char textFirstChar = 'A';
	int16 maxCount = textStringThingCount;
	while (textStringThingCount--) {
		for (int16 idx = 0; idx < maxCount; idx++) {
			char decodedString[200];
			_dungeonMan->decodeText(decodedString, sizeof(decodedString),
					textStringThings[idx], (TextType)(kDMTextTypeMessage | kDMMaskDecodeEvenIfInvisible));
			if (decodedString[1] == textFirstChar) {
				_textMan->clearAllRows();
				decodedString[1] = '\n'; /* New line */
				_textMan->printMessage(kDMColorWhite, &decodedString[1]);
				fuseSequenceUpdate();
				delay(780);
				textFirstChar++;
				break;
			}
		}
	}

	for (int16 attackId = 55; attackId <= 255; attackId += 40) {
		_projexpl->createExplosion(_thingExplHarmNonMaterial, attackId, lordChaosMapX, lordChaosMapY, kDMCreatureTypeSingleCenteredCreature);
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

Common::Language DMEngine::getGameLanguage() {
	return _gameVersion->_desc.language;
}

} // End of namespace DM
