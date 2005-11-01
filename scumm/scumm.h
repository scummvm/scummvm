/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * $Header$
 *
 */

#ifndef SCUMM_H
#define SCUMM_H

#include "base/engine.h"
#include "common/file.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"

#include "scumm/gfx.h"
#include "scumm/script.h"

namespace GUI {
	class Dialog;
}
using GUI::Dialog;
class GameDetector;
namespace Common {
	class InSaveFile;
	class OutSaveFile;
}

namespace Scumm {

class Actor;
class BaseCostumeLoader;
class BaseCostumeRenderer;
class BaseScummFile;
class CharsetRenderer;
class IMuse;
class IMuseDigital;
class Insane;
class MusicEngine;
class ScummEngine;
class ScummDebugger;
class Serializer;
class Sound;

struct Box;
struct BoxCoords;
struct FindObjectInRoom;
struct ScummGameSettings;

// Use g_scumm from error() ONLY
extern ScummEngine *g_scumm;

/* System Wide Constants */
enum {
	NUM_SENTENCE = 6,
	NUM_SHADOW_PALETTE = 8
};

/**
 * SCUMM feature flags define for every game which specific set of engine
 * features are used by that game.
 * Note that some of them could be replaced by checks for the SCUMM version.
 */
enum GameFeatures {
	/** Games with the new camera system (ScummEngine_v7 and subclasses). */
	GF_NEW_CAMERA          = 1 << 1,

	/** Games with the AKOS costume system (ScummEngine_v7 and subclasses, HE games). */
	GF_NEW_COSTUMES        = 1 << 2,

	/** Games with digital iMUSE (ScummEngine_v7 and subclasses). */
	GF_DIGI_IMUSE          = 1 << 3,

	/** Games using XOR encrypted data files. */
	GF_USE_KEY             = 1 << 4,

	/** Small header games (ScummEngine_v4 and subclasses). */
	GF_SMALL_HEADER        = 1 << 5,

	/** Old bundle games (ScummEngine_v3old and subclasses). */
	GF_OLD_BUNDLE          = 1 << 6,

	/** EGA games. */
	GF_16COLOR             = 1 << 7,

	/** VGA versions of V3 games.  Equivalent to (version == 3 && not GF_16COLOR) */
	GF_OLD256              = 1 << 8,

	/** Games which have Audio CD tracks. */
	GF_AUDIOTRACKS         = 1 << 9,

	/** Games without actor scaling (ScummEngine_v3 and subclasses). */
	GF_NO_SCALING          = 1 << 10,

	/**
	 * Games using only very few local variables in scripts.
	 * Apparently that is only the case for 256 color version of Indy3.
	 */
	GF_FEW_LOCALS          = 1 << 11,

	/** HE games without cursor resources */
	GF_HE_CURSORLESS       = 1 << 12,

	/** HE games for which localized versions exist */
	GF_HE_LOCALIZED        = 1 << 13,

	/**
	 *  HE Games with more global scripts and different sprite handling
	 *  i.e. read it as HE version 9.85. Used for HE98 only.
	 */
	GF_HE_985		= 1 << 14,

	/** HE games with 16 bit color */
	GF_16BIT_COLOR         = 1 << 15,

	/** HE games which use sprites for subtitles */
	GF_HE_NOSUBTITLES      = 1 << 16,

	/** A demo, not a full blown game. */
	GF_DEMO                = 1 << 17
};

/* SCUMM Debug Channels */
void CDECL debugC(int level, const char *s, ...);

struct dbgChannelDesc {
	const char *channel, *desc;
	uint32 flag;
};

enum {
	DEBUG_GENERAL	=	1 << 0,		// General debug
	DEBUG_SCRIPTS	=	1 << 2,		// Track script execution (start/stop/pause)
	DEBUG_OPCODES	=	1 << 3,		// Track opcode invocations
	DEBUG_VARS	=	1 << 4,		// Track variable changes
	DEBUG_RESOURCE	=	1 << 5,		// Track resource loading / allocation
	DEBUG_IMUSE	=	1 << 6,		// Track iMUSE events
	DEBUG_SOUND	=	1 << 7,		// General Sound Debug
	DEBUG_ACTORS	=	1 << 8,		// General Actor Debug
	DEBUG_INSANE	=	1 << 9,		// Track INSANE
	DEBUG_SMUSH	=	1 << 10		// Track SMUSH
};

/**
 * Internal header for any memory block allocated by the resource manager.
 *
 * @todo Hide MemBlkHeader; no code outside the resource manager should
 * have to use it, ever. Currently script code needs it to detect whether
 * some scripts have moved (in fetchScriptByte()).
 */
struct MemBlkHeader {
	uint32 size;
};

struct VerbSlot;
struct ObjectData;

enum {
	LIGHTMODE_dark			= 0,
	LIGHTMODE_actor_base	= 1,
	LIGHTMODE_screen		= 2,
	LIGHTMODE_flashlight	= 4,
	LIGHTMODE_actor_color	= 8
};

enum {
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK | MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
};

enum ScummGameId {
	GID_TENTACLE,
	GID_MONKEY2,
	GID_INDY4,
	GID_MONKEY,
	GID_SAMNMAX,
	GID_MONKEY_EGA,
	GID_PASS,
	GID_LOOM256,
	GID_ZAK256,
	GID_INDY3,
	GID_LOOM,
	GID_FT,
	GID_DIG,
	GID_MONKEY_VGA,
	GID_CMI,
	GID_MANIAC,
	GID_ZAK,
	GID_MONKEY_SEGA,
	GID_HEGAME,      // Generic name for all HE games with default behaviour
	GID_FBEAR,
	GID_FUNPACK,
	GID_FREDDI2,
	GID_PUTTDEMO,
	GID_PUTTRACE,
	GID_FUNSHOP,	// Used for all three funshops
	GID_FOOTBALL
};

struct SentenceTab {
	byte verb;
	byte preposition;
	uint16 objectA;
	uint16 objectB;
	uint8 freezeCount;
};

struct StringSlot {
	int16 xpos;
	int16 ypos;
	int16 right;
	int16 height;
	byte color;
	byte charset;
	bool center;
	bool overhead;
	bool no_talk_anim;
};

struct StringTab : StringSlot {
	// The 'default' values for this string slot. This is used so that the
	// string slot can temporarily be set to different values, and then be
	// easily reset to a previously set default.
	StringSlot _default;

	void saveDefault() {
		StringSlot &s = *this;
		_default = s;
	}

	void loadDefault() {
		StringSlot &s = *this;
		s = _default;
	}
};



enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4
};

struct AuxBlock {
	bool visible;
	Common::Rect r;

	void reset() {
		visible = false;
		r.left = r.top = 0;
		r.right = r.bottom = -1;
	}
};

struct AuxEntry {
	int actorNum;
	int subIndex;
};

// TODO: Rename InfoStuff to something more descriptive
struct InfoStuff {
	uint32 date;
	uint16 time;
	uint32 playtime;
};

enum ResTypes {
	rtFirst = 1,
	rtRoom = 1,
	rtScript = 2,
	rtCostume = 3,
	rtSound = 4,
	rtInventory = 5,
	rtCharset = 6,
	rtString = 7,
	rtVerb = 8,
	rtActorName = 9,
	rtBuffer = 10,
	rtScaleTable = 11,
	rtTemp = 12,
	rtFlObject = 13,
	rtMatrix = 14,
	rtBox = 15,
	rtObjectName = 16,
	rtRoomScripts = 17,
	rtRoomImage = 18,
	rtImage = 19,
	rtTalkie = 20,
	rtLast = 20,
	rtNumTypes = 21
};

class ResourceManager {
	friend class ScummDebugger;
	friend class ScummEngine;
protected:
	ScummEngine *_vm;

public:
	byte mode[rtNumTypes];
	uint16 num[rtNumTypes];
	uint32 tags[rtNumTypes];
	const char *name[rtNumTypes];
	byte **address[rtNumTypes];
protected:
	byte *flags[rtNumTypes];
public:
	byte *roomno[rtNumTypes];
	uint32 *roomoffs[rtNumTypes];
	uint32 *globsize[rtNumTypes];

	uint32 _allocatedSize;
	uint32 _maxHeapThreshold, _minHeapThreshold;
	byte _expireCounter;

public:
	ResourceManager(ScummEngine *vm);

	byte *createResource(int type, int index, uint32 size);
	void nukeResource(int type, int i);

	void freeResources();

	bool isResourceLoaded(int type, int index) const;

	void lock(int type, int i);
	void unlock(int type, int i);
	bool isLocked(int type, int i) const;

	void setResourceCounter(int type, int index, byte flag);
	void increaseResourceCounter();

	void resourceStats();
	void expireResources(uint32 size);

protected:
	bool validateResource(const char *str, int type, int index) const;
};

class ScummEngine : public Engine {
	friend class ScummDebugger;
	friend class SmushPlayer;
	friend class Insane;
	friend class CharsetRenderer;
	friend class ResourceManager;

	void errorString(const char *buf_input, char *buf_output);
public:
	/* Put often used variables at the top.
	 * That results in a shorter form of the opcode
	 * on some architectures. */
	IMuse *_imuse;
	IMuseDigital *_imuseDigital;
	MusicEngine *_musicEngine;
	Sound *_sound;

	VerbSlot *_verbs;
	ObjectData *_objs;
	ObjectData *_storedFlObjects;
	ScummDebugger *_debugger;

	// Core variables
	byte _gameId;
	byte _version;
	uint8 _heversion;
	uint32 _features;						// Should only be accessed for reading (TODO enforce it compiler-wise with making it private and creating an accessor)
	Common::Platform _platform;
	uint8 _gameMD5[16];

	/** Random number generator */
	Common::RandomSource _rnd;

	/** Graphics manager */
	Gdi gdi;

	/** Central resource data. */
	ResourceManager res;

protected:
	VirtualMachineState vm;

public:
	// Constructor / Destructor
	ScummEngine(GameDetector *detector, OSystem *syst, const ScummGameSettings &gs, uint8 md5sum[16], int substResFileNameIndex);
	virtual ~ScummEngine();

	/** Startup function, main loop. */
	int go();

	// Init functions
	int init(GameDetector &detector);

	virtual void setupScummVars();
	virtual void initScummVars();

	virtual void scummInit();

	void loadCJKFont();
	void setupMusic(int midi);
	void setupVolumes();

	// Scumm main loop
	int scummLoop(int delta);

	// Event handling
	void parseEvents();
	void waitForTimer(int msec_delay);
	void processKbd(bool smushMode);
	void clearClickedStatus();

	// Misc utility functions
	uint32 _debugFlags;
	const char *getGameName() const { return _gameName.c_str(); }

	// Cursor/palette
	void updateCursor();
	virtual void animateCursor() {}
	virtual void updatePalette();

	/**
	 * Flag which signals that the SMUSH video playback should end now
	 * (e.g. because it was aborted by the user or it's simply finished).
	 */
	bool _smushVideoShouldFinish;
	/** This flag is a hack to allow the pause dialog to pause SMUSH playback, too. */
	bool _smushPaused;
	/** This flag tells IMuseDigital that INSANE is running. */
	bool _insaneRunning;

protected:
	Insane *_insane;

public:
	void pauseGame();
	void restart();
	void shutDown();

	/** We keep running until this is set to true. */
	bool _quit;

protected:
	Dialog *_pauseDialog;
	Dialog *_versionDialog;
	Dialog *_mainMenuDialog;

	int runDialog(Dialog &dialog);
	void confirmExitDialog();
	void confirmRestartDialog();
	void pauseDialog();
	void versionDialog();
	void mainMenuDialog();

	char displayMessage(const char *altButton, const char *message, ...);

	byte _fastMode;

	byte _numActors;
	Actor *_actors;	// Has _numActors elements
	Actor **_sortedActors;

	byte *_arraySlot;
	uint16 *_inventory;
	uint16 *_newNames;
public:
	// VAR is a wrapper around scummVar, which attempts to include additional
	// useful information should an illegal var access be detected.
	#define VAR(x)	scummVar(x, #x, __FILE__, __LINE__)
	int32& scummVar(byte var, const char *varName, const char *file, int line)
	{
		if (var == 0xFF) {
			error("Illegal access to variable %s in file %s, line %d", varName, file, line);
		}
		return _scummVars[var];
	}
	int32 scummVar(byte var, const char *varName, const char *file, int line) const
	{
		if (var == 0xFF) {
			error("Illegal access to variable %s in file %s, line %d", varName, file, line);
		}
		return _scummVars[var];
	}

protected:
	int16 _varwatch;
	int32 *_roomVars;
	int32 *_scummVars;
	byte *_bitVars;

	/* Global resource tables */
	int _numVariables, _numBitVariables, _numLocalObjects;
	int _numGlobalObjects, _numArray, _numVerbs, _numFlObject;
	int _numInventory;
	int _numNewNames, _numGlobalScripts;
	int _numRoomVariables;
	int _numPalettes, _numSprites, _numTalkies, _numUnk;
	int _numStoredFlObjects;
	int _HEHeapSize;
public:
	int _numLocalScripts, _numImages, _numRooms, _numScripts, _numSounds;	// Used by HE games
	int _numCostumes;	// FIXME - should be protected, used by Actor::remapActorPalette
	int _numCharsets;	// FIXME - should be protected, used by CharsetRenderer

	BaseCostumeLoader *_costumeLoader;
	BaseCostumeRenderer *_costumeRenderer;

	int _NESCostumeSet;
	void NES_loadCostumeSet(int n);
	byte *_NEScostdesc, *_NEScostlens, *_NEScostoffs, *_NEScostdata;
	byte _NESPatTable[2][4096];
	byte _NESPalette[2][16];
	byte _NESBaseTiles;

	int _NESStartStrip;

protected:
	/* Current objects - can go in their respective classes */
	byte _curActor;
	int _curVerb;
	int _curVerbSlot;
	int _curPalIndex;

public:
	byte _currentRoom;	// FIXME - should be protected but Actor::isInCurrentRoom uses it
	int _roomResource;  // FIXME - should be protected but Sound::pauseSounds uses it
	bool _egoPositioned;	// Used by Actor::putActor, hence public

	int generateSubstResFileName(const char *filename, char *buf, int bufsize, int index = -3);
	int _substResFileNameIndex;
	int _substResFileNameIndexBundle; // Used with Mac bundles

protected:
	int _keyPressed;
	uint16 _lastKeyHit;
	bool _keyDownMap[512]; // FIXME - 512 is a guess. it's max(kbd.ascii)

	Common::Point _mouse;
	Common::Point _virtualMouse;

	uint16 _mouseAndKeyboardStat;
	byte _leftBtnPressed, _rightBtnPressed;

	/** The bootparam, to be passed to the script 1, the bootscript. */
	int _bootParam;

	// Various options useful for debugging
	bool _dumpScripts;
	bool _hexdumpScripts;
	bool _showStack;
	uint16 _debugMode;

	// Save/Load class - some of this may be GUI
	byte _saveLoadFlag, _saveLoadSlot;
	uint32 _lastSaveTime;
	bool _saveTemporaryState;
	char _saveLoadName[32];

	bool saveState(int slot, bool compat);
	bool loadState(int slot, bool compat);
	virtual void saveOrLoad(Serializer *s);
	void saveLoadResource(Serializer *ser, int type, int index);	// "Obsolete"
	void saveResource(Serializer *ser, int type, int index);
	void loadResource(Serializer *ser, int type, int index);
	void makeSavegameName(char *out, int slot, bool temporary);

	int getKeyState(int key);

public:
	bool getSavegameName(int slot, char *desc);
	void listSavegames(bool *marks, int num);

	void requestSave(int slot, const char *name, bool temporary = false);
	void requestLoad(int slot);

// thumbnail + info stuff
public:
	Graphics::Surface *loadThumbnailFromSlot(int slot);
	bool loadInfosFromSlot(int slot, InfoStuff *stuff);

protected:
	Graphics::Surface *loadThumbnail(Common::InSaveFile *file);
	bool loadInfos(Common::InSaveFile *file, InfoStuff *stuff);
	void saveThumbnail(Common::OutSaveFile *file);
	void saveInfos(Common::OutSaveFile* file);

	int32 _engineStartTime;
	int32 _dialogStartTime;

protected:
	/* Script VM - should be in Script class */
	uint32 _localScriptOffsets[1024];
	const byte *_scriptPointer, *_scriptOrgPointer;
	byte _opcode, _currentScript;
	uint16 _curExecScript;
	const byte * const *_lastCodePtr;
	int _resultVarNumber, _scummStackPos;
	int _vmStack[150];
	int _keyScriptKey, _keyScriptNo;

	virtual void setupOpcodes() = 0;
	virtual void executeOpcode(byte i) = 0;
	virtual const char *getOpcodeDesc(byte i) = 0;

	void initializeLocals(int slot, int *vars);
	int	getScriptSlot();

	void startScene(int room, Actor *a, int b);
	void startManiac();

public:
	void runScript(int script, bool freezeResistant, bool recursive, int *lvarptr, int cycle = 0);
	void stopScript(int script);
	void nukeArrays(byte scriptSlot);

protected:
	void runObjectScript(int script, int entry, bool freezeResistant, bool recursive, int *vars, int slot = -1, int cycle = 0);
	void runScriptNested(int script);
	void executeScript();
	void updateScriptPtr();
	virtual void runInventoryScript(int i);
	void inventoryScript();
	void checkAndRunSentenceScript();
	void runExitScript();
	void runEntryScript();
	void runAllScripts();
	void freezeScripts(int scr);
	void unfreezeScripts();

	bool isScriptInUse(int script) const;
	bool isRoomScriptRunning(int script) const;
	bool isScriptRunning(int script) const;

	void killAllScriptsExceptCurrent();
	void killScriptsAndResources();
	void decreaseScriptDelay(int amount);

	void stopObjectCode();
	void stopObjectScript(int script);

	void getScriptBaseAddress();
	void getScriptEntryPoint();
	int getVerbEntrypoint(int obj, int entry);

	byte fetchScriptByte();
	virtual uint fetchScriptWord();
	virtual int fetchScriptWordSigned();
	void ignoreScriptWord() { fetchScriptWord(); }
	void ignoreScriptByte() { fetchScriptByte(); }
	virtual void getResultPos();
	void setResult(int result);
	void push(int a);
	int pop();
	virtual int readVar(uint var);
	virtual void writeVar(uint var, int value);

	void beginCutscene(int *args);
	void endCutscene();
	void abortCutscene();
	void beginOverride();
	void endOverride();

	void copyScriptString(byte *dst);
	int resStrLen(const byte *src) const;
	void doSentence(int c, int b, int a);

	/* Should be in Resource class */
	BaseScummFile *_fileHandle;
	uint32 _fileOffset;
public:
	/** The name of the (macintosh/rescumm style) container file, if any. */
	Common::String _containerFile;

	bool openFile(BaseScummFile &file, const char *filename);

protected:
	int _resourceHeaderSize;
	Common::String _gameName;	// This is the name we use for opening resource files
	Common::String _targetName;	// This is the game the user calls it, so use for saving
	byte _resourceMapper[128];
	byte *_heV7DiskOffsets;
	uint32 *_heV7RoomIntOffsets;
	const byte *_resourceLastSearchBuf; // FIXME: need to put it to savefile?
	uint32 _resourceLastSearchSize;    // FIXME: need to put it to savefile?

	virtual void allocateArrays();
	void openRoom(int room);
	void closeRoom();
	void deleteRoomOffsets();
	virtual void readRoomsOffsets();
	void askForDisk(const char *filename, int disknum);
	bool openResourceFile(const char *filename, byte encByte);

	void loadPtrToResource(int type, int i, const byte *ptr);
	virtual void readResTypeList(int id, uint32 tag, const char *name);
	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
//	byte *createResource(int type, int index, uint32 size);
	int loadResource(int type, int i);
//	void nukeResource(int type, int i);
	int getResourceSize(int type, int idx);

public:
	byte *getResourceAddress(int type, int i);
	virtual byte *getStringAddress(int i);
	byte *getStringAddressVar(int i);
	void ensureResourceLoaded(int type, int i);
	int getResourceRoomNr(int type, int index);

protected:
	int readSoundResource(int type, int index);
	int convert_extraflags(byte *ptr, byte * src_ptr);
	void convertMac0Resource(int type, int index, byte *ptr, int size);
	void convertADResource(int type, int index, byte *ptr, int size);
	int readSoundResourceSmallHeader(int type, int index);
	bool isResourceInUse(int type, int i) const;
	virtual void loadRoomSubBlocks();
	virtual void initRoomSubBlocks();
	void clearRoomObjects();
	void storeFlObject(int slot);
	void restoreFlObjects();
	virtual void loadRoomObjects();

	virtual void readArrayFromIndexFile();
	virtual void readMAXS(int blockSize) = 0;
	virtual void readGlobalObjects();
	virtual void readIndexFile();
	virtual void readIndexBlock(uint32 block, uint32 itemsize);
	virtual void loadCharset(int i);
	void nukeCharset(int i);

	int _lastLoadedRoom;
public:
	const byte *findResourceData(uint32 tag, const byte *ptr);
	const byte *findResource(uint32 tag, const byte *ptr);
	int getResourceDataSize(const byte *ptr) const;
	void dumpResource(const char *tag, int index, const byte *ptr, int length = -1);

public:
	/* Should be in Object class */
	byte OF_OWNER_ROOM;
	int getInventorySlot();
	int findInventory(int owner, int index);
	int getInventoryCount(int owner);

protected:
	byte *_objectOwnerTable, *_objectRoomTable, *_objectStateTable;
	int _numObjectsInRoom;

public:
	uint32 *_classData;

protected:
	virtual void setupRoomObject(ObjectData *od, const byte *room, const byte *searchptr = NULL);
	void markObjectRectAsDirty(int obj);
	void loadFlObject(uint object, uint room);
	void nukeFlObjects(int min, int max);
	int findFlObjectSlot();
	int findLocalObjectSlot();
	void addObjectToInventory(uint obj, uint room);
	void updateObjectStates();
public:
	bool getClass(int obj, int cls) const;		// Used in actor.cpp, hence public
protected:
	void putClass(int obj, int cls, bool set);
	int getState(int obj);
	void putState(int obj, int state);
	void setObjectState(int obj, int state, int x, int y);
	int getOwner(int obj) const;
	void putOwner(int obj, int owner);
	void setOwnerOf(int obj, int owner);
	void clearOwnerOf(int obj);
	int getObjectRoom(int obj) const;
	int getObjX(int obj);
	int getObjY(int obj);
	void getObjectXYPos(int object, int &x, int &y)	{ int dir; getObjectXYPos(object, x, y, dir); }
	void getObjectXYPos(int object, int &x, int &y, int &dir);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int getObjectIndex(int object) const;
	int getObjectImageCount(int object);
	int whereIsObject(int object) const;
	int findObject(int x, int y);
	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);
public:
	int getObjectOrActorXY(int object, int &x, int &y);	// Used in actor.cpp, hence public
protected:
	int getObjActToObjActDist(int a, int b); // Not sure how to handle
	const byte *getObjOrActorName(int obj);		 // these three..
	void setObjectName(int obj);

	void addObjectToDrawQue(int object);
	void removeObjectFromDrawQue(int object);
	void clearDrawObjectQueue();
	void processDrawQue();

	virtual void clearDrawQueues();

	uint32 getOBCDOffs(int object) const;
	byte *getOBCDFromObject(int obj);
	const byte *getOBIMFromObjectData(const ObjectData &od);
	const byte *getObjectImage(const byte *ptr, int state);
	virtual int getObjectIdFromOBIM(const byte *obim);

	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);

protected:
	/* Should be in Verb class */
	uint16 _verbMouseOver;
	int _inventoryOffset;
	int8 _userPut;
	uint16 _userState;

	int _activeObject;

	virtual void handleMouseOver(bool updateInventory);
	virtual void redrawVerbs();
	virtual void checkExecVerbs();

	void verbMouseOver(int verb);
	int findVerbAtPos(int x, int y) const;
	virtual void drawVerb(int verb, int mode);
	void runInputScript(int a, int cmd, int mode);
	void restoreVerbBG(int verb);
	void drawVerbBitmap(int verb, int x, int y);
	int getVerbSlot(int id, int mode) const;
	void killVerb(int slot);
	void setVerbObject(uint room, uint object, uint verb);

public:
	/* Should be in Actor class */
	Actor *derefActor(int id, const char *errmsg = 0) const;
	Actor *derefActorSafe(int id, const char *errmsg) const;

	int getAngleFromPos(int x, int y) const;

protected:
	void walkActors();
	void playActorSounds();
	void redrawAllActors();
	void setActorRedrawFlags();
	void putActors();
	void showActors();
	void setupV1ActorTalkColor();
	void resetActorBgs();
	virtual void processActors();
	void processUpperActors();
	virtual int getActorFromPos(int x, int y);

public:
	/* Actor talking stuff */
	byte _actorToPrintStrFor, _V1TalkingActor;
	int _sentenceNum;
	SentenceTab _sentence[NUM_SENTENCE];
	StringTab _string[6];
	int16 _talkDelay;
	int _NES_lastTalkingActor;
	int _NES_talkColor;

	virtual void actorTalk(const byte *msg);
	void stopTalk();
	int getTalkingActor();		// Wrapper around VAR_TALK_ACTOR for V1 Maniac
	void setTalkingActor(int variable);

	// Generic costume code
	bool isCostumeInUse(int i) const;

	// Akos Class
	struct {
		int16 cmd;
		int16 actor;
		int16 param1;
		int16 param2;
	} _akosQueue[32];
	int16 _akosQueuePos;

	Common::Rect _actorClipOverride;

	bool akos_increaseAnims(const byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, const byte *aksq, const uint16 *akfo, int numakfo);
	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	virtual void akos_processQueue();

protected:
	/* Should be in Graphics class? */
	uint16 _screenB, _screenH;
public:
	int _roomHeight, _roomWidth;
	int _screenHeight, _screenWidth;
	VirtScreen virtscr[4];		// Virtual screen areas
	CameraData camera;			// 'Camera' - viewport

	int _screenStartStrip, _screenEndStrip;
	int _screenTop;

	Common::RenderMode _renderMode;

protected:
	ColorCycle _colorCycle[16];	// Palette cycles
	uint8 _colorUsedByCycle[256];

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs, _EPAL_offs;
	uint32 _IM00_offs, _PALS_offs;

	//ender: fullscreen
	bool _fullRedraw, _bgNeedsRedraw;
	bool _screenEffectFlag, _completeScreenRedraw;

	struct {
		int hotspotX, hotspotY, width, height;
		byte animate, animateIndex;
		int8 state;
	} _cursor;
	byte _grabbedCursor[8192];
	byte _currentCursor;

	byte _newEffect, _switchRoomEffect2, _switchRoomEffect;
	bool _doEffect;

	byte *_scrollBuffer;

	struct {
		int x, y, w, h;
		byte *buffer;
		uint16 xStrips, yStrips;
		bool isDrawn;
	} _flashlight;

public:
	bool isLightOn() const;

protected:
	void initScreens(int b, int h);
	void initVirtScreen(VirtScreenNumber slot, int top, int width, int height, bool twobufs, bool scrollable);
	void initBGBuffers(int height);
	void initCycl(const byte *ptr);	// Color cycle

	void decodeNESBaseTiles();

	void drawObject(int obj, int arg);
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void drawBox(int x, int y, int x2, int y2, int color);

	void restoreBG(Common::Rect rect, byte backcolor = 0);
	void redrawBGStrip(int start, int num);
	virtual void redrawBGAreas();

	void cameraMoved();
	void setCameraAtEx(int at);
	virtual void setCameraAt(int pos_x, int pos_y);
	virtual void setCameraFollows(Actor *a);
	virtual void moveCamera();
	virtual void panCameraTo(int x, int y);
	void clampCameraPos(Common::Point *pt);
	void actorFollowCamera(int act);

	const byte *getPalettePtr(int palindex, int room);
	void setupC64Palette();
	void setupNESPalette();
	void setupAmigaPalette();
	void setupHercPalette();
	void setupCGAPalette();
	void setupEGAPalette();
	void setupV1ManiacPalette();
	void setupV1ZakPalette();
	void setPalette(int pal);
	void setRoomPalette(int pal, int room);
	virtual void setPaletteFromPtr(const byte *ptr, int numcolor = -1);
	virtual void setPalColor(int index, int r, int g, int b);
	void setDirtyColors(int min, int max);
	const byte *findPalInPals(const byte *pal, int index);
	void swapPalColors(int a, int b);
	virtual void copyPalColor(int dst, int src);
	void cyclePalette();
	void stopCycle(int i);
	virtual void palManipulateInit(int resID, int start, int end, int time);
	void palManipulate();
public:
	int remapPaletteColor(int r, int g, int b, int threshold);		// Used by Actor::remapActorPalette
protected:
	void moveMemInPalRes(int start, int end, byte direction);
	void setupShadowPalette(int slot, int redScale, int greenScale, int blueScale, int startColor, int endColor);
	void setupShadowPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor, int start, int end);
	virtual void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor);

	void setupCursor();

	void setCursorFromBuffer(const byte *ptr, int width, int height, int pitch);

public:
	void markRectAsDirty(VirtScreenNumber virt, int left, int right, int top, int bottom, int dirtybit = 0);
	void markRectAsDirty(VirtScreenNumber virt, const Common::Rect& rect, int dirtybit = 0) {
		markRectAsDirty(virt, rect.left, rect.right, rect.top, rect.bottom, dirtybit);
	}
protected:
	// Screen rendering
	byte *_compositeBuf;
	byte *_herculesBuf;
	virtual void drawDirtyScreenParts();
	void updateDirtyScreen(VirtScreenNumber slot);
	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void ditherCGA(byte *dst, int dstPitch, int x, int y, int width, int height) const;
	void ditherHerc(byte *src, byte *hercbuf, int srcPitch, int *x, int *y, int *width, int *height) const;

public:
	VirtScreen *findVirtScreen(int y);
	byte *getMaskBuffer(int x, int y, int z);

protected:
	void drawFlashlight();

	void fadeIn(int effect);
	void fadeOut(int effect);

	void unkScreenEffect6();
	void transitionEffect(int a);
	void dissolveEffect(int width, int height);
	void scrollEffect(int dir);

	// bomp
public:
	byte *_bompActorPalettePtr;
	void drawBomp(const BompDrawData &bd, bool mirror);

protected:
	bool _shakeEnabled;
	uint _shakeFrame;
	void setShake(int mode);

	int _drawObjectQueNr;
	byte _drawObjectQue[200];

	/* For each of the 410 screen strips, gfxUsageBits contains a
	 * bitmask. The lower 80 bits each correspond to one actor and
	 * signify if any part of that actor is currently contained in
	 * that strip.
	 *
	 * If the leftmost bit is set, the strip (background) is dirty
	 * needs to be redrawn.
	 *
	 * The second leftmost bit is set by removeBlastObject() and
	 * restoreBG(), but I'm not yet sure why.
	 */
	uint32 gfxUsageBits[410 * 3];

	void upgradeGfxUsageBits();
	void setGfxUsageBit(int strip, int bit);
	void clearGfxUsageBit(int strip, int bit);
	bool testGfxUsageBit(int strip, int bit);
	bool testGfxAnyUsageBits(int strip);
	bool testGfxOtherUsageBits(int strip, int bit);

public:
	uint8 *_hePalettes;
	byte _HEV7ActorPalette[256];
	byte _roomPalette[256];
	byte *_shadowPalette;
	bool _skipDrawObject;
	int _timers[4];
	int _voiceMode;

protected:
	int _shadowPaletteSize;
	byte _currentPalette[3 * 256];
	byte _darkenPalette[3 * 256];

	int _palDirtyMin, _palDirtyMax;

	byte _palManipStart, _palManipEnd;
	uint16 _palManipCounter;
	byte *_palManipPalette;
	byte *_palManipIntermediatePal;

	byte _haveMsg;
	bool _useTalkAnims;
	uint16 _defaultTalkDelay;
	int _tempMusic;
	int _saveSound;
	bool _native_mt32;
	bool _enable_gs;
	int _midi;
	int _midiDriver; // Use the MD_ values from mididrv.h
	bool _copyProtection;
	bool _demoMode;
	bool _confirmExit;

public:
	uint16 _extraBoxFlags[65];

	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();
	int getPathToDestBox(byte from, byte to);
	void getGates(int trap1, int trap2, Common::Point gateA[2], Common::Point gateB[2]);
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	int getClosestPtOnBox(int box, int x, int y, int16& outX, int16& outY);
	int getSpecialBox(int param1, int param2);

	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);

	bool checkXYInBoxBounds(int box, int x, int y);
	uint distanceFromPt(int x, int y, int ptx, int pty);
	void getBoxCoordinates(int boxnum, BoxCoords *bc);
	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);

	int getScale(int box, int x, int y);
	int getScaleFromSlot(int slot, int x, int y);

protected:
	// Scaling slots/items
	struct ScaleSlot {
		int x1, y1, scale1;
		int x2, y2, scale2;
	};
	ScaleSlot _scaleSlots[20];
	void setScaleSlot(int slot, int x1, int y1, int scale1, int x2, int y2, int scale2);
	void setBoxScaleSlot(int box, int slot);
	void convertScaleTableToScaleSlot(int slot);

	void createBoxMatrix();
	bool areBoxesNeighbours(int i, int j);

	/* String class */
public:
	CharsetRenderer *_charset;
	byte _charsetColorMap[16];
protected:
	byte _charsetColor;
	byte _charsetData[15][16];

	int _charsetBufPos;
	byte _charsetBuffer[512];

	bool _keepText;

	virtual void initCharset(int charset);

	void printString(int m, const byte *msg);

	void CHARSET_1();
	void drawString(int a, const byte *msg);
	void debugMessage(const byte *msg);
	void showMessageDialog(const byte *msg);

	int convertMessageToString(const byte *msg, byte *dst, int dstSize);
	int convertIntMessage(byte *dst, int dstSize, int var);
	int convertVerbMessage(byte *dst, int dstSize, int var);
	int convertNameMessage(byte *dst, int dstSize, int var);
	int convertStringMessage(byte *dst, int dstSize, int var);

	virtual void loadLanguageBundle() {}

public:
	Common::Language _language;	// Accessed by a hack in NutRenderer::loadFont

	// Used by class ScummDialog:
	virtual void translateText(const byte *text, byte *trans_buff);

	// Somewhat hackish stuff for 2 byte support (Chinese/Japanese/Korean)
	bool _useCJKMode;
	int _2byteHeight;
	int _2byteWidth;
	byte *get2byteCharPtr(int idx);

protected:
	byte *_2byteFontPtr;

	uint32 fileReadDword();

public:

	/* Scumm Vars */
	byte VAR_LANGUAGE;
	byte VAR_KEYPRESS;
	byte VAR_SYNC;
	byte VAR_EGO;
	byte VAR_CAMERA_POS_X;
	byte VAR_HAVE_MSG;
	byte VAR_ROOM;
	byte VAR_OVERRIDE;
	byte VAR_MACHINE_SPEED;
	byte VAR_ME;
	byte VAR_NUM_ACTOR;
	byte VAR_CURRENT_LIGHTS;
	byte VAR_CURRENTDRIVE;
	byte VAR_CURRENTDISK;
	byte VAR_TMR_1;
	byte VAR_TMR_2;
	byte VAR_TMR_3;
	byte VAR_MUSIC_TIMER;
	byte VAR_ACTOR_RANGE_MIN;
	byte VAR_ACTOR_RANGE_MAX;
	byte VAR_CAMERA_MIN_X;
	byte VAR_CAMERA_MAX_X;
	byte VAR_TIMER_NEXT;
	byte VAR_VIRT_MOUSE_X;
	byte VAR_VIRT_MOUSE_Y;
	byte VAR_ROOM_RESOURCE;
	byte VAR_LAST_SOUND;
	byte VAR_CUTSCENEEXIT_KEY;
	byte VAR_OPTIONS_KEY;
	byte VAR_TALK_ACTOR;
	byte VAR_CAMERA_FAST_X;
	byte VAR_SCROLL_SCRIPT;
	byte VAR_ENTRY_SCRIPT;
	byte VAR_ENTRY_SCRIPT2;
	byte VAR_EXIT_SCRIPT;
	byte VAR_EXIT_SCRIPT2;
	byte VAR_VERB_SCRIPT;
	byte VAR_SENTENCE_SCRIPT;
	byte VAR_INVENTORY_SCRIPT;
	byte VAR_CUTSCENE_START_SCRIPT;
	byte VAR_CUTSCENE_END_SCRIPT;
	byte VAR_CHARINC;
	byte VAR_WALKTO_OBJ;
	byte VAR_DEBUGMODE;
	byte VAR_HEAPSPACE;
	byte VAR_RESTART_KEY;
	byte VAR_PAUSE_KEY;
	byte VAR_MOUSE_X;
	byte VAR_MOUSE_Y;
	byte VAR_TIMER;
	byte VAR_TMR_4;
	byte VAR_SOUNDCARD;
	byte VAR_VIDEOMODE;
	byte VAR_MAINMENU_KEY;
	byte VAR_FIXEDDISK;
	byte VAR_CURSORSTATE;
	byte VAR_USERPUT;
	byte VAR_SOUNDRESULT;
	byte VAR_TALKSTOP_KEY;
	byte VAR_FADE_DELAY;
	byte VAR_NOSUBTITLES;

	// V5+
	byte VAR_SOUNDPARAM;
	byte VAR_SOUNDPARAM2;
	byte VAR_SOUNDPARAM3;
	byte VAR_MOUSEPRESENT;
	byte VAR_MEMORY_PERFORMANCE;
	byte VAR_VIDEO_PERFORMANCE;
	byte VAR_ROOM_FLAG;
	byte VAR_GAME_LOADED;
	byte VAR_NEW_ROOM;

	// V4/V5
	byte VAR_V5_TALK_STRING_Y;

	// V6+
	byte VAR_ROOM_WIDTH;
	byte VAR_ROOM_HEIGHT;
	byte VAR_SUBTITLES;
	byte VAR_V6_EMSSPACE;

	// V7/V8 (=GF_NEW_CAMERA) specific variables
	byte VAR_CAMERA_POS_Y;
	byte VAR_CAMERA_MIN_Y;
	byte VAR_CAMERA_MAX_Y;
	byte VAR_CAMERA_THRESHOLD_X;
	byte VAR_CAMERA_THRESHOLD_Y;
	byte VAR_CAMERA_SPEED_X;
	byte VAR_CAMERA_SPEED_Y;
	byte VAR_CAMERA_ACCEL_X;
	byte VAR_CAMERA_ACCEL_Y;
	byte VAR_CAMERA_DEST_X;
	byte VAR_CAMERA_DEST_Y;
	byte VAR_CAMERA_FOLLOWED_ACTOR;

	// V7/V8 specific variables
	byte VAR_VERSION_KEY;
	byte VAR_DEFAULT_TALK_DELAY;
	byte VAR_CUSTOMSCALETABLE;
	byte VAR_BLAST_ABOVE_TEXT;
	byte VAR_VOICE_MODE;
	byte VAR_MUSIC_BUNDLE_LOADED;
	byte VAR_VOICE_BUNDLE_LOADED;

	byte VAR_LEFTBTN_DOWN;	// V7
	byte VAR_RIGHTBTN_DOWN;	// V7
	byte VAR_LEFTBTN_HOLD;	// V6/V72HE/V7
	byte VAR_RIGHTBTN_HOLD;	// V6/V72HE/V7
	byte VAR_MOUSE_BUTTONS;	// V8
	byte VAR_MOUSE_HOLD;	// V8
	byte VAR_SAVELOAD_SCRIPT;	// V6/V7 (not HE)
	byte VAR_SAVELOAD_SCRIPT2;	// V6/V7 (not HE)

	// V6/V7 specific variables (actually, they are only used in FT and Sam, it seems?)
	byte VAR_CHARSET_MASK;

	// V6 specific variables
	byte VAR_V6_SOUNDMODE;

	// V1/V2 specific variables
	byte VAR_CHARCOUNT;
	byte VAR_VERB_ALLOWED;
	byte VAR_ACTIVE_VERB;
	byte VAR_ACTIVE_OBJECT1;
	byte VAR_ACTIVE_OBJECT2;
	byte VAR_CLICK_AREA;

	// HE specific variables
	byte VAR_SKIP_RESET_TALK_ACTOR;
	byte VAR_MUSIC_CHANNEL;
	byte VAR_SOUND_CHANNEL;

	byte VAR_SCRIPT_CYCLE;
	byte VAR_NUM_SCRIPT_CYCLES;

	byte VAR_KEY_STATE;		// Used in parseEvents()
	byte VAR_MOUSE_STATE;

	// Exists both in V7 and in V72HE:
	byte VAR_NUM_GLOBAL_OBJS;
};

} // End of namespace Scumm

#endif
