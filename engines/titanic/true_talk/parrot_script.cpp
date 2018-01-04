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

#include "titanic/true_talk/parrot_script.h"
#include "titanic/true_talk/true_talk_manager.h"
#include "titanic/titanic.h"
#include "common/textconsole.h"

namespace Titanic {

ParrotScript::ParrotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {

	loadRanges("Ranges/Parrot");
	setupSentences();
}

void ParrotScript::setupSentences() {
	_mappings.load("Mappings/Parrot", 1);
	_entries.load("Sentences/Parrot");
	_field68 = 0;
	_entryCount = 0;
}

int ParrotScript::chooseResponse(const TTroomScript *roomScript, const TTsentence *sentence, uint tag) {
	if (tag == MKTAG('B', 'Y', 'Z', 'A')) {
		addResponse(getDialogueId(280246));
		applyResponse();
		return 2;
	} else {
		return 1;
	}
}

int ParrotScript::process(const TTroomScript *roomScript, const TTsentence *sentence) {
	if (processEntries(roomScript, sentence) != 2) {
		int tagId = g_vm->_trueTalkManager->_quotes.find(sentence->_normalizedLine);
		if (!tagId || chooseResponse(roomScript, sentence, tagId) != 2) {
			addResponse(getDialogueId(sentence->checkCategory() ? 280248 : 280235));
			applyResponse();
		}
	}

	return 2;
}

ScriptChangedResult ParrotScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	if (id >= 280000 && id <= 280276) {
		if (id == 280258) {
			if (CTrueTalkManager::_currentNPC) {
				CGameObject *chicken;
				if (CTrueTalkManager::_currentNPC->find("Chicken", &chicken, FIND_PET))
					id = 280147 - getRandomBit();
			}

			id = getDialogueId(id);
		} else {
			if ((id == 280146 || id == 280147) && CTrueTalkManager::_currentNPC) {
				CGameObject *chicken;
				if (!CTrueTalkManager::_currentNPC->find("Chicken", &chicken, FIND_PET))
					id = 280142;
			}

			addResponse(getDialogueId(id));
			if (id == 280192)
				addResponse(getDialogueId(280222));
			applyResponse();
		}
	}

	if (id >= 80000 && id <= 80244) {
		if ((id == 80155 || id == 80156) && CTrueTalkManager::_currentNPC) {
			CGameObject *chicken;
			if (!CTrueTalkManager::_currentNPC->find("Chicken", &chicken, FIND_PET))
				id = 80151;
		}

		addResponse(id);
		if (id == 80201)
			addResponse(getDialogueId(280222));
		applyResponse();
	}

	return (id == 3) ? SCR_2 : SCR_1;
}

int ParrotScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	return 0;
}

} // End of namespace Titanic
