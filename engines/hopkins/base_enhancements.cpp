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

struct EnhancementControlDefinition {
	WBASEEnhancementControl control;
	int left;
	int top;
	int right;
	int bottom;
};

static const EnhancementControlDefinition kEnhancementControls[] = {
	{ kWBASEEnhancementControlForward,         2, 182,  26, 198 },
	{ kWBASEEnhancementControlBackward,       28, 182,  52, 198 },
	{ kWBASEEnhancementControlTurnLeft,       54, 182,  78, 198 },
	{ kWBASEEnhancementControlTurnRight,      80, 182, 104, 198 },
	{ kWBASEEnhancementControlFire,          110, 182, 146, 198 },
	{ kWBASEEnhancementControlExit,          150, 182, 186, 198 },
	{ kWBASEEnhancementControlEscape,        192, 182, 228, 198 },
	{ kWBASEEnhancementControlNavigationMap, 244, 182, 280, 198 },
	{ kWBASEEnhancementControlAutoplay,      282, 182, 318, 198 }
};

static const uint16 kTurnLeftGlyphRows[10] = {
	0x040, 0x080, 0x100, 0x3ff, 0x101,
	0x081, 0x041, 0x001, 0x001, 0x001
};

static const uint16 kTryExitGlyphRows[11] = {
	0x1fc0, 0x1040, 0x1048, 0x1044, 0x1002, 0x13ff,
	0x1002, 0x1044, 0x1048, 0x1040, 0x1fc0
};

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

static void drawGlyphMask(byte *framebuffer, const uint16 *rows, int width, int height,
		int x, int y, byte color, bool mirrorX) {
	for (int row = 0; row < height; ++row) {
		for (int column = 0; column < width; ++column) {
			const int sourceColumn = mirrorX ? width - 1 - column : column;
			if (rows[row] & (1U << (width - 1 - sourceColumn)))
				drawPixel(framebuffer, x + column, y + row, color);
		}
	}
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

static void drawFrame(byte *framebuffer, int left, int top, int right, int bottom, byte color) {
	fillRect(framebuffer, left, top, right, top, color);
	fillRect(framebuffer, left, bottom, right, bottom, color);
	fillRect(framebuffer, left, top, left, bottom, color);
	fillRect(framebuffer, right, top, right, bottom, color);
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

static void drawMapButtonGlyph(byte *framebuffer, int left, int top, byte color) {
	const int x = left + 4;
	const int y = top + 4;
	drawLine(framebuffer, x, y, x + 4, y - 1, color);
	drawLine(framebuffer, x + 4, y - 1, x + 8, y + 1, color);
	drawLine(framebuffer, x + 8, y + 1, x + 12, y, color);
	drawLine(framebuffer, x, y, x, y + 8, color);
	drawLine(framebuffer, x + 4, y - 1, x + 4, y + 7, color);
	drawLine(framebuffer, x + 8, y + 1, x + 8, y + 9, color);
	drawLine(framebuffer, x + 12, y, x + 12, y + 8, color);
	drawLine(framebuffer, x, y + 8, x + 4, y + 7, color);
	drawLine(framebuffer, x + 4, y + 7, x + 8, y + 9, color);
	drawLine(framebuffer, x + 8, y + 9, x + 12, y + 8, color);
}

static void drawAutoplayButtonGlyph(byte *framebuffer, int left, int top, byte color, bool locked) {
	fillTriangle(framebuffer, left + 5, top + 4, left + 5, top + 11, left + 11, top + 7, color);
	if (!locked)
		return;
	const int lockX = left + 27;
	const int lockY = top + 4;
	drawLine(framebuffer, lockX, lockY + 3, lockX, lockY + 1, color);
	drawLine(framebuffer, lockX, lockY + 1, lockX + 4, lockY + 1, color);
	drawLine(framebuffer, lockX + 4, lockY + 1, lockX + 4, lockY + 3, color);
	fillRect(framebuffer, lockX - 1, lockY + 3, lockX + 5, lockY + 8, color);
}

static void drawDirectionButtonGlyph(byte *framebuffer, const EnhancementControlDefinition &definition,
		WBASEEnhancementControl control, byte color) {
	const int centerX = (definition.left + definition.right - 1) / 2;
	const int centerY = (definition.top + definition.bottom - 1) / 2;
	if (control == kWBASEEnhancementControlForward || control == kWBASEEnhancementControlBackward) {
		const int direction = control == kWBASEEnhancementControlForward ? -1 : 1;
		drawLine(framebuffer, centerX, centerY - direction * 4,
				centerX, centerY + direction * 4, color);
		drawLine(framebuffer, centerX, centerY + direction * 4,
				centerX - 3, centerY + direction, color);
		drawLine(framebuffer, centerX, centerY + direction * 4,
				centerX + 3, centerY + direction, color);
		return;
	}

	drawGlyphMask(framebuffer, kTurnLeftGlyphRows, 10, 10,
			definition.left + 7, definition.top + 3, color,
			control == kWBASEEnhancementControlTurnRight);
}

static void drawFireButtonGlyph(byte *framebuffer, const EnhancementControlDefinition &definition,
		byte color) {
	const int centerX = (definition.left + definition.right - 1) / 2;
	const int centerY = (definition.top + definition.bottom - 1) / 2;
	drawLine(framebuffer, centerX - 5, centerY, centerX + 5, centerY, color);
	drawLine(framebuffer, centerX, centerY - 5, centerX, centerY + 5, color);
	drawLine(framebuffer, centerX - 3, centerY - 3, centerX + 3, centerY + 3, color);
	drawLine(framebuffer, centerX - 3, centerY + 3, centerX + 3, centerY - 3, color);
}

static void drawExitButtonGlyph(Graphics::Surface &surface, byte *framebuffer,
		const EnhancementControlDefinition &definition, const Graphics::Font &font, byte color) {
	const Common::String label("Try");
	const int gap = 2;
	const int glyphWidth = 13;
	const int glyphHeight = 11;
	const int buttonWidth = definition.right - definition.left;
	const int buttonHeight = definition.bottom - definition.top;
	const int textWidth = font.getStringWidth(label);
	const int groupWidth = textWidth + gap + glyphWidth;
	const int groupLeft = definition.left + (buttonWidth - groupWidth) / 2;
	const int textTop = definition.top + (buttonHeight - font.getFontHeight()) / 2;
	const int glyphLeft = groupLeft + textWidth + gap;
	const int glyphTop = definition.top + (buttonHeight - glyphHeight) / 2;

	font.drawString(&surface, label, groupLeft, textTop, textWidth, color);
	drawGlyphMask(framebuffer, kTryExitGlyphRows, glyphWidth, glyphHeight,
			glyphLeft, glyphTop, color, false);
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
		byte textColor, byte playerColor, byte exitColor, byte guardColor, byte doorColor,
		bool forcedAutoplay) {
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
	const Common::String footer = forcedAutoplay ? _("M: close  A: autoplay  Esc: menu") :
			_("M/Esc: close  A: autoplay");
	font->drawString(&surface, footer, 0, 169, kBaseFrameWidth, textColor,
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
		byte *framebuffer, const NavigationMapColors &colors, bool forcedAutoplay) {
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
			colors.exit, colors.guard, colors.door, forcedAutoplay);
}

} // End of anonymous namespace

WBASEEnhancements::WBASEEnhancements(const Common::String &targetName) :
		_enabled(ConfMan.getBool(kWBASEEnhancementsConfigKey, targetName)),
		_forcedAutoplay(ConfMan.getBool(kWBASEForcedAutoplayConfigKey, targetName)) {
}

void WBASEEnhancements::renderNavigationMap(const BaseData &data, const BaseEngine &engine, byte *framebuffer) const {
	if (!framebuffer)
		return;

	const NavigationMapColors colors = navigationMapColors(data);
	renderNavigationMapOpaque(data, engine, framebuffer, colors, _forcedAutoplay);
}

WBASEEnhancementControl WBASEEnhancements::controlAtPoint(int x, int y) const {
	if (!controlsEnabled())
		return kWBASEEnhancementControlNone;
	for (uint i = 0; i < ARRAYSIZE(kEnhancementControls); ++i) {
		const EnhancementControlDefinition &definition = kEnhancementControls[i];
		if (x >= definition.left && x < definition.right &&
				y >= definition.top && y < definition.bottom)
			return definition.control;
	}
	return kWBASEEnhancementControlNone;
}

bool WBASEEnhancements::controlEnabled(WBASEEnhancementControl control,
		WBASEEnhancementPanel panel) const {
	if (!controlsEnabled() || control == kWBASEEnhancementControlNone)
		return false;
	if (control == kWBASEEnhancementControlNavigationMap ||
			control == kWBASEEnhancementControlAutoplay ||
			control == kWBASEEnhancementControlEscape)
		return true;
	return panel == kWBASEEnhancementPanelNone && !_forcedAutoplay;
}

void WBASEEnhancements::renderControls(const BaseData &data, WBASEEnhancementPanel panel,
		WBASEEnhancementControl hoveredControl, uint32 pressedControls,
		bool autoplayActive, byte *framebuffer) const {
	if (!controlsEnabled() || !framebuffer)
		return;

	Graphics::Surface surface;
	surface.init(kBaseFrameWidth, kBaseFrameHeight, kBaseFrameWidth, framebuffer,
			Graphics::PixelFormat::createFormatCLUT8());
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	if (!font)
		return;

	const byte *palette = data.palette();
	const byte background = nearestPaletteColor(palette, 0, 0, 0);
	const byte idle = nearestPaletteColor(palette, 105, 105, 105);
	const byte hover = nearestPaletteColor(palette, 255, 255, 255);
	const byte pressed = nearestPaletteColor(palette, 255, 140, 0);
	const byte disabled = nearestPaletteColor(palette, 55, 55, 55);
	const byte mapActive = nearestPaletteColor(palette, 0, 220, 255);
	const byte chooserActive = nearestPaletteColor(palette, 255, 220, 0);
	const byte autoplayRunning = nearestPaletteColor(palette, 40, 255, 60);

	for (uint i = 0; i < ARRAYSIZE(kEnhancementControls); ++i) {
		const EnhancementControlDefinition &definition = kEnhancementControls[i];
		const bool enabled = controlEnabled(definition.control, panel);
		byte color = enabled ? idle : disabled;
		if (definition.control == kWBASEEnhancementControlNavigationMap &&
				panel == kWBASEEnhancementPanelNavigationMap)
			color = mapActive;
		else if (definition.control == kWBASEEnhancementControlAutoplay)
			color = panel == kWBASEEnhancementPanelAutoplay ? chooserActive :
					(autoplayActive ? autoplayRunning : color);
		if (enabled && (pressedControls & (1U << definition.control)))
			color = pressed;
		const byte border = enabled && hoveredControl == definition.control ? hover : color;

		fillRect(framebuffer, definition.left, definition.top,
				definition.right - 1, definition.bottom - 1, background);
		drawFrame(framebuffer, definition.left, definition.top,
				definition.right - 1, definition.bottom - 1, border);
		switch (definition.control) {
		case kWBASEEnhancementControlForward:
		case kWBASEEnhancementControlBackward:
		case kWBASEEnhancementControlTurnLeft:
		case kWBASEEnhancementControlTurnRight:
			drawDirectionButtonGlyph(framebuffer, definition, definition.control, color);
			break;
		case kWBASEEnhancementControlFire:
			drawFireButtonGlyph(framebuffer, definition, color);
			break;
		case kWBASEEnhancementControlExit:
			drawExitButtonGlyph(surface, framebuffer, definition, *font, color);
			break;
		case kWBASEEnhancementControlEscape:
			font->drawString(&surface, Common::String("Esc"), definition.left + 2,
					definition.top + 4, definition.right - definition.left - 4, color,
					Graphics::kTextAlignCenter);
			break;
		case kWBASEEnhancementControlNavigationMap:
			drawMapButtonGlyph(framebuffer, definition.left, definition.top, color);
			font->drawString(&surface, Common::String("M"), definition.left + 22,
					definition.top + 4, 9, color, Graphics::kTextAlignCenter);
			break;
		case kWBASEEnhancementControlAutoplay:
			drawAutoplayButtonGlyph(framebuffer, definition.left, definition.top,
					color, _forcedAutoplay);
			font->drawString(&surface, Common::String("A"), definition.left + 15,
					definition.top + 4, 9, color, Graphics::kTextAlignCenter);
			break;
		default:
			break;
		}
	}
}

} // End of namespace Hopkins
