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

#include "tsage/ringworld_scenes3.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 2000 - Cockpit cutscenes
 *
 *--------------------------------------------------------------------------*/

void Scene2000::Action1::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2075, this);
		break;
	case 2:
		setDelay(4);
		break;
	case 3:
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(1250);
		break;
	}
}

void Scene2000::Action2::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object2.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		if (_globals->_randomSource.getRandomNumber(4) >= 2)
			_actionIndex = 0;
		break;
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = _globals->_randomSource.getRandomNumber(1);
		break;
	}
}

void Scene2000::Action3::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object6.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object6.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action4::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object4.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object4.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action5::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(125) + 300);
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(125) + 300);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action6::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(130);
		break;
	case 1:
		scene->_soundHandler2.startSound(79);
		scene->_stripManager.start(2000, this);
		break;
	case 2:
		_globals->_soundHandler.startSound(81);
		scene->_object6.postInit();
		scene->_object6.setVisage(2003);
		scene->_object6.setAction(NULL);
		scene->_object6.setStrip2(2);
		scene->_object6.setPosition(Common::Point(184, 137));
		scene->_object6.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_stripManager.start(95, this);
		break;
	case 4:
		scene->_object6.animate(ANIM_MODE_6, this);
		break;
	case 5:
		_globals->_soundHandler.startSound(80);
		scene->_object6.remove();
		_globals->_sceneManager.changeScene(1001);
		break;
	}
}

void Scene2000::Action7::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2072, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2222);
		break;
	}
}

void Scene2000::Action8::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2073, this);
		break;
	case 2:
		setDelay(10);
		break;
	case 3:
		_globals->_stripNum = 2005;
		_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene2000::Action9::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2074, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_stripNum = 2008;
		_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action10::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		error("Old stuff");
		break;
	case 2:
		SceneItem::display(2000, 17, SET_Y, 20, SET_X, 110, SET_FONT, 2, SET_BG_COLOUR, -1,
			SET_FG_COLOUR, 17, SET_WIDTH, 200, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, 1, LIST_END);
		break;
	case 3:
		SceneItem::display(0, 0);
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action11::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2076, this);
		break;
	case 2:
		scene->_stripManager.start(2077, this);
		break;
	case 3:
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(1400);
		break;
	}
}

void Scene2000::Action12::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2020, this);
		break;
	case 2:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2300);
		break;
	}
}

void Scene2000::Action13::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2078, this);
		break;
	case 2:
		SceneItem::display(0, 0);
		_globals->_stripNum = 2751;
		_globals->_sceneManager.changeScene(1500);
		break;
	}
}

void Scene2000::Action14::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2070, this);
		break;
	case 2:
		setDelay(60);
		break;
	case 3:
		_globals->_soundHandler.startSound(99);
		scene->_object8.unflag100();
		scene->_object8.animate(ANIM_MODE_5, this);
		break;
	case 4:
		_globals->_soundHandler.startSound(12);
		scene->_object8.setStrip(2);
		scene->_object8.setFrame(1);
		scene->_object9.unflag100();
		scene->_object10.unflag100();
		setDelay(60);
		break;
	case 5:
		scene->_stripManager.start(2001, this, scene);
		break;
	case 6:
		_globals->_soundHandler.proc1(false);
		scene->_object8.setStrip(1);
		scene->_object8.setFrame(scene->_object8.getFrameCount());
		scene->_object8.animate(ANIM_MODE_6, this);

		scene->_object9.remove();
		scene->_object10.remove();
		break;
	case 7:
		_globals->_soundHandler.startSound(111);
		scene->_object8.remove();
		setDelay(5);
		break;
	case 8:
		scene->_stripManager.start(2071, this);
		break;
	case 9:
		_globals->_stripNum = 1250;
		_globals->_sceneManager.changeScene(1000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2000::postInit(SceneObjectList *OwnerList) {
	loadScene(2000);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerHText);

	_speakerQText._npc = &_object2;
	_speakerSText._npc = &_object3;
	_speakerMText._npc = &_object6;
	_speakerHText._npc = &_object6;
	_stripManager.setCallback(this);
	
	_object3.postInit();
	_object3.setVisage(2002);
	_object3.setPosition(Common::Point(65, 199));
	
	_object4.postInit();
	_object4.setVisage(2002);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(125, 199));
	_object4.setAction(&_action4);

	_object2.postInit();
	_object2.setVisage(2001);
	_object2.setPosition(Common::Point(43, 115));
	_object2.setAction(&_action2);

	_globals->_player.disableControl();

	_object6.postInit();
	_object6.setVisage(2003);
	_object6.setPosition(Common::Point(267, 170));
	_object6.setAction(&_action3);

	_object8.postInit();
	_object8.setVisage(2005);
	_object8.setPosition(Common::Point(169, 133));
	_object8.setPriority(133);
	_object8.flag100();

	_object9.postInit();
	_object9.setVisage(2005);
	_object9.setStrip2(3);
	_object9.setFrame(4);
	_object9.setPosition(Common::Point(136, 86));
	_object9.setPriority2(190);
	_object9.flag100();

	_object10.postInit();
	_object10.setVisage(2005);
	_object10.setStrip2(5);
	_object10.setFrame(4);
	_object10.setPosition(Common::Point(202, 86));
	_object10.setPriority2(195);
	_object10.flag100();

	switch (_globals->_sceneManager._previousScene) {
	case 1000:
		setAction(&_action7);
		break;
	case 1001:
		_object6.remove();
		setAction(&_action12);
		break;
	case 1500:
		setAction(&_action13);
		break;
	case 2200:
		_globals->_soundHandler.startSound(111);
		setAction(&_action14);
		break;
	case 2222:
		_globals->_soundHandler.startSound(115);
		setAction(&_action8);
		break;
	case 3500:
		setAction(&_action11);
		break;
	default:
		_object6.remove();
		_globals->_soundHandler.startSound(80);
		setAction(&_action6);
		break;
	}

	_soundHandler1.startSound(78);
	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2000::stripCallback(int v) {
	switch (v) {
	case 0:
		_object9.setStrip(3);
		_object9.animate(ANIM_MODE_7, NULL);
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		break;
	case 1:
		_object10.setStrip(5);
		_object10.animate(ANIM_MODE_7, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		_object9.animate(ANIM_MODE_NONE, NULL);
		break;
	case 3:
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	}
}

} // End of namespace tSage
