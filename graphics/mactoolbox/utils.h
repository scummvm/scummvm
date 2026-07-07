
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

#ifndef GRAPHICS_MACTOOLBOX_UTILS_H
#define GRAPHICS_MACTOOLBOX_UTILS_H

#include "graphics/managed_surface.h"

#include "graphics/mactoolbox/toolbox.h"

namespace Graphics {
namespace MacToolbox {

Common::Rect readRect(Common::SeekableReadStream &stream);
Common::Point readPoint(Common::SeekableReadStream &stream);
PolyHandle readPolygon(Common::SeekableReadStream &stream);
Region readRegion(Common::SeekableReadStream &stream);
Pattern readPattern(Common::SeekableReadStream &stream);
PixMap readPixMap(Common::SeekableReadStream &stream, bool hasBaseAddr);
BitMap readBitsRectMono(Common::SeekableReadStream &stream, PixMap &pixMap, bool compressed);

Common::Rect blitMono(const BitMap &src, BitMap &dst, const BitMap &mask, const Common::Point &dstPos, SourceMode mode, uint32 black, uint32 white);
Common::Rect blitMono(const BitMap &src, BitMap &dst, const BitMap &mask, const Common::Point &dstPos, PatternMode mode, uint32 black, uint32 white);

Graphics::ManagedSurface *createRemappedSurface(Graphics::MacWindowManager *wm, const Graphics::Surface *surface, const byte *palette, uint colorCount);

} // End of namespace MacToolbox
} // End of namespace Graphics

#endif // GRAPHICS_MACTOOLBOX_UTILS_H
