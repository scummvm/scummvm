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

#ifndef BCB_TEXT_RENDERER_H
#define BCB_TEXT_RENDERER_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"
#include "common/ustr.h"
#include "graphics/pixelformat.h"

namespace Graphics {
struct Surface;
}

namespace BCB {

void drawHighQualityTextLine(Graphics::Surface &surface, const Common::String &fontName, int size,
	const Common::Array<Common::Point> &shadowOffsets, const Common::U32String &text,
	int x, int y, uint32 textColor, uint32 shadowColor, int strokeWidth);
Graphics::Surface *downscaleRgbaLayerBicubic(const Graphics::Surface &source, int width, int height);
void alphaCompositeRgbaLayer(Graphics::Surface &target, const Graphics::Surface &layer);

} // End of namespace BCB

#endif
