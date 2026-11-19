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
#include "hollywood/runaway/scene.h"

namespace Hollywood {
namespace {

const RunawayHotspot kHotspots[] = {
	{"", 0, 0, 0, {0, 0}, {0, 0}},
	{"Oil well area", 910, 599, 3, {301, 302}, {0, 3}},
	{"Entrance to the house", 686, 510, 0, {303, 304}, {0, 0}},
	{"Sign", 846, 509, 0, {305, 306}, {0, 0}},
	{"Big dude", 676, 510, 7, {307, 308}, {0, 1}},
	{"Ladder", 244, 416, 7, {309, 310}, {0, 0}},
	{"Vessel", 180, 446, 0, {312, 313}, {0, 1}},
	{"Path", 50, 599, 5, {315, 316}, {0, 3}}
};

const RunawayResponse kResponses[] = {
	{6090, 301, 13, 0}, // Look toward the abandoned oil well.
	{6090, 303, 2, 0},  // Examine the curtain across the doorway.
	{6090, 305, 4, 0},  // Read Mama Dorita's sign.
	{6090, 309, 7, 0},  // Examine the ladder.
	{6090, 315, 1, 0}   // Look along the deserted path.
};

const RunawayItemAction kItemActions[] = {
	{6090, 27, 4, 314}, // Offer untreated tobacco leaves.
	{6090, 28, 4, 311}  // Give Oscar chewing tobacco.
};

// Oscar uses the same idle sequence in the wide view and at the doorway.
const byte kOscarFrames[] = {
	106, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
	11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,
	106, 106, 106, 106, 106, 106, 106, 106,
	42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60,
	53, 54, 55, 56, 57, 58, 59, 60, 53, 54, 55, 56, 57, 58, 59, 60,
	61, 62, 63, 64, 65, 66, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42,
	106, 106, 106, 106, 106, 106, 106, 106, 106,
	71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106
};

} // End of anonymous namespace

const RunawayRoom kRunawayScene6090 = {
	6090, 1024, 600, kHotspots, ARRAYSIZE(kHotspots),
	{{100, 100, 100}, {100, 95, 90}, {80, 75, 70}}, Common::Point(838, 551),
	kResponses, ARRAYSIZE(kResponses), kItemActions, ARRAYSIZE(kItemActions), nullptr, 0
};

void RunawayRoomScene::start6090() {
	if (!_town.oscarMet) {
		walkTo(Common::Point(676, 510), 7);
		deferTownAction(400);
	} else {
		walkTo(Common::Point(837, 550), 7);
	}
}

void RunawayRoomScene::updateOscar() {
	if (_oscarTalking) {
		_oscarTick = _now;
		return;
	}
	if (_now - _oscarTick < 80) {
		return;
	}
	_oscarTick = _now;
	const uint last = _oscarIdle == 1 ? 62 : _oscarIdle == 2 ? 121 : _room == 6090 ? 165 : 164;
	if (_oscarStep < last) {
		++_oscarStep;
		if (_room == 6090 && _oscarIdle == 2 && _oscarStep == 80) {
			_audio.playEffect(80, 26, 1);
		}
	} else if (_oscarIdle == 3 && !_random.getRandomNumber(_room == 6090 ? 4 : 9)) {
		_oscarIdle = 1 + _random.getRandomBit();
		_oscarStep = _oscarIdle == 1 ? 0 : 70;
	} else {
		_oscarIdle = 3;
		_oscarStep = 130;
	}
	if (_room == 6090 && _oscarIdle == 3 && !_audio.effectPlaying(1)) {
		_audio.playEffect(76, 153, 1, true);
	}
}

uint RunawayRoomScene::oscarFrame() const {
	if (_oscarTalking) {
		return 107 + (speaker() ? speechFrame() % 6 : 0);
	}
	const uint frame = kOscarFrames[_oscarStep];
	return _room == 6130 && frame == 106 ? 105 : frame;
}

void RunawayRoomScene::drawScene6090(Graphics::ManagedSurface &viewport, bool foreground) {
	if ((_position.y < 460) == foreground && !drawingRoomAnimation()) {
		_npc.draw(viewport, oscarFrame(), _camera.x, -_camera.y);
	}
	if (foreground) {
		drawForeground(viewport, 0);
	}
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6090(uint16 action) {
	switch (action) {
	case 302: // Return to the wide view of the house.
		leaveTownRoom(6131);
		break;
	case 304: // Oscar will not allow Brian into Mama Dorita's house yet.
		_facing = 4;
		if (_town.oscarTobacco == 1) {
			say(3, _random.getRandomBit());
		} else {
			say(11, 0, true);
		}
		break;
	case 306: // Refuse to remove the sign.
		if (_town.oscarTobacco == 1) {
			say(5);
		} else {
			say(14, _random.getRandomNumber(2), true);
		}
		break;
	case 307: { // Look at Oscar.
		const byte variation = _random.getRandomNumber(2);
		if (variation == 2) {
			_facing = 4;
		}
		say(6, variation);
		break;
	}
	case 308: // Talk to Oscar.
		startOscarConversation();
		break;
	case 310: // Refuse to sneak into the house via the ladder.
		if (_town.oscarTobacco == 1) {
			say(8);
		} else {
			say(14, _random.getRandomNumber(2), true);
		}
		break;
	case 311: // Offer the prepared chewing tobacco.
		_oscarTalking = true;
		_audio.stopEffect(1);
		say(9);
		say(9, 1, false, 1);
		say(9, 2);
		deferTownAction(411);
		break;
	case 411: { // Oscar and Brian exchange the tobacco.
		byte oscar[35], brian[35];
		for (uint i = 0; i < ARRAYSIZE(oscar); ++i) {
			oscar[i] = i < 6 || i == 34 ? 0 : i - 5;
			brian[i] = i ? MIN<uint>(23, i - 1) : 0;
		}
		beginRoomAnimation(411, 7, 29, oscar, ARRAYSIZE(oscar));
		setRoomAnimationOverlay(8, 24, brian, ARRAYSIZE(brian));
		break;
	}
	case 1411:
		_inventory.remove(28);
		_inventory.select(0);
		_town.oscarTobacco = 2;
		say(9, 3, false, 1);
		say(9, 4);
		say(9, 5, false, 1);
		deferTownAction(412);
		break;
	case 412:
		_oscarTalking = false;
		walkTo(Common::Point(718, 543), 4);
		say(9, 6);
		break;
	case 312: // Examine the clay vessel.
		say(10);
		_town.vessel = 1;
		break;
	case 313: // Take the vessel while Oscar is occupied.
		if (!_town.vessel) {
			execute6090(312);
		}
		if (_town.oscarTobacco == 1) {
			say(11);
		} else {
			say(18, 0, true);
		}
		deferTownAction(413);
		break;
	case 413:
		_town.vessel = 2;
		_inventory.add(31);
		say(11, 1);
		return kBackgroundChanged;
	case 314: // Oscar chews tobacco; he does not smoke the untreated leaves.
		_oscarTalking = true;
		_audio.stopEffect(1);
		say(12);
		say(12, 1, false, 1);
		_town.rawTobaccoRefused = 1;
		deferTownAction(401);
		break;
	case 316: // Return directly to the overview.
		leaveTownRoom(6350);
		break;
	case 400: // First meeting at Mama Dorita's door.
		_oscarTalking = true;
		_audio.stopEffect(1);
		for (uint i = 0; i <= 14; ++i) {
			say(0, i, false, (i % 2) == 0);
		}
		_town.oscarMet = 1;
		deferTownAction(401);
		break;
	case 401: // Oscar returns to his idle animation.
		_oscarTalking = false;
		_oscarIdle = 3;
		_oscarStep = 130;
		break;
	case 402: // Oscar has agreed to move the mine's rock.
		leaveTownRoom(6082);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::startOscarConversation() {
	_conversation = 6;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	_afterConversation = 401;
	_oscarTalking = true;
	_audio.stopEffect(1);
	say(81, _town.oscarTobacco == 1 ? 0 : 8);
	say(91, 0, false, 1);

	// Original page 81: the favour becomes available after Kevin names Oscar.
	static const DialogueChoice topics[] = {
		{1, 0, 3, 1, 1, 1, 255}, {1, 0, 3, 2, 2, 1, 255},
		{1, 0, 3, 3, 3, 1, 255}, {0, 0, 1, 5, 5, 2, 255},
		{1, 0, 0, 4, 4, 1, 0}
	};
	memcpy(_dialogue[0], topics, sizeof(topics));
	_dialogue[0][1].enabled = _town.oscarTobacco < 2;
	_dialogue[0][3].enabled = _town.oscarTobacco && !_town.rockMoved;
	_dialogue[10][0] = {1, 0, 2, 6, (byte)(_town.oscarHelpRefused ? 7 : 6), 3, 255};
	if (_town.oscarTobacco == 2) {
		_dialogue[10][0].transition = 0;
		_dialogue[10][0].response = 8;
		_dialogue[10][0].disable = 5;
	}
}

void RunawayRoomScene::oscarDialogueChoice(byte effect) {
	if (effect == 3) { // Remember that Oscar has already refused the favour.
		_town.oscarHelpRefused = 1;
	} else if (effect == 5) { // A gift of chewing tobacco earns his help.
		say(81, 7);
		_afterConversation = 402;
	}
}

} // End of namespace Hollywood
