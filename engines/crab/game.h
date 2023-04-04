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
 * This code is based on the CRAB engine
 *
 * Copyright (c) Arvind Raja Yadav
 *
 * Licensed under MIT
 *
 */

#ifndef CRAB_GAME_H
#define CRAB_GAME_H

#include "crab/ui/DevConsole.h"
#include "crab/GameClock.h"
#include "crab/event/GameEventInfo.h"
#include "crab/image/ImageManager.h"
#include "crab/LoadingScreen.h"
#include "crab/common_header.h"
#include "crab/input/cursor.h"
#include "crab/event/gameeventmanager.h"
#include "crab/gamestate_container.h"
#include "crab/ui/hud.h"
#include "crab/level/level.h"
#include "crab/ui/map.h"
#include "crab/ui/menu.h"

namespace Crab {

class Game : public GameState {
private:
	enum State {
		STATE_GAME,
		STATE_MAP,
		STATE_PAUSE,
		STATE_CHARACTER,
		STATE_JOURNAL,
		STATE_INVENTORY,
		STATE_HELP,
		STATE_LOSE_MENU,
		STATE_LOSE_LOAD
	} state;

	enum SaveGameType {
		SAVEGAME_NORMAL, // Save the game normally when user uses the save menu
		SAVEGAME_EVENT,  // Auto-save the game at certain points using events
		SAVEGAME_EXIT,   // Auto-save the game on exit
		SAVEGAME_QUICK   // You can use quick-save and quick-load keys
	};

	// These things don't need to be saved
	pyrodactyl::ui::HUD hud;
	std::vector<pyrodactyl::event::EventResult> event_res;
	pyrodactyl::ui::ParagraphData pop_default;
	pyrodactyl::ui::DebugConsole debug_console;

	// These things need to be saved
	pyrodactyl::ui::Map map;
	pyrodactyl::event::Manager gem;
	pyrodactyl::event::Info info;
	pyrodactyl::level::Level level;
	pyrodactyl::event::TriggerSet game_over;

	// Keeps track of the time player has spent in the game
	GameClock clock;

	// The name of the auto save and quick save files
	struct SaveFile {
		bool auto_slot;
		std::string auto_1, auto_2, auto_quit, quick, ironman;

		SaveFile() : auto_1("AutoSave 1"), auto_2("AutoSave 2"), auto_quit("AutoSave"), quick("Quick Save") { auto_slot = false; }

		void Load(rapidxml::xml_node<char> *node) {
			LoadStr(auto_1, "auto_1", node);
			LoadStr(auto_2, "auto_2", node);
			LoadStr(auto_quit, "quit", node);
			LoadStr(quick, "quick", node);
		}
	} savefile;

	void StartNewGame();
	void LoadGame(const std::string &filename);
	static void Quit(bool &ShouldChangeState, GameStateID &NewStateID, const GameStateID &NewStateVal);

	bool ApplyResult();
	void ApplyResult(LevelResult result);

	// Load a level
	bool LoadLevel(const std::string &id, int player_x = -1, int player_y = -1);

	void ToggleState(const State &s);

	// A nice simple function for saving games
	void CreateSaveGame(const SaveGameType &savetype);

	std::string FullPath(const std::string &filename) {
		std::string res = gFilePath.appdata.c_str();
		res += gFilePath.save_dir.c_str();
		res += filename;
		res += gFilePath.save_ext.c_str();

		return res;
	}

	// Load the current player image
	void PlayerImg() { hud.PlayerImg(pyrodactyl::event::gEventStore.img.at(info.PlayerImg())); }

public:
	Game() { StartNewGame(); }
	Game(const std::string &filename) { LoadGame(filename); }
	~Game() {}

	void Init(const std::string &filename);

	void HandleEvents(Common::Event &Event, bool &ShouldChangeState, GameStateID &NewStateID);
#if 0
	void HandleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID);
#endif
	void InternalEvents(bool &ShouldChangeState, GameStateID &NewStateID);
	void Draw();

	void LoadState(const std::string &filename);

	// Raw function to save game to file - generally, using the CreateSaveGame function is recommended
	void SaveState(const std::string &filename, const bool &overwrite);

	void AutoSave() { CreateSaveGame(SAVEGAME_EXIT); }

	void SetUI();
};

} // End of namespace Crab

#endif // CRAB_GAME_H
