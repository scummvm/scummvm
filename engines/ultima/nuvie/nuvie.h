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

#ifndef NUVIE_NUVIE_H
#define NUVIE_NUVIE_H

#include "ultima/shared/engine/ultima.h"
#include "ultima/shared/std/string.h"
#include "ultima/nuvie/conf/configuration.h"
#include "common/archive.h"
#include "common/random.h"
#include "engines/engine.h"
#include "gui/debugger.h"

namespace Ultima {
namespace Nuvie {

class Game;
class SaveGame;
class Screen;
class Script;
class SoundManager;

class NuvieEngine : public Ultima::Shared::UltimaEngine {
private:
	Configuration *_config;
	Screen *_screen;
	Script *_script;
	Game *_game;
	SaveGame *_savegame;

	SoundManager *_soundManager;
private:
	void initConfig();
	void assignGameConfigValues(uint8 game_type);
	bool checkGameDir(uint8 game_type);
	bool checkDataDir();

	bool playIntro();
protected:
    bool initialize() override;

    /**
     * Returns the data archive folder and version that's required
     */
    bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;
public:
	const Std::string c_empty_string;
public:
	NuvieEngine(OSystem *syst, const Ultima::UltimaGameDescription *gameDesc);
	~NuvieEngine() override;

	/**
	 * Play the game
	 */
	Common::Error run() override;

	/**
	 * Synchronize sound settings
	 */
	void syncSoundSettings() override;

	/**
	 * Indicates whether a game state can be loaded.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	bool canLoadGameStateCurrently(bool isAutosave) override;

	/**
	 * Indicates whether a game state can be saved.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	bool canSaveGameStateCurrently(bool isAutosave) override;

	/**
	 * Load a game state.
	 * @param slot	the slot from which a savestate should be loaded
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @param isAutosave If true, autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) override;

	/**
	 * Either starts the most recently saved game, or falls back on starting a new game
	 */
	bool journeyOnwards();

	/**
	 * Loads the most recently saved game
	 */
	bool loadLatestSave();

	/**
	 * Quick save or load a savegame
	 */
	bool quickSave(int saveSlot, bool isLoad);

	/**
	 * Return a reference to the sound manager
	 */
	SoundManager *getSoundManager() const {
		return _soundManager;
	}
};

extern NuvieEngine *g_engine;

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
