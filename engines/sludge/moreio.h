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
#ifndef SLUDGE_MOREIO_H
#define SLUDGE_MOREIO_H

#include <stdint.h>

#if ALLOW_FILE
int get2bytes(FILE *fp);
void put2bytes(int numtoput, FILE *fp);
char *readString(FILE *fp);
void writeString(char *s, FILE *fp);
void putFloat(float f, FILE *fp);
float getFloat(FILE *fp);
void putSigned(short f, FILE *fp);
short getSigned(FILE *fp);
int32_t get4bytes(FILE *fp);
void put4bytes(uint32_t f, FILE *fp);
#endif
char *encodeFilename(char *nameIn);
char *decodeFilename(char *nameIn);

#endif
