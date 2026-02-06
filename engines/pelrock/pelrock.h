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
#include "pelrock/graphics.h"
#include "pelrock/menu.h"
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
	VideoManager *_videoManager = nullptr;
	SoundManager *_sound = nullptr;
	DialogManager *_dialog = nullptr;
	MenuManager *_menu = nullptr;

	void init();
	void loadAnims();

	/*
		Walking alforithm
	*/
	void walkTo(int x, int y);
	void walkAndAction(HotSpot *hotspot, VerbIcon action);
	AlfredDirection calculateAlfredsDirection(HotSpot *hotspot);

	Common::Array<VerbIcon> availableActions(HotSpot *hotspot);
	VerbIcon isActionUnder(int x, int y);
	bool isItemUnder(int x, int y);
	bool isAlfredUnder(int x, int y);
	int isHotspotUnder(int x, int y);
	Exit *isExitUnder(int x, int y);
	bool isSpriteUnder(Sprite *sprite, int x, int y);

	void showActionBalloon(int posx, int posy, int curFrame);

	void checkMouse();
	void copyBackgroundToBuffer();
	void updateAnimations();
	void presentFrame();
	void updatePaletteAnimations();
	void paintDebugLayer();
	void placeStickersFirstPass();
	void placeStickersSecondPass();
	void placeSticker(Sticker sticker);
	void renderOverlay(int overlayMode);

	void animateFadePalette(PaletteAnim *anim);
	void animateRotatePalette(PaletteAnim *anim);
	void doAction(VerbIcon action, HotSpot *hotspot);
	void talkTo(HotSpot *hotspot);
	void lookAt(HotSpot *hotspot);

	void chooseAlfredStateAndDraw();
	void drawAlfred(byte *buf);
	void drawNextFrame(Sprite *animSet);
	void animateTalkingNPC(Sprite *animSet);
	void pickupIconFlash();

	void playSoundIfNeeded();


	void gameLoop();
	void computerLoop();
	void extraScreenLoop();
	void walkLoop(int16 x, int16 y, AlfredDirection direction);

	void checkMouseHover();
	void checkMouseClick(int x, int y);
	void checkLongMouseClick(int x, int y);

	void calculateScalingMasks();
	ScaleCalculation calculateScaling(int yPos, ScalingParams scalingParams);
	byte *scale(int scaleY, int finalWidth, int finalHeight, byte *buf);

	Common::Array<Common::Array<int>> _widthScalingTable;
	Common::Array<Common::Array<int>> _heightScalingTable;

	// walking
	int _currentStep = 0;
	PathContext _currentContext;

	byte *_currentBackground = nullptr; // Clean background - NEVER modified
	byte *_extraScreen = nullptr;

	ActionPopupState _actionPopupState;

	HotSpot *_currentHotspot = nullptr;
	int _flashingIcon = -1;

	Common::Point _curWalkTarget;
	QueuedAction _queuedAction;

	bool showShadows = false;

	bool shouldPlayIntro = false;
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
	GraphicsManager *_graphics = nullptr;
	Graphics::Screen *_screen = nullptr;
	ResourceManager *_res = nullptr;
	RoomManager *_room = nullptr;
	ChronoManager *_chrono = nullptr;
	PelrockEventManager *_events = nullptr;
	AlfredState _alfredState;
	byte *_compositeBuffer = nullptr; // Working composition buffer

	bool _mouseDisabled = false;
	GameStateData *_state = new GameStateData();

	SmallFont *_smallFont = nullptr;
	LargeFont *_largeFont = nullptr;
	DoubleSmallFont *_doubleSmallFont = nullptr;


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

	void loadGame(SaveGameData &saveGame);

	SaveGameData *createSaveGameData() const;

	Common::Error saveGameStream(Common::WriteStream *stream, bool isAutosave = false) override {
		Common::Serializer s(nullptr, stream);
		return syncGame(s);
	}
	Common::Error loadGameStream(Common::SeekableReadStream *stream) override {
		Common::Serializer s(stream, nullptr);
		return syncGame(s);
	}

	void setScreen(int s, AlfredDirection dir);
	void loadExtraScreenAndPresent(int screenIndex);
	void waitForSpecialAnimation();
	bool renderScene(int overlayMode = OVERLAY_NONE);
	void frameTriggers();

	void passerByAnim(uint32 frameCount);
	void reflectionEffect(byte *buf, int x, int y, int width, int height);
	void changeCursor(Cursor cursor);

	void travelToEgypt();

	// Actions
	void doExtraActions(int roomNumber);
	void addInventoryItem(int item);
	void buyFromStore(HotSpot *hotspot, int stickerId);
	void performActionTrigger(uint16 actionTrigger);
	void dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex);

	void executeAction(VerbIcon action, HotSpot *hotspot);
	void openRoomDrawer(HotSpot *hotspot);
	void closeRoomDrawer(HotSpot *hotspot);
	void openRoomDoor(HotSpot *hotspot);
	void closeRoomDoor(HotSpot *hotspot);
	void pickUpAndDisable(HotSpot *hotspot);
	void grabKetchup(HotSpot *hotspot);
	void grabMustard(HotSpot *hotspot);
	void grabSpicey(HotSpot *hotspot);
	void openKitchenDoor(HotSpot *hotspot);
	void closeKitchenDoor(HotSpot *HotSpot);
	void openKitchenDrawer(HotSpot *hotspot);
	void openKitchenDoorFromInside(HotSpot *hotspot);
	void useSpicySauceWithBurger(int inventoryObject, HotSpot *hotspot);
	void openShopDoor(HotSpot *hotspot);
	void closeShopDoor(HotSpot *hotspot);
	void openLamppost(HotSpot *hotspot);
	void openDoor(HotSpot *hotspot, int doorIndex, int sticker, bool masculine, bool stayClosed);
	void closeDoor(HotSpot *hotspot, int doorIndex, int sticker, bool masculine, bool stayOpen);
	void pickUpPhoto(HotSpot *hotspot);
	void pickYellowBook(HotSpot *hotspot);
	void pickUpBrick(HotSpot *hotspot);
	void openIceCreamShopDoor(HotSpot *hotspot);
	void noOpAction(HotSpot *hotspot);
	void noOpItem(int item, HotSpot *hotspot);
	void useOnAlfred(int inventoryObject);
	void useCardWithATM(int inventoryObject, HotSpot *hotspot);
	void useBrickWithWindow(int inventoryObject, HotSpot *hotspot);
	void moveCable(HotSpot *hotspot);
	void useBrickWithShopWindow(int inventoryObject, HotSpot *hotspot);
	void pickGuitar(HotSpot *hotspot);
	void pickFish(HotSpot *hotspot);
	void pickTeddyBear(HotSpot *hotspot);
	void pickDiscs(HotSpot *hotspot);
	void pickMonkeyBrain(HotSpot *hotspot);
	void pickBooks(HotSpot *hotspot);
	void pickPalette(HotSpot *hotspot);
	void pickCandy(HotSpot *hotspot);
	void pickConch(HotSpot *hotspot);
	void pickHat(HotSpot *hotspot);
	void pickCord(HotSpot *hotspot);
	void pickAmulet(HotSpot *hotspot);
	void openPlug(HotSpot *hotspot);
	void useCordWithPlug(int inventoryObject, HotSpot *hotspot);
	void pickCables(HotSpot *hotspot);
	void giveIdToGuard(int inventoryObject, HotSpot *hotspot);
	void unlockMuseum();
	void giveMoneyToGuard(int inventoryObject, HotSpot *hotspot);
	void openMuseumDoor(HotSpot *hotspot);
	void useAmuletWithStatue(int inventoryObject, HotSpot *hotspot);
	void pickUpLetter(HotSpot *hotspot);
	void openLibraryOutdoorsDoor(HotSpot *hotspot);
	void closeLibraryOutdoorsDoor(HotSpot *hotspot);
	void openLibraryIndoorsDoor(HotSpot *hotspot);
	void closeLibraryIndoorsDoor(HotSpot *hotspot);
	void pickBooksFromShelf1(HotSpot *hotspot);
	void pickBooksFromShelf2(HotSpot *hotspot);
	void pickBooksFromShelf3(HotSpot *hotspot);
	void giveSecretCodeToLibrarian(int inventoryObject, HotSpot *hotspot);
	void openNewspaperDoor(HotSpot *hotspot);
	void closeNewspaperDoor(HotSpot *hotspot);
	void openNewspaperBossDor(HotSpot *hotspot);
	void closeNewspaperBossDoor(HotSpot *hotspot);
	void openTravelAgencyDoor(HotSpot *hotspot);
	void closeTravelAgencyDoor(HotSpot *hotspot);
	void pickUpBook(int i);
	void openMcDoor(HotSpot *hotspot);
	void closeMcDoor(HotSpot *hotspot);

	void animateStatuePaletteFade(bool reverse = false);
	void checkObjectsForPart2();
	void waitForActionEnd();
};

extern PelrockEngine *g_engine;
#define SHOULD_QUIT ::Pelrock::g_engine->shouldQuit()

} // End of namespace Pelrock

#endif // PELROCK_H
