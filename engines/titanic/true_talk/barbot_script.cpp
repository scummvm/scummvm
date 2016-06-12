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
#include "titanic/true_talk/barbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

static const int STATE_ARRAY[7] = {
	0xCAB0, 0xCAB2, 0xCAB3, 0xCAB4, 0xCAB5, 0xCAB6, 0xCAB7
};

static const uint ARRAY1[] = {
	0, 50033, 50044, 50045, 50046, 50047, 50048, 50049,
	50050, 50051, 50034, 50035, 50036, 50037, 50038, 50039,
	50040, 50041, 50042, 50043, 50411, 0
};

static const uint ARRAY2[] = {
	51899, 51900, 51901, 51902, 51903, 51904, 51905, 51906, 51907, 0
};

BarbotScript::BarbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	_state = 0;
	_arrIndex = 0;

	loadRanges("Ranges/Barbot");
	loadResponses("Responses/Barbot");
	setupSentences();
	_tagMappings.load("TagMap/Barbot");
}

void BarbotScript::setupSentences() {
	for (int idx = 28; idx < 35; ++idx)
		CTrueTalkManager::setFlags(idx, 0);
	setupDials(100, 100, 100);

	if (!_field74)
		_field74 = 2;

	_mappings.load("Mappings/Barbot", 8);
	_entries.load("Sentences/Barbot");
}

int BarbotScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	if (tag == MKTAG('D', 'N', 'A', '1') || tag == MKTAG('H', 'H', 'G', 'Q') ||
			tag == MKTAG('A', 'N', 'S', 'W') || tag == MKTAG('S', 'U', 'M', 'S')) {
		if (_state < 7) {
			addResponse(STATE_ARRAY[_state++]);
		} else {
			selectResponse(51896);
			set34(1);
			_state = 0;
		}

		applyResponse();
		return 2;

	} else if (tag == MKTAG('S', 'W', 'E', 'R')) {
		adjustDial(0, -18);
		adjustDial(1, -5);

		if (getRandomNumber(100) > 50) {
			addResponse(getDialogueId(getDialRegion(0) == 0 ? 250200 : 250062));
			applyResponse();
			return 2;
		}

	} else if (tag == MKTAG('B', 'A', 'R', 'K') && getRandomNumber(100) > 50) {
		selectResponse(250025);
		switch (proc23()) {
		case 4:
		case 6:
			addResponse(getDialogueId(250125));
			break;
		default:
			break;
		}

		applyResponse();
		return 2;
	
	} else if (tag == MKTAG('B', 'A', 'R', 'U') && getRandomNumber(100) > 50) {
		selectResponse(250025);
		switch (proc23()) {
		case 4:
		case 6:
			addResponse(getDialogueId(250112));
			break;
		default:
			break;
		}

		applyResponse();
		return 2;	
	}

	if (tag == MKTAG('T', 'H', 'R', 'T') || tag == MKTAG('S', 'L', 'O', 'W') ||
			tag == MKTAG('S', 'E', 'X', '1') || tag == MKTAG('P', 'K', 'U', 'P')) {
		adjustDial(0, -7);
		adjustDial(1, -3);
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int BarbotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	int dialogueId = 0;

	if (roomScript->_scriptId != 112)
		return 2;

	checkItems(roomScript, sentence);
	if (isState9()) {
		if (sentence->localWord("visioncenter") || sentence->localWord("brain") ||
				sentence->contains("vision") || sentence->contains("visual") ||
				sentence->contains("brain") || sentence->contains("crystal")) {
			if (CTrueTalkManager::getStateValue(2)) {
				addResponse(getDialogueId(251003));
				applyResponse();
				CTrueTalkManager::triggerAction(6, 0);
				return 2;
			}
		}

		if (sentence->contains("goldfish")) {
			addResponse(getDialogueId(250184));
			applyResponse();
			return 2;
		}

		dialogueId = ARRAY1[getRandomNumber(20)];
		if (!ARRAY2[_arrIndex])
			_arrIndex = 0;

		if (_arrIndex) {
			dialogueId = ARRAY2[_arrIndex++];
		} else if (getRandomNumber(100) > 35) {
			dialogueId = ARRAY2[0];
			_arrIndex = 1;		
		} else if (getRandomNumber(100) > 60) {
			switch (sentence->_field2C) {
			case 2:
				dialogueId = 51914;
				break;
			case 3:
				dialogueId = 51911;
				break;
			case 4:
				dialogueId = 51913;
				break;
			case 5:
				dialogueId = 51912;
				break;
			case 6:
				dialogueId = 51915;
				break;
			case 7:
				dialogueId = 51909;
				break;
			default:
				break;
			}
		}

		addResponse(dialogueId);
		if (getRandomNumber(100) > 65)
			addResponse(getDialogueId(251250));
		applyResponse();
		return 2;
	}



	// TODO
	return 2;
}

ScriptChangedResult BarbotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int BarbotScript::proc15() const {
	warning("TODO");
	return 0;
}

bool BarbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
	int val, uint tagId, uint remainder) const {
	warning("TODO: handleQuote");
	return false;
}

int BarbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int BarbotScript::proc22(int id) const {
	warning("TODO");
	return 0;
}

int BarbotScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *BarbotScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int BarbotScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void BarbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

void BarbotScript::proc32() {
	warning("TODO");
}

int BarbotScript::proc36(int tagId) const {
	warning("TODO");
	return 0;
}

void BarbotScript::adjustDial(int dialNum, int amount) {
	int level = CLIP(getDialLevel(dialNum) + amount, 0, 100);
	setDial(dialNum, level);
}

bool BarbotScript::isState9() const {
	return CTrueTalkManager::getStateValue(9) != 0;
}

} // End of namespace Titanic
