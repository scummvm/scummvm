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

#ifndef TWP_IDS_H
#define TWP_IDS_H

#include "common/hashmap.h"

#define START_ACTORID 1000
#define END_ACTORID 2000
#define START_ROOMID 2000
#define END_ROOMID 3000
#define START_OBJECTID 3000
#define END_OBJECTID 100000
#define START_LIGHTID 100000
#define END_LIGHTID 200000
#define START_SOUNDDEFID 200000
#define END_SOUNDDEFID 250000
#define START_SOUNDID 250000
#define END_SOUNDID 300000
#define START_THREADID 300000
#define END_THREADID 8000000
#define START_CALLBACKID 8000000
#define END_CALLBACKID 10000000

#define ALL  1
#define HERE  0
#define GONE  4
#define OFF  0
#define ON  1
#define FULL  0
#define EMPTY  1
#define OPEN  1
#define CLOSED  0
#define FALSE  0
#define TRUE  1
#define MOUSE  1
#define CONTROLLER  2
#define DIRECTDRIVE  3
#define TOUCH  4
#define REMOTE  5
#define FADE_IN  0
#define FADE_OUT  1
#define FADE_WOBBLE  2
#define FADE_WOBBLE_TO_SEPIA  3
#define FACE_FLIP  16
#define DIR_FRONT  4
#define DIR_BACK  8
#define DIR_LEFT  2
#define DIR_RIGHT  1
#define LINEAR  0
#define EASE_IN  1
#define EASE_INOUT  2
#define EASE_OUT  3
#define SLOW_EASE_IN  4
#define SLOW_EASE_OUT  5
#define LOOPING  0x10
#define SWING  0X20
#define ALIGN_LEFT    0x0000000010000000
#define ALIGN_CENTER  0x0000000020000000
#define ALIGN_RIGHT   0x0000000040000000
#define ALIGN_TOP     0xFFFFFFFF80000000
#define ALIGN_BOTTOM  0x0000000001000000
#define LESS_SPACING  0x0000000000200000
#define EX_ALLOW_SAVEGAMES  0x01
#define EX_POP_CHARACTER_SELECTION  0x02
#define EX_CAMERA_TRACKING  0x03
#define EX_BUTTON_HOVER_SOUND  0x04
#define EX_RESTART  0x06
#define EX_IDLE_TIME  0x07
#define EX_AUTOSAVE  0x08
#define EX_AUTOSAVE_STATE  0x09
#define EX_DISABLE_SAVESYSTEM  0x0A
#define EX_SHOW_OPTIONS  11
#define EX_OPTIONS_MUSIC  12
#define EX_FORCE_TALKIE_TEXT  13
#define GRASS_BACKANDFORTH  0x00
#define DOOR  0x40
#define DOOR_LEFT  0x140
#define DOOR_RIGHT  0x240
#define DOOR_BACK  0x440
#define DOOR_FRONT  0x840
#define FAR_LOOK  0x8
#define USE_WITH  2
#define USE_ON  4
#define USE_IN  32
#define GIVEABLE  0x1000
#define TALKABLE  0x2000
#define IMMEDIATE  0x4000
#define FEMALE  0x80000
#define MALE  0x100000
#define PERSON  0x200000
#define REACH_HIGH  0x8000
#define REACH_MED  0x10000
#define REACH_LOW  0x20000
#define REACH_NONE  0x40000
#define VERB_WALKTO  1
#define VERB_LOOKAT  2
#define VERB_TALKTO  3
#define VERB_PICKUP  4
#define VERB_OPEN  5
#define VERB_CLOSE  6
#define VERB_PUSH  7
#define VERB_PULL  8
#define VERB_GIVE  9
#define VERB_USE  10
#define VERB_DIALOG  13
#define VERBFLAG_INSTANT  1
#define TWP_NO  0
#define TWP_YES  1
#define UNSELECTABLE  0
#define SELECTABLE  1
#define TEMP_UNSELECTABLE  2
#define TEMP_SELECTABLE  3
#define MAC  1
#define WIN  2
#define LINUX  3
#define XBOX  4
#define IOS  5
#define ANDROID  6
#define SWITCH  7
#define PS4  8
#define EFFECT_NONE          0
#define EFFECT_SEPIA         1
#define EFFECT_EGA           2
#define EFFECT_VHS           3
#define EFFECT_GHOST         4
#define EFFECT_BLACKANDWHITE 5
#define UI_INPUT_ON  1
#define UI_INPUT_OFF  2
#define UI_VERBS_ON  4
#define UI_VERBS_OFF  8
#define UI_HUDOBJECTS_ON  0x10
#define UI_HUDOBJECTS_OFF  0x20
#define UI_CURSOR_ON  0x40
#define UI_CURSOR_OFF  0x80

// these codes corresponds to SDL key codes used in TWP
#define KEY_UP  0x40000052
#define KEY_RIGHT  0x4000004F
#define KEY_DOWN  0x40000051
#define KEY_LEFT  0x40000050
#define KEY_PAD1  0x40000059
#define KEY_PAD2  0x4000005A
#define KEY_PAD3  0x4000005B
#define KEY_PAD4  0x4000005C
#define KEY_PAD5  0x4000005D
#define KEY_PAD6  0x4000005E
#define KEY_PAD7  0x4000005F
#define KEY_PAD8  0x40000056
#define KEY_PAD9  0x40000061
#define KEY_ESCAPE  0x08
#define KEY_TAB  0x09
#define KEY_RETURN  0x0D
#define KEY_BACKSPACE  0x1B
#define KEY_SPACE  0X20
#define KEY_A  0x61
#define KEY_B  0x62
#define KEY_C  0x63
#define KEY_D  0x64
#define KEY_E  0x65
#define KEY_F  0x66
#define KEY_G  0x67
#define KEY_H  0x68
#define KEY_I  0x69
#define KEY_J  0x6A
#define KEY_K  0x6B
#define KEY_L  0x6C
#define KEY_M  0x6D
#define KEY_N  0x6E
#define KEY_O  0x6F
#define KEY_P  0x70
#define KEY_Q  0x71
#define KEY_R  0x72
#define KEY_S  0x73
#define KEY_T  0x74
#define KEY_U  0x75
#define KEY_V  0x76
#define KEY_W  0x77
#define KEY_X  0x78
#define KEY_Y  0x79
#define KEY_Z  0x7A
#define KEY_0  0x30
#define KEY_1  0x31
#define KEY_2  0x32
#define KEY_3  0x33
#define KEY_4  0x34
#define KEY_5  0x35
#define KEY_6  0x36
#define KEY_7  0x37
#define KEY_8  0x38
#define KEY_9  0x39
#define KEY_F1  0x4000003A
#define KEY_F2  0x4000003B
#define KEY_F3  0x4000003C
#define KEY_F4  0x4000003D
#define KEY_F5  0x4000003E
#define KEY_F6  0x4000003F
#define KEY_F7  0x40000040
#define KEY_F8  0x40000041
#define KEY_F9  0x40000042
#define KEY_F10  0x40000043
#define KEY_F11  0x40000044
#define KEY_F12  0x40000045

#define BUTTON_A  0x3E8
#define BUTTON_B  0x3E9
#define BUTTON_X  0x3EA
#define BUTTON_Y  0x3EB
#define BUTTON_START  0x3EC
#define BUTTON_BACK  0x3EC
#define BUTTON_MOUSE_LEFT  0x3ED
#define BUTTON_MOUSE_RIGHT  0x3EE

namespace Twp {

enum Facing {
	FACE_RIGHT = 1,
	FACE_LEFT = 2,
	FACE_FRONT = 4,
	FACE_BACK = 8
};

}

namespace Common {
template<> struct Hash<Twp::Facing> : public UnaryFunction<Twp::Facing, uint> {
	uint operator()(Twp::Facing val) const { return (uint)val; }
};
}


namespace Twp {

bool isThread(int id);
bool isRoom(int id);
bool isActor(int id);
bool isObject(int id);
bool isSound(int id);
bool isLight(int id);
bool isCallback(int id);

int newRoomId();
int newObjId();
int newActorId();
int newSoundDefId();
int newSoundId();
int newThreadId();
int newCallbackId();
int newLightId();
void setCallbackId(int id);
int getCallbackId();

Facing getOppositeFacing(Facing facing);

} // namespace Twp

#endif
