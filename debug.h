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

struct ScummDebugger {
	Scumm *_s;
	byte _command;
	char *_parameters;

	bool _welcome;

	int _go_amount;
	
	char _cmd_buffer[256];
	
	void on_frame();
	bool do_command();
	void enter();
	int get_command();
	void attach(Scumm *s);
	void detach();

	void printActors(int act);
	void printScripts();
	
	void boxTest(int box);
};

#endif
