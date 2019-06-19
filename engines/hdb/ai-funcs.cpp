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

#include "hdb/hdb.h"

namespace HDB {

AIEntity *AI::locateEntity(const char *luaName) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if (Common::matchString((*it)->entityName, luaName)) {
			return *it;
		}
	}
	return NULL;
}

// Check to see if we can get this entity
bool AI::getTableEnt(AIType type) {
	switch (type) {
	case ITEM_CELL:
	case ITEM_ENV_WHITE:
	case ITEM_ENV_RED:
	case ITEM_ENV_BLUE:
	case ITEM_ENV_GREEN:
	case ITEM_TRANSCEIVER:
	case ITEM_CLUB:
	case ITEM_ROBOSTUNNER:
	case ITEM_SLUGSLINGER:
	case ITEM_MONKEYSTONE:
	case ITEM_GOO_CUP:
	case ITEM_TEACUP:
	case ITEM_BURGER:
	case ITEM_PDA:
	case ITEM_BOOK:
	case ITEM_CLIPBOARD:
	case ITEM_NOTE:
	case ITEM_KEYCARD_WHITE:
	case ITEM_KEYCARD_BLUE:
	case ITEM_KEYCARD_RED:
	case ITEM_KEYCARD_GREEN:
	case ITEM_KEYCARD_PURPLE:
	case ITEM_KEYCARD_BLACK:
	case ITEM_SEED:
	case ITEM_SODA:
	case ITEM_SLICER:
	case ITEM_DOLLYTOOL1:
	case ITEM_DOLLYTOOL2:
	case ITEM_DOLLYTOOL3:
	case ITEM_DOLLYTOOL4:
		return true;
	default:
		return false;
	}
}

// Check to see if it's okay to move through this entity
bool AI::walkThroughEnt(AIType type) {
	switch (type) {
	case AI_VORTEXIAN:
	case AI_MEERKAT:
	case AI_GOODFAIRY:
	case AI_BADFAIRY:
	case AI_GATEPUDDLE:
	case AI_BUZZFLY:
	case AI_OMNIBOT:
	case AI_PUSHBOT:
	case AI_TURNBOT:
	case AI_RIGHTBOT:

	case ITEM_GEM_WHITE:
	case ITEM_GEM_BLUE:
	case ITEM_GEM_RED:
	case ITEM_GEM_GREEN:
		return true;
	default:
		return getTableEnt(type);
	}
}

// Play special sound for every item you get
void AI::getItemSound(AIType type) {
	warning("STUB: AI: getItemSound required");
}
} // End of Namespace
