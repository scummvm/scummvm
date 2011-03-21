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
			setAction(&_sequenceManager1, this, &_globals->_player, NULL);

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

} // End of namespace tSage
