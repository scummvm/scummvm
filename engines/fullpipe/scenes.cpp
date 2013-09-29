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

namespace Fullpipe {

int defaultUpdateCursor();
void setElevatorButton(const char *name, int state);

int sceneIntro_updateCursor();
void sceneIntro_initScene(Scene *sc);
int sceneHandlerIntro(ExCommand *cmd);

void scene01_fixEntrance();
void scene01_initScene(Scene *sc, int entrance);
int sceneHandler01(ExCommand *cmd);

void sceneDbgMenu_initScene(Scene *sc);
int sceneHandlerDbgMenu(ExCommand *cmd);

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

	selector = 0;
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

#if 0
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
		j_Scene_sc03_sub_40F160(scene);
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

void setElevatorButton(const char *name, int state) {
	GameVar *var = g_fullpipe->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName(sO_LiftButtons);

	if (var)
		var->setSubVarAsInt(name, state);
}

void global_messageHandler_KickStucco() {
	warning("STUB: global_messageHandler_KickStucco()");
}

void global_messageHandler_KickMetal() {
	warning("STUB: global_messageHandler_KickMetal()");
}

int global_messageHandler1(ExCommand *cmd) {
	debug(0, "global_messageHandler1: %d %d", cmd->_messageKind, cmd->_messageNum);

	if (cmd->_excFlags & 0x10000) {
		if (cmd->_messageNum == MV_MAN_TOLADDER)
			cmd->_messageNum = MV_MAN_TOLADDER2;
		if (cmd->_messageNum == MV_MAN_STARTLADDER)
			cmd->_messageNum = MV_MAN_STARTLADDER2;
		if (cmd->_messageNum == MV_MAN_GOLADDER)
			cmd->_messageNum = MV_MAN_GOLADDER2;
		if (cmd->_messageNum == MV_MAN_STOPLADDER)
			cmd->_messageNum = MV_MAN_STOPLADDER2;
	}

	if (g_fullpipe->_inputDisabled) {
		if (cmd->_messageKind == 17) {
			switch (cmd->_messageNum) {
			case 29:
			case 30:
			case 36:
			case 106:
				cmd->_messageKind = 0;
				break;
			default:
				break;
			}
		}
	} else if (cmd->_messageKind == 17) {
		switch (cmd->_messageNum) {
		case MSG_MANSHADOWSON:
			g_fullpipe->_aniMan->_shadowsOn = 1;
			break;
		case MSG_HMRKICK_STUCCO:
			global_messageHandler_KickStucco();
			break;
		case MSG_MANSHADOWSOFF:
			g_fullpipe->_aniMan->_shadowsOn = 0;
			break;
		case MSG_DISABLESAVES:
			g_fullpipe->disableSaves(cmd);
			break;
		case MSG_ENABLESAVES:
			g_fullpipe->enableSaves();
			break;
		case MSG_HMRKICK_METAL:
			global_messageHandler_KickMetal();
			break;
		case 29: // left mouse
			if (g_fullpipe->_inventoryScene) {
				if (getGameLoaderInventory()->handleLeftClick(cmd))
					cmd->_messageKind = 0;
			}
			break;
		case 107: // right mouse
			if (getGameLoaderInventory()->getSelectedItemId()) {
				getGameLoaderInventory()->unselectItem(0);
				cmd->_messageKind = 0;
			}
			break;
		case 36: // keydown
			g_fullpipe->defHandleKeyDown(cmd->_keyCode);

			switch (cmd->_keyCode) {
			case '\x1B': // ESC
				if (g_fullpipe->_currentScene) {
					getGameLoaderInventory()->unselectItem(0);
					g_fullpipe->openMainMenu();
					cmd->_messageKind = 0;
				}
				break;
			case 't':
				g_fullpipe->stopAllSounds();
				cmd->_messageKind = 0;
				break;
			case 'u':
				g_fullpipe->toggleMute();
				cmd->_messageKind = 0;
				break;
			case ' ':
				if (getGameLoaderInventory()->getIsLocked()) {
					if (getGameLoaderInventory()->getIsInventoryOut()) {
						getGameLoaderInventory()->setIsLocked(0);
					}
				} else {
					getGameLoaderInventory()->slideOut();
					getGameLoaderInventory()->setIsLocked(1);
				}
				break;
			case '\t':
				if (g_fullpipe->_flgCanOpenMap)
					g_fullpipe->openMap();
				cmd->_messageKind = 0;
				break;
			case 'p':
				if (g_fullpipe->_flgCanOpenMap)
					g_fullpipe->openHelp();
				cmd->_messageKind = 0;
				break;
			default:
				break;
			}
			break;
		case 33:
			if (!g_fullpipe->_inventoryScene)
				break;

			int invItem;

			if (g_fullpipe->_updateFlag && (invItem = g_fullpipe->_inventory->getHoveredItem(&g_fullpipe->_mouseScreenPos))) {
				g_fullpipe->_cursorId = PIC_CSR_ITN;
				if (!g_fullpipe->_currSelectedInventoryItemId && !g_fullpipe->_aniMan->_movement && 
					!(g_fullpipe->_aniMan->_flags & 0x100) && g_fullpipe->_aniMan->isIdle()) {
					int st = g_fullpipe->_aniMan->_statics->_staticsId;
					ExCommand *newex = 0;

					if (st == ST_MAN_RIGHT) {
						newex = new ExCommand(g_fullpipe->_aniMan->_id, 1, rMV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					} else if (st == (0x4000 | ST_MAN_RIGHT)) {
						newex = new ExCommand(g_fullpipe->_aniMan->_id, 1, MV_MAN_LOOKUP, 0, 0, 0, 1, 0, 0, 0);
					}

					if (newex) {
						newex->_keyCode = g_fullpipe->_aniMan->_okeyCode;
						newex->_excFlags |= 3;
						newex->postMessage();
					}
				}

				if (g_fullpipe->_currSelectedInventoryItemId != invItem)
					g_fullpipe->playSound(SND_CMN_070, 0);

				g_fullpipe->_currSelectedInventoryItemId = invItem;
				g_fullpipe->setCursor(g_fullpipe->_cursorId);
				break;
			}
			if (g_fullpipe->_updateCursorCallback)
				g_fullpipe->_updateCursorCallback();

			g_fullpipe->_currSelectedInventoryItemId = 0;
			g_fullpipe->setCursor(g_fullpipe->_cursorId);
			break;
		case 65: // open map
			if (cmd->_field_2C == 11 && cmd->_field_14 == ANI_INV_MAP && g_fullpipe->_flgCanOpenMap)
				g_fullpipe->openMap();
			break;
		default:
			break;
		}
	}

	if (cmd->_messageKind == 56) {
		getGameLoaderInventory()->rebuildItemRects();

		ExCommand *newex = new ExCommand(0, 35, SND_CMN_031, 0, 0, 0, 1, 0, 0, 0);

		newex->_field_14 = 1;
		newex->_excFlags |= 3;
		newex->postMessage();

		return 1;
	} else if (cmd->_messageKind == 57) {
		getGameLoaderInventory()->rebuildItemRects();

		return 1;
	}

	return 0;
}

void staticANIObjectCallback(int *arg) {
	(*arg)--;
}

int global_messageHandler2(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	int res = 0;
	StaticANIObject *ani;

	switch (cmd->_messageNum) {
	case 0x44c8:
		error("0x44c8");
		// Unk3_sub_4477A0(&unk3, _parentId, _field_14 != 0);
		break;

	case 28:
		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani)
			ani->_priority = cmd->_field_14;
		break;

	case 25:
		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani) {
			if (cmd->_field_14) {
				ani->setFlags40(true);
				ani->_callback2 = staticANIObjectCallback;
			} else {
				ani->setFlags40(false);
				ani->_callback2 = 0;
			}
		}
		break;

	case 26:
		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (ani) {
			Movement *mov = ani->_movement;
			if (mov)
				mov->_currDynamicPhase->_field_68 = 0;
		}
		break;

	default:
#if 0
		// We never put anything into _defMsgArray
		while (::iterator it = g_fullpipe->_defMsgArray.begin(); it != g_fullpipe->_defMsgArray.end(); ++it)
			if (((ExCommand *)*it)->_field_24 == _messageNum) {
				((ExCommand *)*it)->firef34(v13);
				res = 1;
			}
#endif

		//debug_msg(_messageNum);

		if (!g_fullpipe->_soundEnabled || cmd->_messageNum != 33 || g_fullpipe->_currSoundListCount <= 0)
			return res;

		for (int snd = 0; snd < g_fullpipe->_currSoundListCount; snd++) {
			SoundList *s = g_fullpipe->_currSoundList1[snd];
		    int ms = s->getCount();
			for (int i = 0; i < ms; i++) {
				s->getSoundByIndex(i)->setPanAndVolumeByStaticAni();
			}
		}
	}

	return res;
}

int global_messageHandler3(ExCommand *cmd) {
	int result = 0;

	if (cmd->_messageKind == 17) {
		switch (cmd->_messageNum) {
		case 29:
		case 30:
		case 31:
		case 32:
		case 36:
			if (g_fullpipe->_inputDisabled)
				cmd->_messageKind = 0;
			break;
		default:
			break;
		}
	}

	StaticANIObject *ani, *ani2;

	switch (cmd->_messageKind) {
	case 17:
		switch (cmd->_messageNum) {
		case 61:
			return g_fullpipe->_gameLoader->preloadScene(cmd->_parentId, cmd->_keyCode);
		case 62:
			return g_fullpipe->_gameLoader->gotoScene(cmd->_parentId, cmd->_keyCode);
		case 64:
			if (g_fullpipe->_currentScene && g_fullpipe->_msgObjectId2
					&& (!(cmd->_keyCode & 4) || g_fullpipe->_msgObjectId2 != cmd->_field_14 || g_fullpipe->_msgId != cmd->_field_20)) {
				ani = g_fullpipe->_currentScene->getStaticANIObject1ById(g_fullpipe->_msgObjectId2, g_fullpipe->_msgId);
				if (ani) {
					ani->_flags &= 0xFF7F;
					ani->_flags &= 0xFEFF;
					ani->deleteFromGlobalMessageQueue();
				}
			}
			g_fullpipe->_msgX = 0;
			g_fullpipe->_msgY = 0;
			g_fullpipe->_msgObjectId2 = 0;
			g_fullpipe->_msgId = 0;
			if ((cmd->_keyCode & 1) || (cmd->_keyCode & 2)) {
				g_fullpipe->_msgX = cmd->_x;
				g_fullpipe->_msgY = cmd->_y;
			}
			if (cmd->_keyCode & 4) {
				g_fullpipe->_msgObjectId2 = cmd->_field_14;
				g_fullpipe->_msgId = cmd->_field_20;
			}
			return result;
		case 29:
			if (!g_fullpipe->_currentScene)
				return result;

			if (g_fullpipe->_gameLoader->_interactionController->_flag24) {
				ani = g_fullpipe->_currentScene->getStaticANIObjectAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
				ani2 = g_fullpipe->_currentScene->getStaticANIObject1ById(g_fullpipe->_gameLoader->_field_FA, -1);
				if (ani) {
					if (g_fullpipe->_msgObjectId2 == ani->_id && g_fullpipe->_msgId == ani->_okeyCode) {
						cmd->_messageKind = 0;
						return result;
					}
					if (canInteractAny(ani2, ani, cmd->_keyCode)) {
						handleObjectInteraction(ani2, ani, cmd->_keyCode);
						return 1;
					}
				} else {
					int id = g_fullpipe->_currentScene->getPictureObjectIdAtPos(cmd->_sceneClickX, cmd->_sceneClickY);
					PictureObject *pic = g_fullpipe->_currentScene->getPictureObjectById(id, 0);
					if (pic) {
						if (g_fullpipe->_msgObjectId2 == pic->_id && g_fullpipe->_msgId == pic->_okeyCode) {
							cmd->_messageKind = 0;
							return result;
						}
						if (!ani2 || canInteractAny(ani2, pic, cmd->_keyCode)) {
							if (!ani2 || (ani2->isIdle() && !(ani2->_flags & 0x80) && !(ani2->_flags & 0x100)))
								handleObjectInteraction(ani2, pic, cmd->_keyCode);
							return 1;
						}
					}
				}
			}
			if (getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->_isEnabled && cmd->_keyCode <= 0) {
				if (g_fullpipe->_msgX != cmd->_sceneClickX || g_fullpipe->_msgY != cmd->_sceneClickY) {
					ani = g_fullpipe->_currentScene->getStaticANIObject1ById(g_fullpipe->_gameLoader->_field_FA, -1);
					if (!ani || (ani->isIdle() && !(ani->_flags & 0x80) && !(ani->_flags & 0x100))) {
						result = startWalkTo(g_fullpipe->_gameLoader->_field_FA, -1, cmd->_sceneClickX, cmd->_sceneClickY, 0);
						if (result) {
							ExCommand *ex = new ExCommand(g_fullpipe->_gameLoader->_field_FA, 17, 64, 0, 0, 0, 1, 0, 0, 0);

							ex->_keyCode = 1;
							ex->_excFlags |= 3;
							ex->_x = cmd->_sceneClickX;
							ex->_y = cmd->_sceneClickY;
							ex->postMessage();
						}
					}
				} else {
					cmd->_messageKind = 0;
				}
			}
			return result;
		default:
			return result;
		}
	case 58:
		g_fullpipe->setCursor(cmd->_keyCode);
		return result;
	case 59:
		setInputDisabled(1);
		return result;
	case 60:
		setInputDisabled(0);
		return result;
	case 56:
		if (cmd->_field_2C) {
			ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
			if (ani) {
				getGameLoaderInventory()->addItem2(ani);
				result = 1;
			}
		} else {
			result = 1;
			getGameLoaderInventory()->addItem(cmd->_parentId, 1);
		}
		getGameLoaderInventory()->rebuildItemRects();
		return result;
	case 57:
		if (cmd->_field_2C) {
			if (!cmd->_field_20) {
				getGameLoaderInventory()->removeItem2(g_fullpipe->_currentScene, cmd->_parentId, cmd->_x, cmd->_y, cmd->_field_14);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
			ani = g_fullpipe->_currentScene->getStaticANIObject1ById(g_fullpipe->_gameLoader->_field_FA, -1);
			if (ani) {
				getGameLoaderInventory()->removeItem2(g_fullpipe->_currentScene, cmd->_parentId, ani->_ox + cmd->_x, ani->_oy + cmd->_y, ani->_priority + cmd->_field_14);
				getGameLoaderInventory()->rebuildItemRects();
				return 1;
			}
		} else {
			getGameLoaderInventory()->removeItem(cmd->_parentId, 1);
		}
		getGameLoaderInventory()->rebuildItemRects();
		return 1;
	case 55:
		if (g_fullpipe->_currentScene) {
			GameObject *obj;
			if (cmd->_field_14)
				obj = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_x, cmd->_y);
			else
				obj = g_fullpipe->_currentScene->getPictureObjectById(cmd->_x, cmd->_y);
			handleObjectInteraction(g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode), obj, cmd->_field_20);
			result = 1;
		}
		return result;
	case 51:
		return startWalkTo(cmd->_parentId, cmd->_keyCode, cmd->_x, cmd->_y, cmd->_field_20);
	case 52:
		return doSomeAnimation(cmd->_parentId, cmd->_keyCode, cmd->_field_20);
	case 53:
		return doSomeAnimation2(cmd->_parentId, cmd->_keyCode);
	case 63:
		if (cmd->_objtype == kObjTypeObjstateCommand) {
			ObjstateCommand *c = (ObjstateCommand *)cmd;
			result = 1;
			g_fullpipe->setObjectState(c->_objCommandName, c->_value);
		}
		return result;
	default:
		return result;
	}
}

int global_messageHandler4(ExCommand *cmd) {
	StaticANIObject *ani = 0;

	switch (cmd->_messageKind) {
	case 18: {
		MessageQueue *mq = new MessageQueue(g_fullpipe->_currentScene->getMessageQueueById(cmd->_messageNum), cmd->_parId, 0);

		if (cmd->_excFlags & 1)
			mq->_flag1 = 1;
		else
			mq->_flag1 = 0;

		mq->sendNextCommand();
		break;
	}
	case 2:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->trySetMessageQueue(cmd->_messageNum, cmd->_parId);
		break;

	case 1: {
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		int flags = cmd->_field_14;
		if (flags <= 0)
			flags = -1;

		if (cmd->_excFlags & 1)
			ani->startAnim(cmd->_messageNum, 0, flags);
		else
			ani->startAnim(cmd->_messageNum, cmd->_parId, flags);

		break;
	}
	case 8:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->startAnimEx(cmd->_messageNum, cmd->_parId, -1, -1);
		break;

	case 20: {
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		int flags = cmd->_field_14;
		if (flags <= 0)
			flags = -1;

		ExCommand2 *cmd2 = (ExCommand2 *)cmd;

		if (cmd->_excFlags & 1) {
			ani->startAnimSteps(cmd->_messageNum, 0, cmd->_x, cmd->_y, cmd2->_points, cmd2->_pointsSize >> 3, flags);
		} else {
			ani->startAnimSteps(cmd->_messageNum, cmd->_parId, cmd->_x, cmd->_y, cmd2->_points, cmd2->_pointsSize >> 3, flags);
		}
		break;
	}
	case 21:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->queueMessageQueue(0);
		ani->playIdle();
		break;
	case 9:
		// Nop in original
		break;
	case 3:
		g_fullpipe->_currentScene->_y = cmd->_messageNum - cmd->_messageNum % g_fullpipe->_scrollSpeed;
		break;

	case 4:
		g_fullpipe->_currentScene->_x = cmd->_messageNum - cmd->_messageNum % g_fullpipe->_scrollSpeed;
		break;

	case 19: {
		if (!g_fullpipe->_currentScene)
			break;
		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		MessageQueue *mq = ani->getMessageQueue();
		MessageQueue *mq2 = ani->changeStatics1(cmd->_messageNum);

		if (!mq2 || !mq2->getExCommandByIndex(0) || !mq)
			break;

		mq2->_parId = mq->_id;
		mq2->_flag1 = (cmd->_field_24 == 0);
		break;
	}
	case 22:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->_flags |= 4;
		ani->changeStatics2(cmd->_messageNum);
		break;

	case 6:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->hide();
		break;

	case 27:
		if (!g_fullpipe->_currentScene || g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode) == 0) {
			ani = g_fullpipe->accessScene(cmd->_field_20)->getStaticANIObject1ById(cmd->_parentId, -1);
			if (ani) {
				ani = new StaticANIObject(ani);
				g_fullpipe->_currentScene->addStaticANIObject(ani, 1);
			}
		}

		// fall through
	case 5:
		if (g_fullpipe->_currentScene)
			ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);

		if (!ani)
			break;

		if (cmd->_field_14 >= 0)
			ani->_priority = cmd->_field_14;

		ani->show1(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 10:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		if (cmd->_field_14 >= 0)
			ani->_priority = cmd->_field_14;

		ani->show2(cmd->_x, cmd->_y, cmd->_messageNum, cmd->_parId);
		break;

	case 7: {
		if (!g_fullpipe->_currentScene->_picObjList.size())
			break;

		int offX = g_fullpipe->_scrollSpeed * (cmd->_x / g_fullpipe->_scrollSpeed);
		int offY = g_fullpipe->_scrollSpeed * (cmd->_y / g_fullpipe->_scrollSpeed);

		if (cmd->_messageNum) {
			g_fullpipe->_currentScene->_x = offX - g_fullpipe->_sceneRect.left;
			g_fullpipe->_currentScene->_y = offY - g_fullpipe->_sceneRect.top;

			if (cmd->_field_24) {
				g_fullpipe->_currentScene->_messageQueueId = cmd->_parId;
			}
		} else {
			g_fullpipe->_sceneRect.moveTo(offX, offY);

			g_fullpipe->_currentScene->_x = 0;
			g_fullpipe->_currentScene->_y = 0;

			g_fullpipe->_currentScene->updateScrolling2();
		}
		break;
	}
	case 34:
		if (!g_fullpipe->_currentScene)
			break;

		ani = g_fullpipe->_currentScene->getStaticANIObject1ById(cmd->_parentId, cmd->_keyCode);
		if (!ani)
			break;

		ani->_flags = cmd->_messageNum | (ani->_flags & ~cmd->_field_14);

		break;

	case 35:
		global_messageHandler_handleSound(cmd);
		break;

	case 11:
	case 12:
		break;
	default:
		return 0;
		break;
	}

	return 1;
}

int MovGraph_messageHandler(ExCommand *cmd) {
	if (cmd->_messageKind != 17)
		return 0;

	if (cmd->_messageNum != 33)
		return 0;

	StaticANIObject *ani = g_fullpipe->_currentScene->getStaticANIObject1ById(g_fullpipe->_gameLoader->_field_FA, -1);

	if (!getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId))
		return 0;

	if (getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId)->_objtype != kObjTypeMovGraph || !ani)
		return 0;

	MovGraph *gr = (MovGraph *)getSc2MctlCompoundBySceneId(g_fullpipe->_currentScene->_sceneId);

	MovGraphLink *link = 0;
	double mindistance = 1.0e10;
	Common::Point point;

	for (ObList::iterator i = gr->_links.begin(); i != gr->_links.end(); ++i) {
		point.x = ani->_ox;
		point.y = ani->_oy;

		double dst = gr->calcDistance(&point, (MovGraphLink *)(*i), 0);
		if (dst >= 0.0 && dst < mindistance) {
			mindistance = dst;
			link = (MovGraphLink *)(*i);
		}
	}

	int top;

	if (link) {
		MovGraphNode *node = link->_movGraphNode1;

		double sq = (ani->_oy - node->_y) * (ani->_oy - node->_y) + (ani->_ox - node->_x) * (ani->_ox - node->_x);
		int off = (node->_field_14 >> 16) & 0xFF;
		double off2 = ((link->_movGraphNode2->_field_14 >> 8) & 0xff) - off;

		top = off + (int)(sqrt(sq) * off2 / link->_distance);
	} else {
		top = (gr->calcOffset(ani->_ox, ani->_oy)->_field_14 >> 8) & 0xff;
	}

	if (ani->_movement) {
		ani->_movement->_currDynamicPhase->_rect->top = 255 - top;
		return 0;
	}

	if (ani->_statics)
		ani->_statics->_rect->top = 255 - top;

	return 0;
}

int defaultUpdateCursor() {
	g_fullpipe->updateCursorsCommon();

	return g_fullpipe->_cursorId;
}

int sceneIntro_updateCursor() {
	g_fullpipe->_cursorId = 0;

	return 0;
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

void sceneIntro_initScene(Scene *sc) {
	g_fullpipe->_gameLoader->loadScene(SC_INTRO2);

	g_vars->sceneIntro_aniin1man = sc->getStaticANIObject1ById(ANI_IN1MAN, -1);
	g_vars->sceneIntro_needSleep = true;
	g_vars->sceneIntro_needGetup = false;
	g_vars->sceneIntro_playing = true;
	g_vars->sceneIntro_needBlackout = false;

	if (g_fullpipe->_recordEvents || g_fullpipe->_inputArFlag)
		g_vars->sceneIntro_skipIntro = false;

	g_fullpipe->_modalObject = new ModalIntro;
}

int sceneHandlerIntro(ExCommand *cmd) {
	warning("STUB: sceneHandlerIntro()");

	return 0;
}

void scene01_fixEntrance() {
	GameVar *var = g_fullpipe->getGameLoaderGameVar()->getSubVarByName("OBJSTATES")->getSubVarByName("SAVEGAME");
	if (var->getSubVarAsInt("Entrance") == TrubaLeft)
		var->setSubVarAsInt("Entrance", TrubaRight);
}

void scene01_initScene(Scene *sc, int entrance) {
	g_vars->scene01_picSc01Osk = sc->getPictureObjectById(PIC_SC1_OSK, 0);
	g_vars->scene01_picSc01Osk->_flags &= 0xFFFB;

	g_vars->scene01_picSc01Osk2 = sc->getPictureObjectById(PIC_SC1_OSK2, 0);
	g_vars->scene01_picSc01Osk2->_flags &= 0xFFFB;

	if (g_fullpipe->getObjectState(sO_EggCracker) == g_fullpipe->getObjectEnumState(sO_EggCracker, sO_DidNotCrackEgg)) {
		PictureObject *pic = sc->getPictureObjectById(PIC_SC1_KUCHKA, 0);
		if (pic)
			pic->_flags &= 0xFFFB;
	}

	if (entrance != TrubaLeft) {
		StaticANIObject *bootAnim = sc->getStaticANIObject1ById(ANI_BOOT_1, -1);
		if (bootAnim)
			bootAnim->_flags &= ~0x04;
	}

	setElevatorButton(sO_Level2, ST_LBN_2N);
}

int sceneHandler01(ExCommand *cmd) {
	int res = 0;

	if (cmd->_messageKind != 17)
		return 0;

	if (cmd->_messageNum > MSG_SC1_SHOWOSK) {
		if (cmd->_messageNum == MSG_SC1_UTRUBACLICK)
			handleObjectInteraction(g_fullpipe->_aniMan, g_fullpipe->_currentScene->getPictureObjectById(PIC_SC1_LADDER, 0), 0);

		return 0;
	}

	if (cmd->_messageNum == MSG_SC1_SHOWOSK) {
		g_vars->scene01_picSc01Osk->_flags |= 4;

		g_vars->scene01_picSc01Osk->_priority = 20;
		g_vars->scene01_picSc01Osk2->_priority = 21;

		return 0;
	}

	if (cmd->_messageNum != 0x21) {
		if (cmd->_messageNum == MSG_SC1_SHOWOSK2) {
			g_vars->scene01_picSc01Osk2->_flags |= 4;
			g_vars->scene01_picSc01Osk2->_priority = 20;
			g_vars->scene01_picSc01Osk->_priority = 21;

			return 0;
		}

		return 0;
	}

	if (g_fullpipe->_aniMan2) {
		if (g_fullpipe->_aniMan2->_ox < g_fullpipe->_sceneRect.left + 200) {
			g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.left - 300;
		}

		if (g_fullpipe->_aniMan2->_ox > g_fullpipe->_sceneRect.right - 200)
			g_fullpipe->_currentScene->_x = g_fullpipe->_aniMan2->_ox - g_fullpipe->_sceneRect.right + 300;

		res = 1;
	}
	g_fullpipe->_behaviorManager->updateBehaviors();

	g_fullpipe->startSceneTrack();

	return res;
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
