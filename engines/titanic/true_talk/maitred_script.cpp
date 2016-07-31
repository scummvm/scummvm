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

namespace Titanic {

MaitreDScript::MaitreDScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0) {
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
	// TODO
	return 0;
}

ScriptChangedResult MaitreDScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int MaitreDScript::handleQuote(const TTroomScript *roomScript, const TTsentence *sentence,
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
	case MKTAG('S', 'P', 'R', 'T'):
	case MKTAG('S', 'O', 'N', 'G'):
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
		tagId = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 'T'):
	case MKTAG('F', 'I', 'L', 'M'):
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

	}

	return TTnpcScript::handleQuote(roomScript, sentence, val, tagId, remainder);
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
		trigger12(true);
		setFlags10(newId, index);
		return newId;
	}

	if (!getValue(12)) {
		static const uint FLAG_IDS[] = {
			260080, 260066, 260067, 260062, 260050, 260087, 260090, 260171, 260173,
			260184, 260193, 260202, 260205, 260220, 260221, 260223, 260231, 260232,
			260365, 260373, 260374, 260387, 260421, 260622, 260695, 0
		};

		for (uint idx = 0; FLAG_IDS[idx]; ++idx) {
			if (FLAG_IDS[idx] == newId) {
				setFlags12();
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

int MaitreDScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	warning("TODO");
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


void MaitreDScript::setFlags12() {
	int val = getValue(12);
	CTrueTalkManager::setFlags(12, 1);

	if (!val) {
		CTrueTalkManager::triggerAction(8, 0);
		resetRange(260121);
		resetRange(260122);
		resetRange(260123);
		resetRange(260124);
		resetRange(260125);
		resetRange(260126);
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

void MaitreDScript::trigger12(bool flag) {
	int val = getValue(12);
	CTrueTalkManager::setFlags(12, 0);

	if (val) {
		CTrueTalkManager::triggerAction(flag ? 10 : 9, 0);
	}
}

} // End of namespace Titanic
