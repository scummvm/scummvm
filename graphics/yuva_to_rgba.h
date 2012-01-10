/* Residual - A 3D game interpreter
 *
 * Residual is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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
 */

/**
 * @file
 * YUV to RGB conversion used in engines:
 * - scumm (he)
 * - sword25
 */

#ifndef GRAPHICS_YUVA_TO_RGBA_H
#define GRAPHICS_YUVA_TO_RGBA_H

#include "common/scummsys.h"
#include "graphics/surface.h"

namespace Graphics {

struct Surface;

/**
 * Convert a YUVA420 image to an RGBA surface
 *
 * @param dst     the destination surface
 * @param ySrc    the source of the y component
 * @param uSrc    the source of the u component
 * @param vSrc    the source of the v component
 * @param aSrc    the source of the a component
 * @param yWidth  the width of the y surface (must be divisible by 2)
 * @param yHeight the height of the y surface (must be divisible by 2)
 * @param yPitch  the pitch of the y surface
 * @param uvPitch the pitch of the u and v surfaces
 */
void convertYUVA420ToRGBA(Graphics::Surface *dst, const byte *ySrc, const byte *uSrc, const byte *vSrc, const byte *aSrc, int yWidth, int yHeight, int yPitch, int uvPitch);

} // End of namespace Graphics

#endif
