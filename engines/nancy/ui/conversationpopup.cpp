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

#include "engines/nancy/ui/conversationpopup.h"

namespace Nancy {
namespace UI {

static const uint16 kHypertextSurfaceHeight = 4096;

ConversationPopup::ConversationPopup() :
		RenderObject(12), // z=12: above the viewport (6) and the taskbar (7).
		_uicoData(nullptr),
		_tboxData(nullptr),
		_highlightRObj(13) {}

void ConversationPopup::init() {
	_uicoData = GetEngineData(UICO);
	assert(_uicoData);
	_tboxData = GetEngineData(TBOX);
	assert(_tboxData);

	g_nancy->_resource->loadImage(_uicoData->header.imageName, _overlayImage);

	Common::Rect popupRect = _uicoData->header.normalDestRect;
	if (_uicoData->header.overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			popupRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	moveTo(popupRect);

	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	initSurfaces(_uicoData->textRect.width(), kHypertextSurfaceHeight,
				 g_nancy->_graphics->getInputPixelFormat(), 0, 0);

	_highlightRObj.moveTo(popupRect);

	drawBackground();
	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void ConversationPopup::registerGraphics() {
	RenderObject::registerGraphics();
	_highlightRObj.registerGraphics();
	_highlightRObj.setVisible(false);
}

void ConversationPopup::open() {
	// Always clear, even if already visible — the conversation action
	// calls open() on every new dialogue state (matching Textbox::clear()
	// behavior for Nancy 1-9).
	_scrollPos = 0.0f;
	_scrollbarDragging = false;
	_scrollbarHovered = false;
	_responseStartIdx = 0;

	HypertextParser::clear();
	_drawnTextHeight = 0;
	_fullSurface.fillRect(Common::Rect(0, 0, _fullSurface.w, _fullSurface.h), 0);
	_textHighlightSurface.fillRect(Common::Rect(0, 0, _textHighlightSurface.w, _textHighlightSurface.h), 0);

	drawBackground();
	drawScrollbar(0);
	setVisible(true);
}

void ConversationPopup::addTextLine(const Common::String &text) {
	HypertextParser::addTextLine(text);

	drawBackground();
	drawContent();
	drawScrollbar(0);
}

void ConversationPopup::close() {
	setVisible(false);
	_highlightRObj.setVisible(false);
}

void ConversationPopup::drawBackground() {
	_drawSurface.blitFrom(_overlayImage, _uicoData->header.normalSrcRect,
						  Common::Point(0, 0));
	_needsRedraw = true;
}

void ConversationPopup::drawContent() {
	_drawnTextHeight = 0;
	_numDrawnLines = 0;
	_hotspots.clear();
	_fullSurface.fillRect(Common::Rect(0, 0, _fullSurface.w, _fullSurface.h), 0);
	_textHighlightSurface.fillRect(Common::Rect(0, 0, _textHighlightSurface.w, _textHighlightSurface.h), 0);

	// TODO: Padding doesn't match the original game. leftOffset returns an
	// unexpected value for Nancy 10+, so upOffset is used as a fallback for
	// both axes until the TBOX layout for Nancy 10+ is better understood.

	// TODO: Line spacing is tighter than the original game.
	
	Common::Rect textBounds(0, 0, _fullSurface.w, _fullSurface.h);
	textBounds.top += _tboxData->upOffset;
	textBounds.left += _tboxData->upOffset;

	drawAllText(textBounds, 0, _tboxData->conversationFontID, _tboxData->highlightConversationFontID);

	Common::Rect localTextRect = _uicoData->textRect;
	localTextRect.translate(-_uicoData->header.normalDestRect.left,
							-_uicoData->header.normalDestRect.top);

	const uint16 inner = getInnerHeight();
	const uint16 outer = localTextRect.height();
	int scrollY = 0;
	if (inner > outer) {
		scrollY = MIN<int>((int)(_scrollPos * (inner - outer)),
						   MAX<int>(0, _fullSurface.h - outer));
	}

	_drawSurface.blitFrom(_fullSurface,
		Common::Rect(0, scrollY, _fullSurface.w, scrollY + outer),
		Common::Point(localTextRect.left, localTextRect.top));

	_needsRedraw = true;
}

uint16 ConversationPopup::getInnerHeight() const {
	return _drawnTextHeight + _tboxData->upOffset;
}

Common::Rect ConversationPopup::toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const {
	// Chunk-space rects need the viewport offset applied when their own
	// destUsesGameFrameOffset flag is set, then the popup's screen
	// position subtracted to get surface-local coordinates.
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

Common::Point ConversationPopup::popupLocalMouse(const Common::Point &screenMouse) const {
	return Common::Point(screenMouse.x - _screenPosition.left,
						 screenMouse.y - _screenPosition.top);
}

Common::Rect ConversationPopup::computeThumbRect() const {
	const UISliderRecord &sl = _uicoData->header.slider;
	if (!_uicoData->header.sliderEnabled || sl.destRect.isEmpty() || sl.sourceRects[0].isEmpty()) {
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

void ConversationPopup::drawScrollbar(uint state) {
	const UISliderRecord &sl = _uicoData->header.slider;
	if (!_uicoData->header.sliderEnabled) {
		return;
	}

	const Common::Rect thumb = computeThumbRect();
	if (thumb.isEmpty()) {
		return;
	}

	_drawSurface.blitFrom(_overlayImage, sl.sourceRects[state],
						  Common::Point(thumb.left, thumb.top));
}

void ConversationPopup::handleInput(NancyInput &input) {
	if (!_isVisible) {
		return;
	}

	const Common::Point localMouse = popupLocalMouse(input.mousePos);
	const UISliderRecord &slider = _uicoData->header.slider;

	if (_uicoData->header.sliderEnabled) {
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
			_scrollPos = travel > 0 ? (float)(clamped - trackLocal.top) / (float)travel : 0.0f;

			drawBackground();
			drawContent();
			drawScrollbar(2);

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_scrollbarDragging = false;
				drawScrollbar(overThumb ? 1 : 0);
				_needsRedraw = true;
			}
			input.eatMouseInput();
			return;
		}

		if (overThumb != _scrollbarHovered) {
			_scrollbarHovered = overThumb;
			drawScrollbar(overThumb ? 1 : 0);
			_needsRedraw = true;
		}

		if (overThumb) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (slider.isDraggable && (input.input & NancyInput::kLeftMouseButtonDown)) {
				_scrollbarDragging = true;
				_scrollbarGrabOffset = localMouse.y - thumbY;
				drawScrollbar(2);
				_needsRedraw = true;
				input.eatMouseInput();
				return;
			}
		}
	}

	// Response hotspot handling — mirrors Textbox::handleInput().
	Common::Rect localTextRect = _uicoData->textRect;
	localTextRect.translate(-_uicoData->header.normalDestRect.left,
							-_uicoData->header.normalDestRect.top);

	const uint16 inner = getInnerHeight();
	const uint16 outer = localTextRect.height();
	const int scrollY = inner > outer ? (int)(_scrollPos * (inner - outer)) : 0;

	bool hasHighlight = false;
	for (uint i = _responseStartIdx; i < _hotspots.size(); ++i) {
		Common::Rect hotspot = _hotspots[i];
		hotspot.translate(localTextRect.left, localTextRect.top - scrollY);
		Common::Rect hotspotOnScreen = convertToScreen(hotspot).findIntersectingRect(_screenPosition);
		if (hotspotOnScreen.contains(input.mousePos)) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);

			_highlightRObj.setVisible(true);
			Common::Rect hotspotInside = convertToLocal(hotspotOnScreen);
			hotspotInside.translate(-localTextRect.left, scrollY - localTextRect.top);
			_highlightRObj._drawSurface.create(_textHighlightSurface, hotspotInside);
			_highlightRObj.moveTo(hotspotOnScreen);
			hasHighlight = true;

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.input &= ~NancyInput::kLeftMouseButtonUp;
				NancySceneState.clearLogicConditions();
				NancySceneState.setLogicCondition(i - _responseStartIdx, g_nancy->_true);
			}

			break;
		}
	}

	if (!hasHighlight && _highlightRObj.isVisible()) {
		_highlightRObj.setVisible(false);
	}

	// Swallow clicks on the popup itself so they don't fall through.
	if (_screenPosition.contains(input.mousePos)) {
		input.eatMouseInput();
	}
}

} // End of namespace UI
} // End of namespace Nancy
