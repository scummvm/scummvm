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
#include "engines/myst3/scene.h"
#include "engines/myst3/state.h"

#include "common/archive.h"

#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/fonts/ttf.h"

#include "video/bink_decoder.h"

namespace Myst3 {

class FontSubtitles : public Subtitles {
public:
	FontSubtitles(Myst3Engine *vm);
	virtual ~FontSubtitles();

protected:
	void loadResources() override;
	bool loadSubtitles(int32 id) override;
	void drawToTexture(const Phrase *phrase) override;

private:
	void loadCharset(int32 id);
	void createTexture();
	void readPhrases(const ResourceDescription *desc);
	static Common::String fakeBidiProcessing(const Common::String &phrase);

	const Graphics::Font *_font;
	Graphics::Surface *_surface;
	float _scale;
	uint8 *_charset;
};

FontSubtitles::FontSubtitles(Myst3Engine *vm) :
	Subtitles(vm),
	_font(0),
	_surface(0),
	_scale(1.0),
	_charset(nullptr) {
}

FontSubtitles::~FontSubtitles() {
	if (_surface) {
		_surface->free();
		delete _surface;
	}

	delete _font;
	delete[] _charset;
}

void FontSubtitles::loadResources() {
	// We draw the subtitles in the adequate resolution so that they are not
	// scaled up. This is the scale factor of the current resolution
	// compared to the original
	_scale = getPosition().width() / (float) getOriginalPosition().width();

#ifdef USE_FREETYPE2
	Common::String ttfFile;
	if (_fontFace == "Arial Narrow") {
		// Use the TTF font provided by the game if TTF support is available
		ttfFile = "arir67w.ttf";
	} else if (_fontFace == "MS Gothic") {
		// The Japanese font has to be supplied by the user
		ttfFile = "msgothic.ttf";
	} else if (_fontFace == "Arial2") {
		// The Hebrew font has to be supplied by the user
		ttfFile = "hebrew.ttf";
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

void FontSubtitles::loadCharset(int32 id) {
	ResourceDescription fontCharset = _vm->getFileDescription("CHAR", id, 0, Archive::kRawData);

	// Load the font charset if any
	if (fontCharset.isValid()) {
		Common::SeekableReadStream *data = fontCharset.getData();

		_charset = new uint8[data->size()];

		data->read(_charset, data->size());

		delete data;
	}
}

bool FontSubtitles::loadSubtitles(int32 id) {
	// No game-provided charset for the Japanese version
	if (_fontCharsetCode == 0) {
		loadCharset(1100);
	}

	int32 overridenId = checkOverridenId(id);

	ResourceDescription desc = loadText(overridenId, overridenId != id);

	if (!desc.isValid())
		return false;

	readPhrases(&desc);

	if (_vm->getGameLanguage() == Common::HE_ISR) {
		for (uint i = 0; i < _phrases.size(); i++) {
			_phrases[i].string = fakeBidiProcessing(_phrases[i].string);
		}
	}

	return true;
}

void FontSubtitles::readPhrases(const ResourceDescription *desc) {
	Common::SeekableReadStream *crypted = desc->getData();

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
}

static bool isPunctuation(char c) {
	return c == '.' || c == ',' || c == '\"'  || c == '!' || c == '?';
}

Common::String FontSubtitles::fakeBidiProcessing(const Common::String &phrase) {
	// The Hebrew subtitles are stored in logical order:
	// .ABC DEF GHI
	// This line should be rendered in visual order as:
	// .IHG FED CBA

	// Notice how the dot is on the left both in logical and visual order. This is
	// because it is in left to right order while the Hebrew characters are in right to
	// left order. Text rendering code needs to apply what is called the BiDirectional
	// algorithm to know which parts of an input string are LTR or RTL and how to render
	// them. This is a quite complicated algorithm. Fortunately the subtitles in Myst III
	// only require very specific BiDi processing. The punctuation signs at the beginning of
	// each line need to be moved to the end so that they are visually to the left once
	// the string is rendered from right to left.
	// This method works around the need to implement proper BiDi processing
	// by exploiting that fact.

	uint punctuationCounter = 0;
	while (punctuationCounter < phrase.size() && isPunctuation(phrase[punctuationCounter])) {
		punctuationCounter++;
	}

	Common::String output = Common::String(phrase.c_str() + punctuationCounter);
	for (uint i = 0; i < punctuationCounter; i++) {
		output += phrase[i];
	}

	// Also reverse the string so that it is in visual order.
	// This is necessary because our text rendering code does not actually support RTL.
	for (int i = 0, j = output.size() - 1; i < j; i++, j--) {
		char c = output[i];
		output.setChar(output[j], i);
		output.setChar(c, j);
	}

	return output;
}

void FontSubtitles::createTexture() {
	// Create a surface to draw the subtitles on
	// Use RGB 565 to allow use of BDF fonts
	if (!_surface) {
		uint16 width = Renderer::kOriginalWidth * _scale;
		uint16 height = _surfaceHeight * _scale;

		// Make sure the width is even. Some graphics drivers have trouble reading from
		// surfaces with an odd width (Mesa 18 on Intel).
		width &= ~1;

		_surface = new Graphics::Surface();
		_surface->create(width, height, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));
	}

	if (!_texture) {
		_texture = _vm->_gfx->createTexture(_surface);
	}
}

/** Return an encoding from a GDI Charset as provided to CreateFont */
static Common::CodePage getEncodingFromCharsetCode(uint32 gdiCharset) {
	static const struct {
		uint32 charset;
		Common::CodePage encoding;
	} codepages[] = {
			{ 128, Common::kWindows932            }, // SHIFTJIS_CHARSET
			{ 177, Common::kWindows1255           }, // HEBREW_CHARSET
			{ 204, Common::kWindows1251           }, // RUSSIAN_CHARSET
			{ 238, Common::kMacCentralEurope }  // EASTEUROPE_CHARSET
	};

	for (uint i = 0; i < ARRAYSIZE(codepages); i++) {
		if (gdiCharset == codepages[i].charset) {
			return codepages[i].encoding;
		}
	}

	error("Unknown font charset code '%d'", gdiCharset);
}

void FontSubtitles::drawToTexture(const Phrase *phrase) {
	const Graphics::Font *font;
	if (_font)
		font = _font;
	else
		font = FontMan.getFontByUsage(Graphics::FontManager::kLocalizedFont);

	if (!font)
		error("No available font");

	if (!_texture || !_surface) {
		createTexture();
	}

	// Draw the new text
	memset(_surface->getPixels(), 0, _surface->pitch * _surface->h);


	if (_fontCharsetCode == 0) {
		font->drawString(_surface, phrase->string, 0, _singleLineTop * _scale, _surface->w, 0xFFFFFFFF, Graphics::kTextAlignCenter, 0, false);
	} else {
		Common::CodePage encoding = getEncodingFromCharsetCode(_fontCharsetCode);
		Common::U32String unicode = Common::U32String(phrase->string, encoding);
		font->drawString(_surface, unicode, 0, _singleLineTop * _scale, _surface->w, 0xFFFFFFFF, Graphics::kTextAlignCenter, 0, false);
	}

	// Update the texture
	_texture->update(_surface);
}

class MovieSubtitles : public Subtitles {
public:
	MovieSubtitles(Myst3Engine *vm);
	virtual ~MovieSubtitles();

protected:
	void loadResources() override;
	bool loadSubtitles(int32 id) override;
	void drawToTexture(const Phrase *phrase) override;

private:
	ResourceDescription loadMovie(int32 id, bool overriden);
	void readPhrases(const ResourceDescription *desc);

	Video::BinkDecoder _bink;
};

MovieSubtitles::MovieSubtitles(Myst3Engine *vm) :
		Subtitles(vm) {
}

MovieSubtitles::~MovieSubtitles() {
}

void MovieSubtitles::readPhrases(const ResourceDescription *desc) {
	Common::SeekableReadStream *frames = desc->getData();

	// Read the frames
	uint index = 0;
	while (true) {
		Phrase s;
		s.frame = frames->readUint32LE();
		s.offset = index;

		if (!s.frame)
			break;

		_phrases.push_back(s);
		index++;
	}

	delete frames;
}

ResourceDescription MovieSubtitles::loadMovie(int32 id, bool overriden) {
	ResourceDescription desc;
	if (overriden) {
		desc = _vm->getFileDescription("IMGR", 200000 + id, 0, Archive::kMovie);
	} else {
		desc = _vm->getFileDescription("", 200000 + id, 0, Archive::kMovie);
	}
	return desc;
}

bool MovieSubtitles::loadSubtitles(int32 id) {
	int32 overridenId = checkOverridenId(id);

	ResourceDescription phrases = loadText(overridenId, overridenId != id);
	ResourceDescription movie = loadMovie(overridenId, overridenId != id);

	if (!phrases.isValid() || !movie.isValid())
		return false;

	readPhrases(&phrases);

	// Load the movie
	Common::SeekableReadStream *movieStream = movie.getData();
	_bink.setDefaultHighColorFormat(Texture::getRGBAPixelFormat());
	_bink.loadStream(movieStream);
	_bink.start();

	return true;
}

void MovieSubtitles::loadResources() {
}

void MovieSubtitles::drawToTexture(const Phrase *phrase) {
	_bink.seekToFrame(phrase->offset);
	const Graphics::Surface *surface = _bink.decodeNextFrame();

	if (!_texture) {
		_texture = _vm->_gfx->createTexture(surface);
	} else {
		_texture->update(surface);
	}
}

Subtitles::Subtitles(Myst3Engine *vm) :
		Window(),
		_vm(vm),
		_texture(0),
		_frame(-1) {
	_scaled = !_vm->isWideScreenModEnabled();
}

Subtitles::~Subtitles() {
	freeTexture();
}

void Subtitles::loadFontSettings(int32 id) {
	// Load font settings
	const ResourceDescription fontNums = _vm->getFileDescription("NUMB", id, 0, Archive::kNumMetadata);

	if (!fontNums.isValid())
		error("Unable to load font settings values");

	_fontSize = fontNums.getMiscData(0);
	_fontBold = fontNums.getMiscData(1);
	_surfaceHeight = fontNums.getMiscData(2);
	_singleLineTop = fontNums.getMiscData(3);
	_line1Top = fontNums.getMiscData(4);
	_line2Top = fontNums.getMiscData(5);
	_surfaceTop = fontNums.getMiscData(6);
	_fontCharsetCode = fontNums.getMiscData(7);

	if (_fontCharsetCode > 0) {
		_fontCharsetCode = 128; // The Japanese subtitles are encoded in CP 932 / Shift JIS
	}

	if (_vm->getGameLanguage() == Common::HE_ISR) {
		// The Hebrew subtitles are encoded in CP 1255, but the game data does not specify the appropriate encoding
		_fontCharsetCode = 177;
	}

	if (_fontCharsetCode < 0) {
		_fontCharsetCode = -_fontCharsetCode; // Negative values are GDI charset codes
	}

	ResourceDescription fontText = _vm->getFileDescription("TEXT", id, 0, Archive::kTextMetadata);

	if (!fontText.isValid())
		error("Unable to load font face");

	_fontFace = fontText.getTextData(0);
}

int32 Subtitles::checkOverridenId(int32 id) {
	// Subtitles may be overridden using a variable
	if (_vm->_state->getMovieOverrideSubtitles()) {
		id = _vm->_state->getMovieOverrideSubtitles();
		_vm->_state->setMovieOverrideSubtitles(0);
	}
	return id;
}

ResourceDescription Subtitles::loadText(int32 id, bool overriden) {
	ResourceDescription desc;
	if (overriden) {
		desc = _vm->getFileDescription("IMGR", 100000 + id, 0, Archive::kText);
	} else {
		desc = _vm->getFileDescription("", 100000 + id, 0, Archive::kText);
	}
	return desc;
}

void Subtitles::setFrame(int32 frame) {
	const Phrase *phrase = nullptr;

	for (uint i = 0; i < _phrases.size(); i++) {
		if (_phrases[i].frame > frame)
			break;

		phrase = &_phrases[i];
	}

	if (!phrase) {
		freeTexture();
		return;
	}

	if (phrase->frame == _frame) {
		return;
	}

	_frame = phrase->frame;

	drawToTexture(phrase);
}

void Subtitles::drawOverlay() {
	if (!_texture) return;

	Common::Rect screen = _vm->_gfx->viewport();
	Common::Rect bottomBorder = Common::Rect(Renderer::kOriginalWidth, _surfaceHeight);
	bottomBorder.translate(0, _surfaceTop);

	if (_vm->isWideScreenModEnabled()) {
		// Draw a black background to cover the main game frame
		_vm->_gfx->drawRect2D(Common::Rect(screen.width(), Renderer::kBottomBorderHeight), 0xFF000000);

		// Center the subtitles in the screen
		bottomBorder.translate((screen.width() - Renderer::kOriginalWidth) / 2, 0);
	}

	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);

	_vm->_gfx->drawTexturedRect2D(bottomBorder, textureRect, _texture);
}

Subtitles *Subtitles::create(Myst3Engine *vm, uint32 id) {
	Subtitles *s;

	if (vm->getPlatform() == Common::kPlatformXbox) {
		s = new MovieSubtitles(vm);
	} else {
		s = new FontSubtitles(vm);
	}

	s->loadFontSettings(1100);

	if (!s->loadSubtitles(id)) {
		delete s;
		return 0;
	}

	s->loadResources();

	return s;
}

void Subtitles::freeTexture() {
	if (_texture) {
		_vm->_gfx->freeTexture(_texture);
		_texture = nullptr;
	}
}

Common::Rect Subtitles::getPosition() const {
	Common::Rect screen = _vm->_gfx->viewport();

	Common::Rect frame;

	if (_vm->isWideScreenModEnabled()) {
		frame = Common::Rect(screen.width(), Renderer::kBottomBorderHeight);

		Common::Rect scenePosition = _vm->_scene->getPosition();
		int16 top = CLIP<int16>(screen.height() - frame.height(), 0, scenePosition.bottom);

		frame.translate(0, top);
	} else {
		frame = Common::Rect(screen.width(), screen.height() * Renderer::kBottomBorderHeight / Renderer::kOriginalHeight);
		frame.translate(screen.left, screen.top + screen.height() * (Renderer::kTopBorderHeight + Renderer::kFrameHeight) / Renderer::kOriginalHeight);
	}

	return frame;
}

Common::Rect Subtitles::getOriginalPosition() const {
	Common::Rect originalPosition = Common::Rect(Renderer::kOriginalWidth, Renderer::kBottomBorderHeight);
	originalPosition.translate(0, Renderer::kTopBorderHeight + Renderer::kFrameHeight);
	return originalPosition;
}

} // End of namespace Myst3
