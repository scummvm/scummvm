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

#include "common/textconsole.h"
#include "common/util.h"
#include "hollywood/runaway/scene2080.h"
#include "hollywood/runaway/strings.h"

namespace Hollywood {
namespace {

// RESOURCE.B08; interaction points and actions from native 0x44a430.
const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{526, 441, 1, {301, 302}, {0, 1}, {595, 423}}, // 1: Wastebasket.
	{587, 471, 1, {309, 310}, {0, 0}, {682, 439}}, // 2: Faucet.
	{50, 586, 5, {308, 303}, {0, 2}, {0, 580}, 7}, // 3: Door.
	{577, 466, 1, {305, 306}, {1, 1}, {647, 429}}, // 4: Bottle.
	{442, 377, 0, {311, 312}, {0, 0}, {442, 377}}, // 5: Sign.
	{490, 372, 0, {313, 314}, {0, 0}, {490, 372}}, // 6: Pipe.
	{442, 377, 0, {315, 316}, {0, 0}, {442, 377}}, // 7: Shower nozzle.
	{595, 423, 0, {317, 318}, {0, 0}, {595, 423}}, // 8: Medicine cabinet.
	{295, 327, 0, {319, 320}, {0, 0}, {295, 327}} // 9: Toilet bowl.
};

// Item IDs 1-19, then hotspot IDs in kHotspots order. Zero uses kItemActions.
const byte kItemResponses[19][9] = {
	{19, 20,  1,  5,  1,  0,  1,  1, 19}, // 1: Wallet.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 2: Car keys.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 3: Order form.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 4: Amber.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 5: Gina's bag.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 6: Gina's chart.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 7: Sheet.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 8: Pills.
	{19,  5,  1, 20,  1,  0,  1,  1, 19}, // 9: Matches.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 10: Wig.
	{19, 17,  1, 24,  1,  0,  1,  1, 19}, // 11: Marker.
	{19,  5,  1,  1,  1,  0,  1,  1, 19}, // 12: Alcohol.
	{19,  0,  1, 17,  1,  0,  1,  1, 19}, // 13: Glass.
	{19,  5,  1,  5,  1,  0,  1,  1, 19}, // 14: Pillows.
	{19, 17,  1, 24,  1,  0,  1,  1, 19}, // 15: Syringe.
	{19, 20,  1,  5,  1,  0,  1,  1, 19}, // 16: Blank chart.
	{19, 20,  1,  5,  1,  0,  1,  1, 19}, // 17: Head.
	{19, 20,  1,  5,  1,  0,  1,  1, 19}, // 18: Spray.
	{19, 20,  1,  5,  1,  0,  1,  1, 19}, // 19: Medical reference book.
};

const RunawayResponse kResponses[] = {
	{kHospitalBathroom, 301, 1, 0}, // Look at the wastebasket.
	{kHospitalBathroom, 309, 5, 0}, // Look at the faucet.
	{kHospitalBathroom, 310, 5, 1}, // Use the faucet.
	{kHospitalBathroom, 313, 8, 0}, // Look at the pipe.
	{kHospitalBathroom, 314, 9, 0}, // Use the pipe.
	{kHospitalBathroom, 315, 10, 0}, // Look at the shower nozzle.
	{kHospitalBathroom, 317, 12, 0}, // Look at the medicine cabinet.
	{kHospitalBathroom, 318, 13, 0}, // Open the medicine cabinet.
	{kHospitalBathroom, 319, 14, 0}, // Look at the toilet.
	{kHospitalBathroom, 321, 16, 0} // Use an item with the pipe.
};

const RunawayItemAction kItemActions[] = {
	{kHospitalBathroom, 13, 2, 307}, // Fill the glass at the faucet.
	{kHospitalBathroom, 0, 6, 321, false} // Use an item with the pipe.
};

// The faucet animation opens with a held frame, then returns from frame 22.
const byte kFillGlassFrames[] = {
	0, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
	15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
	31, 32, 33, 34, 35, 36, 37, 22, 21, 20, 19, 18, 17, 16, 15, 14,
	13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene2080 = {
	kHospitalBathroom, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {90, 90, 100}, {70, 70, 80}}, Common::Point(227, 546),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene2080::Scene2080(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

byte Scene2080::defaultItemAction(byte item, byte id) const {
	if (!item || item > ARRAYSIZE(kItemResponses) || !id || id > ARRAYSIZE(kItemResponses[0])) {
		return 1;
	}
	return kItemResponses[item - 1][id - 1];
}

bool Scene2080::loadCustomResources() {
	return _wastebasketAnimation.load(_archive, 6, 43) && loadForeground(0, 7) && loadForeground(1, 8);
}

bool Scene2080::applyCustomBackgroundPatches() {
	return _hospital.alcohol != 1 || applyPatch(5);
}

void Scene2080::applyCustomHotspotState() {
	if (_hospital.alcohol == 1) {
		for (uint i = 0; i < 256; ++i) {
			if (_maps[256 + i] == 4) {
				_maps[256 + i] = 0;
			}
		}
	}
}

RunawayRoomScene::ActionResult Scene2080::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 312: // Use the sign.
	case 316: // Use the shower nozzle.
		say(action == 312 ? 7 : 11, _hospital.medicineRead);
		return kHandled;
	case 302: // Search the wastebasket for the marker.
		_roomAction = action;
		_actionStage = 0;
		_actionStarted = _now;
		_actionStep = 0;
		return kHandled;
	case 303: // Return to Gina's room.
	case 304: // Return to Gina's room without the door sound.
		leaveHospitalRoom(2021, Common::Point(1265, 546), 5);
		return kHandled;
	case 305: // Look at the alcohol bottle.
		say(3, _hospital.alcohol == 2 ? 1 : 0);
		_hospital.alcohol = 2;
		return kHandled;
	case 306: // Take the alcohol bottle.
		_hospital.alcohol = 1;
		_inventory.add(12);
		say(18, _random.getRandomNumber(2), true);
		return kBackgroundChanged;
	case 307: // Fill the glass at the faucet.
		if (_inventory.water() == 1) {
			say(4, 1);
			return kHandled;
		}
		if (!_faucetAnimation.load(_archive, 4, 38)) {
			error("Cannot load Runaway bathroom faucet animation");
		}
		_roomAction = action;
		_actionStarted = _now;
		_actionStep = 0;
		return kHandled;
	case 308: // Look at the bathroom door.
		say(2, _hospital.attacked);
		return kHandled;
	case 311: // Read the sign.
	case 320: // Use the toilet.
		say(action == 311 ? 6 : 15);
		_roomAction = action;
		return kHandled;
	default:
		return kUnhandled;
	}
}

void Scene2080::runCustomEntrySequence() {
	_audio.playEffect(48);
	walkTo(Common::Point(227, 546), 1, true);
}

bool Scene2080::updateCustomSequence() {
	if (!_hospital.attacked && _hospital.chartMoved && _hospital.decoy && !_walkPath.active()) {
		_hospital.attacked = 1;
		_transition.scene = 2023;
		_transition.position = Common::Point(1075, 534);
		_transition.facing = 7;
		return true;
	}
	if (_roomAction == 302) {
		if (_actionStage == 1) {
			if (speaking()) {
				return true;
			}
			_actionStage = 2;
			_actionStarted = _now;
			_actionStep = 0;
		}
		const uint previous = _actionStep;
		_actionStep = (_now - _actionStarted) / 80;
		if (!_actionStage) {
			if (previous < 12 && _actionStep >= 12) {
				_audio.playEffect(31);
			}
			if (_actionStep >= 19) {
				_actionStage = 1;
				_actionStep = 19;
				say(1, _hospital.markerTaken ? 2 : 3);
			}
		} else if (_actionStage == 2) {
			if (_hospital.markerTaken && previous < 2 && _actionStep >= 2) {
				_audio.playEffect(30, 204);
			}
			const uint duration = _hospital.markerTaken ? 19 : 12;
			if (_actionStep >= duration) {
				_roomAction = 0;
				if (!_hospital.markerTaken) {
					_hospital.markerTaken = 1;
					_inventory.add(11);
					say(1, 1);
				}
			}
		}
	} else if (_roomAction == 307) {
		const uint previous = _actionStep;
		_actionStep = (_now - _actionStarted) / 80;
		if (previous < 20 && _actionStep >= 20) {
			_audio.playEffect(38, 204);
		}
		if (_actionStep >= ARRAYSIZE(kFillGlassFrames)) {
			_roomAction = 0;
			_inventory.setWater(1);
			_inventory.remove(13);
			_inventory.add(13);
			say(4);
		}
	} else if ((_roomAction == 311 || _roomAction == 320) && !speaking()) {
		_facing = 4;
		say(_roomAction == 311 ? 6 : 15, 1);
		_roomAction = 0;
	}
	return true;
}

void Scene2080::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_roomAction == 302) {
		uint frame = MIN<uint>(19, _actionStep);
		if (_actionStage == 1) {
			frame = 19 + speechFrame();
		} else if (_actionStage == 2) {
			frame = _hospital.markerTaken ? 19 - MIN<uint>(18, _actionStep) : _actionStep ? 30 + _actionStep : 19;
		}
		_wastebasketAnimation.draw(viewport, frame, 0, 0);
	} else {
		_wastebasketAnimation.draw(viewport, 42, 0, 0);
		if (_roomAction == 307) {
			_faucetAnimation.draw(viewport, kFillGlassFrames[MIN<uint>(_actionStep, ARRAYSIZE(kFillGlassFrames) - 1)], 0, 0);
		}
	}
}

void Scene2080::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (shouldDrawActor()) {
		if (_position.x < 300) {
			drawForeground(viewport, 1);
		} else if (_position.x > 350) {
			drawForeground(viewport, 0);
		}
	}
}

bool Scene2080::shouldDrawActor() const {
	return _roomAction != 302 && _roomAction != 307;
}

int8 Scene2080::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (id == 4 && !use && !item && _hospital.alcohol == 2) {
		return false;
	}
	return approach;
}

float Scene2080::actorHeight() const {
	return _position.y * 0.266272187f + 295.502960f;
}

Common::U32String Scene2080::hotspotName(byte id) const {
	if (id == 4 && _hospital.alcohol == 2) {
		return _strings.itemName(1, 12);
	}
	return RunawayRoomScene::hotspotName(id);
}

} // End of namespace Runaway
} // End of namespace Hollywood
