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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "glk/archetype/wrap.h"
#include "glk/archetype/archetype.h"

namespace Glk {
namespace Archetype {

const int
	MAXCOLS = 75,			// leave room for punctuation
	SAFETY_MARGIN = 3,
	MAXROWS = 24,

	REVERSE_VID = 3,
	BOLDFACE = 8;

int Rows;
int cursor;

void wrap_init() {
	cursor_reset();
	Rows = 0;
}

static void wrap_wait() {
#ifdef UNUSED
	char ch;

	TextColor(BOLDFACE); TextBackground(REVERSE_VID);
	write('Hit any key to continue...');
	ch := ReadKey;
	write(chr(13));
	NormVideo;
	ClrScr;				//or ClrEol if you don't want the whole screen }
	Rows : = 0
#endif
}

void wrapint(int i, bool terminate) {
	String s = String::format("%d", i);
	wrapout(s, terminate);
}

void wrapout(const String &str, bool terminate) {
	int thisline, maxchars, startnext;
	String s = str;

	// 'thisline' starts out as the maximum number of characters that can be
	// written before a newline; it gets trimmed back to being the number of
	// characters from the string that are actually written on this line. }
	maxchars = MAXCOLS - cursor;

	const char CHARS[7] = { '.', ',', ':', ';', ')', '-', '"' };
	for (int i = 0; i < 7; ++i) {
		if (s[0] == CHARS[i]) {
			maxchars += SAFETY_MARGIN;
			break;
		}
	}

	thisline = maxchars;
	while (thisline < (int)s.size()) {
		while (thisline >= 0 && s[thisline] != ' ')
			--thisline;
	}

	// If we were unable to find a wrapping point then it means one of two
	// things : a) the string is too long to fit on one line, andmust be
	// split unnaturally; or b) we are near the end of a line andmust wrap
	// the entire string; i.e.print nothing, finish the line andgo on
	if (thisline == 0 && s.size() > MAXCOLS)
	thisline = maxchars + 1;

	g_vm->writeln(s.substring(0, thisline - 1));
	++Rows;
	if (Rows >= MAXROWS)
		wrap_wait();

	startnext = thisline;
	while (startnext < (int)s.size() && s[startnext] == ' ') {
		++startnext;

		s = s.substring(startnext, s.size());
		cursor = 1;
		thisline = MAXCOLS - cursor;
	}

	g_vm->write(s);
	cursor += s.size();

	if (terminate) {
		g_vm->writeln();
		++Rows;
		if (Rows >= MAXROWS)
			wrap_wait();
		cursor = 1;
	}
}

void wraperr(const String &s) {
	if (cursor > 1)
		g_vm->writeln();
	g_vm->writeln(s);

	String tmp;
	for (int i = 1; i < cursor; ++i)
		tmp += ' ';
	g_vm->write(tmp);
}

StringPtr ReadLine(bool full_line) {
	String s;

	if (full_line)
		g_vm->readln(s);
	else
		s = g_vm->ReadKey();

	return NewDynStr(s);
}

void cursor_reset() {
	cursor = 1;
}

} // End of namespace Archetype
} // End of namespace Glk
