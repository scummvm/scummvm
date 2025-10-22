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

#include "gamos/gamos.h"
namespace Gamos {

KeyCodes KeyCodes::_instance;

KeyCodes::KeyCodes() {
	for (int16 i = 0; i < Common::KEYCODE_LAST; i++) {
		_winCodes[i] = WIN_INVALID;
	}

	for (int i = 0; i < 256; i++) {
		_scummCodes[i] = Common::KEYCODE_INVALID;
	}

	_winCodes[Common::KEYCODE_BACKSPACE] = WIN_BACK;
	_winCodes[Common::KEYCODE_TAB] = WIN_TAB;
	_winCodes[Common::KEYCODE_CLEAR] = WIN_CLEAR;
	_winCodes[Common::KEYCODE_RETURN] = WIN_RETURN;
	_winCodes[Common::KEYCODE_PAUSE] = WIN_PAUSE;
	_winCodes[Common::KEYCODE_ESCAPE] = WIN_ESCAPE;
	_winCodes[Common::KEYCODE_SPACE] = WIN_SPACE;
	_winCodes[Common::KEYCODE_QUOTE] = WIN_OEM_7;
	_winCodes[Common::KEYCODE_COMMA] = WIN_OEM_COMMA;
	_winCodes[Common::KEYCODE_MINUS] = WIN_OEM_MINUS;
	_winCodes[Common::KEYCODE_PERIOD] = WIN_OEM_PERIOD;
	_winCodes[Common::KEYCODE_SLASH] = WIN_OEM_2;
	_winCodes[Common::KEYCODE_0] = WIN_0;
	_winCodes[Common::KEYCODE_1] = WIN_1;
	_winCodes[Common::KEYCODE_2] = WIN_2;
	_winCodes[Common::KEYCODE_3] = WIN_3;
	_winCodes[Common::KEYCODE_4] = WIN_4;
	_winCodes[Common::KEYCODE_5] = WIN_5;
	_winCodes[Common::KEYCODE_6] = WIN_6;
	_winCodes[Common::KEYCODE_7] = WIN_7;
	_winCodes[Common::KEYCODE_8] = WIN_8;
	_winCodes[Common::KEYCODE_9] = WIN_9;
	_winCodes[Common::KEYCODE_SEMICOLON] = WIN_OEM_1;
	_winCodes[Common::KEYCODE_EQUALS] = WIN_OEM_PLUS;
	_winCodes[Common::KEYCODE_LEFTBRACKET] = WIN_OEM_4;
	_winCodes[Common::KEYCODE_BACKSLASH] = WIN_OEM_5;
	_winCodes[Common::KEYCODE_RIGHTBRACKET] = WIN_OEM_6;
	_winCodes[Common::KEYCODE_BACKQUOTE] = WIN_OEM_3;
	_winCodes[Common::KEYCODE_a] = WIN_A;
	_winCodes[Common::KEYCODE_b] = WIN_B;
	_winCodes[Common::KEYCODE_c] = WIN_C;
	_winCodes[Common::KEYCODE_d] = WIN_D;
	_winCodes[Common::KEYCODE_e] = WIN_E;
	_winCodes[Common::KEYCODE_f] = WIN_F;
	_winCodes[Common::KEYCODE_g] = WIN_G;
	_winCodes[Common::KEYCODE_h] = WIN_H;
	_winCodes[Common::KEYCODE_i] = WIN_I;
	_winCodes[Common::KEYCODE_j] = WIN_J;
	_winCodes[Common::KEYCODE_k] = WIN_K;
	_winCodes[Common::KEYCODE_l] = WIN_L;
	_winCodes[Common::KEYCODE_m] = WIN_M;
	_winCodes[Common::KEYCODE_n] = WIN_N;
	_winCodes[Common::KEYCODE_o] = WIN_O;
	_winCodes[Common::KEYCODE_p] = WIN_P;
	_winCodes[Common::KEYCODE_q] = WIN_Q;
	_winCodes[Common::KEYCODE_r] = WIN_R;
	_winCodes[Common::KEYCODE_s] = WIN_S;
	_winCodes[Common::KEYCODE_t] = WIN_T;
	_winCodes[Common::KEYCODE_u] = WIN_U;
	_winCodes[Common::KEYCODE_v] = WIN_V;
	_winCodes[Common::KEYCODE_w] = WIN_W;
	_winCodes[Common::KEYCODE_x] = WIN_X;
	_winCodes[Common::KEYCODE_y] = WIN_Y;
	_winCodes[Common::KEYCODE_z] = WIN_Z;
	_winCodes[Common::KEYCODE_DELETE] = WIN_DELETE;
	_winCodes[Common::KEYCODE_KP_PERIOD] = WIN_DECIMAL;
	_winCodes[Common::KEYCODE_KP_DIVIDE] = WIN_DIVIDE;
	_winCodes[Common::KEYCODE_KP_MULTIPLY] = WIN_MULTIPLY;
	_winCodes[Common::KEYCODE_KP_MINUS] = WIN_SUBTRACT;
	_winCodes[Common::KEYCODE_KP_PLUS] = WIN_ADD;
	_winCodes[Common::KEYCODE_KP_ENTER] = WIN_RETURN;
	_winCodes[Common::KEYCODE_KP_EQUALS] = WIN_CLEAR;
	_winCodes[Common::KEYCODE_UP] = WIN_UP;
	_winCodes[Common::KEYCODE_DOWN] = WIN_DOWN;
	_winCodes[Common::KEYCODE_RIGHT] = WIN_RIGHT;
	_winCodes[Common::KEYCODE_LEFT] = WIN_LEFT;
	_winCodes[Common::KEYCODE_INSERT] = WIN_INSERT;
	_winCodes[Common::KEYCODE_HOME] = WIN_HOME;
	_winCodes[Common::KEYCODE_END] = WIN_END;
	_winCodes[Common::KEYCODE_PAGEUP] = WIN_PRIOR;
	_winCodes[Common::KEYCODE_PAGEDOWN] = WIN_NEXT;
	_winCodes[Common::KEYCODE_F1] = WIN_F1;
	_winCodes[Common::KEYCODE_F2] = WIN_F2;
	_winCodes[Common::KEYCODE_F3] = WIN_F3;
	_winCodes[Common::KEYCODE_F4] = WIN_F4;
	_winCodes[Common::KEYCODE_F5] = WIN_F5;
	_winCodes[Common::KEYCODE_F6] = WIN_F6;
	_winCodes[Common::KEYCODE_F7] = WIN_F7;
	_winCodes[Common::KEYCODE_F8] = WIN_F8;
	_winCodes[Common::KEYCODE_F9] = WIN_F9;
	_winCodes[Common::KEYCODE_F10] = WIN_F10;
	_winCodes[Common::KEYCODE_F11] = WIN_F11;
	_winCodes[Common::KEYCODE_F12] = WIN_F12;
	_winCodes[Common::KEYCODE_F13] = WIN_F13;
	_winCodes[Common::KEYCODE_F14] = WIN_F14;
	_winCodes[Common::KEYCODE_F15] = WIN_F15;
	_winCodes[Common::KEYCODE_CAPSLOCK] = WIN_CAPITAL;
	_winCodes[Common::KEYCODE_RSHIFT] = WIN_RSHIFT;
	_winCodes[Common::KEYCODE_LSHIFT] = WIN_LSHIFT;
	_winCodes[Common::KEYCODE_RCTRL] = WIN_RCONTROL;
	_winCodes[Common::KEYCODE_LCTRL] = WIN_LCONTROL;
	_winCodes[Common::KEYCODE_RALT] = WIN_RMENU;
	_winCodes[Common::KEYCODE_LALT] = WIN_LMENU;
	_winCodes[Common::KEYCODE_SCROLLOCK] = WIN_SCROLL;
	_winCodes[Common::KEYCODE_NUMLOCK] = WIN_NUMLOCK;
	_winCodes[Common::KEYCODE_LSUPER] = WIN_LWIN;
	_winCodes[Common::KEYCODE_RSUPER] = WIN_RWIN;
	_winCodes[Common::KEYCODE_PRINT] = WIN_SNAPSHOT;
	_winCodes[Common::KEYCODE_COMPOSE] = WIN_APPS;
	_winCodes[Common::KEYCODE_KP0] = WIN_NUMPAD0;
	_winCodes[Common::KEYCODE_KP1] = WIN_NUMPAD1;
	_winCodes[Common::KEYCODE_KP2] = WIN_NUMPAD2;
	_winCodes[Common::KEYCODE_KP3] = WIN_NUMPAD3;
	_winCodes[Common::KEYCODE_KP4] = WIN_NUMPAD4;
	_winCodes[Common::KEYCODE_KP5] = WIN_NUMPAD5;
	_winCodes[Common::KEYCODE_KP6] = WIN_NUMPAD6;
	_winCodes[Common::KEYCODE_KP7] = WIN_NUMPAD7;
	_winCodes[Common::KEYCODE_KP8] = WIN_NUMPAD8;
	_winCodes[Common::KEYCODE_KP9] = WIN_NUMPAD9;
	_winCodes[Common::KEYCODE_TILDE] = WIN_OEM_3;
	_winCodes[Common::KEYCODE_F16] = WIN_F16;
	_winCodes[Common::KEYCODE_F17] = WIN_F17;
	_winCodes[Common::KEYCODE_F18] = WIN_F18;
	_winCodes[Common::KEYCODE_SLEEP] = WIN_SLEEP;
	_winCodes[Common::KEYCODE_VOLUMEUP] = WIN_VOLUME_UP;
	_winCodes[Common::KEYCODE_VOLUMEDOWN] = WIN_VOLUME_DOWN;
	_winCodes[Common::KEYCODE_AUDIONEXT] = WIN_MEDIA_NEXT_TRACK;
	_winCodes[Common::KEYCODE_AUDIOPREV] = WIN_MEDIA_PREV_TRACK;
	_winCodes[Common::KEYCODE_AUDIOSTOP] = WIN_MEDIA_STOP;
	_winCodes[Common::KEYCODE_AUDIOPLAYPAUSE] = WIN_MEDIA_PLAY_PAUSE;
	_winCodes[Common::KEYCODE_AUDIOMUTE] = WIN_VOLUME_MUTE;
	_winCodes[Common::KEYCODE_AC_SEARCH] = WIN_BROWSER_SEARCH;
	_winCodes[Common::KEYCODE_AC_HOME] = WIN_BROWSER_HOME;
	_winCodes[Common::KEYCODE_AC_BACK] = WIN_BROWSER_BACK;
	_winCodes[Common::KEYCODE_AC_FORWARD] = WIN_BROWSER_FORWARD;
	_winCodes[Common::KEYCODE_AC_STOP] = WIN_BROWSER_STOP;
	_winCodes[Common::KEYCODE_AC_REFRESH] = WIN_BROWSER_REFRESH;
	_winCodes[Common::KEYCODE_AC_BOOKMARKS] = WIN_BROWSER_FAVORITES;


	_scummCodes[WIN_BACK] = Common::KEYCODE_BACKSPACE;
	_scummCodes[WIN_TAB] = Common::KEYCODE_TAB;
	_scummCodes[WIN_CLEAR] = Common::KEYCODE_CLEAR;
	_scummCodes[WIN_RETURN] = Common::KEYCODE_RETURN;
	_scummCodes[WIN_PAUSE] = Common::KEYCODE_PAUSE;
	_scummCodes[WIN_ESCAPE] = Common::KEYCODE_ESCAPE;
	_scummCodes[WIN_SPACE] = Common::KEYCODE_SPACE;
	_scummCodes[WIN_OEM_7] = Common::KEYCODE_QUOTE;
	_scummCodes[WIN_OEM_COMMA] = Common::KEYCODE_COMMA;
	_scummCodes[WIN_OEM_MINUS] = Common::KEYCODE_MINUS;
	_scummCodes[WIN_OEM_PERIOD] = Common::KEYCODE_PERIOD;
	_scummCodes[WIN_OEM_2] = Common::KEYCODE_SLASH;
	_scummCodes[WIN_0] = Common::KEYCODE_0;
	_scummCodes[WIN_1] = Common::KEYCODE_1;
	_scummCodes[WIN_2] = Common::KEYCODE_2;
	_scummCodes[WIN_3] = Common::KEYCODE_3;
	_scummCodes[WIN_4] = Common::KEYCODE_4;
	_scummCodes[WIN_5] = Common::KEYCODE_5;
	_scummCodes[WIN_6] = Common::KEYCODE_6;
	_scummCodes[WIN_7] = Common::KEYCODE_7;
	_scummCodes[WIN_8] = Common::KEYCODE_8;
	_scummCodes[WIN_9] = Common::KEYCODE_9;
	_scummCodes[WIN_OEM_1] = Common::KEYCODE_SEMICOLON;
	_scummCodes[WIN_OEM_PLUS] = Common::KEYCODE_EQUALS;
	_scummCodes[WIN_OEM_4] = Common::KEYCODE_LEFTBRACKET;
	_scummCodes[WIN_OEM_5] = Common::KEYCODE_BACKSLASH;
	_scummCodes[WIN_OEM_6] = Common::KEYCODE_RIGHTBRACKET;
	_scummCodes[WIN_OEM_3] = Common::KEYCODE_BACKQUOTE;
	_scummCodes[WIN_A] = Common::KEYCODE_a;
	_scummCodes[WIN_B] = Common::KEYCODE_b;
	_scummCodes[WIN_C] = Common::KEYCODE_c;
	_scummCodes[WIN_D] = Common::KEYCODE_d;
	_scummCodes[WIN_E] = Common::KEYCODE_e;
	_scummCodes[WIN_F] = Common::KEYCODE_f;
	_scummCodes[WIN_G] = Common::KEYCODE_g;
	_scummCodes[WIN_H] = Common::KEYCODE_h;
	_scummCodes[WIN_I] = Common::KEYCODE_i;
	_scummCodes[WIN_J] = Common::KEYCODE_j;
	_scummCodes[WIN_K] = Common::KEYCODE_k;
	_scummCodes[WIN_L] = Common::KEYCODE_l;
	_scummCodes[WIN_M] = Common::KEYCODE_m;
	_scummCodes[WIN_N] = Common::KEYCODE_n;
	_scummCodes[WIN_O] = Common::KEYCODE_o;
	_scummCodes[WIN_P] = Common::KEYCODE_p;
	_scummCodes[WIN_Q] = Common::KEYCODE_q;
	_scummCodes[WIN_R] = Common::KEYCODE_r;
	_scummCodes[WIN_S] = Common::KEYCODE_s;
	_scummCodes[WIN_T] = Common::KEYCODE_t;
	_scummCodes[WIN_U] = Common::KEYCODE_u;
	_scummCodes[WIN_V] = Common::KEYCODE_v;
	_scummCodes[WIN_W] = Common::KEYCODE_w;
	_scummCodes[WIN_X] = Common::KEYCODE_x;
	_scummCodes[WIN_Y] = Common::KEYCODE_y;
	_scummCodes[WIN_Z] = Common::KEYCODE_z;
	_scummCodes[WIN_DELETE] = Common::KEYCODE_DELETE;
	_scummCodes[WIN_DECIMAL] = Common::KEYCODE_KP_PERIOD;
	_scummCodes[WIN_DIVIDE] = Common::KEYCODE_KP_DIVIDE;
	_scummCodes[WIN_MULTIPLY] = Common::KEYCODE_KP_MULTIPLY;
	_scummCodes[WIN_SUBTRACT] = Common::KEYCODE_KP_MINUS;
	_scummCodes[WIN_ADD] = Common::KEYCODE_KP_PLUS;
	_scummCodes[WIN_RETURN] = Common::KEYCODE_KP_ENTER;
	_scummCodes[WIN_CLEAR] = Common::KEYCODE_KP_EQUALS;
	_scummCodes[WIN_UP] = Common::KEYCODE_UP;
	_scummCodes[WIN_DOWN] = Common::KEYCODE_DOWN;
	_scummCodes[WIN_RIGHT] = Common::KEYCODE_RIGHT;
	_scummCodes[WIN_LEFT] = Common::KEYCODE_LEFT;
	_scummCodes[WIN_INSERT] = Common::KEYCODE_INSERT;
	_scummCodes[WIN_HOME] = Common::KEYCODE_HOME;
	_scummCodes[WIN_END] = Common::KEYCODE_END;
	_scummCodes[WIN_PRIOR] = Common::KEYCODE_PAGEUP;
	_scummCodes[WIN_NEXT] = Common::KEYCODE_PAGEDOWN;
	_scummCodes[WIN_F1] = Common::KEYCODE_F1;
	_scummCodes[WIN_F2] = Common::KEYCODE_F2;
	_scummCodes[WIN_F3] = Common::KEYCODE_F3;
	_scummCodes[WIN_F4] = Common::KEYCODE_F4;
	_scummCodes[WIN_F5] = Common::KEYCODE_F5;
	_scummCodes[WIN_F6] = Common::KEYCODE_F6;
	_scummCodes[WIN_F7] = Common::KEYCODE_F7;
	_scummCodes[WIN_F8] = Common::KEYCODE_F8;
	_scummCodes[WIN_F9] = Common::KEYCODE_F9;
	_scummCodes[WIN_F10] = Common::KEYCODE_F10;
	_scummCodes[WIN_F11] = Common::KEYCODE_F11;
	_scummCodes[WIN_F12] = Common::KEYCODE_F12;
	_scummCodes[WIN_F13] = Common::KEYCODE_F13;
	_scummCodes[WIN_F14] = Common::KEYCODE_F14;
	_scummCodes[WIN_F15] = Common::KEYCODE_F15;
	_scummCodes[WIN_CAPITAL] = Common::KEYCODE_CAPSLOCK;
	_scummCodes[WIN_RSHIFT] = Common::KEYCODE_RSHIFT;
	_scummCodes[WIN_LSHIFT] = Common::KEYCODE_LSHIFT;
	_scummCodes[WIN_RCONTROL] = Common::KEYCODE_RCTRL;
	_scummCodes[WIN_LCONTROL] = Common::KEYCODE_LCTRL;
	_scummCodes[WIN_RMENU] = Common::KEYCODE_RALT;
	_scummCodes[WIN_LMENU] = Common::KEYCODE_LALT;
	_scummCodes[WIN_SCROLL] = Common::KEYCODE_SCROLLOCK;
	_scummCodes[WIN_NUMLOCK] = Common::KEYCODE_NUMLOCK;
	_scummCodes[WIN_LWIN] = Common::KEYCODE_LSUPER;
	_scummCodes[WIN_RWIN] = Common::KEYCODE_RSUPER;
	_scummCodes[WIN_SNAPSHOT] = Common::KEYCODE_PRINT;
	_scummCodes[WIN_APPS] = Common::KEYCODE_COMPOSE;
	_scummCodes[WIN_NUMPAD0] = Common::KEYCODE_KP0;
	_scummCodes[WIN_NUMPAD1] = Common::KEYCODE_KP1;
	_scummCodes[WIN_NUMPAD2] = Common::KEYCODE_KP2;
	_scummCodes[WIN_NUMPAD3] = Common::KEYCODE_KP3;
	_scummCodes[WIN_NUMPAD4] = Common::KEYCODE_KP4;
	_scummCodes[WIN_NUMPAD5] = Common::KEYCODE_KP5;
	_scummCodes[WIN_NUMPAD6] = Common::KEYCODE_KP6;
	_scummCodes[WIN_NUMPAD7] = Common::KEYCODE_KP7;
	_scummCodes[WIN_NUMPAD8] = Common::KEYCODE_KP8;
	_scummCodes[WIN_NUMPAD9] = Common::KEYCODE_KP9;
	_scummCodes[WIN_OEM_3] = Common::KEYCODE_TILDE;
	_scummCodes[WIN_F16] = Common::KEYCODE_F16;
	_scummCodes[WIN_F17] = Common::KEYCODE_F17;
	_scummCodes[WIN_F18] = Common::KEYCODE_F18;
	_scummCodes[WIN_SLEEP] = Common::KEYCODE_SLEEP;
	_scummCodes[WIN_VOLUME_UP] = Common::KEYCODE_VOLUMEUP;
	_scummCodes[WIN_VOLUME_DOWN] = Common::KEYCODE_VOLUMEDOWN;
	_scummCodes[WIN_MEDIA_NEXT_TRACK] = Common::KEYCODE_AUDIONEXT;
	_scummCodes[WIN_MEDIA_PREV_TRACK] = Common::KEYCODE_AUDIOPREV;
	_scummCodes[WIN_MEDIA_STOP] = Common::KEYCODE_AUDIOSTOP;
	_scummCodes[WIN_MEDIA_PLAY_PAUSE] = Common::KEYCODE_AUDIOPLAYPAUSE;
	_scummCodes[WIN_VOLUME_MUTE] = Common::KEYCODE_AUDIOMUTE;
	_scummCodes[WIN_BROWSER_SEARCH] = Common::KEYCODE_AC_SEARCH;
	_scummCodes[WIN_BROWSER_HOME] = Common::KEYCODE_AC_HOME;
	_scummCodes[WIN_BROWSER_BACK] = Common::KEYCODE_AC_BACK;
	_scummCodes[WIN_BROWSER_FORWARD] = Common::KEYCODE_AC_FORWARD;
	_scummCodes[WIN_BROWSER_STOP] = Common::KEYCODE_AC_STOP;
	_scummCodes[WIN_BROWSER_REFRESH] = Common::KEYCODE_AC_REFRESH;
	_scummCodes[WIN_BROWSER_FAVORITES] = Common::KEYCODE_AC_BOOKMARKS;
}

}
