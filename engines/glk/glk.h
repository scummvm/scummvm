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

#ifndef GLK_GLK_H
#define GLK_GLK_H

#include "common/scummsys.h"
#include "common/random.h"
#include "common/system.h"
#include "common/serializer.h"
#include "engines/advancedDetector.h"
#include "engines/engine.h"
#include "glk/glk_types.h"
#include "glk/streams.h"

namespace Glk {

class Clipboard;
class Conf;
class Events;
class Pictures;
class Screen;
class Selection;
class Streams;
class Windows;

enum GlkDebugChannels {
	kDebugCore      = 1 << 0,
	kDebugScripts   = 1 << 1,
	kDebugGraphics  = 1 << 2,
	kDebugSound     = 1 << 3
};


#define GLK_SAVEGAME_VERSION 1

struct GlkGameDescription {
	Common::String _gameId;
	Common::Language _language;
	Common::Platform _platform;
	Common::String _filename;
	Common::String _md5;
};

/**
 * Base class for the different interpreters
 */
class GlkEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	void initialize();

	/**
	 * Setup the video mode
	 */
	void initGraphicsMode();
protected:
	const GlkGameDescription _gameDescription;
	Common::RandomSource _random;
	int _loadSaveSlot;

	// Engine APIs
	virtual Common::Error run();

	/**
	  * Returns true whether a given feature is supported by the engine
	  */
	virtual bool hasFeature(EngineFeature f) const;

	/**
	 * Main game loop for the individual interpreters
	 */
	virtual void runGame(Common::SeekableReadStream *gameFile) = 0;
public:
	Clipboard *_clipboard;
	Conf *_conf;
	Events *_events;
	Pictures *_pictures;
	Screen *_screen;
	Selection *_selection;
	Streams *_streams;
	Windows *_windows;
	bool _copySelect;
	bool _terminated;
	void (*gli_unregister_obj)(void *obj, glui32 objclass, gidispatch_rock_t objrock);
	gidispatch_rock_t (*gli_register_arr)(void *array, glui32 len, const char *typecode);
	void (*gli_unregister_arr)(void *array, glui32 len, const char *typecode, gidispatch_rock_t objrock);

public:
	GlkEngine(OSystem *syst, const GlkGameDescription &gameDesc);
	virtual ~GlkEngine();

	/**
	 * Returns true if a savegame can be loaded
	 */
	virtual bool canLoadGameStateCurrently() override {
		return true;
	}

	/**
	 * Returns true if the game can be saved
	 */
	virtual bool canSaveGameStateCurrently() override {
		return true;
	}

	/**
	 * Returns the bitset of game features
	 */
	uint32 getFeatures() const;

	/**
	 * Returns whether the game is a demo
	 */
	bool isDemo() const;

	/**
	 * Returns the language
	 */
	Common::Language getLanguage() const { return _gameDescription._language; };

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const = 0;

	/**
	 * Returns the game's md5
	 */
	const Common::String &getGameMD5() const { return _gameDescription._md5; }

	/**
	 * Returns the primary filename for the game
	 */
	const Common::String &getFilename() const { return _gameDescription._filename; }

	/**
	 * Return the game engine's target name
	 */
	const Common::String &getTargetName() const {
		return _targetName;
	}

	/**
	 * Display a message in a GUI dialog
	 */
	void GUIError(const char *msg, ...);

	/**
	 * Return the filename for a given save slot
	 */
	Common::String getSaveName(uint slot) const {
		return Common::String::format("%s.%.3u", getTargetName().c_str(), slot);
	}

	/**
	 * Prompt the user for a savegame to load, and then load it
	 */
	Common::Error loadGame();

	/**
	 * Prompt the user to save their game, and then save it
	 */
	Common::Error saveGame();

	/**
	 * Load a savegame from a given slot
	 */
	virtual Common::Error loadGameState(int slot) override;

	/**
	 * Save the game to a given slot
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc) override;

	/**
	 * Load a savegame from the passed file
	 */
	virtual Common::Error loadGameData(strid_t file) = 0;

	/**
	 * Save the game to the passed file
	 */
	virtual Common::Error saveGameData(strid_t file) = 0;
};

extern GlkEngine *g_vm;

} // End of namespace Glk

#endif
