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

#ifndef NANCY_MISC_MOUSEFOLLOW_H
#define NANCY_MISC_MOUSEFOLLOW_H

#include "engines/nancy/renderobject.h"
#include "engines/nancy/input.h"

namespace Nancy {

struct VIEW;

namespace Misc {

// Describes an object that follows the mouse's movement, making
// sure that its center stays above the mouse hotspot. The position
// of such an object is clipped to the Viewport. Used in puzzles:
// - AssemblyPuzzle
// - CubePuzzle
// - RippedLetterPuzzle
// - TowerPuzzle
// - TangramPuzzle
class MouseFollowObject : public RenderObject {
public:
	MouseFollowObject();
	virtual ~MouseFollowObject() {}

	virtual void pickUp() { _isPickedUp = true; }
	virtual void putDown() { _isPickedUp = false; }

	void setZ(uint16 z) { _z = z; _needsRedraw = true; }
	void handleInput(NancyInput &input);

protected:
	bool isViewportRelative() const override { return true; }

	bool _isPickedUp = false;
	byte _rotation = 0;

	const VIEW *_viewportData = nullptr;
};

} // End of namespace Misc
} // End of namespace Nancy

#endif // NANCY_MISC_MOUSEFOLLOW_H
