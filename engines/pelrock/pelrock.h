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
#include "pelrock/dialog.h"
#include "pelrock/events.h"
#include "pelrock/fonts/large_font.h"
#include "pelrock/fonts/small_font.h"
#include "pelrock/fonts/small_font_double.h"
#include "pelrock/resources.h"
#include "pelrock/room.h"
#include "pelrock/sound.h"
#include "pelrock/types.h"
#include "pelrock/video/video.h"

namespace Pelrock {

struct PelrockGameDescription;

class PelrockEngine : public Engine {
private:
	const ADGameDescription *_gameDescription;
	Common::RandomSource _randomSource;
	ResourceManager *_res = nullptr;
	ChronoManager *_chrono = nullptr;
	VideoManager *_videoManager = nullptr;
	SoundManager *_sound = nullptr;
	PelrockEventManager *_events = nullptr;
	DialogManager *_dialog = nullptr;

	void init();
	void loadAnims();

	/*
		Walking alforithm
	*/
	void walkTo(int x, int y);

	void talk(byte object);
	void sayAlfred(Common::String text);
	void sayNPC(Sprite *anim, Common::String text, byte color);

	byte *grabBackgroundSlice(int x, int y, int w, int h);
	void putBackgroundSlice(int x, int y, int w, int h, byte *slice);

	Common::Array<VerbIcon> availableActions(HotSpot *hotspot);
	VerbIcon isActionUnder(int x, int y);
	bool isAlfredUnder(int x, int y);
	int isHotspotUnder(int x, int y);
	Exit *isExitUnder(int x, int y);
	Sprite *isSpriteUnder(int x, int y);
	void showActionBalloon(int posx, int posy, int curFrame);

	void checkMouse();
	void copyBackgroundToBuffer();
	void updateAnimations();
	void presentFrame();
	void updatePaletteAnimations();
	void paintDebugLayer();


	void animateFadePalette(PaletteAnim *anim);
	void animateRotatePalette(PaletteAnim *anim);
	void doAction(byte action, HotSpot *hotspot);
	void talkTo(HotSpot *hotspot);
	void lookAt(HotSpot *hotspot);
	void open(HotSpot *hotspot);
	void renderText(Common::Array<Common::String> lines, int color, int x, int y);
	void chooseAlfredStateAndDraw();
	void drawAlfred(byte *buf);
	void drawNextFrame(Sprite *animSet);
	void changeCursor(Cursor cursor);
	void drawTalkNPC(Sprite *animSet);
	void playSoundIfNeeded();

	void gameLoop();
	void menuLoop();

	void checkMouseHover();
	void checkMouseClick(int x, int y);
	void checkLongMouseClick(int x, int y);
	void checkMouseClickOnSettings(int x, int y);

	void calculateScalingMasks();
	ScaleCalculation calculateScaling(int yPos, ScalingParams scalingParams);

	Common::Array<Common::Array<int>> _widthScalingTable;
	Common::Array<Common::Array<int>> _heightScalingTable;

	// walking
	int _currentStep = 0;
	PathContext _currentContext;

	// text display
	byte _textColor = 0;
	Common::Point _textPos;
	int _textDurationFrames = 0;
	Common::Array<Common::Array<Common::String>> _currentTextPages = Common::Array<Common::Array<Common::String>>();
	int _currentTextPageIndex = 0;

	// Alfred
	bool alfredFrameSkip = false;
	bool isAlkfredWalking = false;

	byte *_currentBackground = nullptr; // Clean background - NEVER modified

	bool _displayPopup = false;
	byte _iconBlink = 0;
	int _popupX = 0;
	int _popupY = 0;
	int _currentPopupFrame = 0;

	HotSpot *_currentHotspot = nullptr;


	Common::Point _curWalkTarget;
	bool isNPCATalking = false;
	uint16 whichNPCTalking = 0;
	bool isNPCBTalking = false;

	bool showShadows = false;

	// Temporary
	int selectedInvIndex = 0;
	int curInventoryPage = 0;
	Common::String _menuText;

	// JAVA
	bool shouldPlayIntro = false;
	GameState stateGame = INTRO;
	bool inConversation = false;
	bool gameInitialized = false;
	bool screenReady = false;

	// int prevDirX = 0;
	// int prevDirY = 0;
	// Common::String objectToShow = "";
	// int prevWhichScreen = 0;
	// int whichScreen = 0;
	// byte *pixelsShadows; // =new int[640*400];

protected:
	// Engine APIs
	Common::Error run() override;

public:
	Graphics::Screen *_screen = nullptr;
	RoomManager *_room = nullptr;
	AlfredState alfredState;
	byte *_compositeBuffer;             // Working composition buffer

	SmallFont *_smallFont = nullptr;
	LargeFont *_largeFont = nullptr;
	DoubleSmallFont *_doubleSmallFont = nullptr;
	void renderScene(bool showTextOverlay = false);

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

	void setScreen(int s, AlfredDirection dir);
};

extern PelrockEngine *g_engine;
#define SHOULD_QUIT ::Pelrock::g_engine->shouldQuit()

} // End of namespace Pelrock

#endif // PELROCK_H
