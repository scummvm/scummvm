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

#ifndef AGS_AGS_H
#define AGS_AGS_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/random.h"
#include "common/hash-str.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/surface.h"

#include "ags/detection.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/lib/allegro/system.h"
#include "ags/engine/util/mutex_std.h"

namespace AGS3 {
class Globals;
}

namespace AGS {

/**
 * @defgroup agsengine AGS Engine
 * @brief Engine to run Adventure Game Studio games.
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
	Graphics::Screen *_rawScreen;
	::AGS3::BITMAP *_screen;
	::AGS3::GFX_DRIVER *_gfxDriver;
	::AGS3::AGS::Engine::Mutex _sMutex;
	::AGS3::AGS::Engine::Mutex _soundCacheMutex;
	::AGS3::AGS::Engine::Mutex _mp3Mutex;
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
	 * Setse up the graphics mode
	 */
	void setGraphicsMode(size_t w, size_t h);

	bool hasFeature(EngineFeature f) const override {
		return
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime) ||
			(f == kSupportsReturnToLauncher);
	};

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
};

extern AGSEngine *g_vm;
#define screen ::AGS::g_vm->_screen
#define gfx_driver ::AGS::g_vm->_gfxDriver
#define SHOULD_QUIT ::AGS::g_vm->shouldQuit()

} // namespace AGS

#endif
