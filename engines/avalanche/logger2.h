/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*
 * This code is based on the original source code of Lord Avalot d'Argent version 1.3.
 * Copyright (c) 1994-1995 Mike, Mark and Thomas Thurman.
 */

/* LOGGER	Handles the logging. */

#ifndef LOGGER2_H
#define LOGGER2_H

#include "common/scummsys.h"
#include "common/str.h"

namespace Avalanche {
class AvalancheEngine;

class Logger {
public:
	void setParent(AvalancheEngine *vm);

	void log_setup(Common::String name, bool printing);

	void log_divider();    /* Prints the divider sign. */

	void log_command(Common::String x);      /* Prints a command */

	void log_scrollchar(Common::String x);      /* print one character */

	void log_italic();

	void log_roman();

	void log_epsonroman();

	void log_scrollline();    /* Set up a line for the scroll driver */

	void log_scrollendline(bool centred);

	void log_bubbleline(byte linenum, byte whom, Common::String x);

	void log_newline();

	void log_newroom(Common::String where);

	void log_aside(Common::String what);

	void log_score(uint16 credit, uint16 now);

private:
	AvalancheEngine *_vm;

	static const char divide[];

	static const Common::String quote;
	static const Common::String unquote;
	static const Common::String copyright;
	static const int16 divide_indent;

	Common::String scroll_line;
	byte scroll_line_length;

	void centre(byte size, byte x);

	void log_addstuff(Common::String x);

};

} // End of namespace Avalanche

#endif // LOGGER2_H
