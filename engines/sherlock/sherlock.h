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

#ifndef SHERLOCK_HOLMES_H
#define SHERLOCK_HOLMES_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/hash-str.h"
#include "common/serializer.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/util.h"
#include "engines/engine.h"
#include "sherlock/animation.h"
#include "sherlock/debugger.h"
#include "sherlock/events.h"
#include "sherlock/inventory.h"
#include "sherlock/journal.h"
#include "sherlock/map.h"
#include "sherlock/people.h"
#include "sherlock/resources.h"
#include "sherlock/saveload.h"
#include "sherlock/scene.h"
#include "sherlock/screen.h"
#include "sherlock/sound.h"
#include "sherlock/talk.h"
#include "sherlock/user_interface.h"

namespace Sherlock {

enum {
	kDebugScript = 1 << 0
};

enum GameType {
	GType_SerratedScalpel = 0,
	GType_RoseTattoo = 1
};

#define SHERLOCK_SCREEN_WIDTH _vm->_screen->w()
#define SHERLOCK_SCREEN_HEIGHT _vm->_screen->h()
#define SHERLOCK_SCENE_HEIGHT 138

struct SherlockGameDescription;

class Resource;

class SherlockEngine : public Engine {
private:
	/**
	 * Main loop for displaying a scene and handling all that occurs within it
	 */
	void sceneLoop();

	/**
	 * Handle all player input
	 */
	void handleInput();

	/**
	 * Load game configuration esttings
	 */
	void loadConfig();
protected:
	/**
	 * Does basic initialization of the game engine
	 */
	virtual void initialize();

	virtual void showOpening() = 0;

	virtual void startScene() {}

	/**
	 * Returns a list of features the game itself supports
	 */
	virtual bool hasFeature(EngineFeature f) const;
public:
	const SherlockGameDescription *_gameDescription;
	Animation *_animation;
	Debugger *_debugger;
	Events *_events;
	Inventory *_inventory;
	Journal *_journal;
	Map *_map;
	People *_people;
	Resources *_res;
	SaveManager *_saves;
	Scene *_scene;
	Screen *_screen;
	Sound *_sound;
	Talk *_talk;
	UserInterface *_ui;
	Common::RandomSource _randomSource;
	Common::Array<bool> _flags;
	bool _useEpilogue2;
	int _loadGameSlot;
	bool _canLoadSave;
	bool _showOriginalSavesDialog;
	bool _interactiveFl;
public:
	SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	virtual ~SherlockEngine();

	/**
	 * Main method for running the game
	 */
	virtual Common::Error run();

	/**
	 * Returns true if a savegame can be loaded
	 */
	virtual bool canLoadGameStateCurrently();

	/**
	 * Returns true if the game can be saved
	 */
	virtual bool canSaveGameStateCurrently();

	/**
	 * Called by the GMM to load a savegame
	 */
	virtual Common::Error loadGameState(int slot);

	/**
	 * Called by the GMM to save the game
	 */
	virtual Common::Error saveGameState(int slot, const Common::String &desc);

	/**
	 * Called by the engine when sound settings are updated
	 */
	virtual void syncSoundSettings();

	/**
	 * Returns whether the version is a demo
	 */
	virtual bool isDemo() const;

	/**
	 * Returns the Id of the game
	 */
	GameType getGameID() const;

	/**
	 * Returns the platform the game's datafiles are for
	 */
	Common::Platform getPlatform() const;

	/**
	 * Return a random number
	 */
	int getRandomNumber(int limit) { return _randomSource.getRandomNumber(limit - 1); }

	/**
	 * Read the state of a global flag
	 * @remarks		If a negative value is specified, it will return the inverse value
	 *		of the positive flag number
	 */
	bool readFlags(int flagNum);

	/**
	 * Sets a global flag to either true or false depending on whether the specified
	 * flag is positive or negative
	 */
	void setFlags(int flagNum);

	/**
	 * Saves game configuration information
	 */
	void saveConfig();

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Common::Serializer &s);
};

#define IS_ROSE_TATTOO (_vm->getGameID() == GType_RoseTattoo)
#define IS_SERRATED_SCALPEL (_vm->getGameID() == GType_SerratedScalpel)

} // End of namespace Sherlock

#endif
