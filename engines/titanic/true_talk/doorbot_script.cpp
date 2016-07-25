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

int DoorbotScript::proc15() const {
	warning("TODO");
	return 0;
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

int DoorbotScript::proc23() const {
	warning("TODO");
	return 0;
}

int DoorbotScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void DoorbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

void DoorbotScript::proc32() {
	warning("TODO");
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

} // End of namespace Titanic
