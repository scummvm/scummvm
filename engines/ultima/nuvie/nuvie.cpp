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

#include "ultima/nuvie/nuvie.h"
#include "ultima/nuvie/meta_engine.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/debugger.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/gui/widgets/console.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/save/save_game.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "common/config-manager.h"
#include "common/translation.h"
#include "common/unzip.h"

namespace Ultima {
namespace Nuvie {

NuvieEngine *g_engine;

NuvieEngine::NuvieEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc) :
		Ultima::Shared::UltimaEngine(syst, gameDesc),  _config(nullptr), _savegame(nullptr),
		_screen(nullptr), _script(nullptr), _game(nullptr), _soundManager(nullptr) {
	g_engine = this;
}

NuvieEngine::~NuvieEngine() {
	delete _config;
	delete _events;
	delete _savegame;
	delete _screen;
	delete _script;
	delete _game;

	g_engine = nullptr;
}

bool NuvieEngine::isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) {
	folder = "ultima6";
	majorVersion = 1;
	minorVersion = 1;
	return true;
}


bool NuvieEngine::initialize() {
	uint8 gameType;
	bool playEnding = false;
	bool showVirtueMsg = false;

	if (!Ultima::Shared::UltimaEngine::initialize())
		return false;

	// Get which game to play
	switch (getGameId()) {
	case GAME_ULTIMA6:
		gameType = NUVIE_GAME_U6;
		break;
	case GAME_MARTIAN_DREAMS:
		gameType = NUVIE_GAME_MD;
		break;
	case GAME_SAVAGE_EMPIRE:
		gameType = NUVIE_GAME_SE;
		break;
	default:
		error("Unknown game");
		break;
	}

	// Find and load config file
	initConfig();

	// Setup events
	Events *events = new Ultima::Nuvie::Events(this, _config);
	_events = events;

	// Setup savegame handler
	_savegame = new SaveGame(_config);

	// Setup debugger
	setDebugger(new Debugger());

	// Setup screen
	_screen = new Screen(_config);

	if (_screen->init() == false) {
		DEBUG(0, LEVEL_ERROR, "Initializing screen!\n");
		return false;
	}

	GUI *gui = new GUI(_config, _screen);

	ConsoleInit(_config, _screen, gui, _screen->get_width(), _screen->get_height());
	ConsoleAddInfo("ScummVM Nuvie");
	ConsoleAddInfo("\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t\t");

	if (showVirtueMsg) {
		ConsoleShow();
		ConsoleAddInfo("");
		ConsoleAddInfo("");
		ConsoleAddInfo("\"The path to victory is marked with the laws of");
		ConsoleAddInfo(" Virtue, not the commands of DOS.\"");
		ConsoleAddInfo("");
		ConsoleAddInfo("     -- Lord British");
		return false;
	}

	// Setup various game related config variables.
	assignGameConfigValues(gameType);

	// Check for a valid path to the selected game.
	if (checkGameDir(gameType) == false)
		return false;

	if (checkDataDir() == false)
		return false;

	_soundManager = new SoundManager(_mixer);
	_soundManager->nuvieStartup(_config);

	_game = new Game(_config, events, _screen, gui, gameType, _soundManager);

	_script = new Script(_config, gui, _soundManager, gameType);
	if (_script->init() == false)
		return false;

	if (playEnding) {
		_script->play_cutscene("/ending.lua");
		return false;
	}

	if (playIntro() == false) {
		ConsoleDelete();
		return false;
	}

	if (_game->loadGame(_script) == false) {
		delete _game;
		return false;
	}

	ConsoleDelete();

	return true;
}

Common::Error NuvieEngine::run() {
	if (initialize()) {
		if (_game)
			_game->play();
	}

	return Common::kNoError;
}

void NuvieEngine::initConfig() {
	_config = new Configuration();
	_config->load(_gameDescription->gameId, isEnhanced());
}

void NuvieEngine::assignGameConfigValues(uint8 gameType) {
	Std::string game_name, game_id;

	_config->set("config/GameType", gameType);

	switch (gameType) {
	case NUVIE_GAME_U6 :
		game_name.assign("ultima6");
		game_id.assign("u6");
		break;
	case NUVIE_GAME_MD :
		game_name.assign("martian");
		game_id.assign("md");
		break;
	case NUVIE_GAME_SE :
		game_name.assign("savage");
		game_id.assign("se");
		break;
	}

	_config->set("config/GameName", game_name);
	_config->set("config/GameID", game_id);

	return;
}

bool NuvieEngine::checkGameDir(uint8 gameType) {
	Std::string path;

	config_get_path(_config, "", path);
	ConsoleAddInfo("gamedir: \"%s\"", path.c_str());

	return true;
}

bool NuvieEngine::checkDataDir() {
	Std::string path;
	_config->value("config/datadir", path, "");
	ConsoleAddInfo("datadir: \"%s\"", path.c_str());

	return true;
}

void NuvieEngine::syncSoundSettings() {
	Ultima::Shared::UltimaEngine::syncSoundSettings();
	if (!_soundManager)
		return;

	_soundManager->set_audio_enabled(
		!ConfMan.hasKey("mute") || !ConfMan.getBool("mute"));
	_soundManager->set_sfx_enabled(
		!ConfMan.hasKey("sfx_mute") || !ConfMan.getBool("sfx_mute"));
	_soundManager->set_music_enabled(
		!ConfMan.hasKey("music_mute") || !ConfMan.getBool("music_mute"));
	_soundManager->set_speech_enabled(
		!ConfMan.hasKey("speech_mute") || !ConfMan.getBool("speech_mute"));

	_soundManager->set_sfx_volume(ConfMan.hasKey("sfx_volume") ?
		ConfMan.getInt("sfx_volume") : 255);
	_soundManager->set_music_volume(ConfMan.hasKey("music_volume") ?
		ConfMan.getInt("music_volume") : 255);
}

bool NuvieEngine::canLoadGameStateCurrently(bool isAutosave) {
	if (_game == nullptr || !_game->isLoaded())
		return false;

	// Note that to mimic what Nuvie originally did, any attempt to try and open
	// the save dialog will result in active gumps being closed
	Events *events = static_cast<Events *>(_events);
	MapWindow *mapWindow = _game->get_map_window();
	
	if (isAutosave) {
		return events->get_mode() == MOVE_MODE;
	
	} else {
		events->close_gumps();

		switch (events->get_mode()) {
		case EQUIP_MODE:
			events->cancelAction();
			return false;
		case MOVE_MODE:
			mapWindow->set_looking(false);
			mapWindow->set_walking(false);
			return true;
		default:
			// Saving/loading only available in standard move mode in-game
			return false;
		}
	}
}

bool NuvieEngine::canSaveGameStateCurrently(bool isAutosave) {
	if (!canLoadGameStateCurrently(isAutosave))
		return false;

	// Further checks against saving
	Events *events = static_cast<Events *>(_events);
	MsgScroll *scroll = _game->get_scroll();

	if (_game->is_armageddon()) {
		if (!isAutosave)
			scroll->message("Can't save. You killed everyone!\n\n");
		return false;
	} else if (events->using_control_cheat()) {
		if (!isAutosave)
			scroll->message(" Can't save while using control cheat\n\n");
		return false;
	}

	return true;
}

Common::Error NuvieEngine::loadGameState(int slot) {
	Common::String filename = getSaveStateName(slot);

	if (slot == ORIGINAL_SAVE_SLOT) {
		// For Nuvie, unless a savegame is already present for the slot,
		// loading it loads in the original game savegame
		Common::InSaveFile *saveFile = _saveFileMan->openForLoading(filename);
		bool isPresent = saveFile != nullptr;
		delete saveFile;

		if (!isPresent) {
			_savegame->load_original();
			return Common::kNoError;
		}
	}

	return _savegame->load(filename) ? Common::kNoError : Common::kReadingFailed;
}

Common::Error NuvieEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::String filename = getSaveStateName(slot);
	if (_savegame->save(filename, desc, isAutosave)) {
		if (!isAutosave) {
			// Store which savegame was most recently saved
			ConfMan.setInt("latest_save", slot);
			ConfMan.flushToDisk();

			// Display that the game was saved
			MsgScroll *scroll = Game::get_game()->get_scroll();
			scroll->display_string("\nGame Saved\n\n");
			scroll->display_prompt();
		}

		return Common::kNoError;
	} else {
		return Common::kReadingFailed;
	}
}

bool NuvieEngine::journeyOnwards() {
	// If savegame selected from ScummVM launcher, load it now
	if (ConfMan.hasKey("save_slot")) {
		int saveSlot = ConfMan.getInt("save_slot");
		return loadGameState(saveSlot).getCode() == Common::kNoError;
	}

	// Check for new game needed
	bool newsave = false;
	_config->value("config/newgame", newsave, false);

	if (newsave) {
		return _savegame->load_new();
	}

	// Otherwise start a new game
	return _savegame->load_new();
}

bool NuvieEngine::loadLatestSave() {
	if (ConfMan.hasKey("latest_save")) {
		int saveSlot = ConfMan.getInt("latest_save");
		return loadGameState(saveSlot).getCode() == Common::kNoError;
	}

	return _savegame->load_new();
}

bool NuvieEngine::quickSave(int saveSlot, bool isLoad) {
	if (saveSlot < 0 || saveSlot > 99)
		return false;

	Std::string text;
	MsgScroll *scroll = _game->get_scroll();

	if (isLoad) {
		if (!canLoadGameStateCurrently(false))
			return false;

		text = Common::convertFromU32String(_("loading quick save %d"));
	} else {
		if (!canSaveGameStateCurrently(false))
			return false;

		text = Common::convertFromU32String(_("saving quick save %d"));
	}

	text = Std::string::format(text.c_str(), saveSlot);
	scroll->display_string(text);

	if (isLoad) {
		if (loadGameState(saveSlot).getCode() == Common::kNoError) {
			return true;
		} else {
			scroll->message("\nfailed!\n\n");
			return false;
		}
	} else {
		Common::String saveDesc = Common::String::format("Quicksave %03d", saveSlot);
		return saveGameState(saveSlot, saveDesc, false).getCode() == Common::kNoError;
	}
}

bool NuvieEngine::playIntro() {
	if (ConfMan.hasKey("save_slot") && ConfMan.getInt("save_slot") >= 0)
		// Loading a savegame from the launcher, so skip intro
		return true;

	if (_script->play_cutscene("/intro.lua")) {
		bool should_quit = false;
		_config->value("config/quit", should_quit, false);
		if (!should_quit) {
			ConsoleHide();
			return true;
		}
	}

	return false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
