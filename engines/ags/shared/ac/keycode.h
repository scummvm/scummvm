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

#ifndef AGS_SHARED_AC_KEYCODE_H
#define AGS_SHARED_AC_KEYCODE_H

#include "ags/shared/core/platform.h"
#include "ags/shared/core/types.h"

namespace AGS3 {

#define EXTENDED_KEY_CODE ('\0')
#define EXTENDED_KEY_CODE_MACOS ('?')

// Constant used to define Alt+Key codes
#define AGS_EXT_KEY_SHIFT  300
#define AGS_EXT_KEY_ALPHA(key)  (AGS_EXT_KEY_SHIFT + (key - eAGSKeyCodeCtrlA) + 1)

// These are based on eKeyCode values in AGS Script.
// The actual values are based on scan codes of the old backend (allegro 3 and/or 4),
// which in turn mostly match ASCII values (at least for ones below 128), including
// Ctrl + letter combination codes.
// More codes are added at much higher ranges, for example Alt + letter combo codes
// are defined as 300 + letter's order.
// It should be specifically noted that eAGSKeyCode is directly conversible to ASCII
// at the range of 1 - 128, and AGS script makes use of this.
// Another important thing to note is that letter codes are always sent into script
// callbacks (like "on_key_pressed") in capitalized form, and that's how they are
// declared in script API (that's why in these callbacks user would have to check
// the Shift key state if they want to know if it's A or Shift + A).
enum eAGSKeyCode {
	eAGSKeyCodeNone = 0,

	eAGSKeyCodeCtrlA = 1,
	eAGSKeyCodeCtrlB = 2,
	eAGSKeyCodeCtrlC = 3,
	eAGSKeyCodeCtrlD = 4,
	eAGSKeyCodeCtrlE = 5,
	eAGSKeyCodeCtrlF = 6,
	eAGSKeyCodeCtrlG = 7,
	eAGSKeyCodeCtrlH = 8,
	eAGSKeyCodeCtrlI = 9,
	eAGSKeyCodeCtrlJ = 10,
	eAGSKeyCodeCtrlK = 11,
	eAGSKeyCodeCtrlL = 12,
	eAGSKeyCodeCtrlM = 13,
	eAGSKeyCodeCtrlN = 14,
	eAGSKeyCodeCtrlO = 15,
	eAGSKeyCodeCtrlP = 16,
	eAGSKeyCodeCtrlQ = 17,
	eAGSKeyCodeCtrlR = 18,
	eAGSKeyCodeCtrlS = 19,
	eAGSKeyCodeCtrlT = 20,
	eAGSKeyCodeCtrlU = 21,
	eAGSKeyCodeCtrlV = 22,
	eAGSKeyCodeCtrlW = 23,
	eAGSKeyCodeCtrlX = 24,
	eAGSKeyCodeCtrlY = 25,
	eAGSKeyCodeCtrlZ = 26,

	eAGSKeyCodeBackspace = 8, // matches Ctrl + H
	eAGSKeyCodeTab = 9,       // matches Ctrl + I
	eAGSKeyCodeReturn = 13,   // matches Ctrl + M
	eAGSKeyCodeEscape = 27,

	/* printable chars - from eAGSKeyCodeSpace to eAGSKeyCode_z */
	eAGSKeyCodeSpace = 32,
	eAGSKeyCodeExclamationMark = 33,
	eAGSKeyCodeDoubleQuote = 34,
	eAGSKeyCodeHash = 35,
	eAGSKeyCodeDollar = 36,
	eAGSKeyCodePercent = 37,
	eAGSKeyCodeAmpersand = 38,
	eAGSKeyCodeSingleQuote = 39,
	eAGSKeyCodeOpenParenthesis = 40,
	eAGSKeyCodeCloseParenthesis = 41,
	eAGSKeyCodeAsterisk = 42,
	eAGSKeyCodePlus = 43,
	eAGSKeyCodeComma = 44,
	eAGSKeyCodeHyphen = 45,
	eAGSKeyCodePeriod = 46,
	eAGSKeyCodeForwardSlash = 47,

	eAGSKeyCode0 = 48,
	eAGSKeyCode1 = 49,
	eAGSKeyCode2 = 50,
	eAGSKeyCode3 = 51,
	eAGSKeyCode4 = 52,
	eAGSKeyCode5 = 53,
	eAGSKeyCode6 = 54,
	eAGSKeyCode7 = 55,
	eAGSKeyCode8 = 56,
	eAGSKeyCode9 = 57,

	eAGSKeyCodeColon = 58,
	eAGSKeyCodeSemiColon = 59,
	eAGSKeyCodeLessThan = 60,
	eAGSKeyCodeEquals = 61,
	eAGSKeyCodeGreaterThan = 62,
	eAGSKeyCodeQuestionMark = 63,
	eAGSKeyCodeAt = 64, // '@'

	/* Notice that default letter codes match capital ASCII letters */
	eAGSKeyCodeA = 65, // 'A'
	eAGSKeyCodeB = 66, // 'B', etc
	eAGSKeyCodeC = 67,
	eAGSKeyCodeD = 68,
	eAGSKeyCodeE = 69,
	eAGSKeyCodeF = 70,
	eAGSKeyCodeG = 71,
	eAGSKeyCodeH = 72,
	eAGSKeyCodeI = 73,
	eAGSKeyCodeJ = 74,
	eAGSKeyCodeK = 75,
	eAGSKeyCodeL = 76,
	eAGSKeyCodeM = 77,
	eAGSKeyCodeN = 78,
	eAGSKeyCodeO = 79,
	eAGSKeyCodeP = 80,
	eAGSKeyCodeQ = 81,
	eAGSKeyCodeR = 82,
	eAGSKeyCodeS = 83,
	eAGSKeyCodeT = 84,
	eAGSKeyCodeU = 85,
	eAGSKeyCodeV = 86,
	eAGSKeyCodeW = 87,
	eAGSKeyCodeX = 88,
	eAGSKeyCodeY = 89,
	eAGSKeyCodeZ = 90, // 'Z'

	eAGSKeyCodeOpenBracket = 91,
	eAGSKeyCodeBackSlash = 92,
	eAGSKeyCodeCloseBracket = 93,
	eAGSKeyCodeCaret = 94, // '^'
	eAGSKeyCodeUnderscore = 95,
	eAGSKeyCodeBackquote = 96, // '`'

	/* Small ASCII letter codes are declared here for consistency, but unused in script callbacks */
	eAGSKeyCode_a = 97, // 'a'
	eAGSKeyCode_b = 98, // 'b', etc
	eAGSKeyCode_c = 99,
	eAGSKeyCode_d = 100,
	eAGSKeyCode_e = 101,
	eAGSKeyCode_f = 102,
	eAGSKeyCode_g = 103,
	eAGSKeyCode_h = 104,
	eAGSKeyCode_i = 105,
	eAGSKeyCode_j = 106,
	eAGSKeyCode_k = 107,
	eAGSKeyCode_l = 108,
	eAGSKeyCode_m = 109,
	eAGSKeyCode_n = 110,
	eAGSKeyCode_o = 111,
	eAGSKeyCode_p = 112,
	eAGSKeyCode_q = 113,
	eAGSKeyCode_r = 114,
	eAGSKeyCode_s = 115,
	eAGSKeyCode_t = 116,
	eAGSKeyCode_u = 117,
	eAGSKeyCode_v = 118,
	eAGSKeyCode_w = 119,
	eAGSKeyCode_x = 120,
	eAGSKeyCode_y = 121,
	eAGSKeyCode_z = 122, // 'z'

	/* extended symbol codes */
	eAGSKeyCodeF1 = AGS_EXT_KEY_SHIFT + 59,
	eAGSKeyCodeF2 = AGS_EXT_KEY_SHIFT + 60,
	eAGSKeyCodeF3 = AGS_EXT_KEY_SHIFT + 61,
	eAGSKeyCodeF4 = AGS_EXT_KEY_SHIFT + 62,
	eAGSKeyCodeF5 = AGS_EXT_KEY_SHIFT + 63,
	eAGSKeyCodeF6 = AGS_EXT_KEY_SHIFT + 64,
	eAGSKeyCodeF7 = AGS_EXT_KEY_SHIFT + 65,
	eAGSKeyCodeF8 = AGS_EXT_KEY_SHIFT + 66,
	eAGSKeyCodeF9 = AGS_EXT_KEY_SHIFT + 67,
	eAGSKeyCodeF10 = AGS_EXT_KEY_SHIFT + 68,
	eAGSKeyCodeF11 = AGS_EXT_KEY_SHIFT + 133,
	eAGSKeyCodeF12 = AGS_EXT_KEY_SHIFT + 134,

	eAGSKeyCodeHome = AGS_EXT_KEY_SHIFT + 71,
	eAGSKeyCodeUpArrow = AGS_EXT_KEY_SHIFT + 72,
	eAGSKeyCodePageUp = AGS_EXT_KEY_SHIFT + 73,
	eAGSKeyCodeLeftArrow = AGS_EXT_KEY_SHIFT + 75,
	eAGSKeyCodeNumPad5 = AGS_EXT_KEY_SHIFT + 76,
	eAGSKeyCodeRightArrow = AGS_EXT_KEY_SHIFT + 77,
	eAGSKeyCodeEnd = AGS_EXT_KEY_SHIFT + 79,
	eAGSKeyCodeDownArrow = AGS_EXT_KEY_SHIFT + 80,
	eAGSKeyCodePageDown = AGS_EXT_KEY_SHIFT + 81,
	eAGSKeyCodeInsert = AGS_EXT_KEY_SHIFT + 82,
	eAGSKeyCodeDelete = AGS_EXT_KEY_SHIFT + 83,

	// [sonneveld] These are only used by debugging and abort keys.
	// They're based on allegro4 codes ...
	eAGSKeyCodeAltV = AGS_EXT_KEY_ALPHA(eAGSKeyCodeV),
	eAGSKeyCodeAltX = AGS_EXT_KEY_ALPHA(eAGSKeyCodeX),
	eAGSKeyCodeAltY = AGS_EXT_KEY_ALPHA(eAGSKeyCodeY),
	eAGSKeyCodeAltZ = AGS_EXT_KEY_ALPHA(eAGSKeyCodeZ),

	// The beginning of "service key list": mod keys and other special keys
	// not normally intended to affect the default game logic
	eAGSKeyCode_FirstServiceKey = 391,

	// not certain if necessary anymore (and not certain what was the origin of this value)
	eAGSKeyCodeAltTab = AGS_EXT_KEY_SHIFT + 99,

	// Mod-key codes
	// *probably* made-up numbers, not derived from allegro scan codes.
	eAGSKeyCodeLShift = 403,
	eAGSKeyCodeRShift = 404,
	eAGSKeyCodeLCtrl = 405,
	eAGSKeyCodeRCtrl = 406,
	eAGSKeyCodeLAlt = 407,

	// [sonneveld]
	// The following are the AGS_EXT_KEY_SHIFT, derived from applying arithmetic to the original keycodes.
	// These do not have a corresponding ags key enum, do not appear in the manual and may not be accessible because of OS contraints.
	eAGSKeyCodeRAlt = 420,
	// TODO: judging that above works (at least on Win), following might also work,
	// but idk which ones may be necessary; still keeping here this excerpt from an old code
	// if they'd want to be restored (also add them to script API then!).
	// Also see allegro 4's keyboard.h, where these were declared.
	/*
	case 392: __allegro_KEY_PRTSCR
	case 393: __allegro_KEY_PAUSE
	case 394: __allegro_KEY_ABNT_C1  // The ABNT_C1 (Brazilian) key
	case 395: __allegro_KEY_YEN)
	case 396: __allegro_KEY_KANA
	case 397: __allegro_KEY_CONVERT
	case 398: __allegro_KEY_NOCONVERT
	case 400: __allegro_KEY_CIRCUMFLEX
	case 402: __allegro_KEY_KANJI
	case 421: __allegro_KEY_LWIN
	case 422: __allegro_KEY_RWIN
	case 423: __allegro_KEY_MENU
	case 424: __allegro_KEY_SCRLOCK
	case 425: __allegro_KEY_NUMLOCK
	case 426: __allegro_KEY_CAPSLOCK
	*/

	// Mask defines the key code position if packed in the int32;
	// takes only 12 bits, as minimal necessary to accommodate historical codes.
	eAGSKeyMask = 0x0FFF
};

// AGS key modifiers
enum eAGSKeyMod {
	eAGSModLShift = 0x00010000,
	eAGSModRShift = 0x00020000,
	eAGSModLCtrl  = 0x00040000,
	eAGSModRCtrl  = 0x00080000,
	eAGSModLAlt   = 0x00100000,
	eAGSModRAlt   = 0x00200000,
	eAGSModNum    = 0x00400000,
	eAGSModCaps   = 0x00800000,

	// Mask defines the key mod position if packed in the int32;
	// the upper 8 bits are reserved for "input type" codes;
	// potentially may take 4 bits below (4th pos), as KeyMask takes only 12.
	eAGSModMask   = 0x00FF0000
};

// Combined key code and a textual representation in UTF-8
struct KeyInput {
	const static size_t UTF8_ARR_SIZE = 5;

	eAGSKeyCode Key = eAGSKeyCodeNone; // actual key code
	eAGSKeyCode CompatKey = eAGSKeyCodeNone; // old-style key code, combined with mods
	int         Mod = 0; // key modifiers
	int         UChar = 0; // full character value (supports unicode)
	char        Text[UTF8_ARR_SIZE]{}; // character in a string format

	KeyInput() = default;
};

// AGS own mouse button codes;
// These correspond to MouseButton enum in script and plugin API (sans special values)
enum eAGSMouseButton
{
	kMouseNone = 0,
	kMouseLeft = 1,
	kMouseRight = 2,
	kMouseMiddle = 3,
	kNumMouseButtons
};

// Tells if the AGS keycode refers to the modifier key (ctrl, alt, etc)
inline bool IsAGSModKey(eAGSKeyCode keycode) {
	return (keycode >= eAGSKeyCodeLShift && keycode <= eAGSKeyCodeLAlt) || keycode == eAGSKeyCodeRAlt;
}

// Tells if the AGS keycode refers to the service key (modifier, PrintScreen and similar);
// this lets distinct keys that normally should not affect the game
inline bool IsAGSServiceKey(eAGSKeyCode keycode) {
	return keycode >= eAGSKeyCode_FirstServiceKey;
}

// Converts eAGSKeyCode to script API code, for "on_key_press" and similar callbacks
eAGSKeyCode AGSKeyToScriptKey(eAGSKeyCode keycode);
// Converts eAGSKeyCode to ASCII text representation with the range check; returns 0 on failure
// Not unicode compatible.
char AGSKeyToText(eAGSKeyCode keycode);

} // namespace AGS3

#endif
