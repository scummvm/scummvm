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

#include "gargoyle/frotz/buffer.h"
#include "gargoyle/frotz/frotz.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Gargoyle {
namespace Frotz {

// TODO: Replace these method stubs with correct calls
void stream_char(zchar) {}
void stream_word(const zchar *) {}
void stream_new_line(void) {}

Buffer::Buffer() : _bufPos(0), _locked(false), _prevC('\0') {
	Common::fill(&_buffer[0], &_buffer[TEXT_BUFFER_SIZE], '\0');
}

void Buffer::flush() {
	/* Make sure we stop when flush_buffer is called from flush_buffer.
	 * Note that this is difficult to avoid as we might print a newline
	 * during flush_buffer, which might cause a newline interrupt, that
	 * might execute any arbitrary opcode, which might flush the buffer.
	 */
	if (_locked || empty())
		return;

	// Send the buffer to the output streams
	_buffer[_bufPos] = '\0';
	
	_locked = true;
	stream_word(_buffer);
	_locked = false;

	// Reset the buffer
	_bufPos = 0;
	_prevC = '\0';
}

void Buffer::printChar(zchar c) {
	static bool flag = false;

	if (g_vm->_message || g_vm->_ostream_memory || g_vm->_enableBuffering) {
		if (!flag) {
			// Characters 0 and ZC_RETURN are special cases
			if (c == ZC_RETURN) {
				newLine();
				return;
			}
			if (c == 0)
				return;

			// Flush the buffer before a whitespace or after a hyphen
			if (c == ' ' || c == ZC_INDENT || c == ZC_GAP || (_prevC == '-' && c != '-'))
				flush();

			// Set the flag if this is part one of a style or font change
			if (c == ZC_NEW_FONT || c == ZC_NEW_STYLE)
				flag = true;

			// Remember the current character code
			_prevC = c;
		} else {
			flag = false;
		}

		// Insert the character into the buffer
		_buffer[_bufPos++] = c;

		if (_bufPos == TEXT_BUFFER_SIZE)
			error("Text buffer overflow");
	} else {
		stream_char(c);
	}
}

void Buffer::newLine()  {
	flush();
	stream_new_line();
}

} // End of namespace Scott
} // End of namespace Gargoyle
