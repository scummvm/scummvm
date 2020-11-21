
#include "ac/keycode.h"

#include <allegro.h>

int GetKeyForKeyPressCb(int keycode)
{
    // lower case 'a'..'z' do not exist as keycodes, only ascii. 'A'..'Z' do though!
    return (keycode >= 'a' && keycode <= 'z') ? keycode - 32 : keycode;
}

int PlatformKeyFromAgsKey(int key)
{
    int platformKey = -1;

    switch (key) {
        // ctrl-[A-Z] keys are numbered 1-26 for A-Z
        case eAGSKeyCodeCtrlA: platformKey = 1; break;
        case eAGSKeyCodeCtrlB: platformKey = 2; break;
        case eAGSKeyCodeCtrlC: platformKey = 3; break;
        case eAGSKeyCodeCtrlD: platformKey = 4; break;
        case eAGSKeyCodeCtrlE: platformKey = 5; break;
        case eAGSKeyCodeCtrlF: platformKey = 6; break;
        case eAGSKeyCodeCtrlG: platformKey = 7; break;
        // case eAGSKeyCodeCtrlH: // overlap with backspace
        // case eAGSKeyCodeCtrlI: // overlap with tab
        case eAGSKeyCodeCtrlJ: platformKey = 10; break;
        case eAGSKeyCodeCtrlK: platformKey = 11; break;
        case eAGSKeyCodeCtrlL: platformKey = 12; break;
        // case eAGSKeyCodeCtrlM: // overlap with return
        case eAGSKeyCodeCtrlN: platformKey = 14; break;
        case eAGSKeyCodeCtrlO: platformKey = 15; break;
        case eAGSKeyCodeCtrlP: platformKey = 16; break;
        case eAGSKeyCodeCtrlQ: platformKey = 17; break;
        case eAGSKeyCodeCtrlR: platformKey = 18; break;
        case eAGSKeyCodeCtrlS: platformKey = 19; break;
        case eAGSKeyCodeCtrlT: platformKey = 20; break;
        case eAGSKeyCodeCtrlU: platformKey = 21; break;
        case eAGSKeyCodeCtrlV: platformKey = 22; break;
        case eAGSKeyCodeCtrlW: platformKey = 23; break;
        case eAGSKeyCodeCtrlX: platformKey = 24; break;
        case eAGSKeyCodeCtrlY: platformKey = 25; break;
        case eAGSKeyCodeCtrlZ: platformKey = 26; break;

        case eAGSKeyCodeBackspace: platformKey = (__allegro_KEY_BACKSPACE << 8) | 8; break;
        case eAGSKeyCodeTab: platformKey = (__allegro_KEY_TAB << 8) | 9; break;
        case eAGSKeyCodeReturn: platformKey = (__allegro_KEY_ENTER << 8) | 13; break;
        case eAGSKeyCodeEscape: platformKey = (__allegro_KEY_ESC << 8) | 27; break;
        case eAGSKeyCodeSpace: platformKey = (__allegro_KEY_SPACE << 8) | ' '; break;
        case eAGSKeyCodeExclamationMark: platformKey = '!'; break;
        case eAGSKeyCodeDoubleQuote: platformKey = '"'; break;
        case eAGSKeyCodeHash: platformKey = '#'; break;
        case eAGSKeyCodeDollar: platformKey = '$'; break;
        case eAGSKeyCodePercent: platformKey = '%'; break;
        case eAGSKeyCodeAmpersand: platformKey = '&'; break;
        case eAGSKeyCodeSingleQuote: platformKey = '\''; break;
        case eAGSKeyCodeOpenParenthesis: platformKey = '('; break;
        case eAGSKeyCodeCloseParenthesis: platformKey = ')'; break;
        case eAGSKeyCodeAsterisk: platformKey = '*'; break;
        case eAGSKeyCodePlus: platformKey = '+'; break;
        case eAGSKeyCodeComma: platformKey = ','; break;
        case eAGSKeyCodeHyphen: platformKey = '-'; break;
        case eAGSKeyCodePeriod: platformKey = '.'; break;
        case eAGSKeyCodeForwardSlash: platformKey = '/'; break;
        case eAGSKeyCodeColon: platformKey = ':'; break;
        case eAGSKeyCodeSemiColon: platformKey = ';'; break;
        case eAGSKeyCodeLessThan: platformKey = '<'; break;
        case eAGSKeyCodeEquals: platformKey = '='; break;
        case eAGSKeyCodeGreaterThan: platformKey = '>'; break;
        case eAGSKeyCodeQuestionMark: platformKey = '?'; break;
        case eAGSKeyCodeAt: platformKey = '@'; break;
        case eAGSKeyCodeOpenBracket: platformKey = '['; break;
        case eAGSKeyCodeBackSlash: platformKey = '\\'; break;
        case eAGSKeyCodeCloseBracket: platformKey = ']'; break;
        case eAGSKeyCodeUnderscore: platformKey = '_'; break;

        case eAGSKeyCode0: platformKey = (__allegro_KEY_0 << 8) | '0'; break;
        case eAGSKeyCode1: platformKey = (__allegro_KEY_1 << 8) | '1'; break;
        case eAGSKeyCode2: platformKey = (__allegro_KEY_2 << 8) | '2'; break;
        case eAGSKeyCode3: platformKey = (__allegro_KEY_3 << 8) | '3'; break;
        case eAGSKeyCode4: platformKey = (__allegro_KEY_4 << 8) | '4'; break;
        case eAGSKeyCode5: platformKey = (__allegro_KEY_5 << 8) | '5'; break;
        case eAGSKeyCode6: platformKey = (__allegro_KEY_6 << 8) | '6'; break;
        case eAGSKeyCode7: platformKey = (__allegro_KEY_7 << 8) | '7'; break;
        case eAGSKeyCode8: platformKey = (__allegro_KEY_8 << 8) | '8'; break;
        case eAGSKeyCode9: platformKey = (__allegro_KEY_9 << 8) | '9'; break;

        case eAGSKeyCodeA: platformKey = (__allegro_KEY_A << 8) | 'a'; break;
        case eAGSKeyCodeB: platformKey = (__allegro_KEY_B << 8) | 'b'; break;
        case eAGSKeyCodeC: platformKey = (__allegro_KEY_C << 8) | 'c'; break;
        case eAGSKeyCodeD: platformKey = (__allegro_KEY_D << 8) | 'd'; break;
        case eAGSKeyCodeE: platformKey = (__allegro_KEY_E << 8) | 'e'; break;
        case eAGSKeyCodeF: platformKey = (__allegro_KEY_F << 8) | 'f'; break;
        case eAGSKeyCodeG: platformKey = (__allegro_KEY_G << 8) | 'g'; break;
        case eAGSKeyCodeH: platformKey = (__allegro_KEY_H << 8) | 'h'; break;
        case eAGSKeyCodeI: platformKey = (__allegro_KEY_I << 8) | 'i'; break;
        case eAGSKeyCodeJ: platformKey = (__allegro_KEY_J << 8) | 'j'; break;
        case eAGSKeyCodeK: platformKey = (__allegro_KEY_K << 8) | 'k'; break;
        case eAGSKeyCodeL: platformKey = (__allegro_KEY_L << 8) | 'l'; break;
        case eAGSKeyCodeM: platformKey = (__allegro_KEY_M << 8) | 'm'; break;
        case eAGSKeyCodeN: platformKey = (__allegro_KEY_N << 8) | 'n'; break;
        case eAGSKeyCodeO: platformKey = (__allegro_KEY_O << 8) | 'o'; break;
        case eAGSKeyCodeP: platformKey = (__allegro_KEY_P << 8) | 'p'; break;
        case eAGSKeyCodeQ: platformKey = (__allegro_KEY_Q << 8) | 'q'; break;
        case eAGSKeyCodeR: platformKey = (__allegro_KEY_R << 8) | 'r'; break;
        case eAGSKeyCodeS: platformKey = (__allegro_KEY_S << 8) | 's'; break;
        case eAGSKeyCodeT: platformKey = (__allegro_KEY_T << 8) | 't'; break;
        case eAGSKeyCodeU: platformKey = (__allegro_KEY_U << 8) | 'u'; break;
        case eAGSKeyCodeV: platformKey = (__allegro_KEY_V << 8) | 'v'; break;
        case eAGSKeyCodeW: platformKey = (__allegro_KEY_W << 8) | 'w'; break;
        case eAGSKeyCodeX: platformKey = (__allegro_KEY_X << 8) | 'x'; break;
        case eAGSKeyCodeY: platformKey = (__allegro_KEY_Y << 8) | 'y'; break;
        case eAGSKeyCodeZ: platformKey = (__allegro_KEY_Z << 8) | 'z'; break;

        case eAGSKeyCodeF1: platformKey = __allegro_KEY_F1 << 8; break;
        case eAGSKeyCodeF2: platformKey = __allegro_KEY_F2 << 8; break;
        case eAGSKeyCodeF3: platformKey = __allegro_KEY_F3 << 8; break;
        case eAGSKeyCodeF4: platformKey = __allegro_KEY_F4 << 8; break;
        case eAGSKeyCodeF5: platformKey = __allegro_KEY_F5 << 8; break;
        case eAGSKeyCodeF6: platformKey = __allegro_KEY_F6 << 8; break;
        case eAGSKeyCodeF7: platformKey = __allegro_KEY_F7 << 8; break;
        case eAGSKeyCodeF8: platformKey = __allegro_KEY_F8 << 8; break;
        case eAGSKeyCodeF9: platformKey = __allegro_KEY_F9 << 8; break;
        case eAGSKeyCodeF10: platformKey = __allegro_KEY_F10 << 8; break;
        case eAGSKeyCodeF11: platformKey = __allegro_KEY_F11 << 8; break;
        case eAGSKeyCodeF12: platformKey = __allegro_KEY_F12 << 8; break;

        case eAGSKeyCodeHome: platformKey = __allegro_KEY_HOME << 8; break;
        case eAGSKeyCodeUpArrow: platformKey = __allegro_KEY_UP << 8; break;
        case eAGSKeyCodePageUp: platformKey = __allegro_KEY_PGUP << 8; break;
        case eAGSKeyCodeLeftArrow: platformKey = __allegro_KEY_LEFT << 8; break;
        case eAGSKeyCodeNumPad5: platformKey = __allegro_KEY_5_PAD << 8; break;
        case eAGSKeyCodeRightArrow: platformKey = __allegro_KEY_RIGHT << 8; break;
        case eAGSKeyCodeEnd: platformKey = __allegro_KEY_END << 8; break;
        case eAGSKeyCodeDownArrow: platformKey = __allegro_KEY_DOWN << 8; break;
        case eAGSKeyCodePageDown: platformKey = __allegro_KEY_PGDN << 8; break;
        case eAGSKeyCodeInsert: platformKey = __allegro_KEY_INSERT << 8; break;
        case eAGSKeyCodeDelete: platformKey = __allegro_KEY_DEL << 8; break;
    }

    return platformKey;
}
