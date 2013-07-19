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

namespace Fullpipe {

signed int sceneSwitcher(EntranceInfo *a1, int a2) {
	EntranceInfo *entrance; // ebx@1
	Scene *scene; // esi@1
	CGameVar *sceneVar; // eax@21
	signed int result; // eax@2
	POINT *v6; // eax@3
	int v7; // eax@3
	CInventory2 *v8; // eax@4
	CInventory2 *v9; // eax@4
	int v10; // edi@8
	Sound *v11; // eax@9
	int v12; // ST08_4@12
	int v13; // eax@12
	Scene *v14; // edi@12
	int v15; // eax@13
	int v16; // eax@13
	int v17; // eax@13
	int v18; // eax@13
	CNode *v19; // edi@16
	CNode *v20; // eax@17
	Scene *v21; // eax@18
	PictureObject *v22; // eax@18
	POINT point; // [sp+Ch] [bp-8h]@3

	entrance = a1;
	scene = accessScene(a1->_sceneId);

	if (!scene)
		return 0;

	v6 = PictureObject_getDimensions((PictureObject *)scene->bg.picObjList.m_pNodeHead->data, &point);
	g_sceneWidth = v6->x;
	v7 = v6->y;
	g_sceneHeight = v7;
	g_sceneRect.top = 0;
	g_sceneRect.left = 0;
	g_sceneRect.right = 799;
	g_sceneRect.bottom = 599;
	scene->bg.x = 0;
	scene->bg.y = 0;
	(*(void (__stdcall **)(_DWORD, _DWORD, int))(g_aniMan->GameObject.CObject.vmt + offsetof(GameObjectVmt, setOXY)))(0, 0, a2);
	(*(void (**)(void))(g_aniMan->GameObject.CObject.vmt + offsetof(GameObjectVmt, clearFlags)))();
	g_aniMan->callback2 = 0;
	g_aniMan->callback1 = 0;
	g_aniMan->shadowsOn = 1;
	g_scrollSpeed = 8;
	savesEnabled = 1;
	updateFlag = 1;
	flgCanOpenMap = 1;
	if (entrance->sceneId == SC_DBGMENU) {
		g_inventoryScene = 0;
	} else {
		CGameLoader_loadScene(g_gameLoader, SC_INV);
		v8 = getGameLoaderInventory();
		CInventory2_rebuildItemRects(v8);
		v9 = getGameLoaderInventory();
		g_inventoryScene = CInventory2_getScene(v9);
	}
	if (soundEnabled) {
		if (scene->soundList) {
			g_currSoundListCount = 2;
			v10 = 0;
			g_currSoundList1 = accessScene(SC_COMMON)->soundList;
			for (*(&g_currSoundList1 + 1) = scene->soundList; v10 < SoundList_getCount(scene->soundList); ++v10) {
				v11 = SoundList_getSoundByItemByIndex(scene->soundList, v10);
				(*(void (__thiscall **)(Sound *))(v11->MemoryObject.obj.vmt + offsetof(SoundVmt, updateVolume)))(v11);
			}
		} else {
			g_currSoundListCount = 1;
			g_currSoundList1 = accessScene(SC_COMMON)->soundList;
		}
	}
	v12 = scene->sceneId;
	v13 = (int)getGameLoaderInteractionController();
	CInteractionController_sortInteractions(v13, v12);
	v14 = g_currentScene;
	g_currentScene = v4;
	Scene_addStaticANIObject(scene, (int)g_aniMan, 1);
	g_scene2 = v4;
	g_aniMan->movementObj = 0;
	g_aniMan->staticsObj = StaticANIObject_getStaticsById(g_aniMan, ST_MAN_EMPTY);
	(*(void (__stdcall **)(_DWORD))(g_aniMan->GameObject.CObject.vmt + 24))(0);
	if (g_aniMan) {
		g_aniMan2 = (int)g_aniMan;
		v15 = getSc2MotionControllerBySceneId(LOWORD(entrance->sceneId));
		initMovGraph2((void *)v15);
		v16 = getSc2MotionControllerBySceneId(LOWORD(entrance->sceneId));
		(*(void (__thiscall **)(int, StaticANIObject *))(*(_DWORD *)v16 + offsetof(CMotionControllerVmt, addObject)))(v16, g_aniMan);
		v17 = getSc2MotionControllerBySceneId(LOWORD(entrance->sceneId));
		(*(void (__thiscall **)(int))(*(_DWORD *)v17 + offsetof(CMotionControllerVmt, setField8)))(v17);
		v18 = (int)getGameLoaderInteractionController();
		CInteractionController_enableFlag24(v18);
		input_setInputDisabled(0);
	} else {
		g_aniMan2 = 0;
	}
	g_currentScene = v14;
	Scene_setPictureObjectsFlag4((int)scene);
	if (scene->staticANIObjectList1.m_nCount) {
		v19 = scene->staticANIObjectList1.m_pNodeHead;
		while (v19) {
			v20 = v19;
			v19 = v19->pNext;
			GameObject_setFlags((GameObject *)v20->data, *((_WORD *)v20->data + 6) & 0xFE7F);
		}
	}
	v21 = accessScene(SC_INV);
	v22 = Scene_getPictureObjectById(v21, PIC_INV_MENU, 0);
	GameObject_setFlags(&v22->GameObject, v22->GameObject.flags & 0xFFFB);
	removeMessageHandler(2, -1);
	g_updateScreenCallback = 0;

	switch (entrance->sceneId) {
	case SC_INTRO1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_INTRO1");
		scene->preloadMovements(sceneVar);
		sceneIntro_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_INTRO1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandlerIntro, 2);
		_updateCursorCallback = sceneIntro_updateCursor;
		result = 1;
		break;

	case SC_1:
		scene01_sub_40E160();
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_1");
		scene->preloadMovements(sceneVar);
		scene01_initScene(scene, entrance->field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler01, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
		break;

	case SC_2:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_2");
		scene->preloadMovements(sceneVar);
		scene02_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_2");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler02, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene03_updateCursor;
		result = 1;
		break;

	case SC_4:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_4");
		scene->preloadMovements(sceneVar);
		scene04_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_4");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler04, 2, 2);
		g_updateCursorCallback = scene04_updateCursor;
		result = 1;
		break;

	case SC_5:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_5");
		scene->preloadMovements(sceneVar);
		scene05_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_5");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler05, 2, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene06_updateCursor;
		result = 1;
		break;

	case SC_7:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_7");
		scene->preloadMovements(sceneVar);
		scene07_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_7");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler07, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene08_updateCursor;
		result = 1;
		break;

	case SC_9:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_9");
		scene->preloadMovements(sceneVar);
		scene09_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_9");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler09, 2, 2);
		g_updateCursorCallback = scene09_updateCursor;
		result = 1;
		break;

	case SC_10:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_10");
		scene->preloadMovements(sceneVar);
		scene10_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_10");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler10, 2, 2);
		g_updateCursorCallback = scene10_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene11_updateCursor;
		result = 1;
		break;

	case SC_12:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_12");
		scene->preloadMovements(sceneVar);
		scene12_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_12");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler12, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
		break;

	case SC_13:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_13");
		scene->preloadMovements(sceneVar);
		scene13_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_13");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler13, 2, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene14_updateCursor;
		result = 1;
		break;

	case SC_15:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_15");
		scene->preloadMovements(sceneVar);
		scene15_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_15");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler15, 2, 2);
		g_updateCursorCallback = scene15_updateCursor;
		result = 1;
		break;

	case SC_16:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_16");
		scene->preloadMovements(sceneVar);
		scene16_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_16");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler16, 2);
		g_updateCursorCallback = scene16_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene17_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene18_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene19_updateCursor;
		result = 1;
		break;

	case SC_20:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_20");
		scene->preloadMovements(sceneVar);
		scene20_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_20");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler20, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
		break;

	case SC_21:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_21");
		scene->preloadMovements(sceneVar);
		scene21_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_21");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler21, 2, 2);
		g_updateCursorCallback = scene21_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene22_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene23_updateCursor;
		result = 1;
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
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene25_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene26_updateCursor;
		result = 1;
		break;

	case SC_27:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_27");
		scene->preloadMovements(sceneVar);
		scene27_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_27");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler27, 2);
		g_updateCursorCallback = scene27_updateCursor;
		result = 1;
		break;

	case SC_28:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_28");
		scene->preloadMovements(sceneVar);
		scene28_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_28");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler28, 2, 2);
		g_updateCursorCallback = scene28_updateCursor;
		result = 1;
		break;

	case SC_29:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_29");
		scene->preloadMovements(sceneVar);
		scene29_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_29");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler29, 2);
		g_updateCursorCallback = scene29_updateCursor;
		result = 1;
		break;

	case SC_30:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_30");
		scene->preloadMovements(sceneVar);
		scene30_initScene(scene, entrance->field_4);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_30");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler30, 2);
		g_updateCursorCallback = scene30_updateCursor;
		result = 1;
		break;

	case SC_31:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_31");
		scene->preloadMovements(sceneVar);
		scene31_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_31");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler31, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
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
		g_updateCursorCallback = scene32_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene33_updateCursor;
		result = 1;
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
		g_updateCursorCallback = scene34_updateCursor;
		result = 1;
		break;

	case SC_35:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_35");
		scene->preloadMovements(sceneVar);
		scene35_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_35");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler35, 2, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
		break;

	case SC_36:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_36");
		scene->preloadMovements(sceneVar);
		scene36_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_36");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler36, 2);
		g_updateCursorCallback = scene36_updateCursor;
		result = 1;
		break;

	case SC_37:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_37");
		scene->preloadMovements(sceneVar);
		scene37_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_37");
		setSceneMusicParameters(sceneVar);
		insertMessageHandler(sceneHandler37, 2, 2);
		g_updateCursorCallback = scene37_updateCursor;
		result = 1;
		break;

	case SC_38:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_38");
		scene->preloadMovements(sceneVar);
		scene38_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_38");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandler38, 2);
		g_updateCursorCallback = defaultUpdateCursorCallback;
		result = 1;
		break;

	case SC_FINAL1:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_FINAL1");
		scene->preloadMovements(sceneVar);
		sceneFinal1_initScene();
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_FINAL1");
		setSceneMusicParameters(sceneVar);
		addMessageHandler(sceneHandlerFinal1, 2);
		g_updateCursorCallback = sceneFinal1_updateCursor;
		result = 1;
		break;

	case SC_DBGMENU:
		sceneVar = _gameLoader->_gameVar->getSubVarByName("SC_DBGMENU");
		scene->preloadMovements(sceneVar);
		sceneDbgMenu_initScene(scene);
		_behaviorManager->initBehavior(scene, sceneVar);
		scene->initObjectCursors("SC_DBGMENU");
		addMessageHandler(sceneHandlerDbgMenu, 2);
		result = 1;
		break;

	default:
		_behaviorManager->initBehavior(0, 0);
		result = 1;
		break;
	}

	return result;
}

} // End of namespace Fullpipe
