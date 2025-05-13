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

#ifndef HODJNPODJ_DFA_ERRORS_H
#define HODJNPODJ_DFA_ERRORS_H

namespace Bagel {
namespace HodjNPodj {
namespace DFA {

//
// error reporting codes
//
typedef unsigned char ERROR_CODE;
#define ERR_NONE     0              // no error
#define ERR_MEMORY   1              // not enough memory
#define ERR_FOPEN    2              // error opening a file
#define ERR_FCLOSE   3              // error closing a file
#define ERR_FREAD    4              // error reading a file
#define ERR_FWRITE   5              // error writing a file
#define ERR_FSEEK    6              // error seeking a file
#define ERR_FDEL     7              // error deleting a file
#define ERR_FFIND    8              // could not find file
#define ERR_FTYPE    9              // invalid file type
#define ERR_PATH    10              // invalid path or filename
#define ERR_DISK    11              // unrecoverable disk error
#define ERR_UNKNOWN 12              // unknown error

#define ERR_FUTURE1 13              // future use
#define ERR_FUTURE2 14              //
#define ERR_FUTURE3 15              //
#define ERR_FUTURE4 16              //
#define ERR_FUTURE5 17              // future use

} // namespace DFA
} // namespace HodjNPodj
} // namespace Bagel

#endif
