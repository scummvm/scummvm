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
#include "titanic/true_talk/liftbot_script.h"
#include "titanic/true_talk/true_talk_manager.h"

namespace Titanic {

static const int STATE_ARRAY[7] = {
	0x78BE, 0x78C0, 0x78C1, 0x78C2, 0x78C3, 0x78C4, 0x78C5
};

LiftbotScript::LiftbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	_state = 0;

	loadRanges("Ranges/Liftbot");
	loadResponses("Responses/Liftbot");
	setupSentences();
}

void LiftbotScript::setupSentences() {
	CTrueTalkManager::setFlags(27, 0);
	setupDials(getRandomNumber(40) + 60, getRandomNumber(40) + 60, 0);

	_mappings.load("Mappings/Liftbot", 4);
	_entries.load("Sentences/Liftbot");
	_field68 = 0;
	_entryCount = 0;
}

int LiftbotScript::chooseResponse(TTroomScript *roomScript, TTsentence *sentence, uint tag) {
	switch (tag) {
	case MKTAG('D', 'N', 'A', '1'):
	case MKTAG('H', 'H', 'G', 'Q'):
	case MKTAG('A', 'N', 'S', 'W'):
		if (_state >= 7) {
			selectResponse(30918);
			set34(2);
			_state = 0;
		} else {
			addResponse(STATE_ARRAY[_state++]);
		}

		applyResponse();
		return 2;

	case MKTAG('O', 'R', 'D', '8'):
		addResponse(30475);
		addResponse(30467);
		addResponse(30466);
		addResponse(30474);
		applyResponse();
		return SS_2;

	default:
		return TTnpcScript::chooseResponse(roomScript, sentence, tag);
	}
}

void LiftbotScript::proc7(int v1, int v2) {
	warning("TODO");
}

int LiftbotScript::proc9() const {
	warning("TODO");
	return 0;
}

int LiftbotScript::proc10() const {
	warning("TODO");
	return 0;
}

int LiftbotScript::proc15() const {
	warning("TODO");
	return 0;
}

bool LiftbotScript::handleQuote(TTroomScript *roomScript, TTsentence *sentence,
		int val, uint tagId, uint remainder) const {
	warning("TODO");
	return 0;
}

bool LiftbotScript::proc18() const {
	warning("TODO");
	return 0;
}

int LiftbotScript::proc21(int v1, int v2, int v3) {
	warning("TODO");
	return 0;
}

int LiftbotScript::proc22(int id) const {
	warning("TODO");
	return 0;
}

int LiftbotScript::proc23() const {
	warning("TODO");
	return 0;
}

const int *LiftbotScript::getTablePtr(int id) {
	warning("TODO");
	return nullptr;
}

int LiftbotScript::proc25(int val1, int val2, TTroomScript *roomScript, TTsentence *sentence) const {
	warning("TODO");
	return 0;
}

void LiftbotScript::proc26(int v1, const TTsentenceEntry *entry, TTroomScript *roomScript, TTsentence *sentence) {
}

void LiftbotScript::proc32() {
	warning("TODO");
}

uint LiftbotScript::translateId(uint id) const {
	warning("TODO");
	return 0;
}

} // End of namespace Titanic
