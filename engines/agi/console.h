/* ScummVM - Scumm Interpreter
 * Copyright (C) 2006 The ScummVM project
 *
 * Copyright (C) 1999-2001 Sarien Team
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

#ifndef __AGI_CONSOLE_H
#define __AGI_CONSOLE_H

namespace Agi {

#ifdef USE_CONSOLE

#define CONSOLE_LINES_BUFFER    80
#define CONSOLE_LINE_SIZE	(GFX_WIDTH / 8)
#define	CONSOLE_ACTIVATE_KEY	'`'
#define CONSOLE_SWITCH_KEY	'~'

struct agi_console {
	int active;
	int input_active;
	int index;
	int y;
	int max_y;
	int first_line;
	int count;
	char *line[CONSOLE_LINES_BUFFER];
};

struct agi_debug {
	int enabled;
	int opcodes;
	int logic0;
	int steps;
	int priority;
	int statusline;
	int ignoretriggers;
};

extern struct agi_console console;

#endif				/* USE_CONSOLE */

int console_keyhandler(int);
int console_init(void);
void console_cycle(void);
void console_lock(void);
void console_prompt(void);
void report(char *, ...);

}                             // End of namespace Agi

#endif				/* __AGI_CONSOLE_H */
