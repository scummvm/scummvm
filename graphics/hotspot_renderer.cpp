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
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include <math.h>

namespace Graphics {

HotspotRenderer::HotspotRenderer()
	: _radius(10), _glowIntensity(150), _markerShape(MARKER_CIRCLE) {
}

HotspotRenderer::~HotspotRenderer() {
}

void HotspotRenderer::render(Surface *surface,
							 const Common::Array<HotspotInfo> &hotspots,
							 int gameWidth, int gameHeight,
							 int overlayWidth, int overlayHeight,
							 const PixelFormat &format,
							 const Font *font) {
	if (!surface || hotspots.empty())
		return;

	float scaleX = (float)overlayWidth / (float)gameWidth;
	float scaleY = (float)overlayHeight / (float)gameHeight;

	for (uint i = 0; i < hotspots.size(); i++) {
		int gameX = hotspots[i].position.x;
		int gameY = hotspots[i].position.y;

		int overlayX = (int)(gameX * scaleX);
		int overlayY = (int)(gameY * scaleY);

		drawMarker(surface, overlayX, overlayY, format);

		if (font && !hotspots[i].name.empty()) {
			int textX = overlayX + _radius / 2 + 8;
			int textY = overlayY - font->getFontHeight() / 2;

			int textWidth = font->getStringWidth(hotspots[i].name);
			int textHeight = font->getFontHeight();

			int bgX = textX - 4;
			int bgY = textY - 2;
			int bgW = textWidth + 8;
			int bgH = textHeight + 4;

			drawRoundedRectWithGlow(surface, bgX, bgY, bgW, bgH,
									overlayWidth, overlayHeight, format);

			uint32 textColor = format.RGBToColor(255, 255, 255);
			font->drawString(surface, hotspots[i].name, textX, textY,
							 overlayWidth - textX, textColor, kTextAlignLeft);
		}
	}
}

void HotspotRenderer::drawMarker(Surface *surface, int x, int y, const PixelFormat &format) {
	int width = surface->w;
	int height = surface->h;

	switch (_markerShape) {
	case MARKER_CIRCLE:
		drawCircleMarker(surface, x, y, width, height, format);
		break;
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

void HotspotRenderer::drawCircleMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int borderWidth = 0;
	const int glowSize = 3;
	const int radius = _radius / 2;
	int maxRadius = radius + glowSize;

	for (int dy = -maxRadius; dy <= maxRadius; dy++) {
		for (int dx = -maxRadius; dx <= maxRadius; dx++) {
			int px = x + dx;
			int py = y + dy;

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distSq = dx * dx + dy * dy;
			int dist = (int)(sqrtf((float)distSq) + 0.5f);

			int ringInner = radius - borderWidth;
			int ringOuter = radius;
			int glowOuter = radius + glowSize;

			byte r, g, b;
			bool shouldDraw = false;

			if (dist >= ringInner && dist <= ringOuter) {
				uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
				byte bgR, bgG, bgB, bgA;
				format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

				r = ((255 * 180) + (bgR * 75)) / 255;
				g = ((255 * 180) + (bgG * 75)) / 255;
				b = ((255 * 180) + (bgB * 75)) / 255;
				shouldDraw = true;
			} else if (dist > ringOuter && dist <= glowOuter) {
				uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
				byte bgR, bgG, bgB, bgA;
				format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

				int glowDist = dist - ringOuter;
				int alpha = ((glowSize - glowDist) * 80) / glowSize;

				r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
				g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
				b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
				shouldDraw = true;
			}

			if (shouldDraw) {
				uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
				*destPixel = format.RGBToColor(r, g, b);
			}
		}
	}
}

void HotspotRenderer::drawCrosshairMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int lineLength = 2;
	const int borderWidth = 0;
	const int glowSize = 3;

	for (int i = -lineLength - glowSize; i <= lineLength + glowSize; i++) {
		for (int thickness = -borderWidth - glowSize; thickness <= borderWidth + glowSize; thickness++) {
			int hx = x + i;
			int hy = y + thickness;
			int vx = x + thickness;
			int vy = y + i;

			bool drawHorizontal = (hx >= 0 && hx < width && hy >= 0 && hy < height);
			bool drawVertical = (vx >= 0 && vx < width && vy >= 0 && vy < height);

			if (drawHorizontal || drawVertical) {
				int distFromCenter = ABS(thickness);
				byte r, g, b;

				if (distFromCenter <= borderWidth) {
					r = 255;
					g = 255;
					b = 255;

					if (drawHorizontal) {
						uint16 *destPixel = (uint16 *)surface->getBasePtr(hx, hy);
						byte bgR, bgG, bgB, bgA;
						format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);
						r = ((255 * 180) + (bgR * 75)) / 255;
						g = ((255 * 180) + (bgG * 75)) / 255;
						b = ((255 * 180) + (bgB * 75)) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
					if (drawVertical) {
						uint16 *destPixel = (uint16 *)surface->getBasePtr(vx, vy);
						byte bgR, bgG, bgB, bgA;
						format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);
						r = ((255 * 180) + (bgR * 75)) / 255;
						g = ((255 * 180) + (bgG * 75)) / 255;
						b = ((255 * 180) + (bgB * 75)) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
				} else if (distFromCenter > borderWidth && distFromCenter <= borderWidth + glowSize) {
					int glowDist = distFromCenter - borderWidth;
					int alpha = ((glowSize - glowDist) * 80) / glowSize;

					if (drawHorizontal) {
						uint16 *destPixel = (uint16 *)surface->getBasePtr(hx, hy);
						byte bgR, bgG, bgB, bgA;
						format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);
						r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
						g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
						b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
					if (drawVertical) {
						uint16 *destPixel = (uint16 *)surface->getBasePtr(vx, vy);
						byte bgR, bgG, bgB, bgA;
						format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);
						r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
						g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
						b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
				}
			}
		}
	}
}

void HotspotRenderer::drawSquareMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int cornerRadius = 4;
	const int glowSize = 3;
	const int borderWidth = 1;

	int size = _radius;
	int rectX = x - _radius / 2;
	int rectY = y - _radius / 2;

	for (int py = rectY - glowSize; py < rectY + size + glowSize; py++) {
		for (int px = rectX - glowSize; px < rectX + size + glowSize; px++) {
			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int dx = 0, dy = 0;

			if (px < rectX + cornerRadius && py < rectY + cornerRadius) {
				dx = (rectX + cornerRadius) - px;
				dy = (rectY + cornerRadius) - py;
			} else if (px >= rectX + size - cornerRadius && py < rectY + cornerRadius) {
				dx = px - (rectX + size - cornerRadius - 1);
				dy = (rectY + cornerRadius) - py;
			} else if (px < rectX + cornerRadius && py >= rectY + size - cornerRadius) {
				dx = (rectX + cornerRadius) - px;
				dy = py - (rectY + size - cornerRadius - 1);
			} else if (px >= rectX + size - cornerRadius && py >= rectY + size - cornerRadius) {
				dx = px - (rectX + size - cornerRadius - 1);
				dy = py - (rectY + size - cornerRadius - 1);
			}

			int distSq = dx * dx + dy * dy;
			int cornerRadiusSq = cornerRadius * cornerRadius;

			bool insideCorner = (dx != 0 || dy != 0) && distSq <= cornerRadiusSq;
			bool insideRect = (px >= rectX && px < rectX + size && py >= rectY && py < rectY + size);
			bool inMainArea = insideRect || insideCorner;

			int innerCornerRadiusSq = (cornerRadius - borderWidth) * (cornerRadius - borderWidth);
			bool insideInnerCorner = (dx != 0 || dy != 0) && distSq <= innerCornerRadiusSq;
			bool insideInnerRect = (px >= rectX + borderWidth && px < rectX + size - borderWidth &&
									py >= rectY + borderWidth && py < rectY + size - borderWidth);
			bool inFillArea = insideInnerRect || insideInnerCorner;

			if ((dx == 0 && dy == 0) || distSq <= (cornerRadius + glowSize) * (cornerRadius + glowSize)) {
				uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
				byte bgR, bgG, bgB, bgA;
				format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

				if (inMainArea && !inFillArea) {
					byte r = ((255 * 180) + (bgR * 75)) / 255;
					byte g = ((255 * 180) + (bgG * 75)) / 255;
					byte b = ((255 * 180) + (bgB * 75)) / 255;
					*destPixel = format.RGBToColor(r, g, b);
				} else if (!inMainArea) {
					int glowDist = 0;
					if (dx != 0 || dy != 0) {
						glowDist = (int)(sqrtf((float)distSq) + 0.5f) - cornerRadius;
					} else {
						if (px < rectX)
							glowDist = rectX - px;
						else if (px >= rectX + size)
							glowDist = px - (rectX + size - 1);
						else if (py < rectY)
							glowDist = rectY - py;
						else if (py >= rectY + size)
							glowDist = py - (rectY + size - 1);
					}

					if (glowDist > 0 && glowDist <= glowSize) {
						int alpha = ((glowSize - glowDist) * 80) / glowSize;
						byte r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
						byte g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
						byte b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
				}
			}
		}
	}
}

void HotspotRenderer::drawPointMarker(Surface *surface, int x, int y, int width, int height, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int pointRadius = 2;
	const int glowSize = 3;
	int maxRadius = pointRadius + glowSize;

	for (int dy = -maxRadius; dy <= maxRadius; dy++) {
		for (int dx = -maxRadius; dx <= maxRadius; dx++) {
			int px = x + dx;
			int py = y + dy;

			if (px < 0 || px >= width || py < 0 || py >= height)
				continue;

			int distSq = dx * dx + dy * dy;
			int dist = (int)(sqrtf((float)distSq) + 0.5f);

			uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
			byte bgR, bgG, bgB, bgA;
			format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

			if (dist <= pointRadius) {
				byte r = ((255 * 200) + (bgR * 55)) / 255;
				byte g = ((255 * 200) + (bgG * 55)) / 255;
				byte b = ((255 * 200) + (bgB * 55)) / 255;
				*destPixel = format.RGBToColor(r, g, b);
			} else if (dist <= pointRadius + glowSize) {
				int glowDist = dist - pointRadius;
				int alpha = ((glowSize - glowDist) * 80) / glowSize;
				byte r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
				byte g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
				byte b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
				*destPixel = format.RGBToColor(r, g, b);
			}
		}
	}
}

void HotspotRenderer::drawRoundedRectWithGlow(Surface *surface, int x, int y, int w, int h,
											  int overlayWidth, int overlayHeight, const PixelFormat &format) {
	if (format.bytesPerPixel != 2)
		return;

	const int cornerRadius = 4;
	const int glowSize = 3;
	const int borderWidth = 1;

	for (int py = y - glowSize; py < y + h + glowSize; py++) {
		for (int px = x - glowSize; px < x + w + glowSize; px++) {
			if (px < 0 || px >= overlayWidth || py < 0 || py >= overlayHeight)
				continue;

			int dx = 0, dy = 0;

			if (px < x + cornerRadius && py < y + cornerRadius) {
				dx = (x + cornerRadius) - px;
				dy = (y + cornerRadius) - py;
			} else if (px >= x + w - cornerRadius && py < y + cornerRadius) {
				dx = px - (x + w - cornerRadius - 1);
				dy = (y + cornerRadius) - py;
			} else if (px < x + cornerRadius && py >= y + h - cornerRadius) {
				dx = (x + cornerRadius) - px;
				dy = py - (y + h - cornerRadius - 1);
			} else if (px >= x + w - cornerRadius && py >= y + h - cornerRadius) {
				dx = px - (x + w - cornerRadius - 1);
				dy = py - (y + h - cornerRadius - 1);
			}

			int distSq = dx * dx + dy * dy;
			int cornerRadiusSq = cornerRadius * cornerRadius;

			bool insideCorner = (dx != 0 || dy != 0) && distSq <= cornerRadiusSq;
			bool insideRect = (px >= x && px < x + w && py >= y && py < y + h);
			bool inMainArea = insideRect || insideCorner;

			int innerCornerRadiusSq = (cornerRadius - borderWidth) * (cornerRadius - borderWidth);
			bool insideInnerCorner = (dx != 0 || dy != 0) && distSq <= innerCornerRadiusSq;
			bool insideInnerRect = (px >= x + borderWidth && px < x + w - borderWidth &&
									py >= y + borderWidth && py < y + h - borderWidth);
			bool inFillArea = insideInnerRect || insideInnerCorner;

			if ((dx == 0 && dy == 0) || distSq <= (cornerRadius + glowSize) * (cornerRadius + glowSize)) {
				uint16 *destPixel = (uint16 *)surface->getBasePtr(px, py);
				byte bgR, bgG, bgB, bgA;
				format.colorToARGB(*destPixel, bgA, bgR, bgG, bgB);

				if (inMainArea) {
					byte r, g, b;

					if (inFillArea) {
						r = (bgR * 40) / 100;
						g = (bgG * 40) / 100;
						b = (bgB * 40) / 100;
					} else {
						r = ((255 * 180) + (bgR * 75)) / 255;
						g = ((255 * 180) + (bgG * 75)) / 255;
						b = ((255 * 180) + (bgB * 75)) / 255;
					}
					*destPixel = format.RGBToColor(r, g, b);
				} else {
					int glowDist = 0;
					if (dx != 0 || dy != 0) {
						glowDist = (int)(sqrtf((float)distSq) + 0.5f) - cornerRadius;
					} else {
						if (px < x)
							glowDist = x - px;
						else if (px >= x + w)
							glowDist = px - (x + w - 1);
						else if (py < y)
							glowDist = y - py;
						else if (py >= y + h)
							glowDist = py - (y + h - 1);
					}

					if (glowDist > 0 && glowDist <= glowSize) {
						int alpha = ((glowSize - glowDist) * 80) / glowSize;
						byte r = ((255 * alpha) + (bgR * (255 - alpha))) / 255;
						byte g = ((255 * alpha) + (bgG * (255 - alpha))) / 255;
						byte b = ((255 * alpha) + (bgB * (255 - alpha))) / 255;
						*destPixel = format.RGBToColor(r, g, b);
					}
				}
			}
		}
	}
}

} // End of namespace Graphics
