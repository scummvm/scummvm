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

const RunawayRoom kRunawayScene6370 = {
	6370, 1444, 800, nullptr, 0,
	{{100, 100, 100}, {80, 80, 80}, {70, 70, 70}, {50, 50, 50}, {60, 60, 60}, {50, 50, 50}, {40, 40, 40}},
	Common::Point(1443, 648), nullptr, 0, nullptr, 0, nullptr, 0
};

void RunawayRoomScene::start6370() {
	_willyPose = 0;
	walkTo(Common::Point(1292, 689), 7, true);
	deferTownAction(401);
}

RunawayRoomScene::ActionResult RunawayRoomScene::execute6370(uint16 action) {
	switch (action) {
	case 401: // Sushi turns away from her computer.
		beginRoomAnimationRange(401, 4, 88, 0, 63, false);
		break;
	case 1401: // Introduce Brian or greet him on a return visit.
		_willyPose = 63;
		if (!_town.sushiMet) {
			say(90, 0, false, 1);
			say(80, 1);
			say(90, 1, false, 1);
			startSushiConversation(true);
		} else {
			say(80, 11);
			say(90, 11, false, 1);
			say(80, 12);
			say(90, 12, false, 1);
			startSushiConversation(false);
		}
		break;
	case 402: // Explain the connection between Douglasville and the Hopi village.
		for (uint i = 5; i <= 10; ++i) {
			say(80, i);
			say(90, i, false, 1);
		}
		startSushiConversation(false);
		break;
	case 403: // Finish the conversation and return to the street.
		_town.sushiMet = 1;
		_audio.playMusic(20);
		_willyPose = 79;
		_ambientTick = _now;
		walkTo(Common::Point(1443, 648), 1, true);
		leaveTownRoom(6222);
		break;
	default:
		return kUnhandled;
	}
	return kHandled;
}

void RunawayRoomScene::drawScene6370(Graphics::ManagedSurface &viewport) {
	_ambient.draw(viewport, (_now / 80) % 20, _camera.x, -_camera.y);
	if (!drawingRoomAnimation()) {
		uint frame = _willyPose;
		if (_willyPose == 79) {
			frame += MIN<uint>(8, (_now - _ambientTick) / 80);
		} else if (_willyPose == 63 && speaker()) {
			frame += speechFrame();
		}
		_npc.draw(viewport, frame, _camera.x, -_camera.y);
	}
}

void RunawayRoomScene::startSushiConversation(bool introduction) {
	_conversation = 5;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	_choiceIndices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	_afterConversation = introduction ? 402 : 403;
	if (introduction) {
		// Original page 40: Brian chooses how to explain his interest in the Hopi.
		for (uint i = 0; i < 3; ++i) {
			_dialogue[0][i] = {1, 0, 0, (byte)(i + 2), (byte)(i + 2), 0, 255};
		}
		return;
	}
	static const DialogueChoice topics[][8] = {
		{{0, 0, 1, 13, 13, 1, 255}, {0, 0, 1, 14, 14, 1, 255},
		 {1, 0, 3, 15, 15, 2, 255}, {1, 1, 1, 17, 17, 1, 255},
		 {0, 2, 1, 18, 18, 1, 255}, {0, 3, 1, 19, 19, 10, 255},
		 {1, 0, 0, 20, 20, 0, 0}},
		{{1, 1, 3, 21, 21, 1, 255}, {1, 1, 3, 22, 22, 3, 255},
		 {1, 1, 3, 23, 23, 1, 255}, {1, 0, 2, 24, 24, 0, 0}},
		{{1, 0, 2, 25, 25, 1, 255}}
	};
	memcpy(_dialogue[0], topics[0], sizeof(topics[0]));
	memcpy(_dialogue[10], topics[1], sizeof(topics[1]));
	memcpy(_dialogue[11], topics[2], sizeof(topics[2]));
	_dialogue[0][0].enabled = !_town.sushiMet;
	_dialogue[0][1].enabled = _town.sushiMet;
	// The safe and the prisoner become topics after visiting those rooms.
}

void RunawayRoomScene::sushiDialogueChoice(byte effect) {
	if (effect == 2) { // Ask Sushi to repeat Douglasville's history.
		say(90, 6, false, 1);
		say(80, 7);
		say(90, 7, false, 1);
		say(80, 8);
		say(90, 8, false, 1);
	} else if (effect == 3) { // Learn where a map of the mine might be kept.
		_town.mineMapKnown = 1;
	}
}

} // End of namespace Hollywood
