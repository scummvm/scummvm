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
			SceneItem::display(410, 12);
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

} // End of namespace BlueForce

} // End of namespace TsAGE
