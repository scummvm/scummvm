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

#include "common/util.h"

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

bool RA2PSXTextureSet::append(const Common::Array<byte> &data) {
	return loadRA2PSXTextures(data, _textures);
}

const RA2PSXTexture *RA2PSXTextureSet::find(const char *name) const {
	for (uint i = 0; i < _textures.size(); ++i) {
		if (_textures[i].name.equalsIgnoreCase(name))
			return &_textures[i];
	}
	return nullptr;
}

void RA2PSXTextureSet::draw(Graphics::Surface &surface, const char *name,
		int x, int y, const Common::Rect &source, int brightness, BlendMode blend) const {
	const RA2PSXTexture *texture = find(name);
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

void RA2PSXTextureSet::drawText(Graphics::Surface &surface, const char *font,
		const char *text, int x, int y) const {
	static const char glyphs[] = "abcdefghijklmnopqrstuvwxyz0123456789%-:.,+/C ";
	static const byte widths[] = {
		6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 8, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 8, 6, 7, 6, 6, 4, 6, 6, 6, 6,
		6, 6, 6, 6, 6, 6, 2, 2, 2, 6, 6, 8, 2
	};
	static_assert(ARRAYSIZE(glyphs) == ARRAYSIZE(widths) + 1,
			"RA2 PSX glyph widths do not match the font map");

	for (; *text; ++text) {
		int glyph = -1;
		for (uint i = 0; i < ARRAYSIZE(widths); ++i) {
			if (*text == glyphs[i]) {
				glyph = i;
				break;
			}
		}
		if (glyph < 0)
			continue;

		const int sourceX = (glyph % 12) * 8;
		const int sourceY = (glyph / 12) * 8;
		draw(surface, font, x, y, Common::Rect(sourceX, sourceY,
				sourceX + widths[glyph], sourceY + 8));
		x += widths[glyph] + 2;
	}
}

bool RA2PSXMainMenuUI::load(const RA2PSXArchive &archive) {
	Common::Array<byte> data;
	_textures.clear();
	return archive.getMember("menuTex", data) && _textures.append(data) &&
			_textures.has("BACK_L") && _textures.has("BACK_R") &&
			_textures.has("TITLE") && _textures.has("STD_FT2") &&
			_textures.has("STD_FT4") && _textures.has("STD_FT6");
}

void RA2PSXMainMenuUI::draw(Graphics::Surface &surface, int selection) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	_textures.draw(surface, "BACK_L", xOffset, yOffset, Common::Rect(0, 0, 224, 240));
	_textures.draw(surface, "BACK_R", xOffset + 224, yOffset, Common::Rect(0, 0, 96, 240));
	_textures.draw(surface, "TITLE", xOffset + 72, yOffset + 22, Common::Rect(0, 0, 176, 124));

	static const char *const items[] = { "start", "options" };
	static const int itemX[] = { 141, 134 };
	for (uint i = 0; i < ARRAYSIZE(items); ++i) {
		_textures.drawText(surface, i == (uint)selection ? "STD_FT4" : "STD_FT6",
				items[i], xOffset + itemX[i], yOffset + 166 + i * 10);
	}
	_textures.drawText(surface, "STD_FT2", "developed by factor 5",
			xOffset + 80, yOffset + 202);
	_textures.drawText(surface, "STD_FT2", "C 1996 lucasarts entertainment company",
			xOffset + 16, yOffset + 212);
}

Common::Rect RA2PSXMainMenuUI::itemRect(int item) const {
	return Common::Rect(120, 164 + item * 10, 200, 174 + item * 10);
}

bool RA2PSXLevel1UI::load(const RA2PSXArchive &archive) {
	Common::Array<byte> data;
	_textures.clear();
	if (!archive.getMember("tex/Common", data) || !_textures.append(data))
		return false;
	if (!archive.getMember("tex/BWingCockp", data) || !_textures.append(data))
		return false;

	static const char *const required[] = {
		"COCKPITL", "COCKPITR", "PANEL1", "PANEL2", "PANEL3", "STATTEXT",
		"FONT8X9", "REBLSIGN", "ENRGYMSK", "SMALLEX"
	};
	for (uint i = 0; i < ARRAYSIZE(required); ++i) {
		if (!_textures.has(required[i]))
			return false;
	}
	return true;
}

void RA2PSXLevel1UI::drawCockpit(Graphics::Surface &surface) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2 + 120;
	_textures.draw(surface, "COCKPITL", xOffset, yOffset, Common::Rect(0, 0, 224, 120));
	_textures.draw(surface, "COCKPITR", xOffset + 224, yOffset, Common::Rect(0, 0, 120, 120));
}

void RA2PSXLevel1UI::drawExplosion(Graphics::Surface &surface, int x, int y, int frame) const {
	const int stage = CLIP<int>(frame / 2, 0, 4);
	_textures.draw(surface, "SMALLEX", x - 8, y - 8,
			Common::Rect(stage * 16, 0, stage * 16 + 16, 16), 0x80,
			RA2PSXTextureSet::kBlendAdditive);
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
			_textures.draw(surface, "ENRGYMSK", left, y,
					Common::Rect(MAX(0, sourceLeft), row, 68, row + 1), 0x50,
					RA2PSXTextureSet::kBlendAdditive);
	}
}

void RA2PSXLevel1UI::drawHUD(Graphics::Surface &surface, int score, int lives,
		int shield, int frame) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;

	_textures.draw(surface, "PANEL3", xOffset + 2, yOffset + 17, Common::Rect(0, 0, 126, 21));
	_textures.draw(surface, "PANEL2", xOffset + 262, yOffset + 17, Common::Rect(0, 0, 56, 29));
	_textures.draw(surface, "PANEL1", xOffset + 210, yOffset + 207, Common::Rect(0, 0, 108, 21));
	drawShield(surface, shield, xOffset, yOffset);

	_textures.draw(surface, "STATTEXT", xOffset + 14, yOffset + 21,
			Common::Rect(42, 0, 62, 13), 0x5a);
	score = CLIP<int>(score, 0, 9999999);
	int divisor = 1000000;
	for (int digit = 0; digit < 7; ++digit) {
		const int value = score / divisor % 10;
		_textures.draw(surface, "FONT8X9", xOffset + 39 + digit * 10, yOffset + 23,
				Common::Rect(value * 8, 0, value * 8 + 8, 9));
		divisor /= 10;
	}

	const int rebelFrame = (MAX(frame, 0) / 2) % 12;
	_textures.draw(surface, "REBLSIGN", xOffset + 292, yOffset + 22,
			Common::Rect((rebelFrame % 6) * 20, (rebelFrame / 6) * 19,
					(rebelFrame % 6 + 1) * 20, (rebelFrame / 6 + 1) * 19));
	const int reserveLives = CLIP<int>(lives - 1, 0, 9);
	_textures.draw(surface, "FONT8X9", xOffset + 274, yOffset + 28,
			Common::Rect(reserveLives * 8, 0, reserveLives * 8 + 8, 9));
	_textures.draw(surface, "FONT8X9", xOffset + 283, yOffset + 29, Common::Rect(80, 0, 88, 9));

	int shieldLabelBrightness = 0x5a;
	if (shield <= 31) {
		const int phase = (MAX(frame, 0) / 2) % 14;
		shieldLabelBrightness += phase < 7 ? -50 + phase * 10 : 20 - (phase - 7) * 10;
	}
	_textures.draw(surface, "STATTEXT", xOffset + 291, yOffset + 211,
			Common::Rect(0, 0, 20, 13), shieldLabelBrightness);
}

} // End of namespace Scumm
