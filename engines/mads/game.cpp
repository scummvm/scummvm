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
#include "mads/nebular/game_nebular.h"
#include "mads/graphics.h"
#include "mads/msurface.h"
#include "mads/resources.h"

namespace MADS {

Game *Game::init(MADSEngine *vm) {
	if (vm->getGameID() == GType_RexNebular)
		return new Nebular::GameNebular(vm);

	return nullptr;
}

Game::Game(MADSEngine *vm): _vm(vm), _surface(nullptr), _scene(vm) {
	_sectionNumber = _priorSectionNumber = 0;
	_difficultyLevel = DIFFICULTY_HARD;
	_saveSlot = -1;
	_statusFlag = 0;
	_sectionHandler = nullptr;
	_sectionNumber = 1;
	_priorSectionNumber = 0;
	_currentSectionNumber = -1;
	_v1 = _v2 = 0;
	_v3 = _v4 = 0;
	_v5 = _v6 = 0;
	_aaName = "*I0.AA";
	_playerSpritesFlag = false;
}

Game::~Game() {
	delete _surface;
	delete _sectionHandler;
}

void Game::run() {
	_statusFlag = true;
	int protectionResult = checkCopyProtection();
	switch (protectionResult) {
	case 1:
		// Copy protection failed
		_scene._nextSceneId = 804;
		initialiseGlobals();
		_globalFlags[5] = 0xFFFF;
		_saveSlot = -1;
		break;
	case 2:
		_statusFlag = 0;
		break;
	default:
		break;
	}

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

		_scene.clearSprites(true);

		if (_sectionNumber == _currentSectionNumber) {
			sectionLoop();
		}

		// TODO: Extra reset methods
		_vm->_events->resetCursor();
		_vm->_events->freeCursors();
		_vm->_sound->closeDriver();
	}

	_vm->_palette->close();
}

void Game::sectionLoop() {
	while (!_vm->shouldQuit() && _statusFlag && _sectionNumber == _currentSectionNumber) {
		_v1 = 3;
		_player._spritesChanged = true;
		_v5 = 0;
		_v6 = 0;
		_vm->_events->resetCursor();

		_quotes = nullptr;
		_scene.clearVocab();
		_scene.loadSceneLogic();

		_v4 = 0;
		_player._stepEnabled = true;
		_player._visible = true;
		_vm->_dialogs->_defaultPosition = Common::Point(-1, -1);
		addVisitedScene(_scene._nextSceneId);

		_scene._screenObjects._v8333C = -1;
		_scene._screenObjects._v832EC = 0;
		_scene._screenObjects._yp = 0;
		_v3 = -1;

		_scene._sceneLogic->setup();
		if (_player._spritesChanged || _v3) {
			if (_player._spritesLoaded)
				_scene.releasePlayerSprites();
			_vm->_palette->resetGamePalette(18, 10);
			_scene.clearSprites(true);
		} else {
			_vm->_palette->initGamePalette();
		}

		// TODO: Further palette init
		
		//_scene.loadScene(_aaName, _scene._nextSceneId, 0);
		_vm->_sound->queueNewCommands();
		if (!_player._spritesLoaded) {
			_player.loadSprites("");
			_playerSpritesFlag = false;
		}


		// TODO: main section loop logic goes here

		// Clear the scene
		_scene.free();
		_sectionNumber = _scene._nextSceneId / 100;

		// TODO: sub_1DD46(3)

		// Check whether to show a dialog
		if (_vm->_dialogs->_pendingDialog && _player._stepEnabled && !_globalFlags[5]) {
			_scene.releasePlayerSprites();
			_vm->_dialogs->showDialog();
			_vm->_dialogs->_pendingDialog = DIALOG_NONE;
		}
	}
}

void Game::initSection(int sectionNumber) {
	_priorSectionNumber = _currentSectionNumber;
	_currentSectionNumber = sectionNumber;

	_vm->_palette->resetGamePalette(18, 10);
	_vm->_palette->setLowRange();
	_vm->_events->loadCursors("*CURSOR.SS");
	
	assert(_vm->_events->_cursorSprites);
	_vm->_events->setCursor2((_vm->_events->_cursorSprites->getCount() <= 1) ? 
		CURSOR_ARROW : CURSOR_WAIT);
}

void Game::loadObjects() {
	File f("*OBJECTS.DAT");

	// Get the total numer of inventory objects
	int count = f.readUint16LE();
	_objects.reserve(count);

	// Read in each object
	for (int i = 0; i < count; ++i) {
		InventoryObject obj;
		obj.load(f);
		_objects.push_back(obj);

		// If it's for the player's inventory, add the index to the inventory list
		if (obj._roomNumber == PLAYER_INVENTORY) {
			_inventoryList.push_back(i);
			assert(_inventoryList.size() <= 32);
		}
	}
}

void Game::setObjectData(int objIndex, int id, const byte *p) {
	// TODO: This whole method seems weird. Check it out more thoroughly once
	// more of the engine is implemented
	for (int i = 0; i < (int)_objects.size(); ++i) {
		InventoryObject &obj = _objects[i];
		if (obj._vocabList[0]._actionFlags1 <= i)
			break;

		if (obj._mutilateString[6 + i] == id) {
			_objects[objIndex]._objFolder = p;
		}
	}
}

void Game::setObjectRoom(int objectId, int roomNumber) {
	warning("TODO: setObjectRoom");
}

void Game::loadResourceSequence(const Common::String prefix, int v) {
	warning("TODO: loadResourceSequence");
}

void Game::addVisitedScene(int sceneId) {
	if (!visitedScenesExists(sceneId))
		_visitedScenes.push_back(sceneId);
}

bool Game::visitedScenesExists(int sceneId) {
	for (uint i = 0; i < _visitedScenes.size(); ++i) {
		if (_visitedScenes[i] == sceneId)
			return true;
	}

	return false;
}

/*------------------------------------------------------------------------*/

void InventoryObject::load(Common::SeekableReadStream &f) {
	_descId = f.readUint16LE();
	_roomNumber = f.readUint16LE();
	_article = f.readByte();
	_vocabCount = f.readByte();
	
	for (int i = 0; i < 3; ++i) {
		_vocabList[i]._actionFlags1 = f.readByte();
		_vocabList[i]._actionFlags2 = f.readByte();
		_vocabList[i]._vocabId = f.readByte();
	}

	f.skip(4);	// field12
	f.read(&_mutilateString[0], 10);
	f.skip(16);
}

/*------------------------------------------------------------------------*/

Player::Player() {
	_direction = 8;
	_newDirection = 8;
	_spritesLoaded = false;
	_spriteListStart = _numSprites = 0;
	_stepEnabled = false;
	_visible = false;
}

} // End of namespace MADS
