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

void scene01_fixEntrance() {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");
	if (var->getSubVarAsInt("Entrance") == TrubaLeft)
		var->setSubVarAsInt("Entrance", TrubaRight);
}

void scene01_initObjects(Scene *sc, int entrance, bool eggUncracked) {
	g_vars->scene01_picSc01Osk = sc->findPictureObjectById(PIC_SC1_OSK, 0);
	g_vars->scene01_picSc01Osk->_flags &= 0xFFFB;

	g_vars->scene01_picSc01Osk2 = sc->findPictureObjectById(PIC_SC1_OSK2, 0);
	g_vars->scene01_picSc01Osk2->_flags &= 0xFFFB;

	if (eggUncracked || g_nmi->isAndroidFullPipe()) {
		PictureObject *pic = sc->findPictureObjectById(PIC_SC1_KUCHKA, 0);
		if (pic) {
			if (eggUncracked)
				pic->_flags &= ~4;
			else
				pic->_flags |= 4;
		}
	}

	if (entrance != TrubaLeft) {
		AniObject *bootAnim = sc->findAniObjectById(ANI_BOOT_1, -1);
		if (bootAnim)
			bootAnim->_flags &= ~0x04;
	}
}

static void scene01_onGuiEvent(Scene &inventory, const Common::String &id, const Common::String &event) {
	if (event != "unpress")
		return;
	if (id == "BTN_SKIP") {
		GameVar *root = g_nmi->getGameLoaderGameVar();
		root->setSubVarAsInt("TutorialDisabled", 1);
		root->setSubVarAsInt("Scene1ExitsDisabled", 0);
		g_nmi->_currentScene->setExitState(0, 1);
		inventory._guiManager->getElement(id)->hideAndDeactivate();
	} else if (id == "PIC_INV_TASK") {
		sceneNotebook_show();
	} else {
		warning("STUB: Inventory::onGuiEvent(%s, %s)", id.c_str(), event.c_str());
	}
}

void scene01_initScene(Scene *sc, int entrance) {
	scene01_initObjects(sc, entrance, g_nmi->getStateId(sO_EggCracker) == g_nmi->getStateValue(sO_EggCracker, sO_DidNotCrackEgg));
	if (g_nmi->isAndroidFullPipe()) {
		g_nmi->_inventoryScene = g_nmi->findScene(SC_INV);
		if (g_nmi->_inventoryScene) {
			g_nmi->_inventoryScene->_guiEventHandler = scene01_onGuiEvent;
			getGameLoaderInventory()->recalcItemPos();
		}
		uint32 buttonId;
		if (g_nmi->_gameProject->_objectIds.findId("ST_LBN_2N", buttonId))
			g_nmi->lift_setButton(sO_Level2, buttonId);
		GameVar *root = g_nmi->getGameLoaderGameVar();
		GameVar *visited = root->getSubVarByName("VISITED");
		if (!visited->getSubVarAsInt("SC_1")) {
			sceneChapter_show("CHAPTER_1");
			root->getSubVarByName("TASKS")->setSubVarAsInt("FIND_BOOT_SC4", 1);
			visited->setSubVarAsInt("SC_1", 1);
		}
	} else {
		g_nmi->lift_setButton(sO_Level2, ST_LBN_2N);
	}
}

bool scene01_click(Common::Point pos) {
	Scene &scene = *g_nmi->_currentScene;
	AniObject *hero = g_nmi->_aniMan;
	if (!hero || hero->_messageQueueId || !scene._motionController)
		return false;
	for (const MessageQueue *queue : *g_nmi->_globalMessageQueueList)
		if (queue->_queueId == QU_SC1_ENTER && !queue->_isFinished)
			return false;
	pos += Common::Point(scene._x, scene._y);
	PictureObject *picture = scene.findPictureObjectAtPos(pos.x, pos.y);
	AniObject *actor = scene.findAniObjectAtPos(pos.x, pos.y);
	GameObject *target = picture;
	if (actor && (!picture || actor->_z < picture->_z))
		target = actor;
	if (target) {
		uint32 upperPipe;
		if (g_nmi->_gameProject->_objectIds.findId("PIC_SC1_UTRUBA", upperPipe) && upperPipe == (uint16)target->_id)
			target = scene.findPictureObjectById(PIC_SC1_LADDER, 0);
		const int item = getGameLoaderInventory()->getCursorPictureId();
		if (target && canInteractAny(hero, target, item)) {
			if (target->_objtype == kObjTypePictureObject && g_nmi->getGameLoaderGameVar()->getSubVarAsInt("Scene1ExitsDisabled"))
				return false;
			return handleObjectInteraction(hero, target, item);
		}
	}
	return scene._motionController->startMove(hero, pos.x, pos.y, 0, 0) != nullptr;
}

int sceneHandler01(Command *cmd) {
	int res = 0;

	if (cmd->_cmd != 17)
		return 0;

	if (g_nmi->isAndroidFullPipe() && !cmd->_messageName.empty()) {
		if (cmd->_messageName == "MSG_ENABLE_BHV_SOUND") {
			if (g_nmi->_currentScene->_behaviorController)
				g_nmi->_currentScene->_behaviorController->enableAllMusic();
			return 0;
		}
		if (cmd->_messageName == "MSG_DISABLE_EXIT_MANAGER") {
			g_nmi->getGameLoaderGameVar()->setSubVarAsInt("Scene1ExitsDisabled", 1);
			g_nmi->_currentScene->setExitState(0, 0);
			Scene *inventory = g_nmi->_inventoryScene;
			if (inventory && inventory->_guiManager) {
				if (GuiElement *skip = inventory->_guiManager->getElement("BTN_SKIP"))
					skip->showAndActivate();
			}
			return 0;
		}
		if (cmd->_messageName == "MSG_DEACT_OBJ_SC1") {
			Inventory *inventory = getGameLoaderInventory();
			uint32 blackEgg, coinEgg;
			const ObjectIdList &ids = g_nmi->_gameProject->_objectIds;
			if (ids.findId("ANI_INV_EGGBLACK", blackEgg) && ids.findId("ANI_INV_EGGCOIN", coinEgg) &&
					!inventory->getCountItemsWithId(blackEgg) && !inventory->getCountItemsWithId(coinEgg))
				g_nmi->getGameLoaderGameVar()->getSubVarByName("TASKS")->setSubVarAsInt("GET_OBJECT_SC1", 0);
			return 0;
		}
		if (!cmd->_messageNum) {
			warning("STUB: %s", cmd->_messageName.c_str());
			return 0;
		}
	}

	if (cmd->_messageNum > MSG_SC1_SHOWOSK) {
		if (cmd->_messageNum == MSG_SC1_UTRUBACLICK)
			handleObjectInteraction(g_nmi->_aniMan, g_nmi->_currentScene->findPictureObjectById(PIC_SC1_LADDER, 0), 0);

		return 0;
	}

	if (cmd->_messageNum == MSG_SC1_SHOWOSK) {
		g_vars->scene01_picSc01Osk->_flags |= 4;

		g_vars->scene01_picSc01Osk->_z = 20;
		g_vars->scene01_picSc01Osk2->_z = 21;

		return 0;
	}

	if (cmd->_messageNum != 0x21) {
		if (cmd->_messageNum == MSG_SC1_SHOWOSK2) {
			g_vars->scene01_picSc01Osk2->_flags |= 4;
			g_vars->scene01_picSc01Osk2->_z = 20;
			g_vars->scene01_picSc01Osk->_z = 21;

			return 0;
		}

		return 0;
	}

	if (g_nmi->isAndroidFullPipe())
		return 0; // Scene::update drives timed behavior and audio.

	if (g_nmi->_aniMan2) {
		if (g_nmi->_aniMan2->_x < g_nmi->_sceneRect.left + 200) {
			g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.left - 300;
		}

		if (g_nmi->_aniMan2->_x > g_nmi->_sceneRect.right - 200)
			g_nmi->_currentScene->_x = g_nmi->_aniMan2->_x - g_nmi->_sceneRect.right + 300;

		res = 1;
	}
	g_nmi->_behaviorManager->update();

	g_nmi->startSceneTrack();

	return res;
}

} // End of namespace NGI
