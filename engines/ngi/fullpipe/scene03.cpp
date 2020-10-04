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
	Inventory2 *inv = getGameLoaderInventory();
	inv->addItem(ANI_INV_EGGAPL, 1);
	inv->addItem(ANI_INV_EGGDOM, 1);
	inv->addItem(ANI_INV_EGGCOIN, 1);
	inv->addItem(ANI_INV_EGGBOOT, 1);
	inv->addItem(ANI_INV_EGGGLS, 1);
	inv->rebuildItemRects();
#endif

	g_vars->scene03_eggeater = sc->getStaticANIObject1ById(ANI_EGGEATER, -1);
	g_vars->scene03_domino = sc->getStaticANIObject1ById(ANI_DOMINO_3, -1);

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
	if (g_nmi->getObjectState(sO_EggGulperGaveCoin) == g_nmi->getObjectEnumState(sO_EggGulperGaveCoin, sO_Yes)) {
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_SLIM, QU_EGTR_SLIMSHOW, 0);
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_MID1, QU_EGTR_MD1_SHOW, 0);
		g_nmi->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_MID2, QU_EGTR_MD2_SHOW, 0);
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

	g_vars->scene03_eggeater->startAnim(MV_EGTR_FATASK, 0, -1);
}

void sceneHandler03_swallowEgg(int item) {
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

		g_nmi->setObjectState(sO_EggGulperGaveCoin, g_nmi->getObjectEnumState(sO_EggGulperGaveCoin, sO_Yes));

		scene03_setEaterState();
	}
}

void sceneHandler03_giveItem(ExCommand *ex) {
	debugC(2, kDebugSceneLogic, "scene03: giveItem");

	if (ex->_parentId == ANI_INV_EGGAPL || ex->_parentId == ANI_INV_EGGDOM ||
		ex->_parentId == ANI_INV_EGGCOIN || ex->_parentId == ANI_INV_EGGBOOT ||
		ex->_parentId == ANI_INV_EGGGLS)
		sceneHandler03_swallowEgg(ex->_parentId);
}

int sceneHandler03_swallowedEgg1State() {
	return g_vars->swallowedEgg1->_value.intValue;
}

void sceneHandler03_giveCoin(ExCommand *ex) {
	debugC(2, kDebugSceneLogic, "scene03: giveCoin");

	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (mq && mq->getCount() > 0) {
		ExCommand *ex0 = mq->getExCommandByIndex(0);
		ExCommand *ex1 = mq->getExCommandByIndex(1);

		if (sceneHandler03_swallowedEgg1State()) {
			ex0->_messageKind = 1;
			ex1->_messageKind = 1;

			getGameLoaderInventory()->removeItem(ANI_INV_COIN, 1);
		} else {
			ex0->_messageKind = 0;
			ex0->_excFlags |= 1;

			ex1->_messageKind = 0;
			ex1->_excFlags |= 1;

			g_vars->scene03_eggeater->_flags &= 0xFF7Fu;
		}
	}
}

void sceneHandler03_goLadder() {
	handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->getPictureObjectById(PIC_SC3_LADDER, 0), 0);
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

void sceneHandler03_takeEgg(ExCommand *ex) {
	debugC(2, kDebugSceneLogic, "scene03: taking egg");

	MessageQueue *mq = g_nmi->_globalMessageQueueList->getMessageQueueById(ex->_parId);

	if (mq && mq->getCount() > 0) {
		ExCommand *ex0 = mq->getExCommandByIndex(0);
		ExCommand *ex1 = mq->getExCommandByIndex(1);

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

				com->_value = g_nmi->getObjectEnumState(sO_EggGulper, sO_WantsNothing);
			}
		}
	}
}

int sceneHandler03(ExCommand *ex) {
	if (ex->_messageKind != 17 && ex->_messageNum != 33)
		debugC(3, kDebugSceneLogic, "scene03: got message: kind %d, num: %d", ex->_messageKind, ex->_messageNum);

	if (ex->_messageKind != 17) {
		if (ex->_messageKind == 57)
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
		sceneHandler03_takeEgg(ex);
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
			StaticANIObject *ani = g_nmi->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);
			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_nmi->lift_animateButton(ani);
				ex->_messageKind = 0;

				return 0;
			}

			if (g_nmi->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY) == PIC_SC3_DOMIN) {
				if (g_vars->scene03_domino)
					if (g_vars->scene03_domino->_flags & 4)
						if (g_nmi->_aniMan->isIdle())
							if (!(g_nmi->_aniMan->_flags & 0x100) && g_nmi->_msgObjectId2 != g_vars->scene03_domino->_id) {
								handleObjectInteraction(g_nmi->_aniMan, g_vars->scene03_domino, ex->_param);
								ex->_messageKind = 0;

								return 0;
							}
			}
		}
		break;

	case 33:
		{
			int res = 0;

			if (g_nmi->_aniMan2) {
				if (g_nmi->_aniMan2->_ox < g_nmi->_sceneRect.left + 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.left - 300;

				if (g_nmi->_aniMan2->_ox > g_nmi->_sceneRect.right - 200)
					g_nmi->_currentScene->_x = g_nmi->_aniMan2->_ox - g_nmi->_sceneRect.right + 300;

				res = 1;
			}

			g_nmi->_behaviorManager->updateBehaviors();

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
