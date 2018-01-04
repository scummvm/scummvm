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
#include "titanic/true_talk/succubus_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/titanic.h"
#include "titanic/translation.h"

namespace Titanic {

SuccUBusScript::SuccUBusScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7),
		_isRoom101(false) {

	loadRanges("Ranges/SuccUBus");
	setupSentences();
}

void SuccUBusScript::setupSentences() {
	_mappings.load("Mappings/SuccUBus", 1);
	_entries.load("Sentences/SuccUBus");
	_field68 = 0;
	_entryCount = 0;
}

int SuccUBusScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	uint dialogueId = tag;

	switch (tag) {
	case MKTAG('S', 'L', 'O', 'W'):
	case MKTAG('T', 'H', 'R', 'T'):
		dialogueId = 70021;
		break;

	case MKTAG('S', 'U', 'C', '1'):
		dialogueId = getDialogueId(230009);
		break;

	case MKTAG('S', 'U', 'C', '2'):
		dialogueId = 70117;
		break;

	case MKTAG('S', 'W', 'E', 'R'):
		dialogueId = getRandomNumber(100) > 40 ? 70103 : getDialogueId(230030);
		break;

	default:
		break;
	}

	if (dialogueId) {
		addResponse(dialogueId);
		applyResponse();
		return 2;
	} else {
		return 1;
	}
}

int SuccUBusScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (!CTrueTalkManager::getStateValue(1))
		return 2;

	if (roomScript && roomScript->_scriptId == 101)
		_isRoom101 = true;

	int currState = getState();
	if (currState) {
		int currMode = sentence->_category;
		bool modeFlag1 = currMode == 11 || currMode == 13;
		bool modeFlag2 = currMode == 12;
		setState(0);

		switch (currState) {
		case 1:
			if (currMode == 3 || currMode == 10)
				return setResponse(70050, 0);
			break;

		case 2:
			if (modeFlag1 || modeFlag2)
				return setResponse(70070 + (getRandomBit() ? 254 : 0), 0);
			break;

		case 3:
			if (currMode == 3 || currMode == 10)
				return setResponse(70074, 0);
			break;

		case 4:
			if (currMode == 4)
				return setResponse(70077, 0);
			if (currMode == 3)
				return setResponse(getDialogueId(230117), 0);
			break;

		case 5:
			if (currMode == 3 || currMode == 10)
				return setResponse(70089, 0);
			break;

		case 6:
			if (modeFlag1)
				return setResponse(70103, 0);
			if (modeFlag2)
				return setResponse(70102, 0);
			break;

		case 7:
			if (modeFlag1)
				return setResponse(getDialogueId(230157), 0);
			break;

		case 8:
			if (modeFlag1)
				return setResponse(getDialogueId(230159), 0);
			break;

		case 9:
			if (modeFlag1)
				return setResponse(getDialogueId(230160), 0);
			break;

		case 10:
			if (modeFlag1)
				return setResponse(getDialogueId(230161), 0);
			break;

		case 11:
			if (modeFlag1)
				return setResponse(getDialogueId(230142), 0);
			break;

		case 12:
			return setResponse(70030, 0);

		default:
			break;
		}
	}

	if (processEntries(&_entries, _entryCount, roomScript, sentence) != 2) {
		uint tagId = g_vm->_trueTalkManager->_quotes.find(sentence->_normalizedLine.c_str());
		if (!tagId || chooseResponse(roomScript, sentence, tagId) != 2) {
			addResponse(getDialogueId(230030));
			applyResponse();
		}
	}

	return 2;
}

ScriptChangedResult SuccUBusScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	if (g_language == Common::EN_ANY) {
		if (id == 148)
			CTrueTalkManager::setFlags(3, 1);
		else if (id == 150)
			CTrueTalkManager::setFlags(2, 1);
	} else {
		if (id == 70211 || id == 230013) {
			addResponse(getDialogueId(230163));
			applyResponse();
			return SCR_2;
		} else if (id < 70211) {
			if (id == 148)
				CTrueTalkManager::setFlags(3, 1);
			else if (id == 150)
				CTrueTalkManager::setFlags(2, 1);
		}
	}

	if (id >= 230000 && id <= 230245) {
		addResponse(getDialogueId(id));
		applyResponse();
	} else if (id >= 70000 && id <= (uint)TRANSLATE(70243, 70248)) {
		addResponse(id);
		applyResponse();
	}

	return SCR_2;
}

int SuccUBusScript::updateState(uint oldId, uint newId, int index) {
	if (newId == 230199) {
		return _isRoom101 ? 230148 : newId;
	} else if (newId >= 230208 && newId <= 230235) {
		addResponse(70158 - getRandomBit());
		return newId;
	} else if (newId >= 230061 && newId <= 230063) {
		if (getValue(2))
			return 230125;
	}

	static const uint UPDATE_STATES[][2] = {
		{ 230078, 1 }, { 230106, 2 }, { 230112, 3 }, { 230115, 4 },
		{ 230127, 5 }, { 230140, 6 }, { 230156, 7 }, { 230157, 8 },
		{ 230159, 9 }, { 230160, 10 }, { 230161, 11 }, { 230072, 12 }
	};

	for (int idx = 0; idx < 12; ++idx) {
		if (UPDATE_STATES[idx][0] == newId) {
			setState(UPDATE_STATES[idx][1]);
			break;
		}
	}

	return newId;
}

int SuccUBusScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	if (val1 == 1 && roomScript && roomScript->_scriptId == 101) {
		addResponse(getDialogueId(230239));
		applyResponse();
		return 2;
	}

	return 0;
}

int SuccUBusScript::setResponse(int dialogueId, int state) {
	addResponse(dialogueId);
	applyResponse();

	if (state != -1)
		setState(state);

	return 2;
}

} // End of namespace Titanic
