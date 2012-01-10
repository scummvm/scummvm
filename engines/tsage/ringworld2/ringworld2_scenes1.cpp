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
	case R2_7:
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

void Scene1200::Area1::postInit() {
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
	warning("_object1.sub9EDE8(tmpRect);");
	warning("_object1.sub51AE9(1);");
	warning("_object1.sub51AFD(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);");
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
		warning("_object1.sub9EE22(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);");
		int unk = _object1.sub51AF8(event.mousePos);
		switch (R2_GLOBALS._events.getCursor()) {
		case CURSOR_ARROW:
			event.handled = true;
			if ((event.mousePos.x > 179) && (event.mousePos.x < 210) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				warning("sub9DAD6(1);");

			if ((event.mousePos.x > 109) && (event.mousePos.x < 140) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				warning("sub9DAD6(2);");
				
			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 89) && (event.mousePos.y < 120))
				warning("sub9DAD6(3);");
				
			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 19) && (event.mousePos.y < 50))
				warning("sub9DAD6(4);");
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
			warning("sub9DAD6(4);");
			break;
		case Common::KEYCODE_2:
			warning("FIXME: keycode = 0x4B00");
			warning("sub9DAD6(2);");
			break;
		case Common::KEYCODE_3:
			warning("FIXME: keycode = 0x4D00");
			warning("sub9DAD6(1);");
			break;
		case Common::KEYCODE_4:
			warning("FIXME: keycode = 0x5000");
			warning("sub9DAD6(3);");
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
		warning("_object1.sub51AFD(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);");
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
		warning("_object1.sub51AFD(R2_GLOBALS._v56AA2, R2_GLOBALS._v56AA4);");
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


} // End of namespace Ringworld2
} // End of namespace TsAGE
