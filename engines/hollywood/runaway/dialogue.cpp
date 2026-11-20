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

namespace Hollywood {

void RunawayRoomScene::startConversation(byte character) {
	_conversation = character;
	_dialogueDepth = _dialogueNode = _dialogueIntro = 0;
	_dialogueEnding = false;
	_choices.clear();
	memset(_dialogue, 0, sizeof(_dialogue));
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
		if (!_dialogue[page][i].enabled) {
			continue;
		}
		const SceneSpeechCue cue = _text.stageCue(dialogueCue(i, false), _dialogue[page][i].player);
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
	if (choice >= _choiceIndices.size() || speaking()) {
		return;
	}
	const byte index = _choiceIndices[choice];
	const uint page = _dialogueDepth * 10 + _dialogueNode;
	DialogueChoice &entry = _dialogue[page][index];
	_choices.clear();
	_choiceIndices.clear();
	say(dialogueCue(index, false), entry.player);
	if (handleCustomDialogueReply()) {
		return;
	}
	if (entry.response != 255) {
		say(dialogueCue(index, true), entry.response, false, 1);
	}
	if (entry.disable) {
		entry.enabled = 0;
	}
	handleCustomDialogueChoice(entry.disable);
	switch (entry.transition) {
	case 0: // End the conversation.
		_dialogueEnding = true;
		break;
	case 1: // Enter a topic.
		++_dialogueDepth;
		_dialogueNode = entry.node;
		break;
	case 2: // Return to the parent topic.
		_dialogueDepth -= MIN<byte>(_dialogueDepth, 1);
		_dialogueNode = entry.node;
		break;
	case 4: // Return two topic levels.
		_dialogueDepth -= MIN<byte>(_dialogueDepth, 2);
		_dialogueNode = entry.node;
		break;
	default:
		break;
	}
}

void RunawayRoomScene::updateConversation() {
	if (!_conversation || _roomAction || speaking() || !_choices.empty()) {
		return;
	}
	if (_dialogueIntro) {
		_dialogueIntro = 0;
		runCustomDialogueIntro();
		return;
	}
	if (_dialogueEnding) {
		finishConversation();
	} else {
		buildDialogueChoices();
	}
}

void RunawayRoomScene::finishConversation() {
	_conversation = 0;
	_choices.clear();
	_choiceIndices.clear();
	if (_afterConversation) {
		if (townChapter()) {
			deferTownAction(_afterConversation);
		} else {
			deferMuseumAction(_afterConversation);
		}
		_afterConversation = 0;
	} else {
		finishCustomConversation();
	}
}

} // End of namespace Hollywood
