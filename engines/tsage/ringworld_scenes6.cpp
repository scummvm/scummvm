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

#include "tsage/ringworld_scenes6.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 5000 - Caverns - Entrance
 *
 *--------------------------------------------------------------------------*/

void Scene5000::Action1::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_soundHandler.startSound(190);
		scene->_soundHandler.proc5(true);
		ADD_MOVER(scene->_hotspot1, 283, 12);
		break;
	case 2:
		break;
	case 3:
		scene->_hotspot1._moveDiff.y = 1;
		scene->_hotspot4.setPosition(Common::Point(scene->_hotspot1._position.x,
			scene->_hotspot1._position.y + 15));
		scene->_hotspot4.unflag100();
		setDelay(15);
		break;
	case 4:
		scene->_soundHandler.proc4();
		ADD_MOVER(scene->_hotspot1, 233, 80);
		break;
	case 5:
		scene->_hotspot3.animate(ANIM_MODE_8, 0, NULL);
		scene->_hotspot3.unflag100();
		ADD_MOVER(scene->_hotspot1, 233, 90);
		break;
	case 6:
		scene->_hotspot3.remove();
		scene->_hotspot2.setPosition(Common::Point(233, 76));
		scene->_hotspot2.unflag100();
		scene->_hotspot2.animate(ANIM_MODE_5, this);

		scene->_hotspot4.remove();
		break;
	case 7:
		setDelay(60);
		scene->_hotspot2.remove();
		break;
	case 8:
		scene->_hotspot5.unflag100();
		scene->_hotspot5.animate(ANIM_MODE_5, this);
		break;
	case 9:
		scene->setAction(&scene->_action2);
		break;
	}
}

void Scene5000::Action1::dispatch() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;
	Action::dispatch();

	if (_actionIndex == 3) {
		if (scene->_hotspot1._percent % 2 == 0) {
			++scene->_hotspot1._position.y;
			if (scene->_hotspot1._position.x > 233)
				--scene->_hotspot1._position.x;
		}

		scene->_hotspot1.changeZoom(++scene->_hotspot1._percent);
		scene->_hotspot1._flags |= OBJFLAG_PANES;

		if (scene->_hotspot1._percent >= 100)
			signal();
	}

	if ((_actionIndex == 5) || (_actionIndex == 6)) {
		scene->_hotspot4.setPosition(Common::Point(scene->_hotspot1._position.x,
			scene->_hotspot1._position.y + 15));
	}
}

void Scene5000::Action2::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.setPosition(Common::Point(217, 76));
		setDelay(10);
		break;
	case 1:
		_globals->_player.setStrip2(3);
		_globals->_player.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(_globals->_player, 214, 89);
		break;
	case 2:
		if (!_globals->getFlag(59))
			setAction(&scene->_action3, this);
		_globals->_player.setPriority2(15);
		ADD_MOVER(_globals->_player, 208, 100);
		break;
	case 3:
		ADD_MOVER(_globals->_player, 213, 98);
		break;
	case 4:
		ADD_MOVER(_globals->_player, 215, 115);
		break;
	case 5:
		_globals->_player.changeZoom(47);
		ADD_MOVER(_globals->_player, 220, 125);
		break;
	case 6:
		ADD_MOVER(_globals->_player, 229, 115);
		break;
	case 7:
		_globals->_player.changeZoom(-1);
		_globals->_player.setPriority2(35);
		ADD_MOVER(_globals->_player, 201, 166);
		break;
	case 8:
		_globals->_player.updateZoom();
		ADD_MOVER(_globals->_player, 205, 146);
		break;
	case 9:
		_globals->_player.changeZoom(-1);
		_globals->_player.setPriority2(50);
		ADD_MOVER(_globals->_player, 220, 182);
		break;
	case 10:
		_globals->_player.updateZoom();
		ADD_MOVER(_globals->_player, 208, 163);
		break;
	case 11:
		_globals->_player.changeZoom(-1);
		_globals->_player.setStrip2(-1);
		_globals->_player.setPriority2(-1);
		ADD_MOVER(_globals->_player, 208, 175);
		break;
	case 12:
		_globals->_player.setStrip(8);
		_globals->_player.setFrame(1);
		setDelay(10);
		break;
	case 13:
		if (!_globals->_sceneObjects->contains(&scene->_hotspot7))
			setDelay(10);
		break;
	case 14:
		setDelay(30);
		break;
	case 15:
		_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene5000::Action3::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_hotspot7.postInit();
		scene->_hotspot7.setVisage(2809);
		scene->_hotspot7.setObjectWrapper(new SceneObjectWrapper());
		scene->_hotspot7.setPosition(Common::Point(217, 76));
		scene->_hotspot7.changeZoom(10);
		scene->_hotspot7.setStrip2(3);
		scene->_hotspot7.setPriority2(200);
		scene->_hotspot7._moveDiff.y = 2;
		scene->_hotspot7.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(scene->_hotspot7, 214, 89);

		_globals->_sceneItems.push_front(&scene->_hotspot7);
		break;
	case 2:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.setPriority2(14);
		ADD_MOVER(scene->_hotspot7, 208, 100);
		break;
	case 3:
		ADD_MOVER(scene->_hotspot7, 213, 98);
		break;
	case 4:
		scene->_hotspot7.setPriority2(19);
		ADD_MOVER(scene->_hotspot7, 213, 98);
		break;
	case 5:
		scene->_hotspot7.changeZoom(46);
		ADD_MOVER(scene->_hotspot7, 220, 125);
		break;
	case 6:
		ADD_MOVER(scene->_hotspot7, 229, 115);
		break;
	case 7:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.setPriority2(34);
		ADD_MOVER(scene->_hotspot7, 201, 166);
		break;
	case 8:
		scene->_hotspot7.updateZoom();
		ADD_MOVER(scene->_hotspot7, 205, 146);
		break;
	case 9:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.setPriority2(49);
		ADD_MOVER(scene->_hotspot7, 210, 182);
		break;
	case 10:
		scene->_hotspot7.updateZoom();
		ADD_MOVER(scene->_hotspot7, 208, 163);
		break;
	case 11:
		scene->_hotspot7.changeZoom(-1);
		scene->_hotspot7.setStrip2(-1);
		scene->_hotspot7.setPriority2(-1);
		ADD_MOVER(scene->_hotspot7, 175, 166);
		break;
	case 12:
		ADD_MOVER(scene->_hotspot7, 126, 146);
		break;
	case 13:
		scene->_hotspot7.setStrip(2);
		remove();
		break;
	}
}

void Scene5000::Action4::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		_globals->_player.setPriority2(50);
		_globals->_player.setStrip2(4);
		ADD_MOVER(_globals->_player, 210, 182);
		break;
	case 2:
		ADD_MOVER(_globals->_player, 205, 146);
		break;
	case 3:
		_globals->_player.setPriority2(35);
		ADD_MOVER(_globals->_player, 201, 166);
		break;
	case 4:
		ADD_MOVER(_globals->_player, 229, 115);
		break;
	case 5:
		_globals->_player.setPriority2(20);
		_globals->_player.changeZoom(47);
		ADD_MOVER(_globals->_player, 220, 125);
		break;
	case 6:
		ADD_MOVER(_globals->_player, 215, 115);
		break;
	case 7:
		_globals->_player.changeZoom(-1);
		ADD_MOVER(_globals->_player, 213, 98);
		break;
	case 8:
		_globals->_player.setPriority2(15);
		ADD_MOVER(_globals->_player, 208, 100);
		break;
	case 9:
		ADD_MOVER(_globals->_player, 214, 89);
		break;
	case 10:
		ADD_MOVER(_globals->_player, 217, 76);
		break;
	case 11:
		_globals->_player.flag100();
		setDelay(60);
		break;
	case 12:
		if (!_globals->_sceneObjects->contains(&scene->_hotspot7))
			_globals->_sceneManager.changeScene(2320);
		remove();
		break;
	}
}

void Scene5000::Action5::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		ADD_MOVER(_globals->_player, 91, 155);
		break;
	case 1:
		_globals->_player.setVisage(2670);
		_globals->_player._strip = 4;
		_globals->_player._frame = 1;
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2:
		SceneItem::display2(5000, _globals->_sceneObjects->contains(&scene->_hotspot7) ? 17 : 13);
		_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		_globals->_player.setVisage(0);
		_globals->_player._strip = 8;
		_globals->_player.animate(ANIM_MODE_1, NULL);

		_globals->_player.enableControl();
		remove();
	}
}

void Scene5000::Action6::signal() {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2150, this);
		break;
	case 2:
		_globals->_events.setCursor(CURSOR_NONE);
		scene->setAction(&scene->_sequenceManager, this, 5001, &scene->_hotspot7, NULL);
		break;
	case 3:
		ADD_PLAYER_MOVER(208, 163);
		break;
	case 4:
		_globals->_player.setPriority2(50);
		_globals->_player.setStrip2(4);
		ADD_MOVER(_globals->_player, 210, 182);
		break;
	case 5:
		_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene5000::Hotspot7::doAction(int action) {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5000, 12);
		break;
	case CURSOR_TALK:
		setAction(&scene->_action6);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5000::Hotspot8::doAction(int action) {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(5000, 10);
		break;
	case OBJECT_STUNNER:
		SceneItem::display2(5000, 14);
		break;
	case OBJECT_SCANNER:
		setAction(&scene->_action5);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene5000::HotspotGroup1::doAction(int action) {
	Scene5000 *scene = (Scene5000 *)_globals->_sceneManager._scene;

	if (_globals->_sceneObjects->contains(&scene->_hotspot7))
		scene->setAction(&scene->_action6);
	else
		SceneItem::display2(5000, 11);
}
/*--------------------------------------------------------------------------*/

Scene5000::Scene5000():
		_hotspot1(0, CURSOR_LOOK, 5000, 3, LIST_END),
		_hotspot12(0, CURSOR_LOOK, 5000, 6, CURSOR_USE, 5000, 7, OBJECT_STUNNER, 5000, 14,
				OBJECT_SCANNER, 5000, 16, LIST_END), 
		_hotspot13(0, CURSOR_LOOK, 5000, 8, CURSOR_USE, 5000, 9, OBJECT_STUNNER, 5000, 0, 
				OBJECT_SCANNER, 5000, 16, LIST_END),
		_hotspot14(0, CURSOR_LOOK, 5000, 8, CURSOR_USE, 5000, 9, OBJECT_STUNNER, 5000, 0, 
				OBJECT_SCANNER, 5000, 16, LIST_END),
		_hotspot15(0, CURSOR_LOOK, 5000, 2, OBJECT_STUNNER, 5000, 15, LIST_END),
		_hotspot16(0, CURSOR_LOOK, 5000, 4, CURSOR_USE, 5000, 5, LIST_END),
		_hotspot17(0, CURSOR_LOOK, 5000, 1, LIST_END),
		_hotspot18(0, CURSOR_LOOK, 5000, 0, LIST_END) {
}


void Scene5000::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	loadScene(5000);

	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);

	_globals->_player.postInit();
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setVisage(0);
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player._moveDiff = Common::Point(4, 2);
	_globals->_player.changeZoom(-1);
	_globals->_player.disableControl();

	_hotspot1.postInit();
	_hotspot1.setVisage(5001);
	_hotspot1.setFrame2(1);
	_hotspot1._moveDiff = Common::Point(5, 5);
	_hotspot1.setPriority2(10);
	_hotspot1.changeZoom(10);

	_hotspot4.postInit();
	_hotspot4.setVisage(5001);
	_hotspot4.setStrip2(2);
	_hotspot4._moveDiff = Common::Point(5, 1);
	_hotspot4.setPriority2(10);
	_hotspot4.changeZoom(100);
	_hotspot4.animate(ANIM_MODE_8, 0, NULL);
	_hotspot4.flag100();

	_hotspot2.postInit();
	_hotspot2.setVisage(5001);
	_hotspot2.setStrip2(3);
	_hotspot2._numFrames = 5;
	_hotspot2.flag100();

	_hotspot3.postInit();
	_hotspot3.setVisage(5001);
	_hotspot3.setStrip2(5);
	_hotspot3._numFrames = 5;
	_hotspot3.setPosition(Common::Point(233, 76));
	_hotspot3.flag100();

	_hotspot5.postInit();
	_hotspot5.setVisage(5001);
	_hotspot5.setStrip2(4);
	_hotspot5._numFrames = 5;
	_hotspot5.setPriority2(15);
	_hotspot5.setPosition(Common::Point(218, 76));
	_hotspot5.flag100();

	_hotspot9.postInit();
	_hotspot9.setVisage(5002);
	_hotspot9.setPriority2(80);
	_hotspot9.setPosition(Common::Point(71, 174));

	_hotspot10.postInit();
	_hotspot10.setVisage(5002);
	_hotspot10.setStrip2(2);
	_hotspot10.setPosition(Common::Point(87, 120));

	_hotspot11.postInit();
	_hotspot11.setVisage(5002);
	_hotspot11.setStrip2(2);
	_hotspot11.setFrame(3);
	_hotspot10.setPosition(Common::Point(93, 118));

	setZoomPercents(95, 10, 145, 100);

	_hotspot8.setBounds(Rect(0, 73, 87, 144));
	_hotspot18.setBounds(Rect(54, 0, 319, 85));
	_hotspot17.setBounds(Rect(184, 0, 199, 79));
	_hotspot13.setBounds(Rect(0, 164, 135, 200));
	_hotspot14.setBounds(Rect(0, 0, 105, 140));
	_hotspot15.setBounds(Rect(266, 70, 291, 85));
	_hotspot16.setBounds(Rect(0, 86, 3219, 200));
	_hotspot12.setBounds(Rect(230, 143, 244, 150));

	_globals->_sceneItems.addItems(&_hotspot9, &_hotspot10, &_hotspot11, &_hotspot8, &_hotspot13,
		&_hotspot14, &_hotspot12, &_hotspot15, &_hotspot17, &_hotspot18, &_hotspot16, NULL);

	switch (_globals->_sceneManager._previousScene) {
	case 1000:
	case 2100:
	case 2320:
		if (_globals->getFlag(59)) {
			_hotspot1.setPosition(Common::Point(233, 90));
			_hotspot1.changeZoom(100);
			_hotspot1.unflag100();
			
			_hotspot5.setFrame(1);
			_hotspot5.animate(ANIM_MODE_5, NULL);
			_hotspot5.setPosition(Common::Point(218, 76));
			_hotspot5.unflag100();

			_globals->_player.setPosition(Common::Point(217, -10));
			_globals->_player.disableControl();

			setAction(&_action2);
		} else {
			_globals->_player.setPosition(Common::Point(217, -10));
			_hotspot1.setPosition(Common::Point(320, -10));
			_globals->_player.disableControl();

			setAction(&_action1);
		}
		break;	
	default:
		_globals->_player.disableControl();
		_globals->_player.setPosition(Common::Point(0, 146));
		
		_hotspot1.changeZoom(100);
		_hotspot1.setPosition(Common::Point(233, 90));
		_hotspot1.unflag100();

		_hotspot5.setFrame(_hotspot5.getFrameCount());
		_hotspot5.unflag100();

		_sceneMode = 5004;
		setAction(&_sequenceManager, this, 5004, &_globals->_player, NULL);
		break;
	}

	_globals->_soundHandler.startSound(190);
}

void Scene5000::signal() {
	switch (_sceneMode) {
	case 5002:
	case 5003:
	case 5004:
		_globals->_player.enableControl();
		break;
	case 5005:
		_globals->_sceneManager.changeScene(5100);
		break;
	}
}

void Scene5000::dispatch() {
	Scene::dispatch();
	
	if (!_action) {
		if (!_globals->_sceneObjects->contains(&_hotspot7) && (_globals->_player.getRegionIndex() == 10)) {
			_globals->_player.disableControl();
			_sceneMode = 5005;
			setAction(&_sequenceManager, this, 5005, &_globals->_player, NULL);
		}

		if (_globals->_player.getRegionIndex() == 8) {
			_globals->_player.disableControl();

			if (_globals->_sceneObjects->contains(&_hotspot7)) {
				_sceneMode = 5003;
				_globals->_player.addMover(NULL);
				setAction(&_sequenceManager, this, 5003, &_globals->_player, NULL);
			} else {
				setAction(&_action4);
			}
		}

		if (_globals->_sceneObjects->contains(&_hotspot7) && (_globals->_player.getRegionIndex() == 15)) {
			_sceneMode = 5002;
			_globals->_player.disableControl();
			_globals->_player.addMover(NULL);
			setAction(&_sequenceManager, this, 5002, &_globals->_player, NULL);
		}
	}
}

} // End of namespace tSage
