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

#ifndef BACKENDS_GRAPHICS_ATARI_VIDEL_RESOLUTIONS_H
#define BACKENDS_GRAPHICS_ATARI_VIDEL_RESOLUTIONS_H

#include "common/scummsys.h"

constexpr int SCP_SIZE = 158;

extern const byte scp_320x200x8_rgb[SCP_SIZE];
extern const byte scp_320x200x8_rgb60[SCP_SIZE];
extern       byte scp_320x200x8_vga[SCP_SIZE];

extern const byte scp_320x240x8_rgb[SCP_SIZE];
extern       byte scp_320x240x8_vga[SCP_SIZE];
extern const byte scp_320x240x16_rgb[SCP_SIZE];
extern const byte scp_320x240x16_vga[SCP_SIZE];

extern const byte scp_640x400x8_rgb[SCP_SIZE];
extern const byte scp_640x400x8_rgb60[SCP_SIZE];
extern       byte scp_640x400x8_vga[SCP_SIZE];

extern const byte scp_640x480x8_rgb[SCP_SIZE];
extern       byte scp_640x480x8_vga[SCP_SIZE];
extern const byte scp_640x480x16_rgb[SCP_SIZE];
extern const byte scp_640x480x16_vga[SCP_SIZE];

#endif
