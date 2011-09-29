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

#include "tsage/blue_force/blueforce_scenes5.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 550 - Outside Bikini Hut
 *
 *--------------------------------------------------------------------------*/

void Scene550::Action1::signal() {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(5);
		break;
	case 1:
		scene->_stripManager.start(scene->_sceneMode, this);
		break;
	case 2:
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene550::Object1::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) || 
				(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
			if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) &&
					(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
				BF_GLOBALS.setFlag(fToldLyleOfSchedule);
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 5501;
				scene->setAction(&scene->_sequenceManager, scene, 5514, &BF_GLOBALS._player, this, NULL);
			} else {
				scene->_sceneMode = 0;
				scene->_stripManager.start(5509, scene);
			}
		} else if (BF_GLOBALS._sceneManager._previousScene == 930) {
			scene->_sceneMode = 5512;
			scene->setAction(&scene->_action1);
		} else {
			scene->_sceneMode = BF_INVENTORY.getObjectScene(BF_LAST_INVENT) == 1 ? 5513 : 5512;
			scene->_stripManager.setAction(&scene->_action1);
		}
		return true;
	case INV_SCHEDULE:
		BF_GLOBALS.setFlag(fToldLyleOfSchedule);
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 5501;
		scene->setAction(&scene->_sequenceManager, scene, 5514, &BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene550::CaravanDoor::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 0);
		return true;
	case CURSOR_USE:
		SceneItem::display2(550, 7);
		return true;
	case BF_LAST_INVENT:
		if ((BF_GLOBALS._dayNumber == 3) || !BF_GLOBALS.getFlag(fWithLyle))
			SceneItem::display2(550, 33);
		else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5500;
			scene->setAction(&scene->_sequenceManager, scene, 5500, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene550::Vechile::startAction(CursorType action, Event &event) {
	Scene550 *scene = (Scene550 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(550, 3);
		return true;
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 5510;
			scene->setAction(&scene->_sequenceManager, scene, BF_GLOBALS.getFlag(fWithLyle) ? 5510 : 5515,
				&BF_GLOBALS._player, this, NULL);
		} else if (BF_GLOBALS.getFlag(fToldLyleOfSchedule)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 5501, &BF_GLOBALS._player, NULL);
		} else if ((BF_INVENTORY.getObjectScene(INV_SCHEDULE) == 1) || 
					(BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1)) {
			if (BF_INVENTORY.getObjectScene(INV_9MM_BULLETS) == 1) {
				scene->_sceneMode = 5501;
				scene->_stripManager.start(5511, scene);
			} else {
				scene->_sceneMode = 0;
				scene->_stripManager.start(5509, scene);
			}
		} else if (BF_GLOBALS._sceneManager._previousScene == 930) {
			scene->_sceneMode = 5512;
			scene->setAction(&scene->_action1);
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 5501, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene550::Scene550(): SceneExt() {
	_fieldF90 = 0;
}

void Scene550::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._sound1.fadeSound(16);

	if ((BF_GLOBALS._bookmark == bInspectionDone) && !BF_GLOBALS.getFlag(fHasDrivenFromDrunk)) {
		_sceneMode = 1;
		signal();
		return;
	} 

	SceneExt::postInit();
	loadScene(550);
	
	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(160, 100));
	BF_GLOBALS._player._moveDiff = Common::Point(2, 1);
	BF_GLOBALS._player.enableControl();

	_caravanDoor.postInit();
	_caravanDoor.setVisage(550);
	_caravanDoor.setPosition(Common::Point(34, 66));
	BF_GLOBALS._sceneItems.push_back(&_caravanDoor);

	_vechile.postInit();
	_vechile.fixPriority(70);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._walkRegions.proc1(10);
		BF_GLOBALS._walkRegions.proc1(11);

		_vechile.setVisage(444);
		_vechile.setStrip(4);
		_vechile.setFrame2(2);
		_vechile.setPosition(Common::Point(110, 85));
		_vechile.fixPriority(76);

		_object1.postInit();
		_object1.setVisage(835);
		_object1.setPosition(Common::Point(139, 83));
		_object1.setDetails(550, 29, 30, 31, 1, NULL);
		_object1.setStrip(8);

		BF_GLOBALS._player.setVisage(303);
		BF_GLOBALS._player.setPosition(Common::Point(89, 76));
		BF_GLOBALS._player.updateAngle(_object1._position);
	} else {
		BF_GLOBALS._walkRegions.proc1(12);

		_vechile.setPosition(Common::Point(205, 77));
		_vechile.changeZoom(80);

		if (BF_GLOBALS.getFlag(onDuty)) {
			_vechile.setVisage(301);
			_vechile.setStrip(1);
			
			BF_GLOBALS._player.setVisage(304);
		} else {
			_vechile.setVisage(580);
			_vechile.setStrip(2);
			_vechile.setFrame(2);

			BF_GLOBALS._player.setVisage(303);
		}		

		BF_GLOBALS._sceneItems.push_back(&_vechile);

		if (BF_GLOBALS._sceneManager._previousScene == 930) {
			_caravanDoor.setFrame(_caravanDoor.getFrameCount());
			BF_GLOBALS._player.disableControl();

			_sceneMode = 0;
			setAction(&_sequenceManager, this, 5512, &BF_GLOBALS._player, &_caravanDoor, NULL);
		} else if (BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager, this, 5502, &BF_GLOBALS._player, &_vechile, NULL);
		} else if (!BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._player.setPosition(Common::Point(185, 70));
		} else if (BF_GLOBALS._bookmark == bFlashBackOne) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 0;
			setAction(&_sequenceManager, this, 5513, &_object1, NULL);
		} else {
			_sceneMode = 0;
		}
	}

	_item2.setDetails(Rect(0, 26, 53, 67), 550, 1, -1, 2, 1, NULL);
	_item3.setDetails(Rect(53, 12, 173, 65), 550, 4, -1, 5, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 170), 550, 6, -1, -1, 1, NULL);
}

void Scene550::signal() {
	switch (_sceneMode) {
	case 0:
		BF_GLOBALS._player.enableControl();
		break;
	case 1:
		BF_GLOBALS._sceneManager.changeScene(551);
		break;
	case 2:
	case 5510:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 3:
		BF_GLOBALS._driveFromScene = 16;
		BF_GLOBALS._driveToScene = 128;
		BF_GLOBALS._mapLocationId = 128;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(800);
		break;
	case 5500:
		BF_GLOBALS._sceneManager.changeScene(930);
		break;
	case 5501:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3;
		setAction(&_sequenceManager, this, 5501, &BF_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
