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
}

void MaitreDScript::setupSentences() {
	_mappings.load("Mappings/MaitreD", 1);
	_entries.load("Sentences/MaitreD");
	_field68 = 0;
	_entryCount = 0;
}

int MaitreDScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	if (tag == MKTAG('F', 'O', 'O', 'D') || tag == MKTAG('F', 'I', 'S', 'H') ||
			tag == MKTAG('C', 'H', 'S', 'E')) {
		addResponse(getDialogueId(260388));
		addResponse(getDialogueId(260659));
		applyResponse();
		return 2;
	}

	return TTnpcScript::chooseResponse(roomScript, sentence, tag);
}

int MaitreDScript::process(TTroomScript *roomScript, TTsentence *sentence) {
	// TODO
	return 0;
}

ScriptChangedResult MaitreDScript::scriptChanged(TTscriptBase *roomScript, uint id) {
	warning("TODO");
	return SCR_1;
}

bool MaitreDScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const {
	warning("TODO");
	return 0;
}

bool MaitreDScript::proc18() const {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc22(int id) const {
	warning("TODO");
	return 0;
}

int MaitreDScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *MaitreDScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int MaitreDScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void MaitreDScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

} // End of namespace Titanic
