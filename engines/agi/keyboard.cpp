/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2003 Sarien Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/stdafx.h"

#include "agi/agi.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/text.h"		/* remove later */

namespace Agi {

/*
 * IBM-PC keyboard scancodes
 */
uint8 scancode_table[26] = {
	30,			/* A */
	48,			/* B */
	46,			/* C */
	32,			/* D */
	18,			/* E */
	33,			/* F */
	34,			/* G */
	35,			/* H */
	23,			/* I */
	36,			/* J */
	37,			/* K */
	38,			/* L */
	50,			/* M */
	49,			/* N */
	24,			/* O */
	25,			/* P */
	16,			/* Q */
	19,			/* R */
	31,			/* S */
	20,			/* T */
	22,			/* U */
	47,			/* V */
	17,			/* W */
	45,			/* X */
	21,			/* Y */
	44			/* Z */
};

void AgiEngine::init_words() {
	game.num_ego_words = 0;
}

void AgiEngine::clean_input() {
	while (game.num_ego_words)
		free(game.ego_words[--game.num_ego_words].word);
}

void AgiEngine::get_string(int x, int y, int len, int str) {
	new_input_mode(INPUT_GETSTRING);
	stringdata.x = x;
	stringdata.y = y;
	stringdata.len = len;
	stringdata.str = str;
}

/**
 * Raw key grabber.
 * poll_keyboard() is the raw key grabber (above the gfx driver, that is).
 * It handles console keys and insulates AGI from the console. In the main
 * loop, handle_keys() handles keyboard input and ego movement.
 */
int AgiEngine::do_poll_keyboard() {
	int key = 0;

	/* If a key is ready, rip it */
	if (_gfx->keypress()) {
		key = _gfx->getKey();
		debugC(3, kDebugLevelInput, "key %02x pressed", key);
	}

	return key;
}

int AgiEngine::handle_controller(int key) {
	struct vt_entry *v = &game.view_table[0];
	int i;

	/* The Black Cauldron needs KEY_ESCAPE to use menus */
	if (key == 0 /*|| key == KEY_ESCAPE */ )
		return false;

	debugC(3, kDebugLevelInput, "key = %04x", key);

	for (i = 0; i < MAX_DIRS; i++) {
		if (game.ev_keyp[i].data == key) {
			debugC(3, kDebugLevelInput, "event %d: key press", i);
			game.ev_keyp[i].occured = true;
			report("event AC:%i occured\n", i);
			return true;
		}
	}

	if (key == BUTTON_LEFT) {
		if (getflag(F_menus_work) && g_mouse.y <= CHAR_LINES) {
			new_input_mode(INPUT_MENU);
			return true;
		}
	}

	if (game.player_control) {
		int d = 0;

		if (!KEY_ASCII(key)) {
			switch (key) {
			case KEY_UP:
				d = 1;
				break;
			case KEY_DOWN:
				d = 5;
				break;
			case KEY_LEFT:
				d = 7;
				break;
			case KEY_RIGHT:
				d = 3;
				break;
			case KEY_UP_RIGHT:
				d = 2;
				break;
			case KEY_DOWN_RIGHT:
				d = 4;
				break;
			case KEY_UP_LEFT:
				d = 8;
				break;
			case KEY_DOWN_LEFT:
				d = 6;
				break;
			}
		}

		if (key == BUTTON_LEFT &&
				(int)g_mouse.y >= game.line_user_input * CHAR_LINES &&
				(int)g_mouse.y <= (game.line_user_input + 1) * CHAR_LINES) {
			if (predictiveDialog()) {
				strcpy((char *)game.input_buffer, _predictiveResult);
				handle_keys(KEY_ENTER);
			}
			return true;
		}

		if (!opt.agimouse) {
			/* Handle mouse button events */
			if (key == BUTTON_LEFT) {
				v->flags |= ADJ_EGO_XY;
				v->parm1 = WIN_TO_PIC_X(g_mouse.x);
				v->parm2 = WIN_TO_PIC_Y(g_mouse.y);
				return true;
			}
		}

		v->flags &= ~ADJ_EGO_XY;

		if (d || key == KEY_STATIONARY) {
			v->direction = v->direction == d ? 0 : d;
			return true;
		}
	}

	return false;
}

void AgiEngine::handle_getstring(int key) {
	static int pos = 0;	/* Cursor position */
	static char buf[40];

	if (KEY_ASCII(key) == 0)
		return;

	debugC(3, kDebugLevelInput, "handling key: %02x", key);

	switch (key) {
	case BUTTON_LEFT:
		if ((int)g_mouse.y >= stringdata.y * CHAR_LINES &&
				(int)g_mouse.y <= (stringdata.y + 1) * CHAR_LINES) {
			if (predictiveDialog()) {
				strcpy(game.strings[stringdata.str], _predictiveResult);
				new_input_mode(INPUT_NORMAL);
				_gfx->printCharacter(stringdata.x + strlen(game.strings[stringdata.str]) + 1,
								stringdata.y, ' ', game.color_fg, game.color_bg);
				return;
			}
		}
		break;
	case KEY_ENTER:
		debugC(3, kDebugLevelInput, "KEY_ENTER");
		game.has_prompt = 0;
		buf[pos] = 0;
		strcpy(game.strings[stringdata.str], buf);
		debugC(3, kDebugLevelInput, "buffer=[%s]", buf);
		buf[pos = 0] = 0;
		new_input_mode(INPUT_NORMAL);
		_gfx->printCharacter(stringdata.x + strlen(game.strings[stringdata.str]) + 1,
				stringdata.y, ' ', game.color_fg, game.color_bg);
		return;
	case KEY_ESCAPE:
		debugC(3, kDebugLevelInput, "KEY_ESCAPE");
		game.has_prompt = 0;
		buf[pos = 0] = 0;
		strcpy(game.strings[stringdata.str], buf);
		new_input_mode(INPUT_NORMAL);
		/* new_input_mode (INPUT_MENU); */
		break;
	case KEY_BACKSPACE:	/*0x08: */
		if (!pos)
			break;

		_gfx->printCharacter(stringdata.x + (pos + 1), stringdata.y,
				' ', game.color_fg, game.color_bg);
		pos--;
		buf[pos] = 0;
		break;
	default:
		if (key < 0x20 || key > 0x7f)
			break;

		if (pos >= stringdata.len)
			break;

		buf[pos++] = key;
		buf[pos] = 0;

		/* Echo */
		_gfx->printCharacter(stringdata.x + pos, stringdata.y, buf[pos - 1],
				game.color_fg, game.color_bg);

		break;
	}

	/* print cursor */
	_gfx->printCharacter(stringdata.x + pos + 1, stringdata.y,
			(char)game.cursor_char, game.color_fg, game.color_bg);
}

void AgiEngine::handle_keys(int key) {
	uint8 *p = NULL;
	int c = 0;
	static uint8 formated_entry[256];
	int l = game.line_user_input;
	int fg = game.color_fg, bg = game.color_bg;

	setvar(V_word_not_found, 0);

	debugC(3, kDebugLevelInput, "handling key: %02x", key);

	switch (key) {
	case KEY_ENTER:
		debugC(3, kDebugLevelInput, "KEY_ENTER");
		game.keypress = 0;

		/* Remove all leading spaces */
		for (p = game.input_buffer; *p && *p == 0x20; p++);

		/* Copy to internal buffer */
		for (; *p; p++) {
			/* Squash spaces */
			if (*p == 0x20 && *(p + 1) == 0x20) {
				p++;
				continue;
			}
			formated_entry[c++] = tolower(*p);
		}
		formated_entry[c++] = 0;

		/* Handle string only if it's not empty */
		if (formated_entry[0]) {
			strcpy((char *)game.echo_buffer, (const char *)game.input_buffer);
			strcpy(last_sentence, (const char *)formated_entry);
			dictionary_words(last_sentence);
		}

		/* Clear to start a new line */
		game.has_prompt = 0;
		game.input_buffer[game.cursor_pos = 0] = 0;
		debugC(3, kDebugLevelInput, "clear lines");
		clear_lines(l, l + 1, bg);
		flush_lines(l, l + 1);

		break;
	case KEY_ESCAPE:
		debugC(3, kDebugLevelInput, "KEY_ESCAPE");
		new_input_mode(INPUT_MENU);
		break;
	case KEY_BACKSPACE:
		/* Ignore backspace at start of line */
		if (game.cursor_pos == 0)
			break;

		/* erase cursor */
		_gfx->printCharacter(game.cursor_pos + 1, l, ' ', fg, bg);
		game.input_buffer[--game.cursor_pos] = 0;
		/* Print cursor */
		_gfx->printCharacter(game.cursor_pos + 1, l, game.cursor_char, fg, bg);
		break;
	default:
		/* Ignore invalid keystrokes */
		if (key < 0x20 || key > 0x7f)
			break;

		/* Maximum input size reached */
		if (game.cursor_pos >= getvar(V_max_input_chars))
			break;

		game.input_buffer[game.cursor_pos++] = key;
		game.input_buffer[game.cursor_pos] = 0;

		/* echo */
		_gfx->printCharacter(game.cursor_pos, l, game.input_buffer[game.cursor_pos - 1], fg, bg);

		/* Print cursor */
		_gfx->printCharacter(game.cursor_pos + 1, l, game.cursor_char, fg, bg);
		break;
	}
}

int AgiEngine::wait_key() {
	int key;

	/* clear key queue */
	while (_gfx->keypress()) {
		_gfx->getKey();
	}

	debugC(3, kDebugLevelInput, "waiting...");
	for (;;) {
		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		if (key == KEY_ENTER || key == KEY_ESCAPE || key == BUTTON_LEFT)
			break;
		_gfx->doUpdate();
	}
	return key;
}

int AgiEngine::wait_any_key() {
	int key;

	/* clear key queue */
	while (_gfx->keypress()) {
		_gfx->getKey();
	}

	debugC(3, kDebugLevelInput, "waiting...");
	for (;;) {
		_gfx->pollTimer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		if (key)
			break;
		_gfx->doUpdate();
	}
	return key;
}

}                             // End of namespace Agi
