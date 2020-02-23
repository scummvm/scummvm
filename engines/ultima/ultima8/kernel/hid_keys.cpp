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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/kernel/hid_keys.h"

namespace Ultima {
namespace Ultima8 {

struct KeyName {
	HID_Key key;
	const char *name;
};

static const KeyName keyNames[] = {
	{HID_BACKSPACE, "Backspace"},
	{HID_TAB, "Tab"},

	{HID_ENTER, "Enter"},
	{HID_ENTER, "Return"},

	{HID_ESCAPE, "Escape"},
	{HID_SPACE, "Space"},

	{HID_EXCLAIM, "Exclaim"},
	{HID_SPACE, ""},

	{HID_DOUBLEQUOTE, "Double-Quote"},
	{HID_DOUBLEQUOTE, "\""},

	{HID_HASH, "Hash"},
	{HID_HASH, "#"},

	{HID_DOLLAR, "Dollar"},
	{HID_DOLLAR, "$"},

	{HID_PERCENT, "Percent"},
	{HID_PERCENT, "%"},

	{HID_AMPERSAND, "Ampersand"},
	{HID_AMPERSAND, "&"},

	{HID_QUOTE, "Quote"},
	{HID_QUOTE, "\'"},

	{HID_LEFTPAREN, "Left-Paren"},
	{HID_LEFTPAREN, "("},
	{HID_RIGHTPAREN, "Right-Paren"},
	{HID_RIGHTPAREN, ")"},

	{HID_ASTERISK, "Asterisk"},
	{HID_ASTERISK, "Star"},
	{HID_ASTERISK, "*"},

	{HID_PLUS, "Plus"},
	{HID_PLUS, "+"},

	{HID_COMMA, "Comma"},
	{HID_COMMA, ","},

	{HID_MINUS, "Minus"},
	{HID_MINUS, "-"},

	{HID_PERIOD, "Period"},
	{HID_PERIOD, "."},

	{HID_SLASH, "Slash"},
	{HID_SLASH, "/"},

	{HID_0, "0"},
	{HID_1, "1"},
	{HID_2, "2"},
	{HID_3, "3"},
	{HID_4, "4"},
	{HID_5, "5"},
	{HID_6, "6"},
	{HID_7, "7"},
	{HID_8, "8"},
	{HID_9, "9"},

	{HID_COLON, "Colon"},
	{HID_COLON, ":"},

	{HID_SEMICOLON, "Semicolon"},
	{HID_SEMICOLON, ";"},

	{HID_LEFTANGLE, "Left-Angle"},
	{HID_LEFTANGLE, "Less"},
	{HID_LEFTANGLE, "<"},

	{HID_EQUALS, "Equals"},
	{HID_EQUALS, "="},

	{HID_RIGHTANGLE, "Right-Angle"},
	{HID_RIGHTANGLE, "Greater"},
	{HID_RIGHTANGLE, ">"},

	{HID_QUESTION, "Question"},
	{HID_QUESTION, "?"},

	{HID_AT, "At"},
	{HID_AT, "@"},

	{HID_A, "A"},
	{HID_B, "B"},
	{HID_C, "C"},
	{HID_D, "D"},
	{HID_E, "E"},
	{HID_F, "F"},
	{HID_G, "G"},
	{HID_H, "H"},
	{HID_I, "I"},
	{HID_J, "J"},
	{HID_K, "K"},
	{HID_L, "L"},
	{HID_M, "M"},
	{HID_N, "N"},
	{HID_O, "O"},
	{HID_P, "P"},
	{HID_Q, "Q"},
	{HID_R, "R"},
	{HID_S, "S"},
	{HID_T, "T"},
	{HID_U, "U"},
	{HID_V, "V"},
	{HID_W, "W"},
	{HID_X, "X"},
	{HID_Y, "Y"},
	{HID_Z, "Z"},

	{HID_LEFTBRACKET, "Left-Bracket"},
	{HID_LEFTBRACKET, "["},

	{HID_BACKSLASH, "Backslash"},
	{HID_BACKSLASH, "\\"},

	{HID_RIGHTBRACKET, "Right-Bracket"},
	{HID_RIGHTBRACKET, "]"},

	{HID_CARET, "Caret"},
	{HID_CARET, "Hat"},
	{HID_CARET, "^"},

	{HID_UNDERSCORE, "Underscore"},
	{HID_UNDERSCORE, "_"},

	{HID_BACKQUOTE, "Backquote"},
	{HID_BACKQUOTE, "Grave"},
	{HID_BACKQUOTE, "`"},

	{HID_LEFTBRACE, "Left-Brace"},
	{HID_LEFTBRACE, "{"},

	{HID_VERTICALBAR, "Vertical-Bar"},
	{HID_VERTICALBAR, "Pipe"},
	{HID_VERTICALBAR, "Bar"},
	{HID_VERTICALBAR, "|"},

	{HID_RIGHTBRACE, "Right-Brace"},
	{HID_RIGHTBRACE, "}"},

	{HID_TILDE, "Tilde"},
	{HID_TILDE, "~"},

	{HID_KP0, "Keypad0"},
	{HID_KP1, "Keypad1"},
	{HID_KP2, "Keypad2"},
	{HID_KP3, "Keypad3"},
	{HID_KP4, "Keypad4"},
	{HID_KP5, "Keypad5"},
	{HID_KP6, "Keypad6"},
	{HID_KP7, "Keypad7"},
	{HID_KP8, "Keypad8"},
	{HID_KP9, "Keypad9"},

	{HID_KP_PERIOD, "Keypad-Period"},
	{HID_KP_SLASH, "Keypad-Slash"},

	{HID_KP_ASTERISK, "Keypad-Asterisk"},
	{HID_KP_ASTERISK, "Keypad-Star"},

	{HID_KP_MINUS, "Keypad-Minus"},
	{HID_KP_PLUS, "Keypad-Plus"},
	{HID_KP_ENTER, "Keypad-Enter"},

	{HID_UP, "Up"},
	{HID_DOWN, "Down"},
	{HID_LEFT, "Left"},
	{HID_RIGHT, "Right"},
	{HID_INSERT, "Insert"},
	{HID_DELETE, "Delete"},
	{HID_HOME, "Home"},
	{HID_END, "End"},
	{HID_PAGEUP, "Page-Up"},
	{HID_PAGEDOWN, "Page-Down"},

	{HID_F1, "F1"},
	{HID_F2, "F2"},
	{HID_F3, "F3"},
	{HID_F4, "F4"},
	{HID_F5, "F5"},
	{HID_F6, "F6"},
	{HID_F7, "F7"},
	{HID_F8, "F8"},
	{HID_F9, "F9"},
	{HID_F10, "F10"},
	{HID_F11, "F11"},
	{HID_F12, "F12"},
	{HID_F13, "F13"},
	{HID_F14, "F14"},
	{HID_F15, "F15"},

	{HID_PRINT, "Print"},
	{HID_NUM_LOCK, "Num-Lock"},
	{HID_CAPS_LOCK, "Caps-Lock"},
	{HID_SCROLL_LOCK, "Scroll-Lock"},
	{HID_PAUSE, "Pause"},

	{HID_MOUSE1, "Mouse1"},
	{HID_MOUSE2, "Mouse2"},
	{HID_MOUSE3, "Mouse3"},
	{HID_MOUSE4, "Mouse4"},
	{HID_MOUSE5, "Mouse5"},
	{HID_MOUSE6, "Mouse6"},
	{HID_MOUSE7, "Mouse7"},
	{HID_MOUSE8, "Mouse8"},
	{HID_MOUSE9, "Mouse9"},
	{HID_MOUSE10, "Mouse10"},
	{HID_MOUSE11, "Mouse11"},
	{HID_MOUSE12, "Mouse12"},
	{HID_MOUSE13, "Mouse13"},
	{HID_MOUSE14, "Mouse14"},
	{HID_MOUSE15, "Mouse15"},
	{HID_MOUSE16, "Mouse16"},

	{HID_JOY1, "Joy1"},
	{HID_JOY2, "Joy2"},
	{HID_JOY3, "Joy3"},
	{HID_JOY4, "Joy4"},
	{HID_JOY5, "Joy5"},
	{HID_JOY6, "Joy6"},
	{HID_JOY7, "Joy7"},
	{HID_JOY8, "Joy8"},
	{HID_JOY9, "Joy9"},
	{HID_JOY10, "Joy10"},
	{HID_JOY11, "Joy11"},
	{HID_JOY12, "Joy12"},
	{HID_JOY13, "Joy13"},
	{HID_JOY14, "Joy14"},
	{HID_JOY15, "Joy15"},
	{HID_JOY16, "Joy16"},

	{HID_LAST, ""}
};

struct EventName {
	HID_Event event;
	const char *name;
};

static const EventName EVENT_NAMES[] = {
	{ HID_EVENT_DEPRESS, "Depress" },
	{ HID_EVENT_RELEASE, "Release" },
	{ HID_EVENT_DOUBLE, "Double" },
	{ HID_EVENT_CLICK, "Click" },
	{ HID_EVENT_PREEMPT, "Preempt" },
	{ HID_EVENT_LAST, "" },
	{ HID_FLAGS_CTRL, "Ctrl" },
	{ HID_FLAGS_ALT, "Alt" },
	{ HID_FLAGS_SHIFT, "Shift" },
	{ HID_FLAGS_META, "Meta" },
	{ HID_FLAGS_LAST, "" }
};

const char *HID_GetKeyName(HID_Key key) {
	int i;
	for (i = 0; keyNames[i].key != HID_LAST; ++i) {
		if (key == keyNames[i].key)
			return keyNames[i].name;
	}

	return "";
}

HID_Key HID_GetKeyFromName(istring &name) {
	int i;
	for (i = 0; keyNames[i].key != HID_LAST; ++i) {
		if (name == keyNames[i].name)
			return keyNames[i].key;
	}

	return HID_LAST;
}

HID_Key HID_translateKey(Common::KeyCode key) {
	switch (key) {
	case Common::KEYCODE_BACKSPACE:
		return HID_BACKSPACE;
	case Common::KEYCODE_TAB:
		return HID_TAB;
	case Common::KEYCODE_RETURN:
		return HID_ENTER;
	case Common::KEYCODE_SPACE:
		return HID_SPACE;
	case Common::KEYCODE_EXCLAIM:
		return HID_EXCLAIM;
	case Common::KEYCODE_QUOTEDBL:
		return HID_DOUBLEQUOTE;
	case Common::KEYCODE_HASH:
		return HID_HASH;
	case Common::KEYCODE_DOLLAR:
		return HID_DOLLAR;
//		case : return HID_PERCENT;
	case Common::KEYCODE_AMPERSAND:
		return HID_AMPERSAND;
	case Common::KEYCODE_QUOTE:
		return HID_QUOTE;
	case Common::KEYCODE_LEFTPAREN:
		return HID_LEFTPAREN;
	case Common::KEYCODE_RIGHTPAREN:
		return HID_RIGHTPAREN;
	case Common::KEYCODE_ASTERISK:
		return HID_ASTERISK;
	case Common::KEYCODE_PLUS:
		return HID_PLUS;
	case Common::KEYCODE_COMMA:
		return HID_COMMA;
	case Common::KEYCODE_MINUS:
		return HID_MINUS;
	case Common::KEYCODE_PERIOD:
		return HID_PERIOD;
	case Common::KEYCODE_SLASH:
		return HID_SLASH;
	case Common::KEYCODE_0:
		return HID_0;
	case Common::KEYCODE_1:
		return HID_1;
	case Common::KEYCODE_2:
		return HID_2;
	case Common::KEYCODE_3:
		return HID_3;
	case Common::KEYCODE_4:
		return HID_4;
	case Common::KEYCODE_5:
		return HID_5;
	case Common::KEYCODE_6:
		return HID_6;
	case Common::KEYCODE_7:
		return HID_7;
	case Common::KEYCODE_8:
		return HID_8;
	case Common::KEYCODE_9:
		return HID_9;
	case Common::KEYCODE_COLON:
		return HID_COLON;
	case Common::KEYCODE_SEMICOLON:
		return HID_SEMICOLON;
	case Common::KEYCODE_LESS:
		return HID_LEFTANGLE;
	case Common::KEYCODE_EQUALS:
		return HID_EQUALS;
	case Common::KEYCODE_GREATER:
		return HID_RIGHTANGLE;
	case Common::KEYCODE_QUESTION:
		return HID_QUESTION;
	case Common::KEYCODE_AT:
		return HID_AT;
	case Common::KEYCODE_a:
		return HID_A;
	case Common::KEYCODE_b:
		return HID_B;
	case Common::KEYCODE_c:
		return HID_C;
	case Common::KEYCODE_d:
		return HID_D;
	case Common::KEYCODE_e:
		return HID_E;
	case Common::KEYCODE_f:
		return HID_F;
	case Common::KEYCODE_g:
		return HID_G;
	case Common::KEYCODE_h:
		return HID_H;
	case Common::KEYCODE_i:
		return HID_I;
	case Common::KEYCODE_j:
		return HID_J;
	case Common::KEYCODE_k:
		return HID_K;
	case Common::KEYCODE_l:
		return HID_L;
	case Common::KEYCODE_m:
		return HID_M;
	case Common::KEYCODE_n:
		return HID_N;
	case Common::KEYCODE_o:
		return HID_O;
	case Common::KEYCODE_p:
		return HID_P;
	case Common::KEYCODE_q:
		return HID_Q;
	case Common::KEYCODE_r:
		return HID_R;
	case Common::KEYCODE_s:
		return HID_S;
	case Common::KEYCODE_t:
		return HID_T;
	case Common::KEYCODE_u:
		return HID_U;
	case Common::KEYCODE_v:
		return HID_V;
	case Common::KEYCODE_w:
		return HID_W;
	case Common::KEYCODE_x:
		return HID_X;
	case Common::KEYCODE_y:
		return HID_Y;
	case Common::KEYCODE_z:
		return HID_Z;
	case Common::KEYCODE_LEFTBRACKET:
		return HID_LEFTBRACKET;
	case Common::KEYCODE_BACKSLASH:
		return HID_BACKSLASH;
	case Common::KEYCODE_RIGHTBRACKET:
		return HID_RIGHTBRACKET;
	case Common::KEYCODE_CARET:
		return HID_CARET;
	case Common::KEYCODE_UNDERSCORE:
		return HID_UNDERSCORE;
	case Common::KEYCODE_BACKQUOTE:
		return HID_BACKQUOTE;
//		case : return HID_LEFTBRACE;
//		case : return HID_VERTICALBAR;
//		case : return HID_RIGHTBRACE;
//		case : return HID_TILDE;
	case Common::KEYCODE_DELETE:
		return HID_DELETE;
	case Common::KEYCODE_INSERT:
		return HID_INSERT;
	case Common::KEYCODE_END:
		return HID_END;
	case Common::KEYCODE_HOME:
		return HID_HOME;
	case Common::KEYCODE_PAGEDOWN:
		return HID_PAGEDOWN;
	case Common::KEYCODE_PAGEUP:
		return HID_PAGEUP;
	case Common::KEYCODE_UP:
		return HID_UP;
	case Common::KEYCODE_DOWN:
		return HID_DOWN;
	case Common::KEYCODE_LEFT:
		return HID_LEFT;
	case Common::KEYCODE_RIGHT:
		return HID_RIGHT;
	case Common::KEYCODE_KP0:
		return HID_KP0;
	case Common::KEYCODE_KP1:
		return HID_KP1;
	case Common::KEYCODE_KP2:
		return HID_KP2;
	case Common::KEYCODE_KP3:
		return HID_KP3;
	case Common::KEYCODE_KP4:
		return HID_KP4;
	case Common::KEYCODE_KP5:
		return HID_KP5;
	case Common::KEYCODE_KP6:
		return HID_KP6;
	case Common::KEYCODE_KP7:
		return HID_KP7;
	case Common::KEYCODE_KP8:
		return HID_KP8;
	case Common::KEYCODE_KP9:
		return HID_KP9;
	case Common::KEYCODE_KP_PERIOD:
		return HID_KP_PERIOD;
	case Common::KEYCODE_KP_DIVIDE:
		return HID_KP_SLASH;
	case Common::KEYCODE_KP_MULTIPLY:
		return HID_KP_ASTERISK;
	case Common::KEYCODE_KP_MINUS:
		return HID_KP_MINUS;
	case Common::KEYCODE_KP_PLUS:
		return HID_KP_PLUS;
	case Common::KEYCODE_KP_ENTER:
		return HID_KP_ENTER;
	case Common::KEYCODE_KP_EQUALS:
		return HID_KP_EQUALS;
	case Common::KEYCODE_F1:
		return HID_F1;
	case Common::KEYCODE_F2:
		return HID_F2;
	case Common::KEYCODE_F3:
		return HID_F3;
	case Common::KEYCODE_F4:
		return HID_F4;
	case Common::KEYCODE_F5:
		return HID_F5;
	case Common::KEYCODE_F6:
		return HID_F6;
	case Common::KEYCODE_F7:
		return HID_F7;
	case Common::KEYCODE_F8:
		return HID_F8;
	case Common::KEYCODE_F9:
		return HID_F9;
	case Common::KEYCODE_F10:
		return HID_F10;
	case Common::KEYCODE_F11:
		return HID_F11;
	case Common::KEYCODE_F12:
		return HID_F12;
	case Common::KEYCODE_F13:
		return HID_F13;
	case Common::KEYCODE_F14:
		return HID_F14;
	case Common::KEYCODE_F15:
		return HID_F15;
	case Common::KEYCODE_PRINT:
		return HID_PRINT;
	case Common::KEYCODE_NUMLOCK:
		return HID_NUM_LOCK;
	case Common::KEYCODE_CAPSLOCK:
		return HID_CAPS_LOCK;
	case Common::KEYCODE_SCROLLOCK:
		return HID_SCROLL_LOCK;
	case Common::KEYCODE_PAUSE:
		return HID_PAUSE;
	case Common::KEYCODE_ESCAPE:
		return HID_ESCAPE;
	default:
		break;
	}
	return HID_LAST;
}

HID_Events HID_translateKeyFlags(byte flags) {
	HID_Events result = 0;

	if (flags & Common::KBD_CTRL)
		result |= HID_FLAGS_CTRL;
	if (flags & Common::KBD_ALT)
		result |= HID_FLAGS_ALT;
	if (flags & Common::KBD_SHIFT)
		result |= HID_FLAGS_SHIFT;
	if (flags & Common::KBD_META)
		result |= HID_FLAGS_META;

	return result;
}

HID_Key HID_translateMouseButton(uint8 button) {
	switch (button) {
	case 1:
		return HID_MOUSE1;
	case 2:
		return HID_MOUSE2;
	case 3:
		return HID_MOUSE3;
	case 4:
		return HID_MOUSE4;
	case 5:
		return HID_MOUSE5;
	case 6:
		return HID_MOUSE6;
	case 7:
		return HID_MOUSE7;
	case 8:
		return HID_MOUSE8;
	case 9:
		return HID_MOUSE9;
	case 10:
		return HID_MOUSE10;
	case 11:
		return HID_MOUSE11;
	case 12:
		return HID_MOUSE12;
	case 13:
		return HID_MOUSE13;
	case 14:
		return HID_MOUSE14;
	case 15:
		return HID_MOUSE15;
	case 16:
		return HID_MOUSE16;
	default:
		break;
	}
	return HID_LAST;
}

HID_Key HID_translateJoystickButton(uint8 button) {
	switch (button) {
	case 1:
		return HID_JOY1;
	case 2:
		return HID_JOY2;
	case 3:
		return HID_JOY3;
	case 4:
		return HID_JOY4;
	case 5:
		return HID_JOY5;
	case 6:
		return HID_JOY6;
	case 7:
		return HID_JOY7;
	case 8:
		return HID_JOY8;
	case 9:
		return HID_JOY9;
	case 10:
		return HID_JOY10;
	case 11:
		return HID_JOY11;
	case 12:
		return HID_JOY12;
	case 13:
		return HID_JOY13;
	case 14:
		return HID_JOY14;
	case 15:
		return HID_JOY15;
	case 16:
		return HID_JOY16;
	default:
		break;
	}
	return HID_LAST;
}

const char *HID_GetEventsName(HID_Events events) {
	static char buffer[32];
	strcpy(buffer, "<");

	int i;
	for (i = 0; EVENT_NAMES[i].event != HID_FLAGS_LAST; ++i) {
		if ((EVENT_NAMES[i].event <= HID_EVENT_LAST) ? (events & 0xff) == EVENT_NAMES[i].event :
				(events & EVENT_NAMES[i].event) != 0) {
			if (EVENT_NAMES[i].event == HID_EVENT_DEPRESS && events != HID_EVENT_DEPRESS)
				continue;
			
			strcat(buffer, EVENT_NAMES[i].name);
			strcat(buffer, ",");
		}
	}

	buffer[strlen(buffer) - 1] = '>';
	return buffer;
}

HID_Events HID_GetEventFromName(const istring &name) {
	// Split up the name(s)
	istring str = name;
	if (name.hasPrefix("<") && name.hasSuffix(">")) {
		str.deleteChar(0);
		str.deleteLastChar();
	}

	Common::Array<istring> events;
	str.split(events);

	HID_Events result = 0;

	for (uint termIdx = 0; termIdx < events.size(); ++termIdx) {
		// Scan for matching event/flag name
		for (int i = 0; EVENT_NAMES[i].event != HID_FLAGS_LAST; ++i) {
			if (EVENT_NAMES[i].event == HID_EVENT_LAST)
				continue;

			if (events[termIdx] == EVENT_NAMES[i].name) {
				if (EVENT_NAMES[i].event < HID_EVENT_LAST)
					result = (result & ~0xff) | EVENT_NAMES[i].event;
				else
					result |= EVENT_NAMES[i].event;
			}
		}
	}

	return result;
}

} // End of namespace Ultima8
} // End of namespace Ultima
