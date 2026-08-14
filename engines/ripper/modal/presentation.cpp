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

#include "ripper/modal/constants.h"

#include "common/debug.h"
#include "common/system.h"
#include "graphics/paletteman.h"
#include "graphics/surface.h"

#include "ripper/cursor.h"
#include "ripper/detection.h"
#include "ripper/ripper.h"

namespace Ripper {

namespace {

byte findNearestPaletteIndex(const byte *palette, byte red, byte green,
		byte blue) {
	uint bestDistance = 0xffffffff;
	byte bestIndex = 0;
	for (uint index = 0; index < 256; ++index) {
		const int redDistance = palette[index * 3] - red;
		const int greenDistance = palette[index * 3 + 1] - green;
		const int blueDistance = palette[index * 3 + 2] - blue;
		const uint distance = redDistance * redDistance +
			greenDistance * greenDistance + blueDistance * blueDistance;
		if (distance < bestDistance) {
			bestDistance = distance;
			bestIndex = index;
		}
	}
	return bestIndex;
}

const char *presentationStyleName(ModalDialogManager::PresentationStyle style) {
	switch (style) {
	case ModalDialogManager::kWacPresentation:
		return "wacmnu";
	case ModalDialogManager::kPrimaryPresentation:
		return "primary";
	case ModalDialogManager::kSceneEntryPresentation:
		return "scene-entry";
	default:
		return "menub";
	}
}

} // End of anonymous namespace

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

void ModalDialogManager::drawFrame(byte *screen, uint pitch,
		const Common::Rect &bounds, PresentationStyle style) const {
	if (style == kPrimaryPresentation)
		return;
	const Common::Array<BitmapAssetFrame> &skin =
		style == kWacPresentation ? _wacSkin : _skin;
	// RIPPER stores bitmap dimensions and presentation coordinates in
	// vertical/horizontal order. Translating ResolveChooserFrameTileIndex at
	// 0x55250 to screen x/y makes MENUB0..8 row-major.
	IndexedBitmapRenderer::drawNineSlice(screen, pitch, skin, bounds);
}

void ModalDialogManager::drawOverflowBar(byte *screen, uint pitch,
		const Common::Rect &bounds, uint firstVisible,
		uint maximumFirstVisible, TextPanelScrollControl hoveredScrollControl,
		PresentationStyle style) const {
	if (style == kSceneEntryPresentation)
		return;
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
		IndexedBitmapRenderer::drawBitmap(screen, pitch, up,
			upBounds.left, upBounds.top);
		IndexedBitmapRenderer::drawBitmap(screen, pitch, down,
			downBounds.left, downBounds.top);
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
	IndexedBitmapRenderer::drawBitmap(screen, pitch, thumb,
		thumbBounds.left, thumbBounds.top);
	IndexedBitmapRenderer::drawBitmap(screen, pitch, up,
		upBounds.left, upBounds.top);
	IndexedBitmapRenderer::drawBitmap(screen, pitch, down,
		downBounds.left, downBounds.top);
}

Common::Rect ModalDialogManager::textPanelScrollControlBounds(
		const Common::Rect &bounds, TextPanelScrollControl control,
		uint firstVisible, uint maximumFirstVisible, PresentationStyle style) const {
	if (style == kSceneEntryPresentation)
		return Common::Rect();
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
		TextPanelScrollControl hoveredScrollControl, bool present) const {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1) {
		if (screen)
			g_system->unlockScreen();
		return;
	}

	byte *pixels = (byte *)screen->getPixels();
	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const bool sceneEntryStyle = style == kSceneEntryPresentation;
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
	const int textHorizontalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextHorizontalInset;
	const int textVerticalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextVerticalInset;
	const int rowHeight = primaryStyle ? kPrimaryRowHeight : kModalRowHeight;
	const byte titleColor = primaryStyle ? kPrimaryTextColor :
		(wacStyle ? kWacModalTitleColor :
		(sceneEntryStyle ? kSceneEntryTextColor : kModalTitleColor));
	const byte textColor = primaryStyle ? kPrimaryTextColor :
		(wacStyle ? kWacModalTextColor :
		(sceneEntryStyle ? kSceneEntryTextColor : kModalTextColor));
	if (primaryStyle) {
		// RunCircuitChipPlacementPuzzleScene at 0x28aa4 constructs resource
		// 0xb6 with g_primaryChooserPresentationTemplate at 0x8a284. The
		// wrapped control has no frame or client fill: ED_WAC.SMK supplies
		// the backing, while RenderChooserTextRow at 0x58651 selects the
		// active-row style at template offset +0xc. Startup points that style
		// at palette index 254, the shared interface red.
	} else if (sceneEntryStyle) {
		// InitializeSharedPresentationTemplates at 0x11bd1 initializes the
		// v1.05 media-text template at 0x9a78e without MENUB bitmap
		// descriptors. Its client/background byte is palette index zero and
		// both normal text-template bytes are palette index 14.
		for (int y = bounds.top; y < bounds.bottom; ++y)
			memset(screen->getBasePtr(bounds.left, y),
				kSceneEntryBackgroundColor, bounds.width());
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
		const int fontHeight = primaryStyle ?
			_primaryFont.lineHeight : _font.lineHeight;
		const int titleX = bounds.left + leftPadding +
			(contentWidth - measureText(title)) / 2;
		// DrawPromptChooserTemplateLabelCallback at 0x16def reads the
		// WAC template's +0x19 byte as an absolute top inset rather than
		// vertically centering the label in its 20-pixel heading.
		const int titleY = wacStyle ?
			bounds.top + kWacModalTitleTopInset :
			bounds.top + (headingHeight - fontHeight) / 2;
		drawText(pixels, screen->pitch, titleX,
			titleY,
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
	if (present) {
		_engine->getCursor()->refresh();
		presentScreen();
	}
}

bool ModalDialogManager::drawRetainedTextPanel(uint bodyResourceId,
		const Common::Rect &bounds, uint firstVisible, uint &maximumFirstVisible,
		uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl, bool present) {
	const Common::String &body = resourceString(bodyResourceId);
	if (!drawRetainedTextPanelText(body, bounds, firstVisible,
			maximumFirstVisible, visibleRows, style, hoveredScrollControl,
			present)) {
		warning("Ripper: could not draw retained text panel resource=%u", bodyResourceId);
		return false;
	}
	debugC(3, kDebugScene,
		"Ripper: drew retained text panel resource=%u style=%s firstLine=%u visibleRows=%u bounds=%d,%d,%d,%d",
		bodyResourceId, presentationStyleName(style),
		firstVisible, visibleRows, bounds.left, bounds.top,
		bounds.width(), bounds.height());
	return true;
}

bool ModalDialogManager::drawRetainedTextPanelText(const Common::String &body,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl, bool present) {
	if (!_initialized || body.empty())
		return false;

	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const bool sceneEntryStyle = style == kSceneEntryPresentation;
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int horizontalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextHorizontalInset;
	if (bounds.width() <= leftPadding + rightPadding +
			horizontalInset * 2)
		return false;

	Common::Array<Common::String> lines;
	wrapText(body, bounds.width() - leftPadding - rightPadding -
		horizontalInset * 2, lines, style);
	return drawRetainedTextPanelLines(lines, bounds, firstVisible,
		maximumFirstVisible, visibleRows, style, hoveredScrollControl,
		present);
}

bool ModalDialogManager::drawRetainedTitlePanel(uint titleResourceId,
		const Common::Rect &bounds, PresentationStyle style, bool present) {
	const Common::String &title = resourceString(titleResourceId);
	if (title.empty()) {
		warning("Ripper: could not draw retained title panel resource=%u",
			titleResourceId);
		return false;
	}
	return drawRetainedTitlePanelText(title, bounds, style, present);
}

bool ModalDialogManager::drawRetainedTitlePanelText(
		const Common::String &title, const Common::Rect &bounds,
		PresentationStyle style, bool present) {
	if (!_initialized || title.empty())
		return false;
	if (style == kMenubPresentation)
		applyModalPalette();

	Common::Array<Common::String> lines;
	drawDialog(title, lines, 0, 0, bounds, style,
		kTextPanelScrollNone, present);
	debugC(3, kDebugScene,
		"Ripper: drew retained title panel style=%s title='%s' bounds=%d,%d,%d,%d",
		presentationStyleName(style),
		title.c_str(), bounds.left, bounds.top, bounds.width(),
		bounds.height());
	return true;
}

bool ModalDialogManager::drawRetainedPlainTextButton(uint titleResourceId,
		const Common::Rect &bounds, bool present) {
	const Common::String &title = resourceString(titleResourceId);
	if (!_initialized || title.empty() || bounds.isEmpty())
		return false;

	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	const byte backgroundColor = findNearestPaletteIndex(palette,
		0xff, 0xff, 0xff);
	const byte textColor = findNearestPaletteIndex(palette, 0, 0, 0);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen || screen->format.bytesPerPixel != 1 || bounds.left < 0 ||
			bounds.top < 0 || bounds.right > screen->w ||
			bounds.bottom > screen->h) {
		if (screen)
			g_system->unlockScreen();
		return false;
	}
	for (int y = bounds.top; y < bounds.bottom; ++y)
		memset(screen->getBasePtr(bounds.left, y), backgroundColor,
			bounds.width());
	drawText((byte *)screen->getPixels(), screen->pitch,
		bounds.left + (bounds.width() - measureText(title)) / 2,
		bounds.top + (bounds.height() - _font.lineHeight) / 2,
		title, textColor, kSceneEntryPresentation);
	g_system->unlockScreen();
	if (present) {
		_engine->getCursor()->refresh();
		presentScreen();
	}
	debugC(3, kDebugScene,
		"Ripper: drew retained plain text button resource=%u title='%s' "
		"bounds=%d,%d,%d,%d background=%u text=%u",
		titleResourceId, title.c_str(), bounds.left, bounds.top,
		bounds.width(), bounds.height(), backgroundColor, textColor);
	return true;
}

bool ModalDialogManager::drawRetainedTextPanelLines(
		const Common::Array<Common::String> &lines,
		const Common::Rect &bounds, uint firstVisible,
		uint &maximumFirstVisible, uint &visibleRows, PresentationStyle style,
		TextPanelScrollControl hoveredScrollControl, bool present) {
	const bool wacStyle = style == kWacPresentation;
	const bool primaryStyle = style == kPrimaryPresentation;
	const bool sceneEntryStyle = style == kSceneEntryPresentation;
	const int bottomPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalBottomPadding : kModalBottomPadding);
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int topPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalHeadingTopPadding : kModalBottomPadding);
	const int horizontalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextHorizontalInset;
	const int verticalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextVerticalInset;
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
		hoveredScrollControl, present);
	debugC(3, kDebugScene,
		"Ripper: drew retained text panel lines style=%s lines=%u "
		"firstLine=%u visibleRows=%u viewport=%d,%d,%d,%d bounds=%d,%d,%d,%d",
		presentationStyleName(style), lines.size(),
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
	const bool sceneEntryStyle = style == kSceneEntryPresentation;
	const int bottomPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalBottomPadding : kModalBottomPadding);
	const int leftPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalLeftPadding : kModalLeftPadding);
	const int rightPadding = primaryStyle ? 0 :
		(wacStyle ? kWacModalRightPadding : kModalRightPadding);
	const int topPadding = primaryStyle ? 0 : (wacStyle ?
		kWacModalHeadingTopPadding : kModalBottomPadding);
	const int horizontalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextHorizontalInset;
	const int verticalInset = primaryStyle || sceneEntryStyle ? 0 :
		kModalTextVerticalInset;
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
		(wacStyle ? kWacModalTextColor :
			(sceneEntryStyle ? kSceneEntryTextColor : kModalTextColor));
	if (!primaryStyle) {
		const byte backgroundColor = wacStyle ?
			kWacModalBackgroundColor : (sceneEntryStyle ?
				kSceneEntryBackgroundColor : kModalBackgroundColor);
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
	presentScreen();
	debugC(3, kDebugScene,
		"Ripper: redrew retained text panel line style=%s line=%u "
		"firstLine=%u visibleRow=%u bounds=%d,%d,%d,%d",
		presentationStyleName(style),
		lineIndex, firstVisible, visibleRow,
		bounds.left, bounds.top, bounds.width(), bounds.height());
	return true;
}

} // End of namespace Ripper
