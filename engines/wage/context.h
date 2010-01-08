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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef WAGE_CONTEXT_H
#define WAGE_CONTEXT_H
 
namespace Wage {

class Context {
	enum StatVariables {
	/** The base physical accuracy of the player. */
		PHYS_ACC_BAS = 0,
	/** The current physical accuracy of the player. */
		PHYS_ACC_CUR = 1,
	/** The base physical armor of the player. */
		PHYS_ARM_BAS = 2,
	/** The current physical armor of the player. */
		PHYS_ARM_CUR = 3,
	/** The base physical hit points of the player. */
		PHYS_HIT_BAS = 4,
	/** The current physical hit points of the player. */
		PHYS_HIT_CUR = 5,
	/** The base physical speed of the player. */
		PHYS_SPE_BAS = 6,
	/** The current physical speed of the player. */
		PHYS_SPE_CUR = 7,
	/** The base physical strength of the player. */
		PHYS_STR_BAS = 8,
	/** The current physical strength of the player. */
		PHYS_STR_CUR = 9,
	/** The base spiritual accuracy of the player. */
		SPIR_ACC_BAS = 10,
	/** The current spiritual accuracy of the player. */
		SPIR_ACC_CUR = 11,
	/** The base spiritual armor of the player. */
		SPIR_ARM_BAS = 12,
	/** The current spiritual armor of the player. */
		SPIR_ARM_CUR = 13,
	/** The base spiritual hit points of the player. */
		SPIR_HIT_BAS = 14,
	/** The current spiritual hit points of the player. */
		SPIR_HIT_CUR = 15,
	/** The base spiritual strength of the player. */
		SPIR_STR_BAS = 16,
	/** The current spiritual strength of the player. */
		SPIR_STR_CUR = 17
	};

private:
	int16 _visits; // Number of scenes visited, including repeated visits
	int16 _kills;  // Number of characters killed
	int16 _experience;
	int16 _userVariables[26 * 9];
	int16 _statVariables[18];

public:
	int16 getUserVariable(int index) { return _userVariables[index]; }
	void setUserVariable(int index, int16 value) { _userVariables[index] = value; }

	int16 getVisits() { return _visits; }
	void setVisits(int16 visits) { _visits = visits; }

	int16 getKills() { return _kills; }
	void setKills(int16 kills) { _kills = kills; }
	
	int16 getExperience() { return _experience; }
	void setExperience(int16 experience) { _experience = experience; }

	int16 getStatVariable(int index) { return _statVariables[index]; }
	void setStatVariable(int index, int16 value) { _statVariables[index] = value; }
};

} // End of namespace Wage
 
#endif
