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
#include <stdio.h>
#include "yslib.h"

namespace Saga {

void
ys_read_4cc(char *fourcc,
    const unsigned char *data_p, const unsigned char **data_pp)
{
	fourcc[0] = (char)data_p[0];
	fourcc[1] = (char)data_p[1];
	fourcc[2] = (char)data_p[2];
	fourcc[3] = (char)data_p[3];

	if (data_pp) {

		*data_pp = data_p + 4;
	}

	return;
}

unsigned int
ys_read_u8(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 8 bit integer in from the array of bytes pointed to by
 *  'data_p'. If 'data_pp' is not null, it will set '*data_pp' to point past
 *  the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u8 = *data_p;

	if (data_pp != NULL) {
		*data_pp = data_p + 1;
	}

	return u8;
}

int ys_read_s8(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 8 bit integer in two's complement notation from the array
 *  of bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u8 = *data_p;
	int s8;

#ifndef YS_ASSUME_2S_COMP
	if (u8 & 0x80U) {
		s8 = (int)(u8 - 0x80U) - 0x7F - 1;
	} else
#endif
		s8 = u8;

	if (data_pp != NULL) {
		*data_pp = data_p + 1;
	}

	return s8;
}

unsigned int
ys_read_u16_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 16 bit integer in big-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set   
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_be = ((unsigned int)data_p[0] << 8) | data_p[1];

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return u16_be;
}

unsigned int
ys_read_u16_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 16 bit integer in little-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set   
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_le = ((unsigned int)data_p[1] << 8) | data_p[0];

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return u16_le;
}

int ys_read_s16_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 16 bit integer in big-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_be = ((unsigned int)data_p[0] << 8) | data_p[1];
	int s16_be;

#ifndef YS_ASSUME_2S_COMP
	if (u16_be & 0x8000U) {
		s16_be = (int)(u16_be - 0x8000U) - 0x7FFF - 1;
	} else
#endif
		s16_be = u16_be;

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return s16_be;
}

int ys_read_s16_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 16 bit integer in little-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned int u16_le = ((unsigned int)data_p[1] << 8) | data_p[0];
	int s16_le;

#ifndef YS_ASSUME_2S_COMP
	if (u16_le & 0x8000U) {
		s16_le = (int)(u16_le - 0x8000U) - 0x7FFF - 1;
	} else
#endif
		s16_le = u16_le;

	if (data_pp != NULL) {
		*data_pp = data_p + 2;
	}

	return s16_le;
}

unsigned long
ys_read_u24_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 24 bit integer in big-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_be = ((unsigned long)data_p[0] << 16) |
	    ((unsigned long)data_p[1] << 8) | data_p[2];

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return u24_be;
}

unsigned long
ys_read_u24_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 24 bit integer in big-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_le = ((unsigned long)data_p[3] << 16) |
	    ((unsigned long)data_p[2] << 8) | data_p[0];

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return u24_le;
}

long ys_read_s24_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 24 bit integer in big-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_be = ((unsigned long)data_p[0] << 16) |
	    ((unsigned long)data_p[1] << 8) | data_p[2];
	long s24_be;

#ifndef YS_ASSUME_2S_COMP
	if (u24_be & 0x800000UL) {
		s24_be = (long)(u24_be - 0x800000UL) - 0x800000;
	} else
#endif
		s24_be = u24_be;

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return s24_be;
}

long ys_read_s24_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 24 bit integer in little-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned long u24_be = ((unsigned long)data_p[2] << 16) |
	    ((unsigned long)data_p[1] << 8) | data_p[0];
	long s24_be;

#ifndef YS_ASSUME_2S_COMP
	if (u24_be & 0x800000UL) {
		s24_be = (long)(u24_be - 0x800000UL) - 0x800000;
	} else
#endif
		s24_be = u24_be;

	if (data_pp != NULL) {
		*data_pp = data_p + 3;
	}

	return s24_be;
}

unsigned long
ys_read_u32_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 32 bit integer in big-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_be = ((unsigned long)data_p[0] << 24) |
	    ((unsigned long)data_p[1] << 16) |
	    ((unsigned long)data_p[2] << 8) | data_p[3];

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return u32_be;
}

unsigned long
ys_read_u32_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads an unsigned 32 bit integer in little-endian format from the array of 
 * bytes pointed to by 'data_p'. If 'data_pp' is not null, it will set
 * '*data_pp' to point past the integer read. 
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_le = ((unsigned long)data_p[3] << 24) |
	    ((unsigned long)data_p[2] << 16) |
	    ((unsigned long)data_p[1] << 8) | data_p[0];

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return u32_le;
}

long ys_read_s32_be(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 32 bit integer in big-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_be = ((unsigned long)data_p[0] << 24) |
	    ((unsigned long)data_p[1] << 16) |
	    ((unsigned long)data_p[2] << 8) | data_p[3];
	long s32_be;

#ifndef YS_ASSUME_2S_COMP
	if (u32_be & 0x80000000UL) {
		s32_be = (long)(u32_be - 0x80000000UL) - 0x7FFFFFFF - 1;
	} else
#endif
		s32_be = u32_be;

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return s32_be;
}

long ys_read_s32_le(const unsigned char *data_p, const unsigned char **data_pp)
/*---------------------------------------------------------------------------*\
 * Reads a signed 32 bit integer in little-endian, 2's complement format from
 *  the array of bytes pointed to by 'data_p'. 
 * If 'data_pp' is not null, it will set '*data_pp' to point past the integer
 *  read.
\*---------------------------------------------------------------------------*/
{
	unsigned long u32_le = ((unsigned long)data_p[3] << 24) |
	    ((unsigned long)data_p[2] << 16) |
	    ((unsigned long)data_p[1] << 8) | data_p[0];
	long s32_le;

#ifndef YS_ASSUME_2S_COMP
	if (u32_le & 0x80000000UL) {
		s32_le = (long)(u32_le - 0x80000000UL) - 0x7FFFFFFF - 1;
	} else
#endif
		s32_le = u32_le;

	if (data_pp != NULL) {
		*data_pp = data_p + 4;
	}

	return s32_le;
}

} // End of namespace Saga
