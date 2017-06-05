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
#include "allfiles.h"
#include "sprites.h"
#include "sprbanks.h"
#include "newfatal.h"
#include "debug.h"

namespace Sludge {

loadedSpriteBank *allLoadedBanks = NULL;
extern spriteBank theFont;
extern int loadedFontNum, fontTableSize;

loadedSpriteBank *loadBankForAnim(int ID) {
	//debugOut ("loadBankForAnim: Looking for sprite bank with ID %d\n", ID);
	loadedSpriteBank *returnMe = allLoadedBanks;
	while (returnMe) {
		if (returnMe->ID == ID) {
			//debugOut ("loadBankForAnim: Found existing sprite bank with ID %d\n", returnMe -> ID);
			return returnMe;
		}
		returnMe = returnMe->next;
	}
	returnMe = new loadedSpriteBank;
	//debugOut ("loadBankForAnim: No existing sprite bank with ID %d\n", ID);
	if (checkNew(returnMe)) {
		returnMe->ID = ID;
		if (loadSpriteBank(ID, returnMe->bank, false)) {
			returnMe->timesUsed = 0;
			returnMe->next = allLoadedBanks;
			allLoadedBanks = returnMe;
			debugOut("loadBankForAnim: New sprite bank created OK\n");
			return returnMe;
		} else {
			debugOut("loadBankForAnim: I guess I couldn't load the sprites...\n");
			return NULL;
		}
	} else
		return NULL;
}

void reloadSpriteTextures() {
	loadedSpriteBank *spriteBank = allLoadedBanks;
	while (spriteBank) {
		//fprintf (stderr, "Reloading bank %d: %s.\n", spriteBank->ID, resourceNameFromNum (spriteBank->ID));
		delete spriteBank->bank.sprites;
		spriteBank->bank.sprites = NULL;
		loadSpriteBank(spriteBank->ID, spriteBank->bank, false);
		spriteBank = spriteBank->next;
	}
	if (fontTableSize) {
		delete theFont.sprites;
		theFont.sprites = NULL;
		loadSpriteBank(loadedFontNum, theFont, true);
	}
}

} // End of namespace Sludge
