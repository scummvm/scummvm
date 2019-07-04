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

#ifndef GLK_ALAN3_GLKIO
#define GLK_ALAN3_GLKIO

#include "glk/alan3/alan3.h"
#include "glk/alan3/jumps.h"
#include "glk/windows.h"

namespace Glk {
namespace Alan3 {

extern winid_t glkMainWin;
extern winid_t glkStatusWin;
extern bool onStatusLine;

#undef printf
#define printf glkio_printf
extern void glkio_printf(const char *, ...);

extern void showImage(int image, int align);

extern void playSound(int sound);

extern void setStyle(int style);

extern void statusline(CONTEXT);

extern bool readline(CONTEXT, char *usrBuf, size_t maxLen);

} // End of namespace Alan3
} // End of namespace Glk

#endif
