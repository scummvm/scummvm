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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/notebookpopup.h"

namespace Nancy {
namespace UI {

NotebookPopup::NotebookPopup() :
		// z=12: above the viewport (6) and the taskbar (7). All Nancy
		// 10+ taskbar popups render on top of the entire scene UI.
		RenderObject(12),
		_uinbData(nullptr),
		_activeTab(0) {}

// Cap on how tall HypertextParser's working surface can grow. Notebook
// journals on Nancy 10+ rarely exceed a few hundred wrapped lines; this
// gives plenty of headroom while keeping the allocation bounded.
static const uint16 kHypertextSurfaceHeight = 4096;

void NotebookPopup::init() {
	_uinbData = GetEngineData(UINB);
	assert(_uinbData);

	g_nancy->_resource->loadImage(_uinbData->header.imageName, _overlayImage);

	// Close (X) button image.
	if (_uinbData->header.secondaryButtonEnabled &&
		!_uinbData->header.secondaryButton.primaryImageName.empty()) {
		g_nancy->_resource->loadImage(_uinbData->header.secondaryButton.primaryImageName,
										_closeButtonImage);
	}

	Common::Rect popupRect = _uinbData->header.normalDestRect;
	if (_uinbData->header.overlayInGameFrame) {
		const VIEW *view = GetEngineData(VIEW);
		if (view) {
			popupRect.translate(view->screenPosition.left, view->screenPosition.top);
		}
	}
	moveTo(popupRect);
	Common::Rect bounds = _screenPosition;
	bounds.moveTo(0, 0);
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getInputPixelFormat());

	// Set up HypertextParser's scratch surfaces. Width matches the
	// chunk's text rect; height is generously oversized so journal
	// content for any plausible save state fits without truncation
	// (overflow is handled by scrolling, not by growing the surface).
	// Background color is irrelevant — paintPaperIntoFullSurface()
	// re-tiles the popup's paper texture after every clear() so the
	// text always sits on real notebook paper.
	initSurfaces(_uinbData->textRect.width(), kHypertextSurfaceHeight,
		g_nancy->_graphics->getInputPixelFormat(), 0, 0);

	// Pick the first enabled tab as the initially active one
	_activeTab = 0;
	for (uint i = 0; i < kNumTabs; ++i) {
		if (_uinbData->tabs[i].enabled) {
			_activeTab = (int)i;
			break;
		}
	}

	drawBackground();
	drawTabs();
	drawContent();
	drawForeground();

	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void NotebookPopup::registerGraphics() {
	RenderObject::registerGraphics();
}

void NotebookPopup::open() {
	if (_isVisible)
		return;

	setVisible(true);

	// JournalData entries may have changed since the last open (added by
	// ModifyListEntry, marked complete, etc.) — re-render content.
	refreshContent();

	if (!_uinbData->header.sounds[0].name.empty()) {
		g_nancy->_sound->loadSound(_uinbData->header.sounds[0]);
		g_nancy->_sound->playSound(_uinbData->header.sounds[0]);
	}
}

void NotebookPopup::close() {
	if (!_isVisible)
		return;

	setVisible(false);

	if (!_uinbData->header.sounds[1].name.empty()) {
		g_nancy->_sound->loadSound(_uinbData->header.sounds[1]);
		g_nancy->_sound->playSound(_uinbData->header.sounds[1]);
	}
}

void NotebookPopup::drawBackground() {
	_drawSurface.blitFrom(_overlayImage, _uinbData->header.normalSrcRect, Common::Point(0, 0));
}

void NotebookPopup::drawForeground() {
	drawCloseButton(_closeButtonHovered ? kStateHover : kStateIdle);

	WidgetState sliderState = kStateIdle;
	if (_scrollbarDragging) {
		sliderState = kStatePressed;
	} else if (_scrollbarHovered) {
		sliderState = kStateHover;
	}
	drawScrollbar(sliderState);
}

Common::Rect NotebookPopup::toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const {
	// Build the element's absolute screen rect: apply the viewport
	// offset iff its own `destUsesGameFrameOffset` flag is set, then
	// subtract the popup's absolute screen position. `_screenPosition`
	// already includes the popup's own game-frame translation, so this
	// works correctly regardless of which combination of flags the
	// chunk uses.
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

Common::Point NotebookPopup::popupLocalMouse(const Common::Point &screenMouse) const {
	return Common::Point(screenMouse.x - _screenPosition.left,
							screenMouse.y - _screenPosition.top);
}

Common::Rect NotebookPopup::computeThumbRect() const {
	const UISliderRecord &sl = _uinbData->header.slider;
	if (!_uinbData->header.sliderEnabled || sl.destRect.isEmpty() || sl.sourceRects[0].isEmpty()) {
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

void NotebookPopup::drawScrollbar(WidgetState state) {
	const UISliderRecord &sl = _uinbData->header.slider;
	if (!_uinbData->header.sliderEnabled)
		return;

	Common::Rect spr = sl.sourceRects[state];
	const Common::Rect thumb = computeThumbRect();
	if (thumb.isEmpty())
		return;

	_drawSurface.blitFrom(_overlayImage, spr, Common::Point(thumb.left, thumb.top));
}

void NotebookPopup::drawCloseButton(WidgetState state) {
	const UIButtonRecord &btn = _uinbData->header.secondaryButton;
	if (!_uinbData->header.secondaryButtonEnabled || btn.destRect.isEmpty())
		return;

	Common::Rect spr = btn.sourceRects[state];
	const Common::Rect dstLocal = toPopupLocal(btn.destRect, btn.destUsesGameFrameOffset != 0);

	const Graphics::ManagedSurface &srcSurf = _closeButtonImage.w != 0 ? _closeButtonImage : _overlayImage;
	_drawSurface.blitFrom(srcSurf, spr, Common::Point(dstLocal.left, dstLocal.top));
}

void NotebookPopup::drawTabs() {
	for (uint i = 0; i < kNumTabs; ++i) {
		drawTab(i);
	}

	_needsRedraw = true;
}

void NotebookPopup::drawTab(uint index, bool drawHover) {
	const UIButtonSlot &tab = _uinbData->tabs[index];
	if (!tab.enabled)
		return;

	WidgetState stateIdx = ((int)index == _activeTab) ? kStatePressed : kStateIdle;
	if (drawHover)
		stateIdx = kStateHover;
	Common::Rect src = tab.button.sourceRects[stateIdx];
	if (src.isEmpty()) {
		src = tab.button.sourceRects[kStatePressed];
	}
	if (src.isEmpty())
		return;

	const Common::Rect dstLocal = toPopupLocal(tab.button.destRect,
												tab.button.destUsesGameFrameOffset != 0);
	_drawSurface.blitFrom(_overlayImage, src,
							Common::Point(dstLocal.left, dstLocal.top));
	_needsRedraw = true;
}

void NotebookPopup::handleInput(NancyInput &input) {
	if (!_isVisible)
		return;

	const Common::Point localMouse = popupLocalMouse(input.mousePos);

	// Scrollbar interaction takes priority while dragging.
	const UISliderRecord &slider = _uinbData->header.slider;
	if (_uinbData->header.sliderEnabled) {
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
			refreshContent();

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_scrollbarDragging = false;
				drawScrollbar(overThumb ? kStateHover : kStateIdle);
				_needsRedraw = true;
			}
			input.eatMouseInput();
			return;
		}

		if (overThumb != _scrollbarHovered) {
			_scrollbarHovered = overThumb;
			drawScrollbar(overThumb ? kStateHover : kStateIdle);
			_needsRedraw = true;
		}
		if (overThumb) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (slider.isDraggable && (input.input & NancyInput::kLeftMouseButtonDown)) {
				_scrollbarDragging = true;
				_scrollbarGrabOffset = localMouse.y - thumbY;
				drawScrollbar(kStatePressed);
				_needsRedraw = true;
				input.eatMouseInput();
				return;
			}
		}
	}

	// Close (X) button takes priority.
	if (_uinbData->header.secondaryButtonEnabled) {
		const UIButtonRecord &closeBtn = _uinbData->header.secondaryButton;
		const Common::Rect closeLocal = toPopupLocal(closeBtn.destRect,
														closeBtn.destUsesGameFrameOffset != 0);
		const bool overClose = closeLocal.contains(localMouse);
		if (overClose != _closeButtonHovered) {
			_closeButtonHovered = overClose;
			drawCloseButton(overClose ? kStateHover : kStateIdle);
			_needsRedraw = true;
		}
		if (overClose) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				input.eatMouseInput();
				close();
				return;
			}
		}
	}

	// Tab hover + click. Mirrors the inventory popup's filter-tab
	// handling so the same enter/exit redraw semantics apply: track
	// whether any tab is hovered, restore non-hovered tabs to their
	// idle/active sprite on transitions, and pop the hover sprite for
	// the one currently under the cursor.
	const bool wasTabHovered = _tabHovered;
	_tabHovered = false;
	for (uint i = 0; i < kNumTabs; ++i) {
		const UIButtonSlot &tab = _uinbData->tabs[i];
		if (!tab.enabled)
			continue;
		const Common::Rect tabLocal = toPopupLocal(tab.button.destRect,
													tab.button.destUsesGameFrameOffset != 0);
		if (tabLocal.contains(localMouse)) {
			_tabHovered = true;
			break;
		}
	}

	for (uint i = 0; i < kNumTabs; ++i) {
		const UIButtonSlot &tab = _uinbData->tabs[i];
		if (!tab.enabled)
			continue;

		const Common::Rect tabLocal = toPopupLocal(tab.button.destRect,
													tab.button.destUsesGameFrameOffset != 0);
		if (!tabLocal.contains(localMouse)) {
			// Restore the idle/active sprite when the cursor has just
			// left this tab (or any tab) so the hover highlight doesn't
			// linger after the mouse moves away.
			if (_tabHovered || wasTabHovered)
				drawTab(i);
			continue;
		}

		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		drawTab(i, true);

		if (input.input & NancyInput::kLeftMouseButtonUp) {
			if (_activeTab != (int)i) {
				_activeTab = (int)i;
				_scrollPos = 0.0f;
				_scrollbarDragging = false;

				// Play the page-flip sound (first slot of either
				// actionable or no-action set; both have 3 alternates).
				const Common::Path &soundName = _uinbData->noActionClickSounds[0];
				if (!soundName.empty()) {
					g_nancy->_sound->playSound(soundName.toString());
				}

				refreshContent();
			}
			input.eatMouseInput();
			return;
		}
		break;
	}

	// Swallow clicks on the popup itself so they don't fall through.
	if (_screenPosition.contains(input.mousePos)) {
		input.eatMouseInput();
	}
}

void NotebookPopup::refreshContent() {
	// Re-blit the popup background so previous text is wiped, then put
	// the active tab sprite back on top of it before drawing new text.
	// Foreground widgets (close button, slider) are painted last so they
	// always sit visually above the text layer.
	drawBackground();
	drawTabs();
	drawContent();
	drawForeground();
}

void NotebookPopup::paintPaperIntoFullSurface() {
	const Common::Rect &normSrc = _uinbData->header.normalSrcRect;
	const Common::Rect &normDest = _uinbData->header.normalDestRect;
	const Common::Rect &chunkTextRect = _uinbData->textRect;

	const int16 paperLeft = normSrc.left + (chunkTextRect.left - normDest.left);
	const int16 paperTop  = normSrc.top  + (chunkTextRect.top  - normDest.top);
	const Common::Rect paperSrc(paperLeft, paperTop,
								paperLeft + chunkTextRect.width(),
								paperTop  + chunkTextRect.height());

	const int stripH = paperSrc.height();
	if (stripH <= 0)
		return;

	int y = 0;
	while (y < (int)_fullSurface.h) {
		const int rowH = MIN<int>(stripH, (int)_fullSurface.h - y);
		Common::Rect src = paperSrc;
		src.bottom = src.top + rowH;
		_fullSurface.blitFrom(_overlayImage, src, Common::Point(0, y));
		y += rowH;
	}
}

void NotebookPopup::buildTextLines() {
	if (!_uinbData)
		return;

	const UIButtonSlot &tab = _uinbData->tabs[_activeTab];
	if (!tab.enabled)
		return;

	const uint16 surfaceID = (uint16)tab.id + 2;

	JournalData *journalData = (JournalData *)NancySceneState.getPuzzleData(JournalData::getTag());
	if (!journalData)
		return;

	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");
	if (!autotext)
		return;

	// Senior-detective Tasks page: hide the to-do list and show the
	// AUTOTEXT placeholder body instead.
	if (surfaceID == kNotebookTabTasks && NancySceneState.getDifficulty() == 2) {
		// TODO: This is specific for Nancy10, adapt it for others, too
		if (autotext->texts.contains("SHAT70")) {
			addTextLine(autotext->texts["SHAT70"]);
		}
		return;
	}

	if (!journalData->journalEntries.contains(surfaceID))
		return;

	const Common::Array<JournalData::Entry> &entries = journalData->journalEntries[surfaceID];
	for (uint i = 0; i < entries.size(); ++i) {
		const Common::String &stringID = entries[i].stringID;
		if (!autotext->texts.contains(stringID))
			continue;

		Common::String body = autotext->texts[stringID];
		if (body.empty())
			continue;

		// Tasks are prefixed with a checkbox showing completion state.
		// mark % 10 == 8 means "complete".
		if (surfaceID == kNotebookTabTasks) {
			const uint16 markStatus = entries[i].mark % 10;
			body = Common::String(markStatus == 8 ? "<2>" : "<1>") + body;
		}

		addTextLine(body);
	}
}

void NotebookPopup::drawContent() {
	if (!_uinbData) {
		return;
	}

	// textRect from UINB is in chunk coords (relative to normalDestRect);
	// convert to popup-local for the on-surface blit destination.
	Common::Rect localTextRect = _uinbData->textRect;
	localTextRect.translate(-_uinbData->header.normalDestRect.left,
							-_uinbData->header.normalDestRect.top);

	// Reset HypertextParser state, repaint the paper background under
	// the text layer, then route content through the shared pipeline.
	HypertextParser::clear();
	paintPaperIntoFullSurface();
	buildTextLines();

	const uint16 fontID = _uinbData->primaryFontID;
	Common::Rect hypertextBounds(0, 0, _fullSurface.w, _fullSurface.h);
	drawAllText(hypertextBounds, 0, fontID, fontID);

	// Blit the scrolled vertical slice of the rendered hypertext onto
	// the popup surface. _drawnTextHeight is the inclusive height of
	// the rendered content; anything past localTextRect.height() is
	// reachable via the slider.
	const int visibleH = localTextRect.height();
	const int maxScroll = MAX<int>(0, (int)_drawnTextHeight - visibleH);
	const int scrollY = (int)(_scrollPos * maxScroll);
	Common::Rect srcSlice(0, scrollY,
							_fullSurface.w, scrollY + visibleH);
	_drawSurface.blitFrom(_fullSurface, srcSlice,
							Common::Point(localTextRect.left, localTextRect.top));

	_needsRedraw = true;
}

} // End of namespace UI
} // End of namespace Nancy
