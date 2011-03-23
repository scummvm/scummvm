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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "tsage/ringworld_scenes5.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 4000 - Village
 *
 *--------------------------------------------------------------------------*/

void Scene4000::Action1::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2870);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setPosition(Common::Point(116, 160));
		
		ADD_PLAYER_MOVER_NULL(scene->_hotspot5, 208, 169);
		
		_globals->_inventory._ale._sceneNumber = 0;
		_globals->clearFlag(42);
		_globals->clearFlag(36);
		_globals->clearFlag(43);
		_globals->clearFlag(37);
		break;
	}
	case 1: {
		scene->_hotspot9.postInit();
		scene->_hotspot9.setVisage(4001);
		scene->_hotspot9.animate(ANIM_MODE_1, NULL);
		scene->_hotspot9.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot9.setPosition(Common::Point(314, 132));

		ADD_PLAYER_MOVER_NULL(scene->_hotspot9, 288, 167);

		scene->_hotspot4.postInit();
		scene->_hotspot4.setVisage(4006);
		scene->_hotspot4.animate(ANIM_MODE_1, NULL);
		scene->_hotspot4.setStrip(1);
		scene->_hotspot4.setPosition(Common::Point(207, 136));
		
		ADD_PLAYER_MOVER_NULL(scene->_hotspot4, 220, 151);

		scene->_hotspot7.postInit();
		scene->_hotspot7.setVisage(2701);
		scene->_hotspot7.animate(ANIM_MODE_1, NULL);
		scene->_hotspot7.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot7._moveDiff = Common::Point(4, 2);
		scene->_hotspot7.setPosition(Common::Point(300, 135));

		ADD_PLAYER_MOVER_NULL(_globals->_player, 266, 169);
		break;
	}
	case 2:
		scene->_stripManager.start(4400, this);
		break;
	case 3: {
		Common::Point pt1(30, 86);
		PlayerMover *mover1 = new PlayerMover();
		scene->_hotspot7.addMover(mover1, &pt1, this);

		ADD_PLAYER_MOVER_NULL(scene->_hotspot5, 3, 86);
		break;
	}
	case 4:
		ADD_MOVER(scene->_hotspot7, -30, 86);
		ADD_MOVER(scene->_hotspot5, -40, 86);
		break;
	case 5:
		_globals->_soundHandler.startSound(155);
		_globals->setFlag(43);
		_globals->setFlag(114);
		scene->_stripManager.start(4430, this);
		break;
	case 6:
		ADD_PLAYER_MOVER_THIS(scene->_hotspot4, 277, 175);
		ADD_PLAYER_MOVER_NULL(_globals->_player, 258, 187);
		break;
	case 7:
		scene->_stripManager.start(4440, this);
		break;
	case 8:
		setDelay(30);
		break;
	case 9:
		_globals->setFlag(96);
		_globals->_sceneManager.changeScene(4025);
		break;
	}
}

void Scene4000::Action2::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setAction(&scene->_sequenceManager1, this, 4001, &_globals->_player, NULL);

		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2801);
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5._moveDiff.x = 5;
		scene->_hotspot5.setPosition(Common::Point(-8, 88));

		scene->_hotspot3.setAction(&scene->_sequenceManager3, NULL, 4003, &scene->_hotspot5, NULL);
		scene->_hotspot7.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_hotspot7, NULL);
		break;
	case 1:
		_globals->_player.disableControl();

		scene->_hotspot3.remove();
		ADD_MOVER(scene->_hotspot9, scene->_hotspot5._position.x + 30, scene->_hotspot5._position.y - 10);
		break;
	case 2:
		_globals->_player.checkAngle(&scene->_hotspot9);
		scene->_hotspot5.checkAngle(&scene->_hotspot9);
		scene->_hotspot7.checkAngle(&scene->_hotspot9);
		scene->_stripManager.start(4000, this);
		break;
	case 3:
		scene->_hotspot2.setVisage(4017);
		scene->_hotspot2.animate(ANIM_MODE_1, NULL);
		scene->_hotspot2.setStrip(2);

		ADD_MOVER(scene->_hotspot2, 116, 160);
		ADD_MOVER(scene->_hotspot5, 116, 160);

		_globals->setFlag(37);
		break;
	case 4:
		break;
	case 5:
		scene->_stripManager.start(4010, this);
		break;
	case 6:
		ADD_PLAYER_MOVER_NULL(scene->_hotspot9, 230, 149);
		ADD_PLAYER_MOVER(210, 136);
		ADD_PLAYER_MOVER_NULL(scene->_hotspot7, 210, 133);
		break;
	case 7:
		_globals->_sceneManager.changeScene(4045);
		break;
	}
}

void Scene4000::Action3::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_hotspot8.setVisage(4017);
		scene->_hotspot8.setFrame2(-1);
		scene->_hotspot8.animate(ANIM_MODE_1, NULL);
		scene->_hotspot8.setObjectWrapper(new SceneObjectWrapper());
		ADD_MOVER(scene->_hotspot8, 118, 145);
		break;
	case 1:
		scene->_hotspot8.remove();
		remove();
		break;
	}
}

void Scene4000::Action4::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player._uiEnabled = false;
		ADD_MOVER(_globals->_player, 257, 57);
		break;
	case 1:
		_globals->_player.setVisage(4000);
		_globals->_player.setPosition(Common::Point(258, 83));
		_globals->_player._frame = 1;
		_globals->_player._strip = 3;
		_globals->_player.animate(ANIM_MODE_4, 2, 1, this);
		break;
	case 2:
		scene->_hotspot6.postInit();
		scene->_hotspot6.setVisage(4000);
		scene->_hotspot6.setStrip(7);
		scene->_hotspot6.setFrame(3);
		scene->_hotspot6.setPosition(Common::Point(268, 44));

		_globals->_inventory._rope._sceneNumber = 4000;
		_globals->_events.setCursor(CURSOR_USE);
		_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		_globals->_player.setVisage(2602);
		_globals->_player.setPosition(Common::Point(257, 57));
		_globals->_player.animate(ANIM_MODE_1, NULL);
		_globals->_player._uiEnabled = true;

		_globals->setFlag(41);
		remove();
		break;
	}
}

void Scene4000::Action5::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(15);
		break;
	case 1:
		scene->_stripManager.start(_globals->_stripNum, this);
		break;
	case 2:
		setDelay(10);
		break;
	case 3:
		scene->_hotspot8.setVisage(4017);
		scene->_hotspot8.animate(ANIM_MODE_1, NULL);
		scene->_hotspot8.setFrame2(-1);
		scene->_hotspot8.setAction(&scene->_action3);

		_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4000::Action6::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 4001, &_globals->_player, NULL);

		if (!_globals->getFlag(36))
			scene->_hotspot7.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_hotspot7, NULL);
		break;
	case 2:
		_globals->_player.disableControl();
		ADD_MOVER(scene->_hotspot9, _globals->_player._position.x + 30, _globals->_player._position.y - 5);
		break;
	case 3:
		scene->_stripManager.start(_globals->getFlag(35) ? 4500 : 4502, this);
		break;
	case 4:
		_globals->clearFlag(35);
		ADD_MOVER_NULL(scene->_hotspot9, 292, 138);
		ADD_PLAYER_MOVER(283, 147);

		if (!_globals->getFlag(36)) {
			ADD_PLAYER_MOVER_NULL(scene->_hotspot7, 280, 150);
		}
		break;
	case 5:
		_globals->_sceneManager.changeScene(4100);
		break;
	}
}

void Scene4000::Action7::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		
		scene->_hotspot6.setFrame(1);
		ADD_MOVER(_globals->_player, 247, 53);
		break;
	case 1:
		_globals->_player.setVisage(4008);
		_globals->_player.setStrip(4);
		_globals->_player.setFrame(1);
		_globals->_player.setPriority2(16);
		_globals->_player.setPosition(Common::Point(260, 55));
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		_globals->_sceneManager.changeScene(4050);
		break;
	}
}

void Scene4000::Action8::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		if (_globals->getFlag(41))
			scene->_hotspot6.setFrame(2);

		ADD_MOVER(_globals->_player, 289, 53);
		break;
	case 1:
		_globals->_player.setVisage(4008);
		_globals->_player.setStrip(5);
		_globals->_player.setPriority(16);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(283, 52));
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		_globals->_player.remove();
		setDelay(60);
		break;
	case 3:
		_globals->_soundHandler.startSound(170);
		scene->_hotspot27.setVisage(4000);
		scene->_hotspot27.setStrip(6);
		scene->_hotspot27.animate(ANIM_MODE_2, NULL);
		setDelay(60);
		break;
	case 4:
		_globals->_soundHandler.startSound(77, this);
		break;
	case 5:
		_globals->_game.endGame(4000, 15);
		remove();
		break;
	}
}

void Scene4000::Action9::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(119) + 240);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
		break;
	}
}

void Scene4000::Action10::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(119) + 240);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
		break;
	}
}

void Scene4000::Action11::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		ADD_MOVER(scene->_hotspot3, -30, 70);
		break;
	case 1:
		setDelay(60);
		break;
	case 2:
		scene->_hotspot5.postInit();
		scene->_hotspot5.setVisage(2801);
		scene->_hotspot5.animate(ANIM_MODE_1, NULL);
		scene->_hotspot5.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot5._moveDiff.x = 4;
		scene->_hotspot5.setPosition(Common::Point(-8, 88));

		setAction(&scene->_sequenceManager1, this, 4001, &_globals->_player, NULL);

		scene->_hotspot7.setPosition(Common::Point(-210, 139));
		scene->_hotspot7.setAction(&scene->_sequenceManager2, NULL, 4002, &scene->_hotspot7, NULL);
		scene->_hotspot3.setAction(&scene->_sequenceManager3, NULL, 4003, &scene->_hotspot5, NULL);
		break;
	case 3:
		scene->_stripManager.start(8000, this);
		break;
	case 4:
		ADD_MOVER(scene->_hotspot4, 263, 187);
		scene->_hotspot4.animate(ANIM_MODE_1, NULL);
		break;
	case 5:
		scene->_soundHandler1.proc3();
		scene->_hotspot11.remove();

		ADD_MOVER(_globals->_player, 340, 163);
		ADD_MOVER_NULL(scene->_hotspot7, 340, 169);
		ADD_MOVER_NULL(scene->_hotspot5, 340, 165);
		break;
	case 6:
		_globals->_sceneManager.changeScene(4250);
		break;
	}
}

void Scene4000::Action12::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(5);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(4015, this);
		break;
	case 2:
		_globals->setFlag(32);
		if (scene->_stripManager._field2E8 == 275) {
			_globals->setFlag(82);
			ADD_MOVER_NULL(scene->_hotspot9, 292, 138);
			ADD_PLAYER_MOVER(283, 147);
		} else {
			setDelay(30);
		}
		break;
	case 3:
		if (scene->_stripManager._field2E8 == 275) {
			_globals->_sceneManager.changeScene(4100);
		} else {
			ADD_PLAYER_MOVER_THIS(scene->_hotspot9, 300, 132);
		}
		break;
	case 4:
		scene->_hotspot9.flag100();
		scene->_stripManager.start(4020, this);
		break;
	case 5:
		_globals->setFlag(35);
		_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene4000::Action13::signal() {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		scene->_soundHandler2.startSound(151);
		scene->_soundHandler2.proc5(true);
		ADD_MOVER(scene->_hotspot3, -30, 70);
		break;
	case 2:
		scene->_soundHandler2.proc4();
		_globals->_sceneManager.changeScene(4010);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4000::Hotspot7::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 31);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 29);
		break;
	case CURSOR_TALK:
		if (_globals->getFlag(31)) {
			if (!_globals->getFlag(111)) {
				_globals->setFlag(111);
				_globals->_stripNum = 4070;
			} else if (!_globals->getFlag(33))
				_globals->_stripNum = 4094;
			else if (!_globals->getFlag(112)) {
				_globals->setFlag(112);
				_globals->_stripNum = 4300;
			} else if (!_globals->getFlag(113)) {
				_globals->setFlag(113);
				_globals->_stripNum = 4093;
			} else
				_globals->_stripNum = 4094;
		} else {
			if (!_globals->getFlag(33))
				_globals->_stripNum = 4094;
			else if (!_globals->getFlag(112)) {
				_globals->setFlag(112);
				_globals->_stripNum = 4300;
			} else
				_globals->_stripNum = 4094;
		}

		scene->setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot8::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 26);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 25);
		break;
	case CURSOR_TALK:
		if (_globals->_inventory._peg._sceneNumber == 1)
			SceneItem::display2(4000, 34);
		else {
			switch (_ctr) {
			case 0:
				_globals->_stripNum = 4090;
				break;
			case 1:
				_globals->_stripNum = 4091;
				break;
			case 2:
				_globals->_stripNum = 4092;
				break;
			default:
				SceneItem::display2(4000, 34);
				break;
			}

			if (_globals->_stripNum) {
				setAction(NULL);
				addMover(NULL);
				++_ctr;
				scene->setAction(&scene->_action5);
			}
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot9::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 28);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4000, 27);
		break;
	case CURSOR_TALK:
		error("*** Do we need dialog.");
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot10::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 30);
		break;
	case CURSOR_USE:
		_globals->_player.disableControl();
		if (_globals->getFlag(40)) {
			scene->_sceneMode = 4005;
			scene->setAction(&scene->_sequenceManager1, scene, 4005, &_globals->_player, NULL);
		} else {
			scene->_sceneMode = 4004;
			scene->setAction(&scene->_sequenceManager1, scene, 4004, &_globals->_player, &scene->_hotspot10, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot12::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 13);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4000, 19);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 20);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(40))
			SceneItem::display2(4000, 37);
		else {
			_globals->_player.disableControl();
			if (_globals->_sceneObjects->contains(&scene->_hotspot7))
				_globals->clearFlag(96);

			scene->_sceneMode = 4012;
			scene->setAction(&scene->_sequenceManager1, scene, 4012, &_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot13::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 21);
		break;
	case OBJECT_ROPE:
		if (_globals->getFlag(40))
			scene->setAction(&scene->_action4);
		else
			SceneItem::display2(4000, 22);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 16);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(4000, 17);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(4000, 18);
		break;
	case OBJECT_LADDER:
		_globals->_player.disableControl();

		scene->_hotspot10.postInit();
		scene->_hotspot10.setVisage(4000);
		scene->_hotspot10.setStrip(5);
		scene->_hotspot10.setPosition(Common::Point(245, 147));
		scene->_hotspot10.flag100();
		_globals->_sceneItems.push_front(&scene->_hotspot10);

		if (_globals->_sceneObjects->contains(&scene->_hotspot8)) {
			scene->_hotspot8.setAction(NULL);
			ADD_MOVER_NULL(scene->_hotspot8, 118, 145);
		}

		scene->_sceneMode = 4004;
		scene->setAction(&scene->_sequenceManager1, scene, 4011, &_globals->_player, &scene->_hotspot10, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}	
}

void Scene4000::Hotspot17::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 23);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(40))
			scene->setAction(&scene->_action7);
		else
			SceneItem::display2(4000, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot18::doAction(int action) {
	Scene4000 *scene = (Scene4000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, 36);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(40))
			scene->setAction(&scene->_action8);
		else
			SceneItem::display2(4000, 24);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4000::Hotspot23::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4000, _globals->getFlag(31) ? 10 : 9);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4000::Scene4000():
	_hotspot11(0, CURSOR_LOOK, 4000, 14, CURSOR_USE, 4000, 32, OBJECT_STUNNER, 4000, 33, 
		OBJECT_SCANNER, 4000, 19, LIST_END),
	_hotspot19(0, CURSOR_LOOK, 4000, 7, LIST_END),
	_hotspot20(0, CURSOR_LOOK, 4000, 3, LIST_END),
	_hotspot21(0, CURSOR_LOOK, 4000, 1, LIST_END),
	_hotspot22(0, CURSOR_LOOK, 4000, 8, LIST_END),
	_hotspot24(0, CURSOR_LOOK, 4000, 11, LIST_END),
	_hotspot25(0, CURSOR_LOOK, 4000, 4, LIST_END),
	_hotspot26(0, CURSOR_LOOK, 4000, 0, LIST_END) {
}

void Scene4000::postInit(SceneObjectList *OwnerList) {
	loadScene(4000);
	Scene::postInit();
	setZoomPercents(0, 20, 70, 50);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerCHFL);
	_stripManager.addSpeaker(&_speakerPL);
	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerCHFR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerCHFText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);

	_speakerCHFText._npc = &_hotspot9;
	_speakerSText._npc = &_hotspot5;
	_speakerMText._npc = &_hotspot7;
	_speakerPText._npc = &_hotspot4;
	_speakerQText._npc = &_globals->_player;

	_hotspot13.setBounds(Rect(263, 41, 278, 55));
	_hotspot14.setBounds(Rect(140, 177, 140 /*96*/, 204));
	_hotspot15.setBounds(Rect(227, 101, 264, 143));
	_hotspot16.setBounds(Rect(306, 100, 319, 148));
	_hotspot17.setBounds(Rect(231, 53, 254, 60));
	_hotspot18.setBounds(Rect(285, 51, 310, 60));
	_hotspot26.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_hotspot21.setBounds(Rect(28, 7, 53, 64));
	_hotspot22.setBounds(Rect(125, 155, 140, 189));
	_hotspot23.setBounds(Rect(205, 102, 220, 142));
	_hotspot24.setBounds(Rect(270, 111, 297, 147));

	_hotspot19._sceneRegionId = 16;
	_hotspot20._sceneRegionId = 18;
	_hotspot25._sceneRegionId = 17;

	_hotspot1.postInit();
	_hotspot1.setVisage(4000);
	_hotspot1.setPriority2(1);
	_hotspot1.setFrame(2);
	_hotspot1.setPosition(Common::Point(242, 59));
	_hotspot1.animate(ANIM_MODE_2, NULL);

	_hotspot27.postInit();
	_hotspot27.setVisage(4000);
	_hotspot27.setStrip(2);
	_hotspot27.setPriority2(1);
	_hotspot27.setFrame(2);
	_hotspot27.setPosition(Common::Point(299, 59));
	_hotspot27.animate(ANIM_MODE_2, NULL);

	if (_globals->_inventory._ladder._sceneNumber != 4000) {
		_hotspot8.postInit();
		_hotspot8.setVisage(4018);
		_hotspot8.setObjectWrapper(new SceneObjectWrapper());
		_hotspot8._strip = 2;
		_hotspot8._numFrames = 5;
		_hotspot8.setPosition(Common::Point(306, 154));
		_hotspot8.setAction(&_action9);
	}

	_hotspot12.postInit();
	_hotspot12.setVisage(4000);
	_hotspot12.setStrip(3);
	_hotspot12.setFrame(3);
	_hotspot12.setPriority2(200);
	_hotspot12.setPosition(Common::Point(281, 176));
	
	if (_globals->getFlag(34)) {
		_soundHandler1.startSound(156);

		_hotspot11.postInit();
		_hotspot11.setVisage(4000);
		_hotspot11.setStrip(4);
		_hotspot11.setPosition(Common::Point(312, 174));
		_hotspot11.setPriority2(200);
		_hotspot11.animate(ANIM_MODE_8, 0, NULL);

		_globals->_sceneItems.push_back(&_hotspot11);
	}

	_globals->_player.postInit();
	_globals->_player.setVisage(2602);
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setPosition(Common::Point(-28, 86));

	if (!_globals->getFlag(36) && !_globals->getFlag(43)) {
		_hotspot7.postInit();
		_hotspot7.setVisage(2701);
		_hotspot7.animate(ANIM_MODE_1, NULL);
		_hotspot7.setObjectWrapper(new SceneObjectWrapper());
		_hotspot7._moveDiff = Common::Point(4, 2);
		_hotspot7.setPosition(Common::Point(-210, 139));
		
		_globals->_sceneItems.push_back(&_hotspot7);
	}

	switch (_globals->_sceneManager._previousScene) {
	case 2320:
		_globals->_soundHandler.startSound(155);

		if (_globals->_inventory._ale._sceneNumber == 1) {
			_hotspot9.postInit();
			_hotspot9.setVisage(4001);
			_hotspot9.animate(ANIM_MODE_1, NULL);
			_hotspot9.setObjectWrapper(new SceneObjectWrapper());
			_hotspot9.setPosition(Common::Point(314, 132));

			setAction(&_action6);
		} else {
			_globals->_player.disableControl();
			_sceneMode = 4001;
			setAction(&_sequenceManager1, this, 4001, &_globals->_player, NULL);

			if (!_globals->getFlag(36) && !_globals->getFlag(43))
				_hotspot7.setAction(&_sequenceManager2, NULL, 4002, &_hotspot7, NULL);
		}

		if (_globals->getFlag(42))
			_hotspot8.setAction(&_action3);
		else if (_globals->getFlag(91))
			_hotspot8.remove();
		break;

	case 4010:
		_hotspot7.setPosition(Common::Point(-210, 139));
		
		_hotspot9.postInit();
		_hotspot9.setVisage(4001);
		_hotspot9.animate(ANIM_MODE_1, NULL);
		_hotspot9.setObjectWrapper(new SceneObjectWrapper());
		_hotspot9.setPosition(Common::Point(314, 132));
		
		_hotspot2.postInit();
		_hotspot2.setVisage(4018);
		_hotspot2._strip = 1;
		_hotspot2._numFrames = 5;
		_hotspot2.setPosition(Common::Point(182, 146));
		_hotspot2.setAction(&_action10);

		setAction(&_action2);
		break;

	case 4025:
		if (_globals->_inventory._ladder._sceneNumber != 4000)
			_hotspot8.remove();

		_globals->_player.setPosition(Common::Point(260, 185));
		
		if (!_globals->getFlag(36) && !_globals->getFlag(43))
			_hotspot7.setPosition(Common::Point(246, 146));

		if (_globals->getFlag(96)) {
			_hotspot4.postInit();
			_hotspot4.setVisage(4006);
			_hotspot4.animate(ANIM_MODE_1, NULL);
			_hotspot4.setObjectWrapper(new SceneObjectWrapper());
			_hotspot4.setPosition(Common::Point(290, 163));
		}

		if (_globals->_stripNum == 4025) {
			_soundHandler1.startSound(182);
			_hotspot11.remove();

			_hotspot5.postInit();
			_hotspot5.setVisage(2801);
			_hotspot5.animate(ANIM_MODE_1, NULL);
			_hotspot5.setObjectWrapper(new SceneObjectWrapper());
			_hotspot5._moveDiff.x = 4;
			_hotspot5.setPosition(Common::Point(-18, 86));

			_globals->_player.disableControl();
			
			if (!_globals->getFlag(96)) {
				_hotspot4.postInit();
				_hotspot4.setVisage(4006);
				_hotspot4.animate(ANIM_MODE_1, NULL);
				_hotspot4.setObjectWrapper(new SceneObjectWrapper());
				_hotspot4.setPosition(Common::Point(195, 128));

				Common::Point pt(268, 157);
				PlayerMover *mover = new PlayerMover();
				_hotspot4.addMover(mover, &pt, NULL);
			}

			_sceneMode = 4003;
			setAction(&_sequenceManager1, this, 4003, &_hotspot5, NULL);
		} else if (_globals->getFlag(96)) {
			_globals->_player.disableControl();
			_sceneMode = 4013;
			setAction(&_sequenceManager1, this, 4013, &_hotspot4, NULL);
		}

		_globals->clearFlag(96);
		break;

	case 4045:
		_globals->_player.enableControl();

		if (_globals->_inventory._ladder._sceneNumber != 4000) {
			_hotspot8.postInit();
			_hotspot8.setVisage(4017);
			_hotspot8.animate(ANIM_MODE_1, NULL);
			_hotspot8.setPosition(Common::Point(199, 188));
			_hotspot8.setAction(&_action3);
		}

		_globals->_player.setPosition(Common::Point(208, 153));
		if (!_globals->getFlag(36) && !_globals->getFlag(43))
			_hotspot7.setPosition(Common::Point(246, 146));

		if (_globals->getFlag(39)) {
			_globals->clearFlag(39);

			_hotspot4.postInit();
			_hotspot4.setVisage(4006);
			_hotspot4.animate(ANIM_MODE_1, NULL);
			_hotspot4.setObjectWrapper(new SceneObjectWrapper());
			_hotspot4.setPosition(Common::Point(219, 150));

			_globals->_player.disableControl();
			setAction(&_sequenceManager1, this, 4010, &_globals->_player, NULL);
		}

		if (_globals->_stripNum == 4000) {
			_globals->_stripNum = 0;

			_hotspot9.postInit();
			_hotspot9.setVisage(4001);
			_hotspot9.animate(ANIM_MODE_1, NULL);
			_hotspot9.setObjectWrapper(new SceneObjectWrapper());
			_hotspot9.setPosition(Common::Point(231, 159));
			_hotspot9.setStrip(4);
			
			setAction(&_action12);
		}
		break;

	case 4050:
		_globals->_soundHandler.startSound(155);
		_globals->_player.disableControl();

		if (_globals->_stripNum == 4050) {
			_globals->_player.setVisage(4008);
			_globals->_player.setStrip(4);
			_globals->_player.setFrame(_globals->_player.getFrameCount());
			_globals->_player.setPriority2(16);
			_globals->_player.setPosition(Common::Point(260, 55));
			
			_sceneMode = 4007;
			setAction(&_sequenceManager1, this, 4007, &_globals->_player, NULL);
		} else {
			_globals->_player.setPosition(Common::Point(208, 153));
			_globals->_player.enableControl();
		}

		if (_globals->_inventory._ladder._sceneNumber != 4000)
			_hotspot8.remove();
		break;

	case 4100:
		_globals->_player.enableControl();
		_globals->_player.setPosition(Common::Point(270, 155));

		if (_globals->getFlag(42) && (_globals->_inventory._ladder._sceneNumber != 4000)) {
			_hotspot8.setVisage(4017);
			_hotspot8.animate(ANIM_MODE_1, NULL);
			_hotspot8.setPosition(Common::Point(244, 151));
			_hotspot8.setAction(&_action3);
		}

		if (!_globals->getFlag(36) && !_globals->getFlag(43))
			_hotspot7.setPosition(Common::Point(246, 146));
		break;

	default:
		_globals->_soundHandler.startSound(155);

		_hotspot3.postInit();
		_hotspot3.setVisage(4002);
		_hotspot3._moveDiff = Common::Point(10, 10);
		_hotspot3.setPosition(Common::Point(-100, 80));
		_hotspot3.changeZoom(-1);
		_hotspot3.setPosition(Common::Point(130, -1));
		_hotspot3.animate(ANIM_MODE_2, NULL);

		if (_globals->_stripNum == 9000) {
			_hotspot4.postInit();
			_hotspot4.setVisage(4006);
			_hotspot4.setPosition(Common::Point(235, 153));

			_hotspot9.postInit();
			_hotspot9.setVisage(4001);
			_hotspot9.setStrip(3);
			_hotspot9.setPosition(Common::Point(255, 153));
			
			setAction(&_action11);
			_globals->_inventory._ladder._sceneNumber = 4100;
			_globals->_inventory._rope._sceneNumber = 4150;

			_soundHandler1.startSound(156);

			_hotspot11.postInit();
			_hotspot11.setVisage(4000);
			_hotspot11.setStrip(4);
			_hotspot11.setPosition(Common::Point(312, 174));
			_hotspot11.setPriority2(200);
			_hotspot11.animate(ANIM_MODE_8, 0, NULL);
		} else {
			if (!_globals->getFlag(37)) {
				_hotspot2.postInit();
				_hotspot2.setVisage(4018);
				_hotspot2._strip = 1;
				_hotspot2._numFrames = 5;
				_hotspot2.setPosition(Common::Point(182, 146));
				_hotspot2.setAction(&_action10);
			}

			_hotspot7.setPosition(Common::Point(-210, 139));
			setAction(&_action13);
		}
		break;
	}

	if (_globals->_inventory._ladder._sceneNumber == 4000) {
		_hotspot10.postInit();
		_hotspot10.setVisage(4000);
		_hotspot10.setStrip(5);
		_hotspot10.setPosition(Common::Point(245, 147));

		_globals->_sceneItems.push_back(&_hotspot10);
	}

	if (_globals->_inventory._rope._sceneNumber == 4000) {
		_hotspot6.postInit();
		_hotspot6.setVisage(4000);
		_hotspot6.setStrip(7);
		_hotspot6.setPriority2(1);
		_hotspot6.setPosition(Common::Point(268, 44));
	}

	_globals->_sceneItems.addItems(&_hotspot8, &_hotspot17, &_hotspot18, &_hotspot14, &_hotspot15,
		&_hotspot16, &_hotspot12, &_hotspot13, &_hotspot21, &_hotspot20, &_hotspot22, &_hotspot23,
		&_hotspot24, &_hotspot25, &_hotspot19, &_hotspot26, NULL);
}

void Scene4000::signal() {
	switch (_sceneMode) {
	case 4001:
		_globals->_player.enableControl();
		break;
	case 4002:
	case 4011:
		break;
	case 4003:
		_sceneMode = 4014;
		setAction(&_sequenceManager1, this, 4014, &_globals->_player, &_hotspot5, NULL);
		break;
	case 4004:
		_globals->_inventory._ladder._sceneNumber = 4000;
		// Deliberate fall-through
	case 4007:
		_globals->_events.setCursor(CURSOR_USE);
		_globals->setFlag(40);
		break;
	case 4005:
		_globals->_player.enableControl();
		_globals->_events.setCursor(CURSOR_WALK);
		_globals->clearFlag(40);
		break;
	case 4006:
		_globals->_sceneManager.changeScene(4045);
		break;
	case 4008:
		_globals->_sceneManager.changeScene(2320);
		break;
	case 4009:
		_globals->_sceneManager.changeScene(2200);
		break;
	case 4010:
		_globals->setFlag(38);
		_hotspot4.remove();
		break;
	case 4012:
		_globals->_player.checkAngle(&_hotspot12);
		_globals->_sceneManager.changeScene(4025);
		break;
	case 4013:
		_globals->_player.enableControl();
		_hotspot4.remove();
		break;
	case 4014:
		_globals->_sceneManager.changeScene(4250);
		break;
	case 4015:
		ADD_MOVER_NULL(_hotspot7, 0, _hotspot7._position.y - 5);
		break;
	}
}

void Scene4000::dispatch() {
	Scene::dispatch();

	if ((_globals->_player.getRegionIndex() == 10) || (_globals->_player.getRegionIndex() == 6))
		_globals->_player.setPriority2(200);
	if (_globals->_player.getRegionIndex() == 11)
		_globals->_player.setPriority2(-1);
	if (_globals->_player.getRegionIndex() == 5)
		_globals->_player.setPriority2(94);

	if (_globals->_sceneObjects->contains(&_hotspot5)) {
		if ((_hotspot5.getRegionIndex() == 10) || (_hotspot5.getRegionIndex() == 6))
			_hotspot5.setPriority2(200);
		if (_hotspot5.getRegionIndex() == 11)
			_hotspot5.setPriority2(-1);
		if (_hotspot5.getRegionIndex() == 5)
			_hotspot5.setPriority2(94);
	}

	if (_globals->_sceneObjects->contains(&_hotspot7)) {
		if (!_hotspot7._mover)
			_hotspot7.checkAngle(&_globals->_player);
		if (!_action && _globals->_player.getRegionIndex() == 23) {
			ADD_MOVER_NULL(_hotspot7, 204, 186);
		}

		if ((_hotspot7.getRegionIndex() == 10) || (_hotspot7.getRegionIndex() == 6))
			_hotspot7.setPriority2(200);
		if (_hotspot7.getRegionIndex() == 11)
			_hotspot7.setPriority2(-1);
		if (_hotspot7.getRegionIndex() == 5)
			_hotspot7.setPriority2(94);
	}

	if (!_action) {
		if ((_globals->_inventory._peg._sceneNumber == 1) && _globals->getFlag(34) &&
				_globals->getFlag(37) && !_globals->getFlag(40)) {
			_globals->_player.disableControl();
			_soundHandler1.startSound(177);
			_globals->_soundHandler.startSound(178);
			
			setAction(&_action1);
		}

		if (_globals->_player.getRegionIndex() == 2)
			_globals->_sceneManager.changeScene(4045);
		if (_globals->_player.getRegionIndex() == 15)
			_globals->_sceneManager.changeScene(4100);

		if ((_globals->_player._position.x > 5) && (_globals->_player._position.y < 100)) {
			_globals->_player.disableControl();

			if (!_globals->_sceneObjects->contains(&_hotspot7) || (_hotspot7._position.y <= 100)) {
				_sceneMode = 4008;
				setAction(&_sequenceManager1, this, 4008, &_globals->_player, NULL);
			} else {
				_sceneMode = 4015;
				_globals->_player.addMover(NULL);
				setAction(&_sequenceManager1, this, 4015, &_globals->_player, &_hotspot7, NULL);
			}
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 4010 - Village - Outside Lander
 *
 *--------------------------------------------------------------------------*/

void Scene4010::postInit(SceneObjectList *OwnerList) {
	loadScene(4010);
	Scene::postInit();
	setZoomPercents(0, 20, 70, 50);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerMText);
	_speakerSText._npc = &_hotspot1;
	_speakerMText._npc = &_hotspot2;
	_speakerQText._npc = &_globals->_player;

	_globals->_player.postInit();
	_globals->_player.setVisage(0);
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setPosition(Common::Point(-38, 175));
	_globals->_player.changeZoom(75);

	_hotspot2.postInit();
	_hotspot2.setVisage(2705);
	_hotspot2.animate(ANIM_MODE_1, NULL);
	_hotspot2.setObjectWrapper(new SceneObjectWrapper());
	_hotspot2._moveDiff = Common::Point(4, 2);
	_hotspot2.setPosition(Common::Point(-50, 185));
	_hotspot2.changeZoom(75);

	_hotspot1.postInit();
	_hotspot1.setVisage(2806);
	_hotspot1.setPosition(Common::Point(-20, 175));
	_hotspot1.changeZoom(75);
	_hotspot1.animate(ANIM_MODE_1, NULL);
	_hotspot1.setObjectWrapper(new SceneObjectWrapper());
	
	_globals->_player.disableControl();
	setAction(&_sequenceManager, this, 4017, &_globals->_player, &_hotspot1, &_hotspot2, NULL);
}

void Scene4010::signal() {
	_globals->_sceneManager.changeScene(4000);
}

/*--------------------------------------------------------------------------
 * Scene 4025 - Village - Puzzle Board
 *
 *--------------------------------------------------------------------------*/

void Scene4025::Action1::signal() {
	Scene4025 *scene = (Scene4025 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_armHotspot._strip = scene->_pegPtr->_armStrip;
		scene->_armHotspot._frame = 4;
		scene->_armHotspot.animate(ANIM_MODE_4, 2, -1, this);

		if (scene->_pegPtr->_armStrip > 3) {
			if (scene->_hole1._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole1._pegPtr = NULL;
			if (scene->_hole2._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole2._pegPtr = NULL;
			if (scene->_hole3._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole3._pegPtr = NULL;
			if (scene->_hole4._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole4._pegPtr = NULL;
			if (scene->_hole5._armStrip == scene->_pegPtr->_armStrip)
				scene->_hole5._pegPtr = NULL;
		}
		break;
	case 1:
		scene->_pegPtr->flag100();

		if (scene->_pegPtr2) {
			if (scene->_pegPtr->_armStrip == 3)
				scene->_pegPtr2->_strip = 2;

			scene->_pegPtr2->setPosition(scene->_pegPtr->_position);
			scene->_pegPtr2->unflag100();
			scene->_pegPtr2->_armStrip = scene->_pegPtr->_armStrip;
		}
		
		scene->_pegPtr->_armStrip = 0;
		scene->_pegPtr->setPosition(Common::Point(-10, -10));
		scene->_pegPtr2 = scene->_pegPtr;
		scene->_armHotspot.animate(ANIM_MODE_5, this);
		break;

	case 2:
		_globals->_player._uiEnabled = true;
		_globals->_events.setCursor(CURSOR_USE);
		remove();
		break;
	}
}

void Scene4025::Action2::signal() {
	Scene4025 *scene = (Scene4025 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_armHotspot._strip = scene->_holePtr->_armStrip;
		scene->_armHotspot.animate(ANIM_MODE_4, 2, -1, this);
		break;
	case 1:
		if (!scene->_pegPtr2) {
			// Getting a peg from a hole
			scene->_holePtr->_pegPtr->flag100();
			scene->_pegPtr = scene->_holePtr->_pegPtr;
			scene->_pegPtr->_armStrip = 0;
			scene->_pegPtr->setPosition(Common::Point(-10, -10));
			scene->_pegPtr2 = scene->_holePtr->_pegPtr;
			scene->_holePtr->_pegPtr = NULL;
		} else {
			// Placing a peg into a hole
			scene->_pegPtr2 = NULL;
			if (scene->_holePtr->_pegPtr) {
				scene->_holePtr->_pegPtr->flag100();
				scene->_pegPtr2 = scene->_holePtr->_pegPtr;
			}

			assert(scene->_pegPtr);
			scene->_pegPtr->setPosition(scene->_holePtr->_newPosition);
			scene->_pegPtr->setStrip(1);
			scene->_pegPtr->unflag100();
			scene->_pegPtr->_armStrip = scene->_holePtr->_armStrip;
			
			scene->_holePtr->_pegPtr = scene->_pegPtr;
			scene->_pegPtr = scene->_pegPtr2;
		}
		scene->_armHotspot.animate(ANIM_MODE_5, this);
		break;
	case 2:
		_globals->_player._uiEnabled = true;
		_globals->_events.setCursor(CURSOR_USE);
		remove();
		break;
	}
}

void Scene4025::Action3::signal() {
	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		_globals->_scenePalette.addRotation(64, 111, -1);
		setDelay(120);
		break;
	case 1:
		_globals->clearFlag(34);
		_globals->_stripNum = 4025;
		_globals->_sceneManager.changeScene(4000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4025::Hole::synchronise(Serialiser &s) {
	SceneObject::synchronise(s);
	SYNC_POINTER(_pegPtr);
	s.syncAsSint16LE(_armStrip);
	s.syncAsSint16LE(_newPosition.x);
	s.syncAsSint16LE(_newPosition.y);
}

void Scene4025::Hole::doAction(int action) {
	Scene4025 *scene = (Scene4025 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4025, 3);
		break;
	case CURSOR_USE:
		if (!scene->_pegPtr && !_pegPtr) {
			setAction(&scene->_sequenceManager, scene, 4028, NULL);
		} else {
			_globals->_player.disableControl();
			scene->_holePtr = this;
			scene->setAction(&scene->_action2);
		}
		break;
	case OBJECT_PEG:
		if (!scene->_pegPtr2) {
			_globals->_player.disableControl();
			_globals->_events.setCursor(CURSOR_USE);
			_globals->_inventory._peg._sceneNumber = 4025;

			scene->_pegPtr = &scene->_peg5;
			scene->_holePtr = this;
			scene->_pegPtr->_armStrip = 0;
			scene->_pegPtr2 = scene->_pegPtr;

			scene->setAction(&scene->_action2);
		} else {
			scene->_sceneMode = 4027;
			scene->setAction(&scene->_sequenceManager, scene, 4027, NULL);
		}
		break;
	}
}

void Scene4025::Peg::synchronise(Serialiser &s) {
	SceneObject::synchronise(s);
	s.syncAsSint16LE(_field88);
	SYNC_POINTER(_armStrip);
}

void Scene4025::Peg::doAction(int action) {
	Scene4025 *scene = (Scene4025 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4025, 1);
		break;
	case CURSOR_USE:
		_globals->_player.disableControl();
		scene->_pegPtr = this;
		scene->setAction(&scene->_action1);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4025::Scene4025(): Scene() {
	_holePtr = NULL;
	_pegPtr = _pegPtr2 = NULL;
}

void Scene4025::postInit(SceneObjectList *OwnerList) {
	loadScene(4025);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_globals->_events.setCursor(CURSOR_USE);
	_pegPtr = _pegPtr2 = NULL;

	_peg1.postInit();
	_peg1._field88 = 1;
	_peg1.setVisage(4025);
	_peg1.setStrip(2);
	_peg1.setFrame(1);
	_peg1.setPosition(Common::Point(203, 61));

	_peg2.postInit();
	_peg2._field88 = 4;
	_peg2.setVisage(4025);
	_peg2.setStrip(2);
	_peg2.setFrame(2);
	_peg2.setPosition(Common::Point(195, 57));

	_peg3.postInit();
	_peg3._field88 = 0;
	_peg3.setVisage(4025);
	_peg3.setStrip(2);
	_peg3.setFrame(3);
	_peg3.setPosition(Common::Point(202, 66));

	_peg4.postInit();
	_peg4._field88 = 3;
	_peg4.setVisage(4025);
	_peg4.setStrip(2);
	_peg4.setFrame(4);
	_peg4.setPosition(Common::Point(194, 68));

	_peg5.postInit();
	_peg5._field88 = 2;
	_peg5.setVisage(4025);
	_peg5.setStrip(1);
	_peg5.setFrame(5);
	_peg5.flag100();

	_hole1.postInit();
	_hole1.setVisage(4025);
	_hole1.setStrip(1);
	_hole1.setFrame2(6);
	_hole1.setPosition(Common::Point(123, 51));
	_hole1._pegPtr = NULL;
	_hole1._newPosition = Common::Point(123, 44);
	_hole1._armStrip = 8;

	_hole2.postInit();
	_hole2.setVisage(4025);
	_hole2.setStrip(1);
	_hole2.setFrame2(7);
	_hole2.setPosition(Common::Point(167, 51));
	_hole2._pegPtr = NULL;
	_hole2._newPosition = Common::Point(166, 44);
	_hole2._armStrip = 7;

	_hole3.postInit();
	_hole3.setVisage(4025);
	_hole3.setStrip(1);
	_hole3.setFrame2(8);
	_hole3.setPosition(Common::Point(145, 69));
	_hole3._pegPtr = NULL;
	_hole3._newPosition = Common::Point(145, 60);
	_hole3._armStrip = 6;

	_hole4.postInit();
	_hole4.setVisage(4025);
	_hole4.setStrip(1);
	_hole4.setFrame2(6);
	_hole4.setPosition(Common::Point(123, 87));
	_hole4._pegPtr = NULL;
	_hole4._newPosition = Common::Point(123, 80);
	_hole4._armStrip = 5;

	_hole5.postInit();
	_hole5.setVisage(4025);
	_hole5.setStrip(1);
	_hole5.setFrame2(10);
	_hole5.setPosition(Common::Point(167, 87));
	_hole5._pegPtr = NULL;
	_hole5._newPosition = Common::Point(166, 80);
	_hole5._armStrip = 4;

	_hole1.setPriority2(1);
	_hole2.setPriority2(1);
	_hole3.setPriority2(1);
	_hole4.setPriority2(1);
	_hole5.setPriority2(1);

	_armHotspot.postInit();
	_armHotspot.setVisage(4025);
	_armHotspot.setPosition(Common::Point(190, 161));
	_armHotspot.setStrip(3);
	_armHotspot.setFrame(4);

	_globals->_sceneItems.addItems(&_hole1, &_hole2, &_hole3, &_hole4, &_hole5,
		&_peg1, &_peg2, &_peg3, &_peg4, &_peg5, NULL);

	_globals->_player._uiEnabled = true;
	_globals->_player.disableControl();
	setAction(&_sequenceManager, this, 4026, NULL);
}

void Scene4025::synchronise(Serialiser &s) {
	Scene::synchronise(s);
	SYNC_POINTER(_pegPtr);
	SYNC_POINTER(_pegPtr2);
	SYNC_POINTER(_holePtr);
}

void Scene4025::remove() {
	_globals->_scenePalette.clearListeners();
	Scene::remove();
}

void Scene4025::signal() {
	if (_sceneMode != 4027) {
		if (_sceneMode != 4028) {
			_gfxButton.setText(EXIT_MSG);
			_gfxButton._bounds.centre(144, 107);
			_gfxButton.draw();
			_gfxButton._bounds.expandPanes();
		}

		_globals->_player._uiEnabled = true;
	}

	_globals->_events.setCursor(CURSOR_USE);
}

void Scene4025::process(Event &event) {
	Scene::process(event);

	if (_gfxButton.process(event)) {
		if (_globals->_inventory._peg._sceneNumber == 4025)
			_globals->_inventory._peg._sceneNumber = 1;

		_globals->_sceneManager.changeScene(4000);
	}
}

void Scene4025::dispatch() {
	if (!_action && (_peg1._armStrip == 7) && (_peg2._armStrip == 4) && (_peg3._armStrip == 8) &&
			(_peg4._armStrip == 5) && (_peg5._armStrip == 6))
		setAction(&_action3);

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 4045 - Village - Temple Antechamber
 *
 *--------------------------------------------------------------------------*/

void Scene4045::Action1::signal() {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(60);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(4040, this, scene);
		break;
	case 2:
		scene->_hotspot5.animate(ANIM_MODE_NONE, NULL);
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 3:
		setDelay(60);
		break;
	case 4:
		scene->_hotspot4.animate(ANIM_MODE_6, this);
		break;
	case 5:
		ADD_MOVER(_globals->_player, 150, 300);
		break;
	case 6:
		_globals->_stripNum = 4000;
		_globals->_sceneManager.changeScene(4000);
		remove();
		break;
	}
}

void Scene4045::Action2::signal() {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(15);
		break;
	case 1:
		scene->_stripManager.start(_globals->_stripNum, this, scene);
		break;
	case 2:
		scene->_hotspot5.animate(ANIM_MODE_NONE, NULL);
		setDelay(10);
		break;
	case 3:
		if (_globals->getFlag(38)) {
			_globals->_player.enableControl();
			remove();
		} else {
			ADD_MOVER(_globals->_player, 150, 300);
		}
		break;
	case 4:
		_globals->setFlag(39);
		_globals->_sceneManager.changeScene(4000);
		break;
	}
}		

void Scene4045::Action3::signal() {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		scene->_hotspot4.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_stripManager.start(4504, this, scene);
		break;
	case 2:
		scene->_hotspot4.animate(ANIM_MODE_6, NULL);
		scene->_hotspot5.animate(ANIM_MODE_NONE, NULL);
		ADD_MOVER(_globals->_player, 91, 1264);
		break;
	case 3:
		_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene4045::Hotspot1::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(4045, 19);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(4045, (_strip == 1) ? 5 : 14);
		break;
	case CURSOR_USE:
		SceneItem::display2(4045, 18);
		break;
	case CURSOR_TALK:
		if (_strip == 5) {
			setStrip(6);
			animate(ANIM_MODE_NONE, 0);
		}
		if (_globals->_player._position.y < 135) {
			scene->_sceneMode = 4046;
			_numFrames = 10;
			_globals->_player.disableControl();

			scene->setAction(&scene->_sequenceManager, this, 4046, &_globals->_player, this, NULL);
		} else {
			if (!_globals->getFlag(31)) {
				_globals->setFlag(31);
				_globals->_stripNum = 4080;
			} else if (!_globals->getFlag(38))
				_globals->_stripNum = 4060;
			else 
				_globals->_stripNum = 4503;
			
			scene->setAction(&scene->_action2);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4045::Hotspot2::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4045, 9);
		break;
	case CURSOR_TALK:
		scene->_sceneMode = 4102;
		_globals->_player.disableControl();
		scene->setAction(&scene->_sequenceManager, scene, 4104, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene4045::Hotspot6::doAction(int action) {
	Scene4045 *scene = (Scene4045 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(4045, 17);
		break;
	case CURSOR_USE:
		if (_globals->_player._position.y < 135) {
			SceneItem::display2(4045, 16);
			_globals->_inventory._peg._sceneNumber = 1;
			_globals->_events.setCursor(CURSOR_WALK);
			remove();
		} else {
			scene->_sceneMode = 4047;
			_globals->_player.disableControl();
			scene->setAction(&scene->_sequenceManager, scene, 4047, &_globals->_player, &scene->_hotspot1, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene4045::Scene4045(): 
		_hotspot3(0, CURSOR_LOOK, 4045, 1, CURSOR_USE, 4100, 21, LIST_END),
		_hotspot7(9, CURSOR_LOOK, 4045, 0, CURSOR_USE, 4045, 15, LIST_END),
		_hotspot8(10, CURSOR_LOOK, 4045, 2, LIST_END),
		_hotspot9(11, CURSOR_LOOK, 4045, 3, CURSOR_USE, 4045, 15, LIST_END),
		_hotspot10(12, CURSOR_LOOK, 4045, 4, CURSOR_USE, 4045, 19, LIST_END),
		_hotspot11(13, CURSOR_LOOK, 4045, 6, CURSOR_USE, 4045, 15, LIST_END),
		_hotspot12(14, CURSOR_LOOK, 4045, 7, CURSOR_USE, 4045, 29, LIST_END),
		_hotspot13(15, CURSOR_LOOK, 4045, 8, CURSOR_USE, 4045, 19, LIST_END),
		_hotspot14(0, CURSOR_LOOK, 4045, 10, LIST_END) {

	_hotspot14.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
}

void Scene4045::postInit(SceneObjectList *OwnerList) {
	loadScene(4045);
	Scene::postInit();
	setZoomPercents(100, 60, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerML);
	_stripManager.addSpeaker(&_speakerPR);
	_stripManager.addSpeaker(&_speakerPText);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.setCallback(this);

	_speakerQText._textPos.y = 140;

	_hotspot3.postInit();
	_hotspot3.setVisage(4045);
	_hotspot3.setPosition(Common::Point(47, 111));
	_hotspot3.animate(ANIM_MODE_2, NULL);
	_hotspot3.setPriority2(156);
	_globals->_sceneItems.push_back(&_hotspot3);

	_globals->_player.postInit();
	_globals->_player.setVisage(4200);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player._moveDiff = Common::Point(7, 4);

	_hotspot1.postInit();
	_hotspot1.setVisage(4051);

	_hotspot5.postInit();
	_hotspot5.setVisage(4051);
	_hotspot5.setStrip(4);
	_hotspot5.setPriority2(152);

	if(_globals->_sceneManager._previousScene == 4050) {
		_globals->_soundHandler.startSound(155);
		_globals->_player.setPosition(Common::Point(72, 128));
		_globals->_player.enableControl();

		_hotspot1.setStrip(5);
		_hotspot1.setPosition(Common::Point(173, 99));
		_hotspot1._numFrames = 1;
		_hotspot1.animate(ANIM_MODE_2, NULL);

		_hotspot5.setPosition(Common::Point(177, 40));

		if (_globals->_inventory._peg._sceneNumber == 4045) {
			_hotspot6.postInit();
			_hotspot6.setVisage(4045);
			_hotspot6.setStrip(2);
			_hotspot6.setPosition(Common::Point(108, 82));
			_globals->_sceneItems.push_back(&_hotspot6);
		}
	} else {
		_globals->_player.setPosition(Common::Point(108, 192));
		_globals->_player.setStrip(4);

		if (!_globals->getFlag(36) && !_globals->getFlag(43)) {
			_hotspot2.postInit();
			_hotspot2.setVisage(4102);
			_hotspot2.animate(ANIM_MODE_NONE, NULL);
			_hotspot2.setStrip(3);
			_hotspot2.changeZoom(-1);
			_hotspot2.setPosition(Common::Point(66, 209));
			_globals->_sceneItems.push_back(&_hotspot2);
		}

		if (_globals->getFlag(31)) {
			_hotspot1.setVisage(4051);
			_hotspot1.setStrip(5);
			_hotspot1.setPosition(Common::Point(173, 99));
			_hotspot1._numFrames = 1;
			_hotspot1.animate(ANIM_MODE_2, NULL);

			_hotspot5.setPosition(Common::Point(177, 40));
			
			if (_globals->_inventory._peg._sceneNumber == 4045) {
				_hotspot6.postInit();
				_hotspot6.setVisage(4045);
				_hotspot6.setStrip(2);
				_hotspot6.setPosition(Common::Point(108, 82));
				_globals->_sceneItems.push_back(&_hotspot6);
			}
		} else {
			_hotspot1.setPosition(Common::Point(186, 149));

			_hotspot4.postInit();
			_hotspot4.setVisage(4051);
			_hotspot4.setStrip(2);
			_hotspot4.setPriority2(152);
			_hotspot4.setPosition(Common::Point(202, 80));

			_hotspot5.setPosition(Common::Point(192, 77));
			_globals->setFlag(31);
			setAction(&_action1);

			_globals->_player.disableControl();
		}
	}

	_globals->_sceneItems.addItems(&_hotspot1, &_hotspot7, &_hotspot8, &_hotspot9, &_hotspot10,
		&_hotspot13, &_hotspot11, &_hotspot12, &_hotspot14, NULL);
}

void Scene4045::stripCallback(int v) {
	switch (v) {
	case 1:
		_hotspot5.animate(ANIM_MODE_7, 0, NULL);
		break;
	case 2:
		_hotspot5.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene4045::signal() {
	switch (_sceneMode) {
	case 4046:
	case 4047:
		_hotspot5.animate(ANIM_MODE_NONE, NULL);
		break;
	case 4050:
		_globals->_sceneManager.changeScene(4000);
		break;
	case 4102:
		_globals->_player.enableControl();
		break;
	}
}

void Scene4045::dispatch() {
	if (!_action) {
		if (_globals->_player.getRegionIndex() == 8) {
			_globals->_player.addMover(NULL);
			if (_hotspot1._strip == 1) {
				_globals->_player.disableControl();
				_sceneMode = 4046;
				_hotspot1._numFrames = 10;
				setAction(&_sequenceManager, this, 4046, &_globals->_player, &_hotspot1, NULL);
			} else {
				setAction(&_action3);
			}
		}

		if (_globals->_player.getRegionIndex() == 10)
			_globals->_sceneManager.changeScene(4050);
		if (_globals->_player._position.y >= 196) {
			_sceneMode = 4050;
			_globals->_player.disableControl();
			setAction(&_sequenceManager, this, 4105, &_globals->_player, NULL);
		}
	}

	Scene::dispatch();
}

} // End of namespace tSage
