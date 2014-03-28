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

#include "common/scummsys.h"
#include "mads/mads.h"
#include "mads/game.h"
#include "mads/game_data.h"
#include "mads/events.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/nebular/game_nebular.h"

namespace MADS {

Game *Game::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular)
		return new Nebular::GameNebular(vm);

	return nullptr;
}

Game::Game(MADSEngine *vm): _vm(vm), _surface(nullptr), _objects(vm), 
		_scene(vm), _screenObjects(vm), _player(vm) {
	_sectionNumber = _priorSectionNumber = 0;
	_difficulty = DIFFICULTY_HARD;
	_saveSlot = -1;
	_statusFlag = 0;
	_sectionHandler = nullptr;
	_sectionNumber = 1;
	_priorSectionNumber = 0;
	_currentSectionNumber = -1;
	_kernelMode = KERNEL_GAME_LOAD;
	_v2 = 0;
	_quoteEmergency = false;
	_vocabEmergency = false;
	_aaName = "*I0.AA";
	_playerSpritesFlag = false;
	_priorFrameTimer = 0;
	_updateSceneFlag = false;
	_abortTimersMode = ABORTMODE_0;
	_abortTimersMode2 = ABORTMODE_0;
	_ticksExpiry = 0;
	_winStatus = 0;

	// Load the inventory object list
	_objects.load();
	if (_objects._inventoryList.size() > 0)
		// At least one item in default inventory, so select first item for display
		_scene._userInterface._selectedInvIndex = 0;

	// Load the quotes
	loadQuotes();
}

Game::~Game() {
	delete _surface;
	delete _sectionHandler;
}

void Game::run() {
	initialiseGlobals();

	_statusFlag = true;
	int protectionResult = checkCopyProtection();
	switch (protectionResult) {
	case PROTECTION_FAIL:
		// Copy protection failed
		_scene._nextSceneId = 804;
		_saveSlot = -1;
		break;
	case PROTECTION_ESCAPE:
		// User escaped out of copy protection dialog
		_vm->quitGame();
		break;
	default:
		// Copy protection check succeeded
		_scene._nextSceneId = 103;
		_scene._priorSceneId = 102;
		break;
	}

	// Get the initial starting time for the first scene
	_scene._frameStartTime = _vm->_events->getFrameCounter();

	if (_saveSlot == -1 && protectionResult != -1 && protectionResult != -2) {
		initSection(_sectionNumber);
		_statusFlag = true;

		_vm->_dialogs->_pendingDialog = DIALOG_DIFFICULTY;
		_vm->_dialogs->showDialog();
		_vm->_dialogs->_pendingDialog = DIALOG_NONE;

		_priorSectionNumber = 0;
		_priorSectionNumber = -1;
		_scene._priorSceneId = 0;
		_scene._currentSceneId = -1;
	}

	if (protectionResult != 1 && protectionResult != 2) {
		initialiseGlobals();

		if (_saveSlot != -1) {
			warning("TODO: loadGame(\"REX.SAV\", 210)");
			_statusFlag = false;
		}
	}

	if (_statusFlag)
		gameLoop();
}

void Game::gameLoop() {
	while (!_vm->shouldQuit() && _statusFlag) {
		setSectionHandler();
		_sectionHandler->preLoadSection();
		initSection(_sectionNumber);
		_sectionHandler->postLoadSection();

		_scene._spriteSlots.reset();

		if (_sectionNumber == _currentSectionNumber) {
			sectionLoop();
		}

		// TODO: Extra reset methods
		_vm->_events->waitCursor();
		_vm->_events->freeCursors();
		_vm->_sound->closeDriver();
	}

	_vm->_palette->close();
}

void Game::sectionLoop() {
	while (!_vm->shouldQuit() && _statusFlag && _sectionNumber == _currentSectionNumber) {
		_kernelMode = KERNEL_ROOM_PRELOAD;
		_player._spritesChanged = true;
		_quoteEmergency = false;
		_vocabEmergency = false;
		_vm->_events->waitCursor();

		_scene.clearVocab();
		_scene._dynamicHotspots.clear();
		_scene.loadSceneLogic();

		_player._walkAnywhere = false;
		_player._stepEnabled = true;
		_player._visible = true;
		_vm->_dialogs->_defaultPosition = Common::Point(-1, -1);
		_visitedScenes.add(_scene._nextSceneId);

		// Reset the user interface
		_screenObjects._v8333C = true;
		_screenObjects._v832EC = 0;
		_scene._userInterface._scrollerY = 0;
		
		_player._loadsFirst = true;

		_scene._sceneLogic->setup();
		if (_player._spritesChanged || _player._loadsFirst) {
			if (_player._spritesLoaded)
				_scene._spriteSlots.releasePlayerSprites();
			_vm->_palette->resetGamePalette(18, 10);
			_scene._spriteSlots.reset();
		} else {
			_vm->_palette->initPalette();
		}

		_vm->_palette->_paletteUsage.load(3, 0xF0, 0xF1, 0xF2);
		
		if (!_player._spritesLoaded && _player._loadsFirst) {
			if (_player.loadSprites(""))
				_vm->quitGame();
			_playerSpritesFlag = true;
		}

		_scene.loadScene(_scene._nextSceneId, _aaName, 0);
		_vm->_sound->pauseNewCommands();

		if (!_player._spritesLoaded) {
			if (_player.loadSprites(""))
				_vm->quitGame();
			_playerSpritesFlag = false;
		}

		_vm->_events->initVars();
		_scene._userInterface._highlightedActionIndex = -1;
		_scene._userInterface._v1C = -1;
		_scene._userInterface._v1E = -1;

		_scene._action.clear();
		_player.turnToDestFacing();
		_player._facing = _player._turnToFacing;
		_player.moveComplete();

		switch (_vm->_screenFade) {
		case SCREEN_FADE_SMOOTH:
			_abortTimers2 = kTransitionFadeOutIn;
			break;
		case SCREEN_FADE_FAST:
			_abortTimers2 = kCenterVertTransition;
			break;
		default:
			_abortTimers2 = kTransitionNone;
			break;
		}

		_abortTimers = 0;
		_abortTimersMode2 = ABORTMODE_1;
		_priorFrameTimer = _scene._frameStartTime;

		// Call the scene logic for entering the given scene
		_scene._sceneLogic->enter();

		// Set player data
		_player._destPos = _player._playerPos;
		_player._turnToFacing = _player._facing;
		_player._targetFacing = _player._facing;
		_player.setupFrame();
		_player.updateFrame();
		_player._visible3 = _player._visible;
		_player._special = _scene.getDepthHighBits(_player._playerPos);
		_player._priorTimer = _scene._frameStartTime - _player._ticksAmount;
		_player.idle();

		if (_scene._userInterface._selectedInvIndex >= 0) {
			_scene._userInterface.loadInventoryAnim(
				_objects._inventoryList[_scene._userInterface._selectedInvIndex]);
		} else {
			_scene._userInterface.noInventoryAnim();
		}

		_kernelMode = KERNEL_ACTIVE_CODE;
		_scene._roomChanged = false;

		if ((_quoteEmergency || _vocabEmergency) && !_updateSceneFlag) {
			_scene._currentSceneId = _scene._priorSceneId;
			_updateSceneFlag = true;
		} else {
			_updateSceneFlag = false;
			_scene.loop();
		}

		_vm->_events->waitCursor();
		_kernelMode = KERNEL_ROOM_PRELOAD;

		delete _scene._animationData;
		_scene._animationData = nullptr;

		_scene._reloadSceneFlag = false;

		warning("TODO: sub_1DD8C, sub_1DD7E");

		if (!_playerSpritesFlag) {
			_player._spritesLoaded = false;
			_player._spritesChanged = true;
		}

		// Clear the scene
		_scene.free();
		_sectionNumber = _scene._nextSceneId / 100;

		// TODO: sub_1DD46(3)

		// Check whether to show a dialog
		checkShowDialog();
	}
}

void Game::initSection(int sectionNumber) {
	_priorSectionNumber = _currentSectionNumber;
	_currentSectionNumber = sectionNumber;

	_vm->_palette->resetGamePalette(18, 10);
	_vm->_palette->setLowRange();

	if (_scene._layer == LAYER_GUI)
		_vm->_palette->setPalette(_vm->_palette->_mainPalette, 0, 4);

	_vm->_events->loadCursors("*CURSOR.SS");
	
	assert(_vm->_events->_cursorSprites);
	_vm->_events->setCursor2((_vm->_events->_cursorSprites->getCount() <= 1) ? 
		CURSOR_ARROW : CURSOR_WAIT);
}

void Game::loadResourceSequence(const Common::String prefix, int v) {
	warning("TODO: loadResourceSequence");
}

void Game::loadQuotes() {
	File f("*QUOTES.DAT");

	Common::String msg;
	while (true) {
		uint8 b = f.readByte();

		msg += b;
		if (f.eos() || b == '\0') {
			// end of string, add it to the strings list
			_quotes.push_back(msg);
			msg = "";
		}

		if (f.eos()) break;
	}

	f.close();
}

} // End of namespace MADS
