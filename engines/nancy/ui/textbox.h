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
	Textbox(RenderObject &redrawFrom);
	virtual ~Textbox();

	void init() override;
	void registerGraphics() override;
	void updateGraphics() override;
	void handleInput(NancyInput &input);

	void drawTextbox();
	void clear();

	void addTextLine(const Common::String &text);
	void onScrollbarPositionChanged(float data);

	static void assembleTextLine(char *rawCaption, Common::String &output, uint size);

private:
	uint16 getInnerHeight() const;
	void onScrollbarMove();

	struct Response {
		Common::String text;
		Common::Rect hotspot;
	};

	Graphics::ManagedSurface _fullSurface;

	Scrollbar *_scrollbar;

	Common::Array<Common::String> _textLines;
	Common::Array<Common::Rect> _hotspots;

	uint16 _firstLineOffset;
	uint16 _lineHeight;
	uint16 _borderWidth;
	uint16 _maxWidthDifference;
	uint16 _numLines;
	uint16 _fontID;

	bool _needsTextRedraw;
	float _scrollbarPos;

	static const char _CCBeginToken[];
	static const char _CCEndToken[];
	static const char _colorBeginToken[];
	static const char _colorEndToken[];
	static const char _hotspotToken[];
	static const char _newLineToken[];
	static const char _tabToken[];
	static const char _telephoneEndToken[];
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_TEXTBOX_H
