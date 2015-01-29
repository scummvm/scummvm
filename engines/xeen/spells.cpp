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

#include "xeen/spells.h"
#include "xeen/files.h"
#include "xeen/resources.h"
#include "xeen/xeen.h"

namespace Xeen {

Spells::Spells(XeenEngine *vm) : _vm(vm) {
	load();
}

void Spells::load() {
	File f1("spells.xen");
	while (f1.pos() < f1.size())
		_spellNames.push_back(f1.readString());
	f1.close();

	File f2("mae.xen");
	while (f2.pos() < f2.size())
		_maeNames.push_back(f2.readString());
	f2.close();
}

int Spells::calcSpellCost(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? (amount * 100) << expenseFactor :
		(amount * -500) << expenseFactor;
}

int Spells::calcSpellPoints(int spellId, int expenseFactor) const {
	int amount = SPELL_COSTS[spellId];
	return (amount >= 0) ? amount : amount * -1 * expenseFactor;
}

} // End of namespace Xeen
