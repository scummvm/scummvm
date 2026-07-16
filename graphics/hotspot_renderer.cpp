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

namespace Graphics {

HotspotRenderer::HotspotRenderer() :
		_sizeScale(1.0f), _markerSize(kBaseMarkerSize),
		_pointRadius(kBasePointRadius), _lineThickness(kBaseLineThickness) {
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

	if (format.bytesPerPixel < 2)
		return;

	float scaleX = (float)overlayWidth / (float)gameWidth;
	float scaleY = (float)overlayHeight / (float)gameHeight;

	_sizeScale = scaleX;
	_markerSize = MAX(2, (int)(kBaseMarkerSize * _sizeScale));
	_pointRadius = MAX(1, (int)(kBasePointRadius * _sizeScale));
	_lineThickness = MAX(1, (int)(kBaseLineThickness * _sizeScale));

	const Font *font = nullptr;
	if (showText) {
		font = FontMan.getFontByUsage(FontManager::kGUIFont);
	}

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
	case kMarkerCrosshair:
		drawCrosshairMarker(surface, x, y, width, height, format);
		break;
	case kMarkerSquare:
		drawSquareMarker(surface, x, y, width, height, format);
		break;
	case kMarkerPoint:
		drawPointMarker(surface, x, y, width, height, format);
		break;
	}
}

void HotspotRenderer::drawHotspotLabel(Surface *surface, int overlayX, int overlayY,
		const Common::U32String &label, int overlayWidth, int overlayHeight,
		const PixelFormat &format, const Font *font) {
	int baseTextWidth = font->getStringWidth(label);
	int baseTextHeight = font->getFontHeight();
	if (baseTextWidth <= 0 || baseTextHeight <= 0)
		return;

	int textWidth = MAX(1, (int)(baseTextWidth * _sizeScale));
	int textHeight = MAX(1, (int)(baseTextHeight * _sizeScale));

	int gap = MAX(1, (int)(8 * _sizeScale));
	int textX = overlayX + _markerSize / 2 + gap;
	int textY = overlayY - textHeight / 2;

	int padX = MAX(1, (int)(4 * _sizeScale));
	int padY = MAX(1, (int)(2 * _sizeScale));
	int bgX = textX - padX;
	int bgY = textY - padY;
	int bgW = textWidth + padX * 2;
	int bgH = textHeight + padY * 2;

	drawLabelBox(surface, bgX, bgY, bgW, bgH,
		overlayWidth, overlayHeight, format);

	Surface textSurface;
	textSurface.create(baseTextWidth, baseTextHeight, format);
	textSurface.fillRect(Common::Rect(0, 0, baseTextWidth, baseTextHeight), 0);

	uint32 textColor = format.RGBToColor(255, 255, 255);
	font->drawString(&textSurface, label, 0, 0, baseTextWidth, textColor, kTextAlignLeft);

	Surface *scaledText = textSurface.scale((int16)textWidth, (int16)textHeight, false);
	textSurface.free();
	if (!scaledText)
		return;

	for (int sy = 0; sy < textHeight; sy++) {
		int py = textY + sy;
		if (py < 0 || py >= overlayHeight)
			continue;
		for (int sx = 0; sx < textWidth; sx++) {
			int px = textX + sx;
			if (px < 0 || px >= overlayWidth)
				continue;
			uint32 pixel = scaledText->getPixel(sx, sy);
			if (pixel == 0)
				continue;
			surface->setPixel(px, py, pixel);
		}
	}

	scaledText->free();
	delete scaledText;
}

void HotspotRenderer::drawCrosshairMarker(Surface *surface, int x, int y,
		int width, int height, const PixelFormat &format) {
	const int lineLength = (_markerSize / 2) - 1;

	drawLine(surface, x - lineLength, y, x + lineLength, y, width, height, format, _lineThickness);
	drawLine(surface, x, y - lineLength, x, y + lineLength, width, height, format, _lineThickness);
}

void HotspotRenderer::drawSquareMarker(Surface *surface, int x, int y,
		int width, int height, const PixelFormat &format) {
	int size = _markerSize;
	int rectX = x - _markerSize / 2;
	int rectY = y - _markerSize / 2;

	drawRect(surface, rectX, rectY, size, size, width, height, format);
}

void HotspotRenderer::drawPointMarker(Surface *surface, int x, int y,
		int width, int height, const PixelFormat &format) {
	int maxRadius = _pointRadius;

	for (int dy = -maxRadius; dy <= maxRadius; dy++) {
		for (int dx = -maxRadius; dx <= maxRadius; dx++) {
			int px = x + dx;
			int py = y + dy;

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distSq = dx * dx + dy * dy;
			int dist = (int)(sqrtf((float)distSq) + 0.5f);

			blendPixel(surface, px, py, format, dist, _pointRadius);
		}
	}
}

void HotspotRenderer::blendPixel(Surface *surface, int px, int py,
		const PixelFormat &format, int distance, int solidSize) {
	// Convert solidSize to maximum solid distance (solidSize=1 means center pixel only, distance=0)
	int maxSolidDistance = solidSize - 1;

	if (distance > maxSolidDistance)
		return;

	byte bgR, bgG, bgB, bgA;
	format.colorToARGB(surface->getPixel(px, py), bgA, bgR, bgG, bgB);

	const int solidBeta = 180;
	const int solidAlpha = 255 - solidBeta;
	byte r = ((255 * solidBeta) + (bgR * solidAlpha)) / 255;
	byte g = ((255 * solidBeta) + (bgG * solidAlpha)) / 255;
	byte b = ((255 * solidBeta) + (bgB * solidAlpha)) / 255;
	surface->setPixel(px, py, format.RGBToColor(r, g, b));
}

void HotspotRenderer::drawLine(Surface *surface, int x1, int y1, int x2, int y2,
		int width, int height, const PixelFormat &format, int lineThickness) {
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
		for (int thickness = -lineThickness; thickness <= lineThickness; thickness++) {
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
			blendPixel(surface, px, py, format, distFromCenter, lineThickness);
		}
	}
}

void HotspotRenderer::drawRect(Surface *surface, int x, int y, int w, int h,
		int overlayWidth, int overlayHeight, const PixelFormat &format) {
	drawLine(surface, x, y, x + w - 1, y, overlayWidth, overlayHeight, format, _lineThickness);
	drawLine(surface, x, y + h - 1, x + w - 1, y + h - 1,
			overlayWidth, overlayHeight, format, _lineThickness);
	drawLine(surface, x, y + 1, x, y + h - 2, overlayWidth, overlayHeight, format, _lineThickness);
	drawLine(surface, x + w - 1, y + 1, x + w - 1, y + h - 2,
			overlayWidth, overlayHeight, format, _lineThickness);
}

void HotspotRenderer::drawLabelBox(Surface *surface, int x, int y, int w, int h,
		int overlayWidth, int overlayHeight, const PixelFormat &format) {
	for (int py = y + 1; py < y + h - 1; py++) {
		for (int px = x + 1; px < x + w - 1; px++) {
			if (px < 0 || px >= overlayWidth || py < 0 || py >= overlayHeight)
				continue;

			byte bgR, bgG, bgB, bgA;
			format.colorToARGB(surface->getPixel(px, py), bgA, bgR, bgG, bgB);

			byte r = (bgR * 40) / 100;
			byte g = (bgG * 40) / 100;
			byte b = (bgB * 40) / 100;
			surface->setPixel(px, py, format.RGBToColor(r, g, b));
		}
	}

	drawRect(surface, x, y, w, h, overlayWidth, overlayHeight, format);
}

} // End of namespace Graphics
