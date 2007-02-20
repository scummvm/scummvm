/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef AGI_H
#define AGI_H

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/endian.h"
#include "common/util.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/hash-str.h"

#include "engines/engine.h"

namespace Agi {

typedef signed int Err;

/*
 * Version and other definitions
 */

#define USE_IIGS_SOUND

#define	TITLE		"AGI engine"

#define DIR_		"dir"
#define LOGDIR		"logdir"
#define PICDIR		"picdir"
#define VIEWDIR		"viewdir"
#define	SNDDIR		"snddir"
#define OBJECTS		"object"
#define WORDS		"words.tok"

#define	MAX_DIRS	256
#define	MAX_VARS	256
#define	MAX_FLAGS	(256 >> 3)
#define MAX_VIEWTABLE	255	/* KQ3 uses o255! */
#define MAX_WORDS	20
#define	MAX_STRINGS	24		/* MAX_STRINGS + 1 used for get.num */
#define MAX_STRINGLEN	40
#ifndef MAX_PATH
#define MAX_PATH	260
#endif

#define	_EMPTY		0xfffff
#define	EGO_OWNED	0xff

#define	CRYPT_KEY_SIERRA	"Avis Durgan"
#define CRYPT_KEY_AGDS		"Alex Simkin"

#ifndef INLINE
#define INLINE
#endif

#define	MSG_BOX_COLOUR	0x0f	/* White */
#define MSG_BOX_TEXT	0x00	/* Black */
#define MSG_BOX_LINE	0x04	/* Red */
#define STATUS_FG	0x00		/* Black */
#define	STATUS_BG	0x0f		/* White */
#define PATCH_LOGIC				/* disable copy protection on some games */

#define ADD_PIC 1
#define ADD_VIEW 2

} // End of namespace Agi

/* AGI resources */
#include "agi/console.h"
#include "agi/view.h"
#include "agi/picture.h"
#include "agi/logic.h"
#include "agi/sound.h"

namespace Agi {

enum AgiGameType {
	GType_V2 = 1,
	GType_V3
};

enum AgiGameFeatures {
	GF_AGIMOUSE =    (1 << 0),
	GF_AGDS =        (1 << 1),
	GF_AGI256 =      (1 << 2),
	GF_AGI256_2 =    (1 << 3),
	GF_AGIPAL =      (1 << 4),
	GF_MACGOLDRUSH = (1 << 5),
	GF_FANMADE =     (1 << 6)
};

struct AGIGameDescription;

enum {
	NO_GAMEDIR = 0,
	GAMEDIR
};

enum AGIErrors {
	errOK = 0,
	errDoNothing,
	errBadCLISwitch,
	errInvalidAGIFile,
	errBadFileOpen,
	errNotEnoughMemory,
	errBadResource,
	errUnknownAGIVersion,
	errRestartGame,
	errNoLoopsInView,
	errViewDataError,
	errNoGameList,

	errUnk = 127
};

enum kDebugLevels {
	kDebugLevelMain =      1 << 0,
	kDebugLevelResources = 1 << 1,
	kDebugLevelSprites =   1 << 2,
	kDebugLevelInventory = 1 << 3,
	kDebugLevelInput =     1 << 4,
	kDebugLevelMenu =      1 << 5,
	kDebugLevelScripts =   1 << 6,
	kDebugLevelSound =     1 << 7,
	kDebugLevelText =      1 << 8,
	kDebugLevelSavegame =  1 << 9
};

/**
 * AGI resources.
 */
enum {
	rLOGIC = 1,
	rSOUND,
	rVIEW,
	rPICTURE
};

enum {
	RES_LOADED = 1,
	RES_COMPRESSED = 0x40
};

enum {
	lCOMMAND_MODE = 1,
	lTEST_MODE
};

struct gameIdList {
	gameIdList *next;
	uint32 version;
	uint32 crc;
	char *gName;
	char *switches;
};

struct Mouse {
	int button;
	unsigned int x;
	unsigned int y;
};

#define report printf

enum GameId {
	GID_AGI = 1
};

#define WIN_TO_PIC_X(x) ((x) / 2)
#define WIN_TO_PIC_Y(y) ((y) < 8 ? 999 : (y) >= (8 + _HEIGHT) ? 999 : (y) - 8)

/**
 * AGI variables.
 */
enum {
	vCurRoom = 0,		/* 0 */
	vPrevRoom,
	vBorderTouchEgo,
	vScore,
	vBorderCode,
	vBorderTouchObj,	/* 5 */
	vEgoDir,
	vMaxScore,
	vFreePages,
	vWordNotFound,
	vTimeDelay,		/* 10 */
	vSeconds,
	vMinutes,
	vHours,
	vDays,
	vJoystickSensitivity,	/* 15 */
	vEgoViewResource,
	vAgiErrCode,
	vAgiErrCodeInfo,
	vKey,
	vComputer,		/* 20 */
	vWindowReset,
	vSoundgen,
	vVolume,
	vMaxInputChars,
	vSelItem,		/* 25 */
	vMonitor
};

/**
 * AGI flags
 */
enum {
	fEgoWater = 0,	/* 0 */
	fEgoInvisible,
	fEnteredCli,
	fEgoTouchedP2,
	fSaidAcceptedInput,
	fNewRoomExec,	/* 5 */
	fRestartGame,
	fScriptBlocked,
	fJoySensitivity,
	fSoundOn,
	fDebuggerOn,		/* 10 */
	fLogicZeroFirsttime,
	fRestoreJustRan,
	fStatusSelectsItems,
	fMenusWork,
	fOutputMode,		/* 15 */
	fAutoRestart
};

struct AgiEvent {
	uint16 data;
	uint8 occured;
};

struct AgiObject {
	int location;
	char *name;
};

struct AgiWord {
	int id;
	char *word;
};

struct AgiDir {
	uint8 volume;
	uint32 offset;
	uint32 len;
	uint32 clen;
	uint8 flags;
	/* 0 = not in mem, can be freed
	 * 1 = in mem, can be released
	 * 2 = not in mem, cant be released
	 * 3 = in mem, cant be released
	 * 0x40 = was compressed
	 */
};

struct AgiBlock {
	int active;
	int x1, y1;
	int x2, y2;
	uint8 *buffer;		/* used for window background */
};

#define EGO_VIEW_TABLE	0
#define	HORIZON		36
#define _WIDTH		160
#define _HEIGHT		168

/**
 * AGI game structure.
 * This structure contains all global data of an AGI game executed
 * by the interpreter.
 */
struct AgiGame {
#define STATE_INIT	0x00
#define STATE_LOADED	0x01
#define STATE_RUNNING	0x02
	int state;		/**< state of the interpreter */

	char name[8];	/**< lead in id (e.g. `GR' for goldrush) */
	char id[8];		/**< game id */
	uint32 crc;		/**< game CRC */

	/* game flags and variables */
	uint8 flags[MAX_FLAGS]; /**< 256 1-bit flags */
	uint8 vars[MAX_VARS];   /**< 256 variables */

	/* internal variables */
	int horizon;			/**< horizon y coordinate */
	int lineStatus;		/**< line number to put status on */
	int lineUserInput;	/**< line to put user input on */
	int lineMinPrint;		/**< num lines to print on */
	int cursorPos;			/**< column where the input cursor is */
	uint8 inputBuffer[40]; /**< buffer for user input */
	uint8 echoBuffer[40]; 	/**< buffer for echo.line */
	int keypress;
#define INPUT_NORMAL	0x01
#define INPUT_GETSTRING	0x02
#define INPUT_MENU	0x03
#define INPUT_NONE	0x04
	int inputMode;			/**< keyboard input mode */
	int inputEnabled;		/**< keyboard input enabled */
	int lognum;				/**< current logic number */

	/* internal flags */
	int playerControl;		/**< player is in control */
	int quitProgNow;		/**< quit now */
	int statusLine;		/**< status line on/off */
	int clockEnabled;		/**< clock is on/off */
	int exitAllLogics;	/**< break cycle after new.room */
	int pictureShown;		/**< show.pic has been issued */
	int hasPrompt;			/**< input prompt has been printed */
#define ID_AGDS		0x00000001
#define ID_AMIGA	0x00000002
	int gameFlags;			/**< agi options flags */

	uint8 priTable[_HEIGHT];/**< priority table */

	/* windows */
	uint32 msgBoxTicks;	/**< timed message box tick counter */
	AgiBlock block;
	AgiBlock window;
	int hasWindow;

	/* graphics & text */
	int gfxMode;
	char cursorChar;
	unsigned int colorFg;
	unsigned int colorBg;
	uint8 *sbuf;			/**< 160x168 AGI screen buffer */

	/* player command line */
	AgiWord egoWords[MAX_WORDS];
	int numEgoWords;

	unsigned int numObjects;

	AgiEvent evKeyp[MAX_DIRS];  /**< keyboard keypress events */
	char strings[MAX_STRINGS + 1][MAX_STRINGLEN]; /**< strings */

	/* directory entries for resources */
	AgiDir dirLogic[MAX_DIRS];
	AgiDir dirPic[MAX_DIRS];
	AgiDir dirView[MAX_DIRS];
	AgiDir dirSound[MAX_DIRS];

	/* resources */
	AgiPicture pictures[MAX_DIRS]; 	/**< AGI picture resources */
	AgiLogic logics[MAX_DIRS];		/**< AGI logic resources */
	AgiView views[MAX_DIRS];		/**< AGI view resources */
	AgiSound sounds[MAX_DIRS];		/**< AGI sound resources */

	/* view table */
	VtEntry viewTable[MAX_VIEWTABLE];

	int32 ver;								/**< detected game version */

	int simpleSave;						/**< select simple savegames */
};

class AgiLoader {
private:
	int intVersion;
	AgiEngine *_vm;

public:

	AgiLoader() {}
	virtual ~AgiLoader() {}

	virtual int init() = 0;
	virtual int deinit() = 0;
	virtual int detectGame() = 0;
	virtual int loadResource(int, int) = 0;
	virtual int unloadResource(int, int) = 0;
	virtual int loadObjects(const char *) = 0;
	virtual int loadWords(const char *) = 0;
	virtual int version() = 0;
	virtual void setIntVersion(int) = 0;
	virtual int getIntVersion() = 0;
};

class AgiLoader_v2 : public AgiLoader {
private:
	int _intVersion;
	AgiEngine *_vm;

	int loadDir(AgiDir *agid, const char *fname);
	uint8 *loadVolRes(AgiDir *agid);

public:

	AgiLoader_v2(AgiEngine *vm) {
		_vm = vm;
		_intVersion = 0;
	}

	virtual int init();
	virtual int deinit();
	virtual int detectGame();
	virtual int loadResource(int, int);
	virtual int unloadResource(int, int);
	virtual int loadObjects(const char *);
	virtual int loadWords(const char *);
	virtual int version();
	virtual void setIntVersion(int);
	virtual int getIntVersion();
};

class AgiLoader_v3 : public AgiLoader {
private:
	int _intVersion;
	AgiEngine *_vm;

	int loadDir(AgiDir *agid, Common::File *fp, uint32 offs, uint32 len);
	uint8 *loadVolRes(AgiDir *agid);

public:

	AgiLoader_v3(AgiEngine *vm) {
		_vm = vm;
		_intVersion = 0;
	}

	virtual int init();
	virtual int deinit();
	virtual int detectGame();
	virtual int loadResource(int, int);
	virtual int unloadResource(int, int);
	virtual int loadObjects(const char *);
	virtual int loadWords(const char *);
	virtual int version();
	virtual void setIntVersion(int);
	virtual int getIntVersion();
};

class GfxMgr;
class SpritesMgr;
class Menu;
class SearchTree;

extern struct Mouse g_mouse;

/* Image stack support */
struct ImageStackElement {
	uint8 type;
	uint8 pad;
	int16 parm1;
	int16 parm2;
	int16 parm3;
	int16 parm4;
	int16 parm5;
	int16 parm6;
	int16 parm7;
};

struct StringData {
	int x;
	int y;
	int len;
	int str;
};

#define TICK_SECONDS 20

#define KEY_QUEUE_SIZE 16

class AgiEngine : public ::Engine {
	int _gameId;

protected:
	int init();
	int go();
	void shutdown();
	void initialize();

	bool initGame();

public:
	AgiEngine(OSystem *syst);
	virtual ~AgiEngine();
	int getGameId() {
		return _gameId;
	}

	const AGIGameDescription *_gameDescription;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	Common::Platform getPlatform() const;

private:

	int _keyQueue[KEY_QUEUE_SIZE];
	int _keyQueueStart;
	int _keyQueueEnd;

	int checkPriority(VtEntry *v);
	int checkCollision(VtEntry *v);
	int checkPosition(VtEntry *v);

	uint32 matchVersion(uint32 crc);

public:
	AgiGame _game;
	AgiObject *_objects;	/* objects in the game */

	StringData _stringdata;

	AgiLoader *_loader;	/* loader */

	Common::RandomSource *_rnd;
	const char *_savePath;

	const char *getSavegameFilename(int num);
	int selectSlot();
	int saveGame(const char *fileName, const char *saveName);
	int saveGameDialog();
	int saveGameSimple();
	int loadGame(const char *fileName);
	int loadGameDialog();
	int loadGameSimple();

	volatile uint32 _clockCount;

	uint8 *_intobj;
	int _oldMode;

	Menu* _menu;

	char _lastSentence[40];

	SpritesMgr *_sprites;
	GfxMgr *_gfx;
	SoundMgr *_sound;
	PictureMgr *_picture;

	#define INITIAL_IMAGE_STACK_SIZE 32

	int _stackSize;
	ImageStackElement *_imageStack;
	int _imageStackPointer;

	void clearImageStack();
	void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
		int16 p4, int16 p5, int16 p6, int16 p7);
	void releaseImageStack();

	AgiDebug _debug;
	Common::RenderMode _renderMode;
	int _soundemu;

	int _keyControl;
	int _keyAlt;

	Console *_console;

	int agiInit();
	int agiDeinit();
	int agiVersion();
	int agiGetRelease();
	void agiSetRelease(int);
	int agiDetectGame();
	int agiLoadResource(int, int);
	int agiUnloadResource(int, int);
	void agiUnloadResources();

	void agiTimerLow();
	int agiGetKeypressLow();
	int agiIsKeypressLow();
	static void agiTimerFunctionLow(void *refCon);
	void initPriTable();

	void newInputMode(int);
	void oldInputMode();

	int getflag(int);
	void setflag(int, int);
	void flipflag(int);
	int getvar(int);
	void setvar(int, int);
	void decrypt(uint8 * mem, int len);
	void releaseSprites();
	int mainCycle();
	int viewPictures();
	int parseCli(int, char **);
	int runGame();
	void inventory();
	void listGames();
	uint32 matchCrc(uint32, char *, int);
	int v2IdGame();
	int v3IdGame();
	int v4IdGame(uint32 ver);
	void updateTimer();
	int getAppDir(char *appDir, unsigned int size);

	int setupV2Game(int ver, uint32 crc);
	int setupV3Game(int ver, uint32 crc);

	void newRoom(int n);
	void resetControllers();
	void interpretCycle();
	int playGame();

	void printItem(int n, int fg, int bg);
	int findItem();
	int showItems();
	void selectItems(int n);

	void processEvents();

	// Objects
	int showObjects();
	int decodeObjects(uint8 *mem, uint32 flen);
	int loadObjects(const char *fname);
	int allocObjects(int);
	void unloadObjects();
	const char *objectName(unsigned int);
	int objectGetLocation(unsigned int);
	void objectSetLocation(unsigned int, int);

	// Logic
	int decodeLogic(int);
	void unloadLogic(int);
	int runLogic(int);
	void patchLogic(int n);

	void debugConsole(int, int, const char *);
	int testIfCode(int);
	void executeAgiCommand(uint8, uint8 *);

	// View
private:

	void lSetCel(VtEntry *v, int n);
	void lSetLoop(VtEntry *v, int n);
	void updateView(VtEntry *v);

public:

	void setCel(VtEntry *, int);
	void setLoop(VtEntry *, int);
	void setView(VtEntry *, int);
	void startUpdate(VtEntry *);
	void stopUpdate(VtEntry *);
	void updateViewtable();
	void unloadView(int);
	int decodeView(int);
	void addToPic(int, int, int, int, int, int, int);
	void drawObj(int);
	bool isEgoView(const VtEntry *v);

	// Words
	int showWords();
	int loadWords(const char *);
	void unloadWords();
	int findWord(char *word, int *flen);
	void dictionaryWords(char *);

	// Motion
private:
	int checkStep(int delta, int step);
	int checkBlock(int x, int y);
	void changePos(VtEntry *v);
	void motionWander(VtEntry *v);
	void motionFollowEgo(VtEntry *v);
	void motionMoveObj(VtEntry *v);
	void checkMotion(VtEntry *v);
public:
	void checkAllMotions();
	void moveObj(VtEntry *);
	void inDestination(VtEntry *);
	void fixPosition(int);
	void updatePosition();
	int getDirection(int x0, int y0, int x, int y, int s);

	// Keyboard
	void initWords();
	void cleanInput();
	int doPollKeyboard();
	void cleanKeyboard();
	void handleKeys(int);
	void handleGetstring(int);
	int handleController(int);
	void getString(int, int, int, int);
	uint16 agiGetKeypress();
	int waitKey();
	int waitAnyKey();

	// Text
public:
	#define MAXWORDLEN 24

	typedef Common::String String;

	int messageBox(const char *);
	int selectionBox(const char *, const char **);
	void closeWindow(void);
	void drawWindow(int, int, int, int);
	void printText(const char *, int, int, int, int, int, int, bool checkerboard = false);
	void printTextConsole(const char *, int, int, int, int, int);
	int print(const char *, int, int, int);
	char *wordWrapString(char *, int *);
	char *agiSprintf(const char *);
	void writeStatus(void);
	void writePrompt(void);
	void clearLines(int, int, int);
	void flushLines(int, int);
	bool predictiveDialog(void);

private:
	void printStatus(const char *message, ...);
	void printText2(int l, const char *msg, int foff, int xoff, int yoff, int len, int fg, int bg, bool checkerboard = false);
	void blitTextbox(const char *p, int y, int x, int len);
	void eraseTextbox();
	char *safeStrcat(char *s, const char *t);
	void loadDict(void);
	bool matchWord(void);

	SearchTree *_searchTreeRoot;
	SearchTree *_activeTreeNode;
	
	void insertSearchNode(const char *word);

	String _currentCode;
	String _currentWord;
	int _wordNumber;
public:
	char _predictiveResult[40];
};

} // End of namespace Agi

#endif /* AGI_H */
