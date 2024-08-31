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

#ifndef AGI_AGI_H
#define AGI_AGI_H

#include "common/scummsys.h"
#include "common/error.h"
#include "common/util.h"
#include "common/file.h"
#include "common/keyboard.h"
#include "common/rect.h"
#include "common/rendermode.h"
#include "common/stack.h"
#include "common/str.h"
#include "common/system.h"

#include "engines/engine.h"

#include "gui/debugger.h"

// AGI resources
#include "agi/console.h"
#include "agi/view.h"
#include "agi/picture.h"
#include "agi/logic.h"
#include "agi/sound.h"

namespace Common {
class RandomSource;
}

/**
 * This is the namespace of the AGI engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Early Sierra adventure games
 * - many fan made games
 * - Mickey's Space Adventure (Pre-AGI)
 * - Winnie the Pooh in the Hundred Acre Wood (Pre-AGI)
 * - Troll's Tale (Pre-AGI)
 */
namespace Agi {

#define TITLE       "AGI engine"

#define DIR_        "dir"
#define LOGDIR      "logdir"
#define PICDIR      "picdir"
#define VIEWDIR     "viewdir"
#define SNDDIR      "snddir"
#define OBJECTS     "object"
#define WORDS       "words.tok"

#define MAX_DIRECTORY_ENTRIES      256
#define MAX_CONTROLLERS            256
#define MAX_VARS                   256
#define MAX_FLAGS                  (256 >> 3)
#define SCREENOBJECTS_MAX          255        // KQ3 uses o255!
#define SCREENOBJECTS_EGO_ENTRY    0          // first entry is ego
#define MAX_WORDS                  20
#define MAX_STRINGS                24         // MAX_STRINGS + 1 used for get.num
#define MAX_STRINGLEN              40
#define MAX_CONTROLLER_KEYMAPPINGS 39

#define SAVEDGAME_DESCRIPTION_LEN 30

#define _EMPTY       0xfffff
#define EGO_OWNED    0xff
#define EGO_OWNED_V1 0xf9

#define CRYPT_KEY_SIERRA    "Avis Durgan"
#define CRYPT_KEY_AGDS      "Alex Simkin"

#define ADD_PIC  1
#define ADD_VIEW 2

#define CMD_BSIZE 12

enum AgiGameType {
	GType_PreAGI = 0,
	GType_V1 = 1,
	GType_V2 = 2,
	GType_V3 = 3,
	GType_A2 = 4
};

enum AgiGameFeatures {
	GF_AGIMOUSE    = (1 << 0), // marks games created with AGIMOUSE, disables "Click-to-walk mouse interface"
	GF_AGDS        = (1 << 1), // marks games created with AGDS - all using AGI version 2.440
	GF_AGI256      = (1 << 2), // marks fanmade AGI-256 games
	GF_FANMADE     = (1 << 3), // marks fanmade games
	GF_2GSOLDSOUND = (1 << 5),
	GF_EXTCHAR     = (1 << 6)  // use WORDS.TOK.EXTENDED
};

enum AgiGameID {
	GID_AGIDEMO,
	GID_BC,
	GID_DDP,
	GID_GOLDRUSH,	// V3
	GID_KQ1,
	GID_KQ2,
	GID_KQ3,
	GID_KQ4,
	GID_LSL1,
	GID_MH1,		// V3
	GID_MH2,		// V3
	GID_MIXEDUP,
	GID_PQ1,
	GID_SQ1,
	GID_SQ2,
	GID_XMASCARD,
	GID_FANMADE,
	GID_GETOUTTASQ, // Fanmade
	GID_MICKEY,     // PreAGI
	GID_WINNIE,     // PreAGI
	GID_TROLL       // PreAGI
};

enum AGIErrors {
	errOK = 0,
	errFilesNotFound,
	errBadFileOpen,
	errNotEnoughMemory,
	errBadResource,
	errIOError,

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
	RESOURCETYPE_LOGIC = 1,
	RESOURCETYPE_SOUND,
	RESOURCETYPE_VIEW,
	RESOURCETYPE_PICTURE
};

enum {
	RES_LOADED                 = 0x01,
	RES_COMPRESSED             = 0x40,
	RES_PICTURE_V3_NIBBLE_PARM = 0x80  // Flag that gets set for picture resources,
	                                   // which use a nibble instead of a byte as F0+F2 parameters
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
	Common::Point pos;

	Mouse() : button(0) {}
};

// Used by AGI Mouse protocol 1.0 for v27 (i.e. button pressed -variable).
enum AgiMouseButton {
	kAgiMouseButtonUp,    // Mouse button is up (not pressed)
	kAgiMouseButtonLeft,  // Left mouse button
	kAgiMouseButtonRight, // Right mouse button
	kAgiMouseButtonMiddle // Middle mouse button
};

/**
 * AGI variables.
 */
enum {
	VM_VAR_CURRENT_ROOM = 0,        // 0
	VM_VAR_PREVIOUS_ROOM,           // 1
	VM_VAR_BORDER_TOUCH_EGO,        // 2
	VM_VAR_SCORE,                   // 3
	VM_VAR_BORDER_CODE,             // 4
	VM_VAR_BORDER_TOUCH_OBJECT,     // 5
	VM_VAR_EGO_DIRECTION,           // 6
	VM_VAR_MAX_SCORE,               // 7
	VM_VAR_FREE_PAGES,              // 8
	VM_VAR_WORD_NOT_FOUND,          // 9
	VM_VAR_TIME_DELAY,              // 10
	VM_VAR_SECONDS,                 // 11
	VM_VAR_MINUTES,                 // 12
	VM_VAR_HOURS,                   // 13
	VM_VAR_DAYS,                    // 14
	VM_VAR_JOYSTICK_SENSITIVITY,    // 15
	VM_VAR_EGO_VIEW_RESOURCE,       // 16
	VM_VAR_AGI_ERROR_CODE,          // 17
	VM_VAR_AGI_ERROR_INFO,          // 18
	VM_VAR_KEY,                     // 19
	VM_VAR_COMPUTER,                // 20
	VM_VAR_WINDOW_AUTO_CLOSE_TIMER, // 21
	VM_VAR_SOUNDGENERATOR,          // 22
	VM_VAR_VOLUME,                  // 23
	VM_VAR_MAX_INPUT_CHARACTERS,    // 24
	VM_VAR_SELECTED_INVENTORY_ITEM, // 25
	VM_VAR_MONITOR = 26,            // 26
	VM_VAR_MOUSE_BUTTONSTATE = 27,  // 27
	VM_VAR_MOUSE_X = 28,            // 28
	VM_VAR_MOUSE_Y = 29             // 29
};

/**
 * Different monitor types.
 * Used with AGI variable 26 i.e. vMonitor.
 */
enum AgiMonitorType {
	kAgiMonitorCga = 0,
	//kAgiMonitorTandy = 1, // Not sure about this
	kAgiMonitorHercules = 2,
	kAgiMonitorEga = 3
	//kAgiMonitorVga = 4 // Not sure about this
};

/**
 * Different computer types.
 * Used with AGI variable 20 i.e. vComputer.
 */
enum AgiComputerType {
	kAgiComputerPC = 0,
	kAgiComputerApple2 = 3,
	kAgiComputerAtariST = 4,
	kAgiComputerAmiga = 5,
	kAgiComputerApple2GS = 7
};

enum AgiSoundType {
	kAgiSoundPC = 1,
	kAgiSoundTandy = 3, // Tandy (This value is also used by the Amiga AGI and Apple IIGS AGI)
	kAgiSound2GSOld = 8 // Apple IIGS's Gold Rush! (Version 1.0M 1989-02-28 (CE), AGI 3.003) uses value 8
};

/**
 * AGI flags
 */
enum {
	VM_FLAG_EGO_WATER = 0,  // 0
	VM_FLAG_EGO_INVISIBLE,
	VM_FLAG_ENTERED_CLI,
	VM_FLAG_EGO_TOUCHED_P2,
	VM_FLAG_SAID_ACCEPTED_INPUT,
	VM_FLAG_NEW_ROOM_EXEC,  // 5
	VM_FLAG_RESTART_GAME,
	VM_FLAG_SCRIPT_BLOCKED,
	VM_FLAG_JOY_SENSITIVITY,
	VM_FLAG_SOUND_ON,
	VM_FLAG_DEBUGGER_ON,        // 10
	VM_FLAG_LOGIC_ZERO_FIRST_TIME,
	VM_FLAG_RESTORE_JUST_RAN,
	VM_FLAG_STATUS_SELECTS_ITEMS,
	VM_FLAG_MENUS_ACCESSIBLE,
	VM_FLAG_OUTPUT_MODE,        // 15
	VM_FLAG_AUTO_RESTART
};

struct AgiControllerKeyMapping {
	uint16 keycode;
	byte   controllerSlot;

	AgiControllerKeyMapping() : keycode(0), controllerSlot(0) {}
};

struct AgiObject {
	int location;
	Common::String name;
};

struct AgiDir {
	uint8 volume;
	uint32 offset;
	uint32 len;
	uint32 clen;

	// 0 = not in mem, can be freed
	// 1 = in mem, can be released
	// 2 = not in mem, cant be released
	// 3 = in mem, cant be released
	// 0x40 = was compressed
	uint8 flags;

	void reset() {
		volume = 0xff;
		offset = _EMPTY;
		len = 0;
		clen = 0;
		flags = 0;
	}

	AgiDir() { reset(); }
};

struct AgiBlock {
	bool active;
	int16 x1, y1;
	int16 x2, y2;

	AgiBlock() : active(false), x1(0), y1(0), x2(0), y2(0) {}
};

struct ScriptPos {
	int script;
	int curIP;
};

enum CycleInnerLoopType {
	CYCLE_INNERLOOP_GETSTRING = 0,
	CYCLE_INNERLOOP_GETNUMBER,
	CYCLE_INNERLOOP_INVENTORY,
	CYCLE_INNERLOOP_MENU_VIA_KEYBOARD,
	CYCLE_INNERLOOP_MENU_VIA_MOUSE,
	CYCLE_INNERLOOP_SYSTEMUI_SELECTSAVEDGAMESLOT,
	CYCLE_INNERLOOP_SYSTEMUI_VERIFICATION,
	CYCLE_INNERLOOP_MESSAGEBOX,
	CYCLE_INNERLOOP_HAVEKEY
};

typedef Common::Array<int16> SavedGameSlotIdArray;

/**
 * AGI game structure.
 * This structure contains all global data of an AGI game executed
 * by the interpreter.
 */
struct AgiGame {
	AgiEngine *_vm;

	// TODO: Check whether adjMouseX and adjMouseY must be saved and loaded when using savegames.
	//       If they must be then loading and saving is partially broken at the moment.
	int adjMouseX;  /**< last given adj.ego.move.to.x.y-command's 1st parameter */
	int adjMouseY;  /**< last given adj.ego.move.to.x.y-command's 2nd parameter */

	char id[8];     /**< game id */
	uint32 crc;     /**< game CRC */

	// game flags and variables
	uint8 flags[MAX_FLAGS]; /**< 256 1-bit flags combined into a total of 32 bytes */
	uint8 vars[MAX_VARS];   /**< 256 variables */

	// internal variables
	int16 horizon;          /**< horizon y coordinate */

	bool  cycleInnerLoopActive;
	int16 cycleInnerLoopType;

	int16 curLogicNr;               /**< current logic number */
	Common::Array<ScriptPos> execStack;

	// internal flags
	bool playerControl; /**< player is in control */
	bool exitAllLogics; /**< break cycle after new.room */
	bool pictureShown;  /**< show.pic has been issued */

	// windows
	AgiBlock block;

	// graphics & text
	bool gfxMode;

	unsigned int numObjects;

	bool controllerOccurred[MAX_CONTROLLERS];  /**< keyboard keypress events */
	AgiControllerKeyMapping controllerKeyMapping[MAX_CONTROLLER_KEYMAPPINGS];

	char strings[MAX_STRINGS + 1][MAX_STRINGLEN]; /**< strings */

	// directory entries for resources
	AgiDir dirLogic[MAX_DIRECTORY_ENTRIES];
	AgiDir dirPic[MAX_DIRECTORY_ENTRIES];
	AgiDir dirView[MAX_DIRECTORY_ENTRIES];
	AgiDir dirSound[MAX_DIRECTORY_ENTRIES];

	// resources
	AgiPicture pictures[MAX_DIRECTORY_ENTRIES]; /**< AGI picture resources */
	AgiLogic logics[MAX_DIRECTORY_ENTRIES];     /**< AGI logic resources */
	AgiView views[MAX_DIRECTORY_ENTRIES];       /**< AGI view resources */
	AgiSound *sounds[MAX_DIRECTORY_ENTRIES];    /**< Pointers to AGI sound resources */

	AgiLogic *_curLogic;

	// view table
	ScreenObjEntry screenObjTable[SCREENOBJECTS_MAX];

	ScreenObjEntry addToPicView;

	bool automaticSave;             /**< set by CmdSetSimple() */
	char automaticSaveDescription[SAVEDGAME_DESCRIPTION_LEN + 1];

	Common::Rect mouseFence;        /**< rectangle set by fence.mouse command */
	bool mouseEnabled;              /**< if mouse is supposed to be active */
	bool mouseHidden;               /**< if mouse is currently hidden */

	// IF condition handling
	bool testResult;

	int max_logics;
	int logic_list[256];

	// used to detect situations, where the game shows some text and changes rooms right afterwards
	// for example Space Quest 2 intro right at the start
	// or Space Quest 2, when entering the vent also right at the start
	// The developers assumed that loading the new room would take a bit.
	// In ScummVM it's basically done in an instant, which means that
	// the text would only get shown for a split second.
	// We delay a bit as soon as such situations get detected.
	bool nonBlockingTextShown;
	int16 nonBlockingTextCyclesLeft;

	bool automaticRestoreGame;

	uint16 appleIIgsSpeedControllerSlot;
	int appleIIgsSpeedLevel;

	const char *getString(int number);
	void setString(int number, const char *str);
	void setAppleIIgsSpeedLevel(int appleIIgsSpeedLevel);

	AgiGame() {
		_vm = nullptr;

		adjMouseX = 0;
		adjMouseY = 0;

		memset(id, 0, sizeof(id));
		crc = 0;
		memset(flags, 0, sizeof(flags));
		memset(vars, 0, sizeof(vars));

		horizon = 0;

		cycleInnerLoopActive = false;
		cycleInnerLoopType = 0;

		curLogicNr = 0;

		// execStack is defaulted by Common::Array constructor

		playerControl = false;
		exitAllLogics = false;
		pictureShown = false;

		// block defaulted by AgiBlock constructor

		gfxMode = false;

		numObjects = 0;

		memset(controllerOccurred, 0, sizeof(controllerOccurred));

		// controllerKeyMapping defaulted by AgiControllerKeyMapping constructor

		memset(strings, 0, sizeof(strings));

		// dirLogic cleared by AgiDir constructor
		// dirPic cleared by AgiDir constructor
		// dirView cleared by AgiDir constructor
		// dirSound cleared by AgiDir constructor

		// pictures cleared by AgiPicture constructor
		// logics cleared by AgiLogic constructor
		// views cleared by AgiView constructor
		memset(sounds, 0, sizeof(sounds));

		_curLogic = nullptr;

		// screenObjTable cleared by ScreenObjEntry constructor

		// addToPicView cleared by ScreenObjEntry constructor

		automaticSave = false;
		memset(automaticSaveDescription, 0, sizeof(automaticSaveDescription));

		// mouseFence cleared by Common::Rect constructor
		mouseEnabled = false;
		mouseHidden = false;

		testResult = false;

		max_logics = 0;
		memset(logic_list, 0, sizeof(logic_list));

		nonBlockingTextShown = false;
		nonBlockingTextCyclesLeft = 0;

		automaticRestoreGame = false;

		appleIIgsSpeedControllerSlot = 0xffff;	// we didn't add yet speed menu
		appleIIgsSpeedLevel = 2;  // normal speed
	}
};

struct AgiDiskVolume {
	uint32 disk;
	uint32 offset;

	AgiDiskVolume() : disk(_EMPTY), offset(0) {}
	AgiDiskVolume(uint32 d, uint32 o) : disk(d), offset(o) {}
};

/**
 * Apple II version of the format for LOGDIR, VIEWDIR, etc.
 * See AgiLoader_A2::loadDir for more details.
 */
enum A2DirVersion {
	A2DirVersionOld,  // 4 bits for volume, 8 for track
	A2DirVersionNew,  // 5 bits for volume, 7 for track
};

class AgiLoader {
public:
	AgiLoader(AgiEngine *vm) : _vm(vm) {}
	virtual ~AgiLoader() {}

	/**
	 * Performs one-time initializations, such as locating files
	 * with dynamic names.
	 */
	virtual void init() {}

	/**
	 * Loads all AGI directory entries from disk and and populates
	 * the AgiDir arrays in AgiGame with them.
	 */
	virtual int loadDirs() = 0;

	/**
	 * Loads a volume resource from disk.
	 */
	virtual uint8 *loadVolumeResource(AgiDir *agid) = 0;

	/**
	 * Loads AgiEngine::_objects from disk.
	 */
	virtual int loadObjects() = 0;

	/**
	 * Loads AgiBase::_words from disk.
	 */
	virtual int loadWords() = 0;

protected:
	AgiEngine *_vm;
};

class AgiLoader_A2 : public AgiLoader {
public:
	AgiLoader_A2(AgiEngine *vm) : AgiLoader(vm) {}
	~AgiLoader_A2() override;

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::Array<Common::SeekableReadStream *> _disks;
	Common::Array<AgiDiskVolume> _volumes;
	AgiDir _logDir;
	AgiDir _picDir;
	AgiDir _viewDir;
	AgiDir _soundDir;
	AgiDir _objects;
	AgiDir _words;

	int readDiskOne(Common::SeekableReadStream &stream, Common::Array<uint32> &volumeMap);
	static bool readInitDir(Common::SeekableReadStream &stream, byte index, AgiDir &agid);
	static bool readDir(Common::SeekableReadStream &stream, int position, AgiDir &agid);
	static bool readVolumeMap(Common::SeekableReadStream &stream, uint32 position, uint32 bufferLength, Common::Array<uint32> &volumeMap);

	A2DirVersion detectDirVersion(Common::SeekableReadStream &stream);
	bool loadDir(AgiDir *dir, Common::SeekableReadStream &disk, uint32 dirOffset, uint32 dirLength, A2DirVersion dirVersion);
};

class AgiLoader_v1 : public AgiLoader {
public:
	AgiLoader_v1(AgiEngine *vm) : AgiLoader(vm) {}

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;

private:
	Common::Array<Common::String> _imageFiles;
	Common::Array<AgiDiskVolume> _volumes;
	AgiDir _logDir;
	AgiDir _picDir;
	AgiDir _viewDir;
	AgiDir _soundDir;
	AgiDir _objects;
	AgiDir _words;

	bool readDiskOneV1(Common::SeekableReadStream &stream);
	bool readDiskOneV2001(Common::SeekableReadStream &stream, int &vol0Offset);
	static bool readInitDirV1(Common::SeekableReadStream &stream, byte index, AgiDir &agid);
	static bool readInitDirV2001(Common::SeekableReadStream &stream, byte index, AgiDir &agid);

	bool loadDir(AgiDir *dir, Common::File &disk, uint32 dirOffset, uint32 dirLength);
};

class AgiLoader_v2 : public AgiLoader {
private:
	bool _hasV3VolumeFormat;

	int loadDir(AgiDir *agid, const char *fname);
	bool detectV3VolumeFormat();

public:
	AgiLoader_v2(AgiEngine *vm) : _hasV3VolumeFormat(false), AgiLoader(vm) {}

	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;
};

class AgiLoader_v3 : public AgiLoader {
private:
	Common::String _name; /**< prefix in directory and/or volume file names (`GR' for goldrush) */

	int loadDir(AgiDir *agid, Common::File *fp, uint32 offs, uint32 len);

public:
	AgiLoader_v3(AgiEngine *vm) : AgiLoader(vm) {}

	void init() override;
	int loadDirs() override;
	uint8 *loadVolumeResource(AgiDir *agid) override;
	int loadObjects() override;
	int loadWords() override;
};

class GfxFont;
class GfxMgr;
class SpritesMgr;
class InventoryMgr;
class TextMgr;
class GfxMenu;
class SystemUI;
class Words;
struct AGIGameDescription;

// Image stack support
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

#define TICK_SECONDS 20

#define KEY_QUEUE_SIZE 16

class AgiBase : public ::Engine {
protected:
	// Engine API
	Common::Error init();
	virtual Common::Error go() = 0;
	Common::Error run() override {
		Common::Error err;
		err = init();
		if (err.getCode() != Common::kNoError)
			return err;
		return go();
	}
	bool hasFeature(EngineFeature f) const override;

	virtual void initialize() = 0;

	void initRenderMode();

public:
	Words *_words;

	GfxFont *_font;
	GfxMgr  *_gfx;

	Common::RenderMode _renderMode;
	AgiDebug _debug;
	AgiGame _game;
	Common::RandomSource *_rnd;

	SoundMgr *_sound;

	Mouse _mouse;

	bool _noSaveLoadAllowed;

	virtual bool promptIsEnabled() {
		return false;
	}

	virtual int getKeypress() = 0;
	virtual bool isKeypress() = 0;
	virtual void clearKeyQueue() = 0;

	AgiBase(OSystem *syst, const AGIGameDescription *gameDesc);
	~AgiBase() override;

	virtual void clearImageStack() = 0;
	virtual void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                                  int16 p4, int16 p5, int16 p6, int16 p7) = 0;
	virtual void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                                  int16 p4, int16 p5, int16 p6, int16 p7) = 0;
	virtual void releaseImageStack() = 0;

	int _soundemu;

	bool getFlag(int16 flagNr);
	void setFlag(int16 flagNr, bool newState);
	void flipFlag(int16 flagNr);

	const AGIGameDescription *_gameDescription;

	uint32 _gameFeatures;
	uint16 _gameVersion;

	uint32 getGameID() const;
	uint32 getFeatures() const;
	uint16 getVersion() const;
	uint16 getGameType() const;
	Common::Language getLanguage() const;
	bool isLanguageRTL() const;
	Common::Platform getPlatform() const;
	const char *getGameMD5() const;
	void initFeatures();
	void initVersion();

	const char *getDiskName(uint16 id);

	bool canLoadGameStateCurrently(Common::U32String *msg = nullptr) override;
	bool canSaveGameStateCurrently(Common::U32String *msg = nullptr) override;

	const byte *getFontData();

	void cycleInnerLoopActive(int16 loopType) {
		_game.cycleInnerLoopActive = true;
		_game.cycleInnerLoopType = loopType;
	};
	void cycleInnerLoopInactive() {
		_game.cycleInnerLoopActive = false;
	};
	bool cycleInnerLoopIsActive() {
		return _game.cycleInnerLoopActive;
	}
};

enum AgiArtificialDelayTriggerType {
	ARTIFICIALDELAYTYPE_NEWROOM = 0,
	ARTIFICIALDELAYTYPE_NEWPICTURE = 1,
	ARTIFICIALDELAYTYPE_END = -1
};

struct AgiArtificialDelayEntry {
	uint32 gameId;
	Common::Platform platform;
	AgiArtificialDelayTriggerType triggerType;
	int16 orgNr;
	int16 newNr;
	uint16 millisecondsDelay;
};

typedef void (*AgiOpCodeFunction)(AgiGame *state, AgiEngine *vm, uint8 *p);

struct AgiOpCodeEntry {
	const char *name;
	const char *parameters;
	AgiOpCodeFunction functionPtr;
	uint16     parameterSize;
};

struct AgiOpCodeDefinitionEntry {
	const char *name;
	const char *parameters;
	AgiOpCodeFunction functionPtr;
};

class AgiEngine : public AgiBase {
protected:
	// Engine APIs
	Common::Error go() override;

	void initialize() override;

public:
	AgiEngine(OSystem *syst, const AGIGameDescription *gameDesc);
	~AgiEngine() override;

	bool promptIsEnabled() override;

	Common::Error loadGameState(int slot) override;
	Common::Error saveGameState(int slot, const Common::String &description, bool isAutosave = false) override;

private:
	int _keyQueue[KEY_QUEUE_SIZE];
	int _keyQueueStart;
	int _keyQueueEnd;

	bool _allowSynthetic;

	bool checkPriority(ScreenObjEntry *v);
	bool checkCollision(ScreenObjEntry *v);
	bool checkPosition(ScreenObjEntry *v);

	int _firstSlot;

public:
	Common::Array<AgiObject> _objects;    // objects in the game

	SavedGameSlotIdArray getSavegameSlotIds();
	bool getSavegameInformation(int16 slotId, Common::String &saveDescription, uint32 &saveDate, uint32 &saveTime, bool &saveIsValid);

	int saveGame(const Common::String &fileName, const Common::String &descriptionString);
	int loadGame(const Common::String &fileName, bool checkId = true);
	bool saveGameDialog();
	bool saveGameAutomatic();
	bool loadGameDialog();
	bool loadGameAutomatic();
	int doSave(int slot, const Common::String &desc);
	int doLoad(int slot, bool showMessages);
	int scummVMSaveLoadDialog(bool isSave);

	uint8 *_intobj;
	bool _restartGame;

	SpritesMgr *_sprites;
	TextMgr *_text;
	InventoryMgr *_inventory;
	PictureMgr *_picture;
	AgiLoader *_loader;
	GfxMenu *_menu;
	SystemUI *_systemUI;
	Common::DumpFile *_logFile; // File used for the log() agi command.

	Common::Stack<ImageStackElement> _imageStack;

	void clearImageStack() override;
	void recordImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                          int16 p4, int16 p5, int16 p6, int16 p7) override;
	void replayImageStackCall(uint8 type, int16 p1, int16 p2, int16 p3,
	                          int16 p4, int16 p5, int16 p6, int16 p7) override;
	void releaseImageStack() override;

	void wait(uint32 msec, bool busy = false);

	int agiInit();
	void agiDeinit();
	int loadResource(int16 resourceType, int16 resourceNr);
	void unloadResource(int16 resourceType, int16 resourceNr);
	/**
	 * Unload all resources except Logic 0
	 */
	void unloadResources();

	int getKeypress() override;
	bool isKeypress() override;
	void clearKeyQueue() override;

	byte getVar(int16 varNr);
	void setVar(int16 varNr, byte newValue);

private:
	void applyVolumeToMixer();

public:
	void syncSoundSettings() override;

public:
	void decrypt(uint8 *mem, int len);
	uint16 processAGIEvents();
	int runGame();

	void newRoom(int16 newRoomNr);
	void resetControllers();
	void interpretCycle();
	void playGame();

	void allowSynthetic(bool);
	void processScummVMEvents();
	void checkQuickLoad();

	const Common::String getTargetName() const { return _targetName; }

	// Objects
public:
	int loadObjects(const char *fname);
	int loadObjects(Common::SeekableReadStream &fp, int flen);
	const char *objectName(uint16 objectNr);
	int objectGetLocation(uint16 objectNr);
	void objectSetLocation(uint16 objectNr, int location);
private:
	int decodeObjects(uint8 *mem, uint32 flen);

	// Logic
public:
	int decodeLogic(int16 logicNr);
	void unloadLogic(int16 logicNr);
	int runLogic(int16 logicNr);
	void debugConsole(int lognum, int mode, const char *str);
	bool testIfCode(int16 logicNr);
	void executeAgiCommand(uint8 op, uint8 *p);

private:
	bool _veryFirstInitialCycle; /**< signals, that currently the very first cycle is executed (restarts, etc. do not count!) */
	uint32 _instructionCounter; /**< counts every instruction, that got executed, can wrap around */

	bool _setVolumeBrokenFangame;

	void resetGetVarSecondsHeuristic();
	void getVarSecondsHeuristicTrigger();
	uint32 _getVarSecondsHeuristicLastInstructionCounter; /**< last time VM_VAR_SECONDS were read */
	uint16 _getVarSecondsHeuristicCounter; /**< how many times heuristic was triggered */

	uint32 _playTimeInSecondsAdjust; /**< milliseconds to adjust for calculating current play time in seconds, see setVarSecondsTrigger() */

	void setVarSecondsTrigger(byte newSeconds);

public:
	// Some submethods of testIfCode
	void skipInstruction(byte op);
	void skipInstructionsUntil(byte v);
	bool testObjRight(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2);
	bool testObjCenter(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2);
	bool testObjInBox(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2);
	bool testPosn(uint8 n, uint8 x1, uint8 y1, uint8 x2, uint8 y2);
	bool testSaid(uint8 nwords, uint8 *cc);
	bool testController(uint8 cont);
	bool testCompareStrings(uint8 s1, uint8 s2);

	// View
private:
	void updateView(ScreenObjEntry *screenObj);

public:
	void setView(ScreenObjEntry *screenObj, int16 viewNr);
	void setLoop(ScreenObjEntry *screenObj, int16 loopNr);
	void setCel(ScreenObjEntry *screenObj, int16 celNr);

	void clipViewCoordinates(ScreenObjEntry *screenObj);

	void startUpdate(ScreenObjEntry *viewPtr);
	void stopUpdate(ScreenObjEntry *viewPtr);
	void updateScreenObjTable();
	void unloadView(int16 viewNr);
	int decodeView(byte *resourceData, uint16 resourceSize, int16 viewNr);

private:
	void unpackViewCelData(AgiViewCel *celData, byte *compressedData, uint16 compressedSize, int16 viewNr);
	void unpackViewCelDataAGI256(AgiViewCel *celData, byte *compressedData, uint16 compressedSize, int16 viewNr);

public:
	bool isEgoView(const ScreenObjEntry *screenObj);

	// Motion
private:
	int checkStep(int delta, int step);
	bool checkBlock(int16 x, int16 y);
	void changePos(ScreenObjEntry *screenObj);
	void motionWander(ScreenObjEntry *screenObj);
	void motionFollowEgo(ScreenObjEntry *screenObj);
	void motionMoveObj(ScreenObjEntry *screenObj);
	void motionMoveObjStop(ScreenObjEntry *screenObj);
	void checkMotion(ScreenObjEntry *screenObj);

public:
	void motionActivated(ScreenObjEntry *screenObj);
	void cyclerActivated(ScreenObjEntry *screenObj);
	void checkAllMotions();
	void moveObj(ScreenObjEntry *screenObj);
	void inDestination(ScreenObjEntry *screenObj);
	void fixPosition(int16 screenObjNr);
	void fixPosition(ScreenObjEntry *screenObj);
	void updatePosition();
	int getDirection(int16 objX, int16 objY, int16 destX, int16 destY, int16 stepSize);

	bool _keyHoldMode;
	Common::KeyCode _keyHoldModeLastKey;

	// Keyboard
	int doPollKeyboard();

	bool handleMouseClicks(uint16 &key);
	bool handleController(uint16 key);

	bool showPredictiveDialog();

	int waitKey();
	int waitAnyKey();
	void waitAnyKeyOrFinishedSound();

	void nonBlockingText_IsShown();
	void nonBlockingText_Forget();

	void artificialDelay_Reset();
	void artificialDelay_CycleDone();

	uint16 artificialDelay_SearchTable(AgiArtificialDelayTriggerType triggerType, int16 orgNr, int16 newNr);

	void artificialDelayTrigger_NewRoom(int16 newRoomNr);
	void artificialDelayTrigger_DrawPicture(int16 newPictureNr);

private:
	int16 _artificialDelayCurrentRoom;
	int16 _artificialDelayCurrentPicture;

public:
	void redrawScreen();

	void inGameTimerReset(uint32 newPlayTime = 0);
	void inGameTimerResetPassedCycles();
	uint32 inGameTimerGet();
	uint32 inGameTimerGetPassedCycles();

	void inGameTimerUpdate();

private:
	uint32 _lastUsedPlayTimeInCycles; // 40 per second
	uint32 _lastUsedPlayTimeInSeconds; // actual seconds
	uint32 _passedPlayTimeCycles; // increased by 1 every time we passed a cycle

private:
	AgiOpCodeEntry _opCodes[256]; // always keep those at 256, so that there is no way for invalid memory access
	AgiOpCodeEntry _opCodesCond[256];

	void setupOpCodes(uint16 version);

public:
	const AgiOpCodeEntry *getOpCodesTable() { return _opCodes; }
};

} // End of namespace Agi

#endif /* AGI_AGI_H */
