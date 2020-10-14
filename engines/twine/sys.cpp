/** @file sys.cpp
	@brief
	This file contains system types definitions

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "sys.h"

// Little endian

FORCEINLINE uint16 READ_LE_UINT16(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[1] << 8) | b[0];
}

FORCEINLINE int16 READ_LE_INT16(const void *ptr) {
	return (int16)READ_LE_UINT16(ptr);
}

FORCEINLINE uint32 READ_LE_UINT32(const void *ptr) {
	const uint8 *b = (const uint8 *)ptr;
	return (b[3] << 24) | (b[2] << 16) | (b[1] << 8) | (b[0]);
}

FORCEINLINE int32 READ_LE_INT32(const void *ptr) {
	return (int32)READ_LE_UINT32(ptr);
}

FORCEINLINE void WRITE_LE_UINT16(void *ptr, uint16 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >> 0);
	b[1] = (uint8)(value >> 8);
}

FORCEINLINE void WRITE_LE_INT16(void *ptr, int16 value) {
	WRITE_LE_UINT16(ptr, (uint16)value);
}

FORCEINLINE void WRITE_LE_UINT32(void *ptr, uint32 value) {
	uint8 *b = (uint8 *)ptr;
	b[0] = (uint8)(value >>  0);
	b[1] = (uint8)(value >>  8);
	b[2] = (uint8)(value >> 16);
	b[3] = (uint8)(value >> 24);
}

FORCEINLINE void WRITE_LE_INT32(void *ptr, int32 value) {
	WRITE_LE_UINT32(ptr, (uint32)value);
}

// TODO: big endian

// Main endian functions

FORCEINLINE uint8 READ_BYTE(const void *ptr) {
	return *((const uint8 *)ptr);
}

FORCEINLINE uint16 READ_UINT16(const void *ptr) {
	return READ_LE_UINT16(ptr);
}

FORCEINLINE int16 READ_INT16(const void *ptr) {
	return READ_LE_INT16(ptr);
}

FORCEINLINE uint32 READ_UINT32(const void *ptr) {
	return READ_LE_UINT32(ptr);
}

FORCEINLINE int32 READ_INT32(const void *ptr) {
	return READ_LE_INT32(ptr);
}

FORCEINLINE void WRITE_UINT16(void *ptr, uint16 value) {
	WRITE_LE_UINT16(ptr, value);
}

FORCEINLINE void WRITE_INT16(void *ptr, int16 value) {
	WRITE_LE_INT16(ptr, value);
}

FORCEINLINE void WRITE_UINT32(void *ptr, uint32 value) {
	WRITE_LE_UINT32(ptr, value);
}

FORCEINLINE void WRITE_INT32(void *ptr, int32 value) {
	WRITE_LE_INT32(ptr, value);
}
