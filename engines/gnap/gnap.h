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

#ifndef GNAP_GNAP_H
#define GNAP_GNAP_H

#include "common/array.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/random.h"
#include "common/savefile.h"
#include "common/serializer.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/system.h"
#include "engines/engine.h"
#include "graphics/pixelformat.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

#include "gnap/debugger.h"
#include "gnap/resource.h"
#include "gnap/scenes/scenecore.h"
#include "gnap/character.h"
#include "gnap/music.h"

struct ADGameDescription;

namespace Common {
class PEResources;
}

namespace Gnap {

class DatManager;
class SequenceResource;
class SpriteResource;
class GameSys;
class SoundMan;
class MusicPlayer;

#define GNAP_SAVEGAME_VERSION 2

struct MouseButtonState {
	bool _left;
	bool _right;
	MouseButtonState() : _left(false), _right(false) {
	}
};

struct Hotspot {
	Common::Rect _rect;
	uint16 _flags;

	bool isPointInside(Common::Point pos) const {
		return _rect.contains(pos);
	}

	bool isFlag(uint16 flag) const {
		return (_flags & flag) != 0;
	}

	void clearRect() {
		_rect = Common::Rect(0, 0, 0, 0);
	}
};

const int kMaxTimers = 10;

enum GnapDebugChannels {
	kDebugBasic	= 1 << 0,
	kDebugMusic = 1 << 1
};

enum {
	SF_NONE				= 0x0000,
	SF_LOOK_CURSOR		= 0x0001,
	SF_GRAB_CURSOR		= 0x0002,
	SF_TALK_CURSOR		= 0x0004,
	SF_PLAT_CURSOR		= 0x0008,
	SF_DISABLED			= 0x0010,
	SF_WALKABLE			= 0x0020,
	SF_EXIT_L_CURSOR	= 0x0040,
	SF_EXIT_R_CURSOR	= 0x0080,
	SF_EXIT_U_CURSOR	= 0x0100,
	SF_EXIT_D_CURSOR	= 0x0200,
	SF_EXIT_NW_CURSOR	= 0x0400,
	SF_EXIT_NE_CURSOR	= 0x0800,
	SF_EXIT_SW_CURSOR	= 0x1000,
	SF_EXIT_SE_CURSOR	= 0x2000
};

enum {
	LOOK_CURSOR		= 0,
	GRAB_CURSOR		= 1,
	TALK_CURSOR		= 2,
	PLAT_CURSOR		= 3,
	NOLOOK_CURSOR	= 4,
	NOGRAB_CURSOR	= 5,
	NOTALK_CURSOR	= 6,
	NOPLAT_CURSOR	= 7,
	EXIT_L_CURSOR	= 8,
	EXIT_R_CURSOR	= 9,
	EXIT_U_CURSOR	= 10,
	EXIT_D_CURSOR	= 11,
	EXIT_NE_CURSOR	= 12,
	EXIT_NW_CURSOR	= 13,
	EXIT_SE_CURSOR	= 14,
	EXIT_SW_CURSOR	= 15,
	WAIT_CURSOR		= 16
};

enum {
	kGSPullOutDevice			= 0,
	kGSPullOutDeviceNonWorking	= 1,
	kGSIdle						= 2,
	kGSBrainPulsating			= 3,
	kGSImpossible				= 4,
	kGSScratchingHead			= 5,
	kGSDeflect					= 6,
	kGSUseDevice				= 7,
	kGSMoan1					= 8,
	kGSMoan2					= 9
};

enum {
	kItemMagazine			= 0,
	kItemMud				= 1,
	kItemGrass				= 2,
	kItemDisguise			= 3,
	kItemNeedle				= 4,
	kItemTwig				= 5,
	kItemGas				= 6,
	kItemKeys				= 7,
	kItemDice				= 8,
	kItemTongs				= 9,
	kItemQuarter			= 10,
	kItemQuarterWithHole	= 11,
	kItemDiceQuarterHole	= 12,
	kItemWrench				= 13,
	kItemCowboyHat			= 14,
	kItemGroceryStoreHat	= 15,
	kItemBanana				= 16,
	kItemTickets			= 17,
	kItemPicture			= 18,
	kItemEmptyBucket		= 19,
	kItemBucketWithBeer		= 20,
	kItemBucketWithPill		= 21,
	kItemPill				= 22,
	kItemHorn				= 23,
	kItemJoint				= 24,
	kItemChickenBucket		= 25,
	kItemGum				= 26,
	kItemSpring				= 27,
	kItemLightbulb			= 28,
	kItemCereals			= 29
};

enum {
	kGFPlatypus				= 0,
	kGFMudTaken				= 1,
	kGFNeedleTaken			= 2,
	kGFTwigTaken			= 3,
	kGFUnk04				= 4,
	kGFKeysTaken			= 5,
	kGFGrassTaken			= 6,
	kGFBarnPadlockOpen		= 7,
	kGFTruckFilledWithGas	= 8,
	kGFTruckKeysUsed		= 9,
	kGFPlatypusDisguised	= 10,
	kGFSceneFlag1			= 11,
	kGFGnapControlsToyUFO	= 12,
	kGFUnk13				= 13, // Tongue Fight Won?
	kGFUnk14				= 14,
	kGFSpringTaken			= 15,
	kGFUnk16				= 16,
	kGFJointTaken			= 17,
	kGFUnk18				= 18,
	kGFGroceryStoreHatTaken	= 19,
	kGFPictureTaken			= 20,
	kGFUnk21				= 21,
	kGFUnk22				= 22,
	kGFUnk23				= 23,
	kGFUnk24				= 24,
	kGFUnk25				= 25,
	kGFPlatypusTalkingToAssistant = 26,
	kGFUnk27				= 27,
	kGFUnk28				= 28,
	kGFGasTaken				= 29,
	kGFUnk30				= 30,
	kGFUnk31				= 31
};

struct GnapSavegameHeader {
	uint8 _version;
	Common::String _saveName;
	Graphics::Surface *_thumbnail;
	int _year, _month, _day;
	int _hour, _minute;
};

class GnapEngine : public Engine {
protected:
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;
public:
	GnapEngine(OSystem *syst, const ADGameDescription *gd);
	~GnapEngine() override;
private:
	const ADGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;
	int _loadGameSlot;

public:
	Common::RandomSource *_random;
	Common::PEResources *_exe;

	DatManager *_dat;
	SpriteCache *_spriteCache;
	SoundCache *_soundCache;
	SequenceCache *_sequenceCache;
	GameSys *_gameSys;
	SoundMan *_soundMan;
	Debugger *_debugger;
	Scene *_scene;
	PlayerGnap *_gnap;
	PlayerPlat *_plat;
	MusicPlayer *_music;
	Graphics::Font *_font;

	Common::MemoryWriteStreamDynamic *_tempThumbnail;

	int _lastUpdateClock;
	bool _gameDone;

	bool _keyPressState[512];
	bool _keyDownState[512];

	bool _isPaused;
	Graphics::Surface *_pauseSprite;
	int _timers[kMaxTimers], _savedTimers[kMaxTimers];

	MouseButtonState _mouseButtonState;
	MouseButtonState _mouseClickState;

	bool _sceneSavegameLoaded, _wasSavegameLoaded;

	Graphics::Surface *_backgroundSurface;
	int _prevSceneNum, _currentSceneNum, _newSceneNum;
	bool _sceneDone, _sceneWaiting;

	uint32 _inventory, _gameFlags;

	Hotspot _hotspots[20];
	Common::Point _hotspotsWalkPos[20];
	int _hotspotsCount;
	int _sceneClickedHotspot;

	bool _isWaiting;
	bool _isLeavingScene;

	bool _isStockDatLoaded;

	int _newCursorValue, _cursorValue;

	int _verbCursor, _cursorIndex;
	Common::Point _mousePos;
	int _leftClickMouseX, _leftClickMouseY;

	Graphics::Surface *_grabCursorSprite;
	int _currGrabCursorX, _currGrabCursorY;
	int _grabCursorSpriteIndex, _newGrabCursorSpriteIndex;

	Graphics::Surface *_fullScreenSprite;
	int _fullScreenSpriteId;

	int _deviceX1, _deviceY1;

	int _soundTimerIndexA;
	int _soundTimerIndexB;
	int _soundTimerIndexC;
	int _idleTimerIndex;

	void updateEvents();
	void gameUpdateTick();
	void saveTimers();
	void restoreTimers();

	void pauseGame();
	void resumeGame();
	void updatePause();

	int getRandom(int max);

	int readSavegameDescription(int savegameNum, Common::String &description);
	int loadSavegame(int savegameNum);
	Common::Error saveGameState(int slot, const Common::String &desc, bool isAutosave = false) override;
	Common::Error loadGameState(int slot) override;
	void synchronize(Common::Serializer &s);
	void writeSavegameHeader(Common::OutSaveFile *out, GnapSavegameHeader &header);
	WARN_UNUSED_RESULT static bool readSavegameHeader(Common::InSaveFile *in, GnapSavegameHeader &header, bool skipThumbnail = true);

	void delayTicks(int val, int idx, bool updateCursor);
	void delayTicksA(int val, int idx);
	void delayTicksCursor(int val);

	void setHotspot(int index, int16 x1, int16 y1, int16 x2, int16 y2, uint16 flags = SF_NONE,
		int16 walkX = -1, int16 walkY = -1);
	int getHotspotIndexAtPos(Common::Point pos);
	void updateCursorByHotspot();
	int getClickedHotspotId();

	int getInventoryItemSpriteNum(int index);

	void updateMouseCursor();
	void setVerbCursor(int verbCursor);
	void setCursor(int cursorIndex);
	void showCursor();
	void hideCursor();

	void setGrabCursorSprite(int index);
	void createGrabCursorSprite(int spriteId);
	void freeGrabCursorSprite();
	void updateGrabCursorSprite(int x, int y);

	void invClear();
	void invAdd(int itemId);
	void invRemove(int itemId);
	bool invHas(int itemId);

	void clearFlags();
	void setFlag(int num);
	void clearFlag(int num);
	bool isFlag(int num);

	Graphics::Surface *addFullScreenSprite(int resourceId, int id);
	void removeFullScreenSprite();
	void showFullScreenSprite(int resourceId);

	void queueInsertDeviceIcon();
	void insertDeviceIconActive();
	void removeDeviceIconActive();
	void setDeviceHotspot(int hotspotIndex, int x1, int y1, int x2, int y2);

	int getSequenceTotalDuration(int resourceId);

	bool isSoundPlaying(int resourceId);
	void playSound(int resourceId, bool looping);
	void stopSound(int resourceId);
	void setSoundVolume(int resourceId, int volume);

	void updateTimers();

	void initGameFlags(int num);
	void loadStockDat();

	void mainLoop();
	void initScene();
	void endSceneInit();
	void afterScene();

	int initSceneLogic();
	void runSceneLogic();

	void checkGameKeys();

	void startSoundTimerA(int timerIndex);
	int playSoundA();
	void startSoundTimerB(int timerIndex);
	int playSoundB();
	void startSoundTimerC(int timerIndex);
	int playSoundC();
	void startIdleTimer(int timerIndex);
	void updateIdleTimer();

	void screenEffect(int dir, byte r, byte g, byte b);

	bool isKeyStatus1(int key);
	bool isKeyStatus2(int key);
	void clearKeyStatus1(int key);
	void clearAllKeyStatus1();

	void deleteSurface(Graphics::Surface **surface);

	// Menu
	int _menuStatus;
	int _menuSpritesIndex;
	bool _menuDone;
	Graphics::Surface *_menuBackgroundSurface;
	Graphics::Surface *_menuQuitQuerySprite;
	Graphics::Surface *_largeSprite;
	Graphics::Surface *_menuSaveLoadSprite;
	Graphics::Surface *_menuSprite2;
	Graphics::Surface *_menuSprite1;
	char _savegameFilenames[7][30];
	Graphics::Surface *_savegameSprites[7];
	Graphics::Surface *_spriteHandle;
	Graphics::Surface *_cursorSprite;
	int _menuInventoryIndices[30];
	Graphics::Surface *_menuInventorySprites[30];
	int _savegameIndex;
	void createMenuSprite();
	void freeMenuSprite();
	void initMenuHotspots1();
	void initMenuHotspots2();
	void initMenuQuitQueryHotspots();
	void initSaveLoadHotspots();
	void drawInventoryFrames();
	void insertInventorySprites();
	void removeInventorySprites();
	void runMenu();
	void updateMenuStatusInventory();
	void updateMenuStatusMainMenu();
	void updateMenuStatusSaveGame();
	void updateMenuStatusLoadGame();
	void updateMenuStatusQueryQuit();

	// Grid common
	int _gridMinX, _gridMinY;
	int _gridMaxX, _gridMaxY;
	bool isPointBlocked(int gridX, int gridY);
	bool isPointBlocked(Common::Point gridPos);
	void initSceneGrid(int gridMinX, int gridMinY, int gridMaxX, int gridMaxY);
	bool testWalk(int animationIndex, int someStatus, int gridX1, int gridY1, int gridX2, int gridY2);

	// Gnap
	void doCallback(int callback);

	// Scenes
	int _toyUfoNextSequenceId, _toyUfoSequenceId;
	int _toyUfoId;
	int _toyUfoActionStatus;
	int _toyUfoX;
	int _toyUfoY;

	void initGlobalSceneVars();
	void playSequences(int fullScreenSpriteId, int sequenceId1, int sequenceId2, int sequenceId3);

	// Shared by scenes 17 & 18
	int _s18GarbageCanPos;

	// Scene 4x
	void toyUfoSetStatus(int flagNum);
	int toyUfoGetSequenceId();
	bool toyUfoCheckTimer();
	void toyUfoFlyTo(int destX, int destY, int minX, int maxX, int minY, int maxY, int animationIndex);

	void playMidi(const char *name);
	void stopMidi();
};

} // End of namespace Gnap

#endif // GNAP_GNAP_H
