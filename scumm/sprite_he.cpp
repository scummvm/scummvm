/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2005 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "stdafx.h"

#include "scumm/intern.h"
#include "scumm/resource.h"
#include "scumm/scumm.h"
#include "scumm/sprite_he.h"
#include "scumm/wiz_he.h"

namespace Scumm {

void ScummEngine_v90he::allocateArrays() {
	_spriteGroups = (SpriteGroup *)calloc(_numSprites, sizeof(SpriteGroup));
	_spriteTable = (SpriteInfo *)calloc(_numSprites, sizeof(SpriteInfo));
	
	ScummEngine::allocateArrays();
}

int ScummEngine_v90he::spriteInfoGet_flags_1(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_1: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0xE;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_2(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_2: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0xD;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_3(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_3: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0xF;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_4(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_4: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0xC;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_5(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_5: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0x13;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_6(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_6: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags;
	return (val & 0x200000);
}

int ScummEngine_v90he::spriteInfoGet_flags_7(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_7: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0x16;
	return (val & 1);
}

int ScummEngine_v90he::spriteInfoGet_flags_8(int spriteNum) {
	checkRange(_numSprites, 1, spriteNum, "_spriteTableGet_flags_8: Invalid sprite %d");

	int val = _spriteTable[spriteNum].flags >> 0x1E;
	return (val & 1);
}

} // End of namespace Scumm
