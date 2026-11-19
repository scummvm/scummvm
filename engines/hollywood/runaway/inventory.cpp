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

const char *const kMuseumItemNames[] = {
	"", "Wallet", "Car keys", "Condom", "Piece of amber", "Tape recorder", "Battery",
	"Crucifix", "Strange Mayan object", "Colorless varnish", "Talcum powder", "Mayan mask",
	"Ruby", "Willy's business card", "Bowl with coffee beans", "Cup of coffee",
	"Pair of books", "Strange key", "Ladle", "Package of coffee"
};

const char *const kHospitalItemNames[] = {
	"", "Wallet", "Car keys", "Order form", "Piece of amber", "Gina's bag", "Gina's medical chart",
	"Sheet", "Package of pills", "Matchbox", "Wig", "Marker pen", "Bottle of alcohol", "Glass",
	"Pillows", "Syringe", "Blank medical chart", "Head", "Flammable spray can", "Vademecum"
};

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
	state.water = _water;
	state.markerInk = _markerInk;
	state.syringeFilled = _syringeFilled;
	state.chartWritten = _chartWritten;
	state.headWig = _headWig;
	state.museumItems = _museum;
}

void RunawayInventory::restoreState(const RunawaySaveState &state) {
	assert(state.valid());
	_chapter = state.chapter();
	_items.clear();
	for (uint i = 0; i < state.itemCount; ++i)
		_items.push_back(state.items[i]);
	_selected = state.selected;
	_clothState = state.cloth;
	_water = state.water;
	_markerInk = state.markerInk;
	_syringeFilled = state.syringeFilled;
	_chartWritten = state.chartWritten;
	_headWig = state.headWig;
	_museum = state.museumItems;
	_newItem = 0;
}

void RunawayInventory::startChapter(byte chapter) {
	_chapter = chapter;
	_items.clear();
	_selected = _newItem = 0;
	if (chapter == 2) {
		_museum = RunawaySaveState::MuseumItems();
		const byte initial[] = {1, 2, 3, 4, 7};
		for (uint i = 0; i < ARRAYSIZE(initial); ++i)
			add(initial[i]);
	}
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
	if (!item || item > kItemCount || (_chapter != 3 && item > 19) || contains(item))
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
	if (_chapter == 1) {
		if (item == 11 && _markerInk == 1)
			return Common::U32String("Dry marker pen");
		if (item == 15 && _syringeFilled)
			return Common::U32String("Syringe with alcohol");
		if (item == 16 && _chartWritten)
			return Common::U32String("Fake medical chart");
		if (item == 17 && _headWig)
			return Common::U32String("Head with wig");
	}
	if (_chapter == 1 && item == 13 && _water == 1)
		return Common::U32String("Glass with water");
	if (_chapter == 2) {
		if (item == 6 && _museum.frozenBattery)
			return Common::U32String("Frozen battery");
		if (item == 11 && _museum.rubyRemoved)
			return Common::U32String("Mayan mask without ruby");
		if (item == 14 && _museum.coffee)
			return Common::U32String(_museum.coffee == 1 ? "Bowl with ground coffee" : "Empty bowl");
		if (item == 18 && _museum.batteryInLadle)
			return Common::U32String("Ladle with battery");
	}
	if (_chapter == 2)
		return item < ARRAYSIZE(kMuseumItemNames) ? Common::U32String(kMuseumItemNames[item]) : Common::U32String();
	if (_chapter == 1)
		return item < ARRAYSIZE(kHospitalItemNames) ? Common::U32String(kHospitalItemNames[item]) : Common::U32String();
	if (item == 1 && _clothState)
		return _strings.text(_clothState == 1 ? RunawayStrings::kWetCloth : RunawayStrings::kDirtyCloth);
	return _strings.itemName(item);
}

uint RunawayInventory::icon(byte item) const {
	if (_chapter == 2) {
		if (item == 7 && _museum.restoredCrucifix) return 24;
		if (item == 14 && _museum.coffee) return 20 + _museum.coffee;
		if (item == 6 && _museum.frozenBattery) return 25;
		if (item == 11 && _museum.rubyRemoved) return 20;
		if (item == 18 && _museum.batteryInLadle) return 23;
		if (item == 19 && _museum.packageFilled) return 26;
	}
	if (_chapter == 1) {
		if (item == 15 && _syringeFilled)
			return 20;
		if (item == 16 && _chartWritten)
			return 22;
		if (item == 17 && _headWig)
			return 21;
	}
	if (_chapter == 1 && item == 13 && _water == 1)
		return 23;
	if (_chapter != 3)
		return item;
	if (item == 1 && _clothState)
		return _clothState == 1 ? 34 : 35;
	return item == 15 ? 29 : item;
}

uint RunawayInventory::cursor(byte item) const {
	if (_chapter == 1) {
		if (item == 15 && _syringeFilled)
			return 77;
		if (item == 16 && _chartWritten)
			return 83;
		if (item == 17 && _headWig)
			return 79;
	}
	if (_chapter == 1 && item == 13 && _water == 1)
		return 85;
	if (_chapter == 2)
		return item && item < ARRAYSIZE(kMuseumItemNames) ? (_museumDemo ? 11 + item * 2 : 39 + icon(item) * 2) : 0;
	if (_chapter == 1)
		return item && item <= 19 ? (item == 16 ? 81 : 39 + item * 2 - (item > 16 ? 2 : 0)) : 0;
	if (item == 1 && _clothState)
		return _clothState == 1 ? 107 : 109;
	return item <= kItemCount ? kItemCursors[item] : 0;
}

RunawayInventory::Reply RunawayInventory::examine(byte item) const {
	if (_chapter == 1) {
		if (item == 11)
			return Reply(60, _markerInk >= 2 ? 2 : _markerInk);
		if (item == 15)
			return Reply(64, _syringeFilled);
		if (item == 16)
			return Reply(65, _chartWritten);
		if (item == 17)
			return Reply(66, _headWig);
	}
	if (_chapter == 1 && item == 13)
		return Reply(62, _water == 1 ? 1 : 0);
	if (_chapter == 2) {
		if (!item || item >= ARRAYSIZE(kMuseumItemNames))
			return Reply();
		byte variation = 0;
		switch (item) {
		case 5: variation = _museum.voiceRecorded; break;
		case 6: variation = _museum.frozenBattery; break;
		case 7: variation = _museum.restoredCrucifix; break;
		case 8: case 11: variation = _museum.rubyRemoved; break;
		case 14: variation = _museum.coffee; break;
		case 18: variation = _museum.batteryInLadle; break;
		case 19: variation = _museum.packageFilled; break;
		default: break;
		}
		return Reply(item < 5 ? 49 + item : 95 + item, variation);
	}
	if (_chapter == 1)
		return Reply(item && item <= 19 ? (item == 3 ? 74 : 49 + item) : 1);
	return Reply(item && item <= kItemCount ? kExamineCues[item] : 1, item == 1 ? _clothState : 0);
}

RunawayInventory::Reply RunawayInventory::combine(byte first, byte second) {
	if (!contains(first) || !contains(second) || first == second)
		return Reply();
	if (first > second)
		SWAP(first, second);
	if (_chapter == 1) {
		if (first == 10 && second == 17) {
			_headWig = 1;
			remove(10);
			return Reply(71);
		}
		if (first == 11 && second == 16) {
			if (_markerInk < 2) {
				const byte previous = _markerInk;
				_markerInk = 1;
				return Reply(72, previous);
			}
			if (_chartWritten)
				return Reply(72, 3);
			_chartWritten = 1;
			_markerInk = 3;
			return Reply(72, 2);
		}
		if (first == 12 && second == 15) {
			const byte previous = _syringeFilled;
			_syringeFilled = 1;
			return Reply(69, previous);
		}
		if (first == 11 && second == 15) {
			if (!_syringeFilled)
				return Reply(73);
			if (_markerInk >= 2)
				return Reply(73, 2);
			_markerInk = 2;
			_syringeFilled = 0;
			return Reply(73, 1);
		}
		if (first == 11 && second == 12)
			return Reply(70);
		if (first == 12 && second == 13)
			return Reply(70, 2);
		if (first == 13 && second == 15)
			return Reply(70, 1);
		if (first == 6 && second == 11)
			return Reply(72, 4);
		return Reply();
	}
	if (_chapter == 2) {
		if (first == 5 && second == 6) {
			if (!_museum.frozenBattery)
				return Reply(115);
			_museum.recorderBattery = 3;
			remove(6);
			return Reply(115, 1);
		}
		if (first == 6 && second == 18) {
			if (_museum.frozenBattery)
				return Reply(116, 2);
			_museum.batteryInLadle = 1;
			remove(6);
			return Reply(116);
		}
		if (first == 8 && second == 11) {
			if (_museum.rubyRemoved)
				return Reply(117, 2);
			_museum.rubyRemoved = 1;
			add(12);
			return Reply(117);
		}
		if (first == 11 && second == 12)
			return Reply(120);
		if (first == 14 && second == 19) {
			if (_museum.coffee != 1)
				return Reply(_museum.coffee ? 1 : 121);
			_museum.coffee = 2;
			_museum.packageFilled = 1;
			return Reply(121, 1);
		}
		if (first == 9 || second == 9) return Reply(118);
		if (first == 10 || second == 10) return Reply(119);
		if (first == 15 || second == 15) return Reply(122);
		return Reply();
	}
	if (_chapter != 3)
		return Reply();
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
