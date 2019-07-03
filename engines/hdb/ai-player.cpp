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
		warning("AI-PLAYER: aiPlayerInit: DIR_NONE found");
		break;
	}

	e->moveSpeed = kPlayerMoveSpeed;
	strcpy(e->entityName, "player");
	g_hdb->_ai->assignPlayer(e);
}

void aiPlayerInit2(AIEntity *e) {
	if (!g_hdb->_ai->_clubUpGfx[0]) {
		warning("STUB: weapon_sel_gfx uninitialized");
		g_hdb->_ai->_clubUpGfx[0] = new Picture;
		g_hdb->_ai->_clubUpGfx[0]->load(g_hdb->_fileMan->findFirstData("clubup1", TYPE_PIC));
		g_hdb->_ai->_clubUpGfx[1] = new Picture;
		g_hdb->_ai->_clubUpGfx[1]->load(g_hdb->_fileMan->findFirstData("clubup2", TYPE_PIC));
		g_hdb->_ai->_clubUpGfx[2] = new Picture;
		g_hdb->_ai->_clubUpGfx[2]->load(g_hdb->_fileMan->findFirstData("clubup3", TYPE_PIC));
		g_hdb->_ai->_clubUpGfx[3] = new Picture;
		g_hdb->_ai->_clubUpGfx[3]->load(g_hdb->_fileMan->findFirstData("clubup3", TYPE_PIC));

		g_hdb->_ai->_clubDownGfx[0] = new Picture;
		g_hdb->_ai->_clubDownGfx[0]->load(g_hdb->_fileMan->findFirstData("clubdown1", TYPE_PIC));
		g_hdb->_ai->_clubDownGfx[1] = new Picture;
		g_hdb->_ai->_clubDownGfx[1]->load(g_hdb->_fileMan->findFirstData("clubdown2", TYPE_PIC));
		g_hdb->_ai->_clubDownGfx[2] = new Picture;
		g_hdb->_ai->_clubDownGfx[2]->load(g_hdb->_fileMan->findFirstData("clubdown3", TYPE_PIC));
		g_hdb->_ai->_clubDownGfx[3] = new Picture;
		g_hdb->_ai->_clubDownGfx[3]->load(g_hdb->_fileMan->findFirstData("clubdown3", TYPE_PIC));

		g_hdb->_ai->_clubLeftGfx[0] = new Picture;
		g_hdb->_ai->_clubLeftGfx[0]->load(g_hdb->_fileMan->findFirstData("clubleft1", TYPE_PIC));
		g_hdb->_ai->_clubLeftGfx[1] = new Picture;
		g_hdb->_ai->_clubLeftGfx[1]->load(g_hdb->_fileMan->findFirstData("clubleft2", TYPE_PIC));
		g_hdb->_ai->_clubLeftGfx[2] = new Picture;
		g_hdb->_ai->_clubLeftGfx[2]->load(g_hdb->_fileMan->findFirstData("clubleft3", TYPE_PIC));
		g_hdb->_ai->_clubLeftGfx[3] = new Picture;
		g_hdb->_ai->_clubLeftGfx[3]->load(g_hdb->_fileMan->findFirstData("clubleft3", TYPE_PIC));

		g_hdb->_ai->_clubRightGfx[0] = new Picture;
		g_hdb->_ai->_clubRightGfx[0]->load(g_hdb->_fileMan->findFirstData("clubright1", TYPE_PIC));
		g_hdb->_ai->_clubRightGfx[1] = new Picture;
		g_hdb->_ai->_clubRightGfx[1]->load(g_hdb->_fileMan->findFirstData("clubright2", TYPE_PIC));
		g_hdb->_ai->_clubRightGfx[2] = new Picture;
		g_hdb->_ai->_clubRightGfx[2]->load(g_hdb->_fileMan->findFirstData("clubright3", TYPE_PIC));
		g_hdb->_ai->_clubRightGfx[3] = new Picture;
		g_hdb->_ai->_clubRightGfx[3]->load(g_hdb->_fileMan->findFirstData("clubright3", TYPE_PIC));

		g_hdb->_ai->_clubUpFrames = 4;
		g_hdb->_ai->_clubDownFrames = 4;
		g_hdb->_ai->_clubLeftFrames = 4;
		g_hdb->_ai->_clubRightFrames = 4;

		g_hdb->_ai->_slugAttackGfx[0] = new Picture;
		g_hdb->_ai->_slugAttackGfx[0]->load(g_hdb->_fileMan->findFirstData("slug_shot1", TYPE_PIC));
		g_hdb->_ai->_slugAttackGfx[1] = new Picture;
		g_hdb->_ai->_slugAttackGfx[1]->load(g_hdb->_fileMan->findFirstData("slug_shot2", TYPE_PIC));
		g_hdb->_ai->_slugAttackGfx[2] = new Picture;
		g_hdb->_ai->_slugAttackGfx[2]->load(g_hdb->_fileMan->findFirstData("slug_shot3", TYPE_PIC));
		g_hdb->_ai->_slugAttackGfx[3] = new Picture;
		g_hdb->_ai->_slugAttackGfx[3]->load(g_hdb->_fileMan->findFirstData("slug_shot4", TYPE_PIC));

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
	AIState stand[5] = {STATE_NONE, STATE_STANDUP, STATE_STANDDOWN, STATE_STANDLEFT, STATE_STANDRIGHT};
	int xvAhead[5] = {9, 0, 0, -1, 1}, yvAhead[5] = {9, -1, 1, 0, 0};
	AIEntity *hit = NULL;

	// Draw the STUN lightning if it exists
	if (e->sequence) {
		e->aiDraw = aiPlayerDraw;
		warning("STUB: Play SND_STUNNER_FIRE");
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
				debug(9, "STUB: stunEnemy: Play sound");
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
				debug(9, "STUB: stunEnemy: Play sound");
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
			case STATE_GRABUP:		e->draw = e->standupGfx[0];	e->state = STATE_STANDUP; break;
			case STATE_GRABDOWN:	e->draw = e->standdownGfx[0];	e->state = STATE_STANDDOWN; break;
			case STATE_GRABLEFT:	e->draw = e->standleftGfx[0];	e->state = STATE_STANDLEFT; break;
			case STATE_GRABRIGHT:	e->draw = e->standrightGfx[0];	e->state = STATE_STANDRIGHT; break;
			default:
				debug(9, "STUB: stunEnemy: Play sound");
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
		case STATE_ATK_CLUB_UP:		cycleFrames(e, g_hdb->_ai->_clubUpFrames); break;
		case STATE_ATK_CLUB_DOWN:	cycleFrames(e, g_hdb->_ai->_clubDownFrames); break;
		case STATE_ATK_CLUB_LEFT:	cycleFrames(e, g_hdb->_ai->_clubLeftFrames); break;
		case STATE_ATK_CLUB_RIGHT:	cycleFrames(e, g_hdb->_ai->_clubRightFrames); break;
		default:
			debug(9, "STUB: stunEnemy: Play sound");
			break;
		}
		// Whack!
		if ((e->animFrame >= 1) && (e->animDelay == e->animCycle)) {
			switch (e->dir) {
			case DIR_UP:	hit = g_hdb->_ai->playerCollision(32, 0, 16, 16); break;
			case DIR_DOWN:	hit = g_hdb->_ai->playerCollision(0, 32, 16, 16); break;
			case DIR_LEFT:	hit = g_hdb->_ai->playerCollision(16, 16, 32, 0); break;
			case DIR_RIGHT:	hit = g_hdb->_ai->playerCollision(16, 16, 0, 32); break;
			default:
				warning("aiPlayerAction: DIR_NONE found");
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
					warning("STUB: Play MetalorFlesh SND");
					break;
				default:
					warning("aiPlayerAction: DIR_NONE found");
					break;
				}
			}
		}
		if ((!e->animFrame) && (e->animDelay == e->animCycle)) {
			e->state = stand[e->dir];
			e->aiDraw = NULL;
			switch (e->state) {
			case STATE_ATK_CLUB_UP:		e->draw = e->standupGfx[0]; break;
			case STATE_ATK_CLUB_DOWN:	e->draw = e->standdownGfx[0]; break;
			case STATE_ATK_CLUB_LEFT:	e->draw = e->standleftGfx[0]; break;
			case STATE_ATK_CLUB_RIGHT:	e->draw = e->standrightGfx[0]; break;
			default:
				debug(9, "STUB: stunEnemy: Play sound");
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
		debug(9, "STUB: stunEnemy: Play sound");
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
			g_hdb->_ai->playerOnIce() ? warning("STUB: Play SND_STEPS_ICE") : warning("STUB: Play SND_FOOTSTEPS");

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
		warning("AI-PLAYER: aiPlayerDraw: Unintended State");
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
			warning("AI-PLAYER: aiPlayerDraw: DIR_NONE found");
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
	warning("Play SND_GEM_THROW");
}

void aiGemAttackAction(AIEntity *e) {
	int xv[5] = {9, 0, 0, -1, 1}, yv[5] = {9, -1, 1, 0, 0};
	AIEntity *hit;
	int		result;

	switch (e->sequence) {
		// flying out at something
	case 0:
		if (e->goalX)
			g_hdb->_ai->animateEntity(e);
		else {
			g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false);
			g_hdb->_ai->checkAutoList(e, e->tileX, e->tileY);

			hit = g_hdb->_ai->findEntityIgnore(e->tileX, e->tileY, e);
			uint32 bgFlags = g_hdb->_map->getMapBGTileFlags(e->tileX, e->tileY);
			uint32 fgFlags = g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY);
			result = (e->level == 1 ? (bgFlags & (kFlagSolid)) : !(fgFlags & kFlagGrating) && (bgFlags & (kFlagSolid)));
			if (hit) {
				switch (hit->type) {
				case AI_CHICKEN:
					g_hdb->_ai->addAnimateTarget(hit->x, hit->y, 0, 3, ANIM_NORMAL, false, false, GROUP_STEAM_PUFF_SIT);
					g_hdb->_ai->removeEntity(hit);
					warning("Play SND_CHICKEN_BAGAWK");
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

						warning("Play SND_CLUB_HIT_FLESH");
						AIEntity *found = g_hdb->_ai->findEntity(atoi(num1), atoi(num2));
						if (found)
							aiDragonWake(found);
					}
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					warning("Play SND_INV_SELECT");
					break;
				case AI_DRAGON:
					warning("Play SND_CLUB_HIT_FLESH");
					aiDragonWake(hit);
				default:
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					warning("Play SND_CLUB_HIT_FLESH");
				}
				if (e->value1)
					e->sequence = 1;
				else
					g_hdb->_ai->removeEntity(e);	// bye bye!
				return;
			} else if (result) {		// hit a wall
					g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
					warning("Play SND_INV_SELECT");
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
			warning("Play SND_GET_GEM");
		}
		break;
	}
}

void aiChickenAction(AIEntity *e) {
	static int delay = 64;

	if (g_hdb->_map->checkEntOnScreen(e) && !delay) {
		warning("Play SND_CHICKEN_AMBIENT");
		delay = g_hdb->_rnd->getRandomNumber(128) + 160;
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
	warning("Play SND_CHICKEN_BAGAWK");
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
		debug(9, "STUB: AI-PLAYER: aiSergeantAction: Play SND_FOOTSTEPS sounds");
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

void aiBarrelExplosionEnd(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplosionEnd required");
}

void aiBarrelExplosionAction(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplosionAction required");
}

void aiBarrelExplode(AIEntity *e) {
	e->state = STATE_EXPLODING;
	e->animDelay = e->animCycle;
	e->animFrame = 0;
	warning("STUB: Play SND_BARREL_EXPLODE");
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
	AIEntity *e2;
	int	x = e->tileX;
	int	y = e->tileY;
	int	xv, yv;
	int	index = e->animFrame;
	int	xv1[4] = {-1,  1, -1,  0};
	int yv1[4] = {-1, -1,  0, -1};
	int xv2[4] = {1,  0,  1, -1};
	int yv2[4] = {0,  1,  1,  1};

	// are we just starting an explosion ring?
	if (e->animDelay != e->animCycle)
		return;

	// the animation frame is the index into which set of 2 explosions to spawn
	xv = xv1[index];
	yv = yv1[index];

	// explosion 1: check to see if we can explode (non-solid tile)
	// if so, spawn it and mark it in the explosion matrix
	if (!(g_hdb->_map->getMapBGTileFlags(x + xv, y + yv) & kFlagSolid) && !g_hdb->_map->explosionExist(x + xv, y + yv)) {
		aiBarrelBlowup(e, x + xv, y + yv);
		// are we blowing up on another BOOMBARREL?  if so, start it exploding.
		e2 = g_hdb->_ai->findEntity(x + xv, y + yv);
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
		e2 = g_hdb->_ai->findEntity(x + xv, y + yv);
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
	int xv[5] = {9, 0, 0, -1, 1}, yv[5] = {9, -1, 1, 0, 0};
	AIEntity *hit;
	int	result;

	if (e->goalX)
		g_hdb->_ai->animateEntity(e);

	g_hdb->_ai->checkActionList(e, e->tileX, e->tileY, false);
	g_hdb->_ai->checkAutoList(e, e->tileX, e->tileY);

	hit = g_hdb->_ai->findEntityIgnore(e->tileX, e->tileY, e);
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
	result = (e->level == 1 ? (bg_flags & (kFlagSolid)) : !(fg_flags & kFlagGrating) && (bg_flags & (kFlagSolid)));
	if (hit) {
		warning("STUB: Play SND_SLUG_HIT");
		warning("STUB: Play MetalOrFleshSnd");
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
			warning("STUB: Play SND_CLUB_HIT_METAL");
			aiBarrelExplode(hit);
			aiBarrelBlowup(hit, hit->tileX, hit->tileY);
			break;
			// ACTION MODE entities go away - except the FOURFIRER
		case AI_GATEPUDDLE:
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 7, ANIM_NORMAL, false, false, TELEPORT_FLASH);
			g_hdb->_ai->removeEntity(hit);
			warning("STUB: Play SND_TELEPORT");
			break;
		case AI_DEADEYE:
			g_hdb->_ai->addAnimateTarget(e->tileX * kTileWidth,
				e->tileY * kTileHeight, 0, 3, ANIM_NORMAL, false, false, GROUP_EXPLOSION_BOOM_SIT);
			g_hdb->_ai->removeEntity(hit);
			warning("STUB: Play SND_BARREL_EXPLODE");
			break;

		case AI_NONE:
			if (hit->value1 == (int)AI_DRAGON) {
				// pull dragon's coords out of "lua_func_use" string.
				char num1[4], num2[4];
				memset(num1, 0, 4);
				memset(num2, 0, 4);
				memcpy(num1, hit->luaFuncUse, 3);
				memcpy(num2, hit->luaFuncUse + 3, 3);

				warning("STUB: Play SND_CLUB_HIT_FLESH");
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
		warning("STUB: Play SND_SLUG_HIT");
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
	int xv[5] = {9, 0, 0, -1, 1}, yv[5] = {9, -1, 1, 0, 0};
	e->moveSpeed = kPlayerMoveSpeed << 1;
	g_hdb->_ai->setEntityGoal(e, e->tileX + xv[e->dir], e->tileY + yv[e->dir]);
	e->draw = NULL;					// use custom draw function
	e->aiDraw = aiSlugAttackDraw;
	e->state = STATE_MOVEDOWN;		// so it will draw & animate
	e->aiAction = aiSlugAttackAction;
	warning("STUB: Play SND_SLUG_FIRE");
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
	warning("STUB: AI: aiKeycardWhiteInit required");
}

void aiKeycardWhiteInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardWhiteInit2 required");
}

void aiKeycardBlueInit(AIEntity *e) {
	warning("STUB: AI: aiKeycardBlueInit required");
}

void aiKeycardBlueInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardBlueInit2 required");
}

void aiKeycardRedInit(AIEntity *e) {
	warning("STUB: AI: aiKeycardRedInit required");
}

void aiKeycardRedInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardRedInit2 required");
}

void aiKeycardGreenInit(AIEntity *e) {
	warning("STUB: AI: aiKeycardGreenInit required");
}

void aiKeycardGreenInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardGreenInit2 required");
}

void aiKeycardPurpleInit(AIEntity *e) {
	warning("STUB: AI: aiKeycardPurpleInit required");
}

void aiKeycardPurpleInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardPurpleInit2 required");
}

void aiKeycardBlackInit(AIEntity *e) {
	warning("STUB: AI: aiKeycardBlackInit required");
}

void aiKeycardBlackInit2(AIEntity *e) {
	warning("STUB: AI: aiKeycardBlackInit2 required");
}

void aiSeedInit(AIEntity *e) {
	warning("STUB: AI: aiSeedInit required");
}

void aiSeedInit2(AIEntity *e) {
	warning("STUB: AI: aiSeedInit2 required");
}

void aiSodaInit(AIEntity *e) {
	warning("STUB: AI: aiSodaInit required");
}

void aiSodaInit2(AIEntity *e) {
	warning("STUB: AI: aiSodaInit2 required");
}

void aiDollyTool1Init(AIEntity *e) {
	warning("STUB: AI: aiDollyTool1Init required");
}

void aiDollyTool1Init2(AIEntity *e) {
	warning("STUB: AI: aiDollyTool1Init2 required");
}

void aiDollyTool2Init(AIEntity *e) {
	warning("STUB: AI: aiDollyTool2Init required");
}

void aiDollyTool2Init2(AIEntity *e) {
	warning("STUB: AI: aiDollyTool2Init2 required");
}

void aiDollyTool3Init(AIEntity *e) {
	warning("STUB: AI: aiDollyTool3Init required");
}

void aiDollyTool3Init2(AIEntity *e) {
	warning("STUB: AI: aiDollyTool3Init2 required");
}

void aiDollyTool4Init(AIEntity *e) {
	warning("STUB: AI: aiDollyTool4Init required");
}

void aiDollyTool4Init2(AIEntity *e) {
	warning("STUB: AI: aiDollyTool4Init2 required");
}

void aiRouterInit(AIEntity *e) {
	warning("STUB: AI: aiRouterInit required");
}

void aiRouterInit2(AIEntity *e) {
	warning("STUB: AI: aiRouterInit2 required");
}

void aiSlicerInit(AIEntity *e) {
	warning("STUB: AI: aiSlicerInit required");
}

void aiSlicerInit2(AIEntity *e) {
	warning("STUB: AI: aiSlicerInit2 required");
}

void aiPackageInit(AIEntity *e) {
	warning("STUB: AI: aiPackageInit required");
}

void aiPackageInit2(AIEntity *e) {
	warning("STUB: AI: aiPackageInit2 required");
}

void aiMagicEggAction(AIEntity *e) {
	warning("STUB: AI: aiMagicEggAction required");
}

void aiMagicEggInit(AIEntity *e) {
	warning("STUB: AI: aiMagicEggInit required");
}

void aiMagicEggInit2(AIEntity *e) {
	warning("STUB: AI: aiMagicEggInit2 required");
}

void aiMagicEggUse(AIEntity *e) {
	warning("STUB: AI: aiMagicEggUse required");
}

void aiIceBlockAction(AIEntity *e) {
	warning("STUB: AI: aiIceBlockAction required");
}

void aiIceBlockInit(AIEntity *e) {
	warning("STUB: AI: aiIceBlockInit required");
}

void aiIceBlockInit2(AIEntity *e) {
	warning("STUB: AI: aiIceBlockInit2 required");
}

void aiCabKeyInit(AIEntity *e) {
	warning("STUB: AI: aiCabKeyInit required");
}

void aiCabKeyInit2(AIEntity *e) {
	warning("STUB: AI: aiCabKeyInit2 required");
}

void aiItemChickenInit(AIEntity *e) {
	warning("STUB: AI: aiItemChickenInit required");
}

void aiItemChickenInit2(AIEntity *e) {
	warning("STUB: AI: aiItemChickenInit2 required");
}

void aiPdaInit(AIEntity *e) {
	strcpy(e->printedName, "a P.D.A.");
	e->aiAction = aiGetItemAction;
}

void aiPdaInit2(AIEntity *e) {
	e->draw = e->standdownGfx[0];
}

void aiCellUse(AIEntity *e) {
	warning("STUB: AI: aiCellUse required");
}

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
	warning("STUB: AI: aiTransceiverInit required");
}

void aiTransceiverInit2(AIEntity *e) {
	warning("STUB: AI: aiTransceiverInit2 required");
}

void aiTransceiverAction(AIEntity *e) {
	warning("STUB: AI: aiTransceiverAction required");
}

void aiTransceiverUse(AIEntity *e) {
	warning("STUB: AI: aiTransceiverUse required");
}

void aiMonkeystoneInit(AIEntity *e) {
	warning("STUB: AI: aiMonkeystoneInit required");
}

void aiMonkeystoneAction(AIEntity *e) {
	warning("STUB: AI: aiMonkeystoneAction required");
}

void aiMonkeystoneInit2(AIEntity *e) {
	warning("STUB: AI: aiMonkeystoneInit2 required");
}

void aiMonkeystoneUse(AIEntity *e) {
	warning("STUB: AI: aiMonkeystoneUse required");
}

void aiMonkeystoneUse2(AIEntity *e) {
	warning("STUB: AI: aiMonkeystoneUse2 required");
}

void aiGemAction(AIEntity *e) {
	AIEntity *p;
	int tolerance;

	e->animFrame++;
	if (e->animFrame >= e->standdownFrames) {
		e->animFrame = 0;

		// every 4th frame, check for player collision &
		// add to inventory if it happens
		p = g_hdb->_ai->getPlayer();
		tolerance = 16;
		if (g_hdb->_ai->playerRunning())
			tolerance = 24;

		if (e->onScreen && abs(p->x - e->x) < tolerance && abs(p->y - e->y) < tolerance && e->level == p->level) {
			g_hdb->_ai->addAnimateTarget(e->x, e->y, 0, 3, ANIM_NORMAL, false, false, GEM_FLASH);
			g_hdb->_ai->addToInventory(e);
			warning("Play SND_GET_GEM");
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
	warning("STUB: AI: aiGooCupUse required");
}

void aiGooCupInit(AIEntity *e) {
	warning("STUB: AI: aiGooCupInit required");
}

void aiGooCupInit2(AIEntity *e) {
	warning("STUB: AI: aiGooCupInit2 required");
}

void aiVortexianAction(AIEntity *e) {
	warning("STUB: AI: aiVortexianAction required");
}

void aiVortexianUse(AIEntity *e) {
	warning("STUB: AI: aiVortexianUse required");
}

void aiVortexianInit(AIEntity *e) {
	warning("STUB: AI: aiVortexianInit required");
}

void aiVortexianInit2(AIEntity *e) {
	warning("STUB: AI: aiVortexianInit2 required");
}

void aiNoneInit(AIEntity *e) {
	warning("STUB: AI: aiNoneInit required");
}

void aiNoneInit2(AIEntity *e) {
	warning("STUB: AI: aiNoneInit2 required");
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
		debug(9, "STUB: Play SND_FOOTSTEPS");
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
