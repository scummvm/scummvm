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

	e->standdownFrames = e->standupFrames = e->standleftFrames = e->standrightFrames = 0;
	e->movedownFrames = e->moveupFrames = e->moveleftFrames = e->moverightFrames = 0;
	e->blinkFrames = 0;

	if (!cacheEntGfx(e, (bool)callInit)) {
		return NULL;
	} else {
		_ents->push_back(e);
	}

	return e;
}

bool AI::cacheEntGfx(AIEntity *e, bool init) {
	int i = 0;
	while (true) {
		if (aiEntList[i].type == END_AI_TYPES) {
			return false;
		}

		// Load Gfx for corresponding Entity
		if (aiEntList[i].type == e->type) {
			int j = 0;
			AIStateDef *list = aiEntList[i].stateDef;

			while (list[j].state != STATE_ENDSTATES) {

				Common::Array<const char *> *gfxFiles = g_hdb->_fileMan->findFiles(list[j].name, TYPE_TILE32);
				uint32 size;

				for (Common::Array<const char *>::iterator it = gfxFiles->begin(); it != gfxFiles->end(); it++) {
					size = g_hdb->_fileMan->getLength((*it), TYPE_TILE32);

					Tile *gfx = g_hdb->_drawMan->getGfx((*it), size);
					switch (list[j].state) {
					case STATE_STANDDOWN:
						e->standdownGfx[e->standdownFrames] = gfx;
						e->standdownFrames++;
						break;
					case STATE_STANDUP:
						e->standupGfx[e->standupFrames] = gfx;
						e->standupFrames++;
						break;
					case STATE_STANDLEFT:
						e->standleftGfx[e->standleftFrames] = gfx;
						e->standleftFrames++;
						break;
					case STATE_STANDRIGHT:
						e->standrightGfx[e->standrightFrames] = gfx;
						e->standrightFrames++;
						break;
					case STATE_BLINK:
						e->blinkGfx[e->blinkFrames] = gfx;
						e->blinkFrames++;
						break;
					case STATE_MOVEDOWN:
						e->movedownGfx[e->movedownFrames] = gfx;
						e->movedownFrames++;
						break;
					case STATE_MOVEUP:
						e->moveupGfx[e->moveupFrames] = gfx;
						e->moveupFrames++;
						break;
					case STATE_MOVELEFT:
						e->moveleftGfx[e->moveleftFrames] = gfx;
						e->moveleftFrames++;
						break;
					case STATE_MOVERIGHT:
						e->moverightGfx[e->moverightFrames] = gfx;
						e->moverightFrames++;
						break;

					// Special Player Frames
					case STATE_PUSHDOWN:
						_pushdownGfx[_pushdownFrames] = gfx;
						_pushdownFrames++;
						break;
					case STATE_PUSHUP:
						_pushupGfx[_pushupFrames] = gfx;
						_pushupFrames++;
						break;
					case STATE_PUSHLEFT:
						_pushleftGfx[_pushleftFrames] = gfx;
						_pushleftFrames++;
						break;
					case STATE_PUSHRIGHT:
						_pushrightGfx[_pushrightFrames] = gfx;
						_pushrightFrames++;
						break;
					case STATE_GRABUP:
						_getGfx[DIR_UP] = gfx; break;
					case STATE_GRABDOWN:
						_getGfx[DIR_DOWN] = gfx; break;
					case STATE_GRABLEFT:
						_getGfx[DIR_LEFT] = gfx; break;
					case STATE_GRABRIGHT:
						_getGfx[DIR_RIGHT] = gfx; break;

					case STATE_ATK_CLUB_UP:
						_clubUpGfx[_clubUpFrames] = gfx;
						_clubUpFrames++;
						break;
					case STATE_ATK_CLUB_DOWN:
						_clubDownGfx[_clubDownFrames] = gfx;
						_clubDownFrames++;
						break;
					case STATE_ATK_CLUB_LEFT:
						_clubLeftGfx[_clubLeftFrames] = gfx;
						_clubLeftFrames++;
						break;
					case STATE_ATK_CLUB_RIGHT:
						_clubRightGfx[_clubRightFrames] = gfx;
						_clubRightFrames++;
						break;

					case STATE_ATK_STUN_UP:
						_stunUpGfx[_stunUpFrames] = gfx;
						_stunUpFrames++;
						break;
					case STATE_ATK_STUN_DOWN:
						_stunDownGfx[_stunDownFrames] = gfx;
						_stunDownFrames++;
						break;
					case STATE_ATK_STUN_LEFT:
						_stunLeftGfx[_stunLeftFrames] = gfx;
						_stunLeftFrames++;
						break;
					case STATE_ATK_STUN_RIGHT:
						_stunRightGfx[_stunRightFrames] = gfx;
						_stunRightFrames++;
						break;

					case STATE_ATK_SLUG_UP:
						_slugUpGfx[_slugUpFrames] = gfx;
						_slugUpFrames++;
						break;
					case STATE_ATK_SLUG_DOWN:
						_slugDownGfx[_slugDownFrames] = gfx;
						_slugDownFrames++;
						break;
					case STATE_ATK_SLUG_LEFT:
						_slugLeftGfx[_slugLeftFrames] = gfx;
						_slugLeftFrames++;
						break;
					case STATE_ATK_SLUG_RIGHT:
						_slugRightGfx[_slugRightFrames] = gfx;
						_slugRightFrames++;
						break;

					// Maintenance Bot
					case STATE_USEUP:
						e->standupGfx[4] = gfx;
						break;
					case STATE_USEDOWN:
						e->standdownGfx[4] = gfx;
						break;
					case STATE_USELEFT:
						e->standleftGfx[4] = gfx;
						break;
					case STATE_USERIGHT:
						e->standrightGfx[4] = gfx;
						break;

					// Death & Dying for Player
					case STATE_DYING:
						_dyingGfx[_dyingFrames] = gfx;
						_dyingFrames++;
						break;
					case STATE_GOODJOB:
						_goodjobGfx = gfx;
						break;

					case STATE_HORRIBLE1:
						_horrible1Gfx[_horrible1Frames] = gfx;
						_horrible1Frames++;
						break;
					case STATE_HORRIBLE2:
						_horrible2Gfx[_horrible2Frames] = gfx;
						_horrible2Frames++;
						break;
					case STATE_HORRIBLE3:
						_horrible3Gfx[_horrible3Frames] = gfx;
						_horrible3Frames++;
						break;
					case STATE_HORRIBLE4:
						_horrible4Gfx[_horrible4Frames] = gfx;
						_horrible4Frames++;
						break;
					case STATE_PLUMMET:
						_plummetGfx[_plummetFrames] = gfx;
						_plummetFrames++;
						break;

					// floating frames - overwrite "standup" info
					case STATE_FLOATING:
						e->blinkGfx[e->blinkFrames] = gfx;
						e->blinkFrames++;
						break;

					// melted frames - go in the special area (lightbarrels)
					// shocking frames - go in the special1 area (shockbots)
					// exploding frames, same
					case STATE_MELTED:
					case STATE_SHOCKING:
					case STATE_EXPLODING:
						e->special1Gfx[e->special1Frames] = gfx;
						e->special1Frames++;
						break;

					// ICEPUFF spawning states
					case STATE_ICEP_PEEK:
						e->blinkGfx[e->blinkFrames] = gfx;
						e->blinkFrames++;
						break;
					case STATE_ICEP_APPEAR:
						e->standupGfx[e->standupFrames] = gfx;
						e->standupFrames++;
						break;
					case STATE_ICEP_THROWDOWN:
						e->standdownGfx[e->standdownFrames] = gfx;
						e->standdownFrames++;
						break;
					case STATE_ICEP_THROWRIGHT:
						e->standrightGfx[e->standrightFrames] = gfx;
						e->standrightFrames++;
						break;
					case STATE_ICEP_THROWLEFT:
						e->standleftGfx[e->standleftFrames] = gfx;
						e->standleftFrames++;
						break;
					case STATE_ICEP_DISAPPEAR:
						e->special1Gfx[e->special1Frames] = gfx;
						e->special1Frames++;
						break;

					// FATFROG spawning states
					case STATE_LICKDOWN:
						e->movedownGfx[e->movedownFrames] = gfx;
						e->movedownFrames++;
						break;
					case STATE_LICKLEFT:
						e->moveleftGfx[e->moveleftFrames] = gfx;
						e->moveleftFrames++;
						break;
					case STATE_LICKRIGHT:
						e->moverightGfx[e->moverightFrames] = gfx;
						e->moverightFrames++;
						break;

					// MEERKAT spawning states
					case STATE_MEER_MOVE:
						e->standdownGfx[e->standdownFrames] = gfx;
						e->standdownFrames++;
						break;
					case STATE_MEER_APPEAR:
						e->standleftGfx[e->standleftFrames] = gfx;
						e->standleftFrames++;
						break;
					case STATE_MEER_BITE:
						e->standrightGfx[e->standrightFrames] = gfx;
						e->standrightFrames++;
						break;
					case STATE_MEER_DISAPPEAR:
						e->standupGfx[e->standupFrames] = gfx;
						e->standupFrames++;
						break;
					case STATE_MEER_LOOK:
						e->movedownGfx[e->movedownFrames] = gfx;
						e->movedownFrames++;
						break;

					// DIVERTER spawning states
					case STATE_DIVERTER_BL:
						e->standdownGfx[e->standdownFrames] = gfx;
						e->standdownFrames++;
						break;
					case STATE_DIVERTER_BR:
						e->standupGfx[e->standupFrames] = gfx;
						e->standupFrames++;
						break;
					case STATE_DIVERTER_TL:
						e->standleftGfx[e->standleftFrames] = gfx;
						e->standleftFrames++;
						break;
					case STATE_DIVERTER_TR:
						e->standrightGfx[e->standrightFrames] = gfx;
						e->standrightFrames++;
						break;
					// DOLLY states
					// angry[4] = standright[4]
					// kissright[4]/kissleft[4] = standleft[8]
					// panic[4]/laugh[4] = standdown[8]
					// dollyuseright[5] = special1[5]
					case STATE_ANGRY:
						e->standrightGfx[e->standrightFrames] = gfx;
						e->standrightFrames++;
						break;
					case STATE_KISSRIGHT:
						e->standleftGfx[e->standleftFrames] = gfx;
						e->standleftFrames++;
						break;
					case STATE_KISSLEFT:
						e->standleftGfx[4 + e->int1] = gfx;
						e->int1++;
						break;
					case STATE_PANIC:
						e->standdownGfx[e->standdownFrames] = gfx;
						e->standdownFrames++;
						break;
					case STATE_LAUGH:
						e->standdownGfx[4 + e->value1] = gfx;
						e->value1++;
						break;
					case STATE_DOLLYUSERIGHT:
						e->special1Gfx[e->special1Frames] = gfx;
						e->special1Frames++;
						break;

					// SARGE yelling
					case STATE_YELL:
						e->special1Gfx[e->special1Frames] = gfx;
						e->special1Frames++;
						break;
					}
				}
				j++;
			}

			e->aiInit = aiEntList[i].initFunc;
			e->aiInit2 = aiEntList[i].initFunc2;
			if (init) {
				e->aiInit(e);
				if (e->aiInit2) {
					e->aiInit2(e);
				}
				if (e->luaFuncInit[0]) {
					g_hdb->_lua->callFunction(e->luaFuncInit, 2);

					const char *str1 = g_hdb->_lua->getStringOffStack();
					const char *str2 = g_hdb->_lua->getStringOffStack();
					if (str1) {
						strcpy(e->entityName, str1);
					}
					if (str2) {
						strcpy(e->printedName, str2);
					}
				}
			} else {
				if (e->aiInit2) {
					e->aiInit2(e);
				}
			}

			break; // Entity Initiated
		}
		i++;
	}
	return true;
}

// Stops the movement of an entity
void AI::stopEntity(AIEntity *e) {
	if (e == _player) {
		clearWaypoints();
		// Reset Player speed
		e->moveSpeed = kPlayerMoveSpeed;
	}

	// Reset animation
	e->animFrame = 0;

	// Align with tile boundaries
	e->x = e->tileX * kTileWidth;
	e->y = e->tileY * kTileHeight;
	e->goalX = e->tileX;
	e->goalY = e->tileY;
	e->drawXOff = e->drawYOff = 0;

	// Don't change the state of Diverters or Floating entities
	switch (e->state) {
	case STATE_FLOATLEFT:
	case STATE_FLOATRIGHT:
	case STATE_FLOATUP:
	case STATE_FLOATDOWN:
		e->state = STATE_FLOATING;
		return;
	}

	if (e->type != AI_DIVERTER) {
		switch (e->dir) {
		case DIR_UP:
			if (e->standupFrames)
				e->state = STATE_STANDUP;
			else
				e->state = STATE_NONE;
			break;
		case DIR_DOWN:
			if (e->standdownFrames)
				e->state = STATE_STANDDOWN;
			else
				e->state = STATE_NONE;
			break;
		case DIR_LEFT:
			if (e->standleftFrames)
				e->state = STATE_STANDLEFT;
			else
				e->state = STATE_NONE;
			break;
		case DIR_RIGHT:
			if (e->standrightFrames)
				e->state = STATE_STANDRIGHT;
			else
				e->state = STATE_NONE;
			break;
		}
	}
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

void AI::setEntityGoal(AIEntity *e, int x, int y) {
	int xv, yv;

	e->xVel = e->yVel = 0;

	xv = x - e->tileX;
	if (xv < 0) {
		e->xVel = -e->moveSpeed;
		e->state = STATE_MOVELEFT;
		e->dir = DIR_LEFT;
	} else if (xv > 0) {
		e->xVel = e->moveSpeed;
		e->state = STATE_MOVERIGHT;
		e->dir = DIR_RIGHT;
	}

	yv = y - e->tileY;
	if (yv < 0) {
		e->yVel = -e->moveSpeed;
		e->state = STATE_MOVELEFT;
		e->dir = DIR_LEFT;
	} else if (yv > 0) {
		e->yVel = e->moveSpeed;
		e->state = STATE_MOVERIGHT;
		e->dir = DIR_RIGHT;
	}

	if (e->type == AI_GUY && _playerRunning) {
		e->xVel = e->xVel << 1;
		e->yVel = e->yVel << 1;
	}

	e->goalX = x;
	e->goalY = y;
	e->animFrame = 0;
	e->drawXOff = e->drawYOff = 0;
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

	for (int i = 0; i < _numInventory; i++) {
		AIEntity *temp = &_inventory[i].ent;

		// Clear out all ptrs in entity before writing
		for (int j = 0; i < kMaxAnimFrames; i++) {
			temp->blinkGfx[j] = NULL;
			temp->movedownGfx[j] = NULL;
			temp->moveupGfx[j] = NULL;
			temp->moveleftGfx[j] = NULL;
			temp->moverightGfx[j] = NULL;
			temp->standdownGfx[j] = NULL;
			temp->standupGfx[j] = NULL;
			temp->standleftGfx[j] = NULL;
			temp->standrightGfx[j] = NULL;
			temp->special1Gfx[j] = NULL;
		}

		temp->blinkFrames = 0;
		temp->movedownFrames = 0;
		temp->moveupFrames = 0;
		temp->moveleftFrames = 0;
		temp->moverightFrames = 0;
		temp->standdownFrames = 0;
		temp->standupFrames = 0;
		temp->standleftFrames = 0;
		temp->standrightFrames = 0;

		temp->draw = NULL;
		temp->aiDraw = NULL;
		temp->aiAction = temp->aiInit = temp->aiUse = NULL;

		cacheEntGfx(temp, false);
	}

	warning("STUB: initAllEnts: Cache graphics for Deliveries");
	warning("STUB: initAllEnts: LaserScan required");
}

void AI::animLuaEntity(const char *initName, AIState st) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if (Common::matchString((*it)->entityName, initName)) {
			(*it)->state = st;
			(*it)->animFrame = 0;
			(*it)->animDelay = (*it)->animCycle;
		}
	}
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
