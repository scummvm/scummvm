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

#ifndef NANCY_UI_SCROLLBAR_H
#define NANCY_UI_SCROLLBAR_H

#include "engines/nancy/renderobject.h"

namespace Nancy {

struct NancyInput;

namespace UI {

// Returns the number of pixels a single mouse wheel notch should scroll a view of
// the given height, so that scrolling advances by a fixed part of a page.
int wheelScrollPixels(int visibleHeight);

// Same, expressed as a fraction of the total scroll range. Returns 0 when the
// content fits inside the viewport and thus cannot be scrolled.
float wheelScrollStep(int visibleHeight, int contentHeight);

// Applies mouse wheel input to a [0, 1] scroll position, as long as the mouse is
// inside the hotspot. Consumes the wheel input and returns true if the position changed.
bool scrollWithMouseWheel(NancyInput &input, const Common::Rect &hotspot, float &scrollPos, float step);

class Scrollbar : public RenderObject {
public:
	Scrollbar(uint16 zOrder, const Common::Rect &srcBounds, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical = true);
	Scrollbar(uint16 zOrder, const Common::Rect &srcBounds, Graphics::ManagedSurface &srcSurf, const Common::Point &topPosition, uint16 scrollDistance, bool isVertical = true);
	virtual ~Scrollbar() = default;

	void init() override;

	void handleInput(NancyInput &input);

	void resetPosition();
	float getPos() const { return _currentPosition; }

	// The screen rect the thumb travels along. Unlike _screenPosition, which only
	// covers the thumb itself, this covers the whole scrollbar.
	Common::Rect getTrackRect() const;

	void setPosition(float pos);
	void calculatePosition();

	Common::Point _startPosition;
	uint _maxDist;
	bool _isVertical;

	float _currentPosition;
	bool _isClicked;
	Common::Point _mousePosOnClick;
};

} // End of namespace UI
} // End of namespace Nancy

#endif // NANCY_UI_SCROLLBAR_H
