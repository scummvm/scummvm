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

AIEntity *AI::spawn(AIType type, AIDir dir, int x, int y, char *funcInit, char *funcAction, char *funcUse, AIDir dir2, int level, int value1, int value2, int callInit) {
	AIEntity *e = new AIEntity;

	e->type = type;
	e->dir = dir;

	// Set Co-ordinates & Speed
	e->x = x * kTileWidth;
	e->tileX = x;
	e->y = y * kTileHeight;
	e->tileY = y;
	e->moveSpeed = kPlayerMoveSpeed; // Default Speed
	if (!g_hdb->getActionMode()) {
		e->moveSpeed /= 2;
	}

	// Other variables
	e->dir2 = dir2;
	if (!level)
		level = 1;
	e->level = level;
	e->value1 = value1;
	e->value2 = value2;
	e->animCycle = 2;	// Game frames to wait before animating graphic frames
	e->animDelay = e->animCycle;
	e->animFrame = 0;

	if (funcInit) {
		strcpy(e->luaFuncInit, funcInit);
	}
	if (funcAction) {
		strcpy(e->luaFuncAction, funcAction);
	}
	if (funcUse) {
		strcpy(e->luaFuncUse, funcUse);
	}

	if (e->luaFuncInit[0] == '*')
		e->luaFuncInit[0] = 0;
	if (e->luaFuncAction[0] == '*')
		e->luaFuncAction[0] = 0;
	if (e->luaFuncUse[0] == '*')
		e->luaFuncUse[0] = 0;

	_ents->push_back(e);

	warning("STUB: AI::spawn: CacheEntGfx required");

	return e;
}

AIEntity *AI::locateEntity(const char *luaName) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if (Common::matchString((*it)->entityName, luaName)) {
			return *it;
		}
	}
	return NULL;
}

void AI::removeEntity(AIEntity *e) {
	_ents->erase(&e);
}

// Initializes each entity after map is loaded
void AI::initAllEnts() {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		(*it)->aiInit((*it));
		if ((*it)->luaFuncInit[0]) {
			if (g_hdb->_lua->callFunction((*it)->luaFuncInit, 2)) {
				strcpy((*it)->entityName, g_hdb->_lua->getStringOffStack());
				strcpy((*it)->printedName, g_hdb->_lua->getStringOffStack());
			} else {
				warning("'%s' doesn't exists", (*it)->luaFuncInit);
			}
		}
	}

	warning("STUB: initAllEnts: Cache graphics for Inventory and Deliveries");
	warning("STUB: initAllEnts: LaserScan required");
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

void AI::lookAtEntity(AIEntity *e) {
	lookAtXY(e->tileX, e->tileY);
}

// Change player direction to XY
void AI::lookAtXY(int x, int y) {
	int distX, distY;

	distX = abs(_player->tileX - x);
	distY = abs(_player->tileY - y);

	if (distX > distY) {
		// X takes precedence
		if (x < _player->tileX) {
			_player->dir = DIR_LEFT;
		} else if (x > _player->tileX) {
			_player->dir = DIR_RIGHT;
		} else if (y < _player->tileY) {
			_player->dir = DIR_UP;
		} else {
			_player->dir = DIR_DOWN;
		}
	} else {
		// Y takes precedence
		if (y < _player->tileY) {
			_player->dir = DIR_UP;
		} else if (y > _player->tileY) {
			_player->dir = DIR_DOWN;
		} else if (x < _player->tileX) {
			_player->dir = DIR_LEFT;
		} else {
			_player->dir = DIR_RIGHT;
		}
	}

	switch (_player->dir) {
	case DIR_UP:
		_player->state = STATE_STANDUP;
		warning("STUB: Set _player->draw to Player standup_gfx");
		break;
	case DIR_DOWN:
		_player->state = STATE_STANDDOWN;
		warning("STUB: Set _player->draw to Player standdown_gfx");
		break;
	case DIR_LEFT:
		_player->state = STATE_STANDLEFT;
		warning("STUB: Set _player->draw to Player standleft_gfx");
		break;
	case DIR_RIGHT:
		_player->state = STATE_STANDRIGHT;
		warning("STUB: Set _player->draw to Player standright_gfx");
		break;
	}
}
} // End of Namespace
