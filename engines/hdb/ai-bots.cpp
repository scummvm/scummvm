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

void aiOmniBotInit(AIEntity *e) {
	if (e->value1 == 1)
		e->aiAction = aiOmniBotMove;
	else if (g_hdb->_ai->findPath(e))
		e->aiAction = aiOmniBotAction;
}

void aiOmniBotInit2(AIEntity *e) {
	e->standdownGfx[0] = e->movedownGfx[0];
	e->standupGfx[0] = e->movedownGfx[0];
	e->standleftGfx[0] = e->moveleftGfx[0];
	e->standrightGfx[0] = e->moverightGfx[0];
	e->standdownFrames = e->standupFrames = e->standleftFrames = e->standrightFrames = 1;
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiOmniBotMove(AIEntity *e) {
	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else
		g_hdb->_ai->animEntFrames(e);
}

void aiOmniBotAction(AIEntity *e) {
	AIEntity *p = g_hdb->_ai->getPlayer();
	if (e->goalX) {
		if (!e->sequence) {
			g_hdb->_ai->animateEntity(e);
			// Is the Player collding?
			if (e->onScreen && (p->level = e->level) && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead()) {
				g_hdb->_ai->killPlayer(DEATH_FRIED);
				return;
			}

			// Shoot player ?
			if (onEvenTile(e->x, e->y) && g_hdb->getActionMode()) {
				int xv = 0, yv = 0, result;
				int shoot = 0;
				AIEntity *omni;
				// FIXME: Is reloading Player required here?
				p = g_hdb->_ai->getPlayer();

				// On same level/screen?
				if ((e->level != p->level) || g_hdb->_ai->playerDead() || !e->onScreen)
					return;

				// Is Player in Line of Sight?
				switch (e->dir) {
				case DIR_UP:
					if (p->x == e->x && p->y < e->y) {
						shoot = 1;
						yv = -1;
					}
					break;
				case DIR_DOWN:
					if (p->x == e->x && p->y > e->y) {
						shoot = 1;
						yv = 1;
					}
					break;
				case DIR_LEFT:
					if (p->x < e->x && p->y == e->y) {
						shoot = 1;
						xv = -1;
					}
					break;
				case DIR_RIGHT:
					if (p->x > e->x && p->y == e->y) {
						shoot = 1;
						xv = 1;
					}
					break;
				case DIR_NONE:
					warning("aiOmniBotAction: DIR_NONE found");
					break;
				}

				// If shoot = 1, take the shot
				// (1) Check we're not shooting into a solid tile
				// (2) Check we're not shooting into an Entity unless it's the player
				AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX+xv, e->tileY + yv, e->level, &result);
				if (shoot && !hit && result) {
					omni = g_hdb->_ai->spawn(AI_OMNIBOT_MISSILE, e->dir, e->tileX + xv, e->tileY + yv, NULL, NULL, NULL, DIR_NONE, e->level, 0, 0, 1);
					omni->xVel = xv * kPlayerMoveSpeed * 2;
					omni->yVel = yv * kPlayerMoveSpeed * 2;
					if (g_hdb->_map->onScreen(e->tileX, e->tileY))
						warning("STUB: aiOmniBotAction: Play SND_OMNIBOT_FIRE");
					if (!g_hdb->getActionMode()) {
						omni->xVel >>= 1;
						omni->yVel >>= 1;
					}
					e->sequence = 16;
				}
			}
		}
	} else {
		g_hdb->_ai->findPath(e);
		if (e->onScreen)
			warning("STUB: aiOmniBotAction: Play SND_OMNIBOT_AMBIENT");
	}

	if (e->sequence)
		e->sequence--;
}

void aiOmniBotMissileInit(AIEntity *e) {
	e->state = STATE_MOVEDOWN;
	e->aiAction = aiOmniBotMissileAction;
}

void aiOmniBotMissileInit2(AIEntity *e) {
	for (int i = 0; i < e->movedownFrames; i++)
		e->moveleftGfx[i] = e->moverightGfx[i] = e->moveupGfx[i] = e->movedownGfx[i];

	e->moveleftFrames = e->moverightFrames = e->moveupFrames = e->movedownFrames;
	e->draw = e->movedownGfx[0];
}

void aiOmniBotMissileAction(AIEntity *e) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	g_hdb->_ai->animEntFrames(e);
	e->x += e->xVel;
	e->y += e->yVel;
	e->tileX = e->x / kTileWidth;
	e->tileY = e->y / kTileHeight;

	// Did we hit a solid wall?
	int result;
	AIEntity *hit = g_hdb->_ai->legalMoveOverWaterIgnore(e->tileX, e->tileY, e->level, &result, e);

	if (hit || !result) {
		g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_FAST, false, false, "steam_puff_sit");
		g_hdb->_ai->removeEntity(e);
	}

	// On Even tiles, check for hitting player
	if (onEvenTile(e->x, e->y))
		if (e->onScreen && (p->level == e->level) && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead()) {
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_FAST, false, false, "steam_puff_sit");
			g_hdb->_ai->removeEntity(e);
		}
}

void aiTurnBotInit(AIEntity *e) {
	e->aiAction = aiTurnBotAction;
}

void aiTurnBotInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiTurnBotChoose(AIEntity *e) {
	int xvAhead[5] = { 9, 0, 0, -1, 1 }, yvAhead[5] = { 9, -1, 1, 0, 0 };
	AIDir turnRight[5] = { DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN };
	AIState dirState[5] = { STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT };
	int xv, yv;

	xv = xvAhead[e->dir];
	yv = yvAhead[e->dir];
	if (g_hdb->_map->getMapBGTileFlags(e->tileX + xv, e->tileY + yv) & (kFlagSolid | kFlagWater)) {
		e->xVel = e->yVel = 0;
		e->animFrame = 0;
		e->animDelay = e->animCycle;
		e->dir = turnRight[e->dir];
		e->state = dirState[e->dir];
	} else {
		e->xVel = xv * kPlayerMoveSpeed;
		e->yVel = yv * kPlayerMoveSpeed;
		if (!g_hdb->getActionMode()) {
			e->xVel >>= 1;
			e->yVel >>= 1;
		}
		e->goalX = e->tileX + xv;
		e->goalY = e->tileY + yv;
		e->state = dirState[e->dir];
		if (e->dir == DIR_DOWN)
			e->animFrame = 3;
	}
}

void aiTurnBotAction(AIEntity *e) {
	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else {
		aiTurnBotChoose(e);
		g_hdb->_ai->animateEntity(e);
		if (e->onScreen)
			warning("STUB: aiTurnBotAction: Play SND_TURNBOT_TURN");
	}

	if (e->onScreen && onEvenTile(e->x, e->y) && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 0) && !g_hdb->_ai->playerDead())
		g_hdb->_ai->killPlayer(DEATH_NORMAL);
}

void aiShockBotInit(AIEntity *e) {
	g_hdb->_ai->findPath(e);
	e->aiAction = aiShockBotAction;
	e->animCycle = 0;
	e->sequence = 0;
	e->aiDraw = aiShockBotShock;
}

void aiShockBotInit2(AIEntity *e) {
	e->standupFrames = e->standdownFrames = e->standleftFrames = e->standrightFrames =
		e->moveupFrames = e->moverightFrames = e->moveleftFrames = e->movedownFrames;

	for (int i = 0; i < e->movedownFrames; i++)
		e->standupGfx[i] = e->standleftGfx[i] = e->standrightGfx[i] = e->standdownGfx[i] = e->moveupGfx[i] = e->moveleftGfx[i] = e->moverightGfx[i] = e->movedownGfx[i];

	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiShockBotAction(AIEntity *e) {
	if (e->goalX) {
		if (!e->sequence) {
			if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead())
				g_hdb->_ai->killPlayer(DEATH_SHOCKED);
			g_hdb->_ai->animateEntity(e);
		} else
			g_hdb->_ai->animEntFrames(e);
	} else {
		g_hdb->_ai->findPath(e);
		e->sequence = 20;
		g_hdb->_ai->animEntFrames(e);
		if (e->onScreen)
			warning("STUB: aiShockBotAction: Play SND_SHOCKBOT_AMBIENT");

	}

	if (e->sequence)
		e->sequence--;
}

void aiShockBotShock(AIEntity *e, int mx, int my) {
	int offX[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	int offY[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };

	// Only on a exact tile boundary do we change the shocked tiles
	// Start at top left and go around
	if (g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY) & kFlagMetal)
		e->special1Gfx[e->animFrame]->drawMasked(e->tileX * kTileWidth - mx, e->tileY * kTileHeight - my);

	uint32 flags;
	for (int i = 0; i < 8; i++) {
		flags = g_hdb->_map->getMapBGTileFlags(e->tileX + offX[i], e->tileY + offY[i]);
		if (flags & kFlagMetal) {
			// Is the shocking tile onScreen?
			if (g_hdb->_map->checkXYOnScreen((e->tileX + offX[i]) * kTileWidth, (e->tileY + offY[i]) * kTileHeight)) {
				// Draw shocking tile animation
				e->special1Gfx[e->animFrame]->drawMasked((e->tileX + offX[i])*kTileWidth - mx, (e->tileY + offY[i])*kTileHeight - my);
				// Did the player get fried?
				// Check every 4 frames
				if (e->onScreen && !e->animFrame && g_hdb->_ai->checkPlayerTileCollision(e->tileX + offX[i], e->tileY + offY[i]) && !g_hdb->_ai->playerDead()) {
					g_hdb->_ai->killPlayer(DEATH_SHOCKED);
					return;
				}
				if (!e->animFrame && g_hdb->_map->boomBarrelExist(e->tileX + offX[i], e->tileY + offY[i])) {
					AIEntity *e2 = g_hdb->_ai->findEntityType(AI_BOOMBARREL, e->tileX + offX[i], e->tileY + offY[i]);
					aiBarrelExplode(e2);
				}
			}
		}
	}
}

void aiRightBotInit(AIEntity *e) {
	e->moveSpeed = kPlayerMoveSpeed;
	if (!g_hdb->getActionMode())
		e->moveSpeed >>= 1;
	e->aiAction = aiRightBotAction;
}

void aiRightBotInit2(AIEntity *e) {
	switch (e->dir) {
	case DIR_UP:
		e->draw = e->moveupGfx[0];
		e->state = STATE_MOVEUP;
		break;
	case DIR_DOWN:
		e->draw = e->movedownGfx[0];
		e->state = STATE_MOVEDOWN;
		break;
	case DIR_LEFT:
		e->draw = e->moveleftGfx[0];
		e->state = STATE_MOVELEFT;
		break;
	case DIR_RIGHT:
		e->draw = e->moverightGfx[0];
		e->state = STATE_MOVERIGHT;
		break;
	case DIR_NONE:
		warning("aiRightBotInit2: DIR_NONE found");
		break;
	}
}

void aiRightBotFindGoal(AIEntity *e) {
	int	xv, yv, xv2, yv2, xv3, yv3;
	int	bg, bg2, bg3;
	AIEntity *e1, *e2, *e3, *p;
	int	hit, sx, sy, rotate;

	int		xvAhead[5]	= { 9, 0, 0,-1, 1 }, yvAhead[5]	= { 9,-1, 1, 0, 0 };
	int		xvAToR[5]	= { 9, 1,-1,-1, 1 }, yvAToR[5]	= { 9,-1, 1,-1, 1 };
	int		xvToR[5]	= { 9, 1,-1, 0, 0 }, yvToR[5]	= { 9, 0, 0,-1, 1 };
	int		xvToL[5]	= { 9,-1, 1, 0, 0 }, yvToL[5]	= { 9, 0, 0, 1,-1 };

	p = g_hdb->_ai->getPlayer();
	rotate = 0;

	do {
		xv = xvAhead[e->dir];	// Search Ahead
		yv = yvAhead[e->dir];
		xv2 = xvAToR[e->dir];	// Search Ahead and to the Right
		yv2 = yvAToR[e->dir];
		xv3 = xvToR[e->dir];	// Search to the Right
		yv3 = yvToR[e->dir];

		// Search until we hit a wall...or empty space to our right (and forward)
		hit = 0;
		sx = e->tileX;
		sy = e->tileY;

		while (!hit) {
			bg = g_hdb->_map->getMapBGTileFlags(sx + xv, sy + yv) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e1 = g_hdb->_ai->findEntity(sx + xv, sy + yv);
			if (e1 && e1 == p)
				e1 = NULL;
			bg2 = g_hdb->_map->getMapBGTileFlags(sx + xv2, sy + yv2) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e2 = g_hdb->_ai->findEntity(sx + xv2, sy + yv2);
			if (e2 && e2 == p)
				e2 = NULL;
			bg3 = g_hdb->_map->getMapBGTileFlags(sx + xv3, sy + yv3) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e3 = g_hdb->_ai->findEntity(sx + xv3, sy + yv3);
			if (e3 && e3 == p)
				e3 = NULL;

			// Okay to move forward?
			if ((!bg && !e1) && (bg2 || e2 || bg3 || e3)) {
				sx += xv;
				sy += yv;
				rotate = 0;
			} else
				hit = 1;
		}

		// Are we stuck in a corner?
		if (sx == e->tileX && sy == e->tileY) {
			sx = e->tileX;
			sy = e->tileY;
			rotate += 1;

			// Need to check for turning RIGHT when we're in a corner
			xv = xvToL[e->dir];
			yv = yvToL[e->dir];

			// Check Tile flags to our left and right
			bg = g_hdb->_map->getMapBGTileFlags(sx + xv, sy + yv) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e1 = g_hdb->_ai->findEntity(sx + xv, sy + yv);
			bg2 = g_hdb->_map->getMapBGTileFlags(sx + xv3, sy + yv3) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e2 = g_hdb->_ai->findEntity(sx + xv3, sy + yv3);
			if (e1 && e1->type == AI_GUY)
				e1 = NULL;
			if (e2 && e2->type == AI_GUY)
				e2 = NULL;

			// Is tile to the right clear?
			// Is tile to the left clear?
			// If neither, go backwards
			if (!bg2 && !e2) {
				switch (e->dir) {
				case DIR_UP: e->dir = DIR_RIGHT; break;
				case DIR_DOWN: e->dir = DIR_LEFT; break;
				case DIR_LEFT: e->dir = DIR_UP; break;
				case DIR_RIGHT: e->dir = DIR_DOWN; break;
				case DIR_NONE:
					warning("aiRightBotFindGoal: DIR_NONE found");
				}
			} else if (!bg && !e1) {
				switch (e->dir) {
				case DIR_UP: e->dir = DIR_LEFT; break;
				case DIR_DOWN: e->dir = DIR_RIGHT; break;
				case DIR_LEFT: e->dir = DIR_DOWN; break;
				case DIR_RIGHT: e->dir = DIR_UP; break;
				case DIR_NONE:
					warning("aiRightBotFindGoal: DIR_NONE found");
				}
			} else {
				switch (e->dir) {
				case DIR_UP: e->dir = DIR_DOWN; yv = 1; xv = 0; break;
				case DIR_DOWN: e->dir = DIR_UP; yv = -1; xv = 0; break;
				case DIR_LEFT: e->dir = DIR_RIGHT; yv = 0; xv = 1; break;
				case DIR_RIGHT: e->dir = DIR_LEFT; yv = 0; xv = -1; break;
				case DIR_NONE:
					warning("aiRightBotFindGoal: DIR_NONE found");
				}
				sx += xv;
				sy += yv;
				rotate = 4;
			}
		}
	} while (rotate >= 1 && rotate < 4);

	switch (e->dir) {
	case DIR_UP:
		e->state = STATE_MOVEUP;
		break;
	case DIR_DOWN:
		e->state = STATE_MOVEDOWN;
		break;
	case DIR_LEFT:
		e->state = STATE_MOVELEFT;
		break;
	case DIR_RIGHT:
		e->state = STATE_MOVERIGHT;
		break;
	case DIR_NONE:
		warning("aiRightBotFindGoal: DIR_NONE found");
		break;
	}

	e->goalX = sx;
	e->goalY = sy;
	e->xVel = xv * e->moveSpeed;
	e->yVel = yv * e->moveSpeed;
	if (e->onScreen)
		warning("STUB: aiRightBotFindGoal: Play SND_RIGHTBOT_TURN");
}

void aiRightBotAction(AIEntity *e) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	if (e->goalX) {
		if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 0) && p->state != STATE_DEAD && p->level == e->level && !g_hdb->_ai->playerDead())
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
		g_hdb->_ai->animateEntity(e);
	} else {
		aiRightBotFindGoal(e);
		g_hdb->_ai->animEntFrames(e);
	}
}

void aiPushBotInit(AIEntity *e) {
	if (e->value1 != 1)
		e->aiAction = aiPushBotAction;
}

void aiPushBotInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiPushBotAction(AIEntity *e) {
	AIState moveState[5] = { STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT };
	int	xvAhead[5] = { 9, 0, 0,-1, 1 }, yvAhead[5] = { 9,-1, 1, 0, 0 };
	AIDir oneEighty[5] = { DIR_NONE, DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };

	uint32 bgFlags, fgFlags;
	int nx, ny, nx2, ny2, result;
	AIEntity *e1 = NULL, *e2;

	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead())
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
	} else {
		if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead())
			g_hdb->_ai->killPlayer(DEATH_NORMAL);

		// Where to go next
		nx = e->tileX + xvAhead[e->dir];
		ny = e->tileY + yvAhead[e->dir];

		e1 = g_hdb->_ai->legalMove(nx, ny, e->level, &result);

		// Push something
		// Turn Around
		// Move Forward
		if (e1 && onEvenTile(e1->x, e1->y) && (e1->type == AI_LIGHTBARREL || e1->type == AI_HEAVYBARREL || e1->type == AI_BOOMBARREL || e1->type == AI_CRATE)) {
			// Actually going over a floating crate?
			if (e1 && (e1->state == STATE_FLOATING || e1->state == STATE_MELTED)) {
				e->state = moveState[e->dir];
				g_hdb->_ai->setEntityGoal(e, nx, ny);
				g_hdb->_ai->animateEntity(e);
				return;
			}

			nx2 = nx + xvAhead[e->dir];
			ny2 = ny + yvAhead[e->dir];

			bgFlags = g_hdb->_map->getMapBGTileFlags(nx2, ny2);
			fgFlags = g_hdb->_map->getMapFGTileFlags(nx2, ny2);
			e2 = g_hdb->_ai->findEntity(nx2, ny2);
			result = (e->level == 1) ? (bgFlags & kFlagSolid) : !(fgFlags & kFlagGrating) && (bgFlags & kFlagSolid);

			// If we're going to push something onto a floating thing, that's ok
			if (e2 && (e2->state == STATE_FLOATING || e2->state == STATE_MELTED))
				e2 = NULL;

			// If no walls in front & no entities
			if (!result && !e2 && e1->state != STATE_EXPLODING) {
				e->state = moveState[e->dir];
				g_hdb->_ai->setEntityGoal(e, nx, ny);

				e1->dir = e->dir;
				e1->state = e->state;
				e1->moveSpeed = e->moveSpeed;
				g_hdb->_ai->setEntityGoal(e1, nx2, ny2);
				warning("STUB: aiPushBotAction: Switch Case for Sound");
			} else {
				if (e->onScreen)
					warning("STUB: aiPushBotAction: Play SND_PUSHBOT_STRAIN");
				e->dir = oneEighty[e->dir];
				e->state = moveState[e->dir];
				nx = e->tileX + xvAhead[e->dir];
				ny = e->tileY + yvAhead[e->dir];
				e1 = g_hdb->_ai->legalMove(nx, ny, e->level, &result);
				if (!e1 && result)
					g_hdb->_ai->setEntityGoal(e, nx, ny);
			}
		} else if (!result || (e1 && !onEvenTile(e1->x, e1->y))) {
			e->dir = oneEighty[e->dir];
			e->state = moveState[e->dir];
			nx = e->tileX + xvAhead[e->dir];
			ny = e->tileY + yvAhead[e->dir];
			e1 = g_hdb->_ai->legalMove(nx, ny, e->level, &result);
			if (!e1 && result)
				g_hdb->_ai->setEntityGoal(e, nx, ny);
		} else {
			e->state = moveState[e->dir];
			g_hdb->_ai->setEntityGoal(e, nx, ny);
		}
		g_hdb->_ai->animateEntity(e);
	}
}

void aiRailRiderInit(AIEntity *e) {
	if (e->type == AI_RAILRIDER_ON) {
		// On the tracks already - spawn RED arrow
		g_hdb->_ai->addToPathList(e->tileX, e->tileY, 0, e->dir);
		e->state = STATE_STANDUP;
		e->aiAction = aiRailRiderOnAction;
		e->aiUse = aiRailRiderOnUse;
	} else {
		e->state = STATE_STANDDOWN;
		e->sequence = 0;
		e->aiAction = aiRailRiderAction;
		e->aiUse = aiRailRiderUse;
	}
	e->moveSpeed = kPlayerMoveSpeed;
}

void aiRailRiderInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

// Talking to RailRider off track
void aiRailRiderUse(AIEntity *e) {
	e->sequence = 1;
}

void aiRailRiderAction(AIEntity *e) {
	switch (e->sequence) {
		// Waiting for Dialog to goaway
	case 1:
		// Dialog gone?
		if (!g_hdb->_window->dialogActive()) {
			e->sequence = 2;
			switch (e->dir) {
			case DIR_UP:	e->xVel = 0;	e->yVel = -1;	break;
			case DIR_DOWN:	e->xVel = 0;	e->yVel = 1;	break;
			case DIR_LEFT:	e->xVel = -1;	e->yVel = 0;	break;
			case DIR_RIGHT:	e->xVel = 1;	e->yVel = 0;	break;
			case DIR_NONE:
				warning("aiRailRiderAction: DIR_NONE found");
				break;
			}
		}
		break;
		// Walking over to track
	case 2:
		e->x += e->xVel;
		e->y += e->yVel;
		if (onEvenTile(e->x, e->y)) {
			ArrowPath *arrowPath;
			e->tileX = e->x / kTileWidth;
			e->tileY = e->y / kTileHeight;
			e->sequence = 3;	// Wait for use
			e->type = AI_RAILRIDER_ON;
			e->state = STATE_STANDUP;
			e->aiAction = aiRailRiderOnAction;
			e->aiUse = aiRailRiderOnUse;
			arrowPath = g_hdb->_ai->findArrowPath(e->tileX, e->tileY);
			e->dir = arrowPath->dir;
			e->value1 = 0;	// Not in a tunnel
		}
		break;
	}

	// Cycle through animation frames
	if (e->animDelay-- > 0)
		return;
	e->animDelay = e->animCycle;
	e->animFrame++;
	if (e->animFrame == e->standdownFrames)
		e->animFrame = 0;

	e->draw = e->standdownGfx[e->animFrame];
}

// Talking to RailRider on track
void aiRailRiderOnUse(AIEntity *e) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	if (p->tileX == e->tileX) {
		if (p->tileY > e->tileY)
			g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY - 1);
		else
			g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY + 1);
	} else {
		if (p->tileX > e->tileX)
			g_hdb->_ai->setEntityGoal(p, p->tileX - 1, p->tileY);
		else
			g_hdb->_ai->setEntityGoal(p, p->tileX + 1, p->tileY);
	}

	e->sequence = -1;	// Waiting for player to board
}

void aiRailRiderOnAction(AIEntity *e) {
	int	xv[5] = { 9, 0, 0, -1, 1 }, yv[5] = { 9, -1, 1, 0, 0 };
	AIEntity*p = g_hdb->_ai->getPlayer();
	SingleTele t;

	switch (e->sequence) {
	// Player is boarding
	case -1:
		if (!p->goalX)
			e->sequence = 1; // Boarded yet?
	// Cycle Animation Frames
	case 3:
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle;
		e->animFrame++;
		if (e->animFrame == e->standupFrames)
			e->animFrame = 0;

		e->draw = e->standupGfx[e->animFrame];
		break;
	// Player is in - lock him
	case 1:
		g_hdb->_ai->setPlayerInvisible(true);
		g_hdb->_ai->setPlayerLock(true);
		g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
		warning("STUB: aiRailRiderOnAction: Play SND_RAILRIDER_TASTE");
		e->sequence = 2;
		e->value1 = 0;

	// New RailRider gfx
	// Move the RailRider
	case 2:
		// Done moving to next spot?
		if (!e->goalX) {
			ArrowPath *arrowPath = g_hdb->_ai->findArrowPath(e->tileX, e->tileY);
			if (arrowPath) {
				// Stop Arrow?
				if (!arrowPath->type) {
					HereT *h;
					e->sequence = 4;	// Get Player off RailRider - RIGHT SIDE ONLY
					p->tileX = e->tileX;
					p->tileY = e->tileY;
					p->x = e->x;
					p->y = e->y;
					// Try to find a HERE icon to either side of the track and go there
					switch (e->dir) {
					case DIR_UP:
						h = g_hdb->_ai->findHere(e->tileX - 1, e->tileY);
						if (h)
							g_hdb->_ai->setEntityGoal(p, e->tileX - 1, e->tileY);
						else
							g_hdb->_ai->setEntityGoal(p, e->tileX + 1, e->tileY);
						break;
					case DIR_DOWN:
						h = g_hdb->_ai->findHere(e->tileX + 1, e->tileY);
						if (h)
							g_hdb->_ai->setEntityGoal(p, e->tileX + 1, e->tileY);
						else
							g_hdb->_ai->setEntityGoal(p, e->tileX - 1, e->tileY);
						break;
					case DIR_LEFT:
						h = g_hdb->_ai->findHere(e->tileX, e->tileY + 1);
						if (h)
							g_hdb->_ai->setEntityGoal(p, e->tileX, e->tileY + 1);
						else
							g_hdb->_ai->setEntityGoal(p, e->tileX, e->tileY - 1);
						break;
					case DIR_RIGHT:
						h = g_hdb->_ai->findHere(e->tileX, e->tileY - 1);
						if (h)
							g_hdb->_ai->setEntityGoal(p, e->tileX, e->tileY - 1);
						else
							g_hdb->_ai->setEntityGoal(p, e->tileX, e->tileY + 1);
						break;
					case DIR_NONE:
						warning("aiRailRiderOnAction: DIR_NOW found");
						break;
					}
					g_hdb->_ai->setPlayerInvisible(false);
					warning("STUB: aiRailRiderOnAction: Play SND_RAILRIDER_EXIT");
				} else if (arrowPath->type == 1) {
					e->dir = arrowPath->dir;
					g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
				}
			} else
				g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);

			warning("STUB: aiRailRiderOnAction: Play SND_RAILRIDER_ONTRACK");
		}

		p->tileX = e->tileX;
		p->tileY = e->tileY;
		p->x = e->x;
		p->y = e->y;
		g_hdb->_ai->animateEntity(e);
		switch (e->dir) {
		case DIR_UP:	e->draw = e->moveupGfx[0]; break;
		case DIR_DOWN:	e->draw = e->movedownGfx[0]; break;
		case DIR_LEFT:	e->draw = e->moveleftGfx[0]; break;
		case DIR_RIGHT: e->draw = e->moverightGfx[0]; break;
		}
		g_hdb->_map->centerMapXY(e->x + 16, e->y + 16);

		// Did we hit a tunnel entrance?
		if (onEvenTile(e->x, e->y) && g_hdb->_ai->findTeleporterDest(e->tileX, e->tileY, &t) && !e->value1 && !e->dir2) {
			// Set tunnel destination
			e->value1 = t.x;
			e->value2 = t.y;
			e->dir2 = (AIDir)(t.x + t.y);	// Flag for coming out of tunnel
		}

		// Are we going through a tunnel?
		if (e->value1) {
			// Reach the End?
			// If not, don't draw RailRider
			if (onEvenTile(e->x, e->y) && e->tileX == e->value1 && e->tileY == e->value2)
				e->value1 = 0;
			else
				e->draw = NULL;
		} else if (e->dir2 && e->dir2 != (AIDir)(e->tileX + e->tileY))
			e->dir2 = DIR_NONE;
		break;
	// Waiting for Player to move to Dest
	case 4:
		if (!p->goalX) {
			g_hdb->_ai->setPlayerLock(false);
			e->sequence = 3;	// Wait for Use
		}

		// Cycle Animation frames
		if (e->animDelay-- > 0)
			return;

		e->animDelay = e->animCycle;
		e->animFrame++;
		if (e->animFrame == e->standupFrames)
			e->animFrame = 0;

		e->draw = e->standupGfx[e->animFrame];
		break;
	}
}

void aiMaintBotInit(AIEntity *e) {
	// value1 field determines whether the "MMM!" sound plays
	// 1 means NO
	e->int1 = e->value1;
	e->aiAction = aiMaintBotAction;
	e->value1 = 0;
	g_hdb->_ai->findPath(e);
}

void aiMaintBotInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiMaintBotAction(AIEntity *e) {
	AIState useState[5] = {STATE_NONE, STATE_USEUP, STATE_USEDOWN, STATE_USELEFT, STATE_USERIGHT};
	AIState standState[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	int	xvAhead[5] = {9, 0, 0,-1, 1}, yvAhead[5] = {9,-1, 1, 0, 0};
	AIEntity *it;
	int nx, ny;
	warning("STUB: aiMaintBotAction: Add sounds");

	// Waiting at an arrow (or hit by player)?
	if (e->sequence) {
		e->sequence--;
		g_hdb->_ai->animEntFrames(e);

		// Use Something here
		if (!e->value2)
			switch (e->sequence) {
			case 50:
				if (e->onScreen && !e->int1) {
					warning("STUB: aiMaintBotAction: Play SND_MBOT_HMMM or SND_MBOT_HMMM2");
				}
				break;
			// Need to USE the object
			case 30:
				e->state = useState[e->dir];
				nx = e->tileX + xvAhead[e->dir];
				ny = e->tileY + yvAhead[e->dir];
				it = g_hdb->_ai->findEntity(nx, ny);
				if (it) {
					if (e->onScreen)
						e->value1 = 1;
					g_hdb->useEntity(it);
					break;
				}
				// Did the MaintBot use an Action Tile?
				if (g_hdb->_ai->checkActionList(e, nx, ny, true)) {
					if (e->onScreen)
						e->value1 = 1;
					break;
				}
				// Did the MaintBot use an AutoAction Tile?
				if (g_hdb->_ai->checkAutoList(e, nx, ny)) {
					if (e->onScreen)
						e->value1 = 1;
					break;
				}
				// Did the MaintBot use a LUA Tile?
				warning("STUB: aiMaintBotAction: Check in LUA List");
				break;
			// Play a sound if we used something
			case 25:
				e->value1 = 0;
				break;
			// Change to Standing frames
			case 20:
				e->state = standState[e->dir];
				break;
			// All done - find a new path
			case 0:
				e->dir = e->dir2;
				g_hdb->_ai->findPath(e);
				g_hdb->_ai->animateEntity(e);
				break;
			}
		// Deciding where to go at 4-way
		else {
			AIDir lookRight[5] = {DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};
			AIDir lookLeft[5] = {DIR_NONE, DIR_LEFT, DIR_RIGHT, DIR_DOWN, DIR_UP};
			AIDir dirList[5] = {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};
			switch (e->sequence) {
			// HMM
			case 50:
				if (e->onScreen && !e->int1)
					warning("STUB: aiMaintBotAction: Play SND_MBOT_HMMM");
				break;
			// Look Right
			case 40:
				e->dir = lookRight[e->dir2];
				e->state = standState[e->dir];
				break;
			// Look Left
			case 30:
				e->dir = lookLeft[e->dir];
				e->state = standState[e->dir];
				break;
			// HMM2
			case 25:
				if (e->onScreen && !e->int1)
					warning("STUB: aiMaintBotAction: Play SND_MBOT_HMM2");
				break;
			// Decide direction and GO
			case 0:
				int dir = (g_hdb->_rnd->getRandomNumber(4)) + 1;
				e->dir = dirList[dir];
				g_hdb->_ai->findPath(e);
				if (e->onScreen)
					warning("STUB: aiMaintBotAction: Play whistle");
				break;
			}
		}
		return;
	}

	// Moving already, keep going
	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 4) && !g_hdb->_ai->playerDead()) {
			g_hdb->_ai->killPlayer(DEATH_GRABBED);
			warning("STUB: aiMaintBotAction: Play SND_MBOT_DEATH");
		}
	} else {
		// Check if there's an arrow UNDER the bot, and if its RED
		// If so, turn in that direction and use something
		ArrowPath *ar = g_hdb->_ai->findArrowPath(e->tileX, e->tileY);
		if (ar) {
			// STOP		Arrow
			// GO		Arrow
			// 4-way	Arrow
			if (!ar->type) {
				e->dir2 = e->dir; // dir2 holds the last direction we were travelling in
				e->dir = ar->dir;
				e->sequence = 64; // sequence is the timer of events
				e->state = standState[e->dir];
				e->value2 = 0;
				return;
			} else if (ar->type == 1) {
				g_hdb->_ai->findPath(e);
				warning("STUB: aiMaintBotAction: Play whistle");
			} else {
				e->sequence = 64;
				e->dir2 = e->dir;
				e->value2 = 1;
				return;
			}
		}
		g_hdb->_ai->animateEntity(e);
	}
}

void aiFourFirerInit(AIEntity *e) {
	e->value1 = 0;
	e->aiAction = aiFourFirerAction;
}

void aiFourFirerInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiFourFirerAction(AIEntity *e) {
	AIEntity *p = g_hdb->_ai->getPlayer(), *fire, *hit;
	AIState state[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	AIDir turn[5] = {DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};
	int	shoot, xv, yv, result;

	// Time to turn right?
	if (!e->value1) {
		e->dir = turn[e->dir];
		e->state = state[e->dir];
		e->value1 = 16;
		if (e->onScreen)
			warning("STUB: aiFourFirerAction: Play SND_FOURFIRE_TURN");
	}
	e->value1--;

	// Waiting before firing again?
	if (e->sequence) {
		e->sequence--;
		return;
	}

	g_hdb->_ai->animEntFrames(e);

	// Can we see the player on the same level?
	if ((e->level != p->level) || g_hdb->_ai->playerDead() || !e->onScreen)
		return;

	// Check player direction
	shoot = xv = yv = 0;
	switch (e->dir) {
	case DIR_UP:	if (p->x == e->x && p->y < e->y) { shoot = 1; yv = -1; } break;
	case DIR_DOWN:	if (p->x == e->x && p->y > e->y) { shoot = 1; yv = 1; } break;
	case DIR_LEFT:	if (p->y == e->y && p->x < e->x) { shoot = 1; xv = -1; } break;
	case DIR_RIGHT:	if (p->y == e->y && p->x > e->x) { shoot = 1; xv = 1; } break;
	case DIR_NONE: warning("aiFourFirerAction: DIR_NONE found"); break;
	}

	// Shoot if needed
	// Make sure not shooting into solid tile
	// Make sure if shooting at entity it is the player
	hit = g_hdb->_ai->legalMoveOverWater(e->tileX + xv, e->tileY + yv, e->level, &result);
	if (hit && hit->type == AI_GUY)
		hit = NULL;

	if (shoot && !hit && result) {
		fire = g_hdb->_ai->spawn(AI_OMNIBOT_MISSILE, e->dir, e->tileX + xv, e->tileY + yv, NULL, NULL, NULL, DIR_NONE, e->level, 0, 0, 1);
		if (g_hdb->_map->onScreen(e->tileX, e->tileY))
			warning("STUB: aiFourFirerAction: Play SND_FOUR_FIRE");
		fire->xVel = xv * kPlayerMoveSpeed * 2;
		fire->yVel = yv * kPlayerMoveSpeed * 2;
		if (!g_hdb->getActionMode()) {
			fire->xVel >>= 1;
			fire->yVel >>= 1;
		}
		e->sequence = 16;
		if (hitPlayer(fire->tileX*kTileWidth, fire->tileY*kTileHeight))
			g_hdb->_ai->killPlayer(DEATH_FRIED);
	}
}

} // End of Namespace
