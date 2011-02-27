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

#include "tsage/ringworld_scenes2.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 1000 - Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene1000::Action1::signal() {
	Scene1000 *scene = (Scene1000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		scene->_object4.postInit();
		scene->_object4.setVisage(1001);
		scene->_object4._frame = 1;
		scene->_object4.setStrip2(5);
		scene->_object4.changeZoom(100);
		scene->_object4.animate(ANIM_MODE_2, NULL);
		scene->_object4.setPosition(Common::Point(403, 163));
		setDelay(90);
		break;
	case 2: {
		SceneItem::display(0, 0);
		scene->_object4.remove();
		scene->_object1.changeZoom(-1);
		NpcMover *mover = new NpcMover();
		Common::Point pt(180, 100);
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 3:
		_globals->_sceneManager.changeScene(1400);
		break;
	}

}

void Scene1000::Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		SceneItem::display(1000, 0, SET_Y, 20, SET_FONT, 2, SET_BG_COLOUR, -1,
				SET_EXT_BGCOLOUR, 35, SET_WIDTH, 200, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(180);
		break;
	case 2:
		SceneItem::display(0, 0);
		_globals->_sceneManager.changeScene(2000);
		break;
	default:
		break;
	}
}

void Scene1000::Action3::signal() {
	Scene1000 *scene = (Scene1000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_sceneManager._scene->loadBackground(0, 0);
		setDelay(60);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(158, 31);
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 2:
	case 3:
		setDelay(60);
		break;
	case 4:
		_globals->_player.unflag100();
		setDelay(240);
		break;
	case 5: {
		// Intro.txt file presence is used to allow user option to skip the introduction
		_globals->_player.enableControl();
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading("Intro.txt");
		if (!in) {
			// File not present, so create it
			Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving("Intro.txt");
			out->finalize();
			delete out;
			setDelay(1);
		} else {
			delete in;

			// Prompt user for whether to start play or watch introduction
			if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
				_actionIndex = 20;
				_globals->_soundHandler.proc1(this);
			} else {
				setDelay(1);
			}

			_globals->_player.disableControl();
		}
		break;
	}
	case 6: {
		scene->_object3.remove();
		_globals->_player.setStrip2(2);
		NpcMover *mover = new NpcMover();
		Common::Point pt(480, 100);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		_globals->_scenePalette.loadPalette(1002);
		_globals->_scenePalette.refresh();
		_globals->_scenePalette.addRotation(80, 95, -1);
		scene->_object3.postInit();
		scene->_object3.setVisage(1002);
		scene->_object3.setStrip(1);
		scene->_object3.setPosition(Common::Point(284, 122));
		scene->_object3.changeZoom(1);

		zoom(true);
		setDelay(200);
		break;
	case 8:
		zoom(false);
		setDelay(10);
		break;
	case 9:
		scene->_object3.setStrip(2);
		scene->_object3.setPosition(Common::Point(285, 155));

		zoom(true);
		setDelay(400);
		break;
	case 10:
		zoom(false);
		setDelay(10);
		break;
	case 11:
		scene->_object3.setStrip(3);
		scene->_object3.setPosition(Common::Point(279, 172));

		zoom(true);
		setDelay(240);
		break;
	case 12:
		zoom(false);
		setDelay(10);
		break;
	case 13:
		scene->_object3.setStrip(4);
		scene->_object3.setPosition(Common::Point(270, 128));

		zoom(true);
		setDelay(300);
		break;
	case 14:
		zoom(false);
		setDelay(10);
		break;
	case 15:
		scene->_object3.setStrip(1);
		scene->_object3.setFrame(2);
		scene->_object3.setPosition(Common::Point(283, 137));

		zoom(true);
		setDelay(300);
		break;
	case 16:
		zoom(false);
		setDelay(10);
		break;
	case 17:
		scene->_object3.setStrip(5);
		scene->_object3.setFrame(1);
		scene->_object3.setPosition(Common::Point(292, 192));
		
		zoom(true);
		setDelay(300);
		break;
	case 18:
		zoom(false);
		_globals->_scenePalette.clearListeners();
		_globals->_soundHandler.proc1(this);
		break;
	case 19:
		_globals->_sceneManager.changeScene(10);
		break;
	case 20:
		_globals->_sceneManager.changeScene(30);
		break;
	default:
		break;
	}		
}

void Scene1000::Action3::zoom(bool up) {
	Scene1000 *scene = (Scene1000 *)_globals->_sceneManager._scene;

	if (up) {
		while ((scene->_object3._percent < 100) && !_vm->shouldQuit()) {
			scene->_object3.changeZoom(MIN(scene->_object3._percent + 5, 100));
			_globals->_sceneObjects->draw();
			_globals->_events.delay(1);
		}
	} else {
		while ((scene->_object3._percent > 0) && !_vm->shouldQuit()) {
			scene->_object3.changeZoom(MAX(scene->_object3._percent - 5, 0));
			_globals->_sceneObjects->draw();
			_globals->_events.delay(1);
		}
	}
}

/*--------------------------------------------------------------------------*/

void Scene1000::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	if (_globals->_sceneManager._previousScene == 2000) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(1, 1);
		_object1.setPosition(Common::Point(120, 180));

		setAction(&_action2);

		_globals->_sceneManager._scene->_sceneBounds.centre(_object1._position.x, _object1._position.y);
		_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
		
		_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
		_globals->_soundHandler.startSound(114);
	} else if (_globals->_sceneManager._previousScene == 2222) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(2, 2);
		_object1.setPosition(Common::Point(120, 180));
		
		_globals->_sceneManager._scene->_sceneBounds.centre(_object1._position.x, _object1._position.y);
		_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
		_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

		setAction(&_action1);
	} else {
		_globals->_soundHandler.startSound(4);
		setZoomPercents(0, 10, 30, 100);
		_object3.postInit();
		_object3.setVisage(1050);
		_object3.changeZoom(-1);
		_object3.setPosition(Common::Point(158, 0));
		
		_globals->_player.postInit();
		_globals->_player.setVisage(1050);
		_globals->_player.setStrip(3);
		_globals->_player.setPosition(Common::Point(160, 191));
		_globals->_player._moveDiff.x = 12;
		_globals->_player.flag100();
		_globals->_player.disableControl();

		_globals->_sceneManager._scene->_sceneBounds.centre(_object3._position.x, _object3._position.y);

		setAction(&_action3);
	}

	loadScene(1000);
}

/*--------------------------------------------------------------------------
 * Scene 1001 - Fleeing planet cutscene
 *
 *--------------------------------------------------------------------------*/

void Scene1001::Action1::signal() {
	Scene1001 *scene = (Scene1001 *)_globals->_sceneManager._scene;
	
	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(108, 171);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 3: {
		Common::Point pt(170, 159);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 4: {
		scene->_object2.postInit();
		scene->_object2.setVisage(16);
		scene->_object2.setStrip2(4);
		scene->_object2.setPosition(Common::Point(61, 177));
		scene->_object2.animate(ANIM_MODE_5, this);

		Common::Point pt(320, 100);
		NpcMover *mover = new NpcMover();
		scene->_object3.addMover(mover, &pt, this);
		break;
	}
	case 5: {
		Common::Point pt(82, 166);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		Common::Point pt(64, 149);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 7: {
		Common::Point pt(15, 136);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 8: {
		Common::Point pt(-5, 120);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 9: {
		scene->_object1.postInit();
		scene->_object1.setVisage(16);
		scene->_object1.setStrip2(1);
		scene->_object1.setFrame(1);
		scene->_object1.setPosition(Common::Point(-75, 87));
		scene->_object1.animate(ANIM_MODE_2, NULL);
		
		Common::Point pt(0, 100);
		NpcMover *mover = new NpcMover();
		scene->_object1.addMover(mover, &pt, this);
		break;
	}
	case 10: {
		Common::Point pt1(107, 115);
		NpcMover *mover1 = new NpcMover();
		scene->_object1.addMover(mover1, &pt1, NULL);

		scene->_object3.setVisage(16);
		scene->_object3.setStrip2(5);
		scene->_object3.setFrame2(2);
		scene->_object3.setPosition(Common::Point(220, 200));

		Common::Point pt2(187, 181);
		NpcMover *mover2 = new NpcMover();
		scene->_object3.addMover(mover2, &pt2, this);
		break;
	}
	case 11: {
		scene->_object2.setVisage(16);
		scene->_object2.setStrip2(5);
		scene->_object2.setFrame2(1);
		scene->_object2.setPosition(Common::Point(211, 0));

		Common::Point pt(189, 30);
		NpcMover *mover = new NpcMover();
		scene->_object2.addMover(mover, &pt, this);
		break;
	}
	case 12:
		scene->_stripManager.start(100, this);
		break;
	case 13: {
		scene->_object4.postInit();
		scene->_object4.setVisage(16);
		scene->_object4.setStrip2(2);
		scene->_object4.setFrame(4);
		scene->_object4.setPosition(Common::Point(360, 80));
		scene->_object4.animate(ANIM_MODE_2, NULL);

		Common::Point pt(303, 97);
		NpcMover *mover = new NpcMover();
		scene->_object4.addMover(mover, &pt, this);
		break;
	}
	case 14:
		scene->_stripManager.start(110, this);
		break;
	case 15:
		setDelay(10);
		break;
	case 16: {
		scene->_soundHandler1.startSound(90);

		scene->_object6.postInit();
		scene->_object6.setVisage(16);
		scene->_object6.setStrip2(6);
		scene->_object6.setFrame2(2);
		scene->_object6._moveDiff = Common::Point(20, 20);
		scene->_object6.setPriority2(20);
		scene->_object6.setPosition(Common::Point(scene->_object2._position.x - 6, scene->_object2._position.y + 7));
		scene->_object6.animate(ANIM_MODE_5, NULL);

		Common::Point pt(scene->_object6._position.x - 70, scene->_object6._position.y + 70);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		break;
	}		
	case 17: {
		scene->_soundHandler1.startSound(90);
		scene->_object6.remove();

		scene->_object7.postInit();
		scene->_object7.setVisage(16);
		scene->_object7.setStrip2(6);
		scene->_object7.setFrame2(1);
		scene->_object7._moveDiff = Common::Point(20, 20);
		scene->_object7.setPosition(Common::Point(scene->_object3._position.x - 28, scene->_object3._position.y - 11));
		scene->_object7.setPriority2(200);
		scene->_object7.animate(ANIM_MODE_5, NULL);

		Common::Point pt(scene->_object7._position.x - 70, scene->_object7._position.y - 70);
		NpcMover *mover = new NpcMover();
		scene->_object7.addMover(mover, &pt, this);
		break;
	}		
	case 18:
		scene->_object7.remove();

		scene->_object5.postInit();
		scene->_object5.setVisage(16);
		scene->_object5.setPosition(Common::Point(306, 93));
		scene->_object5._strip = 3;
		scene->_object5.setPriority2(200);
		scene->_object5.animate(ANIM_MODE_2, NULL);
		setDelay(30);
		break;
	case 19: {
		_globals->_soundHandler.startSound(91);
		byte adjustData[4] = {0xff, 0xff, 0xff, 0};
		_globals->_scenePalette.fade(adjustData, true, 0);

		scene->_object1._strip = 7;
		scene->_object1._frame = 1;
		scene->_object1.setPosition(Common::Point(314, 112));
		scene->_object1.addMover(NULL);
		setDelay(2);
	}
	case 20:
		_globals->_scenePalette.loadPalette(16);
		_globals->_scenePalette.refresh();
		setDelay(6);
		break;
	case 21:
		scene->_object1._numFrames = 15;
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 22:
		_globals->_soundHandler.startSound(92);
		scene->_stripManager.start(111, this);
		break;
	case 23:
		setDelay(60);
		break;
	case 24:
		_globals->_sceneManager.changeScene(2000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene1001::postInit(SceneObjectList *OwnerList) {
	loadScene(16);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerCText);
	_stripManager.addSpeaker(&_speakerCR);
	_stripManager.addSpeaker(&_speakerSL);
	_speakerQText._colour1 = 11;

	_object3.postInit();
	_object3.setVisage(16);
	_object3.setStrip2(4);
	_object3.setPosition(Common::Point(61, 177));

	_globals->_soundHandler.startSound(85);
	setAction(&_action1);
}


/*--------------------------------------------------------------------------
 * Scene 1250 - 
 *
 *--------------------------------------------------------------------------*/

void Scene1250::Action1::signal() {
	Scene1250 *scene = (Scene1250 *)_globals->_sceneManager._scene;
	
	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(120) + 60);
		break;
	case 1:
		scene->_object1.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

void Scene1250::Action2::signal() {
	Scene1250 *scene = (Scene1250 *)_globals->_sceneManager._scene;
	
	switch (_actionIndex++) {
	case 0:
		switch (_globals->_randomSource.getRandomNumber(2)) {
		case 0:
			scene->_object2.setPosition(Common::Point(163, 75));
			break;
		case 1:
			scene->_object2.setPosition(Common::Point(109, 65));
			break;
		case 2:
			scene->_object2.setPosition(Common::Point(267, 20));
			break;
		}

		setDelay(30);
		break;
	case 1:
		scene->_object2.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

void Scene1250::Action3::signal() {
	Scene1250 *scene = (Scene1250 *)_globals->_sceneManager._scene;
	
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(1251, this);
		break;
	case 2:
		setDelay(6);
		break;
	case 3:
		_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene1250::Action4::signal() {
	Scene1250 *scene = (Scene1250 *)_globals->_sceneManager._scene;
	
	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_stripManager.start(1250, this);
		break;
	case 2:
		setDelay(6);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene1250::postInit(SceneObjectList *OwnerList) {
	loadScene(1250);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_speakerQText._textPos = Common::Point(120, 120);
	_speakerQText._textWidth = 180;

	_object1.postInit();
	_object1.setVisage(1250);
	_object1.setPosition(Common::Point(126, 69));
	_object1.setStrip2(1);
	_object1._frame = 1;
	_object1.setAction(&_action1);

	_object2.postInit();
	_object2.setVisage(1250);
	_object2.setPosition(Common::Point(126, 69));
	_object2.setStrip2(2);
	_object2.setPriority2(255);
	_object2._frame = 1;
	_object2.setAction(&_action2);
	
	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	if ((_globals->_sceneManager._previousScene != 2000) || (_globals->_stripNum != 1250)) {
		setAction(&_action4);
	} else {
		setAction(&_action3);
		_globals->_soundHandler.startSound(114);
	}
}



} // End of namespace tSage
