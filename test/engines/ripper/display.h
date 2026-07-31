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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ripper/display.h"
#include "ripper/resources.h"

#include <cxxtest/TestSuite.h>

class RipperDisplayTestSuite : public CxxTest::TestSuite {
public:
	void testBitmapFontMeasurement() {
		Ripper::BitmapFontAsset font = makeFont();

		TS_ASSERT_EQUALS(Ripper::BitmapFontRenderer::measureText(font, "A A"), 8U);
		TS_ASSERT_EQUALS(Ripper::BitmapFontRenderer::measureText(font, "A?"), 3U);
	}

	void testBitmapFontRenderingUsesMaskAndSolidColor() {
		Ripper::BitmapFontAsset font = makeFont();
		byte pixels[20];
		memset(pixels, 0, sizeof(pixels));

		Ripper::BitmapFontRenderer::drawText(pixels, 5, font, 1, 1, "A", 9);

		TS_ASSERT_EQUALS(pixels[1 * 5 + 1], 0);
		TS_ASSERT_EQUALS(pixels[1 * 5 + 2], 9);
		TS_ASSERT_EQUALS(pixels[2 * 5 + 1], 9);
		TS_ASSERT_EQUALS(pixels[2 * 5 + 2], 0);
	}

	void testBitmapFontRenderingHonorsClip() {
		Ripper::BitmapFontAsset font = makeFont();
		byte pixels[20];
		memset(pixels, 0, sizeof(pixels));

		Ripper::BitmapFontRenderer::drawTextClipped(pixels, 5, font, 1, 1, "A", 9,
			Common::Rect(0, 0, 2, 4));

		TS_ASSERT_EQUALS(pixels[1 * 5 + 2], 0);
		TS_ASSERT_EQUALS(pixels[2 * 5 + 1], 9);
	}

	void testIndexedBitmapRenderingPreservesTransparentPixels() {
		Ripper::BitmapAssetFrame bitmap;
		bitmap.width = 2;
		bitmap.height = 1;
		bitmap.transparentColor = 0;
		bitmap.pixels.push_back(0);
		bitmap.pixels.push_back(7);
		byte pixels[9];
		memset(pixels, 3, sizeof(pixels));

		Ripper::IndexedBitmapRenderer::drawBitmap(pixels, 3, bitmap, 1, 1);

		TS_ASSERT_EQUALS(pixels[1 * 3 + 1], 3);
		TS_ASSERT_EQUALS(pixels[1 * 3 + 2], 7);
	}

	void testIndexedBitmapNineSliceUsesRowMajorFrameTiles() {
		Common::Array<Ripper::BitmapAssetFrame> skin;
		for (uint index = 0; index < 9; ++index) {
			Ripper::BitmapAssetFrame frame;
			frame.width = 1;
			frame.height = 1;
			frame.transparentColor = 0;
			frame.pixels.push_back(index + 1);
			skin.push_back(frame);
		}
		byte pixels[25];
		memset(pixels, 0, sizeof(pixels));

		TS_ASSERT(Ripper::IndexedBitmapRenderer::drawNineSlice(
			pixels, 5, skin, Common::Rect(1, 1, 4, 4)));

		for (uint row = 0; row < 3; ++row) {
			for (uint column = 0; column < 3; ++column)
				TS_ASSERT_EQUALS(pixels[(row + 1) * 5 + column + 1],
					row * 3 + column + 1);
		}
	}

private:
	Ripper::BitmapFontAsset makeFont() const {
		Ripper::BitmapFontAsset font;
		font.firstCharacter = 'A';
		font.lineHeight = 2;
		font.characterSpacing = 1;
		font.spaceWidth = 2;
		font.transparentColor = 0;
		font.glyphs.resize(1);
		font.glyphs[0].width = 2;
		font.glyphs[0].height = 2;
		font.pixels.push_back(0);
		font.pixels.push_back(7);
		font.pixels.push_back(8);
		font.pixels.push_back(0);
		return font;
	}
};
