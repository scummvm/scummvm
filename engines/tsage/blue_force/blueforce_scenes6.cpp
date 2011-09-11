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

#include "tsage/blue_force/blueforce_scenes6.h"
#include "tsage/blue_force/blueforce_dialogs.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 666 - Death Scene
 *
 *--------------------------------------------------------------------------*/

void Scene666::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		BF_GLOBALS._player.hide();
		setDelay(6);
		break;
	case 1:
		BF_GLOBALS._game->restartGame();
		break;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene666::Item1::startAction(CursorType action, Event &event) {
	return true;
}

/*--------------------------------------------------------------------------*/

void Scene666::postInit(SceneObjectList *OwnerList) {
	BF_GLOBALS._sound1.play(27);
	SceneExt::postInit();
	BF_GLOBALS._interfaceY = 200;
	loadScene(999);
	BF_GLOBALS._screenSurface.fillRect(BF_GLOBALS._screenSurface.getBounds(), 0);

	if (BF_GLOBALS._dayNumber == 0) {
		BF_GLOBALS._dayNumber = 1;
		BF_GLOBALS._deathReason = BF_GLOBALS._randomSource.getRandomNumber(23);
	}

	BF_GLOBALS._scenePalette.loadPalette(BF_GLOBALS._sceneManager._previousScene);
	BF_GLOBALS._uiElements._active = false;

	_item1.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 666, -1, -1, -1, 1, NULL);
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._events.setCursor(CURSOR_WALK);

	if (BF_GLOBALS._sceneManager._previousScene == 271) {
		setAction(&_action1);
	} else {
		switch (BF_GLOBALS._deathReason) {
		case 4:
		case 18:
		case 19:
		case 20:
			BF_GLOBALS._scenePalette.loadPalette(668);
			BF_GLOBALS._player.setVisage(668);
			BF_GLOBALS._player.setStrip2(1);
			BF_GLOBALS._player.setPosition(Common::Point(77, 155));
			BF_GLOBALS._player.animate(ANIM_MODE_5, this);
			break;
		case 5:
			BF_GLOBALS._scenePalette.loadPalette(900);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(666);
			BF_GLOBALS._player.setPosition(Common::Point(60, 160));
			signal();
			break;
		case 7:
		case 11:
		case 12:
		case 22:
			BF_GLOBALS._scenePalette.loadPalette(667);
			BF_GLOBALS._scenePalette.refresh();

			_object1.postInit();
			_object2.postInit();
			_object3.postInit();
			setAction(&_sequenceManager, this, 6660, &BF_GLOBALS._player, &_object1, &_object2,
				&_object3, NULL);
			break;
		case 13:
		case 14:
			BF_GLOBALS._scenePalette.loadPalette(665);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(665);
			BF_GLOBALS._player.setPosition(Common::Point(80, 140));
			signal();
			break;
		case 24:
			BF_GLOBALS._player.setVisage(664);
			BF_GLOBALS._player.setPosition(Common::Point(70, 160));
			signal();
			break;
		default:
			BF_GLOBALS._scenePalette.loadPalette(669);
			BF_GLOBALS._scenePalette.refresh();
			BF_GLOBALS._player.setVisage(669);
			BF_GLOBALS._player.setStrip(1);
			BF_GLOBALS._player.setPosition(Common::Point(27, 27));
			BF_GLOBALS._player.animate(ANIM_MODE_5, this);
			break;
		}
	}
}

void Scene666::remove() {
	BF_GLOBALS._sound1.fadeOut2(NULL);
	BF_GLOBALS._scrollFollower = &BF_GLOBALS._player;
	SceneExt::remove();
	BF_GLOBALS._uiElements._active = true;
}

void Scene666::signal() {
	BF_GLOBALS._player.enableControl();
	Rect textRect, sceneBounds;

	_text._color1 = 19;
	_text._color2 = 9;
	_text._color3 = 13;
	_text._fontNumber = 4;
	_text._width = 150;

	Common::String msg = _resourceManager->getMessage(666, BF_GLOBALS._deathReason);
	sceneBounds = BF_GLOBALS._sceneManager._scene->_sceneBounds;
	sceneBounds.collapse(4, 2);
	BF_GLOBALS.gfxManager()._font.getStringBounds(msg.c_str(), textRect, _text._width);
	textRect.moveTo(160, 10);
	textRect.contain(sceneBounds);

	_text.setup(msg);
	_text.setPosition(Common::Point(textRect.left, textRect.top));
	_text.setPriority(255);
	_text.show();
}

} // End of namespace BlueForce

} // End of namespace TsAGE
