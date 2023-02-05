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

#ifndef BACKENDS_GRAPHICS_ATARI_SUPERBLITTER_H
#define BACKENDS_GRAPHICS_ATARI_SUPERBLITTER_H

#include <mint/trap14.h>

#define ct60_vm(mode, value) (long)trap_14_wwl((short)0xc60e, (short)(mode), (long)(value))
#define ct60_vmalloc(value) ct60_vm(0, value)
#define ct60_vmfree(value)  ct60_vm(1, value)

// bits 26:0
#define SV_BLITTER_SRC1           ((volatile long*)0x80010058)
#define SV_BLITTER_SRC2           ((volatile long*)0x8001005C)
#define SV_BLITTER_DST            ((volatile long*)0x80010060)
// The amount of bytes that are to be copied in a horizontal line, minus 1
#define SV_BLITTER_COUNT          ((volatile long*)0x80010064)
// The amount of bytes that are to be added to the line start adress after a line has been copied, in order to reach the next one
#define SV_BLITTER_SRC1_OFFSET    ((volatile long*)0x80010068)
#define SV_BLITTER_SRC2_OFFSET    ((volatile long*)0x8001006C)
#define SV_BLITTER_DST_OFFSET     ((volatile long*)0x80010070)
// bits 11:0 - The amount of horizontal lines to do
#define SV_BLITTER_MASK_AND_LINES ((volatile long*)0x80010074)
// bit    0 - busy / start
// bits 4:1 - blit mode
#define SV_BLITTER_CONTROL        ((volatile long*)0x80010078)
// bits 9:0
#define SV_VERSION                ((volatile long*)0x8001007C)
// bit 0 - empty (read only)
// bit 1 - full (read only)
// bits 31:0 - data (write only)
#define SV_BLITTER_FIFO           ((volatile long*)0x80010080)

#endif
