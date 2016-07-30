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
#include "titanic/true_talk/doorbot_script.h"
#include "titanic/true_talk/tt_room_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

static const int STATE_ARRAY[9] = {
	0x2E2A, 0x2E2B, 0x2E2C, 0x2E2D, 0x2E2E, 0x2E2F, 0x2E30, 0x2E31, 0x2E32
};

struct RoomDialogueId {
	int _roomNum;
	int _dialogueId;
};
static const RoomDialogueId ROOM_DIALOGUES1[] = {
	{ 100, 10523 }, { 101, 10499 }, { 107, 10516 }, { 108, 10500 },
	{ 109, 10490 }, { 110, 10504 }, { 111, 10506 }, { 112, 10498 },
	{ 113, 10502 }, { 114, 10507 }, { 115, 10497 }, { 116, 10508 },
	{ 117, 10505 }, { 118, 10505 }, { 122, 10516 }, { 123, 10383 },
	{ 124, 10510 }, { 125, 10511 }, { 126, 10513 }, { 127, 10512 },
	{ 128, 10495 }, { 129, 10496 }, { 130, 10491 }, { 131, 10493 },
	{ 132, 10492 }, { 0, 0 }
};
static const RoomDialogueId ROOM_DIALOGUES2[] = {
	{ 102, 221981 }, { 110, 221948 }, { 111, 221968 }, { 107, 222000 },
	{ 101, 221935 }, { 112, 221924 }, { 113, 221942 }, { 116, 221977 },
	{ 124, 221987 }, { 125, 221984 }, { 127, 221991 }, { 128, 221916 },
	{ 129, 221919 }, { 131, 221912 }, { 132, 221908 }, { 0, 0 }
};

DoorbotScript::DoorbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	loadRanges("Ranges/Doorbot");
	loadResponses("Responses/Doorbot");
	setupSentences();
	_tagMappings.load("TagMap/Doorbot");
	_words.load("Words/Doorbot");
	_quotes.load("Quotes/Doorbot");
	_states.load("States/Doorbot");
}

void DoorbotScript::setupSentences() {
	for (int idx = 35; idx < 40; ++idx)
		CTrueTalkManager::setFlags(idx, 0);
	_state = 1;
	_dialValues[0] = _dialValues[1] = 100;

	_mappings.load("Mappings/Doorbot", 4);
	_entries.load("Sentences/Doorbot");
	_field68 = 0;
	_entryCount = 0;
}

int DoorbotScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	if (tag == MKTAG('D', 'N', 'A', '1') || tag == MKTAG('H', 'H', 'G', 'Q') ||
		tag == MKTAG('A', 'N', 'S', 'W') || tag == MKTAG('S', 'U', 'M', 'S')) {
		if (_stateIndex > 9)
			_stateIndex = 0;
		addResponse(STATE_ARRAY[_stateIndex]);
		applyResponse();

		if (STATE_ARRAY[_stateIndex] == 11826)
			setState(1);
		++_stateIndex;
		return 2;
	}

	if (tag == MKTAG('C', 'H', 'S', 'E') || tag == MKTAG('C', 'M', 'N', 'T') ||
			tag == MKTAG('J', 'F', 'O', 'D'))
		tag = MKTAG('F', 'O', 'O', 'D');

	if (tag == MKTAG('F', 'O', 'O', 'D') && roomScript->_scriptId == 132) {
		return setResponse(getDialogueId(220818));
	}

	if (tag == MKTAG('T', 'R', 'A', 'V')) {
		return setResponse(11858 - getRandomBit());
	} else if (tag == MKTAG('C', 'S', 'P', 'Y')) {
		return setResponse(10405, 3);
	} else if (tag == MKTAG('S', 'C', 'I', 'T')) {
		return setResponse(10410, 14);
	} else if (tag == MKTAG('L', 'I', 'T', 'E')) {
		return setResponse(10296, 17);
	} else if (tag == MKTAG('D', 'O', 'R', '1')) {
		return setResponse(getDialogueId(222034));
	} else if (tag == MKTAG('W', 'T', 'H', 'R')) {
		return setResponse(getDialogueId(222126));
	} else if (tag == MKTAG('N', 'A', 'U', 'T')) {
		return setResponse(getDialogueId(222259));
	} else if (tag == MKTAG('T', 'R', 'A', '2')) {
		return setResponse(getRandomBit() ? 11860 : 11859);
	} else if (tag == MKTAG('T', 'R', 'A', '3')) {
		return setResponse(getRandomBit() ? 11859 : 11858);
	} else if (tag == MKTAG('B', 'R', 'N', 'D')) {
		switch (getRandomNumber(3)) {
		case 1:
			tag = MKTAG('B', 'R', 'N', '2');
			break;
		case 2:
			tag = MKTAG('B', 'R', 'N', '3');
			break;
		default:
			break;
		}
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int DoorbotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult DoorbotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int DoorbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
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
	case MKTAG('W', 'W', 'E', 'B'):
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
	case MKTAG('B', 'L', 'T', '1'):
	case MKTAG('B', 'L', 'T', '2'):
	case MKTAG('B', 'L', 'T', '3'):
	case MKTAG('B', 'L', 'T', '4'):
	case MKTAG('B', 'L', 'T', '5'):
	case MKTAG('B', 'O', 'Y', 'S'):
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

int DoorbotScript::updateState(uint oldId, uint newId, int index) {
	getValue(38);
	bool flag39 = getValue(39) != 0;
	CTrueTalkManager::setFlags(38, 0);
	CTrueTalkManager::setFlags(39, 0);
	
	if (newId > 220890) {
		switch (newId) {
		case 221064:
			return getValue(1) == 2 ? newId : 221062;
		case 221080:
			return getValue(1) >= 2 ? newId : 221066;
		case 221078:
		case 221079:
			return getValue(1) >= 3 ? newId : 221065;
		case 221081:
			return getValue(7) == 0 ? newId : 221070;
		case 221251:
			CTrueTalkManager::triggerAction(28, 0);
			break;
		default:
			break;
		}
	} else if (newId >= 220883) {
		CTrueTalkManager::setFlags(38, 1);
		CTrueTalkManager::triggerAction(28, 0);
	} else if (newId >= 220076) {
		switch (newId) {
		case 220078:
		case 220080:
		case 220081:
		case 220082:
		case 220083:
		case 220084:
			if (flag39)
				return getRangeValue(221381);
			break;
		default:
			break;
		}

		CTrueTalkManager::setFlags(39, 1);
	} else if (newId == 220075) {
		if (flag39)
			return getRangeValue(221381);
		CTrueTalkManager::setFlags(39, 1);
	} else if (newId == 220038) {
		return 220038;
	}

	for (uint idx = 0; idx < _states.size(); ++idx) {
		TTupdateState &us = _states[idx];
		if (us._newId == newId) {
			uint bits = us._dialBits;

			if (!bits
				|| (index == 0 && (bits == 5 || bits == 1))
				|| (index == 1 && (bits == 6 || bits == 2))
				|| (index == 2 && (bits == 9 || bits == 1))
				|| (index == 3 && (bits == 10 || bits == 2))) {
				setState(us._newValue);
				break;
			}
		}
	}

	return newId;
}

int DoorbotScript::preResponse(uint id) {
	uint newId = 0;
	if (getDialRegion(0) != 1 && getRandomNumber(100) > 60) {
		addResponse(11195);
		newId = 222193;
	}

	return newId;
}

uint DoorbotScript::getDialsBitset() const {
	uint bits = 0;
	if (!getDialRegion(1))
		bits = 1;
	if (!getDialRegion(0))
		bits |= 2;

	return bits;
}

int DoorbotScript::doSentenceEntry(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence) {
	int id2, id = 0;

	switch (val1) {
	case 2:
		if (getValue(1) != 1)
			return 1;
		break;
	case 3:
		if (getValue(1) != 2)
			return 1;
		break;
	case 4:
		if (getValue(1) != 3)
			return 1;
		break;
	case 5:
		if (getValue(1) == 3)
			return 1;
	case 6:
		if (getRoom54(132))
			return 1;
		break;
	case 9:
		if (sentence->localWord("my") || sentence->contains("my"))
			return true;
		id2 = getRoomDialogueId1(roomScript);
		if (id2) {
			addResponse(id2);
			applyResponse();
			return 2;
		}
		break;
	case 11:
		switch (getValue(1)) {
		case 1:
			id = 220837;
			break;
		case 2:
			id = 220849;
			break;
		default:
			id = 220858;
			break;
		}
		break;
	case 12:
		if (getValue(4) != 1)
			id = 221157;
		break;
	case 13:
		if (getValue(4) != 2)
			id = 221157;
		break;
	case 14:
		if (getValue(4) != 3)
			id = 221157;
		break;
	case 15:
		if (getValue(4) != 0)
			id = 221157;
		break;
	case 16:
		if (!sentence->localWord("weather"))
			return true;
		switch (getRandomNumber(4)) {
		case 1:
			if (getValue(4) != 0)
				id = 221354 - getRandomNumber(2) ? -489 : 0;
			break;
		case 2:
			switch (getValue(4)) {
			case 0:
				id = 220851;
				break;
			case 1:
				id = 221268;
				break;
			case 2:
				id = 221270;
				break;
			default:
				id = 220865;
			}
			break;
		case 3:
			id = 221280;
			break;
		default:
			break;
		}
		break;
	case 17:
		if (getState())
			return 1;
		setState(0);
		break;
	case 18:
		if (roomScript->_scriptId == 100) {
			CTrueTalkManager::triggerAction(3, 0);
			return 2;
		}
		break;
	case 19:
		CTrueTalkManager::_v9 = 104;
		CTrueTalkManager::triggerAction(4, 0);
		break;
	case 20:
		CTrueTalkManager::triggerAction(28, 0);
		break;
	case 22:
		CTrueTalkManager::triggerAction(29, 1);
		break;
	case 23:
		CTrueTalkManager::triggerAction(29, 2);
		break;
	case 24:
		CTrueTalkManager::triggerAction(29, 3);
		break;
	case 25:
		CTrueTalkManager::triggerAction(29, 4);
		break;
	case 26:
		if (!sentence->localWord("my") && !sentence->contains("my"))
			return 1;
		break;
	case 27:
		if (!sentence->localWord("earth") && !sentence->contains("earth"))
			return 1;
		break;
	case 28:
		id2 = getRoomDialogueId2(roomScript);
		if (id2) {
			addResponse(id2);
			applyResponse();
			return 2;
		}
		break;
	case 29:
		if (sentence->localWord("another") || sentence->localWord("more") ||
				sentence->localWord("additional") || sentence->contains("another") ||
				sentence->contains("more") || sentence->contains("additional")) { 
			addResponse(getDialogueId(220058));
			applyResponse();
			return 2;
		}
		break;
	case 30:
		if (!sentence->localWord("because") && !sentence->contains("because"))
			return 1;
		break;
	case 0x200:
		if (getValue(4) != 1)
			id = 221157;
		break;
	case 0x201:
		if (getValue(4) != 2)
			id = 221157;
		break;
	case 0x202:
		if (getValue(4) != 3)
			id = 221157;
		break;
	case 0x203:
		if (getValue(4) != 0)
			id = 221157;
		break;
	default:
		break;
	}

	warning("TODO");
	return 0;
}

void DoorbotScript::setDialRegion(int dialNum, int region) {
	TTnpcScript::setDialRegion(dialNum, region);
	if (dialNum == 1 && region != 1) {
		CTrueTalkManager::setFlags(37, dialNum);
	} else {
		addResponse(getDialogueId(221777));
		applyResponse();
	}
}

bool DoorbotScript::randomResponse(int index) {
	static const int DIALOGUE_IDS[] = {
		220133, 220074, 220000, 220008, 220009, 220010, 220011,
		220012, 220013, 220014, 220015, 220016, 221053, 221054,
		221055, 221056, 221057, 221058, 221059, 221060, 221061,
		221173, 221174, 221175, 221176, 221177, 222415, 222416,
		221157, 221165, 221166, 221167, 221168, 221169, 221170,
		221171, 221172, 221158, 221159, 221356, 221364, 221365,
		221366, 221367, 221368, 221369, 221370, 221371, 221357,
		221358, 221359, 221360, 221252, 221019, 221355, 220952,
		220996, 220916, 220924, 220926, 220931, 220948, 220956,
		220965, 220967, 220968, 220980, 220981, 220982, 220983,
		220984, 220988, 220903, 221095, 222202, 222239, 221758,
		221759, 221762, 221763, 221766, 221767, 221768, 0
	};

	int *dataP = _data.getSlot(index);
	bool flag = false;
	for (const int *idP = DIALOGUE_IDS; *idP && !flag; ++idP) {
		flag = *idP == *dataP;
	}

	if (flag || (getDialRegion(1) != 1 && getRandomNumber(100) > 33)
			|| getRandomNumber(8) <= index)
		return false;

	if (getRandomNumber(100) > 40) {
		deleteResponses();
		addResponse(getDialogueId(221242));
		applyResponse();
	} else {
		setResponseFromArray(index, 221245);
	}

	return true;
}

int DoorbotScript::setResponse(int dialogueId, int v34) {
	addResponse(dialogueId);
	applyResponse();

	if (v34 != -1)
		setState(v34);
	return 2;
}

int DoorbotScript::getRoomDialogueId1(const TTroomScript *roomScript) {
	for (const RoomDialogueId *r = ROOM_DIALOGUES1; r->_roomNum; ++r) {
		if (r->_roomNum == roomScript->_scriptId == r->_roomNum)
			return getDialogueId(r->_dialogueId);
	}

	return 0;
}

int DoorbotScript::getRoomDialogueId2(const TTroomScript *roomScript) {
	for (const RoomDialogueId *r = ROOM_DIALOGUES2; r->_roomNum; ++r) {
		if (r->_roomNum == roomScript->_scriptId == r->_roomNum)
			return getDialogueId(r->_dialogueId);
	}

	return 0;
}

} // End of namespace Titanic
