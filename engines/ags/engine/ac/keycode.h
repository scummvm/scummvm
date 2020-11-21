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

#ifndef AGS_ENGINE_AC_KEYCODE_H
#define AGS_ENGINE_AC_KEYCODE_H

#include "core/platform.h"

#define EXTENDED_KEY_CODE ('\0')
#define EXTENDED_KEY_CODE_MACOS ('?')

#define AGS_EXT_KEY_SHIFT  300

// These are based on values in agsdefn.sh
enum eAGSKeyCode
{
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

    eAGSKeyCodeBackspace = 8,
    eAGSKeyCodeTab = 9,
    eAGSKeyCodeReturn = 13,
    eAGSKeyCodeEscape = 27,
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
    eAGSKeyCodeColon = 58,
    eAGSKeyCodeSemiColon = 59,
    eAGSKeyCodeLessThan = 60,
    eAGSKeyCodeEquals = 61,
    eAGSKeyCodeGreaterThan = 62,
    eAGSKeyCodeQuestionMark = 63,
    eAGSKeyCodeAt = 64,
    eAGSKeyCodeOpenBracket = 91,
    eAGSKeyCodeBackSlash = 92,
    eAGSKeyCodeCloseBracket = 93,
    eAGSKeyCodeUnderscore = 95,

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

    eAGSKeyCodeA = 65,
    eAGSKeyCodeB = 66,
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
    eAGSKeyCodeZ = 90,

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

    eAGSKeyCodeAltTab = AGS_EXT_KEY_SHIFT + 99,

    // These are only used by debugging and abort keys.
    // They're based on allegro4 codes so I won't expand here.
    eAGSKeyCodeAltV = 322,
    eAGSKeyCodeAltX = 324
};

#define AGS_KEYCODE_INSERT (eAGSKeyCodeInsert)
#define AGS_KEYCODE_DELETE (eAGSKeyCodeDelete)
#define AGS_KEYCODE_ALT_TAB (eAGSKeyCodeAltTab)
#define READKEY_CODE_ALT_TAB 0x4000

// Gets a key code for "on_key_press" script callback
int GetKeyForKeyPressCb(int keycode);

// Allegro4 "platform" keycode from an AGS keycode.
// Returns -1 if not found.
int PlatformKeyFromAgsKey(int key);

#endif
