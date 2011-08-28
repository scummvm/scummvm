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

#include "tsage/blue_force/blueforce_scenes0.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

/*--------------------------------------------------------------------------
 * Scene 20 - Tsunami Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene20::Action1::signal() {
	Scene20 *scene = (Scene20 *)BF_GLOBALS._sceneManager._scene;
	static byte black[3] = { 0, 0, 0 };

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		_sound.play(1);
		BF_GLOBALS._scenePalette.addRotation(64, 127, -1, 1, this);
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
		BF_GLOBALS._scenePalette.addFader(scene->_scenePalette._palette, 256, 8, this);
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
		BF_GLOBALS._scenePalette.addFader(black, 1, 5, this);
		break;
	case 8:
		BF_GLOBALS._sceneManager.changeScene(100);
		remove();
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene20::postInit(SceneObjectList *OwnerList) {
	loadScene(20);
	Scene::postInit();
	setZoomPercents(60, 85, 200, 100);

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

/*--------------------------------------------------------------------------
 * Scene 50 - Map Screen
 *
 *--------------------------------------------------------------------------*/

Scene50::Tooltip::Tooltip(): SavedObject() {
	_newSceneNumber = _locationId = 0;
}

void Scene50::Tooltip::synchronize(Serializer &s) {
	SavedObject::synchronize(s);
	_bounds.synchronize(s);
	s.syncString(_msg);
}

void Scene50::Tooltip2::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1: {
		Common::Point pt(410, 181);
		NpcMover *mover = new NpcMover();
		((SceneObject *)_owner)->addMover(mover, &pt, this);
		break;
	}
	case 2:
		_owner->remove();
		break;
	default:
		break;
	}
}

void Scene50::Tooltip2::dispatch() {
	Action::dispatch();
	SceneObject *owner = (SceneObject *)_owner;

	if ((_actionIndex == 2) && (owner->_percent < 100)) {
		owner->changeZoom(owner->_percent + 1);
	}
}

void Scene50::Tooltip::set(const Rect &bounds, int v60, const Common::String &msg, int v62) {
	_bounds = bounds;
	_newSceneNumber = v60;
	_msg = msg;
	_locationId = v62;
}

void Scene50::Tooltip::update() {
	// Set up the text object for the scene with the appropriate settings
	Scene50 *scene = (Scene50 *)BF_GLOBALS._sceneManager._scene;
	scene->_text._fontNumber = 10;
	scene->_text._color1 = BF_GLOBALS._scenePalette._colors.foreground;
	scene->_text._color2 = BF_GLOBALS._scenePalette._colors.background;
	scene->_text.setup(_msg);

	int yp = (scene->_text._textSurface.getBounds().height() == 0) ? _bounds.bottom : _bounds.top;
	scene->_text.setPosition(Common::Point(_bounds.left, yp));
}

void Scene50::Tooltip::highlight(bool btnDown) {
	Scene50 *scene = (Scene50 *)BF_GLOBALS._sceneManager._scene;

	scene->_field382 = _newSceneNumber;
	if ((scene->_field380 != 0) || (scene->_field380 != _newSceneNumber))
		update();

	if (btnDown) {
		if ((BF_GLOBALS._bookmark == bCalledToDrunkStop) && BF_GLOBALS.getFlag(beenToJRDay2))
			scene->_sceneNumber = 600;
		else if (BF_GLOBALS._bookmark == bBookedGreen)
			scene->_sceneNumber = 410;
		else {
			BF_GLOBALS._driveToScene = _newSceneNumber;

			switch (BF_GLOBALS._driveFromScene) {
			case 330:
			case 340:
			case 342:
				BF_GLOBALS._player.disableControl();
				BF_GLOBALS._mapLocationId = _locationId;

				if (BF_GLOBALS._driveToScene == 330) {
					scene->_sceneNumber = 330;
				} else {
					scene->_sceneNumber = (BF_GLOBALS._dayNumber != 1) || (BF_GLOBALS._bookmark < bStartOfGame) ||
						(BF_GLOBALS._bookmark >= bCalledToDomesticViolence) ? 342 : 340;
				}
				break;

			case 410:
			case 551:
				if (BF_GLOBALS.getFlag((BF_GLOBALS._driveFromScene == 410) ? fSearchedTruck : didDrunk)) {
					BF_GLOBALS._mapLocationId = _locationId;
					BF_GLOBALS._player.disableControl();
					scene->_sceneNumber = _newSceneNumber;
				} else {
					BF_GLOBALS._v4CEA8 = 4;
					BF_GLOBALS._sceneManager.changeScene(666);
					return;
				}
				break;

			case 300:
				if (_locationId == 1) {
					BF_GLOBALS._driveToScene = 300;
					_newSceneNumber = 300;
				}
				// Deliberate fall through to default
			default:
				BF_GLOBALS._mapLocationId = _locationId;
				BF_GLOBALS._player.disableControl();
				scene->_sceneNumber = _newSceneNumber;
				break;
			}

			// Signal the scene to change to the new scene
			scene->_sceneMode = 1;
			scene->signal();
		}
	}
}

/*--------------------------------------------------------------------------*/

Scene50::Scene50(): SceneExt() {
	_field382 = 0;
	_field380 = 0;
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();

	BF_GLOBALS._interfaceY = 200;
	BF_GLOBALS._player.postInit();
	BF_GLOBALS._player.setVisage(830);
	BF_GLOBALS._player.setStrip(3);
	BF_GLOBALS._player.setPosition(Common::Point(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2));
	BF_GLOBALS._player.hide();
	BF_GLOBALS._player.enableControl();
	BF_GLOBALS._player._uiEnabled = false;

	BF_GLOBALS._scrollFollower = NULL;
	_text._color1 = 19;
	_text._color2 = 9;
	_text._color3 = 9;
	_text._width = 75;
	_text._fontNumber = 4;
	_text._textMode = ALIGN_LEFT;
	_text.fixPriority(256);

	// Set all the hotspots
	_location3.set(Rect(10, 92, 24, 105), 180, GRANDMA_FRANNIE, 4);
	_location2.set(Rect(443, 149, 508, 178), 330, MARINA, 2);
	_location1.set(Rect(573, 103, 604, 130), 190, POLICE_DEPARTMENT, 1);
	_location4.set(Rect(313, 21, 325, 33), 114, TONYS_BAR, 8);
	_location8.set(Rect(69, 79, 82, 88), 580, CHILD_PROTECTIVE_SERVICES, 256);
	_location6.set(Rect(242, 131, 264, 144), 440, ALLEY_CAT, 64);
	_location5.set(Rect(383, 57, 402, 70), 380, CITY_HALL_JAIL, 32);
	_location7.set(Rect(128, 32, 143, 42), 800, JAMISON_RYAN, 128);
	_location9.set(Rect(349, 125, 359, 132), 
		(BF_GLOBALS._bookmark == bInspectionDone) || (BF_GLOBALS._bookmark == bCalledToDrunkStop) ? 551 : 550,
		BIKINI_HUT, 16);

	_item.setBounds(Rect(0, 0, SCREEN_WIDTH * 2, SCREEN_HEIGHT));
	BF_GLOBALS._sceneItems.push_back(&_item);

	// Find the location to start at
	int selectedTooltip = BF_GLOBALS._mapLocationId;
	Tooltip *tooltip = NULL;
	int xp = 0;

	switch (selectedTooltip) {
	case 1:
		tooltip = &_location1;
		xp = 588;
		break;
	case 2:
		tooltip = &_location2;
		xp = 475;
		break;
	case 4:
		tooltip = &_location3;
		xp = 17;
		break;
	case 8:
		tooltip = &_location4;
		xp = 319;
		break;
	case 32:
		tooltip = &_location5;
		xp = 392;
		break;
	case 64:
		tooltip = &_location6;
		xp = 253;
		break;
	case 128:
		tooltip = &_location7;
		xp = 135;
		break;
	case 16:
		tooltip = &_location9;
		xp = 354;
		break;
	case 256:
		tooltip = &_location8;
		xp = 75;
		break;
	}

	_timer.set(240, this);
	_sceneBounds.center(xp, SCREEN_HEIGHT / 2);
	loadScene(55);
	_sceneMode = 2;

	loadBackground(xp - 160, 0);
	tooltip->update();
}

void Scene50::remove() {
	// Blank out the screen
	BF_GLOBALS._screenSurface.fillRect(BF_GLOBALS._screenSurface.getBounds(), 0);

	SceneExt::remove();
	BF_GLOBALS._v4E238 = 1;
}

void Scene50::signal() {
	if (_sceneMode == 1) {
		// Destination selected
		if ((BF_GLOBALS._driveFromScene == 551) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.setFlag(109);
			BF_GLOBALS.setFlag(115);
			BF_GLOBALS.setFlag(121);
			BF_GLOBALS.setFlag(127);
			BF_GLOBALS.setFlag(133);
		}

		if ((BF_GLOBALS._driveFromScene == 410) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.setFlag(125);
		}

		if ((BF_GLOBALS._driveFromScene == 340) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			BF_GLOBALS.setFlag(123);
		}

		if ((BF_GLOBALS._driveFromScene == 380) && (_sceneNumber != BF_GLOBALS._driveFromScene)) {
			if (BF_GLOBALS._bookmark >= bLauraToParamedics)
				BF_GLOBALS.setFlag(129);
			if (BF_GLOBALS._bookmark >= bStoppedFrankie)
				BF_GLOBALS.setFlag(131);
			if (BF_GLOBALS._bookmark == bArrestedGreen) {
				BF_GLOBALS._v4CEA8 = 19;
				_sceneNumber = 666;
			}
		}

		if ((_sceneNumber == 551) && BF_GLOBALS.getFlag(147))
			_sceneNumber = 550;

		BF_GLOBALS._sound1.fadeOut2(NULL);
		BF_GLOBALS._sceneManager.changeScene(_sceneNumber);

	} else if (_sceneMode == 2) {
		// Initial delay complete, time to switch to interactive mode
		_text.remove();
		BF_GLOBALS._player.enableControl();
		_sceneMode = 0;
		_field380 = 0;
	}
}

void Scene50::process(Event &event) {
	SceneExt::process(event);
	Common::Point pt(event.mousePos.x + _sceneBounds.left, event.mousePos.y + _sceneBounds.top);
	bool mouseDown = false;
	_field382 = 0;

	if ((event.mousePos.x > 270 && (_sceneBounds.right < (SCREEN_WIDTH * 2))))
		loadBackground(4, 0);
	else if ((event.mousePos.x < 50) && (_sceneBounds.left > 0))
		loadBackground(-4, 0);
	else
		mouseDown = event.eventType == EVENT_BUTTON_DOWN;

	if (BF_GLOBALS._player._uiEnabled) {
		Tooltip *tooltipList[9] = { &_location1, &_location2, &_location3, &_location4,
			&_location5, &_location6, &_location7, &_location8, &_location9 };

		for (int idx = 0; idx < 9; ++idx) {
			if (tooltipList[idx]->_bounds.contains(pt)) {
				// Found a tooltip to highlight
				tooltipList[idx]->highlight(mouseDown);
				return;
			}
		}

		// No hotspot selected, so remove any current tooltip display
		_text.remove();
	}
}

} // End of namespace BlueForce

} // End of namespace TsAGE
