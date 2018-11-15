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

namespace Glk {

class Clipboard;
class Conf;
class Events;
class PicList;
class Screen;
class Selection;
class Streams;
class Windows;

enum InterpreterType {
	INTERPRETER_ADVSYS = 0,
	INTERPRETER_AGILITY = 1,
	INTERPRETER_ALAN2 = 2,
	INTERPRETER_ALAN3 = 3,
	INTERPRETER_BOCFEL = 4,
	INTERPRETER_FROTZ = 5,
	INTERPRETER_GEAS = 6,
	INTERPRETER_HUGO = 7,
	INTERPRETER_JACL = 8,
	INTERPRETER_LEVEL9 = 9,
	INTERPRETER_MAGNETIC = 10,
	INTERPRETER_NITFOL = 11,
	INTERPRETER_SCARE = 12,
	INTERPRETER_SCOTT = 13,
	INTERPRETER_TADS = 14
};

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
	Common::String _filename;
	InterpreterType _interpType;
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
	PicList *_picList;
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
	InterpreterType getInterpreterType() const { return _gameDescription._interpType; }

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
	 * Return the filename for a given save slot
	 */
	Common::String getSaveName(uint slot) const {
		return Common::String::format("%s.%.3u", getTargetName().c_str(), slot);
	}

	/**
	 * Display a message in a GUI dialog
	 */
	void GUIError(const char *msg, ...);
};

extern GlkEngine *g_vm;

} // End of namespace Glk

#endif
