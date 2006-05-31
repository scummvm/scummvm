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

#include "agi/agi.h"
#include "agi/sprite.h"		/* for commit_both() */
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/text.h"

namespace Agi {

static void print_text2(int l, const char *msg, int foff, int xoff, int yoff,
						int len, int fg, int bg) {
	int x1, y1;
	int maxx, minx, ofoff;
	int update;
	/* Note: Must be unsigned to use AGDS cyrillic characters! */
	const unsigned char *m;

	/* kludge! */
	update = 1;
	if (l == 2) {
		update = l = 0;
	}

	/* FR: strings with len == 1 were not printed
	 */
	if (len == 1) {
		put_text_character(l, xoff + foff, yoff, *msg, fg, bg);
		maxx = 1;
		minx = 0;
		ofoff = foff;
		y1 = 0;		/* Check this */
	} else {
		maxx = 0;
		minx = GFX_WIDTH;
		ofoff = foff;

		for (m = (const unsigned char *)msg, x1 = y1 = 0; *m; m++) {

			if (*m >= 0x20 || *m == 1 || *m == 2 || *m == 3) {
				/* FIXME */
				int ypos;

				ypos = (y1 * CHAR_LINES) + yoff;

				if ((x1 != (len - 1) || x1 == 39) && (ypos <= (GFX_HEIGHT - CHAR_LINES))) {
					int xpos;

					xpos = (x1 * CHAR_COLS) + xoff + foff;

					if (xpos >= GFX_WIDTH)
						continue;

					put_text_character(l, xpos, ypos, *m, fg, bg);

					if (x1 > maxx)
						maxx = x1;
					if (x1 < minx)
						minx = x1;
				}

				x1++;
				/* DF: changed the len-1 to len... */
				if (x1 == len && m[1] != '\n')
					y1++, x1 = foff = 0;
			} else {
				y1++;
				x1 = foff = 0;
			}
		}
	}

	if (l)
		return;

	if (maxx < minx)
		return;

	maxx *= CHAR_COLS;
	minx *= CHAR_COLS;

	if (update) {
		schedule_update(foff + xoff + minx, yoff, ofoff + xoff + maxx + CHAR_COLS - 1,
				yoff + y1 * CHAR_LINES + CHAR_LINES + 1);
		/* Making synchronous text updates reduces CPU load
		 * when updating status line and input area
		 */
		do_update();
	}
}

/* len is in characters, not pixels!!
 */
static void blit_textbox(const char *p, int y, int x, int len) {
	/* if x | y = -1, then centre the box */
	int xoff, yoff, lin, h, w;
	char *msg, *m;

	debugC(3, kDebugLevelText, "x=%d, y=%d, len=%d", x, y, len);
	if (game.window.active)
		close_window();

	if (x == 0 && y == 0 && len == 0)
		x = y = -1;

	if (len <= 0 || len >= 40)
		len = 32;

	xoff = x * CHAR_COLS;
	yoff = y * CHAR_LINES;
	len--;

	m = msg = word_wrap_string(agi_sprintf(p), &len);

	for (lin = 1; *m; m++) {
		/* Test \r for MacOS 8 */
		if (*m == '\n' || *m == '\r')
			lin++;
	}

	if (lin * CHAR_LINES > GFX_HEIGHT)
		lin = (GFX_HEIGHT / CHAR_LINES);

	w = (len + 2) * CHAR_COLS;
	h = (lin + 2) * CHAR_LINES;

	if (xoff < 0)
		xoff = (GFX_WIDTH - w - CHAR_COLS) / 2;
	else
		xoff -= CHAR_COLS;

	if (yoff < 0)
		yoff = (GFX_HEIGHT - 3 * CHAR_LINES - h) / 2;

	draw_window(xoff, yoff, xoff + w - 1, yoff + h - 1);

	print_text2(2, msg, 0, CHAR_COLS + xoff, CHAR_LINES + yoff,
			len + 1, MSG_BOX_TEXT, MSG_BOX_COLOUR);

	free(msg);

	do_update();
}

static void erase_textbox() {
	if (!game.window.active) {
		debugC(3, kDebugLevelText, "no window active");
		return;
	}

	debugC(4, kDebugLevelText, "x1=%d, y1=%d, x2=%d, y2=%d", game.window.x1,
			game.window.y1, game.window.x2, game.window.y2);

	restore_block(game.window.x1, game.window.y1,
			game.window.x2, game.window.y2, game.window.buffer);

	free(game.window.buffer);
	game.window.active = false;

	do_update();
}

/*
 * Public functions
 */

/**
 * Print text in the AGI engine screen.
 */
void print_text(const char *msg, int f, int x, int y, int len, int fg, int bg) {
	f *= CHAR_COLS;
	x *= CHAR_COLS;
	y *= CHAR_LINES;

	debugC(4, kDebugLevelText, "%s, %d, %d, %d, %d, %d, %d", msg, f, x, y, len, fg, bg);
	print_text2(0, agi_sprintf(msg), f, x, y, len, fg, bg);
}

/**
 * Print text in the AGI engine console.
 */
void print_text_console(const char *msg, int x, int y, int len, int fg, int bg) {
	x *= CHAR_COLS;
	y *= 10;

	print_text2(1, msg, 0, x, y, len, fg, bg);
}

/**
 * Wrap text line to the specified width. 
 * @param str  String to wrap.
 * @param len  Length of line.
 */
char *word_wrap_string(char *str, int *len) {
	/* If the message has a long word (longer than 31 character) then
	 * loop in line 239 (for (; *v != ' '; v--, c--);) can wrap
	 * around 0 and write large number in c. This causes returned
	 * length to be negative (!) and eventually crashes in calling
	 * code. The fix is simple -- remove unsigned in maxc, c, l
	 * declaration.  --Vasyl
	 */
	char *msg, *v, *e;
	int maxc, c, l = *len;

	v = msg = strdup(str);
	e = msg + strlen(msg);
	maxc = 0;

	while (42) {
		debugC(3, kDebugLevelText, "[%s], %d", msg, maxc);
		if (strchr(v, ' ') == NULL && (int)strlen(v) > l) {
			debugC(1, kDebugLevelText | kDebugLevelMain, "Word too long in message");
			l = strlen(v);
		}
		/* Must include \r for MacOS 8 */
		while ((c = strcspn(v, "\n\r")) <= l) {
			debugC(3, kDebugLevelText, "c = %d, maxc = %d", c, maxc);
			if (c > maxc)
				maxc = c;
			if ((v += c + 1) >= e)
				goto end;
		}
		c = l;
		if ((v += l) >= e)
			break;

		/* The same line that caused that bug I mentioned
		 * should also do another check:
		 * for (; *v != ' ' && *v != '\n'; v--, c--);
		 * While this does not matter in most cases, in the case of
		 * long words it caused extra \n inserted in the line
		 * preceding long word. This one is definitely non-critical;
		 * one might argue that the function is not supposed to deal
		 * with long words. BTW, that condition at the beginning of
		 * the while loop that checks word length does not make much
		 * sense -- it verifies the length of the first word but for
		 * the rest it does something odd. Overall, even with these
		 * changes the function is still not completely robust.
		 * --Vasyl
		 */
		if (*v != ' ')
			for (; *v != ' ' && *v != '\n' && *v != '\r';
			    v--, c--);
		if (c > maxc)
			maxc = c;
		*v++ = '\n';
	}
      end:
	*len = maxc;
	return msg;
}

/**
 * Remove existing window, if any.
 */
void close_window() {
	debugC(4, kDebugLevelText, "close window");
	_sprites->erase_both();
	erase_textbox();	/* remove window, if any */
	_sprites->blit_both();
	_sprites->commit_both();		/* redraw sprites */
	game.has_window = false;
}

/**
 * Display a message box.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a key is pressed.
 * @param p The text to be displayed
 */
int message_box(const char *s) {
	int k;

	_sprites->erase_both();
	blit_textbox(s, -1, -1, -1);
	_sprites->blit_both();
	k = wait_key();
	debugC(4, kDebugLevelText, "wait_key returned %02x", k);
	close_window();

	return k;
}

/**
 * Display a message box with buttons.
 * This function displays the specified message in a text box
 * centered in the screen and waits until a button is pressed.
 * @param p The text to be displayed
 * @param b NULL-terminated list of button labels
 */
int selection_box(const char *m, const char **b) {
	int x, y, i, s;
	int key, active = 0;
	int rc = -1;
	int bx[5], by[5];

	_sprites->erase_both();
	blit_textbox(m, -1, -1, -1);

	x = game.window.x1 + 5 * CHAR_COLS / 2;
	y = game.window.y2 - 5 * CHAR_LINES / 2;
	s = game.window.x2 - game.window.x1 + 1 - 5 * CHAR_COLS;
	debugC(3, kDebugLevelText, "s = %d", s);

	/* Automatically position buttons */
	for (i = 0; b[i]; i++) {
		s -= CHAR_COLS * strlen(b[i]);
	}

	if (i > 1) {
		debugC(3, kDebugLevelText, "s / %d = %d", i - 1, s / (i - 1));
		s /= (i - 1);
	} else {
		x += s / 2;
	}

	for (i = 0; b[i]; i++) {
		bx[i] = x;
		by[i] = y;
		x += CHAR_COLS * strlen(b[i]) + s;
	}

	_sprites->blit_both();

	/* clear key queue */
	while (keypress()) {
		get_key();
	}

	debugC(4, kDebugLevelText, "waiting...");
	while (42) {
		for (i = 0; b[i]; i++)
			draw_button(bx[i], by[i], b[i], i == active, 0);

		poll_timer();	/* msdos driver -> does nothing */
		key = do_poll_keyboard();
		switch (key) {
		case KEY_ENTER:
			rc = active;
			goto press;
		case KEY_ESCAPE:
			rc = -1;
			goto getout;
		case BUTTON_LEFT:
			for (i = 0; b[i]; i++) {
				if (test_button(bx[i], by[i], b[i])) {
					rc = active = i;
					goto press;
				}
			}
			break;
		case 0x09:	/* Tab */
			debugC(3, kDebugLevelText, "Focus change");
			active++;
			active %= i;
			break;
		}
		do_update();
	}

      press:
	debugC(4, kDebugLevelText, "Button pressed: %d", rc);

      getout:
	close_window();
	debugC(2, kDebugLevelText, "Result = %d", rc);

	return rc;
}

/**
 *
 */
int print(const char *p, int lin, int col, int len) {
	if (p == NULL)
		return 0;

	debugC(4, kDebugLevelText, "lin = %d, col = %d, len = %d", lin, col, len);

	if (col == 0 && lin == 0 && len == 0)
		lin = col = -1;

	if (len == 0)
		len = 30;

	blit_textbox(p, lin, col, len);

	if (getflag(F_output_mode)) {
		/* non-blocking window */
		setflag(F_output_mode, false);
		return 1;
	}

	/* blocking */

	if (game.vars[V_window_reset] == 0) {
		int k;
		setvar(V_key, 0);
		k = wait_key();
		close_window();
		return k;
	}

	/* timed window */

	debugC(3, kDebugLevelText, "f15==0, v21==%d => timed", getvar(21));
	game.msg_box_ticks = getvar(V_window_reset) * 10;
	setvar(V_key, 0);

	do {
		main_cycle();
		if (game.keypress == KEY_ENTER) {
			debugC(4, kDebugLevelText, "KEY_ENTER");
			setvar(V_window_reset, 0);
			game.keypress = 0;
			break;
		}
	} while (game.msg_box_ticks > 0);

	setvar(V_window_reset, 0);

	close_window();

	return 0;
}

/**
 *
 */
static void print_status(const char *message, ...) {
	char x[42];
	va_list args;

	va_start(args, message);

#ifdef HAVE_VSNPRINTF
	vsnprintf(x, 41, message, args);
#else
	vsprintf(x, message, args);
#endif

	va_end(args);

	debugC(4, kDebugLevelText, "fg=%d, bg=%d", STATUS_FG, STATUS_BG);
	print_text(x, 0, 0, game.line_status, 40, STATUS_FG, STATUS_BG);
}

static char *safe_strcat(char *s, const char *t) {
	if (t != NULL)
		strcat(s, t);

	return s;
}

/**
 * Formats AGI string.
 * This function turns a AGI string into a real string expanding values
 * according to the AGI format specifiers.
 * @param s  string containing the format specifier
 * @param n  logic number
 */
#define MAX_LEN 768
char *agi_sprintf(const char *s) {
	static char y[MAX_LEN];
	char x[MAX_LEN];
	char z[16], *p;

	debugC(3, kDebugLevelText, "logic %d, '%s'", game.lognum, s);
	p = x;

	for (*p = 0; *s;) {
		switch (*s) {
		case '\\':
			s++;
			goto literal;
		case '%':
			s++;
			switch (*s++) {
				int i;
			case 'v':
				i = strtoul(s, NULL, 10);
				while (*s >= '0' && *s <= '9')
					s++;
				sprintf(z, "%015i", getvar(i));

				i = 99;
				if (*s == '|') {
					s++;
					i = strtoul(s, NULL, 10);
					while (*s >= '0' && *s <= '9')
						s++;
				}

				if (i == 99) {
					/* remove all leading 0 */
					/* don't remove the 3rd zero if 000 */
					for (i = 0;
					    z[i] == '0' && i < 14; i++);
				} else {
					i = 15 - i;
				}
				safe_strcat(p, z + i);
				break;
			case '0':
				i = strtoul(s, NULL, 10) - 1;
				safe_strcat(p, object_name(i));
				break;
			case 'g':
				i = strtoul(s, NULL, 10) - 1;
				safe_strcat(p, game.logics[0].texts[i]);
				break;
			case 'w':
				i = strtoul(s, NULL, 10) - 1;
				safe_strcat(p, game.ego_words[i].word);
				break;
			case 's':
				i = strtoul(s, NULL, 10);
				safe_strcat(p, game.strings[i]);
				break;
			case 'm':
				i = strtoul(s, NULL, 10) - 1;
				if (game.logics[game.lognum].num_texts > i)
					safe_strcat(p, agi_sprintf(game. logics[game.lognum].texts[i]));
				break;
			}

			while (*s >= '0' && *s <= '9')
				s++;
			while (*p)
				p++;
			break;

		default:
		      literal:
			assert(p < x + MAX_LEN);
			*p++ = *s++;
			*p = 0;
			break;
		}
	}

	strcpy(y, x);
	return y;
}

/**
 * Write the status line.
 */
void write_status() {
	char x[64];

	if (debug_.statusline) {
		print_status("%3d(%03d) %3d,%3d(%3d,%3d)               ",
				getvar(0), getvar(1), game.view_table[0].x_pos,
				game.view_table[0].y_pos, WIN_TO_PIC_X(mouse.x),
				WIN_TO_PIC_Y(mouse.y));
		return;
	}

	if (!game.status_line) {
		int l = game.line_status;
		clear_lines(l, l, 0);
		flush_lines(l, l);
		return;
	}

	sprintf(x, " Score:%i of %-3i", game.vars[V_score], game.vars[V_max_score]);
	print_status("%-17s             Sound:%s ", x, getflag(F_sound_on) ? "on " : "off");
}

/**
 * Print user input prompt.
 */
void write_prompt() {
	int l, fg, bg, pos;

	if (!game.input_enabled || game.input_mode != INPUT_NORMAL)
		return;

	l = game.line_user_input;
	fg = game.color_fg;
	bg = game.color_bg;
	pos = game.cursor_pos;

	debugC(4, kDebugLevelText, "erase line %d", l);
	clear_lines(l, l, game.color_bg);

	debugC(4, kDebugLevelText, "prompt = '%s'", agi_sprintf(game.strings[0]));
	print_text(game.strings[0], 0, 0, l, 1, fg, bg);
	print_text((char *)game.input_buffer, 0, 1, l, pos + 1, fg, bg);
	print_character(pos + 1, l, game.cursor_char, fg, bg);

	flush_lines(l, l);
	do_update();
}

/**
 * Clear text lines in the screen.
 * @param l1  start line
 * @param l2  end line
 * @param c   color
 */
void clear_lines(int l1, int l2, int c) {
	/* do we need to adjust for +8 on topline?
	 * inc for endline so it matches the correct num
	 * ie, from 22 to 24 is 3 lines, not 2 lines.
	 */

	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	draw_rectangle(0, l1, GFX_WIDTH - 1, l2, c);
}

/**
 *
 */
void flush_lines(int l1, int l2) {
	l1 *= CHAR_LINES;
	l2 *= CHAR_LINES;
	l2 += CHAR_LINES - 1;

	flush_block(0, l1, GFX_WIDTH - 1, l2);
}

/**
 *
 */
void draw_window(int x1, int y1, int x2, int y2) {
	game.window.active = true;
	game.window.x1 = x1;
	game.window.y1 = y1;
	game.window.x2 = x2;
	game.window.y2 = y2;
	game.window.buffer = (uint8 *) malloc((x2 - x1 + 1) * (y2 - y1 + 1));

	debugC(4, kDebugLevelText, "x1=%d, y1=%d, x2=%d, y2=%d", x1, y1, x2, y2);
	save_block(x1, y1, x2, y2, game.window.buffer);
	draw_box(x1, y1, x2, y2, MSG_BOX_COLOUR, MSG_BOX_LINE, 2);
}

}                             // End of namespace Agi
