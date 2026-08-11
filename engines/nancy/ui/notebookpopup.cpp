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

#include "common/random.h"
#include "common/system.h"

#include "engines/nancy/cursor.h"
#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"
#include "engines/nancy/input.h"
#include "engines/nancy/nancy.h"
#include "engines/nancy/puzzledata.h"
#include "engines/nancy/resource.h"
#include "engines/nancy/sound.h"

#include "engines/nancy/state/scene.h"

#include "engines/nancy/ui/taskbar.h"

#include "engines/nancy/ui/notebookpopup.h"

namespace Nancy {
namespace UI {

NotebookPopup::NotebookPopup() :
		// z=12: above the viewport (6) and the taskbar (7). All Nancy
		// 10+ taskbar popups render on top of the entire scene UI.
		RenderObject(12),
		_uinbData(nullptr),
		_activeTab(0) {}

// Working-surface height for HypertextParser. Matches the original
// engine's allocation so long journals don't get clipped.
static const uint16 kHypertextSurfaceHeight = 16000;

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
	_drawSurface.create(bounds.width(), bounds.height(), g_nancy->_graphics->getScreenPixelFormat());

	// Transparent-keyed scratch surfaces so text blits over the paper
	// painted by drawBackground() — paper stays stationary while text
	// scrolls. Color 0 would clip real font pixels in Nancy fonts.
	const uint32 trans = g_nancy->_graphics->getTransColor();
	initSurfaces(_uinbData->textRect.width(), kHypertextSurfaceHeight,
		g_nancy->_graphics->getInputPixelFormat(), trans, trans);
	_fullSurface.setTransparentColor(trans);
	_textHighlightSurface.setTransparentColor(trans);

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
	drawCaption();
	drawContent();
	drawForeground();

	setTransparent(false);
	setVisible(false);

	RenderObject::init();
}

void NotebookPopup::registerGraphics() {
	RenderObject::registerGraphics();
}

void NotebookPopup::updateGraphics() {
	// Fire the deferred "I'm finished with that" line once its delay elapses.
	if (_completeVoiceTime != 0 && g_system->getMillis() >= _completeVoiceTime) {
		_completeVoiceTime = 0;
		playCheckboxSound(true);
	}
}

int16 NotebookPopup::getPrepSceneID() const {
	return _uinbData ? (int16)_uinbData->header.linkbackScene : (int16)kNoScene;
}

void NotebookPopup::open() {
	if (_isVisible)
		return;

	setVisible(true);

	g_nancy->_cursor->warpCursor(Common::Point(_screenPosition.left + _screenPosition.width() / 2,
												_screenPosition.top + _screenPosition.height() / 2));

	// Only the tab being shown is acknowledged; the other keeps its badge.
	clearActiveTabNotification();

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
}

void NotebookPopup::drawBackground() {
	_drawSurface.blitFrom(_overlayImage, _uinbData->header.normalSrcRect, Common::Point(0, 0));
}

void NotebookPopup::drawForeground() {
	drawCloseButton(_closeButtonHovered);

	drawScrollbar(_scrollbarDragging ? kUIButtonPressed : (_scrollbarHovered ? kUIButtonHover : kUIButtonIdle));
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

void NotebookPopup::drawScrollbar(UIButtonState state) {
	const UISliderRecord &sl = _uinbData->header.slider;
	if (!_uinbData->header.sliderEnabled)
		return;

	Common::Rect spr = sl.sourceRects[state];
	const Common::Rect thumb = computeThumbRect();
	if (thumb.isEmpty())
		return;

	_drawSurface.blitFrom(_overlayImage, spr, Common::Point(thumb.left, thumb.top));
}

void NotebookPopup::drawCloseButton(bool hovered) {
	const UIButtonRecord &btn = _uinbData->header.secondaryButton;
	if (!_uinbData->header.secondaryButtonEnabled || btn.destRect.isEmpty())
		return;

	Common::Rect spr = btn.sourceRects[hovered ? kUIButtonHover : kUIButtonIdle];
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

void NotebookPopup::drawCaption() {
	if ((uint)_activeTab >= _uinbData->tabCaptionSrcRects.size()) {
		return;
	}
	const Common::Rect &spr = _uinbData->tabCaptionSrcRects[_activeTab];
	if (spr.isEmpty() || _uinbData->tabCaptionDestRect.isEmpty()) {
		return;
	}

	// Use the same game-frame-aware conversion as the tabs / close button so
	// the caption lines up with them when the popup overlays the game frame.
	const Common::Rect dstLocal = toPopupLocal(_uinbData->tabCaptionDestRect, false);
	_drawSurface.blitFrom(_overlayImage, spr, Common::Point(dstLocal.left, dstLocal.top));
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
			const float newScrollPos = travel > 0 ? (float)(clamped - trackLocal.top) / (float)travel : 0.0f;

			// Re-composite only when the thumb actually moves, and via redrawScroll()
			// (a cheap slice re-blit) rather than a full text re-layout. This keeps
			// dragging smooth instead of re-rendering the whole journal each frame.
			if (newScrollPos != _scrollPos) {
				_scrollPos = newScrollPos;
				redrawScroll();
			}

			if (input.input & NancyInput::kLeftMouseButtonUp) {
				_scrollbarDragging = false;
				drawScrollbar(overThumb ? kUIButtonHover : kUIButtonIdle);
				_needsRedraw = true;
			}
			input.eatMouseInput();
			return;
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

	// Close (X) button takes priority.
	if (_uinbData->header.secondaryButtonEnabled) {
		const UIButtonRecord &closeBtn = _uinbData->header.secondaryButton;
		const Common::Rect closeLocal = toPopupLocal(closeBtn.destRect,
														closeBtn.destUsesGameFrameOffset != 0);
		const bool overClose = closeLocal.contains(localMouse);
		if (overClose != _closeButtonHovered) {
			_closeButtonHovered = overClose;
			drawCloseButton(overClose);
			_needsRedraw = true;
		}
		if (overClose) {
			g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
			if (input.input & NancyInput::kLeftMouseButtonUp) {
				playButtonClickSound(closeBtn);
				input.eatMouseInput();
				close();
				return;
			}
		}
	}

	// Tasklist checkboxes: an unchecked box gets a hotspot cursor and, on
	// click, either checks off (event flag satisfied) or plays the rejection
	// line. Checked before the tabs since the boxes sit inside the text area.
	for (uint k = 0; k < _checkboxRects.size(); ++k) {
		if (!_checkboxRects[k].contains(localMouse)) {
			continue;
		}
		g_nancy->_cursor->setCursorType(CursorManager::kHotspotArrow);
		if (input.input & NancyInput::kLeftMouseButtonUp) {
			toggleCheckbox(_checkboxEntryIndices[k]);
		}
		// Swallow the input (as the popup does for its other widgets) so the
		// viewport / action manager behind the popup don't override the hotspot
		// cursor we just set.
		input.eatMouseInput();
		return;
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

				playButtonClickSound(tab.button);
				clearActiveTabNotification();
				refreshContent();
			}
			input.eatMouseInput();
			return;
		}
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
	drawCaption();
	drawContent();
	drawForeground();
}

uint16 NotebookPopup::notebookJournalTabId() const {
	return g_nancy->getGameType() >= kGameTypeNancy13 ? 0 : 1;
}

void NotebookPopup::clearActiveTabNotification() {
	if (!_uinbData || (uint)_activeTab >= kNumTabs) {
		return;
	}
	UI::Taskbar *taskbar = NancySceneState.getTaskbar();
	if (!taskbar) {
		return;
	}
	// Journal = sub 0, Tasks = sub 1 (see ModifyListEntry).
	const bool journalActive = (_uinbData->tabs[_activeTab].id == notebookJournalTabId());
	taskbar->clearNotification(kTaskButtonNotebook, journalActive ? 0 : 1);
}

void NotebookPopup::buildTextLines() {
	if (!_uinbData)
		return;

	const UIButtonSlot &tab = _uinbData->tabs[_activeTab];
	if (!tab.enabled)
		return;

	// The lower tab id (top/book) is the Journal; the higher (bottom/clipboard)
	// is the Tasks list. Nancy 13 renumbered the tab ids from {1,2} to {0,1}.
	const uint16 journalTabId = notebookJournalTabId();
	const uint16 surfaceID = (tab.id == journalTabId) ? kNotebookTabJournal : kNotebookTabTasks;

	JournalData *journalData = (JournalData *)NancySceneState.getPuzzleData(JournalData::getTag());
	if (!journalData)
		return;

	const CVTX *autotext = (const CVTX *)g_nancy->getEngineData("AUTOTEXT");

	// Senior-detective Tasks: chunk supplies a CVTX placeholder.
	if (surfaceID == kNotebookTabTasks && NancySceneState.getDifficulty() != 0 &&
			_uinbData->useFilenameTextFlag != 0 && autotext) {
		Common::String key = _uinbData->conditionalTextFilename.toString();
		if (!key.empty() && autotext->texts.contains(key)) {
			addTextLine(autotext->texts[key]);
		}
		return;
	}

	if (!journalData->journalEntries.contains(surfaceID))
		return;

	// Newest-first. All entries go into one addTextLine — separate
	// calls would put every mark on its own "first line" and stack
	// them at the textbox top.
	const Common::Array<JournalData::Entry> &entries = journalData->journalEntries[surfaceID];
	Common::String combined;
	for (int i = (int)entries.size() - 1; i >= 0; --i) {
		Common::String stringID = entries[i].stringID;
		Common::String body = autotext->texts.getValOrDefault(stringID, "");

		if (surfaceID == kNotebookTabTasks && entries[i].mark != 0) {
			uint16 markValue = entries[i].mark;
			if (markValue == 8) {
				markValue = _uinbData->secondaryFontAttr;
			} else if (markValue == 7) {
				// Engine maps <7> -> sprite index 0, same as <1>.
				markValue = 1;
			}
			if (markValue >= 1 && markValue <= 5) {
				body = Common::String::format("<%u>", markValue) + body;
				// Record which entry this mark belongs to so the recorded
				// mark hotspots (in the same draw order) map back to entries.
				_markEntryIndices.push_back((uint)i);
			}
		}

		// Entries concatenate directly; each entry's text already ends with its
		// own newline, so an extra <n> here would double the inter-entry gap.
		combined += body;
	}

	if (!combined.empty()) {
		addTextLine(combined);
	}
}

void NotebookPopup::drawContent() {
	layoutText();
	paintVisibleText();
}

void NotebookPopup::layoutText() {
	if (!_uinbData) {
		return;
	}

	HypertextParser::clear();
	_checkboxRects.clear();
	_checkboxEntryIndices.clear();
	_markEntryIndices.clear();

	// Only the Tasklist has clickable checkboxes; record their glyph rects.
	const UIButtonSlot &activeTab = _uinbData->tabs[_activeTab];
	const bool tasksTab = activeTab.enabled && activeTab.id != notebookJournalTabId();
	_recordMarkHotspots = tasksTab;

	buildTextLines();

	// Chunk's textRect already provides top padding from the chrome.
	// A small left inset gives breathing room. The original draws text
	// across the full text-rect height (source blit height == dest rect
	// height), so no bottom strip is reserved.
	const uint16 fontID = _uinbData->primaryFontID;
	const Font *font = g_nancy->_graphics->getFont(fontID);
	const int leftInset = font ? font->getCharWidth('o') : 0;

	Common::Rect hypertextBounds(leftInset, 0, _fullSurface.w, _fullSurface.h);
	drawAllText(hypertextBounds, 0, fontID, fontID);
}

void NotebookPopup::paintVisibleText() {
	if (!_uinbData) {
		return;
	}

	// Convert the text rect to popup-local with the same game-frame-aware
	// conversion the tabs / close button use, so text and caption stay aligned
	// with them when the popup overlays the game frame.
	const Common::Rect localTextRect = toPopupLocal(_uinbData->textRect, false);

	const int visibleH = localTextRect.height();
	const int maxScroll = MAX<int>(0, (int)_drawnTextHeight - visibleH);
	const int safeMax = MAX<int>(0, (int)_fullSurface.h - visibleH);
	int scrollY = (int)(_scrollPos * maxScroll);
	if (scrollY > safeMax) {
		scrollY = safeMax;
	}

	// The text is already laid out in _fullSurface; scrolling just re-blits a
	// different vertical slice of it.
	Common::Rect srcSlice(0, scrollY,
							_fullSurface.w, scrollY + visibleH);
	_drawSurface.blitFrom(_fullSurface, srcSlice,
							Common::Point(localTextRect.left, localTextRect.top));

	const UIButtonSlot &activeTab = _uinbData->tabs[_activeTab];
	const bool tasksTab = activeTab.enabled && activeTab.id != notebookJournalTabId();
	if (tasksTab) {
		buildCheckboxRects(localTextRect, scrollY, visibleH);
	}

	_needsRedraw = true;
}

void NotebookPopup::redrawScroll() {
	// The text layout in _fullSurface is unchanged; re-composite the popup at the
	// new scroll offset. drawBackground() wipes the previous (transparent-keyed)
	// text, so the chrome must be repainted, but the expensive text re-layout in
	// layoutText() is skipped.
	drawBackground();
	drawTabs();
	drawCaption();
	paintVisibleText();
	drawForeground();
}

void NotebookPopup::buildCheckboxRects(const Common::Rect &localTextRect, int scrollY, int visibleH) {
	JournalData *journalData = (JournalData *)NancySceneState.getPuzzleData(JournalData::getTag());
	if (!journalData || !journalData->journalEntries.contains(kNotebookTabTasks)) {
		return;
	}
	const Common::Array<JournalData::Entry> &entries = journalData->journalEntries[kNotebookTabTasks];

	const Common::Rect visibleWindow(localTextRect.left, localTextRect.top,
										localTextRect.left + localTextRect.width(),
										localTextRect.top + visibleH);

	const uint count = MIN(_markHotspots.size(), _markEntryIndices.size());
	for (uint k = 0; k < count; ++k) {
		const uint entryIndex = _markEntryIndices[k];
		// Only unchecked boxes (mark 7) are clickable.
		if (entryIndex >= entries.size() || entries[entryIndex].mark != 7) {
			continue;
		}

		// Mark rects are in _fullSurface coords; map to popup-local (offset by
		// the text rect, minus the scroll) and widen the hit area to the right.
		Common::Rect box = _markHotspots[k];
		box.translate(localTextRect.left, localTextRect.top - scrollY);
		box.right += 20;

		const Common::Rect clipped = box.findIntersectingRect(visibleWindow);
		if (clipped.isEmpty()) {
			continue;
		}
		_checkboxRects.push_back(clipped);
		_checkboxEntryIndices.push_back(entryIndex);
	}
}

void NotebookPopup::toggleCheckbox(uint entryIndex) {
	JournalData *journalData = (JournalData *)NancySceneState.getPuzzleData(JournalData::getTag());
	if (!journalData || !journalData->journalEntries.contains(kNotebookTabTasks)) {
		return;
	}
	Common::Array<JournalData::Entry> &entries = journalData->journalEntries[kNotebookTabTasks];
	if (entryIndex >= entries.size() || entries[entryIndex].mark != 7) {
		return;
	}

	// For a clickable task, sceneID doubles as the completion event-flag index
	// (-1 = no requirement). The box can be checked off only once that flag is
	// set; otherwise Nancy says she isn't finished yet.
	const int16 flag = (int16)entries[entryIndex].sceneID;
	const bool canComplete = (flag == -1) || NancySceneState.getEventFlag(flag, g_nancy->_true);
	if (canComplete) {
		entries[entryIndex].mark = 8;
		refreshContent();
		// A check-off plays an immediate click, then Nancy's spoken line a beat
		// later (deferred so the two cues don't step on each other).
		playButtonClickSound(_uinbData->header.secondaryButton);
		_completeVoiceTime = g_system->getMillis() + 400;
	} else {
		playCheckboxSound(false);
	}
}

void NotebookPopup::playButtonClickSound(const UIButtonRecord &button) {
	SoundDescription sound = button.clickSound;
	if (sound.name.empty() || sound.name.equalsIgnoreCase("NO SOUND"))
		return;

	g_nancy->_sound->loadSound(sound);
	g_nancy->_sound->playSound(sound);
}

void NotebookPopup::playCheckboxSound(bool actionable) {
	const Common::Path *set = actionable ? _uinbData->actionableClickSounds
										 : _uinbData->noActionClickSounds;

	// Pick a random variant, falling back to any valid one.
	const uint start = g_nancy->_randomSource->getRandomNumber(UINB::kNumPageSoundsPerSet - 1);
	for (uint n = 0; n < UINB::kNumPageSoundsPerSet; ++n) {
		const Common::String name = set[(start + n) % UINB::kNumPageSoundsPerSet].toString();
		if (name.empty() || name.equalsIgnoreCase("NO SOUND")) {
			continue;
		}
		// The names are raw filenames; play them on the checkbox sound's
		// channel / volume, taken from the close button's click sound (the
		// header sound slots are all "NO SOUND" for the notebook).
		SoundDescription sound = _uinbData->header.secondaryButton.clickSound;
		sound.name = name;
		sound.numLoops = 1;
		g_nancy->_sound->loadSound(sound);
		g_nancy->_sound->playSound(sound);
		return;
	}
}

} // End of namespace UI
} // End of namespace Nancy
