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

#include "tsage/blue_force/blueforce_scenes9.h"
#include "tsage/globals.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 900 - Outside Warehouse
 *
 *--------------------------------------------------------------------------*/

bool Scene900::Item1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_LOOK) {
		SceneItem::display2(900, 6);
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

bool Scene900::Item4::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	BF_GLOBALS._player.disableControl();
	scene->_sceneMode = 9001;
	Common::Point pt(0, 117);
	PlayerMover *mover = new PlayerMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

/*--------------------------------------------------------------------------*/
bool Scene900::Object1::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS._v4CEC0 == 0) {
			return NamedObject::startAction(action, event);
		} else {
			BF_GLOBALS._player.disableControl();
			if (BF_GLOBALS._v4CEC0 == 2) {
				scene->_sceneMode = 9006;
				BF_GLOBALS._v4CEC0 = 1;
				scene->setAction(&scene->_sequenceManager1, scene, 9006, &BF_GLOBALS._player, this, NULL);
			} else {
				BF_GLOBALS._v4CEC0 = 2;
				if (scene->_object3._flag == false) {
					BF_GLOBALS._player.setAction(&scene->_action4);
				} else {
					scene->_sceneMode = 9005;
					scene->setAction(&scene->_sequenceManager1, scene, 9005, &BF_GLOBALS._player, this, NULL);
					BF_GLOBALS._walkRegions.proc2(24);
				}
			}
			return true;
		}
		break;
	case INV_WAREHOUSE_KEYS:
		if (BF_GLOBALS._v4CEC0 == 2) {
			SceneItem::display2(900, 14);
		} else {
			if (BF_GLOBALS._v4CEC0 == 0) {
				if (!BF_GLOBALS.getFlag(fGotPointsForUnlockGate)) {
					BF_GLOBALS.setFlag(fGotPointsForUnlockGate);
					BF_GLOBALS._uiElements.addScore(30);
				}
				BF_GLOBALS._v4CEC0 = 1;
			} else {
				if (!BF_GLOBALS.getFlag(fGotPointsForLockGate)) {
					if (BF_GLOBALS._bookmark == bEndDayThree) {
						BF_GLOBALS.setFlag(fGotPointsForLockGate);
						BF_GLOBALS._uiElements.addScore(30);
					}
				}
				BF_GLOBALS._v4CEC0 = 0;
			}
			scene->_sceneMode = 9004;
			BF_GLOBALS._player.disableControl();
			scene->setAction(&scene->_sequenceManager1, scene, 9004, &BF_GLOBALS._player, NULL);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Object2::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS._v4CEC0 == 2) {
			if (_flag) {
				SceneItem::display2(900, 1);
			} else {
				BF_GLOBALS._player.disableControl();
				BF_GLOBALS._walkRegions.proc2(26);
				scene->_sceneMode = 9007;
				scene->setAction(&scene->_sequenceManager1, scene, 9007, &BF_GLOBALS._player, &scene->_object2, this, NULL);
			}
			return true;
		} else
			return NamedObject::startAction(action, event);
		break;
	case INV_WAREHOUSE_KEYS:
		if (BF_GLOBALS._v4CEC0 == 2) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9012;
			scene->setAction(&scene->_sequenceManager1, scene, 9012, &BF_GLOBALS._player, &scene->_object2, NULL);
		} else
			SceneItem::display2(900, 5);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Object3::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(900, 8);
		return true;
	case CURSOR_USE:
		SceneItem::display2(900, 9);
		return true;
	case INV_HOOK:
		if (_flag) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9010;
			scene->setAction(&scene->_sequenceManager1, scene, 9010, &BF_GLOBALS._player, &scene->_object5, this, NULL);
		} else
			SceneItem::display2(900, 23);
		return true;
	case INV_FISHING_NET:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setAction(&scene->_action2);
		return true;
	case INV_DOG_WHISTLE:
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._player.setAction(&scene->_action3);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene900::Object6::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		if (BF_GLOBALS._sceneManager._sceneLoadCount == 0) {
			if (!_action) {
				if (scene->_object3._flag) {
					if (BF_GLOBALS._v4CEC0 == 0)
						scene->_stripManager.start(9004, &BF_GLOBALS._stripProxy);
					else {
						if (scene->_object2._flag == 1) {
							if (BF_GLOBALS._v4CEC0 == 2)
								scene->_stripManager.start(9005, &BF_GLOBALS._stripProxy);
							else
								scene->_stripManager.start(9001, &BF_GLOBALS._stripProxy);
						} else
							scene->_stripManager.start(9001, &BF_GLOBALS._stripProxy);
					}
				} else {
					if (scene->_field1974)
						scene->_stripManager.start(9003, &BF_GLOBALS._stripProxy);
					else {
						++scene->_field1974;
						scene->_stripManager.start(9002, &BF_GLOBALS._stripProxy);
					}
				}
			}
		}
		return true;
	} else
		return NamedObject::startAction(action, event);
}

bool Scene900::Object7::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 9016;
		scene->setAction(&scene->_sequenceManager1, scene, 9016, &BF_GLOBALS._player, NULL);
		return true;
	} else
		return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/
void Scene900::Action1::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (scene->_object3._flag == 0) {
			scene->_object3.setStrip(3);
			if ((BF_GLOBALS._randomSource.getRandomNumber(3) == 1) || (BF_GLOBALS._player._position.x > 790) || (scene->_field1976 != 0)) {
				Common::Point pt(864, 130);
				NpcMover *mover = new NpcMover();
				scene->_object3.addMover(mover, &pt, this);
			} else {
				_actionIndex = 4;
				Common::Point pt(775, 107);
				NpcMover *mover = new NpcMover();
				scene->_object3.addMover(mover, &pt, this);
			}
		}
		break;
	case 1:
		scene->_object3.setPosition(Common::Point(864, 117));
		scene->_object3.setStrip(7);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		if (BF_GLOBALS._randomSource.getRandomNumber(3) == 1)
			scene->_sound1.play(92);
		else
			scene->_sound1.play(44);
		if ((BF_GLOBALS._player._position.x > 750) || (scene->_field1976 != 0))
			_actionIndex = 7;
		break;
	case 2:
		scene->_object3.animate(ANIM_MODE_6, this);
		break;
	case 3: {
		scene->_object3.setStrip(3);
		scene->_object3.setPosition(Common::Point(864, 130));
		scene->_object3.fixPriority(122);
		scene->_object3.animate(ANIM_MODE_1, NULL);
		Common::Point pt(775, 107);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
		}
	case 6:
		_actionIndex = 0;
		// No break on purpose
	case 4:
		setDelay(30);
		break;
	case 5: {
		scene->_object3.setStrip(4);
		Common::Point pt(940, 145);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
		}
	case 7:
		_actionIndex = 7;
		setDelay(5);
		if (BF_GLOBALS._player._position.x < 790)
			_actionIndex = 2;
		if (scene->_field1976 != 0)
			_actionIndex = 8;
		break;
	case 8:
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(7);
		scene->_object3.animate(ANIM_MODE_6, NULL);
		break;
	case 9:
		scene->_field1976 = 0;
		scene->_object3._flag = 0;
		_actionIndex = 7;
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(1);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	default:
		break;
	}
}

void Scene900::Action2::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_field1976 = 1;
		if (scene->_object3._action->getActionIndex() == 8)
			_actionIndex = 0;
		setDelay(5);
		break;
	case 1:
		scene->_object5.postInit();
		scene->_object5.setVisage(902);
		scene->_object5.setStrip(2);
		scene->_object5.setPosition(Common::Point(-20, -20));
		scene->_object5._moveDiff.y = 10;
		setAction(&scene->_sequenceManager1, this, 9009, &BF_GLOBALS._player, &scene->_object5, &scene->_object3, NULL);
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 900);
		break;
	case 2:
		BF_GLOBALS._player._strip = 7;
		if (!BF_GLOBALS.getFlag(fGotPointsForTrapDog)) {
			BF_GLOBALS.setFlag(fGotPointsForTrapDog);
			BF_GLOBALS._uiElements.addScore(50);
		}
		SceneItem::display2(900, 10);
		scene->_object3._flag = 1;
		scene->_object3.fixPriority(130);
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene900::Action3::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if ((BF_GLOBALS._player._position.x < 790) && (!BF_GLOBALS._player._mover)) {
			Common::Point pt(821, 136);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &pt, 0);
		}
		if (scene->_object3._action->getActionIndex() != 7) {
			_actionIndex = 0;
		}
		setDelay(5);
		break;
	case 1:
		if (scene->_object3._strip == 3) {
			_actionIndex = 3;
			Common::Point pt(775, 107);
			NpcMover *mover = new NpcMover();
			scene->_object3.addMover(mover, &pt, this);
		} else
			scene->_object3.animate(ANIM_MODE_6, this);
		break;
	case 2: {
		scene->_object3.setStrip(3);
		scene->_object3.setPosition(Common::Point(864, 130));
		scene->_object3.fixPriority(122);
		scene->_object3.animate(ANIM_MODE_1, NULL);
		Common::Point pt(775, 107);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
		}
	case 3:
		scene->_object3.remove();
		scene->_object3._flag = 1;
		SceneItem::display2(900, 24);
		if (!BF_GLOBALS.getFlag(fGotPointsForLockWarehouse)) {
			BF_GLOBALS.setFlag(fGotPointsForLockWarehouse);
			BF_GLOBALS._uiElements.addScore(10);
		}
		BF_GLOBALS._player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene900::Action4::signal() {
	Scene900 *scene = (Scene900 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_field1976 = 1;
		if (scene->_object3._action->getActionIndex() != 8)
			_actionIndex = 0;
		setDelay(5);
		break;
	case 1:
		scene->setAction(&scene->_sequenceManager1, scene, 9005, &BF_GLOBALS._player, &scene->_object1, NULL);
		break;
	case 2:
		scene->setAction(&scene->_sequenceManager1, scene, 9008, &BF_GLOBALS._player, &scene->_object3, NULL);
		break;
	case 3:
		BF_GLOBALS._deathReason = 5;
		BF_GLOBALS._sceneManager.changeScene(666);
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene900::Scene900(): PalettedScene() {
	_field1974 = _field1976 = 0;
}

void Scene900::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit();
	loadScene(900);

	if (BF_GLOBALS._sceneManager._previousScene == 910)
		BF_GLOBALS._sound1.changeSound(91);
	_field1974 = 0;
	_field1976 = 0;
	BF_GLOBALS._uiElements._active = true;
	BF_GLOBALS.clearFlag(fCanDrawGun);
	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 4;
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 1);
		BF_INVENTORY.setObjectScene(INV_HOOK, 1);
	}
	_object3._flag = 0;
	if (BF_GLOBALS._bookmark >= bFinishedWGreen) {
		_object7.postInit();
		_object7.fixPriority(120);
		_object7.setVisage(901);
		_object7.setPosition(Common::Point(159,128));
		_object7.setDetails(900, 15, 16, 17, ANIM_MODE_1, NULL);
	}
	if (BF_GLOBALS._sceneManager._previousScene == 910) {
		_sceneBounds.moveTo(640, 0);
		BF_GLOBALS._v4CEC0 = 2;
		BF_INVENTORY.setObjectScene(INV_FISHING_NET, 900);
		_object3._flag = 1;
	}
	if (BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 900)
		_object3._flag = 1;

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);

	_item4.setDetails(Rect(0, 85, 20, 130), 900, -1, -1, -1, 1, 0);
	BF_GLOBALS._player.postInit();

	_object3.postInit();
	_object3.setVisage(902);
	_object3.setPosition(Common::Point(845, 135));
	_object3.fixPriority(122);
	_object3.setDetails(900, 8, -1, 9, 1, NULL);

	if (_object3._flag == 0) {
		_object3.animate(ANIM_MODE_1, NULL);
		_object3.setAction(&_action1);
	} else {
		_object3.setAction(&_action1);
		_object3.fixPriority(130);
		if (BF_GLOBALS._dayNumber == 4) {
			_object3.setPosition(Common::Point(879, 120));
			_object3.setStrip(2);
		} else {
			_object3.setPosition(Common::Point(864, 117));
			_object3.setStrip(6);
			_object3.setFrame(6);
		}
	}

	_object1.postInit();
	_object1.setVisage(900);
	_object1.setStrip(2);

	if (BF_GLOBALS._v4CEC0 == 2)
		_object1.setPosition(Common::Point(758, 127));
	else {
		BF_GLOBALS._walkRegions.proc1(24);
		_object1.setPosition(Common::Point(804, 132));
	}

	if (BF_GLOBALS._dayNumber == 5)
		BF_GLOBALS._v4CEC8 = 0;

	if ((BF_GLOBALS._v4CEC8 == 0) && (BF_GLOBALS.getFlag(fWithLyle))) {
		_object4.postInit();
		_object4.setVisage(900);
		_object4.setStrip(3);
		_object4.fixPriority(1);
		_object4.setPosition(Common::Point(866, 117));
	}

	_object2.postInit();
	_object2.setVisage(900);
	
	if (BF_GLOBALS._v4CEC8 == 0)
		_object2.setStrip(4);
	else
		_object2.setStrip(1);

	_object2.setPosition(Common::Point(847, 45));
	_object2._flag = 1;

	if ((BF_GLOBALS._sceneManager._previousScene == 880) || (BF_GLOBALS._sceneManager._previousScene != 910)) {
		BF_GLOBALS._walkRegions.proc1(26);
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS._bookmark == bFinishedWGreen) {
			_sceneMode = 9013;
			_lyle.postInit();
			_lyle._moveDiff.y = 7;
			_lyle._flags |= 0x1000;
			_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, NULL);
			_lyleHatSpeaker._xp = 210;
			_jakeJacketSpeaker._xp = 75;
			setAction(&_sequenceManager1, this, 9013, &BF_GLOBALS._player, &_lyle, NULL);
			BF_GLOBALS._bookmark = bAmbushed;
			BF_GLOBALS.setFlag(fWithLyle);
		} else if (BF_GLOBALS._bookmark > bFinishedWGreen) {
			_lyle.postInit();
			_lyle.setVisage(811);
			_lyle.setPosition(Common::Point(780, 153));
			_lyle._moveDiff.y = 7;
			_lyle._flags |= 0x1000;
			_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, NULL);
			_lyle.animate(ANIM_MODE_1, NULL);
			_lyle.setObjectWrapper(new SceneObjectWrapper());
		} // no else on purpose
		_sceneMode = 9000;
		setAction(&_sequenceManager1, this, 9000, &BF_GLOBALS._player, NULL);
	} else {
		_object2._flag = 0;
		_object2.setFrame(_object2.getFrameCount());
		BF_GLOBALS._player.disableControl();
		_sceneMode = 9002;
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			_lyle.postInit();
			_lyle._flags |= 0x1000;
			_lyle.setDetails(900, 19, 20, 21, ANIM_MODE_1, NULL);
			setAction(&_sequenceManager1, this, 9014, &BF_GLOBALS._player, &_object2, &_lyle, NULL);
		} else
			setAction(&_sequenceManager1, this, 9002, &BF_GLOBALS._player, &_object2, NULL);
	}

	_object1.setDetails(900, 0, -1, 1, 1, 0);
	_object2.setDetails(900, 2, -1, 5, 1, 0);
	_item2.setDetails(Rect(0, 0, 225, 21), 666, 25, -1, -1, 1, NULL);
	_item3.setDetails(Rect(37, 21, 324, 50), 666, 26, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 960, 200), 900, 7, -1, -1, 1, NULL);
}

void Scene900::signal() {
	static uint32 v50E8B = 0;

	switch (_sceneMode++) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(910);
		break;
	case 3:
		BF_GLOBALS._walkRegions.proc1(24);
		_sceneMode = 9004;
		setAction(&_sequenceManager1, this, 9006, &BF_GLOBALS._player, &_object1, NULL);
		break;
	case 9000:
		BF_GLOBALS._player.enableControl();
		break;
	case 9001:
		if ((BF_INVENTORY.getObjectScene(INV_FISHING_NET) == 900) || (BF_GLOBALS._v4CEC0 != 0) || 
				(_object2._flag == 0))
			BF_GLOBALS.setFlag(fLeftTraceIn900);
		else
			BF_GLOBALS.clearFlag(fLeftTraceIn900);

		BF_GLOBALS._sceneManager.changeScene(880);
		break;
	case 9002:
		BF_GLOBALS._walkRegions.proc1(26);
		BF_GLOBALS._player.enableControl();
		break;
	case 9004:
		if (BF_GLOBALS._v4CEC0 == 0)
			SceneItem::display2(900, 3);
		else
			SceneItem::display2(900, 4);
		BF_GLOBALS._player.enableControl();
		break;
	case 9005:
		if (_object3._flag == 0)
			BF_GLOBALS._player.setAction(&_action4);
		else
			BF_GLOBALS._player.enableControl();
		break;
	case 9006:
		BF_GLOBALS._walkRegions.proc1(24);
		BF_GLOBALS._player.enableControl();
		break;
	case 9007:
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			Common::Point pt(862, 119);
			PlayerMover *mover = new PlayerMover();
			_lyle.addMover(mover, &pt, NULL);
			_sceneMode = 1;
			addFader((const byte *)&v50E8B, 5, this);
		} else
			BF_GLOBALS._sceneManager.changeScene(910);
		break;
	case 9008:
		BF_GLOBALS._deathReason = 5;
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 9010:
		_sound1.play(92);
		if (BF_GLOBALS._v4CEC0 == 2) {
			_sceneMode = 9008;
			setAction(&_sequenceManager1, this, 9008, &BF_GLOBALS._player, &_object3, NULL);
		} else {
			BF_GLOBALS._player._strip = 7;
			_action1.setActionIndex(9);
			_object3.signal();
			if ((!BF_GLOBALS.getFlag(fGotPointsForFreeDog)) && (BF_GLOBALS._bookmark == bEndDayThree)) {
				BF_GLOBALS.setFlag(fGotPointsForFreeDog);
				BF_GLOBALS._uiElements.addScore(50);
			}
			BF_INVENTORY.setObjectScene(INV_FISHING_NET, 1);
			SceneItem::display2(900, 11);
			BF_GLOBALS._player.enableControl();
		}
		break;
	case 9012:
		if (_object2._flag == 0) {
			SceneItem::display2(900, 12);
			_object2._flag = 1;
			if ((!BF_GLOBALS.getFlag(fGotPointsForLockWarehouse)) && (BF_GLOBALS._bookmark == bEndDayThree)) {
				BF_GLOBALS.setFlag(fGotPointsForLockWarehouse);
				BF_GLOBALS._uiElements.addScore(30);
			}
		} else {
			SceneItem::display2(900, 13);
			_object2._flag = 0;
			if (!BF_GLOBALS.getFlag(fGotPointsForUnlockWarehouse)) {
				BF_GLOBALS.setFlag(fGotPointsForUnlockWarehouse);
				BF_GLOBALS._uiElements.addScore(30);
			}
		}
		BF_GLOBALS._player.enableControl();
		break;
	case 9013:
		_lyleHatSpeaker._xp = 75;
		_jakeJacketSpeaker._xp = 210;
		_lyle.setAction(&_sequenceManager2, &_lyle, 9015, &_lyle, NULL);
		BF_GLOBALS._player.enableControl();
		break;
	case 9016:
		if ((BF_GLOBALS._clip1Bullets == 0) && (BF_GLOBALS._clip2Bullets == 0)){
			BF_GLOBALS._clip1Bullets = 8;
			BF_GLOBALS._clip1Bullets = 8;
			SceneItem::display2(900, 25);
		} else if (BF_GLOBALS._clip1Bullets == 0) {
			BF_GLOBALS._clip1Bullets = 8;
			SceneItem::display2(900, 26);
		} else if (BF_GLOBALS._clip2Bullets == 0) {
			BF_GLOBALS._clip2Bullets = 8;
			SceneItem::display2(900, 26);
		} else
			SceneItem::display2(900, 27);

		BF_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene900::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_eventHandler && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		if (_item4.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_N);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene900::dispatch() {
	SceneExt::dispatch();

	if (BF_GLOBALS.getFlag(fWithLyle) && _lyle.isNoMover()) {
		_lyle.updateAngle(BF_GLOBALS._player._position);
	}

	if (!_action) {
		if ((BF_GLOBALS._player._position.x <= 20) && (BF_GLOBALS._player._position.y < 130)) {
			BF_GLOBALS._player.disableControl();
			_sceneMode = 9001;
			setAction(&_sequenceManager1, this, 9001, &BF_GLOBALS._player, NULL);
		}
	}
}

void Scene900::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_field1974);
	s.syncAsSint16LE(_field1976);
}

} // End of namespace BlueForce

} // End of namespace TsAGE
