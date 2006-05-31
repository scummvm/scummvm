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

#ifndef AGI_TEXT_H
#define AGI_TEXT_H

#include "agi/agi.h"

namespace Agi {

int message_box(const char *);
int selection_box(const char *, const char **);
void close_window(void);
void draw_window(int, int, int, int);
void print_text(const char *, int, int, int, int, int, int);
void print_text_console(const char *, int, int, int, int, int);
int print(const char *, int, int, int);
char *word_wrap_string(char *, int *);
char *agi_sprintf(const char *);
void write_status(void);
void write_prompt(void);
void clear_lines(int, int, int);
void flush_lines(int, int);

}                             // End of namespace Agi

#endif				/* AGI_TEXT_H */
