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

#ifndef M4_H
#define M4_H

#include "common/scummsys.h"
#include "common/system.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"

#include "m4/detection.h"
#include "m4/vars.h"
#include "m4/core/rooms.h"

namespace M4 {

struct M4GameDescription;

class M4Engine : public Engine, public Sections {
private:
	const M4GameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	/**
	 * Main game loop
	 */
	void m4_inflight();

	/**
	 * Opens up a savefile in a given slot, and if it's an
	 * original savegame, returns a reference to it
	 */
	Common::InSaveFile *getOriginalSave(int slot) const;

protected:
	// Engine APIs
	Common::Error run() override;

	/**
	 * Creates globals
	 */
	virtual Vars *createVars() = 0;

public:
	Graphics::Screen *_screen = nullptr;

public:
	M4Engine(OSystem *syst, const M4GameDescription *gameDesc);
	~M4Engine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

	/**
	 * Returns the game type
	 */
	int getGameType() const;

	/**
	 * Return game language
	 */
	Common::Language getLanguage() const;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String * msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	Common::Error loadGameState(int slot) override;
	Common::Error loadGameStateDoIt(int slot);

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	virtual void syncFlags(Common::Serializer &s) = 0;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	/**
	 * Returns true if an autosave exists
	 */
	bool autosaveExists() const;

	/**
	 * Returns true if any saves exist
	 */
	bool savesExist() const;

	/**
	 * Lists the saves
	 */
	SaveStateList listSaves() const;

	/**
	 * Show the engine information
	 */
	virtual void showEngineInfo() = 0;
};

extern M4Engine *g_engine;
#define SHOULD_QUIT ::M4::g_engine->shouldQuit()

} // End of namespace M4

#endif // M4_H
