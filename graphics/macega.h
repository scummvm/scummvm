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
 */

#ifndef GRAPHICS_MACEGA_H
#define GRAPHICS_MACEGA_H

namespace Graphics {

// This is the palette that Basilisk II uses by default for 16-color games.
// It's basically the same as EGA, but a little bit brighter. While it does
// seem like games can have their own palette (CLUT) resources, at least Loom
// didn't seem to use it. (I could be wrong about that.)

static const byte macEGAPalette[] = {
	0x00, 0x00, 0x00,	// Black
	0x00, 0x00, 0xBE,	// Blue
	0x00, 0xBE, 0x00,	// Green
	0x00, 0xBE, 0xBE,	// Cyan
	0xBE, 0x00, 0x00,	// Red
	0xBE, 0x00, 0xBE,	// Magenta
	0xBE, 0x75, 0x00,	// Brown
	0xBE, 0xBE, 0xBE,	// Light Gray
	0x75, 0x75, 0x75,	// Dark Gray
	0x75, 0x75, 0xFC,	// Bright Blue
	0x75, 0xFC, 0x75,	// Bright Green
	0x75, 0xFC, 0xFC,	// Bright Cyan
	0xFC, 0x75, 0x75,	// Bright Red
	0xFC, 0x75, 0xFC,	// Bright Magenta
	0xFC, 0xFC, 0x75,	// Bright Yellow
	0xFC, 0xFC, 0xFC	// White
};

// Each color has its own 2x2 dithering pattern. This array remaps them to the
// color indexes for black and white. The order of the pixels is upper left,
// upper right, lower left, lower right. I don't know if this is the standard
// method, but it does seem to be what LucasArts used so maybe it's useful for
// other games as well.
//
// One obvious candidate would be the Mac AGI games, though looking at
// screenshots makes me suspect that they used 3x2 pixels for each color.

const byte macEGADither[16][4] = {
	{  0,  0,  0,  0 },	// Black
	{  0,  0,  0, 15 },	// Blue
	{  0,  0, 15,  0 },	// Green
	{  0,  0, 15, 15 },	// Cyan
	{  0, 15,  0,  0 },	// Red
	{  0, 15,  0, 15 },	// Magenta
	{  0, 15, 15,  0 },	// Brown
	{  0, 15, 15, 15 },	// Light Gray
	{ 15,  0,  0,  0 },	// Dark Gray
	{ 15,  0,  0, 15 },	// Bright Blue
	{ 15,  0, 15,  0 },	// Bright Green
	{ 15,  0, 15, 15 },	// Bright Cyan
	{ 15, 15,  0,  0 },	// Bright Red
	{ 15, 15,  0, 15 },	// Bright Magenta
	{ 15, 15, 15,  0 },	// Bright Yellow
	{ 15, 15, 15, 15 }	// White
};

} // end of namespace Graphics

#endif // GRAPHICS_MACEGA_H
