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

					if (g_hdb->_drawMan->selectGfxType((*it))) {
						Tile *gfx = g_hdb->_drawMan->getTileGfx((*it), size);

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
						Picture *gfx = g_hdb->_drawMan->getPicGfx((*it), size);

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
		AIEntity *temp = _inventory[i]->ent;

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
	}
	break;
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
	}
	break;
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
		warning("AI-FUNCS: animEntFrames: Unintended State");
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

void AI::animLuaEntity(const char *initName, AIState st) {
	for (Common::Array<AIEntity *>::iterator it = _ents->begin(); it != _ents->end(); it++) {
		if (Common::matchString((*it)->entityName, initName)) {
			(*it)->state = st;
			(*it)->animFrame = 0;
			(*it)->animDelay = (*it)->animCycle;
		}
	}
}

void AI::drawEnts(int x, int y, int w, int h) {

	static int stunAnim = 0;

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
		if (e->type == AI_LASER || e->type == AI_DIVERTER) {
			if (e->aiDraw) {
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents]->aiDraw = e->aiDraw;
					_entsLevel2[_numLevel2Ents]->x = x;
					_entsLevel2[_numLevel2Ents]->y = y;
					_entsLevel2[_numLevel2Ents]->e = e;
					_entsLevel2[_numLevel2Ents]->stunnedWait = 0;
					_numLevel2Ents++;
				} else {
					e->aiDraw(e, x, y);
				}
			}
		}

		if ((e->x > x - kTileWidth) && (e->x < x + w) && (e->y > y - kTileHeight) && (e->y < y + h)) {
			// If extra drawing func is present, call it
			if (e->aiDraw && e->type != AI_LASER && e->type != AI_DIVERTER) {
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents]->aiDraw = e->aiDraw;
					_entsLevel2[_numLevel2Ents]->draw = e->draw;
					_entsLevel2[_numLevel2Ents]->x = x;
					_entsLevel2[_numLevel2Ents]->y = y;
					_entsLevel2[_numLevel2Ents]->e = e;
					_entsLevel2[_numLevel2Ents]->stunnedWait = 0;
					_numLevel2Ents++;
				} else
					e->aiDraw(e, x, y);
			}

			switch (e->type) {
			case AI_VORTEXIAN:
				warning("STUB: AI::drawEnts: Tile Alpha Blitting required");
				break;
			case AI_GUY: // Draw Player Last
				break;
			default:
				if (e->level == 2 && _numLevel2Ents < kMaxLevel2Ents) {
					_entsLevel2[_numLevel2Ents]->aiDraw = NULL;
					_entsLevel2[_numLevel2Ents]->draw = e->draw;
					_entsLevel2[_numLevel2Ents]->x = e->x - x + e->drawXOff;
					_entsLevel2[_numLevel2Ents]->y = e->y - y + e->drawYOff;
					_entsLevel2[_numLevel2Ents]->e = NULL;
					_entsLevel2[_numLevel2Ents]->stunnedWait = e->stunnedWait;
					_numLevel2Ents++;
				} else {
					if (e->draw)
						e->draw->drawMasked(e->x - x + e->drawXOff, e->y - y + e->drawYOff);

					if (e->stunnedWait)
						g_hdb->_ai->_stunnedGfx[stunAnim]->drawMasked(e->x - x, e->y - y);
				}
				break;
			}
			e->onScreen = 1;
		} else
			e->onScreen = 0;
	}

	warning("STUB: AI::drawEnts: Increment Stun Timer");

	// Draw player last
	if (_player && _player->level < 2 && !_playerInvisible && _player->draw) {
		_player->draw->drawMasked(_player->x - x + _player->drawXOff, _player->y - y + _player->drawYOff);
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
	default:
		warning("AI-FUNCS: lookAtXY: DIR_NONE");
	}
}
} // End of Namespace
