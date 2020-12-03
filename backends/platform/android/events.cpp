/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#if defined(__ANDROID__)

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(printf, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <android/input.h>

#include "backends/platform/android/android.h"
#include "backends/platform/android/graphics.h"
#include "backends/platform/android/jni-android.h"

// floating point. use sparingly
template<class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

static const int kQueuedInputEventDelay = 50;

// event type
enum {
	JE_SYS_KEY = 0,
	JE_KEY = 1,
	JE_DPAD = 2,
	JE_DOWN = 3,
	JE_SCROLL = 4,
	JE_TAP = 5,
	JE_DOUBLE_TAP = 6,
	JE_MULTI = 7,
	JE_BALL = 8,
	JE_LMB_DOWN = 9,
	JE_LMB_UP = 10,
	JE_RMB_DOWN = 11,
	JE_RMB_UP = 12,
	JE_MOUSE_MOVE = 13,
	JE_GAMEPAD = 14,
	JE_JOYSTICK = 15,
	JE_MMB_DOWN = 16,
	JE_MMB_UP = 17,
	JE_BMB_DOWN = 18,
	JE_BMB_UP = 19,
	JE_FMB_DOWN = 20,
	JE_FMB_UP = 21,
	JE_QUIT = 0x1000
};

// meta modifier
enum {
	AMETA_CTRL_MASK  = AMETA_CTRL_ON  | AMETA_CTRL_LEFT_ON  | AMETA_CTRL_RIGHT_ON,
	AMETA_META_MASK  = AMETA_META_ON  | AMETA_META_LEFT_ON  | AMETA_META_RIGHT_ON,
	AMETA_SHIFT_MASK = AMETA_SHIFT_ON | AMETA_SHIFT_LEFT_ON | AMETA_SHIFT_RIGHT_ON,
	AMETA_ALT_MASK   = AMETA_ALT_ON   | AMETA_ALT_LEFT_ON   | AMETA_ALT_RIGHT_ON
};

// map android key codes to our kbd codes (common/keyboard.h)
static const Common::KeyCode jkeymap[] = {
	Common::KEYCODE_INVALID, // AKEYCODE_UNKNOWN
	Common::KEYCODE_LEFTSOFT, // AKEYCODE_SOFT_LEFT
	Common::KEYCODE_RIGHTSOFT, // AKEYCODE_SOFT_RIGHT
	Common::KEYCODE_AC_HOME, // AKEYCODE_HOME
	Common::KEYCODE_AC_BACK, // AKEYCODE_BACK
	Common::KEYCODE_CALL, // AKEYCODE_CALL
	Common::KEYCODE_HANGUP, // AKEYCODE_ENDCALL
	Common::KEYCODE_0, // AKEYCODE_0
	Common::KEYCODE_1, // AKEYCODE_1
	Common::KEYCODE_2, // AKEYCODE_2
	Common::KEYCODE_3, // AKEYCODE_3
	Common::KEYCODE_4, // AKEYCODE_4
	Common::KEYCODE_5, // AKEYCODE_5
	Common::KEYCODE_6, // AKEYCODE_6
	Common::KEYCODE_7, // AKEYCODE_7
	Common::KEYCODE_8, // AKEYCODE_8
	Common::KEYCODE_9, // AKEYCODE_9
	Common::KEYCODE_ASTERISK, // AKEYCODE_STAR
	Common::KEYCODE_HASH, // AKEYCODE_POUND
	Common::KEYCODE_UP, // AKEYCODE_DPAD_UP
	Common::KEYCODE_DOWN, // AKEYCODE_DPAD_DOWN
	Common::KEYCODE_LEFT, // AKEYCODE_DPAD_LEFT
	Common::KEYCODE_RIGHT, // AKEYCODE_DPAD_RIGHT
	Common::KEYCODE_SELECT, // AKEYCODE_DPAD_CENTER
	Common::KEYCODE_VOLUMEUP, // AKEYCODE_VOLUME_UP
	Common::KEYCODE_VOLUMEDOWN, // AKEYCODE_VOLUME_DOWN
	Common::KEYCODE_POWER, // AKEYCODE_POWER
	Common::KEYCODE_CAMERA, // AKEYCODE_CAMERA
	Common::KEYCODE_CLEAR, // AKEYCODE_CLEAR
	Common::KEYCODE_a, // AKEYCODE_A
	Common::KEYCODE_b, // AKEYCODE_B
	Common::KEYCODE_c, // AKEYCODE_C
	Common::KEYCODE_d, // AKEYCODE_D
	Common::KEYCODE_e, // AKEYCODE_E
	Common::KEYCODE_f, // AKEYCODE_F
	Common::KEYCODE_g, // AKEYCODE_G
	Common::KEYCODE_h, // AKEYCODE_H
	Common::KEYCODE_i, // AKEYCODE_I
	Common::KEYCODE_j, // AKEYCODE_J
	Common::KEYCODE_k, // AKEYCODE_K
	Common::KEYCODE_l, // AKEYCODE_L
	Common::KEYCODE_m, // AKEYCODE_M
	Common::KEYCODE_n, // AKEYCODE_N
	Common::KEYCODE_o, // AKEYCODE_O
	Common::KEYCODE_p, // AKEYCODE_P
	Common::KEYCODE_q, // AKEYCODE_Q
	Common::KEYCODE_r, // AKEYCODE_R
	Common::KEYCODE_s, // AKEYCODE_S
	Common::KEYCODE_t, // AKEYCODE_T
	Common::KEYCODE_u, // AKEYCODE_U
	Common::KEYCODE_v, // AKEYCODE_V
	Common::KEYCODE_w, // AKEYCODE_W
	Common::KEYCODE_x, // AKEYCODE_X
	Common::KEYCODE_y, // AKEYCODE_Y
	Common::KEYCODE_z, // AKEYCODE_Z
	Common::KEYCODE_COMMA, // AKEYCODE_COMMA
	Common::KEYCODE_PERIOD, // AKEYCODE_PERIOD
	Common::KEYCODE_LALT, // AKEYCODE_ALT_LEFT
	Common::KEYCODE_RALT, // AKEYCODE_ALT_RIGHT
	Common::KEYCODE_LSHIFT, // AKEYCODE_SHIFT_LEFT
	Common::KEYCODE_RSHIFT, // AKEYCODE_SHIFT_RIGHT
	Common::KEYCODE_TAB, // AKEYCODE_TAB
	Common::KEYCODE_SPACE, // AKEYCODE_SPACE
	Common::KEYCODE_LCTRL, // AKEYCODE_SYM
	Common::KEYCODE_WWW, // AKEYCODE_EXPLORER
	Common::KEYCODE_MAIL, // AKEYCODE_ENVELOPE
	Common::KEYCODE_RETURN, // AKEYCODE_ENTER
	Common::KEYCODE_BACKSPACE, // AKEYCODE_DEL
	Common::KEYCODE_BACKQUOTE, // AKEYCODE_GRAVE
	Common::KEYCODE_MINUS, // AKEYCODE_MINUS
	Common::KEYCODE_EQUALS, // AKEYCODE_EQUALS
	Common::KEYCODE_LEFTPAREN, // AKEYCODE_LEFT_BRACKET
	Common::KEYCODE_RIGHTPAREN, // AKEYCODE_RIGHT_BRACKET
	Common::KEYCODE_BACKSLASH, // AKEYCODE_BACKSLASH
	Common::KEYCODE_SEMICOLON, // AKEYCODE_SEMICOLON
	Common::KEYCODE_QUOTE, // AKEYCODE_APOSTROPHE
	Common::KEYCODE_SLASH, // AKEYCODE_SLASH
	Common::KEYCODE_AT, // AKEYCODE_AT
	Common::KEYCODE_INVALID, // AKEYCODE_NUM
	Common::KEYCODE_INVALID, // AKEYCODE_HEADSETHOOK
	Common::KEYCODE_INVALID, // AKEYCODE_FOCUS
	Common::KEYCODE_PLUS, // AKEYCODE_PLUS
	Common::KEYCODE_MENU, // AKEYCODE_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_NOTIFICATION
	Common::KEYCODE_AC_SEARCH, // AKEYCODE_SEARCH
	Common::KEYCODE_AUDIOPLAYPAUSE, // AKEYCODE_MEDIA_PLAY_PAUSE
	Common::KEYCODE_AUDIOSTOP, // AKEYCODE_MEDIA_STOP
	Common::KEYCODE_AUDIONEXT, // AKEYCODE_MEDIA_NEXT
	Common::KEYCODE_AUDIOPREV, // AKEYCODE_MEDIA_PREVIOUS
	Common::KEYCODE_AUDIOREWIND, // AKEYCODE_MEDIA_REWIND
	Common::KEYCODE_AUDIOFASTFORWARD, // AKEYCODE_MEDIA_FAST_FORWARD
	Common::KEYCODE_MUTE, // AKEYCODE_MUTE
	Common::KEYCODE_PAGEUP, // AKEYCODE_PAGE_UP
	Common::KEYCODE_PAGEDOWN, // AKEYCODE_PAGE_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_PICTSYMBOLS
	Common::KEYCODE_INVALID, // AKEYCODE_SWITCH_CHARSET
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_A
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_B
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_C
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_X
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_Y
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_Z
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_L1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_R1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_L2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_R2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_THUMBL
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_THUMBR
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_START
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_SELECT
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_MODE
	Common::KEYCODE_ESCAPE, // AKEYCODE_ESCAPE
	Common::KEYCODE_DELETE, // AKEYCODE_FORWARD_DEL
	Common::KEYCODE_LCTRL, // AKEYCODE_CTRL_LEFT
	Common::KEYCODE_RCTRL, // AKEYCODE_CTRL_RIGHT
	Common::KEYCODE_CAPSLOCK, // AKEYCODE_CAPS_LOCK
	Common::KEYCODE_SCROLLOCK, // AKEYCODE_SCROLL_LOCK
	Common::KEYCODE_LSUPER, // AKEYCODE_META_LEFT
	Common::KEYCODE_RSUPER, // AKEYCODE_META_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_FUNCTION
	Common::KEYCODE_SYSREQ, // AKEYCODE_SYSRQ
	Common::KEYCODE_BREAK, // AKEYCODE_BREAK
	Common::KEYCODE_HOME, // AKEYCODE_MOVE_HOME
	Common::KEYCODE_END, // AKEYCODE_MOVE_END
	Common::KEYCODE_INSERT, // AKEYCODE_INSERT
	Common::KEYCODE_AC_FORWARD, // AKEYCODE_FORWARD
	Common::KEYCODE_AUDIOPLAY, // AKEYCODE_MEDIA_PLAY
	Common::KEYCODE_AUDIOPAUSE, // AKEYCODE_MEDIA_PAUSE
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_CLOSE
	Common::KEYCODE_EJECT, // AKEYCODE_MEDIA_EJECT
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_RECORD
	Common::KEYCODE_F1, // AKEYCODE_F1
	Common::KEYCODE_F2, // AKEYCODE_F2
	Common::KEYCODE_F3, // AKEYCODE_F3
	Common::KEYCODE_F4, // AKEYCODE_F4
	Common::KEYCODE_F5, // AKEYCODE_F5
	Common::KEYCODE_F6, // AKEYCODE_F6
	Common::KEYCODE_F7, // AKEYCODE_F7
	Common::KEYCODE_F8, // AKEYCODE_F8
	Common::KEYCODE_F9, // AKEYCODE_F9
	Common::KEYCODE_F10, // AKEYCODE_F10
	Common::KEYCODE_F11, // AKEYCODE_F11
	Common::KEYCODE_F12, // AKEYCODE_F12
	Common::KEYCODE_NUMLOCK, // AKEYCODE_NUM_LOCK
	Common::KEYCODE_KP0, // AKEYCODE_NUMPAD_0
	Common::KEYCODE_KP1, // AKEYCODE_NUMPAD_1
	Common::KEYCODE_KP2, // AKEYCODE_NUMPAD_2
	Common::KEYCODE_KP3, // AKEYCODE_NUMPAD_3
	Common::KEYCODE_KP4, // AKEYCODE_NUMPAD_4
	Common::KEYCODE_KP5, // AKEYCODE_NUMPAD_5
	Common::KEYCODE_KP6, // AKEYCODE_NUMPAD_6
	Common::KEYCODE_KP7, // AKEYCODE_NUMPAD_7
	Common::KEYCODE_KP8, // AKEYCODE_NUMPAD_8
	Common::KEYCODE_KP9, // AKEYCODE_NUMPAD_9
	Common::KEYCODE_KP_DIVIDE, // AKEYCODE_NUMPAD_DIVIDE
	Common::KEYCODE_KP_MULTIPLY, // AKEYCODE_NUMPAD_MULTIPLY
	Common::KEYCODE_KP_MINUS, // AKEYCODE_NUMPAD_SUBTRACT
	Common::KEYCODE_KP_PLUS, // AKEYCODE_NUMPAD_ADD
	Common::KEYCODE_KP_PERIOD, // AKEYCODE_NUMPAD_DOT
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_COMMA
	Common::KEYCODE_KP_ENTER, // AKEYCODE_NUMPAD_ENTER
	Common::KEYCODE_KP_EQUALS, // AKEYCODE_NUMPAD_EQUALS
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_LEFT_PAREN
	Common::KEYCODE_INVALID, // AKEYCODE_NUMPAD_RIGHT_PAREN
	Common::KEYCODE_INVALID, // AKEYCODE_VOLUME_MUTE
	Common::KEYCODE_INVALID, // AKEYCODE_INFO
	Common::KEYCODE_INVALID, // AKEYCODE_CHANNEL_UP
	Common::KEYCODE_INVALID, // AKEYCODE_CHANNEL_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_ZOOM_IN
	Common::KEYCODE_INVALID, // AKEYCODE_ZOOM_OUT
	Common::KEYCODE_INVALID, // AKEYCODE_TV
	Common::KEYCODE_INVALID, // AKEYCODE_WINDOW
	Common::KEYCODE_INVALID, // AKEYCODE_GUIDE
	Common::KEYCODE_INVALID, // AKEYCODE_DVR
	Common::KEYCODE_AC_BOOKMARKS, // AKEYCODE_BOOKMARK
	Common::KEYCODE_INVALID, // AKEYCODE_CAPTIONS
	Common::KEYCODE_INVALID, // AKEYCODE_SETTINGS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_STB_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_STB_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_AVR_POWER
	Common::KEYCODE_INVALID, // AKEYCODE_AVR_INPUT
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_RED
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_GREEN
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_YELLOW
	Common::KEYCODE_INVALID, // AKEYCODE_PROG_BLUE
	Common::KEYCODE_INVALID, // AKEYCODE_APP_SWITCH
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_1
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_2
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_3
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_4
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_5
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_6
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_7
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_8
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_9
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_10
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_11
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_12
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_13
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_14
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_15
	Common::KEYCODE_INVALID, // AKEYCODE_BUTTON_16
	Common::KEYCODE_INVALID, // AKEYCODE_LANGUAGE_SWITCH
	Common::KEYCODE_INVALID, // AKEYCODE_MANNER_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_3D_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_CONTACTS
	Common::KEYCODE_INVALID, // AKEYCODE_CALENDAR
	Common::KEYCODE_INVALID, // AKEYCODE_MUSIC
	Common::KEYCODE_CALCULATOR, // AKEYCODE_CALCULATOR
	Common::KEYCODE_INVALID, // AKEYCODE_ZENKAKU_HANKAKU
	Common::KEYCODE_INVALID, // AKEYCODE_EISU
	Common::KEYCODE_INVALID, // AKEYCODE_MUHENKAN
	Common::KEYCODE_INVALID, // AKEYCODE_HENKAN
	Common::KEYCODE_INVALID, // AKEYCODE_KATAKANA_HIRAGANA
	Common::KEYCODE_INVALID, // AKEYCODE_YEN
	Common::KEYCODE_INVALID, // AKEYCODE_RO
	Common::KEYCODE_INVALID, // AKEYCODE_KANA
	Common::KEYCODE_INVALID, // AKEYCODE_ASSIST
	Common::KEYCODE_INVALID, // AKEYCODE_BRIGHTNESS_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_BRIGHTNESS_UP
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_AUDIO_TRACK
	Common::KEYCODE_SLEEP, // AKEYCODE_SLEEP
	Common::KEYCODE_INVALID, // AKEYCODE_WAKEUP
	Common::KEYCODE_INVALID, // AKEYCODE_PAIRING
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_TOP_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_11
	Common::KEYCODE_INVALID, // AKEYCODE_12
	Common::KEYCODE_INVALID, // AKEYCODE_LAST_CHANNEL
	Common::KEYCODE_INVALID, // AKEYCODE_TV_DATA_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_VOICE_ASSIST
	Common::KEYCODE_INVALID, // AKEYCODE_TV_RADIO_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TELETEXT
	Common::KEYCODE_INVALID, // AKEYCODE_TV_NUMBER_ENTRY
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TERRESTRIAL_ANALOG
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TERRESTRIAL_DIGITAL
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_BS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_CS
	Common::KEYCODE_INVALID, // AKEYCODE_TV_SATELLITE_SERVICE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_NETWORK
	Common::KEYCODE_INVALID, // AKEYCODE_TV_ANTENNA_CABLE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_3
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_HDMI_4
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPOSITE_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPOSITE_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPONENT_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_COMPONENT_2
	Common::KEYCODE_INVALID, // AKEYCODE_TV_INPUT_VGA_1
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_UP
	Common::KEYCODE_INVALID, // AKEYCODE_TV_AUDIO_DESCRIPTION_MIX_DOWN
	Common::KEYCODE_INVALID, // AKEYCODE_TV_ZOOM_MODE
	Common::KEYCODE_INVALID, // AKEYCODE_TV_CONTENTS_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_TV_MEDIA_CONTEXT_MENU
	Common::KEYCODE_INVALID, // AKEYCODE_TV_TIMER_PROGRAMMING
	Common::KEYCODE_HELP, // AKEYCODE_HELP
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_PREVIOUS
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_NEXT
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_IN
	Common::KEYCODE_INVALID, // AKEYCODE_NAVIGATE_OUT
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_PRIMARY
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_1
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_2
	Common::KEYCODE_INVALID, // AKEYCODE_STEM_3
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_UP_LEFT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_DOWN_LEFT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_UP_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_DPAD_DOWN_RIGHT
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_SKIP_FORWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_SKIP_BACKWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_STEP_FORWARD
	Common::KEYCODE_INVALID, // AKEYCODE_MEDIA_STEP_BACKWARD
	Common::KEYCODE_INVALID, // AKEYCODE_SOFT_SLEEP
	Common::KEYCODE_CUT, // AKEYCODE_CUT
	Common::KEYCODE_COPY, // AKEYCODE_COPY
	Common::KEYCODE_PASTE // AKEYCODE_PASTE
};

void OSystem_Android::pushEvent(int type, int arg1, int arg2, int arg3,
								int arg4, int arg5, int arg6) {
	Common::Event e;

	switch (type) {
	case JE_SYS_KEY:
	case JE_KEY:
		switch (arg1) {
		case AKEY_EVENT_ACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case AKEY_EVENT_ACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on key: %d", arg1);
			return;
		}

		if (arg2 < 1 || arg2 > ARRAYSIZE(jkeymap)) {
			if (arg3 < 1) {
				LOGE("received invalid keycode: %d (%d)", arg2, arg3);
				return;
			} else {
				// lets bet on the ascii code
				e.kbd.keycode = Common::KEYCODE_INVALID;
			}
		} else {
			e.kbd.keycode = jkeymap[arg2];
		}

		if (arg5 > 0)
			e.kbdRepeat = true;

		// map special keys to 'our' ascii codes
		switch (e.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			LOGD("received BACKSPACE");
			e.kbd.ascii = Common::ASCII_BACKSPACE;
			break;
		case Common::KEYCODE_TAB:
			e.kbd.ascii = Common::ASCII_TAB;
			break;
		case Common::KEYCODE_RETURN:
			e.kbd.ascii = Common::ASCII_RETURN;
			break;
		case Common::KEYCODE_ESCAPE:
			e.kbd.ascii = Common::ASCII_ESCAPE;
			break;
		case Common::KEYCODE_SPACE:
			e.kbd.ascii = Common::ASCII_SPACE;
			break;
		case Common::KEYCODE_F1:
			e.kbd.ascii = Common::ASCII_F1;
			break;
		case Common::KEYCODE_F2:
			e.kbd.ascii = Common::ASCII_F2;
			break;
		case Common::KEYCODE_F3:
			e.kbd.ascii = Common::ASCII_F3;
			break;
		case Common::KEYCODE_F4:
			e.kbd.ascii = Common::ASCII_F4;
			break;
		case Common::KEYCODE_F5:
			e.kbd.ascii = Common::ASCII_F5;
			break;
		case Common::KEYCODE_F6:
			e.kbd.ascii = Common::ASCII_F6;
			break;
		case Common::KEYCODE_F7:
			e.kbd.ascii = Common::ASCII_F7;
			break;
		case Common::KEYCODE_F8:
			e.kbd.ascii = Common::ASCII_F8;
			break;
		case Common::KEYCODE_F9:
			e.kbd.ascii = Common::ASCII_F9;
			break;
		case Common::KEYCODE_F10:
			e.kbd.ascii = Common::ASCII_F10;
			break;
		case Common::KEYCODE_F11:
			e.kbd.ascii = Common::ASCII_F11;
			break;
		case Common::KEYCODE_F12:
			e.kbd.ascii = Common::ASCII_F12;
			break;
		default:
			e.kbd.ascii = arg3;
			break;
		}

		// arg4 is the metastate of the key press event
		// check for "Shift" key modifier
		if (arg4 & AMETA_SHIFT_MASK)
			e.kbd.flags |= Common::KBD_SHIFT;

		// We revert the commit to disable the Alt modifier
		// TODO revisit this old comment from commit https://github.com/scummvm/scummvm/commit/bfecb37501b6fdc35f2802216db5fb2b0e54b8ee
		//      for possible issues with physical keyboards and Fn key combos.
		//      It still seems like a bad idea to disable Alt key combos over Fn key combos
		//      and our keymapper should be able to handle this issue anyway
		//      (since it explicitly shows what key combination results from pressing keys, when the user edits a key mapping)
		//      More info: Back then we were checking for JMETA_ALT (defined as 0x02) which is now NDK's AMETA_ALT_ON (0x02) (see https://developer.android.com/ndk/reference/group/input)
		//      JMETA_ALT was defined in our own enum. (see full old file: https://github.com/scummvm/scummvm/blob/bfecb37501b6fdc35f2802216db5fb2b0e54b8ee/backends/platform/android/events.cpp#L113)
		// Old comment:
		// JMETA_ALT (0x02) is Fn on physical keyboards!
		// when mapping this to ALT - as we know it from PC keyboards - all
		// Fn combos will be broken (like Fn+q, which needs to end as 1 and
		// not ALT+1). Do not want.
		//if (arg4 & JMETA_ALT)
		//	e.kbd.flags |= Common::KBD_ALT;
		// end of old comment --
		// check for "Alt" key modifier
		if (arg4 & (AMETA_ALT_MASK)) {
			e.kbd.flags |= Common::KBD_ALT;
		}

		// check for "Ctrl" key modifier (We set Sym key to also behave as Ctrl)
		if (arg4 & (AMETA_SYM_ON | AMETA_CTRL_MASK)) {
			e.kbd.flags |= Common::KBD_CTRL;
		}

		// check for "Meta" key modifier
		if (arg4 & (AMETA_META_MASK)) {
			e.kbd.flags |= Common::KBD_META;
		}

		//  check for CAPS key modifier
		if (arg4 & (AMETA_CAPS_LOCK_ON)) {
			e.kbd.flags |= Common::KBD_CAPS;
		}

		//  check for NUM Lock key modifier
		if (arg4 & (AMETA_NUM_LOCK_ON)) {
			e.kbd.flags |= Common::KBD_NUM;
		}

		//  check for Scroll Lock key modifier
		if (arg4 & (AMETA_SCROLL_LOCK_ON)) {
			e.kbd.flags |= Common::KBD_SCRL;
		}

		pushEvent(e);

		return;

	case JE_DPAD:
		switch (arg2) {
		case AKEYCODE_DPAD_UP:
		case AKEYCODE_DPAD_DOWN:
		case AKEYCODE_DPAD_LEFT:
		case AKEYCODE_DPAD_RIGHT:
			if (arg1 != AKEY_EVENT_ACTION_DOWN)
				return;

			e.type = Common::EVENT_MOUSEMOVE;

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			{
				int16 *c;
				int s;

				if (arg2 == AKEYCODE_DPAD_UP || arg2 == AKEYCODE_DPAD_DOWN) {
					c = &e.mouse.y;
					s = _eventScaleY;
				} else {
					c = &e.mouse.x;
					s = _eventScaleX;
				}

				// the longer the button held, the faster the pointer is
				// TODO put these values in some option dlg?
				int f = CLIP(arg5, 1, 8) * _dpad_scale * 100 / s;

				if (arg2 == AKEYCODE_DPAD_UP || arg2 == AKEYCODE_DPAD_LEFT)
					*c -= f;
				else
					*c += f;
			}

			pushEvent(e);

			return;

		case AKEYCODE_DPAD_CENTER:
			switch (arg1) {
			case AKEY_EVENT_ACTION_DOWN:
				e.type = Common::EVENT_LBUTTONDOWN;
				break;
			case AKEY_EVENT_ACTION_UP:
				e.type = Common::EVENT_LBUTTONUP;
				break;
			default:
				LOGE("unhandled jaction on dpad key: %d", arg1);
				return;
			}

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			pushEvent(e);

			return;
		}

	case JE_DOWN:
		_touch_pt_down = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
		break;

	case JE_SCROLL:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			e.mouse.x = (arg3 - _touch_pt_scroll.x) * 100 / _touchpad_scale;
			e.mouse.y = (arg4 - _touch_pt_scroll.y) * 100 / _touchpad_scale;
			e.mouse += _touch_pt_down;

		} else {
			e.mouse.x = arg3;
			e.mouse.y = arg4;
		}

		pushEvent(e);

		return;

	case JE_TAP:
		if (_fingersDown > 0) {
			_fingersDown = 0;
			return;
		}

		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		} else {
			e.mouse.x = arg1;
			e.mouse.y = arg2;
		}

		{
			Common::EventType down, up;

			// TODO put these values in some option dlg?
			if (arg3 > 1000) {
				down = Common::EVENT_MBUTTONDOWN;
				up = Common::EVENT_MBUTTONUP;
			} else if (arg3 > 500) {
				down = Common::EVENT_RBUTTONDOWN;
				up = Common::EVENT_RBUTTONUP;
			} else {
				down = Common::EVENT_LBUTTONDOWN;
				up = Common::EVENT_LBUTTONUP;
			}

			_event_queue_lock->lock();

			if (_queuedEventTime)
				_event_queue.push(_queuedEvent);

			if (!_touchpad_mode)
				_event_queue.push(e);

			e.type = down;
			_event_queue.push(e);

			e.type = up;
			_queuedEvent = e;
			_queuedEventTime = getMillis() + kQueuedInputEventDelay;

			_event_queue_lock->unlock();
		}

		return;

	case JE_DOUBLE_TAP:
		e.type = Common::EVENT_MOUSEMOVE;

		if (_touchpad_mode) {
			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
		} else {
			e.mouse.x = arg1;
			e.mouse.y = arg2;
		}

		{
			Common::EventType dptype = Common::EVENT_INVALID;

			switch (arg3) {
			case AMOTION_EVENT_ACTION_DOWN:
				dptype = Common::EVENT_LBUTTONDOWN;
				_touch_pt_dt.x = -1;
				_touch_pt_dt.y = -1;
				break;
			case AMOTION_EVENT_ACTION_UP:
				dptype = Common::EVENT_LBUTTONUP;
				break;
			// held and moved
			case AMOTION_EVENT_ACTION_MOVE:
				if (_touch_pt_dt.x == -1 && _touch_pt_dt.y == -1) {
					_touch_pt_dt.x = arg1;
					_touch_pt_dt.y = arg2;
					return;
				}

				dptype = Common::EVENT_MOUSEMOVE;

				if (_touchpad_mode) {
					e.mouse.x = (arg1 - _touch_pt_dt.x) * 100 / _touchpad_scale;
					e.mouse.y = (arg2 - _touch_pt_dt.y) * 100 / _touchpad_scale;
					e.mouse += _touch_pt_down;
				}

				break;
			default:
				LOGE("unhandled jaction on double tap: %d", arg3);
				return;
			}

			_event_queue_lock->lock();
			_event_queue.push(e);
			e.type = dptype;
			_event_queue.push(e);
			_event_queue_lock->unlock();
		}

		return;

	case JE_MULTI:
		switch (arg2) {
		case AMOTION_EVENT_ACTION_POINTER_DOWN:
			if (arg1 > _fingersDown)
				_fingersDown = arg1;

			return;

		case AMOTION_EVENT_ACTION_POINTER_UP:
			if (arg1 != _fingersDown)
				return;

			{
				Common::EventType up;

				switch (_fingersDown) {
				case 1:
					e.type = Common::EVENT_RBUTTONDOWN;
					up = Common::EVENT_RBUTTONUP;
					break;
				case 2:
					e.type = Common::EVENT_MBUTTONDOWN;
					up = Common::EVENT_MBUTTONUP;
					break;
				default:
					LOGD("unmapped multi tap: %d", _fingersDown);
					return;
				}

				e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

				_event_queue_lock->lock();

				if (_queuedEventTime)
					_event_queue.push(_queuedEvent);

				_event_queue.push(e);

				e.type = up;
				_queuedEvent = e;
				_queuedEventTime = getMillis() + kQueuedInputEventDelay;

				_event_queue_lock->unlock();
				return;

			default:
				LOGE("unhandled jaction on multi tap: %d", arg2);
				return;
			}
		}

		return;

	case JE_BALL:
		e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

		switch (arg1) {
		case AMOTION_EVENT_ACTION_DOWN:
			e.type = Common::EVENT_LBUTTONDOWN;
			break;
		case AMOTION_EVENT_ACTION_UP:
			e.type = Common::EVENT_LBUTTONUP;
			break;
		case AMOTION_EVENT_ACTION_MOVE:
			e.type = Common::EVENT_MOUSEMOVE;

			// already multiplied by 100
			e.mouse.x += arg2 * _trackball_scale / _eventScaleX;
			e.mouse.y += arg3 * _trackball_scale / _eventScaleY;

			break;
		default:
			LOGE("unhandled jaction on system key: %d", arg1);
			return;
		}

		pushEvent(e);

		return;

	case JE_MOUSE_MOVE:
		e.type = Common::EVENT_MOUSEMOVE;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_LMB_DOWN:
		e.type = Common::EVENT_LBUTTONDOWN;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_LMB_UP:
		e.type = Common::EVENT_LBUTTONUP;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_RMB_DOWN:
		e.type = Common::EVENT_RBUTTONDOWN;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_RMB_UP:
		e.type = Common::EVENT_RBUTTONUP;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_MMB_DOWN:
		e.type = Common::EVENT_MBUTTONDOWN;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_MMB_UP:
		e.type = Common::EVENT_MBUTTONUP;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_BMB_DOWN:
		e.type = Common::EVENT_X1BUTTONDOWN;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_BMB_UP:
		e.type = Common::EVENT_X1BUTTONUP;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_FMB_DOWN:
		e.type = Common::EVENT_X2BUTTONDOWN;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_FMB_UP:
		e.type = Common::EVENT_X2BUTTONUP;
		e.mouse.x = arg1;
		e.mouse.y = arg2;

		pushEvent(e);

		return;

	case JE_GAMEPAD:
		switch (arg1) {
		case AKEY_EVENT_ACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case AKEY_EVENT_ACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on gamepad key: %d", arg1);
			return;
		}

		switch (arg2) {
		case AKEYCODE_BUTTON_A:
		case AKEYCODE_BUTTON_B:
			switch (arg1) {
			case AKEY_EVENT_ACTION_DOWN:
				e.type = (arg2 == AKEYCODE_BUTTON_A?
					  Common::EVENT_LBUTTONDOWN :
					  Common::EVENT_RBUTTONDOWN);
				break;
			case AKEY_EVENT_ACTION_UP:
				e.type = (arg2 == AKEYCODE_BUTTON_A?
					  Common::EVENT_LBUTTONUP :
					  Common::EVENT_RBUTTONUP);
				break;
			}

			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

			break;

		case AKEYCODE_BUTTON_X:
			e.kbd.keycode = Common::KEYCODE_ESCAPE;
			e.kbd.ascii = Common::ASCII_ESCAPE;
			break;

		case AKEYCODE_BUTTON_Y:
			e.type = Common::EVENT_MAINMENU;
			break;

		default:
			LOGW("unmapped gamepad key: %d", arg2);
			return;
		}

		pushEvent(e);

		break;

	case JE_JOYSTICK:

		switch (arg1) {
		// AMOTION_EVENT_ACTION_MOVE is 2 in NDK (https://developer.android.com/ndk/reference/group/input)
		case AMOTION_EVENT_ACTION_MOVE:
			e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();
			e.type = Common::EVENT_MOUSEMOVE;

			// already multiplied by 100
			e.mouse.x += arg2 * _joystick_scale / _eventScaleX;
			e.mouse.y += arg3 * _joystick_scale / _eventScaleY;

			break;
		case AKEY_EVENT_ACTION_DOWN:
			e.type = Common::EVENT_KEYDOWN;
			break;
		case AKEY_EVENT_ACTION_UP:
			e.type = Common::EVENT_KEYUP;
			break;
		default:
			LOGE("unhandled jaction on joystick: %d", arg1);
			return;
		}

		if (arg1 != AMOTION_EVENT_ACTION_MOVE) {
			switch (arg2) {
			case AKEYCODE_BUTTON_1:
			case AKEYCODE_BUTTON_2:
				switch (arg1) {
				case AKEY_EVENT_ACTION_DOWN:
					e.type = (arg2 == AKEYCODE_BUTTON_1?
						  Common::EVENT_LBUTTONDOWN :
						  Common::EVENT_RBUTTONDOWN);
					break;
				case AKEY_EVENT_ACTION_UP:
					e.type = (arg2 == AKEYCODE_BUTTON_1?
						  Common::EVENT_LBUTTONUP :
						  Common::EVENT_RBUTTONUP);
					break;
				}

				e.mouse = dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->getMousePosition();

				break;

			case AKEYCODE_BUTTON_3:
				e.kbd.keycode = Common::KEYCODE_ESCAPE;
				e.kbd.ascii = Common::ASCII_ESCAPE;
				break;

			case AKEYCODE_BUTTON_4:
				e.type = Common::EVENT_MAINMENU;
				break;

			default:
				LOGW("unmapped gamepad key: %d", arg2);
				return;
			}
		}

		pushEvent(e);

		return;

	case JE_QUIT:
		e.type = Common::EVENT_QUIT;

		pushEvent(e);

		return;

	default:
		LOGE("unknown jevent type: %d", type);

		break;
	}
}

bool OSystem_Android::pollEvent(Common::Event &event) {
	//ENTER();

	if (pthread_self() == _main_thread) {
		if (_screen_changeid != JNI::surface_changeid) {
			_screen_changeid = JNI::surface_changeid;

			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				// surface changed
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->deinitSurface();
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->initSurface();

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			} else {
				// surface lost
				dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->deinitSurface();
			}
		}

		if (JNI::pause) {
			LOGD("main thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("main thread woke up");
		}
	}

	_event_queue_lock->lock();

	if (_queuedEventTime && (getMillis() > _queuedEventTime)) {
		event = _queuedEvent;
		_queuedEventTime = 0;
		// _event_queue_lock->unlock();
		// return true;
	} else if (_event_queue.empty()) {
		_event_queue_lock->unlock();
		return false;
	} else {
		event = _event_queue.pop();
	}

	_event_queue_lock->unlock();

	if (Common::isMouseEvent(event)) {
		if (_graphicsManager)
			return dynamic_cast<AndroidGraphicsManager *>(_graphicsManager)->notifyMousePosition(event.mouse);
	}

	return true;
}

void OSystem_Android::pushEvent(const Common::Event &event) {
	_event_queue_lock->lock();
	_event_queue.push(event);
	_event_queue_lock->unlock();
}

#endif
