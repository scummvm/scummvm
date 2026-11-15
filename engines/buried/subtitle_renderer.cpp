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

#include "buried/subtitle_renderer.h"
#include "buried/buried.h"
#include "buried/graphics.h"
#include "buried/scene_view.h"
#include "buried/window.h"

#include "common/algorithm.h"
#include "common/config-manager.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/surface.h"

namespace Buried {

struct ColorRGB {
	byte r;
	byte g;
	byte b;
};

// Subtitle Box Layout Constants
// Controls position, width, and height of the subtitle overlay box.
// The subtitle box is designed to appear below the jumpsuit's viewport, spanning from one end of the window to the other
// and including the viewport bezels.
//
// This design deliberately obstructs the display of the current date while subtitles are showing. This was deemed preferable
// to showing the subtitles on top of the viewport and obstructing the already-small jumpsuit viewport.
//
// The X coordinate of the subtitle box's top-left edge.
static constexpr int kSubtitleBoxX = 46;
// The Y coordinate of the subtitle box's top-left edge.
static constexpr int kSubtitleBoxY = 317;
// The width of the subtitle box.
static constexpr int kSubtitleBoxWidth = 462;
// Top Y coordinate of the Jumpsuit HUD inventory tray (matches InventoryWindow::_rect.top in inventory_window.cpp).
static constexpr int kInventoryWindowTopY = 375;
// The opacity of the subtitle box. 0f would be fully transparent. 1f would be fully opaque.
static constexpr float kSubtitleBoxOpacity = 0.75f;
// Horizontal padding of the interior of the subtitle box.
static constexpr int kSubtitlePadX = 8;
// Additional vertical padding of the interior of the subtitle box. This is added additionally to space reserved
// for font ascenders, descenders, and the angled chamfer section at the bottom of the subtitle box.
static constexpr int kSubtitlePadY = 2;
// Maximum number of wrapped dialogue lines displayed in the default subtitle box height.
static constexpr int kMaxSubtitleLines = 2;

// Height in pixels of the angled chamfer bevel at the bottom of the subtitle box.
// The bevel is rendered at a 2:1 horizontal-to-vertical slope ratio.
// This constant controls the vertical span of the cutouts; the maximum horizontal inset at the bottom-most row is
// derived as (kSubtitleChamferHeight * 2) pixels.
static constexpr int kSubtitleChamferHeight = 6;

// Border stroke thickness in pixels for the top, bottom, and side frame edges.
static constexpr int kSubtitleBorderWidth = 1;

// Vertical spacing in pixels between consecutive lines of wrapped text.
static constexpr int kSubtitleInterlineSpacing = 2;

// Solid black used to clear letterbox subtitle regions in the off-screen buffer.
static constexpr ColorRGB kBlackColor = { 0, 0, 0 };
// Background color for the subtitle box.
static constexpr ColorRGB kBoxBgColor = { 38, 12, 12 };
// Color for the subtitle box top border. (Muted-copper-orange)
static constexpr ColorRGB kTopBorderColor = { 140, 60, 35 };
// Color for the subtitle box bottom border. (Bright orange)
static constexpr ColorRGB kBottomBorderColor = { 237, 109, 66 };
// Color for the subtitle box side borders. (Muted dark red)
static constexpr ColorRGB kSideBorderColor = { 105, 36, 28 };
// Color for the speaker name text. (Neon orange)
static constexpr ColorRGB kSpeakerTextColor = { 237, 109, 66 };
// Color for the subtitle text, chosen to have high contrast with the background. (Cream amber)
static constexpr ColorRGB kDialogTextColor = { 255, 230, 180 };

SubtitleRenderer::SubtitleRenderer(BuriedEngine *vm, const SubtitleManager *subtitles) : _vm(vm), _subtitles(subtitles) {
	loadFont();
}

SubtitleRenderer::~SubtitleRenderer() = default;

bool SubtitleRenderer::loadFont() {
	int requestedSize = kDefaultSubtitleFontSize;
	if (ConfMan.hasKey(kSubtitleFontSizeKey)) {
		requestedSize = ConfMan.getInt(kSubtitleFontSizeKey);
	}

	if (_fonts.matchesSize(requestedSize)) {
		return false;
	}

	FontSet updated;
	updated.size = requestedSize;
	updated.regular.reset(_vm->_gfx->createFont(requestedSize, /* bold= */ false));
	updated.bold.reset(_vm->_gfx->createFont(requestedSize, /* bold= */ true));
	_fonts = Common::move(updated);
	return true;
}

void SubtitleRenderer::updateFont(Window *targetWindow) {
	if (loadFont()) {
		markDirty(targetWindow);
	}
}

void SubtitleRenderer::markDirty(Window *targetWindow) {
	if (!targetWindow) {
		return;
	}

	if (!SubtitleManager::areSubtitlesEnabled() && _lastRenderedBounds.isEmpty()) {
		// If the player disabled subtitles while a line was already showing on the screen,
		// we still need to invalidate to remove the previous subtitles from the screen.
		return;
	}

	// Invalidate targetWindow so its onPaint() method runs, and separately invalidate the subtitle
	// box bounds because subtitles render outside targetWindow's own rectangle (for example, below
	// SceneViewWindow or below a standalone VideoWindow).
	targetWindow->invalidateWindow(/* erase= */ false);
	_vm->_gfx->invalidateRect(getDefaultBoxBounds(), /* erase= */ false);

	if (!_lastRenderedBounds.isEmpty()) {
		// Clear the previous subtitle rectangle in the off-screen buffer before repainting.
		// Windows with background bitmaps overwrite this during onPaint() before display output,
		// while standalone letterbox cutscenes (whose parent windows draw nothing in onPaint())
		// rely on this black fill to erase the previous subtitle.
		if (_vm->_gfx->getScreen()) {
			_vm->_gfx->fillRect(_lastRenderedBounds, getColor(kBlackColor));
		}
		_vm->_gfx->invalidateRect(_lastRenderedBounds, /* erase= */ false);
		_lastRenderedBounds = Common::Rect();
	}
}

uint32 SubtitleRenderer::getColor(const ColorRGB &color) const {
	return _vm->_gfx->getColor(color.r, color.g, color.b);
}

static byte blendColorComponent(byte srcComp, byte targetComp, float alpha) {
	return (byte)(srcComp * (1.0f - alpha) + targetComp * alpha);
}

static void drawPixel(Graphics::Surface *destSurface, int x, int y, uint32 color) {
	if (destSurface->format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)destSurface->getBasePtr(x, y);
		*ptr = (uint16)color;
	} else if (destSurface->format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)destSurface->getBasePtr(x, y);
		*ptr = color;
	} else {
		byte *ptr = (byte *)destSurface->getBasePtr(x, y);
		*ptr = (byte)color;
	}
}

// Blends the existing color at pixel (x,y) on destSurface with the target color.
static void blendPixel(Graphics::Surface *destSurface, int x, int y, ColorRGB target, float alpha, uint32 fallbackColor) {
	if (destSurface->format.bytesPerPixel == 2) {
		uint16 *ptr = (uint16 *)destSurface->getBasePtr(x, y);
		byte r, g, b;
		destSurface->format.colorToRGB(*ptr, r, g, b);
		byte blendedR = blendColorComponent(r, target.r, alpha);
		byte blendedG = blendColorComponent(g, target.g, alpha);
		byte blendedB = blendColorComponent(b, target.b, alpha);
		*ptr = destSurface->format.RGBToColor(blendedR, blendedG, blendedB);
	} else if (destSurface->format.bytesPerPixel == 4) {
		uint32 *ptr = (uint32 *)destSurface->getBasePtr(x, y);
		byte r, g, b;
		destSurface->format.colorToRGB(*ptr, r, g, b);
		byte blendedR = blendColorComponent(r, target.r, alpha);
		byte blendedG = blendColorComponent(g, target.g, alpha);
		byte blendedB = blendColorComponent(b, target.b, alpha);
		*ptr = destSurface->format.RGBToColor(blendedR, blendedG, blendedB);
	} else {
		drawPixel(destSurface, x, y, fallbackColor);
	}
}

bool SubtitleRenderer::renderForMedia(Graphics::Surface *destSurface, const Common::Rect &boxRect, const Common::Path &filePath, uint32 currentMs) {
	if (!destSurface || !SubtitleManager::areSubtitlesEnabled() || !_subtitles || !_fonts.regular || !_fonts.bold) {
		return false;
	}

	const SubtitleManager::SubtitleEntry *entry = _subtitles->getSubtitleForTime(filePath, currentMs);
	if (!entry || entry->text.empty()) {
		return false;
	}

	const SubtitleLayout layout = buildSubtitleLayout(*entry, boxRect, Common::Rect(0, 0, destSurface->w, destSurface->h));
	if (layout.clippedBounds.isEmpty()) {
		return false;
	}

	drawSubtitleBox(destSurface, layout.boxBounds, layout.clippedBounds);
	drawSubtitleText(destSurface, layout);

	_lastRenderedBounds = layout.clippedBounds;
	_vm->_gfx->invalidateRect(layout.clippedBounds, /* erase= */ false);
	return true;
}

bool SubtitleRenderer::renderForVideo(Graphics::Surface *destSurface, const Window *videoWindow, const Common::Path &filePath, uint32 currentMs, const Common::Rect &mediaRect) {
	if (!destSurface) {
		return false;
	}

	Common::Rect boxBounds = calculateBoxBoundsForVideo(videoWindow, mediaRect);
	return renderForMedia(destSurface, boxBounds, filePath, currentMs);
}

SubtitleRenderer::SubtitleLayout SubtitleRenderer::buildSubtitleLayout(const SubtitleManager::SubtitleEntry &entry, const Common::Rect &targetBoxRect, const Common::Rect &surfaceBounds) const {
	SubtitleLayout layout;
	const int interiorWidth = targetBoxRect.width() - kSubtitlePadX * 2;
	if (interiorWidth <= 0) {
		return layout;
	}

	// Build speaker prefix (e.g. "Arthur: ")
	layout.speakerPrefix = entry.speaker.empty()
		? Common::U32String()
		: entry.speaker + Common::U32String(": ");
	layout.speakerWidth = layout.speakerPrefix.empty() ? 0 : _fonts.bold->getStringWidth(layout.speakerPrefix);

	// Word-wrap text using ScummVM's native Font::wordWrapText, passing speakerWidth as initWidth for line 1.
	_fonts.regular->wordWrapText(entry.text, interiorWidth, layout.lines, layout.speakerWidth);

	const int numLines = MAX<int>(kMaxSubtitleLines, (int)layout.lines.size());
	layout.boxBounds = targetBoxRect;
	layout.boxBounds.bottom = layout.boxBounds.top + getBoxHeight(numLines);

	// For HUD subtitles (starting at kSubtitleBoxY), if 3+ lines push the bottom edge past
	// kInventoryWindowTopY, shift the box upward so InventoryWindow does not paint over it.
	if (targetBoxRect.top == kSubtitleBoxY && layout.boxBounds.bottom > kInventoryWindowTopY) {
		const int overflow = layout.boxBounds.bottom - kInventoryWindowTopY;
		layout.boxBounds.translate(0, -overflow);
	}

	layout.clippedBounds = layout.boxBounds;
	layout.clippedBounds.clip(surfaceBounds);
	return layout;
}

void SubtitleRenderer::drawSubtitleBox(Graphics::Surface *destSurface, const Common::Rect &boxBounds, const Common::Rect &clippedBounds) const {
	const float alpha = CLIP<float>(kSubtitleBoxOpacity, 0.0f, 1.0f);
	const uint32 topBorderColor = getColor(kTopBorderColor);
	const uint32 bottomBorderColor = getColor(kBottomBorderColor);
	const uint32 sideBorderColor = getColor(kSideBorderColor);
	const uint32 boxBgColor = getColor(kBoxBgColor);

	// Render subtitle box scrim & bottom chamfered border.
	//
	// 1. Chamfer Cutouts: For the bottom rows (distanceFromBottom < kSubtitleChamferHeight), inset the left and
	//    right bounds by (kSubtitleChamferHeight - distanceFromBottom) * 2 pixels (2:1 horizontal-to-vertical bevel).
	// 2. CRT Scanline Interlacing: Alternate row opacity (1.15x alpha on even rows, 0.85x alpha on odd rows) to
	//    simulate an interlaced glass CRT monitor unless alpha is configured to fully opaque (1.0f).
	for (int y = clippedBounds.top; y < clippedBounds.bottom; ++y) {
		const int distanceFromTop = y - boxBounds.top;
		const int distanceFromBottom = boxBounds.bottom - 1 - y;

		int boxHorizontalInset = 0;
		if (distanceFromBottom < kSubtitleChamferHeight) {
			boxHorizontalInset = (kSubtitleChamferHeight - distanceFromBottom) * 2;
		}

		const int rowStartX = boxBounds.left + boxHorizontalInset;
		const int rowEndX = boxBounds.right - boxHorizontalInset;
		const int drawStartX = MAX<int>(clippedBounds.left, rowStartX);
		const int drawEndX = MIN<int>(clippedBounds.right, rowEndX);

		float curAlpha = alpha;
		if (alpha < 1.0f) {
			curAlpha = distanceFromTop % 2 == 0 ? alpha * 1.15f : alpha * 0.85f;
			curAlpha = CLIP<float>(curAlpha, 0.0f, 1.0f);
		}

		const bool isTopBorder = y < boxBounds.top + kSubtitleBorderWidth;
		const bool isBottomBorder = y >= boxBounds.bottom - kSubtitleBorderWidth;

		for (int x = drawStartX; x < drawEndX; ++x) {
			const bool isSideBorder = x < rowStartX + kSubtitleBorderWidth || x >= rowEndX - kSubtitleBorderWidth;

			if (isTopBorder) {
				drawPixel(destSurface, x, y, topBorderColor);
			} else if (isBottomBorder) {
				drawPixel(destSurface, x, y, bottomBorderColor);
			} else if (isSideBorder) {
				drawPixel(destSurface, x, y, sideBorderColor);
			} else {
				blendPixel(destSurface, x, y, kBoxBgColor, curAlpha, boxBgColor);
			}
		}
	}
}

void SubtitleRenderer::drawSubtitleText(Graphics::Surface *destSurface, const SubtitleLayout &layout) const {
	const int interiorWidth = layout.boxBounds.width() - kSubtitlePadX * 2;
	const int fontHeight = _fonts.regular->getFontHeight();
	const uint32 speakerColor = getColor(kSpeakerTextColor);
	const uint32 dialogColor = getColor(kDialogTextColor);

	const int innerX = layout.boxBounds.left + kSubtitlePadX;
	int curY = layout.boxBounds.top + kSubtitlePadY;

	for (size_t i = 0; i < layout.lines.size(); ++i) {
		int curX = innerX;
		if (i == 0) {
			// Special handling to draw the speaker name on the first line.
			if (!layout.speakerPrefix.empty()) {
				_fonts.bold->drawString(destSurface, layout.speakerPrefix, curX, curY, interiorWidth, speakerColor, Graphics::kTextAlignLeft);
				curX += layout.speakerWidth;
			}
			const int line1AvailableTextWidth = interiorWidth - layout.speakerWidth;
			if (line1AvailableTextWidth > 0 && !layout.lines[0].empty()) {
				_fonts.regular->drawString(destSurface, layout.lines[0], curX, curY, line1AvailableTextWidth, dialogColor, Graphics::kTextAlignLeft);
			}
		} else {
			// Remaining lines only have dialog text.
			if (!layout.lines[i].empty()) {
				_fonts.regular->drawString(destSurface, layout.lines[i], curX, curY, interiorWidth, dialogColor, Graphics::kTextAlignLeft);
			}
		}
		curY += fontHeight + kSubtitleInterlineSpacing;
	}
}

int SubtitleRenderer::getFontHeight() const {
	return _fonts.regular->getFontHeight();
}

int SubtitleRenderer::getBoxHeight() const {
	return getBoxHeight(kMaxSubtitleLines);
}

int SubtitleRenderer::getBoxHeight(int numLines) const {
	int maxTextHeight = getFontHeight() * numLines;
	int interLineSpacingHeight = kSubtitleInterlineSpacing * (numLines - 1);
	int paddingHeight = kSubtitlePadY * 2;
	int borderHeight = kSubtitleBorderWidth * 2;
	return maxTextHeight + interLineSpacingHeight + paddingHeight + borderHeight + kSubtitleChamferHeight;
}

Common::Rect SubtitleRenderer::getDefaultBoxBounds() const {
	int boxHeight = getBoxHeight();
	return {
		kSubtitleBoxX,
		kSubtitleBoxY,
		kSubtitleBoxX + kSubtitleBoxWidth,
		(int16)(kSubtitleBoxY + boxHeight) };
}

bool SubtitleRenderer::isJumpsuitVideoWindow(const Window *videoWindow) const {
	if (!videoWindow) {
		return false;
	}

	// In-game Jumpsuit visor videos (such as INN broadcasts and scene animations) are
	// created as direct children of SceneViewWindow, whereas standalone letterbox cutscenes
	// (such as INTRO_1.BTV on GameUIWindow or FA_FIN.BTV on CompletionWindow) have a
	// different parent window.
	return dynamic_cast<const SceneViewWindow *>(videoWindow->getParent()) != nullptr;
}

Common::Rect SubtitleRenderer::calculateBoxBoundsForVideo(const Window *videoWindow, const Common::Rect &mediaRect) const {
	// The behavior is bifurcated for videos that play in the Jumpsuit view (like INN) and videos that play over
	// a blank screen like the intro and finale videos. These videos do not play in the same screen location.
	// For Jumpsuit videos, we want the subtitle box to align with where it is for non-video media.
	if (isJumpsuitVideoWindow(videoWindow)) {
		return getDefaultBoxBounds();
	}

	int boxHeight = getBoxHeight();
	return Common::Rect(
		mediaRect.left,
		mediaRect.bottom,
		mediaRect.right,
		mediaRect.bottom + boxHeight
	);
}

} // End of namespace Buried
