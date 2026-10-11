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

#ifndef ULTIMA2_GFX_EGA_PICTURE_H
#define ULTIMA2_GFX_EGA_PICTURE_H

#include "common/str.h"
#include "graphics/managed_surface.h"
#include "ultima/ultima2/data/data.h"

namespace Ultima {
namespace Ultima2 {
namespace Gfx {

/**
 * Loads one of the title/demo pictures as the "Ultima II Upgrade" patch's
 * EGA-family renderers show it, into a 320x200 CLUT8 surface whose pixels
 * index EGA_PALETTE16 at EGA_PALETTE_BASE. The name is the picture's
 * original CGA filename, in lower case ("picdra", "picout", ...).
 */
bool loadEgaPicture(const Common::String &name, Data::RenderMode mode,
	Graphics::ManagedSurface &surf);

} // namespace Gfx
} // namespace Ultima2
} // namespace Ultima

#endif
