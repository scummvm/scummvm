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

// Allow use of stuff in <time.h>
#define FORBIDDEN_SYMBOL_EXCEPTION_time_h

// Disable printf override in common/forbidden.h to avoid
// clashes with log.h from the Android SDK.
// That header file uses
//   __attribute__ ((format(printf, 3, 4)))
// which gets messed up by our override mechanism; this could
// be avoided by either changing the Android SDK to use the equally
// legal and valid
//   __attribute__ ((format(__printf__, 3, 4)))
// or by refining our printf override to use a varadic macro
// (which then wouldn't be portable, though).
// Anyway, for now we just disable the printf override globally
// for the Android port
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include <android/input.h>

#include "backends/platform/android/android.h"
#include "backends/graphics/android/android-graphics.h"
#include "backends/platform/android/jni-android.h"

// floating point. use sparingly
template<class T>
static inline T scalef(T in, float numerator, float denominator) {
	return static_cast<float>(in) * numerator / denominator;
}

// analog joystick axis id (for internal use) - Should match the logic in ScummVMEventsModern.java
enum {
	// auxiliary movement axis bitflags
	JE_JOY_AXIS_X_bf        = 0x01, // (0x01 << 0)
	JE_JOY_AXIS_Y_bf        = 0x02, // (0x01 << 1)
	JE_JOY_AXIS_HAT_X_bf    = 0x04, // (0x01 << 2)
	JE_JOY_AXIS_HAT_Y_bf    = 0x08, // (0x01 << 3)
	JE_JOY_AXIS_Z_bf        = 0x10, // (0x01 << 4)
	JE_JOY_AXIS_RZ_bf       = 0x20, // (0x01 << 5)
	JE_JOY_AXIS_LTRIGGER_bf = 0x40, // (0x01 << 6)
	JE_JOY_AXIS_RTRIGGER_bf = 0x80  // (0x01 << 7)
};

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
	JE_MOUSE_WHEEL_UP = 22,
	JE_MOUSE_WHEEL_DOWN = 23,
	JE_TV_REMOTE = 24,
	JE_QUIT = 0x1000,
	JE_MENU = 0x1001
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
	Common::Event ev0;

	switch (type) {
	case JE_SYS_KEY:
		// fall through
	case JE_KEY:
		switch (arg1) {
		case AKEY_EVENT_ACTION_DOWN:
			ev0.type = Common::EVENT_KEYDOWN;
			break;

		case AKEY_EVENT_ACTION_UP:
			ev0.type = Common::EVENT_KEYUP;
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
				ev0.kbd.keycode = Common::KEYCODE_INVALID;
			}
		} else {
//			LOGD("Received JE_KEY keycode: %d", arg3);
			ev0.kbd.keycode = jkeymap[arg2];
		}

		if (arg5 > 0) {
			ev0.kbdRepeat = true;
		}

//		// HACK: Special case for when the arrow keys on the virtual keyboard are pressed and held down
//		// This emulates what happens when the arrow keys on a physical keyboard are pressed and held down
//		// In the case of the physical keyboard Android keeps sending successive EVENT_KEYDOWN, EVENT_KEYUP events, non-"repeated"
//		if (ev0.kbdRepeat
//		    && ev0.type == Common::EVENT_KEYDOWN
//		    && (ev0.kbd.keycode == Common::KEYCODE_UP
//		        || ev0.kbd.keycode == Common::KEYCODE_DOWN
//		        || ev0.kbd.keycode == Common::KEYCODE_LEFT
//		        || ev0.kbd.keycode == Common::KEYCODE_RIGHT)) {
//			switch (ev0.kbd.keycode) {
//			case Common::KEYCODE_UP:
//				ev0.kbd.ascii = Common::KEYCODE_UP;
//				break;
//
//			case Common::KEYCODE_DOWN:
//				ev0.kbd.ascii = Common::KEYCODE_DOWN;
//				break;
//
//			case Common::KEYCODE_RIGHT:
//				ev0.kbd.ascii = Common::KEYCODE_RIGHT;
//				break;
//
//			case Common::KEYCODE_LEFT:
//				ev0.kbd.ascii = Common::KEYCODE_LEFT;
//				break;
//			}
//			// TODO Maybe only handle 1 every X such events to lower the spam on the queue of these very fast up/down events?
//			ev0.kbdRepeat = false;
//			Common::Event ev1 = ev0;
//			ev1.type = Common::EVENT_KEYUP;
////			LOGD("JE_KEY pushing rep event type: %d kbdcode: %d ascii: %d flags: %d repeats: %d", ev1.type, ev1.kbd.keycode, ev1.kbd.ascii, ev1.kbd.flags, ev1.kbdRepeat? 1: 0);
////			LOGD("JE_KEY pushing rep event type: %d kbdcode: %d ascii: %d flags: %d repeats: %d", ev0.type, ev0.kbd.keycode, ev0.kbd.ascii, ev0.kbd.flags, ev0.kbdRepeat? 1: 0);
//			pushEvent(ev1, ev0);
//			return;
//		}

		// map special keys to 'our' ascii codes
		switch (ev0.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
//			LOGD("received BACKSPACE");
			ev0.kbd.ascii = Common::ASCII_BACKSPACE;
			break;

		case Common::KEYCODE_TAB:
			ev0.kbd.ascii = Common::ASCII_TAB;
			break;

		case Common::KEYCODE_RETURN:
			ev0.kbd.ascii = Common::ASCII_RETURN;
			break;

		case Common::KEYCODE_ESCAPE:
			ev0.kbd.ascii = Common::ASCII_ESCAPE;
			break;

		case Common::KEYCODE_SPACE:
			ev0.kbd.ascii = Common::ASCII_SPACE;
			break;

		case Common::KEYCODE_F1:
			ev0.kbd.ascii = Common::ASCII_F1;
			break;

		case Common::KEYCODE_F2:
			ev0.kbd.ascii = Common::ASCII_F2;
			break;

		case Common::KEYCODE_F3:
			ev0.kbd.ascii = Common::ASCII_F3;
			break;

		case Common::KEYCODE_F4:
			ev0.kbd.ascii = Common::ASCII_F4;
			break;

		case Common::KEYCODE_F5:
			ev0.kbd.ascii = Common::ASCII_F5;
			break;

		case Common::KEYCODE_F6:
			ev0.kbd.ascii = Common::ASCII_F6;
			break;

		case Common::KEYCODE_F7:
			ev0.kbd.ascii = Common::ASCII_F7;
			break;

		case Common::KEYCODE_F8:
			ev0.kbd.ascii = Common::ASCII_F8;
			break;

		case Common::KEYCODE_F9:
			ev0.kbd.ascii = Common::ASCII_F9;
			break;

		case Common::KEYCODE_F10:
			ev0.kbd.ascii = Common::ASCII_F10;
			break;

		case Common::KEYCODE_F11:
			ev0.kbd.ascii = Common::ASCII_F11;
			break;

		case Common::KEYCODE_F12:
			ev0.kbd.ascii = Common::ASCII_F12;
			break;

		case Common::KEYCODE_UP:
			// Set to match what a physical keyboard sends as ascii code for the arrow keys
			ev0.kbd.ascii = Common::KEYCODE_UP;
			break;

		case Common::KEYCODE_DOWN:
			// Set to match what a physical keyboard sends as ascii code for the arrow keys
			ev0.kbd.ascii = Common::KEYCODE_DOWN;
			break;

		case Common::KEYCODE_RIGHT:
			// Set to match what a physical keyboard sends as ascii code for the arrow keys
			ev0.kbd.ascii = Common::KEYCODE_RIGHT;
			break;

		case Common::KEYCODE_LEFT:
			// Set to match what a physical keyboard sends as ascii code for the arrow keys
			ev0.kbd.ascii = Common::KEYCODE_LEFT;
			break;

		default:
			ev0.kbd.ascii = arg3;
			break;
		}

		// arg4 is the metastate of the key press event
		// check for "Shift" key modifier
		if (arg4 & AMETA_SHIFT_MASK) {
			ev0.kbd.flags |= Common::KBD_SHIFT;
		}

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
		//	ev0.kbd.flags |= Common::KBD_ALT;
		// end of old comment --
		// check for "Alt" key modifier
		if (arg4 & (AMETA_ALT_MASK)) {
			ev0.kbd.flags |= Common::KBD_ALT;
		}

		// check for "Ctrl" key modifier (We set Sym key to also behave as Ctrl)
		if (arg4 & (AMETA_SYM_ON | AMETA_CTRL_MASK)) {
			ev0.kbd.flags |= Common::KBD_CTRL;
		}

		// check for "Meta" key modifier
		if (arg4 & (AMETA_META_MASK)) {
			ev0.kbd.flags |= Common::KBD_META;
		}

		//  check for CAPS key modifier
		if (arg4 & (AMETA_CAPS_LOCK_ON)) {
			ev0.kbd.flags |= Common::KBD_CAPS;
		}

		//  check for NUM Lock key modifier
		if (arg4 & (AMETA_NUM_LOCK_ON)) {
			ev0.kbd.flags |= Common::KBD_NUM;
		}

		//  check for Scroll Lock key modifier
		if (arg4 & (AMETA_SCROLL_LOCK_ON)) {
			ev0.kbd.flags |= Common::KBD_SCRL;
		}

//		LOGD("JE_KEY pushing event type: %d kbdcode: %d ascii: %d flags: %d repeats: %d", ev0.type, ev0.kbd.keycode, ev0.kbd.ascii, ev0.kbd.flags, ev0.kbdRepeat? 1: 0);
		pushEvent(ev0);
		break;

	case JE_DPAD:
		// For now, this behavior, emulating mouse movement and left mouse clicking here for DPAD button presses,
		// is no longer used.
		switch (arg2) {
		case AKEYCODE_DPAD_UP:
		// fall through
		case AKEYCODE_DPAD_DOWN:
		// fall through
		case AKEYCODE_DPAD_LEFT:
		// fall through
		case AKEYCODE_DPAD_RIGHT:
			// Treat as mouse movement
			if (arg1 != AKEY_EVENT_ACTION_DOWN)
				break;

			ev0.type = Common::EVENT_MOUSEMOVE;
			ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
			{
				int16 *c;
				int s;

				if (arg2 == AKEYCODE_DPAD_UP || arg2 == AKEYCODE_DPAD_DOWN) {
					c = &ev0.mouse.y;
					s = _eventScaleY;
				} else {
					c = &ev0.mouse.x;
					s = _eventScaleX;
				}

				// the longer the button held, the faster the pointer is
				// TODO put these values in some option dlg?
				int f = CLIP(arg5, 1, 8) * _dpad_scale * 100 / s;

				if (arg2 == AKEYCODE_DPAD_UP || arg2 == AKEYCODE_DPAD_LEFT) {
					*c -= f;
				} else {
					*c += f;
				}
			}
			pushEvent(ev0);
			break;

		case AKEYCODE_DPAD_CENTER:
			// Treat as mouse click (left click)
			switch (arg1) {
			case AKEY_EVENT_ACTION_DOWN:
				ev0.type = Common::EVENT_LBUTTONDOWN;
				break;

			case AKEY_EVENT_ACTION_UP:
				ev0.type = Common::EVENT_LBUTTONUP;
				break;

			default:
				LOGE("unhandled jaction on dpad key: %d", arg1);
				return;
			}

			ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
			pushEvent(ev0);
			break;
		}
		break;

	case JE_TV_REMOTE:
		switch (arg1) {
		case AKEY_EVENT_ACTION_DOWN:
			ev0.type = Common::EVENT_KEYDOWN;
			break;

		case AKEY_EVENT_ACTION_UP:
			ev0.type = Common::EVENT_KEYUP;
			break;

		default:
			LOGE("unhandled jaction on key: %d", arg1);
			return;
		}

		switch (arg2) {
		case AKEYCODE_MEDIA_FAST_FORWARD:
		// fall through
		case AKEYCODE_MEDIA_REWIND:
		// fall through
		case AKEYCODE_MEDIA_PLAY_PAUSE:
			// Treat as keyboard presses, since they have equivalent hardware keyboard keys
			ev0.kbd.keycode = jkeymap[arg2];
			if (arg5 > 0) {
				ev0.kbdRepeat = true;
			}
			break;

		 // Unfortunately CHANNEL_UP or CHANNEL_DOWN do not trigger for the Fire TV Stick remote (3rd gen)
		 // despite the documentation (https://developer.amazon.com/docs/fire-tv/remote-input.html)
		 // so there's no way as of yet to test for them.
		 // TODO Maybe enable them anyway? Should we create hardware input keys for them in the main code?
//		 case AKEYCODE_CHANNEL_UP:
//		 // fall through
//		 case AKEYCODE_CHANNEL_DOWN:
//			break;

		}

		pushEvent(ev0);
		break;

	case JE_DOWN:
//		LOGD("JE_DOWN");
		_touch_pt_down = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		// If the cursor was outside the area (because the screen rotated) clip it
		// to not scroll several times to make the cursor appear in the area
		// Do not clamp the cursor position while rotating the screen because if the user rotated by mistake
		// rotating again will see the cursor position preserved
		if (_touch_pt_down.x > JNI::egl_surface_width) {
			_touch_pt_down.x = JNI::egl_surface_width;
		}
		if (_touch_pt_down.y > JNI::egl_surface_height) {
			_touch_pt_down.y = JNI::egl_surface_height;
		}
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
//		if (_touch_mode != TOUCH_MODE_TOUCHPAD) {
//			ev0.type = Common::EVENT_MOUSEMOVE;
//			ev0.mouse.x = arg1;
//			ev0.mouse.y = arg2;
//			pushEvent(ev0);
//		}
		break;

	case JE_SCROLL:
//		LOGD("JE_SCROLL");
		ev0.type = Common::EVENT_MOUSEMOVE;

		if (_touch_mode == TOUCH_MODE_TOUCHPAD) {
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg3;
				_touch_pt_scroll.y = arg4;
				return;
			}

			ev0.mouse.x = (arg3 - _touch_pt_scroll.x) * 100 / _touchpad_scale;
			ev0.mouse.y = (arg4 - _touch_pt_scroll.y) * 100 / _touchpad_scale;
			ev0.mouse += _touch_pt_down;
		} else {
			ev0.mouse.x = arg3;
			ev0.mouse.y = arg4;
		}
		ev0.relMouse.x = arg5;
		ev0.relMouse.y = arg6;

		pushEvent(ev0);
		break;

	case JE_TAP:
		// arg1 = mouse x
		// arg2 = mouse y
		// arg3 is (int)(e.getEventTime() - e.getDownTime())
//		LOGD("JE_TAP - arg3 %d", arg3);

		ev0.type = Common::EVENT_MOUSEMOVE;

		if (_touch_mode == TOUCH_MODE_TOUCHPAD) {
			ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		} else {
			ev0.mouse.x = arg1;
			ev0.mouse.y = arg2;
		}

		{
			Common::EventType down, up;

			// Based on this check, we check how long the tap finger was held down
			// and distinguish cases for:
			//  > 1.0 seconds: Middle Mouse Button
			//  > 0.5 seconds: Right Mouse Button
			//  < 0.5 seconds: Left Mouse Button
			// Due to how these are detected we cannot have hold and move detection for any of them
			// They only act as clicks
			// Note: for hold one finger and move gesture, this is the "move cursor around" action.
			//       Thus, the simple tap and hold cannot be used for "hold left mouse button and drag" behavior.
			//       This is the reason we use "double tap and move" to emulate that specific behavior.
			// Note: This one-finger gesture cannot currently be used for "hold right mouse and drag" or "hold middle mouse and drag" either.
			//       The gesture for those uses more than one fingers (see JE_MULTI).
			// TODO This might be unwanted "alternate" behavior (better to have it as optional?)
			// TODO put these time (in milliseconds) values in some option dlg?
			if (arg3 > 1500) {
//				LOGD("JE_TAP - arg3 %d --> Middle Mouse Button", arg3);
				down = Common::EVENT_MBUTTONDOWN;
				up = Common::EVENT_MBUTTONUP;
			} else if (arg3 > 500) {
//				LOGD("JE_TAP - arg3 %d --> Right Mouse Button", arg3);
				down = Common::EVENT_RBUTTONDOWN;
				up = Common::EVENT_RBUTTONUP;
			} else {
//				LOGD("JE_TAP - arg3 %d --> Left Mouse Button", arg3);
				down = Common::EVENT_LBUTTONDOWN;
				up = Common::EVENT_LBUTTONUP;
			}

			pushDelayedTouchMouseBtnEvents();
			Common::Event ev1 = ev0;
			ev1.type = down; // mouse down
			_event_queue_lock->lock();
			if (_touch_mode != TOUCH_MODE_TOUCHPAD) {
				// In this case the mouse move is done in "direct mode"
				// ie. the cursor jumps to where the tap occurred
				// so we don't have relMouse coordinates to set for the event
				// However, in Direct Touch mode, some games seem to expect a delay 
				// between the move event and the subsequence mouse down event (eg. Curse of Monkey Island, Tony Tough)
				// Thus, thie Mouse Button Down event is also sent with a delay
				_event_queue.push(ev0);
				_delayedMouseBtnDownEvent.mouse = ev1.mouse;
				_delayedMouseBtnDownEvent.type = down;
				_delayedMouseBtnDownEvent.referTimeMillis = getMillis(true);
				_delayedMouseBtnDownEvent.delayMillis = kQueuedInputEventDelay;
				_delayedMouseBtnDownEvent.connectedType = ev0.type;
				_delayedMouseBtnDownEvent.connectedTypeExecuted = false;
			} else {
				_event_queue.push(ev1);
			}

			_delayedMouseBtnUpEvent.mouse = ev1.mouse;
			_delayedMouseBtnUpEvent.type = up;
			// Mouse-up event is handled with a small delay as some engines require such a delay for mouse up events (Gob, Toonstruck)
			// See bug ticket: https://bugs.scummvm.org/ticket/5942
//			LOGD("JE_TAP - VALID: ev0: mx %d my %d t: %d | ev1: mx %d my %d t: %d | ev2: mx %d my %d t: %d", ev0.mouse.x, ev0.mouse.y, ev0.type, ev1.mouse.x, ev1.mouse.y, ev1.type, ev2.mouse.x, ev2.mouse.y, ev2.type);
			_delayedMouseBtnUpEvent.referTimeMillis = getMillis(true);
			_delayedMouseBtnUpEvent.delayMillis = kQueuedInputEventDelay;
			_delayedMouseBtnUpEvent.connectedType = ev1.type;
			_delayedMouseBtnUpEvent.connectedTypeExecuted = false;
			_event_queue_lock->unlock();
		}
		break;

	case JE_DOUBLE_TAP:
		// arg1 = mouse x
		// arg2 = mouse y
		// arg3 = AMOTION_EVENT_ACTION_DOWN, AMOTION_EVENT_ACTION_UP, AMOTION_EVENT_ACTION_MOVE
		// NOTE: Typically in a double tap event:
		//       Before the ACTION_DOWN event, we also have ALREADY pushed a JE_DOWN event (via ScummVMEventsBase's onDown())
		//       and then a JE_TAP event (action UP) via ScummVMEventsBase's onSingleTapUp().
		//       Before the ACTION_UP event, we also have ALREADY pushed a JE_DOWN event (via ScummVMEventsBase's onDown()).
//		LOGD("JE_DOUBLE_TAP - x: %d y: %d, arg3: %d", arg1, arg2, arg3);

		ev0.type = Common::EVENT_MOUSEMOVE;
		ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();

//		LOGD("JE_DOUBLE_TAP 2 - x: %d y: %d, type: %d, arg3: %d", ev0.mouse.x , ev0.mouse.y, ev0.type, arg3);

		{
			Common::Event ev1 = ev0;
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

			case AMOTION_EVENT_ACTION_MOVE:
				// held and moved
				if (_touch_pt_dt.x == -1 && _touch_pt_dt.y == -1) {
//					LOGD("JE_DOUBLE_TAP MOVE UPDATE dtx = %d, dty = %d and return", arg1, arg2);
					_touch_pt_dt.x = arg1;
					_touch_pt_dt.y = arg2;
					return;
				}

				dptype = Common::EVENT_MOUSEMOVE;

				if (_touch_mode == TOUCH_MODE_TOUCHPAD) {
					ev1.mouse.x = (arg1 - _touch_pt_dt.x) * 100 / _touchpad_scale;
					ev1.mouse.y = (arg2 - _touch_pt_dt.y) * 100 / _touchpad_scale;
					ev1.mouse += _touch_pt_down;
//					LOGD("JE_DOUBLE_TAP MOVE EVENT updating coords and mouse- x: %d y: %d, arg3: %d", ev1.mouse.x , ev1.mouse.y, arg3);
				} else {
					ev1.mouse.x = arg1;
					ev1.mouse.y = arg2;
				}

				break;

			default:
				LOGE("JE_DOUBLE_TAP - unhandled jaction on double tap: %d", arg3);
				return;
			}

			ev1.type = dptype;
			if (ev1.type == Common::EVENT_LBUTTONDOWN) {
				pushDelayedTouchMouseBtnEvents();
			}
//			// Commented out: Don't do a "move mouse" pre-event on a double_tap
//			pushEvent(ev0, ev1);
//			LOGD("JE_DOUBLE_TAP - Pushing DOUBLE TAP event - x: %d y: %d, type: %d, arg3: %d", ev1.mouse.x , ev1.mouse.y, ev1.type, arg3);
			pushEvent(ev1);
		}
		break;

	case JE_MULTI:
		// Documentation: https://developer.android.com/training/gestures/multi
		// TODO also look into: https://developer.android.com/training/gestures/movement
		//      for possible tweaks to gesture detection
		// arg1 = fingers down
		// arg2 = AMOTION_EVENT_ACTION_POINTER_DOWN, AMOTION_EVENT_ACTION_POINTER_UP, CANCEL, OUTSIDE, MOVE
		//        we handle CANCEL and OUTSIDE externally
		//
		// TODO Other related events for Android multitouch gestures events are:
		// - ACTION_DOWN — For the first pointer that touches the screen. This starts the gesture. The pointer data for this pointer is always at index 0 in the MotionEvent.
		// - ACTION_MOVE — A change has happened during a press gesture
		// - ACTION_UP   — Sent when the last pointer leaves the screen.
		LOGD("JE_MULTI - fingersDown=%d arg2=%d", arg1, arg2);

		ev0.type = Common::EVENT_MOUSEMOVE;

		if (_touch_mode == TOUCH_MODE_TOUCHPAD) {
			ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		} else {
			ev0.mouse.x = arg3;
			ev0.mouse.y = arg4;
		}

		{
			Common::Event ev1 = ev0;
			Common::EventType multitype = Common::EVENT_INVALID;

			switch (arg2 & AMOTION_EVENT_ACTION_MASK) {
			case AMOTION_EVENT_ACTION_POINTER_DOWN:
				// (From Android Developers documentation)
				// [This event is fired] For extra pointers that enter the screen beyond the first.
				// The pointer data for this pointer is at the index returned by getActionIndex()
//				LOGD("AMOTION_EVENT_ACTION_POINTER_DOWN fingersDown: %d", arg1);
//				if (arg1 > _fingersDown) {
//					LOGD("AMOTION_EVENT_ACTION_POINTER_DOWN changing  _fingersDown to arg1: %d", arg1);
//					_fingersDown = arg1;
//				}

				_touch_pt_multi.x = -1;
				_touch_pt_multi.y = -1;

				// TODO also handle Cancel event
				// TODO also handle going from 3 to 2 fingers
				// TODO also handle case of receiving a new ACTION_DOWN without first having received an ACTION_UP
				switch (arg1) {
				case 2:
//					LOGD("AMOTION_EVENT_ACTION_POINTER_DOWN arg1: %d --> Right Mouse Button", arg1);
					multitype = Common::EVENT_RBUTTONDOWN;
					break;

				case 3:
//					LOGD("AMOTION_EVENT_ACTION_POINTER_DOWN arg1: %d --> Middle Mouse Button", arg1);
					multitype = Common::EVENT_MBUTTONDOWN;
					break;

				default:
					LOGE("AMOTION_EVENT_ACTION_POINTER_DOWN - unmapped multi tap (arg1): %d", arg1);
					return;
				}
				break;

 			case AMOTION_EVENT_ACTION_CANCEL:
//				LOGD("AMOTION_EVENT_ACTION_CANCEL - (arg1): %d", arg1);
 				return;

  			case AMOTION_EVENT_ACTION_OUTSIDE:
//				LOGD("AMOTION_EVENT_ACTION_OUTSIDE - (arg1): %d", arg1);
 				return;

			case AMOTION_EVENT_ACTION_POINTER_UP:
				// (From Android Developers documentation)
				// Sent when a non-primary pointer goes up.
//				LOGD("AMOTION_EVENT_ACTION_POINTER_UP arg1: %d", arg1);
//				if (arg1 != _fingersDown) {
//					LOGD("returning as AMOTION_EVENT_ACTION_POINTER_UP arg1 != _fingersDown");
//					_fingersDown = 0;
//					return;
//				}

				switch (arg1) {
				case 2:
//					LOGD("AMOTION_EVENT_ACTION_POINTER_UP arg1: %d --> Right Mouse Button", arg1);
					multitype = Common::EVENT_RBUTTONUP;
					break;

				case 3:
//					LOGD("AMOTION_EVENT_ACTION_POINTER_UP arg1: %d --> Middle Mouse Button", arg1);
					multitype = Common::EVENT_MBUTTONUP;
					break;

				default:
					LOGE("AMOTION_EVENT_ACTION_POINTER_UP - unmapped multi tap (arg1): %d", arg1);
					return;
				}
				break;

			case AMOTION_EVENT_ACTION_MOVE:
				// TODO wrong event?
				// https://developer.android.com/training/gestures/movement
				// https://developer.android.com/training/gestures/multi
				//
//				LOGD("AMOTION_EVENT_ACTION_MOVE arg1: %d", arg1);
				// held and moved
				if (_touch_pt_multi.x == -1 && _touch_pt_multi.y == -1) {
					_touch_pt_multi.x = arg3;
					_touch_pt_multi.y = arg4;
					return;
				}

				multitype = Common::EVENT_MOUSEMOVE;

				if (_touch_mode == TOUCH_MODE_TOUCHPAD) {
					ev1.mouse.x = (arg3 - _touch_pt_multi.x) * 100 / _touchpad_scale;
					ev1.mouse.y = (arg4 - _touch_pt_multi.y) * 100 / _touchpad_scale;
					ev1.mouse += _touch_pt_down; // TODO maybe we need another reference point???
				} else {
					ev1.mouse.x = arg3;
					ev1.mouse.y = arg4;
				}
				break;

			default:
				LOGE("JE_MULTI - unhandled jaction on multi tap: %d", arg2);
				return;
			}

			ev1.type = multitype;
			if (ev1.type == Common::EVENT_RBUTTONDOWN || ev1.type == Common::EVENT_MBUTTONDOWN) {
				pushDelayedTouchMouseBtnEvents();

				if (_touch_mode != TOUCH_MODE_TOUCHPAD) {
					// Only send an early move event if:
					// - in direct touch mode
					// - and the multi touch event is a mouse button down (right or middle)
					_event_queue_lock->lock();
					_event_queue.push(ev0);
					_delayedMouseBtnDownEvent.mouse = ev1.mouse;
					_delayedMouseBtnDownEvent.type = ev1.type;
					_delayedMouseBtnDownEvent.referTimeMillis = getMillis(true);
					_delayedMouseBtnDownEvent.delayMillis = kQueuedInputEventDelay;
					_delayedMouseBtnDownEvent.connectedType = ev0.type;
					_delayedMouseBtnDownEvent.connectedTypeExecuted = false;
					_event_queue_lock->unlock();
				} else {
					pushEvent(ev1);
				}
			} else if (ev1.type != Common::EVENT_INVALID) {
				pushEvent(ev1);
			}
		}
		break;

	case JE_BALL:
		ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();

		switch (arg1) {
		case AMOTION_EVENT_ACTION_DOWN:
			ev0.type = Common::EVENT_LBUTTONDOWN;
			break;

		case AMOTION_EVENT_ACTION_UP:
			ev0.type = Common::EVENT_LBUTTONUP;
			break;

		case AMOTION_EVENT_ACTION_MOVE:
			ev0.type = Common::EVENT_MOUSEMOVE;

			// already multiplied by 100
			ev0.mouse.x += arg2 * _trackball_scale / _eventScaleX;
			ev0.mouse.y += arg3 * _trackball_scale / _eventScaleY;

			break;

		default:
			LOGE("unhandled JE_BALL jaction on system key: %d", arg1);
			return;
		}
		pushEvent(ev0);
		break;

	case JE_MOUSE_MOVE:
		ev0.type = Common::EVENT_MOUSEMOVE;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_LMB_DOWN:
		ev0.type = Common::EVENT_LBUTTONDOWN;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_LMB_UP:
		ev0.type = Common::EVENT_LBUTTONUP;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_RMB_DOWN:
		ev0.type = Common::EVENT_RBUTTONDOWN;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_RMB_UP:
		ev0.type = Common::EVENT_RBUTTONUP;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_MMB_DOWN:
		ev0.type = Common::EVENT_MBUTTONDOWN;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_MMB_UP:
		ev0.type = Common::EVENT_MBUTTONUP;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_BMB_DOWN:
		ev0.type = Common::EVENT_X1BUTTONDOWN;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_BMB_UP:
		ev0.type = Common::EVENT_X1BUTTONUP;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_FMB_DOWN:
		ev0.type = Common::EVENT_X2BUTTONDOWN;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_FMB_UP:
		ev0.type = Common::EVENT_X2BUTTONUP;
		ev0.mouse.x = arg1;
		ev0.mouse.y = arg2;
		pushEvent(ev0);
		break;

	case JE_MOUSE_WHEEL_UP:
		// Rolling wheel upwards
		ev0.type = Common::EVENT_WHEELUP;
		if (arg3 == 1 && _touch_mode == TOUCH_MODE_TOUCHPAD) {
			// event comes from touch gesture
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg1;
				_touch_pt_scroll.y = arg2;
				return;
			}

			ev0.mouse.x = (arg1 - _touch_pt_scroll.x) * 100 / _touchpad_scale;
			ev0.mouse.y = (arg2 - _touch_pt_scroll.y) * 100 / _touchpad_scale;
			ev0.mouse += _touch_pt_down;
		} else {
			ev0.mouse.x = arg1;
			ev0.mouse.y = arg2;
		}
//		ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		pushEvent(ev0);
		break;

	case JE_MOUSE_WHEEL_DOWN:
		// Rolling wheel downwards
		ev0.type = Common::EVENT_WHEELDOWN;
		if (arg3 == 1 && _touch_mode == TOUCH_MODE_TOUCHPAD) {
			// event comes from touch gesture
			if (_touch_pt_scroll.x == -1 && _touch_pt_scroll.y == -1) {
				_touch_pt_scroll.x = arg1;
				_touch_pt_scroll.y = arg2;
				return;
			}

			ev0.mouse.x = (arg1 - _touch_pt_scroll.x) * 100 / _touchpad_scale;
			ev0.mouse.y = (arg2 - _touch_pt_scroll.y) * 100 / _touchpad_scale;
			ev0.mouse += _touch_pt_down;
		} else {
			ev0.mouse.x = arg1;
			ev0.mouse.y = arg2;
		}
//		ev0.mouse = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		pushEvent(ev0);
		break;

	case JE_GAMEPAD:
		switch (arg1) {
		case AKEY_EVENT_ACTION_DOWN:
			ev0.type = Common::EVENT_JOYBUTTON_DOWN;
			break;
		case AKEY_EVENT_ACTION_UP:
			ev0.type = Common::EVENT_JOYBUTTON_UP;
			break;
		default:
			LOGE("unhandled jaction on gamepad key: %d", arg1);
			return;
		}

		switch (arg2) {
		case AKEYCODE_BUTTON_START:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_START;
			break;

		case AKEYCODE_BUTTON_SELECT:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_BACK;
			break;

		case AKEYCODE_BUTTON_MODE:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_GUIDE;
			break;

		case AKEYCODE_BUTTON_A:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_A;
			break;

		case AKEYCODE_BUTTON_B:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_B;
			break;

		case AKEYCODE_BUTTON_X:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_X;
			break;

		case AKEYCODE_BUTTON_Y:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_Y;
			break;

		case AKEYCODE_BUTTON_L1:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_LEFT_SHOULDER;
			break;

		case AKEYCODE_BUTTON_R1:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_RIGHT_SHOULDER;
			break;
//		// NOTE As of yet JOYSTICK_BUTTON_LEFT_TRIGGER, JOYSTICK_BUTTON_RIGHT_TRIGGER are missing as "buttons" from the hardware-input source code
//		// There are controllers like PS5's DualSense that trigger these buttons presses, albeit for wrong buttons (Create and Menu gamepad buttons)
//		// which could be due to Android OS not fully supporting them.
//		// PS3's DS3 also triggers these button presses but also generates a movement event so perhaps we can capture them that way
//		// (as generic joystick movement, "JOYSTICK_AXIS_LEFT_TRIGGER", "JOYSTICK_AXIS_RIGHT_TRIGGER" hardware-input).
//		case AKEYCODE_BUTTON_L2:
//			ev0.joystick.button = Common::JOYSTICK_BUTTON_LEFT_TRIGGER;
//			break;
//
//		case AKEYCODE_BUTTON_R2:
//			ev0.joystick.button = Common::JOYSTICK_BUTTON_RIGHT_TRIGGER;
//			break;
//
		case AKEYCODE_BUTTON_THUMBL:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_LEFT_STICK;
			break;

		case AKEYCODE_BUTTON_THUMBR:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_RIGHT_STICK;
			break;

		case AKEYCODE_DPAD_UP:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_DPAD_UP;
			break;

		case AKEYCODE_DPAD_DOWN:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_DPAD_DOWN;
			break;

		case AKEYCODE_DPAD_LEFT:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_DPAD_LEFT;
			break;

		case AKEYCODE_DPAD_RIGHT:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_DPAD_RIGHT;
			break;

		case AKEYCODE_DPAD_CENTER:
			ev0.joystick.button = Common::JOYSTICK_BUTTON_DPAD_CENTER;
			break;

		default:
			LOGW("unmapped gamepad key: %d", arg2);
			return;
		}
		pushEvent(ev0);
		break;

	case JE_JOYSTICK:
		switch (arg1) {
		// AMOTION_EVENT_ACTION_MOVE is 2 in NDK (https://developer.android.com/ndk/reference/group/input)
		case AMOTION_EVENT_ACTION_MOVE:
			ev0.type = Common::EVENT_JOYAXIS_MOTION;

			switch (arg4) {
			case JE_JOY_AXIS_X_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_LEFT_STICK_X;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_Y_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_LEFT_STICK_Y;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_HAT_X_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_HAT_X;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_HAT_Y_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_HAT_Y;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_Z_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_RIGHT_STICK_X;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_RZ_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_RIGHT_STICK_Y;
				ev0.joystick.position = CLIP<int32>(arg2, Common::JOYAXIS_MIN, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_LTRIGGER_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_LEFT_TRIGGER;
				ev0.joystick.position = CLIP<int32>(arg2, 0, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			case JE_JOY_AXIS_RTRIGGER_bf:
				ev0.joystick.axis = Common::JOYSTICK_AXIS_RIGHT_TRIGGER;
				ev0.joystick.position = CLIP<int32>(arg2, 0, Common::JOYAXIS_MAX);
				pushEvent(ev0);
				break;

			default:
				// unsupported axis case
				break;
			}
			break;

		case AKEY_EVENT_ACTION_DOWN:
			ev0.type = Common::EVENT_JOYBUTTON_DOWN;
			break;

		case AKEY_EVENT_ACTION_UP:
			ev0.type = Common::EVENT_JOYBUTTON_UP;
			break;

		default:
			LOGE("unhandled jaction on joystick: %d", arg1);
			return;
		}

		if (arg1 != AMOTION_EVENT_ACTION_MOVE) {
			switch (arg2) {
			case AKEYCODE_BUTTON_1:
				ev0.joystick.button = Common::JOYSTICK_BUTTON_A;
				break;

			case AKEYCODE_BUTTON_2:
				ev0.joystick.button = Common::JOYSTICK_BUTTON_B;
				break;

			case AKEYCODE_BUTTON_3:
				ev0.joystick.button = Common::JOYSTICK_BUTTON_X;
				break;

			case AKEYCODE_BUTTON_4:
				ev0.joystick.button = Common::JOYSTICK_BUTTON_Y;
				break;

			default:
				LOGW("unmapped gamepad key: %d", arg2);
				return;
			}
			pushEvent(ev0);
		}
		break;

	case JE_QUIT:
		ev0.type = Common::EVENT_QUIT;
		pushEvent(ev0);
		break;

	case JE_MENU:
		ev0.type = Common::EVENT_MAINMENU;
		pushEvent(ev0);
		break;

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
			// If we lose the surface, don't deinit as we lose the EGL context and this may lead to crashes
			// Keep a dangling surface until we get a resize
			if (JNI::egl_surface_width > 0 && JNI::egl_surface_height > 0) {
				// surface changed
				dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->resizeSurface();

				event.type = Common::EVENT_SCREEN_CHANGED;

				return true;
			}
		}

		if (_virtkeybd_on != JNI::virt_keyboard_state) {
			_virtkeybd_on = JNI::virt_keyboard_state;
			dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->syncVirtkeyboardState(_virtkeybd_on);
		}

		if (JNI::pause) {
			LOGD("main thread going to sleep");
			sem_wait(&JNI::pause_sem);
			LOGD("main thread woke up");
		}
	}

	_event_queue_lock->lock();

	// We currently allow only one delayed event at any time, and it's always a mouse up event that comes from a touch event/gesture
	// Handling multiple delayed events gets complicated and is practically unnecessary too
	if (_delayedMouseBtnDownEvent.delayMillis > 0
	    && _delayedMouseBtnDownEvent.connectedTypeExecuted
	    && (getMillis(true) - _delayedMouseBtnDownEvent.referTimeMillis > _delayedMouseBtnDownEvent.delayMillis)) {
		Common::Event evHP = _delayedMouseBtnDownEvent;
		event = evHP;
		if ((_delayedMouseBtnUpEvent.delayMillis > 0)
		    && (event.type == _delayedMouseBtnUpEvent.connectedType)) {
			_delayedMouseBtnUpEvent.connectedTypeExecuted = true;
			_delayedMouseBtnUpEvent.referTimeMillis = getMillis(true);
		}
		_delayedMouseBtnDownEvent.reset();
	} else if (_delayedMouseBtnUpEvent.delayMillis > 0
	    && _delayedMouseBtnUpEvent.connectedTypeExecuted
	    && (getMillis(true) - _delayedMouseBtnUpEvent.referTimeMillis > _delayedMouseBtnUpEvent.delayMillis)) {
		Common::Event evHP = _delayedMouseBtnUpEvent;
		event = evHP;
		_delayedMouseBtnUpEvent.reset();
	} else if (_event_queue.empty()) {
		_event_queue_lock->unlock();
		return false;

	} else {
		event = _event_queue.pop();
		if ((_delayedMouseBtnDownEvent.delayMillis > 0)
		    && (event.type == _delayedMouseBtnDownEvent.connectedType)) {
			_delayedMouseBtnDownEvent.connectedTypeExecuted = true;
			_delayedMouseBtnDownEvent.referTimeMillis = getMillis(true);
		} else if ((_delayedMouseBtnUpEvent.delayMillis > 0)
		    && (event.type == _delayedMouseBtnUpEvent.connectedType)) {
			_delayedMouseBtnUpEvent.connectedTypeExecuted = true;
			_delayedMouseBtnUpEvent.referTimeMillis = getMillis(true);
		}
	}
	_event_queue_lock->unlock();

	if (Common::isMouseEvent(event)) {
		if (_graphicsManager)
			return dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->notifyMousePosition(event.mouse);
	}

	return true;
}

void OSystem_Android::pushEvent(const Common::Event &event) {
	_event_queue_lock->lock();
	_event_queue.push(event);
	_event_queue_lock->unlock();
}

void OSystem_Android::pushEvent(const Common::Event &event1, const Common::Event &event2) {
	_event_queue_lock->lock();
	_event_queue.push(event1);
	_event_queue.push(event2);
	_event_queue_lock->unlock();
}

void OSystem_Android::pushDelayedTouchMouseBtnEvents() {
	_event_queue_lock->lock();
	Common::Event evHP;
	if (_delayedMouseBtnDownEvent.delayMillis > 0) {
		evHP = _delayedMouseBtnDownEvent;
		_event_queue.push(_delayedMouseBtnDownEvent);
		_delayedMouseBtnDownEvent.reset();
	}
	if (_delayedMouseBtnUpEvent.delayMillis > 0) {
		evHP = _delayedMouseBtnUpEvent;
		_event_queue.push(_delayedMouseBtnUpEvent);
		_delayedMouseBtnUpEvent.reset();
	}
	_event_queue_lock->unlock();
}

void OSystem_Android::setupTouchMode(int oldValue, int newValue) {
	_touch_mode = newValue;

	if (newValue == TOUCH_MODE_TOUCHPAD) {
		// Make sure we have a proper touch point if we switch to touchpad mode with finger down
		if (_graphicsManager) {
			_touch_pt_down = dynamic_cast<AndroidCommonGraphics *>(_graphicsManager)->getMousePosition();
		} else {
			_touch_pt_down.x = 0;
			_touch_pt_down.y = 0;
		}
		_touch_pt_scroll.x = -1;
		_touch_pt_scroll.y = -1;
	}
}
