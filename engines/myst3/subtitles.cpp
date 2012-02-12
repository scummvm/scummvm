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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/myst3/subtitles.h"
#include "engines/myst3/myst3.h"
#include "engines/myst3/state.h"

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
	_font(0) {
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


	const DirectorySubEntry *fontText = _vm->getFileDescription("TEXT", id, 0, DirectorySubEntry::kTextMetadata);

	if (!fontText)
		error("Unable to load font face");

	_fontFace = fontText->getTextData(0);

	const DirectorySubEntry *fontCharset = _vm->getFileDescription("CHAR", id, 0, DirectorySubEntry::kCursor);

	if (!fontCharset)
		error("Unable to load font charset");

	Common::MemoryReadStream *data = fontCharset->getData();
	data->read(_charset, sizeof(_charset));
	delete data;
}

void Subtitles::loadFont() {
	// Use the TTF font provided by the game if TTF support is available
#ifdef USE_FREETYPE2
	Common::SeekableReadStream *s = SearchMan.createReadStreamForMember("arir67w.ttf");
	if (s) {
		_font = Graphics::loadTTFFont(*s, _fontSize);
		delete s;
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
		uint8 c = 0;
		do {
			c = crypted->readByte() ^ key++;

			if (c >= 32)
				c = _charset[c - 32];

			_phrases[i].string += c;
		} while (c);
	}

	delete crypted;

	return true;
}

void Subtitles::createTexture() {
	// Create a surface to draw the subtitles on
	// Use RGB 565 to allow use of BDF fonts
	_surface = new Graphics::Surface();
	_surface->create(Renderer::kOriginalWidth, _surfaceHeight, Graphics::PixelFormat(2, 5, 6, 5, 0, 11, 5, 0, 0));

	_texture = _vm->_gfx->createTexture(_surface);
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
	memset(_surface->pixels, 0, _surface->pitch * _surface->h);
	font->drawString(_surface, phrase->string, 0, _singleLineTop, _surface->w, 0xFFFFFFFF, Graphics::kTextAlignCenter);

	// Update the texture
	_texture->update(_surface);
}

void Subtitles::drawOverlay() {
	Common::Rect textureRect = Common::Rect(_texture->width, _texture->height);
	Common::Rect bottomBorder = textureRect;
	bottomBorder.translate(0, _surfaceTop);

	_vm->_gfx->drawTexturedRect2D(bottomBorder, textureRect, _texture);
}

} /* namespace Myst3 */
