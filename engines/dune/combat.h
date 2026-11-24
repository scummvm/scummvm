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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef DUNE_COMBAT_H
#define DUNE_COMBAT_H

#include "common/scummsys.h"
#include "common/util.h"

namespace Dune {

// DOS 342D: only the skill addition saturates; doubling the rate wraps AL.
// The four weapon bits add 2, 4, 8 and 16 times the initial worth. 347A
// returns the HIGH byte, with a minimum of one for a nonempty troop.
inline byte combatStrength(byte rate, byte skill, byte equipment, byte men) {
	uint16 worth = (byte)(2 * rate) + skill;
	if (worth > 0xff)
		worth = 0xff;
	uint32 strength = ((uint32)worth * men) >> 4;
	uint32 doubling = strength;
	byte carried = (byte)(equipment << 2);
	for (uint i = 0; i != 4 && strength < 0xffff; i++) {
		doubling <<= 1;
		const bool bit = (carried & 0x80) != 0;
		carried <<= 1;
		if (bit)
			strength += doubling;
	}
	const byte result = (byte)(MIN<uint32>(strength, 0xffff) >> 8);
	return result == 0 && men != 0 ? 1 : result;
}

// DOS 758D: two byte subtractions, then unsigned multiplication. A product
// overflowing AX saturates to FF; losses cannot exceed the troop's size.
inline byte combatLosses(uint16 strength, byte skill, byte men) {
	const byte vulnerability = (byte)(255 - 2 * skill);
	const uint32 damage = ((uint32)strength * vulnerability) >> 8;
	return (byte)MIN<uint32>(MIN<uint32>(damage, 255), men);
}

// DOS 33D9: equal forces give 128; the stronger side approaches 252 and
// the weaker side 4. This is the byte threshold used by battle/report code.
inline byte combatOdds(uint16 allies, uint16 enemies) {
	const uint16 smaller = MIN(allies, enemies);
	const uint16 larger = MAX(allies, enemies);
	const uint32 ratio = smaller ? ((uint32)larger << 8) / smaller / 2 : 252;
	const byte chance = (byte)MIN<uint32>(ratio, 252);
	return allies < enemies ? (byte)-chance : chance;
}

// E3CC/E3B7 return the middle two bytes after multiplying their own DS seed.
// INC affects only AX: a low-word carry must not propagate into the high byte.
inline uint16 combatRandom(uint16 &seed, uint16 multiplier) {
	const uint32 product = (uint32)seed * multiplier;
	seed = (uint16)(product + 1);
	return (uint16)((seed >> 8) | (product & 0x00ff0000) >> 8);
}

} // End of namespace Dune

#endif
