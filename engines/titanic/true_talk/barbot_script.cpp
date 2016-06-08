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

namespace Titanic {

static const int STATE_ARRAY[7] = {
	0xCAB0, 0xCAB2, 0xCAB3, 0xCAB4, 0xCAB5, 0xCAB6, 0xCAB7
};

BarbotScript::BarbotScript(int val1, const char *charClass, int v2,
		const char *charName, int v3, int val2, int v4, int v5, int v6, int v7) :
		TTnpcScript(val1, charClass, v2, charName, v3, val2, v4, v5, v6, v7) {
	_state = 0;
	loadRanges("Ranges/Barbot");
	loadResponses("Responses/Barbot");
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

void BarbotScript::proc7(int v1, int v2) {
	warning("TODO");
}

int BarbotScript::proc10() const {
	warning("TODO");
	return 0;
}

int BarbotScript::proc15() const {
	warning("TODO");
	return 0;
}

bool BarbotScript::proc16() const {
	warning("TODO");
	return false;
}

bool BarbotScript::proc18() const {
	warning("TODO");
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

uint BarbotScript::translateId(uint id) const {
	warning("TODO");
	return 0;
}

void BarbotScript::adjustDial(int dialNum, int amount) {
	int level = CLIP(getDialLevel(dialNum) + amount, 0, 100);
	setDial(dialNum, level);
}

} // End of namespace Titanic
