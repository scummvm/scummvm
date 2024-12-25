/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "common/random.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/ai-player.h"
#include "hdb/gfx.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/mpc.h"
#include "hdb/sound.h"
#include "hdb/window.h"

namespace HDB {

//-------------------------------------------------------------------
//
//	OMNIBOT : This guy moves on a path and if he sees the player
//		directly ahead, he will shoot at him
//
//-------------------------------------------------------------------

void aiOmniBotInit(AIEntity *e, int mx, int my) {
	if (e->value1 == 1)
		e->aiAction = aiOmniBotMove;
	else if (g_hdb->_ai->findPath(e))
		e->aiAction = aiOmniBotAction;
}

void aiOmniBotInit2(AIEntity *e, int mx, int my) {
	e->standdownGfx[0] = e->movedownGfx[0];
	e->standupGfx[0] = e->movedownGfx[0];
	e->standleftGfx[0] = e->moveleftGfx[0];
	e->standrightGfx[0] = e->moverightGfx[0];
	e->standdownFrames = e->standupFrames = e->standleftFrames = e->standrightFrames = 1;
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiOmniBotMove(AIEntity *e, int mx, int my) {
	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else
		g_hdb->_ai->animEntFrames(e);
}

void aiOmniBotAction(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();
	if (e->goalX) {
		if (!e->sequence) {
			g_hdb->_ai->animateEntity(e);
			// Is the Player collding?
			if (hitPlayer(e->x, e->y) && (p->level == e->level)) {
				g_hdb->_ai->killPlayer(DEATH_FRIED);
				return;
			}

			// Shoot player ?
			if (onEvenTile(e->x, e->y) && g_hdb->getActionMode()) {
				int xv = 0, yv = 0, result;
				bool shoot = false;

				// FIXME: Is reloading Player required here?
				p = g_hdb->_ai->getPlayer();

				// On same level/screen?
				if ((e->level != p->level) || g_hdb->_ai->playerDead() || !e->onScreen)
					return;

				// Is Player in Line of Sight?
				switch (e->dir) {
				case DIR_UP:
					if (p->x == e->x && p->y < e->y) {
						shoot = true;
						yv = -1;
					}
					break;
				case DIR_DOWN:
					if (p->x == e->x && p->y > e->y) {
						shoot = true;
						yv = 1;
					}
					break;
				case DIR_LEFT:
					if (p->x < e->x && p->y == e->y) {
						shoot = true;
						xv = -1;
					}
					break;
				case DIR_RIGHT:
					if (p->x > e->x && p->y == e->y) {
						shoot = true;
						xv = 1;
					}
					break;
				case DIR_NONE:
				default:
					break;
				}

				// If shoot = true, take the shot
				// (1) Check we're not shooting into a solid tile
				// (2) Check we're not shooting into an Entity unless it's the player
				AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + xv, e->tileY + yv, e->level, &result);
				if (shoot && !hit && result) {
					AIEntity *omni = g_hdb->_ai->spawn(AI_OMNIBOT_MISSILE, e->dir, e->tileX + xv, e->tileY + yv, nullptr, nullptr, nullptr, DIR_NONE, e->level, 0, 0, 1);
					omni->xVel = xv * kPlayerMoveSpeed * 2;
					omni->yVel = yv * kPlayerMoveSpeed * 2;
					if (g_hdb->_map->onScreen(e->tileX, e->tileY))
						g_hdb->_sound->playSound(SND_OMNIBOT_FIRE);
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
			g_hdb->_sound->playSound(SND_OMNIBOT_AMBIENT);
	}

	if (e->sequence)
		e->sequence--;
}

//-------------------------------------------------------------------
//
//	OMNIBOT MISSILE : Used by the FOURFIRER and OMNIBOT, this deadly
//		missile flies through the air, killing anything it hits
//
//-------------------------------------------------------------------

void aiOmniBotMissileInit(AIEntity *e, int mx, int my) {
	e->state = STATE_MOVEDOWN;
	e->aiAction = aiOmniBotMissileAction;
}

void aiOmniBotMissileInit2(AIEntity *e, int mx, int my) {
	for (int i = 0; i < e->movedownFrames; i++)
		e->moveleftGfx[i] = e->moverightGfx[i] = e->moveupGfx[i] = e->movedownGfx[i];

	e->moveleftFrames = e->moverightFrames = e->moveupFrames = e->movedownFrames;
	e->draw = e->movedownGfx[0];
}

void aiOmniBotMissileAction(AIEntity *e, int mx, int my) {
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
		if (hitPlayer(e->x, e->y) && (p->level == e->level)) {
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_FAST, false, false, "steam_puff_sit");
			g_hdb->_ai->removeEntity(e);
		}
}

//-------------------------------------------------------------------
//
//	TURNBOT : Moves straight ahead until it hits a wall, then turns
//		right and continues.
//
//-------------------------------------------------------------------

void aiTurnBotInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiTurnBotAction;
}

void aiTurnBotInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiTurnBotChoose(AIEntity *e, int mx, int my) {
	static const int xvAhead[5] = { 9, 0, 0, -1, 1 };
	static const int yvAhead[5] = { 9, -1, 1, 0, 0 };
	static const AIDir turnRight[5] = { DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN };
	static const AIState dirState[5] = { STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT };

	int xv = xvAhead[e->dir];
	int yv = yvAhead[e->dir];
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

void aiTurnBotAction(AIEntity *e, int mx, int my) {
	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else {
		aiTurnBotChoose(e, 0, 0);
		g_hdb->_ai->animateEntity(e);
		if (e->onScreen)
			g_hdb->_sound->playSound(SND_TURNBOT_TURN);
	}

	if (e->onScreen && onEvenTile(e->x, e->y) && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 0) && !g_hdb->_ai->playerDead())
		g_hdb->_ai->killPlayer(DEATH_NORMAL);
}

//-------------------------------------------------------------------
//
//	SHOCKBOT : Moves on a path, electrifying all tiles surrounding it
//		that are METAL.  Will pause when changing directions.
//
//-------------------------------------------------------------------

void aiShockBotInit(AIEntity *e, int mx, int my) {
	g_hdb->_ai->findPath(e);
	e->aiAction = aiShockBotAction;
	e->animCycle = 0;
	e->sequence = 0;
	e->aiDraw = aiShockBotShock;
}

void aiShockBotInit2(AIEntity *e, int mx, int my) {
	e->standupFrames = e->standdownFrames = e->standleftFrames = e->standrightFrames =
		e->moveupFrames = e->moverightFrames = e->moveleftFrames = e->movedownFrames;

	for (int i = 0; i < e->movedownFrames; i++)
		e->standupGfx[i] = e->standleftGfx[i] = e->standrightGfx[i] = e->standdownGfx[i] = e->moveupGfx[i] = e->moveleftGfx[i] = e->moverightGfx[i] = e->movedownGfx[i];

	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiShockBotAction(AIEntity *e, int mx, int my) {
	if (e->goalX) {
		if (!e->sequence) {
			if (hitPlayer(e->x, e->y))
				g_hdb->_ai->killPlayer(DEATH_SHOCKED);
			g_hdb->_ai->animateEntity(e);
		} else
			g_hdb->_ai->animEntFrames(e);
	} else {
		g_hdb->_ai->findPath(e);
		e->sequence = 20;
		g_hdb->_ai->animEntFrames(e);
		if (e->onScreen)
			g_hdb->_sound->playSound(SND_SHOCKBOT_AMBIENT);

	}

	if (e->sequence)
		e->sequence--;
}

void aiShockBotShock(AIEntity *e, int mx, int my) {
	static const int offX[8] = { -1, 0, 1, 1, 1, 0, -1, -1 };
	static const int offY[8] = { -1, -1, -1, 0, 1, 1, 1, 0 };

	// Only on a exact tile boundary do we change the shocked tiles
	// Start at top left and go around
	if (g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY) & kFlagMetal)
		e->special1Gfx[e->animFrame]->drawMasked(e->tileX * kTileWidth - mx, e->tileY * kTileHeight - my);

	for (int i = 0; i < 8; i++) {
		uint32 flags = g_hdb->_map->getMapBGTileFlags(e->tileX + offX[i], e->tileY + offY[i]);
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
					aiBarrelExplode(e2, 0, 0);
				}
			}
		}
	}
}

//-------------------------------------------------------------------
//
//	RIGHTBOT
//
//	Rules: Follows the right-hand wall.  That's it!
//
//-------------------------------------------------------------------

void aiRightBotInit(AIEntity *e, int mx, int my) {
	e->moveSpeed = kPlayerMoveSpeed;
	if (!g_hdb->getActionMode())
		e->moveSpeed >>= 1;
	e->aiAction = aiRightBotAction;
}

void aiRightBotInit2(AIEntity *e, int mx, int my) {
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
	default:
		break;
	}
}

void aiRightBotFindGoal(AIEntity *e, int mx, int my) {
	static const int xvAhead[5] = { 9, 0, 0,-1, 1 };
	static const int yvAhead[5] = { 9,-1, 1, 0, 0 };
	static const int xvAToR[5]  = { 9, 1,-1,-1, 1 };
	static const int yvAToR[5]  = { 9,-1, 1,-1, 1 };
	static const int xvToR[5]   = { 9, 1,-1, 0, 0 };
	static const int yvToR[5]   = { 9, 0, 0,-1, 1 };
	static const int xvToL[5]   = { 9,-1, 1, 0, 0 };
	static const int yvToL[5]   = { 9, 0, 0, 1,-1 };

	AIEntity *p = g_hdb->_ai->getPlayer();
	int rotate = 0;

	int	xv, yv;
	int	bg, bg2, bg3;
	AIEntity *e1, *e2, *e3;
	int	sx, sy;

	do {
		xv = xvAhead[e->dir];	// Search Ahead
		yv = yvAhead[e->dir];
		int xv2 = xvAToR[e->dir];	// Search Ahead and to the Right
		int yv2 = yvAToR[e->dir];
		int xv3 = xvToR[e->dir];	// Search to the Right
		int yv3 = yvToR[e->dir];

		// Search until we hit a wall...or empty space to our right (and forward)
		bool hit = false;
		sx = e->tileX;
		sy = e->tileY;

		while (!hit) {
			bg = g_hdb->_map->getMapBGTileFlags(sx + xv, sy + yv) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e1 = g_hdb->_ai->findEntity(sx + xv, sy + yv);
			if (e1 && e1 == p)
				e1 = nullptr;
			bg2 = g_hdb->_map->getMapBGTileFlags(sx + xv2, sy + yv2) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e2 = g_hdb->_ai->findEntity(sx + xv2, sy + yv2);
			if (e2 && e2 == p)
				e2 = nullptr;
			bg3 = g_hdb->_map->getMapBGTileFlags(sx + xv3, sy + yv3) & (kFlagSolid | kFlagWater | kFlagSlime | kFlagSpecial);
			e3 = g_hdb->_ai->findEntity(sx + xv3, sy + yv3);
			if (e3 && e3 == p)
				e3 = nullptr;

			// Okay to move forward?
			if ((!bg && !e1) && (bg2 || e2 || bg3 || e3)) {
				sx += xv;
				sy += yv;
				rotate = 0;
			} else
				hit = true;
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
				e1 = nullptr;
			if (e2 && e2->type == AI_GUY)
				e2 = nullptr;

			// Is tile to the right clear?
			// Is tile to the left clear?
			// If neither, go backwards
			if (!bg2 && !e2) {
				switch (e->dir) {
				case DIR_UP:
					e->dir = DIR_RIGHT;
					break;
				case DIR_DOWN:
					e->dir = DIR_LEFT;
					break;
				case DIR_LEFT:
					e->dir = DIR_UP;
					break;
				case DIR_RIGHT:
					e->dir = DIR_DOWN;
					break;
				case DIR_NONE:
				default:
					break;
				}
			} else if (!bg && !e1) {
				switch (e->dir) {
				case DIR_UP:
					e->dir = DIR_LEFT;
					break;
				case DIR_DOWN:
					e->dir = DIR_RIGHT;
					break;
				case DIR_LEFT:
					e->dir = DIR_DOWN;
					break;
				case DIR_RIGHT:
					e->dir = DIR_UP;
					break;
				case DIR_NONE:
				default:
					break;
				}
			} else {
				switch (e->dir) {
				case DIR_UP:
					e->dir = DIR_DOWN;
					yv = 1;
					xv = 0;
					break;
				case DIR_DOWN:
					e->dir = DIR_UP;
					yv = -1;
					xv = 0;
					break;
				case DIR_LEFT:
					e->dir = DIR_RIGHT;
					yv = 0;
					xv = 1;
					break;
				case DIR_RIGHT:
					e->dir = DIR_LEFT;
					yv = 0;
					xv = -1;
					break;
				case DIR_NONE:
				default:
					break;
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
	default:
		break;
	}

	e->goalX = sx;
	e->goalY = sy;
	e->xVel = xv * e->moveSpeed;
	e->yVel = yv * e->moveSpeed;
	if (e->onScreen)
		g_hdb->_sound->playSound(SND_RIGHTBOT_TURN);
}

void aiRightBotAction(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	if (e->goalX) {
		if (e->onScreen && g_hdb->_ai->checkPlayerCollision(e->x, e->y, 0) && p->state != STATE_DEAD && p->level == e->level && !g_hdb->_ai->playerDead())
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
		g_hdb->_ai->animateEntity(e);
	} else {
		aiRightBotFindGoal(e, 0, 0);
		g_hdb->_ai->animEntFrames(e);
	}
}

//-------------------------------------------------------------------
//
//	PUSHBOT : Very simple, this guy goes forward and pushes anything in his
//		path all the way until it can't go any further.  Then, he turns 180
//		degrees and comes back until he can't go any further.  Then... he
//		turns 180 degrees and does it all over again!
//
//-------------------------------------------------------------------

void aiPushBotInit(AIEntity *e, int mx, int my) {
	if (e->value1 != 1)
		e->aiAction = aiPushBotAction;
}

void aiPushBotInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiPushBotAction(AIEntity *e, int mx, int my) {
	static const AIState moveState[5] = { STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT };
	static const int xvAhead[5] = { 9, 0, 0,-1, 1 };
	static const int yvAhead[5] = { 9,-1, 1, 0, 0 };
	static const AIDir oneEighty[5] = { DIR_NONE, DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT };

	AIEntity *e1 = nullptr;

	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		if (hitPlayer(e->x, e->y))
			g_hdb->_ai->killPlayer(DEATH_NORMAL);
	} else {
		if (hitPlayer(e->x, e->y))
			g_hdb->_ai->killPlayer(DEATH_NORMAL);

		// Where to go next
		int nx = e->tileX + xvAhead[e->dir];
		int ny = e->tileY + yvAhead[e->dir];

		int result;
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

			int nx2 = nx + xvAhead[e->dir];
			int ny2 = ny + yvAhead[e->dir];

			uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(nx2, ny2);
			uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(nx2, ny2);
			AIEntity *e2 = g_hdb->_ai->findEntity(nx2, ny2);
			result = (e->level == 1) ? (bgFlags & kFlagSolid) : !(fgFlags & kFlagGrating) && (bgFlags & kFlagSolid);

			// If we're going to push something onto a floating thing, that's ok
			if (e2 && (e2->state == STATE_FLOATING || e2->state == STATE_MELTED))
				e2 = nullptr;

			// If no walls in front & no entities
			if (!result && !e2 && e1->state != STATE_EXPLODING) {
				e->state = moveState[e->dir];
				g_hdb->_ai->setEntityGoal(e, nx, ny);

				e1->dir = e->dir;
				e1->state = e->state;
				e1->moveSpeed = e->moveSpeed;
				g_hdb->_ai->setEntityGoal(e1, nx2, ny2);
				switch (e1->type) {
				case AI_CRATE:
					g_hdb->_sound->playSound(SND_CRATE_SLIDE);
					break;
				case AI_HEAVYBARREL:
				case AI_BOOMBARREL:
					g_hdb->_sound->playSound(SND_HEAVY_SLIDE);
					break;
				case AI_LIGHTBARREL:
					g_hdb->_sound->playSound(SND_LIGHT_SLIDE);
					break;
				default:
					break;
				}
			} else {
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_PUSHBOT_STRAIN);
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

//-------------------------------------------------------------------
//
//	RAILRIDER : crazy green goopy dude -- he gives you rides on his
//		special track!
//
//-------------------------------------------------------------------

void aiRailRiderInit(AIEntity *e, int mx, int my) {
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

void aiRailRiderInit2(AIEntity *e, int mx, int my) {
	e->draw = e->standdownGfx[0];
}

// Talking to RailRider off track
void aiRailRiderUse(AIEntity *e, int mx, int my) {
	e->sequence = 1;
}

void aiRailRiderAction(AIEntity *e, int mx, int my) {
	switch (e->sequence) {
		// Waiting for Dialog to goaway
	case 1:
		// Dialog gone?
		if (!g_hdb->_window->dialogActive()) {
			e->sequence = 2;
			switch (e->dir) {
			case DIR_UP:
				e->xVel = 0;
				e->yVel = -1;
				break;
			case DIR_DOWN:
				e->xVel = 0;
				e->yVel = 1;
				break;
			case DIR_LEFT:
				e->xVel = -1;
				e->yVel = 0;
				break;
			case DIR_RIGHT:
				e->xVel = 1;
				e->yVel = 0;
				break;
			case DIR_NONE:
			default:
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

			if (arrowPath == nullptr)
				return;

			e->dir = arrowPath->dir;
			e->value1 = 0;	// Not in a tunnel
		}
		break;
	default:
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
void aiRailRiderOnUse(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	if (p->tileX == e->tileX) {
		if (p->tileY > e->tileY)
			g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY - 1);
		else
			g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY + 1);
	} else if (p->tileX > e->tileX)
		g_hdb->_ai->setEntityGoal(p, p->tileX - 1, p->tileY);
	else
		g_hdb->_ai->setEntityGoal(p, p->tileX + 1, p->tileY);

	e->sequence = -1;	// Waiting for player to board
}

void aiRailRiderOnAction(AIEntity *e, int mx, int my) {
	static const int xv[5] = { 9, 0, 0, -1, 1 };
	static const int yv[5] = { 9, -1, 1, 0, 0 };

	AIEntity*p = g_hdb->_ai->getPlayer();

	switch (e->sequence) {
	// Player is boarding
	case -1:
		if (!p->goalX)
			e->sequence = 1; // Boarded yet?
		// fallthrough
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
		g_hdb->_sound->playSound(SND_RAILRIDER_TASTE);
		e->sequence = 2;
		e->value1 = 0;
		// fallthrough

	// New RailRider gfx
	// Move the RailRider
	case 2: {
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
					default:
						break;
					}
					g_hdb->_ai->setPlayerInvisible(false);
					g_hdb->_sound->playSound(SND_RAILRIDER_EXIT);
				} else if (arrowPath->type == 1) {
					e->dir = arrowPath->dir;
					g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
				}
			} else
				g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);

			g_hdb->_sound->playSound(SND_RAILRIDER_ONTRACK);
		}

		p->tileX = e->tileX;
		p->tileY = e->tileY;
		p->x = e->x;
		p->y = e->y;
		g_hdb->_ai->animateEntity(e);
		switch (e->dir) {
		case DIR_UP:
			e->draw = e->moveupGfx[0];
			break;
		case DIR_DOWN:
			e->draw = e->movedownGfx[0];
			break;
		case DIR_LEFT:
			e->draw = e->moveleftGfx[0];
			break;
		case DIR_RIGHT:
			e->draw = e->moverightGfx[0];
			break;
		case DIR_NONE:
		default:
			break;
		}
		g_hdb->_map->centerMapXY(e->x + 16, e->y + 16);

		SingleTele t;
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
				e->draw = nullptr;
		} else if (e->dir2 && e->dir2 != (AIDir)(e->tileX + e->tileY))
			e->dir2 = DIR_NONE;
		break;
		}
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
	default:
		break;
	}
}

//-------------------------------------------------------------------
//
//	MAINTBOT : This little fella likes to cause trouble!  He just jubs
//		around the map and looks for stuff to press.  Touch him and you die.
//
//-------------------------------------------------------------------

void aiMaintBotInit(AIEntity *e, int mx, int my) {
	// value1 field determines whether the "MMM!" sound plays
	// 1 means NO
	e->int1 = e->value1;
	e->aiAction = aiMaintBotAction;
	e->value1 = 0;
	g_hdb->_ai->findPath(e);
}

void aiMaintBotInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiMaintBotAction(AIEntity *e, int mx, int my) {
	static const AIState useState[5]   = {STATE_NONE, STATE_USEUP, STATE_USEDOWN, STATE_USELEFT, STATE_USERIGHT};
	static const AIState standState[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	static const int xvAhead[5]  = {9, 0, 0,-1, 1};
	static const int yvAhead[5]  = {9,-1, 1, 0, 0};
	static const int whistles[3] = {SND_MBOT_WHISTLE1, SND_MBOT_WHISTLE2, SND_MBOT_WHISTLE3};
	static const AIDir lookRight[5] = {DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};
	static const AIDir lookLeft[5] = {DIR_NONE, DIR_LEFT, DIR_RIGHT, DIR_DOWN, DIR_UP};
	static const AIDir dirList[5] = {DIR_NONE, DIR_UP, DIR_DOWN, DIR_LEFT, DIR_RIGHT};

	// Waiting at an arrow (or hit by player)?
	if (e->sequence) {
		e->sequence--;
		g_hdb->_ai->animEntFrames(e);

		// Use Something here
		if (!e->value2)
			switch (e->sequence) {
			case 50:
				if (e->onScreen && !e->int1 && !g_hdb->isDemo()) {
					if (g_hdb->_rnd->getRandomNumber(1))
						g_hdb->_sound->playSound(SND_MBOT_HMMM2);
					else
						g_hdb->_sound->playSound(SND_MBOT_HMMM);
				}
				break;
			// Need to USE the object
			case 30: {
				e->state = useState[e->dir];
				int nx = e->tileX + xvAhead[e->dir];
				int ny = e->tileY + yvAhead[e->dir];
				AIEntity *it = g_hdb->_ai->findEntity(nx, ny);
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
				if (g_hdb->_ai->checkLuaList(e, nx, ny)) {
					if (e->onScreen)
						e->value1 = 1;
					break;
				}
				break;
				}
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
			default:
				break;
			}
		// Deciding where to go at 4-way
		else {
			switch (e->sequence) {
			// HMM
			case 50:
				if (e->onScreen && !e->int1 && !g_hdb->isDemo())
					g_hdb->_sound->playSound(SND_MBOT_HMMM);
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
				if (e->onScreen && !e->int1 && !g_hdb->isDemo())
					g_hdb->_sound->playSound(SND_MBOT_HMMM2);
				break;
			// Decide direction and GO
			case 0:
				{
					int dir = (g_hdb->_rnd->getRandomNumber(3)) + 1;
					e->dir = dirList[dir];
					g_hdb->_ai->findPath(e);
					if (e->onScreen && !g_hdb->isDemo())
						g_hdb->_sound->playSound(whistles[g_hdb->_rnd->getRandomNumber(2)]);
				}
				break;
			default:
				break;
			}
		}
		return;
	}

	// Moving already, keep going
	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		if (hitPlayer(e->x, e->y)) {
			g_hdb->_ai->killPlayer(DEATH_GRABBED);
			if (!g_hdb->isDemo())
				g_hdb->_sound->playSound(SND_MBOT_DEATH);
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
				if (!g_hdb->isDemo())
					g_hdb->_sound->playSound(whistles[g_hdb->_rnd->getRandomNumber(2)]);
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

//-------------------------------------------------------------------
//
//	FOURFIRER : This bot turns and fires in the direction it's facing,
//		but only if the player is visible
//
//-------------------------------------------------------------------

void aiFourFirerInit(AIEntity *e, int mx, int my) {
	e->value1 = 0;
	e->aiAction = aiFourFirerAction;
}

void aiFourFirerInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiFourFirerAction(AIEntity *e, int mx, int my) {
	static const AIState state[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	static const AIDir turn[5] = {DIR_NONE, DIR_RIGHT, DIR_LEFT, DIR_UP, DIR_DOWN};

	AIEntity *p = g_hdb->_ai->getPlayer();
	// Time to turn right?
	if (!e->value1) {
		e->dir = turn[e->dir];
		e->state = state[e->dir];
		e->value1 = 16;
		if (e->onScreen)
			g_hdb->_sound->playSound(SND_FOURFIRE_TURN);
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
	bool shoot = false;
	int xv = 0;
	int yv = 0;

	switch (e->dir) {
	case DIR_UP:
		if (p->x == e->x && p->y < e->y) {
			shoot = true;
			yv = -1; }
		break;
	case DIR_DOWN:
		if (p->x == e->x && p->y > e->y) {
			shoot = true;
			yv = 1;
		}
		break;
	case DIR_LEFT:
		if (p->y == e->y && p->x < e->x) {
			shoot = true;
			xv = -1;
		}
		break;
	case DIR_RIGHT:
		if (p->y == e->y && p->x > e->x) {
			shoot = true;
			xv = 1;
		}
		break;
	case DIR_NONE:
	default:
		break;
	}

	// Shoot if needed
	// Make sure not shooting into solid tile
	// Make sure if shooting at entity it is the player
	int result;
	AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + xv, e->tileY + yv, e->level, &result);
	if (hit && hit->type == AI_GUY)
		hit = nullptr;

	if (shoot && !hit && result) {
		AIEntity *fire = g_hdb->_ai->spawn(AI_OMNIBOT_MISSILE, e->dir, e->tileX + xv, e->tileY + yv, nullptr, nullptr, nullptr, DIR_NONE, e->level, 0, 0, 1);
		if (g_hdb->_map->onScreen(e->tileX, e->tileY))
			g_hdb->_sound->playSound(SND_FOUR_FIRE);
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

//-------------------------------------------------------------------
//
//	DEADEYE : Crazy attack dog with Chompie(tm) sounds!  Will sit in one spot
//		looking around, then run in a random direction and distance.  If, while
//		scanning, Deadeye sees the player, he goes nuts and attacks!
//
//-------------------------------------------------------------------

void aiDeadEyeInit(AIEntity *e, int mx, int my) {
	e->sequence = 64;
	e->blinkFrames = e->goalX = 0;
	if (e->value1 == 1)
		e->aiAction = aiDeadEyeWalkInPlace;
	else
		e->aiAction = aiDeadEyeAction;
}

void aiDeadEyeInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiDeadEyeWalkInPlace(AIEntity *e, int mx, int my) {
	static const AIState state[5] = {STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT};

	e->sequence--;

	switch (e->sequence) {
	case 50:
	case 40:
	case 30:
	case 20:
	case 10:
		{
			int rnd = g_hdb->_rnd->getRandomNumber(3) + 1;
			e->dir = (AIDir)rnd;
			e->state = state[rnd];
			if (e->onScreen) {
				if (e->sequence == 50)
					g_hdb->_sound->playSound(SND_DEADEYE_AMB01);
				else if (e->sequence == 10)
					g_hdb->_sound->playSound(SND_DEADEYE_AMB02);
			}
		}
		break;
	case 0:
		e->sequence = 64;
		break;
	default:
		break;
	}
	g_hdb->_ai->animEntFrames(e);
}

void aiDeadEyeAction(AIEntity *e, int mx, int my) {
	static const AIState state[5] = {STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT};
	static const int xvAhead[5] = {9, 0, 0, -1, 1};
	static const int yvAhead[5] = {9, -1, 1, 0, 0};

	if (e->sequence) {
		e->sequence--;

		if (e->blinkFrames)	// Between attacks timer
			e->blinkFrames--;

		// Is player visible to us?
		AIEntity *p = g_hdb->_ai->getPlayer();
		if (e->onScreen && p->level == e->level && !e->blinkFrames) {
			bool nuts = false;
			switch (e->dir) {
			case DIR_UP:
				if (p->tileX == e->tileX && p->tileY < e->tileY)
					nuts = true;
				break;
			case DIR_DOWN:
				if (p->tileX == e->tileX && p->tileY > e->tileY)
					nuts = true;
				break;
			case DIR_LEFT:
				if (p->tileY == e->tileY && p->tileX < e->tileX)
					nuts = true;
				break;
			case DIR_RIGHT:
				if (p->tileY == e->tileY && p->tileX > e->tileX)
					nuts = true;
				break;
			case DIR_NONE:
			default:
				break;
			}

			// Did we see the player (and we're done moving)?
			if (nuts && e->aiAction != aiDeadEyeWalkInPlace) {
				e->sequence = 0;
				e->blinkFrames = 20;

				int xv = xvAhead[e->dir];
				int yv = yvAhead[e->dir];
				int newX = e->tileX + xv;
				int newY = e->tileY + yv;

				bool okToMove = false;
				bool done = false;
				do {
					int result;
					AIEntity *hit = g_hdb->_ai->legalMove(newX, newY, e->level, &result);
					if (hit && hit->type == AI_GUY)
						hit = nullptr;
					if (result && !hit) {
						okToMove = true;
						newX += xv;
						newY += yv;
						if (newX == p->tileX && newY == p->tileY)
							done = true;
					} else {
						newX -= xv;
						newY -= yv;
						done = true;
					}
				} while (!done);

				// If we can move in the direction of the player, set our goal at him
				if (okToMove) {
					e->moveSpeed = kPlayerMoveSpeed << 1;
					g_hdb->_ai->setEntityGoal(e, newX, newY);
					(p->tileX & 1) ? g_hdb->_sound->playSound(SND_DEADEYE_ATTACK01) : g_hdb->_sound->playSound(SND_DEADEYE_ATTACK02);
				}
				g_hdb->_ai->animateEntity(e);
				return;
			}
		}

		switch (e->sequence) {
		// Look around
		case 50:
		case 40:
		case 30:
		case 20:
		case 10:
			{
				int dir = g_hdb->_rnd->getRandomNumber(3) + 1;
				e->dir = (AIDir)dir;
				e->state = state[dir];

				if (e->onScreen) {
					if (e->sequence == 50)
						g_hdb->_sound->playSound(SND_DEADEYE_AMB01);
					else if (e->sequence == 10)
						g_hdb->_sound->playSound(SND_DEADEYE_AMB02);
				}
			}
			break;
		case 0:
			{
				// Pick a random direction and random number of tiles in that direction
				int dir = g_hdb->_rnd->getRandomNumber(3) + 1;
				int walk = g_hdb->_rnd->getRandomNumber(4) + 1;

				e->dir = (AIDir)dir;
				e->state = state[dir];

				int xv = xvAhead[dir] * walk;
				if (e->tileX + xv < 1)
					xv = 1 - e->tileX;
				if (e->tileX + xv > g_hdb->_map->_width)
					xv = g_hdb->_map->_width - e->tileX - 1;

				int yv = yvAhead[dir] * walk;
				if (e->tileY + yv < 1)
					yv = 1 - e->tileY;
				if (e->tileY + yv > g_hdb->_map->_height)
					yv = g_hdb->_map->_height - e->tileY - 1;

				e->value1 = xvAhead[dir];
				e->value2 = yvAhead[dir];
				e->moveSpeed = kPlayerMoveSpeed;
				int result;
				AIEntity *hit = g_hdb->_ai->legalMove(e->tileX + xvAhead[e->dir], e->tileY + yvAhead[e->dir], e->level, &result);
				if (hit && hit->type == AI_GUY)
					hit = nullptr;

				if (!hit && result)
					g_hdb->_ai->setEntityGoal(e, e->tileX + xv, e->tileY + yv);
			}
			break;
		default:
			break;
		}
		g_hdb->_ai->animEntFrames(e);
		return;
	}

	// In the process of moving around
	if (e->goalX) {
		// Hit the player
		if (hitPlayer(e->x, e->y)) {
			g_hdb->_ai->killPlayer(DEATH_GRABBED);
			return;
		}
		// Did we run into a wall, entity, water, slime etc?
		// If so, Pick new direction
		if (onEvenTile(e->x, e->y)) {
			int result;
			AIEntity *hit = g_hdb->_ai->legalMove(e->tileX + e->value1, e->tileY + e->value2, e->level, &result);
			if (hit && hit->type == AI_GUY)
				hit = nullptr;
			if (!result || hit) {
				g_hdb->_ai->stopEntity(e);
				e->state = STATE_MOVEDOWN;
				e->sequence = 64;
				return;
			}
		}
		g_hdb->_ai->animateEntity(e);
	} else
		// If not, start looking around
		e->sequence = 64;
}

//-------------------------------------------------------------------
//
//	LASER
//
//-------------------------------------------------------------------

void aiLaserInit(AIEntity *e, int mx, int my) {
	e->aiDraw = aiLaserDraw;
	// start & end of laser beam
	e->value1 = e->value2 = 0;
}

void aiLaserInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
	if (!g_hdb->_ai->_gfxLaserbeamUD[0]) {
		char name[64];
		for (int i = 0; i < 4; i++) {
			Common::sprintf_s(name, FORCEFIELD_UD"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamUD[i] = g_hdb->_gfx->loadTile(name);
			Common::sprintf_s(name, FORCESPLASH_TOP"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamUDTop[i] = g_hdb->_gfx->loadTile(name);
			Common::sprintf_s(name, FORCESPLASH_BTM"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamUDBottom[i] = g_hdb->_gfx->loadTile(name);
			Common::sprintf_s(name, FORCEFIELD_LR"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamLR[i] = g_hdb->_gfx->loadTile(name);
			Common::sprintf_s(name, FORCESPLASH_LEFT"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamLRLeft[i] = g_hdb->_gfx->loadTile(name);
			Common::sprintf_s(name, FORCESPLASH_RIGHT"0%d", i + 1);
			g_hdb->_ai->_gfxLaserbeamLRRight[i] = g_hdb->_gfx->loadTile(name);
		}
	}
}

void aiLaserAction(AIEntity *e, int mx, int my) {
	static const int xva[] = {9, 0, 0,-1, 1};
	static const int yva[] = {9,-1, 1, 0, 0};

	AIEntity *hit = e;
	int moveOK = 0;
	int moveCount = 0;

	do {
		int nx = hit->tileX;
		int ny = hit->tileY;

		if (hit->type != AI_DIVERTER) {
			hit->int1 = xva[hit->dir];
			hit->int2 = yva[hit->dir];

			if (hit->dir == DIR_UP || hit->dir == DIR_DOWN)
				hit->value1 = ny;
			else
				hit->value1 = nx;
		} else {
			// diverter is on y-plane?
			if (hit->tileX == e->tileX) {
				hit->value1 = nx;
				hit->int2 = 0;
				switch (hit->dir2) {
				case DIR_UP:
					hit->int1 = 1;
					break;
				case DIR_DOWN:
					hit->int1 = -1;
					break;
				case DIR_LEFT:
					hit->int1 = -1;
					break;
				case DIR_RIGHT:
					hit->int1 = 1;
					break;
				case DIR_NONE:
				default:
					break;
				}
			} else {
				// diverter is on x-plane
				hit->value1 = ny;
				hit->int1 = 0;
				switch (hit->dir2) {
				case DIR_UP:
					hit->int2 = 1;
					break;
				case DIR_DOWN:
					hit->int2 = 1;
					break;
				case DIR_LEFT:
					hit->int2 = -1;
					break;
				case DIR_RIGHT:
					hit->int2 = -1;
					break;
				case DIR_NONE:
				default:
					break;
				}
			}
		}
		e = hit;

		//
		// scan for all legal moves
		//
		do {
			nx += e->int1;
			ny += e->int2;
			hit = g_hdb->_ai->legalMoveOverWater(nx, ny, e->level, &moveOK);
			g_hdb->_map->setLaserBeam(nx, ny, 1);
			if (hit && hit->type != AI_LASERBEAM) {
				//
				// hit player = death
				//
				if (hit == g_hdb->_ai->getPlayer() && onEvenTile(hit->x, hit->y) && !g_hdb->_ai->playerDead())
					g_hdb->_ai->killPlayer(DEATH_FRIED);
				else if (hit->type == AI_BOOMBARREL && hit->state != STATE_EXPLODING && onEvenTile(hit->x, hit->y)) {
					// hit BOOM BARREL = explodes
					aiBarrelExplode(hit, 0, 0);
					aiBarrelBlowup(hit, nx, ny);
				} else if (hit->type == AI_LIGHTBARREL || hit->type == AI_HEAVYBARREL || hit->type == AI_CRATE) {
					// hit LIGHT/HEAVY BARREL = blocking
					moveOK = 0;
				} else if (hit->type == AI_DIVERTER) {
					// hit a diverter?
					moveOK = 0;
				} else if (onEvenTile(hit->x, hit->y) && hit != g_hdb->_ai->getPlayer()) {
					switch (hit->type) {
					// cannot kill Vortexians!
					case AI_VORTEXIAN:
						continue;

					case AI_BOOMBARREL:
						if (hit->state == STATE_EXPLODING)
							continue;
						break;
					case AI_LASER:
						g_hdb->_ai->_laserRescan = true;
						break;
					case ITEM_KEYCARD_WHITE:
					case ITEM_KEYCARD_BLUE:
					case ITEM_KEYCARD_RED:
					case ITEM_KEYCARD_GREEN:
					case ITEM_KEYCARD_PURPLE:
					case ITEM_KEYCARD_BLACK:
					case ITEM_CABKEY:
						g_hdb->_window->centerTextOut("CARD DESTROYED!", 306, 5 * 60);
						g_hdb->_sound->playSound(SND_QUEST_FAILED);
						break;
					default:
						break;
					}
					g_hdb->_ai->removeEntity(hit);
					g_hdb->_ai->addAnimateTarget(nx * kTileWidth,
						ny * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
					g_hdb->_sound->playSound(SND_BARREL_EXPLODE);
				}
			}
		} while (moveOK);

		if (e->int2) {
			e->value2 = ny;
			// check for hitting the BACK of a Diverter.  It stops the laser.
			if (hit && hit->type == AI_DIVERTER) {
				if (e->int2 < 0 && hit->state != STATE_DIVERTER_BL && hit->state != STATE_DIVERTER_BR)
					hit = nullptr;
				else if (e->int2 > 0 && hit->state != STATE_DIVERTER_TL && hit->state != STATE_DIVERTER_TR)
					hit = nullptr;
			}
		} else {
			e->value2 = nx;
			// check for hitting the BACK of a Diverter.  It stops the laser.
			if (hit && hit->type == AI_DIVERTER) {
				if (e->int1 < 0 && hit->state != STATE_DIVERTER_BR && hit->state != STATE_DIVERTER_TR)
					hit = nullptr;
				else if (e->int1 > 0 && hit->state != STATE_DIVERTER_TL && hit->state != STATE_DIVERTER_BL)
					hit = nullptr;
			}
		}

		moveCount++;

		// It is possible to set a configuration which leads to a closed loop.
		// Thus, we're breaking it here
		if (moveCount > 1000)
			hit = nullptr;
	} while (hit && hit->type == AI_DIVERTER);
}

void aiLaserDraw(AIEntity *e, int mx, int my) {
	int	i;
	int	frame = e->movedownFrames & 3;
	int onScreen = 0;

	switch (e->dir) {
	case DIR_UP:
	{
		for (i = e->value1 - 1; i > e->value2; i--)
			onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileWidth - my);
		onScreen += g_hdb->_ai->_gfxLaserbeamUDBottom[frame & 3]->drawMasked(e->x - mx, i * kTileWidth - my);
		if (onScreen) {
			g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
			g_hdb->_ai->_laserOnScreen = true;
		}
	}
		break;
	case DIR_DOWN:
	{
		for (i = e->value1 + 1; i < e->value2; i++)
			onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileWidth - my);
		onScreen += g_hdb->_ai->_gfxLaserbeamUDBottom[frame]->drawMasked(e->x - mx, i * kTileWidth - my);
		if (onScreen) {
			g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
			g_hdb->_ai->_laserOnScreen = true;
		}
	}
		break;
	case DIR_LEFT:
	{
		for (i = e->value1 - 1; i > e->value2; i--)
			onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
		onScreen += g_hdb->_ai->_gfxLaserbeamLRRight[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
		if (onScreen) {
			g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
			g_hdb->_ai->_laserOnScreen = true;
		}
	}
		break;
	case DIR_RIGHT:
	{
		for (i = e->value1 + 1; i < e->value2; i++)
			onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
		onScreen += g_hdb->_ai->_gfxLaserbeamLRLeft[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
		if (onScreen) {
			g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
			g_hdb->_ai->_laserOnScreen = true;
		}
	}
		break;
	case DIR_NONE:
	default:
		break;
	}
	e->movedownFrames++;
}

//-------------------------------------------------------------------
//
//	DIVERTER
//
//-------------------------------------------------------------------

void aiDiverterInit(AIEntity *e, int mx, int my) {
	e->aiDraw = aiDiverterDraw;
	e->aiAction = aiDiverterAction;
	e->moveSpeed = kPlayerMoveSpeed << 1;
	e->dir2 = e->dir;
}

void aiDiverterInit2(AIEntity *e, int mx, int my) {
	e->movedownGfx[0] = e->standdownGfx[0];
	e->moveupGfx[0] = e->standupGfx[0];
	e->moveleftGfx[0] = e->standleftGfx[0];
	e->moverightGfx[0] = e->standrightGfx[0];
	e->movedownFrames =
		e->moveupFrames =
		e->moveleftFrames =
		e->moverightFrames = 1;

	// this is to handle loadgames...
	AIDir d = e->dir2;
	if (e->dir2 == DIR_NONE)
		d = e->dir;
	switch (d) {
	case DIR_DOWN:
		e->state = STATE_DIVERTER_BL;
		e->draw = e->standdownGfx[0];
		break;
	case DIR_UP:
		e->state = STATE_DIVERTER_BR;
		e->draw = e->standupGfx[0];
		break;
	case DIR_LEFT:
		e->state = STATE_DIVERTER_TL;
		e->draw = e->standleftGfx[0];
		break;
	case DIR_RIGHT:
		e->state = STATE_DIVERTER_TR;
		e->draw = e->standrightGfx[0];
		break;
	case DIR_NONE:
	default:
		break;
	}

	g_hdb->_ai->_laserRescan = true;
}

void aiDiverterAction(AIEntity *e, int mx, int my) {
	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		g_hdb->_ai->_laserRescan = true;

		// have to reset the state because we might have been moved...
		switch (e->dir2) {
		case DIR_DOWN:
			e->state = STATE_DIVERTER_BL;
			e->draw = e->standdownGfx[0];
			break;
		case DIR_UP:
			e->state = STATE_DIVERTER_BR;
			e->draw = e->standupGfx[0];
			break;
		case DIR_LEFT:
			e->state = STATE_DIVERTER_TL;
			e->draw = e->standleftGfx[0];
			break;
		case DIR_RIGHT:
			e->state = STATE_DIVERTER_TR;
			e->draw = e->standrightGfx[0];
			break;
		case DIR_NONE:
		default:
			break;
		}
	}
}

void aiDiverterDraw(AIEntity *e, int mx, int my) {
	if (!e->value1 && !e->value2)
		return;

	int	frame = e->movedownFrames & 3;
	int onScreen = 0;
	int i;
	switch (e->dir2) {
	case DIR_UP:
		if (e->tileY == e->value1 && e->int2) {	// going down or right?
			for (i = e->value1 + 1; i < e->value2; i++)
				onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamUDTop[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		} else {
			for (i = e->value1 + 1; i < e->value2; i++)
				onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamLRLeft[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		}
		break;
	case DIR_DOWN:
		if (e->tileY == e->value1 && e->int2) {	// going down or left?
			for (i = e->value1 + 1; i < e->value2; i++)
				onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamUDTop[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		} else {
			for (i = e->value1 - 1; i > e->value2; i--)
				onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamLRRight[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		}
		break;
	case DIR_LEFT:
		if (e->tileY == e->value1 && e->int2) {	// going up or left?
			for (i = e->value1 - 1; i > e->value2; i--)
				onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamUDBottom[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		} else {
			for (i = e->value1 - 1; i > e->value2; i--)
				onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamLRRight[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		}
		break;
	case DIR_RIGHT:
		if (e->tileY == e->value1 && e->int2) {	// going up or right?
			for (i = e->value1 - 1; i > e->value2; i--)
				onScreen += g_hdb->_ai->_gfxLaserbeamUD[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamUDBottom[frame]->drawMasked(e->x - mx, i * kTileHeight - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		} else {
			for (i = e->value1 + 1; i < e->value2; i++)
				onScreen += g_hdb->_ai->_gfxLaserbeamLR[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			onScreen += g_hdb->_ai->_gfxLaserbeamLRLeft[frame]->drawMasked(i * kTileWidth - mx, e->y - my);
			if (onScreen) {
				g_hdb->_sound->playSoundEx(SND_LASER_LOOP, kLaserChannel, true);
				g_hdb->_ai->_laserOnScreen = true;
			}
		}
		break;
	case DIR_NONE:
	default:
		break;
	}
	e->movedownFrames++;
}

//-------------------------------------------------------------------
//
//	MEERKAT : nutty little groundhog dude that will bite Guy if he's on
//		his mound.  That blows 1-5 gems outta Guy!
//
//-------------------------------------------------------------------

void aiMeerkatInit(AIEntity *e, int mx, int my) {
	e->state = STATE_NONE;
	e->sequence = 0;
	if (e->value1 == 1) {
		e->aiAction = aiMeerkatLookAround;
		e->state = STATE_MEER_LOOK;
	} else
		e->aiAction = aiMeerkatAction;
}

void aiMeerkatInit2(AIEntity *e, int mx, int my) {
	//  hidden at the start!
	e->draw = nullptr;

	// make the looking around cycle better...
	e->movedownGfx[3] = e->movedownGfx[1];
	e->movedownFrames++;
}

void aiMeerkatDraw(AIEntity *e, int mx, int my) {
	char word[3];
	g_hdb->_window->getGemGfx()->drawMasked(e->value1 - mx, e->value2 - my, 255 - e->blinkFrames * 16);
	g_hdb->_gfx->setCursor(e->value1 + 12 - mx, e->value2 - 8 - my);
	word[2] = 0;
	if (!e->special1Frames) {
		word[0] = '0';
		word[1] = 0;
	} else {
		word[0] = '-';
		word[1] = '0' + e->special1Frames;
	}
	g_hdb->_gfx->drawText(word);
}

void aiMeerkatAction(AIEntity *e, int mx, int my) {
	static const int gem_xv[] = { 0, 0,-2,-3,-4,-4,-3,-2,-2,-2,-2,-1,-1, 100};
	static const int gem_yv[] = {-6,-5,-4,-3,-2,-1, 0, 0, 1, 2, 3, 4, 5, 100};

	AIEntity *p = g_hdb->_ai->getPlayer();

	switch (e->sequence) {
		// waiting to see the player
	case 0:
		if ((abs(p->tileX - e->tileX) <= 1 && p->tileY == e->tileY) ||
			(abs(p->tileY - e->tileY) <= 1 && p->tileX == e->tileX)) {
			e->sequence = 1;
			e->state = STATE_MEER_MOVE;
			e->animFrame = 0;
			e->animCycle = 1;
			e->animDelay = e->animCycle;
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_MEERKAT_WARNING);
		}
		break;
		// time to show the mound for a sec...
	case 1:
		g_hdb->_ai->animateEntity(e);
		if (!e->animFrame && e->animDelay == e->animCycle)
			e->sequence++;
		if (e->sequence == 2) {
			e->state = STATE_MEER_APPEAR;
			e->animFrame = 0;
			e->animDelay = e->animCycle;
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_MEERKAT_APPEAR);
		}
		break;

		// pop outta the dirt!
	case 2:
		g_hdb->_ai->animateEntity(e);
		// done w/sequence?
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->sequence++;
			e->state = STATE_MEER_LOOK;
			e->animFrame = 0;
			e->animCycle = 2;
			e->animDelay = e->animCycle;
		}
		break;

		// looking around...... time to bite the player!?
	case 3:
	case 4:
		g_hdb->_ai->animateEntity(e);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->sequence++;
			if (e->sequence == 5)
				e->state = STATE_MEER_DISAPPEAR;
		}
		if (g_hdb->_ai->checkPlayerTileCollision(e->tileX, e->tileY)) {
			e->state = STATE_MEER_BITE;
			e->sequence = 6;
			e->animFrame = 0;
			e->animDelay = e->animCycle;
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_MEERKAT_BITE);
		}
		break;

		// going back underground!
	case 5:
		g_hdb->_ai->animateEntity(e);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->sequence = 0;
			e->state = STATE_NONE;
			e->draw = nullptr;
		}
		break;

		// biting the player right now!
	case 6:
		g_hdb->_ai->animateEntity(e);
		// hit the player?
		if (g_hdb->_ai->checkPlayerTileCollision(e->tileX, e->tileY)) {
			g_hdb->_ai->stopEntity(p);
			g_hdb->_ai->setPlayerLock(true);
			e->sequence = 7;
			p->moveSpeed <<= 1;
			if (g_hdb->_ai->findEntity(p->tileX, p->tileY + 1))
				g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY - 1);
			else
				g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY + 1);
			e->aiDraw = aiMeerkatDraw;
			e->value1 = e->x;
			e->value2 = e->y;
			e->blinkFrames = 0;		// index into movement table...

			// figure # of gems to take
			e->special1Frames = g_hdb->_rnd->getRandomNumber(4) + 1;
			int	amt = g_hdb->_ai->getGemAmount();
			if (amt - e->special1Frames < 0)
				e->special1Frames = amt;

			// if we're in Puzzle Mode and there's no gems left, give one back
			if (!g_hdb->getActionMode() && !(e->special1Frames - amt) && e->special1Frames)
				e->special1Frames--;

			amt -= e->special1Frames;
			g_hdb->_ai->setGemAmount(amt);
		}
		// go back to looking?
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->sequence = 3;
			e->state = STATE_MEER_LOOK;
			e->animFrame = 0;
			e->animDelay = e->animCycle;
		}
		break;

		// waiting for player to blast backward
	case 7:
		g_hdb->_ai->animateEntity(e);
		if (!p->goalX) {
			p->moveSpeed = kPlayerMoveSpeed;
			g_hdb->_ai->setPlayerLock(false);
			e->sequence = 5;
			e->state = STATE_MEER_DISAPPEAR;
			e->animFrame = 0;
			e->animDelay = e->animCycle;
		}
		break;

	default:
		break;
	}

	// blasting a gem outta Guy?
	if (e->value1) {
		if (gem_xv[e->blinkFrames] == 100) {
			e->value1 = 0;
			e->aiDraw = nullptr;
			return;
		}
		e->value1 += gem_xv[e->blinkFrames];
		e->value2 += gem_yv[e->blinkFrames];
		e->blinkFrames++;
	}
}

void aiMeerkatLookAround(AIEntity *e, int mx, int my) {
	g_hdb->_ai->animEntFrames(e);
}

//-------------------------------------------------------------------
//
//	FATFROG : Just sits in place and blasts out his tongue if you're
//		within range.
//
//-------------------------------------------------------------------

void aiFatFrogInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiFatFrogAction;
}

void aiFatFrogInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
	// load tongue tiles
	switch (e->dir) {
	case DIR_DOWN:
		if (!g_hdb->_ai->_tileFroglickMiddleUD) {
			g_hdb->_ai->_tileFroglickMiddleUD = g_hdb->_gfx->loadTile(TILE_FFTONGUE_UD_MIDDLE);
			g_hdb->_ai->_tileFroglickWiggleUD[0] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_UD_WIGGLE_L);
			g_hdb->_ai->_tileFroglickWiggleUD[1] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_UD_WIGGLE_M);
			g_hdb->_ai->_tileFroglickWiggleUD[2] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_UD_WIGGLE_R);
		}
		e->state = STATE_STANDDOWN;
		break;
	case DIR_LEFT:
		if (!g_hdb->_ai->_tileFroglickMiddleLR)
			g_hdb->_ai->_tileFroglickMiddleLR = g_hdb->_gfx->loadTile(TILE_FFTONGUE_LR_MIDDLE);

		if (!g_hdb->_ai->_tileFroglickWiggleLeft[0]) {
			g_hdb->_ai->_tileFroglickWiggleLeft[0] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_L_WIGGLE_U);
			g_hdb->_ai->_tileFroglickWiggleLeft[1] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_L_WIGGLE_M);
			g_hdb->_ai->_tileFroglickWiggleLeft[2] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_L_WIGGLE_D);
		}
		e->state = STATE_STANDLEFT;
		break;
	case DIR_RIGHT:
		if (!g_hdb->_ai->_tileFroglickMiddleLR)
			g_hdb->_ai->_tileFroglickMiddleLR = g_hdb->_gfx->loadTile(TILE_FFTONGUE_LR_MIDDLE);

		if (!g_hdb->_ai->_tileFroglickWiggleRight[0]) {
			g_hdb->_ai->_tileFroglickWiggleRight[0] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_R_WIGGLE_U);
			g_hdb->_ai->_tileFroglickWiggleRight[1] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_R_WIGGLE_M);
			g_hdb->_ai->_tileFroglickWiggleRight[2] = g_hdb->_gfx->loadTile(TILE_FFTONGUE_R_WIGGLE_D);
		}
		e->state = STATE_STANDRIGHT;
		break;
	default:
		break;
	}
}

void aiFatFrogAction(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	switch (e->state) {
		//-------------------------------------------------------------------
		// WAITING TO ATTACK
		//-------------------------------------------------------------------
	case STATE_STANDDOWN:
		e->draw = e->standdownGfx[e->animFrame];
		// is player within 2 tiles below fatfrog?
		if (p->tileX == e->tileX && p->tileY - e->tileY < 3 && p->tileY > e->tileY) {
			e->state = STATE_LICKDOWN;
			e->animDelay = e->animCycle << 2;
			e->animFrame = 0;
		}
		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle << 2;
		e->animFrame++;
		if (e->animFrame == e->standdownFrames)
			e->animFrame = 0;
		if (!g_hdb->_rnd->getRandomNumber(29) && e->onScreen)
			g_hdb->_sound->playSound(SND_FROG_RIBBIT1);
		break;

	case STATE_STANDLEFT:
		e->draw = e->standleftGfx[e->animFrame];
		// is player within 2 tiles below fatfrog?
		if (p->tileY == e->tileY && e->tileX - p->tileX < 3 && p->tileX < e->tileX) {
			e->state = STATE_LICKLEFT;
			e->animDelay = e->animCycle << 2;
			e->animFrame = 0;
		}
		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle << 2;
		e->animFrame++;
		if (e->animFrame == e->standleftFrames)
			e->animFrame = 0;
		if (!g_hdb->_rnd->getRandomNumber(29) && e->onScreen)
			g_hdb->_sound->playSound(SND_FROG_RIBBIT2);
		break;

	case STATE_STANDRIGHT:
		e->draw = e->standrightGfx[e->animFrame];
		// is player within 2 tiles below fatfrog?
		if (p->tileY == e->tileY && p->tileX - e->tileX < 3 && p->tileX > e->tileX) {
			e->state = STATE_LICKRIGHT;
			e->animDelay = e->animCycle << 2;
			e->animFrame = 0;
		}
		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle << 2;
		e->animFrame++;
		if (e->animFrame == e->standrightFrames)
			e->animFrame = 0;
		if (!g_hdb->_rnd->getRandomNumber(29) && e->onScreen)
			g_hdb->_sound->playSound(SND_FROG_RIBBIT2);
		break;

		//-------------------------------------------------------------------
		// LICK ATTACK
		//-------------------------------------------------------------------
	case STATE_LICKDOWN:
		e->draw = e->movedownGfx[e->animFrame];
		// ready to start licking?
		if (e->animFrame == e->movedownFrames - 1 && !e->value1) {
			e->value1 = 1;
			e->aiDraw = aiFatFrogTongueDraw;
			g_hdb->_sound->playSound(SND_FROG_LICK);
		} else if (e->animFrame == e->movedownFrames - 1 && e->value1) {
			// animate licking

			// check player death
			if (((p->tileX == e->tileX && p->tileY == e->tileY + 1) ||					// in front of frog + 1 tile!?
				(e->value1 > 3 && p->tileX == e->tileX && p->tileY < e->tileY + 3)) &&	// in front of frog + 2 tiles!?
				g_hdb->_ai->playerDead() == false)
				g_hdb->_ai->killPlayer(DEATH_NORMAL);

			e->value1++;
			if (e->value1 == 14) {
				e->animFrame = e->value1 = 0;
				e->aiDraw = nullptr;
				e->state = STATE_STANDDOWN;
			}
		} else {
			// animate pre-licking
			// cycle animation frames
			if (e->animDelay-- > 0)
				return;
			e->animDelay = e->animCycle;
			e->animFrame++;
		}
		break;

	case STATE_LICKLEFT:
		e->draw = e->moveleftGfx[e->animFrame];
		// ready to start licking?
		if (e->animFrame == e->moveleftFrames - 1 && !e->value1) {
			e->value1 = 1;
			e->aiDraw = aiFatFrogTongueDraw;
			g_hdb->_sound->playSound(SND_FROG_LICK);
		} else if (e->animFrame == e->moveleftFrames - 1 && e->value1) {
			// animate licking

			// check player death
			if (((p->tileY == e->tileY && p->tileX == e->tileX - 1) ||					// in front of frog + 1 tile!?
				(e->value1 > 3 && p->tileY == e->tileY && p->tileX > e->tileX - 3)) &&	// in front of frog + 2 tiles!?
				g_hdb->_ai->playerDead() == false)
				g_hdb->_ai->killPlayer(DEATH_NORMAL);

			e->value1++;
			if (e->value1 == 14) {
				e->animFrame = e->value1 = 0;
				e->aiDraw = nullptr;
				e->state = STATE_STANDLEFT;
			}
		} else {
			// animate pre-licking
			// cycle animation frames
			if (e->animDelay-- > 0)
				return;
			e->animDelay = e->animCycle;
			e->animFrame++;
		}
		break;

	case STATE_LICKRIGHT:
		e->draw = e->moverightGfx[e->animFrame];
		// ready to start licking?
		if (e->animFrame == e->moverightFrames - 1 && !e->value1) {
			e->value1 = 1;
			e->aiDraw = aiFatFrogTongueDraw;
			g_hdb->_sound->playSound(SND_FROG_LICK);
		} else if (e->animFrame == e->moverightFrames - 1 && e->value1) {
			// animate licking
			// check player death
			//
			if (((p->tileY == e->tileY && p->tileX == e->tileX + 1) ||					// in front of frog + 1 tile!?
				(e->value1 > 3 && p->tileY == e->tileY && p->tileX < e->tileX + 3)) &&	// in front of frog + 2 tiles!?
				g_hdb->_ai->playerDead() == false)
				g_hdb->_ai->killPlayer(DEATH_NORMAL);

			e->value1++;
			if (e->value1 == 14) {
				e->animFrame = e->value1 = 0;
				e->aiDraw = nullptr;
				e->state = STATE_STANDRIGHT;
			}
		} else {
			// animate pre-licking
			// cycle animation frames
			if (e->animDelay-- > 0)
				return;
			e->animDelay = e->animCycle;
			e->animFrame++;
		}
		break;
	default:
		// no op
		break;
	}
}

void aiFatFrogTongueDraw(AIEntity *e, int mx, int my) {
	int	nx, ny;

	switch (e->state) {
	case STATE_LICKDOWN:
		switch (e->value1) {
		case 1:
		case 2:
		case 3:
		case 13:
		case 14:
			nx = e->x;
			ny = e->y + 32;
			g_hdb->_ai->_tileFroglickWiggleUD[1]->drawMasked(nx - mx, ny - my);
			break;
		case 4:
		case 7:
		case 10:
			nx = e->x;
			ny = e->y + 32;
			g_hdb->_ai->_tileFroglickMiddleUD->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleUD[0]->drawMasked(nx - mx, ny + 32 - my);
			break;
		case 5:
		case 8:
		case 11:
			nx = e->x;
			ny = e->y + 32;
			g_hdb->_ai->_tileFroglickMiddleUD->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleUD[1]->drawMasked(nx - mx, ny + 32 - my);
			break;
		case 6:
		case 9:
		case 12:
			nx = e->x;
			ny = e->y + 32;
			g_hdb->_ai->_tileFroglickMiddleUD->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleUD[2]->drawMasked(nx - mx, ny + 32 - my);
			break;
		default:
			break;
		}
		break;

	case STATE_LICKLEFT:
		switch (e->value1) {
		case 1:
		case 2:
		case 3:
		case 13:
		case 14:
			nx = e->x - 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickWiggleLeft[1]->drawMasked(nx - mx, ny - my);
			break;
		case 4:
		case 7:
		case 10:
			nx = e->x - 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleLeft[0]->drawMasked(nx - 32 - mx, ny - my);
			break;
		case 5:
		case 8:
		case 11:
			nx = e->x - 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleLeft[1]->drawMasked(nx - 32 - mx, ny - my);
			break;
		case 6:
		case 9:
		case 12:
			nx = e->x - 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleLeft[2]->drawMasked(nx - 32 - mx, ny - my);
			break;
		default:
			break;
		}
		break;

	case STATE_LICKRIGHT:
		switch (e->value1) {
		case 1:
		case 2:
		case 3:
		case 13:
		case 14:
			nx = e->x + 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickWiggleRight[1]->drawMasked(nx - 32 - mx, ny - my);
			break;
		case 4:
		case 7:
		case 10:
			nx = e->x + 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleRight[0]->drawMasked(nx + 32 - mx, ny - my);
			break;
		case 5:
		case 8:
		case 11:
			nx = e->x + 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleRight[1]->drawMasked(nx + 32 - mx, ny - my);
			break;
		case 6:
		case 9:
		case 12:
			nx = e->x + 32;
			ny = e->y;
			g_hdb->_ai->_tileFroglickMiddleLR->drawMasked(nx - mx, ny - my);
			g_hdb->_ai->_tileFroglickWiggleRight[2]->drawMasked(nx + 32 - mx, ny - my);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

//-------------------------------------------------------------------
//
//	GOODFAIRY
//
//-------------------------------------------------------------------

void aiGoodFairyInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiGoodFairyAction;
	e->sequence = 20;
	e->blinkFrames = e->goalX = 0;
}

void aiGoodFairyInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiGoodFairyAction(AIEntity *e, int mx, int my) {
	static const AIState state[5] = {STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT};
	static const int xvAhead[5] = {9, 0, 0,-1, 1};
	static const int yvAhead[5] = {9,-1, 1, 0, 0};

	int	result;

	if (e->sequence) {
		e->sequence--;

		// look around...
		switch (e->sequence) {
		case 19:
			e->state = STATE_MOVEDOWN;
			break;
		case 0:
			{
				// Create a GEM?
				if (g_hdb->_rnd->getRandomNumber(99) > 98) {
					// spawn a gem in a random direction
					int	d = g_hdb->_rnd->getRandomNumber(3) + 1;
					int xv = xvAhead[d];
					int yv = yvAhead[d];

					e->sequence = 30;
					e->state = STATE_MOVEDOWN;
					// is something there already?
					if ((g_hdb->_ai->findEntityType(AI_CRATE, e->tileX + xv, e->tileY + yv) != nullptr) ||
						(g_hdb->_ai->findEntityType(AI_LIGHTBARREL, e->tileX + xv, e->tileY + yv) != nullptr))
						return;
					int spawnOK;
					AIEntity *hit = g_hdb->_ai->legalMove(e->tileX + xv, e->tileY + yv, e->level, &spawnOK);
					uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX + xv, e->tileY + yv);
					if (hit || !spawnOK || (bg_flags & kFlagSpecial))
						return;

					g_hdb->_ai->spawn(ITEM_GEM_WHITE, e->dir, e->tileX + xv, e->tileY + yv, nullptr, nullptr, nullptr, DIR_NONE, e->level, 0, 0, 1);
					g_hdb->_ai->addAnimateTarget(e->x + xv * kTileWidth, e->y + yv * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					if (e->onScreen) {
						g_hdb->_sound->playSound(SND_GET_GEM);
						g_hdb->_sound->playSound(SND_GOOD_FAERIE_SPELL);
					}
					return;
				}

				int	tries = 4;
				do {
					// pick a random direction, then a random # of tiles in that direction
					AIDir d = (AIDir)(g_hdb->_rnd->getRandomNumber(3) + 1);
					int walk = g_hdb->_rnd->getRandomNumber(4) + 1;
					AIEntity *p = g_hdb->_ai->getPlayer();

					// if player is within 3 tiles, move closer
					if (abs(p->tileX - e->tileX) < 3 && abs(p->tileY - e->tileY) < 3) {
						if (abs(p->tileX - e->tileX) > abs(p->tileY - e->tileY)) {
						testx:
							if (p->tileX != e->tileX) {
								if (p->tileX < e->tileX)
									d = DIR_LEFT;
								else
									d = DIR_RIGHT;
							} else if (p->tileY != e->tileY)
								goto testy;
						} else {
						testy:
							if (p->tileY != e->tileY) {
								if (p->tileY <= e->tileY)
									d = DIR_UP;
								else
									d = DIR_DOWN;
							} else if (p->tileX != e->tileX)
								goto testx;
						}
					}

					// special case: if player is exactly 2 tiles away, move out of the way
					if (abs(p->tileX - e->tileX) == 2 && p->tileY == e->tileY) {
						int	move_ok;
						d = DIR_UP;
						AIEntity *h = g_hdb->_ai->legalMoveOverWater(e->tileX, e->tileY - 1, e->level, &move_ok);
						if (h || !move_ok)
							d = DIR_DOWN;
					} else if (abs(p->tileY - e->tileY) == 2 && p->tileX == e->tileX) {
						int	move_ok;
						d = DIR_LEFT;
						AIEntity *h = g_hdb->_ai->legalMoveOverWater(e->tileX - 1, e->tileY, e->level, &move_ok);
						if (h || !move_ok)
							d = DIR_RIGHT;
					}

					e->dir = d;
					int tmpDir = (int)d;
					e->state = state[tmpDir];
					int xv = xvAhead[tmpDir] * walk;
					if (e->tileX + xv < 1)
						xv = -e->tileX + 1;
					if (e->tileX + xv > g_hdb->_map->_width)
						xv = g_hdb->_map->_width - e->tileX - 1;

					int yv = yvAhead[d] * walk;
					if (e->tileY + yv < 1)
						yv = -e->tileY + 1;
					if (e->tileY + yv > g_hdb->_map->_height)
						yv = g_hdb->_map->_height - e->tileY - 1;

					e->value1 = xvAhead[d];
					e->value2 = yvAhead[d];
					e->moveSpeed = kPlayerMoveSpeed;

					// make sure we can move over water & white gems, but not fg_hdb->_ai->y blockers and solids
					AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + e->value1, e->tileY + e->value2, e->level, &result);
					if (hit && ((hit->type == ITEM_GEM_WHITE) || (hit->type == AI_GUY)))
						hit = nullptr;
					uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX + e->value1, e->tileY + e->value2);
					if (result && !hit && !(bg_flags & kFlagSpecial)) {
						g_hdb->_ai->setEntityGoal(e, e->tileX + xv, e->tileY + yv);
						if (e->onScreen && !g_hdb->_rnd->getRandomNumber(29))
							g_hdb->_sound->playSound(SND_GOOD_FAERIE_AMBIENT);
						g_hdb->_ai->animateEntity(e);
						return;
					}
					tries--;		// don't lock the system if the fg_hdb->_ai->y is cornered
				} while (!result && tries);

				// couldn't find a place to move so just sit here for a sec & try agg_hdb->_ai->
				e->dir = DIR_NONE;
				e->state = STATE_MOVEDOWN;
				e->sequence = 1;
				e->value1 = e->value2 = e->xVel = e->yVel = 0;
			}
			break;
		default:
			break;
		}
		g_hdb->_ai->animEntFrames(e);
		return;
	}

	// in the process of moving around...
	if (e->goalX) {
		// did we run into a wall, entity, water, slime etc?
		// if so, pick a new direction!
		if (onEvenTile(e->x, e->y)) {
			// did we hit a Fg_hdb->_ai->YSTONE??? if so - teleport the thing at the other end to here!
			int index = g_hdb->_ai->checkFairystones(e->tileX, e->tileY);
			if (index >= 0) {
				int	sx, sy;
				g_hdb->_ai->getFairystonesSrc(index, &sx, &sy);
				AIEntity *hit = g_hdb->_ai->findEntity(sx, sy);
				if (hit && (hit != g_hdb->_ai->getPlayer())) {
					hit->tileX = e->tileX;
					hit->tileY = e->tileY;
					hit->x = hit->tileX * kTileWidth;
					hit->y = hit->tileY * kTileHeight;
					hit->goalX = hit->goalY = 0;
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
					g_hdb->_ai->addAnimateTarget(sx * kTileWidth, sy * kTileHeight, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
					if (e->onScreen)
						g_hdb->_sound->playSound(SND_TELEPORT);
					if (hit->onScreen)
						g_hdb->_sound->playSound(SND_TELEPORT);
				}
			}

			// see if we're about to move to a bad spot, which means:
			// (1) we're gonna hit a solid wall; ok to move over water/slime
			// (2) ok to move thru white gems
			// (3) cannot move thru SPECIAL flagged tiles (fg_hdb->_ai->y blockers)
			AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + e->value1, e->tileY + e->value2, e->level, &result);
			uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX + e->value1, e->tileY + e->value2);
			if (!result || (hit && hit->type != ITEM_GEM_WHITE && hit->type != AI_GUY) || (bg_flags & kFlagSpecial)) {
				g_hdb->_ai->stopEntity(e);
				e->value1 = e->value2 = 0;
				e->state = STATE_MOVEDOWN;
				e->sequence = 20;
				return;
			}
		}
		g_hdb->_ai->animateEntity(e);
	} else {
		// if not, start looking around!
		e->sequence = 20;
	}
}

//-------------------------------------------------------------------
//
//	BADFAIRY
//
//-------------------------------------------------------------------

void aiBadFairyInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiBadFairyAction;
	e->sequence = 20;
	e->blinkFrames = e->goalX = 0;
}

void aiBadFairyInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiBadFairyAction(AIEntity *e, int mx, int my) {
	static const AIState state[5] = {STATE_NONE, STATE_MOVEUP, STATE_MOVEDOWN, STATE_MOVELEFT, STATE_MOVERIGHT};
	static const AIDir opposite[5] = {DIR_NONE, DIR_DOWN, DIR_UP, DIR_RIGHT, DIR_LEFT};
	static const int xvAhead[5] = {9, 0, 0,-1, 1};
	static const int yvAhead[5] = {9,-1, 1, 0, 0};

	if (e->sequence) {
		e->sequence--;

		// look around...
		switch (e->sequence) {
		case 19:
			e->state = STATE_MOVEDOWN;
			break;
		case 0:
			{
				// Create a GATE PUDDLE?
				if (e->onScreen && (g_hdb->_rnd->getRandomNumber(99) > 90) && g_hdb->getActionMode() && (g_hdb->_ai->getGatePuddles() < kMaxGatePuddles)) {
					if (e->onScreen)
						g_hdb->_sound->playSound(SND_BADFAIRY_SPELL);

					e->sequence = 30;
					e->state = STATE_MOVEUP;
					g_hdb->_ai->spawn(AI_GATEPUDDLE, opposite[e->dir], e->tileX, e->tileY, nullptr, nullptr, nullptr, DIR_NONE, e->level, 0, 0, 1);
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
					g_hdb->_ai->addGatePuddle(1);
					if (e->onScreen)
						g_hdb->_sound->playSound(SND_GATEPUDDLE_SPAWN);
					return;
				}

				int	tries = 4;
				int	result;
				do {
					// pick a random direction, then a random # of tiles in that direction
					int d = g_hdb->_rnd->getRandomNumber(3) + 1;
					int	walk = g_hdb->_rnd->getRandomNumber(4) + 1;
					AIEntity *p = g_hdb->_ai->getPlayer();

					e->dir = (AIDir)d;
					e->state = state[d];
					int xv = xvAhead[d] * walk;
					if (e->tileX + xv < 1)
						xv = -e->tileX + 1;
					if (e->tileX + xv > g_hdb->_map->_width)
						xv = g_hdb->_map->_width - e->tileX - 1;

					int yv = yvAhead[d] * walk;
					if (e->tileY + yv < 1)
						yv = -e->tileY + 1;
					if (e->tileY + yv > g_hdb->_map->_height)
						yv = g_hdb->_map->_height - e->tileY - 1;

					e->value1 = xvAhead[d];
					e->value2 = yvAhead[d];
					e->moveSpeed = kPlayerMoveSpeed;
					if (!g_hdb->getActionMode())
						e->moveSpeed >>= 1;

					AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + e->value1, e->tileY + e->value2, e->level, &result);
					uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX + e->value1, e->tileY + e->value2);
					if (hit == p && !g_hdb->_ai->playerDead()) {
						g_hdb->_ai->killPlayer(DEATH_FRIED);
						hit = nullptr;
					}

					if (!hit && result && !(bg_flags & kFlagSpecial)) {
						g_hdb->_ai->setEntityGoal(e, e->tileX + xv, e->tileY + yv);
						g_hdb->_ai->animateEntity(e);
						if (e->onScreen && !g_hdb->_rnd->getRandomNumber(19))
							g_hdb->_sound->playSound(SND_BADFAIRY_AMBIENT);
						return;
					}
					tries--;		// don't lock the system if the player gets the fairy cornered
				} while (!result && tries);

				// couldn't find a place to move so just sit here for a sec & try again
				e->dir = DIR_NONE;
				e->state = STATE_MOVEDOWN;
				e->sequence = 1;
				e->value1 = e->value2 = e->xVel = e->yVel = 0;
			}
			break;
		default:
			break;
		}
		g_hdb->_ai->animEntFrames(e);
		return;
	}

	// in the process of moving around...
	if (e->goalX) {
		// hit the player?
		if (hitPlayer(e->x, e->y)) {
			g_hdb->_ai->killPlayer(DEATH_FRIED);
			g_hdb->_sound->playSound(SND_MBOT_DEATH);
			return;
		}

		// did we run into a wall, entity, water, slime etc?
		// if so, pick a new direction!
		if (onEvenTile(e->x, e->y)) {
			int	result;
			AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->tileX + e->value1, e->tileY + e->value2, e->level, &result);
			uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX + e->value1, e->tileY + e->value2);
			if (!result || (hit && hit->type != AI_GUY) || (bg_flags & kFlagSpecial)) {
				g_hdb->_ai->stopEntity(e);
				e->state = STATE_MOVEDOWN;
				e->sequence = 20;
				return;
			}
		}
		g_hdb->_ai->animateEntity(e);
	} else {
		// if not, start looking around!
		e->sequence = 20;
	}
}

//-------------------------------------------------------------------
//
//	BADFAIRY's GATE PUDDLE!
//
//-------------------------------------------------------------------

void aiGatePuddleInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiGatePuddleAction;
	e->value1 = 50;
}

void aiGatePuddleInit2(AIEntity *e, int mx, int my) {
}

void aiGatePuddleAction(AIEntity *e, int mx, int my) {
	static const int xva[5] = {9, 0, 0,-1, 1};
	static const int yva[5] = {9,-1, 1, 0, 0};

	AIEntity *p = g_hdb->_ai->getPlayer();

	if (e->goalX) {
		g_hdb->_ai->animateEntity(e);
		if (hitPlayer(e->x, e->y)) {
			for (int i = 0; i < kMaxTeleporters; i++) {
				if (g_hdb->_ai->_teleporters[i].anim1 == 2) {	// PANIC ZONE?
					p->tileX = g_hdb->_ai->_teleporters[i].x1;
					p->tileY = g_hdb->_ai->_teleporters[i].y1;
					p->x = p->tileX * kTileWidth;
					p->y = p->tileY * kTileHeight;
					p->xVel = p->yVel = 0;
					p->goalX = p->goalY = 0;
					p->animFrame = 0;
					p->drawXOff = p->drawYOff = 0;
					p->dir = g_hdb->_ai->_teleporters[i].dir1;
					p->level = g_hdb->_ai->_teleporters[i].level1;
					g_hdb->_ai->addAnimateTarget(p->x, p->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
					g_hdb->_sound->playSound(SND_TELEPORT);
					g_hdb->_ai->clearWaypoints();
					g_hdb->_window->startPanicZone();
					g_hdb->_map->centerMapXY(p->x + 16, p->y + 16);
					switch (p->dir) {
					case DIR_UP:
						g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY - 1);
						break;
					case DIR_DOWN:
						g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY + 1);
						break;
					case DIR_LEFT:
						g_hdb->_ai->setEntityGoal(p, p->tileX - 1, p->tileY);
						break;
					case DIR_RIGHT:
						g_hdb->_ai->setEntityGoal(p, p->tileX + 1, p->tileY);
						break;
					case DIR_NONE:
					default:
						break;
					}
					g_hdb->_ai->_playerEmerging = true;
					break;
				} else if (g_hdb->_ai->_teleporters[i].anim2 == 2) {	// PANIC ZONE?
					p->tileX = g_hdb->_ai->_teleporters[i].x2;
					p->tileY = g_hdb->_ai->_teleporters[i].y2;
					p->x = p->tileX * kTileWidth;
					p->y = p->tileY * kTileHeight;
					p->xVel = p->yVel = 0;
					p->goalX = p->goalY = 0;
					p->animFrame = 0;
					p->drawXOff = p->drawYOff = 0;
					p->dir = g_hdb->_ai->_teleporters[i].dir2;
					p->level = g_hdb->_ai->_teleporters[i].level2;
					g_hdb->_ai->addAnimateTarget(p->x, p->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
					g_hdb->_sound->playSound(SND_TELEPORT);
					g_hdb->_ai->clearWaypoints();
					g_hdb->_window->startPanicZone();
					g_hdb->_map->centerMapXY(p->x + 16, p->y + 16);
					switch (p->dir) {
					case DIR_UP:
						g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY - 1);
						break;
					case DIR_DOWN:
						g_hdb->_ai->setEntityGoal(p, p->tileX, p->tileY + 1);
						break;
					case DIR_LEFT:
						g_hdb->_ai->setEntityGoal(p, p->tileX - 1, p->tileY);
						break;
					case DIR_RIGHT:
						g_hdb->_ai->setEntityGoal(p, p->tileX + 1, p->tileY);
						break;
					case DIR_NONE:
					default:
						break;
					}
					g_hdb->_ai->_playerEmerging = true;
					break;
				}
			}
		}
	} else {
		int rnd = g_hdb->_rnd->getRandomNumber(3) + 1;
		e->dir = (AIDir)rnd;
		int nx = e->tileX + xva[rnd];
		int ny = e->tileY + yva[rnd];

		int move_ok;
		AIEntity *hit = g_hdb->_ai->legalMoveOverWater(nx, ny, e->level, &move_ok);
		if (hit == p)
			hit = nullptr;

		if (!hit && move_ok) {
			uint32	bg_flags = g_hdb->_map->getMapBGTileFlags(nx, ny);
			// Gate Puddles can't go over METAL!!! It's in their genes...
			if (!(bg_flags & kFlagMetal)) {
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_GATEPUDDLE_AMBIENT);

				g_hdb->_ai->setEntityGoal(e, nx, ny);
				e->state = STATE_MOVEDOWN;
				g_hdb->_ai->animateEntity(e);
			}
		}

		// can only move 50 spaces or collisions
		e->value1--;
		if (!e->value1) {
			g_hdb->_ai->addGatePuddle(-1);
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_GATEPUDDLE_DISSIPATE);
			g_hdb->_ai->removeEntity(e);
		}
	}
}

//-------------------------------------------------------------------
//
//	ICEPUFF : Little icy dude peeks out of the ground and pops up and
//		throws a snowball at you if he sees you....then he blasts back
//		into the snow and hides for a while....
//
// Variables used specially:
//	value1, value2	: x,y of snowball
//	dir2			: direction of snowball. DIR_NONE = no snowball
//	sequence	: timer for peeking
//
//-------------------------------------------------------------------

void aiIcePuffSnowballInit(AIEntity *e, int mx, int my) {
	// which direction are we throwing in? Load the graphic if we need to
	switch (e->dir) {
	case DIR_DOWN:
		e->value1 = e->x + 12;
		e->value2 = e->y + 32;
		break;
	case DIR_LEFT:
		e->value1 = e->x - 4;
		e->value2 = e->y + 16;
		break;
	case DIR_RIGHT:
		e->value1 = e->x + 32;
		e->value2 = e->y + 16;
		break;
	default:
		break;
	}
	e->aiDraw = aiIcePuffSnowballDraw;
}

void aiIcePuffSnowballAction(AIEntity *e, int mx, int my) {
	// check for hit BEFORE moving so snowball is closer to object
	// NOTE: Need to do logic in this draw routine just in case the ICEPUFF gets stunned!
	int result;
	AIEntity *hit = g_hdb->_ai->legalMoveOverWater(e->value1 / kTileWidth, e->value2 / kTileHeight, e->level, &result);
	if (hit && hit->type == AI_GUY && !g_hdb->_ai->playerDead()) {
		g_hdb->_ai->killPlayer(DEATH_NORMAL);
		g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_WATER_SPLASH_SIT);
		result = 0; // fall thru...
	}

	// hit something solid - kill the snowball
	if (!result) {
		e->dir2 = DIR_NONE;
		e->aiDraw = nullptr;
		return;
	}

	int speed = kPlayerMoveSpeed;
	if (!g_hdb->getActionMode())
		speed >>= 1;

	switch (e->dir2) {
	case DIR_DOWN:
		e->value2 += speed;
		break;
	case DIR_LEFT:
		e->value1 -= speed;
		break;
	case DIR_RIGHT:
		e->value1 += speed;
		break;
	default:
		break;
	}
}

void aiIcePuffSnowballDraw(AIEntity *e, int mx, int my) {
	// did we throw a snowball?  make it move!
	if (e->dir2 != DIR_NONE)
		aiIcePuffSnowballAction(e, 0, 0);

	switch (e->dir2) {
	case DIR_DOWN:
		if (!g_hdb->_ai->_icepSnowballGfxDown)
			g_hdb->_ai->_icepSnowballGfxDown = g_hdb->_gfx->loadPic(ICEPUFF_SNOWBALL_DOWN);
		g_hdb->_ai->_icepSnowballGfxDown->drawMasked(e->value1 - mx, e->value2 - my);
		break;
	case DIR_LEFT:
		if (!g_hdb->_ai->_icepSnowballGfxLeft)
			g_hdb->_ai->_icepSnowballGfxLeft = g_hdb->_gfx->loadPic(ICEPUFF_SNOWBALL_LEFT);
		g_hdb->_ai->_icepSnowballGfxLeft->drawMasked(e->value1 - mx, e->value2 - my);
		break;
	case DIR_RIGHT:
		if (!g_hdb->_ai->_icepSnowballGfxRight)
			g_hdb->_ai->_icepSnowballGfxRight = g_hdb->_gfx->loadPic(ICEPUFF_SNOWBALL_RIGHT);
		g_hdb->_ai->_icepSnowballGfxRight->drawMasked(e->value1 - mx, e->value2 - my);
		break;
	default:
		break;
	}
}

void aiIcePuffInit(AIEntity *e, int mx, int my) {
	// PEEK - but no head up yet
	e->sequence = 30;				// timed sequence for peeking
	e->state = STATE_ICEP_PEEK;			// start in PEEK mode
	e->dir2 = DIR_NONE;					// no snowball out
	e->aiAction = aiIcePuffAction;
}

void aiIcePuffInit2(AIEntity *e, int mx, int my) {
	// empty frame
	e->draw = e->blinkGfx[3];
}

void aiIcePuffAction(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	switch (e->state) {
	case STATE_ICEP_PEEK:
		e->sequence--;
		switch (e->sequence) {
		case 20: // underground
			e->draw = e->blinkGfx[0];
			break;
		case 16: // peek - looking
			e->draw = e->blinkGfx[1];
			break;
		case 12: // peek - blinking
			e->draw = e->blinkGfx[2];
			break;
		case 8: // peek - looking
			e->draw = e->blinkGfx[1];
			break;
		case 4: // peek - looking
			e->draw = e->blinkGfx[0];
			break;
		case 3:
			if (e->onScreen && !g_hdb->_rnd->getRandomNumber(5))
				g_hdb->_sound->playSound(SND_ICEPUFF_WARNING);
			break;
		case 0:
			// underground
			e->draw = e->blinkGfx[3];
			e->sequence = 30;
			break;
		default:
			break;
		}

		// can we see the player? (and no snowball is out)
		if (e->sequence <= 20 && !g_hdb->_ai->playerDead() && e->onScreen) {
			if (p->tileX == e->tileX && p->tileY > e->tileY && e->dir2 == DIR_NONE) {
				e->dir = DIR_DOWN;
				e->state = STATE_ICEP_APPEAR;
				e->animFrame = 0;
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_ICEPUFF_APPEAR);
			} else if (p->tileY == e->tileY && e->dir2 == DIR_NONE) {
				p->tileX < e->tileX ? e->dir = DIR_LEFT : e->dir = DIR_RIGHT;
				e->state = STATE_ICEP_APPEAR;
				e->animFrame = 0;
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_ICEPUFF_APPEAR);
			}
		}
		break;

	case STATE_ICEP_APPEAR:
		e->draw = e->standupGfx[e->animFrame];

		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle;

		e->animFrame++;
		if (e->animFrame == e->standupFrames) {
			e->animFrame = 0;
			switch (e->dir) {
			case DIR_DOWN:
				e->state = STATE_ICEP_THROWDOWN;
				g_hdb->_sound->playSound(SND_ICEPUFF_THROW);
				break;
			case DIR_LEFT:
				e->state = STATE_ICEP_THROWLEFT;
				g_hdb->_sound->playSound(SND_ICEPUFF_THROW);
				break;
			case DIR_RIGHT:
				e->state = STATE_ICEP_THROWRIGHT;
				g_hdb->_sound->playSound(SND_ICEPUFF_THROW);
				break;
			default:
				break;
			}
		}
		break;

	case STATE_ICEP_THROWDOWN:
		e->draw = e->standdownGfx[e->animFrame];

		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle;

		e->animFrame++;
		if (e->animFrame == e->standdownFrames && e->state != STATE_ICEP_DISAPPEAR) {
			// dir2 = direction snowball is moving
			e->dir2 = e->dir;
			// throw it!
			aiIcePuffSnowballInit(e, 0, 0);
			e->animFrame = 0;
			e->state = STATE_ICEP_DISAPPEAR;
		} else if (e->animFrame == e->special1Frames) {
			e->state = STATE_ICEP_PEEK;
			e->draw = e->blinkGfx[3];
			e->sequence = g_hdb->_rnd->getRandomNumber(99) + 30;
		}
		break;

	case STATE_ICEP_THROWLEFT:
		e->draw = e->standleftGfx[e->animFrame];

		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle;

		e->animFrame++;
		if (e->animFrame == e->standdownFrames && e->state != STATE_ICEP_DISAPPEAR) {
			// dir2 = direction snowball is moving
			e->dir2 = e->dir;
			// throw it!
			aiIcePuffSnowballInit(e, 0, 0);
			e->animFrame = 0;
			e->state = STATE_ICEP_DISAPPEAR;
		} else if (e->animFrame == e->special1Frames) {
			e->state = STATE_ICEP_PEEK;
			e->draw = e->blinkGfx[3];
			e->sequence = g_hdb->_rnd->getRandomNumber(99) + 30;
		}
		break;

	case STATE_ICEP_THROWRIGHT:
		e->draw = e->standrightGfx[e->animFrame];

		// cycle animation frames
		if (e->animDelay-- > 0)
			return;
		e->animDelay = e->animCycle;

		e->animFrame++;
		if (e->animFrame == e->standdownFrames && e->state != STATE_ICEP_DISAPPEAR) {
			// dir2 = direction snowball is moving
			e->dir2 = e->dir;
			// throw it!
			aiIcePuffSnowballInit(e, 0, 0);
			e->animFrame = 0;
			e->state = STATE_ICEP_DISAPPEAR;
		} else if (e->animFrame == e->special1Frames) {
			e->state = STATE_ICEP_PEEK;
			e->draw = e->blinkGfx[3];
			e->sequence = g_hdb->_rnd->getRandomNumber(99) + 30;
		}
		break;

	case STATE_ICEP_DISAPPEAR:
		e->draw = e->special1Gfx[e->animFrame];
	default:
		break;
	}
}

//-------------------------------------------------------------------
//
//	BUZZFLY : Simply flies around on paths.... kills you if you touch him.
//		He pauses at corners, too.
//
//-------------------------------------------------------------------

void aiBuzzflyInit(AIEntity *e, int mx, int my) {
	e->aiAction = aiBuzzflyAction;
	e->sequence = 0;

	g_hdb->_ai->findPath(e);
}

void aiBuzzflyInit2(AIEntity *e, int mx, int my) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
	for (int i = 0; i < e->movedownFrames; i++) {
		e->standdownGfx[i] = e->movedownGfx[i];
		e->standupGfx[i] = e->moveupGfx[i];
		e->standleftGfx[i] = e->moveleftGfx[i];
		e->standrightGfx[i] = e->moverightGfx[i];
	}
	e->standdownFrames = e->movedownFrames;
	e->standupFrames = e->moveupFrames;
	e->standleftFrames = e->moveleftFrames;
	e->standrightFrames = e->moverightFrames;
}

void aiBuzzflyAction(AIEntity *e, int mx, int my) {
	if (!e->goalX) {
		switch (e->sequence) {
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
			if (!e->animFrame && e->animDelay == e->animCycle)
				e->sequence++;

			e->draw = e->standdownGfx[e->animFrame];

			// cycle animation frames
			if (e->animDelay-- > 0)
				return;
			e->animDelay = e->animCycle;
			e->animFrame++;
			if (e->animFrame == e->standdownFrames)
				e->animFrame = 0;

			break;

		case 5:
			g_hdb->_ai->findPath(e);
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_BUZZFLY_FLY);
			e->sequence = 0;
			break;

		default:
			break;
		}
	} else {
		g_hdb->_ai->animateEntity(e);
		if (g_hdb->_ai->checkPlayerCollision(e->x, e->y, 6) && !g_hdb->_ai->playerDead()) {
			g_hdb->_sound->playSound(SND_BUZZFLY_STING);
			g_hdb->_ai->killPlayer(DEATH_GRABBED);
		}
	}
}

//-------------------------------------------------------------------
//
//	DRAGON
//
//-------------------------------------------------------------------

void aiDragonInit(AIEntity *e, int mx, int my) {
	e->state = STATE_STANDDOWN;
	e->sequence = 0;	// 0 = sleeping
	e->aiAction = aiDragonAction;
	e->aiDraw = aiDragonDraw;
	e->animCycle = 10;	// time between flaps

	// need to save the dragon's coords and type in the blocking entity for gem-hit-blocking detection
	AIEntity *block = spawnBlocking(e->tileX - 1, e->tileY, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
	block = spawnBlocking(e->tileX + 1, e->tileY, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
	block = spawnBlocking(e->tileX - 1, e->tileY - 1, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
	block = spawnBlocking(e->tileX + 1, e->tileY - 1, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
	block = spawnBlocking(e->tileX - 1, e->tileY - 2, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
	block = spawnBlocking(e->tileX + 1, e->tileY - 2, e->level);
	block->value1 = (int)AI_DRAGON;
	Common::sprintf_s(block->luaFuncUse, "%03d%03d", e->tileX, e->tileY);
}

void aiDragonInit2(AIEntity *e, int mx, int my) {
	e->draw = nullptr;
	if (!g_hdb->_ai->_gfxDragonAsleep) {
		g_hdb->_ai->_gfxDragonAsleep = g_hdb->_gfx->loadPic(DRAGON_ASLEEP);
		g_hdb->_ai->_gfxDragonFlap[0] = g_hdb->_gfx->loadPic(DRAGON_FLAP1);
		g_hdb->_ai->_gfxDragonFlap[1] = g_hdb->_gfx->loadPic(DRAGON_FLAP2);
		g_hdb->_ai->_gfxDragonBreathe[0] = g_hdb->_gfx->loadPic(DRAGON_BREATHE_START);
		g_hdb->_ai->_gfxDragonBreathe[1] = g_hdb->_gfx->loadPic(DRAGON_BREATHING_1);
		g_hdb->_ai->_gfxDragonBreathe[2] = g_hdb->_gfx->loadPic(DRAGON_BREATHING_2);
	}
}

void aiDragonWake(AIEntity *e, int mx, int my) {
	// woke up, start flapping and breathing!
	e->sequence = 1;
	e->animFrame = 0;
	e->animDelay = e->animCycle;
}

void aiDragonUse(AIEntity *e, int mx, int my) {
	aiDragonWake(e, 0, 0);
}

void aiDragonAction(AIEntity *e, int mx, int my) {
	AIEntity *p = g_hdb->_ai->getPlayer();

	switch (e->sequence) {
	// Sleeping, waiting for the player to wake him up
	case 0:
		if (e->onScreen &&
			p->tileX >= e->tileX - 1 &&
			p->tileX <= e->tileX + 1 &&
			p->tileY <= e->tileY + 1 &&
			p->tileY >= e->tileY - 3) {
			if ((p->state >= STATE_ATK_CLUB_UP &&
				p->state <= STATE_ATK_SLUG_RIGHT) || g_hdb->_window->inPanicZone()) {
				aiDragonWake(e, 0, 0);
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_DRAGON_WAKE);
			}
		}
		break;

	// Woke up - flapping wings 3 times!
	case 1:
		e->animDelay--;

		if (e->animDelay < 1) {
			if (e->onScreen)
				g_hdb->_sound->playSound(SND_DRAGON_WAKE);
			e->animDelay = e->animCycle;
			e->animFrame++;
			if (e->animFrame >= 8) {
				e->animFrame = 0;
				e->sequence = 2;
				e->animCycle = 2;
			}
		}
		break;

	// Start breathing fire!
	case 2:
		e->animDelay--;

		if (e->onScreen)
			g_hdb->_sound->playSound(SND_DRAGON_BREATHEFIRE);
		if (e->animDelay < 1) {
			e->animDelay = e->animCycle;
			e->animFrame++;
			if (e->animFrame >= 1) {
				e->animFrame = 0;
				e->sequence = 3;
				e->animCycle = 2;		// time between flaps
			}
		}

		break;

	// Breathing fire!
	case 3:
		{
			if (hitPlayer(e->x, e->y + 32)) {
				g_hdb->_ai->killPlayer(DEATH_FRIED);
				return;
			}

			// whatever entity is in front of the dragon is gettin' USED!
			AIEntity *hit = g_hdb->_ai->findEntity(e->tileX, e->tileY + 1);
			if (hit) {
				switch (hit->type) {
				case AI_CHICKEN:
					g_hdb->_ai->addAnimateTarget(hit->tileX * kTileWidth, hit->tileY * kTileHeight, 0, 2, ANIM_NORMAL, false, false, GROUP_ENT_CHICKEN_DIE);
					g_hdb->_sound->playSound(SND_CHICKEN_DEATH);
					g_hdb->_ai->removeEntity(hit);
					e->sequence = 4;
					break;
				case AI_MAGIC_EGG:
				case AI_ICE_BLOCK:
					aiMagicEggUse(hit, 0, 0);
					break;
				default:
					if (hit->aiUse)
						hit->aiUse(hit, 0, 0);
					if (hit->luaFuncUse[0])
						g_hdb->_lua->callFunction(hit->luaFuncUse, 0);
				}
			}

			e->animDelay--;

			if (e->animDelay < 1) {
				if (e->onScreen && !(e->animFrame & 7))
					g_hdb->_sound->playSound(SND_DRAGON_BREATHEFIRE);

				e->animDelay = e->animCycle;
				e->animFrame++;
				if (e->animFrame >= 30) {
					e->animFrame = 0;
					e->sequence = 4;
					e->animCycle = 10;		// time between flaps
				}
			}
		}

		break;

	// Done burning - flapping wings 3 times
	case 4:
		e->animDelay--;

		if (e->animDelay < 1) {
			e->animDelay = e->animCycle;
			e->animFrame++;
			if (e->animFrame >= 8) {
				e->animFrame = 0;
				e->sequence = 0;
				if (e->onScreen)
					g_hdb->_sound->playSound(SND_DRAGON_FALLASLEEP);
			}
		}
		break;

	default:
		break;
	}
}

void aiDragonDraw(AIEntity *e, int mx, int my) {
	switch (e->sequence) {
	// sleeping
	case 0:
		g_hdb->_ai->_gfxDragonAsleep->drawMasked(e->x - 32 - mx, e->y - 96 - my);
		break;
	// flapping 3 times
	case 1:
		g_hdb->_ai->_gfxDragonFlap[e->animFrame & 1]->drawMasked(e->x - 32 - mx, e->y - 96 - my);
		break;
	// start breathing (very short)
	case 2:
		g_hdb->_ai->_gfxDragonBreathe[0]->drawMasked(e->x - 32 - mx, e->y - 96 - my);
		break;
	// breathing
	case 3:
		g_hdb->_ai->_gfxDragonBreathe[(e->animFrame & 1) + 1]->drawMasked(e->x - 32 - mx, e->y - 96 - my);
		break;
	// flapping 3 times
	case 4:
		g_hdb->_ai->_gfxDragonBreathe[e->animFrame & 1]->drawMasked(e->x - 32 - mx, e->y - 96 - my);
		break;
	default:
		break;
	}
}

} // End of Namespace
