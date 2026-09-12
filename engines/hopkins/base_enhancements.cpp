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

#include "common/config-manager.h"
#include "common/util.h"

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

} // End of anonymous namespace

WBASEEnhancements::WBASEEnhancements(const Common::String &targetName) :
		_enabled(ConfMan.getBool(kWBASEEnhancementsConfigKey, targetName)) {
}

void WBASEEnhancements::renderNavigationMap(const BaseData &data, const BaseEngine &engine, byte *framebuffer) const {
	if (!framebuffer)
		return;

	const byte *palette = data.palette();
	const byte backgroundColor = nearestPaletteColor(palette, 0, 0, 0);
	const byte wallColor = nearestPaletteColor(palette, 190, 190, 190);
	const byte doorColor = nearestPaletteColor(palette, 255, 210, 0);
	const byte exitColor = nearestPaletteColor(palette, 0, 220, 255);
	const byte guardColor = nearestPaletteColor(palette, 255, 70, 20);
	const byte playerColor = nearestPaletteColor(palette, 40, 255, 60);
	Common::fill(framebuffer, framebuffer + kBaseFrameWidth * kBaseFrameHeight, backgroundColor);

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

			byte color = wallColor;
			if (lowCode == kBaseDoorXCode || lowCode == kBaseDoorYCode)
				color = doorColor;
			else if (lowCode == kBaseExitBitmap)
				color = exitColor;

			const int pixelX = mapLeft + mapX * kMapScale;
			const int pixelY = mapTop + mapY * kMapScale;
			for (int y = 0; y < kMapScale; ++y)
				for (int x = 0; x < kMapScale; ++x)
					drawPixel(framebuffer, pixelX + x, pixelY + y, color);
		}
	}

	for (int objectId = 1; objectId <= kBaseMaxObjects; ++objectId) {
		const BaseObject &object = engine.object(objectId);
		if (!object.active || object.mode == kBaseObjectDead)
			continue;
		const int pixelX = mapLeft + (object.x >> kBaseCellShift) * kMapScale;
		const int pixelY = mapTop + (object.y >> kBaseCellShift) * kMapScale;
		drawPixel(framebuffer, pixelX, pixelY, guardColor);
	}

	const int playerX = mapLeft + (engine.playerX() >> kBaseCellShift) * kMapScale;
	const int playerY = mapTop + (engine.playerY() >> kBaseCellShift) * kMapScale;
	for (int y = -2; y <= 2; ++y)
		for (int x = -2; x <= 2; ++x)
			drawPixel(framebuffer, playerX + x, playerY + y, playerColor);
	const int directionX = playerX + (int)(((int64)data.cosQ16(engine.playerAngle()) * 10) >> kBaseFixedShift);
	const int directionY = playerY + (int)(((int64)data.sinQ16(engine.playerAngle()) * 10) >> kBaseFixedShift);
	drawLine(framebuffer, playerX, playerY, directionX, directionY, playerColor);
}

} // End of namespace Hopkins
