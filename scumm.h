/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2002 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SCUMM_H
#define SCUMM_H

#include "scummsys.h"
#include "system.h"
#include "sound/mixer.h"

#define SCUMMVM_VERSION "0.2.2 CVS"
#define SCUMMVM_CVS "070802"

#define SWAP(a,b) do{int tmp=a; a=b; b=tmp; } while(0)
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

class GameDetector;
class Gui;
class NewGui;
class Scumm;
class IMuse;
class Actor;
struct ScummDebugger;
struct Serializer;

typedef void (Scumm::*OpcodeProc)();

/* Use this one from error() ONLY */
extern Scumm *g_scumm;
/* BIG HACK for MidiEmu - FIXME */
extern OSystem *g_system;
extern SoundMixer *g_mixer;

/* System Wide Constants */
enum {
#ifdef _WIN32_WCE
	SAMPLES_PER_SEC =  11025,
#else
	SAMPLES_PER_SEC =  22050,
#endif
	BITS_PER_SAMPLE = 16,
	NUM_MIXER = 4,
	NUM_SCRIPT_SLOT = 40,
	NUM_LOCALSCRIPT = 60,
	NUM_SHADOW_PALETTE = 8,
    MAX_ACTORS = 30,
    KEY_SET_OPTIONS = 3456 // WinCE
};

/* Script status type (slot.status) */
enum {
	ssDead = 0,
	ssPaused = 1,
	ssRunning = 2
};

struct ScummPoint {
	int x, y;
};

struct MemBlkHeader {
	uint32 size;
};

#if !defined(__GNUC__)
	#pragma START_PACK_STRUCTS
#endif	

struct ResHdr {
	uint32 tag, size;
} GCC_PACK;

#define RES_DATA(x) (((byte*)x) + sizeof(ResHdr))
#define RES_SIZE(x) ( READ_BE_UINT32_UNALIGNED(&((ResHdr*)x)->size) )


struct RoomHeader {
	union {
		struct {
			uint32 version;
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK v7;
		struct {
			uint16 width, height;
			uint16 numObjects;
		} GCC_PACK old;
	} GCC_PACK;
} GCC_PACK;

struct CodeHeader {
	union {
		struct {
			uint16 obj_id;
			byte x, y, w, h;
			byte flags;
			byte parent;
			int16 walk_x;
			int16 walk_y;
			byte actordir;
		} GCC_PACK v5;

		struct {
			uint16 obj_id;
			int16 x, y;
			uint16 w, h;
			byte flags, parent;
			uint16 unk1;
			uint16 unk2;
			byte actordir;
		} GCC_PACK v6;

		struct {
			uint32 version;
			uint16 obj_id;
			byte parent;
			byte parentstate;
		} GCC_PACK v7;
							
	} GCC_PACK;
} GCC_PACK;

struct ImageHeader { /* file format */
	union {
		struct {
			uint16 obj_id;
			uint16 unk[5];
			uint16 width;
			uint16 height;
			uint16 unk_2;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK old;

		struct {
			uint32 version;
			uint16 obj_id;
			uint16 unk[1];
			int16 x_pos, y_pos;
			uint16 width, height;
			byte unk2[3];
			byte actordir;
			uint16 unk_2;
			struct {
				int16 x, y;
			} GCC_PACK hotspot[15];
		} GCC_PACK v7;
	} GCC_PACK;
} GCC_PACK;

#if !defined(__GNUC__)
	#pragma END_PACK_STRUCTS
#endif

struct VerbSlot {
	int16 x, y;
	int16 right, bottom;
	int16 oldleft, oldtop, oldright, oldbottom;
	uint8 verbid;
	uint8 color, hicolor, dimcolor, bkcolor, type;
	uint8 charset_nr, curmode;
	uint8 saveid;
	uint8 key;
	bool center;
	uint8 field_1B;
	uint16 imgindex;
};

class ObjectData {
public:
	uint32 offs_obim_to_room;
	uint32 offs_obcd_to_room;
	int16 walk_x, walk_y;
	uint16 obj_nr;
	int16 x_pos;
	int16 y_pos;
	uint16 width;
	uint16 height;
	byte actordir;
	byte parent;
	byte parentstate;
	byte state;
	byte fl_object_index;
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	uint16 newfield;
	byte status;
	byte where;
	byte unk1, unk2, freezeCount, didexec;
	byte cutsceneOverride;
	byte unk5;
};

struct NestedScript {
	uint16 number;
	uint8 where;
	uint8 slot;
};
 
enum {
	sleByte = 1,
	sleUint8 = 1,
	sleInt8 = 1,
	sleInt16 = 2,
	sleUint16 = 3,
	sleInt32 = 4,
	sleUint32 = 5
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
	rtLast = 17,
	rtNumTypes = 18
};

enum {
	LIGHTMODE_dark			= 0,
	LIGHTMODE_actor_base	= 1,
	LIGHTMODE_screen		= 2,
	LIGHTMODE_flashlight	= 4,
	LIGHTMODE_actor_color	= 8
};

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,
	
	OF_STATE_SHL = 4
};

enum {
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK | MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
};

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE
};

#define _maxRooms res.num[rtRoom]
#define _maxScripts res.num[rtScript]
#define _maxCostumes res.num[rtCostume]
#define _maxInventoryItems res.num[rtInventory]
#define _maxCharsets res.num[rtCharset]
#define _maxStrings res.num[rtString]
#define _maxVerbs res.num[rtVerb]
#define _maxActorNames res.num[rtActorName]
#define _maxBuffer res.num[rtBuffer]
#define _maxScaleTable res.num[rtScaleTable]
#define _maxTemp res.num[rtTemp]
#define _maxFLObject res.num[rtFlObject]
#define _maxMatrixes res.num[rtMatrix]
#define _maxBoxes res.num[rtBox]

#define _baseRooms res.address[rtRoom]
#define _baseScripts res.address[rtScript]
#define _baseInventoryItems res.address[rtInventory]
#define _baseFLObject res.address[rtFlObject]
#define _baseArrays res.address[rtString]

#define _roomFileOffsets res.roomoffs[rtRoom]

struct CharsetRenderer {
	Scumm *_vm;
	int _top;
	int _drawTop;
	int _left, _left2;
	byte _center;
	int _right;
	byte _color;
	bool _hasMask;
	bool _blitAlso;
	
	int _strLeft, _strRight, _strTop, _strBottom;
	byte _curId;

	int _xpos2, _ypos2;
	
	int _bufPos;
	byte _unk12, _disableOffsX;
	byte *_ptr;
	byte _unk2, _bpp;
	byte _invNumBits;
	uint32 _charOffs;
	byte *_charPtr;
	int _width, _height;
	int _offsX, _offsY;
	byte _bitMask, _revBitMask;
	int _bottom;
	int _virtScreenHeight;

	byte _ignoreCharsetMask;

	byte *_backbuff_ptr, *_bgbak_ptr;
	byte *_mask_ptr;
	byte *_dest_ptr;
		
	byte _colorMap[16];
	byte _buffer[512];

	void drawBits();
	void printChar(int chr);
	void printCharOld(int chr);
	int getSpacing(char chr);
	int getStringWidth(int a, byte *str, int pos);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
};

struct LoadedCostume {
	byte *_ptr;
	byte *_dataptr;
	byte _numColors;
	
};

struct CostumeRenderer {
	Scumm *_vm;
	
	LoadedCostume _loaded;
		
	byte *_shadow_table;

	byte *_frameptr;
	byte *_srcptr;
	byte *_bgbak_ptr, *_backbuff_ptr, *_mask_ptr, *_mask_ptr_dest;
	int _actorX, _actorY;
	byte _zbuf;
	uint _scaleX, _scaleY;
	int _xmove, _ymove;
	bool _mirror;
	byte _maskval;
	byte _shrval;
	byte _width2;
	int _width;
	byte _height2;
	int _height;
	int _xpos, _ypos;

	uint _outheight;
	int _scaleIndexXStep;
	int _scaleIndexYStep;
	byte _scaleIndexX;						/* must wrap at 256 */
	byte _scaleIndexY, _scaleIndexYTop;
	int _left, _right;
	int _dir2;
	int _top, _bottom;
	int _ypostop;
	int _ypitch;
	byte _docontinue;
	int _imgbufoffs;
	byte _repcolor;
	byte _replen;
	byte _palette[32];
	byte _transEffect[0x100];

	void proc6();
	void proc5();
	void proc4();
	void proc3();
	void proc2();
	void proc1();
	void proc_special(Actor *a, byte mask);
	byte mainRoutine(Actor *a, int slot, int frame);
	void ignorePakCols(int num);

	byte drawOneSlot(Actor *a, int slot);
	byte drawCostume(Actor *a);

	void setPalette(byte *palette);
	void setFacing(uint16 facing);
	void setCostume(int costume);
};

#define ARRAY_HDR_SIZE 6
struct ArrayHeader {
	int16 dim1_size;
	int16 type;
	int16 dim2_size;
	byte data[1];
};

struct SentenceTab {
	byte unk5;
	byte unk2;
	uint16 unk4;
	uint16 unk3;
	byte unk;
	byte pad;
};

struct StringTab {
	int16 t_xpos, t_ypos;
	int16 t_right;
	int16 xpos, ypos;
	int16 right;
	byte color, t_color;
	byte charset, t_charset;
	bool center, t_center;
	bool overhead, t_overhead;
	bool no_talk_anim, t_no_talk_anim;
};

enum GameId {
	GID_TENTACLE = 1,
	GID_MONKEY2 = 2,
	GID_INDY4 = 3,
	GID_MONKEY = 4,
	GID_SAMNMAX = 5,
	GID_MONKEY_EGA = 6,
	GID_LOOM256 = 7,
	GID_ZAK256 = 8,
	GID_INDY3_256 = 9,
	GID_LOOM = 10,
	GID_FT = 11,
	GID_DIG = 12,
	GID_MONKEY_VGA = 13,
	GID_CMI = 14,

	/* Simon the Sorcerer */
	GID_SIMON_FIRST = 20,
	GID_SIMON_LAST = GID_SIMON_FIRST + 3,
};

enum GameFeatures {
	GF_NEW_OPCODES = 1,
	GF_AFTER_V6 = 2,
	GF_AFTER_V7 = 4,
	GF_HAS_ROOMTABLE = GF_AFTER_V7,
	GF_USE_KEY = 8,
	GF_NEW_COSTUMES = GF_AFTER_V7,
	GF_USE_ANGLES = GF_AFTER_V7,
	GF_DRAWOBJ_OTHER_ORDER = 16,

	GF_DEFAULT = GF_USE_KEY,

	GF_SMALL_HEADER = 32,
    GF_EXTERNAL_CHARSET = GF_SMALL_HEADER,
    GF_SMALL_NAMES = 64,
    GF_OLD_BUNDLE = 128,
	GF_16COLOR = 256,
	GF_OLD256 = 512,
	GF_AUDIOTRACKS = 1024,
	GF_NO_SCALLING = 2048,
	GF_ADLIB_DEFAULT = 4096,
	GF_AMIGA = 8192,
	GF_HUMONGOUS = 16384,
	GF_AFTER_V8 = 32768
};

enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4
};

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2
};

#include "gfx.h"
#include "boxes.h"
#include "smush.h"

class Scumm {
public:
	/* Put often used variables at the top.
	 * That results in a shorter form of the opcode
	 * on some architectures. */
	OSystem *_system;
	IMuse *_imuse;
	Gui *_gui;
	NewGui *_newgui;
	uint32 _features;
	VerbSlot *_verbs;
	ObjectData *_objs;
	ScummDebugger *_debugger;

	struct {
		byte mode[rtNumTypes];
		uint16 num[rtNumTypes];
		uint32 tags[rtNumTypes];
		const char *name[rtNumTypes];
		byte **address[rtNumTypes];
		byte *flags[rtNumTypes];
		byte *roomno[rtNumTypes];
		uint32 *roomoffs[rtNumTypes];
	} res;

	struct {
		uint32 cutScenePtr[5];
		byte cutSceneScript[5];
		int16 cutSceneData[5];
		int16 cutSceneScriptIndex;
		byte cutSceneStackPointer;
		ScriptSlot slot[NUM_SCRIPT_SLOT];
		NestedScript nest[15];
		int16 localvar[NUM_SCRIPT_SLOT][17];
	} vm;

	struct {
		int16 x, y;
	} mouse;

	/* Init functions, etc */
	byte _fastMode;
	char *getGameName();
	Scumm(); // constructor
	virtual ~Scumm();

	/* video buffer */
	byte *_videoBuffer;

	/* system call object */

	/* Scumm main loop */

	void mainRun();

	/* _insane vars */

	int _insaneFlag;
	int _insaneState;
	int videoFinished;
	
	
	void scummInit();
	void scummMain(int argc, char **argv); // is it still used ?
	int scummLoop(int delta);
	void initScummVars();

	void pauseGame(bool user);
	void shutDown(int i);
	void setOptions(void);

	void main();
	void parseCommandLine(int argc, char **argv);
	void showHelpAndExit();
	bool detectGame();
	void processKbd();
	void clear_fullRedraw();

	int checkKeyHit();
	void convertKeysToClicks();

	/* Random number generation */
	uint32 _randSeed1, _randSeed2;
	void initRandSeeds();
	uint getRandomNumber(uint max);
	uint getRandomNumberRng(uint min, uint max);

	/* Core variable definitions */
	byte _gameId;
	const char *_gameText;
	char *_gameDataPath;

	/* Core class/array definitions */
	Gdi gdi;

	Actor *actor;	// Has MAX_ACTORS elements, see init.cpp
	
	uint16 *_inventory;
	byte *_arrays;
	uint16 *_newNames;
	int16 *_vars;
	int16 _varwatch;
	byte *_bitVars;

	/* Global resource tables */
	int _numVariables, _numBitVariables, _numLocalObjects;
	int _numGlobalObjects, _numArray, _numVerbs, _numFlObject;
	int _numInventory, _numRooms, _numScripts, _numSounds;
	int _numCharsets, _numCostumes, _numNewNames, _numGlobalScripts;
	int NUM_ACTORS;

	/* Current objects - can go in their respective classes */
	byte _curActor;
	int _curVerb;
	int _curVerbSlot;
	int _curPalIndex;
	byte _currentRoom;
	VirtScreen *_curVirtScreen;

	bool _egoPositioned;
	int _xPos, _yPos, _dir;
	int _keyPressed;
	uint16 _lastKeyHit;
	uint16 _mouseButStat;
	byte _leftBtnPressed, _rightBtnPressed;

	int16 _virtual_mouse_x, _virtual_mouse_y, _bootParam;
	uint16 _debugMode, _currentDrive, _soundCardType;
	byte _mousePresent;

	/* Not sure where this stuff goes */
	byte isMaskActiveAt(int l, int t, int r, int b, byte *mem);
	void startScene(int room, Actor *a, int b);
	void setupScummVarsOld();	// Both of these will simply be one
	void setupScummVarsNew();	// 'setupScummVars' in each Scumm_Vx
	byte *_objectOwnerTable, *_objectRoomTable, *_objectStateTable;
	byte _numObjectsInRoom;
	int8 _userPut;
	int _resourceHeaderSize;
	void unkRoomFunc3(int a, int b, int c, int d, int e);
	void palManipulate(int a, int b, int c, int d, int e);
	void setScaleItem(int slot, int a, int b, int c, int d);
	void clearClickedStatus();
	void startManiac();

	/* GUI class */
	void drawString(int a);
	int getKeyInput(int a);

	/* Save/Load class - some of this may be GUI */
	byte _saveLoadFlag, _saveLoadSlot;
	bool _doAutosave;
	bool _saveLoadCompatible;
	char _saveLoadName[32];

	bool saveState(int slot, bool compat);
	bool loadState(int slot, bool compat);
	void saveOrLoad(Serializer *s);

	bool getSavegameName(int slot, char *desc);
	void makeSavegameName(char *out, int slot, bool compatible);
	void saveLoadResource(Serializer *ser, int type, int index);

	/* Heap and memory management */
	uint32 _maxHeapThreshold, _minHeapThreshold;
	void checkRange(int max, int min, int no, const char *str);
	void lock(int type, int i);
	void unlock(int type, int i);
	void heapClear(int mode);
	void unkHeapProc2(int a, int b);

	/* Script VM - should be in Script class */
	uint32 _localScriptList[NUM_LOCALSCRIPT];
	byte *_scriptPointer, *_scriptOrgPointer, *_scriptPointerStart;
	byte _opcode, _numNestedScripts, _currentScript;
	uint16 _curExecScript;
	const OpcodeProc *_opcodes;
	const char **_opcodes_lookup;
	byte **_lastCodePtr;
	int _resultVarNumber, _scummStackPos;
	int16 _localParamList[16],  _scummStack[150];
	
	OpcodeProc getOpcode(int i) { return _opcodes[i]; }
	void initializeLocals(int slot, int16 *vars);
	int	getScriptSlot();
	void runScript(int script, int a, int b, int16 *lvarptr);
	void stopScriptNr(int script);
	void runScriptNested(int script);
	void executeScript();	
	void updateScriptPtr();
	void getScriptBaseAddress();
	void getScriptEntryPoint();
	byte fetchScriptByte();
	int fetchScriptWord();
	void ignoreScriptWord() { fetchScriptWord(); }
	void ignoreScriptByte() { fetchScriptByte(); }
	int getVarOrDirectWord(byte mask);
	int getVarOrDirectByte(byte mask);
	void getResultPos();
	void setResult(int result);
	int readVar(uint var);
	void writeVar(uint var, int value);
	void push(int a);
	int pop();
	void runHook(int i);
	bool isScriptInUse(int script);
	int getStringLen(byte *ptr);
	
	void freezeScripts(int scr);
	void unfreezeScripts();
	void runAllScripts();
	void setupOpcodes();
	void setupOpcodes2();	
	void cutscene(int16 *args);
	void endCutscene();
	void exitCutscene();
	void runExitScript();
	void runEntryScript();

	void beginOverride();
	void endOverride();
	void killScriptsAndResources();
	void checkAndRunVar33();
	void decreaseScriptDelay(int amount);
	bool isScriptRunning(int script);
	bool isRoomScriptRunning(int script);
	void arrayop_1(int a, byte *ptr);
	void copyString(byte *dst, byte *src, int len);
	void doSentence(int c, int b, int a);
	int popRoomAndObj(int *room);
	int getWordVararg(int16 *ptr);
	void decodeParseString();
	void decodeParseString2(int a, int b);

	/* Script VM or Object class? */
	void stopObjectCode();
	void stopObjectScript(int script);

	/* Should be in Resource class */
	byte _encbyte;
	void *_fileHandle;
	char *_resFilePrefix, *_resFilePath;
	uint32 _fileOffset;
	char *_exe_name;
	bool _dynamicRoomOffsets;
	byte _resourceMapper[128];
	uint32 _allocatedSize;
	byte _expire_counter;

	void allocateArrays();
	void openRoom(int room);
	void deleteRoomOffsets();
	void readRoomsOffsets();
	void askForDisk(const char *filename);
	bool openResourceFile(const char *filename);
	void loadPtrToResource(int type, int i, byte *ptr);
	void readResTypeList(int id, uint32 tag, const char *name);
	char *resTypeFromId(int id);
	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);
	byte *createResource(int type, int index, uint32 size);
	void nukeResource(int type, int i);	
	byte *getResourceAddress(int type, int i);
	byte *getStringAddress(int i);
	byte *getStringAddressVar(int i);
	void ensureResourceLoaded(int type, int i);
	int loadResource(int type, int i);
	int getResourceRoomNr(int type, int index);
	int readSoundResource(int type, int index);
	void setResourceCounter(int type, int index, byte flag);
	void validateResource(const char *str, int type, int index);
	void increaseResourceCounter();
	bool isResourceInUse(int type, int i);
	bool isResourceLoaded(int type, int index);
	void initRoomSubBlocks();
	void loadRoomObjects();
    void loadRoomObjectsSmall();
	void readArrayFromIndexFile();
	void readMAXS();
	bool isGlobInMemory(int type, int index);
	virtual void readIndexFile();
	virtual void loadCharset(int i);
	void nukeCharset(int i);

	bool fileReadFailed(void *handle);
	void clearFileReadFailed(void *handle);

	int _lastLoadedRoom, _roomResource;
	byte _resFilePathId, _fileReadFailed;
	byte *findResourceData(uint32 tag, byte *ptr);
	int getResourceDataSize(byte *ptr);
	
	int getArrayId();
	void nukeArray(int a);
	int defineArray(int a, int b, int c, int d);
	int readArray(int array, int index, int base);
	void writeArray(int array, int index, int base, int value);

	void resourceStats();
	void expireResources(uint32 size);
	void freeResources();
	void destroy();
	void dumpResource(char *tag, int index, byte *ptr);


	/* Should be in Object class */
	byte OF_OWNER_ROOM;
	struct FindObjectInRoom {
		CodeHeader *cdhd;
		byte *obcd;
		ImageHeader *imhd;
		byte *obim;
		byte *roomptr;
	};

	enum FindObjectWhat {
		foCodeHeader = 1,
		foImageHeader = 2,
		foCheckAlreadyLoaded = 4
	};
	int getInventorySlot();
	void SamInventoryHack(int obj);	// FIXME: Sam and Max hack
	int findInventory(int owner, int index);
	int getInventoryCount(int owner);

	void setupRoomObject(ObjectData *od, byte *room);
	void removeObjectFromRoom(int obj);
	void loadFlObject(uint object, uint room);
	void nukeFlObjects(int min, int max);
	int findFlObjectSlot();
	void addObjectToInventory(uint obj, uint room);
	void fixObjectFlags();
	bool getClass(int obj, int cls);
	void putClass(int obj, int cls, bool set);
	int getState(int obj);
	void putState(int obj, int state);
	void setObjectState(int obj, int state, int x, int y);
	int getOwner(int obj);
	void putOwner(int obj, int owner);
	void setOwnerOf(int obj, int owner);
	void clearOwnerOf(int obj);
	int getObjectRoom(int obj);
	int getObjX(int obj);
	int getObjY(int obj);
	void getObjectXYPos(int object);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int getObjectIndex(int object);
	int whereIsObject(int object);
	int findObject(int x, int y);
	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);	
	int getObjectOrActorXY(int object);		 // Object and Actor...
	int getObjActToObjActDist(int a, int b); // Not sure how to handle
	byte *getObjOrActorName(int obj);		 // these three..

	void addObjectToDrawQue(int object);
	void clearDrawObjectQueue();
	void processDrawQue();

	uint32 getOBCDOffs(int object);
	byte *getOBCDFromObject(int obj);	
	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);

	/* Should be in Costume class */
	void loadCostume(LoadedCostume *lc, int costume);
	byte cost_increaseAnims(LoadedCostume *lc, Actor *a);
	byte cost_increaseAnim(LoadedCostume *lc, Actor *a, int slot);
	void cost_decodeData(Actor *a, int frame, uint usemask);
	int cost_frameToAnim(Actor *a, int frame);


	/* Should be in Verb class */
	uint16 _verbMouseOver;
	int _inventoryOffset;	
	void redrawVerbs();
	void checkExecVerbs();
	void verbMouseOver(int verb);
	int checkMouseOver(int x, int y);
	void drawVerb(int verb, int mode);
	void runInputScript(int a, int cmd, int mode);
	void restoreVerbBG(int verb);
	void drawVerbBitmap(int vrb, int x, int y);
	int getVerbEntrypoint(int obj, int entry);
	int getVerbSlot(int id, int mode);
	void killVerb(int slot);
	void runVerbCode(int script, int entry, int a, int b, int16 *vars);
	void setVerbObject(uint room, uint object, uint verb);

	/* Should be in Sound class */
	union {
		SoundMixer _mixer[1];
		uint32 xxxx_1;
	};
	//SoundMixer _mixer[1];

//	MixerChannel _mixer_channel[NUM_MIXER];
	byte _sfxMode;
	bool _use_adlib;
	int16 _sound_volume_master, _sound_volume_music, _sound_volume_sfx;
	int _saveSound;
	void *_sfxFile;
	uint16 _soundParam, _soundParam2, _soundParam3;
	uint32 _talk_sound_a, _talk_sound_b;
	byte _talk_sound_mode;
	bool _mouthSyncMode;
	bool _endOfMouthSync;
	uint16 _mouthSyncTimes[52];
	uint _curSoundPos;
	int current_cd_sound, _cd_loops, _cd_frame, _cd_track, _cd_end;

	int tempMusic;

#ifdef COMPRESSED_SOUND_FILE

	#define CACHE_TRACKS 10

	/* used for mp3 CD music */

	int _current_cache;
	int _cached_tracks[CACHE_TRACKS];
	struct mad_header _mad_header[CACHE_TRACKS];
	long _mp3_size[CACHE_TRACKS];
	FILE *_mp3_tracks[CACHE_TRACKS];
	int _mp3_index;
	bool _mp3_cd_playing;

	int getCachedTrack(int track);
	int playMP3CDTrack(int track, int num_loops, int start, int delay);
	int stopMP3CD();
	int updateMP3CD();
	int pollMP3CD();
#endif

	int16 _soundQuePos, _soundQue[0x100];
	byte _soundQue2Pos, _soundQue2[10];
	bool _soundsPaused, _soundsPaused2;
	bool _soundVolumePreset;

	void setupSound();
	void processSoundQues();
	void playSound(int sound);
	void stopAllSounds();
	void stopSound(int sound);
	bool isSoundInQueue(int sound);
	void clearSoundQue();
	void talkSound(uint32 a, uint32 b, int mode);
	void processSfxQueues();
	int startTalkSound(uint32 a, uint32 b, int mode);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	int startSfxSound(void *file, int size);
	void *openSfxFile();
	void addSoundToQueue(int sound);
	void addSoundToQueue2(int sound);
	void soundKludge(int16 *list);
	MP3OffsetTable *offset_table;	// SO3 MP3 compressed audio
	int num_sound_effects;		// SO3 MP3 compressed audio

	BundleAudioTable *bundle_table; // DIG/CMI bundles
	void pauseSounds(bool pause);
	bool isSfxFinished();
	void playBundleSound(char *sound);
	void decompressBundleSound(int index);
	int playSfxSound(void *sound, uint32 size, uint rate);
 	int playSfxSound_MP3(void *sound, uint32 size);
	void stopSfxSound();

	int _talkChannel;	/* Mixer channel actor is talking on */
	bool _useTalkAnims;
	uint16 _defaultTalkDelay;
	byte _haveMsg;
	int isSoundRunning(int a);


	/* Should be in Actor class */
	Actor *derefActor(int id);
	Actor *derefActorSafe(int id, const char *errmsg);
	Actor *getFirstActor() {return actor;} 
	void putActor(Actor *a, int x, int y, byte room);
	void showActors();

	uint32 *_classData;

	static int newDirToOldDir(int dir);
	static int oldDirToNewDir(int dir);

	static int normalizeAngle(int angle);
	int getAngleFromPos(int x, int y);
	static int fromSimpleDir(int dirtype, int dir);
	static int toSimpleDir(int dirtype, int dir);
	static int numSimpleDirDirections(int dirType);
	void startAnimActorEx(Actor *a, int frame, int direction);
	int getProgrDirChange(Actor *a, int mode);

	int getActorXYPos(Actor *a);
	void walkActors();
	void playActorSounds();
	void setActorRedrawFlags();
	void resetActorBgs();
	void processActors();
	int getActorFromPos(int x, int y);
	void faceActorToObj(int act, int obj);
	void animateActor(int act, int anim);
	void actorFollowCamera(int act);
	
	bool isCostumeInUse(int i);

	/* Actor talking stuff */
	byte _actorToPrintStrFor;
	int _sentenceNum;
	SentenceTab sentence[6];
	StringTab string[6];
	void actorTalk();
	void stopTalk();	

	/* Akos Class */

	bool akos_increaseAnims(byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, byte *aksq, uint16 *akfo, int numakfo);

	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	bool akos_compare(int a, int b, byte cmd);
	void akos_decodeData(Actor *a, int frame, uint usemask);
	int akos_frameToAnim(Actor *a, int frame);
	bool akos_hasManyDirections(Actor *a);


	/* Should be in Graphics class? */
	uint16 _screenB, _screenH;
	int _scrHeight, _scrWidth, _realHeight, _realWidth;
	VirtScreen virtscr[4];		// Virtual screen areas
	CameraData camera;			// 'Camera' - viewport
	ColorCycle _colorCycle[16];	// Palette cycles

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs, _EPAL_offs;
	uint32 _IM00_offs, _PALS_offs;

	//ender: fullscreen
	bool _fullRedraw, _BgNeedsRedraw, _shakeEnabled;
	bool _screenEffectFlag, _completeScreenRedraw;

	int _cursorHotspotX, _cursorHotspotY, _cursorWidth, _cursorHeight;
	byte _cursorAnimate, _cursorAnimateIndex, _grabbedCursor[2048];
	int8 _cursorState;

	byte _newEffect, _switchRoomEffect2, _switchRoomEffect;
	bool _doEffect;

	void getGraphicsPerformance();
	void initScreens(int a, int b, int w, int h);
	void initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs, bool fourextra);
	void initBGBuffers(int height);
	void initCycl(byte *ptr);	// Color cycle

	void createSpecialPalette(int16 a, int16 b, int16 c, int16 d, int16 e, int16 colorMin, int16 colorMax);

	void drawObject(int obj, int arg);	
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void drawBox(int x, int y, int x2, int y2, int color);
	void drawBomp(BompDrawData *bd, int param1, byte *dataPtr, int param2, int param3);

	void restoreBG(int left, int top, int right, int bottom);
	void redrawBGStrip(int start, int num);	
	void redrawBGAreas();	
	
	void moveCamera();
	void cameraMoved();
	void setCameraAtEx(int at);
	void setCameraAt(int pos_x, int pos_y);
	void panCameraTo(int x, int y);
	void setCameraFollows(Actor *a);
	void clampCameraPos(ScummPoint *pt);

	byte *getPalettePtr();
	void setPalette(int pal);
	void setPaletteFromPtr(byte *ptr);
	void setPaletteFromRes();
	void setPalColor(int index, int r, int g, int b);
	void setDirtyColors(int min, int max);
	byte *findPalInPals(byte *pal, int index);
	void swapPalColors(int a, int b);
	void cyclePalette();
	void stopCycle(int i);
	void palManipulate();
	int remapPaletteColor(int r, int g, int b, uint threshold);
	void moveMemInPalRes(int start, int end, byte direction);
	void setupShadowPalette(int slot, int rfact, int gfact, int bfact, int from, int to);
	void darkenPalette(int a, int b, int c, int d, int e);

	void setShake(int mode);

	void setCursor(int cursor);
	void setCursorImg(uint img, uint room, uint imgindex);
	void setCursorHotspot2(int x, int y);
	void grabCursor(int x, int y, int w, int h);
	void grabCursor(byte *ptr, int width, int height);
	void makeCursorColorTransparent(int a);
	void setupCursor() { _cursorAnimate = 1; }
	void decompressDefaultCursor(int index);
	void useIm01Cursor(byte *im, int w, int h);
	void useBompCursor(byte *im, int w, int h);


	void updateDirtyRect(int virt, int left, int right, int top, int bottom, uint32 dirtybits);
	void setDirtyRange(int slot, int a, int height);
	void drawDirtyScreenParts();
	void updateDirtyScreen(int slot);

	VirtScreen *findVirtScreen(int y);
	void fadeOut(int a);
	static void setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom);

	void fadeIn(int effect);
	void unkScreenEffect1();
	void unkScreenEffect2();
	void unkScreenEffect3();
	void unkScreenEffect4();
	void unkScreenEffect5(int a);
	void unkScreenEffect6();
	void transitionEffect(int a);		// former unkScreenEffect7

	void decompressBomp(byte *dst, byte *src, int w, int h);
	uint _shakeFrame;
	int _screenStartStrip, _screenEndStrip;
	int _screenLeft, _screenTop;
	uint16 _enqueue_b, _enqueue_c, _enqueue_d, _enqueue_e;
	int _enqueuePos; 
	BlastObject _enqueuedObjects[32];

	void enqueueObject(int a, int b, int c, int d, int e, int f, int g, int h, int mode);
	void clearEnqueue() { _enqueuePos = 0; }
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);
	void removeBlastObjects();
	void removeBlastObject(BlastObject *eo);

	int _drawObjectQueNr;
	byte _drawObjectQue[200];
	int16 _palManipStart, _palManipEnd, _palManipCounter;
	uint32 gfxUsageBits[200];
	byte *_shadowPalette;
	int _shadowPaletteSize;
	byte _currentPalette[0x300];

	byte _proc_special_palette[256];
	int _palDirtyMin, _palDirtyMax;
	byte _bkColor;
	uint16 _lastXstart;

	



	/* Walkbox / Navigation class */
	int _maxBoxVertexHeap, _boxPathVertexHeapIndex, _boxMatrixItem;
	byte *_boxMatrixPtr4, *_boxMatrixPtr1, *_boxMatrixPtr3;	

	ScummPoint gateLoc[5];	/* Gate locations */
	int gate1ax, gate1ay, gate1bx, gate1by, gate2ax, gate2ay, gate2bx, gate2by;
	uint16 _extraBoxFlags[65];
	int16 _foundPathX, _foundPathY;
	int CloX[8], CloY[8];

	PathVertex *unkMatrixProc1(PathVertex *vtx, PathNode *node);
	PathNode *unkMatrixProc2(PathVertex *vtx, int i);
	bool areBoxesNeighbours(int i, int j);
	void addToBoxMatrix(byte b);
	bool compareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3);
	void SetGate(int line1, int line2, int polyx[8], int polyy[8]);
	void *addToBoxVertexHeap(int size);
	PathVertex *addPathVertex();
	bool checkXYInBoxBounds(int box, int x, int y);
	uint distanceFromPt(int x, int y, int ptx, int pty);
	ScummPoint closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y);
	void getBoxCoordinates(int boxnum, BoxCoords *bc);
	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);
	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();
	int getPathToDestBox(byte from, byte to);
	int findPathTowards(Actor *a, byte box, byte box2, byte box3);
	int findPathTowardsOld(Actor *a, byte box, byte box2, byte box3);
	void GetGates(int trap1, int trap2);
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	AdjustBoxResult getClosestPtOnBox(int box, int x, int y);
	int getSpecialBox(int param1, int param2);
	
	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);
	void createBoxMatrix();


	/* String class */
	CharsetRenderer charset;
	byte _charsetColor;
	uint16 _noSubtitles;	// Skip all subtitles?
	byte _charsetData[15][16];
	void initCharset(int charset);
	void restoreCharsetBg();
	int hasCharsetMask(int x, int y, int x2, int y2);
	void CHARSET_1();
	void description();
	byte *_msgPtrToAdd;
	byte *addMessageToStack(byte *msg);
	void unkAddMsgToStack2(int var);
	void unkAddMsgToStack3(int var);
	void unkAddMsgToStack4(int var);
	void unkAddMsgToStack5(int var);
	void unkMessage1();
	void unkMessage2();
	void clearMsgQueue();
	int _numInMsgStack;
	byte *_messagePtr;
	int16 _talkDelay;
	bool _keepText;


	/* Should be in System class */
	byte _fileMode;
	uint32 _whereInResToRead;
	void fileClose(void *file);
	void *fileOpen(const char *filename, int mode);
	void fileSeek(void *file, long offs, int whence);
	void fileRead(void *handle, void *ptr, uint32 size);
	bool fileEof(void *handle);
	uint32 filePos(void *handle);
	bool checkFixedDisk();
	int _cdrom;

	int fileReadByte();
	uint32 fileReadDwordLE();
	uint32 fileReadDwordBE();
	int fileReadByte(void *handle);
	uint32 fileReadDwordLE(void *handle);
	uint32 fileReadDwordBE(void *handle);

#if defined(SCUMM_LITTLE_ENDIAN)
	uint32 fileReadDword() { return fileReadDwordLE(); }
	uint32 fileReadDword(void *handle) { return fileReadDwordLE(handle); }
#elif defined(SCUMM_BIG_ENDIAN)
	uint32 fileReadDword() { return fileReadDwordBE(); }
	uint32 fileReadDword(void *handle) {return fileReadDwordBE(handle);}
#endif
	uint fileReadWordLE();
	uint fileReadWordBE();
	uint fileReadWordLE(void *handle);
	uint fileReadWordBE(void *handle);

	static char *Strdup(const char *);

	/* Version 5 script opcodes */
	void o5_actorFollowCamera();
	void o5_actorFromPos();
	void o5_actorSet();
	void o5_add();
	void o5_and();
	void o5_animateActor();
	void o5_badOpcode();
	void o5_breakHere();
	void o5_chainScript();
	void o5_cursorCommand();
	void o5_cutscene();
	void o5_debug();
	void o5_decrement();
	void o5_delay();
	void o5_delayVariable();
	void o5_divide();
	void o5_doSentence();
	void o5_drawBox();
	void o5_drawObject();
	void o5_dummy();
	void o5_endCutscene();
	void o5_equalZero();
	void o5_expression();
	void o5_faceActor();
	void o5_findInventory();
	void o5_findObject();
	void o5_freezeScripts();
	void o5_getActorCostume();
	void o5_getActorElevation();
	void o5_getActorFacing();
	void o5_getActorMoving();
	void o5_getActorRoom();
	void o5_getActorScale();
	void o5_getActorWalkBox();
	void o5_getActorWidth();
	void o5_getActorX();
	void o5_getActorY();
	void o5_getAnimCounter();
	void o5_getClosestObjActor();
	void o5_getDist();
	void o5_getInventoryCount();
	void o5_getObjectOwner();
	void o5_getObjectState();
	void o5_getRandomNr();
	void o5_getScriptRunning();
	void o5_getVerbEntrypoint();
	void o5_ifClassOfIs();
	void o5_increment();
	void o5_isActorInBox();
	void o5_isEqual();
	void o5_isGreater();
	void o5_isGreaterEqual();
	void o5_isLess();
	void o5_isNotEqual();
	void o5_isSoundRunning();
	void o5_jumpRelative();
	void o5_lessOrEqual();
	void o5_lights();
	void o5_loadRoom();
	void o5_loadRoomWithEgo();
	void o5_matrixOps();
	void o5_move();
	void o5_multiply();
	void o5_notEqualZero();
	void o5_or();
	void o5_overRide();
	void o5_panCameraTo();
	void o5_pickupObject();
	void o5_print();
	void o5_printEgo();
	void o5_pseudoRoom();
	void o5_putActor();
	void o5_putActorAtObject();
	void o5_putActorInRoom();
	void o5_quitPauseRestart();
	void o5_resourceRoutines();
	void o5_roomOps();
	void o5_saveRestoreVerbs();
	void o5_setCameraAt();
	void o5_setClass();
	void o5_setObjectName();
	void o5_setOwnerOf();
	void o5_setState();
	void o5_setVarRange();
	void o5_soundKludge();
	void o5_startMusic();
	void o5_startObject();
	void o5_startScript();
	void o5_startSound();
	void o5_stopMusic();
	void o5_stopObjectCode();
	void o5_stopObjectScript();
	void o5_stopScript();
	void o5_stopSound();
	void o5_stringOps();
	void o5_subtract();
	void o5_verbOps();
	void o5_wait();
	void o5_walkActorTo();
	void o5_walkActorToActor();
	void o5_walkActorToObject();
    void o5_oldRoomEffect();
	void o5_pickupObjectOld();

	/* Version 6 script opcodes */
	int getStackList(int16 *args, uint maxnum);
	void setStringVars(int i);
	void unkMiscOp9();

	void o6_setBlastObjectWindow();
	void o6_pushByte();
	void o6_pushWord();
	void o6_pushByteVar();
	void o6_pushWordVar();
	void o6_invalid();
	void o6_byteArrayRead();
	void o6_wordArrayRead();
	void o6_byteArrayIndexedRead();
	void o6_wordArrayIndexedRead();
	void o6_dup();
	void o6_zero();
	void o6_eq();
	void o6_neq();
	void o6_gt();
	void o6_lt();
	void o6_le();
	void o6_ge();
	void o6_add();
	void o6_sub();
	void o6_mul();
	void o6_div();
	void o6_land();
	void o6_lor();
	void o6_kill();
	void o6_writeByteVar();
	void o6_writeWordVar();
	void o6_byteArrayWrite();
	void o6_wordArrayWrite();
	void o6_byteArrayIndexedWrite();
	void o6_wordArrayIndexedWrite();
	void o6_byteVarInc();
	void o6_wordVarInc();
	void o6_byteArrayInc();
	void o6_wordArrayInc();
	void o6_byteVarDec();
	void o6_wordVarDec();
	void o6_byteArrayDec();
	void o6_wordArrayDec();
	void o6_jumpTrue();
	void o6_jumpFalse();
	void o6_jump();
	void o6_startScriptEx();
	void o6_startScript();
	void o6_startObject();
	void o6_setObjectState();
	void o6_setObjectXY();
	void o6_stopObjectCode();
	void o6_endCutscene();
	void o6_cutScene();
	void o6_stopMusic();
	void o6_freezeUnfreeze();
	void o6_cursorCommand();
	void o6_breakHere();
	void o6_ifClassOfIs();
	void o6_setClass();
	void o6_getState();
	void o6_setState();
	void o6_setOwner();
	void o6_getOwner();
	void o6_startSound();
	void o6_stopSound();
	void o6_startMusic();
	void o6_stopObjectScript();
	void o6_panCameraTo();
	void o6_actorFollowCamera();
	void o6_setCameraAt();
	void o6_loadRoom();
	void o6_stopScript();
	void o6_walkActorToObj();
	void o6_walkActorTo();
	void o6_putActorInRoom();
	void o6_putActorAtObject();
	void o6_faceActor();
	void o6_animateActor();
	void o6_doSentence();
	void o6_pickupObject();
	void o6_loadRoomWithEgo();
	void o6_getRandomNumber();
	void o6_getRandomNumberRange();
	void o6_getActorMoving();
	void o6_getScriptRunning();
	void o6_getActorRoom();
	void o6_getObjectX();
	void o6_getObjectY();
	void o6_getObjectOldDir();
	void o6_getObjectNewDir();
	void o6_getActorWalkBox();
	void o6_getActorCostume();
	void o6_findInventory();
	void o6_getInventoryCount();
	void o6_getVerbFromXY();
	void o6_beginOverride();
	void o6_endOverride();
	void o6_setObjectName();
	void o6_isSoundRunning();
	void o6_setBoxFlags();
	void o6_createBoxMatrix();
	void o6_resourceRoutines();
	void o6_roomOps();
	void o6_actorSet();
	void o6_verbOps();
	void o6_getActorFromXY();
	void o6_findObject();
	void o6_pseudoRoom();
	void o6_getActorElevation();
	void o6_getVerbEntrypoint();
	void o6_arrayOps();
	void o6_saveRestoreVerbs();
	void o6_drawBox();
	void o6_getActorWidth();
	void o6_wait();
	void o6_getActorScaleX();
	void o6_getActorAnimCounter1();
	void o6_soundKludge();
	void o6_isAnyOf();
	void o6_quitPauseRestart();
	void o6_isActorInBox();
	void o6_delay();
	void o6_delayLonger();
	void o6_delayVeryLong();
	void o6_stopSentence();
	void o6_print_0();
	void o6_print_1();
	void o6_print_2();
	void o6_print_3();
	void o6_printActor();
	void o6_printEgo();
	void o6_talkActor();
	void o6_talkEgo();
	void o6_dim();
	void o6_runVerbCodeQuick();
	void o6_runScriptQuick();
	void o6_dim2();
	void o6_abs();
	void o6_distObjectObject();
	void o6_distObjectPt();
	void o6_distPtPt();
	void o6_dummy_stacklist();
	void o6_miscOps();
	void o6_breakMaybe();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelFunction();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorPriority();

	/* Scumm Vars */
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
	byte VAR_TMR_1;
	byte VAR_TMR_2;
	byte VAR_TMR_3;
	byte VAR_MUSIC_FLAG;
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
	byte VAR_HOOK_SCRIPT;
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
	byte VAR_SAVELOADDIALOG_KEY;
	byte VAR_FIXEDDISK;
	byte VAR_CURSORSTATE;
	byte VAR_USERPUT;
	byte VAR_SOUNDRESULT;
	byte VAR_TALKSTOP_KEY;
	byte VAR_59;
	
	byte VAR_SOUNDPARAM;
	byte VAR_SOUNDPARAM2;
	byte VAR_SOUNDPARAM3;
	byte VAR_MOUSEPRESENT;
	byte VAR_PERFORMANCE_1;
	byte VAR_PERFORMANCE_2;
	byte VAR_ROOM_FLAG;
	byte VAR_GAME_LOADED;
	byte VAR_NEW_ROOM;
	byte VAR_VERSION;

	byte VAR_V5_DRAWFLAGS;
	byte VAR_MI1_TIMER;
	byte VAR_V5_OBJECT_LO;
	byte VAR_V5_OBJECT_HI;
	byte VAR_V5_TALK_STRING_Y;
	byte VAR_V5_CHARFLAG;

	byte VAR_V6_SCREEN_WIDTH;
	byte VAR_V6_SCREEN_HEIGHT;
	byte VAR_V6_EMSSPACE;
	byte VAR_V6_RANDOM_NR;

	byte VAR_STRING2DRAW;
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

	byte VAR_LEFTBTN_DOWN;
	byte VAR_RIGHTBTN_DOWN;
	byte VAR_LEFTBTN_HOLD;
	byte VAR_RIGHTBTN_HOLD;

	byte VAR_UNK_SCRIPT;
	byte VAR_UNK_SCRIPT_2;

	byte VAR_DEFAULT_TALK_DELAY;
	byte VAR_CHARSET_MASK;

	byte VAR_CUSTOMSCALETABLE;
	byte VAR_VIDEONAME;

	void launch();

	static Scumm *createFromDetector(GameDetector *detector, OSystem *syst);
	void go();

	void setupGUIColors();
	byte getDefaultGUIColor(int color);
	void waitForTimer(int msec_delay);

	void updateCursor();
	void animateCursor();
	void updatePalette();
};

class Scumm_v3 : public Scumm
{
public:
	void readIndexFile();
	virtual void loadCharset(int no);
};

class Scumm_v4 : public Scumm_v3
{
	void loadCharset(int no);
};

class Scumm_v5 : public Scumm
{
};

class Scumm_v6 : public Scumm
{
};

class Scumm_v7 : public Scumm
{
};

struct ScummDebugger {
	Scumm *_s;
	byte _command;
	char *_parameters;

	bool _welcome;

	int _go_amount;
	
	char _cmd_buffer[256];
	
	void on_frame();
	bool do_command();
	void enter();
	int get_command();
	void attach(Scumm *s);
	void detach();

	void printActors(int act);
	void printScripts();
};

extern const uint32 IMxx_tags[];
extern const byte default_scale_table[768];
extern uint16 _debugLevel;

void outputdisplay2(Scumm *s, int disp);
extern const byte revBitMask[8];
//void blitToScreen(Scumm *s, byte *src, int x, int y, int w, int h);

#if defined(__GNUC__)
void CDECL error(const char *s, ...) NORETURN;
#else
void CDECL NORETURN error(const char *s, ...);
#endif

void CDECL warning(const char *s, ...);
void CDECL debug(int level, const char *s, ...);
void checkHeap();
void blit(byte *dst, byte *src, int w, int h);
byte *findResource(uint32 tag, byte *searchin, int index);
byte *findResourceSmall(uint32 tag, byte *searchin, int index);
byte *findResource(uint32 tag, byte *searchin);
byte *findResourceSmall(uint32 tag, byte *searchin);
void setWindowName(Scumm *s);
uint16 newTag2Old(uint32 oldTag);

#endif
