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
#include "fullpipe/objects.h"
#include "fullpipe/statics.h"
#include "fullpipe/gameloader.h"
#include "fullpipe/motion.h"
#include "fullpipe/input.h"
#include "fullpipe/behavior.h"

#include "fullpipe/constants.h"
#include "fullpipe/scenes.h"
#include "fullpipe/interaction.h"

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

	scene04_bottle = 0;
	scene04_hand = 0;
	scene04_plank = 0;
	scene04_clock = 0;
	scene04_hand = 0;
	scene04_spring = 0;
	scene04_mamasha = 0;
	scene04_boot = 0;
	scene04_speaker = 0;

	scene04_ladder = 0;
	scene04_coinPut = false;
	scene04_soundPlaying = false;
	scene04_dynamicPhaseIndex = 0;

	scene04_sceneClickX = 0;
	scene04_sceneClickY = 0;

	scene04_var01 = 0;
	scene04_var02 = 0;
	scene04_var04 = 0;
	scene04_var05 = 0;
	scene04_var06 = 0;
	scene04_var07 = 0;
	scene04_var08 = 0;
	scene04_var09 = 0;
	scene04_var10 = 0;
	scene04_var11 = 0;
	scene04_var12 = 0;
	scene04_var13 = 0;
	scene04_var14 = 0;
	scene04_var15 = 0;
	scene04_var16 = 0;
	scene04_var17 = 0;
	scene04_var18 = 0;
	scene04_var19 = 0;
	scene04_var20 = 0;
	scene04_var24 = 0;

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

#if 0
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

void FullpipeEngine::initArcadeKeys(const char *varname) {
	warning("STUB: FullpipeEngine::initArcadeKeys(\"%s\")", varname);
}

} // End of namespace Fullpipe
