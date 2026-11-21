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
#include "hollywood/runaway/scene5160.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{0, 0, 0, {0, 0}, {0, 0}, {0, 0}},
	{556, 310, 0, {301, 302}, {0, 1}, {566, 252}, 5}, // 1: Front Of The Bus.
	{629, 403, 1, {303, 304}, {0, 0}, {755, 340}}, // 2: Refrigerator.
	{578, 466, 5, {305, 306}, {0, 1}, {320, 559}, 3}, // 3: Carla.
	{488, 415, 5, {307, 308}, {0, 0}, {443, 527}}, // 4: Glass.
	{536, 417, 0, {309, 310}, {1, 0}, {536, 388}}, // 5: Grating.
	{229, 475, 7, {311, 312}, {0, 0}, {171, 422}}, // 6: Boots.
	{629, 403, 1, {315, 316}, {0, 0}, {755, 340}}, // 7: Bull.
	{229, 475, 7, {317, 318}, {0, 0}, {171, 422}}, // 8: Painting.
	{229, 475, 7, {313, 314}, {1, 1}, {171, 422}}, // 9: Needle And Thread.
	{556, 310, 0, {301, 302}, {0, 1}, {566, 252}, 4}, // 10: Front Of The Bus.
};

const RunawayResponse kResponses[] = {
	{5160, 301, 1, 0}, // Look toward the front of the bus.
	{5160, 308, 6, 0}, // Take the glass.
	{5160, 310, 8, 0}, // Use the ventilation grating.
	{5160, 311, 9, 0}, // Look at the boots.
	{5160, 312, 10, 0}, // Take the boots.
	{5160, 316, 14, 0}, // Use the bull.
	{5160, 317, 15, 0}, // Look at the painting.
	{5160, 318, 16, 0}, // Take the painting.
	{5160, 321, 19, 0}, // Give the sleeping pill to Carla.
	{5160, 323, 21, 0}, // Give the butter to Carla.
	{5160, 324, 22, 0}, // Put the sleeping pill in the glass.
};

const RunawayItemAction kItemActions[] = {
	{5160, 4, 3, 323, false},
	{5160, 9, 3, 321, false},
	{5160, 9, 4, 324, false},
	{5160, 22, 5, 320, true},
	{5160, 26, 2, 319, true},
	{5160, 26, 3, 322, false},
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene5160 = {
	5160, 1024, 600, kHotspots, ARRAYSIZE(kHotspots), {{100, 100, 100}, {85, 77, 85}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}, {100, 100, 100}},
	Common::Point(519, 438),
	kResponses, ARRAYSIZE(kResponses),
	kItemActions, ARRAYSIZE(kItemActions),
	nullptr, 0
};

namespace Runaway {

Scene5160::Scene5160(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene5160::loadCustomResources() {
	return _ambientAnimations[0].load(_archive, 4, 16) &&
		_ambientAnimations[1].load(_archive, 5, 24) &&
		_ambientAnimations[2].load(_archive, 6, 24) &&
		_ambientAnimations[3].load(_archive, 7, 93) &&
		_refrigeratorAnimation.load(_archive, 12, 1);
}

bool Scene5160::applyCustomBackgroundPatches() {
	return (_desert.needle != 2 || applyPatch(8)) && (!_desert.glassesTaken || applyPatch(9)) &&
		(_desert.lipstickShelf >= 2 || applyPatch(10)) &&
		((_desert.busDoor != 1 && _desert.busDoor != 3) || (applyPatch(11) && applyPatch(13))) &&
		(_desert.busDoor != 2 || applyPatch(14));
}

void Scene5160::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		byte &id = _maps[256 + i];
		if (_desert.needle == 2 && id == 9) {
			id = 0;
		}
	}
}

float Scene5160::actorHeight() const {
	return _position.y * 1.148148179f - 90.370369f;
}

void Scene5160::setupActorDepth(RunawayActor::RenderParams &params) const {
	params.depthY[1] = 599;
}

RunawayRoomScene::ActionResult Scene5160::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 302: // Go to the front of the bus.
		leaveDesertRoom(5171);
		break;
	case 303: // Look at the refrigerator.
		say(2, _desert.fridgeOpened);
		_desert.fridgeSeen = 1;
		break;
	case 304: // Open the refrigerator without the key.
		say(3, _inventory.contains(26));
		break;
	case 305: // Look at Carla.
		say(4, _random.getRandomNumber(_desert.carlaStory ? 3 : 2));
		break;
	case 306: // Talk to Carla.
		startCarlaConversation();
		break;
	case 307: // Look at the glass.
		say(5, _random.getRandomNumber(1));
		break;
	case 309: // Look at the ventilation grating.
		say(7, !_desert.ventSeen ? 0 : _desert.ventCleaned ? 2 : 1);
		break;
	case 313: // Look at the needle and thread.
		say(11, _desert.needle != 0);
		if (!_desert.needle) {
			_desert.needle = 1;
		}
		break;
	case 314: // Take the needle and thread.
		if (!_desert.needle) {
			dispatchCustomSceneAction(313);
		}
		say(12);
		_inventory.add(16);
		_desert.needle = 2;
		say(12, 1);
		return kBackgroundChanged;
	case 315: // Look at the bull.
		say(13, _random.getRandomNumber(1));
		break;
	case 319: // Unlock and search the refrigerator.
		if (!_desert.fridgeOpened) {
			say(17);
		}
		beginRoomAnimationRange(action, 15, 73, 0, 54);
		break;
	case 1319: // Collect the butter from the refrigerator.
		if (!_desert.fridgeOpened) {
			say(17, 1);
			_inventory.add(4);
			say(17, 2);
		} else {
			say(17, 4);
		}
		holdRoomAnimation(419, 15, 73, 54);
		break;
	case 1419: { // Close the refrigerator after Brian finishes speaking.
		static const byte frames[] = {
			54, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
			26, 25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1
		};
		beginRoomAnimation(420, 15, 73, frames, ARRAYSIZE(frames));
		break;
	}
	case 1420: // Finish searching the refrigerator.
		if (!_desert.fridgeOpened) {
			_desert.fridgeOpened = 1;
			say(17, 3);
		}
		break;
	case 320: // Vacuum the ventilation grating.
		if (!_desert.ventSeen) {
			say(18);
		} else if (_desert.ventCleaned) {
			say(18, 4);
		} else {
			say(18, 1);
			beginRoomAnimation(action, 16, 40);
		}
		break;
	case 1320: // Recover the sleeping pill from the vacuum.
		say(18, 2);
		_inventory.add(9);
		_inventory.desert().pill = 1;
		_desert.ventCleaned = 1;
		say(18, 3);
		break;
	case 322: // Give the refrigerator key to Carla.
		say(20, _desert.fridgeOpened);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene5160::runCustomEntrySequence() {
	visitDesertRoom(8, 0);
}

void Scene5160::startCarlaConversation() {
	static const DialogueChoice choices[][8] = {
		{{1,0,1,1,1,1,255}, {1,0,3,2,2,1,255}, {0,1,1,3,3,1,255}, {1,0,3,4,4,1,255}, {1,0,0,5,5,0,0}},
		{{1,0,3,6,6,1,255}, {1,0,3,7,7,2,255}, {1,0,3,8,8,1,255}, {1,0,3,14,14,1,255}, {1,0,2,9,9,0,0}},
		{{1,1,3,10,10,1,255}, {1,1,3,11,11,1,255}, {1,1,3,12,12,1,255}, {1,0,2,13,13,0,0}}
	};
	_conversation = 4;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	memcpy(_dialogue[0], choices[0], sizeof(choices[0]));
	memcpy(_dialogue[10], choices[1], sizeof(choices[1]));
	memcpy(_dialogue[11], choices[2], sizeof(choices[2]));
	_dialogue[0][2].enabled = _desert.fridgeSeen && !_desert.fridgeOpened;
	_dialogue[10][1].enabled = !_desert.ventCleaned;
	say(80);
	say(90, 0, false, 1);
}

void Scene5160::handleCustomDialogueChoice(byte effect) {
	if (effect == 2) {
		_desert.ventSeen = 1;
	}
}

void Scene5160::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || (_roomAction != 319 && _roomAction != 419 && _roomAction != 420)) {
		_refrigeratorAnimation.draw(viewport, 0, _camera.x, -_camera.y);
	}
	drawAmbientAnimations(viewport, _ambientAnimations, 3);
}

void Scene5160::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	_ambientAnimations[3].draw(viewport, _carla.frame(speaker() ? speechFrame() : 0), _camera.x, -_camera.y);
}

void Scene5160::advanceCustomGameplayLoop(uint32 delta) {
	_carla.update(_now, _conversation == 4 || speaker(), _random);
	for (uint i = 0; i < 256; ++i) {
		if (_originalMaps[256 + i] == 4) {
			_maps[256 + i] = _carla.holdsGlass() ? 0 : 4;
		}
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
