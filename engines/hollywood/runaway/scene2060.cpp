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
#include "hollywood/runaway/scene2060.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// RESOURCE.B06; the storeroom reached across the ledge (native 0x4478e0).
const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{419, 546, 7, {301, 302}, {1, 1}, {419, 544}, 4}, // 1: Window.
	{419, 546, 7, {304, 305}, {0, 0}, {419, 544}}, // 2: Radiator.
	{347, 563, 7, {306, 307}, {1, 1}, {284, 521}}, // 3: Box.
	{347, 563, 7, {308, 309}, {0, 1}, {292, 521}}, // 4: Spray can.
	{587, 564, 7, {310, 311}, {1, 1}, {545, 513}}, // 5: Book.
	{627, 564, 7, {312, 313}, {1, 1}, {595, 513}}, // 6: Piece of paper.
	{627, 564, 1, {314, 315}, {0, 1}, {682, 513}}, // 7: Human body.
	{627, 564, 7, {316, 8}, {0, 0}, {595, 513}}, // 8: File cabinet.
	{627, 564, 1, {317, 8}, {0, 0}, {733, 577}}, // 9: File cabinet.
	{627, 564, 1, {318, 319}, {0, 0}, {733, 577}}, // 10: Bottles.
	{627, 564, 7, {320, 321}, {0, 0}, {595, 513}}, // 11: Box.
	{627, 564, 5, {322, 323}, {0, 0}, {238, 557}}, // 12: Boxes of medicine.
	{627, 564, 3, {324, 5}, {0, 0}, {958, 599}}, // 13: Serum stand.
	{627, 564, 3, {325, 326}, {0, 0}, {958, 599}}, // 14: Serum bag.
	{347, 563, 5, {327, 328}, {0, 1}, {242, 576}} // 15: Pillows.
};

// Item IDs 1-19, then hotspot IDs in kHotspots order. Zero uses kItemActions.
const byte kItemResponses[19][15] = {
	{19,  5,  5, 20,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 1: Wallet.
	{19,  5,  5, 20,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 2: Car keys.
	{19,  5,  5, 20,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 3: Order form.
	{19,  5,  5, 20,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 4: Amber.
	{19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 5: Gina's bag.
	{19,  5,  5,  5,  5,  1,  5,  5,  5,  3,  5,  5,  5,  3,  5}, // 6: Gina's chart.
	{19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 7: Sheet.
	{19,  5,  5,  5, 24,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 8: Pills.
	{19,  5, 29, 20, 29, 29, 29,  5,  5,  5, 29, 29,  5, 29, 29}, // 9: Matches.
	{19,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 10: Wig.
	{19,  5,  5, 15,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 11: Marker.
	{19,  5,  5, 20,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 12: Alcohol.
	{19,  5,  5, 17, 17, 17, 17,  5,  5, 17,  5,  5, 17, 17,  5}, // 13: Glass.
	{19,  5,  5,  5,  3,  3,  5,  5,  5,  5,  5,  5,  5,  3,  1}, // 14: Pillows.
	{19,  5,  5, 20,  3,  3,  5,  5,  5,  5,  5,  5,  5, 17,  5}, // 15: Syringe.
	{19,  5,  5,  5,  5,  1,  5,  5,  5,  5,  5,  5,  5,  5,  5}, // 16: Blank chart.
	{19,  5,  5,  5,  3,  3, 20,  5,  5,  5,  5,  5,  5,  5,  5}, // 17: Head.
	{19,  5,  5,  5, 20, 20, 20,  5,  5,  5,  5,  5,  5,  5,  5}, // 18: Spray.
	{19,  5,  5,  5,  1,  1,  1,  3,  5,  5,  5,  5,  5,  5,  5}, // 19: Medical reference book.
};

const RunawayResponse kResponses[] = {
	{kHospitalStoreroom, 301, 1, 0}, // Look at the window.
	{kHospitalStoreroom, 304, 2, 0}, // Look at the radiator.
	{kHospitalStoreroom, 305, 2, 1}, // Use the radiator.
	{kHospitalStoreroom, 308, 4, 0}, // Look at the spray can.
	{kHospitalStoreroom, 316, 8, 0}, // Look at the left file cabinet.
	{kHospitalStoreroom, 317, 9, 0}, // Look at the right file cabinet.
	{kHospitalStoreroom, 318, 10, 0}, // Look at the bottles.
	{kHospitalStoreroom, 319, 10, 1}, // Take the bottles.
	{kHospitalStoreroom, 320, 11, 0}, // Look at the box beneath the book.
	{kHospitalStoreroom, 321, 11, 1}, // Open the box beneath the book.
	{kHospitalStoreroom, 322, 12, 0}, // Look at the medicine boxes.
	{kHospitalStoreroom, 323, 12, 1}, // Take the medicine boxes.
	{kHospitalStoreroom, 324, 13, 0}, // Look at the serum stand.
	{kHospitalStoreroom, 325, 14, 0}, // Look at the serum bag.
	{kHospitalStoreroom, 326, 14, 1}, // Take the serum bag.
	{kHospitalStoreroom, 327, 15, 0} // Look at the pillows.
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene2060 = {
	kHospitalStoreroom, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {40, 40, 60}, {60, 60, 80}}, Common::Point(419, 546),
	kResponses, ARRAYSIZE(kResponses), nullptr, 0, nullptr, 0
};

namespace Runaway {

Scene2060::Scene2060(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

byte Scene2060::defaultItemAction(byte item, byte id) const {
	if (!item || item > ARRAYSIZE(kItemResponses) || !id || id > ARRAYSIZE(kItemResponses[0])) {
		return 1;
	}
	return kItemResponses[item - 1][id - 1];
}

bool Scene2060::applyCustomBackgroundPatches() {
	return (!_hospital.headTaken || applyPatch(5)) && (_hospital.blankChart != 2 || applyPatch(6)) &&
		(!_hospital.sprayTaken || applyPatch(7)) && (_hospital.book != 2 || applyPatch(8)) &&
		(!_hospital.pillowsTaken || applyPatch(9));
}

void Scene2060::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if ((_hospital.sprayTaken && id == 4) || (_hospital.pillowsTaken && id == 15) ||
				(_hospital.headTaken && id == 16)) {
			id = 0;
		} else if (id == 5 && _hospital.book == 2) {
			id = 11;
		} else if (id == 6 && _hospital.blankChart == 2) {
			id = 8;
		} else if (id == 16) {
			id = 7;
		}
	}
}

RunawayRoomScene::ActionResult Scene2060::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Climb back to Gina's room.
		beginRoomAnimation(action, 4, 44);
		return kHandled;
	case 303: // Return to Gina's room without the climbing animation.
		_transition.scene = 2022;
		_transition.position = Common::Point(265, 456);
		_transition.facing = 3;
		return kHandled;
	case 306: // Look inside the syringe box.
		say(3, _hospital.syringesSeen);
		_hospital.syringesSeen = 1;
		return kHandled;
	case 307: // Take a syringe.
		if (_inventory.contains(15)) {
			say(3, 3);
		} else {
			say(3, 2);
			_inventory.add(15);
			say(64, 0, true);
		}
		return kHandled;
	case 309: // Take the spray can.
		_hospital.sprayTaken = 1;
		_inventory.add(18);
		say(18, _random.getRandomNumber(2), true);
		return kBackgroundChanged;
	case 310: // Look at the medical reference book.
		say(5, _hospital.book != 0);
		_hospital.book = 1;
		return kHandled;
	case 311: // Take the medical reference book.
		_hospital.book = 2;
		_inventory.add(19);
		say(18, _random.getRandomNumber(2), true);
		return kBackgroundChanged;
	case 312: // Look at the blank medical chart.
		say(6, _hospital.blankChart != 0);
		_hospital.blankChart = 1;
		return kHandled;
	case 313: // Take the blank medical chart.
		_hospital.blankChart = 2;
		_inventory.add(16);
		say(65, 0, true);
		return kBackgroundChanged;
	case 314: // Look at the anatomical model.
		say(7, _hospital.headTaken ? 2 : _random.getRandomBit());
		return kHandled;
	case 315: // Take the model head.
		if (_hospital.headTaken) {
			say(7, 4);
			return kHandled;
		}
		say(7, 3);
		_hospital.headTaken = 1;
		_inventory.add(17);
		say(66, 0, true);
		return kBackgroundChanged;
	case 328: // Take the pillows.
		say(15, 1);
		_hospital.pillowsTaken = 1;
		_inventory.add(14);
		say(63, 0, true);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
}

void Scene2060::runCustomEntrySequence() {
	if (_inventory.water() == 1) {
		_inventory.setWater(2);
	}
	static const byte frames[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
	beginRoomAnimation(_entryScene, 10, 13, frames, ARRAYSIZE(frames));
}

void Scene2060::finishCustomAnimation(uint16 action) {
	switch (action) {
	case 302: // Return through the storeroom window.
		_transition.scene = 2022;
		_transition.position = Common::Point(265, 456);
		_transition.facing = 3;
		return;
	case kHospitalStoreroom:
		if (!_hospital.storeroomVisited) {
			say(0);
		}
		_hospital.storeroomVisited = 1;
		_facing = 3;
		return;
	default:
		return;
	}
}

void Scene2060::handleAnimationFrameHook() {
	if (_roomAction == 302 && (_actionStep == 23 || _actionStep == 36)) {
		const byte volume = CLIP<int>((int)(actorHeight() * 255 / 455), 0, 255);
		_audio.playFootstep(1, _random.getRandomBit(), volume, _position.x);
	}
}

float Scene2060::actorHeight() const {
	return _position.y * 3.55555558f - 1550.33337f;
}

Common::U32String Scene2060::hotspotName(byte id) const {
	if (id == 3 && _hospital.syringesSeen) {
		return _strings.text(RunawayStrings::kBoxOfSyringes);
	}
	if (id == 5 && _hospital.book) {
		return _strings.itemName(1, 19);
	}
	if (id == 6 && _hospital.blankChart) {
		return _strings.text(RunawayStrings::kMedicalChart);
	}
	return RunawayRoomScene::hotspotName(id);
}

} // End of namespace Runaway
} // End of namespace Hollywood
