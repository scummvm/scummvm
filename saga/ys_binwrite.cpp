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
#include <stddef.h>

namespace Saga {
void
ys_write_u8(unsigned int u8, unsigned char *data_p, unsigned char **data_pp)
{
	*data_p = (unsigned char)(u8 & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 1;
	}

	return;
}

void
ys_write_u16_be(unsigned int u16_be,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 16 bit integer in big-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)((u16_be >> 8) & 0xFFU);
	data_p[1] = (unsigned char)(u16_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return;
}

void
ys_write_u16_le(unsigned int u16_le,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 16 bit integer in little-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)(u16_le & 0xFFU);
	data_p[1] = (unsigned char)((u16_le >> 8) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return;
}

void
ys_write_s16_be(int s16_be, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 16 bit integer in big-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_be = s16_be;

	data_p[0] = (unsigned char)((u16_be >> 8) & 0xFFU);
	data_p[1] = (unsigned char)(u16_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return;
}

void
ys_write_s16_le(int s16_le, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 16 bit integer in little-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_le = s16_le;

	data_p[0] = (unsigned char)(u16_le & 0xFFU);
	data_p[1] = (unsigned char)((u16_le >> 8) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return;
}

void
ys_write_u24_be(unsigned long u24_be,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 24 bit integer in big-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)((u24_be >> 16) & 0xFFU);
	data_p[1] = (unsigned char)((u24_be >> 8) & 0xFFU);
	data_p[2] = (unsigned char)(u24_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return;
}

void
ys_write_u24_le(unsigned long u24_le,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 24 bit integer in little-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)(u24_le & 0xFFU);
	data_p[1] = (unsigned char)((u24_le >> 8) & 0xFFU);
	data_p[2] = (unsigned char)((u24_le >> 16) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return;
}

void
ys_write_s24_be(long s24_be, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 24 bit integer in big-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_be = s24_be;

	data_p[0] = (unsigned char)((u24_be >> 16) & 0xFFU);
	data_p[1] = (unsigned char)((u24_be >> 8) & 0xFFU);
	data_p[2] = (unsigned char)(u24_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return;
}

void
ys_write_s24_le(long s24_le, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 24 bit integer in little-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_le = s24_le;

	data_p[0] = (unsigned char)(u24_le & 0xFFU);
	data_p[1] = (unsigned char)((u24_le >> 8) & 0xFFU);
	data_p[2] = (unsigned char)((u24_le >> 16) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = (unsigned char *)(data_p + 3);
	}

	return;
}

void
ys_write_u32_be(unsigned long u32_be,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 32 bit integer in big-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)((u32_be >> 24) & 0xFFU);
	data_p[1] = (unsigned char)((u32_be >> 16) & 0xFFU);
	data_p[2] = (unsigned char)((u32_be >> 8) & 0xFFU);
	data_p[3] = (unsigned char)(u32_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return;
}

void
ys_write_u32_le(unsigned long u32_le,
    unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an unsigned 32 bit integer in little-endian format to the buffer
 *  pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	data_p[0] = (unsigned char)(u32_le & 0xFFU);
	data_p[1] = (unsigned char)((u32_le >> 8) & 0xFFU);
	data_p[2] = (unsigned char)((u32_le >> 16) & 0xFFU);
	data_p[3] = (unsigned char)((u32_le >> 24) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return;
}

void
ys_write_s32_be(long s32_be, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 32 bit integer in big-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_be = s32_be;

	data_p[0] = (unsigned char)((u32_be >> 24) & 0xFFU);
	data_p[1] = (unsigned char)((u32_be >> 16) & 0xFFU);
	data_p[2] = (unsigned char)((u32_be >> 8) & 0xFFU);
	data_p[3] = (unsigned char)(u32_be & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return;
}

void
ys_write_s32_le(long s32_le, unsigned char *data_p, unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Writes an signed 32 bit integer in little-endian format and two's
 *  complement representation to the buffer pointed to by 'data_p'.
 * If 'data_pp' is not null, the function will set it to point just beyond
 *  the integer written. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_le = s32_le;

	data_p[0] = (unsigned char)(u32_le & 0xFFU);
	data_p[1] = (unsigned char)((u32_le >> 8) & 0xFFU);
	data_p[2] = (unsigned char)((u32_le >> 16) & 0xFFU);
	data_p[3] = (unsigned char)((u32_le >> 24) & 0xFFU);

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return;
}

} // End of namespace Saga
