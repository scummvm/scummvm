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

#include "tsage/blue_force/blueforce_scenes4.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 410 - Traffic Stop Gang Members
 *
 *--------------------------------------------------------------------------*/

void Scene410::Action1::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (scene->_field1FB6++) {
	case 0:
		if (BF_GLOBALS.getFlag(fTalkedDriverNoBkup)) {
			setDelay(3);
		} else {
			scene->_sceneMode = 4101;
			scene->_stripManager.start(4103, scene);
		}
		break;
	case 1:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4104, scene);
		break;
	case 2:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4105, scene);
		break;
	default:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4106, scene);
		break;
	}
}

void Scene410::Action2::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;
	BF_GLOBALS._player.disableControl();

	switch (scene->_field1FB8++) {
	case 0:
		scene->_sceneMode = 4105;
		scene->_stripManager.start(BF_GLOBALS.getFlag(fTalkedShooterNoBkup) ? 4123 : 4107, scene);
		break;
	case 1:
		scene->_sceneMode = 4110;
		scene->_stripManager.start(4102, scene);
		break;
	case 2:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4108, scene);
		break;
	case 3:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4109, scene);
		break;
	case 4:
		scene->_sceneMode = 0;
		scene->_stripManager.start(4110, scene);
		break;
	default:
		SceneItem::display(410, 11, SET_WIDTH, 300,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 10,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + BF_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 13, SET_EXT_BGCOLOR, 82,
				SET_EXT_FGCOLOR, 13, LIST_END);
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene410::Action3::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;
	if (BF_GLOBALS.getFlag(fTalkedShooterNoBkup)) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 2;
		scene->setAction(&scene->_sequenceManager1, scene, 4102, &scene->_object2, &BF_GLOBALS._player, NULL);
	} else {
		BF_GLOBALS.setFlag(fTalkedShooterNoBkup);
		scene->_sceneMode = 0;
		scene->_stripManager.start(4107, scene);
	}
}

void Scene410::Action4::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (scene->_field1FC4 == 0) {
			ADD_PLAYER_MOVER(114, 133);
		} else {
			ADD_PLAYER_MOVER(195, 139);
		}
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_object2._position);
		setDelay(3);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		if (BF_GLOBALS.getFlag(fCalledBackup))
			scene->setAction(&scene->_action2);
		else
			scene->setAction(&scene->_action3);
		remove();
		break;
	default:
		break;
	}
}

void Scene410::Action5::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (scene->_field1FC4 == 0) {
			ADD_PLAYER_MOVER(114, 133);
		} else {
			ADD_PLAYER_MOVER(195, 139);
		}
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_object2._position);
		setDelay(3);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		if (BF_GLOBALS.getFlag(fCalledBackup))
			scene->setAction(&scene->_action2);
		else
			scene->setAction(&scene->_action3);
		remove();
		break;
	default:
		break;
	}
}

void Scene410::Action6::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if (scene->_field1FC4 == 0) {
			ADD_PLAYER_MOVER(114, 133);
		} else {
			ADD_PLAYER_MOVER(126, 99);
		}
		break;
	case 1:
		BF_GLOBALS._player.updateAngle(scene->_object1._position);
		setDelay(3);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		if (BF_GLOBALS.getFlag(fCalledBackup))
			scene->setAction(&scene->_action1);
		else
			scene->setAction(&scene->_action4);
		remove();
		break;
	default:
		break;
	}
}

void Scene410::Action7::signal() {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(2);
		break;
	case 1:
		BF_GLOBALS._bookmark = bStoppedFrankie;
		BF_GLOBALS.set2Flags(f1098Frankie);
		BF_GLOBALS.clearFlag(f1098Marina);
		scene->_stripManager.start(4113, this);
		break;
	case 2:
	case 4:
		setDelay(2);
		break;
	case 3:
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.updateAngle(Common::Point(100, 170));
		scene->setAction(&scene->_sequenceManager1, this, 4112, &scene->_object1, &scene->_object2,
			&scene->_object3, NULL);
		break;
	case 5:
		BF_GLOBALS._uiElements.addScore(30);
		setAction(&scene->_sequenceManager1, this, 4118, &BF_GLOBALS._player, NULL);
		BF_GLOBALS._player.disableControl();
		break;
	case 6:
		scene->_sceneMode = 3;
		scene->signal();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene410::Item2::startAction(CursorType action, Event &event) {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fCalledBackup)) {
			scene->_sceneMode = 4103;
			scene->signal();
		} else if (BF_GLOBALS.getFlag(fSearchedTruck) && !BF_GLOBALS._sceneObjects->contains(&scene->_object3)) {
			scene->_sceneMode = 4103;
			scene->signal();
		} else if (scene->_field1FBC != 0) {
			SceneItem::display2(410, 12);
		} else {
			scene->_sceneMode = 4103;
			scene->signal();
		}
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene410::Item4::startAction(CursorType action, Event &event) {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if ((BF_GLOBALS._bookmark < bStoppedFrankie) && (!scene->_field1FBC || !scene->_field1FBA))
			break;
		else if (BF_GLOBALS.getFlag(fSearchedTruck))
			SceneItem::display2(410, 13);
		else if (BF_GLOBALS.getFlag(fGangInCar)) {
			BF_GLOBALS._uiElements.addScore(30);
			scene->_sceneMode = 4118;
			scene->signal();
		} else {
			scene->setAction(&scene->_action7);
		}
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}


/*--------------------------------------------------------------------------*/

bool Scene410::Object1::startAction(CursorType action, Event &event) {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fCalledBackup)) {
			if (BF_GLOBALS.getFlag(fTalkedDriverNoBkup)) {
				scene->setAction(&scene->_action4);
			} else {
				SceneItem::display2(410, 7);
			}
		} else if (!scene->_field1FBC) {
			SceneItem::display2(410, 7);
		} else if (!scene->_field1FC0) {
			scene->_sceneMode = 4124;
			scene->_field1FC0 = 1;
			BF_GLOBALS._uiElements.addScore(30);
			scene->signal();
		} else {
			break;
		}
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.setAction(&scene->_action6);
		return true;
	case INV_HANDCUFFS:
		if (BF_GLOBALS.getFlag(fCalledBackup)) {
			if ((scene->_field1FB8 < 5) || (scene->_field1FB6 < 1) || (scene->_field1FBC != 0))
				break;

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 4123;
			scene->_stripManager.start(4125, scene);
			scene->_field1FBC = 1;
			BF_GLOBALS._uiElements.addScore(30);
		} else {
			if (BF_GLOBALS.getFlag(fTalkedDriverNoBkup)) {
				scene->setAction(&scene->_action4);
			} else {
				break;
			}
		}
		return true;
	case INV_TICKET_BOOK:
		if (!BF_GLOBALS.getFlag(fDriverOutOfTruck)) {
			return startAction(CURSOR_TALK, event);
		} else if (!scene->_field1FC4) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager1, scene, 4120, &scene->_object2, &BF_GLOBALS._player, NULL);
		} else if ((scene->_field1FBC != 0) || (scene->_field1FC2 != 0)) {
			break;
		} else {
			scene->_field1FC2 = 1;
			BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 4127;
			scene->setAction(&scene->_sequenceManager1, scene, 4127, &scene->_object1, NULL);
		}
		return true;
	case INV_MIRANDA_CARD:
		if (scene->_field1FBC == 0)
			return false;

		if (BF_GLOBALS.getFlag(readFrankRights)) {
			SceneItem::display2(390, 15);
		} else {
			BF_GLOBALS.setFlag(readFrankRights);
			BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 0;

			// TODO: Original code pushes a copy of the entirety of object1 onto stack
			scene->setAction(&scene->_sequenceManager1, scene, 4126, &BF_GLOBALS._player, &scene->_object1, NULL);
		}
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

bool Scene410::Object2::startAction(CursorType action, Event &event) {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (!BF_GLOBALS.getFlag(fCalledBackup)) {
			if (BF_GLOBALS.getFlag(fTalkedShooterNoBkup)) {
				scene->setAction(&scene->_action3);
			} else {
				SceneItem::display2(410, 5);
			}
		} else if (!scene->_field1FBA) {
			SceneItem::display2(410, 5);
		} else if (!scene->_field1FBE) {
			scene->_sceneMode = 4121;
			scene->_field1FBE = 1;
			BF_GLOBALS._uiElements.addScore(50);
			scene->signal();
		} else {
			break;
		}
		return true;
	case CURSOR_TALK:
		scene->setAction(&scene->_action5);
		return true;
	case INV_HANDCUFFS:
		if (BF_GLOBALS.getFlag(fCalledBackup)) {
			if ((scene->_field1FB8 < 5) || (scene->_field1FBA != 0))
				break;

			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 9;
			ADD_PLAYER_MOVER(195, 139);
		} else if (BF_GLOBALS.getFlag(fTalkedShooterNoBkup)) {
			scene->setAction(&scene->_action3);
		} else {
			break;
		}
		return true;
	case INV_MIRANDA_CARD:
		if (!scene->_field1FBA)
			break;

		if (BF_GLOBALS.getFlag(readFrankRights)) {
			SceneItem::display2(390, 15);
		} else {
			BF_GLOBALS.setFlag(readFrankRights);
			BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS._player.disableControl();

			scene->_sceneMode = 0;
			scene->setAction(&scene->_sequenceManager1, scene, 4125, &BF_GLOBALS._player,
				&scene->_object2, NULL);
		}
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

bool Scene410::Object3::startAction(CursorType action, Event &event) {
	Scene410 *scene = (Scene410 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (BF_GLOBALS._bookmark == bStoppedFrankie) {
			if (BF_GLOBALS.getFlag(fSearchedTruck)) {
				BF_GLOBALS._player.disableControl();
				scene->_sceneMode = 8;
				ADD_PLAYER_MOVER(147, 143);
			} else {
				SceneItem::display2(350, 13);
			}
		} else if ((scene->_field1FBA != 0) && (scene->_field1FBC != 0)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 4112;
			scene->_stripManager.start(4113, scene);
			BF_GLOBALS._bookmark = bStoppedFrankie;
			BF_GLOBALS.set2Flags(f1098Frankie);
			BF_GLOBALS.clearFlag(f1098Marina);
		} else if ((BF_INVENTORY.getObjectScene(INV_HANDCUFFS) == 1) ||
				(!scene->_field1FBA && (scene->_field1FB8 < 5))) {
			SceneItem::display2(350, 13);
		} else if (!scene->_field1FBA) {
			if (scene->_field1FBA)
				error("Error - want to cuff shooter, but he's cuffed already");

			BF_GLOBALS._player.disableControl();
			scene->_field1FBA = 1;
			scene->_field1FBE = 1;
			BF_GLOBALS._walkRegions.proc2(22);
			scene->_sceneMode = 4122;
			scene->_stripManager.start(4112, scene);
		} else if (scene->_field1FB6 < 1) {
			break;
		} else if (scene->_field1FBC != 0) {
			error("Error - want to cuff driver, but he's cuffed already");
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_field1FBC = 1;
			scene->_field1FC0 = 1;
			BF_GLOBALS._walkRegions.proc2(22);
			scene->_sceneMode = 4109;
			scene->_stripManager.start(4112, scene);
		}
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}


/*--------------------------------------------------------------------------*/

Scene410::Scene410(): SceneExt() {
	_field1FB6 = _field1FB8 = _field1FBA = _field1FBC = 0;
	_field1FBE = _field1FC0 = _field1FC2 = _field1FC4 = 0;
}

void Scene410::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_field1FB6);
	s.syncAsSint16LE(_field1FB8);
	s.syncAsSint16LE(_field1FBA);
	s.syncAsSint16LE(_field1FBC);
	s.syncAsSint16LE(_field1FBE);
	s.syncAsSint16LE(_field1FC0);
	s.syncAsSint16LE(_field1FC2);
	s.syncAsSint16LE(_field1FC4);
}

void Scene410::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(410);
	setZoomPercents(74, 75, 120, 100);

	if (BF_GLOBALS._sceneManager._previousScene != 60)
		_sound1.fadeSound(11);
	BF_GLOBALS._driveToScene = 64;
	BF_GLOBALS._mapLocationId = 64;
	BF_GLOBALS.setFlag(fArrivedAtGangStop);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);
	_stripManager.addSpeaker(&_harrisonSpeaker);
	_stripManager.addSpeaker(&_shooterSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(1341);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.changeAngle(90);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setPosition(Common::Point(114, 133));
	BF_GLOBALS._player.changeZoom(-1);

	_item2.setDetails(8, 410, 15, -1, -1, 1);

	_object2.postInit();
	_object2.setVisage(415);
	_object2.setStrip(1);
	_object2.setPosition(Common::Point(278, 92));
	_object2.setDetails(410, 4, -1, 5, 1, NULL);

	_object1.postInit();
	_object1.setVisage(416);
	_object1.setStrip(2);
	_object1.setPosition(Common::Point(244, 85));
	_object1.setDetails(410, 6, -1, 7, 1, NULL);
	_object1.changeZoom(-1);

	_object5.postInit();
	_object5.setVisage(410);
	_object5.setStrip(2);
	_object5.setPosition(Common::Point(282, 96));

	_object6.postInit();
	_object6.setVisage(410);
	_object6.setStrip(4);
	_object6.setPosition(Common::Point(240, 43));

	_item4.setDetails(6, 410, 3, -1, -1, 1);
	_item3.setDetails(7, 410, 3, -1, -1, 1);

	switch (BF_GLOBALS._sceneManager._previousScene) {
	case 415:
		BF_GLOBALS.setFlag(fSearchedTruck);
		BF_GLOBALS._player.setPosition(Common::Point(210, 90));

		_object2.remove();
		_object1.remove();
		BF_GLOBALS._walkRegions.proc1(21);
		BF_GLOBALS._walkRegions.proc1(22);

		_object3.postInit();
		_object3.setVisage(343);
		_object3.setObjectWrapper(new SceneObjectWrapper());
		_object3.animate(ANIM_MODE_1, NULL);
		_object3.setDetails(350, 12, 13, 14, 1, NULL);
		_object3.setPosition(Common::Point(97, 185));
		_object3.changeZoom(-1);

		_object4.postInit();
		_object4.setVisage(410);
		_object4.setDetails(410, 8, 9, 10, 1, NULL);
		_object4.fixPriority(148);
		_object4.setPosition(Common::Point(39, 168));

		_field1FC4 = 1;
		_sceneMode = 0;
		signal();
		break;
	case 60:
		if (BF_GLOBALS.getFlag(fSearchedTruck)) {
			_object2.remove();
			_object1.remove();
			_sceneMode = 0;
		} else {
			_field1FC4 = BF_GLOBALS._v50CC8;
			_field1FBA = BF_GLOBALS._v50CC2;
			_field1FB8 = BF_GLOBALS._v50CC6;
			_field1FB6 = BF_GLOBALS._v50CC4;

			_object2.setVisage(418);
			_object2.setStrip(6);
			_object2.setPosition(Common::Point(227, 137));

			if (_field1FB8 > 0) {
				_object2.setVisage(415);
				_object2.setStrip(2);
				_object2.setFrame(5);
			}
			if (_field1FBA) {
				_object2.setVisage(415);
				_object2.setStrip(6);
				_object2.setFrame(8);
			}

			BF_GLOBALS._walkRegions.proc1(16);
			if (BF_GLOBALS.getFlag(fDriverOutOfTruck)) {
				_object1.setVisage(417);
				_object1.setStrip(1);
				_object1.setPosition(Common::Point(152, 97));

				BF_GLOBALS._walkRegions.proc1(7);
			}

			if (BF_GLOBALS.getFlag(fCalledBackup)) {
				BF_GLOBALS._walkRegions.proc1(21);
				BF_GLOBALS._walkRegions.proc1(22);

				_object3.postInit();
				_object3.setVisage(343);
				_object3.setObjectWrapper(new SceneObjectWrapper());
				_object3.animate(ANIM_MODE_1, NULL);
				_object3.setDetails(350, 12, 13, 14, 1, NULL);
				BF_GLOBALS._sceneItems.addBefore(&_object1, &_object3);

				_object3.setPosition(Common::Point(-10, 124));
				_object3.changeZoom(-1);

				_object4.postInit();
				_object4.setVisage(410);
				_object4.setDetails(410, 8, 9, 10, 1, NULL);
				_object4.fixPriority(148);

				if (_field1FC4) {
					_object3.setPosition(Common::Point(108, 112));
					_object4.fixPriority(148);
					_object4.setPosition(Common::Point(39, 168));

					_sceneMode = 0;
				} else {
					_sceneMode = 4104;
				}
			} else {
				_sceneMode = 0;
			}

			_field1FC4 = 1;
		}
		break;
	case 50:
	default:
		BF_GLOBALS.setFlag(onDuty);
		_sound1.play(21);
		_sceneMode = 4100;
		break;
	}

	signal();

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y), 410, 0, 1, 2, 1, NULL);
}

void Scene410::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS.set2Flags(f1097Frankie);
		BF_GLOBALS.clearFlag(f1097Marina);

		BF_GLOBALS._v50CC8 = _field1FC4;
		BF_GLOBALS._v50CC2 = _field1FBA;
		BF_GLOBALS._v50CC6 = _field1FB8;
		BF_GLOBALS._v50CC4 = _field1FB6;
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 2:
		BF_GLOBALS._deathReason = 3;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(666);
		break;
	case 3:
		BF_GLOBALS._sceneManager.changeScene(415);
		break;
	case 5:
		BF_INVENTORY.setObjectScene(INV_HANDCUFFS, 315);
		_sceneMode = 0;
		BF_GLOBALS.set2Flags(f1015Frankie);
		BF_GLOBALS.clearFlag(f1015Marina);
		signal();
		break;
	case 6:
		BF_INVENTORY.setObjectScene(INV_22_SNUB, 1);
		BF_INVENTORY.setObjectScene(INV_FRANKIE_ID, 1);
		BF_GLOBALS.set2Flags(f1027Frankie);
		BF_GLOBALS.clearFlag(f1027Marina);
		_sceneMode = 0;
		signal();
		break;
	case 7:
		BF_INVENTORY.setObjectScene(INV_TYRONE_ID, 1);
		signal();
		break;
	case 8:
		BF_GLOBALS._walkRegions.proc2(22);
		BF_GLOBALS._player.changeAngle(225);
		_object3.changeAngle(45);
		_sceneMode = 4114;
		_stripManager.start(4120, this);
		break;
	case 9:
		_sceneMode = 4106;
		_stripManager.start(4111, this);
		_field1FBA = 1;
		BF_GLOBALS.setFlag(fCuffedFrankie);
		BF_GLOBALS._uiElements.addScore(30);
		break;
	case 10:
		BF_GLOBALS._player.updateAngle(_object3._position);
		_sceneMode = 0;
		break;
	case 4100:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4100, &_object2, &_object5, NULL);
		BF_GLOBALS._walkRegions.proc1(16);
		break;
	case 4101:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4100, &_object1, &_object6, NULL);
		BF_GLOBALS.setFlag(fDriverOutOfTruck);
		BF_GLOBALS._walkRegions.proc1(7);
		break;
	case 4103:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 1;
		setAction(&_sequenceManager1, this, 4103, &BF_GLOBALS._player, NULL);
		break;
	case 4104:
		_field1FC4 = 1;
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4104, &_object4, &_object3, NULL);
		break;
	case 4105:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4105, &_object2, NULL);
	case 4106:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4119;
		setAction(&_sequenceManager1, this, 4106, &_object2, NULL);
		break;
	case 4107:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 5;
		setAction(&_sequenceManager1, this, 4107, &_object2, NULL);
		break;
	case 4108:
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.updateAngle(Common::Point(100, 170));
		BF_GLOBALS._walkRegions.proc2(22);
		BF_GLOBALS._walkRegions.proc2(16);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4108, &_object3, NULL);
		break;
	case 4109:
		if ((BF_GLOBALS._player._position.x > 116) && (BF_GLOBALS._player._position.x != 195) &&
				(BF_GLOBALS._player._position.y != 139)) {
			ADD_PLAYER_MOVER(195, 139);
		}

		BF_GLOBALS._walkRegions.proc2(22);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4109, &_object1, &_object3, NULL);
		break;
	case 4110:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4110, &_object3, &_object1, NULL);
		break;
	case 4112:
		BF_GLOBALS.setFlag(fGangInCar);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.updateAngle(Common::Point(100, 170));
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4108;
		setAction(&_sequenceManager1, this, 4112, &_object1, &_object2, &_object3, NULL);
		break;
	case 4114:
		BF_GLOBALS._walkRegions.proc2(22);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4116;
		setAction(&_sequenceManager1, this, 4114, &_object3, &_object4, NULL);
		break;
	case 4116:
		BF_GLOBALS._walkRegions.proc2(21);
		BF_GLOBALS._walkRegions.proc2(22);
		_object3.remove();
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4116, &_object4, NULL);
		break;
	case 4118:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 3;
		setAction(&_sequenceManager1, this, 4118, &BF_GLOBALS._player, NULL);
		break;
	case 4119:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 4107;
		setAction(&_sequenceManager1, this, 4119, &_object2, NULL);
		break;
	case 4121:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 6;
		setAction(&_sequenceManager1, this, 4121, &BF_GLOBALS._player, &_object2, NULL);
		break;
	case 4122:
		BF_GLOBALS._walkRegions.proc2(22);
		BF_INVENTORY.setObjectScene(INV_22_SNUB, 0);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 0;
		setAction(&_sequenceManager1, this, 4122, &_object2, &_object3, NULL);
		break;
	case 4123:
		BF_INVENTORY.setObjectScene(INV_TYRONE_ID, 0);
		BF_GLOBALS._player.disableControl();
		_sceneMode = 5;
		setAction(&_sequenceManager1, this, 4123, &_object1, &BF_GLOBALS._player, NULL);
		break;
	case 4124:
		BF_GLOBALS._player.disableControl();
		_sceneMode = 7;
		setAction(&_sequenceManager1, this, 4124, &_object1, &BF_GLOBALS._player, NULL);
		break;
	case 4127:
		SceneItem::display2(410, 16);
		BF_GLOBALS._player.enableControl();
		break;
	case 0:
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene410::process(Event &event) {
	// Check for gun being clicked on player
	if ((event.eventType == EVENT_BUTTON_DOWN) && (BF_GLOBALS._events.getCursor() == INV_COLT45) &&
			BF_GLOBALS._player.contains(event.mousePos) && !BF_GLOBALS.getFlag(fCalledBackup)) {
		if (BF_GLOBALS.getFlag(fTalkedShooterNoBkup)) {
			if (!_action) {
				event.handled = true;
				BF_GLOBALS._player.disableControl();
				_sceneMode = 2;
				setAction(&_sequenceManager1, this, 4102, &_object2, &BF_GLOBALS._player, NULL);
			}
		} else if (BF_GLOBALS.getFlag(fTalkedDriverNoBkup)) {
			if (!_action) {
				_sceneMode = 1;
				BF_GLOBALS._player.disableControl();
				_sceneMode = 2;
				setAction(&_sequenceManager1, this, 4120, &_object2, &BF_GLOBALS._player, NULL);
			}
		}
	}

	if (!event.handled)
		SceneExt::process(event);
}

void Scene410::dispatch() {
	SceneExt::dispatch();
	if ((_sceneMode == 4112) || (_sceneMode == 4101)) {
		_object3.updateAngle(_object1._position);
	}
}

/*--------------------------------------------------------------------------
 * Scene 415 - Searching Truck
 *
 *--------------------------------------------------------------------------*/

bool Scene415::GunInset::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (BF_GLOBALS.getFlag(fGotAutoWeapon)) {
			FocusObject::startAction(action, event);
		} else {
			remove();
			scene->_gunAndWig.remove();
		}
		return true;
	} else {
		return FocusObject::startAction(action, event);
	}
}

bool Scene415::GunAndWig::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		NamedObject::startAction(action, event);
		BF_INVENTORY.setObjectScene(INV_AUTO_RIFLE, 1);
		BF_INVENTORY.setObjectScene(INV_WIG, 1);
		BF_GLOBALS.setFlag(fGotAutoWeapon);
		BF_GLOBALS._uiElements.addScore(30);

		remove();
		return true;
	case INV_FOREST_RAP:
		if (scene->_fieldE14)
			break;

		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		scene->_stripManager.start(4126, scene);
		BF_GLOBALS._uiElements.addScore(50);
		scene->_fieldE14 = true;
		return true;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

bool Scene415::BulletsInset::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if (BF_GLOBALS.getFlag(fGotAutoWeapon)) {
			FocusObject::startAction(action, event);
		} else {
			remove();
			scene->_theBullets.remove();
		}
		return true;
	} else {
		return FocusObject::startAction(action, event);
	}
}

bool Scene415::DashDrawer::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_LOOK) || (action == CURSOR_USE)) {
		scene->showBullets();
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

bool Scene415::TheBullets::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		NamedObject::startAction(action, event);
		BF_INVENTORY.setObjectScene(INV_22_BULLET, 1);
		BF_GLOBALS.setFlag(fGotBulletsFromDash);
		BF_GLOBALS._uiElements.addScore(30);

		remove();
		scene->_dashDrawer.remove();
		return true;
	case INV_FOREST_RAP:
		if (scene->_fieldE16) {
			SceneItem::display2(415, 35);
			return true;
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 0;
			scene->_stripManager.start(4122, scene);
			BF_GLOBALS._uiElements.addScore(50);
			scene->_fieldE16 = true;
		}
		break;
	default:
		break;
	}

	return NamedObject::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene415::Lever::startAction(CursorType action, Event &event) {
	Scene415 *scene = (Scene415 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (BF_GLOBALS.getFlag(fGotAutoWeapon)) {
			SceneItem::display2(415, 20);
		} else {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 2;
			scene->setAction(&scene->_sequenceManager, scene, 4150, &scene->_object6, NULL);
		}
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene415::Scene415(): SceneExt() {
	_fieldE14 = _fieldE16 = false;
}

void Scene415::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_fieldE14);
	s.syncAsSint16LE(_fieldE16);
}

void Scene415::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(415);

	_stripManager.addSpeaker(&_jakeRadioSpeaker);

	_dashDrawer.postInit();
	_dashDrawer.setVisage(411);
	_dashDrawer.setStrip(3);
	_dashDrawer.setPosition(Common::Point(151, 97));
	_dashDrawer.setDetails(415, 22, -1, -1, 1, NULL);

	_object6.postInit();
	_object6.setVisage(419);
	_object6.setStrip(1);
	_object6.setPosition(Common::Point(306, 116));
	_object6.fixPriority(80);
	
	_windowLever.setDetails(16, 415, 25, -1, 26, 1);
	_item7.setDetails(17, 415, 32, -1, 33, 1);
	_seatBelt.setDetails(14, 415, 29, -1, 30, 1);
	_lever.setDetails(19, 415, 23, 24, -1, 1);
	_seat.setDetails(18, 415, 3, 4, 2, 1);
	_dashboard.setDetails(20, 415, 11, 12, 19, 1);
	_steeringWheel.setDetails(15, 415, 5, 6, 7, 1);
	_horn.setDetails(31, 415, 8, 9, 10, 1);
	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y), 415, 0, 1, 2, 1, NULL);

	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._player._canWalk = false;
	BF_GLOBALS._events.setCursor(CURSOR_WALK);

	signal();
}

void Scene415::signal() {
	switch (_sceneMode) {
	case 1:
		BF_GLOBALS._sceneManager.changeScene(410);
		break;
	case 2:
		showGunAndWig();
		_sceneMode = 0;
		signal();
		break;
	case 0:
	default:
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene415::dispatch() {
	SceneExt::dispatch();
	if (BF_GLOBALS.getFlag(fGotAutoWeapon) && BF_GLOBALS.getFlag(fGotBulletsFromDash)) {
		_sceneMode = 1;
		signal();
	}
}

void Scene415::showBullets() {
	_bulletsInset.postInit();
	_bulletsInset.setVisage(411);
	_bulletsInset.setStrip(1);
	_bulletsInset.setPosition(Common::Point(158, 100));
	_bulletsInset.setDetails(415, -1, -1, -1);

	_theBullets.postInit();
	_theBullets.setVisage(411);
	_theBullets.setStrip(1);
	_theBullets.setFrame(2);
	_theBullets.setPosition(Common::Point(184, 86));
	_theBullets.fixPriority(105);
	_theBullets.setDetails(415, 16, 17, 18, 1, NULL);
	BF_GLOBALS._sceneItems.remove(&_theBullets);
	BF_GLOBALS._sceneItems.push_front(&_theBullets);
}

void Scene415::showGunAndWig() {
	_gunInset.postInit();
	_gunInset.setVisage(411);
	_gunInset.setStrip(2);
	_gunInset.setPosition(Common::Point(158, 100));
	_gunInset.setDetails(415, -1, -1, -1);

	_gunAndWig.postInit();
	_gunAndWig.setVisage(411);
	_gunAndWig.setStrip(2);
	_gunAndWig.setFrame(2);
	_gunAndWig.setPosition(Common::Point(159, 88));
	_gunAndWig.fixPriority(105);
	_gunAndWig.setDetails(415, 13, 14, 15, 1, NULL);

	BF_GLOBALS._sceneItems.remove(&_gunAndWig);
	BF_GLOBALS._sceneItems.push_front(&_gunAndWig);
}

/*--------------------------------------------------------------------------
 * Scene 440 - Outside Alleycat Bowl
 *
 *--------------------------------------------------------------------------*/

bool Scene440::Doorway::startAction(CursorType action, Event &event) {
	Scene440 *scene = (Scene440 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(440, 1);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4400;
		scene->setAction(&scene->_sequenceManager, scene, 4400, &BF_GLOBALS._player, this, &scene->_lyle, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene440::Vechile::startAction(CursorType action, Event &event) {
	Scene440 *scene = (Scene440 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			SceneItem::display2(440, 3);
		} else {
			SceneItem::display2(440, 2);
		}
		return true;
	case CURSOR_USE:
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			BF_GLOBALS._player.disableControl();
			scene->_sceneMode = 4403;
			scene->setAction(&scene->_sequenceManager, scene, 4403, &BF_GLOBALS._player, &scene->_lyle, NULL);
		} else {
			BF_GLOBALS._sceneManager.changeScene(60);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene440::Lyle::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(440, 4);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(440, 5);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene440::Item1::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(440, 0);
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene440::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(440);
	setZoomPercents(75, 60, 120, 100);
	BF_GLOBALS._sound1.fadeSound(33);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(303);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();
	BF_GLOBALS._player.setPosition(Common::Point(203, 113));

	_vechile.postInit();

	_lyle.postInit();
	_lyle.setVisage(835);
	_lyle.animate(ANIM_MODE_1, NULL);
	_lyle.setObjectWrapper(new SceneObjectWrapper());
	_lyle.setPosition(Common::Point(-40, -10));
	_lyle.changeZoom(-1);
	_lyle.hide();
	BF_GLOBALS._sceneItems.push_back(&_lyle);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		_vechile.setVisage(444);
		_vechile.setFrame(2);
		_vechile.setPosition(Common::Point(147, 128));
		_vechile.fixPriority(114);

		BF_GLOBALS._player.setVisage(303);
		BF_GLOBALS._player.setPosition(Common::Point(187, 104));
		
		_lyle.setPosition(Common::Point(135, 128));
		_lyle.show();

		BF_GLOBALS._walkRegions.proc1(12);
		BF_GLOBALS._walkRegions.proc1(13);
	} else {
		_vechile.setPosition(Common::Point(169, 121));
		_vechile.fixPriority(117);

		if (BF_GLOBALS.getFlag(onDuty)) {
			_vechile.setVisage(440);
			_vechile.setStrip(1);

			BF_GLOBALS._player.setVisage(304);
			BF_GLOBALS._player.setStrip(3);
		} else {
			_vechile.setVisage(580);
			_vechile.setStrip(2);
			_vechile.setFrame(3);
			
			BF_GLOBALS._player.setVisage(303);
		}
	}

	BF_GLOBALS._sceneItems.push_back(&_vechile);
	BF_GLOBALS._walkRegions.proc1(11);

	_doorway.postInit();
	_doorway.setVisage(440);
	_doorway.setStrip(5);
	_doorway.setPosition(Common::Point(198, 91));
	_doorway.fixPriority(80);
	BF_GLOBALS._sceneItems.push_back(&_doorway);

	if (BF_GLOBALS._sceneManager._previousScene == 450) {
		_lyle.setPosition(Common::Point(143, 93));
		_lyle.setStrip(5);
		_lyle.fixPriority(90);
		
		_doorway.setFrame(_doorway.getFrameCount());
		_sceneMode = 4401;
		setAction(&_sequenceManager, this, 4401, &BF_GLOBALS._player, &_doorway, NULL);
	} else if (BF_GLOBALS.getFlag(fWithLyle)) {
		_sceneMode = 4402;
		setAction(&_sequenceManager, this, 4402, &_lyle, NULL);
	} else {
		BF_GLOBALS._player.enableControl();
	}

	_item1.setBounds(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y - 1));
	BF_GLOBALS._sceneItems.push_back(&_item1);
}

void Scene440::signal() {
	switch (_sceneMode) {
	case 4400:
		BF_GLOBALS._sceneManager.changeScene(450);
		break;
	case 4401:
		BF_GLOBALS._player.fixPriority(-1);
		BF_GLOBALS._player.enableControl();
		break;
	case 4402:
		BF_GLOBALS._player.enableControl();
		break;
	case 4403:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 450 - Inside Alleycat Bowl
 *
 *--------------------------------------------------------------------------*/

bool Scene450::Object1::startAction(CursorType action, Event &event) {
	Scene450 *scene = (Scene450 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(450, 0);
		return true;
	case CURSOR_USE:
		SceneItem::display2(450, 1);
		return true;
	case CURSOR_TALK:
	case INV_ID:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4504;
		if (_flag) {
			scene->setAction(&scene->_sequenceManager, scene, 4515, &BF_GLOBALS._player, this, NULL);
		} else {
			++_flag;
			scene->setAction(&scene->_sequenceManager, scene, 4504, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	case INV_FOREST_RAP:
		BF_INVENTORY.setObjectScene(INV_FOREST_RAP, 450);
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._uiElements.addScore(30);

		scene->_sceneMode = 4505;
		scene->setAction(&scene->_sequenceManager, scene, 4505, &BF_GLOBALS._player, this, 
			&scene->_counterDoor, &scene->_object2, NULL);
		return true;
	default:
		return NamedObjectExt::startAction(action, event);
	}
}

bool Scene450::Object3::startAction(CursorType action, Event &event) {
	Scene450 *scene = (Scene450 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(450, 4);
		return true;
	case CURSOR_USE:
		SceneItem::display2(450, 5);
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4502;
		if (BF_GLOBALS.getFlag(onDuty)) {
			scene->setAction(&scene->_sequenceManager, scene, 4516, &BF_GLOBALS._player, this, NULL);
		} else {
			scene->setAction(&scene->_sequenceManager, scene, 4502, &BF_GLOBALS._player, this, NULL);
		}
		return true;
	case INV_NAPKIN:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4509;
		scene->setAction(&scene->_sequenceManager, scene, 4509, &BF_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene450::Object4::startAction(CursorType action, Event &event) {
	Scene450 *scene = (Scene450 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(450, 6);
		return true;
	case CURSOR_USE:
		SceneItem::display2(450, 7);
		return true;
	case CURSOR_TALK:
		BF_GLOBALS._player.disableControl();
		if (BF_GLOBALS.getFlag(takenWeasel) && !BF_GLOBALS.getFlag(gotTrailer450)) {
			BF_GLOBALS.setFlag(gotTrailer450);
			scene->_sceneMode = 4517;
			scene->setAction(&scene->_sequenceManager, scene, 4517, &BF_GLOBALS._player, this, 
				&scene->_door, NULL);
		} else {
			animate(ANIM_MODE_8, 1, NULL);

			if (scene->_field19AC) {
				scene->_sceneMode = 2;
				if (scene->_field19AE) {
					scene->_stripManager.start(4521, scene);
				} else {
					scene->_field19AE = 1;
					scene->_stripManager.start(4512, scene);
				}
			} else {
				scene->_sceneMode = 4506;
				if (scene->_field19AE) {
					scene->setAction(&scene->_sequenceManager, scene, 4518, &BF_GLOBALS._player, this, NULL);
				} else {
					scene->_sceneMode = 4506;
					scene->_field19AE = 1;
					scene->setAction(&scene->_sequenceManager, scene, 4506, &BF_GLOBALS._player, this, NULL);
				}
			}
		}
		return true;
	case INV_FOREST_RAP:
		SceneItem::display2(450, 19);
		return true;
	case INV_NAPKIN:
		animate(ANIM_MODE_8, 1, NULL);
		BF_GLOBALS._player.disableControl();

		if (BF_GLOBALS.getFlag(showEugeneNapkin)) {
			SceneItem::display2(450, 16);
			BF_GLOBALS._player.enableControl();
		} else {
			BF_GLOBALS.setFlag(showEugeneNapkin);

			if (!BF_GLOBALS.getFlag(showEugeneID)) {
				scene->_sceneMode = 4513;
				scene->setAction(&scene->_sequenceManager, scene, 4513, &BF_GLOBALS._player, this, NULL);
			} else if (BF_GLOBALS.getFlag(fMgrCallsWeasel)) {
				SceneItem::display2(450, 16);
				BF_GLOBALS._player.enableControl();
			} else {
				BF_GLOBALS._uiElements.addScore(30);
				scene->_sceneMode = 4510;
				BF_INVENTORY.setObjectScene(INV_NAPKIN, 450);
				scene->setAction(&scene->_sequenceManager, scene, 4510, &BF_GLOBALS._player, this, NULL);
			}
		}
		return true;
	case INV_ID:
		if (BF_GLOBALS.getFlag(takenWeasel)) {
			return startAction(CURSOR_TALK, event);
		} else {
			animate(ANIM_MODE_8, 1, NULL);
			BF_GLOBALS._player.disableControl();
			
			if (!BF_GLOBALS.getFlag(showEugeneID))
				BF_GLOBALS._uiElements.addScore(30);
			BF_GLOBALS.setFlag(showEugeneID);
			
			if ((BF_GLOBALS.getFlag(showRapEugene) || BF_GLOBALS.getFlag(showEugeneNapkin)) &&
					!BF_GLOBALS.getFlag(fMgrCallsWeasel)) {
				BF_GLOBALS._uiElements.addScore(30);
				scene->_sceneMode = 4511;
				scene->setAction(&scene->_sequenceManager, scene, 4511, &BF_GLOBALS._player, this, NULL);
			} else {
				scene->_sceneMode = 4506;
				scene->setAction(&scene->_sequenceManager, scene, 4512, &BF_GLOBALS._player, this, NULL);
			}
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene450::Item1::startAction(CursorType action, Event &event) {
	Scene450 *scene = (Scene450 *)BF_GLOBALS._sceneManager._scene;

	if (event.eventType == EVENT_BUTTON_DOWN) {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 4501;
		scene->signal();
		return true;
	} else {
		return false;
	}
}

/*--------------------------------------------------------------------------*/

Scene450::Scene450(): SceneExt() {
	_field19AC = _field19AE = 0;
}

void Scene450::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_field19AC);
	s.syncAsSint16LE(_field19AE);
}

void Scene450::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(450);
	setZoomPercents(110, 90, 155, 115);
	BF_GLOBALS._sound1.fadeSound(13);

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_eugeneSpeaker);
	_stripManager.addSpeaker(&_billySpeaker);
	_stripManager.addSpeaker(&_weaselSpeaker);
	_stripManager.addSpeaker(&_jakeJacketSpeaker);
	_stripManager.addSpeaker(&_lyleHatSpeaker);
	_stripManager.addSpeaker(&_jakeUniformSpeaker);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1341 : 129);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(-30, 155));
	BF_GLOBALS._player.changeZoom(-1);
	BF_GLOBALS._player.disableControl();

	_door.postInit();
	_door.setVisage(450);
	_door.setStrip(2);
	_door.setPosition(Common::Point(72, 80));
	_door.setDetails(450, 15, -1, 13, 1, NULL);

	_counterDoor.postInit();
	_counterDoor.setVisage(450);
	_counterDoor.setPosition(Common::Point(39, 104));
	_counterDoor.fixPriority(100);
	_counterDoor.setDetails(450, 12, -1, 13, 1, NULL);

	if (BF_GLOBALS._dayNumber != 3) {
		_object3.postInit();
		_object3.setVisage(463);
		_object3.setPosition(Common::Point(138, 121));
		_object3.fixPriority(100);
		_object3.setFrame(_object3.getFrameCount());
		BF_GLOBALS._sceneItems.push_back(&_object3);
	} else if (!BF_GLOBALS.getFlag(fWithLyle) || !BF_GLOBALS.getFlag(fGivenNapkin) ||
			(BF_INVENTORY.getObjectScene(BF_LAST_INVENT) == 1)) {
		_object3.postInit();
		_object3.setVisage(463);
		_object3.setPosition(Common::Point(138, 121));
		_object3.fixPriority(100);
		_object3.setFrame(_object3.getFrameCount());
		BF_GLOBALS._sceneItems.push_back(&_object3);
	} else {
		_object4.postInit();
		_object4.setVisage(467);
		_object4.setPosition(Common::Point(138, 121));
		_object4.changeZoom(-1);
		BF_GLOBALS._sceneItems.push_back(&_object4);

		if (!BF_GLOBALS.getFlag(takenWeasel)) {
			_object2.postInit();
			_object2.setVisage(469);
			_object2.animate(ANIM_MODE_1, NULL);
			_object2.setObjectWrapper(new SceneObjectWrapper());
			_object2.setPosition(Common::Point(-30, 126));
			ADD_MOVER_NULL(_object2, 27, 126);
			_object2.changeZoom(-1);
			_object2.setDetails(450, 2, 18, 3, 1, NULL);

			BF_GLOBALS._walkRegions.proc1(4);

			_object1.postInit();
			_object1.setVisage(466);
			_object1.animate(ANIM_MODE_1, NULL);
			_object1.setObjectWrapper(new SceneObjectWrapper());
			_object1.setPosition(Common::Point(70, 80));
			_object1.setStrip(5);
			_object1.changeZoom(90);
			_object1.fixPriority(65);
			_object1._flag = 0;
			BF_GLOBALS._sceneItems.push_back(&_object1);
		}
	}

	_sceneMode = 4500;
	setAction(&_sequenceManager, this, 4500, &BF_GLOBALS._player, NULL);

	_exit.setDetails(Rect(0, 100, 4, 167), 450, -1, -1, -1, 1, NULL);
	_counter.setDetails(8, 450, 8, -1, 9, 1);
	_shelf.setDetails(Rect(114, 10, 179, 77), 450, 10, -1, 11, 1, NULL);
	_interior.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 450, 14, -1, -1, 1, NULL);
}

void Scene450::signal() {
	switch (_sceneMode) {
	case 450:
	case 451:
		BF_GLOBALS._sceneManager.changeScene(440);
		break;
	case 4501:
		if (BF_GLOBALS._sceneObjects->contains(&_object2)) {
			ADD_MOVER(_object2, -20, 135);
		} else {
			ADD_PLAYER_MOVER(0, 160);
		}
		break;
	case 4503:
		_object1.fixPriority(100);
		BF_GLOBALS._player.enableControl();
		break;
	case 4505:
		BF_GLOBALS.setFlag(takenWeasel);
		_object1.remove();
		_object2.remove();
		BF_GLOBALS._walkRegions.proc2(4);
		BF_GLOBALS._player.enableControl();
		break;
	case 4507:
	case 4510:
	case 4511:
		BF_GLOBALS.setFlag(fMgrCallsWeasel);
		_field19AC = 1;
		_sceneMode = 4503;
		setAction(&_sequenceManager, this, 4503, &_object1, &_door, &_object4, NULL);
		break;
	case 4508:
		_object4.remove();
		BF_GLOBALS._player.enableControl();
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		break;
	case 4517:
		BF_GLOBALS.setFlag(gotTrailer450);
		BF_INVENTORY.setObjectScene(BF_LAST_INVENT, 1);
		_sceneMode = 4508;
		setAction(&_sequenceManager, this, 4508, &BF_GLOBALS._player, &_object4, &_door, NULL);
		break;
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene450::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_exit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_SW);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
