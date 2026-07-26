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

#include "common/memstream.h"
#include "common/stream.h"
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

RA2PSXUIColor interpolateColor(const RA2PSXUIColor &from,
		const RA2PSXUIColor &to, int value, int maximum) {
	if (maximum <= 0)
		return from;
	RA2PSXUIColor color;
	color.r = from.r + (to.r - from.r) * value / maximum;
	color.g = from.g + (to.g - from.g) * value / maximum;
	color.b = from.b + (to.b - from.b) * value / maximum;
	return color;
}

RA2PSXUIColor shieldColor(const RA2PSXUIGradientStop *stops, uint count, int index) {
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

bool RA2PSXTextureSet::appendRaw24(const char *name, const Common::Array<byte> &data,
		uint16 width, uint16 height) {
	if (!width || !height || data.size() != (uint32)width * height * 3)
		return false;

	RA2PSXTexture texture;
	texture.name = name;
	texture.width = width;
	texture.height = height;
	texture.pixels.resize((uint32)width * height);
	for (uint32 i = 0; i < texture.pixels.size(); ++i) {
		const byte *pixel = data.data() + i * 3;
		if (!(pixel[0] | pixel[1] | pixel[2])) {
			texture.pixels[i] = 0;
			continue;
		}
		texture.pixels[i] = 0x01000000 | ((uint32)pixel[0] << 16) |
				((uint32)pixel[1] << 8) | pixel[2];
	}
	_textures.push_back(texture);
	return true;
}

const Common::Array<uint32> *RA2PSXTextureSet::palette(const char *name) const {
	if (!name)
		return nullptr;
	const RA2PSXTexture *texture = find(name);
	return texture && !texture->palette.empty() ? &texture->palette : nullptr;
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
			if (blend != kBlendOpaque) {
				byte destR, destG, destB;
				surface.format.colorToRGB(surface.getPixel(destX, destY), destR, destG, destB);
				if (blend == kBlendAdditive) {
					r = MIN<int>(0xff, r + destR);
					g = MIN<int>(0xff, g + destG);
					b = MIN<int>(0xff, b + destB);
				} else {
					r = (r + destR) / 2;
					g = (g + destG) / 2;
					b = (b + destB) / 2;
				}
			}
			surface.setPixel(destX, destY, surface.format.RGBToColor(r, g, b));
		}
	}
}

void RA2PSXTextureSet::drawShape(Graphics::Surface &surface, const char *name,
		int x, int y, const Common::Rect &source, byte r, byte g, byte b) const {
	const RA2PSXTexture *texture = find(name);
	if (!texture)
		return;

	const int sourceLeft = MAX<int>(0, source.left);
	const int sourceTop = MAX<int>(0, source.top);
	const int sourceRight = MIN<int>(texture->width, source.right);
	const int sourceBottom = MIN<int>(texture->height, source.bottom);
	const uint32 color = surface.format.RGBToColor(r, g, b);
	for (int sourceY = sourceTop; sourceY < sourceBottom; ++sourceY) {
		const int destY = y + sourceY - source.top;
		if (destY < 0 || destY >= surface.h)
			continue;
		for (int sourceX = sourceLeft; sourceX < sourceRight; ++sourceX) {
			const int destX = x + sourceX - source.left;
			if (destX < 0 || destX >= surface.w)
				continue;
			if (texture->pixels[sourceY * texture->width + sourceX] & 0x01000000)
				surface.setPixel(destX, destY, color);
		}
	}
}

void subtractRA2PSXRect(Graphics::Surface &surface, const Common::Rect &rect,
		int r, int g, int b) {
	const int left = MAX<int>(0, rect.left);
	const int top = MAX<int>(0, rect.top);
	const int right = MIN<int>(surface.w, rect.right);
	const int bottom = MIN<int>(surface.h, rect.bottom);
	for (int y = top; y < bottom; ++y) {
		for (int x = left; x < right; ++x) {
			byte destR, destG, destB;
			surface.format.colorToRGB(surface.getPixel(x, y), destR, destG, destB);
			surface.setPixel(x, y, surface.format.RGBToColor(MAX<int>(0, destR - r),
					MAX<int>(0, destG - g), MAX<int>(0, destB - b)));
		}
	}
}

void drawRA2PSXGouraudLine(Graphics::Surface &surface, int x0, int y0, int x1, int y1,
		const byte *from, const byte *to) {
	const int steps = MAX(ABS(x1 - x0), ABS(y1 - y0));
	for (int step = 0; step <= steps; ++step) {
		const int x = steps ? x0 + (x1 - x0) * step / steps : x0;
		const int y = steps ? y0 + (y1 - y0) * step / steps : y0;
		if (x < 0 || x >= surface.w || y < 0 || y >= surface.h)
			continue;
		const int r = steps ? from[0] + (to[0] - from[0]) * step / steps : from[0];
		const int g = steps ? from[1] + (to[1] - from[1]) * step / steps : from[1];
		const int b = steps ? from[2] + (to[2] - from[2]) * step / steps : from[2];
		surface.setPixel(x, y, surface.format.RGBToColor(r, g, b));
	}
}

void drawRA2PSXMenuHints(Graphics::Surface &surface, const RA2PSXTextureSet &textures) {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	textures.draw(surface, "BACK", xOffset + 20, yOffset + 216, Common::Rect(0, 0, 46, 11));
	textures.draw(surface, "SELECT", xOffset + 252, yOffset + 216, Common::Rect(0, 0, 56, 11));
}

const char kSmallGlyphs[] = "abcdefghijklmnopqrstuvwxyz0123456789%-:.?+/C ";
const byte kSmallWidths[] = {
	6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 8, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 8, 6, 7, 6, 6, 4, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 2, 2, 6, 6, 6, 8, 2
};
static_assert(ARRAYSIZE(kSmallGlyphs) == ARRAYSIZE(kSmallWidths) + 1,
		"RA2 PSX glyph widths do not match the font map");

int findSmallGlyph(char character) {
	for (uint i = 0; i < ARRAYSIZE(kSmallWidths); ++i) {
		if (character == kSmallGlyphs[i])
			return i;
	}
	return -1;
}

void RA2PSXTextureSet::drawText(Graphics::Surface &surface, const char *font,
		const char *text, int x, int y, int brightness) const {
	for (; *text; ++text) {
		const int glyph = findSmallGlyph(*text);
		if (glyph < 0)
			continue;

		const int sourceX = (glyph % 12) * 8;
		const int sourceY = (glyph / 12) * 8;
		draw(surface, font, x, y, Common::Rect(sourceX, sourceY,
				sourceX + kSmallWidths[glyph], sourceY + 8), brightness);
		x += kSmallWidths[glyph] + 2;
	}
}

int RA2PSXTextureSet::measureText(const char *text) const {
	int width = 0;
	for (; *text; ++text) {
		const int glyph = findSmallGlyph(*text);
		if (glyph >= 0)
			width += kSmallWidths[glyph] + 2;
	}
	return width ? width - 2 : 0;
}

struct RA2PSXMovieFont {
	const char *texture;
	const char *characters;
	const byte *widths;
	const byte *advances;
	int fixedAdvance;
	int height;
	int rowStep;
};

struct RA2PSXMovieGlyph {
	int x;
	int row;
	int width;
};

struct RA2PSXMovieTextRecord {
	uint16 firstFrame;
	uint16 duration;
	byte style;
	byte color;
	byte initialCharacters;
	int16 x;
	int16 y;
	const char *text;
};

const byte kMovieBigAdvances[] = {
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

const byte kMovieBigWidths[] = {
	6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 10, 6, 6, 6, 6,
	6, 6, 6, 6, 8, 14, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6,
	6, 2, 4, 6, 2, 10, 6, 4, 6, 6, 6, 4, 6, 6, 6, 6, 10,
	6, 6, 6, 8, 2, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 8, 4
};

const byte kMovieSmallWidths[] = {
	6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 8, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 8, 6, 6, 6, 6, 4, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 2, 2, 6, 2, 6, 10, 10, 10, 10, 2
};

const byte kMovieTinyWidths[] = {
	10, 10, 10, 10, 10, 10, 12, 12, 4, 10, 10, 8, 14, 10, 12,
	10, 12, 10, 10, 12, 12, 12, 14, 10, 12, 10, 8, 8, 8, 8,
	8, 6, 8, 8, 4, 6, 8, 4, 12, 8, 8, 8, 8, 6, 8, 6, 8, 8,
	12, 10, 8, 8, 8, 6, 8, 8, 10, 8, 8, 8, 8, 8, 4, 6, 10,
	4, 10, 4, 6, 4, 4, 6, 6, 8, 4, 8, 8, 14, 10, 4, 8, 8,
	6, 6, 6, 6, 8, 6, 4, 6
};

const RA2PSXMovieFont kMovieBigFont = {
	"fNT24b",
	"ABCDEFGHIJKLMN" "\x01" "OPQRSTUVWXYZ-" "\x01"
	"abcdefghijklmn," "\x01" "opqrstuvwxyz%." "\x01" "0123456789? ",
	kMovieBigWidths, kMovieBigAdvances, 0, 16, 16
};

const RA2PSXMovieFont kMovieSmallFont = {
	"fNT24s",
	"abcdefghijkl" "\x01" "mnopqrstuvwx" "\x01" "yz0123456789" "\x01"
	"%-:.? /{}[]|",
	kMovieSmallWidths, nullptr, 12, 10, 10
};

const RA2PSXMovieFont kMovieTinyFont = {
	"fNT24t",
	"ABCDEFGHIJ" "\x01" "KLMNOPQRST" "\x01" "UVWXYZabcd" "\x01"
	"efghijklmn" "\x01" "opqrstuvwx" "\x01" "yz01234567" "\x01"
	"89!'+,-./:" "\x01" "^()*;?=%_ " "\x01" "<>`[\\]@{|}",
	kMovieTinyWidths, nullptr, 16, 16, 17
};

const RA2PSXMovieTextRecord kOpeningText[] = {
	{ 720, 30, 1, 2, 40,   0,  65, "starring" },
	{ 720, 30, 6, 129, 40, 70,  85, "Jamison Jones" },
	{ 720, 30, 6, 129, 40, 250, 100, "Julie Eccles" },
	{ 758, 30, 1, 2, 40,   0,  85, "original design and story" },
	{ 758, 30, 7, 129, 40,  0, 100, "Vince Lee" },
	{ 796, 30, 1, 2, 40,   0,  85, "lead programmer" },
	{ 796, 30, 7, 129, 40,  0, 100, "Jens Petersam" },
	{ 834, 30, 1, 2, 40,   0,  85, "lead artist/animator" },
	{ 834, 30, 7, 129, 40,  0, 100, "Richard Green" },
	{ 872, 30, 1, 2, 40,   0,  80, "sound designers" },
	{ 872, 30, 7, 129, 40,  0,  95, "Rudolf Stember" },
	{ 872, 30, 7, 129, 40,  0, 115, "Larry the O" },
	{ 910, 30, 1, 2, 40,   0,  85, "director of live action" },
	{ 910, 30, 7, 129, 40,  0, 100, "Hal Barwood" },
	{ 948, 68, 0, 2, 40, 146,  55, "artist-animators" },
	{ 948, 34, 6, 129, 40, 60,  70, "Richard Green" },
	{ 948, 34, 6, 129, 40, -1,  70, "Garry M.Gaber" },
	{ 948, 34, 6, 129, 40, 60,  90, "Jon Knoles" },
	{ 948, 34, 6, 129, 40, -1,  90, "Craig Rundels" },
	{ 948, 34, 6, 129, 40, 60, 110, "Daniel Colon Jr." },
	{ 948, 34, 6, 129, 40, -1, 110, "Clint Young" },
	{ 948, 34, 6, 129, 40, 60, 130, "Ron K. Lussier" },
	{ 948, 34, 6, 129, 40, -1, 130, "Seth Piezas" },
	{ 982, 34, 6, 129, 40, 60,  70, "Eric Ingerson" },
	{ 982, 34, 6, 129, 40, -1,  70, "Bill Stoneham" },
	{ 982, 34, 6, 129, 40, 60,  90, "Mario Wagner" },
	{ 982, 34, 6, 129, 40, -1,  90, "Andreas Escher" },
	{ 982, 34, 0, 2, 40, 214, 110, "and" },
	{ 982, 34, 6, 129, 40, 176, 130, "Mechadeus" },
	{ 1024, 30, 0, 2, 40, 146, 85, "real-time models" },
	{ 1024, 30, 6, 129, 40, 150, 100, "Tobias J. Richter" },
	{ 1062, 30, 0, 2, 40, 40, 50, "lead video effects compositor" },
	{ 1062, 30, 6, 129, 40, 100, 65, "Mark Christiansen" },
	{ 1062, 30, 0, 2, 40, 160, 100, "video effects compositor" },
	{ 1062, 30, 6, 129, 40, 230, 115, "Chris Weakley" },
	{ 1100, 30, 0, 2, 40, 100, 50, "lead art technician" },
	{ 1100, 30, 6, 129, 40, 120, 65, "Aaron Muszalski" },
	{ 1100, 30, 0, 2, 40, 200, 100, "art technician" },
	{ 1100, 30, 6, 129, 40, 200, 115, "Doug Shannon" },
	{ 1138, 30, 1, 2, 40,   0,  80, "additional programmers" },
	{ 1138, 30, 7, 129, 40,  0,  95, "Thomas Engel" },
	{ 1138, 30, 7, 129, 40,  0, 115, "Holger Schmidt" },
	{ 1176, 30, 1, 2, 40,   0,  85, "voice director and producer" },
	{ 1176, 30, 7, 129, 40,  0, 100, "Tamlynn Barra" },
	{ 1214, 30, 1, 2, 40,   0,  85, "lead tester" },
	{ 1214, 30, 7, 129, 40,  0, 100, "Matthew Azeveda" },
	{ 1252, 30, 1, 2, 40,   0,  85, "producer - factor 5" },
	{ 1252, 30, 7, 129, 40,  0, 100, "Julian Eggebrecht" },
	{ 1290, 30, 1, 2, 40,   0,  85, "director of production" },
	{ 1290, 30, 7, 129, 40,  0, 100, "Steve Dauterman" },
	{ 1328, 30, 1, 2, 40,   0,  80, "production coordinators" },
	{ 1328, 30, 7, 129, 40,  0,  95, "Rachel Bryant" },
	{ 1328, 30, 7, 129, 40,  0, 115, "Peggy Stok" },
	{ 1366, 30, 1, 2, 40,   0,  85, "special thanks to" },
	{ 1366, 30, 7, 129, 40,  0, 100, "George Lucas" }
};

const RA2PSXMovieTextRecord kChapter1Text[] = {
	{ 30, 80, 1, 1, 0, 0, 24, "chapter 1" },
	{ 40, 70, 3, 128, 0, 0, 37, "The Dreighton Triangle" }
};

bool findMovieGlyph(const RA2PSXMovieFont &font, char character,
		RA2PSXMovieGlyph &glyph) {
	int metric = 0;
	int row = 0;
	int x = 0;
	for (const char *entry = font.characters; *entry; ++entry) {
		if (*entry == '\x01') {
			++row;
			x = 0;
			continue;
		}
		if (*entry == character) {
			glyph.x = x;
			glyph.row = row;
			glyph.width = font.widths[metric];
			return true;
		}
		x += font.advances ? font.advances[metric] : font.fixedAdvance;
		++metric;
	}
	return false;
}

const RA2PSXMovieFont &getMovieFont(byte style) {
	if (style == 2 || style == 3)
		return kMovieBigFont;
	if (style == 6 || style == 7)
		return kMovieTinyFont;
	return kMovieSmallFont;
}

int measureMovieText(const RA2PSXMovieFont &font, const char *text,
		uint characters, int spacing) {
	int width = 0;
	uint drawn = 0;
	for (; *text && drawn < characters; ++text, ++drawn) {
		RA2PSXMovieGlyph glyph;
		if (findMovieGlyph(font, *text, glyph))
			width += glyph.width + spacing;
	}
	return width ? width - spacing : 0;
}

int RA2PSXTextureSet::measureHeadline(const char *text) const {
	return measureMovieText(kMovieBigFont, text, 0xffff, 2);
}

void RA2PSXTextureSet::drawHeadline(Graphics::Surface &surface, const char *text,
		int x, int y) const {
	for (; *text; ++text) {
		RA2PSXMovieGlyph glyph;
		if (!findMovieGlyph(kMovieBigFont, *text, glyph))
			continue;
		const Common::Rect source(glyph.x, glyph.row * kMovieBigFont.rowStep,
				glyph.x + glyph.width, glyph.row * kMovieBigFont.rowStep + kMovieBigFont.height);
		drawShape(surface, "FNT24BIG", x + 1, y + 1, source, 0, 0, 0);
		draw(surface, "FNT24BIG", x, y, source);
		x += glyph.width + 2;
	}
}

int scaleMovieX(int x) {
	const int remainder = x % 3;
	if (remainder)
		x += 3 - remainder;
	return x * 2 / 3;
}

int findMovieFontArchive(const Common::Array<byte> &data) {
	for (uint offset = 0; offset + 40 <= data.size(); offset += 4) {
		if (!memcmp(data.data() + offset, "fNT24s", 7) &&
				!memcmp(data.data() + offset + 16, "fNT24b", 7) &&
				!memcmp(data.data() + offset + 32, "fNT24t", 7))
			return offset;
	}
	return -1;
}

bool RA2PSXMovieText::load(Common::SeekableReadStream &executable) {
	if (executable.size() <= 0 || executable.size() > 2 * 1024 * 1024)
		return false;

	Common::Array<byte> executableData;
	executableData.resize((uint32)executable.size());
	executable.seek(0);
	if (executable.read(executableData.data(), executableData.size()) != executableData.size())
		return false;
	const int archiveOffset = findMovieFontArchive(executableData);
	if (archiveOffset < 0)
		return false;

	Common::MemoryReadStream stream(executableData.data() + archiveOffset,
			executableData.size() - archiveOffset);
	RA2PSXArchive archive;
	if (!archive.load(stream))
		return false;

	Common::Array<byte> data;
	_textures.clear();
	// These sheets are packed RGB24, despite being uploaded as 16-bit VRAM words.
	return archive.getMember("fNT24s", data) &&
			_textures.appendRaw24("fNT24s", data, 144, 120) &&
			archive.getMember("fNT24b", data) &&
			_textures.appendRaw24("fNT24b", data, 128, 80) &&
			archive.getMember("fNT24t", data) &&
			_textures.appendRaw24("fNT24t", data, 160, 153);
}

void RA2PSXMovieText::draw(Graphics::Surface &surface,
		RA2PSXMovieTextSequence sequence, int frame, int xOffset, int yOffset) const {
	const RA2PSXMovieTextRecord *records = nullptr;
	uint recordCount = 0;
	if (sequence == kRA2PSXMovieTextOpening) {
		records = kOpeningText;
		recordCount = ARRAYSIZE(kOpeningText);
	} else if (sequence == kRA2PSXMovieTextChapter1) {
		records = kChapter1Text;
		recordCount = ARRAYSIZE(kChapter1Text);
	}

	for (uint i = 0; i < recordCount; ++i) {
		const RA2PSXMovieTextRecord &record = records[i];
		if (frame < record.firstFrame || frame >= record.firstFrame + record.duration)
			continue;

		const RA2PSXMovieFont &font = getMovieFont(record.style);
		const uint characters = MIN<uint>(40,
				record.initialCharacters + frame - record.firstFrame);
		const int spacing = record.style < 4 ? 2 : 0;
		const int textWidth = measureMovieText(font, record.text, characters, spacing);
		int movieX;
		if (record.style & 1)
			movieX = (480 - textWidth * 3 / 2) / 2;
		else if (record.x < 0)
			movieX = 360 - textWidth;
		else
			movieX = record.x;
		int x = xOffset + scaleMovieX(movieX);

		uint drawn = 0;
		for (const char *text = record.text; *text && drawn < characters; ++text, ++drawn) {
			RA2PSXMovieGlyph glyph;
			if (!findMovieGlyph(font, *text, glyph))
				continue;
			int sourceY = glyph.row * font.rowStep;
			if (record.style < 2)
				sourceY += MIN<int>(record.color, 2) * 40;
			_textures.draw(surface, font.texture, x, yOffset + record.y,
					Common::Rect(glyph.x, sourceY, glyph.x + glyph.width,
							sourceY + font.height));
			x += glyph.width + spacing;
		}
	}
}

bool RA2PSXMainMenuUI::load(const RA2PSXArchive &archive) {
	static const char *const required[] = {
		"BACK_L", "BACK_R", "TITLE", "FNT24BIG", "VOLUMES", "BACK", "SELECT",
		"STD_FT2", "STD_FT3", "STD_FT4", "STD_FT5", "STD_FT6"
	};

	Common::Array<byte> data;
	_textures.clear();
	if (!archive.getMember("menuTex", data) || !_textures.append(data))
		return false;
	for (uint i = 0; i < ARRAYSIZE(required); ++i) {
		if (!_textures.has(required[i]))
			return false;
	}
	return true;
}

void RA2PSXMainMenuUI::drawBackground(Graphics::Surface &surface) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	// The original dims both halves of the backdrop to 0x60/0x80.
	_textures.draw(surface, "BACK_L", xOffset, yOffset, Common::Rect(0, 0, 224, 240), 0x60);
	_textures.draw(surface, "BACK_R", xOffset + 224, yOffset, Common::Rect(0, 0, 96, 240), 0x60);
}

void RA2PSXMainMenuUI::drawForeground(Graphics::Surface &surface, int selection) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	subtractRA2PSXRect(surface, Common::Rect(xOffset + 127, yOffset + 161,
			xOffset + 193, yOffset + 191), 0x46, 0x46, 0x46);
	_textures.draw(surface, "TITLE", xOffset + 72, yOffset + 22, Common::Rect(0, 0, 176, 124),
			0x80, RA2PSXTextureSet::kBlendAverage);

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

Common::Rect RA2PSXMainMenuUI::itemRect(int item) {
	return Common::Rect(120, 164 + item * 10, 200, 174 + item * 10);
}

// Row positions from the original widget tables.
const int16 kOptionRowY[] = { 70, 85, 100, 115, 130, 145, 160, 190 };
const int16 kSoundRowY[] = { 87, 102, 117, 132, 162 };

Common::Rect RA2PSXOptionsUI::mainItemRect(int item) {
	const int y = kOptionRowY[CLIP<int>(item, 0, ARRAYSIZE(kOptionRowY) - 1)];
	return Common::Rect(90, y - 1, 230, y + 9);
}

Common::Rect RA2PSXOptionsUI::soundItemRect(int item) {
	const int y = kSoundRowY[CLIP<int>(item, 0, ARRAYSIZE(kSoundRowY) - 1)];
	return Common::Rect(90, y - 1, 230, y + 9);
}

enum {
	kOptionStateNormal,
	kOptionStateValue,
	kOptionStateSelected,
	// Ours, not the original's: the plain colour dimmed.
	kOptionStateUnavailable
};

bool RA2PSXOptionsUI::isItemAvailable(int item) {
	switch (item) {
	case kItemDifficulty:
	case kItemAdjustSound:
	case kItemResetSettings:
	case kItemExit:
		return true;
	default:
		// These four still need their own screens.
		return false;
	}
}

void RA2PSXOptionsUI::drawItem(Graphics::Surface &surface, const char *text, int x, int y,
		bool centered, int state) const {
	static const char *const fonts[] = { "STD_FT3", "STD_FT2", "STD_FT4", "STD_FT3" };
	static const int brightness[] = { 100, 0x80, 0x80, 0x30 };
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	if (centered)
		x = (320 - _textures.measureText(text)) / 2;
	_textures.drawText(surface, fonts[state], text, xOffset + x, yOffset + y,
			brightness[state]);
}

void RA2PSXOptionsUI::drawMain(Graphics::Surface &surface, int selection,
		const RA2PSXSettings &settings) const {
	struct Entry {
		const char *text;
		int16 x;
		bool centered;
	};
	static const Entry entries[kItemCount] = {
		{ "difficulty:",    96, false },
		{ "adjust sound",   70, true },
		{ "enter passcode", 70, true },
		{ "memory card",    70, true },
		{ "high scores",    70, true },
		{ "controls",       70, true },
		{ "reset settings", 70, true },
		{ "exit",           70, true }
	};
	static const Entry difficulties[] = {
		{ "easy",   182, false },
		{ "medium", 176, false },
		{ "hard",   182, false }
	};

	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	subtractRA2PSXRect(surface, Common::Rect(xOffset + 90, yOffset + 63,
			xOffset + 230, yOffset + 203), 0x20, 0x20, 0x20);
	_textures.drawHeadline(surface, "Options",
			xOffset + (320 - _textures.measureHeadline("Options")) / 2, yOffset + 36);

	for (int item = 0; item < kItemCount; ++item) {
		int state = kOptionStateNormal;
		if (item == selection)
			state = kOptionStateSelected;
		else if (!isItemAvailable(item))
			state = kOptionStateUnavailable;
		drawItem(surface, entries[item].text, entries[item].x, kOptionRowY[item],
				entries[item].centered, state);
	}

	drawRA2PSXMenuHints(surface, _textures);

	// The value dims while the difficulty row itself is highlighted.
	const Entry &value = difficulties[CLIP<int>(settings.difficulty, 0, 2)];
	drawItem(surface, value.text, value.x, kOptionRowY[kItemDifficulty], false,
			selection == kItemDifficulty ? kOptionStateValue : kOptionStateNormal);
}

void RA2PSXOptionsUI::drawVolume(Graphics::Surface &surface, int y, int level) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	const int filled = CLIP(level, 0, 8) * 4;
	if (filled)
		_textures.draw(surface, "VOLUMES", xOffset + 180, yOffset + y,
				Common::Rect(0, 0, filled, 8), 0x60);
	if (filled < 32)
		_textures.draw(surface, "VOLUMES", xOffset + 180 + filled, yOffset + y,
				Common::Rect(filled, 8, 32, 16), 0x32);
}

void RA2PSXOptionsUI::drawSound(Graphics::Surface &surface, int selection,
		const RA2PSXSettings &settings) const {
	static const char *const labels[kSoundItemCount] = {
		"sound mode:", "game f/x:", "game music:", "movies:", "exit"
	};
	static const int16 labelX[kSoundItemCount] = { 94, 110, 96, 126, 0 };

	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	subtractRA2PSXRect(surface, Common::Rect(xOffset + 90, yOffset + 83,
			xOffset + 230, yOffset + 173), 0x20, 0x20, 0x20);
	_textures.drawHeadline(surface, "Adjust Sound",
			xOffset + (320 - _textures.measureHeadline("Adjust Sound")) / 2, yOffset + 37);

	for (int item = 0; item < kSoundItemCount; ++item) {
		drawItem(surface, labels[item], labelX[item], kSoundRowY[item],
				item == kSoundItemExit, item == selection ?
						kOptionStateSelected : kOptionStateNormal);
	}

	drawItem(surface, settings.mono ? "mono" : "stereo", settings.mono ? 186 : 179, 87,
			false, selection == kSoundItemMode ? kOptionStateValue : kOptionStateNormal);
	drawRA2PSXMenuHints(surface, _textures);
	drawVolume(surface, 102, (settings.sfx + 1) / 14);
	drawVolume(surface, 117, settings.music / RA2PSXSettings::kCDStep);
	drawVolume(surface, 132, settings.movies / RA2PSXSettings::kCDStep);
}

void RA2PSXOptionsUI::drawDialog(Graphics::Surface &surface, const char *headline,
		const char *question, const char *detail, int selection) const {
	static const byte kBright[3] = { 0xfc, 0xc8, 0x19 };
	static const byte kDark[3] = { 0x80, 0x65, 0x0c };
	static const byte kMessageBright[3] = { 0xff, 0xff, 0xff };
	static const byte kMessageDark[3] = { 0x80, 0x80, 0x80 };

	const bool message = headline == nullptr;
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	const int height = message ? 0x20 : 0x40;
	int width = MAX(_textures.measureText(question), detail ? _textures.measureText(detail) : 0);
	if (!message)
		width = MAX(width, _textures.measureText(headline));
	const int left = (320 - width) / 2;
	const int top = (240 - height) / 2;

	const Common::Rect box(xOffset + left - 6, yOffset + top - 4,
			xOffset + left - 6 + width + 12, yOffset + top - 4 + height + 8);
	subtractRA2PSXRect(surface, box, 0xff, 0xff, 0xff);

	const byte *bright = message ? kMessageBright : kBright;
	const byte *dark = message ? kMessageDark : kDark;
	const int midX = box.left + (width + 12) / 2;
	const int midY = box.top + (height + 8) / 2;
	drawRA2PSXGouraudLine(surface, box.left, box.top, midX, box.top, bright, dark);
	drawRA2PSXGouraudLine(surface, midX, box.top, box.right, box.top, dark, bright);
	drawRA2PSXGouraudLine(surface, box.right, box.top, box.right, midY, bright, dark);
	drawRA2PSXGouraudLine(surface, box.right, midY, box.right, box.bottom, dark, bright);
	drawRA2PSXGouraudLine(surface, box.right, box.bottom, midX, box.bottom, bright, dark);
	drawRA2PSXGouraudLine(surface, midX, box.bottom, box.left, box.bottom, dark, bright);
	drawRA2PSXGouraudLine(surface, box.left, box.bottom, box.left, midY, bright, dark);
	drawRA2PSXGouraudLine(surface, box.left, midY, box.left, box.top, dark, bright);

	const char *const bodyFont = message ? "STD_FT3" : "STD_FT2";
	const int questionY = message ? (detail ? 9 : 13) : (detail ? 0x14 : 0x19);
	if (!message)
		_textures.drawText(surface, "STD_FT5", headline,
				xOffset + left + (width - _textures.measureText(headline)) / 2,
				yOffset + top, 0x80);
	_textures.drawText(surface, bodyFont, question,
			xOffset + left + (width - _textures.measureText(question)) / 2,
			yOffset + top + questionY, 0x80);
	if (detail)
		_textures.drawText(surface, bodyFont, detail,
				xOffset + left + (width - _textures.measureText(detail)) / 2,
				yOffset + top + (message ? 0x13 : 0x1e), 0x80);

	if (selection >= 0) {
		_textures.drawText(surface, selection == 1 ? "STD_FT4" : "STD_FT3", "yes",
				xOffset + left + 10, yOffset + top + 0x32, selection == 1 ? 0x80 : 100);
		_textures.drawText(surface, selection == 0 ? "STD_FT4" : "STD_FT3", "no",
				xOffset + left + width - 0x1e, yOffset + top + 0x32,
				selection == 0 ? 0x80 : 100);
	}
}

// Row positions and label offsets from the original chapter table.
struct RA2PSXChapterEntry {
	const char *title;
	int16 titleX;
	const char *name;
	int16 nameX;
	int16 barWidth;
};

const RA2PSXChapterEntry kChapters[RA2PSXChapterSelectUI::kChapterCount] = {
	{ "chapter 1:",  142, "the dreighton triangle",       52, 158 },
	{ "chapter 2:",  110, "the corellia star",           110, 122 },
	{ "chapter 3:",  140, "mining tunnels",              110, 100 },
	{ "chapter 4:",  110, "the mine field",              110,  96 },
	{ "chapter 5:",  140, "interceptor attack",           76, 134 },
	{ "chapter 6:",  110, "the mining facility",         110, 128 },
	{ "chapter 7:",  140, "tie training",                132,  78 },
	{ "chapter 8:",  110, "flight to imdaar",            110, 112 },
	{ "chapter 9:",  140, "the asteroid field",           84, 126 },
	{ "chapter 10:", 110, "speeder bikes",               110,  94 },
	{ "chapter 11:", 136, "aboard the terror",            84, 126 },
	{ "chapter 12:", 110, "the sewer",                   110,  76 },
	{ "chapter 13:", 134, "escaping the star destroyer",  12, 198 },
	{ "chapter 14:", 110, "tie attack",                  110,  78 },
	{ "chapter 15:", 134, "imdaar alpha",                122,  88 },
	{ "finale:",     110, "the return home",             110, 112 }
};

int RA2PSXChapterSelectUI::rowY(int chapter, int scroll) {
	return 91 - scroll + chapter * kRowPitch;
}

Common::Rect RA2PSXChapterSelectUI::tileRect(int chapter, int scroll) {
	// Even chapters put the tile on the right, odd ones on the left.
	const int x = (chapter & 1) ? 20 : 220;
	const int y = rowY(chapter, scroll);
	return Common::Rect(x, y, x + kTileWidth, y + kTileHeight);
}

void RA2PSXChapterSelectUI::drawTile(Graphics::Surface &surface, int chapter, int y,
		const Graphics::Surface *previews, bool selected, bool unlocked) const {
	static const byte kIdleCorner[3] = { 0x80, 0x80, 0x80 };
	static const byte kIdleEdge[3] = { 0xff, 0xff, 0xff };
	// The original cycles the highlight through an animated CLUT; this uses the
	// same accent the menu dialogs are drawn with.
	static const byte kActiveCorner[3] = { 0xfc, 0xc8, 0x19 };
	static const byte kActiveEdge[3] = { 0x80, 0x65, 0x0c };

	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	const int left = xOffset + ((chapter & 1) ? 20 : 220);
	const int top = yOffset + y;

	if (unlocked && previews && previews->w >= 320 && previews->h >= 240) {
		const int sourceX = (chapter % 4) * kTileWidth;
		const int sourceY = (chapter / 4) * kTileHeight;
		Common::Rect source(sourceX, sourceY, sourceX + kTileWidth, sourceY + kTileHeight);
		int destX = left;
		int destY = top;
		if (destY < 0) {
			source.top -= destY;
			destY = 0;
		}
		source.bottom = MIN<int>(source.bottom, source.top + surface.h - destY);
		if (source.top < source.bottom && destX >= 0 && destX + kTileWidth <= surface.w)
			surface.copyRectToSurface(*previews, destX, destY, source);
	}

	const byte *corner = selected ? kActiveCorner : kIdleCorner;
	const byte *edge = selected ? kActiveEdge : kIdleEdge;
	const int midX = left + kTileWidth / 2;
	const int midY = top + kTileHeight / 2;
	const int right = left + kTileWidth;
	const int bottom = top + kTileHeight;
	drawRA2PSXGouraudLine(surface, left, top, midX, top, corner, edge);
	drawRA2PSXGouraudLine(surface, midX, top, right, top, edge, corner);
	drawRA2PSXGouraudLine(surface, right, top, right, midY, corner, edge);
	drawRA2PSXGouraudLine(surface, right, midY, right, bottom, edge, corner);
	drawRA2PSXGouraudLine(surface, right, bottom, midX, bottom, corner, edge);
	drawRA2PSXGouraudLine(surface, midX, bottom, left, bottom, edge, corner);
	drawRA2PSXGouraudLine(surface, left, bottom, left, midY, corner, edge);
	drawRA2PSXGouraudLine(surface, left, midY, left, top, edge, corner);
}

void RA2PSXChapterSelectUI::drawLabel(Graphics::Surface &surface, int chapter, int y,
		bool selected, bool unlocked) const {
	const RA2PSXChapterEntry &entry = kChapters[chapter];
	const char *const font = selected ? "STD_FT4" : "STD_FT2";
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;

	if (!unlocked) {
		// Locked chapters only get their number, without the episode name.
		Common::String text = chapter == kChapterCount - 1 ?
				Common::String("finale") : Common::String::format("chapter %d", chapter + 1);
		const int width = _textures.measureText(text.c_str());
		const int x = (chapter & 1) ? 110 : 210 - width;
		if (selected)
			subtractRA2PSXRect(surface, Common::Rect(xOffset + ((chapter & 1) ? 106 : 206 - width),
					yOffset + y + 40, xOffset + ((chapter & 1) ? 106 : 206 - width) + width + 8,
					yOffset + y + 54), 0x28, 0x28, 0x28);
		_textures.drawText(surface, font, text.c_str(), xOffset + x, yOffset + y + 26, 0x7f);
		return;
	}

	if (selected)
		subtractRA2PSXRect(surface, Common::Rect(xOffset + ((chapter & 1) ? 106 : 206 - entry.barWidth),
				yOffset + y + 17, xOffset + ((chapter & 1) ? 106 : 206 - entry.barWidth) +
				entry.barWidth + 8, yOffset + y + 44), 0x28, 0x28, 0x28);
	_textures.drawText(surface, font, entry.title, xOffset + entry.titleX, yOffset + y + 20, 0x7f);
	_textures.drawText(surface, font, entry.name, xOffset + entry.nameX, yOffset + y + 33, 0x7f);
}

void RA2PSXChapterSelectUI::draw(Graphics::Surface &surface, const Graphics::Surface *previews,
		int scroll, int selection, int unlocked) const {
	const int xOffset = (surface.w - 320) / 2;
	const int yOffset = (surface.h - 240) / 2;
	for (int chapter = 0; chapter < kChapterCount; ++chapter) {
		const int y = rowY(chapter, scroll);
		if (y < -kRowPitch || y > 240)
			continue;
		const bool selected = chapter == selection;
		const bool open = chapter < unlocked;
		drawTile(surface, chapter, y, previews, selected, open);
		drawLabel(surface, chapter, y, selected, open);
	}
	// The headline and the button captions sit over the rows.
	_textures.drawHeadline(surface, "Select Chapter", xOffset + 110, yOffset + 20);
	drawRA2PSXMenuHints(surface, _textures);
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

// The damage wash: a full screen Gouraud quad whose corners run through the five frame
// CFlash table in the executable's own resource directory, green against orange.
const byte kRA2PSXHitFlash[4][kRA2PSXHitFlashFrames][3] = {
	{ {   0,  39,   0 }, {   0,  91,   0 }, {   0,  69,   0 }, {   0,  48,   0 }, {   0,  27,   0 } },
	{ {  63,   0,   0 }, { 147,  23,   0 }, { 116,  19,   0 }, {  85,  15,   0 }, {  55,  11,   0 } },
	{ {  55,  11,   0 }, {  85,  15,   0 }, { 116,  19,   0 }, { 147,  23,   0 }, {  63,   0,   0 } },
	{ {   0,  27,   0 }, {   0,  48,   0 }, {   0,  69,   0 }, {   0,  91,   0 }, {   0,  39,   0 } }
};

void drawRA2PSXHitFlash(Graphics::Surface &surface, int frame) {
	if (frame < 0 || frame >= kRA2PSXHitFlashFrames)
		return;
	const int lastX = MAX(1, surface.w - 1);
	const int lastY = MAX(1, surface.h - 1);
	for (int y = 0; y < surface.h; ++y) {
		int left[3];
		int right[3];
		for (int channel = 0; channel < 3; ++channel) {
			left[channel] = (kRA2PSXHitFlash[0][frame][channel] * (lastY - y) +
					kRA2PSXHitFlash[2][frame][channel] * y) / lastY;
			right[channel] = (kRA2PSXHitFlash[1][frame][channel] * (lastY - y) +
					kRA2PSXHitFlash[3][frame][channel] * y) / lastY;
		}
		for (int x = 0; x < surface.w; ++x) {
			byte r, g, b;
			surface.format.colorToRGB(surface.getPixel(x, y), r, g, b);
			const int addR = (left[0] * (lastX - x) + right[0] * x) / lastX;
			const int addG = (left[1] * (lastX - x) + right[1] * x) / lastX;
			const int addB = (left[2] * (lastX - x) + right[2] * x) / lastX;
			surface.setPixel(x, y, surface.format.RGBToColor(
					MIN(0xff, r + addR), MIN(0xff, g + addG), MIN(0xff, b + addB)));
		}
	}
}

void RA2PSXLevel1UI::drawCockpit(Graphics::Surface &surface, int scale,
		int driftX, int driftY) const {
	const int xOffset = (surface.w - 320) / 2 + driftX;
	const int yOffset = (surface.h - 240) / 2 + 120 + driftY;
	if (scale >= 0x1000) {
		_textures.draw(surface, "COCKPITL", xOffset, yOffset, Common::Rect(0, 0, 224, 120));
		_textures.draw(surface, "COCKPITR", xOffset + 224, yOffset, Common::Rect(0, 0, 120, 120));
		return;
	}

	// Draw the shell once, then rescale it about the screen centre.
	Graphics::Surface shell;
	shell.create(320, 120, surface.format);
	_textures.draw(shell, "COCKPITL", 0, 0, Common::Rect(0, 0, 224, 120));
	_textures.draw(shell, "COCKPITR", 224, 0, Common::Rect(0, 0, 120, 120));

	const int centerX = surface.w / 2;
	const int centerY = surface.h / 2;
	const uint32 transparent = shell.format.RGBToColor(0, 0, 0);
	for (int y = 0; y < surface.h; ++y) {
		const int sourceY = ((y - centerY) * 0x1000 / scale) + centerY - yOffset;
		if (sourceY < 0 || sourceY >= shell.h)
			continue;
		for (int x = 0; x < surface.w; ++x) {
			const int sourceX = ((x - centerX) * 0x1000 / scale) + centerX - xOffset;
			if (sourceX < 0 || sourceX >= shell.w)
				continue;
			const uint32 pixel = shell.getPixel(sourceX, sourceY);
			if (pixel != transparent)
				surface.setPixel(x, y, pixel);
		}
	}
	shell.free();
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

	// The gauge runs on the original's 0x1000 scale: 68 pixels wide, 32 colour steps.
	shield = CLIP<int>(shield, 0, kRA2PSXShieldFull);
	const int width = shield * 0x44 >> 12;
	if (!width)
		return;
	const int colorIndex = CLIP<int>(0x20 - (shield >> 7), 0, 31);
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
	if (shield < kRA2PSXLowShield) {
		const int phase = (MAX(frame, 0) / 2) % 14;
		shieldLabelBrightness += phase < 7 ? -50 + phase * 10 : 20 - (phase - 7) * 10;
	}
	_textures.draw(surface, "STATTEXT", xOffset + 291, yOffset + 211,
			Common::Rect(0, 0, 20, 13), shieldLabelBrightness);
}

} // End of namespace Scumm
