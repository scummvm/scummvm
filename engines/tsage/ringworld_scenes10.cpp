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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.cpp $
 * $Id: scene_logic.cpp 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#include "graphics/cursorman.h"
#include "tsage/ringworld_scenes10.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {
/*--------------------------------------------------------------------------
 * Scene 9350
 *
 *--------------------------------------------------------------------------*/
void Scene9350::Object1::postInit() {
	warning("Scene9350::Object1::postInit - Weird cast to be verified");
	_globals->_sceneManager.postInit((SceneObjectList*) &_globals->_sceneManager._sceneChangeListeners);
}

void Scene9350::Object1::draw() {
	warning("Scene9350::Object1::draw - TODO");
}

void Scene9350::SceneHotspot1::doAction(int action) {
	switch (action) {
	case CURSOR_WALK:
		// Nothing
		break;
	case CURSOR_LOOK:
		if (_field28 == -1)
			SceneHotspot::doAction(action);
		else
			SceneItem::display(_field26, _field28, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	case CURSOR_USE:
		if (_field2A == -1)
			SceneHotspot::doAction(action);
		else
			SceneItem::display(_field26, _field2A, SET_Y, 20, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene9350::signal() {
	switch (_field30A ++) {
	case 0:
	case 9352:
	case 9353:
	case 9354:
		_globals->_player.enableControl();
		break;
	case 9355:
		_globals->_sceneManager.changeScene(9300);
		break;
	case 9356:
		_globals->_sceneManager.changeScene(9360);
		break;
	case 9357:
	case 9359:
		_globals->_sceneManager.changeScene(9400);
		break;
	default:
		break;
	}
}

void Scene9350::dispatch() {
	if (_action == 0) {
		if ((_globals->_player._position.x <= 300) || (_globals->_player._position.y >= 160)) {
			if ((_globals->_player._position.x <= 110) || (_globals->_player._position.y < 195)) {
				_globals->_player.disableControl();
				_field30A = 9355;
				Scene::setAction(&_sequenceManager, this, 9355, &_globals->_player, &_object2, 0);
			} else {
				_globals->_player.disableControl();
				_field30A = 9357;
				Scene::setAction(&_sequenceManager, this, 9357, &_globals->_player, &_object2, 0);
			}
		} else {
			_globals->_player.disableControl();
			_field30A = 9356;
			Scene::setAction(&_sequenceManager, this, 9356, &_globals->_player, &_object2, 0);
		}
	} else {
		Scene::dispatch();
	}
}

void Scene9350::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(95, 80, 200, 100);
	_globals->_player.postInit();

	//TODO: Implement and use SUB_1F1DF in order to reduce code size
	// SUB_1F1DF(&_object1, 9350, 1, 3, 139, 97, 0);
	_object1.postInit();
	_object1.setVisage(9350);
	_object1.setStrip(1);
	_object1.setFrame(3);
	_object1.setPosition(Common::Point(139, 97), 0);
	_object1.setPriority2(0);

	//TODO: Implement and use SUB_4C09
	// SUB_4C09(&_sceneItem1, 42, 0, 97, 60, 9350, 0, -1);
	_sceneHotspot1.setBounds(42, 0, 97, 60);
	_sceneHotspot1._field26 = 9350;
	_sceneHotspot1._field28 = 0;
	_sceneHotspot1._field2A = -1;
	_globals->_sceneItems.addItems(&_sceneHotspot1, NULL);

	// SUB_4C09(&_sceneItem2, 37, 205, 82, 256, 9350, 0, -1);
	_sceneHotspot2.setBounds(37, 205, 82, 256);
	_sceneHotspot2._field26 = 9350;
	_sceneHotspot2._field28 = 0;
	_sceneHotspot2._field2A = -1;
	_globals->_sceneItems.addItems(&_sceneHotspot2, NULL);

	// SUB_4C09(&_sceneItem3, 29, 93, 92, 174, 9350, 1, -1);
	_sceneHotspot3.setBounds(29, 93, 92, 174);
	_sceneHotspot3._field26 = 9350;
	_sceneHotspot3._field28 = 1;
	_sceneHotspot3._field2A = -1;
	_globals->_sceneItems.addItems(&_sceneHotspot3, NULL);

	// SUB_4C09(&_sceneItem4, 0, 308, 109, 320, 9350, 2, -1);
	_sceneHotspot4.setBounds(0, 308, 109, 320);
	_sceneHotspot4._field26 = 9350;
	_sceneHotspot4._field28 = 2;
	_sceneHotspot4._field2A = -1;
	_globals->_sceneItems.addItems(&_sceneHotspot4, NULL);

	// SUB_4C09(&_sceneItem5, 0, 0, 200, 320, 9350, 3, -1);
	_sceneHotspot5.setBounds(0, 0, 200, 320);
	_sceneHotspot5._field26 = 9350;
	_sceneHotspot5._field28 = 3;
	_sceneHotspot5._field2A = -1;
	_globals->_sceneItems.addItems(&_sceneHotspot5, NULL);

	_globals->_events.setCursor(CURSOR_WALK);
	_globals->_player.disableControl();

	if (_globals->_sceneManager._previousScene == 9360) {
		_globals->_player.disableControl();
		_field30A = 9352;
		setAction(&_sequenceManager, this, 9352, &_globals->_player, &_object2, 0);
	} else if (_globals->_sceneManager._previousScene == 9400) {
		_globals->_player.disableControl();
		_field30A = 9353;
		setAction(&_sequenceManager, this, 9353, &_globals->_player, &_object2, 0);
	} else {
		if (!_globals->getFlag(84)) {
			_globals->clearFlag(84);
			_object2.postInit();
			_globals->_player.disableControl();
			_field30A = 9359;
			setAction(&_sequenceManager, this, 9359, &_globals->_player, &_object2, 0);
		} else {
			_globals->_player.disableControl();
			_field30A = 9354;
			setAction(&_sequenceManager, this, 9354, &_globals->_player, &_object2, 0);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 9750
 *
 *--------------------------------------------------------------------------*/
void Scene9750::signal() {
	switch (_sceneMode ++) {
	case 9751:
		_globals->_soundHandler.proc1(this);
		break;
	case 9752:
		_globals->_sceneManager.changeScene(2100);		
	default:
		break;
	}
}

void Scene9750::dispatch() {
	Scene::dispatch();
}

void Scene9750::postInit(SceneObjectList *OwnerList) {
	loadScene(9750);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	
	_globals->_player.postInit();
	_object1.postInit();
	_object1.flag100();
	_object2.postInit();
	_object2.flag100();
	_globals->_player.disableControl();
	_sceneMode = 9751;
	setAction(&_sequenceManager, this, 9751, &_globals->_player, &_object1, &_object2, 0);
}

/*--------------------------------------------------------------------------
 * Scene 9999
 *
 *--------------------------------------------------------------------------*/

void Scene9999::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(600);
		break;
	case 1:
		_globals->_sceneManager.changeScene(3500);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene9999::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		SceneItem::display(9999, 0, SET_Y, 10, SET_X, 30, SET_FONT, 2, SET_BG_COLOUR, -1, SET_EXT_BGCOLOUR, 23, SET_WIDTH, 260, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(300);
		break;
	case 2:
		_globals->_stripNum = 3600;
		_globals->_sceneManager.changeScene(3600);
	default:
		break;
	}
}

void Scene9999::postInit(SceneObjectList *OwnerList) {
	loadScene(9998);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	
	_object1.postInit();
	_object1.setVisage(1303);
	_object1.setStrip2(3);
	_object1.setPosition(Common::Point(160, 152), 0);
	
	_globals->_player.postInit();
	_globals->_player.setVisage(1303);
	_globals->_player.setStrip2(1);
	_globals->_player.setPriority2(250);
	_globals->_player.animate(ANIM_MODE_2, 0);
	_globals->_player.setPosition(Common::Point(194, 98), 0);
	_globals->_player._numFrames = 20;
	_globals->_player.disableControl();

	_object2.postInit();
	_object2.setVisage(1303);
	_object2.setStrip2(2);
	_object2.setPriority2(2);
	_object2.setPosition(Common::Point(164, 149), 0);

	_object3.postInit();
	_object3.setVisage(1303);
	_object3.setStrip2(2);
	_object3.setPriority2(2);
	_object3.setFrame(2);
	_object3.setPosition(Common::Point(292, 149), 0);
	_object3.setAction(&_action3);

	if (_globals->_sceneManager._previousScene == 3500)
		setAction(&_action2);
	else
		setAction(&_action1);

	_globals->_sceneManager._scene->_sceneBounds.centre(_globals->_player._position.x, _globals->_player._position.y);
	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	if (_globals->_sceneManager._previousScene == 3500)
		_globals->_stripNum = 2222;
	else
		_globals->_stripNum = 2121;

	_globals->_soundHandler.startSound(118, 0, 127);

}

} // End of namespace tSage
