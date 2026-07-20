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

#ifndef NANCY_UI_SCROLLTEXTBOX_H
#define NANCY_UI_SCROLLTEXTBOX_H

#include "engines/nancy/commontypes.h"
#include "engines/nancy/misc/hypertext.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;
struct UIPopupHeader;
struct TBOX;

namespace UI {

// Nancy 10+ game text box: a UIPopupHeader overlay popup at the bottom of the
// screen for narration and inventory/timer captions. The header comes from UICO
// (Nancy 10) or SCTB (Nancy 11), which are structurally identical, so one widget
// serves both. Shows caption text only (no response hotspots; conversation
// dialogue uses ConversationPopup). The legacy Textbox forwards to it for
// Nancy 10+.
class ScrollTextBox : public RenderObject, public Misc::HypertextParser {
public:
	ScrollTextBox();
	~ScrollTextBox() override = default;

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	// Append a line of text. The popup is shown on the first non-empty
	// addition. A non-zero autoClearTime schedules an automatic clear() after
	// that many milliseconds (used by inventory closed captions).
	void addTextLine(const Common::String &text, uint32 autoClearTime = 0);
	void setOverrideFont(uint fontID);
	void clear() override;

private:
	void drawBackground();
	void drawContent();
	// Re-composite the expanded overlay at the current scroll position without
	// re-laying out the text (which drawContent() does). Used while dragging the
	// scrollbar, so a drag is a cheap slice re-blit rather than a full re-render.
	void redrawScroll();
	void drawScrollbar(UIButtonState state);
	uint16 getInnerHeight() const;

	// The text content area in popup-local coordinates (the screen-space text
	// viewport brought into the popup surface's space).
	Common::Rect textViewportLocal() const;

	// Popup-local bounding rect of the scrollbar thumb at the current scroll
	// position, or an empty rect when the slider is disabled.
	Common::Rect computeThumbRect() const;

	Common::Rect toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const;
	Common::Point popupLocalMouse(const Common::Point &screenMouse) const;

	// Popup chrome (overlay image, dest rects, slider) supplied by the
	// version-specific chunk: UICO in Nancy 10, SCTB in Nancy 11.
	const UIPopupHeader *_header;
	// Text content area, in screen coordinates.
	Common::Rect _textViewportScreen;
	// The fully-expanded popup rect on screen, with the game-frame offset
	// applied. The mini strip shares its top edge.
	Common::Rect _fullPopupRect;
	const TBOX *_tboxData;

	Graphics::ManagedSurface _overlayImage;

	float _scrollPos;
	bool _scrollbarDragging;
	bool _scrollbarHovered;
	int _scrollbarGrabOffset;

	// True in the expanded state (> 2 lines): full overlay with scrollbar.
	// False in the mini state (<= 2 lines): strip above the taskbar, no scrollbar.
	bool _expanded;

	int _fontIDOverride;
	uint32 _autoClearTime;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_SCROLLTEXTBOX_H
