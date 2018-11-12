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

#ifndef GARGOYLE_FROTZ_ERR
#define GARGOYLE_FROTZ_ERR

#include "gargoyle/frotz/frotz_types.h"

namespace Gargoyle {
namespace Frotz {

enum ErrorCode {
	ERR_TEXT_BUF_OVF   = 1,		///< Text buffer overflow
	ERR_STORE_RANGE    = 2,		///< Store out of dynamic memory
	ERR_DIV_ZERO       = 3,		///< Division by zero
	ERR_ILL_OBJ        = 4,		///< Illegal object
	ERR_ILL_ATTR       = 5,		///< Illegal attribute
	ERR_NO_PROP        = 6,		///< No such property
	ERR_STK_OVF        = 7,		///< Stack overflow
	ERR_ILL_CALL_ADDR  = 8,		///< Call to illegal address
	ERR_CALL_NON_RTN   = 9,		///< Call to non-routine
	ERR_STK_UNDF       = 10,	///< Stack underflow
	ERR_ILL_OPCODE     = 11,	///< Illegal opcode
	ERR_BAD_FRAME      = 12,	///< Bad stack frame
	ERR_ILL_JUMP_ADDR  = 13,	///< Jump to illegal address
	ERR_SAVE_IN_INTER  = 14,	///< Can't save while in interrupt
	ERR_STR3_NESTING   = 15,	///< Nesting stream #3 too deep
	ERR_ILL_WIN        = 16,	///< Illegal window
	ERR_ILL_WIN_PROP   = 17,	///< Illegal window property
	ERR_ILL_PRINT_ADDR = 18,	///< Print at illegal address
	ERR_DICT_LEN       = 19,	///< Illegal dictionary word length
	ERR_MAX_FATAL      = 19,

	// Less serious errors
	ERR_JIN_0            = 20,	///< @jin called with object 0
	ERR_GET_CHILD_0      = 21,	///< @get_child called with object 0
	ERR_GET_PARENT_0     = 22,	///< @get_parent called with object 0
	ERR_GET_SIBLING_0    = 23,	///< @get_sibling called with object 0
	ERR_GET_PROP_ADDR_0  = 24,	///< @get_prop_addr called with object 0
	ERR_GET_PROP_0       = 25,	///< @get_prop called with object 0
	ERR_PUT_PROP_0       = 26,	///< @put_prop called with object 0
	ERR_CLEAR_ATTR_0     = 27,	///< @clear_attr called with object 0
	ERR_SET_ATTR_0       = 28,	///< @set_attr called with object 0
	ERR_TEST_ATTR_0      = 29,	///< @test_attr called with object 0
	ERR_MOVE_OBJECT_0    = 30,	///< @move_object called moving object 0
	ERR_MOVE_OBJECT_TO_0 = 31,	///< @move_object called moving into object 0
	ERR_REMOVE_OBJECT_0  = 32,	///< @remove_object called with object 0
	ERR_GET_NEXT_PROP_0  = 33,	///< @get_next_prop called with object 0
	ERR_NUM_ERRORS       = 33
};

class Errors {
private:
	static const char *const ERR_MESSAGES[ERR_NUM_ERRORS];
	int _count[ERR_NUM_ERRORS];
private:
	/**
	 * Print an unsigned 32bit number in decimal or hex.
	 */
	void printLong(uint value, int base);

	/**
	 * Print a character
	 */
	void printChar(const char c);

	/**
	 * Print a string
	 */
	void printString(const char *str);

	/**
	 * Add a newline
	 */
	void newLine();
protected:
	/**
	 * Get the PC. Is implemented by the Processor class, which derives from Errors
	 */
	virtual zword getPC() const = 0;
public:
	/**
	 * Constructor
	 */
	Errors();

	/**
	 * An error has occurred. Ignore it, pass it to os_fatal or report
	 * it according to err_report_mode.
	 * @param errNum		Numeric code for error (1 to ERR_NUM_ERRORS)
	 */
	void runtimeError(ErrorCode errNum);
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
