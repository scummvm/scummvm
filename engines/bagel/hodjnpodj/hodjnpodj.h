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

#ifndef HODJNPODJ_H
#define HODJNPODJ_H

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
#include "graphics/fonts/winfont.h"
#include "graphics/screen.h"
#include "bagel/hodjnpodj/hnplibs/gamedll.h"
#include "bagel/bagel.h"

namespace Bagel {
namespace HodjNPodj {

struct CBfcMgr {
	bool m_bRestart = false;
};

extern GAMESTRUCT *pGameParams;
extern CBfcMgr *lpMetaGame;

class HodjNPodjEngine : public BagelEngine {
private:
	CBofSound *_backgroundMidi = nullptr;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Common::String _gameId;
	CBfcMgr _metaGame;
	GAMESTRUCT _gameInfo;
	bool _bDonePodj = false;
	bool _bReturnToZoom = false;
	bool _bReturnToMeta = false;
	bool _bReturnToGrandTour = false;
	bool _bAnimationsEnabled = true;
	bool _bScrollingEnabled = false;
	bool _bSlowCPU = false;
	bool _bLowMemory = false;
	size_t _dwFreeSpaceMargin = 850000;
	size_t _dwFreePhysicalMargin = 850000;

public:
	HodjNPodjEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~HodjNPodjEngine() override;

	/**
	 * Gets a random number
	 */
	uint32 getRandomNumber(uint maxNum) {
		return BagelEngine::getRandomNumber(maxNum);
	}

	/**
	 * Returns the underlying screen
	 */
	Graphics::Screen *getScreen() const {
		return _screen;
	}

	bool hasFeature(EngineFeature f) const override {
		return
		    (f == kSupportsLoadingDuringRuntime) ||
		    (f == kSupportsSavingDuringRuntime) ||
		    (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}

	/**
	 * Uses a serializer to allow implementing savegame
	 * loading and saving using a single method
	 */
	Common::Error syncGame(Common::Serializer &s);

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}
};

extern HodjNPodjEngine *g_engine;

} // namespace HodjNPodj
} // namespace Bagel

#endif
