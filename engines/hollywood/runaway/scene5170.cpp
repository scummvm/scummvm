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
#include "hollywood/runaway/scene5170.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{339, 512, 7, {301, 302}, {0, 0}, {113, 540}}, // 1: Outside Of The Bus.
	{666, 303, 0, {303, 304}, {0, 1}, {679, 290}, 5}, // 2: Back Of The Bus.
	{727, 436, 1, {305, 306}, {1, 1}, {800, 400}}, // 3: Right-Hand Shelf.
	{649, 499, 4, {307, 308}, {0, 1}, {649, 500}}, // 4: Axle.
	{113, 540, 5, {309, 310}, {-1, 1}, {71, 552}}, // 5: Pocket.
	{727, 436, 1, {311, 312}, {0, 1}, {800, 400}}, // 6: Sunglasses.
	{462, 448, 7, {313, 314}, {1, 1}, {424, 414}}, // 7: Item Under Bed.
	{727, 436, 1, {315, 316}, {1, 0}, {800, 400}}, // 8: Middle Shelf.
	{727, 436, 1, {317, 318}, {1, 0}, {800, 400}}, // 9: Left-Hand Shelf.
	{666, 303, 0, {303, 304}, {0, 1}, {679, 290}, 4}, // 10: Back Of The Bus.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 11: Door.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 12: Pile Of Lipstick.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 13: Makeshift Crank.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 14: Handle.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 15: Necklaces.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 16: Rings And Earrings.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 17: Outside Of The Bus.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 18: Right-Hand Shelf.
};

const RunawayResponse kResponses[] = {
	{5170, 303, 3, 0}, // Look toward the back of the bus.
	{5170, 311, 10, 0}, // Look at the sunglasses.
	{5170, 319, 18, 0}, // Return lipstick to the shelf.
};

const RunawayItemAction kItemActions[] = {
	{5170, 10, 3, 319, false},
	{5170, 11, 3, 319, false},
	{5170, 21, 4, 320, true},
	{5170, 26, 5, 15, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5170 = {
	5170, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {80, 72, 80}, {80, 72, 80}, {85, 77, 85}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(339, 512),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5170::Scene5170(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene5170::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item) {
		if (id == 1 && doorClosed()) {
			return "closed";
		}
		if (id == 3 && _desert.lipstickShelf && _desert.lipstickShelf < 3) {
			return "lipstick";
		}
	}
	return nullptr;
}

bool Scene5170::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 6, 25) &&
		_ambientAnimations[1].load(_archive, 5, 24) &&
		_ambientAnimations[2].load(_archive, 4, 50) &&
		_ambientAnimations[3].load(_archive, 11, 3);
}

bool Scene5170::applyCustomBackgroundPatches() {
	_doorFrame = doorClosed() ? 0 : 24;
	return (_desert.vacuum != 2 || applyPatch(7)) && (!_desert.glassesTaken || applyPatch(8)) &&
		(_desert.lipstickShelf != 2 || applyPatch(9)) && (_desert.lipstickShelf != 3 || applyPatch(10));
}

void Scene5170::advanceCustomGameplayLoop(uint32 delta) {
	_carla.update(_now, false, _random);
	while (_doorMoving && _now - _doorTick >= 80) {
		_doorTick += 80;
		const bool opening = doorClosed();
		_doorFrame += opening ? 1 : -1;
		if (_doorFrame == (opening ? 5 : 11)) {
			_audio.playEffect(opening ? 65 : 64, 255, 1);
		}
		_doorMoving = _doorFrame != (opening ? 24 : 0);
	}
}

void Scene5170::handleAnimationFrameHook() {
	if (_roomAction == 308 && _actionStep == (doorClosed() ? 10 : 15)) {
		_doorMoving = true;
		_doorTick = _actionStarted;
		_audio.playEffect(doorClosed() ? 63 : 62);
	}
}

void Scene5170::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (id == 5 && !doorClosed()) {
			id = 1;
		}
		if ((_desert.glassesTaken && id == 6) || (_desert.vacuum == 2 && id == 7)) {
			id = 0;
		}
		if (id == 11) {
			id = _desert.crank == 2 && !doorClosed() ? 4 : 10;
		}
		if (id == 12) {
			id = _desert.crank == 2 && doorClosed() ? 4 : 2;
		}
	}
}

float Scene5170::actorHeight() const {
	return _position.y * 0.536585391f + 165.243896f;
}

void Scene5170::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[4] = 599;
}

int8 Scene5170::customizeSceneActionApproach(byte id, bool use, byte item, int8 approach) const {
	if (!item) {
		if (id == 5 && use && _desert.keyTaken) {
			return 0;
		}
		if (!use) {
			switch (id) {
			case 3:
				return !_desert.lipstickShelf;
			case 7:
				return !_desert.vacuum;
			case 8:
				return !_desert.necklacesSeen;
			case 9:
				return !_desert.ringsSeen;
			default:
				break;
			}
		}
	}
	return approach;
}

RunawayRoomScene::ActionResult Scene5170::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 301: // Look outside the bus.
		say(1, _desert.busDoor != 0);
		break;
	case 302: // Leave the bus.
		if (!doorClosed()) {
			leaveDesertRoom(5061);
		} else {
			say(2);
		}
		break;
	case 304: // Go to the back of the bus.
		leaveDesertRoom(5160);
		break;
	case 305: // Look at the right-hand shelf.
		say(4, _desert.lipstickShelf == 0 ? 0 : _desert.lipstickShelf == 3 ? 2 : 1);
		if (!_desert.lipstickShelf) {
			_desert.lipstickShelf = 1;
		}
		break;
	case 306: // Take lipstick from the shelf.
		if (_desert.lipstickShelf < 2) {
			say(5);
			_inventory.add(10);
			_desert.lipstickShelf = 2;
			say(5, 1);
		} else if (_desert.lipstickShelf == 2) {
			if (!_inventory.desert().lipstick) {
				say(5, 2);
			} else {
				say(5, 3);
				_inventory.add(11);
				_desert.lipstickShelf = 3;
				say(5, 4);
			}
		} else {
			say(5, 5);
		}
		return kBackgroundChanged;
	case 307: // Look at the door mechanism.
		say(6, _desert.crank != 0);
		if (!_desert.crank) {
			_desert.crank = 1;
		}
		break;
	case 308: // Turn the makeshift crank.
		if (_desert.crank < 2) {
			say(7);
		} else {
			beginRoomAnimationRange(action, 13, 29, doorClosed() ? 28 : 0, doorClosed() ? 0 : 28);
		}
		break;
	case 1308: // Finish opening or closing the bus door.
		if (_doorMoving) {
			deferDesertAction(1308);
			break;
		}
		if (!_desert.busDoor) {
			say(7, 1);
		}
		_desert.busDoor = doorClosed() ? 2 : 1;
		return kBackgroundChanged;
	case 309: // Look in the pocket.
		say(8);
		walkTo(Common::Point(339, 512), 3, true);
		break;
	case 310: // Search the pocket for the refrigerator key.
		if (_desert.keyTaken) {
			say(9, 2);
		} else {
			say(9);
			_inventory.add(26);
			_desert.keyTaken = 1;
			if (_desert.fridgeSeen) {
				say(9, 1);
			} else {
				say(18, _random.getRandomNumber(2), true);
			}
			walkTo(Common::Point(340, 511), 3, true);
		}
		break;
	case 312: // Take the sunglasses.
		say(11);
		_inventory.add(20);
		_desert.glassesTaken = 1;
		say(11, 1);
		return kBackgroundChanged;
	case 313: // Look at the object under the bed.
		say(12, _desert.vacuum != 0);
		if (!_desert.vacuum) {
			_desert.vacuum = 1;
		}
		break;
	case 314: // Take the vacuum cleaner.
		if (!_desert.vacuum) {
			say(12);
		}
		_inventory.add(22);
		_desert.vacuum = 2;
		say(13);
		return kBackgroundChanged;
	case 315: // Look at the necklaces.
		say(14, _desert.necklacesSeen);
		_desert.necklacesSeen = 1;
		break;
	case 316: // Take the necklaces.
		if (!_desert.necklacesSeen) {
			dispatchCustomSceneAction(315);
		}
		say(15);
		break;
	case 317: // Look at the rings and earrings.
		say(16, _desert.ringsSeen);
		_desert.ringsSeen = 1;
		break;
	case 318: // Take the rings and earrings.
		if (!_desert.ringsSeen) {
			dispatchCustomSceneAction(317);
		}
		say(17);
		break;
	case 320: // Attach the footrest to the door mechanism.
		say(19);
		beginRoomAnimation(action, 12, 42);
		break;
	case 1320: // Finish assembling the makeshift crank.
		_inventory.remove(21);
		_desert.crank = 2;
		say(19, 1);
		return kBackgroundChanged;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5170::runCustomEntrySequence() {
	if (_entryScene == 5171) {
		_position = Common::Point(636, 382);
		_walkPath.reset(_position);
		_facing = 4;
	}
	visitDesertRoom(9, 2);
}

Common::U32String Scene5170::hotspotName(byte id) const {
	if (id == 1 && doorClosed()) {
		return RunawayRoomScene::hotspotName(11);
	}
	if (id == 3 && _desert.lipstickShelf && _desert.lipstickShelf < 3) {
		return RunawayRoomScene::hotspotName(12);
	}
	if (id == 4 && _desert.crank == 2) {
		return RunawayRoomScene::hotspotName(13);
	}
	if (id == 7 && _desert.vacuum) {
		return RunawayRoomScene::hotspotName(14);
	}
	if (id == 8 && _desert.necklacesSeen) {
		return RunawayRoomScene::hotspotName(15);
	}
	if (id == 9 && _desert.ringsSeen) {
		return RunawayRoomScene::hotspotName(16);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene5170::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	for (uint i = 0; i < 3; ++i) {
		const RunawayAnimation &animation = _ambientAnimations[i];
		if (!animation.frameCount()) {
			continue;
		}
		uint frame = (_now / 80) % animation.frameCount();
		if (i == 0) {
			frame = _doorFrame;
		}
		if (i == 2) {
			frame = _carla.frame();
		}
		animation.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

void Scene5170::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	// Brian's assembly and turning animations already include the crank.
	if (!drawingRoomAnimation()) {
		const uint frame = _desert.crank == 2 ? (doorClosed() ? 1 : 2) : 0;
		_ambientAnimations[3].draw(viewport, frame, _camera.x, -_camera.y);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
