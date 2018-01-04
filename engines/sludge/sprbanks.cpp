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
#include "sludge/newfatal.h"
#include "sludge/sludge.h"
#include "sludge/sprites.h"
#include "sludge/sprbanks.h"

namespace Sludge {

LoadedSpriteBank *GraphicsManager::loadBankForAnim(int ID) {
	// Check if already exist
	LoadedSpriteBanks::iterator it;
	for (it = _allLoadedBanks.begin(); it != _allLoadedBanks.end(); ++it) {
		if ((*it)->ID == ID) {
			return (*it);
		}
	}

	// Else create a new sprite bank
	LoadedSpriteBank *returnMe = new LoadedSpriteBank;
	if (checkNew(returnMe)) {
		returnMe->ID = ID;
		if (loadSpriteBank(ID, returnMe->bank, false)) {
			returnMe->timesUsed = 0;
			debugC(3, kSludgeDebugDataLoad, "loadBankForAnim: New sprite bank created OK");
			_allLoadedBanks.push_back(returnMe);
			return returnMe;
		} else {
			debugC(3, kSludgeDebugDataLoad, "loadBankForAnim: I guess I couldn't load the sprites...");
			return nullptr;
		}
	} else
		return nullptr;
}

} // End of namespace Sludge
