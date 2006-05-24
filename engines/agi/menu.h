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

namespace Agi {

#define MENU_BG		0x0f	/* White */
#define MENU_DISABLED	0x07	/* Grey */

#define MENU_FG		0x00	/* Black */
#define MENU_LINE	0x00	/* Black */

void menu_init(void);
void menu_deinit(void);
void menu_add(char *);
void menu_add_item(char *, int);
void menu_submit(void);
void menu_set_item(int, int);
int menu_keyhandler(int);
void menu_enable_all(void);

}                             // End of namespace Agi

#endif				/* AGI_MENU_H */
