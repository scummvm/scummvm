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
 */

#include "common/endian.h"
#include "common/util.h"

#include "scumm/insane/rebel2/psx/psx.h"
#include "scumm/insane/rebel2/psx/ui.h"

namespace Scumm {

struct RA2PSXUIColor {
	int r;
	int g;
	int b;
};

struct RA2PSXUIGradientStop {
	int index;
	RA2PSXUIColor color;
};

static RA2PSXUIColor interpolateColor(const RA2PSXUIColor &from,
		const RA2PSXUIColor &to, int value, int maximum) {
	if (maximum <= 0)
		return from;
	RA2PSXUIColor color;
	color.r = from.r + (to.r - from.r) * value / maximum;
	color.g = from.g + (to.g - from.g) * value / maximum;
	color.b = from.b + (to.b - from.b) * value / maximum;
	return color;
}

static RA2PSXUIColor shieldColor(const RA2PSXUIGradientStop *stops, uint count, int index) {
	for (uint i = 1; i < count; ++i) {
		if (index <= stops[i].index)
			return interpolateColor(stops[i - 1].color, stops[i].color,
					index - stops[i - 1].index, stops[i].index - stops[i - 1].index);
	}
	return stops[count - 1].color;
}

bool RA2PSXLevel1UI::loadTextures(const Common::Array<byte> &data) {
	const uint initialCount = _textures.size();
	uint32 offset = 0;
	while (offset + 20 <= data.size()) {
		Common::String name;
		for (uint i = 0; i < 8 && data[offset + i]; ++i) {
			if (data[offset + i] < 0x20 || data[offset + i] >= 0x7f)
				return _textures.size() > initialCount;
			name += (char)data[offset + i];
		}
		if (name.empty())
			break;

		const uint16 recordSize = READ_LE_UINT16(data.data() + offset + 12);
		const uint16 widthField = READ_LE_UINT16(data.data() + offset + 16);
		const uint16 heightField = READ_LE_UINT16(data.data() + offset + 18);
		const uint16 width = (widthField & 0xff) ? widthField & 0xff : 256;
		const uint16 height = (heightField & 0xff) ? heightField & 0xff : 256;
		const bool eightBit = (widthField & 0x100) != 0;
		const uint32 paletteColors = eightBit ? 256 : 16;
		const uint32 pixelCount = (uint32)width * height;
		const uint32 pixelBytes = eightBit ? pixelCount : (pixelCount + 1) / 2;
		const uint32 paletteOffset = offset + 20;
		const uint32 pixelsOffset = paletteOffset + paletteColors * 2;
		if (recordSize < 20 || offset + recordSize > data.size() ||
				pixelsOffset + pixelBytes > offset + recordSize)
			break;

		Texture texture;
		texture.name = name;
		texture.width = width;
		texture.height = height;
		texture.pixels.resize(pixelCount);
		for (uint32 i = 0; i < pixelCount; ++i) {
			const byte packed = data[pixelsOffset + (eightBit ? i : i / 2)];
			const byte paletteIndex = eightBit ? packed : ((i & 1) ? packed >> 4 : packed & 0xf);
			const uint16 value = READ_LE_UINT16(data.data() + paletteOffset + paletteIndex * 2);
			if (!value) {
				texture.pixels[i] = 0;
				continue;
			}

			const uint32 r = ((value & 0x1f) << 3) | ((value & 0x1f) >> 2);
			const uint32 g = (((value >> 5) & 0x1f) << 3) | (((value >> 5) & 0x1f) >> 2);
			const uint32 b = (((value >> 10) & 0x1f) << 3) | (((value >> 10) & 0x1f) >> 2);
			texture.pixels[i] = 0x01000000 | (r << 16) | (g << 8) | b;
		}
		_textures.push_back(texture);
		offset += recordSize;
	}
	return _textures.size() > initialCount;
}

const RA2PSXLevel1UI::Texture *RA2PSXLevel1UI::findTexture(const char *name) const {
	for (uint i = 0; i < _textures.size(); ++i) {
		if (_textures[i].name.equalsIgnoreCase(name))
			return &_textures[i];
	}
	return nullptr;
}

bool RA2PSXLevel1UI::load(const RA2PSXArchive &archive) {
	Common::Array<byte> data;
	_textures.clear();
	if (!archive.getMember("tex/Common", data) || !loadTextures(data))
		return false;
	if (!archive.getMember("tex/BWingCockp", data) || !loadTextures(data))
		return false;

	static const char *const required[] = {
		"COCKPITL", "COCKPITR", "PANEL1", "PANEL2", "PANEL3", "STATTEXT",
		"FONT8X9", "REBLSIGN", "ENRGYMSK", "SMALLEX"
	};
	for (uint i = 0; i < ARRAYSIZE(required); ++i) {
		if (!findTexture(required[i]))
			return false;
	}
	return true;
}

void RA2PSXLevel1UI::drawTexture(Graphics::Surface &surface, const char *name,
		int x, int y, const Common::Rect &source, int brightness, BlendMode blend) const {
	const Texture *texture = findTexture(name);
	if (!texture)
		return;

	int sourceLeft = MAX<int>(0, source.left);
	int sourceTop = MAX<int>(0, source.top);
	int sourceRight = MIN<int>(texture->width, source.right);
	int sourceBottom = MIN<int>(texture->height, source.bottom);
	x += sourceLeft - source.left;
	y += sourceTop - source.top;
	if (x < 0) {
		sourceLeft -= x;
		x = 0;
	}
	if (y < 0) {
		sourceTop -= y;
		y = 0;
	}
	sourceRight = MIN<int>(sourceRight, sourceLeft + surface.w - x);
	sourceBottom = MIN<int>(sourceBottom, sourceTop + surface.h - y);
	if (sourceLeft >= sourceRight || sourceTop >= sourceBottom)
		return;

	brightness = CLIP<int>(brightness, 0, 0xff);
	for (int sourceY = sourceTop; sourceY < sourceBottom; ++sourceY) {
		const int destY = y + sourceY - sourceTop;
		for (int sourceX = sourceLeft; sourceX < sourceRight; ++sourceX) {
			const uint32 pixel = texture->pixels[sourceY * texture->width + sourceX];
			if (!(pixel & 0x01000000))
				continue;

			int r = ((pixel >> 16) & 0xff) * brightness / 0x80;
			int g = ((pixel >> 8) & 0xff) * brightness / 0x80;
			int b = (pixel & 0xff) * brightness / 0x80;
			r = MIN(r, 0xff);
			g = MIN(g, 0xff);
			b = MIN(b, 0xff);
			const int destX = x + sourceX - sourceLeft;
			if (blend == kBlendAdditive) {
				byte destR, destG, destB;
				surface.format.colorToRGB(surface.getPixel(destX, destY), destR, destG, destB);
				r = MIN<int>(0xff, r + destR);
				g = MIN<int>(0xff, g + destG);
				b = MIN<int>(0xff, b + destB);
			}
			surface.setPixel(destX, destY, surface.format.RGBToColor(r, g, b));
		}
	}
}

void RA2PSXLevel1UI::drawCockpit(Graphics::Surface &surface) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2 + 120;
	drawTexture(surface, "COCKPITL", xOffset, yOffset, Common::Rect(0, 0, 224, 120));
	drawTexture(surface, "COCKPITR", xOffset + 224, yOffset, Common::Rect(0, 0, 120, 120));
}

void RA2PSXLevel1UI::drawExplosion(Graphics::Surface &surface, int x, int y, int frame) const {
	const int stage = CLIP<int>(frame / 2, 0, 4);
	drawTexture(surface, "SMALLEX", x - 8, y - 8,
			Common::Rect(stage * 16, 0, stage * 16 + 16, 16), 0x80, kBlendAdditive);
}

void RA2PSXLevel1UI::drawShield(Graphics::Surface &surface, int shield,
		int xOffset, int yOffset) const {
	static const RA2PSXUIGradientStop topLeft[] = {
		{ 0, { 0, 83, 0 } }, { 14, { 171, 147, 0 } }, { 31, { 63, 0, 0 } }
	};
	static const RA2PSXUIGradientStop topRight[] = {
		{ 0, { 51, 151, 0 } }, { 6, { 207, 183, 0 } },
		{ 16, { 202, 0, 0 } }, { 31, { 155, 0, 0 } }
	};
	static const RA2PSXUIGradientStop bottomLeft[] = {
		{ 0, { 0, 159, 0 } }, { 13, { 246, 214, 0 } }, { 31, { 183, 0, 0 } }
	};
	static const RA2PSXUIGradientStop bottomRight[] = {
		{ 0, { 226, 207, 0 } }, { 15, { 219, 24, 0 } }, { 31, { 139, 3, 0 } }
	};

	shield = CLIP<int>(shield, 0, 100);
	const int width = shield * 68 / 100;
	if (!width)
		return;
	const int colorIndex = CLIP<int>(32 - shield * 32 / 100, 0, 31);
	const RA2PSXUIColor tl = shieldColor(topLeft, ARRAYSIZE(topLeft), colorIndex);
	const RA2PSXUIColor tr = shieldColor(topRight, ARRAYSIZE(topRight), colorIndex);
	const RA2PSXUIColor bl = shieldColor(bottomLeft, ARRAYSIZE(bottomLeft), colorIndex);
	const RA2PSXUIColor br = shieldColor(bottomRight, ARRAYSIZE(bottomRight), colorIndex);

	for (int row = 0; row < 9; ++row) {
		const int left = xOffset + 297 - width - row;
		const int right = xOffset + 288;
		const int y = yOffset + 213 + row;
		if (left >= right || y < 0 || y >= surface.h)
			continue;
		const RA2PSXUIColor leftColor = interpolateColor(tl, bl, row, 8);
		const RA2PSXUIColor rightColor = interpolateColor(tr, br, row, 8);
		for (int x = MAX(0, left); x < MIN<int>(right, surface.w); ++x) {
			const RA2PSXUIColor color = interpolateColor(leftColor, rightColor,
					x - left, right - left - 1);
			surface.setPixel(x, y, surface.format.RGBToColor(color.r, color.g, color.b));
		}

		const int sourceLeft = 77 - width - row;
		if (sourceLeft < 68)
			drawTexture(surface, "ENRGYMSK", left, y,
					Common::Rect(MAX(0, sourceLeft), row, 68, row + 1), 0x50, kBlendAdditive);
	}
}

void RA2PSXLevel1UI::drawHUD(Graphics::Surface &surface, int score, int lives,
		int shield, int frame) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;

	drawTexture(surface, "PANEL3", xOffset + 2, yOffset + 17, Common::Rect(0, 0, 126, 21));
	drawTexture(surface, "PANEL2", xOffset + 262, yOffset + 17, Common::Rect(0, 0, 56, 29));
	drawTexture(surface, "PANEL1", xOffset + 210, yOffset + 207, Common::Rect(0, 0, 108, 21));
	drawShield(surface, shield, xOffset, yOffset);

	drawTexture(surface, "STATTEXT", xOffset + 14, yOffset + 21,
			Common::Rect(42, 0, 62, 13), 0x5a);
	score = CLIP<int>(score, 0, 9999999);
	int divisor = 1000000;
	for (int digit = 0; digit < 7; ++digit) {
		const int value = score / divisor % 10;
		drawTexture(surface, "FONT8X9", xOffset + 39 + digit * 10, yOffset + 23,
				Common::Rect(value * 8, 0, value * 8 + 8, 9));
		divisor /= 10;
	}

	const int rebelFrame = (MAX(frame, 0) / 2) % 12;
	drawTexture(surface, "REBLSIGN", xOffset + 292, yOffset + 22,
			Common::Rect((rebelFrame % 6) * 20, (rebelFrame / 6) * 19,
					(rebelFrame % 6 + 1) * 20, (rebelFrame / 6 + 1) * 19));
	const int reserveLives = CLIP<int>(lives - 1, 0, 9);
	drawTexture(surface, "FONT8X9", xOffset + 274, yOffset + 28,
			Common::Rect(reserveLives * 8, 0, reserveLives * 8 + 8, 9));
	drawTexture(surface, "FONT8X9", xOffset + 283, yOffset + 29, Common::Rect(80, 0, 88, 9));

	int shieldLabelBrightness = 0x5a;
	if (shield <= 31) {
		const int phase = (MAX(frame, 0) / 2) % 14;
		shieldLabelBrightness += phase < 7 ? -50 + phase * 10 : 20 - (phase - 7) * 10;
	}
	drawTexture(surface, "STATTEXT", xOffset + 291, yOffset + 211,
			Common::Rect(0, 0, 20, 13), shieldLabelBrightness);
}

} // End of namespace Scumm
