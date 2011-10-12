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
		Common::String msg1 = g_resourceManager->getMessage(100, _state++);
		if (msg1.compareTo("LASTCREDIT")) {
			Common::String msg2 = g_resourceManager->getMessage(100, _state++);
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
	Scene100 *scene = (Scene100 *)g_globals->_sceneManager._scene;
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
			g_globals->_player.enableControl();
			g_globals->_events.setCursor(CURSOR_WALK);

			if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
				// Signal to start the game
				scene->_index = 190;
				remove();
				return;
			}
		}

		// At this point the introduction needs to start
		g_globals->_scenePalette.addFader(black, 1, 2, this);
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
	SceneExt::postInit();
	if (BF_GLOBALS._dayNumber < 6) {
		// Title
		loadScene(100);
	} else {
		// Credits
		loadScene(101);
	}
	BF_GLOBALS._scenePalette.loadPalette(2);
	BF_GLOBALS._v51C44 = 1;
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;

	g_globals->_player.postInit();
	g_globals->_player.hide();
	g_globals->_player.disableControl();
	_index = 109;

	if (BF_GLOBALS._dayNumber < 6) {
		// Title
		BF_GLOBALS._sound1.play(2);
		setAction(&_action2, this);
	} else {
		// Credits
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

Scene109::Scene109(): PalettedScene() {
}

void Scene109::postInit(SceneObjectList *OwnerList) {
	PalettedScene::postInit(OwnerList);
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

/*--------------------------------------------------------------------------
 * Scene 110 - Intro - Outside the bar
 *
 *--------------------------------------------------------------------------*/

void Scene110::Action1::signal() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(5);
		break;
	case 1:
		scene->_object8.show();
		scene->_object7.show();
		scene->_object1.show();
		scene->_object4.show();
		scene->_object5.show();
		scene->_object6.show();
		scene->_object9.show();
		scene->_object10.show();
		BF_GLOBALS._v51C44 = 1;
		scene->loadScene(110);
		setDelay(10);
		break;
	case 2:
		BF_GLOBALS._v51C44 = 1;
		scene->_object1.animate(ANIM_MODE_5, this);
		break;
	case 3: {
		Common::Point pt(123, 121);
		NpcMover *mover = new NpcMover();
		scene->_object4.addMover(mover, &pt, this);
		}
	// No break on purpose
	case 7:
		setDelay(30);
		break;
	case 4:
		// Guy running to the moto
		scene->_object1.animate(ANIM_MODE_6, NULL);
		break;
	case 5:
		scene->_object4.setStrip(2);
		scene->_object4.setPosition(Common::Point(117, 106));
		scene->_object4.animate(ANIM_MODE_5, this);
		break;
	case 6:
		// Play "Vroum"
		scene->_sound.play(31);
		// The guy starts the engine 
		scene->_object4.setStrip(3);
		scene->_object4._frame = 1;
		scene->_object4.animate(ANIM_MODE_5, NULL);
		// The guy looks behind him
		scene->_object2.setPosition(Common::Point(227, 190));
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 8:
		scene->_object2.animate(ANIM_MODE_6, this);
		break;
	case 9:
		scene->_object2.remove();
		scene->_object4.remove();
		scene->_object5.setStrip(4);
		scene->_object5.setFrame2(5);
		scene->_object5.setPosition(Common::Point(105, 120));
		setDelay(10);
		scene->_object6.setPosition(Common::Point(127, 96));
		scene->_object6.animate(ANIM_MODE_2, NULL);
		scene->_object6.setAction(&scene->_action2);
		scene->_sound.play(22);
		break;
	case 10:
		scene->_object8.setAction(&scene->_action4);
		scene->_object5.setFrame2(3);
		scene->_object5.setPosition(Common::Point(87, 120));
		setDelay(8);
		break;
	case 11:
		scene->_object5.setFrame2(4);
		scene->_object5.setPosition(Common::Point(62, 122));
		setDelay(6);
		break;
	case 12:
		scene->_object5.setFrame2(5);
		scene->_object5.setPosition(Common::Point(35, 120));
		scene->_object5.fixPriority(70);
		setDelay(4);
		break;
	case 13:
		scene->_object5.setFrame2(6);
		scene->_object5.setPosition(Common::Point(22, 111));
		setDelay(2);
		break;
	case 14:
		scene->_object5.setFrame2(7);
		scene->_object5.setPosition(Common::Point(19, 99));
		setDelay(2);
		break;
	case 15:
		scene->_object5.setFrame2(8);
		scene->_object5.setPosition(Common::Point(54, 76));
		setDelay(1);
		break;
	case 16: {
		Common::Point pt(182, 54);
		NpcMover *mover = new NpcMover();
		scene->_object5.addMover(mover, &pt, this);
		}
		break;
	case 17:
		scene->_object5.remove();
		break;
	default:
		break;
	}
}

void Scene110::Action2::signal() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		Common::Point pt(152, 94);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		}
		break;
	case 1: {
		Common::Point pt(167, 108);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		}
		break;
	case 2: {
		Common::Point pt(172, 109);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		}
		break;
	case 3: {
		Common::Point pt(179, 117);
		NpcMover *mover = new NpcMover();
		scene->_object6.addMover(mover, &pt, this);
		}
		break;
	case 4:
		setDelay(30);
		break;
	case 5:
		scene->_object6.animate(ANIM_MODE_NONE, NULL);
		break;
	default:
		break;
	}
}

void Scene110::Action3::signal() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(3);
		break;
	case 1:
		scene->_object7.setPosition(scene->_object7._position);
		scene->_object1.setFrame(2);
		setDelay(1);
		break;
	case 2:
		scene->_object1.setFrame(3);
		setDelay(1);
		break;
	case 3:
		scene->_object1.setFrame(4);
		scene->_object7.animate(ANIM_MODE_5, this);
		break;
	case 4: {
		scene->_object1.animate(ANIM_MODE_6, NULL);
		scene->_object7.setPosition(Common::Point(205, 99));
		scene->_object7.setStrip(2);
		scene->_object7.animate(ANIM_MODE_1, NULL);
		scene->_object7.changeZoom(90);
		Common::Point pt(151, 88);
		NpcMover *mover = new NpcMover();
		scene->_object7.addMover(mover, &pt, this);
		}
		break;
	case 5:
		scene->_object7.fixPriority(76);
		scene->_object7.setStrip(3);
		scene->_object7.changeZoom(90);
		scene->_object7.setFrame(1);
		scene->_object7.animate(ANIM_MODE_5, this);
		scene->_object10.animate(ANIM_MODE_5, NULL);
		break;
	case 6:
		scene->_object7.fixPriority(73);
		scene->_object10.animate(ANIM_MODE_6, this);
		break;
	case 7: {
		scene->_object7._yDiff = 1000;
		scene->_object10.remove();
		scene->_object9.setFrame2(2);
		Common::Point pt(230, 53);
		NpcMover *mover = new NpcMover();
		scene->_object9.addMover(mover, &pt, this);
		scene->_sound.play(23);
		}
		break;
	case 8:
		scene->_object9.remove();
		BF_GLOBALS._sceneManager.changeScene(125);
		break;
	default:
		break;
	}
}

void Scene110::Action3::dispatch() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	Action::dispatch();
	if ((_actionIndex == 5) && (scene->_object7._percent > 80))
		scene->_object7.changeZoom(scene->_object7._percent - 1);
}

void Scene110::Action4::signal() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object8.setPosition(scene->_object8._position);
		scene->_object1.setFrame(2);
		setDelay(1);
		break;
	case 1:
		scene->_object1.setFrame(3);
		setDelay(1);
		break;
	case 2: {
		scene->_object1.setFrame(4);
		scene->_object8.fixPriority(90);
		Common::Point pt(194, 119);
		NpcMover *mover = new NpcMover();
		scene->_object8.addMover(mover, &pt, this);
		}
		break;
	case 3:
		scene->_object1.animate(ANIM_MODE_6, NULL);
		scene->_object8.setPosition(Common::Point(186, 117));
		scene->_object8.setStrip(2);
		scene->_object8.setFrame(1);
		scene->_object3.setPosition(scene->_object3._position);
		scene->_object3.setFrame(1);
		setDelay(7);
		break;
	case 4:
		scene->_object8.setFrame2(2);
		scene->_object3.setFrame(3);
		setDelay(7);
		break;
	case 5:
		scene->_object8.setFrame2(3);
		scene->_object3.setFrame(3);
		setDelay(7);
		scene->_object6.remove();
		break;
	case 6:
		scene->_object8.setFrame2(4);
		scene->_object3.setFrame(4);
		setDelay(7);
		break;
	case 7:
		scene->_object8.setFrame2(5);
		scene->_object3.setFrame(5);
		setDelay(7);
		break;
	case 8:
		scene->_object8.setFrame2(7);
		scene->_object3.setFrame(6);
		setDelay(30);
		break;
	case 9: {
		scene->_object3.remove();
		scene->_object8.setFrame2(-1);
		scene->_object8.setPosition(Common::Point(176, 105));
		scene->_object8.animate(ANIM_MODE_1, NULL);
		scene->_object8.setStrip(3);
		Common::Point pt(141, 94);
		NpcMover *mover = new NpcMover();
		scene->_object8.addMover(mover, &pt, this);
		scene->_object7.setAction(&scene->_action3);
		}
		break;
	case 10: {
		scene->_object8.fixPriority(77);
		Common::Point pt(78, 76);
		NpcMover *mover = new NpcMover();
		scene->_object8.addMover(mover, &pt, this);
		}
		break;
	case 11: {
		scene->_object8.setPosition(Common::Point(87, 76));
		scene->_object8.fixPriority(71);
		scene->_object8.setStrip(6);
		scene->_object8.changeZoom(75);
		Common::Point pt(103, 74);
		NpcMover *mover = new NpcMover();
		scene->_object8.addMover(mover, &pt, this);
		}
		break;
	case 12:
		scene->_object8.setPosition(Common::Point(119, 59));
		scene->_object8.setStrip(5);
		scene->_object8.changeZoom(100);
		scene->_object8.animate(ANIM_MODE_5, this);
		break;
	case 13:
		scene->_object8.setPosition(Common::Point(119, 59), 300);
		break;
	default:
		break;
	}
}

void Scene110::Action4::dispatch() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	Action::dispatch();
	if ((_actionIndex > 9) && (_actionIndex < 12) && (scene->_object8._percent > 80))
		scene->_object8.changeZoom(scene->_object8._percent - 1);
}

void Scene110::Action5::signal() {
	Scene110 *scene = (Scene110 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.setPosition(scene->_object3._position);
		scene->_object3.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_object3.remove();
		break;
	default:
		break;
	}
}

void Scene110::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(999);
	_object8.postInit();
	_object8.setVisage(112);
	_object8.setPosition(Common::Point(239, 98), 300);
	_object8.setFrame(1);
	_object8.setStrip(1);
	_object8.changeZoom(90);
	_object8.fixPriority(90);
	_object8.animate(ANIM_MODE_1, NULL);
	_object8._moveDiff.x = 7;
	_object8._moveDiff.y = 3;
	_object8.hide();

	_object3.postInit();
	_object3.setVisage(112);
	_object3.setPosition(Common::Point(247, 190), 300);
	_object3.setFrame(1);
	_object3.setStrip(4);
	_object3.changeZoom(100);
	_object3.fixPriority(255);

	_object7.postInit();
	_object7.setVisage(113);
	_object7.setPosition(Common::Point(236, 86), 300);
	_object7._frame = 1;
	_object7.setStrip(1);
	_object7.changeZoom(110);
	_object7.fixPriority(89);
	_object7._moveDiff.x = 7;
	_object7._moveDiff.y = 3;
	_object7.hide();

	_object1.postInit();
	_object1.setVisage(110);
	_object1.setPosition(Common::Point(216, 88), 0);
	_object1._frame = 1;
	_object1.setStrip(1);
	_object1.changeZoom(100);
	_object1.fixPriority(83);
	_object1.hide();

	_object2.postInit();
	_object2.setVisage(111);
	_object2.setPosition(Common::Point(527, 190), 0);
	_object2._frame = 1;
	_object2.fixPriority(250);
	_object2.setStrip(5);
	_object2.changeZoom(100);

	_object4.postInit();
	_object4.setVisage(111);
	_object4.setPosition(Common::Point(253, 98), 0);
	_object4._frame = 1;
	_object4.setStrip(1);
	_object4.changeZoom(100);
	_object4.fixPriority(90);
	_object4.animate(ANIM_MODE_1, NULL);
	_object4._moveDiff.x = 10;
	_object4._moveDiff.y = 10;
	_object4.hide();

	_object5.postInit();
	_object5.setVisage(111);
	_object5.setPosition(Common::Point(114, 119), 0);
	_object5.setFrame2(1);
	_object5.setStrip(7);
	_object5.fixPriority(70);
	_object5.changeZoom(100);
	_object5._moveDiff.x = 20;
	_object5._moveDiff.y = 20;
	_object5.hide();

	_object6.postInit();
	_object6.setVisage(111);
	_object6.setPosition(Common::Point(527, 96), 0);
	_object6._frame = 1;
	_object6.setStrip(6);
	_object6.changeZoom(100);
	_object6.fixPriority(238);
	_object6._moveDiff.x = 10;
	_object6._moveDiff.y = 10;
	_object6.hide();

	_object9.postInit();
	_object9.setVisage(110);
	_object9.setPosition(Common::Point(148, 69), 0);
	_object9._frame = 1;
	_object9.setStrip(2);
	_object9.changeZoom(100);
	_object9.fixPriority(72);
	_object9._moveDiff.x = 10;
	_object9._moveDiff.y = 10;
	_object9.hide();

	_object10.postInit();
	_object10.setVisage(110);
	_object10._frame = 1;
	_object10.setStrip(3);
	_object10.changeZoom(100);
	_object10.fixPriority(74);
	_object10._moveDiff.x = 10;
	_object10._moveDiff.y = 10;
	_object10.setPosition(_object9._position);
	_object10.hide();
	
	setAction(&_action1);
}
/*--------------------------------------------------------------------------
 * Scene 114 - Outside the Bar
 *
 *--------------------------------------------------------------------------*/

bool Scene114::Object1::startAction(CursorType action, Event &event) {
	Scene114 *scene = (Scene114 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display(114, 0, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + BF_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 1142;
		if (BF_GLOBALS.getFlag(fWithLyle)) {
			scene->_object3.fixPriority(-1);
			scene->setAction(&scene->_sequenceManager1, this, 1142, &BF_GLOBALS._player, &scene->_object3, NULL);
		} else {
			Common::Point pt(155, 111);
			PlayerMover *mover = new PlayerMover();
			BF_GLOBALS._player.addMover(mover, &pt, scene);
		}
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

bool Scene114::Object2::startAction(CursorType action, Event &event) {
	Scene114 *scene = (Scene114 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display(114, 1, SET_WIDTH, 312,
				SET_X, GLOBALS._sceneManager._scene->_sceneBounds.left + 4,
				SET_Y, GLOBALS._sceneManager._scene->_sceneBounds.top + BF_INTERFACE_Y + 2,
				SET_FONT, 4, SET_BG_COLOR, 1, SET_FG_COLOR, 19, SET_EXT_BGCOLOR, 9,
				SET_EXT_FGCOLOR, 13, LIST_END);
		return true;
	case CURSOR_USE:
		BF_GLOBALS._walkRegions.proc2(2);
		BF_GLOBALS._player.disableControl();
		scene->_object3.fixPriority(-1);
		scene->_sceneMode = 1140;
		setAction(&scene->_sequenceManager1, this, 1140, &BF_GLOBALS._player, this, &scene->_object3, NULL);
		return true;
	default:
		return NamedObject::startAction(action, event);
	}
}

void Scene114::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();

	BF_GLOBALS._sound1.fadeSound(33);
	loadScene(110);

	setZoomPercents(85, 80, 105, 100);
	BF_GLOBALS._walkRegions.proc1(9);
	BF_GLOBALS._walkRegions.proc1(22);

	_object2.postInit();
	_object2.setVisage(110);
	_object2.setPosition(Common::Point(216, 88));
	BF_GLOBALS._sceneItems.push_front(&_object2);

	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(1358);
	BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
	BF_GLOBALS._player.setPosition(Common::Point(155, 111));
	BF_GLOBALS._player._moveDiff.x = 3;
	BF_GLOBALS._player._moveDiff.y = 2;
	BF_GLOBALS._player.changeZoom(-1);

	_object3.postInit();
	_object3.setVisage(1359);
	_object3.setObjectWrapper(new SceneObjectWrapper());
	_object3.animate(ANIM_MODE_1, NULL);
	_object3.setPosition(Common::Point(-118, -146));
	_object3.changeZoom(-1);
	_object3._moveDiff.x = 2;
	_object3._moveDiff.y = 1;
	_object3.hide();
	_object3.setDetails(114, 2, -1, -1, 1, NULL);

	_object1.postInit();
	if (BF_GLOBALS.getFlag(fWithLyle)) {
		_object1.setVisage(444);
		_object1.setPosition(Common::Point(127, 78));
		_object1.fixPriority(20);
		BF_GLOBALS._player.setPosition(Common::Point(139, 79));
		_object3.setPosition(Common::Point(86, 79));
		_object3.show();
	} else {
		_object1.setVisage(380);
		_object1.setPosition(Common::Point(128, 125));
		_object1.setFrame(1);
		if (BF_GLOBALS.getFlag(onDuty)) {
			_object1.setStrip(2);
			_object1.changeZoom(70);
			BF_GLOBALS._player.setVisage(352);
		} else {
			_object1.setStrip(1);
			_object1.changeZoom(77);
		}
		BF_GLOBALS._walkRegions.proc1(17);
	}
	BF_GLOBALS._sceneItems.push_front(&_object1);
	BF_GLOBALS._walkRegions.proc1(2);

	if (BF_GLOBALS._sceneManager._previousScene == 115) {
		BF_GLOBALS._player.setPosition(Common::Point(219, 100));
		BF_GLOBALS._player.setStrip(6);
		_object3.setPosition(Common::Point(253, 110));
		_object3.fixPriority(108);
		_object3.setStrip(6);
		BF_GLOBALS._player.enableControl();
	} else if (BF_GLOBALS.getFlag(fWithLyle)) {
		_sceneMode = 1141;
		setAction(&_sequenceManager1, this, 1141, &_object3, NULL);
	} else {
		BF_GLOBALS._player.enableControl();
	}

	_item1.setDetails(Rect(0, 0, 320, 200), 114, 3, -1, -1, 1, NULL);
}

void Scene114::signal() {
	switch (_sceneMode) {
	case 1140:
		BF_GLOBALS._sceneManager.changeScene(115);
		break;
	case 1141:
		BF_GLOBALS._player.enableControl();
		break;
	case 1142:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	default:
		break;
	}
}
/*--------------------------------------------------------------------------
 * Scene 180 - Front of Home
 *
 *--------------------------------------------------------------------------*/

bool Scene180::Vechile::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		BF_GLOBALS._sceneManager.changeScene(60);
		return true;
	} else {
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene180::GarageExit::startAction(CursorType action, Event &event) {
	Scene180 *scene = (Scene180 *)BF_GLOBALS._sceneManager._scene;

	if (scene->_garageExit.contains(event.mousePos)) {
		ADD_PLAYER_MOVER_NULL(BF_GLOBALS._player, 256, 114);
		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene180::Scene180(): SceneExt() {
	_fieldC56 = 0;
}

void Scene180::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_fieldC56);
}

void Scene180::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	if (((BF_GLOBALS._bookmark == bLyleStoppedBy) && (BF_GLOBALS._dayNumber == 1)) ||
			((BF_GLOBALS._bookmark == bDroppedOffLyle) && (BF_GLOBALS._dayNumber == 3)) ||
			((BF_GLOBALS._bookmark == bDoneAtLyles) && (BF_GLOBALS._dayNumber == 4)))
		loadScene(999);
	else
		loadScene(1180);
	setZoomPercents(121, 60, 125, 70);

	_gameTextSpeaker._textPos.y = 180;
	_stripManager.addSpeaker(&_gameTextSpeaker);
	
	if ((BF_GLOBALS._bookmark == bLyleStoppedBy) && (BF_GLOBALS._dayNumber == 1)) {
		BF_GLOBALS._v501FC = 87;
		BF_GLOBALS._v501FA = _sceneBounds.left;
//		_unk.setupText(THE_NEXT_DAY);
		_sceneMode = 6;
//		setAction(&_unk);
		BF_GLOBALS._driveFromScene = 4;
		BF_GLOBALS._driveToScene = 4;
		BF_GLOBALS._mapLocationId = 4;
	} else if (((BF_GLOBALS._bookmark == bDroppedOffLyle) && (BF_GLOBALS._dayNumber == 3)) ||
			((BF_GLOBALS._bookmark == bDoneAtLyles) && (BF_GLOBALS._dayNumber == 4))) {
		BF_GLOBALS._v501FC = 87;
		BF_GLOBALS._v501FA = _sceneBounds.left;
//		_unk.setupText(THE_NEXT_DAY);
		_sceneMode = 6;
//		setAction(&_unk);
	} else if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._player.setPosition(Common::Point(0, 150));
		_garageExit.postInit();

		BF_GLOBALS._driveToScene = 190;
		BF_GLOBALS._sound1.play(31);
		BF_GLOBALS._sound1.holdAt(1);
		_sceneMode = 1800;

		setAction(&_sequenceManager, this, 1800, &_vechile, &_object1, NULL);
	} else if (BF_GLOBALS._driveFromScene == 180) {
		BF_GLOBALS._player.postInit();
		BF_GLOBALS._player.setVisage(831);
		BF_GLOBALS._player.setPosition(Common::Point(285, 125));
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player._strip = 3;
		BF_GLOBALS._player.changeZoom(-1);

		_vechile.postInit();
		_vechile.setVisage(580);
		_vechile.setStrip(2);
		_vechile.setPosition(Common::Point(262, 131));
		_vechile.setZoom(65);
		_vechile.setDetails(180, 33, 34, 35, 1, NULL);

		_object1.postInit();
		_object1.setVisage(182);
		_object1.setStrip(2);
		_object1.setPosition(Common::Point(258, 122));
		_object1.setFrame(6);

		BF_GLOBALS._driveFromScene = 0;
		BF_GLOBALS._player.enableControl();
	} else if (BF_GLOBALS._driveToScene != 180) {
		BF_GLOBALS._player.postInit();
		BF_GLOBALS._player.disableControl();

		_vechile.postInit();
		_vechile.setDetails(180, 33, 34, 35, 1, NULL);

		_object1.postInit();

		BF_GLOBALS._driveFromScene = 4;
		BF_GLOBALS._driveToScene = 4;
		BF_GLOBALS._mapLocationId = 4;

		BF_GLOBALS._sound1.fadeSound(33);
		_sceneMode = 1801;
		setAction(&_sequenceManager, this, 1801, &BF_GLOBALS._player, &_vechile, &_object1, NULL);
	} else if (!BF_GLOBALS.getFlag(onDuty) && !BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._player.setPosition(Common::Point(0, 150));

		_object1.postInit();
		_object1.setVisage(182);
		_object1.setStrip(2);
		_object1.setPosition(Common::Point(258, 122));

		_vechile.postInit();
		_vechile.setVisage(181);
		_vechile.setStrip(2);
		_vechile.changeZoom(80);
		_vechile.fixPriority(150);
		_vechile._moveDiff = Common::Point(40, 5);
		_vechile.setPosition(Common::Point(-25, 171));

		_fieldC56 = 1;
		BF_GLOBALS._sound1.play(29);
		_sceneMode = 1;
		ADD_MOVER(_vechile, 259, 150);
	} else {
		BF_GLOBALS._player.setPosition(Common::Point(0, 150));

		_object1.postInit();
		_object1.setVisage(182);
		_object1.setStrip(2);
		_object1.setPosition(Common::Point(258, 122));

		_vechile.postInit();
		if (BF_GLOBALS.getFlag(onDuty)) {
			BF_GLOBALS._sound1.play(29);

			_vechile.setVisage(191);
			_vechile.setStrip(3);
			_vechile._frame = 5;
			_vechile.changeZoom(75);

			_fieldC56 = 1;
			_vechile._moveDiff.x = 45;
		} else {
			_vechile.setVisage(444);
			_vechile.setStrip(2);
			_vechile.changeZoom(85);

			_fieldC56 = 3;
			_vechile._moveDiff.x = 30;
		}

		_vechile.fixPriority(150);
		_vechile._moveDiff.y = 5;
		_vechile.setPosition(Common::Point(-25, 171));

		_sceneMode = 3;
		ADD_MOVER(_vechile, 258, 145);
	}

	if (_sceneMode != 6) {
		_frontDoor.setDetails(Rect(183, 92, 218, 122), 180, 27, 28, 29, 1, NULL);
		_driveway.setDetails(8, 180, 36, 37, 38, 1);
		_street.setDetails(1, 180, 21, 22, 23, 1);
		_lawn.setDetails(3, 180, 18, 19, 20, 1);
		_bushes.setDetails(4, 180, 15, 16, 17, 1);
		_palms.setDetails(6, 180, 12, 13, 14, 1);
		_garage.setDetails(Rect(241, 85, 319, 121), 180, 30, 31, 32, 1, NULL);
		_fence.setDetails(Rect(0, 109, 21, 125), 180, 9, 10, 11, 1, NULL);
		_house.setDetails(5, 180, 24, 25, 26, 1);
		_steps.setDetails(7, 180, 6, 7, 8, 1);
		_curb.setDetails(2, 180, 3, 4, 5, 1);
		_sky.setDetails(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y), 180, 0, 1, 2, 1, NULL);
	}
}

void Scene180::signal() {
	switch (_sceneMode) {
	case 1:
		_fieldC56 = 0;
		switch (BF_GLOBALS._bookmark) {
		case bFlashBackThree:
			BF_GLOBALS._bookmark = bDroppedOffLyle;
			_sceneMode = 7;
			break;
		case bDoneWithIsland:
			BF_GLOBALS._bookmark = bDoneAtLyles;
			_sceneMode = 8;
			break;
		default:
			_sceneMode = 1802;
			break;
		}

		setAction(&_sequenceManager, this, 1802, &_vechile, &_object1, NULL);
		break;
	case 2:
		_fieldC56 = 0;
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(BF_GLOBALS._driveToScene);
		break;
	case 3:
		_fieldC56 = 0;
		BF_GLOBALS._sound1.stop();
		_stripManager.start(1800, this);
		_sceneMode = 4;
		break;
	case 4:
		_sceneMode = 5;
		BF_GLOBALS._sound1.fadeSound(29);
		ADD_MOVER(_vechile, 340, 140);
		_vechile._moveDiff.y = 1;
		break;
	case 5:
		BF_GLOBALS._sceneManager.changeScene(50);
		break;
	case 6:
		loadScene(1180);
		BF_GLOBALS._sound1.fadeSound(33);

		switch (BF_GLOBALS._bookmark) {
		case bLyleStoppedBy:
			BF_GLOBALS._dayNumber = 2;
			BF_INVENTORY.alterInventory(2);
			break;
		case bDroppedOffLyle:
			BF_GLOBALS._dayNumber = 4;
			BF_INVENTORY.alterInventory(4);
			break;
		case bDoneAtLyles:
			BF_GLOBALS._dayNumber = 5;
			BF_INVENTORY.alterInventory(5);
			break;
		default:
			break;
		}

		BF_GLOBALS._player.postInit();
		BF_GLOBALS._player.disableControl();

		_vechile.postInit();
		_vechile.setDetails(180, 33, 34, 35, 1, NULL);

		_object1.postInit();
		_sceneMode = 1801;
		setAction(&_sequenceManager, this, 1801, &BF_GLOBALS._player, &_vechile, &_object1, NULL);

		_frontDoor.setDetails(Rect(183, 92, 218, 122), 180, 27, 28, 29, 1, NULL);
		_driveway.setDetails(8, 180, 36, 37, 38, 1);
		_street.setDetails(1, 180, 21, 22, 23, 1);
		_lawn.setDetails(3, 180, 18, 19, 20, 1);
		_bushes.setDetails(4, 180, 15, 16, 17, 1);
		_palms.setDetails(6, 180, 12, 13, 14, 1);
		_garage.setDetails(Rect(241, 85, 319, 121), 180, 30, 31, 32, 1, NULL);
		_fence.setDetails(Rect(0, 109, 21, 125), 180, 9, 10, 11, 1, NULL);
		_house.setDetails(4, 180, 24, 25, 26, 1);
		_steps.setDetails(7, 180, 6, 7, 8, 1);
		_curb.setDetails(2, 180, 3, 4, 5, 1);
		_sky.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 180, 0, 1, 2, 1, NULL);
		break;
	case 7:
		BF_INVENTORY.setObjectScene(INV_COBB_RAP, 0);
		BF_INVENTORY.setObjectScene(INV_MUG_SHOT, 0);
		BF_INVENTORY.setObjectScene(INV_FOREST_RAP, 0);
		BF_INVENTORY.setObjectScene(INV_LYLE_CARD, 0);
		BF_INVENTORY.setObjectScene(INV_NAPKIN, 0);
		BF_INVENTORY.setObjectScene(INV_9MM_BULLETS, 0);
		BF_INVENTORY.setObjectScene(INV_SCHEDULE, 0);
		BF_INVENTORY.setObjectScene(INV_PRINT_OUT, 0);
		break;
	case 8:
		if (BF_GLOBALS.getFlag(fLeftTraceIn900) || BF_GLOBALS.getFlag(fGotPointsForSearchingDA) ||
				BF_GLOBALS.getFlag(fLeftTraceIn920)) {
			BF_GLOBALS._sound1.fadeOut2(NULL);
			BF_GLOBALS._driveToScene = 0;
			BF_GLOBALS._driveFromScene = 0;
			BF_GLOBALS._sceneManager.changeScene(271);
		} else {
			BF_GLOBALS._sceneManager.changeScene(180);
		}
		break;
	case 1800:
		_fieldC56 = 2;
		_vechile._moveDiff.x = 10;
		ADD_MOVER(_vechile, -25, 171);
		break;
	case 1801:
		BF_GLOBALS._player._strip = 3;
		BF_GLOBALS._player.enableControl();
		break;
	case 1802:
		BF_GLOBALS._sound1.release();
		BF_GLOBALS._driveToScene = 0;
		BF_GLOBALS._driveFromScene = 0;
		BF_GLOBALS._sceneManager.changeScene(270);
		break;
	default:
		break;
	}
}

void Scene180::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		if (_vechile.contains(event.mousePos)) {
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		} else if (_garageExit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(EXITFRAME_NW);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene180::dispatch() {
	switch (_fieldC56) {
	case 1:
		if (_vechile._mover && (_vechile._percent > 50))
			_vechile.changeZoom(_vechile._percent);
		if (_vechile._moveDiff.x > 15)
			--_vechile._moveDiff.x;
		break;
	case 2:
		if (_vechile._mover && (_vechile._percent < 100))
			_vechile.changeZoom(_vechile._percent);
		if (_vechile._moveDiff.x < 35)
			++_vechile._moveDiff.x;
		break;
	case 3:
		if (_vechile._mover && (_vechile._percent > 70))
			_vechile.changeZoom(_vechile._percent);
		if (_vechile._moveDiff.x > 15)
			--_vechile._moveDiff.x;
		break;
	default:
		break;
	}

	SceneExt::dispatch();

	if (!_action && (BF_GLOBALS._player._position.y < 120)) {
		BF_GLOBALS._player.disableControl();
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._driveToScene = 0;
		BF_GLOBALS._driveFromScene = 0;

		if (BF_GLOBALS.getFlag(fLeftTraceIn900) || BF_GLOBALS.getFlag(fGotPointsForSearchingDA) ||
				BF_GLOBALS.getFlag(fLeftTraceIn920))
			BF_GLOBALS._sceneManager.changeScene(271);
		else
			BF_GLOBALS._sceneManager.changeScene(270);
	}
}

/*--------------------------------------------------------------------------
 * Scene 190 - Front of Police Station
 *
 *--------------------------------------------------------------------------*/

bool Scene190::Object4::startAction(CursorType action, Event &event) {
	Scene190 *scene = (Scene190 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE: {
		BF_GLOBALS._player.disableControl();
		scene->_sceneMode = 13;
		Common::Point pt(62, 96);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, scene);
		return true;
	}
	default:
		return NamedObject::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene190::Item1::startAction(CursorType action, Event &event) {
	Scene190 *scene = (Scene190 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		scene->setAction(&scene->_action1);
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene190::Item2::startAction(CursorType action, Event &event) {
	Scene190 *scene = (Scene190 *)BF_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		scene->_stripManager.start(1900, scene);
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

bool Scene190::Exit::startAction(CursorType action, Event &event) {
	Scene190 *scene = (Scene190 *)BF_GLOBALS._sceneManager._scene;

	Common::Point pt(316, 91);
	PlayerMover *mover = new PlayerMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene190::Action1::signal() {
	Scene190 *scene = (Scene190 *)BF_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.disableControl();
		setDelay(2);
		break;
	case 1: {
		ADD_MOVER(BF_GLOBALS._player, 165, 91);
		break;
	}
	case 2:
		scene->_sound.play(82);
		scene->_object2.animate(ANIM_MODE_5, this);
		break;
	case 3:
		ADD_MOVER(BF_GLOBALS._player, 180, 86);
		break;
	case 4:
		scene->_sound.play(82);
		scene->_object2.animate(ANIM_MODE_6, this);
		break;
	case 5:
		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(315);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene190::Scene190(): SceneExt() {
	_fieldB52 = true;
	_cursorVisage.setVisage(1, 8);
}

void Scene190::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._dialogCenter.y = 100;
	if ((BF_GLOBALS._sceneManager._previousScene == 100) ||
			(BF_GLOBALS._sceneManager._previousScene == 20)) {
//		clearScreen();
	}
	if (BF_GLOBALS._dayNumber == 0)
		// If at start of game, change to first day
		BF_GLOBALS._dayNumber = 1;

	// Load the scene data
	loadScene(190);
	BF_GLOBALS._scenePalette.loadPalette(2);

	_stripManager.addSpeaker(&_speaker);
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.disableControl();

	// Initialise objects
	_object2.postInit();
	_object2.setVisage(190);
	_object2.setStrip(1);
	_object2.setPosition(Common::Point(179, 88));

	_object3.postInit();
	_object3.setVisage(190);
	_object3.setStrip(2);
	_object3.fixPriority(200);
	_object3.setPosition(Common::Point(170, 31));
	_object3.animate(ANIM_MODE_7, 0, NULL);
	_object3.setDetails(190, 8, 26, 19, 1, NULL);

	if (BF_GLOBALS.getFlag(fWithLyle)) {
		BF_GLOBALS._player.setVisage(303);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		BF_GLOBALS._player._moveDiff = Common::Point(3, 1);

		_object4.postInit();
		_object4.setVisage(444);
		_object4.setFrame(2);
		_object4.setPosition(Common::Point(54, 114));
		_object4.setDetails(190, -1, -1, -1, 1, NULL);

		switch (BF_GLOBALS._sceneManager._previousScene) {
		case 300: {
			_sceneMode = 12;
			BF_GLOBALS._player.setPosition(Common::Point(316, 91));
			ADD_MOVER(BF_GLOBALS._player, 305, 91);
			break;
		}
		case 315:
			_sceneMode = 1901;
			setAction(&_sequenceManager, this, 1901, &BF_GLOBALS._player, &_object2, NULL);
			break;
		case 50:
		case 60:
		default:
			_fieldB52 = false;
			BF_GLOBALS._player.setPosition(Common::Point(62, 96));
			BF_GLOBALS._player._strip = 3;
			BF_GLOBALS._player.enableControl();
			break;
		}
	} else {
		BF_GLOBALS._player.setVisage(BF_GLOBALS._player._visage);
		BF_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		BF_GLOBALS._player.animate(ANIM_MODE_1, NULL);

		switch (BF_GLOBALS._sceneManager._previousScene) {
		case 300: {
			if (!BF_GLOBALS.getFlag(onBike)) {
				BF_GLOBALS._player._moveDiff = Common::Point(3, 1);
				_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 11 : 12;
				BF_GLOBALS._player.setVisage(BF_GLOBALS.getFlag(onDuty) ? 1304 : 303);
				BF_GLOBALS._player.setPosition(Common::Point(316, 91));
				ADD_MOVER(BF_GLOBALS._player, 305, 91);
			} else {
				BF_GLOBALS._player.disableControl();
				_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 193 : 191;
				setAction(&_sequenceManager, this, 193, &BF_GLOBALS._player, NULL);
			}
			break;
		}
		case 315:
			BF_GLOBALS._player._moveDiff = Common::Point(3, 1);
			_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 1900 : 1901;
			setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, &_object2, NULL);
			break;
		case 50:
		case 60:
		default:
			BF_GLOBALS.setFlag(onBike);
			BF_GLOBALS._player.disableControl();
			BF_GLOBALS._uiElements._active = true;
			_sceneMode = BF_GLOBALS.getFlag(onDuty) ? 192 : 190;
			setAction(&_sequenceManager, this, _sceneMode, &BF_GLOBALS._player, NULL);
			break;
		}
	}

	if (BF_GLOBALS.getFlag(onBike)) {
		BF_GLOBALS._sound1.play(BF_GLOBALS.getFlag(onDuty) ? 37 : 29);
	} else if (BF_GLOBALS._sceneManager._previousScene != 300) {
		BF_GLOBALS._sound1.play(33);
	}

	_exit.setDetails(Rect(310, 50, 320, 125), 190, -1, -1, -1, 1, NULL);
	_item2.setDetails(Rect(108, 1, 111, 94), 190, 7, 11, 18, 1, NULL);
	_item4.setDetails(2, 190, 5, 10, 16, 1);
	_item3.setDetails(1, 190, 4, 10, 15, 1);
	_item8.setDetails(6, 190, 20, 21, 22, 1);
	_item1.setDetails(7, 190, 1, 10, -1, 1);
	_item7.setDetails(5, 190, 0, 10, 12, 1);
	_item6.setDetails(4, 190, 2, 10, 13, 1);
	_item5.setDetails(3, 190, 3, 10, 14, 1);
	_item9.setDetails(Rect(0, 0, 89, 68), 190, 6, 10, 17, 1, NULL);
	_item10.setDetails(Rect(0, 0, SCREEN_WIDTH, BF_INTERFACE_Y), 190, 23, -1, -1, 1, NULL);
}

void Scene190::signal() {
	switch (_sceneMode) {
	case 10:
		if ((BF_GLOBALS._dayNumber == 2) && (BF_GLOBALS._bookmark < bEndDayOne))
			BF_GLOBALS._sound1.changeSound(49);
		BF_GLOBALS._sceneManager.changeScene(300);
		break;
	case 11:
	case 12:
	case 1900:
	case 1901:
		BF_GLOBALS._player.enableControl();
		_fieldB52 = false;
		break;
	case 13:
	case 191:
	case 193:
		BF_GLOBALS._sceneManager.changeScene(60);
		break;
	case 190:
	case 192:
		BF_GLOBALS._sceneManager.changeScene(300);
		break;
	case 0:
	default:
		BF_GLOBALS._player.enableControl();
		break;
	}
}

void Scene190::process(Event &event) {
	SceneExt::process(event);

	if (BF_GLOBALS._player._enabled && !_focusObject && (event.mousePos.y < (BF_INTERFACE_Y - 1))) {
		// Check if the cursor is on an exit
		if (_exit.contains(event.mousePos)) {
			GfxSurface surface = _cursorVisage.getFrame(3);
			BF_GLOBALS._events.setCursor(surface);
		} else {
			// In case an exit cursor was being shown, restore the previously selected cursor
			CursorType cursorId = BF_GLOBALS._events.getCursor();
			BF_GLOBALS._events.setCursor(cursorId);
		}
	}
}

void Scene190::dispatch() {
	SceneExt::dispatch();

	if (!_action && !_fieldB52 && (BF_GLOBALS._player._position.x >= 310)
			&& !BF_GLOBALS.getFlag(onBike)) {
		// Handle walking off to the right side of the screen
		BF_GLOBALS._player.disableControl();
		_fieldB52 = true;
		_sceneMode = 10;

		ADD_MOVER(BF_GLOBALS._player, 330, BF_GLOBALS._player._position.y);
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
