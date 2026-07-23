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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef RIPPER_CURSOR_H
#define RIPPER_CURSOR_H

#include "ripper/resources.h"

namespace Ripper {

class ResourceManager;

class CursorManager {
public:
	CursorManager();
	~CursorManager();

	bool initialize(ResourceManager &resources);
	void setSelectionIndex(int cursorIndex);
	void dispatchSelectionIndexChange(int cursorIndex);
	void applyCustomCursor(const BitmapAssetFrame &frame,
		int hotspotX, int hotspotY, uint scale = 1);
	void update(uint cursorIndex);
	void refresh();
	void setVisible(bool visible);
	bool isVisible() const { return _visible; }
	uint getSelectionIndex() const { return _selectionIndex; }

private:
	void applyFrame();
	void getHotspot(const BitmapAssetFrame &frame, int &x, int &y) const;

	BitmapAssetSequence _assets[24];
	uint _selectionIndex;
	uint _cursorIndex;
	uint _frameIndex;
	uint32 _nextFrameMillis;
	bool _initialized;
	bool _visible;
};

} // End of namespace Ripper

#endif // RIPPER_CURSOR_H
