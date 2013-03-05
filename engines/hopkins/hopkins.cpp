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

HopkinsEngine *g_vm;

HopkinsEngine::HopkinsEngine(OSystem *syst, const HopkinsGameDescription *gameDesc) : Engine(syst),
		_gameDescription(gameDesc), _randomSource("Hopkins"), _animationManager() {
	g_vm = this;
	_debugger.setParent(this);
	_animationManager.setParent(this);
	_computerManager.setParent(this);
	_dialogsManager.setParent(this);
	_eventsManager.setParent(this);
	_fileManager.setParent(this);
	_fontManager.setParent(this);
	_globals.setParent(this);
	_graphicsManager.setParent(this);
	_linesManager.setParent(this);
	_menuManager.setParent(this);
	_objectsManager.setParent(this);
	_saveLoadManager.setParent(this);
	_scriptManager.setParent(this);
	_soundManager.setParent(this);
	_talkManager.setParent(this);
}

HopkinsEngine::~HopkinsEngine() {
}

Common::String HopkinsEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool HopkinsEngine::canLoadGameStateCurrently() {
	return !_globals._exitId && !_globals._cityMapEnabledFl && _eventsManager._mouseFl;
}

/**
 * Returns true if it is currently okay to save the game
 */
bool HopkinsEngine::canSaveGameStateCurrently() {
	return !_globals._exitId && !_globals._cityMapEnabledFl && _eventsManager._mouseFl;
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error HopkinsEngine::loadGameState(int slot) {
	return _saveLoadManager.loadGame(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error HopkinsEngine::saveGameState(int slot, const Common::String &desc) {
	return _saveLoadManager.saveGame(slot, desc);
}

Common::Error HopkinsEngine::run() {
	_saveLoadManager.initSaves();

	_globals.setConfig();
	_fileManager.initCensorship();
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
	_globals.loadObjects();
	_objectsManager.changeObject(14);
	_objectsManager.addObject(14);
	_objectsManager._helicopterFl = false;

	_globals.iRegul = 1;

	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();

	_graphicsManager.loadImage("H2");
	_graphicsManager.fadeInLong();

	if (!_eventsManager._escKeyFl)
		playIntro();

	_eventsManager._rateCounter = 0;
	_globals.iRegul = 1;
	_globals._speed = 1;

	for (int i = 1; i < 50; i++) {
		_graphicsManager.copySurface(_graphicsManager._vesaScreen, 0, 0, 640, 440, _graphicsManager._vesaBuffer, 0, 0);
		_eventsManager.refreshScreenAndEvents();
	}

	_globals.iRegul = 0;
	if (_eventsManager._rateCounter > 475)
		_globals._speed = 2;
	if (_eventsManager._rateCounter > 700)
		_globals._speed = 3;
	_graphicsManager.fadeOutLong();
	_globals.iRegul = 1;
	_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
	_globals._characterType = 0;
	_objectsManager._mapCarPosX = _objectsManager._mapCarPosY = 0;
	memset(_globals._saveData, 0, 2000);
	_globals._exitId = 0;

	if (getLanguage() != Common::PL_POL)
		if (!displayAdultDisclaimer())
			return Common::kNoError;

	for (;;) {
		if (_globals._exitId == 300)
			_globals._exitId = 0;

		if (!_globals._exitId) {
			_globals._exitId = _menuManager.menu();
			if (_globals._exitId == -1) {
				_globals.PERSO = _globals.freeMemory(_globals.PERSO);
				restoreSystem();
				return false;
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals._exitId) {
		case 1:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 2, true);
			break;

		case 3:
			if (!_globals._saveData->_data[svBankAttackAnimPlayedFl]) {
				_soundManager.playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsManager.loadImage("fond");
				else {
					if (_globals._language == LANG_FR)
						_graphicsManager.loadImage("fondfr");
					else if (_globals._language == LANG_EN)
						_graphicsManager.loadImage("fondan");
					else if (_globals._language == LANG_SP)
						_graphicsManager.loadImage("fondes");
				}
				_graphicsManager.fadeInLong();
				_eventsManager.delay(500);
				_graphicsManager.fadeOutLong();
				_globals.iRegul = 1;
				_soundManager._specialSoundNum = 2;
				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				if (!_globals._censorshipFl)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager._specialSoundNum = 0;
				_soundManager.removeSample(1);
				_soundManager.removeSample(2);
				_soundManager.removeSample(3);
				_soundManager.removeSample(4);
				_graphicsManager.fadeOutLong();
				_globals._saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			_globals._disableInventFl = true;
			_objectsManager.handleCityMap();
			_globals._disableInventFl = false;
			break;

		case 5:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 455;

			if (_globals._saveData->_data[svFreedHostageFl]) {
				if (_globals._saveData->_data[svFreedHostageFl] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			} else {
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			}
			break;

		case 6:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals._saveData->_data[svBombBoxOpenedFl])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_globals._characterMaxPosY = 440;
			_linesManager.setMaxLineIdx(20);
			if (_globals._saveData->_data[svBombDisarmedFl])
			  _objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			else
			  bombExplosion();
			break;

		case 10:
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_globals._characterMaxPosY = 450;
			_linesManager.setMaxLineIdx(20);
			if (_globals._saveData->_data[svBombDisarmedFl]) {
				if (_globals._language == LANG_FR)
					_graphicsManager.loadImage("ENDFR");
				else
					_graphicsManager.loadImage("ENDUK");
				_graphicsManager.fadeInLong();
				_eventsManager.mouseOn();
				do
					_eventsManager.refreshScreenAndEvents();
				while (_eventsManager.getMouseButton() != 1);
				_graphicsManager.fadeOutLong();
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
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 113;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_computerManager.showComputer(COMPUTER_HOPKINS);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.updateScreen();
			memset(_graphicsManager._vesaBuffer, 0, 307200);
			memset(_graphicsManager._vesaScreen, 0, 307200);
			_graphicsManager.clearPalette();
			_graphicsManager.resetVesaSegment();
			break;

		case 114:
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 114;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_globals._exitId = 0;
			_computerManager.showComputer(COMPUTER_SAMANTHA);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 115:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 115;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 150:
			_soundManager.playSound(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 151:
			_soundManager.playSound(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_graphicsManager.loadImage("njour3a");
			_graphicsManager.fadeInLong();
			_eventsManager.delay(5000);
			_graphicsManager.fadeOutLong();
			_globals._exitId = 300;
			_globals.iRegul = 0;
			break;

		case 152:
			_soundManager.playSound(28);
			_globals.iRegul = 4; // CHECKME!
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runLinuxDemo() {
	_globals.loadObjects();
	_objectsManager.changeObject(14);
	_objectsManager.addObject(14);
	_objectsManager._helicopterFl = false;

	_eventsManager.mouseOff();

	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();

	_graphicsManager.loadImage("LINUX");
	_graphicsManager.fadeInLong();
	_eventsManager.delay(1500);
	_graphicsManager.fadeOutLong();

	_graphicsManager.loadImage("H2");
	_graphicsManager.fadeInLong();
	_eventsManager.delay(500);
	_graphicsManager.fadeOutLong();

	if (!_eventsManager._escKeyFl)
		playIntro();

	_globals.iRegul = 0;
	_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
	_globals._characterType = 0;
	_objectsManager._mapCarPosX = _objectsManager._mapCarPosY = 0;
	memset(_globals._saveData, 0, 2000);
	_globals._exitId = 0;

	for (;;) {
		if (_globals._exitId == 300)
			_globals._exitId = 0;

		if (!_globals._exitId) {
			_globals._exitId = _menuManager.menu();
			if (_globals._exitId == -1) {
				if (!g_system->getEventManager()->shouldQuit())
					endLinuxDemo();
				_globals.PERSO = _globals.freeMemory(_globals.PERSO);
				restoreSystem();
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals._exitId) {
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
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1, true);
			break;

		case 3:
			if (!_globals._saveData->_data[svBankAttackAnimPlayedFl]) {
				_soundManager.playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsManager.loadImage("fond");
				else {
					if (_globals._language == LANG_FR)
						_graphicsManager.loadImage("fondfr");
					else if (_globals._language == LANG_EN)
						_graphicsManager.loadImage("fondan");
					else if (_globals._language == LANG_SP)
						_graphicsManager.loadImage("fondes");
				}
				_graphicsManager.fadeInLong();
				_eventsManager.delay(500);
				_graphicsManager.fadeOutLong();
				_globals.iRegul = 1;
				_soundManager._specialSoundNum = 2;

				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				_graphicsManager.FADE_LINUX = 2;

				if (!_globals._censorshipFl)
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				else
					_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				_soundManager._specialSoundNum = 0;
				_soundManager.removeSample(1);
				_soundManager.removeSample(2);
				_soundManager.removeSample(3);
				_soundManager.removeSample(4);
				_globals._saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}

			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			_globals._disableInventFl = true;
			_objectsManager.handleCityMap();
			_globals._disableInventFl = false;
			break;

		case 5:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 455;
			if (_globals._saveData->_data[svFreedHostageFl] == 1)
					_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			else
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			break;

		case 6:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals._saveData->_data[svBombBoxOpenedFl])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 440;

			if (!_globals._saveData->_data[svBombDisarmedFl])
				bombExplosion();
			else 
				_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			break;

		case 10:
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 450;
			if (_globals._saveData->_data[svBombDisarmedFl])
				_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1, false);
			else
				bombExplosion();
			break;

		case 13:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1, true);
			break;

		case 14:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1, true);
			break;

		case 15:
			_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29, false);
			break;

		case 16:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;

			if (_globals._saveData->_data[svForestAvailableFl] == 1) {
				_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7, true);
			} else if (!_globals._saveData->_data[svForestAvailableFl]) {
				_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7, true);
			}
			break;

		case 25:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30, true);
			break;

		case 26:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30, true);

		case 33:
			_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8, false);
			break;

		case 35:
			displayEndDemo();
			break;

		case 111:
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 113;
			_globals._saveData->_data[svLastScreenId] = 113;
			_computerManager.showComputer(COMPUTER_HOPKINS);

			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.updateScreen();
			memset(_graphicsManager._vesaBuffer, 0, 307200);
			memset(_graphicsManager._vesaScreen, 0, 307200);
			_graphicsManager.clearPalette();
			_graphicsManager.resetVesaSegment();
			break;

		case 114:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 114;
			_globals._saveData->_data[svLastScreenId] = 114;
			_computerManager.showComputer(COMPUTER_SAMANTHA);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 115:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._screenId;
			_globals._screenId = 115;
			_globals._saveData->_data[svLastScreenId] = 115;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 150:
			_soundManager.playSound(16);
			_globals.iRegul = 1;

			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR1A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 151:
			_soundManager.playSound(16);
			_globals.iRegul = 1;

			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR3A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 152:
			_soundManager.playSound(16);
			_globals.iRegul = 1;

			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR4A.anm", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;
		}
	}
	return true;
}

bool HopkinsEngine::runFull() {
	if (getPlatform() == Common::kPlatformLinux)
		_soundManager.playSound(16);

	_globals.loadObjects();
	_objectsManager.changeObject(14);
	_objectsManager.addObject(14);

	if (getPlatform() == Common::kPlatformLinux) {
		_objectsManager._helicopterFl = false;
		_eventsManager.mouseOff();
		// No code has been added to display the version as it's wrong 
		// in my copy: it mentions a Win95 version v4 using DirectDraw (Strangerke)
	} else if (getPlatform() == Common::kPlatformWindows) {
		_objectsManager._helicopterFl = false;
		_globals.iRegul = 1;
		// This code displays the game version.
		// It wasn't present in the original and could be put in the debugger
		// It has been added there for debug purposes
		_graphicsManager.loadImage("VERSW");
		_graphicsManager.fadeInLong();
		_eventsManager.delay(500);
		_graphicsManager.fadeOutLong();
	} else {
		// This piece of code, though named "display_version" in the original, 
		// displays a "loading please wait" screen.
		_graphicsManager.loadImage("VERSW");
		_graphicsManager.fadeInLong();
		_eventsManager.delay(500);
		_graphicsManager.fadeOutLong();
		_globals.iRegul = 1;
	}

	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();
	
	if (getPlatform() == Common::kPlatformLinux) {
		_graphicsManager.loadImage("H2");
		_graphicsManager.fadeInLong();
		_eventsManager.delay(500);
		_graphicsManager.fadeOutLong();

		_globals._speed = 2;
		_globals.iRegul = 1;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("MP.ANM", 10, 16, 200);
	} else {
		_animationManager.playAnim("MP.ANM", 10, 16, 200);
		_graphicsManager.fadeOutLong();
	}

	if (!_eventsManager._escKeyFl)
		playIntro();
	if (getPlatform() != Common::kPlatformLinux) {
		_graphicsManager.fadeOutShort();
		_graphicsManager.loadImage("H2");
		_graphicsManager.fadeInLong();
		_eventsManager.delay(500);
		_graphicsManager.fadeOutLong();
	}
	_globals.iRegul = 0;
	_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
	_globals._characterType = 0;
	_objectsManager._mapCarPosX = _objectsManager._mapCarPosY = 0;
	memset(_globals._saveData, 0, 2000);
	
	_globals._exitId = 0;

	for (;;) {
		if (_globals._exitId == 300)
			_globals._exitId = 0;
		if (!_globals._exitId) {
			_globals._exitId = _menuManager.menu();
			if (_globals._exitId == -1) {
				_globals.PERSO = _globals.freeMemory(_globals.PERSO);
				restoreSystem();
				return false;
			}
		}

		if (g_system->getEventManager()->shouldQuit())
			return false;

		switch (_globals._exitId) {
		case 1:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM01", "IM01", "ANIM01", "IM01", 1, true);
			break;

		case 3:
			if (!_globals._saveData->_data[svBankAttackAnimPlayedFl]) {
				_soundManager.playSound(3);
				if (getPlatform() == Common::kPlatformOS2 || getPlatform() == Common::kPlatformBeOS)
					_graphicsManager.loadImage("fond");
				else {
					if (_globals._language == LANG_FR)
						_graphicsManager.loadImage("fondfr");
					else if (_globals._language == LANG_EN)
						_graphicsManager.loadImage("fondan");
					else if (_globals._language == LANG_SP)
						_graphicsManager.loadImage("fondes");
				}
				_graphicsManager.fadeInLong();
				_eventsManager.delay(500);
				_graphicsManager.fadeOutLong();
				_globals.iRegul = 1;
				_soundManager._specialSoundNum = 2;
				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows) {
					if (getPlatform() == Common::kPlatformLinux)
						_graphicsManager.FADE_LINUX = 2;

					if (!_globals._censorshipFl)
						_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
					else
						_animationManager.playAnim("BANKUK.ANM", 200, 28, 200);
				} else {
					_animationManager.playAnim("BANQUE.ANM", 200, 28, 200);
				}
				_soundManager._specialSoundNum = 0;
				_soundManager.removeSample(1);
				_soundManager.removeSample(2);
				_soundManager.removeSample(3);
				_soundManager.removeSample(4);
				if (getPlatform() != Common::kPlatformLinux)
					_graphicsManager.fadeOutLong();
				_globals._saveData->_data[svBankAttackAnimPlayedFl] = 1;
			}
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM03", "IM03", "ANIM03", "IM03", 2, false);
			break;

		case 4:
			_globals._disableInventFl = true;
			_objectsManager.handleCityMap();
			_globals._disableInventFl = false;
			break;

		case 5:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 455;
			if (_globals._saveData->_data[svFreedHostageFl] == 1)
				_objectsManager.PERSONAGE2("IM05", "IM05A", "ANIM05B", "IM05", 3, false);
			else
				_objectsManager.PERSONAGE2("IM05", "IM05", "ANIM05", "IM05", 3, false);
			break;

		case 6:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 460;
			_objectsManager.PERSONAGE2("IM06", "IM06", "ANIM06", "IM06", 2, true);
			break;

		case 7:
			if (_globals._saveData->_data[svBombBoxOpenedFl])
				_objectsManager.PERSONAGE("BOMBEB", "BOMBE", "BOMBE", "BOMBE", 2, true);
			else
				_objectsManager.PERSONAGE("BOMBEA", "BOMBE", "BOMBE", "BOMBE", 2, true);
			break;

		case 8:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM08", "IM08", "ANIM08", "IM08", 2, true);
			break;

		case 9:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 440;
			if (_globals._saveData->_data[svBombDisarmedFl])
				_objectsManager.PERSONAGE2("IM09", "IM09", "ANIM09", "IM09", 10, true);
			else
				bombExplosion();
			break;

		case 10:
			_objectsManager.PERSONAGE("IM10", "IM10", "ANIM10", "IM10", 9, false);
			break;

		case 11:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 450;
			_objectsManager.PERSONAGE2("IM11", "IM11", "ANIM11", "IM11", 2, false);
			break;

		case 12:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 450;
			if (_globals._saveData->_data[svBombDisarmedFl])
				_objectsManager.PERSONAGE2("IM12", "IM12", "ANIM12", "IM12", 1, false);
			else
				bombExplosion();
			break;

		case 13:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM13", "IM13", "ANIM13", "IM13", 1, true);
			break;

		case 14:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM14", "IM14", "ANIM14", "IM14", 1, true);
			break;

		case 15:
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 29, false);
			else
				_objectsManager.PERSONAGE("IM15", "IM15", "ANIM15", "IM15", 18, false);
			break;

		case 16:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			if (_globals._saveData->_data[svForestAvailableFl] == 1)
				_objectsManager.PERSONAGE2("IM16", "IM16A", "ANIM16", "IM16", 7, true);
			else
				_objectsManager.PERSONAGE2("IM16", "IM16", "ANIM16", "IM16", 7, true);
			break;

		case 17:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			if (_globals._saveData->_data[svHutBurningFl] == 1)
				_objectsManager.PERSONAGE2("IM17", "IM17A", "ANIM17", "IM17", 11, true);
			else if (!_globals._saveData->_data[svHutBurningFl])
				_objectsManager.PERSONAGE2("IM17", "IM17", "ANIM17", "IM17", 11, true);
			if (_globals._exitId == 18) {
				_globals.iRegul = 1;
				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				_soundManager.stopSound();
				if (getPlatform() == Common::kPlatformLinux) {
					_soundManager.playSound(29);
					_graphicsManager.FADE_LINUX = 2;
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				} else if (getPlatform() == Common::kPlatformWindows) {
					_soundManager.playSound(29);
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
					_graphicsManager.fadeOutShort();
				} else {
					_soundManager.playSound(6);
					_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
					_graphicsManager.fadeOutShort();
				}
				_globals.iRegul = 0;
			}
			break;

		case 18:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsManager.PERSONAGE2("IM18", "IM18", "ANIM18", "IM18", 29, false);
			else
				_objectsManager.PERSONAGE2("IM18", "IM18", "ANIM18", "IM18", 6, false);
			break;

		case 19:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 440;
			if (_globals._saveData->_data[svHeavenGuardGoneFl])
				_objectsManager.PERSONAGE2("IM19", "IM19A", "ANIM19", "IM19", 6, true);
			else
				_objectsManager.PERSONAGE2("IM19", "IM19", "ANIM19", "IM19", 6, true);
			break;

		case 20:
			_linesManager.setMaxLineIdx(10);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM20", "IM20", "ANIM20", "IM20", 6, true);
			if (_globals._exitId == 17) {
				_globals.iRegul = 1;
				_soundManager.stopSound();
				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				_soundManager.playSound(6);
				if (getPlatform() == Common::kPlatformLinux)
					_graphicsManager.FADE_LINUX = 2;
				_animationManager.playAnim("PURG2A.ANM", 12, 18, 50);
				if (getPlatform() != Common::kPlatformLinux)
					_graphicsManager.fadeOutShort();
				_globals.iRegul = 0;
			}
			break;

		case 22:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM22", "IM22", "ANIM22", "IM22", 6, true);
			break;

		case 23:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM23", "IM23", "ANIM23", "IM23", 6, true);
			break;

		case 24:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			if (_globals._saveData->_data[svCinemaDogGoneFl] == 1)
				_objectsManager.PERSONAGE2("IM24", "IM24A", "ANIM24", "IM24", 1, true);
			else
				_objectsManager.PERSONAGE2("IM24", "IM24", "ANIM24", "IM24", 1, true);
			break;

		case 25:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 445;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 30, true);
			else
				_objectsManager.PERSONAGE2("IM25", "IM25", "ANIM25", "IM25", 8, true);
			break;

		case 26:
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
				_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 30, true);
			else
				_objectsManager.PERSONAGE2("IM26", "IM26", "ANIM26", "IM26", 8, true);
			break;

		case 27:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 440;
			if (_globals._saveData->_data[svPoolDogGoneFl] == 1)
				_objectsManager.PERSONAGE2("IM27", "IM27A", "ANIM27", "IM27", 27, true);
			else
				_objectsManager.PERSONAGE2("IM27", "IM27", "ANIM27", "IM27", 27, true);
			break;

		case 28:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 450;
			if (_globals._saveData->_data[svCinemaCurtainCond1] != 1 || _globals._saveData->_data[svCinemaCurtainCond2] != 1)
				_objectsManager.PERSONAGE2("IM28", "IM28", "ANIM28", "IM28", 1, false);
			else
				_objectsManager.PERSONAGE2("IM28A", "IM28", "ANIM28", "IM28", 1, false);
			break;

		case 29:
			_linesManager.setMaxLineIdx(50);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM29", "IM29", "ANIM29", "IM29", 1, true);
			break;

		case 30:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM30", "IM30", "ANIM30", "IM30", 24, false);
			break;

		case 31:
			_objectsManager.PERSONAGE("IM31", "IM31", "ANIM31", "IM31", 10, true);
			break;

		case 32:
			_linesManager.setMaxLineIdx(20);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM32", "IM32", "ANIM32", "IM32", 2, true);
			break;

		case 33:
			_objectsManager.PERSONAGE("IM33", "IM33", "ANIM33", "IM33", 8, false);
			break;

		case 34:
			_objectsManager.PERSONAGE("IM34", "IM34", "ANIM34", "IM34", 2, false);
			break;

		case 35:
		case 36:
		case 37:
		case 38:
		case 39:
		case 40:
		case 41: {
			_linesManager.setMaxLineIdx(40);
			_globals._characterMaxPosY = 435;
			_globals._disableInventFl = false;
			_objectsManager._forestFl = true;
			Common::String im = Common::String::format("IM%d", _globals._exitId);
			_soundManager.playSound(13);
			if (_objectsManager._forestSprite == g_PTRNUL) {
				_objectsManager._forestSprite = _objectsManager.loadSprite("HOPDEG.SPR");
				_soundManager.loadSample(1, "SOUND41.WAV");
			}
			_objectsManager.PERSONAGE2(im, im, "BANDIT", im, 13, false);
			if (_globals._exitId < 35 || _globals._exitId > 49) {
				_objectsManager._forestSprite = _globals.freeMemory(_objectsManager._forestSprite);
				_objectsManager._forestFl = false;
				_soundManager.removeSample(1);
			}
			break;
			}

		case 50:
			playPlaneCutscene();
			_globals._exitId = 51;
			break;

		case 51:
			_linesManager.setMaxLineIdx(10);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM51", "IM51", "ANIM51", "IM51", 14, true);
			break;

		case 52:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM52", "IM52", "ANIM52", "IM52", 14, true);
			break;

		case 54:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM54", "IM54", "ANIM54", "IM54", 14, true);
			break;

		case 55:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 460;
			_objectsManager.PERSONAGE2("IM55", "IM55", "ANIM55", "IM55", 14, false);
			break;

		case 56:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM56", "IM56", "ANIM56", "IM56", 14, false);
			break;

		case 57:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM57", "IM57", "ANIM57", "IM57", 14, true);
			break;

		case 58:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM58", "IM58", "ANIM58", "IM58", 14, false);
			break;

		case 59:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM59", "IM59", "ANIM59", "IM59", 21, false);
			break;

		case 60:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM60", "IM60", "ANIM60", "IM60", 21, false);
			break;

		case 61:
			if (_globals._saveData->_data[svBaseElevatorCond1] == 1 && !_globals._saveData->_data[svBaseFireFl])
				handleConflagration();
			_objectsManager.PERSONAGE("IM61", "IM61", "ANIM61", "IM61", 21, false);
			break;

		case 62:
			_linesManager.setMaxLineIdx(8);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM62", "IM62", NULL, "IM62", 21, false);
			break;

		case 63:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM63", "IM63", "ANIM63", "IM63", 21, false);
			break;

		case 64:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM64", "IM64", "ANIM64", "IM64", 21, true);
			break;

		case 65:
			_linesManager.setMaxLineIdx(30);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM65", "IM65", "ANIM65", "IM65", 21, false);
			break;

		case 66:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM66", "IM66", "ANIM66", "IM66", 21, false);
			break;

		case 67:
			_linesManager.setMaxLineIdx(8);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM67", "IM67", NULL, "IM67", 21, false);
			break;

		case 68:
			_linesManager.setMaxLineIdx(8);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM68", "IM68", "ANIM68", "IM68", 21, true);
			break;

		case 69:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM69", "IM69", "ANIM69", "IM69", 21, false);
			break;

		case 70:
			_linesManager.setMaxLineIdx(8);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM70", "IM70", NULL, "IM70", 21, false);
			break;

		case 71:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 445;
			_objectsManager.PERSONAGE2("IM71", "IM71", "ANIM71", "IM71", 21, false);
			break;

		case 73:
			_linesManager.setMaxLineIdx(15);
			_globals._characterMaxPosY = 445;
			if (_globals._saveData->_data[svSecondElevatorAvailableFl] == 1)
				_objectsManager.PERSONAGE2("IM73", "IM73A", "ANIM73", "IM73", 21, true);
			else
				_objectsManager.PERSONAGE2("IM73", "IM73", "ANIM73", "IM73", 21, true);
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
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 445;
			if (_globals._saveData->_data[svEscapeLeftJailFl]) {
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
					_objectsManager.PERSONAGE2("IM93", "IM93C", "ANIM93", "IM93", 29, true);
				else
					_objectsManager.PERSONAGE2("IM93", "IM93C", "ANIM93", "IM93", 26, true);
			} else {
				if (getPlatform() == Common::kPlatformLinux || getPlatform() == Common::kPlatformWindows)
					_objectsManager.PERSONAGE2("IM93", "IM93", "ANIM93", "IM93", 29, true);
				else
					_objectsManager.PERSONAGE2("IM93", "IM93", "ANIM93", "IM93", 26, true);
			}
			break;

		case 94:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 440;
			_objectsManager.PERSONAGE2("IM94", "IM94", "ANIM94", "IM94", 19, true);
			break;

		case 95:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM95", "IM95", "ANIM95", "IM95", 19, false);
			break;

		case 96:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM96", "IM96", "ANIM96", "IM96", 19, false);
			break;

		case 97:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM97", "IM97", "ANIM97", "IM97", 19, false);
			if (_globals._exitId == 18) {
				_globals.iRegul = 1;
				_soundManager.stopSound();
				_graphicsManager.lockScreen();
				_graphicsManager.clearScreen();
				_graphicsManager.unlockScreen();
				_graphicsManager.clearPalette();
				_soundManager.playSound(6);
				_animationManager.playAnim("PURG1A.ANM", 12, 18, 50);
				_graphicsManager.fadeOutShort();
				_globals.iRegul = 0;
			}
			break;

		case 98:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM98", "IM98", "ANIM98", "IM98", 19, true);
			break;

		case 99:
			_linesManager.setMaxLineIdx(5);
			_globals._characterMaxPosY = 435;
			_objectsManager.PERSONAGE2("IM99", "IM99", "ANIM99", "IM99", 19, true);
			break;

		case 100:
			playEnding();
			break;

		case 111:
			_objectsManager.PERSONAGE("IM111", "IM111", "ANIM111", "IM111", 10, false);
			break;

		case 112:
			_objectsManager.PERSONAGE("IM112", "IM112", "ANIM112", "IM112", 10, false);
			break;

		case 113:
			_globals._prevScreenId = _globals._screenId;
			_globals._screenId = 113;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._prevScreenId;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_globals._exitId = 0;
			_computerManager.showComputer(COMPUTER_HOPKINS);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.updateScreen();
			memset(_graphicsManager._vesaBuffer, 0, 307200);
			memset(_graphicsManager._vesaScreen, 0, 307200);
			_graphicsManager.clearPalette();
			_graphicsManager.resetVesaSegment();
			break;

		case 114:
			_globals._exitId = 0;
			_globals._prevScreenId = _globals._screenId;
			_globals._screenId = 114;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._prevScreenId;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_computerManager.showComputer(COMPUTER_SAMANTHA);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 115:
			_globals._prevScreenId = _globals._screenId;
			_globals._screenId = 115;
			_globals._saveData->_data[svLastPrevScreenId] = _globals._prevScreenId;
			_globals._saveData->_data[svLastScreenId] = _globals._screenId;
			_globals._exitId = 0;
			_computerManager.showComputer(COMPUTER_PUBLIC);
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			break;

		case 150:
			_soundManager.playSound(16);
			_globals.iRegul = 1;
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR1A.ANM", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 151:
			_soundManager.playSound(16);
			_globals.iRegul = 1;
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR3A.ANM", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 152:
			_soundManager.playSound(16);
			_globals.iRegul = 1;
			_graphicsManager.lockScreen();
			_graphicsManager.clearScreen();
			_graphicsManager.unlockScreen();
			_graphicsManager.clearPalette();
			if (getPlatform() == Common::kPlatformLinux)
				_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("JOUR4A.ANM", 12, 12, 2000);
			_globals.iRegul = 0;
			_globals._exitId = 300;
			break;

		case 194:
		case 195:
		case 196:
		case 197:
		case 198:
		case 199:
			_globals.PERSO = _globals.freeMemory(_globals.PERSO);
			_globals.iRegul = 1;
			_soundManager.stopSound();
			_soundManager.playSound(23);
			_globals._exitId = handleBaseMap();	// Handles the base map (non-Windows)
			//_globals._exitId = WBASE();	// Handles the 3D Doom level (Windows)
			_soundManager.stopSound();
			_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
			_globals._characterType = 0;
			_globals.iRegul = 0;
			_graphicsManager._lineNbr = SCREEN_WIDTH;
			break;
		}
	}
	_globals.PERSO = _globals.freeMemory(_globals.PERSO);
	restoreSystem();
	return true;
}

bool HopkinsEngine::shouldQuit() const {
	return g_system->getEventManager()->shouldQuit();
}

int HopkinsEngine::getRandomNumber(int maxNumber) {
	return _randomSource.getRandomNumber(maxNumber);
}

void HopkinsEngine::initializeSystem() {
	// Set graphics mode
	_graphicsManager.setGraphicalMode(SCREEN_WIDTH, SCREEN_HEIGHT);

	// Synchronize the sound settings from ScummVM
	_soundManager.syncSoundSettings();

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

	_globals.clearAll();

	_eventsManager.initMouseData();
	_fontManager.initData();

	_dialogsManager._inventoryIcons = _fileManager.loadFile("ICONE.SPR");
	_objectsManager._headSprites = _fileManager.loadFile("TETE.SPR");

	_eventsManager.setMouseOn();
	_eventsManager._mouseFl = false;

	_globals.loadCharacterData();

	_eventsManager._mouseOffset.x = 0;
	_eventsManager._mouseOffset.y = 0;
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
	_eventsManager.refreshScreenAndEvents();
	_eventsManager._mouseFl = false;
	_globals.iRegul = 1;
	_eventsManager.refreshScreenAndEvents();
	_soundManager.playSound(16);
	_animationManager._clearAnimationFl = true;
	_animationManager.playAnim("J1.anm", 12, 12, 50);
	if (shouldQuit() || _eventsManager._escKeyFl)
		return;

	_soundManager.mixVoice(1, 3);
	_animationManager.playAnim("J2.anm", 12, 12, 50);

	if (shouldQuit() || _eventsManager._escKeyFl)
		return;

	_soundManager.mixVoice(2, 3);
	_animationManager.playAnim("J3.anm", 12, 12, 50);

	if (shouldQuit() || _eventsManager._escKeyFl)
		return;

	_soundManager.mixVoice(3, 3);
	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();
	_graphicsManager.updateScreen();
	_soundManager.playSound(11);
	_graphicsManager.loadImage("intro1");
	_graphicsManager.scrollScreen(0);
	_graphicsManager._scrollOffset = 0;
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	for (int i = 0; i <= 4; i++)
		_eventsManager.refreshScreenAndEvents();

	_globals.iRegul = 1;
	_graphicsManager.fadeInLong();
	if (_graphicsManager._largeScreenFl) {
		_graphicsManager._scrollStatus = 2;
		_graphicsManager._scrollPosX = 0;

		bool loopCond = false;
		do {
			_graphicsManager._scrollPosX += 2;
			if (_graphicsManager._scrollPosX > (SCREEN_WIDTH - 2)) {
				_graphicsManager._scrollPosX = SCREEN_WIDTH;
				loopCond = true;
			}

			if (_eventsManager.getMouseX() < _graphicsManager._scrollPosX + 10)
				_eventsManager.setMouseXY(_eventsManager._mousePos.x + 4, _eventsManager.getMouseY());
			_eventsManager.refreshScreenAndEvents();
		} while (!shouldQuit() && !loopCond && _graphicsManager._scrollPosX != SCREEN_WIDTH);

		_eventsManager.refreshScreenAndEvents();
		_graphicsManager._scrollStatus = 0;

		if (shouldQuit())
			return;
	}

	_soundManager.mixVoice(4, 3);
	_graphicsManager.fadeOutLong();
	_graphicsManager._scrollStatus = 0;
	_graphicsManager.loadImage("intro2");
	_graphicsManager.scrollScreen(0);
	_animationManager.loadAnim("INTRO2");
	_graphicsManager.displayAllBob();
	_soundManager.playSound(23);
	_objectsManager.stopBobAnimation(3);
	_objectsManager.stopBobAnimation(5);
	_graphicsManager._scrollOffset = 0;
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	for (int i = 0; i <= 4; i++)
		_eventsManager.refreshScreenAndEvents();

	_globals.iRegul = 1;
	_graphicsManager.fadeInLong();
	for (uint i = 0; i < 200 / _globals._speed; ++i)
		_eventsManager.refreshScreenAndEvents();

	_objectsManager.setBobAnimation(3);
	_soundManager.mixVoice(5, 3);
	_objectsManager.stopBobAnimation(3);
	_eventsManager.refreshScreenAndEvents();
	memcpy(&paletteData2, _graphicsManager._palette, 796);

	_graphicsManager.setPaletteVGA256WithRefresh(paletteData, _graphicsManager._vesaBuffer);
	_graphicsManager.endDisplayBob();

	if (shouldQuit() || _eventsManager._escKeyFl)
		return;

	_soundManager._specialSoundNum = 5;
	_graphicsManager.FADE_LINUX = 2;
	_animationManager.playAnim("ELEC.ANM", 10, 26, 200);
	_soundManager._specialSoundNum = 0;

	if (shouldQuit() || _eventsManager._escKeyFl)
		return;

	_graphicsManager.loadImage("intro2");
	_graphicsManager.scrollScreen(0);
	_animationManager.loadAnim("INTRO2");
	_graphicsManager.displayAllBob();
	_soundManager.playSound(23);
	_objectsManager.stopBobAnimation(3);
	_objectsManager.stopBobAnimation(5);
	_objectsManager.stopBobAnimation(1);
	_graphicsManager._scrollOffset = 0;
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);

	for (int i = 0; i <= 3; i++)
		_eventsManager.refreshScreenAndEvents();

	_globals.iRegul = 1;
	_graphicsManager.setPaletteVGA256WithRefresh(paletteData2, _graphicsManager._vesaBuffer);

	int introIndex = 0;
	while (!shouldQuit() && !_eventsManager._escKeyFl) {
		if (introIndex == 12) {
			_objectsManager.setBobAnimation(3);
			_eventsManager.refreshScreenAndEvents();
			_soundManager.mixVoice(6, 3);
			_eventsManager.refreshScreenAndEvents();
			_objectsManager.stopBobAnimation(3);
		}

		Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager._palette[0]);

		for (int i = 1, v12 = 4 * introIndex; i <= PALETTE_BLOCK_SIZE; i++) {
			if (_graphicsManager._palette[i] > v12)
				_graphicsManager._palette[i] -= v12;
		}

		_graphicsManager.setPaletteVGA256WithRefresh(_graphicsManager._palette, _graphicsManager._vesaBuffer);

		for (int i = 1; i < 2 * introIndex; i++)
			_eventsManager.refreshScreenAndEvents();

		_graphicsManager.setPaletteVGA256WithRefresh(paletteData2, _graphicsManager._vesaBuffer);

		for (int i = 1; i < 20 - introIndex; i++)
			_eventsManager.refreshScreenAndEvents();

		introIndex += 2;
		if (introIndex > 15) {
			_graphicsManager.setPaletteVGA256WithRefresh(paletteData, _graphicsManager._vesaBuffer);
			for (uint j = 1; j < 100 / _globals._speed; ++j)
				_eventsManager.refreshScreenAndEvents();

			_objectsManager.setBobAnimation(3);
			_soundManager.mixVoice(7, 3);
			_objectsManager.stopBobAnimation(3);

			for (uint k = 1; k < 60 / _globals._speed; ++k)
				_eventsManager.refreshScreenAndEvents();
			_objectsManager.setBobAnimation(5);
			for (uint l = 0; l < 20 / _globals._speed; ++l)
				_eventsManager.refreshScreenAndEvents();

			Common::copy(&paletteData2[0], &paletteData2[PALETTE_BLOCK_SIZE], &_graphicsManager._palette[0]);
			_graphicsManager.setPaletteVGA256WithRefresh(_graphicsManager._palette, _graphicsManager._vesaBuffer);

			for (uint m = 0; m < 50 / _globals._speed; ++m) {
				if (m == 30 / _globals._speed) {
					_objectsManager.setBobAnimation(3);
					_soundManager.mixVoice(8, 3);
					_objectsManager.stopBobAnimation(3);
				}

				_eventsManager.refreshScreenAndEvents();
			}

			_graphicsManager.fadeOutLong();
			_graphicsManager.endDisplayBob();
			_animationManager._clearAnimationFl = true;
			_soundManager.playSound(3);
			_soundManager._specialSoundNum = 1;
			_animationManager.playAnim("INTRO1.anm", 10, 24, 18);
			_soundManager._specialSoundNum = 0;
			if (shouldQuit() || _eventsManager._escKeyFl)
				return;

			_animationManager.playAnim("INTRO2.anm", 10, 24, 18);
			if (shouldQuit() || _eventsManager._escKeyFl)
				return;

			_animationManager.playAnim("INTRO3.anm", 10, 24, 200);
			if (shouldQuit() || _eventsManager._escKeyFl)
				return;

			_animationManager._clearAnimationFl = false;
			_graphicsManager.FADE_LINUX = 2;
			_animationManager.playAnim("J4.anm", 12, 12, 1000);
			break;
		}
	}

	_eventsManager._escKeyFl = false;
}

/**
 * If in demo, displays a 'not available' screen and returns to the city map
 */
void HopkinsEngine::displayNotAvailable() {
	if (!getIsDemo())
		return;

	if (_globals._language == LANG_FR)
		_graphicsManager.loadImage("ndfr");
	else
		_graphicsManager.loadImage("nduk");

	_graphicsManager.fadeInLong();
	if (_soundManager._voiceOffFl)
		_eventsManager.delay(500);
	else
		_soundManager.mixVoice(628, 4);

	_graphicsManager.fadeOutLong();
	_globals._exitId = 4;
}

void HopkinsEngine::handleNotAvailable(int sortie) {
	// Use the code of the linux demo instead of the code of the Windows demo.
	// The behavior is somewhat better, and common code is easier to maintain.
	displayNotAvailable();
	_globals._exitId = sortie;
}

void HopkinsEngine::displayEndDemo() {
	_soundManager.playSound(28);
	if (_globals._language == LANG_FR)
		_graphicsManager.loadImage("endfr");
	else
		_graphicsManager.loadImage("enduk");

	_graphicsManager.fadeInLong();
	_eventsManager.delay(1500);
	_graphicsManager.fadeOutLong();
	_globals._exitId = 0;
}

void HopkinsEngine::bombExplosion() {
	_graphicsManager._lineNbr = SCREEN_WIDTH;
	_graphicsManager.setScreenWidth(SCREEN_WIDTH);
	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();

	_globals.iRegul = 1;
	_soundManager._specialSoundNum = 199;
	_graphicsManager.FADE_LINUX = 2;
	_animationManager.playAnim("BOMBE2A.ANM", 50, 14, 500);
	_soundManager._specialSoundNum = 0;
	_graphicsManager.loadImage("IM15");
	_animationManager.loadAnim("ANIM15");
	_graphicsManager.displayAllBob();
	_objectsManager.stopBobAnimation(7);

	for (int idx = 0; idx < 5; ++idx) {
		_eventsManager.refreshScreenAndEvents();
	}

	_graphicsManager.fadeInLong();
	_eventsManager.mouseOff();

	for (int idx = 0; idx < 20; ++idx) {
		_eventsManager.refreshScreenAndEvents();
	}

	_globals._introSpeechOffFl = true;
	_talkManager.startStaticCharacterDialogue("vire.pe2");
	_globals._introSpeechOffFl = false;
	_objectsManager.setBobAnimation(7);

	for (int idx = 0; idx < 100; ++idx) {
		_eventsManager.refreshScreenAndEvents();
	}

	_graphicsManager.fadeOutLong();
	_graphicsManager.endDisplayBob();
	_globals.iRegul = 0;
	_globals._exitId = 151;
}

void HopkinsEngine::restoreSystem() {
	quitGame();
	_eventsManager.refreshEvents();
}

void HopkinsEngine::endLinuxDemo() {
	_globals._linuxEndDemoFl = true;
	_graphicsManager.resetVesaSegment();
	_objectsManager._forestFl = false;
	_eventsManager._breakoutFl = false;
	_globals._disableInventFl = true;
	_graphicsManager.loadImage("BOX");
	_soundManager.playSound(28);
	_graphicsManager.fadeInLong();
	_eventsManager.mouseOn();
	_eventsManager.changeMouseCursor(0);
	_eventsManager._mouseCursorId = 0;
	_eventsManager._mouseSpriteId = 0;

	bool mouseClicked = false;

	do {
		_eventsManager.refreshScreenAndEvents();

		if (_eventsManager.getMouseButton() == 1)
			mouseClicked = true;
	} while (!mouseClicked && !g_system->getEventManager()->shouldQuit());

	// Original tried to open a web browser link here. Since ScummVM doesn't support
	// that, it's being skipped in favor of simply exiting

	_graphicsManager.fadeOutLong();
}

void HopkinsEngine::handleConflagration() {
	_globals._disableInventFl = true;
	_globals.iRegul = 1;
	_graphicsManager.loadImage("IM71");
	_animationManager.loadAnim("ANIM71");
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_graphicsManager.displayAllBob();

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.refreshScreenAndEvents();

	_graphicsManager.fadeInLong();
	_globals.iRegul = 1;

	for (int cpt = 0; cpt <= 249; cpt++)
		_eventsManager.refreshScreenAndEvents();

	_globals._introSpeechOffFl = true;
	_talkManager.startAnimatedCharacterDialogue("SVGARD1.pe2");
	_globals._introSpeechOffFl = false;

	for (int cpt = 0; cpt <= 49; cpt++)
		_eventsManager.refreshScreenAndEvents();

	_graphicsManager.fadeOutLong();
	_graphicsManager.endDisplayBob();
	_globals._saveData->_data[svBaseFireFl] = 1;
	_globals._disableInventFl = false;
}

void HopkinsEngine::playSubmarineCutscene() {
	_globals.iRegul = 1;
	_graphicsManager._lineNbr = SCREEN_WIDTH;
	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();
	_animationManager._clearAnimationFl = true;
	_soundManager.playSound(25);
	_animationManager.playAnim("base00a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base05a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base10a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base20a.anm", 10, 18, 18);
	// CHECKME: The original code was doing the opposite test, which was a bug.
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base30a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base40a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("base50a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("OC00a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("OC05a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("OC10a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("OC20a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl) {
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("OC30a.anm", 10, 18, 18);
	}

	_eventsManager._escKeyFl = false;
	_animationManager._clearAnimationFl = false;
	_globals._exitId = 85;
}

void HopkinsEngine::playUnderwaterBaseCutscene() {
	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();
	_animationManager.NO_SEQ = false;
	_soundManager.playSound(26);
	_globals.iRegul = 1;
	_globals._disableInventFl = true;
	_animationManager.NO_COUL = true;
	_graphicsManager.FADE_LINUX = 2;
	_animationManager.playSequence("abase.seq", 50, 15, 50);
	_animationManager.NO_COUL = false;
	_graphicsManager.loadImage("IM92");
	_animationManager.loadAnim("ANIM92");
	_graphicsManager.displayAllBob();
	_objectsManager.loadLinkFile("IM92");

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.refreshScreenAndEvents();

	_graphicsManager.fadeInLong();
	_globals.enableHiding();

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(8) != 22);

	_graphicsManager.fadeOutLong();
	_graphicsManager.endDisplayBob();
	_globals.resetHidingItems();
	_globals._disableInventFl = false;
	_globals._exitId = 93;
	_globals.iRegul = 0;
}

void HopkinsEngine::playEnding() {
	_globals.PERSO = _globals.freeMemory(_globals.PERSO);
	_dialogsManager._removeInventFl = true;
	_globals._disableInventFl = true;
	_graphicsManager._scrollOffset = 0;
	_globals._cityMapEnabledFl = false;
	_globals.iRegul = 1;
	_soundManager.playSound(26);
	_linesManager._route = (RouteItem *)g_PTRNUL;
	_globals._freezeCharacterFl = true;
	_globals._exitId = 0;
	_soundManager.loadSample(1, "SOUND90.WAV");
	_graphicsManager.loadImage("IM100");
	_animationManager.loadAnim("ANIM100");
	_graphicsManager.displayAllBob();
	_eventsManager.mouseOn();
	_objectsManager.stopBobAnimation(7);
	_objectsManager.stopBobAnimation(8);
	_objectsManager.stopBobAnimation(9);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.changeMouseCursor(0);

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.refreshScreenAndEvents();

	_graphicsManager.fadeInLong();
	_globals.iRegul = 1;

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(6) != 54);

	_globals._introSpeechOffFl = true;
	_talkManager.startAnimatedCharacterDialogue("GM4.PE2");
	_globals._disableInventFl = true;
	_objectsManager.stopBobAnimation(6);
	_objectsManager.stopBobAnimation(10);
	_objectsManager.setBobAnimation(9);
	_objectsManager.setBobAnimation(7);

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(7) != 54);

	_soundManager.playSample(1);

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(7) != 65);

	_globals._introSpeechOffFl = true;
	_talkManager.startAnimatedCharacterDialogue("DUELB4.PE2");
	_eventsManager.mouseOff();
	_globals._disableInventFl = true;

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(7) != 72);

	_globals._introSpeechOffFl = true;
	_talkManager.startAnimatedCharacterDialogue("DUELH1.PE2");

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(7) != 81);

	_globals._introSpeechOffFl = true;
	_talkManager.startAnimatedCharacterDialogue("DUELB5.PE2");

	do
		_eventsManager.refreshScreenAndEvents();
	while (_objectsManager.getBobAnimDataIdx(7) != 120);

	_objectsManager.stopBobAnimation(7);
	if (_globals._saveData->_data[svGameWonFl] == 1) {
		_soundManager._specialSoundNum = 200;
		_soundManager._skipRefreshFl = true;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("BERM.ANM", 100, 24, 300);
		_graphicsManager.endDisplayBob();
		_soundManager.removeSample(1);
		_graphicsManager.loadImage("PLAN3");
		_graphicsManager.fadeInLong();

		_eventsManager._rateCounter = 0;
		if (!_eventsManager._escKeyFl) {
			do
				_eventsManager.refreshEvents();
			while (_eventsManager._rateCounter < 2000 / _globals._speed && !_eventsManager._escKeyFl);
		}
		_eventsManager._escKeyFl = false;
		_graphicsManager.fadeOutLong();
		_globals.iRegul = 1;
		_soundManager._specialSoundNum = 0;
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("JOUR2A.anm", 12, 12, 1000);
		_soundManager.playSound(11);
		_graphicsManager.lockScreen();
		_graphicsManager.clearScreen();
		_graphicsManager.unlockScreen();
		_graphicsManager.clearPalette();
		_animationManager.playAnim("FF1a.anm", 18, 18, 9);
		_animationManager.playAnim("FF1a.anm", 9, 18, 9);
		_animationManager.playAnim("FF1a.anm", 9, 18, 18);
		_animationManager.playAnim("FF1a.anm", 9, 18, 9);
		_animationManager.playAnim("FF2a.anm", 24, 24, 100);
		displayCredits();
		_globals.iRegul = 0;
		_globals._exitId = 300;
		_dialogsManager._removeInventFl = false;
		_globals._disableInventFl = false;
	} else {
		_soundManager._specialSoundNum = 200;
		_soundManager._skipRefreshFl = true;
		_animationManager.playAnim2("BERM.ANM", 100, 24, 300);
		_objectsManager.stopBobAnimation(7);
		_objectsManager.setBobAnimation(8);
		_globals._introSpeechOffFl = true;
		_talkManager.startAnimatedCharacterDialogue("GM5.PE2");
		_globals._disableInventFl = true;

		do
			_eventsManager.refreshScreenAndEvents();
		while (_objectsManager.getBobAnimDataIdx(8) != 5);

		_soundManager.directPlayWav("SOUND41.WAV");

		do
			_eventsManager.refreshScreenAndEvents();
		while (_objectsManager.getBobAnimDataIdx(8) != 21);

		_graphicsManager.fadeOutLong();
		_graphicsManager.endDisplayBob();
		_soundManager.removeSample(1);
		_soundManager.playSound(16);
		_globals.iRegul = 1;
		_soundManager._specialSoundNum = 0;
		_dialogsManager._removeInventFl = false;
		_globals._disableInventFl = false;
		_animationManager.playAnim("JOUR4A.anm", 12, 12, 1000);
		_globals.iRegul = 0;
		_globals._exitId = 300;
	}
	_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
	_globals._characterType = 0;
	_globals.iRegul = 0;
}

void HopkinsEngine::playPlaneCutscene() {
	_soundManager.playSound(28);
	_globals.iRegul = 1;
	_graphicsManager.lockScreen();
	_graphicsManager.clearScreen();
	_graphicsManager.unlockScreen();
	_graphicsManager.clearPalette();

	_animationManager._clearAnimationFl = false;
	_animationManager.playAnim("aerop00a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("serop10a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop20a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop30a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop40a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop50a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop60a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("aerop70a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans00a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans10a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans15a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans20a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans30a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl)
		_animationManager.playAnim("trans40a.anm", 10, 18, 18);
	if (!_eventsManager._escKeyFl) {
		_graphicsManager.FADE_LINUX = 2;
		_animationManager.playAnim("PARA00a.anm", 9, 9, 9);
	}

	_eventsManager._escKeyFl = false;
	_animationManager._clearAnimationFl = false;
}

void HopkinsEngine::loadBaseMap() {
	Common::String filename	= Common::String::format("%s.PCX", "PBASE");
	Common::File f;

	if (f.exists(filename)) {
		// PBASE file exists, so go ahead and load it
		_graphicsManager.loadImage("PBASE");
	} else {
		// PBASE file doesn't exist, so draw a substitute screen
		drawBaseMap();
	}
}

void HopkinsEngine::drawBaseMap() {
	memset(_graphicsManager._vesaScreen, 0, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

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
			byte *pDest = _graphicsManager._vesaScreen + yp * SCREEN_WIDTH + r.left;
			Common::fill(pDest, pDest + r.width(), 0xff);
		}
	}

	// Copy the calculated screen
	memcpy(_graphicsManager._vesaBuffer, _graphicsManager._vesaScreen, SCREEN_WIDTH * 2 * SCREEN_HEIGHT);

	// Write some explanatory text
	_fontManager.displayText(40, 200, "ScummVM base map - select a square for different rooms", 255);
}

int HopkinsEngine::handleBaseMap() {
	_globals._disableInventFl = true;

	// Load the map image
	loadBaseMap();

	// Set needed colours
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_eventsManager.changeMouseCursor(0);
	_graphicsManager.fadeInLong();
	bool loopCond = false;
	int zone;
	do {
		if (shouldQuit())
			return 0;

		int mouseButton = _eventsManager.getMouseButton();
		int posX = _eventsManager.getMouseX();
		int posY = _eventsManager.getMouseY();
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
			_eventsManager.changeMouseCursor(4);
			_globals._baseMapColor += 25;
			if (_globals._baseMapColor > 100)
				_globals._baseMapColor = 0;
			_graphicsManager.SETCOLOR4(251, _globals._baseMapColor, _globals._baseMapColor, _globals._baseMapColor);
		} else {
			_eventsManager.changeMouseCursor(0);
			_graphicsManager.SETCOLOR4(251, 100, 100, 100);
		}
		_eventsManager.refreshScreenAndEvents();
		if ((mouseButton == 1) && zone)
			loopCond = true;
	} while (!loopCond);

	_globals._disableInventFl = false;
	_graphicsManager.fadeOutLong();

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
	_globals._creditsPosY = 440;
	_globals._creditsStep = 45;
	byte *bufPtr;
	switch (_globals._language) {
	case LANG_EN:
		bufPtr = _fileManager.loadFile("CREAN.TXT");
		break;
	case LANG_FR:
		bufPtr = _fileManager.loadFile("CREFR.TXT");
		break;
	case LANG_SP:
		bufPtr = _fileManager.loadFile("CREES.TXT");
		break;
	default:
		error("Unhandled language");
		break;
	}

	byte *curPtr = bufPtr;
	int idxLines = 0;
	bool loopCond = false;
	do {
		if (*curPtr == '%') {
			if (curPtr[1] == '%') {
				loopCond = true;
				break;
			}
			_globals._creditsItem[idxLines]._colour = curPtr[1];
			_globals._creditsItem[idxLines]._actvFl = true;
			_globals._creditsItem[idxLines]._linePosY = _globals._creditsPosY + idxLines * _globals._creditsStep;

			int idxBuf = 0;
			for(; idxBuf < 49; idxBuf++) {
				byte curChar = curPtr[idxBuf + 3];
				if (curChar == '%' || curChar == 10)
					break;
				_globals._creditsItem[idxLines]._line[idxBuf] = curChar;
			}
			_globals._creditsItem[idxLines]._line[idxBuf] = 0;
			_globals._creditsItem[idxLines]._lineSize = idxBuf - 1;
			curPtr = curPtr + idxBuf + 2;
			++idxLines;
		} else {
			curPtr++;
		}
		_globals._creditsLineNumb = idxLines;
	} while (!loopCond);

	_globals.freeMemory(bufPtr);
}

void HopkinsEngine::displayCredits(int startPosY, byte *buffer, char colour) {
	byte *bufPtr = buffer;
	int strWidth = 0;
	byte curChar;
	for (;;) {
		curChar = *bufPtr++;
		if (!curChar)
			break;
		if (curChar > 31)
			strWidth += _objectsManager.getWidth(_fontManager._font, curChar - 32);
	}
	int startPosX = 320 - strWidth / 2;
	int endPosX = strWidth + startPosX;
	int endPosY = startPosY + 12;
	if ((_globals._creditsStartX == -1) && (_globals._creditsEndX == -1) && (_globals._creditsStartY == -1) && (_globals._creditsEndY == -1)) {
		_globals._creditsStartX = startPosX;
		_globals._creditsEndX = endPosX;
		_globals._creditsStartY = startPosY;
		_globals._creditsEndY = endPosY;
	}
	if (startPosX < _globals._creditsStartX)
		_globals._creditsStartX = startPosX;
	if (endPosX > _globals._creditsEndX)
		_globals._creditsEndX = endPosX;
	if (_globals._creditsStartY > startPosY)
		_globals._creditsStartY = startPosY;
	if (endPosY > _globals._creditsEndY)
		_globals._creditsEndY = endPosY;

	bufPtr = buffer;
	for (;;) {
		curChar = *bufPtr++;
		if (!curChar)
			break;
		if (curChar > 31) {
			_graphicsManager.displayFont(_graphicsManager._vesaBuffer, _fontManager._font, startPosX, startPosY, curChar - 32, colour);
			startPosX += _objectsManager.getWidth(_fontManager._font, curChar - 32);
		}
	}
}

void HopkinsEngine::displayCredits() {
	loadCredits();
	_globals._creditsPosY = 436;
	_graphicsManager.loadImage("GENERIC");
	_graphicsManager.fadeInLong();
	_soundManager.playSound(28);
	_eventsManager._mouseFl = false;
	_globals.iRegul = 3;
	_globals._creditsStartX = _globals._creditsEndX = _globals._creditsStartY = _globals._creditsEndY = -1;
	int soundId = 28;
	do {
		for (int i = 0; i < _globals._creditsLineNumb; ++i) {
			if (_globals._creditsItem[i]._actvFl) {
				int nextY = _globals._creditsPosY + i * _globals._creditsStep;
				_globals._creditsItem[i]._linePosY = nextY;

				if ((nextY - 21  >= 0) && (nextY - 21 <= 418)) {
					int col = 0;
					switch (_globals._creditsItem[i]._colour) {
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
						warning("Unknown colour, default to col #1");
						col = 163;
						break;
					}
					if (_globals._creditsItem[i]._lineSize != -1)
						displayCredits(nextY, _globals._creditsItem[i]._line, col);
				}
			}
		}
		--_globals._creditsPosY;
		if (_globals._creditsStartX != -1 || _globals._creditsEndX != -1 || _globals._creditsStartY != -1 || _globals._creditsEndY != -1) {
			_eventsManager.refreshScreenAndEvents();
			_graphicsManager.copySurface(_graphicsManager._vesaScreen, 60, 50, 520, 380, _graphicsManager._vesaBuffer, 60, 50);
		} else {
			_eventsManager.refreshScreenAndEvents();
		}
		if (_globals._creditsItem[_globals._creditsLineNumb - 1]._linePosY <= 39) {
			_globals._creditsPosY = 440;
			++soundId;
			if (soundId > 31)
				soundId = 28;
			_soundManager.playSound(soundId);
		}
		_globals._creditsStartX = -1;
		_globals._creditsEndX = -1;
		_globals._creditsStartY = -1;
		_globals._creditsEndY = -1;
	} while ((_eventsManager.getMouseButton() != 1) && (!g_system->getEventManager()->shouldQuit()));
	_graphicsManager.fadeOutLong();
	_globals.iRegul = 1;
	_eventsManager._mouseFl = true;
}

void HopkinsEngine::handleOceanMouseEvents() {
	_fontManager.hideText(9);
	if (_eventsManager._mouseCursorId != 16)
		return;

	_eventsManager.getMouseX();
	if (_objectsManager._zoneNum <= 0)
		return;

	int oldPosX = _eventsManager.getMouseX();
	int oldPosY = _eventsManager.getMouseY();
	bool displAnim = false;
	int oldX;
	switch (_objectsManager._zoneNum) {
	case 1:
		switch (_globals._oceanDirection) {
		case DIR_UP:
			_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,-1,", 6, false);
			break;
		case DIR_RIGHT:
			_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,-1,", 6, false);
			break;
		default:
			break;
		}

		_globals._oceanDirection = DIR_LEFT;
		_globals._exitId = 1;
		oldX = _objectsManager.getSpriteX(0);
		for (;;) {
			if (_globals._speed == 1)
				oldX -= 2;
			else if (_globals._speed == 2)
				oldX -= 4;
			else if (_globals._speed == 3)
				oldX -= 6;
			_objectsManager.setSpriteX(0, oldX);
			setSubmarineSprites();
			_eventsManager.refreshScreenAndEvents();
			if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY) {
				displAnim = true;
				break;
			}

			if (oldX <= -100)
				break;
		}
		break;
	case 2:
		switch (_globals._oceanDirection) {
		case DIR_UP:
			_objectsManager.SPACTION(_globals.PERSO, "27,28,29,30,31,32,33,34,35,36,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsManager.SPACTION(_globals.PERSO, "9,8,7,6,5,4,3,2,1,0,-1,", 6, false);
			break;
		case DIR_LEFT:
			_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,-1,", 6, false);
			break;
		default:
			break;
		}
		_globals._oceanDirection = DIR_RIGHT;
		_globals._exitId = 2;
		oldX = _objectsManager.getSpriteX(0);
		for (;;) {
			if (_globals._speed == 1)
				oldX += 2;
			else if (_globals._speed == 2)
				oldX += 4;
			else if (_globals._speed == 3)
				oldX += 6;
			_objectsManager.setSpriteX(0, oldX);
			setSubmarineSprites();
			_eventsManager.refreshScreenAndEvents();
			if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY) {
				displAnim = true;
				break;
			}
			if (oldX > 499)
				break;
		}
		break;
	case 3:
		switch (_globals._oceanDirection) {
		case DIR_RIGHT:
			oldX = _objectsManager.getSpriteX(0);
			do {
				if (_globals._speed == 1)
					oldX += 2;
				else if (_globals._speed == 2)
					oldX += 4;
				else if (_globals._speed == 3)
					oldX += 6;
				_objectsManager.setSpriteX(0, oldX);
				setSubmarineSprites();
				_eventsManager.refreshScreenAndEvents();
				if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX <= 235);
			if (!displAnim)
				_objectsManager.SPACTION(_globals.PERSO, "36,35,34,33,32,31,30,29,28,27,-1,", 6, false);
			break;
		case DIR_DOWN:
			_objectsManager.SPACTION(_globals.PERSO, "9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,-1,", 6, false);
			break;
		case DIR_LEFT:
			oldX = _objectsManager.getSpriteX(0);
			do {
				if (_globals._speed == 1)
					oldX -= 2;
				else if (_globals._speed == 2)
					oldX -= 4;
				else if (_globals._speed == 3)
					oldX -= 6;
				_objectsManager.setSpriteX(0, oldX);
				setSubmarineSprites();
				_eventsManager.refreshScreenAndEvents();
				if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX > 236);
			if (!displAnim)
				_objectsManager.SPACTION(_globals.PERSO, "18,19,20,21,22,23,24,25,26,27,-1,", 6, false);
			break;
		default:
			break;
		}
		_globals._oceanDirection = DIR_UP;
		_globals._exitId = 3;
		break;
	case 4:
		switch (_globals._oceanDirection) {
		case DIR_UP:
			_objectsManager.SPACTION(_globals.PERSO, "27,26,25,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,-1,", 6, false);
			break;
		case DIR_RIGHT:
			oldX = _objectsManager.getSpriteX(0);
			do {
				if (_globals._speed == 1)
					oldX += 2;
				else if (_globals._speed == 2)
					oldX += 4;
				else if (_globals._speed == 3)
					oldX += 6;
				_objectsManager.setSpriteX(0, oldX);
				setSubmarineSprites();
				_eventsManager.refreshScreenAndEvents();
				if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY) {
					displAnim = true;
					break;
				}
			} while (oldX <= 235);
			if (!displAnim)
				_objectsManager.SPACTION(_globals.PERSO, "0,1,2,3,4,5,6,7,8,9,-1,", 6, false);
			break;
		case DIR_LEFT:
			oldX = _objectsManager.getSpriteX(0);
			for (;;) {
				if (_globals._speed == 1)
					oldX -= 2;
				else if (_globals._speed == 2)
					oldX -= 4;
				else if (_globals._speed == 3)
					oldX -= 6;
				_objectsManager.setSpriteX(0, oldX);
				setSubmarineSprites();
				_eventsManager.refreshScreenAndEvents();
				if (_eventsManager.getMouseButton() == 1 && oldPosX == _eventsManager.getMouseX() && _eventsManager.getMouseY() == oldPosY)
					break;

				if (oldX <= 236) {
					if (!displAnim)
						_objectsManager.SPACTION(_globals.PERSO, "18,17,16,15,14,13,12,11,10,9,-1,", 6, false);
					break;
				}
			}
			break;
		default:
			break;
		}
		_globals._oceanDirection = DIR_DOWN;
		_globals._exitId = 4;
		break;
	}
}

void HopkinsEngine::setSubmarineSprites() {
	switch (_globals._oceanDirection) {
	case DIR_UP:
		_objectsManager.setSpriteIndex(0, 27);
		break;
	case DIR_RIGHT:
		_objectsManager.setSpriteIndex(0, 0);
		break;
	case DIR_DOWN:
		_objectsManager.setSpriteIndex(0, 9);
		break;
	case DIR_LEFT:
		_objectsManager.setSpriteIndex(0, 18);
		break;
	default:
		break;
	}
}

void HopkinsEngine::handleOceanMaze(int16 curExitId, Common::String backgroundFilename, Directions defaultDirection, int16 exit1, int16 exit2, int16 exit3, int16 exit4, int16 soundId) {
	_globals._cityMapEnabledFl = false;
	_graphicsManager._noFadingFl = false;
	_globals._freezeCharacterFl = false;
	_globals._exitId = 0;
	_globals._disableInventFl = true;
	_soundManager.playSound(soundId);
	_globals.PERSO = _fileManager.loadFile("VAISSEAU.SPR");
	if (backgroundFilename.size())
		_graphicsManager.loadImage(backgroundFilename);

	if (curExitId == 77)
		_objectsManager.loadLinkFile("IM77");
	else if (curExitId == 84)
		_objectsManager.loadLinkFile("IM84");
	else if (curExitId == 91)
		_objectsManager.loadLinkFile("IM91");
	else
		_objectsManager.loadLinkFile("ocean");

	if (!exit1)
		_linesManager.disableZone(1);
	if (!exit2)
		_linesManager.disableZone(2);
	if (!exit3)
		_linesManager.disableZone(3);
	if (!exit4)
		_linesManager.disableZone(4);

	if (!_globals._oceanDirection)
		_globals._oceanDirection = defaultDirection;

	switch (_globals._oceanDirection) {
	case DIR_UP:
		_objectsManager._characterPos.x = 236;
		_objectsManager._startSpriteIndex = 27;
		break;
	case DIR_RIGHT:
		_objectsManager._characterPos.x = -20;
		_objectsManager._startSpriteIndex = 0;
		break;
	case DIR_DOWN:
		_objectsManager._characterPos.x = 236;
		_objectsManager._startSpriteIndex = 9;
		break;
	case DIR_LEFT:
		_objectsManager._characterPos.x = 415;
		_objectsManager._startSpriteIndex = 18;
		break;
	default:
		break;
	}

	_objectsManager.addStaticSprite(_globals.PERSO, Common::Point(_objectsManager._characterPos.x, 110), 0, _objectsManager._startSpriteIndex, 0, false, 0, 0);
	_graphicsManager.SETCOLOR3(252, 100, 100, 100);
	_graphicsManager.SETCOLOR3(253, 100, 100, 100);
	_graphicsManager.SETCOLOR3(251, 100, 100, 100);
	_graphicsManager.SETCOLOR3(254, 0, 0, 0);
	_objectsManager.animateSprite(0);
	_linesManager._route = (RouteItem *)g_PTRNUL;
	_eventsManager.mouseOn();
	_eventsManager.changeMouseCursor(4);

	for (int cpt = 0; cpt <= 4; cpt++)
		_eventsManager.refreshScreenAndEvents();

	if (!_graphicsManager._noFadingFl)
		_graphicsManager.fadeInLong();
	_graphicsManager._noFadingFl = false;
	_globals.iRegul = 1;

	for (;;) {
		int mouseButton = _eventsManager.getMouseButton();
		if (mouseButton && mouseButton == 1)
			handleOceanMouseEvents();
		_linesManager.checkZone();
		setSubmarineSprites();
		_eventsManager.refreshScreenAndEvents();
		if (_globals._exitId || g_system->getEventManager()->shouldQuit())
			break;
	}

	if (_globals._exitId == 1)
		_globals._exitId = exit1;
	else if (_globals._exitId == 2)
		_globals._exitId = exit2;
	else if (_globals._exitId == 3)
		_globals._exitId = exit3;
	else if (_globals._exitId == 4)
		_globals._exitId = exit4;
	_graphicsManager.fadeOutLong();
	_objectsManager.removeSprite(0);
	_objectsManager.clearScreen();
	_globals.PERSO = _fileManager.loadFile("PERSO.SPR");
	_globals._characterType = 0;
}

void HopkinsEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_soundManager.syncSoundSettings();
}

bool HopkinsEngine::displayAdultDisclaimer() {
	int xp, yp;
	int buttonIndex;

	_graphicsManager._minX = 0;
	_graphicsManager._minY = 0;
	_graphicsManager._maxX = SCREEN_WIDTH;
	_graphicsManager._maxY = SCREEN_HEIGHT - 1;
	_eventsManager._breakoutFl = false;
	_objectsManager._forestFl = false;
	_globals._disableInventFl = true;
	_globals._exitId = 0;

	_graphicsManager.loadImage("ADULT");
	_graphicsManager.fadeInLong();
	_eventsManager.mouseOn();
	_eventsManager.changeMouseCursor(0);
	_eventsManager._mouseCursorId = 0;
	_eventsManager._mouseSpriteId = 0;

	do {
		xp = _eventsManager.getMouseX();
		yp = _eventsManager.getMouseY();

		buttonIndex = 0;
		if (xp >= 37 && xp <= 169 && yp >= 406 && yp <= 445)
			buttonIndex = 2;
		else if (xp >= 424 && xp <= 602 && yp >= 406 && yp <= 445)
			buttonIndex = 1;

		_eventsManager.refreshScreenAndEvents();
	} while (!shouldQuit() && (buttonIndex == 0 || _eventsManager.getMouseButton() != 1));

	_globals._disableInventFl = false;
	_graphicsManager.fadeOutLong();

	if (buttonIndex != 2) {
		// Quit game
		return false;
	} else {
		// Continue
		_graphicsManager._minX = 0;
		_graphicsManager._maxY = 20;
		_graphicsManager._maxX = SCREEN_WIDTH;
		_graphicsManager._maxY = SCREEN_HEIGHT - 20;
		return true;
	}
}

} // End of namespace Hopkins
