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

#ifndef HOLLYWOOD_RUNAWAY_INVENTORY_H
#define HOLLYWOOD_RUNAWAY_INVENTORY_H

#include "common/array.h"
#include "common/ustr.h"

namespace Hollywood {

class RunawayStrings;
struct RunawaySaveState;

enum RunawayVerb { kVerbWalk, kVerbLook, kVerbUse };

class RunawayInventory {
public:
	enum { kItemCount = 26 };
	struct Reply {
		uint16 cue;
		byte variation;
		Reply(uint16 c = 1, byte v = 0) : cue(c), variation(v) {}
	};
	explicit RunawayInventory(const RunawayStrings &strings) : _strings(strings) {}
	bool contains(byte item) const;
	bool add(byte item);
	void remove(byte item);
	uint size() const { return _items.size(); }
	byte itemAt(uint slot) const { return slot < size() ? _items[slot] : 0; }
	byte selected() const { return _selected; }
	void select(byte item) { _selected = contains(item) ? item : 0; }
	byte takeNewItem() { const byte item = _newItem; _newItem = 0; return item; }
	Common::U32String name(byte item) const;
	uint icon(byte item) const;
	uint cursor(byte item) const;
	Reply examine(byte item) const;
	Reply combine(byte first, byte second);
	byte clothState() const { return _clothState; }
	void soilCloth() { _clothState = 2; }
	void captureState(RunawaySaveState &state) const;
	void restoreState(const RunawaySaveState &state);

private:
	const RunawayStrings &_strings;
	Common::Array<byte> _items;
	byte _selected = 0, _newItem = 0, _clothState = 0;
};

} // End of namespace Hollywood

#endif
