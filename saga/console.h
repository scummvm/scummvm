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

#define CON_INPUTBUF_LEN 80

#define CONSOLE_BGCOLOR   0x00A0A0A0UL
#define CONSOLE_TXTCOLOR  0x00FFFFFFUL
#define CONSOLE_TXTSHADOW 0x00202020UL

struct CONSOLE_LINE {
	CONSOLE_LINE *next;
	CONSOLE_LINE *prev;
	char *str_p;
	int str_len;
};

struct CON_SCROLLBACK {
	CONSOLE_LINE *head;
	CONSOLE_LINE *tail;
	int lines;
};

#define CON_DEFAULTPOS 136
#define CON_DEFAULTLINES 100
#define CON_DEFAULTCMDS 10
#define CON_DROPTIME 400
#define CON_PRINTFLIMIT 1024
#define CON_LINE_H 10
#define CON_INPUT_H 10

class Console {
 public:
	int reg(void);
	Console(SagaEngine *vm);
	~Console(void);

	int activate(void);
	int deactivate(void);
	bool isActive(void);

	int type(int in_char);
	int draw(SURFACE *ds);
	int print(const char *fmt_str, ...);

	int cmdUp(void);
	int cmdDown(void);
	int pageUp(void);
	int pageDown(void);

	int dropConsole(double percent);
	int raiseConsole(double percent);

 private:
	int addLine(CON_SCROLLBACK *scroll, int line_max, const char *constr_p);
	int deleteLine(CON_SCROLLBACK *scroll);
	int deleteScroll(CON_SCROLLBACK *scroll);
	int setDropPos(double percent);

 private:
	SagaEngine *_vm;

	CON_SCROLLBACK _scrollback;
	CON_SCROLLBACK _history;

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
	char _inputBuf[CON_INPUTBUF_LEN + 1];
	int _inputPos;
};

} // End of namespace Saga

#endif
