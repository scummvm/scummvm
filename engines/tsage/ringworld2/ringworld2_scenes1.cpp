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
#include "tsage/ringworld2/ringworld2_scenes1.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 1010 - Cutscene: A pixel lost in space!
 *
 *--------------------------------------------------------------------------*/
void Scene1010::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(1010);

	R2_GLOBALS._v58CE2 = 0;
	setZoomPercents(100, 1, 160, 100);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setObjectWrapper(NULL);
	R2_GLOBALS._player.setPosition(Common::Point(30, 264));
	R2_GLOBALS._player.changeZoom(-1);
	R2_GLOBALS._player.disableControl();

	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);

	if (R2_GLOBALS.getFlag(57))
		_sceneMode = 1;
	else {
		R2_GLOBALS._sound1.play(89);
		_sceneMode = 0;
	}
}

void Scene1010::signal() {
	switch (_sceneMode) {
	case 1: {
		_sceneMode = 2;
		R2_GLOBALS._player.setup(1010, 2, 1);
		R2_GLOBALS._player.setPosition(Common::Point(297, 101));
		Common::Point pt(30, 264);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2:
		_sceneMode = 3;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		R2_GLOBALS._player.hide();
		break;
	case 3:
		if (R2_GLOBALS.getFlag(57))
			R2_GLOBALS._sceneManager.changeScene(1500);
		else
			R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	default: {
		_sceneMode = 2;
		R2_GLOBALS._player.setup(1010, 1, 1);
		Common::Point pt(297, 101);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1020 -
 *
 *--------------------------------------------------------------------------*/
void Scene1020::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(1020);

	if (R2_GLOBALS._sceneManager._previousScene == 1010)
		g_globals->gfxManager()._bounds.moveTo(Common::Point(160, 0));

	R2_GLOBALS._v558B6.set(160, 0, 160, 161);
	R2_GLOBALS._v58CE2 = 0;
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._sceneManager._previousScene == 1010) {
		R2_GLOBALS._player.setPosition(Common::Point(500, 100));
		R2_GLOBALS._player.setup(1020, 1, 1);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(0, 100));
		R2_GLOBALS._player.setup(1020, 2, 1);
	}

	R2_GLOBALS._player.setObjectWrapper(NULL);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();
	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);

	if (R2_GLOBALS._sceneManager._previousScene == 1010)
		_sceneMode = 0;
	else 
		_sceneMode = 10;
}

void Scene1020::signal() {
	switch (_sceneMode) {
	case 0: {
		_sceneMode = 1;
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setPosition(Common::Point(347, 48));
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
		R2_GLOBALS._player.setZoom(0);
		Common::Point pt(392, 41);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 1:
		_sceneMode = 2;
		R2_GLOBALS._player.setZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		_sceneMode = 3;
		R2_GLOBALS._player._moveDiff = Common::Point(30, 15);
		Common::Point pt(-15, 149);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		_sceneMode = 4;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		R2_GLOBALS.setFlag(51);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 10: {
		_sceneMode = 11;
		R2_GLOBALS._player.setPosition(Common::Point(25, 133));
		R2_GLOBALS._player._moveDiff = Common::Point(30, 15);
		R2_GLOBALS._player.setZoom(100);
		Common::Point pt(355, 60);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 11:
		R2_GLOBALS._player.setPosition(Common::Point(355, 57));
		_sceneMode = 12;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 12: {
		R2_GLOBALS._player.setPosition(Common::Point(355, 60));
		_sceneMode = 13;
		R2_GLOBALS._player._moveDiff = Common::Point(3, 1);
		Common::Point pt(347, 48);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 13:
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		if (R2_GLOBALS._player._percent < 1)
			_sceneMode = 14;
		break;
	case 14:
		R2_GLOBALS._sceneManager.changeScene(1010);
		break;
	default:
		break;
	}
}

void Scene1020::dispatch() {
	if (_sceneMode == 1) {
		R2_GLOBALS._player.setZoom(R2_GLOBALS._player._percent + 1);
		if (R2_GLOBALS._player._percent > 10)
			R2_GLOBALS._player._moveDiff.x = 3;
		if (R2_GLOBALS._player._percent > 20)
			R2_GLOBALS._player._moveDiff.x = 4;
	}

	if ((_sceneMode == 13) && (R2_GLOBALS._player._percent != 0)) {
		R2_GLOBALS._player.setZoom(R2_GLOBALS._player._percent - 2);
		if (R2_GLOBALS._player._percent < 80)
			R2_GLOBALS._player._moveDiff.x = 2;
		if (R2_GLOBALS._player._percent < 70)
			R2_GLOBALS._player._moveDiff.x = 1;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1100 -
 *
 *--------------------------------------------------------------------------*/
Scene1100::Scene1100() {
	_field412 = 0;
	_field414 = 0;
}

void Scene1100::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
}

bool Scene1100::Actor16::startAction(CursorType action, Event &event) {
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS.getFlag(52)) {
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->_field412 = 327;
		else
			scene->_field412 = 328;
		scene->_sceneMode = 53;
		scene->setAction(&scene->_sequenceManager1, scene, 1122, &R2_GLOBALS._player, NULL);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 55;
		if (R2_GLOBALS._v565AE >= 3) {
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_stripManager.start3(329, scene, R2_GLOBALS._stripManager_lookupList);
			else
				scene->_stripManager.start3(330, scene, R2_GLOBALS._stripManager_lookupList);
		} else {
			++R2_GLOBALS._v565AE;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_stripManager.start3(304, scene, R2_GLOBALS._stripManager_lookupList);
			else
				scene->_stripManager.start3(308, scene, R2_GLOBALS._stripManager_lookupList);
		}
	}
	return true;
}

bool Scene1100::Actor17::startAction(CursorType action, Event &event) {
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_NEGATOR_GUN:
		if (_visage == 1105) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1114;
			scene->setAction(&scene->_sequenceManager1, scene, 1114, &R2_GLOBALS._player, &scene->_actor17, NULL);
			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
		break;
	case R2_SONIC_STUNNER:
	// No break on purpose
	case R2_44:
		if (_visage == 1105) {
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == 1) {
				scene->_sceneMode = 1112;
				scene->setAction(&scene->_sequenceManager1, scene, 1112, &R2_GLOBALS._player, &scene->_actor17, NULL);
			} else {
				scene->_sceneMode = 1115;
				scene->setAction(&scene->_sequenceManager1, scene, 1115, &R2_GLOBALS._player, &scene->_actor17, NULL);
			}
			return true;
		} else if (_strip == 2) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1113;
			if (R2_GLOBALS._player._characterIndex == 1) {
				scene->setAction(&scene->_sequenceManager1, scene, 1113, &R2_GLOBALS._player, &scene->_actor17, NULL);
			} else {
				scene->setAction(&scene->_sequenceManager1, scene, 1118, &R2_GLOBALS._player, &scene->_actor17, NULL);
			}
			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

bool Scene1100::Actor18::startAction(CursorType action, Event &event) {
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_TALK) && (!R2_GLOBALS.getFlag(54)) && (R2_GLOBALS.getFlag(52))) {
		scene->_field412 = 0;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 53;
		scene->setAction(&scene->_sequenceManager1, scene, 1122, &R2_GLOBALS._player, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene1100::postInit(SceneObjectList *OwnerList) {
	if ((R2_GLOBALS._sceneManager._previousScene == 300) || (R2_GLOBALS._sceneManager._previousScene == 1100))
		loadScene(1150);
	else
		loadScene(1100);

	if ((R2_GLOBALS._sceneManager._previousScene == 1000) && (!R2_GLOBALS.getFlag(44))) {
		R2_GLOBALS._v58CE2 = 0;
		R2_GLOBALS._v5589E.left = 0;
		R2_GLOBALS._v5589E.right = 200;
	}

	if (R2_GLOBALS._player._characterScene[1] == 1100)
		R2_GLOBALS._sceneManager._previousScene = 1100;

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._v58CE2 = 0;
		R2_GLOBALS._v5589E.left = 0;
		R2_GLOBALS._v5589E.right = 200;
	}

	SceneExt::postInit();
		
	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1000;

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_chiefSpeaker);

	scalePalette(65, 65, 65);

	_actor2.postInit();
	_actor2.setup(1100, 1, 1);
	_actor2.fixPriority(10);

	R2_GLOBALS._scrollFollower = NULL;

	_item3.setDetails(Rect(56, 47, 68, 83), 1100, 7, -1, -1, 1, NULL);
	_item4.setDetails(Rect(167, 132, 183, 167), 1100, 7, -1, -1, 1, NULL);
	_item5.setDetails(Rect(26, 112, 87, 145), 1100, 13, -1, -1, 1, NULL);
	_item7.setDetails(Rect(4, 70, 79, 167), 1100, 16, -1, -1, 1, NULL);

	R2_GLOBALS._sound1.stop();

	if (R2_GLOBALS._sceneManager._previousScene == 300) {
		if (R2_GLOBALS._player._characterIndex == 3)
			R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player._characterScene[1] = 1100;
		R2_GLOBALS._player._characterScene[2] = 1100;
		_actor2.setPosition(Common::Point(150, 30));
		R2_GLOBALS._sound1.play(93);
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.hide();
		R2_GLOBALS._player.disableControl();

		_actor16.postInit();
		_actor16.hide();
		if (R2_GLOBALS._player._characterIndex == 1)
			_actor16.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		else
			_actor16.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);

		_actor18.postInit();
		_actor18.setup(1113, 3, 1);
		_actor18.setPosition(Common::Point(181, 125));
		_actor18.fixPriority(110);

		if (R2_GLOBALS.getFlag(54))
			_actor18.setDetails(1100, 4, -1, -1, 1, (SceneItem *) NULL);
		else
			_actor18.setDetails(1100, 3, -1, -1, 1, (SceneItem *) NULL);
		
		_actor17.postInit();
		_actor17.setup(1105, 3, 1);
		_actor17.setPosition(Common::Point(312, 165));
		_actor17._numFrames = 5;
		_actor17.setDetails(1100, 22, 23, 24, 1, (SceneItem *) NULL);

		_actor1.postInit();
		_actor1.setup(1512, 1, 1);
		_actor1.setPosition(Common::Point(187, -25));
		_actor1.fixPriority(48);
		_actor1._moveDiff.y = 1;
		_actor1.setDetails(1100, 37, -1, -1, 1, (SceneItem *) NULL);

		_sceneMode = 20;

		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
	} else if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		_actor2.setPosition(Common::Point(50, 30));
		_field414 = 0;
		_palette1.loadPalette(1101);
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player._effect = 5;
		R2_GLOBALS._player._field9C = _field312;
		R2_GLOBALS._player.setup(1102, 3, 2);
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setPosition(Common::Point(111,-20));
		R2_GLOBALS._player.fixPriority(150);
		R2_GLOBALS._player._moveRate = 30;
		R2_GLOBALS._player._moveDiff = Common::Point(16, 2);

		_object1.setup2(1104, 2, 1, 175, 125, 102, 1);
		_object2.setup2(1102, 5, 1, 216, 167, 1, 0);

		_actor12.postInit();
		_actor12.setup(1113, 2, 1);
		_actor12.setPosition(Common::Point(67, 151));
		_actor12.fixPriority(255);

		_actor3.postInit();
		_actor3.setup(1102, 6, 1);
		_actor3._moveRate = 30;
		_actor3._moveDiff.x = 2;

		_actor4.postInit();
		_actor4.setup(1102, 6, 2);
		_actor4._moveRate = 30;
		_actor4._moveDiff.x = 2;
		_actor4._effect = 5;
		_actor4._field9C = _field312;

		R2_GLOBALS._sound1.play(86);
		
		_sceneMode = 0;

		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
	} else {
		_actor2.setPosition(Common::Point(180, 30));
		if (R2_GLOBALS.getFlag(52))
			R2_GLOBALS._sound1.play(98);
		else
			R2_GLOBALS._sound1.play(95);

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		
		_actor16.postInit();

		if (R2_GLOBALS.getFlag(52)) {
			if (R2_GLOBALS._player._characterIndex == 1) {
				R2_GLOBALS._player.setup(19, 7, 1);
				_actor16.setup(29, 6, 1);
			} else {
				R2_GLOBALS._player.setup(29, 7, 1);
				_actor16.setup(19, 6, 1);
			}
			R2_GLOBALS._player.setPosition(Common::Point(140, 124));
			_actor16.setPosition(Common::Point(237, 134));
			R2_GLOBALS._player.enableControl();
		} else {
			if (R2_GLOBALS._player._characterIndex == 1) {
				R2_GLOBALS._player.setup(1107, 2, 1);
				_actor16.setup(1107, 4, 1);
				R2_GLOBALS._player.setPosition(Common::Point(247, 169));
				_actor16.setPosition(Common::Point(213, 169));
			} else {
				R2_GLOBALS._player.setup(1107, 4, 1);
				_actor16.setup(1107, 2, 1);
				R2_GLOBALS._player.setPosition(Common::Point(213, 169));
				_actor16.setPosition(Common::Point(247, 169));
			}
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
		}
		
		if (R2_GLOBALS._player._characterIndex == 1)
			_actor16.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		else
			_actor16.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);

		_actor18.postInit();
		_actor18.setup(1113, 3, 1);
		_actor18.setPosition(Common::Point(181, 125));
		_actor18.fixPriority(110);

		if (R2_GLOBALS.getFlag(54))
			_actor18.setDetails(1100, 4, -1, -1, 1, (SceneItem *) NULL);
		else
			_actor18.setDetails(1100, 3, -1, -1, 1, (SceneItem *) NULL);

		if (!R2_GLOBALS.getFlag(52)) {
			_actor17.postInit();
			if (R2_GLOBALS.getFlag(53))
				_actor17.setup(1106, 2, 4);
			else
				_actor17.setup(1105, 4, 4);

			_actor17.setPosition(Common::Point(17, 54));
			_actor17._numFrames = 5;

			if (R2_GLOBALS.getFlag(53))
				_actor17.setDetails(1100, 28, -1, -1, 1, (SceneItem *) NULL);
			else
				_actor17.setDetails(1100, 22, 23, 24, 1, (SceneItem *) NULL);

			_actor17.fixPriority(200);
		}
		_actor1.postInit();
		_actor1.setup(1512, 1, 1);
		_actor1.setPosition(Common::Point(187, 45));
		_actor1.fixPriority(48);
		_actor1._moveDiff.y = 1;
		_actor1.setDetails(1100, 37, -1, -1, 1, (SceneItem *) NULL);
	}
	_item6.setDetails(Rect(123, 69, 222, 105), 1100, 13, -1, -1, 1, NULL);
	_item2.setDetails(Rect(0, 0, 480, 46), 1100, 0, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 480, 200), 1100, 40, 41, 42, 1, NULL);
}	

void Scene1100::remove() {
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	if (_sceneMode > 20)
		R2_GLOBALS._sound1.fadeOut2(NULL);
	g_globals->gfxManager()._bounds.moveTo(Common::Point(0, 0));
	R2_GLOBALS._v58CE2 = 1;
	SceneExt::remove();
}

void Scene1100::signal() {
	switch (_sceneMode++) {
	case 0:
		_actor3.setPosition(Common::Point(350, 20));
		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 1:{
		Common::Point pt(-150, 20);
		NpcMover *mover = new NpcMover();
		_actor3.addMover(mover, &pt, this);
		_actor4.setPosition(Common::Point(350, 55));

		Common::Point pt2(-150, 55);
		NpcMover *mover2 = new NpcMover();
		_actor4.addMover(mover2, &pt2, NULL);
		}
		break;
	case 2:
		_actor3.remove();
		_actor4.remove();
		_actor5.postInit();
		_actor6.postInit();
		_actor7.postInit();
		_actor8.postInit();
		_actor9.postInit();
		_actor10.postInit();
		setAction(&_sequenceManager1, this, 1102, &_actor5, &_actor6, &_actor7, &_actor8, &_actor9, &_actor10, NULL);
		break;
	case 3: {
		R2_GLOBALS._sound2.play(84);
		R2_GLOBALS._player.setPosition(Common::Point(-50, 126));
		Common::Point pt(350, 226);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 4:
		_actor18.postInit();
		_actor18.show();
		setAction(&_sequenceManager1, this, 1101, &_actor18, &_actor10, NULL);
		break;
	case 5:
		_actor13.postInit();
		_actor13._effect = 6;
		_actor13.setup(1103, 3, 1);
		_actor13._moveRate = 30;

		_actor14.postInit();
		_actor14._effect = 6;
		_actor14.setup(1103, 4, 1);
		_actor4._moveRate = 25;

		_actor13.setAction(&_sequenceManager2, this, 1109, &_actor13, &_actor14, NULL);
		break;
	case 6: {
		_actor13.remove();
		_actor14.remove();
		R2_GLOBALS._player.setPosition(Common::Point(-50, 136));
		R2_GLOBALS._sound2.play(84);
		Common::Point pt(350, 236);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 7:
		setAction(&_sequenceManager1, this, 1103, &_actor18, &_actor10);
		break;
	case 8:
		R2_GLOBALS._player._effect = 0;
		_actor11.postInit();
		setAction(&_sequenceManager1, this, 1105, &R2_GLOBALS._player, &_actor10, &_actor11, &_actor18, NULL);
		break;
	case 9:
		_object1.proc27();

		_actor15.postInit();
		_actor15.setup(1103, 2, 1);
		_actor15._moveRate = 30;
		_actor15.setAction(&_sequenceManager3, this, 1107, &_actor15, NULL);
		break;
	case 10:
		_actor13.postInit();
		_actor13.setup(1103, 1, 1);
		_actor13._moveRate = 15;
		_actor13.setAction(&_sequenceManager2, this, 1108, &_actor13, NULL);
		break;
	case 11: {
		setAction(&_sequenceManager1, this, 1116, &_actor11, &_actor10, &_actor12, NULL);
		R2_GLOBALS._player._effect = 5;
		R2_GLOBALS._player.setup(1102, 3, 2);
		R2_GLOBALS._player.setPosition(Common::Point(-50, 131));
		R2_GLOBALS._sound2.play(84);
		Common::Point pt(350, 231);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 12:
	// Really nothing
		break;
	case 13:
		_actor17.postInit();
		R2_GLOBALS._scrollFollower = &_actor17;

		_actor11.setup(1100, 2, 1);
		_actor11.setPosition(Common::Point(408, 121));

		_actor10.setup(1100, 3, 5);
		_actor10.setPosition(Common::Point(409, 121));

		setAction(&_sequenceManager1, this, 1104, &_actor17, NULL);
		break;
	case 14:
		setAction(&_sequenceManager1, this, 1100, &_actor11, &_actor10, NULL);
		break;
	case 15:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 20: {
		Common::Point pt(187, -13);
		NpcMover *mover = new NpcMover();
		_actor1.addMover(mover, &pt, this);
		}
		break;
	case 21: {
		R2_GLOBALS._sound2.play(92);
		_actor17.animate(ANIM_MODE_5, NULL);
		Common::Point pt(187, 45);
		NpcMover *mover = new NpcMover();
		_actor1.addMover(mover, &pt, this);
		}
		break;
	case 22:
		setAction(&_sequenceManager1, this, 1110, &_actor16, &R2_GLOBALS._player, NULL);
		break;
	case 23:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(312, this);
		R2_GLOBALS._player.setAction(&_sequenceManager1, this, 1119, &R2_GLOBALS._player, NULL);
		break;
	case 24:
		if (!_stripManager._endHandler)
			R2_GLOBALS._player.disableControl();
		break;
	case 25:
		R2_GLOBALS._player.disableControl();
		_stripManager._lookupList[9] = 1;
		_stripManager._lookupList[10] = 1;
		_stripManager._lookupList[11] = 1;
		R2_GLOBALS._sound1.play(95);
		setAction(&_sequenceManager1, this, 1111, &_actor17, &R2_GLOBALS._player, &_actor16, NULL);
		break;
	case 26:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(302, this);
		break;		
	case 27:
		R2_GLOBALS._player.disableControl();
		setAction(&_sequenceManager1, this, 1120, &_actor16, &R2_GLOBALS._player, NULL);
		break;
	case 28:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(303, this);
		break;
	case 51:
		R2_GLOBALS.setFlag(53);
		_actor17.setDetails(1100, 28, -1, -1, 3, (SceneItem *) NULL);
	// No break on purpose
	case 50:
	// No break on purpose
	case 29:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	case 52:
		R2_GLOBALS._sound1.play(98);
		R2_GLOBALS.setFlag(52);
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1116;
		if (R2_GLOBALS._player._characterIndex == 1) {
			setAction(&_sequenceManager1, this, 1116, &R2_GLOBALS._player, NULL);
			_actor16.setAction(&_sequenceManager2, NULL, 1123, &_actor16, NULL);
		} else {
			setAction(&_sequenceManager1, this, 1124, &R2_GLOBALS._player, NULL);
			_actor16.setAction(&_sequenceManager2, NULL, 1117, &_actor16, NULL);
		}
		break;
	case 53:
		_sceneMode = 54;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (_field412 == 0) {
			R2_GLOBALS.setFlag(55);
			if (R2_GLOBALS.getFlag(55)) {
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(318, this);
				else
					_stripManager.start(323, this);
			} else {
				// This part is totally useless as flag 55 has been set right before the check
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(317, this);
				else
					_stripManager.start(322, this);
			}
		} else {
			_stripManager.start3(_field412, this, _stripManager._lookupList);
		}
		break;
	case 54:
		if (_stripManager._field2E8 == 1) {
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1125;
			setAction(&_sequenceManager1, this, 1125, &R2_GLOBALS._player, &_actor16, NULL);
		} else 
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 55:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 99:
		R2_GLOBALS._player._characterScene[1] = 300;
		R2_GLOBALS._player._characterScene[2] = 300;
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 1112:
		_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(313, this, _stripManager._lookupList);
		break;
	case 1113:
		_sceneMode = 52;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS._sound1.play(96);
		_stripManager.start3(316, this, _stripManager._lookupList);
		break;
	case 1114:
		_sceneMode = 51;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(315, this, _stripManager._lookupList);
		break;
	case 1115:
		_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(314, this, _stripManager._lookupList);
		break;
	case 1116:
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		_stripManager._lookupList[9] = 1;
		_stripManager._lookupList[10] = 1;
		_stripManager._lookupList[11] = 1;
		break;
	case 1125: {
		_sceneMode = 99;
		R2_GLOBALS._sound2.play(100);
		R2_GLOBALS._sound1.play(101);
		Common::Point pt(187, -13);
		NpcMover *mover = new NpcMover();
		_actor1.addMover(mover, &pt, this);
		}
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene1100::dispatch() {
	if ((g_globals->_sceneObjects->contains(&_actor10)) && (_actor10._visage == 1102) && (_actor10._strip == 4) && (_actor10._frame == 1) && (_actor10._flags & OBJFLAG_HIDING)) {
		if (_field414 == 1) {
			_field414 = 2;
			R2_GLOBALS._scenePalette.refresh();
		}
	} else {
		if (_field414 == 2)
			R2_GLOBALS._scenePalette.refresh();
		_field414 = 1;
	}
	
	Scene::dispatch();
	
	if (R2_GLOBALS._player._bounds.contains(_actor13._position))
		_actor13._shade = 3;
	else 
		_actor13._shade = 0;

	if (R2_GLOBALS._player._bounds.contains(_actor14._position))
		_actor14._shade = 3;
	else 
		_actor14._shade = 0;

	if (R2_GLOBALS._player._bounds.contains(_actor15._position))
		_actor15._shade = 3;
	else 
		_actor15._shade = 0;
}

void Scene1100::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::saveCharacter(characterIndex);
}

/*--------------------------------------------------------------------------
 * Scene 1200 -
 *
 *--------------------------------------------------------------------------*/
Scene1200::Scene1200() {
	_field412 = 0;
	_field414 = 0;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;
	_field41C = 1; //CHECKME: Only if fixup_flag == 6??
}

void Scene1200::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
	s.syncAsSint16LE(_field41C);
}

Scene1200::Area1::Area1() {
	_field20 = 0;
}

void Scene1200::Area1::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsByte(_field20);
}

void Scene1200::Area1::Actor3::init(int state) {
	_state = state;

	SceneActor::postInit();
	setup(1003, 1, 1);
	fixPriority(255);

	switch (_state) {
	case 1:
		switch (R2_GLOBALS._v56AA6) {
		case 1:
			setFrame2(2);
			setPosition(Common::Point(129, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(135, 95));
			break;
		default:
			break;
		}
	case 2:
		switch (R2_GLOBALS._v56AA7) {
		case 1:
			setFrame2(2);
			setPosition(Common::Point(152, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(158, 122));
			break;
		case 3:
			setFrame2(3);
			setPosition(Common::Point(135, 122));
			break;
		default:
			break;
		}
	case 3:
		switch (R2_GLOBALS._v56AA8) {
		case 1:
			setFrame2(3);
			setPosition(Common::Point(158, 95));
			break;
		case 2:
			setFrame2(2);
			setPosition(Common::Point(175, 101));
			break;
		default:
			break;
		}
	default:
		break;
	}

	setDetails(1200, 12, -1, -1, 2, (SceneItem *) NULL);
}

bool Scene1200::Area1::Actor3::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	
	R2_GLOBALS._sound2.play(260);
	switch (_state) {
	case 1:
		if (R2_GLOBALS._v56AA6 == 1) {
			R2_GLOBALS._v56AA6 = 2;
			setFrame2(3);
			setPosition(Common::Point(135, 95));
		} else {
			R2_GLOBALS._v56AA6 = 1;
			setFrame2(2);
			setPosition(Common::Point(129, 101));
		}
		break;
	case 2:
		++R2_GLOBALS._v56AA7;
		if (R2_GLOBALS._v56AA7 == 4)
			R2_GLOBALS._v56AA7 = 1;

		switch (R2_GLOBALS._v56AA7) {
		case 1:
			setFrame2(1);
			setPosition(Common::Point(152, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(158, 122));
			break;
		case 3:
			setFrame2(3);
			setPosition(Common::Point(135, 122));
			break;
		default:
			break;
		}
		break;
	case 3:
		if (R2_GLOBALS._v56AA8 == 1) {
			R2_GLOBALS._v56AA8 = 2;
			setFrame2(2);
			setPosition(Common::Point(175, 101));
		} else {
			R2_GLOBALS._v56AA8 = 1;
			setFrame2(3);
			setPosition(Common::Point(158, 95));
		}
		break;
	default:
		break;
	}
	
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;
	scene->_field418 = 0;
	
	if ((R2_GLOBALS._v56AA6 == 1) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 1))
		scene->_field418 = 1;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 1))
		scene->_field418 = 2;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 2))
		scene->_field418 = 3;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 3) && (R2_GLOBALS._v56AA8 == 1))
		scene->_field418 = 4;
	
	return true;
}

void Scene1200::Area1::postInit(SceneObjectList *OwnerList) {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;

	scene->_field41A = 1;
	R2_GLOBALS._events.setCursor(CURSOR_USE);
	proc12(1003, 1, 1, 100, 40);
	proc13(1200, 11, -1, -1);
	R2_GLOBALS._sound2.play(259);
	_actor3.init(1);
	_actor4.init(2);
	_actor5.init(3);

	R2_GLOBALS._player._canWalk = false;
}

void Scene1200::Area1::remove() {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;
	
	scene->_field41A = 0;
	warning("Unexpected _sceneAreas.remove() call");
//	scene->_sceneAreas.remove(&_actor3);
//	scene->_sceneAreas.remove(&_actor4);
//	scene->_sceneAreas.remove(&_actor5);
	_actor3.remove();
	_actor4.remove();
	_actor5.remove();
	
	// sub201EA
	R2_GLOBALS._sceneItems.remove((SceneItem *)this);
	_actor2.remove();
	SceneArea::remove();
	R2_GLOBALS._insetUp--;
	//
	
	R2_GLOBALS._player._canWalk = true;
}

void Scene1200::Area1::process(Event &event) {
	if (_field20 != R2_GLOBALS._insetUp)
		return;

	CursorType cursor = R2_GLOBALS._events.getCursor();

	if (_actor2._bounds.contains(event.mousePos.x + g_globals->gfxManager()._bounds.left , event.mousePos.y)) {
		if (cursor == _cursorNum)
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(_savedCursorNum); //, _cursorState);
	} else if (event.mousePos.y < 168) {
		if (cursor != _cursorNum) {
			_savedCursorNum = cursor;
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(CURSOR_INVALID);
		}
		if (event.eventType == EVENT_BUTTON_DOWN) {
			event.handled = true;
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(_savedCursorNum); //, _cursorState);
			remove();
		}
	}
}

void Scene1200::Area1::proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;

	_actor2.postInit();
	_actor2.setup(visage, stripFrameNum, frameNum);
	_actor2.setPosition(Common::Point(posX, posY));
	_actor2.fixPriority(250);
	_cursorNum = CURSOR_INVALID;
	scene->_sceneAreas.push_front(this);
	++R2_GLOBALS._insetUp;
	_field20 = R2_GLOBALS._insetUp;
}

void Scene1200::Area1::proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	_actor2.setDetails(resNum, lookLineNum, talkLineNum, useLineNum, 2, (SceneItem *) NULL);
}

Scene1200::Object1::Object1() {
	_field16 = _field26 = _field28 = _field2A = _field2C = _field2E = _field30 = 0;
}

void Scene1200::Object1::synchronize(Serializer &s) {
	SavedObject::synchronize(s);

	s.syncAsSint16LE(_field16);
	s.syncAsSint16LE(_field26);
	s.syncAsSint16LE(_field28);
	s.syncAsSint16LE(_field2A);
	s.syncAsSint16LE(_field2C);
	s.syncAsSint16LE(_field2E);
	s.syncAsSint16LE(_field30);
}

int Scene1200::Object1::sub51AF8(Common::Point pt) {
	if (!_rect1.contains(pt))
		return -1;
	
	int tmp1 = (pt.x - _rect1.left + _field2E) / _field2A;
	int tmp2 = (pt.y - _rect1.top + _field30) / _field2C;

	if ((tmp1 >= 0) && (tmp2 >= 0) && (_field26 > tmp1) && (_field28 > tmp2))
		return _field16 + (((_field26 * tmp2) + tmp1)* 2);

	return -1;
}

bool Scene1200::Object1::sub51AFD(Common::Point pt) {
	int retval = false;

	_field2E = pt.x;
	_field30 = pt.y;

	if (_field2E < _rect2.top) {
		_field2E = _rect2.top;
		retval = true;
	}

	if (_field30 < _rect2.left) {
		_field30 = _rect2.left;
		retval = true;
	}

	if (_field2E + _rect1.width() > _rect2.right) {
		_field2E = _rect2.right - _rect1.width();
		retval = true;
	}

	if (_field30 + _rect1.height() > _rect2.bottom) {
		_field30 = _rect2.bottom - _rect1.height();
		retval = true;
	}

	return retval;
}

void Scene1200::Object1::sub9EDE8(Rect rect) {
	_rect1 = rect;
	warning("FIXME: Scene1200::Object1::sub9EDE8()");
//	_rect1.clip(g_globals->gfxManager()._bounds);
}

int Scene1200::Object1::sub9EE22(int &arg1, int &arg2) {
	arg1 /= _field2A;
	arg2 /= _field2C;
	
	if ((arg1 >= 0) && (arg2 >= 0) && (_field26 > arg1) && (_field28 > arg2)) {
		return (((_field26 * arg2) + arg1) * 2) + _field16;
	}

	return -1;
}

void Scene1200::sub9DAD6(int indx) {
	_object1.sub9EE22(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);
	
	switch (indx) {
	case 0:
		if ( ((_object1.sub51AF8(Common::Point(200, 50)) > 36) || (_object1.sub51AF8(Common::Point(200, 88)) > 36)) 
			&& ( ((R2_GLOBALS._v56AA2 == 3) && (R2_GLOBALS._v56AA4 == 33) && (_field418 != 4)) 
				|| ((R2_GLOBALS._v56AA2 == 13) && (R2_GLOBALS._v56AA4 == 21) && (_field418 != 2)) 
				|| ((R2_GLOBALS._v56AA2 == 29) && (R2_GLOBALS._v56AA4 == 17) && (_field418 != 1)) 
				|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 41)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1200;
			setAction(&_sequenceManager, this, 1200, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(200, 69)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 15;
				else
					_sceneMode = 10;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 76;
				else
					_sceneMode = 75;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 101;
				else
					_sceneMode = 100;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 111;
				else
					_sceneMode = 110;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 1;
			signal();
		}
		break;
	case 1:
		if ( ((_object1.sub51AF8(Common::Point(120, 50)) > 36) || (_object1.sub51AF8(Common::Point(120, 88)) > 36)) 
			&& ( ((R2_GLOBALS._v56AA2 == 7) && (R2_GLOBALS._v56AA4 == 33) && (_field418 != 4)) 
				|| ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 21) && (_field418 != 2)) 
				|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 17) && (_field418 != 1)) 
				|| ((R2_GLOBALS._v56AA2 == 5) && (R2_GLOBALS._v56AA4 == 5)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1201;
			setAction(&_sequenceManager, this, 1201, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(120, 69)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 56;
				else
					_sceneMode = 55;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 25;
				else
					_sceneMode = 20;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 91;
				else
					_sceneMode = 90;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 121;
				else
					_sceneMode = 120;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 2;
			signal();
		}
		break;
	case 2:
		if ( ((_object1.sub51AF8(Common::Point(140, 110)) > 36) || (_object1.sub51AF8(Common::Point(178, 110)) > 36)) 
			&& ( ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 5) && (_field418 != 3)) 
				|| ((R2_GLOBALS._v56AA2 == 41) && (R2_GLOBALS._v56AA4 == 21)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1203;
			setAction(&_sequenceManager, this, 1203, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(160, 110)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 51;
				else
					_sceneMode = 50;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 81;
				else
					_sceneMode = 80;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 35;
				else
					_sceneMode = 30;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 116;
				else
					_sceneMode = 115;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 3;
			signal();
		}
		break;
	case 3:
		if ( ((_object1.sub51AF8(Common::Point(140, 30)) > 36) || (_object1.sub51AF8(Common::Point(178, 30)) > 36)) 
			&& ( ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 9) && (_field418 != 3)) 
				|| ((R2_GLOBALS._v56AA2 == 35) && (R2_GLOBALS._v56AA4 == 17)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1202;
			setAction(&_sequenceManager, this, 1202, &_actor1, NULL);
		} else if (_object1.sub51AF8(Common::Point(160, 30)) == 36) {
			switch (_field412 - 1) {
			case 0:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 61;
				else
					_sceneMode = 60;
				break;
			case 1:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 71;
				else
					_sceneMode = 70;
				break;
			case 2:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 96;
				else
					_sceneMode = 95;
				break;
			case 3:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 45;
				else
					_sceneMode = 40;
				break;
			default:
				_sceneMode = 1;
				R2_GLOBALS._player.setup(3156, 4, 6);
				break;
			}
			R2_GLOBALS._player.disableControl();
			_field412 = 4;
			signal();
		}
		break;
	default:
		break;
	}
}
void Scene1200::postInit(SceneObjectList *OwnerList) {
	Rect tmpRect;

	loadScene(1200);
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene < 3200)
		R2_GLOBALS._sound1.play(257);

	_field412 = 1;
	_field414 = 0;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;
	_field41C = 0;

	if ((R2_GLOBALS._v56AA6 == 1) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 1))
		_field418 = 1;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 1))
		_field418 = 2;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 1) && (R2_GLOBALS._v56AA8 == 2))
		_field418 = 3;
	else if ((R2_GLOBALS._v56AA6 == 2) && (R2_GLOBALS._v56AA7 == 3) && (R2_GLOBALS._v56AA8 == 1))
		_field418 = 4;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player.setup(3156, 1, 6);
	R2_GLOBALS._player.setPosition(Common::Point(160, 70));
	R2_GLOBALS._player._numFrames = 10;
	R2_GLOBALS._player._oldCharacterScene[3] = 1200;

	_actor1.postInit();
	_actor1.hide();

	tmpRect.set(110, 20, 210, 120);
	_object1.sub9EDE8(tmpRect);

	warning("_object1.sub51AE9(1);");
	_object1.sub51AFD(Common::Point(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4));
	warning("int unk = set_pane_p(_paneNumber);");
	warning("_object1.sub51B02();");
	warning("set_pane_p(unk);");

	R2_GLOBALS._player.enableControl();
	_item1.setDetails(Rect(0, 0, 320, 200), 1200, 0, 1, 2, 1, NULL);
}

void Scene1200::signal() {
	switch (_sceneMode++) {
	case 1:
	// No break on purpose
	case 1200:
	// No break on purpose
	case 1201:
	// No break on purpose
	case 1202:
	// No break on purpose
	case 1203:
		R2_GLOBALS._player.enableControl();
		warning("_eventManager.waitEvent()");
		_sceneMode = 2;
		break;
	case 10:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(1);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 11:
	// No break on purpose
	case 21:
	// No break on purpose
	case 31:
	// No break on purpose
	case 41:
		_field416 = 0;
		break;
	case 12:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 1, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 13:
	// No break on purpose
	case 16:
	// No break on purpose
	case 23:
	// No break on purpose
	case 26:
	// No break on purpose
	case 33:
	// No break on purpose
	case 36:
	// No break on purpose
	case 43:
	// No break on purpose
	case 46:
		R2_GLOBALS._player.setFrame(4);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 15:
	// No break on purpose
	case 25:
	// No break on purpose
	case 35:
	// No break on purpose
	case 45:
		_field414 = 20;
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 20:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 22:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 2, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 30:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 32:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 3, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 40:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(4);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 42:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 4, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 50:
	// No break on purpose
	case 55:
	// No break on purpose
	case 60:
		R2_GLOBALS._player.setup(3156, 5, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 51:
	// No break on purpose
	case 56:
	// No break on purpose
	case 117:
		R2_GLOBALS._player.setup(3157, 1, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 52:
	// No break on purpose
	case 82:
	// No break on purpose
	case 118:
		R2_GLOBALS._player.setup(3156, 3, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 57:
	// No break on purpose
	case 91:
	// No break on purpose
	case 96:
		R2_GLOBALS._player.setup(3157, 2, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 58:
	// No break on purpose
	case 92:
	// No break on purpose
	case 122:
		R2_GLOBALS._player.setup(3156, 2, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 61:
		R2_GLOBALS._player.setup(3157, 4, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 62:
	// No break on purpose
	case 72:
	// No break on purpose
	case 98:
		R2_GLOBALS._player.setup(3156, 4, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 70:
	// No break on purpose
	case 75:
	// No break on purpose
	case 80:
		R2_GLOBALS._player.setup(3156, 6, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 71:
	// No break on purpose
	case 76:
	// No break on purpose
	case 97:
		R2_GLOBALS._player.setup(3157, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 77:
	// No break on purpose
	case 111:
	// No break on purpose
	case 116:
		R2_GLOBALS._player.setup(3157, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 78:
	// No break on purpose
	case 102:
	// No break on purpose
	case 112:
		R2_GLOBALS._player.setup(3156, 1, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 81:
		R2_GLOBALS._player.setup(3157, 2, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 90:
	// No break on purpose
	case 95:
	// No break on purpose
	case 100:
		R2_GLOBALS._player.setup(3156, 7, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 101:
		R2_GLOBALS._player.setup(3157, 1, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 110:
	// No break on purpose
	case 115:
	// No break on purpose
	case 120:
		R2_GLOBALS._player.setup(3156, 8, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 121:
		R2_GLOBALS._player.setup(3157, 3, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	default:
		warning("_eventManager.waitEvent()");
		_sceneMode = 2;
		break;
	}
}

void Scene1200::process(Event &event) {
	if (_field414 != 0)
		return;
	
	Scene::process(event);
	
	if (!R2_GLOBALS._player._canWalk)
		return;

	if (event.eventType == EVENT_BUTTON_DOWN) {
		_object1.sub9EE22(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);
		int unk = _object1.sub51AF8(event.mousePos);
		switch (R2_GLOBALS._events.getCursor()) {
		case CURSOR_ARROW:
			event.handled = true;
			if ((event.mousePos.x > 179) && (event.mousePos.x < 210) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				sub9DAD6(1);

			if ((event.mousePos.x > 109) && (event.mousePos.x < 140) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				sub9DAD6(2);
				
			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 89) && (event.mousePos.y < 120))
				sub9DAD6(3);
				
			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 19) && (event.mousePos.y < 50))
				sub9DAD6(4);
			break;
		case CURSOR_USE:
			if (unk > 36) {
				if ( ((R2_GLOBALS._v56AA2 == 3)  && (R2_GLOBALS._v56AA4 == 33))
					|| ((R2_GLOBALS._v56AA2 == 7)  && (R2_GLOBALS._v56AA4 == 33))
					|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 41))
					|| ((R2_GLOBALS._v56AA2 == 5)  && (R2_GLOBALS._v56AA4 == 5))
					|| ((R2_GLOBALS._v56AA2 == 13) && (R2_GLOBALS._v56AA4 == 21))
					|| ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 21))
					|| ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 5))
					|| ((R2_GLOBALS._v56AA2 == 17) && (R2_GLOBALS._v56AA4 == 9))
					|| ((R2_GLOBALS._v56AA2 == 29) && (R2_GLOBALS._v56AA4 == 17))
					|| ((R2_GLOBALS._v56AA2 == 33) && (R2_GLOBALS._v56AA4 == 17))
					|| ((R2_GLOBALS._v56AA2 == 35) && (R2_GLOBALS._v56AA4 == 17))
					|| ((R2_GLOBALS._v56AA2 == 41) && (R2_GLOBALS._v56AA4 == 21)) ) {
					_area1.postInit();
					event.handled = true;
				}
			}

			if ((unk == 1) || (unk == 4) || (unk == 11) || (unk == 14)) {
				if ( ((R2_GLOBALS._v56AA2 == 3)  && (R2_GLOBALS._v56AA4 == 9))
				  || ((R2_GLOBALS._v56AA2 == 11)  && (R2_GLOBALS._v56AA4 == 27))
				  || ((R2_GLOBALS._v56AA2 == 17)  && (R2_GLOBALS._v56AA4 == 7))
				  || ((R2_GLOBALS._v56AA2 == 17)  && (R2_GLOBALS._v56AA4 == 27))
				  || ((R2_GLOBALS._v56AA2 == 17)  && (R2_GLOBALS._v56AA4 == 33))
				  || (R2_GLOBALS._v56AA2 == 33) ) {
					switch (R2_GLOBALS._v56AA2) {
					case 3:
						R2_GLOBALS._sceneManager.changeScene(3150);
						break;
					case 33:
						if (R2_GLOBALS._v56AA1 >= 4)
							R2_GLOBALS._sceneManager.changeScene(3250);
						else
							SceneItem::display(1200, 6, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
						break;
					default:
						SceneItem::display(1200, 5, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, -999);
						break;
					}
					event.handled = true;
				}
			}
			break;
		case CURSOR_LOOK:
			if ((unk == 1) || (unk == 4) || (unk == 11) || (unk == 14)) {
				event.handled = true;
				switch (R2_GLOBALS._v56AA2) {
				case 3:
					SceneItem::display(1200, 8, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
						break;
				case 9:
						R2_GLOBALS._sceneManager.changeScene(3240);
						break;
				case 11:
					if (R2_GLOBALS._v56AA4 == 27)
						R2_GLOBALS._sceneManager.changeScene(3210);
					else
						SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
					break;
				case 17:
					switch (R2_GLOBALS._v56AA4) {
					case 5:
						R2_GLOBALS._sceneManager.changeScene(3230);
						break;
					case 21:
						R2_GLOBALS._sceneManager.changeScene(3220);
						break;
					case 33:
						R2_GLOBALS._sceneManager.changeScene(3200);
						break;
					default:
						SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
						break;
					}
				case 33:
					R2_GLOBALS._sceneManager.changeScene(3245);
					break;
				default:
					SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
					break;
				}
			}
			if (unk > 36) {
				event.handled = true;
				SceneItem::display(1200, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
			}
			break;
		case CURSOR_TALK:
			event.handled = true;
			break;
		default:
			return;
		}
	} else if (event.eventType == EVENT_KEYPRESS) {
		if (_field414 == 0) {
			event.handled = false;
			return;
		}
		
		switch (event.kbd.keycode) {
		case Common::KEYCODE_1:
			warning("FIXME: keycode = 0x4800");
			sub9DAD6(4);
			break;
		case Common::KEYCODE_2:
			warning("FIXME: keycode = 0x4B00");
			sub9DAD6(2);
			break;
		case Common::KEYCODE_3:
			warning("FIXME: keycode = 0x4D00");
			sub9DAD6(1);
			break;
		case Common::KEYCODE_4:
			warning("FIXME: keycode = 0x5000");
			sub9DAD6(3);
			break;
		default:
			event.handled = false;
			return;
			break;
		}
	} else {
		return;
	}
}
	
void Scene1200::dispatch() {
	Rect tmpRect;
	Scene::dispatch();
	if (_field41C != 0) {
		_object1.sub51AFD(Common::Point(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4));
		warning("int unk = set_pane_p(_paneNumber);");
		warning("_object1.sub51B02();");
		warning("_gfxManager.sub294AC(unk);");
		warning("tmpRect.sub14DF3();");
		_field41C = 0;
	}
	
	if (_field414 != 0) {
		tmpRect.set(110, 20, 210, 120);
		_field414--;
		switch (_field412 - 1) {
		case 0:
			R2_GLOBALS._v56AA2 += 2;
			break;
		case 1:
			R2_GLOBALS._v56AA2 -= 2;
			break;
		case 2:
			R2_GLOBALS._v56AA4 += 2;
			break;
		case 3:
			R2_GLOBALS._v56AA4 -= 2;
			break;
		default:
			break;
		}
		_object1.sub51AFD(Common::Point(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4));
		warning("int unk = set_pane_p(_paneNumber);");
		warning("_object1.sub51B02();");
		warning("_gfxManager.sub294AC(unk);");
		warning("tmpRect.sub14DF3();");
		
		if (_field416 != 0) {
			switch(_field412 - 1) {
			case 0:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x - 2, R2_GLOBALS._player._position.y));
				break;
			case 1:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x + 2, R2_GLOBALS._player._position.y));
				break;
			case 2:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 2));
				break;
			case 3:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 2));
				break;
			default:
				break;
			}
		}
		if (_field414 == 0) {
			if (_field416 == 0)
				R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			signal();
		}
	}
}

void Scene1200::saveCharacter(int characterIndex) {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::saveCharacter(characterIndex);
}

/*--------------------------------------------------------------------------
 * Scene 1500 - Cutscene: Ship landing
 *
 *--------------------------------------------------------------------------*/
void Scene1500::postInit(SceneObjectList *OwnerList) {
	loadScene(1500);
	R2_GLOBALS._v58CE2 = 0;
	R2_GLOBALS._v5589E.top = 0;
	R2_GLOBALS._v5589E.bottom = 200;
	setZoomPercents(170, 13, 240, 100);
	SceneExt::postInit();
	scalePalette(65, 65, 65);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_actor2.postInit();
	_actor2.setup(1401, 1, 1);
	_actor2._effect = 5;
	_actor2.fixPriority(10);
	_actor2._field9C = _field312;

	_actor1.postInit();
	_actor1.setup(1400, 1, 1);
	_actor1._moveDiff = Common::Point(1, 1);
	_actor1._linkedActor = &_actor2;

	if (R2_GLOBALS._sceneManager._previousScene != 1010) {
		_actor4.postInit();
		_actor4.setup(1401, 2, 1);
		_actor4._effect = 5;
		_actor4.fixPriority(10);
		_actor4._field9C = _field312;

		_actor3.postInit();
		_actor3._moveRate = 30;
		_actor3._moveDiff = Common::Point(1, 1);
		_actor3._linkedActor = &_actor4;
	}

	if (R2_GLOBALS._sceneManager._previousScene == 300) {
		_actor1.setPosition(Common::Point(189, 139), 5);

		_actor3.setup(1400, 1, 2);
		_actor3.setPosition(Common::Point(148, 108), 0);

		_sceneMode = 20;
		R2_GLOBALS._sound1.play(110);
	} else if (R2_GLOBALS._sceneManager._previousScene == 1550) {
		_actor1.setPosition(Common::Point(189, 139), 5);
		
		_actor3.setup(1400, 2, 1);
		_actor3.changeZoom(-1);
		_actor3.setPosition(Common::Point(298, 258), 5);

		_sceneMode = 10;
		R2_GLOBALS._sound1.play(106);
	} else {
		_actor1.setPosition(Common::Point(289, 239), -30);
		_sceneMode = 0;
		R2_GLOBALS._sound1.play(102);
	}
	signal();
}

void Scene1500::remove() {
	R2_GLOBALS._v5589E.top = 3;
	R2_GLOBALS._v5589E.bottom = 168;
	R2_GLOBALS._v58CE2 = 1;

	SceneExt::remove();
}

void Scene1500::signal() {
	switch(_sceneMode++) {
	case 0:
		R2_GLOBALS.setFlag(25);
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
	// No break on purpose
	case 1:
		if (_actor1._yDiff < 50) {
			_actor1.setPosition(Common::Point(289, 239), _actor1._yDiff + 1);
			_sceneMode = 1;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 2: {
		Common::Point pt(189, 139);
		NpcMover *mover = new NpcMover();
		_actor1.addMover(mover, &pt, this);
		}
		break;
	case 3:
		if (_actor1._yDiff > 5) {
			_actor1.setPosition(Common::Point(189, 139), _actor1._yDiff - 1);
			_sceneMode = 3;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 13:
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	// No break on purpose
	case 4:
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 10:
	// No break on purpose
	case 20:
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 11: {
		Common::Point pt(148, 108);
		NpcMover *mover = new NpcMover();
		_actor3.addMover(mover, &pt, this);
		}
		break;
	case 12:
		setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
		break;
	case 21: {
		Common::Point pt(-2, -42);
		NpcMover *mover = new NpcMover();
		_actor3.addMover(mover, &pt, NULL);
		signal();
		}
		break;
	case 22:
		if (_actor1._yDiff < 50) {
			_actor1.setPosition(Common::Point(189, 139), _actor1._yDiff + 1);
			_sceneMode = 22;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 23: {
		Common::Point pt(-13, -61);
		NpcMover *mover = new NpcMover();
		_actor1.addMover(mover, &pt, this);
		}
		break;
	case 24:
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	default:
		break;
	}
}

void Scene1500::dispatch() {
	if (_sceneMode > 10) {
		float yDiff = sqrt((float) (_actor3._position.x * _actor3._position.x) + (_actor3._position.y * _actor3._position.y));
		if (yDiff > 6)
			_actor3.setPosition(_actor3._position, (int) yDiff);
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1525 - Cutscene - Ship
 *
 *--------------------------------------------------------------------------*/
void Scene1525::postInit(SceneObjectList *OwnerList) {
	loadScene(1525);
	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._sceneManager._previousScene == 525)
		R2_GLOBALS._player.setup(1525, 1, 1);
	else
		R2_GLOBALS._player.setup(1525, 1, 16);
	R2_GLOBALS._player.setPosition(Common::Point(244, 148));
	R2_GLOBALS._player.disableControl();

	_sceneMode = 0;
	setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
}

void Scene1525::signal() {
	switch (_sceneMode++) {
	case 0:
		if (R2_GLOBALS._sceneManager._previousScene == 525)
			setAction(&_sequenceManager, this, 1525, &R2_GLOBALS._player, NULL);
		else
			setAction(&_sequenceManager, this, 1526, &R2_GLOBALS._player, NULL);
		break;
	case 1:
		setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		if (R2_GLOBALS._sceneManager._previousScene == 1530)
			R2_GLOBALS._sceneManager.changeScene(1550);
		else
			R2_GLOBALS._sceneManager.changeScene(1530);
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1530 - Cutscene - Elevator
 *
 *--------------------------------------------------------------------------*/
void Scene1530::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 1000)
		loadScene(1650);
	else if (R2_GLOBALS._sceneManager._previousScene == 1580)
		loadScene(1550);
	else
		loadScene(1530);

	R2_GLOBALS._v58CE2 = 0;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
		
	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.hide();
		R2_GLOBALS._player.disableControl();

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(538, this);
		R2_GLOBALS._sound1.play(114);

		_sceneMode = 3;
	} else if (R2_GLOBALS._sceneManager._previousScene == 1580) {
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setup(1516, 6, 1);
		R2_GLOBALS._player.setPosition(Common::Point(160, 125));
		R2_GLOBALS._player._moveRate = 30;
		R2_GLOBALS._player._moveDiff = Common::Point(4, 1);

		_actor2.postInit();
		_actor2.setup(1516, 7, 1);
		_actor2.setPosition(Common::Point(121, 41));
		_actor2.animate(ANIM_MODE_2, NULL);

		_actor3.postInit();
		_actor3.setup(1516, 8, 1);
		_actor3.setPosition(Common::Point(107, 116));
		_actor3.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._player.disableControl();
		Common::Point pt(480, 75);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		R2_GLOBALS._sound1.play(111);

		_sceneMode = 1;
	} else {
		_actor1.postInit();
		_actor1._effect = 1;

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.disableControl();

		setAction(&_sequenceManager, this, 1530, &R2_GLOBALS._player, &_actor1, NULL);

		_sceneMode = 2;
	}

}

void Scene1530::signal() {
	switch (_sceneMode - 1) {
	case 0:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 1:
		R2_GLOBALS._sceneManager.changeScene(1525);
		break;
	case 2:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 4;
		R2_GLOBALS._player.show();
		setAction(&_sequenceManager, this, 1650, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		R2_GLOBALS._sceneManager.changeScene(1700);
		break;
	default:
		break;
	}
}

void Scene1530::dispatch() {
	int16 x = R2_GLOBALS._player._position.x;
	int16 y = R2_GLOBALS._player._position.y;

	_actor2.setPosition(Common::Point(x - 39, y - 85));
	_actor3.setPosition(Common::Point(x - 53, y - 9));

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1550 -
 *
 *--------------------------------------------------------------------------*/
Scene1550::UnkObj15501::UnkObj15501() {
	_fieldA4 = _fieldA6 = 0;
}

void Scene1550::UnkObj15501::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
	s.syncAsSint16LE(_fieldA6);
}

bool Scene1550::UnkObj15501::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_visage == 1561) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 40;
			Common::Point pt(_position.x + 5, _position.y + 20);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case CURSOR_LOOK:
		if (_visage == 1561) {
			switch (_frame) {
			case 2:
				SceneItem::display(1550, 23, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			case 3:
				SceneItem::display(1550, 26, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			case 4:
				SceneItem::display(1550, 35, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			default:
				break;
			}
		} else {
			switch ((((_strip - 1) * 5) + _frame) % 3) {
			case 0:
				SceneItem::display(1550, 62, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			case 1:
				SceneItem::display(1550, 53, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			case 2:
				SceneItem::display(1550, 76, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
				break;
			default:
				break;
			}
		}
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

Scene1550::UnkObj15502::UnkObj15502() {
	_fieldA4 = 0;
}

void Scene1550::UnkObj15502::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
}

bool Scene1550::UnkObj15502::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_fieldA4 == 8) {
			scene->_field412 = 1;
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1576;
			else 
				scene->_sceneMode = 1584;
			// strcpy(scene->_arrUnkObj15502[7]._actorName, 'hatch');
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[7], NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case CURSOR_LOOK:
		if (_fieldA4 == 8)
			SceneItem::display(1550, 75, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		else if (_frame == 1)
			SceneItem::display(1550, 70, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		else
			SceneItem::display(1550, 71, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
		break;
	case R2_17:
		scene->_field412 = 1;
		if (_fieldA4 == 6) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1574;
			else
				scene->_sceneMode = 1582;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[5], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_18:
		scene->_field412 = 1;
		if (_fieldA4 == 3) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1571;
			else
				scene->_sceneMode = 1581;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[2], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_22:
		scene->_field412 = 1;
		if (_fieldA4 == 1) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1569;
			else
				scene->_sceneMode = 1579;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[0], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_23:
		scene->_field412 = 1;
		if (_fieldA4 == 4) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1572;
			scene->_actor1.postInit();
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[3], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_25:
		scene->_field412 = 1;
		if (_fieldA4 == 2) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1570;
			else
				scene->_sceneMode = 1580;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[1], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_27:
		scene->_field412 = 1;
		if (_fieldA4 == 5) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1573;
			scene->_actor1.postInit();
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[4], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_45:
		scene->_field412 = 1;
		if (_fieldA4 == 7) {
			R2_GLOBALS._player.disableControl();
			scene->_actor1.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				scene->_sceneMode = 1575;
			else
				scene->_sceneMode = 1583;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_arrUnkObj15502[6], &scene->_actor1, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

void Scene1550::UnkObj15502::subA5CDF(int strip) {
	_fieldA4 = strip;
	postInit();
	setup(1517, _fieldA4, 1);
	switch (_fieldA4 - 1) {
	case 0:
		if (R2_INVENTORY.getObjectScene(R2_22) == 0)
			setFrame(5);
		setPosition(Common::Point(287, 85));
		break;
	case 1:
		if (R2_INVENTORY.getObjectScene(R2_25) == 0)
			setFrame(5);
		setPosition(Common::Point(248, 100));
		break;
	case 2:
		if (R2_INVENTORY.getObjectScene(R2_28) == 0)
			setFrame(5);
		setPosition(Common::Point(217, 85));
		break;
	case 3:
		if (R2_INVENTORY.getObjectScene(R2_23))
			setFrame(5);
		setPosition(Common::Point(161, 121));
		break;
	case 4:
		if (R2_INVENTORY.getObjectScene(R2_27))
			setFrame(5);
		setPosition(Common::Point(117, 121));
		break;
	case 5:
		if (R2_INVENTORY.getObjectScene(R2_17))
			setFrame(5);
		setPosition(Common::Point(111, 85));
		break;
	case 6:
		if (R2_INVENTORY.getObjectScene(R2_45))
			setFrame(5);
		setPosition(Common::Point(95, 84));
		break;
	case 7: {
		setup(1516, 1, 1);
		setPosition(Common::Point(201, 45));
		Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
		if ((scene->_sceneMode == 1577) || (scene->_sceneMode == 1578))
			hide();
		fixPriority(92);
		setDetails(1550, 70, -1, -1, 2, (SceneItem *) NULL);
		}
		break;
	default:
		break;
	}
}

Scene1550::UnkObj15503::UnkObj15503() {
	_fieldA4 = 0;
}

void Scene1550::UnkObj15503::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
}

bool Scene1550::UnkObj15503::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	switch (_fieldA4) {
	case 1:
		if (scene->_actor13._frame == 5) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 25;
			if (scene->_actor4._frame == 1) {
				scene->setAction(&scene->_sequenceManager1, scene, 1560, &scene->_actor4, NULL);
				R2_GLOBALS.setFlag(20);
				setFrame(2);
			} else {
				scene->setAction(&scene->_sequenceManager1, scene, 1561, &scene->_actor4, NULL);
				R2_GLOBALS.clearFlag(20);
				setFrame(1);
			}
			scene->_unkArea1.remove();
		}
		break;
	case 2:
		R2_GLOBALS._player.disableControl();
		if (scene->_actor13._frame == 1) {
			scene->_sceneMode = 23;
			scene->setAction(&scene->_sequenceManager1, scene, 1560, this, NULL);
		} else {
			if (scene->_actor4._frame == 1)
				scene->_sceneMode = 24;
			else 
				scene->_sceneMode = 22;
			scene->setAction(&scene->_sequenceManager1, scene, 1561, this, NULL);
		}
	break;
	default:
		break;
	}
	return true;
}

void Scene1550::UnkArea1550::remove() {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	_unkObj155031.remove();
	_unkObj155032.remove();
	// sub201EA is a common part with UnkArea1200
	R2_GLOBALS._sceneItems.remove((SceneItem *)this);
	_areaActor.remove();
	SceneArea::remove();
	R2_GLOBALS._insetUp--;
	//
	if ((scene->_sceneMode >= 20) && (scene->_sceneMode <= 29))
		return;
	
	R2_GLOBALS._player.disableControl();
	if (scene->_actor4._frame == 1) {
		scene->_sceneMode = 1559;
		scene->setAction(&scene->_sequenceManager1, scene, 1559, &R2_GLOBALS._player, NULL);
	} else {
		scene->_sceneMode = 1562;
		scene->setAction(&scene->_sequenceManager1, scene, 1562, &R2_GLOBALS._player, NULL);
	}
}

void Scene1550::UnkArea1550::process(Event &event) {
// This is a copy of Scene1200::Area1::process
	if (_field20 != R2_GLOBALS._insetUp)
		return;

	CursorType cursor = R2_GLOBALS._events.getCursor();

	if (_areaActor._bounds.contains(event.mousePos.x + g_globals->gfxManager()._bounds.left , event.mousePos.y)) {
		if (cursor == _cursorNum)
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(_savedCursorNum); //, _cursorState);
	} else if (event.mousePos.y < 168) {
		if (cursor != _cursorNum) {
			_savedCursorNum = cursor;
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(CURSOR_INVALID);
		}
		if (event.eventType == EVENT_BUTTON_DOWN) {
			event.handled = true;
			warning("TODO: _cursorState = ???");
			R2_GLOBALS._events.setCursor(_savedCursorNum); //, _cursorState);
			remove();
		}
	}
}

void Scene1550::UnkArea1550::proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	// UnkArea1200::proc12();
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	_areaActor.postInit();
	_areaActor.setup(visage, stripFrameNum, frameNum);
	_areaActor.setPosition(Common::Point(posX, posY));
	_areaActor.fixPriority(250);
	_cursorNum = CURSOR_INVALID;
	scene->_sceneAreas.push_front(this);
	++R2_GLOBALS._insetUp;
	_field20 = R2_GLOBALS._insetUp;
	//

	proc13(1550, 67, -1, -1);
	_unkObj155031.postInit();
	_unkObj155031._fieldA4 = 1;
	if (scene->_actor4._frame == 1)
		_unkObj155031.setup(1559, 3, 1);
	else
		_unkObj155031.setup(1559, 3, 2);
	_unkObj155031.setPosition(Common::Point(142, 79));
	_unkObj155031.fixPriority(251);
	_unkObj155031.setDetails(1550, 68, -1, -1, 2, (SceneItem *) NULL);

	_unkObj155032.postInit();
	_unkObj155032._numFrames = 5;
	_unkObj155032._fieldA4 = 2;
	if (scene->_actor13._frame == 1)
		_unkObj155032.setup(1559, 2, 1);
	else
		_unkObj155032.setup(1559, 2, 2);
	_unkObj155032.setPosition(Common::Point(156, 103));
	_unkObj155032.fixPriority(251);
	_unkObj155032.setDetails(1550, 69, -1, -1, 2, (SceneItem *) NULL);
}

void Scene1550::UnkArea1550::proc13(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	// Copy of Scene1200::Area1::proc13
	_areaActor.setDetails(resNum, lookLineNum, talkLineNum, useLineNum, 2, (SceneItem *) NULL);
}

bool Scene1550::Hotspot1::startAction(CursorType action, Event &event) {
	return SceneHotspot::startAction(action, event);
}

bool Scene1550::Hotspot3::startAction(CursorType action, Event &event) {
	// Arrays related to this scene are all hacky in the origina: they are based on the impossibility to use Miranda
	assert ((R2_GLOBALS._player._characterIndex == 1) || (R2_GLOBALS._player._characterIndex == 2));
	// The original contains a debug message when CURSOR_TALK is used.
	// This part is totally useless, we could remove it (and the entire function as well)
	if (action == CURSOR_TALK)
		warning("Location: %d/%d - %d", R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex], R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2], k5A4D6[(R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] * 30)] + R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]);

	return SceneHotspot::startAction(action, event);
}

bool Scene1550::Actor6::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene1550::Actor7::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneMode = 80;
	scene->signal();
	
	return true;
}

bool Scene1550::Actor8::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	
	R2_GLOBALS._player.disableControl();
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	scene->_field412 = 1;
	if (R2_GLOBALS._player._characterIndex == 1)
		scene->_sceneMode = 1552;
	else
		scene->_sceneMode = 1588;
	
	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor8, NULL);
	return true;
}

bool Scene1550::Actor9::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		scene->_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->_stripManager.start(518, scene);
		else
			scene->_stripManager.start(520, scene);
		return true;
		break;
	case CURSOR_LOOK:
		SceneItem::display(1550, 41, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene1550::Actor10::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._characterIndex == 1)
		scene->_sceneMode = 1555;
	else
		scene->_sceneMode = 1589;

	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor10, NULL);
	return true;		
}

bool Scene1550::Actor11::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();
	scene->_field412 = 1;
	if (R2_GLOBALS._player._characterIndex == 1)
		scene->_sceneMode = 1586;
	else
		scene->_sceneMode = 1587;

	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor11, NULL);
	return true;		
}

bool Scene1550::Actor12::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._characterIndex == 2) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 1585;
		scene->setAction(&scene->_sequenceManager1, scene, 1585, &R2_GLOBALS._player, NULL);
	} else {
		R2_GLOBALS._player.disableControl();
		switch(scene->_field415) {
		case 0:
			scene->_actor13.fixPriority(168);
			scene->_actor4.fixPriority(125);
			scene->_sceneMode = 1558;
			scene->setAction(&scene->_sequenceManager1, scene, 1558, &R2_GLOBALS._player, NULL);
			break;
		case 1:
			return SceneActor::startAction(action, event);
			break;
		case 2:
			scene->_field415 = 1;
			scene->_sceneMode = 1563;
			scene->setAction(&scene->_sequenceManager1, scene, 1563, &R2_GLOBALS._player, &scene->_actor4, NULL);
			break;
		default:
			break;
		}
	}
	return true;

}

bool Scene1550::Actor13::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_field415 != 2)
			return SceneActor::startAction(action, event);
		
		if (R2_INVENTORY.getObjectScene(R2_45) == 1550) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1564;
			scene->setAction(&scene->_sequenceManager1, scene, 1564, &R2_GLOBALS._player, NULL);
		} else
			SceneItem::display(1550, 64, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
		break;
	case CURSOR_LOOK:
		if (scene->_field415 != 2)
			return SceneActor::startAction(action, event);

		if (R2_INVENTORY.getObjectScene(R2_45) == 1550) {
			SceneItem::display(1550, 74, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		} else
			SceneItem::display(1550, 64, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

Scene1550::Scene1550() {
	_field412 = 0;
	_field414 = 0;
	_field415 = 0;
	_field417 = 0;
	_field419 = 0;
}

void Scene1550::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsByte(_field414);
	s.syncAsSint16LE(_field415);
	s.syncAsSint16LE(_field417);
	s.syncAsSint16LE(_field419);
}

void Scene1550::postInit(SceneObjectList *OwnerList) {
	if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 9) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11))
		loadScene(1234);
	else
		loadScene(1550);
	
	scalePalette(65, 65, 65);
	setZoomPercents(30, 75, 170, 100);
	_field417 = 1550;
	_field419 = 0;
	SceneExt::postInit();
	
	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS.setFlag(R2_16);

	if ((R2_GLOBALS._player._characterScene[1] != 1550) && (R2_GLOBALS._player._characterScene[1] != 1580)) {
		R2_GLOBALS._player._characterScene[1] = 1550;
		R2_GLOBALS._player._characterScene[2] = 1550;
	}
	
	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._effect = 6;

	if (R2_GLOBALS._player._characterIndex == 1)
		R2_GLOBALS._player.setup(1500, 3, 1);
	else
		R2_GLOBALS._player.setup(1505, 3, 1);

	R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	
	if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 9) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11))
		R2_GLOBALS._player.setPosition(Common::Point(157, 135));
	else
		R2_GLOBALS._player.setPosition(Common::Point(160, 100));
	
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();
	
	_field414 = 0;
	_actor7.changeZoom(-1);
	R2_GLOBALS._player.changeZoom(-1);
	
	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1530:
		R2_GLOBALS._v565AE = 0;
	// No break on purpose
	case 300:
	// No break on purpose
	case 1500:
	// No break on purpose
	case 3150:
		R2_GLOBALS._sound1.play(105);
		break;
	case 1580:
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1580) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			
			_field412 = 1;
			
			_actor1.postInit();
			_arrUnkObj15502[7].subA5CDF(8);
			_arrUnkObj15502[7].hide();
			if (R2_GLOBALS._player._characterIndex == 1)
				_sceneMode = 1577;
			else
				_sceneMode = 1578;
			
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_actor1, &_arrUnkObj15502[7], NULL);
			R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1550;
		} else {
			R2_GLOBALS._player.enableControl();
		}
		break;
	default:
		break;
	}
	
	subA2B2F();
	
	_item1.setDetails(16, 1550, 10, -1, -1);
	_item2.setDetails(24, 1550, 10, -1, -1);
	_item3.setDetails(Rect(0, 0, 320, 200), 1550, 0, 1, -1, 1, NULL);
	
	if ((R2_GLOBALS._sceneManager._previousScene == 1500) && (R2_GLOBALS.getFlag(16))) {
		_sceneMode = 70;
		if (!R2_GLOBALS._sceneObjects->contains(&_actor7))
			_actor7.postInit();

		if (R2_GLOBALS._player._characterIndex == 1)
			_actor7.setVisage(1505);
		else
			_actor7.setVisage(1500);

		_actor7.changeZoom(77);
		_actor7.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);

		assert(_field419 >= 1550);
		R2_GLOBALS._walkRegions.enableRegion(k5A750[_field419 - 1550]);

		setAction(&_sequenceManager1, this, 1590, &_actor7, NULL);
	} else if ((_sceneMode != 1577) && (_sceneMode != 1578))
		R2_GLOBALS._player.enableControl();
}

void Scene1550::signal() {
	switch (_sceneMode) {
	case 1:
	// No break on purpose
	case 3:
	// No break on purpose
	case 5:
	// No break on purpose
	case 7:
		_field412 = 0;
		R2_GLOBALS._v56AAB = 0;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	case 20:
	// No break on purpose
	case 21:
	// No break on purpose
	case 25:
	// No break on purpose
	case 1563:
		R2_GLOBALS.clearFlag(20);
		_unkArea1.proc12(1559, 1, 1, 160, 125);
		R2_GLOBALS._player.enableControl();
		_sceneMode = 0;
		break;
	case 22:
		_unkArea1.remove();
		_sceneMode = 24;
		setAction(&_sequenceManager1, this, 1561, &_actor4, NULL);
		R2_GLOBALS.clearFlag(20);
		break;
	case 23:
		_unkArea1.remove();
		_sceneMode = 20;
		setAction(&_sequenceManager1, this, 1566, &_actor13, &_actor5, NULL);
		R2_GLOBALS.setFlag(21);
		break;
	case 24:
		_unkArea1.remove();
		_sceneMode = 21;
		setAction(&_sequenceManager1, this, 1567, &_actor13, &_actor5, NULL);
		R2_GLOBALS.clearFlag(19);
		break;
	case 30:
	// No break on purpose
	case 1556:
	// No break on purpose
	case 1557:
	// Nothing on purpose
		break;
	case 40: {
			_sceneMode = 41;
			Common::Point pt(_arrUnkObj15501[0]._position.x, _arrUnkObj15501[0]._position.y + 20);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 41:
		_sceneMode = 42;
		if (R2_GLOBALS._player._characterIndex == 1) {
			R2_GLOBALS._player.setup(1502, 8, 1);
		} else {
			R2_GLOBALS._player.changeZoom(R2_GLOBALS._player._percent + 14);
			R2_GLOBALS._player.setup(1516, 4, 1);
		}
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 42:
		_sceneMode = 43;
		warning("TODO: unknown use of arrUnkObj15501[0]._fieldA6");
		switch (_arrUnkObj15501[0]._frame - 1) {
		case 0:
			R2_INVENTORY.setObjectScene(26, R2_GLOBALS._player._characterIndex);
			break;
		case 1:
			R2_INVENTORY.setObjectScene(17, R2_GLOBALS._player._characterIndex);
			break;
		case 2:
			R2_INVENTORY.setObjectScene(22, R2_GLOBALS._player._characterIndex);
			break;
		case 3:
			R2_INVENTORY.setObjectScene(25, R2_GLOBALS._player._characterIndex);
			break;
		case 4:
			R2_INVENTORY.setObjectScene(45, R2_GLOBALS._player._characterIndex);
			break;
		case 5:
			R2_INVENTORY.setObjectScene(28, R2_GLOBALS._player._characterIndex);
			break;
		default:
			break;
		}
		_arrUnkObj15501[0].remove();
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 43:
		warning("TODO: unknown use of arrUnkObj15501[0]._fieldA6");
		if (R2_GLOBALS._player._characterIndex == 1)
			R2_GLOBALS._player.setVisage(1500);
		else {
			R2_GLOBALS._player.changeZoom(-1);
			R2_GLOBALS._player.setVisage(1505);
		}
		R2_GLOBALS._player.animate(ANIM_MODE_1, this);
		R2_GLOBALS._player.setStrip(8);
		R2_GLOBALS._player.enableControl();
		break;
	case 50:
		warning("STUB: sub_1D227()");
		++_sceneMode;
		setAction(&_sequenceManager1, this, 1591, &R2_GLOBALS._player, NULL);
		if (g_globals->_sceneObjects->contains(&_actor7))
			signal();
		else {
			_actor7.postInit();
			if (R2_GLOBALS._player._characterIndex == 1)
				_actor7.setVisage(1505);
			else
				_actor7.setVisage(1500);
			_actor7.changeZoom(77);
			_actor7.setAction(&_sequenceManager2, this, 1590, &_actor7, NULL);
			_actor7.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
		}
		break;
	case 51:
		++_sceneMode;
		break;
	case 52:
		_actor7.changeZoom(-1);
		_sceneMode = 1592;
		if (R2_GLOBALS._player._characterIndex == 1)
			setAction(&_sequenceManager1, this, 1592, &R2_GLOBALS._player, &_actor7, &_arrUnkObj15501[0], &_actor9, NULL);
		else
			setAction(&_sequenceManager1, this, 1593, &R2_GLOBALS._player, &_actor7, &_arrUnkObj15501[0], &_actor9, NULL);
		break;
	case 61:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		_field415 = 2;
		break;
	case 62:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		if (_field415 == 2) {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 70:
		R2_GLOBALS._v565EC[2] = R2_GLOBALS._v565EC[1];
		R2_GLOBALS._v565EC[4] = R2_GLOBALS._v565EC[3];
		R2_GLOBALS._v565EC[0] = 1;
		_sceneMode = 60;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(500, this);
		break;
	case 80:
		if (R2_GLOBALS.getFlag(16)) {
			_sceneMode = 60;
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._v565AE >= 3) {
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(572, this);
				else
					_stripManager.start(573, this);
			} else {
				++R2_GLOBALS._v565AE;
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(499 + R2_GLOBALS._v565AE, this);
				else					
					_stripManager.start(502 + R2_GLOBALS._v565AE, this);
			}
		} else {
			_sceneMode = 60;
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._v565AE >= 4) {
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(572, this);
				else
					_stripManager.start(573, this);
			} else {
				++R2_GLOBALS._v565AE;
				if (R2_GLOBALS._player._characterIndex == 1)
					_stripManager.start(563 + R2_GLOBALS._v565AE, this);
				else					
					_stripManager.start(567 + R2_GLOBALS._v565AE, this);
			}
		}
		break;
	case 1550:
		R2_GLOBALS._sceneManager.changeScene(1525);
		break;
	case 1552:
	// No break on purpose
	case 1588:
		R2_INVENTORY.setObjectScene(R2_19, R2_GLOBALS._player._characterIndex);
		_actor8.remove();
		_field412 = 0;
		R2_GLOBALS._player.enableControl();
		break;
	case 1553:
		R2_GLOBALS._sceneManager.changeScene(1575);
		break;
	case 1554:
		R2_GLOBALS._player.enableControl();
		_field412 = 0;
		break;
	case 1555:
	// No break on purpose
	case 1589:
		R2_INVENTORY.setObjectScene(R2_18, R2_GLOBALS._player._characterIndex);
		_actor10.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 1558:
		_actor13.fixPriority(124);
		_field415 = 1;
		_unkArea1.proc12(1559, 1, 1, 160, 125);
		R2_GLOBALS._player.enableControl();
		break;
	case 1559:
		_actor13.fixPriority(168);
		_actor4.fixPriority(169);
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.changeZoom(-1);
		_field415 = 0;
		R2_GLOBALS._player.enableControl();
		break;
	case 1562:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		_field415 = 2;
		break;
	case 1564:
		R2_INVENTORY.setObjectScene(R2_45, 1);
		_sceneMode = 1565;
		setAction(&_sequenceManager1, this, 1565, &R2_GLOBALS._player, NULL);
		break;
	case 1565:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 1569:
	// No break on purpose
	case 1579:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_22, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1570:
	// No break on purpose
	case 1580:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_25, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1571:
	// No break on purpose
	case 1581:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_18, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1572:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_23, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1573:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_27, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1574:
	// No break on purpose
	case 1582:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_17, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1575:
	// No break on purpose
	case 1583:
		_field412 = 0;
		_actor1.remove();
		R2_INVENTORY.setObjectScene(R2_45, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1576:
	// No break on purpose
	case 1584:
		R2_GLOBALS._sceneManager.changeScene(1580);
		R2_GLOBALS._player.enableControl();
		break;
	case 1577:
	// No break on purpose
	case 1578:
		_sceneMode = 0;
		_actor1.remove();
		_field412 = 0;
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl();
		break;
	case 1585:
		SceneItem::display(1550, 66, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);
		R2_GLOBALS._player.enableControl();
		break;
	case 1586:
	// No break on purpose
	case 1587:
		R2_INVENTORY.setObjectScene(R2_28, R2_GLOBALS._player._characterIndex);
		_actor1.remove();
		_field412 = 0;
		R2_GLOBALS._player.enableControl();
		break;
	case 1592:
		_actor9.remove();
		R2_INVENTORY.setObjectScene(R2_26, 1);
		if (R2_GLOBALS._player._characterIndex == 1) {
			R2_GLOBALS._v565EC[2] = R2_GLOBALS._v565EC[1];
			R2_GLOBALS._v565EC[4] = R2_GLOBALS._v565EC[3];
		} else {
			R2_GLOBALS._v565EC[1] = R2_GLOBALS._v565EC[2];
			R2_GLOBALS._v565EC[3] = R2_GLOBALS._v565EC[4];
		}
		R2_GLOBALS._player.enableControl();
		break;
	default:
		_sceneMode = 62;
		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
		break;
	}
}

void Scene1550::process(Event &event) {
	if ((!R2_GLOBALS._player._canWalk) && (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN) && (event.eventType == EVENT_BUTTON_DOWN) && (this->_screenNumber == 1234)) {
		int curReg = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);
		if (curReg == 0)
			_field412 = 1;
		else if (((R2_GLOBALS._player._position.y < 90) && (event.mousePos.y > 90)) || ((R2_GLOBALS._player._position.y > 90) && (event.mousePos.y < 90)))
			_field412 = 1;
		else
			_field412 = 0;
		
		if ((curReg == 13) || (curReg == 14))
			_field412 = 0;
	}
	
	Scene::process(event);
}

void Scene1550::dispatch() {
	Scene::dispatch();

	// Arrays related to this scene are all hacky in the origina: they are based on the impossibility to use Miranda
	assert ((R2_GLOBALS._player._characterIndex == 1) || (R2_GLOBALS._player._characterIndex == 2));

	if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 15) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 16)) {
		R2_GLOBALS._player._shade = 0;
		// Original game contains a switch based on an uninitialised variable.
		// Until we understand what should really happen there, this code is unused on purpose
		int missingVariable = 0;
		switch (missingVariable) {
		case 144:
		// No break on purpose
		case 146:
			_actor13._frame = 5;
			R2_GLOBALS._player._shade = 3;
			break;
		case 148:
		// No break on purpose
		case 149:
			_actor13._frame = 1;
		// No break on purpose
		case 147:
		// No break on purpose
		case 150:
			R2_GLOBALS._player._shade = 3;
			break;
		default:
			break;
		}
	}
	
	if (_field412 != 0)
		return;
	
	switch (R2_GLOBALS._player.getRegionIndex() - 11) {
	case 0:
	// No break on purpose
	case 5:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1;
		_field412 = 1;
		--R2_GLOBALS._v565EC[2 + R2_GLOBALS._player._characterIndex];
		
		subA2B2F();
		
		R2_GLOBALS._player.setPosition(Common::Point( 160 - (((((160 - R2_GLOBALS._player._position.x) * 100) / 108) * 172) / 100), 145));
		if (R2_GLOBALS._player._position.x < 160) {
			Common::Point pt(R2_GLOBALS._player._position.x + 5, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if (R2_GLOBALS._player._position.x <= 160) { // the check is really in the original...
			Common::Point pt(R2_GLOBALS._player._position.x, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			Common::Point pt(R2_GLOBALS._player._position.x - 5, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 1:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 3;
		_field412 = 1;
		++R2_GLOBALS._v565EC[2 + R2_GLOBALS._player._characterIndex];
		
		subA2B2F();
		
		R2_GLOBALS._player.setPosition(Common::Point( 160 - (((((160 - R2_GLOBALS._player._position.x) * 100) / 172) * 108) / 100), 19));
		if (R2_GLOBALS._player._position.x < 160) {
			Common::Point pt(R2_GLOBALS._player._position.x + 5, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if (R2_GLOBALS._player._position.x <= 160) { // the check is really in the original...
			Common::Point pt(R2_GLOBALS._player._position.x, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			Common::Point pt(R2_GLOBALS._player._position.x - 5, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 5;
		_field412 = 1;
		++R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex];
		
		subA2B2F();
		
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 9) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11) && (R2_GLOBALS._player._position.y > 50) && (R2_GLOBALS._player._position.y < 135)) {
			if (R2_GLOBALS._player._position.y >= 85) {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 10));
				Common::Point pt(R2_GLOBALS._player._position.x + 30, R2_GLOBALS._player._position.y + 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 10));
				Common::Point pt(R2_GLOBALS._player._position.x + 30, R2_GLOBALS._player._position.y - 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y));
			Common::Point pt(R2_GLOBALS._player._position.x + 10, R2_GLOBALS._player._position.y);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 7;
		_field412 = 1;
		--R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex];
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 24) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11)) {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y / 2));
			Common::Point pt(265, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 9) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11) && (R2_GLOBALS._player._position.y > 50) && (R2_GLOBALS._player._position.y < 135)) {
			if (R2_GLOBALS._player._position.y >= 85) {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 10));
				Common::Point pt(R2_GLOBALS._player._position.x - 30, R2_GLOBALS._player._position.y + 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 10));
				Common::Point pt(R2_GLOBALS._player._position.x - 30, R2_GLOBALS._player._position.y - 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y));
			Common::Point pt(R2_GLOBALS._player._position.x - 10, R2_GLOBALS._player._position.y);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	default:
		break;
	}
}

void Scene1550::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == 3)
		R2_GLOBALS._sound1.fadeOut2(NULL);
	
	SceneExt::saveCharacter(characterIndex);
}

void Scene1550::SceneActor1550::subA4D14(int frameNumber, int strip) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	postInit();
	if (scene->_field414 == 2)
		setup(1551, strip, frameNumber);
	else
		setup(1554, strip, frameNumber);

	switch (strip) {
	case 0:
		switch (frameNumber - 1) {
		case 0:
			setup(1551, 1, 1);
			setPosition(Common::Point(30, 67));
			break;
		case 1:
			setup(1560, 1, 5);
			setPosition(Common::Point(141, 54));
			break;
		case 2:
			setup(1560, 2, 5);
			setPosition(Common::Point(178, 54));
			break;
		case 3:
			setup(1560, 2, 1);
			setPosition(Common::Point(289, 67));
			break;
		case 4:
			setup(1560, 2, 2);
			setPosition(Common::Point(298, 132));
			break;
		case 5:
			setup(1560, 1, 2);
			setPosition(Common::Point(21, 132));
			break;
		case 6:
			setup(1560, 2, 4);
			setPosition(Common::Point(285, 123));
			break;
		case 7:
			setup(1560, 1, 3);
			setPosition(Common::Point(30, 111));
			break;
		case 8:
			setup(1560, 2, 3);
			setPosition(Common::Point(289, 111));
			break;
		case 9:
			setup(1560, 1, 4);
			setPosition(Common::Point(34, 123));
			break;
		default:
			break;
		}
		fixPriority(1);
		break;
	case 1:
		if (frameNumber == 3) {
			setup(1553, 3, 1);
			setPosition(Common::Point(48, 44));
			fixPriority(2);
		} else {
			fixPriority(1);
			setPosition(Common::Point(32, 17));
		}

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 3:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 2:
		fixPriority(1);
		switch (frameNumber) {
		case 4:
			setup(1553, 4, 1);
			setPosition(Common::Point(48, 168));
			break;
		case 5:
			setup(1553, 3, 2);
			setPosition(Common::Point(20, 168));
			fixPriority(250);
			break;
		default:
			setPosition(Common::Point(28, 116));
			break;
		}

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 4:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 5:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 3:
		switch (frameNumber) {
		case 2:
			fixPriority(2);
			if (scene->_field414 == 2)
				setup(1553, 2, 1);
			else 
				setup(1556, 2, 1);
			setPosition(Common::Point(160, 44));
			break;
		case 3:
			fixPriority(2);
			setup(1553, 5, 1);
			setPosition(Common::Point(178, 44));
			break;
		default:
			fixPriority(1);
			setPosition(Common::Point(160, 17));
			break;
		}

		if (frameNumber == 1)
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
		else
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
		break;
	case 4:
		if (frameNumber == 2) {
			fixPriority(250);
			if (scene->_field414 == 2)
				setup(1553, 1, 1);
			else
				setup(1556, 1, 1);
		} else {
			fixPriority(2);
		}
		
		if (frameNumber != 1)
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);

		setPosition(Common::Point(160, 168));
		break;
	case 5:
		fixPriority(1);
		setPosition(Common::Point(287, 17));

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 3:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 6:
		fixPriority(1);
		setPosition(Common::Point(291, 116));

		if (frameNumber == 2)
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
		else
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
		break;
	default:
		break;
	}

}

void Scene1550::subA2B2F() {
	Rect tmpRect;
	_field419 = 0;
	_field415 = 0;
	
	tmpRect = R2_GLOBALS._v5589E;
	
	_actor14.remove();
	_actor17.remove();
	_actor15.remove();
	_actor19.remove();
	_actor16.remove();
	_actor18.remove();
	
	for (int i = 0; i < 8; ++i)
		_arrUnkObj15501[i].remove();
	
	_actor6.remove();

	for (int i = 0; i < 8; ++i)
		_arrUnkObj15502[i].remove();
	
	_actor8.remove();
	_actor9.remove();
	_actor10.remove();
	_actor3.remove();
	_actor11.remove();
	
	if ((_sceneMode != 1577) && (_sceneMode != 1578))
		_actor1.remove();
	
	_actor2.remove();
	_actor7.remove();
	_actor13.remove();
	_actor5.remove();
	_actor12.remove();
	_actor4.remove();
	
	switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2]) {
	case 0:
		switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]) {
		case 3:
			R2_GLOBALS._walkRegions.load(1554);
			_field419 = 1554;
			break;
		case 4:
			R2_GLOBALS._walkRegions.load(1553);
			_field419 = 1553;
			break;
		default:
			break;
		}
		break;
	case 3:
	// No break on purpose
	case 4:
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 23) || (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex])) {
			if (!R2_GLOBALS.getFlag(16)) {
				R2_GLOBALS._walkRegions.load(1559);
				_field419 = 1559;
			}
		}
		break;
	case 7:
		switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]) {
		case 10:
			R2_GLOBALS._walkRegions.load(1555);
			_field419 = 1555;
			break;
		case 11:
			R2_GLOBALS._walkRegions.load(1556);
			_field419 = 1556;
			break;
		default:
			break;
		}
		break;
	case 11:
		switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]) {
		case 24:
			R2_GLOBALS._walkRegions.load(1558);
			_field419 = 1558;
			break;
		case 25:
			R2_GLOBALS._walkRegions.load(1557);
			_field419 = 1557;
			break;
		default:
			break;
		}
		break;
	case 16:
		switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]) {
		case 2:
			R2_GLOBALS._walkRegions.load(1552);
			_field419 = 1552;
			break;
		case 3:
			R2_GLOBALS._walkRegions.load(1551);
			_field419 = 1551;
			break;
		case 15:
			R2_GLOBALS._walkRegions.load(1575);
			_field419 = 1575;
		default:
			break;
		}
		break;
	default:
		break;
	}

	int varA = 0;

	if (!R2_GLOBALS.getFlag(16)) {
		switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] - 2) {
		case 0:
			switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] - 22) {
				case 0:
					varA = 1553;
					_actor15.subA4D14(6, 0);
					break;
				case 1:
				// No break on purpose
				case 2:
				// No break on purpose
				case 3:
				// No break on purpose
				case 4:
					varA = 1553;
					break;
				case 5:
					varA = 1553;
					_actor15.subA4D14(6, 0);
					break;
				default:
					break;
			}
			break;
		case 1:
		// No break on purpose
		case 2:
			switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] - 21) {
			case 0:
				varA = 1550;
				_actor15.subA4D14(9, 0);
				break;
			case 1:
				varA = 1552;
				_actor15.subA4D14(10, 0);
				break;
			case 2:
			// No break on purpose
			case 3:
			// No break on purpose
			case 4:
			// No break on purpose
			case 5:
				varA = 1552;
				break;
			case 6:
				varA = 1552;
				_actor15.subA4D14(7, 0);
				break;
			case 7:
				varA = 1550;
				_actor15.subA4D14(8, 0);
				break;
			default:
				break;
			}
			break;
		case 3:
			switch (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] - 21) {
			case 0:
				varA = 1550;
				_actor15.subA4D14(4, 0);
				break;
			case 1:
				varA = 1550;
				_actor15.subA4D14(3, 0);
				break;
			case 2:
			// No break on purpose
			case 3:
			// No break on purpose
			case 4:
			// No break on purpose
			case 5:
				varA = 1551;
				break;
			case 6:
				varA = 1550;
				_actor15.subA4D14(2, 0);
				break;
			case 7:
				varA = 1550;
				_actor15.subA4D14(1, 0);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] > 0) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] <= 29) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] >= 20) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] > 7)) {
			R2_GLOBALS.setFlag(16);
			R2_GLOBALS._sceneManager.changeScene(1500);
		}
	}

	if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == 9) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 11)) {
		if (R2_GLOBALS._sceneManager._sceneNumber != 1234) {
			R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;
			loadScene(1234);
			R2_GLOBALS._sceneManager._hasPalette = false;
			_field414 = 0;
		}
	} else {
		if (R2_GLOBALS._sceneManager._sceneNumber == 1234) {
			R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;
			loadScene(1550);
			R2_GLOBALS._sceneManager._hasPalette = false;
		}
	}

	if (R2_GLOBALS._sceneManager._sceneNumber == 1234)
		_field419 = 1576;

	if (_field414 == 0) {
		_field414 = 1;
	} else {
		if (_field414 == 2) {
			_field414 = 3;
		} else {
			_field414 = 2;
		}
		
		if (R2_GLOBALS._sceneManager._sceneNumber == 1550){
			warning("Mouse_hideIfNeeded()");
			warning("gfx_set_pane_p");
			for (int i = 3; i != 168; ++i) {
				warning("sub294D2(4, i, 312,  var14C)");
				warning("missing for loop, to be implemented");
				warning("gfx_draw_slice");
			}
			warning("Missing sub2957D()");
			warning("gfx_set_pane_p()");
			R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;

			if (varA == 0) {
				if (_field417 != 1550)
					g_globals->_scenePalette.loadPalette(1550);
					R2_GLOBALS._sceneManager._hasPalette = true;
			} else {
				g_globals->_scenePalette.loadPalette(varA);
				R2_GLOBALS._sceneManager._hasPalette = true;
			}

			if (R2_GLOBALS._sceneManager._hasPalette)
				_field417 = varA;

			warning("sub_2C429()");
		}
	}

	switch (k5A4D6[(R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] * 30)] + R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]) {
	case 0:
		if (_field419 == 0) {
			R2_GLOBALS._walkRegions.load(1550);
			_field419 = 1550;
		}
		break;
	case 1:
		if (_field419 == 0) {
			R2_GLOBALS._walkRegions.load(1560);
			_field419 = 1560;
		}
		_actor14.subA4D14(2, 1);
		_actor15.subA4D14(1, 3);
		_actor16.subA4D14(2, 5);
		break;
	case 2:
		R2_GLOBALS._walkRegions.load(1561);
		_field419 = 1561;
		_actor14.subA4D14(2, 1);
		_actor17.subA4D14(2, 2);
		_actor15.subA4D14(1, 3);
		_actor16.subA4D14(2, 5);
		break;
	case 3:
		R2_GLOBALS._walkRegions.load(1562);
		_field419 = 1562;
		_actor14.subA4D14(2, 1);
		_actor15.subA4D14(1, 3);
		_actor16.subA4D14(2, 5);
		_actor18.subA4D14(2, 6);
		break;
	case 4:
		R2_GLOBALS._walkRegions.load(1563);
		_field419 = 1563;
		_actor15.subA4D14(2, 3);
		break;
	case 5:
		R2_GLOBALS._walkRegions.load(1564);
		_field419 = 1564;
		_actor19.subA4D14(2, 4);
		break;
	case 6:
		R2_GLOBALS._walkRegions.load(1565);
		_field419 = 1565;
		_actor14.subA4D14(1, 1);
		_actor17.subA4D14(1, 2);
		_actor15.subA4D14(3, 3);
		break;
	case 7:
		R2_GLOBALS._walkRegions.load(1566);
		_field419 = 1566;
		_actor14.subA4D14(1, 1);
		_actor17.subA4D14(1, 2);
		_actor15.subA4D14(2, 4);
		break;
	case 8:
		R2_GLOBALS._walkRegions.load(1567);
		_field419 = 1567;
		_actor17.subA4D14(5, 2);
		break;
	case 9:
		R2_GLOBALS._walkRegions.load(1568);
		_field419 = 1568;
		_actor17.subA4D14(4, 2);
		break;
	case 10:
		R2_GLOBALS._walkRegions.load(1569);
		_field419 = 1569;
		_actor14.subA4D14(3, 1);
		break;
	case 11:
		R2_GLOBALS._walkRegions.load(1570);
		_field419 = 1570;
		_actor14.subA4D14(1, 1);
		_actor17.subA4D14(1, 2);
		break;
	case 12:
		R2_GLOBALS._walkRegions.load(1571);
		_field419 = 1571;
		_actor16.subA4D14(1, 5);
		_actor18.subA4D14(1, 6);
		break;
	case 13:
		R2_GLOBALS._walkRegions.load(1572);
		_field419 = 1572;
		_actor14.subA4D14(1, 1);
		_actor17.subA4D14(1, 2);
		_actor19.subA4D14(1, 4);
		break;
	case 14:
		R2_GLOBALS._walkRegions.load(1573);
		_field419 = 1573;
		_actor19.subA4D14(1, 4);
		_actor16.subA4D14(1, 5);
		_actor18.subA4D14(1, 6);
		break;
	case 15:
		R2_GLOBALS._walkRegions.load(1574);
		_field419 = 1574;
		_actor19.subA4D14(1, 4);
		break;
	case 16:
		R2_GLOBALS._walkRegions.load(1570);
		_field419 = 1570;
		_actor14.subA4D14(2, 1);
		_actor17.subA4D14(2, 2);
		break;
	case 17:
		R2_GLOBALS._walkRegions.load(1570);
		_field419 = 1570;
		_actor14.subA4D14(2, 1);
		_actor17.subA4D14(3, 2);
		break;
	case 18:
		R2_GLOBALS._walkRegions.load(1571);
		_field419 = 1571;
		_actor16.subA4D14(2, 5);
		_actor18.subA4D14(2, 6);
		break;
	case 19:
		R2_GLOBALS._walkRegions.load(1571);
		_field419 = 1571;
		_actor16.subA4D14(2, 5);
		_actor18.subA4D14(3, 6);
		break;
	default:
		break;
	}

	int di = 0;
	int tmpIdx = 0;
	// Original game was checking "i < 129" but it was clearly a bug as it's out of bounds
	for (int i = 0; i < 129 * 4; i += 4) {
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == k562CC[i]) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == k562CC[i + 1]) && (k562CC[i + 2] != 0)) {
			tmpIdx = k562CC[i + 3];
			_arrUnkObj15501[di].postInit();
			_arrUnkObj15501[di]._effect = 6;
			_arrUnkObj15501[di]._shade = 0;
			_arrUnkObj15501[di]._fieldA4 = tmpIdx;
			_arrUnkObj15501[di]._fieldA6 = i;
			_arrUnkObj15501[di].setDetails(1550, 62, -1, 63, 2, (SceneItem *) NULL);
			if (k562CC[i + 2] == 41) {
				_arrUnkObj15501[di].changeZoom(-1);
				_arrUnkObj15501[di].setPosition(Common::Point(150, 70));
				_arrUnkObj15501[di].setup(1562, 1, 1);
				R2_GLOBALS._walkRegions.enableRegion(k5A78C);
				R2_GLOBALS._walkRegions.enableRegion(k5A78D);
				R2_GLOBALS._walkRegions.enableRegion(k5A790);
				R2_GLOBALS._walkRegions.enableRegion(k5A791);
				if (R2_INVENTORY.getObjectScene(R2_26) == 1550) {
					_actor9.postInit();
					_actor9.setup(1562, 3, 1);
					_actor9.setPosition(Common::Point(150, 70));
					_actor9.fixPriority(10);
					_actor9.setDetails(1550, 41, -1, 42, 2, (SceneItem *) NULL);
				}
			} else {
				if (k562CC[i + 2] > 40) {
					_arrUnkObj15501[di].changeZoom(100);
					_arrUnkObj15501[di].setup(1561, 1, k562CC[i + 2] - 40);
				} else {
					_arrUnkObj15501[di].changeZoom(-1);
					_arrUnkObj15501[di].setup(1552, ((k562CC[i + 2] - 1) / 5) + 1, ((k562CC[i + 2] - 1) % 5) + 1);
				}
				_arrUnkObj15501[di].setPosition(Common::Point(k5A72E[tmpIdx], k5A73F[tmpIdx]));
				if (k5A78A[tmpIdx] != 0)
					R2_GLOBALS._walkRegions.enableRegion(k5A78A[tmpIdx]);
				di++;
			}
		}
	}

	for (int i = 0; i < 15 * 3; i++) {
		if ((R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex] == k5A79B[i]) && (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == k5A79B[i + 1])) {
			tmpIdx = k5A79B[i + 2];
			switch (tmpIdx - 1) {
			case 0:
				if (!R2_GLOBALS.getFlag(16)) {
					_actor1.postInit();
					if (R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] == 3)
						_actor1.setup(1555, 2, 1);
					else
						_actor1.setup(1555, 1, 1);
					_actor1.setPosition(Common::Point(150, 100));
					_actor1.fixPriority(92);
					_actor1.setDetails(1550, 73, -1, -1, 2, (SceneItem *) NULL);
				}
				break;
			case 1:
				_actor13.postInit();
				warning("_actor13._actorName = \"dish\";");
				if (R2_GLOBALS.getFlag(19))
					_actor13.setup(1556, 3, 5);
				else
					_actor13.setup(1556, 3, 1);
				_actor13.changeZoom(95);
				_actor13.setPosition(Common::Point(165, 83));
				_actor13.fixPriority(168);
				_actor13.setDetails(1550, 17, -1, 19, 2, (SceneItem *) NULL);

				_actor12.postInit();
				_actor12.setup(1556, 4, 1);
				_actor12.setPosition(Common::Point(191, 123));
				_actor12.changeZoom(95);
				_actor12.setDetails(1550, 65, -1, 66, 2, (SceneItem *) NULL);

				_actor5.postInit();
				_actor5._numFrames = 5;
				if (R2_GLOBALS.getFlag(19))
					_actor5.setup(1556, 8, 5);
				else
					_actor5.setup(1556, 8, 1);

				_actor5.setPosition(Common::Point(156, 151));
				_actor5.fixPriority(10);

				_actor4.postInit();
				if (R2_GLOBALS.getFlag(20))
					_actor4.setup(1558, 3, 10);
				else
					_actor4.setup(1558, 3, 1);

				_actor4.setPosition(Common::Point(172, 48));
				_actor4.fixPriority(169);
				R2_GLOBALS._walkRegions.enableRegion(k5A78A[15]);
				break;
			case 2:
				_actor6.postInit();
				_actor6.setup(1550, 1, 1);
				_actor6.setPosition(Common::Point(259, 55));
				_actor6.fixPriority(133);
				_actor6.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_actor1.postInit();
				_actor1.setup(1550, 1, 2);
				_actor1.setPosition(Common::Point(259, 133));
				_actor1.fixPriority(105);
				_actor1.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				if (R2_INVENTORY.getObjectScene(R2_18) == 1550) {
					_actor10.postInit();
					_actor10.setup(1550, 7, 2);
					_actor10.setPosition(Common::Point(227, 30));
					_actor10.fixPriority(130);
					_actor10.setDetails(1550, 29, -1, 63, 2, (SceneItem *) NULL);
				}
				break;
			case 3:
				_actor6.postInit();
				_actor6.setup(1550, 1, 4);
				_actor6.setPosition(Common::Point(76, 131));
				_actor6.fixPriority(10);
				_actor6.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_actor1.postInit();
				_actor1.setup(1550, 1, 3);
				_actor1.setPosition(Common::Point(76, 64));
				_actor1.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				if (R2_INVENTORY.getObjectScene(R2_28) == 1550) {
					_actor11.postInit();
					_actor11.setup(1504, 4, 1);
					_actor11.setPosition(Common::Point(49, 35));
					_actor11.animate(ANIM_MODE_2, NULL);
					_actor11._numFrames = 4;
					_actor11.fixPriority(65);
					_actor11.setDetails(1550, 14, 15, 63, 2, (SceneItem *) NULL);
				}
				if (R2_INVENTORY.getObjectScene(R2_19) == 1550) {
					_actor8.postInit();
					_actor8.setup(1550, 7, 1);
					_actor8.setPosition(Common::Point(45, 44));
					_actor8.fixPriority(150);
					_actor8.setDetails(1550, 44, -1, 63, 2, (SceneItem *) NULL);
				}
				break;
			case 4:
				_actor6.postInit();
				_actor6.setup(1550, 2, 4);
				_actor6.setPosition(Common::Point(243, 131));
				_actor6.fixPriority(10);
				_actor6.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				
				_actor1.postInit();
				_actor1.setup(1550, 2, 3);
				_actor1.setPosition(Common::Point(243, 64));
				_actor1.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 5:
				_actor6.postInit();
				_actor6.setup(1550, 2, 1);
				_actor6.setPosition(Common::Point(60, 55));
				_actor6.fixPriority(133);
				_actor6.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_actor1.postInit();
				_actor1.setup(1550, 2, 2);
				_actor1.setPosition(Common::Point(60, 133));
				_actor1.fixPriority(106);
				_actor1.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 6:
				_actor6.postInit();
				_actor6.setup(1550, 3, 1);
				_actor6.setPosition(Common::Point(281, 132));
				_actor6.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 7:
				_actor6.postInit();
				_actor6.setup(1550, 3, 2);
				_actor6.setPosition(Common::Point(57, 96));
				_actor6.fixPriority(70);
				_actor6.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor1.postInit();
				_actor1.setup(1550, 3, 3);
				_actor1.setPosition(Common::Point(145, 88));
				_actor1.fixPriority(55);
				_actor1.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor2.postInit();
				_actor2.setup(1550, 3, 4);
				_actor2.setPosition(Common::Point(64, 137));
				_actor2.fixPriority(115);
				_actor2.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor3.postInit();
				_actor3.setup(1550, 5, 1);
				_actor3.setPosition(Common::Point(60, 90));
				_actor3.fixPriority(45);
				break;
			case 8:
				_actor6.postInit();
				_actor6.setup(1550, 4, 2);
				_actor6.setPosition(Common::Point(262, 96));
				_actor6.fixPriority(70);
				_actor6.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor1.postInit();
				_actor1.setup(1550, 4, 3);
				_actor1.setPosition(Common::Point(174, 88));
				_actor1.fixPriority(55);
				_actor1.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor2.postInit();
				_actor2.setup(1550, 4, 4);
				_actor2.setPosition(Common::Point(255, 137));
				_actor2.fixPriority(115);
				_actor2.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_actor3.postInit();
				_actor3.setup(1550, 6, 1);
				_actor3.setPosition(Common::Point(259, 90));
				_actor3.fixPriority(45);
				break;
			case 9:
				_actor6.postInit();
				_actor6.setup(1550, 4, 1);
				_actor6.setPosition(Common::Point(38, 132));
				_actor6.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 11:
				_arrUnkObj15502[7].subA5CDF(8);
				_arrUnkObj15502[0].subA5CDF(1);
				_arrUnkObj15502[1].subA5CDF(2);
				_arrUnkObj15502[2].subA5CDF(3);
				_arrUnkObj15502[3].subA5CDF(4);
				_arrUnkObj15502[4].subA5CDF(5);
				_arrUnkObj15502[5].subA5CDF(6);
				_arrUnkObj15502[6].subA5CDF(7);
			default:
				break;
			}
		}
	}

	if ((R2_GLOBALS._v565EC[1] == R2_GLOBALS._v565EC[2]) && (R2_GLOBALS._v565EC[3] == R2_GLOBALS._v565EC[4])) {
		_actor7.postInit();
		_actor7._effect = 7;
		_actor7.changeZoom(-1);

		assert((_field419 >= 1550) && (_field419 <= 2008));
		R2_GLOBALS._walkRegions.enableRegion(k5A750[_field419 - 1550]);
		_actor7.setPosition(Common::Point(k5A72E[k5A76D[_field419 - 1550]], k5A73F[k5A76D[_field419 - 1550]] + 8));
		if (R2_GLOBALS._player._characterIndex == 1) {
			if (R2_GLOBALS._player._characterScene[2] == 1580) {
				_actor7.setup(1516, 3, 17);
				_actor7.setPosition(Common::Point(272, 94));
				_actor7.fixPriority(91);
				_actor7.changeZoom(100);
				_actor7.setDetails(1550, -1, -1, -1, 5, &_arrUnkObj15502[7]);
			} else {
				_actor7.setup(1505, 6, 1);
				_actor7.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
			}
		} else {
			if (R2_GLOBALS._player._characterScene[1] == 1580) {
				_actor7.setup(1516, 2, 14);
				_actor7.setPosition(Common::Point(276, 97));
				_actor7.fixPriority(91);
				_actor7.changeZoom(100);
				_actor7.setDetails(1550, -1, -1, -1, 5, &_arrUnkObj15502[7]);
			} else {
				_actor7.setup(1500, 6, 1);
				_actor7.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
			}
		}
	}
	R2_GLOBALS._uiElements.updateInventory();
}

/*--------------------------------------------------------------------------
 * Scene 1575 - 
 *
 *--------------------------------------------------------------------------*/
Scene1575::Scene1575() {
	_field412 = 0;
	_field414 = 0;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;
}

void Scene1575::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
}

Scene1575::Hotspot1::Hotspot1() {
	_field34 = 0;
	_field36 = 0;
}

void Scene1575::Hotspot1::synchronize(Serializer &s) {
	NamedHotspot::synchronize(s);

	s.syncAsSint16LE(_field34);
	s.syncAsSint16LE(_field36);
}

void Scene1575::Hotspot1::process(Event &event) {
	if ((event.eventType != EVENT_BUTTON_DOWN) || (R2_GLOBALS._events.getCursor() != R2_STEPPING_DISKS) || (!_bounds.contains(event.mousePos))) {
		if (_field36 == 0)
			return;
		if ((_field34 == 1)  || (event.eventType == EVENT_BUTTON_UP) || (!_bounds.contains(event.mousePos))) {
			_field36 = 0;
			return;
		}
	}
	_field36 = 1;
	Scene1575 *scene = (Scene1575 *)R2_GLOBALS._sceneManager._scene;

	event.handled = true;
	if (R2_GLOBALS.getFlag(18) && (_field34 > 1) && (_field34 < 6)) {
		warning("sub1A03B(\"Better not move the laser while it\'s firing!\", 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);");
		return;
	}
	int di = scene->_actor1._position.x;
	
	switch (_field34 - 1) {
	case 0:
		if (R2_GLOBALS.getFlag(18)) {
			scene->_actor14.hide();
			scene->_actor15.hide();
			R2_GLOBALS.clearFlag(18);
		} else if ((scene->_actor12._position.x == 85) && (scene->_actor12._position.y == 123)) {
			scene->_actor14.show();
			scene->_actor15.show();
			R2_GLOBALS.setFlag(18);
		} else {
			warning("sub1A03B(\"That\'s probably not a good thing, ya know!\", 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, -999);");
		}
		break;
	case 1:
		if (scene->_field41A < 780) {
			if (di > 54)
				di -= 65;
			di += 2;
			scene->_field41A += 2;
			
			for (int i = 0; i < 17; i++)
				scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x + 2, scene->_arrActor[i]._position.y));
			
			scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x + 2, scene->_actor13._position.y));
			scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x + 2, scene->_actor12._position.y));
			scene->_actor1.setPosition(Common::Point(di, scene->_actor1._position.y));
			scene->_actor2.setPosition(Common::Point(di + 65, scene->_actor1._position.y));
			scene->_actor3.setPosition(Common::Point(di + 130, scene->_actor1._position.y));
		}
		break;
	case 2:
		if (scene->_field41A > 0) {
			if (di < -8)
				di += 65;
			
			di -= 2;
			scene->_field41A -= 2;
			for (int i = 0; i < 178; i++)
				scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x - 2, scene->_arrActor[i]._position.y));
			
			scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x - 2, scene->_actor13._position.y));
			scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x - 2, scene->_actor12._position.y));
			scene->_actor1.setPosition(Common::Point(di, scene->_actor1._position.y));
			scene->_actor2.setPosition(Common::Point(di + 65, scene->_actor1._position.y));
			scene->_actor3.setPosition(Common::Point(di + 130, scene->_actor1._position.y));
		}
		break;
	case 3: {
			int tmpPosY = scene->_actor1._position.y;
			if (tmpPosY < 176) {
				++tmpPosY;
				for (int i = 0; i < 17; ++i)
					scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x, scene->_arrActor[i]._position.y + 1));

				scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x, scene->_actor13._position.y + 1));
				scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x, scene->_actor12._position.y + 1));
				scene->_actor1.setPosition(Common::Point(di, scene->_actor1._position.y));
				scene->_actor2.setPosition(Common::Point(di + 65, scene->_actor1._position.y));
				scene->_actor3.setPosition(Common::Point(di + 130, scene->_actor1._position.y));
			}
		}
		break;
	case 4: {
			int tmpPosY = scene->_actor1._position.y;
			if (tmpPosY > 145) {
				tmpPosY--;
				for (int i = 0; i < 17; ++i)
					scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x, scene->_arrActor[i]._position.y - 1));

				scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x, scene->_actor13._position.y - 1));
				scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x, scene->_actor12._position.y - 1));
				scene->_actor1.setPosition(Common::Point(di, scene->_actor1._position.y));
				scene->_actor2.setPosition(Common::Point(di + 65, scene->_actor1._position.y));
				scene->_actor3.setPosition(Common::Point(di + 130, scene->_actor1._position.y));
			}
		}
		break;
	case 5:
		R2_GLOBALS._sceneManager.changeScene(1550);
		break;
	default:
		break;
	}

	int j = 0;
	for (int i = 0; i < 17; i++) {
		if (scene->_arrActor[i]._bounds.contains(85, 116))
			j = i;
	}

	if (scene->_actor13._bounds.contains(85, 116))
		j = 18;

	if (scene->_actor12._bounds.contains(85, 116))
		j = 19;

	if (j)
		scene->_actor11.show();
	else
		scene->_actor11.hide();
}

bool Scene1575::Hotspot1::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;
	return SceneHotspot::startAction(action, event);
}

void Scene1575::Hotspot1::subA910D(int indx) {
	warning("STUB: Scene1575:Hotspot1::subA910D(%d)", indx);
}

void Scene1575::postInit(SceneObjectList *OwnerList) {
	loadScene(1575);
	R2_GLOBALS._v58CE2 = 0;
	R2_GLOBALS._v5589E = Rect(0, 0, 320, 200);
	SceneExt::postInit();
	_field414 = 390;

	_actor1.postInit();
	_actor1.setup(1575, 1, 1);
	_actor1.setPosition(Common::Point(54, 161));
	_actor1.fixPriority(5);

	_actor2.postInit();
	_actor2.setup(1575, 1, 1);
	_actor2.setPosition(Common::Point(119, 161));
	_actor2.fixPriority(5);

	_actor3.postInit();
	_actor3.setup(1575, 1, 1);
	_actor3.setPosition(Common::Point(184, 161));
	_actor3.fixPriority(5);

	for (int i = 0; i < 17; i++) {
		_arrActor[i].postInit();
		_arrActor[i].setup(1575, 2, k5A7F6[(3 * i) + 2]);
		warning("TODO: immense pile of floating operations");
		_arrActor[i].fixPriority(6);
	}

	_actor4.postInit();
	_actor4.setup(1575, 3, 1);
	_actor4.setPosition(Common::Point(48, 81));

	_actor5.postInit();
	_actor5.setup(1575, 3,1);
	_actor5.setPosition(Common::Point(121, 81));

	_actor6.postInit();
	_actor6.setup(1575, 3, 2);
	_actor6.setPosition(Common::Point(203, 80));

	_actor7.postInit();
	_actor7.setup(1575, 3, 2);
	_actor7.setPosition(Common::Point(217, 80));

	_actor8.postInit();
	_actor8.setup(1575, 3, 2);
	_actor8.setPosition(Common::Point(231, 80));

	_actor9.postInit();
	_actor9.setup(1575, 3, 2);
	_actor9.setPosition(Common::Point(273, 91));

	_actor10.postInit();
	_actor10.setup(1575, 3, 2);
	_actor10.setPosition(Common::Point(287, 91));

	_item1.subA910D(1);
	_item1.subA910D(2);
	_item1.subA910D(3);
	_item1.subA910D(4);
	_item1.subA910D(5);
	_item1.subA910D(6);

	_actor11.postInit();
	_actor11.setup(1575, 4, 2);
	_actor11.setPosition(Common::Point(84, 116));
	_actor11.hide();

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();

	do {
		_field412 = R2_GLOBALS._randomSource.getRandomNumber(20) - 10;
		_field414 = R2_GLOBALS._randomSource.getRandomNumber(20) - 10;
	} while ((_field412) && (_field414));

	if (_field412 < 0)
		_actor4.hide();

	if (_field414 < 0)
		_actor5.hide();

	_field416 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	_field418 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;

	_actor13.postInit();
	_actor13.setup(1575, 2, 4);

	warning("TODO: another immense pile of floating operations");

	_actor12.postInit();
	_actor12.fixPriority(12);

	if (R2_GLOBALS.getFlag(17)) {
		_actor13.setPosition(Common::Point(_actor13._position.x + 5, _actor13._position.y));
		_actor12.setPosition(Common::Point(_actor12._position.x + 5, _actor12._position.y));
	}

	_actor14.postInit();
	_actor14.setup(1575, 5, 1);
	_actor14.setPosition(Common::Point(85, 176));
	_actor14.fixPriority(7);
	_actor14.hide();

	_actor15.postInit();
	_actor15.setup(1575, 5, 2);
	_actor15.setPosition(Common::Point(85, 147));
	_actor15.fixPriority(7);
	_actor15.hide();
}

void Scene1575::remove() {
	SceneExt::remove();
	R2_GLOBALS._v5589E.top = 3;
	R2_GLOBALS._v5589E.bottom = 168;
	R2_GLOBALS._v58CE2 = 1;
}

void Scene1575::signal() {
	R2_GLOBALS._player.enableControl();
}

void Scene1575::process(Event &event) {
	Scene::process(event);

	g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
	warning("TODO: check Scene1575::process");
}

void Scene1575::dispatch() {
	if (_field412 <= 0) {
		++_field412;
		if (_field412 == 0) {
			_actor4.show();
			_field412 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	} else {
		_field412--;
		if (_field412 ==0) {
			_actor4.hide();
			_field412 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	}

	if (_field414 <= 0) {
		++_field414;
		if (_field414 == 0) {
			_actor5.show();
			_field414 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	} else {
		_field414--;
		if (_field414 == 0) {
			_actor5.hide();
			_field414 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	}

	if (_field416 == 0) {
		switch(R2_GLOBALS._randomSource.getRandomNumber(3)) {
		case 0:
			_actor6.hide();
			_actor7.hide();
			_actor8.hide();
			break;
		case 1:
			_actor6.show();
			_actor7.hide();
			_actor8.hide();
			break;
		case 2:
			_actor6.show();
			_actor7.show();
			_actor8.hide();
			break;
		case 3:
			_actor6.show();
			_actor7.show();
			_actor8.show();
			break;
		default:
			break;
		}
		_field416 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	} else {
		--_field416;
	}

	if (_field418 == 0) {
		switch(R2_GLOBALS._randomSource.getRandomNumber(2)) {
		case 0:
			_actor9.hide();
			_actor10.hide();
			break;
		case 1:
			_actor9.show();
			_actor10.hide();
			break;
		case 2:
			_actor9.show();
			_actor10.show();
			break;
		default:
			break;
		}
		_field418 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	} else {
		_field418--;
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1580 - Inside wreck
 *
 *--------------------------------------------------------------------------*/
Scene1580::Scene1580() {
	_field412 = 0;
}

void Scene1580::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene1580::Hotspot1::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	if (action == R2_26) {
		R2_INVENTORY.setObjectScene(26, 1580);
		R2_GLOBALS._sceneItems.remove(&scene->_item1);
		scene->_actor2.postInit();
		scene->_actor2.setup(1580, 1, 4);
		scene->_actor2.setPosition(Common::Point(159, 163));
		scene->_actor2.setDetails(1550, 78, -1, -1, 2, (SceneItem *) NULL);
		
		scene->_arrActor[5].remove();
		
		return true;
	}

	return SceneHotspot::startAction(action, event);
}

bool Scene1580::Hotspot2::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	if (action == R2_28) {
		R2_INVENTORY.setObjectScene(28, 1580);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneItems.remove(&scene->_item2);

		scene->_actor3.postInit();
		scene->_actor3.setup(1580, 1, 1);
		scene->_actor3.setPosition(Common::Point(124, 108));
		scene->_actor3.fixPriority(10);
		
		if (R2_INVENTORY.getObjectScene(26) == 1580)
			scene->_actor3.setDetails(1550, 14, -1, -1, 5, &scene->_actor2);
		else
			scene->_actor3.setDetails(1550, 14, -1, -1, 2, (SceneItem *)NULL);
		
		scene->_actor1.postInit();
		scene->_actor1.setup(1580, 3, 1);
		scene->_actor1.setPosition(Common::Point(124, 109));
		scene->_actor1.fixPriority(20);
		scene->_field412 = 1;
		scene->_sceneMode = 10;
		scene->setAction(&scene->_sequenceManager, scene, 1, &R2_GLOBALS._player, NULL);

		return true;
	}

	return SceneHotspot::startAction(action, event);
}

bool Scene1580::Actor2::startAction(CursorType action, Event &event) {
	if ( (action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(28) == 1580)
		&& (R2_INVENTORY.getObjectScene(17) == 0) && (R2_INVENTORY.getObjectScene(22) == 0)
		&& (R2_INVENTORY.getObjectScene(25) == 0) && (R2_INVENTORY.getObjectScene(18) == 0)
		&& (R2_INVENTORY.getObjectScene(23) == 0) && (R2_INVENTORY.getObjectScene(27) == 0)) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;
		scene->_sceneMode = 31;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->_stripManager.start(536, scene);
		else
			scene->_stripManager.start(537, scene);
		
		return true;
	}

	return SceneActor::startAction(action, event);
}

bool Scene1580::Actor3::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(51) == 1580)) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

		R2_INVENTORY.setObjectScene(51, R2_GLOBALS._player._characterIndex);
		scene->_item2.setDetails(Rect(69, 29, 177, 108), 1550, 82, -1, -1, 2, NULL);
		scene->_actor1.remove();
		remove();
		return true;
	}
	
	return SceneActor::startAction(action, event);
}

bool Scene1580::Actor4::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._sceneItems.remove(&scene->_actor4);
	scene->_sceneMode = 0;
	animate(ANIM_MODE_5, scene);
	
	return true;
}

bool Scene1580::Actor5::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	setFrame(2);
	scene->_sceneMode = 20;
	scene->setAction(&scene->_sequenceManager, scene, 2, &R2_GLOBALS._player, NULL);
	
	return true;
}

bool Scene1580::Actor6::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == 1) {
			R2_INVENTORY.setObjectScene(23, 1);
			remove();
			return true;
		}
		break;
	case R2_9:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(529, scene);
		return true;
		break;
	case R2_39:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(527, scene);
		return true;
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene1580::Actor7::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == 1) {
			R2_INVENTORY.setObjectScene(27, 1);
			remove();
			return true;
		}
		break;
	case R2_9:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(529, scene);
		return true;
		break;
	case R2_39:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(527, scene);
		return true;
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

void Scene1580::postInit(SceneObjectList *OwnerList) {
	loadScene(1580);
	R2_GLOBALS._sceneManager._fadeMode = FADEMODE_GRADUAL;
	SceneExt::postInit();
	_field412 = 0;

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	
	_sceneMode = 0;
	
	R2_GLOBALS._player.disableControl();
	if (R2_INVENTORY.getObjectScene(26) == 1580) {
		_actor2.postInit();
		_actor2.setup(1580, 1, 4);
		_actor2.setPosition(Common::Point(159, 163));
		_actor2.setDetails(1550, 78, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_item1.setDetails(Rect(141, 148, 179, 167), 1550, 79, -1, -1, 1, NULL);
	}
	
	if (R2_INVENTORY.getObjectScene(51) == 1580) {
		_actor3.postInit();
		_actor3.setup(1580, 1, 1);
		_actor3.setPosition(Common::Point(124, 108));
		_actor3.fixPriority(10);
		_actor3.setDetails(1550, 13, -1, -1, 1, (SceneItem *) NULL);
		
		_actor1.postInit();
		_actor1.setup(1580, 1, 3);
		_actor1.setPosition(Common::Point(124, 96));
		_actor1.fixPriority(20);
	} else if (R2_INVENTORY.getObjectScene(28) == 1580) {
		_actor3.postInit();
		_actor3.setup(1580, 1, 1);
		_actor3.setPosition(Common::Point(124, 108));
		_actor3.fixPriority(10);
		_actor3.setDetails(1550, 14, -1, -1, 1, (SceneItem *) NULL);

		_actor1.postInit();
		_actor1.setup(1580, 3, 1);
		_actor1.setPosition(Common::Point(124, 109));
		_actor1.fixPriority(20);
			
		_sceneMode = 10;
	} else {
		_item2.setDetails(Rect(69, 29, 177, 108), 1550, 82, -1, -1, 1, NULL);
	}

	_actor4.postInit();
	if (R2_INVENTORY.getObjectScene(58) == 0) {
		_actor4.setup(1580, 5, 1);
		_actor4.setDetails(1550, 80, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_actor4.setup(1580, 5, 6);
	}

	_actor4.setPosition(Common::Point(216, 108));
	_actor4.fixPriority(100);
	
	_actor5.postInit();
	_actor5.setup(1580, 4, 1);
	_actor5.setPosition(Common::Point(291, 147));
	_actor5.fixPriority(100);
	_actor5.setDetails(1550, 81, -1, -1, 1, (SceneItem *) NULL);

	if (R2_INVENTORY.getObjectScene(23) == 1580) {
		_actor6.postInit();
		_actor6.setup(1580, 6, 2);
		_actor6.setPosition(Common::Point(222, 108));
		_actor6.fixPriority(50);
		_actor6.setDetails(1550, 32, -1, 34, 1, (SceneItem *) NULL);
	}
	
	if (R2_INVENTORY.getObjectScene(27) == 1580) {
		_actor7.postInit();
		_actor7.setup(1580, 6, 1);
		_actor7.setPosition(Common::Point(195, 108));
		_actor7.fixPriority(50);
		_actor7.setDetails(1550, 38, -1, 34, 1, (SceneItem *) NULL);
	}
	
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1580;
	R2_GLOBALS._player.hide();
	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
	_item3.setDetails(Rect(0, 0, 320, 200), 1550, 50, -1, -1, 1, NULL);
	
}

void Scene1580::signal() {
	switch (_sceneMode++) {
	case 10:
		_actor1.animate(ANIM_MODE_5, this);
		break;
	case 11:
		_actor1.setup(1580, 1, 2);
		_actor1.setPosition(Common::Point(124, 94));

		if (R2_INVENTORY.getObjectScene(18) != 0) {
			_arrActor[0].postInit();
			_arrActor[0].setup(1580, 2, 1);
			_arrActor[0].setPosition(Common::Point(138, 56));
		}
		
		if (R2_INVENTORY.getObjectScene(25) != 0) {
			_arrActor[1].postInit();
			_arrActor[1].setup(1580, 2, 2);
			_arrActor[1].setPosition(Common::Point(140, 66));
		}

		if (R2_INVENTORY.getObjectScene(27) != 0) {
			_arrActor[2].postInit();
			_arrActor[2].setup(1580, 2, 3);
			_arrActor[2].setPosition(Common::Point(142, 85));
		}

		if (R2_INVENTORY.getObjectScene(23) != 0) {
			_arrActor[3].postInit();
			_arrActor[3].setup(1580, 2, 4);
			_arrActor[3].setPosition(Common::Point(142, 92));
		}

		if (R2_INVENTORY.getObjectScene(22) != 0) {
			_arrActor[4].postInit();
			_arrActor[4].setup(1580, 2, 5);
			_arrActor[4].setPosition(Common::Point(108, 54));
		}

		if (R2_INVENTORY.getObjectScene(26) != 0) {
			_arrActor[5].postInit();
			_arrActor[5].setup(1580, 2, 6);
			_arrActor[5].setPosition(Common::Point(110, 64));
		}

		if (R2_INVENTORY.getObjectScene(45) != 0) {
			_arrActor[6].postInit();
			_arrActor[6].setup(1580, 2, 7);
			_arrActor[6].setPosition(Common::Point(108, 80));
		}

		if (R2_INVENTORY.getObjectScene(17) != 0) {
			_arrActor[7].postInit();
			_arrActor[7].setup(1580, 2, 8);
			_arrActor[7].setPosition(Common::Point(111, 92));
		}

		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 20:
		R2_GLOBALS._sceneManager.changeScene(1550);
		break;
	case 31:
		R2_GLOBALS._sceneManager.changeScene(1530);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1625 - Miranda being questioned
 *
 *--------------------------------------------------------------------------*/
Scene1625::Scene1625() {
	_field412 = 0;
}

void Scene1625::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene1625::Actor7::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();

	scene->_sceneMode = 1631;
	scene->_actor3.postInit();
	scene->setAction(&scene->_sequenceManager, scene, 1631, &scene->_actor3, &scene->_actor7, NULL);
	return true;
}

void Scene1625::postInit(SceneObjectList *OwnerList) {
	loadScene(1625);
	R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_soldierSpeaker);

	R2_GLOBALS._player.postInit();

	_actor7.postInit();
	_actor7.setup(1626, 2, 1);
	_actor7.setPosition(Common::Point(206, 133));
	_actor7.setDetails(1625, 0, -1, -1, 1, (SceneItem *) NULL);

	_actor5.postInit();
	_actor5.setup(1625, 8, 1);
	_actor5.setPosition(Common::Point(190, 131));
	_actor5.setDetails(1625, 6, -1, 2, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._player._oldCharacterScene[3] == 1625) {
		if (!R2_GLOBALS.getFlag(83)) {
			_actor4.postInit();
			_actor4.setup(1626, 4, 1);
			_actor4.setPosition(Common::Point(96, 166));
			_actor4.setDetails(1625, -1, -1, -1, 1, (SceneItem *) NULL);
		}
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
	} else {
		_actor1.postInit();
		_actor1.fixPriority(10);

		_actor6.postInit();

		R2_GLOBALS._player.disableControl();
		_sceneMode = 1625;
		setAction(&_sequenceManager, this, 1625, &_actor1, &_actor6, NULL);
	}

	R2_GLOBALS._sound1.play(245);
	_item1.setDetails(Rect(0, 0, 320, 200), 1625, 12, -1, -1, 1, NULL);
	R2_GLOBALS._player._oldCharacterScene[3] = 1625;
	R2_GLOBALS._player._characterScene[3] = 1625;
}

void Scene1625::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1625::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.disableControl();
		_actor4.postInit();
		_actor4.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
		_sceneMode = 1626;
		setAction(&_sequenceManager, this, 1626, &_actor2, &_actor4, NULL);
		break;
	case 12:
		// TODO: check if OK_BTN_STRING is required
		MessageDialog::show(DONE_MSG, OK_BTN_STRING);
		break;
	case 14:
		_actor2.postInit();
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(831, this);
		break;
	case 99:
		R2_GLOBALS._player.disableControl();
		switch (_stripManager._field2E8) {
		case 0:
			_sceneMode = 1627;
			setAction(&_sequenceManager, this, 1627, &_actor3, &_actor4, NULL);
			break;
		case 1:
			_sceneMode = 1629;
			setAction(&_sequenceManager, this, 1629, &_actor2, &_actor5, NULL);
			break;
		case 3:
			R2_GLOBALS._player._oldCharacterScene[3] = 3150;
			R2_GLOBALS._player._characterScene[3] = 3150;
			R2_GLOBALS._player._characterIndex = R2_QUINN;
			R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._characterScene[1]);
			break;
		case 4:
			_sceneMode = 1628;
			_actor2.remove();
			setAction(&_sequenceManager, this, 1628, &_actor3, &_actor4, NULL);
			break;
		case 5:
			_actor4.postInit();
			_actor4.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
			_sceneMode = 1632;
			setAction(&_sequenceManager, this, 1632, &_actor4, NULL);
			break;
		case 6:
			_sceneMode = 1633;
			setAction(&_sequenceManager, this, 1633, &_actor4, NULL);
			break;
		case 7:
			_sceneMode = 1635;
			setAction(&_sequenceManager, this, 1635, &_actor3, &_actor5, NULL);
			break;
		case 8:
			_actor4.postInit();
			_actor4.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
			_sceneMode = 1634;
			setAction(&_sequenceManager, this, 1634, &_actor3, &_actor5, NULL);
			break;
		case 2:
		// No break on purpose
		default:
			_sceneMode = 1630;
			_actor2.postInit();
			setAction(&_sequenceManager, this, 1630, &_actor1, &_actor6, NULL);
			break;
		}
		_field412 = _stripManager._field2E8;
		_stripManager._field2E8 = 0;
		break;
	case 1625:
		_actor2.postInit();
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));
		_sceneMode = 10;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(800, this);
		break;
	case 1626:
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));
		_actor2.show();

		_actor3.postInit();
		_actor3.setup(1627, 3, 1);
		_actor3.setPosition(Common::Point(196, 65));

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(832, this);
		break;
	case 1627:
		_actor3.setup(1627, 3, 1);
		_actor3.setPosition(Common::Point(196, 65));
		_actor3.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(833, this);
		break;
	case 1628:
		R2_GLOBALS.setFlag(83);
		_actor2.postInit();
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));

		_actor3.setup(1627, 3, 1);
		_actor3.setPosition(Common::Point(196, 65));
		_actor3.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(834, this);
		break;
	case 1629:
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));
		_actor2.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(805, this);
		break;
	case 1630:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = true;
		break;
	case 1631:
		_actor3.setup(1627, 3, 1);
		_actor3.setPosition(Common::Point(196, 65));
		_actor3.show();

		_actor7.remove();

		_actor1.postInit();
		_actor1.fixPriority(10);

		_actor6.postInit();
		warning("_actor6._actorName = \"arm\";");

		R2_INVENTORY.setObjectScene(40, 3);
		_sceneMode = 14;

		setAction(&_sequenceManager, this, 1625, &_actor1, &_actor6, NULL);
		break;
	case 1632:
		_actor2.setup(1627, 1, 1);
		_actor2.setPosition(Common::Point(68, 68));
		_actor2.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(835, this);
		break;
	case 1633:
		_actor4.remove();
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(818, this);
		break;
	case 1634:
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(836, this);
		break;
	case 1635:
		_actor3.setup(1627, 3, 1);
		_actor3.setPosition(Common::Point(196, 65));
		_actor3.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(818, this);
		break;
	default:
		break;
	}
}

void Scene1625::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE))
		event.handled = true;
	else
		Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 1700 - 
 *
 *--------------------------------------------------------------------------*/
Scene1700::Scene1700() {
	_field77A = 0;
	_field77C = 0;
}

void Scene1700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field77A);
	s.syncAsSint16LE(_field77C);
}

bool Scene1700::Item2::startAction(CursorType action, Event &event) {
	// The original contains a debug trace. It's currently skipped.
	// TODO: either add the debug trace, or remove this function and associated class
	return SceneHotspot::startAction(action, event);
}

bool Scene1700::Actor11::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._v558B6.set(80, 0, 240, 200);
	scene->_sceneMode = 4;

	Common::Point pt(271, 90);
	PlayerMover *mover = new PlayerMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

bool Scene1700::Actor12::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneMode = 30;
	scene->signal();

	return true;
}

void Scene1700::Exit1::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 1;

	Common::Point pt(R2_GLOBALS._player._position.x, 0);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::Exit2::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 2;

	Common::Point pt(R2_GLOBALS._player._position.x, 170);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::Exit3::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 6;

	Common::Point pt(0, R2_GLOBALS._player._position.y);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::subAF3F8() {
	Rect tmpRect;
	R2_GLOBALS._walkRegions.load(1700);
	
	_actor3.remove();
	_actor4.remove();
	_actor5.remove();
	_actor6.remove();
	_actor7.remove();
	_actor8.remove();
	_actor11.remove();
	
	if (_sceneMode != 40) {
		_actor9.remove();
		_actor10.remove();
	}
	
	warning("tmpRect = _v5589E;");
	warning("Mouse_hideIfNeeded");
	warning("set_pane_p(_paneNumber);");
	warning("Big loop calling gfx_draw_slice_p");
	
	if (_field77A == 0)
		_field77A = 1;
	else
		_field77A = 0;
	
	warning("set_pane_p(_paneNumber);");

	if ((_sceneMode != 40) && (R2_GLOBALS._v565F6 != 0)){
		_actor9.postInit();
		_actor9.setup(1701, 1, 1);
		_actor9.setPosition(Common::Point(220, 137));
		_actor9.setDetails(1700, 6, -1, -1, 2, (SceneItem *) NULL);
		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(12);
	} 
	
	if ((R2_GLOBALS._v565F6 + 2) % 4 == 0) {
		_actor3.postInit();
		_actor3.setup(1700, 1, 1);
		_actor3.setPosition(Common::Point(222, 82));
		_actor3.setDetails(100, -1, -1, -1, 2, (SceneItem *) NULL);
		
		_actor5.postInit();
		_actor5.setup(1700, 2, 1);
		_actor5.setPosition(Common::Point(177, 82));
		_actor5.fixPriority(0);
		
		_actor6.postInit();
		_actor6.setup(1700, 2, 2);
		_actor6.setPosition(Common::Point(332, 96));
		_actor6.fixPriority(0);
	
		_actor4.postInit();
		_actor4.setup(1700, 1, 2);
		_actor4.setPosition(Common::Point(424, 84));
		
		R2_GLOBALS._walkRegions.enableRegion(11);
	}
	
	if ((R2_GLOBALS._v565F6 + 399) % 800 == 0) {
		_actor7.postInit();
		_actor7.setup(1700, 3, 2);
		_actor7.setPosition(Common::Point(51, 141));
		_actor7.fixPriority(0);
		_actor7.setDetails(100, -1, -1, -1, 2, (SceneItem *) NULL);
		
		_exit3._enabled = true;
	} else {
		R2_GLOBALS._walkRegions.enableRegion(1);
		_exit3._enabled = false;
	}
	
	if (  ((!R2_GLOBALS.getFlag(15)) && ((R2_GLOBALS._v565F6 == 25) || (R2_GLOBALS._v565F6 == -3))) 
		 || ((R2_GLOBALS.getFlag(15)) && (R2_GLOBALS._v565F6 == R2_GLOBALS._v565FA))
		 ) {
		R2_GLOBALS._v565FA = R2_GLOBALS._v565F6;
		if (!R2_GLOBALS.getFlag(15))
			_field77C = 1;
		
		_actor11.postInit();
		_actor11.setup(1700, 3, 1);
		_actor11.setPosition(Common::Point(338, 150));
		_actor11.setDetails(1700, 9, -1, -1, 2, (SceneItem *) NULL);
		_actor11.fixPriority(15);
		
		_actor8.postInit();
		_actor8.setup(1700, 4, 1);
		_actor8.setPosition(Common::Point(312, 106));
		_actor8.fixPriority(130);
	}
}

void Scene1700::postInit(SceneObjectList *OwnerList) {
	loadScene(1700);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1530;
	
	scalePalette(65, 65, 65);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_field77A = 0;
	_field77C = 0;
	
	_exit1.setDetails(Rect(94, 0, 319, 12), EXITCURSOR_N, 1700);
	_exit2.setDetails(Rect(0, 161, 319, 168), EXITCURSOR_S, 1700);
	_exit3.setDetails(Rect(0, 0, 2, 138), EXITCURSOR_W, 1800);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setPosition(Common::Point(0, 0));
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(1501);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
	} else {
		R2_GLOBALS._player.setVisage(1506);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 1);
	}
	
	_actor12.postInit();
	_actor12.animate(ANIM_MODE_1, NULL);
	_actor12.setObjectWrapper(new SceneObjectWrapper());
	
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		_actor12.setVisage(1506);
		_actor12._moveDiff = Common::Point(3, 1);
		_actor12.setDetails(9002, 1, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_actor12.setVisage(1501);
		_actor12._moveDiff = Common::Point(2, 1);
		_actor12.setDetails(9001, 1, -1, -1, 1, (SceneItem *) NULL);
	}
	
	R2_GLOBALS._sound1.play(134);
	
	_actor1.postInit();
	_actor1.fixPriority(10);
	
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor1.setVisage(1112);
	else 
		_actor1.setVisage(1111);
	
	_actor1._effect = 5;
	_actor1._field9C = _field312;
	R2_GLOBALS._player._linkedActor = &_actor1;
	
	_actor2.postInit();
	_actor2.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor2.setVisage(1111);
	else 
		_actor2.setVisage(1112);
	
	_actor2._effect = 5;
	_actor2._field9C = _field312;
	_actor12._linkedActor = &_actor2;
	
	R2_GLOBALS._sound1.play(134);
	
	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1530:
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();
		_actor12.hide();
		
		_actor10.postInit();
		warning("_actor10._actorName = \"hatch\";");
		_actor10.hide();

		_actor9.postInit();
		_actor9.setup(1701, 1, 1);
		_actor9.setPosition(Common::Point(220, 137));
		_actor9.setDetails(1700, 6, -1, -1, 1, (SceneItem *) NULL);
		
		_actor1.hide();
		_actor2.hide();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(539, this);
		_sceneMode = 40;
		break;
	case 1750: {
		R2_GLOBALS._player.setPosition(Common::Point(282, 121));
		_actor12.setPosition(Common::Point(282, 139));
		_sceneMode = 8;
		Common::Point pt(262, 101);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		Common::Point pt2(262, 119);
		NpcMover *mover2 = new NpcMover();
		_actor12.addMover(mover2, &pt2, this);
		}
		break;
	case 1800: {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.setPosition(Common::Point(0, 86));
		_actor12.setPosition(Common::Point(0, 64));
		_sceneMode = 7;
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player._strip = 1;
		Common::Point pt(64, 86);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		_actor12.setObjectWrapper(NULL);
		_actor12._strip = 1;
		Common::Point pt2(77, 64);
		NpcMover *mover2 = new NpcMover();
		_actor12.addMover(mover2, &pt2, NULL);
		}
		break;
	default:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setPosition(Common::Point(109, 160));
			_actor12.setPosition(Common::Point(156, 160));
			R2_GLOBALS._walkRegions.enableRegion(15);
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(156, 160));
			_actor12.setPosition(Common::Point(109, 160));
			R2_GLOBALS._walkRegions.enableRegion(17);
		}
		_sceneMode = 50;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	}
	R2_GLOBALS._player._characterScene[1] = 1700;
	R2_GLOBALS._player._characterScene[2] = 1700;
	R2_GLOBALS._player._oldCharacterScene[1] = 1700;
	R2_GLOBALS._player._oldCharacterScene[2] = 1700;

	R2_GLOBALS._v558B6.set(20, 0, 320, 200);
	subAF3F8();
	_item1.setDetails(1, 1700, 3, -1, -1);
	_item2.setDetails(Rect(0, 0, 480, 200), 1700, 0, -1, -1, 1, NULL);
}

void Scene1700::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1700::signal() {
	switch (_sceneMode) {
	case 1: {
		_sceneMode = 3;
		if ((R2_GLOBALS._v565F6 < 2400) && (R2_GLOBALS._v565F6 >= 0))
			++R2_GLOBALS._v565F6;
		subAF3F8();
		R2_GLOBALS._player.setPosition(Common::Point(235 - (((((235 - R2_GLOBALS._player._position.x) * 100) / 103) * 167) / 100), 170));
		Common::Point pt(R2_GLOBALS._player._position.x, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		
		if (R2_GLOBALS._player._position.x < 132) {
			_actor12.setPosition(Common::Point(156, 170));
			Common::Point pt2(156, 160);
			NpcMover *mover2 = new NpcMover();
			_actor12.addMover(mover2, &pt2, NULL);
			R2_GLOBALS._walkRegions.enableRegion(15);
		} else {
			_actor12.setPosition(Common::Point(109, 170));
			Common::Point pt3(109, 160);
			NpcMover *mover3 = new NpcMover();
			_actor12.addMover(mover3, &pt3, NULL);
			R2_GLOBALS._walkRegions.enableRegion(17);
		}
		}
		break;
	case 2: {
		_sceneMode = 3;
		if ((R2_GLOBALS._v565F6 > -2400) && (R2_GLOBALS._v565F6 < 0))
			R2_GLOBALS._v565F6--;
		subAF3F8();
		R2_GLOBALS._player.setPosition(Common::Point(235 - (((((235 - R2_GLOBALS._player._position.x) * 100) / 167) * 103) / 100), 0));
		Common::Point pt(R2_GLOBALS._player._position.x, 10);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		
		if (R2_GLOBALS._player._position.x >= 171) {
			_actor12.setPosition(Common::Point(155, 0));
			Common::Point pt2(155, 10);
			NpcMover *mover2 = new NpcMover();
			_actor12.addMover(mover2, &pt2, NULL);
			R2_GLOBALS._walkRegions.enableRegion(15);
		} else {
			_actor12.setPosition(Common::Point(188, 0));
			Common::Point pt3(188, 10);
			NpcMover *mover3 = new NpcMover();
			_actor12.addMover(mover3, &pt3, NULL);
			R2_GLOBALS._walkRegions.enableRegion(17);
		}
		}
		break;
	case 3:
		if (_field77C == 0) {
			R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		} else {
			R2_GLOBALS.setFlag(15);
			_field77C = 0;
			_sceneMode = 31;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_stripManager.start(542, this);
			else
				_stripManager.start(543, this);
		}
		break;
	case 4: {
		_sceneMode = 5;
		Common::Point pt(271, 90);
		PlayerMover *mover = new PlayerMover();
		_actor12.addMover(mover, &pt, NULL);
		if (R2_GLOBALS._player._characterIndex == 1)
			setAction(&_sequenceManager, this, 1700, &R2_GLOBALS._player, &_actor8, NULL);
		else
			setAction(&_sequenceManager, this, 1701, &R2_GLOBALS._player, &_actor8, NULL);
		}
		break;
	case 5:
		R2_GLOBALS._sceneManager.changeScene(1750);
		break;
	case 6:
		R2_GLOBALS._sceneManager.changeScene(1800);
		break;
	case 7:
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 1;
		_actor12.setObjectWrapper(new SceneObjectWrapper());
		_actor12._strip = 1;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		R2_GLOBALS._walkRegions.enableRegion(14);
		break;
	case 8:
		R2_GLOBALS._player._strip = 2;
		_actor12._strip = 1;
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		R2_GLOBALS._walkRegions.enableRegion(12);
		break;
	case 30:
		_sceneMode = 31;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_stripManager.start(540, this);
		else
			_stripManager.start(541, this);
		break;
	case 31:
		R2_GLOBALS._v56AAB = 0;
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 40:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1704;
		setAction(&_sequenceManager, this, 1704, &R2_GLOBALS._player, &_actor12, &_actor10, &_actor9, &_actor1, &_actor2, NULL);
		break;
	case 50:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			R2_GLOBALS._walkRegions.enableRegion(15);
		else
			R2_GLOBALS._walkRegions.enableRegion(17);
		
		R2_GLOBALS._player.enableControl();
		break;
	case 1704:
		R2_GLOBALS._sound1.play(134);
		R2_GLOBALS._walkRegions.enableRegion(15);
		R2_GLOBALS._walkRegions.enableRegion(2);
		R2_GLOBALS._walkRegions.enableRegion(12);
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1800 - 
 *
 *--------------------------------------------------------------------------*/
Scene1800::Scene1800() {
	_field412 = 0;
}

void Scene1800::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
}

bool Scene1800::Hotspot5::startAction(CursorType action, Event &event) {
	if ((action != R2_9) && (action != R2_39))
		return false;
	
	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);	

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS._v565F6 == 1201) {
			scene->_stripManager.start(548, this);
		} else if (R2_GLOBALS.getFlag(66)) {
			return false;
		} else {
			scene->_stripManager.start(546, this);
		}
	} else {
		if (R2_GLOBALS._v565F6 == 1201) {
			scene->_stripManager.start(549, this);
		} else if (R2_GLOBALS.getFlag(66)) {
			return false;
		} else {
			scene->_stripManager.start(547, this);
		}
	}

	R2_GLOBALS.setFlag(66);
	return true;
}

bool Scene1800::Actor6::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (!R2_GLOBALS.getFlag(14))
		return false;

	if (R2_GLOBALS._player._characterIndex != R2_QUINN)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();

	if (_frame == 1) {
		R2_GLOBALS.setFlag(64);
		scene->_sceneMode = 1810;
		scene->setAction(&scene->_sequenceManager, scene, 1810, &R2_GLOBALS._player, &scene->_actor6, &scene->_actor4, &scene->_actor5, NULL);
	} else {
		R2_GLOBALS.clearFlag(64);
		scene->_sceneMode = 1811;
		scene->setAction(&scene->_sequenceManager, scene, 1811, &R2_GLOBALS._player, &scene->_actor6, &scene->_actor4, &scene->_actor5, NULL);
	}
	return true;
}

bool Scene1800::Actor7::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		R2_GLOBALS._player.disableControl();
		if (scene->_field412 >= 2) {
			if (R2_GLOBALS.getFlag(14)) {
				scene->_sceneMode = 1809;
				scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor7, NULL);
				R2_GLOBALS.clearFlag(14);
			} else {
				scene->_sceneMode = 1808;
				scene->setAction(&scene->_sequenceManager, scene, 1808, &R2_GLOBALS._player, &scene->_actor7, NULL);
				R2_GLOBALS.setFlag(14);
			}
		} else {
			scene->_sceneMode = 1813;
			scene->setAction(&scene->_sequenceManager, scene, 1813, &R2_GLOBALS._player, NULL);
		}
	} else if (R2_GLOBALS.getFlag(14)) {
		return SceneActor::startAction(action, event);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 1812;
		scene->setAction(&scene->_sequenceManager, scene, 1812, &R2_GLOBALS._player, NULL);
	}

	return true;
}

bool Scene1800::Actor8::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	if (_position.x < 160) {
		if (scene->_actor4._frame == 1) {
			return SceneActor::startAction(action, event);
		} else {
			R2_GLOBALS.setFlag(29);
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 1804;
					scene->setAction(&scene->_sequenceManager, scene, 1804, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor8, NULL);
				} else {
					scene->_sceneMode = 1;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor7, NULL);
					R2_GLOBALS.clearFlag(14);
				}
			} else {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 1;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor7, NULL);
					R2_GLOBALS.clearFlag(14);
				} else {
					scene->_sceneMode = 1805;
					scene->setAction(&scene->_sequenceManager, scene, 1805, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor8, NULL);
				}
			}
		}
	} else {
		if (scene->_actor4._frame == 1) {
			return SceneActor::startAction(action, event);
		} else {
			R2_GLOBALS.clearFlag(29);
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 2;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor7, NULL);
					R2_GLOBALS.clearFlag(14);
				} else {
					scene->_sceneMode = 1806;
					scene->setAction(&scene->_sequenceManager, scene, 1806, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor9, NULL);
				}
			} else {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 2;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor7, NULL);
					R2_GLOBALS.clearFlag(14);
				} else {
					scene->_sceneMode = 1807;
					scene->setAction(&scene->_sequenceManager, scene, 1807, &R2_GLOBALS._player, &scene->_actor2, &scene->_actor9, NULL);
				}
			}
		}
	}

	return true;
}

void Scene1800::Exit1::changeScene() {
	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS.getFlag(14)) {
		scene->_sceneMode = 3;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_actor7, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1809, &scene->_actor2, &scene->_actor7, NULL);
		R2_GLOBALS.clearFlag(14);
	} else {
		scene->_sceneMode = 1802;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 1802, &R2_GLOBALS._player, &scene->_actor2, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1802, &R2_GLOBALS._player, &scene->_actor2, NULL);
	}
}

void Scene1800::postInit(SceneObjectList *OwnerList) {
	loadScene(1800);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(116);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	
	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._v565F6 = 1201;
	
	if (R2_GLOBALS._v565F6 == 1201)
		_field412 = 2;
	else
		_field412 = 0;
	
	scalePalette(65, 65, 65);
	_exit1.setDetails(Rect(0, 160, 319, 168), EXITCURSOR_S, 1800);
	_item5.setDetails(Rect(0, 0, 320, 200), -1, -1, -1, -1, 1, NULL);
	
	_actor6.postInit();
	_actor6.setup(1801, 4, 1);
	_actor6.setPosition(Common::Point(170, 24));
	_actor6.setDetails(1800, 13, 14, 15, 1, (SceneItem *) NULL);
	
	_actor7.postInit();
	_actor7.setup(1801, 3, 1);
	_actor7.setPosition(Common::Point(160, 139));
	_actor7.setDetails(1800, 6, -1, -1, 1, (SceneItem *) NULL);
	
	_actor8.postInit();
	_actor8.setup(1800, 1, 1);
	_actor8.setPosition(Common::Point(110, 78));
	_actor8.fixPriority(135);
	_actor8.setDetails(1800, 20, -1, -1, 1, (SceneItem *) NULL);
	
	_actor9.postInit();
	_actor9.setup(1800, 2, 1);
	_actor9.setPosition(Common::Point(209, 78));
	_actor9.fixPriority(135);
	_actor9.setDetails(1800, 20, -1, -1, 1, (SceneItem *) NULL);
	
	_actor4.postInit();
	if ((_field412 != 1) && (_field412 != 3) && (!R2_GLOBALS.getFlag(64)))
		_actor4.setup(1801, 2, 1);
	else
		_actor4.setup(1801, 2, 10);
	_actor4.setPosition(Common::Point(76, 142));
	_actor4.setDetails(1800, 3, -1, -1, 1, (SceneItem *) NULL);
	
	_actor5.postInit();
	if ((_field412 != 1) && (_field412 != 3) && (!R2_GLOBALS.getFlag(64)))
		_actor5.setup(1801, 1, 1);
	else
		_actor5.setup(1801, 1, 10);
	_actor5.setPosition(Common::Point(243, 142));
	_actor5.setDetails(1800, 3, -1, -1, 1, (SceneItem *) NULL);
	
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS.getFlag(14)) {
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			R2_GLOBALS._player.setObjectWrapper(NULL);
			R2_GLOBALS._player.setup(1801, 5, 12);
			R2_GLOBALS._player.setPosition(Common::Point(160, 139));
			R2_GLOBALS._walkRegions.enableRegion(9);
			_actor7.hide();
		} else {
			R2_GLOBALS._player.setVisage(1507);
		}
		R2_GLOBALS._player._moveDiff = Common::Point(4, 2);
	} else {
		R2_GLOBALS._player.setVisage(1503);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	}
	
	_actor2.postInit();
	_actor2.animate(ANIM_MODE_1, NULL);
	_actor2.setObjectWrapper(new SceneObjectWrapper());
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS.getFlag(14)) {
			_actor2.animate(ANIM_MODE_NONE, NULL);
			_actor2.setObjectWrapper(NULL);
			_actor2.setup(1801, 5, 12);

			R2_GLOBALS._walkRegions.enableRegion(9);
			_actor7.hide();
		} else {
			_actor2.setup(1507, 1, 1);
			_actor2.setPosition(Common::Point(180, 160));
		}
		_actor2.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		_actor2._moveDiff = Common::Point(4, 2);
	} else {
		_actor2.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);
		_actor2.setVisage(1503);
		_actor2._moveDiff = Common::Point(2, 2);
	}
	
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1800) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setPosition(Common::Point(114, 150));
			R2_GLOBALS._player.setStrip(5);
			if (R2_GLOBALS.getFlag(14)) {
				_actor2.setPosition(Common::Point(160, 139));
				R2_GLOBALS._walkRegions.enableRegion(8);
			} else {
				_actor2.setPosition(Common::Point(209, 150));
				_actor2.setStrip(6);
				R2_GLOBALS._walkRegions.enableRegion(8);
			}
		} else {
			if (R2_GLOBALS.getFlag(14)) {
				R2_GLOBALS._player.setup(1801, 5, 12);
				R2_GLOBALS._player.setPosition(Common::Point(160, 139));
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(209, 150));
				R2_GLOBALS._player.setStrip(6);
			}
			_actor2.setPosition(Common::Point(114, 150));
			_actor2.setStrip(5);
			R2_GLOBALS._walkRegions.enableRegion(10);
			R2_GLOBALS._walkRegions.enableRegion(11);
		}
	} else {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setPosition(Common::Point(140, 160));
			_actor2.setPosition(Common::Point(180, 160));
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(180, 160));
			_actor2.setPosition(Common::Point(140, 160));
		}
	}
	
	_actor1.postInit();
	_actor1.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor1.setVisage(1111);
	else
		_actor1.setVisage(1110);
	
	_actor1._effect = 5;
	_actor1._field9C = _field312;

	R2_GLOBALS._player._linkedActor = &_actor1;
	
	_actor3.postInit();
	_actor3.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor3.setVisage(1110);
	else
		_actor3.setVisage(1111);
	
	_actor3._effect = 5;
	_actor3._field9C = _field312;
	
	_actor2._linkedActor = &_actor3;
	
	R2_GLOBALS._player._characterScene[1] = 1800;
	R2_GLOBALS._player._characterScene[2] = 1800;
	
	_item2.setDetails(Rect(128, 95, 190, 135), 1800, 10, -1, -1, 1, NULL);
	_item1.setDetails(Rect(95, 3, 223, 135), 1800, 0, -1, -1, 1, NULL);

	// Original was calling _item3.setDetails(Rect(1800, 11, 24, 23), 25, -1, -1, -1, 1, NULL);
	// This is *wrong*. The following statement is a wild guess based on good common sense
	_item3.setDetails(11, 1800, 23, 24, 25);
	_item4.setDetails(Rect(0, 0, 320, 200), 1800, 17, -1, 19, 1, NULL);
	
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1800) {
		if ((R2_GLOBALS.getFlag(14)) && (R2_GLOBALS._player._characterIndex == R2_SEEKER)) {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		}
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1850) {
		if (R2_GLOBALS.getFlag(29)) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				_sceneMode = 1814;
				setAction(&_sequenceManager, this, 1814, &R2_GLOBALS._player, &_actor2, &_actor8, NULL);
			} else {
				_sceneMode = 1815;
				setAction(&_sequenceManager, this, 1815, &R2_GLOBALS._player, &_actor2, &_actor8, NULL);
			}
		} else {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				_sceneMode = 1816;
				setAction(&_sequenceManager, this, 1816, &R2_GLOBALS._player, &_actor2, &_actor9, NULL);
			} else {
				_sceneMode = 1817;
				setAction(&_sequenceManager, this, 1817, &R2_GLOBALS._player, &_actor2, &_actor9, NULL);
			}
		}
	} else {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1800;
			setAction(&_sequenceManager, this, 1800, &R2_GLOBALS._player, &_actor2, NULL);
		} else {
			_sceneMode = 1801;
			setAction(&_sequenceManager, this, 1801, &R2_GLOBALS._player, &_actor2, NULL);
		}
	}
	
	R2_GLOBALS._player._oldCharacterScene[1] = 1800;
	R2_GLOBALS._player._oldCharacterScene[2] = 1800;
}

void Scene1800::signal() {
	switch (_sceneMode) {
	case 1:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1804;
			setAction(&_sequenceManager, this, 1804, &R2_GLOBALS._player, &_actor2, &_actor8, NULL);
		} else {
			_sceneMode = 1805;
			setAction(&_sequenceManager, this, 1805, &R2_GLOBALS._player, &_actor2, &_actor8, NULL);
		}
		break;
	case 2:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1806;
			setAction(&_sequenceManager, this, 1806, &R2_GLOBALS._player, &_actor2, &_actor9, NULL);
		} else {
			_sceneMode = 1807;
			setAction(&_sequenceManager, this, 1807, &R2_GLOBALS._player, &_actor2, &_actor9, NULL);
		}
		break;
	case 3:
		_sceneMode = 1802;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			setAction(&_sequenceManager, this, 1802, &R2_GLOBALS._player, &_actor2, NULL);
		else
			setAction(&_sequenceManager, this, 1803, &R2_GLOBALS._player, &_actor2, NULL);
		break;
	case 10:
	// No break on purpose
	case 11:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 12:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 13:
		_sceneMode = 14;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS._player.setup(1801, 7, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_8, NULL);
		_stripManager.start(550, this);
		break;
	case 14:
		_sceneMode = 15;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.setup(1801, 6, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 15:
		R2_GLOBALS._player.setup(1503, 4, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	case 1800:
		R2_GLOBALS._walkRegions.enableRegion(8);
		if (R2_GLOBALS.getFlag(63))
			R2_GLOBALS._player.enableControl(CURSOR_USE);
		else {
			_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_stripManager.start(544, this);
		}
		break;
	case 1801:
		R2_GLOBALS._walkRegions.enableRegion(10);
		R2_GLOBALS._walkRegions.enableRegion(11);
		R2_GLOBALS.setFlag(63);

		// The following check is completely dumb.
		// Either an original bug, or dead code.
		if (R2_GLOBALS.getFlag(63)) {
			R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		} else {
			_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_stripManager.start(545, this);
		}
		break;
	case 1802:
		R2_GLOBALS.clearFlag(14);
		R2_GLOBALS._sceneManager.changeScene(1700);
		break;
	case 1804:
	// No break on purpose
	case 1805:
	// No break on purpose
	case 1806:
	// No break on purpose
	case 1807:
		R2_GLOBALS.clearFlag(14);
		R2_GLOBALS._sceneManager.changeScene(1850);
		break;
	case 1808:
		_sceneMode = 12;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(553, this);
		break;
	case 1812:
		_sceneMode = 13;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 1814:
	// No break on purpose
	case 1815:
		R2_GLOBALS._walkRegions.enableRegion(10);
		R2_GLOBALS._walkRegions.enableRegion(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 1816:
	// No break on purpose
	case 1817:
		R2_GLOBALS._walkRegions.enableRegion(8);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene1800::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
		R2_GLOBALS._sound1.fadeOut2(NULL);

	SceneExt::saveCharacter(characterIndex);
}

/*--------------------------------------------------------------------------
 * Scene 1875 - 
 *
 *--------------------------------------------------------------------------*/
Scene1875::Actor1875::Actor1875() {
	_fieldA4 = 0;
	_fieldA6 = 0;
}

void Scene1875::Actor1875::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_fieldA4);
	s.syncAsSint16LE(_fieldA6);
}

void Scene1875::Actor1875::subB84AB() {
	Scene1875 *scene = (Scene1875 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._sound1.play(227);

	int newFrameNumber;
	switch (_fieldA4) {
	case 3:
		if ((scene->_actor1._frame == 1) && (scene->_actor4._strip == 2)) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			scene->_sceneMode = 10;
			scene->_stripManager.start(576, this);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1890;
			scene->_actor2.postInit();
			scene->setAction(&scene->_sequenceManager, scene, 1890, &scene->_actor2, NULL);
		}
		break;
	case 4:
		newFrameNumber = scene->_actor1._frame + 1;
		if (newFrameNumber > 6)
			newFrameNumber = 1;
		scene->_actor1.setFrame(newFrameNumber);
		break;
	case 5:
		newFrameNumber = scene->_actor1._frame - 1;
		if (newFrameNumber < 1)
			newFrameNumber = 6;
		scene->_actor1.setFrame(newFrameNumber);
		break;
	default:
		break;
	}
}

void Scene1875::Actor1875::subB8271(int indx) {
	postInit();
	_fieldA4 = indx;
	_fieldA6 = 0;
	setVisage(1855);

	if (_fieldA4 == 1)
		setStrip(2);
	else
		setStrip(1);

	setFrame(_fieldA4);
	switch (_fieldA4 - 1) {
	case 0:
		setPosition(Common::Point(20, 144));
		break;
	case 1:
		setPosition(Common::Point(82, 144));
		break;
	case 2:
		setPosition(Common::Point(136, 144));
		break;
	case 3:
		setPosition(Common::Point(237, 144));
		break;
	case 4:
		setPosition(Common::Point(299, 144));
		break;
	default:
		break;
	}

	setDetails(1875, 6, 1, -1, 2, (SceneItem *) NULL);
}

void Scene1875::Actor1875::process(Event &event) {
	if ((R2_GLOBALS._player._uiEnabled) || (event.handled))
		return;
	
	Scene1875 *scene = (Scene1875 *)R2_GLOBALS._sceneManager._scene;

	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == R2_STEPPING_DISKS) && (_bounds.contains(event.mousePos)) && (_fieldA6 == 0)) {
		setStrip(2);
		switch (_fieldA4) {
		case 1:
			R2_GLOBALS._sound2.play(227);
			scene->_actor5.setStrip(1);
			break;
		case 2:
			R2_GLOBALS._sound2.play(227);
			scene->_actor4.setStrip(1);
			break;
		default:
			break;
		}
		_fieldA6 = 1;
		event.handled = true;
	}
	
	if ((event.eventType == EVENT_BUTTON_UP) && (_fieldA6 != 0)) {
		if ((_fieldA4 == 3) || (_fieldA4 == 4) || (_fieldA4 == 5)) {
			setStrip(1);
			subB84AB();
		}
		_fieldA6 = 0;
		event.handled = true;
	}
}

void Scene1875::postInit(SceneObjectList *OwnerList) {
	loadScene(1875);
	SceneExt::postInit();
	
	R2_GLOBALS._player._characterScene[1] = 1875;
	R2_GLOBALS._player._characterScene[2] = 1875;
	
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_actor4.subB8271(1);
	_actor5.subB8271(2);
	_actor6.subB8271(3);
	_actor7.subB8271(4);
	_actor8.subB8271(5);
	
	_actor1.postInit();
	_actor1.setup(1855, 4, 1);
	_actor1.setPosition(Common::Point(160, 116));
	
	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._sceneManager._previousScene == 1625) {
		R2_GLOBALS._sound1.play(122);
		R2_GLOBALS._player.disableControl();
		_sceneMode = 11;
		_actor2.postInit();
		setAction(&_sequenceManager, this, 1892, &_actor2, NULL);
	} else if (R2_GLOBALS._sceneManager._previousScene == 3150) {
		R2_GLOBALS._sound1.play(116);
	} else {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
	}
	
	_item2.setDetails(Rect(43, 14, 275, 122), 1875, 9, 1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 1875, 3, -1, -1, 1, NULL);
	
	R2_GLOBALS._player._characterScene[1] = 1875;
	R2_GLOBALS._player._characterScene[2] = 1875;
	R2_GLOBALS._player._oldCharacterScene[1] = 1875;
	R2_GLOBALS._player._oldCharacterScene[2] = 1875;
}

void Scene1875::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1891;
		_actor2.postInit();
		setAction(&_sequenceManager, this, 1891, &_actor2, NULL);
		break;
	case 11:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 1892;
		_stripManager.start(577, this);
		break;
	case 1890:
		_actor2.remove();
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 1891:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneManager.changeScene(1625);
		break;
	case 1892:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneManager.changeScene(1900);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene1875::process(Event &event) {
	Scene::process(event);
	
	_actor4.process(event);
	_actor5.process(event);
	_actor6.process(event);
	_actor7.process(event);
	_actor8.process(event);
}

/*--------------------------------------------------------------------------
 * Scene 1900 - 
 *
 *--------------------------------------------------------------------------*/
bool Scene1900::Actor2::startAction(CursorType action, Event &event) {
	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if ((_frame != 1) || (R2_GLOBALS._player._characterIndex != R2_SEEKER)) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			return SceneActor::startAction(action, event);
		else
			return true;
	}

	R2_GLOBALS._player.enableControl(CURSOR_USE);

	if (_position.x >= 160) {
		scene->_sceneMode = 1905;
		scene->setAction(&scene->_sequenceManager1, scene, 1905, &R2_GLOBALS._player, &scene->_actor3, NULL);
	} else {
		R2_GLOBALS.setFlag(29);
		scene->_sceneMode = 1904;
		scene->setAction(&scene->_sequenceManager1, scene, 1904, &R2_GLOBALS._player, &scene->_actor2, NULL);
	}

	return true;
}

void Scene1900::Exit1::changeScene() {
	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 10;

	Common::Point pt(-10, 135);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1900::Exit2::changeScene() {
	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 11;

	Common::Point pt(330, 135);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1900::postInit(SceneObjectList *OwnerList) {
	loadScene(1900);
	SceneExt::postInit();

	// Debug message, skipped

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1925;
		R2_GLOBALS._player._characterIndex = R2_SEEKER;
		R2_GLOBALS._player._oldCharacterScene[2] = 1925;
	}

	if (R2_GLOBALS._sceneManager._previousScene != 1875)
		R2_GLOBALS._sound1.play(200);
	
	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_seekerSpeaker);
	
	_exit1.setDetails(Rect(0, 105, 14, 145), R2_9, 2000);
	_exit1.setDest(Common::Point(14, 135));

	_exit2.setDetails(Rect(305, 105, 320, 145), R2_10, 2000);
	_exit2.setDest(Common::Point(315, 135));

	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		R2_GLOBALS._player.setup(2008, 3, 1);
	else
		R2_GLOBALS._player.setup(20, 3, 1);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	else
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	R2_GLOBALS._player.disableControl();
	
	if (R2_GLOBALS._sceneManager._previousScene != 1925)
		R2_GLOBALS.clearFlag(29);

	_actor2.postInit();
	_actor2.setup(1901, 1, 1);
	_actor2.setPosition(Common::Point(95, 109));
	_actor2.fixPriority(100);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor2.setDetails(1900, 0, 1, 2, 1, (SceneItem *) NULL);
	else
		_actor2.setDetails(1900, 0, 1, -1, 1, (SceneItem *) NULL);

	_actor3.postInit();
	_actor3.setup(1901, 2, 1);
	_actor3.setPosition(Common::Point(225, 109));
	_actor3.fixPriority(100);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_actor3.setDetails(1900, 0, 1, 2, 1, (SceneItem *) NULL);
	else
		_actor3.setDetails(1900, 0, 1, -1, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._sceneManager._previousScene != 1875) {
		_object1.postInit();
		_object1.setup(1945, 6, 1);
		_object1.setPosition(Common::Point(96, 109));
		_object1.fixPriority(80);

		_object2.postInit();
		_object2.setup(1945, 6, 2);
		_object2.setPosition(Common::Point(223, 109));
		_object2.fixPriority(80);
	}
	
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1875) {
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		_actor1.postInit();
		_sceneMode = 20;
		R2_GLOBALS._player.setAction(&_sequenceManager1, NULL, 1901, &R2_GLOBALS._player, &_actor2, NULL);
		_actor1.setAction(&_sequenceManager2, this, 1900, &_actor1, &_actor3, NULL);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1925) {
		if (R2_GLOBALS.getFlag(29)) {
			R2_GLOBALS.clearFlag(29);
			_actor2.hide();

			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player.setPosition(Common::Point(90, 106));
			_sceneMode = 1906;
			setAction(&_sequenceManager1, this, 1906, &R2_GLOBALS._player, &_actor2, NULL);
		} else {
			_actor3.hide();
			R2_GLOBALS._player.setStrip(5);
			R2_GLOBALS._player.setPosition(Common::Point(230, 106));
			_sceneMode = 1907;
			setAction(&_sequenceManager1, this, 1907, &R2_GLOBALS._player, &_actor3, NULL);
		}

		if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
			_actor1.postInit();
			_actor1.setPosition(Common::Point(30, 110));
			R2_GLOBALS._walkRegions.enableRegion(1);
			_actor1.setup(2008, 3, 1);
			_actor1.setDetails(9001, 0, -1, -1, 1, (SceneItem *) NULL);
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1900;
	} else if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor1.postInit();
		_actor1.setPosition(Common::Point(30, 110));
		R2_GLOBALS._walkRegions.enableRegion(1);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_actor1.setup(20, 3, 1);
			_actor1.setDetails(9002, 1, -1, -1, 1, (SceneItem *) NULL);
		} else {
			_actor1.setup(2008, 3, 1);
			_actor1.setDetails(9001, 0, -1, -1, 1, (SceneItem *) NULL);
		}

		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				if (R2_GLOBALS._v56605[1] == 5) {
					_sceneMode = 1902;
					setAction(&_sequenceManager1, this, 1902, &R2_GLOBALS._player, NULL);
				} else {
					_sceneMode = 1903;
					setAction(&_sequenceManager1, this, 1903, &R2_GLOBALS._player, NULL);
				}
			} else {
				if (R2_GLOBALS._v56605[2] == 5) {
					_sceneMode = 1908;
					setAction(&_sequenceManager1, this, 1908, &R2_GLOBALS._player, NULL);
				} else {
					_sceneMode = 1909;
					setAction(&_sequenceManager1, this, 1909, &R2_GLOBALS._player, NULL);
				}
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(160, 135));
			R2_GLOBALS._player.enableControl();
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1900;
	}
	
	_item2.setDetails(Rect(77, 2, 240, 103), 1900, 6, -1, -1, 1, NULL);
	_item1.setDetails(Rect(0, 0, 320, 200), 1900, 3, -1, -1, 1, NULL);
}

void Scene1900::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene1900::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 5;
		R2_GLOBALS._sceneManager.changeScene(2000);
		break;
	case 11:
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 6;
		R2_GLOBALS._sceneManager.changeScene(2000);
		break;
	case 20:
		++_sceneMode;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(1300, this);
		break;
	case 21:
		++_sceneMode;
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._action)
			R2_GLOBALS._player._action->_endHandler = this;
		else
			signal();
		break;
	case 22:
		_sceneMode = 1910;
		_actor1.setAction(&_sequenceManager2, this, 1910, &_actor1, NULL);
		break;
	case 1904:
		R2_GLOBALS._scene1925CurrLevel = -3;
	// No break on purpose
	case 1905:
		R2_GLOBALS._player.disableControl(CURSOR_ARROW);
		R2_GLOBALS._sceneManager.changeScene(1925);
		break;
	case 1910:
		R2_INVENTORY.setObjectScene(22, 2535);
		R2_GLOBALS._player.disableControl(CURSOR_ARROW);
		R2_GLOBALS._player._oldCharacterScene[1] = 1900;
		R2_GLOBALS._player._oldCharacterScene[2] = 1900;
		R2_GLOBALS._sceneManager.changeScene(2450);
		break;
	case 1906:
		R2_GLOBALS._scene1925CurrLevel = -3;
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1925 -
 *
 *--------------------------------------------------------------------------*/
Scene1925::Scene1925() {
	_field9B8 = 0;
	for (int i = 0; i < 5; i++)
		_levelResNum[i] = 0;
}

void Scene1925::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field9B8);
	for (int i = 0; i < 5; i++)
		s.syncAsSint16LE(_levelResNum[i]);
}

bool Scene1925::Hotspot2::startAction(CursorType action, Event &event) {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100))
		return SceneHotspot::startAction(action, event);

	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20))
		scene->_sceneMode = 1928;
	else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200))
		scene->_sceneMode = 1929;
	else
		scene->_sceneMode = 1930;

	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, NULL);
	return true;
}

bool Scene1925::Hotspot3::startAction(CursorType action, Event &event) {
	if ((!R2_GLOBALS.getFlag(29)) || (action != CURSOR_USE))
		return SceneHotspot::startAction(action, event);

	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_exit3._enabled = false;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return true;
	} 
	
	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20) && (event.mousePos.y >= 30)) {
		scene->_sceneMode = 1931;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200) && (event.mousePos.y < 140)) {
		scene->_sceneMode = 1932;
	} else if ( (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101)) 
		         || ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))
				 ) && (event.mousePos.y >= 100)) {
		scene->_sceneMode = 1926;
	} else if ( (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101)) 
		         || ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))
				 ) && (event.mousePos.y < 60)) {
		scene->_sceneMode = 1927;
	} else {
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);

	return true;
}

void Scene1925::ExitUp::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_exit3._enabled = false;
		scene->_field9B8 = 1927;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return;
	}

	if (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		|| ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))) {
		scene->_sceneMode = 1927;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200)) {
		scene->_sceneMode = 1932;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
	else {
		scene->_sceneMode = 1932;
		scene->signal();
	}
}

void Scene1925::Exit2::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_exit3._enabled = false;
		scene->_field9B8 = 1926;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return;
	}

	if (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		|| ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))) {
		scene->_sceneMode = 1926;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20)) {
		scene->_sceneMode = 1931;
	}

	if (scene->_sceneMode == 0) {
		scene->_sceneMode = 1931;
		scene->signal();
	} else
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::Exit3::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 1921;
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::Exit4::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 1920;
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::changeLevel(bool upFlag) {
	if (R2_GLOBALS._scene1925CurrLevel < 0)
		R2_GLOBALS._scene1925CurrLevel = 3;

	if (upFlag) {
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 200));
		++R2_GLOBALS._scene1925CurrLevel;
	} else {
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 20));
		R2_GLOBALS._scene1925CurrLevel--;
	}

	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -1:
		R2_GLOBALS._sceneManager.changeScene(1945);
		return;
		break;
	case 3:
		loadScene(_levelResNum[4]);
		_item2.setDetails(Rect(133, 68, 140, 77), 1925, 3, -1, 5, 2, NULL);
		_actor1.setDetails(1925, 0, 1, 2, 2, (SceneItem *) NULL);
		_actor1.show();
		break;
	case 512:
		R2_GLOBALS._scene1925CurrLevel = 508;
	// No break on purpose
	default:
		loadScene(_levelResNum[(R2_GLOBALS._scene1925CurrLevel % 4)]);
		R2_GLOBALS._sceneItems.remove(&_item2);
		R2_GLOBALS._sceneItems.remove(&_actor1);
		_actor1.hide();
		break;
	}

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
}

void Scene1925::postInit(SceneObjectList *OwnerList) {
	_levelResNum[0] = 1930;
	_levelResNum[1] = 1935;
	_levelResNum[2] = 1940;
	_levelResNum[3] = 1935;
	_levelResNum[4] = 1925;

	if (R2_GLOBALS.getFlag(29)) {
		if ((R2_GLOBALS._scene1925CurrLevel == -3) || (R2_GLOBALS._scene1925CurrLevel == 3))
			loadScene(_levelResNum[4]);
		else
			loadScene(_levelResNum[R2_GLOBALS._scene1925CurrLevel % 4]);
	} else {
		R2_GLOBALS._scene1925CurrLevel = -2;
		loadScene(1920);
	}
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(220);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player._characterScene[2] = 1925;
	R2_GLOBALS._player._characterIndex = R2_SEEKER;
	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -2:
		_exit4.setDetails(Rect(203, 44, 247, 111), EXITCURSOR_E, 1925);
		_item3.setDetails(Rect(31, 3, 45, 167), 1925, 6, -1, 8, 1, NULL);
		break;
	case 3:
		_actor1.setDetails(1925, 0, 1, 2, 1, (SceneItem *) NULL);
		_item2.setDetails(Rect(133, 68, 140, 77), 1925, 3, -1, 5, 1, NULL);
	// No break on purpose
	case -3: 
		_exit3.setDetails(Rect(83, 38, 128, 101), EXITCURSOR_W, 1925);
	// No break on purpose
	default:
		_exitUp.setDetails(Rect(128, 0, 186, 10), EXITCURSOR_N, 1925);
		_exit2.setDetails(Rect(128, 160, 190, 167), EXITCURSOR_S, 1925);
		_item3.setDetails(Rect(141, 11, 167, 159),	1925, 6, -1, -1, 1, NULL);
		break;
	}

	_actor1.postInit();
	_actor1.setup(1925, 5, 1);
	_actor1.setPosition(Common::Point(128, 35));
	_actor1.hide();

	if (R2_GLOBALS._scene1925CurrLevel == 3)
		_actor1.show();

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -2:
		R2_GLOBALS._player.setup(20, 6, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(224, 109));
		break;
	case -3:
		_actor1.hide();
		R2_GLOBALS._player.setup(20, 5, 1);
		R2_GLOBALS._player.setPosition(Common::Point(110, 100));
		break;
	case 0:
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 200));
		break;
	default:
		R2_GLOBALS._player.setup(1925, 1, 3);
		R2_GLOBALS._player.setPosition(Common::Point(154, 110));
		break;
	}

	R2_GLOBALS._player._canWalk = false;
	_field9B8 = 0;
	R2_GLOBALS._sceneManager._previousScene = 1925;
	_item1.setDetails(Rect(27, 0, 292, 200), 1925, 9, -1, -1, 1, NULL);
}

void Scene1925::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._player._oldCharacterScene[2] = 1925;
	SceneExt::remove();
}

void Scene1925::signal() {
	switch (_sceneMode) {
	case 1920:
	// No break on purpose
	case 1921:
	// No break on purpose
	case 1928:
	// No break on purpose
	case 1929:
	// No break on purpose
	case 1930:
		R2_GLOBALS._scene1925CurrLevel = -3;
		R2_GLOBALS._sceneManager.changeScene(1900);
		break;
	case 1926:
	// No break on purpose
	case 1931:
		// Change level, down
		changeLevel(false);
		break;
	case 1927:
	// No break on purpose
	case 1932:
		// Change level, up
		changeLevel(true);
		break;
	case 1925:
		_exit3._enabled = false;
		if (_field9B8 != 0) {
			_sceneMode = _field9B8;
			_field9B8 = 0;
			setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	}

	R2_GLOBALS._player._canWalk = false;
}

/*--------------------------------------------------------------------------
 * Scene 1945 - 
 *
 *--------------------------------------------------------------------------*/
Scene1945::Scene1945() {
	_fieldEAA = 0;
	_fieldEAC = 0;
	_fieldEAE = CURSOR_NONE;
}

void Scene1945::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_fieldEAA);
	s.syncAsSint16LE(_fieldEAC);
	s.syncAsSint16LE(_fieldEAE);
}

bool Scene1945::Hotspot3::startAction(CursorType action, Event &event) {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_37:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142))
			scene->_sceneMode = 1942;
		else {
			scene->_sceneMode = 1940;
			scene->_fieldEAA = 1942;
		}
		// At this point the original check if _sceneMode != 0. Skipped.
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		return true;
		break;
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl(CURSOR_USE);
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50) && (event.mousePos.x >= 130)) {
			scene->_sceneMode = 1940;
			// At this point the original check if _sceneMode != 0. Skipped.
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
			return true;
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
			if (event.mousePos.x > 130)
				scene->_item3.setDetails(1945, 3, -1, -1, 3, (SceneItem *) NULL);
			else
				scene->_item3.setDetails(1945, 3, -1, 5, 3, (SceneItem *) NULL);
		}
	// No break on purpose
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene1945::Hotspot4::startAction(CursorType action, Event &event) {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 221) && (R2_GLOBALS._player._position.y == 142)) {
		scene->_sceneMode = 1949;
		scene->_fieldEAA = 1947;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1947;
	} else if ((R2_GLOBALS._player._position.x == 221) && (R2_GLOBALS._player._position.y == 142) && (event.mousePos.y >= 30)) {
		scene->_sceneMode = 1940;
	} else {
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}
	
	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);

	return true;
}

bool Scene1945::Actor3::startAction(CursorType action, Event &event) {
	if ((action == R2_50) && (action == R2_49)) {
		Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

		scene->_fieldEAE = action;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) {
			scene->_sceneMode= 1947;
			scene->_fieldEAA = 1943;
		} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50)) {
			scene->_sceneMode = 1940;
			scene->_fieldEAA = 1943;
		} else {
			scene->_sceneMode = 1949;
			scene->_fieldEAA = 1947;
			scene->_fieldEAC = 1943;
		}
		// At this point the original check if _sceneMode != 0. Skipped.
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene1945::ExitUp::changeScene() {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 221) && (R2_GLOBALS._player._position.y == 142)) {
		scene->_sceneMode = 1949;
		scene->_fieldEAA = 1947;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1947;
	}

	if (scene->_sceneMode == 0) {
		scene->_sceneMode = 1941;
		signal();
	} else {
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
	}
}

void Scene1945::Exit2::changeScene() {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50)) {
		scene->_sceneMode = 1940;
		scene->_fieldEAA = 1945;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1945;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1945::postInit(SceneObjectList *OwnerList) {
	loadScene(1945);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(220);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
	R2_GLOBALS._player._characterScene[2] = 1945;
	R2_GLOBALS._player._characterIndex = R2_SEEKER;

	_exitUp.setDetails(Rect(128, 0, 186, 10), EXITCURSOR_N, 1945);
	_exit2.setDetails(Rect(238, 144, 274, 167), EXITCURSOR_E, 1945);

	_item4.setDetails(Rect(141, 3, 167, 126), 1945, 9, -1, -1, 1, NULL);

	if (!R2_GLOBALS.getFlag(43)) {
		_exit2._enabled = false;
		_actor3.postInit();
		_actor3.setup(1945, 4, 1);
		_actor3.setPosition(Common::Point(253, 169));
		_actor3.fixPriority(150);

		if (R2_GLOBALS.getFlag(42))
			_actor3.setDetails(1945, 15, -1, -1, 1, (SceneItem *) NULL);
		else
			_actor3.hide();

		_actor1.postInit();
		_actor1.setup(1945, 8, 1);
		_actor1.setPosition(Common::Point(253, 169));
		_actor1.fixPriority(130);

		_actor2.postInit();
		_actor2.setup(1945, 3, 1);
		_actor2.hide();
	} else {
		_exit2._enabled = true;
	}

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1925:
		R2_GLOBALS._scene1925CurrLevel = 0;
		R2_GLOBALS.clearFlag(29);
		R2_GLOBALS._player.setup(1925, 1, 10);
		R2_GLOBALS._player.setPosition(Common::Point(154, 50));
		break;
	case 1950:
		_sceneMode = 1944;
		setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		R2_GLOBALS._player.disableControl(CURSOR_USE);
		break;
	default:
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setup(20, 5, 1);
		R2_GLOBALS._player.setPosition(Common::Point(191, 142));
		break;
	}

	R2_GLOBALS._player._canWalk = false;
	_fieldEAA = 0;
	_fieldEAC = 0;

	_item3.setDetails(11, 1945, 3, -1, 5);
	_item1.setDetails(Rect(238, 144, 274, 167), 1945, 0, -1, 2, 1, NULL);
	_item2.setDetails(Rect(27, 3, 292, 167), 1945, 3, -1, -1, 1, NULL);
}

void Scene1945::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1945::signal() {
	switch (_sceneMode) {
	case 1940:
		if (_fieldEAA == 1943) {
			_sceneMode = _fieldEAA;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_actor2, NULL);
		} else {
			_sceneMode = 1946;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
		break;
	case 1941:
		if (_fieldEAA == 0) {
			R2_GLOBALS._scene1925CurrLevel = 0;
			R2_GLOBALS.setFlag(29);
			R2_GLOBALS._sceneManager.changeScene(1925);
		} else {
			_sceneMode = _fieldEAA;
			_fieldEAA = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	case 1942:
		R2_INVENTORY.setObjectScene(R2_37, 0);
		_actor3.setDetails(1945, 15, -1, -1, 2, (SceneItem *) NULL);
		R2_GLOBALS.setFlag(42);
		break;
	case 1943:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_INVENTORY.setObjectScene(_fieldEAE, 0);
		_sceneMode = 1948;
		setAction(&_sequenceManager1, this, _sceneMode, &_actor3, &_actor2, &_actor1, NULL);
		setAction(&_sequenceManager2, NULL, 1941, &R2_GLOBALS._player, NULL);
		return;
	case 1944:
		break;
	case 1945:
		R2_GLOBALS._sceneManager.changeScene(1950);
		return;
	case 1946:
		if (_fieldEAA == 1942) {
			_sceneMode = _fieldEAA;
			_fieldEAA = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_actor3, NULL);
			return;
		}
		break;
	case 1947:
		if (_fieldEAA == 1943) {
			_sceneMode = _fieldEAA;
			_fieldEAA = 1948;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_actor2, NULL);
		} else {
			_sceneMode = 1941;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	case 1948:
		R2_GLOBALS._sound1.play(220);
		_exit2._enabled = true;
		R2_GLOBALS._sceneItems.remove(&_actor3);
		R2_GLOBALS.clearFlag(42);
		R2_GLOBALS.clearFlag(43);
		_fieldEAA = 1940;
	// No break on purpose
	case 1949:
		_sceneMode = _fieldEAA;
		if (_fieldEAC == 1943) {
			_fieldEAA = _fieldEAC;
			_fieldEAC = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_actor2, NULL);
		} else {
			_fieldEAA = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	default:
		break;
	}

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
