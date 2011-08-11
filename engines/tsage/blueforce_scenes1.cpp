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
#include "tsage/blueforce_scenes1.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/globals.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 100 - Tsunami Title Screen #2
 *
 *--------------------------------------------------------------------------*/

void BF_Scene100::Action1::signal() {
	static byte black[3] = { 0, 0, 0 };

	switch (_actionIndex++) {
	case 0:
		_state = 0;
		setDelay(6);
		break;
	case 1: {
		Common::String msg1 = _resourceManager->getMessage(100, _state++);
		if (!msg1.compareTo("LASTCREDIT")) {
			Common::String msg2 = _resourceManager->getMessage(100, _state++);
			setTextStrings(msg1, msg2, this);
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

void BF_Scene100::Action1::setTextStrings(const Common::String &msg1, const Common::String &msg2, Action *action) {
	// Set data for first text control
	_sceneText1._fontNumber = 10;
	_sceneText1._width = 160;
	_sceneText1._textMode = ALIGN_RIGHT;
	_sceneText1._color1 = _globals->_fontColors.background;
	_sceneText1._color2 = _globals->_fontColors.foreground;
	_sceneText1._color3 = _globals->_fontColors.background;
	_sceneText1.setup(msg1);
	_sceneText1.setFrame2(-1);
	_sceneText1.setPosition(Common::Point(
		(SCREEN_WIDTH - _sceneText1.getFrame().getBounds().width()) / 2, 202));
	_sceneText1._moveRate = 30;
	_sceneText1._moveDiff.y = 1;

	// Set data for second text control
	_sceneText2._fontNumber = 10;
	_sceneText2._width = _sceneText1._width;
	_sceneText2._textMode = _sceneText1._textMode;
	_sceneText2._color1 = _globals->_fontColors.background;
	_sceneText2._color2 = _globals->_fontColors.foreground;
	_sceneText2._color3 = _globals->_fontColors.background;
	_sceneText2.setup(msg1);
	_sceneText2.setFrame2(-1);
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

void BF_Scene100::Action2::signal() {
	BF_Scene100 *scene = (BF_Scene100 *)_globals->_sceneManager._scene;
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

BF_Scene100::BF_Scene100(): Scene() {
	_index = 0;
}

void BF_Scene100::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._scenePalette.loadPalette(2);
	BF_GLOBALS._v51C44 = 1;
	Scene::postInit();
	BF_GLOBALS._v51C24 = 200;

	_globals->_player.enableControl();
	_globals->_player.hide();
	_globals->_player.disableControl();
	_index = 109;

	if (BF_GLOBALS._v4CEA2 < 6) {
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

	loadScene(20);
	setZoomPercents(60, 85, 200, 100);
}

void BF_Scene100::signal() {
	++_sceneMode;
	if (BF_GLOBALS._v4CEA2 < 6) {
		BF_GLOBALS._scenePalette.clearListeners();
		BF_GLOBALS._scenePalette.loadPalette(100);
		BF_GLOBALS._sceneManager.changeScene(_index);
	} else {
		if (_sceneMode > 1)
			BF_GLOBALS._events.setCursor(CURSOR_ARROW);

		setAction(this, &_action1, this);
	}
}

} // End of namespace tSage
