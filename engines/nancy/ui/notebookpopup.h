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

#ifndef NANCY_UI_NOTEBOOKPOPUP_H
#define NANCY_UI_NOTEBOOKPOPUP_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/renderobject.h"
#include "engines/nancy/misc/hypertext.h"

namespace Nancy {

struct NancyInput;
struct UINB;

namespace UI {

// Nancy 10+ notebook popup. Driven by the UINB chunk: overlay image + two
// tab buttons.
class NotebookPopup : public RenderObject, public Misc::HypertextParser {
public:
	NotebookPopup();
	~NotebookPopup() override = default;

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void open();
	void close();
	void toggle() { if (_isVisible) close(); else open(); }

	// Nancy 11+ lazily populates the notebook via a hidden "prep scene" run when
	// it opens. Returns that scene ID (UINB header.linkbackScene), or kNoScene
	// (9999) for games without one (e.g. Nancy 10), which populate inline.
	int16 getPrepSceneID() const;

	// Re-render the active tab's text content into the text rect.
	// Called automatically on open() and on tab switch; Scene also
	// invokes it after a ModifyListEntry AR runs while the popup is open.
	void refreshContent();

private:
	static const uint kNumTabs = 2;

	enum WidgetState {
		kStatePressed = 0,
		kStateHover = 1,
		kStateIdle = 2,
		kStateDisabled = 3
	};

	void drawBackground();
	void drawTabs();
	void drawTab(uint index, bool drawHover = false);
	// Blit the active tab's title image ("CASE JOURNAL" / "TASKS") into the
	// header strip above the text area.
	void drawCaption();
	void drawContent();
	// Paint foreground widgets (close button, scrollbar) on top of the
	// already-drawn background + content layers.
	void drawForeground();
	void drawCloseButton(bool hovered);
	void drawScrollbar(UIButtonState state);

	// Play a popup button's click sound (close X and the tab buttons), like the
	// inventory popup. Falls back to the shared button-click slot in the header.
	void playButtonClickSound(const UIButtonRecord &button);

	// Returns the on-popup-surface bounding rect of the slider thumb at
	// the current scroll position (in popup-local coords).
	Common::Rect computeThumbRect() const;

	// Convert a chunk-space destRect into popup-local coordinates.
	Common::Rect toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const;
	Common::Point popupLocalMouse(const Common::Point &screenMouse) const;

	// Populate HypertextParser's text-line list with the active tab's
	// entries.
	void buildTextLines();

	// Tasklist checkboxes. Rebuild the popup-local hit rects for the clickable
	// (unchecked) checkboxes from the mark hotspots recorded during drawContent.
	void buildCheckboxRects(const Common::Rect &localTextRect, int scrollY, int visibleH);
	// Check off a task if its completion event flag is set, else play the
	// "not finished yet" line.
	void toggleCheckbox(uint entryIndex);
	// Play a random checkbox voice line: actionable = "finished with that",
	// !actionable = "can't check that off yet".
	void playCheckboxSound(bool actionable);

	const UINB *_uinbData;

	Graphics::ManagedSurface _overlayImage;     // popup background image
	Graphics::ManagedSurface _closeButtonImage; // header.secondaryButton.primaryImageName

	bool _closeButtonHovered = false;
	bool _tabHovered = false;
	int _activeTab; // 0..1, matching UINB::tabs index

	// Scrollbar state. Driven by header.slider.
	float _scrollPos = 0.0f;        // [0, 1]: 0 = top, 1 = bottom
	bool _scrollbarDragging = false;
	bool _scrollbarHovered = false;
	int _scrollbarGrabOffset = 0;

	// Deferred "I'm finished with that" voice: checking a task off plays an
	// immediate click, then this fires the spoken line a beat later (0 = none).
	uint32 _completeVoiceTime = 0;

	// Tasklist checkboxes: popup-local hit rects for the clickable (unchecked)
	// boxes and the task-entry index each maps to. Rebuilt every drawContent().
	Common::Array<Common::Rect> _checkboxRects;
	Common::Array<uint> _checkboxEntryIndices;
	// Task-entry index for each mark buildTextLines emits, in draw order, so
	// the recorded mark hotspots can be mapped back to their entries.
	Common::Array<uint> _markEntryIndices;

	// journalEntries HashMap keys: _surfaceID = 3 holds task entries,
	// _surfaceID = 4 holds journal entries.
	enum NotebookTab {
		kNotebookTabTasks   = 3,
		kNotebookTabJournal = 4
	};
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NOTEBOOKPOPUP_H
