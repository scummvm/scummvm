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

} // End of Namespace
