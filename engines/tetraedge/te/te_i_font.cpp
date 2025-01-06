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

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_core.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "common/unicode-bidi.h"

namespace Tetraedge {

TeIFont::TeIFont() {
	_codePage = codePage();
}

TeIFont::~TeIFont() {
}

TeIFont::GlyphData TeIFont::glyph(uint pxSize, uint charcode) {
	Graphics::Font *font = getAtSize(pxSize);
	Common::Rect bbox = font->getBoundingBox(charcode);
	TeImage *img = new TeImage();
	Common::SharedPtr<TePalette> nullpal;
	img->createImg(bbox.width(), bbox.height(), nullpal, TeImage::RGBA8);
	font->drawChar(img, charcode, 0, 0, 0xffffffff);
	GlyphData retval;
	retval._charcode = charcode;
	retval._bitmapSize = bbox;
	retval._img = img;
	return retval;
}

Common::CodePage TeIFont::codePage() const {
	Common::String lang = g_engine->getCore()->language();
	if (g_engine->isUtf8Release())
		return Common::CodePage::kUtf8;
	if (lang == "ru")
		return Common::kISO8859_5;
	if (g_engine->getGamePlatform() == Common::Platform::kPlatformAndroid)
		return Common::CodePage::kUtf8;
	if (lang == "he")
		return Common::kWindows1255;
	return Common::kLatin1;
}

int TeIFont::wordWrapText(const Common::String &str, int fontSize, int maxWidth, Common::Array<Common::String> &lines) {
	Graphics::Font *font = getAtSize(fontSize);
	Common::Array<Common::U32String> u32lines;
	int retval = font->wordWrapText(str.decode(_codePage), maxWidth, u32lines);
	for (auto &line: u32lines) {
		lines.push_back(line.encode(_codePage));
	}
	return retval;
}

Common::Rect TeIFont::getBBox(const Common::String &str, int fontSize) {
	Graphics::Font *font = getAtSize(fontSize);
	return font->getBoundingBox(str.decode(_codePage));
}

int TeIFont::getHeight(int fontSize) {
	Graphics::Font *font = getAtSize(fontSize);
	return font->getFontHeight();
}

void TeIFont::draw(TeImage &destImage, const Common::String &str, int fontSize, int yoff, const TeColor &col, TeIFont::AlignStyle align) {
	Graphics::Font *font = getAtSize(fontSize);
	Graphics::TextAlign talign;
	switch (align) {
		case AlignLeft:
			talign = Graphics::kTextAlignLeft;
			break;
		case AlignRight:
			talign = Graphics::kTextAlignRight;
			break;
		// Note: we don't support justify.. just center. (justify is not used anyway)
		case AlignJustify:
		case AlignCenter:
		default:
			talign = Graphics::kTextAlignCenter;
			break;
	}
	const Graphics::PixelFormat &fmt = destImage.format;

	uint32 uintcol = ((uint32)col.a() << fmt.aShift) | ((uint32)(col.r()) << fmt.rShift)
						| ((uint32)(col.g()) << fmt.gShift) | ((uint32)(col.b()) << fmt.bShift);
	Common::U32String line = str.decode(_codePage);
	if (g_engine->getCore()->language() == "he")
		line = Common::convertBiDiU32String(line).visual;
	font->drawString(&destImage, line, 0, yoff, destImage.w, uintcol, talign);
}

float TeIFont::ascender(uint pxSize) {
	Graphics::Font *font = getAtSize(pxSize);
	return font->getFontAscent();
}

float TeIFont::descender(uint pxSize) {
	error("TODO: Implement TeFont3::descender");
}

float TeIFont::height(uint pxSize) {
	Graphics::Font *font = getAtSize(pxSize);
	return font->getFontHeight();
}

TeVector3f32 TeIFont::kerning(uint pxSize, uint charcode1, uint charcode2) {
	Graphics::Font *font = getAtSize(pxSize);
	int offset = font->getKerningOffset(charcode1, charcode2);
	// note: not perfect because we have no Y, but it's ok..
	return TeVector3f32(offset, 0.0f, 0.0f);
}

} // end namespace Tetraedge
