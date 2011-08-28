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

#include "common/config-manager.h"
#include "tsage/blue_force/blueforce_scenes1.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 100 - Tsunami Title Screen #2
 *
 *--------------------------------------------------------------------------*/

void Scene100::Text::dispatch() {
	SceneText::dispatch();

	// Keep the second text string below the first one
	Scene100 *scene = (Scene100 *)BF_GLOBALS._sceneManager._scene;
	Common::Point &pt = scene->_action1._sceneText1._position;
	scene->_action1._sceneText2.setPosition(Common::Point(pt.x, 
		pt.y + scene->_action1._textHeight));
}


void Scene100::Action1::signal() {
	static byte black[3] = { 0, 0, 0 };

	switch (_actionIndex++) {
	case 0:
		_state = 0;
		setDelay(6);
		break;
	case 1: {
		Common::String msg1 = _resourceManager->getMessage(100, _state++);
		if (msg1.compareTo("LASTCREDIT")) {
			Common::String msg2 = _resourceManager->getMessage(100, _state++);
			setTextStrings(msg1, msg2, this);
			--_actionIndex;
		} else {
			setTextStrings(BF_NAME, BF_ALL_RIGHTS_RESERVED, this);
			
			Common::Point pt(_sceneText1._position.x, 80);
			NpcMover *mover = new NpcMover();
			_sceneText1.addMover(mover, &pt, this);
		}
		break;
	}
	case 2:
		setDelay(600);
		break;
	case 3:
		BF_GLOBALS._sound1.fade(0, 10, 10, 1, this);
		GLOBALS._scenePalette.addFader(black, 1, 2, NULL);
		break;
	case 4:
		error("??exit");
		break;
	}
}

void Scene100::Action1::setTextStrings(const Common::String &msg1, const Common::String &msg2, Action *action) {
	// Set data for first text control
	_sceneText1._fontNumber = 10;
	_sceneText1._width = 160;
	_sceneText1._textMode = ALIGN_RIGHT;
	_sceneText1._color1 = BF_GLOBALS._scenePalette._colors.foreground;
	_sceneText1._color2 = BF_GLOBALS._scenePalette._colors.background;
	_sceneText1._color3 = BF_GLOBALS._scenePalette._colors.foreground;
	_sceneText1.setup(msg1);
	_sceneText1.fixPriority(255);
	_sceneText1.setPosition(Common::Point(
		(SCREEN_WIDTH - _sceneText1.getFrame().getBounds().width()) / 2, 202));
	_sceneText1._moveRate = 30;
	_sceneText1._moveDiff.y = 1;

	// Set data for second text control
	_sceneText2._fontNumber = 10;
	_sceneText2._width = _sceneText1._width;
	_sceneText2._textMode = _sceneText1._textMode;
	_sceneText2._color1 = _sceneText1._color1;
	_sceneText2._color2 = 31;
	_sceneText2._color3 = _sceneText1._color3;
	_sceneText2.setup(msg2);
	_sceneText2.fixPriority(255);
	GfxSurface textSurface = _sceneText2.getFrame();
	_sceneText2.setPosition(Common::Point((SCREEN_WIDTH - textSurface.getBounds().width()) / 2, 202));
	_sceneText2._moveRate = 30;
	_sceneText2._moveDiff.y = 1;
	
	_textHeight = textSurface.getBounds().height();
	int yp = -(_textHeight * 2);

	Common::Point pt(_sceneText1._position.x, yp); 
	NpcMover *mover = new NpcMover();
	_sceneText1.addMover(mover, &pt, action); 
}

void Scene100::Action2::signal() {
	Scene100 *scene = (Scene100 *)_globals->_sceneManager._scene;
	static byte black[3] = {0, 0, 0};

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._scenePalette.addFader(black, 1, -2, this);
		break;
	case 1:
		setDelay(180);
		break;
	case 2: {
		const char *SEEN_INTRO = "seen_intro";
		if (!ConfMan.hasKey(SEEN_INTRO) || !ConfMan.getBool(SEEN_INTRO)) {
			// First time being played, so will need to show the intro
			ConfMan.setBool(SEEN_INTRO, true);
			ConfMan.flushToDisk();
		} else {
			// Prompt user for whether to start play or watch introduction
			_globals->_player.enableControl();

			if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
				// Signal to start the game
				scene->_index = 190;
				remove();
				return;
			}
		}

		// At this point the introduction needs to start
		_globals->_scenePalette.addFader(black, 1, 2, this);
		break;
	}
	case 3:
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene100::Scene100(): SceneExt() {
	_index = 0;
}

void Scene100::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._scenePalette.loadPalette(2);
	BF_GLOBALS._v51C44 = 1;
	Scene::postInit();
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_globals->_player.enableControl();
	_globals->_player.hide();
	_globals->_player.disableControl();
	_index = 109;

	if (BF_GLOBALS._dayNumber < 6) {
		// Title
		loadScene(100);
		BF_GLOBALS._sound1.play(2);
		setAction(&_action2, this);
	} else {
		// Credits
		loadScene(101);
		BF_GLOBALS._sound1.play(118);
		setAction(&_action1, this);
	}
}

void Scene100::signal() {
	++_sceneMode;
	if (BF_GLOBALS._dayNumber < 6) {
		BF_GLOBALS._scenePalette.clearListeners();
		BF_GLOBALS._scenePalette.loadPalette(100);
		BF_GLOBALS._sceneManager.changeScene(_index);
	} else {
		if (_sceneMode > 1)
			BF_GLOBALS._events.setCursor(CURSOR_ARROW);

		setAction(this, &_action1, this);
	}
}

/*--------------------------------------------------------------------------
 * Scene 109 - Introduction Bar Room
 *
 *--------------------------------------------------------------------------*/

void Scene109::Action1::signal() {
	Scene109 *scene = (Scene109 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		BF_GLOBALS._sound1.play(12);
		BF_GLOBALS._sceneObjects->draw();
		BF_GLOBALS._scenePalette.loadPalette(2);
		BF_GLOBALS._scenePalette.refresh();
		setDelay(10);
		break;
	case 2:
		scene->_text.setup(BF_19840515, this);
		break;
	case 3:
		BF_GLOBALS._v51C44 = 1;
		scene->loadScene(115);

		scene->_protaginist2.show();
		scene->_protaginist2.setPriority(133);
		scene->_protaginist1.show();
		scene->_bartender.show();
		scene->_object1.show();
		scene->_drunk.show();
		scene->_drunk.setAction(&scene->_action3);
		scene->_object2.show();
		scene->_object9.show();
		scene->_object9.setAction(&scene->_action2);

		BF_GLOBALS._v501FC = 170;
		setDelay(60);
		break;
	case 4:
		// Start drinking
		scene->_bartender.setAction(&scene->_sequenceManager4, NULL, 109, &scene->_bartender, &scene->_object2, NULL);
		scene->_protaginist1.setAction(&scene->_sequenceManager5, NULL, 107, &scene->_protaginist1, NULL);
		scene->_protaginist2.setAction(&scene->_sequenceManager6, this, 106, &scene->_protaginist2, NULL);
		break;
	case 5:
		// Open briefcase and pass over disk
		setAction(&scene->_sequenceManager6, this, 105, &scene->_object10, NULL);
		break;
	case 6:
		// Protaginist 2 walk to the bar
		scene->_object10.remove();
		setAction(&scene->_sequenceManager6, this, 100, &scene->_protaginist2, NULL);
		break;
	case 7:
		// Two thugs enter and walk to table
		scene->_object7.setAction(&scene->_sequenceManager7, NULL, 103, &scene->_object7, NULL);
		scene->_object5.setAction(&scene->_sequenceManager8, this, 102, &scene->_object5, NULL);
		scene->_protaginist2.setAction(&scene->_sequenceManager6, NULL, 104, &scene->_protaginist2, &scene->_bartender, NULL);
		break;
	case 8:
		// Protaginist 1 leaves, protaginist 2 stands up
		setAction(&scene->_sequenceManager8, this, 101, &scene->_object5, &scene->_protaginist1, NULL);
		break;
	case 9:
		// Shots fired!
		scene->_protaginist1.setAction(&scene->_sequenceManager5, this, 98, &scene->_protaginist1, NULL);
		scene->_object7.setAction(&scene->_sequenceManager7, NULL, 99, &scene->_object7, NULL);
		break;
	case 10:
		// End scene
		scene->_sceneMode = 1;
		remove();
		break;
	}
}

void Scene109::Action2::signal() {
	Scene109 *scene = (Scene109 *)BF_GLOBALS._sceneManager._scene;
	scene->setAction(&scene->_sequenceManager2, this, 3117, &scene->_object9, NULL);
}

void Scene109::Action3::signal() {
	Scene109 *scene = (Scene109 *)BF_GLOBALS._sceneManager._scene;
	scene->setAction(&scene->_sequenceManager3, this, 108, &scene->_drunk, NULL);
}

/*--------------------------------------------------------------------------*/

Scene109::Text::Text(): SceneText() {
	_action = NULL;
	_frameNumber = 0;
	_diff = 0;
}

void Scene109::Text::setup(const Common::String &msg, Action *action) {
	_frameNumber = BF_GLOBALS._events.getFrameNumber();
	_diff = 180;
	_action = action;
	_fontNumber = 4;
	_width = 300;
	_textMode = ALIGN_CENTER;
	_color1 = BF_GLOBALS._scenePalette._colors.background;
	_color2 = _color3 = 0;

	SceneText::setup(msg);

	// Center the text on-screen
	reposition();
	_bounds.center(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);

	// Set the new position
	_position.x = _bounds.left;
	_position.y = _bounds.top;
}

void Scene109::Text::synchronize(Serializer &s) {
	SceneText::synchronize(s);
	SYNC_POINTER(_action);
	s.syncAsUint32LE(_frameNumber);
	s.syncAsSint16LE(_diff);
}

void Scene109::Text::dispatch() {
	if (_diff) {
		uint32 frameNumber = BF_GLOBALS._events.getFrameNumber();
		if (_frameNumber < frameNumber) {
			_diff -= frameNumber - _frameNumber;
			_frameNumber = frameNumber;

			if (_diff <= 0) {
				// Time has expired, so remove the text and signal the designated action
				remove();
				if (_action)
					_action->signal();
			}
		}
	}
}

/*--------------------------------------------------------------------------*/

Scene109::Scene109(): GameScene() {
}

void Scene109::postInit(SceneObjectList *OwnerList) {
	GameScene::postInit(OwnerList);
	loadScene(999);

	_protaginist2.postInit();
	_protaginist2.setVisage(119);
	_protaginist2.setFrame(11);
	_protaginist2.fixPriority(133);
	_protaginist2.setPosition(Common::Point(165, 124));
	_protaginist2.hide();

	_protaginist1.postInit();
	_protaginist1.setVisage(118);
	_protaginist1.setStrip(1);
	_protaginist1.setFrame(8);
	_protaginist1.fixPriority(132);
	_protaginist1.setPosition(Common::Point(143, 125));
	_protaginist1.hide();

	_bartender.postInit();
	_bartender.setVisage(121);
	_bartender.setStrip(2);
	_bartender.setFrame(1);
	_bartender.setPriority(-1);
	_bartender.setPosition(Common::Point(92, 64));
	_bartender.hide();

	_object1.postInit();
	_object1.setVisage(121);
	_object1.setStrip(6);
	_object1.setFrame(1);
	_object1.setPriority(-1);
	_object1.setPosition(Common::Point(110, 64));
	_object1.hide();

	_drunk.postInit();
	_drunk.setVisage(120);
	_drunk.setStrip(2);
	_drunk.setFrame(5);
	_drunk.setPriority(-1);
	_drunk.setPosition(Common::Point(127, 97));
	_drunk.hide();

	_object2.postInit();
	_object2.setVisage(121);
	_object2.setStrip(5);
	_object2.setFrame(1);
	_object2.setPriority(-1);
	_object2.setPosition(Common::Point(104, 64));
	_object2.hide();

	_object9.postInit();
	_object9.setVisage(115);
	_object9.setStrip(4);
	_object9.setFrame(1);
	_object9.setPosition(Common::Point(262, 29));
	_object9.hide();

	_object5.postInit();
	_object5.hide();

	_object7.postInit();
	_object7.hide();

	_object10.postInit();
	_object10.hide();

	BF_GLOBALS._player.disableControl();
	setAction(&_action1, this);
}

void Scene109::signal() {
	if (_sceneMode == 1) {
		BF_GLOBALS._scenePalette.clearListeners();
		BF_GLOBALS._sceneManager.changeScene(110);
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
