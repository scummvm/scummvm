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
#include "hollywood/runaway/scene5110.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{327, 441, 7, {301, 302}, {0, 1}, {247, 373}, 4}, // 1: Outside Of Train Car.
	{574, 434, 1, {303, 304}, {1, 0}, {618, 400}}, // 2: Barrels.
	{801, 441, 3, {305, 306}, {0, 0}, {835, 467}}, // 3: Chain.
	{517, 374, 0, {307, 308}, {0, 0}, {517, 353}}, // 4: Contraptions.
	{510, 377, 0, {309, 310}, {1, 1}, {517, 353}}, // 5: Barrel.
	{365, 485, 5, {311, 312}, {1, 1}, {347, 500}}, // 6: Bucket.
	{801, 441, 3, {313, 314}, {0, 0}, {937, 558}}, // 7: Clock.
	{510, 377, 0, {320, 321}, {0, 1}, {517, 353}}, // 8: Bucket.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 9: Barrel Of Gunpowder.
};

const RunawayResponse kResponses[] = {
	{5110, 301, 1, 0}, // Look at the exit door.
	{5110, 304, 3, 0}, // Open the compartment door by hand.
	{5110, 305, 4, 0}, // Look at the chain.
	{5110, 306, 5, 0}, // Pull the chain.
	{5110, 307, 6, 0}, // Look at the contraptions.
	{5110, 308, 7, 0}, // Use the contraptions.
	{5110, 313, 12, 0}, // Look at the clock.
	{5110, 314, 13, 0}, // Use the clock.
	{5110, 315, 14, 0}, // Use the crowbar with the barrel.
	{5110, 318, 15, 0}, // Use the crowbar with the chain.
};

const RunawayItemAction kItemActions[] = {
	{5110, 2, 2, 319, true},
	{5110, 2, 3, 318, false},
	{5110, 2, 5, 315, false},
	{5110, 4, 5, 324, false},
	{5110, 6, 5, 323, false},
	{5110, 7, 5, 316, false},
	{5110, 17, 5, 317, true},
	{5110, 25, 5, 322, true},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5110 = {
	5110, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {75, 70, 65}, {65, 60, 55}, {55, 50, 45}, {45, 40, 35}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(327, 441),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5110::Scene5110(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5110::loadCustomResources() {
	Graphics::ManagedSurface scratch(1, 1, _screen.format);
	return _archive.readVariableChunk(14, _gunpowder) && applyRunawayPatch(_gunpowder, scratch) &&
		_curtainAnimation.load(_archive, 4, 50) &&
		_shadowAnimation.load(_archive, 5, 50) &&
		_bucketAnimation.load(_archive, 8, 1) &&
		_doorAnimation.load(_archive, 9, 2);
}

bool Scene5110::applyCustomBackgroundPatches() {
	return _desert.barrel != 2 || applyPatch(13);
}

void Scene5110::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 8) {
			id = _desert.bucket == 1 || _desert.bucket == 2 ? 8 : 4;
		}
	}
}

bool Scene5110::adjustCustomWalkTargetToFloorMask(Common::Point &target) const {
	target.x = CLIP<int>(target.x, 293, 801);
	return false;
}

float Scene5110::actorHeight() const {
	return _position.y * 1.142857194f - 170.571426f;
}

void Scene5110::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
	params.depthY[2] = 408;
}

RunawayRoomScene::ActionResult Scene5110::dispatchCustomSceneAction(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 1400: // Finish climbing into the train car.
		visitDesertRoom(7, 1);
		break;
	case 302: // Leave the train car.
		beginRoomAnimation(action, 6, 12, nullptr, 0, true, 60);
		_actionFrames.insert_at(0, 0);
		break;
	case 1302: // Finish climbing out of the train car.
		leaveDesertRoom(5081);
		break;
	case 303: // Look at the closed compartment door.
		say(2, _desert.trainDoorOpen != 0);
		break;
	case 309: // Look at the barrel.
		say(8, _desert.barrel);
		if (!_desert.barrel) {
			_desert.barrel = 1;
		}
		break;
	case 310: // Use the barrel.
		if (!_desert.barrel) {
			dispatchCustomSceneAction(309);
		}
		say(9, _desert.barrel == 2);
		break;
	case 311: // Look inside the bucket.
		say(10, _desert.screwSeen != 0);
		_desert.screwSeen = 1;
		break;
	case 312: // Take the screw from the bucket.
		if (_desert.screwTaken) {
			say(11, 2);
		} else {
			if (!_desert.screwSeen) {
				dispatchCustomSceneAction(311);
			}
			say(11);
			_inventory.add(7);
			_desert.screwTaken = 1;
			say(11, 1);
		}
		break;
	case 316: // Use the screw with the barrel.
		say(16);
		_desert.powderNeeded = 1;
		break;
	case 317: // Drill the barrel with the makeshift drill.
		if (!_desert.bucket) {
			say(17);
			_desert.powderNeeded = 1;
		} else if (_desert.bucket == 1) {
			say(17, 1);
			static const byte frames[] = {
				30, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 40, 41, 42, 43, 44, 45, 46, 47, 48,
				39, 38, 37, 36, 35, 34, 33, 32, 31, 30
			};
			beginRoomAnimation(action, 11, 49, frames, ARRAYSIZE(frames), true, 60);
		} else {
			say(17, 3);
		}
		break;
	case 1317: // Let the gunpowder fall while Brian watches.
		beginRoomAnimation(417, 12, 121, nullptr, 0, false, 60);
		break;
	case 1417: // Finish filling the bucket with gunpowder.
		_desert.barrel = 2;
		_desert.bucket = 2;
		say(17, 2);
		return kBackgroundChanged;
	case 319: // Pry open the compartment door.
		if (_desert.trainDoorOpen) {
			say(18, 3);
		} else {
			say(18);
			beginRoomAnimation(action, 10, 49, nullptr, 0, true, 60);
			_actionFrames.insert_at(0, 0);
		}
		break;
	case 1319: // Collect the peanuts behind the door.
		_desert.trainDoorOpen = 1;
		say(18, 1);
		_inventory.add(18);
		say(18, 2);
		return kBackgroundChanged;
	case 320: // Look at the bucket beneath the barrel.
		say(19, _desert.bucket != 1);
		break;
	case 321: // Take the bucket beneath the barrel.
		if (_desert.bucket == 1) {
			say(20);
		} else {
			_desert.bucket = 3;
			items.bucket = 1;
			_inventory.add(25);
			say(20, 1);
			return kBackgroundChanged;
		}
		break;
	case 322: // Place the empty bucket beneath the barrel.
		if (!_desert.powderNeeded) {
			say(21);
		} else if (_desert.bucket) {
			say(17, 3);
		} else {
			say(21, 1);
			beginRoomAnimationRange(action, 11, 49, 0, 29, true, 60);
			_actionFrames.insert_at(0, 0);
		}
		break;
	case 1322: // Finish positioning the bucket.
		_desert.bucket = 1;
		_inventory.remove(25);
		return kBackgroundChanged;
	case 323: // Use the helmet with the barrel.
		if (!_desert.powderNeeded) {
			say(14, 0, true);
		} else {
			say(22, items.helmet == 0);
		}
		break;
	case 324: // Use the butter with the barrel.
		say(_desert.powderNeeded ? 23 : 14, 0, !_desert.powderNeeded);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5110::runCustomEntrySequence() {
	_facing = 3;
	beginRoomAnimation(400, 7, 12, nullptr, 0, true, 60);
	_actionFrames.insert_at(0, 0);
}

void Scene5110::handleAnimationFrameHook() {
	if (_roomAction == 400 && _actionStep == 10) {
		_audio.playFootstep(3, _random.getRandomBit(), 230, 327);
	} else if (_roomAction == 319 && _actionStep == 28) {
		_audio.playEffectAt(53, Common::Point(632, 200));
	} else if (_roomAction == 322 && _actionStep == 22) {
		_audio.playEffectAt(38, Common::Point(512, 200), 204);
	} else if (_roomAction == 317 && _actionStep == 10) {
		_audio.playEffectAt(36, Common::Point(512, 200), 230);
	} else if (_roomAction == 417) {
		if (_actionStep == 10) {
			_audio.playEffectAt(37, Common::Point(512, 200), 204);
		} else if (_actionStep == 80) {
			_desert.bucket = 2;
			updateMaps();
		}
	}
}

Common::U32String Scene5110::hotspotName(byte id) const {
	if (id == 5 && _desert.barrel) {
		return RunawayRoomScene::hotspotName(9);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene5110::advanceCustomGameplayLoop(uint32 delta) {
	_curtainTime = (_curtainTime + delta) % (98 * 80);
}

void Scene5110::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (_desert.bucket != 0 && _desert.bucket != 3) {
		_bucketAnimation.draw(viewport, 0, _camera.x, -_camera.y);
		if (_desert.bucket == 2) {
			applyRunawayPatch(_gunpowder, viewport, -_camera.y, -_camera.x);
		}
	}
	_doorAnimation.draw(viewport, _desert.trainDoorOpen != 0, _camera.x, -_camera.y);
	const uint step = _curtainTime / 80;
	const uint frame = step <= 49 ? step : 98 - step;
	_curtainAnimation.draw(viewport, frame, _camera.x, -_camera.y);
	_shadowAnimation.draw(viewport, frame, _camera.x, -_camera.y);
}

} // End of namespace Runaway
} // End of namespace Hollywood
