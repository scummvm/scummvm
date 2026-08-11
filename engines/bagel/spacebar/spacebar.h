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

#ifndef BAGEL_SPACEBAR_H
#define BAGEL_SPACEBAR_H

#include "bagel/bagel.h"
#include "bagel/spacebar/baglib/bagel.h"
#include "bagel/spacebar/bibble_window.h"

namespace Bagel {
namespace SpaceBar {

class SpaceBarEngine : public BagelEngine, public CBagel {
private:
	StBagelSave _saveData;

	ErrorCode InitializeSoundSystem(uint16 nChannels = 1, uint32 nFreq = 11025, uint16 nBitsPerSample = 8);
	ErrorCode ShutDownSoundSystem();

	bool canSaveLoadFromWindow(bool save) const;

protected:
	// Engine APIs
	Common::Error run() override;

	ErrorCode initialize() override;
	ErrorCode shutdown() override;

	/**
	 * Pause all internal timers.
	 */
	void pauseEngineIntern(bool pause) override;

public:
	CBetArea g_cBetAreas[BIBBLE_NUM_BET_AREAS];
	const CBofRect viewPortRect = CBofRect(80, 10, 559, 369);
	CBofRect viewRect;
	Graphics::Screen *_screen = nullptr;

public:
	SpaceBarEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~SpaceBarEngine() override;
	void initializePath(const Common::FSNode &gamePath) override;

	Graphics::Screen *getScreen() const override {
		return _screen;
	}

	bool shouldQuit() const override {
		return BagelEngine::shouldQuit();
	}

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	/**
	 * Save a game state
	 */
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave,
	                            SpaceBar::StBagelSave &saveData);

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
};

extern SpaceBarEngine *g_engine;

} // namespace SpaceBar
} // namespace Bagel

#endif
