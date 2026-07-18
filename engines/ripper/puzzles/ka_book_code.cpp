/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of their respective copyright holders.
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
 */

#include "ripper/puzzles/ka_book_code.h"

#include "common/debug.h"
#include "common/file.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"
#include "image/pcx.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/media.h"
#include "ripper/modal_dialog.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const char *const kBackgroundMedia = "ka_puz.pcx";
static const char *const kKeyAudio = "ka_key.wav";
static const char *const kSuccessAudio = "li1_1_ve.wav";
static const char *const kFailureAudio = "li1_1_vf.wav";
static const uint16 kEscapeCommand = 0x1b;
static const uint16 kHelpCommand = 0x3b00;
static const uint kHelpSelectionTable = 0x1a6;
static const int kBackgroundY = 50;
static const int kTextY = 310;
static const int kCellX[7] = { 64, 82, 101, 119, 138, 156, 175 };
static const byte kTextColor = 255;
static const char kExpectedCode[] = "HC2021R";

} // End of anonymous namespace

KaBookCodePuzzle::KaBookCodePuzzle(RipperEngine *engine) : _engine(engine) {
}

bool KaBookCodePuzzle::loadAssets() {
	if (!_engine->getResources()->loadInterfaceBitmapFont("small.fnt", _font)) {
		warning("Ripper: could not load Ka book-code font");
		return false;
	}

	Common::File file;
	Image::PCXDecoder decoder;
	if (!file.open(Common::Path(kBackgroundMedia)) || !decoder.loadStream(file)) {
		warning("Ripper: could not decode Ka book-code background '%s'", kBackgroundMedia);
		return false;
	}
	const Graphics::Surface *surface = decoder.getSurface();
	if (!surface || surface->format.bytesPerPixel != 1 || surface->w <= 0 || surface->h <= 0)
		return false;
	_background.width = surface->w;
	_background.height = surface->h;
	_background.pixels.resize((uint32)surface->w * surface->h);
	for (int y = 0; y < surface->h; ++y)
		memcpy(_background.pixels.data() + y * surface->w,
			surface->getBasePtr(0, y), surface->w);
	const Graphics::Palette &palette = decoder.getPalette();
	_background.palette.resize(palette.size() * 3);
	if (!_background.palette.empty())
		memcpy(_background.palette.data(), palette.data(), _background.palette.size());
	debugC(1, kDebugPuzzles,
		"Ripper: loaded Ka book-code puzzle background='%s' size=%ux%u colors=%u glyphs=%u",
		kBackgroundMedia, _background.width, _background.height, palette.size(),
		_font.glyphs.size());
	return true;
}

uint KaBookCodePuzzle::measureText(const Common::String &text) const {
	uint width = 0;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			width += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
				character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		width += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
	return width;
}

void KaBookCodePuzzle::drawText(byte *screen, uint pitch, int x, int y,
		const Common::String &text, byte color) const {
	int drawX = x;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = (byte)text[i];
		if (character == ' ') {
			drawX += _font.spaceWidth;
			continue;
		}
		if (character < _font.firstCharacter ||
				character >= _font.firstCharacter + _font.glyphs.size())
			continue;
		const BitmapFontGlyph &glyph = _font.glyphs[character - _font.firstCharacter];
		for (uint glyphY = 0; glyphY < glyph.height; ++glyphY) {
			for (uint glyphX = 0; glyphX < glyph.width; ++glyphX) {
				const byte pixel = _font.pixels[glyph.pixelOffset + glyphY * glyph.width + glyphX];
				const int targetX = drawX + glyph.xOffset + glyphX;
				const int targetY = y + glyph.yOffset + glyphY;
				if (pixel != _font.transparentColor && targetX >= 0 && targetX < 640 &&
						targetY >= 0 && targetY < 400)
					screen[targetY * pitch + targetX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void KaBookCodePuzzle::drawPrompt(const Common::String &typedCode) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}
	byte *pixels = (byte *)screen->getPixels();
	for (uint y = 0; y < _background.height && y + kBackgroundY < 400; ++y) {
		const uint width = MIN<uint>(_background.width, 640);
		memcpy(screen->getBasePtr(0, y + kBackgroundY),
			_background.pixels.data() + y * _background.width, width);
	}
	for (uint i = 0; i < typedCode.size() && i < ARRAYSIZE(kCellX); ++i) {
		const Common::String character(typedCode.c_str() + i, 1);
		const int x = kCellX[i] + (15 - (int)measureText(character)) / 2;
		drawText(pixels, screen->pitch, x, kTextY, character, kTextColor);
	}
	g_system->unlockScreen();
	if (_background.palette.size() >= 256 * 3)
		g_system->getPaletteManager()->setPalette(_background.palette.data(), 0, 256);
	g_system->updateScreen();
}

KaBookCodePuzzle::Result KaBookCodePuzzle::run() {
	// RunKaBookCodeEntryPrompt at 0x2bc33 owns the KA_PUZ.PCX display,
	// KA_KEY.WAV feedback, seven input cells, and HC2021R comparison. Its caller
	// in RunKaDialogueScene only consumes the returned solved/failed state.
	if (!loadAssets())
		return kLoadFailed;

	Common::String typedCode;
	_engine->getCursor()->setVisible(false);
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	drawPrompt(typedCode);
	debugC(1, kDebugPuzzles,
		"Ripper: entered Ka book-code puzzle expectedLength=7 helpTable=0x%x",
		kHelpSelectionTable);

	bool solved = false;
	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			uint16 command = _engine->getInput()->consumeKey();
			if (command == kEscapeCommand) {
				active = false;
				break;
			}
			if (command == kHelpCommand) {
				_engine->getModalDialog()->run(kHelpSelectionTable);
				drawPrompt(typedCode);
				continue;
			}
			if (command == 8 && !typedCode.empty()) {
				_engine->getMedia()->playSoundEffect(kKeyAudio, _keyHandle);
				typedCode.deleteLastChar();
				drawPrompt(typedCode);
				continue;
			}
			if (command >= 'a' && command <= 'z')
				command -= 'a' - 'A';
			if (typedCode.size() >= ARRAYSIZE(kCellX) ||
					!((command >= '0' && command <= '9') ||
					(command >= 'A' && command <= 'Z')))
				continue;
			_engine->getMedia()->playSoundEffect(kKeyAudio, _keyHandle);
			typedCode += (char)command;
			drawPrompt(typedCode);
			if (typedCode == kExpectedCode) {
				solved = true;
				active = false;
				break;
			}
		}
		g_system->delayMillis(10);
	}

	_engine->getMedia()->stopSoundEffect(_keyHandle);
	if (!_engine->shouldQuit()) {
		const char *resultAudio = solved ? kSuccessAudio : kFailureAudio;
		if (!_engine->getMedia()->play(resultAudio, true)) {
			warning("Ripper: could not play Ka book-code result audio '%s'", resultAudio);
			return kLoadFailed;
		}
	}
	debugC(1, kDebugPuzzles,
		"Ripper: left Ka book-code puzzle result=%s typedLength=%u",
		solved ? "solved" : "failed", typedCode.size());
	return solved ? kSolved : kFailed;
}

} // End of namespace Ripper
