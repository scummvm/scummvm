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

#include "engines/nancy/cursor.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/resource.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/scrollbar.h"
#include "engines/nancy/ui/scrolltextbox.h"

namespace Nancy {
namespace UI {

static const uint16 kHypertextSurfaceHeight = 4096;

ScrollTextBox::ScrollTextBox() :
		RenderObject(10), // z=10: the textbox layer, above viewport (6) and taskbar (7).
		_header(nullptr),
		_tboxData(nullptr),
		_scrollPos(0.0f),
		_scrollbarDragging(false),
		_scrollbarHovered(false),
		_scrollbarGrabOffset(0),
		_fullMode(false),
		_expanded(false),
		_fontIDOverride(-1),
		_autoClearTime(0) {}

void ScrollTextBox::init() {
	_tboxData = GetEngineData(TBOX);
	assert(_tboxData);

	// The popup chrome and the text viewport come from different chunks
	// depending on the engine version, but both expose the same UIPopupHeader.
	if (g_nancy->getGameType() >= kGameTypeNancy11) {
		auto *sctb = GetEngineData(SCTB);
		assert(sctb);
		_header = &sctb->header;
		_textViewportScreen = sctb->restoreSrcRect;
	} else {
		auto *uico = GetEngineData(UICO);
		assert(uico);
		_header = &uico->header;
		_textViewportScreen = uico->textRect;
	}

	g_nancy->_resource->loadImage(_header->imageName, _overlayImage);

	_fullPopupRect = _header->normalDestRect;
	if (_header->overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			_fullPopupRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	moveTo(_fullPopupRect);

	// The taskbar owns the caption strip, so take the strip's text origin from the
	// TASK chunk rather than the conversation overlay.
	auto *task = GetEngineData(TASK);
	if (task) {
		_stripScreenRect = task->ccTextboxScreenRect;
	}

	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	// Key on the transparent color so only glyphs composite. Size for the wider
	// (strip) layout plus the line-start inset so either fits.
	const uint32 transColor = g_nancy->_graphics->getTransColor();
	const int surfaceWidth = _tboxData->lineStartXCursor + MAX<int>(stripTextWidth(), textViewportLocal().width());
	initSurfaces(surfaceWidth, kHypertextSurfaceHeight,
				 g_nancy->_graphics->getInputPixelFormat(), transColor, transColor);
	_fullSurface.setTransparentColor(transColor);

	setVisible(false);

	RenderObject::init();
}

void ScrollTextBox::registerGraphics() {
	RenderObject::registerGraphics();
}

void ScrollTextBox::updateGraphics() {
	if (_autoClearTime && g_nancy->getTotalPlayTime() > _autoClearTime) {
		clear();
	}

	RenderObject::updateGraphics();
}

Common::Rect ScrollTextBox::textViewportLocal() const {
	// Nancy13 stores the viewport as a source rect within the overlay image, so
	// it's already popup-local. Earlier games store it in screen space and share
	// the popup origin, so subtract the dest origin there.
	Common::Rect r = _textViewportScreen;
	if (g_nancy->getGameType() < kGameTypeNancy13) {
		r.translate(-_header->normalDestRect.left,
					-_header->normalDestRect.top);
	}
	return r;
}

int ScrollTextBox::stripTextWidth() const {
	// Strip text spans lineStartXCursor..(contentWidth - stripRightMargin), narrower
	// than the conversation viewport (matches FrameTextBox's normal path).
	return (_tboxData->contentWidth - _tboxData->stripRightMargin - _tboxData->lineStartXCursor) + 1;
}

void ScrollTextBox::drawBackground() {
	_drawSurface.blitFrom(_overlayImage, _header->normalSrcRect,
						  Common::Point(0, 0));
	_needsRedraw = true;
}

void ScrollTextBox::layoutText(int wrapWidth, uint16 fontID) {
	const uint32 transColor = g_nancy->_graphics->getTransColor();
	_drawnTextHeight = 0;
	_numDrawnLines = 0;
	_hotspots.clear();
	_fullSurface.fillRect(Common::Rect(0, 0, _fullSurface.w, _fullSurface.h), transColor);
	_textHighlightSurface.fillRect(Common::Rect(0, 0, _textHighlightSurface.w, _textHighlightSurface.h), transColor);

	// The line-start X cursor is the text's horizontal inset; there is no vertical
	// inset, so the first line starts at the top of the text area.
	Common::Rect textBounds(_tboxData->lineStartXCursor, 0,
							_tboxData->lineStartXCursor + wrapWidth, _fullSurface.h);
	drawAllText(textBounds, 0, fontID, _tboxData->highlightConversationFontID);
}

void ScrollTextBox::drawContent() {
	const uint32 transColor = g_nancy->_graphics->getTransColor();
	const Common::Rect viewport = textViewportLocal();
	const int fullHeight = _fullPopupRect.height();

	// Narration/caption text uses the (white) highlight font, not the (cyan)
	// conversation body font that the ConversationPopup uses.
	const uint16 fontID = _fontIDOverride != -1 ? (uint16)_fontIDOverride : _tboxData->highlightConversationFontID;

	// Lay out at the strip's text width first, then pick the box: Nancy 11+ by line
	// count, Nancy 10 by which FrameTextBox AR wrote the text (74 full / 75 strip).
	layoutText(stripTextWidth(), fontID);
	_expanded = g_nancy->getGameType() >= kGameTypeNancy11 ? (_numDrawnLines > 2) : _fullMode;

	if (_expanded) {
		// The expanded box reserves the gutter for its slider, so re-lay-out
		// narrower to clear it.
		layoutText(viewport.width(), fontID);
	}

	const int contentHeight = getInnerHeight();

	Common::Rect boxRect;
	int visibleTextHeight;
	int scrollY = 0;

	if (_expanded) {
		boxRect = _fullPopupRect;
		visibleTextHeight = viewport.height();
		if (contentHeight > visibleTextHeight) {
			scrollY = (int)(_scrollPos * (contentHeight - visibleTextHeight));
		}
	} else {
		// Fixed two-line strip; overflow is clipped (hidden beneath the taskbar).
		// Line 3 starts at exactly 2 * lineStep, so the clip must add no padding.
		const Font *font = g_nancy->_graphics->getFont(fontID);
		const int lineStep = font->getLineHeight() + font->getLineHeight() / 9;
		const int stripContent = 2 * lineStep;
		const int miniHeight = MIN(viewport.top + 1 + stripContent, fullHeight);
		boxRect = Common::Rect(_fullPopupRect.left, _fullPopupRect.top,
							   _fullPopupRect.right, _fullPopupRect.top + miniHeight);
		visibleTextHeight = MIN(contentHeight, stripContent);
	}

	moveTo(boxRect);
	_drawSurface.create(boxRect.width(), boxRect.height(), g_nancy->_graphics->getInputPixelFormat());

	// Expanded text clears the slider gutter; the strip aligns to the taskbar's
	// caption rect (on top of the baked-in lineStartXCursor), or falls back to 0.
	const bool haveStripRect = !_expanded && !_stripScreenRect.isEmpty();
	int textLeft = _expanded ? viewport.left
							 : (haveStripRect ? _stripScreenRect.left - _fullPopupRect.left : 0);
	int textTop = viewport.top;
	if (_expanded) {
		// The textured overlay panel, with its slider.
		setTransparent(false);
		drawBackground();
	} else {
		// The taskbar already draws the strip, so keep it transparent with no chrome.
		setTransparent(true);
		_drawSurface.clear(transColor);
		if (haveStripRect) {
			// The caption rect has a small top inset (the vertical counterpart of
			// lineStartXCursor).
			textTop = (_stripScreenRect.top - _fullPopupRect.top) + 2;
		} else {
			// Nudge the first line down one pixel so its top padding matches the
			// original's caption strip.
			textTop += 1;
		}
	}

	_drawSurface.blitFrom(_fullSurface,
		Common::Rect(0, scrollY, _fullSurface.w, scrollY + visibleTextHeight),
		Common::Point(textLeft, textTop));

	if (_expanded) {
		drawScrollbar(_scrollbarDragging ? kUIButtonPressed : (_scrollbarHovered ? kUIButtonHover : kUIButtonIdle));
	}

	_needsRedraw = true;
}

void ScrollTextBox::redrawScroll() {
	// Only runs while dragging the scrollbar, so the layout is already settled:
	// re-composite the visible slice at the new scroll offset, no re-layout.
	const Common::Rect viewport = textViewportLocal();
	const int visibleTextHeight = viewport.height();
	const int contentHeight = getInnerHeight();
	int scrollY = 0;
	if (contentHeight > visibleTextHeight) {
		scrollY = (int)(_scrollPos * (contentHeight - visibleTextHeight));
	}

	// drawBackground() repaints the chrome, wiping the previous (transparent-keyed)
	// text so the new slice doesn't smear over the old one.
	drawBackground();
	_drawSurface.blitFrom(_fullSurface,
		Common::Rect(0, scrollY, _fullSurface.w, scrollY + visibleTextHeight),
		Common::Point(viewport.left, viewport.top));
	drawScrollbar(_scrollbarDragging ? kUIButtonPressed : (_scrollbarHovered ? kUIButtonHover : kUIButtonIdle));
	_needsRedraw = true;
}

uint16 ScrollTextBox::getInnerHeight() const {
	return _drawnTextHeight + _tboxData->scrollbarDefaultPos.y;
}

Common::Rect ScrollTextBox::toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const {
	Common::Rect r = chunkRect;
	if (useGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			r.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	r.translate(-_screenPosition.left, -_screenPosition.top);
	return r;
}

Common::Point ScrollTextBox::popupLocalMouse(const Common::Point &screenMouse) const {
	return Common::Point(screenMouse.x - _screenPosition.left,
						 screenMouse.y - _screenPosition.top);
}

Common::Rect ScrollTextBox::computeThumbRect() const {
	const UISliderRecord &sl = _header->slider;
	if (!_header->sliderEnabled || sl.destRect.isEmpty() || sl.sourceRects[0].isEmpty()) {
		return Common::Rect();
	}

	const int trackHeight = sl.destRect.height();
	const int thumbHeight = sl.sourceRects[0].height();
	const int travel = MAX(0, trackHeight - thumbHeight);
	const int thumbY = sl.destRect.top + (int)(_scrollPos * travel);

	Common::Rect chunkThumb(sl.destRect.left, thumbY,
							sl.destRect.left + sl.sourceRects[0].width(),
							thumbY + thumbHeight);
	return toPopupLocal(chunkThumb, sl.destUsesGameFrameOffset != 0);
}

void ScrollTextBox::drawScrollbar(UIButtonState state) {
	const UISliderRecord &sl = _header->slider;
	if (!_header->sliderEnabled) {
		return;
	}

	const Common::Rect thumb = computeThumbRect();
	if (thumb.isEmpty()) {
		return;
	}

	_drawSurface.blitFrom(_overlayImage, sl.sourceRects[state],
						  Common::Point(thumb.left, thumb.top));
}

void ScrollTextBox::addTextLine(const Common::String &text, uint32 autoClearTime) {
	HypertextParser::addTextLine(text);

	if (autoClearTime != 0) {
		_autoClearTime = g_nancy->getTotalPlayTime() + autoClearTime;
	}

	drawContent();
	setVisible(true);
}

void ScrollTextBox::setOverrideFont(uint fontID) {
	auto *bsum = GetEngineData(BSUM);
	assert(bsum);

	if (fontID >= bsum->numFonts) {
		error("Requested invalid override font ID %u in ScrollTextBox", fontID);
	}

	_fontIDOverride = (int)fontID;
}

void ScrollTextBox::setFullMode(bool fullMode) {
	// Nancy 11+ has no second box; the mode only matters for Nancy 10.
	if (g_nancy->getGameType() >= kGameTypeNancy11 || _fullMode == fullMode) {
		return;
	}

	_fullMode = fullMode;

	// The AR writes its text before selecting the mode, so re-render.
	if (_isVisible) {
		drawContent();
	}
}

void ScrollTextBox::clear() {
	HypertextParser::clear();

	_scrollPos = 0.0f;
	_scrollbarDragging = false;
	_scrollbarHovered = false;
	_fullMode = false;
	_expanded = false;
	_fontIDOverride = -1;
	_autoClearTime = 0;
	_drawnTextHeight = 0;

	const uint32 transColor = g_nancy->_graphics->getTransColor();
	_fullSurface.fillRect(Common::Rect(0, 0, _fullSurface.w, _fullSurface.h), transColor);
	_textHighlightSurface.fillRect(Common::Rect(0, 0, _textHighlightSurface.w, _textHighlightSurface.h), transColor);

	setVisible(false);
}

void ScrollTextBox::handleInput(NancyInput &input) {
	if (!_isVisible) {
		return;
	}

	const Common::Point localMouse = popupLocalMouse(input.mousePos);
	const UISliderRecord &slider = _header->slider;

	// The scrollbar only exists on the expanded box.
	if (_header->sliderEnabled && _expanded) {
		const Common::Rect trackLocal = toPopupLocal(slider.destRect, slider.destUsesGameFrameOffset != 0);
		const int trackHeight = trackLocal.height();
		const int thumbHeight = slider.sourceRects[0].height();
		const int travel = MAX(0, trackHeight - thumbHeight);
		const int thumbY = trackLocal.top + (int)(_scrollPos * travel);
		Common::Rect thumbLocal(trackLocal.left, thumbY,
								trackLocal.left + slider.sourceRects[0].width(),
								thumbY + thumbHeight);
		const bool overThumb = thumbLocal.contains(localMouse);

		if (_scrollbarDragging) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

			const int newThumbTop = localMouse.y - _scrollbarGrabOffset;
			const int clamped = CLIP<int>(newThumbTop, trackLocal.top, trackLocal.top + travel);
			const float newScrollPos = travel > 0 ? (float)(clamped - trackLocal.top) / (float)travel : 0.0f;

			bool released = false;
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_scrollbarDragging = false;
				released = true;
			}

			// Re-composite (via the cheap redrawScroll()) only when the thumb moves
			// or the drag ends, to keep dragging smooth.
			if (newScrollPos != _scrollPos || released) {
				_scrollPos = newScrollPos;
				redrawScroll();
			}

			input.eatMouseInput();
			return;
		}

		if (scrollWithMouseWheel(input, _screenPosition, _scrollPos,
				wheelScrollStep(textViewportLocal().height(), getInnerHeight()))) {
			redrawScroll();
		}

		if (overThumb != _scrollbarHovered) {
			_scrollbarHovered = overThumb;
			drawScrollbar(overThumb ? kUIButtonHover : kUIButtonIdle);
			_needsRedraw = true;
		}

		if (overThumb) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (slider.isDraggable && (input.input & NancyInput::kLeftMouseButtonDown)) {
				_scrollbarDragging = true;
				_scrollbarGrabOffset = localMouse.y - thumbY;
				drawScrollbar(kUIButtonPressed);
				_needsRedraw = true;
				input.eatMouseInput();
				return;
			}
		}
	}

	// Don't swallow clicks over the whole rect: the box overlaps the taskbar and
	// captions can persist, so only the scrollbar consumes input.
}

} // End of namespace UI
} // End of namespace Nancy
