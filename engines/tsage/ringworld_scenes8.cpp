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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.cpp $
 * $Id: scene_logic.cpp 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#include "tsage/ringworld_scenes8.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 7000
 *
 *--------------------------------------------------------------------------*/

void Scene7000::Action1::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		setAction(&scene->_action6, this);
		break;
	case 2:
		scene->_soundHandler.startSound(252, 0, 127);
		scene->_object8.remove();
		scene->_object1.postInit();
		scene->_object1.setVisage(7003);
		scene->_object1.animate(ANIM_MODE_5, this);
		scene->_object1.setPosition(Common::Point(151, 182), 0);
		scene->_object1.setPriority2(205);
		_globals->_sceneItems.push_front(&scene->_object1);
		break;
	case 3:
		scene->_object1.setStrip(4);
		scene->_object1.animate(ANIM_MODE_8, 0, 0);
		scene->_stripManager.start(7005, this);
		break;
	case 4:
		scene->_object1.animate(ANIM_MODE_2, 0);
		setDelay(3);
		break;
	case 5:
		_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action2::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		_globals->_player.addMover(0, 0);
		_globals->_player.setVisage(7006);
		_globals->_player.setStrip(1);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(_globals->_player._position.x, _globals->_player._position.y + 13));
		_globals->_player.changeZoom(68);
		_globals->_player.animate(ANIM_MODE_5, this);
		scene->_object1.remove();
		break;
	case 2:
		_globals->_sceneManager.changeScene(7100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action3::dispatch() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	Action::dispatch();
	if (_actionIndex == 4)
		scene->_object4.setPosition(Common::Point(scene->_object3._position.x, scene->_object3._position.y));
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action3::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		scene->setZoomPercents(10, 10, 62, 100);
		scene->_object4.postInit();
		scene->_object4.setVisage(5001);
		scene->_object4.setStrip2(2);
		scene->_object4.animate(ANIM_MODE_8, 0, 0);
		scene->_object4.setPosition(Common::Point(10, 18), 0);
		scene->_object4.setPriority2(10);
		scene->_object4.changeZoom(100);
		scene->_object4.flag100();
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(107, 65);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 2:
		scene->_object3.setPriority2(10);
		scene->_object4.setPosition(Common::Point(scene->_object3._position.x, scene->_object3._position.y + 15), 0);
		scene->_object4.unflag100();
		setDelay(30);
		break;
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(107, 92);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4:
		scene->_object4.remove();
		_globals->_sceneManager.changeScene(2100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action4::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 1:
		scene->_object1.flag100();
		setDelay(300);
		break;
	case 2:
		_globals->_soundHandler.startSound(252, 0, 127);
		scene->_object1.unflag100();
		scene->_object1.setStrip(3);
		scene->_object1.setFrame(1);
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object1.setStrip(1);
		scene->_object1.animate(ANIM_MODE_8, 0, 0);
		_globals->setFlag(81);
		_globals->_player.enableControl();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action5::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		NpcMover *playerMover = new NpcMover();
		Common::Point pt(88, 121);
		_globals->_player.addMover(playerMover, &pt, this);
		break;
	}
	case 1:
		_globals->_player.checkAngle(&scene->_object1);
		_globals->_soundHandler.startSound(252, 0, 127);
		scene->_object1.setStrip(2);
		scene->_stripManager.start(7015, this);
		break;
	case 2:
		scene->_object1.setStrip(1);
		scene->_object1.setFrame(4);
		scene->_object1.animate(ANIM_MODE_6, this);
		break;
	case 3: {
		scene->_object1.remove();
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 94);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 94);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		_globals->_player.setPriority2(10);
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 89);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(41, 89);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		_globals->clearFlag(36);
		_globals->clearFlag(37);
		_globals->clearFlag(72);
		_globals->clearFlag(13);
		_globals->_sceneManager.changeScene(2100);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action6::signal() {
	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(12, 91);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(8, 91);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 96);
		_globals->_player.addMover(mover, &pt, this);
		_globals->_player.setPriority2(-1);
		break;
	}
	case 4: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(83, 117);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(95, 121);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6:
		_globals->_player.setStrip(3);
		_globals->_player.setFrame(1);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::Action7::signal() {
	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(3);
		break;
	case 1:
		setDelay(3);
		break;
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(31, 94);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 94);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		_globals->_player.setPriority2(10);
		NpcMover *mover = new NpcMover();
		Common::Point pt(11, 89);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(41, 89);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6:
		if (_globals->getFlag(13))
			_globals->_sceneManager.changeScene(2280);
		else
			_globals->_sceneManager.changeScene(2320);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::SceneItem1::doAction(int action) {
	if (action == CURSOR_LOOK)
		SceneItem::display(7000, 2, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);

	SceneItem::doAction(action);
}

/*--------------------------------------------------------------------------*/

void Scene7000::Object1::doAction(int action) {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_TRANSLATOR:
		_globals->_player.disableControl();
		_globals->_inventory._translator._sceneNumber = 7000;
		if (_globals->_inventory._waldos._sceneNumber == 7000) {
			if (_globals->_inventory._jar._sceneNumber == 7000) {
				scene->_sceneMode = 7012;
				scene->setAction(&scene->_sequenceManager, scene, 7010, &_globals->_player, &scene->_object1, 0, 0);
			} else {
				scene->_sceneMode = 7015;
				scene->setAction(&scene->_sequenceManager, scene, 7017, &_globals->_player, 0, 0);
			}
		} else {
			if (_globals->_inventory._jar._sceneNumber == 7000) {
				scene->_sceneMode = 7011;
				scene->setAction(&scene->_sequenceManager, scene, 7010, &_globals->_player, &scene->_object1, 0, 0);
			} else {
				scene->_sceneMode = 7004;
				scene->setAction(&scene->_sequenceManager, scene, 7004, &_globals->_player, this, 0, 0);
			}
		}
		break;
	case OBJECT_WALDOS:
		_globals->_player.disableControl();
		_globals->_inventory._waldos._sceneNumber = 7000;
		if (_globals->_inventory._translator._sceneNumber == 7000) {
			if (_globals->_inventory._jar._sceneNumber == 7000) {
				scene->_sceneMode = 7015;
				scene->setAction(&scene->_sequenceManager, scene, 7015, &_globals->_player, 0, 0);
			} else {
				scene->_sceneMode = 7006;
				scene->setAction(&scene->_sequenceManager, scene, 7006, &_globals->_player, 0, 0);
			}
		} else {
			scene->_sceneMode = 7009;
			scene->setAction(&scene->_sequenceManager, scene, 7009, &_globals->_player, 0, 0);
		}
		break;
	case OBJECT_JAR:
		_globals->_player.disableControl();
		_globals->_inventory._jar._sceneNumber = 7000;
		if (_globals->_inventory._translator._sceneNumber == 7000) {
			if (_globals->_inventory._waldos._sceneNumber == 7000) {
				scene->_sceneMode = 7007;
				scene->setAction(&scene->_sequenceManager, scene, 7007, &_globals->_player, &scene->_object1, 0, 0);
			} else {
				scene->_sceneMode = 7015;
				scene->setAction(&scene->_sequenceManager, scene, 7016, &_globals->_player, 0, 0);
			}
		} else {
			scene->_sceneMode = 7008;
			scene->setAction(&scene->_sequenceManager, scene, 7008, &_globals->_player, 0, 0);
		}
		break;
	case CURSOR_LOOK:
		if (_globals->getFlag(81))
			SceneItem::display(7000, 1, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		else
			SceneItem::display(7000, 0, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(81)) {
			_globals->_inventory._stasisBox._sceneNumber = 1;
			_globals->_player.disableControl();
			scene->setAction(&scene->_action5);
		} else {
			SceneItem::display(7000, 5, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
		}
		break;
	case CURSOR_TALK:
		if (_globals->getFlag(81)) {
			_globals->_inventory._stasisBox._sceneNumber = 1;
			_globals->_player.disableControl();
			scene->setAction(&scene->_action5);
		} else if (_globals->getFlag(52)) {
			scene->_sceneMode = 7005;
			scene->setAction(&scene->_sequenceManager, scene, 7013, 0, 0);
		} else if (_globals->getFlag(13)) {
			_globals->_sceneManager._sceneNumber = 7002;
			scene->setAction(&scene->_sequenceManager, scene, 7014, 0, 0);
		} else {
			_globals->_sceneManager._sceneNumber = 7002;
			scene->setAction(&scene->_sequenceManager, scene, 7002, 0, 0);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::dispatch() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;
	
	if (!_action) {
		if (_globals->_sceneRegions.indexOf(_globals->_player._position) == 8) {
			if (!_globals->getFlag(13)) {
				_globals->_player.disableControl();
				_globals->_player.addMover(0);
				SceneItem::display(7000, 3, SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
				_sceneMode = 7001;
				setAction(&scene->_sequenceManager, this, 7001, &_globals->_player, NULL);
			} else if (!_globals->getFlag(52)) {
				setAction(&_action2);
			} else {
				_globals->_player.disableControl();
				_sceneMode = 7003;
				setAction(&scene->_sequenceManager, this, 7003, &_globals->_player, 0);
			}
		}
		if (_globals->_sceneRegions.indexOf(_globals->_player._position) == 9)
			scene->setAction(&scene->_action7);
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene7000::postInit(SceneObjectList *OwnerList) {
	loadScene(7000);
	Scene::postInit();
	setZoomPercents(93, 25, 119, 55);
	_stripManager.addSpeaker(&_speakerSKText);
	_stripManager.addSpeaker(&_speakerSKL);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);

	_speakerSKText._npc = &_object1;
	_speakerQText._npc = &_globals->_player;

	_object5.postInit();
	_object5.setVisage(7001);
	_object5.setStrip2(1);
	_object5.animate(ANIM_MODE_2, 0);
	_object5.setPosition(Common::Point(49, 147), 0);
	_object5.setPriority2(1);

	_object6.postInit();
	_object6.setVisage(7001);
	_object6.setStrip2(2);
	_object6.animate(ANIM_MODE_2, 0);
	_object6.setPosition(Common::Point(160, 139), 0);
	_object6.setPriority2(1);

	_object7.postInit();
	_object7.setVisage(7001);
	_object7.setStrip2(3);
	_object7.animate(ANIM_MODE_2, 0);
	_object7.setPosition(Common::Point(272, 129), 0);
	_object7.setPriority2(1);

	_object8.postInit();
	_object8.setVisage(7001);
	_object8.setStrip2(4);
	_object8.animate(ANIM_MODE_2, 0);
	_object8.setPosition(Common::Point(176, 175), 0);
	_object8.setPriority2(1);

	if (_globals->getFlag(72)) {
		_object3.postInit();
		_object3.setVisage(5001);
		_object3.setStrip2(1);
		_object3.animate(ANIM_MODE_2, 0);
		_object3.setPosition(Common::Point(107, 92), 0);
		_object3.changeZoom(100);
		_object3.setPriority2(10);
		
		_object1.postInit();
		_object1.setVisage(7003);
		if (_globals->getFlag(81))
			_object1.setStrip(4);
		else
			_object1.setStrip(2);
		_object1.setPosition(Common::Point(87, 129), 0);
		_object1._numFrames = 4;
		_object1.changeZoom(45);
		_object1.animate(ANIM_MODE_8, 0, 0);
		_globals->_sceneItems.addItems(&_object1, 0);
	}
	_soundHandler.startSound(251, 0, 127);
	if (_globals->_sceneManager._previousScene == 2100) {
		if (_globals->getFlag(72)) {
			_globals->_player.postInit();
			_globals->_player.setVisage(0);
			_globals->_player.animate(ANIM_MODE_1, 0);
			SceneObjectWrapper *wrapper = new SceneObjectWrapper();
			_globals->_player.setObjectWrapper(wrapper);
			_globals->_player.setPosition(Common::Point(57, 94), 0);
			_globals->_player.changeZoom(-1);
			_globals->_player.setPriority2(10);
			if (_globals->getFlag(81)) {
				setAction(&_action4);
			} else {
				_object1.setPosition(Common::Point(151, 182), 0);
				setAction(&_action1);
			}
		} else {
			_globals->_soundHandler.startSound(250, 0, 127);
			_globals->setFlag(72);

			_object3.postInit();
			_object3.setVisage(5001);
			_object3.setStrip2(1);
			_object3.animate(ANIM_MODE_1, 0);
			_object3.setPosition(Common::Point(307, 0), 0);
			_object3.changeZoom(-1);
			setAction(&_action3);
		}
	} else if (_globals->_sceneManager._previousScene == 2280) {
		_globals->_player.postInit();
		_globals->_player.setVisage(2170);
		_globals->_player.animate(ANIM_MODE_1, 0);
		SceneObjectWrapper *wrapper = new SceneObjectWrapper();
		_globals->_player.setObjectWrapper(wrapper);
		_globals->_player.setPosition(Common::Point(57, 94), 0);
		_globals->_player.changeZoom(-1);
		_globals->_player.setPriority2(10);
		_globals->_player.disableControl();
		_sceneMode = 7001;
		setAction(&_action6, this);
		if (!_globals->getFlag(81)) {
			_object1.setPosition(Common::Point(151, 182), 0);
			_object1.changeZoom(100);
		} 
		_object8.remove();
		_object9.remove();
	} else if (_globals->_sceneManager._previousScene == 2320) {
		_globals->_player.postInit();
		_globals->_player.setVisage(0);
		_globals->_player.animate(ANIM_MODE_1, 0);
		SceneObjectWrapper *wrapper = new SceneObjectWrapper();
		_globals->_player.setObjectWrapper(wrapper);
		_globals->_player.setPosition(Common::Point(57, 94), 0);
		_globals->_player.changeZoom(-1);
		_globals->_player.setPriority2(10);
		_sceneMode = 7001;
		setAction(&_action6, this);
	} else {
		_globals->setFlag(72);

		_object3.postInit();
		_object3.setVisage(5001);
		_object3.setStrip2(1);
		_object3.setPosition(Common::Point(307, 0), 0);
		_soundHandler.startSound(151, 0, 127);
		_soundHandler.proc5(1);
		_globals->_soundHandler.startSound(250, 0, 127);
		setAction(&_action3);
	}
}

/*--------------------------------------------------------------------------*/

void Scene7000::signal() {
	Scene7000 *scene = (Scene7000 *)_globals->_sceneManager._scene;
	switch (_sceneMode) {
		case 7001:
		case 7002:
		case 7004:
		case 7009:
			_globals->_player.enableControl();
			break;
		case 7003:
			_sceneMode = 7001;
			setAction(&scene->_sequenceManager, this, 7001, &_globals->_player, NULL);
			break;
		case 7011:
			_sceneMode = 7005;
			setAction(&scene->_sequenceManager, this, 7005, &_globals->_player, NULL);
			break;
		case 7012:
			_sceneMode = 7005;
			setAction(&scene->_sequenceManager, this, 7012, &_globals->_player, NULL);
			break;
		case 7015:
			setAction(&_action4);
			break;
	}
}


/*--------------------------------------------------------------------------
 * Scene 7200
 *
 *--------------------------------------------------------------------------*/

void Scene7200::Action1::signal() {
	Scene7200 *scene = (Scene7200 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(15);
		break;
	case 1: {
		PlayerMover *mover1 = new PlayerMover();
		Common::Point pt1(165, 147);
		scene->_swimmer.addMover(mover1, &pt1, this);
		Common::Point pt2(207, 138);
		PlayerMover *mover2 = new PlayerMover();
		_globals->_player.addMover(mover2, &pt2, this);
		break;
	}
	case 2:
		break;
	case 3:
		_globals->_sceneManager.changeScene(7300);
		remove();
	break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7200::Action2::signal() {
	Scene7200 *scene = (Scene7200 *)_globals->_sceneManager._scene;

printf("Action %d\n", _actionIndex); 

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1: {
		scene->_object2.setPriority2(25);
		scene->_object3.setPriority2(25);
		scene->_object4.setPriority2(25);
		scene->_object2.setStrip(1);
		scene->_object3.setStrip(1);
		scene->_object4.setStrip(1);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt1(54, 90);
		scene->_object2.addMover(mover1, &pt1, this);
		NpcMover *mover2 = new NpcMover();
		Common::Point pt2(56, 85);
		scene->_object3.addMover(mover2, &pt2, 0);
		NpcMover *mover3 = new NpcMover();
		Common::Point pt3(54, 80);
		scene->_object4.addMover(mover3, &pt3, 0);
		break;
	}
	case 2: {
		scene->_object2.setPriority2(160);
		scene->_object3.setPriority2(160);
		scene->_object4.setPriority2(160);
		scene->_object2.setStrip(2);
		scene->_object3.setStrip(2);
		scene->_object4.setStrip(2);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt1(10, 89);
		scene->_object2.addMover(mover1, &pt1, this);
		NpcMover *mover2 = new NpcMover();
		Common::Point pt2(12, 84);
		scene->_object3.addMover(mover2, &pt2, 0);
		NpcMover *mover3 = new NpcMover();
		Common::Point pt3(10, 79);
		scene->_object4.addMover(mover3, &pt3, 0);
		break;
	}
	case 3:
		_actionIndex = 0;
		setDelay(1);
		remove();
	break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7200::postInit(SceneObjectList *OwnerList) {
	loadScene(7200);
	Scene::postInit();
	_object2.postInit();
	_object2.setVisage(7160);
	_object2.animate(ANIM_MODE_2, 0);
	_object2.setZoom(10);
	_object2.setPosition(Common::Point(53, 88), 0);
	_object2.setAction(&_action2);

	_object3.postInit();
	_object3.setVisage(7160);
	_object3.animate(ANIM_MODE_2, 0);
	_object3.setZoom(10);
	_object3.setPosition(Common::Point(55, 83), 0);

	_object4.postInit();
	_object4.setVisage(7160);
	_object4.animate(ANIM_MODE_2, 0);
	_object4.setZoom(10);
	_object4.setPosition(Common::Point(57, 78), 0);

	_object5.postInit();
	_object5.setVisage(7201);
	_object5.setPosition(Common::Point(300, 172), 0);
	_object5.setPriority(172);
	_object5.animate(ANIM_MODE_2, 0);
	_object5._numFrames = 3;

	_object6.postInit();
	_object6.setVisage(7201);
	_object6.setStrip2(3);
	_object6.setPosition(Common::Point(144, 97), 0);
	_object6.setPriority(199);
	_object6.animate(ANIM_MODE_2, 0);
	_object6._numFrames = 3;

	_object7.postInit();
	_object7.setVisage(7201);
	_object7.setStrip2(4);
	_object7.setPosition(Common::Point(115, 123), 0);
	_object7.setPriority(199);
	_object7.animate(ANIM_MODE_2, 0);
	_object7._numFrames = 3;

	_object8.postInit();
	_object8.setVisage(7201);
	_object8.setStrip2(6);
	_object8.setPosition(Common::Point(140, 173), 0);
	_object8.setPriority(199);
	_object8.animate(ANIM_MODE_2, 0);
	_object8._numFrames = 3;

	_object9.postInit();
	_object9.setVisage(7201);
	_object9.setStrip2(7);
	_object9.setPosition(Common::Point(215, 196), 0);
	_object9.setPriority(199);
	_object9.animate(ANIM_MODE_2, 0);
	_object9._numFrames = 3;

	// Orange swimmer
	_globals->_player.postInit();
	_globals->_player.setVisage(7110);
	_globals->_player.animate(ANIM_MODE_1, 0);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setZoom(50);
	_globals->_player.setPosition(Common::Point(-18, 16), 0);
	_globals->_player.disableControl();

	_swimmer.postInit();
	_swimmer.setVisage(7101);
	_swimmer.animate(ANIM_MODE_1, 0);
	_swimmer.setObjectWrapper(new SceneObjectWrapper());
	_swimmer.setZoom(50);
	_swimmer.setPosition(Common::Point(-8, 16), 0);

	setAction(&_action1);
	_soundHandler.startSound(271, 0, 127);
}

/*--------------------------------------------------------------------------
 * Scene 7600
 *
 *--------------------------------------------------------------------------*/

void Scene7600::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1: {
		PlayerMover *mover = new PlayerMover();
		Common::Point pt(389, 57);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		_globals->_sceneManager.changeScene(7700);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7600::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(-30, 195);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		_globals->_sceneManager.changeScene(2320);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene7600::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(100, 0, 200, 100);

	_object2.postInit();
	_object2.setVisage(7601);
	_object2.setStrip(1);
	_object2.animate(ANIM_MODE_2, 0);
	_object2.setPosition(Common::Point(48, 135));
	_object2.setPriority2(1);

	_object3.postInit();
	_object3.setVisage(7601);
	_object3.setStrip(2);
	_object3.animate(ANIM_MODE_2, 0);
	_object3.setPosition(Common::Point(158, 136));
	_object3.setPriority2(1);

	_object4.postInit();
	_object4.setVisage(7601);
	_object4.setStrip(3);
	_object4.animate(ANIM_MODE_2, 0);
	_object4.setPosition(Common::Point(293, 141));
	_object4.setPriority2(1);

	_object5.postInit();
	_object5.setVisage(7601);
	_object5.setStrip(4);
	_object5.animate(ANIM_MODE_2, 0);
	_object5.setPosition(Common::Point(405, 143));
	_object5.setPriority2(1);

	_object6.postInit();
	_object6.setVisage(7601);
	_object6.setStrip(5);
	_object6.animate(ANIM_MODE_2, 0);
	_object6.setPosition(Common::Point(379, 191));
	_object6.setPriority2(1);

	_globals->_player.postInit();
	_globals->_player.setVisage(2333);
	_globals->_player.animate(ANIM_MODE_1, 0);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setStrip(1);
	_globals->_player._moveDiff = Common::Point(16, 16);
	_globals->_player.changeZoom(-1);
	_globals->_player.disableControl();

	if (_globals->_sceneManager._previousScene == 7700) {
		_globals->_player.setPosition(Common::Point(389, 57));
		setAction(&_action2);
	} else {
		_globals->_player.setPosition(Common::Point(-50, 195));
		setAction(&_action1);
	}
	_sceneBounds.centre(_globals->_player._position.x, _globals->_player._position.y);
	loadScene(7600);
	_soundHandler2.startSound(255);
	_soundHandler1.startSound(251);
}
} // End of namespace tSage
