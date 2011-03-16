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
