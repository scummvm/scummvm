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

#ifndef SCUMM_HE_MOONBASE_AI_WEAPON_H
#define SCUMM_HE_MOONBASE_AI_WEAPON_H

namespace Scumm {

class Weapon {
private:
	int _typeID;
	float _damage;
	int _radius;
	int _cost;

public:
	Weapon() {}
	Weapon(int typeID);
	virtual ~Weapon() {}

	void setTypeID(int typeID) { _typeID = typeID; }
	void setDamage(float damage) { _damage = damage; }
	void setRadius(int radius) { _radius = radius; }
	void setCost(int cost) { _cost = cost; }

	int getTypeID() { return _typeID; }
	float getDamage() { return _damage; }
	int getRadius() { return _radius; }
	int getCost() { return _cost; }

	void becomeBomb();
	void becomeCluster();
	void becomeCrawler();
	void becomeEMP();
	void becomeSpike();
};

} // End of namespace Scumm

#endif
