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

#include "common/config-manager.h"
#include "tsage/blue_force/blueforce_scenes3.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 300 - Outside Police Station
 *
 *--------------------------------------------------------------------------*/

void Scene300::Object::startMover(CursorType action) {
	if (action == CURSOR_TALK) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		scene->_stripManager.start(_stripNumber, scene);
	} else {
		NamedObject::startMover(action);
	}
}

void Scene300::Object17::startMover(CursorType action) {
	if ((action != CURSOR_USE) || !BF_GLOBALS.getFlag(3)) {
		NamedObject::startMover(action);
	} else if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark >= bEndDayOne)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		setAction(&scene->_action4);
	} else {
		SceneItem::display2(300, 33);
	}
}

void Scene300::Item1::startMover(CursorType action) {
	if (action == CURSOR_TALK) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 305;
		scene->setAction(&scene->_sequenceManager1, scene, 305, &BF_GLOBALS._player,
			&scene->_object8, NULL);
	} else {
		NamedHotspot::startMover(action);
	}
}

void Scene300::Item2::startMover(CursorType action) {
	if ((action == CURSOR_LOOK) || (action == CURSOR_USE)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		scene->setAction(&scene->_sequenceManager1, scene, 304, &scene->_object11, NULL);
	} else {
		NamedHotspot::startMover(action);
	}
}

void Scene300::Item14::startMover(CursorType action) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 151, 54);
}

void Scene300::Item15::startMover(CursorType action) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 316, 90);
}

/*--------------------------------------------------------------------------*/

void Scene300::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		if (BF_GLOBALS.getFlag(7))
			SceneItem::display2(300, 0);
		else
			SceneItem::display2(666, 27);
		setDelay(1);
		break;
	case 2: {
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 8, 
			BF_GLOBALS._player._position.y);
		break;
	}
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene300::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		SceneItem::display2(300, 28);
		setDelay(1);
		break;
	case 2: {
		ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 8,
			BF_GLOBALS._player._position.y);
		break;
	}
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;		
	default:
		break;
	}
}

void Scene300::Action3::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager1, this, 306, &BF_GLOBALS._player,
			&scene->_object8, NULL);
		break;
	case 2:
		SceneItem::display2(300, 35);
		setDelay(1);
		break;
	case 3:
		BF_GLOBALS._player.enableControl();
		remove();
		break;		
	default:
		break;
	}
}

void Scene300::Action4::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 316, &BF_GLOBALS._player, &scene->_object19, NULL);
		break;
	case 2:
		BF_GLOBALS._sceneManager.changeScene(15);
		break;
	case 3:
		setAction(&scene->_sequenceManager1, this, 319, &scene->_object19, NULL);
		break;
	case 4:
		BF_GLOBALS.setFlag(2);
		BF_GLOBALS._sceneManager.changeScene(190);
		break;		
	default:
		break;
	}
}

void Scene300::Action5::signal() {
	Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		scene->_field2760 = 1;
		setDelay(1);
		break;
	case 1:
		setAction(&scene->_sequenceManager1, this, 1306, &scene->_object1, &scene->_object8, NULL);
		break;
	case 2:
		scene->_stripManager.start(3004, this);
		BF_GLOBALS._sceneManager.changeScene(15);
		break;
	case 3: {
		ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 186, 140);
		break;
	}
	case 4:
		remove();
		break;		
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene300::Scene300(): SceneExt(), _object13(3000), _object14(3001), _object15(3002),
			_object16(3003) {
	_field2760 = _field2762 = 0;
}

void Scene300::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(300);

	// Add the speakers
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_sutterSpeaker);
	_stripManager.addSpeaker(&_dougSpeaker);
	_stripManager.addSpeaker(&_jakeSpeaker);

	_field2762 = 0;
	_item14.setup(Rect(144, 27, 160, 60), 300, -1, -1, -1, 1, NULL);
	_item15.setup(Rect(310, 76, SCREEN_WIDTH, 105), 300, -1, -1, -1, 1, NULL);

	// Setup the player
	int playerVisage = BF_GLOBALS._player._visage;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(playerVisage);
	BF_GLOBALS._player.setStrip(3);
	BF_GLOBALS._player.setPosition(Common::Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player._moveDiff = Common::Point(3, 1);
	BF_GLOBALS._player.disableControl();

	_object8.postInit();
	_object8.setVisage(301);
	_object8.setStrip(2);
	_object8.setPosition(Common::Point(300, 77));

	if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark < bEndDayOne)) {
		_object17.postInit();
		_object17.setVisage(301);
		_object17.setStrip(1);
		_object17.setPosition(Common::Point(87, 88));
		_object17.setup(300, 11, 13, 2, 1, NULL);
		
		_object18.postInit();
		_object18.setVisage(301);
		_object18.setStrip(1);
		_object18.setPosition(Common::Point(137, 92));
		_object18.setup(300, 11, 13, 3, 1, NULL);
	}

	_object19.postInit();
	_object19.setVisage(301);
	_object19.setStrip(1);
	_object19.setPosition(Common::Point(175, 99));
	_object19.setup(300, 11, 13, 34, 1, NULL);

	_object11.postInit();
	_object11.setVisage(301);
	_object11.setStrip(8);
	_object11.setPosition(Common::Point(265, 91));
	_object11.hide();

 //***DEBUG***
BF_GLOBALS.setFlag(2);
BF_GLOBALS._sceneManager._previousScene = 315; // 190;
BF_GLOBALS._player.setVisage(190);
BF_GLOBALS._player.setStrip(1);

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
		BF_GLOBALS.clearFlag(onBike);
		if (BF_GLOBALS.getFlag(3)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 318;
			setAction(&_sequenceManager1, this, 318, &BF_GLOBALS._player, &_object19, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 300;
			setAction(&_sequenceManager1, this, 1300, &BF_GLOBALS._player, NULL);
		}
		break;
	case 190:
		_sceneMode = 0;
		if (!BF_GLOBALS.getFlag(2)) {
			_sceneMode = 7308;
			BF_GLOBALS._player.setPosition(Common::Point(175, 50));
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 123, 71);

			if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._bookmark < bEndDayOne))
				setup();
		} else if (!BF_GLOBALS.getFlag(3)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 300;
			setAction(&_sequenceManager1, this, 300, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 318;
			setAction(&_sequenceManager1, this, 318, &BF_GLOBALS._player, &_object19, NULL);
		}
		break;
	case 315:
		BF_GLOBALS._player.setPosition(Common::Point(305, 66));
		if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark >= bEndDayOne)) {
			BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1304 : 303);
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		} else {
			BF_GLOBALS._player.setVisage(1304);
			setup();
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		}
		break;
	default:
		_sceneMode = 0;
		BF_GLOBALS._player.setVisage(1304);
		BF_GLOBALS._player.disableControl();
		setAction(&_sequenceManager1, this, 306, &BF_GLOBALS._player, &_object8, NULL);
		break;
	}
}

void Scene300::signal() {
	switch (_sceneMode) {
	case 300:
		BF_GLOBALS._sound1.fadeSound(33);
		BF_GLOBALS.clearFlag(onBike);
		_sceneMode = 0;

		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark != bNone)) {
			signal();
		} else {
			_stripManager.start(3005, this);
		}
		break;
	case 301:
		if (_field2760) {
			_sceneMode = 1302;
			signal();
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 1302;
			setAction(&_sequenceManager1, this, 306, &_object1, &_object8, NULL);
		}

		_object12.show();
		_object5.dispatch();
		BF_GLOBALS._player.hide();
		break;
	case 303:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 2307;
		setAction(&_sequenceManager1, this, 303, &_object13, &_object1, NULL);
		break;
	case 305:
		if ((BF_GLOBALS._dayNumber == 4) || (BF_GLOBALS._dayNumber == 5)) {
			_sceneMode = 0;
			setAction(&_action3);
		} else {
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._sceneManager.changeScene(315);
		}
		break;
	case 309:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3307;
		setAction(&_sequenceManager1, this, 309, &_object14, &_object1, NULL);
		break;
	case 310:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4307;
		setAction(&_sequenceManager1, this, 310, &_object12, &_object1, NULL);
		break;
	case 311:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 5307;
		setAction(&_sequenceManager1, this, 311, &_object15, &_object1, NULL);
		break;
	case 312:
	case 5307:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1305;
		setAction(&_sequenceManager1, this, 312, &_object1, &_object16, NULL);
		break;
	case 317:
		BF_GLOBALS.setFlag(2);
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 318:
		BF_GLOBALS.clearFlag(onBike);
		_sceneMode = 0;
		signal();
		break;
	case 1302:
		_field2762 = 0;
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1308;
		setAction(&_sequenceManager1, this, 302, &_object1, NULL);
		break;
	case 1305:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1313;
		setAction(&_sequenceManager1, this, 305, &_object1, &_object8, NULL);
		BF_GLOBALS._player.show();
		_object12.hide();
		break;
	case 1307:
	case 2308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 303;
		setAction(&_sequenceManager1, this, 308, &_object14, NULL);
		break;
	case 1308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1307;
		setAction(&_sequenceManager1, this, 308, &_object13, NULL);
		break;
	case 1313:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		_object15.setAction(&_sequenceManager4, NULL, 315, &_object15, &_object16, NULL);
		_object13.setAction(&_sequenceManager2, NULL, 313, &_object13, &_object17, NULL);
		_object14.setAction(&_sequenceManager3, this, 314, &_object14, &_object18, NULL);

		BF_GLOBALS._bookmark = bEndDayOne;
		BF_GLOBALS._sound1.changeSound(33);
		break;
	case 2307:
	case 3308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 309;
		setAction(&_sequenceManager1, this, 308, &_object12, NULL);
		break;
	case 3307:
		_object9.postInit();
		_object9.hide();
		_object10.postInit();
		_object10.hide();
		
		if (BF_GLOBALS.getFlag(1)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 4308;
			setAction(&_sequenceManager1, this, 6307, &_object2, &_object1, &_object9, &_object10, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 4308;
			setAction(&_sequenceManager1, this, 7307, &_object12, &_object1, &_object9, &_object10, NULL);
		}
		break;
	case 4307:
	case 5308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 311;
		setAction(&_sequenceManager1, this, 308, &_object16, NULL);
		break;
	case 4308:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 310;
		setAction(&_sequenceManager1, this, 308, &_object15, NULL);
		break;
	case 6308:
		BF_GLOBALS._sceneManager.changeScene(190);
		break;
	case 7308:
		if (_field2762) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 0:
	default:
		if (_field2762) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
		}
		break;
	}
}

void Scene300::process(Event &event) {
	SceneExt::process(event);

	if ((BF_GLOBALS._player._field8E != 0) && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		Visage visage;

		if (_item14.contains(event.mousePos)) {
			visage.setVisage(1, 8);
			GfxSurface surface = visage.getFrame(2);
			BF_GLOBALS._events.setCursor(surface);
		} else if (_item15.contains(event.mousePos)) {
			visage.setVisage(1, 8);
			GfxSurface surface = visage.getFrame(3);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			CursorType cursorId = BF_GLOBALS._events.hideCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene300::dispatch() {
	SceneExt::dispatch();

	if (_action) {
		int regionIndex = BF_GLOBALS._player.getRegionIndex();
		if ((regionIndex == 1) && (_field2762 == 1)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 301;
			setAction(&_sequenceManager1, this, 301, &BF_GLOBALS._player, NULL);
		}

		if ((BF_GLOBALS._player._position.y < 59) && (BF_GLOBALS._player._position.x > 137) &&
				(_sceneMode != 6308) && (_sceneMode != 7308)) {
			BF_GLOBALS._v4CEA4 = 3;
			_sceneMode = 6308;
			BF_GLOBALS._player.disableControl();
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 20, 
				BF_GLOBALS._player._position.y - 5);
		}
	}
}

void Scene300::setup() {
	_object13.postInit();
	_object13.setVisage(307);
	_object13.setStrip(6);
	_object13.setPosition(Common::Point(156, 134));
	_object13._moveDiff = Common::Point(3, 1);
	_object3.setup(&_object13, 306, 1, 29);

	_object14.postInit();
	_object14.setVisage(307);
	_object14.setStrip(6);
	_object14.setPosition(Common::Point(171, 137));
	_object14._moveDiff = Common::Point(3, 1);
	_object4.setup(&_object14, 306, 2, 29);

	_object12.postInit();
	_object12.setVisage(307);
	_object12.setStrip(6);
	_object12.setPosition(Common::Point(186, 140));
	_object12._moveDiff = Common::Point(3, 1);
	_object5.setup(&_object12, 306, 2, 29);
	_object12.hide();

	_object15.postInit();
	_object15.setVisage(307);
	_object15.setStrip(6);
	_object15.setPosition(Common::Point(201, 142));
	_object15._moveDiff = Common::Point(3, 1);
	_object6.setup(&_object15, 306, 3, 29);

	_object16.postInit();
	_object16.setVisage(307);
	_object16.setStrip(6);
	_object16.setPosition(Common::Point(216, 145));
	_object16._moveDiff = Common::Point(3, 1);
	_object7.setup(&_object16, 306, 1, 29);

	_object1.postInit();
	_object1.setVisage(307);
	_object1.setStrip(6);
	_object1.setPosition(Common::Point(305, 66));
	_object1._moveDiff = Common::Point(3, 1);
	_object1.setObjectWrapper(new SceneObjectWrapper());
	_object1.animate(ANIM_MODE_1, NULL);
	_object2.setup(&_object1, 306, 4, 9);

	BF_GLOBALS._sceneItems.addItems(&_object13, &_object14, &_object15, &_object16, NULL);
	_timer.set(3600, this, &_action5);

	_field2760 = 0;
	_field2762 = 1;
}

} // End of namespace BlueForce

} // End of namespace TsAGE
