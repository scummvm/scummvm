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
#include "titanic/true_talk/bellbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/pet_control/pet_control.h"
#include "titanic/core/node_item.h"
#include "titanic/titanic.h"

namespace Titanic {

uint BellbotScript::_oldId;

BellbotScript::BellbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, -1, -1, -1, 0),
		_field2D0(0), _field2D4(0), _field2D8(0), _field2DC(0) {
	CTrueTalkManager::setFlags(25, 0);
	CTrueTalkManager::setFlags(24, 0);
	CTrueTalkManager::setFlags(40, 0);
	CTrueTalkManager::setFlags(26, 0);

	setupDials(0, 0, 0);
	_array[0] = 100;
	_array[1] = 0;

	loadRanges("Ranges/Bellbot");
	loadResponses("Responses/Bellbot", 4);
	setupSentences();
	_tagMappings.load("TagMap/Bellbot");
	_words.load("Words/Bellbot");
	_quotes.load("Quotes/Bellbot");
	_states.load("States/Bellbot");
	_preResponses.load("PreResponses/Bellbot");
}

void BellbotScript::setupSentences() {
	_mappings.load("Mappings/Bellbot", 1);
	_entries.load("Sentences/Bellbot");
	_field2DC = 0;
	_field68 = 0;
	_entryCount = 0;
}

int BellbotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult BellbotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int BellbotScript::proc15() const {
	warning("TODO");
	return 0;
}

int BellbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
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
	case MKTAG('B', 'L', 'R', '1'):
	case MKTAG('B', 'L', 'R', '2'):
	case MKTAG('B', 'L', 'T', '1'):
	case MKTAG('B', 'L', 'T', '2'):
	case MKTAG('B', 'L', 'T', '3'):
	case MKTAG('B', 'L', 'T', '4'):
	case MKTAG('B', 'L', 'T', '5'):
	case MKTAG('C', 'O', 'P', 'S'):
	case MKTAG('D', 'C', 'T', 'R'):
	case MKTAG('F', 'A', 'S', 'H'):
	case MKTAG('F', 'A', 'M', 'E'):
	case MKTAG('H', 'E', 'R', 'D'):
	case MKTAG('H', 'O', 'S', 'T'):
	case MKTAG('K', 'N', 'O', 'B'):
	case MKTAG('N', 'H', 'R', 'O'):
	case MKTAG('R', 'A', 'C', 'E'):
	case MKTAG('S', 'C', 'I', 'T'):
	case MKTAG('T', 'O', 'U', 'P'):
	case MKTAG('T', 'W', 'A', 'T'):
	case MKTAG('W', 'E', 'A', 'T'):
		tagId = MKTAG('P', 'R', 'S', 'N');
		break;
	case MKTAG('C', 'H', 'S', 'E'):
	case MKTAG('C', 'M', 'N', 't'):
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

int BellbotScript::updateState(uint oldId, uint newId, int index) {
	if (!getValue(25)) {
		newId = 202043 - getValue(1) <= 2 ? 994 : 0;
		CTrueTalkManager::setFlags(25, 1);
	}

	if (oldId == _oldId && _rangeResetCtr >= 3) {
		TTscriptRange *range = findRange(oldId);
		if (range)
			range->_priorIndex = 0;

		_rangeResetCtr = 0;
		return getRangeValue(200370);
	}

	if (oldId != _oldId) {
		_oldId = oldId;
		_rangeResetCtr = 0;
	}

	if (oldId >= 201709 && oldId <= 201754) {
		addResponse(getDialogueId(201705));
		addResponse(getDialogueId(201706));
		newId = getRangeValue(201707);
	}

	if (newId == 202276)
		newId = addLocation();
	if (newId == 202275)
		newId = getStateDialogueId();

	if (getValue(1) >= 2) {
		if (newId == 200840 || newId == 200845 || newId == 200846 || newId == 200851) {
			if (getValue(1) == 2) {
				newId = 202047;
			} else {
				newId = getRangeValue(202848);
			}
		}
	}

	if (getValue(1) >= 3) {
		if (newId == 200841 || newId == 200842 || newId == 200843 ||
				newId == 200847 || newId == 200848 || newId == 200854) {
			newId = getRangeValue(202038);
		}
	}

	if (newId == 200264 && getValue(1) == 1)
		newId = 200267;
	if (newId == 202231 && getValue(1) == 1)
		newId = 200848;

	int v4 = getValue(4);
	if (newId == 200187 && v4) {
		return 200188;
	} else if (newId == 200188 && !v4) {
		return 200187;
	} else if (newId == 200014 && (v4 == 1 || v4 == 2)) {
		return 200011;
	} else if (newId == 200011 && !v4) {
		return 200014;
	}

	if (oldId == 200612) {
		CTrueTalkManager::setFlags(25, 2);
		CTrueTalkManager::setFlags(5, 1);
	}

	if (newId == 200423 || newId == 200424 || newId == 200425) {
		if (getValue(5)) {
			CTrueTalkManager::triggerAction(16, 0);
		} else {
			newId = 200611;
		}
	}

	if (oldId == 200261 && getRandomNumber(10) == 1) {
		if (getValue(1) >= 3)
			newId = getRangeValue(200283);
		else if (getValue(1) == 2)
			newId = getRangeValue(200279);
	}

	if (oldId == 200962) {
		if (getValue(1) == 2)
			return 200963;
		if (getValue(1) == 1)
			return 200964;
	}
	if (oldId == 200989 && getValue(1) <= 2)
		return 200990;

	if (oldId == 201760) {
		CGameManager *gameManager = g_vm->_trueTalkManager->getGameManager();
		CPetControl *pet = getPetControl(gameManager);

		if (pet) {
			bool canSummon = pet->canSummonBot("DoorBot");
			if (canSummon) {
				CTrueTalkManager::_v9 = 101;
				CTrueTalkManager::triggerAction(5, 0);
			} else {
				newId = 201857;
			}
		}
	}

	setValue23(newId);
	return newId;
}

int BellbotScript::preResponse(uint id) {
	int newId = _preResponses.find(id);

	if (newId == 202277) {
		applyResponse();
		CTrueTalkManager::triggerAction(1, 0);
	}
	if (newId == 200769) {
		applyResponse();
		CTrueTalkManager::triggerAction(18, 0);
	}

	if (id == 21790)
		CTrueTalkManager::triggerAction(13, 0);

	return newId;
}

int BellbotScript::proc23() const {
	warning("TODO");
	return 0;
}

int BellbotScript::proc25(int val1, const int *srcIdP, TTroomScript *roomScript, TTsentence *sentence) {
	switch (val1) {
	case 1:
		addResponse(getDialogueId(*srcIdP));
		applyResponse();
		return 2;

	case 2:
		addResponse(getDialogueId(*srcIdP));
		addResponse(getDialogueId(getRandomNumber(2) == 1 ? 200192 : 200157));
		addResponse(getDialogueId(200176));
		applyResponse();
		return 2;

	case 21:
		if (CTrueTalkManager::getStateValue(7) == 0) {
			selectResponse(21372);
			applyResponse();
			return 2;
		}

		if (!sentence->localWord("broken") && !sentence->contains("broken") &&
				CTrueTalkManager::_currentNPC) {
			CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
			if (node) {
				CString nodeName = node->getName();
				if (nodeName == "5" || nodeName == "6" || nodeName == "7") {
					CTrueTalkManager::triggerAction(29, 2);
					selectResponse(201571);
					applyResponse();
					return 2;
				}
			}
		}

		CTrueTalkManager::triggerAction(29, 1);
		selectResponse(201771);
		applyResponse();
		return 2;

	case 22:
		if (CTrueTalkManager::getStateValue(7) == 0) {
			selectResponse(21372);
			applyResponse();
			return 2;
		}

		if (!sentence->localWord("broken") && !sentence->contains("broken") &&
			CTrueTalkManager::_currentNPC) {
			CNodeItem *node = CTrueTalkManager::_currentNPC->getNode();
			if (node) {
				CString nodeName = node->getName();
				if (nodeName == "5" || nodeName == "6" || nodeName != "7") {
					CTrueTalkManager::triggerAction(29, 2);
					selectResponse(201571);
					applyResponse();
					return 2;
				}
			}
		}

		CTrueTalkManager::triggerAction(29, 1);
		selectResponse(201771);
		applyResponse();
		return 2;

	case 23:
	case 24:
		if (CTrueTalkManager::getStateValue(7) == 0) {
			selectResponse(21372);
			applyResponse();
			return 2;
		}

		CTrueTalkManager::triggerAction(29, val1 == 23 ? 3 : 4);
		break;

	default:
		break;
	}

	return 0;
}

int BellbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

int BellbotScript::addLocation() {
	addResponse(getDialogueId(202228));
	int roomNum, floorNum, elevatorNum;
	getAssignedRoom(&roomNum, &floorNum, &elevatorNum);

	addResponse(getDialogueId(202071 + roomNum));
	addResponse(getDialogueId(201933 + floorNum));
	addResponse(getDialogueId(201916 + elevatorNum));

	return 200858;
}

int BellbotScript::getStateDialogueId() const {
	switch (getValue(1)) {
	case 1:
		return 201253;
	case 2:
		return 200282;
	default:
		return 201246;
	}
}

void BellbotScript::setValue23(uint id) {
	uint val = 0;
	for (uint idx = 0; idx < _states.size() && !val; ++idx) {
		TTmapEntry &us = _states[idx];
		if (us._src == id)
			val = us._dest;
	}

	CTrueTalkManager::setFlags(23, val);
}

} // End of namespace Titanic
