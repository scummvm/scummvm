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

#ifndef PELROCK_H
#define PELROCK_H

#include "common/error.h"
#include "common/fs.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/random.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/system.h"
#include "common/util.h"
#include "engines/engine.h"
#include "engines/savestate.h"
#include "graphics/screen.h"
#include "image/png.h"

#include "pelrock/chrono.h"
#include "pelrock/detection.h"
#include "pelrock/types.h"

namespace Pelrock {

struct PelrockGameDescription;

const int kAlfredFrameWidth = 51;
const int kAlfredFrameHeight = 102;

class PelrockEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	Image::PNGDecoder *decoder = new Image::PNGDecoder();
	void init();
	void playIntro();
	void setScreen(int s, int dir);
	void setScreenJava(int s, int dir);
	void loadAnims();
	// Room data
	void getPalette(Common::File *roomFile, int roomOffset, byte *palette);
	void getBackground(Common::File *roomFile, int roomOffset, byte *background);
	Common::List<AnimSet> getRoomAnimations(Common::File *roomFile, int roomOffset);
	void loadHotspots(Common::File *roomFile, int roomOffset);
	void loadMainCharacterAnims();
	Common::List<WalkBox> loadWalkboxes(Common::File *roomFile, int roomOffset);

	// render loop
	void frames();
	void checkMouseHover();

	ChronoManager *_chronoManager = nullptr;
	byte *standingAnim = new byte[3060 * 102];
	Common::List<HotSpot> _hotspots;
	Common::List<AnimSet> _currentRoomAnims;
	int *_currentAnimFrames = nullptr;
	int curAlfredFrame = 9;
	uint16 mouseX = 0;
	uint16 mouseY = 0;
	byte *_currentBackground = nullptr;

	// From the original code
	int xAlfred = 200;
	int yAlfred = 200;
	bool shouldPlayIntro = false;
	GameState stateGame = GAME;
	bool gameInitialized = false;
	bool screenReady = false;
	int dirAlfred = 0;
	int prevDirX = 0;
	int prevDirY = 0;
	Common::String objectToShow = "";
	int prevWhichScreen = 0;
	int whichScreen = 0;
	byte *pixelsShadows; // =new int[640*400];
protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;

public:
	PelrockEngine(OSystem *syst, const ADGameDescription *gameDesc);
	~PelrockEngine() override;

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
		return (f == kSupportsLoadingDuringRuntime) ||
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

extern PelrockEngine *g_engine;
#define SHOULD_QUIT ::Pelrock::g_engine->shouldQuit()

} // End of namespace Pelrock

#endif // PELROCK_H
