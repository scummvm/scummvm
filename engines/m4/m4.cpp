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

#include "common/scummsys.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/system.h"
#include "common/savefile.h"
#include "engines/util.h"
#include "graphics/palette.h"
#include "m4/m4.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/conv_io.h"
#include "m4/gui/hotkeys.h"
#include "m4/platform/sound/digi.h"
#include "m4/platform/sound/midi.h"
#include "m4/detection.h"
#include "m4/console.h"
#include "m4/metaengine.h"
#include "m4/core/param.h"

namespace M4 {

M4Engine *g_engine;

M4Engine::M4Engine(OSystem *syst, const M4GameDescription *gameDesc) : Engine(syst),
	_gameDescription(gameDesc), _randomSource("M4") {
	g_engine = this;
}

M4Engine::~M4Engine() {
	delete _screen;
}

uint32 M4Engine::getFeatures() const {
	return _gameDescription->desc.flags;
}

Common::String M4Engine::getGameId() const {
	return _gameDescription->desc.gameId;
}

int M4Engine::getGameType() const {
	return _gameDescription->gameType;
}

Common::Language M4Engine::getLanguage() const {
	return _gameDescription->desc.language;
}

Common::Error M4Engine::run() {
	// Initialize 320x200 paletted graphics mode
	initGraphics(640, 480);

	// Instantiate globals and setup
	Vars *vars = createVars();

	if (vars->init()) {
		// Set the console
		setDebugger(new Console());

		// Check for launcher savegame to load
		if (ConfMan.hasKey("save_slot")) {
			_G(kernel).restore_slot = ConfMan.getInt("save_slot");
			_G(game).previous_room = RESTORING_GAME;
		}

		// Run game here
		m4_inflight();
	}

	delete vars;
	return Common::kNoError;
}

#define KEEP_PLAYING (_G(kernel).going && !shouldQuit())

void M4Engine::m4_inflight() {
	g_vars->getHotkeys()->add_hot_keys();
	_G(kernel).going = true;

	while (KEEP_PLAYING) {
		if (_G(game).previous_room == RESTORING_GAME) {
			midi_stop();
			kernel_load_game(_G(kernel).restore_slot);
		}

		// Start up next section
		_G(between_rooms) = true;
		global_section_constructor();		// Sets the active section
		_G(kernel).going = kernel_section_startup();
		section_init();

		while (_G(game).new_section == _G(game).section_id && KEEP_PLAYING) {
			m4SceneLoad();
			m4RunScene();
			m4EndScene();
		}
	}
}

bool M4Engine::canLoadGameStateCurrently(Common::U32String *msg) {
	return g_vars && g_vars->getInterface() && g_vars->getInterface()->_visible;
}

bool M4Engine::canSaveGameStateCurrently(Common::U32String *msg) {
	return g_vars && g_vars->getInterface() && g_vars->getInterface()->_visible;
}

Common::Error M4Engine::loadGameState(int slot) {
	// Don't load savegame immediately, just set the slot for the engine's
	// kernel to take care of in the outer game loop
	_G(kernel).restore_slot = slot;
	_G(game).new_room = RESTORING_GAME;
	_G(game).new_section = RESTORING_GAME;
	_G(game).previous_room = RESTORING_GAME;

	return Common::kNoError;
}

Common::Error M4Engine::loadGameStateDoIt(int slot) {
	return Engine::loadGameState(slot);
}

Common::Error M4Engine::syncGame(Common::Serializer &s) {
	_G(game).syncGame(s);
	_G(player).syncGame(s);
	_G(player_info).syncGame(s);

	syncFlags(s);

	player_been_sync(s);
	_G(conversations).syncGame(s);
	_G(inventory)->syncGame(s);


	if (s.isLoading()) {
		// set up variables so everyone knows we've teleported
		_G(kernel).restore_game = true;
		_G(between_rooms) = true;
		_G(game).previous_room = KERNEL_RESTORING_GAME;

		digi_set_overall_volume(_G(game).digi_overall_volume_percent);
		midi_set_overall_volume(_G(game).midi_overall_volume_percent);
	}

	return Common::kNoError;
}

bool M4Engine::autosaveExists() const {
	Common::String slotName = getSaveStateName(getAutosaveSlot());
	Common::InSaveFile *saveFile = g_system->getSavefileManager()->openForLoading(slotName);
	bool result = saveFile != nullptr;
	delete saveFile;

	return result;
}

bool M4Engine::savesExist() const {
	return getMetaEngine()->listSaves(_targetName.c_str()).empty();
}

} // End of namespace M4
