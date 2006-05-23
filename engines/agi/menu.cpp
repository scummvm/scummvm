/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2002 Sarien Team
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
#include "agi/sprite.h"
#include "agi/graphics.h"
#include "agi/keyboard.h"
#include "agi/menu.h"
#include "agi/text.h"
#include "agi/list.h"

namespace Agi {

struct agi_menu {
	struct list_head list;		/**< list head for menubar list */
	struct list_head down;		/**< list head for menu options */
	int index;			/**< number of menu in menubar */
	int width;			/**< width of menu in characters */
	int height;			/**< height of menu in characters */
	int col;			/**< column of menubar entry */
	int wincol;			/**< column of menu window */
	char *text;			/**< menu name */
};

struct agi_menu_option {
	struct list_head list;		/**< list head for menu options */
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	int index;			/**< number of option in this menu */
	char *text;			/**< text of menu option */
};

static LIST_HEAD(menubar);

static int h_cur_menu;
static int v_cur_menu;

static struct agi_menu *get_menu(int i) {
	struct list_head *h;
	struct agi_menu *m;

	list_for_each(h, &menubar, next) {
		m = list_entry(h, struct agi_menu, list);
		if (m->index == i)
			return m;
	}
	return NULL;
}

static struct agi_menu_option *get_menu_option(int i, int j) {
	struct list_head *h;
	struct agi_menu *m;
	struct agi_menu_option *d;

	m = get_menu(i);

	list_for_each(h, &m->down, next) {
		d = list_entry(h, struct agi_menu_option, list);
		if (d->index == j)
			return d;
	}

	return NULL;
}

static void draw_menu_bar() {
	struct list_head *h;
	struct agi_menu *m;

	clear_lines(0, 0, MENU_BG);
	flush_lines(0, 0);

	list_for_each(h, &menubar, next) {
		m = list_entry(h, struct agi_menu, list);
		print_text(m->text, 0, m->col, 0, 40, MENU_FG, MENU_BG);
	}

}

static void draw_menu_hilite(int cur_menu) {
	struct agi_menu *m;

	m = get_menu(cur_menu);
	debugC(6, kDebugLevelMenu, "[%s]", m->text);
	print_text(m->text, 0, m->col, 0, 40, MENU_BG, MENU_FG);
	flush_lines(0, 0);
}

/* draw box and pulldowns. */
static void draw_menu_option(int h_menu) {
	struct list_head *h;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	/* find which vertical menu it is */
	m = get_menu(h_menu);

	draw_box(m->wincol * CHAR_COLS, 1 * CHAR_LINES, (m->wincol + m->width + 2) * CHAR_COLS,
			(1 + m->height + 2) * CHAR_LINES, MENU_BG, MENU_LINE, 0);

	list_for_each(h, &m->down, next) {
		d = list_entry(h, struct agi_menu_option, list);
		print_text(d->text, 0, m->wincol + 1, d->index + 2, m->width + 2,
				d->enabled ? MENU_FG : MENU_DISABLED, MENU_BG);
	}
}

static void draw_menu_option_hilite(int h_menu, int v_menu) {
	struct agi_menu *m;
	struct agi_menu_option *d;

	m = get_menu(h_menu);
	d = get_menu_option(h_menu, v_menu);

	print_text(d->text, 0, m->wincol + 1, v_menu + 2, m->width + 2,
			MENU_BG, d->enabled ? MENU_FG : MENU_DISABLED);
}

static void new_menu_selected(int i) {
	show_pic();
	draw_menu_bar();
	draw_menu_hilite(i);
	draw_menu_option(i);
}

#ifdef USE_MOUSE
static int mouse_over_text(unsigned int line, unsigned int col, char *s) {
	if (mouse.x < col * CHAR_COLS)
		return false;

	if (mouse.x > (col + strlen(s)) * CHAR_COLS)
		return false;

	if (mouse.y < line * CHAR_LINES)
		return false;

	if (mouse.y >= (line + 1) * CHAR_LINES)
		return false;

	return true;
}
#endif

static int h_index;
static int v_index;
static int h_col;
static int h_max_menu;
static int v_max_menu[10];

#if 0
static void add_about_option() {
	struct agi_menu *m;
	struct agi_menu_option *d;
	char text[] = "About AGI engine";

	d = malloc(sizeof(struct agi_menu_option));
	d->text = strdup(text);
	d->enabled = true;
	d->event = 255;
	d->index = (v_max_menu[0] += 1);

	m = list_entry(menubar.next, struct agi_menu, list);
	list_add_tail(&d->list, &m->down);
	m->height++;
	if (m->width < strlen(text))
		m->width = strlen(text);
}
#endif

/*
 * Public functions
 */

void menu_init() {
	h_index = 0;
	h_col = 1;
	h_cur_menu = 0;
	v_cur_menu = 0;
}

void menu_deinit() {
	struct list_head *h, *h2, *v, *v2;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	for (h = (&menubar)->prev; h != (&menubar); h = h2) {
		m = list_entry(h, struct agi_menu, list);
		h2 = h->prev;
		debugC(3, kDebugLevelMenu, "deiniting hmenu %s", m->text);
		for (v = (&m->down)->prev; v != (&m->down); v = v2) {
			d = list_entry(v, struct agi_menu_option, list);
			v2 = v->prev;
			debugC(3, kDebugLevelMenu, "  deiniting vmenu %s", d->text);
			list_del(v);
			free(d->text);
			free(d);
		}
		list_del(h);
		free(m->text);
		free(m);
	}
}

void menu_add(char *s) {
	struct agi_menu *m;

	m = (agi_menu *) malloc(sizeof(struct agi_menu));
	m->text = strdup(s);
	while (m->text[strlen(m->text) - 1] == ' ')
		m->text[strlen(m->text) - 1] = 0;
	m->down.next = &m->down;
	m->down.prev = &m->down;
	m->width = 0;
	m->height = 0;
	m->index = h_index++;
	m->col = h_col;
	m->wincol = h_col - 1;
	v_index = 0;
	v_max_menu[m->index] = 0;
	h_col += strlen(m->text) + 1;
	h_max_menu = m->index;

	debugC(3, kDebugLevelMenu, "add menu: '%s' %02x", s, m->text[strlen(m->text)]);
	list_add_tail(&m->list, &menubar);
}

void menu_add_item(char *s, int code) {
	struct agi_menu *m;
	struct agi_menu_option *d;
	int l;

	d = (agi_menu_option *) malloc(sizeof(struct agi_menu_option));
	d->text = strdup(s);
	d->enabled = true;
	d->event = code;
	d->index = v_index++;

	m = list_entry(menubar.prev, struct agi_menu, list);
	m->height++;

	v_max_menu[m->index] = d->index;

	l = strlen(d->text);
	if (l > 40)
		l = 38;
	if (m->wincol + l > 38)
		m->wincol = 38 - l;
	if (l > m->width)
		m->width = l;

	debugC(3, kDebugLevelMenu, "Adding menu item: %s (size = %d)", s, m->height);
	list_add_tail(&d->list, &m->down);
}

void menu_submit() {
	struct list_head *h, *h2;
	struct agi_menu *m = NULL;

	debugC(3, kDebugLevelMenu, "Submitting menu");

	/* add_about_option (); */

	/* If a menu has no options, delete it */
	for (h = (&menubar)->prev; h != (&menubar); h = h2) {
		m = list_entry(h, struct agi_menu, list);
		h2 = h->prev;
		if ((&m->down)->prev == (&m->down)) {
			list_del(h);
			free(m->text);
			free(m);
			h_max_menu--;
		}
	}
}

int menu_keyhandler(int key) {
	static int clock_val;
	static int menu_active = false;
	struct agi_menu_option *d;
	struct list_head *h;
	struct agi_menu *m;
	static int button_used = 0;

	if (!getflag(F_menus_work))
		return false;

	if (!menu_active) {
		clock_val = game.clock_enabled;
		game.clock_enabled = false;
		draw_menu_bar();
	}
#ifdef USE_MOUSE
	/*
	 * Mouse handling
	 */
	if (mouse.button) {
		int hmenu, vmenu;

		button_used = 1;	/* Button has been used at least once */
		if (mouse.y <= CHAR_LINES) {
			/* on the menubar */
			hmenu = 0;

			list_for_each(h, &menubar, next) {
				m = list_entry(h, struct agi_menu, list);
				if (mouse_over_text(0, m->col, m->text)) {
					break;
				} else {
					hmenu++;
				}
			}

			if (hmenu <= h_max_menu) {
				if (h_cur_menu != hmenu) {
					v_cur_menu = -1;
					new_menu_selected(hmenu);
				}
				h_cur_menu = hmenu;
			}
		} else {
			/* not in menubar */
			struct agi_menu_option *do1;

			vmenu = 0;

			m = get_menu(h_cur_menu);
			list_for_each(h, &m->down, next) {
				do1 = list_entry(h, struct agi_menu_option, list);
				if (mouse_over_text(2 + do1->index, m->wincol + 1, do1->text)) {
					break;
				} else {
					vmenu++;
				}
			}

			if (vmenu <= v_max_menu[h_cur_menu]) {
				if (v_cur_menu != vmenu) {
					draw_menu_option(h_cur_menu);
					draw_menu_option_hilite(h_cur_menu, vmenu);
				}
				v_cur_menu = vmenu;
			}
		}
	} else if (button_used) {
		/* Button released */
		button_used = 0;

		debugC(6, kDebugLevelMenu | kDebugLevelInput, "button released!");

		if (v_cur_menu < 0)
			v_cur_menu = 0;

		draw_menu_option_hilite(h_cur_menu, v_cur_menu);

		if (mouse.y <= CHAR_LINES) {
			/* on the menubar */
		} else {
			/* see which option we selected */
			m = get_menu(h_cur_menu);
			list_for_each(h, &m->down, next) {
				d = list_entry(h, struct agi_menu_option, list);
				if (mouse_over_text(2 + d->index,
					m->wincol + 1, d->text)) {
					/* activate that option */
					if (d->enabled) {
						debugC(6, kDebugLevelMenu | kDebugLevelInput, "event %d registered", d->event);
						game.ev_keyp[d->event].occured = true;
						game.ev_keyp[d->event].data = d->event;
						goto exit_menu;
					}
				}
			}
			goto exit_menu;
		}
	}
#endif				/* USE_MOUSE */

	if (!menu_active) {
		if (h_cur_menu >= 0) {
			draw_menu_hilite(h_cur_menu);
			draw_menu_option(h_cur_menu);
			if (!button_used && v_cur_menu >= 0)
				draw_menu_option_hilite(h_cur_menu, v_cur_menu);
		}
		menu_active = true;
	}

	switch (key) {
	case KEY_ESCAPE:
		debugC(6, kDebugLevelMenu | kDebugLevelInput, "KEY_ESCAPE");
		goto exit_menu;
	case KEY_ENTER:
		debugC(6, kDebugLevelMenu | kDebugLevelInput, "KEY_ENTER");
		d = get_menu_option(h_cur_menu, v_cur_menu);
		if (d->enabled) {
			debugC(6, kDebugLevelMenu | kDebugLevelInput, "event %d registered", d->event);
			game.ev_keyp[d->event].occured = true;
			goto exit_menu;
		}
		break;
	case KEY_DOWN:
	case KEY_UP:
		v_cur_menu += key == KEY_DOWN ? 1 : -1;

		if (v_cur_menu < 0)
			v_cur_menu = 0;
		if (v_cur_menu > v_max_menu[h_cur_menu])
			v_cur_menu = v_max_menu[h_cur_menu];

		draw_menu_option(h_cur_menu);
		draw_menu_option_hilite(h_cur_menu, v_cur_menu);
		break;
	case KEY_RIGHT:
	case KEY_LEFT:
		h_cur_menu += key == KEY_RIGHT ? 1 : -1;

		if (h_cur_menu < 0)
			h_cur_menu = h_max_menu;
		if (h_cur_menu > h_max_menu)
			h_cur_menu = 0;

		v_cur_menu = 0;
		new_menu_selected(h_cur_menu);
		draw_menu_option_hilite(h_cur_menu, v_cur_menu);
		break;
	}

	return true;

exit_menu:
	button_used = 0;
	show_pic();
	write_status();

	setvar(V_key, 0);
	game.keypress = 0;
	game.clock_enabled = clock_val;
	old_input_mode();
	debugC(3, kDebugLevelMenu, "exit_menu: input mode reset to %d", game.input_mode);
	menu_active = false;

	return true;
}

void menu_set_item(int event, int state) {
	struct list_head *h, *v;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	/* scan all menus for event number # */

	debugC(6, kDebugLevelMenu, "event = %d, state = %d", event, state);
	list_for_each(h, &menubar, next) {
		m = list_entry(h, struct agi_menu, list);
		list_for_each(v, &m->down, next) {
			d = list_entry(v, struct agi_menu_option, list);
			if (d->event == event) {
				d->enabled = state;
				return;
			}
		}
	}
}

void menu_enable_all() {
	struct list_head *h, *v;
	struct agi_menu *m = NULL;
	struct agi_menu_option *d = NULL;

	list_for_each(h, &menubar, next) {
		m = list_entry(h, struct agi_menu, list);
		list_for_each(v, &m->down, next) {
			d = list_entry(v, struct agi_menu_option, list);
			d->enabled = true;
		}
	}

}

}                             // End of namespace Agi
