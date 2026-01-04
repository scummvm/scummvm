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

#include "graphics/hotspot_renderer.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include <math.h>

namespace Graphics {

HotspotRenderer::HotspotRenderer() {
}

HotspotRenderer::~HotspotRenderer() {
}

void HotspotRenderer::render(Surface *surface,
							 const Common::Array<HotspotInfo> &hotspots,
							 int gameWidth, int gameHeight,
							 int overlayWidth, int overlayHeight,
							 const PixelFormat &format,
							 MarkerShape markerShape,
							 bool showText) {
	if (!surface || hotspots.empty())
		return;

	const Font *font = nullptr;
	if (showText) {
		font = FontMan.getFontByUsage(FontManager::kGUIFont);
	}

	float scaleX = (float)overlayWidth / (float)gameWidth;
	float scaleY = (float)overlayHeight / (float)gameHeight;

	for (uint i = 0; i < hotspots.size(); i++) {
		int gameX = hotspots[i].position.x;
		int gameY = hotspots[i].position.y;

		int overlayX = (int)(gameX * scaleX);
		int overlayY = (int)(gameY * scaleY);

		drawMarker(surface, overlayX, overlayY, format, markerShape);

		if (font && !hotspots[i].name.empty()) {
			drawHotspotLabel(surface, overlayX, overlayY, hotspots[i].name,
							 overlayWidth, overlayHeight, format, font);
		}
	}
}

void HotspotRenderer::drawMarker(Surface *surface, int x, int y, const PixelFormat &format, MarkerShape markerShape) {
	int width = surface->w;
	int height = surface->h;

	switch (markerShape) {
	case MARKER_CROSSHAIR:
		drawCrosshairMarker(surface, x, y, width, height, format);
		break;
	case MARKER_SQUARE:
		drawSquareMarker(surface, x, y, width, height, format);
		break;
	case MARKER_POINT:
		drawPointMarker(surface, x, y, width, height, format);
		break;
	}
}

void HotspotRenderer::drawHotspotLabel(Surface *surface, int overlayX, int overlayY, const Common::String &label,
									   int overlayWidth, int overlayHeight, const PixelFormat &format, const Font *font) {
	int textX = overlayX + kMarkerSize / 2 + 8;
	int textY = overlayY - font->getFontHeight() / 2;

	int textWidth = font->getStringWidth(label);
	int textHeight = font->getFontHeight();

	int bgX = textX - 4;
	int bgY = textY - 2;
	int bgW = textWidth + 8;
	int bgH = textHeight + 4;

	drawLabelBox(surface, bgX, bgY, bgW, bgH,
				 overlayWidth, overlayHeight, format);

	uint32 textColor = format.RGBToColor(255, 255, 255);
	font->drawString(surface, label, textX, textY,
					 overlayWidth - textX, textColor, kTextAlignLeft);
}

void HotspotRenderer::drawCrosshairMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int lineLength = (kMarkerSize / 2) - 1;

	drawLineWithGlow(surface, x - lineLength, y, x + lineLength, y, width, height, format, kLineThickness);
	drawLineWithGlow(surface, x, y - lineLength, x, y + lineLength, width, height, format, kLineThickness);
}

void HotspotRenderer::drawSquareMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	int size = kMarkerSize;
	int rectX = x - kMarkerSize / 2;
	int rectY = y - kMarkerSize / 2;

	drawRectWithGlow(surface, rectX, rectY, size, size, width, height, format);
}

void HotspotRenderer::drawPointMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	int maxRadius = kPointRadius + kGlowSize;

	for (int dy = -maxRadius; dy <= maxRadius; dy++) {
		for (int dx = -maxRadius; dx <= maxRadius; dx++) {
			int px = x + dx;
			int py = y + dy;

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distSq = dx * dx + dy * dy;
			int dist = (int)(sqrtf((float)distSq) + 0.5f);

			blendPixelWithGlow(surface, px, py, format, dist, kPointRadius);
		}
	}
}

void HotspotRenderer::blendPixelWithGlow(Surface *surface, int px, int py, const PixelFormat &format,
										 int distance, int solidSize) {
	uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
	byte bgR, bgG, bgB, bgA;
	format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

	// Convert solidSize to maximum solid distance (solidSize=1 means center pixel only, distance=0)
	int maxSolidDistance = solidSize - 1;

	if (distance <= maxSolidDistance) {
		const int solidBeta = 180;
		const int solidAlpha = 255 - solidBeta;
		byte r = ((255 * solidBeta) + (bgR * solidAlpha)) / 255;
		byte g = ((255 * solidBeta) + (bgG * solidAlpha)) / 255;
		byte b = ((255 * solidBeta) + (bgB * solidAlpha)) / 255;
		*destPixel = format.RGBToColor(r, g, b);
	} else if (distance <= maxSolidDistance + kGlowSize) {
		int glowDist = distance - maxSolidDistance;
		int alpha = ((kGlowSize - glowDist) * 80) / kGlowSize;
		byte r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
		byte g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
		byte b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
		*destPixel = format.RGBToColor(r, g, b);
	}
}

void HotspotRenderer::drawLineWithGlow(Surface *surface, int x1, int y1, int x2, int y2,
									   int width, int height, const PixelFormat &format,
									   int lineThickness) {
	if (format.bytesPerPixel != 2)
		return;

	bool isHorizontal = (y1 == y2);
	bool isVertical = (x1 == x2);

	if (!isHorizontal && !isVertical)
		return;

	int alongMin, alongMax, acrossFixed;

	if (isHorizontal) {
		alongMin = MIN(x1, x2);
		alongMax = MAX(x1, x2);
		acrossFixed = y1;
	} else {
		alongMin = MIN(y1, y2);
		alongMax = MAX(y1, y2);
		acrossFixed = x1;
	}

	for (int along = alongMin; along <= alongMax; along++) {
		for (int thickness = -lineThickness - kGlowSize; thickness <= lineThickness + kGlowSize; thickness++) {
			int px, py;
			if (isHorizontal) {
				px = along;
				py = acrossFixed + thickness;
			} else {
				px = acrossFixed + thickness;
				py = along;
			}

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distFromCenter = ABS(thickness);
			blendPixelWithGlow(surface, px, py, format, distFromCenter, lineThickness);
		}
	}
}

void HotspotRenderer::drawRectWithGlow(Surface *surface, int x, int y, int w, int h,
									   int overlayWidth, int overlayHeight, const PixelFormat &format) {
	drawLineWithGlow(surface, x, y, x + w - 1, y, overlayWidth, overlayHeight, format, kLineThickness);
	drawLineWithGlow(surface, x, y + h - 1, x + w - 1, y + h - 1, overlayWidth, overlayHeight, format, kLineThickness);
	drawLineWithGlow(surface, x, y + 1, x, y + h - 2, overlayWidth, overlayHeight, format, kLineThickness);
	drawLineWithGlow(surface, x + w - 1, y + 1, x + w - 1, y + h - 2, overlayWidth, overlayHeight, format, kLineThickness);
}

void HotspotRenderer::drawLabelBox(Surface *surface, int x, int y, int w, int h,
									int overlayWidth, int overlayHeight, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	for (int py = y + 1; py < y + h - 1; py++) {
		for (int px = x + 1; px < x + w - 1; px++) {
			if (px < 0 || px >= overlayWidth || py < 0 || py >= overlayHeight)
				continue;

			uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
			byte bgR, bgG, bgB, bgA;
			format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

			byte r = (bgR * 40) / 100;
			byte g = (bgG * 40) / 100;
			byte b = (bgB * 40) / 100;
			*destPixel = format.RGBToColor(r, g, b);
		}
	}

	drawRectWithGlow(surface, x, y, w, h, overlayWidth, overlayHeight, format);
}

} // End of namespace Graphics
