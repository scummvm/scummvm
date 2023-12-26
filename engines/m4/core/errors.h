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

#ifndef M4_CORE_ERRORS_H
#define M4_CORE_ERRORS_H

#include "common/scummsys.h"
#include "m4/m4_types.h"

namespace M4 {

#define FL __FILE__,__LINE__
#define ERROR_FILE "error.m4"

void NORETURN_PRE error_show(const char *filename, uint32 line, quadchar errorcode, const char *fmt, ...) NORETURN_POST;
void NORETURN_PRE error_show(const char *filename, uint32 line, quadchar errorcode) NORETURN_POST;
void error_look_up(quadchar errorcode, char *result_string);

} // namespace M4

#endif
