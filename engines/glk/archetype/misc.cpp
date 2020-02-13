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

#include "glk/archetype/misc.h"
#include "glk/archetype/archetype.h"
#include "glk/quetzal.h"
#include "common/debug-channels.h"

namespace Glk {
namespace Archetype {

const char *const VERSION_STUB = "Archetype version ";
const double VERSION_NUM = 1.01;

size_t Bytes;
bool KeepLooking;
bool AllErrors;

void *Prior, *NextExit;

void misc_init() {
	//NextExit = ExitProc;
	//ExitProc = @exit_prog;
	//HeapError = @HeapFunc;
	//Mark(Prior)
	Bytes = 0;
	KeepLooking = true;
	AllErrors = false;
}

/*----------------------------------------------------------------------*/

bool progfile::open(const String &name) {
	filename = name;

	if (!_file.open(name)) {
		return false;

	} else {
		file_line = 0;
		line_buffer = "";
		line_pos = 0;
		newlines = false;
		consumed = true;
		last_ch = NULL_CH;

		return true;
	}
}

void progfile::close() {
	_file.close();
}

bool progfile::readChar(char &ch) {
	if (last_ch != NULL_CH) {
		ch = last_ch;
		last_ch = NULL_CH;
	} else {
		++line_pos;
		while (line_pos >= (int)line_buffer.size()) {
			if (_file.eos()) {
				ch = NULL_CH;
				return false;
			}

			line_buffer = QuetzalReader::readString(&_file);
			line_buffer += NEWLINE_CH;
			++file_line;
			line_pos = 0;
		}

		ch = line_buffer[line_pos];
	}

	return true;
}

void progfile::unreadChar(char ch) {
	last_ch = ch;
}

void progfile::sourcePos() {
	/* With the /A switch specified, multiple source_pos messages can be called,
	 * so long as there is no fatal syntax error.Otherwise, the first error
	 * of any kind, regardless of severity, is the only error printed.This is
	 * done as a courtesy to those of us without scrolling DOS windows
	 */
	if (KeepLooking) {
		if (!AllErrors)
			KeepLooking = false;

		g_vm->writeln("Error in %s at line %d", filename.c_str(), file_line);
		g_vm->writeln(line_buffer);

		String s;
		for (int i = 0; i < line_pos; ++i)
			s += ' ';
		s += '^';
		g_vm->writeln(s);
	}
}

/*----------------------------------------------------------------------*/

void add_bytes(int delta) {
	Bytes += delta;

	if (DebugMan.isDebugChannelEnabled(DEBUG_BYTES)) {
		String line = (delta >= 0) ? "Allocated   " : "Deallocated ";
		line += String::format("%.3u bytes.  Current consumed memory: %.6u", (uint)ABS(delta), (uint)Bytes);
		debug("%s", line.c_str());
	}
}

String formatFilename(const String &name, const String &ext, bool replace) {
	String s;
	int period = 0;
	bool noExt;

	// Check for a period for an extension
	period = name.lastIndexOf('.');
	noExt = period == -1;

	if (replace || noExt) {
		return name + "." + ext;
	} else {
		return String(name.c_str(), name.c_str() + period + 1) + ext;
	}
}

void load_string(Common::ReadStream *fIn, String &the_string) {
	char buffer[257];
	size_t strSize = fIn->readByte();
	size_t strSize2 = fIn->readByte();		// Redundant second copy of the length
	assert(strSize2 == strSize);

	fIn->read(buffer, strSize);
	buffer[strSize] = '\0';

	cryptstr(buffer, strSize);
	the_string = String(buffer);
}

void dump_string(Common::WriteStream *fOut, const String &the_string) {
	assert(the_string.size() < 256);
	fOut->writeByte(the_string.size());
	fOut->writeByte(the_string.size());

	if (Encryption == NONE) {
		fOut->write(the_string.c_str(), the_string.size());

	} else {
		char buffer[257];
		strncpy(buffer, the_string.c_str(), 256);
		buffer[256] = '\0';

		cryptstr(buffer, the_string.size());
		fOut->write(buffer, the_string.size());
	}
}

StringPtr NewConstStr(const String &s) {
	return new String(s);
}

void FreeConstStr(StringPtr &sp) {
	delete sp;
}

StringPtr NewDynStr(const String &s) {
	return new String(s);
}

void FreeDynStr(StringPtr &sp) {
	delete sp;
}

} // End of namespace Archetype
} // End of namespace Glk
