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
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

int DeskbotScript::_oldId;

#define CURRENT_STATE 17

DeskbotScript::DeskbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	CTrueTalkManager::setFlags(18, 0);
	CTrueTalkManager::setFlags(19, 0);
	CTrueTalkManager::setFlags(20, 0);
	CTrueTalkManager::setFlags(21, 0);
	CTrueTalkManager::setFlags(22, 0);

	setupDials(0, 0, 0);
	_data[0] = 100;
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
	_entries2.load("Sentences/Deskbot/2");
	_entries3.load("Sentences/Deskbot/3");
	_dialValues[0] = _dialValues[1] = 0;
	_field68 = 0;
	_entryCount = 0;
}

int DeskbotScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (roomScript->_scriptId != 110)
		return 2;

	bool flag20 = getValue(20) != 0;
	CTrueTalkManager::setFlags(20, 0);
	checkItems(nullptr, nullptr);

	if (preprocess(roomScript, sentence) != 1)
		return 1;

	CTrueTalkManager::setFlags(CURRENT_STATE, 0);
	setState(0);
	updateCurrentDial(false);

	if (getValue(1) == 3) {
		if (sentence->localWord("competition") || sentence->contains("competition")
				|| sentence->localWord("won") || sentence->contains("won")
				|| sentence->localWord("winning") || sentence->contains("winning")
				|| sentence->localWord("winner") || sentence->contains("winner")
				|| sentence->contains("35279") || sentence->contains("3 5 2 7 9")
				|| (g_language == Common::DE_DEU && (
					sentence->contains("preisaus") ||
					sentence->contains("tombola") ||
					sentence->contains("lotterie") ||
					sentence->contains("gewonnen") ||
					sentence->contains("glueck") ||
					sentence->contains("das goldene los") ||
					sentence->contains("sechs richtige")
				))
			) {
			addResponse(getDialogueId(41773));
			applyResponse();
			return 2;
		} else if (sentence->localWord("magazine") || sentence->contains("magazine")) {
			addResponse(getDialogueId(41771));
			applyResponse();
			return 2;
		} else if (sentence->localWord("upgrade") || sentence->contains("upgrade")
				|| sentence->localWord("secondclass")) {
			if (CTrueTalkManager::_currentNPC) {
				CGameObject *obj;
				if (CTrueTalkManager::_currentNPC->find("Magazine", &obj, FIND_PET)) {
					addResponse(getDialogueId(41773));
					applyResponse();
					return 2;
				}
			}
		}
	}

	if (processEntries(&_entries, _entryCount, roomScript, sentence) != 2
			&& processEntries(&_entries2, 0, roomScript, sentence) != 2) {
		if (sentence->localWord("sauce") || sentence->localWord("pureed")) {
			addResponse(getDialogueId(240398));
			applyResponse();
		} else if (sentence->contains("cherries")) {
			addResponse(getDialogueId(240358));
			applyResponse();
		} else if (sentence->contains("42")) {
			addResponse(getDialogueId(240453));
			applyResponse();
		} else if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241778));
			applyResponse();
		} else {
			if (sentence->contains("98129812"))
				setDialRegion(1, 1);

			if (!defaultProcess(roomScript, sentence)
					&& processEntries(&_entries3, 0, roomScript, sentence) != 2
					&& processEntries(_defaultEntries, 0, roomScript, sentence) != 2) {
				if (flag20)
					CTrueTalkManager::setFlags(20, 1);
				addResponse(getDialogueId(240569));
				applyResponse();
			}
		}
	}

	return 2;
}

ScriptChangedResult DeskbotScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	switch (id) {
	case 3:
	case 100:
	case 108:
		CTrueTalkManager::setFlags(21, getValue(21) + 1);
		addResponse(getDialogueId(getValue(22) ? 240577 : 241261));
		applyResponse();
		break;

	case 109:
		addResponse(getDialogueId(241627));
		applyResponse();
		break;

	case 140:
		if (getValue(1) == 3)
			addAssignedRoomDialogue3();
		break;

	case 148:
		CTrueTalkManager::setFlags(3, 1);
		break;

	case 150:
		CTrueTalkManager::setFlags(2, 1);
		break;

	default:
		break;
	}

	return SCR_2;
}

int DeskbotScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
		uint tag1, uint tag2, uint remainder) {
	switch (tag2) {
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
		tag2 = MKTAG('E', 'N', 'T', 'N');
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
		tag2 = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
	case MKTAG('J', 'F', 'O', 'D'):
	case MKTAG('L', 'I', 'Q', 'D'):
		tag2 = MKTAG('F', 'O', 'O', 'D');
		break;
	case MKTAG('C', 'R', 'I', 'M'):
	case MKTAG('C', 'S', 'P', 'Y'):
	case MKTAG('D', 'R', 'U', 'G'):
		tag2 = MKTAG('V', 'B', 'A', 'D');
		break;
	case MKTAG('E', 'A', 'R', 'T'):
	case MKTAG('H', 'O', 'M', 'E'):
	case MKTAG('N', 'P', 'L', 'C'):
	case MKTAG('P', 'L', 'A', 'N'):
		tag2 = MKTAG('P', 'L', 'A', 'C');
		break;
	case MKTAG('F', 'A', 'U', 'N'):
	case MKTAG('F', 'I', 'S', 'H'):
	case MKTAG('F', 'L', 'O', 'R'):
		tag2 = MKTAG('N', 'A', 'T', 'R');
		break;
	case MKTAG('H', 'H', 'L', 'D'):
	case MKTAG('T', 'O', 'Y', 'S'):
	case MKTAG('W', 'E', 'A', 'P'):
		tag2 = MKTAG('M', 'A', 'C', 'H');
		break;
	case MKTAG('M', 'L', 'T', 'Y'):
	case MKTAG('P', 'G', 'R', 'P'):
	case MKTAG('P', 'T', 'I', 'C'):
		tag2 = MKTAG('G', 'R', 'U', 'P');
		break;
	case MKTAG('P', 'K', 'U', 'P'):
	case MKTAG('S', 'E', 'X', '1'):
	case MKTAG('S', 'W', 'E', 'R'):
		tag2 = MKTAG('R', 'U', 'D', 'E');
		break;
	case MKTAG('P', 'H', 'I', 'L'):
	case MKTAG('R', 'C', 'K', 'T'):
		tag2 = MKTAG('S', 'C', 'I', 'E');
		break;
	case MKTAG('T', 'R', 'A', '2'):
	case MKTAG('T', 'R', 'A', '3'):
		tag2 = MKTAG('T', 'R', 'A', 'V');
		break;
	default:
		break;
	}

	return TTnpcScript::handleQuote(roomScript, sentence, tag1, tag2, remainder);
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
	setCurrentState(newId, index);

	return newId;
}

int DeskbotScript::preResponse(uint id) {
	int newId = 0;
	if (getValue(1) >= 3 && (
			id == (uint)TRANSLATE(41176, 41190) ||
			id == (uint)TRANSLATE(41738, 41429) ||
			id == (uint)TRANSLATE(41413, 41755) ||
			id == (uint)TRANSLATE(41740, 41757)
		)) {
		newId = 241601;
	}
	if (id == (uint)TRANSLATE(42114, 42132))
		CTrueTalkManager::triggerAction(20, 0);

	return newId;
}

uint DeskbotScript::getDialsBitset() const {
	if (getDialRegion(1))
		return getDialRegion(0) ? 2 : 3;
	else
		return getDialRegion(0) ? 0 : 1;
}

int DeskbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	uint id = *srcIdP;

	if (g_language == Common::DE_DEU) {
		if (val1 != 4501)
			return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);

		switch (getValue(1)) {
		case 1:
			id = 240336;
			break;

		case 2:
			id = addAssignedRoomDialogue();
			break;

		case 3:
			if (id == 240431 || id == 240432) {
				if (getValue(3) == 1) {
					if (id == 240431)
						id = 240432;
				} else {
					if (id == 240432)
						id = 240431;
				}
			}
			break;

		default:
			break;
		}

		addResponse(getDialogueId(id));
		applyResponse();
		return 2;
	} else {
		switch (val1) {
		case 1:
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
					} else {
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
				return 1;
			break;

		case 3:
			if (getValue(1) != 1)
				return 1;
			break;

		default:
			break;
		}

		return 0;
	}
}

bool DeskbotScript::randomResponse(uint index) {
	if (getValue(1) == 1 || getRandomNumber(100) > 10 || getRandomNumber(2) <= index)
		return 0;

	if (getRandomNumber(100) > 95) {
		deleteResponses();
		addResponse(getDialogueId(241195));
		applyResponse();
	} else {
		setResponseFromArray(index, 241193);
	}

	return true;
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

void DeskbotScript::setCurrentState(uint newId, uint index) {
	int newValue = getValue(CURRENT_STATE);

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

	CTrueTalkManager::setFlags(CURRENT_STATE, newValue);
}

int DeskbotScript::preprocess(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!roomScript || !sentence)
		return 1;

	bool stateFlag = true, applyFlag = false;
	switch (getValue(CURRENT_STATE)) {
	case 1:
		if (sentence->_category != 3 && sentence->_category != 4
				&& sentence->_category != 6 && sentence->_category != 10
				&& sentence->_category != 2) {
			addResponse(getDialogueId(240423));
			applyFlag = true;
		}
		break;

	case 2:
		if (sentence->localWord("gobbledygook")) {
			addResponse(getDialogueId(240427));
			applyFlag = true;
		}
		break;

	case 3:
		if (sentence->_category == 11 || sentence->_category == 13
				|| sentence->_category == 3 || sentence->localWord("upgrade")) {
			addResponse(getDialogueId(240433));
			applyFlag = true;
		}
		break;

	case 4:
		addResponse(getDialogueId(sentence->_category == 11 || sentence->_category == 13
			? 240495 : 240494));
		applyFlag = true;
		break;

	case 5:
		if (isDial1Low() && getValue(1) == 4) {
			if (sentence->localWord("name") || sentence->localWord("called")
				|| sentence->localWord("passenger")) {
				addResponse(getDialogueId(240867));
				addResponse(getDialogueId(240745));
			} else {
				addResponse(getDialogueId(240446));
			}

			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 6:
		if (isDial1Low() && getValue(1) == 4) {
			if (sentence->localWord("passenger")) {
				addResponse(getDialogueId(240449));
				applyFlag = true;
				stateFlag = false;
			} else if (sentence->localWord("home")
				|| sentence->localWord("destroy")) {
				addResponse(getDialogueId(240574));
				stateFlag = true;
			} else if (sentence->localWord("name")
				|| sentence->localWord("called")
				|| sentence->_category == 11) {
				addResponse(getDialogueId(240448));
				stateFlag = true;
			} else {
				addResponse(getDialogueId(240489));
				stateFlag = true;
			}
		}
		break;

	case 7:
		if (sentence->_category == 11 || sentence->_category == 13
				|| (sentence->localWord("what") && sentence->localWord("wrong"))
				|| sentence->localWord("clothes")) {
			addResponse(getDialogueId(240489));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 8:
		if (isDial1Low() && getValue(1) == 4) {
			if (sentence->_category == 12 || sentence->_category == 13 ||
					(g_language != Common::DE_DEU && sentence->contains("do not"))) {
				addResponse(getDialogueId(240447));
				setDialRegion(0, 0);
				setDialRegion(1, 0);
				CTrueTalkManager::setFlags(1, 3);
				CTrueTalkManager::triggerAction(19, 3);
				CTrueTalkManager::setFlags(22, 1);
				applyFlag = true;
			} else if (sentence->_category == 11) {
				addResponse(getDialogueId(240746));
				applyFlag = true;
				stateFlag = false;
			} else {
				addResponse(getDialogueId(240448));
				applyFlag = true;
				stateFlag = false;
			}
		}
		break;

	case 9:
		if (searchQuotes(roomScript, sentence)) {
			if (isDial0Medium()) {
				addResponse(getDialogueId(240382));
				applyFlag = true;
				stateFlag = false;
			} else {
				addResponse(getDialogueId(240548));
				applyFlag = true;
			}
		}
		break;

	case 10:
		if (isDial1Medium() && searchQuotes(roomScript, sentence)) {
			if (isDial0Medium()) {
				addResponse(getDialogueId(240405));
				applyFlag = true;
				stateFlag = false;
			} else {
				addResponse(getDialogueId(240548));
				applyFlag = true;
			}
		}
		break;

	case 11:
		if (isDial0Medium() && isDial1Medium()
				&& searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240406));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 12:
		if (isDial0Medium() && isDial1Medium()
				&& searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240548));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 13:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240348));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 14:
		if (isDial1Medium()) {
			addResponse(getDialogueId(240568));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 15:
		if (sentence->localWord("magazine")) {
			addAssignedRoomDialogue3();
			stateFlag = true;
		}
		break;

	case 17:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240401));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 18:
		if (!isDial0Low() || !isDial1Low()) {
			if (searchQuotes(roomScript, sentence)) {
				addResponse(getDialogueId(240402));
				applyFlag = true;
				stateFlag = false;
			}
		}
		break;

	case 19:
		if (!isDial0Low() && searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240403));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 20:
		if (!isDial1Medium() && searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240548));
			applyFlag = true;
		}
		break;

	case 21:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240382));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 22:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240383));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 23:
		if (isDial0Medium() && searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240384));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 24:
		setDialRegion(0, 0);

		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240548));
			applyFlag = true;
		}
		break;

	case 25:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240349));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 26:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240350));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 27:
	case 30:
	case 33:
	case 36:
	case 40:
	case 43:
	case 46:
	case 50:
	case 55:
	case 58:
	case 61:
	case 68:
	case 73:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240548));
			applyFlag = true;
		}
		break;

	case 28:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240373));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 29:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240374));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 31:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240346));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 32:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240347));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 34:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240376));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 35:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240377));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 37:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240369));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 38:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240370));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 39:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240371));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 41:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240352));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 42:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240353));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 44:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240389));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 45:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240390));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 47:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240395));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 48:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240396));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 49:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240397));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 51:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240363));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 52:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240364));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 53:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240365));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 54:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240366));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 56:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240392));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 57:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240393));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 59:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240386));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 60:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240387));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 62:
		if (isDial1Medium() && getValue(1) == 1) {
			if (isDial0Medium()) {
				if (sentence->localWord("down")) {
					addResponse(getDialogueId(240413));
					applyFlag = true;
				}
			} else if (sentence->contains("left") || sentence->contains("right")) {
				addResponse(getDialogueId(240415));
				CTrueTalkManager::triggerAction(sentence->localWord("down") ? 22 : 23, 0);
			} else {
				addResponse(getDialogueId(240414));
				applyFlag = true;
			}
		}
		break;

	case 63:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240836));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 64:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240837));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 65:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240838));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 66:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240839));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 67:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(240840));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 69:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241104));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 70:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241105));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 71:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241106));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 72:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241107));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 74:
	case 75:
		if (sentence->_category == 24) {
			addResponse(getDialogueId(240972));
			applyFlag = true;
		} else if (sentence->localWord("good") || sentence->localWord("yes")
				|| sentence->localWord("well") || sentence->localWord("ill")
				|| sentence->localWord("sad")) {
			addResponse(getDialogueId(240805));
			applyFlag = true;
		}
		break;

	case 76:
		if (sentence->_category == 6) {
			addResponse(getDialogueId(240767));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 77:
		if (sentence->_category == 3) {
			addResponse(getDialogueId(241109));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 78:
		// "Do you have a reservation?"
		if (sentence->_category == 11 || sentence->_category == 13) {
			// Player said they have a reservation
			addResponse(getDialogueId(241262));
		} else if (sentence->_category == 12 ||
				(g_language != Common::DE_DEU && sentence->contains("do not"))) {
			// Player said they don't have a reservation
			setDialRegion(0, 0);
			setDialRegion(1, 0);
			addResponse(getDialogueId(241268));
			addAskBreakfast();
		} else if (sentence->contains("skip")) {
			// WORKAROUND: Added 'skip' to allow skipping entire checkin
			addAssignedRoom();
			setState(0);
			CTrueTalkManager::setFlags(CURRENT_STATE, 0);
		} else {
			// Player didn't say yes or no
			addResponse(getDialogueId(240745));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 79:
		switch (checkCommonWords(sentence)) {
		case 1:
			addResponse(getDialogueId(241263));
			break;
		case 2:
			addResponse(getDialogueId(241266));
			break;
		case 3:
			addAssignedRoom();
			setState(0);
			CTrueTalkManager::setFlags(CURRENT_STATE, 0);
			return 2;
		default:
			addResponse(getDialogueId(241267));
			break;
		}

		addAskBreakfast();
		applyFlag = true;
		stateFlag = false;
		break;

	case 81:
		addResponse(getDialogueId(sentence->_category == 12 ? 240602 : 241337));
		applyResponse();
		setState(0);
		CTrueTalkManager::setFlags(CURRENT_STATE, 0);
		return 2;

	case 82:
		if (sentence->_category == 2) {
			addResponse(getDialogueId(241339));
			applyFlag = true;
		}
		break;

	case 83:
		if ((isDial1Medium() && isDial0Low()) ||
				(isDial1Low() && isDial0Medium())) {
			if (sentence->localWord("yes") || sentence->localWord("but")) {
				if (sentence->localWord("will") || sentence->localWord("do")) {
					addResponse(getDialogueId(241366));
					applyFlag = true;
				}
			}
		}
		break;

	case 84:
		if (sentence->_category == 12 || sentence->contains("vegetarian")
				|| sentence->contains("vegitarian")) {
			addResponse(getDialogueId(241718));
			addResponse(getDialogueId(241709));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->contains("continental")
				|| sentence->contains("kontinent")
				|| sentence->contains("full")
				|| sentence->contains("porky")
				|| sentence->contains("the 1")
				|| sentence->contains("the 2")
				|| sentence->contains("former")
				|| sentence->contains("latter")
				|| sentence->contains("speck")
				|| sentence->contains("wurst")
				|| sentence->contains("englisch")
			) {
			addResponse(getDialogueId(241717));
			addResponse(getDialogueId(241709));
			applyFlag = true;
			stateFlag = false;
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));
			addResponse(getDialogueId(241719));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 85:
		if (sentence->_category == 12 || sentence->contains("bugle")
				|| sentence->contains("buggle") || sentence->contains("trumpet")
				|| sentence->contains("saxophone") || sentence->contains("kazoo")
				|| sentence->contains("blerontin 1") || sentence->contains("the 1")
				|| sentence->contains("the 2") || sentence->contains("the 3")
				|| sentence->contains("the 4") || sentence->contains("all of them")
				|| sentence->contains("the lot") || sentence->contains("buegelhorn")
				|| sentence->contains("bugelhorn") || sentence->contains("trompete")
				|| sentence->contains("saxophon") || sentence->contains("popanz")
			) {
			addResponse(getDialogueId(241710));
			addResponse(getDialogueId(241713));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			addResponse(getDialogueId(241711));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 86:
		if (sentence->_category == 12 || sentence->_category == 11
				|| sentence->contains(TRANSLATE("view", "aussicht"))) {
			addResponse(getDialogueId(241714));
			addResponse(getDialogueId(241699));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			if (getRandomNumber(100) < 50) {
				addResponse(getDialogueId(241715));
			} else {
				addResponse(getDialogueId(241712));
				addResponse(getDialogueId(241713));
			}

		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 87:
		if (sentence->contains("corner") || sentence->contains("on the end")
				|| sentence->contains("balcony") || sentence->contains("neither")
				|| sentence->contains("the 1") || sentence->contains("the 2")
				|| sentence->contains("former") || sentence->contains("latter")
				|| sentence->contains("either") || sentence->contains("suedlage")
				|| sentence->contains("balkon")) {
			addResponse(getDialogueId(241700));
			addResponse(getDialogueId(241687));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			addResponse(getDialogueId(241701));
			addResponse(getDialogueId(241699));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 88:
		if (sentence->contains("imperial") || sentence->contains(TRANSLATE("the 1", "fuerstlich"))) {
			addResponse(getDialogueId(241700));
			addResponse(getDialogueId(241739));
		} else if (sentence->contains("royal") || sentence->contains(TRANSLATE("the 2", "majest"))) {
			addResponse(getDialogueId(241690));
		} else if (sentence->contains("despotic") || sentence->contains("despot")
				|| sentence->contains("the last") || sentence->contains("latter")) {
			addResponse(getDialogueId(241688));
		} else if (sentence->contains("president") || sentence->contains("presidential")
				|| sentence->contains("the 3") || sentence->contains("praesident")) {
			addResponse(getDialogueId(241689));
			addResponse(getDialogueId(241739));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			addResponse(getDialogueId(241692));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 89:
		if (sentence->contains("king")) {
			addResponse(getDialogueId(241691));
		} else if (sentence->contains("queen") || sentence->contains("prince")
				|| sentence->contains("princess") || sentence->contains("small")
				|| sentence->contains("the 1") || sentence->contains("the 2")
				|| sentence->contains("the 3") || sentence->contains("the 4")
				|| sentence->contains("big") || sentence->contains("large")
			|| sentence->contains("prinz") || sentence->contains("gross")) {
			addResponse(getDialogueId(241700));
			addResponse(getDialogueId(241739));
		} else {
			if (getRandomNumber(100) < 100 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			addResponse(getDialogueId(241690));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 90:
		if (sentence->contains("constitutional") || sentence->contains("const")
				|| sentence->contains("absolute") || sentence->contains("small")
				|| sentence->contains("the 1") || sentence->contains("the 2")
				|| sentence->contains("big") || sentence->contains("large")
				|| sentence->contains("verfassung") || sentence->contains("absolut")
				|| sentence->contains("gross")) {
			addResponse(getDialogueId(241700));
			addResponse(getDialogueId(241739));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241708));

			addResponse(getDialogueId(241691));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 91:
		if (sentence->contains("benev") || sentence->contains("dict")
				|| sentence->contains("small") || sentence->contains("the 1")
				|| sentence->contains("the 2") || sentence->contains("big")
				|| sentence->contains("large") || sentence->contains("former")
				|| sentence->contains("latter")
				|| sentence->contains("aufgekl") || sentence->contains("diktator")
				|| sentence->contains("gross")) {
			addResponse(getDialogueId(241700));
			addResponse(getDialogueId(241739));
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241708));

			addResponse(getDialogueId(241688));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 92:
	case 93:
		// "Fish?"
		if (sentence->_category == 11 || sentence->_category == 13) {
			// Yes
			addResponse(getDialogueId(241077));
			addResponse(getDialogueId(241706));
		} else if (sentence->_category == 12) {
			// No
			addAssignedRoom();
			setState(0);
			CTrueTalkManager::setFlags(CURRENT_STATE, 0);
			return 2;
		} else if (g_vm->_trueTalkManager->_quotes.find(sentence->_initialLine.c_str())
				== MKTAG('F', 'I', 'S', 'H')) {
			// WORKAROUND: Original passed _normalizedLine, but "fish" is one of the common
			// phrases replaced with with alternative text "completelyrandom"
			addResponse(getDialogueId(240877));
			addResponse(getDialogueId(241706));
		}else {
			// All other responses
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));

			addResponse(getDialogueId(241705));
			if (getRandomNumber(100) < 80)
				addResponse(getDialogueId(241739));
		}

		applyFlag = true;
		stateFlag = false;
		break;

	case 94:
		if (sentence->contains("seperate") || sentence->contains("separate")
				|| sentence->contains("detached") || sentence->contains("outside")
				|| sentence->contains("onsweet") || sentence->contains("ensuite")
				|| sentence->contains("suite") || sentence->contains("next door")
				|| sentence->contains("the 1") || sentence->contains("the 2")
				|| sentence->contains("former") || sentence->contains("latter")
				|| sentence->contains("same room") || sentence->contains("ohne")
				|| sentence->contains("mit") || sentence->contains("die gleiche kabine")
				|| sentence->contains("dieselbe kabine")) {
			addAssignedRoom();
			setState(0);
			CTrueTalkManager::setFlags(CURRENT_STATE, 0);
			return 2;
		} else {
			if (getRandomNumber(100) < 80 && sentence2C(sentence))
				addResponse(getDialogueId(241707));
			addResponse(getDialogueId(241706));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 95:
		if (isDial1Medium()) {
			if ((sentence->localWord("i") && sentence->localWord("am"))
				|| sentence->localWord("me")) {
				addResponse(getDialogueId(240632));
				applyFlag = true;
				stateFlag = false;
			}
		}
		break;

	case 96:
		if (sentence->_category == 2) {
			addResponse(getDialogueId(241350));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 97:
		if (searchQuotes(roomScript, sentence)) {
			addResponse(getDialogueId(241351));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	default:
		break;
	}

	if (applyFlag)
		applyResponse();
	if (stateFlag) {
		setState(0);
		CTrueTalkManager::setFlags(CURRENT_STATE, 0);
	}

	return applyFlag ? 2 : 1;
}

int DeskbotScript::searchQuotes(const TTroomScript *roomScript, const TTsentence *sentence) {
	TTtreeResult treeResult[32];
	return g_vm->_trueTalkManager->_quotesTree.search(sentence->_normalizedLine.c_str(),
		TREE_2, &treeResult[0], 0, 0) != -1;
}

int DeskbotScript::checkCommonWords(const TTsentence *sentence) {
	if (sentence->contains("xyzzy"))
		return 3;

	const TTquotes &quotes = g_vm->_trueTalkManager->_quotes;
	if (quotes._loaded) {
		uint tagId = quotes.find(sentence->_normalizedLine.c_str());
		if (tagId == MKTAG('F', 'U', 'L', 'N')
				|| tagId == MKTAG('T', 'D', 'V', 'P')
				|| tagId == MKTAG('H', 'E', 'R', 'O')
				|| sentence->contains("douglas adam"))
			return 1;
		else if (tagId == MKTAG('J', 'N', 'A', 'M')
				|| tagId == MKTAG('N', 'I', 'K', 'N')
				|| tagId == MKTAG('B', 'O', 'Y', 'S')
				|| tagId == MKTAG('G', 'I', 'R', 'L'))
			return 2;
	} else {
		if (sentence->contains("douglas adams")
				|| sentence->contains("shaikh")
				|| sentence->contains("millican")
				|| sentence->contains("williams")
				|| sentence->contains("henkes")
				|| sentence->contains("kenny"))
			return 1;
		else if (sentence->contains("richard")
				|| sentence->contains("jason")
				|| sentence->contains("mike")
				|| sentence->contains("renata"))
			return 2;
	}

	return 0;
}

void DeskbotScript::addAskBreakfast() {
	addResponse(getDialogueId(241716));
}

void DeskbotScript::addAssignedRoom() {
	addResponse(getDialogueId(241696));
	addResponse(getDialogueId(241697));
	CTrueTalkManager::setFlags(1, 3);
	CTrueTalkManager::triggerAction(19, 3);
	CTrueTalkManager::setFlags(22, 1);

	int roomNum = 1, floorNum = 1, elevatorNum = 1;
	getAssignedRoom(&roomNum, &floorNum, &elevatorNum);
	addResponse(getDialogueId(241313 + elevatorNum));
	addResponse(getDialogueId(241271 + floorNum));
	addResponse(getDialogueId(241317 + roomNum));
	addResponse(getDialogueId(241698));
	applyResponse();
}

} // End of namespace Titanic
