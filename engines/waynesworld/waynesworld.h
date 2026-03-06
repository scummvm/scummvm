/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef WAYNESWORLD_WAYNESWORLD_H
#define WAYNESWORLD_WAYNESWORLD_H

#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/str.h"
#include "common/system.h"
#include "engines/engine.h"
#include "engines/advancedDetector.h"

#include "image/pcx.h"

#include "waynesworld/sound.h"


namespace WaynesWorld {
class GxlArchive;

#define WAYNESWORLD_SAVEGAME_VERSION 0

enum {
	GF_GUILANGSWITCH =    (1 << 0) // If GUI language switch is required for menus
};

class Screen;
class WWSurface;
class GFTFont;
class GameLogic;

struct RoomObject {
    int roomNumber;
    const char *name;
    int x1, y1, x2, y2;
    int direction;
    int walkX, walkY;
};

struct StaticRoomObjectMapEntry {
	int index, count;
};

struct StaticRoomObject {
	const char *name;
	int x1, y1, x2, y2;
};

struct WalkPoint {
    int x, y, direction;
};

struct AnimationTimer {
	uint32 nextUpdateTicks;
	uint32 delay;
	int counter;
	bool expired;
	AnimationTimer() : nextUpdateTicks(0), delay(0), counter(0), expired(false) {}
};

enum {
	kLeftButtonClicked = 1 << 0,
	kRightButtonClicked = 1 << 1,
	kKeyPressed = 1 << 2
};

const int kRoomObjectsCount = 404;
const int kWalkPointsCount = 300;
const uint kWalkMapSize = (320 * 150) / 8;
const uint kRoomAnimationsCount = 20;
const uint kStaticRoomObjectsMapCount = 40;
const uint kStaticRoomObjectsCount = 58;
const uint kStaticRoomObjectSpritesCount = 4;
const uint kAnimationTimersCount = 4;
const int kInventorySize = 50;
const int kFirstInventoryObjectId = 28;
const int kLastInventoryObjectId = 77;

class WaynesWorldEngine : public Engine {
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	WaynesWorldEngine(OSystem *syst, const ADGameDescription *gd);
	~WaynesWorldEngine() override;
	const Common::String getTargetName() { return _targetName; }
	const ADGameDescription *_gameDescription;
	bool _isSoundEnabled = true;
	bool _isMusicEnabled = true;

private:
	Graphics::PixelFormat _pixelFormat;

#ifdef USE_TRANSLATION
	Common::String _oldGUILanguage;
#endif
	void wwEffect(int arg0, int arg1, bool flag);
	
	bool introPt1();
	bool introPt2();
	bool introPt3(bool flag);
	bool introPt4();

	WWSurface *_introBackg1Image = nullptr;
	WWSurface *_introWbodyImage = nullptr;
	WWSurface *_introGbodyImage = nullptr;
	WWSurface *_introWhead1[8] = {nullptr};
	WWSurface *_introGhead1[11] = {nullptr};

	int _old_arg_refreshBackgFl = -1;
	int _old_arg_wBodyIndex = -1;
	int _old_arg_gBodyIndex = -1;
	int _old_argWHead1Index = -1;
	int _old_argGHead1Index = -1;
	
	void sub2FEFB(int arg_refreshBackgFl, int arg_wBodyIndex, int arg_gBodyIndex, int arg_wHead1Index, int arg_gHead1Index, int arg_TextId);

	void introPt4_sub1();
	bool introPt4_sub2();
	bool introPt4_sub3();
	bool introPt4_sub4();
	bool introPt4_sub5();
	bool introPt4_sub6();
	bool introPt4_sub7();
	void introPt4_sub8();
	bool introPt4_sub9();

	WWSurface *_demoPt2Surface = nullptr;
	WWSurface *_backg2Surface = nullptr;
	WWSurface *_logoSurface = nullptr;
	WWSurface *_outlineSurface = nullptr;

	GxlArchive *_oanGxl = nullptr;

public:
	Common::RandomSource *_random;
	GameLogic *_logic;

	void updateEvents();

	// Graphics
	byte _palette2[768] = {0};
	Screen *_screen = nullptr;
	WWSurface *_backgroundSurface = nullptr;
	WWSurface *_backgroundScrollSurface = nullptr;
	GFTFont *_fontWW = nullptr;
	GFTFont *_fontWWInv = nullptr;
	GFTFont *_fontBit5x7 = nullptr;
	WWSurface *_roomAnimations[kRoomAnimationsCount] = {nullptr};
	GxlArchive *_m00Gxl = nullptr;
	GxlArchive *_m01Gxl = nullptr;
	GxlArchive *_m02Gxl = nullptr;
	GxlArchive *_m03Gxl = nullptr;
	GxlArchive *_m05Gxl = nullptr;

	GxlArchive *_r10Gxl = nullptr;

	// Sound and Music
	SoundManager *_sound;
	MusicManager *_midi;

	// Room
	Common::String _roomName;
	GxlArchive *_roomGxl = nullptr;
	byte *_walkMap = nullptr;
	int _scrollWidth = 0;
	int _scrollRemaining = 0;
	int _roomChangeCtr = 0;
	int _scrollPosition = 0;
	bool _doScrollRight = false;
	bool _hasRoomAnimationCallback = false;

	// Room animations
	AnimationTimer _animationTimers[kAnimationTimersCount];

	// Input
	int _mouseX = 0, _mouseY = 0;
	int _mouseClickY, _mouseClickX;
	Common::Rect _mouseZone = {0, 0, 319, 199};
	uint _mouseClickButtons;
	Common::KeyCode _keyCode;

	// Text
	Common::String _currentText;
	int _currentTextX, _currentTextY;
	bool _isTextVisible;

	// Audio
	int _musicIndex;

	// Game
	int _gameState; // TODO Use enum
	int _currentActorNum;
	int _currentRoomNumber;
	int _verbNumber;
	int _verbNumber2;
	int _objectNumber;
	int _hoverObjectNumber;
	int _firstObjectNumber;
	Common::String _firstObjectName;
	int _roomEventNum;
	int _animationsCtr;
	bool _animationsRedrawBackground;

	// Actors
	int _wayneSpriteX, _wayneSpriteY, _wayneKind, _wayneActorScale;
	int _garthSpriteX, _garthSpriteY, _garthKind, _garthActorScale;
	int _actorSpriteValue;
	int _actorSpriteIndex;
	WWSurface *_wayneSprites[8], *_wayneWalkSprites[8][4], *_wayneReachRightSprite, *_wayneReachLeftSprite;
	WWSurface *_garthSprites[8], *_garthWalkSprites[8][4], *_garthReachRightSprite, *_garthReachLeftSprite;
	WalkPoint _wayneWalkPoints[kWalkPointsCount];
	WalkPoint _garthWalkPoints[kWalkPointsCount];

	// Inventory
	WWSurface *_inventorySprite;
	int _inventoryItemsCount;
	int _inventoryItemsObjectMap[kInventorySize];
	int _wayneInventory[kInventorySize];
	int _garthInventory[kInventorySize];

	// Dialog
	int _selectedDialogChoice;
	int _dialogChoices[5];

	// Room objects
	static const RoomObject kRoomObjects[kRoomObjectsCount];
	// _roomObjects is a writable copy of kRoomObjects
	RoomObject _roomObjects[kRoomObjectsCount];

	// Static room objects
	static const StaticRoomObjectMapEntry kStaticRoomObjectsMap[kStaticRoomObjectsMapCount];
	static const StaticRoomObject kStaticRoomObjects[kStaticRoomObjectsCount];
	// _staticRoomObjects is a writable copy of kStaticRoomObjects
	StaticRoomObject _staticRoomObjects[kStaticRoomObjectsCount];
	WWSurface *_staticRoomObjectSprites[kStaticRoomObjectSpritesCount];

	// Game map
	int _gameMapRoomNumber;
	int _gameMapWayneSpriteX, _gameMapGarthSpriteX;
	int _currentMapItemIndex;
	int _gameMapDestinationRoomNum;
	bool _gameMapFlag;

	void runIntro();

	// Utils
	int getRandom(int max);
	void waitMillis(uint millis);
	void waitSeconds(uint seconds);

	// Input handling
	void initMouseCursor();
	bool isPointAtWayne(int x, int y);
	bool isPointAtGarth(int x, int y);
	void updateMouseMove();
	void handleMouseClick();
	void handleMouseLeftClick();
	void handleMouseRightClick();

	// Palette loading
	void loadPalette(GxlArchive *lib, const char *filename);

	void paletteFadeIn(int index, int count, int stepsSize);
	void paletteFadeOut(int index, int count, int stepsSize);

	// Image drawing
	void drawImageToSurfaceIntern(GxlArchive *lib, const char *filename, WWSurface *destSurface, int x, int y, bool transparent);
	void drawImageToScreenIntern(GxlArchive *lib, const char *filename, int x, int y, bool transparent);
	void drawImageToScreen(GxlArchive *lib, const char *filename, int x, int y);
	void drawImageToSurface(GxlArchive *lib, const char *filename, WWSurface *destSurface, int x, int y);
	void drawRoomImageToBackground(const char *filename, int x, int y);
	void drawRoomImageToBackgroundTransparent(const char *filename, int x, int y);
	void drawRoomImageToScreen(const char *filename, int x, int y);
	void drawRoomImageToSurface(const char *filename, WWSurface *destSurface, int x, int y);

	void drawSpiralEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight);
	void drawRandomEffect(Graphics::Surface *surface, int x, int y, int grainWidth, int grainHeight);

	// Text
	Common::String loadString(const char *filename, int index, int flag);
	void drawCurrentTextToSurface(WWSurface *destSurface, int x, int y);
	void drawCurrentText(int x, int y, WWSurface *destSurface);
	void displayText(const char *filename, int index, int flag, int x, int y, int drawToVirtual);
	void displayTextLines(const char *filename, int baseIndex, int x, int y, int count);

	// Audio
	void playSound(const char *filename, int flag);
	void changeMusic();
	void stopMusic();

	// Interface
	void drawInterface(int verbNum);
	void selectVerbNumber2(int x);
	void selectVerbNumber(int x);
	void changeActor();
	void drawVerbLine(int verbNumber, int objectNumber, const char *objectName);
	void rememberFirstObjectName(int objectId);

	// Inventory
	void redrawInventory();
	void refreshInventory(bool doRefresh);
	void drawInventory();
	void setWayneInventoryItemQuantity(int objectId, int quantity);
	void setGarthInventoryItemQuantity(int objectId, int quantity);
	int getWayneInventoryItemQuantity(int objectId);
	int getGarthInventoryItemQuantity(int objectId);

	// Actors and animations
	void loadMainActorSprites();
	void unloadMainActorSprites();
	int getActorScaleFromY(int actorY);
	void drawActorReachObject(int objectId, int spriteIndex);
	int drawActors(int direction, int wayneKind, int garthKind, int spriteIndex, int wayneX, int wayneY, int garthX, int garthY);
	void refreshActors();
	void pickupObject(int objectId, byte &flags, byte flagsSet, int inventoryObjectId);
	void playAnimation(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks);
	void playAnimationLoops(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks, int loopCount);
	void setWaynePosition(int x, int y);
	void setGarthPosition(int x, int y);
	bool isActorWayne();
	bool isActorGarth();
	void selectActorWayne();
	void selectActorGarth();
	void toggleActor();

	// Pathfinding
	bool walkIsPixelWalkable(int x, int y);
	bool walkAdjustDestPoint(int &x, int &y);
	void walkGetNextPoint(int sourceX, int sourceY, int destX, int destY, int &nextX, int &nextY);
	void walkCalcOtherActorDest(int flag, int &x, int &y);
	int walkCalcPath(int flag, int sourceX, int sourceY, int destX, int destY, int pointsCount);
	bool walkFindPoint(int flag, int &sourceX, int &sourceY, int &nextSourceX, int &nextSourceY, int destX, int destY, int pointsCount);
	int walkAddWalkLine(int flag, int x1, int y1, int x2, int y2, int pointsCount);
	bool walkTestPoint(int sourceX, int sourceY, int nextSourceX, int nextSourceY, int destX, int destY);
	bool walkIsLineWalkable(int sourceX, int sourceY, int destX, int destY);
	int walkCalcDirection(int deltaX, int deltaY);
	bool walkTo(int actor1_destX, int actor1_destY, int direction, int actor2_destX, int actor2_destY);

	void gxCloseLib(GxlArchive *lib);
	void setMouseBounds(int x1, int x2, int y1, int y2);
	// Room
	void openRoomLibrary(int roomNum);
	void loadRoomBackground();
	void changeRoom(int roomNum);
	void refreshRoomBackground(int roomNum);
	void handleRoomEvent();
	void changeRoomScrolling();
	void loadScrollSprite();
	void scrollRoom();
	void loadRoomMask(int roomNum);
	void fillRoomMaskArea(int x1, int y1, int x2, int y2, bool blocked);

	// Room animations
	void loadAnimationSpriteRange(int baseIndex, const char *filename, int count);
	void loadAnimationSprite(int index, const char *filename);
	void drawAnimationSprite(int index, int x, int y);
	void drawAnimationSpriteTransparent(int index, int x, int y);
	void updateRoomAnimations();
	void startRoomAnimations();
	void stopRoomAnimations();
	void updateAnimationTimers();
	void setAnimationTimer(uint index, uint32 delay, int initialCounter = 0);
	bool isAnimationTimerExpired(uint index);
	int getAnimationTimerCounter(uint index);

	// Static room objects
	void initStaticRoomObjects();
	void loadStaticRoomObjects(int roomNum);
	void unloadStaticRoomObjects();
	void setStaticRoomObjectPosition(int roomNum, int fromIndex, int toIndex, int x, int y);
	void drawStaticRoomObjects(int roomNum, int x, int y, int actorHeight, int actorWidth, WWSurface *surface);

	// Room objects
	void initRoomObjects();
	void moveObjectToRoom(int objectId, int roomNum);
	void moveObjectToNowhere(int objectId);
	const RoomObject *getRoomObject(int objectId);
	const char *getRoomObjectName(int objectId);
	int getObjectRoom(int objectId);
	int getObjectDirection(int objectId);
	int findRoomObjectIdAtPoint(int x, int y);
	void walkToObject();

	// Dialog
	void startDialog();
	void setDialogChoices(int choice1, int choice2, int choice3, int choice4, int choice5);
	void drawDialogChoices(int choiceIndex);
	void handleDialogMouseClick();

	// Verb handlers
	void handleVerb(int verbFlag);
	void handleVerbPickUp();
	void handleVerbLookAt();
	void handleVerbUse();
	void handleVerbTalkTo();
	void handleVerbPush();
	void handleVerbPull();
	void handleVerbExtremeCloseupOf();
	void handleVerbGive();
	void handleVerbOpen();
	void handleVerbClose();

	void lookAtUnusedTicket();
	void unusedTicketHandleMouseMove();
	void unusedTicketHandleMouseClick();

	void extremeCloseUpHandleMouseClick();

	void gameMapOpen();
	void gameMapFinish();
	void gameMapHandleMouseMove(int objectNumber);
	void gameMapHandleMouseClick();
	void gameMapSelectItem(const char *prefix, int animX, int animY);

	// Savegame API

	enum kReadSaveHeaderError {
		kRSHENoError = 0,
		kRSHEInvalidType = 1,
		kRSHEInvalidVersion = 2,
		kRSHEIoError = 3
	};

	struct SaveHeader {
		Common::String description;
		uint32 version;
		byte gameID;
		uint32 flags;
		uint32 saveDate;
		uint32 saveTime;
		uint32 playTime;
		Graphics::Surface *thumbnail;
	};

	bool _isSaveAllowed;

	/* TODO
	bool canLoadGameStateCurrently() override { return _isSaveAllowed; }
	bool canSaveGameStateCurrently() override { return _isSaveAllowed; }
	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false) override;
	void savegame(const char *filename, const char *description);
	void loadgame(const char *filename);
	bool existsSavegame(int num);
	static Common::String getSavegameFilename(const Common::String &target, int num);
	WARN_UNUSED_RESULT static kReadSaveHeaderError readSaveHeader(Common::SeekableReadStream *in, SaveHeader &header, bool skipThumbnail = true);
	*/

};

} // End of namespace WaynesWorld

#endif // WAYNESWORLD_WAYNESWORLD_H
