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
 * $Header$
 *
 */

#include "scummsys.h"

#define SCUMMVM_VERSION "0.1.0 devel"

#define SWAP(a,b) do{int tmp=a; a=b; b=tmp; } while(0)
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))

struct Scumm;
struct Actor;

#include "smush.h"

typedef void (Scumm::*OpcodeProc)();

/* System Wide Constants */
enum {
	SAMPLES_PER_SEC =  22050,
	BITS_PER_SAMPLE = 16,
	NUM_MIXER = 4,
	NUM_SCRIPT_SLOT = 25,
	NUM_LOCALSCRIPT = 60,
	NUM_SHADOW_PALETTE = 8,
	NUM_ACTORS = 30
};

const uint16 many_direction_tab[18] = {
	4,
	8,
	71,
	109,
	251,
	530,
	0,
	0,
	0,
	0,
	22,
	72,
	107,
	157,
	202,
	252,
	287,
	337 };

const int16 many_direction_tab_2 [16] = {
	0,
	90,
	180,
	270,
	-1,
	-1,
	-1,
	-1,
	0,
	45,
	90,
	135,
	180,
	225,
	270,
	315 };

struct Point {
	int x,y;
};

struct MemBlkHeader {
	uint32 size;
};


#pragma START_PACK_STRUCTS
	
#define SIZEOF_BOX 20
struct Box { /* file format */
	int16 ulx,uly;
	int16 urx,ury;
	int16 llx,lly;
	int16 lrx,lry;
	byte mask;
	byte flags;
	uint16 scale;
} GCC_PACK;

struct ResHdr {
	uint32 tag, size;
} GCC_PACK;

#define RES_DATA(x) (((byte*)x) + sizeof(ResHdr))
#define RES_SIZE(x) ( READ_BE_UINT32(&((ResHdr*)x)->size) )


struct RoomHeader {
	union {
		struct {
			uint32 version;
			uint16 width,height;
			uint16 numObjects;
		} v7;
		struct {
			uint16 width,height;
			uint16 numObjects;
		} old;
	};
} GCC_PACK;

struct BompHeader {
	uint16 unk;
	uint16 width,height;
} GCC_PACK;

struct AkosHeader {
	byte x_1[2];
	byte flags;
	byte x_2;
	uint16 num_anims;
	uint16 x_3;
	uint16 codec;
} GCC_PACK;

struct AkosOffset {
	uint32 akcd;
	uint16 akci;
} GCC_PACK;

struct AkosCI {
	uint16 width,height;
	int16 rel_x, rel_y;
	int16 move_x, move_y;
} GCC_PACK;

struct CodeHeader {
	union {
		struct {
			uint16 obj_id;
			byte x,y,w,h;
			byte flags;
			byte parent;
			uint16 walk_x;
			uint16 walk_y;
			byte actordir;
		} v5;

		struct {
			uint16 obj_id;
			int16 x, y;
			uint16 w,h;
			byte flags, parent;
			uint16 unk1;
			uint16 unk2;
			byte actordir;
		} v6;

		struct {
			uint32 version;
			uint16 obj_id;
			byte parent;
			byte parentstate;
		} v7;
							
	};
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
				int16 x,y;
			} hotspot[15];
		} old;

		struct {
			uint32 version;
			uint16 obj_id;
			uint16 unk[1];
			int16 x_pos,y_pos;
			uint16 width,height;
			byte unk2[3];
			byte actordir;
			uint16 unk_2;
			struct {
				int16 x,y;
			} hotspot[15];
		} v7;
	};
} GCC_PACK;
#pragma END_PACK_STRUCTS

struct AdjustBoxResult {
	int16 x,y;
	uint16 dist;
};

struct VerbSlot {
	int16 x,y;
	int16 right, bottom;
	int16 oldleft, oldtop, oldright,oldbottom;
	uint8 verbid;
	uint8 color,hicolor,dimcolor,bkcolor,type;
	uint8 charset_nr,curmode;
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
	uint16 walk_x, walk_y;
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

struct CostumeData {
	byte active[16];
	uint16 animCounter1;
	byte animCounter2;
	uint16 stopped;
	uint16 curpos[16];
	uint16 start[16];
	uint16 end[16];
	uint16 frame[16];
};

struct EnqueuedObject {
	uint16 a,b,c,d,e;
	int16 x,y;
	uint16 width,height;
	uint16 j,k,l;
};

struct PathNode {
	uint index;
	struct PathNode *left, *right;
};

struct PathVertex {
	PathNode *left;
	PathNode *right;
};

struct VirtScreen {
	int number;
	uint16 unk1;
	uint16 topline;
	uint16 width,height;
	uint16 size;
	byte alloctwobuffers;
	byte scrollable;
	uint16 xstart;
	byte tdirty[40];
	byte bdirty[40];
};

struct ActorWalkData {
	int16 destx,desty;
	byte destbox;
	int16 destdir;
	byte curbox;
	int16 x,y,newx,newy;
	int32 XYFactor, YXFactor;
	uint16 xfrac,yfrac;
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
	uint16 newfield;
	byte status;
	byte where;
	byte unk1,unk2,freezeCount,didexec;
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
	rtLast = 16,
	rtNumTypes = 17,
};

enum {
	OF_OWNER_MASK = 0x0F,
	OF_STATE_MASK = 0xF0,
	
	OF_STATE_SHL = 4
};

/* Camera Modes */
enum {
	CM_NORMAL = 1,
	CM_FOLLOW_ACTOR = 2,
	CM_PANNING = 3,
};

enum {
	MBS_LEFT_CLICK = 0x8000,
	MBS_RIGHT_CLICK = 0x4000,
	MBS_MOUSE_MASK = (MBS_LEFT_CLICK|MBS_RIGHT_CLICK),
	MBS_MAX_KEY	= 0x0200
};

enum {
	RF_LOCK = 0x80,
	RF_USAGE = 0x7F,
	RF_USAGE_MAX = RF_USAGE
};

enum MoveFlags {
	MF_NEW_LEG = 1,
	MF_IN_LEG = 2,
	MF_TURN = 4,
	MF_LAST_LEG = 8,
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

	byte *_backbuff_ptr, *_bgbak_ptr;
	byte *_mask_ptr;
	byte *_dest_ptr;
		
	byte _colorMap[16];
	byte _buffer[512];

	void drawBits();
	void printChar(int chr);
	void printCharOld(int chr);
	int getStringWidth(int a, byte *str, int pos);
	void addLinebreaks(int a, byte *str, int pos, int maxwidth);
};

struct AkosRenderer {
	CostumeData *cd;
	int x,y; /* where to draw costume */
	byte scale_x, scale_y; /* scaling */
	byte clipping; /* clip mask */
	bool charsetmask;
	byte shadow_mode;
	uint16 codec;
	bool mirror; /* draw actor mirrored */
	byte dirty_id;
	byte *outptr;
	uint outwidth, outheight;
	
	/* pointer to various parts of the costume resource */
	byte *akos;
	AkosHeader *akhd;
	
	/* current move offset */
	int move_x, move_y;
	/* movement of cel to decode */
	int move_x_cur, move_y_cur;
	/* width and height of cel to decode */
	int width,height;
	
	byte *srcptr;
	byte *shadow_table;

	struct {
		/* codec stuff */
		const byte *scaletable;
		byte mask,shl;
		bool doContinue;
		byte repcolor;
		byte replen;
		int scaleXstep;
		int x,y;
		int tmp_x, tmp_y;
		int y_pitch;
		int skip_width;
		byte *destptr;
		byte *mask_ptr;
		int imgbufoffs;
	} v1;

	/* put less used stuff at the bottom to optimize opcodes */
	int draw_top, draw_bottom;
	byte *akpl,*akci,*aksq;
	AkosOffset *akof;
	byte *akcd;

	byte palette[256];
};

struct BompDrawData {
	byte *out;
	int outwidth, outheight;
	int x,y;
	byte scale_x, scale_y;
	byte *dataptr;
	int srcwidth, srcheight;
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
	byte _scaleIndexX; /* must wrap at 256*/
	byte _scaleIndexY, _scaleIndexYTop;
	int _left,_right;
	int _dir2;
	int _top,_bottom;
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
	void proc_special(byte code);
	byte mainRoutine(Actor *a, int slot, int frame);
	void ignorePakCols(int num);

	byte drawOneSlot(Actor *a, int slot);
	byte drawCostume(Actor *a);

};

struct Actor {
	int x,y,top,bottom;
	int elevation;
	uint width;
	byte number;
	uint16 facing;
	uint16 costume;
	byte room;
	byte talkColor;
	byte scalex,scaley;
	byte charset;
	int16 newDirection;
	byte moving;
	byte ignoreBoxes;
	byte forceClip;
	byte initFrame,walkFrame,standFrame,talkFrame1,talkFrame2;
	bool needRedraw, needBgReset,costumeNeedsInit,visible;
	byte shadow_mode;
	bool flip;
	uint speedx,speedy;
	byte frame;
	byte walkbox;
	byte mask;
	byte animProgress, animSpeed;
	int16 new_1,new_2;
	uint16 talk_script, walk_script;
	byte new_3;
	int8 layer;
	ActorWalkData walkdata;
	int16 animVariable[16];
	uint16 sound[8];
	CostumeData cost;
	byte palette[64];
};

struct CameraData {
	Point _cur;
	Point _dest;
	Point _accel;
	Point _last;
	int _leftTrigger, _rightTrigger;
	byte _follows, _mode;
	bool _movingToActor;
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
	bool no_talk_anim,t_no_talk_anim;
};

struct ColorCycle {
	uint16 delay;
	uint16 counter;
	uint16 flags;
	byte start;
	byte end;
};

struct Gdi {
	Scumm *_vm;

	byte *_readPtr;
	uint _readOffs;

	int8 _cursorActive;

	int _numZBuffer;
	int _imgBufOffs[4];
	byte _disable_zbuffer;

	bool _useOrDecompress;
	byte _numLinesToProcess;
	byte _tempNumLines;
	byte _currentX;
	byte _hotspot_x;
	byte _hotspot_y;
	int16 _drawMouseX;
	int16 _drawMouseY;
	int16 _mask_top, _mask_bottom, _mask_right, _mask_left;
	byte _currentCursor;
	byte _mouseColors[4];
	byte _mouseColor;
	byte _mouseClipMask1, _mouseClipMask2, _mouseClipMask3;
	byte _mouseColorIndex;
	byte *_mouseMaskPtr;
	byte *_smap_ptr;
	byte *_backbuff_ptr;
	byte *_bgbak_ptr;
	byte *_mask_ptr;
	byte *_mask_ptr_dest;
	byte *_z_plane_ptr;

	byte _decomp_shr, _decomp_mask;
	byte _transparency;
	uint16 _vertStripNextInc;
	byte *_backupIsWhere;
	
	void unkDecode1();
	void unkDecode2();
	void unkDecode3();
	void unkDecode4();
	void unkDecode5();
	void unkDecode6();
	void unkDecode7();
        void unkDecode8();
        void unkDecode9();
        void unkDecode10();
        void unkDecode11();

	void decompressBitmap();

	void drawBitmap(byte *ptr, VirtScreen *vs, int x, int y, int h, int stripnr, int numstrip, byte flag);
	void clearUpperMask();

	void disableZBuffer() { _disable_zbuffer++; }
	void enableZBuffer() { _disable_zbuffer--; }

	void draw8ColWithMasking();
	void clear8ColWithMasking();
	void clear8Col();
	void decompressMaskImgOr();
	void decompressMaskImg();

	void resetBackground(byte top, byte bottom, int strip);
	void drawStripToScreen(VirtScreen *vs, int x, int w, int t, int b);
	void updateDirtyScreen(VirtScreen *vs);


	enum DrawBitmapFlags {
		dbAllowMaskOr = 1,
		dbDrawMaskOnBoth = 2,
		dbClear = 4,
	};
};

struct MixerChannel {
	void *_sfx_sound;
	uint32 _sfx_pos;
	uint32 _sfx_size;
	uint32 _sfx_fp_speed;
	uint32 _sfx_fp_pos;

	void mix(int16 *data, uint32 len);
	void clear();
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
};

struct ScummDebugger;
struct Serializer;

enum WhereIsObject {
	WIO_NOT_FOUND = -1,
	WIO_INVENTORY = 0,
	WIO_ROOM = 1,
	WIO_GLOBAL = 2,
	WIO_LOCAL = 3,
	WIO_FLOBJECT = 4,
};

enum MouseButtonStatus {
	msDown = 1,
	msClicked = 2,
};

struct BoxCoords {
	Point ul;
	Point ur;
	Point ll;
	Point lr;
};

struct Scumm {
	uint32 _features;
	const char *_gameText;
	byte _gameId;
//	byte _majorScummVersion;
//	byte _middleScummVersion;
//	byte _minorScummVersion;
	ScummDebugger *_debugger;
	void *_gui; /* actually a pointer to a Gui */

	byte OF_OWNER_ROOM;
	
        int _gameTempo;
	int _lastLoadedRoom;
	int _roomResource;
	byte _encbyte;
	void *_fileHandle;
	void *_sfxFile;
	char *_exe_name;
	char *_gameDataPath;

	int akos_findManyDirection(int16 ManyDirection, uint16 facing);
	
	byte _saveLoadFlag;
	byte _saveLoadSlot;
	bool _saveLoadCompatible;

	bool _dynamicRoomOffsets;
	byte _resFilePathId;

	bool _soundsPaused;

	bool _useTalkAnims;
	
	char *_resFilePrefix;
	char *_resFilePath;

	int _keyPressed;

	void *_soundEngine;

	uint16 *_inventory;
	byte *_arrays;
	VerbSlot *_verbs;
	ObjectData *_objs;
	uint16 *_newNames;
	int16 *_vars;
	int16 _varwatch;
	byte *_bitVars;

	const OpcodeProc *_opcodes;
	const char* *_opcodes_lookup;

	int _xPos, _yPos;
	int _dir;

	byte _curActor;
	int _curVerb;
	int _curVerbSlot;

	int _curPalIndex;

	VirtScreen *_curVirtScreen;

	
	byte *_scriptPointer, *_scriptOrgPointer;
	byte *_scriptPointerStart;
	byte _opcode;

	int _numVariables;
	int _numBitVariables;
	int _numLocalObjects;
	int _numGlobalObjects;
	int _numArray;
	int _numVerbs;
	int _numFlObject;
	int _numInventory;
	int _numRooms;
	int _numScripts;
	int _numSounds;
	int _numCharsets;
	int _numCostumes;
	int _numNewNames;
	int _numGlobalScripts;

	byte *_msgPtrToAdd;
	
	uint8 *_roomFileIndexes;
	byte *_objectOwnerTable;
	byte *_objectRoomTable;
	byte *_objectStateTable;
	uint32 *_classData;

	byte _expire_counter;

	bool _noTalkAnims;

	bool _mouthSyncMode;
	bool _endOfMouthSync;

	uint32 _randSeed1;
	uint32 _randSeed2;

	uint16 _screenB, _screenH;

	uint16 _defaultTalkDelay;
	byte _haveMsg;
	byte _newEffect;
	bool _fullRedraw;
	uint16 _soundParam,_soundParam2,_soundParam3;
	
	byte _switchRoomEffect2, _switchRoomEffect;

	int _resourceHeaderSize;

	bool _egoPositioned;
	bool _doEffect;
	bool _screenEffectFlag;
	bool _keepText;

	uint32 _maxHeapThreshold;
	uint32 _minHeapThreshold;
	
	bool _fullScreen;

	byte _bkColor;
	uint16 _lastXstart;
		
	int16 _talkDelay;

	bool _shakeEnabled;
	uint _shakeFrame;

	int16 _virtual_mouse_x, _virtual_mouse_y;

	int _cursorHotspotX, _cursorHotspotY;
	int _cursorWidth, _cursorHeight;
	byte _cursorAnimateIndex;
	byte _cursorAnimate;

	byte _charsetColor;

	uint16 _debugMode;

	uint16 _noSubtitles;  // skip all subtitles?
	unsigned int _scale;  // multiplier to resolution (2 is default)

	byte *_messagePtr;

	byte _numNestedScripts;
	byte _currentScript;

	byte _currentRoom;
	byte _numObjectsInRoom;
	byte _actorToPrintStrFor;

	int _screenStartStrip;
	int _screenEndStrip;

	int _screenLeft;
	int _screenTop;

	byte _fastMode;
	
	bool _completeScreenRedraw;

	int8 _userPut;
	int8 _cursorState;

	byte _sfxMode;

	uint16 _mouseButStat;
	byte _leftBtnPressed, _rightBtnPressed;

	int _numInMsgStack;

	uint32 _localScriptList[NUM_LOCALSCRIPT];

	VirtScreen virtscr[4];

	uint32 _ENCD_offs, _EXCD_offs;
	uint32 _CLUT_offs, _EPAL_offs;
	uint32 _IM00_offs;
	uint32 _PALS_offs;

	uint32 _allocatedSize;

	uint32 _talk_sound_a, _talk_sound_b;
	byte _talk_sound_mode;

	int _drawObjectQueNr;
	byte _drawObjectQue[200];

	uint16 _currentDrive;
	uint16 _soundCardType;
	byte _mousePresent;

	int16 _palManipStart;
	int16 _palManipEnd;
	int16 _palManipCounter;

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
		int16 x,y;
	} mouse;

	Actor actor[NUM_ACTORS];

	uint32 gfxUsageBits[200];

	CharsetRenderer charset;
	
	byte _charsetData[10][16];

	byte _resourceMapper[128];

	uint16 _extraBoxFlags[65];

	byte **_lastCodePtr;

	byte *_shadowPalette;
	int _shadowPaletteSize;
	
//	int _numSoundTags;
//	byte *_soundTagTable;

	int16 _bootParam;

	uint32 _fileOffset;

	byte _fileReadFailed;
	byte _fileMode;

	uint32 _whereInResToRead;

	CameraData camera;

	int _resultVarNumber;

	int _sentenceNum;
	SentenceTab sentence[6];

	StringTab string[6];

	uint16 _mouthSyncTimes[52];

	int16 _soundQuePos;
	int16 _soundQue[0x100];

	uint16 _enqueue_b,_enqueue_c,_enqueue_d,_enqueue_e;

	int _enqueuePos; 
	EnqueuedObject _enqueuedObjects[32];

	byte _soundQue2Pos;
	byte _soundQue2[10];

	int16 _vararg_temp_pos[16];

	uint16 _curExecScript;

	int _scrWidth;
	int _scrHeight;

	byte _currentPalette[0x300];

	int _palDirtyMin, _palDirtyMax;

	uint _curSoundPos;

	ColorCycle _colorCycle[16];

	Gdi gdi;
	
	bool _BgNeedsRedraw;

	int16 _localParamList[16];

	uint16 _verbMouseOver;

	int16 _foundPathX;
	int16 _foundPathY;

	uint16 _lastKeyHit;

	int _scummStackPos;
	int16 _scummStack[150];

	int _maxBoxVertexHeap;
	byte *_boxMatrixPtr4, *_boxMatrixPtr1, *_boxMatrixPtr3;
	int _boxPathVertexHeapIndex;
	int _boxMatrixItem;

	byte _grabbedCursor[2048];

	char _saveLoadName[32];

	MixerChannel _mixer_channel[NUM_MIXER];

	OpcodeProc getOpcode(int i) { return _opcodes[i]; }

	void openRoom(int room);
	void deleteRoomOffsets();
	void readRoomsOffsets();
	void askForDisk(const char *filename);

	bool openResourceFile(const char *filename);
	
	void fileClose(void *file);
	void *fileOpen(const char *filename, int mode);
	void fileSeek(void *file, long offs, int whence);
	void fileRead(void *handle, void *ptr, uint32 size);
	bool fileEof(void *handle);
	uint32 filePos(void *handle);

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

	static byte *alloc(int size);
	static void free(void *mem);

	void readResTypeList(int id, uint32 tag, const char *name);
	void allocResTypeData(int id, uint32 tag, int num, const char *name, int mode);

	void initRandSeeds();

	uint getRandomNumber(uint max);
	uint getRandomNumberRng(uint min, uint max);

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
	byte *getStringAddress(int i);
	void ensureResourceLoaded(int type, int i);
	int loadResource(int type, int i);
	int getResourceRoomNr(int type, int index);
	int readSoundResource(int type, int index);
	void setResourceCounter(int type, int index, byte flag);
	void validateResource(const char *str, int type, int index);
	
	void initVirtScreen(int slot, int top, int height, bool twobufs, bool fourextra);
	void setDirtyRange(int slot, int a, int height);
	void drawDirtyScreenParts();
	void updateDirtyScreen(int slot);
	void unkVirtScreen4(int a);
	
	void restoreMouse();
	void initActor(Actor *a, int mode);
	bool checkFixedDisk();

	void setActorWalkSpeed(Actor *a, uint speed1, uint speed2);
	int calcMovementFactor(Actor *a, int newx, int newy);
	int actorWalkStep(Actor *a);
	int remapDirection(Actor *a, int dir);

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
	int getObjectRoom(int obj);
	void SamInventoryHack(int obj);	// FIXME: Sam and Max hack

	void main();

	uint distanceFromPt(int x, int y, int ptx, int pty);
	Point closestPtOnLine(int ulx, int uly, int llx, int lly, int x, int y);
	bool getSideOfLine(int x1,int y1, int x2, int y2, int x, int y, int box);
	void getBoxCoordinates(int boxnum, BoxCoords *bc);
	byte getMaskFromBox(int box);
	Box *getBoxBaseAddr(int box);
	byte getBoxFlags(int box);
	int getBoxScale(int box);
	byte getNumBoxes();
	byte *getBoxMatrixBaseAddr();

//	void startAnimActor(Actor *a, int frame);
	void startAnimActor(Actor *a, int frame);
	void startAnimActorEx(Actor *a, int frame, int direction);
//	void startAnimActor(Actor *a, int frame, byte direction);
	int getProgrDirChange(Actor *a, int mode);
	void initActorCostumeData(Actor *a);
	void fixActorDirection(Actor *a, int direction);
	void cpst_decodeData(Actor *a, int frame, uint mask);

	void scummInit();
	void scummMain(int argc, char **argv);

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
	void ignoreScriptWord() { fetchScriptWord(); }
	void ignoreScriptByte() { fetchScriptByte(); }
	int getVarOrDirectWord(byte mask);
	int getVarOrDirectByte(byte mask);
	int readVar(uint var);
	void writeVar(uint var, int value);
	void getResultPos();
	void setResult(int result);
	
	int getObjectIndex(int object);

	void o5_actorFollowCamera();
	void o5_actorFromPos();
	void o5_actorSet();
	void o5_actorSetClass();
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

	int popRoomAndObj(int *room);

	void soundKludge(int16 *list);

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
	bool isScriptInUse(int script);
	int getActorXYPos(Actor *a);
	void getObjectXYPos(int object);
	AdjustBoxResult adjustXYToBeInBox(Actor *a, int x, int y, int pathfrom);

	int getWordVararg(int16 *ptr);

	int getObjActToObjActDist(int a, int b);
	void processSoundQues();
	bool inBoxQuickReject(int box, int x, int y, int threshold);
	AdjustBoxResult getClosestPtOnBox(int box, int x, int y);

	void setCameraAt(int pos_x, int pos_y);
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

	void increaseResourceCounter();
	bool isResourceInUse(int type, int i);
	void initRoomSubBlocks();
	void loadRoomObjects();
        void loadRoomObjectsSmall();

	void setPaletteFromRes();
	void initCycl(byte *ptr);
	void initBGBuffers();
	void setDirtyColors(int min, int max);

	void setScaleItem(int slot, int a, int b, int c, int d);

	void cyclePalette();

	void moveMemInPalRes(int start, int end, byte direction);

	void redrawBGAreas();
	void drawRoomObjects(int arg);
	void drawRoomObject(int i, int arg);
	void redrawBGStrip(int start, int num);
	void drawObject(int obj, int arg);

	int hasCharsetMask(int x, int y, int x2, int y2);

	void restoreBG(int left, int top, int right, int bottom);
	void updateDirtyRect(int virt, int left, int right, int top, int bottom, uint32 dirtybits);
	VirtScreen *findVirtScreen(int y);

	void unkScreenEffect1();
	void unkScreenEffect2();
	void unkScreenEffect3();
	void unkScreenEffect4();
	void unkScreenEffect5(int a);
	void unkScreenEffect6();
	void unkScreenEffect7(int a);
	
	void playSound(int sound);

	void decreaseScriptDelay(int amount);
	void processKbd();
	
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

	int getPathToDestBox(byte from, byte to);
	int findPathTowards(Actor *a, byte box, byte box2, byte box3);

	void drawActorCostume(Actor *a);
	void actorAnimate(Actor *a);

	int getActorFromPos(int x, int y);

	void restoreVerbBG(int verb);

	void drawString(int a);
	void drawVerbBitmap(int vrb, int x, int y);

	void setActorCostume(Actor *a, int c);
	void loadPtrToResource(int type, int i, byte *ptr);

	void push(int a);
	int pop();

	void walkActorTo(Actor *a, int x, int y, int direction);

	void setCursorImg(uint img, uint room, uint imgindex);
//	void setCursorHotspot(int cursor, int x, int y);
	void initCharset(int charset);
	void addObjectToDrawQue(int object);
	int getVerbEntrypoint(int obj, int entry);
	int isSoundRunning(int a);
	void startWalkActor(Actor *a, int x, int y, int dir);
	void setBoxFlags(int box, int val);
	void setBoxScale(int box, int b);
	void createBoxMatrix();
	void addObjectToInventory(uint obj, uint room);
	void removeObjectFromRoom(int obj);
	void decodeParseString();
	void pauseGame(bool user);
	void shutDown(int i);
	void lock(int type, int i);
	void unlock(int type, int i);
	void heapClear(int mode);
	void unkHeapProc2(int a, int b);
	void loadFlObject(uint object, uint room);
	void setPalColor(int index, int r, int g, int b);
	void darkenPalette(int a, int b, int c, int d, int e);
	void unkRoomFunc3(int a, int b, int c, int d, int e);
	void unkRoomFunc4(int a, int b, int c, int d, int e);
	int getVerbSlot(int id, int mode);
	void killVerb(int slot);
	byte *getOBCDFromObject(int obj);
	byte *getObjOrActorName(int obj);
	void clearOwnerOf(int obj);
	void runVerbCode(int script, int entry, int a, int b, int16 *vars);
	void setVerbObject(uint room, uint object, uint verb);
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

	void drawBox(int x, int y, int x2, int y2, int color);

	void drawMouse();

	void dumpResource(char *tag, int index, byte *ptr);

	bool saveState(int slot, bool compat);
	bool loadState(int slot, bool compat);
	void saveOrLoad(Serializer *s);

	void saveLoadResource(Serializer *ser, int type, int index);
	bool isResourceLoaded(int type, int index);


	Actor *derefActor(int id) { return &actor[id]; }
	Actor *derefActorSafe(int id, const char *errmsg);
	Actor *getFirstActor() { return actor; } 

	PathVertex *unkMatrixProc1(PathVertex *vtx, PathNode *node);
	PathNode *unkMatrixProc2(PathVertex *vtx, int i);
	bool areBoxesNeighbours(int i, int j);
	void addToBoxMatrix(byte b);
	PathVertex *addPathVertex();
	void *addToBoxVertexHeap(int size);

	void parseCommandLine(int argc, char **argv);

	void showHelpAndExit();
	
	char *getGameName();
	bool detectGame();

	void setupOpcodes();
	void setupOpcodes2();
	void endCutscene();
	void cutscene(int16 *args);

	void setOwnerOf(int obj, int owner);
	void panCameraTo(int x, int y);
	void actorFollowCamera(int act);
	void setCameraAtEx(int at);

	void clampCameraPos(Point *pt);

	void setCursorHotspot2(int x,int y);

	void makeCursorColorTransparent(int a);

	void faceActorToObj(int act, int obj);
	void animateActor(int act, int anim);
	bool isScriptRunning(int script);
	bool isRoomScriptRunning(int script);
	int getObjX(int obj);
	int getObjY(int obj);
	int getObjOldDir(int obj);
	int getObjNewDir(int obj);
	int findInventory(int owner, int index);
	int getInventoryCount(int owner);

	void beginOverride();
	void endOverride();

	void setPalette(int pal);
	void setPaletteFromPtr(byte *ptr);
	byte *findPalInPals(byte *pal, int index);

	int getStringLen(byte *ptr);

	void readArrayFromIndexFile();
	void readMAXS();
        void readIndexFile();
        void readIndexFileSmall();

	int readArray(int array, int index, int base);
	void writeArray(int array, int index, int base, int value);
	
	int getStackList(int16 *args, uint maxnum);
	void setObjectState(int obj, int state, int x, int y);

	void setStringVars(int i);
	void decodeParseString2(int a, int b);

	void arrayop_1(int a, byte *ptr);
	
	void copyString(byte *dst, byte *src, int len);
	int getArrayId();

	void nukeArray(int a);
	int defineArray(int a, int b, int c, int d);
	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);
	void grabCursor(int x, int y, int w, int h);
	void unkMiscOp9();
	void startManiac();

	void grabCursor(byte *ptr, int width, int height);
	byte *getPalettePtr();
	void setupSound();
	void stopAllSounds();
	void stopSound(int sound);
	bool isSoundInQueue(int sound);
	void clearSoundQue();
	void talkSound(uint32 a, uint32 b, int mode);
	void processSfxQueues();
	void startTalkSound(uint32 a, uint32 b, int mode);
	void stopTalkSound();
	bool isMouthSyncOff(uint pos);
	void startSfxSound(void *file);
	void *openSfxFile();
	void resourceStats();
	bool isCostumeInUse(int i);
	void expireResources(uint32 size);
	
	void freeResources();
	void destroy();

	void useIm01Cursor(byte *im, int w, int h);
	void useBompCursor(byte *im, int w, int h);

	void decompressBomp(byte *dst, byte *src, int w, int h);

	void setupCursor() { _cursorAnimate = 1; }

	void decompressDefaultCursor(int index);

	void allocateArrays();

	void initializeLocals(int slot, int16 *vars);

	static void setVirtscreenDirty(VirtScreen *vs, int left, int top, int right, int bottom);
	int scummLoop(int delta);

	bool getSavegameName(int slot, char *desc);
	void makeSavegameName(char *out, int slot, bool compatible);

	void exitCutscene();
	void nukeFlObjects(int min, int max);

	void swapPalColors(int a, int b);

	void enqueueObject(int a, int b, int c, int d, int e, int f, int g, int h, int mode);

	void clearEnqueue() { _enqueuePos = 0; }
	void drawEnqueuedObjects();
	void drawEnqueuedObject(EnqueuedObject *eo);
	void removeEnqueuedObjects();
	void removeEnqueuedObject(EnqueuedObject *eo);

	void pauseSounds(bool pause);

	MixerChannel *allocateMixer();
	bool isSfxFinished();
	void playSfxSound(void *sound, uint32 size, uint rate);
	void stopSfxSound();

	void mixWaves(int16 *sounds, int len);

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
		foCheckAlreadyLoaded = 4,
	};

	void findObjectInRoom(FindObjectInRoom *fo, byte findWhat, uint object, uint room);
	void setupRoomObject(ObjectData *od, byte *room);
	int findFlObjectSlot();

	void runTalkScript(int frame);

	int remapPaletteColor(int r, int g, int b, uint threshold);
	void remapActor(Actor *a, int b, int c, int d, int e);

	byte *findResourceData(uint32 tag, byte *ptr);
	int getResourceDataSize(byte *ptr);

	void akos_decodeData(Actor *a, int frame, uint usemask);
	int akos_frameToAnim(Actor *a, int frame);
	bool akos_hasManyDirections(Actor *a);
	void stopActorMoving(Actor *a);

	int newDirToOldDir(int dir);
	int oldDirToNewDir(int dir);
	void startWalkAnim(Actor *a, int cmd, int angle);
	void setActorBox(Actor *a, int box);
	int getAngleFromPos(int x, int y);
	int updateActorDirection(Actor *a);

	bool akos_drawCostume(AkosRenderer *ar);
	void akos_setPalette(AkosRenderer *ar, byte *palette);
	void akos_setCostume(AkosRenderer *ar, int costume);
	void akos_setFacing(AkosRenderer *ar, Actor *a);
	bool akos_drawCostumeChannel(AkosRenderer *ar, int chan);
	void akos_codec1(AkosRenderer *ar);
	void akos_codec5(AkosRenderer *ar);
	void akos_codec16(AkosRenderer *ar);
	void akos_codec1_ignorePakCols(AkosRenderer *ar, int num);
	void akos_c1_spec2(AkosRenderer *ar);
	void akos_c1_spec3(AkosRenderer *ar);

	void akos_c1_0_decode(AkosRenderer *ar);
	void akos_c1_12_decode(AkosRenderer *ar);
	void akos_c1_12y_decode(AkosRenderer *ar);
	void akos_c1_3_decode(AkosRenderer *ar);
	void akos_c1_4_decode(AkosRenderer *ar);
	void akos_c1_4y_decode(AkosRenderer *ar);
	void akos_c1_56_decode(AkosRenderer *ar);
	void akos_c1_56y_decode(AkosRenderer *ar);
	void akos_c1_7_decode(AkosRenderer *ar);

	bool akos_increaseAnims(byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, byte *aksq, uint16 *akfo, int numakfo);

	int getAnimVar(Actor *a, byte var);
	void setAnimVar(Actor *a, byte var, int value);

	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	bool akos_compare(int a, int b, byte cmd);

	static int normalizeAngle(int angle);
	static int fromSimpleDir(int dirtype, int dir);
	static int toSimpleDir(int dirtype, int dir);
	static int numSimpleDirDirections(int dirType);

	void doSentence(int c, int b, int a);
	int cost_frameToAnim(Actor *a, int frame);

	void setupShadowPalette(int slot,int rfact,int gfact,int bfact,int from,int to);

	void drawBomp(BompDrawData *bd);
	void loadCostume(LoadedCostume *lc, int costume);

	void cost_setPalette(CostumeRenderer *cr, byte *palette);
	void cost_setFacing(CostumeRenderer *cr, Actor *a);
	void cost_setCostume(CostumeRenderer *cr, int costume);
	byte cost_increaseAnims(LoadedCostume *lc, Actor *a);
	byte cost_increaseAnim(LoadedCostume *lc, Actor *a, int slot);
	void cost_decodeData(Actor *a, int frame, uint usemask);

	void redrawLines(int from, int to);

	void setupScummVarsOld();
	void setupScummVarsNew();

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

	byte VAR_LEFTBTN_DOWN;
	byte VAR_RIGHTBTN_DOWN;
	byte VAR_LEFTBTN_HOLD;
	byte VAR_RIGHTBTN_HOLD;

	byte VAR_UNK_SCRIPT;
	byte VAR_UNK_SCRIPT_2;
	
	byte VAR_DEFAULT_TALK_DELAY;
	byte VAR_CHARSET_MASK;
	
	byte _videoPath[50];
};

enum AkosOpcodes{
	AKC_Return  = 0xC001,
	AKC_SetVar  = 0xC010,
	AKC_CmdQue3  = 0xC015,
	AKC_ComplexChan  = 0xC020,
	AKC_Jump  = 0xC030,
	AKC_JumpIfSet  = 0xC031,
	AKC_AddVar  = 0xC040,
	AKC_Ignore  = 0xC050,
	AKC_IncVar  = 0xC060,
	AKC_CmdQue3Quick  = 0xC061,
	AKC_SkipStart = 0xC070,
	AKC_SkipE  = 0xC070,
	AKC_SkipNE  = 0xC071,
	AKC_SkipL  = 0xC072,
	AKC_SkipLE  = 0xC073,
	AKC_SkipG  = 0xC074,
	AKC_SkipGE  = 0xC075,
	AKC_StartAnim  = 0xC080,
	AKC_StartVarAnim  = 0xC081,
	AKC_Random  = 0xC082,
	AKC_SetActorClip  = 0xC083,
	AKC_StartAnimInActor  = 0xC084,
	AKC_SetVarInActor  = 0xC085,
	AKC_HideActor  = 0xC086,
	AKC_SetDrawOffs  = 0xC087,
	AKC_JumpTable  = 0xC088,
	AKC_SoundStuff  = 0xC089,
	AKC_Flip  = 0xC08A,
	AKC_Cmd3  = 0xC08B,
	AKC_Ignore3  = 0xC08C,
	AKC_Ignore2  = 0xC08D,
	AKC_JumpStart = 0xC090,
	AKC_JumpE  = 0xC090,
	AKC_JumpNE  = 0xC091,
	AKC_JumpL  = 0xC092,
	AKC_JumpLE  = 0xC093,
	AKC_JumpG  = 0xC094,
	AKC_JumpGE  = 0xC095,
	AKC_ClearFlag  = 0xC09F,
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

struct SaveLoadEntry {
	uint16 offs;
	uint8 type;
	uint8 size;
};

typedef int SerializerSaveReference(void *me, byte type, void *ref);
typedef void *SerializerLoadReference(void *me, byte type, int ref);


struct SerializerStream {
#ifdef NONSTANDARD_SAVE
	void *context;

	bool fopen(const char *filename, const char *mode);
	void fclose();
	int fread(void *buf, int size, int cnt);
	int fwrite(void *buf, int size, int cnt);
#else
	FILE *out;

	FILE *fopen(const char *filename, const char *mode) {
		return out = ::fopen(filename, mode);
	}
	void fclose() {
		::fclose(out);
	}
	int fread(void *buf, int size, int cnt) {
		return ::fread(buf, size, cnt, out);
	}
	int fwrite(void *buf, int size, int cnt) {
		return ::fwrite(buf, size, cnt, out);
	}
#endif
};

struct Serializer {
	SerializerStream _saveLoadStream;

	union {
		SerializerSaveReference *_save_ref;
		SerializerLoadReference *_load_ref;
		void *_saveload_ref;
	};
	void *_ref_me;

	bool _saveOrLoad;

	void saveLoadBytes(void *b, int len);
	void saveLoadArrayOf(void *b, int len, int datasize, byte filetype);
	void saveLoadEntries(void *d, const SaveLoadEntry *sle);
	void saveLoadArrayOf(void *b, int num, int datasize, const SaveLoadEntry *sle);

	void saveUint32(uint32 d);
	void saveWord(uint16 d);
	void saveByte(byte b);

	byte loadByte();
	uint16 loadWord();
	uint32 loadUint32();

	bool isSaving() { return _saveOrLoad; }

};

extern const uint32 IMxx_tags[];
extern const byte default_scale_table[768];

void outputdisplay2(Scumm *s, int disp);
extern const byte revBitMask[8];
void blitToScreen(Scumm *s, byte *src, int x, int y, int w, int h);

#if defined(__GNUC__)
void CDECL error(const char *s, ...) NORETURN;
#else
void CDECL NORETURN error(const char *s, ...);
#endif

void CDECL warning(const char *s, ...);
void CDECL debug(int level, const char *s, ...);
void checkHeap();
void initGraphics(Scumm *s, bool fullScreen, unsigned int scaleFactor = 2);
void updateScreen(Scumm *s);
void drawMouse(Scumm *s, int x, int y, int color, byte *mask, bool visible);
void drawMouse(Scumm *s, int x, int y, int w, int h, byte *buf, bool visible);
void blit(byte *dst, byte *src, int w, int h);
byte *findResource(uint32 tag, byte *searchin, int index);
byte *findResourceSmall(uint32 tag, byte *searchin, int index);
byte *findResource(uint32 tag, byte *searchin);
byte *findResourceSmall(uint32 tag, byte *searchin);
void playSfxSound(void *sound, uint32 size, uint rate);
bool isSfxFinished();
void waitForTimer(Scumm *s, int msec_delay);
void setShakePos(Scumm *s, int shake_pos);
uint16 newTag2Old(uint32 oldTag);
void cd_playtrack(int track, int offset, int delay);
