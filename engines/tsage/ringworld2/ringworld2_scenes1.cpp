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
} // End of namespace Ringworld2
} // End of namespace TsAGE
