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

#ifndef AGI_MENU_H
#define AGI_MENU_H

#include "common/list.h"

namespace Agi {

#define MENU_BG		0x0f	/* White */
#define MENU_DISABLED	0x07	/* Grey */

#define MENU_FG		0x00	/* Black */
#define MENU_LINE	0x00	/* Black */

struct agi_menu;	
struct agi_menu_option;
typedef Common::List<agi_menu*> MenuList;
typedef Common::List<agi_menu_option*> MenuOptionList;

class Menu {
public:
	Menu();
	~Menu();

	void add(char *s);
	void add_item(char *s, int code);
	void submit();
	void set_item(int event, int state);
	bool keyhandler(int key);
	void enable_all();

private:
	MenuList menubar;

	int h_cur_menu;
	int v_cur_menu;

	int h_index;
	int v_index;
	int h_col;
	int h_max_menu;
	int v_max_menu[10];

	agi_menu* get_menu(int i);
	agi_menu_option *get_menu_option(int i, int j);
	void draw_menu_bar();
	void draw_menu_hilite(int cur_menu);
	void draw_menu_option(int h_menu);
	void draw_menu_option_hilite(int h_menu, int v_menu);
	void new_menu_selected(int i);
	bool mouse_over_text(unsigned int line, unsigned int col, char *s);
	
};

extern Menu* menu;

}                             // End of namespace Agi

#endif				/* AGI_MENU_H */
