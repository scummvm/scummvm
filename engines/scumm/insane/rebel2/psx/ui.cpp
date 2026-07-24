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

static const byte kMovieBigAdvances[] = {
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 16, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 16,
	8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

static const byte kMovieBigWidths[] = {
	6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 10, 6, 6, 6, 6,
	6, 6, 6, 6, 8, 14, 6, 6, 6, 4, 6, 6, 6, 6, 6, 6, 6,
	6, 2, 4, 6, 2, 10, 6, 4, 6, 6, 6, 4, 6, 6, 6, 6, 10,
	6, 6, 6, 8, 2, 6, 4, 6, 6, 6, 6, 6, 6, 6, 6, 8, 4
};

static const byte kMovieSmallWidths[] = {
	6, 6, 6, 6, 6, 6, 6, 6, 2, 6, 6, 6, 8, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 8, 6, 6, 6, 6, 4, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 2, 2, 6, 2, 6, 10, 10, 10, 10, 2
};

static const byte kMovieTinyWidths[] = {
	10, 10, 10, 10, 10, 10, 12, 12, 4, 10, 10, 8, 14, 10, 12,
	10, 12, 10, 10, 12, 12, 12, 14, 10, 12, 10, 8, 8, 8, 8,
	8, 6, 8, 8, 4, 6, 8, 4, 12, 8, 8, 8, 8, 6, 8, 6, 8, 8,
	12, 10, 8, 8, 8, 6, 8, 8, 10, 8, 8, 8, 8, 8, 4, 6, 10,
	4, 10, 4, 6, 4, 4, 6, 6, 8, 4, 8, 8, 14, 10, 4, 8, 8,
	6, 6, 6, 6, 8, 6, 4, 6
};

static const RA2PSXMovieFont kMovieBigFont = {
	"fNT24b",
	"ABCDEFGHIJKLMN" "\x01" "OPQRSTUVWXYZ-" "\x01"
	"abcdefghijklmn," "\x01" "opqrstuvwxyz%." "\x01" "0123456789? ",
	kMovieBigWidths, kMovieBigAdvances, 0, 16, 16
};

static const RA2PSXMovieFont kMovieSmallFont = {
	"fNT24s",
	"abcdefghijkl" "\x01" "mnopqrstuvwx" "\x01" "yz0123456789" "\x01"
	"%-:.? /{}[]|",
	kMovieSmallWidths, nullptr, 12, 10, 10
};

static const RA2PSXMovieFont kMovieTinyFont = {
	"fNT24t",
	"ABCDEFGHIJ" "\x01" "KLMNOPQRST" "\x01" "UVWXYZabcd" "\x01"
	"efghijklmn" "\x01" "opqrstuvwx" "\x01" "yz01234567" "\x01"
	"89!'+,-./:" "\x01" "^()*;?=%_ " "\x01" "<>`[\\]@{|}",
	kMovieTinyWidths, nullptr, 16, 16, 17
};

static const RA2PSXMovieTextRecord kOpeningText[] = {
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

static const RA2PSXMovieTextRecord kChapter1Text[] = {
	{ 30, 80, 1, 1, 0, 0, 24, "chapter 1" },
	{ 40, 70, 3, 128, 0, 0, 37, "The Dreighton Triangle" }
};

static bool findMovieGlyph(const RA2PSXMovieFont &font, char character,
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

static const RA2PSXMovieFont &getMovieFont(byte style) {
	if (style == 2 || style == 3)
		return kMovieBigFont;
	if (style == 6 || style == 7)
		return kMovieTinyFont;
	return kMovieSmallFont;
}

static int measureMovieText(const RA2PSXMovieFont &font, const char *text,
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

static int scaleMovieX(int x) {
	const int remainder = x % 3;
	if (remainder)
		x += 3 - remainder;
	return x * 2 / 3;
}

static int findMovieFontArchive(const Common::Array<byte> &data) {
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
