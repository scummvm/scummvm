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
 */

#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 100 - Quinn's Room
 *
 *--------------------------------------------------------------------------*/

bool Scene100::Object7::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_state) {
			SceneItem::display2(100, 6);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 101;
			scene->setAction(&scene->_sequenceManager1, scene, 101, &R2_GLOBALS._player, this, NULL);
		}
		return true;
	case CURSOR_TALK:
		if (_state) {
			SceneItem::display2(100, 26);
			_state = 0;
			scene->_object10.setFrame(1);
		} else {
			SceneItem::display2(100, 27);
			_state = 1;
			scene->_object10.setFrame(2);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::Object8::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		if (_strip == 2) {
			scene->_sceneMode = 108;
			scene->_object3.postInit();
			scene->_object9.postInit();

			if (R2_INVENTORY.getObjectScene(R2_3) == 1) {
				scene->_object9.setup(100, 7, 2);
			} else {
				scene->_object9.setup(100, 7, 1);
				scene->_object9.setDetails(100, 21, 22, 23, 2, NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 108, this, &scene->_object3, 
				&scene->_object9, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 109;
			scene->setAction(&scene->_sequenceManager2, scene, 109, this, &scene->_object3, 
				&scene->_object9, &R2_GLOBALS._player, NULL);
		}
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		
		if (_strip == 2) {
			SceneItem::display2(100, 18);
			scene->_sceneMode = 102;
			scene->_object3.postInit();
			scene->_object9.postInit();

			if (R2_INVENTORY.getObjectScene(R2_3) == 1) {
				scene->_object9.setup(100, 7, 2);
			} else {
				scene->_object9.setup(100, 7, 1);
				scene->_object9.setDetails(100, 21, 22, 23, 2, NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 102, this, &scene->_object3, 
				&scene->_object9, NULL);
		} else {
			SceneItem::display2(100, 19);
			scene->_sceneMode = 103;
			scene->setAction(&scene->_sequenceManager2, scene, 103, this, &scene->_object3, 
				&scene->_object9, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::Object9::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 107;
		scene->setAction(&scene->_sequenceManager1, scene, 107, &R2_GLOBALS._player, &scene->_object9, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::Object10::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(100, _state ? 24 : 25);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(100, _state ? 26 : 27);
		return true;
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 110;
		scene->setAction(&scene->_sequenceManager1, scene, 110, &R2_GLOBALS._player, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::SteppingDisks::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 111;
		scene->setAction(&scene->_sequenceManager1, scene, 111, &R2_GLOBALS._player, this, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene100::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(100);
	R2_GLOBALS._scenePalette.loadPalette(0);

	if (R2_GLOBALS._sceneManager._previousScene != 125)
		R2_GLOBALS._sound1.play(10);

	_object7.postInit();
	_object7._state = 0;
	_object7.setVisage(100);
	_object7.setPosition(Common::Point(160, 84));
	_object7.setDetails(100, 3, 4, 5, 1, NULL);

	_object10.postInit();
	_object10.setup(100, 2, 1);
	_object10.setDetails(100, -1, -1, -1, 1, NULL);

	_object8.postInit();
	_object8.setup(100, 2, 3);
	_object8.setPosition(Common::Point(175, 157));
	_object8.setDetails(100, 17, 18, 20, 1, NULL);

	_object1.postInit();
	_object1.setup(100, 3, 1);
	_object1.setPosition(Common::Point(89, 79));
	_object1.fixPriority(250);
	_object1.animate(ANIM_MODE_2, NULL);
	_object1._numFrames = 3;

	_object2.postInit();
	_object2.setup(100, 3, 1);
	_object2.setPosition(Common::Point(89, 147));
	_object2.fixPriority(250);
	_object2.animate(ANIM_MODE_7, 0, NULL); 
	_object2._numFrames = 3;

	_object6.postInit();
	_object6.setVisage(101);
	_object6.setPosition(Common::Point(231, 126));
	_object6.fixPriority(10);
	_object6.setDetails(100, 37, -1, 39, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_STEPPING_DISKS) == 100) {
		_steppingDisks.postInit();
		_steppingDisks.setup(100, 8, 1);
		_steppingDisks.setPosition(Common::Point(274, 130));
		_steppingDisks.setDetails(100, 40, -1, 42, 1, NULL);
	}

	_item5.setDetails(11, 100, 14, 15, 16);
	_item4.setDetails(12, 100, 11, -1, 13);
	_item3.setDetails(13, 100, 8, 9, 10);
	_item2.setDetails(14, 100, 34, -1, 36);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();
	
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 100, 0, 1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 180:
		_object5.postInit();
		_object4.postInit();
		_sceneMode = 104;
		setAction(&_sequenceManager1, this, 104, &R2_GLOBALS._player, &_object6, &_object4, &_object5, NULL);
		break;
	case 125:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 106, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 100, &R2_GLOBALS._player, &_object7, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::remove() {
	R2_GLOBALS._sound1.play(10);
	SceneExt::remove();
}

void Scene100::signal() {
	switch (_sceneMode) {
	case 101:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 103:
	case 109:
		_object8.setStrip(2);
		_object8.setFrame(3);
		
		_object3.remove();
		_object9.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 104:
		_sceneMode = 0;
		_object5.remove();
		_object4.remove();

		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl();
		break;
	case 105:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 107:
		R2_GLOBALS._sceneItems.remove(&_object9);

		_object9.setFrame(2);
		R2_INVENTORY.setObjectScene(3, 1);
		R2_GLOBALS._player.enableControl();
		break;
	case 110:
		if (_object7._state) {
			_object7._state = 0;
			_object10.setFrame(1);
		} else {
			_object7._state = 1;
			_object10.setFrame(2);
		}
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::dispatch() {
/*
	int regionIndex = R2_GLOBALS._player.getRegionIndex();
	if (regionIndex == 13)
		R2_GLOBALS._player._shade = 4;

	if ((R2_GLOBALS._player._visage == 13) || (R2_GLOBALS._player._visage == 101))
	(R2_GLOBALS._player._shade = 0;
*/
	SceneExt::dispatch();

	if ((_sceneMode == 101) && (_object7._frame == 2) && (_object8._strip == 5)) {
		_object8.setAction(&_sequenceManager2, NULL, 103, &_object8, &_object3, &_object9, NULL);
	}
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
