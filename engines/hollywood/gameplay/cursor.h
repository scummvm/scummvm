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

#ifndef HOLLYWOOD_GAMEPLAY_CURSOR_H
#define HOLLYWOOD_GAMEPLAY_CURSOR_H

#include "common/array.h"
#include "common/types.h"

namespace Common {
struct Point;
}

namespace Graphics {
struct WinCursorGroup;
}

namespace Hollywood {

class HollywoodCursor {
public:
	HollywoodCursor();
	~HollywoodCursor();

	bool load();
	bool isLoaded() const { return _loaded; }

	void enterInteractiveMode();
	void leaveInteractiveMode();
	void updatePosition(const Common::Point &point);
	void advance(uint32 deltaMillis);

	uint16 surfaceX() const { return _surfaceX; }
	uint16 surfaceY() const { return _surfaceY; }

private:
	void clear();
	bool loadFromExecutableResources();
	bool loadFallbackCursorGroups();
	void installCurrentFrame();

	Common::Array<Graphics::WinCursorGroup *> _cursorGroups;
	uint32 _frameTimer;
	byte _animationFrameIndex;
	uint16 _surfaceX;
	uint16 _surfaceY;
	bool _loaded;
	bool _visible;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_GAMEPLAY_CURSOR_H
