/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mm/mm1/data/party.h"
#include "mm/mm1/globals.h"

namespace MM {
namespace MM1 {

#define SHARE_FIELD(FIELD) \
	for (uint i = 0; i < party.size(); ++i) \
		total += party[i].FIELD; \
	avg = total / party.size(); \
	party[0].FIELD = avg + (total % party.size()); \
	for (uint i = 1; i < party.size(); ++i) \
		party[i].FIELD = avg;

void Party::share(TransferKind shareType) {
	auto &party = g_globals->_party;
	int total = 0, avg;

	switch (shareType) {
	case TK_GEMS:
		SHARE_FIELD(_gems);
		break;
	case TK_GOLD:
		SHARE_FIELD(_gold);
		break;
	case TK_FOOD:
		SHARE_FIELD(_food);
		break;
	default:
		break;
	}
}

uint Party::getPartyGold() const {
	uint total = 0;
	for (uint i = 0; i < size(); ++i)
		total += (*this)[i]._gold;

	return total;
}

void Party::clearPartyGold() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._gold = 0;
}

void Party::clearPartyGems() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._gems = 0;
}

void Party::clearPartyFood() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i]._food = 0;
}

void Party::updateAC() {
	for (uint i = 0; i < size(); ++i)
		(*this)[i].updateAC();
}

void Party::combatDone() {
	for (uint i = 0; i < size(); ++i) {
		Character &c = (*this)[i];
		c.updateAttributes();
		c.update58();

		if (!(c._condition & BAD_CONDITION))
			c._condition &= ~(ASLEEP | SILENCED);
	}
}

} // namespace MM1
} // namespace MM
