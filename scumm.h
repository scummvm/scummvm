/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
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
 * Change Log:
 * $Log$
 * Revision 1.2  2001/10/09 17:38:20  strigeus
 * Autodetection of endianness.
 *
 * Revision 1.1.1.1  2001/10/09 14:30:12  strigeus
 *
 * initial revision
 *
 *
 */

#include "scummsys.h"

#ifdef WIN32
#pragma warning (disable: 4018)
#pragma warning (disable: 4244)
#endif

#define SWAP(a,b) do{int tmp=a; a=b; b=tmp; } while(0)

#define BYPASS_COPY_PROT
#define DUMP_SCRIPTS

struct Scumm;
struct Actor;

typedef void (Scumm::*OpcodeProc)();

#pragma START_PACK_STRUCTS
	
struct Point {
	int x,y;
};

struct AdjustBoxResult {
	int16 x,y;
	uint16 dist;
};

#define SIZEOF_BOX 20
struct Box { /* file format */
	int16 ulx,uly;
	int16 urx,ury;
	int16 llx,lly;
	int16 lrx,lry;
	byte mask;
	byte flags;
	uint16 scale;
};

struct VerbSlot {
	int16 x,y;
	int16 right, bottom;
	int16 oldleft, oldtop, oldright,oldbottom;
	uint8 verbid;
	uint8 color,hicolor,dimcolor,bkcolor,type;
	uint8 charset_nr,curmode;
	uint8 saveid;
	uint8 key,center;
	uint8 field_1B;
	uint16 imgindex;
};

struct VirtScreen {
	uint16 unk1;
	uint16 topline;
	uint16 width,height;
	uint16 size;
	byte alloctwobuffers;
	byte fourlinesextra;
	uint16 xstart;
	byte tdirty[40];
	byte bdirty[40];
};

struct ActorWalkData {
	int16 destx,desty;
	byte destbox;
	byte destdir;
	byte curbox;
	byte field_7;
	int16 x,y,newx,newy;
	int32 XYFactor, YXFactor;
	uint16 xfrac,yfrac;
};

struct CostumeData {
	uint16 hdr;
	uint16 animCounter1;
	byte animCounter2;
	byte x_1;
	uint16 a[16], b[16], c[16], d[16];
};

struct MouseCursor {
	int8 hotspot_x, hotspot_y;
	byte colors[4];
	byte data[32];
};

struct ScriptSlot {
	uint32 offs;
	int32 delay;
	uint16 number;
	byte status;
	byte type;
	byte unk1,unk2,freezeCount,didexec;
	byte cutsceneOverride;
	byte unk5;
};

struct NestedScript {
	uint16 number;
	uint8 type;
	uint8 slot;
};

struct ObjectData {
	uint32 offs_obim_to_room;
	uint32 offs_obcd_to_room;
	uint16 cdhd_10, cdhd_12;
	uint16 obj_nr;
	byte x_pos;
	byte y_pos;
	byte numstrips;
	byte height;
	byte actordir;
	byte cdhd_0f;
	byte cdhd_0e;
	byte ownerstate;
	byte fl_object_index;
	byte unk_3;
};

struct RoomHeader {
	uint32 tag, size;
	uint16 width,height;
	uint16 numObjects;
};

struct CodeHeader { /* file format */
	uint32 id;
	uint32 size;
	uint16 obj_id;
	byte x,y,w,h;
	byte flags;
	byte unk1;
	uint16 unk2;
	uint16 unk3;
	byte unk4;
};

struct ImageHeader { /* file format */
	uint32 id;
	uint32 size;
	uint16 obj_id;
};

struct PathNode {
	uint index;
	struct PathNode *left, *right;
};

struct PathVertex {
	PathNode *left;
	PathNode *right;
};

#pragma END_PACK_STRUCTS

enum ScummVars {
	VAR_UNK_ACTOR = 1,
	VAR_WALKTO_OBJ = 38,
	VAR_OVERRIDE = 5,
	VAR_NUM_ACTOR = 8,
	VAR_OBJECT_LO = 15,
	VAR_OBJECT_HI = 16,
	VAR_CURRENTDRIVE = 10,
	VAR_TALK_ACTOR = 25,
	VAR_DEBUGMODE = 39,
	VAR_VERSION = 75,
	VAR_FIXEDDISK = 51,
	VAR_CURSORSTATE = 52,
	VAR_USERPUT = 53,
	VAR_SOUNDCARD = 48,
	VAR_VIDEOMODE = 49,
	VAR_HEAPSPACE = 40,
	VAR_MOUSEPRESENT = 67,
	VAR_SOUNDPARAM = 64,
	VAR_SOUNDPARAM2 = 65,
	VAR_SOUNDPARAM3 = 66,
	VAR_GAME_LOADED = 71,
	VAR_VIRT_MOUSE_X = 20,
	VAR_VIRT_MOUSE_Y = 21,
	VAR_PERFORMANCE_1 = 68,
	VAR_PERFORMANCE_2 = 69,
	VAR_ROOM_FLAG = 70,
	VAR_HAVE_MSG = 3,
	VAR_ENTRY_SCRIPT = 28,
	VAR_ENTRY_SCRIPT2 = 29,
	VAR_EXIT_SCRIPT = 30,
	VAR_EXIT_SCRIPT2 = 31,
	VAR_VERB_SCRIPT = 32,
	VAR_SENTENCE_SCRIPT = 33,
	VAR_LAST_SOUND = 23,
	VAR_HOOK_SCRIPT = 34,
	VAR_CUTSCENE_START_SCRIPT = 35,
	VAR_CUTSCENE_END_SCRIPT = 36,
	VAR_SCROLL_SCRIPT = 27,
	VAR_CAMERA_MIN = 17,
	VAR_CAMERA_MAX = 18,
	VAR_CAMERA_FAST = 26,
	VAR_CAMERA_CUR_POS = 2,
	VAR_NEW_ROOM = 72,
	VAR_ROOM = 4,
	VAR_ROOM_RESOURCE = 22,

	VAR_MOUSE_X = 44,
	VAR_MOUSE_Y = 45,
	
	VAR_TIMER = 46,
	VAR_TIMER_NEXT = 19,

	VAR_TMR_1 = 11,
	VAR_TMR_2 = 12,
	VAR_TMR_3 = 13,
	VAR_TMR_4 = 47,

	VAR_DRAWFLAGS = 9,

	VAR_SOUNDRESULT = 56,

	VAR_PLAYBACKTIMER = 19,

	VAR_TALK_STRING_Y = 54,
	VAR_CHARFLAG = 60,
	VAR_CHARINC = 37,

	VAR_RESTART_KEY = 42,
	VAR_PAUSE_KEY = 43,
	VAR_CUTSCENEEXIT_KEY = 24,
	VAR_TALKSTOP_KEY = 57,
	VAR_SAVELOADDIALOG_KEY = 50,
};

#define _maxRooms res.num[1]
#define _maxScripts res.num[2]
#define _maxCostumes res.num[3]
#define _maxInventoryItems res.num[5]
#define _maxCharsets res.num[6]
#define _maxStrings res.num[7]
#define _maxVerbs res.num[8]
#define _maxActorNames res.num[9]
#define _maxBuffer res.num[10]
#define _maxScaleTable res.num[11]
#define _maxTemp res.num[12]
#define _maxFLObject res.num[13]
#define _maxMatrixes res.num[14]
#define _maxBoxes res.num[15]

#define _baseRooms res.address[1]
#define _baseScripts res.address[2]
#define _baseInventoryItems res.address[5]
#define _baseFLObject res.address[13]

#define _roomFileOffsets res.roomoffs[1]

struct CharsetRenderer {
	Scumm *_vm;
	int _top;
	int _drawTop;
	int _left, _left2;
	byte _center;
	uint _right;
	byte _color;
	bool _hasMask;
	
	int _strLeft, _strRight, _strTop, _strBottom;
	int _mask_bottom, _mask_right, _mask_top, _mask_left;
	byte _curId;
	
	byte _bufPos;
	byte _unk12,_disableOffsX;
	byte *_ptr;
	byte _unk2, _bpp;
	byte _invNumBits;
	uint32 _charOffs;
	byte *_charPtr;
	int _width, _height;
	int _offsX,_offsY;
	byte _bitMask, _revBitMask;
	int _bottom;
	int _virtScreenHeight;

	byte _ignoreCharsetMask;

	byte *_bg_ptr, *_where_to_draw_ptr;
	byte *_mask_ptr;
		
	byte _colorMap[16];
	byte _buffer[256];

	void drawBits();
	void printChar(int chr);
	int getStringWidth(int a, byte *str, int pos);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
};

struct CostumeRenderer {
	Scumm *_vm;
	byte *_ptr;
	byte _numColors;
	byte *_dataptr;
	byte *_frameptr;
	byte *_srcptr;
	byte *_where_to_draw_ptr, *_bg_ptr, *_mask_ptr, *_mask_ptr_dest;
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
	int _scaleIndexXStep;
	int _scaleIndexYStep;
	byte _scaleIndexX; /* must wrap at 256*/
	byte _scaleIndexY, _scaleIndexYTop;
	int _left,_right;
	int _dir2;
	int _top,_bottom;
	int _ypostop;
	uint _vscreenheight;
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
	byte mainRoutine(Actor *a, int slot, int frame);
	void ignorePakCols(int num);

	void loadCostume(int id);
	byte drawOneSlot(Actor *a, int slot);
	byte drawCostume(Actor *a);

	byte animateOneSlot(CostumeData *cd, int slot);
	byte animate(CostumeData *cd);
};

struct Actor {
	int16 x,y,top,bottom;
	int16 elevation;
	uint16 width;
	byte number;
	byte facing;
	byte costume;
	byte room;
	byte talkColor;
	byte scalex,scaley;
	byte charset,sound;
	byte newDirection;
	byte moving;
	byte ignoreBoxes;
	byte neverZClip;
	byte initFrame,walkFrame,standFrame,talkFrame1,talkFrame2;
	bool needRedraw, needBgReset,costumeNeedsInit,visible;
	uint16 speedx,speedy;
	byte data8; /* unused */
	byte animIndex;
	byte walkbox;
	byte mask;
	byte animProgress, animSpeed;
	ActorWalkData walkdata;
	CostumeData cost;
	byte palette[32];
};

struct CameraData {
	int16 _destPos, _curPos, _lastPos;
	int16 _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	uint16 _movingToActor;
};

struct Scumm {
	int _lastLoadedRoom;
	int _roomResource;
	byte _encbyte;
	void *_fileHandle;
	char *_exe_name;
	
	bool _dynamicRoomOffsets;
	byte _resFilePathId;
	
	char *_resFilePrefix;
	char *_resFilePath;

	int _keyPressed;

	uint8 *_roomFileIndexes;
	byte *_objectFlagTable;
	uint32 *_classData;

	byte _numGlobalScriptsUsed;
	
	uint16 _maxNrObjects;
	
	uint16 _numZBuffer;
	
	uint32 _randSeed1;
	uint32 _randSeed2;

	uint16 dseg_3A76;
	uint16 _defaultTalkDelay;
	uint16 _lightsValueA,_lightsValueB;
	byte _haveMsg;
	byte _newEffect;
	uint16 _fullRedraw;
	uint16 dseg_3DB6;
	uint16 dseg_2456; /* lastDrawnRoom */
	uint16 dseg_4E8A;
	uint16 _soundParam,_soundParam2,_soundParam3;
	uint16 dseg_4F8C;
	
	byte _switchRoomEffect2, _switchRoomEffect;
	uint16 dseg_4AC2;
	uint16 dseg_4F8A;
	
	uint16 _drawBmpX;
	uint16 dseg_719E;
	uint16 _drawBmpY;
	uint16 dseg_4174;
	byte dseg_4E3C;
	uint16 _lastXstart;
	uint16 dseg_4EA0;
	
	bool _keepText;
	
	int16 _talkDelay;
	int16 _shakeMode;

	int16 _virtual_mouse_x, _virtual_mouse_y;

	byte _charsetColor;

	uint32 _localScriptList[0x39];

	uint16 _inventory[0x50];

	uint16 _debugMode;

	byte *_messagePtr;

	byte _numNestedScripts;
	byte _unkTabIndex;
	byte _currentScript;

	byte _currentRoom;
	byte _numObjectsInRoom;
	byte _actorToPrintStrFor;

	int16 _screenStartStrip;
	int16 _screenEndStrip;

	int16 _scummTimer;

	byte _playBackFile;
	byte _fastMode;
	
	uint16 _completeScreenRedraw;

	byte _saveLoadFlag;

	int8 _userPut;
	int8 _cursorState;

	
	uint16 _mouseButStat;
	byte _leftBtnPressed, _rightBtnPressed;

	int _numInMsgStack;

	VerbSlot verbs[102];
	VirtScreen virtscr[4];

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs, _EPAL_offs;

	int _drawObjectQueNr;
	byte _drawObjectQue[0xC8];

	uint16 _currentDrive;
	uint16 _soundCardType;
	uint16 _videoMode;
	uint16 _heapSpace;
	byte _mousePresent;

	int16 _palManipStart;
	int16 _palManipEnd;
	int16 _palManipCounter;

	struct {
		byte mode[16];
		uint16 num[16];
		uint32 tags[16];
		const char *name[16];
		byte **address[16];
		byte *flags[16];
		byte *roomno[16];
		uint32 *roomoffs[16];
	} res;

	struct {
		int16 vars[801];
		byte bitvars[256];
		uint16 scriptoffs[180];
		uint16 inventory[80];
		uint32 cutScenePtr[5];
		byte cutSceneScript[5];
		int16 cutSceneData[5];
		int16 cutSceneScriptIndex;
		byte cutSceneStackPointer;
		ScriptSlot slot[20];
		NestedScript nest[15];
		int16 localvar[20*17];
	} vm;

	struct {
		int16 x,y;
	} mouse;


	struct {
		int16 x[6];
		int16 y[6];
		int16 center[6];
		int16 overhead[6];
		int16 right[6];
		int16 color[6];
		int16 charset[6];
	} textslot;

	struct {
		byte *readPtr;
		uint16 readOffs;
		uint16 drawY;
		uint16 drawHeight;
		uint16 drawWidth;
		uint16 draw8xPos;
		uint16 unk3;
		int16 virtScreen;
		uint16 drawBottom;
		uint16 drawTop;

		int8 unk4;

		byte numLinesToProcess;
		byte tempNumLines;
		byte currentX;
		byte hotspot_x;
		byte hotspot_y;
		int16 drawMouseX;
		int16 drawMouseY;
		byte currentCursor;
		byte mouseColors[4];
		byte mouseColor;
		byte mouseClipMask1, mouseClipMask2, mouseClipMask3;
		byte mouseColorIndex;
		byte mouseMask[0x200];
		byte *mouseMaskPtr;
		byte *smap_ptr;
		byte *bg_ptr;
		byte *where_to_draw_ptr;
		byte *mask_ptr;
		byte *mask_ptr_dest;
		byte *z_plane_ptr;

		byte decomp_shr, decomp_mask;
		byte transparency;
		uint16 vertStripNextInc;
		byte *backupIsWhere;
		byte mouseBackup[16*24];
	} gdi;

	Actor actor[13];

	uint16 actorDrawBits[160];

	struct {
		int upperLeftX;
		int upperRightX;
		int lowerLeftX;
		int lowerRightX;
		int upperLeftY;
		int upperRightY;
		int lowerLeftY;
		int lowerRightY;
	} box;

	CharsetRenderer charset;
	
	byte _charsetData[10][16];

	byte _resourceMapper[128];

	uint16 _resIndexToLoad, _resTypeToLoad;

	byte **_lastCodePtr;
	byte _dir;

	int _numSoundTags;
	byte *_soundTagTable;

	int16 _bootParam;

	uint32 _fileOffset;

	byte _fileReadFailed;
	byte _fileMode;

	uint32 _whereInResToRead;

	byte *_scriptPointer, *_scriptOrgPointer;
	byte *_scriptPointerStart;
	byte _opcode;

	int _xPos, _yPos;

	CameraData camera;

	int _resultVarNumber;

	uint16 _imgBufOffs[4];

	byte _sentenceIndex;
	byte _sentenceTab[6];
	byte _sentenceTab2[6];
	uint16 _sentenceTab3[6];
	uint16 _sentenceTab4[6];
	byte _sentenceTab5[6];

	uint16 _stringOverhead[6];
	uint16 _stringCenter[6];
	uint16 _stringRight[6];
	uint16 _stringColor[6];

	int16 _stringXpos[6];
	int16 _stringYpos[6];
	uint16 _stringCharset[6];

	int16 _stringXpos2[6];
	int16 _stringYpos2[6];

	CostumeRenderer cost;

	ObjectData objs[184];

	int16 _soundQuePos;
	int16 _soundQue[0x100];

	byte _soundQue2Pos;
	byte _soundQue2[10];

	int16 _vararg_temp_pos[16];

	int16 _curExecScript;

	int _scrWidthIn8Unit;
	int _scrHeight;

	uint32 _IM00_offs;

	int _colorsInPalette;

	byte _currentPalette[0x300];

	int _palDirtyMin, _palDirtyMax;

	byte _saveLoadData;

	uint16 _colorCycleDelays[17];
	uint16 _colorCycleCounter[17];
	uint16 _colorCycleFlags[17];
	byte _colorCycleStart[17];
	byte _colorCycleEnd[17];

	byte dseg_4E3B;

	uint32 _findResSize, _findResHeaderSize;
	byte *_findResPos;

	uint32 _findResSize2, _findResHeaderSize2;
	byte *_findResPos2;

	bool _BgNeedsRedraw;

	int _stringXPos[4], _stringYPos[4];

	int16 _localParamList[16];

	uint16 _verbMouseOver;

	int16 _foundPathX;
	int16 _foundPathY;

	uint16 _onlyActorFlags;

	uint16 _lastKeyHit;

	int _scummStackPos;
	int16 _scummStack[0x15];

	int _maxBoxVertexHeap;
	byte *_boxMatrixPtr4, *_boxMatrixPtr1, *_boxMatrixPtr3;
	int _boxPathVertexHeapIndex;
	int _boxMatrixItem;
	
	/* all these can be made local */
	byte *_msgPtrToAdd;

	void openRoom(int room);
	void deleteRoomOffsets();
	void readRoomsOffsets();
	void askForDisk();

	void readIndexFile(int i);
	bool openResourceFile(const char *filename);
	
	void fileClose(void *file);
	void *fileOpen(const char *filename, int mode);
	void fileSeek(void *file, long offs, int whence);
	void fileRead(void *handle, void *ptr, uint32 size);
	bool fileEof(void *handle);

	int fileReadByte();
	uint32 fileReadDwordLE();
	uint32 fileReadDwordBE();
#if defined(SCUMM_LITTLE_ENDIAN)
	uint32 fileReadDword() { return fileReadDwordLE(); }
#elif defined(SCUMM_BIG_ENDIAN)
	uint32 fileReadDword() { return fileReadDwordBE(); }
#endif
	uint fileReadWordLE();
	uint fileReadWordBE();

	byte *alloc(int size);
	void free(void *mem);

	void readResTypeList(int id, uint32 tag, const char *name);
	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);

	void initThings();

	void initVideoMode();
	void initKbdAndMouse();
	void detectSound();
	void initRandSeeds();

	uint getRandomNumber(uint max);

	void loadCharset(int i);
	void nukeCharset(int i);
	void initScreens(int a, int b, int w, int h);

	void setShake(int mode);
	void setCursor(int cursor);

	void clearDrawObjectQueue();

	byte *createResource(int type, int index, uint32 size);

	void initScummVars();
	void getGraphicsPerformance();

	void nukeResource(int type, int i);
	byte *getResourceAddress(int type, int i);
	void ensureResourceLoaded(int type, int i);
	int loadResource(int type, int i);
	int getResourceRoomNr(int type, int index);
	int readSoundResource(int type, int index);
	void setResourceFlags(int type, int index, byte flag);
	void validateResource(const char *str, int type, int index);
	
	void initVirtScreen(int slot, int top, int height, bool twobufs, bool fourextra);
	void setDirtyRange(int slot, int a, int height);
	void unkVirtScreen2();
	void updateDirtyScreen(int slot);
	void unkVirtScreen4(int a);
	void unkVirtScreen5(int a);

	void removeMouseCursor();

	void showMouseCursor();
	void drawStripToScreen();
	void restoreMouse();
	void initActor(Actor *a, int mode);
	bool checkFixedDisk();

	void setActorWalkSpeed(Actor *a, int speed1, int speed2);
	int calcMovementFactor(Actor *a, int newx, int newy);
	int actorWalkStep(Actor *a);
	int getProgrDirChange(Actor *a, int mode);

	bool checkXYInBoxBounds(int box, int x, int y);
	void setupActorScale(Actor *a);
	
	void checkRange(int max, int min, int no, const char *str);

	bool fileReadFailed(void *handle);
	void clearFileReadFailed(void *handle);

	bool getClass(int obj, int cls);
	void putClass(int obj, int cls, bool set);
	int getState(int obj);
	void putState(int obj, int state);
	int getOwner(int obj);
	void putOwner(int obj, int owner);

	void main();

	uint distanceFromPt(int x, int y, int ptx, int pty);
	Point closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y);
	bool getSideOfLine(int x1,int y1, int x2, int y2, int x, int y, int box);
	void getBoxCoordinates(int box);
	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);
	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();

	void startAnimActor(Actor *a, int frame, byte direction);
	void initActorCostumeData(Actor *a);
	void fixActorDirection(Actor *a, byte direction);
	void decodeCostData(Actor *a, int frame, uint mask);

	void scummInit();
	void scummMain();

	void runScript(int script, int a, int b, int16 *lvarptr);
	void stopScriptNr(int script);
	int getScriptSlot();
	void runScriptNested(int script);
	void updateScriptPtr();
	void getScriptBaseAddress();
	void getScriptEntryPoint();
	void executeScript();
	byte fetchScriptByte();
	int fetchScriptWord();
	void ignoreScriptWord();
	void ignoreScriptByte();
	int getVarOrDirectWord(byte mask);
	int getVarOrDirectByte(byte mask);
	int readVar(uint var);
	void getResultPos();
	void setResult(int result);
	
	int getObjectIndex(int object);

	void o_actorFollowCamera();
	void o_actorFromPos();
	void o_actorSet();
	void o_actorSetClass();
	void o_add();
	void o_and();
	void o_animateActor();
	void o_badOpcode();
	void o_breakHere();
	void o_chainScript();
	void o_cursorCommand();
	void o_cutscene();
	void o_debug();
	void o_decrement();
	void o_delay();
	void o_delayVariable();
	void o_divide();
	void o_doSentence();
	void o_drawBox();
	void o_drawObject();
	void o_dummy();
	void o_endCutscene();
	void o_equalZero();
	void o_expression();
	void o_faceActor();
	void o_findInventory();
	void o_findObject();
	void o_freezeScripts();
	void o_getActorCostume();
	void o_getActorElevation();
	void o_getActorFacing();
	void o_getActorMoving();
	void o_getActorRoom();
	void o_getActorScale();
	void o_getActorWalkBox();
	void o_getActorWidth();
	void o_getActorX();
	void o_getActorY();
	void o_getAnimCounter();
	void o_getClosestObjActor();
	void o_getDist();
	void o_getInventoryCount();
	void o_getObjectOwner();
	void o_getObjectState();
	void o_getRandomNr();
	void o_getScriptRunning();
	void o_getVerbEntrypoint();
	void o_ifClassOfIs();
	void o_increment();
	void o_isActorInBox();
	void o_isEqual();
	void o_isGreater();
	void o_isGreaterEqual();
	void o_isLess();
	void o_isNotEqual();
	void o_isSoundRunning();
	void o_jumpRelative();
	void o_lessOrEqual();
	void o_lights();
	void o_loadRoom();
	void o_loadRoomWithEgo();
	void o_matrixOps();
	void o_move();
	void o_multiply();
	void o_notEqualZero();
	void o_or();
	void o_overRide();
	void o_panCameraTo();
	void o_pickupObject();
	void o_print();
	void o_printEgo();
	void o_pseudoRoom();
	void o_putActor();
	void o_putActorAtObject();
	void o_putActorInRoom();
	void o_quitPauseRestart();
	void o_resourceRoutines();
	void o_roomOps();
	void o_saveRestoreVerbs();
	void o_setCameraAt();
	void o_setObjectName();
	void o_setOwnerOf();
	void o_setState();
	void o_setVarRange();
	void o_soundKludge();
	void o_startMusic();
	void o_startObject();
	void o_startScript();
	void o_startSound();
	void o_stopMusic();
	void o_stopObjectCode();
	void o_stopObjectScript();
	void o_stopScript();
	void o_stopSound();
	void o_stringOps();
	void o_subtract();
	void o_verbOps();
	void o_wait();
	void o_walkActorTo();
	void o_walkActorToActor();
	void o_walkActorToObject();

	void stopObjectCode();
	void stopObjectScript(int script);
	void putActor(Actor *a, int x, int y, byte room);
	void clearMsgQueue();
	void adjustActorPos(Actor *a);
	
	void hideActor(Actor *a);
	void showActor(Actor *a);
	void showActors();
	void turnToDirection(Actor *a, int newdir);

	int whereIsObject(int object);
	int getObjectOrActorXY(int object);
	void addSoundToQueue(int sound);
	void addSoundToQueue2(int sound);
	bool isScriptLoaded(int script);
	int getActorXYPos(Actor *a);
	void getObjectXYPos(int object);
	AdjustBoxResult adjustXYToBeInBox(Actor *a, int x, int y);

	int getWordVararg(int16 *ptr);

	int getObjActToObjActDist(int a, int b);
	void unkSoundProc22();
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	AdjustBoxResult getClosestPtOnBox(int box, int x, int y);

	void setCameraAt(int dest);
	void stopTalk();
	void restoreCharsetBg();

	void setCameraFollows(Actor *a);
	void runHook(int i);
	void startScene(int room, Actor *a, int b);

	void freezeScripts(int scr);
	void unfreezeScripts();

	void runAllScripts();

	int findObject(int x, int y);
	void stopCycle(int i);
	void killScriptsAndResources();
	void runExitScript();
	void runEntryScript();

	void unkResourceProc();
	void initRoomSubBlocks();
	void loadRoomObjects();

	void setPaletteFromRes();
	void initCycl(byte *ptr);
	void initBGBuffers();
	void setDirtyColors(int min, int max);

	byte *findResource(uint32 tag, byte *searchin);
	byte *findResource2(uint32 tag, byte *searchin);

	void setScaleItem(int slot, int a, int b, int c, int d);

	void cyclePalette();

	void moveMemInPalRes(int start, int end, byte direction);

	void redrawBGAreas();
	void drawRoomObjects(int arg);
	void redrawBGStrip(int start, int num);
	void drawObject(int obj, int arg);

	void drawBmp(byte *ptr, int a, int b, int c, const char *str, int objnr);
	void decompressBitmap();
	int hasCharsetMask(int x, int y, int x2, int y2);
	void draw8ColWithMasking();
	void clear8ColWithMasking();
	void clear8Col();
	void decompressMaskImgOr();
	void decompressMaskImg();

	void GDI_UnkDecode1();
	void GDI_UnkDecode2();
	void GDI_UnkDecode3();
	void GDI_UnkDecode4();
	void GDI_UnkDecode5();
	void GDI_UnkDecode6();
	void GDI_UnkDecode7();

	void restoreBG(int left, int top, int right, int bottom);
	void updateDirtyRect(int virt, int left, int right, int top, int bottom, uint16 dirtybits);
	int findVirtScreen(int y);

	void unkScreenEffect6();
	void unkScreenEffect5(int a);

	void playSound(int sound);

	void decreaseScriptDelay(int amount);
	void processKbd();

	void clearUpperMask();
	void redrawVerbs();
	void checkExecVerbs();
	void checkAndRunVar33();
	void CHARSET_1();
	void walkActors();
	void moveCamera();
	void fixObjectFlags();
	void clear_fullRedraw();
	void palManipulate();
	void screenEffect(int effect);
	void clearClickedStatus();
	void verbMouseOver(int verb);
	int checkMouseOver(int x, int y);
	void playActorSounds();
	void processDrawQue();
	void setActorRedrawFlags();
	void resetActorBgs();
	void processActors();
	void drawVerb(int verb, int mode);

	void runInputScript(int a, int cmd, int mode);

	void cameraMoved();

	void walkActor(Actor *a);

	int checkKeyHit();

	int getPathToDestBox(int from, int to);
	int findPathTowards(Actor *a, int box, int box2, int box3);

	void setActorCostPalette(Actor *a);
	void drawActorCostume(Actor *a);
	void actorAnimate(Actor *a);

	int getActorFromPos(int x, int y);

	void restoreVerbBG(int verb);

	void drawString(int a);
	void drawVerbBitmap(int vrb, int x, int y);

	void setActorCostume(Actor *a, int c);
	void loadPtrToResource(int type, int i, byte *ptr);

	void stackPush(int a);
	int stackPop();

	void walkActorTo(Actor *a, int x, int y, int direction);

	void setCursorImg(int cursor, int img);
	void setCursorHotspot(int cursor, int x, int y);
	void initCharset(int charset);
	void addObjectToDrawQue(int object);
	int getVerbEntrypoint(int obj, int entry);
	int unkSoundProc23(int a);
	void startWalkActor(Actor *a, int x, int y, int dir);
	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);
	void createBoxMatrix();
	void addObjectToInventory(int obj, int room);
	void removeObjectFromRoom(int obj);
	void decodeParseString();
	void pauseGame(int i);
	void shutDown(int i);
	void lock(int type, int i);
	void unlock(int type, int i);
	void heapClear(int mode);
	void unkHeapProc2(int a, int b);
	void unkResProc(int a, int b);
	void setPalColor(int index, int r, int g, int b);
	void unkRoomFunc2(int a, int b, int c, int d, int e);
	void unkRoomFunc3(int a, int b, int c, int d, int e);
	void unkRoomFunc4(int a, int b, int c, int d, int e);
	int getVerbSlot(int id, int mode);
	void killVerb(int slot);
	byte *getObjectAddress(int obj);
	byte *getObjOrActorName(int obj);
	void clearOwnerOf(int obj);
	void runVERBCode(int script, int entry, int a, int b, int16 *vars);
	void unkSoundProc1(int a);
	void setVerbObject(int room, int object, int verb);
	void unkMessage1();
	void unkMessage2();
	void actorTalk();

	byte *addMessageToStack(byte *msg);

	void unkAddMsgToStack2(int var);
	void unkAddMsgToStack3(int var);
	void unkAddMsgToStack4(int var);
	void unkAddMsgToStack5(int var);

	byte *getActorName(Actor *a);
	uint32 getOBCDOffs(int object);

	byte isMaskActiveAt(int l, int t, int r, int b, byte *mem);

	int getInventorySlot();


	int getKeyInput(int a);
	void convertKeysToClicks();

	OpcodeProc getOpcode(int i);

	void drawBox(int x, int y, int x2, int y2, int color);

	void drawMouse();

	void GDI_drawMouse();
	void GDI_removeMouse();

	void dumpResource(char *tag, int index, byte *ptr);

	FILE *_saveLoadStream;
	bool _saveOrLoad;
	bool saveState(const char *filename);
	bool loadState(const char *filename);
	void saveOrLoad(FILE *inout, bool mode);
	void saveLoadBytes(void *b, int len);

	Actor *derefActor(int id) { return &actor[id]; }
	Actor *derefActorSafe(int id, const char *errmsg);
	Actor *getFirstActor() { return actor; } 

	void setupCostumeRenderer(CostumeRenderer *c, Actor *a);

	PathVertex *unkMatrixProc1(PathVertex *vtx, PathNode *node);
	PathNode *unkMatrixProc2(PathVertex *vtx, int i);
	bool areBoxesNeighbours(int i, int j);
	void addToBoxMatrix(byte b);
	PathVertex *addPathVertex();
	void *addToBoxVertexHeap(int size);
};

void waitForTimer(Scumm *s);
void outputdisplay2(Scumm *s, int disp);
extern const byte revBitMask[8];
void blitToScreen(Scumm *s, byte *src, int x, int y, int w, int h);

void NORETURN CDECL error(const char *s, ...);
void CDECL warning(const char *s, ...);
void CDECL debug(int level, const char *s, ...);
void checkHeap();

void updateScreen(Scumm *s);
