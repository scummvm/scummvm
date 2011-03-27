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

#include "graphics/cursorman.h"
#include "tsage/ringworld_scenes10.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 9750
 *
 *--------------------------------------------------------------------------*/
void Scene9750::signal() {
	switch (_sceneMode ++) {
	case 9751:
		_globals->_soundHandler.proc1(this);
		break;
	case 9752:
		_globals->_sceneManager.changeScene(2100);		
	default:
		break;
	}
}

void Scene9750::dispatch() {
	Scene::dispatch();
}

void Scene9750::postInit(SceneObjectList *OwnerList) {
	loadScene(9750);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	
	_globals->_player.postInit();
	_object1.postInit();
	_object1.flag100();
	_object2.postInit();
	_object2.flag100();
	_globals->_player.disableControl();
	_sceneMode = 9751;
	setAction(&_sequenceManager, this, 9751, &_globals->_player, &_object1, &_object2, 0);
}

/*--------------------------------------------------------------------------
 * Scene 9999
 *
 *--------------------------------------------------------------------------*/

void Scene9999::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(600);
		break;
	case 1:
		_globals->_sceneManager.changeScene(3500);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene9999::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		SceneItem::display(9999, 0, SET_Y, 10, SET_X, 30, SET_FONT, 2, SET_BG_COLOUR, -1, SET_EXT_BGCOLOUR, 23, SET_WIDTH, 260, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(300);
		break;
	case 2:
		_globals->_stripNum = 3600;
		_globals->_sceneManager.changeScene(3600);
	default:
		break;
	}
}

void Scene9999::postInit(SceneObjectList *OwnerList) {
	loadScene(9998);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	
	_object1.postInit();
	_object1.setVisage(1303);
	_object1.setStrip2(3);
	_object1.setPosition(Common::Point(160, 152), 0);
	
	_globals->_player.postInit();
	_globals->_player.setVisage(1303);
	_globals->_player.setStrip2(1);
	_globals->_player.setPriority2(250);
	_globals->_player.animate(ANIM_MODE_2, 0);
	_globals->_player.setPosition(Common::Point(194, 98), 0);
	_globals->_player._numFrames = 20;
	_globals->_player.disableControl();

	_object2.postInit();
	_object2.setVisage(1303);
	_object2.setStrip2(2);
	_object2.setPriority2(2);
	_object2.setPosition(Common::Point(164, 149), 0);

	_object3.postInit();
	_object3.setVisage(1303);
	_object3.setStrip2(2);
	_object3.setPriority2(2);
	_object3.setFrame(2);
	_object3.setPosition(Common::Point(292, 149), 0);
	_object3.setAction(&_action3);

	if (_globals->_sceneManager._previousScene == 3500)
		setAction(&_action2);
	else
		setAction(&_action1);

	_globals->_sceneManager._scene->_sceneBounds.centre(_globals->_player._position.x, _globals->_player._position.y);
	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	if (_globals->_sceneManager._previousScene == 3500)
		_globals->_stripNum = 2222;
	else
		_globals->_stripNum = 2121;

	_globals->_soundHandler.startSound(118, 0, 127);

}

} // End of namespace tSage
