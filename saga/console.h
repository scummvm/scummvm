/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
/*
 Description:	
 
	Console module header file

 Notes: 
*/

#ifndef SAGA_CONSOLE_H_
#define SAGA_CONSOLE_H_

namespace Saga {

#define R_CON_INPUTBUF_LEN 80

#define R_CONSOLE_BGCOLOR   0x00A0A0A0UL
#define R_CONSOLE_TXTCOLOR  0x00FFFFFFUL
#define R_CONSOLE_TXTSHADOW 0x00202020UL

struct R_CONSOLEINFO {

	int active;
	int y_max;
	int line_max;
	int hist_max;
	int hist_pos;
	int line_pos;
	int y_pos;
	char *prompt;
	int prompt_w;
	char input_buf[R_CON_INPUTBUF_LEN + 1];
	int input_pos;

};

struct R_CONSOLE_LINE {

	R_CONSOLE_LINE *next;
	R_CONSOLE_LINE *prev;
	char *str_p;
	int str_len;

};

struct R_CON_SCROLLBACK {

	R_CONSOLE_LINE *head;
	R_CONSOLE_LINE *tail;
	int lines;

};

static int
CON_AddLine(R_CON_SCROLLBACK * scroll, int line_max, const char *constr_p);

static int CON_DeleteLine(R_CON_SCROLLBACK * scroll);

static int CON_DeleteScroll(R_CON_SCROLLBACK * scroll);

static int CON_SetDropPos(double percent);

#define R_CON_DEFAULTPOS 136
#define R_CON_DEFAULTLINES 100
#define R_CON_DEFAULTCMDS 10
#define R_CON_DROPTIME 400
#define R_CON_PRINTFLIMIT 1024
#define R_CON_LINE_H 10
#define R_CON_INPUT_H 10

} // End of namespace Saga

#endif				/* R_CONSOLE_H_ */

/* end "r_console.h" */
