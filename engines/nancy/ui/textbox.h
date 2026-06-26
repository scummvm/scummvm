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

#ifndef NANCY_UI_TEXTBOX_H
#define NANCY_UI_TEXTBOX_H

#include "engines/nancy/misc/hypertext.h"
#include "engines/nancy/renderobject.h"

namespace Nancy {

class NancyEngine;
class Scene;
struct NancyInput;

namespace UI {

class Scrollbar;
class ScrollTextBox;

class Textbox : public RenderObject, public Misc::HypertextParser {
public:
	Textbox();
	virtual ~Textbox();

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void drawTextbox();
	void clear() override;

	void addTextLine(const Common::String &text, uint32 autoClearTime = 0);
	void setOverrideFont(const uint fontID);

	// Vestigial "full mode" toggle, now a no-op. Kept for the FrameTextBox AR.
	void setFullMode(bool open, uint32 timeoutMs = 15000);
	bool isFullMode() const { return false; }

private:
	uint16 getInnerHeight() const;
	void onScrollbarMove();

	RenderObject _highlightRObj;
	Scrollbar *_scrollbar;

	// Nancy 10+ overlay popup the public API forwards to (UICO/SCTB-driven).
	// The legacy members above are unused in that mode.
	ScrollTextBox *_scrollTextBox;

	float _scrollbarPos;

	uint32 _autoClearTime;
	int _fontIDOverride;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TEXTBOX_H
