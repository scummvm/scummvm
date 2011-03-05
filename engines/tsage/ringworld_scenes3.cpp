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

/*--------------------------------------------------------------------------
 * Scene 2100 - Cockpit
 *
 *--------------------------------------------------------------------------*/

void Scene2100::Action1::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		if (!scene->_field1800)
			setDelay(1);
		else {
			setAction(&scene->_sequenceManager, this, 2102, &_globals->_player, NULL);
			scene->_field1800 = 0;
		}
		break;
	case 1: {
		Common::Point pt(157, 62);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt(157, 56);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4:
		_globals->_player._strip = 3;
		setDelay(3);
		break;
	case 5:
		_globals->_player.setPriority2(1);
		scene->_area1.display();
		scene->_area2.display();
		scene->_area3.display();
		scene->_area4.display();

		scene->_area1.draw(true);
		_state = 0;
		_globals->_events.setCursor(CURSOR_USE);

		while (!_state && !_vm->getEventManager()->shouldQuit()) {
			// Wait for an event
			Event event;
			if (!_globals->_events.getEvent(event)) {
				g_system->updateScreen();
				g_system->delayMillis(10);
				continue;
			}

			if (scene->_area1._bounds.contains(event.mousePos)) {
				scene->_area1.draw(true);
				_state = scene->_area1._actionId;
			}
			if (scene->_area2._bounds.contains(event.mousePos)) {
				scene->_area1.draw(false);
				scene->_area2.draw(true);
				_state = scene->_area2._actionId;
			}
			if (scene->_area3._bounds.contains(event.mousePos)) {
				scene->_area1.draw(false);
				scene->_area3.draw(true);
				_state = scene->_area3._actionId;
			}
		}

		scene->_soundHandler.startSound(161);
		scene->_area1.restore();
		scene->_area2.restore();
		scene->_area3.restore();
		scene->_area4.restore();

		if (_state == 2100) {
			setDelay(1);
		} else {
			scene->_soundHandler.startSound(162);
			scene->_object1.animate(ANIM_MODE_6, this);
		}
		

		//TODO
		break;
	case 6:
		if (_state == 2100) {
			Common::Point pt(157, 65);
			NpcMover *mover = new NpcMover();
			_globals->_player.addMover(mover, &pt, this);
			break;
		} else {
			_globals->_sceneManager.changeScene(_state);
		}
		break;
	case 7:
		_globals->_player.setPriority2(-1);
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 8:
		_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene2100::Action2::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3._numFrames = 5;
		setDelay(_globals->_randomSource.getRandomNumber(59));
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(59));
		break;
	case 3:
		scene->_object3.animate(ANIM_MODE_6, this);
		_actionIndex = 0;
		break;
	}
}

void Scene2100::Action3::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(119));
		break;
	case 1:
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object2.animate(ANIM_MODE_6, this);
		_actionIndex = 0;
		break;
	}
}

void Scene2100::Action4::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		if (!scene->_field1800)
			setDelay(1);
		else
			setAction(&scene->_sequenceManager, this, 2102, &_globals->_player, NULL);
		break;
	case 1: {
		Common::Point pt(80, 66);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, NULL);
		break;
	}
	case 2:
		_globals->_player.setVisage(2109);
		_globals->_player._frame = 1;
		_globals->_player._strip = 2;
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2120);
		break;
	}
}

void Scene2100::Action5::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, &scene->_object1, NULL);
		break;
	case 2: {
		Common::Point pt(272, 127);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		_globals->_player.checkAngle(&scene->_object3);
		setDelay(30);
		break;
	case 4:
		_globals->_sceneManager.changeScene(3700);
		break;
	}
}

void Scene2100::Action6::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_object2.setVisage(2806);
		scene->_object2.setStrip(1);
		scene->_object2.setStrip2(-1);
		scene->_object2.changeZoom(-1);
		scene->_object2.setPosition(Common::Point(155, 116));
		scene->_object2.setObjectWrapper(new SceneObjectWrapper());
		scene->_object2.setAction(NULL);
		scene->_object2.animate(ANIM_MODE_1, NULL);

		Common::Point pt(130, 116);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, NULL);
		break;
	}
	case 1: {
		scene->_object2.setPriority2(-1);
		Common::Point pt(153, 67);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, NULL);
		break;
	}
	case 2:
		remove();
		break;
	}
}

void Scene2100::Action7::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, NULL);
		break;
	case 3:
		_globals->_sceneManager.changeScene(8100);
		break;
	}
}

void Scene2100::Action8::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, &scene->_object1, NULL);
		break;
	case 2: {
		Common::Point pt(200, 174);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		_globals->_player.checkAngle(&scene->_object3);
		scene->_stripManager.start((_globals->_inventory._translator._sceneNumber == 1) ? 7720 : 7710, this);
		break;
	case 4:
		if (_globals->_inventory._translator._sceneNumber != 1)
			_globals->_sceneManager.changeScene(7600);
		else {
			_globals->setFlag(24);
			_globals->_player.enableControl();
			remove();
		}
		break;
	}
}

void Scene2100::Action9::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_stripManager.start(6050, this);
		break;
	case 2:
		scene->_soundHandler.startSound(99);
		scene->_object4.unflag100();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_soundHandler.startSound(12);
		scene->_object4.setStrip(2);
		scene->_stripManager.start(6051, this, scene);
		break;
	case 4:
		scene->_soundHandler.proc1(false);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 5:
		scene->_object4.flag100();
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(6010, this);
		break;
	case 6:
		if (scene->_stripManager._field2E8 != 165)
			setAction(&scene->_action10);
		else
			setAction(&scene->_action11);
		break;
	}
}

void Scene2100::Action10::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2102, &_globals->_player, NULL);
		break;
	case 2: {
		_globals->_player.disableControl();
		Common::Point pt(155, 64);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 4: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.setPriority2(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(166, 64);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);

		setAction(&scene->_action6, NULL);
		break;
	}		
	case 5:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		_globals->_player.setPriority2(1);
		Common::Point pt(144, 54);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7: {
		scene->_object3.setPriority2(2);
		Common::Point pt1(163, 55);
		NpcMover *mover1 = new NpcMover();
		scene->_object3.addMover(mover1, &pt1, NULL);
		
		scene->_object2.setPriority2(2);
		Common::Point pt2(158, 55);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 8:
		_globals->_player.setPriority2(1);
		_globals->_player.setStrip(1);
		scene->_object3.setPriority2(1);
		scene->_object3.setStrip(2);
		scene->_object2.setPriority2(2);
		scene->_object2.setStrip(3);
		
		setDelay(45);
		break;
	case 9:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 10:
		_globals->setFlag(70);
		_globals->_stripNum = 2101;
		_globals->_sceneManager.changeScene(2320);
		break;
	}
}

void Scene2100::Action11::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.setPriority2(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(158, 62);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);

		setAction(&scene->_action6, NULL);
		break;
	}
	case 3:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 4: {
		scene->_object3.setPriority2(1);
		Common::Point pt1(163, 55);
		NpcMover *mover1 = new NpcMover();
		scene->_object3.addMover(mover1, &pt1, NULL);

		scene->_object2.setPriority2(1);
		Common::Point pt2(158, 55);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 5:
		scene->_object3.setStrip(2);
		scene->_object2.setStrip(3);
		setDelay(45);
		break;
	case 6:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_object3.remove();
		scene->_object2.remove();
		_globals->setFlag(70);
		_globals->_stripNum = 2102;
		_globals->_player.enableControl();
		_globals->_player._canWalk = false;
		break;
	}
}

void Scene2100::Action12::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(6000, this);
		break;
	case 2:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt1(158, 74);
		NpcMover *mover1 = new NpcMover();
		_globals->_player.addMover(mover1, &pt1, this);

		Common::Point pt2(158, 68);
		NpcMover *mover2 = new NpcMover();
		scene->_object2.addMover(mover2, &pt2, NULL);
		break;
	}
	case 4: {
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, NULL);

		_globals->_player.setPriority2(-1);
		Common::Point pt1(277, 84);
		PlayerMover *mover1 = new PlayerMover();
		_globals->_player.addMover(mover1, &pt1, this);

		scene->_object2.setPriority2(-1);
		Common::Point pt2(255, 76);
		PlayerMover *mover2 = new PlayerMover();
		scene->_object2.addMover(mover2, &pt2, this);
		break;
	}
	case 6:
		_globals->_player.setStrip(4);
		scene->_object2.setStrip(4);
		setDelay(60);
		break;
	case 7:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(6052, this);
		break;
	case 8:
		if (scene->_stripManager._field2E8 == 320)
			_globals->setFlag(74);
		setDelay(30);
		break;
	case 9:
		_globals->_events.setCursor(OBJECT_STUNNER);
		scene->_object2.setAction(&scene->_action13);
		setDelay(60);
		break;
	case 10:
		if (_globals->getFlag(74))
			setDelay(1);
		break;
	case 11:
		scene->_stripManager.start(2170, this);
		break;
	case 12:
		setDelay(5);
		break;
	case 13:
		scene->_stripManager.start(_globals->getFlag(74) ? 2172 : 2174, this);
		break;
	case 14:
		if (_globals->getFlag(74)) {
			_globals->_stripNum = 6100;
			_globals->_sceneManager.changeScene(2320);
		} else {
			_globals->_sceneManager.changeScene(6100);
		}
		remove();
		break;
	}
}

void Scene2100::Action13::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1: {
		scene->_object2.setPriority2(113);
		Common::Point pt(178, 116);
		PlayerMover *mover = new PlayerMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		Common::Point pt(150, 116);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 3:
		scene->_object2.setVisage(2108);
		scene->_object2._strip = 3;
		scene->_object2.setPosition(Common::Point(150, 100));
		scene->_object2.animate(ANIM_MODE_NONE, NULL);
		scene->_object2.changeZoom(100);
		scene->_object2.setAction(&scene->_action3);
		setDelay(15);
		break;
	case 4:
		remove();
		break;
	}
}

void Scene2100::Action14::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2101, &_globals->_player, NULL);
		break;
	case 3:
		scene->_stripManager.start(6008, this);
		break;
	case 4:
		scene->_soundHandler.startSound(99);
		scene->_object4.unflag100();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_object4.setStrip(2);
		scene->_stripManager.start(6009, this, scene);
		break;
	case 6:
		scene->_soundHandler.proc1(false);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_stripManager.start(6060, this);
		break;
	case 8:
		scene->_object3._numFrames = 10;
		scene->_object3.setAction(NULL);
		scene->_object3.setVisage(2105);
		scene->_object3.setStrip(2);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 9: {
		scene->_object3.setVisage(2705);
		scene->_object3.setStrip2(-1);
		scene->_object3.changeZoom(-1);
		scene->_object3.setPriority2(-1);
		scene->_object3.setPosition(Common::Point(260, 156));
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.animate(ANIM_MODE_1, NULL);

		Common::Point pt(157, 65);
		PlayerMover *mover = new PlayerMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 10:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 11: {
		Common::Point pt(159, 51);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 12:
		scene->_object3.setStrip(2);
		setDelay(30);
	case 13:
		scene->_object3.setPriority2(1);
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 14:
		setDelay(90);
		break;
	case 15:
		_globals->_sceneManager.changeScene(7000);
		remove();
		break;
	}
}

void Scene2100::Action15::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		scene->_object3.postInit();
		scene->_object3.setVisage(2705);
		scene->_object3.animate(ANIM_MODE_1, NULL);
		scene->_object3.setObjectWrapper(new SceneObjectWrapper());
		scene->_object3.setPosition(Common::Point(157, 56));
		scene->_object3.setPriority2(1);
		scene->_object3.changeZoom(-1);

		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		scene->_object3.setPriority2(-1);
		Common::Point pt(177, 68);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		
		Common::Point pt(272, 140);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		Common::Point pt(266, 150);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		scene->_object3.setPriority2(156);

		Common::Point pt(260, 156);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 6:
		scene->_object3.setVisage(2105);
		scene->_object3._strip = 1;
		scene->_object3._frame = 1;
		scene->_object3.setPosition(Common::Point(256, 156));
		scene->_object3.animate(ANIM_MODE_5, this);
		scene->_object3.changeZoom(100);

		scene->_object3.animate(ANIM_MODE_NONE, NULL);
		break;
	case 7:
		remove();
		break;
	}
}

void Scene2100::Action16::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 4:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(7001, this);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2102, &_globals->_player, NULL);
		break;
	case 3: {
		_globals->_player.disableControl();
		Common::Point pt(155, 63);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		Common::Point pt(160, 54);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		_globals->_player.setPriority2(1);
		_globals->_player.setStrip(3);
		setDelay(45);
		break;
	case 8:
		scene->_soundHandler.startSound(162);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 9:
		_globals->setFlag(15);
		_globals->setFlag(36);
		_globals->_sceneManager.changeScene(7000);
		remove();
		break;
	}
}

void Scene2100::Action17::signal() {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		setAction(&scene->_sequenceManager, this, 2104, &_globals->_player, &scene->_object1, NULL);
		break;
	case 2:
		setAction(&scene->_sequenceManager, this, 2101, &_globals->_player, NULL);
		break;
	case 3:
		scene->_stripManager.start(7070, this);
		break;
	case 4:
		scene->_soundHandler.startSound(99);
		scene->_object4.unflag100();
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 5:
		scene->_soundHandler.startSound(12);
		scene->_object4.setStrip(2);
		scene->_stripManager.start(7071, this, scene);
		break;
	case 6:
		scene->_soundHandler.proc1(NULL);
		scene->_object4.setStrip(1);
		scene->_object4.setFrame(scene->_object4.getFrameCount());
		scene->_object4.animate(ANIM_MODE_6, this);
		break;
	case 7:
		scene->_stripManager.start(7072, this);
		break;
	case 8:
		_globals->_inventory._stasisNegator._sceneNumber = 1;
		_globals->_sceneManager.changeScene(9100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2100::Hotspot2::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 3);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot3::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 4);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot4::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 5);
		break;
	case CURSOR_USE:
		SceneItem::display2(2100, 6);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot8::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 12);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			_globals->_player.disableControl();
			scene->setAction(&scene->_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot10::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 13);
		break;
	case CURSOR_USE:
		if (scene->_field1800) {
			_globals->_player.disableControl();
			scene->_sceneMode = 2102;
			scene->setAction(&scene->_sequenceManager, scene, 2102, &_globals->_player, NULL);
		} else if (_globals->getFlag(13)) {
			SceneItem::display2(2100, 28);
		} else {
			scene->setAction(&scene->_sequenceManager, scene, 2101, &_globals->_player, NULL);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Hotspot14::doAction(int action) {
	switch (action) {
	case CURSOR_LOOK:
		if (_globals->getFlag(0))
			SceneItem::display2(2100, 19);
		else
			SceneItem::display2(2100, 18);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(1))
			SceneItem::display2(2100, 21);
		else
			SceneItem::display2(2100, 20);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object1::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 1);
		break;
	case CURSOR_USE:
		scene->setAction(&scene->_action4);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object2::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 30);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(72)) {
			_globals->_player.disableControl();
			if (!_globals->getFlag(52))
				scene->setAction(&scene->_sequenceManager, scene, 2111, NULL);
			else {
				scene->_sceneMode = _globals->getFlag(53) ? 2112 : 2110;
				scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, NULL);
			}
		} else {
			if (_globals->getFlag(14))
				SceneItem::display2(2100, 32);
			else {
				_globals->setFlag(14);
				_globals->_player.disableControl();
				scene->_sceneMode = 2108;
				scene->setAction(&scene->_sequenceManager, scene, 2109, NULL);
			}
		}

		scene->setAction(&scene->_action4);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene2100::Object3::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (_globals->getFlag(59))
			SceneItem::display2(2100, 34);
		else
			error("***I have no response.");
		break;

	case CURSOR_TALK:
		if (_globals->getFlag(59)) {
			_globals->_player.disableControl();
			scene->_sceneMode = 2108;
			scene->setAction(&scene->_sequenceManager, scene, 2108, NULL);
		} else {
			error("***I have no response.");
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene2100::SceneArea::SceneArea() {
	_savedArea = NULL;
	_pt.x = _pt.y = 0;
}

Scene2100::SceneArea::~SceneArea() {
	delete _savedArea;
}

void Scene2100::SceneArea::setup(int resNum, int rlbNum, int subNum, int actionId) {
	_resNum = resNum;
	_rlbNum = rlbNum;
	_subNum = subNum;
	_actionId = actionId;

	_surface = surfaceFromRes(resNum, rlbNum, subNum);
}

void Scene2100::SceneArea::draw2() {
	_surface.draw(Common::Point(_bounds.left, _bounds.top));	
}

void Scene2100::SceneArea::display() {
	_bounds.left = _pt.x - (_surface.getBounds().width() / 2);
	_bounds.top = _pt.y + 1 - _surface.getBounds().height();
	_bounds.setWidth(_surface.getBounds().width());
	_bounds.setHeight(_surface.getBounds().height());

	_savedArea = Surface_getArea(_globals->_gfxManagerInstance.getSurface(), _bounds);
	draw2();
}

void Scene2100::SceneArea::restore() {
	assert(_savedArea);
	_savedArea->draw(Common::Point(_bounds.left, _bounds.top));
	delete _savedArea;
	_savedArea = NULL;
}

void Scene2100::SceneArea::draw(bool flag) {
	_surface = surfaceFromRes(_resNum, _rlbNum, flag ? _subNum + 1 : _subNum);
	_surface.draw(Common::Point(_bounds.left, _bounds.top));
}

void Scene2100::SceneArea::wait() {
	// Wait until a mouse or keypress
	Event event;
	while (!_vm->getEventManager()->shouldQuit() && !_globals->_events.getEvent(event)) {
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	List<SceneItem *>::iterator ii;
	for (ii = _globals->_sceneItems.begin(); ii != _globals->_sceneItems.end(); ++ii) {
		SceneItem *sceneItem = *ii;
		if (sceneItem->contains(event.mousePos)) {
			sceneItem->doAction(_actionId);
			break;
		}
	}

	_globals->_events.setCursor(CURSOR_ARROW);
}

void Scene2100::SceneArea::synchronise(Serialiser &s) {
	s.syncAsSint16LE(_pt.x);
	s.syncAsSint16LE(_pt.y);
	s.syncAsSint32LE(_resNum);
	s.syncAsSint32LE(_rlbNum);
	s.syncAsSint32LE(_subNum);
	s.syncAsSint32LE(_actionId);
	_bounds.synchronise(s);
}

/*--------------------------------------------------------------------------*/

Scene2100::Scene2100(): 
		_hotspot1(0, CURSOR_LOOK, 2100, 2, LIST_END), 
		_hotspot5(0, CURSOR_LOOK, 2100, 9, LIST_END),
		_hotspot6(0, CURSOR_LOOK, 2100, 7, CURSOR_USE, 2100, 8, LIST_END),
		_hotspot7(0, CURSOR_LOOK, 2100, 7, CURSOR_USE, 2100, 11, LIST_END),
		_hotspot9(0, CURSOR_LOOK, 2100, 14, LIST_END),
		_hotspot11(0, CURSOR_LOOK, 2100, 15, CURSOR_USE, 2100, 16, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 2100, 24, CURSOR_USE, 2100, 25, LIST_END),
		_hotspot13(0, CURSOR_LOOK, 2100, 17, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 2100, 22, CURSOR_USE, 2100, 23, LIST_END) {
}

void Scene2100::postInit(SceneObjectList *OwnerList) {
	loadScene(2100);
	Scene::postInit();
	setZoomPercents(60, 80, 200, 100);

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerSAL);
	_stripManager.addSpeaker(&_speakerHText);
	_stripManager.addSpeaker(&_speakerGameText);
	_speakerMText._npc = &_object3;
	_speakerQText._npc = &_globals->_player;
	_speakerSText._npc = &_object2;
	
	_object1.postInit();
	_object1.setVisage(2100);
	_object1.animate(ANIM_MODE_NONE, NULL);
	_object1.setPosition(Common::Point(157, 57));
	_object1.setPriority(5);

	_hotspot3.postInit();
	_hotspot3.setVisage(2101);
	_hotspot3._frame = 1;
	_hotspot3.animate(ANIM_MODE_2, NULL);
	_hotspot3.setPosition(Common::Point(53, 44));
	_hotspot3.changeZoom(100);
	_hotspot3.setPriority2(1);

	_hotspot4.postInit();
	_hotspot4.setVisage(2101);
	_hotspot4._frame = 1;
	_hotspot4._strip = 3;
	_hotspot4.animate(ANIM_MODE_8, 0, NULL);
	_hotspot4.setPosition(Common::Point(274, 52));
	_hotspot4.changeZoom(100);
	_hotspot4.setPriority2(1);

	_hotspot5.postInit();
	_hotspot5.setVisage(2101);
	_hotspot5._frame = 1;
	_hotspot5._strip = 4;
	_hotspot5.animate(ANIM_MODE_8, 0, NULL);
	_hotspot5.setPosition(Common::Point(219, 141));
	_hotspot5.changeZoom(100);
	_hotspot5.setPriority2(160);

	_hotspot6.postInit();
	_hotspot6.setVisage(2101);
	_hotspot6._frame = 1;
	_hotspot6._strip = 5;
	_hotspot6.setPriority2(175);
	_hotspot6.animate(ANIM_MODE_8, 0, NULL);
	_hotspot6.setPosition(Common::Point(97, 142));
	_hotspot6.changeZoom(100);

	_hotspot7.postInit();
	_hotspot7.setVisage(2101);
	_hotspot7._frame = 1;
	_hotspot7._strip = 6;
	_hotspot7.animate(ANIM_MODE_NONE, NULL);
	_hotspot7.setPosition(Common::Point(133, 46));
	_hotspot7.changeZoom(100);
	_hotspot7.setPriority2(1);

	_hotspot8.postInit();
	_hotspot8.setVisage(2101);
	_hotspot8._frame = 1;
	_hotspot8._strip = 7;
	_hotspot8.animate(ANIM_MODE_8, 0, NULL);
	_hotspot8.setPosition(Common::Point(20, 45));
	_hotspot8.changeZoom(100);
	_hotspot8.setPriority2(1);

	_hotspot2.postInit();
	_hotspot2.setVisage(2101);
	_hotspot2._frame = 1;
	_hotspot2._strip = 7;
	_hotspot2.animate(ANIM_MODE_8, 0, NULL);
	_hotspot2.setPosition(Common::Point(88, 41));
	_hotspot2.changeZoom(100);
	_hotspot2.setPriority2(1);

	_hotspot11.setBounds(Rect(139, 74, 173, 96));
	_hotspot10.setBounds(Rect(71, 100, 91, 135));
	_hotspot9.setBounds(Rect(225, 110, 251, 136));
	_hotspot14.setBounds(Rect(100, 97, 216, 130));
	_hotspot13.setBounds(Rect(13, 124, 94, 168));
	_hotspot12.setBounds(Rect(217, 141, 307, 155));
	_hotspot15.setBounds(Rect(14, 90, 46, 107));
	_hotspot1.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	if (!_globals->getFlag(36) && !_globals->getFlag(70) && !_globals->getFlag(43)) {
		_object3.postInit();
		_object3.setPosition(Common::Point(246, 156));
		_object3.animate(ANIM_MODE_NONE, NULL);
		_object3.changeZoom(100);
		_object3.setPriority2(156);
		_object3.setVisage(2107);
		_object3.setStrip(1);
		_object3.setAction(&_action2);
		_globals->_sceneItems.push_back(&_object3);
	}

	if (!_globals->getFlag(59) && !_globals->getFlag(70) && !_globals->getFlag(37) && !_globals->getFlag(114)) {
		_object2.postInit();
		_object2.setVisage(2108);
		_object2._strip = 3;
		_object2.setPosition(Common::Point(150, 100));
		_object2.animate(ANIM_MODE_NONE, NULL);
		_object2.changeZoom(100);
		_object2.setPriority2(113);
		_object2.setAction(&_action3);
		_globals->_sceneItems.push_back(&_object2);
	}

	_globals->_sceneItems.addItems(&_hotspot15, &_hotspot11, &_hotspot10, &_hotspot9, &_hotspot14,
		&_hotspot13, &_hotspot12, &_hotspot8, &_object1, &_hotspot2, &_hotspot3, &_hotspot4, &_hotspot5,
		&_hotspot6, &_hotspot7, &_hotspot1, NULL);

	_area1.setup(2153, 2, 1, 2100);
	_area1._pt = Common::Point(200, 31);
	_area2.setup(2153, 3, 1, 2150);
	_area2._pt = Common::Point(200, 50);
	_area3.setup(2153, 4, 1, 2320);
	_area3._pt = Common::Point(200, 75);
	_area4.setup(2153, 1, 1, OBJECT_TRANSLATOR);
	_area4._pt = Common::Point(237, 77);

	_globals->_player.postInit();
	_globals->_player.setVisage(_globals->getFlag(13) ? 2170 : 0);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player._moveDiff.x = 4;
	_globals->_player.changeZoom(-1);
	_globals->_player.disableControl();
	_field1800 = 0;

	switch (_globals->_sceneManager._previousScene) {
	case 2120:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_object1.setPriority2(-1);
		_globals->_player.setPriority2(-1);
		_globals->_player.setPosition(Common::Point(80, 66));
		_globals->_player.enableControl();
		break;
	case 2150:
		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));
		_sceneMode = 2104;
		setAction(&_sequenceManager, this, 2104, &_globals->_player, &_object1, NULL);
		break;
	case 2222:
		if (_globals->_sceneObjects->contains(&_object3))
			_object3.remove();

		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(144, 55));
		
		_object2.setVisage(2806);
		_object2.changeZoom(-1);
		_object2.setPosition(Common::Point(158, 55));
		_object2.setPriority2(1);
		_object2.setAction(NULL);
		_object2.setObjectWrapper(new SceneObjectWrapper());
		_object2.animate(ANIM_MODE_1, NULL);
		_object2.setStrip(3);
		setAction(&_action12);
		break;
	case 2320:
		if (_globals->_stripNum == 2321) {
			if (_globals->_sceneObjects->contains(&_object3))
				_object3.remove();
			
			_globals->_player.setPriority2(1);
			_globals->_player.setPosition(Common::Point(144, 55));

			_object2.postInit();
			_object2.setVisage(2806);
			_object2.setStrip(1);
			_object2.changeZoom(-1);
			_object2.setPosition(Common::Point(158, 55));
			_object2.setPriority2(1);
			_object2.setAction(NULL);
			_object2.setObjectWrapper(new SceneObjectWrapper());
			_object2.animate(ANIM_MODE_1, NULL);

			setAction(&_action12);
		} else if (_globals->_stripNum == 6100) {
			_globals->_player.setPosition(Common::Point(157, 56));
			_globals->_player.setPriority2(1);

			_object4.postInit();
			_object4.setVisage(2102);
			_object4.setPosition(Common::Point(160, 199));
			_object4.flag100();

			setAction(&_action14);
		} else {
			_globals->_player.disableControl();
			_globals->_player.setPosition(Common::Point(157, 56));
			_sceneMode = 2104;

			setAction(&_sequenceManager, this, 2104, &_globals->_player, &_object1, NULL);
		}
		break;
	case 3700:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		Scene::setZoomPercents(80, 75, 100, 90);

		if (_globals->_sceneObjects->contains(&_object2))
			_object2.remove();
		
		_globals->_player._angle = 225;
		_globals->_player.setStrip(6);
		_globals->_player.setFrame(1);
		_globals->_player.setPriority2(-1);
		_globals->_player.setPosition(Common::Point(272, 127));

		_object3.setPosition(Common::Point(246, 156));
		_object3.setPriority2(156);
		_sceneMode = 2105;
		setAction(&_sequenceManager, this, 2105, &_object3, NULL);
		break;
	case 4250:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_globals->clearFlag(43);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);

		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.flag100();

		_sceneMode = 2107;
		setAction(&_sequenceManager, this, 2107, &_object4, NULL);
		break;
	case 5000:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_sceneObjects->contains(&_object2))
			_object2.remove();

		_globals->_player.setStrip(3);
		_globals->_player.setFrame(1);
		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));

		_object3.setPosition(Common::Point(246, 156));
		_object3.setPriority2(156);

		setAction(&_action5);
		break;
	case 5100:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_field1800 = 1;

		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.flag100();

		_globals->_inventory._stasisBox._sceneNumber = 0;
		setAction(&_action9);
		break;
	case 7000:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_inventory._stasisBox2._sceneNumber == 1) {
			_globals->_player.setPriority2(1);
			_globals->_player.setPosition(Common::Point(157, 56));
			
			_object4.postInit();
			_object4.setVisage(2102);
			_object4.setPosition(Common::Point(160, 199));
			_object4.flag100();
			_globals->clearFlag(15);
			_globals->clearFlag(109);
			_globals->clearFlag(72);

			setAction(&_action17);
		} else {
			_globals->_player.setVisage(2104);
			_globals->_player.setFrame(1);
			_globals->_player.setPosition(Common::Point(65, 149));
			_globals->_player.setPriority2(152);
			_globals->_player.setStrip(2);

			_field1800 = 1;
			setAction(&_action16);
		}
		break;
	case 7600:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_sceneObjects->contains(&_object2))
			_object2.remove();

		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));

		setAction(&_action8);
		break;
	case 8100:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_sceneMode = 2106;
		setAction(&_sequenceManager, this, 2106, NULL);
		break;
	case 9750:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_object4.postInit();
		_object4.setVisage(2102);
		_object4.setPosition(Common::Point(160, 199));
		_object4.flag100();

		_sceneMode = 2103;
		setAction(&_sequenceManager, this, 2103, &_object4, NULL);
		break;
	default:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player._uiEnabled = true;
		break;
	}

	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2100::stripCallback(int v) {
	switch (v) {
	case 1:
		_object4._numFrames = 4;
		_object4.animate(ANIM_MODE_7, NULL);
		break;
	case 2:
		_object4.animate(ANIM_MODE_NONE, NULL);
		break;
	}
}

void Scene2100::signal() {
	switch (_sceneMode) {
	case 2101:
		_field1800 = 1;
		_globals->_player._uiEnabled = true;
		_globals->_events.setCursor(CURSOR_USE);
		break;
	case 2102:
		_field1800 = 0;
		_globals->_player.enableControl();
		break;
	case 2103:
		_globals->_stripNum = 9000;
		_globals->_sceneManager.changeScene(4000);
		break;
	case 2106:
		_globals->_sceneManager.changeScene(7000);
		break;
	case 2107:
		_globals->_sceneManager.changeScene(5000);
		break;
	case 2104:
	case 2105:
	case 2108:
	case 2110:
	case 2111:
	case 2112:
		_globals->_player.enableControl();
		break;
	}
}

} // End of namespace tSage
