/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/textconsole.h"
#include "titanic/true_talk/deskbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

int DeskbotScript::_oldId;

DeskbotScript::DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	CTrueTalkManager::setFlags(18, 0);
	CTrueTalkManager::setFlags(19, 0);
	CTrueTalkManager::setFlags(20, 0);
	CTrueTalkManager::setFlags(21, 0);
	CTrueTalkManager::setFlags(22, 0);

	setupDials(0, 0, 0);
	_array[0] = 100;
	if (_currentDialNum == 1)
		_currentDialNum = 0;

	loadRanges("Ranges/Deskbot");
	loadResponses("Responses/Deskbot", 4);
	setupSentences();
	_tagMappings.load("TagMap/Deskbot");
	_words.load("Words/Deskbot");
	_quotes.load("Quotes/Deskbot");
	_states.load("States/Deskbot");
}

void DeskbotScript::setupSentences() {
	_mappings.load("Mappings/Deskbot", 4);
	_entries.load("Sentences/Deskbot");
	_dialValues[0] = _dialValues[1] = 0;
	_field68 = 0;
	_entryCount = 0;
}

int DeskbotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult DeskbotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int DeskbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		uint val, uint tagId, uint remainder) {
	switch (tagId) {
	case MKTAG('A', 'D', 'V', 'T'):
	case MKTAG('A', 'R', 'T', 'I'):
	case MKTAG('A', 'R', 'T', 'Y'):
	case MKTAG('B', 'R', 'N', 'D'):
	case MKTAG('C', 'O', 'M', 'D'):
	case MKTAG('D', 'N', 'C', 'E'):
	case MKTAG('H', 'B', 'B', 'Y'):
	case MKTAG('L', 'I', 'T', 'R'):
	case MKTAG('M', 'A', 'G', 'S'):
	case MKTAG('M', 'C', 'P', 'Y'):
	case MKTAG('M', 'I', 'N', 'S'):
	case MKTAG('M', 'U', 'S', 'I'):
	case MKTAG('N', 'I', 'K', 'E'):
	case MKTAG('S', 'F', 'S', 'F'):
	case MKTAG('S', 'O', 'A', 'P'):
	case MKTAG('S', 'O', 'N', 'G'):
	case MKTAG('S', 'P', 'R', 'T'):
	case MKTAG('T', 'E', 'A', 'M'):
	case MKTAG('T', 'V', 'S', 'H'):
		tagId = MKTAG('E', 'N', 'T', 'N');
		break;
	case MKTAG('A', 'C', 'T', 'R'):
	case MKTAG('A', 'C', 'T', 'S'):
	case MKTAG('A', 'U', 'T', 'H'):
	case MKTAG('B', 'A', 'R', 'K'):
	case MKTAG('B', 'A', 'R', 'U'):
	case MKTAG('B', 'L', 'F', '1'):
	case MKTAG('B', 'L', 'F', '2'):
	case MKTAG('B', 'L', 'P', '1'):
	case MKTAG('B', 'L', 'P', '2'):
	case MKTAG('B', 'L', 'P', '3'):
	case MKTAG('B', 'L', 'P', '4'):
	case MKTAG('B', 'L', 'R', '1'):
	case MKTAG('B', 'L', 'R', '2'):
	case MKTAG('B', 'L', 'T', '1'):
	case MKTAG('B', 'L', 'T', '2'):
	case MKTAG('B', 'L', 'T', '3'):
	case MKTAG('B', 'L', 'T', '4'):
	case MKTAG('B', 'L', 'T', '5'):
	case MKTAG('B', 'O', 'Y', 'S'):
	case MKTAG('C', 'O', 'P', 'S'):
	case MKTAG('D', 'C', 'T', 'R'):
	case MKTAG('F', 'A', 'M', 'E'):
	case MKTAG('F', 'A', 'S', 'H'):
	case MKTAG('G', 'I', 'R', 'L'):
	case MKTAG('H', 'E', 'R', 'O'):
	case MKTAG('H', 'O', 'S', 'T'):
	case MKTAG('K', 'N', 'O', 'B'):
	case MKTAG('N', 'H', 'R', 'O'):
	case MKTAG('R', 'A', 'C', 'E'):
	case MKTAG('S', 'C', 'I', 'T'):
	case MKTAG('T', 'D', 'V', 'P'):
	case MKTAG('T', 'W', 'A', 'T'):
	case MKTAG('W', 'E', 'A', 'T'):
	case MKTAG('W', 'W', 'E', 'B'):
		tagId = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
	case MKTAG('J', 'F', 'O', 'D'):
	case MKTAG('L', 'I', 'Q', 'D'):
		tagId = MKTAG('F', 'O', 'O', 'D');
		break;
	case MKTAG('C', 'R', 'I', 'M'):
	case MKTAG('C', 'S', 'P', 'Y'):
	case MKTAG('D', 'R', 'U', 'G'):
		tagId = MKTAG('V', 'B', 'A', 'D');
		break;
	case MKTAG('E', 'A', 'R', 'T'):
	case MKTAG('H', 'O', 'M', 'E'):
	case MKTAG('N', 'P', 'L', 'C'):
	case MKTAG('P', 'L', 'A', 'N'):
		tagId = MKTAG('P', 'L', 'A', 'C');
		break;
	case MKTAG('F', 'A', 'U', 'N'):
	case MKTAG('F', 'I', 'S', 'H'):
	case MKTAG('F', 'L', 'O', 'R'):
		tagId = MKTAG('N', 'A', 'T', 'R');
		break;
	case MKTAG('H', 'H', 'L', 'D'):
	case MKTAG('T', 'O', 'Y', 'S'):
	case MKTAG('W', 'E', 'A', 'P'):
		tagId = MKTAG('M', 'A', 'C', 'H');
		break;
	case MKTAG('M', 'L', 'T', 'Y'):
	case MKTAG('P', 'G', 'R', 'P'):
	case MKTAG('P', 'T', 'I', 'C'):
		tagId = MKTAG('G', 'R', 'U', 'P');
		break;
	case MKTAG('P', 'K', 'U', 'P'):
	case MKTAG('S', 'E', 'X', '1'):
	case MKTAG('S', 'W', 'E', 'R'):
		tagId = MKTAG('R', 'U', 'D', 'E');
		break;
	case MKTAG('P', 'H', 'I', 'L'):
	case MKTAG('R', 'C', 'K', 'T'):
		tagId = MKTAG('S', 'C', 'I', 'E');
		break;
	case MKTAG('T', 'R', 'A', '2'):
	case MKTAG('T', 'R', 'A', '3'):
		tagId = MKTAG('T', 'R', 'A', 'V');
		break;
	default:
		break;
	}
	
	return TTnpcScript::handleQuote(roomScript, sentence, val, tagId, remainder);

}

int DeskbotScript::updateState(uint oldId, uint newId, int index) {
	if (isDial1Medium() || getValue(1) < 4)
		CTrueTalkManager::setFlags(22, 1);

	if (newId == 240420 || newId == 240947 || newId == 241261) {
		if (getValue(22) && (newId == 240947 || newId == 241261))
			newId = getRangeValue(241184);
	}

	if (newId == 240832)
		setDialRegion(1, 0);

	if (oldId == 241183) {
		if (getValue(1) == 2)
			newId = getRangeValue(241182);
		else if (getValue(1) == 1)
			newId = getRangeValue(241181);
	}

	if (newId == 240931 && getValue(1) <= 2) {
		newId = 240924;
	} else if (newId == 240924 && getValue(1) > 2) {
		newId = 240931;
	}

	if (newId == 240830 && getValue(1) == 1) {
		newId = 240801;
	} else if (newId == 240801 && getValue(1) > 1) {
		newId = 240830;
	}

	if (oldId >= 241217 && oldId <= 241259) {
		addResponse(getDialogueId(241202));
		addResponse(getDialogueId(241200));
		newId = getRangeValue(241199);
	}

	if (newId == 241354)
		newId = addAssignedRoomDialogue2();
	if (newId == 241353)
		newId = getStateDialogueId();
	if (newId == 240464 && getValue(1) != 1)
		newId = 240462;
	
	if (newId == 241635 && isDial1Medium()) {
		addResponse(getDialogueId(241556));
		newId = getRangeValue(241632);
	}

	if (getValue(20) && (oldId == 240569 || oldId == 240576))
		newId = 240460;
	if (!getValue(20)) {
		if (newId != 240460)
			goto exit;
		CTrueTalkManager::setFlags(20, 1);
	}
	if (newId == 240460 && _oldId != 240569) {
		CTrueTalkManager::setFlags(20, 0);
		newId = 240455;
	}

exit:
	_oldId = oldId;
	setFlags17(newId, index);

	return newId;
}

int DeskbotScript::preResponse(uint id) {
	int newId = 0;
	if (getValue(1) >= 3 && (id == 41176 || id == 41738 || id == 41413 || id == 41740))
		newId = 241601;

	if (id == 42114)
		CTrueTalkManager::triggerAction(20, 0);

	return newId;
}

uint DeskbotScript::getDialsBitset() const {
	if (getDialRegion(1))
		return getDialRegion(0) ? 2 : 3;
	else
		return getDialRegion(0) ? 0 : 1;
}

int DeskbotScript::doSentenceEntry(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence) {
	uint id;

	switch (val1) {
	case 1:
		id = *srcIdP;
		if (id == 240431 || id == 240432) {
			switch (getValue(1)) {
			case 1:
				id = 240336;
				break;
			case 2:
				id = addAssignedRoomDialogue();
				break;
			case 3:
				if (getValue(3) == 1) {
					if (id == 240431)
						id = 240432;
				}
				else {
					if (id == 240432)
						id = 240431;
				}
			default:
				break;
			}

			addResponse(getDialogueId(id));
			applyResponse();
			return 2;
		}
		break;

	case 2:
		if (getValue(1) == 1)
			return true;
		break;

	default:
		break;
	}
	
	return 0;
}

int DeskbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

bool DeskbotScript::isDial0Medium() const {
	return getDialRegion(0) == 1;
}

bool DeskbotScript::isDial0Low() const {
	return getDialRegion(0) == 0;
}

bool DeskbotScript::isDial1Medium() const {
	return getDialRegion(1) == 1;
}

bool DeskbotScript::isDial1Low() const {
	return getDialRegion(1) == 0;
}

uint DeskbotScript::addAssignedRoomDialogue() {
	if (isDial1Medium()) {
		addResponse(getDialogueId(240407));
		addResponse(getDialogueId(241510));
		CTrueTalkManager::setFlags(1, 1);
		CTrueTalkManager::triggerAction(19, 1);

		int roomNum, floorNum, elevatorNum;
		getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

		addResponse(getDialogueId(241317 + roomNum));
		addResponse(getDialogueId(241271 + floorNum));
		addResponse(getDialogueId(241511));
		addResponse(getDialogueId(241313 + elevatorNum));

		return 241512;
	} else {
		return 240567;
	}
}

uint DeskbotScript::addAssignedRoomDialogue2() {
	addResponse(getDialogueId(241355));
	int roomNum = 0, floorNum = 0, elevatorNum = 0;
	getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

	addResponse(getDialogueId(241317 + roomNum));
	addResponse(getDialogueId(241271 + floorNum));
	addResponse(getDialogueId(241356));
	addResponse(getDialogueId(241313 + elevatorNum));

	return 241357;
}

void DeskbotScript::addAssignedRoomDialogue3() {
	addResponse(getDialogueId(241513));
	addResponse(getDialogueId(241510));
	
	CTrueTalkManager::setFlags(1, 2);
	setDialRegion(0, 0);
	setDialRegion(1, 0);
	CTrueTalkManager::triggerAction(19, 2);
	CTrueTalkManager::setFlags(3, 0);

	int roomNum = 1, floorNum = 1, elevatorNum = 1;
	getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

	addResponse(getDialogueId(241317 + roomNum));
	addResponse(getDialogueId(241271 + floorNum));
	addResponse(getDialogueId(241511));
	addResponse(getDialogueId(241313 + elevatorNum));
	addResponse(getDialogueId(241512));
	applyResponse();
}

uint DeskbotScript::getStateDialogueId() const {
	switch (getValue(1)) {
	case 1:
		return 241503;
	case 2:
		return 241504;
	default:
		return 241505;
	}
}

void DeskbotScript::setFlags17(uint newId, uint index) {
	int newValue = getValue(17);

	for (uint idx = 0; idx < _states.size(); ++idx) {
		const TTupdateState &us = _states[idx];
		if (newId == (idx == 0 ? 0 : us._newId)) {
			uint bits = us._dialBits;

			if (!bits
				|| (index == 1 && (bits & 1) && (bits & 4))
				|| (index == 0 && (bits & 2) && (bits & 4))
				|| (index == 3 && (bits & 1) && (bits & 8))
				|| (index == 2 && (bits & 2) && (bits & 8))) {
				newValue = us._newValue;
				break;
			}
		}
	}

	CTrueTalkManager::setFlags(17, newValue);
}

} // End of namespace Titanic
