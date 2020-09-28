/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef COMMON_CONSOLE_H
#define COMMON_CONSOLE_H

#include "common/scummsys.h"

namespace Common {

/**
 * @defgroup common_text_console Text console
 * @ingroup common
 *
 * @brief Output formatter, typically used for debugging.
 *
 * @{
 */

/**
 * An output formatter takes a source string and 'decorates' it with
 * extra information, storing the result in a destination buffer.
 *
 * A typical use is to (optionally) enhance the output given by
 * the error() and debug() functions with extra information on
 * the state of the active engine.
 */
typedef void (*OutputFormatter)(char *dst, const char *src, size_t dstSize);

/**
 * Set the output formatter used by error().
 */
void setErrorOutputFormatter(OutputFormatter f);


/**
 * A callback that is invoked by error() just before aborting.
 *
 * A typical example would be a function that shows a debug
 * console and displays the given message in it.
 */
typedef bool (*ErrorHandler)(const char *msg);

/**
 * Set a callback that is invoked by error() after the error
 * message has been printed, but before the application is
 * terminated.
 *
 * This can be used to e.g. show a debugger console.
 */
void setErrorHandler(ErrorHandler handler);

/** @} */

} // End of namespace Common

/**
 * @addtogroup common_text_console
 * @{
 */

/**
 * Print an error message to the text console and then terminate the process.
 */
void NORETURN_PRE error(MSVC_PRINTF const char *s, ...) GCC_PRINTF(1, 2) NORETURN_POST;

#ifdef DISABLE_TEXT_CONSOLE

inline void GCC_PRINTF(1, 2) warning(MSVC_PRINTF const char *s, ...) {}

#else

/**
 * Print a warning message to the text console (stderr).
 *
 * Automatically prepends the text "WARNING: " and appends
 * an exclamation mark and a newline.
 */
void warning(MSVC_PRINTF const char *s, ...) GCC_PRINTF(1, 2);

#endif
/** @} */

/**
 * Test the given expression and call error() with the file, line, and expression
 * if it is false.
 *
 * Unlike assert(), scumm_assert() is not disabled when NDEBUG is defined. Also
 * the message is visible to the user (or at least logged) on all platforms.
 */
#define scumm_assert(expr) \
	if (expr) {} \
	else \
		error("Assertion \"%s\" failed: file \"%s\", line %d", #expr, __FILE__, __LINE__)

#endif
