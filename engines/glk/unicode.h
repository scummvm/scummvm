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

#ifndef GLK_UNICODE_H
#define GLK_UNICODE_H

#include "glk/glk_types.h"

namespace Glk {

typedef uint gli_case_block_t[2]; // upper, lower
enum BufferChangeCase { CASE_UPPER = 0, CASE_LOWER = 1, CASE_TITLE = 2, CASE_IDENT = 3 };
enum BufferChangeCond { COND_ALL = 0, COND_LINESTART = 1 };

/*
 * Get the length of a unicode string
 */
size_t strlen_uni(const uint32 *s);

/**
 * Apply a case change to the buffer. The len is the length of the buffer
 * array; numchars is the number of characters originally in it. (This
 * may be less than len.) The result will be clipped to fit len, but
 * the return value will be the full number of characters that the
 *converted string should have contained.
 */
extern uint bufferChangeCase(uint32 *buf, uint len,
							   uint numchars, BufferChangeCase destcase, BufferChangeCond cond, int changerest);

} // End of namespace Glk

#endif
