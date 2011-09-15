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

#include "tsage/blue_force/blueforce_scenes8.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 800 - Jamison & Ryan
 *
 *--------------------------------------------------------------------------*/

bool Scene800::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_LOOK) {
		SceneItem::display2(800, 11);
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene800::Item2::startAction(CursorType action, Event &event) {
	if (action == CURSOR_LOOK) {
		SceneItem::display2(800, 1);
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene800::Doorway::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(800, 4);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS._dayNumber == 5)
			SceneItem::display2(800, 14);
		else if (BF_GLOBALS._dayNumber < 2)
			SceneItem::display2(800, BF_GLOBALS.getFlag(onDuty) ? 6 : 15);
		else if (((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) && (BF_GLOBALS._dayNumber == 3)) ||
				(BF_GLOBALS._bookmark == bDoneWithIsland))
			SceneItem::display(800, 5);
		else {
			if (BF_GLOBALS.getFlag(fWithLyle)) {
				ADD_PLAYER_MOVER_NULL(scene->_lyle, 277, 145);
			}

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 8000;
			setAction(&scene->_sequenceManager, scene, 8000, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene800::Car1::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(800, 2);
		return true;
	case CURSOR_USE:
		SceneItem::display2(800, 3);
		return true;
	case INV_TICKET_BOOK:
		if (BF_GLOBALS.getFlag(ticketVW))
			SceneItem::display2(800, 12);
		else if (!BF_GLOBALS.getFlag(onDuty))
			SceneItem::display2(800, 13);
		else {
			BF_GLOBALS.setFlag(ticketVW);
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._uiElements.addScore(30);
			scene->_sceneMode = 8005;
			scene->setAction(&scene->_action1);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene800::Motorcycle::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(800, 0);
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(800, 8);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 8002;
			setAction(&scene->_sequenceManager, scene, 8002, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene800::Lyle::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(800, 7);
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 8001;
		scene->_stripManager.start(8003, scene);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene800::Car2::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(800, 9);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(800, 10);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 8003;
			setAction(&scene->_sequenceManager, scene, 8003, &BF_GLOBALS._player, &scene->_lyle, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene800::Action1::signal() {
	Scene800 *scene = (Scene800 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		BF_GLOBALS._player.disableControl();
		ADD_PLAYER_MOVER_THIS(BF_GLOBALS._player, 95, 153);
		break;
	}
	case 1: {
		ADD_MOVER(BF_GLOBALS._player, 70, 157);
		break;
	}
	case 2:
		BF_GLOBALS._player.setVisage(800);
		BF_GLOBALS._player.setStrip(1);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.fixPriority(200);
		BF_GLOBALS._player.animate(ANIM_MODE_4, 16, 1, this);
		break;
	case 3:
		scene->_object6.postInit();
		scene->_object6.setVisage(800);
		scene->_object6.setStrip(2);
		scene->_object6.setFrame(2);
		scene->_object6.setPosition(Common::Point(58, 135));
		scene->_object6.fixPriority(170);
		BF_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 4:
		BF_GLOBALS._player.setVisage(352);
		BF_GLOBALS._player.setStrip(2);
		BF_GLOBALS._player.setFrame(1);
		BF_GLOBALS._player.fixPriority(-1);
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		ADD_MOVER(BF_GLOBALS._player, 89, 154);
		break;
	case 5:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}

}

/*--------------------------------------------------------------------------*/

void Scene800::postInit(SceneObjectList *OwnerList) {
	loadScene(800);
	SceneExt::postInit();
	setZoomPercents(130, 50, 155, 100);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(1358);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(319, 144));
	BF_GLOBALS._player._moveDiff = Common::Point(3, 2);
	BF_GLOBALS._player.changeZoom(-1);

	_motorcycle.postInit();
	_motorcycle.setVisage(301);
	_motorcycle.setPosition(Common::Point(319, 162));

	if ((BF_GLOBALS._dayNumber >= 2) && (BF_GLOBALS._dayNumber < 5)) {
		_car2.postInit();
		_car2.setVisage(444);
		_car2.setStrip(3);
		_car2.setFrame(2);
		_car2.setPosition(Common::Point(237, 161));
		_car2.changeZoom(110);
		_car2.fixPriority(158);
		BF_GLOBALS._sceneItems.push_back(&_car2);

		BF_GLOBALS._walkRegions.proc1(8);
	}

	if ((BF_GLOBALS._dayNumber == 4) && (BF_GLOBALS._bookmark < bEndDayThree)) {
		_car2.remove();
		BF_GLOBALS._walkRegions.proc2(8);
	}

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		_lyle.postInit();
		_lyle.setVisage(1359);
		_lyle.setObjectWrapper(new SceneObjectWrapper());
		_lyle.animate(ANIM_MODE_1, NULL);
		_lyle.setPosition(Common::Point(210, 160));
		_lyle.changeZoom(-1);
		BF_GLOBALS._sceneItems.push_back(&_lyle);

		_motorcycle.setVisage(580);
		_motorcycle.setStrip(2);
		_motorcycle.setFrame(2);
	} else if (BF_GLOBALS.getFlag(onDuty)) {
		BF_GLOBALS._player.setVisage(352);
	} else {
		_motorcycle.setVisage(580);
		_motorcycle.setStrip(2);
		_motorcycle.setFrame(2);
	}
	BF_GLOBALS._sceneItems.push_back(&_motorcycle);

	_doorway.postInit();
	_doorway.setVisage(800);
	_doorway.setStrip(3);
	_doorway.setPosition(Common::Point(287, 140));
	BF_GLOBALS._sceneItems.push_back(&_doorway);

	_car1.postInit();
	_car1.setVisage(800);
	_car1.setStrip(2);
	_car1.setFrame(1);
	_car1.setPosition(Common::Point(58, 163));
	BF_GLOBALS._sceneItems.push_back(&_car1);

	if (BF_GLOBALS._sceneManager._previousScene == 810) {
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._player.setPosition(Common::Point(271, 148));
			BF_GLOBALS._player.enableControl();
		} else {
			BF_GLOBALS._player.changeZoom(75);
			BF_GLOBALS._player.setPosition(Common::Point(275, 135));
			BF_GLOBALS._player.disableControl();
			_sceneMode = 8001;
			setAction(&_sequenceManager, this, 8001, &BF_GLOBALS._player, &_doorway, NULL);
		}
	} else if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) && (BF_GLOBALS._bookmark == bFlashBackThree)) {
		BF_GLOBALS._bookmark = bFlashBackThree;
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setPosition(Common::Point(231, 150));
		BF_GLOBALS._player.setStrip(3);

		_lyle.setPosition(Common::Point(244, 162));
		_lyle.setStrip(4);
		_sceneMode = 8004;
		setAction(&_sequenceManager, this, 8001, &_lyle, &_doorway, NULL);
	} else {
		BF_GLOBALS._player.updateAngle(_motorcycle._position);
		BF_GLOBALS._player.enableControl();
	}

	_item2.setBounds(Rect(47, 26, 319, 143));
	BF_GLOBALS._sceneItems.push_back(&_item2);
	_item1.setBounds(Rect(0, 0, 320, 165));
	BF_GLOBALS._sceneItems.push_back(&_item1);

	BF_GLOBALS._sound1.fadeSound(33);
}

void Scene800::signal() {
	switch (_sceneMode) {
	case 8000:
		BF_GLOBALS._sceneManager.changeScene(810);
		break;
	case 8001:
	case 8005:
		BF_GLOBALS._player.enableControl();
		break;
	case 8002:
	case 8003:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 8004:
		BF_GLOBALS.clearFlag(fWithLyle);
		_lyle.remove();
	}
}

void Scene800::dispatch() {
	SceneExt::dispatch();
	if (BF_GLOBALS._player.getRegionIndex() == 7) {
		BF_GLOBALS._player.updateZoom();
	} else {
		BF_GLOBALS._player.changeZoom(-1);
	}
}

/*--------------------------------------------------------------------------
 * Scene 830 - Outside Boat Rentals
 *
 *--------------------------------------------------------------------------*/

bool Scene830::Lyle::startAction(CursorType action, Event &event) {
	Scene830 *scene = (Scene830 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (scene->_field18AA) {
			scene->_stripManager.start(8304, &BF_GLOBALS._stripProxy);
		} else {
			scene->_stripManager.start(8303, &BF_GLOBALS._stripProxy);
			++scene->_field18AA;
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene830::Door::startAction(CursorType action, Event &event) {
	Scene830 *scene = (Scene830 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (BF_GLOBALS._dayNumber == 2) {
			scene->_sceneMode = 14;
			Common::Point pt(186, 102);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &pt, NULL);
			return true;
		}
		break;
	case CURSOR_USE:
		if (BF_GLOBALS._dayNumber == 2) {
			scene->_sceneMode = 14;
			Common::Point pt(186, 102);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &pt, NULL);
		} else if (scene->_field18A4) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 832;
			
			if (BF_GLOBALS.getFlag(onDuty)) {
				scene->setAction(&scene->_sequenceManager, scene, 8302, &BF_GLOBALS._player, &scene->_door, NULL);
			} else if (BF_GLOBALS.getFlag(fWithLyle)) {
				scene->_sceneMode = 11;
				scene->setAction(&scene->_sequenceManager, scene, 8308, &BF_GLOBALS._player, &scene->_door, &scene->_lyle, NULL);
			} else {
				scene->setAction(&scene->_sequenceManager, scene, 832, &BF_GLOBALS._player, &scene->_door, NULL);
			}
		} else {
			scene->_stripManager.start(8305, &BF_GLOBALS._stripProxy);
			ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 192, 145);
		}
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

bool Scene830::Object4::startAction(CursorType action, Event &event) {
	Scene830 *scene = (Scene830 *)BF_GLOBALS._sceneManager._scene;

	if (action == INV_RENTAL_KEYS) {
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			scene->_sceneMode = 8300;
			scene->setAction(&scene->_sequenceManager, scene, 8300, &BF_GLOBALS._player, &scene->_lyle, NULL);
		} else {
			scene->_sceneMode = 834;
			scene->setAction(&scene->_sequenceManager, scene, 8300, &BF_GLOBALS._player, &scene->_object4, NULL);
		}
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene830::Object5::startAction(CursorType action, Event &event) {
	Scene830 *scene = (Scene830 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 8307;
		scene->setAction(&scene->_sequenceManager, scene, 8307, &BF_GLOBALS._player, &scene->_object5, NULL);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene830::SouthEastExit::startAction(CursorType action, Event &event) {
	ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x + 80, BF_GLOBALS._player._position.y + 100);
	return true;
}

bool Scene830::NoteBoard::startAction(CursorType action, Event &event) {
	Scene830 *scene = (Scene830 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (BF_GLOBALS._sceneObjects->contains(&scene->_object6)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 8309;
			scene->setAction(&scene->_sequenceManager, scene, 8309, &BF_GLOBALS._player, NULL);
			return true;
		}
		break;
	case CURSOR_USE:
		if (BF_GLOBALS._sceneObjects->contains(&scene->_object6)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 8305;
			scene->setAction(&scene->_sequenceManager, scene, 8305, &BF_GLOBALS._player, &scene->_object6, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene830::Scene830(): PalettedScene() {
	_field18A4 = _field18A6 = _field18A8 = _field18AA = _field18AC = 0;
}

void Scene830::synchronize(Serializer &s) {
	PalettedScene::synchronize(s);
	s.syncAsByte(_field18A4);
	s.syncAsByte(_field18A6);
	s.syncAsByte(_field18A8);
	s.syncAsByte(_field18AA);
	s.syncAsByte(_field18AC);
}

void Scene830::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(830);
	_sound1.fadeSound(35);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._sceneManager._previousScene = 340;
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS.setFlag(fWithLyle);
	}

	_field18A8 = (BF_GLOBALS._dayNumber <= 3) ? 0 : 1;
	_field18AC = 0;
	BF_GLOBALS.clearFlag(fCanDrawGun);

	_seExit.setDetails(Rect(183, 161, 320, 167), 830, -1, -1, -1, 1, NULL);

	PaletteRotation *rot = BF_GLOBALS._scenePalette.addRotation(184, 189, 1);
	rot->setDelay(30);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.disableControl();

	if (_field18A8) {
		_object4.postInit();
		_object4.setVisage(830);
		_object4.setStrip(1);
		_object4.setPosition(Common::Point(271, 146));
		_object4.fixPriority(90);
		_object4.setDetails(830, 0, 1, 2, 1, NULL);
	}
	
	_door.postInit();
	_door.setVisage(830);
	_door.setDetails(830, 3, 4, -1, 1, NULL);
	_door.setStrip((BF_GLOBALS._dayNumber == 2) ? 4 : 3);
	_door.setPosition(Common::Point(182, 97));

	_field18A4 = 1;
	_noteBoard.setDetails(1, 830, 30, 31, 32, 1);

	switch (BF_GLOBALS._dayNumber) {
	case 1:
		if ((BF_GLOBALS._bookmark >= bEndOfWorkDayOne) && (BF_INVENTORY.getObjectScene(INV_CARTER_NOTE) != 1) &&
				(BF_INVENTORY.getObjectScene(INV_CARTER_NOTE) != 3)) {
			_object6.postInit();
			_object6.setVisage(830);
			_object6.setStrip(4);
			_object6.setFrame(2);
			_object6.setPosition(Common::Point(302, 124));
		}
		break;
	case 4:
		if (BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 830) {
			_object5.postInit();
			_object5.setVisage(830);
			_object5.setStrip(2);
			_object5.setFrame(2);
			_object5.setPosition(Common::Point(126, 133));
			_object5.setDetails(830, 7, 8, -1, 1, NULL);
		}
		break;
	case 5:
		if ((BF_GLOBALS._sceneManager._previousScene == 840) && (BF_INVENTORY.getObjectScene(INV_RENTAL_KEYS) == 1) &&
					!BF_GLOBALS.getFlag(fWithLyle)) {
			_field18A4 = 0;

			_lyle.postInit();
			_lyle._flags = OBJFLAG_CHECK_REGION;
			_lyle.setVisage(835);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.setStrip(4);
			_lyle.setPosition(Common::Point(180, 154));
			_lyle._moveDiff = Common::Point(2, 0);
			_lyle.setDetails(830, 28, -1, 29, 1, NULL);

			_field18AC = 1;
		}
		break;
	}

	_item2.setDetails(7, 830, 9, 10, 11, 1);
	_item4.setDetails(2, 830, 12, 13, 14, 1);
	_item5.setDetails(3, 830, 15, 16, 17, 1);
	_item6.setDetails(4, 830, 18, 19, 20, 1);
	_item7.setDetails(5, 830, 21, 22, 23, 1);
	_item8.setDetails(6, 830, 24, 25, 26, 1);

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 840:
		_sceneMode = 833;
		if (BF_GLOBALS.getFlag(onDuty)) {
			setAction(&_sequenceManager, this, 8304, &BF_GLOBALS._player, &_door, NULL);
		} else if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle._moveDiff = Common::Point(2, 0);
			_sceneMode = 837;
			setAction(&_sequenceManager, this, 837, &_lyle, &_door, &BF_GLOBALS._player, NULL);
		} else {
			setAction(&_sequenceManager, this, 833, &BF_GLOBALS._player, &_door, NULL);
		}
		break;
	case 850:
		BF_GLOBALS._player.setVisage(831);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player.setStrip(3);
		BF_GLOBALS._player._moveDiff = Common::Point(2, 0);
		BF_GLOBALS._player.setPosition(Common::Point(220, 122));
		BF_GLOBALS._player.enableControl();

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle.setVisage(835);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.setStrip(3);
			_lyle._moveDiff = Common::Point(2, 0);
			_lyle.setPosition(Common::Point(224, 126));
		}

		if ((BF_GLOBALS._bookmark == bEndDayThree) && (BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 1)) {
			BF_INVENTORY.setObjectScene(INV_FISHING_NET, 0);
			BF_INVENTORY.setObjectScene(INV_HOOK, 0);
		}
		break;
	case 340:
	case 830:
	default:
		_sceneMode = 830;
		setAction(&_sequenceManager, this, BF_GLOBALS.getFlag(onDuty) ? 8303 : 830, &BF_GLOBALS._player, NULL);

		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle.setVisage(835);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.setPosition(Common::Point(SCREEN_WIDTH, SCREEN_HEIGHT));
			_lyle._moveDiff = Common::Point(2, 0);
			ADD_MOVER(_lyle, 280, 160);
		}
		break;
	}

	_field18A6 = 1;
}

void Scene830::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	PalettedScene::remove();
}

void Scene830::signal() {
	static uint32 black = 0;

	switch (_sceneMode) {
	case 11:
		_sceneMode = 832;
		BF_GLOBALS._scenePalette.clearListeners();
		addFader((const byte *)&black, 5, this);
		BF_GLOBALS._v51C44 = 0;
		break;
	case 12:
		_sceneMode = 831;
		BF_GLOBALS._scenePalette.clearListeners();
		addFader((const byte *)&black, 5, this);
		BF_GLOBALS._v51C44 = 0;
		break;
	case 13:
		BF_GLOBALS._sceneManager.changeScene(850);
		break;
	case 14:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 8312;

		_object1.postInit();
		setAction(&_sequenceManager, this, 8312, &_object1, NULL);
		break;
	case 830:
		_field18A6 = 1;
		BF_GLOBALS._player.enableControl();
		break;
	case 831:
	case 8306:
		BF_GLOBALS._sceneManager.changeScene((BF_GLOBALS._bookmark < bStartOfGame) || 
			(BF_GLOBALS._bookmark >= bCalledToDomesticViolence) ? 342 : 340);
		break;
	case 832:
		BF_GLOBALS._sceneManager.changeScene(840);
		break;
	case 833:
		if (_field18AC) {
			BF_GLOBALS.setFlag(fWithLyle);
			_sceneMode = 8311;
			setAction(&_sequenceManager, this, 8311, &BF_GLOBALS._player, &_lyle, NULL);
		} else {
			BF_GLOBALS._player.enableControl();
			BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
			BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			BF_GLOBALS._player._strip = 3;
			BF_GLOBALS._player._frame = 1;
		}
		break;
	case 834:
		BF_GLOBALS._sceneManager.changeScene(850);
		break;
	case 837:
		BF_GLOBALS._player.enableControl();
		break;
	case 8300:
		_sceneMode = 13;
		BF_GLOBALS._scenePalette.clearListeners();
		addFader((const byte *)&black, 5, this);
		break;
	case 8305:
		_object6.remove();
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player._strip = 7;

		BF_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene(INV_CARTER_NOTE, 1);
		break;
	case 8307:
		BF_GLOBALS._player.enableControl();
		_object5.remove();
		BF_GLOBALS._uiElements.addScore(30);
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 1);
		break;
	case 8309:
	case 8311:
		BF_GLOBALS._player.enableControl();
		break;
	case 8312:
		_object1.remove();
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene830::process(Event &event) {
	if (BF_GLOBALS._player._enabled && (event.eventType == EVENT_BUTTON_DOWN) &&
			(BF_GLOBALS._events.getCursor() == CURSOR_WALK) && !_field18A4) {
		_stripManager.start(8306, &BF_GLOBALS._stripProxy);
		event.handled = true;
	}

	if (BF_GLOBALS._player._enabled && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_seExit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SE);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene830::dispatch() {
	SceneExt::dispatch();

	if (!_action && _field18A6 && (BF_GLOBALS._player._position.y > 160)) {
		if (!_field18A4) {
			_stripManager.start(8305, &BF_GLOBALS._player);
			ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 192, 145);
		} else if ((BF_INVENTORY.getObjectScene(INV_RENTAL_KEYS) == 1) && (BF_GLOBALS._bookmark == bEndDayThree)) {
			BF_GLOBALS._player.disableControl();
			SceneItem::display2(830, 27);
			_field18A6 = 0;
			_sceneMode = 830;
			ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x - 10, BF_GLOBALS._player._position.y - 10);
		} else {
			BF_GLOBALS._player.disableControl();
			_field18A6 = 0;
			_sceneMode = 831;
			ADD_PLAYER_MOVER(BF_GLOBALS._player._position.x + 40, BF_GLOBALS._player._position.y + 40);

			if (BF_GLOBALS.getFlag(fWithLyle)) {
				ADD_MOVER_NULL(_lyle, SCREEN_WIDTH, SCREEN_HEIGHT);
				if (_lyle._position.y < 160)
					_sceneMode = 12;
			}
		}
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
