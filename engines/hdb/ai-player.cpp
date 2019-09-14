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

#include "common/random.h"

#include "hdb/hdb.h"
#include "hdb/ai.h"
#include "hdb/ai-player.h"
#include "hdb/file-manager.h"
#include "hdb/gfx.h"
#include "hdb/sound.h"
#include "hdb/menu.h"
#include "hdb/lua-script.h"
#include "hdb/map.h"
#include "hdb/mpc.h"
#include "hdb/window.h"

namespace HDB {

void aiPlayerInit(AIEntity *e) {
	g_hdb->_ai->clearInventory();
	e->aiAction = aiPlayerAction;
	e->draw = g_hdb->_ai->getStandFrameDir(e);

	switch (e->dir) {
	case DIR_UP:
		e->state = STATE_STANDUP;
		break;
	case DIR_DOWN:
		e->state = STATE_STANDDOWN;
		break;
	case DIR_LEFT:
		e->state = STATE_STANDLEFT;
		break;
	case DIR_RIGHT:
		e->state = STATE_STANDRIGHT;
		break;
	case DIR_NONE:
		break;
	}

	e->moveSpeed = kPlayerMoveSpeed;
	strcpy(e->entityName, "player");
	g_hdb->_ai->assignPlayer(e);
}

void aiPlayerInit2(AIEntity *e) {
	if (!g_hdb->_ai->_clubUpGfx[0]) {
		g_hdb->_ai->_weaponSelGfx = g_hdb->_gfx->loadTile(TILE_WEAPON_EQUIPPED);
		g_hdb->_ai->_clubUpGfx[0] = g_hdb->_gfx->getPicGfx(CLUBUP1, -1);
		g_hdb->_ai->_clubUpGfx[1] = g_hdb->_gfx->getPicGfx(CLUBUP2, -1);
		g_hdb->_ai->_clubUpGfx[2] = g_hdb->_gfx->getPicGfx(CLUBUP3, -1);
		g_hdb->_ai->_clubUpGfx[3] = g_hdb->_gfx->getPicGfx(CLUBUP3, -1);

		g_hdb->_ai->_clubDownGfx[0] = g_hdb->_gfx->getPicGfx(CLUBDOWN1, -1);
		g_hdb->_ai->_clubDownGfx[1] = g_hdb->_gfx->getPicGfx(CLUBDOWN2, -1);
		g_hdb->_ai->_clubDownGfx[2] = g_hdb->_gfx->getPicGfx(CLUBDOWN3, -1);
		g_hdb->_ai->_clubDownGfx[3] = g_hdb->_gfx->getPicGfx(CLUBDOWN3, -1);

		g_hdb->_ai->_clubLeftGfx[0] = g_hdb->_gfx->getPicGfx(CLUBLEFT1, -1);
		g_hdb->_ai->_clubLeftGfx[1] = g_hdb->_gfx->getPicGfx(CLUBLEFT2, -1);
		g_hdb->_ai->_clubLeftGfx[2] = g_hdb->_gfx->getPicGfx(CLUBLEFT3, -1);
		g_hdb->_ai->_clubLeftGfx[3] = g_hdb->_gfx->getPicGfx(CLUBLEFT3, -1);

		g_hdb->_ai->_clubRightGfx[0] = g_hdb->_gfx->getPicGfx(CLUBRIGHT1, -1);
		g_hdb->_ai->_clubRightGfx[1] = g_hdb->_gfx->getPicGfx(CLUBRIGHT2, -1);
		g_hdb->_ai->_clubRightGfx[2] = g_hdb->_gfx->getPicGfx(CLUBRIGHT3, -1);
		g_hdb->_ai->_clubRightGfx[3] = g_hdb->_gfx->getPicGfx(CLUBRIGHT3, -1);

		g_hdb->_ai->_clubUpFrames = g_hdb->_ai->_clubDownFrames =
			g_hdb->_ai->_clubLeftFrames = g_hdb->_ai->_clubRightFrames = 4;

		g_hdb->_ai->_slugAttackGfx[0] = g_hdb->_gfx->loadPic(SLUG_SHOT1);
		g_hdb->_ai->_slugAttackGfx[1] = g_hdb->_gfx->loadPic(SLUG_SHOT2);
		g_hdb->_ai->_slugAttackGfx[2] = g_hdb->_gfx->loadPic(SLUG_SHOT3);
		g_hdb->_ai->_slugAttackGfx[3] = g_hdb->_gfx->loadPic(SLUG_SHOT4);

		int32 size = g_hdb->_fileMan->getLength("shock_spark_sit01", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[0] = g_hdb->_gfx->getTileGfx("shock_spark_sit01", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit02", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[1] = g_hdb->_gfx->getTileGfx("shock_spark_sit02", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit03", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[2] = g_hdb->_gfx->getTileGfx("shock_spark_sit03", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit04", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[3] = g_hdb->_gfx->getTileGfx("shock_spark_sit04", size);

		size = g_hdb->_fileMan->getLength("starstun_sit01", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[0] = g_hdb->_gfx->getTileGfx("starstun_sit01", size);
		size = g_hdb->_fileMan->getLength("starstun_sit02", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[1] = g_hdb->_gfx->getTileGfx("starstun_sit02", size);
		size = g_hdb->_fileMan->getLength("starstun_sit03", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[2] = g_hdb->_gfx->getTileGfx("starstun_sit03", size);
		size = g_hdb->_fileMan->getLength("starstun_sit04", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[3] = g_hdb->_gfx->getTileGfx("starstun_sit04", size);
	}

	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiPlayerAction(AIEntity *e) {
	static const AIState stand[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	static const int xvAhead[5] = {9, 0, 0, -1, 1};
	static const int yvAhead[5] = {9, -1, 1, 0, 0};

	AIEntity *hit = NULL;

	// Draw the STUN lightning if it exists
	if (e->sequence) {
		e->aiDraw = aiPlayerDraw;
		g_hdb->_sound->playSound(SND_STUNNER_FIRE);
		hit = g_hdb->_ai->findEntity(e->tileX + xvAhead[e->dir], e->tileY + yvAhead[e->dir]);
		if (hit)
			switch (hit->type) {
			case AI_MEERKAT:
				if (hit->sequence > 2)
					g_hdb->_ai->stunEnemy(hit, 8);
				break;
			case AI_ICEPUFF:
				if (hit->state == STATE_ICEP_APPEAR || hit->state == STATE_ICEP_THROWDOWN || hit->state == STATE_ICEP_THROWLEFT || hit->state == STATE_ICEP_THROWRIGHT) {
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
					g_hdb->_ai->stunEnemy(hit, 8);
				}
				break;
			case AI_BADFAIRY:
			case AI_GOODFAIRY:
			case AI_CHICKEN:
			case AI_OMNIBOT:
			case AI_TURNBOT:
			case AI_PUSHBOT:
			case AI_DEADEYE:
			case AI_FATFROG:
			case AI_BUZZFLY:
			case AI_MAINTBOT:
			case AI_RIGHTBOT:
			case AI_GATEPUDDLE:
				g_hdb->_ai->stunEnemy(hit, 8);
				break;
			default:
				break;
			}

		hit = g_hdb->_ai->findEntity(e->tileX + (xvAhead[e->dir] << 1), e->tileY + (yvAhead[e->dir] << 1));
		if (hit)
			switch (hit->type) {
			case AI_MEERKAT:
				if (hit->sequence > 2)
					g_hdb->_ai->stunEnemy(hit, 8);
				break;
			case AI_ICEPUFF:
				if (hit->state == STATE_ICEP_APPEAR || hit->state == STATE_ICEP_THROWDOWN || hit->state == STATE_ICEP_THROWLEFT || hit->state == STATE_ICEP_THROWRIGHT) {
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
					g_hdb->_ai->stunEnemy(hit, 8);
				}
				break;
			case AI_BADFAIRY:
			case AI_GOODFAIRY:
			case AI_CHICKEN:
			case AI_OMNIBOT:
			case AI_TURNBOT:
			case AI_PUSHBOT:
			case AI_DEADEYE:
			case AI_FATFROG:
			case AI_BUZZFLY:
			case AI_MAINTBOT:
			case AI_RIGHTBOT:
			case AI_GATEPUDDLE:
				g_hdb->_ai->stunEnemy(hit, 8);
				break;
			default:
				break;
			}
	}

	int xOff[] = {0, 0, -8,-16};
	int yOff[] = {-8,-24,-16,-16};
	// If the player is supposed to animate for abit, check for it here
	switch (e->state) {
	case STATE_GRABUP:
	case STATE_GRABDOWN:
	case STATE_GRABLEFT:
	case STATE_GRABRIGHT:
		if (!e->animFrame--) {
			// Done with the Grabbing Animation, switch to standing
			switch (e->state) {
			case STATE_GRABUP:
				e->draw = e->standupGfx[0];
				e->state = STATE_STANDUP;
				break;
			case STATE_GRABDOWN:
				e->draw = e->standdownGfx[0];
				e->state = STATE_STANDDOWN;
				break;
			case STATE_GRABLEFT:
				e->draw = e->standleftGfx[0];
				e->state = STATE_STANDLEFT;
				break;
			case STATE_GRABRIGHT:
				e->draw = e->standrightGfx[0];
				e->state = STATE_STANDRIGHT;
				break;
			default:
				break;
			}
			e->animDelay = 1;
			e->animCycle = 1;
		}
		break;
	case STATE_ATK_CLUB_UP:
	case STATE_ATK_CLUB_DOWN:
	case STATE_ATK_CLUB_LEFT:
	case STATE_ATK_CLUB_RIGHT:
		g_hdb->_ai->setPlayerInvisible(true);
		e->aiDraw = aiPlayerDraw;
		e->drawXOff = xOff[e->state - STATE_ATK_CLUB_UP];
		e->drawYOff = yOff[e->state - STATE_ATK_CLUB_UP];
		switch (e->state) {
		case STATE_ATK_CLUB_UP:
			cycleFrames(e, g_hdb->_ai->_clubUpFrames);
			break;
		case STATE_ATK_CLUB_DOWN:
			cycleFrames(e, g_hdb->_ai->_clubDownFrames);
			break;
		case STATE_ATK_CLUB_LEFT:
			cycleFrames(e, g_hdb->_ai->_clubLeftFrames);
			break;
		case STATE_ATK_CLUB_RIGHT:
			cycleFrames(e, g_hdb->_ai->_clubRightFrames);
			break;
		default:
			break;
		}
		// Whack!
		if ((e->animFrame >= 1) && (e->animDelay == e->animCycle)) {
			switch (e->dir) {
			case DIR_UP:
				hit = g_hdb->_ai->playerCollision(32, 0, 16, 16);
				break;
			case DIR_DOWN:
				hit = g_hdb->_ai->playerCollision(0, 32, 16, 16);
				break;
			case DIR_LEFT:
				hit = g_hdb->_ai->playerCollision(16, 16, 32, 0);
				break;
			case DIR_RIGHT:
				hit = g_hdb->_ai->playerCollision(16, 16, 0, 32);
				break;
			default:
				break;
			}

			if (hit && hit->level == e->level && !hit->stunnedWait) {
				switch (hit->type) {
				case AI_MEERKAT:
					if (hit->sequence > 2)		// out of the ground?
						g_hdb->_ai->stunEnemy(hit, 2);
					break;
				case AI_ICEPUFF:
					if (hit->state == STATE_ICEP_APPEAR ||
						hit->state == STATE_ICEP_THROWDOWN ||
						hit->state == STATE_ICEP_THROWLEFT ||
						hit->state == STATE_ICEP_THROWRIGHT)
						g_hdb->_ai->stunEnemy(hit, 2);
					break;
				case AI_CHICKEN:
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
					g_hdb->_ai->removeEntity(hit);
					break;
				case AI_BADFAIRY:
				case AI_GOODFAIRY:
				case AI_OMNIBOT:
				case AI_TURNBOT:
				case AI_PUSHBOT:
				case AI_DEADEYE:
				case AI_FATFROG:
				case AI_BUZZFLY:
				case AI_MAINTBOT:
				case AI_RIGHTBOT:
				case AI_SHOCKBOT:
				case AI_GATEPUDDLE:
					g_hdb->_ai->stunEnemy(hit, 2);
					g_hdb->_sound->playSound(g_hdb->_ai->metalOrFleshSND(hit));
					break;
				default:
					break;
				}
			}
		}
		if ((!e->animFrame) && (e->animDelay == e->animCycle)) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			switch (e->state) {
			case STATE_ATK_CLUB_UP:
				e->draw = e->standupGfx[0];
				break;
			case STATE_ATK_CLUB_DOWN:
				e->draw = e->standdownGfx[0];
				break;
			case STATE_ATK_CLUB_LEFT:
				e->draw = e->standleftGfx[0];
				break;
			case STATE_ATK_CLUB_RIGHT:
				e->draw = e->standrightGfx[0];
				break;
			default:
				break;
			}
			g_hdb->_ai->setPlayerInvisible(false);
			e->drawXOff = e->drawYOff = 0;
		}
		return;
	case STATE_ATK_STUN_UP:
		e->draw = g_hdb->_ai->_stunUpGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_stunUpFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			e->sequence = 0;
		}
		return;
	case STATE_ATK_STUN_DOWN:
		e->draw = g_hdb->_ai->_stunDownGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_stunDownFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			e->sequence = 0;
		}
		return;
	case STATE_ATK_STUN_LEFT:
		e->draw = g_hdb->_ai->_stunLeftGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_stunLeftFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			e->sequence = 0;
		}
		return;
	case STATE_ATK_STUN_RIGHT:
		e->draw = g_hdb->_ai->_stunRightGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_stunRightFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			e->sequence = 0;
		}
		return;
	case STATE_ATK_SLUG_UP:
		e->draw = g_hdb->_ai->_slugUpGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_slugUpFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->sequence = 0;
		}
		return;
	case STATE_ATK_SLUG_DOWN:
		e->draw = g_hdb->_ai->_slugDownGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_slugDownFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->sequence = 0;
		}
		return;
	case STATE_ATK_SLUG_LEFT:
		e->draw = g_hdb->_ai->_slugLeftGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_slugLeftFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->sequence = 0;
		}
		return;
	case STATE_ATK_SLUG_RIGHT:
		e->draw = g_hdb->_ai->_slugRightGfx[e->animFrame];
		cycleFrames(e, g_hdb->_ai->_slugRightFrames);
		if (!e->animFrame && e->animDelay == e->animCycle) {
			e->state = stand[e->dir];
			e->sequence = 0;
		}
		return;
	default:
		break;
	}

	// If the touchplate wait is on, keep it timing
	if (e->touchpWait) {
		e->touchpWait--;
		if (!e->touchpWait)
			e->touchpTile = -e->touchpTile;
	} else if (e->touchpTile < 0 && (e->touchpX != e->tileX || e->touchpY != e->tileY)) {
		g_hdb->_ai->checkActionList(e, e->touchpX, e->touchpY, false);
		g_hdb->_map->setMapBGTileIndex(e->touchpX, e->touchpY, -e->touchpTile);
		e->touchpX = e->touchpY = e->touchpTile = 0;
	}

	// If the player is moving somewhere, animate him
	int bgFlags, fgFlags;
	if (e->goalX) {
		if (onEvenTile(e->x, e->y)) {
			g_hdb->_ai->playerOnIce() ? g_hdb->_sound->playSound(SND_STEPS_ICE) : g_hdb->_sound->playSound(SND_FOOTSTEPS);

			// Did we just fall down a PLUMMET?
			bgFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
			fgFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
			if ((bgFlags & kFlagPlummet) && !(fgFlags & kFlagGrating) && !g_hdb->_ai->playerDead()) {
				g_hdb->_ai->killPlayer(DEATH_PLUMMET);
				g_hdb->_ai->animEntFrames(e);
				return;
			}
		}
		g_hdb->_ai->animateEntity(e);
	} else {
		// Sometimes the fading stays black
		if (!g_hdb->_ai->cinematicsActive() && g_hdb->_gfx->isFadeStaying())
			g_hdb->_gfx->turnOffFade();

		// Did we just fall down a PLUMMET?
		bgFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
		fgFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
		if ((bgFlags & kFlagPlummet) && !(fgFlags & kFlagGrating) && !g_hdb->_ai->playerDead()) {
			g_hdb->_ai->killPlayer(DEATH_PLUMMET);
			g_hdb->_ai->animEntFrames(e);
			return;
		}

		// Standing on a TouchPlate will activate something WHILE standing on it
		int bgTile = g_hdb->_ai->checkForTouchplate(e->tileX, e->tileY);
		if (bgTile && !e->touchpWait && !e->touchpTile) {
			if (g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false)) {
				e->touchpTile = bgTile;
				e->touchpX = e->tileX;
				e->touchpY = e->tileY;
				e->touchpWait = kPlayerTouchPWait;
				g_hdb->_ai->stopEntity(e);
			}
		}
		g_hdb->_ai->animEntFrames(e);
	}
}

void aiPlayerDraw(AIEntity *e, int mx, int my) {
	switch (e->state) {
	case STATE_ATK_CLUB_UP:
		g_hdb->_ai->_clubUpGfx[e->animFrame]->drawMasked(e->x + e->drawXOff - mx, e->y + e->drawYOff - my);
		break;
	case STATE_ATK_CLUB_DOWN:
		g_hdb->_ai->_clubDownGfx[e->animFrame]->drawMasked(e->x + e->drawXOff - mx, e->y + e->drawYOff - my);
		break;
	case STATE_ATK_CLUB_LEFT:
		g_hdb->_ai->_clubLeftGfx[e->animFrame]->drawMasked(e->x + e->drawXOff - mx, e->y + e->drawYOff - my);
		break;
	case STATE_ATK_CLUB_RIGHT:
		g_hdb->_ai->_clubRightGfx[e->animFrame]->drawMasked(e->x + e->drawXOff - mx, e->y + e->drawYOff - my);
		break;
	default:
		break;
	}

	if (e->sequence) {
		static int frame = 0;
		switch (e->dir) {
		case DIR_UP:
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - mx, e->y - 32 - my);
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - mx, e->y - 64 - my);
			break;
		case DIR_DOWN:
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - mx, e->y + 32 - my);
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - mx, e->y + 64 - my);
			break;
		case DIR_LEFT:
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - 32 - mx, e->y - my);
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x - 64 - mx, e->y - my);
			break;
		case DIR_RIGHT:
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x + 32 - mx, e->y - my);
			g_hdb->_ai->_stunLightningGfx[frame]->drawMasked(e->x + 64 - mx, e->y - my);
			break;
		case DIR_NONE:
			break;
		}

		frame = (frame + 1) & 3;
	}
}

void aiGemAttackInit(AIEntity *e) {
	int xv[5] = {9, 0, 0, -1, 1}, yv[5] = {9, -1, 1, 0, 0};
	e->moveSpeed = kPlayerMoveSpeed << 1;
	g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
	e->state = STATE_MOVEDOWN;		// so it will draw & animate
	e->sequence = 0;	// flying out at something
	e->aiAction = aiGemAttackAction;
	e->draw = e->movedownGfx[0];
	g_hdb->_sound->playSound(SND_GEM_THROW);
}

void aiGemAttackAction(AIEntity *e) {
	static const int xv[5] = {9, 0, 0, -1, 1};
	static const int yv[5] = {9, -1, 1, 0, 0};

	switch (e->sequence) {
		// flying out at something
	case 0:
		if (e->goalX)
			g_hdb->_ai->animateEntity(e);
		else {
			g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false);
			g_hdb->_ai->checkAutoList(e, e->tileX, e->tileY);

			AIEntity *hit = g_hdb->_ai->findEntityIgnore(e->tileX, e->tileY, e);
			uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
			uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
			int result = (e->level == 1 ? (bgFlags & (kFlagSolid)) : !(fgFlags & kFlagGrating) && (bgFlags & (kFlagSolid)));
			if (hit) {
				switch (hit->type) {
				case AI_CHICKEN:
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
					g_hdb->_ai->removeEntity(hit);
					g_hdb->_sound->playSound(SND_CHICKEN_BAGAWK);
					break;
				case AI_BADFAIRY:
					g_hdb->_ai->stunEnemy(hit, 2);
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					break;
				case AI_NONE:
					if (hit->value1 == (int)AI_DRAGON) {
						// pull dragon's coords out of "lua_func_use" string.
						char num1[4], num2[4];
						memset(num1, 0, 4);
						memset(num2, 0, 4);
						memcpy(num1, hit->luaFuncUse, 3);
						memcpy(num2, hit->luaFuncUse + 3, 3);

						g_hdb->_sound->playSound(SND_CLUB_HIT_FLESH);
						AIEntity *found = g_hdb->_ai->findEntity(atoi(num1), atoi(num2));
						if (found)
							aiDragonWake(found);
					}
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					g_hdb->_sound->playSound(SND_INV_SELECT);
					break;
				case AI_DRAGON:
					g_hdb->_sound->playSound(SND_CLUB_HIT_FLESH);
					aiDragonWake(hit);
					// fallthrough
				default:
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					g_hdb->_sound->playSound(SND_CLUB_HIT_FLESH);
				}
				if (e->value1)
					e->sequence = 1;
				else
					g_hdb->_ai->removeEntity(e);	// bye bye!
				return;
			} else if (result) {		// hit a wall
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					g_hdb->_sound->playSound(SND_INV_SELECT);
					// come back to daddy?
					if (e->value1)
						e->sequence = 1;
					else {
						g_hdb->_ai->removeEntity(e);
						return;
					}
			} else {
					g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
					e->state = STATE_MOVEDOWN;		// so it will draw & animate
			}
			g_hdb->_ai->animateEntity(e);
		}
		break;
		// coming back to daddy?
	case 1:
		AIEntity *p = g_hdb->_ai->getPlayer();
		if (e->x < p->x)
			e->x++;
		else
			e->x--;

		if (e->y < p->y)
			e->y++;
		else
			e->y--;

		if (abs(e->x - p->x) < 4 && abs(e->y - p->y) < 4)
		{
			int	amt = g_hdb->_ai->getGemAmount();
			g_hdb->_ai->setGemAmount(amt + 1);
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
			g_hdb->_ai->removeEntity(e);
			g_hdb->_sound->playSound(SND_GET_GEM);
		}
		break;
	}
}

void aiChickenAction(AIEntity *e) {
	static int delay = 64;

	if (g_hdb->_map->checkEntOnScreen(e) && !delay) {
		g_hdb->_sound->playSound(SND_CHICKEN_AMBIENT);
		delay = g_hdb->_rnd->getRandomNumber(127) + 160;
		aiChickenUse(e);
	}

	if (delay)
		delay--;

	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else
		g_hdb->_ai->animEntFrames(e);
}

void aiChickenUse(AIEntity *e) {
	g_hdb->_sound->playSound(SND_CHICKEN_BAGAWK);
}

void aiChickenInit(AIEntity *e) {
	e->aiUse = aiChickenUse;
	e->aiAction = aiChickenAction;
}

void aiChickenInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiDollyInit(AIEntity *e) {
	e->moveSpeed = kPlayerMoveSpeed >> 1;
	e->aiAction = aiGenericAction;
}

void aiDollyInit2(AIEntity *e) {
	e->draw = e->movedownGfx[0];
}

void aiSergeantInit(AIEntity *e) {
	e->moveSpeed = kPlayerMoveSpeed >> 1;
	if (e->value1)
		e->aiAction = aiSergeantAction;
}

void aiSergeantInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiSergeantAction(AIEntity *e) {
	if (e->goalX) {
		g_hdb->_sound->playSound(SND_FOOTSTEPS);
		g_hdb->_ai->animateEntity(e);
	} else
		g_hdb->_ai->animEntFrames(e);
}

void aiSpacedudeInit(AIEntity *e) {
	e->moveSpeed = kPlayerMoveSpeed >> 1;
	if (e->value1)
		e->aiAction = aiGenericAction;
}

void aiSpacedudeInit2(AIEntity *e) {
	e->standdownFrames = 1;
	e->standdownGfx[0] = e->movedownGfx[0];
	e->standupFrames = 1;
	e->standupGfx[0] = e->moveupGfx[0];
	e->standleftFrames = 1;
	e->standleftGfx[0] = e->moveleftGfx[0];
	e->standrightFrames = 1;
	e->standrightGfx[0] = e->moverightGfx[0];
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiCrateAction(AIEntity *e) {
	// if crate isn't moving somewhere, don't move it
	if (!e->goalX) {
		// crate is stopped in the water... should it continue downstream?
		// not if it's marked by the Number of the Beast!
		if (e->state == STATE_FLOATING) {
			if (e->value1 != 0x666) {
				int flags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
				if (flags & (kFlagPushRight | kFlagPushLeft | kFlagPushUp | kFlagPushDown)) {
					g_hdb->_ai->setEntityGoal(e, e->tileX, e->tileY);
					g_hdb->_ai->animateEntity(e);
				} else
					g_hdb->_ai->animEntFrames(e);
			} else
				g_hdb->_ai->animEntFrames(e);
		}
		return;
	}

	g_hdb->_ai->animateEntity(e);
}

void aiCrateInit2(AIEntity *e) {
	// point all crate move frames to the standing one
	e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiCrateInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiCrateAction;
	e->value1 = 0;
}

void aiBarrelLightAction(AIEntity *e) {
	if (!e->goalX) {
		if (e->state == STATE_FLOATING)
			g_hdb->_ai->animEntFrames(e);
		return;
	}

	g_hdb->_ai->animateEntity(e);
}

void aiBarrelLightInit2(AIEntity *e) {
	// point all light barrel move frames to the standing one
	e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiBarrelLightInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiBarrelLightAction;
}

void aiBarrelHeavyAction(AIEntity *e) {
	if (!e->goalX) {
		if (e->state == STATE_FLOATING)
			g_hdb->_ai->animEntFrames(e);
		return;
	}

	g_hdb->_ai->animateEntity(e);
}

void aiBarrelHeavyInit2(AIEntity *e) {
	// point all heavy barrel move frames to the standing one
	e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiBarrelHeavyInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiBarrelHeavyAction;
}

void aiBarrelExplode(AIEntity *e) {
	e->state = STATE_EXPLODING;
	e->animDelay = e->animCycle;
	e->animFrame = 0;

	if (!g_hdb->isDemo())
		g_hdb->_sound->playSound(SND_BARREL_EXPLODE);

	g_hdb->_map->setBoomBarrel(e->tileX, e->tileY, 0);
}

void aiBarrelExplodeInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiBarrelExplodeAction;
	g_hdb->_map->setBoomBarrel(e->tileX, e->tileY, 1);
}

void aiBarrelExplodeInit2(AIEntity *e) {
	// point all exploding barrel MOVE frames to the standing one
	e->blinkFrames =
		e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->blinkGfx[0] =
		e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiBarrelExplodeAction(AIEntity *e) {
	if (e->goalX)
		g_hdb->_ai->animateEntity(e);
	else if (e->state == STATE_EXPLODING)
		g_hdb->_ai->animEntFrames(e);
}

void aiBarrelExplodeSpread(AIEntity *e) {
	static const int xv1[4] = {-1,  1, -1,  0};
	static const int yv1[4] = {-1, -1,  0, -1};
	static const int xv2[4] = {1,  0,  1, -1};
	static const int yv2[4] = {0,  1,  1,  1};

	int	x = e->tileX;
	int	y = e->tileY;
	int	index = e->animFrame;

	// are we just starting an explosion ring?
	if (e->animDelay != e->animCycle)
		return;

	// the animation frame is the index into which set of 2 explosions to spawn
	int xv = xv1[index];
	int yv = yv1[index];

	// explosion 1: check to see if we can explode (non-solid tile)
	// if so, spawn it and mark it in the explosion matrix
	if (!(g_hdb->_map->getMapBGTileFlags(x + xv, y + yv) & kFlagSolid) && !g_hdb->_map->explosionExist(x + xv, y + yv)) {
		aiBarrelBlowup(e, x + xv, y + yv);
		// are we blowing up on another BOOMBARREL?  if so, start it exploding.
		AIEntity *e2 = g_hdb->_ai->findEntity(x + xv, y + yv);
		if (e2 && e2->state != STATE_EXPLODING) {
			switch (e2->type) {
			case AI_GUY:
				g_hdb->_ai->killPlayer(DEATH_FRIED);
				break;
			case AI_BOOMBARREL:
				aiBarrelExplode(e2);
				break;
			case AI_OMNIBOT:
			case AI_TURNBOT:
			case AI_SHOCKBOT:
			case AI_RIGHTBOT:
			case AI_PUSHBOT:
			case AI_RAILRIDER:
			case AI_MAINTBOT:
			case AI_DEADEYE:
			case AI_FATFROG:
			case AI_ICEPUFF:
			case AI_MEERKAT:
			case AI_BUZZFLY:
			case AI_GOODFAIRY:
			case AI_GATEPUDDLE:
			case AI_BADFAIRY:
				g_hdb->_ai->addAnimateTarget(x * kTileWidth,
					y * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
				if (e2->type != AI_LASERBEAM)
					g_hdb->_ai->removeEntity(e2);
				break;
			default:
				break;
			}
		}
	}

	xv = xv2[index];
	yv = yv2[index];

	// explosion 2: check to see if we can explode (non-solid tile)
	// if so, spawn it and mark it in the explosion matrix

	if (!(g_hdb->_map->getMapBGTileFlags(x + xv, y + yv) & kFlagSolid) && !g_hdb->_map->explosionExist(x + xv, y + yv)) {
		aiBarrelBlowup(e, x + xv, y + yv);
		// are we blowing up on another BOOMBARREL?  if so, start it exploding.
		AIEntity *e2 = g_hdb->_ai->findEntity(x + xv, y + yv);
		if (e2 && e2->state != STATE_EXPLODING) {
			switch (e2->type) {
			case AI_GUY:
				g_hdb->_ai->killPlayer(DEATH_FRIED);
				break;
			case AI_BOOMBARREL:
				aiBarrelExplode(e2);
				break;
			case AI_OMNIBOT:
			case AI_TURNBOT:
			case AI_SHOCKBOT:
			case AI_RIGHTBOT:
			case AI_PUSHBOT:
			case AI_RAILRIDER:
			case AI_MAINTBOT:
			case AI_DEADEYE:
			case AI_FATFROG:
			case AI_ICEPUFF:
			case AI_MEERKAT:
			case AI_BUZZFLY:
			case AI_GOODFAIRY:
			case AI_GATEPUDDLE:
			case AI_BADFAIRY:
				g_hdb->_ai->addAnimateTarget(x * kTileWidth,
					y * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
				if (e2->type != AI_LASERBEAM)
					g_hdb->_ai->removeEntity(e2);
				break;
			default:
				break;
			}
		}
	}
}

void aiBarrelExplosionEnd(int x, int y) {
	g_hdb->_map->setExplosion(x, y, 0);
}

void aiBarrelBlowup(AIEntity *e, int x, int y) {
	g_hdb->_ai->addAnimateTarget(x * kTileWidth,
		y * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
	g_hdb->_map->setExplosion(x, y, 1);
	g_hdb->_ai->addCallback(AI_BARREL_EXPLOSION_END, x, y, e->animCycle * 4);
}

void aiScientistInit(AIEntity *e) {
	e->moveSpeed = kPlayerMoveSpeed >> 1;
	if (g_hdb->_ai->findPath(e))
		e->aiAction = aiGenericAction;
	else if (e->value1)
		e->aiAction = aiGenericAction;
}

void aiScientistInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiSlugAttackAction(AIEntity *e) {
	static const int xv[5] = {9, 0, 0, -1, 1};
	static const int yv[5] = {9, -1, 1, 0, 0};

	if (e->goalX)
		g_hdb->_ai->animateEntity(e);

	g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false);
	g_hdb->_ai->checkAutoList(e, e->tileX, e->tileY);

	AIEntity *hit = g_hdb->_ai->findEntityIgnore(e->tileX, e->tileY, e);
	if (hit && hit->type == AI_GUY)
		hit = NULL;

	// don't hit anything you can walk through...
	if (hit && true == g_hdb->_ai->getTableEnt(hit->type))
		hit = NULL;

	// don't hit floating stuff
	if (hit && hit->state == STATE_FLOATING)
		hit = NULL;

	uint32 bg_flags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
	uint32 fg_flags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
	int	result = (e->level == 1 ? (bg_flags & (kFlagSolid)) : !(fg_flags & kFlagGrating) && (bg_flags & (kFlagSolid)));
	if (hit) {
		g_hdb->_sound->playSound(SND_SLUG_HIT);
		g_hdb->_sound->playSound(g_hdb->_ai->metalOrFleshSND(hit));
		switch (hit->type) {
		case AI_MEERKAT:
			if (hit->sequence > 2) {		// out of the ground?
				g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
				g_hdb->_ai->stunEnemy(hit, 8);
			} else {
				g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
				e->state = STATE_MOVEDOWN;		// so it will draw & animate
				g_hdb->_ai->animateEntity(e);
				return;
			}
			break;
		case AI_ICEPUFF:
			if (hit->state == STATE_ICEP_APPEAR ||
				hit->state == STATE_ICEP_THROWDOWN ||
				hit->state == STATE_ICEP_THROWLEFT ||
				hit->state == STATE_ICEP_THROWRIGHT) {
				g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
				g_hdb->_ai->stunEnemy(hit, 8);
			} else {
				g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
				e->state = STATE_MOVEDOWN;		// so it will draw & animate
				g_hdb->_ai->animateEntity(e);
				return;
			}
			break;
		case AI_OMNIBOT:
		case AI_TURNBOT:
		case AI_SHOCKBOT:
		case AI_RIGHTBOT:
		case AI_PUSHBOT:
		case AI_LISTENBOT:
		case AI_MAINTBOT:
		case AI_FATFROG:
		case AI_BADFAIRY:
		case AI_BUZZFLY:
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
			g_hdb->_ai->stunEnemy(hit, 8);
			break;

		case AI_CHICKEN:
			g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
			g_hdb->_ai->removeEntity(hit);
			break;
		case AI_BOOMBARREL:
			g_hdb->_sound->playSound(SND_CLUB_HIT_METAL);
			aiBarrelExplode(hit);
			aiBarrelBlowup(hit, hit->tileX, hit->tileY);
			break;
			// ACTION MODE entities go away - except the FOURFIRER
		case AI_GATEPUDDLE:
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
			g_hdb->_ai->removeEntity(hit);
			g_hdb->_sound->playSound(SND_TELEPORT);
			break;
		case AI_DEADEYE:
			g_hdb->_ai->addAnimateTarget(e->tileX * kTileWidth,
				e->tileY * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
			g_hdb->_ai->removeEntity(hit);
			g_hdb->_sound->playSound(SND_BARREL_EXPLODE);
			break;

		case AI_NONE:
			if (hit->value1 == (int)AI_DRAGON) {
				// pull dragon's coords out of "lua_func_use" string.
				char num1[4], num2[4];
				memset(num1, 0, 4);
				memset(num2, 0, 4);
				memcpy(num1, hit->luaFuncUse, 3);
				memcpy(num2, hit->luaFuncUse + 3, 3);

				g_hdb->_sound->playSound(SND_CLUB_HIT_FLESH);
				AIEntity *found = g_hdb->_ai->findEntity(atoi(num1), atoi(num2));
				if (found)
					aiDragonWake(found);
				g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
			}
			break;
		case AI_DRAGON:
			aiDragonWake(hit);
			break;
		default:
			break;
		}
		g_hdb->_ai->removeEntity(e);	// bye bye!
		return;
	} else if (result) {		// hit a wall
		g_hdb->_sound->playSound(SND_SLUG_HIT);
		g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
		g_hdb->_ai->removeEntity(e);
	} else {
		g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
		e->state = STATE_MOVEDOWN;		// so it will draw & animate
		g_hdb->_ai->animateEntity(e);
	}
}

void aiSlugAttackDraw(AIEntity *e, int mx, int my) {
	g_hdb->_ai->_slugAttackGfx[e->animFrame]->drawMasked(e->x - mx + 8, e->y - my + 8);
}

void aiSlugAttackInit(AIEntity *e) {
	static const int xv[5] = {9,  0, 0, -1, 1};
	static const int yv[5] = {9, -1, 1,  0, 0};

	if (g_hdb->isDemo())
		return;

	e->moveSpeed = kPlayerMoveSpeed << 1;
	g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
	e->draw = NULL;					// use custom draw function
	e->aiDraw = aiSlugAttackDraw;
	e->state = STATE_MOVEDOWN;		// so it will draw & animate
	e->aiAction = aiSlugAttackAction;
	g_hdb->_sound->playSound(SND_SLUG_FIRE);
}

void aiSlugAttackInit2(AIEntity *e) {
	e->movedownFrames = 4;
}

void aiDeadWorkerInit(AIEntity *e) {
}

void aiDeadWorkerInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiWorkerInit(AIEntity *e) {
	if (e->value1)
		e->aiAction = aiGenericAction;
	e->moveSpeed = kPlayerMoveSpeed >> 1;
}

void aiWorkerInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiAccountantInit(AIEntity *e) {
}

void aiAccountantInit2(AIEntity *e) {
	e->draw = g_hdb->_ai->getStandFrameDir(e);
}

void aiFrogStatueInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiFrogStatueAction;
}

void aiFrogStatueInit2(AIEntity *e) {
	// point all frog statue MOVE frames to the standing one
	e->blinkFrames =
		e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->blinkGfx[0] =
		e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiFrogStatueAction(AIEntity *e) {
	// if frog statue isn't moving somewhere, don't move it
	if (!e->goalX)
		return;

	g_hdb->_ai->animateEntity(e);
}

void aiRoboStunnerAction(AIEntity *e) {
	aiAnimateStanddown(e, 1);
	aiGetItemAction(e);
}

void aiRoboStunnerInit(AIEntity *e) {
	e->aiAction = aiRoboStunnerAction;
	strcpy(e->printedName, "Robostunner");
}

void aiRoboStunnerInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiClubInit(AIEntity *e) {
	strcpy(e->printedName, "Creature Clubber");
	e->aiAction = aiGetItemAction;
}

void aiClubInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiSlugSlingerInit(AIEntity *e) {
	strcpy(e->printedName, "Slugslinger");
	e->aiAction = aiGetItemAction;
}

void aiSlugSlingerInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiEnvelopeGreenInit(AIEntity *e) {
	strcpy(e->printedName, "Green envelope");
	e->aiAction = aiGetItemAction;
}

void aiEnvelopeGreenInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiGemBlueInit(AIEntity *e) {
	e->aiAction = aiGemAction;
}

void aiGemBlueInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiGemRedInit(AIEntity *e) {
	e->aiAction = aiGemAction;
}

void aiGemRedInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiGemGreenInit(AIEntity *e) {
	e->aiAction = aiGemAction;
}

void aiGemGreenInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiTeaCupInit(AIEntity *e) {
	strcpy(e->printedName, "a Teacup");
	e->aiAction = aiGetItemAction;
}

void aiTeaCupInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiCookieInit(AIEntity *e) {
	strcpy(e->printedName, "a Cookie");
	e->aiAction = aiGetItemAction;
}

void aiCookieInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiBurgerInit(AIEntity *e) {
	strcpy(e->printedName, "a Burger");
	e->aiAction = aiGetItemAction;
}

void aiBurgerInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiBookInit(AIEntity *e) {
	strcpy(e->printedName, "a Book");
	e->aiAction = aiGetItemAction;
}

void aiBookInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiClipboardInit(AIEntity *e) {
	strcpy(e->printedName, "a Clipboard");
	e->aiAction = aiGetItemAction;
}

void aiClipboardInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiNoteInit(AIEntity *e) {
	strcpy(e->printedName, "a Note");
	e->aiAction = aiGetItemAction;
}

void aiNoteInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardWhiteInit(AIEntity *e) {
	strcpy(e->printedName, "a White keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardWhiteInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardBlueInit(AIEntity *e) {
	strcpy(e->printedName, "a Blue keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardBlueInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardRedInit(AIEntity *e) {
	strcpy(e->printedName, "a Red keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardRedInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardGreenInit(AIEntity *e) {
	strcpy(e->printedName, "a Green keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardGreenInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardPurpleInit(AIEntity *e) {
	strcpy(e->printedName, "a Purple keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardPurpleInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiKeycardBlackInit(AIEntity *e) {
	strcpy(e->printedName, "a Black keycard");
	e->aiAction = aiGetItemAction;
}

void aiKeycardBlackInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiSeedInit(AIEntity *e) {
	strcpy(e->printedName, "some Henscratch");
	e->aiAction = aiGetItemAction;
}

void aiSeedInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiSodaInit(AIEntity *e) {
	strcpy(e->printedName, "a Dr. Frostee");
	e->aiAction = aiGetItemAction;
}

void aiSodaInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiDollyTool1Init(AIEntity *e) {
	strcpy(e->printedName, "Dolly's Wrench");
	e->aiAction = aiGetItemAction;
}

void aiDollyTool1Init2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiDollyTool2Init(AIEntity *e) {
	strcpy(e->printedName, "Dolly's Torch");
	e->aiAction = aiGetItemAction;
}

void aiDollyTool2Init2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiDollyTool3Init(AIEntity *e) {
	strcpy(e->printedName, "Dolly's EMF Resonator");
	e->aiAction = aiGetItemAction;
}

void aiDollyTool3Init2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiDollyTool4Init(AIEntity *e) {
	strcpy(e->printedName, "Dolly's Toolbox");
	e->aiAction = aiGetItemAction;
}

void aiDollyTool4Init2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiRouterInit(AIEntity *e) {
	strcpy(e->printedName, "a Computer Router");
	e->aiAction = aiGetItemAction;
}

void aiRouterInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiSlicerInit(AIEntity *e) {
	strcpy(e->printedName, "a Pizza Slicer");
	e->aiAction = aiGetItemAction;
}

void aiSlicerInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiPackageInit(AIEntity *e) {
	strcpy(e->printedName, "a Package");
	e->aiAction = aiGetItemAction;
}

void aiPackageInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiMagicEggAction(AIEntity *e) {
	// if magic egg isn't moving somewhere, don't move it
	if (!e->goalX)
		return;

	g_hdb->_ai->animateEntity(e);
}

void aiMagicEggInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiMagicEggAction;
}

void aiMagicEggInit2(AIEntity *e) {
	// point all magic egg move frames to the standing one
	e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiMagicEggUse(AIEntity *e) {
	if (!scumm_strnicmp(e->luaFuncAction, "ai_", 3) || !scumm_strnicmp(e->luaFuncAction, "item_", 5)) {
		int	i = 0;
		AIEntity *spawned = NULL;
		while (aiEntList[i].type != END_AI_TYPES) {
			if (!scumm_stricmp(aiEntList[i].luaName, e->luaFuncAction)) {
				spawned = g_hdb->_ai->spawn(aiEntList[i].type, e->dir, e->tileX, e->tileY, NULL, NULL, NULL, DIR_NONE, e->level, 0, 0, 1);
				break;
			}
			i++;
		}
		if (spawned) {
			g_hdb->_ai->addAnimateTarget(e->tileX * kTileWidth,
			e->tileY * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);

			if (!g_hdb->isDemo())
				g_hdb->_sound->playSound(SND_BARREL_EXPLODE);

			g_hdb->_ai->removeEntity(e);
		}
	}
}

void aiIceBlockAction(AIEntity *e) {
	// if ice block isn't moving somewhere, don't move it
	if (!e->goalX)
		return;

	g_hdb->_ai->animateEntity(e);
}

void aiIceBlockInit(AIEntity *e) {
	e->moveSpeed = kPushMoveSpeed;
	e->aiAction = aiIceBlockAction;
}

void aiIceBlockInit2(AIEntity *e) {
	// point all ice block move frames to the standing one
	e->movedownFrames =
		e->moveleftFrames =
		e->moverightFrames =
		e->moveupFrames = 1;

	e->movedownGfx[0] =
		e->moveupGfx[0] =
		e->moveleftGfx[0] =
		e->moverightGfx[0] = e->standdownGfx[0];

	e->draw = e->standdownGfx[0];			// standing frame - doesn't move
}

void aiCabKeyInit(AIEntity *e) {
	strcpy(e->printedName, "a Cabinet key");
}

void aiCabKeyInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiItemChickenInit(AIEntity *e) {
	strcpy(e->printedName, "Cooper's chicken");
}

void aiItemChickenInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiPdaInit(AIEntity *e) {
	strcpy(e->printedName, "a P.D.A.");
	e->aiAction = aiGetItemAction;
}

void aiPdaInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

#if 0
void aiCellUse(AIEntity *e) {
	g_hdb->_window->openMessageBar("You got the Energy Cell!", kMsgDelay);
}
#endif

void aiCellInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiCellInit(AIEntity *e) {
	strcpy(e->printedName, "Energy Cell");
	e->aiAction = aiGetItemAction;
}

void aiEnvelopeWhiteInit(AIEntity *e) {
	strcpy(e->printedName, "White envelope");
	e->aiAction = aiGetItemAction;
}

void aiEnvelopeWhiteInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiEnvelopeBlueInit(AIEntity *e) {
	strcpy(e->printedName, "Blue envelope");
	e->aiAction = aiGetItemAction;
}

void aiEnvelopeBlueInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiEnvelopeRedInit(AIEntity *e) {
	strcpy(e->printedName, "Red envelope");
	e->aiAction = aiGetItemAction;
}

void aiEnvelopeRedInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiTransceiverInit(AIEntity *e) {
	e->aiAction = aiTransceiverAction;
	strcpy(e->printedName, "Transceiver");
}

void aiTransceiverInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiTransceiverAction(AIEntity *e) {
	aiAnimateStanddown(e, 5);
	if (e->onScreen)
		aiGetItemAction(e);
}

#if 0
void aiTransceiverUse(AIEntity *e) {
	g_hdb->_window->openMessageBar("You got the Transceiver!", kMsgDelay);
}
#endif

void aiMonkeystoneInit(AIEntity *e) {
	e->aiUse = aiMonkeystoneUse;
	e->aiAction = aiMonkeystoneAction;
}

void aiMonkeystoneAction(AIEntity *e) {
	if (!e->onScreen)
		return;

	AIEntity *p = g_hdb->_ai->getPlayer();
	if (abs(p->x - e->x) < 16 && abs(p->y - e->y) < 16 && e->level == p->level) {
		if (e->luaFuncUse[0])
			g_hdb->_lua->callFunction(e->luaFuncUse, 0);

		g_hdb->_ai->addToInventory(e);
		aiMonkeystoneUse(NULL);
	}
}

void aiMonkeystoneInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiMonkeystoneUse(AIEntity *e) {
	int	val = g_hdb->_ai->getMonkeystoneAmount();
	Common::String monkString = Common::String::format("You have %d Monkeystone%s!", val, (val > 1) ? "s" : "");
	g_hdb->_sound->playSound(SND_GET_MONKEYSTONE);
	g_hdb->_window->openMessageBar(monkString.c_str(), kMsgDelay);

	// have we unlocked a secret star(tm)???
	if (val == 7) {
		g_hdb->_window->openMessageBar("Red Star is Ready!", kMsgDelay * 2);
		g_hdb->setStarsMonkeystone7(STARS_MONKEYSTONE_7);
		g_hdb->_menu->writeConfig();
		g_hdb->_gfx->turnOnBonusStars(0);
	}
	if (val == 14) {
		g_hdb->_window->openMessageBar("Green Star is GO!", kMsgDelay * 2);
		g_hdb->setStarsMonkeystone14(STARS_MONKEYSTONE_14);
		g_hdb->_menu->writeConfig();
		g_hdb->_gfx->turnOnBonusStars(1);
	}
	if (val == 21) {
		g_hdb->_window->openMessageBar("Blue Star is Born!", kMsgDelay * 2);
		g_hdb->setStarsMonkeystone21(STARS_MONKEYSTONE_21);
		g_hdb->_menu->writeConfig();
		g_hdb->_gfx->turnOnBonusStars(2);
	}
}

void aiGemAction(AIEntity *e) {
	e->animFrame++;
	if (e->animFrame >= e->standdownFrames) {
		e->animFrame = 0;

		// every 4th frame, check for player collision &
		// add to inventory if it happens
		AIEntity *p = g_hdb->_ai->getPlayer();
		int tolerance = 16;
		if (g_hdb->_ai->playerRunning())
			tolerance = 24;

		if (e->onScreen && abs(p->x - e->x) < tolerance && abs(p->y - e->y) < tolerance && e->level == p->level) {
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
			g_hdb->_ai->addToInventory(e);
			g_hdb->_sound->playSound(SND_GET_GEM);
			return;
		}
	}
	e->draw = e->standdownGfx[e->animFrame];
}

void aiGemWhiteInit(AIEntity *e) {
	e->aiAction = aiGemAction;
}

void aiGemWhiteInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiGooCupUse(AIEntity *e) {
	g_hdb->_window->openMessageBar("Got a... cup of goo.", kMsgDelay);
}

void aiGooCupInit(AIEntity *e) {
	e->aiUse = aiGooCupUse;
	e->aiAction = aiGetItemAction;
}

void aiGooCupInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiVortexianAction(AIEntity *e) {
	// anim the alpha blending : down to 32, up to 180, back down...
	e->value2 += e->value1;
	if ((e->value2 & 0xff) > 128) {
		e->value2 = (e->value2 & 0xff00) | 128;
		e->value1 = -e->value1;
	} else if ((e->value2 & 0xff) < 32) {
		e->value2 = (e->value2 & 0xff00) | 32;
		e->value1 = -e->value1;
	}

	// anim the shape
	e->animFrame++;
	if (e->animFrame >= e->standdownFrames) {
		e->animFrame = 0;

		// every 4th frame, check for player collision &
		// do an autosave
		AIEntity *p = g_hdb->_ai->getPlayer();
		if (abs(p->x - e->x) < 4 && abs(p->y - e->y) < 4) {
			if (!(e->value2 & 0xff00)) {
				// let's make sure we don't autosave every frikken second!
				e->value2 |= 0xff00;

				g_hdb->saveWhenReady(kAutoSaveSlot);
				g_hdb->_window->openMessageBar("Saving progress at Vortexian...", 1);
			}
		} else
			e->value2 &= 0x00ff;
	}
	e->draw = e->standdownGfx[e->animFrame];
}

void aiVortexianUse(AIEntity *e) {
}

void aiVortexianInit(AIEntity *e) {
	e->aiUse = aiVortexianUse;
	e->aiAction = aiVortexianAction;
	e->value1 = 5;
	e->value2 = 128;
}

void aiVortexianInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiNoneInit(AIEntity *e) {
}

// Utility Functions
void aiAnimateStanddown(AIEntity *e, int speed) {
	if (e->value2-- > 0)
		return;
	e->value2 = speed;

	if (e->type == AI_GUY && e->animFrame > 0)
		e->value2 = 0;
	e->draw = e->standdownGfx[e->animFrame];
	e->animFrame++;
	if (e->animFrame >= e->standdownFrames)
		e->animFrame = 0;
}

void aiGenericAction(AIEntity *e) {
	if (!e->goalX)
		g_hdb->_ai->findPath(e);
	else if (onEvenTile(e->x, e->y))
		g_hdb->_sound->playSound(SND_FOOTSTEPS);
	g_hdb->_ai->animateEntity(e);
}

void aiGetItemAction(AIEntity *e) {
	if (!e->onScreen)
		return;

	AIEntity *p = g_hdb->_ai->getPlayer();
	if (abs(p->x - e->x) < 16 && abs(p->y - e->y) < 16 && e->level == p->level) {
		if (e->aiUse)
			e->aiUse(e);
		if (e->luaFuncUse[0])
			g_hdb->_lua->callFunction(e->luaFuncUse, 0);

		g_hdb->_ai->getItemSound(e->type);
		g_hdb->_ai->addToInventory(e);
	}
}

} // End of Namespace
