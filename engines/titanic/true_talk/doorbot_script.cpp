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
		if (_state > 9)
			_state = 0;
		addResponse(STATE_ARRAY[_state]);
		applyResponse();

		if (STATE_ARRAY[_state] == 11826)
			set34(1);
		++_state;
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

bool DoorbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc22(int id) const {
	warning("TODO");
	return 0;
}

uint DoorbotScript::getDialsBitset() const {
	uint bits = 0;
	if (!getDialRegion(1))
		bits = 1;
	if (!getDialRegion(0))
		bits |= 2;

	return bits;
}

int DoorbotScript::proc25(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence) {
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
		if (get34())
			return 1;
		set34(0);
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

void DoorbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
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

int DoorbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

int DoorbotScript::setResponse(int dialogueId, int v34) {
	addResponse(dialogueId);
	applyResponse();

	if (v34 != -1)
		set34(v34);
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
