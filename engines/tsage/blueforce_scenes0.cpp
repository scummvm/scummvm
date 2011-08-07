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

#include "tsage/blueforce_scenes0.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 20 - Tsunami Title Screen
 *
 *--------------------------------------------------------------------------*/

void BF_Scene20::Action1::signal() {
	BF_Scene20 *scene = (BF_Scene20 *)_globals->_sceneManager._scene;
	static byte black[3] = { 0, 0, 0 };

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		_sound.play(1);
		_globals->_scenePalette.addRotation(64, 127, -1, 1, this);
		break;
	case 2:
		scene->_object1.setVisage(22);
		scene->_object1._strip = 1;
		scene->_object1._frame = 1;
		scene->_object1.changeZoom(100);

		scene->_object2.setVisage(22);
		scene->_object2._strip = 2;
		scene->_object2._frame = 1;
		scene->_object2.changeZoom(100);

		scene->_object3.setVisage(22);
		scene->_object3._strip = 3;
		scene->_object3._frame = 1;
		scene->_object3.changeZoom(100);

		scene->_object4.setVisage(22);
		scene->_object4._strip = 4;
		scene->_object4._frame = 1;
		scene->_object4.changeZoom(100);

		scene->_object5.setVisage(22);
		scene->_object5._strip = 5;
		scene->_object5._frame = 1;
		scene->_object5.changeZoom(100);

		scene->_object6.setVisage(22);
		scene->_object6._strip = 6;
		scene->_object6._frame = 1;
		scene->_object6.changeZoom(100);

		scene->_object7.setVisage(22);
		scene->_object7._strip = 7;
		scene->_object7._frame = 1;
		scene->_object7.changeZoom(100);

		scene->_object8.setVisage(22);
		scene->_object8._strip = 8;
		scene->_object8._frame = 1;
		scene->_object8.changeZoom(100);

		setDelay(1);
		break;
	case 3:
		_globals->_scenePalette.addFader(scene->_scenePalette._palette, 256, 8, this);
		break;
	case 4:
		setDelay(60);
		break;
	case 5:
		scene->_object2.animate(ANIM_MODE_5, NULL);
		scene->_object3.animate(ANIM_MODE_5, NULL);
		scene->_object4.animate(ANIM_MODE_5, NULL);
		scene->_object5.animate(ANIM_MODE_5, NULL);
		scene->_object6.animate(ANIM_MODE_5, NULL);
		scene->_object7.animate(ANIM_MODE_5, this);
		break;
	case 6:
		setDelay(120);
		break;
	case 7:
		_globals->_scenePalette.addFader(black, 1, 5, this);
		break;
	case 8:
		_globals->_sceneManager.changeScene(100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void BF_Scene20::postInit(SceneObjectList *OwnerList) {
	loadScene(20);
	Scene::postInit();
	setZoomPercents(60, 85, 200, 100);

	preloadVisage(21);
	preloadVisage(22);
	_scenePalette.loadPalette(1);
	_scenePalette.loadPalette(22);

	_object1.postInit();
	_object1.setVisage(21);
	_object1._strip = 1;
	_object1._frame = 1;
	_object1.animate(ANIM_MODE_NONE, NULL);
	_object1.setPosition(Common::Point(62, 85));
	_object1.changeZoom(100);

	_object2.postInit();
	_object2.setVisage(21);
	_object2._strip = 2;
	_object2._frame = 1;
	_object2.animate(ANIM_MODE_NONE, NULL);
	_object2.setPosition(Common::Point(27, 94));
	_object2.changeZoom(100);

	_object3.postInit();
	_object3.setVisage(21);
	_object3._strip = 2;
	_object3._frame = 2;
	_object3.animate(ANIM_MODE_NONE, NULL);
	_object3.setPosition(Common::Point(68, 94));
	_object3.changeZoom(100);

	_object4.postInit();
	_object4.setVisage(21);
	_object4._strip = 2;
	_object4._frame = 3;
	_object4.animate(ANIM_MODE_NONE, NULL);
	_object4.setPosition(Common::Point(110, 94));
	_object4.changeZoom(100);

	_object5.postInit();
	_object5.setVisage(21);
	_object5._strip = 2;
	_object5._frame = 4;
	_object5.animate(ANIM_MODE_NONE, NULL);
	_object5.setPosition(Common::Point(154, 94));
	_object5.changeZoom(100);

	_object6.postInit();
	_object6.setVisage(21);
	_object6._strip = 2;
	_object6._frame = 5;
	_object6.animate(ANIM_MODE_NONE, NULL);
	_object6.setPosition(Common::Point(199, 94));
	_object6.changeZoom(100);

	_object7.postInit();
	_object7.setVisage(21);
	_object7._strip = 2;
	_object7._frame = 6;
	_object7.animate(ANIM_MODE_NONE, NULL);
	_object7.setPosition(Common::Point(244, 94));
	_object7.changeZoom(100);

	_object8.postInit();
	_object8.setVisage(21);
	_object8._strip = 2;
	_object8._frame = 7;
	_object8.animate(ANIM_MODE_NONE, NULL);
	_object8.setPosition(Common::Point(286, 94));
	_object8.changeZoom(100);

	setAction(&_action1);
}

} // End of namespace tSage
