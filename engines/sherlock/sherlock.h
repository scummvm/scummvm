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

#ifndef SHERLOCK_SHERLOCK_H
#define SHERLOCK_SHERLOCK_H

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
#include "sherlock/fixed_text.h"
#include "sherlock/inventory.h"
#include "sherlock/journal.h"
#include "sherlock/map.h"
#include "sherlock/music.h"
#include "sherlock/people.h"
#include "sherlock/resources.h"
#include "sherlock/saveload.h"
#include "sherlock/scene.h"
#include "sherlock/screen.h"
#include "sherlock/sound.h"
#include "sherlock/talk.h"
#include "sherlock/user_interface.h"
#include "sherlock/detection.h"

namespace Sherlock {

enum {
	kDebugLevelScript      = 1 << 0,
	kDebugLevelAdLibDriver = 2 << 0,
	kDebugLevelMT32Driver  = 3 << 0,
	kDebugLevelMusic       = 4 << 0
};

#define SHERLOCK_SCREEN_WIDTH _vm->_screen->width()
#define SHERLOCK_SCREEN_HEIGHT _vm->_screen->height()
#define SHERLOCK_SCENE_WIDTH _vm->_screen->_backBuffer1.width()
#define SHERLOCK_SCENE_HEIGHT (IS_SERRATED_SCALPEL ? 138 : 480)
#define SCENES_COUNT (IS_SERRATED_SCALPEL ? 63 : 101)
#define MAX_BGSHAPES (IS_SERRATED_SCALPEL ? 64 : 150)

#define COL_INFO_FOREGROUND (IS_SERRATED_SCALPEL ? (byte)Scalpel::INFO_FOREGROUND : (byte)Tattoo::INFO_FOREGROUND)
#define COL_PEN_COLOR (IS_SERRATED_SCALPEL ? (byte)Scalpel::PEN_COLOR : (byte)Tattoo::PEN_COLOR)
#define COL_PEN_HIGHLIGHT (IS_SERRATED_SCALPEL ? 15 : 129)

class Resource;

class SherlockEngine : public Engine {
private:
\
	/**
	 * Main loop for displaying a scene and handling all that occurs within it
	 */
	void sceneLoop();

	/**
	 * Handle all player input
	 */
	void handleInput();
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
	bool hasFeature(EngineFeature f) const override;

	/**
	 * Load game configuration esttings
	 */
	virtual void loadConfig();
public:
	const SherlockGameDescription *_gameDescription;
	Animation *_animation;
	Debugger *_debugger;
	Events *_events;
	FixedText *_fixedText;
	Inventory *_inventory;
	Journal *_journal;
	Map *_map;
	Music *_music;
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
	bool _isScreenDoubled;
public:
	SherlockEngine(OSystem *syst, const SherlockGameDescription *gameDesc);
	~SherlockEngine() override;

	/**
	 * Main method for running the game
	 */
	Common::Error run() override;

	/**
	 * Returns true if a savegame can be loaded
	 */
	bool canLoadGameStateCurrently() override;

	/**
	 * Returns true if the game can be saved
	 */
	bool canSaveGameStateCurrently() override;

	/**
	 * Called by the GMM to load a savegame
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Called by the GMM to save the game
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Called by the engine when sound settings are updated
	 */
	void syncSoundSettings() override;

	/**
	 * Saves game configuration information
	 */
	virtual void saveConfig();

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
	 * Return the game's language
	 */
	Common::Language getLanguage() const;

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
	 * Set a global flag to 0 or 1 depending on whether the passed flag is negative or positive.
	 * @remarks		We don't use the global setFlags method because we don't want to check scene flags
	 */
	void setFlagsDirect(int flagNum);

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);
};

#define IS_ROSE_TATTOO (_vm->getGameID() == GType_RoseTattoo)
#define IS_SERRATED_SCALPEL (_vm->getGameID() == GType_SerratedScalpel)
#define IS_3DO (_vm->getPlatform() == Common::kPlatform3DO)

} // End of namespace Sherlock

#endif
