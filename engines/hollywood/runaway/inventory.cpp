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

#include "common/random.h"
#include "common/util.h"
#include "hollywood/runaway/inventory.h"
#include "hollywood/runaway/saveload.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// Selected item followed by target item; the original combinations are directional.
const byte kHospitalItemActions[19][19] = {
	{ 0,  5,  5,  5, 20,  3,  5,  5,  5,  5,  5, 20, 17,  5,  3,  3,  3, 20,  5}, // 1: Wallet.
	{ 5,  0, 15, 20, 20,  5,  5,  3,  5, 20,  3, 20, 17,  5,  3,  5, 20, 15,  5}, // 2: Car keys.
	{ 5, 15,  0,  5, 20, 17,  5,  5, 15,  5, 15,  5, 17,  5,  5,  3,  5,  5,  3}, // 3: Order form.
	{ 5,  5,  5,  0, 20,  3,  5,  3, 20, 17,  5, 20, 17, 15,  5,  3,  5, 20,  3}, // 4: Amber.
	{ 5,  5,  5,  5,  0,  5,  5,  5,  5,  5,  5,  5, 17,  5,  5,  5,  5,  5,  5}, // 5: Gina's bag.
	{ 3,  3,  3,  3,  5,  0,  5,  3,  5,  3,  3,  5,  5,  5,  5,  3,  3,  5,  3}, // 6: Gina's chart.
	{ 5,  5,  5,  5,  5,  5,  0,  5,  5,  5,  5,  5, 17,  5,  5,  5,  5, 20,  5}, // 7: Sheet.
	{ 5,  5,  5,  5,  5,  3,  5,  0, 20,  3,  5,  5, 17,  5,  3,  3,  3,  5, 17}, // 8: Pills.
	{29, 29, 29, 29, 29, 29, 29, 29,  0, 29, 29, 15, 29, 29, 29, 29, 29, 29, 29}, // 9: Matches.
	{ 5,  5,  5,  5,  5,  5,  5,  5,  5,  0,  5,  5,  5,  5,  5,  5, 73,  5,  5}, // 10: Wig.
	{ 5,  5, 15,  5,  5, 75,  5,  5,  5,  5,  0, 70,  5,  5, 76, 74,  5,  5,  5}, // 11: Marker.
	{ 5,  5,  5,  5,  5,  5,  5,  5,  5,  5, 70,  0, 72,  5, 71,  5,  5,  5,  5}, // 12: Alcohol.
	{ 5,  3, 17,  5,  5,  3,  5,  3,  5,  5,  5,  5,  0,  5,  5,  3,  5,  5,  3}, // 13: Glass.
	{ 3,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  0,  5,  5,  5,  5,  5}, // 14: Pillows.
	{ 5,  5, 15,  5,  5,  5,  5,  5,  5,  5, 76, 69, 17,  5,  0,  5,  5, 15,  5}, // 15: Syringe.
	{ 5,  5,  3,  3,  5, 17,  5,  5,  5,  5, 74,  5, 17,  5,  5,  0,  5,  5,  3}, // 16: Blank chart.
	{ 3,  5,  5,  3,  5,  3,  5,  3,  5, 73,  5,  5, 17,  5,  5,  3,  0,  5,  3}, // 17: Head.
	{15,  5,  5, 15,  5,  5, 15,  5, 15,  5,  5, 15, 17,  5,  5,  5,  5,  0,  5}, // 18: Spray.
	{ 5,  3,  3,  3,  5,  5,  5,  3,  5,  3,  5,  5, 17,  5,  5,  5,  5,  5,  0}, // 19: Medical reference book.
};

const uint kMuseumItemCount = 19;
const uint kTownItemCount = 32;
const uint kHopiItemCount = 16;
const uint kFinaleItemCount = 21;

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
	state.desertItems = _desert;
	state.townItems = _town;
	state.hopiItems = _hopi;
	state.finaleItems = _finale;
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
	_desert = state.desertItems;
	_town = state.townItems;
	_hopi = state.hopiItems;
	_finale = state.finaleItems;
	_newItem = 0;
}

void RunawayInventory::startChapter(byte chapter) {
	_chapter = chapter;
	_items.clear();
	_selected = _newItem = 0;
	if (chapter == 1) {
		_water = _markerInk = _syringeFilled = _chartWritten = _headWig = 0;
	}
	if (chapter == 3) {
		_desert = RunawaySaveState::DesertItems();
		_clothState = 0;
	}
	if (chapter == 2) {
		_museum = RunawaySaveState::MuseumItems();
		const byte initial[] = {1, 2, 3, 4, 7};
		for (uint i = 0; i < ARRAYSIZE(initial); ++i)
			add(initial[i]);
	}
	if (chapter == 4) {
		_town = RunawaySaveState::TownItems();
		for (uint i = 1; i <= 5; ++i) {
			add(i);
		}
	}
	if (chapter == 5) {
		_hopi = RunawaySaveState::HopiItems();
		for (uint i = 1; i <= 7; ++i) {
			add(i);
		}
	}
	if (chapter == 6) {
		_finale = RunawaySaveState::FinaleItems();
		const byte initial[] = {1, 2, 3, 5, 6, 7, 8, 9};
		for (uint i = 0; i < ARRAYSIZE(initial); ++i) {
			add(initial[i]);
		}
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

uint RunawayInventory::maxItemId() const {
	return _chapter == 6 ? kFinaleItemCount : _chapter == 5 ? kHopiItemCount :
		_chapter == 4 ? kTownItemCount : _chapter == 3 ? kItemCount : 19;
}

bool RunawayInventory::add(byte item) {
	if (!item || item > maxItemId() || contains(item)) {
		return false;
	}
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
	if (_chapter == 6) {
		if (item == 19 && _finale.scalpel == 1) {
			return _strings.text(RunawayStrings::kHotScalpel);
		}
		if (item == 20 && _finale.podsOpened) {
			return _strings.text(RunawayStrings::kOpenPods);
		}
		return _strings.itemName(_chapter, item);
	}
	if (_chapter == 5) {
		if (item == 15 && _hopi.toolHandle) {
			return _strings.text(RunawayStrings::kMakeshiftMiningTool);
		}
		return _strings.itemName(_chapter, item);
	}
	if (_chapter == 4) {
		if (item == 11 && _town.strap == 2) {
			return _strings.text(RunawayStrings::kMendedStrap);
		}
		if (item == 24 && _town.bottleContents) {
			return _strings.text(_town.bottleContents == 1 ? RunawayStrings::kBottleWithWater : RunawayStrings::kBottleWithGasoline);
		}
		if ((item == 25 && _town.tube30) || (item == 26 && _town.tube50)) {
			return Common::U32String::format(_strings.text(RunawayStrings::kFilledMeasuringTube),
				item == 25 ? 30U : 50U, 10U * (item == 25 ? _town.tube30 : _town.tube50));
		}
		if (item == 29 && _town.sculpture) {
			return _strings.text(_town.sculpture == 1 ? RunawayStrings::kPolishedStone : RunawayStrings::kStoneOfLight);
		}
		if (item == 31 && _town.mortar) {
			return _strings.text(RunawayStrings::kMakeshiftMortar);
		}
		return _strings.itemName(_chapter, item);
	}
	if (_chapter == 1) {
		if (item == 11 && _markerInk == 1) {
			return _strings.text(RunawayStrings::kDryMarkerPen);
		}
		if (item == 15 && _syringeFilled) {
			return _strings.text(RunawayStrings::kSyringeWithAlcohol);
		}
		if (item == 16 && _chartWritten) {
			return _strings.text(RunawayStrings::kFakeMedicalChart);
		}
		if (item == 17 && _headWig) {
			return _strings.text(RunawayStrings::kHeadWithWig);
		}
		if (item == 13 && _water == 1) {
			return _strings.text(RunawayStrings::kGlassWithWater);
		}
		return _strings.itemName(_chapter, item);
	}
	if (_chapter == 2) {
		if (item == 3 && _museumDemo) {
			return _strings.text(RunawayStrings::kMuseumDemoCondom);
		}
		if (item == 6 && _museum.frozenBattery) {
			return _strings.text(RunawayStrings::kFrozenBattery);
		}
		if (item == 8 && _museum.rubyRemoved) {
			return _strings.text(RunawayStrings::kMayanPryingTool);
		}
		if (item == 14 && _museum.coffee) {
			return _strings.text(_museum.coffee == 1 ? RunawayStrings::kGroundCoffeeBowl : RunawayStrings::kEmptyBowl);
		}
		if (item == 18 && _museum.batteryInLadle) {
			return _strings.text(RunawayStrings::kLadleWithBattery);
		}
		return _strings.itemName(_chapter, item);
	}
	if (item == 1 && _clothState) {
		return _strings.text(_clothState == 1 ? RunawayStrings::kWetCloth : RunawayStrings::kDirtyCloth);
	}
	if ((item == 10 && _desert.lipstick) || (item == 11 && _desert.lipsticks)) {
		return _strings.text(item == 10 ? RunawayStrings::kMakeshiftBullet : RunawayStrings::kPileOfMakeshiftBullets);
	}
	return _strings.itemName(_chapter, item);
}

uint RunawayInventory::icon(byte item) const {
	if (_chapter == 6) {
		if (item == 19 && _finale.scalpel == 1) {
			return 22;
		}
		if (item == 20 && _finale.podsOpened) {
			return 23;
		}
		return item;
	}
	if (_chapter == 5) {
		return item == 15 && _hopi.toolHandle ? 17 : item;
	}
	if (_chapter == 4) {
		if (item == 11 && _town.strap == 2) {
			return 33;
		}
		if (item == 20 && _town.strap == 2) {
			return 47;
		}
		if (item == 24 && _town.bottleContents) {
			return 33 + _town.bottleContents;
		}
		if (item == 25 && _town.tube30) {
			return 35 + _town.tube30;
		}
		if (item == 26 && _town.tube50) {
			return 38 + _town.tube50;
		}
		if (item == 29 && _town.sculpture) {
			return 43 + _town.sculpture;
		}
		if (item == 31 && _town.mortar) {
			return 46;
		}
		return item;
	}
	if (_chapter == 2) {
		if (item == 7 && _museum.restoredCrucifix)
			return 24;
		if (item == 14 && _museum.coffee)
			return 20 + _museum.coffee;
		if (item == 6 && _museum.frozenBattery)
			return 25;
		if (item == 11 && _museum.rubyRemoved)
			return 20;
		if (item == 18 && _museum.batteryInLadle)
			return 23;
		if (item == 19 && _museum.packageFilled)
			return 26;
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
	if (item == 6) {
		static const byte helmetIcons[] = {6, 32, 33, 36, 37};
		return helmetIcons[_desert.helmet];
	}
	if (item == 14)
		return _desert.belt ? 26 + _desert.belt : 14;
	if (item == 15)
		return _desert.ball == 2 ? 15 : 29 + _desert.ball;
	if (item == 19 && _desert.helmet >= 3)
		return 38;
	if (item == 20 && _desert.glasses == 1)
		return 31;
	if (item == 23)
		return _desert.gunReady || _desert.bombReady || _desert.helicopterReady ?
			38 + _desert.gunReady + _desert.bombReady + _desert.helicopterReady : 23;
	if (item == 25 && _desert.bucket == 1)
		return 42;
	return item == 15 ? 29 : item;
}

uint RunawayInventory::cursor(byte item) const {
	if (_chapter == 6) {
		return item && item <= kFinaleItemCount ? 39 + icon(item) * 2 : 0;
	}
	if (_chapter == 5) {
		return item && item <= kHopiItemCount ? 39 + icon(item) * 2 : 0;
	}
	if (_chapter == 4) {
		return item && item <= kTownItemCount ? 39 + icon(item) * 2 : 0;
	}
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
	if (_chapter == 2) {
		return item && item <= kMuseumItemCount ? (_museumDemo ? 11 + item * 2 : 39 + icon(item) * 2) : 0;
	}
	if (_chapter == 1)
		return item && item <= 19 ? (item == 16 ? 81 : 39 + item * 2 - (item > 16 ? 2 : 0)) : 0;
	if (item == 1 && _clothState)
		return _clothState == 1 ? 107 : 109;
	if (item == 6) {
		static const byte helmetCursors[] = {51, 53, 115, 117, 119};
		return helmetCursors[_desert.helmet];
	}
	if (item == 14)
		return _desert.belt == 1 ? 113 : _desert.belt == 2 ? 71 : 69;
	if (item == 15)
		return _desert.ball == 2 ? 73 : 75 + _desert.ball * 2;
	if (item == 19 && _desert.helmet >= 3)
		return 121;
	if (item == 20 && _desert.glasses == 1)
		return 89;
	if (item == 23)
		return 95 + 2 * (_desert.gunReady + _desert.bombReady + _desert.helicopterReady);
	if (item == 25 && _desert.bucket == 1)
		return 123;
	return item <= kItemCount ? kItemCursors[item] : 0;
}

RunawayInventory::Reply RunawayInventory::examine(byte item) const {
	if (_chapter == 6) {
		if (item == 19) {
			return Reply(310, _finale.scalpel ? _finale.scalpel + 1 : _finale.heatNeeded);
		}
		if (item == 20) {
			return Reply(311, _finale.podsOpened);
		}
		static const uint16 cues[] = {1, 50, 51, 74, 200, 300, 301, 253, 303, 256, 304, 1, 305, 306, 307, 308, 309, 1, 1, 310, 311, 312};
		return Reply(item < ARRAYSIZE(cues) ? cues[item] : 1);
	}
	if (_chapter == 5) {
		static const uint16 cues[] = {1, 50, 51, 74, 200, 250, 251, 252, 253, 254, 255, 256, 251, 257, 258, 259, 260};
		return Reply(item < ARRAYSIZE(cues) ? cues[item] : 1, item == 12 ? 1 : item == 15 ? _hopi.toolHandle : 0);
	}
	if (_chapter == 4) {
		const uint16 cues[] = {1, 50, 51, 74, 53, 200};
		if (item < ARRAYSIZE(cues)) {
			return Reply(cues[item]);
		}
		switch (item) {
		case 7: // Stethoscope.
		case 8: // Liniment.
		case 9: // Cell key.
			return Reply(194 + item);
		case 10: // Wrench.
			return Reply(223);
		case 12: // Concentrated gasoline.
		case 13: // Telepathic helmet.
		case 15: // Blowtorch.
			return Reply(193 + item);
		case 17: // Map of the mine.
		case 18: // Miner's light.
		case 19: // Rope.
			return Reply(192 + item);
		case 24: // Empty, water-filled, or fuel-filled bottle.
			return _town.bottleContents ? Reply(215, _town.bottleContents - 1) : Reply(19, 1);
		case 25: // The 30 cc measuring tube.
			return Reply(216, _town.tube30 == 3 ? 5 : _town.tube30);
		case 26: // The 50 cc measuring tube.
			return Reply(216, _town.tube50 == 5 ? 6 : _town.tube50);
		case 28: // Homemade chewing tobacco.
			return Reply(218);
		case 30: // Sanding block.
			return Reply(220);
		case 11: // Wagon strap.
			return Reply(204, _town.strap);
		case 14: // Firewood.
			return Reply(207);
		case 20: // Ancient stapler.
			return Reply(212, _town.strap == 2);
		case 22: // Pruning shears.
			return Reply(213);
		case 23: // Oil can.
			return Reply(214, _town.oilCanFull);
		case 27: // Tobacco leaves.
			return Reply(217);
		case 29: // Stone with a hole.
			return Reply(219, _town.sculpture * 2);
		case 31: // Clay vessel.
			return Reply(221, _town.mortar);
		case 32: // Bank stamp.
			return Reply(222);
		default:
			return Reply();
		}
	}
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
		if (!item || item > kMuseumItemCount) {
			return Reply();
		}
		if (_fullGame && item == 3) {
			return Reply(74);
		}
		byte variation = 0;
		switch (item) {
		case 5:
			variation = _museum.voiceRecorded;
			break;
		case 6:
			variation = _museum.frozenBattery;
			break;
		case 7:
			variation = _museum.restoredCrucifix;
			break;
		case 8:
		case 11:
			variation = _museum.rubyRemoved;
			break;
		case 14:
			variation = _museum.coffee;
			break;
		case 18:
			variation = _museum.batteryInLadle;
			break;
		case 19:
			variation = _museum.packageFilled;
			break;
		default:
			break;
		}
		return Reply(item < 5 ? 49 + item : 95 + item, variation);
	}
	if (_chapter == 1)
		return Reply(item && item <= 19 ? (item == 3 ? 74 : 49 + item) : 1);
	byte variation = 0;
	switch (item) {
	case 1:
		variation = _clothState;
		break;
	case 6:
		variation = _desert.helmet;
		break;
	case 10:
		variation = _desert.lipstick;
		break;
	case 11:
		variation = _desert.lipsticks;
		break;
	case 14:
		variation = _desert.belt;
		break;
	case 15:
		variation = _desert.ball;
		break;
	case 20:
		variation = _desert.glasses;
		break;
	case 25:
		variation = _desert.bucket == 1;
		break;
	default:
		break;
	}
	return Reply(item && item <= kItemCount ? kExamineCues[item] : 1, variation);
}

Common::U32String RunawayInventory::combinationCaption(byte item, byte target) const {
	if (!_fullGame || !item || !target || item == target) {
		return Common::U32String();
	}
	return RunawayStrings::formatActionCaption(_strings.combinationCaption(_chapter, item, target), name(item), name(target));
}

RunawayInventory::Reply RunawayInventory::randomReply(Common::RandomSource &random) {
	// Native 0x410000 selects weighted refusals for unsuitable combinations.
	const uint roll = random.getRandomNumber(34);
	if (roll < 10) {
		return Reply(14, random.getRandomNumber(2));
	}
	if (roll < 20) {
		return Reply(1, 2);
	}
	if (roll < 25) {
		return Reply(2);
	}
	if (roll < 28) {
		Reply reply(20);
		reply.nextVariation = 1 + random.getRandomNumber(2);
		return reply;
	}
	if (roll < 30) {
		return Reply(1, random.getRandomNumber(2));
	}
	return Reply(roll < 32 ? 25 : roll < 34 ? 26 : 4);
}

RunawayInventory::Reply RunawayInventory::combineHospitalItems(byte first, byte second) {
	assert(first && first <= ARRAYSIZE(kHospitalItemActions) && second && second <= ARRAYSIZE(kHospitalItemActions[0]));
	const byte action = kHospitalItemActions[first - 1][second - 1];
	switch (action) {
	case 69: { // Fill the syringe from the alcohol bottle.
		const byte previous = _syringeFilled;
		_syringeFilled = 1;
		return Reply(69, previous);
	}
	case 70: // Try to pour alcohol into the marker.
		return Reply(70);
	case 71: // Pouring alcohol onto the syringe does not fill it.
		return Reply(70, 1);
	case 72: // Try to pour alcohol into the glass.
		return Reply(70, 2);
	case 73: // Put the wig on the model head.
		_headWig = 1;
		remove(10);
		return Reply(71);
	case 74: // Write on the blank medical chart.
		if (_markerInk < 2) {
			const byte previous = _markerInk;
			_markerInk = 1;
			return Reply(72, previous);
		}
		if (_chartWritten) {
			return Reply(72, 3);
		}
		_chartWritten = 1;
		_markerInk = 3;
		return Reply(72, 2);
	case 75: // Try to write on Gina's medical chart.
		return Reply(72, 4);
	case 76: // Restore the marker using the filled syringe.
		if (!_syringeFilled) {
			return Reply(73);
		}
		if (_markerInk >= 2) {
			return Reply(73, 2);
		}
		_markerInk = 2;
		_syringeFilled = 0;
		return Reply(73, 1);
	default:
		return Reply::fromAction(action);
	}
}

RunawayInventory::Reply RunawayInventory::combineMuseumItems(byte first, byte second) {
	// Varnish, talcum powder and coffee have replies only when selected.
	if (first == 9 || (!_fullGame && second == 9)) {
		return Reply(118);
	}
	if (first == 10 || (!_fullGame && second == 10)) {
		return Reply(119);
	}
	if (first == 15 || (!_fullGame && second == 15)) {
		return Reply(122);
	}
	if ((first == 12 && second == 11) || (!_fullGame && first == 11 && second == 12)) {
		return Reply(120);
	}
	if (first > second) {
		SWAP(first, second);
	}
	if (first == 5 && second == 6) {
		if (!_museum.frozenBattery) {
			return Reply(115);
		}
		_museum.recorderBattery = 3;
		remove(6);
		return Reply(115, 1);
	}
	if (first == 6 && second == 18) {
		if (_museum.frozenBattery) {
			return Reply(116, 2);
		}
		_museum.batteryInLadle = 1;
		remove(6);
		return Reply(116);
	}
	if (first == 8 && second == 11) {
		if (_museum.rubyRemoved) {
			return Reply(117, 2);
		}
		_museum.rubyRemoved = 1;
		add(12);
		return Reply(117);
	}
	if (first == 14 && second == 19) {
		if (_museum.coffee != 1) {
			return Reply(_museum.coffee ? 1 : 121);
		}
		_museum.coffee = 2;
		_museum.packageFilled = 1;
		return Reply(121, 1);
	}
	return _fullGame ? Reply::fromAction(1) : Reply();
}

RunawayInventory::Reply RunawayInventory::combine(byte first, byte second) {
	if (!contains(first) || !contains(second) || first == second)
		return Reply();
	if (_chapter == 1) {
		return combineHospitalItems(first, second);
	}
	if (_chapter == 2) {
		return combineMuseumItems(first, second);
	}
	if (_chapter == 4) {
		return combineTownItems(first, second);
	}
	if (first > second)
		SWAP(first, second);
	if (_chapter == 5) {
		return combineHopiItems(first, second);
	}
	if (_chapter == 6) {
		if (first == 19 && second == 20) {
			if (_finale.podsOpened) {
				return Reply(313, 6);
			}
			if (_finale.scalpel) {
				_finale.scalpel = 2;
				_finale.podsOpened = 1;
				add(10);
				return Reply(313, 4);
			}
			const byte variation = _finale.heatNeeded ? 2 : 0;
			_finale.heatNeeded = 1;
			return Reply(313, variation);
		}
		return Reply::fromAction(1);
	}

	if (_chapter != 3)
		return Reply();
	// Chapter 3's item-pair table (retail 0x46d400).
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
	if (first == 4 && second == 6) {
		if (!_desert.antTrickKnown)
			return Reply(14);
		_desert.helmet = 1;
		remove(4);
		return Reply(178);
	}
	if (first == 6 && second == 18) {
		if (!_desert.antTrickKnown)
			return Reply(14);
		if (!_desert.helmet)
			return Reply(190);
		_desert.helmet = 2;
		remove(18);
		return Reply(190, 1);
	}
	if ((first == 10 || first == 11) && second == 25) {
		if (_desert.bucket != 1)
			return Reply(17);
		if (first == 10) {
			if (_desert.lipstick)
				return Reply(184, 2);
			_desert.lipstick = 1;
			return Reply(184);
		}
		if (_desert.lipsticks)
			return Reply(1, 2);
		_desert.lipsticks = 1;
		_desert.bucket = 2;
		remove(10);
		return Reply(185);
	}
	if ((first == 10 || first == 11) && second == 14) {
		if (!(first == 10 ? _desert.lipstick : _desert.lipsticks))
			return Reply(first == 10 ? 186 : 187);
		_desert.belt = first == 10 ? 1 : 2;
		remove(first);
		return Reply(first == 10 ? 186 : 187, 1);
	}
	if (first == 15 && second == 16) {
		if (_desert.ball)
			return Reply(188, 2);
		_desert.ball = 1;
		return Reply(188);
	}
	if (first == 15 && second == 24) {
		if (_desert.ball != 1)
			return Reply(189, _desert.ball ? 3 : 0);
		_desert.ball = 2;
		return Reply(189, 1);
	}
	if (first == 4 && second == 18)
		return Reply(_desert.antTrickKnown ? 177 : 14);
	if (first == 4 && second == 19)
		return Reply(179);
	if (first == 4 && second == 25)
		return Reply(_desert.antTrickKnown ? 180 : 14, _desert.antTrickKnown && _desert.bucket == 1);
	if (first == 5 && second == 25)
		return Reply(181);
	if (first == 6 && second == 19)
		return Reply(!_desert.helmet ? 23 : _desert.helmet == 3 ? 182 : 14);
	if (first == 18 && second == 19)
		return Reply(191);
	if (first == 18 && second == 25)
		return Reply(_desert.antTrickKnown ? (_desert.bucket == 1 ? 192 : 180) : 14);
	return _fullGame ? Reply::fromAction(1) : Reply();
}

} // End of namespace Hollywood
