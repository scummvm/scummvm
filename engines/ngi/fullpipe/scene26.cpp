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

void scene26_initScene(Scene *sc) {
	g_vars->scene26_chhi = sc->getStaticANIObject1ById(ANI_CHHI, -1);
	g_vars->scene26_drop = sc->getStaticANIObject1ById(ANI_DROP_26, -1);
	g_vars->scene26_sockPic = sc->getPictureObjectById(PIC_SC26_SOCK, 0);
	g_vars->scene26_sock = sc->getStaticANIObject1ById(ANI_SOCK_26, -1);

	if (g_nmi->getObjectState(sO_Hatch_26) == g_nmi->getObjectEnumState(sO_Hatch_26, sO_WithSock)) {
		g_nmi->setObjectState(sO_Hatch_26, g_nmi->getObjectEnumState(sO_Hatch_26, sO_Closed));
		g_nmi->setObjectState(sO_Sock_26, g_nmi->getObjectEnumState(sO_Sock_26, sO_HangsOnPipe));
	}

	Interaction *inter = getGameLoaderInteractionController()->getInteractionByObjectIds(ANI_LUK26, ANI_MAN, ANI_INV_SOCK);

	if (getGameLoaderInventory()->getCountItemsWithId(ANI_INV_VENT) == 0)
		inter->_flags &= 0xFFFDFFFF;
	else
		inter->_flags |= 0x20000;

	if (g_nmi->getObjectState(sO_Sock_26) == g_nmi->getObjectEnumState(sO_Sock_26, sO_HangsOnPipe))
		g_vars->scene26_sockPic->_flags |= 4;
	else
		g_vars->scene26_sockPic->_flags &= 0xFFFB;

	if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Opened))
		g_nmi->playSound(SND_26_018, 1);
}

int scene26_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_objectIdAtCursor != ANI_VENT || g_nmi->_cursorId != PIC_CSR_DEFAULT) {
		if (g_nmi->_cursorId == PIC_CSR_ITN && g_nmi->_objectIdAtCursor == PIC_SC26_LTRUBA)
			g_nmi->_cursorId = PIC_CSR_GOL;

		// WORKAROUND: Fixing cursor
		if (g_nmi->_objectIdAtCursor == PIC_SC26_LTRUBA &&
			(g_nmi->_cursorId == PIC_CSR_GOU || g_nmi->_cursorId == PIC_CSR_GOD || g_nmi->_cursorId == PIC_CSR_GOR))
				g_nmi->_cursorId = PIC_CSR_GOL;
	} else {
		g_nmi->_cursorId = PIC_CSR_ITN;
	}

  return g_nmi->_cursorId;
}

void sceneHandler26_updateDrop() {
	if (g_nmi->getObjectState(sO_Valve5_26) == g_nmi->getObjectEnumState(sO_Valve5_26, sO_Closed))
		g_nmi->_behaviorManager->setFlagByStaticAniObject(g_vars->scene26_drop, 0);
	else
		g_nmi->_behaviorManager->setFlagByStaticAniObject(g_vars->scene26_drop, 1);
}

void scene26_setupDrop(Scene *sc) {
	sceneHandler26_updateDrop();
}

void sceneHandler26_showChi() {
	g_vars->scene26_chhi->changeStatics2(ST_CHI_EMPTY);

	chainQueue(QU_CHI_SHOW, 1);
}

void sceneHandler26_updatePool() {
	if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Opened))
		g_nmi->setObjectState(sO_Pool, g_nmi->getObjectEnumState(sO_Pool, sO_Overfull));
	else if (g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_Overfull))
		g_nmi->setObjectState(sO_Pool, g_nmi->getObjectEnumState(sO_Pool, sO_Full));

	if (g_nmi->getObjectState(sO_Valve2_26) == g_nmi->getObjectEnumState(sO_Valve2_26, sO_Opened)) {
		if (g_nmi->getObjectState(sO_Pool) >= g_nmi->getObjectEnumState(sO_Pool, sO_Full))
			g_nmi->setObjectState(sO_Pool, g_nmi->getObjectEnumState(sO_Pool, sO_HalfFull));
	}

	if (g_nmi->getObjectState(sO_Valve3_26) == g_nmi->getObjectEnumState(sO_Valve3_26, sO_Opened)) {
		if (g_nmi->getObjectState(sO_Pool) >= g_nmi->getObjectEnumState(sO_Pool, sO_HalfFull))
			g_nmi->setObjectState(sO_Pool, g_nmi->getObjectEnumState(sO_Pool, sO_Empty));
	}
}

void sceneHandler26_hideChi() {
	g_vars->scene26_chhi->changeStatics2(ST_CHI_NORM);

	chainQueue(QU_CHI_HIDE, 1);
}

void sceneHandler26_testVent() {
	if (!g_vars->scene26_activeVent)
		return;

	if (g_vars->scene26_activeVent->_odelay == 0) {
		if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Opened))
			g_nmi->stopAllSoundInstances(SND_26_018);
		else
			g_nmi->playSound(SND_26_018, 1);

		if (g_nmi->getObjectState(sO_Valve2_26) == g_nmi->getObjectEnumState(sO_Valve2_26, sO_Opened)) {
			chainQueue(QU_SC26_AUTOCLOSE2, 0);

			g_nmi->playSound(SND_26_020, 0);
		}

		if (g_nmi->getObjectState(sO_Valve3_26) == g_nmi->getObjectEnumState(sO_Valve3_26, sO_Opened)) {
			chainQueue(QU_SC26_AUTOCLOSE3, 0);

			g_nmi->playSound(SND_26_020, 0);
		}
	} else if (g_vars->scene26_activeVent->_odelay == 1) {
		if (g_nmi->getObjectState(sO_Valve2_26) == g_nmi->getObjectEnumState(sO_Valve2_26, sO_Opened))
			g_nmi->playSound(SND_26_020, 0);
		else
			g_nmi->playSound(SND_26_019, 0);

		if (g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_Overfull)
			|| g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_Full))
			g_nmi->playSound(SND_26_003, 0);

		if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Opened)) {
			g_nmi->stopAllSoundInstances(SND_26_018);

			chainQueue(QU_SC26_AUTOCLOSE1, 0);
		}
	} else if (g_vars->scene26_activeVent->_odelay == 2) {
		if (g_nmi->getObjectState(sO_Valve3_26) == g_nmi->getObjectEnumState(sO_Valve3_26, sO_Opened))
			g_nmi->playSound(SND_26_020, 0);
		else
			g_nmi->playSound(SND_26_019, 0);

		if (g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_Overfull)
			 || g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_Full)
			 || g_nmi->getObjectState(sO_Pool) == g_nmi->getObjectEnumState(sO_Pool, sO_HalfFull))
			g_nmi->playSound(SND_26_003, 0);

		if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Opened)) {
			g_nmi->stopAllSoundInstances(SND_26_018);

			chainQueue(QU_SC26_AUTOCLOSE1, 0);
		}
	}
}

void sceneHandler26_showVent() {
	if (g_vars->scene26_activeVent) {
		int id = g_vars->scene26_activeVent->_statics->_staticsId;

		if (id == ST_VNT26_UP2) {
			g_vars->scene26_activeVent->changeStatics2(ST_VNT26_RIGHT2);
		} else {
			if (id != ST_VNT26_RIGHT2)
				return;

			g_vars->scene26_activeVent->changeStatics2(ST_VNT26_UP2);
		}
		g_vars->scene26_activeVent->show1(-1, -1, -1, 0);
	}
}

void sceneHandler26_hideVent() {
	if (g_vars->scene26_activeVent)
		g_vars->scene26_activeVent->hide();
}

void sceneHandler26_animateVents(StaticANIObject *ani) {
	int qId = 0;

	switch (ani->_odelay) {
	case 0:
		if (g_nmi->getObjectState(sO_Valve1_26) == g_nmi->getObjectEnumState(sO_Valve1_26, sO_Closed)) {
			qId = QU_SC26_OPEN1;
		} else {
			qId = QU_SC26_CLOSE1;
		}
		break;

	case 1:
		if (g_nmi->getObjectState(sO_Valve2_26) == g_nmi->getObjectEnumState(sO_Valve2_26, sO_Closed)) {
			qId = QU_SC26_OPEN2;
		} else {
			qId = QU_SC26_CLOSE2;
		}
		break;

	case 2:
		if (g_nmi->getObjectState(sO_Valve3_26) == g_nmi->getObjectEnumState(sO_Valve3_26, sO_Closed)) {
			qId = QU_SC26_OPEN3;
		} else {
			qId = QU_SC26_CLOSE3;
		}
		break;

	case 3:
		if (g_nmi->getObjectState(sO_Valve4_26) == g_nmi->getObjectEnumState(sO_Valve4_26, sO_Closed)) {
			qId = QU_SC26_OPEN4;
		} else {
			qId = QU_SC26_CLOSE4;
		}
		break;

	case 4:
		if (g_nmi->getObjectState(sO_Valve5_26) == g_nmi->getObjectEnumState(sO_Valve5_26, sO_Closed)) {
			qId = QU_SC26_OPEN5;
		} else {
			qId = QU_SC26_CLOSE5;
		}
		break;

	default:
		return;
	}

	if (qId) {
		MessageQueue *mq = new MessageQueue(g_nmi->_currentScene->getMessageQueueById(qId), 0, 0);

		mq->setFlags(mq->getFlags() | 1);
		mq->chain(0);
	}
}

void sceneHandler26_clickVent(StaticANIObject *ani, ExCommand *cmd) {
	if (ani->_odelay || g_nmi->getObjectState(sO_Hatch_26) != g_nmi->getObjectEnumState(sO_Hatch_26, sO_Opened)) {
		if (g_nmi->_aniMan->isIdle() && !(g_nmi->_aniMan->_flags & 0x100)) {
			g_vars->scene26_activeVent = ani;

			int x = ani->_ox - 20;
			int y = ani->_oy + 61;

			if (abs(x - g_nmi->_aniMan->_ox) > 1 || abs(y - g_nmi->_aniMan->_oy) > 1 || g_nmi->_aniMan->_movement || g_nmi->_aniMan->_statics->_staticsId != ST_MAN_UP) {
				MessageQueue *mq = getCurrSceneSc2MotionController()->startMove(g_nmi->_aniMan, x, y, 1, ST_MAN_UP);

				if (mq) {
					ExCommand *ex = new ExCommand(0, 17, MSG_SC26_CLICKVENT, 0, 0, 0, 1, 0, 0, 0);

					ex->_excFlags |= 3;
					ex->_param = ani->_odelay;

					mq->addExCommandToEnd(ex);

					postExCommand(g_nmi->_aniMan->_id, 2, x, y, 0, -1);
				}
			} else {
				sceneHandler26_animateVents(ani);
			}
		}
	}

	cmd->_messageKind = 0;
}

int sceneHandler26(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	switch (cmd->_messageNum) {
	case MSG_SC26_UPDATEDROP:
		sceneHandler26_updateDrop();
		break;

	case MSG_SC26_SHOWCHI:
		sceneHandler26_showChi();
		break;

	case MSG_SC26_UPDATEPOOL:
		sceneHandler26_updatePool();
		break;

	case MSG_SC26_HIDECHI:
		sceneHandler26_hideChi();
		break;

	case MSG_SC26_TESTVENT:
		sceneHandler26_testVent();
		break;

	case MSG_SC26_SHOWVENT:
		sceneHandler26_showVent();
		break;

	case MSG_SC26_CLICKVENT:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObject1ById(ANI_VENT, cmd->_param);

			if (ani && ani->_id == ANI_VENT) {
				sceneHandler26_clickVent(ani, cmd);
			}
		}
		break;

	case MSG_SC26_HIDEVENT:
		sceneHandler26_hideVent();
		break;

	case 29:
		{
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);

			if (ani && ani->_id == ANI_VENT) {
				sceneHandler26_clickVent(ani, cmd);
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

		g_nmi->_behaviorManager->updateBehaviors();

		g_nmi->startSceneTrack();

		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
