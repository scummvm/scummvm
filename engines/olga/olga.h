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

#ifndef OLGA_H
#define OLGA_H

#include "common/error.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/system.h"
#include "engines/engine.h"
#include "graphics/screen.h"

#include "olga/detection.h"
#include "olga/lbm.h"

namespace Olga {

// Screen layout of a room (notes/interface.md)
enum {
	kRoomTop = 8,          // the 320x138 background is drawn at y 8
	kSentenceTop = 146,    // sentence line 146..155
	kPanelTop = 156        // verbs and inventory below
};

// One frame of the original's loop: 4 ticks of its 100 Hz timer
static const uint32 kFrameMs = 40;

class OlgaEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	LbmFile _room;
	Common::String _roomName;

	/** Main_ShowPicture: a full-screen LBM with its palette, image 1 at 0,0. */
	bool showPicture(const Common::Path &name);
	/** KeyWait_WaitMsOrKey: wait ms milliseconds or until a key (or click). */
	void waitOrKey(uint32 ms);
	/** Responses_1e0d: the empty interface panel. */
	void drawPanel();

protected:
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;

	OlgaEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~OlgaEngine() override;

	uint32 getFeatures() const;
	Common::String getGameId() const;

	uint32 getRandomNumber(uint maxNum) {
		return _randomSource.getRandomNumber(maxNum);
	}

	bool hasFeature(EngineFeature f) const override {
		return f == kSupportsReturnToLauncher;
	}

	/** Show a room picture (name without .LBM) with the panel; for testing. */
	bool showRoom(const Common::String &name);
	const Common::String &getRoomName() const { return _roomName; }
};

extern OlgaEngine *g_engine;
#define SHOULD_QUIT ::Olga::g_engine->shouldQuit()

} // End of namespace Olga

#endif // OLGA_H
