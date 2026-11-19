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

#include "common/scummsys.h"
#include "common/str-enc.h"
#include "common/util.h"
#include "hollywood/runaway/scene.h"
#include "hollywood/runaway/scene_data.h"

namespace Hollywood {

const RunawayRoom kRunawayScene4070 = {
	kMuseumWilly, 1024, 600, nullptr, 0, {{100, 100, 100}}, Common::Point(548, 580),
	nullptr, 0, nullptr, 0, nullptr, 0
};

bool RunawayRoomScene::loadScene4070() {
	return _npc.load(_archive, 4, 188);
}

void RunawayRoomScene::drawScene4070(Graphics::ManagedSurface &viewport) {
	if (!drawingRoomAnimation() || (_actionHidesActor && _secondaryFrames.empty()))
		_npc.draw(viewport, _willyPose + (speaker() ? speechFrame() : 0), 0, 0);
}

void RunawayRoomScene::start4070() {
	_facing = 7;
	_audio.playEffect(57, 102, 1, true);
	if (_entryScene == 4071) {
		say(32);
		say(33, 3, false, 1);
		deferMuseumAction(417);
	} else if (_entryScene == 4072) {
		say(32);
		if (_inventory.museum().coffee == 0) {
			say(33, 0, false, 1);
			_museum.needsGroundCoffee = 1;
		} else {
			say(33, 1, false, 1);
			say(32, 1);
			say(33, 2, false, 1);
			_museum.needsPackage = 1;
		}
		deferMuseumAction(409);
	} else {
		startConversation(2);
	}
}

void RunawayRoomScene::finish4070(uint16 action) {
	switch (action) {
	case 417:
		beginRoomAnimationRange(427, 5, 53, 19, 33);
		break;
	case 427: {
		byte willy[31], brian[31];
		for (uint i = 0; i < ARRAYSIZE(willy); ++i) {
			willy[i] = !i || i == 30 ? 54 : 158 + i;
			brian[i] = i < 10 ? 33 : i < 19 ? 24 + i : 18;
		}
		beginRoomAnimation(407, 5, 53, brian, ARRAYSIZE(brian));
		setRoomAnimationOverlay(4, 188, willy, ARRAYSIZE(willy));
		break;
	}
	case 407:
		_inventory.remove(19);
		say(33, 4, false, 1);
		if (_museum.batteryRequested) {
			say(33, 5, false, 1);
			say(36, 1);
			say(33, 6, false, 1);
		}
		say(33, 7, false, 1);
		_museum.coffeeMachine = 1;
		_museum.willyInLab = 2;
		deferMuseumAction(448);
		break;
	case 408: {
		byte frames[32];
		frames[0] = 54;
		for (uint i = 1; i < ARRAYSIZE(frames); ++i)
			frames[i] = 74 + i;
		_audio.playEffect(15);
		beginRoomAnimation(418, 4, 188, frames, ARRAYSIZE(frames), false);
		break;
	}
	case 418:
		_willyPose = 105;
		for (uint i = 0; i < 6; ++i)
			say(91, i, false, 1);
		deferMuseumAction(428);
		break;
	case 428: {
		byte frames[32];
		for (uint i = 0; i < ARRAYSIZE(frames) - 1; ++i)
			frames[i] = 105 - i;
		frames[31] = 54;
		beginRoomAnimation(438, 4, 188, frames, ARRAYSIZE(frames), false);
		break;
	}
	case 438:
		_willyPose = 54;
		say(91, 6, false, 1);
		say(81);
		_museum.willyMet = 2;
		_museum.willyInLab = 1;
		_museum.sawCode = 1;
		deferMuseumAction(448);
		break;
	case 448:
		beginRoomAnimation(409, 6, 72, nullptr, 0, false);
		break;
	case 409:
		_upperFloor = false;
		leaveMuseumRoom(4064, Common::Point(921, 813), 7);
		break;
	}
}

void RunawayRoomScene::startConversation(byte character) {
	_conversation = character;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
	// Eight choices per node, with the original forward/back transitions.
	static const DialogueChoice susan[][8] = {
		{{1,0,3,3,3,1,255}, {1,0,1,4,4,1,255}, {1,1,1,5,5,1,255}, {1,0,3,6,6,1,255}, {1,0,0,7,7,1,0}},
		{{1,0,3,8,8,2,0}, {1,0,3,9,9,2,0}, {1,0,3,10,10,2,0}},
		{{1,1,3,11,11,2,0}, {1,1,3,12,12,2,0}, {1,1,3,13,13,2,0}},
		{{1,0,2,14,14,1,255}}
	};
	static const DialogueChoice willy[][8] = {
		{{0,0,1,6,5,1,255}, {0,1,1,7,6,5,255}, {0,2,1,8,7,1,255}, {0,0,3,9,8,1,255},
		 {0,0,3,10,9,1,255}, {0,3,1,11,10,1,255}, {1,0,0,12,11,0,0}},
		{{1,0,3,13,12,10,255}, {1,0,3,14,13,10,255}, {1,0,3,15,14,10,255}, {1,0,2,16,15,0,0}},
		{{1,0,3,17,16,1,255}, {1,0,1,18,17,1,255}, {1,0,2,19,18,0,0}},
		{{1,0,3,20,19,1,255}, {1,0,3,21,20,1,255}, {1,0,2,22,21,0,0}},
		{{1,0,3,23,22,2,0}, {1,0,3,24,23,2,0}},
		{{1,1,2,25,24,1,255}}
	};
	if (character == 1) {
		memcpy(_dialogue[0], susan[0], sizeof(susan[0]));
		memcpy(_dialogue[10], susan[1], sizeof(susan[1]));
		memcpy(_dialogue[11], susan[2], sizeof(susan[2]));
		memcpy(_dialogue[20], susan[3], sizeof(susan[3]));
		say(80, 15);
		say(90, 15, false, 1);
	} else if (character == 2) {
		memcpy(_dialogue[0], willy[0], sizeof(willy[0]));
		for (uint i = 1; i <= 4; ++i)
			memcpy(_dialogue[9 + i], willy[i], sizeof(willy[i]));
		memcpy(_dialogue[20], willy[5], sizeof(willy[5]));
		_dialogue[0][1].enabled = _inventory.museum().voiceRecorded && _inventory.contains(6) && !_inventory.museum().frozenBattery;
		_dialogue[0][2].enabled = _museum.coffeeSeen && !_museum.coffeeMachine;
		_dialogue[0][5].enabled = _museum.willyMet == 2 && !_museum.labVisited;
		const uint count = 1 + _dialogue[0][1].enabled + _dialogue[0][2].enabled + _dialogue[0][5].enabled;
		_dialogue[0][0].enabled = count <= 4;
		_dialogue[0][3].enabled = count <= 3;
		_dialogue[0][4].enabled = count <= 2;
		if (!_museum.willyMet) {
			_museum.willyMet = 1;
			for (uint i = 0; i < 4; ++i) {
				say(80, i);
				say(90, i, false, 1);
			}
			_dialogueIntro = 1;
		} else {
			say(80, 5);
			say(90, 4, false, 1);
		}
	} else {
		for (uint i = 0; i < 5; ++i)
			_dialogue[0][i] = {1, 0, 0, (byte)i, 1, 0, 0};
		_dialogue[0][3].enabled = _inventory.museum().businessCardExamined;
		_museum.voiceLockSeen = 1;
		say(90, 0, false, 1);
	}
}

void RunawayRoomScene::buildDialogueChoices() {
	_choices.clear();
	_choiceIndices.clear();
	const uint page = _dialogueDepth * 10 + _dialogueNode;
	if (page >= ARRAYSIZE(_dialogue)) {
		finishConversation();
		return;
	}
	for (uint i = 0; i < 8; ++i) {
		if (!_dialogue[page][i].enabled)
			continue;
		const SceneSpeechCue cue = _text.stageCue(80, _dialogue[page][i].player);
		const Common::U32String text(_text.largeTextRecord(cue.textRecordId), Common::kDos850);
		if (!text.empty()) {
			_choices.push_back(text);
			_choiceIndices.push_back(i);
		}
	}
	if (_choices.empty()) {
		if (_dialogueDepth) {
			--_dialogueDepth;
			_dialogueNode = 0;
			buildDialogueChoices();
		} else {
			finishConversation();
		}
	}
}

void RunawayRoomScene::chooseDialogue(uint choice) {
	if (choice >= _choiceIndices.size() || speaking())
		return;
	const byte index = _choiceIndices[choice];
	const uint page = _dialogueDepth * 10 + _dialogueNode;
	DialogueChoice &entry = _dialogue[page][index];
	_choices.clear();
	_choiceIndices.clear();
	say(80, entry.player);
	if (_conversation == 2 && _museum.willyMet == 1) {
		_dialogueEnding = true;
		_afterConversation = 408;
		return;
	}
	if (entry.response != 255)
		say(90, entry.response, false, 1);
	if (_conversation == 3)
		_computerReject = true;
	if (entry.disable)
		entry.enabled = 0;
	if (_conversation == 2 && entry.disable == 5)
		_museum.batteryRequested = 1;
	if (_conversation == 4 && entry.disable == 2)
		_desert.ventSeen = 1;
	if (_conversation == 2 && entry.disable == 10) {
		_dialogue[10][3].player = 22;
		_dialogue[10][3].response = 21;
	}
	switch (entry.transition) {
	case 0: _dialogueEnding = true; break;
	case 1: ++_dialogueDepth; _dialogueNode = entry.node; break;
	case 2: _dialogueDepth -= MIN<byte>(_dialogueDepth, 1); _dialogueNode = entry.node; break;
	case 4: _dialogueDepth -= MIN<byte>(_dialogueDepth, 2); _dialogueNode = entry.node; break;
	default: break;
	}
}

void RunawayRoomScene::updateConversation() {
	if (!_conversation || _roomAction || speaking() || !_choices.empty())
		return;
	if (_dialogueIntro) {
		_dialogueIntro = 0;
		_inventory.add(13);
		_inventory.museum().businessCardExamined = 1;
		say(50);
		say(80, 4);
		return;
	}
	if (_dialogueEnding)
		finishConversation();
	else
		buildDialogueChoices();
}

void RunawayRoomScene::finishConversation() {
	const byte character = _conversation;
	_conversation = 0;
	_choices.clear();
	_choiceIndices.clear();
	if (_afterConversation) {
		deferMuseumAction(_afterConversation);
		_afterConversation = 0;
	} else if (character == 2) {
		deferMuseumAction(409);
	} else if (character == 3) {
		_computerReject = false;
		say(7, _museum.ownVoiceTried);
		_museum.ownVoiceTried = 1;
	}
}

} // End of namespace Hollywood
