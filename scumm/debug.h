/* ScummVM - Scumm Interpreter
 * Copyright (C) 2002 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef DEBUG_H
#define DEBUG_H

class Scumm;

//
// HACK FIXME TODO - enable this for the PURELY EXPERIMENTAL console debug mode
//
//#define USE_CONSOLE 1


class ScummDebugger {
public:
	void on_frame();
	void attach(Scumm *s);

protected:
	Scumm *_s;
	byte _command;
	char *_parameters;

	bool _welcome;

	int _go_amount;
	
	char _cmd_buffer[256];
	
	bool do_command(int cmd);
	void enter();
	int get_command();
	int parse_command(char *buf);
	void detach();

	void printActors(int act);
	void printScripts();

	void printBox(int box);
	void printBoxes();
	void boxTest(int box);
	
#ifdef USE_CONSOLE
	static bool ScummDebugger::debuggerInputCallback(ConsoleDialog *console, const char *input, void *refCon);
#endif
};

#endif
