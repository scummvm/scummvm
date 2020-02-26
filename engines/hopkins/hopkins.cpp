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

#include "hopkins/hopkins.h"
#include "hopkins/graphics.h"
#include "hopkins/files.h"
#include "hopkins/saveload.h"
#include "hopkins/sound.h"
#include "hopkins/talk.h"

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/file.h"

namespace Hopkins {

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins") {
	DebugMan.addDebugChannel(kDebugPath, "Path", "Pathfinding debug level");
	DebugMan.addDebugChannel(kDebugGraphics, "Graphics", "Graphics debug level");
	_animMan = new AnimationManager(this);
	_computer = new ComputerManager(this);
	_dialog = new DialogsManager(this);
	setDebugger(new Debugger(this));
	_events = new EventsManager(this);
	_fileIO = new FileManager(this);
	_fontMan = new FontManager(this);
	_globals = new Globals(this);
	_graphicsMan = new GraphicsManager(this);
	_linesMan = new LinesManager(this);
	_menuMan = new MenuManager(this);
	_objectsMan = new ObjectsManager(this);
	_saveLoad = new SaveLoadManager(this);
	_script = new ScriptManager(this);
	_soundMan = new SoundManager(this);
	_talkMan = new TalkManager(this);

	_startGameSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
}

HopkinsEngine::~HopkinsEngine() {
	delete _talkMan;
	delete _soundMan;
	delete _script;
	delete _saveLoad;
	delete _objectsMan;
	delete _menuMan;
	delete _linesMan;
	delete _graphicsMan;
	delete _globals;
	delete _fontMan;
	delete _fileIO;
	delete _events;
	delete _dialog;
	delete _computer;
	delete _animMan;
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool HopkinsEngine::canLoadGameStateCurrently() {
	return !_globals->_exitId && !_globals->_cityMapEnabledFl && _events->_mouseFl && _globals->_curRoomNum != 0;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool HopkinsEngine::canSaveGameStateCurrently() {
	return !_globals->_exitId && !_globals->_cityMapEnabledFl && _events->_mouseFl
		&& _globals->_curRoomNum != 0 && !isUnderwaterSubScene();
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error HopkinsEngine::loadGameState(int slot) {
	return _saveLoad->loadGame(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error HopkinsEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return _saveLoad->saveGame(slot, desc);
}

Common::Error HopkinsEngine::run() {
	_globals->setConfig();
	_fileIO->initCensorship();
	initializeSystem();

	if (!getIsDemo())
		runFull();
	else if (getPlatform() == Common::kPlatformLinux)
		runLinuxDemo();
	else if (getPlatform() == Common::kPlatformWindows)
		runWin95Demo();
	else {
		warning("Unhandled version, switching to Linux demo. Please report this version to ScummVM developers");
		runLinuxDemo();
	}

	return Common::kNoError;
}

bool HopkinsEngine::runWin95Demo() {
	_objectsMan->loadObjects();
	_objectsMan->changeObject(14);
	_objectsMan->addObject(14);
	_objectsMan->_helicopterFl = false;

	_globals->_eventMode = EVENTMODE_IGNORE;

	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();

	if (_startGameSlot == -1) {
		_graphicsMan->loadImage("H2");
		_graphicsMan->fadeInLong();

		if (!_events->_escKeyFl)
			playIntro();
	}

	_events->_rateCounter = 0;
	_globals->_eventMode = EVENTMODE_IGNORE;
	_globals->_speed = 1;
	_events->delay(500);
	_globals->_eventMode = EVENTMODE_DEFAULT;
	if (_events->_rateCounter > 475)
		_globals->_speed = 2;
	if (_events->_rateCounter > 700)
		_globals->_speed = 3;

	if (_startGameSlot == -1)
		_graphicsMan->fadeOutShort();

	_globals->_eventMode = EVENTMODE_IGNORE;
	_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");

	_globals->_characterType = CHARACTER_HOPKINS;
	_objectsMan->_mapCarPosX = _objectsMan->_mapCarPosY = 0;
	_globals->_saveData->reset();
	_globals->_exitId = 0;

	if (getLanguage() != Common::PL_POL)
		if (!displayAdultDisclaimer())
			return Common::kNoError;

	if (_startGameSlot != -1)
		_saveLoad->loadGame(_startGameSlot);

	for (;;) {
		if (_globals->_exitId == 300)
			_globals->_exitId = 0;

		if (!_globals->_exitId) {
			_globals->_exitId = _menuMan->menu();
			if (_globals->_exitId == -1) {
				_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
				restoreSystem();
				return false;
			}
		}

		if (shouldQuit())
			return false;

		_globals->_curRoomNum = _globals->_exitId;

		switch (_globals->_exitId) {
		case 1:
			// Handles room: Apartment
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM01", "IM01", "ANIM01", "IM01", 2, true);
			break;

		case 3:
			// - Displays bank attack when leaving the apartment
			// - Handles room: bottom of the apartment
			if (!_globals->_saveData->_data[svBankAttackAnimPlayedFl]) {
				_soundMan->playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsMan->loadImage("fond");
				else {
					switch (_globals->_language) {
					case LANG_FR:
						_graphicsMan->loadImage("fondfr");
						break;
					case LANG_EN:
						_graphicsMan->loadImage("fondan");
						break;
					case LANG_SP:
						_graphicsMan->loadImage("fondes");
						break;
					default:
						break;
					}
				}
				_graphicsMan->fadeInLong();
				_events->delay(500);
				_graphicsMan->fadeOutLong();
				_globals->_eventMode = EVENTMODE_IGNORE;
				_soundMan->_specialSoundNum = 2;
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				if (!_globals->_censorshipFl)
					_animMan->playAnim("BANQUE.ANM", "BANKUK.ANM", 200, 28, 200);
				else
					_animMan->playAnim("BANKUK.ANM", "BANQUE.ANM", 200, 28, 200);
				_soundMan->_specialSoundNum = 0;
				_soundMan->removeSample(1);
				_soundMan->removeSample(2);
				_soundMan->removeSample(3);
				_soundMan->removeSample(4);
				_graphicsMan->fadeOutShort();
				_globals->_saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			// Handle room: City map
			_globals->_disableInventFl = true;
			_objectsMan->handleCityMap();
			_globals->_disableInventFl = false;
			break;

		case 5:
			// Handle room: Outside the bank
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 455;

			if (_globals->_saveData->_data[svFreedHostageFl]) {
				if (_globals->_saveData->_data[svFreedHostageFl] == 1)
					_objectsMan->sceneControl2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			} else {
				_objectsMan->sceneControl2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			}
			break;

		case 6:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 460;
			_objectsMan->sceneControl2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals->_saveData->_data[svBombBoxOpenedFl])
				_objectsMan->sceneControl("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsMan->sceneControl("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_globals->_characterMaxPosY = 440;
			_linesMan->setMaxLineIdx(20);
			if (_globals->_saveData->_data[svBombDisarmedFl])
			  _objectsMan->sceneControl2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			else
			  bombExplosion();
			break;

		case 10:
			_objectsMan->sceneControl("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_globals->_characterMaxPosY = 450;
			_linesMan->setMaxLineIdx(20);
			if (_globals->_saveData->_data[svBombDisarmedFl]) {
				if (_globals->_language == LANG_FR)
					_graphicsMan->loadImage("ENDFR");
				else
					_graphicsMan->loadImage("ENDUK");
				_graphicsMan->fadeInLong();
				_events->mouseOn();
				do {
					_events->refreshScreenAndEvents();
				} while (_events->getMouseButton() != 1);
				_graphicsMan->fadeOutLong();
				restoreSystem();
			} else
				bombExplosion();
			break;

		case 13:
		case 14:
		case 15:
			handleNotAvailable(11);
			break;

		case 16:
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 33:
		case 32:
		case 34:
			handleNotAvailable(4);
			break;

		case 17:
			handleNotAvailable(1);
			break;

		case 111:
			_objectsMan->sceneControl("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsMan->sceneControl("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 113;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_computer->showComputer(COMPUTER_HOPKINS);
			_graphicsMan->clearScreen();
			_graphicsMan->updateScreen();
			memset(_graphicsMan->_frontBuffer, 0, 307200);
			memset(_graphicsMan->_backBuffer, 0, 307200);
			_graphicsMan->clearPalette();
			_graphicsMan->resetDirtyRects();
			break;

		case 114:
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 114;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_globals->_exitId = 0;
			_computer->showComputer(COMPUTER_SAMANTHA);
			_graphicsMan->clearScreen();
			break;

		case 115:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 115;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_computer->showComputer(COMPUTER_PUBLIC);
			_graphicsMan->clearScreen();
			break;

		case 150:
			_soundMan->playSound(28);
			_globals->_eventMode = EVENTMODE_ALT; // CHECKME!
			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			_animMan->playAnim("JOUR1A.ANM", "JOUR1A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 151:
			if (_fileIO->fileExists("JOUR3A.ANM")) {
				// The Polish demo uses the animation file than the complete versions
				_soundMan->playSound(16);
				_globals->_eventMode = EVENTMODE_IGNORE;

				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_graphicsMan->_fadingFl = true;
				_animMan->playAnim("JOUR3A.ANM", "JOUR3A.ANM", 12, 12, 2000);
			} else {
				// The other demos only display a nag screen
				_soundMan->playSound(28);
				_globals->_eventMode = EVENTMODE_ALT; // CHECKME!
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_graphicsMan->loadImage("njour3a");
				_graphicsMan->fadeInLong();
				_events->delay(5000);
				_graphicsMan->fadeOutLong();
			}

			_globals->_exitId = 300;
			_globals->_eventMode = EVENTMODE_DEFAULT;
			break;

		case 152:
			_soundMan->playSound(28);
			_globals->_eventMode = EVENTMODE_ALT; // CHECKME!
			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			_animMan->playAnim("JOUR4A.ANM", "JOUR4A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		default:
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runLinuxDemo() {
	_objectsMan->loadObjects();
	_objectsMan->changeObject(14);
	_objectsMan->addObject(14);
	_objectsMan->_helicopterFl = false;

	_events->mouseOff();

	_graphicsMan->clearScreen();

	if (_startGameSlot == -1) {
		_graphicsMan->loadImage("LINUX");
		_graphicsMan->fadeInLong();
		_events->delay(1500);
		_graphicsMan->fadeOutLong();

		_graphicsMan->loadImage("H2");
		_graphicsMan->fadeInLong();
		_events->delay(500);
		_graphicsMan->fadeOutLong();

		if (!_events->_escKeyFl)
			playIntro();
	}

	_globals->_eventMode = EVENTMODE_DEFAULT;
	_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");
	_globals->_characterType = CHARACTER_HOPKINS;
	_objectsMan->_mapCarPosX = _objectsMan->_mapCarPosY = 0;
	_globals->_saveData->reset();
	_globals->_exitId = 0;

	if (_startGameSlot != -1)
		_saveLoad->loadGame(_startGameSlot);

	for (;;) {
		if (_globals->_exitId == 300)
			_globals->_exitId = 0;

		if (!_globals->_exitId) {
			_globals->_exitId = _menuMan->menu();
			if (_globals->_exitId == -1) {
				if (!shouldQuit())
					endLinuxDemo();
				_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
				restoreSystem();
			}
		}

		if (shouldQuit())
			return false;

		_globals->_curRoomNum = _globals->_exitId;

		switch (_globals->_exitId) {
		case 17:
		case 18:
		case 19:
		case 20:
		case 22:
		case 23:
		case 24:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
		case 32:
		case 34:
		case 38:
			displayNotAvailable();
			break;

		case 1:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM01", "IM01", "ANIM01", "IM01", 1, true);
			break;

		case 3:
			if (!_globals->_saveData->_data[svBankAttackAnimPlayedFl]) {
				_soundMan->playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsMan->loadImage("fond");
				else {
					switch (_globals->_language) {
					case LANG_FR:
						_graphicsMan->loadImage("fondfr");
						break;
					case LANG_EN:
						_graphicsMan->loadImage("fondan");
						break;
					case LANG_SP:
						_graphicsMan->loadImage("fondes");
						break;
					default:
						break;
					}
				}
				_graphicsMan->fadeInLong();
				_events->delay(500);
				_graphicsMan->fadeOutLong();
				_globals->_eventMode = EVENTMODE_IGNORE;
				_soundMan->_specialSoundNum = 2;

				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_graphicsMan->_fadingFl = true;

				if (!_globals->_censorshipFl)
					_animMan->playAnim("BANQUE.ANM", "BANKUK.ANM", 200, 28, 200);
				else
					_animMan->playAnim("BANKUK.ANM", "BANQUE.ANM", 200, 28, 200);
				_soundMan->_specialSoundNum = 0;
				_soundMan->removeSample(1);
				_soundMan->removeSample(2);
				_soundMan->removeSample(3);
				_soundMan->removeSample(4);
				_globals->_saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}

			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			_globals->_disableInventFl = true;
			_objectsMan->handleCityMap();
			_globals->_disableInventFl = false;
			break;

		case 5:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 455;
			if (_globals->_saveData->_data[svFreedHostageFl] == 1)
					_objectsMan->sceneControl2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			else
				_objectsMan->sceneControl2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			break;

		case 6:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 460;
			_objectsMan->sceneControl2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals->_saveData->_data[svBombBoxOpenedFl])
				_objectsMan->sceneControl("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsMan->sceneControl("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 440;

			if (!_globals->_saveData->_data[svBombDisarmedFl])
				bombExplosion();
			else
				_objectsMan->sceneControl2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			break;

		case 10:
			_objectsMan->sceneControl("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 450;
			if (_globals->_saveData->_data[svBombDisarmedFl])
				_objectsMan->sceneControl2("IM12", "IM12", "ANIM12", "IM12", 1, false);
			else
				bombExplosion();
			break;

		case 13:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM13", "IM13", "ANIM13", "IM13", 1, true);
			break;

		case 14:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM14", "IM14", "ANIM14", "IM14", 1, true);
			break;

		case 15:
			_objectsMan->sceneControl("IM15", "IM15", "ANIM15", "IM15", 29, false);
			break;

		case 16:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;

			if (_globals->_saveData->_data[svForestAvailableFl] == 1) {
				_objectsMan->sceneControl2("IM16", "IM16A", "ANIM16", "IM16", 7, true);
			} else if (!_globals->_saveData->_data[svForestAvailableFl]) {
				_objectsMan->sceneControl2("IM16", "IM16", "ANIM16", "IM16", 7, true);
			}
			break;

		case 25:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM25", "IM25", "ANIM25", "IM25", 30, true);
			break;

		case 26:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM26", "IM26", "ANIM26", "IM26", 30, true);
			break;

		case 33:
			_objectsMan->sceneControl("IM33", "IM33", "ANIM33", "IM33", 8, false);
			break;

		case 35:
			displayEndDemo();
			break;

		case 111:
			_objectsMan->sceneControl("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsMan->sceneControl("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 113;
			_globals->_saveData->_data[svLastScreenId] = 113;
			_computer->showComputer(COMPUTER_HOPKINS);

			_graphicsMan->clearScreen();
			_graphicsMan->updateScreen();
			memset(_graphicsMan->_frontBuffer, 0, 307200);
			memset(_graphicsMan->_backBuffer, 0, 307200);
			_graphicsMan->clearPalette();
			_graphicsMan->resetDirtyRects();
			break;

		case 114:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 114;
			_globals->_saveData->_data[svLastScreenId] = 114;
			_computer->showComputer(COMPUTER_SAMANTHA);
			_graphicsMan->clearScreen();
			break;

		case 115:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_screenId;
			_globals->_screenId = 115;
			_globals->_saveData->_data[svLastScreenId] = 115;
			_computer->showComputer(COMPUTER_PUBLIC);
			_graphicsMan->clearScreen();
			break;

		case 150:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;

			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR1A.ANM", "JOUR1A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 151:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;

			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR3A.ANM", "JOUR3A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 152:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;

			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR4A.ANM", "JOUR4A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		default:
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runFull() {
	if (_startGameSlot == -1 && getPlatform() == Common::kPlatformLinux)
		_soundMan->playSound(16);

	_objectsMan->loadObjects();
	_objectsMan->changeObject(14);
	_objectsMan->addObject(14);

	if (getPlatform() == Common::kPlatformLinux) {
		_objectsMan->_helicopterFl = false;
		_events->mouseOff();
		// No code has been added to display the version as it's wrong
		// in my copy: it mentions a Win95 version v4 using DirectDraw (Strangerke)
	} else if (getPlatform() == Common::kPlatformWindows) {
		_objectsMan->_helicopterFl = false;
		_globals->_eventMode = EVENTMODE_IGNORE;
		// This code displays the game version.
		// It wasn't present in the original and could be put in the debugger
		// It has been added there for debug purposes
		if (_startGameSlot == -1) {
			_graphicsMan->loadImage("VERSW");
			_graphicsMan->fadeInLong();
			_events->delay(500);
			_graphicsMan->fadeOutLong();
		}
		_graphicsMan->clearVesaScreen();
	} else {
		// This piece of code, though named "display_version" in the original,
		// displays a "loading please wait" screen.
		if (_startGameSlot == -1) {
			_graphicsMan->loadImage("VERSW");
			_graphicsMan->fadeInLong();
			_events->delay(500);
			_graphicsMan->fadeOutLong();
		}
		_graphicsMan->clearVesaScreen();

		_globals->_eventMode = EVENTMODE_IGNORE;
	}

	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();

	if (_startGameSlot == -1) {
		if (getPlatform() == Common::kPlatformLinux) {
			_graphicsMan->loadImage("H2");
			_graphicsMan->fadeInLong();
			_events->delay(500);
			_graphicsMan->fadeOutLong();
			_globals->_speed = 2;
			_globals->_eventMode = EVENTMODE_IGNORE;
			_graphicsMan->_fadingFl = true;
			_animMan->playAnim("MP.ANM", "MP.ANM", 10, 16, 200);
		} else {
			_animMan->playAnim("MP.ANM", "MP.ANM", 10, 16, 200);
			_graphicsMan->fadeOutLong();
		}
	}

	_events->mouseOff();

	if (!_events->_escKeyFl && _startGameSlot == -1) {
		playIntro();
		if (shouldQuit())
			return false;
	}

	if (getPlatform() != Common::kPlatformLinux && _startGameSlot == -1) {
		_graphicsMan->fadeOutShort();
		_graphicsMan->loadImage("H2");
		_graphicsMan->fadeInLong();
		_events->delay(500);
		_graphicsMan->fadeOutLong();
	}
	_globals->_eventMode = EVENTMODE_DEFAULT;
	_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");
	_globals->_characterType = CHARACTER_HOPKINS;
	_objectsMan->_mapCarPosX = _objectsMan->_mapCarPosY = 0;
	_globals->_saveData->reset();

	_globals->_exitId = 0;


	if (_startGameSlot != -1) {
		_soundMan->playSound(28);
		_saveLoad->loadGame(_startGameSlot);
	}

	for (;;) {
		if (_globals->_exitId == 300)
			_globals->_exitId = 0;
		if (!_globals->_exitId) {
			_globals->_exitId = _menuMan->menu();
			if (_globals->_exitId == -1) {
				_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
				restoreSystem();
				return false;
			}
		}

		if (shouldQuit())
			return false;

		_globals->_curRoomNum = _globals->_exitId;

		switch (_globals->_exitId) {
		case 1:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM01", "IM01", "ANIM01", "IM01", 1, true);
			break;

		case 3:
			if (!_globals->_saveData->_data[svBankAttackAnimPlayedFl]) {
				// Play the bank attack animation
				_soundMan->playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsMan->loadImage("fond");
				else {
					switch (_globals->_language) {
					case LANG_FR:
						_graphicsMan->loadImage("fondfr");
						break;
					case LANG_EN:
						_graphicsMan->loadImage("fondan");
						break;
					case LANG_SP:
						_graphicsMan->loadImage("fondes");
						break;
					default:
						break;
					}
				}
				_graphicsMan->fadeInLong();
				_events->delay(500);
				_graphicsMan->fadeOutLong();
				_globals->_eventMode = EVENTMODE_IGNORE;
				_soundMan->_specialSoundNum = 2;
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows) {
					if (getPlatform() == Common::kPlatformLinux)
						_graphicsMan->_fadingFl = true;

					if (!_globals->_censorshipFl)
						_animMan->playAnim("BANQUE.ANM", "BANKUK.ANM", 200, 28, 200);
					else
						_animMan->playAnim("BANKUK.ANM", "BANQUE.ANM", 200, 28, 200);
				} else {
					_animMan->playAnim("BANQUE.ANM", "BANKUK.ANM", 200, 28, 200);
				}

				_soundMan->_specialSoundNum = 0;
				_soundMan->removeSample(1);
				_soundMan->removeSample(2);
				_soundMan->removeSample(3);
				_soundMan->removeSample(4);

				if (getPlatform() != Common::kPlatformLinux) {
					// Copy the end of the animation into the secondary buffer and fade out the screen
					Common::fill(_graphicsMan->_frontBuffer, _graphicsMan->_frontBuffer +
						SCREEN_WIDTH * 2 * SCREEN_HEIGHT, 0);
					_graphicsMan->fadeOutLong();
				}

				_globals->_saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			_globals->_disableInventFl = true;
			_objectsMan->handleCityMap();
			_globals->_disableInventFl = false;
			break;

		case 5:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 455;
			if (_globals->_saveData->_data[svFreedHostageFl] == 1)
				_objectsMan->sceneControl2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			else
				_objectsMan->sceneControl2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			break;

		case 6:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 460;
			_objectsMan->sceneControl2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals->_saveData->_data[svBombBoxOpenedFl])
				_objectsMan->sceneControl("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsMan->sceneControl("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 440;
			if (_globals->_saveData->_data[svBombDisarmedFl])
				_objectsMan->sceneControl2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			else
				bombExplosion();
			break;

		case 10:
			_objectsMan->sceneControl("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 450;
			_objectsMan->sceneControl2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 450;
			if (_globals->_saveData->_data[svBombDisarmedFl])
				_objectsMan->sceneControl2("IM12", "IM12", "ANIM12", "IM12", 1, false);
			else
				bombExplosion();
			break;

		case 13:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM13", "IM13", "ANIM13", "IM13", 1, true);
			break;

		case 14:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM14", "IM14", "ANIM14", "IM14", 1, true);
			break;

		case 15:
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsMan->sceneControl("IM15", "IM15", "ANIM15", "IM15", 29, false);
			else
				_objectsMan->sceneControl("IM15", "IM15", "ANIM15", "IM15", 18, false);
			break;

		case 16:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			if (_globals->_saveData->_data[svForestAvailableFl] == 1)
				_objectsMan->sceneControl2("IM16", "IM16A", "ANIM16", "IM16", 7, true);
			else
				_objectsMan->sceneControl2("IM16", "IM16", "ANIM16", "IM16", 7, true);
			break;

		case 17:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			if (_globals->_saveData->_data[svHutBurningFl] == 1)
				_objectsMan->sceneControl2("IM17", "IM17A", "ANIM17", "IM17", 11, true);
			else if (!_globals->_saveData->_data[svHutBurningFl])
				_objectsMan->sceneControl2("IM17", "IM17", "ANIM17", "IM17", 11, true);
			if (_globals->_exitId == 18) {
				_globals->_eventMode = EVENTMODE_IGNORE;
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_soundMan->stopSound();
				if (getPlatform() == Common::kPlatformLinux) {
					_soundMan->playSound(29);
					_graphicsMan->_fadingFl = true;
					_animMan->playAnim("PURG1A.ANM", "PURG1.ANM", 12, 18, 50);
				} else if (getPlatform() == Common::kPlatformWindows) {
					_soundMan->playSound(29);
					_animMan->playAnim("PURG1A.ANM", "PURG1.ANM", 12, 18, 50);
					_graphicsMan->fadeOutShort();
				} else {
					_soundMan->playSound(6);
					_animMan->playAnim("PURG1A.ANM", "PURG1.ANM", 12, 18, 50);
					_graphicsMan->fadeOutShort();
				}
				_globals->_eventMode = EVENTMODE_DEFAULT;
			}
			break;

		case 18:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsMan->sceneControl2("IM18", "IM18", "ANIM18", "IM18", 29, false);
			else
				_objectsMan->sceneControl2("IM18", "IM18", "ANIM18", "IM18", 6, false);
			break;

		case 19:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 440;
			if (_globals->_saveData->_data[svHeavenGuardGoneFl])
				_objectsMan->sceneControl2("IM19", "IM19A", "ANIM19", "IM19", 6, true);
			else
				_objectsMan->sceneControl2("IM19", "IM19", "ANIM19", "IM19", 6, true);
			break;

		case 20:
			_linesMan->setMaxLineIdx(10);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM20", "IM20", "ANIM20", "IM20", 6, true);
			if (_globals->_exitId == 17) {
				_globals->_eventMode = EVENTMODE_IGNORE;
				_soundMan->stopSound();
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_soundMan->playSound(6);
				if (getPlatform() == Common::kPlatformLinux)
					_graphicsMan->_fadingFl = true;
				_animMan->playAnim("PURG2A.ANM", "PURG2.ANM", 12, 18, 50);
				if (getPlatform() != Common::kPlatformLinux)
					_graphicsMan->fadeOutShort();
				_globals->_eventMode = EVENTMODE_DEFAULT;
			}
			break;

		case 22:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM22", "IM22", "ANIM22", "IM22", 6, true);
			break;

		case 23:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM23", "IM23", "ANIM23", "IM23", 6, true);
			break;

		case 24:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			if (_globals->_saveData->_data[svCinemaDogGoneFl] == 1)
				_objectsMan->sceneControl2("IM24", "IM24A", "ANIM24", "IM24", 1, true);
			else
				_objectsMan->sceneControl2("IM24", "IM24", "ANIM24", "IM24", 1, true);
			break;

		case 25:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 445;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsMan->sceneControl2("IM25", "IM25", "ANIM25", "IM25", 30, true);
			else
				_objectsMan->sceneControl2("IM25", "IM25", "ANIM25", "IM25", 8, true);
			break;

		case 26:
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsMan->sceneControl2("IM26", "IM26", "ANIM26", "IM26", 30, true);
			else
				_objectsMan->sceneControl2("IM26", "IM26", "ANIM26", "IM26", 8, true);
			break;

		case 27:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 440;
			if (_globals->_saveData->_data[svPoolDogGoneFl] == 1)
				_objectsMan->sceneControl2("IM27", "IM27A", "ANIM27", "IM27", 27, true);
			else
				_objectsMan->sceneControl2("IM27", "IM27", "ANIM27", "IM27", 27, true);
			break;

		case 28:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 450;
			if (_globals->_saveData->_data[svCinemaCurtainCond1] != 1 || _globals->_saveData->_data[svCinemaCurtainCond2] != 1)
				_objectsMan->sceneControl2("IM28", "IM28", "ANIM28", "IM28", 1, false);
			else
				_objectsMan->sceneControl2("IM28A", "IM28", "ANIM28", "IM28", 1, false);
			break;

		case 29:
			_linesMan->setMaxLineIdx(50);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM29", "IM29", "ANIM29", "IM29", 1, true);
			break;

		case 30:
			// Shooting
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM30", "IM30", "ANIM30", "IM30", 24, false);
			break;

		case 31:
			// Shooting target
			_objectsMan->sceneControl("IM31", "IM31", "ANIM31", "IM31", 10, true);
			break;

		case 32:
			_linesMan->setMaxLineIdx(20);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM32", "IM32", "ANIM32", "IM32", 2, true);
			break;

		case 33:
			_objectsMan->sceneControl("IM33", "IM33", "ANIM33", "IM33", 8, false);
			break;

		case 34:
			// In the airport, before the flight cut-scene
			_objectsMan->sceneControl("IM34", "IM34", "ANIM34", "IM34", 2, false);
			break;

		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41: {
			_linesMan->setMaxLineIdx(40);
			_globals->_characterMaxPosY = 435;
			_globals->_disableInventFl = false;
			_objectsMan->_forestFl = true;
			Common::String im = Common::String::format("IM%d", _globals->_exitId);
			_soundMan->playSound(13);
			if (_objectsMan->_forestSprite == NULL) {
				_objectsMan->_forestSprite = _objectsMan->loadSprite("HOPDEG.SPR");
				_soundMan->loadSample(1, "SOUND41.WAV");
			}
			_objectsMan->sceneControl2(im, im, "BANDIT", im, 13, false);
			if (_globals->_exitId < 35 || _globals->_exitId > 49) {
				_objectsMan->_forestSprite = _globals->freeMemory(_objectsMan->_forestSprite);
				_objectsMan->_forestFl = false;
				_soundMan->removeSample(1);
			}
			break;
			}

		case 50:
			// Flight cut scene
			playPlaneCutscene();
			_globals->_exitId = 51;
			break;

		case 51:
			_linesMan->setMaxLineIdx(10);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM51", "IM51", "ANIM51", "IM51", 14, true);
			break;

		case 52:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM52", "IM52", "ANIM52", "IM52", 14, true);
			break;

		case 54:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM54", "IM54", "ANIM54", "IM54", 14, true);
			break;

		case 55:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 460;
			_objectsMan->sceneControl2("IM55", "IM55", "ANIM55", "IM55", 14, false);
			break;

		case 56:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM56", "IM56", "ANIM56", "IM56", 14, false);
			break;

		case 57:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM57", "IM57", "ANIM57", "IM57", 14, true);
			break;

		case 58:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM58", "IM58", "ANIM58", "IM58", 14, false);
			break;

		case 59:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM59", "IM59", "ANIM59", "IM59", 21, false);
			break;

		case 60:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM60", "IM60", "ANIM60", "IM60", 21, false);
			break;

		case 61:
			if (_globals->_saveData->_data[svBaseElevatorCond1] == 1 && !_globals->_saveData->_data[svBaseFireFl])
				handleConflagration();
			_objectsMan->sceneControl("IM61", "IM61", "ANIM61", "IM61", 21, false);
			break;

		case 62:
			_linesMan->setMaxLineIdx(8);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM62", "IM62", NULL, "IM62", 21, false);
			break;

		case 63:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM63", "IM63", "ANIM63", "IM63", 21, false);
			break;

		case 64:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM64", "IM64", "ANIM64", "IM64", 21, true);
			break;

		case 65:
			_linesMan->setMaxLineIdx(30);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM65", "IM65", "ANIM65", "IM65", 21, false);
			break;

		case 66:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM66", "IM66", "ANIM66", "IM66", 21, false);
			break;

		case 67:
			_linesMan->setMaxLineIdx(8);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM67", "IM67", NULL, "IM67", 21, false);
			break;

		case 68:
			_linesMan->setMaxLineIdx(8);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM68", "IM68", "ANIM68", "IM68", 21, true);
			break;

		case 69:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM69", "IM69", "ANIM69", "IM69", 21, false);
			break;

		case 70:
			_linesMan->setMaxLineIdx(8);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM70", "IM70", NULL, "IM70", 21, false);
			break;

		case 71:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 445;
			_objectsMan->sceneControl2("IM71", "IM71", "ANIM71", "IM71", 21, false);
			break;

		case 73:
			_linesMan->setMaxLineIdx(15);
			_globals->_characterMaxPosY = 445;
			if (_globals->_saveData->_data[svSecondElevatorAvailableFl] == 1)
				_objectsMan->sceneControl2("IM73", "IM73A", "ANIM73", "IM73", 21, true);
			else
				_objectsMan->sceneControl2("IM73", "IM73", "ANIM73", "IM73", 21, true);
			break;

		case 75:
			playSubmarineCutscene();
			break;

		case 77:
			handleOceanMaze(77, "OCEAN01", DIR_RIGHT, 0, 84, 0, 0, 25);
			break;

		case 78:
			handleOceanMaze(78, "OCEAN02", DIR_UP, 0, 91, 84, 0, 25);
			break;

		case 79:
			handleOceanMaze(79, "OCEAN03", DIR_LEFT, 87, 0, 0, 83, 25);
			break;

		case 80:
			handleOceanMaze(80, "OCEAN04", DIR_UP, 86, 88, 0, 81, 25);
			break;

		case 81:
			handleOceanMaze(81, "OCEAN05", DIR_UP, 91, 82, 80, 85, 25);
			break;

		case 82:
			handleOceanMaze(82, "OCEAN06", DIR_LEFT, 81, 0, 88, 0, 25);
			break;

		case 83:
			handleOceanMaze(83, "OCEAN07", DIR_UP, 89, 0, 79, 88, 25);
			break;

		case 84:
			handleOceanMaze(84, "OCEAN08", DIR_UP, 77, 0, 0, 78, 25);
			break;

		case 85:
			handleOceanMaze(85, "OCEAN09", DIR_UP, 0, 0, 81, 0, 25);
			break;

		case 86:
			handleOceanMaze(86, "OCEAN10", DIR_UP, 0, 80, 0, 91, 25);
			break;

		case 87:
			handleOceanMaze(87, "OCEAN11", DIR_RIGHT, 0, 79, 90, 0, 25);
			break;

		case 88:
			handleOceanMaze(88, "OCEAN12", DIR_UP, 80, 0, 83, 82, 25);
			break;

		case 89:
			handleOceanMaze(89, "OCEAN13", DIR_RIGHT, 0, 83, 0, 0, 25);
			break;

		case 90:
			playUnderwaterBaseCutscene();
			break;

		case 91:
			handleOceanMaze(91, "OCEAN15", DIR_RIGHT, 78, 81, 86, 0, 25);
			break;

		case 93:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 445;
			if (_globals->_saveData->_data[svEscapeLeftJailFl]) {
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
					_objectsMan->sceneControl2("IM93", "IM93C", "ANIM93", "IM93", 29, true);
				else
					_objectsMan->sceneControl2("IM93", "IM93C", "ANIM93", "IM93", 26, true);
			} else {
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
					_objectsMan->sceneControl2("IM93", "IM93", "ANIM93", "IM93", 29, true);
				else
					_objectsMan->sceneControl2("IM93", "IM93", "ANIM93", "IM93", 26, true);
			}
			break;

		case 94:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 440;
			_objectsMan->sceneControl2("IM94", "IM94", "ANIM94", "IM94", 19, true);
			break;

		case 95:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM95", "IM95", "ANIM95", "IM95", 19, false);
			break;

		case 96:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM96", "IM96", "ANIM96", "IM96", 19, false);
			break;

		case 97:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM97", "IM97", "ANIM97", "IM97", 19, false);
			if (_globals->_exitId == 18) {
				_globals->_eventMode = EVENTMODE_IGNORE;
				_soundMan->stopSound();
				_graphicsMan->clearScreen();
				_graphicsMan->clearPalette();
				_soundMan->playSound(6);
				_animMan->playAnim("PURG1A.ANM", "PURG1.ANM", 12, 18, 50);
				_graphicsMan->fadeOutShort();
				_globals->_eventMode = EVENTMODE_DEFAULT;
			}
			break;

		case 98:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM98", "IM98", "ANIM98", "IM98", 19, true);
			break;

		case 99:
			_linesMan->setMaxLineIdx(5);
			_globals->_characterMaxPosY = 435;
			_objectsMan->sceneControl2("IM99", "IM99", "ANIM99", "IM99", 19, true);
			break;

		case 100:
			playEnding();
			break;

		case 111:
			_objectsMan->sceneControl("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsMan->sceneControl("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_screenId = 113;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_prevScreenId;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_globals->_exitId = 0;
			_computer->showComputer(COMPUTER_HOPKINS);
			_graphicsMan->clearScreen();
			_graphicsMan->updateScreen();
			memset(_graphicsMan->_frontBuffer, 0, 307200);
			memset(_graphicsMan->_backBuffer, 0, 307200);
			_graphicsMan->clearPalette();
			_graphicsMan->resetDirtyRects();
			break;

		case 114:
			_globals->_exitId = 0;
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_screenId = 114;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_prevScreenId;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_computer->showComputer(COMPUTER_SAMANTHA);
			_graphicsMan->clearScreen();
			break;

		case 115:
			_globals->_prevScreenId = _globals->_screenId;
			_globals->_screenId = 115;
			_globals->_saveData->_data[svLastPrevScreenId] = _globals->_prevScreenId;
			_globals->_saveData->_data[svLastScreenId] = _globals->_screenId;
			_globals->_exitId = 0;
			_computer->showComputer(COMPUTER_PUBLIC);
			_graphicsMan->clearScreen();
			break;

		case 150:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;
			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR1A.ANM", "JOUR1A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 151:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;
			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR3A.ANM", "JOUR3A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 152:
			_soundMan->playSound(16);
			_globals->_eventMode = EVENTMODE_IGNORE;
			_graphicsMan->clearScreen();
			_graphicsMan->clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsMan->_fadingFl = true;
			_animMan->playAnim("JOUR4A.ANM", "JOUR4A.ANM", 12, 12, 2000);
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_globals->_exitId = 300;
			break;

		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:
			_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
			_globals->_eventMode = EVENTMODE_IGNORE;
			_soundMan->stopSound();
			_soundMan->playSound(23);
			_globals->_exitId = handleBaseMap();	// Handles the base map (non-Windows)
			//_globals->_exitId = WBASE();	// Handles the 3D Doom level (Windows)
			_soundMan->stopSound();
			_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");
			_globals->_characterType = CHARACTER_HOPKINS;
			_globals->_eventMode = EVENTMODE_DEFAULT;
			_graphicsMan->_lineNbr = SCREEN_WIDTH;
			break;

		default:
			break;
		}
	}
	_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
	restoreSystem();
	return true;
}

int HopkinsEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void HopkinsEngine::initializeSystem() {
	// Set graphics mode
	_graphicsMan->setGraphicalMode(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Synchronize the sound settings from ScummVM
	_soundMan->syncSoundSettings();

	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "SYSTEM");
	SearchMan.addSubDirectoryMatching(gameDataDir, "LINK");
	SearchMan.addSubDirectoryMatching(gameDataDir, "BUFFER");
	SearchMan.addSubDirectoryMatching(gameDataDir, "ANIM");
	SearchMan.addSubDirectoryMatching(gameDataDir, "ANM");
	SearchMan.addSubDirectoryMatching(gameDataDir, "BASE");
	SearchMan.addSubDirectoryMatching(gameDataDir, "MUSIC");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SEQ");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SAVE");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SOUND");
	SearchMan.addSubDirectoryMatching(gameDataDir, "SVGA");
	SearchMan.addSubDirectoryMatching(gameDataDir, "VOICE");
	SearchMan.addSubDirectoryMatching(gameDataDir, "TSVGA");

	_globals->clearAll();

	_events->initMouseData();
	_fontMan->initData();

	_dialog->loadIcons();
	_objectsMan->_headSprites = _fileIO->loadFile("TETE.SPR");

	_events->setMouseOn();
	_events->_mouseFl = false;

	_globals->loadCharacterData();

	_events->_mouseOffset.x = 0;
	_events->_mouseOffset.y = 0;
}

/**
 * Play the intro of the game
 */
void HopkinsEngine::playIntro() {
	// Win95 EN demo doesn't include the intro
	if ((getLanguage() == Common::EN_ANY) && (getPlatform() == Common::kPlatformWindows) && (getIsDemo()))
		return;

	byte paletteData[PALETTE_EXT_BLOCK_SIZE];
	byte paletteData2[PALETTE_EXT_BLOCK_SIZE];

	memset(&paletteData, 0, PALETTE_EXT_BLOCK_SIZE);
	_events->refreshScreenAndEvents();
	_events->_mouseFl = false;
	_globals->_eventMode = EVENTMODE_IGNORE;
	_events->refreshScreenAndEvents();
	_soundMan->playSound(16);
	_animMan->setClearAnimFlag();

	_animMan->playAnim("J1.ANM", "J1.ANM", 12, 12, 50);
	if (shouldQuit() || _events->_escKeyFl)
		return;
	_events->mouseOff();
	_soundMan->mixVoice(1, 3);
	_animMan->playAnim("J2.ANM", "J2.ANM", 12, 12, 50);

	if (shouldQuit() || _events->_escKeyFl)
		return;

	_events->mouseOff();
	_soundMan->mixVoice(2, 3);
	_animMan->playAnim("J3.ANM", "J3.ANM", 12, 12, 50);

	if (shouldQuit() || _events->_escKeyFl)
		return;

	_events->mouseOff();
	_soundMan->mixVoice(3, 3);
	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();
	_graphicsMan->updateScreen();
	_soundMan->playSound(11);
	_graphicsMan->loadImage("intro1");
	_graphicsMan->scrollScreen(0);
	_graphicsMan->_scrollOffset = 0;
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);

	_events->delay(500);

	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->fadeInLong();
	if (_graphicsMan->_largeScreenFl) {
		_graphicsMan->_scrollStatus = 2;
		_graphicsMan->_scrollPosX = 0;

		bool loopCond = false;
		do {
			_graphicsMan->_scrollPosX += 2;
			if (_graphicsMan->_scrollPosX > (SCREEN_WIDTH - 2)) {
				_graphicsMan->_scrollPosX = SCREEN_WIDTH;
				loopCond = true;
			}

			if (_events->getMouseX() < _graphicsMan->_scrollPosX + 10)
				_events->setMouseXY(_events->_mousePos.x + 4, _events->getMouseY());
			_events->refreshScreenAndEvents();
		} while (!shouldQuit() && !loopCond && _graphicsMan->_scrollPosX != SCREEN_WIDTH);

		_events->refreshScreenAndEvents();
		_graphicsMan->_scrollStatus = 0;

		if (shouldQuit())
			return;
	}

	_soundMan->mixVoice(4, 3);
	_graphicsMan->fadeOutLong();
	_graphicsMan->_scrollStatus = 0;
	_graphicsMan->loadImage("intro2");
	_graphicsMan->scrollScreen(0);
	_animMan->loadAnim("INTRO2");
	_graphicsMan->displayAllBob();
	_soundMan->playSound(23);
	_objectsMan->stopBobAnimation(3);
	_objectsMan->stopBobAnimation(5);
	_graphicsMan->_scrollOffset = 0;
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);

	for (int i = 0; i <= 4; i++)
		_events->refreshScreenAndEvents();

	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->fadeInLong();
	for (uint i = 0; i < 200 / _globals->_speed; ++i)
		_events->refreshScreenAndEvents();

	_objectsMan->setBobAnimation(3);
	_soundMan->mixVoice(5, 3);
	_objectsMan->stopBobAnimation(3);
	_events->refreshScreenAndEvents();
	memcpy(&paletteData2, _graphicsMan->_palette, 796);

	_graphicsMan->setPaletteVGA256WithRefresh(paletteData, _graphicsMan->_frontBuffer);
	_graphicsMan->endDisplayBob();

	if (shouldQuit() || _events->_escKeyFl)
		return;

	_soundMan->_specialSoundNum = 5;
	_graphicsMan->_fadingFl = true;
	_animMan->playAnim("ELEC.ANM", "ELEC.ANM", 10, 26, 200);
	_soundMan->_specialSoundNum = 0;

	if (shouldQuit() || _events->_escKeyFl)
		return;

	_graphicsMan->loadImage("intro2");
	_graphicsMan->scrollScreen(0);
	_animMan->loadAnim("INTRO2");
	_graphicsMan->displayAllBob();
	_soundMan->playSound(23);
	_objectsMan->stopBobAnimation(3);
	_objectsMan->stopBobAnimation(5);
	_objectsMan->stopBobAnimation(1);
	_graphicsMan->_scrollOffset = 0;
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);

	for (int i = 0; i <= 3; i++)
		_events->refreshScreenAndEvents();

	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->setPaletteVGA256WithRefresh(paletteData2, _graphicsMan->_frontBuffer);

	int introIndex = 0;
	while (!shouldQuit() && !_events->_escKeyFl) {
		if (introIndex == 12) {
			_objectsMan->setBobAnimation(3);
			_events->refreshScreenAndEvents();
			_soundMan->mixVoice(6, 3);
			_events->refreshScreenAndEvents();
			_objectsMan->stopBobAnimation(3);
		}

		Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsMan->_palette[0]);

		for (int i = 1, maxPalVal = 4 * introIndex; i <= PALETTE_BLOCK_SIZE; i++) {
			if (_graphicsMan->_palette[i] > maxPalVal)
				_graphicsMan->_palette[i] -= maxPalVal;
		}

		_graphicsMan->setPaletteVGA256WithRefresh(_graphicsMan->_palette, _graphicsMan->_frontBuffer);

		for (int i = 1; i < 2 * introIndex; i++)
			_events->refreshScreenAndEvents();

		_graphicsMan->setPaletteVGA256WithRefresh(paletteData2, _graphicsMan->_frontBuffer);

		for (int i = 1; i < 20 - introIndex; i++)
			_events->refreshScreenAndEvents();

		introIndex += 2;
		if (introIndex > 15) {
			_graphicsMan->setPaletteVGA256WithRefresh(paletteData, _graphicsMan->_frontBuffer);
			for (uint j = 1; j < 100 / _globals->_speed; ++j)
				_events->refreshScreenAndEvents();

			_objectsMan->setBobAnimation(3);
			_soundMan->mixVoice(7, 3);
			_objectsMan->stopBobAnimation(3);

			for (uint k = 1; k < 60 / _globals->_speed; ++k)
				_events->refreshScreenAndEvents();
			_objectsMan->setBobAnimation(5);
			for (uint l = 0; l < 20 / _globals->_speed; ++l)
				_events->refreshScreenAndEvents();

			Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsMan->_palette[0]);
			_graphicsMan->setPaletteVGA256WithRefresh(_graphicsMan->_palette, _graphicsMan->_frontBuffer);

			for (uint m = 0; m < 50 / _globals->_speed; ++m) {
				if (m == 30 / _globals->_speed) {
					_objectsMan->setBobAnimation(3);
					_soundMan->mixVoice(8, 3);
					_objectsMan->stopBobAnimation(3);
				}

				_events->refreshScreenAndEvents();
			}

			_graphicsMan->fadeOutLong();
			_graphicsMan->endDisplayBob();
			_soundMan->playSound(3);
			_soundMan->_specialSoundNum = 1;
			_animMan->setClearAnimFlag();
			_animMan->playAnim("INTRO1.ANM", "INTRO1.ANM", 10, 24, 18);
			_soundMan->_specialSoundNum = 0;
			if (shouldQuit() || _events->_escKeyFl)
				return;

			_animMan->playAnim("INTRO2.ANM", "INTRO2.ANM", 10, 24, 18);
			if (shouldQuit() || _events->_escKeyFl)
				return;

			_animMan->playAnim("INTRO3.ANM", "INTRO3.ANM", 10, 24, 200);
			if (shouldQuit() || _events->_escKeyFl)
				return;

			_graphicsMan->_fadingFl = true;
			_animMan->unsetClearAnimFlag();
			_animMan->playAnim("J4.ANM", "J4.ANM", 12, 12, 1000);
			break;
		}
	}

	_events->_escKeyFl = false;
}

/**
 * If in demo, displays a 'not available' screen and returns to the city map
 */
void HopkinsEngine::displayNotAvailable() {
	if (!getIsDemo())
		return;

	if (_globals->_language == LANG_FR)
		_graphicsMan->loadImage("ndfr");
	else
		_graphicsMan->loadImage("nduk");

	_graphicsMan->fadeInLong();
	if (_soundMan->_voiceOffFl)
		_events->delay(500);
	else
		_soundMan->mixVoice(628, 4);

	_graphicsMan->fadeOutLong();
	_globals->_exitId = 4;
}

void HopkinsEngine::handleNotAvailable(int nextScreen) {
	// Use the code of the Linux demo instead of the code of the Windows demo.
	// The behavior is somewhat better, and common code is easier to maintain.
	displayNotAvailable();
	_globals->_exitId = nextScreen;
}

void HopkinsEngine::displayEndDemo() {
	_soundMan->playSound(28);
	if (_globals->_language == LANG_FR)
		_graphicsMan->loadImage("endfr");
	else
		_graphicsMan->loadImage("enduk");

	_graphicsMan->fadeInLong();
	_events->delay(1500);
	_graphicsMan->fadeOutLong();
	_globals->_exitId = 0;
}

void HopkinsEngine::bombExplosion() {
	_graphicsMan->_lineNbr = SCREEN_WIDTH;
	_graphicsMan->setScreenWidth(SCREEN_WIDTH);
	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();

	_globals->_eventMode = EVENTMODE_IGNORE;
	_soundMan->_specialSoundNum = 199;
	_graphicsMan->_fadingFl = true;
	_animMan->playAnim("BOMBE2A.ANM", "BOMBE2.ANM", 50, 14, 500);
	_soundMan->_specialSoundNum = 0;
	_graphicsMan->loadImage("IM15");
	_animMan->loadAnim("ANIM15");
	_graphicsMan->displayAllBob();
	_objectsMan->stopBobAnimation(7);

	for (int idx = 0; idx < 5; ++idx) {
		_events->refreshScreenAndEvents();
	}

	_graphicsMan->fadeInLong();
	_events->mouseOff();

	for (int idx = 0; idx < 20; ++idx) {
		_events->refreshScreenAndEvents();
	}

	_globals->_introSpeechOffFl = true;
	_talkMan->startStaticCharacterDialogue("vire.pe2");
	_globals->_introSpeechOffFl = false;
	_objectsMan->setBobAnimation(7);

	for (int idx = 0; idx < 100; ++idx) {
		_events->refreshScreenAndEvents();
	}

	_graphicsMan->fadeOutLong();
	_graphicsMan->endDisplayBob();
	_globals->_eventMode = EVENTMODE_DEFAULT;
	_globals->_exitId = 151;
}

void HopkinsEngine::restoreSystem() {
	_events->refreshEvents();
}

void HopkinsEngine::endLinuxDemo() {
	_globals->_linuxEndDemoFl = true;
	_graphicsMan->resetDirtyRects();
	_objectsMan->_forestFl = false;
	_events->_breakoutFl = false;
	_globals->_disableInventFl = true;
	_graphicsMan->loadImage("BOX");
	_soundMan->playSound(28);
	_graphicsMan->fadeInLong();
	_events->mouseOn();
	_events->changeMouseCursor(0);
	_events->_mouseCursorId = 0;
	_events->_mouseSpriteId = 0;

	bool mouseClicked = false;

	do {
		_events->refreshScreenAndEvents();

		if (_events->getMouseButton() == 1)
			mouseClicked = true;
	} while (!mouseClicked && !shouldQuit());

	// Original tried to open a web browser link here. Since ScummVM doesn't support
	// that, it's being skipped in favor of simply exiting

	_graphicsMan->fadeOutLong();
}

void HopkinsEngine::handleConflagration() {
	_globals->_disableInventFl = true;
	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->loadImage("IM71");
	_animMan->loadAnim("ANIM71");
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);
	_graphicsMan->displayAllBob();

	for (int cpt = 0; cpt <= 4; cpt++)
		_events->refreshScreenAndEvents();

	_graphicsMan->fadeInLong();
	_globals->_eventMode = EVENTMODE_IGNORE;

	for (int cpt = 0; cpt <= 249; cpt++)
		_events->refreshScreenAndEvents();

	_globals->_introSpeechOffFl = true;
	_talkMan->startAnimatedCharacterDialogue("SVGARD1.pe2");
	_globals->_introSpeechOffFl = false;

	for (int cpt = 0; cpt <= 49; cpt++)
		_events->refreshScreenAndEvents();

	_graphicsMan->fadeOutLong();
	_graphicsMan->endDisplayBob();
	_globals->_saveData->_data[svBaseFireFl] = 1;
	_globals->_disableInventFl = false;
}

void HopkinsEngine::playSubmarineCutscene() {
	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->_lineNbr = SCREEN_WIDTH;
	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();
	_soundMan->playSound(25);
	_animMan->setClearAnimFlag();
	_animMan->playAnim("BASE00A.ANM", "BASE00.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE05A.ANM", "BASE05.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE10A.ANM", "BASE10.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE20A.ANM", "BASE20.ANM", 10, 18, 18);
	// CHECKME: The original code was doing the opposite test, which was a bug.
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE30A.ANM", "BASE30.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE40A.ANM", "BASE40.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("BASE50A.ANM", "BASE50.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("OC00A.ANM", "OC00.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("OC05A.ANM", "OC05.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("OC10A.ANM", "OC10.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("OC20A.ANM", "OC20.ANM", 10, 18, 18);
	if (!_events->_escKeyFl) {
		_graphicsMan->_fadingFl = true;
		_animMan->playAnim("OC30A.ANM", "OC30.ANM", 10, 18, 18);
	}

	_events->_escKeyFl = false;
	_animMan->unsetClearAnimFlag();
	_globals->_exitId = 85;
}

void HopkinsEngine::playUnderwaterBaseCutscene() {
	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();
	_soundMan->playSound(26);
	_globals->_eventMode = EVENTMODE_IGNORE;
	_globals->_disableInventFl = true;
	_graphicsMan->_fadingFl = true;
	_animMan->playSequence("abase.seq", 50, 15, 50, false, false, true);
	_graphicsMan->loadImage("IM92");
	_animMan->loadAnim("ANIM92");
	_graphicsMan->displayAllBob();
	_objectsMan->loadLinkFile("IM92");
/*
	for (int cpt = 0; cpt <= 4 && !shouldQuit(); cpt++)
		_eventsManager->refreshScreenAndEvents();
*/
	_graphicsMan->fadeInLong();
	_objectsMan->enableHidingBehavior();

	do {
		_events->refreshScreenAndEvents();
	} while (!shouldQuit() && _objectsMan->getBobAnimDataIdx(8) != 22);

	if (!shouldQuit()) {
		_graphicsMan->fadeOutLong();
		_graphicsMan->endDisplayBob();
		_objectsMan->resetHidingItems();
		_globals->_disableInventFl = false;
		_globals->_exitId = 93;
		_globals->_eventMode = EVENTMODE_DEFAULT;
	}
}

void HopkinsEngine::playEnding() {
	_globals->_characterSpriteBuf = _globals->freeMemory(_globals->_characterSpriteBuf);
	_dialog->disableInvent();
	_globals->_disableInventFl = true;
	_graphicsMan->_scrollOffset = 0;
	_globals->_cityMapEnabledFl = false;
	_globals->_eventMode = EVENTMODE_IGNORE;
	_soundMan->playSound(26);
	_linesMan->_route = NULL;
	_globals->_freezeCharacterFl = true;
	_globals->_exitId = 0;
	_soundMan->loadSample(1, "SOUND90.WAV");
	_graphicsMan->loadImage("IM100");
	_animMan->loadAnim("ANIM100");
	_graphicsMan->displayAllBob();
	_events->mouseOn();
	_objectsMan->stopBobAnimation(7);
	_objectsMan->stopBobAnimation(8);
	_objectsMan->stopBobAnimation(9);
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);
	_events->changeMouseCursor(0);

	for (int cpt = 0; cpt <= 4; cpt++)
		_events->refreshScreenAndEvents();

	_graphicsMan->fadeInLong();
	_globals->_eventMode = EVENTMODE_IGNORE;

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(6) != 54);

	_globals->_introSpeechOffFl = true;
	_talkMan->startAnimatedCharacterDialogue("GM4.PE2");
	_globals->_disableInventFl = true;
	_objectsMan->stopBobAnimation(6);
	_objectsMan->stopBobAnimation(10);
	_objectsMan->setBobAnimation(9);
	_objectsMan->setBobAnimation(7);

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(7) != 54);

	_soundMan->playSample(1);

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(7) != 65);

	_globals->_introSpeechOffFl = true;
	_talkMan->startAnimatedCharacterDialogue("DUELB4.PE2");
	_events->mouseOff();
	_globals->_disableInventFl = true;

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(7) != 72);

	_globals->_introSpeechOffFl = true;
	_talkMan->startAnimatedCharacterDialogue("DUELH1.PE2");

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(7) != 81);

	_globals->_introSpeechOffFl = true;
	_talkMan->startAnimatedCharacterDialogue("DUELB5.PE2");

	do {
		_events->refreshScreenAndEvents();
	} while (_objectsMan->getBobAnimDataIdx(7) != 120);

	_objectsMan->stopBobAnimation(7);
	if (_globals->_saveData->_data[svGameWonFl] == 1) {
		_soundMan->_specialSoundNum = 200;
		_soundMan->_skipRefreshFl = true;
		_graphicsMan->_fadingFl = true;
		_animMan->playAnim("BERM.ANM", "BERM.ANM", 100, 24, 300);
		_graphicsMan->endDisplayBob();
		_soundMan->removeSample(1);
		_graphicsMan->loadImage("PLAN3");
		_graphicsMan->fadeInLong();

		_events->_rateCounter = 0;
		if (!_events->_escKeyFl) {
			do {
				_events->refreshEvents();
			} while (_events->_rateCounter < 2000 / _globals->_speed && !_events->_escKeyFl);
		}
		_events->_escKeyFl = false;
		_graphicsMan->fadeOutLong();
		_globals->_eventMode = EVENTMODE_IGNORE;
		_soundMan->_specialSoundNum = 0;
		_graphicsMan->_fadingFl = true;
		_animMan->playAnim("JOUR2A.anm", "JOUR2A.anm", 12, 12, 1000);
		_soundMan->playSound(11);
		_graphicsMan->clearScreen();
		_graphicsMan->clearPalette();
		_animMan->playAnim("FF1a.anm", "FF1.anm", 18, 18, 9);
		_animMan->playAnim("FF1a.anm", "FF1.anm", 9, 18, 9);
		_animMan->playAnim("FF1a.anm", "FF1.anm", 9, 18, 18);
		_animMan->playAnim("FF1a.anm", "FF1.anm", 9, 18, 9);
		_animMan->playAnim("FF2a.anm", "FF2.anm", 24, 24, 100);
		_events->mouseOff();
		displayCredits();
		_globals->_eventMode = EVENTMODE_DEFAULT;
		_globals->_exitId = 300;
		_dialog->enableInvent();
		_globals->_disableInventFl = false;
	} else {
		_soundMan->_specialSoundNum = 200;
		_soundMan->_skipRefreshFl = true;
		_animMan->playAnim2("BERM.ANM", "BERM.ANM", 100, 24, 300);
		_objectsMan->stopBobAnimation(7);
		_objectsMan->setBobAnimation(8);
		_globals->_introSpeechOffFl = true;
		_talkMan->startAnimatedCharacterDialogue("GM5.PE2");
		_globals->_disableInventFl = true;

		do {
			_events->refreshScreenAndEvents();
		} while (_objectsMan->getBobAnimDataIdx(8) != 5);

		_soundMan->directPlayWav("SOUND41.WAV");

		do {
			_events->refreshScreenAndEvents();
		} while (_objectsMan->getBobAnimDataIdx(8) != 21);

		_graphicsMan->fadeOutLong();
		_graphicsMan->endDisplayBob();
		_soundMan->removeSample(1);
		_soundMan->playSound(16);
		_globals->_eventMode = EVENTMODE_IGNORE;
		_soundMan->_specialSoundNum = 0;
		_dialog->enableInvent();
		_globals->_disableInventFl = false;
		_animMan->playAnim("JOUR4A.ANM", "JOUR4A.ANM", 12, 12, 1000);
		_globals->_eventMode = EVENTMODE_DEFAULT;
		_globals->_exitId = 300;
	}
	_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");
	_globals->_characterType = CHARACTER_HOPKINS;
	_globals->_eventMode = EVENTMODE_DEFAULT;
}

void HopkinsEngine::playPlaneCutscene() {
	_soundMan->playSound(28);
	_globals->_eventMode = EVENTMODE_IGNORE;
	_graphicsMan->clearScreen();
	_graphicsMan->clearPalette();

	_animMan->unsetClearAnimFlag();
	_animMan->playAnim("AEROP00A.ANM", "AEROP00.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("SEROP10A.ANM", "SEROP10A.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP20A.ANM", "AEROP20.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP30A.ANM", "AEROP30.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP40A.ANM", "AEROP40.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP50A.ANM", "AEROP50.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP60A.ANM", "AEROP60.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("AEROP70A.ANM", "AEROP70.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS00A.ANM", "TRANS00.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS10A.ANM", "TRANS10.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS15A.ANM", "TRANS15.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS20A.ANM", "TRANS20.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS30A.ANM", "TRANS30.ANM", 10, 18, 18);
	if (!_events->_escKeyFl)
		_animMan->playAnim("TRANS40A.ANM", "TRANS40.ANM", 10, 18, 18);
	if (!_events->_escKeyFl) {
		_graphicsMan->_fadingFl = true;
		_animMan->playAnim("PARA00A.ANM", "PARA00.ANM", 9, 9, 9);
	} else {
		_graphicsMan->fadeOutShort();
	}

	_events->_escKeyFl = false;
	_animMan->unsetClearAnimFlag();
}

void HopkinsEngine::loadBaseMap() {
	Common::String filename	= Common::String::format("%s.PCX", "PBASE");
	Common::File f;

	if (f.exists(filename)) {
		// PBASE file exists, so go ahead and load it
		_graphicsMan->loadImage("PBASE");
	} else {
		// PBASE file doesn't exist, so draw a substitute screen
		drawBaseMap();
	}
}

void HopkinsEngine::drawBaseMap() {
	memset(_graphicsMan->_backBuffer, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	// List of rectangle areas to draw for exit points
	const int rects[] = {
		181, 66, 181 + 16, 66 + 22,
		353, 116, 353 + 22, 116 + 16,
		483, 250, 483 + 20, 250 + 25,
		471, 326, 471 + 27, 326 + 20,
		162, 365, 162 + 21, 365 + 23,
		106, 267, 106 + 20, 267 + 26
	};

	// Loop through displaying
	const int *rectP = &rects[0];
	for (int rectIndex = 0; rectIndex < 6; ++rectIndex, rectP += 4) {
		Common::Rect r(rectP[0], rectP[1], rectP[2], rectP[3]);

		for (int yp = r.top; yp <= r.bottom; ++yp) {
			byte *pDest = _graphicsMan->_backBuffer + yp * SCREEN_WIDTH + r.left;
			Common::fill(pDest, pDest + r.width(), 0xff);
		}
	}

	// Copy the calculated screen
	memcpy(_graphicsMan->_frontBuffer, _graphicsMan->_backBuffer, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	// Write some explanatory text
	_fontMan->displayText(40, 200, "ScummVM base map - select a square for different rooms", 255);
}

int HopkinsEngine::handleBaseMap() {
	_globals->_disableInventFl = true;

	// Load the map image
	loadBaseMap();

	// Set needed colors
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);
	_events->changeMouseCursor(0);
	_graphicsMan->fadeInLong();
	bool loopCond = false;
	int zone;
	do {
		if (shouldQuit())
			return 0;

		int mouseButton = _events->getMouseButton();
		int posX = _events->getMouseX();
		int posY = _events->getMouseY();
		zone = 0;
		if ((posX - 181 <= 16) && (posY - 66 <= 22) &&
		    (posX - 181 >= 0) && (posY - 66 >= 0))
			zone = 1;
		if ((posX - 353 <= 22) && (posY - 116 <= 19) &&
		    (posX - 353 >= 0) && (posY - 116 >= 0))
			zone = 2;
		if ((posX - 483 <= 20) && (posY - 250 <= 25) &&
		    (posX - 483 >= 0) && (posY - 250 >= 0))
			zone = 3;
		if ((posX - 471 <= 27) && (posY - 326 <= 20) &&
		    (posX - 471 >= 0) && (posY - 326 >= 0))
			zone = 4;
		if ((posX - 162 <= 21) && (posY - 365 <= 23) &&
		    (posX - 162 >= 0) && (posY - 365 >= 0))
			zone = 5;
		if ((posX - 106 <= 20) && (posY - 267 <= 26) &&
		    (posX - 106 >= 0) && (posY - 267 >= 0))
			zone = 6;
		if (zone) {
			_events->changeMouseCursor(4);
			_globals->_baseMapColor += 25;
			if (_globals->_baseMapColor > 100)
				_globals->_baseMapColor = 0;
			_graphicsMan->setColorPercentage2(251, _globals->_baseMapColor, _globals->_baseMapColor, _globals->_baseMapColor);
		} else {
			_events->changeMouseCursor(0);
			_graphicsMan->setColorPercentage2(251, 100, 100, 100);
		}
		_events->refreshScreenAndEvents();
		if ((mouseButton == 1) && zone)
			loopCond = true;
	} while (!loopCond);

	_globals->_disableInventFl = false;
	_graphicsMan->fadeOutLong();

	int result;
	switch (zone) {
	case 1:
		result = 94;
		break;
	case 2:
		result = 95;
		break;
	case 3:
		result = 96;
		break;
	case 4:
		result = 97;
		break;
	case 5:
		result = 98;
		break;
	case 6:
		result = 99;
		break;
	default:
		result = 0;
		break;
	}
	return result;
}

void HopkinsEngine::loadCredits() {
	_globals->_creditsPosY = 440;
	_globals->_creditsStep = 45;
	byte *bufPtr;
	Common::String filename;
	switch (_globals->_language) {
	case LANG_EN:
		filename = "CREAN.TXT";
		break;
	case LANG_FR:
		filename = "CREFR.TXT";
		break;
	case LANG_SP:
		filename = "CREES.TXT";
		break;
	default:
		error("Unhandled language");
		break;
	}

	if (!_fileIO->fileExists(filename)) {
		_globals->_creditsLineNumb = 1;
		_globals->_creditsItem[0]._color = '1';
		_globals->_creditsItem[0]._actvFl = true;
		_globals->_creditsItem[0]._linePosY = _globals->_creditsPosY;
		strcpy((char *)_globals->_creditsItem[0]._line, "The End");
		_globals->_creditsItem[0]._lineSize = 7;
		return;
	}

	bufPtr = _fileIO->loadFile(filename);

	byte *curPtr = bufPtr;
	int idxLines = 0;
	bool loopCond = false;
	do {
		if (*curPtr == '%') {
			if (curPtr[1] == '%') {
				loopCond = true;
				break;
			}
			_globals->_creditsItem[idxLines]._color = curPtr[1];
			_globals->_creditsItem[idxLines]._actvFl = true;
			_globals->_creditsItem[idxLines]._linePosY = _globals->_creditsPosY + idxLines * _globals->_creditsStep;

			int idxBuf = 0;
			for(; idxBuf < 49; idxBuf++) {
				byte curChar = curPtr[idxBuf + 3];
				if (curChar == '%' || curChar == 10)
					break;
				_globals->_creditsItem[idxLines]._line[idxBuf] = curChar;
			}
			_globals->_creditsItem[idxLines]._line[idxBuf] = 0;
			_globals->_creditsItem[idxLines]._lineSize = idxBuf - 1;
			curPtr = curPtr + idxBuf + 2;
			++idxLines;
		} else {
			curPtr++;
		}
		_globals->_creditsLineNumb = idxLines;
	} while (!loopCond);

	_globals->freeMemory(bufPtr);
}

void HopkinsEngine::displayCredits(int startPosY, byte *buffer, char color) {
	byte *bufPtr = buffer;
	int strWidth = 0;
	byte curChar;
	for (;;) {
		curChar = *bufPtr++;
		if (!curChar)
			break;
		if (curChar > 31)
			strWidth += _objectsMan->getWidth(_fontMan->_font, curChar - 32);
	}
	int startPosX = 320 - strWidth / 2;
	int endPosX = strWidth + startPosX;
	int endPosY = startPosY + 12;
	if ((_globals->_creditsStartX == -1) && (_globals->_creditsEndX == -1) && (_globals->_creditsStartY == -1) && (_globals->_creditsEndY == -1)) {
		_globals->_creditsStartX = startPosX;
		_globals->_creditsEndX = endPosX;
		_globals->_creditsStartY = startPosY;
		_globals->_creditsEndY = endPosY;
	}

	_globals->_creditsStartX = MIN(_globals->_creditsStartX, startPosX);
	_globals->_creditsEndX = MAX(_globals->_creditsEndX, endPosX);
	_globals->_creditsStartY = MIN(_globals->_creditsStartY, startPosY);
	_globals->_creditsEndY = MAX(_globals->_creditsEndY, endPosY);

	Common::String message = Common::String((char *)buffer);
	_fontMan->displayText(startPosX, startPosY, message, color);
}

void HopkinsEngine::displayCredits() {
	loadCredits();
	_globals->_creditsPosY = 436;
	_graphicsMan->loadImage("GENERIC");
	_graphicsMan->fadeInLong();
	_soundMan->playSound(28);
	_events->_mouseFl = false;
	_globals->_eventMode = EVENTMODE_CREDITS;
	_globals->_creditsStartX = _globals->_creditsEndX = _globals->_creditsStartY = _globals->_creditsEndY = -1;
	int soundId = 28;

	do {
		for (int i = 0; i < _globals->_creditsLineNumb; ++i) {
			if (_globals->_creditsItem[i]._actvFl) {
				int nextY = _globals->_creditsPosY + i * _globals->_creditsStep;
				_globals->_creditsItem[i]._linePosY = nextY;

				if ((nextY >= 51) && (nextY <= 460)) {
					int col = 0;
					switch (_globals->_creditsItem[i]._color) {
					case '1':
						col = 163;
						break;
					case '2':
						col = 161;
						break;
					case '3':
						col = 162;
						break;
					default:
						warning("Unknown color, default to col #1");
						col = 163;
						break;
					}
					if (_globals->_creditsItem[i]._lineSize != -1)
						displayCredits(nextY, _globals->_creditsItem[i]._line, col);
				}
			}
		}
		--_globals->_creditsPosY;
		if (_globals->_creditsStartX != -1 || _globals->_creditsEndX != -1 || _globals->_creditsStartY != -1 || _globals->_creditsEndY != -1) {
			_events->refreshScreenAndEvents();
			_graphicsMan->copySurface(_graphicsMan->_backBuffer, 60, 50, 520, 430, _graphicsMan->_frontBuffer, 60, 50);
		} else {
			_events->refreshScreenAndEvents();
		}
		if (_globals->_creditsItem[_globals->_creditsLineNumb - 1]._linePosY <= 39) {
			_globals->_creditsPosY = 440;
			++soundId;
			if (soundId > 31)
				soundId = 28;
			_soundMan->playSound(soundId);
		}
		_globals->_creditsStartX = -1;
		_globals->_creditsEndX = -1;
		_globals->_creditsStartY = -1;
		_globals->_creditsEndY = -1;
	} while ((_events->getMouseButton() != 1) && (!shouldQuit()));
	_graphicsMan->fadeOutLong();
	_globals->_eventMode = EVENTMODE_IGNORE;
	_events->_mouseFl = true;
}

void HopkinsEngine::handleOceanMouseEvents() {
	_fontMan->hideText(9);
	if (_events->_mouseCursorId != 16)
		return;

	_events->getMouseX();
	if (_objectsMan->_zoneNum <= 0)
		return;

	int oldPosX = _events->getMouseX();
	int oldPosY = _events->getMouseY();
	bool displAnim = false;
	int oldX;
	switch (_objectsMan->_zoneNum) {
	case 1:
		switch (_globals->_oceanDirection) {
		case DIR_UP:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "27,26,25,24,23,22,21,20,19,18,-1,", 6, false);
			break;
		case DIR_RIGHT:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "9,10,11,12,13,14,15,16,17,18,-1,", 6, false);
			break;
		default:
			break;
		}

		_globals->_oceanDirection = DIR_LEFT;
		_globals->_exitId = 1;
		oldX = _objectsMan->getSpriteX(0);
		for (;;) {
			if (_globals->_speed == 1)
				oldX -= 2;
			else if (_globals->_speed == 2)
				oldX -= 4;
			else if (_globals->_speed == 3)
				oldX -= 6;
			_objectsMan->setSpriteX(0, oldX);
			setSubmarineSprites();
			_events->refreshScreenAndEvents();
			if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY) {
				displAnim = true;
				break;
			}

			if (oldX <= -100)
				break;
		}
		break;
	case 2:
		switch (_globals->_oceanDirection) {
		case DIR_UP:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "27,28,29,30,31,32,33,34,35,36,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "9,8,7,6,5,4,3,2,1,0,-1,", 6, false);
			break;
		case DIR_LEFT:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,-1,", 6, false);
			break;
		default:
			break;
		}
		_globals->_oceanDirection = DIR_RIGHT;
		_globals->_exitId = 2;
		oldX = _objectsMan->getSpriteX(0);
		for (;;) {
			if (_globals->_speed == 1)
				oldX += 2;
			else if (_globals->_speed == 2)
				oldX += 4;
			else if (_globals->_speed == 3)
				oldX += 6;
			_objectsMan->setSpriteX(0, oldX);
			setSubmarineSprites();
			_events->refreshScreenAndEvents();
			if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY) {
				displAnim = true;
				break;
			}
			if (oldX > 499)
				break;
		}
		break;
	case 3:
		switch (_globals->_oceanDirection) {
		case DIR_RIGHT:
			oldX = _objectsMan->getSpriteX(0);
			do {
				if (_globals->_speed == 1)
					oldX += 2;
				else if (_globals->_speed == 2)
					oldX += 4;
				else if (_globals->_speed == 3)
					oldX += 6;
				_objectsMan->setSpriteX(0, oldX);
				setSubmarineSprites();
				_events->refreshScreenAndEvents();
				if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX <= 235);
			if (!displAnim)
				_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "36,35,34,33,32,31,30,29,28,27,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,-1,", 6, false);
			break;
		case DIR_LEFT:
			oldX = _objectsMan->getSpriteX(0);
			do {
				if (_globals->_speed == 1)
					oldX -= 2;
				else if (_globals->_speed == 2)
					oldX -= 4;
				else if (_globals->_speed == 3)
					oldX -= 6;
				_objectsMan->setSpriteX(0, oldX);
				setSubmarineSprites();
				_events->refreshScreenAndEvents();
				if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX > 236);
			if (!displAnim)
				_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "18,19,20,21,22,23,24,25,26,27,-1,", 6, false);
			break;
		default:
			break;
		}
		_globals->_oceanDirection = DIR_UP;
		_globals->_exitId = 3;
		break;
	case 4:
		switch (_globals->_oceanDirection) {
		case DIR_UP:
			_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,-1,", 6, false);
			break;
		case DIR_RIGHT:
			oldX = _objectsMan->getSpriteX(0);
			do {
				if (_globals->_speed == 1)
					oldX += 2;
				else if (_globals->_speed == 2)
					oldX += 4;
				else if (_globals->_speed == 3)
					oldX += 6;
				_objectsMan->setSpriteX(0, oldX);
				setSubmarineSprites();
				_events->refreshScreenAndEvents();
				if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX <= 235);
			if (!displAnim)
				_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "0,1,2,3,4,5,6,7,8,9,-1,", 6, false);
			break;
		case DIR_LEFT:
			oldX = _objectsMan->getSpriteX(0);
			for (;;) {
				if (_globals->_speed == 1)
					oldX -= 2;
				else if (_globals->_speed == 2)
					oldX -= 4;
				else if (_globals->_speed == 3)
					oldX -= 6;
				_objectsMan->setSpriteX(0, oldX);
				setSubmarineSprites();
				_events->refreshScreenAndEvents();
				if (_events->getMouseButton() == 1 && oldPosX == _events->getMouseX() && _events->getMouseY() == oldPosY)
					break;

				if (oldX <= 236) {
					if (!displAnim)
						_objectsMan->showSpecialActionAnimationWithFlip(_globals->_characterSpriteBuf, "18,17,16,15,14,13,12,11,10,9,-1,", 6, false);
					break;
				}
			}
			break;
		default:
			break;
		}
		_globals->_oceanDirection = DIR_DOWN;
		_globals->_exitId = 4;
		break;
	default:
		break;
	}
}

void HopkinsEngine::setSubmarineSprites() {
	switch (_globals->_oceanDirection) {
	case DIR_UP:
		_objectsMan->setSpriteIndex(0, 27);
		break;
	case DIR_RIGHT:
		_objectsMan->setSpriteIndex(0, 0);
		break;
	case DIR_DOWN:
		_objectsMan->setSpriteIndex(0, 9);
		break;
	case DIR_LEFT:
		_objectsMan->setSpriteIndex(0, 18);
		break;
	default:
		break;
	}
}

void HopkinsEngine::handleOceanMaze(int16 curExitId, Common::String backgroundFilename, Directions defaultDirection, int16 exit1, int16 exit2, int16 exit3, int16 exit4, int16 soundId) {
	_globals->_cityMapEnabledFl = false;
	_graphicsMan->_noFadingFl = false;
	_globals->_freezeCharacterFl = false;
	_globals->_exitId = 0;
	_globals->_disableInventFl = true;
	_soundMan->playSound(soundId);
	_globals->_characterSpriteBuf = _fileIO->loadFile("VAISSEAU.SPR");
	if (backgroundFilename.size())
		_graphicsMan->loadImage(backgroundFilename);

	if (curExitId == 77)
		_objectsMan->loadLinkFile("IM77");
	else if (curExitId == 84)
		_objectsMan->loadLinkFile("IM84");
	else if (curExitId == 91)
		_objectsMan->loadLinkFile("IM91");
	else
		_objectsMan->loadLinkFile("ocean");

	if (!exit1)
		_linesMan->disableZone(1);
	if (!exit2)
		_linesMan->disableZone(2);
	if (!exit3)
		_linesMan->disableZone(3);
	if (!exit4)
		_linesMan->disableZone(4);

	if (_globals->_oceanDirection == DIR_NONE)
		_globals->_oceanDirection = defaultDirection;

	switch (_globals->_oceanDirection) {
	case DIR_UP:
		_objectsMan->_characterPos.x = 236;
		_objectsMan->_startSpriteIndex = 27;
		break;
	case DIR_RIGHT:
		_objectsMan->_characterPos.x = -20;
		_objectsMan->_startSpriteIndex = 0;
		break;
	case DIR_DOWN:
		_objectsMan->_characterPos.x = 236;
		_objectsMan->_startSpriteIndex = 9;
		break;
	case DIR_LEFT:
		_objectsMan->_characterPos.x = 415;
		_objectsMan->_startSpriteIndex = 18;
		break;
	default:
		break;
	}

	_objectsMan->addStaticSprite(_globals->_characterSpriteBuf, Common::Point(_objectsMan->_characterPos.x, 110), 0, _objectsMan->_startSpriteIndex, 0, false, 0, 0);
	_graphicsMan->setColorPercentage(252, 100, 100, 100);
	_graphicsMan->setColorPercentage(253, 100, 100, 100);
	_graphicsMan->setColorPercentage(251, 100, 100, 100);
	_graphicsMan->setColorPercentage(254, 0, 0, 0);
	_objectsMan->animateSprite(0);
	_linesMan->_route = NULL;
	_events->mouseOn();
	_events->changeMouseCursor(4);

	for (int cpt = 0; cpt <= 4; cpt++)
		_events->refreshScreenAndEvents();

	if (!_graphicsMan->_noFadingFl)
		_graphicsMan->fadeInLong();
	_graphicsMan->_noFadingFl = false;
	_globals->_eventMode = EVENTMODE_IGNORE;

	for (;;) {
		int mouseButton = _events->getMouseButton();
		if (mouseButton && mouseButton == 1)
			handleOceanMouseEvents();
		_linesMan->checkZone();
		setSubmarineSprites();

		_events->refreshScreenAndEvents();
		if (_globals->_exitId || shouldQuit())
			break;
	}

	if (_globals->_exitId == 1)
		_globals->_exitId = exit1;
	else if (_globals->_exitId == 2)
		_globals->_exitId = exit2;
	else if (_globals->_exitId == 3)
		_globals->_exitId = exit3;
	else if (_globals->_exitId == 4)
		_globals->_exitId = exit4;
	_graphicsMan->fadeOutLong();
	_objectsMan->removeSprite(0);
	_objectsMan->clearScreen();
	_globals->_characterSpriteBuf = _fileIO->loadFile("PERSO.SPR");
	_globals->_characterType = CHARACTER_HOPKINS;
}

void HopkinsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_soundMan->syncSoundSettings();
}

bool HopkinsEngine::displayAdultDisclaimer() {
	int buttonIndex;

	_graphicsMan->_minX = 0;
	_graphicsMan->_minY = 0;
	_graphicsMan->_maxX = SCREEN_WIDTH;
	_graphicsMan->_maxY = SCREEN_HEIGHT - 1;
	_events->_breakoutFl = false;
	_objectsMan->_forestFl = false;
	_globals->_disableInventFl = true;
	_globals->_exitId = 0;

	_graphicsMan->loadImage("ADULT");
	_graphicsMan->fadeInLong();
	_events->mouseOn();
	_events->changeMouseCursor(0);
	_events->_mouseCursorId = 0;
	_events->_mouseSpriteId = 0;

	do {
		int xp = _events->getMouseX();
		int yp = _events->getMouseY();

		buttonIndex = 0;
		if (xp >= 37 && xp <= 169 && yp >= 406 && yp <= 445)
			buttonIndex = 2;
		else if (xp >= 424 && xp <= 602 && yp >= 406 && yp <= 445)
			buttonIndex = 1;

		_events->refreshScreenAndEvents();
	} while (!shouldQuit() && (buttonIndex == 0 || _events->getMouseButton() != 1));

	_globals->_disableInventFl = false;
	_graphicsMan->fadeOutLong();

	if (buttonIndex != 2) {
		// Quit game
		return false;
	} else {
		// Continue
		_graphicsMan->_minX = 0;
		_graphicsMan->_maxY = 20;
		_graphicsMan->_maxX = SCREEN_WIDTH;
		_graphicsMan->_maxY = SCREEN_HEIGHT - 20;
		return true;
	}
}

bool HopkinsEngine::isUnderwaterSubScene() {
	return (_globals->_curRoomNum >= 77) && (_globals->_curRoomNum <= 89);
}

} // End of namespace Hopkins
