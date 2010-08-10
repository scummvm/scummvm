package org.inodes.gus.scummvm;

import android.view.KeyEvent;

import java.util.Collections;
import java.util.HashMap;
import java.util.Map;

public class Event {
	// Common::EventType enum.
	// Must be kept in sync with common/events.h
	public final static int EVENT_INVALID = 0;
	public final static int EVENT_KEYDOWN = 1;
	public final static int EVENT_KEYUP = 2;
	public final static int EVENT_MOUSEMOVE = 3;
	public final static int EVENT_LBUTTONDOWN = 4;
	public final static int EVENT_LBUTTONUP = 5;
	public final static int EVENT_RBUTTONDOWN = 6;
	public final static int EVENT_RBUTTONUP = 7;
	public final static int EVENT_WHEELUP = 8;
	public final static int EVENT_WHEELDOWN = 9;
	public final static int EVENT_QUIT = 10;
	public final static int EVENT_SCREEN_CHANGED = 11;
	public final static int EVENT_PREDICTIVE_DIALOG = 12;
	public final static int EVENT_MBUTTONDOWN = 13;
	public final static int EVENT_MBUTTONUP = 14;
	public final static int EVENT_MAINMENU = 15;
	public final static int EVENT_RTL = 16;

	// common/keyboard.h
	public final static int ASCII_F1 = 315;
	public final static int ASCII_F2 = 316;
	public final static int ASCII_F3 = 317;
	public final static int ASCII_F4 = 318;
	public final static int ASCII_F5 = 319;
	public final static int ASCII_F6 = 320;
	public final static int ASCII_F7 = 321;
	public final static int ASCII_F8 = 322;
	public final static int ASCII_F9 = 323;
	public final static int ASCII_F10 = 324;
	public final static int ASCII_F11 = 325;
	public final static int ASCII_F12 = 326;
	public final static int KBD_CTRL  = 1 << 0;
	public final static int KBD_ALT	  = 1 << 1;
	public final static int KBD_SHIFT = 1 << 2;

	public final static int KEYCODE_INVALID = 0;
	public final static int KEYCODE_BACKSPACE = 8;
	public final static int KEYCODE_TAB = 9;
	public final static int KEYCODE_CLEAR = 12;
	public final static int KEYCODE_RETURN = 13;
	public final static int KEYCODE_PAUSE = 19;
	public final static int KEYCODE_ESCAPE = 27;
	public final static int KEYCODE_SPACE = 32;
	public final static int KEYCODE_EXCLAIM = 33;
	public final static int KEYCODE_QUOTEDBL = 34;
	public final static int KEYCODE_HASH = 35;
	public final static int KEYCODE_DOLLAR = 36;
	public final static int KEYCODE_AMPERSAND = 38;
	public final static int KEYCODE_QUOTE = 39;
	public final static int KEYCODE_LEFTPAREN = 40;
	public final static int KEYCODE_RIGHTPAREN = 41;
	public final static int KEYCODE_ASTERISK = 42;
	public final static int KEYCODE_PLUS = 43;
	public final static int KEYCODE_COMMA = 44;
	public final static int KEYCODE_MINUS = 45;
	public final static int KEYCODE_PERIOD = 46;
	public final static int KEYCODE_SLASH = 47;
	public final static int KEYCODE_0 = 48;
	public final static int KEYCODE_1 = 49;
	public final static int KEYCODE_2 = 50;
	public final static int KEYCODE_3 = 51;
	public final static int KEYCODE_4 = 52;
	public final static int KEYCODE_5 = 53;
	public final static int KEYCODE_6 = 54;
	public final static int KEYCODE_7 = 55;
	public final static int KEYCODE_8 = 56;
	public final static int KEYCODE_9 = 57;
	public final static int KEYCODE_COLON = 58;
	public final static int KEYCODE_SEMICOLON = 59;
	public final static int KEYCODE_LESS = 60;
	public final static int KEYCODE_EQUALS = 61;
	public final static int KEYCODE_GREATER = 62;
	public final static int KEYCODE_QUESTION = 63;
	public final static int KEYCODE_AT = 64;
	public final static int KEYCODE_LEFTBRACKET = 91;
	public final static int KEYCODE_BACKSLASH = 92;
	public final static int KEYCODE_RIGHTBRACKET = 93;
	public final static int KEYCODE_CARET = 94;
	public final static int KEYCODE_UNDERSCORE = 95;
	public final static int KEYCODE_BACKQUOTE = 96;
	public final static int KEYCODE_a = 97;
	public final static int KEYCODE_b = 98;
	public final static int KEYCODE_c = 99;
	public final static int KEYCODE_d = 100;
	public final static int KEYCODE_e = 101;
	public final static int KEYCODE_f = 102;
	public final static int KEYCODE_g = 103;
	public final static int KEYCODE_h = 104;
	public final static int KEYCODE_i = 105;
	public final static int KEYCODE_j = 106;
	public final static int KEYCODE_k = 107;
	public final static int KEYCODE_l = 108;
	public final static int KEYCODE_m = 109;
	public final static int KEYCODE_n = 110;
	public final static int KEYCODE_o = 111;
	public final static int KEYCODE_p = 112;
	public final static int KEYCODE_q = 113;
	public final static int KEYCODE_r = 114;
	public final static int KEYCODE_s = 115;
	public final static int KEYCODE_t = 116;
	public final static int KEYCODE_u = 117;
	public final static int KEYCODE_v = 118;
	public final static int KEYCODE_w = 119;
	public final static int KEYCODE_x = 120;
	public final static int KEYCODE_y = 121;
	public final static int KEYCODE_z = 122;
	public final static int KEYCODE_DELETE = 127;
	// Numeric keypad
	public final static int KEYCODE_KP0 = 256;
	public final static int KEYCODE_KP1 = 257;
	public final static int KEYCODE_KP2 = 258;
	public final static int KEYCODE_KP3 = 259;
	public final static int KEYCODE_KP4 = 260;
	public final static int KEYCODE_KP5 = 261;
	public final static int KEYCODE_KP6 = 262;
	public final static int KEYCODE_KP7 = 263;
	public final static int KEYCODE_KP8 = 264;
	public final static int KEYCODE_KP9 = 265;
	public final static int KEYCODE_KP_PERIOD = 266;
	public final static int KEYCODE_KP_DIVIDE = 267;
	public final static int KEYCODE_KP_MULTIPLY = 268;
	public final static int KEYCODE_KP_MINUS = 269;
	public final static int KEYCODE_KP_PLUS = 270;
	public final static int KEYCODE_KP_ENTER = 271;
	public final static int KEYCODE_KP_EQUALS = 272;
	// Arrows + Home/End pad
	public final static int KEYCODE_UP = 273;
	public final static int KEYCODE_DOWN = 274;
	public final static int KEYCODE_RIGHT = 275;
	public final static int KEYCODE_LEFT = 276;
	public final static int KEYCODE_INSERT = 277;
	public final static int KEYCODE_HOME = 278;
	public final static int KEYCODE_END = 279;
	public final static int KEYCODE_PAGEUP = 280;
	public final static int KEYCODE_PAGEDOWN = 281;
	// Function keys
	public final static int KEYCODE_F1 = 282;
	public final static int KEYCODE_F2 = 283;
	public final static int KEYCODE_F3 = 284;
	public final static int KEYCODE_F4 = 285;
	public final static int KEYCODE_F5 = 286;
	public final static int KEYCODE_F6 = 287;
	public final static int KEYCODE_F7 = 288;
	public final static int KEYCODE_F8 = 289;
	public final static int KEYCODE_F9 = 290;
	public final static int KEYCODE_F10 = 291;
	public final static int KEYCODE_F11 = 292;
	public final static int KEYCODE_F12 = 293;
	public final static int KEYCODE_F13 = 294;
	public final static int KEYCODE_F14 = 295;
	public final static int KEYCODE_F15 = 296;
	// Key state modifier keys
	public final static int KEYCODE_NUMLOCK = 300;
	public final static int KEYCODE_CAPSLOCK = 301;
	public final static int KEYCODE_SCROLLOCK = 302;
	public final static int KEYCODE_RSHIFT = 303;
	public final static int KEYCODE_LSHIFT = 304;
	public final static int KEYCODE_RCTRL = 305;
	public final static int KEYCODE_LCTRL = 306;
	public final static int KEYCODE_RALT = 307;
	public final static int KEYCODE_LALT = 308;
	public final static int KEYCODE_RMETA = 309;
	public final static int KEYCODE_LMETA = 310;
	public final static int KEYCODE_LSUPER = 311; // Left "Windows" key
	public final static int KEYCODE_RSUPER = 312; // Right "Windows" key
	public final static int KEYCODE_MODE = 313; // "Alt Gr" key
	public final static int KEYCODE_COMPOSE = 314; // Multi-key compose key
	// Miscellaneous function keys
	public final static int KEYCODE_HELP = 315;
	public final static int KEYCODE_PRINT = 316;
	public final static int KEYCODE_SYSREQ = 317;
	public final static int KEYCODE_BREAK = 318;
	public final static int KEYCODE_MENU = 319;
	public final static int KEYCODE_POWER = 320; // Power Macintosh power key
	public final static int KEYCODE_EURO = 321; // Some european keyboards
	public final static int KEYCODE_UNDO = 322; // Atari keyboard has Undo

	// Android KeyEvent keycode -> ScummVM keycode
	public final static Map<Integer, Integer> androidKeyMap;
	static {
		Map<Integer, Integer> map = new HashMap<Integer, Integer>();

		map.put(KeyEvent.KEYCODE_DEL, KEYCODE_BACKSPACE);
		map.put(KeyEvent.KEYCODE_TAB, KEYCODE_TAB);
		map.put(KeyEvent.KEYCODE_CLEAR, KEYCODE_CLEAR);
		map.put(KeyEvent.KEYCODE_ENTER, KEYCODE_RETURN);
		//map.put(??, KEYCODE_PAUSE);
		map.put(KeyEvent.KEYCODE_BACK, KEYCODE_ESCAPE);
		map.put(KeyEvent.KEYCODE_SPACE, KEYCODE_SPACE);
		//map.put(??, KEYCODE_EXCLAIM);
		//map.put(??, KEYCODE_QUOTEDBL);
		map.put(KeyEvent.KEYCODE_POUND, KEYCODE_HASH);
		//map.put(??, KEYCODE_DOLLAR);
		//map.put(??, KEYCODE_AMPERSAND);
		map.put(KeyEvent.KEYCODE_APOSTROPHE, KEYCODE_QUOTE);
		//map.put(??, KEYCODE_LEFTPAREN);
		//map.put(??, KEYCODE_RIGHTPAREN);
		//map.put(??, KEYCODE_ASTERISK);
		map.put(KeyEvent.KEYCODE_PLUS, KEYCODE_PLUS);
		map.put(KeyEvent.KEYCODE_COMMA, KEYCODE_COMMA);
		map.put(KeyEvent.KEYCODE_MINUS, KEYCODE_MINUS);
		map.put(KeyEvent.KEYCODE_PERIOD, KEYCODE_PERIOD);
		map.put(KeyEvent.KEYCODE_SLASH, KEYCODE_SLASH);
		map.put(KeyEvent.KEYCODE_0, KEYCODE_0);
		map.put(KeyEvent.KEYCODE_1, KEYCODE_1);
		map.put(KeyEvent.KEYCODE_2, KEYCODE_2);
		map.put(KeyEvent.KEYCODE_3, KEYCODE_3);
		map.put(KeyEvent.KEYCODE_4, KEYCODE_4);
		map.put(KeyEvent.KEYCODE_5, KEYCODE_5);
		map.put(KeyEvent.KEYCODE_6, KEYCODE_6);
		map.put(KeyEvent.KEYCODE_7, KEYCODE_7);
		map.put(KeyEvent.KEYCODE_8, KEYCODE_8);
		map.put(KeyEvent.KEYCODE_9, KEYCODE_9);
		//map.put(??, KEYCODE_COLON);
		map.put(KeyEvent.KEYCODE_SEMICOLON, KEYCODE_SEMICOLON);
		//map.put(??, KEYCODE_LESS);
		map.put(KeyEvent.KEYCODE_EQUALS, KEYCODE_EQUALS);
		//map.put(??, KEYCODE_GREATER);
		//map.put(??, KEYCODE_QUESTION);
		map.put(KeyEvent.KEYCODE_AT, KEYCODE_AT);
		map.put(KeyEvent.KEYCODE_LEFT_BRACKET, KEYCODE_LEFTBRACKET);
		map.put(KeyEvent.KEYCODE_BACKSLASH, KEYCODE_BACKSLASH);
		map.put(KeyEvent.KEYCODE_RIGHT_BRACKET, KEYCODE_RIGHTBRACKET);
		//map.put(??, KEYCODE_CARET);
		//map.put(??, KEYCODE_UNDERSCORE);
		//map.put(??, KEYCODE_BACKQUOTE);
		map.put(KeyEvent.KEYCODE_A, KEYCODE_a);
		map.put(KeyEvent.KEYCODE_B, KEYCODE_b);
		map.put(KeyEvent.KEYCODE_C, KEYCODE_c);
		map.put(KeyEvent.KEYCODE_D, KEYCODE_d);
		map.put(KeyEvent.KEYCODE_E, KEYCODE_e);
		map.put(KeyEvent.KEYCODE_F, KEYCODE_f);
		map.put(KeyEvent.KEYCODE_G, KEYCODE_g);
		map.put(KeyEvent.KEYCODE_H, KEYCODE_h);
		map.put(KeyEvent.KEYCODE_I, KEYCODE_i);
		map.put(KeyEvent.KEYCODE_J, KEYCODE_j);
		map.put(KeyEvent.KEYCODE_K, KEYCODE_k);
		map.put(KeyEvent.KEYCODE_L, KEYCODE_l);
		map.put(KeyEvent.KEYCODE_M, KEYCODE_m);
		map.put(KeyEvent.KEYCODE_N, KEYCODE_n);
		map.put(KeyEvent.KEYCODE_O, KEYCODE_o);
		map.put(KeyEvent.KEYCODE_P, KEYCODE_p);
		map.put(KeyEvent.KEYCODE_Q, KEYCODE_q);
		map.put(KeyEvent.KEYCODE_R, KEYCODE_r);
		map.put(KeyEvent.KEYCODE_S, KEYCODE_s);
		map.put(KeyEvent.KEYCODE_T, KEYCODE_t);
		map.put(KeyEvent.KEYCODE_U, KEYCODE_u);
		map.put(KeyEvent.KEYCODE_V, KEYCODE_v);
		map.put(KeyEvent.KEYCODE_W, KEYCODE_w);
		map.put(KeyEvent.KEYCODE_X, KEYCODE_x);
		map.put(KeyEvent.KEYCODE_Y, KEYCODE_y);
		map.put(KeyEvent.KEYCODE_Z, KEYCODE_z);
		//map.put(KeyEvent.KEYCODE_DEL, KEYCODE_DELETE); use BACKSPACE instead
		//map.put(??, KEYCODE_KP_*);
		map.put(KeyEvent.KEYCODE_DPAD_UP, KEYCODE_UP);
		map.put(KeyEvent.KEYCODE_DPAD_DOWN, KEYCODE_DOWN);
		map.put(KeyEvent.KEYCODE_DPAD_RIGHT, KEYCODE_RIGHT);
		map.put(KeyEvent.KEYCODE_DPAD_LEFT, KEYCODE_LEFT);
		//map.put(??, KEYCODE_INSERT);
		//map.put(??, KEYCODE_HOME);
		//map.put(??, KEYCODE_END);
		//map.put(??, KEYCODE_PAGEUP);
		//map.put(??, KEYCODE_PAGEDOWN);
		//map.put(??, KEYCODE_F{1-15});
		map.put(KeyEvent.KEYCODE_NUM, KEYCODE_NUMLOCK);
		//map.put(??, KEYCODE_CAPSLOCK);
		//map.put(??, KEYCODE_SCROLLLOCK);
		map.put(KeyEvent.KEYCODE_SHIFT_RIGHT, KEYCODE_RSHIFT);
		map.put(KeyEvent.KEYCODE_SHIFT_LEFT, KEYCODE_LSHIFT);
		//map.put(??, KEYCODE_RCTRL);
		//map.put(??, KEYCODE_LCTRL);
		map.put(KeyEvent.KEYCODE_ALT_RIGHT, KEYCODE_RALT);
		map.put(KeyEvent.KEYCODE_ALT_LEFT, KEYCODE_LALT);
		// ?? META, SUPER
		// ?? MODE, COMPOSE
		// ?? HELP, PRINT, SYSREQ, BREAK, EURO, UNDO
		map.put(KeyEvent.KEYCODE_MENU, KEYCODE_MENU);
		map.put(KeyEvent.KEYCODE_POWER, KEYCODE_POWER);

		androidKeyMap = Collections.unmodifiableMap(map);
	}

	public int type;
	public boolean synthetic;
	public int kbd_keycode;
	public int kbd_ascii;
	public int kbd_flags;
	public int mouse_x;
	public int mouse_y;
	public boolean mouse_relative;	// Used for trackball events

	public Event() {
		type = EVENT_INVALID;
		synthetic = false;
	}

	public Event(int type) {
		this.type = type;
		synthetic = false;
	}

	public static Event KeyboardEvent(int type, int keycode, int ascii,
									  int flags) {
		Event e = new Event();
		e.type = type;
		e.kbd_keycode = keycode;
		e.kbd_ascii = ascii;
		e.kbd_flags = flags;
		return e;
	}

	public static Event MouseEvent(int type, int x, int y) {
		Event e = new Event();
		e.type = type;
		e.mouse_x = x;
		e.mouse_y = y;
		return e;
	}
}
