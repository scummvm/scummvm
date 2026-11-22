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

void NGIEngine::setSwallowedEggsState() {
	GameVar *v = _gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = v->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = v->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = v->getSubVarByName(sO_Egg3);

	g_vars->swallowedEgg1->_value.intValue = 0;
	g_vars->swallowedEgg2->_value.intValue = 0;
	g_vars->swallowedEgg3->_value.intValue = 0;
}

void scene03_initScene(Scene *sc) {
	debugC(1, kDebugSceneLogic, "scene03_initScene()");

#if 0
	Inventory *inv = getGameLoaderInventory();
	inv->addItem(ANI_INV_EGGAPL, 1);
	inv->addItem(ANI_INV_EGGDOM, 1);
	inv->addItem(ANI_INV_EGGCOIN, 1);
	inv->addItem(ANI_INV_EGGBOOT, 1);
	inv->addItem(ANI_INV_EGGGLS, 1);
	inv->recalcItemPos();
#endif

	g_vars->scene03_eggeater = sc->findAniObjectById(ANI_EGGEATER, -1);
	g_vars->scene03_domino = sc->findAniObjectById(ANI_DOMINO_3, -1);

	GameVar *v = g_nmi->_gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = v->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = v->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = v->getSubVarByName(sO_Egg3);

	g_nmi->lift_setButton(sO_Level2, ST_LBN_2N);

	g_nmi->lift_init(sc, QU_SC3_ENTERLIFT, QU_SC3_EXITLIFT);

	debugC(2, kDebugSceneLogic, "scene03: egg1: %d egg2: %d egg3: %d", g_vars->swallowedEgg1->_value.intValue,
		g_vars->swallowedEgg2->_value.intValue, g_vars->swallowedEgg3->_value.intValue);
}

void scene03_setEaterState() {
	if (g_nmi->getStateId(sO_EggGulperGaveCoin) == g_nmi->getStateValue(sO_EggGulperGaveCoin, sO_Yes)) {
		g_nmi->_behaviorManager->enableAni(g_vars->scene03_eggeater, ST_EGTR_SLIM, QU_EGTR_SLIMSHOW, 0);
		g_nmi->_behaviorManager->enableAni(g_vars->scene03_eggeater, ST_EGTR_MID1, QU_EGTR_MD1_SHOW, 0);
		g_nmi->_behaviorManager->enableAni(g_vars->scene03_eggeater, ST_EGTR_MID2, QU_EGTR_MD2_SHOW, 0);
	}
}

int scene03_updateCursor() {
	g_nmi->updateCursorCommon();

	if (g_nmi->_cursorId == PIC_CSR_DEFAULT && g_nmi->_objectIdAtCursor == PIC_SC3_DOMIN && g_vars->scene03_domino) {
		if (g_vars->scene03_domino->_flags & 4)
			g_nmi->_cursorId = PIC_CSR_ITN;
	}

	return g_nmi->_cursorId;
}

void sceneHandler03_eaterFat() {
	debugC(2, kDebugSceneLogic, "scene03: eaterFat");

	g_vars->scene03_eggeater->_flags &= 0xFF7F;

	g_vars->scene03_eggeater->startMovement(MV_EGTR_FATASK, 0, -1);
}

void sceneHandler03_addLastEaten(int item) {
	debugC(2, kDebugSceneLogic, "scene03: swallowEgg");

	if (!g_vars->swallowedEgg1->_value.intValue) {
		g_vars->swallowedEgg1->_value.intValue = item;
		debugC(2, kDebugSceneLogic, "scene03: setting egg1: %d", g_vars->swallowedEgg1->_value.intValue);
	} else if (!g_vars->swallowedEgg2->_value.intValue) {
		g_vars->swallowedEgg2->_value.intValue = item;
		debugC(2, kDebugSceneLogic, "scene03: setting egg2: %d", g_vars->swallowedEgg2->_value.intValue);
	} else if (!g_vars->swallowedEgg3->_value.intValue) {
		g_vars->swallowedEgg3->_value.intValue = item;
		debugC(2, kDebugSceneLogic, "scene03: setting egg3: %d", g_vars->swallowedEgg3->_value.intValue);

		g_nmi->setState(sO_EggGulperGaveCoin, g_nmi->getStateValue(sO_EggGulperGaveCoin, sO_Yes));

		scene03_setEaterState();
	}
}

void sceneHandler03_giveItem(Command *ex) {
	debugC(2, kDebugSceneLogic, "scene03: giveItem");

	if (ex->_parentId == ANI_INV_EGGAPL || ex->_parentId == ANI_INV_EGGDOM ||
		ex->_parentId == ANI_INV_EGGCOIN || ex->_parentId == ANI_INV_EGGBOOT ||
		ex->_parentId == ANI_INV_EGGGLS)
		sceneHandler03_addLastEaten(ex->_parentId);
}

int sceneHandler03_swallowedEgg1State() {
	return g_vars->swallowedEgg1->_value.intValue;
}

void sceneHandler03_giveCoin(Command *ex) {
	debugC(2, kDebugSceneLogic, "scene03: giveCoin");

	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (mq && mq->getCount() > 0) {
		Command *ex0 = mq->getCommandByIndex(0);
		Command *ex1 = mq->getCommandByIndex(1);

		if (sceneHandler03_swallowedEgg1State()) {
			ex0->_cmd = 1;
			ex1->_cmd = 1;

			getGameLoaderInventory()->removeItem(ANI_INV_COIN, 1);
		} else {
			ex0->_cmd = 0;
			ex0->_excFlags |= 1;

			ex1->_cmd = 0;
			ex1->_excFlags |= 1;

			g_vars->scene03_eggeater->_flags &= 0xFF7Fu;
		}
	}
}

void sceneHandler03_goLadder() {
	handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->findPictureObjectById(PIC_SC3_LADDER, 0), 0);
}

void sceneHandler03_pushEggStack() {
	debugC(2, kDebugSceneLogic, "scene03: pushEggStack");

	g_vars->swallowedEgg1->_value.intValue = g_vars->swallowedEgg2->_value.intValue;
	g_vars->swallowedEgg2->_value.intValue = g_vars->swallowedEgg3->_value.intValue;
	g_vars->swallowedEgg3->_value.intValue = 0;

	if (g_vars->swallowedEgg2->_value.intValue == ANI_INV_EGGBOOT
		 && g_vars->swallowedEgg1->_value.intValue == ANI_INV_EGGAPL) {
		g_vars->swallowedEgg1->_value.intValue = ANI_INV_EGGBOOT;
		g_vars->swallowedEgg2->_value.intValue = ANI_INV_EGGAPL;
  }
}

void sceneHandler03_releaseEgg() {
	debugC(2, kDebugSceneLogic, "scene03: releaseEgg");

	g_vars->scene03_eggeater->_flags &= 0xFF7F;

	g_vars->scene03_eggeater->show1(-1, -1, -1, 0);
}

void sceneHandler03_doTakeEgg(Command *ex) {
	debugC(2, kDebugSceneLogic, "scene03: taking egg");

	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (mq && mq->getCount() > 0) {
		Command *ex0 = mq->getCommandByIndex(0);
		Command *ex1 = mq->getCommandByIndex(1);

		int egg1 = sceneHandler03_swallowedEgg1State();

		if (egg1 && ex0) {
			ex0->_parentId = egg1;
			sceneHandler03_pushEggStack();
		}

		if ( g_vars->swallowedEgg1->_value.intValue == ANI_INV_EGGAPL
			 && !g_vars->swallowedEgg2->_value.intValue
			 && !g_vars->swallowedEgg3->_value.intValue
			 && ex1) {

			if (ex1->_objtype == kObjTypeObjstateCommand) {
				ObjstateCommand *com = static_cast<ObjstateCommand *>(ex1);

				com->_value = g_nmi->getStateValue(sO_EggGulper, sO_WantsNothing);
			}
		}
	}
}

int sceneHandler03(Command *ex) {
	if (ex->_cmd != 17 && ex->_messageNum != 33)
		debugC(3, kDebugSceneLogic, "scene03: got message: kind %d, num: %d", ex->_cmd, ex->_messageNum);

	if (ex->_cmd != 17) {
		if (ex->_cmd == 57)
			sceneHandler03_giveItem(ex);
		return 0;
	}

	switch (ex->_messageNum) {
	case MSG_LIFT_EXITLIFT:
		g_nmi->lift_exitSeq(ex);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_nmi->lift_closedoorSeq();
		break;

	case MSG_SC3_ONTAKECOIN:
		sceneHandler03_eaterFat();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_nmi->lift_startExitQueue();
		break;

	case MSG_SC3_RELEASEEGG:
		sceneHandler03_releaseEgg();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_nmi->lift_clickButton();
		break;

	case MSG_SC3_HIDEDOMINO:
		g_vars->scene03_domino->_flags &= 0xFFFB;
		break;

	case MSG_SC3_TAKEEGG:
		sceneHandler03_doTakeEgg(ex);
		break;

	case MSG_LIFT_GO:
		g_nmi->lift_goAnimation();
		break;

	case MSG_SC3_UTRUBACLICK:
		sceneHandler03_goLadder();
		break;

	case MSG_SC3_TESTFAT:
		sceneHandler03_giveCoin(ex);
		break;

	case 64:
		g_nmi->lift_hoverButton(ex);
		break;

	case 29:
		{
			AniObject *ani = g_nmi->_currentScene->findAniObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);
			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);
				ex->_cmd = 0;

				return 0;
			}

			if (g_nmi->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY) == PIC_SC3_DOMIN) {
				if (g_vars->scene03_domino)
					if (g_vars->scene03_domino->_flags & 4)
						if (g_nmi->_aniMan->isIdle())
							if (!(g_nmi->_aniMan->_flags & 0x100) && g_nmi->_msgObjectId2 != g_vars->scene03_domino->_id) {
								handleObjectInteraction(g_nmi->_aniMan, g_vars->scene03_domino, ex->_param);
								ex->_cmd = 0;

								return 0;
							}
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan2->_x < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.left - 300;

				if (g_nmi->_aniMan2->_x > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.right + 300;

				res = 1;
			}

			g_nmi->_behaviorManager->update();

			g_nmi->startSceneTrack();

			return res;
		}
		break;

	default:
		break;
	}

	return 0;
}

} // End of namespace NGI
