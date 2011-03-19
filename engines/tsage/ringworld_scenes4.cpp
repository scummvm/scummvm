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
 * $URL$
 * $Id$
 *
 */

#include "common/config-manager.h"
#include "tsage/ringworld_scenes4.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 3500 - Ringworld Scan
 *
 *--------------------------------------------------------------------------*/

void Scene3500::Action1::signal() {
	Scene3500 *scene = (Scene3500 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(3500, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene3500::Action2::signal() {
	Scene3500 *scene = (Scene3500 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_stripManager.start(3501, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2012);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene3500::postInit(SceneObjectList *OwnerList) {
	loadScene((_globals->_stripNum == 3600) ? 3600 : 3500);
	Scene::postInit();

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);

	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.top / 160) * 160;

	setAction((_globals->_stripNum == 3600) ? (Action *)&_action2 : (Action *)&_action1);
}

} // End of namespace tSage
