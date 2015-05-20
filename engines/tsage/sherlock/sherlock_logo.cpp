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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "tsage/sherlock/sherlock_logo.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"

namespace TsAGE {

namespace Sherlock {

void SherlockLogo::start() {
	// Start the demo's single scene
	g_globals->_sceneManager.changeScene(1);

	g_globals->_events.setCursor(CURSOR_NONE);
}

Scene *SherlockLogo::createScene(int sceneNumber) {
	// The demo only has a single scene, so ignore the scene number and always return it
	return new SherlockLogoScene();
}

bool SherlockLogo::canLoadGameStateCurrently() {
	return false;
}

bool SherlockLogo::canSaveGameStateCurrently() {
	return false;
}

void SherlockLogo::processEvent(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN || (event.eventType == EVENT_KEYPRESS &&
			event.kbd.keycode == Common::KEYCODE_ESCAPE))
		quitGame();
}

void SherlockLogo::quitGame() {
	g_vm->quitGame();
}

/*--------------------------------------------------------------------------*/

void SherlockLogoScene::Action1::signal() {
	SherlockLogoScene &scene = *(SherlockLogoScene *)GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		GLOBALS._scenePalette.loadPalette(1111);
		GLOBALS._scenePalette.loadPalette(1);
		GLOBALS._scenePalette.refresh();
		setDelay(1);
		break;

	case 1:
		GLOBALS._scenePalette.addFader(scene._palette1._palette, 256, 6, this);
		break;

	case 2:
		scene._object1.postInit();
		scene._object1.setVisage("0016.vis");
		scene._object1._strip = 1;
		scene._object1._frame = 1;
		scene._object1.setPosition(Common::Point(169, 107));
		scene._object1.changeZoom(100);
		scene._object1._numFrames = 7;
		scene._object1.animate(ANIM_MODE_5, this);
		break;

	case 3:
		scene._object1._strip = 2;
		scene._object1._strip = 1;
		scene._object1.changeZoom(100);
		scene._object1.animate(ANIM_MODE_4, 4, 11, 1, this);
		break;

	case 4:
		GLOBALS._scenePalette.addFader(scene._palette2._palette, 256, 6, this);
		break;

	case 5:
		scene._rect1 = Rect(0, 26, 312, 190);
		scene._gfxManager2.activate();
		scene._gfxManager2.fillRect(scene._rect1, 0);
		scene._gfxManager2.deactivate();
		//word_2B4AA = 3;

		setDelay(10);
		break;

	case 6:
		GLOBALS._scenePalette.loadPalette(12);
		GLOBALS._scenePalette.refresh();
		break;

	// TODO

	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void SherlockLogoScene::postInit(SceneObjectList *OwnerList) {
	loadScene(10);
	Scene::postInit(OwnerList);

	_palette1.loadPalette(10);
	_palette2.loadPalette(1111);
	_palette2.loadPalette(1);
	_palette3.loadPalette(1111);
	_palette3.loadPalette(14);

	_object4.postInit();
	_object4.setVisage("0019.vis");
	_object4._strip = 1;
	_object4._frame = 1;
	_object4.setPosition(Common::Point(155, 94));
	_object4.changeZoom(100);
	_object4.animate(ANIM_MODE_NONE, nullptr);
	_object4.hide();

	setAction(&_action1);
}


} // End of namespace Sherlock

} // End of namespace TsAGE
