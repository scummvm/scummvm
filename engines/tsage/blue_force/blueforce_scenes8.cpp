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

} // End of namespace BlueForce

} // End of namespace TsAGE
