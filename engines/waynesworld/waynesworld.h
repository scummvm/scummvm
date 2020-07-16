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

#include "audio/mixer.h"
#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "engines/engine.h"

#include "image/pcx.h"

struct ADGameDescription;

namespace WaynesWorld {

#define WAYNESWORLD_SAVEGAME_VERSION 0

enum {
	GF_GUILANGSWITCH =    (1 << 0) // If GUI language switch is required for menus
};

class Screen;
class WWSurface;

struct RoomObject {
    int roomNumber;
    const char *name;
    int x1, y1, x2, y2;
    int direction;
    int walkX, walkY;
};

const uint kRoomObjectsCount = 404;

class WaynesWorldEngine : public Engine {
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	WaynesWorldEngine(OSystem *syst, const ADGameDescription *gd);
	~WaynesWorldEngine() override;
	const Common::String getTargetName() { return _targetName; }
	const ADGameDescription *_gameDescription;

private:
	Graphics::PixelFormat _pixelFormat;

#ifdef USE_TRANSLATION
	Common::String _oldGUILanguage;
#endif

public:
	Common::RandomSource *_random;

	void updateEvents();

	// Graphics
	byte _palette2[768];
	Screen *_screen;
	WWSurface *_backgroundSurface;

	// Room
	Common::String _roomName;
	int _word_306DB;
	int _byte_306E7;
	int _from_x1;
	bool _doScrollRight;

	// Input
	int _mouseX, _mouseY;
	int _mouseClickY, _mouseClickX;
	// uint _mouseButtons;
	uint _mouseClickButtons;
	Common::KeyCode _keyCode;

	// Text
	int _currentTextX, _currentTextY;
	bool _isTextVisible;

	// Game
	int _gameState;
	int _currentActorNum;
	int _currentRoomNumber;
	int _verbNumber;
	int _verbNumber2;
	int _objectNumber;
	int _hoverObjectNumber;
	int _firstObjectNumber;
	Common::String _firstObjectName;
	int _roomEventNum;

	// Actors
	int _wayneSpriteX, _wayneSpriteY, _wayneKind, _wayneActorScale;
	int _garthSpriteX, _garthSpriteY, _garthKind, _garthActorScale;
	int _actorSpriteValue;
	int _actorSpriteIndex;
	WWSurface *_wayneSprites[8], *_wayneWalkSprites[8][4], *_wayneReachRightSprite, *_wayneReachLeftSprite;
	WWSurface *_garthSprites[8], *_garthWalkSprites[8][4], *_garthReachRightSprite, *_garthReachLeftSprite;

	// Inventory
	WWSurface *_inventorySprite;
	int _inventoryItemsCount;
	int _inventoryItemsObjectMap[50];
	int _wayneInventory[50];
	int _garthInventory[50];

	// Dialog
	int _selectedDialogChoice;
	int _dialogChoices[5];

	// Room objects
	static const RoomObject kRoomObjects[kRoomObjectsCount];
	// _roomObjects is a writable copy of kRoomObjects
	RoomObject _roomObjects[kRoomObjectsCount];

	// Utils
	int getRandom(int max);
	void waitMillis(uint millis);
	void waitSeconds(uint seconds);

	void initMouseCursor();

	// Image loading
	Image::PCXDecoder *loadImage(const char *filename, bool appendRoomName);
	WWSurface *loadSurfaceIntern(const char *filename, bool appendRoomName);
	WWSurface *loadSurface(const char *filename);
	WWSurface *loadRoomSurface(const char *filename);
	void loadPalette(const char *filename);

	// Image drawing
	void drawImageToSurfaceIntern(const char *filename, WWSurface *destSurface, int x, int y, bool transparent, bool appendRoomName);
	void drawImageToScreenIntern(const char *filename, int x, int y, bool transparent, bool appendRoomName);
	void drawImageToBackground(const char *filename, int x, int y);
	void drawImageToBackgroundTransparent(const char *filename, int x, int y);
	void drawImageToScreen(const char *filename, int x, int y);
	void drawImageToSurface(const char *filename, WWSurface *destSurface, int x, int y);
	void drawRoomImageToBackground(const char *filename, int x, int y);
	void drawRoomImageToBackgroundTransparent(const char *filename, int x, int y);
	void drawRoomImageToScreen(const char *filename, int x, int y);
	void drawRoomImageToSurface(const char *filename, WWSurface *destSurface, int x, int y);

	// Text
	void loadString(const char *filename, int index, int flag);
	void drawCurrentTextToSurface(WWSurface *destSurface, int x, int y);
	void drawCurrentText(int x, int y, WWSurface *destSurface);
	void displayText(const char *filename, int index, int flag, int x, int y, int drawToVirtual);
	void displayTextLines(const char *filename, int baseIndex, int x, int y, int count);

	// Audio
	void playSound(const char *filename, int flag);

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

	// Actors and animations
	int getActorScaleFromY(int actorY);
	void drawActorReachObject(int objectId, int spriteIndex);
	int drawActors(int direction, int wayneKind, int garthKind, int spriteIndex, int wayneX, int wayneY, int garthX, int garthY);
	void refreshActors();
	void pickupObject(int objectId, byte &flags, byte flagsSet, int inventoryObjectId);
	void playAnimation(const char *prefix, int startIndex, int count, int x, int y, int flag, uint ticks);
	bool walkTo(int actor1_destX, int actor1_destY, int direction, int actor2_destX, int actor2_destY);

	// Room
	void openRoomLibrary(int roomNum);
	void loadRoomBackground(int roomNum);
	void changeRoom(int roomNum);
	void refreshRoomBackground(int roomNum);
	void changeRoomScrolling();
	void loadScrollSprite();
	void loadRoomMask(int roomNum);

	void updateRoomAnimations(bool doUpdate);

	void loadStaticRoomObjects(int roomNum);
	void unloadStaticRoomObjects();
	void drawStaticRoomObjects(int roomNumber, int x, int y, int actorHeight, int actorWidth, WWSurface *surface);

	// Room objects
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

	// Game logic
	int logic_handleVerbPickUp();
	int logic_handleVerbUse();
	void logic_handleVerbTalkTo();
	int logic_handleVerbPush();
	int logic_handleVerbPull();
	int logic_handleVerbOpen();
	int logic_handleVerbClose();
	int logic_handleVerbGive();
	void logic_handleDialogReply(int index, int x, int y);
	int logic_handleDialogSelect(int &replyTextX, int &replyTextY, int &replyTextIndex1, int &replyTextIndex2, int &replyTextIndex3);
	void logic_refreshRoomBackground(int roomNum);

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
