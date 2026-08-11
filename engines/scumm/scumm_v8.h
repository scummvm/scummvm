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

#ifndef SCUMM_SCRIPT_V8_H
#define SCUMM_SCRIPT_V8_H

#ifdef ENABLE_SCUMM_7_8

#include "scumm/scumm_v7.h"

namespace Scumm {

class ScummEngine_v8 : public ScummEngine_v7 {
protected:
	enum SubOpType {
		SO_INT_ARRAY = 10,				// SO_ARRAY_SCUMMVAR
		SO_STRING_ARRAY = 11,			// SO_ARRAY_STRING
		SO_UNDIM_ARRAY = 12,			// SO_ARRAY_UNDIM

		SO_ASSIGN_STRING = 20,
		SO_ASSIGN_INT_LIST = 21,		// SO_ASSIGN_SCUMMVAR_LIST
		SO_ASSIGN_2DIM_LIST = 22,

		SO_WAIT_FOR_ACTOR = 30,
		SO_WAIT_FOR_MESSAGE = 31,
		SO_WAIT_FOR_CAMERA = 32,
		SO_WAIT_FOR_SENTENCE = 33,
		SO_WAIT_FOR_ANIMATION = 34,
		SO_WAIT_FOR_TURN = 35,

		SO_RESTART = 40,				// SO_SYSTEM_RESTART
		SO_QUIT = 41,					// SO_SYSTEM_QUIT

		SO_CAMERA_PAUSE = 50,
		SO_CAMERA_RESUME = 51,

		SO_HEAP_LOAD_CHARSET = 60,
		SO_HEAP_LOAD_COSTUME = 61,
		SO_HEAP_LOAD_OBJECT = 62,
		SO_HEAP_LOAD_ROOM = 63,
		SO_HEAP_LOAD_SCRIPT = 64,
		SO_HEAP_LOAD_SOUND = 65,
		SO_HEAP_LOCK_COSTUME = 66,
		SO_HEAP_LOCK_ROOM = 67,
		SO_HEAP_LOCK_SCRIPT = 68,
		SO_HEAP_LOCK_SOUND = 69,
		SO_HEAP_UNLOCK_COSTUME = 70,
		SO_HEAP_UNLOCK_ROOM = 71,
		SO_HEAP_UNLOCK_SCRIPT = 72,
		SO_HEAP_UNLOCK_SOUND = 73,
		SO_HEAP_NUKE_COSTUME = 74,
		SO_HEAP_NUKE_ROOM = 75,
		SO_HEAP_NUKE_SCRIPT = 76,
		SO_HEAP_NUKE_SOUND = 77,

		SO_ROOM_PALETTE = 82,

		SO_ROOM_FADE = 87,
		SO_ROOM_RGB_INTENSITY = 88,
		SO_ROOM_TRANSFORM = 89,

		SO_ROOM_NEW_PALETTE = 92,
		SO_ROOM_SAVE_GAME = 93,
		SO_ROOM_LOAD_GAME = 94,
		SO_ROOM_SATURATION = 95,

		SO_COSTUME = 100,
		SO_STEP_DIST = 101,

		SO_ANIMATION_DEFAULT = 103,		// SO_ACTOR_ANIMATION_DEFAULT
		SO_INIT_ANIMATION = 104,		// SO_ACTOR_ANIMATION_INIT
		SO_TALK_ANIMATION = 105,		// SO_ACTOR_ANIMATION_TALK
		SO_WALK_ANIMATION = 106,		// SO_ACTOR_ANIMATION_WALK
		SO_STAND_ANIMATION = 107,		// SO_ACTOR_ANIMATION_STAND
		SO_ANIMATION_SPEED = 108,		// SO_ACTOR_ANIMATION_SPEED
		SO_DEFAULT = 109,				// SO_ACTOR_DEFAULT
		SO_ELEVATION = 110,				// SO_ACTOR_ELEVATION
		SO_PALETTE = 111,				// SO_ACTOR_PALETTE
		SO_TALK_COLOR = 112,			// SO_ACTOR_TALK_COLOR
		SO_ACTOR_NAME = 113,
		SO_ACTOR_WIDTH = 114,
		SO_SCALE = 115,					// SO_ACTOR_SCALE
		SO_NEVER_ZCLIP = 116,			// SO_ACTOR_NEVER_ZCLIP
		SO_ALWAYS_ZCLIP = 117,			// SO_ACTOR_ALWAYS_ZCLIP
		SO_IGNORE_BOXES = 118,			// SO_ACTOR_IGNORE_BOXES
		SO_FOLLOW_BOXES = 119,			// SO_ACTOR_FOLLOW_BOXES
		SO_SHADOW = 120,				// SO_ACTOR_SPECIAL_DRAW
		SO_TEXT_OFFSET = 121,			// SO_ACTOR_TEXT_OFFSET
		SO_ACTOR_INIT = 122,
		SO_ACTOR_VARIABLE = 123,
		SO_ACTOR_IGNORE_TURNS_ON = 124,
		SO_ACTOR_IGNORE_TURNS_OFF = 125,
		SO_NEW = 126,					// SO_ACTOR_NEW
		SO_ACTOR_DEPTH = 127,
		SO_ACTOR_STOP = 128,
		SO_ACTOR_FACE = 129,
		SO_ACTOR_TURN = 130,
		SO_ACTOR_WALK_SCRIPT = 131,
		SO_ACTOR_TALK_SCRIPT = 132,
		SO_ACTOR_WALK_PAUSE = 133,
		SO_ACTOR_WALK_RESUME = 134,
		SO_ACTOR_VOLUME = 135,
		SO_ACTOR_FREQUENCY = 136,
		SO_ACTOR_PAN = 137,

		SO_VERB_INIT = 150,
		SO_VERB_NEW = 151,
		SO_VERB_DELETE = 152,
		SO_VERB_NAME = 153,
		SO_VERB_AT = 154,
		SO_VERB_ON = 155,
		SO_VERB_OFF = 156,
		SO_VERB_COLOR = 157,
		SO_VERB_HICOLOR = 158,

		SO_VERB_DIMCOLOR = 160,
		SO_VERB_DIM = 161,
		SO_VERB_KEY = 162,
		SO_VERB_IMAGE = 163,
		SO_VERB_NAME_STR = 164,
		SO_VERB_CENTER = 165,
		SO_VERB_CHARSET = 166,
		SO_VERB_LINE_SPACING = 167,

		SO_SAVE_VERBS = 180,			// SO_VERBS_SAVE
		SO_RESTORE_VERBS = 181,			// SO_VERBS_RESTORE
		SO_DELETE_VERBS = 182,			// SO_VERBS_DELETE

		SO_BASEOP = 200,				// SO_PRINT_BASEOP
		SO_END = 201,					// SO_PRINT_END
		SO_AT = 202,					// SO_PRINT_AT
		SO_COLOR = 203,					// SO_PRINT_COLOR
		SO_CENTER = 204,				// SO_PRINT_CENTER
		SO_PRINT_CHARSET = 205,
		SO_LEFT = 206,					// SO_PRINT_LEFT
		SO_OVERHEAD = 207,				// SO_PRINT_OVERHEAD
		SO_MUMBLE = 208,				// SO_PRINT_MUMBLE
		SO_PRINT_STRING = 209,
		SO_PRINT_WRAP = 210,

		SO_CURSOR_ON = 220,
		SO_CURSOR_OFF = 221,
		SO_CURSOR_SOFT_ON = 222,
		SO_CURSOR_SOFT_OFF = 223,
		SO_USERPUT_ON = 224,
		SO_USERPUT_OFF = 225,
		SO_USERPUT_SOFT_ON = 226,
		SO_USERPUT_SOFT_OFF = 227,
		SO_CURSOR_IMAGE = 228,
		SO_CURSOR_HOTSPOT = 229,
		SO_CURSOR_TRANSPARENT = 230,
		SO_CHARSET_SET = 231,
		SO_CHARSET_COLOR = 232,
		SO_CURSOR_PUT = 233,
	};

	struct ObjectNameId {
		char name[40];
		int id;
	};
	int _objectIDMapSize = 0;
	ObjectNameId *_objectIDMap = nullptr;

	struct StampShot {
		int slot;
		int boxX;
		int boxY;
		int boxWidth;
		int boxHeight;
		int brightness;
	};

	int _savegameThumbnailV8Palette[256] = {};
	byte _savegameThumbnailV8[160 * 120] = {}; // One fourth of the nominal 640x480 resolution
	StampShot _stampShots[20] = {};
	int _stampShotsInQueue = 0;

	int _keyScriptKey = 0, _keyScriptNo = 0;

public:
	ScummEngine_v8(OSystem *syst, const DetectorResult &dr);
	~ScummEngine_v8() override;

	void setKeyScriptVars(int _keyScriptKey, int _keyScriptNo);
	void stampShotDequeue();

protected:
	void setupOpcodes() override;

	void printString(int m, const byte *msg) override;

	void scummLoop_handleSaveLoad() override;

	void setupScummVars() override;
	void resetScummVars() override;
	void decodeParseString(int m, int n) override;
	void readArrayFromIndexFile() override;

	void readMAXS(int blockSize) override;
	void readGlobalObjects() override;

	uint fetchScriptWord() override;
	int fetchScriptWordSigned() override;

	int readVar(uint var) override;
	void writeVar(uint var, int value) override;

	int getObjectIdFromOBIM(const byte *obim) override;

	void processKeyboard(Common::KeyState lastKeyHit) override;
	void setDefaultCursor() override;
	void desaturatePalette(int hueScale, int satScale, int lightScale, int startColor, int endColor);

	void stampShotEnqueue(int slot, int boxX, int boxY, int boxWidth, int boxHeight, int brightness);
	void stampScreenShot(int slot, int boxX, int boxY, int boxWidth, int boxHeight, int brightness);
	void saveLoadWithSerializer(Common::Serializer &s) override;
	void createInternalSaveStateThumbnail();
	bool fetchInternalSaveStateThumbnail(int slotId, bool isHeapSave);
	uint32 *fetchScummVMSaveStateThumbnail(int slotId, bool isHeapSave, int brightness);

	const char *getGUIString(int stringId) override;

	/* Version 8 script opcodes */
	void o8_mod();
	void o8_wait();

	void o8_dimArray();
	void o8_dim2dimArray();
	void o8_arrayOps();
	void o8_blastText();

	void o8_cursorCommand();
	void o8_resourceRoutines();
	void o8_roomOps();
	void o8_actorOps();
	void o8_cameraOps();
	void o8_verbOps();

	void o8_systemOps();
	void o8_startVideo();
	void o8_kernelSetFunctions();
	void o8_kernelGetFunctions();

	void o8_getActorChore();
	void o8_getActorZPlane();

	void o8_drawObject();
	void o8_getObjectImageX();
	void o8_getObjectImageY();
	void o8_getObjectImageWidth();
	void o8_getObjectImageHeight();

	void o8_getStringWidth();

	byte VAR_LANGUAGE;
};

} // End of namespace Scumm

#endif // ENABLE_SCUMM_7_8

#endif
