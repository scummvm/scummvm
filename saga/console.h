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

 // Console module header file

#ifndef SAGA_CONSOLE_H_
#define SAGA_CONSOLE_H_

namespace Saga {

#define R_CON_INPUTBUF_LEN 80

#define R_CONSOLE_BGCOLOR   0x00A0A0A0UL
#define R_CONSOLE_TXTCOLOR  0x00FFFFFFUL
#define R_CONSOLE_TXTSHADOW 0x00202020UL

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

#define R_CON_DEFAULTPOS 136
#define R_CON_DEFAULTLINES 100
#define R_CON_DEFAULTCMDS 10
#define R_CON_DROPTIME 400
#define R_CON_PRINTFLIMIT 1024
#define R_CON_LINE_H 10
#define R_CON_INPUT_H 10

class Console {
 public:
	int reg(void);
	Console(SagaEngine *vm);
	~Console(void);

	int activate(void);
	int deactivate(void);
	bool isActive(void);

	int type(int in_char);
	int draw(R_SURFACE *ds);
	int print(const char *fmt_str, ...);

	int cmdUp(void);
	int cmdDown(void);
	int pageUp(void);
	int pageDown(void);

	int dropConsole(double percent);
	int raiseConsole(double percent);

 private:
	int addLine(R_CON_SCROLLBACK *scroll, int line_max, const char *constr_p);
	int deleteLine(R_CON_SCROLLBACK *scroll);
	int deleteScroll(R_CON_SCROLLBACK *scroll);
	int setDropPos(double percent);

 private:
	SagaEngine *_vm;

	R_CON_SCROLLBACK _scrollback;
	R_CON_SCROLLBACK _history;

	int _resize;
	int _droptime;

	bool _active;
	int _yMax;
	int _lineMax;
	int _histMax;
	int _histPos;
	int _linePos;
	int _yPos;
	char *_prompt;
	int _promptW;
	char _inputBuf[R_CON_INPUTBUF_LEN + 1];
	int _inputPos;
};

} // End of namespace Saga

#endif
