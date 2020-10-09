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

#include "glk/zcode/processor.h"
#include "common/algorithm.h"
#include "common/textconsole.h"

namespace Glk {
namespace ZCode {

const char *const Processor::ERR_MESSAGES[ERR_NUM_ERRORS] = {
	"Text buffer overflow",
	"Store out of dynamic memory",
	"Division by zero",
	"Illegal object",
	"Illegal attribute",
	"No such property",
	"Stack overflow",
	"Call to illegal address",
	"Call to non-routine",
	"Stack underflow",
	"Illegal opcode",
	"Bad stack frame",
	"Jump to illegal address",
	"Can't save while in interrupt",
	"Nesting stream #3 too deep",
	"Illegal window",
	"Illegal window property",
	"Print at illegal address",
	"Illegal dictionary word length",
	"@jin called with object 0",
	"@get_child called with object 0",
	"@get_parent called with object 0",
	"@get_sibling called with object 0",
	"@get_prop_addr called with object 0",
	"@get_prop called with object 0",
	"@put_prop called with object 0",
	"@clear_attr called with object 0",
	"@set_attr called with object 0",
	"@test_attr called with object 0",
	"@move_object called moving object 0",
	"@move_object called moving into object 0",
	"@remove_object called with object 0",
	"@get_next_prop called with object 0"
};

void Processor::flush_buffer() {
	/* Make sure we stop when flush_buffer is called from flush_buffer.
	 * Note that this is difficult to avoid as we might print a newline
	 * during flush_buffer, which might cause a newline interrupt, that
	 * might execute any arbitrary opcode, which might flush the buffer.
	 */
	if (_locked || bufferEmpty())
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

void Processor::print_char(zchar c) {
	static bool flag = false;

	if (message || ostream_memory || enable_buffering) {
		if (!flag) {
			// Characters 0 and ZC_RETURN are special cases
			if (c == ZC_RETURN) {
				new_line();
				return;
			}
			if (c == 0)
				return;

			// Flush the buffer before a whitespace or after a hyphen
			if (c == ' ' || c == ZC_INDENT || c == ZC_GAP || (_prevC == '-' && c != '-'))
				flush_buffer();

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

void Processor::print_string(const char *s) {
	char c;

	while ((c = *s++) != 0) {
		if (c == '\n')
			new_line();
		else
			print_char(c);
	}
}

void Processor::print_string_uni(const uint32 *s) {
	uint32 c;
	while ((c = *s++) != 0) {
		if (c == '\n')
			new_line();
		else
			print_char(c);
	}
}

void Processor::print_long(uint value, int base) {
	unsigned long i;
	char c;

	for (i = (base == 10 ? 1000000000 : 0x10000000); i != 0; i /= base) {
		if (value >= i || i == 1) {
			c = (value / i) % base;
			print_char(c + (c <= 9 ? '0' : 'a' - 10));
		}
	}
}

void Processor::new_line()  {
	flush_buffer();
	stream_new_line();
}

void Processor::runtimeError(ErrorCode errNum) {
	int wasfirst;

	if (errNum <= 0 || errNum > ERR_NUM_ERRORS)
		return;

	if (_err_report_mode == ERR_REPORT_FATAL
		|| (!_ignore_errors && errNum <= ERR_MAX_FATAL)) {
		flush_buffer();
		error("%s", ERR_MESSAGES[errNum - 1]);
		return;
	}

	wasfirst = (_errorCount[errNum - 1] == 0);
	_errorCount[errNum - 1]++;

	if ((_err_report_mode == ERR_REPORT_ALWAYS)
		|| (_err_report_mode == ERR_REPORT_ONCE && wasfirst)) {
		offset_t pc;
		GET_PC(pc);
		print_string("Warning: ");
		print_string(ERR_MESSAGES[errNum - 1]);
		print_string(" (PC = ");
		print_long(pc, 16);
		print_char(')');

		if (_err_report_mode == ERR_REPORT_ONCE) {
			print_string(" (will ignore further occurrences)");
		} else {
			print_string(" (occurence ");
			print_long(_errorCount[errNum - 1], 10);
			print_char(')');
		}

		new_line();
	}
}

} // End of namespace ZCode
} // End of namespace Glk
