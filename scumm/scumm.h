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

#include "common/engine.h"
#include "common/gameDetector.h"
#include "common/timer.h"
#include "common/file.h"

class GameDetector;
class NewGui;
class Dialog;
class Scumm;
class IMuse;
class IMuseDigital;
class Actor;
class Sound;
class Bundle;
struct ScummDebugger;
struct Serializer;
struct FindObjectInRoom;

typedef void (Scumm::*OpcodeProc)();

struct OpcodeEntry {
	OpcodeProc proc;
	const char *desc;
};

// Use g_scumm from error() ONLY
extern Scumm *g_scumm;

/* System Wide Constants */
enum {
	NUM_MIXER = 4,
	NUM_SCRIPT_SLOT = 40,
	NUM_LOCALSCRIPT = 60,
	NUM_SHADOW_PALETTE = 8,
	MAX_ACTORS = 30,
	KEY_SET_OPTIONS = 3456 // WinCE
};

struct ScummPoint {
	int x, y;
};

#include "gfx.h"
#include "boxes.h"

struct MemBlkHeader {
	uint32 size;
};

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
	uint16 delayFrameCount;
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
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK | MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
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
	byte *_ptr;	// FIXME: This field is *NOT* used - remove next time save game format changes
	byte _unk2, _bpp;
	byte _invNumBits;
	uint32 _charOffs;
	byte *_charPtr;
	int _width, _height;
	int _offsX, _offsY;
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
	int getSpacing(byte chr, byte *charset);
	int getStringWidth(int a, byte *str, int pos);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
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
	int8 unk;
	byte pad;
};

// TODO / FIXME: next time save game format changes, Fingolfin would like to
// revise StringTab. In particular, all t_* fields can be removed, making some
// code a bit cleaner & easier to understand.
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

class Scumm : public Engine {
public:
	/* Put often used variables at the top.
	 * That results in a shorter form of the opcode
	 * on some architectures. */
	IMuse *_imuse;
	IMuseDigital *_imuseDigital;
	uint32 _features;
	VerbSlot *_verbs;
	ObjectData *_objs;
	ScummDebugger *_debugger;
	Bundle * _bundle;
	Timer * _timer;
	Sound * _sound;

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

	/* system call object */

	/* Scumm main loop */

	void mainRun();

	/* _insane vars */

	int _insaneFlag;
	bool _insaneState;
	bool _videoFinished;
	
	
	void scummInit();
	void scummMain(int argc, char **argv); // is it still used ?
	int scummLoop(int delta);
	void initScummVars();

	void pauseGame(bool user);
	void shutDown(int i);
	void setOptions(void);


	// GUI
	NewGui *_newgui;

	Dialog *_pauseDialog;
	Dialog *_optionsDialog;
	Dialog *_saveLoadDialog;

	void runDialog(Dialog *dialog);
	void pauseDialog();
	void saveloadDialog();
	void optionsDialog();

	// Misc startup/event functions
	void main();
	void parseCommandLine(int argc, char **argv);
	void showHelpAndExit();
	bool detectGame();
	void processKbd();

	int checkKeyHit();
	void convertKeysToClicks();

	/* Random number generation */
	uint32 _randSeed1, _randSeed2;
	void initRandSeeds();
	uint getRandomNumber(uint max);
	uint getRandomNumberRng(uint min, uint max);

	/* Core variable definitions */
	byte _gameId;

	/* Core class/array definitions */
	Gdi gdi;

	Actor *_actors;	// Has MAX_ACTORS elements
	
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

	char *_audioNames;
	int32 _numAudioNames;

	/* Current objects - can go in their respective classes */
	byte _curActor;
	int _curVerb;
	int _curVerbSlot;
	int _curPalIndex;
	byte _currentRoom;
	VirtScreen *_curVirtScreen;

	bool _egoPositioned;
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
	const OpcodeEntry *_opcodes;
	byte **_lastCodePtr;
	int _resultVarNumber, _scummStackPos;
	int16 _localParamList[16],  _scummStack[150];
	
	OpcodeProc getOpcode(int i) { return _opcodes[i].proc; }
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
	//void setupOpcodes3();	
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
	File _fileHandle;
	char *_resFilePrefix, *_resFilePath;
	uint32 _fileOffset;
	char *_exe_name;	// This is the name we use for opening resource files
	char *_game_name;	// This is the game the user calls it, so use for saving
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
	int readSoundResourceSmallHeader(int type, int index);
	void setResourceCounter(int type, int index, byte flag);
	bool validateResource(const char *str, int type, int index);
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
	void getObjectXYPos(int object, int &x, int &y)	{ int dir; getObjectXYPos(object, x, y, dir); }
	void getObjectXYPos(int object, int &x, int &y, int &dir);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int getObjectIndex(int object);
	int whereIsObject(int object);
	int findObject(int x, int y);
	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);	
	int getObjectOrActorXY(int object, int &x, int &y);		 // Object and Actor...
	int getObjActToObjActDist(int a, int b); // Not sure how to handle
	byte *getObjOrActorName(int obj);		 // these three..

	void addObjectToDrawQue(int object);
	void clearDrawObjectQueue();
	void processDrawQue();

	uint32 getOBCDOffs(int object);
	byte *getOBCDFromObject(int obj);	
	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);

	/* Should be in Costume class */
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
	void drawVerbBitmap(int verb, int x, int y);
	int getVerbEntrypoint(int obj, int entry);
	int getVerbSlot(int id, int mode);
	void killVerb(int slot);
	void runVerbCode(int script, int entry, int a, int b, int16 *vars);
	void setVerbObject(uint room, uint object, uint verb);

	/* Should be in Actor class */
	Actor *derefActor(int id);
	Actor *derefActorSafe(int id, const char *errmsg);
	Actor *getFirstActor() { return _actors; } 
	void showActors();

	uint32 *_classData;

	int getAngleFromPos(int x, int y);

	void walkActors();
	void playActorSounds();
	void setActorRedrawFlags(bool fg, bool bg);
	void resetActorBgs();
	void processActors();
	int getActorFromPos(int x, int y);
	void faceActorToObj(int act, int obj);
	void actorFollowCamera(int act);
	
	bool isCostumeInUse(int i);

	/* Actor talking stuff */
	byte _actorToPrintStrFor;
	int _sentenceNum;
	SentenceTab _sentence[6];
	StringTab _string[6];
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
	bool _fullRedraw, _BgNeedsRedraw, _shakeEnabled, _verbRedraw;
	bool _screenEffectFlag, _completeScreenRedraw;

	int _cursorHotspotX, _cursorHotspotY, _cursorWidth, _cursorHeight;
	byte _cursorAnimate, _cursorAnimateIndex, _grabbedCursor[2048];
	int8 _cursorState;

	byte _newEffect, _switchRoomEffect2, _switchRoomEffect;
	bool _doEffect;
	
	uint16 _flashlightXStrips, _flashlightYStrips;
	bool _flashlightIsDrawn;

	void getGraphicsPerformance();
	void initScreens(int a, int b, int w, int h);
	void initVirtScreen(int slot, int number, int top, int width, int height, bool twobufs, bool scrollable);
	void initBGBuffers(int height);
	void initCycl(byte *ptr);	// Color cycle

	void createSpecialPalette(int16 a, int16 b, int16 c, int16 d, int16 e, int16 colorMin, int16 colorMax);

	void drawObject(int obj, int arg);	
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void drawBox(int x, int y, int x2, int y2, int color);

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
	void copyPalColor(int dst, int src);
	void cyclePalette();
	void stopCycle(int i);
	void palManipulateInit(int start, int end, int string_id, int time);
	void palManipulate();
	void unkRoomFunc3(int a, int b, int c, int d, int e);
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
	void setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom);

	void drawFlashlight();
	
	void fadeIn(int effect);
	void fadeOut(int effect);

	void unkScreenEffect1();
	void unkScreenEffect2();
	void unkScreenEffect3();
	void unkScreenEffect4();
	void unkScreenEffect5(int a);
	void unkScreenEffect6();
	void transitionEffect(int a);
	void dissolveEffect(int width, int height);
	void scrollEffect(int dir);

	void blit(byte *dst, byte *src, int w, int h);

	// bomp
	void decompressBomp(byte *dst, byte *src, int w, int h);
	void drawBomp(BompDrawData *bd, int decode_mode, int mask);
	int32 setupBompScale(byte *scalling, int32 size, byte scale);
	void bompScaleFuncX(byte *line_buffer, byte *scalling_x_ptr, byte skip, int32 size);
	int32 bompDecodeLineMode0(byte *src, byte *line_buffer, int32 size);
	int32 bompDecodeLineMode1(byte *src, byte *line_buffer, int32 size);
	int32 bompDecodeLineMode3(byte *src, byte *line_buffer, int32 size);
	void bompApplyMask(byte *line_buffer, byte *mask_out, byte bits, int32 size);
	void bompApplyShadow0(byte *line_buffer, byte *dst, int32 size);
	void bompApplyShadow1(byte *line_buffer, byte *dst, int32 size);
	void bompApplyShadow3(byte *line_buffer, byte *dst, int32 size);
	void bompApplyActorPalette(byte *line_buffer, int32 size);

	int32 _bompScaleRight, _bompScaleBottom;
	byte *_bompScallingXPtr, *_bompScallingYPtr;
	byte *_bompMaskPtr;
	int32 _bompMaskPitch;
	byte *_bompActorPalletePtr;


	uint _shakeFrame;
	int _screenStartStrip, _screenEndStrip;
	int _screenLeft, _screenTop;
	int _enqueuePos; 
	BlastObject _enqueuedObjects[128];

	void enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
                       int objectHeight, int scaleX, int scaleY, int image, int mode);
	void clearEnqueue() { _enqueuePos = 0; }
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);
	void removeBlastObjects();
	void removeBlastObject(BlastObject *eo);

	int _drawObjectQueNr;
	byte _drawObjectQue[200];
	byte _palManipStart, _palManipEnd;
	uint16 _palManipCounter;
	byte *_palManipPalette;
	byte *_palManipIntermediatePal;
	uint32 gfxUsageBits[410];
	byte *_shadowPalette;
	int _shadowPaletteSize;
	byte _currentPalette[3 * 256];

	byte _proc_special_palette[256];
	int _palDirtyMin, _palDirtyMax;
	byte _bkColor;
	uint16 _lastXstart;

	byte _haveMsg;
	bool _useTalkAnims;
	uint16 _defaultTalkDelay;
	bool _use_adlib;
	int tempMusic;
	bool _silentDigitalImuse;
	int _saveSound;
	uint16 _soundParam, _soundParam2, _soundParam3;
	int current_cd_sound, _cd_loops, _cd_frame, _cd_track, _cd_end;

	/* Walkbox / Navigation class */
	int _maxBoxVertexHeap, _boxPathVertexHeapIndex, _boxMatrixItem;
	byte *_boxMatrixPtr4, *_boxMatrixPtr1, *_boxMatrixPtr3;	

	uint16 _extraBoxFlags[65];

	PathNode *unkMatrixProc2(PathVertex *vtx, int i);
	bool areBoxesNeighbours(int i, int j);
	void addToBoxMatrix(byte b);
	bool compareSlope(int X1, int Y1, int X2, int Y2, int X3, int Y3);
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
	bool findPathTowards(Actor *a, byte box, byte box2, byte box3, int16 &foundPathX, int16 &foundPathY);
	void findPathTowardsOld(Actor *a, byte box, byte box2, byte box3, ScummPoint gateLoc[5]);
	void getGates(int trap1, int trap2, ScummPoint gateA[2], ScummPoint gateB[2]);
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	AdjustBoxResult getClosestPtOnBox(int box, int x, int y);
	int getSpecialBox(int param1, int param2);
	
	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);
	void createBoxMatrix();


	/* String class */
	CharsetRenderer charset;
	byte _charsetColor;
	bool _noSubtitles;	// Skip all subtitles?
	byte _charsetData[15][16];
	void initCharset(int charset);
	void restoreCharsetBg();
	int hasCharsetMask(int x, int y, int x2, int y2);
	void CHARSET_1();
	void description();
	void drawDescString(byte *msg);
	byte *_msgPtrToAdd;
	byte *addMessageToStack(byte *msg);
	void addIntToStack(int var);
	void addVerbToStack(int var);
	void addNameToStack(int var);
	void addStringToStack(int var);
	void unkMessage1();
	void unkMessage2();
	void clearMsgQueue();
	int _numInMsgStack;
	byte *_messagePtr;
	int16 _talkDelay;
	bool _keepText;
	bool _existLanguageFile;
	char *_languageBuffer;
	void loadLanguageBundle();
	void translateText(byte *text, byte *trans_buff);
	byte _transText[256];

	bool checkFixedDisk();

#if defined(SCUMM_LITTLE_ENDIAN)
	uint32 fileReadDword() { return _fileHandle.readUint32LE(); }
#elif defined(SCUMM_BIG_ENDIAN)
	uint32 fileReadDword() { return _fileHandle.readUint32BE(); }
#endif

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
	void o5_ifState();
	void o5_ifNotState();
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
	void o6_delayFrames();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelFunction();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorPriority();
	void o6_unknownCD();
	void o6_bor();
	void o6_band();
	// void o7_userfaceOps();

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
	byte VAR_V6_SOUNDMODE;

	void launch();

	Scumm(GameDetector *detector, OSystem *syst);
	virtual ~Scumm();

	void go();

	byte getDefaultGUIColor(int color);
	void waitForTimer(int msec_delay);

	void updateCursor();
	void animateCursor();
	void updatePalette();
};

class Scumm_v2 : public Scumm
{
public:
	Scumm_v2(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}

	virtual void readIndexFile();
};

class Scumm_v3 : public Scumm
{
public:
	Scumm_v3(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}

	void readIndexFile();
	virtual void loadCharset(int no);
};

class Scumm_v4 : public Scumm_v3
{
public:
	Scumm_v4(GameDetector *detector, OSystem *syst) : Scumm_v3(detector, syst) {}

	void loadCharset(int no);
};

class Scumm_v5 : public Scumm
{
public:
	Scumm_v5(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}
};

class Scumm_v6 : public Scumm
{
public:
	Scumm_v6(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}
};

class Scumm_v7 : public Scumm
{
public:
	Scumm_v7(GameDetector *detector, OSystem *syst) : Scumm(detector, syst) {}
};

// This is a constant lookup table of reverse bit masks
extern const byte revBitMask[8];

/* Direction conversion functions (between old dir and new dir format) */
int newDirToOldDir(int dir);
int oldDirToNewDir(int dir);

int normalizeAngle(int angle);
int fromSimpleDir(int dirtype, int dir);
int toSimpleDir(int dirtype, int dir);


#endif
