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

#ifndef SCUMM_HE_INTERN_HE_H
#define SCUMM_HE_INTERN_HE_H

#include "scumm/scumm_v6.h"
#ifdef ENABLE_HE
#include "scumm/he/wiz_he.h"
#endif
#include "scumm/actor_he.h"	// For HEEraseAuxEntry & HEAnimAuxEntry

namespace Common {
class SeekableReadStream;
class SeekableWriteStream;
}

namespace Scumm {

#define MAGIC_ARRAY_NUMBER 0x33539000

class ActorHE;
class ResExtractor;
#ifdef ENABLE_HE
class LogicHE;
class MoviePlayer;
class Sprite;
class CUP_Player;

class Moonbase;
class Basketball;
#endif

class ScummEngine_v60he : public ScummEngine_v6 {
#ifdef ENABLE_HE
	friend class Moonbase;
	friend class Basketball;
#endif
protected:
	enum SubOpType {
		SO_ACTOR_DEFAULT_CLIPPED = 30,
		SO_BACKGROUND_ON = 218,
		SO_BACKGROUND_OFF = 219,
		SO_ROOM_COPY_PALETTE = 220,
		SO_ROOM_SAVEGAME_BY_NAME = 221,
		SO_SOUND_START_VOLUME = 222,
		SO_SOUND_VOLUME_RAMP = 223,
		SO_SOUND_FREQUENCY = 224,
		SO_TALKIE = 225,
		SO_OBJECT_ORDER = 234,
		SO_ROOM_PALETTE_IN_ROOM = 236,
		SO_COLOR_LIST = 249,
	};

public:
	Common::SeekableReadStream *_hInFileTable[17];
	Common::SeekableWriteStream *_hOutFileTable[17];

	Common::Rect _defaultActorClipping;	// HE specific

	int _heTimers[16];
	uint32 _pauseStartTime = 0;

	int getHETimer(int timer);
	void setHETimer(int timer);
	void pauseHETimers(bool pause);

#ifdef ENABLE_HE
public:
	Moonbase *_moonbase;
	Basketball *_basketball;
#endif

public:
	ScummEngine_v60he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v60he() override;

	Common::Path generateFilename(const int room) const override;
	void setActorClippingRect(int actor, int x1, int y1, int x2, int y2);
	bool actorsOverlapInStrip(int actorA, int actorB, int stripNumber);

	void resetScumm() override;

protected:
	void setupOpcodes() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void localizeArray(int slot, byte scriptSlot);
	void redimArray(int arrayId, int newX, int newY, int d);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int resID);

	int virtScreenSave(byte *dst, int x1, int y1, int x2, int y2);
	void virtScreenLoad(int resIdx, int x1, int y1, int x2, int y2);

	void decodeParseString(int a, int b) override;
	void swapObjects(int object1, int object2);

	Common::Path convertFilePath(const byte *src);
	Common::String convertSavePath(const byte *src);
	Common::String convertSavePathOld(const byte *src);

	Common::SeekableReadStream *openFileForReading(const byte *fileName);
	Common::SeekableReadStream *openSaveFileForReading(const byte *fileName);
	Common::SeekableWriteStream *openSaveFileForWriting(const byte *fileName);
	Common::SeekableWriteStream *openSaveFileForAppending(const byte *fileName);
	void deleteSaveFile(const byte *fileName);
	void renameSaveFile(const byte *from, const byte *to);
	void pauseEngineIntern(bool pause) override;

	Common::SeekableReadStream *openSaveFileForReading(int slot, bool compat, Common::String &fileName) override;
	Common::SeekableWriteStream *openSaveFileForWriting(int slot, bool compat, Common::String &fileName) override;

	/* HE version 60 script opcodes */
	void o60_setState();
	void o60_roomOps();
	void o60_actorOps();
	void o60_kernelSetFunctions();
	void o60_kernelGetFunctions();
	void o60_openFile();
	void o60_closeFile();
	void o60_deleteFile();
	void o60_readFile();
	void o60_rename();
	void o60_writeFile();
	void o60_soundOps();
	void o60_isSoundRunning();
	void o60_seekFilePos();
	void o60_localizeArrayToScript();
	void o60_redimArray();
	void o60_readFilePos();
};

class ScummEngine_v70he : public ScummEngine_v60he {
	friend class ResExtractor;
	friend class SoundHE;

public:
	// Used in akos.cpp
	enum HESndFlags {
		HE_SND_LOOP = 1,
		HE_SND_APPEND = 2,
		HE_SND_SOFT_SOUND = 4,
		HE_SND_QUICK_START = 8,
		HE_SND_OFFSET = 16,
		HE_SND_VOL = 32,
		HE_SND_FREQUENCY = 64,
		HE_SND_PAN = 128
	};


protected:
	enum SubOpType {
		SO_SOFT = 9,
		SO_IMAGE_LOADED = 18,
		SO_VARIABLE = 23,
		SO_SOUND_VOLUME = 25,
		SO_NOW = 56,
		SO_PRELOAD_SCRIPT = 120,
		SO_PRELOAD_SOUND = 121,
		SO_PRELOAD_COSTUME = 122,
		SO_PRELOAD_ROOM = 123,
		SO_UNLOCK_IMAGE = 159,
		SO_SOUND_ADD = 164,
		SO_NUKE_IMAGE = 192,
		SO_LOAD_IMAGE = 201,
		SO_LOCK_IMAGE = 202,
		SO_PRELOAD_IMAGE = 203,
		SO_ROOM_LOADED = 226,
		SO_COSTUME_LOADED = 227,
		SO_SOUND_LOADED = 228,
		SO_SCRIPT_LOADED = 229,
		SO_SOUND_CHANNEL = 230,
		SO_AT = 231,
		SO_SOUND_START = 232,
		SO_LOCK_FLOBJECT = 233,
		SO_UNLOCK_FLOBJECT = 235,
		SO_PRELOAD_FLUSH = 239,
		SO_PAUSE_MESSAGE = 240,
		SO_PAUSE_TITLE = 241,
		SO_PAUSE_OPTION = 242,
		SO_TITLE_BAR = 243,
		SO_QUIT_QUIT = 244,
		SO_SOUND_LOOPING = 245,
		SO_START_SYSTEM = 250,
		SO_START_SYSTEM_STRING = 251,
		SO_RESTART_STRING = 252,
		SO_RESTART_ARRAY = 253,
	};

	ResExtractor *_resExtractor;

	byte *_heV7DiskOffsets;
	byte *_heV7RoomOffsets;
	uint32 *_heV7RoomIntOffsets;

	int32 _heSndSoundId, _heSndOffset, _heSndChannel, _heSndFlags,
		_heSndFrequency, _heSndFrequencyShift, _heSndPan, _heSndVol;
	bool _heSndStartNewSoundFlag;

	int _numStoredFlObjects;
	ObjectData *_storedFlObjects;

public:
	ScummEngine_v70he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v70he() override;

	Common::Path generateFilename(const int room) const override;

	void backgroundToForegroundBlit(Common::Rect rect, int dirtybit = 0);
	void setActorRedrawFlags() override;

protected:
	void allocateArrays() override;
	int readResTypeList(ResType type) override;
	uint32 getResourceRoomOffset(ResType type, ResId idx) override;
	void setupOpcodes() override;

	void setupScummVars() override;
	void resetScummVars() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void readRoomsOffsets() override;
	void readGlobalObjects() override;
	void readIndexBlock(uint32 blocktype, uint32 itemsize) override;

	void clearRoomObjects() override;
	void resetRoomObjects() override;

	int getActorFromPos(int x, int y) override;

	void loadFlObject(uint object, uint room) override;
	void storeFlObject(int slot);
	void restoreFlObjects();

	void setCursorFromImg(uint img, uint room, uint imgindex) override;
	void setDefaultCursor() override;

	void resetActorBgs() override;

	/* HE version 70 script opcodes */
	void o70_soundOps();
	void o70_pickupObject();
	void o70_getActorRoom();
	void o70_resourceRoutines();
	void o70_systemOps();
	void o70_getStringLen();
	void o70_isResourceLoaded();
	void o70_readINI();
	void o70_writeINI();
	void o70_createDirectory();
	void o70_setSystemMessage();

	byte VAR_NUM_SOUND_CHANNELS;
};

#ifdef ENABLE_HE

#ifdef USE_ENET
class Net;
#ifdef USE_LIBCURL
class Lobby;
#endif
#endif

class ScummEngine_v71he : public ScummEngine_v70he {
	friend class Wiz;
	friend class Gdi;

protected:
	enum SubOpType {
		SO_SET_POLYGON = 246,
		SO_DELETE_POLYGON = 247,
		SO_SET_POLYGON_LOCAL = 248,
	};

public:
	ScummEngine_v71he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v71he() override;

	byte *heFindResourceData(uint32 tag, byte *ptr);
	byte *heFindResource(uint32 tag, byte *ptr);
	byte *findWrappedBlock(uint32 tag, byte *ptr, int state, bool flagError);

	Wiz *_wiz;
	bool _disableActorDrawingFlag = false;

	virtual int setupStringArray(int size);

protected:
	void setupOpcodes() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void redrawBGAreas() override;

	void processActors() override;
	void heFlushAuxEraseQueue();
	virtual void heFlushAuxQueues();

	void clearDrawQueues() override;

	int getStringCharWidth(byte chr);
	void appendSubstring(int dst, int src, int len2, int len);

	/* HE version 71 script opcodes */
	void o71_kernelSetFunctions();
	void o71_copyString();
	void o71_getStringWidth();
	void o71_appendString();
	void o71_concatString();
	void o71_compareString();
	void o71_getStringLenForWidth();
	void o71_getCharIndexInString();
	void o71_findBox();
	void o71_polygonOps();
	void o71_polygonHit();

	byte VAR_WIZ_TRANSPARENT_COLOR;
public:
	/* Actor AuxQueue stuff (HE) */
	HEEraseAuxEntry _heAuxEraseActorTable[16];
	int _heAuxEraseActorIndex = 0;
	HEAnimAuxEntry _heAuxAnimTable[16];
	int _heAuxAnimTableIndex = 0;

	void heQueueEraseAuxActor(ActorHE *a);
	void heQueueAnimAuxFrame(int actorNum, int subIndex);

	void buildRemapTable(byte *remapTablePtr, const byte *palDPtr, const byte *palSPtr);
};

class ScummEngine_v72he : public ScummEngine_v71he {
	friend class Wiz;

protected:
	enum SubOpType {
		SO_NONE = 1,
		SO_BIT = 2,  // SO_INT1
		SO_NIBBLE = 3,  // SO_INT4
		SO_BYTE = 4,  // SO_INT8
		SO_INT = 5,  // SO_INT16
		SO_DWORD = 6,  // SO_INT32
		SO_STRING = 7,
		SO_ARRAY = 8,
		SO_SOUND_SIZE = 13,
		SO_ROOM_SIZE = 14,
		SO_IMAGE_SIZE = 15,
		SO_COSTUME_SIZE = 16,
		SO_SCRIPT_SIZE = 17,
		SO_CONDITION = 21,
		SO_FLUSH_OBJECT_DRAW_QUE = 22,
		SO_TALK_CONDITION = 24,
		SO_UPDATE_SCREEN = 26,
		SO_PRIORITY = 43,
		SO_AT_IMAGE = 62,
		SO_IMAGE = 63,
		SO_ACTOR_DEFAULT_CLIPPED = 64,
		SO_ERASE = 68,
		SO_COMPLEX_ARRAY_ASSIGNMENT = 126,
		SO_COMPLEX_ARRAY_COPY_OPERATION = 127,
		SO_RANGE_ARRAY_ASSIGNMENT = 128,
		SO_COMPLEX_ARRAY_MATH_OPERATION = 138,
		SO_FORMATTED_STRING = 194,
		SO_REC = 195,
		SO_BAK = 199,
		SO_BAKREC = 200,
		SO_UNDIM_ARRAY = 204,
	};

#include "common/pack-start.h"	// START STRUCT PACKING

	struct ArrayHeader {
		int32 type;
		int32 acrossMin;
		int32 acrossMax;
		int32 downMin;
		int32 downMax;
		byte data[1];
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	int _stringLength = 1;
	byte _stringBuffer[4096] = {};

	WizImageCommand _wizImageCommand = {};
	FloodFillCommand _floodFillCommand = {};

public:
	ScummEngine_v72he(OSystem *syst, const DetectorResult &dr);

	void resetScumm() override;

	byte *getStringAddress(ResId idx) override;
	int setupStringArray(int size) override;
	virtual int setupStringArrayFromString(const char *cStr);
	virtual void getStringFromArray(int arrayNumber, char *buffer, int maxLength);

protected:
	void setupOpcodes() override;

	void setupScummVars() override;
	void resetScummVars() override;
	void readArrayFromIndexFile() override;

	void readMAXS(int blockSize) override;

	void redrawBGAreas() override;
	void checkExecVerbs() override;

	byte *defineArray(int array, int type, int downMin, int downMax, int acrossMin, int acrossMax, bool newArray = false, int *newid = NULL);
	int readArray(int array, int idx2, int idx1) override;
	void writeArray(int array, int idx2, int idx1, int value) override;
	void redimArray(int arrayId, int newDim2start, int newDim2end,
					int newDim1start, int newDim1end, int type);
	void checkArrayLimits(int array, int downMin, int downMax, int acrossMin, int acrossMax);
	void copyArray(int array1, int a1_dim2start, int a1_dim2end, int a1_dim1start, int a1_dim1end,
					int array2, int a2_dim2start, int a2_dim2end, int a2_dim1start, int a2_dim1end);
	void getArrayDataPtrAndDataSize(ArrayHeader *ah, int idx2, int idx1, int len1, byte **data, int *size, int *num);
	int readFileToArray(int slot, int32 size);
	void writeFileFromArray(int slot, int32 resID);
	void arrayBlockOperation(
		int dstVariable, int dstDownMin, int dstDownMax, int dstAcrossMin, int dstAcrossMax,
		int a2Variable, int a2DownMin, int a2DownMax, int a2AcrossMin, int a2AcrossMax,
		int a1Variable, int a1DownMin, int a1DownMax, int a1AcrossMin, int a1AcrossMax,
		int (*op)(int a2, int a1));

	void decodeParseString(int a, int b) override;
	void decodeScriptString(byte *dst, bool scriptString = false);
	void copyScriptString(byte *dst, int dstSize);

	int findObject(int x, int y, int num, int *args);
	int getSoundResourceSize(ResId idx);

	bool handleNextCharsetCode(Actor *a, int *c) override;
	int convertMessageToString(const byte *msg, byte *dst, int dstSize) override;
	void fakeBidiString(byte *ltext, bool ignoreVerb, int ltextSize) const override;

	void debugInput(byte *string);

	/* HE version 72 script opcodes */
	void o72_pushDWord();
	void o72_getScriptString();
	void o72_isAnyOf();
	void o72_resetCutscene();
	void o72_findObjectWithClassOf();
	void o72_getObjectImageX();
	void o72_getObjectImageY();
	void o72_captureWizImage();
	void o72_getTimer();
	void o72_setTimer();
	void o72_getSoundPosition();
	void o72_startScript();
	void o72_startObject();
	void o72_drawObject();
	void o72_printWizImage();
	void o72_getArrayDimSize();
	void o72_getNumFreeArrays();
	void o72_roomOps();
	void o72_actorOps();
	void o72_verbOps();
	void o72_findObject();
	void o72_arrayOps();
	void o72_systemOps();
	void o72_talkActor();
	void o72_talkEgo();
	void o72_dimArray();
	void o72_dim2dimArray();
	void o72_traceStatus();
	void o72_debugInput();
	void o72_drawWizImage();
	void o72_kernelGetFunctions();
	void o72_jumpToScript();
	void o72_openFile();
	void o72_readFile();
	void o72_writeFile();
	void o72_findAllObjects();
	void o72_deleteFile();
	void o72_rename();
	void o72_getPixel();
	void o72_pickVarRandom();
	void o72_redimArray();
	void o72_readINI();
	void o72_writeINI();
	void o72_getResourceSize();
	void o72_createDirectory();
	void o72_setSystemMessage();

	byte VAR_NUM_ROOMS;
	byte VAR_NUM_SCRIPTS;
	byte VAR_NUM_SOUNDS;
	byte VAR_NUM_COSTUMES;
	byte VAR_NUM_IMAGES;
	byte VAR_NUM_CHARSETS;

	byte VAR_SOUND_ENABLED;

	byte VAR_POLYGONS_ONLY;

	byte VAR_MOUSE_STATE;			// Used in checkExecVerbs();
	byte VAR_PLATFORM;
};

class ScummEngine_v80he : public ScummEngine_v72he {
protected:
	int32 _heSndResId = 0;

	enum SubOpType {
		SO_CURSOR_IMAGE = 19,
		SO_CURSOR_COLOR_IMAGE = 20,
		SO_ADD = 27,
		SO_ACTOR = 55,
		SO_BUTTON = 60,
	};

	enum LineType {
		kLTColor = 1,
		kLTActor = 2,
		kLTImage = 3
	};

public:
	ScummEngine_v80he(OSystem *syst, const DetectorResult &dr);

protected:
	void setupOpcodes() override;

	void setupScummVars() override;
	void resetScummVars() override;

	void parseEvent(Common::Event event) override;

	void initCharset(int charset) override;

	void clearDrawQueues() override;

	void drawLine(int x1, int y1, int x, int unk1, int unk2, int type, int id);
	void drawPixel(int x, int y, int flags);

	void setDefaultCursor() override;

	/* HE version 80 script opcodes */
	void o80_createSound();
	void o80_getFileSize();
	void o80_stringToInt();
	void o80_getSoundVar();
	void o80_localizeArrayToRoom();
	void o80_sourceDebug();
	void o80_readConfigFile();
	void o80_writeConfigFile();
	void o80_cursorCommand();
	void o80_setState();
	void o80_drawWizPolygon();
	void o80_drawLine();
	void o80_pickVarRandom();

	byte VAR_PLATFORM_VERSION;
	byte VAR_CURRENT_CHARSET;
	byte VAR_KEY_STATE;
	byte VAR_COLOR_DEPTH;
};

class ScummEngine_v90he : public ScummEngine_v80he {
	friend class LogicHE;
#ifdef USE_ENET
	friend class Net;
#ifdef USE_LIBCURL
	friend class Lobby;
#endif
#endif
	friend class Moonbase;
	friend class Basketball;
	friend class MoviePlayer;
	friend class Sprite;
	friend class Wiz;
	friend class ScummEngine_v99he;

protected:
	enum SubOpType {
		SO_COORD_2D = 28,
		SO_COORD_3D = 29,
		SO_XPOS = 30,
		SO_YPOS = 31,
		SO_WIDTH = 32,
		SO_HEIGHT = 33,
		SO_STEP_DIST_X = 34,
		SO_STEP_DIST_Y = 35,
		SO_COUNT = 36,
		SO_GROUP = 37,
		SO_DRAW_XPOS = 38,
		SO_DRAW_YPOS = 39,
		SO_PROPERTY = 42,
		SO_PRIORITY = 43,
		SO_MOVE = 44,
		SO_FIND = 45,
		SO_GENERAL_CLIP_STATE = 46,
		SO_GENERAL_CLIP_RECT = 47,
		SO_DRAW = 48,
		SO_LOAD = 49,
		SO_SAVE = 50,
		SO_CAPTURE = 51,
		SO_STATE = 52,
		SO_ANGLE = 53,
		SO_SET_FLAGS = 54,
		SO_INIT = 57,
		SO_AT_IMAGE = 62,
		SO_IMAGE = 63,
		SO_AT = 65,
		SO_ERASE = 68,
		SO_TO = 70,
		SO_STEP_DIST = 77,
		SO_ANIMATION = 82,
		SO_PALETTE = 86,
		SO_SCALE = 92,
		SO_ANIMATION_SPEED = 97,
		SO_SHADOW = 98,
		SO_UPDATE = 124,
		SO_CLASS = 125,
		SO_SORT = 129,
		SO_HISTOGRAM = 130,
		SO_POLY_POLYGON = 131,
		SO_CHANNEL = 132,
		SO_RENDER_RECTANGLE = 133,
		SO_RENDER_LINE = 134,
		SO_RENDER_PIXEL = 135,
		SO_RENDER_FLOOD_FILL = 136,
		SO_RENDER_INTO_IMAGE = 137,
		SO_NEW_GENERAL_PROPERTY = 139,
		SO_MASK = 140,
		SO_FONT_START = 141,
		SO_FONT_CREATE = 142,
		SO_FONT_RENDER = 143,
		SO_CLOSE = 165,
		SO_RENDER_ELLIPSE = 189,
		SO_FONT_END = 196,
		SO_ACTOR_VARIABLE = 198
	};

	struct VideoParameters {
		byte filename[260];
		int32 status;
		int32 flags;
		int32 number;
		int32 wizResNum;
	};

	VideoParameters _videoParams;

	int32 _heObject = 0;
	int32 _heObjectNum = 0;
	int32 _hePaletteNum = 0;

	int32 _maxSpriteNum = 0;
	int32 _minSpriteNum = 0;
	int32 _curSpriteGroupId = 0;

	LogicHE *_logicHE = nullptr;
	MoviePlayer *_moviePlay;
	Sprite *_sprite;

#ifdef USE_ENET
public:
	Net *_net;
#ifdef USE_LIBCURL
	Lobby *_lobby;
#endif
#endif

public:
	ScummEngine_v90he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v90he() override;

protected:
	void allocateArrays() override;
	void setupOpcodes() override;

	void resetScumm() override;

	void setupScummVars() override;
	void resetScummVars() override;

	void scummLoop(int delta) override;
	void scummLoop_handleDrawing() override;
	void runBootscript() override;

	void processInput() override;
	void clearClickedStatus() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void readMAXS(int blockSize) override;
	void setResourceOffHeap(int typeId, int resId, int val);

	void processActors() override;
	void heFlushAuxQueues() override;
	const byte *heAuxFindBlock(HEAnimAuxData *auxInfoPtr, int32 id);
	void heAuxReleaseAuxDataInfo(HEAnimAuxData *auxInfoPtr);
	void heAuxGetAuxDataInfo(HEAnimAuxData *auxInfoPtr, int whichActor, int auxIndex);
	bool heAuxProcessFileRelativeBlock(HEAnimAuxData *auxInfoPtr, const byte *dataBlockPtr);
	bool heAuxProcessDisplacedBlock(HEAnimAuxData *auxInfoPtr, const byte *displacedBlockPtr);

	void getArrayDim(int array, int *downMin, int *downMax, int *acrossMin, int *acrossMax);
	void sortArray(int array, int downMin, int downMax, int acrossMin, int acrossMax, int sortOrder);

public:
	int getGroupSpriteArray(int spriteGroupId);

protected:
	uint8 *getHEPaletteIndex(int palSlot);
	int getHEPaletteColor(int palSlot, int color);
	int getHEPaletteSimilarColor(int palSlot, int red, int green, int start, int end);
	int getHEPalette16BitColorComponent(int component, int type);
	int getHEPaletteColorComponent(int palSlot, int color, int component);
	void setHEPaletteColor(int palSlot, uint8 color, uint8 r, uint8 g, uint8 b);
	void setHEPaletteFromPtr(int palSlot, const uint8 *palData);
	void setHEPaletteFromCostume(int palSlot, int resId);
	void setHEPaletteFromImage(int palSlot, int resId, int state);
	void setHEPaletteFromRoom(int palSlot, int resId, int state);
	void restoreHEPalette(int palSlot);
	void copyHEPalette(int dstPalSlot, int srcPalSlot);
	void copyHEPaletteColor(int palSlot, uint8 dstColor, uint16 srcColor);

	/*
	* Math functions
	*
	* Please do not attempt to revert these to our standard math functions!
	* These are accurate for the games, ours aren't :-P
	*/
	int scummMathSin(int angle);
	int scummMathCos(int angle);
	int scummMathSqrt(int value);
	int scummMathDist2D(int x1, int y1, int x2, int y2);
	int scummMathAngleFromDelta(int dx, int dy);
	int scummMathAngleOfLineSegment(int x1, int y1, int x2, int y2);


protected:
	/* HE version 90 script opcodes */
	void o90_dup_n();
	void o90_min();
	void o90_max();
	void o90_sin();
	void o90_cos();
	void o90_sqrt();
	void o90_getAngleFromDelta();
	void o90_getAngleFromLine();
	void o90_getActorData();
	void o90_priorityStartScript();
	void o90_priorityChainScript();
	void o90_videoOps();
	void o90_getVideoData();
	void o90_wizImageOps();
	void o90_getDistanceBetweenPoints();
	void o90_getSpriteInfo();
	void o90_setSpriteInfo();
	void o90_getSpriteGroupInfo();
	void o90_setSpriteGroupInfo();
	void o90_getWizData();
	void o90_floodFill();
	void o90_mod();
	void o90_shl();
	void o90_shr();
	void o90_xor();
	void o90_findAllObjectsWithClassOf();
	void o90_getOverlap();
	void o90_cond();
	void o90_dim2dim2Array();
	void o90_redim2dimArray();
	void o90_getLinesIntersectionPoint();
	void o90_sortArray();
	void o90_getObjectData();
	void o90_getPaletteData();
	void o90_paletteOps();
	void o90_fontEnum();
	void o90_getActorAnimProgress();
	void o90_kernelGetFunctions();
	void o90_kernelSetFunctions();

	byte VAR_NUM_SPRITE_GROUPS;
	byte VAR_NUM_SPRITES;
	byte VAR_NUM_PALETTES;
	byte VAR_NUM_UNK;
	byte VAR_SPRITE_IMAGE_CHANGE_DOES_NOT_RESET_SETTINGS;

	byte VAR_U32_VERSION;
	byte VAR_U32_ARRAY_UNK;

#ifdef USE_ENET
	byte VAR_REMOTE_START_SCRIPT = 98;
	byte VAR_NETWORK_AVAILABLE = 109;
	byte VAR_NETWORK_RECEIVE_ARRAY_SCRIPT = 101;

public:
	int networkSessionDialog();
#endif
};

class ScummEngine_v95he : public ScummEngine_v90he {
public:
	ScummEngine_v95he(OSystem *syst, const DetectorResult &dr) : ScummEngine_v90he(syst, dr) {}

protected:
	void resetActorBgs() override;
	bool prepareForActorErase();
};

class ScummEngine_v99he : public ScummEngine_v95he {
public:
	ScummEngine_v99he(OSystem *syst, const DetectorResult &dr) : ScummEngine_v95he(syst, dr) {}

	void resetScumm() override;

protected:
	void resetScummVars() override;

	void readMAXS(int blockSize) override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void copyPalColor(int dst, int src) override;
	void darkenPalette(int redScale, int greenScale, int blueScale, int startColor, int endColor) override;
	void setPaletteFromPtr(const byte *ptr, int numcolor = -1) override;
	void setPalColor(int index, int r, int g, int b) override;
	void updatePalette() override;
};

class ScummEngine_v100he : public ScummEngine_v99he {
friend class AI;
friend class Moonbase;
friend class LogicHEBasketball;
friend class Basketball;

// The following engine versions use sub opcodes from this version
friend class ScummEngine_v71he;
friend class ScummEngine_v72he;
friend class ScummEngine_v90he;
friend class ScummEngine_v80he;

protected:
	ResType _heResType;
	int32 _heResId;

	byte _debugInputBuffer[256];
	enum SubOpType {
		// COMMON
		SO_INIT = 0,
		SO_ACTOR = 1,
		SO_ANGLE = 2,
		SO_ANIMATION = 3,
		SO_ANIMATION_SPEED = 4,
		SO_ARRAY = 5,
		SO_AT = 6,
		SO_AT_IMAGE = 7,
		SO_BACKGROUND_OFF = 8,
		SO_BACKGROUND_ON = 9,
		SO_CAPTURE = 11,
		SO_CENTER = 12,
		SO_CHANNEL = 13,
		SO_CHARSET_SET = 14, // SO_CHARSET
		SO_CHARSET_COLOR = 15,
		SO_CLASS = 16,
		SO_CLIPPED = 18,
		SO_CLOSE = 19,
		SO_COLOR = 20,
		SO_COLOR_LIST = 21,
		SO_CONDITION = 22,
		SO_COORD_2D = 23,
		SO_COORD_3D = 24,
		SO_COSTUME = 25,
		SO_COUNT = 26,
		SO_DEFAULT = 27,
		SO_DELETE_POLYGON = 28,
		SO_DRAW = 29,
		SO_DRAW_XPOS = 30,
		SO_DRAW_YPOS = 31,
		SO_ERASE = 32,
		SO_FIND = 33,
		SO_FLOBJECT = 34,
		SO_FORMATTED_STRING = 35,
		SO_GENERAL_CLIP_RECT = 36,
		SO_GENERAL_CLIP_STATE = 37,
		SO_GROUP = 38,
		SO_HEIGHT = 39,
		SO_IMAGE = 40,
		SO_BIT = 41, // SO_INT1
		SO_INT = 42, // SO_INT16
		SO_DWORD = 43, // SO_INT32
		SO_NIBBLE = 44, // SO_INT4
		SO_BYTE = 45, // SO_INT8
		SO_LEFT = 46,
		SO_LOAD = 47,
		SO_MASK = 48,
		SO_MOVE = 49,
		SO_MUMBLE = 51,
		SO_NAME = 52,
		SO_NEW = 53,
		SO_NEW_GENERAL_PROPERTY = 54,
		SO_NOW = 55,
		SO_OVERHEAD = 56,
		SO_PALETTE = 57,
		SO_POLY_TO_POLY = 58,
		SO_PRIORITY = 59,
		SO_PROPERTY = 60,
		SO_RESTART = 61,
		SO_ROOM = 62,
		SO_ROOM_PALETTE = 63,
		SO_SAVE = 64,
		SO_SCALE = 65,
		SO_SCRIPT = 66,
		SO_SET_FLAGS = 67,
		SO_SET_POLYGON = 68,
		SO_SET_POLYGON_LOCAL = 69,
		SO_SHADOW = 70,
		SO_SOUND = 72,
		SO_STATE = 73,
		SO_STEP_DIST = 74,
		SO_STEP_DIST_X = 75,
		SO_STEP_DIST_Y = 76,
		SO_STRING = 77,
		SO_TALKIE = 78,
		SO_TEXTSTRING = 79,
		SO_TITLE_BAR = 80,
		SO_TO = 81,
		SO_UPDATE = 82,
		SO_VARIABLE = 83,
		SO_WIDTH = 84,
		SO_XPOS = 85,
		SO_YPOS = 86,
		SO_ALWAYS_ZCLIP = 87,
		SO_IMAGE_ZCLIP = 88,
		SO_NEVER_ZCLIP = 89,
		SO_BASEOP = 91,
		SO_END = 92,

		// ACTOR
		SO_ACTOR_DEFAULT_CLIPPED = 128,
		SO_ACTOR_INIT = 129,
		SO_ACTOR_SOUNDS = 130,
		SO_ACTOR_WIDTH = 131,
		SO_ANIMATION_DEFAULT = 132,
		SO_ELEVATION = 133,
		SO_FOLLOW_BOXES = 134,
		SO_IGNORE_BOXES = 135,
		SO_ACTOR_IGNORE_TURNS_OFF = 136, // SO_IGNORE_TURNS_OFF
		SO_ACTOR_IGNORE_TURNS_ON = 137, // SO_IGNORE_TURNS_ON
		SO_INIT_ANIMATION = 138,
		SO_STAND_ANIMATION = 139,
		SO_TALK_ANIMATION = 140,
		SO_TALK_COLOR = 141,
		SO_TALK_CONDITION = 142,
		SO_TEXT_OFFSET = 143,
		SO_WALK_ANIMATION = 144,

		// ARRAY
		SO_ASSIGN_2DIM_LIST = 128,
		SO_ASSIGN_INT_LIST = 129,
		SO_COMPLEX_ARRAY_ASSIGNMENT = 130,
		SO_COMPLEX_ARRAY_COPY_OPERATION = 131,
		SO_COMPLEX_ARRAY_MATH_OPERATION = 132,
		SO_RANGE_ARRAY_ASSIGNMENT = 133,
		SO_SORT = 134,
		SO_UNDIM_ARRAY = 135,

		// HEAP
		SO_CLEAR_HEAP = 128,
		SO_PRELOAD_FLUSH = 129,
		SO_LOCK = 132,
		SO_NUKE = 133,
		SO_OFF_HEAP = 134,
		SO_ON_HEAP = 135,
		SO_PRELOAD = 136,
		SO_UNLOCK = 137,

		// IMAGE
		SO_FONT_CREATE = 128,
		SO_FONT_END = 129,
		SO_FONT_RENDER = 130,
		SO_FONT_START = 131,
		SO_HISTOGRAM = 132,
		SO_RENDER_ELLIPSE = 133,
		SO_RENDER_FLOOD_FILL = 134,
		SO_RENDER_INTO_IMAGE = 135,
		SO_RENDER_LINE = 136,
		SO_RENDER_PIXEL = 137,
		SO_RENDER_RECTANGLE = 138,

		// CURSOR
		SO_CURSOR_IMAGE = 128,
		SO_CURSOR_COLOR_IMAGE = 129,
		SO_CURSOR_COLOR_PAL_IMAGE = 130,
		SO_CURSOR_HOTSPOT = 132,
		SO_CURSOR_ON = 134,
		SO_CURSOR_OFF = 135,
		SO_CURSOR_SOFT_ON = 136,
		SO_CURSOR_SOFT_OFF = 137,
		SO_USERPUT_ON = 139,
		SO_USERPUT_OFF = 140,
		SO_USERPUT_SOFT_ON = 141,
		SO_USERPUT_SOFT_OFF = 142,

		// ROOM
		SO_OBJECT_ORDER = 129,
		SO_ROOM_COPY_PALETTE = 130,
		SO_ROOM_FADE = 131,
		SO_ROOM_INTENSITY = 132,
		SO_ROOM_INTENSITY_RGB = 133,
		SO_ROOM_NEW_PALETTE = 134,
		SO_ROOM_PALETTE_IN_ROOM = 135,
		SO_ROOM_SAVEGAME = 136,
		SO_ROOM_SAVEGAME_BY_NAME = 137,
		SO_ROOM_SCREEN = 138,
		SO_ROOM_SCROLL = 139,
		SO_ROOM_SHAKE_OFF = 140,
		SO_ROOM_SHAKE_ON = 141,
		SO_ROOM_TRANSFORM = 142,

		// SCRIPT
		SO_BAK = 128,
		SO_BAKREC = 129,
		SO_REC = 130,

		// SYSTEM
		SO_FLUSH_OBJECT_DRAW_QUEUE = 128,
		SO_PAUSE_TITLE = 131,
		SO_QUIT = 132,
		SO_QUIT_QUIT = 133,
		SO_RESTART_STRING = 134,
		SO_START_SYSTEM_STRING = 135,
		SO_UPDATE_SCREEN = 136,

		// SOUND
		SO_SOUND_ADD = 128,
		SO_SOUND_CHANNEL = 129,
		SO_SOUND_FREQUENCY = 130,
		SO_SOUND_LOOPING = 131,
		SO_SOUND_MODIFY = 132,
		SO_SOUND_PAN = 133,
		SO_SOUND_START = 134,
		SO_SOUND_SOFT = 135,
		SO_SOUND_VOLUME = 136,

		// WAIT
		SO_WAIT_FOR_ACTOR = 128,
		SO_WAIT_FOR_CAMERA = 129,
		SO_WAIT_FOR_MESSAGE = 130,
		SO_WAIT_FOR_SENTENCE = 131,
	};

public:
	ScummEngine_v100he(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v100he() override;

	void resetScumm() override;

	void setupScummVars() override;
	void resetScummVars() override;

protected:
	void setupOpcodes() override;

	void saveLoadWithSerializer(Common::Serializer &s) override;

	void decodeParseString(int a, int b) override;

	/* HE version 100 script opcodes */
	void o100_actorOps();
	void o100_arrayOps();
	void o100_dim2dimArray();
	void o100_redim2dimArray();
	void o100_dimArray();
	void o100_drawLine();
	void o100_drawObject();
	void o100_floodFill();
	void o100_setSpriteGroupInfo();
	void o100_resourceRoutines();
	void o100_wizImageOps();
	void o100_jumpToScript();
	void o100_createSound();
	void o100_dim2dim2Array();
	void o100_paletteOps();
	void o100_priorityChainScript();
	void o100_priorityStartScript();
	void o100_redimArray();
	void o100_roomOps();
	void o100_setSystemMessage();
	void o100_soundOps();
	void o100_setSpriteInfo();
	void o100_startScript();
	void o100_systemOps();
	void o100_cursorCommand();
	void o100_videoOps();
	void o100_wait();
	void o100_writeFile();
	void o100_debugInput();
	void o100_isResourceLoaded();
	void o100_getResourceSize();
	void o100_getSpriteGroupInfo();
	void o100_getPaletteData();
	void o100_readFile();
	void o100_getSpriteInfo();
	void o100_getWizData();
	void o100_getVideoData();

protected:
	byte VAR_U32_USER_VAR_A;
	byte VAR_U32_USER_VAR_B;
	byte VAR_U32_USER_VAR_C;
	byte VAR_U32_USER_VAR_D;
	byte VAR_U32_USER_VAR_E;
	byte VAR_U32_USER_VAR_F;
	byte VAR_U32_USER_VAR_G;
	byte VAR_U32_USER_VAR_H;

	byte VAR_REMOTE_START_SCRIPT;
	byte VAR_NETWORK_AVAILABLE;
	byte VAR_NETWORK_RECEIVE_ARRAY_SCRIPT;

public:
	bool mapGeneratorDialog(bool demo);
};

class ScummEngine_vCUPhe : public Engine {
protected:
	CUP_Player *_cupPlayer;
	FilenamePattern _filenamePattern;

public:
	ScummEngine_vCUPhe(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_vCUPhe() override;

	Common::Error run() override;

	void parseEvents();

	OSystem *_syst;

	GameSettings _game;
};

#endif


} // End of namespace Scumm

#endif
