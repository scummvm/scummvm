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

#ifndef BAGEL_H
#define BAGEL_H

#include "common/random.h"

#include "bagel/detection.h"
#include "bagel/music.h"
#include "bagel/baglib/master_win.h"

namespace Bagel {

struct BagelGameDescription;

class BagelEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	StBagelSave _saveData;

	bool canSaveLoadFromWindow(bool save) const;

public:
	Graphics::Screen *_screen = nullptr;
	MusicPlayer *_midi = nullptr;
	bool _useOriginalSaveLoad = false;
	CBagMasterWin *_masterWin = nullptr;
	CBofPoint g_cInitLoc;       // This is the initial location for the next new pan (only option at this point)
	bool g_bUseInitLoc = false;
	bool g_getVilVarsFl = true;

public:
	BagelEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~BagelEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Return the game's platform
	 */
	Common::Platform getPlatform() const;

	/**
	 * Return whether it's a demo
	 */
	bool isDemo() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum = 0x7fffffff) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Save a game state
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave,
		StBagelSave &saveData);

	/**
	 * Load a game state
	 */
	Common::Error loadGameState(int slot) override;

	/**
	 * Handles saving the game
	 */
	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override;

	/**
	 * Handles loading a savegame
	 */
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override;

	/**
	 * Returns a list of savegames
	 */
	SaveStateList listSaves() const;

	/**
	 * Returns true if any savegames exist
	 */
	bool savesExist() const;

	/**
	 * Pause all internal timers.
	 */
	void pauseEngineIntern(bool pause) override;

	void errorDialog(const char *msg) const;

	void enableKeymapper(bool enabled);
};

extern BagelEngine *g_engine;
#define SHOULD_QUIT ::Bagel::g_engine->shouldQuit()

} // End of namespace Bagel

#endif // BAGEL_H
