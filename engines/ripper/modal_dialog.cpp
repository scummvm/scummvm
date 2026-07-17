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

#include "ripper/modal_dialog.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/input.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

static const uint kModalSkinFrameCount = 15;
static const uint kModalFrameTileCount = 9;
static const uint kModalTitleResourceId = 0x42;
static const uint kModalBodyWidth = 300;
static const uint kModalMaximumRows = 10;
static const int kModalHeadingTopPadding = 17;
static const int kModalBottomPadding = 5;
static const int kModalLeftPadding = 5;
static const int kModalRightPadding = 5;
static const int kModalRowHeight = 14;
static const byte kModalBackgroundColor = 4;
static const byte kModalHeadingColor = 250;
static const byte kModalTitleColor = 248;
static const byte kModalTextColor = 251;

} // End of anonymous namespace

ModalDialogManager::ModalDialogManager(RipperEngine *engine) :
		_engine(engine), _initialized(false) {
}

bool ModalDialogManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!resources.loadGameText(_gameText))
		return false;

	_skin.clear();
	for (uint frameIndex = 0; frameIndex < kModalSkinFrameCount; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("menub%u", frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		_skin.push_back(Common::move(sequence.frames.front()));
	}

	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized modal text dialogs skin=MENUB frames=%u font=small.fnt strings=%u",
		_skin.size(), _gameText.size());
	return true;
}

const Common::String &ModalDialogManager::resourceString(uint resourceId) const {
	static const Common::String empty;
	if (resourceId == 0 || resourceId > _gameText.size())
		return empty;
	return _gameText[resourceId - 1];
}

bool ModalDialogManager::captureDisplay() {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || screen->w != 640 || screen->h != 400) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	_savedPixels.resize(640 * 400);
	for (int y = 0; y < 400; ++y)
		memcpy(_savedPixels.data() + y * 640, screen->getBasePtr(0, y), 640);
	g_system->unlockScreen();
	_savedPalette.resize(256 * 3);
	g_system->getPaletteManager()->grabPalette(_savedPalette.data(), 0, 256);
	return true;
}

void ModalDialogManager::restoreDisplay() {
	if (_savedPixels.size() != 640 * 400 || _savedPalette.size() != 256 * 3)
		return;

	g_system->copyRectToScreen(_savedPixels.data(), 640, 0, 0, 640, 400);
	g_system->getPaletteManager()->setPalette(_savedPalette.data(), 0, 256);
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	_savedPixels.clear();
	_savedPalette.clear();
}

uint ModalDialogManager::measureText(const Common::String &text) const {
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

void ModalDialogManager::wrapText(const Common::String &text, uint maxWidth,
		Common::Array<Common::String> &lines) const {
	lines.clear();
	Common::String line;
	Common::String word;
	for (uint i = 0; i <= text.size(); ++i) {
		const char character = i < text.size() ? text[i] : '\n';
		if (character == '\r')
			continue;
		if (character != ' ' && character != '\n') {
			word += character;
			continue;
		}

		if (!word.empty()) {
			const Common::String candidate = line.empty() ? word : line + " " + word;
			if (!line.empty() && measureText(candidate) > maxWidth) {
				lines.push_back(line);
				line = word;
			} else {
				line = candidate;
			}
			word.clear();
		}
		if (character == '\n') {
			lines.push_back(line);
			line.clear();
		}
	}
	if (lines.empty())
		lines.push_back(Common::String());
}

void ModalDialogManager::drawText(byte *screen, uint pitch, int x, int y,
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
				if (pixel != _font.transparentColor)
					screen[(y + glyph.yOffset + glyphY) * pitch + drawX +
						glyph.xOffset + glyphX] = color;
			}
		}
		drawX += glyph.xOffset + glyph.width + _font.characterSpacing;
	}
}

void ModalDialogManager::drawBitmap(byte *screen, uint pitch,
		const BitmapAssetFrame &bitmap, int x, int y) const {
	for (uint row = 0; row < bitmap.height; ++row) {
		for (uint column = 0; column < bitmap.width; ++column) {
			const byte pixel = bitmap.pixels[row * bitmap.width + column];
			if (pixel != bitmap.transparentColor)
				screen[(y + row) * pitch + x + column] = pixel;
		}
	}
}

void ModalDialogManager::drawFrame(byte *screen, uint pitch,
		const Common::Rect &bounds) const {
	if (_skin.size() < kModalFrameTileCount)
		return;
	const int tileWidth = _skin[0].width;
	const int tileHeight = _skin[0].height;
	const int columns = (bounds.width() + tileWidth - 1) / tileWidth;
	const int rows = (bounds.height() + tileHeight - 1) / tileHeight;
	for (int column = 0; column < columns; ++column) {
		for (int row = 0; row < rows; ++row) {
			const uint columnBand = column == 0 ? 0 : (column == columns - 1 ? 2 : 1);
			const uint rowBand = row == 0 ? 0 : (row == rows - 1 ? 2 : 1);
			const BitmapAssetFrame &tile = _skin[rowBand * 3 + columnBand];
			const int x = column == columns - 1 ? bounds.right - tile.width :
				bounds.left + column * tileWidth;
			const int y = row == rows - 1 ? bounds.bottom - tile.height :
				bounds.top + row * tileHeight;
			drawBitmap(screen, pitch, tile, x, y);
		}
	}
}

void ModalDialogManager::drawDialog(const Common::String &title,
		const Common::Array<Common::String> &lines, uint firstVisible,
		uint visibleRows, const Common::Rect &bounds) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen->getBasePtr(bounds.left, y), kModalBackgroundColor, bounds.width());
	for (int y = bounds.top + 2; y < bounds.top + kModalHeadingTopPadding; ++y)
		memset(screen->getBasePtr(bounds.left + kModalLeftPadding, y),
			kModalHeadingColor, bounds.width() - kModalLeftPadding - kModalRightPadding);

	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, bounds);
	const int titleX = bounds.left + (bounds.width() - measureText(title)) / 2;
	drawText(pixels, screen->pitch, titleX,
		bounds.top + (kModalHeadingTopPadding - _font.lineHeight) / 2,
		title, kModalTitleColor);
	for (uint row = 0; row < visibleRows; ++row) {
		const uint lineIndex = firstVisible + row;
		if (lineIndex >= lines.size())
			break;
		drawText(pixels, screen->pitch, bounds.left + kModalLeftPadding,
			bounds.top + kModalHeadingTopPadding + row * kModalRowHeight +
				(kModalRowHeight - _font.lineHeight) / 2,
			lines[lineIndex], kModalTextColor);
	}
	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

bool ModalDialogManager::run(uint bodyResourceId) {
	const Common::String &title = resourceString(kModalTitleResourceId);
	const Common::String &body = resourceString(bodyResourceId);
	if (!_initialized || title.empty() || body.empty() || !captureDisplay()) {
		warning("Ripper: could not present modal text resource=%u", bodyResourceId);
		return false;
	}

	Common::Array<Common::String> lines;
	wrapText(body, kModalBodyWidth, lines);
	const uint visibleRows = MIN<uint>(lines.size(), kModalMaximumRows);
	const int width = kModalBodyWidth + kModalLeftPadding + kModalRightPadding;
	const int height = kModalHeadingTopPadding + visibleRows * kModalRowHeight +
		kModalBottomPadding;
	const int left = (640 - width) / 2;
	const int top = (400 - height) / 2;
	const Common::Rect bounds(left, top, left + width, top + height);
	uint firstVisible = 0;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	drawDialog(title, lines, firstVisible, visibleRows, bounds);
	debugC(1, kDebugScene,
		"Ripper: entered modal text dialog resource=%u title='%s' lines=%u bounds=%d,%d,%d,%d",
		bodyResourceId, title.c_str(), lines.size(), bounds.left, bounds.top,
		bounds.width(), bounds.height());

	bool active = true;
	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		bool redraw = false;
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			if (command == 0x1b || command == 0x0d) {
				active = false;
				break;
			}
			const uint maximumFirst = lines.size() > visibleRows ?
				lines.size() - visibleRows : 0;
			uint nextFirst = firstVisible;
			switch (command) {
			case 0x4700:
				nextFirst = 0;
				break;
			case 0x4800:
				nextFirst = firstVisible > 0 ? firstVisible - 1 : 0;
				break;
			case 0x4900:
				nextFirst = firstVisible > visibleRows ? firstVisible - visibleRows : 0;
				break;
			case 0x4f00:
				nextFirst = maximumFirst;
				break;
			case 0x5000:
				nextFirst = MIN(firstVisible + 1, maximumFirst);
				break;
			case 0x5100:
				nextFirst = MIN(firstVisible + visibleRows, maximumFirst);
				break;
			default:
				break;
			}
			if (nextFirst != firstVisible) {
				firstVisible = nextFirst;
				redraw = true;
				debugC(2, kDebugScene,
					"Ripper: scrolled modal text resource=%u firstLine=%u visibleRows=%u",
					bodyResourceId, firstVisible, visibleRows);
			}
		}
		_engine->getInput()->publishMouseState();
		if (redraw)
			drawDialog(title, lines, firstVisible, visibleRows, bounds);
		g_system->delayMillis(10);
	}

	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	debugC(1, kDebugScene,
		"Ripper: exited modal text dialog resource=%u", bodyResourceId);
	return true;
}

} // End of namespace Ripper
