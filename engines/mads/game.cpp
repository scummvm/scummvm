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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "graphics/palette.h"
#include "graphics/scaler.h"
#include "graphics/thumbnail.h"
#include "mads/mads.h"
#include "mads/compression.h"
#include "mads/game.h"
#include "mads/game_data.h"
#include "mads/events.h"
#include "mads/screen.h"
#include "mads/msurface.h"
#include "mads/resources.h"
#include "mads/dragonsphere/game_dragonsphere.h"
#include "mads/nebular/game_nebular.h"
#include "mads/phantom/game_phantom.h"

namespace MADS {

Game *Game::init(MADSEngine *vm) {
	switch (vm->getGameID()) {
	case GType_RexNebular:
		return new Nebular::GameNebular(vm);
	case GType_Dragonsphere:
		return new Dragonsphere::GameDragonsphere(vm);
	case GType_Phantom:
		return new Phantom::GamePhantom(vm);
	default:
		error("Game: Unknown game");
	}

	return nullptr;
}

Game::Game(MADSEngine *vm)
	: _vm(vm), _surface(nullptr), _objects(vm), _scene(vm), _screenObjects(vm), _player(vm), _camX(vm), _camY(vm) {
	_sectionNumber = 1;
	_priorSectionNumber = 0;
	_loadGameSlot = -1;
	_lastSave = -1;
	_saveFile = nullptr;
	_saveThumb = nullptr;
	_statusFlag = 0;
	_sectionHandler = nullptr;
	_sectionNumber = 1;
	_priorSectionNumber = 0;
	_currentSectionNumber = -1;
	_kernelMode = KERNEL_GAME_LOAD;
	_quoteEmergency = false;
	_vocabEmergency = false;
	_aaName = "*I0.AA";
	_priorFrameTimer = 0;
	_anyEmergency = false;
	_triggerMode = SEQUENCE_TRIGGER_PARSER;
	_triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
	_trigger = 0;
	_winStatus = 0;
	_widepipeCtr = 0;
	_fx = kTransitionNone;
	_panningSpeed = 1; // Medium speed

	// Load the inventory object list
	_objects.load();
	if (_objects._inventoryList.size() > 0)
		// At least one item in default inventory, so select first item for display
		_scene._userInterface._selectedInvIndex = 0;

	// Load the quotes
	loadQuotes();
}

Game::~Game() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}

	delete _saveFile;
	_surface->free();
	delete _surface;
	delete _sectionHandler;
}

void Game::run() {
	// If requested, load a savegame instead of showing the intro
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999)
			_loadGameSlot = saveSlot;
	}

	_statusFlag = true;

	while (!_vm->shouldQuit()) {
		if (_loadGameSlot == -1) {
			startGame();
		}

		// Get the initial starting time for the first scene
		_scene._frameStartTime = _vm->_events->getFrameCounter();

		if (!_vm->shouldQuit())
			gameLoop();
	}
}

void Game::splitQuote(const Common::String &source, Common::String &line1, Common::String &line2) {
	// Make the first line up the end of the word at the half-way point
	const char *strP = source.c_str() + source.size() / 2;
	while (*strP != ' ') ++strP;

	line1 = Common::String(source.c_str(), strP);

	// The rest of the string goes in the second line
	while (*strP == ' ') ++strP;
	line2 = Common::String(strP);
}

void Game::gameLoop() {
	while (!_vm->shouldQuit() && _statusFlag && !_winStatus) {
		if (_loadGameSlot != -1) {
			loadGame(_loadGameSlot);
			_loadGameSlot = -1;
		}

		setSectionHandler();
		_sectionHandler->preLoadSection();
		initSection(_sectionNumber);
		_vm->_sound->init(_sectionNumber);
		_sectionHandler->postLoadSection();

		_scene._spriteSlots.reset();

		if (_sectionNumber == _currentSectionNumber)
			sectionLoop();

		_player.releasePlayerSprites();
		assert(_scene._sprites.size() == 0);

		_vm->_palette->unlock();
		_vm->_events->waitCursor();
		_vm->_events->freeCursors();
		_vm->_sound->closeDriver();
	}
}

void Game::sectionLoop() {
	while (!_vm->shouldQuit() && _statusFlag && !_winStatus &&
			(_sectionNumber == _currentSectionNumber)) {
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
		_screenObjects._forceRescan = true;
		_screenObjects._inputMode = kInputBuildingSentences;
		_scene._userInterface._scrollbarActive = SCROLLBAR_NONE;

		_player._loadsFirst = true;

		_scene._sceneLogic->setup();
		if (_player._spritesChanged || _player._loadsFirst) {
			if (_player._spritesLoaded)
				_player.releasePlayerSprites();
			_vm->_palette->resetGamePalette(18, 10);
			_scene._spriteSlots.reset();
		} else {
			_vm->_palette->initPalette();
		}

		// Set up scene palette usage
		_scene._scenePaletteUsage.clear();
		_scene._scenePaletteUsage.push_back(PaletteUsage::UsageEntry(0xF0));
		_scene._scenePaletteUsage.push_back(PaletteUsage::UsageEntry(0xF1));
		_scene._scenePaletteUsage.push_back(PaletteUsage::UsageEntry(0xF2));
		_vm->_palette->_paletteUsage.load(&_scene._scenePaletteUsage);

		if (!_player._spritesLoaded && _player._loadsFirst) {
			if (_player.loadSprites(""))
				_vm->quitGame();
			_player._loadedFirst = true;
		}

		_scene.loadScene(_scene._nextSceneId, _aaName, 0);
		camInitDefault();
		camSetSpeed();


		_vm->_sound->pauseNewCommands();

		if (!_player._spritesLoaded) {
			if (_player.loadSprites(""))
				_vm->quitGame();
			_player._loadedFirst = false;
		}

		_vm->_events->initVars();
		_scene._userInterface._highlightedCommandIndex = -1;
		_scene._userInterface._highlightedInvIndex = -1;
		_scene._userInterface._highlightedItemVocabIndex = -1;

		_scene._action.clear();
		_player.setFinalFacing();
		_player._facing = _player._turnToFacing;
		_player.cancelCommand();
		_kernelMode = KERNEL_ROOM_INIT;

		switch (_vm->_screenFade) {
		case SCREEN_FADE_SMOOTH:
			_fx = kTransitionFadeOutIn;
			break;
		case SCREEN_FADE_FAST:
			_fx = kNullPaletteCopy;
			break;
		default:
			_fx = kTransitionNone;
			break;
		}

		_trigger = 0;
		_priorFrameTimer = _scene._frameStartTime;

		// If in the middle of restoring a game, handle the rest of the loading
		if (_saveFile != nullptr) {
			Common::Serializer s(_saveFile, nullptr);
			synchronize(s, false);
			delete _saveFile;
			_saveFile = nullptr;
		}

		// Call the scene logic for entering the given scene
		_triggerSetupMode = SEQUENCE_TRIGGER_DAEMON;
		_scene._sceneLogic->enter();

		// Set player data
		_player._targetPos = _player._playerPos;
		_player._turnToFacing = _player._facing;
		_player._targetFacing = _player._facing;
		_player.selectSeries();
		_player.updateFrame();

		_player._beenVisible = _player._visible;
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

		if ((_quoteEmergency || _vocabEmergency) && !_anyEmergency) {
			_scene._currentSceneId = _scene._priorSceneId;
			_anyEmergency = true;
		} else {
			_anyEmergency = false;
			_scene.loop();
		}

		_vm->_events->waitCursor();
		_kernelMode = KERNEL_ROOM_PRELOAD;

		for (int i = 0; i < 10; i++) {
			delete _scene._animation[i];
			_scene._animation[i] = nullptr;
		}

		_scene._reloadSceneFlag = false;

		_scene._userInterface.noInventoryAnim();
		_scene.removeSprites();

		if (!_player._loadedFirst) {
			_player._spritesLoaded = false;
			_player._spritesChanged = true;
		}

		// Clear the scene
		_scene.freeCurrentScene();
		_sectionNumber = _scene._nextSceneId / 100;

		// Check whether to show a dialog
		checkShowDialog();
	}
}

void Game::initSection(int sectionNumber) {
	_priorSectionNumber = _currentSectionNumber;
	_currentSectionNumber = sectionNumber;

	_vm->_palette->resetGamePalette(18, 10);
	_vm->_palette->setLowRange();

	if (_scene._mode == SCREENMODE_VGA)
		_vm->_palette->setPalette(_vm->_palette->_mainPalette, 0, 4);

	_vm->_events->loadCursors("*CURSOR.SS");

	assert(_vm->_events->_cursorSprites);
	_vm->_events->setCursor2((_vm->_events->_cursorSprites->getCount() <= 1) ?
		CURSOR_ARROW : CURSOR_WAIT);
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

Common::StringArray Game::getMessage(uint32 id) {
	File f("*MESSAGES.DAT");
	int count = f.readUint16LE();

	for (int idx = 0; idx < count; ++idx) {
		uint32 itemId = f.readUint32LE();
		uint32 offset = f.readUint32LE();
		uint16 size = f.readUint16LE();

		if (itemId == id) {
			// Get the source buffer size
			uint16 sizeIn;
			if (idx == (count - 1)) {
				sizeIn = f.size() - offset;
			} else {
				f.skip(4);
				uint32 nextOffset = f.readUint32LE();
				sizeIn = nextOffset - offset;
			}

			// Get the compressed data
			f.seek(offset);
			byte *bufferIn = new byte[sizeIn];
			f.read(bufferIn, sizeIn);

			// Decompress it
			char *bufferOut = new char[size];
			FabDecompressor fab;
			fab.decompress(bufferIn, sizeIn, (byte *)bufferOut, size);

			// Form the output string list
			Common::StringArray result;
			const char *p = bufferOut;
			while (p < (bufferOut + size)) {
				result.push_back(p);
				p += strlen(p) + 1;
			}

			delete[] bufferIn;
			delete[] bufferOut;
			return result;
		}
	}

	error("Invalid message Id specified");
}

static const char *const DEBUG_STRING = "WIDEPIPE";

void Game::handleKeypress(const Common::KeyState &kbd) {
	if (kbd.flags & Common::KBD_CTRL) {
		if (_widepipeCtr == 8) {
			// Implement original game cheating keys here someday
		} else {
			if (kbd.keycode == (Common::KEYCODE_a +
					(DEBUG_STRING[_widepipeCtr] - 'a'))) {
				if (++_widepipeCtr == 8) {
					MessageDialog *dlg = new MessageDialog(_vm, 2,
						"CHEATING ENABLED", "(for your convenience).");
					dlg->show();
					delete dlg;
				}
			}
		}
	}

	Scene &scene = _vm->_game->_scene;
	switch (kbd.keycode) {
	case Common::KEYCODE_F1:
		_vm->_dialogs->_pendingDialog = DIALOG_GAME_MENU;
		break;
	case Common::KEYCODE_F5:
		_vm->_dialogs->_pendingDialog = DIALOG_SAVE;
		break;
	case Common::KEYCODE_F7:
		_vm->_dialogs->_pendingDialog = DIALOG_RESTORE;
		break;
	case Common::KEYCODE_PAGEUP:
		scene._userInterface._scrollbarStrokeType = SCROLLBAR_UP;
		scene._userInterface.changeScrollBar();
		break;
	case Common::KEYCODE_PAGEDOWN:
		scene._userInterface._scrollbarStrokeType = SCROLLBAR_DOWN;
		scene._userInterface.changeScrollBar();
		break;


	default:
		break;
	}
}

void Game::synchronize(Common::Serializer &s, bool phase1) {
	if (phase1) {
		s.syncAsSint16LE(_fx);
		s.syncAsSint16LE(_trigger);
		s.syncAsUint16LE(_triggerSetupMode);
		s.syncAsUint16LE(_triggerMode);
		s.syncString(_aaName);
		s.syncAsSint16LE(_lastSave);

		_scene.synchronize(s);
		_objects.synchronize(s);
		_visitedScenes.synchronize(s, _scene._nextSceneId);
		_player.synchronize(s);
		_screenObjects.synchronize(s);
	} else {
		// Load scene specific data for the loaded scene
		_scene._sceneLogic->synchronize(s);
	}
}

void Game::loadGame(int slotNumber) {
	_saveFile = g_system->getSavefileManager()->openForLoading(
		_vm->getSaveStateName(slotNumber));

	Common::Serializer s(_saveFile, nullptr);

	// Load the savaegame header
	MADSSavegameHeader header;
	if (!readSavegameHeader(_saveFile, header))
		error("Invalid savegame");

	// Load most of the savegame data with the exception of scene specific info
	synchronize(s, true);

	// Set up section/scene and other initial states for post-load
	_currentSectionNumber = -2;
	_scene._currentSceneId = -2;
	_sectionNumber = _scene._nextSceneId / 100;
	_scene._frameStartTime = _vm->_events->getFrameCounter();
	_vm->_screen->_shakeCountdown = -1;

	// Default the selected inventory item to the first one, if the player has any
	_scene._userInterface._selectedInvIndex = _objects._inventoryList.size() > 0 ? 0 : -1;

	// Set player sprites sets flags
	_player._spritesLoaded = false;
	_player._spritesChanged = true;
}

void Game::saveGame(int slotNumber, const Common::String &saveName) {
	Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving(
		_vm->getSaveStateName(slotNumber));

	MADSSavegameHeader header;
	header._saveName = saveName;
	writeSavegameHeader(out, header);

	Common::Serializer s(nullptr, out);
	synchronize(s, true);
	synchronize(s, false);

	out->finalize();
	delete out;
}

const char *const SAVEGAME_STR = "MADS";
#define SAVEGAME_STR_SIZE 4

WARN_UNUSED_RESULT bool Game::readSavegameHeader(Common::InSaveFile *in, MADSSavegameHeader &header, bool skipThumbnail) {
	char saveIdentBuffer[SAVEGAME_STR_SIZE + 1];

	// Validate the header Id
	in->read(saveIdentBuffer, SAVEGAME_STR_SIZE + 1);
	if (strncmp(saveIdentBuffer, SAVEGAME_STR, SAVEGAME_STR_SIZE))
		return false;

	header._version = in->readByte();
	if (header._version > MADS_SAVEGAME_VERSION)
		return false;

	// Read in the string
	header._saveName.clear();
	char ch;
	while ((ch = (char)in->readByte()) != '\0') header._saveName += ch;

	// Get the thumbnail
	if (!Graphics::loadThumbnail(*in, header._thumbnail, skipThumbnail)) {
		return false;
	}

	// Read in save date/time
	header._year = in->readSint16LE();
	header._month = in->readSint16LE();
	header._day = in->readSint16LE();
	header._hour = in->readSint16LE();
	header._minute = in->readSint16LE();
	header._totalFrames = in->readUint32LE();

	return true;
}

void Game::writeSavegameHeader(Common::OutSaveFile *out, MADSSavegameHeader &header) {
	// Write out a savegame header
	out->write(SAVEGAME_STR, SAVEGAME_STR_SIZE + 1);

	out->writeByte(MADS_SAVEGAME_VERSION);

	// Write savegame name
	out->write(header._saveName.c_str(), header._saveName.size());
	out->writeByte('\0');

	// Handle the thumbnail. If there's already one set by the game, create one
	if (!_saveThumb)
		createThumbnail();
	Graphics::saveThumbnail(*out, *_saveThumb);

	_saveThumb->free();
	delete _saveThumb;
	_saveThumb = nullptr;

	// Write out the save date/time
	TimeDate td;
	g_system->getTimeAndDate(td);
	out->writeSint16LE(td.tm_year + 1900);
	out->writeSint16LE(td.tm_mon + 1);
	out->writeSint16LE(td.tm_mday);
	out->writeSint16LE(td.tm_hour);
	out->writeSint16LE(td.tm_min);
	out->writeUint32LE(_vm->_events->getFrameCounter());
}

void Game::createThumbnail() {
	if (_saveThumb) {
		_saveThumb->free();
		delete _saveThumb;
	}

	uint8 thumbPalette[PALETTE_SIZE];
	_vm->_palette->grabPalette(thumbPalette, 0, PALETTE_COUNT);
	_saveThumb = new Graphics::Surface();
	::createThumbnail(_saveThumb, (const byte *)_vm->_screen->getPixels(),
		MADS_SCREEN_WIDTH, MADS_SCREEN_HEIGHT, thumbPalette);
}

void Game::syncTimers(SyncType slaveType, int slaveId, SyncType masterType, int masterId) {
	uint32 syncTime = 0;

	switch (masterType) {
	case SYNC_SEQ:
		syncTime = _scene._sequences[masterId]._timeout;
		break;

	case SYNC_ANIM:
		syncTime = _scene._animation[masterId]->getNextFrameTimer();
		break;

	case SYNC_CLOCK:
		syncTime = _scene._frameStartTime + masterId;
		break;

	case SYNC_PLAYER:
		syncTime = _player._priorTimer;
		break;

	default:
		break;
	}


	switch (slaveType) {
	case SYNC_SEQ:
		_scene._sequences[slaveId]._timeout = syncTime;
		break;

	case SYNC_PLAYER:
		_player._priorTimer = syncTime;
		break;

	case SYNC_ANIM:
		_scene._animation[slaveId]->setNextFrameTimer(syncTime);
		break;

	case SYNC_CLOCK:
		error("syncTimer is trying to force _frameStartTime");
		break;

	default:
		break;
	}
}

void Game::camInitDefault() {
	_camX.setDefaultPanX();
	_camY.setDefaultPanY();
}

void Game::camSetSpeed() {
	switch (_panningSpeed) {
	case 1:
		_camX._speed = 8;
		_camY._speed = 4;
		break;

	case 2:
		_camX._speed = 320;
		_camY._speed = 160;
		break;

	default:
		_camX._speed = 4;
		_camY._speed = 2;
		break;
	}
}

void Game::camUpdate() {
	bool any_pan = _camX.camPan(&_scene._posAdjust.x, &_player._playerPos.x, 320, _scene._sceneInfo->_width);
	any_pan |= _camY.camPan(&_scene._posAdjust.y, &_player._playerPos.y, 156, _scene._sceneInfo->_height);

	if (any_pan) {
		_scene.setCamera(_scene._posAdjust);
		_screenObjects._forceRescan = true;
	}
}

} // End of namespace MADS
