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

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/hash-str.h"
#include "common/serializer.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/util.h"
#include "graphics/surface.h"
#include "engines/engine.h"
#include "ultima/detection.h"

#include "ultima/shared/engine/ultima.h"

namespace Ultima {

struct UltimaGameDescription;

namespace Shared {

struct UltimaSavegameHeader {
	uint8 _version;
	uint8 _gameId;
	uint8 _language;
	uint8 _videoMode;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
	int _totalFrames;
};

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
	bool initialize() override;

	/**
	 * Deinitialize the engine
	 */

	void deinitialize() override;

	/**
	 * Returns the data archive folder and version that's required
	 */
	bool isDataRequired(Common::String &folder, int &majorVersion, int &minorVersion) override;
public:
	GameBase *_game;
	MouseCursor *_mouseCursor;
	Gfx::Screen *_screen;
public:
	UltimaEarlyEngine(OSystem *syst, const UltimaGameDescription *gameDesc);
	~UltimaEarlyEngine() override;

	/**
	 * Main method for running the game
	 */
	Common::Error run() override;

	/**
	 * Play the game
	 */
	void playGame();

	/**
	 * Get the screen
	 */
	Graphics::Screen *getScreen() const override;
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
	 * Load a savegame
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Save a game state.
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	/*
	 * Creates a new hierarchy for the game, that contains all the logic for playing that particular game.
	 */
	Game *createGame() const;
};

} // End of namespace Shared

extern Shared::UltimaEarlyEngine *g_vm;

} // End of namespace Ultima

#endif
