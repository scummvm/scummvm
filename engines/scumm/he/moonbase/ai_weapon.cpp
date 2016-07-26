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

#include "scumm/he/moonbase/ai_weapon.h"
#include "scumm/he/moonbase/ai_main.h"

namespace Scumm {

Weapon::Weapon(int typeID) { //, float damage, int radius)
	switch (typeID) {
	default:
	case ITEM_BOMB:
		becomeBomb();
		break;

	case ITEM_CLUSTER:
		becomeCluster();
		break;

	case ITEM_CRAWLER:
		becomeCrawler();
		break;

	case ITEM_EMP:
		becomeEMP();
		break;

	case ITEM_SPIKE:
		becomeSpike();
		break;
	}
}

void Weapon::becomeBomb() {
	_typeID = ITEM_BOMB;
	_damage = 3;
	_radius = 30;
	_cost = 1;
}

void Weapon::becomeCluster() {
	_typeID = ITEM_CLUSTER;
	_damage = 1.5;
	_radius = 20;
	_cost = 1;
}

void Weapon::becomeCrawler() {
	_typeID = ITEM_CRAWLER;
	_damage = 4;
	_radius = 180;
	_cost = 7;
}

void Weapon::becomeEMP() {
	_typeID = ITEM_EMP;
	_damage = .1f;
	_radius = 215;
	_cost = 3;
}

void Weapon::becomeSpike() {
	_typeID = ITEM_SPIKE;
	_damage = 6;
	_radius = 180;
	_cost = 3;
}

} // End of namespace Scumm
