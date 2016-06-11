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
#include "titanic/true_talk/parrot_script.h"

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

int ParrotScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	if (tag == MKTAG('B', 'Y', 'Z', 'A')) {
		addResponse(getDialogueId(280246));
		applyResponse();
		return 2;
	} else {
		return 1;
	}
}

int ParrotScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult ParrotScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

bool ParrotScript::proc18() const {
	warning("TODO");
	return 0;
}

int ParrotScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *ParrotScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int ParrotScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void ParrotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

} // End of namespace Titanic
