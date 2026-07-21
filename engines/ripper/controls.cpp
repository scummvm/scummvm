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

#include "ripper/controls.h"

#include "common/debug.h"

#include "ripper/detection.h"

namespace Ripper {

uint UiControlRegistry::add(const Common::Rect &bounds, uint16 action) {
	_controls.push_back(UiControl(bounds, action));
	debugC(3, kDebugInput,
		"Ripper: registered UI control index=%u action=0x%04x bounds=%d,%d,%d,%d",
		_controls.size() - 1, action, bounds.left, bounds.top,
		bounds.right, bounds.bottom);
	return _controls.size() - 1;
}

int UiControlRegistry::findFirst(const Common::Point &point) const {
	for (uint i = 0; i < _controls.size(); ++i) {
		if (_controls[i].enabled && _controls[i].bounds.contains(point))
			return i;
	}
	return -1;
}

ChooserModel::ChooserModel() : _itemCount(0), _selectedIndex(0),
		_firstVisibleIndex(0), _visibleCount(0) {
}

void ChooserModel::reset(uint itemCount, uint maximumVisibleCount,
		uint selectedIndex) {
	_itemCount = itemCount;
	_visibleCount = MIN(itemCount, maximumVisibleCount);
	_selectedIndex = itemCount == 0 ? 0 : MIN(selectedIndex, itemCount - 1);
	_firstVisibleIndex = 0;
	ensureSelectionVisible();
}

bool ChooserModel::restore(uint itemCount, uint maximumVisibleCount,
		uint selectedIndex, uint firstVisibleIndex) {
	if (itemCount == 0) {
		if (selectedIndex != 0 || firstVisibleIndex != 0)
			return false;
		clear();
		return true;
	}
	if (maximumVisibleCount == 0 || selectedIndex >= itemCount ||
			firstVisibleIndex >= itemCount)
		return false;
	_itemCount = itemCount;
	_visibleCount = MIN(itemCount, maximumVisibleCount);
	_selectedIndex = selectedIndex;
	_firstVisibleIndex = firstVisibleIndex;
	return true;
}

void ChooserModel::clear() {
	_itemCount = 0;
	_selectedIndex = 0;
	_firstVisibleIndex = 0;
	_visibleCount = 0;
}

bool ChooserModel::moveSelection(int delta) {
	if (_itemCount == 0 || delta == 0)
		return false;
	const uint previous = _selectedIndex;
	if (delta < 0 && _selectedIndex > 0)
		--_selectedIndex;
	else if (delta > 0 && _selectedIndex + 1 < _itemCount)
		++_selectedIndex;
	ensureSelectionVisible();
	return _selectedIndex != previous;
}

bool ChooserModel::select(uint index, bool ensureVisible) {
	if (index >= _itemCount)
		return false;
	const bool changed = index != _selectedIndex;
	_selectedIndex = index;
	if (ensureVisible)
		ensureSelectionVisible();
	return changed;
}

bool ChooserModel::scrollWindow(int delta) {
	if (_itemCount == 0 || _visibleCount == 0 || delta == 0)
		return false;
	const uint maximumFirst = _itemCount > _visibleCount ?
		_itemCount - _visibleCount : 0;
	const uint previous = _firstVisibleIndex;
	if (delta < 0 && _firstVisibleIndex > 0)
		--_firstVisibleIndex;
	else if (delta > 0 && _firstVisibleIndex < maximumFirst)
		++_firstVisibleIndex;
	if (_selectedIndex < _firstVisibleIndex)
		_selectedIndex = _firstVisibleIndex;
	else if (_selectedIndex >= _firstVisibleIndex + _visibleCount)
		_selectedIndex = _firstVisibleIndex + _visibleCount - 1;
	return _firstVisibleIndex != previous;
}

bool ChooserModel::resolveVisibleRow(uint row, uint &index) const {
	if (row >= _visibleCount || _firstVisibleIndex + row >= _itemCount)
		return false;
	index = _firstVisibleIndex + row;
	return true;
}

void ChooserModel::ensureSelectionVisible() {
	if (_itemCount == 0 || _visibleCount == 0) {
		_firstVisibleIndex = 0;
		return;
	}
	if (_selectedIndex < _firstVisibleIndex)
		_firstVisibleIndex = _selectedIndex;
	else if (_selectedIndex >= _firstVisibleIndex + _visibleCount)
		_firstVisibleIndex = _selectedIndex - _visibleCount + 1;
}

} // End of namespace Ripper
