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
#include "common/file.h"
#include "common/fs.h"
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
#include "pelrock/fonts/large_font.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/room.h"
#include "pelrock/types.h"

namespace Pelrock {

struct PelrockGameDescription;

const int kAlfredFrameWidth = 51;
const int kAlfredFrameHeight = 102;

class PelrockEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;

	void init();
	void playIntro();
	void setScreen(int s, int dir);
	void setScreenJava(int s, int dir);
	void loadAnims();

	void loadAlfredAnims();

	void walkTo(int x, int y);
	bool pathFind(int x, int y, PathContext *context);
	uint8_t find_walkbox_for_point(uint16_t x, uint16_t y);
	bool point_in_walkbox(WalkBox *box, uint16_t x, uint16_t y);
	uint16_t build_walkbox_path(uint8_t start_box, uint8_t dest_box, uint8_t *path_buffer);
	uint8_t get_adjacent_walkbox(uint8_t current_box_index);
	void clear_visited_flags();
	uint16_t generate_movement_steps(uint8_t *path_buffer,
									 uint16_t path_length,
									 uint16_t start_x, uint16_t start_y,
									 uint16_t dest_x, uint16_t dest_y,
									 MovementStep *movement_buffer);

	void talk(byte object);
	void loadCursors();
	void loadInteractionIcons();
	byte *grabBackgroundSlice(int x, int y, int w, int h);
	void putBackgroundSlice(int x, int y, int w, int h, byte *slice);

	Common::Array<VerbIcons> availableActions(HotSpot *hotspot);
	Common::Point calculateWalkTarget(int mouseX, int mouseY);
	void drawText(Common::String text, int x, int y, int w, byte color);

	void sayAlfred(Common::String text);
	void sayNPC(AnimSet *anim, Common::String text, byte color);
	// render loop
	void frames();
	void doAction(byte action, byte object);
	void renderText(Common::Array<Common::String> lines, int color, int x, int y);
	void drawAlfred(byte *buf);
	void drawNextFrame(AnimSet *animSet);
	void checkMouseHover();
	void checkMouseClick(int x, int y);
	void checkLongMouseClick(int x, int y);
	void changeCursor(Cursor cursor);
	int isHotspotUnder(int x, int y);
	Exit *isExitUnder(int x, int y);
	AnimSet *isSpriteUnder(int x, int y);
	void showActionBalloon(int posx, int posy, int curFrame);
	void talkNPC(AnimSet *animSet);

	ChronoManager *_chronoManager = nullptr;

	byte **walkingAnimFrames[4];              // 4 arrays of arrays
	byte *standingAnimFrames[4] = {nullptr};  // 4 directions
	int walkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array
	byte **talkingAnimFrames[4];              // 4 arrays of arrays
	int talkingAnimLengths[4] = {8, 8, 4, 4}; // size of each inner array


	PathContext _currentContext;
	int _current_step = 0;

	byte _textColor = 0;
	Common::Point _textPos;
	Common::Array<Common::Array<Common::String> > _currentTextPages = Common::Array<Common::Array<Common::String> >();
	int _currentTextPageIndex = 0;

	int *_currentAnimFrames = nullptr;
	// From the original code
	int xAlfred = 319;
	int yAlfred = 302;
	int dirAlfred = 0;
	int curAlfredFrame = 0;
	uint16 mouseX = 0;
	uint16 mouseY = 0;
	byte *_cursorMasks[5] = {nullptr};

	uint32 _mouseClickTime;
	bool _isMouseDown = false;
	bool _longClick = false;

	byte *_verbIcons[9] = {nullptr};
	byte *_popUpBalloon = nullptr;

	byte *_currentBackground; // Clean background - NEVER modified
	byte *_compositeBuffer;   // Working composition buffer

	bool _lMouseDown = false;
	bool _displayPopup = false;
	int _popupX = 0;
	int _popupY = 0;
	int _currentPopupFrame = 0;
	HotSpot *_currentHotspot = nullptr;

	SmallFont *_smallFont = nullptr;
	LargeFont *_largeFont = nullptr;

	Common::Point _curWalkTarget;
	bool isNPCATalking = false;
	uint16 whichNPCTalking = 0;
	bool isNPCBTalking = false;

	// JAVA
	bool shouldPlayIntro = false;
	GameState stateGame = GAME;
	bool gameInitialized = false;
	bool screenReady = false;
	// int prevDirX = 0;
	// int prevDirY = 0;
	// Common::String objectToShow = "";
	// int prevWhichScreen = 0;
	// int whichScreen = 0;
	// byte *pixelsShadows; // =new int[640*400];

	RoomManager *_room = nullptr;

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	bool isAlfredWalking = false;
	bool isAlfredTalking = false;

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
