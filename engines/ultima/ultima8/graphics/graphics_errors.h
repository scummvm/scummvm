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

#ifndef ULTIMA8_GRAPHICS_GRAPHICSERRORS_H
#define ULTIMA8_GRAPHICS_GRAPHICSERRORS_H


////////////////////////////////////
//                                //
// Graphics Subsystem Error Codes //
//                                //
////////////////////////////////////

#include "ultima/ultima8/misc/errors.h"

namespace Ultima {
namespace Ultima8 {

//
// Graphics Error Code Bases
//

// Software Rendering Error Code Base
#define GR_SOFT_ERROR_BASE                      (GRAPHICS_ERROR_BASE-0x1000)

// OpenGL Rendering Error Code Base
#define GR_OGL_ERROR_BASE                       (GRAPHICS_ERROR_BASE-0x2000)

// Texture Error Code Base
#define GR_TEX_ERROR_BASE                       (GRAPHICS_ERROR_BASE-0x3000)


//
// Generic Graphics Errors
//


//
// SoftRenderSurface Error Codes
//

// Surface Locked with NULL SoftRenderSurface::pixels pointer
#define GR_SOFT_ERROR_LOCKED_NULL_PIXELS        (GR_SOFT_ERROR_BASE-1)

// BeginPainting()/EndPainting() Mismatch
#define GR_SOFT_ERROR_BEGIN_END_MISMATCH        (GR_SOFT_ERROR_BASE-3)


//
// OpenGL Error Codes
//



//
// Texturing Error Codes
//

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
