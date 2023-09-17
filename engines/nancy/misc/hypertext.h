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

#ifndef NANCY_MISC_HYPERTEXT_H
#define NANCY_MISC_HYPERTEXT_H

#include "engines/nancy/renderobject.h"

namespace Nancy {
namespace Misc {

// Base class for handling the engine's custom hypertext format
// Used by the Textbox and by Autotext action records
class HypertextParser {
public:
	HypertextParser() :
		_numDrawnLines(0),
		_drawnTextHeight(0),
		_needsTextRedraw(false) {}
	virtual ~HypertextParser() {};

protected:
	void initSurfaces(uint width, uint height, const struct Graphics::PixelFormat &format, uint32 backgroundColor, uint32 highlightBackgroundColor);

	void addTextLine(const Common::String &text);

	void drawAllText(const Common::Rect &textBounds, uint fontID, uint highlightFontID);
	virtual void clear();

	Graphics::ManagedSurface _fullSurface; 			// Contains all rendered text (may be cropped)
	Graphics::ManagedSurface _textHighlightSurface; // Same as above, but drawn with the highlight font

	uint32 _backgroundColor;
	uint32 _highlightBackgroundColor;

	Common::Array<Common::String> _textLines;
	Common::Array<Common::Rect> _hotspots;

	uint16 _numDrawnLines;
	uint16 _drawnTextHeight;
	bool _needsTextRedraw;
};

} // End of namespace Misc
} // End of namespace Nancy

#endif // NANCY_MISC_HYPERTEXT_H
