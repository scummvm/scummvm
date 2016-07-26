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
#include "titanic/core/node_item.h"

namespace Titanic {

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

bool BellbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
	int val, uint tagId, uint remainder) const {
	warning("TODO: handleQuote");
	return false;
}

int BellbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int BellbotScript::proc22(int id) const {
	warning("TODO");
	return 0;
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

void BellbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

int BellbotScript::proc36(int id) const {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic
