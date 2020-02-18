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

#ifndef ULTIMA8_WORLD_WEAPONINFO_H
#define ULTIMA8_WORLD_WEAPONINFO_H


struct WeaponInfo {
	uint32 _shape;
	uint8 _overlayType;
	uint32 _overlayShape;
	uint8 _damageModifier;
	uint8 _baseDamage;
	uint8 _dexAttackBonus;
	uint8 _dexDefendBonus;
	uint8 _armourBonus;
	uint16 _damageType;
	int _treasureChance;

	enum DmgType {
		DMG_NORMAL = 0x0001,
		DMG_BLADE  = 0x0002,
		DMG_BLUNT  = 0x0004,
		DMG_FIRE   = 0x0008,
		DMG_UNDEAD = 0x0010,
		DMG_MAGIC  = 0x0020,
		DMG_SLAYER = 0x0040,
		DMG_PIERCE = 0x0080,
		DMG_FALLING = 0x0100
	};
};


#endif
