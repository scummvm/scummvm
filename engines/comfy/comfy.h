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

#ifndef COMFY_H
#define COMFY_H

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

#include "comfy/detection.h"

#define COMFY_SCREEN_WIDTH 320
#define COMFY_SCREEN_HEIGHT 200
#define COMFY_PIT_INPUT_FREQUENCY 1193182
#define COMFY_PIT_TIMER_DIVISOR 0x2E9B

namespace Comfy {

struct ComfyGameDescription;

class ComfyEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Graphics::Screen *_screen;

	void gameMainLoop();
	void gameMainLoopTick();
	void waitForTimerTick(uint64 pitAccumulator);
	void processEvents();

	uint16 timerTick();
	void midiTrackTickAndRemove();
	void animFileTickCommands();
	void sceneTickEvent();
	void midiPollChannels(uint16 ticks);
	void paletteFadeStep(uint16 ticks);
	void lptKeyboardScanAndProcess();
	void actorTickTree();
	void renderFrame();
	void processInput();
	void processMusicEvents();
	void processSceneTransition();

protected:
	// Engine APIs
	Common::Error run() override;

public:
	ComfyEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~ComfyEngine() override;

	uint32 getFeatures() const;

	/**
	 * Returns the game Id
	 */
	Common::String getGameId() const;

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

extern ComfyEngine *g_engine;
#define SHOULD_QUIT ::Comfy::g_engine->shouldQuit()

} // End of namespace Comfy

#endif // COMFY_H
