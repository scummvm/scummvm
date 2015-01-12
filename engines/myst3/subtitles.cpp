/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/subtitles.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"

#ifdef USE_ICONV
#include "common/iconv.h"
#endif

#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

namespace Myst3 {

Subtitles *Subtitles::create(Myst3Engine *vm, uint32 id) {
	Subtitles *s = new Subtitles(vm);

	s->loadFontSettings(1100);

	if (!s->loadSubtitles(id)) {
		delete s;
		return 0;
	}

	s->loadFont();
	s->createTexture();

	return s;
}

Subtitles::Subtitles(Myst3Engine *vm) :
		_vm(vm),
		_surface(0),
		_texture(0),
		_frame(-1),
		_font(0),
		_charset(nullptr) {
}

Subtitles::~Subtitles() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}
	if (_texture) {
		_vm->_gfx->freeTexture(_texture);
	}

	delete _font;
	delete[] _charset;
}

void Subtitles::loadFontSettings(int32 id) {
	// Load font settings
	const DirectorySubEntry *fontNums = _vm->getFileDescription("NUMB", id, 0, DirectorySubEntry::kNumMetadata);

	if (!fontNums)
		error("Unable to load font settings values");

	_fontSize = fontNums->getMiscData(0);
	_fontBold = fontNums->getMiscData(1);
	_surfaceHeight = fontNums->getMiscData(2);
	_singleLineTop = fontNums->getMiscData(3);
	_line1Top = fontNums->getMiscData(4);
	_line2Top = fontNums->getMiscData(5);
	_surfaceTop = fontNums->getMiscData(6) + Renderer::kTopBorderHeight + Renderer::kFrameHeight;
	_fontCharsetCode = fontNums->getMiscData(7);

	if (_fontCharsetCode > 0) {
		_fontCharsetCode = 128; // The Japanese subtitles are encoded in CP 932 / Shift JIS
	}

	if (_fontCharsetCode < 0) {
		_fontCharsetCode = -_fontCharsetCode; // Negative values are GDI charset codes
	}

	// We draw the subtitles in the adequate resolution so that they are not
	// scaled up. This is the scale factor of the current resolution
	// compared to the original
	Common::Rect screen = _vm->_gfx->viewport();
	_scale = screen.width() / Renderer::kOriginalWidth;

	const DirectorySubEntry *fontText = _vm->getFileDescription("TEXT", id, 0, DirectorySubEntry::kTextMetadata);

	if (!fontText)
		error("Unable to load font face");

	_fontFace = fontText->getTextData(0);

	const DirectorySubEntry *fontCharset = _vm->getFileDescription("CHAR", id, 0, DirectorySubEntry::kRawData);

	// Load the font charset if any
	if (fontCharset) {
		Common::MemoryReadStream *data = fontCharset->getData();

		_charset = new uint8[data->size()];

		data->read(_charset, data->size());

		delete data;
	}
}

void Subtitles::loadFont() {
#ifdef USE_FREETYPE2
	Common::String ttfFile;
	if (_fontFace == "Arial Narrow") {
		// Use the TTF font provided by the game if TTF support is available
		ttfFile = "arir67w.ttf";
	} else if (_fontFace == "MS Gothic") {
		// The Japanese font has to be supplied by the user
		ttfFile = "msgothic.ttf";
	} else {
		error("Unknown subtitles font face '%s'", _fontFace.c_str());
	}

	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember(ttfFile);
	if (s) {
		_font = Graphics::loadTTFFont(*s, _fontSize * _scale);
		delete s;
	} else {
		warning("Unable to load the subtitles font '%s'", ttfFile.c_str());
	}
#endif
}

bool Subtitles::loadSubtitles(int32 id) {
	// Subtitles may be overridden using a variable
	const DirectorySubEntry *desc;
	if (_vm->_state->getMovieOverrideSubtitles()) {
		id = _vm->_state->getMovieOverrideSubtitles();
		_vm->_state->setMovieOverrideSubtitles(0);

		desc = _vm->getFileDescription("IMGR", 100000 + id, 0, DirectorySubEntry::kText);
	} else {
		desc = _vm->getFileDescription(0, 100000 + id, 0, DirectorySubEntry::kText);
	}

	if (!desc)
		return false;

	Common::MemoryReadStream *crypted = desc->getData();

	// Read the frames and associated text offsets
	while (true) {
		Phrase s;
		s.frame = crypted->readUint32LE();
		s.offset = crypted->readUint32LE();

		if (!s.frame)
			break;

		_phrases.push_back(s);
	}

	// Read and decrypt the frames subtitles
	for (uint i = 0; i < _phrases.size(); i++) {
		crypted->seek(_phrases[i].offset);

		uint8 key = 35;
		while (true) {
			uint8 c = crypted->readByte() ^ key++;

			if (c >= 32 && _charset)
				c = _charset[c - 32];

			if (!c)
				break;

			_phrases[i].string += c;
		}
	}

	delete crypted;

	return true;
}

void Subtitles::createTexture() {
	// Create a surface to draw the subtitles on
	// Use RGB 565 to allow use of BDF fonts
	_surface = new Graphics::Surface();
	_surface->create(Renderer::kOriginalWidth * _scale, _surfaceHeight * _scale, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	_texture = _vm->_gfx->createTexture(_surface);
}

const char *Subtitles::getCodePage(uint32 gdiCharset) {
	static const struct {
		uint32 charset;
		const char *codepage;
	} codepages[] = {
			{ 128, "cp932"  }, // SHIFTJIS_CHARSET
			{ 129, "cp949"  }, // HANGUL_CHARSET
			{ 130, "cp1361" }, // JOHAB_CHARSET
			{ 134, "cp936"  }, // GB2312_CHARSET
			{ 136, "cp950"  }, // CHINESEBIG5_CHARSET
			{ 161, "cp1253" }, // GREEK_CHARSET
			{ 162, "cp1254" }, // TURKISH_CHARSET
			{ 163, "cp1258" }, // VIETNAMESE_CHARSET
			{ 177, "cp1255" }, // HEBREW_CHARSET
			{ 178, "cp1256" }, // ARABIC_CHARSET
			{ 186, "cp1257" }, // BALTIC_CHARSET
			{ 204, "cp1251" }, // RUSSIAN_CHARSET
			{ 222, "cp874"  }, // THAI_CHARSET
			{ 238, "cp1250" }  // EASTEUROPE_CHARSET
	};

	for (uint i = 0; i < ARRAYSIZE(codepages); i++) {
		if (gdiCharset == codepages[i].charset) {
			return codepages[i].codepage;
		}
	}

	error("Unknown font charset code '%d'", gdiCharset);
}

void Subtitles::setFrame(int32 frame) {
	const Phrase *phrase = 0;

	for (uint i = 0; i < _phrases.size(); i++) {
		if (_phrases[i].frame > frame)
			break;

		phrase = &_phrases[i];
	}

	if (phrase == 0
			|| phrase->frame == _frame)
		return;

	_frame = phrase->frame;


	const Graphics::Font *font;
	if (_font)
		font = _font;
	else
		font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	if (!font)
		error("No available font");

	// Draw the new text
	memset(_surface->getPixels(), 0, _surface->pitch * _surface->h);


	if (_fontCharsetCode == 0) {
		font->drawString(_surface, phrase->string, 0, _singleLineTop * _scale, _surface->w, 0xFFFFFFFF, Graphics::kTextAlignCenter);
	} else {
		const char *codepage = getCodePage(_fontCharsetCode);
#ifdef USE_ICONV
		Common::U32String unicode = Common::convertToU32String(codepage, phrase->string);
		font->drawString(_surface, unicode, 0, _singleLineTop * _scale, _surface->w, 0xFFFFFFFF, Graphics::kTextAlignCenter);
#else
		warning("Unable to display codepage '%s' subtitles, iconv support is not compiled in.", codepage);
#endif
	}

	// Update the texture
	_texture->update(_surface);
}

void Subtitles::drawOverlay() {
	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);
	Common::Rect bottomBorder = Common::Rect(Renderer::kOriginalWidth, _surfaceHeight);
	bottomBorder.translate(0, _surfaceTop);

	_vm->_gfx->drawTexturedRect2D(bottomBorder, textureRect, _texture);
}

} // End of namespace Myst3
