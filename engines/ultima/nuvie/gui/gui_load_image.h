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

#ifndef NUVIE_GUI_GUI_LOAD_IMAGE_H
#define NUVIE_GUI_GUI_LOAD_IMAGE_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Nuvie {

/************************************************************************/
/* C functions for C image loading support:
   Use 'genimage' to turn an 8-bit BMP file into a C image, and include
   the output in your application, either directly or as a header file.
   Create an SDL surface in your program:
    surface = GUI_LoadImage(image_w, image_h, image_pal, image_data);
 */
/************************************************************************/

extern Graphics::ManagedSurface *GUI_LoadImage(int w, int h, uint8 *pal, uint8 *data);

/* Load the internal 8x8 font and return the associated font surface */
extern Graphics::ManagedSurface *GUI_DefaultFont(void);

extern Graphics::ManagedSurface *GUI_Font6x8(void);

extern Graphics::ManagedSurface *GUI_FontGump(void);

extern uint8 *GUI_FontGumpWData(void);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
