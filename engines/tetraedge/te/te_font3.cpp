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

// Since FreeType2 includes files, which contain forbidden symbols, we need to
// allow all symbols here.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include "tetraedge/tetraedge.h"
#include "tetraedge/te/te_font3.h"
#include "tetraedge/te/te_core.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"
#include "common/unicode-bidi.h"

namespace Tetraedge {


static uint getUnicodeFromISO(uint chr)
{
	uint retval;

	TeCore *core = g_engine->getCore();
	bool isrus = (core->fileFlagSystemFlag("language") == "ru");

	if (!isrus) {
		uint uVar1 = 0x152;
		if (chr != 0xbc) {
			uVar1 = chr;
		}
		retval = 0x153;
		if (uVar1 != 0xbd) {
			retval = uVar1;
		}
	} else {
		retval = 0xad;
		if (chr != 0xad) {
			if (chr == 0xf0) {
				retval = 0x2116;
			}
			else if (chr == 0xfd) {
				retval = 0xa7;
			} else {
				retval = chr + 0x360;
				if (0x5e < chr - 0xa1) {
					retval = chr;
				}
			}
		}
	}
	return retval;
}

TeFont3::TeFont3() {
	init();
}

TeFont3::~TeFont3() {
	unload();
}

Graphics::Font *TeFont3::getAtSize(uint size) {
	if (_fonts.contains(size))
		return _fonts.getVal(size);

	if (!_fontFile.isOpen())
		load(getAccessName());

	if (!_fontFile.isOpen())
		error("TeFont3::: Couldn't open font file %s.", getAccessName().c_str());

	_fontFile.seek(0);
	Graphics::Font *newFont = Graphics::loadTTFFont(_fontFile, size, Graphics::kTTFSizeModeCharacter, 0, Graphics::kTTFRenderModeNormal);
	if (!newFont) {
		error("TeFont3::: Couldn't load font %s at size %d.", _loadedPath.c_str(), size);
	}
	_fonts.setVal(size, newFont);
	return newFont;
}

TeFont3::GlyphData TeFont3::glyph(uint pxSize, uint charcode) {
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

Common::CodePage TeFont3::codePage() const {
	Common::String lang = g_engine->getCore()->language();
	if (lang == "he")
		return Common::kWindows1255; 
	if (lang == "ru")
		return Common::kISO8859_5;
	return Common::kLatin1;
}


int TeFont3::wordWrapText(const Common::String &str, int fontSize, int maxWidth, Common::Array<Common::String> &lines) {
	Graphics::Font *font = getAtSize(fontSize);
	Common::Array<Common::U32String> u32lines;
	int retval = font->wordWrapText(str.decode(_codePage), maxWidth, u32lines);
	for (auto &line: u32lines) {
		lines.push_back(line.encode(_codePage));
	}
	return retval;
}

Common::Rect TeFont3::getBoundingBox(const Common::String &str, int fontSize) {
	Graphics::Font *font = getAtSize(fontSize);
	return font->getBoundingBox(str.decode(_codePage));
}

int TeFont3::getHeight(int fontSize) {
	Graphics::Font *font = getAtSize(fontSize);
	return font->getFontHeight();
}

void TeFont3::draw(TeImage &destImage, const Common::String &str, int fontSize, int yoff, const TeColor &col, TeFont3::AlignStyle align) {
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

bool TeFont3::load(const Common::String &path) {
	if (_loadedPath == path && _fontFile.isOpen())
		return true; // already open

	TeCore *core = g_engine->getCore();
	Common::FSNode node = core->findFile(path);
	return load(node);
}

bool TeFont3::load(const Common::FSNode &node) {
	const Common::String path = node.getPath();
	if (_loadedPath == path && _fontFile.isOpen())
		return true; // already open

	setAccessName(path);
	_loadedPath = path;

	if (!node.exists()) {
		warning("TeFont3::load: File %s doesn't exist", path.c_str());
		return false;
	}

	if (_fontFile.isOpen())
		_fontFile.close();

	if (!_fontFile.open(node)) {
		warning("TeFont3::load: can't open %s", path.c_str());
		return false;
	}
	return true;
}

void TeFont3::unload() {
	for (auto &entry : _fonts) {
		delete entry._value;
	}
	_fonts.clear();
	_fontFile.close();
}

void TeFont3::init() {
	_codePage = codePage();
}

float TeFont3::ascender(uint pxSize) {
	Graphics::Font *font = getAtSize(pxSize);
	return font->getFontAscent();
}

float TeFont3::descender(uint pxSize) {
	error("TODO: Implement TeFont3::descender");
}

float TeFont3::height(uint pxSize) {
	Graphics::Font *font = getAtSize(pxSize);
	return font->getFontHeight();
}

TeVector3f32 TeFont3::kerning(uint pxSize, uint isocode1, uint isocode2) {
	uint32 uni1 = getUnicodeFromISO(isocode1);
	uint32 uni2 = getUnicodeFromISO(isocode2);
	Graphics::Font *font = getAtSize(pxSize);
	int offset = font->getKerningOffset(uni1, uni2);
	// note: not perfect because we have no Y, but it's ok..
	return TeVector3f32(offset, 0.0f, 0.0f);
}


} // end namespace Tetraedge
