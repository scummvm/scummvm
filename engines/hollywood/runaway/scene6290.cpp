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
#include "hollywood/runaway/scene6290.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Crater", 0, 537, 5, {301, 302}, {0, 3}},
	{"Trail", 1739, 442, 3, {303, 304}, {0, 3}},
	{"Panel of buttons", 440, 556, 7, {305, 306}, {0, 1}},
	{"Switch", 607, 549, 7, {307, 308}, {0, 0}},
	{"Motorcycle", 1133, 434, 0, {309, 310}, {0, 0}},
	{"Camping tent", 1396, 293, 3, {311, 312}, {0, 0}},
	{"Lighting panel", 0, 0, 7, {313, 314}, {0, 0}},
	{"Joshua", 440, 556, 7, {315, 316}, {0, 1}},
	{"Strap", 0, 0, 7, {317, 318}, {0, 0}},
	{"Wrench", 0, 0, 7, {319, 320}, {0, 0}},
	{"Machine", 0, 0, 0, {321, 322}, {0, 0}},
	{"", 0, 0, 0, {1, 1}, {0, 0}},
	{"Rotor", 0, 0, 7, {330, 331}, {0, 0}},
	{"", 0, 0, 0, {1, 1}, {0, 0}}
};

const RunawayResponse kResponses[] = {
	{6290, 301, 1, 0},   // Look down into the crater.
	{6290, 303, 2, 0},   // Look at the trail back to town.
	{6290, 305, 3, 0},   // Examine the musical keyboard.
	{6290, 306, 4, 1},   // The keyboard has no power yet.
	{6290, 307, 5, 0},   // Look at the machine's switch.
	{6290, 308, 6, 1},   // The switch cannot start the unpowered machine.
	{6290, 311, 9, 0},   // Look at Joshua's tent.
	{6290, 312, 10, 0},  // Do not rummage through Joshua's belongings.
	{6290, 313, 11, 0},  // Examine the lights before using the machine.
	{6290, 314, 12, 0},  // Leave the lighting panel alone.
	{6290, 318, 15, 0},  // Leave the delivered strap for Joshua.
	{6290, 319, 16, 0},  // Examine the delivered wrench.
	{6290, 320, 15, 0},  // Leave the delivered wrench for Joshua.
	{6290, 322, 18, 0},  // Refuse to move the machine.
	{6290, 326, 23, 0},  // Do not give Joshua the gasoline.
	{6290, 327, 24, 0},  // Do not pour concentrated gasoline into the motorcycle.
	{6290, 330, 26, 0},  // Identify the rotor's purpose.
	{6290, 331, 27, 0},  // Turning the rotor by hand would not help.
	{6290, 332, 28, 0}   // Let Joshua use the wrench.
};

const RunawayItemAction kItemActions[] = {
	{6290, 10, 5, 332, false}, {6290, 10, 8, 324},
	{6290, 11, 5, 323, false}, {6290, 11, 8, 325}, {6290, 11, 13, 323, false},
	{6290, 12, 5, 327, false}, {6290, 12, 8, 326, false},
	{6290, 24, 5, 328, false}, {6290, 24, 8, 326, false}
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6290 = {
	6290, 1740, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {80, 70, 70}, {70, 60, 60}, {60, 50, 50}}, Common::Point(1739, 442),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

namespace Runaway {

Scene6290::Scene6290(RunawayAudio &audio, RunawayInventory &inventory, const RunawayStrings &strings, RunawayGameType game) :
	RunawayRoomScene(audio, inventory, strings, game) {}

bool Scene6290::loadCustomResources() {
	return _joshuaAnimation.load(_archive, 10, 131) && _machineAnimation.load(_archive, 5, 52) &&
		_machineAmbientAnimation.load(_archive, 7, 6) && _machineOverlayAnimation.load(_archive, 8, 31) && _machineIndicator.load(_archive, 12, 2) &&
		loadForeground(0, 4) && loadForeground(1, 11);
}

bool Scene6290::applyCustomBackgroundPatches() {
	return (_town.strapDelivered != 1 || applyPatch(13)) && (_town.wrenchDelivered != 1 || applyPatch(14));
}

void Scene6290::prepareCustomGameplayLoop() {
	_joshuaTick = _now;
}

void Scene6290::resumeAmbientAudio(bool entering) {
	if (entering || !_audio.musicPlaying()) {
		_audio.playMusic(19);
	}
}

void Scene6290::runCustomEntrySequence() {
	walkTo(Common::Point(1363, 484), 5);
	if (!_town.joshuaMet) {
		deferTownAction(400);
	} else if ((_town.strapDelivered == 1 && !_town.wrenchNeeded) || _town.strapDelivered == 2) {
		deferTownAction(420);
	}
}

void Scene6290::advanceCustomGameplayLoop(uint32 delta) {
	if (drawingRoomAnimation() || _now - _joshuaTick < 80) {
		return;
	}
	_joshuaTick = _now;
	if (_joshuaTalking) {
		if (_joshuaFrame > 6 || speaker()) {
			_joshuaFrame = 6;
		} else if (_joshuaFrame < 6) {
			++_joshuaFrame;
		}
	} else if (_joshuaFrame > 11) {
		if (_joshuaFrame == 99 || _joshuaFrame == 130) {
			_joshuaFrame = 0;
		} else {
			++_joshuaFrame;
		}
	} else if (_joshuaFrame) {
		--_joshuaFrame;
	} else if (!_random.getRandomNumber(49)) {
		_joshuaFrame = _random.getRandomBit() ? 12 : 100;
	}
}

void Scene6290::applyCustomHotspotState() {
	for (uint i = 0; i < 256; ++i) {
		if (_maps[i] == 3) {
			_maps[i] = 0; // Joshua occupies the approach to the machine.
		}
		switch (_originalMaps[256 + i]) {
		case 1: // The crater floor becomes accessible after Joshua's departure.
			_maps[256 + i] = 0;
			break;
		case 3: // The controls are part of the machine until it is powered.
		case 4:
		case 14:
			_maps[256 + i] = 11;
			break;
		case 9:
			_maps[256 + i] = _town.strapDelivered == 1 ? 9 : 5;
			break;
		case 10:
			_maps[256 + i] = _town.wrenchDelivered == 1 ? 10 : 0;
			break;
		case 12:
			_maps[256 + i] = 8;
			break;
		case 13:
			_maps[256 + i] = _town.rotorKnown ? 13 : 11;
			break;
		default:
			break;
		}
	}
}

float Scene6290::actorHeight() const {
	if (_position.x < 1713 && _position.y < 356) {
		return _position.y * 1.75757575f - 259.69696f;
	}
	return (_position.x * -0.0231607631f + 425.276581f +
		_position.y * 0.440251559f + 191.289307f) / 2;
}

void Scene6290::findWalkPath(Common::Point target) {
	walkTown(target);
}

Common::U32String Scene6290::hotspotName(byte id) const {
	if (id == 11 && _town.machineKnown) {
		return Common::U32String("Communication machine");
	}
	return RunawayRoomScene::hotspotName(id);
}

void Scene6290::drawCustomForegroundComposite(Graphics::ManagedSurface &viewport) {
	if (_position.x > 1380) {
		drawForeground(viewport, 0);
	}
	if (_position.y < 356) {
		drawForeground(viewport, 1);
	}
}

void Scene6290::drawCustomBackgroundComposite(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation()) {
		_joshuaAnimation.draw(viewport, _joshuaTalking && speaker() ? 6 + speechFrame() % 6 : _joshuaFrame, _camera.x, 0);
	}
	_machineAnimation.draw(viewport, _town.strapDelivered >= 2 ? 1 : 0, _camera.x, 0);
	_machineOverlayAnimation.draw(viewport, 0, _camera.x, 0);
	_machineAmbientAnimation.draw(viewport, 0, _camera.x, 0);
	_machineIndicator.draw(viewport, 0, _camera.x, 0);
}

RunawayRoomScene::ActionResult Scene6290::dispatchCustomSceneAction(uint16 action) {
	switch (action) {
	case 304: // Return to the overview; Joshua works while Brian is away.
		if (_town.strapDelivered == 1 && _town.wrenchDelivered == 1) {
			_town.strapDelivered = 2;
			_town.wrenchDelivered = 2;
		}
		_town.campLeft = 1;
		_audio.playMusic(12 + _random.getRandomNumber(4));
		leaveTownRoom(6350);
		break;
	case 309: // Examine the motorcycle before or after the repair.
		say(7, _town.fuelNeeded);
		break;
	case 310: // Try to start the motorcycle without fuel.
		say(8, _town.fuelNeeded);
		break;
	case 315: // Look at Joshua.
		say(13, _random.getRandomNumber(2));
		break;
	case 316: // Talk to Joshua, or give him the wrench he requested.
		if (_town.strapDelivered == 1 && (!_town.wrenchNeeded || _town.wrenchDelivered)) {
			say(22);
		} else if (_town.strapDelivered == 1 && _town.wrenchNeeded && _inventory.contains(10)) {
			dispatchCustomSceneAction(324);
		} else {
			startJoshuaConversation(false);
		}
		break;
	case 317: // Look at the strap waiting to be fitted.
		say(14, _town.wrenchNeeded && !_inventory.contains(10) && !_town.wrenchDelivered);
		break;
	case 321: // Name the machine after discussing it with Joshua.
		say(17, _town.machineKnown);
		break;
	case 323: // Work out how the strap could drive the rotor.
		if (!_town.rotorKnown) {
			say(19);
		} else if (_inventory.town().strap != 2) {
			say(19, _town.strapIdea ? 2 : 1);
			_town.strapIdea = 1;
			_inventory.town().strap = 1;
		} else {
			say(19, 3);
			deferTownAction(423);
		}
		break;
	case 423: // Brian decides to leave the assembly to Joshua.
		_facing = 4;
		say(19, 4);
		break;
	case 324: // Give Joshua the number ten wrench.
		_inventory.select(0);
		_joshuaTalking = true;
		say(20);
		say(20, 1, false, 1);
		say(20, 2);
		say(20, 3, false, 1);
		say(20, 4);
		deferTownAction(424);
		break;
	case 424:
		_joshuaTalking = false;
		walkTo(Common::Point(1133, 434), 7, false, true);
		deferTownAction(425);
		break;
	case 425: // Put the wrench beside the strap.
		_inventory.remove(10);
		_town.wrenchDelivered = 1;
		walkTo(Common::Point(512, 568), 5, false, true);
		deferTownAction(426);
		return kBackgroundChanged;
	case 426:
		say(20, 5);
		break;
	case 325: // Explain the drive belt idea and give Joshua the joined strap.
		if (!_town.rotorKnown) {
			say(25, 0, true);
		} else if (_inventory.town().strap != 2) {
			say(19, 2);
			_inventory.town().strap = 1;
		} else {
			_joshuaTalking = true;
			say(20);
			say(20, 1, false, 1);
			for (uint i = 0; i <= 6; ++i) {
				say(21, i, false, i % 2);
			}
			deferTownAction(430);
		}
		break;
	case 430:
		_joshuaTalking = false;
		walkTo(Common::Point(1133, 434), 7, false, true);
		deferTownAction(431);
		break;
	case 431: // Put the strap beside the motorcycle.
		_inventory.select(0);
		_inventory.remove(11);
		_town.strapDelivered = 1;
		walkTo(Common::Point(512, 568), 5, false, true);
		deferTownAction(432);
		return kBackgroundChanged;
	case 432:
		say(21, 7);
		break;
	case 328: // Only correctly diluted gasoline can run the motorcycle.
		if (!_town.bottleContents) {
			say(19, 1, true);
		} else if (_town.bottleContents == 1) {
			say(25);
		} else {
			return kUnhandled; // Starting the powered machine is not implemented yet.
		}
		break;
	case 400: // Call to Joshua from the trail.
		say(0);
		deferTownAction(401);
		break;
	case 401:
		walkTo(Common::Point(440, 556), 7, false, true);
		deferTownAction(402);
		break;
	case 402:
		say(0, 1);
		deferTownAction(403);
		break;
	case 403: // Joshua finishes receiving a telepathic message.
		beginRoomAnimationRange(403, 10, 131, 12, 99, false);
		break;
	case 1403:
		beginRoomAnimationRange(404, 10, 131, 0, 6, false);
		break;
	case 1404:
		_joshuaFrame = 6;
		_joshuaTalking = true;
		say(0, 4, false, 1);
		say(0, 5);
		deferTownAction(405);
		break;
	case 405:
		beginRoomAnimationRange(405, 10, 131, 100, 130, false);
		break;
	case 1405: // Joshua introduces Trantor and the communication machine.
		for (uint i = 6; i <= 18; ++i) {
			say(0, i, false, !(i % 2));
		}
		_town.joshuaMet = 1;
		deferTownAction(406);
		break;
	case 406:
		startJoshuaConversation(true);
		break;
	case 410: // Joshua resumes his idle animation after a conversation.
		_joshuaTalking = false;
		break;
	case 420: // On returning, discover Joshua needs a wrench or fuel.
		say(0, _town.strapDelivered == 1 ? 19 : 26);
		deferTownAction(421);
		break;
	case 421:
		walkTo(Common::Point(440, 556), 7, false, true);
		deferTownAction(422);
		break;
	case 422:
		_joshuaTalking = true;
		say(20);
		say(20, 1, false, 1);
		if (_town.strapDelivered == 1) {
			_town.wrenchNeeded = 1;
			say(0, 20);
			say(0, 21, false, 1);
			say(0, 22);
			deferTownAction(427);
		} else {
			_town.strapDelivered = 3;
			_town.fuelNeeded = 1;
			say(0, 27);
			say(0, 28, false, 1);
			say(0, 29);
			say(0, 30, false, 1);
			deferTownAction(410);
		}
		break;
	case 427:
		_facing = 4;
		say(0, 23);
		deferTownAction(428);
		break;
	case 428:
		_facing = 7;
		say(0, 24);
		say(0, 25, false, 1);
		deferTownAction(410);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void Scene6290::startJoshuaConversation(bool introduction) {
	_conversation = 7;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	_afterConversation = 410;
	_joshuaTalking = true;
	if (!introduction) {
		say(20);
		say(20, 1, false, 1);
	}

	static const DialogueChoice topics[][8] = {
		{
			{1, 0, 3, 0, 0, 1, 255}, {1, 0, 3, 1, 1, 1, 255},
			{0, 0, 1, 15, 15, 1, 255}, {0, 0, 1, 16, 15, 1, 255},
			{0, 1, 3, 6, 6, 5, 255}, {0, 0, 3, 7, 7, 1, 255},
			{0, 0, 0, 8, 8, 11, 255}, {1, 0, 0, 9, 9, 0, 0}
		},
		{
			{0, 0, 3, 2, 2, 6, 255}, {0, 0, 3, 3, 3, 1, 255},
			{0, 0, 1, 4, 4, 7, 255}, {0, 0, 1, 5, 5, 1, 255},
			{1, 0, 2, 17, 17, 0, 0}
		},
		{
			{0, 0, 3, 10, 10, 15, 255}, {0, 0, 3, 11, 11, 8, 255},
			{1, 0, 3, 12, 12, 1, 255}, {1, 0, 3, 13, 13, 1, 255},
			{1, 0, 2, 14, 14, 0, 0}
		}
	};
	for (uint i = 0; i < ARRAYSIZE(topics); ++i) {
		memcpy(_dialogue[i * 10], topics[i], sizeof(topics[i]));
	}
	_dialogue[0][2].enabled = !_town.campLeft;
	_dialogue[0][3].enabled = _town.campLeft;
	_dialogue[0][4].enabled = !_town.helmetKnown;
	_dialogue[0][5].enabled = _town.helmetKnown;
	_dialogue[10][0].enabled = !_town.dimensionalDoorKnown;
	_dialogue[10][1].enabled = _town.dimensionalDoorKnown;
	_dialogue[10][2].enabled = !_town.machineKnown;
	_dialogue[10][3].enabled = _town.machineKnown;
	_dialogue[20][0].enabled = !_town.batteryKnown;
	_dialogue[20][1].enabled = _town.batteryKnown;
}

void Scene6290::handleCustomDialogueChoice(byte effect) {
	switch (effect) {
	case 5: // Joshua explains his telepathic helmet.
		say(90, 7, false, 1);
		_town.helmetKnown = 1;
		break;
	case 6:
		_town.dimensionalDoorKnown = 1;
		break;
	case 7:
		_town.machineKnown = 1;
		break;
	case 8: // Brian can now suggest driving the rotor with the motorcycle.
		_town.rotorKnown = 1;
		updateMaps();
		break;
	case 15: // Asking about the broken battery reveals the request for help.
		_town.batteryKnown = 1;
		_dialogue[20][1].enabled = 1;
		break;
	default:
		break;
	}
}

} // End of namespace Runaway
} // End of namespace Hollywood
