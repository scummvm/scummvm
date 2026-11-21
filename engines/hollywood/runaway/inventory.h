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
#include "hollywood/runaway/detection.h"
#include "hollywood/runaway/saveload.h"

namespace Common {
class RandomSource;
}

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
		byte nextVariation = 0xff;
		uint16 action = 0;
		Reply(uint16 c = 1, byte v = 0) : cue(c), variation(v) {}
		static Reply fromAction(uint16 action) {
			Reply reply;
			reply.action = action;
			return reply;
		}
	};
	RunawayInventory(const RunawayStrings &strings, RunawayGameType game) :
		_strings(strings), _museumDemo(game == kRunawayMuseumDemo), _fullGame(game == kRunawayFullGame), _chapter(game == kRunawayFullGame ? 1 : game == kRunawayMuseumDemo ? 2 : 3) {}
	void setChapter(byte chapter) { _chapter = chapter; }
	void startChapter(byte chapter);
	RunawaySaveState::MuseumItems &museum() { return _museum; }
	RunawaySaveState::DesertItems &desert() { return _desert; }
	RunawaySaveState::TownItems &town() { return _town; }
	RunawaySaveState::HopiItems &hopi() { return _hopi; }
	RunawaySaveState::FinaleItems &finale() { return _finale; }
	bool contains(byte item) const;
	uint maxItemId() const;
	bool add(byte item);
	void remove(byte item);
	uint size() const { return _items.size(); }
	byte itemAt(uint slot) const { return slot < size() ? _items[slot] : 0; }
	byte selected() const { return _selected; }
	void select(byte item) { _selected = contains(item) ? item : 0; }
	byte takeNewItem() { const byte item = _newItem; _newItem = 0; return item; }
	Common::U32String name(byte item) const;
	Common::U32String combinationCaption(byte item, byte target) const;
	static Reply randomReply(Common::RandomSource &random);
	uint icon(byte item) const;
	uint cursor(byte item) const;
	Reply examine(byte item) const;
	Reply combine(byte first, byte second);
	byte clothState() const { return _clothState; }
	void soilCloth() { _clothState = 2; }
	byte water() const { return _water; }
	void setWater(byte water) { _water = water; }
	bool chartWritten() const { return _chartWritten; }
	bool headWig() const { return _headWig; }
	void captureState(RunawaySaveState &state) const;
	void restoreState(const RunawaySaveState &state);

private:
	Reply combineHospitalItems(byte first, byte second);
	Reply combineMuseumItems(byte first, byte second);
	Reply combineTownItems(byte first, byte second);
	Reply combineHopiItems(byte first, byte second);
	const RunawayStrings &_strings;
	const bool _museumDemo;
	const bool _fullGame;
	byte _chapter;
	RunawaySaveState::MuseumItems _museum;
	RunawaySaveState::DesertItems _desert;
	RunawaySaveState::TownItems _town;
	RunawaySaveState::HopiItems _hopi;
	RunawaySaveState::FinaleItems _finale;
	Common::Array<byte> _items;
	byte _selected = 0, _newItem = 0, _clothState = 0;
	byte _water = 0;
	byte _markerInk = 0, _syringeFilled = 0, _chartWritten = 0, _headWig = 0;
};

} // End of namespace Hollywood

#endif
