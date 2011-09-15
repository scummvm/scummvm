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

bool Scene300::Object::startAction(CursorType action, Event &event) {
	if (action == CURSOR_TALK) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		scene->_stripManager.start(_stripNumber, scene);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene300::Object19::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || !BF_GLOBALS.getFlag(onDuty)) {
		return NamedObject::startAction(action, event);
	} else if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._bookmark >= bEndDayOne)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		setAction(&scene->_action4);
	} else {
		SceneItem::display2(300, 33);
	}

	return true;
}

bool Scene300::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 305;
		scene->setAction(&scene->_sequenceManager1, scene, 305, &BF_GLOBALS._player,
			&scene->_object8, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene300::Item2::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_LOOK) || (action == CURSOR_USE)) {
		Scene300 *scene = (Scene300 *)BF_GLOBALS._sceneManager._scene;
		scene->setAction(&scene->_sequenceManager1, scene, 304, &scene->_object11, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene300::Item14::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 151, 54);
	return true;
}

bool Scene300::Item15::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 316, 90);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene300::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(1);
		break;
	case 1:
		if (BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(666, 27);
		else
			SceneItem::display2(300, 0);
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
		BF_GLOBALS._sceneManager.changeScene(60);
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
	_item14.setDetails(Rect(144, 27, 160, 60), 300, -1, -1, -1, 1, NULL);
	_item15.setDetails(Rect(310, 76, SCREEN_WIDTH, 105), 300, -1, -1, -1, 1, NULL);

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
		_object17.setDetails(300, 11, 13, 2, 1, NULL);
		
		_object18.postInit();
		_object18.setVisage(301);
		_object18.setStrip(1);
		_object18.setPosition(Common::Point(137, 92));
		_object18.setDetails(300, 11, 13, 3, 1, NULL);
	}

	_object19.postInit();
	_object19.setVisage(301);
	_object19.setStrip(1);
	_object19.setPosition(Common::Point(175, 99));
	_object19.setDetails(300, 11, 13, 34, 1, NULL);

	_object11.postInit();
	_object11.setVisage(301);
	_object11.setStrip(8);
	_object11.setPosition(Common::Point(265, 91));
	_object11.hide();

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
		BF_GLOBALS.clearFlag(onBike);
		if (BF_GLOBALS.getFlag(onDuty)) {
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

	_item10.setDetails(4, 300, 7, 13, 16, 1);
	_item11.setDetails(2, 300, 9, 13, 18, 1);
	_item12.setDetails(5, 300, 10, 13, 19, 1);
	_item13.setDetails(3, 300, 25, 26, 27, 1);
	_item2.setDetails(Rect(266, 54, 272, 59), 300, -1, -1, -1, 1, NULL);
	_item1.setDetails(Rect(262, 47, 299, 76), 300, 1, 13, -1, 1, NULL);
	_item4.setDetails(Rect(0, 85, SCREEN_WIDTH - 1, BF_INTERFACE_Y - 1), 300, 6, 13, 15, 1, NULL);
	_item7.setDetails(Rect(219, 46, 251, 74), 300, 22, 23, 24, 1, NULL);
	_item8.setDetails(Rect(301, 53, 319, 78), 300, 22, 23, 24, 1, NULL);
	_item5.setDetails(Rect(179, 44, 200, 55), 300, 8, 13, 17, 1, NULL);
	_item6.setDetails(Rect(210, 46, 231, 55), 300, 8, 13, 17, 1, NULL);
	_item3.setDetails(Rect(160, 0, SCREEN_WIDTH - 1, 75), 300, 4, 13, 14, 1, NULL);
	_item9.setDetails(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y), 300, 29, 30, 31, 1, NULL);
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

	if (BF_GLOBALS._player._enabled && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_item14.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_NE);
			BF_GLOBALS._events.setCursor(surface);
		} else if (_item15.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_E);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene300::dispatch() {
	SceneExt::dispatch();

	if (!_action) {
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

		if (BF_GLOBALS._player._position.x <= 5)
			setAction(&_action2);

		if (BF_GLOBALS._player._position.x >= 315) {
			if (BF_GLOBALS.getFlag(onDuty) || (BF_GLOBALS._bookmark == bNone) || !BF_GLOBALS.getFlag(fWithLyle)) {
				setAction(&_action1);
			} else {
				BF_GLOBALS._player.disableControl();
				_sceneMode = 317;
				setAction(&_sequenceManager1, this, 1301, &BF_GLOBALS._player, NULL);
			}
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

/*--------------------------------------------------------------------------
 * Scene 315 - Inside Police Station
 *
 *--------------------------------------------------------------------------*/

bool Scene315::Item1::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;
	scene->_currentCursor = action;

	switch (action) {
	case CURSOR_USE:
		if (scene->_field1B60 || scene->_field1B64)
			SceneItem::display2(320, 51);
		else
			NamedHotspot::startAction(action, event);
		break;
	case CURSOR_TALK:
		if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._sceneManager._previousScene == 325))
			NamedHotspot::startAction(action, event);
		else {
			if (!BF_GLOBALS.getFlag(onDuty))
				scene->_stripNumber = 3172;
			else if (BF_GLOBALS.getFlag(fTalkedToBarry))
				scene->_stripNumber = 3166;
			else if (BF_GLOBALS.getFlag(fTalkedToLarry))
				scene->_stripNumber = 3164;
			else
				scene->_stripNumber = 3165;

			scene->setAction(&scene->_action1);
			BF_GLOBALS.setFlag(fTalkedToBarry);
		}
		break;
	case INV_GREENS_GUN:
	case INV_GREENS_KNIFE:
		BF_GLOBALS._player.disableControl();
		if (BF_INVENTORY._bookingGreen._sceneNumber != 390) {
			scene->_stripNumber = 3174;
			scene->setAction(&scene->_action1);
		} else {
			++scene->_field1B62;
			scene->_stripNumber = (action == INV_GREENS_GUN) ? 3168 : 0;
			scene->_sceneMode = 3152;
			scene->setAction(&scene->_sequenceManager, scene, 3153, 1888, NULL);
		} 
		break;
	case INV_FOREST_RAP:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = BF_GLOBALS.getFlag(onDuty) ? 3178 : 3173;
		scene->setAction(&scene->_action1);
		break;
	case INV_GREEN_ID:
	case INV_FRANKIE_ID:
	case INV_TYRONE_ID:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3175;
		scene->setAction(&scene->_action1);
		break;
	case INV_BOOKING_GREEN:
	case INV_BOOKING_FRANKIE:
	case INV_BOOKING_GANG:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3167;
		scene->setAction(&scene->_action1);
		break;
	case INV_COBB_RAP:
		if (BF_INVENTORY._mugshot._sceneNumber == 1)
			NamedHotspot::startAction(action, event);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3169;
			if (BF_GLOBALS._dayNumber > 2)
				scene->_stripNumber = 3176;
			else if (BF_GLOBALS.getFlag(onDuty))
				scene->_stripNumber = 3177;
			else
				scene->_stripNumber = 3170;
			scene->setAction(&scene->_action1);
		}
		break;
	case INV_22_BULLET:
	case INV_AUTO_RIFLE:
	case INV_WIG:
	case INV_22_SNUB:
		BF_GLOBALS._player.disableControl();
		if ((BF_GLOBALS.getFlag(fCuffedFrankie) && (BF_INVENTORY._bookingFrankie._sceneNumber == 0)) ||
				(!BF_GLOBALS.getFlag(fCuffedFrankie) && (BF_INVENTORY._bookingGang._sceneNumber == 0))) {
			scene->_stripNumber = 3174;
			scene->setAction(&scene->_action1);
		} else {
			if (!scene->_field1B6C & (scene->_field1B66 == 1)) {
				scene->_field1B6C = 1;
				scene->_stripNumber = 3169;
			} else {
				scene->_stripNumber = 0;
			}

			scene->_sceneMode = 3153;
			scene->setAction(&scene->_sequenceManager, scene, 3153, &BF_GLOBALS._player, NULL);
		}
		break;
	default:
		return NamedHotspot::startAction(action, event);
	}

	return true;
}

bool Scene315::Item2::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case INV_GREENS_GUN:
	case INV_22_BULLET:
	case INV_AUTO_RIFLE:
	case INV_WIG:
	case INV_22_SNUB:
		SceneItem::display2(315, 30);
		break;
	case INV_GREEN_ID:
	case INV_FRANKIE_ID:
	case INV_TYRONE_ID:
		BF_GLOBALS._player.disableControl();
		scene->_stripNumber = 3175;
		scene->setAction(&scene->_action1);
		break;
	case INV_BOOKING_GREEN:
	case INV_BOOKING_FRANKIE:
	case INV_BOOKING_GANG:
		if (action == INV_BOOKING_GREEN)
			++scene->_field1B62;
		else
			++scene->_field1B66;

		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 12;
		scene->setAction(&scene->_sequenceManager, scene, 3154, &BF_GLOBALS._player, NULL);
		break;
	default:
		return NamedHotspot::startAction(action, event);
	}

	return true;
}

bool Scene315::Item4::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.addMover(NULL);
		scene->_object9.postInit();
		scene->_object9.hide();
		scene->_sceneMode = 3167;
		scene->setAction(&scene->_sequenceManager, scene, 3167, &scene->_object9, this, NULL);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::Item5::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_LOOK) {
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3154, &BF_GLOBALS._stripProxy);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::Item14::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if ((action == INV_COLT45) && BF_GLOBALS.getFlag(onDuty)) {
		if (!BF_GLOBALS.getFlag(onDuty))
			SceneItem::display2(315, 27);
		else if (BF_GLOBALS.getHasBullets()) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3162;
			scene->setAction(&scene->_sequenceManager, scene, 3162, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS.getFlag(fGunLoaded))
			SceneItem::display2(315, 46);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 3159;
			scene->setAction(&scene->_sequenceManager, scene, 3159, &BF_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene315::Item15::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return NamedHotspot::startAction(action, event);
	else if (BF_INVENTORY._forestRap._sceneNumber == 1) {
		SceneItem::display2(315, 37);
		return true;
	} else {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3158;
		scene->setAction(&scene->_sequenceManager, scene, 3158, &BF_GLOBALS._player, NULL);
		return true;
	}
}

bool Scene315::Item16::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 190, 75);
	return true;
}

bool Scene315::Item17::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, event.mousePos.x, event.mousePos.y);
	return true;
}

/*--------------------------------------------------------------------------*/

bool Scene315::Object1::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		scene->_stripManager.start(3157, &BF_GLOBALS._stripProxy);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fGotPointsForCleaningGun)) {
			BF_GLOBALS._uiElements.addScore(10);
			BF_GLOBALS.setFlag(fGotPointsForCleaningGun);
		}
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3159, &BF_GLOBALS._stripProxy);
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene315::Object2::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		BF_GLOBALS._player.disableControl();
		scene->_object9.postInit();
		scene->_object9.hide();
		scene->_sceneMode = 3157;
		scene->setAction(&scene->_sequenceManager, scene, 3157, &BF_GLOBALS._player, &scene->_object9, NULL);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 3156;
		scene->setAction(&scene->_sequenceManager, scene, BF_GLOBALS.getFlag(onDuty) ? 3156 : 3168,
			&BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene315::Object3::startAction(CursorType action, Event &event) {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3156, &BF_GLOBALS._stripProxy);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fGotPointsForMemo)) {
			BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fGotPointsForMemo);
		}

		BF_GLOBALS._player.addMover(NULL);
		scene->_stripManager.start(3158, &BF_GLOBALS._stripProxy);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene315::Action1::signal() {
	Scene315 *scene = (Scene315 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 128, 128);
		break;
	case 1:
		BF_GLOBALS._player.changeAngle(315);
		setDelay(2);
		break;
	case 2:
		scene->_stripManager.start(scene->_stripNumber, this);
		break;
	case 3:
		if (scene->_sceneMode == 3169) {
			BF_GLOBALS._uiElements.addScore(30);
			BF_INVENTORY.setObjectScene(INV_MUG_SHOT, 1);
		}

		remove();
		BF_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene315::Scene315() {
	BF_GLOBALS._v51C44 = 1;
	_field1B6C = _field139C = 0;
	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	BF_GLOBALS.clearFlag(fCanDrawGun);
	_field1B68 = true;
	_field1B6A = false;
	_field1B60 = _field1B62 = 0;
	_field1B64 = _field1B66 = 0;
}

void Scene315::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field1390);
	s.syncAsSint16LE(_stripNumber);
	s.syncAsSint16LE(_field1398);
	s.syncAsSint16LE(_field1B60);
	s.syncAsSint16LE(_field1B62);
	s.syncAsSint16LE(_field1B64);
	s.syncAsSint16LE(_field1B66);
	s.syncAsSint16LE(_field1B6C);
	s.syncAsSint16LE(_field139C);
	s.syncAsByte(_field1B68);
	s.syncAsByte(_field1B6A);
	s.syncAsSint16LE(_currentCursor);
}

void Scene315::postInit(SceneObjectList *OwnerList) {
	loadScene(315);
	
	if (BF_GLOBALS._sceneManager._previousScene != 325)
		BF_GLOBALS._sound1.fadeSound(11);

	setZoomPercents(67, 72, 124, 100);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_sutterSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_jailerSpeaker);

	_object8.postInit();
	_object8.setVisage(315);
	_object8.setPosition(Common::Point(272, 69));

	if (BF_GLOBALS._bookmark >= bLauraToParamedics) {
		_object3.postInit();
		_object3.setVisage(315);
		_object3.setPosition(Common::Point(167, 53));
		_object3.setStrip(4);
		_object3.setFrame(4);
		_object3.fixPriority(82);
		_object3.setDetails(315, -1, -1, -1, 1, NULL);
	}

	if (BF_GLOBALS._dayNumber == 1) {
		if (BF_GLOBALS._bookmark >= bLauraToParamedics) {
			_object1.postInit();
			_object1.setVisage(315);
			_object1.setPosition(Common::Point(156, 51));
			_object1.setStrip(4);
			_object1.setFrame(2);
			_object1.fixPriority(82);
			_object1.setDetails(315, -1, -1, -1, 1, NULL);
		}
	} else if ((BF_INVENTORY._daNote._sceneNumber != 1) && (BF_GLOBALS._dayNumber < 3)) {
		_object2.postInit();
		_object2.setVisage(315);
		_object2.setStrip(3);
		_object2.setFrame(2);
		_object2.setPosition(Common::Point(304, 31));
		_object2.fixPriority(70);
		_object2.setDetails(315, 3, 4, -1, 1, NULL);
	}

	_item2.setDetails(12, 315, 35, -1, 36, 1);
	_item5.setDetails(3, 315, -1, -1, -1, 1);
	_item1.setDetails(4, 315, 10, 11, 12, 1);
	_item3.setDetails(2, 315, 0, 1, 2, 1);
	_item4.setDetails(Rect(190, 17, 208, 30), 315, -1, -1, -1, 1, NULL);
	_item16.setDetails(Rect(184, 31, 211, 80), 315, -1, -1, -1, 1, NULL);
	_item17.setDetails(Rect(0, 157, 190, 167), 315, -1, -1, -1, 1, NULL);
	
	if (!BF_GLOBALS.getFlag(onDuty) && ((BF_GLOBALS._bookmark == bNone) || (BF_GLOBALS._bookmark == bLyleStoppedBy))) {
		_field1398 = 1;
		BF_GLOBALS.setFlag(onDuty);
	} else {
		_field1398 = 0;
	}
	
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();

	if ((BF_GLOBALS._dayNumber != 2) || (BF_GLOBALS._sceneManager._previousScene != 325)) {
		_object4.postInit();
		_object4.setVisage(316);
		_object4.setPosition(Common::Point(99, 82));
		_object4.fixPriority(95);

		_object5.postInit();
		_object5.setVisage(395);
		_object5.setStrip(2);
		_object5.setPosition(Common::Point(96, 86));
	}

	// Set up evidence objects in inventory
	if (BF_INVENTORY._bookingGreen.inInventory())
		++_field1B60;
	if (BF_INVENTORY._greensGun.inInventory())
		++_field1B60;
	if (BF_INVENTORY._greensKnife.inInventory())
		++_field1B60;

	if (BF_INVENTORY._bullet22.inInventory())
		++_field1B64;
	if (BF_INVENTORY._autoRifle.inInventory())
		++_field1B64;
	if (BF_INVENTORY._wig.inInventory())
		++_field1B64;
	if (BF_INVENTORY._bookingFrankie.inInventory())
		++_field1B64;
	if (BF_INVENTORY._bookingGang.inInventory())
		++_field1B64;
	if (BF_INVENTORY._snub22.inInventory())
		++_field1B64;

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 190:
		if (_field1398)
			_field1B6A = true;
		_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 3150 : 3165;
		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, NULL);
		break;
	case 325:
		BF_GLOBALS._uiElements._active = false;
		_object6.postInit();
		_object7.postInit();
		_object8.setFrame(8);
		_sceneMode = (BF_GLOBALS._dayNumber == 1) ? 3152 : 3155;
		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, &_object6, 
			&_object7, &_object8, NULL);
		break;
	case 300:
	default:
		if (_field1398)
			_field1B6A = true;
		if (!BF_GLOBALS.getFlag(onDuty))
			_sceneMode = 3166;
		else if (!_field1398)
			_sceneMode = 3164;
		else
			_sceneMode = 3163;

		setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, NULL);
		break;
	}

	if (_field1B6A) {
		_object8.setFrame(8);
	} else {
		BF_GLOBALS._walkRegions.proc1(4);
	}

	_item15.setDetails(24, 315, 38, 39, 40, 1);
	_item14.setDetails(14, 315, 24, 25, 26, 1);
	_item7.setDetails(5, 315, 8, 9, -1, 1);
	_item6.setDetails(6, 315, 5, 6, 7, 1);
	_item10.setDetails(8, 315, 13, -1, -1, 1);
	_item11.setDetails(9, 315, 14, -1, -1, 1);
	_item8.setDetails(7, 315, 15, 16, 17, 1);
	_item9.setDetails(10, 315, 18, 19, 20, 1);
}

void Scene315::signal() {
	int ctr = 0;

	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 10:
		if (_field1B62) {
			if (_field1B62 >= _field1B60)
				BF_GLOBALS.setFlag(fLeftTraceIn910);
			else
				++ctr;
		}

		if (_field1B66) {
			if (_field1B66 < _field1B64)
				++ctr;
			else if (BF_GLOBALS._bookmark < bBookedFrankieEvidence)
				BF_GLOBALS._bookmark = bBookedFrankieEvidence;
		}

		if (ctr) {
			BF_GLOBALS._deathReason = 20;
			BF_GLOBALS._sceneManager.changeScene(666);
		} else {
			BF_GLOBALS._sceneManager.changeScene(300);
		}
		BF_GLOBALS._sound1.fadeOut2(NULL);
		break;
	case 11:
		if (_field1B62) {
			if (_field1B62 >= _field1B60)
				BF_GLOBALS.setFlag(fLeftTraceIn910);
			else
				++ctr;
		}

		if (_field1B66) {
			if (_field1B66 < _field1B64)
				++ctr;
			else if (BF_GLOBALS._bookmark < bBookedFrankie)
				BF_GLOBALS._bookmark = bBookedFrankie;
			else if (BF_GLOBALS._bookmark < bBookedFrankieEvidence)
				BF_GLOBALS._bookmark = bBookedFrankie;
		}

		if (ctr == 1) {
			BF_GLOBALS._deathReason = 20;
			BF_GLOBALS._sound1.fadeOut2(NULL);
		} else if ((BF_GLOBALS._bookmark != bBookedFrankie) || !BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._sceneManager.changeScene(190);
		} else {
			BF_GLOBALS._bookmark = bBookedFrankieEvidence;
			_field139C = 0;
			BF_GLOBALS.clearFlag(onDuty);
			BF_INVENTORY.setObjectScene(INV_TICKET_BOOK, 60);
			BF_INVENTORY.setObjectScene(INV_MIRANDA_CARD, 60);
			_sceneMode = 3165;
			setAction(&_sequenceManager, this, 3165, &BF_GLOBALS._player, NULL);
		}
		break;
	case 12:
		BF_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene((int)_currentCursor, 315);

		if (!_field1B64 || (_field1B66 != _field1B64))
			BF_GLOBALS._player.enableControl();
		else {
			_field139C = 1;
			_stripNumber = 3171;
			setAction(&_action1);
		}
		break;
	case 3150:
	case 3164:
	case 3165:
	case 3166:
		BF_GLOBALS._player.enableControl();
		_field1B68 = false;
		break;
	case 3151:
		BF_GLOBALS._sceneManager.changeScene(325);
		break;
	case 3152:
		BF_GLOBALS._walkRegions.proc1(4);
		_object7.remove();
		_object6.remove();

		BF_GLOBALS._player.enableControl();
		_field1B68 = false;
		BF_GLOBALS._walkRegions.proc1(4);
		BF_GLOBALS._uiElements._active = true;
		BF_GLOBALS._uiElements.show();
		break;
	case 3153:
		BF_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene((int)_currentCursor, 315);
		
		if (_stripNumber != 0)
			setAction(&_action1);
		else if (!_field1B64 || (_field1B66 != _field1B64))
			BF_GLOBALS._player.enableControl();
		else {
			_stripNumber = 3171;
			setAction(&_action1);
			_field139C = 1;
		}
		break;
	case 3155:
		BF_GLOBALS._player.enableControl();
		_field1B68 = false;
		BF_GLOBALS._walkRegions.proc1(4);
		BF_GLOBALS._uiElements._active = true;
		BF_GLOBALS._uiElements.show();
		break;
	case 3156:
		BF_GLOBALS._uiElements.addScore(10);
		BF_INVENTORY.setObjectScene(INV_DA_NOTE, 1);
		_object2.remove();
		BF_GLOBALS._player.enableControl();
		break;
	case 3157:
		BF_GLOBALS._player.enableControl();
		_object9.remove();
		break;
	case 3158:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._uiElements.addScore(10);
		BF_INVENTORY.setObjectScene(INV_FOREST_RAP, 1);
		break;
	case 3159:
		if (!BF_GLOBALS.getFlag(fBookedGreenEvidence)) {
			BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(fBookedGreenEvidence);
		}
		BF_GLOBALS.setFlag(gunClean);
		BF_GLOBALS._player.enableControl();
		break;
	case 3161:
		BF_GLOBALS._deathReason = 21;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 3162:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3161;
		setAction(&_sequenceManager, this, 3161, &BF_GLOBALS._player, NULL);
		BF_GLOBALS.setFlag(fShotSuttersDesk);
		break;
	case 3163:
		_sceneMode = 3150;
		setAction(&_sequenceManager, this, 3150, &BF_GLOBALS._player, NULL);
		break;
	case 3167:
		BF_GLOBALS._player.enableControl();
		_object9.remove();
		break;
	case 3154:
	default:
		break;
	}
}

void Scene315::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_item17.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
			BF_GLOBALS._events.setCursor(surface);
		} else if ((BF_GLOBALS._bookmark != bBookedFrankie) && _item16.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene315::dispatch() {
	SceneExt::dispatch();

	if (_field1B68)
		return;
	
	if (_field1B6A) {
		if (BF_GLOBALS._player._position.y < 69) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			_sceneMode = 3151;
			setAction(&_sequenceManager, this, 3151, &BF_GLOBALS._player, NULL);
		} else if (BF_GLOBALS._player.getRegionIndex() == 1) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			SceneItem::display2(315, 28);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 30, 
				BF_GLOBALS._player._position.y + 15);
		} else if (BF_GLOBALS._player._position.y > 156) {
			BF_GLOBALS._player.disableControl();
			_field1B68 = true;
			SceneItem::display2(315, 28);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x + 30,
				BF_GLOBALS._player._position.y - 24);
		}	
	}  else if (BF_GLOBALS._player.getRegionIndex() == 1) {
		BF_GLOBALS._player.disableControl();
		_field1B68 = true;
		_sceneMode = 11;
		ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 30,
			BF_GLOBALS._player._position.y - 5);
	} else if (BF_GLOBALS._player._position.y > 156) {
		BF_GLOBALS._player.disableControl();
		_field1B68 = true;

		if (_field139C) {
			SceneItem::display2(315, 45);
			_sceneMode = 3150;
			ADD_MOVER(BF_GLOBALS._player, 112, 152);
		} else {
			_sceneMode = 10;
			ADD_MOVER(BF_GLOBALS._player, BF_GLOBALS._player._position.x - 150,
				BF_GLOBALS._player._position.y + 120);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 325 - Police Station Conference Room
 *
 *--------------------------------------------------------------------------*/

bool Scene325::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_EXIT) {
		BF_GLOBALS._events.setCursor(CURSOR_WALK);
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._sceneManager.changeScene(315);
		return true;
	} else {
		return false;
	}
}

/*--------------------------------------------------------------------------*/

void Scene325::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(325);
	BF_GLOBALS._interfaceY = 200;
	BF_GLOBALS.clearFlag(fCanDrawGun);

	if (BF_GLOBALS._dayNumber == 0)
		BF_GLOBALS._dayNumber = 1;

	// Add the speakers
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_PSutterSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.hide();

	if (BF_GLOBALS._dayNumber == 1) {
		_object1.postInit();
		_object1.setVisage(325);
		_object1.setStrip(8);
		_object1.setPosition(Common::Point(128, 44));
	} else {
		_object1.postInit();
		_object1.setVisage(325);
		_object1.setStrip(8);
		_object1.setFrame(2);
		_object1.setPosition(Common::Point(132, 28));
		
		_object2.postInit();
		_object2.setVisage(325);
		_object2.setStrip(8);
		_object2.setFrame(3);
		_object2.setPosition(Common::Point(270, 24));
	}

	_object3.postInit();
	_object3.setVisage(335);
	_object3.setStrip(4);
	_object3.setPosition(Common::Point(202, 122));

	_object4.postInit();
	_object4.setVisage(335);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(283, 102));

	_object5.postInit();
	_object5.setVisage(335);
	_object5.setStrip(1);
	_object5.setPosition(Common::Point(135, 167));

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 560, -1, -1, -1, 1, NULL);
	BF_GLOBALS._player.disableControl();

	_sceneMode = (BF_GLOBALS._dayNumber == 1) ? 3250 : 3251;
	setAction(&_sequenceManager, this, _sceneMode, &_object3, &_object4, &_object5, NULL);
}

void Scene325::signal() {
	BF_GLOBALS._player._uiEnabled = 0;
	BF_GLOBALS._player._canWalk = true;
	BF_GLOBALS._player._enabled = true;
	BF_GLOBALS._events.setCursor(CURSOR_EXIT);
}

/*--------------------------------------------------------------------------
 * Scene 330 - Approaching Marina
 *
 *--------------------------------------------------------------------------*/

void Scene330::Timer1::signal() {
	PaletteRotation *rotation = BF_GLOBALS._scenePalette.addRotation(240, 254, 1);
	rotation->setDelay(25);
	remove();
}

/*--------------------------------------------------------------------------*/

Scene330::Scene330() {
	_seqNumber = 0;
}

void Scene330::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_seqNumber);
}

void Scene330::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	BF_GLOBALS._sound1.changeSound(35);
	_sound1.fadeSound(35);

	loadScene(850);
	_timer.set(2, NULL);

	if (BF_GLOBALS._dayNumber >= 4) {
		_object2.postInit();
		_object2.setVisage(851);
		_object2.setPosition(Common::Point(120, 112));
	}

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 850 : 852);
	BF_GLOBALS._player.setStrip(2);
	BF_GLOBALS._player.setFrame(1);
	BF_GLOBALS._player.hide();
	if (BF_GLOBALS.getFlag(fWithLyle))
		BF_GLOBALS._player.setStrip(5);

	if ((BF_GLOBALS._dayNumber == 1) && BF_GLOBALS.getFlag(fBackupArrived340)) {
		_object1.postInit();
		_object1.setVisage(850);
		_object1.setStrip(6);
		_object1.setFrame(1);
		_object1.setPosition(Common::Point(47, 169));
		_object1.animate(ANIM_MODE_2);
	}

	if (BF_GLOBALS._sceneManager._previousScene == 50) {
		// Coming from map
		if ((BF_GLOBALS._driveFromScene == 340) || (BF_GLOBALS._driveFromScene == 342) ||
				(BF_GLOBALS._driveFromScene == 330)) {
			if (BF_GLOBALS.getFlag(fWithLyle)) {
				_seqNumber = 3304;
			} else {
				_seqNumber = 3302;
				_sound2.play(123);
				BF_GLOBALS.setFlag(onBike);
			}
		} else if (BF_GLOBALS.getFlag(fWithLyle)) {
			_seqNumber = 3303;
		} else {
			_sound2.play(123);
			_seqNumber = 3301;

			if ((BF_GLOBALS._dayNumber == 1) && (BF_GLOBALS._bookmark >= bStartOfGame) &&
					(BF_GLOBALS._bookmark < bCalledToDomesticViolence)) {
				BF_GLOBALS._player.animate(ANIM_MODE_2);
			}
		}
	} else if (BF_GLOBALS.getFlag(fWithLyle)) {
		_seqNumber = 3303;
	} else {
		_seqNumber = 3301;
		_sound2.play(123);

		if ((BF_GLOBALS._dayNumber == 1) && (BF_GLOBALS._bookmark >= bStartOfGame) &&
				(BF_GLOBALS._bookmark < bCalledToDomesticViolence)) {
			BF_GLOBALS._player.animate(ANIM_MODE_2);
		}
	}

	BF_GLOBALS._player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager, this, _seqNumber, &BF_GLOBALS._player, NULL);
}

void Scene330::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

void Scene330::signal() {
	if ((BF_GLOBALS._driveFromScene == 330) || (BF_GLOBALS._driveFromScene == 340) ||
			(BF_GLOBALS._driveFromScene == 342)) {
		// Leaving marina
		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark >= bCalledToDomesticViolence))
			// Leave scene normally
			BF_GLOBALS._sceneManager.changeScene(BF_GLOBALS._driveToScene);
		else {
			// Player leaves with domestic violence unresolved
			BF_GLOBALS._player.hide();
			BF_GLOBALS._deathReason = 4;
			BF_GLOBALS._sceneManager.changeScene(666);
		}
	} else {
		// Arriving at marina
		BF_GLOBALS.clearFlag(onBike);

		if ((BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
				(BF_GLOBALS._bookmark >= bCalledToDomesticViolence))
			BF_GLOBALS._sceneManager.changeScene(342);
		else
			BF_GLOBALS._sceneManager.changeScene(340);
	}
}

/*--------------------------------------------------------------------------
 * Scene 340 - Marina, Domestic Disturbance
 *
 *--------------------------------------------------------------------------*/

bool Scene340::Child::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 5);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 14);
		return true;
	case CURSOR_TALK:
		if (!BF_GLOBALS.getFlag(fBackupArrived340) || (BF_GLOBALS._v4CEB4 < 3) || !BF_GLOBALS.getFlag(fGotAllSkip340))
			scene->setAction(&scene->_action3);
		else
			scene->setAction(&scene->_action2);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene340::Woman::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 4);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 13);
		return true;
	case CURSOR_TALK:
		if (!BF_GLOBALS.getFlag(fBackupArrived340) || (BF_GLOBALS._v4CEB4 < 3) || !BF_GLOBALS.getFlag(fGotAllSkip340))
			scene->setAction(&scene->_action1);
		else
			scene->setAction(&scene->_action2);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}

bool Scene340::Lyle::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 6);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 15);
		return true;
	case CURSOR_TALK:
		scene->setAction(&scene->_action5);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
		break;
	}
}


/*--------------------------------------------------------------------------*/

bool Scene340::Item1::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	if (action == INV_COLT45) {
		scene->gunDisplay();
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene340::WestExit::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	if (BF_GLOBALS.getFlag(fBackupArrived340)) {
		scene->setAction(&scene->_action6);
	} else {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4;
		setAction(&scene->_sequenceManager1, scene, 1348, &BF_GLOBALS._player, NULL);
	}
	return true;
}

bool Scene340::SouthWestExit::startAction(CursorType action, Event &event) {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 3;
	setAction(&scene->_sequenceManager1, scene, 1340, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene340::NorthExit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 254, 106);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene340::Action1::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		if (!BF_GLOBALS._v4CEB4) {
			setAction(&scene->_action8, this);
		} else if (!_action) {
			BF_GLOBALS._player.setAction(&scene->_sequenceManager1, this, 
					MIN(BF_GLOBALS._v4CEB4, 3) + 2340, &scene->_woman, &scene->_child,
					&scene->_object4, NULL);
		}
		break;
	case 3:
		if ((BF_GLOBALS._v4CEB4 != 1) || BF_GLOBALS.getFlag(fCalledBackup)) {
			setDelay(3);
		} else {
			scene->_sound1.play(8);
			scene->_stripManager.start(3413, this);
		}
		break;
	case 4:
		if (BF_GLOBALS._v4CEB4 == 1)
			++BF_GLOBALS._v4CEB4;

		if (BF_GLOBALS.getFlag(fBackupArrived340)) {
			scene->_field2654 = 1;
			scene->_lyle.setPosition(Common::Point(46, 154));
			BF_GLOBALS._walkRegions.proc1(19);
		} else if (BF_GLOBALS.getFlag(fCalledBackup)) {
			scene->_timer1.set(40, &scene->_lyle, &scene->_action4);
		}
		break;
	}
}

void Scene340::Action2::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager3, this, 1341, &scene->_woman, &scene->_child, NULL);
		break;
	case 3:
		scene->_woman.remove();
		scene->_child.remove();
		BF_GLOBALS.setFlag(fToldToLeave340);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene340::Action3::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		ADD_PLAYER_MOVER(64, 155);
		break;
	}
	case 1:
		BF_GLOBALS._player.changeAngle(45);
		setDelay(3);
		break;
	case 2:
		scene->_stripManager.start(scene->_field2652 + 3404, this);
		break;
	case 3:
		if (++scene->_field2652 > 2) {
			if (!BF_GLOBALS.getFlag(fGotAllSkip340))
				BF_GLOBALS.setFlag(fGotAllSkip340);
			scene->_field2652 = 0;
		}

		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene340::Action4::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (!_action) {
			BF_GLOBALS._player.disableControl();
			setDelay(3);
		} else {
			scene->_timer1.set(30, &scene->_lyle, &scene->_action4);
			remove();
		}
		break;
	case 1:
		BF_GLOBALS.setFlag(fBackupArrived340);
		scene->_field2654 = 1;
		setDelay(3);
		break;
	case 2:
		BF_GLOBALS._player.setAction(&scene->_sequenceManager3, this, 1347, &scene->_lyle, NULL);
		break;
	case 3:
		BF_GLOBALS._walkRegions.proc1(19);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene340::Action5::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		if (scene->_field2654) {
			ADD_PLAYER_MOVER(64, 155);
		} else {
			BF_GLOBALS._player.changeAngle(45);
			setDelay(3);
		}
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_lyle._position);
		setDelay(3);
		break;
	case 2:
		setDelay(15);
		break;
	case 3:
		if (BF_GLOBALS.getFlag(fBriefedBackup))
			scene->_stripManager.start(3416, this);
		else {
			BF_GLOBALS.setFlag(fBriefedBackup);
			scene->_stripManager.start(3407, this);
		}
		break;
	case 4:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

void Scene340::Action6::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(10, 110);
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_lyle._position);
		scene->_lyle.updateAngle(BF_GLOBALS._player._position);
		scene->_stripManager.start(3415, this);
		break;
	case 2: {
		ADD_MOVER(BF_GLOBALS._player, -8, 110);
		break;
	}
	case 3:
		scene->_sceneMode = 4;
		scene->signal();
		remove();
		break;
	}
}

void Scene340::Action7::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER(254, 121);
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_lyle._position);
		scene->_stripManager.start(BF_GLOBALS.getFlag(fBriefedBackup) ? 3414 : 3417, this);
		break;
	case 2:
		BF_GLOBALS.setFlag(fBackupIn350);
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	}
}

void Scene340::Action8::signal() {
	Scene340 *scene = (Scene340 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_stripManager.start(3400, this);
		break;
	case 1:
		setDelay(2);
		break;
	case 2:
		scene->_object4.setPriority(250);
		scene->_object4.setPosition(Common::Point(138, 130));
		scene->_object4.setVisage(347);
		scene->_object4.setStrip(6);
		scene->_object4.setFrame(1);
		scene->_object4._numFrames = 2;
		scene->_object4.animate(ANIM_MODE_5, NULL);
		scene->_object4.show();
		break;
	case 3:
		scene->_object4.hide();

		scene->_woman.setPriority(123);
		scene->_woman.setPosition(Common::Point(88, 143));
		scene->_woman.setVisage(344);
		scene->_woman.setStrip(2);
		scene->_woman.setFrame(1);
		scene->_woman.changeZoom(100);
		scene->_woman._numFrames = 10;
		scene->_woman._moveRate = 10;
		scene->_woman._moveDiff = Common::Point(3, 2);
		scene->_woman.show();

		scene->_child.setPriority(120);
		scene->_child.setPosition(Common::Point(81, 143));
		scene->_child.setVisage(347);
		scene->_child.setStrip(3);
		scene->_child.setFrame(1);
		scene->_child.changeZoom(100);
		scene->_child._numFrames = 10;
		scene->_child._moveRate = 10;
		scene->_child.show();

		setDelay(6);
		break;
	case 4:
		remove();
		break;
	}
}

void Scene340::Action8::process(Event &event) {
	if ((_actionIndex != 3) || (event.eventType == EVENT_NONE))
		Action::process(event);
	else if (event.eventType == EVENT_BUTTON_DOWN) {
		event.handled = true;
		setDelay(2);
	}
}

/*--------------------------------------------------------------------------*/

void Scene340::Timer2::signal() {
	PaletteRotation *item;
	
	item = BF_GLOBALS._scenePalette.addRotation(235, 239, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(247, 249, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(240, 246, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(252, 254, 1);
	item->setDelay(30);

	remove();
}

/*--------------------------------------------------------------------------*/

Scene340::Scene340(): PalettedScene() {
	_seqNumber1 = _field2652 = _field2654 = 0;
}

void Scene340::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_seqNumber1);
	s.syncAsSint16LE(_field2652);
	s.syncAsSint16LE(_field2654);
}

void Scene340::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(340);
	setZoomPercents(126, 70, 162, 100);
	
	BF_GLOBALS._walkRegions.proc1(13);
	BF_GLOBALS._walkRegions.proc1(15);
	_timer2.set(2, NULL);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	
	_field2652 = 0;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	_westExit.setDetails(Rect(0, 76, 19, 115), 340, -1, -1, -1, 1, NULL);
	_swExit.setDetails(15, 340, -1, -1, -1, 1);
	_northExit.setDetails(16, 340, -1, -1, -1, 1);

	BF_GLOBALS._player._regionBitList = 0x10000;
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1341 : 129);
	BF_GLOBALS._player._moveDiff = Common::Point(5, 2);

	if (BF_GLOBALS._dayNumber < 5) {
		_object5.postInit();
		_object5.setVisage(340);
		_object5.setStrip(4);
		_object5.setPosition(Common::Point(259, 61));
	}

	if (!BF_GLOBALS.getFlag(fToldToLeave340)) {
		_woman.postInit();
		_woman.setVisage(344);
		_woman.setStrip(3);
		_woman.setFrame(1);
		_woman.fixPriority(120);
		_woman.setPosition(Common::Point(88, 143));

		_object4.postInit();
		_object4.hide();

		_child.postInit();
		_child.setVisage(347);
		_child.setStrip(3);
		_child.setFrame(1);
		_child.fixPriority(120);
		_child.setPosition(Common::Point(81, 143));

		_woman.setAction(&_sequenceManager2, NULL, 348, &_woman, &_child, &_object4, NULL);
		BF_GLOBALS._sceneItems.addItems(&_child, &_woman, NULL);

		_stripManager.addSpeaker(&_jordanSpeaker);
		_stripManager.addSpeaker(&_skipBSpeaker);

		BF_GLOBALS.set2Flags(f1097Marina);
	}

	if (BF_GLOBALS.getFlag(fCalledBackup)) {
		_lyle.postInit();
		_lyle.setVisage(326);
		_lyle.setObjectWrapper(new SceneObjectWrapper());
		_lyle.animate(ANIM_MODE_1, NULL);
		_lyle.setPosition(Common::Point(-30, 219));
		BF_GLOBALS._sceneItems.push_back(&_lyle);

		_stripManager.addSpeaker(&_harrisonSpeaker);
		if (BF_GLOBALS.getFlag(fBackupIn350)) {
			_field2654 = 0;
			_lyle.setVisage(1355);
			_lyle.setPosition(Common::Point(289, 112));
			_lyle.changeAngle(225);
			_lyle.setFrame(1);
			_lyle.fixPriority(75);

			BF_GLOBALS._walkRegions.proc1(23);
		} else if (BF_GLOBALS.getFlag(fBackupArrived340)) {
			_field2654 = 1;
			_lyle.setPosition(Common::Point(46, 154));
			BF_GLOBALS._walkRegions.proc1(19);
		} else if (BF_GLOBALS.getFlag(fCalledBackup) && (BF_GLOBALS._v4CEB4 > 0)) {
			_timer1.set(900, &_lyle, &_action4);
		} 
	}

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 350:
		_seqNumber1 = 1342;
		break;
	case 830:
		BF_GLOBALS._player._regionBitList |= 0x800;
		_seqNumber1 = 1343;
		break;
	case 60:
		_seqNumber1 = 342;
		break;
	default:
		_sound1.fadeSound(35);
		BF_GLOBALS._sound1.fadeSound((BF_GLOBALS._bookmark < bStartOfGame) || 
				(BF_GLOBALS._bookmark > bCalledToDomesticViolence) ? 10 : 19);
		_seqNumber1 = 342;
		break;
	}

	_item3.setDetails(7, 340, 3, 9, 12, 1);
	_item2.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, 39), 340, 2, 8, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, BF_INTERFACE_Y), 340, 1, 7, 10, 1, NULL);

	BF_GLOBALS._player.disableControl();
	_sceneMode = 0;
	setAction(&_sequenceManager1, this, _seqNumber1, &BF_GLOBALS._player, NULL);
}

void Scene340::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	PalettedScene::remove();
}

void Scene340::signal() {
	switch (_sceneMode) {
	case 1:
	case 2:
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	case 3:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 4:
		BF_GLOBALS._sceneManager.changeScene(830);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene340::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		SceneItem::display2(350, 26);
		
		signal();
		event.handled = true;
	}

	if (!event.handled) {
		SceneExt::process(event);

		if (BF_GLOBALS._player._enabled && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_westExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_swExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_northExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}
	}
}

void Scene340::dispatch() {
	SceneExt::dispatch();
	int idx = BF_GLOBALS._player.getRegionIndex();

	if (idx == 20) {
		BF_GLOBALS._player.updateZoom();
		BF_GLOBALS._player.fixPriority(75);
	}
	if (idx == 26) {
		BF_GLOBALS._player.updateZoom();
	}
	if (idx == 31) {
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.fixPriority(-1);
	}
	
	if (BF_GLOBALS._player._regionIndex == 16) {
		BF_GLOBALS._player._regionBitList &= ~0x10000;

		if (!BF_GLOBALS.getFlag(fBackupArrived340)) {
			_sceneMode = 1;
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 254, 110);
		} else if (!BF_GLOBALS.getFlag(fBackupIn350)) {
			setAction(&_action7);
		} else {
			_sceneMode = 1;
			ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 254, 110);
		}
	}
}

/*--------------------------------------------------------------------------
 * Scene 342 - Marina, Normal
 *
 *--------------------------------------------------------------------------*/

bool Scene342::Lyle::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(340, 16);
		return true;
	case CURSOR_USE:
		SceneItem::display2(340, 15);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(340, 17);
		return true;
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene342::Item1::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case INV_COLT45:
		scene->gunDisplay();
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene342::WestExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 4;
	setAction(&scene->_sequenceManager1, scene, 1348, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene342::SouthWestExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 3;
	setAction(&scene->_sequenceManager1, scene, 1340, &BF_GLOBALS._player, NULL);
	return true;
}

bool Scene342::NorthExit::startAction(CursorType action, Event &event) {
	Scene342 *scene = (Scene342 *)BF_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 1;
	ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 254, 106);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene342::Timer1::signal() {
	PaletteRotation *item;
	
	item = BF_GLOBALS._scenePalette.addRotation(235, 239, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(247, 249, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(240, 246, 1);
	item->setDelay(30);
	item = BF_GLOBALS._scenePalette.addRotation(252, 254, 1);
	item->setDelay(30);

	remove();
}

/*--------------------------------------------------------------------------*/

Scene342::Scene342(): PalettedScene() {
	_field1A1A = 0;
}

void Scene342::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);

	s.syncAsSint16LE(_field1A1A);
}

void Scene342::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(340);
	setZoomPercents(126, 70, 162, 100);

	BF_GLOBALS._walkRegions.proc1(13);
	BF_GLOBALS._walkRegions.proc1(15);
	
	_field1A1A = 0;
	_timer1.set(2, NULL);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	_westExit.setDetails(Rect(0, 76, 19, 115), 340, -1, -1, -1, 1, NULL);
	_swExit.setDetails(15, 340, -1, -1, -1, 1);
	_northExit.setDetails(16, 340, -1, -1, -1, 1);

	if (BF_GLOBALS._dayNumber < 5) {
		_object3.postInit();
		_object3.setVisage(340);
		_object3.setStrip(4);
		_object3.setPosition(Common::Point(259, 61));
	}

	if (BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.setVisage(1341);
		BF_GLOBALS._player._moveDiff = Common::Point(5, 2);
	} else {
		BF_GLOBALS._player.setVisage(129);
		BF_GLOBALS._player._moveDiff = Common::Point(5, 2);

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle.setVisage(469);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.hide();
		}
	}

	switch (BF_GLOBALS._randomSource.getRandomNumber(2)) {
	case 0:
		_object2.setPosition(Common::Point(46, 59));
		ADD_MOVER_NULL(_object2, 300, -10);
		break;
	case 1:
		_object2.setPosition(Common::Point(311, 57));
		ADD_MOVER_NULL(_object2, 140, -10);
		break;
	case 2:
		_object2.setPosition(Common::Point(-5, 53));
		ADD_MOVER_NULL(_object2, 170, -10);
		break;
	default:
		break;
	}

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 60:
	case 330:
		if (BF_GLOBALS._sceneManager._previousScene != 60) {
			_sound1.fadeSound(35);
			BF_GLOBALS._sound1.fadeSound((BF_GLOBALS._bookmark < bStartOfGame) || 
				(BF_GLOBALS._bookmark > bCalledToDomesticViolence) ? 10 : 19);
		}

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._walkRegions.proc1(19);
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;

			setAction(&_sequenceManager1, this, 346, &_lyle, NULL);
			BF_GLOBALS._player.setAction(&_sequenceManager2, NULL, 342, &BF_GLOBALS._player, NULL);
		} else {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager1, this, 342, &BF_GLOBALS._player, NULL);
		}
		break;
	case 350:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 1342, &BF_GLOBALS._player, NULL);
		break;
	default:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 1343, &BF_GLOBALS._player, NULL);

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.setPosition(Common::Point(-21, 118));
			_lyle.setStrip(5);
			_lyle.setFrame(1);
			_lyle.setPriority(75);
			_lyle.setZoom(75);
			_lyle.setAction(&_sequenceManager2, NULL, 347, &_lyle, NULL);
			BF_GLOBALS._sceneItems.push_back(&_lyle);
		}
		break;		
	}

	_item3.setDetails(7, 340, 3, 9, 12, 1);
	_item2.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, 39), 340, 2, 8, 11, 1, NULL);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH - 1, BF_INTERFACE_Y), 340, 1, 7, 10, 1, NULL);
}

void Scene342::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	PalettedScene::remove();
}

void Scene342::signal() {
	switch (_sceneMode) {
	case 1:
		ADD_PLAYER_MOVER(254, 106);
		BF_GLOBALS._sceneManager.changeScene(350);
		break;
	case 3:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 4:
		BF_GLOBALS._sceneManager.changeScene(830);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene342::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos)) {
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		SceneItem::display2(350, 26);
		
		signal();
		event.handled = true;
	}

	if (!event.handled) {
		SceneExt::process(event);

		if (BF_GLOBALS._player._enabled && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
			// Check if the cursor is on an exit
			if (_westExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_W);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_swExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
				BF_GLOBALS._events.setCursor(surface);
			} else if (_northExit.contains(event.mousePos)) {
				GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
				BF_GLOBALS._events.setCursor(surface);
			} else {
				// In case an exit cursor was being shown, restore the previously selected cursor
				CursorType cursorId = BF_GLOBALS._events.getCursor();
				BF_GLOBALS._events.setCursor(cursorId);
			}
		}
	}
}

void Scene342::dispatch() {
	SceneExt::dispatch();
	int idx = BF_GLOBALS._player.getRegionIndex();

	if (idx == 20) {
		BF_GLOBALS._player.updateZoom();
		BF_GLOBALS._player.fixPriority(75);
	}
	if (idx == 26) {
		BF_GLOBALS._player.updateZoom();
	}
	if (idx == 31) {
		BF_GLOBALS._player.changeZoom(-1);
		BF_GLOBALS._player.fixPriority(-1);
	}
	
	if (idx == 16) {
		BF_GLOBALS._player.enableControl();
		ADD_PLAYER_MOVER(254, 110);
	}
}


} // End of namespace BlueForce

} // End of namespace TsAGE
