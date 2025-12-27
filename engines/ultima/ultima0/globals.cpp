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

namespace Ultima {
namespace Ultima0 {

struct _OInfStruct {
	const char *Name; int Cost; int MaxDamage; char Key;
};
struct _MInfStruct {
	const char *Name; int Level;
};

static struct _OInfStruct _OInfo[] = {
	{ "Food", 1, 0, 'F' }, 
	{ "Rapier", 8, 10, 'R' }, 
	{ "Axe", 5, 5, 'A' }, 
	{ "Shield", 6, 1, 'S' }, 
	{ "Bow+Arrow", 3, 4, 'B' }, 
	{ "Amulet", 15, 0, 'M' }
};

static struct _MInfStruct _MInfo[] = {
	{ "Skeleton", 1 }, 
	{ "Thief", 2 }, 
	{ "Giant Rat", 3 }, 
	{ "Orc", 4 }, 
	{ "Viper", 5 }, 
	{ "Carrion Crawler", 6 }, 
	{ "Gremlin", 7 }, 
	{ "Mimic", 8 }, 
	{ "Daemon", 9 }, 
	{ "Balrog", 10 }
};

static const char *_AName[] = { "HP", "Strength", "Dexterity", "Stamina", "Wisdom", "Gold" };

/************************************************************************/
/*																		*/
/*						Return name of object							*/
/*																		*/
/************************************************************************/

const char *GLOObjName(int n) {
	return _OInfo[n].Name;
}

void GLOGetInfo(int n, int *pDamage, int *pCost, int *pKey) {
	if (pDamage != nullptr) *pDamage = _OInfo[n].MaxDamage;
	if (pCost != nullptr) 	 *pCost = _OInfo[n].Cost;
	if (pKey != nullptr) 	 *pKey = _OInfo[n].Key;
}

/************************************************************************/
/*																		*/
/*						Return name of attribute						*/
/*																		*/
/************************************************************************/

const char *GLOAttribName(int n) {
	return _AName[n];
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
	return _MInfo[n - 1].Name;
}

int GLOMonsterLevel(int n) {
	return _MInfo[n - 1].Level;
}

} // namespace Ultima0
} // namespace Ultima
