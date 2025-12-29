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

/************************************************************************/
/************************************************************************/
/*																		*/
/*				Global Constants : Lists of things, etc.				*/
/*																		*/
/************************************************************************/
/************************************************************************/

#include "ultima/ultima0/akalabeth.h"
#include "ultima/ultima0/data/data.h"

namespace Ultima {
namespace Ultima0 {

/************************************************************************/
/*																		*/
/*						Return name of object							*/
/*																		*/
/************************************************************************/

const char *GLOObjName(int n) {
	return OBJECT_INFO[n].Name;
}

void GLOGetInfo(int n, int *pDamage, int *pCost, int *pKey) {
	if (pDamage != nullptr) *pDamage = OBJECT_INFO[n].MaxDamage;
	if (pCost != nullptr) 	 *pCost = OBJECT_INFO[n].Cost;
	if (pKey != nullptr) 	 *pKey = OBJECT_INFO[n].Key;
}

/************************************************************************/
/*																		*/
/*						Return name of attribute						*/
/*																		*/
/************************************************************************/

const char *GLOAttribName(int n) {
	return ATTRIB_NAMES[n];
}

/************************************************************************/
/*																		*/
/*							Return name of class						*/
/*																		*/
/************************************************************************/

const char *GLOClassName(char c) {
	return (c == 'F') ? "Fighter" : "Mage";
}

const char *GLOMonsterName(int n) {
	return MONSTER_INFO[n - 1].Name;
}

int GLOMonsterLevel(int n) {
	return MONSTER_INFO[n - 1].Level;
}

} // namespace Ultima0
} // namespace Ultima
