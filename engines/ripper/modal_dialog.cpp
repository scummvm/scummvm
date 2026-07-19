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
#include "ripper/script.h"

namespace Ripper {

namespace {

static const uint kModalSkinFrameCount = 15;
static const uint kWacModalSkinFrameCount = 16;
static const uint kModalFrameTileCount = 9;
static const uint kModalScrollStartFrame = 9;
static const uint kModalScrollEndFrame = 10;
static const uint kModalScrollTrackFrame = 14;
static const uint kModalTitleResourceId = 0x42;
static const uint kModalMaximumRows = 10;
static const int kModalHeadingTopPadding = 17;
static const int kModalBottomPadding = 5;
static const int kModalLeftPadding = 5;
static const int kModalRightPadding = 5;
static const int kModalScrollStartInset = 16;
static const int kModalScrollEndInset = 4;
static const int kModalScrollEdgeInset = 4;
static const uint kModalWidth = 300;
static const int kModalRowHeight = 14;
static const byte kModalBackgroundColor = 253;
static const byte kModalHeadingColor = 255;
static const byte kModalTitleColor = 254;
static const byte kModalTextColor = 4;
static const int kWacModalHeadingTopPadding = 20;
static const int kWacModalBottomPadding = 6;
static const int kWacModalLeftPadding = 5;
static const int kWacModalRightPadding = 20;
static const byte kWacModalBackgroundColor = 4;
static const byte kWacModalTitleColor = 248;
static const byte kWacModalTextColor = 251;
static const uint kModalCursor = 16;
static const int kTextEntryLeft = 228;
static const int kTextEntryTop = 312;
static const int kTextEntryWidth = 194;
static const int kTextEntryHeight = 20;
static const int kTextEntryPadding = 5;
static const uint32 kTextEntryCaretBlinkMillis = 500;

} // End of anonymous namespace

ModalDialogManager::ModalDialogManager(RipperEngine *engine) :
		_engine(engine), _textEntryMaximumLength(0), _textEntryHelpResourceId(0),
		_textEntryFirstVisible(0), _textEntryCursorPosition(0),
		_textEntryNextCaretMillis(0), _textEntryOverwrite(false),
		_textEntryCaretVisible(false), _textEntryActive(false),
		_textEntryRestoreCursor(false), _initialized(false) {
}

bool ModalDialogManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!resources.loadGameText(_gameText))
		return false;

	_skin.clear();
	_wacSkin.clear();
	_modalPalette.clear();
	for (uint frameIndex = 0; frameIndex < kModalSkinFrameCount; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("menub%u", frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		_skin.push_back(Common::move(sequence.frames.front()));
		if (_modalPalette.empty() && _skin.back().palette.size() >= 256 * 3)
			_modalPalette = _skin.back().palette;
	}
	for (uint frameIndex = 0; frameIndex < kWacModalSkinFrameCount; ++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("wacmnu%u", frameIndex), sequence) ||
				sequence.frames.empty())
			return false;
		_wacSkin.push_back(Common::move(sequence.frames.front()));
	}

	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized modal text dialogs menubFrames=%u wacmnuFrames=%u font=small.fnt strings=%u",
		_skin.size(), _wacSkin.size(), _gameText.size());
	debugC(3, kDebugScene,
		"Ripper: modal text font first=%u glyphs=%u lineHeight=%u spacing=%u spaceWidth=%u "
		"transparent=%u paletteBytes=%u",
		_font.firstCharacter, _font.glyphs.size(), _font.lineHeight,
		_font.characterSpacing, _font.spaceWidth, _font.transparentColor,
		_modalPalette.size());
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

void ModalDialogManager::applyModalPalette() {
	if (_modalPalette.size() < 256 * 3)
		return;

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	// InitializeSharedPresentationTemplates at 0x1196f captures these ranges
	// from the MENUB palette through CaptureSharedDisplayPalettePatch at
	// 0x205a9. ApplySharedDisplayPalettePatch at 0x205d0 restores them for
	// chooser text and frame pixels without replacing the active scene palette.
	memset(palette, 0, 3);
	memcpy(palette + 4 * 3, _modalPalette.data() + 4 * 3, 6 * 3);
	memcpy(palette + 246 * 3, _modalPalette.data() + 246 * 3, 10 * 3);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
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
		const Common::Rect &bounds, PresentationStyle style) const {
	const Common::Array<BitmapAssetFrame> &skin =
		style == kWacPresentation ? _wacSkin : _skin;
	if (skin.size() < kModalFrameTileCount)
		return;
	const int tileWidth = skin[0].width;
	const int tileHeight = skin[0].height;
	const int columns = (bounds.width() + tileWidth - 1) / tileWidth;
	const int rows = (bounds.height() + tileHeight - 1) / tileHeight;
	int y = bounds.top;
	for (int row = 0; row < rows; ++row) {
		int x = bounds.left;
		const BitmapAssetFrame *lastTile = nullptr;
		for (int column = 0; column < columns; ++column) {
			const uint columnBand = column == 0 ? 0 : (column == columns - 1 ? 2 : 1);
			const uint rowBand = row == 0 ? 0 : (row == rows - 1 ? 2 : 1);
			// RIPPER stores bitmap dimensions and presentation coordinates in
			// vertical/horizontal order. Translating ResolveChooserFrameTileIndex
			// at 0x55250 to screen x/y makes MENUB0..8 row-major.
			const BitmapAssetFrame &tile = skin[rowBand * 3 + columnBand];
			drawBitmap(screen, pitch, tile, x, y);
			lastTile = &tile;
			// TileChooserControlFrame at 0x54fbe uses the selected tile's
			// dimensions and snaps the last column to the control's right edge.
			if (column == columns - 2)
				x = bounds.right - tile.width;
			else
				x += tile.width;
		}
		if (!lastTile)
			continue;
		// The original performs the matching bottom-edge snap after the
		// penultimate row has been tiled.
		if (row == rows - 2)
			y = bounds.bottom - lastTile->height;
		else
			y += lastTile->height;
	}
}

void ModalDialogManager::drawOverflowBar(byte *screen, uint pitch,
		const Common::Rect &bounds, PresentationStyle style) const {
	const Common::Array<BitmapAssetFrame> &skin =
		style == kWacPresentation ? _wacSkin : _skin;
	if (skin.size() <= kModalScrollTrackFrame)
		return;

	const BitmapAssetFrame &start = skin[kModalScrollStartFrame];
	const BitmapAssetFrame &end = skin[kModalScrollEndFrame];
	const BitmapAssetFrame &track = skin[kModalScrollTrackFrame];
	const int startY = bounds.top + kModalScrollStartInset;
	const int endY = bounds.bottom - kModalScrollEndInset - end.height;
	for (int y = startY + start.height; y <= endY; y += track.height)
		drawBitmap(screen, pitch, track,
			bounds.right - kModalScrollEdgeInset - track.width, y);
	drawBitmap(screen, pitch, start,
		bounds.right - kModalScrollEdgeInset - start.width, startY);
	drawBitmap(screen, pitch, end,
		bounds.right - kModalScrollEdgeInset - end.width, endY);
}

void ModalDialogManager::drawDialog(const Common::String &title,
		const Common::Array<Common::String> &lines, uint firstVisible,
		uint visibleRows, const Common::Rect &bounds,
		PresentationStyle style) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	const bool wacStyle = style == kWacPresentation;
	const int headingHeight = title.empty() ?
		(wacStyle ? kWacModalBottomPadding : kModalBottomPadding) :
		(wacStyle ? kWacModalHeadingTopPadding : kModalHeadingTopPadding);
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kModalLeftPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kModalRightPadding;
	const byte titleColor = wacStyle ? kWacModalTitleColor : kModalTitleColor;
	const byte textColor = wacStyle ? kWacModalTextColor : kModalTextColor;
	if (wacStyle) {
		// ServiceWacSceneInputAction at 0x21eef uses the tertiary WACMNU
		// chooser template. Its bitmap tiles retain the textured heading and
		// frame over the black client field.
		for (int y = bounds.top; y < bounds.bottom; ++y)
			memset(screen->getBasePtr(bounds.left, y), kWacModalBackgroundColor,
				bounds.width());
		drawFrame(pixels, screen->pitch, bounds, style);
	} else {
		drawFrame(pixels, screen->pitch, bounds, style);
		// RebuildChooserControlVisual draws the tiled frame before the row renderer
		// and DrawPromptChooserTemplateLabelCallback repaint the framed interior.
		for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
			memset(screen->getBasePtr(bounds.left + leftPadding, y),
				kModalBackgroundColor, bounds.width() - leftPadding - rightPadding);
		if (!title.empty()) {
			for (int y = bounds.top + 2; y < bounds.top + headingHeight; ++y)
				memset(screen->getBasePtr(bounds.left + leftPadding, y),
					kModalHeadingColor, bounds.width() - leftPadding - rightPadding);
		}
	}
	if (!title.empty()) {
		const int contentWidth = bounds.width() - leftPadding - rightPadding;
		const int titleX = bounds.left + leftPadding +
			(contentWidth - measureText(title)) / 2;
		drawText(pixels, screen->pitch, titleX,
			bounds.top + (headingHeight - _font.lineHeight) / 2,
			title, titleColor);
	}
	for (uint row = 0; row < visibleRows; ++row) {
		const uint lineIndex = firstVisible + row;
		if (lineIndex >= lines.size())
			break;
		drawText(pixels, screen->pitch, bounds.left + leftPadding,
			bounds.top + headingHeight + row * kModalRowHeight +
				(kModalRowHeight - _font.lineHeight) / 2,
			lines[lineIndex], textColor);
	}
	if (lines.size() > visibleRows)
		drawOverflowBar(pixels, screen->pitch, bounds, style);
	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

bool ModalDialogManager::drawRetainedTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, uint firstVisible, uint &maximumFirstVisible,
		uint &visibleRows) {
	const Common::String &body = resourceString(bodyResourceId);
	if (!_initialized || body.empty() || bounds.width() <=
			kModalLeftPadding + kModalRightPadding ||
			bounds.height() <= kModalBottomPadding * 2) {
		warning("Ripper: could not draw retained text panel resource=%u", bodyResourceId);
		return false;
	}

	Common::Array<Common::String> lines;
	wrapText(body, bounds.width() - kModalLeftPadding - kModalRightPadding, lines);
	visibleRows = MAX<uint>(1,
		(bounds.height() - kModalBottomPadding * 2) / kModalRowHeight);
	maximumFirstVisible = lines.size() > visibleRows ? lines.size() - visibleRows : 0;
	firstVisible = MIN(firstVisible, maximumFirstVisible);
	applyModalPalette();
	drawDialog(Common::String(), lines, firstVisible, visibleRows, bounds,
		kMenubPresentation);
	debugC(2, kDebugScene,
		"Ripper: drew retained text panel resource=%u lines=%u firstLine=%u visibleRows=%u bounds=%d,%d,%d,%d",
		bodyResourceId, lines.size(), firstVisible, visibleRows,
		bounds.left, bounds.top, bounds.width(), bounds.height());
	return true;
}

void ModalDialogManager::drawTextEntry(const Common::String &prompt,
		const Common::String &text, uint firstVisible, uint cursorPosition,
		bool caretVisible, const Common::Rect &bounds) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, bounds, kMenubPresentation);
	for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
		memset(screen->getBasePtr(bounds.left + kTextEntryPadding, y),
			kModalBackgroundColor, bounds.width() - kTextEntryPadding * 2);

	const int textTop = bounds.top + (bounds.height() - _font.lineHeight) / 2;
	int textLeft = bounds.left + kTextEntryPadding;
	if (!prompt.empty()) {
		drawText(pixels, screen->pitch, textLeft, textTop, prompt, kModalTextColor);
		textLeft += measureText(prompt) + _font.spaceWidth;
	}
	const uint availableWidth = bounds.right - kTextEntryPadding - textLeft;
	uint endVisible = firstVisible;
	while (endVisible < text.size()) {
		const Common::String candidate =
			text.substr(firstVisible, endVisible - firstVisible + 1);
		if (measureText(candidate) > availableWidth)
			break;
		++endVisible;
	}
	const Common::String visible = text.substr(firstVisible,
		endVisible - firstVisible);
	drawText(pixels, screen->pitch, textLeft, textTop, visible, kModalTextColor);
	if (caretVisible && cursorPosition >= firstVisible && cursorPosition <= endVisible) {
		const int caretLeft = textLeft + measureText(text.substr(firstVisible,
			cursorPosition - firstVisible));
		drawText(pixels, screen->pitch, caretLeft, textTop, "_", kModalTextColor);
	}

	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

uint ModalDialogManager::textEntryCursorFromPoint(const Common::String &text,
		uint firstVisible, int x, const Common::Rect &bounds) const {
	const int relativeX = x - bounds.left - kTextEntryPadding;
	if (relativeX <= 0)
		return firstVisible;
	for (uint position = firstVisible; position < text.size(); ++position) {
		const uint left = measureText(text.substr(firstVisible,
			position - firstVisible));
		const uint right = measureText(text.substr(firstVisible,
			position - firstVisible + 1));
		if ((uint)relativeX < left + (right - left) / 2)
			return position;
		if (right >= (uint)bounds.width() - kTextEntryPadding * 2)
			return position + 1;
	}
	return text.size();
}

void ModalDialogManager::updateTextEntryFirstVisible(const Common::Rect &bounds) {
	int textLeft = bounds.left + kTextEntryPadding;
	if (!_textEntryPrompt.empty())
		textLeft += measureText(_textEntryPrompt) + _font.spaceWidth;
	const uint availableWidth = bounds.right - kTextEntryPadding - textLeft;
	const uint caretWidth = measureText("_");
	if (_textEntryCursorPosition < _textEntryFirstVisible)
		_textEntryFirstVisible = _textEntryCursorPosition;
	while (_textEntryFirstVisible < _textEntryCursorPosition &&
			measureText(_textEntryText.substr(_textEntryFirstVisible,
				_textEntryCursorPosition - _textEntryFirstVisible)) + caretWidth >
				availableWidth)
		++_textEntryFirstVisible;
	while (_textEntryFirstVisible != 0 &&
			measureText(_textEntryText.substr(_textEntryFirstVisible - 1,
				_textEntryCursorPosition - _textEntryFirstVisible + 1)) + caretWidth <=
				availableWidth)
		--_textEntryFirstVisible;
}

bool ModalDialogManager::beginTextEntry(const Common::String &prompt,
		uint maximumLength, uint helpResourceId, const char *source) {
	if (!_initialized || _textEntryActive) {
		warning("Ripper: could not begin scene text request source='%s' active=%d",
			source, _textEntryActive);
		return false;
	}
	_textEntryPrompt = prompt;
	_textEntryText.clear();
	_textEntrySource = source;
	_textEntryMaximumLength = maximumLength;
	_textEntryHelpResourceId = helpResourceId;
	_textEntryFirstVisible = 0;
	_textEntryCursorPosition = 0;
	_textEntryOverwrite = false;
	_textEntryCaretVisible = true;
	_textEntryNextCaretMillis = g_system->getMillis() + kTextEntryCaretBlinkMillis;
	_textEntryRestoreCursor = _engine->getCursor()->isVisible();
	_textEntryActive = true;
	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->update(kModalCursor);
	_engine->getCursor()->setVisible(true);
	applyModalPalette();
	const Common::Rect bounds(kTextEntryLeft, kTextEntryTop,
		kTextEntryLeft + kTextEntryWidth, kTextEntryTop + kTextEntryHeight);
	drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
		_textEntryCursorPosition, _textEntryCaretVisible, bounds);
	debugC(1, kDebugScene,
		"Ripper: entered scene text request source='%s' control=0x4e2 bounds=%d,%d,%d,%d maximumLength=%u helpResource=%u",
		source, bounds.left, bounds.top, bounds.width(), bounds.height(),
		maximumLength, helpResourceId);
	return true;
}

void ModalDialogManager::finishTextEntry(Common::String &text) {
	text = _textEntryText;
	_textEntryActive = false;
	_engine->getInput()->discardMouseTransitions();
	_engine->getCursor()->setVisible(_textEntryRestoreCursor);
}

ModalDialogManager::TextEntryResult ModalDialogManager::serviceTextEntry(
		Common::String &text) {
	if (!_textEntryActive)
		return kTextEntryFailed;
	if (_engine->getInput()->pollEvents()) {
		_engine->quitGame();
		finishTextEntry(text);
		return kTextEntryFailed;
	}

	const Common::Rect bounds(kTextEntryLeft, kTextEntryTop,
		kTextEntryLeft + kTextEntryWidth, kTextEntryTop + kTextEntryHeight);
	TextEntryResult result = kTextEntryPending;
	bool edited = false;
	while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			debugC(3, kDebugInput,
				"Ripper: scene text request command=0x%04x cursor=%u length=%u overwrite=%d",
				command, _textEntryCursorPosition, _textEntryText.size(),
				_textEntryOverwrite);
			if (command == 0x0d) {
				result = kTextEntryAccepted;
				break;
			}
			if (command == 0x1b) {
				result = kTextEntryCancelled;
				break;
			}
			if (command == 0x3b00) {
				if (!_engine->getModalDialog()->run(_textEntryHelpResourceId, true)) {
					result = kTextEntryFailed;
					break;
				}
				applyModalPalette();
				continue;
			}

			const uint previousLength = _textEntryText.size();
			switch (command) {
			case 0x08:
				if (_textEntryCursorPosition != 0) {
					_textEntryText.deleteChar(--_textEntryCursorPosition);
					edited = true;
				}
				break;
			case 0x4700:
				_textEntryCursorPosition = 0;
				edited = true;
				break;
			case 0x4b00:
				if (_textEntryCursorPosition != 0) {
					--_textEntryCursorPosition;
					edited = true;
				}
				break;
			case 0x4d00:
				if (_textEntryCursorPosition < _textEntryText.size()) {
					++_textEntryCursorPosition;
					edited = true;
				}
				break;
			case 0x4f00:
				_textEntryCursorPosition = _textEntryText.size();
				edited = true;
				break;
			case 0x5200:
				_textEntryOverwrite = !_textEntryOverwrite;
				break;
			case 0x5300:
				if (_textEntryCursorPosition < _textEntryText.size()) {
					_textEntryText.deleteChar(_textEntryCursorPosition);
					edited = true;
				}
				break;
			default:
				if (command <= 0xff && (command == ' ' ||
						(command >= _font.firstCharacter &&
						command < _font.firstCharacter + _font.glyphs.size()))) {
					if (_textEntryOverwrite &&
							_textEntryCursorPosition < _textEntryText.size()) {
						_textEntryText.setChar((char)command,
							_textEntryCursorPosition++);
						edited = true;
					} else if (_textEntryText.size() < _textEntryMaximumLength) {
						_textEntryText.insertChar((char)command,
							_textEntryCursorPosition++);
						edited = true;
					}
				}
				break;
			}
			if (_textEntryText.size() != previousLength)
				debugC(2, kDebugInput,
					"Ripper: scene text request edited length=%u cursor=%u",
					_textEntryText.size(), _textEntryCursorPosition);
	}

	const MouseState mouse = _engine->getInput()->publishMouseState();
	_engine->getCursor()->update(kModalCursor);
	if ((mouse.pressed & kMouseButtonLeft) != 0 && bounds.contains(mouse.position)) {
		_textEntryCursorPosition = textEntryCursorFromPoint(_textEntryText,
			_textEntryFirstVisible, mouse.position.x, bounds);
		edited = true;
		debugC(3, kDebugInput,
			"Ripper: scene text request caret point=%d,%d cursor=%u firstVisible=%u",
			mouse.position.x, mouse.position.y, _textEntryCursorPosition,
			_textEntryFirstVisible);
	}

	const uint32 now = g_system->getMillis();
	if (now >= _textEntryNextCaretMillis) {
		_textEntryCaretVisible = !_textEntryCaretVisible;
		_textEntryNextCaretMillis = now + kTextEntryCaretBlinkMillis;
	}
	if (edited) {
		_textEntryCaretVisible = true;
		_textEntryNextCaretMillis = now + kTextEntryCaretBlinkMillis;
	}
	updateTextEntryFirstVisible(bounds);
	if (result == kTextEntryPending) {
		applyModalPalette();
		drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
			_textEntryCursorPosition, _textEntryCaretVisible, bounds);
		return result;
	}

	const Common::String source = _textEntrySource;
	const uint length = _textEntryText.size();
	finishTextEntry(text);
	debugC(1, kDebugScene,
		"Ripper: exited scene text request source='%s' result=%d length=%u",
		source.c_str(), result, length);
	return result;
}

bool ModalDialogManager::run(uint bodyResourceId, bool retainSceneCursorRegions,
		PresentationStyle style) {
	const Common::String &title = resourceString(kModalTitleResourceId);
	const Common::String &body = resourceString(bodyResourceId);
	if (title.empty() || body.empty()) {
		warning("Ripper: could not present modal text resource=%u", bodyResourceId);
		return false;
	}
	return runTextInternal(title, body, bodyResourceId, "game text",
		retainSceneCursorRegions, style);
}

bool ModalDialogManager::runText(const Common::String &title,
		const Common::String &body, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style) {
	return runTextInternal(title, body, 0, source, retainSceneCursorRegions,
		style);
}

bool ModalDialogManager::runTextInternal(const Common::String &title,
		const Common::String &body, uint bodyResourceId, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style) {
	if (!_initialized || title.empty() || !captureDisplay()) {
		warning("Ripper: could not present modal text source='%s'", source);
		return false;
	}

	const bool wacStyle = style == kWacPresentation;
	const int headingTopPadding = wacStyle ?
		kWacModalHeadingTopPadding : kModalHeadingTopPadding;
	const int bottomPadding = wacStyle ? kWacModalBottomPadding : kModalBottomPadding;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kModalLeftPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kModalRightPadding;
	Common::Array<Common::String> lines;
	wrapText(body, kModalWidth - leftPadding - rightPadding, lines);
	const uint visibleRows = MIN<uint>(lines.size(), kModalMaximumRows);
	const int width = kModalWidth;
	const int height = headingTopPadding + visibleRows * kModalRowHeight +
		bottomPadding;
	const int left = (640 - width) / 2;
	const int top = (400 - height) / 2;
	const Common::Rect bounds(left, top, left + width, top + height);
	uint firstVisible = 0;
	int modalCursorRegion = -1;
	Common::Point cursorPoint = _engine->getInput()->peekMouseState().position;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	if (!wacStyle)
		applyModalPalette();
	drawDialog(title, lines, firstVisible, visibleRows, bounds, style);
	debugC(1, kDebugScene,
		"Ripper: entered modal text dialog source='%s' resource=%u style=%s title='%s' lines=%u bounds=%d,%d,%d,%d",
		source, bodyResourceId, wacStyle ? "wacmnu" : "menub", title.c_str(),
		lines.size(), bounds.left, bounds.top, bounds.width(), bounds.height());

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
					"Ripper: scrolled modal text source='%s' resource=%u firstLine=%u visibleRows=%u",
					source, bodyResourceId, firstVisible, visibleRows);
			}
		}
		const MouseState mouse = _engine->getInput()->publishMouseState();
		cursorPoint = mouse.position;
		const bool nextModalCursorActive = !retainSceneCursorRegions ||
			bounds.contains(mouse.position);
		if (nextModalCursorActive)
			_engine->getCursor()->update(kModalCursor);
		else
			_engine->getScripts()->updateModalSceneCursor(mouse.position);
		if ((int)nextModalCursorActive != modalCursorRegion) {
			modalCursorRegion = nextModalCursorActive;
			debugC(2, kDebugCursor,
				"Ripper: modal cursor region=%s source='%s' resource=%u point=%d,%d bounds=%d,%d,%d,%d",
				modalCursorRegion != 0 ? "dialog" : "scene", source, bodyResourceId,
				mouse.position.x, mouse.position.y, bounds.left, bounds.top,
				bounds.width(), bounds.height());
		}
		if (mouse.wheel != 0) {
			const uint maximumFirst = lines.size() > visibleRows ?
				lines.size() - visibleRows : 0;
			int nextFirst = (int)firstVisible - mouse.wheel;
			if (nextFirst < 0)
				nextFirst = 0;
			else if ((uint)nextFirst > maximumFirst)
				nextFirst = maximumFirst;
			if ((uint)nextFirst != firstVisible) {
				firstVisible = nextFirst;
				redraw = true;
				debugC(3, kDebugScene,
					"Ripper: mouse-wheel scrolled modal text source='%s' resource=%u "
					"delta=%d firstLine=%u visibleRows=%u",
					source, bodyResourceId, mouse.wheel, firstVisible, visibleRows);
			}
		}
		if (redraw)
			drawDialog(title, lines, firstVisible, visibleRows, bounds, style);
		else
			// The modal owns the main loop, so it must keep presenting frames
			// for CursorMan's software cursor to follow mouse-motion events.
			g_system->updateScreen();
		g_system->delayMillis(10);
	}

	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	if (retainSceneCursorRegions)
		_engine->getScripts()->updateModalSceneCursor(cursorPoint);
	debugC(1, kDebugScene,
		"Ripper: exited modal text dialog source='%s' resource=%u",
		source, bodyResourceId);
	return true;
}

} // End of namespace Ripper
