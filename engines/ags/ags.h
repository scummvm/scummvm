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

#ifndef AGS_AGS_H
#define AGS_AGS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/surface.h"

#include "ags/detection.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/lib/allegro/system.h"

// DEBUG: @eklipsed TAKE OUT!!!
//#define ENABLE_AGS_TESTS 1

namespace AGS3 {
class Globals;
}

namespace AGS {

/**
 * @defgroup agsengine AGS Engine
 * @brief Engine to run Adventure Game Studio games.
 */

/* Synced up to upstream: ---
 * ----
 */
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200

struct AGSGameDescription;
struct PluginVersion;
class EventsManager;
class Music;

class AGSEngine : public Engine {
private:
	const AGSGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
public:
	EventsManager *_events;
	Music *_music;
	::AGS3::GFX_DRIVER *_gfxDriver;
	::AGS3::Globals *_globals;
	bool _forceTextAA;
protected:
	// Engine APIs
	Common::Error run() override;
public:
	AGSEngine(OSystem *syst, const AGSGameDescription *gameDesc);
	~AGSEngine() override;
	void GUIError(const Common::String &msg);

	void set_window_title(const char *str) {
		// No implementation
	}

	uint32 getFeatures() const;

	const PluginVersion *getNeededPlugins() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Returns the current list of savegames
	 */
	SaveStateList listSaves() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	/**
	 * Sets the random number seed
	 */
	void setRandomNumberSeed(uint32 seed) {
		_randomSource.setSeed(seed);
	}

	/**
	 * Returns a pixel format for the given color depth.
	 */
	bool getPixelFormat(int depth, Graphics::PixelFormat &format) const;

	/**
	 * Sets up the graphics mode
	 */
	void setGraphicsMode(size_t w, size_t h, int depth);

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	/**
	 * Returns true if the selected game is an unsupported one
	 * earlier than version 2.5
	 */
	bool isUnsupportedPre25() const;

	/*
	 * Returns true if the game has data files greater than 2Gb
	 */
	bool is64BitGame() const;

	/**
	 * Returns the game folder as a ScummVM filesystem node
	 */
	Common::FSNode getGameFolder();

	/**
	 * Indicate whether a game state can be loaded.
	 */
	bool canLoadGameStateCurrently() override;

	/**
	 * Indicate whether a game state can be saved.
	 */
	bool canSaveGameStateCurrently() override;

	/**
	 * Load a savegame
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Save a savegame
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;

	/**
	 * Synchronize user volume settings
	 */
	void syncSoundSettings() override;
};

extern AGSEngine *g_vm;
#define gfx_driver ::AGS::g_vm->_gfxDriver
#define SHOULD_QUIT ::AGS::g_vm->shouldQuit()

} // namespace AGS

#endif
