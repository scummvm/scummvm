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

#define ERR_NUM_ERRORS 33
#define ERR_MAX_FATAL 19

class Errors {
private:
	static const char *const ERR_MESSAGES[ERR_NUM_ERRORS];
	int _count[ERR_NUM_ERRORS];
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
	void runtimeError(int errNum);

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
};

} // End of namespace Frotz
} // End of namespace Gargoyle

#endif
