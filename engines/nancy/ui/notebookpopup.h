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
	void handleInput(NancyInput &input);

	void open();
	void close();
	void toggle() { if (_isVisible) close(); else open(); }

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
	void drawContent();
	// Paint foreground widgets (close button, scrollbar) on top of the
	// already-drawn background + content layers.
	void drawForeground();
	void drawCloseButton(WidgetState state);
	void drawScrollbar(WidgetState state);

	// Returns the on-popup-surface bounding rect of the slider thumb at
	// the current scroll position (in popup-local coords).
	Common::Rect computeThumbRect() const;

	// Convert a chunk-space destRect into popup-local coordinates.
	Common::Rect toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const;
	Common::Point popupLocalMouse(const Common::Point &screenMouse) const;

	// Populate HypertextParser's text-line list with the active tab's
	// entries.
	void buildTextLines();

	void paintPaperIntoFullSurface();

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

	enum NotebookTab {
		kNotebookTabJournal = 3,
		kNotebookTabTasks = 4
	};
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_NOTEBOOKPOPUP_H
