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

#include "common/debug-channels.h"
#include "engines/util.h"

#include "tsage/tsage.h"
#include "tsage/core.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/resources.h"
#include "tsage/globals.h"

namespace TsAGE {

TSageEngine *g_vm = NULL;

TSageEngine::TSageEngine(OSystem *system, const tSageGameDescription *gameDesc) : Engine(system),
		_gameDescription(gameDesc) {
	g_vm = this;
	DebugMan.addDebugChannel(kRingDebugScripts, "scripts", "Scripts debugging");

	if (g_vm->getGameID() == GType_Ringworld) {
		if (g_vm->getFeatures() & GF_DEMO)
			setDebugger(new DemoDebugger());
		else
			setDebugger(new RingworldDebugger());
	} else if (g_vm->getGameID() == GType_BlueForce)
		setDebugger(new BlueForceDebugger());
	else if (g_vm->getGameID() == GType_Ringworld2)
		setDebugger(new Ringworld2Debugger());
	else if (g_vm->getGameID() == GType_Sherlock1)
		setDebugger(new DemoDebugger());
}

Common::Error TSageEngine::init() {
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);

	return Common::kNoError;
}

TSageEngine::~TSageEngine() {
	// Remove all of our debug levels here
	DebugMan.clearAllDebugChannels();
}

bool TSageEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsReturnToLauncher) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void TSageEngine::initialize() {
	// Set up the correct graphics mode
	init();

	g_saver = new Saver();

	// Set up the resource manager
	g_resourceManager = new ResourceManager();
	if (g_vm->getGameID() == GType_Ringworld) {
		if (g_vm->getFeatures() & GF_DEMO) {
			// Add the single library file associated with the demo
			g_resourceManager->addLib(getPrimaryFilename());
			g_globals = new Globals();
		} else {
			g_resourceManager->addLib("RING.RLB");
			g_resourceManager->addLib("TSAGE.RLB");
			g_globals = new Globals();
		}
	} else if (g_vm->getGameID() == GType_BlueForce) {
		g_resourceManager->addLib("BLUE.RLB");
		if (g_vm->getFeatures() & GF_FLOPPY) {
			g_resourceManager->addLib("FILES.RLB");
			g_resourceManager->addLib("TSAGE.RLB");
		}
		g_globals = new BlueForce::BlueForceGlobals();

		// Setup the user interface
		T2_GLOBALS._uiElements.setup(Common::Point(0, UI_INTERFACE_Y - 2));

		// Reset all global variables
		BF_GLOBALS.reset();
	} else if (g_vm->getGameID() == GType_Ringworld2) {
		g_resourceManager->addLib("R2RW.RLB");
		g_globals = new Ringworld2::Ringworld2Globals();

		// Setup the user interface
		T2_GLOBALS._uiElements.setup(Common::Point(0, UI_INTERFACE_Y));

		// Reset all global variables
		R2_GLOBALS.reset();
	} else if (g_vm->getGameID() == GType_Sherlock1) {
#ifdef TSAGE_SHERLOCK_ENABLED
		g_resourceManager->addLib("SF3.RLB");
		g_globals = new Globals();

		return;
#endif
	}

	g_globals->gfxManager().setDefaults();

	// Setup sound settings
	syncSoundSettings();
}

void TSageEngine::deinitialize() {
	delete g_globals;
	delete g_resourceManager;
	delete g_saver;
	g_resourceManager = NULL;
	g_saver = NULL;
}

Common::Error TSageEngine::run() {
	// Basic initialization
	initialize();

	g_globals->_sceneHandler->registerHandler();
	g_globals->_game->execute();

	deinitialize();
	return Common::kNoError;
}

/**
 * Returns true if it is currently okay to restore a game
 */
bool TSageEngine::canLoadGameStateCurrently() {
	return (g_globals != NULL) && (g_globals->_game != NULL) && g_globals->_game->canLoadGameStateCurrently();
}

/**
 * Returns true if it is currently okay to save the game
 */
bool TSageEngine::canSaveGameStateCurrently() {
	return (g_globals != NULL) && (g_globals->_game != NULL) && g_globals->_game->canSaveGameStateCurrently();
}

/**
 * Load the savegame at the specified slot index
 */
Common::Error TSageEngine::loadGameState(int slot) {
	return g_saver->restore(slot);
}

/**
 * Save the game to the given slot index, and with the given name
 */
Common::Error TSageEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	return g_saver->save(slot, desc);
}

void TSageEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	g_globals->_soundManager.syncSounds();
}

} // End of namespace TsAGE
