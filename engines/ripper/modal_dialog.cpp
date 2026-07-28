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
static const int kModalTextHorizontalInset = 2;
static const int kModalTextVerticalInset = 1;
static const uint kModalWidth = 300;
static const int kModalRowHeight = 14;
static const byte kModalBackgroundColor = 253;
static const byte kModalHeadingColor = 255;
static const byte kModalTitleColor = 254;
static const byte kModalTextColor = 4;
static const byte kModalSelectedBackgroundColor = 248;
static const uint kBinaryPromptYesResourceId = 0x3c;
static const uint kBinaryPromptNoResourceId = 0x3d;
static const uint kBinaryPromptOptionCount = 2;
static const int kWacModalHeadingTopPadding = 20;
static const int kWacModalBottomPadding = 6;
static const int kWacModalLeftPadding = 5;
static const int kWacModalRightPadding = 20;
static const byte kWacModalBackgroundColor = 4;
static const byte kWacModalTitleColor = 248;
static const byte kWacModalTextColor = 251;
static const uint kPrimaryScrollSkinFrameCount = 4;
static const int kPrimaryRowHeight = 9;
static const int kPrimaryScrollGap = 5;
static const byte kPrimaryTextColor = 254;
static const uint kModalCursor = 16;
static const int kTextEntryLeft = 228;
static const int kTextEntryTop = 312;
static const int kTextEntryWidth = 194;
static const int kTextEntryHeight = 20;
static const int kTextEntryPadding = 5;
static const uint32 kTextEntryCaretBlinkMillis = 500;

} // End of anonymous namespace

ModalDialogManager::ModalDialogManager(RipperEngine *engine) :
		_engine(engine), _textEntryStyle(kMenubPresentation),
		_textEntryMaximumLength(0), _textEntryHelpResourceId(0),
		_textEntryFirstVisible(0), _textEntryCursorPosition(0),
		_textEntryNextCaretMillis(0), _textEntryOverwrite(false),
		_textEntryCaretVisible(false), _textEntryActive(false),
		_textEntryRestoreCursor(false), _initialized(false) {
}

bool ModalDialogManager::initialize(ResourceManager &resources) {
	if (!resources.loadInterfaceBitmapFont("small.fnt", _font) ||
			!resources.loadInterfaceBitmapFont("7pt_font.fnt", _primaryFont) ||
			!resources.loadGameText(_gameText))
		return false;

	_skin.clear();
	_wacSkin.clear();
	_primaryScrollSkin.clear();
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
	for (uint frameIndex = 0; frameIndex < kPrimaryScrollSkinFrameCount;
			++frameIndex) {
		BitmapAssetSequence sequence;
		if (!resources.loadInterfaceBitmapSequence(
				Common::String::format("mnarrow%u.bbm", frameIndex),
				sequence) ||
				sequence.frames.empty())
			return false;
		_primaryScrollSkin.push_back(Common::move(sequence.frames.front()));
	}

	_initialized = true;
	debugC(1, kDebugScene,
		"Ripper: initialized modal text dialogs menubFrames=%u "
		"wacmnuFrames=%u primaryScrollFrames=%u fonts=small.fnt,"
		"7pt_font.fnt strings=%u",
		_skin.size(), _wacSkin.size(), _primaryScrollSkin.size(),
		_gameText.size());
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
	return _savedDisplay.capture();
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
	if (!_savedDisplay.isValid())
		return;

	_savedDisplay.restore(true, false);
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	_savedDisplay.clear();
}

uint ModalDialogManager::measureText(const Common::String &text,
		PresentationStyle style) const {
	return BitmapFontRenderer::measureText(
		style == kPrimaryPresentation ? _primaryFont : _font, text);
}

void ModalDialogManager::wrapText(const Common::String &text, uint maxWidth,
		Common::Array<Common::String> &lines, PresentationStyle style) const {
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
			if (!line.empty() && measureText(candidate, style) > maxWidth) {
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
		const Common::String &text, byte color,
		PresentationStyle style) const {
	BitmapFontRenderer::drawText(screen, pitch,
		style == kPrimaryPresentation ? _primaryFont : _font,
		x, y, text, color);
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
	if (style == kPrimaryPresentation)
		return;
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
		const Common::Rect &bounds, uint firstVisible,
		uint maximumFirstVisible, TextPanelScrollControl hoveredScrollControl,
		PresentationStyle style) const {
	if (style == kPrimaryPresentation) {
		if (_primaryScrollSkin.size() < kPrimaryScrollSkinFrameCount)
			return;
		const BitmapAssetFrame &up = _primaryScrollSkin[
			hoveredScrollControl == kTextPanelScrollUp && firstVisible > 0 ?
				1 : 0];
		const BitmapAssetFrame &down = _primaryScrollSkin[
			hoveredScrollControl == kTextPanelScrollDown &&
				firstVisible < maximumFirstVisible ? 3 : 2];
		const Common::Rect upBounds = textPanelScrollControlBounds(bounds,
			kTextPanelScrollUp, firstVisible, maximumFirstVisible, style);
		const Common::Rect downBounds = textPanelScrollControlBounds(bounds,
			kTextPanelScrollDown, firstVisible, maximumFirstVisible, style);
		drawBitmap(screen, pitch, up, upBounds.left, upBounds.top);
		drawBitmap(screen, pitch, down, downBounds.left, downBounds.top);
		return;
	}
	const Common::Array<BitmapAssetFrame> &skin =
		style == kWacPresentation ? _wacSkin : _skin;
	if (skin.size() <= kModalScrollTrackFrame)
		return;

	const BitmapAssetFrame &up = skin[
		hoveredScrollControl == kTextPanelScrollUp && firstVisible > 0 ?
			kModalScrollStartFrame + 2 : kModalScrollStartFrame];
	const BitmapAssetFrame &down = skin[
		hoveredScrollControl == kTextPanelScrollDown &&
			firstVisible < maximumFirstVisible ?
			kModalScrollEndFrame + 2 : kModalScrollEndFrame];
	const BitmapAssetFrame &thumb = skin[kModalScrollStartFrame + 4];
	const BitmapAssetFrame &track = skin[kModalScrollTrackFrame];
	const Common::Rect upBounds = textPanelScrollControlBounds(bounds,
		kTextPanelScrollUp, firstVisible, maximumFirstVisible, style);
	const Common::Rect downBounds = textPanelScrollControlBounds(bounds,
		kTextPanelScrollDown, firstVisible, maximumFirstVisible, style);
	const Common::Rect trackBounds = textPanelScrollControlBounds(bounds,
		kTextPanelScrollTrack, firstVisible, maximumFirstVisible, style);
	const Common::Rect thumbBounds = textPanelScrollControlBounds(bounds,
		kTextPanelScrollThumb, firstVisible, maximumFirstVisible, style);

	for (int y = trackBounds.top; y < trackBounds.bottom; y += track.height) {
		const uint rows = MIN<uint>(track.height, trackBounds.bottom - y);
		for (uint row = 0; row < rows; ++row) {
			for (uint column = 0; column < track.width; ++column) {
				const byte pixel = track.pixels[row * track.width + column];
				if (pixel != track.transparentColor)
					screen[(y + row) * pitch + trackBounds.left + column] = pixel;
			}
		}
	}
	drawBitmap(screen, pitch, thumb, thumbBounds.left, thumbBounds.top);
	drawBitmap(screen, pitch, up, upBounds.left, upBounds.top);
	drawBitmap(screen, pitch, down, downBounds.left, downBounds.top);
}

Common::Rect ModalDialogManager::textPanelScrollControlBounds(
		const Common::Rect &bounds, TextPanelScrollControl control,
		uint firstVisible, uint maximumFirstVisible, PresentationStyle style) const {
	if (style == kPrimaryPresentation) {
		if (_primaryScrollSkin.size() < kPrimaryScrollSkinFrameCount ||
				maximumFirstVisible == 0)
			return Common::Rect();
		const BitmapAssetFrame &up = _primaryScrollSkin[0];
		const BitmapAssetFrame &down = _primaryScrollSkin[2];
		const Common::Rect upBounds(bounds.right + kPrimaryScrollGap,
			bounds.top, bounds.right + kPrimaryScrollGap + up.width,
			bounds.top + up.height);
		const Common::Rect downBounds(bounds.right + kPrimaryScrollGap,
			bounds.bottom - down.height,
			bounds.right + kPrimaryScrollGap + down.width, bounds.bottom);
		if (control == kTextPanelScrollUp)
			return upBounds;
		if (control == kTextPanelScrollDown)
			return downBounds;
		return Common::Rect();
	}
	const Common::Array<BitmapAssetFrame> &skin =
		style == kWacPresentation ? _wacSkin : _skin;
	if (skin.size() <= kModalScrollTrackFrame || maximumFirstVisible == 0)
		return Common::Rect();

	const BitmapAssetFrame &up = skin[kModalScrollStartFrame];
	const BitmapAssetFrame &down = skin[kModalScrollEndFrame];
	const BitmapAssetFrame &thumb = skin[kModalScrollStartFrame + 4];
	const BitmapAssetFrame &track = skin[kModalScrollTrackFrame];
	const bool wacStyle = style == kWacPresentation;
	const int arrowTopInset = wacStyle ? 5 : kModalScrollStartInset;
	const int arrowBottomInset = wacStyle ? 5 : kModalScrollEndInset;
	const int edgeInset = wacStyle ? 0 : kModalScrollEdgeInset;
	const Common::Rect upBounds(bounds.right - edgeInset - up.width,
		bounds.top + arrowTopInset, bounds.right - edgeInset,
		bounds.top + arrowTopInset + up.height);
	const Common::Rect downBounds(bounds.right - edgeInset - down.width,
		bounds.bottom - arrowBottomInset - down.height,
		bounds.right - edgeInset, bounds.bottom - arrowBottomInset);
	const Common::Rect trackBounds(bounds.right - edgeInset - track.width,
		upBounds.bottom, bounds.right - edgeInset, downBounds.top);
	const int thumbTravel = MAX<int>(0, trackBounds.height() - thumb.height);
	const int thumbTop = trackBounds.top +
		(firstVisible * thumbTravel + maximumFirstVisible / 2) /
		maximumFirstVisible;
	const Common::Rect thumbBounds(
		trackBounds.left + (trackBounds.width() - thumb.width) / 2,
		thumbTop,
		trackBounds.left + (trackBounds.width() + thumb.width) / 2,
		thumbTop + thumb.height);

	switch (control) {
	case kTextPanelScrollUp:
		return upBounds;
	case kTextPanelScrollDown:
		return downBounds;
	case kTextPanelScrollThumb:
		return thumbBounds;
	case kTextPanelScrollTrack:
		return trackBounds;
	default:
		return Common::Rect();
	}
}

ModalDialogManager::TextPanelScrollControl
ModalDialogManager::findTextPanelScrollControl(const Common::Rect &bounds,
		const Common::Point &point, uint firstVisible, uint maximumFirstVisible,
		PresentationStyle style) const {
	if (textPanelScrollControlBounds(bounds, kTextPanelScrollUp,
			firstVisible, maximumFirstVisible, style).contains(point))
		return kTextPanelScrollUp;
	if (textPanelScrollControlBounds(bounds, kTextPanelScrollDown,
			firstVisible, maximumFirstVisible, style).contains(point))
		return kTextPanelScrollDown;
	if (textPanelScrollControlBounds(bounds, kTextPanelScrollThumb,
			firstVisible, maximumFirstVisible, style).contains(point))
		return kTextPanelScrollThumb;
	if (textPanelScrollControlBounds(bounds, kTextPanelScrollTrack,
			firstVisible, maximumFirstVisible, style).contains(point))
		return kTextPanelScrollTrack;
	return kTextPanelScrollNone;
}

void ModalDialogManager::drawDialog(const Common::String &title,
		const Common::Array<Common::String> &lines, uint firstVisible,
		uint visibleRows, const Common::Rect &bounds,
		PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	// ComputeChooserControlLayout at 0x54a74 selects the tertiary template's
	// 20-pixel alternate top padding even without an optional title because
	// layout56 bit 0 is set. The WAC database's circuit-manual view embeds its
	// heading in resource 0xb6, but still receives that forced inset.
	const int headingHeight = primaryStyle ? 0 :
		(wacStyle ? kWacModalHeadingTopPadding :
		(title.empty() ? kModalBottomPadding : kModalHeadingTopPadding));
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int textHorizontalInset = primaryStyle ? 0 :
		kModalTextHorizontalInset;
	const int textVerticalInset = primaryStyle ? 0 :
		kModalTextVerticalInset;
	const int rowHeight = primaryStyle ? kPrimaryRowHeight : kModalRowHeight;
	const byte titleColor = primaryStyle ? kPrimaryTextColor :
		(wacStyle ? kWacModalTitleColor : kModalTitleColor);
	const byte textColor = primaryStyle ? kPrimaryTextColor :
		(wacStyle ? kWacModalTextColor : kModalTextColor);
	if (primaryStyle) {
		// RunCircuitChipPlacementPuzzleScene at 0x28aa4 constructs resource
		// 0xb6 with g_primaryChooserPresentationTemplate at 0x8a284. The
		// wrapped control has no frame or client fill: ED_WAC.SMK supplies
		// the backing, while RenderChooserTextRow at 0x58651 selects the
		// active-row style at template offset +0xc. Startup points that style
		// at palette index 254, the shared interface red.
	} else if (wacStyle) {
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
			bounds.top + (headingHeight -
				(primaryStyle ? _primaryFont.lineHeight :
					_font.lineHeight)) / 2,
			title, titleColor, style);
	}
	for (uint row = 0; row < visibleRows; ++row) {
		const uint lineIndex = firstVisible + row;
		if (lineIndex >= lines.size())
			break;
		drawText(pixels, screen->pitch,
			bounds.left + leftPadding + textHorizontalInset,
			bounds.top + headingHeight + textVerticalInset +
				row * rowHeight +
				(rowHeight - (primaryStyle ?
					_primaryFont.lineHeight : _font.lineHeight)) / 2,
			lines[lineIndex], textColor, style);
	}
	if (lines.size() > visibleRows)
		drawOverflowBar(pixels, screen->pitch, bounds, firstVisible,
			lines.size() - visibleRows, hoveredScrollControl, style);
	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

bool ModalDialogManager::drawRetainedTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, uint firstVisible, uint &maximumFirstVisible,
		uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl) {
	const Common::String &body = resourceString(bodyResourceId);
	if (!drawRetainedTextPanelText(body, bounds, firstVisible,
			maximumFirstVisible, visibleRows, style, hoveredScrollControl)) {
		warning("Ripper: could not draw retained text panel resource=%u", bodyResourceId);
		return false;
	}
	const char *styleName = style == kWacPresentation ? "wacmnu" :
		(style == kPrimaryPresentation ? "primary" : "menub");
	debugC(3, kDebugScene,
		"Ripper: drew retained text panel resource=%u style=%s firstLine=%u visibleRows=%u bounds=%d,%d,%d,%d",
		bodyResourceId, styleName,
		firstVisible, visibleRows, bounds.left, bounds.top,
		bounds.width(), bounds.height());
	return true;
}

bool ModalDialogManager::drawRetainedTextPanelText(const Common::String &body,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl) {
	if (!_initialized || body.empty())
		return false;

	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int horizontalInset = primaryStyle ? 0 : kModalTextHorizontalInset;
	if (bounds.width() <= leftPadding + rightPadding +
			horizontalInset * 2)
		return false;

	Common::Array<Common::String> lines;
	wrapText(body, bounds.width() - leftPadding - rightPadding -
		horizontalInset * 2, lines, style);
	return drawRetainedTextPanelLines(lines, bounds, firstVisible,
		maximumFirstVisible, visibleRows, style, hoveredScrollControl);
}

bool ModalDialogManager::drawRetainedTextPanelLines(
		const Common::Array<Common::String> &lines,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl) {
	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const int bottomPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalBottomPadding : kModalBottomPadding);
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int topPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalHeadingTopPadding : kModalBottomPadding);
	const int horizontalInset = primaryStyle ? 0 : kModalTextHorizontalInset;
	const int verticalInset = primaryStyle ? 0 : kModalTextVerticalInset;
	const int rowHeight = primaryStyle ? kPrimaryRowHeight : kModalRowHeight;
	if (!_initialized || lines.empty() || bounds.width() <=
			leftPadding + rightPadding + horizontalInset * 2 ||
			bounds.height() <= topPadding + bottomPadding +
				verticalInset * 2)
		return false;

	visibleRows = MAX<uint>(1,
		(bounds.height() - topPadding - bottomPadding -
			verticalInset * 2) / rowHeight);
	maximumFirstVisible = lines.size() > visibleRows ? lines.size() - visibleRows : 0;
	firstVisible = MIN(firstVisible, maximumFirstVisible);
	if (style == kMenubPresentation)
		applyModalPalette();
	drawDialog(Common::String(), lines, firstVisible, visibleRows, bounds, style,
		hoveredScrollControl);
	const char *styleName = wacStyle ? "wacmnu" :
		(primaryStyle ? "primary" : "menub");
	debugC(3, kDebugScene,
		"Ripper: drew retained text panel lines style=%s lines=%u "
		"firstLine=%u visibleRows=%u viewport=%d,%d,%d,%d bounds=%d,%d,%d,%d",
		styleName, lines.size(),
		firstVisible, visibleRows, bounds.left + leftPadding + horizontalInset,
		bounds.top + topPadding + verticalInset,
		bounds.width() - leftPadding - rightPadding - horizontalInset * 2,
		visibleRows * rowHeight,
		bounds.left, bounds.top, bounds.width(), bounds.height());
	return true;
}

bool ModalDialogManager::drawRetainedTextPanelLine(
		const Common::Array<Common::String> &lines,
		const Common::Rect &bounds, uint firstVisible, uint visibleRows,
		uint lineIndex, PresentationStyle style) {
	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const int bottomPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalBottomPadding : kModalBottomPadding);
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int topPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalHeadingTopPadding : kModalBottomPadding);
	const int horizontalInset = primaryStyle ? 0 : kModalTextHorizontalInset;
	const int verticalInset = primaryStyle ? 0 : kModalTextVerticalInset;
	const int rowHeight = primaryStyle ? kPrimaryRowHeight : kModalRowHeight;
	const int fontHeight = primaryStyle ?
		_primaryFont.lineHeight : _font.lineHeight;
	if (!_initialized || lines.empty() || visibleRows == 0 ||
			bounds.width() <= leftPadding + rightPadding ||
			bounds.height() <= topPadding + bottomPadding +
				verticalInset * 2)
		return false;
	if (lineIndex >= lines.size() || lineIndex < firstVisible ||
			lineIndex >= firstVisible + visibleRows)
		return true;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}

	const uint visibleRow = lineIndex - firstVisible;
	const int rowTop = bounds.top + topPadding + verticalInset +
		visibleRow * rowHeight;
	const int rowBottom = MIN<int>(rowTop + rowHeight,
		bounds.bottom - bottomPadding);
	const int clientLeft = bounds.left + leftPadding;
	const int clientWidth = bounds.width() - leftPadding - rightPadding;
	const byte textColor = primaryStyle ? kPrimaryTextColor :
		(wacStyle ? kWacModalTextColor : kModalTextColor);
	if (!primaryStyle) {
		const byte backgroundColor = wacStyle ?
			kWacModalBackgroundColor : kModalBackgroundColor;
		for (int y = rowTop; y < rowBottom; ++y)
			memset(screen->getBasePtr(clientLeft, y),
				backgroundColor, clientWidth);
	}
	drawText((byte *)screen->getPixels(), screen->pitch,
		clientLeft + horizontalInset,
		rowTop + (rowHeight - fontHeight) / 2,
		lines[lineIndex], textColor, style);
	g_system->unlockScreen();
	if (style == kMenubPresentation)
		applyModalPalette();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
	debugC(3, kDebugScene,
		"Ripper: redrew retained text panel line style=%s line=%u "
		"firstLine=%u visibleRow=%u bounds=%d,%d,%d,%d",
		wacStyle ? "wacmnu" : (primaryStyle ? "primary" : "menub"),
		lineIndex, firstVisible, visibleRow,
		bounds.left, bounds.top, bounds.width(), bounds.height());
	return true;
}

void ModalDialogManager::drawTextEntry(const Common::String &prompt,
		const Common::String &text, uint firstVisible, uint cursorPosition,
		bool caretVisible, const Common::Rect &bounds, PresentationStyle style) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	const bool wacStyle = style == kWacPresentation;
	const byte backgroundColor = wacStyle ? kWacModalBackgroundColor :
		kModalBackgroundColor;
	const byte textColor = wacStyle ? kWacModalTextColor : kModalTextColor;
	if (wacStyle) {
		for (int y = bounds.top; y < bounds.bottom; ++y)
			memset(screen->getBasePtr(bounds.left, y), backgroundColor,
				bounds.width());
		drawFrame(pixels, screen->pitch, bounds, style);
	} else {
		drawFrame(pixels, screen->pitch, bounds, style);
		for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
			memset(screen->getBasePtr(bounds.left + kTextEntryPadding, y),
				backgroundColor, bounds.width() - kTextEntryPadding * 2);
	}

	int textTop;
	int textLeft;
	int textRight;
	if (wacStyle) {
		// CreateTextEntryChooserControl at 0x59d93 passes the prompt as the
		// chooser heading. BuildChooserControlVisuals centers it in the WAC
		// template's 20-pixel heading, above the single editable row.
		const int promptWidth = measureText(prompt);
		const int contentWidth =
			bounds.width() - kWacModalLeftPadding - kWacModalRightPadding;
		drawText(pixels, screen->pitch,
			bounds.left + kWacModalLeftPadding +
				(contentWidth - promptWidth) / 2,
			bounds.top +
				(kWacModalHeadingTopPadding - _font.lineHeight) / 2,
			prompt, textColor);
		textTop = bounds.top + kWacModalHeadingTopPadding +
			kModalTextVerticalInset +
			(kModalRowHeight - _font.lineHeight) / 2;
		textLeft = bounds.left + kWacModalLeftPadding;
		textRight = bounds.right - kWacModalRightPadding;
	} else {
		textTop = bounds.top + (bounds.height() - _font.lineHeight) / 2;
		textLeft = bounds.left + kTextEntryPadding;
		textRight = bounds.right - kTextEntryPadding;
		if (!prompt.empty()) {
			drawText(pixels, screen->pitch, textLeft, textTop, prompt, textColor);
			textLeft += measureText(prompt) + _font.spaceWidth;
		}
	}
	const uint availableWidth = textRight - textLeft;
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
	drawText(pixels, screen->pitch, textLeft, textTop, visible, textColor);
	if (caretVisible && cursorPosition >= firstVisible && cursorPosition <= endVisible) {
		const int caretLeft = textLeft + measureText(text.substr(firstVisible,
			cursorPosition - firstVisible));
		drawText(pixels, screen->pitch, caretLeft, textTop, "_", textColor);
	}

	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

void ModalDialogManager::drawBinaryPrompt(const Common::String &prompt,
		const Common::String *options, uint selectedIndex,
		const Common::Rect &bounds) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	drawFrame(pixels, screen->pitch, bounds, kMenubPresentation);
	for (int y = bounds.top + 2; y < bounds.bottom - 2; ++y)
		memset(screen->getBasePtr(bounds.left + kModalLeftPadding, y),
			kModalBackgroundColor,
			bounds.width() - kModalLeftPadding - kModalRightPadding);
	for (int y = bounds.top + 2;
			y < bounds.top + kModalHeadingTopPadding; ++y)
		memset(screen->getBasePtr(bounds.left + kModalLeftPadding, y),
			kModalHeadingColor,
			bounds.width() - kModalLeftPadding - kModalRightPadding);

	const int contentWidth =
		bounds.width() - kModalLeftPadding - kModalRightPadding;
	drawText(pixels, screen->pitch,
		bounds.left + kModalLeftPadding +
			(contentWidth - measureText(prompt)) / 2,
		bounds.top +
			(kModalHeadingTopPadding - _font.lineHeight) / 2,
		prompt, kModalTitleColor);
	for (uint option = 0; option < kBinaryPromptOptionCount; ++option) {
		const int rowTop = bounds.top + kModalHeadingTopPadding +
			option * kModalRowHeight;
		if (option == selectedIndex) {
			for (int y = rowTop; y < rowTop + kModalRowHeight; ++y)
				memset(screen->getBasePtr(
					bounds.left + kModalLeftPadding, y),
					kModalSelectedBackgroundColor, contentWidth);
		}
		drawText(pixels, screen->pitch,
			bounds.left + kModalLeftPadding +
				kModalTextHorizontalInset,
			rowTop + (kModalRowHeight - _font.lineHeight) / 2,
			options[option], kModalTextColor);
	}

	g_system->unlockScreen();
	_engine->getCursor()->refresh();
	g_system->updateScreen();
}

uint ModalDialogManager::textEntryCursorFromPoint(const Common::String &text,
		uint firstVisible, int x, const Common::Rect &bounds) const {
	const bool wacStyle = _textEntryStyle == kWacPresentation;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kTextEntryPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kTextEntryPadding;
	int textLeft = bounds.left + leftPadding;
	if (!wacStyle && !_textEntryPrompt.empty())
		textLeft += measureText(_textEntryPrompt) + _font.spaceWidth;
	const int relativeX = x - textLeft;
	if (relativeX <= 0)
		return firstVisible;
	for (uint position = firstVisible; position < text.size(); ++position) {
		const uint left = measureText(text.substr(firstVisible,
			position - firstVisible));
		const uint right = measureText(text.substr(firstVisible,
			position - firstVisible + 1));
		if ((uint)relativeX < left + (right - left) / 2)
			return position;
		if (right >= (uint)bounds.width() - leftPadding - rightPadding)
			return position + 1;
	}
	return text.size();
}

void ModalDialogManager::updateTextEntryFirstVisible(const Common::Rect &bounds) {
	const bool wacStyle = _textEntryStyle == kWacPresentation;
	const int leftPadding = wacStyle ? kWacModalLeftPadding : kTextEntryPadding;
	const int rightPadding = wacStyle ? kWacModalRightPadding : kTextEntryPadding;
	int textLeft = bounds.left + leftPadding;
	if (!wacStyle && !_textEntryPrompt.empty())
		textLeft += measureText(_textEntryPrompt) + _font.spaceWidth;
	const uint availableWidth = bounds.right - rightPadding - textLeft;
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
		uint maximumLength, uint helpResourceId, const char *source,
		PresentationStyle style, const Common::Rect &bounds) {
	if (!_initialized || _textEntryActive) {
		warning("Ripper: could not begin scene text request source='%s' active=%d",
			source, _textEntryActive);
		return false;
	}
	_textEntryPrompt = prompt;
	_textEntryText.clear();
	_textEntrySource = source;
	_textEntryStyle = style;
	_textEntryBounds = bounds.isEmpty() ? Common::Rect(kTextEntryLeft,
		kTextEntryTop, kTextEntryLeft + kTextEntryWidth,
		kTextEntryTop + kTextEntryHeight) : bounds;
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
	if (_textEntryStyle == kMenubPresentation)
		applyModalPalette();
	drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
		_textEntryCursorPosition, _textEntryCaretVisible, _textEntryBounds,
		_textEntryStyle);
	debugC(1, kDebugScene,
		"Ripper: entered scene text request source='%s' control=0x4e2 style=%s bounds=%d,%d,%d,%d maximumLength=%u helpResource=%u",
		source, _textEntryStyle == kWacPresentation ? "wacmnu" : "menub",
		_textEntryBounds.left, _textEntryBounds.top, _textEntryBounds.width(),
		_textEntryBounds.height(), maximumLength, helpResourceId);
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

	const Common::Rect &bounds = _textEntryBounds;
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
				if (!_engine->getModalDialog()->run(_textEntryHelpResourceId, true,
						_textEntryStyle, _textEntryStyle == kWacPresentation ?
						kPreserveActivePalette : kApplyModalPalette)) {
					result = kTextEntryFailed;
					break;
				}
				if (_textEntryStyle == kMenubPresentation)
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
		if (_textEntryStyle == kMenubPresentation)
			applyModalPalette();
		drawTextEntry(_textEntryPrompt, _textEntryText, _textEntryFirstVisible,
			_textEntryCursorPosition, _textEntryCaretVisible, bounds,
			_textEntryStyle);
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
		PresentationStyle style, PaletteBehavior paletteBehavior) {
	const Common::String &title = resourceString(kModalTitleResourceId);
	const Common::String &body = resourceString(bodyResourceId);
	if (title.empty() || body.empty()) {
		warning("Ripper: could not present modal text resource=%u", bodyResourceId);
		return false;
	}
	return runTextInternal(title, body, bodyResourceId, "game text",
		retainSceneCursorRegions, style, paletteBehavior);
}

bool ModalDialogManager::runText(const Common::String &title,
		const Common::String &body, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style,
		PaletteBehavior paletteBehavior) {
	return runTextInternal(title, body, 0, source, retainSceneCursorRegions,
		style, paletteBehavior);
}

bool ModalDialogManager::runBinaryPrompt(uint promptResourceId,
		bool defaultFirstOption) {
	const Common::String &prompt = resourceString(promptResourceId);
	Common::String options[kBinaryPromptOptionCount] = {
		resourceString(kBinaryPromptYesResourceId),
		resourceString(kBinaryPromptNoResourceId)
	};
	if (!_initialized || prompt.empty() || options[0].empty() ||
			options[1].empty() || !captureDisplay()) {
		warning("Ripper: could not present binary prompt resource=%u",
			promptResourceId);
		return false;
	}

	// RunBinaryPromptChooser at 0x1803c creates a two-row list using the
	// secondary chooser template at 0x8a392. Its auto-sized client is centered
	// and uses the template's five-pixel horizontal padding, 17-pixel heading,
	// and 14-pixel rows. A false default argument selects row one ("No").
	const uint contentWidth = MAX<uint>(measureText(prompt),
		MAX<uint>(measureText(options[0]), measureText(options[1])));
	const int width = contentWidth + kModalLeftPadding +
		kModalRightPadding + kModalTextHorizontalInset * 2;
	const int height = kModalHeadingTopPadding +
		kBinaryPromptOptionCount * kModalRowHeight + kModalBottomPadding;
	const int left = (640 - width) / 2;
	const int top = (400 - height) / 2;
	const Common::Rect bounds(left, top, left + width, top + height);
	uint selectedIndex = defaultFirstOption ? 0 : 1;
	int pressedIndex = -1;
	bool accepted = false;
	bool active = true;
	bool redraw = true;
	const bool restoreCursorVisible = _engine->getCursor()->isVisible();
	Common::Point cursorPoint =
		_engine->getInput()->peekMouseState().position;

	_engine->getInput()->drainKeys();
	_engine->getInput()->discardMouseTransitions();
	applyModalPalette();
	_engine->getCursor()->update(kModalCursor);
	debugC(1, kDebugScene,
		"Ripper: entered binary prompt function=RunBinaryPromptChooser@0x1803c "
		"resource=%u prompt='%s' default=%u bounds=%d,%d,%d,%d",
		promptResourceId, prompt.c_str(), selectedIndex,
		bounds.left, bounds.top, bounds.width(), bounds.height());

	while (active && !_engine->shouldQuit()) {
		if (_engine->getInput()->pollEvents()) {
			_engine->quitGame();
			break;
		}
		while (_engine->getInput()->hasPendingKey()) {
			const uint16 command = _engine->getInput()->consumeKey();
			debugC(3, kDebugInput,
				"Ripper: binary prompt command=0x%04x selected=%u",
				command, selectedIndex);
			uint nextIndex = selectedIndex;
			switch (command) {
			case 0x1b:
				active = false;
				break;
			case 0x0d:
				accepted = true;
				active = false;
				break;
			case 0x4700:
			case 0x4800:
			case 'y':
			case 'Y':
				nextIndex = 0;
				break;
			case 0x4f00:
			case 0x5000:
			case 'n':
			case 'N':
				nextIndex = 1;
				break;
			default:
				break;
			}
			if (nextIndex != selectedIndex) {
				selectedIndex = nextIndex;
				redraw = true;
				debugC(2, kDebugInput,
					"Ripper: binary prompt keyboard selection=%u text='%s'",
					selectedIndex, options[selectedIndex].c_str());
			}
			if (!active)
				break;
		}
		if (!active)
			break;

		const MouseState mouse = _engine->getInput()->publishMouseState();
		cursorPoint = mouse.position;
		_engine->getCursor()->update(kModalCursor);
		int hoveredIndex = -1;
		for (uint option = 0; option < kBinaryPromptOptionCount; ++option) {
			const Common::Rect rowBounds(
				bounds.left + kModalLeftPadding,
				bounds.top + kModalHeadingTopPadding +
					option * kModalRowHeight,
				bounds.right - kModalRightPadding,
				bounds.top + kModalHeadingTopPadding +
					(option + 1) * kModalRowHeight);
			if (rowBounds.contains(mouse.position)) {
				hoveredIndex = option;
				break;
			}
		}
		if (hoveredIndex >= 0 && (uint)hoveredIndex != selectedIndex) {
			selectedIndex = hoveredIndex;
			redraw = true;
			debugC(2, kDebugInput,
				"Ripper: binary prompt hover selection=%u text='%s' point=%d,%d",
				selectedIndex, options[selectedIndex].c_str(),
				mouse.position.x, mouse.position.y);
		}
		if ((mouse.pressed & kMouseButtonLeft) != 0)
			pressedIndex = hoveredIndex;
		if ((mouse.released & kMouseButtonLeft) != 0) {
			if (pressedIndex >= 0 && pressedIndex == hoveredIndex) {
				selectedIndex = pressedIndex;
				accepted = true;
				active = false;
				debugC(2, kDebugInput,
					"Ripper: binary prompt mouse accepted selection=%u "
					"text='%s' point=%d,%d",
					selectedIndex, options[selectedIndex].c_str(),
					mouse.position.x, mouse.position.y);
			}
			pressedIndex = -1;
		}

		if (redraw) {
			drawBinaryPrompt(prompt, options, selectedIndex, bounds);
			redraw = false;
		} else {
			g_system->updateScreen();
		}
		g_system->delayMillis(10);
	}

	_engine->getInput()->discardMouseTransitions();
	restoreDisplay();
	if (!_engine->shouldQuit())
		_engine->getScripts()->updateModalSceneCursor(cursorPoint);
	_engine->getCursor()->setVisible(restoreCursorVisible);
	const bool firstOptionSelected = accepted && selectedIndex == 0;
	debugC(1, kDebugScene,
		"Ripper: exited binary prompt resource=%u accepted=%d selection=%u "
		"confirmed=%d quit=%d",
		promptResourceId, accepted, selectedIndex, firstOptionSelected,
		_engine->shouldQuit());
	return firstOptionSelected;
}

bool ModalDialogManager::runTextInternal(const Common::String &title,
		const Common::String &body, uint bodyResourceId, const char *source,
		bool retainSceneCursorRegions, PresentationStyle style,
		PaletteBehavior paletteBehavior) {
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
	const Common::Array<BitmapAssetFrame> &skin =
		wacStyle ? _wacSkin : _skin;
	const int scrollEdgeInset = wacStyle ? 0 : kModalScrollEdgeInset;
	int wrapWidth = kModalWidth - leftPadding - rightPadding;
	Common::Array<Common::String> lines;
	wrapText(body, wrapWidth, lines);
	if (lines.size() > kModalMaximumRows &&
			skin.size() > kModalScrollTrackFrame) {
		// ComputeChooserControlLayout at 0x54e9a through 0x54f17 adjusts
		// an overflowing text client's width by the scrollbar track width
		// plus its edge inset, less the right padding already reserved by
		// the chooser template.
		wrapWidth -= skin[kModalScrollTrackFrame].width +
			scrollEdgeInset - rightPadding;
		if (wrapWidth > 0)
			wrapText(body, wrapWidth, lines);
	}
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
	const bool applyPalette = !wacStyle && paletteBehavior == kApplyModalPalette;
	if (applyPalette)
		applyModalPalette();
	drawDialog(title, lines, firstVisible, visibleRows, bounds, style);
	debugC(3, kDebugScene,
		"Ripper: modal text layout source='%s' resource=%u wrapWidth=%d "
		"scrollTrackWidth=%u edgeInset=%d rightPadding=%d",
		source, bodyResourceId, wrapWidth,
		skin.size() > kModalScrollTrackFrame ?
			skin[kModalScrollTrackFrame].width : 0,
		scrollEdgeInset, rightPadding);
	debugC(1, kDebugScene,
		"Ripper: entered modal text dialog source='%s' resource=%u style=%s palette=%s title='%s' lines=%u bounds=%d,%d,%d,%d",
		source, bodyResourceId, wacStyle ? "wacmnu" : "menub",
		applyPalette ? "menub" : "active", title.c_str(),
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
