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

#ifndef LASTEXPRESS_LASTEXPRESS_H
#define LASTEXPRESS_LASTEXPRESS_H

#include "lastexpress/debug.h"
#include "lastexpress/game/beetle.h"
#include "lastexpress/game/vcr.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/sound/slot.h"
#include "lastexpress/sound/sound.h"
#include "lastexpress/sound/subtitle.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/data/sprites.h"
#include "lastexpress/game/nis.h"
#include "lastexpress/game/otis.h"
#include "lastexpress/game/events.h"
#include "lastexpress/fight/fight.h"
#include "lastexpress/menu/menu.h"
#include "lastexpress/graphics.h"
#include "lastexpress/memory.h"

#include "common/random.h"
#include "common/file.h"

#include "engines/engine.h"

#include "graphics/pixelformat.h"

struct ADGameDescription;

/**
 * This is the namespace of the LastExpress engine.
 *
 * Status of this engine:
 *  The engine is completely implemented, the game is fully playable
 *  and the code is as faithful to the original disassembly as it reasonably can be.
 * 
 *  Notable features:
 *    - Original savegames are supported and swappable from ScummVM to the original
 *      (and viceversa).
 *    - The debugger uses DearImGui and provides a real-time visualization of all the
 *      characters movements.
 *
 * Original maintainers:
 *  littleboy, jvprat, clone2727
 *
 * Engine rewrite/additional reverse engineering:
 *  Bosca
 *
 * Supported games:
 *  - The Last Express
 */
namespace LastExpress {

class ArchiveManager;
class GoldArchiveManager;
class CBeetle;
class CFight;
class Clock;
class GraphicsManager;
class LogicManager;
class MemoryManager;
class Menu;
class MessageManager;
class NISManager;
class OtisManager;
class ResourceManager;
class SaveManager;
class Slot;
class SoundManager;
class SpriteManager;
class SubtitleManager;
class VCR;

struct Extent;

enum {
	GF_COMPRESSED = 1 << 0
};

typedef struct Item {
	uint8 mnum;
	uint16 closeUp;
	int8 useable;
	int8 haveIt;
	int8 inPocket;
	int8 floating;

	Item() {
		clear();
	}

	void clear() {
		mnum = 0;
		closeUp = 0;
		useable = 0;
		haveIt = 0;
		inPocket = 0;
		floating = 0;
	}
} Item;

typedef struct Door {
	int8 who;
	int8 status;
	uint8 windowCursor;
	uint8 handleCursor;
	int8 model;

	Door() {
		clear();
	}

	void clear() {
		who = 0;
		status = 0;
		windowCursor = 0;
		handleCursor = 0;
		model = 0;
	}
} Door;

typedef struct Position {
	uint16 position;
	uint16 location;
	uint16 car;

	Position() {
		clear();
	}

	void clear() {
		position = 0;
		location = 0;
		car = 0;
	}
} Position;

typedef struct Sprite {
	byte *compData;
	byte *eraseMask;
	uint16 *colorPalette;
	Extent rect;
	uint16 hotspotX1;
	uint16 hotspotX2;
	uint16 hotspotY1;
	uint16 hotspotY2;
	byte compBits;
	byte compType;
	byte copyScreenAndRedrawFlag;
	byte spritesUnk3;
	byte ticksToWaitUntilCycleRestart;
	byte soundDelay;
	byte soundAction;
	byte flags;
	byte position;
	byte spritesUnk9;
	byte spritesUnk10;
	byte spritesUnk11;
	int spritesUnk8;
	uint16 visibilityDist;
	uint16 hotspotPriority;
	Sprite *nextSprite;

	Sprite() {
		compData = nullptr;
		eraseMask = nullptr;
		colorPalette = nullptr;
		hotspotX1 = 0;
		hotspotX2 = 0;
		hotspotY1 = 0;
		hotspotY2 = 0;
		compBits = 0;
		compType = 0;
		copyScreenAndRedrawFlag = 0;
		spritesUnk3 = 0;
		ticksToWaitUntilCycleRestart = 0;
		soundDelay = 0;
		soundAction = 0;
		flags = 0;
		position = 0;
		spritesUnk9 = 0;
		spritesUnk10 = 0;
		spritesUnk11 = 0;
		spritesUnk8 = 0;
		visibilityDist = 0;
		hotspotPriority = 0;
		nextSprite = nullptr;
	}
} Sprite;

typedef struct Seq {
	int numFrames;
	Sprite *sprites;
	Sprite spriteArrayStart;
	byte gap[18];
	byte *rawSeqData;

	Seq() {
		numFrames = 0;
		sprites = nullptr;
		memset(gap, 0, sizeof(gap));
		rawSeqData = nullptr;
	}

	~Seq() {
		delete[] sprites;
		sprites = nullptr;
	}
} Seq;

typedef struct CharacterCallParams {
	int parameters[32];

	CharacterCallParams() {
		clear();
	}

	void clear() {
		for (int i = 0; i < 32; i++) {
			parameters[i] = 0;
		}
	}
} CharacterCallParams;

typedef struct Character {
	CharacterCallParams callParams[9];
	uint8 callbacks[16];
	uint8 currentCall;
	Position characterPosition;
	uint8 walkCounter;
	int8 attachedConductor;
	uint8 inventoryItem;
	int8 direction;
	int16 waitedTicksUntilCycleRestart;
	int16 currentFrameSeq1;
	int16 currentFrameSeq2;
	int16 elapsedFrames;
	int16 walkStepSize;
	int8 clothes;
	int8 position2;
	int8 car2;
	int8 doProcessEntity;
	int8 needsPosFudge;
	int8 needsSecondaryPosFudge;
	int8 directionSwitch;
	char sequenceName[13];
	char sequenceName2[13];
	char sequenceNamePrefix[7];
	char sequenceNameCopy[13];
	Sprite *frame1;
	Sprite *frame2;
	Seq *sequence1;
	Seq *sequence2;
	Seq *sequence3;

	Character() {
		reset();
	}

	void reset() {
		memset(callbacks, 0, sizeof(callbacks));
		currentCall = 0;
		walkCounter = 0;
		attachedConductor = 0;
		inventoryItem = 0;
		direction = 0;
		waitedTicksUntilCycleRestart = 0;
		currentFrameSeq1 = 0;
		currentFrameSeq2 = 0;
		elapsedFrames = 0;
		walkStepSize = 0;
		clothes = 0;
		position2 = 0;
		car2 = 0;
		doProcessEntity = 0;
		needsPosFudge = 0;
		needsSecondaryPosFudge = 0;
		directionSwitch = 0;

		memset(sequenceName, 0, sizeof(sequenceName));
		memset(sequenceName2, 0, sizeof(sequenceName2));
		memset(sequenceNamePrefix, 0, sizeof(sequenceNamePrefix));
		memset(sequenceNameCopy, 0, sizeof(sequenceNameCopy));

		frame1 = nullptr;
		frame2 = nullptr;
		sequence1 = nullptr;
		sequence2 = nullptr;
		sequence3 = nullptr;
	}

} Character;

typedef struct Characters {
	Character characters[40];
} Characters;

typedef struct Line7 {
	int slope;
	int intercept;
	char lineType;
	Line7 *next;

	Line7() {
		slope = 0;
		intercept = 0;
		lineType = 0;
		next = nullptr;
	}

	~Line7() {
		if (next) {
			delete next;
			next = nullptr;
		}
	}

} Line7;

typedef struct Link {
	uint16 left;
	uint16 right;
	uint16 top;
	uint16 bottom;
	Line7 *lineList;
	uint16 scene;
	uint8 location;
	uint8 action;
	uint8 param1;
	uint8 param2;
	uint8 param3;
	uint8 cursor;
	Link *next;

	Link() {
		left = 0;
		right = 0;
		top = 0;
		bottom = 0;
		lineList = nullptr;
		scene = 0;
		location = 0;
		action = 0;
		param1 = 0;
		param2 = 0;
		param3 = 0;
		cursor = 0;
		next = nullptr;
	}

	~Link() {
		if (lineList) {
			delete lineList;
			lineList = nullptr;
		}

		if (next) {
			delete next;
			next = nullptr;
		}
	}

	void copyFrom(Link *otherLink) {
		left = otherLink->left;
		right = otherLink->right;
		top = otherLink->top;
		bottom = otherLink->bottom;
		lineList = nullptr;
		scene = otherLink->scene;
		location = otherLink->location;
		action = otherLink->action;
		param1 = otherLink->param1;
		param2 = otherLink->param2;
		param3 = otherLink->param3;
		cursor = otherLink->cursor;
		next = nullptr;
	}
} Link;

typedef struct Node {
	char sceneFilename[9];
	Position nodePosition;
	uint8 cathDir;
	uint8 property;
	uint8 parameter1;
	uint8 parameter2;
	uint8 parameter3;
	Link *link;

	Node() {
		memset(sceneFilename, 0, sizeof(sceneFilename));
		cathDir = 0;
		property = 0;
		parameter1 = 0;
		parameter2 = 0;
		parameter3 = 0;
		link = nullptr;
	}

	~Node() {
		if (link) {
			delete link;
			link = nullptr;
		}
	}
} Node;

#include "common/pack-start.h"
typedef struct TGAHeader {
	uint8 idLength;
	uint8 colorMapType;
	uint8 imageType;
	uint16 colorMapFirstEntryIndex;
	uint16 colorMapLength;
	uint8 colorMapEntrySize;
	uint16 xOrigin;
	uint16 yOrigin;
	uint16 width;
	uint16 height;
	uint8 bitsPerPixel;
	uint8 imageDescriptor;
	byte *rawDataPtr; // Not in the original, but we need it to avoid pointer fix-ups...

	TGAHeader() {
		idLength = 0;
		colorMapType = 0;
		imageType = 0;
		colorMapFirstEntryIndex = 0;
		colorMapLength = 0;
		colorMapEntrySize = 0;
		xOrigin = 0;
		yOrigin = 0;
		width = 0;
		height = 0;
		bitsPerPixel = 0;
		imageDescriptor = 0;
		rawDataPtr = nullptr;
	}
} TGAHeader;
#include "common/pack-end.h"

typedef struct FontData {
	uint16 palette[16];
	uint8 charMap[256];
	uint8 charKerning[256];
	byte *fontData;

	FontData() {
		memset(palette, 0, sizeof(palette));
		memset(charMap, 0, sizeof(charMap));
		memset(charKerning, 0, sizeof(charKerning));
		fontData = nullptr;
	}

	~FontData() {
		memset(palette, 0, sizeof(palette));
		memset(charMap, 0, sizeof(charMap));
		memset(charKerning, 0, sizeof(charKerning));

		if (fontData) {
			free(fontData);
			fontData = nullptr;
		}
	}
} FontData;

typedef struct NisSprite {
	byte *compData;
	byte *eraseMask;
	uint16 *colorPalette;
	Extent rect;
	uint16 gammaPalette[128];
	byte compBits;

	NisSprite() {
		compData = nullptr;
		eraseMask = nullptr;
		colorPalette = nullptr;
		memset(gammaPalette, 0, sizeof(gammaPalette));
		compBits = 0;
	}
} NisSprite;

typedef struct NisEvents {
	int16 eventType;
	int16 eventTime;
	int32 eventSize;

	NisEvents() {
		eventType = 0;
		eventTime = 0;
		eventSize = 0;
	}
} NisEvents;

#define MEM_PAGE_SIZE  0x800

#define DEMO_TIMEOUT 2700

typedef uint16 PixMap;

class LastExpressEngine : public Engine {
	// AARGH! Remove!
	friend class VCR;
	friend class SubtitleManager;

protected:
	// Engine APIs
	Common::Error run() override;
	bool hasFeature(EngineFeature f) const override;

public:
	LastExpressEngine(OSystem *syst, const ADGameDescription *gd);
	~LastExpressEngine() override;

	// Misc
	Common::RandomSource& getRandom() {return _random; }

	// Managers
	ArchiveManager  *getArchiveManager()  const { return _archiveMan;  }
	GraphicsManager *getGraphicsManager() const { return _graphicsMan; }
	SubtitleManager *getSubtitleManager() const { return _subtitleMan; }

	MemoryManager  *getMemoryManager()  const { return _memMan;    }
	MessageManager *getMessageManager() const { return _msgMan;    }
	NISManager     *getNISManager()     const { return _nisMan;    }
	SoundManager   *getSoundManager()   const { return _soundMan;  }
	LogicManager   *getLogicManager()   const { return _logicMan;  }
	OtisManager    *getOtisManager()    const { return _otisMan;   }
	SpriteManager  *getSpriteManager()  const { return _spriteMan; }
	SaveManager    *getSaveManager()    const { return _saveMan;   }

	VCR   *getVCR()   const { return _vcr;   }
	Menu  *getMenu()  const { return _menu;  }
	Clock *getClock() const { return _clock; }

	// Event handling
	bool handleEvents();

	bool isDemo() const;
	bool isGoldEdition() const;
	bool isCompressed() const;

	Common::String getTargetName() const;

	// NEW STUFF
	static void soundTimerHandler(void *refCon);

	int32 _soundFrameCounter = 0;

	Common::Mutex *_soundMutex;
	const char *_savegameFilename = nullptr;
	const char *_savegameTempNames[7] = {
		"blue.tmp",
		"red.tmp",
		"green.tmp",
		"purple.tmp",
		"teal.tmp",
		"gold.tmp",
		"temp.tmp"
	};

	const char *_savegameNames[6] = {
		"blue.egg",
		"red.egg",
		"green.egg",
		"purple.egg",
		"teal.egg",
		"gold.egg"
	};

	CVCRFile *_savegame = nullptr;

	int32 _currentGameFileColorId = -1;
	int32 _gracePeriodTimer = 0;
	bool _eventTickInternal = false;
	
	bool _doShowCredits = false;
	int32 _currentSavePoint = 0;
	int32 _lastSavePointIdInFile = 0;
	int32 _gameTimeOfLastSavePointInFile = 0;
	int32 _gracePeriodIndex = 0;
	int32 _fightSkipCounter = 0;
	
	bool _navigationEngineIsRunning = false;
	bool _stopUpdatingCharacters = false;
	bool _charactersUpdateRecursionFlag = false;
	bool _mouseHasLeftClicked = false;
	bool _mouseHasRightClicked = false;
	bool _fastWalkJustDeactivated = false;

	int32 _cursorX	= 320;
	int32 _cursorY	= 240;
	int32 _cursorXOffscreenOffset = 0;
	int32 _cursorYOffscreenOffset = 0;
	int _cursorType = 0;

	bool _systemEventLeftMouseDown = false;
	bool _systemEventRightMouseDown = false;
	Common::Point _systemEventLastMouseCoords;

	bool _pendingExitEvent = false;
	bool _exitFromMenuButton = false;
	int32 _lastForcedScreenUpdateTicks = 0; // Not in the original

	Seq *_doorSeqs[8] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	Seq *_clockHandsSeqs[2] = { nullptr, nullptr };
	int32 _clockHandsValues[2] = { 0, 0 };

	int32 _fudgePosition[41] = {
		   0,  851, 1430, 2110,    0, 2410, 2980, 3450,
		3760, 4100, 4680, 5140, 5440, 5810, 6410, 6850,
		7160, 7510, 8514,    0,    0,    0, 2086, 2690,
		   0, 3110, 3390, 3890, 4460, 4770, 5090, 5610,
		6160, 6460, 6800, 7320, 7870, 8160, 8500, 9020,
		9269
	};

	// DYNAMIC ALLOC
	byte *_globalMemoryPool = nullptr;
	byte *_soundMemoryPool = nullptr;
	byte *_cursorsMemoryPool = nullptr;
	Characters *_characters = nullptr;

	SVCRSavePointHeader *_savePointHeaders = nullptr;

	// GENERAL
	void startUp();
	void shutDown();

	void waitForTimer(int millis);
	void initGameData();
	void startNewGame();
	void engineEventHandler(Event *event);

	int32 getSoundFrameCounter();

	bool getEventTickInternal();
	void setEventTickInternal(bool flag);

	void mouseSetLeftClicked(bool hasClicked);
	void mouseSetRightClicked(bool hasClicked);
	bool mouseHasLeftClicked();
	bool mouseHasRightClicked();

	// BEETLE
	CBeetle *_beetle = nullptr;
	void doBeetle();
	void endBeetle();
	void positionSprite(Sprite *sprite, Common::Point coord);

	// CREDITS
	void doCredits();
	void abortCredits();
	void creditsMouse(Event *event);
	void creditsTimer(Event *event);
	int32 readTGAIntoMemory(const char *filename, TGAHeader *tgaHeader);
	void constructPalette(TGAHeader *tgaHeader, uint16 *palette);

	// DEMO CREDITS
	bool demoEnding(bool wonGame);
	void demoEndingMouse(Event *event);
	void demoEndingTimer(Event *event);

	// EVENT HANDLERS
	void engineEventHandlerWrapper(Event *event);

	void nodeStepMouseWrapper(Event *event);
	void nodeStepTimerWrapper(Event *event);

	void eggMouseWrapper(Event *event);
	void eggTimerWrapper(Event *event);

	void nisMouseWrapper(Event *event);
	void nisTimerWrapper(Event *event);

	void creditsMouseWrapper(Event *event);
	void creditsTimerWrapper(Event *event);

	void demoEndingMouseWrapper(Event *event);
	void demoEndingTimerWrapper(Event *event);

	void fightMouseWrapper(Event *event);
	void fightTimerWrapper(Event *event);

	void emptyHandler(Event *event);

	int _savedFrameCounter = 0;
	int32 _savedFrameInterval = 0;
	int _doCredits = 0;

	void (LastExpressEngine::*_savedTimerEventHandle)(Event *) = nullptr;
	void (LastExpressEngine::*_savedMouseEventHandle)(Event *) = nullptr;

	// FIGHT
	int doFight(int fightId);
	void abortFight();

	CFight *_fight = nullptr;

	// DEBUGGER
	void showEngineInfo();

	bool _lockGracePeriod = false;

private:
	const ADGameDescription *_gameDescription;
	Graphics::PixelFormat _pixelFormat;

	// Misc
	Debugger *_debugger;
	Common::RandomSource _random;

	// Game
	Menu *_menu = nullptr;
	Clock *_clock = nullptr;
	VCR *_vcr = nullptr;

	// Managers
	ArchiveManager  *_archiveMan = nullptr;
	GraphicsManager *_graphicsMan = nullptr;
	SoundManager    *_soundMan = nullptr;
	LogicManager    *_logicMan = nullptr;
	OtisManager     *_otisMan = nullptr;
	SpriteManager   *_spriteMan = nullptr;
	MemoryManager   *_memMan = nullptr;
	MessageManager  *_msgMan = nullptr;
	NISManager      *_nisMan = nullptr;
	SaveManager     *_saveMan = nullptr;
	SubtitleManager *_subtitleMan = nullptr;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_LASTEXPRESS_H
