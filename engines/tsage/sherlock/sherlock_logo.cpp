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

#ifdef TSAGE_SHERLOCK_ENABLED
#include "tsage/sherlock/sherlock_logo.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"

namespace TsAGE {

namespace Sherlock {

void SherlockLogo::start() {
	GLOBALS._gfxFontNumber = -1;
	GLOBALS.gfxManager().setDefaults();

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

void SherlockSceneHandler::postInit(SceneObjectList *OwnerList) {
	_delayTicks = 2;

	GLOBALS._soundManager.postInit();
	GLOBALS._soundManager.buildDriverList(true);
	GLOBALS._soundManager.installConfigDrivers();

	GLOBALS._sceneManager.setNewScene(10);
	GLOBALS._game->start();
}

/*--------------------------------------------------------------------------*/

void Object::setVisage(const Common::String &name) {
	int visageNum = atoi(name.c_str());
	SceneObject::setVisage(visageNum);
}

/*--------------------------------------------------------------------------*/

void SherlockLogoScene::Action1::signal() {
	SherlockLogoScene &scene = *(SherlockLogoScene *)GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		// Load scene palette
		GLOBALS._scenePalette.loadPalette(1111);
		GLOBALS._scenePalette.loadPalette(1);
		GLOBALS._scenePalette.refresh();
		setDelay(1);
		break;

	case 1:
		// Fade in the spotlight background
		GLOBALS._scenePalette.addFader(scene._palette1._palette, 256, 6, this);
		break;

	case 2:
		// First half of square, circle, and triangle bouncing
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
		// Remainder of bouncing square, circle, and triangle coming to rest
		scene._object1._strip = 2;
		scene._object1._frame = 1;
		scene._object1.changeZoom(100);
		scene._object1.animate(ANIM_MODE_4, 11, 1, this);
		break;

	case 4:
		// Fade out background without fading out the shapes
		GLOBALS._scenePalette.addFader(scene._palette2._palette, 256, 6, this);
		break;

	case 5:
		scene._backSurface.fillRect(scene._sceneBounds, 0);
		scene._gfxManager2.activate();
		scene._gfxManager2.fillRect(scene._sceneBounds, 0);
		scene._gfxManager2.deactivate();
		//word_2B4AA = 3;
		setDelay(10);
		break;

	case 6:
		GLOBALS._scenePalette.loadPalette(12);
		GLOBALS._scenePalette.refresh();
		setDelay(1);
		break;

	case 7:
		// Animation of shapes expanding upwards to form larger EA logo
		scene._object1.setVisage("0012.vis");
		scene._object1._strip = 1;
		scene._object1._frame = 1;
		scene._object1.changeZoom(100);
		scene._object1.setPosition(Common::Point(170, 142));
		scene._object1._numFrames = 7;
		scene._object1.animate(ANIM_MODE_5, (const void *)nullptr);
		ADD_MOVER(scene._object1, 158, 71);
		break;

	case 8:
		GLOBALS._scenePalette.addFader(scene._palette3._palette, 256, 40, this);
		break;

	case 9:
		// Show 'Electronic Arts' company name
		scene._object2.postInit(nullptr);
		scene._object2.setVisage("0014.vis");
		scene._object2._strip = 1;
		scene._object2._frame = 1;
		scene._object2.setPosition(Common::Point(152, 98));
		scene._object2.changeZoom(100);
		scene._object2.animate(ANIM_MODE_NONE, (const void *)nullptr);
		setDelay(120);
		break;

	case 10:
		// Remainder of steps is positioning and sizing hand cursorin an arc
		scene._object3.postInit();
		scene._object3.setVisage("0018.vis");
		scene._object3._strip = 1;
		scene._object3._frame = 1;
		scene._object3.setPosition(Common::Point(33, 91));
		scene._object3.changeZoom(100);
		scene._object3.animate(ANIM_MODE_NONE, (const void *)nullptr);
		setDelay(5);
		break;

	case 11:
		scene._object3._frame = 2;
		scene._object3.setPosition(Common::Point(44, 124));
		setDelay(5);
		break;

	case 12:
		scene._object3._frame = 3;
		scene._object3.setPosition(Common::Point(64, 153));
		setDelay(5);
		break;

	case 13:
		scene._object3._frame = 4;
		scene._object3.setPosition(Common::Point(87, 174));
		setDelay(5);
		break;

	case 14:
		scene._object3._frame = 5;
		scene._object3.setPosition(Common::Point(114, 191));
		setDelay(5);
		break;

	case 15:
		scene._object3._frame = 6;
		scene._object3.setPosition(Common::Point(125, 184));
		setDelay(5);
		break;

	case 16:
		scene._object3._frame = 7;
		scene._object3.setPosition(Common::Point(154, 187));
		setDelay(5);
		break;

	case 17:
		scene._object3._frame = 8;
		scene._object3.setPosition(Common::Point(181, 182));
		setDelay(5);
		break;

	case 18:
		scene._object3._frame = 9;
		scene._object3.setPosition(Common::Point(191, 167));
		setDelay(5);
		break;

	case 19:
		scene._object3._frame = 10;
		scene._object3.setPosition(Common::Point(190, 150));
		setDelay(5);
		break;

	case 20:
		scene._object3._frame = 11;
		scene._object3.setPosition(Common::Point(182, 139));
		setDelay(5);
		break;

	case 21:
		scene._object3._frame = 11;
		scene._object3.setPosition(Common::Point(170, 130));
		setDelay(5);
		break;

	case 22:
		scene._object3._frame = 11;
		scene._object3.setPosition(Common::Point(158, 121));
		setDelay(8);
		break;

	case 23:
		// Show a highlighting of the company name
		scene._object3.hide();
		scene._object4.show();
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;

	case 24:
		scene._object4._frame = 2;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;

	case 25:
		scene._object2.remove();
		setDelay(1);
		break;

	case 26:
		scene._object4._frame = 3;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;

	case 27:
		scene._object4._frame = 4;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;
		break;

	case 28:
		scene._object4._frame = 5;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;
		break;

	case 29:
		scene._object4._frame = 6;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;
		break;

	case 30:
		scene._object4._frame = 7;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;
		break;

	case 31:
		scene._object4._frame = 8;
		scene._object4.setPosition(Common::Point(155, 94));
		setDelay(8);
		break;

	case 32:
		setDelay(180);
		break;

	default:
		scene.finish();
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void SherlockLogoScene::postInit(SceneObjectList *OwnerList) {
	loadScene(10);
	Scene::postInit(OwnerList);

	_palette1.loadPalette(1111);
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
	_object4.animate(ANIM_MODE_NONE, (const void *)nullptr);
	_object4.hide();

	setAction(&_action1);
}

void SherlockLogoScene::finish() {
	g_vm->quitGame();
}

} // End of namespace Sherlock

} // End of namespace TsAGE

#endif
