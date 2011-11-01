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

namespace TsAGE {

namespace Ringworld2 {

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
			scene->_object10.setFrame(1);
		} else {
			SceneItem::display2(100, 27);
			_state = 1;
			scene->_object10.setFrame(2);
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

bool Scene100::Object10::startAction(CursorType action, Event &event) {
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

	_object10.postInit();
	_object10.setup(100, 2, 1);
	_object10.setDetails(100, -1, -1, -1, 1, NULL);

	_table.postInit();
	_table.setup(100, 2, 3);
	_table.setPosition(Common::Point(175, 157));
	_table.setDetails(100, 17, 18, 20, 1, NULL);

	_object1.postInit();
	_object1.setup(100, 3, 1);
	_object1.setPosition(Common::Point(89, 79));
	_object1.fixPriority(250);
	_object1.animate(ANIM_MODE_2, NULL);
	_object1._numFrames = 3;

	_object2.postInit();
	_object2.setup(100, 3, 1);
	_object2.setPosition(Common::Point(89, 147));
	_object2.fixPriority(250);
	_object2.animate(ANIM_MODE_7, 0, NULL); 
	_object2._numFrames = 3;

	_object6.postInit();
	_object6.setVisage(101);
	_object6.setPosition(Common::Point(231, 126));
	_object6.fixPriority(10);
	_object6.setDetails(100, 37, -1, 39, 1, NULL);

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
		setAction(&_sequenceManager1, this, 104, &R2_GLOBALS._player, &_object6, &_object4, &_object5, NULL);
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
			_object10.setFrame(1);
		} else {
			_door._state = 1;
			_object10.setFrame(2);
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
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldSceneNumber) {
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
		R2_GLOBALS._player._oldSceneNumber = R2_GLOBALS._sceneManager._previousScene;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldSceneNumber) {
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
		if (!_soundCount || (R2_GLOBALS._v57C2C == 2)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex++], this);
		}
		break;
	case 125:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, R2_GLOBALS._player._oldSceneNumber);
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
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._oldSceneNumber);
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
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != R2_GLOBALS._player._oldSceneNumber) {
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

		if ((_soundCount > 0) && (R2_GLOBALS._v57C2C != 2)) {
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

} // End of namespace Ringworld2

} // End of namespace TsAGE
