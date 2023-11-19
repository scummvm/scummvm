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

#include "crab/GameClock.h"
#include "crab/gamestate_container.h"
#include "crab/event/GameEventInfo.h"
#include "crab/event/gameeventmanager.h"
#include "crab/ui/DevConsole.h"
#include "crab/ui/hud.h"
#include "crab/ui/map.h"

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
	} _state;

	enum SaveGameType {
		SAVEGAME_NORMAL, // Save the game normally when user uses the save menu
		SAVEGAME_EVENT,  // Auto-save the game at certain points using events
		SAVEGAME_EXIT,   // Auto-save the game on exit
		SAVEGAME_QUICK   // You can use quick-save and quick-load keys
	};

	// These things don't need to be saved
	bool _isInited;
	pyrodactyl::ui::HUD _hud;
	Common::Array<pyrodactyl::event::EventResult> _eventRes;
	pyrodactyl::ui::ParagraphData _popDefault;
	pyrodactyl::ui::DebugConsole _debugConsole;

	// These things need to be saved
	pyrodactyl::ui::Map _map;
	pyrodactyl::event::Manager _gem;
	pyrodactyl::event::Info _info;
	pyrodactyl::level::Level _level;
	pyrodactyl::event::TriggerSet _gameOver;

	// Keeps track of the time player has spent in the game
	GameClock _clock;

	// The name of the auto save and quick save files
	struct SaveFile {
		bool _autoSlot;
		Common::String _auto1, _auto2, _autoQuit, _quick, _ironman;

		SaveFile() : _auto1("autoSave 1"), _auto2("autoSave 2"), _autoQuit("autoSave"), _quick("Quick Save") { _autoSlot = false; }

		void load(rapidxml::xml_node<char> *node) {
			loadStr(_auto1, "auto_1", node);
			loadStr(_auto2, "auto_2", node);
			loadStr(_autoQuit, "quit", node);
			loadStr(_quick, "quick", node);
		}
	} _savefile;

	static void quit(bool &shouldChangeState, GameStateID &newStateId, const GameStateID &newStateVal);

	bool applyResult();
	void applyResult(LevelResult result);

	// Load a level
	bool loadLevel(const Common::String &id, int playerX = -1, int playerY = -1);

	void toggleState(const State &s);

	// A nice simple function for saving games
	void createSaveGame(const SaveGameType &savetype);

	Common::String fullPath(const Common::String &filename) {
		Common::String res = "CRAB_" + filename;
		res += g_engine->_filePath->_saveExt;
		return res;
	}

	// Load the current player image
	void playerImg() {
		_hud.playerImg(g_engine->_eventStore->_img[_info.playerImg()]);
	}

public:
	Game() : _isInited(false), _state(STATE_GAME) {}

	void init(const Common::String &filename);

	void startNewGame();
	void loadGame();

	void handleEvents(Common::Event &event, bool &shouldChangeState, GameStateID &newStateId);
#if 0
	void handleEvents(SDL_Event &Event, bool &ShouldChangeState, GameStateID &NewStateID);
#endif
	void internalEvents(bool &shouldChangeState, GameStateID &newStateId);
	void draw();

	bool loadState(Common::SeekableReadStream *stream);

	// Raw function to save game to file - generally, using the CreateSaveGame function is recommended
	void saveState(Common::SeekableWriteStream *stream);

	void autoSave() {
		createSaveGame(SAVEGAME_EXIT);
	}

	void setUI();
};

} // End of namespace Crab

#endif // CRAB_GAME_H
