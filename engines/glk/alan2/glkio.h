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

#ifndef GLK_ALAN2_GLKIO
#define GLK_ALAN2_GLKIO

/* Header file for Glk output for Alan interpreter
 */

#include "glk/windows.h"
#include "glk/alan2/types.h"

namespace Glk {
namespace Alan2 {

extern winid_t glkMainWin;
extern winid_t glkStatusWin;

/* NB: this header must be included in any file which calls print() */

#undef printf
#define printf glkio_printf

void glkio_printf(const char *, ...);

#define LINELENGTH 80
#define HISTORYLENGTH 20

extern Boolean readline(char usrbuf[]);

} // End of namespace Alan2
} // End of namespace Glk

#endif

