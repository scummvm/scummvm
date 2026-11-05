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

#include "common/hashmap.h"
#include "common/ptr.h"
#include "common/util.h"

#include "engines/nancy/graphics.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource/resource.h"

#include "engines/nancy/script/v5/bitmaptext.h"

namespace Nancy {
namespace Script {

static const int kDefaultFontID = 4;
static const int kDefaultFontColor = 0;

typedef ScriptBootData::BitmapFont BitmapFont;

static const BitmapFont *findFont(int id) {
	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	if (!boot) {
		return nullptr;
	}

	for (const BitmapFont &font : boot->bitmapFonts) {
		if (font.id == id) {
			return &font;
		}
	}

	return nullptr;
}

static uint32 findFontColor(int id) {
	const ScriptBootData *boot = GetEngineData(ScriptBootData);
	if (boot) {
		for (const ScriptBootData::FontColor &color : boot->fontColors) {
			if (color.id == id) {
				return color.normal;
			}
		}
	}

	return 0xFFFFFFFF;
}

// The pictures of the letters, kept once loaded
static const Graphics::ManagedSurface *getFaceImage(const Common::Path &name) {
	typedef Common::HashMap<Common::Path, Common::SharedPtr<Graphics::ManagedSurface>, Common::Path::IgnoreCase_Hash, Common::Path::IgnoreCase_EqualTo> ImageMap;
	static ImageMap s_images;

	ImageMap::iterator it = s_images.find(name);
	if (it != s_images.end()) {
		return it->_value.get();
	}

	Common::SharedPtr<Graphics::ManagedSurface> image(new Graphics::ManagedSurface());
	Graphics::ManagedSurface loaded;
	if (g_nancy->_resource->loadImage(name, loaded)) {
		image->copyFrom(loaded);
		image->convertToInPlace(g_nancy->_graphics->getTransparentPixelFormat());
	}

	s_images.setVal(name, image);
	return image.get();
}

// The face of a font for the bold and italic settings; the regular one when the
// font has no such face
static const BitmapFont::Face *getFace(const BitmapFont *font, bool isBold, bool isItalic) {
	if (!font) {
		return nullptr;
	}

	int style = BitmapFont::kRegular;
	if (isBold && isItalic) {
		style = BitmapFont::kBoldItalic;
	} else if (isBold) {
		style = BitmapFont::kBold;
	} else if (isItalic) {
		style = BitmapFont::kItalic;
	}

	if (font->faces[style].imageName.empty()) {
		style = BitmapFont::kRegular;
	}

	return &font->faces[style];
}

// Reads the number of a tag like <c12>; returns false when it is something else
static bool readTagNumber(const Common::String &text, uint &pos, char letter, int &number) {
	if (pos + 3 > text.size() || text[pos] != '<' || text[pos + 1] != letter || !Common::isDigit(text[pos + 2])) {
		return false;
	}

	uint end = pos + 2;
	number = 0;
	while (end < text.size() && Common::isDigit(text[end])) {
		number = number * 10 + (text[end] - '0');
		++end;
	}

	if (end >= text.size() || text[end] != '>') {
		return false;
	}

	pos = end + 1;
	return true;
}

static bool readTag(const Common::String &text, uint &pos, const char *tag) {
	const uint length = strlen(tag);
	if (pos + length > text.size() || strncmp(text.c_str() + pos, tag, length) != 0) {
		return false;
	}

	pos += length;
	return true;
}

namespace {

enum HorizontalAlign { kAlignLeft, kAlignCenter, kAlignRight };
enum VerticalAlign { kAlignTop, kAlignMiddle, kAlignBottom };

// What the tags have set so far
struct TextStyle {
	int fontID = kDefaultFontID;
	int colorID = kDefaultFontColor;
	bool isBold = false;
	bool isItalic = false;
	HorizontalAlign horizontalAlign = kAlignLeft;
	VerticalAlign verticalAlign = kAlignTop;
};

// One letter on its way into a line
struct PendingLetter {
	uint32 code = 0;
	const BitmapFont::Face *face = nullptr;
	uint32 color = 0xFFFFFFFF;
	int x = 0;			// of the pen, from the start of the line
	int advance = 0;
};

struct Line {
	Common::Array<PendingLetter> letters;
	int width = 0;
	int height = 0;
	HorizontalAlign align = kAlignLeft;
};

} // End of anonymous namespace

// Takes a tag at pos, and returns whether there was one
static bool readStyleTag(const Common::String &text, uint &pos, TextStyle &style, bool &isNewLine) {
	int number = 0;
	isNewLine = false;

	if (readTagNumber(text, pos, 'f', number)) {
		style.fontID = number;
	} else if (readTagNumber(text, pos, 'c', number)) {
		style.colorID = number;
	} else if (readTag(text, pos, "<b>")) {
		style.isBold = true;
	} else if (readTag(text, pos, "</b>")) {
		style.isBold = false;
	} else if (readTag(text, pos, "<i>")) {
		style.isItalic = true;
	} else if (readTag(text, pos, "</i>")) {
		style.isItalic = false;
	} else if (readTag(text, pos, "<hl>")) {
		style.horizontalAlign = kAlignLeft;
	} else if (readTag(text, pos, "<hc>")) {
		style.horizontalAlign = kAlignCenter;
	} else if (readTag(text, pos, "<hr>")) {
		style.horizontalAlign = kAlignRight;
	} else if (readTag(text, pos, "<vt>")) {
		style.verticalAlign = kAlignTop;
	} else if (readTag(text, pos, "<vc>")) {
		style.verticalAlign = kAlignMiddle;
	} else if (readTag(text, pos, "<vb>")) {
		style.verticalAlign = kAlignBottom;
	} else if (readTag(text, pos, "<n>")) {
		isNewLine = true;
	} else {
		return false;
	}

	return true;
}

void BitmapText::layout(const Common::String &text, int width, int height, int overrideColor) {
	_letters.clear();
	_bounds = ScriptRect();

	TextStyle style;
	Common::Array<Line> lines;
	Line line;
	int breakIndex = -1;	// the letter after the last space of the line
	uint32 lastCode = 0;

	const BitmapFont::Face *face = getFace(findFont(style.fontID), false, false);
	line.height = face ? face->height : 0;

	uint pos = 0;
	while (pos <= text.size()) {
		bool isNewLine = false;
		bool isEnd = pos == text.size();

		if (!isEnd && text[pos] == '<' && readStyleTag(text, pos, style, isNewLine)) {
			if (!isNewLine) {
				if (line.letters.empty()) {
					line.align = style.horizontalAlign;
				}

				continue;
			}
		} else if (!isEnd && (text[pos] == '\n' || text[pos] == '\r')) {
			isNewLine = text[pos] == '\n';
			++pos;
			if (!isNewLine) {
				continue;
			}
		}

		if (isNewLine || isEnd) {
			lines.push_back(line);
			if (isEnd) {
				break;
			}

			line = Line();
			line.align = style.horizontalAlign;
			face = getFace(findFont(style.fontID), style.isBold, style.isItalic);
			line.height = face ? face->height : 0;
			breakIndex = -1;
			lastCode = 0;
			continue;
		}

		const uint32 code = (byte)text[pos++];
		face = getFace(findFont(style.fontID), style.isBold, style.isItalic);
		if (!face) {
			continue;
		}

		PendingLetter letter;
		letter.code = code;
		letter.face = face;
		letter.color = findFontColor(overrideColor >= 0 ? overrideColor : style.colorID);

		int kerning = 0;
		if (lastCode && face->kerning.contains((lastCode << 16) | code)) {
			kerning = face->kerning.getVal((lastCode << 16) | code);
		}

		const BitmapFont::Glyph *glyph = face->glyphs.contains(code) ? &face->glyphs.getVal(code) : nullptr;
		letter.x = line.width + kerning;
		letter.advance = glyph ? glyph->xAdvance : 0;

		// A letter that doesn't fit takes the words after the last space along
		if (width > 0 && code != ' ' && letter.x + letter.advance > width && breakIndex > 0) {
			Line next;
			next.align = style.horizontalAlign;
			next.height = face->height;

			for (uint i = breakIndex; i < line.letters.size(); ++i) {
				PendingLetter moved = line.letters[i];
				moved.x = next.width;
				next.width += moved.advance;
				next.height = MAX(next.height, moved.face->height);
				next.letters.push_back(moved);
			}

			line.letters.resize(breakIndex);

			// The spaces at the end of the line take no room
			while (!line.letters.empty() && line.letters.back().code == ' ') {
				line.letters.pop_back();
			}

			line.width = line.letters.empty() ? 0 : line.letters.back().x + line.letters.back().advance;
			lines.push_back(line);

			line = next;
			breakIndex = -1;
			letter.x = line.width;
		}

		line.letters.push_back(letter);
		line.width = letter.x + letter.advance;
		line.height = MAX(line.height, face->height);
		lastCode = code;

		if (code == ' ') {
			breakIndex = line.letters.size();
		}
	}

	int totalHeight = 0;
	for (const Line &l : lines) {
		totalHeight += l.height;
	}

	// An area without a height or a width puts the text at its top or its left
	int y = 0;
	if (height != 0 && style.verticalAlign == kAlignMiddle) {
		y = (height - totalHeight) / 2;
	} else if (height != 0 && style.verticalAlign == kAlignBottom) {
		y = height - totalHeight;
	}

	// The bounds go around the lines, each as high as its tallest font, which
	// is what the scripts lay their texts out with
	bool hasBounds = false;
	for (const Line &l : lines) {
		int x = 0;
		if (width != 0 && l.align == kAlignCenter) {
			x = (width - l.width) / 2;
		} else if (width != 0 && l.align == kAlignRight) {
			x = width - l.width;
		}

		if (!hasBounds) {
			_bounds = ScriptRect(x, y, x + l.width, y + l.height);
			hasBounds = true;
		} else {
			_bounds.left = MIN<int32>(_bounds.left, x);
			_bounds.right = MAX<int32>(_bounds.right, x + l.width);
			_bounds.bottom = y + l.height;
		}

		for (const PendingLetter &pending : l.letters) {
			Letter letter;
			letter.color = pending.color;
			letter.image = getFaceImage(pending.face->imageName);
			if (pending.face->glyphs.contains(pending.code)) {
				letter.glyph = &pending.face->glyphs.getVal(pending.code);
				letter.position = Common::Point(x + pending.x + letter.glyph->xOffset, y + letter.glyph->yOffset);
			}

			_letters.push_back(letter);
		}

		y += l.height;
	}
}

Common::Rect BitmapText::getInkArea() const {
	Common::Rect area;
	for (const Letter &letter : _letters) {
		if (!letter.glyph) {
			continue;
		}

		Common::Rect glyphArea(letter.glyph->source.width(), letter.glyph->source.height());
		glyphArea.moveTo(letter.position);
		if (area.isEmpty()) {
			area = glyphArea;
		} else {
			area.extend(glyphArea);
		}
	}

	return area;
}

static inline uint32 multiplyColors(uint32 a, uint32 b) {
	uint32 result = 0;
	for (int shift = 0; shift < 32; shift += 8) {
		const uint32 product = (((a >> shift) & 0xFF) * ((b >> shift) & 0xFF) + 127) / 255;
		result |= product << shift;
	}

	return result;
}

void BitmapText::draw(Graphics::ManagedSurface &surface, const Common::Point &origin, uint32 modColor, int first, int count) const {
	const Graphics::PixelFormat &format = surface.format;
	const int last = count < 0 ? (int)_letters.size() : MIN<int>(_letters.size(), first + count);

	for (int i = MAX(first, 0); i < last; ++i) {
		const Letter &letter = _letters[i];
		if (!letter.glyph || !letter.image || !letter.image->getPixels()) {
			continue;
		}

		const Graphics::ManagedSurface &image = *letter.image;
		const uint32 color = multiplyColors(letter.color, modColor);
		const Common::Rect &source = letter.glyph->source;

		for (int y = 0; y < source.height(); ++y) {
			const int destY = origin.y + letter.position.y + y;
			const int sourceY = source.top + y;
			if (destY < 0 || destY >= surface.h || sourceY < 0 || sourceY >= image.h) {
				continue;
			}

			for (int x = 0; x < source.width(); ++x) {
				const int destX = origin.x + letter.position.x + x;
				const int sourceX = source.left + x;
				if (destX < 0 || destX >= surface.w || sourceX < 0 || sourceX >= image.w) {
					continue;
				}

				uint8 a, r, g, b;
				image.format.colorToARGB(image.getPixel(sourceX, sourceY), a, r, g, b);
				const uint32 glyphColor = multiplyColors(((uint32)a << 24) | ((uint32)r << 16) | ((uint32)g << 8) | b, color);
				const uint32 alpha = glyphColor >> 24;
				if (alpha == 0) {
					continue;
				}

				// Laid over what is there already
				uint8 da, dr, dg, db;
				format.colorToARGB(surface.getPixel(destX, destY), da, dr, dg, db);
				const uint32 outAlpha = alpha + da * (255 - alpha) / 255;
				if (outAlpha == 0) {
					continue;
				}

				const uint32 rest = da * (255 - alpha) / 255;
				const uint32 outR = (((glyphColor >> 16) & 0xFF) * alpha + dr * rest) / outAlpha;
				const uint32 outG = (((glyphColor >> 8) & 0xFF) * alpha + dg * rest) / outAlpha;
				const uint32 outB = ((glyphColor & 0xFF) * alpha + db * rest) / outAlpha;
				surface.setPixel(destX, destY, format.ARGBToColor(outAlpha, outR, outG, outB));
			}
		}
	}
}

int getBitmapTextWidth(const Common::String &text, int fontID) {
	BitmapText layout;
	layout.layout(Common::String::format("<f%d>", fontID) + text, 0, 0);
	return layout.getBounds().width();
}

} // End of namespace Script
} // End of namespace Nancy
