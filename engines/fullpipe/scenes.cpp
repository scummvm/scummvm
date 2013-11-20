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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "fullpipe/fullpipe.h"

#include "fullpipe/utils.h"
#include "fullpipe/gfx.h"
#include "fullpipe/objects.h"
#include "fullpipe/statics.h"
#include "fullpipe/scene.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/sound.h"
#include "fullpipe/motion.h"
#include "fullpipe/input.h"
#include "fullpipe/messages.h"
#include "fullpipe/behavior.h"

#include "fullpipe/constants.h"
#include "fullpipe/objectnames.h"
#include "fullpipe/scenes.h"
#include "fullpipe/modal.h"
#include "fullpipe/interaction.h"
#include "fullpipe/floaters.h"

namespace Fullpipe {

Vars::Vars() {
	sceneIntro_aniin1man = 0;
	sceneIntro_needSleep = true;
	sceneIntro_needGetup = false;
	sceneIntro_skipIntro = true;
	sceneIntro_playing = false;
	sceneIntro_needBlackout = false;

	swallowedEgg1 = 0;
	swallowedEgg2 = 0;
	swallowedEgg3 = 0;

	scene01_picSc01Osk = 0;
	scene01_picSc01Osk2 = 0;

	scene02_guvTheDrawer = 0;
	scene02_boxDelay = 0;
	scene02_boxOpen = false;

	scene03_eggeater = 0;
	scene03_domino = 0;

	selector = 0;
}

static int scenes[] = {
	SC_1,  SC_2,  SC_3,  SC_4,  SC_5,  SC_6,  SC_7,  SC_8,  SC_9,  SC_10,
	SC_11, SC_12, SC_13, SC_14, SC_15, SC_16, SC_17, SC_18, SC_19, SC_20,
	SC_21, SC_22, SC_23, SC_24, SC_25, SC_26, SC_27, SC_28, SC_29, SC_30,
	SC_31, SC_32, SC_33, SC_34, SC_35, SC_36, SC_37, SC_38, SC_DBGMENU
};

int FullpipeEngine::convertScene(int scene) {
	if (!scene || scene >= SC_1)
		return scene;

	if (scene < 1 || scene > 39)
		return SC_1;

	return scenes[scene - 1];
}

bool FullpipeEngine::sceneSwitcher(EntranceInfo *entrance) {
	GameVar *sceneVar;
	Common::Point sceneDim;

	Scene *scene = accessScene(entrance->_sceneId);

	if (!scene)
		return 0;

	((PictureObject *)scene->_picObjList.front())->getDimensions(&sceneDim);
	_sceneWidth = sceneDim.x;
	_sceneHeight = sceneDim.y;

	_sceneRect.top = 0;
	_sceneRect.left = 0;
	_sceneRect.right = 799;
	_sceneRect.bottom = 599;

	scene->_x = 0;
	scene->_y = 0;

	_aniMan->setOXY(0, 0);
	_aniMan->clearFlags();
	_aniMan->_callback1 = 0;
	_aniMan->_callback2 = 0;
	_aniMan->_shadowsOn = 1;

	_scrollSpeed = 8;

	_isSaveAllowed = true;
	_updateFlag = true;
	_flgCanOpenMap = true;

	if (entrance->_sceneId == SC_DBGMENU) {
		_inventoryScene = 0;
	} else {
		_gameLoader->loadScene(SC_INV);
		getGameLoaderInventory()->rebuildItemRects();
		_inventoryScene = getGameLoaderInventory()->getScene();
	}
	if (_soundEnabled) {
		if (scene->_soundList) {
			_currSoundListCount = 2;
			_currSoundList1[0] = accessScene(SC_COMMON)->_soundList;
			_currSoundList1[1] = scene->_soundList;

			for (int i = 0; i < scene->_soundList->getCount(); i++) {
				scene->_soundList->getSoundByIndex(i)->updateVolume();
			}
		} else {
			_currSoundListCount = 1;
			_currSoundList1[0] = accessScene(SC_COMMON)->_soundList;
		}
	}

	getGameLoaderInteractionController()->sortInteractions(scene->_sceneId);
	_currentScene = scene;
	scene->addStaticANIObject(_aniMan, 1);
	_scene2 = scene;
	_aniMan->_movement = 0;
	_aniMan->_statics = _aniMan->getStaticsById(ST_MAN_EMPTY);
	_aniMan->setOXY(0, 0);

	_aniMan2 = _aniMan;
	MctlCompound *cmp = getSc2MctlCompoundBySceneId(entrance->_sceneId);
	cmp->initMovGraph2();
	cmp->addObject(_aniMan);
	cmp->setEnabled();
	getGameLoaderInteractionController()->enableFlag24();
	setInputDisabled(0);

	scene->setPictureObjectsFlag4();

	for (PtrList::iterator s = scene->_staticANIObjectList1.begin(); s != scene->_staticANIObjectList1.end(); ++s) {
		StaticANIObject *o = (StaticANIObject *)*s;
		o->setFlags(o->_flags & 0xFE7F);
	}

	PictureObject *p = accessScene(SC_INV)->getPictureObjectById(PIC_INV_MENU, 0);
	p->setFlags(p->_flags & 0xFFFB);

	removeMessageHandler(2, -1);
	_updateScreenCallback = 0;

	switch (entrance->_sceneId) {
	case SC_INTRO1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_INTRO1");
		scene->preloadMovements(sceneVar);
		sceneIntro_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_INTRO1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandlerIntro, 2);
		_updateCursorCallback = sceneIntro_updateCursor;
		break;

	case SC_1:
		scene01_fixEntrance();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_1");
		scene->preloadMovements(sceneVar);
		scene01_initScene(scene, entrance->_field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler01, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_2:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_2");
		scene->preloadMovements(sceneVar);
		scene02_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_2");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler02, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_3:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_3");
		scene->preloadMovements(sceneVar);
		scene03_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_3");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler03, 2);
		scene03_setEaterState();
		_updateCursorCallback = scene03_updateCursor;
		break;

#if 0
	case SC_4:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_4");
		scene->preloadMovements(sceneVar);
		scene04_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_4");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler04, 2, 2);
		_updateCursorCallback = scene04_updateCursor;
		break;

	case SC_5:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_5");
		scene->preloadMovements(sceneVar);
		scene05_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_5");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler05, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_6:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_6");
		scene->preloadMovements(sceneVar);
		scene06_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_6");
		setSceneMusicParameters(sceneVar);
		sub_415300();
		insertMessageHandler(sceneHandler06, 2, 2);
		_updateCursorCallback = scene06_updateCursor;
		break;

	case SC_7:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_7");
		scene->preloadMovements(sceneVar);
		scene07_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_7");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler07, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_8:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_8");
		scene->preloadMovements(sceneVar);
		scene08_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_8");
		setSceneMusicParameters(sceneVar);
		sub_416890();
		addMessageHandler(sceneHandler08, 2);
		_updateCursorCallback = scene08_updateCursor;
		break;

	case SC_9:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_9");
		scene->preloadMovements(sceneVar);
		scene09_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_9");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler09, 2, 2);
		_updateCursorCallback = scene09_updateCursor;
		break;

	case SC_10:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_10");
		scene->preloadMovements(sceneVar);
		scene10_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_10");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler10, 2, 2);
		_updateCursorCallback = scene10_updateCursor;
		break;

	case SC_11:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_11");
		scene->preloadMovements(sceneVar);
		scene11_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_11");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler11, 2, 2);
		scene11_sub_41A980();
		_updateCursorCallback = scene11_updateCursor;
		break;

	case SC_12:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_12");
		scene->preloadMovements(sceneVar);
		scene12_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_12");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler12, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_13:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_13");
		scene->preloadMovements(sceneVar);
		scene13_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_13");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler13, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_14:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_14");
		scene->preloadMovements(sceneVar);
		scene14_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_14");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler14, 2, 2);
		scene14_sub_41D2B0();
		_updateCursorCallback = scene14_updateCursor;
		break;

	case SC_15:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_15");
		scene->preloadMovements(sceneVar);
		scene15_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_15");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler15, 2, 2);
		_updateCursorCallback = scene15_updateCursor;
		break;

	case SC_16:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_16");
		scene->preloadMovements(sceneVar);
		scene16_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_16");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler16, 2);
		_updateCursorCallback = scene16_updateCursor;
		break;

	case SC_17:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_17");
		scene->preloadMovements(sceneVar);
		scene17_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_17");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler17, 2);
		scene17_sub_41F060();
		_updateCursorCallback = scene17_updateCursor;
		break;

	case SC_18:
		sub_40E1B0();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_18");
		scene->preloadMovements(sceneVar);
		sub_4062D0();
		if (dword_476C38)
			scene18_initScene1(scene);
		else
			scene18_initScene2(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_18");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler18, 2, 2);
		_updateCursorCallback = scene18_updateCursor;
		break;

	case SC_19:
		if (!g_scene3) {
			g_scene3 = accessScene(SC_18);
			getGameLoader()->loadScene(SC_18);
			scene18_initScene2(g_scene3);
			sub_40C5F0();
			scene19_sub_420B10(g_scene3, entrance->field_4);
			dword_476C38 = 1;
		}
		sub_40C650();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_19");
		scene->preloadMovements(sceneVar);
		sub_4062D0();
		if (dword_476C38)
			scene18_initScene1(scene);
		else
			scene19_initScene2();
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_19");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler19, 2);
		scene19_sub_4211D0(scene);
		_updateCursorCallback = scene19_updateCursor;
		break;

	case SC_20:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_20");
		scene->preloadMovements(sceneVar);
		scene20_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_20");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler20, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_21:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_21");
		scene->preloadMovements(sceneVar);
		scene21_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_21");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler21, 2, 2);
		_updateCursorCallback = scene21_updateCursor;
		break;

	case SC_22:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_22");
		scene->preloadMovements(sceneVar);
		scene22_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_22");
		setSceneMusicParameters(sceneVar);
		scene22_sub_4228A0();
		insertMessageHandler(sceneHandler22, 2, 2);
		_updateCursorCallback = scene22_updateCursor;
		break;

	case SC_23:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_23");
		scene->preloadMovements(sceneVar);
		scene23_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_23");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler23, 2, 2);
		scene23_sub_423B00();
		_updateCursorCallback = scene23_updateCursor;
		break;

	case SC_24:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_24");
		scene->preloadMovements(sceneVar);
		scene24_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_24");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler24, 2);
		scene24_sub_423DD0();
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_25:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_25");
		scene->preloadMovements(sceneVar);
		scene25_initScene(scene, entrance->field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_25");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler25, 2);
		scene25_sub_4253B0(scene, entrance->field_4);
		_updateCursorCallback = scene25_updateCursor;
		break;

	case SC_26:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_26");
		scene->preloadMovements(sceneVar);
		scene26_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_26");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler26, 2, 2);
		scene26_sub_426140(scene);
		_updateCursorCallback = scene26_updateCursor;
		break;

	case SC_27:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_27");
		scene->preloadMovements(sceneVar);
		scene27_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_27");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler27, 2);
		_updateCursorCallback = scene27_updateCursor;
		break;

	case SC_28:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_28");
		scene->preloadMovements(sceneVar);
		scene28_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_28");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler28, 2, 2);
		_updateCursorCallback = scene28_updateCursor;
		break;

	case SC_29:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_29");
		scene->preloadMovements(sceneVar);
		scene29_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_29");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler29, 2);
		_updateCursorCallback = scene29_updateCursor;
		break;

	case SC_30:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_30");
		scene->preloadMovements(sceneVar);
		scene30_initScene(scene, entrance->field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_30");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler30, 2);
		_updateCursorCallback = scene30_updateCursor;
		break;

	case SC_31:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_31");
		scene->preloadMovements(sceneVar);
		scene31_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_31");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler31, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_32:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_32");
		scene->preloadMovements(sceneVar);
		scene32_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_32");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler32, 2, 2);
		scene32_sub_42C5C0();
		_updateCursorCallback = scene32_updateCursor;
		break;

	case SC_33:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_33");
		scene->preloadMovements(sceneVar);
		scene33_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_33");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler33, 2, 2);
		scene33_sub_42CEF0();
		_updateCursorCallback = scene33_updateCursor;
		break;

	case SC_34:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_34");
		scene->preloadMovements(sceneVar);
		scene34_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_34");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler34, 2, 2);
		scene34_sub_42DEE0();
		_updateCursorCallback = scene34_updateCursor;
		break;

	case SC_35:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_35");
		scene->preloadMovements(sceneVar);
		scene35_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_35");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler35, 2, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_36:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_36");
		scene->preloadMovements(sceneVar);
		scene36_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_36");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler36, 2);
		_updateCursorCallback = scene36_updateCursor;
		break;

	case SC_37:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_37");
		scene->preloadMovements(sceneVar);
		scene37_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_37");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler37, 2, 2);
		_updateCursorCallback = scene37_updateCursor;
		break;

	case SC_38:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_38");
		scene->preloadMovements(sceneVar);
		scene38_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_38");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler38, 2);
		_updateCursorCallback = defaultUpdateCursor;
		break;

	case SC_FINAL1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_FINAL1");
		scene->preloadMovements(sceneVar);
		sceneFinal1_initScene();
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_FINAL1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandlerFinal1, 2);
		_updateCursorCallback = sceneFinal1_updateCursor;
		break;
#endif

	case SC_DBGMENU:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_DBGMENU");
		scene->preloadMovements(sceneVar);
		sceneDbgMenu_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_DBGMENU");
		addMessageHandler(sceneHandlerDbgMenu, 2);
		break;

	default:
		_behaviorManager->initBehavior(0, 0);
		break;
	}

	return true;
}

int defaultUpdateCursor() {
	g_fullpipe->updateCursorCommon();

	return g_fullpipe->_cursorId;
}

void FullpipeEngine::setSwallowedEggsState() {
	GameVar *v = _gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = v->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = v->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = v->getSubVarByName(sO_Egg3);

	g_vars->swallowedEgg1->_value.intValue = 0;
	g_vars->swallowedEgg2->_value.intValue = 0;
	g_vars->swallowedEgg3->_value.intValue = 0;
}

void scene03_initScene(Scene *sc) {
	g_vars->scene03_eggeater = sc->getStaticANIObject1ById(ANI_EGGEATER, -1);
	g_vars->scene03_domino = sc->getStaticANIObject1ById(ANI_DOMINO_3, -1);

	GameVar *v = g_fullpipe->_gameLoader->_gameVar->getSubVarByName("OBJSTATES")->getSubVarByName(sO_GulpedEggs);

	g_vars->swallowedEgg1 = v->getSubVarByName(sO_Egg1);
	g_vars->swallowedEgg2 = v->getSubVarByName(sO_Egg2);
	g_vars->swallowedEgg3 = v->getSubVarByName(sO_Egg3);

	g_fullpipe->lift_setButton(sO_Level2, ST_LBN_2N);

	g_fullpipe->lift_sub5(sc, QU_SC3_ENTERLIFT, QU_SC3_EXITLIFT);
}

void scene03_setEaterState() {
	if (g_fullpipe->getObjectState(sO_EggGulperGaveCoin) == g_fullpipe->getObjectEnumState(sO_EggGulperGaveCoin, sO_Yes)) {
		g_fullpipe->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_SLIM, QU_EGTR_SLIMSHOW, 0);
		g_fullpipe->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_MID1, QU_EGTR_MD1_SHOW, 0);
		g_fullpipe->_behaviorManager->setBehaviorEnabled(g_vars->scene03_eggeater, ST_EGTR_MID2, QU_EGTR_MD2_SHOW, 0);
	}
}

int scene03_updateCursor() {
	g_fullpipe->updateCursorCommon();

	if (g_fullpipe->_cursorId == PIC_CSR_DEFAULT && g_fullpipe->_objectIdAtCursor == PIC_SC3_DOMIN && g_vars->scene03_domino) {
		if (g_vars->scene03_domino->_flags & 4)
			g_fullpipe->_cursorId = PIC_CSR_ITN;
	}

	return g_fullpipe->_cursorId;
}

void sceneHandler03_eaterFat() {
	g_vars->scene03_eggeater->_flags &= 0xFF7F;

	g_vars->scene03_eggeater->startAnim(MV_EGTR_FATASK, 0, -1);
}

void sceneHandler03_swallowEgg(int item) {
	if (!g_vars->swallowedEgg1->_value.intValue) {
		g_vars->swallowedEgg1->_value.intValue = item;
	} else if (!g_vars->swallowedEgg2->_value.intValue) {
		g_vars->swallowedEgg2->_value.intValue = item;
	} else if (!g_vars->swallowedEgg3->_value.intValue) {
		g_vars->swallowedEgg3->_value.intValue = item;

		g_fullpipe->setObjectState(sO_EggGulperGaveCoin, g_fullpipe->getObjectEnumState(sO_EggGulperGaveCoin, sO_Yes));

		scene03_setEaterState();
	}
}

void sceneHandler03_giveItem(ExCommand *ex) {
	if (ex->_parentId == ANI_INV_EGGAPL || ex->_parentId == ANI_INV_EGGDOM ||
		ex->_parentId == ANI_INV_EGGCOIN || ex->_parentId == ANI_INV_EGGBOOT ||
		ex->_parentId == ANI_INV_EGGGLS)
		sceneHandler03_swallowEgg(ex->_parentId);
}

int sceneHandler03_swallowedEgg1State() {
	return g_vars->swallowedEgg1->_value.intValue;
}

void sceneHandler03_giveCoin(ExCommand *ex) {
	MessageQueue *mq = g_fullpipe->_globalMessageQueueList->getMessageQueueById(ex->_parId);

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
	handleObjectInteraction(g_fullpipe->_aniMan, g_fullpipe->_currentScene->getPictureObjectById(PIC_SC3_LADDER, 0), 0);
}

void sceneHandler03_pushEggStack() {
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
	g_vars->scene03_eggeater->_flags &= 0xFF7F;

	g_vars->scene03_eggeater->show1(-1, -1, -1, 0);
}

void sceneHandler03_takeEgg(ExCommand *ex) {
	MessageQueue *mq = g_fullpipe->_globalMessageQueueList->getMessageQueueById(ex->_parId);

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
				ObjstateCommand *com = (ObjstateCommand *)ex1;

				com->_value = g_fullpipe->getObjectEnumState(sO_EggGulper, sO_WantsNothing);
			}
		}
	}
}

int sceneHandler03(ExCommand *ex) {
	if (ex->_messageKind != 17) {
		if (ex->_messageKind == 57)
			sceneHandler03_giveItem(ex);
		return 0;
	}

	switch (ex->_messageNum) {
	case MSG_LIFT_EXITLIFT:
		g_fullpipe->lift_exitSeq(ex);
		break;

	case MSG_LIFT_CLOSEDOOR:
		g_fullpipe->lift_closedoorSeq();
		break;

	case MSG_SC3_ONTAKECOIN:
		sceneHandler03_eaterFat();
		break;

	case MSG_LIFT_STARTEXITQUEUE:
		g_fullpipe->lift_startExitQueue();
		break;

	case MSG_SC3_RELEASEEGG:
		sceneHandler03_releaseEgg();
		break;

	case MSG_LIFT_CLICKBUTTON:
		g_fullpipe->lift_animation3();
		break;

	case MSG_SC3_HIDEDOMINO:
		g_vars->scene03_domino->_flags &= 0xFFFB;
		break;

	case MSG_SC3_TAKEEGG:
		sceneHandler03_takeEgg(ex);
		break;

	case MSG_LIFT_GO:
		g_fullpipe->lift_goAnimation();
		break;

	case MSG_SC3_UTRUBACLICK:
		sceneHandler03_goLadder();
		break;

	case MSG_SC3_TESTFAT:
		sceneHandler03_giveCoin(ex);
		break;

	case 64:
		g_fullpipe->lift_sub05(ex);
		break;

	case 93:
		{
			StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObjectAtPos(ex->_sceneClickX, ex->_sceneClickY);
			if (ani && ani->_id == ANI_LIFTBUTTON) {
				g_fullpipe->lift_sub1(ani);
				ex->_messageKind = 0;

				return 0;
			}

			if (g_fullpipe->_currentScene->getPictureObjectIdAtPos(ex->_sceneClickX, ex->_sceneClickY) == PIC_SC3_DOMIN) {
				if (g_vars->scene03_domino)
					if (g_vars->scene03_domino->_flags & 4)
						if (g_fullpipe->_aniMan->isIdle())
							if (!(g_fullpipe->_aniMan->_flags & 0x100) && g_fullpipe->_msgObjectId2 != g_vars->scene03_domino->_id) {
								handleObjectInteraction(g_fullpipe->_aniMan, g_vars->scene03_domino, ex->_keyCode);
								ex->_messageKind = 0;

								return 0;
							}
			}

			break;
		}

	case 97:
		{
			int res = 0;

			if (g_fullpipe->_aniMan2) {
				if (g_fullpipe->_aniMan2->_ox < g_fullpipe->_sceneRect.left + 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.left - 300;

				if (g_fullpipe->_aniMan2->_ox > g_fullpipe->_sceneRect.right - 200)
					g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.right + 300;

				res = 1;
			}

			g_fullpipe->_behaviorManager->updateBehaviors();

			g_fullpipe->startSceneTrack();

			return res;
		}
	}

	return 0;
}

void sceneDbgMenu_initScene(Scene *sc) {
	g_vars->selector = sc->getPictureObjectById(PIC_SCD_SEL, 0);
	getGameLoaderInteractionController()->disableFlag24();
	setInputDisabled(0);
}

GameObject *sceneHandlerDbgMenu_getObjectAtXY(int x, int y) {
	if (g_fullpipe->_currentScene)
		for (uint i = 0; i < g_fullpipe->_currentScene->_picObjList.size(); i++) {
			PictureObject *pic = (PictureObject *)g_fullpipe->_currentScene->_picObjList[i];

			if (x >= pic->_ox && y >= pic->_oy) {
				Common::Point point;

				pic->getDimensions(&point);

				if (x <= pic->_ox + point.x && y <= pic->_oy + point.y && pic != g_vars->selector)
					return pic;
			}
		}

	return 0;
}

int sceneHandlerDbgMenu(ExCommand *ex) {
	if (ex->_messageKind != 17)
		return 0;

	int mx = g_fullpipe->_mouseScreenPos.x + g_fullpipe->_sceneRect.left;
	int my = g_fullpipe->_mouseScreenPos.y + g_fullpipe->_sceneRect.top;

	if (ex->_messageNum == 29) {
		GameObject *obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(0, obj, -3) ) {
			getGameLoaderInteractionController()->enableFlag24();
			handleObjectInteraction(0, obj, 0);
		}
		return 0;
	}
	if (ex->_messageNum != 33) {
		if (ex->_messageNum == MSG_RESTARTGAME) {
			g_fullpipe->_needRestart = true;
			return 0;
		}
		return 0;
	}

	g_fullpipe->_cursorId = PIC_CSR_DEFAULT;
	GameObject *obj = g_fullpipe->_currentScene->getStaticANIObjectAtPos(mx, my);
	if (obj) {
		if (canInteractAny(0, obj, -3)) {
			g_fullpipe->_cursorId = PIC_CSR_DEFAULT;
			g_fullpipe->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
	} else {
		obj = sceneHandlerDbgMenu_getObjectAtXY(mx, my);
		if (obj && canInteractAny(0, obj, -3) ) {
			g_vars->selector->_flags |= 4;
			g_vars->selector->setOXY(obj->_ox, obj->_oy);
			g_fullpipe->_cursorId = PIC_CSR_DEFAULT;
			g_fullpipe->setCursor(PIC_CSR_DEFAULT);
			return 0;
		}
		g_vars->selector->_flags &= 0xFFFB;
	}
	g_fullpipe->setCursor(g_fullpipe->_cursorId);

	return 0;
}

} // End of namespace Fullpipe
