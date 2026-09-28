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

#ifndef NANCY_MISC_VIEWPORTFADER_H
#define NANCY_MISC_VIEWPORTFADER_H

#include "engines/nancy/renderobject.h"

namespace Nancy {
namespace Misc {

// Nancy16 covers scene changes by fading the viewport to a solid color, holding it,
// and fading back once the next scene is in. An action record arms the fade, and the
// next scene change starts it. The new scene only takes over after the hold.
class ViewportFader : public RenderObject {
public:
	ViewportFader() : RenderObject(16) {}
	virtual ~ViewportFader() {}

	void init() override;
	void updateGraphics() override;

	bool isIdle() const { return _state == kIdle; }

	// The color is RGB; the fade always ends fully opaque
	void arm(uint32 color, uint32 holdTime);

	// Starts an armed fade, when a scene change gets requested
	void onSceneChange();

	// The requested scene waits until the viewport is fully covered
	bool holdsSceneChange() const { return _state == kFadingIn || _state == kHolding; }

private:
	enum State {
		kIdle,
		kArmed,
		kFadingIn,
		kHolding,
		kFadingOut
	};

	bool blendTowards(uint32 target, float seconds);
	void redraw();

	State _state = kIdle;
	float _current[4] = {};		// ARGB channels, starting at transparent black
	uint32 _target = 0;			// ARGB
	uint32 _holdTime = 0;
	uint32 _holdEndTime = 0;
	uint32 _lastUpdateTime = 0;
};

} // End of namespace Misc
} // End of namespace Nancy

#endif // NANCY_MISC_VIEWPORTFADER_H
