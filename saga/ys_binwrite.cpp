/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */
#include "saga.h"

namespace Saga {

// Writes an unsigned 16 bit integer in big-endian format to the buffer
// pointed to by 'data_p'.
// If 'data_pp' is not null, the function will set it to point just beyond
// the integer written. 
void ys_write_u16_be(unsigned int u16_be, unsigned char *data_p, unsigned char **data_pp) {
	data_p[0] = (unsigned char)((u16_be >> 8) & 0xFFU);
	data_p[1] = (unsigned char)(u16_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return;
}

// Writes an unsigned 32 bit integer in big-endian format to the buffer
// pointed to by 'data_p'.
// If 'data_pp' is not null, the function will set it to point just beyond
// the integer written. 
void ys_write_u32_be(unsigned long u32_be, unsigned char *data_p, unsigned char **data_pp) {
	data_p[0] = (unsigned char)((u32_be >> 24) & 0xFFU);
	data_p[1] = (unsigned char)((u32_be >> 16) & 0xFFU);
	data_p[2] = (unsigned char)((u32_be >> 8) & 0xFFU);
	data_p[3] = (unsigned char)(u32_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return;
}

} // End of namespace Saga
