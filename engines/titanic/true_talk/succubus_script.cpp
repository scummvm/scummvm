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

namespace Titanic {

SuccUBusScript::SuccUBusScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7),
		_field2D0(0) {

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
	// TODO
	return 0;
}

ScriptChangedResult SuccUBusScript::scriptChanged(const TTroomScript *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

int SuccUBusScript::updateState(uint oldId, uint newId, int index) {
	warning("TODO");
	return 0;
}

int SuccUBusScript::doSentenceEntry(int val1, const int *srcIdP, const TTroomScript *roomScript, const TTsentence *sentence) {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic
