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

#ifndef RIPPER_CONTROLS_H
#define RIPPER_CONTROLS_H

#include "common/array.h"
#include "common/rect.h"

namespace Ripper {

struct UiControl {
	Common::Rect bounds;
	uint16 action;
	bool enabled;

	UiControl() : action(0), enabled(true) {}
	UiControl(const Common::Rect &controlBounds, uint16 controlAction) :
		bounds(controlBounds), action(controlAction), enabled(true) {}
};

class UiControlRegistry {
public:
	uint add(const Common::Rect &bounds, uint16 action);
	void clear() { _controls.clear(); }
	int findFirst(const Common::Point &point) const;

	uint size() const { return _controls.size(); }
	bool empty() const { return _controls.empty(); }
	UiControl &operator[](uint index) { return _controls[index]; }
	const UiControl &operator[](uint index) const { return _controls[index]; }

private:
	Common::Array<UiControl> _controls;
};

class ChooserModel {
public:
	ChooserModel();

	void reset(uint itemCount, uint maximumVisibleCount, uint selectedIndex = 0);
	bool restore(uint itemCount, uint maximumVisibleCount,
		uint selectedIndex, uint firstVisibleIndex);
	void clear();
	bool moveSelection(int delta);
	bool select(uint index, bool ensureVisible = true);
	bool scrollWindow(int delta);
	bool resolveVisibleRow(uint row, uint &index) const;

	uint itemCount() const { return _itemCount; }
	uint selectedIndex() const { return _selectedIndex; }
	uint firstVisibleIndex() const { return _firstVisibleIndex; }
	uint visibleCount() const { return _visibleCount; }

private:
	void ensureSelectionVisible();

	uint _itemCount;
	uint _selectedIndex;
	uint _firstVisibleIndex;
	uint _visibleCount;
};

} // End of namespace Ripper

#endif // RIPPER_CONTROLS_H
