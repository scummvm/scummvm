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

#ifndef ULTIMA_SHARED_EARLY_ULTIMA_EARLY_H
#define ULTIMA_SHARED_EARLY_ULTIMA_EARLY_H

#include "ultima/shared/engine/ultima.h"

namespace Ultima {

struct UltimaGameDescription;

namespace Shared {

class Debugger;
class Events;
class Game;
class GameBase;
class MouseCursor;
class Resources;

namespace Gfx {
class Screen;
}

class UltimaEarlyEngine : public UltimaEngine {
private:
	/**
	 * Initialize the engine
	 */
	virtual bool initialize() override;

	/**
	 * Deinitialize the engine
	 */

	virtual void deinitialize() override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	virtual bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;
public:
	GameBase *_game;
	MouseCursor *_mouseCursor;
	Gfx::Screen *_screen;
public:
	UltimaEarlyEngine(OSystem *syst, const UltimaGameDescription *gameDesc);
	virtual ~UltimaEarlyEngine();

	/**
	 * Main method for running the game
	 */
	virtual Common::Error run() override;

	/**
	 * Play the game
	 */
	void playGame();

	/**
	 * Get the screen
	 */
	virtual Graphics::Screen *getScreen() const;

	/**
	 * Indicates whether a game state can be loaded.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	virtual bool canLoadGameStateCurrently(bool isAutosave) override {
		return false;
	}

	/**
	 * Indicates whether a game state can be saved.
	 * @param isAutosave	Flags whether it's an autosave check
	 */
	virtual bool canSaveGameStateCurrently(bool isAutosave) override {
		return false;
	}

	/**
	 * Save a game state.
	 * @param slot	the slot into which the savestate should be stored
	 * @param desc	a description for the savestate, entered by the user
	 * @param isAutosave If true, autosave is being created
	 * @return returns kNoError on success, else an error code.
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave) {
		return Common::kWritingFailed;
	}

	/*
	 * Creates a new hierarchy for the game, that contains all the logic for playing that particular game.
	 */
	Game *createGame() const;
};

} // End of namespace Shared

extern Shared::UltimaEarlyEngine *g_vm;

} // End of namespace Ultima

#endif
