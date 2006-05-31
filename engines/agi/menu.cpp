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
#include "common/list.h"

namespace Agi {

Menu* menu;

// TODO: add constructor/destructor for agi_menu, agi_menu_option

struct agi_menu_option {
	int enabled;			/**< option is enabled or disabled */
	int event;			/**< menu event */
	int index;			/**< number of option in this menu */
	char *text;			/**< text of menu option */
};

struct agi_menu {
	MenuOptionList down;		/**< list head for menu options */
	int index;			/**< number of menu in menubar */
	int width;			/**< width of menu in characters */
	int height;			/**< height of menu in characters */
	int col;			/**< column of menubar entry */
	int wincol;			/**< column of menu window */
	char *text;			/**< menu name */
};

agi_menu *Menu::get_menu(int i) {
	MenuList::iterator iter;
	for (iter = menubar.begin(); iter != menubar.end(); ++iter) {
		agi_menu *m = *iter;
		if (m->index == i)
			return m;
	}
	return NULL;
}

agi_menu_option *Menu::get_menu_option(int i, int j) {
	agi_menu *m = get_menu(i);
	MenuOptionList::iterator iter;
	for (iter = m->down.begin(); iter != m->down.end(); ++iter) {	
		agi_menu_option* d = *iter;
		if (d->index == j)
			return d;
	}

	return NULL;
}

void Menu::draw_menu_bar() {
	clear_lines(0, 0, MENU_BG);
	flush_lines(0, 0);

	MenuList::iterator iter;
	for (iter = menubar.begin(); iter != menubar.end(); ++iter) {	
		agi_menu *m = *iter;
		print_text(m->text, 0, m->col, 0, 40, MENU_FG, MENU_BG);
	}

}

void Menu::draw_menu_hilite(int cur_menu) {
	agi_menu *m = get_menu(cur_menu);
	debugC(6, kDebugLevelMenu, "[%s]", m->text);
	print_text(m->text, 0, m->col, 0, 40, MENU_BG, MENU_FG);
	flush_lines(0, 0);
}

/* draw box and pulldowns. */
void Menu::draw_menu_option(int h_menu) {
	/* find which vertical menu it is */
	agi_menu *m = get_menu(h_menu);

	draw_box(m->wincol * CHAR_COLS, 1 * CHAR_LINES, (m->wincol + m->width + 2) * CHAR_COLS,
			(1 + m->height + 2) * CHAR_LINES, MENU_BG, MENU_LINE, 0);

	MenuOptionList::iterator iter;
	for (iter = m->down.begin(); iter != m->down.end(); ++iter) {	
		agi_menu_option* d = *iter;
		print_text(d->text, 0, m->wincol + 1, d->index + 2, m->width + 2,
				d->enabled ? MENU_FG : MENU_DISABLED, MENU_BG);
	}
}

void Menu::draw_menu_option_hilite(int h_menu, int v_menu) {
	agi_menu *m = get_menu(h_menu);
	agi_menu_option *d = get_menu_option(h_menu, v_menu);

	print_text(d->text, 0, m->wincol + 1, v_menu + 2, m->width + 2,
			MENU_BG, d->enabled ? MENU_FG : MENU_DISABLED);
}

void Menu::new_menu_selected(int i) {
	show_pic();
	draw_menu_bar();
	draw_menu_hilite(i);
	draw_menu_option(i);
}

bool Menu::mouse_over_text(unsigned int line, unsigned int col, char *s) {
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

#if 0
static void add_about_option() {
	const char *text = "About AGI engine";

	agi_menu_option *d = new agi_menu_option;
	d->text = strdup(text);
	d->enabled = true;
	d->event = 255;
	d->index = (v_max_menu[0] += 1);

	agi_menu *m = *menubar.begin();
	m->down.push_back(d);
	m->height++;
	if (m->width < (int)strlen(text))
		m->width = strlen(text);
}
#endif

/*
 * Public functions
 */

Menu::Menu() {
	h_index = 0;
	h_col = 1;
	h_cur_menu = 0;
	v_cur_menu = 0;
}

Menu::~Menu() {
	MenuList::iterator iterh;
	for (iterh = menubar.reverse_begin(); iterh != menubar.end(); ) {
		agi_menu *m = *iterh;
		debugC(3, kDebugLevelMenu, "deiniting hmenu %s", m->text);
		MenuOptionList::iterator iterv;
		for (iterv = m->down.reverse_begin(); iterv != m->down.end(); ) {
			agi_menu_option *d = *iterv;
			debugC(3, kDebugLevelMenu, "  deiniting vmenu %s", d->text);
			free(d->text);
			delete d;
			iterv = m->down.reverse_erase(iterv);
		}
		free(m->text);
		delete m;
		iterh = menubar.reverse_erase(iterh);
	}
}

void Menu::add(const char *s) {
	agi_menu *m = new agi_menu;
	m->text = strdup(s);
	while (m->text[strlen(m->text) - 1] == ' ')
		m->text[strlen(m->text) - 1] = 0;
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
	menubar.push_back(m);
}

void Menu::add_item(const char *s, int code) {
	int l;

	agi_menu_option* d = new agi_menu_option;
	d->text = strdup(s);
	d->enabled = true;
	d->event = code;
	d->index = v_index++;

	// add to last menu in list
	assert(menubar.reverse_begin() != menubar.end());
	agi_menu *m = *menubar.reverse_begin();
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
	m->down.push_back(d);
}

void Menu::submit() {
	debugC(3, kDebugLevelMenu, "Submitting menu");

	/* add_about_option (); */

	/* If a menu has no options, delete it */
	MenuList::iterator iter;
	for (iter = menubar.reverse_begin(); iter != menubar.end(); ) {
		agi_menu *m = *iter;
		if (m->down.empty()) {
			free(m->text);
			delete m;
			h_max_menu--;
			iter = menubar.reverse_erase(iter);
		} else {
			--iter;
		}
	}
}

bool Menu::keyhandler(int key) {
	static int clock_val;
	static int menu_active = false;
	static int button_used = 0;

	if (!getflag(F_menus_work))
		return false;

	if (!menu_active) {
		clock_val = game.clock_enabled;
		game.clock_enabled = false;
		draw_menu_bar();
	}
	/*
	 * Mouse handling
	 */
	if (mouse.button) {
		int hmenu, vmenu;

		button_used = 1;	/* Button has been used at least once */
		if (mouse.y <= CHAR_LINES) {
			/* on the menubar */
			hmenu = 0;

			MenuList::iterator iterh;
			for (iterh = menubar.begin(); iterh != menubar.end(); ++iterh) {
				agi_menu *m = *iterh;
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
			vmenu = 0;

			agi_menu *m = get_menu(h_cur_menu);
			MenuOptionList::iterator iterv;
			for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
				agi_menu_option *do1 = *iterv;
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
			agi_menu *m = get_menu(h_cur_menu);
			MenuOptionList::iterator iterv;
			for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
				agi_menu_option *d = *iterv;
				if (mouse_over_text(2 + d->index, m->wincol + 1, d->text)) {
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
	{
		debugC(6, kDebugLevelMenu | kDebugLevelInput, "KEY_ENTER");
		agi_menu_option* d = get_menu_option(h_cur_menu, v_cur_menu);
		if (d->enabled) {
			debugC(6, kDebugLevelMenu | kDebugLevelInput, "event %d registered", d->event);
			game.ev_keyp[d->event].occured = true;
			goto exit_menu;
		}
		break;
	}
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

void Menu::set_item(int event, int state) {
	/* scan all menus for event number # */

	debugC(6, kDebugLevelMenu, "event = %d, state = %d", event, state);
	MenuList::iterator iterh;
	for (iterh = menubar.begin(); iterh != menubar.end(); ++iterh) {
		agi_menu *m = *iterh;
		MenuOptionList::iterator iterv;
		for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
			agi_menu_option *d = *iterv;
			if (d->event == event) {
				d->enabled = state;
				return;
			}
		}
	}
}

void Menu::enable_all() {
	MenuList::iterator iterh;
	for (iterh = menubar.begin(); iterh != menubar.end(); ++iterh) {
		agi_menu *m = *iterh;
		MenuOptionList::iterator iterv;
		for (iterv = m->down.begin(); iterv != m->down.end(); ++iterv) {
			agi_menu_option *d = *iterv;
			d->enabled = true;
		}
	}
}

}                             // End of namespace Agi
