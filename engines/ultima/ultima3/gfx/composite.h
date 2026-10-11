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

#ifndef ULTIMA3_GFX_COMPOSITE_H
#define ULTIMA3_GFX_COMPOSITE_H

#include "graphics/managed_surface.h"

namespace Ultima {
namespace Ultima3 {
namespace Gfx {

/**
 * Decodes CGA-indexed pixels (0-3) of a surface, in place, into the
 * composite monitor's artifact colors: pixel values in the
 * COMPOSITE_PALETTE_BASE range
 */
void compositeDecode(Graphics::ManagedSurface &surf);

/**
 * Called by views once they've finished drawing: decodes the surface if the
 * composite renderer is the active one, and does nothing otherwise
 */
void compositeFinish(Graphics::ManagedSurface &surf);

} // namespace Gfx
} // namespace Ultima3
} // namespace Ultima

#endif
