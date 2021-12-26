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

#ifndef _BLITTERS_H_
#define _BLITTERS_H_

extern "C" {

void ITCM_CODE Rescale_320x256xPAL8_To_256x256x1555(u16 *dest, const u8 *src, int destStride, int srcStride, const u16 *palette, u32 numLines);
void ITCM_CODE Rescale_320x256x1555_To_256x256x1555(u16 *dest, const u16 *src, int destStride, int srcStride);

}

#endif
