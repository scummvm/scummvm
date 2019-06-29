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
		g_hdb->_ai->_stunLightningGfx[0] = g_hdb->_drawMan->getTileGfx("shock_spark_sit01", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit02", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[1] = g_hdb->_drawMan->getTileGfx("shock_spark_sit02", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit03", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[2] = g_hdb->_drawMan->getTileGfx("shock_spark_sit03", size);
		size = g_hdb->_fileMan->getLength("shock_spark_sit04", TYPE_TILE32);
		g_hdb->_ai->_stunLightningGfx[3] = g_hdb->_drawMan->getTileGfx("shock_spark_sit04", size);

		size = g_hdb->_fileMan->getLength("starstun_sit01", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[0] = g_hdb->_drawMan->getTileGfx("starstun_sit01", size);
		size = g_hdb->_fileMan->getLength("starstun_sit02", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[1] = g_hdb->_drawMan->getTileGfx("starstun_sit02", size);
		size = g_hdb->_fileMan->getLength("starstun_sit03", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[2] = g_hdb->_drawMan->getTileGfx("starstun_sit03", size);
		size = g_hdb->_fileMan->getLength("starstun_sit04", TYPE_TILE32);
		g_hdb->_ai->_stunnedGfx[3] = g_hdb->_drawMan->getTileGfx("starstun_sit04", size);
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
		if (!g_hdb->_ai->cinematicsActive() && g_hdb->_drawMan->isFadeStaying())
			g_hdb->_drawMan->turnOffFade();

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
	warning("STUB: AI: aiGemAttackInit required");
}

void aiGemAttackInit2(AIEntity *e) {
	warning("STUB: AI: aiGemAttackInit2 required");
}

void aiGemAttackAction(AIEntity *e) {
	warning("STUB: AI: aiGemAttackAction required");
}

void aiChickenAction(AIEntity *e) {
	warning("STUB: AI: aiChickenAction required");
}

void aiChickenUse(AIEntity *e) {
	warning("STUB: AI: aiChickenUse required");
}

void aiChickenInit(AIEntity *e) {
	warning("STUB: AI: aiChickenInit required");
}

void aiChickenInit2(AIEntity *e) {
	warning("STUB: AI: aiChickenInit2 required");
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
	warning("STUB: AI: aiCrateAction required");
}

void aiCrateInit2(AIEntity *e) {
	warning("STUB: AI: aiCrateInit2 required");
}

void aiCrateInit(AIEntity *e) {
	warning("STUB: AI: aiCrateInit required");
}

void aiBarrelLightAction(AIEntity *e) {
	warning("STUB: AI: aiBarrelLightAction required");
}

void aiBarrelLightInit2(AIEntity *e) {
	warning("STUB: AI: aiBarrelLightInit2 required");
}

void aiBarrelLightInit(AIEntity *e) {
	warning("STUB: AI: aiBarrelLightInit required");
}

void aiBarrelHeavyAction(AIEntity *e) {
	warning("STUB: AI: aiBarrelHeavyAction required");
}

void aiBarrelHeavyInit2(AIEntity *e) {
	warning("STUB: AI: aiBarrelHeavyInit2 required");
}

void aiBarrelHeavyInit(AIEntity *e) {
	warning("STUB: AI: aiBarrelHeavyInit required");
}

void aiBarrelExplosionEnd(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplosionEnd required");
}

void aiBarrelExplosionAction(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplosionAction required");
}

void aiBarrelExplode(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplode required");
}

void aiBarrelExplodeInit(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplodeInit required");
}

void aiBarrelExplodeInit2(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplodeInit2 required");
}

void aiBarrelExplodeAction(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplodeAction required");
}

void aiBarrelExplodeSpread(AIEntity *e) {
	warning("STUB: AI: aiBarrelExplodeSpread required");
}

void aiBarrelExplosionEnd(int x, int y) {
	warning("STUB: AI: aiBarrelExplosionEnd(int, int) required");
}

void aiBarrelBlowup(AIEntity *e, int x, int y) {
	warning("STUB: AI: aiBarrelBlowup required");
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
	warning("STUB: AI: aiSlugAttackAction required");
}

void aiSlugAttackDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiSlugAttackDraw required");
}

void aiSlugAttackInit(AIEntity *e) {
	warning("STUB: AI: aiSlugAttackInit required");
}

void aiSlugAttackInit2(AIEntity *e) {
	warning("STUB: AI: aiSlugAttackInit2 required");
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
	warning("STUB: AI: aiFrogStatueInit required");
}

void aiFrogStatueInit2(AIEntity *e) {
	warning("STUB: AI: aiFrogStatueInit2 required");
}

void aiFrogStatueAction(AIEntity *e) {
	warning("STUB: AI: aiFrogStatueAction required");
}

void aiRoboStunnerAction(AIEntity *e) {
	warning("STUB: AI: aiRoboStunnerAction required");
}

void aiRoboStunnerInit(AIEntity *e) {
	warning("STUB: AI: aiRoboStunnerInit required");
}

void aiRoboStunnerInit2(AIEntity *e) {
	warning("STUB: AI: aiRoboStunnerInit2 required");
}

void aiClubInit(AIEntity *e) {
	warning("STUB: AI: aiClubInit required");
}

void aiClubInit2(AIEntity *e) {
	warning("STUB: AI: aiClubInit2 required");
}

void aiSlugSlingerInit(AIEntity *e) {
	warning("STUB: AI: aiSlugSlingerInit required");
}

void aiSlugSlingerInit2(AIEntity *e) {
	warning("STUB: AI: aiSlugSlingerInit2 required");
}

void aiLaserAction(AIEntity *e) {
	warning("STUB: AI: aiLaserAction required");
}

void aiLaserDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiLaserDraw required");
}

void aiDiverterAction(AIEntity *e) {
	warning("STUB: AI: aiDiverterAction required");
}

void aiDiverterDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiDiverterDraw required");
}

void aiMeerkatDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiMeerkatDraw required");
}

void aiMeerkatAction(AIEntity *e) {
	warning("STUB: AI: aiMeerkatAction required");
}

void aiMeerkatLookAround(AIEntity *e) {
	warning("STUB: AI: aiMeerkatLookAround required");
}

void aiFatFrogAction(AIEntity *e) {
	warning("STUB: AI: aiFatFrogAction required");
}

void aiFatFrogTongueDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiFatFrogTongueDraw required");
}

void aiGoodFairyAction(AIEntity *e) {
	warning("STUB: AI: aiGoodFairyAction required");
}

void aiBadFairyAction(AIEntity *e) {
	warning("STUB: AI: aiBadFairyAction required");
}

void aiGatePuddleAction(AIEntity *e) {
	warning("STUB: AI: aiGatePuddleAction required");
}

void aiIcePuffSnowballAction(AIEntity *e) {
	warning("STUB: AI: aiIcePuffSnowballAction required");
}

void aiIcePuffSnowballDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiIcePuffSnowballDraw required");
}

void aiIcePuffAction(AIEntity *e) {
	warning("STUB: AI: aiIcePuffAction required");
}

void aiBuzzflyAction(AIEntity *e) {
	warning("STUB: AI: aiBuzzflyAction required");
}

void aiDragonAction(AIEntity *e) {
	warning("STUB: AI: aiDragonAction required");
}

void aiDragonDraw(AIEntity *e, int mx, int my) {
	warning("STUB: AI: aiDragonDraw required");
}

void aiListenBotInit(AIEntity *e) {
	warning("STUB: AI: aiListenBotInit required");
}

void aiListenBotInit2(AIEntity *e) {
	warning("STUB: AI: aiListenBotInit2 required");
}

void aiLaserInit(AIEntity *e) {
	warning("STUB: AI: aiLaserInit required");
}

void aiLaserInit2(AIEntity *e) {
	warning("STUB: AI: aiLaserInit2 required");
}

void aiDiverterInit(AIEntity *e) {
	warning("STUB: AI: aiDiverterInit required");
}

void aiDiverterInit2(AIEntity *e) {
	warning("STUB: AI: aiDiverterInit2 required");
}

void aiMeerkatInit(AIEntity *e) {
	warning("STUB: AI: aiMeerkatInit required");
}

void aiMeerkatInit2(AIEntity *e) {
	warning("STUB: AI: aiMeerkatInit2 required");
}

void aiFatFrogInit(AIEntity *e) {
	warning("STUB: AI: aiFatFrogInit required");
}

void aiFatFrogInit2(AIEntity *e) {
	warning("STUB: AI: aiFatFrogInit2 required");
}

void aiGoodFairyInit(AIEntity *e) {
	warning("STUB: AI: aiGoodFairyInit required");
}

void aiGoodFairyInit2(AIEntity *e) {
	warning("STUB: AI: aiGoodFairyInit2 required");
}

void aiGoodFairyMoveaway(AIEntity *e) {
	warning("STUB: AI: aiGoodFairyMoveaway required");
}

void aiBadFairyInit(AIEntity *e) {
	warning("STUB: AI: aiBadFairyInit required");
}

void aiBadFairyInit2(AIEntity *e) {
	warning("STUB: AI: aiBadFairyInit2 required");
}

void aiGatePuddleInit(AIEntity *e) {
	warning("STUB: AI: aiGatePuddleInit required");
}

void aiGatePuddleInit2(AIEntity *e) {
	warning("STUB: AI: aiGatePuddleInit2 required");
}

void aiIcePuffInit(AIEntity *e) {
	warning("STUB: AI: aiIcePuffInit required");
}

void aiIcePuffInit2(AIEntity *e) {
	warning("STUB: AI: aiIcePuffInit2 required");
}

void aiBuzzflyInit(AIEntity *e) {
	warning("STUB: AI: aiBuzzflyInit required");
}

void aiBuzzflyInit2(AIEntity *e) {
	warning("STUB: AI: aiBuzzflyInit2 required");
}

void aiDragonInit(AIEntity *e) {
	warning("STUB: AI: aiDragonInit required");
}

void aiDragonInit2(AIEntity *e) {
	warning("STUB: AI: aiDragonInit2 required");
}

void aiDragonWake(AIEntity *e) {
	warning("STUB: AI: aiDragonWake required");
}

void aiDragonUse(AIEntity *e) {
	warning("STUB: AI: aiDragonUse required");
}

void aiEnvelopeGreenInit(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeGreenInit required");
}

void aiEnvelopeGreenInit2(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeGreenInit2 required");
}

void aiGemBlueInit(AIEntity *e) {
	warning("STUB: AI: aiGemBlueInit required");
}

void aiGemBlueInit2(AIEntity *e) {
	warning("STUB: AI: aiGemBlueInit2 required");
}

void aiGemRedInit(AIEntity *e) {
	warning("STUB: AI: aiGemRedInit required");
}

void aiGemRedInit2(AIEntity *e) {
	warning("STUB: AI: aiGemRedInit2 required");
}

void aiGemGreenInit(AIEntity *e) {
	warning("STUB: AI: aiGemGreenInit required");
}

void aiGemGreenInit2(AIEntity *e) {
	warning("STUB: AI: aiGemGreenInit2 required");
}

void aiTeaCupInit(AIEntity *e) {
	warning("STUB: AI: aiTeaCupInit required");
}

void aiTeaCupInit2(AIEntity *e) {
	warning("STUB: AI: aiTeaCupInit2 required");
}

void aiCookieInit(AIEntity *e) {
	warning("STUB: AI: aiCookieInit required");
}

void aiCookieInit2(AIEntity *e) {
	warning("STUB: AI: aiCookieInit2 required");
}

void aiBurgerInit(AIEntity *e) {
	warning("STUB: AI: aiBurgerInit required");
}

void aiBurgerInit2(AIEntity *e) {
	warning("STUB: AI: aiBurgerInit2 required");
}

void aiBookInit(AIEntity *e) {
	warning("STUB: AI: aiBookInit required");
}

void aiBookInit2(AIEntity *e) {
	warning("STUB: AI: aiBookInit2 required");
}

void aiClipboardInit(AIEntity *e) {
	warning("STUB: AI: aiClipboardInit required");
}

void aiClipboardInit2(AIEntity *e) {
	warning("STUB: AI: aiClipboardInit2 required");
}

void aiNoteInit(AIEntity *e) {
	warning("STUB: AI: aiNoteInit required");
}

void aiNoteInit2(AIEntity *e) {
	warning("STUB: AI: aiNoteInit2 required");
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
	warning("STUB: AI: aiPdaInit required");
}

void aiPdaInit2(AIEntity *e) {
	warning("STUB: AI: aiPdaInit2 required");
}

void aiCellUse(AIEntity *e) {
	warning("STUB: AI: aiCellUse required");
}

void aiCellInit2(AIEntity *e) {
	warning("STUB: AI: aiCellInit2 required");
}

void aiCellInit(AIEntity *e) {
	warning("STUB: AI: aiCellInit required");
}

void aiEnvelopeWhiteInit(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeWhiteInit required");
}

void aiEnvelopeWhiteInit2(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeWhiteInit2 required");
}

void aiEnvelopeBlueInit(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeBlueInit required");
}

void aiEnvelopeBlueInit2(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeBlueInit2 required");
}

void aiEnvelopeRedInit(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeRedInit required");
}

void aiEnvelopeRedInit2(AIEntity *e) {
	warning("STUB: AI: aiEnvelopeRedInit2 required");
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
	warning("STUB: AI: aiGemAction required");
}

void aiGemAction2(AIEntity *e) {
	warning("STUB: AI: aiGemAction2 required");
}

void aiGemWhiteInit(AIEntity *e) {
	warning("STUB: AI: aiGemWhiteInit required");
}

void aiGemWhiteInit2(AIEntity *e) {
	warning("STUB: AI: aiGemWhiteInit2 required");
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

void callbackDoorOpenClose(int x, int y) {
	warning("STUB: AI: callbackDoorOpenClose required");
}

// Utility Functions
void aiAnimateStanddown(AIEntity *e, int speed) {
	warning("STUB: AI: aiAnimateStanddown required");
}

void aiGenericAction(AIEntity *e) {
	warning("STUB: AI: aiGenericAction required");
}

void aiGetItemAction(AIEntity *e) {
	warning("STUB: AI: aiGetItemAction required");
}

} // End of Namespace
