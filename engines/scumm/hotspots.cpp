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

#include "scumm/hotspots.h"
#include "common/util.h"
#include "graphics/surface.h"

namespace Scumm {
using Graphics::Surface;
using Graphics::PixelFormat;

ScummHotspotRenderer::ScummHotspotRenderer() {
	Graphics::HotspotStyle style;
	style.markerSize = 11;
	style.pointRadius = 6;
	style.lineThickness = 2;
	style.labelGap = 4;
	style.paddingX = 3;
	style.labelBorder = false;
	setStyle(style);
}

void ScummHotspotRenderer::drawMarker(Surface *surface, const Graphics::HotspotInfo &hotspot, int x, int y, const PixelFormat &format, Graphics::MarkerShape shape) {
	if (shape != Graphics::kMarkerDefault) {
		HotspotRenderer::drawMarker(surface, hotspot, x, y, format, shape);
		return;
	}
	addDrawRect(Common::Rect(x - _markerSize / 2, y - _markerSize / 2, x + _markerSize / 2 + 1, y + _markerSize / 2 + 1));
	const int width = surface->w, height = surface->h;
	const int radius = _markerSize / 2;
	for (int dy = -radius; dy <= radius; ++dy) {
		for (int dx = -radius; dx <= radius; ++dx) {
			if (ABS(dx - dy) > _lineThickness && ABS(dx + dy) > _lineThickness)
				continue;
			const int px = x + dx, py = y + dy;
			if (px >= 0 && px < width && py >= 0 && py < height)
				blendPixel(surface, px, py, format, 0, 1);
		}
	}
}

} // End of namespace Scumm
