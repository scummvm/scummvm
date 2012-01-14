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
	if ((scene->_sceneMode >= 20) and (scene->_sceneMode <= 29))
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
void Scene1550::UnkArea1550::proc12(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	warning("Scene1550::UnkArea1550::proc12() should be based on Scene1550::UnkArea1200::proc12()");
}

bool Scene1550::Hotspot1::startAction(CursorType action, Event &event) {
	return SceneHotspot::startAction(action, event);
}

bool Scene1550::Hotspot3::startAction(CursorType action, Event &event) {
	// Arrays related to this scene are all hacky in the origina: they are based on the impossibility to use Miranda
	assert ((R2_GLOBALS._player._characterIndex == 1) || (R2_GLOBALS._player._characterIndex == 2));
	// The original contains a debug message when CURSOR_TALK is used.
	if (action == CURSOR_TALK)
		warning("Location: %d/%d - %d", R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex], R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2], _v5A4D6[(R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex + 2] * 30)] + R2_GLOBALS._v565EC[R2_GLOBALS._player._characterIndex]);

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
			warning("_arrUnkObj15502[7].subA5CDF()");
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
	warning("subA2B2F();");
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

		warning("R2_GLOBALS._walkRegions.enableRegion(R2_GLOBALS._v14A72[_field419]);");
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
		warning("subA2B2F();");
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
		warning("subA2B2F();");
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
		warning("subA2B2F();");
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

} // End of namespace Ringworld2
} // End of namespace TsAGE
