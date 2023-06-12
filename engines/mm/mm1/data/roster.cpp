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

#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/system.h"
#include "mm/mm1/data/roster.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

static byte DEFAULT_PORTRAITS[6] = { 0, 11, 9, 7, 4, 3 };

void Roster::synchronize(Common::Serializer &s, bool isLoadingDefaults) {
	for (int i = 0; i < ROSTER_COUNT; ++i)
		_items[i].synchronize(s, s.isLoading() && isLoadingDefaults ?
			(i < 6 ? DEFAULT_PORTRAITS[i] : 0xff) : -1
		);

	for (int i = 0; i < ROSTER_COUNT; ++i)
		s.syncAsByte(_towns[i]);
}

void Roster::load() {
	Common::InSaveFile *sf = g_system->getSavefileManager()->openForLoading(
		rosterSaveName());

	if (sf) {
		Common::Serializer s(sf, nullptr);
		synchronize(s, false);

		while (!sf->eos()) {
			uint32 chunk = sf->readUint32BE();
			if (!sf->eos() && chunk == MKTAG('M', 'A', 'P', 'S')) {
				sf->skip(4);	// Skip chunk size
				g_maps->synchronize(s);
			}
		}
	} else {
		Common::File f;
		if (!f.open("roster.dta"))
			error("Could not open roster.dta");

		Common::Serializer s(&f, nullptr);
		synchronize(s, true);
	}
}

void Roster::update(const IntArray &charNums) {
	int fallbackIndex = ROSTER_COUNT - 1;

	for (int i = (int)g_globals->_party.size() - 1; i >= 0; --i) {
		const Character &c = g_globals->_party[i];

		int destIndex;
		if (charNums.size() == g_globals->_party.size() &&
				charNums[i] < ROSTER_COUNT &&
				!strcmp(_items[charNums[i]]._name, c._name)) {
			// Started game from title screen and set up party,
			// so we known the correct roster index already
			destIndex = charNums[i];

		} else {
			for (destIndex = 0; destIndex < ROSTER_COUNT; ++destIndex) {
				if (!strcmp(_items[destIndex]._name, c._name))
					break;
			}

			if (destIndex == ROSTER_COUNT) {
				// Couldn't find a matching name in roster to update
				for (destIndex = 0; destIndex < ROSTER_COUNT;  ++destIndex) {
					if (!_towns[destIndex])
						break;
				}

				if (destIndex == ROSTER_COUNT)
					// Replace entries at the end of the roster
					destIndex = fallbackIndex--;
			}
		}

		// Copy the entry into the roster
		_items[destIndex] = c;
		_towns[destIndex] = (Maps::TownId)g_maps->_currentMap->dataByte(Maps::MAP_ID);
	}
}

void Roster::save() {
	Common::OutSaveFile *sf = g_system->getSavefileManager()->openForSaving(
		rosterSaveName());
	Common::Serializer s(nullptr, sf);
	synchronize(s, false);

	// Get automap data to save
	Common::MemoryWriteStreamDynamic mapData(DisposeAfterUse::YES);
	Common::Serializer s2(nullptr, &mapData);
	g_maps->synchronize(s2);

	// Write out the map data
	sf->writeUint32BE(MKTAG('M', 'A', 'P', 'S'));
	sf->writeUint32LE(mapData.size());
	sf->write(mapData.getData(), mapData.size());

	sf->finalize();
	delete sf;
}

Common::String Roster::rosterSaveName() const {
	return Common::String::format("%s-roster.dta",
		g_engine->getTargetName().c_str());
}

void Roster::remove(Character *entry) {
	entry->clear();

	size_t idx = entry - _items;
	_towns[idx] = Maps::NO_TOWN;
}

bool Roster::empty() const {
	for (uint i = 0; i < ROSTER_COUNT; ++i) {
		if (_towns[i])
			return false;
	}

	return true;
}

bool Roster::full() const {
	for (uint i = 0; i < ROSTER_COUNT; ++i) {
		if (!_towns[i])
			return false;
	}

	return true;
}

} // namespace MM1
} // namespace MM
