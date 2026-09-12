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

#include "hopkins/base_enhancements.h"

#include "hopkins/base_data.h"
#include "hopkins/base_engine.h"
#include "hopkins/base_types.h"
#include "hopkins/base_enhancements_autoplay.h"

#include "common/config-manager.h"
#include "common/translation.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"

namespace Hopkins {

namespace {

static byte nearestPaletteColor(const byte *palette, int red, int green, int blue) {
	int bestDistance = 0x7fffffff;
	byte bestColor = 0;
	for (int color = 0; color < 256; ++color) {
		const int offset = color * 3;
		const int redDelta = palette[offset] - red;
		const int greenDelta = palette[offset + 1] - green;
		const int blueDelta = palette[offset + 2] - blue;
		const int distance = redDelta * redDelta + greenDelta * greenDelta + blueDelta * blueDelta;
		if (distance < bestDistance) {
			bestDistance = distance;
			bestColor = color;
		}
	}
	return bestColor;
}

static void drawPixel(byte *framebuffer, int x, int y, byte color) {
	if (x >= 0 && x < kBaseFrameWidth && y >= 0 && y < kBaseFrameHeight)
		framebuffer[y * kBaseFrameWidth + x] = color;
}

static void drawLine(byte *framebuffer, int x0, int y0, int x1, int y1, byte color) {
	const int deltaX = ABS(x1 - x0);
	const int stepX = x0 < x1 ? 1 : -1;
	const int deltaY = -ABS(y1 - y0);
	const int stepY = y0 < y1 ? 1 : -1;
	int error = deltaX + deltaY;
	for (;;) {
		drawPixel(framebuffer, x0, y0, color);
		if (x0 == x1 && y0 == y1)
			break;
		const int doubledError = error * 2;
		if (doubledError >= deltaY) {
			error += deltaY;
			x0 += stepX;
		}
		if (doubledError <= deltaX) {
			error += deltaX;
			y0 += stepY;
		}
	}
}

static void fillRect(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	for (int y = top; y <= bottom; ++y)
		for (int x = left; x <= right; ++x)
			drawPixel(framebuffer, x, y, color);
}

static int triangleEdge(int x0, int y0, int x1, int y1, int x, int y) {
	return (x - x0) * (y1 - y0) - (y - y0) * (x1 - x0);
}

static void fillTriangle(byte *framebuffer, int x0, int y0, int x1, int y1,
		int x2, int y2, byte color) {
	const int left = MIN(x0, MIN(x1, x2));
	const int right = MAX(x0, MAX(x1, x2));
	const int top = MIN(y0, MIN(y1, y2));
	const int bottom = MAX(y0, MAX(y1, y2));
	for (int y = top; y <= bottom; ++y) {
		for (int x = left; x <= right; ++x) {
			const int edge0 = triangleEdge(x0, y0, x1, y1, x, y);
			const int edge1 = triangleEdge(x1, y1, x2, y2, x, y);
			const int edge2 = triangleEdge(x2, y2, x0, y0, x, y);
			if ((edge0 >= 0 && edge1 >= 0 && edge2 >= 0) ||
					(edge0 <= 0 && edge1 <= 0 && edge2 <= 0))
				drawPixel(framebuffer, x, y, color);
		}
	}
}

static void drawPlayerArrow(const BaseData &data, byte *framebuffer, int centerX, int centerY,
		int angle, byte fillColor, byte outlineColor) {
	const int32 directionX = data.cosQ16(angle);
	const int32 directionY = data.sinQ16(angle);
	const int tipX = centerX + (int)(((int64)directionX * 8) >> kBaseFixedShift);
	const int tipY = centerY + (int)(((int64)directionY * 8) >> kBaseFixedShift);
	const int tailX = centerX - (int)(((int64)directionX * 5) >> kBaseFixedShift);
	const int tailY = centerY - (int)(((int64)directionY * 5) >> kBaseFixedShift);
	const int wingX = (int)(((int64)-directionY * 5) >> kBaseFixedShift);
	const int wingY = (int)(((int64)directionX * 5) >> kBaseFixedShift);
	const int leftX = tailX + wingX;
	const int leftY = tailY + wingY;
	const int rightX = tailX - wingX;
	const int rightY = tailY - wingY;

	fillTriangle(framebuffer, tipX, tipY, leftX, leftY, rightX, rightY, fillColor);
	drawLine(framebuffer, tipX, tipY, leftX, leftY, outlineColor);
	drawLine(framebuffer, leftX, leftY, rightX, rightY, outlineColor);
	drawLine(framebuffer, rightX, rightY, tipX, tipY, outlineColor);
	drawPixel(framebuffer, centerX, centerY, fillColor);
}

static void drawSmallDigit(byte *framebuffer, int centerX, int centerY, int digit, byte color) {
	static const byte patterns[6][5] = {
		{ 2, 6, 2, 2, 7 }, // 1
		{ 7, 1, 7, 4, 7 }, // 2
		{ 7, 1, 7, 1, 7 }, // 3
		{ 5, 5, 7, 1, 1 }, // 4
		{ 7, 4, 7, 1, 7 }, // 5
		{ 7, 4, 7, 5, 7 }  // 6
	};
	if (digit < 1 || digit > 6)
		return;
	for (int y = 0; y < 5; ++y) {
		for (int x = 0; x < 3; ++x) {
			if (patterns[digit - 1][y] & (1 << (2 - x)))
				drawPixel(framebuffer, centerX - 1 + x, centerY - 2 + y, color);
		}
	}
}

static void drawExitMarker(byte *framebuffer, int centerX, int centerY, byte backgroundColor,
		byte exitColor, byte highlightColor, int destinationNumber = 0) {
	fillRect(framebuffer, centerX - 4, centerY - 4, centerX + 4, centerY + 4, backgroundColor);
	fillRect(framebuffer, centerX - 3, centerY - 3, centerX + 3, centerY + 3, exitColor);
	fillRect(framebuffer, centerX - 2, centerY - 2, centerX + 2, centerY + 2, backgroundColor);
	if (destinationNumber)
		drawSmallDigit(framebuffer, centerX, centerY, destinationNumber, highlightColor);
	else
		drawPixel(framebuffer, centerX, centerY, highlightColor);
}

static void drawGuardMarker(byte *framebuffer, int centerX, int centerY, byte color) {
	fillRect(framebuffer, centerX - 1, centerY - 1, centerX + 1, centerY + 1, color);
}

static void drawOpaqueLegend(const BaseData &data, byte *framebuffer, byte backgroundColor,
		byte textColor, byte playerColor, byte exitColor, byte guardColor, byte doorColor) {
	Graphics::Surface surface;
	surface.init(kBaseFrameWidth, kBaseFrameHeight, kBaseFrameWidth, framebuffer,
			Graphics::PixelFormat::createFormatCLUT8());
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return;

	static const int kSymbolX = 11;
	static const int kTextX = 22;
	static const int kTextWidth = 68;
	static const int kRows[] = { 53, 73, 93, 113 };
	drawPlayerArrow(data, framebuffer, kSymbolX, kRows[0] + 4, 0, playerColor, textColor);
	drawExitMarker(framebuffer, kSymbolX, kRows[1] + 4, backgroundColor, exitColor, textColor);
	drawGuardMarker(framebuffer, kSymbolX, kRows[2] + 4, guardColor);
	drawLine(framebuffer, kSymbolX - 4, kRows[3] + 4, kSymbolX + 4, kRows[3] + 4, doorColor);

	font->drawString(&surface, _("Player"), kTextX, kRows[0], kTextWidth, textColor);
	font->drawString(&surface, _("Exit"), kTextX, kRows[1], kTextWidth, textColor);
	font->drawString(&surface, _("Guard"), kTextX, kRows[2], kTextWidth, textColor);
	font->drawString(&surface, _("Door"), kTextX, kRows[3], kTextWidth, textColor);
	font->drawString(&surface, _("Game paused"), 226, 29, 92, textColor,
			Graphics::kTextAlignCenter);
	for (int index = 0; index < WBASEEnhancementsAutoplay::destinationCount(); ++index) {
		font->drawString(&surface, Common::String::format("%d", index + 1), 229, 51 + index * 15,
				10, exitColor, Graphics::kTextAlignRight);
		font->drawString(&surface, WBASEEnhancementsAutoplay::destinationMapLabel(index),
				243, 51 + index * 15, 75, textColor);
	}
	font->drawString(&surface, _("A: autoplay"), 226, 151, 92, textColor,
			Graphics::kTextAlignCenter);
	font->drawString(&surface, _("M/Esc: close map"), 0, 183, kBaseFrameWidth, textColor,
			Graphics::kTextAlignCenter);
}

struct NavigationMapColors {
	byte background;
	byte wall;
	byte door;
	byte exit;
	byte guard;
	byte player;
	byte highlight;
};

static NavigationMapColors navigationMapColors(const BaseData &data) {
	const byte *palette = data.palette();
	NavigationMapColors colors;
	colors.background = nearestPaletteColor(palette, 0, 0, 0);
	colors.wall = nearestPaletteColor(palette, 190, 190, 190);
	colors.door = nearestPaletteColor(palette, 255, 210, 0);
	colors.exit = nearestPaletteColor(palette, 0, 220, 255);
	colors.guard = nearestPaletteColor(palette, 255, 70, 20);
	colors.player = nearestPaletteColor(palette, 40, 255, 60);
	colors.highlight = nearestPaletteColor(palette, 255, 255, 255);
	return colors;
}

static void drawMapMarkers(const BaseData &data, const BaseEngine &engine, byte *framebuffer,
		int mapScale, int mapLeft, int mapTop, const NavigationMapColors &colors) {
	// BASE.MAP contains exactly the six wall cells that WBASE accepts as
	// transitions when the player faces them and presses Space.
	for (int mapY = 0; mapY < kBaseMapHeight; ++mapY) {
		for (int mapX = 0; mapX < kBaseMapWidth; ++mapX) {
			if ((data.mapCodeXY(mapX, mapY) & 0xff) != kBaseExitBitmap)
				continue;
			const int centerX = mapLeft + mapX * mapScale + mapScale / 2;
			const int centerY = mapTop + mapY * mapScale + mapScale / 2;
			const int destination = WBASEEnhancementsAutoplay::destinationIndexForExitMapPos(baseMapIndex(mapX, mapY));
			drawExitMarker(framebuffer, centerX, centerY, colors.background, colors.exit,
					colors.highlight, destination + 1);
		}
	}

	for (int objectId = 1; objectId <= kBaseMaxObjects; ++objectId) {
		const BaseObject &object = engine.object(objectId);
		if (!object.active || object.mode == kBaseObjectDead)
			continue;
		const int pixelX = mapLeft + (object.x >> kBaseCellShift) * mapScale + mapScale / 2;
		const int pixelY = mapTop + (object.y >> kBaseCellShift) * mapScale + mapScale / 2;
		drawGuardMarker(framebuffer, pixelX, pixelY, colors.guard);
	}

	const int playerX = mapLeft + (engine.playerX() >> kBaseCellShift) * mapScale + mapScale / 2;
	const int playerY = mapTop + (engine.playerY() >> kBaseCellShift) * mapScale + mapScale / 2;
	drawPlayerArrow(data, framebuffer, playerX, playerY, engine.playerAngle(), colors.player, colors.highlight);
}

static void renderNavigationMapOpaque(const BaseData &data, const BaseEngine &engine,
		byte *framebuffer, const NavigationMapColors &colors) {
	Common::fill(framebuffer, framebuffer + kBaseFrameWidth * kBaseFrameHeight, colors.background);

	static const int kMapScale = 2;
	static const int kMapPixelWidth = kBaseMapWidth * kMapScale;
	static const int kMapPixelHeight = kBaseMapHeight * kMapScale;
	const int mapLeft = (kBaseFrameWidth - kMapPixelWidth) / 2;
	const int mapTop = (kBaseFrameHeight - kMapPixelHeight) / 2;
	for (int mapY = 0; mapY < kBaseMapHeight; ++mapY) {
		for (int mapX = 0; mapX < kBaseMapWidth; ++mapX) {
			const uint16 code = data.mapCodeXY(mapX, mapY);
			const byte lowCode = code & 0xff;
			if (!code || lowCode >= 0xfc)
				continue;
			const byte color = (lowCode == kBaseDoorXCode || lowCode == kBaseDoorYCode) ? colors.door : colors.wall;
			const int pixelX = mapLeft + mapX * kMapScale;
			const int pixelY = mapTop + mapY * kMapScale;
			fillRect(framebuffer, pixelX, pixelY, pixelX + kMapScale - 1, pixelY + kMapScale - 1, color);
		}
	}

	drawMapMarkers(data, engine, framebuffer, kMapScale, mapLeft, mapTop, colors);
	drawOpaqueLegend(data, framebuffer, colors.background, colors.highlight, colors.player,
			colors.exit, colors.guard, colors.door);
}

} // End of anonymous namespace

WBASEEnhancements::WBASEEnhancements(const Common::String &targetName) :
		_enabled(ConfMan.getBool(kWBASEEnhancementsConfigKey, targetName)) {
}

void WBASEEnhancements::renderNavigationMap(const BaseData &data, const BaseEngine &engine, byte *framebuffer) const {
	if (!framebuffer)
		return;

	const NavigationMapColors colors = navigationMapColors(data);
	renderNavigationMapOpaque(data, engine, framebuffer, colors);
}

} // End of namespace Hopkins
