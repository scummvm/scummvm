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

#ifndef NANCY_UI_CONVERSATIONPOPUP_H
#define NANCY_UI_CONVERSATIONPOPUP_H

#include "engines/nancy/misc/hypertext.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;
struct TBOX;
struct UICO;

namespace UI {

// Nancy 10+ conversation popup. Replaces the legacy Textbox for dialogue
// display. The conversation action calls open(), addTextLine(), and close()
// to drive the popup lifecycle. Response hotspots and hover highlights
// mirror the Textbox behavior but render inside a UICO-driven overlay window.
class ConversationPopup : public RenderObject, public Misc::HypertextParser {
public:
	ConversationPopup();
	~ConversationPopup() override = default;

	void init() override;
	void registerGraphics() override;
	void handleInput(NancyInput &input);

	// Called by ConversationSound::execute() at each new dialogue state.
	// Always clears previous content, matching Textbox::clear() for Nancy 1-9.
	void open();

	// Called once per text line after open() — NPC speech first, then
	// each response option. Triggers a redraw after each addition.
	void addTextLine(const Common::String &text);

	// Called at kActionTrigger just before finishExecution(). Hides the popup.
	void close();

	// Called after NPC text lines have been added, before response lines.
	// Records the hotspot offset so response clicks map to the correct index.
	void setResponseStart() { _responseStartIdx = (uint)_hotspots.size(); }

private:
	void drawBackground();
	void drawContent();
	void drawScrollbar(uint state);
	uint16 getInnerHeight() const;

	// Returns the popup-local bounding rect of the scrollbar thumb at
	// the current scroll position.
	Common::Rect computeThumbRect() const;

	// Convert a chunk-space rect into popup-local coordinates.
	Common::Rect toPopupLocal(const Common::Rect &chunkRect, bool useGameFrame) const;
	Common::Point popupLocalMouse(const Common::Point &screenMouse) const;

	const UICO *_uicoData;
	const TBOX *_tboxData;

	Graphics::ManagedSurface _overlayImage;

	// Separate render object for drawing hovered response text,
	// mirroring the Textbox::_highlightRObj approach.
	RenderObject _highlightRObj;

	// Scrollbar state, driven by header.slider.
	float _scrollPos = 0.0f;
	bool _scrollbarDragging = false;
	bool _scrollbarHovered = false;
	int _scrollbarGrabOffset = 0;

	uint _responseStartIdx = 0;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_CONVERSATIONPOPUP_H
