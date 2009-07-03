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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */
#ifndef GRAPHICS_CONVERSION_H
#define GRAPHICS_CONVERSION_H

#include "common/scummsys.h"
#include "graphics/pixelformat.h"
namespace Graphics {

// TODO: generic YUV to RGB pixel conversion
// TODO: generic YUV to RGB blit

/**
 * Convert a rectangle from the one format to another, and blits it.
 *
 * @param dstbuf	the buffer which will recieve the converted graphics data
 * @param srcbuf	the buffer containing the original graphics data
 * @param dstpitch	width in bytes of one full line of the dest buffer
 * @param srcpitch	width in bytes of one full line of the source buffer
 * @param w			the width of the graphics data
 * @param h			the height of the graphics data
 * @param dstFmt	the desired pixel format
 * @param srcFmt	the original pixel format
 * @return			true if conversion completes successfully, 
 *					false if there is an error.
 *
 * @note This implementation currently requires the destination's 
 *		 format have at least as high a bitdepth as the source's.
 *
 */
bool crossBlit(byte *dst, const byte *src, int dstpitch, int srcpitch, 
						int w, int h, Graphics::PixelFormat dstFmt, Graphics::PixelFormat srcFmt);
} // end of namespace Graphics
#endif //GRAPHICS_CONVERSION_H
