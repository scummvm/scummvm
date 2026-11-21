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
#include "hollywood/runaway/scene5080.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{-100, 484, 7, {302, 301}, {0, 3}, {0, 504}, 4}, // 1: Path.
	{103, 518, 1, {303, 304}, {1, 0}, {156, 494}}, // 2: Wooden Box.
	{301, 478, 7, {305, 307}, {0, 1}, {243, 446}}, // 3: Shed.
	{301, 478, 7, {306, 307}, {0, 1}, {243, 446}}, // 4: Door.
	{301, 478, 7, {308, 309}, {0, 1}, {243, 446}}, // 5: Explosives.
	{620, 357, 7, {310, 311}, {0, 0}, {557, 320}}, // 6: Water Tank.
	{987, 378, 1, {312, 313}, {0, 1}, {1027, 350}}, // 7: Train Car.
	{987, 378, 1, {312, 313}, {1, 1}, {1027, 350}, 5}, // 8: Train Car Interior.
	{67, 512, 0, {316, 317}, {0, 0}, {66, 389}}, // 9: Ant-Hill.
	{255, 546, 7, {318, 319}, {1, 0}, {212, 495}}, // 10: Sign.
	{620, 357, 7, {320, 321}, {0, 0}, {610, 339}}, // 11: Ladder.
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}}, // 12: Sawdust.
};

const RunawayResponse kResponses[] = {
	{5080, 302, 1, 0}, // Look at the path.
	{5080, 304, 3, 0}, // Use the wooden box.
	{5080, 308, 7, 0}, // Look at the explosives.
	{5080, 310, 9, 0}, // Look at the water tank.
	{5080, 311, 10, 0}, // Use the water tank.
	{5080, 312, 11, 0}, // Look at the train car.
	{5080, 318, 15, 0}, // Read the warning sign.
	{5080, 319, 16, 0}, // Take the warning sign.
	{5080, 320, 17, 0}, // Look at the ladder.
	{5080, 321, 18, 0}, // Climb the ladder.
	{5080, 324, 19, 0}, // Use the crowbar with the shed door.
	{5080, 325, 20, 0}, // Use an item with the water tank.
};

const RunawayItemAction kItemActions[] = {
	{5080, 0, 6, 325, false},
	{5080, 2, 4, 324, false},
	{5080, 6, 3, 326, false},
	{5080, 6, 4, 326, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5080 = {
	5080, 1432, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {100, 95, 90}, {80, 75, 70}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(128, 526),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5080::Scene5080(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

const char *Scene5080::actionCaptionVariant(byte id, bool look, byte item) const {
	if (!look && !item && id == 3 && _desert.antsDistracted) {
		return "open";
	}
	return nullptr;
}

bool Scene5080::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 18, 769) &&
		_ambientAnimations[1].load(_archive, 17, 672) &&
		_ambientAnimations[2].load(_archive, 16, 455) &&
		_ambientAnimations[3].load(_archive, 20, 631) &&
		_ambientAnimations[4].load(_archive, 19, 533) &&
		_ambientAnimations[5].load(_archive, 15, 24) &&
		loadForeground(0, 11) &&
		loadForeground(1, 12) &&
		loadForeground(2, 13) &&
		loadForeground(3, 14) &&
		_idleAnimation.load(_archive, 9, 47);
}

bool Scene5080::applyCustomBackgroundPatches() {
	return loadRunawayMask(_archive, _background.w, _background.h, _mask, _originalMaps, _desert.antsDistracted ? 4 : 1) &&
		(!_desert.antsDistracted || applyPatch(_desert.dynamite == 1 ? 7 : 8));
}

void Scene5080::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_desert.antsDistracted) {
			if (id == 12) {
				id = _desert.dynamite == 1 ? 5 : 0;
			} else if (id == 5 && _desert.dynamite == 2) {
				id = 3;
			}
		}
	}
}

float Scene5080::actorHeight() const {
	return _position.y * 0.905172408f - 162.241379f;
}

RunawayRoomScene::ActionResult Scene5080::dispatchCustomSceneAction(uint16 action) {
	RunawaySaveState::DesertItems &items = _inventory.desert();
	switch (action) {
	case 301: // Go to the desert overview.
		leaveDesertRoom(5010);
		break;
	case 303: // Look inside the wooden box.
		say(2);
		break;
	case 305: // Look at the shed.
		say(4, _desert.antsDistracted);
		break;
	case 306: // Look at the shed door.
		say(5, _desert.shedTried);
		break;
	case 307: // Try to open the shed.
		if (_desert.antsDistracted) {
			say(14, 0, true);
		} else {
			beginRoomAnimation(action, 25, 28);
		}
		break;
	case 1307: // Finish trying the jammed door.
		say(6, _desert.shedTried);
		_desert.shedTried = 1;
		break;
	case 309: // Take the dynamite.
		_desert.dynamite = 2;
		_inventory.add(5);
		say(8);
		return kBackgroundChanged;
	case 313: // Enter the train car.
		if (!_desert.trainEntered) {
			say(12);
			_desert.trainEntered = 1;
		}
		beginRoomAnimationRange(action, 23, 35, 0, 27);
		break;
	case 1313: // Finish entering the train car.
		leaveDesertRoom(5110);
		break;
	case 1400: // Finish climbing out of the train car.
		walkTo(Common::Point(1000, 451), 5, true);
		visitDesertRoom(5, 1);
		break;
	case 316: // Look at the anthill.
		say(13, _desert.antsDistracted ? 2 : 0);
		break;
	case 317: // Use the anthill.
		if (_desert.antsDistracted) {
			say(14, 2);
		} else {
			say(14);
			say(14, 1);
		}
		break;
	case 326: // Smear the peanut mixture on the shed.
		if (!items.antTrickKnown) {
			say(3, 0, true);
		} else if (items.helmet != 3) {
			say(17, 0, true);
		} else {
			say(21);
			walkTo(Common::Point(564, 497), 7, true);
			byte brian[49] = {}, butter[49] = {};
			for (uint i = 1; i < 39; ++i) {
				brian[i] = i - 1;
			}
			for (uint i = 39; i < ARRAYSIZE(brian); ++i) {
				brian[i] = 48 - i;
			}
			for (uint i = 27; i < ARRAYSIZE(butter); ++i) {
				butter[i] = MIN<uint>(12, i - 26);
			}
			beginRoomAnimation(action, 26, 38, brian, ARRAYSIZE(brian));
			setRoomAnimationOverlay(27, 13, butter, ARRAYSIZE(butter));
		}
		break;
	case 1326: // Finish spreading the mixture and watch for ants.
		_secondaryAnimation.draw(_background, 12, 0, 0);
		say(21, 1);
		deferDesertAction(426);
		break;
	case 426: // Start the ants approaching the shed.
		if (!_antsAnimation.load(_archive, 22, 133)) {
			error("Cannot load Runaway ants animation");
		}
		_antsFrame = 0;
		_antsTick = _now;
		deferDesertAction(436);
		break;
	case 436: // Wait for the ants to swarm.
		if (_now - _antsTick < 4000) {
			deferDesertAction(action);
		} else {
			say(21, 2);
			deferDesertAction(446);
		}
		break;
	case 446: // Step back from the ants.
		walkTo(Common::Point(906, 450), 5, true);
		deferDesertAction(456);
		break;
	case 456: // Watch the ants consume the shed.
		if (_antsFrame < 132) {
			deferDesertAction(action);
		} else {
			say(21, 3);
			deferDesertAction(466);
		}
		break;
	case 466: // Expose the dynamite and replace the shed map.
		_desert.antsDistracted = 1;
		_desert.dynamite = 1;
		items.helmet = 4;
		_audio.stopEffect();
		deferDesertAction(476);
		return kBackgroundChanged;
	case 476: // Approach the remains of the shed.
		walkTo(Common::Point(301, 478), 7);
		deferDesertAction(486);
		break;
	case 486: // Describe the exposed dynamite.
		say(21, 4);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5080::runCustomEntrySequence() {
	if (_entryScene == 5081) {
		_position = Common::Point(987, 379);
		_walkPath.reset(_position);
		_facing = 5;
		updateCamera();
		beginRoomAnimationRange(400, 24, 20, 9, 19);
		return;
	}
	visitDesertRoom(5, 1);
}

Common::U32String Scene5080::hotspotName(byte id) const {
	if (id == 3 && _desert.antsDistracted) {
		return RunawayRoomScene::hotspotName(12);
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene5080::advanceCustomGameplayLoop(uint32 delta) {
	if (_roomAction < 436 || _roomAction > 466) {
		return;
	}
	const uint frame = MIN<uint>(132, (_now - _antsTick) / 80);
	if (_antsFrame < 36 && frame >= 36) {
		_audio.playEffect(60);
	}
	_antsFrame = frame;
}

void Scene5080::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	drawAmbientAnimations(viewport, _ambientAnimations, ARRAYSIZE(_ambientAnimations));
	const uint frame = (_now / 80) % 92;
	_idleAnimation.draw(viewport, frame <= 46 ? frame : 92 - frame, _camera.x, -_camera.y);
	if (_roomAction >= 436 && _roomAction <= 466) {
		_antsAnimation.draw(viewport, _antsFrame, _camera.x, -_camera.y);
	}
}

void Scene5080::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	drawForeground(viewport, 0);
	drawForeground(viewport, 1);
	if (walkZone() == 15 || walkZone() == 16) {
		drawForeground(viewport, 2);
	}
	if (walkZone() == 15) {
		drawForeground(viewport, 3);
	}
}

void Scene5080::handleAnimationFrameHook() {
	if (_roomAction == 313 && _actionFrames[_actionStep] == 21) {
		_audio.playFootstep(4, _random.getRandomBit(), 255, _position.x);
	} else if (_roomAction == 400 && _actionFrames[_actionStep] == 15) {
		_audio.playFootstep(2, _random.getRandomBit(), 230, 1000);
	}
	if (_roomAction == 326 && _actionStep == 23) {
		_audio.playEffect(72);
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
