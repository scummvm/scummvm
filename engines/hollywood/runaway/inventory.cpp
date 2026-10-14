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

#include "common/util.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

const byte kItemCursors[] = {
	0, 41, 43, 45, 47, 49, 51, 55, 57, 59, 61, 63, 65, 67, 69,
	75, 79, 81, 83, 85, 87, 91, 93, 95, 103, 105, 111
};
const uint16 kExamineCues[] = {
	0, 150, 152, 153, 157, 158, 151, 159, 154, 160, 161, 162, 163,
	164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 155, 175, 176
};

} // End of anonymous namespace

void RunawayInventory::captureState(RunawaySaveState &state) const {
	state.itemCount = _items.size();
	for (uint i = 0; i < sizeof(state.items); ++i)
		state.items[i] = itemAt(i);
	state.selected = _selected;
	state.cloth = _clothState;
}

void RunawayInventory::restoreState(const RunawaySaveState &state) {
	assert(state.valid());
	_items.clear();
	for (uint i = 0; i < state.itemCount; ++i)
		_items.push_back(state.items[i]);
	_selected = state.selected;
	_clothState = state.cloth;
	_newItem = 0;
}

bool RunawayInventory::contains(byte item) const {
	for (uint i = 0; i < size(); ++i) {
		if (_items[i] == item)
			return true;
	}
	return false;
}

bool RunawayInventory::add(byte item) {
	if (!item || item > kItemCount || contains(item))
		return false;
	_items.push_back(item);
	_newItem = item;
	return true;
}

void RunawayInventory::remove(byte item) {
	for (uint i = 0; i < size(); ++i) {
		if (_items[i] == item) {
			_items.remove_at(i);
			break;
		}
	}
	if (_selected == item)
		_selected = 0;
	if (_newItem == item)
		_newItem = 0;
}

Common::U32String RunawayInventory::name(byte item) const {
	if (item == 1 && _clothState)
		return _strings.text(_clothState == 1 ? RunawayStrings::kWetCloth : RunawayStrings::kDirtyCloth);
	return _strings.itemName(item);
}

uint RunawayInventory::icon(byte item) const {
	if (item == 1 && _clothState)
		return _clothState == 1 ? 34 : 35;
	return item == 15 ? 29 : item;
}

uint RunawayInventory::cursor(byte item) const {
	if (item == 1 && _clothState)
		return _clothState == 1 ? 107 : 109;
	return item <= kItemCount ? kItemCursors[item] : 0;
}

RunawayInventory::Reply RunawayInventory::examine(byte item) const {
	return Reply(item && item <= kItemCount ? kExamineCues[item] : 1, item == 1 ? _clothState : 0);
}

RunawayInventory::Reply RunawayInventory::combine(byte first, byte second) {
	if (!contains(first) || !contains(second) || first == second)
		return Reply();
	if (first > second)
		SWAP(first, second);
	// The combinations available among the shack's items, from 0x4158c0
	// and 0x415bd0. A failed combination leaves both items in place.
	if (first == 1 && second == 3) {
		const byte previous = _clothState;
		if (!previous)
			_clothState = 1;
		return Reply(156, previous);
	}
	if (first == 7 && second == 8) {
		remove(7);
		remove(8);
		add(17);
		return Reply(183, 1);
	}
	return Reply();
}

} // End of namespace Hollywood
