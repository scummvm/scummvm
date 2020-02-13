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
#include "titanic/true_talk/maitred_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/translation.h"

namespace Titanic {

MaitreDScript::MaitreDScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
	_answerCtr = 0;

	CTrueTalkManager::setFlags(9, 1);
	CTrueTalkManager::setFlags(10, 0);
	CTrueTalkManager::setFlags(11, 0);
	CTrueTalkManager::setFlags(12, 0);
	CTrueTalkManager::setFlags(13, 0);
	CTrueTalkManager::setFlags(14, 0);
	CTrueTalkManager::setFlags(15, 0);
	CTrueTalkManager::setFlags(16, 0);

	loadRanges("Ranges/MaitreD");
	loadResponses("Responses/MaitreD");
	setupSentences();
	_tagMappings.load("TagMap/MaitreD");
	_quotes.load("Quotes/MaitreD");
	_states.load("States/MaitreD");
}

void MaitreDScript::setupSentences() {
	_mappings.load("Mappings/MaitreD", 1);
	_entries.load("Sentences/MaitreD");
	_sentences1.load("Sentences/MaitreD/1");
	_field68 = 0;
	_entryCount = 0;
}

int MaitreDScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	if (tag == MKTAG('F', 'O', 'O', 'D') || tag == MKTAG('F', 'I', 'S', 'H') ||
			tag == MKTAG('C', 'H', 'S', 'E')) {
		addResponse(getDialogueId(260388));
		addResponse(getDialogueId(260659));
		applyResponse();
		return 2;
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int MaitreDScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (roomScript->_scriptId != 132)
		return 2;
	if (preprocess(roomScript, sentence) != 1)
		return 1;

	CTrueTalkManager::setFlags(10, 0);
	setState(0);

	if (getValue(12) == 0) {
		stopFighting(false);
		_answerCtr = 0;

		if (sentence->contains(TRANSLATE("restaurant at the end of the universe",
				"restaurant am ende des universums"))
				|| sentence->contains("milliway")
				|| sentence->contains(TRANSLATE("big bang burger bar", "frittenbude"))) {
			addResponse(getDialogueId(260975));
			applyResponse();
		} else if (processEntries(&_entries, _entryCount, roomScript, sentence) == 2) {
			// Do nothing further
		} else if (sentence->localWord("menu")) {
			addResponse(getDialogueId(260683));
			applyResponse();
		} else if (sentence->localWord("table") && sentence->localWord("other")) {
			addResponse(getDialogueId(260091));
			applyResponse();
		} else if ((sentence->localWord("not") && sentence->localWord("busy"))
				|| (sentence->localWord("no") && sentence->localWord("people"))
				|| sentence->localWord("empty")) {
			addResponse(getDialogueId(260087));
			applyResponse();
		} else if (!defaultProcess(roomScript, sentence)
				&& processEntries(&_sentences1, 0, roomScript, sentence) != 2
				&& processEntries(_defaultEntries, 0, roomScript, sentence) != 2) {
			addResponse(getDialogueId(260975));
			applyResponse();
		}

		return 2;
	}

	if (++_answerCtr > 50 || sentence->localWord("stop") || sentence->localWord("enough")
			|| sentence->contains("i give up") || sentence->contains("i give in")
			|| sentence->contains("i surrender") || sentence->contains("i submit")
			|| sentence->contains("ich gebe auf")
			|| sentence->contains("ich ergebe mich")
			|| sentence->contains("ich kapituliere")
			|| sentence->contains("ich unterwerfe mich")
			|| sentence->contains("hoer auf")
			|| sentence->contains("lass")
			|| sentence->contains("schluss")
			|| sentence->contains("hoer sofort")
			|| sentence->contains("genug")
			|| sentence->contains("basta")
			|| sentence->contains("halt")
			|| sentence->contains("voll")
			|| sentence->contains("was soll")) {
		_answerCtr = 0;
		stopFighting(false);
		addResponse(getDialogueId(260063));
	} else if (sentence->localWord("not") && sentence->localWord("fight") &&
			(sentence->localWord("feel") || sentence->localWord("want")
			|| sentence->localWord("do") || sentence->localWord("will"))) {
		_answerCtr = 0;
		stopFighting(false);
		addResponse(getDialogueId(260678));
	} else if (sentence->contains("touche") || sentence->contains("toushe")) {
		addResponse(getDialogueId(260098));
	} else if (sentence->contains("have at you") || sentence->contains("ausfall")) {
		addResponse(getDialogueId(260047));
	} else if (sentence->contains("en garde") || sentence->contains("on guard")
			|| sentence->contains("attacke")) {
		addResponse(getDialogueId(260008));
	} else if (g_language == Common::DE_DEU && !sentence->contains("ich")
			&& (sentence->contains("surrender") || sentence->contains("gebe auf")
				|| sentence->contains("ergebe mich"))) {
		addResponse(getDialogueId(260086));
	} else if ((sentence->localWord("surrender") && !sentence->contains("i surrender"))
			|| (sentence->contains("give up") && !sentence->contains("i give up"))
			|| (sentence->contains("give in") && !sentence->contains("i give in"))
			|| (sentence->contains("submit") && !sentence->contains("i submit"))) {
		addResponse(getDialogueId(260086));
	} else {
		addResponse(getDialogueId(260131));
	}

	applyResponse();
	return 2;
}

ScriptChangedResult MaitreDScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	resetFlags();
	bool flag1 = false, flag2 = false;

	switch (id) {
	case 3:
		if (getValue(8))
			addResponse(getDialogueId(260655));
		else if (getValue(12))
			addResponse(getDialogueId(260622));
		else if (getValue(9) && getValue(16))
			addResponse(getDialogueId(getValue(16)));
		else if (getValue(15))
			addResponse(getDialogueId(260649));
		else
			addResponse(getDialogueId(260112));

		CTrueTalkManager::setFlags(16, 0);
		CTrueTalkManager::setFlags(15, 1);
		applyResponse();
		break;

	case 110:
		addResponse(getDialogueId(260118));
		applyResponse();
		stopFighting(true);
		CTrueTalkManager::setFlags(8, 1);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 111:
		CTrueTalkManager::setFlags(16, 260680);
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 112:
		addResponse(getDialogueId(getValue(8) ? 260095 : 260127));
		applyResponse();
		break;

	case 113:
		CTrueTalkManager::setFlags(16, 260266);
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 114:
		CTrueTalkManager::setFlags(16, 260267);
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 115:
		CTrueTalkManager::setFlags(16, 260268);
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 116:
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 1);
		break;

	case 117:
		CTrueTalkManager::setFlags(8, 0);
		CTrueTalkManager::setFlags(9, 0);
		startFighting();
		break;

	case 132:
		addResponse(getDialogueId(260655));
		applyResponse();
		break;

	default:
		flag1 = true;
		break;
	}

	if (!getValue(8)) {
		switch (id - 118) {
		case 0:
			addResponse(getDialogueId(260676));
			applyResponse();
			break;
		case 1:
			addResponse(getDialogueId(260677));
			applyResponse();
			break;
		case 2:
			addResponse(getDialogueId(260189));
			applyResponse();
			break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			CTrueTalkManager::setFlags(13, id - 120);
			break;
		case 8:
			CTrueTalkManager::setFlags(13, 0);
			break;
		case 9:
			if (!getValue(12)) {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260120));
				applyResponse();
			} else if (getRandomNumber(4) == 1) {
				addResponse(getDialogueId(260067));
				applyResponse();
			} else {
				addResponse(getDialogueId(260131));
				applyResponse();
			}
			break;

		case 10:
			if (getValue(12) == 0) {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			} else if (getRandomNumber(4) == 1) {
				addResponse(getDialogueId(260077));
				applyResponse();
			} else {
				addResponse(getDialogueId(260131));
				applyResponse();
			}
			break;

		case 11:
			if (getValue(12)) {
				addResponse(getDialogueId(260121));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 12:
			if (getValue(12)) {
				addResponse(getDialogueId(260131));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 13:
			startFighting();
			addResponse(getDialogueId(260131));
			applyResponse();
			break;

		case 15:
			CTrueTalkManager::setFlags(13, 1);
			if (getValue(12)) {
				addResponse(getDialogueId(260122));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 16:
			CTrueTalkManager::setFlags(13, 2);
			if (getValue(12)) {
				addResponse(getDialogueId(260123));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 17:
			CTrueTalkManager::setFlags(13, 3);
			if (getValue(12)) {
				addResponse(getDialogueId(260124));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 18:
			CTrueTalkManager::setFlags(13, 4);
			if (getValue(12)) {
				addResponse(getDialogueId(260125));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		case 19:
			CTrueTalkManager::setFlags(13, 5);
			if (getValue(12)) {
				addResponse(getDialogueId(260126));
				applyResponse();
			} else {
				addResponse(getDialogueId(getValue(14) == 1 ? 260063 : 260119));
				applyResponse();
			}
			break;

		default:
			flag2 = true;
			break;
		}
	}

	return !flag1 || !flag2 ? SCR_2 : SCR_0;
}

int MaitreDScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
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
	case MKTAG('S', 'P', 'R', 'T'):
	case MKTAG('S', 'O', 'N', 'G'):
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
	case MKTAG('B', 'L', 'R', '1'):
	case MKTAG('B', 'L', 'R', '2'):
	case MKTAG('B', 'L', 'T', '1'):
	case MKTAG('B', 'L', 'T', '2'):
	case MKTAG('B', 'L', 'T', '3'):
	case MKTAG('B', 'L', 'T', '4'):
	case MKTAG('B', 'L', 'T', '5'):
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
		tag2 = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
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

int MaitreDScript::updateState(uint oldId, uint newId, int index) {
	if (getValue(8)) {
		if (oldId == 260112)
			return getRangeValue(260654);
		if (oldId != 260655 && oldId != 260654)
			return getRangeValue(260655);
	}

	newId = getStateDialogueId(oldId, newId);

	if (newId == 260023) {
		switch (getValue(13)) {
		case 1:
			newId = 260023; break;
		case 2:
			newId = 260024; break;
		case 3:
			newId = 260025; break;
		case 4:
			newId = 260026; break;
		case 5:
			newId = 260027; break;
		default:
			newId = 260016; break;
		}
	}

	if (newId == 260034) {
		switch (getValue(13)) {
		case 1:
			newId = 260034; break;
		case 2:
			newId = 260035; break;
		case 3:
			newId = 260036; break;
		case 4:
			newId = 260037; break;
		case 5:
			newId = 260038; break;
		default:
			newId = 260045; break;
		}
	}

	if (newId == 260070) {
		switch (getValue(13)) {
		case 1:
			newId = 260070; break;
		case 2:
			newId = 260071; break;
		case 3:
			newId = 260072; break;
		case 4:
			newId = 260073; break;
		case 5:
			newId = 260074; break;
		default:
			newId = 260110; break;
		}
	}

	if (newId == 260076 || newId == 260181 || newId == 261010) {
		CTrueTalkManager::setFlags(14, 1);
		stopFighting(true);
		setFlags10(newId, index);
		return newId;
	}

	if (!getValue(12)) {
		static const uint FIGHTING_IDS[] = {
			260080, 260066, 260067, 260062, 260050, 260087, 260090, 260171, 260173,
			260184, 260193, 260202, 260205, 260220, 260221, 260223, 260231, 260232,
			260365, 260373, 260374, 260387, 260421, 260622, 260695, 0
		};

		for (uint idx = 0; FIGHTING_IDS[idx]; ++idx) {
			if (FIGHTING_IDS[idx] == newId) {
				startFighting();
				break;
			}
		}
	}

	if (newId == 261018) {
		if (getValue(8) == 0 && getValue(9) == 0) {
			newId = getRangeValue(260961);
			setFlags10(newId, index);
			return newId;
		}

		if (getValue(8) == 1 && getValue(9) == 1) {
			newId = getRangeValue(260655);
			setFlags10(newId, index);
			return newId;
		}

		if (getValue(9) && getValue(16)) {
			newId = getValue(16);
			setFlags10(newId, index);
			return index;
		}

		newId = 260989;
	}

	setFlags10(newId, index);
	return newId;
}

int MaitreDScript::preResponse(uint id) {
	if (id == 60911)
		return 260101;

	return 0;
}

uint MaitreDScript::getStateDialogueId(uint oldId, uint newId) {
	if (getValue(8) || getValue(9))
		return newId;

	switch (newId) {
	case 260009:
	case 260010:
	case 260011:
	case 260012:
		return getRangeValue(260961);
	case 260052:
		return 260094;
	case 260203:
		return 260204;
	case 260211:
	case 260212:
	case 260761:
	case 260986:
	case 260987:
	case 260989:
		return getRangeValue(260961);
	case 260263:
	case 260264:
		return 260265;
	case 260411:
		return 260457;
	case 260427:
	case 260053:
	case 260054:
	case 260055:
	case 260056:
	case 260057:
	case 260058:
	case 260059:
	case 260060:
		return 260135;
	case 260698:
	case 260895:
	case 260896:
		return 260457;
	case 260799:
		return 260214;

	default:
		return newId;
	}
}

void MaitreDScript::startFighting() {
	bool isFighting = getValue(12);
	CTrueTalkManager::setFlags(12, 1);

	if (!isFighting) {
		CTrueTalkManager::triggerAction(8, 0);
		resetRange(260121);
		resetRange(260122);
		resetRange(260123);
		resetRange(260124);
		resetRange(260125);
		resetRange(260126);
	}
}

void MaitreDScript::stopFighting(bool flag) {
	bool isFighting = getValue(12);
	CTrueTalkManager::setFlags(12, 0);

	if (isFighting) {
		// Surrender
		CTrueTalkManager::triggerAction(flag ? 10 : 9, 0);
	}
}

void MaitreDScript::setFlags10(uint newId, uint index) {
	int val = 28;
	for (uint idx = 0; idx < _states.size(); ++idx) {
		TTmapEntry &us = _states[idx];
		if (us._src == newId) {
			val = us._dest;
			break;
		}
	}

	CTrueTalkManager::setFlags(10, val);
}

int MaitreDScript::preprocess(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!roomScript || !sentence || getValue(8))
		return 1;

	bool stateFlag = true, applyFlag = false;
	switch (getValue(10)) {
	case 1:
		if (!getValue(11) && !getValue(8)) {
			addResponse(getDialogueId(260052));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 2:
		if (sentence->localWord("change") || sentence->localWord("music")) {
			addResponse(getDialogueId(200684));
			applyFlag = true;
		}
		break;

	case 3:
		if (sentence->localWord("chance") && (sentence->localWord("another")
				|| sentence->localWord("other") || sentence->localWord("more"))) {
			addResponse(getDialogueId(260106));
		} else {
			addResponse(getDialogueId(260107));
		}
		applyFlag = true;
		break;

	case 4:
		if (sentence->contains("unless what")) {
			addResponse(getDialogueId(260099));
		} else {
			addResponse(getDialogueId(260131));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 5:
		addResponse(getDialogueId(260096));
		applyFlag = true;
		stateFlag = false;
		break;

	case 6:
		addResponse(getDialogueId(260097));
		applyFlag = true;
		stateFlag = false;
		break;

	case 7:
		if (sentence->_category == 12) {
			addResponse(getDialogueId(260089));
			applyFlag = true;
			stateFlag = false;
		} else {
			addResponse(getDialogueId(260094));
			applyFlag = true;
			CTrueTalkManager::setFlags(11, 1);
		}
		break;

	case 8:
		if (sentence->_category == 11 || sentence->_category == 13) {
			stopFighting(false);
			addResponse(getDialogueId(260094));
			CTrueTalkManager::setFlags(11, 1);
		} else {
			startFighting();
			addResponse(getDialogueId(260131));
		}
		applyFlag = true;
		break;

	case 9:
		startFighting();
		break;

	case 11:
		if ((sentence->localWord("say") || sentence->localWord("talk")) ||
				sentence->localWord("you")) {
			addResponse(getDialogueId(260216));
			applyFlag = true;
		}
		break;

	case 12:
		if (sentence->localWord("why") && sentence->localWord("naughty")) {
			addResponse(getDialogueId(260196));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("what") && sentence->localWord("his")
				&& sentence->localWord("name")) {
			addResponse(getDialogueId(260197));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("i") && sentence->localWord("meet")) {
			addResponse(getDialogueId(260198));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("i") && sentence->localWord("speak")) {
			addResponse(getDialogueId(260206));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 13:
		if (sentence->localWord("why") || sentence->localWord("please")
				|| sentence->contains("go on") || sentence->localWord("need")
				|| sentence->contains("got to") || sentence->localWord("must")) {
			addResponse(getDialogueId(260199));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 14:
		if (sentence->localWord("what") || sentence->localWord("why")
				|| sentence->localWord("kill")) {
			addResponse(getDialogueId(260200));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("you") && sentence->localWord("kill")) {
			addResponse(getDialogueId(260574));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("how") && sentence->localWord("kill")) {
			addResponse(getDialogueId(260557));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 15:
		if ((sentence->localWord("what") && sentence->localWord("way"))
				|| sentence->localWord("how")) {
			addResponse(getDialogueId(260201));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 16:
		addResponse(getDialogueId(sentence->_category == 11 ? 260209 : 260210));
		applyFlag = true;
		stateFlag = false;
		break;

	case 17:
		if (sentence->localWord("what") && sentence->localWord("mean")) {
			addResponse(getDialogueId(260222));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("laugh") && sentence->localWord("with")
				&& sentence->localWord("you")) {
			addResponse(getDialogueId(260221));
			applyFlag = true;
			stateFlag = false;
		} else {
			startFighting();
			addResponse(getDialogueId(260221));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 18:
		if (sentence->_category == 11) {
			addResponse(getDialogueId(260232));
			applyFlag = true;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(260231));
			applyFlag = true;
		} else if (sentence->_category == 13) {
			addResponse(getDialogueId(260444));
			addResponse(getDialogueId(260233));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("what") && sentence->localWord("happen")) {
			addResponse(getDialogueId(260233));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why") && sentence->localWord("stressed")) {
			addResponse(getDialogueId(260245));
			addResponse(getDialogueId(260233));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why")) {
			addResponse(getDialogueId(260453));
			addResponse(getDialogueId(260233));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 19:
		if ((sentence->localWord("what") && sentence->localWord("scral"))
				|| (sentence->localWord("what") && sentence->localWord("happen"))
				|| sentence->contains("go on") || sentence->contains("and then")) {
			addResponse(getDialogueId(260234));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why") && sentence->localWord("stressed")) {
			addResponse(getDialogueId(260245));
			addResponse(getDialogueId(260234));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why")) {
			addResponse(getDialogueId(260276));
			addResponse(getDialogueId(260237));
			addResponse(getDialogueId(260234));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 20:
		if ((sentence->localWord("what") && sentence->localWord("leovinus"))
				|| (sentence->localWord("what") && sentence->localWord("happen"))) {
			addResponse(getDialogueId(260235));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("where") && sentence->localWord("leovinus")) {
			addResponse(getDialogueId(260236));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why") && sentence->localWord("stressed")) {
			addResponse(getDialogueId(260245));
			addResponse(getDialogueId(260235));
			applyFlag = true;
			stateFlag = false;
		} else {
			addResponse(getDialogueId(260237));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 21:
	case 22:
		if (sentence->contains(TRANSLATE("cooking", "kochen"))
				|| (sentence->localWord("what") && sentence->localWord("mean"))) {
			addResponse(getDialogueId(260238));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("where") && sentence->localWord("now")) {
			addResponse(getDialogueId(260236));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why") && sentence->localWord("stressed")) {
			addResponse(getDialogueId(260245));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why")) {
			addResponse(getDialogueId(260239));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 23:
		if (sentence->_category == 11) {
			addResponse(getDialogueId(260237));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 24:
		if ((sentence->localWord("can") && sentence->localWord("i")
				&& sentence->localWord("have"))
				|| (sentence->localWord("give") && sentence->localWord("me"))
				|| (sentence->localWord("i") && sentence->localWord("want"))
				|| (sentence->localWord("i") && sentence->localWord("need"))
			) {
			addResponse(getDialogueId(260251));
			applyFlag = true;
		}
		break;

	case 25:
		if ((sentence->localWord("open") && sentence->localWord("it"))
				|| (sentence->localWord("how") && sentence->localWord("open"))
				|| (sentence->localWord("how") && sentence->localWord("get") && sentence->localWord("in"))
				|| (sentence->localWord("how") && sentence->localWord("change") && sentence->localWord("music"))
			) {
			addResponse(getDialogueId(260253));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("where") && (sentence->localWord("it")
				|| sentence->localWord("that"))) {
			addResponse(getDialogueId(260252));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("where") && sentence->localWord("key")) {
			addResponse(getDialogueId(260254));
			applyFlag = true;
			stateFlag = false;
		} else if ((sentence->localWord("how") && sentence->localWord("work"))
				|| (sentence->localWord("what") && sentence->localWord("i") && sentence->localWord("do"))) {
			addResponse(getDialogueId(260259));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 26:
		if (sentence->localWord("where") && (sentence->localWord("key")
				|| sentence->localWord("it"))) {
			addResponse(getDialogueId(260254));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("where") && (sentence->localWord("hand")
			|| sentence->localWord("that"))) {
			addResponse(getDialogueId(260256));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(260255));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("why") && sentence->localWord("need")) {
			addResponse(getDialogueId(260685));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 27:
		if (sentence->localWord("where") && (sentence->localWord("that")
				|| sentence->localWord("it"))) {
			addResponse(getDialogueId(260262));
			applyFlag = true;
		}
		break;

	case 28:
		if (sentence->localWord("why")) {
			addResponse(getDialogueId(260386));
			applyFlag = true;
			stateFlag = false;
		} else if (sentence->localWord("insist")) {
			addResponse(getDialogueId(260387));
			applyFlag = true;
			stateFlag = false;
		}
		break;

	case 29:
		if (sentence->_category == 11) {
			startFighting();
			addResponse(getDialogueId(260131));
		} else {
			addResponse(getDialogueId(260966));
		}
		applyFlag = true;
		break;

	case 30:
		if (sentence->_category == 11 || sentence->_category == 13) {
			addResponse(getDialogueId(260695));
			applyFlag = true;
		} else if (sentence->_category == 12) {
			addResponse(getDialogueId(260696));
			applyFlag = true;
		}
		break;

	default:
		break;
	}

	if (applyFlag)
		applyResponse();
	if (stateFlag) {
		setState(0);
		CTrueTalkManager::setFlags(10, 0);
	}

	return applyFlag ? 2 : 1;
}

} // End of namespace Titanic
