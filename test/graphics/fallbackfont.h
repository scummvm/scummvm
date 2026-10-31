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

#include <cxxtest/TestSuite.h>

#include "graphics/fonts/fallbackfont.h"

// U+5927 is the CJK ideograph for "large".
static constexpr uint32 kGlyphLarge = 0x5927;
static_assert(kGlyphLarge == static_cast<uint32>(U'大'), "U+5927 must encode the large ideograph");

// U+97D3 is the CJK ideograph used for Korea or Han.
static constexpr uint32 kGlyphHan = 0x97D3;
static_assert(kGlyphHan == static_cast<uint32>(U'韓'), "U+97D3 must encode the Korea or Han ideograph");

// U+FFFD is the Unicode replacement character.
static constexpr uint32 kGlyphReplacement = 0xFFFD;
static_assert(kGlyphReplacement == static_cast<uint32>(U'�'), "U+FFFD must encode the replacement character");

// U+1F600 is the grinning face emoji.
static constexpr uint32 kGlyphGrinningFace = 0x1F600;
static_assert(kGlyphGrinningFace == static_cast<uint32>(U'😀'), "U+1F600 must encode the grinning face emoji");

class StubFallbackFont : public Graphics::Font {
public:
	StubFallbackFont(uint32 firstCharacter, uint32 secondCharacter, int width, int height, int ascent, int descent, int leading, int kerning)
		: _firstCharacter(firstCharacter), _secondCharacter(secondCharacter), _width(width), _height(height), _ascent(ascent), _descent(descent),
		  _leading(leading), _kerning(kerning), _lastDrawnCharacter(0), _lastDrawY(0) {}

	int getFontHeight() const override { return _height; }
	Common::String getFontName() const override { return "stub"; }
	int getFontAscent() const override { return _ascent; }
	int getFontDescent() const override { return _descent; }
	int getFontLeading() const override { return _leading; }
	int getMaxCharWidth() const override { return _width; }
	Graphics::GlyphPresence hasGlyph(uint32 chr) const override {
		return chr == _firstCharacter || chr == _secondCharacter ? Graphics::GlyphPresence::kPresent : Graphics::GlyphPresence::kAbsent;
	}
	int getCharWidth(uint32 chr) const override { return hasGlyph(chr) == Graphics::GlyphPresence::kPresent ? _width : 0; }
	int getKerningOffset(uint32 left, uint32 right) const override {
		return hasGlyph(left) == Graphics::GlyphPresence::kPresent && hasGlyph(right) == Graphics::GlyphPresence::kPresent ? _kerning : 0;
	}
	Common::Rect getBoundingBox(uint32 chr) const override {
		return hasGlyph(chr) == Graphics::GlyphPresence::kPresent ? Common::Rect(1, 2, 1 + _width, 6) : Common::Rect();
	}

	void drawChar(Graphics::Surface *dst, uint32 chr, int x, int y, uint32 color) const override {
		(void)dst;
		(void)x;
		(void)color;
		_lastDrawnCharacter = chr;
		_lastDrawY = y;
	}

	uint32 lastDrawnCharacter() const { return _lastDrawnCharacter; }
	int lastDrawY() const { return _lastDrawY; }

private:
	uint32 _firstCharacter;
	uint32 _secondCharacter;
	int _width;
	int _height;
	int _ascent;
	int _descent;
	int _leading;
	int _kerning;
	mutable uint32 _lastDrawnCharacter;
	mutable int _lastDrawY;
};

class SampleFallbackFont : public StubFallbackFont {
public:
	SampleFallbackFont(bool primary, int width)
		: StubFallbackFont(0, 0, width, 12, 8, 3, 1, 0), _characters(primary ? "한국 韓國 Korea Corée " : "鿿", Common::kUtf8) {}

	Graphics::GlyphPresence hasGlyph(uint32 chr) const override {
		return _characters.contains(chr) ? Graphics::GlyphPresence::kPresent : Graphics::GlyphPresence::kAbsent;
	}

private:
	Common::U32String _characters;
};

class UnknownFallbackFont : public StubFallbackFont {
public:
	UnknownFallbackFont() : StubFallbackFont(0, 0, 7, 12, 8, 3, 1, 0) {}

	Graphics::GlyphPresence hasGlyph(uint32 chr) const override { return Graphics::Font::hasGlyph(chr); }
};

class FallbackFontTestSuite : public CxxTest::TestSuite {
public:
	void testGlyphRoutingAndMetrics() {
		StubFallbackFont *primaryFont = new StubFallbackFont('A', 'B', 5, 12, 8, 3, 1, 2);
		StubFallbackFont *fallbackFont = new StubFallbackFont(kGlyphLarge, kGlyphReplacement, 10, 16, 7, 6, 3, 3);
		Common::Array<Graphics::Font *> fonts;
		fonts.push_back(primaryFont);
		fonts.push_back(fallbackFont);
		Graphics::FallbackFont font(fonts);

		TS_ASSERT_EQUALS(font.getFontHeight(), 12);
		TS_ASSERT_EQUALS(font.getFontAscent(), 8);
		TS_ASSERT_EQUALS(font.getFontDescent(), 3);
		TS_ASSERT_EQUALS(font.getFontLeading(), 1);
		TS_ASSERT_EQUALS(font.getMaxCharWidth(), 10);
		TS_ASSERT_EQUALS(font.hasGlyph('A'), Graphics::GlyphPresence::kPresent);
		TS_ASSERT_EQUALS(font.hasGlyph(kGlyphLarge), Graphics::GlyphPresence::kPresent);
		TS_ASSERT_EQUALS(font.hasGlyph(kGlyphGrinningFace), Graphics::GlyphPresence::kAbsent);
		TS_ASSERT_EQUALS(font.getCharWidth('A'), 5);
		TS_ASSERT_EQUALS(font.getCharWidth(kGlyphLarge), 10);
		TS_ASSERT_EQUALS(font.getCharWidth(kGlyphGrinningFace), 10);
		TS_ASSERT_EQUALS(font.getKerningOffset('A', 'B'), 2);
		TS_ASSERT_EQUALS(font.getKerningOffset(kGlyphLarge, kGlyphLarge), 3);
		TS_ASSERT_EQUALS(font.getKerningOffset('A', kGlyphLarge), 0);

		const Common::Rect fallbackBox = font.getBoundingBox(kGlyphLarge);
		TS_ASSERT_EQUALS(fallbackBox.left, 1);
		TS_ASSERT_EQUALS(fallbackBox.top, 3);
		TS_ASSERT_EQUALS(fallbackBox.right, 11);
		TS_ASSERT_EQUALS(fallbackBox.bottom, 7);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), 'A', 2, 4, 0);
		TS_ASSERT_EQUALS(primaryFont->lastDrawnCharacter(), static_cast<uint32>('A'));
		TS_ASSERT_EQUALS(primaryFont->lastDrawY(), 4);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), kGlyphLarge, 2, 4, 0);
		TS_ASSERT_EQUALS(fallbackFont->lastDrawnCharacter(), kGlyphLarge);
		TS_ASSERT_EQUALS(fallbackFont->lastDrawY(), 5);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), kGlyphGrinningFace, 2, 4, 0);
		TS_ASSERT_EQUALS(fallbackFont->lastDrawnCharacter(), kGlyphReplacement);
		TS_ASSERT_EQUALS(fallbackFont->lastDrawY(), 5);
	}

	void testQuestionMarkReplacement() {
		StubFallbackFont *primaryFont = new StubFallbackFont('A', '?', 5, 12, 8, 3, 1, 2);
		StubFallbackFont *fallbackFont = new StubFallbackFont(kGlyphLarge, kGlyphHan, 10, 16, 7, 6, 3, 3);
		Common::Array<Graphics::Font *> fonts;
		fonts.push_back(primaryFont);
		fonts.push_back(fallbackFont);
		Graphics::FallbackFont font(fonts);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), kGlyphGrinningFace, 2, 4, 0);
		TS_ASSERT_EQUALS(primaryFont->lastDrawnCharacter(), static_cast<uint32>('?'));
		TS_ASSERT_EQUALS(primaryFont->lastDrawY(), 4);
	}

	void testMultipleFallbackFonts() {
		StubFallbackFont *primaryFont = new StubFallbackFont('A', '?', 5, 12, 8, 3, 1, 2);
		StubFallbackFont *firstFallbackFont = new StubFallbackFont(kGlyphLarge, kGlyphReplacement, 10, 16, 7, 6, 3, 3);
		StubFallbackFont *secondFallbackFont = new StubFallbackFont(kGlyphLarge, kGlyphHan, 15, 18, 9, 7, 2, 4);
		Common::Array<Graphics::Font *> fonts;
		fonts.push_back(primaryFont);
		fonts.push_back(firstFallbackFont);
		fonts.push_back(secondFallbackFont);
		Graphics::FallbackFont font(fonts);

		TS_ASSERT_EQUALS(font.getCharWidth(kGlyphLarge), 10);
		TS_ASSERT_EQUALS(font.getCharWidth(kGlyphHan), 15);
		TS_ASSERT_EQUALS(font.getCharWidth(kGlyphGrinningFace), 10);
	}

	void testUnknownGlyphPresence() {
		UnknownFallbackFont *unknownFont = new UnknownFallbackFont();
		StubFallbackFont *knownFont = new StubFallbackFont(kGlyphLarge, kGlyphReplacement, 10, 16, 7, 6, 3, 3);
		Common::Array<Graphics::Font *> fonts;
		fonts.push_back(unknownFont);
		fonts.push_back(knownFont);
		Graphics::FallbackFont font(fonts);

		TS_ASSERT_EQUALS(unknownFont->hasGlyph(kGlyphLarge), Graphics::GlyphPresence::kUnknown);
		TS_ASSERT_EQUALS(font.hasGlyph(kGlyphLarge), Graphics::GlyphPresence::kPresent);
		TS_ASSERT_EQUALS(font.hasGlyph(kGlyphGrinningFace), Graphics::GlyphPresence::kUnknown);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), kGlyphLarge, 2, 4, 0);
		TS_ASSERT_EQUALS(knownFont->lastDrawnCharacter(), kGlyphLarge);

		font.drawChar(static_cast<Graphics::Surface *>(nullptr), kGlyphGrinningFace, 2, 4, 0);
		TS_ASSERT_EQUALS(unknownFont->lastDrawnCharacter(), kGlyphGrinningFace);
	}

	void testKoreanCjkFallbackSampleRouting() {
		SampleFallbackFont *primaryFont = new SampleFallbackFont(true, 5);
		SampleFallbackFont *fallbackFont = new SampleFallbackFont(false, 10);
		Common::Array<Graphics::Font *> fonts;
		fonts.push_back(primaryFont);
		fonts.push_back(fallbackFont);
		Graphics::FallbackFont font(fonts);

		// Route the following original UTF-8 sample through the fallback chain.
		const Common::U32String sample("한국 韓國 Korea Corée 鿿", Common::kUtf8);
		for (uint i = 0; i < sample.size(); i++) {
			const int expectedWidth = i + 1 < sample.size() ? 5 : 10;
			TS_ASSERT_EQUALS(font.getCharWidth(sample[i]), expectedWidth);
		}
	}
};
