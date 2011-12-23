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
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_speakers.h"

namespace TsAGE {

namespace Ringworld2 {

void Scene50::Action1::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		setDelay(180);
		break;
	case 2:
		R2_GLOBALS._sceneManager.changeScene(100);
		break;
	default:
		break;
	}
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit(OwnerList);
	loadScene(110);

	R2_GLOBALS._v58CE2 = 0;
	R2_GLOBALS._scenePalette.loadPalette(0);

	R2_GLOBALS._sound2.play(10);
	R2_GLOBALS._player.disableControl();

	setAction(&_action1);
}

void Scene50::process(Event &event) {
	if ((event.eventType != EVENT_BUTTON_DOWN) && (event.eventType != EVENT_KEYPRESS) && (event.eventType == 27)) {
		event.handled = true;
		warning("TODO: incomplete Scene50::process()");
		// CursorType _oldCursorId = _cursorId;
		g_globals->_events.setCursor(CURSOR_ARROW);
		// _cursorManager.sub_1D474(2, 0);
		// sub_5566A(1);
		// _cursorManager._fieldE = _oldCursorId;
		R2_GLOBALS._sceneManager.changeScene(100);
	}
}

/*--------------------------------------------------------------------------
 * Scene 100 - Quinn's Room
 *
 *--------------------------------------------------------------------------*/

bool Scene100::Door::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_state) {
			SceneItem::display2(100, 6);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 101;
			scene->setAction(&scene->_sequenceManager1, scene, 101, &R2_GLOBALS._player, this, NULL);
		}
		return true;
	case CURSOR_TALK:
		if (_state) {
			SceneItem::display2(100, 26);
			_state = 0;
			scene->_doorDisplay.setFrame(1);
		} else {
			SceneItem::display2(100, 27);
			_state = 1;
			scene->_doorDisplay.setFrame(2);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::Table::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		if (_strip == 2) {
			scene->_sceneMode = 108;
			scene->_object3.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 108, this, &scene->_object3, 
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 109;
			scene->setAction(&scene->_sequenceManager2, scene, 109, this, &scene->_object3, 
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		}
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		
		if (_strip == 2) {
			SceneItem::display2(100, 18);
			scene->_sceneMode = 102;
			scene->_object3.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 102, this, &scene->_object3, 
				&scene->_stasisNegator, NULL);
		} else {
			SceneItem::display2(100, 19);
			scene->_sceneMode = 103;
			scene->setAction(&scene->_sequenceManager2, scene, 103, this, &scene->_object3, 
				&scene->_stasisNegator, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::StasisNegator::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 107;
		scene->setAction(&scene->_sequenceManager1, scene, 107, &R2_GLOBALS._player, &scene->_stasisNegator, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::DoorDisplay::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(100, _state ? 24 : 25);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(100, _state ? 26 : 27);
		return true;
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 110;
		scene->setAction(&scene->_sequenceManager1, scene, 110, &R2_GLOBALS._player, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::SteppingDisks::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 111;
		scene->setAction(&scene->_sequenceManager1, scene, 111, &R2_GLOBALS._player, this, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene100::Terminal::startAction(CursorType action, Event &event) {
	Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 105;
		scene->setAction(&scene->_sequenceManager1, scene, 105, &R2_GLOBALS._player, this, NULL);
		return true;
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene100::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(100);
	R2_GLOBALS._scenePalette.loadPalette(0);

	if (R2_GLOBALS._sceneManager._previousScene != 125)
		R2_GLOBALS._sound1.play(10);

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, 4, 5, 1, NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, NULL);

	_table.postInit();
	_table.setup(100, 2, 3);
	_table.setPosition(Common::Point(175, 157));
	_table.setDetails(100, 17, 18, 20, 1, NULL);

	_bedLights1.postInit();
	_bedLights1.setup(100, 3, 1);
	_bedLights1.setPosition(Common::Point(89, 79));
	_bedLights1.fixPriority(250);
	_bedLights1.animate(ANIM_MODE_2, NULL);
	_bedLights1._numFrames = 3;

	_bedLights2.postInit();
	_bedLights2.setup(100, 3, 1);
	_bedLights2.setPosition(Common::Point(89, 147));
	_bedLights2.fixPriority(250);
	_bedLights2.animate(ANIM_MODE_7, 0, NULL); 
	_bedLights2._numFrames = 3;

	_wardrobe.postInit();
	_wardrobe.setVisage(101);
	_wardrobe.setPosition(Common::Point(231, 126));
	_wardrobe.fixPriority(10);
	_wardrobe.setDetails(100, 37, -1, 39, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_STEPPING_DISKS) == 100) {
		_steppingDisks.postInit();
		_steppingDisks.setup(100, 8, 1);
		_steppingDisks.setPosition(Common::Point(274, 130));
		_steppingDisks.setDetails(100, 40, -1, 42, 1, NULL);
	}

	_terminal.setDetails(11, 100, 14, 15, 16);
	_desk.setDetails(12, 100, 11, -1, 13);
	_bed.setDetails(13, 100, 8, 9, 10);
	_duct.setDetails(14, 100, 34, -1, 36);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();
	
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 100, 0, 1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 50:
	case 180:
		_object5.postInit();
		_object4.postInit();
		_sceneMode = 104;
		setAction(&_sequenceManager1, this, 104, &R2_GLOBALS._player, &_wardrobe, &_object4, &_object5, NULL);
		break;
	case 125:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 106, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		_sceneMode = 100;
		setAction(&_sequenceManager1, this, 100, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::remove() {
	R2_GLOBALS._sound1.play(10);
	SceneExt::remove();
}

void Scene100::signal() {
	switch (_sceneMode) {
	case 101:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 103:
	case 109:
		_table.setStrip(2);
		_table.setFrame(3);
		
		_object3.remove();
		_stasisNegator.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 104:
		_sceneMode = 0;
		_object5.remove();
		_object4.remove();

		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl();
		break;
	case 105:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 107:
		R2_GLOBALS._sceneItems.remove(&_stasisNegator);

		_stasisNegator.setFrame(2);
		R2_INVENTORY.setObjectScene(R2_NEGATOR_GUN, 1);
		R2_GLOBALS._player.enableControl();
		break;
	case 110:
		if (_door._state) {
			_door._state = 0;
			_doorDisplay.setFrame(1);
		} else {
			_door._state = 1;
			_doorDisplay.setFrame(2);
		}
		R2_GLOBALS._player.enableControl();
		break;
	case 111:
		R2_INVENTORY.setObjectScene(R2_STEPPING_DISKS, 1);
		_steppingDisks.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene100::dispatch() {
	int regionIndex = R2_GLOBALS._player.getRegionIndex();
	if (regionIndex == 13)
		R2_GLOBALS._player._shade = 4;

	if ((R2_GLOBALS._player._visage == 13) || (R2_GLOBALS._player._visage == 101))
	R2_GLOBALS._player._shade = 0;

	SceneExt::dispatch();

	if ((_sceneMode == 101) && (_door._frame == 2) && (_table._strip == 5)) {
		_table.setAction(&_sequenceManager2, NULL, 103, &_table, &_object3, &_stasisNegator, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 125 - Computer Terminal
 *
 *--------------------------------------------------------------------------*/

bool Scene125::Object5::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return true;
	else
		return SceneActor::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene125::Icon::Icon(): SceneActor()  {
	_lookLineNum = 0;
	_field98 = 0;
	_pressed = false;
}

void Scene125::Icon::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();
	
	_object1.postInit();
	_object1.fixPriority(255);
	_object1.hide();

	_sceneText1._color1 = 92;
	_sceneText1._color2 = 0;
	_sceneText1._width = 200;
	_sceneText2._color1 = 0;
	_sceneText2._color2 = 0;
	_sceneText2._width = 200;
	setDetails(125, -1, -1, -1, 2, NULL);
}

void Scene125::Icon::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_field98);
	s.syncAsSint16LE(_pressed);
}

void Scene125::Icon::process(Event &event) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled && !(_flags & OBJFLAG_HIDING) && R2_GLOBALS._player._uiEnabled) {

		if (event.eventType == EVENT_BUTTON_DOWN) {
			int regionIndex = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);

			switch (R2_GLOBALS._events.getCursor()) {
			case CURSOR_LOOK:
				if (regionIndex == _sceneRegionId) {
					event.handled = true;
					if (_lookLineNum == 26) {
						SceneItem::display2(130, 7);
					} else {
						SceneItem::display2(130, _lookLineNum);
					}
				}
				break;

			case CURSOR_USE:
				if ((regionIndex == _sceneRegionId) && !_pressed) {
					scene->_sound1.play(14);
					setFrame(2);

					switch (_object1._strip) {
					case 1:
						_object1.setStrip(2);
						break;
					case 3:
						_object1.setStrip(4);
						break;
					case 5:
						_object1.setStrip(6);
						break;
					default:
						break;
					}

					_pressed = true;
					event.handled = true;
				}
				break;

			default:
				break;
			}
		}

		if ((event.eventType == EVENT_BUTTON_UP) && _pressed) {
			setFrame(1);

			switch (_object1._strip) {
			case 2:
				_object1.setStrip(1);
				break;
			case 4:
				_object1.setStrip(3);
				break;
			case 6:
				_object1.setStrip(5);
				break;
			default:
				break;
			}

			_pressed = false;
			event.handled = true;
			scene->consoleAction(_lookLineNum);
		}
	}
}

void Scene125::Icon::setIcon(int id) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;

	_lookLineNum = _field98 = id;
	SceneActor::_lookLineNum = id;

	_sceneText1.remove();
	_sceneText2.remove();

	if (_lookLineNum) {
		showIcon();
		_object1.setup(161, ((id - 1) / 10) * 2 + 1, ((id - 1) % 10) + 1);
		_object1.setPosition(_position);

		_sceneText1._fontNumber = scene->_iconFontNumber;
		_sceneText1.setup(CONSOLE_MESSAGES[id]);
		_sceneText1.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE_MESSAGES[id]);
		_sceneText2.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE_MESSAGES[id]);
		_sceneText2.fixPriority(10);

		switch (_lookLineNum) {
		case 5:
			_sceneText1.setPosition(Common::Point(62, _position.y + 8));
			_sceneText2.setPosition(Common::Point(64, _position.y + 10));
			break;
		case 6:
		case 7:
		case 24:
		case 25:
			_sceneText1.setPosition(Common::Point(65, _position.y + 8));
			_sceneText2.setPosition(Common::Point(67, _position.y + 10));
			break;
		case 26:
			_sceneText1.setPosition(Common::Point(83, _position.y + 8));
			_sceneText2.setPosition(Common::Point(85, _position.y + 10));
			break;
		default:
			_sceneText1.setPosition(Common::Point(121, _position.y + 8));
			_sceneText2.setPosition(Common::Point(123, _position.y + 10));
			break;
		}
	} else {
		hideIcon();
	}
}

void Scene125::Icon::showIcon() {
	_sceneText1.show();
	_sceneText2.show();
	_object1.show();
	_object2.show();
	show();
}

void Scene125::Icon::hideIcon() {
	_sceneText1.hide();
	_sceneText2.hide();
	_object1.hide();
	_object2.hide();
	hide();
}

/*--------------------------------------------------------------------------*/

bool Scene125::Item4::startAction(CursorType action, Event &event) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;
	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[1]) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 126;
			scene->setAction(&scene->_sequenceManager, scene, 126, &scene->_object7, NULL);
			return true;
		}
		break;
	case R2_OPTO_DISK:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1) {
			R2_GLOBALS._player.disableControl();
			scene->_object7.postInit();
			scene->_sceneMode = 125;
			scene->setAction(&scene->_sequenceManager, scene, 125, &scene->_object7, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return SceneHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene125::Scene125(): SceneExt() {
	_iconFontNumber = 50;
	_consoleMode = 5;
	_logIndex = _databaseIndex = _infodiskIndex = 0;

	_soundCount = _soundIndex = 0;
	for (int i = 0; i < 10; ++i)
		_soundIndexes[i] = 0;
}

void Scene125::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(160);
	_palette.loadPalette(0);

	if (R2_GLOBALS._sceneManager._previousScene != 125) 
		// Save the prior scene to return to when the console is turned off
		R2_GLOBALS._player._oldCharacterScene[1] = R2_GLOBALS._sceneManager._previousScene;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[1]) {
		_object7.postInit();
		_object7.setup(160, 3, 5);
		_object7.setPosition(Common::Point(47, 167));
	}

	_object6.postInit();
	_object6.setup(162, 1, 1);
	_object6.setPosition(Common::Point(214, 168));
	
	_item4.setDetails(Rect(27, 145, 81, 159), 126, 9, -1, -1, 1, NULL);
	_item3.setDetails(Rect(144, 119, 286, 167), 126, 6, 7, 8, 1, NULL);
	_item2.setDetails(1, 126, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 126, 0, 1, -1, 1, NULL);

	_sceneMode = 1;
	signal();
}

void Scene125::signal() {
	switch (_sceneMode) {
	case 1:
		_icon1.postInit();
		_icon1._sceneRegionId = 2;
		_icon2.postInit();
		_icon2._sceneRegionId = 3;
		_icon3.postInit();
		_icon3._sceneRegionId = 4;
		_icon4.postInit();
		_icon4._sceneRegionId = 5;

		_sceneMode = 2;
		setAction(&_sequenceManager, this, 127, &_icon1, &_icon2, &_icon3, &_icon4, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		_icon1.setup(160, 1, 1);
		_icon1.setPosition(Common::Point(65, 17));
		_icon1._object2.postInit();
		_icon1._object2.setup(160, 7, 1);
		_icon1._object2.setPosition(Common::Point(106, 41));
		
		_icon2.setup(160, 1, 1);
		_icon2.setPosition(Common::Point(80, 32));
		_icon2._object2.postInit();
		_icon2._object2.setup(160, 7, 2);
		_icon2._object2.setPosition(Common::Point(106, 56));

		_icon3.setup(160, 1, 1);
		_icon3.setPosition(Common::Point(65, 47));
		_icon3._object2.postInit();
		_icon3._object2.setup(160, 7, 1);
		_icon3._object2.setPosition(Common::Point(106, 71));

		_icon4.setup(160, 1, 1);
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4._object2.postInit();
		_icon4._object2.setup(160, 7, 2);
		_icon4._object2.setPosition(Common::Point(106, 86));

		_icon5.postInit();
		_icon5.setup(160, 1, 1);
		_icon5.setPosition(Common::Point(37, 92));
		_icon5.setIcon(6);
		_icon5._sceneRegionId = 7;

		_icon6.postInit();
		_icon6.setup(160, 1, 1);
		_icon6.setPosition(Common::Point(106, 110));
		_icon6.setIcon(5);
		_icon6._sceneRegionId = 8;

		consoleAction(5);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 10:
 		switch (_consoleMode) {
		case 12:
			_sceneMode = 129;

			_object1.postInit();
			_object2.postInit();
			_object3.postInit();

			if (R2_GLOBALS.getFlag(13)) {
				_object4.postInit();
				setAction(&_sequenceManager, this, 130, &R2_GLOBALS._player, &_object1, &_object2,
					&_object3, &_object4, NULL);
			} else {
				setAction(&_sequenceManager, this, 129, &R2_GLOBALS._player, &_object1, &_object2, &_object3, NULL);
			}
			break;
		case 13:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_infodiskIndex = 0;
			setDetails(129, 0);
			break;
		case 23:
			R2_GLOBALS._sceneManager.changeScene(1330);
			break;
		case 27:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 0;
			setDetails(128, 0);
			break;
		case 28:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 37;
			setDetails(128, 37);
			break;
		case 29:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 68;
			setDetails(128, 68);
			break;
		case 30:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databaseIndex = 105;
			setDetails(128, 105);
			break;
		default:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_logIndex = 0;
			setDetails(127, 0);
			break;
		}
		break;
	case 11:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		
		if ((_consoleMode >= 27) && (_consoleMode <= 30)) {
			consoleAction(11);
		}
		consoleAction(2);
		_icon6.setIcon(5);
		break;
	case 12:
		if (_soundCount > 0)
			--_soundCount;
		if (!_soundCount || (R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex++], this);
		}
		break;
	case 125:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, R2_GLOBALS._player._oldCharacterScene[1]);
		break;
	case 126:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 1);
		_object7.remove();
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 128:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		SceneItem::display2(126, 12);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene125::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_consoleMode);
	s.syncAsSint16LE(_iconFontNumber);
	s.syncAsSint16LE(_logIndex);
	s.syncAsSint16LE(_databaseIndex);
	s.syncAsSint16LE(_infodiskIndex);
	s.syncAsSint16LE(_soundCount);
	s.syncAsSint16LE(_soundIndex);

	for (int i = 0; i < 10; ++i)
		s.syncAsSint16LE(_soundIndexes[i]);
}

void Scene125::process(Event &event) {
	SceneExt::process(event);

	if (R2_GLOBALS._player._uiEnabled) {
		_icon1.process(event);
		_icon2.process(event);
		_icon3.process(event);
		_icon4.process(event);
		_icon5.process(event);
		_icon6.process(event);
	}
}

void Scene125::dispatch() {
	if (_soundCount)
		R2_GLOBALS._playStream.proc1();

	Scene::dispatch();
}

/**
 * Handles actions on the console screen.
 */
void Scene125::consoleAction(int id) {
	_icon3.setIcon(0);
	_icon4.setIcon(0);

	if (id == 5)
		_icon5.setIcon(6);
	else {
		switch (_consoleMode) {
		case 10:
		case 12:
		case 13:
		case 27:
		case 28:
		case 29:
		case 30:
			break;
		default:
			_icon5.setIcon(7);
			break;
		}
	}

	switch (id) {
	case 1:
		_icon1.setIcon(8);
		_icon2.setIcon(9);
		break;
	case 2:
		_icon1.setIcon(10);
		_icon2.setIcon(11);
		_icon3.setIcon(12);
		_icon4.setIcon(13);
		break;
	case 3:
		_icon1.setIcon(15);
		_icon2.setIcon(16);
		_icon3.setIcon(17);
		break;
	case 4:
		_icon1.setIcon(22);
		_icon2.setIcon(23);
		break;
	case 6:
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._oldCharacterScene[1]);
		break;
	case 7:
		if (_consoleMode == 11)
			consoleAction(2);
		else if (_consoleMode == 22)
			consoleAction(4);
		else
			consoleAction(5);
		break;
	case 8:
		_iconFontNumber = 50;
		stop();
		_icon6.setIcon(5);
		consoleAction(1);
		break;
	case 9:
		_iconFontNumber = 52;
		stop();
		_icon6.setIcon(5);
		consoleAction(1);
		break;
	case 10:
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon5.setIcon(24);
		
		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(25);
		_icon4._object2.hide();

		_icon6.setIcon(26);
		_sceneMode = 10;
		
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 11:
		_icon1.setIcon(27);
		_icon2.setIcon(28);
		_icon3.setIcon(29);
		_icon4.setIcon(30);
		break;
	case 12:
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.hideIcon();

		_icon6.setIcon(26);
		_sceneMode = 10;
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 13:
		consoleAction(2);
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != R2_GLOBALS._player._oldCharacterScene[1]) {
			SceneItem::display2(126, 17);
		} else {
			R2_GLOBALS._player.disableControl();

			_icon1.hideIcon();
			_icon2.hideIcon();
			_icon3.hideIcon();
			_icon5.setIcon(24);
			
			_icon4.setPosition(Common::Point(52, 107));
			_icon4._sceneRegionId = 9;
			_icon4.setIcon(25);
			_icon4._object2.hide();

			_icon6.setIcon(26);
			_sceneMode = 10;
			
			_palette.loadPalette(161);
			R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		}
		break;
	case 15:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 3) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 2);
			_object5.setPosition(Common::Point(216, UI_INTERFACE_Y));

			R2_GLOBALS._v565F5 += 2;
		} else if (R2_GLOBALS._v565F5 == 3) {
			SceneItem::display2(126, 13);
		} else {
			SceneItem::display2(126, 14);
		}
		break;
	case 16:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 4) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 3);
			_object5.setPosition(Common::Point(218, UI_INTERFACE_Y));

			++R2_GLOBALS._v565F5;
		} else {
			SceneItem::display2(126, 15);
		}
		break;
	case 17:
		consoleAction(3);

		if (R2_GLOBALS._v565F5 < 4) {
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_object5.setup(162, 2, 1);
			_object5.setPosition(Common::Point(215, UI_INTERFACE_Y));

			++R2_GLOBALS._v565F5;
		} else {
			SceneItem::display2(126, 16);
		}
		break;
	case 22:
		_icon1.setIcon(31);
		_icon2.setIcon(32);
		_icon3.setIcon(33);
		_icon4.setIcon(34);
		break;
	case 23:
		R2_GLOBALS._player.disableControl();
		consoleAction(4);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.hideIcon();
		_icon6.hideIcon();

		_sceneMode = 10;
		_palette.loadPalette(161);
		break;
	case 24:
		_icon4.setIcon(25);
		_icon4._object2.hide();

		if (_consoleMode == 10) {
			setDetails(127, --_logIndex);
		} else if (_consoleMode == 13) {
			setDetails(129, --_infodiskIndex);
		} else {
			setDetails(128, --_databaseIndex);
		}
		break;
	case 25:
		_icon4.setIcon(25);
		_icon4._object2.hide();

		if (_consoleMode == 10) {
			setDetails(127, ++_logIndex);
		} else if (_consoleMode == 13) {
			setDetails(129, ++_infodiskIndex);
		} else {
			setDetails(128, ++_databaseIndex);
		}
		break;
	case 26:
		R2_GLOBALS._player.disableControl();
		stop();
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		R2_GLOBALS._player.hide();
		_object1.hide();
		_object2.hide();
		_object3.hide();
		_object4.hide();

		_sceneMode = 11;
		_palette.loadPalette(160);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 27:
	case 28:
	case 29:
	case 30:
		R2_GLOBALS._player.disableControl();
		consoleAction(11);
		_consoleMode = id;

		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();
		_icon5.setIcon(24);
		
		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(25);
		_icon4._object2.hide();

		_icon6.setIcon(26);
		_sceneMode = 10;
		
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 31:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 63 : 10);
		break;
	case 32:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 64 : 10);
		break;
	case 33:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 65 : 10);
		break;
	case 34:
		consoleAction(22);
		R2_GLOBALS._sound1.play((R2_GLOBALS._sound1.getSoundNum() == 10) ? 66 : 10);
		break;
	default:
		_icon1.setIcon(1);
		_icon2.setIcon(2);
		_icon3.setIcon(3);
		_icon4.setIcon(4);
		break;
	}

	if ((id != 6) && (id != 7) && (id != 24) && (id != 25))
		_consoleMode = id;
}

/**
 * Sets the message to be displayed on the console screen.
 */
void Scene125::setDetails(int resNum, int lineNum) {
	stop();
	
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum, true);

	if (!msg.empty()) {
		// Check for any specified sound numbers embedded in the message
		msg = parseMessage(msg);

		_sceneText._fontNumber = _iconFontNumber;
		_sceneText._color1 = 92;
		_sceneText._color2 = 0;
		_sceneText._width = 221;
		_sceneText.fixPriority(20);
		_sceneText.setup(msg);
		_sceneText.setPosition(Common::Point(49, 19));

		R2_GLOBALS._sceneObjects->draw();

		if ((_soundCount > 0) && (R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex], this);
		}
	} else {
		// Passed the start or end of the message set, so return to the menu
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();

		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		_consoleMode = 0;
		_palette.loadPalette(160);
		_sceneMode = 11;
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
	}
}

/**
 * Stops any playing console sounds and hides any current console message.
 */
void Scene125::stop() {
	_sceneText.remove();
	_soundIndex = 0;
	_soundCount = 0;

	R2_GLOBALS._playStream.stop();
}

/**
 * Parses a message to be displayed on the console to see whether there are any sounds to be played.
 */
Common::String Scene125::parseMessage(const Common::String &msg) {
	_soundIndex = 0;
	_soundCount = 0;

	const char *msgP = msg.c_str();
	while (*msgP == '!') {
		// Get the sound number
		_soundIndexes[_soundCount++] = atoi(++msgP);

		while (!((*msgP == '\0') || (*msgP < '0') || (*msgP > '9')))
			++msgP;
	}

	return Common::String(msgP);
}

/*--------------------------------------------------------------------------
 * Scene 150 - Empty Bedroom
 *
 *--------------------------------------------------------------------------*/

void Scene150::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(100);

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, -1, -1, 1, NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, NULL);

	_emptyRoomTable.postInit();
	_emptyRoomTable.setVisage(100);
	_emptyRoomTable.setStrip(4);
	_emptyRoomTable.setFrame(1);
	_emptyRoomTable.setPosition(Common::Point(175, 157));
	_emptyRoomTable.setDetails(150, 3, 4, 5, 1, NULL);

	_wardrobe.postInit();
	_wardrobe.setVisage(101);
	_wardrobe.setPosition(Common::Point(231, 126));
	_wardrobe.fixPriority(10);
	_wardrobe.setDetails(100, 37, -1, 39, 1, NULL);

	_terminal.setDetails(11, 100, 14, 15, 16);
	_desk.setDetails(12, 100, 11, -1, 13);
	_bed.setDetails(13, 100, 8, 9, 10);
	_duct.setDetails(14, 100, 34, -1, 36);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();
	
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 150, 0, 1, -1, 1, NULL);
	_sceneMode = 100;

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 100:
		setAction(&_sequenceManager1, this, 106, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		setAction(&_sequenceManager1, this, 100, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene150::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene150::signal() {
	switch (_sceneMode) {
	case 101:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 105:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 110:
		if (_door._state) {
			_door._state = 0;
			_doorDisplay.setFrame(1);
		} else {
			_door._state = 1;
			_doorDisplay.setFrame(2);
		}
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 200 - Ship Corridor
 *
 *--------------------------------------------------------------------------*/

bool Scene200::NorthDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::EastDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 200;
		scene->setAction(&scene->_sequenceManager, scene, 200, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::WestDoor::startAction(CursorType action, Event &event) {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 204;
		scene->setAction(&scene->_sequenceManager, scene, 204, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene200::EastExit::changeScene() {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 206;
	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene200::WestExit::changeScene() {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 208;
	scene->setAction(&scene->_sequenceManager, scene, 208, &R2_GLOBALS._player, NULL);
}

/*--------------------------------------------------------------------------*/

void Scene200::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(200);

	_westExit.setDetails(Rect(94, 0, 123, 58), EXITCURSOR_W, 175);
	_westExit.setDest(Common::Point(125, 52));
	_eastExit.setDetails(Rect(133, 0, 167, 58), EXITCURSOR_E, 150);
	_eastExit.setDest(Common::Point(135, 52));

	_northDoor.postInit();
	_northDoor.setVisage(200);
	_northDoor.setPosition(Common::Point(188, 79));
	_northDoor.setDetails(200, 3, -1, -1, 1, NULL);

	_eastDoor.postInit();
	_eastDoor.setVisage(200);
	_eastDoor.setStrip(2);
	_eastDoor.setPosition(Common::Point(305, 124));
	_eastDoor.setDetails(200, 6, -1, -1, 1, NULL);

	_westDoor.postInit();
	_westDoor.setVisage(200);
	_westDoor.setStrip(3);
	_westDoor.setPosition(Common::Point(62, 84));
	_westDoor.setDetails(200, 9, -1, -1, 1, NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_compartment.setDetails(Rect(4, 88, 153, 167), 200, 12, -1, -1, 1, NULL);
	_westDoorDisplay.setDetails(Rect(41, 51, 48, 61), 200, 15, -1, -1, 1, NULL);
	_eastDoorDisplay.setDetails(Rect(279, 67, 286, 78), 200, 18, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 200, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 100:
		_sceneMode = 201;
		setAction(&_sequenceManager, this, 201, &R2_GLOBALS._player, &_eastDoor, NULL);
		break;
	case 150:
		_sceneMode = 207;
		setAction(&_sequenceManager, this, 207, &R2_GLOBALS._player, NULL);
		break;
	case 175:
		_sceneMode = 209;
		setAction(&_sequenceManager, this, 209, &R2_GLOBALS._player, NULL);
		break;
	case 250:
		_sceneMode = 203;
		setAction(&_sequenceManager, this, 203, &R2_GLOBALS._player, &_northDoor, NULL);
		break;
	case 400:
		_sceneMode = 205;
		setAction(&_sequenceManager, this, 205, &R2_GLOBALS._player, &_westDoor, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(215, 115));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene200::signal() {
	switch (_sceneMode) {
	case 200:
		R2_GLOBALS._sceneManager.changeScene(100);
		break;
	case 202:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 204:
		R2_GLOBALS._sceneManager.changeScene(400);
		break;
	case 206:
		R2_GLOBALS._sceneManager.changeScene(150);
		break;
	case 208:
		R2_GLOBALS._sceneManager.changeScene(175);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 250 - Lift
 *
 *--------------------------------------------------------------------------*/

Scene250::Button::Button(): SceneActor() {
	_floorNumber = _v2 = 0;
}

void Scene250::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_floorNumber);
	s.syncAsSint16LE(_v2);
}

bool Scene250::Button::startAction(CursorType action, Event &event) {
	Scene250 *scene = (Scene250 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_field414) {
			SceneItem::display2(250, 15);
		} else {
			switch (_floorNumber) {
			case 1:
			case 2:
			case 5:
			case 9:
				scene->_sound1.play(14);
				scene->changeFloor(_floorNumber);
				break;
			case 10:
				// Current Floor
				scene->_sound1.play(14);
				R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);
				break;
			default:
				SceneItem::display2(250, 16);
				break;
			}
		}
		return true;

	case CURSOR_LOOK:
		switch (_floorNumber) {
		case 1:
		case 2:
		case 5:
		case 9:
			SceneItem::display2(250, 12);
			break;
		case 10:
			SceneItem::display2(250, 13);
			break;
		case 11:
			SceneItem::display2(250, 14);
			break;
		default:
			SceneItem::display2(250, 16);
			break;
		}
		return true;

	default:
		return SceneActor::startAction(action, event);
	}
}

void Scene250::Button::setFloor(int floorNumber) {
	SceneActor::postInit();
	_floorNumber = floorNumber;
	_v2 = 0;

	if (_floorNumber <= 9) {
		SceneObject::setup(250, 1, 4);

		switch (_floorNumber) {
		case 1:
		case 2:
		case 5:
		case 9:
			setFrame(6);
			break;
		default:
			break;
		}

		setPosition(Common::Point(111, (_floorNumber - 1) * 12 + 43));
		fixPriority(10);
		setDetails(250, -1, -1, -1, 1, NULL);
	}
}

/*--------------------------------------------------------------------------*/

Scene250::Scene250(): SceneExt() {
	_field412 = _field414 = _field416 = _field418 = _field41A = 0;
}

void Scene250::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
}

void Scene250::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(250);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;

	_currentFloor.setFloor(10);
	_currentFloor.setup(250, 1, 5);
	_currentFloor.setDetails(250, 13, -1, -1, 1, NULL);

	_button1.setFloor(11);
	_button1.setup(250, 1, 3);
	_button1.setPosition(Common::Point(400, 100));
	_button1.setDetails(250, 14, -1, -1, 1, NULL);
	_button1.fixPriority(190);
	_button1.hide();

	_floor1.setFloor(1);
	_floor2.setFloor(2);
	_floor3.setFloor(3);
	_floor4.setFloor(4);
	_floor5.setFloor(5);
	_floor6.setFloor(6);
	_floor7.setFloor(7);
	_floor8.setFloor(8);
	_floor9.setFloor(9);

	_item2.setDetails(Rect(0, 0, 73, SCREEN_HEIGHT), 250, 9, -1, 9, 1, NULL);
	_item4.setDetails(Rect(239, 16, 283, 164), 250, 6, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 250, 0, 1, -1, 1, NULL);

	R2_GLOBALS._events.setCursor(CURSOR_USE);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 200:
		_field412 = 55;
		break;
	case 300:
		_field412 = 43;
		break;
	case 700:
		_field412 = 139;
		break;
	case 850:
		_field412 = 91;
		break;
	default:
		R2_GLOBALS._sceneManager._previousScene = 200;
		_field412 = 55;
		break;
	}

	_currentFloor.setPosition(Common::Point(111, _field412));
}

void Scene250::signal() {
	if (_field41A)
		_sceneMode = 20;

	switch (_sceneMode) {
	case 1:
		_sound1.play(22);
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setup(250, 1, 2);
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);

		_field416 = 0;
		_sceneMode = 2;
		break;
	case 2:
		_sceneMode = ((_field414 - 12) == _field412) ? 4 : 3;
		signal();
		break;
	case 3:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y + 12));
		_field412 += 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);
		
		if ((_field414 - 12) == _field412)
			_sceneMode = 4;
		break;
	case 4:
		_sound1.play(21);
		
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y + 12));
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);
		_sceneMode = 5;
		break;
	case 5:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 20;
		signal();
		break;
	case 6:
		_sound1.play(22);
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setup(250, 1, 2);
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);
		_field416 = 0;
		_sceneMode = 7;
		break;
	case 7:
		_field418 = 1;
		if ((_field414 + 12) == _field412)
			_sceneMode = 8;
		signal();
		break;
	case 8:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y - 12));
		_field412 -= 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);

		if ((_field414 + 12) == _field412)
			_sceneMode = 9;
		break;
	case 9:
		_sound1.play(21);
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y - 12));
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);
		_sceneMode = 10;
		break;
	case 10:
		_sceneMode = 20;
		signal();
		break;
	case 20:
		// Handle changing scene
		switch (_field414) {
		case 55:
			R2_GLOBALS._sceneManager.changeScene(200);
			break;
		case 43:
			R2_GLOBALS._sceneManager.changeScene(300);
			break;
		case 139:
			R2_GLOBALS._sceneManager.changeScene(139);
			break;
		case 91:
			R2_GLOBALS._sceneManager.changeScene(850);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Scene250::changeFloor(int floorNumber) {
	_field414 = (floorNumber - 1) * 12 + 43;
	_button1.setPosition(Common::Point(111, _field414));
	_button1.show();

	_sceneMode = (_field412 >= _field414) ? 6 : 1;
	if (_field414 == _field412)
		_sceneMode = 20;

	signal();
}

void Scene250::process(Event &event) {
	if (!event.handled) {
		if (((event.eventType == EVENT_KEYPRESS) || (event.btnState != 0)) && _field418) {
			_field41A = 1;
			event.handled = true;
		}

		SceneExt::process(event);
	}
}

void Scene250::dispatch() {
	SceneExt::dispatch();

	if (((_sceneMode == 2) || (_sceneMode == 7)) && (_field416 < 100)) {
		++_field416;
		R2_GLOBALS._player._moveDiff.y = _field416 / 5;
	}

	if (((_sceneMode == 5) || (_sceneMode == 10)) && (R2_GLOBALS._player._moveDiff.y > 4)) {
		--_field416;
		R2_GLOBALS._player._moveDiff.y = _field416 / 7 + 3;
	}
}

/*--------------------------------------------------------------------------
 * Scene 300 - Bridge
 *
 *--------------------------------------------------------------------------*/

void Scene300::Action1::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager2, this, 311, (R2_GLOBALS._player._characterIndex == 1) ?
			(SceneObject *)&R2_GLOBALS._player : (SceneObject *)&scene->_quinn);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager2, this, 312, (R2_GLOBALS._player._characterIndex == 1) ?
			(SceneObject *)&R2_GLOBALS._player : (SceneObject *)&scene->_quinn);
		_actionIndex = 0;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);
		break;
	default:
		break;
	}
}

void Scene300::Action2::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager3, this, 302, &scene->_seeker, NULL);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager3, this, 303, &scene->_seeker, NULL);
		_actionIndex = 2;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);
			
		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(119));
		break;
	default:
		break;
	}
}

void Scene300::Action3::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0:
		setAction(&scene->_sequenceManager3, this, 304, &scene->_miranda, NULL);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager3, this, 305, &scene->_miranda, NULL);
		_actionIndex = 2;
		break;
	case 2:
		if (!R2_GLOBALS._playStream.isPlaying())
			_actionIndex = R2_GLOBALS._randomSource.getRandomNumber(1);
			
		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(119));
		break;
	default:
		break;
	}
}


void Scene300::Action4::signal() {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	if (!R2_GLOBALS._playStream.isPlaying()) {
		scene->_object7.setStrip2(R2_GLOBALS._randomSource.getRandomNumber(2));
		scene->_object7.setFrame(1);

		scene->_object9.setStrip2(3);
		scene->_object9.setFrame(1);
	}

	setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(479));
}

/*--------------------------------------------------------------------------*/

bool Scene300::QuinnWorkstation::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 1)
			SceneItem::display2(300, 46);
		else if (R2_GLOBALS.getFlag(44)) {
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._sceneManager.changeScene(325);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 306;
			scene->setAction(&scene->_sequenceManager1, scene, 306, &R2_GLOBALS._player, NULL);
		}
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 1) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene300::MirandaWorkstation::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 3)
			SceneItem::display2(300, 49);
		else
			R2_GLOBALS._sceneManager.changeScene(325);			
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 3) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

bool Scene300::SeekerWorkstation::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == 2) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != 2)
			SceneItem::display2(300, 48);
		else
			R2_GLOBALS._sceneManager.changeScene(325);			
		return true;

	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene300::Miranda::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			// Quinn talking to Miranda
			R2_GLOBALS._player.disableControl();

			if (!R2_GLOBALS.getFlag(44)) {
				if (R2_GLOBALS.getFlag(40))
					scene->_stripId = 119;
				else if (R2_GLOBALS.getFlag(38))
					scene->_stripId = 101;
				else {
					R2_GLOBALS._sound1.play(69);
					scene->_stripId = 100;
				}

				scene->_sceneMode = 309;
				scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
			} else if (!R2_GLOBALS.getFlag(55)) {
				R2_GLOBALS._events.setCursor(CURSOR_ARROW);
				scene->_sceneMode = 10;
				scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
			} else {
				scene->_sceneMode = 16;

				if (!R2_GLOBALS.getFlag(57)) {
					R2_GLOBALS._events.setCursor(CURSOR_ARROW);
					scene->_stripManager.start3(434, scene, R2_GLOBALS._stripManager_lookupList);
				} else if (R2_GLOBALS._player._characterScene[R2_MIRANDA] != 500) {
					R2_GLOBALS._events.setCursor(CURSOR_ARROW);
					scene->_stripManager.start3(407, scene, R2_GLOBALS._stripManager_lookupList);
				} else {
					scene->_stripId = 433;
					scene->_sceneMode = 309;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				}
			}
		} else {
			// Seeker talking to Miranda
			scene->_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 174 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 211;
			else
				scene->_stripId = 438;

			scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		}
		return true;

	case R2_OPTO_DISK:
		SceneItem::display2(300, 54);
		return true;

	case R2_READER:
		if (!R2_GLOBALS.getFlag(2) || !R2_GLOBALS.getFlag(3) || (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1))
			SceneItem::display2(300, 55);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_stripId = R2_GLOBALS.getFlag(4) ? 121 : 120;
			scene->_sceneMode = 309;
			scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
		}
		return true;

	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene300::Seeker::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(44)) {
				if (!R2_GLOBALS.getFlag(38)) {
					R2_GLOBALS._sound1.play(69);
					scene->_stripId = 181;
					scene->_sceneMode = 310;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				} else {
					scene->_stripId = R2_GLOBALS.getFlag(40) ? 170 : 150;
					scene->_sceneMode = 310;
					scene->setAction(&scene->_sequenceManager1, scene, 309, &R2_GLOBALS._player, NULL);
				}
			} else {
				R2_GLOBALS._events.setCursor(CURSOR_ARROW);
				if (!R2_GLOBALS.getFlag(55)) {
					scene->_sceneMode = 10;
					scene->_stripManager.start3(205, scene, R2_GLOBALS._stripManager_lookupList);
				} else {
					scene->_sceneMode = 16;
					scene->_stripManager.start3(R2_GLOBALS.getFlag(57) ? 407 : 401, scene, R2_GLOBALS._stripManager_lookupList);
				}
			}
		} else {
			scene->_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 122 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 209;
			else
				scene->_stripId = 440;

			scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		}
		return true;

	case R2_OPTO_DISK:
		if (R2_GLOBALS.getFlag(13)) {
			SceneItem::display2(300, 53);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_stripId = 171;
		}

		scene->_sceneMode = 310;
		scene->setAction(&scene->_sequenceManager1, scene, 310, &R2_GLOBALS._player, NULL);
		return true;

	case R2_READER:
		if (!R2_GLOBALS.getFlag(2) || !R2_GLOBALS.getFlag(3) || (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1))
			break;

		R2_GLOBALS._player.disableControl();
		scene->_stripId = R2_GLOBALS.getFlag(4) ? 173 : 172;
		scene->_sceneMode = 310;
		scene->setAction(&scene->_sequenceManager1, scene, 310, &R2_GLOBALS._player, NULL);
		return true;
	
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene300::Quinn::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		scene->_sceneMode = 10;

		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA) {
			if (R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500)
				scene->_stripId = 442;
			else if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 177 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 208;
			else
				scene->_stripId = 441;
		} else if (R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500) {
			scene->_stripId = 442;
		} else if (R2_GLOBALS.getFlag(44)) {
			scene->_stripId = R2_GLOBALS.getFlag(55) ? 441 : 208;
		} else {
			scene->_stripId = 125 + R2_GLOBALS._randomSource.getRandomNumber(2);
		}

		scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		return true;

	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene300::Doorway::startAction(CursorType action, Event &event) {
	Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		if ((R2_GLOBALS._player._characterIndex == R2_QUINN) && 
				(!R2_GLOBALS.getFlag(44) || R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 301;
			scene->setAction(&scene->_sequenceManager1, scene, 301, &R2_GLOBALS._player, this, NULL);
		} else {
			SceneItem::display2(300, 45);
		}
		
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene300::Scene300(): SceneExt() {
	_stripId = 0;
	_rotation = NULL;
}

void Scene300::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_stripId);
	SYNC_POINTER(_rotation);
}

void Scene300::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(300);
	_sound1.play(23);
	setZoomPercents(75, 93, 120, 100);

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1000;
		R2_GLOBALS._player._characterIndex = R2_QUINN;
	}

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_quinnLSpeaker);
	_stripManager.addSpeaker(&_seekerLSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_soldierSpeaker);

	_rotation = R2_GLOBALS._scenePalette.addRotation(237, 254, -1);
	_rotation->setDelay(3);
	_rotation->_countdown = 1;

	if (R2_GLOBALS.getFlag(51) && !R2_GLOBALS.getFlag(25)) {
		_object1.postInit();
		_object1.setup(301, 7, 2);
		_object1.setPosition(Common::Point(65, 24));

		_object2.postInit();
		_object2.setup(301, 8, 2);
		_object2.setPosition(Common::Point(254, 24));
	}

	_doorway.postInit();
	_doorway.setVisage(300);
	_doorway.setPosition(Common::Point(159, 79));

	_object3.postInit();
	_object3.setup(300, 4, 1);
	_object3.setPosition(Common::Point(84, 48));
	_object3.animate(ANIM_MODE_2, NULL);
	_object3._numFrames = 5;

	_object4.postInit();
	_object4.setup(300, 5, 1);
	_object4.setPosition(Common::Point(236, 48));
	_object4.animate(ANIM_MODE_2, NULL);

	_protocolDisplay.postInit();
	_protocolDisplay.setup(300, 6, 1);
	_protocolDisplay.setPosition(Common::Point(287, 71));
	_protocolDisplay.animate(ANIM_MODE_7, NULL);
	_protocolDisplay._numFrames = 5;

	_object6.postInit();
	_object6.setup(300, 7, 1);
	_object6.setPosition(Common::Point(214, 37));
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 3;

	_object7.postInit();
	_object7.setup(301, 1, 1);
	_object7.setPosition(Common::Point(39, 97));
	_object7.fixPriority(124);
	_object7.animate(ANIM_MODE_2, NULL);
	_object7._numFrames = 5;
	_object7.setAction(&_action4);

	_object8.postInit();
	_object8.setup(300, 8, 1);
	_object8.setPosition(Common::Point(105, 37));
	_object8.animate(ANIM_MODE_2, NULL);
	_object8._numFrames = 5;

	_object9.postInit();
	_object9.setup(301, 6, 1);
	_object9.setPosition(Common::Point(274, 116));
	_object9.fixPriority(143);
	_object9.animate(ANIM_MODE_2, NULL);
	_object9._numFrames = 5;

	_quinnWorkstation1.setDetails(Rect(243, 148, 315, 167), 300, 30, 31, 32, 1, NULL);
	_mirandaWorkstation1.setDetails(Rect(4, 128, 69, 167), 300, 33, 31, 35, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case 1:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, NULL);

		if ((R2_GLOBALS._player._characterScene[2] == 300) || (R2_GLOBALS._player._characterScene[2] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setVisage(10);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.disableControl();
		break;

	case 2:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, NULL);

		if ((R2_GLOBALS._player._characterScene[1] == 300) || (R2_GLOBALS._player._characterScene[1] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 1, 3);
		R2_GLOBALS._player.setPosition(Common::Point(158, 108));
		R2_GLOBALS._player.fixPriority(130);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	case 3:
		if ((R2_GLOBALS._player._characterScene[2] == 300) || (R2_GLOBALS._player._characterScene[2] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, NULL);
		}

		if ((R2_GLOBALS._player._characterScene[1] == 300) || (R2_GLOBALS._player._characterScene[1] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 2, 1);
		R2_GLOBALS._player.setPosition(Common::Point(47, 128));
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	default:
		break;
	}

	_seekerWorkstation.setDetails(Rect(101, 95, 217, 143), 300, 36, 31, 35, 1, NULL);
	_quinnWorkstation2.setDetails(Rect(224, 102, 315, 143), 300, 30, 31, 32, 1, NULL);
	_mirandaWorkstation2.setDetails(Rect(4, 83, 84, 124), 300, 33, 31, 35, 1, NULL);
	_hull.setDetails(11, 300, 6, -1, -1);
	_statusDisplays.setDetails(12, 300, 9, 10, -1);
	_damageControl.setDetails(13, 300, 12, -1, -1);
	_manualOverrides.setDetails(14, 300, 15, -1, 17);
	_scanners1.setDetails(Rect(126, 15, 183, 25), 300, 18, -1, 20, 1, NULL);
	_scanners2.setDetails(Rect(126, 80, 183, 90), 300, 18, -1, 20, 1, NULL);
	_protocolDisplay.setDetails(300, 27, -1, 29, 1, NULL);
	_indirectLighting1.setDetails(Rect(74, 71, 122, 89), 300, 21, -1, -1, 1, NULL);
	_indirectLighting2.setDetails(Rect(197, 71, 245, 89), 300, 21, -1, -1, 1, NULL);
	_lighting.setDetails(Rect(129, 3, 190, 14), 300, 24, -1, -1, 1, NULL);
	_doorway.setDetails(300, 3, -1, 5, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 300, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case 1:
		_sceneMode = 300;

		switch (R2_GLOBALS._sceneManager._previousScene) {
		case 250:
			setAction(&_sequenceManager1, this, 300, &R2_GLOBALS._player, &_doorway, NULL);
			break;
		case 325:
			if (!R2_GLOBALS.getFlag(44) || R2_GLOBALS.getFlag(25))
				setAction(&_sequenceManager1, this, 309, &R2_GLOBALS._player, NULL);
			else {
				R2_GLOBALS.setFlag(60);
				R2_GLOBALS._player.setup(302, 3, 1);
				R2_GLOBALS._player.setPosition(Common::Point(271, 150));
				R2_GLOBALS._player.setAction(&_action1);
				
				if (R2_GLOBALS.getFlag(55)) {
					if (R2_GLOBALS.getFlag(57)) {
						R2_GLOBALS.clearFlag(60);
						R2_GLOBALS._events.setCursor(CURSOR_ARROW);
						_sceneMode = 16;
						_stripManager.start(404, this);
					} else {
						R2_GLOBALS._player.enableControl();
						R2_GLOBALS._player._canWalk = false;
					}
				} else {
					if (R2_GLOBALS.getFlag(45)) {
						R2_GLOBALS.clearFlag(60);
						R2_GLOBALS._events.setCursor(CURSOR_ARROW);
						_sceneMode = 12;
						_stripManager.start3(204, this, R2_GLOBALS._stripManager_lookupList);
					} else {
						R2_GLOBALS._player.enableControl();
						R2_GLOBALS._player._canWalk = false;
					}
				}
			}
			break;
		case 1000:
			R2_GLOBALS.setFlag(60);
			R2_GLOBALS._player.setup(302, 3, 1);
			R2_GLOBALS._player.setPosition(Common::Point(271, 150));
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);

			if (R2_GLOBALS.getFlag(51)) {
				_sceneMode = 13;
				_stripManager.start3(300, this, R2_GLOBALS._stripManager_lookupList);
			} else {
				_sceneMode = 11;
				_stripManager.start3(200, this, R2_GLOBALS._stripManager_lookupList);
			}
			break;

		case 1100:
			R2_GLOBALS._player.setVisage(10);
			R2_GLOBALS._player.setPosition(Common::Point(160, 95));
			_stripId = 400;
			_sceneMode = 309;
			setAction(&_sequenceManager1, this, 309, &R2_GLOBALS._player, NULL);
			break;

		case 1500:
			R2_GLOBALS.clearFlag(60);
			R2_GLOBALS._player.setup(302, 3, 1);
			R2_GLOBALS._player.setPosition(Common::Point(271, 150));
			_sceneMode = 17;
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);
			_stripManager.start(413, this);
			break;

		default:
			if (R2_GLOBALS.getFlag(60)) {
				R2_GLOBALS._player.setup(302, 3, 1);
				R2_GLOBALS._player.setPosition(Common::Point(271, 150));
				R2_GLOBALS._player.setAction(&_action1);
				R2_GLOBALS._player.enableControl(CURSOR_USE);
				R2_GLOBALS._player._canWalk = false;
			} else {
				R2_GLOBALS._player.setStrip(3);
				R2_GLOBALS._player.setPosition(Common::Point(200, 150));
				R2_GLOBALS._player.enableControl();
			}
			break;
		}
		break;

	case 3:
		if (R2_GLOBALS._sceneManager._previousScene == 1500) {
			R2_GLOBALS._player._oldCharacterScene[3] = 3150;
			R2_GLOBALS._player._characterScene[3] = 3150;
			R2_GLOBALS._player._effect = 0;
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._player.disableControl();

			_quinn.postInit();
			_quinn.setVisage(10);
			_quinn.setPosition(Common::Point(10, 10));
			_quinn.hide();

			_seeker.postInit();
			_seeker.setVisage(20);
			_seeker.setPosition(Common::Point(20, 20));
			_seeker.hide();

			_teal.postInit();
			_soldier.postInit();
			_object12.postInit();

			R2_GLOBALS._sound1.play(107);
			_sceneMode = 308;

			setAction(&_sequenceManager1, this, 308, &R2_GLOBALS._player, &_teal, &_soldier, &_object12, &_doorway, NULL);
		}
		break;
	default:
		break;
	}
}

void Scene300::remove() {
	R2_GLOBALS._player.setAction(NULL);
	SceneExt::remove();
}

void Scene300::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_stripManager._field2E8) {
		case 0:
			R2_GLOBALS._sound1.changeSound(10);
			R2_GLOBALS.setFlag(38);
			break;
		case 1:
			R2_GLOBALS.setFlag(3);
			break;
		case 2:
			R2_GLOBALS.setFlag(4);
			break;
		case 3:
			R2_GLOBALS.setFlag(13);
			if (R2_GLOBALS._stripManager_lookupList[1] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 4:
			if (R2_GLOBALS._stripManager_lookupList[1] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 5:
			R2_GLOBALS._sceneManager.changeScene(1000);
			break;
		default:
			break;
		}

		_stripManager._field2E8 = 0;
		switch (_stripId) {
		case 400:
			R2_GLOBALS._player.disableControl();
			_sceneMode = 15;
			setAction(&_sequenceManager1, this, 306, &R2_GLOBALS._player, NULL);
			break;
		case 181:
			R2_GLOBALS._player.setStrip(6);
			// Deliberate fall-through
		default:
			R2_GLOBALS._player.enableControl(CURSOR_TALK);

			if ((R2_GLOBALS._player._characterIndex != 1) || R2_GLOBALS.getFlag(44))
				R2_GLOBALS._player._canWalk = false;
			break;
		}
		break;

	case 11:
		R2_GLOBALS.setFlag(44);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;

	case 12:
		R2_GLOBALS._player.setAction(NULL);
		R2_GLOBALS._sceneManager.changeScene(1010);
		break;

	case 13:
		R2_GLOBALS._player.disableControl();
		_seeker.changeZoom(-1);
		_sceneMode = 313;
		_seeker.setAction(NULL);
		setAction(&_sequenceManager1, this, 313, &R2_GLOBALS._player, &_seeker, NULL);
		break;

	case 14:
		if (_seeker._action)
			R2_GLOBALS._player.disableControl();
		_sceneMode = 314;
		break;

	case 15:
		R2_GLOBALS.setFlag(55);
		R2_GLOBALS.setFlag(38);
		R2_GLOBALS.setFlag(44);
		R2_GLOBALS.setFlag(51);
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 16;
		_stripManager.start3(401, this, R2_GLOBALS._stripManager_lookupList);
		break;

	case 16:
		if (_stripManager._field2E8 == 1) {
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._sceneManager.changeScene(1000);
		} else {
			R2_GLOBALS._player.setAction(&_action1);
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
		}
		break;

	case 17:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 316;
		_seeker.changeZoom(-1);
		_seeker.setAction(&_sequenceManager3, this, 316, &_seeker, &_doorway, NULL);
		R2_GLOBALS._player.setAction(&_sequenceManager1, NULL, 307, &R2_GLOBALS._player, NULL);
		break;

	case 18:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 317;
		setAction(&_sequenceManager1, this, 317, &_teal, &_doorway, NULL);
		break;

	case 19:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 318;
		setAction(&_sequenceManager1, this, 318, &R2_GLOBALS._player, &_teal, &_soldier, &_object12, NULL);
		break;

	case 20:
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._sceneManager.changeScene(1500);
		break;

	case 300:
	case 307:
		R2_GLOBALS._player.enableControl();
		break;

	case 301:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;

	case 306:
		R2_GLOBALS._sceneManager.changeScene(325);
		break;

	case 308:
		_sceneMode = 18;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(418, this);
		break;

	case 310:
		R2_GLOBALS._player.setStrip(5);
		// Deliberate fall-through
	case 309:
		signal309();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 10;
		_stripManager.start3(_stripId, this, R2_GLOBALS._stripManager_lookupList);
		break;

	case 313:
		_sceneMode = 14;
		R2_GLOBALS._player._effect = 0;
		_seeker.setAction(&_sequenceManager3, this, 314, &_seeker, &_doorway, NULL);
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(301, this);
		break;

	case 314:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 315;
		R2_GLOBALS._player._effect = 1;
		setAction(&_sequenceManager1, this, 315, &R2_GLOBALS._player, &_doorway, NULL);
		break;

	case 315:
		R2_GLOBALS._sceneManager.changeScene(1100);
		break;

	case 316:
		R2_GLOBALS._player._characterScene[2] = 500;
		_seeker.remove();
		R2_GLOBALS._player.enableControl(CURSOR_CROSSHAIRS);
		break;

	case 317:
		_sceneMode = 19;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(419, this);
		break;

	case 318:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(420, this);
		break;

	default:
		break;
	}
}

void Scene300::signal309() {
	if (R2_GLOBALS.getFlag(2))
		R2_GLOBALS._stripManager_lookupList[0] = (R2_INVENTORY.getObjectScene(R2_READER) == 1) ? 3 : 2;

	if (R2_GLOBALS.getFlag(4))
		R2_GLOBALS._stripManager_lookupList[0] = 4;

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1)
		R2_GLOBALS._stripManager_lookupList[0] = 5;

	if (R2_GLOBALS.getFlag(13)) {
		R2_GLOBALS._stripManager_lookupList[0] = 6;
		R2_GLOBALS._stripManager_lookupList[2] = 2;
	}

	if (R2_GLOBALS.getFlag(39))
		R2_GLOBALS._stripManager_lookupList[1] = 2;
	
	if (R2_GLOBALS.getFlag(5))
		R2_GLOBALS._stripManager_lookupList[1] = 3;

	if (R2_GLOBALS.getFlag(6))
		R2_GLOBALS._stripManager_lookupList[1] = 4;

	if (R2_GLOBALS.getFlag(8))
		R2_GLOBALS._stripManager_lookupList[1] = 5;

	if (R2_GLOBALS.getFlag(9)) {
		R2_GLOBALS._stripManager_lookupList[1] = 6;
		R2_GLOBALS._stripManager_lookupList[3] = 2;
	}

	if (R2_GLOBALS.getFlag(48))
		R2_GLOBALS._stripManager_lookupList[4] = 2;

	if (R2_GLOBALS.getFlag(49))
		R2_GLOBALS._stripManager_lookupList[4] = 3;
}

/*--------------------------------------------------------------------------
 * Scene 400 - Science Lab
 *
 *--------------------------------------------------------------------------*/

bool Scene400::Terminal::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 402;
		scene->setAction(&scene->_sequenceManager1, scene, 402, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene400::Door::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 401;
		scene->setAction(&scene->_sequenceManager1, scene, 401, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::Reader::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 405;
		scene->setAction(&scene->_sequenceManager1, scene, 405, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::SensorProbe::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 404;
		scene->setAction(&scene->_sequenceManager1, scene, 404, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::AttractorUnit::startAction(CursorType action, Event &event) {
	Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 406;
		scene->setAction(&scene->_sequenceManager1, scene, 406, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene400::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(400);
	_sound1.play(20);

	_door.postInit();
	_door.setVisage(100);
	_door.setPosition(Common::Point(123, 84));
	_door.setDetails(400, 24, -1, 26, 1, NULL);

	_consoleDisplay.postInit();
	_consoleDisplay.setup(400, 4, 1);
	_consoleDisplay.setPosition(Common::Point(236, 92));
	_consoleDisplay.fixPriority(120);
	_consoleDisplay.animate(ANIM_MODE_2, NULL);
	_consoleDisplay._numFrames = 5;

	_testerDisplay.postInit();
	_testerDisplay.setup(400, 2, 1);
	_testerDisplay.setPosition(Common::Point(198, 83));
	_testerDisplay.animate(ANIM_MODE_2, NULL);
	_testerDisplay._numFrames = 20;

	if (R2_INVENTORY.getObjectScene(R2_READER) == 400) {
		_reader.postInit();
		_reader.setup(400, 5, 2);
		_reader.setPosition(Common::Point(301, 95));
		_reader.setDetails(400, 54, -1, 56, 1, NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_SENSOR_PROBE) == 400) {
		_sensorProbe.postInit();
		_sensorProbe.setup(400, 5, 1);
		_sensorProbe.setPosition(Common::Point(251, 104));
		_sensorProbe.fixPriority(121);
		_sensorProbe.setDetails(400, 57, -1, 59, 1, NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_UNIT) == 400) {
		_attractorUnit.postInit();
		_attractorUnit.setup(400, 5, 3);
		_attractorUnit.setPosition(Common::Point(265, 129));
		_attractorUnit.setDetails(400, 60, -1, 62, 1, NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_equipment1.setDetails(11, 400, 3, -1, -1);
	_equipment2.setDetails(24, 400, 3, -1, -1);
	_equipment3.setDetails(25, 400, 3, -1, -1);
	_equipment4.setDetails(26, 400, 3, -1, -1);
	_equipment5.setDetails(28, 400, 3, -1, -1);
	_equipment6.setDetails(29, 400, 3, -1, -1);
	_desk.setDetails(12, 400, 6, -1, -1);
	_desk2.setDetails(27, 400, 6, -1, -1);
	_terminal.setDetails(13, 400, 6, -1, 11);
	_duct.setDetails(14, 400, 12, -1, -1);
	_console.setDetails(15, 400, 15, -1, 17);
	_equalizer.setDetails(Rect(284, 99, 308, 108), 400, 36, -1, 38, 1, NULL);
	_transducer.setDetails(Rect(295, 67, 314, 79), 400, 39, -1, 41, 1, NULL);
	_optimizer.setDetails(Rect(308, 106, 315, 113), 400, 42, -1, 44, 1, NULL);
	_soundModule.setDetails(Rect(291, 118, 315, 131), 400, 45, -1, 47, 1, NULL);
	_tester.setDetails(Rect(179, 62, 217, 92), 400, 30, -1, 32, 1, NULL);
	_helmet.setDetails(Rect(181, 53, 197, 65), 400, 48, -1, 50, 1, NULL);
	_nullifier.setDetails(Rect(201, 56, 212, 65), 400, 51, -1, 50, 1, NULL);
	_shelves.setDetails(16, 400, 18, -1, 20);
	_cabinet.setDetails(17, 400, 21, -1, -1);
	_doorDisplay.setDetails(Rect(161, 43, 166, 52), 400, 27, -1, -1, 1, NULL);
	_lights.setDetails(Rect(113, 3, 168, 14), 400, 33, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 400, 0, 1, -1, 1, NULL);

	_sceneMode = 400;
	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 125:
		setAction(&_sequenceManager1, this, 403, &R2_GLOBALS._player, NULL);
		break;
	case 200:
		setAction(&_sequenceManager1, this, 400, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(180, 100));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene400::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene400::signal() {
	switch (_sceneMode) {
	case 400:
	case 403:
		R2_GLOBALS._player.enableControl();
		break;
	case 401:
		R2_GLOBALS._sceneManager.changeScene(200);
		break;
	case 402:
		R2_GLOBALS._sceneManager.changeScene(125);
		break;
	case 404:
		// Getting the sensor probe
		R2_INVENTORY.setObjectScene(R2_SENSOR_PROBE, 1);
		_sensorProbe.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 405:
		// Getting the reader
		R2_INVENTORY.setObjectScene(R2_READER, 1);
		_reader.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 406:
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 1);
		_attractorUnit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	default:
		break;
	}
}

void Scene400::dispatch() {
	SceneExt::dispatch();

	switch (R2_GLOBALS._player.getRegionIndex() - 15) {
	case 0:
	case 11:
	case 12:
		R2_GLOBALS._player._shade = 2;
		break;
	case 9:
		R2_GLOBALS._player._shade = 0;
		break;
	case 10:
		R2_GLOBALS._player._shade = 1;
		break;
	case 13:
		R2_GLOBALS._player._shade = 3;
		break;
	case 14:
		R2_GLOBALS._player._shade = 4;
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 800 - Sick Bay
 *
 *--------------------------------------------------------------------------*/

bool Scene800::Button::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE) {
		return NamedHotspot::startAction(action, event);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 802;
		scene->setAction(&scene->_sequenceManager1, scene, 802, &R2_GLOBALS._player, &scene->_autodocCover, NULL);
		return true;
	}
}

bool Scene800::CableJunction::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	if (action != R2_OPTICAL_FIBRE) {
		return NamedHotspot::startAction(action, event);
	} else {
		R2_GLOBALS._player.disableControl();
		scene->_opticalFibre.postInit();
		scene->_sceneMode = 803;

		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 813, &R2_GLOBALS._player, &scene->_opticalFibre, &scene->_reader, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 803, &R2_GLOBALS._player, &scene->_opticalFibre, NULL);

		return true;
	}
}

bool Scene800::DeviceSlot::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(R2_READER) != 800)
			break;

		R2_GLOBALS._player.disableControl();
		scene->_reader.postInit();

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 814, &R2_GLOBALS._player, &scene->_reader, &scene->_opticalFibre, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 804, &R2_GLOBALS._player, &scene->_reader, NULL);
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene800::Door::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 801;
		scene->setAction(&scene->_sequenceManager1, scene, 801, &R2_GLOBALS._player, &scene->_door, NULL);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Tray::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (!R2_GLOBALS.getFlag(10)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 806;
			scene->setAction(&scene->_sequenceManager1, scene, 806, &R2_GLOBALS._player, &scene->_tray, NULL);
		} else if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 825) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 808;
			scene->setAction(&scene->_sequenceManager1, scene, 808, &R2_GLOBALS._player, &scene->_tray, NULL);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 807;
			scene->setAction(&scene->_sequenceManager1, scene, 807, &R2_GLOBALS._player, &scene->_tray, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::ComScanner::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_cabinet._frame == 1)
			return false;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 811;
		scene->setAction(&scene->_sequenceManager1, scene, 811, &R2_GLOBALS._player, &scene->_comScanner, NULL);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(800, 35);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Cabinet::startAction(CursorType action, Event &event) {
	Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl();

		if (scene->_cabinet._frame == 1) {
			scene->_sceneMode = 810;
			scene->setAction(&scene->_sequenceManager1, scene, 810, &R2_GLOBALS._player, &scene->_cabinet, NULL);
			R2_GLOBALS.setFlag(56);
		} else {
			scene->_sceneMode = 812;
			scene->setAction(&scene->_sequenceManager1, scene, 812, &R2_GLOBALS._player, &scene->_cabinet, NULL);
			R2_GLOBALS.clearFlag(56);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene800::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(800);

	_door.postInit();
	_door.setVisage(800);
	_door.setPosition(Common::Point(286, 108));
	_door.fixPriority(50);
	_door.setDetails(800, 3, -1, -1, 1, NULL);

	_autodocCover.postInit();
	_autodocCover.setup(800, 2, 1);
	_autodocCover.setPosition(Common::Point(119, 161));
	_autodocCover.setDetails(800, 6, 7, -1, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800) {
		_opticalFibre.postInit();
		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			_opticalFibre.setup(800, 4, 1);
		else
			_opticalFibre.setup(800, 7, 2);

		_opticalFibre.setPosition(Common::Point(220, 124));
		_opticalFibre.fixPriority(140);
	}

	if (R2_INVENTORY.getObjectScene(R2_READER) == 800) {
		_reader.postInit();
		
		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 800) {
			_opticalFibre.setup(800, 4, 1);
			_reader.hide();
		} else {
			_reader.setup(800, 7, 1);
		}

		_reader.setPosition(Common::Point(230, 120));
		_reader.fixPriority(140);
	}

	_cabinet.postInit();
	_cabinet.setup(801, 1, R2_GLOBALS.getFlag(56) ? 6 : 1);
	_cabinet.setPosition(Common::Point(169, 79));
	_cabinet.setDetails(800, 41, -1, -1, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_9) == 800) {
		_comScanner.postInit();
		_comScanner.setup(801, 2, 1);
		_comScanner.setPosition(Common::Point(174, 73));
		_comScanner.setDetails(800, 34, 35, -1, 1, NULL);
	}

	_tray.postInit();
	_tray.setup(800, R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 825 ? 6 : 5, 1);
	if (R2_GLOBALS.getFlag(10))
		_tray.setFrame(5);
	_tray.setPosition(Common::Point(203, 144));
	_tray.setDetails(800, 12, -1, 14, 1, NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_dataConduits.setDetails(13, 800, 21, -1, -1);
	_cableJunction.setDetails(Rect(206, 111, 223, 125), 800, 24, -1, -1, 1, NULL);
	_deviceSlot.setDetails(Rect(220, 108, 239, 122), 800, 27, -1, -1, 1, NULL);
	_diskSlot.setDetails(Rect(209, 124, 226, 133), 800, 9, -1, 11, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
		_deviceSlot._lookLineNum = 33;

	_button.setDetails(Rect(189, 112, 204, 124), 800, 30, -1, -1, 1, NULL);
	_couch.setDetails(11, 800, 15, -1, 17);
	_autoDoc.setDetails(Rect(152, 92, 247, 151), 800, 6, 7, -1, 1, NULL);
	_medicalDatabase.setDetails(12, 800, 18, -1, -1);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 800, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 825:
		_sceneMode = 800;
		setAction(&_sequenceManager1, this, 805, &R2_GLOBALS._player, &_autodocCover, NULL);
		break;
	case 850:
		_sceneMode = 800;
		setAction(&_sequenceManager1, this, 800, &R2_GLOBALS._player, &_door, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(277, 132));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene800::signal() {
	switch (_sceneMode) {
	case 801:
		R2_GLOBALS._sceneManager.changeScene(850);
		break;
	case 802:
		R2_GLOBALS._sceneManager.changeScene(825);
		break;
	case 803:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBRE, 800);
		break;
	case 804:
		R2_GLOBALS._player.enableControl();
		_deviceSlot._lookLineNum = 33;
		R2_INVENTORY.setObjectScene(R2_READER, 800);
		break;
	case 806:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS.setFlag(10);
		break;
	case 807:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS.clearFlag(10);
		break;
	case 808:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 1);
		break;
	case 809:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_READER, 1);
		break;
	case 811:
		R2_GLOBALS._player.enableControl();
		_comScanner.remove();
		R2_INVENTORY.setObjectScene(R2_9, 1);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 825 - Autodoc
 *
 *--------------------------------------------------------------------------*/

Scene825::Button::Button(): SceneObject() {
	_buttonId = 0;
	_v2 = 0;
	_buttonDown = false;
}

void Scene825::Button::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_v2);
	s.syncAsSint16LE(_buttonDown);
}

void Scene825::Button::process(Event &event) {
	Scene825 *scene = (Scene825 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled) {
		if ((event.eventType == EVENT_BUTTON_DOWN) && _bounds.contains(event.mousePos) && !_buttonDown) {
			scene->_sound1.play(14);
			setFrame(2);
			_buttonDown = true;
			event.handled = true;
		}

		if ((event.eventType == EVENT_BUTTON_UP) && _buttonDown) {
			setFrame(1);
			_buttonDown = false;
			event.handled = true;

			scene->doButtonPress(_buttonId);
		}
	}
}

bool Scene825::Button::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;
	else
		return SceneObject::startAction(action, event);
}

void Scene825::Button::setButton(int buttonId) {
	SceneObject::postInit();
	_v2 = buttonId;
	_buttonDown = 0;
	_sceneText._color1 = 92;
	_sceneText._color2 = 0;
	_sceneText._width = 200;
	_sceneText.fixPriority(20);
	_sceneText._fontNumber = 50;

	switch (buttonId) {
	case 1:
		_sceneText.setPosition(Common::Point(95, 58));
		break;
	case 2:
		_sceneText.setPosition(Common::Point(98, 75));
		break;
	case 3:
		_sceneText.setPosition(Common::Point(102, 95));
		break;
	case 4:
		_sceneText.setPosition(Common::Point(180, 58));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	case 5:
		_sceneText.setPosition(Common::Point(177, 75));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	case 6:
		_sceneText.setPosition(Common::Point(175, 95));
		_sceneText._textMode = ALIGN_RIGHT;
		break;
	default:
		break;
	}

	setDetails(825, 6, 7, -1, 2, NULL);
}

void Scene825::Button::setText(int textId) {
	_buttonId = textId;
	_lookLineNum = textId;

	_sceneText.remove();
	if (_buttonId != 0)
		_sceneText.setup(AUTODOC_ITEMS[textId - 1]);
}

/*--------------------------------------------------------------------------*/

Scene825::Scene825(): SceneExt() {
	_menuId = _frame1 = _frame2 = 0;
}

void Scene825::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(825);
	R2_GLOBALS._player._uiEnabled = false;
	BF_GLOBALS._interfaceY = 200;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._effect = 0;
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_item2.setDetails(1, 825, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 825, 0, -1, -1, 1, NULL);

	_sceneMode = 10;
	signal();
}

void Scene825::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_menuId);
	s.syncAsSint16LE(_frame1);
	s.syncAsSint16LE(_frame2);
}

void Scene825::remove() {
	SceneExt::remove();
	R2_GLOBALS._player._uiEnabled = true;
}

void Scene825::signal() {
	switch (_sceneMode) {
	case 10:
		_button1.setButton(1);
		_button1.setup(825, 1, 1);
		_button1.setPosition(Common::Point(71, 71));
		_button2.setButton(2);
		_button2.setup(825, 3, 1);
		_button2.setPosition(Common::Point(74, 90));
		_button3.setButton(3);
		_button3.setup(825, 5, 1);
		_button3.setPosition(Common::Point(78, 109));
		_button4.setButton(4);
		_button4.setup(825, 2, 1);
		_button4.setPosition(Common::Point(248, 71));
		_button5.setButton(5);
		_button5.setup(825, 4, 1);
		_button5.setPosition(Common::Point(245, 90));
		_button6.setButton(6);
		_button6.setup(825, 6, 1);
		_button6.setPosition(Common::Point(241, 109));

		doButtonPress(1);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 825:
		_object5.remove();
		_sceneText._color1 = 92;
		_sceneText._color2 = 0;
		_sceneText._width = 200;
		_sceneText.fixPriority(20);
		_sceneText._fontNumber = 50;
		_sceneText.setPosition(Common::Point(120, 75));
		_sceneText.setup(NO_MALADY_DETECTED);
		_sceneMode = 826;
		setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
		break;
	case 826:
		_sceneText.remove();
		doButtonPress(1);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 827:
		_object5.remove();
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 825);
		_sceneText.setPosition(Common::Point(108, 75));
		_sceneText.setup(FOREIGN_OBJECT_EXTRACTED);
		_sceneMode = 826;
		setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene825::process(Event &event) {
	SceneExt::process(event);

	if (R2_GLOBALS._player._uiEnabled) {
		_button1.process(event);
		_button2.process(event);
		_button3.process(event);
		_button4.process(event);
		_button5.process(event);
		_button6.process(event);
	}
}

void Scene825::dispatch() {
	if (R2_GLOBALS._sceneObjects->contains(&_object4) && 
			((_object4._frame == 1) || (_object4._frame == 3)) &&
			(_object4._frame != _frame1)) {
		_sound2.play(25);
	}

	if (R2_GLOBALS._sceneObjects->contains(&_object1) &&
			(_object1._frame == 3) && (_object1._frame != _frame2)) {
		_sound3.play(26);
	}

	_frame1 = _object4._frame;
	_frame2 = _object1._frame;

	Scene::dispatch();
}

void Scene825::doButtonPress(int buttonId) {
	if ((_menuId != 4) || (buttonId == 5)) {
		_button1.setText(0);
		_button2.setText(0);
		_button3.setText(0);
		_button4.setText(0);
		_button5.setText(0);
		_button6.setText(0);

		switch (buttonId) {
		case 2:
			R2_GLOBALS._player.disableControl();
			_object5.postInit();
			_sceneMode = 825;
			setAction(&_sequenceManager1, this, 825, &R2_GLOBALS._player, &_object5, NULL);
			break;
		case 3:
			R2_GLOBALS._player.disableControl();
			_sceneText._color1 = 92;
			_sceneText._color2 = 0;
			_sceneText._width = 200;
			_sceneText.fixPriority(20);
			_sceneText._fontNumber = 50;
			_sceneText.setPosition(Common::Point(115, 75));

			if (R2_GLOBALS.getFlag(4)) {
				if ((R2_INVENTORY.getObjectScene(R2_READER) != 800) ||
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 800)) {
					_sceneText.setPosition(Common::Point(116, 75));
					_sceneText.setup(ACCESS_CODE_REQUIRED);
				} else if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != 800) {
					_sceneText.setPosition(Common::Point(115, 75));
					_sceneText.setup(NO_TREATMENT_REQUIRED);
				} else {
					_button6._buttonId = 5;
					
					_object5.postInit();
					setAction(&_sequenceManager1, this, 827, &_object5, NULL);
				}
			} else {
				R2_GLOBALS.setFlag(2);

				if ((R2_INVENTORY.getObjectScene(R2_READER) != 800) ||
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 800)) {
					_sceneText.setPosition(Common::Point(116, 75));
					_sceneText.setup(ACCESS_CODE_REQUIRED);
				} else {
					_sceneText.setPosition(Common::Point(119, 75));
					_sceneText.setup(INVALID_ACCESS_CODE);
				}
			}

			if (_sceneMode != 827) {
				_sceneMode = 826;
				setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
			}
			break;
		case 4:
			_sound4.play(27);
			_button6._buttonId = 5;
			
			_object1.postInit();
			_object1.setup(826, 7, 1);
			_object1.setPosition(Common::Point(112, 67));
			_object1._numFrames = 1;
			_object1.animate(ANIM_MODE_2);

			_object2.postInit();
			_object2.setup(826, 5, 1);
			_object2.setPosition(Common::Point(158, 67));
			_object2._numFrames = 5;
			_object2.animate(ANIM_MODE_2);

			_object3.postInit();
			_object3.setup(826, 6, 1);
			_object3.setPosition(Common::Point(206, 67));
			_object3._numFrames = 1;
			_object3.animate(ANIM_MODE_2);

			_object4.postInit();
			_object4.setup(826, 8, 1);
			_object4.setPosition(Common::Point(158, 84));
			_object4._numFrames = 1;
			_object4.animate(ANIM_MODE_2);

			_object5.postInit();
			_object5.setup(826, 4, 1);
			_object5.setPosition(Common::Point(161, 110));
			break;
		case 5:
			R2_GLOBALS._player.disableControl();
			if (_menuId == 4) {
				_menuId = 0;

				_object1.remove();
				_object2.remove();
				_object3.remove();
				_object4.remove();
				_object5.remove();

				_sound2.stop();
				_sound3.stop();
				_sound4.stop();

				doButtonPress(1);
				R2_GLOBALS._player.enableControl();
				R2_GLOBALS._player._canWalk = false;
			} else {
				R2_GLOBALS._sceneManager.changeScene(800);
			}
			break;
		case 6:
			R2_GLOBALS._player.disableControl();
			_sceneText._color1 = 92;
			_sceneText._color2 = 0;
			_sceneText._width = 200;
			_sceneText.fixPriority(20);
			_sceneText._fontNumber = 50;
			_sceneText.setPosition(Common::Point(115, 75));
			_sceneText.setup(NO_TREATMENT_REQUIRED);

			_sceneMode = 826;
			setAction(&_sequenceManager1, this, 826, &R2_GLOBALS._player, NULL);
			break;
		default:
			_button1.setText(2);
			_button2.setText(3);
			_button3.setText(4);
			_button4.setText(6);
			_button6.setText(5);
			break;
		}

		_menuId = buttonId;
	}
}

/*--------------------------------------------------------------------------
 * Scene 850 - Deck #5 - By Lift
 *
 *--------------------------------------------------------------------------*/

bool Scene850::Indicator::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) != 850))
		return NamedHotspot::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 851;
		scene->setAction(&scene->_sequenceManager1, scene, 851, &R2_GLOBALS._player, &scene->_fibre, NULL);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene850::LiftDoor::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager1, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::SickBayDoor::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 204;
		scene->setAction(&scene->_sequenceManager1, scene, 204, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::Clamp::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if (!R2_GLOBALS.getFlag(7))
		return false;
	else if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_object1.postInit();
		scene->_sceneMode = 850;
		scene->setAction(&scene->_sequenceManager1, scene, 850, &R2_GLOBALS._player, this, &scene->_object1, NULL);
		return true;
	}
}

bool Scene850::Panel::startAction(CursorType action, Event &event) {
	Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

	if ((action != CURSOR_USE) || R2_GLOBALS.getFlag(7))
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 852;
		scene->setAction(&scene->_sequenceManager1, scene, 852, &R2_GLOBALS._player, this, &scene->_object1, NULL);
		return true;
	}		
}

/*--------------------------------------------------------------------------*/

void Scene850::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(850);

	_liftDoor.postInit();
	_liftDoor.setup(850, 2, 1);
	_liftDoor.setPosition(Common::Point(188, 79));
	_liftDoor.setDetails(850, 3, -1, -1, 1, NULL);

	_sickBayDoor.postInit();
	_sickBayDoor.setup(850, 3, 1);
	_sickBayDoor.setPosition(Common::Point(62, 84));
	_sickBayDoor.setDetails(850, 9, -1, -1, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_CLAMP) == 850) {
		_clamp.postInit();
		_clamp.setup(850, 5, 1);
		_clamp.setPosition(Common::Point(242, 93));
		_clamp.fixPriority(81);
		_clamp.animate(ANIM_MODE_2, NULL);
		_clamp.setDetails(850, 27, -1, -1, 1, NULL);
	}

	_panel.postInit();
	_panel.setVisage(850);

	if (R2_GLOBALS.getFlag(7))
		_panel.setFrame(7);

	_panel.setPosition(Common::Point(232, 119));
	_panel.fixPriority(82);
	_panel.setDetails(850, 24, -1, -1, 1, NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBRE) == 850) {
		_fibre.postInit();
		_fibre.setup(850, 6, 1);
		_fibre.setPosition(Common::Point(280, 87));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_eastDoor.setDetails(Rect(289, 53, 315, 125), 850, 6, -1, 8, 1, NULL);
	_indicator.setDetails(Rect(275, 67, 286, 79), 850, 18, -1, 20, 1, NULL);
	_sickBayIndicator.setDetails(Rect(41, 51, 48, 61), 850, 15, -1, -1, 1, NULL);
	_liftControls.setDetails(Rect(156, 32, 166, 44), 850, 21, -1, -1, 1, NULL);
	_compartment.setDetails(Rect(4, 88, 153, 167), 850, 12, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 850, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 250:
		_sceneMode = 203;
		setAction(&_sequenceManager1, this, 203, &R2_GLOBALS._player, &_liftDoor, NULL);
		break;
	case 800:
		_sceneMode = 205;
		setAction(&_sequenceManager1, this, 205, &R2_GLOBALS._player, &_sickBayDoor, NULL);
		break;
	default:
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(215, 115));
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene850::signal() {
	switch (_sceneMode) {
	case 202:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 204:
		R2_GLOBALS._sceneManager.changeScene(800);
		break;
	case 850:
		R2_INVENTORY.setObjectScene(R2_CLAMP, 1);
		_clamp.remove();
		_object1.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 851:
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBRE, 1);
		_fibre.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 852:
		R2_GLOBALS.setFlag(7);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
