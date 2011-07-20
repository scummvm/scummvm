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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug-channels.h"
#include "engines/util.h"

#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/resources.h"
#include "tsage/globals.h"

namespace tSage {

TSageEngine *_vm = NULL;

TSageEngine::TSageEngine(OSystem *system, const tSageGameDescription *gameDesc) : Engine(system),
		_gameDescription(gameDesc) {
	_vm = this;
	DebugMan.addDebugChannel(kRingDebugScripts, "scripts", "Scripts debugging");
	_debugger = new Debugger();
}

Common::Error TSageEngine::init() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT, false);

	return Common::kNoError;
}

TSageEngine::~TSageEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
	delete _debugger;
}

bool TSageEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void TSageEngine::initialize() {
	_saver = new Saver();

	// Set up the resource manager
	_resourceManager = new ResourceManager();
	if (_vm->getFeatures() & GF_DEMO) {
		// Add the single library file associated with the demo
		_resourceManager->addLib(getPrimaryFilename());
	} else if (_vm->getGameID() == GType_Ringworld) {
		_resourceManager->addLib("RING.RLB");
		_resourceManager->addLib("TSAGE.RLB");
	} else if (_vm->getGameID() == GType_BlueForce) {
		_resourceManager->addLib("BLUE.RLB");
		if (_vm->getFeatures() & GF_FLOPPY) {
			_resourceManager->addLib("FILES.RLB");
			_resourceManager->addLib("TSAGE.RLB");
		}
	}

	_globals = new Globals();
	_globals->gfxManager().setDefaults();

	// Setup sound settings
	syncSoundSettings();
}

void TSageEngine::deinitialize() {
	delete _globals;
	delete _resourceManager;
	delete _saver;
	_resourceManager = NULL;
	_saver = NULL;
}

Common::Error TSageEngine::run() {
	// Basic initialisation
	initialize();

	_globals->_sceneHandler.registerHandler();
	_globals->_game->execute();

	deinitialize();
	return Common::kNoError;
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool TSageEngine::canLoadGameStateCurrently() {
	return (_globals->getFlag(50) == 0);
}

/**
 * Returns true if it is currently okay to save the game
 */
bool TSageEngine::canSaveGameStateCurrently() {
	return (_globals->getFlag(50) == 0);
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error TSageEngine::loadGameState(int slot) {
	return _saver->restore(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error TSageEngine::saveGameState(int slot, const Common::String &desc) {
	return _saver->save(slot, desc);
}

/**
 * Support method that generates a savegame name
 * @param slot		Slot number
 */
Common::String TSageEngine::generateSaveName(int slot) {
	return Common::String::format("%s.%03d", _targetName.c_str(), slot);
}

void TSageEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_globals->_soundManager.syncSounds();
}

bool TSageEngine::shouldQuit() {
	return getEventManager()->shouldQuit() || getEventManager()->shouldRTL();
}

} // End of namespace tSage
