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

#include "tsage/blue_force/blueforce_scenes2.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 200 - Credits - Motorcycle Training
 *
 *--------------------------------------------------------------------------*/

void Scene200::Action1::signal() {
	Scene200 *scene = (Scene200 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);
	static const uint32 black = 0;
	assert(owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		_state = 0;
		setDelay(30);
		break;
	case 2:
		assert(_owner);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 3:
		if (++_state < 2) {
			scene->_action2.signal();
			owner->setFrame(1);
			_actionIndex = 2;
		}
		setDelay(2);
		break;
	case 4: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);
		
		scene->setAction(&scene->_sequenceManager, this, 201, &scene->_object1, &scene->_object2,
			&scene->_object3, &scene->_object4, &scene->_object5, &scene->_object6, NULL);
		break;
	}
	case 5:
		BF_GLOBALS._sceneManager.changeScene(210);
		break;
	default:
		break;
	}
}

void Scene200::Action2::signal() {
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);
	assert(owner);

	switch (_actionIndex++) {
	case 1:
		owner->setPosition(owner->_position);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setPosition(owner->_position);
		owner->setFrame(1);
		break;
	default:
		break;
	}
}
	

/*--------------------------------------------------------------------------*/

void Scene200::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(200);
	setZoomPercents(0, 100, 200, 100);
	BF_GLOBALS._sound1.play(3);

	_object10.postInit();
	_object10.setVisage(200);
	_object10.setPosition(Common::Point(114, 102));
	_object10.setStrip(2);
	_object10.setFrame(1);
	_object10.changeZoom(100);

	_object1.postInit();
	_object1.hide();
	_object2.postInit();
	_object2.hide();
	_object3.postInit();
	_object3.hide();
	_object4.postInit();
	_object4.hide();
	_object5.postInit();
	_object5.hide();
	_object6.postInit();
	_object6.hide();

	_object11.postInit();
	_object11.setVisage(200);
	_object11.setPosition(Common::Point(96, 112), 1000);
	_object11.setStrip(3);
	_object11.setFrame(1); 
	_object11.changeZoom(100);

	_object10.setAction(&_action1);
	_object11.setAction(&_action2);
}

void Scene200::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 210 - Credits - Car Training
 *
 *--------------------------------------------------------------------------*/

void Scene210::Action1::signal() {
	Scene210 *scene = (Scene210 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(_owner);
	assert(owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);
		
		scene->setAction(&scene->_sequenceManager, this, 210, &scene->_object10, &scene->_object11,
			&scene->_object12, &scene->_object13, &scene->_object14, &scene->_object15, NULL);
		break;
	}
	case 3:
		BF_GLOBALS._sceneManager.changeScene(220);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene210::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(210);
	BF_GLOBALS._scenePalette.loadPalette(235);
	BF_GLOBALS._scenePalette.refresh();

	_object9.postInit();
	_object9.setVisage(210);
	_object9.setPosition(Common::Point(146, 151));
	_object9.setStrip(1);
	_object9.setFrame(1);
	_object9.changeZoom(100);
	_object9.setAction(&_action1);

	_object10.postInit();
	_object10.hide();
	_object11.postInit();
	_object11.hide();
	_object12.postInit();
	_object12.hide();
	_object13.postInit();
	_object13.hide();
	_object14.postInit();
	_object14.hide();
	_object15.postInit();
	_object15.hide();
}

void Scene210::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 220 - Credits - Martial Arts
 *
 *--------------------------------------------------------------------------*/

void Scene220::Action1::signal() {
	Scene220 *scene = (Scene220 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(30);
		break;
	case 2:
		BF_GLOBALS._scenePalette.clearListeners();
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_object2.setVisage(221);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(178, 122));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 4:
		BF_GLOBALS._scenePalette.loadPalette(2);
		BF_GLOBALS._scenePalette.refresh();
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(5);
		break;
	case 5:
		scene->_object1.remove();

		scene->_object2.setVisage(222);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(164, 138));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 6:
		scene->_object2.setVisage(223);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(164, 139));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 7:
		scene->_object2.setVisage(230);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(168, 136));
		scene->_object2.animate(ANIM_MODE_4, 13, 1, this);
		break;
	case 8:
		scene->_object2.animate(ANIM_MODE_5, this);

		scene->_object3.postInit();
		scene->_object3.setVisage(231);
		scene->_object3.setPosition(Common::Point(65, 179));
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(1);
		scene->_object3.changeZoom(100);
		scene->_object3.setAction(&scene->_action2, this);
		break;
	case 9:
		break;
	case 10:
		scene->_object2.setVisage(224);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(148, 143));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 11: {
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);
		
		scene->setAction(&scene->_sequenceManager, this, 220, &scene->_object4, &scene->_object5,
			&scene->_object6, &scene->_object7, &scene->_object8, &scene->_object9, NULL);
		break;
	}
	case 12:
		scene->_object2.setVisage(232);
		scene->_object2.setFrame(1);
		scene->_object2.setPosition(Common::Point(146, 143));
		scene->_object2._numFrames = 5;
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 13:
		scene->_object2.setVisage(228);
		scene->_object2.setFrame(1);
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 14:
		scene->_object2.setVisage(229);
		scene->_object2.setFrame(1);
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 15:
		BF_GLOBALS._sceneManager.changeScene(225);
		break;
	default:
		break;
	}
}

void Scene220::Action2::signal() {
	Scene220 *scene = (Scene220 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.setPosition(scene->_object3._position);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 1:
		setDelay(50);
		break;
	case 2:
		scene->_object3.remove();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene220::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(220);

	_object2.postInit();
	_object2.setVisage(220);
	_object2.setPosition(Common::Point(182, 122));
	_object2.setStrip(1);
	_object2.setFrame(1);
	_object2.changeZoom(100);

	_object1.postInit();
	_object1.setVisage(220);
	_object1.setPosition(Common::Point(164, 138));
	_object1.setStrip(2);
	_object1.setFrame(1);
	_object1.changeZoom(100);

	_object4.postInit();
	_object4.hide();
	_object5.postInit();
	_object5.hide();
	_object6.postInit();
	_object6.hide();
	_object7.postInit();
	_object7.hide();
	_object8.postInit();
	_object8.hide();
	_object9.postInit();
	_object9.hide();

	_object2.setAction(&_action1);
}

void Scene220::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}

/*--------------------------------------------------------------------------
 * Scene 225 - Credits - Gun Training
 *
 *--------------------------------------------------------------------------*/

void Scene225::Action1::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		BF_GLOBALS._scenePalette.loadPalette(235);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(5);
		break;
	case 2:
		owner->animate(ANIM_MODE_4, 7, 1, this);
		break;
	case 3:
		scene->_object8.animate(ANIM_MODE_5, this);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_object8.remove();
		break;
	case 5:
		scene->_action3.signal();
		break;
	case 6:
		owner->setPosition(Common::Point(owner->_position.x, owner->_position.y - 4));
		owner->setStrip(2);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 7:
		scene->_action2.signal();
		break;
	case 8:
		owner->animate(ANIM_MODE_6, this);
		break;
	case 9:
		owner->setPosition(Common::Point(owner->_position.x - 2, owner->_position.y - 1));
		owner->setStrip(3);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 10: {
		owner->setPosition(Common::Point(owner->_position.x + 10, owner->_position.y + 4));
		owner->setStrip(4);
		owner->setFrame(1);
		owner->fixPriority(116);
		owner->animate(ANIM_MODE_1, NULL);

		Common::Point destPos(138, 117);
		NpcMover *mover = new NpcMover();
		owner->addMover(mover, &destPos, this);
		break;
	}
	case 11: {
		owner->setPosition(Common::Point(owner->_position.x - 12, owner->_position.y - 1));
		owner->setStrip(5);
		owner->setFrame(1);
		owner->_moveDiff.x = 8;

		Common::Point destPos(402, 116);
		NpcMover *mover = new NpcMover();
		owner->addMover(mover, &destPos, this);

		BF_GLOBALS._player.setPosition(Common::Point(owner->_position.x, 0));
		ADD_MOVER_NULL(BF_GLOBALS._player, 500, 0);
		break;
	}
	case 12:
		owner->setVisage(1227);
		owner->setStrip(1);
		owner->setFrame(1);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 13:
		owner->setStrip(2);
		owner->setFrame(4);
		scene->_action4.signal();
		break;
	case 14:
		owner->animate(ANIM_MODE_6, this);
		break;
	case 15:
		scene->_action6.signal();
		break;
	case 16:
		owner->animate(ANIM_MODE_4, 4, 1, this);
		break;
	case 17:
		owner->setFrame(6);
		owner->animate(ANIM_MODE_5, this);
		break;
	case 18:
		scene->_action5.signal();
		break;
	case 19:
		owner->animate(ANIM_MODE_4, 4, -1, this);
		break;
	case 20:
		PaletteRotation *rot;
		rot = BF_GLOBALS._scenePalette.addRotation(64, 79, 1);
		rot->setDelay(10);
		rot = BF_GLOBALS._scenePalette.addRotation(96, 111, 1);
		rot->setDelay(10);
		
		scene->setAction(&scene->_sequenceManager, this, 225, &scene->_object15, &scene->_object16,
			&scene->_object17, &scene->_object18, &scene->_object19, &scene->_object20, NULL);
		break;
	case 21:
		scene->_object21.hide();
		BF_GLOBALS._player._moveDiff.x = 5;
		BF_GLOBALS._sceneManager.changeScene(265);
		break;
	default:
		break;
	}
}

void Scene225::Action2::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(2);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action3::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(1);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action4::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(5);
		owner->setFrame(4);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action5::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(6);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

void Scene225::Action6::signal() {
	Scene225 *scene = (Scene225 *)BF_GLOBALS._sceneManager._scene;
	SceneObject *owner = static_cast<SceneObject *>(this->_owner);

	switch (_actionIndex++) {
	case 0:
		break;
	case 1:
		owner->animate(ANIM_MODE_5, this);
		break;
	case 2:
		owner->setStrip(3);
		owner->setFrame(3);
		scene->_action1.signal();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene225::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(1225);
	loadBackground(-320, 0);

	_object8.postInit();
	_object8.setVisage(1225);
	_object8.setPosition(Common::Point(114, 119));
	_object8._frame = 1;
	_object8.setStrip(2);
	_object8.changeZoom(100);

	_object9.postInit();
	_object9.setVisage(1226);
	_object9.setPosition(Common::Point(83, 128));
	_object9.setStrip(1);
	_object9.changeZoom(100);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setPosition(Common::Point(83, 0));
	BF_GLOBALS._player.hide();

	_object10.postInit();
	_object10.setVisage(1225);
	_object10.setPosition(Common::Point(237, 29));
	_object10.setStrip(1);
	_object10._frame = 1;
	_object10.changeZoom(100);
	_object10._numFrames = 2;

	_object11.postInit();
	_object11.setVisage(1225);
	_object11.setPosition(Common::Point(290, 47));
	_object11.setStrip(1);
	_object11._frame = 1;
	_object11.changeZoom(100);
	_object11._numFrames = 2;
	
	_object12.postInit();
	_object12.setVisage(1225);
	_object12.setPosition(Common::Point(368, 35));
	_object12.setStrip(4);
	_object12._frame = 1;
	_object12.changeZoom(100);
	_object12._numFrames = 2;

	_object13.postInit();
	_object13.setVisage(1225);
	_object13.setPosition(Common::Point(416, 33));
	_object13.setStrip(1);
	_object13._frame = 1;
	_object13.changeZoom(100);
	_object13._numFrames = 2;

	_object14.postInit();
	_object14.setVisage(1225);
	_object14.setPosition(Common::Point(476, 30));
	_object14.setStrip(1);
	_object14._frame = 1;
	_object14.changeZoom(100);
	_object14._numFrames = 2;

	_object21.postInit();
	_object21.setVisage(235);
	_object21.setStrip(1);
	_object21._frame = 1;
	_object21.setPosition(Common::Point(498, 41));
	_object21.changeZoom(100);
	_object21.hide();

	_object15.postInit();
	_object15.hide();
	_object16.postInit();
	_object16.hide();
	_object17.postInit();
	_object17.hide();
	_object18.postInit();
	_object18.hide();
	_object19.postInit();
	_object19.hide();
	_object20.postInit();
	_object20.hide();

	_object9.setAction(&_action1);
	_object10.setAction(&_action2);
	_object11.setAction(&_action3);
	_object12.setAction(&_action4);
	_object13.setAction(&_action5);
	_object14.setAction(&_action6);
}

void Scene225::remove() {
	BF_GLOBALS._scenePalette.clearListeners();
	SceneExt::remove();
}


} // End of namespace BlueForce

} // End of namespace TsAGE
