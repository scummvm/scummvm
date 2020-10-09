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

#include "ngi/ngi.h"

#include "ngi/objectnames.h"
#include "ngi/constants.h"

#include "ngi/gameloader.h"
#include "ngi/motion.h"
#include "ngi/scenes.h"
#include "ngi/statics.h"

#include "ngi/interaction.h"
#include "ngi/behavior.h"


namespace NGI {

void scene38_setBottleState(Scene *sc) {
	ExCommand *ex = sc->getMessageQueueById(QU_SC38_SHOWBOTTLE_ONTABLE)->getExCommandByIndex(0);

	if (g_vars->scene38_bottle->_ox == ex->_x && g_vars->scene38_bottle->_oy == ex->_y) {
		if (g_nmi->lift_checkButton(sO_Level5) ) {
			ex = sc->getMessageQueueById(QU_SC38_SHOWBOTTLE)->getExCommandByIndex(0);

			g_vars->scene38_bottle->setOXY(ex->_x, ex->_y);
			g_vars->scene38_bottle->_priority = ex->_z;

			g_nmi->setObjectState(sO_Bottle_38, g_nmi->getObjectEnumState(sO_Bottle_38, sO_Blocked));
		}
	}
}

void scene38_initScene(Scene *sc) {
	g_vars->scene38_boss = sc->getStaticANIObject1ById(ANI_GLAVAR, -1);
	g_vars->scene38_tally = sc->getStaticANIObject1ById(ANI_DYLDA, -1);
	g_vars->scene38_shorty = sc->getStaticANIObject1ById(ANI_MALYSH, -1);
	g_vars->scene38_domino0 = sc->getStaticANIObject1ById(ANI_DOMINO38, 0);
	g_vars->scene38_dominos = sc->getStaticANIObject1ById(ANI_DOMINOS, 0);
	g_vars->scene38_domino1 = sc->getStaticANIObject1ById(ANI_DOMINO38, 1);
	g_vars->scene38_bottle = sc->getStaticANIObject1ById(ANI_BOTTLE38, 0);
	g_vars->scene38_bossCounter = 0;
	g_vars->scene38_lastBossAnim = 0;
	g_vars->scene38_bossAnimCounter = 0;
	g_vars->scene38_tallyCounter = 15;
	g_vars->scene38_lastTallyAnim = 0;
	g_vars->scene38_tallyAnimCounter = 0;
	g_vars->scene38_shortyCounter = 30;
	g_vars->scene38_lastShortyAnim = 0;
	g_vars->scene38_shortyAnimCounter = 0;

	scene38_setBottleState(sc);

	if (g_nmi->getObjectState(sO_Boss) == g_nmi->getObjectEnumState(sO_Boss, sO_IsSleeping)) {
		g_vars->scene38_shorty->_flags &= 0xFFFB;

		g_vars->scene38_tally->stopAnim_maybe();
		g_vars->scene38_tally->_flags &= 0xFFFB;

		g_vars->scene38_domino0->_flags &= 0xFFFB;
		g_vars->scene38_dominos->_flags &= 0xFFFB;
		g_vars->scene38_domino1->_flags &= 0xFFFB;
	}

	g_nmi->lift_init(sc, QU_SC38_ENTERLIFT, QU_SC38_EXITLIFT);
	g_nmi->lift_setButtonStatics(sc, ST_LBN_0N);
}

void sceneHandler38_tryTakeBottle() {
	g_vars->scene38_boss->changeStatics2(ST_GLV_NOHAMMER);
	g_vars->scene38_boss->startAnim(MV_GLV_LOOKMAN, 0, -1);

	g_vars->scene38_bossCounter = 0;
}

void sceneHandler38_postHammerKick() {
	g_vars->scene38_domino1->setOXY(g_vars->scene38_domino1->_ox, g_vars->scene38_domino1->_oy + 2);
}

void sceneHandler38_propose() {
	if (!g_vars->scene38_tally->_movement) {
		if (g_vars->scene38_tally->_flags & 4) {
			if (!(g_vars->scene38_tally->_flags & 2) && g_vars->scene38_tallyCounter > 0
				&& g_nmi->_rnd.getRandomNumber(32767) < 32767) {
				chainQueue(QU_DLD_DENY, 0);
				g_vars->scene38_tallyCounter = 0;
			}
		}
	}
}

void sceneHandler38_point() {
	if ((!g_vars->scene38_boss->_movement && ((g_vars->scene38_boss->_flags & 4) || !(g_vars->scene38_boss->_flags & 2)))
		&& g_vars->scene38_bossCounter > 0
		&& g_nmi->_rnd.getRandomNumber(32767) < 32767) {
		if (g_vars->scene38_boss->_statics->_staticsId == ST_GLV_HAMMER) {
			chainQueue(QU_GLV_TOSMALL, 0);
			g_vars->scene38_bossCounter = 0;
		} else {
			if (g_vars->scene38_boss->_statics->_staticsId == ST_GLV_NOHAMMER)
				chainQueue(QU_GLV_TOSMALL_NOHMR, 0);

			g_vars->scene38_bossCounter = 0;
		}
	}
}

void sceneHandler38_hammerKick() {
	if (!g_vars->scene38_shorty->_movement) {
		if (g_vars->scene38_shorty->_flags & 4) {
			if (!(g_vars->scene38_shorty->_flags & 2) && g_vars->scene38_shortyCounter > 1
				&& g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_LEFT2
				&& g_nmi->_rnd.getRandomNumber(32767) < 3276) {
				chainQueue(QU_MLS_TURNR, 0);
				g_vars->scene38_shortyCounter = 0;
			}
		}
	}

	g_vars->scene38_domino1->setOXY(g_vars->scene38_domino1->_ox, g_vars->scene38_domino1->_oy - 2);

	if (g_vars->scene38_dominos->_statics->_staticsId == ST_DMS_3)
		g_vars->scene38_dominos->startAnim(MV_DMS_THREE, 0, -1);
	else if (g_vars->scene38_dominos->_statics->_staticsId == ST_DMS_4)
		g_vars->scene38_dominos->startAnim(MV_DMS_FOUR, 0, -1);
}

void sceneHandler38_drink() {
	if (!g_vars->scene38_shorty->_movement) {
		if (g_vars->scene38_shorty->_flags & 4) {
			if (!(g_vars->scene38_shorty->_flags & 2) && g_vars->scene38_shortyCounter > 0
				&& g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_LEFT2
				&& g_nmi->_rnd.getRandomNumber(32767) < 3276) {
				chainQueue(QU_MLS_TURNR, 0);
				g_vars->scene38_shortyCounter = 0;
			}
		}
	}
}

void sceneHandler38_animateAlcoholics() {
	MessageQueue *mq;

	if (g_vars->scene38_boss->_movement || !(g_vars->scene38_boss->_flags & 4) || (g_vars->scene38_boss->_flags & 2)) {
		g_vars->scene38_bossCounter = 0;
	} else {
		g_vars->scene38_bossCounter++;
	}

	if (g_vars->scene38_bossCounter >= 50) {
		int bossSt = g_vars->scene38_boss->_statics->_staticsId;

		if (bossSt == ST_GLV_SLEEP2) {
			g_vars->scene38_bossCounter = 0;
		} else if ((g_vars->scene38_domino0->_flags & 4) && g_vars->scene38_domino0->_statics->_staticsId == ST_DMN38_6) {
			if (bossSt == ST_GLV_HAMMER) {
				chainQueue(QU_GLV_TAKEDOMINO, 1);
				g_vars->scene38_bossCounter = 0;
			}

			if (bossSt == ST_GLV_NOHAMMER) {
				chainQueue(QU_GLV_TAKEDOMINO_NOHMR, 1);
				g_vars->scene38_bossCounter = 0;
			}
		} else {
			if ((g_vars->scene38_bottle->_flags & 4) && g_vars->scene38_bottle->_statics->_staticsId == ST_BTL38_FULL && bossSt == ST_GLV_NOHAMMER) {
				chainQueue(QU_GLV_DRINKBOTTLE, 1);
				g_vars->scene38_bossCounter = 0;
			} else {
				int bossAnim = 0;

				if (g_nmi->_rnd.getRandomNumber(32767) >= 1310 || g_vars->scene38_boss->_statics->_staticsId != ST_GLV_HAMMER) {
					if (g_nmi->_rnd.getRandomNumber(32767) >= 1310) {
						if (g_nmi->_rnd.getRandomNumber(32767) < 1310) {
							if (bossSt == ST_GLV_HAMMER)
								bossAnim = QU_GLV_DRINK;
							else if (bossSt == ST_GLV_NOHAMMER)
								bossAnim = QU_GLV_DRINK_NOHMR;
						}
					} else {
						if (bossSt == ST_GLV_HAMMER)
							bossAnim = QU_GLV_PROPOSE;
						else if (bossSt == ST_GLV_NOHAMMER)
							bossAnim = QU_GLV_PROPOSE_NOHMR;
					}
				} else {
					bossAnim = QU_GLV_HMRKICK;
				}

				if (g_vars->scene38_lastBossAnim == bossAnim) {
					g_vars->scene38_bossAnimCounter++;

					if (g_vars->scene38_bossAnimCounter > 2)
						bossAnim = 0;
				} else {
					g_vars->scene38_lastBossAnim = bossAnim;
					g_vars->scene38_bossAnimCounter = 1;
				}

				if (bossAnim > 0) {
					mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(bossAnim), 0, 0);

					mq->chain(0);

					g_vars->scene38_bossCounter = 0;
				}
			}
		}
	}

	if (g_vars->scene38_tally->_movement || !(g_vars->scene38_tally->_flags & 4) || (g_vars->scene38_tally->_flags & 2)) {
		g_vars->scene38_tallyCounter = 0;
	} else {
		g_vars->scene38_tallyCounter++;
	}

	if (g_vars->scene38_tallyCounter >= 50) {
		int tallyAnim = 0;

		if (g_nmi->_rnd.getRandomNumber(32767) >= 1310) {
			if (g_nmi->_rnd.getRandomNumber(32767) >= 1310) {
				if (g_nmi->_rnd.getRandomNumber(32767) >= 1310) {
					if (g_nmi->_rnd.getRandomNumber(32767) < 1310)
						tallyAnim = QU_DLD_ICK;
				} else {
					tallyAnim = QU_DLD_GLOT;
				}
			} else {
				tallyAnim = QU_DLD_BLINK;
			}
		} else {
			if (g_vars->scene38_domino1->_statics->_staticsId == ST_DMN38_NORM3) {
				tallyAnim = QU_DLD_TAKE1;
			} else if (g_vars->scene38_domino1->_statics->_staticsId == ST_DMN38_NORM4) {
				tallyAnim = QU_DLD_TAKE2;
			}
		}

		if (g_vars->scene38_lastTallyAnim == tallyAnim) {
			g_vars->scene38_tallyAnimCounter++;

			if (g_vars->scene38_tallyAnimCounter++ > 2)
				tallyAnim = 0;
		} else {
			g_vars->scene38_lastTallyAnim = tallyAnim;
			g_vars->scene38_tallyAnimCounter = 1;
		}
		if (tallyAnim > 0) {
			mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(tallyAnim), 0, 0);

			mq->chain(0);
			g_vars->scene38_tallyCounter = 0;
		}
	}

	if (g_vars->scene38_shorty->_movement || !(g_vars->scene38_shorty->_flags & 4) || (g_vars->scene38_shorty->_flags & 2)) {
		g_vars->scene38_shortyCounter = 0;
		return;
	}

	g_vars->scene38_shortyCounter++;

	if (g_vars->scene38_shortyCounter < 50)
		return;

	int shortyAnim = 0;

	if (g_nmi->_rnd.getRandomNumber(32767) >= 1310) {
		if (g_nmi->_rnd.getRandomNumber(32767) >= 1310 || g_vars->scene38_shorty->_statics->_staticsId != ST_MLS_LEFT2) {
			if (g_vars->scene38_boss->_statics->_staticsId != ST_GLV_SLEEP2 && g_vars->scene38_bossCounter > 30 && g_nmi->_rnd.getRandomNumber(32767) < 0x3FFF && g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_LEFT2)
				shortyAnim = QU_MLS_HAND;
		} else {
			shortyAnim = QU_MLS_BLINK;
		}
	} else {
		if (g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_RIGHT2) {
			shortyAnim = QU_MLS_TURNL;
		} else if (g_vars->scene38_shorty->_statics->_staticsId == ST_MLS_LEFT2) {
			shortyAnim = QU_MLS_TURNR;
		}
	}

	if (g_vars->scene38_lastShortyAnim == shortyAnim) {
		g_vars->scene38_shortyAnimCounter++;
		if (g_vars->scene38_shortyAnimCounter > 2)
			return;
	} else {
		g_vars->scene38_lastShortyAnim = shortyAnim;
		g_vars->scene38_shortyAnimCounter = 1;
	}

	if (shortyAnim > 0) {
		mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(shortyAnim), 0, 0);

		mq->chain(0);

		g_vars->scene38_shortyCounter = 0;
	}
}

int sceneHandler38(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(cmd);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_SC38_TRYTAKEBOTTLE:
		sceneHandler38_tryTakeBottle();
		break;

	case MSG_SC38_POSTHMRKICK:
		sceneHandler38_postHammerKick();
		break;

	case MSG_SC38_PROPOSE:
		sceneHandler38_propose();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SC38_POINT:
		sceneHandler38_point();
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case MSG_SC38_HMRKICK:
		sceneHandler38_hammerKick();
		break;

	case MSG_SC38_DRINK:
		sceneHandler38_drink();
		break;

	case 64:
		g_nmi->lift_hoverButton(cmd);
		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(g_nmi->_sceneRect.left + cmd->_x, g_nmi->_sceneRect.top + cmd->_y);

			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);

				cmd->_messageKind = 0;
			}
		}
		break;

	case 33:
		if (g_nmi->_aniMan2) {
			int x = g_nmi->_aniMan2->_ox;

			if (x < g_nmi->_sceneRect.left + 200)
				g_nmi->_currentScene->_x = x - 300 - g_nmi->_sceneRect.left;

			if (x > g_nmi->_sceneRect.right - 200)
				g_nmi->_currentScene->_x = x + 300 - g_nmi->_sceneRect.right;
		}

		sceneHandler38_animateAlcoholics();

		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
