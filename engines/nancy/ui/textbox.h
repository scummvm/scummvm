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

#include "engines/nancy/renderobject.h"

namespace Nancy {

class NancyEngine;
class Scene;
struct NancyInput;

namespace UI {

class Scrollbar;

class Textbox : public Nancy::RenderObject {
public:
	Textbox();
	virtual ~Textbox();

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void drawTextbox();
	void clear();

	void addTextLine(const Common::String &text);
	void overrideFontID(const uint fontID) { _fontIDOverride = fontID; };

	static void assembleTextLine(char *rawCaption, Common::String &output, uint size);

private:
	uint16 getInnerHeight() const;
	void onScrollbarMove();

	struct Response {
		Common::String text;
		Common::Rect hotspot;
	};

	Graphics::ManagedSurface _fullSurface;
	Graphics::ManagedSurface _textHighlightSurface;

	RenderObject _highlightRObj;

	Scrollbar *_scrollbar;

	Common::Array<Common::String> _textLines;
	Common::Array<Common::Rect> _hotspots;

	uint16 _numLines;
	bool _lastResponseisMultiline;

	bool _needsTextRedraw;
	float _scrollbarPos;

	int _fontIDOverride;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TEXTBOX_H
