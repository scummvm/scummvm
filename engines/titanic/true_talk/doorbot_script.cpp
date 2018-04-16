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
#include "titanic/translation.h"
#include "titanic/titanic.h"

namespace Titanic {

static const int STATE_ARRAY_EN[9] = {
	11818, 11819, 11820, 11821, 11822, 11823, 11824, 11825, 11826
};
static const int STATE_ARRAY_DE[9] = {
	11831, 11832, 11833, 11834, 11835, 11836, 11837, 11838, 11839
};

static const RoomDialogueId ROOM_DIALOGUES1_EN[] = {
	{ 100, 10523 }, { 101, 10499 }, { 107, 10516 }, { 108, 10500 },
	{ 109, 10490 }, { 110, 10504 }, { 111, 10506 }, { 112, 10498 },
	{ 113, 10502 }, { 114, 10507 }, { 115, 10497 }, { 116, 10508 },
	{ 117, 10505 }, { 118, 10505 }, { 122, 10516 }, { 123, 10383 },
	{ 124, 10510 }, { 125, 10511 }, { 126, 10513 }, { 127, 10512 },
	{ 128, 10495 }, { 129, 10496 }, { 130, 10491 }, { 131, 10493 },
	{ 132, 10492 }, { 0, 0 }
};
static const RoomDialogueId ROOM_DIALOGUES1_DE[] = {
	{ 101, 10375 }, { 107, 10379 }, { 108, 10378 }, { 109, 10364 },
	{ 110, 10377 }, { 111, 10383 }, { 112, 10374 }, { 113, 10376 },
	{ 114, 10384 }, { 115, 10373 }, { 116, 10385 }, { 117, 10380 },
	{ 118, 10380 }, { 122, 10392 }, { 123, 10390 }, { 124, 10386 },
	{ 125, 10387 }, { 126, 10389 }, { 127, 10388 }, { 128, 10371 },
	{ 129, 10372 }, { 130, 10366 }, { 131, 10368 }, { 132, 10367 },
	{ 0, 0 }
};

static const RoomDialogueId ROOM_DIALOGUES2_EN[] = {
	{ 102, 221981 }, { 110, 221948 }, { 111, 221968 }, { 107, 222000 },
	{ 101, 221935 }, { 112, 221924 }, { 113, 221942 }, { 116, 221977 },
	{ 124, 221987 }, { 125, 221984 }, { 127, 221991 }, { 128, 221916 },
	{ 129, 221919 }, { 131, 221912 }, { 132, 221908 }, { 0, 0 }
};
static const RoomDialogueId ROOM_DIALOGUES2_DE[] = {
	{ 102, 221981 }, { 110, 221948 }, { 111, 221968 }, { 107, 222000 },
	{ 101, 221935 }, { 112, 221924 }, { 113, 221942 }, { 116, 221977 },
	{ 124, 221987 }, { 125, 221984 }, { 127, 221991 }, { 128, 221916 },
	{ 129, 221919 }, { 131, 221912 }, { 132, 221909 }, { 0, 0 }
};

DoorbotScript::DoorbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	_stateIndex = _doorbotState = 0;

	loadRanges("Ranges/Doorbot");
	loadResponses("Responses/Doorbot");
	setupSentences();
	_tagMappings.load("TagMap/Doorbot");
	_words.load("Words/Doorbot");
	_quotes.load("Quotes/Doorbot");
	_states.load("States/Doorbot");
}

DoorbotScript::~DoorbotScript() {
	for (int idx = 0; idx < 11; ++idx)
		_sentences[idx].clear();
}

void DoorbotScript::setupSentences() {
	for (int idx = 35; idx < 40; ++idx)
		CTrueTalkManager::setFlags(idx, 0);
	_doorbotState = 1;
	_field68 = 0;
	_entryCount = 0;
	_dialValues[0] = _dialValues[1] = 100;

	_mappings.load("Mappings/Doorbot", 4);
	_entries.load("Sentences/Doorbot");

	static const int SENTENCE_NUMS[11] = {
		2, 100, 101, 102, 107, 110, 111, 124, 129, 131, 132
	};
	for (int idx = 0; idx < 11; ++idx) {
		_sentences[idx] = TTsentenceEntries();
		_sentences[idx].load(CString::format("Sentences/Doorbot/%d", SENTENCE_NUMS[idx]));
	}
}

int DoorbotScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	if (tag == MKTAG('D', 'N', 'A', '1') || tag == MKTAG('H', 'H', 'G', 'Q') ||
		tag == MKTAG('A', 'N', 'S', 'W') || tag == MKTAG('S', 'U', 'M', 'S')) {
		if (_stateIndex > 8)
			_stateIndex = 0;
		addResponse(TRANSLATE(STATE_ARRAY_EN[_stateIndex], STATE_ARRAY_DE[_stateIndex]));
		applyResponse();

		if (_stateIndex == 8)
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
		return setResponse(getRandomBit() ? TRANSLATE(11860, 11873)
			: TRANSLATE(11858, 11871));
	} else if (tag == MKTAG('T', 'R', 'A', '3')) {
		return setResponse(getRandomBit() ? TRANSLATE(11859, 11872)
			: TRANSLATE(11857, 11870));
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
	} else if (g_language == Common::DE_DEU && tag == MKTAG('K', 'O', 'H', 'L')) {
		return setResponse(getDialogueId(220976));
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int DoorbotScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	int currState;

	switch (roomScript->_scriptId) {
	case 100:
	case 101:
	case 102:
	case 103:
	case 104:
	case 106:
	case 107:
	case 108:
	case 109:
	case 110:
	case 111:
	case 113:
	case 116:
	case 117:
	case 118:
	case 122:
	case 123:
	case 124:
	case 125:
	case 126:
	case 127:
	case 128:
	case 129:
	case 130:
	case 131:
	case 132:
		break;

	default:
		return 2;
	}

	checkItems(nullptr, nullptr);
	if (getState() == 0) {
		if (CTrueTalkManager::_v2 > getValue(36)) {
			if (getDialRegion(1) == 1 && getRandomBit()) {
				setDialRegion(0, getDialRegion(1) ? 0 : 1);
			} else {
				setDialRegion(1, getDialRegion(1) ? 0 : 1);
			}
			CTrueTalkManager::setFlags(36, CTrueTalkManager::_v2 + 3 + getRandomNumber(5));

			if (getValue(37)) {
				CTrueTalkManager::setFlags(37, 0);
				setState(0);
				return setResponse(getDialogueId(221140));
			}
		}
	}

	if (getValue(35) == 0 && roomScript->_scriptId != 100 && sentence->localWord("parrot")) {
		CTrueTalkManager::setFlags(35, 1);
		setState(0);
		return setResponse(getDialogueId(220113));
	}

	if (sentence->_category == 6 && (sentence->contains("why not") ||
				sentence->contains("warum nicht"))) {
		return setResponse(11871, 8);
	}

	currState = getState();
	if (currState) {
		int sentMode = sentence->_category;
		bool flag1 = sentMode == 11 || sentMode == 13;
		bool flag2 = sentMode == 12;

		switch (currState) {
		case 1:
			if (flag1)
				return setResponse(TRANSLATE(11828, 11841), 2);
			if (flag2)
				return setResponse(TRANSLATE(11827, 11840), 0);
			break;

		case 2:
			if (flag1)
				return setResponse(TRANSLATE(11827, 11840), 0);
			break;

		case 3:
			if (sentMode == 3)
				return setResponse(TRANSLATE(10406, 10417), 0);
			break;

		case 4:
			if (flag1)
				return setResponse(TRANSLATE(11332, 11345), 0);
			if (flag2)
				return setResponse(TRANSLATE(11331, 11344), 0);
			break;

		case 5:
			if (g_language == Common::EN_ANY)
				return setResponse(11868, 0);
			break;

		case 6:
			return setResponse(TRANSLATE(11872, 11885), 7);

		case 7:
			return setResponse(TRANSLATE(11869, 11882), 0);

		case 8:
			return setResponse(TRANSLATE(11870, 11883), 0);

		case 12:
			if (flag1)
				return setResponse(TRANSLATE(11894, 11907), 13);
			if (flag2)
				return setResponse(TRANSLATE(11893, 11906), 13);
			break;

		case 13:
			return setResponse(TRANSLATE(11895, 11908), 12);

		case 15:
			if (sentMode == 3 || sentMode == 6)
				return setResponse(TRANSLATE(10257, 10260), 0);
			break;

		case 16: {
			TTtreeResult treeResult[32];
			if (g_vm->_trueTalkManager->_quotesTree.search(sentence->_normalizedLine.c_str(),
					TREE_3, &treeResult[0], 0, nullptr) != -1)
				return setResponse(getDialogueId(221380), 0);
			break;
		}

		case 17:
			return setResponse(getDialogueId(221126), 0);

		case 18:
			if (flag1)
				return setResponse(getDialogueId(221135), 0);
			if (flag2)
				return setResponse(getDialogueId(221134), 0);
			break;

		case 19:
			if (flag1) {
				if (addRandomResponse(true)) {
					setState(10);
					return 2;
				}
			}
			if (flag2)
				return setResponse(getDialogueId(221966), 0);
			break;

		case 20:
			if (flag1) {
				if (addRandomResponse(true)) {
					setState(19);
					return 2;
				}
			}
			if (flag2 || sentMode == 7 || sentMode == 10) {
				return setResponse(getDialogueId(221879), 0);
			}
			break;

		case 21:
			if (flag2)
				return setResponse(TRANSLATE(10935, 10947), 0);
			break;

		case 22:
			if (flag1) {
				if (getRandomBit()) {
					return setResponse(TRANSLATE(11211, 11223), 23);
				} else {
					return setResponse(TRANSLATE(10127, 10128), 0);
				}
			}
			if (flag2)
				return setResponse(TRANSLATE(10136, 10137), 0);
			break;

		case 23:
			return setResponse(TRANSLATE(10212, 10213), 0);

		case 24:
			if (flag1)
				return setResponse(TRANSLATE(11151, 11163), 0);
			if (flag2)
				return setResponse(TRANSLATE(11150, 11162), 0);
			break;

		case 25:
		case 26:
			if (flag2) {
				if (getRandomBit()) {
					return setResponse(TRANSLATE(11211, 11223), 23);
				} else {
					return setResponse(TRANSLATE(10127, 10128), 0);
				}
			}
			if (flag1)
				return setResponse(TRANSLATE(10136, 10137), 0);
			break;

		case 27:
			if (flag1 || sentence->localWord("did") || sentence->contains("did"))
				return setResponse(getDialogueId(221175), 28);
			break;

		case 28:
			if (flag1 || sentence->localWord("did") || sentence->contains("did"))
				return setResponse(getDialogueId(221176), 29);
			break;

		case 29:
			if (flag1 || sentence->localWord("did") || sentence->contains("did"))
				return setResponse(getDialogueId(221177), 30);
			break;

		case 30:
			return setResponse(getDialogueId(221178), 31);

		case 31:
			if (sentMode == 3 || sentMode == 10)
				return setResponse(TRANSLATE(10350, 10353), 0);
			break;

		case 32:
			return setResponse(TRANSLATE(10110, 10111), 0);

		case 33:
			if (sentence->contains("sieve") || sentence->contains("colander")
					|| sentence->contains("vegetable") || sentence->contains("ground")
					|| sentence->contains("earth") || sentence->contains("garden")
					|| sentence->contains("cheese") || sentence->contains("strainer")
					|| sentence->contains("sieb") || sentence->contains("emmenthaler")
					|| sentence->contains("gemuese") || sentence->contains("kaese")
					|| sentence->contains("erde") || sentence->contains("garten")) {
				return setResponse(getDialogueId(221375), 0);
			} else if (getRandomNumber(100) > 30) {
				return setResponse(getDialogueId(221376), 33);
			} else {
				return setResponse(getDialogueId(221376), 0);
			}
			break;

		case 34:
			if (sentence->localWord("bellbot"))
				return setResponse(TRANSLATE(10094, 10095), 0);
			if (sentence->localWord("doorbot"))
				return setResponse(TRANSLATE(10349, 10352), 0);

			if (g_language == Common::EN_ANY) {
				if (sentence->localWord("deskbot") || sentence->localWord("titania"))
					return setResponse(10148, 0);
				if (sentence->localWord("barbot") || sentence->localWord("rowbot")
					|| sentence->localWord("liftbot") || sentence->localWord("maitredbot"))
					return setResponse(10147, 0);
			}
			break;

		case 35:
			return setResponse(TRANSLATE(10811, 10822), 36);

		case 36:
			if (flag1)
				return setResponse(TRANSLATE(10813, 10824), 37);
			if (flag2)
				return setResponse(TRANSLATE(10812, 10823), 37);
			break;

		case 37:
			if (flag1)
				return setResponse(TRANSLATE(10815, 10826), 37);
			if (flag2)
				return setResponse(TRANSLATE(10814, 10825), 37);
			break;

		case 38:
			if (g_language == Common::EN_ANY)
				return setResponse(10848, 39);
			break;

		case 39:
			if (g_language == Common::EN_ANY)
				return setResponse(10823, 40);
			break;

		case 40:
			return setResponse(TRANSLATE(10832, 10843), 41);

		case 41:
			addResponse(TRANSLATE(10833, 10844));
			return setResponse(TRANSLATE(10835, 10846), 0);

		case 42:
			if (sentence->localWord("please"))
				return setResponse(TRANSLATE(10840, 10851), 43);
			return setResponse(TRANSLATE(10844, 10855), 0);

		case 43:
		case 45:
			return setResponse(TRANSLATE(10844, 10855), 0);

		case 44:
			if (sentence->localWord("thanks"))
				return setResponse(TRANSLATE(10843, 10854), 45);
			return setResponse(TRANSLATE(10844, 10855), 0);

		case 46:
			if (flag1)
				return setResponse(getDialogueId(222251), 0);
			if (g_language == Common::EN_ANY && flag2)
				return setResponse(10713, 0);
			break;

		default:
			break;
		}
	}

	if (currState != 14)
		setState(0);

	if (getDialRegion(1) != 1 && getRandomNumber(100) > 92)
		return setResponse(getDialogueId(221043), 0);

	int result = 0;
	if (g_language == Common::EN_ANY) {
		switch (roomScript->_scriptId) {
		case 100:
		case 101:
		case 102:
		case 107:
		case 110:
		case 111:
		case 124:
		case 129:
		case 131:
		case 132:
			result = processEntries(&_sentences[roomScript->_scriptId], 0, roomScript, sentence);
			break;
		default:
			break;
		}

		if (result == 2)
			return 2;
	}

	if (processEntries(&_entries, _entryCount, roomScript, sentence) == 2
			|| processEntries(_defaultEntries, 0, roomScript, sentence) == 2
			|| defaultProcess(roomScript, sentence))
		return 2;

	if (g_language == Common::DE_DEU && sentence->contains("42")) {
		selectResponse(11831);
		applyResponse();
		return 2;
	}

	switch (sentence->_category) {
	case 11:
		if (getRandomNumber(100) > 90)
			return setResponse(TRANSLATE(10839, 10850), 42);
		return setResponse(getDialogueId(222415), 0);

	case 12:
		if (getRandomNumber(100) > 90)
			return setResponse(TRANSLATE(10841, 10852), 44);
		return setResponse(getDialogueId(222416), 0);

	case 13:
		return setResponse(getDialogueId(222415), 0);

	default:
		if (getRandomNumber(100) > 75 && getStateValue())
			return setResponse(getDialogueId(221095));

		if (processEntries(&_sentences[2], 0, roomScript, sentence) != 2)
			return setResponse(getDialogueId(220000));
		break;
	}

	return 2;
}

ScriptChangedResult DoorbotScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	if (id == 3) {
		if (roomScript != nullptr  && roomScript->_scriptId != 100) {
			if (CTrueTalkManager::_v9 == 101) {
				addResponse(getDialogueId(220873));
				applyResponse();
			} else {
				bool flag = false;
				if (CTrueTalkManager::_currentNPC) {
					CGameObject *obj;
					if (CTrueTalkManager::_currentNPC->find("Magazine", &obj, FIND_PET)) {
						setResponse(getDialogueId(222248), 46);
						flag = true;
					}
				}

				if (!flag) {
					if (getRandomNumber(100) > 80 && getStateValue()) {
						addResponse(getDialogueId(221095));
						applyResponse();
						flag = true;
					}

					if (!flag && (_doorbotState || !fn10(true))) {
						addResponse(getDialogueId(220074));
						applyResponse();
					}
				}
			}
		}

		_doorbotState = 0;
		resetFlags();
		CTrueTalkManager::_v9 = 0;
	} else if (id == 4) {
		setState(0);
		if (getValue(38) == 0) {
			addResponse(getDialogueId(220883));
			applyResponse();
		}

		CTrueTalkManager::setFlags(38, 0);
		CTrueTalkManager::setFlags(39, 0);
	}

	if (id >= 220000 && id <= (uint)TRANSLATE(222418, 222430)) {
		addResponse(getDialogueId(id));
		applyResponse();
	} else if (id >= 10000 && id <= (uint)TRANSLATE(11986, 11999)) {
		addResponse(id);
		applyResponse();
	}

	return SCR_2;
}

int DoorbotScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
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
	case MKTAG('W', 'W', 'E', 'B'):
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
	}
	else if (newId >= 220883) {
		CTrueTalkManager::setFlags(38, 1);
		CTrueTalkManager::triggerAction(28, 0);
	}
	else if (newId >= 220076) {
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
	}
	else if (newId == 220075) {
		if (flag39)
			return getRangeValue(221381);
		CTrueTalkManager::setFlags(39, 1);
	}
	else if (newId == 220038) {
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
		addResponse(TRANSLATE(11195, 11207));
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

int DoorbotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	int id2, id = 0;

	if (g_language == Common::DE_DEU) {
		if (val1 == 4010 || (val1 >= 4012 && val1 <= 4015)) {
			return TTnpcScript::doSentenceEntry(val1, srcIdP, roomScript, sentence);
		}

		if (val1 >= 4009 && val1 <= 4030)
			val1 -= 4000;
	}

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
		break;
	case 6:
		if (getRoom54(132))
			return 1;
		break;
	case 9:
		if (sentence->localWord("my") || sentence->contains("my")
				|| sentence->contains("mein"))
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
		if (!sentence->localWord("my")) {
			if (g_language == Common::EN_ANY && !sentence->contains("my"))
				return 1;
			if (g_language == Common::DE_DEU && !sentence->contains("mein"))
				return 1;
		}
		break;
	case 27:
		if (!sentence->localWord("earth")) {
			if (g_language == Common::EN_ANY && !sentence->contains("earth"))
				return 1;
			if (g_language == Common::EN_ANY && !sentence->contains("erde"))
				return 1;
		}
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
		if (sentence->localWord("another") || sentence->localWord("more")
				|| sentence->localWord("additional") || sentence->contains("another")
				|| sentence->contains("more") || sentence->contains("additional")
				|| sentence->contains("noch ein") || sentence->contains("einen anderen")
				|| sentence->contains("ein anderes") || sentence->contains("eine andere")
				|| sentence->contains("zusaetzliche")) {
			addResponse(getDialogueId(220058));
			applyResponse();
			return 2;
		}
		break;
	case 30:
		if (!sentence->localWord("because") && !sentence->contains("because")
				&& !(g_language == Common::DE_DEU && sentence->contains("well")))
			return 1;
		break;

	case 512:
		if (getValue(4) != 1)
			id = 221157;
		break;
	case 513:
		if (getValue(4) != 2)
			id = 221157;
		break;
	case 514:
		if (getValue(4) != 3)
			id = 221157;
		break;
	case 515:
		if (getValue(4) != 0)
			id = 221157;
		break;
	default:
		break;
	}

	if (id) {
		addResponse(getDialogueId(id));
		applyResponse();
		return 2;
	} else {
		return 0;
	}
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

bool DoorbotScript::randomResponse(uint index) {
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
	const RoomDialogueId *r = TRANSLATE(ROOM_DIALOGUES1_EN, ROOM_DIALOGUES1_DE);
	for (; r->_roomNum; ++r) {
		if (r->_roomNum == roomScript->_scriptId)
			return getDialogueId(r->_dialogueId);
	}

	return 0;
}

int DoorbotScript::getRoomDialogueId2(const TTroomScript *roomScript) {
	const RoomDialogueId *r = TRANSLATE(ROOM_DIALOGUES2_EN, ROOM_DIALOGUES2_DE);
	for (; r->_roomNum; ++r) {
		if (r->_roomNum == roomScript->_scriptId)
			return getDialogueId(r->_dialogueId);
	}

	return 0;
}

} // End of namespace Titanic
