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

#ifndef SCUMM_SCRIPT_V6_H
#define SCUMM_SCRIPT_V6_H

#include "scumm/scumm.h"

namespace Scumm {

class ScummEngine_v6 : public ScummEngine {
protected:
	enum ArrayType {
		kBitArray = 1,
		kNibbleArray = 2,
		kByteArray = 3,
		kStringArray = 4,
		kIntArray = 5,
		kDwordArray = 6
	};

	enum SubOpType {
		SO_AT = 65,
		SO_COLOR = 66,
		SO_CLIPPED = 67,
		SO_CENTER = 69,

		SO_LEFT = 71,
		SO_OVERHEAD = 72,

		SO_MUMBLE = 74,
		SO_TEXTSTRING = 75,
		SO_COSTUME = 76,
		SO_STEP_DIST = 77,
		SO_SOUND = 78,
		SO_WALK_ANIMATION = 79,
		SO_TALK_ANIMATION = 80,
		SO_STAND_ANIMATION = 81,
		SO_ANIMATION = 82,
		SO_DEFAULT = 83,
		SO_ELEVATION = 84,
		SO_ANIMATION_DEFAULT = 85,
		SO_PALETTE = 86,
		SO_TALK_COLOR = 87,
		SO_ACTOR_NAME = 88,
		SO_INIT_ANIMATION = 89,

		SO_ACTOR_WIDTH = 91,
		SO_SCALE = 92,
		SO_NEVER_ZCLIP = 93,
		SO_ALWAYS_ZCLIP = 94,
		SO_IGNORE_BOXES = 95,
		SO_FOLLOW_BOXES = 96,
		SO_ANIMATION_SPEED = 97,
		SO_SHADOW = 98,
		SO_TEXT_OFFSET = 99,

		SO_LOAD_SCRIPT = 100,
		SO_LOAD_SOUND = 101,
		SO_LOAD_COSTUME = 102,
		SO_LOAD_ROOM = 103,
		SO_NUKE_SCRIPT = 104,
		SO_NUKE_SOUND = 105,
		SO_NUKE_COSTUME = 106,
		SO_NUKE_ROOM = 107,
		SO_LOCK_SCRIPT = 108,
		SO_LOCK_SOUND = 109,
		SO_LOCK_COSTUME = 110,
		SO_LOCK_ROOM = 111,
		SO_UNLOCK_SCRIPT = 112,
		SO_UNLOCK_SOUND = 113,
		SO_UNLOCK_COSTUME = 114,
		SO_UNLOCK_ROOM = 115,
		SO_CLEAR_HEAP = 116,
		SO_LOAD_CHARSET = 117,
		SO_NUKE_CHARSET = 118,
		SO_LOAD_OBJECT = 119,

		SO_VERB_IMAGE = 124,
		SO_VERB_NAME = 125,
		SO_VERB_COLOR = 126,
		SO_VERB_HICOLOR = 127,
		SO_VERB_AT = 128,
		SO_VERB_ON = 129,
		SO_VERB_OFF = 130,
		SO_VERB_DELETE = 131,
		SO_VERB_NEW = 132,
		SO_VERB_DIMCOLOR = 133,
		SO_VERB_DIM = 134,
		SO_VERB_KEY = 135,
		SO_VERB_CENTER = 136,
		SO_VERB_NAME_STR = 137,

		SO_VERB_IMAGE_IN_ROOM = 139,
		SO_VERB_BAKCOLOR = 140,
		SO_SAVE_VERBS = 141,
		SO_RESTORE_VERBS = 142,
		SO_DELETE_VERBS = 143,
		SO_CURSOR_ON = 144,
		SO_CURSOR_OFF = 145,
		SO_USERPUT_ON = 146,
		SO_USERPUT_OFF = 147,
		SO_CURSOR_SOFT_ON = 148,
		SO_CURSOR_SOFT_OFF = 149,
		SO_USERPUT_SOFT_ON = 150,
		SO_USERPUT_SOFT_OFF = 151,

		SO_CURSOR_IMAGE = 153,
		SO_CURSOR_HOTSPOT = 154,

		SO_CHARSET_SET = 156,
		SO_CHARSET_COLOR = 157,
		SO_RESTART = 158,
		SO_PAUSE = 159,
		SO_QUIT = 160,

		SO_WAIT_FOR_ACTOR = 168,
		SO_WAIT_FOR_MESSAGE = 169,
		SO_WAIT_FOR_CAMERA = 170,
		SO_WAIT_FOR_SENTENCE = 171,
		SO_ROOM_SCROLL = 172,
		SO_ROOM_SCREEN = 174,
		SO_ROOM_PALETTE = 175,
		SO_ROOM_SHAKE_ON = 176,
		SO_ROOM_SHAKE_OFF = 177,
		SO_ROOM_INTENSITY = 179,
		SO_ROOM_SAVEGAME = 180,
		SO_ROOM_FADE = 181,
		SO_RGB_ROOM_INTENSITY = 182,
		SO_ROOM_SHADOW = 183,
		SO_SAVE_STRING = 184,
		SO_LOAD_STRING = 185,
		SO_ROOM_TRANSFORM = 186,
		SO_CYCLE_SPEED = 187,

		SO_VERB_INIT = 196,
		SO_ACTOR_INIT = 197,
		SO_ACTOR_VARIABLE = 198,
		SO_INT_ARRAY = 199,
		SO_BIT_ARRAY = 200,
		SO_NIBBLE_ARRAY = 201,
		SO_BYTE_ARRAY = 202,
		SO_STRING_ARRAY = 203,
		SO_UNDIM_ARRAY = 204,
		SO_ASSIGN_STRING = 205,

		SO_ASSIGN_INT_LIST = 208,

		SO_ASSIGN_2DIM_LIST = 212,
		SO_ROOM_NEW_PALETTE = 213,
		SO_CURSOR_TRANSPARENT = 214,
		SO_ACTOR_IGNORE_TURNS_ON = 215,
		SO_ACTOR_IGNORE_TURNS_OFF = 216,
		SO_NEW = 217,

		SO_ALWAYS_ZCLIP_FT_DEMO = 225,
		SO_WAIT_FOR_ANIMATION = 226,
		SO_ACTOR_DEPTH = 227,
		SO_ACTOR_WALK_SCRIPT = 228,
		SO_ACTOR_STOP = 229,

		SO_ACTOR_FACE = 230,
		SO_ACTOR_TURN = 231,
		SO_WAIT_FOR_TURN = 232,
		SO_ACTOR_WALK_PAUSE = 233,
		SO_ACTOR_WALK_RESUME = 234,
		SO_ACTOR_TALK_SCRIPT = 235,

		SO_BASEOP = 254,
		SO_END = 255,
	};

#include "common/pack-start.h"	// START STRUCT PACKING

	struct ArrayHeader {
		int16 dim1;
		int16 type;
		int16 dim2;
		byte data[1];
	} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

	struct TextObject {
		int16 xpos, ypos;
		byte color;
		byte charset;
		byte text[512];

		void clear() {
			xpos = 0;
			ypos = 0;
			color = 0;
			charset = 0;
			for (uint i = 0; i < ARRAYSIZE(text); i++) text[i] = 0;
		}
	};

	/** BlastObjects to draw */
	struct BlastObject {
		uint16 number;
		Common::Rect rect;
		uint16 scaleX, scaleY;
		uint16 image;
		uint16 mode;

		void clear() {
			number = 0;
			rect = Common::Rect();
			scaleX = 0;
			scaleY = 0;
			image = 0;
			mode = 0;
		}
	};

	int _blastObjectQueuePos;
	int _blastObjectRectsQueue = 0;
	BlastObject _blastObjectQueue[200];
	Common::Rect _blastObjectsRectsToBeRestored[200];

	// Akos Class
	struct {
		int16 cmd;
		int16 actor;
		int16 param1;
		int16 param2;
	} _akosQueue[32];
	int16 _akosQueuePos;

	byte _curActor;
	int _curVerb;
	int _curVerbSlot;

	bool _forcedWaitForMessage;
	bool _skipVideo;

public:
	ScummEngine_v6(OSystem *syst, const DetectorResult &dr);

	void resetScumm() override;

protected:
	void setupOpcodes() override;

	void scummLoop_handleSaveLoad() override;
	void scummLoop_handleActors() override;
	void processKeyboard(Common::KeyState lastKeyHit) override;

	void setupScummVars() override;
	virtual void decodeParseString(int a, int b);
	void readArrayFromIndexFile() override;

	byte *getStringAddress(ResId idx) override;
	void readMAXS(int blockSize) override;

	void palManipulateInit(int resID, int start, int end, int time) override;
	void drawDirtyScreenParts() override;

	int getStackList(int *args, uint maxnum);
	int popRoomAndObj(int *room);

	ArrayHeader *getArray(int array);
	byte *defineArray(int array, int type, int dim2, int dim1);
	int findFreeArrayId();
public: // FIXME. TODO
	void nukeArray(int array);
	void removeBlastObjects();
	void restoreBlastObjectsRects();

protected:
	virtual int readArray(int array, int index, int base);
	virtual void writeArray(int array, int index, int base, int value);
	void shuffleArray(int num, int minIdx, int maxIdx);

	void setDefaultCursor() override;
	void setCursorTransparency(int a) override;
	void setCursorHotspot(int x, int y) override;

	virtual void setCursorFromImg(uint img, uint room, uint imgindex);
	void useIm01Cursor(const byte *im, int w, int h);
	void useBompCursor(const byte *im, int w, int h);
	void grabCursor(int x, int y, int w, int h);
	void setCursorFromBuffer(const byte *ptr, int width, int height, int pitch, bool preventScale = false) override;
	void ditherCursor();

	virtual void drawBlastTexts() {}
	virtual void removeBlastTexts() {}

	void enqueueObject(int objectNumber, int objectX, int objectY, int objectWidth,
	                   int objectHeight, int scaleX, int scaleY, int image, int mode);
	void drawBlastObjects();
	void drawBlastObject(BlastObject *eo);

	void restoreBlastObjectRect(Common::Rect r);

	void clearDrawQueues() override;

	int getBannerColor(int bannerId) override;
	const char *getGUIString(int stringId) override;
	void setSkipVideo(int value) override { _skipVideo = value; }
	void setUpMainMenuControls() override;

public:
	bool akos_increaseAnims(const byte *akos, Actor *a);
	bool akos_increaseAnim(Actor *a, int i, const byte *aksq, const uint16 *akfo, int numakfo);
protected:
	void akos_queCommand(byte cmd, Actor *a, int param_1, int param_2);
	virtual void akos_processQueue();

	void processActors() override;

	int getSpecialBox(int x, int y);

	int getDistanceBetween(bool is_obj_1, int b, int c, bool is_obj_2, int e, int f);

	/* Version 6 script opcodes */
	void o6_setBlastObjectWindow();
	void o6_pushByte();
	void o6_pushWord();
	void o6_pushByteVar();
	void o6_pushWordVar();
	void o6_byteArrayRead();
	void o6_wordArrayRead();
	void o6_byteArrayIndexedRead();
	void o6_wordArrayIndexedRead();
	void o6_dup();
	void o6_pop();
	void o6_not();
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
	void o6_if();
	void o6_ifNot();
	void o6_jump();
	void o6_startScript();
	void o6_startScriptQuick();
	void o6_startObject();
	void o6_drawObject();
	void o6_drawObjectAt();
	void o6_stopObjectCode();
	void o6_endCutscene();
	void o6_cutscene();
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
	void o6_putActorAtXY();
	void o6_putActorAtObject();
	void o6_faceActor();
	void o6_animateActor();
	void o6_doSentence();
	void o6_pickupObject();
	void o6_loadRoomWithEgo();
	void o6_getRandomNumber();
	void o6_getRandomNumberRange();
	void o6_getActorMoving();
	void o6_isScriptRunning();
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
	void o6_actorOps();
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
	void o6_getActorAnimCounter();
	void o6_soundKludge();
	void o6_isAnyOf();
	void o6_systemOps();
	void o6_isActorInBox();
	void o6_delay();
	void o6_delaySeconds();
	void o6_delayMinutes();
	void o6_stopSentence();
	void o6_printLine();
	void o6_printText();
	void o6_printDebug();
	void o6_printSystem();
	void o6_printActor();
	void o6_printEgo();
	void o6_talkActor();
	void o6_talkEgo();
	void o6_dimArray();
	void o6_dummy();
	void o6_startObjectQuick();
	void o6_startScriptQuick2();
	void o6_dim2dimArray();
	void o6_abs();
	void o6_distObjectObject();
	void o6_distObjectPt();
	void o6_distPtPt();
	virtual void o6_kernelSetFunctions();
	void o6_delayFrames();
	void o6_pickOneOf();
	void o6_pickOneOfDefault();
	void o6_jumpToScript();
	void o6_isRoomScriptRunning();
	void o6_kernelGetFunctions();
	void o6_getAnimateVariable();
	void o6_drawBlastObject();
	void o6_getActorLayer();
	void o6_stampObject();
	void o6_bor();
	void o6_band();
	void o6_stopTalking();
	void o6_findAllObjects();
	void o6_pickVarRandom();
	void o6_getDateTime();
	void o6_getPixel();
	void o6_setBoxSet();
	void o6_shuffle();

	byte VAR_VIDEONAME;
	byte VAR_RANDOM_NR;
	byte VAR_STRING2DRAW;

	byte VAR_TIMEDATE_YEAR;
	byte VAR_TIMEDATE_MONTH;
	byte VAR_TIMEDATE_DAY;
	byte VAR_TIMEDATE_HOUR;
	byte VAR_TIMEDATE_MINUTE;
	byte VAR_TIMEDATE_SECOND;
};


} // End of namespace Scumm

#endif
