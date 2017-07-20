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

#include "common/textconsole.h"

#include "sludge/allfiles.h"
#include "sludge/graphics.h"
#include "sludge/sludge.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"
#include "sludge/newfatal.h"

namespace Sludge {

LoadedSpriteBank *allLoadedBanks = NULL;

LoadedSpriteBank *loadBankForAnim(int ID) {
	LoadedSpriteBank *returnMe = allLoadedBanks;
	while (returnMe) {
		if (returnMe->ID == ID) {
			//debugOut ("loadBankForAnim: Found existing sprite bank with ID %d\n", returnMe -> ID);
			return returnMe;
		}
		returnMe = returnMe->next;
	}
	returnMe = new LoadedSpriteBank;
	if (checkNew(returnMe)) {
		returnMe->ID = ID;
		if (g_sludge->_gfxMan->loadSpriteBank(ID, returnMe->bank, false)) {
			returnMe->timesUsed = 0;
			returnMe->next = allLoadedBanks;
			allLoadedBanks = returnMe;
			debug(kSludgeDebugDataLoad, "loadBankForAnim: New sprite bank created OK");
			return returnMe;
		} else {
			debug(kSludgeDebugDataLoad, "loadBankForAnim: I guess I couldn't load the sprites...");
			return NULL;
		}
	} else
		return NULL;
}

} // End of namespace Sludge
