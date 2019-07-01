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

AIEntity *AI::spawn(AIType type, AIDir dir, int x, int y, const char *funcInit, const char *funcAction, const char *funcUse, AIDir dir2, int level, int value1, int value2, int callInit) {
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

					if (g_hdb->_gfx->selectGfxType((*it))) {
						Tile *gfx = g_hdb->_gfx->getTileGfx((*it), size);

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
						default:
							warning("AI-FUNCS: cacheEntGfx: Unintended State");
							break;
						}
					} else {
						Picture *gfx = g_hdb->_gfx->getPicGfx((*it), size);

						switch (list[j].state) {
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
						default:
							warning("AI-FUNCS: cacheEntGfx: Unintended State");
							break;
						}
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
	e->goalX = e->goalY = e->xVel = e->yVel = 0;

	// Don't change the state of Diverters or Floating entities
	switch (e->state) {
	case STATE_FLOATLEFT:
	case STATE_FLOATRIGHT:
	case STATE_FLOATUP:
	case STATE_FLOATDOWN:
		e->state = STATE_FLOATING;
		return;
	default:
		warning("AI-FUNCS: stopEntity: Unintended State");
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
		default:
			warning("AI-FUNCS: stopEntity: DIR_NONE");
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

AIEntity *AI::findEntity(int x, int y) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y) {
			return *it;
		}
	}

	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y) {
			return *it;
		}
	}

	warning("STUB: findEntity: Check for Laser");
	return NULL;
}

AIEntity *AI::findEntityIgnore(int x, int y, AIEntity *ignore) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y && (*it) != ignore) {
			return *it;
		}
	}

	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y && (*it) != ignore) {
			return *it;
		}
	}

	warning("STUB: findEntityIgnore: Check for Laser");
	return NULL;
}

AIEntity *AI::findEntityType(AIType type, int x, int y) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y && (*it)->type == type) {
			return *it;
		}
	}

	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y && (*it)->type == type) {
			return *it;
		}
	}

	warning("STUB: findEntityType: Check for Laser");
	return NULL;
}

void AI::getEntityXY(const char *entName, int *x, int *y) {
	AIEntity *e;
	HereT *h;

	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = *it;
		if (e->entityName && !scumm_stricmp(entName, e->entityName)) {
			*x = e->tileX;
			*y = e->tileY;
			return;
		}
	}

	for (Common::Array<AIEntity *>::iterator jt = _floats->begin(); jt != _floats->end(); jt++) {
		e = *jt;
		if (e->entityName && !scumm_stricmp(entName, e->entityName)) {
			*x = e->tileX;
			*y = e->tileY;
			return;
		}
	}

	for (Common::Array<HereT *>::iterator kt = _hereList->begin(); kt != _hereList->end(); kt++) {
		h = *kt;
		if (!scumm_stricmp(entName, h->entName)) {
			*x = h->x;
			*y = h->y;
			return;
		}
	}
}

bool AI::useLuaEntity(const char *initName) {
	AIEntity *e;

	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = *it;
		if (e->entityName && !scumm_stricmp(initName, e->entityName)) {
			e->aiUse(e);
			checkActionList(e, e->tileX, e->tileY, true);
			if (e->luaFuncUse)
				g_hdb->_lua->callFunction(e->luaFuncUse, 0);
			return true;
		}
	}

	// Check _actions list for activation as well
	for (int i = 0; i < kMaxActions; i++) {
		if (_actions[i].entityName && !scumm_stricmp(initName, _actions[i].entityName)) {
			checkActionList(&_dummyPlayer, _actions[i].x1, _actions[i].y1, false);
			checkActionList(&_dummyPlayer, _actions[i].x2, _actions[i].y2, false);
		}
	}

	return false;
}

void AI::removeLuaEntity(const char *initName) {
	AIEntity *e;

	for (uint i = 0; i < _ents->size(); i++) {
		e = *(_ents->begin() + i);
		if (e->entityName && !scumm_stricmp(initName, e->entityName)) {
			removeEntity(*(_ents->begin() + i));
			i--;
		}
	}
}

void AI::animLuaEntity(const char *initName, AIState st) {
	AIEntity *e;

	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = *it;
		if (e->entityName && !scumm_stricmp(initName, e->entityName)) {
			e->state = st;
			e->animFrame = 0;
			e->animDelay = e->animCycle;
		}
	}
}

void AI::setLuaAnimFrame(const char *initName, AIState st, int frame) {
	AIEntity *e;

	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = *it;
		if (e->entityName && !scumm_stricmp(initName, e->entityName)) {
			e->state = st;
			e->animFrame = frame;
			e->animDelay = e->animCycle;
			animEntFrames(e);
			e->state = STATE_NONE;
		}
	}
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
		e->state = STATE_MOVEUP;
		e->dir = DIR_UP;
	} else if (yv > 0) {
		e->yVel = e->moveSpeed;
		e->state = STATE_MOVEDOWN;
		e->dir = DIR_DOWN;
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
		AIEntity *temp = _inventory[i].ent;

		// Clear out all ptrs in entity before writing
		for (int j = 0; j < kMaxAnimFrames; j++) {
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

void AI::killPlayer(Death method) {

	int x = _player->x, y = _player->y;

	stopEntity(_player);
	_player->x = x;
	_player->y = y;
	_playerInvisible = false;
	_playerDead = true;

	warning("STUB: killPlayer: Close Dialogs");

	switch (method) {
	case DEATH_NORMAL:
		_player->state = STATE_DYING;
		warning("STUB: killPlayer: Play SND_GUY_DYING");
		break;
	case DEATH_FRIED:
		_player->state = STATE_HORRIBLE1;
		warning("STUB: killPlayer: Play SND_GUY_FRIED");
		break;
	case DEATH_SHOCKED:
		_player->state = STATE_HORRIBLE2;
		warning("STUB: killPlayer: Play SND_GUY_DYING");
		warning("STUB: killPlayer: Play SND_SHOCKBOT_SHOCK");
		break;
	case DEATH_GRABBED:
		_player->state = STATE_HORRIBLE3;
		warning("STUB: killPlayer: Play SND_GUY_GRABBED");
		break;
	case DEATH_DROWNED:
		_player->state = STATE_HORRIBLE4;
		warning("STUB: killPlayer: Play SND_GUY_DROWN");
		break;
	case DEATH_PANICZONE:
		_player->state = STATE_DYING;
		warning("STUB: killPlayer: Play SND_PANIC_DEATH");
		break;
	case DEATH_PLUMMET:
		_player->state = STATE_PLUMMET;
		warning("STUB: killPlayer: Play SND_GUY_PLUMMET");
		break;
	}

	warning("STUB: killPlayer: Stop Music");
}

void AI::stunEnemy(AIEntity *e, int time) {
	int ns = 0;

	if (e->stunnedWait)
		ns = 1;

	e->stunnedWait = g_hdb->getTimeSlice() + 1000 * time;

	// Already stunned? If not, play sound
	if (!ns)
		switch (e->type) {
		case AI_BUZZFLY:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_PUSHBOT:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_MEERKAT:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_FATFROG:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_OMNIBOT:
		case AI_SHOCKBOT:
		case AI_LISTENBOT:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_GOODFAIRY:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_BADFAIRY:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_ICEPUFF:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_RIGHTBOT:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_BOOMBARREL:
			warning("STUB: stunEnemy: Play sound");
			break;
		case AI_CHICKEN:	// Fall through
			warning("STUB: stunEnemy: Play sound");
		default:
			warning("STUB: stunEnemy: Play sound");
			break;
		}
}

/*
	Note from original:
	Moves the entity along toward its goal, sets current frame to draw
	depending on its current state.  Special checking is done for the
	player in here to move him along his waypoints.
*/
void AI::animateEntity(AIEntity *e) {

	int bgTileFlags, bgTileIndex;
	int fgTileFlags;
	bool result;
	uint64 flags;

	// Move entity if player is not dead
	debug(3, "Before animateEntity, e->x: %d, e->y: %d", e->x, e->y);
	debug(3, "Before animateEntity, e->tileX: %d, e->tileY: %d", e->tileX, e->tileY);
	if (!_playerDead) {
		e->x += e->xVel;
		e->y += e->yVel;
		e->tileX = e->x / kTileWidth;
		e->tileY = e->y / kTileHeight;
		debug(3, "After animateEntity, e->x: %d, e->y: %d", e->x, e->y);
		debug(3, "After animateEntity, e->tileX: %d, e->tileY: %d", e->tileX, e->tileY);
	}

	// For non-players, check for trigger being hit
	if (onEvenTile(e->x, e->y)) {
		// Check if a trigger is hit
		checkTriggerList(e->entityName, e->tileX, e->tileY);

		/*
			For Non-Players only
			are we on a touchplate?
			Barrels, Crates, Magic Egg & Ice Block ONLY
			standing on a Touchplate will activate
			something WHILE standing on it
		*/
		int bgtile;
		switch (e->type) {
		case AI_CRATE:
		case AI_BOOMBARREL:
		case AI_HEAVYBARREL:
		case AI_LIGHTBARREL:
		case AI_MAGIC_EGG:
		case AI_ICE_BLOCK:
		case AI_FROGSTATUE:
			bgtile = g_hdb->_ai->checkForTouchplate(e->tileX, e->tileY);
			if (bgtile && !e->touchpWait && e->touchpX != e->tileX && e->touchpY != e->tileY) {
				if (g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false)) {
					e->touchpTile = bgtile;
					e->touchpX = e->tileX;
					e->touchpY = e->tileY;
					e->touchpWait = kPlayerTouchPWait;
				}
			}
			warning("STUB: animateEntity: Set laser_rescan to true");
			break;
		default:
			debug(9, "animateEntity: Unintended Type");
			break;
		}

		// Are we on ice?
		bgTileFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
		fgTileFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);

		if (e->level == 1 ? (bgTileFlags & kFlagIce) : ((bgTileFlags & kFlagIce) && !(fgTileFlags & kFlagGrating))) {
			int xva[] = {9, 0, 0, -1, 1}, yva[] = {9, -1, 1, 0, 0};
			int nx, ny, moveOK = 0;
			AIEntity *hit;

			// Types allowed to slide on ice...
			switch (e->type) {
			case AI_GUY:
			case AI_CHICKEN:
			case AI_TURNBOT:
			case AI_RIGHTBOT:
			case AI_PUSHBOT:
			case AI_CRATE:
			case AI_LIGHTBARREL:
			case AI_HEAVYBARREL:
			case AI_BOOMBARREL:
			case AI_MAGIC_EGG:
			case AI_ICE_BLOCK:
			case AI_DIVERTER:
				e->moveSpeed = kPlayerMoveSpeed << 1;
				nx = e->tileX + xva[e->dir];
				ny = e->tileY + yva[e->dir];
				hit = legalMove(nx, ny, e->level, &moveOK);
				bgTileFlags = g_hdb->_map->getMapBGTileFlags(nx, ny);
				if (hit)
					switch (hit->type) {
					case ITEM_GEM_WHITE:
					case ITEM_GEM_BLUE:
					case ITEM_GEM_GREEN:
					case ITEM_GEM_RED:
					case AI_GOODFAIRY:
					case AI_BADFAIRY:
						hit = NULL;
						break;
					default:
						debug(9, "animateEntity: Unintended type");
						break;
					}
				if ((!hit && moveOK) || (bgTileFlags & kFlagPlayerDie))
					setEntityGoal(e, nx, ny);

				if (e == _player) {
					_playerOnIce = true;
					clearWaypoints();
				}
				break;
			default:
				debug(9, "animateEntity: Unintended type");
				break;
			}
		} else if (e == _player)
			_playerOnIce = false;

		/*
			Player only
			are we trying to walk into a solid tile?
			first, let's make sure we're perfectly aligned on
			a tile boundary before the check so we don't snap
			the player back into position...

			if we're on a waypoint, nevermind!
		*/
		if (e == _player) {
			result = e->x == (e->goalX * kTileWidth) && e->y == (e->goalY * kTileWidth);
			if (!result) {
				int xv = 0, yv = 0;
				switch (e->dir) {
				case DIR_UP:	yv = -1;	break;
				case DIR_DOWN:	yv = 1;		break;
				case DIR_LEFT:	xv = -1;	break;
				case DIR_RIGHT: xv = 1;		break;
				case DIR_NONE:	warning("animateEntity: DIR_NONE found"); break;
				}

				bgTileFlags = g_hdb->_map->getMapBGTileFlags(e->tileX + xv, e->tileY + yv);
				fgTileFlags = g_hdb->_map->getMapFGTileFlags(e->tileX + xv, e->tileY + yv);
				if ((bgTileFlags & kFlagSolid) && !(fgTileFlags & kFlagGrating))
					stopEntity(e);
			}
		}
	}

	// If player, then scroll the screen with the player
	if (e == _player && !_playerDead) {
		if (!_cameraLock)
			g_hdb->_map->centerMapXY(e->x + 16, e->y + 16);

		// Check if player walked into teleporter
		checkTeleportList(e, e->tileX, e->tileY);

		// Check for bad tiles (DEATH)
		int cx = (e->x + 16) / kTileWidth;
		int cy = (e->y + 16) / kTileHeight;
		bgTileFlags = g_hdb->_map->getMapBGTileFlags(cx, cy);
		fgTileFlags = g_hdb->_map->getMapFGTileFlags(cx, cy);
		if ((bgTileFlags & kFlagPlayerDie) && !(checkFloating(cx, cy)) && !(fgTileFlags & kFlagGrating)) {
			if (bgTileFlags & kFlagEnergyFloor)
				killPlayer(DEATH_SHOCKED);
			else if ((bgTileFlags & kFlagPlasmaFloor) || (bgTileFlags & kFlagRadFloor))
				killPlayer(DEATH_FRIED);
			else
				killPlayer(DEATH_NORMAL);
			return;
		}

		// Check if player wants to stop
		// If yes, sets last waypoint right in front of player
		int xOff = 0, yOff = 0;
		if (_numWaypoints > 1) {

			switch (e->dir) {
			case DIR_UP:
				xOff = 0;
				yOff = -1;
				break;
			case DIR_DOWN:
				xOff = 0;
				yOff = 1;
				break;
			case DIR_LEFT:
				xOff = -1;
				yOff = 0;
				break;
			case DIR_RIGHT:
				xOff = 1;
				yOff = 0;
				break;
			case DIR_NONE:
				warning("AI-FUNCS: animateEntity: DIR_NONE found");
				break;
			}
			if ((e->tileX + xOff == _waypoints[_numWaypoints - 1].x &&
				 e->tileY + yOff == _waypoints[_numWaypoints - 1].y) &&
				 e->level == _waypoints[_numWaypoints - 1].level) {
				memset(&_waypoints[0], 0, sizeof(_waypoints));
				_numWaypoints = 1;
				_waypoints[0].x = e->tileX + xOff;
				_waypoints[0].y = e->tileY + yOff;
				_waypoints[0].level = e->level;
				e->goalX = e->tileX + xOff;
				e->goalY = e->tileY + yOff;
			}
		}
	}

	// Check for moving up/down stair levels
	bgTileIndex = g_hdb->_map->getMapBGTileIndex(e->tileX, e->tileY);
	bgTileFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
	fgTileFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
	if (bgTileFlags & kFlagStairTop)
		e->level = 2;
	else if (bgTileFlags & kFlagStairBot)
		e->level = 1;

	// Reached goal?
	// Cinematic require less accuracy for NPCs
	if (_cineActive && e != _player)
		result = (abs(e->x - (e->goalX * kTileWidth)) <= abs(e->xVel)) && (abs(e->y - (e->goalY * kTileHeight)) <= abs(e->yVel));
	else
		result = (e->x == e->goalX * kTileWidth) && (e->y == e->goalY * kTileHeight);

	if (result) {
		// If player, this is a waypoint goal.
		// Drop one waypoint from list
		if (e == _player) {
			removeFirstWaypoint();
			_playerEmerging = false;
		}

		// If entity not player, stop it here
		// If entity is player and no waypoints are left, stop it here
		if (e != _player || (!_numWaypoints && e == _player)) {
			e->tileX = e->goalX;
			e->tileY = e->goalY;

			uint16 buttons = g_hdb->_input->getButtons();

			if (e == _player && (buttons & (kButtonUp | kButtonDown | kButtonLeft | kButtonRight))) {
				int xva[] = {9, 0, 0, -1, 1}, yva[] = {9, -1, 1, 0, 0};
				int nx, ny, result2;

				if (e->state != STATE_PUSHRIGHT && e->state != STATE_PUSHLEFT && e->state != STATE_PUSHUP && e->state != STATE_PUSHDOWN) {
					if (buttons & kButtonUp)
						e->dir = DIR_UP;
					else if (buttons & kButtonDown)
						e->dir = DIR_DOWN;
					else if (buttons & kButtonLeft)
						e->dir = DIR_LEFT;
					else if (buttons & kButtonRight)
						e->dir = DIR_RIGHT;

					nx = e->tileX + xva[e->dir];
					ny = e->tileY + yva[e->dir];
					AIEntity *hit = legalMove(nx, ny, e->level, &result2);
					if (!hit && result2) {
						switch (e->dir) {
						case DIR_UP:	e->goalY = ny;	e->xVel = 0; e->yVel = -kPlayerMoveSpeed; e->state = STATE_MOVEUP; break;
						case DIR_DOWN:	e->goalY = ny;  e->xVel = 0; e->yVel =  kPlayerMoveSpeed; e->state = STATE_MOVEDOWN; break;
						case DIR_LEFT:	e->goalX = nx;  e->yVel = 0; e->xVel = -kPlayerMoveSpeed; e->state = STATE_MOVELEFT; break;
						case DIR_RIGHT: e->goalX = nx;  e->yVel = 0; e->xVel =  kPlayerMoveSpeed; e->state = STATE_MOVERIGHT; break;
						case DIR_NONE:	warning("animateEntity: DIR_NONE found"); break;
						}
						if (_playerRunning) {
							e->xVel = e->xVel << 1;
							e->yVel = e->yVel << 1;
						}
					} else
						stopEntity(e);
				} else
					stopEntity(e);
			} else
				stopEntity(e);

			// Handle lasers after entity has stopped
			switch (e->type) {
			case AI_GUY:
			case AI_CRATE:
			case AI_LIGHTBARREL:
			case AI_HEAVYBARREL:
			case AI_BOOMBARREL:
			case AI_MAGIC_EGG:
			case AI_ICE_BLOCK:
			case AI_DIVERTER:
				warning("STUB: animateEntity: Set _laserRescan to true");
				break;
			default:
				debug(9, "animateEntity: Unintended State");
			}

			// Checking at the Destination

			// Can this entity float and it is over-water
			if (((flags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY)) & kFlagWater) && (e->type == AI_CRATE || e->type == AI_LIGHTBARREL || e->type == AI_BOOMBARREL || e->type == AI_HEAVYBARREL || e->type == AI_FROGSTATUE || e->type == AI_DIVERTER)) {
				// On a grating and level2?
				if ((g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY) & kFlagGrating) && e->level == 2) {
					animEntFrames(e);
					return;
				}

				// If it fell in slime
				// If it is a light barrel on a melting floor
				// If it is supposed to slide across the floor
				// If it is being pushed on a floating entity, don't float it
				if (flags & kFlagSlime) {
					// unless its a Heavy Barrel in which case it floats in slime
					if ((e->type == AI_CRATE || e->type == AI_HEAVYBARREL) && !checkFloating(e->tileX, e->tileY)) {
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_SLIME_SPLASH_SIT);
						floatEntity(e, STATE_FLOATING);
						warning("STUB: animateEntity: Play SND_SPLASH");
					} else if (!checkFloating(e->tileX, e->tileY)) {
						if (e->type == AI_BOOMBARREL) {
							aiBarrelExplode(e);
							aiBarrelBlowup(e, e->tileX, e->tileY);
							return;
						} else {
							addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
							removeEntity(e);
							warning("STUB: animateEntity: Play SND_BARREL_MELTING");
						}
					}
				} else if ((flags & kFlagLightMelt) && e->type == AI_LIGHTBARREL) {
					if (!checkFloating(e->tileX, e->tileY)) {
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
						floatEntity(e, STATE_MELTED);
						warning("STUB: animateEntity: Play SND_BARREL_MELTING");
					}
				} else if (flags & kFlagSlide) {
					int xv = 0, yv = 0;
					AIEntity *hit;

					switch (e->dir) {
					case DIR_UP:	yv = -1; break;
					case DIR_DOWN:	yv = 1; break;
					case DIR_LEFT:	xv = -1; break;
					case DIR_RIGHT: xv = 1; break;
					case DIR_NONE:	warning("animateEntity: DIR_NONE found"); break;
					}

					hit = findEntityIgnore(e->tileX + xv, e->tileY + yv, &_dummyLaser);
					if (!hit) {
						e->state = STATE_SLIDING;
						if (flags & kFlagAnimFast)
							e->moveSpeed = kPlayerMoveSpeed << 1;
						else if (flags & kFlagAnimSlow)
							e->moveSpeed = kPlayerMoveSpeed >> 1;
						setEntityGoal(e, e->tileX + xv, e->tileY + yv);
						warning("STUB: animateEntity: Play SND_LIGHT_SLIDE");
					}

				} else if (!checkFloating(e->tileX, e->tileY)) {
					if (e->type == AI_BOOMBARREL || e->type == AI_HEAVYBARREL || e->type == AI_FROGSTATUE || e->type == AI_DIVERTER) {
						// Make it disappear in the water
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_WATER_SPLASH_SIT);
						removeEntity(e);
						warning("STUB: animateEntity: Play SND_SPLASH");
						return;
					} else {
						// Make it float and splash in water
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_WATER_SPLASH_SIT);
						floatEntity(e, STATE_FLOATING);
						warning("STUB: animateEntity: Play SND_SPLASH");
						return;
					}

				// If it is floating downstream, keep moving it
				if (flags & (kFlagPushRight | kFlagPushLeft | kFlagPushUp | kFlagPushDown)) {
					int xv = 0, yv = 0;
					AIState state;

					if (flags & kFlagPushRight) {
						e->dir = DIR_RIGHT;
						xv = 1;
						state = STATE_FLOATRIGHT;
					} else if (flags & kFlagPushLeft) {
						e->dir = DIR_LEFT;
						xv = -1;
						state = STATE_FLOATLEFT;
					} else if (flags & kFlagPushUp) {
						e->dir = DIR_UP;
						yv = -1;
						state = STATE_FLOATUP;
					} else if (flags & kFlagPushDown) {
						e->dir = DIR_DOWN;
						yv = 1;
						state = STATE_FLOATDOWN;
					}

					if (!checkFloating(e->tileX + xv, e->tileY + yv)) {
						if (flags & kFlagAnimFast)
							e->moveSpeed = kPlayerMoveSpeed << 1;
						else if (flags & kFlagAnimMedium)
							e->moveSpeed = kPlayerMoveSpeed;
						else
							e->moveSpeed = kPushMoveSpeed;

						setEntityGoal(e, e->tileX + xv, e->tileY + yv);
						e->state = state;
					} else {
						// Landed on a floatmove entity. Make it float really slow and then it'll speed up
						uint32 flags2 = g_hdb->_map->getMapBGTileFlags(e->tileX + xv, e->tileY + yv);
						if (!(flags2 & (kFlagPushRight | kFlagPushLeft | kFlagPushUp | kFlagPushDown))) {
							floatEntity(e, STATE_FLOATING);
							e->value1 = 0x666;	// Don't move me ever again
							return;
						}

						if (flags & kFlagPushRight) {
							e->dir = DIR_RIGHT;
							xv = 1;
							state = STATE_FLOATRIGHT;
						} else if (flags & kFlagPushLeft) {
							e->dir = DIR_LEFT;
							xv = -1;
							state = STATE_FLOATLEFT;
						} else if (flags & kFlagPushUp) {
							e->dir = DIR_UP;
							yv = -1;
							state = STATE_FLOATUP;
						} else if (flags & kFlagPushDown) {
							e->dir = DIR_DOWN;
							yv = 1;
							state = STATE_FLOATDOWN;
						}

						e->moveSpeed = kPushMoveSpeed >> 1;
						setEntityGoal(e, e->tileX + xv, e->tileY + yv);
						e->state = state;
						}
					}
				}
			} else if (((flags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY)) & kFlagWater) && (e->type == AI_MAGIC_EGG || e->type == AI_ICE_BLOCK)) {
				// And no foreground tile is there
				if (g_hdb->_map->getMapFGTileIndex(e->tileX, e->tileY) < 0 && !checkFloating(e->tileX, e->tileY)) {
					if (flags & kFlagSlime) {
						// Evaporates in Slime
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
						removeEntity(e);
						warning("STUB: animateEntity: Play SND_SPLASH");
						return;
					} else {
						// Drowns in water
						addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_WATER_SPLASH_SIT);
						removeEntity(e);
						warning("STUB: animateEntity: Play SND_SPLASH");
						return;
					}
				}
			}
		} else if (onEvenTile(e->x, e->y))
			setEntityGoal(e, _waypoints[0].x, _waypoints[0].y);
	}

	animEntFrames(e);
}

void AI::animEntFrames(AIEntity *e) {
	int max = 1;
	// Set current graphic to draw
	switch (e->state) {
	case STATE_STANDDOWN:
		e->draw = e->standdownGfx[0];
		max = 1;
		break;
	case STATE_STANDUP:
		e->draw = e->standupGfx[0];
		max = 1;
		break;
	case STATE_STANDLEFT:
		e->draw = e->standleftGfx[0];
		max = 1;
		break;
	case STATE_STANDRIGHT:
		e->draw = e->standrightGfx[0];
		max = 1;
		break;
	case STATE_BLINK:
		e->draw = e->blinkGfx[e->animFrame];
		max = e->blinkFrames;
		break;
	case STATE_MOVEUP:
		e->draw = e->moveupGfx[e->animFrame];
		max = e->moveupFrames;
		break;
	case STATE_MOVEDOWN:
		e->draw = e->movedownGfx[e->animFrame];
		max = e->movedownFrames;
		break;
	case STATE_MOVELEFT:
		e->draw = e->moveleftGfx[e->animFrame];
		max = e->moveleftFrames;
		break;
	case STATE_MOVERIGHT:
		e->draw = e->moverightGfx[e->animFrame];
		max = e->moverightFrames;
		break;
	case STATE_PUSHDOWN:
		e->draw = _pushdownGfx[e->animFrame];
		max = _pushdownFrames;
		break;
	case STATE_PUSHUP:
		e->draw = _pushupGfx[e->animFrame];
		max = _pushupFrames;
		break;
	case STATE_PUSHLEFT:
		e->draw = _pushleftGfx[e->animFrame];
		max = _pushleftFrames;
		break;
	case STATE_PUSHRIGHT:
		e->draw = _pushrightGfx[e->animFrame];
		max = _pushrightFrames;
		break;

	case STATE_GRABUP:
		e->draw = _getGfx[DIR_UP];
		max = 1;
		break;
	case STATE_GRABDOWN:
		e->draw = _getGfx[DIR_DOWN];
		max = 1;
		break;
	case STATE_GRABLEFT:
		e->draw = _getGfx[DIR_LEFT];
		max = 1;
		break;
	case STATE_GRABRIGHT:
		e->draw = _getGfx[DIR_RIGHT];
		max = 1;
		break;

	case STATE_HORRIBLE1:
		e->draw = _horrible1Gfx[e->animFrame];
		max = _horrible1Frames;
		if (e->animFrame == max - 1)
			e->state = STATE_DEAD;
		break;
	case STATE_HORRIBLE2:
	{
		static int		click = 0;
		e->draw = _horrible2Gfx[e->animFrame];
		max = _horrible2Frames;
		click++;
		if (click == 16) {
			warning("STUB: Play SND_SHOCKBOT_SHOCK");
			click = 0;
		}
		break;
	}
	case STATE_HORRIBLE3:
	{
		static int		click = 0;
		e->draw = _horrible3Gfx[e->animFrame];
		max = _horrible3Frames;
		click++;
		if (click == 32) {
			warning("STUB: Play SND_GUY_GRABBED");
			click = 0;
		}
		break;
	}
	case STATE_HORRIBLE4:
		e->draw = _horrible4Gfx[e->animFrame];
		max = _horrible4Frames;
		if (e->animFrame == max - 1)
			e->state = STATE_DEAD;
		break;

	case STATE_PLUMMET:
		e->draw = _plummetGfx[e->animFrame];
		max = _plummetFrames;
		if (e->animFrame == max - 1) {
			e->state = STATE_NONE;
			setPlayerInvisible(true);
		}
		break;

		//
		// maintenance bot uses stuff
		//
	case STATE_USEDOWN:
		e->draw = e->standdownGfx[4];
		return;
	case STATE_USEUP:
		e->draw = e->standupGfx[4];
		return;
	case STATE_USELEFT:
		e->draw = e->standleftGfx[4];
		return;
	case STATE_USERIGHT:
		e->draw = e->standrightGfx[4];
		return;

		//
		// death!
		//
	case STATE_DYING:
		e->draw = _dyingGfx[e->animFrame];
		max = _dyingFrames;
		if (e->animFrame == max - 1)
			e->state = STATE_DEAD;
		break;

	case STATE_DEAD:
		e->draw = _dyingGfx[_dyingFrames - 1];
		max = _dyingFrames;
		break;

	case STATE_GOODJOB:
		e->draw = _goodjobGfx;
		max = 1;
		break;

		//
		// floating stuff uses the "standup" frames for animating the float
		//
	case STATE_FLOATING:
	case STATE_FLOATDOWN:
	case STATE_FLOATUP:
	case STATE_FLOATLEFT:
	case STATE_FLOATRIGHT:
		e->draw = e->blinkGfx[e->animFrame];
		max = e->blinkFrames;
		break;
	case STATE_MELTED:
	case STATE_EXPLODING:
		e->draw = e->special1Gfx[e->animFrame];
		max = e->special1Frames;
		if (e->type == AI_BOOMBARREL) {			// while exploding, call this function
			aiBarrelExplodeSpread(e);
			if (e->animFrame == max - 1) {
				removeEntity(e);
				return;
			}
		}
		break;

		//
		// ICEPUFF states
		//
	case STATE_ICEP_PEEK:
		e->draw = e->blinkGfx[e->animFrame];
		max = e->blinkFrames;
		break;
	case STATE_ICEP_APPEAR:
		e->draw = e->standupGfx[e->animFrame];
		max = e->standupFrames;
		break;
	case STATE_ICEP_THROWDOWN:
		e->draw = e->standdownGfx[e->animFrame];
		max = e->standdownFrames;
		break;
	case STATE_ICEP_THROWRIGHT:
		e->draw = e->standrightGfx[e->animFrame];
		max = e->standrightFrames;
		break;
	case STATE_ICEP_THROWLEFT:
		e->draw = e->standleftGfx[e->animFrame];
		max = e->standleftFrames;
		break;
	case STATE_ICEP_DISAPPEAR:
		e->draw = e->special1Gfx[e->animFrame];
		max = e->special1Frames;
		break;

		//
		// MEERKAT states
		//
	case STATE_MEER_MOVE:
		e->draw = e->standdownGfx[e->animFrame];
		max = e->standdownFrames;
		break;
	case STATE_MEER_APPEAR:
		e->draw = e->standleftGfx[e->animFrame];
		max = e->standleftFrames;
		break;
	case STATE_MEER_BITE:
		e->draw = e->standrightGfx[e->animFrame];
		max = e->standrightFrames;
		break;
	case STATE_MEER_DISAPPEAR:
		e->draw = e->standupGfx[e->animFrame];
		max = e->standupFrames;
		break;
	case STATE_MEER_LOOK:
		e->draw = e->movedownGfx[e->animFrame];
		max = e->movedownFrames;
		break;

		//
		// DIVERTER spawning states
		//
	case STATE_DIVERTER_BL:
		e->draw = e->standdownGfx[e->animFrame];
		max = e->standdownFrames;
		break;
	case STATE_DIVERTER_BR:
		e->draw = e->standupGfx[e->animFrame];
		max = e->standupFrames;
		break;
	case STATE_DIVERTER_TL:
		e->draw = e->standleftGfx[e->animFrame];
		max = e->standleftFrames;
		break;
	case STATE_DIVERTER_TR:
		e->draw = e->standrightGfx[e->animFrame];
		max = e->standrightFrames;
		break;

		//
		// DOLLY states
		// angry[4] = standright[4]
		// kissright[4]/kissleft[4] = standleft[8]
		// panic[4]/laugh[4] = standdown[8]
		// dollyuseright[5] = special1[5]
		//
	case STATE_ANGRY:
		e->draw = e->standrightGfx[e->animFrame];
		max = 2;
		break;
	case STATE_KISSRIGHT:
		e->draw = e->standleftGfx[e->animFrame];
		max = 4;
		break;
	case STATE_KISSLEFT:
		e->draw = e->standleftGfx[e->animFrame + 4];
		max = 4;
		break;
	case STATE_PANIC:
		e->draw = e->standdownGfx[e->animFrame];
		max = 2;
		break;
	case STATE_LAUGH:
		e->draw = e->standdownGfx[e->animFrame + 4];
		max = 2;
		break;
	case STATE_DOLLYUSERIGHT:
		e->draw = e->special1Gfx[e->animFrame];
		max = e->special1Frames;
		break;

		// SARGE yelling
	case STATE_YELL:
		e->draw = e->special1Gfx[e->animFrame];
		max = e->special1Frames;
		break;
	default:
		debug(9, "AI-FUNCS: animEntFrames: Unintended State for entity %s", AIType2Str(e->type));
		break;
	}

	// Cycle animation frames
	if (e->animDelay-- > 0)
		return;
	e->animDelay = e->animCycle;

	e->animFrame++;
	if (e->animFrame == max) {
		e->animFrame = 0;
	}
}

void AI::drawEnts(int x, int y, int w, int h) {

	static int stunAnim = 0;
	static uint32 stunTimer = g_hdb->getTimeSlice();

	// Draw Floating Entities
	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		AIEntity *e = (*it);
		if (e->aiDraw) {
			e->aiDraw(e, x, y);
		}

		if ((e->x > x - kTileWidth) && (e->x < x + w) && (e->y > y - kTileHeight) && (e->y < y + h)) {
			e->draw->drawMasked(e->x - x + e->drawXOff, e->y - y + e->drawYOff);
			e->onScreen = 1;
		} else
			e->onScreen = 0;
	}

	// Draw all other Ents
	_numLevel2Ents = 0;

	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		AIEntity *e = (*it);
		debugN(5, "AI::drawEnts: enity %s(%d) state %s(%d)...", AIType2Str(e->type), e->type, AIState2Str(e->state), e->state);

		if (e->type == AI_LASER || e->type == AI_DIVERTER) {
			if (e->aiDraw) {
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents].aiDraw = e->aiDraw;
					_entsLevel2[_numLevel2Ents].x = x;
					_entsLevel2[_numLevel2Ents].y = y;
					_entsLevel2[_numLevel2Ents].e = e;
					_entsLevel2[_numLevel2Ents].stunnedWait = 0;
					_numLevel2Ents++;
					debugN(5, "not drawing1...");
				} else {
					e->aiDraw(e, x, y);
					debugN(5, "drawing1...");
				}
			}
		}

		if ((e->x > x - kTileWidth) && (e->x < x + w) && (e->y > y - kTileHeight) && (e->y < y + h)) {
			// If extra drawing func is present, call it
			if (e->aiDraw && e->type != AI_LASER && e->type != AI_DIVERTER) {
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents].aiDraw = e->aiDraw;
					_entsLevel2[_numLevel2Ents].draw = e->draw;
					_entsLevel2[_numLevel2Ents].x = x;
					_entsLevel2[_numLevel2Ents].y = y;
					_entsLevel2[_numLevel2Ents].e = e;
					_entsLevel2[_numLevel2Ents].stunnedWait = 0;
					_numLevel2Ents++;
					debugN(5, "not drawing2...");
				} else {
					e->aiDraw(e, x, y);
					debugN(5, "drawing2...");
				}
			}

			switch (e->type) {
			case AI_VORTEXIAN:
				if (e->draw)
					e->draw->drawMasked(e->x - x + e->drawXOff, e->y - y + e->drawYOff);
				debug(9, "STUB: AI::drawEnts: Replace Masked Blitting with Alpha Masked Blitting");
				break;
			case AI_GUY: // Draw Player Last
				break;
			default:
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents].aiDraw = NULL;
					_entsLevel2[_numLevel2Ents].draw = e->draw;
					_entsLevel2[_numLevel2Ents].x = e->x - x + e->drawXOff;
					_entsLevel2[_numLevel2Ents].y = e->y - y + e->drawYOff;
					_entsLevel2[_numLevel2Ents].e = NULL;
					_entsLevel2[_numLevel2Ents].stunnedWait = e->stunnedWait;
					_numLevel2Ents++;
					debugN(5, "not trying to draw...");
				} else {
					debugN(5, "trying to draw...");

					if (e->draw) {
						debugN(5, "at %d %d", e->x, e->y);

						e->draw->drawMasked(e->x - x + e->drawXOff, e->y - y + e->drawYOff);
					} else {
						debugN(5, "no draw function");
					}

					if (e->stunnedWait)
						g_hdb->_ai->_stunnedGfx[stunAnim]->drawMasked(e->x - x, e->y - y);
				}
				break;
			}
			e->onScreen = 1;
		} else {
			e->onScreen = 0;
			debugN(5, "not on screen");
		}
		debug(5, ""); // newline
	}

	if (stunTimer < g_hdb->getTimeSlice()) {
		stunAnim = (stunAnim + 1) & 3;
		stunTimer = g_hdb->getTimeSlice();
	}

	// Draw player last
	if (_player && _player->level < 2 && !_playerInvisible && _player->draw) {
		_player->draw->drawMasked(_player->x - x + _player->drawXOff, _player->y - y + _player->drawYOff);
	}
}

void AI::drawLevel2Ents() {
	int debugging = 0; //game.GetDebug(); // FIXME

	for (int i = 0; i < _numLevel2Ents; i++) {
		// call custom drawing code?
		if (_entsLevel2[i].aiDraw) {
			debug(5, "AI::drawLevel2Ents: entity %s(%d) at %d,%d", AIType2Str(_entsLevel2[i].e->type), _entsLevel2[i].e->type, _entsLevel2[i].x, _entsLevel2[i].y);

			_entsLevel2[i].aiDraw(_entsLevel2[i].e, _entsLevel2[i].x, _entsLevel2[i].y);
		} else if (_entsLevel2[i].draw) {
			debug(5, "AI::drawLevel2Ents: tile '%s' at %d,%d", _entsLevel2[i].draw->getName(), _entsLevel2[i].x, _entsLevel2[i].y);

			_entsLevel2[i].draw->drawMasked(_entsLevel2[i].x, _entsLevel2[i].y);
		} else if (debugging) {
			_debugQMark->drawMasked(_entsLevel2[i].x, _entsLevel2[i].y );
		}

		if (_entsLevel2[i].stunnedWait)
			 g_hdb->_ai->_stunnedGfx[_stunAnim]->drawMasked(_entsLevel2[i].x , _entsLevel2[i].y);
	}

	// always draw the player last
	if (_player && _player->level == 2 && !_playerInvisible) {
		int x, y;
		g_hdb->_map->getMapXY(&x, &y);

		if (_player->draw)
			_player->draw->drawMasked((_player->x - x) + _player->drawXOff, (_player->y - y) + _player->drawYOff);
	}

	if (_stunTimer < g_system->getMillis()) {
		_stunAnim = (_stunAnim + 1) & 3;
		_stunTimer = g_system->getMillis() + 100;
	}
}

void AI::animGrabbing() {
	if (_player->state == STATE_GRABUP ||
		_player->state == STATE_GRABDOWN ||
		_player->state == STATE_GRABLEFT ||
		_player->state == STATE_GRABRIGHT)
		return;

	AIState s = STATE_NONE;

	switch (_player->dir) {
	case DIR_UP:
		s = STATE_GRABUP;
		_player->draw = _getGfx[DIR_UP];
		break;
	case DIR_DOWN:
		s = STATE_GRABDOWN;
		_player->draw = _getGfx[DIR_DOWN];
		break;
	case DIR_LEFT:
		s = STATE_GRABLEFT;
		_player->draw = _getGfx[DIR_LEFT];
		break;
	case DIR_RIGHT:
		s = STATE_GRABRIGHT;
		_player->draw = _getGfx[DIR_RIGHT];
		break;
	default:
		warning("AI-FUNCS: animGrabbing: DIR_NONE found");
		break;
	}

	_player->state = s;
	_player->animFrame = 5;
}

void AI::entityFace(const char *luaName, int dir) {
	AIEntity *e = locateEntity(luaName);
	e->dir = (AIDir)dir;

	switch (e->dir) {
	case DIR_UP: e->state = STATE_STANDUP; break;
	case DIR_DOWN: e->state = STATE_STANDDOWN; break;
	case DIR_LEFT: e->state = STATE_STANDLEFT; break;
	case DIR_RIGHT: e->state = STATE_STANDRIGHT; break;
	case DIR_NONE: break;
	}
}

void AI::moveEnts() {
	static int frameDelay = kAnimFrameDelay;
	AIEntity *e;

	if (frameDelay-- > 0)
		return;

	frameDelay = kAnimFrameDelay;

	// Call aiAction for Floating Entities
	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		if ((*it)->aiAction)
			(*it)->aiAction((*it));
	}

	// Call aiAction for all other Entities
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = (*it);
		if (e->aiAction) {
			// NPC Touchplate Counter
			if (e != _player && e->touchpWait) {
				e->touchpWait--;
				if (!e->touchpWait) {
					if (e->tileX == e->touchpX && e->tileY == e->touchpY && onEvenTile(e->x, e->y))
						e->touchpWait = 1;
					else {
						checkActionList(e, e->touchpX, e->touchpY, false);
						g_hdb->_map->setMapBGTileIndex(e->touchpX, e->touchpY, e->touchpTile);
						e->touchpX = e->touchpY = e->touchpTile = 0;
					}
				}
			}
			// Stunned Entity Timer
			if (!e->stunnedWait)
				e->aiAction(e);
			else {
				if (e->stunnedWait < g_hdb->getTimeSlice())
					e->stunnedWait = 0;
			}
		}
	}

	debug(9, "STUB: moveEnts: Laser Rescan");
	debug(9, "STUB: moveEnts: Laser Looping Sound Channel");
}

int AI::checkForTouchplate(int x, int y) {
	int tileIndex = g_hdb->_map->getMapBGTileIndex(x, y);
	if (tileIndex == _touchplateOff || tileIndex == _templeTouchpOff)
		return tileIndex;
	return 0;
}

bool AI::findPath(AIEntity *e) {
	int x, y, xv = 0, yv = 0, max;
	ArrowPath *here;

	// Initial Pointing Direction to search in
	x = e->tileX;
	y = e->tileY;
	here = findArrowPath(x, y);
	// Only look for GO arrows at this first location
	if (here && here->type == 1)
		e->dir = here->dir;

	switch (e->dir) {
	case DIR_UP:
		yv = -1;
		break;
	case DIR_DOWN:
		yv = 1;
		break;
	case DIR_LEFT:
		xv = -1;
		break;
	case DIR_RIGHT:
		xv = 1;
		break;
	case DIR_NONE:
		warning("findPath: DIR_NONE found");
		break;
	}

	if (xv)
		max = g_hdb->_map->_width;
	else
		max = g_hdb->_map->_height;

	ArrowPath *arrowPath;
	uint32 flags;
	while (max--) {
		arrowPath = findArrowPath(x + xv, y + yv);
		if (arrowPath) {
			setEntityGoal(e, arrowPath->tileX, arrowPath->tileY);
			return true;
		} else {
			flags = g_hdb->_map->getMapBGTileFlags(x + xv, y + yv);
			if (flags & kFlagSolid)
				return false;
		}
		x += xv;
		y += yv;
	}

	return false;
}

AIEntity *AI::legalMove(int tileX, int tileY, int level, int *result) {
	uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(tileX, tileY);
	uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(tileX, tileY);
	AIEntity *hit = findEntity(tileX, tileY);

	if (hit && hit->state != STATE_FLOATING)
		// If player and entity are not at the same level, are they on stairs?
		if (hit->level != level) {
			if (level == 1 && !(bgFlags & kFlagStairTop)) {
				hit = NULL;
			} else if (level == 1 && !(bgFlags & kFlagStairBot)) {
				hit = NULL;
			}
		}

	if (level == 1) {
		if (bgFlags & kFlagSolid) {
			*result = 0;
			return hit;
		}

		if (bgFlags & (kFlagWater | kFlagSlime)) {
			if (hit && hit->state == STATE_FLOATING) {
				*result = 1;
				return NULL;
			} else
				*result = 0;
			return hit;
		} else
			*result = 1;
	} else {
		if (fgFlags & kFlagSolid) {
			*result = 0;
			return hit;
		} else if (fgFlags & kFlagGrating) {
			*result = 1;
			return hit;
		} else if (bgFlags & kFlagSolid) {
			*result = 0;
			return hit;
		}

		if (bgFlags & (kFlagWater | kFlagSlime | kFlagPlummet)) {
			if (hit && hit->state == STATE_FLOATING) {
				*result = 1;
				return NULL;
			} else
				*result = 0;
			return hit;
		} else
			*result = 1;
	}
	return hit;
}

AIEntity *AI::legalMoveOverWater(int tileX, int tileY, int level, int *result) {
	uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(tileX, tileY);
	uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(tileX, tileY);
	AIEntity *hit = findEntity(tileX, tileY);

	if (level == 1 ? (bgFlags & kFlagMonsterBlock) : (!(fgFlags &kFlagGrating) && ((fgFlags & kFlagSolid) || (bgFlags & kFlagMonsterBlock))))
		*result = 0;
	else
		*result = 1;
	return hit;
}

AIEntity *AI::legalMoveOverWaterIgnore(int tileX, int tileY, int level, int *result, AIEntity *ignore) {
	uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(tileX, tileY);
	uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(tileX, tileY);
	AIEntity *hit = findEntityIgnore(tileX, tileY, ignore);

	if (level == 1 ? (bgFlags & kFlagMonsterBlock) : (!(fgFlags &kFlagGrating) && ((fgFlags & kFlagSolid) || (bgFlags & kFlagMonsterBlock))))
		*result = 0;
	else
		*result = 1;
	return hit;
}

AIEntity *AI::playerCollision(int topBorder, int bottomBorder, int leftBorder, int rightBorder) {
	AIEntity *e;
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		e = *it;
		if (e == _player || !e->onScreen)
			continue;
		if (e->x > (_player->x - 32 - leftBorder) && e->x < (_player->x + 32 + rightBorder) && e->y >(_player->y - 32 - topBorder) && e->y < (_player->y + 32 + bottomBorder))
			return e;
	}
	return NULL;
}

bool AI::checkPlayerTileCollision(int x, int y) {
	debug(9, "STUB: checkPlayerTileCollison: g_hdb->getDebug() required");
	return (_player->tileX == x && _player->tileY == y);
}

bool AI::checkPlayerCollision(int x, int y, int border) {
	debug(9, "STUB: checkPlayerCollison: g_hdb->getDebug() required");
	return (x > (_player->x - 32 + border) && x < (_player->x - 32 - border) && y >(_player->y - 32 + border) && y < (_player->y - 32 - border));
}

void AI::floatEntity(AIEntity *e, AIState state) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if (e == *it) {
			_floats->push_back(*it);
			_ents->erase(it);
			break;
		}
	}
	e->state = state;
	e->level = 1;
}

bool AI::checkFloating(int x, int y) {
	for (Common::Array<AIEntity *>::iterator it = _floats->begin(); it != _floats->end(); it++) {
		if ((*it)->tileX == x && (*it)->tileY == y)
			return true;
	}
	return false;
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
		_player->draw = _getGfx[DIR_UP];
		break;
	case DIR_DOWN:
		_player->state = STATE_STANDDOWN;
		_player->draw = _getGfx[DIR_DOWN];
		break;
	case DIR_LEFT:
		_player->state = STATE_STANDLEFT;
		_player->draw = _getGfx[DIR_LEFT];
		break;
	case DIR_RIGHT:
		_player->state = STATE_STANDRIGHT;
		_player->draw = _getGfx[DIR_RIGHT];
		break;
	default:
		warning("AI-FUNCS: lookAtXY: DIR_NONE");
	}
}

void AI::movePlayer(uint16 buttons) {
	AIState stateList[] = {STATE_ATK_CLUB_UP,	STATE_ATK_CLUB_DOWN, STATE_ATK_CLUB_LEFT, STATE_ATK_CLUB_RIGHT,
							STATE_ATK_STUN_UP,	STATE_ATK_STUN_DOWN, STATE_ATK_STUN_LEFT, STATE_ATK_STUN_RIGHT,
							STATE_ATK_SLUG_UP,	STATE_ATK_SLUG_DOWN, STATE_ATK_SLUG_LEFT, STATE_ATK_SLUG_RIGHT,
							STATE_PUSHUP,		STATE_PUSHDOWN,		 STATE_PUSHLEFT,	  STATE_PUSHRIGHT,
							STATE_GRABUP,		STATE_GRABDOWN,		 STATE_GRABLEFT,	  STATE_GRABRIGHT};
	int	xva[5] = {9, 0, 0,-1, 1};
	int	yva[5] = {9,-1, 1, 0, 0};
	AIEntity *hit;
	int	xv = 0, yv = 0, nx, ny;
	int	attackable;

	if (!_player)
		return;

	// If we're already attacking, don't do anything else
	for (int i = 0; i < 20; i++) {
		if (_player->state == stateList[i])
			return;
	}

	// Just trying to put away a dialog?
	if (buttons & kButtonB) {
		if (g_hdb->_window->dialogActive()) {
			g_hdb->_window->closeDialog();
			return;
		} else if (g_hdb->_window->dialogChoiceActive()) {
			g_hdb->_window->closeDialogChoice();
			return;
		} else if (g_hdb->_window->msgBarActive()) {
			g_hdb->_window->closeMsg();
			return;
		}

		if (cinematicsActive() || _playerLock)
			return;

		// Are we trying to use something? An ACTION, AUTO, LUA?
		nx = _player->tileX + xva[_player->dir];
		ny = _player->tileY + yva[_player->dir];
		hit = findEntity(nx, ny);

		// the reason to check for no entity or an AI_NONE is because
		// there's a possibility that an actual entity and a LUA entity
		// can share the same spot, so we need to be able to deal with
		// the real entity first, then the LUA entity.
		if (!hit || (hit && hit->type == AI_NONE)) {
			switch (_player->state) {
			case STATE_STANDUP:
			case STATE_STANDDOWN:
			case STATE_STANDLEFT:
			case STATE_STANDRIGHT:
				if (checkForTouchplate(nx, ny))
					break;
				if (checkActionList(_player, nx, ny, true))
					return;
				if (checkAutoList(_player, nx, ny))
					return;
				if (checkLuaList(_player, nx, ny))
					return;
			default:
				break;
			}
		}

		// Attackable Entity? (we're right up on it)
		int amt = getGemAmount();
		attackable = 0;
		if (hit)
			switch (hit->type) {
			case AI_OMNIBOT:
			case AI_TURNBOT:
			case AI_SHOCKBOT:
			case AI_RIGHTBOT:
			case AI_PUSHBOT:
			case AI_LISTENBOT:
			case AI_MAINTBOT:
			case AI_DEADEYE:
			case AI_MEERKAT:
			case AI_FATFROG:
			case AI_GOODFAIRY:
			case AI_BADFAIRY:
			case AI_ICEPUFF:
			case AI_BUZZFLY:
			case AI_DRAGON:
			case AI_NONE:
				attackable = 1;
				break;
			default:
				break;
			}

		if (g_hdb->getActionMode() && ((hit && attackable) || !hit)) {
			// Attack
			warning("STUB: movePlayer: Attack!");
			return;
		}

		// Puzzle Mode - throw a gem
		// If this is the last gem, throw it and signal that it should come back

		if (amt && (attackable || !hit)) {
			xv = xva[_player->dir];
			yv = yva[_player->dir];
			nx = _player->tileX + xv;
			ny = _player->tileY + yv;

			spawn(AI_GEM_ATTACK, _player->dir, nx, ny, NULL, NULL, NULL, DIR_NONE, _player->level, amt == 1, 0, 1);
			setGemAmount(amt - 1);
			animGrabbing();
			return;
		}

		// Are we trying to use this entity?
		if (hit) {
			g_hdb->useEntity(hit);
			return;
		}
	}	// if kButtonB

	if (!onEvenTile(_player->x, _player->y))
		return;

	// Is the player dead or move-locked?
	if (_player->state == STATE_DEAD || _playerLock || _playerEmerging)
		return;

	// Are we on a touchplate and trying to move within the waiting period
	if (_player->touchpWait > kPlayerTouchPWait / 4)
		return;

	// Is a dialog active?
	if (g_hdb->_window->dialogActive())
		if (!cinematicsActive())
			return;

	// is a choice dialog active?
	if (g_hdb->_window->dialogChoiceActive()) {
		if (!cinematicsActive())
			return;
	}

	// In a cinematic?
	if (_playerLock || _numWaypoints)
		return;

	if (buttons & kButtonUp)
		yv = -1;
	else if (buttons & kButtonDown)
		yv = 1;
	else if (buttons & kButtonLeft)
		xv = -1;
	else if (buttons & kButtonRight)
		xv = 1;
	else if (buttons & kButtonB) {
		playerUse();
		return;
	}

	// Check if we can move there
	nx = _player->tileX + xv;
	if (!nx)	// Don't allow moving to X-cooridinate 0
		return;
	ny = _player->tileY + yv;

	int moveOK;
	hit = legalMove(nx, ny, _player->level, &moveOK);
	if (hit && walkThroughEnt(hit->type))
		hit = NULL;

	if (hit || !moveOK) {
		lookAtXY(nx, ny);
		stopEntity(_player);
		return;
	}

	// Walk into Lua Entity?
	if (checkLuaList(_player, nx, ny))
		return;

	if (buttons & (kButtonUp | kButtonDown | kButtonLeft | kButtonRight)) {
		int temp = _player->animFrame;
		if (_player->state != STATE_MOVELEFT && _player->state != STATE_MOVERIGHT && _player->state != STATE_MOVEUP && _player->state != STATE_MOVEDOWN)
			temp = 0;
		setEntityGoal(_player, nx, ny);
		_player->animFrame = temp;
	} else
		setEntityGoal(_player, nx, ny);
}

void AI::playerUse() {
	int	xv[5] = {9, 0, 0,-1, 1};
	int	yv[5] = {9,-1, 1, 0, 0};

	g_hdb->setTargetXY(kTileWidth * (_player->tileX + xv[_player->dir]), kTileWidth * (_player->tileY + yv[_player->dir]));
}
} // End of Namespace
