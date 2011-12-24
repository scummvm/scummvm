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
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 3100 - 
 *
 *--------------------------------------------------------------------------*/
Scene3100::Scene3100() {
	_field412 = 0;
}

void Scene3100::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene3100::Guard::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene3100 *scene = (Scene3100 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 10;
	R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
	scene->_stripManager.start(606, scene);
	return true;
}

void Scene3100::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		if (R2_GLOBALS._player._oldCharacterScene[1] == 3100) {
			loadScene(3101);
			R2_GLOBALS._v58CE2 = 0;
		} else {
			loadScene(3100);
			g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));
		}
	} else {
		loadScene(3100);
	}
	// Original was doing it twice in a row. Skipped.

	if (R2_GLOBALS._sceneManager._previousScene == 3255)
		R2_GLOBALS._v58CE2 = 0;

	SceneExt::postInit();
	_stripManager.addSpeaker(&_guardSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1000;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._characterIndex = R2_QUINN;
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_item2.setDetails(Rect(212, 97, 320, 114), 3100, 3, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 480, 200), 3100, 0, -1, -1, 1, NULL);
	_field412 = 0;

	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		if (R2_GLOBALS._player._oldCharacterScene[1] == 3100) {
			_sceneMode = 3102;
			_actor3.postInit();
			_actor4.postInit();
			_actor5.postInit();
			R2_GLOBALS._sound1.play(274);
			_sound1.fadeSound(130);
			setAction(&_sequenceManager, this, 3102, &_actor1, &R2_GLOBALS._player, &_actor3, &_actor4, &_actor5, NULL);
		} else {
			_guard.postInit();
			_guard.setup(3110, 5, 1);
			_guard.changeZoom(50);
			_guard.setPosition(Common::Point(10, 149));
			_guard.setDetails(3100, 6, -1, -1, 2, NULL);

			_actor4.postInit();
			_actor4.setup(3103, 1, 1);
			_actor4.setPosition(Common::Point(278, 113));
			_actor4.setDetails(3100, 9, -1, -1, 2, NULL);
			_actor4.animate(ANIM_MODE_2, NULL);

			_field412 = 1;
			_actor1.setDetails(3100, 3, -1, -1, 2, NULL);
			R2_GLOBALS._sound1.play(243);
			R2_GLOBALS._sound2.play(130);
			_sceneMode = 3100;

			setAction(&_sequenceManager, this, 3100, &R2_GLOBALS._player, &_actor1, NULL);
		}
	} else if (R2_GLOBALS._sceneManager._previousScene == 3255) {
		_sceneMode = 3101;
		_actor2.postInit();
		_actor3.postInit();
		_field412 = 1;

		setAction(&_sequenceManager, this, 3101, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor3, NULL);
	} else {
		_guard.postInit();
		_guard.setup(3110, 5, 1);
		_guard.changeZoom(50);
		_guard.setPosition(Common::Point(10, 149));
		_guard.setDetails(3100, 6, -1, -1, 2, NULL);
		
		_actor4.postInit();
		_actor4.setup(3103, 1, 1);
		_actor4.setPosition(Common::Point(278, 113));
		_actor4.setDetails(3100, 9, -1, -1, 2, NULL);
		_actor4.animate(ANIM_MODE_2, NULL);

		_actor1.postInit();
		_actor1.setup(3104, 4, 1);
		_actor1.setPosition(Common::Point(143, 104));
		_actor1.setDetails(3100, 3, -1, -1, 2, NULL);

		R2_GLOBALS._player.setup(3110, 3, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);

		R2_GLOBALS._sound1.play(243);
	}
	
	R2_GLOBALS._player._oldCharacterScene[1] = 3100;
}

void Scene3100::remove() {
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	_sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3100::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 3100:
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	case 3101:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 3102:
		R2_GLOBALS._player._oldCharacterScene[1] = 1000;
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

void Scene3100::dispatch() {
	if ((_sceneMode == 3100) && (_field412 != 0) && (R2_GLOBALS._player._position.y == 104)) {
		_field412 = 0;
		R2_GLOBALS._sound2.fadeOut2(NULL);
	}

	if ((_sceneMode == 3101) && (_field412 != 0) && (R2_GLOBALS._player._position.y < 104)) {
		_field412 = 0;
		_sound1.fadeSound(130);
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3125 - Ghouls dormitory
 *
 *--------------------------------------------------------------------------*/
Scene3125::Scene3125() {
	_field412 = 0;
}

void Scene3125::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene3125::Item1::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1)
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1)
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1)
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return scene->display(action, event);
		break;
	}

	return true;
}

bool Scene3125::Item2::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3125;
		scene->setAction(&scene->_sequenceManager1, scene, 3125, &R2_GLOBALS._player, NULL);
		break;
	case CURSOR_LOOK:
		SceneItem::display(3125, 15, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		SceneItem::display(3125, 13, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}

	return true;
}

bool Scene3125::Item3::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_actor5.postInit();
		scene->_sceneMode = 3126;
		scene->setAction(&scene->_sequenceManager1, scene, 3126, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor3, &scene->_actor4, &scene->_actor1, &scene->_actor5, NULL);
		break;
	case CURSOR_LOOK:
		SceneItem::display(3125, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	case CURSOR_TALK:
		SceneItem::display(3125, 13, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		break;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}

	return true;
}

bool Scene3125::Actor1::startAction(CursorType action, Event &event) {
	Scene3125 *scene = (Scene3125 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3176;
	scene->setAction(&scene->_sequenceManager1, scene, 3176, &R2_GLOBALS._player, &scene->_actor1, NULL);
	return true;
}

void Scene3125::postInit(SceneObjectList *OwnerList) {
	loadScene(3125);
	SceneExt::postInit();
	_field412 = 0;

	_actor1.postInit();
	_actor1.setup(3175, 1, 1);
	_actor1.setPosition(Common::Point(35, 72));
	_actor1.setDetails(3125, 12, 13, -1, 1, NULL);

	_actor2.postInit();
	_actor2.setup(3126, 4, 1);
	_actor2.setPosition(Common::Point(71, 110));
	_actor2._numFrames = 20;

	_actor3.postInit();
	_actor3.setup(3126, 1, 1);
	_actor3.setPosition(Common::Point(215, 62));
	_actor3.fixPriority(71);

	_actor4.postInit();
	_actor4.setup(3126, 1, 1);
	_actor4.setPosition(Common::Point(171, 160));
	_actor4.fixPriority(201);
	
	_item3.setDetails(12, 3125, 9, 13, -1);
	_item2.setDetails(11, 3125, 15, 13, -1);
	_item1.setDetails(Rect(0, 0, 320, 200), 3125, 0, 1, 2, 1, NULL);

	R2_GLOBALS._sound1.play(262);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3250) {
		_sceneMode = 3175;
		setAction(&_sequenceManager1, this, 3175, &R2_GLOBALS._player, &_actor1, NULL);
	} else {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(89, 76));
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3125;
}

void Scene3125::signal() {
	switch (_sceneMode) {
	case 3125:
		SceneItem::display(3125, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		_sceneMode = 3127;
		setAction(&_sequenceManager1, this, 3127, &R2_GLOBALS._player, NULL);
		break;
	case 3126:
		R2_GLOBALS.setFlag(79);
	// No break on purpose
	case 3176:
		R2_GLOBALS._sceneManager.changeScene(3250);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3125::dispatch() {
	if ((_sceneMode == 3126) && (_actor2._frame == 2) && (_field412 == 0)) {
		_field412 = 1;
		R2_GLOBALS._sound1.play(265);
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3150 - Jail
 *
 *--------------------------------------------------------------------------*/
bool Scene3150::Item5::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(47) != 3150)
			return SceneHotspot::startAction(action, event);

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3154;
		scene->setAction(&scene->_sequenceManager, scene, 3154, &R2_GLOBALS._player, &scene->_actor3, NULL);
		return true;
	case R2_40:
		if ((R2_INVENTORY.getObjectScene(47) != 3150) && (R2_GLOBALS.getFlag(75))) {
			R2_GLOBALS._player.disableControl();
			scene->_actor3.postInit();
			scene->_actor3._effect = 3;
			scene->_actor3._shade = 5;
			scene->_sceneMode = 3155;
			scene->setAction(&scene->_sequenceManager, scene, 3155, &R2_GLOBALS._player, &scene->_actor3, NULL);
		} else {
			SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		}
		return true;
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene3150::Item6::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_41:
		R2_GLOBALS._player.disableControl();
		scene->_actor4.postInit();
		scene->_actor4._effect = 6;
		scene->_actor4._shade = 3;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3158;
		scene->setAction(&scene->_sequenceManager, scene, 3158, &R2_GLOBALS._player, &scene->_actor4, NULL);
		return true;
	case R2_42:
		if ((R2_INVENTORY.getObjectScene(47) != 3150) && (R2_INVENTORY.getObjectScene(40) == 3150) && (R2_GLOBALS.getFlag(75))) {
			scene->_actor5.postInit();
			scene->_actor5._effect = 6;
			scene->_actor5._shade = 3;
			scene->_actor5.setDetails(3150, 30, -1, -1, 2, NULL);

			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 3159;
			scene->setAction(&scene->_sequenceManager, scene, 3159, &R2_GLOBALS._player, &scene->_actor5, NULL);
		} else {
			SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		}
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene3150::Actor4::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS.getFlag(75))
			return SceneActor::startAction(action, event);
		
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3151;
		scene->setAction(&scene->_sequenceManager, scene, 3151, &R2_GLOBALS._player, &scene->_actor4, NULL);
		return true;
	case R2_42:
		return false;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

bool Scene3150::Actor5::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (R2_GLOBALS.getFlag(77)))
		return SceneActor::startAction(action ,event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3157;
	scene->setAction(&scene->_sequenceManager, scene, 3157, &R2_GLOBALS._player, &scene->_actor5, NULL);
	return true;
}

bool Scene3150::Actor6::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (R2_GLOBALS.getFlag(75)) {
			if (R2_GLOBALS.getFlag(77)) {
				R2_GLOBALS._player.disableControl();
				if (R2_GLOBALS.getFlag(76)) {
					scene->_sceneMode = 3152;
					scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, NULL);
				} else {
					scene->_sceneMode = 3153;
					scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, &scene->_actor4, NULL);
				}
			} else {
				SceneItem::display(3150, 42, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			}
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 3152;
			scene->setAction(&scene->_sequenceManager, scene, 3152, &R2_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene3150::Actor7::startAction(CursorType action, Event &event) {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_43) && (!R2_GLOBALS.getFlag(80))) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3160;
		scene->setAction(&scene->_sequenceManager, scene, 3160, &R2_GLOBALS._player, &scene->_actor7, NULL);
		return true;
	}
	
	return SceneActor::startAction(action, event);
}

void Scene3150::Exit1::changeScene() {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	g_globals->_events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(-20, 180);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene3150::Exit2::changeScene() {
	Scene3150 *scene = (Scene3150 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	g_globals->_events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 12;

	scene->setAction(&scene->_sequenceManager, scene, 3163, &R2_GLOBALS._player, NULL);
}

void Scene3150::postInit(SceneObjectList *OwnerList) {
	loadScene(3150);
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_INVENTORY.setObjectScene(35, 2000);
		R2_GLOBALS._player._oldCharacterScene[1] = 3100;
		R2_GLOBALS._player._oldCharacterScene[3] = 0;
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	}
	SceneExt::postInit();

	if (R2_GLOBALS.getFlag(78)) {
		_exit1.setDetails(Rect(0, 135, 60, 168), EXITCURSOR_SW, 3275);
		_exit1.setDest(Common::Point(70, 125));
	}

	if (R2_GLOBALS.getFlag(80)) {
		_exit2.setDetails(Rect(249, 36, 279, 60), EXITCURSOR_NE, 3150);
		_exit2.setDest(Common::Point(241, 106));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	
	_actor2.postInit();
	_actor2.setPosition(Common::Point(64, 139));
	if (R2_GLOBALS.getFlag(78)) {
		_actor2.setup(3151, 1, 5);
		_actor2.fixPriority(125);
	} else {
		_actor2.setup(3151, 1, 1);
		_actor2.setDetails(3150, 8, -1, 9, 1, NULL);
	}

	if (R2_GLOBALS.getFlag(78)) {
		_actor1.postInit();
		_actor1.setup(3154, 1, 16);
		_actor1.setPosition(Common::Point(104, 129));
		_actor1._effect = 6;
		_actor1._shade = 3;
		_actor1.setDetails(3150, 24, -1, -1, -1, NULL);
	}

	_actor7.postInit();
	_actor7.setup(3154, 5, 1);
	if (R2_GLOBALS.getFlag(80))
		_actor7.setPosition(Common::Point(264, 108));
	else
		_actor7.setPosition(Common::Point(264, 58));
	_actor7.fixPriority(50);
	_actor7.setDetails(3150, 17, -1, 19, 1, NULL);

	if (R2_INVENTORY.getObjectScene(41) == 3150) {
		_actor4.postInit();
		if (R2_GLOBALS.getFlag(75)) {
			if (R2_GLOBALS.getFlag(76)) {
				R2_GLOBALS._walkRegions.enableRegion(1);
				R2_GLOBALS._walkRegions.enableRegion(4);
				R2_GLOBALS._walkRegions.enableRegion(5);
				R2_GLOBALS._walkRegions.enableRegion(6);
				_actor4.setup(3152, 4, 10);
				_actor4.setDetails(3150, 14, -1, -1, 1, NULL);
			} else {
				_actor4.setup(3152, 7, 4);
				_actor4.setDetails(3150, 13, -1, -1, 1, NULL);
			}
			_actor4.fixPriority(110);
			_actor4.setPosition(Common::Point(83, 88));
			_actor4._effect = 6;
			_actor4._shade = 3;
		} else {
			_actor4.setup(3152, 7, 3);
			_actor4.setPosition(Common::Point(143, 70));
			_actor4.setDetails(3150, 15, -1, -1, 1, NULL);
		}
	}

	if (R2_INVENTORY.getObjectScene(47) == 3150) {
		_actor3.postInit();
		_actor3.setup(3152, 7, 1);
		_actor3.setPosition(Common::Point(73, 83));
	}

	if (R2_INVENTORY.getObjectScene(40) == 3150) {
		_actor3.postInit();
		_actor3.setup(3152, 7, 3);
		_actor3.setPosition(Common::Point(70, 55));
		_actor3.fixPriority(111);
		_actor3._effect = 6;
		_actor3._shade = 5;
	}

	if (R2_INVENTORY.getObjectScene(42) == 3150) {
		_actor5.postInit();
		if (R2_GLOBALS.getFlag(77)) {
			_actor5.setup(3152, 7, 8);
			_actor5.setPosition(Common::Point(82, 92));
			_actor5.fixPriority(111);
			_actor5._effect = 6;
			_actor5._shade = 3;
		} else {
			_actor5.setup(3152, 7, 7);
			_actor5.setPosition(Common::Point(155, 79));
		}
		_actor5.setDetails(3150, 30, -1, -1, 2, NULL);
	}

	_actor6.postInit();
	_actor6.setup(3152, 7, 6);
	_actor6.setPosition(Common::Point(98, 73));
	_actor6.setDetails(3150, 43, -1, -1, 1, NULL);

	_item2.setDetails(12, 3150, 10, -1, 12);
	_item3.setDetails(Rect(186, 17, 210, 36), 3150, 6, -1, -1, 1, NULL);
	_item4.setDetails(Rect(61, 21, 92, 41), 3150, 7, -1, -1, 1, NULL);
	_item5.setDetails(Rect(63, 48, 78, 58), 3150, 6, -1, -1, 1, NULL);
	_item6.setDetails(Rect(63, 81, 100, 95), 3150, 3, 4, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 200, 320), 3150, 0, 1, 2, 1, NULL);

	switch (R2_GLOBALS._player._oldCharacterScene[3]) {
	case 0:
		_sceneMode = 3150;
		_actor1.postInit();
		_actor1._effect = 6;
		_actor1._shade = 5;
		setAction(&_sequenceManager, this, 3150, &R2_GLOBALS._player, &_actor1, &_actor2, NULL);
		break;
	case 1200:
		_sceneMode = 3162;
		setAction(&_sequenceManager, this, 3162, &R2_GLOBALS._player, NULL);
		break;
	case 3275: {
		_sceneMode = 10;
		R2_GLOBALS._player.setup(30, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(-20, 180));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

		Common::Point pt(80, 125);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	default:
		if ((R2_GLOBALS._v56AA0 == 1) && (R2_INVENTORY.getObjectScene(35) == 2000) && (R2_GLOBALS._player._oldCharacterScene[1] == 3100)) {
			++R2_GLOBALS._v56AA0;
			_sceneMode = 3156;
			_actor1.postInit();
			_actor1._effect = 6;
			_actor1._shade = 3;

			_actor2.postInit();
			_actor5.postInit();
			_actor5._effect = 6;
			_actor5._shade = 3;
				
			setAction(&_sequenceManager, this, 3156, &R2_GLOBALS._player, &_actor1, &_actor2, &_actor5, NULL);
		} else {
			if (R2_GLOBALS._v56AA0 != 2)
				++R2_GLOBALS._v56AA0;

			R2_GLOBALS._player.setup(30, 3, 1);
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(155, 120));
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
			R2_GLOBALS._player.enableControl();
		}
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3150;
}

void Scene3150::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._sceneManager.changeScene(3275);
		break;
	case 12:
		R2_GLOBALS._sceneManager.changeScene(1200);
		break;
	case 3151:
		_actor1.remove();
		R2_INVENTORY.setObjectScene(41, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3153:
		R2_GLOBALS.setFlag(76);
		_actor4.setDetails(3150, 14, -1, -1, 3, NULL);
		_actor1.postInit();
		_actor1.setDetails(3150, 24, -1, -1, 2, NULL);
		_sceneMode = 3161;
		setAction(&_sequenceManager, this, 3161, &_actor1, &_actor2, NULL);
		break;
	case 3154:
		_actor3.remove();
		R2_INVENTORY.setObjectScene(47, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3155:
		R2_INVENTORY.setObjectScene(40, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3156:
		_actor5.setDetails(3150, 30, -1, -1, 2, NULL);
		R2_INVENTORY.setObjectScene(42, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3157:
		_actor5.remove();
		R2_INVENTORY.setObjectScene(42, 3);
		R2_GLOBALS._player.enableControl();
		break;
	case 3158:
		R2_GLOBALS.setFlag(75);
		R2_INVENTORY.setObjectScene(41, 3150);
		_actor4.fixPriority(110);
		_actor4.setDetails(3150, 13, -1, -1, 2, NULL);
		R2_GLOBALS._player.enableControl();
		break;
	case 3159:
		R2_GLOBALS.setFlag(77);
		R2_INVENTORY.setObjectScene(42, 3150);
		R2_GLOBALS._player.enableControl();
		break;
	case 3160:
		R2_INVENTORY.setObjectScene(52, 3150);
		R2_GLOBALS.setFlag(80);
		R2_GLOBALS._sceneManager.changeScene(1200);
		break;
	case 3161:
		R2_GLOBALS._sceneItems.remove(&_actor2);
		_exit1.setDetails(Rect(0, 135, 60, 168), EXITCURSOR_SW, 3275);
		_exit1.setDest(Common::Point(70, 125));
		R2_GLOBALS._walkRegions.enableRegion(1);
		R2_GLOBALS._walkRegions.enableRegion(4);
		R2_GLOBALS._walkRegions.enableRegion(5);
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS.setFlag(78);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3150::dispatch() {
	if (_actor5._position.x == 155) {
		_actor5._effect = 0;
		_actor5._shade = 0;
	}

	if (_actor1._visage == 3154) {
		_actor1._effect = 0;
		_actor1._shade = 0;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3175 - Autopsy room
 *
 *--------------------------------------------------------------------------*/
bool Scene3175::Item1::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3175::Actor3::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3175::Actor1::startAction(CursorType action, Event &event) {
	Scene3175 *scene = (Scene3175 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 3176;
		scene->setAction(&scene->_sequenceManager, scene, 3176, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return true;
		break;
	case CURSOR_LOOK:
		SceneItem::display(3175, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		return true;
		break;
	case CURSOR_TALK:
		SceneItem::display(3175, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

void Scene3175::postInit(SceneObjectList *OwnerList) {
	loadScene(3175);
	SceneExt::postInit();

	_actor1.postInit();
	_actor1.setup(3175, 1, 1);
	_actor1.setPosition(Common::Point(35, 72));
	_actor1.setDetails(3175, 9, 10, -1, 1, NULL);

	_actor2.postInit();
	_actor2.setup(3175, 2, 1);
	_actor2.setPosition(Common::Point(87, 148));

	_actor3.postInit();
	_actor3.setup(3175, 3, 1);
	_actor3.setPosition(Common::Point(199, 117));
	_actor3.setDetails(3175, 15, 16, 17, 1, NULL);

	_item2.setDetails(12, 3175, 3, 1, 5);
	_item3.setDetails(11, 3175, 6, 7, 8);
	_item1.setDetails(Rect(0, 0, 320, 200), 3175, 0, 1, 2, 1, NULL);

	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3250) {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(126, 77));
		R2_GLOBALS._player.enableControl();
	} else {
		_sceneMode = 3175;
		setAction(&_sequenceManager, this, 3175, &R2_GLOBALS._player, &_actor1, NULL);
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3175;
}

void Scene3175::signal() {
	if (_sceneMode == 3176)
		R2_GLOBALS._sceneManager.changeScene(3250);
	else
		R2_GLOBALS._player.enableControl();
}

/*--------------------------------------------------------------------------
 * Scene 3200 - Cutscene : Guards - Discussion
 *
 *--------------------------------------------------------------------------*/
void Scene3200::postInit(SceneObjectList *OwnerList) {
	loadScene(3200);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);
	_stripManager.addSpeaker(&_sockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor3.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3200 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, &_actor3, NULL);
}

void Scene3200::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3210 - Cutscene : Captain and Private - Discussion
 *
 *--------------------------------------------------------------------------*/
void Scene3210::postInit(SceneObjectList *OwnerList) {
	loadScene(3210);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_privateSpeaker);
	_stripManager.addSpeaker(&_captainSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3210 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3210::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3220 - Cutscene : Guards in cargo zone
 *
 *--------------------------------------------------------------------------*/
void Scene3220::postInit(SceneObjectList *OwnerList) {
	loadScene(3220);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3220 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3220::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3230 - Cutscene : Guards on duty
 *
 *--------------------------------------------------------------------------*/
void Scene3230::postInit(SceneObjectList *OwnerList) {
	loadScene(3230);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_rockoSpeaker);
	_stripManager.addSpeaker(&_jockoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();
	_actor3.postInit();

	setAction(&_sequenceManager, this, 3230 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, &_actor3, NULL);
}

void Scene3230::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3240 - Cutscene : Teal monolog
 *
 *--------------------------------------------------------------------------*/
void Scene3240::postInit(SceneObjectList *OwnerList) {
	loadScene(3240);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	setAction(&_sequenceManager, this, 3240 + R2_GLOBALS._randomSource.getRandomNumber(1), &_actor1, &_actor2, NULL);
}

void Scene3240::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3245 - Cutscene : Discussions with Dr. Tomko
 *
 *--------------------------------------------------------------------------*/
void Scene3245::postInit(SceneObjectList *OwnerList) {
	loadScene(3245);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_ralfSpeaker);
	_stripManager.addSpeaker(&_tomkoSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor1.postInit();
	_actor2.postInit();

	if (R2_GLOBALS._v56AA1 < 4)
		++R2_GLOBALS._v56AA1;

	if (R2_GLOBALS._v56AA1 >= 4) {
		SceneItem::display(1200, 7, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		signal();
	} else {
		setAction(&_sequenceManager, this, 3244 + R2_GLOBALS._v56AA1, &_actor1, &_actor2, NULL);
	}
}

void Scene3245::signal() {
	R2_GLOBALS._sceneManager.changeScene(1200);
}

/*--------------------------------------------------------------------------
 * Scene 3250 - Room with large stasis field negator
 *
 *--------------------------------------------------------------------------*/
bool Scene3250::Item::startAction(CursorType action, Event &event) {
	Scene3250 *scene = (Scene3250 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_useLineNum != -1) {
			SceneItem::display(_resNum, _useLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_LOOK:
		if (_lookLineNum != -1) {
			SceneItem::display(_resNum, _lookLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	case CURSOR_TALK:
		if (_talkLineNum != -1) {
			SceneItem::display(_resNum, _talkLineNum, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
			return true;
		}
		break;
	default:
		break;
	}

	return scene->display(action, event);
}

bool Scene3250::Actor::startAction(CursorType action, Event &event) {
	Scene3250 *scene = (Scene3250 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();

	switch(_position.x) {
	case 25:
		scene->_sceneMode = 3262;
		scene->setAction(&scene->_sequenceManager, scene, 3262, &R2_GLOBALS._player, &scene->_actor1, NULL);
		break;
	case 259:
		scene->_sceneMode = 3260;
		scene->setAction(&scene->_sequenceManager, scene, 3260, &R2_GLOBALS._player, &scene->_actor2, NULL);
		break;
	case 302:
		scene->_sceneMode = 3261;
		scene->setAction(&scene->_sequenceManager, scene, 3261, &R2_GLOBALS._player, &scene->_actor3, NULL);
		break;
	default:
		break;
	}
	return true;
}

void Scene3250::postInit(SceneObjectList *OwnerList) {
	loadScene(3250);

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._player._oldCharacterScene[3] = 1200;
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	}

	SceneExt::postInit();
	_actor1.postInit();
	_actor1.setup(3250, 6, 1);
	_actor1.setPosition(Common::Point(25, 148));
	_actor1.fixPriority(10);
	_actor1.setDetails(3250, 9, 10, -1, 1, NULL);

	_actor2.postInit();
	_actor2.setup(3250, 4, 1);
	_actor2.setPosition(Common::Point(259, 126));
	_actor2.fixPriority(10);
	_actor2.setDetails(3250, 9, 10, -1, 1, NULL);

	_actor3.postInit();
	_actor3.setup(3250, 5, 1);
	_actor3.setPosition(Common::Point(302, 138));
	_actor3.fixPriority(10);
	_actor3.setDetails(3250, 9, 10, -1, 1, NULL);

	_item3.setDetails(Rect(119, 111, 149, 168), 3250, 6, 7, 2, 1, NULL);
	_item2.setDetails(Rect(58, 85, 231, 138), 3250, 12, 7, 2, 1, NULL);
	_item4.setDetails(12, 3250, 3, 1, 2);
	_item1.setDetails(Rect(0, 0, 320, 200), 3250, 0, 1, 2, 1, NULL);

	R2_GLOBALS._player.postInit();

	switch (R2_GLOBALS._player._oldCharacterScene[3]) {
	case 1200:
		_sceneMode = 3250;
		_actor4.postInit();
		R2_GLOBALS._player._effect = 0;
		setAction(&_sequenceManager, this, 3250, &R2_GLOBALS._player, &_actor4, NULL);
		break;
	case 3125:
		if (R2_GLOBALS.getFlag(79)) {
			_sceneMode = 3254;
			_actor5.postInit();
			_actor5._effect = 1;
			_actor6.postInit();
			_actor6._effect = 1;
			_actor7.postInit();
			_actor7._effect = 1;
			setAction(&_sequenceManager, this, 3254, &R2_GLOBALS._player, &_actor3, &_actor5, &_actor6, &_actor7, &_actor1, NULL);
		} else {
			_sceneMode = 3252;
			setAction(&_sequenceManager, this, 3252, &R2_GLOBALS._player, &_actor3, NULL);
		}
		break;
	case 3175:
		_sceneMode = 3251;
		setAction(&_sequenceManager, this, 3251, &R2_GLOBALS._player, &_actor2, NULL);
		break;
	case 3255:
		_sceneMode = 3253;
		setAction(&_sequenceManager, this, 3253, &R2_GLOBALS._player, &_actor1, NULL);
		break;
	default:
		R2_GLOBALS._player.setup(31, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(185, 150));
		R2_GLOBALS._player.enableControl();
		break;
	}

	R2_GLOBALS._player._oldCharacterScene[3] = 3250;
}

void Scene3250::signal() {
	switch(_sceneMode) {
	case 3250:
		R2_GLOBALS._player._effect = 1;
		R2_GLOBALS._player.enableControl();
		break;
	case 3254:
	//No break on purpose
	case 3262:
		R2_GLOBALS._sceneManager.changeScene(3255);
		break;
	case 3260:
		R2_GLOBALS._sceneManager.changeScene(3175);
		break;
	case 3261:
		R2_GLOBALS._sceneManager.changeScene(3125);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene3250::dispatch() {
	if ((R2_GLOBALS._player._visage == 3250) && (R2_GLOBALS._player._strip == 3) && (R2_GLOBALS._player._effect == 0)) {
		R2_GLOBALS._player._effect = 6;
		R2_GLOBALS._player._shade = 6;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3255 -
 *
 *--------------------------------------------------------------------------*/
void Scene3255::postInit(SceneObjectList *OwnerList) {
	loadScene(3255);
	SceneExt::postInit();

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS.setFlag(79);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS.getFlag(79)) {
		R2_GLOBALS._sound1.play(267);
		R2_GLOBALS._sound2.play(268);
		_sceneMode = 3257;
		_actor3.postInit();
		_actor4.postInit();
		_actor4._effect = 1;
		setAction(&_sequenceManager, this, 3257, &R2_GLOBALS._player, &_actor4, &_actor3, NULL);
	} else {
		_actor1.postInit();
		_actor1.setup(303, 1, 1);
		_actor1.setPosition(Common::Point(208, 128));
		_actor2.postInit();
		_actor2.setup(3107, 3, 1);
		_actor2.setPosition(Common::Point(230, 127));
		_sceneMode = 3255;
		setAction(&_sequenceManager, this, 3255, &R2_GLOBALS._player, NULL);
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3255;
}

void Scene3255::signal() {
	switch (_sceneMode) {
	case 10:
		_sceneMode = 3258;
		_actor5.postInit();
		_actor6.postInit();
		_actor7.postInit();
		setAction(&_sequenceManager, this, 3258, &R2_GLOBALS._player, &_actor4, &_actor3, &_actor5, &_actor6, &_actor7, NULL);
		break;
	case 3256:
		R2_GLOBALS._sceneManager.changeScene(3250);
		break;
	case 3257:
		_sceneMode = 10;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(607, this);
		break;
	case 3258:
		R2_GLOBALS._sceneManager.changeScene(3100);
		break;
	default:
		SceneItem::display(3255, 0, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		_sceneMode = 3256;
		setAction(&_sequenceManager, this, 3256, &R2_GLOBALS._player, NULL);
	}
}

void Scene3255::dispatch() {
	if (R2_GLOBALS.getFlag(79)) {
		if (_actor5._position.y >= 95) {
			if (_actor5._position.y <= 110)
				_actor5._shade = 6 - (_actor5._position.y - 95) / 3;
			else
				_actor5._effect = 1;
		} else {
			_actor5._effect = 6;
			_actor5._shade = 6;
		}

		if (_actor6._position.y >= 95) {
			if (_actor6._position.y <= 110)
				_actor6._shade = 6 - (_actor6._position.y - 95) / 3;
			else
				_actor6._effect = 1;
		} else {
			_actor6._effect = 6;
			_actor6._shade = 6;
		}

		if (_actor7._position.y >= 95) {
			if (_actor7._position.y <= 110)
				_actor7._shade = 6 - (_actor7._position.y - 95) / 3;
			else
				_actor7._effect = 1;
		} else {
			_actor7._effect = 6;
			_actor7._shade = 6;
		}
	}

	if ((R2_GLOBALS._player._position.x > 250) && (R2_GLOBALS._player._shade == 1)) {
		R2_GLOBALS._player._effect = 6;
		_actor4._effect = 6;
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 3260 - Computer room
 *
 *--------------------------------------------------------------------------*/
bool Scene3260::Actor13::startAction(CursorType action, Event &event) {
	Scene3260 *scene = (Scene3260 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3271;
	scene->setAction(&scene->_sequenceManager, scene, 3271, &R2_GLOBALS._player, &scene->_actor13, NULL);
	return true;
}

bool Scene3260::Actor14::startAction(CursorType action, Event &event) {
	Scene3260 *scene = (Scene3260 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 3272;
	scene->setAction(&scene->_sequenceManager, scene, 3272, &R2_GLOBALS._player, &scene->_actor14, NULL);
	return true;
}

void Scene3260::Action1::signal() {
	SceneObjectExt *fmtObj = (SceneObjectExt *) _endHandler;

	fmtObj->setFrame(R2_GLOBALS._randomSource.getRandomNumber(6));
	setDelay(120 + R2_GLOBALS._randomSource.getRandomNumber(179));
}

void Scene3260::postInit(SceneObjectList *OwnerList) {
	loadScene(3260);
	R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(285);

	_actor13.postInit();
	_actor13.setup(3260, 6, 1);
	_actor13.setPosition(Common::Point(40, 106));
	_actor13.setDetails(3260, 18, 1, -1, 1, NULL);

	if (R2_INVENTORY.getObjectScene(52) == 3260) {
		_actor14.postInit();
		_actor14.setup(3260, 7, 1);
		_actor14.setPosition(Common::Point(202, 66));
		_actor14.setDetails(3260, 12, 1, -1, 1, NULL);
	}

	_actor1.postInit();
	_actor1.setup(3260, 1, 1);
	_actor1.setPosition(Common::Point(93, 73));
	_actor1.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor1.setAction(&_action1, &_actor1);

	_actor2.postInit();
	_actor2.setup(3260, 2, 1);
	_actor2.setPosition(Common::Point(142, 63));
	_actor2.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor2.setAction(&_action2, &_actor2);

	_actor3.postInit();
	_actor3.setup(3260, 2, 1);
	_actor3.setPosition(Common::Point(166, 54));
	_actor3.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor3.setAction(&_action3, &_actor3);

	_actor4.postInit();
	_actor4.setup(3260, 2, 1);
	_actor4.setPosition(Common::Point(190, 46));
	_actor4.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor4.setAction(&_action4, &_actor4);

	_actor5.postInit();
	_actor5.setup(3260, 2, 1);
	_actor5.setPosition(Common::Point(142, 39));
	_actor5.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor5.setAction(&_action5, &_actor5);

	_actor6.postInit();
	_actor6.setup(3260, 2, 1);
	_actor6.setPosition(Common::Point(166, 30));
	_actor6.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor6.setAction(&_action6, &_actor6);

	_actor7.postInit();
	_actor7.setup(3260, 2, 1);
	_actor7.setPosition(Common::Point(190, 22));
	_actor7.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor7.setAction(&_action7, &_actor7);

	_actor8.postInit();
	_actor8.setup(3260, 2, 1);
	_actor8.setPosition(Common::Point(142, 14));
	_actor8.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor8.setAction(&_action8, &_actor8);

	_actor9.postInit();
	_actor9.setup(3260, 2, 1);
	_actor9.setPosition(Common::Point(166, 6));
	_actor9.setDetails(3260, 3, 1, 5, 1, NULL);
	_actor9.setAction(&_action9, &_actor9);

	_actor10.postInit();
	_actor10.setup(3260, 3, 1);
	_actor10.setPosition(Common::Point(265, 163));
	_actor10.fixPriority(180);
	_actor10._numFrames = 10;
	_actor10.setDetails(3260, 6, 1, 8, 1, NULL);
	_actor10.animate(ANIM_MODE_2, NULL);

	_actor11.postInit();
	_actor11.setup(3260, 4, 1);
	_actor11.setPosition(Common::Point(127, 108));
	_actor11.fixPriority(120);
	_actor11.setAction(&_action11, &_actor11);
	_actor11._numFrames = 15;
	_actor11.setDetails(3260, 6, 1, 8, 1, NULL);
	_actor11.animate(ANIM_MODE_2, NULL);

	_actor12.postInit();
	_actor12.setup(3260, 5, 1);
	_actor12.setPosition(Common::Point(274, 65));
	_actor12.setAction(&_action12, &_actor12);
	_actor12._numFrames = 5;
	_actor12.setDetails(3260, 9, 1, 11, 1, NULL);
	_actor12.animate(ANIM_MODE_2, NULL);

	_item1.setDetails(Rect(0, 0, 320, 200), 3260, 0, 1, 2, 1, NULL);
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._player._oldCharacterScene[3] == 3275) {
		_sceneMode = 3270;
		setAction(&_sequenceManager, this, 3270, &R2_GLOBALS._player, &_actor13, NULL);
	} else {
		R2_GLOBALS._player.setup(30, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(53, 113));
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[3] = 3260;
}

void Scene3260::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene3260::signal() {
	switch (_sceneMode) {
	case 3271:
		R2_GLOBALS._sceneManager.changeScene(3275);
		break;
	case 3272:
		_sceneMode = 3273;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		SceneItem::display(3260, 15, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
		R2_GLOBALS._player.disableControl();
		R2_INVENTORY.setObjectScene(52, 3);
		R2_INVENTORY.setObjectScene(43, 3);
		setAction(&_sequenceManager, this, 3273, &R2_GLOBALS._player, &_actor14, NULL);
		break;
	case 3273:
		_actor4.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
