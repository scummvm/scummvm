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
#include "engines/engine.h"
#include "glk/glk_types.h"
#include "glk/streams.h"
#include "glk/pc_speaker.h"
#include "glk/quetzal.h"
#include "glk/game_description.h"

namespace Glk {

class Clipboard;
class Blorb;
class Conf;
class Events;
class Pictures;
class Screen;
class Selection;
class Sounds;
class Streams;
class Windows;

enum GlkDebugChannels {
	kDebugCore      = 1 << 0,
	kDebugScripts   = 1 << 1,
	kDebugGraphics  = 1 << 2,
	kDebugSound     = 1 << 3,
	kDebugSpeech    = 1 << 4
};


#define GLK_SAVEGAME_VERSION 1

/**
 * Base class for the different interpreters
 */
class GlkEngine : public Engine {
private:
	/**
	 * Handles basic initialization
	 */
	void initialize();
protected:
	const GlkGameDescription _gameDescription;
	Common::RandomSource _random;
	int _loadSaveSlot;
	Common::File _gameFile;
	PCSpeaker *_pcSpeaker;
	bool _quitFlag;

	// Engine APIs
	Common::Error run() override;

	/**
	  * Returns true whether a given feature is supported by the engine
	  */
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Setup the video mode
	 */
	virtual void initGraphicsMode();

	/**
	 * Create the debugger
	 */
	virtual void createDebugger();

	/**
	 * Create the screen
	 */
	virtual Screen *createScreen();

	/**
	 * Loads the configuration
	 */
	virtual void createConfiguration();

	/**
	 * Main game loop for the individual interpreters
	 */
	virtual void runGame() = 0;

	/**
	 * Switches Glk from the default black on white color scheme
	 * to white on black
	 */
	void switchToWhiteOnBlack();
public:
	Blorb *_blorb;
	Clipboard *_clipboard;
	Conf *_conf;
	Events *_events;
	Pictures *_pictures;
	Screen *_screen;
	Selection *_selection;
	Streams *_streams;
	Sounds *_sounds;
	Windows *_windows;
	bool _copySelect;
	bool _terminated;

	gidispatch_rock_t(*gli_register_obj)(void *obj, uint objclass);
	void(*gli_unregister_obj)(void *obj, uint objclass, gidispatch_rock_t objrock);
	gidispatch_rock_t(*gli_register_arr)(void *array, uint len, const char *typecode);
	void(*gli_unregister_arr)(void *array, uint len, const char *typecode, gidispatch_rock_t objrock);
public:
	GlkEngine(OSystem *syst, const GlkGameDescription &gameDesc);
	~GlkEngine() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently() override;

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently() override;

	/**
	 * Returns the language
	 */
	Common::Language getLanguage() const { return _gameDescription._language; };

	/**
	 * Returns the running interpreter type
	 */
	virtual InterpreterType getInterpreterType() const = 0;

	/**
	 * Returns the game's Id
	 */
	const Common::String &getGameID() const { return _gameDescription._gameId; }

	/**
	 * Returns the game's md5
	 */
	const Common::String &getGameMD5() const { return _gameDescription._md5; }

	/**
	 * Returns the primary filename for the game
	 */
	const Common::String &getFilename() const { return _gameDescription._filename; }

	/**
	 * Returns any options returned with the game's detection entry
	 */
	uint getOptions() const { return _gameDescription._options; }

	/**
	 * Return the game engine's target name
	 */
	const Common::String &getTargetName() const {
		return _targetName;
	}

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
	Common::Error loadGameState(int slot) override;

	/**
	 * Save the game to a given slot
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Loads Quetzal chunks from the passed savegame
	 */
	virtual Common::Error loadGameChunks(QuetzalReader &quetzal);

	/**
	 * Writes out the Quetzal chunks within a savegame
	 */
	virtual Common::Error saveGameChunks(QuetzalWriter &quetzal);

	/**
	 * Load a savegame from the passed Quetzal file chunk stream
	 */
	virtual Common::Error readSaveData(Common::SeekableReadStream *rs) = 0;

	/**
	 * Save the game. The passed write stream represents access to the UMem chunk
	 * in the Quetzal save file that will be created
	 */
	virtual Common::Error writeGameData(Common::WriteStream *ws) = 0;

	/**
	 * Updates sound settings
	 */
	void syncSoundSettings() override;

	/**
	 * Generate a beep
	 */
	void beep();

	/**
	 * Get a random number
	 */
	uint getRandomNumber(uint max) { return _random.getRandomNumber(max); }

	/**
	 * Set a random number seed
	 */
	void setRandomNumberSeed(uint seed) { _random.setSeed(seed); }

	/**
	 * Flags to quit the game
	 */
	void quitGame() {
		_quitFlag = true;
		Engine::quitGame();
	}

	/**
	 * Returns true if the game should be quit
	 */
	bool shouldQuit() const {
		return _quitFlag || Engine::shouldQuit();
	}
};

extern GlkEngine *g_vm;

} // End of namespace Glk

#endif
