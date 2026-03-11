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
#include "graphics/managed_surface.h"
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
	MenuManager *_menu = nullptr;

	void init();
	void loadInventoryArrows();
	void loadAnims();

	/*
		Walking alforithm
	*/
	void walkTo(int x, int y);
	void walkAndAction(HotSpot *hotspot, VerbIcon action);
	AlfredDirection calculateAlfredsDirection(HotSpot *hotspot);

	Common::Array<VerbIcon> availableActions(HotSpot *hotspot);
	VerbIcon isActionUnder(int x, int y);
	bool isAlfredUnder(int x, int y);
	int isHotspotUnder(int x, int y);
	Exit *isExitUnder(int x, int y);
	bool isSpriteUnder(Sprite *sprite, int x, int y);

	void showActionBalloon(int posx, int posy, int curFrame);

	int getScrollPositionForItem(int item);

	void checkMouse();
	void updateAnimations();
	void renderOverlay(int overlayMode);

	void doAction(VerbIcon action, HotSpot *hotspot);
	void talkTo(HotSpot *hotspot);
	void lookAt(HotSpot *hotspot);

	void chooseAlfredStateAndDraw();
	void exitTriggers(Pelrock::Exit *exit);
	void drawIdleFrame();
	void drawAlfred(byte *buf);
	void drawNextFrame(Sprite *animSet);
	void animateTalkingNPC(Sprite *animSet);
	void pickupIconFlash();

	void playSoundIfNeeded();

	void showInventoryOverlay();

	void checkMouseOverInventoryOverlay(int x, int y);
	int checkMouseClickInventoryOverlay(int x, int y);

	void gameLoop();
	void firstScene();
	void computerLoop();
	void extraScreenLoop();
	void walkLoop(int16 x, int16 y, AlfredDirection direction);

	void checkMouseHover();
	void checkMouseClick(int x, int y);
	void checkLongMouseClick(int x, int y);
	void paintDebugLayer();

	// walking
	int _currentStep = 0;
	PathContext _currentContext = {nullptr, nullptr, 0, 0, 0};

	ActionPopupState _actionPopupState;
	InventoryOverlayState _inventoryOverlayState;


	HotSpot *_currentHotspot = nullptr;
	int _newItem = -1;

	Common::Point _curWalkTarget;
	QueuedAction _queuedAction = {NO_ACTION, -1, false, false};

	bool showShadows = false;
	bool shouldPlayIntro = false;
	bool gameInitialized = false;
	bool screenReady = false;

	Common::String _hoveredMapLocation = "";
	byte *_alfredSprite = nullptr;

	int _numPressedX = 0;

	bool _mouseDisabled = false;

	// Original timing: counter-based render-skip to replicate process_game_state(N) slowdown.
	// _renderSkipAmount = N means skip N ticks, then render 1 → (1+N) ticks per render.
	int _renderSkipAmount = 0;
	int _renderSkipCounter = 0;

	int _flightFrameCounter = 0;
	int _flightSorcererSpriteIdx = -1;
	bool _flightSorcererAppeared = false;
	bool _flightSpellCast = false;
	int _flightSpellFrameCounter = 0;
	bool _flightInBlockingAnim = false;
	bool _disableAmbientSounds = false;
	bool _isDogPeeing = false;
	bool _disableAction = false;

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
	DialogManager *_dialog = nullptr;
	AlfredState _alfredState;
	ShakeEffectState _shakeEffectState;
	byte _npcTalkSpeedByte = 0;
	Graphics::ManagedSurface _compositeBuffer; // Working composition buffer
	Graphics::ManagedSurface _currentBackground; // Clean background - NEVER modified
	Graphics::ManagedSurface _bgScreen;
	Graphics::Surface _saveThumbnail;

	GameStateData *_state = new GameStateData();
	bool _saveAllowed = true;
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

	/**
	 * Returns true if "Alternate timing" option is enabled.
	 * When false, the engine uses the original game's half-speed walking/talking timing.
	 */
	bool isAlternateTiming() const;

	bool hasFeature(EngineFeature f) const override {
		return (f == kSupportsLoadingDuringRuntime) ||
			   (f == kSupportsSavingDuringRuntime) ||
			   (f == kSupportsReturnToLauncher);
	};

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override {
		return true;
	}
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override {
		return _saveAllowed;
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

	void setScreen(int s);
	void setScreenAndPrepare(int s, AlfredDirection dir);
	void loadExtraScreenAndPresent(int screenIndex);
	void waitForSpecialAnimation();
	bool renderScene(int overlayMode = OVERLAY_NONE);
	void mouseHoverForMap();
	void frameTriggers();
	void maybeHaveDogPee();
	void maybePlayPostIntro();
	void shakeEffect();
	void handleFlightRoomFrame();

	void passerByAnim(uint32 frameCount);
	void changeCursor(Cursor cursor);

	void travelToEgypt();

	bool shouldSkipFrame();

	// Actions
	void doExtraActions(int roomNumber);
	void pyramidCollapse();
	void endingScene();
	void credits();
	void initGodsSequences(int roomNumber);
	void addInventoryItem(int item);
	void buyFromStore(HotSpot *hotspot, int stickerId);
	void performActionTrigger(uint16 actionTrigger);
	void dialogActionTrigger(uint16 actionTrigger, byte room, byte rootIndex);

	void turnLightsOff();

	void givenItems();
	void advanceQuotesConversation(byte rootIndex, byte room);
	void toJail();

	void executeAction(VerbIcon action, HotSpot *hotspot);
	void openRoomDrawer(HotSpot *hotspot);
	void closeRoomDrawer(HotSpot *hotspot);
	void openClosedDrawer(HotSpot *hotspot);
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
	void pickupGarbageCan(HotSpot *hotspot);
	void noOpAction(HotSpot *hotspot);
	void noOpItem(int item, HotSpot *hotspot);
	void useOnAlfred(int inventoryObject);
	void chooseCorrectDoor();
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
	void showIdToGuard(int inventoryObject, HotSpot *hotspot);
	void unlockMuseum();
	void giveMoneyToGuard(int inventoryObject, HotSpot *hotspot);
	void openMuseumDoor(HotSpot *hotspot);
	void closeMuseumDoor(HotSpot *hotspot);
	void pickupFruit(HotSpot *hotspot);
	void useAmuletWithStatue(int inventoryObject, HotSpot *hotspot);
	void useSecretCodeWithStatue(int inventoryObject, HotSpot *hotspot);
	void pickUpLetter(HotSpot *hotspot);
	void openLibraryOutdoorsDoor(HotSpot *hotspot);
	void closeLibraryOutdoorsDoor(HotSpot *hotspot);
	void openLibraryIndoorsDoor(HotSpot *hotspot);
	void closeLibraryIndoorsDoor(HotSpot *hotspot);
	void pickBooksFromShelf1(HotSpot *hotspot);
	void pickBooksFromShelf2(HotSpot *hotspot);
	void pickBooksFromShelf3(HotSpot *hotspot);
	void giveSecretCodeToLibrarian(int inventoryObject, HotSpot *hotspot);
	void useBrickWithLibrarian(int inventoryObject, HotSpot *hotspot);
	void openNewspaperDoor(HotSpot *hotspot);
	void closeNewspaperDoor(HotSpot *hotspot);
	void openNewspaperBossDor(HotSpot *hotspot);
	void closeNewspaperBossDoor(HotSpot *hotspot);
	void openTravelAgencyDoor(HotSpot *hotspot);
	void closeTravelAgencyDoor(HotSpot *hotspot);
	void usePumpkinWithRiver(int inventoryObject, HotSpot *hotspot);
	void playAlfredSpecialAnim(int anim, bool reverse = false);
	void waitForSoundEnd(int channel = 0);
	void pickupSunflower(HotSpot *hotspot);
	void checkIngredients();
	void pickUpBook(int i);
	void pickUpChainsaw(HotSpot *hotspot);
	void pickUpSpellbook(HotSpot *hotspot);
	void pickUpBoot(HotSpot *hotspot);
	void pickupCondoms(HotSpot *hotspot);
	void openEgyptMuseumDoor(HotSpot *hotspot);
	void closeEgyptMuseumDoor(HotSpot *hotspot);
	void pushSymbol1(HotSpot *hotspot);
	void pushSymbol2(HotSpot *hotspot);
	void pushSymbol3(HotSpot *hotspot);
	void pushSymbol4(HotSpot *hotspot);
	void pickUpHairStrand(HotSpot *hotspot);
	void openJailFloorTile(HotSpot *hotspot);
	void openTunnelDrawer(HotSpot *hotspot);
	void useKeyWithPortrait(int inventoryObject, HotSpot *hotspot);
	void openSafe(HotSpot *hotspot);
	void openTunnelDoor(HotSpot *hotspot);
	void closeTunnelDoor(HotSpot *hotspot);
	void useDollWithBed(int inventoryObject, HotSpot *hotspot);
	void giveMagazineToGuard(int inventoryObject, HotSpot *hotspot);
	void giveWaterToGuard(int inventoryObject, HotSpot *hotspot);
	void guardMovement();
	void pickUpStone(HotSpot *hotspot);
	void playSpecialAnim(uint32 offset, bool compressed, int x, int y, int width, int height, int numFrames);
	void giveStoneToSlaves(int inventoryObject, HotSpot *hotspot);
	void swimmingPoolCutscene(HotSpot *hotspot);
	void pickUpStones(HotSpot *hotspot);
	void pickUpMud(HotSpot *hotspot);
	void openPyramidDoor(HotSpot *hotspot);
	void usePumpkinWithPond(int inventoryObject, HotSpot *hotspot);
	void useWaterOnFakeStone(int inventoryObject, HotSpot *hotspot);
	void useWigWithPot(int inventoryObject, HotSpot *hotspot);
	void magicFormula(int inventoryObject, HotSpot *hotspot);
	void smokeAnimation(int spriteIndex, bool hide = true);
	// void endgameTransportAnimation();
	void openArchitectDoor(HotSpot *hotspot);
	void closeArchitectDoor(HotSpot *hotspot);
	void pickupPyramidMap(HotSpot *hotspot);
	void openArchitectDoorFromInside(HotSpot *hotspot);
	void closeArchitectDoorFromInside(HotSpot *hotspot);
	void checkAllSymbols();
	void openMcDoor(HotSpot *hotspot);
	void closeMcDoor(HotSpot *hotspot);
	void pickupBush(HotSpot *hotspot);
	void teleportToPrincess();

	void animateStatuePaletteFade(bool reverse = false);
	void pickUpMatches(HotSpot *hotspot);
	void antiPiracyEffect();
	void checkObjectsForPart2();
	void waitForActionEnd();
};

extern PelrockEngine *g_engine;
#define SHOULD_QUIT ::Pelrock::g_engine->shouldQuit()

} // End of namespace Pelrock

#endif // PELROCK_H
