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

#include "gargoyle/frotz/err.h"
#include "gargoyle/frotz/frotz.h"
#include "common/textconsole.h"

namespace Gargoyle {
namespace Frotz {

const char *const Errors::ERR_MESSAGES[ERR_NUM_ERRORS] = {
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

Errors::Errors() {
	Common::fill(&_count[0], &_count[ERR_NUM_ERRORS], 0);
}

void Errors::runtimeError(int errNum) {
    int wasfirst;
    
    if (errNum <= 0 || errNum > ERR_NUM_ERRORS)
	return;

    if (g_vm->_options._err_report_mode == ERR_REPORT_FATAL
		|| (!g_vm->_options._ignore_errors && errNum <= ERR_MAX_FATAL)) {
		g_vm->_buffer.flush();
		error(ERR_MESSAGES[errNum - 1]);
		return;
    }

    wasfirst = (_count[errNum - 1] == 0);
    _count[errNum - 1]++;
    
    if ((g_vm->_options._err_report_mode == ERR_REPORT_ALWAYS)
			|| (g_vm->_options._err_report_mode == ERR_REPORT_ONCE && wasfirst)) {
		long pc;
		GET_PC(pc);
		printString("Warning: ");
		printString(ERR_MESSAGES[errNum - 1]);
		printString(" (PC = ");
		printLong(pc, 16);
		printChar(')');
        
		if (g_vm->_options._err_report_mode == ERR_REPORT_ONCE) {
			printString(" (will ignore further occurrences)");
		} else {
			printString(" (occurence ");
			printLong(_count[errNum - 1], 10);
			printChar(')');
		}

		newLine();
    }
}

void Errors::printLong(uint value, int base) {
    unsigned long i;
    char c;

    for (i = (base == 10 ? 1000000000 : 0x10000000); i != 0; i /= base)
	if (value >= i || i == 1) {
	    c = (value / i) % base;
	    printChar(c + (c <= 9 ? '0' : 'a' - 10));
	}
}

void Errors::printChar(const char c) {
	// TODO
}

void Errors::printString(const char *str) {
	// TODO
}

void Errors::newLine() {
	// TODO
}

} // End of namespace Scott
} // End of namespace Gargoyle
