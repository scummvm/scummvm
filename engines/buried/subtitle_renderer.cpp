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
#include "buried/subtitle_manager.h"
#include "buried/window.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/ustr.h"
#include "common/util.h"
#include "graphics/font.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Buried {

constexpr int SubtitleRenderer::kDefaultFontSize;
constexpr const char *SubtitleRenderer::kFontSizeKey;

namespace {

struct ColorRGB {
	byte r;
	byte g;
	byte b;
};

// Precomputed geometry and wrapped lines for rendering a single subtitle card.
struct SubtitleLayout {
	// Screen-space bounds of the subtitle box, clipped to the destination surface.
	Common::Rect boxBounds;
	// Formatted speaker prefix drawn in bold on the first line (e.g. "Arthur: "), or empty.
	Common::U32String speakerPrefix;
	// Rendered width in pixels of speakerPrefix in the bold font.
	int speakerWidthPx = 0;
	// Wrapped dialogue lines (with line 0 wrapped to the remaining width after speakerWidthPx).
	Common::Array<Common::U32String> lines;
};

// External Jumpsuit HUD Geometry
// Top Y coordinate of the Jumpsuit HUD inventory tray (matches InventoryWindow::_rect.top in inventory_window.cpp).
constexpr int kInventoryWindowTopY = 375;

// Subtitle Box Layout Constants
// Controls position, width, and height of the subtitle overlay box.
// The subtitle box is designed to appear below the jumpsuit's viewport, spanning from one end of the window to the other
// and including the viewport bezels.
//
// This design deliberately obstructs the jumpsuit's date display while subtitles are showing. This was deemed preferable
// to showing the subtitles on top of the viewport and obstructing the already-small jumpsuit viewport.
//
// The X coordinate of the subtitle box's top-left edge.
constexpr int kSubtitleBoxX = 46;
// The Y coordinate of the subtitle box's top-left edge.
constexpr int kSubtitleBoxY = 317;
// The width of the subtitle box.
constexpr int kSubtitleBoxWidth = 462;
// The opacity of the subtitle box. 0f would be fully transparent. 1f would be fully opaque.
constexpr float kSubtitleBoxOpacity = 0.75f;
// Horizontal padding of the interior of the subtitle box.
constexpr int kSubtitlePadX = 8;
// Additional vertical padding of the interior of the subtitle box. This is added additionally to space reserved
// for font ascenders, descenders, and the angled chamfer section at the bottom of the subtitle box.
constexpr int kSubtitlePadY = 2;
// Maximum number of wrapped dialogue lines displayed in the default subtitle box height.
constexpr int kMaxSubtitleLines = 2;

// Height in pixels of the angled chamfer bevel at the bottom of the subtitle box.
// The bevel is rendered at a 2:1 horizontal-to-vertical slope ratio.
// This constant controls the vertical span of the cutouts; the maximum horizontal inset at the bottom-most row is
// derived as (kSubtitleChamferHeight * 2) pixels.
constexpr int kSubtitleChamferHeight = 6;

// Border stroke thickness in pixels for the top, bottom, and side frame edges.
constexpr int kSubtitleBorderWidth = 1;

// Vertical spacing in pixels between consecutive lines of wrapped text.
constexpr int kSubtitleInterlineSpacing = 2;

// Solid black used to clear letterbox subtitle regions in the off-screen buffer.
constexpr ColorRGB kBlackColor = { 0, 0, 0 };
// Background color for the subtitle box.
constexpr ColorRGB kBoxBackgroundColor = { 38, 12, 12 };
// Color for the subtitle box top border. (Muted-copper-orange)
constexpr ColorRGB kTopBorderColor = { 140, 60, 35 };
// Color for the subtitle box bottom border. (Bright orange)
constexpr ColorRGB kBottomBorderColor = { 237, 109, 66 };
// Color for the subtitle box side borders. (Muted dark red)
constexpr ColorRGB kSideBorderColor = { 105, 36, 28 };
// Color for the speaker name text. (Neon orange)
constexpr ColorRGB kSpeakerTextColor = { 237, 109, 66 };
// Color for the subtitle text, chosen to have high contrast with the background. (Cream amber)
constexpr ColorRGB kDialogTextColor = { 255, 230, 180 };

int getConfiguredFontSize() {
	if (ConfMan.hasKey(SubtitleRenderer::kFontSizeKey)) {
		return ConfMan.getInt(SubtitleRenderer::kFontSizeKey);
	}
	return SubtitleRenderer::kDefaultFontSize;
}

uint32 resolveColor(GraphicsManager *gfx, const ColorRGB &color) {
	return gfx->getColor(color.r, color.g, color.b);
}

byte blendColorComponent(byte srcComp, byte targetComp, float alpha) {
	return (byte)(srcComp * (1.0f - alpha) + targetComp * alpha);
}

// Linearly blends src and target RGB colors using alpha in [0.0, 1.0].
ColorRGB blendColors(ColorRGB src, ColorRGB target, float alpha) {
	return {
		blendColorComponent(src.r, target.r, alpha),
		blendColorComponent(src.g, target.g, alpha),
		blendColorComponent(src.b, target.b, alpha)
	};
}

// Reads the RGB color of pixel (x, y) from a TrueColor or 8-bit paletted surface.
ColorRGB readPixelRGB(const Graphics::Surface *surface, const GraphicsManager *gfx, int x, int y) {
	uint32 rawPixel = surface->getPixel(x, y);
	ColorRGB rgb = kBlackColor;
	if (surface->format.bytesPerPixel > 1) {
		surface->format.colorToRGB(rawPixel, rgb.r, rgb.g, rgb.b);
	} else if (gfx && gfx->getDefaultPalette()) {
		// GraphicsManager owns the default palette buffer; DisposeAfterUse::NO wraps it
		// as a non-owning view so Palette's destructor does not free gfx's memory.
		Graphics::Palette palette(gfx->getDefaultPalette(), Graphics::PALETTE_COUNT, DisposeAfterUse::NO);
		palette.get(rawPixel, rgb.r, rgb.g, rgb.b);
	}
	return rgb;
}

int getBoxHeight(int fontHeight, int numLines = kMaxSubtitleLines) {
	int maxTextHeight = fontHeight * numLines;
	int interLineSpacingHeight = kSubtitleInterlineSpacing * (numLines - 1);
	int paddingHeight = kSubtitlePadY * 2;
	int borderHeight = kSubtitleBorderWidth * 2;
	return maxTextHeight + interLineSpacingHeight + paddingHeight + borderHeight + kSubtitleChamferHeight;
}

Common::Rect getDefaultBoxBounds(int fontHeight) {
	int boxHeight = getBoxHeight(fontHeight);
	return Common::Rect(
		kSubtitleBoxX,
		kSubtitleBoxY,
		kSubtitleBoxX + kSubtitleBoxWidth,
		kSubtitleBoxY + boxHeight
	);
}

bool isJumpsuitVideoWindow(const Window *videoWindow) {
	if (!videoWindow) {
		return false;
	}

	// In-game Jumpsuit visor videos (such as INN broadcasts and scene animations) are
	// created as direct children of SceneViewWindow, whereas standalone letterbox cutscenes
	// (such as the intro and ending videos) have a different parent window.
	return dynamic_cast<const SceneViewWindow *>(videoWindow->getParent()) != nullptr;
}

// Computes wrapped dialogue lines, speaker prefix metrics, and clipped box bounds for a
// subtitle card. Places the box at the default position and width unless aboveRect is
// provided, in which case the box is placed directly below *aboveRect with matching width.
SubtitleLayout buildSubtitleLayout(
		const SubtitleManager::Subtitle &subtitle,
		const Graphics::Font &regularFont,
		const Graphics::Font &boldFont,
		const Common::Rect *aboveRect,
		const Common::Rect &surfaceBounds) {
	int boxLeft = aboveRect ? aboveRect->left : kSubtitleBoxX;
	int boxTop = aboveRect ? aboveRect->bottom : kSubtitleBoxY;
	int boxWidth = aboveRect ? aboveRect->width() : kSubtitleBoxWidth;

	SubtitleLayout layout;
	int interiorWidth = boxWidth - kSubtitlePadX * 2;
	if (interiorWidth <= 0) {
		return layout;
	}

	// Build speaker prefix (e.g. "Arthur: ")
	layout.speakerPrefix = subtitle.speaker.empty()
		? Common::U32String()
		: subtitle.speaker + Common::U32String(": ");
	layout.speakerWidthPx = layout.speakerPrefix.empty() ? 0 : boldFont.getStringWidth(layout.speakerPrefix);

	// Word-wrap text using ScummVM's native Font::wordWrapText, passing speakerWidthPx as initWidth for line 1.
	regularFont.wordWrapText(subtitle.text, interiorWidth, layout.lines, layout.speakerWidthPx);

	int numLines = MAX<int>(kMaxSubtitleLines, (int)layout.lines.size());
	int boxHeight = getBoxHeight(regularFont.getFontHeight(), numLines);
	layout.boxBounds = Common::Rect(boxLeft, boxTop, boxLeft + boxWidth, boxTop + boxHeight);

	// If 3+ wrapped lines at the default position push the bottom edge past kInventoryWindowTopY,
	// shift the box upward so InventoryWindow does not paint over it. This is uncommon because
	// subtitle cards are split to fit within 2 lines up through the Large font size, so 3+ lines
	// only occur with the Extra Large font preset or translated text.
	if (!aboveRect && layout.boxBounds.bottom > kInventoryWindowTopY) {
		int overflow = layout.boxBounds.bottom - kInventoryWindowTopY;
		layout.boxBounds.translate(0, -overflow);
	}

	layout.boxBounds.clip(surfaceBounds);
	return layout;
}

void drawSubtitleBox(Graphics::Surface *destSurface, GraphicsManager *gfx, const Common::Rect &boxBounds) {
	float alpha = CLIP<float>(kSubtitleBoxOpacity, 0.0f, 1.0f);
	uint32 topBorderColor = resolveColor(gfx, kTopBorderColor);
	uint32 bottomBorderColor = resolveColor(gfx, kBottomBorderColor);
	uint32 sideBorderColor = resolveColor(gfx, kSideBorderColor);
	uint32 boxBackgroundColor = resolveColor(gfx, kBoxBackgroundColor);

	// Render subtitle box scrim & bottom chamfered border.
	//
	// 1. Chamfer Cutouts: For the bottom rows (distanceFromBottom < kSubtitleChamferHeight), inset the left and
	//    right bounds by (kSubtitleChamferHeight - distanceFromBottom) * 2 pixels (2:1 horizontal-to-vertical bevel).
	// 2. CRT Scanline Interlacing: Alternate row opacity (1.15x alpha on even rows, 0.85x alpha on odd rows) to
	//    simulate an interlaced glass CRT monitor unless alpha is configured to fully opaque (1.0f).
	for (int y = boxBounds.top; y < boxBounds.bottom; y++) {
		int distanceFromTop = y - boxBounds.top;
		int distanceFromBottom = boxBounds.bottom - 1 - y;

		// Inset the left and right edges on the bottom rows to form a 2:1 chamfer bevel.
		int boxHorizontalInset = 0;
		if (distanceFromBottom < kSubtitleChamferHeight) {
			boxHorizontalInset = (kSubtitleChamferHeight - distanceFromBottom) * 2;
		}
		int rowStartX = boxBounds.left + boxHorizontalInset;
		int rowEndX = boxBounds.right - boxHorizontalInset;

		if (y < boxBounds.top + kSubtitleBorderWidth) {
			destSurface->hLine(rowStartX, y, rowEndX - 1, topBorderColor);
			continue;
		}
		if (y >= boxBounds.bottom - kSubtitleBorderWidth) {
			destSurface->hLine(rowStartX, y, rowEndX - 1, bottomBorderColor);
			continue;
		}

		// Draw the left and right side borders, leaving [interiorStartX, interiorEndX) for the interior fill.
		int interiorStartX = rowStartX + kSubtitleBorderWidth;
		int interiorEndX = rowEndX - kSubtitleBorderWidth;
		destSurface->hLine(rowStartX, y, interiorStartX - 1, sideBorderColor);
		destSurface->hLine(interiorEndX, y, rowEndX - 1, sideBorderColor);

		// Fill the row's interior: blend the background color over the underlying scene with
		// alternating row opacity to produce a translucent CRT-glass effect.
		if (destSurface->format.bytesPerPixel > 1) {
			float curAlpha = alpha;
			if (alpha < 1.0f) {
				curAlpha = distanceFromTop % 2 == 0 ? alpha * 1.15f : alpha * 0.85f;
				curAlpha = CLIP<float>(curAlpha, 0.0f, 1.0f);
			}

			for (int x = interiorStartX; x < interiorEndX; x++) {
				ColorRGB existing = readPixelRGB(destSurface, gfx, x, y);
				ColorRGB blended = blendColors(existing, kBoxBackgroundColor, curAlpha);
				destSurface->setPixel(x, y, destSurface->format.RGBToColor(blended.r, blended.g, blended.b));
			}
		} else {
			// 8-bit paletted surfaces (1 byte per pixel) use a fixed 256-color palette that
			// cannot smoothly represent blended RGB gradients, so draw a solid fill instead.
			destSurface->hLine(interiorStartX, y, interiorEndX - 1, boxBackgroundColor);
		}
	}
}

void drawSubtitleText(
		Graphics::Surface *destSurface,
		GraphicsManager *gfx,
		const Graphics::Font &regularFont,
		const Graphics::Font &boldFont,
		const SubtitleLayout &layout) {
	int interiorWidth = layout.boxBounds.width() - kSubtitlePadX * 2;
	int fontHeight = regularFont.getFontHeight();
	uint32 speakerColor = resolveColor(gfx, kSpeakerTextColor);
	uint32 dialogColor = resolveColor(gfx, kDialogTextColor);

	int innerX = layout.boxBounds.left + kSubtitlePadX;
	int curY = layout.boxBounds.top + kSubtitlePadY;

	// Draw the speaker prefix
	if (!layout.speakerPrefix.empty()) {
		boldFont.drawString(
			destSurface,
			layout.speakerPrefix,
			innerX,
			curY,
			interiorWidth,
			speakerColor,
			Graphics::kTextAlignLeft);
	}

	// Font::wordWrapText(..., initWidth = speakerWidthPx) reserves speakerWidthPx on line 0;
	// if the first word cannot fit after the speaker prefix, lines[0] is empty and dialogue
	// starts on lines[1], so no text is ever dropped.
	for (size_t i = 0; i < layout.lines.size(); i++) {
		int lineOffset = (i == 0) ? layout.speakerWidthPx : 0;
		int availableWidth = interiorWidth - lineOffset;
		if (!layout.lines[i].empty() && availableWidth > 0) {
			regularFont.drawString(
				destSurface,
				layout.lines[i],
				innerX + lineOffset,
				curY,
				availableWidth,
				dialogColor,
				Graphics::kTextAlignLeft);
		}
		curY += fontHeight + kSubtitleInterlineSpacing;
	}
}

} // End of anonymous namespace

SubtitleRenderer::SubtitleRenderer(BuriedEngine *vm, const SubtitleManager *subtitles)
		: _vm(vm), _subtitles(subtitles), _fonts(createFontSet(getConfiguredFontSize())) {
}

SubtitleRenderer::~SubtitleRenderer() = default;

SubtitleRenderer::FontSet SubtitleRenderer::createFontSet(int fontSize) const {
	FontSet fonts;
	fonts.size = fontSize;
	fonts.regular.reset(_vm->_gfx->createFont(fontSize, /* bold= */ false));
	fonts.bold.reset(_vm->_gfx->createFont(fontSize, /* bold= */ true));
	return fonts;
}

void SubtitleRenderer::updateFont(Window *targetWindow) {
	int requestedSize = getConfiguredFontSize();
	// Early return to avoid recreating the fonts when already loaded at the requested size.
	if (_fonts.matchesSize(requestedSize)) {
		return;
	}

	_fonts = createFontSet(requestedSize);
	// Repaint the window and subtitle region so any active subtitle is immediately redrawn at the new font size.
	markDirty(targetWindow);
}

void SubtitleRenderer::markDirty(Window *targetWindow) {
	if (!targetWindow) {
		return;
	}

	// Skip invalidation when subtitles are disabled and no subtitle is currently on screen;
	// if a previous subtitle is still drawn (_lastRenderedBounds is non-empty), fall through
	// so it gets cleared and invalidated.
	if (!SubtitleManager::areSubtitlesEnabled() && _lastRenderedBounds.isEmpty()) {
		return;
	}

	// Window::updateWindow() only calls targetWindow->onPaint() (which triggers subtitle
	// rendering) if the dirty rect intersects targetWindow's own bounds, and GraphicsManager
	// only repaints/blits pixels inside the dirty rect. Because the subtitle box sits outside
	// targetWindow's bounds (below SceneViewWindow or VideoWindow), both rects must be marked dirty.
	targetWindow->invalidateWindow(/* erase= */ false);
	if (_fonts.regular) {
		_vm->_gfx->invalidateRect(getDefaultBoxBounds(_fonts.regular->getFontHeight()), /* erase= */ false);
	}

	if (!_lastRenderedBounds.isEmpty()) {
		// Clear the previous subtitle rect to black in the off-screen buffer for screens
		// whose parent window draws no background (such as standalone letterbox cutscenes).
		if (_vm->_gfx->getScreen()) {
			_vm->_gfx->fillRect(_lastRenderedBounds, resolveColor(_vm->_gfx, kBlackColor));
		}
		_vm->_gfx->invalidateRect(_lastRenderedBounds, /* erase= */ false);
		_lastRenderedBounds = Common::Rect();
	}
}

bool SubtitleRenderer::renderForMedia(Graphics::Surface *destSurface, const Common::Path &filePath, uint32 currentMs, const Common::Rect *aboveRect) {
	if (!destSurface || !SubtitleManager::areSubtitlesEnabled() || !_subtitles || !_fonts.regular || !_fonts.bold) {
		return false;
	}

	const SubtitleManager::Subtitle *subtitle = _subtitles->getSubtitleForTime(filePath, currentMs);
	if (!subtitle || subtitle->text.empty()) {
		return false;
	}

	SubtitleLayout layout = buildSubtitleLayout(
		*subtitle,
		*_fonts.regular,
		*_fonts.bold,
		aboveRect,
		destSurface->getRect());
	// Nothing to render if the box has no usable interior width or lies completely off-screen.
	if (layout.boxBounds.isEmpty()) {
		return false;
	}

	drawSubtitleBox(destSurface, _vm->_gfx, layout.boxBounds);
	drawSubtitleText(destSurface, _vm->_gfx, *_fonts.regular, *_fonts.bold, layout);

	_lastRenderedBounds = layout.boxBounds;
	// markDirty() only pre-invalidates the default 2-line HUD box, whereas cutscenes and 3+ line
	// cards can extend outside that rect. Expand GraphicsManager's dirty rect before updateScreen()
	// runs copyRectToScreen() so the full rendered box is blitted to the display this frame.
	_vm->_gfx->invalidateRect(layout.boxBounds, /* erase= */ false);
	return true;
}

bool SubtitleRenderer::renderForVideo(Graphics::Surface *destSurface, const Window *videoWindow, const Common::Path &filePath, uint32 currentMs, const Common::Rect &mediaRect) {
	if (!destSurface) {
		return false;
	}

	const Common::Rect *aboveRect = isJumpsuitVideoWindow(videoWindow) ? nullptr : &mediaRect;
	return renderForMedia(destSurface, filePath, currentMs, aboveRect);
}

} // End of namespace Buried
