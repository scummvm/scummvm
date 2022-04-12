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

#ifndef SCALER_SCALE3X_H
#define SCALER_SCALE3X_H

#if defined(_MSC_VER)
#define __restrict__
#endif

typedef unsigned char scale3x_uint8;
typedef unsigned short scale3x_uint16;
typedef unsigned scale3x_uint32;

void scale3x_8_def(scale3x_uint8* dst0, scale3x_uint8* dst1, scale3x_uint8* dst2, const scale3x_uint8* src0, const scale3x_uint8* src1, const scale3x_uint8* src2, unsigned count);
void scale3x_16_def(scale3x_uint16* dst0, scale3x_uint16* dst1, scale3x_uint16* dst2, const scale3x_uint16* src0, const scale3x_uint16* src1, const scale3x_uint16* src2, unsigned count);
void scale3x_32_def(scale3x_uint32* dst0, scale3x_uint32* dst1, scale3x_uint32* dst2, const scale3x_uint32* src0, const scale3x_uint32* src1, const scale3x_uint32* src2, unsigned count);

#endif
