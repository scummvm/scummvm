/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_dialogs.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_speakers.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 50 - Waking up cutscene
 *
 *--------------------------------------------------------------------------*/

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
	loadScene(110);
	SceneExt::postInit(OwnerList);

	R2_GLOBALS._uiElements._active = false;
	R2_GLOBALS._scenePalette.loadPalette(0);

	R2_GLOBALS._sound2.play(10);
	R2_GLOBALS._player.disableControl();

	setAction(&_action1);
}

void Scene50::process(Event &event) {
	if ((event.eventType != EVENT_BUTTON_DOWN) && (event.eventType == EVENT_CUSTOM_ACTIONSTART)
			&& (event.eventType != EVENT_KEYPRESS) && (event.eventType == EVENT_UNK27)) {
		event.handled = true;
		g_globals->_events.setCursor(CURSOR_ARROW);
		HelpDialog::show();
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
			scene->_tableLocker.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, (SceneItem *)NULL);
			}

			// Open table locker
			scene->setAction(&scene->_sequenceManager2, scene, 108, this, &scene->_tableLocker,
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 109;
			// Close table locker
			scene->setAction(&scene->_sequenceManager2, scene, 109, this, &scene->_tableLocker,
				&scene->_stasisNegator, &R2_GLOBALS._player, NULL);
		}
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();

		if (_strip == 2) {
			SceneItem::display2(100, 18);
			scene->_sceneMode = 102;
			scene->_tableLocker.postInit();
			scene->_stasisNegator.postInit();

			if (R2_INVENTORY.getObjectScene(R2_NEGATOR_GUN) == 1) {
				scene->_stasisNegator.setup(100, 7, 2);
			} else {
				scene->_stasisNegator.setup(100, 7, 1);
				scene->_stasisNegator.setDetails(100, 21, 22, 23, 2, (SceneItem *)NULL);
			}

			scene->setAction(&scene->_sequenceManager2, scene, 102, this, &scene->_tableLocker,
				&scene->_stasisNegator, NULL);
		} else {
			SceneItem::display2(100, 19);
			scene->_sceneMode = 103;
			scene->setAction(&scene->_sequenceManager2, scene, 103, this, &scene->_tableLocker,
				&scene->_stasisNegator, NULL);
		}
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::StasisNegator::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 107;
		scene->setAction(&scene->_sequenceManager1, scene, 107, &R2_GLOBALS._player, &scene->_stasisNegator, NULL);
		return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::DoorDisplay::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(100, _state ? 24 : 25);
		return true;
	case CURSOR_TALK:
		SceneItem::display2(100, _state ? 26 : 27);
		return true;
	case CURSOR_USE: {
		Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 110;
		scene->setAction(&scene->_sequenceManager1, scene, 110, &R2_GLOBALS._player, NULL);
		return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene100::SteppingDisks::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 111;
		scene->setAction(&scene->_sequenceManager1, scene, 111, &R2_GLOBALS._player, this, NULL);
		return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene100::Terminal::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene100 *scene = (Scene100 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 105;
		scene->setAction(&scene->_sequenceManager1, scene, 105, &R2_GLOBALS._player, this, NULL);
		return true;
		}
	default:
		return NamedHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene100::postInit(SceneObjectList *OwnerList) {
	loadScene(100);
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene != 125)
		R2_GLOBALS._sound1.play(10);

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, 4, 5, 1, (SceneItem *)NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, (SceneItem *)NULL);

	_table.postInit();
	_table.setup(100, 2, 3);
	_table.setPosition(Common::Point(175, 157));
	_table.setDetails(100, 17, 18, 20, 1, (SceneItem *)NULL);

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
	_wardrobe.setDetails(100, 37, -1, 39, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_STEPPING_DISKS) == 100) {
		_steppingDisks.postInit();
		_steppingDisks.setup(100, 8, 1);
		_steppingDisks.setPosition(Common::Point(274, 130));
		_steppingDisks.setDetails(100, 40, -1, 42, 1, (SceneItem *)NULL);
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
		_wardrobeColorAnim.postInit();
		_wardrobeTopAnim.postInit();
		_sceneMode = 104;
		setAction(&_sequenceManager1, this, 104, &R2_GLOBALS._player, &_wardrobe, &_wardrobeTopAnim, &_wardrobeColorAnim, NULL);
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

		_tableLocker.remove();
		_stasisNegator.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 104:
		_sceneMode = 0;
		_wardrobeColorAnim.remove();
		_wardrobeTopAnim.remove();

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
		_table.setAction(&_sequenceManager2, NULL, 103, &_table, &_tableLocker, &_stasisNegator, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 125 - Computer Terminal
 *
 *--------------------------------------------------------------------------*/

bool Scene125::Food::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return true;

	return SceneActor::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

Scene125::Icon::Icon(): SceneActor()  {
	_lookLineNum = 0;
	_iconId = 0;
	_pressed = false;
}

void Scene125::Icon::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_glyph.postInit();
	_glyph.fixPriority(255);
	_glyph.hide();

	_sceneText1._color1 = 92;
	_sceneText1._color2 = 0;
	_sceneText1._width = 200;
	_sceneText2._color1 = 0;
	_sceneText2._color2 = 0;
	_sceneText2._width = 200;
	setDetails(125, -1, -1, -1, 2, (SceneItem *)NULL);
}

void Scene125::Icon::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_iconId);
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

					switch (_glyph._strip) {
					case 1:
						_glyph.setStrip(2);
						break;
					case 3:
						_glyph.setStrip(4);
						break;
					case 5:
						_glyph.setStrip(6);
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

			switch (_glyph._strip) {
			case 2:
				_glyph.setStrip(1);
				break;
			case 4:
				_glyph.setStrip(3);
				break;
			case 6:
				_glyph.setStrip(5);
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
	_lookLineNum = _iconId = id;
	SceneActor::_lookLineNum = id;

	_sceneText1.remove();
	_sceneText2.remove();

	if (_lookLineNum) {
		Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;

		showIcon();
		_glyph.setup(161, ((id - 1) / 10) * 2 + 1, ((id - 1) % 10) + 1);
		_glyph.setPosition(_position);

		_sceneText1._fontNumber = scene->_iconFontNumber;
		_sceneText1.setup(CONSOLE125_MESSAGES[id]);
		_sceneText1.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE125_MESSAGES[id]);
		_sceneText2.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE125_MESSAGES[id]);
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
	_glyph.show();
	_horizLine.show();
	show();
}

void Scene125::Icon::hideIcon() {
	_sceneText1.hide();
	_sceneText2.hide();
	_glyph.hide();
	_horizLine.hide();
	hide();
}

/*--------------------------------------------------------------------------*/

bool Scene125::DiskSlot::startAction(CursorType action, Event &event) {
	Scene125 *scene = (Scene125 *)R2_GLOBALS._sceneManager._scene;
	switch (action) {
	case CURSOR_USE:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[R2_QUINN]) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 126;
			scene->setAction(&scene->_sequenceManager, scene, 126, &scene->_infoDisk, NULL);
			return true;
		}
		break;
	case R2_OPTO_DISK:
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 1) {
			R2_GLOBALS._player.disableControl();
			scene->_infoDisk.postInit();
			scene->_sceneMode = 125;
			scene->setAction(&scene->_sequenceManager, scene, 125, &scene->_infoDisk, NULL);
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
	loadScene(160);
	SceneExt::postInit();
	_palette.loadPalette(0);

	if ((R2_GLOBALS._sceneManager._previousScene != 125) && (R2_GLOBALS._sceneManager._previousScene != 1337) && (R2_GLOBALS._sceneManager._previousScene != 1330))
		// Save the prior scene to return to when the console is turned off
		R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = R2_GLOBALS._sceneManager._previousScene;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == R2_GLOBALS._player._oldCharacterScene[R2_QUINN]) {
		_infoDisk.postInit();
		_infoDisk.setup(160, 3, 5);
		_infoDisk.setPosition(Common::Point(47, 167));
	}

	_foodDispenser.postInit();
	_foodDispenser.setup(162, 1, 1);
	_foodDispenser.setPosition(Common::Point(214, 168));

	_diskSlot.setDetails(Rect(27, 145, 81, 159), 126, 9, -1, -1, 1, NULL);
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

		setAction(&_sequenceManager, this, 127, &_icon1, &_icon2, &_icon3, &_icon4, &R2_GLOBALS._player, NULL);
		_sceneMode = 2;
		break;
	case 2:
		_icon1.setup(160, 1, 1);
		_icon1.setPosition(Common::Point(65, 17));
		_icon1._horizLine.postInit();
		_icon1._horizLine.setup(160, 7, 1);
		_icon1._horizLine.setPosition(Common::Point(106, 41));

		_icon2.setup(160, 1, 1);
		_icon2.setPosition(Common::Point(80, 32));
		_icon2._horizLine.postInit();
		_icon2._horizLine.setup(160, 7, 2);
		_icon2._horizLine.setPosition(Common::Point(106, 56));

		_icon3.setup(160, 1, 1);
		_icon3.setPosition(Common::Point(65, 47));
		_icon3._horizLine.postInit();
		_icon3._horizLine.setup(160, 7, 1);
		_icon3._horizLine.setPosition(Common::Point(106, 71));

		_icon4.setup(160, 1, 1);
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4._horizLine.postInit();
		_icon4._horizLine.setup(160, 7, 2);
		_icon4._horizLine.setPosition(Common::Point(106, 86));

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
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 10:
		switch (_consoleMode) {
		case 12:
			_sceneMode = 129;

			_starchart1.postInit();
			_starchart2.postInit();
			_starchart3.postInit();

			if (R2_GLOBALS.getFlag(13)) {
				// Show starchart with Ringworld present
				_starchart4.postInit();
				setAction(&_sequenceManager, this, 130, &R2_GLOBALS._player, &_starchart1, &_starchart2,
					&_starchart3, &_starchart4, NULL);
			} else {
				// Show starchart without Ringworld
				setAction(&_sequenceManager, this, 129, &R2_GLOBALS._player, &_starchart1, &_starchart2,
					&_starchart3, NULL);
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
		if (!_soundCount || !(R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 12;
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex++], this);
		}
		break;
	case 125:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, R2_GLOBALS._player._oldCharacterScene[R2_QUINN]);
		R2_GLOBALS._player.enableControl();
		break;
	case 126:
		R2_INVENTORY.setObjectScene(R2_OPTO_DISK, 1);
		_infoDisk.remove();
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
		R2_GLOBALS._playStream.dispatch();

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
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._oldCharacterScene[R2_QUINN]);
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
		_icon4._horizLine.hide();

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
		if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != R2_GLOBALS._player._oldCharacterScene[R2_QUINN]) {
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
			_icon4._horizLine.hide();

			_icon6.setIcon(26);
			_sceneMode = 10;

			_palette.loadPalette(161);
			R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		}
		break;
	case 15:
		consoleAction(3);

		if (R2_GLOBALS._foodCount < 3) {
			R2_GLOBALS._player.disableControl();
			_food.postInit();
			_food.setup(162, 2, 2);
			_food.setPosition(Common::Point(216, UI_INTERFACE_Y));

			R2_GLOBALS._foodCount += 2;

			_sceneMode = 128;
			this->setAction(&_sequenceManager, this, 128, &_foodDispenser, &_food, NULL);
		} else if (R2_GLOBALS._foodCount == 3) {
			SceneItem::display2(126, 13);
		} else {
			SceneItem::display2(126, 14);
		}
		break;
	case 16:
		consoleAction(3);

		if (R2_GLOBALS._foodCount < 4) {
			R2_GLOBALS._player.disableControl();
			_food.postInit();
			_food.setup(162, 2, 3);
			_food.setPosition(Common::Point(218, UI_INTERFACE_Y));

			++R2_GLOBALS._foodCount;

			_sceneMode = 128;
			this->setAction(&_sequenceManager, this, 128, &_foodDispenser, &_food, NULL);
		} else {
			SceneItem::display2(126, 15);
		}
		break;
	case 17:
		consoleAction(3);

		if (R2_GLOBALS._foodCount < 4) {
			R2_GLOBALS._player.disableControl();
			_food.postInit();
			_food.setup(162, 2, 1);
			_food.setPosition(Common::Point(215, UI_INTERFACE_Y));

			++R2_GLOBALS._foodCount;

			_sceneMode = 128;
			this->setAction(&_sequenceManager, this, 128, &_foodDispenser, &_food, NULL);
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
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 5, this);
		break;
	case 24:
		_icon4.setIcon(25);
		_icon4._horizLine.hide();

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
		_icon4._horizLine.hide();

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
		_starchart1.hide();
		_starchart2.hide();
		_starchart3.hide();
		_starchart4.hide();

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
		_icon4._horizLine.hide();

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
			R2_GLOBALS._playStream.play(_soundIndexes[_soundIndex++], this);
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
	loadScene(100);
	SceneExt::postInit();

	_door.postInit();
	_door._state = 0;
	_door.setVisage(100);
	_door.setPosition(Common::Point(160, 84));
	_door.setDetails(100, 3, -1, -1, 1, (SceneItem *)NULL);

	_doorDisplay.postInit();
	_doorDisplay.setup(100, 2, 1);
	_doorDisplay.setPosition(Common::Point(202, 53));
	_doorDisplay.setDetails(100, -1, -1, -1, 1, (SceneItem *)NULL);

	_emptyRoomTable.postInit();
	_emptyRoomTable.setVisage(100);
	_emptyRoomTable.setStrip(4);
	_emptyRoomTable.setFrame(1);
	_emptyRoomTable.setPosition(Common::Point(175, 157));
	_emptyRoomTable.setDetails(150, 3, 4, 5, 1, (SceneItem *)NULL);

	_wardrobe.postInit();
	_wardrobe.setVisage(101);
	_wardrobe.setPosition(Common::Point(231, 126));
	_wardrobe.fixPriority(10);
	_wardrobe.setDetails(100, 37, -1, 39, 1, (SceneItem *)NULL);

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
 * Scene 160 - Credits
 *
 *--------------------------------------------------------------------------*/

void Scene160::Action1::signal() {
	Scene160 *scene = (Scene160 *)R2_GLOBALS._sceneManager._scene;
	bool breakFlag;
	SynchronizedList<SceneText *>::iterator i;
	SceneText *topItem;

	switch (_actionIndex) {
	case 0:
		scene->_yChange = 1;
		scene->_lineNum = 0;
		++_actionIndex;
		// fall through

	case 1:
		setDelay(5);
		breakFlag = true;
		do {
			if (!scene->_lineNum || ((scene->_lineNum != -1) &&
					(((*scene->_creditsList.reverse_begin())->_position.y < 164) || !breakFlag))) {
				breakFlag = true;
				Common::String msg = g_resourceManager->getMessage(160, scene->_lineNum++);

				if (*msg.c_str() == '^') {
					scene->_lineNum = -1;
				} else {
					if (msg.size() == 0)
						msg = " ";

					SceneText *sceneText = new SceneText();
					sceneText->_fontNumber = 50;

					switch (*msg.c_str()) {
					case '$': {
						// Centered text
						msg.deleteChar(0);
						int width = R2_GLOBALS.gfxManager()._font.getStringWidth(msg.c_str());

						sceneText->_textMode = ALIGN_CENTER;
						sceneText->setPosition(Common::Point(160 - (width / 2), 175));
						sceneText->_width = 320;
						break;
					}

					case '%': {
						// Text for position name
						msg.deleteChar(0);
						int width = R2_GLOBALS.gfxManager()._font.getStringWidth(msg.c_str());

						sceneText->_textMode = ALIGN_RIGHT;
						sceneText->setPosition(Common::Point(151 - width, 175));
						sceneText->_width = 320;
						breakFlag = false;
						break;
					}

					case '@':
						// Text for who was in the position
						msg.deleteChar(0);
						sceneText->_textMode = ALIGN_LEFT;
						sceneText->_position = Common::Point(167, 175);
						sceneText->_width = 153;
						break;

					default:
						sceneText->_width = 151;
						sceneText->setPosition(Common::Point(151, 175));
						sceneText->_textMode = ALIGN_RIGHT;
						break;
					}

					sceneText->_color1 = 191;
					sceneText->_color2 = 191;
					sceneText->_color3 = 191;
					sceneText->setup(msg);
					sceneText->_flags |= OBJFLAG_CLONED;
					sceneText->fixPriority(5);

					scene->_creditsList.push_back(sceneText);
				}
			}

		} while (!breakFlag);

		// Move all the active credits
		for (i = scene->_creditsList.begin(); i != scene->_creditsList.end(); ++i) {
			SceneObject *item = *i;
			item->setPosition(Common::Point(item->_position.x, item->_position.y - scene->_yChange));
		}

		topItem = *scene->_creditsList.begin();
		if (topItem->_position.y < 25) {
			// Credit has reached the top, so remove it
			topItem->remove();
			scene->_creditsList.remove(topItem);

			if (scene->_creditsList.size() == 0) {
				// No more items left
				setDelay(10);
				++_actionIndex;
			}
		}
		break;

	case 2:
		HelpDialog::show();
		setDelay(4);
		break;

	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene160::Scene160(): SceneExt() {
	_frameNumber = _yChange = 0;
	_lineNum = 0;
}

void Scene160::postInit(SceneObjectList *OwnerList) {
	loadScene(4001);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	R2_GLOBALS._player._uiEnabled = false;
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;

	R2_GLOBALS._uiElements.hide();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_lineNum = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();

	_sound1.play(337);
	setAction(&_action1);
}

void Scene160::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_frameNumber);
	s.syncAsSint16LE(_yChange);
	s.syncAsSint16LE(_lineNum);

	_creditsList.synchronize(s);
}

void Scene160::remove() {
	// Clear the credit list
	SynchronizedList<SceneText *>::iterator i;
	for (i = _creditsList.begin(); i != _creditsList.end(); ++i) {
		SceneText *item = *i;

		item->remove();
	}
	_creditsList.clear();

	_sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene160::process(Event &event) {
	if ((event.eventType == EVENT_CUSTOM_ACTIONSTART) && (event.customType == kActionEscape)) {
		event.handled = true;
		HelpDialog::show();
	}

	if (!event.handled)
		SceneExt::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 180 - Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene180::Action1::signal() {
	Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
	case 1:
	case 2:
		scene->_shipDisplay.setStrip((_actionIndex == 1) ? 1 : 2);
		scene->_shipDisplay.setFrame(1);
		scene->_shipDisplay.animate(ANIM_MODE_5, this);
		break;
	case 4:
		scene->_shipDisplay.setStrip(3);
		scene->_shipDisplay.setFrame(1);
		scene->_shipDisplay.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene180::Scene180(): SceneExt() {
	_helpEnabled = false;
	_frameInc = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
	_fontNumber = R2_GLOBALS.gfxManager()._font._fontNumber;

	GfxFont font;
	font.setFontNumber(7);
	_fontHeight = font.getHeight() + 1;

	_sceneMode = (R2_GLOBALS._sceneManager._previousScene == 205) ? 10 : 0;
	_gameTextSpeaker._displayMode = 9;
}

void Scene180::postInit(SceneObjectList *OwnerList) {
	loadScene(9999);
	SceneExt::postInit();

	R2_GLOBALS._uiElements._active = true;
	R2_GLOBALS._player.disableControl();

	_stripManager.addSpeaker(&_gameTextSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_dutyOfficerSpeaker);

	signal();
}

void Scene180::remove() {
	_stripManager._currObj44Id = -1;
//	_stripManager._field2EA = -1;
	SceneExt::remove();

	R2_GLOBALS._scenePalette.loadPalette(0);
	R2_GLOBALS._scenePalette.setEntry(255, 255, 255, 255);

	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	// word_575F7 = 0;
	R2_GLOBALS._playStream.stop();
	R2_GLOBALS._sound2.fadeOut2(NULL);
	R2_GLOBALS._sound1.fadeOut2(NULL);
}

void Scene180::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_frameNumber);
	s.syncAsSint16LE(_helpEnabled);
	s.syncAsSint16LE(_frameInc);
	s.syncAsSint16LE(_fontNumber);
	s.syncAsSint16LE(_fontHeight);
}

void Scene180::signal() {
	R2_GLOBALS._playStream.stop();

	switch (_sceneMode++) {
	case 0:
		setSceneDelay(6);
		break;

	case 1:
		_helpEnabled = true;
		R2_GLOBALS._sceneManager._hasPalette = true;
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._isActive = true;
		_animationPlayer._objectMode = ANIMOBJMODE_1;
		R2_GLOBALS._scene180Mode = 1;

		_animationPlayer.load(1);
		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);

		R2_GLOBALS._sound1.play(1);
		break;

	case 2:
		R2_GLOBALS._scene180Mode = 1;
		R2_GLOBALS._paneRefreshFlag[0] = 3;

		if (R2_GLOBALS._sound1.isPlaying()) {
			setSceneDelay(1);
		} else {
			setSceneDelay(180);
		}
		break;

	case 3:
		R2_GLOBALS._scene180Mode = 1;

		if (R2_GLOBALS._sound1.isPlaying())
			_sceneMode = 3;

		setSceneDelay(1);
		break;

	case 4:
	case 8:
	case 30:
	case 43:
	case 47:
		_helpEnabled = false;
		R2_GLOBALS._screen.fillRect(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		_palette.loadPalette(0);
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 8, this);
		break;

	case 5:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._isActive = true;
		_animationPlayer._objectMode = ANIMOBJMODE_1;
		R2_GLOBALS._scene180Mode = 2;
		_animationPlayer.load(2);

		_helpEnabled = true;
		R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 6, NULL);
		R2_GLOBALS._sound1.play(2);
		break;

	case 6:
		R2_GLOBALS._scene180Mode = 2;
		R2_GLOBALS._paneRefreshFlag[0] = 3;

		if (R2_GLOBALS._sound1.isPlaying()) {
			setSceneDelay(1);
		} else {
			setSceneDelay(180);
		}
		break;

	case 7:
		// Title screen. Wait until title music finishes playing
		R2_GLOBALS._scene180Mode = 2;
		if (R2_GLOBALS._sound1.isPlaying())
			_sceneMode = 7;
		setSceneDelay(1);
		break;

	case 9:
		R2_GLOBALS._sound1.play(3);
		clearScreen();
		setSceneDelay(2);
		break;

	case 10:
		loadScene(4002);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._interfaceY = SCREEN_HEIGHT;
		setSceneDelay(6);
		break;

	case 11:
		_helpEnabled = true;
		_door.postInit();
		_shipDisplay.postInit();
		setAction(&_sequenceManager, this, 4000, &_door, &_shipDisplay, NULL);
		break;

	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
	case 22:
	case 24:
	case 26:
	case 46:
		setSceneDelay((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) ? 1 : 18);
		break;

	case 13:
		setAction(&_sequenceManager, this, 4001, &_door, &_shipDisplay, NULL);
		break;

	case 15:
		setAction(&_sequenceManager, this, 4002, &_door, &_shipDisplay, NULL);
		break;

	case 17:
		setAction(&_sequenceManager, this, 4003, &_door, &_shipDisplay, NULL);
		break;

	case 19:
		setAction(&_sequenceManager, this, 4004, &_door, &_shipDisplay, NULL);
		break;

	case 21:
		setAction(&_sequenceManager, this, 4005, &_door, &_shipDisplay, NULL);
		break;

	case 23:
		setAction(&_sequenceManager, this, 4006, &_door, &_shipDisplay, NULL);
		break;

	case 25:
		setAction(&_sequenceManager, this, 4007, &_door, &_shipDisplay, NULL);
		break;

	case 27:
		_helpEnabled = false;
		_door.remove();
		_shipDisplay.remove();
		setSceneDelay(2);
		break;

	case 28:
		_helpEnabled = false;
		_palette.loadPalette(0);
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 100, this);
		break;

	case 29:
		_helpEnabled = true;
		_animationPlayer._paletteMode = ANIMPALMODE_REPLACE_PALETTE;
		_animationPlayer._isActive = true;
		_animationPlayer._objectMode = ANIMOBJMODE_42;
		R2_GLOBALS._scene180Mode = 3;
		_animationPlayer.load(3);
		break;

	case 31:
		R2_GLOBALS._sound2.play(7);

		_door.postInit();
		_door.setVisage(76);
		_door.setStrip(1);
		_door.setFrame(1);
		_door.setPosition(Common::Point(288, 143));
		_door.fixPriority(210);

		loadScene(75);

		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._scenePalette.loadPalette(75);

		if (R2_GLOBALS._sceneManager._hasPalette)
			R2_GLOBALS._scenePalette.refresh();
		setSceneDelay(6);
		break;

	case 32:
		_helpEnabled = true;

		_teal.postInit();
		_teal.setPosition(Common::Point(161, 97));
		_teal.hide();

		_webbster.postInit();
		_webbster.setPosition(Common::Point(60, 96));
		_webbster.hide();
		_stripManager.start(11, this);
		break;

	case 33:
		_teal.hide();

		_webbster.setup(76, 4, 1);
		_webbster.setFrame(_webbster.getFrameCount());

		_shipDisplay.postInit();
		_shipDisplay.setup(75, 1, 1);
		_shipDisplay.setPosition(Common::Point(221, 125));
		_shipDisplay.fixPriority(210);
		_shipDisplay.setAction(&_action1);
		_stripManager.start(12, this);
		break;

	case 34:
		_teal.hide();
		_webbster.remove();

		_dutyOfficer.postInit();
		_dutyOfficer.setup(76, 2, 1);
		_dutyOfficer.setPosition(Common::Point(287, 135));
		_dutyOfficer.fixPriority(200);

		_sound1.play(19);
		_door.animate(ANIM_MODE_5, this);
		break;

	case 35:
		_stripManager.start(13, this);
		break;

	case 36:
		_teal.remove();
		_sound1.play(19);
		_door.animate(ANIM_MODE_6, this);
		break;

	case 37:
		_helpEnabled = false;
		_dutyOfficer.remove();
		_palette.loadPalette(9998);
		R2_GLOBALS._scenePalette.addFader(_palette._palette, 256, 8, this);
		break;

	case 38:
		_door.remove();
		_shipDisplay.setAction(NULL);
		_shipDisplay.remove();

		_backSurface.fillRect(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		R2_GLOBALS._screen.fillRect(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.fadeOut2(this);
		break;

	case 39:
		R2_GLOBALS._sound1.changeSound(8);
		setSceneDelay(1);
		break;

	case 40:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_1;
		R2_GLOBALS._scene180Mode = 4;
		if (_animationPlayer.load(4)) {
			_animationPlayer.dispatch();
			R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 8, this);
		} else {
			_sceneMode = 43;
			setSceneDelay(1);
		}
		break;

	case 41:
		_helpEnabled = true;
		_animationPlayer._isActive = true;
		break;

	case 42:
		R2_GLOBALS._scene180Mode = 4;
		R2_GLOBALS._paneRefreshFlag[0] = 3;
		setSceneDelay(1);
		break;

	case 44:
		loadScene(9997);
		R2_GLOBALS._scenePalette.loadPalette(9997);
		if (R2_GLOBALS._sceneManager._hasPalette)
			R2_GLOBALS._scenePalette.refresh();

		setSceneDelay(6);
		break;

	case 45:
		_helpEnabled = true;
		_stripManager.start(28, this);
		break;

	case 48:
		_helpEnabled = true;
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._isActive = true;
		_animationPlayer._objectMode = ANIMOBJMODE_1;
		R2_GLOBALS._scene180Mode = 15;
		_animationPlayer.load(15, NULL);

		R2_GLOBALS._sound1.play(9);
		R2_GLOBALS._scenePalette.addFader(_animationPlayer._subData._palData, 256, 6, NULL);
		break;

	case 49:
		R2_GLOBALS._scene180Mode = 15;
		R2_GLOBALS._paneRefreshFlag[0] = 3;

		_backSurface.fillRect(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);
		R2_GLOBALS._screen.fillRect(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 0);

		setSceneDelay(1);
		break;

	case 50:
		R2_GLOBALS._scene180Mode = 0;
		_helpEnabled = false;

		// WORKAROUND: The original changed to scene 100 here, Quinn's Bedroom,
		// but instead we're changing to the previously unused scene 50, which shows
		// a closeup of Quinn in the floatation bed first
		R2_GLOBALS._sceneManager.changeScene(50);
		break;

	default:
		break;
	}
}

void Scene180::setSceneDelay(int v) {
	_frameInc = v;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

void Scene180::process(Event &event) {
	if ((event.eventType == EVENT_CUSTOM_ACTIONSTART) && (event.customType == kActionEscape)) {
		event.handled = true;
		if (_helpEnabled) {
			if (R2_GLOBALS._scenePalette._listeners.size() == 0)
				HelpDialog::show();
		}
	}

	if (!event.handled)
		SceneExt::process(event);
}

void Scene180::dispatch() {
	if (_frameInc) {
		uint32 gameFrame = R2_GLOBALS._events.getFrameNumber();

		if (gameFrame >= (uint32)_frameNumber) {
			_frameInc -= gameFrame - _frameNumber;
			_frameNumber = gameFrame;

			if (_frameInc <= 0) {
				_frameInc = 0;
				signal();
			}
		}
	}

	if (_animationPlayer._isActive) {
		if (_animationPlayer.isCompleted()) {
			_animationPlayer._isActive = false;
			_animationPlayer.close();
			_animationPlayer.remove();

			signal();
		} else {
			_animationPlayer.dispatch();
		}
	}

	Scene::dispatch();
}

void Scene180::restore() {
	R2_GLOBALS._gfxColors.background = 0;
	R2_GLOBALS._gfxColors.foreground = 0xff;
	R2_GLOBALS._fontColors.background = 0;
	R2_GLOBALS._fontColors.foreground = 0xff;

	switch (R2_GLOBALS._scene180Mode) {
	case 0:
		R2_GLOBALS._events.setCursor(SHADECURSOR_HAND);

		R2_GLOBALS._gfxColors.foreground = 4;
		R2_GLOBALS._gfxColors.background = 3;
		R2_GLOBALS._fontColors.background = 3;
		R2_GLOBALS._frameEdgeColor = 3;
		break;

	case 1:
		R2_GLOBALS._events.setCursor(R2_CURSOR_20);

		R2_GLOBALS._gfxColors.foreground = 25;
		R2_GLOBALS._gfxColors.background = 43;
		R2_GLOBALS._fontColors.background = 48;
		R2_GLOBALS._frameEdgeColor = 48;
		break;

	case 2:
		R2_GLOBALS._events.setCursor(R2_CURSOR_21);

		R2_GLOBALS._gfxColors.foreground = 106;
		R2_GLOBALS._gfxColors.background = 136;
		R2_GLOBALS._fontColors.background = 48;
		R2_GLOBALS._fontColors.foreground = 253;
		R2_GLOBALS._frameEdgeColor = 48;
		break;

	case 3:
		R2_GLOBALS._events.setCursor(R2_CURSOR_22);

		R2_GLOBALS._gfxColors.foreground = 84;
		R2_GLOBALS._gfxColors.background = 118;
		R2_GLOBALS._fontColors.background = 47;
		R2_GLOBALS._frameEdgeColor = 48;
		break;

	case 14:
		R2_GLOBALS._events.setCursor(R2_CURSOR_23);

		R2_GLOBALS._fontColors.background = 38;
		R2_GLOBALS._fontColors.foreground = 38;
		R2_GLOBALS._gfxColors.foreground = 192;
		R2_GLOBALS._gfxColors.background = 30;
		R2_GLOBALS._frameEdgeColor = 48;
		break;

	default:
		R2_GLOBALS._gfxColors.background = 0;
		R2_GLOBALS._gfxColors.foreground = 59;
		R2_GLOBALS._fontColors.background = 4;
		R2_GLOBALS._fontColors.foreground = 15;

		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 200 - Ship Corridor
 *
 *--------------------------------------------------------------------------*/

bool Scene200::NorthDoor::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::EastDoor::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 200;
		scene->setAction(&scene->_sequenceManager, scene, 200, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene200::WestDoor::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

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
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 206;
	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene200::WestExit::changeScene() {
	Scene200 *scene = (Scene200 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 208;
	scene->setAction(&scene->_sequenceManager, scene, 208, &R2_GLOBALS._player, NULL);
}

/*--------------------------------------------------------------------------*/

void Scene200::postInit(SceneObjectList *OwnerList) {
	loadScene(200);
	SceneExt::postInit();

	_westExit.setDetails(Rect(94, 0, 123, 58), EXITCURSOR_W, 175);
	_westExit.setDest(Common::Point(125, 52));
	_eastExit.setDetails(Rect(133, 0, 167, 58), EXITCURSOR_E, 150);
	_eastExit.setDest(Common::Point(135, 52));

	_northDoor.postInit();
	_northDoor.setVisage(200);
	_northDoor.setPosition(Common::Point(188, 79));
	_northDoor.setDetails(200, 3, -1, -1, 1, (SceneItem *)NULL);

	_eastDoor.postInit();
	_eastDoor.setVisage(200);
	_eastDoor.setStrip(2);
	_eastDoor.setPosition(Common::Point(305, 124));
	_eastDoor.setDetails(200, 6, -1, -1, 1, (SceneItem *)NULL);

	_westDoor.postInit();
	_westDoor.setVisage(200);
	_westDoor.setStrip(3);
	_westDoor.setPosition(Common::Point(62, 84));
	_westDoor.setDetails(200, 9, -1, -1, 1, (SceneItem *)NULL);

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
 * Scene 205 - Star-field Credits
 *
 *--------------------------------------------------------------------------*/

void Scene205::Action1::signal() {
	Scene205 *scene = (Scene205 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;

	case 1:
		scene->_yp = 100 - (scene->_fontHeight * 3 / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}
		setDelay(240);
		break;

	case 2:
	case 4:
	case 6:
	case 8:
	case 10:
	case 12:
	case 14:
	case 16:
	case 18:
	case 20:
		textLoop();
		setDelay(120);
		break;

	case 3:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 2) / 2);
		scene->handleText();

		++scene->_textIndex;
		scene->_yp += scene->_fontHeight;
		scene->handleText();
		setDelay(240);
		break;

	case 5:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 7) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 6; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 7:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 6) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 5; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(300);
		break;

	case 9:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 8) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 7; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 11:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 13:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 15:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 5) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 4; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(240);
		break;

	case 17:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 5) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 4; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(360);
		break;

	case 19:
		scene->_textIndex = 1;
		scene->_yp = 100 - ((scene->_fontHeight * 3) / 2);
		scene->handleText();

		for (int idx = 1; idx <= 2; ++idx) {
			++scene->_textIndex;
			scene->_yp += scene->_fontHeight;
			scene->handleText();
		}

		setDelay(480);
		break;

	case 21:
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);

	default:
		break;
	}
}

void Scene205::Action1::textLoop() {
	Scene205 *scene = (Scene205 *)R2_GLOBALS._sceneManager._scene;

	for (int idx = 1; idx <= 14; ++idx) {
		if (R2_GLOBALS._sceneObjects->contains(&scene->_textList[idx])) {
			scene->_textList[idx].remove();
		}
	}
}

/*--------------------------------------------------------------------------*/

Scene205::Scene205(): SceneExt() {
	_yp = 0;
	_textIndex = 1;
	_lineNum = -1;

	GfxFont font;
	font.setFontNumber(4);
	_fontHeight = font.getHeight();

	for (int i = 0; i < 3; i++) {
		_starList1[i] = nullptr;
		_starList2[i] = nullptr;
	}

	for (int i = 0; i < 4; i++)
		_starList3[i] = nullptr;
}

void Scene205::postInit(SceneObjectList *OwnerList) {
	loadScene(4000);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;
	R2_GLOBALS._player._uiEnabled = false;

	R2_GLOBALS._sound1.play(337);
	R2_GLOBALS._scenePalette.loadPalette(0);
	R2_GLOBALS._player.disableControl();

	setup();
	setAction(&_action1);
}

void Scene205::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	for (int idx = 0; idx < 3; ++idx)
		SYNC_POINTER(_starList1[idx]);
	for (int idx = 0; idx < 3; ++idx)
		SYNC_POINTER(_starList2[idx]);
	for (int idx = 0; idx < 4; ++idx)
		SYNC_POINTER(_starList3[idx]);

	s.syncAsSint16LE(_textIndex);
	s.syncAsSint16LE(_lineNum);
	s.syncAsSint16BE(_yp);
}

void Scene205::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene205::process(Event &event) {
	if ((event.eventType == EVENT_CUSTOM_ACTIONSTART) && (event.customType == kActionEscape)) {
		event.handled = true;
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);
	} else {
		Scene::process(event);
	}
}

void Scene205::dispatch() {
	processList(_starList3, 4, Common::Rect(0, 0, 319, 200), 1, 1, 160, 100);
	processList(_starList2, 3, Common::Rect(0, 0, 319, 200), 2, 2, 160, 100);
	processList(_starList1, 3, Common::Rect(0, 0, 319, 200), 4, 3, 160, 100);

	Scene::dispatch();
}

void Scene205::setup() {
	const Common::Point pointList1[3] = { Common::Point(2, 50), Common::Point(100, 28), Common::Point(53, 15) };
	const Common::Point pointList2[3] = { Common::Point(289, 192), Common::Point(125, 60), Common::Point(130, 40) };
	const Common::Point pointList3[4] = {
		Common::Point(140, 149), Common::Point(91, 166), Common::Point(299, 46), Common::Point(314, 10)
	};

	// Set up the first star list
	for (int idx = 0; idx < 3; ++idx) {
		Star *obj = new Star();
		_starList1[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 1;
		obj->setPosition(pointList1[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(12);
	}

	// Setup the second star list
	for (int idx = 0; idx < 3; ++idx) {
		Star *obj = new Star();
		_starList2[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 2;
		obj->setPosition(pointList2[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(11);
	}

	// Setup the third star list
	for (int idx = 0; idx < 4; ++idx) {
		Star *obj = new Star();
		_starList3[idx] = obj;

		obj->postInit();
		obj->_flags |= OBJFLAG_CLONED;
		obj->setVisage(205);
		obj->_strip = 1;
		obj->_frame = 3;
		obj->setPosition(pointList3[idx]);
		obj->_x100 = obj->_position.x * 100;
		obj->_y100 = obj->_position.y * 100;
		obj->fixPriority(10);
	}
}

/**
 * Handles moving a group of stars in the scene background
 */
void Scene205::processList(Star **ObjList, int count, const Common::Rect &bounds,
						   int xMultiply, int yMultiply, int xCenter, int yCenter) {
	for (int idx = 0; idx < count; ++idx) {
		Star *obj = ObjList[idx];
		Common::Point pt(obj->_position.x - xCenter, obj->_position.y - yCenter);

		if ((obj->_position.x <= 319) && (obj->_position.x >= 0) &&
				(obj->_position.y <= 199) && (obj->_position.y >= 0)) {
			if (!pt.x && !pt.y) {
				pt.x = pt.y = 1;
			}

			pt.x *= xMultiply;
			pt.y *= yMultiply;
			obj->_x100 += pt.x;
			obj->_y100 += pt.y;
		} else {
			obj->_x100 = (bounds.left + R2_GLOBALS._randomSource.getRandomNumber(bounds.right)) * 100;
			obj->_y100 = (bounds.top + R2_GLOBALS._randomSource.getRandomNumber(bounds.bottom)) * 100;
		}

		obj->setPosition(Common::Point(obj->_x100 / 100, obj->_y100 / 100));
   }
}

void Scene205::handleText() {
	_message = g_resourceManager->getMessage(205, ++_lineNum);

	_textList[_textIndex]._fontNumber = 4;
	_textList[_textIndex]._color1 = 0;
	_textList[_textIndex]._color2 = 10;
	_textList[_textIndex]._color3 = 7;
	_textList[_textIndex]._width = 400;
	_textList[_textIndex].setup(_message);
	_textList[_textIndex].fixPriority(199);

	GfxFont font;
	font.setFontNumber(4);
	int width = font.getStringWidth(_message.c_str());

	_textList[_textIndex].setPosition(Common::Point(160 - (width / 2), _yp));
}

/*--------------------------------------------------------------------------
 * Scene 205 Demo - End of Demo
 *
 *--------------------------------------------------------------------------*/

void Scene205Demo::Action1::signal() {
	Scene205Demo *scene = (Scene205Demo *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(2);
		break;
	case 1:
		MessageDialog::show2(BUY_FULL_GAME_MSG, OK_BTN_STRING);
		setDelay(1);
		break;
	case 2:
		scene->leaveScene();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene205Demo::leaveScene() {
	if (g_globals->getFlag(85))
		R2_GLOBALS._sceneManager.changeScene(160);
	else
		R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._sceneManager._previousScene);

	BF_GLOBALS._scenePalette.loadPalette(0);
	BF_GLOBALS._scenePalette.refresh();
}

void Scene205Demo::postInit(SceneObjectList *OwnerList) {
	R2_GLOBALS._sceneManager._hasPalette = true;
	R2_GLOBALS._scenePalette.loadPalette(0);

	loadScene(1000);
	R2_GLOBALS._uiElements._active = false;
	R2_GLOBALS._player.enableControl();

	SceneExt::postInit();

	_sound1.play(337);
	_stripManager.addSpeaker(&_animationPlayer);

	setAction(&_action1);
}

void Scene205Demo::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene205Demo::process(Event &event) {
	if ((event.eventType == EVENT_CUSTOM_ACTIONSTART) && (event.customType == kActionEscape)) {
		event.handled = true;
		leaveScene();
	} else {
		Scene::process(event);
	}
}

/*--------------------------------------------------------------------------
 * Scene 250 - Lift
 *
 *--------------------------------------------------------------------------*/

Scene250::Button::Button(): SceneActor() {
	_floorNumber = 0;
}

void Scene250::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_floorNumber);
}

bool Scene250::Button::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene250 *scene = (Scene250 *)R2_GLOBALS._sceneManager._scene;

		if (scene->_destButtonY) {
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
		}

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
		setDetails(250, -1, -1, -1, 1, (SceneItem *)NULL);
	}
}

/*--------------------------------------------------------------------------*/

Scene250::Scene250(): SceneExt() {
	_currButtonY = _destButtonY = _elevatorSpeed = 0;
	_skippingFl = _skippableFl = false;
}

void Scene250::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_currButtonY);
	s.syncAsSint16LE(_destButtonY);
	s.syncAsSint16LE(_elevatorSpeed);
	s.syncAsSint16LE(_skippableFl);
	s.syncAsSint16LE(_skippingFl);
}

void Scene250::postInit(SceneObjectList *OwnerList) {
	loadScene(250);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;

	_currentFloor.setFloor(10);
	_currentFloor.setup(250, 1, 5);
	_currentFloor.setDetails(250, 13, -1, -1, 1, (SceneItem *)NULL);

	_destinationFloor.setFloor(11);
	_destinationFloor.setup(250, 1, 3);
	_destinationFloor.setPosition(Common::Point(400, 100));
	_destinationFloor.setDetails(250, 14, -1, -1, 1, (SceneItem *)NULL);
	_destinationFloor.fixPriority(190);
	_destinationFloor.hide();

	_floor1.setFloor(1);
	_floor2.setFloor(2);
	_floor3.setFloor(3);
	_floor4.setFloor(4);
	_floor5.setFloor(5);
	_floor6.setFloor(6);
	_floor7.setFloor(7);
	_floor8.setFloor(8);
	_floor9.setFloor(9);

	_door.setDetails(Rect(0, 0, 73, SCREEN_HEIGHT), 250, 9, -1, 9, 1, NULL);
	_directionIndicator.setDetails(Rect(239, 16, 283, 164), 250, 6, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 250, 0, 1, -1, 1, NULL);

	R2_GLOBALS._events.setCursor(CURSOR_USE);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 200:
		_currButtonY = 55;
		break;
	case 300:
		_currButtonY = 43;
		break;
	case 700:
		_currButtonY = 139;
		break;
	case 850:
		_currButtonY = 91;
		break;
	default:
		R2_GLOBALS._sceneManager._previousScene = 200;
		_currButtonY = 55;
		break;
	}

	_currentFloor.setPosition(Common::Point(111, _currButtonY));
}

void Scene250::signal() {
	if (_skippingFl)
		_sceneMode = 20;

	switch (_sceneMode) {
	case 1:
		_sound1.play(22);
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setup(250, 1, 2);
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);

		_elevatorSpeed = 0;
		_sceneMode = 2;
		break;
	case 2:
		if (_destButtonY - 12 == _currButtonY)
			_sceneMode = 4;
		else
			_sceneMode = 3;

		signal();
		break;
	case 3:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y + 12));
		_currButtonY += 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 185));
		ADD_MOVER(R2_GLOBALS._player, 261, 15);

		if ((_destButtonY - 12) == _currButtonY)
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
		_elevatorSpeed = 0;
		_sceneMode = 7;
		break;
	case 7:
		_skippableFl = true;
		if ((_destButtonY + 12) == _currButtonY)
			_sceneMode = 9;
		else
			_sceneMode = 8;
		signal();
		break;
	case 8:
		_currentFloor.setPosition(Common::Point(111, _currentFloor._position.y - 12));
		_currButtonY -= 12;
		R2_GLOBALS._player.setPosition(Common::Point(261, 15));
		ADD_MOVER(R2_GLOBALS._player, 261, 185);

		if ((_destButtonY + 12) == _currButtonY)
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
		switch (_destButtonY) {
		case 55:
			R2_GLOBALS._sceneManager.changeScene(200);
			break;
		case 43:
			R2_GLOBALS._sceneManager.changeScene(300);
			break;
		case 139:
			R2_GLOBALS._sceneManager.changeScene(700);
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
	_destButtonY = (floorNumber - 1) * 12 + 43;
	_destinationFloor.setPosition(Common::Point(111, _destButtonY));
	_destinationFloor.show();

	_skippableFl = true;
	_sceneMode = (_currButtonY >= _destButtonY) ? 6 : 1;
	if (_destButtonY == _currButtonY)
		_sceneMode = 20;

	signal();
}

void Scene250::process(Event &event) {
	if (!event.handled) {
		if (((event.eventType == EVENT_KEYPRESS) || (event.btnState == BTNSHIFT_RIGHT)) && _skippableFl) {
			_skippingFl = true;
			event.handled = true;
		}

		SceneExt::process(event);
	}
}

void Scene250::dispatch() {
	SceneExt::dispatch();

	if (((_sceneMode == 2) || (_sceneMode == 7)) && (_elevatorSpeed < 100)) {
		++_elevatorSpeed;
		R2_GLOBALS._player._moveDiff.y = _elevatorSpeed / 5;
	}

	if (((_sceneMode == 5) || (_sceneMode == 10)) && (R2_GLOBALS._player._moveDiff.y > 4)) {
		--_elevatorSpeed;
		R2_GLOBALS._player._moveDiff.y = (_elevatorSpeed / 7) + 3;
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
		setAction(&scene->_sequenceManager2, this, 311, (R2_GLOBALS._player._characterIndex == R2_QUINN) ?
			(SceneObject *)&R2_GLOBALS._player : (SceneObject *)&scene->_quinn);
		_actionIndex = 2;
		break;
	case 1:
		setAction(&scene->_sequenceManager2, this, 312, (R2_GLOBALS._player._characterIndex == R2_QUINN) ?
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
	if (!R2_GLOBALS._playStream.isPlaying()) {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

		scene->_mirandaScreen.setStrip2(R2_GLOBALS._randomSource.getRandomNumber(2));
		scene->_mirandaScreen.setFrame(1);

		scene->_quinnScreen.setStrip2(3);
		scene->_quinnScreen.setFrame(1);
	}

	setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(479));
}

/*--------------------------------------------------------------------------*/

bool Scene300::QuinnWorkstation::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != R2_QUINN)
			SceneItem::display2(300, 46);
		else if (R2_GLOBALS.getFlag(44)) {
			R2_GLOBALS._player.setAction(NULL);
			R2_GLOBALS._sceneManager.changeScene(325);
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 306;
			scene->setAction(&scene->_sequenceManager1, scene, 306, &R2_GLOBALS._player, NULL);
		}
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
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
		if (R2_GLOBALS._player._characterIndex != R2_MIRANDA)
			SceneItem::display2(300, 49);
		else
			R2_GLOBALS._sceneManager.changeScene(325);
		return true;

	case CURSOR_LOOK:
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA) {
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
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			SceneItem::display2(300, 47);
			return true;
		}
		break;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != R2_SEEKER)
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
				scene->_stripManager.start3(201, scene, R2_GLOBALS._stripManager_lookupList);
			} else {
				scene->_sceneMode = 16;

				if (!R2_GLOBALS.getFlag(57)) {
					R2_GLOBALS._events.setCursor(CURSOR_ARROW);
					scene->_stripManager.start3(434, scene, R2_GLOBALS._stripManager_lookupList);
				} else if (R2_GLOBALS._player._characterScene[R2_SEEKER] != 500) {
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
			if (!R2_GLOBALS.getFlag(44)) {
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

			scene->_sceneMode = 310;
			scene->setAction(&scene->_sequenceManager1, scene, 310, &R2_GLOBALS._player, NULL);
		}
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
	switch (action) {
	case CURSOR_TALK: {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		scene->_sceneMode = 10;

		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA) {
			if (R2_GLOBALS._player._characterScene[R2_MIRANDA] == 500)
				scene->_stripId = 442;
			else if (!R2_GLOBALS.getFlag(44))
				scene->_stripId = 125 + R2_GLOBALS._randomSource.getRandomNumber(2);
			else if (!R2_GLOBALS.getFlag(55))
				scene->_stripId = 439;
			else
				scene->_stripId = 210;
		} else if (R2_GLOBALS.getFlag(44)) {
			scene->_stripId = R2_GLOBALS.getFlag(55) ? 439 : 210;
		} else {
			scene->_stripId = 177 + R2_GLOBALS._randomSource.getRandomNumber(2);
		}

		scene->_stripManager.start3(scene->_stripId, scene, R2_GLOBALS._stripManager_lookupList);
		return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene300::Doorway::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		if ((R2_GLOBALS._player._characterIndex == R2_QUINN) &&
				(!R2_GLOBALS.getFlag(44) || R2_GLOBALS._player._characterScene[R2_SEEKER] == 500)) {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;

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

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
}

void Scene300::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_stripId);
	SYNC_POINTER(_rotation);
}

void Scene300::postInit(SceneObjectList *OwnerList) {
	loadScene(300);
	SceneExt::postInit();

	_sound1.play(23);
	setZoomPercents(75, 93, 120, 100);

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1000;
		R2_GLOBALS._player._characterIndex = R2_QUINN;
	}

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
		_atmosphereLeftWindow.postInit();
		_atmosphereLeftWindow.setup(301, 7, 2);
		_atmosphereLeftWindow.setPosition(Common::Point(65, 24));

		_atmosphereRightWindow.postInit();
		_atmosphereRightWindow.setup(301, 8, 2);
		_atmosphereRightWindow.setPosition(Common::Point(254, 24));
	}

	_doorway.postInit();
	_doorway.setVisage(300);
	_doorway.setPosition(Common::Point(159, 79));

	_leftVerticalBarsAnim.postInit();
	_leftVerticalBarsAnim.setup(300, 4, 1);
	_leftVerticalBarsAnim.setPosition(Common::Point(84, 48));
	_leftVerticalBarsAnim.animate(ANIM_MODE_2, NULL);
	_leftVerticalBarsAnim._numFrames = 5;

	_rightVerticalBarsAnim.postInit();
	_rightVerticalBarsAnim.setup(300, 5, 1);
	_rightVerticalBarsAnim.setPosition(Common::Point(236, 48));
	_rightVerticalBarsAnim.animate(ANIM_MODE_2, NULL);

	_protocolDisplay.postInit();
	_protocolDisplay.setup(300, 6, 1);
	_protocolDisplay.setPosition(Common::Point(287, 71));
	_protocolDisplay.animate(ANIM_MODE_7, 0, NULL);
	_protocolDisplay._numFrames = 5;

	_rightTextDisplay.postInit();
	_rightTextDisplay.setup(300, 7, 1);
	_rightTextDisplay.setPosition(Common::Point(214, 37));
	_rightTextDisplay.animate(ANIM_MODE_2, NULL);
	_rightTextDisplay._numFrames = 3;

	_mirandaScreen.postInit();
	_mirandaScreen.setup(301, 1, 1);
	_mirandaScreen.setPosition(Common::Point(39, 97));
	_mirandaScreen.fixPriority(124);
	_mirandaScreen.animate(ANIM_MODE_2, NULL);
	_mirandaScreen._numFrames = 5;
	_mirandaScreen.setAction(&_action4);

	_leftTextDisplay.postInit();
	_leftTextDisplay.setup(300, 8, 1);
	_leftTextDisplay.setPosition(Common::Point(105, 37));
	_leftTextDisplay.animate(ANIM_MODE_2, NULL);
	_leftTextDisplay._numFrames = 5;

	_quinnScreen.postInit();
	_quinnScreen.setup(301, 6, 1);
	_quinnScreen.setPosition(Common::Point(274, 116));
	_quinnScreen.fixPriority(143);
	_quinnScreen.animate(ANIM_MODE_2, NULL);
	_quinnScreen._numFrames = 5;

	_quinnWorkstation1.setDetails(Rect(243, 148, 315, 167), 300, 30, 31, 32, 1, NULL);
	_mirandaWorkstation1.setDetails(Rect(4, 128, 69, 167), 300, 33, 31, 35, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case R2_QUINN:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, (SceneItem *)NULL);

		if ((R2_GLOBALS._player._characterScene[R2_SEEKER] == 300) || (R2_GLOBALS._player._characterScene[R2_SEEKER] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setVisage(10);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.disableControl();
		break;

	case R2_SEEKER:
		_miranda.postInit();
		_miranda.setup(302, 2, 1);
		_miranda.setPosition(Common::Point(47, 128));
		_miranda.setAction(&_action3);
		_miranda.setDetails(300, 39, 40, 41, 1, (SceneItem *)NULL);

		if ((R2_GLOBALS._player._characterScene[R2_QUINN] == 300) || (R2_GLOBALS._player._characterScene[R2_QUINN] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 1, 3);
		R2_GLOBALS._player.setPosition(Common::Point(158, 108));
		R2_GLOBALS._player.fixPriority(130);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;

	case R2_MIRANDA:
		if ((R2_GLOBALS._player._characterScene[R2_SEEKER] == 300) || (R2_GLOBALS._player._characterScene[R2_SEEKER] == 325)) {
			_seeker.postInit();
			_seeker.setVisage(302);
			_seeker.setPosition(Common::Point(158, 108));
			_seeker.fixPriority(130);
			_seeker.setAction(&_action2);
			_seeker.setDetails(300, 42, 43, 44, 1, (SceneItem *)NULL);
		}

		if ((R2_GLOBALS._player._characterScene[R2_QUINN] == 300) || (R2_GLOBALS._player._characterScene[R2_QUINN] == 325)) {
			_quinn.postInit();
			_quinn.setup(302, 3, 1);
			_quinn.setPosition(Common::Point(271, 150));
			_quinn.setAction(&_action1);
			_quinn.setDetails(300, 50, 51, 52, 1, (SceneItem *)NULL);
		}

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.setup(302, 2, 1);
		R2_GLOBALS._player.setPosition(Common::Point(47, 128));
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
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
	_protocolDisplay.setDetails(300, 27, -1, 29, 1, (SceneItem *)NULL);
	_indirectLighting1.setDetails(Rect(74, 71, 122, 89), 300, 21, -1, -1, 1, NULL);
	_indirectLighting2.setDetails(Rect(197, 71, 245, 89), 300, 21, -1, -1, 1, NULL);
	_lighting.setDetails(Rect(129, 3, 190, 14), 300, 24, -1, -1, 1, NULL);
	_doorway.setDetails(300, 3, -1, 5, 1, (SceneItem *)NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 300, 0, -1, -1, 1, NULL);

	switch (R2_GLOBALS._player._characterIndex) {
	case R2_QUINN:
		_sceneMode = 300;

		switch (R2_GLOBALS._sceneManager._previousScene) {
		case 250:
			setAction(&_sequenceManager1, this, 300, &R2_GLOBALS._player, &_doorway, NULL);
			break;
		case 325:
			if (!R2_GLOBALS.getFlag(44) || R2_GLOBALS.getFlag(25))
				setAction(&_sequenceManager1, this, 307, &R2_GLOBALS._player, NULL);
			else {
				R2_GLOBALS.setFlag(60);
				R2_GLOBALS._player.setup(302, 3, 1);
				R2_GLOBALS._player.setPosition(Common::Point(271, 150));
				R2_GLOBALS._player.setAction(&_action1);

				if (R2_GLOBALS.getFlag(55)) {
					if (R2_GLOBALS.getFlag(57)) {
						// Little hack to get the correct sentence order
						R2_GLOBALS._stripManager_lookupList[8] = 2;

						R2_GLOBALS.clearFlag(60);
						R2_GLOBALS._events.setCursor(CURSOR_ARROW);
						_sceneMode = 16;
						_stripManager.start3(404, this, R2_GLOBALS._stripManager_lookupList);
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
				// Things don't seem right
				_sceneMode = 13;
				_stripManager.start3(300, this, R2_GLOBALS._stripManager_lookupList);
			} else {
				// Back in Ringworld space
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

	case R2_MIRANDA:
		if (R2_GLOBALS._sceneManager._previousScene == 1500) {
			R2_GLOBALS._player._oldCharacterScene[R2_MIRANDA] = 3150;
			R2_GLOBALS._player._characterScene[R2_MIRANDA] = 3150;
			R2_GLOBALS._player._effect = EFFECT_NONE;
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
		switch (_stripManager._exitMode) {
		case 1:
			R2_GLOBALS._sound1.changeSound(10);
			R2_GLOBALS.setFlag(38);
			break;
		case 2:
			R2_GLOBALS.setFlag(3);
			break;
		case 3:
			R2_GLOBALS.setFlag(4);
			break;
		case 4:
			R2_GLOBALS.setFlag(13);
			if (R2_GLOBALS._stripManager_lookupList[1] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 5:
			if (R2_GLOBALS._stripManager_lookupList[0] == 6)
				R2_GLOBALS.setFlag(40);
			break;
		case 6:
			if (g_vm->getFeatures() & GF_DEMO) {
				R2_GLOBALS.setFlag(85);
				R2_GLOBALS._sceneManager.changeScene(205);
			} else
				R2_GLOBALS._sceneManager.changeScene(1000);
			break;
		default:
			break;
		}

		_stripManager._currObj44Id = 0;
		switch (_stripId) {
		case 400:
			R2_GLOBALS._player.disableControl();
			_sceneMode = 15;
			setAction(&_sequenceManager1, this, 306, &R2_GLOBALS._player, NULL);
			break;
		case 181:
			R2_GLOBALS._player.setStrip(6);
			// fall through
		default:
			R2_GLOBALS._player.enableControl(CURSOR_TALK);

			if ((R2_GLOBALS._player._characterIndex != R2_QUINN) || R2_GLOBALS.getFlag(44))
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
		if (_stripManager._exitMode == 1) {
			if (g_vm->getFeatures() & GF_DEMO) {
				R2_GLOBALS._player.setAction(NULL);
				R2_GLOBALS.setFlag(85);
				R2_GLOBALS._sceneManager.changeScene(205);
			} else {
				R2_GLOBALS._player.setAction(NULL);
				R2_GLOBALS._sceneManager.changeScene(1000);
			}
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
		// fall through
	case 309:
		signal309();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._events._currentCursor = CURSOR_ARROW;

		_sceneMode = 10;
		_stripManager.start3(_stripId, this, R2_GLOBALS._stripManager_lookupList);
		break;

	case 313:
		_sceneMode = 14;
		R2_GLOBALS._player._effect = EFFECT_NONE;
		_seeker.setAction(&_sequenceManager3, this, 314, &_seeker, &_doorway, NULL);
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(301, this);
		break;

	case 314:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 315;
		R2_GLOBALS._player._effect = EFFECT_SHADED;
		setAction(&_sequenceManager1, this, 315, &R2_GLOBALS._player, &_doorway, NULL);
		break;

	case 315:
		R2_GLOBALS._sceneManager.changeScene(1100);
		break;

	case 316:
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 500;
		_seeker.remove();
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
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
	// Sets up what conversation items will be available when to talking to the
	// others on the Bridge, and will be set dependent on game flags
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
 * Scene 325 - Bridge Console
 *
 *--------------------------------------------------------------------------*/

const double ADJUST_FACTOR = 0.06419999999999999;

/*--------------------------------------------------------------------------*/

Scene325::Icon::Icon(): SceneActor()  {
	_lookLineNum = 0;
	_iconId = 0;
	_pressed = false;
}

void Scene325::Icon::postInit(SceneObjectList *OwnerList) {
	SceneObject::postInit();

	_glyph.postInit();
	_glyph.fixPriority(21);
	_glyph.hide();

	_sceneText1._color1 = 92;
	_sceneText1._color2 = 0;
	_sceneText1._width = 200;
	_sceneText2._color1 = 0;
	_sceneText2._color2 = 0;
	_sceneText2._width = 200;
	fixPriority(20);
}

void Scene325::Icon::synchronize(Serializer &s) {
	SceneActor::synchronize(s);
	s.syncAsSint16LE(_lookLineNum);
	s.syncAsSint16LE(_iconId);
	s.syncAsSint16LE(_pressed);
}

void Scene325::Icon::process(Event &event) {
	Scene325 *scene = (Scene325 *)R2_GLOBALS._sceneManager._scene;

	if (!event.handled && !(_flags & OBJFLAG_HIDING) && R2_GLOBALS._player._uiEnabled) {

		if (event.eventType == EVENT_BUTTON_DOWN) {
			int regionIndex = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);

			switch (R2_GLOBALS._events.getCursor()) {
			case CURSOR_LOOK:
				if (regionIndex == _sceneRegionId) {
					event.handled = true;
					SceneItem::display2(326, _lookLineNum);
				}
				break;

			case CURSOR_USE:
				if ((regionIndex == _sceneRegionId) && !_pressed) {
					scene->_sound1.play(14);
					setFrame(2);

					switch (_glyph._strip) {
					case 1:
						_glyph.setStrip(2);
						break;
					case 3:
						_glyph.setStrip(4);
						break;
					case 5:
						_glyph.setStrip(6);
						break;
					case 7:
						_glyph.setStrip(8);
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

			switch (_glyph._strip) {
			case 2:
				_glyph.setStrip(1);
				break;
			case 4:
				_glyph.setStrip(3);
				break;
			case 6:
				_glyph.setStrip(5);
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

void Scene325::Icon::setIcon(int id) {
	_lookLineNum = _iconId = id;
	SceneActor::_lookLineNum = id;

	_sceneText1.remove();
	_sceneText2.remove();

	if (_lookLineNum) {
		Scene325 *scene = (Scene325 *)R2_GLOBALS._sceneManager._scene;

		showIcon();
		_glyph.setup(325, ((id - 1) / 10) * 2 + 1, ((id - 1) % 10) + 1);
		_glyph.setPosition(_position);

		_sceneText1._fontNumber = scene->_iconFontNumber;
		_sceneText1.setup(CONSOLE325_MESSAGES[id]);
		_sceneText1.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE325_MESSAGES[id]);
		_sceneText2.fixPriority(20);

		_sceneText2._fontNumber = scene->_iconFontNumber;
		_sceneText2.setup(CONSOLE325_MESSAGES[id]);
		_sceneText2.fixPriority(10);

		switch (_lookLineNum) {
		case 7:
			_sceneText1.setPosition(Common::Point(62, _position.y + 8));
			_sceneText2.setPosition(Common::Point(64, _position.y + 10));
			break;
		case 8:
		case 9:
			_sceneText1.setPosition(Common::Point(65, _position.y + 8));
			_sceneText2.setPosition(Common::Point(67, _position.y + 10));
			break;
		case 12:
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

void Scene325::Icon::showIcon() {
	_sceneText1.show();
	_sceneText2.show();
	_glyph.show();
	_horizLine.show();
	show();
}

void Scene325::Icon::hideIcon() {
	_sceneText1.hide();
	_sceneText2.hide();
	_glyph.hide();
	_horizLine.hide();
	hide();
}

/*--------------------------------------------------------------------------*/

Scene325::Scene325(): SceneExt() {
	_consoleAction = 7;
	_iconFontNumber = 50;
	_databasePage = _priorConsoleAction = 0;
	_moveCounter = _yChange = _yDirection = _scannerLocation = 0;
	_soundCount = _soundIndex = 0;

	for (int idx = 0; idx < 10; ++idx)
		_soundQueue[idx] = 0;
}

void Scene325::postInit(SceneObjectList *OwnerList) {
	loadScene(325);
	SceneExt::postInit();

	R2_GLOBALS.clearFlag(50);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_palette.loadPalette(0);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_terminal.setDetails(1, 325, 3, 4, 5);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 325, 0, 1, 2, 1, (SceneItem *)NULL);
	_sceneMode = 1;
	signal();
}

void Scene325::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_consoleAction);
	s.syncAsSint16LE(_iconFontNumber);
	s.syncAsSint16LE(_databasePage);
	s.syncAsSint16LE(_priorConsoleAction);
	s.syncAsSint16LE(_moveCounter);
	s.syncAsSint16LE(_yChange);
	s.syncAsSint16LE(_yDirection);
	s.syncAsSint16LE(_scannerLocation);
	s.syncAsSint16LE(_soundCount);
	s.syncAsSint16LE(_soundIndex);

	for (int idx = 0; idx < 10; ++idx)
		s.syncAsSint16LE(_soundQueue[idx]);
}

void Scene325::remove() {
	removeText();
	SceneExt::remove();
}

void Scene325::signal() {
	switch (_sceneMode - 1) {
	case 0:
		_icon1.postInit();
		_icon1._sceneRegionId = 2;
		_icon2.postInit();
		_icon2._sceneRegionId = 3;
		_icon3.postInit();
		_icon3._sceneRegionId = 4;
		_icon4.postInit();
		_icon4._sceneRegionId = 5;

		setAction(&_sequenceManager1, this, 127, &_icon1, &_icon2, &_icon3, &_icon4,
			&R2_GLOBALS._player, NULL);
		_sceneMode = 2;
		break;
	case 1:
		_icon1.setup(160, 1, 1);
		_icon1.setPosition(Common::Point(65, 17));
		_icon1._horizLine.postInit();
		_icon1._horizLine.setup(160, 7, 1);
		_icon1._horizLine.setPosition(Common::Point(106, 41));

		_icon2.setup(160, 1, 1);
		_icon2.setPosition(Common::Point(80, 32));
		_icon2._horizLine.postInit();
		_icon2._horizLine.setup(160, 7, 2);
		_icon2._horizLine.setPosition(Common::Point(106, 56));

		_icon3.setup(160, 1, 1);
		_icon3.setPosition(Common::Point(65, 47));
		_icon3._horizLine.postInit();
		_icon3._horizLine.setup(160, 7, 1);
		_icon3._horizLine.setPosition(Common::Point(106, 71));

		_icon4.setup(160, 1, 1);
		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4._horizLine.postInit();
		_icon4._horizLine.setup(160, 7, 2);
		_icon4._horizLine.setPosition(Common::Point(106, 86));

		_icon5.postInit();
		_icon5.setup(160, 1, 1);
		_icon5._sceneRegionId = 7;
		_icon5.setPosition(Common::Point(37, 92));
		_icon5.setIcon(8);

		_icon6.postInit();
		_icon6.setup(160, 1, 1);
		_icon6.setPosition(Common::Point(106, 110));
		_icon6.setIcon(7);
		_icon6._sceneRegionId = 8;

		consoleAction(7);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 9:
		// Fade to black for console sub-section: database, or starchart
		switch (_consoleAction) {
		case 3:
			// Starchart
			_sceneMode = 129;
			_starGrid1.postInit();
			_starGrid2.postInit();
			_starGrid3.postInit();
			if (R2_GLOBALS.getFlag(13)) {
				// Show starchart with Ringworld present
				_starGrid4.postInit();
				setAction(&_sequenceManager1, this, 130, &R2_GLOBALS._player, &_starGrid1,
					&_starGrid2, &_starGrid3, &_starGrid4, NULL);
			} else {
				// Show starchart without Ringworld
				setAction(&_sequenceManager1, this, 129, &R2_GLOBALS._player, &_starGrid1,
					&_starGrid2, &_starGrid3, NULL);
			}
			break;
		case 17:
		case 18:
		case 19:
		case 20: {
			int v = 10 - ((21 - _consoleAction) * 2);
			if (R2_GLOBALS.getFlag(50))
				--v;
			if (_priorConsoleAction == 5)
				v += 8;

			if (R2_GLOBALS.getFlag(51) && (v == 2))
				R2_GLOBALS.setFlag(57);

			if (R2_GLOBALS.getFlag(44) && !R2_GLOBALS.getFlag(51)) {
				if (v != 13) {
					if (_priorConsoleAction == 6) {
					// Fix for original game bug.
					// The passive short scan geographical and astronomical sentences
					// are inverted in the original game.
						if (v == 6)
							v = 8;
						else if (v == 8)
							v = 6;
					}

					setMessage(328, v);
				} else {
					_scannerLocation = 864;

					_starGrid12.postInit();
					_starGrid12.setup(326, 4, 1);
					_starGrid12.setPosition(Common::Point(149, 128));
					_starGrid12.fixPriority(20);

					_starGrid13.postInit();
					_starGrid13.setup(326, 4, 2);
					_starGrid13.setPosition(Common::Point(149, 22 + (int)(_scannerLocation * ADJUST_FACTOR)));
					_starGrid13.fixPriority(21);

					_starGrid10.postInit();
					_starGrid10.setup(326, 1, 1);
					_starGrid10.setPosition(Common::Point(210, 20));
					_starGrid10.fixPriority(10);

					_starGrid1.postInit();
					_starGrid1.setup(326, 1, 1);
					_starGrid1.setPosition(Common::Point(210, 32));
					_starGrid1.fixPriority(10);

					_starGrid2.postInit();
					_starGrid2.setup(326, 1, 1);
					_starGrid2.setPosition(Common::Point(210, 44));
					_starGrid2.fixPriority(10);

					_starGrid3.postInit();
					_starGrid3.setup(326, 1, 1);
					_starGrid3.setPosition(Common::Point(210, 56));
					_starGrid3.fixPriority(10);

					_starGrid4.postInit();
					_starGrid4.setup(326, 1, 1);
					_starGrid4.setPosition(Common::Point(210, 68));
					_starGrid4.fixPriority(10);

					_starGrid5.postInit();
					_starGrid5.setup(326, 1, 1);
					_starGrid5.setPosition(Common::Point(210, 80));
					_starGrid5.fixPriority(10);

					_starGrid6.postInit();
					_starGrid6.setup(326, 1, 1);
					_starGrid6.setPosition(Common::Point(210, 92));
					_starGrid6.fixPriority(10);

					_starGrid7.postInit();
					_starGrid7.setup(326, 1, 1);
					_starGrid7.setPosition(Common::Point(210, 104));
					_starGrid7.fixPriority(10);

					_starGrid8.postInit();
					_starGrid8.setup(326, 1, 1);
					_starGrid8.setPosition(Common::Point(210, 116));
					_starGrid8.fixPriority(10);

					_starGrid9.postInit();
					_starGrid9.setup(326, 1, 1);
					_starGrid9.setPosition(Common::Point(210, 128));
					_starGrid9.fixPriority(10);

					_starGrid11.postInit();
					_starGrid11.setup(326, 1, 1);
					_starGrid11.setPosition(Common::Point(210, 150));
					_starGrid11.fixPriority(10);
				}
			} else if (R2_GLOBALS.getFlag(51)) {
				setMessage(329, (v == 12) ? 10 : v);
			} else {
				setMessage(327, (v >= 15) ? 1 : v);
			}
			break;
		}
		case 21:
			_sceneMode = 129;

			_starGrid1.postInit();
			_starGrid1.setup(327, 1, 1);
			_starGrid1.setPosition(Common::Point(170, 80));
			_starGrid1.fixPriority(10);
			_starGrid1.animate(ANIM_MODE_5, NULL);
			break;
		case 22:
			_sceneMode = 129;

			_starGrid1.postInit();
			_starGrid1.setup(327, 2, 1);
			_starGrid1.setPosition(Common::Point(160, 80));
			_starGrid1.fixPriority(10);
			_starGrid1.animate(ANIM_MODE_5, NULL);
			break;
		case 24:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databasePage = 37;
			setMessage(128, _databasePage);
			break;
		case 25:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databasePage = 68;
			setMessage(128, _databasePage);
			break;
		case 26:
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
			_databasePage = 105;
			setMessage(128, _databasePage);
			break;
		default:
			_databasePage = 0;
			setMessage(128, _databasePage);
			break;
		}

		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 10:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;

		if ((_consoleAction >= 17) && (_consoleAction <= 20)) {
			_icon5.setIcon(8);
			consoleAction(4);
		} else {
			consoleAction(7);
		}

		_icon6.setIcon(7);
		break;
	case 11:
		R2_GLOBALS.setFlag(45);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 12:
		R2_GLOBALS.setFlag(57);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 14:
		if (_soundCount)
			--_soundCount;

		if (!_soundCount || !(R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_soundIndex = 0;
			R2_GLOBALS._playStream.stop();
		} else {
			_sceneMode = 15;
			R2_GLOBALS._playStream.play(_soundQueue[_soundIndex++], this);
		}
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene325::removeText() {
	_text1.remove();
	_soundCount = 0;
	_soundIndex = 0;
	R2_GLOBALS._playStream.stop();
}

void Scene325::consoleAction(int id) {
	_icon1.setIcon(0);
	_icon2.setIcon(0);
	_icon3.setIcon(0);
	_icon4.setIcon(0);

	if (id == 7)
		_icon5.setIcon(9);
	else if ((_consoleAction != 3) && ((_consoleAction < 17) || (_consoleAction > 26)))
		_icon5.setIcon(8);

	switch (id - 1) {
	case 0:
		_icon1.setIcon(10);
		_icon2.setIcon(11);
		break;
	case 1:
		// Database screen
		_icon1.setIcon(23);
		_icon2.setIcon(24);
		_icon3.setIcon(25);
		_icon4.setIcon(26);
		break;
	case 2:
	case 16:
	case 17:
	case 18:
	case 19:
	case 20:
	case 21:
		R2_GLOBALS._player.disableControl();
		consoleAction(7);
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();

		if (id == 2 || (id == 19 && _priorConsoleAction == 5 && R2_GLOBALS.getFlag(50) &&
				R2_GLOBALS.getFlag(44) && !R2_GLOBALS.getFlag(51))) {
			_icon5.setIcon(13);
			_icon4.setPosition(Common::Point(52, 107));
			_icon4._sceneRegionId = 9;
			_icon4.setIcon(14);
			_icon4._horizLine.hide();

		} else {
			_icon4.hideIcon();
			_icon5.hideIcon();
		}

		_icon6.setIcon(12);
		_sceneMode = 10;
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
		break;

	case 22:
	case 23:
	case 24:
	case 25:
		// Database sub-sections: A-G, N-O, P-S, T-Z
		R2_GLOBALS._player.disableControl();
		consoleAction(2);
		_consoleAction = id;
		_icon1.hideIcon();
		_icon2.hideIcon();
		_icon3.hideIcon();
		_icon4.hideIcon();

		_icon5.setIcon(13);
		_icon4.setPosition(Common::Point(52, 107));
		_icon4._sceneRegionId = 9;
		_icon4.setIcon(14);
		_icon4._horizLine.hide();

		_icon6.setIcon(12);
		_sceneMode = 10;
		_palette.loadPalette(161);
		R2_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
		break;

	case 11:
		if (R2_GLOBALS.getFlag(57) && (R2_GLOBALS._player._characterIndex == R2_QUINN) && !R2_GLOBALS.getFlag(25)) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_ARROW);
			_sceneMode = 13;
			_stripManager.start(403, this);
		} else {
			R2_GLOBALS._player.disableControl();
			id = 8;
			_text1.remove();

			_icon4.setPosition(Common::Point(80, 62));
			_icon4._sceneRegionId = 5;
			_icon4.hideIcon();

			R2_GLOBALS._player.hide();
			_starGrid1.remove();
			_starGrid2.remove();
			_starGrid3.remove();
			_starGrid4.remove();
			_starGrid5.remove();
			_starGrid6.remove();
			_starGrid7.remove();
			_starGrid8.remove();
			_starGrid9.remove();
			_starGrid10.remove();
			_starGrid11.remove();
			_starGrid12.remove();
			_starGrid13.remove();

			_palette.loadPalette(160);
			_sceneMode = 11;

			R2_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
		}
		break;

	case 3:
		_icon1.setIcon(5);
		_icon2.setIcon(6);
		_icon3.setIcon(R2_GLOBALS.getFlag(50) ? 16 : 15);
		break;
	case 4:
	case 5:
		_priorConsoleAction = id;
		_icon1.setIcon(17);
		_icon2.setIcon(18);
		_icon3.setIcon(19);
		_icon4.setIcon(20);
		break;
	case 7:
		consoleAction(((_consoleAction == 5) || (_consoleAction == 6) || (_consoleAction == 15)) ? 4 : 7);
		break;
	case 8:
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 9:
	case 10:
		// Set language: Interworld or Hero's Tongue
		_iconFontNumber = (id - 1) == 9 ? 50 : 52;
		_text1.remove();
		_icon6.setIcon(7);
		consoleAction(1);
		break;
	case 12:
		// Page up button
		_icon4.setIcon(14);
		_icon4._horizLine.hide();

		switch (_consoleAction) {
		case 17:
		case 18:
		case 19:
		case 20:
			if (_scannerLocation) {
				R2_GLOBALS._player.disableControl();
				_moveCounter = 1296;
				_yDirection = 1;
			}
			break;
		default:
			setMessage(128, --_databasePage);
			break;
		}
		return;
	case 13:
		// Page down button
		_icon4.setIcon(14);
		_icon4._horizLine.hide();

		switch (_consoleAction) {
		case 17:
		case 18:
		case 19:
		case 20:
			if (_scannerLocation < 1620) {
				R2_GLOBALS._player.disableControl();
				_moveCounter = 1296;
				_yDirection = -1;
			}
			break;
		default:
			setMessage(128, ++_databasePage);
			break;
		}
		return;
	case 14:
		if (R2_GLOBALS.getFlag(55)) {
			consoleAction(4);
			// Workaround for original game bug.
			// Empty message crashing the game. It should be a warning message forbidding to switch to active scan
			// SceneItem::display2(329, 17);
		} else {
			R2_GLOBALS.setFlag(50);
			consoleAction(4);
		}
		id = 4;
		break;
	case 15:
		R2_GLOBALS.clearFlag(50);
		consoleAction(4);
		id = 4;
		break;
	case 6:
	default:
		// Initial starting screen
		_icon1.setIcon(1);
		_icon2.setIcon(2);
		_icon3.setIcon(3);
		int idList[3] = { 4, 22, 21 };
		_icon4.setIcon(idList[R2_GLOBALS._player._characterIndex - 1]);
		break;
	}

	if (id != 8)
		_consoleAction = id;
}

void Scene325::process(Event &event) {
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

void Scene325::dispatch() {
	if (_moveCounter) {
		switch (_moveCounter) {
		case 13:
			_yChange = 1;
			break;
		case 1296:
			R2_GLOBALS._sound3.play(87);
			_yChange = 1;
			break;
		case 33:
		case 1283:
			_yChange = 2;
			break;
		case 63:
		case 1263:
			_yChange = 3;
			break;
		case 103:
		case 1233:
			_yChange = 4;
			break;
		case 153:
		case 1193:
			_yChange = 5;
			break;
		case 213:
		case 1143:
			_yChange = 6;
			break;
		case 283:
		case 1083:
			_yChange = 7;
			break;
		case 1013:
			_yChange = 8;
			break;
		default:
			break;
		}

		_moveCounter -= _yChange;
		int yp = _yDirection * _yChange + _starGrid10._position.y;
		bool flag = false;

		if (yp >= 30) {
			yp -= 12;
			--_scannerLocation;
			flag = true;
		}
		if (yp <= 10) {
			yp += 12;
			++_scannerLocation;
			flag = true;
		}
		_starGrid13.setPosition(Common::Point(149, 22 + (int)(_scannerLocation * ADJUST_FACTOR)));

		for (int idx = 0; idx < 4; ++idx)
			_objList[idx].remove();

		if (flag) {
			int v = _scannerLocation - 758;
			_starGrid10.setFrame((v++ <= 0) ? 1 : v);
			_starGrid1.setFrame((v++ <= 0) ? 1 : v);
			_starGrid2.setFrame((v++ <= 0) ? 1 : v);
			_starGrid3.setFrame((v++ <= 0) ? 1 : v);
			_starGrid4.setFrame((v++ <= 0) ? 1 : v);
			_starGrid5.setFrame((v++ <= 0) ? 1 : v);
			_starGrid6.setFrame((v++ <= 0) ? 1 : v);
			_starGrid7.setFrame((v++ <= 0) ? 1 : v);
			_starGrid8.setFrame((v++ <= 0) ? 1 : v);
			_starGrid9.setFrame((v++ <= 0) ? 1 : v);
			_starGrid11.setFrame((v++ <= 0) ? 1 : v);
		}

		_starGrid10.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid1.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid2.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid3.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid4.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid5.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid6.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid7.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid8.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid9.setPosition(Common::Point(210, yp));
		yp += 12;
		_starGrid11.setPosition(Common::Point(210, yp));

		if (!_moveCounter) {
			R2_GLOBALS._sound3.stop();
			_yChange = 0;

			if (_scannerLocation == 756) {
				R2_GLOBALS._player.disableControl();
				R2_GLOBALS._events.setCursor(CURSOR_USE);
				_sceneMode = 12;
				_stripManager.start(212, this);
			} else {
				R2_GLOBALS._player.enableControl();
				R2_GLOBALS._player._canWalk = false;
			}
		}
	}

	SceneExt::dispatch();
}

void Scene325::setMessage(int resNum, int lineNum) {
	removeText();
	Common::String msg = g_resourceManager->getMessage(resNum, lineNum, true);

	if (!msg.empty()) {
		// Found valid database entry to display
		Common::String msgText = parseMessage(msg);

		_text1._fontNumber = _iconFontNumber;
		_text1._color1 = 92;
		_text1._color2 = 0;
		_text1._width = 221;
		_text1.fixPriority(20);
		_text1.setup(msgText);
		_text1.setPosition(Common::Point(49, 19));

		R2_GLOBALS._sceneObjects->draw();

		if ((_soundCount != 0) && (R2_GLOBALS._speechSubtitles & SPEECH_VOICE)) {
			_sceneMode = 15;
			R2_GLOBALS._playStream.play(_soundQueue[_soundIndex++], this);
		}
	} else {
		// No message for given database index, so we must have passed beyond
		// the start or end of the database
		_consoleAction = 13;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();

		_icon4.setPosition(Common::Point(80, 62));
		_icon4._sceneRegionId = 5;
		_icon4.hideIcon();

		_palette.loadPalette(160);
		_sceneMode = 11;
		R2_GLOBALS._scenePalette.addFader(&_palette._palette[0], 256, 5, this);
	}
}

/**
 * Parses a message to be displayed on the console to see whether there are any sounds to be played.
 */
Common::String Scene325::parseMessage(const Common::String &msg) {
	_soundIndex = 0;
	_soundCount = 0;

	const char *msgP = msg.c_str();
	while (*msgP == '!') {
		// Get the sound number
		_soundQueue[_soundCount++] = atoi(++msgP);

		while (!((*msgP == '\0') || (*msgP < '0') || (*msgP > '9')))
			++msgP;
	}

	return Common::String(msgP);
}

/*--------------------------------------------------------------------------
 * Scene 400 - Science Lab
 *
 *--------------------------------------------------------------------------*/

bool Scene400::Terminal::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

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
	if (action == CURSOR_USE) {
		Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 401;
		scene->setAction(&scene->_sequenceManager1, scene, 401, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::Reader::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 405;
		scene->setAction(&scene->_sequenceManager1, scene, 405, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::SensorProbe::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 404;
		scene->setAction(&scene->_sequenceManager1, scene, 404, &R2_GLOBALS._player, this, NULL);

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene400::AttractorUnit::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene400 *scene = (Scene400 *)R2_GLOBALS._sceneManager._scene;

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
	loadScene(400);
	SceneExt::postInit();

	_sound1.play(20);

	_door.postInit();
	_door.setVisage(100);
	_door.setPosition(Common::Point(123, 84));
	_door.setDetails(400, 24, -1, 26, 1, (SceneItem *)NULL);

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
		_reader.setDetails(400, 54, -1, 56, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_SENSOR_PROBE) == 400) {
		_sensorProbe.postInit();
		_sensorProbe.setup(400, 5, 1);
		_sensorProbe.setPosition(Common::Point(251, 104));
		_sensorProbe.fixPriority(121);
		_sensorProbe.setDetails(400, 57, -1, 59, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_UNIT) == 400) {
		_attractorUnit.postInit();
		_attractorUnit.setup(400, 5, 3);
		_attractorUnit.setPosition(Common::Point(265, 129));
		_attractorUnit.setDetails(400, 60, -1, 62, 1, (SceneItem *)NULL);
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
 * Scene 500 - Lander Bay 2 Storage
 *
 *--------------------------------------------------------------------------*/

Scene500::PanelDialog::Button::Button() {
	_buttonId = 0;
	_buttonDown = false;
}

bool Scene500::ControlPanel::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(26)) {
			scene->_stripNumber = 1104;
			scene->_sceneMode = 524;
			scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 510;
			scene->setAction(&scene->_sequenceManager1, scene, 510, &R2_GLOBALS._player, NULL);
		}
		return true;
	} else {
		return SceneHotspot::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

bool Scene500::Seeker::startAction(CursorType action, Event &event) {
	if (action == CURSOR_TALK) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			scene->_stripNumber = R2_GLOBALS.getFlag(26) ? 1101 : 1103;
		} else {
			scene->_stripNumber = R2_GLOBALS.getFlag(26) ? 1102 : 1105;
		}

		scene->_sceneMode = 524;
		scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Suit::startAction(CursorType action, Event &event) {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(500, R2_GLOBALS.getFlag(28) ? 28 : _strip + 25);
		return true;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if ((_strip != 3) && (_strip != 7))
				SceneItem::display2(500, _strip + 25);
			else if (R2_GLOBALS.getFlag(26)) {
				R2_GLOBALS._player.disableControl();
				scene->_stripNumber = 1103;
				scene->_sceneMode = 524;
				scene->setAction(&scene->_sequenceManager1, scene, 512, &R2_GLOBALS._player, NULL);
			} else if (!R2_GLOBALS.getFlag(28))
				SceneItem::display2(500, 41);
			else if (!R2_GLOBALS.getFlag(25))
				SceneItem::display2(500, 40);
			else {
				R2_GLOBALS._player.disableControl();
				scene->_sceneMode = 512;
				scene->setAction(&scene->_sequenceManager1, scene, 512, &R2_GLOBALS._player, &scene->_suit, NULL);
				R2_GLOBALS.setFlag(26);
			}
		} else {
			SceneItem::display2(500, 42);
		}
		return true;

	case R2_REBREATHER_TANK:
		if (!R2_GLOBALS.getFlag(25))
			SceneItem::display2(500, 10);
		else if (_strip != 3)
			SceneItem::display2(500, _strip + 25);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 515;
			scene->setAction(&scene->_sequenceManager1, scene, 515, &R2_GLOBALS._player, &scene->_suit, NULL);
			R2_GLOBALS.setFlag(28);
		}
		return true;

	case R2_RESERVE_REBREATHER_TANK:
		SceneItem::display2(500, 53);
		return true;

	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Doorway::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(26)) {
			scene->_stripNumber = 1104;
			scene->_sceneMode = 524;
			scene->setAction(&scene->_sequenceManager1, scene, 524, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 500;
			scene->setAction(&scene->_sequenceManager1, scene, 500, &R2_GLOBALS._player, this, NULL);
		}

		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::OxygenTanks::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(500, R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) ? 50 : 49);
		return true;

	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex != R2_QUINN) {
			SceneItem::display2(500, 52);
			return true;
		} else if ((R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) != 1) &&
				(R2_GLOBALS._player._characterIndex != R2_SEEKER) && !R2_GLOBALS.getFlag(28)) {
			Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

			R2_GLOBALS._player.disableControl();

			if (_position.y == 120) {
				scene->_sceneMode = 513;
				scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
					&scene->_tanks1, NULL);
			} else {
				scene->_sceneMode = 514;
				scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
					&scene->_tanks2, NULL);
			}
			return true;
		}
		break;

	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene500::AirLock::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && R2_GLOBALS.getFlag(26)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = (R2_GLOBALS._player._characterIndex == R2_QUINN) ? 521 : 522;
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
			&scene->_seeker, &scene->_airLock, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene500::TransparentDoor::draw() {
	// Determine the area of the screen to be updated
	Rect destRect = _bounds;
	destRect.translate(-g_globals->_sceneManager._scene->_sceneBounds.left,
		-g_globals->_sceneManager._scene->_sceneBounds.top);

	// Get the frame to be drawn
	GfxSurface frame = getFrame();

	Graphics::Surface s = frame.lockSurface();
	Graphics::Surface screen = g_globals->gfxManager().getSurface().lockSurface();

	for (int yp = 0; yp < s.h; ++yp) {
		byte *frameSrcP = (byte *)s.getBasePtr(0, yp);
		byte *screenP = (byte *)screen.getBasePtr(destRect.left, destRect.top + yp);

		for (int xp = 0; xp < s.w; ++xp, ++frameSrcP, ++screenP) {
			if (*frameSrcP != frame._transColor && *frameSrcP < 6) {
				*frameSrcP = R2_GLOBALS._fadePaletteMap[*frameSrcP][*screenP];
			}
		}
	}

	// Finished updating the frame
	frame.unlockSurface();
	g_globals->gfxManager().getSurface().unlockSurface();

	// Draw the processed frame
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(_priority);
	g_globals->gfxManager().copyFrom(frame, destRect, priorityRegion);

}

bool Scene500::Aerosol::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 503;
		scene->setAction(&scene->_sequenceManager1, scene, 503, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::SonicStunner::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 520 : 502;
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Locker1::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(11))
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 517 : 505;
		else
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 516 : 504;

		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

bool Scene500::Locker2::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS.getFlag(12))
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 519 : 507;
		else
			scene->_sceneMode = R2_GLOBALS.getFlag(26) ? 518 : 506;

		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, this, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene500::PanelDialog::setDetails(int visage, int strip, int frameNumber,
		const Common::Point &pt) {
	SceneAreaObject::setDetails(visage, strip, frameNumber, pt);
	SceneAreaObject::setDetails(500, 43, 32, 45);

	_button1.setupButton(1);
	_button2.setupButton(2);
	_button3.setupButton(3);
}

void Scene500::PanelDialog::remove() {
	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneAreas.remove(&_button1);
	scene->_sceneAreas.remove(&_button2);
	scene->_sceneAreas.remove(&_button3);

	_button1.remove();
	_button2.remove();
	_button3.remove();

	SceneAreaObject::remove();

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 511;
	scene->setAction(&scene->_sequenceManager1, scene, 511, &R2_GLOBALS._player, NULL);
}

bool Scene500::PanelDialog::Button::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		return false;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene500::PanelDialog::Button::setupButton(int buttonId) {
	_buttonId = buttonId;
	_buttonDown = false;
	SceneActor::postInit();
	setup(500, 7, 1);
	fixPriority(251);

	switch (_buttonId) {
	case 1:
		setPosition(Common::Point(139, 78));
		break;
	case 2:
		setPosition(Common::Point(139, 96));
		break;
	case 3:
		setPosition(Common::Point(139, 114));
		break;
	default:
		break;
	}

	Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneAreas.push_front(this);
}

void Scene500::PanelDialog::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_buttonDown);
}

void Scene500::PanelDialog::Button::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) &&
			(R2_GLOBALS._events.getCursor() == CURSOR_USE) &&
			_bounds.contains(event.mousePos) && !_buttonDown) {
		_buttonDown = true;
		event.handled = true;
		setFrame(2);
	}

	if ((event.eventType == EVENT_BUTTON_UP) && _buttonDown) {
		setFrame(1);
		_buttonDown = false;
		event.handled = true;

		doButtonPress();
	}
}

void Scene500::PanelDialog::Button::doButtonPress() {
	if (R2_GLOBALS.getFlag(28)) {
		SceneItem::display2(500, 48);
	} else {
		Scene500 *scene = (Scene500 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = _buttonId;

		switch (_buttonId) {
		case 1:
			// Rotate Left
			if (--R2_GLOBALS._landerSuitNumber == 0)
				R2_GLOBALS._landerSuitNumber = R2_MIRANDA;

			if (R2_GLOBALS.getFlag(35)) {
				scene->_sceneMode = 5;
				scene->setAction(&scene->_sequenceManager1, scene, 509, &scene->_suits,
					&scene->_suit, &scene->_transparentDoor, NULL);
			} else {
				scene->_sound1.play(127);
				scene->_suits.animate(ANIM_MODE_6, scene);
			}
			break;

		case 2:
			// Rotate Right
			if (++R2_GLOBALS._landerSuitNumber == 4)
				R2_GLOBALS._landerSuitNumber = R2_QUINN;

			if (R2_GLOBALS.getFlag(35)) {
				scene->_sceneMode = 6;
				scene->setAction(&scene->_sequenceManager1, scene, 509, &scene->_suits,
					&scene->_suit, &scene->_transparentDoor, NULL);
			} else {
				scene->_sound1.play(127);
				scene->_suits.animate(ANIM_MODE_5, scene);
			}
			break;

		case 3:
			if (R2_GLOBALS.getFlag(35)) {
				scene->_sceneMode = 509;
				scene->setAction(&scene->_sequenceManager1, scene, 509, &scene->_suits,
					&scene->_suit, &scene->_transparentDoor, NULL);
			} else {
				scene->_suit.postInit();
				scene->_suit.hide();
				scene->_suit._effect = EFFECT_SHADED;
				scene->_suit.setDetails(500, -1, -1, -1, 2, (SceneItem *)NULL);
				scene->_suit.setup(502, R2_GLOBALS._landerSuitNumber + 2, 1);

				scene->setAction(&scene->_sequenceManager1, scene, 508,
					&R2_GLOBALS._player, &scene->_suits, &scene->_suit,
					&scene->_transparentDoor, NULL);
				R2_GLOBALS.setFlag(35);
			}
			break;

		default:
			break;
		}
	}
}

/*--------------------------------------------------------------------------*/

void Scene500::postInit(SceneObjectList *OwnerList) {
	loadScene(500);
	SceneExt::postInit();

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(50);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);

	if (R2_GLOBALS.getFlag(25)) {
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 500;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._walkRegions.enableRegion(1);

			_seeker.postInit();
			_seeker._effect = EFFECT_SHADED;
			_seeker.setup(1505, 1, 1);
			_seeker._moveDiff.x = 5;
			_seeker.setPosition(Common::Point(42, 151));
			_seeker.setDetails(500, 34, 35, 36, 1, (SceneItem *)NULL);
		} else if (R2_GLOBALS._player._characterScene[R2_QUINN] == 500) {
			_seeker.postInit();
			_seeker._effect = EFFECT_SHADED;
			_seeker.setup(R2_GLOBALS.getFlag(26) ? 1500 : 10, 1, 1);
			_seeker.setPosition(Common::Point(42, 151));

			R2_GLOBALS._walkRegions.enableRegion(1);
			R2_GLOBALS._walkRegions.enableRegion(2);
			R2_GLOBALS._walkRegions.enableRegion(3);

			_seeker.setDetails(500, 37, 38, -1, 1, (SceneItem *)NULL);
		}
	}

	if ((R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 500) && R2_GLOBALS.getFlag(27)) {
		_tanks1.postInit();
		_tanks1.setup(502, 7, 1);
		_tanks1.setPosition(Common::Point(281, 120));
		_tanks1.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
	} else {
		if (R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 500) {
			_tanks1.postInit();
			_tanks1.setup(502, 7, 1);
			_tanks1.setPosition(Common::Point(281, 120));
			_tanks1.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
		}

		_tanks2.postInit();
		_tanks2.setup(502, 7, 1);
		_tanks2.setPosition(Common::Point(286, 121));
		_tanks2.setDetails(500, -1, -1, -1, 1, (SceneItem *)NULL);
	}

	_doorway.postInit();
	_doorway.setup(501, 1, 1);
	_doorway.setPosition(Common::Point(132, 85));
	_doorway.setDetails(500, 15, -1, 17, 1, (SceneItem *)NULL);

	_airLock.postInit();
	_airLock.setup(501, 2, 1);
	_airLock.setPosition(Common::Point(41, 121));

	if (!R2_GLOBALS.getFlag(25))
		_airLock.setDetails(500, 6, -1, 10, 1, (SceneItem *)NULL);
	else if ((R2_GLOBALS._player._characterScene[R2_QUINN] != 500) ||
			(R2_GLOBALS._player._characterScene[R2_SEEKER] != 500))
		_airLock.setDetails(500, 6, -1, 40, 1, (SceneItem *)NULL);
	else
		_airLock.setDetails(500, 6, -1, 9, 1, (SceneItem *)NULL);

	_locker1.postInit();
	_locker1.setup(500, 3, R2_GLOBALS.getFlag(11) ? 6 : 1);
	_locker1.setPosition(Common::Point(220, 82));
	_locker1.setDetails(500, 27, -1, -1, 1, (SceneItem *)NULL);

	_locker2.postInit();
	_locker2.setup(500, 4, R2_GLOBALS.getFlag(12) ? 6 : 1);
	_locker2.setPosition(Common::Point(291, 98));
	_locker2.fixPriority(121);
	_locker2.setDetails(500, 27, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_AEROSOL) == 500) {
		_aerosol.postInit();
		_aerosol.setup(500, 5, 2);
		_aerosol.setPosition(Common::Point(286, 91));
		_aerosol.fixPriority(120);
		_aerosol.setDetails(500, 24, 25, 26, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_SONIC_STUNNER) == 500) {
		_sonicStunner.postInit();
		_sonicStunner.setup(500, 5, 1);
		_sonicStunner.setPosition(Common::Point(214, 76));
		_sonicStunner.setDetails(500, 21, 22, 23, 1, (SceneItem *)NULL);
	}

	_suits.postInit();
	_suits._effect = EFFECT_SHADED;
	_suits.setup(502, 1, 1);
	_suits.setPosition(Common::Point(258, 99));
	_suits.fixPriority(50);

	_transparentDoor.postInit();
	_transparentDoor.setPosition(Common::Point(250, 111));

	if (!R2_GLOBALS.getFlag(35)) {
		_transparentDoor.setup(501, 3, 1);
	} else {
		_transparentDoor.setup(500, 8, 7);

		_suit.postInit();
		_suit._effect = EFFECT_SHADED;
		_suit.setPosition(Common::Point(247, 52));
		_suit.setDetails(500, -1, -1, -1, 2, (SceneItem *)NULL);

		if (!R2_GLOBALS.getFlag(26)) {
			if (R2_GLOBALS.getFlag(28))
				_suit.setup(502, 7, 2);
			else
				_suit.setup(502, R2_GLOBALS._landerSuitNumber + 2, 7);
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_SEEKER)
		R2_GLOBALS._player._moveDiff.x = 5;

	_controlPanel.setDetails(Rect(175, 62, 191, 80), 500, 31, 32, 33, 1, (SceneItem *)NULL);
	_airlockCorridor.setDetails(Rect(13, 58, 70, 118), 500, 12, -1, -1, 1, (SceneItem *)NULL);
	_background.setDetails(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT), 500, 0, -1, -1, 1, (SceneItem *)NULL);

	if ((R2_GLOBALS._player._characterIndex == R2_QUINN) && (R2_GLOBALS._sceneManager._previousScene == 700)) {
		R2_GLOBALS._player.disableControl();
		_sceneMode = 501;
		setAction(&_sequenceManager1, this, 501, &R2_GLOBALS._player, &_doorway, NULL);
	} else {
		if (R2_GLOBALS._player._characterIndex != R2_QUINN) {
			R2_GLOBALS._player.setup(1505, 6, 1);
		} else {
			R2_GLOBALS._player.setup(R2_GLOBALS.getFlag(26) ? 1500 : 10, 6, 1);
		}

		R2_GLOBALS._player.setPosition(Common::Point(123, 135));
		R2_GLOBALS._player.enableControl();
	}
}

void Scene500::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	s.syncAsSint16LE(_stripNumber);
}

void Scene500::signal() {
	switch (_sceneMode) {
	case 3:
		R2_GLOBALS._player.enableControl();
		break;
	case 5:
		_sceneMode = 12;
		_sound1.play(127);
		_suits.animate(ANIM_MODE_6, this);

		R2_GLOBALS.clearFlag(35);
		_suit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 6:
		_sceneMode = 11;
		_sound1.play(127);
		_suits.animate(ANIM_MODE_5, this);

		R2_GLOBALS.clearFlag(35);
		_suit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 7:
		_sound1.play(126);
		_transparentDoor.animate(ANIM_MODE_6, this);

		R2_GLOBALS.clearFlag(35);
		_suit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 500:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 501:
		if (R2_GLOBALS._player._characterScene[R2_SEEKER] == 500) {
			_stripNumber = 1100;
			_sceneMode = 523;
			setAction(&_sequenceManager1, this, 523, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player.enableControl();
		}
		break;
	case 502:
	case 520:
		R2_INVENTORY.setObjectScene(R2_SONIC_STUNNER, 1);
		_sonicStunner.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 503:
		R2_INVENTORY.setObjectScene(R2_AEROSOL, 1);
		_aerosol.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 504:
	case 516:
		R2_GLOBALS.setFlag(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 505:
	case 517:
		R2_GLOBALS.clearFlag(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 506:
	case 518:
		R2_GLOBALS.setFlag(12);
		R2_GLOBALS._player.enableControl();
		break;
	case 507:
	case 519:
		R2_GLOBALS.clearFlag(12);
		R2_GLOBALS._player.enableControl();
		break;
	case 509:
		R2_GLOBALS.clearFlag(35);
		_suit.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 510:
		R2_GLOBALS._player.enableControl();
		_panelDialog.setDetails(500, 6, 1, Common::Point(160, 120));
		R2_GLOBALS._player.enableControl();
		break;
	case 513:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		_tanks1.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 514:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		R2_GLOBALS.setFlag(27);
		_tanks2.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 515:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 0);
		R2_GLOBALS.setFlag(28);
		R2_GLOBALS._player.enableControl();
		break;
	case 521:
	case 522:
		R2_GLOBALS._sceneManager.changeScene(525);
		break;
	case 523:
	case 524:
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_sceneMode = 8;
		_stripManager.start(_stripNumber, this);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}


/*--------------------------------------------------------------------------
 * Scene 525 - Cutscene - Walking in hall
 *
 *--------------------------------------------------------------------------*/

void Scene525::postInit(SceneObjectList *OwnerList) {
	loadScene(525);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	R2_GLOBALS._sound1.play(105);

	_actor1.postInit();
	_actor1._effect = EFFECT_SHADED;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	setAction(&_sequenceManager, this, 525, &R2_GLOBALS._player, &_actor1, NULL);
}

void Scene525::signal() {
	R2_GLOBALS._sceneManager.changeScene(1525);
}

/*--------------------------------------------------------------------------
 * Scene 600 - Drive Room
 *
 *--------------------------------------------------------------------------*/

bool Scene600::CompartmentHotspot::startAction(CursorType action, Event &event) {
	if ((action != R2_NEGATOR_GUN) || (!R2_GLOBALS.getFlag(5)) || (R2_GLOBALS.getFlag(8)))
		return SceneHotspot::startAction(action, event);

	SceneItem::display(600, 32, SET_WIDTH, 280, SET_X, 160, SET_POS_MODE, ALIGN_CENTER,
		SET_Y, 20, SET_EXT_BGCOLOR, 7, LIST_END);
	return true;
}

bool Scene600::EngineCompartment::startAction(CursorType action, Event &event) {
	if ((action != R2_NEGATOR_GUN) || (!R2_GLOBALS.getFlag(1)))
		return SceneHotspot::startAction(action, event);

	if ((R2_GLOBALS.getFlag(5)) && (!R2_GLOBALS.getFlag(8))) {
		SceneItem::display(600, 32, SET_WIDTH, 280, SET_X, 160, SET_POS_MODE, ALIGN_CENTER,
			SET_Y, 20, SET_EXT_BGCOLOR, 7, LIST_END);
		return true;
	}

	if (!R2_GLOBALS.getFlag(5)) {
		SceneItem::display(600, 30, SET_WIDTH, 280, SET_X, 160, SET_POS_MODE, ALIGN_CENTER,
			SET_Y, 20, SET_EXT_BGCOLOR, 7, LIST_END);
		return true;
	}

	if ((!R2_GLOBALS.getFlag(8)) || (R2_GLOBALS.getFlag(9)))
		return SceneHotspot::startAction(action, event);

	R2_GLOBALS._player.disableControl();

	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	scene->_stasisArea.setup(603, 3, 1, 239, 54, 10);
	scene->_stasisField.postInit();
	scene->_computer.postInit();

	scene->_sceneMode = 612;
	scene->setAction(&scene->_sequenceManager1, scene, 612, &scene->_stasisField, &scene->_computer, &R2_GLOBALS._player, NULL);

	// WORKAROUND: For ScummVM, we use a SceneActor rather than BackgroundSceneObject
	// for the stasis field since it doesn't work properly. We override the priority for
	// the stasis field here so that the stasis field dissolve will show up
	scene->_stasisField.fixPriority(12);

	return true;
}

void Scene600::Smoke::signal() {
	Common::Point pt(177 + R2_GLOBALS._randomSource.getRandomNumber(5),
		108 + R2_GLOBALS._randomSource.getRandomNumber(3));
	NpcMover *mover = new NpcMover();
	addMover(mover, &pt, this);
}

bool Scene600::Smoke::startAction(CursorType action, Event &event) {
	if (action >= CURSOR_WALK)
	// Only action cursors
		return SceneActor::startAction(action, event);

	return false;
}

void Scene600::Smoke::draw() {
	// Effect should always be active on smoke, but since the original had this
	// check, include it here too
	if (_effect == EFFECT_NONE) {
		SceneActor::draw();
		return;
	}

	// Determine the area of the screen to be updated
	Rect destRect = _bounds;
	destRect.translate(-g_globals->_sceneManager._scene->_sceneBounds.left,
		-g_globals->_sceneManager._scene->_sceneBounds.top);

	// Get the smoke frame, screen reference, and pixel palette translation map
	GfxSurface frame = getFrame();
	Graphics::Surface s = frame.lockSurface();
	Graphics::Surface screen = g_globals->gfxManager().getSurface().lockSurface();
	byte *pixelMap = static_cast<Scene600 *>(R2_GLOBALS._sceneManager._scene)->_pixelMap;

	// Loop through every pixel of the frame. Any pixel of the frame that's not a
	// tranparency, get the same pixel from the screen background, and shade it using
	// the scene's pixel translation map
	for (int yp = 0; yp < s.h; ++yp) {
		byte *frameSrcP = (byte *)s.getBasePtr(0, yp);
		byte *screenP = (byte *)screen.getBasePtr(destRect.left, destRect.top + yp);

		for (int xp = 0; xp < s.w; ++xp, ++frameSrcP, ++screenP) {
			if (*frameSrcP != frame._transColor) {
				*frameSrcP = pixelMap[*screenP];
			}
		}
	}

	// Finished updating the frame
	frame.unlockSurface();
	g_globals->gfxManager().getSurface().unlockSurface();

	// Draw the processed frame
	Region *priorityRegion = g_globals->_sceneManager._scene->_priorities.find(_priority);
	g_globals->gfxManager().copyFrom(frame, destRect, priorityRegion);

}

bool Scene600::Doorway::startAction(CursorType action, Event &event) {
	// Only action cursors
	if (action < CURSOR_WALK)
		return false;

	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((R2_INVENTORY.getObjectScene(R2_CLAMP) == 600) && (!R2_GLOBALS.getFlag(6))) {
		R2_GLOBALS._player.disableControl();
		scene->_laser.setDetails(600, 11, -1, -1, 3, (SceneItem *) NULL);
		R2_GLOBALS.setFlag(6);
		scene->_sceneMode = 609;
		scene->setAction(&scene->_sequenceManager1, scene, 609, &R2_GLOBALS._player, &scene->_doorway, &scene->_laser, &scene->_laserBeam, NULL);
		return true;
	}

	if (_frame != 1)
		return false;

	if (!R2_GLOBALS.getFlag(6)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 616;
		scene->setAction(&scene->_sequenceManager1, scene, 616, &R2_GLOBALS._player, &scene->_doorway, &scene->_laser, NULL);
		return true;
	}

	if ((R2_GLOBALS.getFlag(9)) && (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600))
		SceneItem::display(600, 31, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
	else {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 601;
		scene->setAction(&scene->_sequenceManager1, scene, 601, &R2_GLOBALS._player, &scene->_doorway, NULL);
	}
	return true;
}

bool Scene600::Laser::startAction(CursorType action, Event &event) {
	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if (action < CURSOR_WALK) {
		switch (action) {
		case R2_COM_SCANNER:
			// If laser is destroyed
			if (R2_GLOBALS.getFlag(6)) {
				if (R2_GLOBALS.getFlag(8)) {
					SceneItem::display(600, 29, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
					return true;
				} else {
					R2_GLOBALS._player.disableControl();
					scene->_scanner.postInit();
					scene->_scanner.setDetails(600, 20, -1, -1, 4, &scene->_laser);
					scene->_sceneMode = 607;
					scene->setAction(&scene->_sequenceManager1, scene, 607, &R2_GLOBALS._player, &scene->_scanner, NULL);
					return true;
				}
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		case R2_AEROSOL:
			if (R2_GLOBALS.getFlag(5)) {
				SceneItem::display(600, 28, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				return true;
			} else {
				R2_GLOBALS._player.disableControl();
				scene->_aerosol.postInit();
				scene->_aerosol.setDetails(600, 27, -1, -1, 5, &scene->_laser);

				scene->_smoke.postInit();
				scene->_smoke.setup(601, 3, 1);
				scene->_smoke._effect = EFFECT_SMOKE;
				scene->_smoke._moveDiff = Common::Point(1, 1);
				scene->_smoke._moveRate = 2;
				scene->_smoke._numFrames = 3;
				scene->_smoke.setDetails(600, 24, 25, 26, 5, &scene->_aerosol);

				scene->_sceneMode = 605;

				scene->setAction(&scene->_sequenceManager1, scene, 605, &R2_GLOBALS._player, &scene->_aerosol, &scene->_smoke, &scene->_doorway, NULL);
				return true;
			}
			break;
		case R2_CLAMP:
			// If cloud is active
			if (R2_GLOBALS.getFlag(5)) {
				R2_GLOBALS._player.disableControl();
				scene->_sceneMode = 606;
				scene->setAction(&scene->_sequenceManager1, scene, 606, &R2_GLOBALS._player, &scene->_laser, NULL);
				return true;
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		default:
			return false;
			break;
		}
	} else if (action == CURSOR_USE) {
		if (R2_GLOBALS.getFlag(5)) {
			return SceneActor::startAction(action, event);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 610;
			scene->setAction(&scene->_sequenceManager1, scene, 610, &scene->_laserBeam, &R2_GLOBALS._player, NULL);
			return true;
		}
	} else
		return SceneActor::startAction(action, event);
}

bool Scene600::Aerosol::startAction(CursorType action, Event &event) {
	// Only action cursors
	if (action < CURSOR_WALK)
		return false;

	if (action == CURSOR_USE) {
		Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 614;
		scene->setAction(&scene->_sequenceManager1, scene, 614, &R2_GLOBALS._player, &scene->_aerosol, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

Scene600::Scene600() {
	_roomState = 0;
	Common::fill(&_pixelMap[0], &_pixelMap[256], 0);
}

void Scene600::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_roomState);
	for (int i = 0; i < 256; i++)
		s.syncAsByte(_pixelMap[i]);
}

bool Scene600::Scanner::startAction(CursorType action, Event &event) {
	Scene600 *scene = (Scene600 *)R2_GLOBALS._sceneManager._scene;

	if ((action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600)) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 615;
		scene->setAction(&scene->_sequenceManager1, scene, 615, &R2_GLOBALS._player, &scene->_scanner, NULL);
	} else if ((action == R2_SONIC_STUNNER) && (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600) && (R2_GLOBALS._scannerFrequencies[1] == 2) && (!R2_GLOBALS.getFlag(8))){
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 608;
		scene->setAction(&scene->_sequenceManager1, scene, 608, &R2_GLOBALS._player, &scene->_smoke, NULL);
	} else {
		return SceneActor::startAction(action, event);
	}

	return true;
}

void Scene600::postInit(SceneObjectList *OwnerList) {
	loadScene(600);
	SceneExt::postInit();
	R2_GLOBALS.setFlag(39);
	R2_GLOBALS._walkRegions.disableRegion(3);
	_roomState = 0;

	// Initialize pixel map for the obscuring effect
	ScenePalette &pal = R2_GLOBALS._scenePalette;
	uint r, g, b;
	for (int i = 0; i < 256; ++i) {
		pal.getEntry(i, &r, &g, &b);
		int av = ((r + g + b) / 48);

		_pixelMap[i] = R2_GLOBALS._paletteMap[(av << 8) | (av << 4) | av];
	}

	_doorway.postInit();
	_doorway.setVisage(600);
	_doorway.setPosition(Common::Point(29, 147));
	_doorway.fixPriority(10);
	_doorway.setDetails(300, 3, -1, -1, 1, (SceneItem *) NULL);

	_laser.postInit();
	_laser.setPosition(Common::Point(246, 41));

	if (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600) {
		_scanner.postInit();
		_scanner.setup(602, 5, 1);
		_scanner.setPosition(Common::Point(246, 41));
		_scanner.setDetails(600, 20, -1, -1, 1, (SceneItem *) NULL);
		switch (R2_GLOBALS._scannerFrequencies[1] - 2) {
		case 0:
			R2_GLOBALS._sound4.play(45);
			break;
		case 1:
			R2_GLOBALS._sound4.play(4);
			break;
		case 2:
			R2_GLOBALS._sound4.play(5);
			break;
		case 3:
			R2_GLOBALS._sound4.play(6);
			break;
		default:
			break;
		}
	}

	if (R2_GLOBALS.getFlag(6)) {
		_laser.setup(602, 7, 1);
		_laser.setDetails(600, 11, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_laser.setup(600, 2, 1);
		_laser.setDetails(600, 10, -1, -1, 1, (SceneItem *) NULL);

		_laserBeam.postInit();
		_laserBeam.setup(600, 3, 5);
		_laserBeam.setPosition(Common::Point(223, 51));
		_laserBeam.fixPriority(200);
	}

	if (! R2_GLOBALS.getFlag(9))
		_stasisArea.setup(603, 1, 1, 244, 50, 10);

	if (R2_GLOBALS.getFlag(5)) {
		if (R2_INVENTORY.getObjectScene(R2_AEROSOL) == 600) {
			_aerosol.postInit();
			_aerosol.setup(602, 2, 2);
			_aerosol.setPosition(Common::Point(189, 95));
			_aerosol.setDetails(600, 27, -1, -1, 1, (SceneItem *) NULL);
		}

		if (R2_GLOBALS.getFlag(8)) {
			if (R2_GLOBALS.getFlag(9)) {
				// Computer is active
				_computer.postInit();
				_computer.setup(603, 2, 1);
				_computer.setPosition(Common::Point(233, 45));
				_computer.animate(ANIM_MODE_2, NULL);
				_computer.fixPriority(11);
			}
		} else {
			_smoke.postInit();
			_smoke.setup(601, 1, 1);
			_smoke.setPosition(Common::Point(180, 110));
			_smoke._moveDiff = Common::Point(1, 1);
			_smoke._moveRate = 2;
			_smoke._numFrames = 3;
			_smoke.animate(ANIM_MODE_2, NULL);
			_smoke.fixPriority(130);
			_smoke._effect = EFFECT_SMOKE;
			_smoke.setDetails(600, 24, 25, 26, 1, (SceneItem *) NULL);
			_smoke.signal();
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_quantumRegulator.setDetails(12, 600, 17, -1, 19);
	_powerNode.setDetails(11, 600, 14, -1, -1);

	if (R2_GLOBALS.getFlag(9)) {
		_quantumDrive.setDetails(Rect(159, 3, 315, 95), 600, 7, -1, -1, 1, NULL);
	} else {
		_engineCompartment.setDetails(Rect(173, 15, 315, 45), 600, 21, -1, 23, 1, NULL);
		_quantumDrive.setDetails(Rect(159, 3, 315, 95), 600, 6, -1, -1, 1, NULL);
	}
	_background.setDetails(Rect(0, 0, 320, 200), 600, 0, -1, -1, 1, NULL);

	_sceneMode = 600;
	if (R2_GLOBALS._sceneManager._previousScene == 700) {
		if (R2_GLOBALS.getFlag(6)) {
			setAction(&_sequenceManager1, this, 600, &R2_GLOBALS._player, &_doorway, NULL);
		} else if (R2_GLOBALS.getFlag(5)) {
			setAction(&_sequenceManager1, this, 603, &R2_GLOBALS._player, &_doorway, &_laser, &_laserBeam, NULL);
		} else {
			setAction(&_sequenceManager1, this, 602, &R2_GLOBALS._player, &_doorway, &_laser, &_laserBeam, NULL);
		}
	} else if (R2_GLOBALS.getFlag(5)) {
		R2_GLOBALS._player.setPosition(Common::Point(50, 140));
		R2_GLOBALS._player.setStrip(3);
		_laser.setFrame(_laser.getFrameCount());
		signal();
	} else {
		_doorway.setFrame(7);
		_laser.setFrame(7);
		R2_GLOBALS._player.setPosition(Common::Point(28, 140));
		R2_GLOBALS._player.setStrip(5);
		signal();
	}
}

void Scene600::remove() {
	if (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 600)
		R2_GLOBALS._sound4.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene600::signal() {
	switch (_sceneMode) {
	case 601:
	// No break on purpose
	case 613:
	// No break on purpose
	case 616:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 605:
	// After cloud is active
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(7);
		R2_GLOBALS._walkRegions.disableRegion(9);
		R2_GLOBALS._walkRegions.disableRegion(10);

		R2_INVENTORY.setObjectScene(R2_AEROSOL, 600);
		R2_GLOBALS.setFlag(5);

		_smoke._effect = EFFECT_SMOKE;
		_smoke.signal();
		break;
	case 606:
	// After Clamp is put on laser
		R2_INVENTORY.setObjectScene(R2_CLAMP, 600);
		R2_GLOBALS._player.enableControl();
		break;
	case 607:
	// After scanner is put on laser
		R2_INVENTORY.setObjectScene(R2_COM_SCANNER, 600);
		R2_GLOBALS._player.enableControl();
		break;
	case 608:
	// deactivate cloud
		R2_GLOBALS.setFlag(8);
		_smoke.remove();
		R2_GLOBALS._walkRegions.enableRegion(6);
		R2_GLOBALS._walkRegions.enableRegion(9);
		R2_GLOBALS._walkRegions.enableRegion(10);
		R2_GLOBALS._player.enableControl();
		break;
	case 612:
	// Deactivate stasis field
		R2_GLOBALS.setFlag(9);
		_stasisField.remove();
		R2_GLOBALS._sceneItems.remove(&_engineCompartment);
		_computer.setDetails(600, 21, -1, 23, 4, &_engineCompartment);
		_engineCompartment.setDetails(600, 7, -1, -1, 3, (SceneItem *) NULL);
		_quantumDrive._lookLineNum = 7;
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	case 614:
	// Pick up Aerosol
		R2_GLOBALS._player.enableControl();
		_aerosol.remove();
		R2_INVENTORY.setObjectScene(R2_AEROSOL, 1);
		R2_GLOBALS._walkRegions.enableRegion(7);
		break;
	case 615:
	// Pick up Com Scanner
		_scanner.remove();
		R2_INVENTORY.setObjectScene(R2_COM_SCANNER, 1);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		_roomState = 0;
		_sceneMode = 0;
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene600::process(Event &event) {
	if (R2_GLOBALS._player._canWalk && (!R2_GLOBALS.getFlag(6)) && (event.eventType == EVENT_BUTTON_DOWN)
			&& (R2_GLOBALS._events.getCursor() == CURSOR_WALK)) {
		if (!_doorway.contains(event.mousePos) || (_doorway._frame <= 1)) {
			if (R2_GLOBALS.getFlag(5)) {
				_roomState += 10;
			} else {
				R2_GLOBALS._player.disableControl();
				_sceneMode = 604;
				setAction(&_sequenceManager1, this, 604, &_laserBeam, &R2_GLOBALS._player, NULL);
				event.handled = true;
			}
		} else {
			R2_GLOBALS._player.disableControl();
			_sceneMode = 613;
			setAction(&_sequenceManager1, this, 613, &R2_GLOBALS._player, &_laser, NULL);
			event.handled = true;
		}
	} else if ((!R2_GLOBALS.getFlag(6)) && (R2_GLOBALS._player._mover) && (_roomState < 10)){
		_roomState += 10;
	}

	Scene::process(event);
}

void Scene600::dispatch() {
	if ((_roomState != 0) && (_sceneMode != 600) && (_sceneMode != 603) && (_sceneMode != 602)) {
		if ( ((_laser._strip == 4) && (_laser._frame > 1))
		  ||  (_sceneMode == 601)
		  || ((_sceneMode == 616) && (_doorway._frame > 1)) )
		  _roomState = 0;
		else {
			_roomState--;
			if (_roomState % 10 == 0) {
				_laserBeam.setAction(&_sequenceManager2, NULL, 611, &_laserBeam, NULL);
			}
			if ((_roomState == 0) && (R2_GLOBALS._player._mover))
				_roomState = 10;
		}
	}

	if (_laserBeam._frame == 2)
		_sound1.play(40);

	Scene::dispatch();
	if ((_smoke._strip == 3) && (_smoke._frame == 3)) {
		_laserBeam.setStrip(4);
		_laserBeam.setFrame(1);
	}
}

/*--------------------------------------------------------------------------
 * Scene 700 - Lander Bay 2
 *
 *--------------------------------------------------------------------------*/

#define CABLE700_X 26
#define CABLE700_Y -5

Scene700::Scene700() {
	_rotation = NULL;
}

void Scene700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	SYNC_POINTER(_rotation);
}

bool Scene700::Loft::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._position.x < 100))
		return false;

	return NamedHotspot::startAction(action, event);
}

bool Scene700::HandGrip::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_CABLE_HARNESS:
		R2_GLOBALS._player.disableControl();
		scene->_cable.postInit();
		scene->_cable.setup(701, 3, 2);
		scene->_cable.setPosition(Common::Point(243, 98));
		scene->_cable.setDetails(700, 37, -1, -1, 2, (SceneItem *) NULL);
		scene->_cable.hide();
		scene->_sceneMode = 20;
		break;
	case R2_ATTRACTOR_CABLE_HARNESS:
		R2_GLOBALS._player.disableControl();
		scene->_cable.postInit();
		scene->_cable.setup(701, 2, 8);
		scene->_cable.setPosition(Common::Point(243, 98));
		scene->_cable.setDetails(700, 38, -1, -1, 2, (SceneItem *) NULL);
		scene->_cable.hide();
		scene->_sceneMode = 21;
		break;
	default:
		return NamedHotspot::startAction(action, event);
		break;
	}

	scene->setAction(&scene->_sequenceManager, scene, 707, &R2_GLOBALS._player, &scene->_cable, NULL);
	return true;
}

bool Scene700::LiftDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 701;
	scene->setAction(&scene->_sequenceManager, scene, 701, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::SuitRoomDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 702;
	scene->setAction(&scene->_sequenceManager, scene, 702, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::ControlPanel::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._position.y <= 100)
		return false;

	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 704;
	scene->setAction(&scene->_sequenceManager, scene, 704, &R2_GLOBALS._player, this, NULL);

	return true;
}

bool Scene700::Cable::startAction(CursorType action, Event &event) {
	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
		case 0:
			if ((_strip == 2) && (_frame == 1)) {
				R2_GLOBALS._player.disableControl();
				if (R2_GLOBALS._player._position.y <= 100) {
					scene->_sceneMode = 710;
					scene->setAction(&scene->_sequenceManager, scene, 710, &R2_GLOBALS._player, this, NULL);
				} else {
					scene->_sceneMode = 709;
					scene->setAction(&scene->_sequenceManager, scene, 709, &R2_GLOBALS._player, this, NULL);
				}
			} else {
				return SceneActor::startAction(action, event);
			}
			break;
		case 700: {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 10;
			Common::Point pt(_position.x - 12, _position.y + 1);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			}
			break;
		default:
			break;
		}
		break;
	case R2_ATTRACTOR_UNIT:
		R2_GLOBALS._player.disableControl();
		if (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) != 700) {
			scene->_sceneMode = 706;
			scene->setAction(&scene->_sequenceManager, scene, 706, &R2_GLOBALS._player, &scene->_cable, NULL);
		} else {
			scene->_sceneMode = 15;
			Common::Point pt(_position.x - 12, _position.y + 1);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
		}
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}

	return true;
}

bool Scene700::LoftDoor::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (R2_GLOBALS._player._position.y >= 100))
		return SceneActor::startAction(action, event);

	Scene700 *scene = (Scene700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 1;
	Common::Point pt(_position.x, 69);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

void Scene700::postInit(SceneObjectList *OwnerList) {
	loadScene(700);
	if (R2_GLOBALS._sceneManager._previousScene == 900)
		_sceneBounds = Rect(160, 0, 480, 200);
	SceneExt::postInit();

	_rotation = R2_GLOBALS._scenePalette.addRotation(237, 246, -1);
	_rotation->setDelay(5);
	_rotation->_countdown = 1;

	_liftDoor.postInit();
	_liftDoor.setVisage(700);
	_liftDoor.setPosition(Common::Point(21, 128));
	_liftDoor.fixPriority(10);
	_liftDoor.setDetails(700, 3, -1, -1, 1, (SceneItem *) NULL);

	_suitRoomDoor.postInit();
	_suitRoomDoor.setup(700, 2, 1);
	_suitRoomDoor.setPosition(Common::Point(217, 120));
	_suitRoomDoor.fixPriority(10);
	_suitRoomDoor.setDetails(700, 15, -1, -1, 1, (SceneItem *) NULL);

	_electromagnet.postInit();
	_electromagnet.setup(700, 4, 1);
	_electromagnet.setPosition(Common::Point(355 - ((R2_GLOBALS._electromagnetZoom * 8) / 5), ((R2_GLOBALS._electromagnetChangeAmount + 20 ) / 5) - 12));
	_electromagnet.fixPriority(10);
	_electromagnet.setDetails(700, 12, -1, 14, 1, (SceneItem *) NULL);

	_loftDoor1.postInit();
	_loftDoor1.setup(700, 8, 1);
	_loftDoor1.setPosition(Common::Point(85, 53));
	_loftDoor1.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_loftDoor2.postInit();
	_loftDoor2.setup(700, 8, 1);
	_loftDoor2.setPosition(Common::Point(164, 53));
	_loftDoor2.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_loftDoor3.postInit();
	_loftDoor3.setup(700, 8, 1);
	_loftDoor3.setPosition(Common::Point(243, 53));
	_loftDoor3.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	_loftDoor4.postInit();
	_loftDoor4.setup(700, 8, 1);
	_loftDoor4.setPosition(Common::Point(324, 53));
	_loftDoor4.setDetails(700, 33, -1, 35, 1, (SceneItem *) NULL);

	if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) != 1) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 1)) {
		_cable.postInit();
		_cable.fixPriority(10);
		switch (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS)) {
		case 0:
			switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
			case 0:
				_cable.setup(701, 3, 2);
				_cable.setPosition(Common::Point(243, 98));
				_cable.setDetails(700, 37, -1, -1, 1, (SceneItem *) NULL);
				break;
			case 700:
				_cable.setup(701, 3, 1);
				_cable.setPosition(Common::Point(356 - (CABLE700_X * 8), 148 - (((CABLE700_Y + 10) / 5) * 4)));
				_cable.setDetails(700, 37, -1, -1, 1, (SceneItem *) NULL);
				break;
			default:
				break;
			}
			break;
		case 700:
			switch (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS)) {
			case 0:
				if (R2_GLOBALS._tractorField && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70))
					_cable.setup(701, 2, 1);
				else
					_cable.setup(701, 2, 8);
				_cable.setPosition(Common::Point(243, 98));
				_cable.fixPriority(77);
				_cable.setDetails(700, 38, -1, -1, 1, (SceneItem *) NULL);
				break;
			case 700:
				_cable.setup(701, 1, 8);
				if (!R2_GLOBALS._cableAttached) {
					_cable.setPosition(Common::Point(356 - (CABLE700_X * 8), 148 - (((CABLE700_Y + 10) / 5) * 4)));
				} else {
					_cable.setup(701, 1, 1);
					_cable.setPosition(Common::Point(_electromagnet._position.x + 1, _electromagnet._position.y + 120));
				}
				_cable.setDetails(700, 38, -1, -1, 1, (SceneItem *) NULL);
				break;
			default:
				break;
			}
		default:
			break;
		}
	}

	_controlPanel.postInit();
	_controlPanel.setup(700, 3, 1);
	_controlPanel.setPosition(Common::Point(454, 117));
	_controlPanel.setDetails(700, 27, -1, -1, 1, (SceneItem *) NULL);

	_handGrip.setDetails(Rect(234, 90, 252, 110), 700, 39, -1, -1, 1, NULL);
	_restraintCollar.setDetails(Rect(91, 158, 385, 167), 700, 6, -1, 8, 1, NULL);
	_debris1.setDetails(Rect(47, 115, 149, 124), 700, 40, -1, 41, 1, NULL);
	_debris2.setDetails(Rect(151, 108, 187, 124), 700, 40, -1, 41, 1, NULL);
	_debris3.setDetails(Rect(247, 108, 275, 124), 700, 40, -1, 41, 1, NULL);
	_debris4.setDetails(Rect(300, 105, 321, 124), 700, 40, -1, 41, 1, NULL);
	_storage2.setDetails(Rect(255, 74, 368, 115), 700, 9, -1, 11, 1, NULL);
	_storage1.setDetails(Rect(69, 74, 182, 115), 700, 9, -1, 11, 1, NULL);
	_stars.setDetails(Rect(370, 58, 475, 103), 700, 18, -1, -1, 1, NULL);
	_light.setDetails(Rect(17, 11, 393, 31), 700, 24, -1, -1, 1, NULL);
	_loft.setDetails(Rect(42, 32, 368, 66), 700, 30, -1, 32, 1, NULL);
	_background.setDetails(Rect(0, 0, 480, 200), 700, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(11);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff.x = 2;
	R2_GLOBALS._player.disableControl();

	R2_GLOBALS._sound1.play(34);

	_sceneMode = 700;

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 250:
		setAction(&_sequenceManager, this, 700, &R2_GLOBALS._player, &_liftDoor, NULL);
		break;
	case 500:
		setAction(&_sequenceManager, this, 703, &R2_GLOBALS._player, &_suitRoomDoor, NULL);
		break;
	case 600: {
		_sceneMode = 4;
		_loftDoor2.setFrame(5);
		R2_GLOBALS._player.setPosition(Common::Point(164, 74));
		R2_GLOBALS._player.setStrip2(3);
		Common::Point pt(164, 69);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 900:
		setAction(&_sequenceManager, this, 705, &R2_GLOBALS._player, &_controlPanel, NULL);
		break;
	default:
		if (R2_GLOBALS.getFlag(41))
			R2_GLOBALS._player.setPosition(Common::Point(107, 67));
		else
			R2_GLOBALS._player.setPosition(Common::Point(60, 140));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl(CURSOR_ARROW);
		break;
	}
}

void Scene700::remove() {
	R2_GLOBALS._sound1.play(10);

	SceneExt::remove();
}

void Scene700::signal() {
	switch (_sceneMode) {
	case 1:
		_sceneMode = 2;
		R2_GLOBALS._player.setStrip(4);
		if (R2_GLOBALS._player._position.x != 164) {
			SceneItem::display(700, 36, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			R2_GLOBALS._player.enableControl();
		} else {
			R2_GLOBALS._sound2.play(19);
			_loftDoor2.animate(ANIM_MODE_5, this);
		}
		break;
	case 2: {
		_sceneMode = 3;
		R2_GLOBALS._player.setStrip2(4);
		Common::Point pt(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 5);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		R2_GLOBALS._sceneManager.changeScene(600);
		break;
	case 4:
		_sceneMode = 5;
		R2_GLOBALS._player.setStrip2(-1);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._sound2.play(19);
		_loftDoor2.animate(ANIM_MODE_6, this);
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS.setFlag(41);
		break;
	case 10:
		_sceneMode = 11;
		R2_GLOBALS._player.setup(16, 7, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 11:
		_sceneMode = 12;
		_cable.remove();
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 12:
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 7;
		if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_UNIT) == 0) {
			R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 1);
			R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 0);
		} else {
			R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 1);
		}
		R2_GLOBALS._player.enableControl();
		break;
	case 15:
		_sceneMode = 16;
		R2_GLOBALS._player.setup(16, 7, 1);
		R2_GLOBALS._player.changeZoom(50);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 16:
		_sceneMode = 17;
		_cable.setup(701, 1, 8);
		_cable.setDetails(700, 38, -1, -1, 3, (SceneItem *) NULL);
		if (R2_GLOBALS._tractorField && (_cable._position.x == _electromagnet._position.x + 1) && (_cable._position.x == 148 - (((R2_GLOBALS._electromagnetChangeAmount + 10) / 5) * 4))) {
			_cable.animate(ANIM_MODE_6, NULL);
			Common::Point pt(_cable._position.x, _electromagnet._position.y + 120);
			NpcMover *mover = new NpcMover();
			_cable.addMover(mover, &pt, NULL);
			R2_GLOBALS._cableAttached = true;
		}
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 17:
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 0);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 7;
		R2_GLOBALS._player.enableControl();
		break;
	case 20:
		R2_INVENTORY.setObjectScene(R2_CABLE_HARNESS, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 21:
		_cable.fixPriority(77);
		if (R2_GLOBALS._tractorField && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70))
			_cable.animate(ANIM_MODE_6, NULL);

		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.enableControl();
		break;
	case 701:
		R2_GLOBALS._sceneManager.changeScene(250);
		break;
	case 702:
		R2_GLOBALS._sceneManager.changeScene(500);
		break;
	case 704:
		R2_GLOBALS._sceneManager.changeScene(900);
		break;
	case 706:
		_cable.setDetails(700, 38, -1, -1, 3, (SceneItem *) NULL);
		_cable.fixPriority(77);
		if (R2_GLOBALS._tractorField && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70))
			_cable.animate(ANIM_MODE_6, NULL);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_UNIT, 0);
		R2_INVENTORY.setObjectScene(R2_ATTRACTOR_CABLE_HARNESS, 700);
		R2_GLOBALS._player.enableControl();
		break;
	case 709:
		R2_GLOBALS.setFlag(41);
		R2_GLOBALS._player.enableControl();
		break;
	case 710:
		R2_GLOBALS.clearFlag(41);
		R2_GLOBALS._player.enableControl();
		break;
	case 5:
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 800 - Sick Bay
 *
 *--------------------------------------------------------------------------*/

bool Scene800::Button::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE) {
		return NamedHotspot::startAction(action, event);
	} else {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 802;
		scene->setAction(&scene->_sequenceManager1, scene, 802, &R2_GLOBALS._player, &scene->_autodocCover, NULL);
		return true;
	}
}

bool Scene800::CableJunction::startAction(CursorType action, Event &event) {
	if (action != R2_OPTICAL_FIBER) {
		return NamedHotspot::startAction(action, event);
	} else {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_opticalFiber.postInit();
		scene->_sceneMode = 803;

		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 813, &R2_GLOBALS._player, &scene->_opticalFiber, &scene->_reader, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 803, &R2_GLOBALS._player, &scene->_opticalFiber, NULL);

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
		_lookLineNum = 27;
		scene->_sceneMode = 809;

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) == 800)
			scene->setAction(&scene->_sequenceManager1, scene, 815, &R2_GLOBALS._player, &scene->_reader, &scene->_opticalFiber, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 809, &R2_GLOBALS._player, &scene->_reader, NULL);
		return true;
	case R2_READER:
		R2_GLOBALS._player.disableControl();
		scene->_reader.postInit();
		scene->_sceneMode = 804;

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) == 800) {
			scene->setAction(&scene->_sequenceManager1, scene, 814, &R2_GLOBALS._player,
				&scene->_reader, &scene->_opticalFiber, NULL);
		} else {
			scene->setAction(&scene->_sequenceManager1, scene, 804, &R2_GLOBALS._player,
				&scene->_reader, NULL);
		}
		return true;
	default:
		break;
	}

	return NamedHotspot::startAction(action, event);
}

/*--------------------------------------------------------------------------*/

bool Scene800::Door::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 801;
		scene->setAction(&scene->_sequenceManager1, scene, 801, &R2_GLOBALS._player, &scene->_door, NULL);
		return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Tray::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

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
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::ComScanner::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

		if (scene->_cabinet._frame == 1)
			return false;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 811;
		scene->setAction(&scene->_sequenceManager1, scene, 811, &R2_GLOBALS._player, &scene->_comScanner, NULL);
		return true;
		}
	case CURSOR_TALK:
		SceneItem::display2(800, 35);
		return true;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene800::Cabinet::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene800 *scene = (Scene800 *)R2_GLOBALS._sceneManager._scene;

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
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

/*--------------------------------------------------------------------------*/

void Scene800::postInit(SceneObjectList *OwnerList) {
	loadScene(800);
	SceneExt::postInit();

	_door.postInit();
	_door.setVisage(800);
	_door.setPosition(Common::Point(286, 108));
	_door.fixPriority(50);
	_door.setDetails(800, 3, -1, -1, 1, (SceneItem *)NULL);

	_autodocCover.postInit();
	_autodocCover.setup(800, 2, 1);
	_autodocCover.setPosition(Common::Point(119, 161));
	_autodocCover.setDetails(800, 6, 7, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) == 800) {
		_opticalFiber.postInit();
		if (R2_INVENTORY.getObjectScene(R2_READER) == 800)
			_opticalFiber.setup(800, 4, 1);
		else
			_opticalFiber.setup(800, 7, 2);

		_opticalFiber.setPosition(Common::Point(220, 124));
		_opticalFiber.fixPriority(140);
	}

	if (R2_INVENTORY.getObjectScene(R2_READER) == 800) {
		_reader.postInit();

		if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) == 800) {
			_opticalFiber.setup(800, 4, 1);
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
	_cabinet.setDetails(800, 41, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_COM_SCANNER) == 800) {
		_comScanner.postInit();
		_comScanner.setup(801, 2, 1);
		_comScanner.setPosition(Common::Point(174, 73));
		_comScanner.setDetails(800, 34, 35, -1, 1, (SceneItem *)NULL);
	}

	_tray.postInit();
	_tray.setup(800, R2_INVENTORY.getObjectScene(R2_OPTO_DISK) == 825 ? 6 : 5, 1);
	if (R2_GLOBALS.getFlag(10))
		_tray.setFrame(5);
	_tray.setPosition(Common::Point(203, 144));
	_tray.setDetails(800, 12, -1, 14, 1, (SceneItem *)NULL);

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
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBER, 800);
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
		R2_INVENTORY.setObjectScene(R2_COM_SCANNER, 1);
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
	_buttonDown = false;
}

void Scene825::Button::synchronize(Serializer &s) {
	SceneObject::synchronize(s);
	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_buttonDown);
}

void Scene825::Button::process(Event &event) {
	if (!event.handled) {
		Scene825 *scene = (Scene825 *)R2_GLOBALS._sceneManager._scene;

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

	setDetails(825, 6, 7, -1, 2, (SceneItem *)NULL);
}

void Scene825::Button::setText(int textId) {
	_buttonId = textId;
	_lookLineNum = textId;

	_sceneText.remove();
	if (textId != 0) {
		Scene825 *scene = (Scene825 *)R2_GLOBALS._sceneManager._scene;
		_sceneText.setup(scene->_autodocItems[textId - 1]);
	}
}

/*--------------------------------------------------------------------------*/

Scene825::Scene825(): SceneExt() {
	_menuId = _frame1 = _frame2 = 0;

	// Setup Autodoc items list
	_autodocItems[0] = MAIN_MENU;
	_autodocItems[1] = DIAGNOSIS;
	_autodocItems[2] = ADVANCED_PROCEDURES;
	_autodocItems[3] = VITAL_SIGNS;
	_autodocItems[4] = OPEN_DOOR;
	_autodocItems[5] = TREATMENTS;
	_autodocItems[6] = NO_MALADY_DETECTED;
	_autodocItems[7] = NO_TREATMENT_REQUIRED;
	_autodocItems[8] = ACCESS_CODE_REQUIRED;
	_autodocItems[9] = INVALID_ACCESS_CODE;
	_autodocItems[10] = FOREIGN_OBJECT_EXTRACTED;
}

void Scene825::postInit(SceneObjectList *OwnerList) {
	loadScene(825);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._effect = EFFECT_NONE;
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();


	_console.setDetails(1, 825, 3, 4, 5);
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
		_vertLine5.remove();
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
		_vertLine5.remove();
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
	if (R2_GLOBALS._sceneObjects->contains(&_vertLine4) &&
			((_vertLine4._frame == 1) || (_vertLine4._frame == 3)) &&
			(_vertLine4._frame != _frame1)) {
		_sound2.play(25);
	}

	if (R2_GLOBALS._sceneObjects->contains(&_vertLine1) &&
			(_vertLine1._frame == 3) && (_vertLine1._frame != _frame2)) {
		_sound3.play(26);
	}

	_frame1 = _vertLine4._frame;
	_frame2 = _vertLine1._frame;

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
			_vertLine5.postInit();
			_sceneMode = 825;
			setAction(&_sequenceManager1, this, 825, &R2_GLOBALS._player, &_vertLine5, NULL);
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
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) != 800)) {
					_sceneText.setPosition(Common::Point(116, 75));
					_sceneText.setup(ACCESS_CODE_REQUIRED);
				} else if (R2_INVENTORY.getObjectScene(R2_OPTO_DISK) != 800) {
					_sceneText.setPosition(Common::Point(115, 75));
					_sceneText.setup(NO_TREATMENT_REQUIRED);
				} else {
					_button6._buttonId = 5;
					_sceneMode = 827;
					_vertLine5.postInit();

					setAction(&_sequenceManager1, this, 827, &_vertLine5, NULL);
				}
			} else {
				R2_GLOBALS.setFlag(2);

				if ((R2_INVENTORY.getObjectScene(R2_READER) != 800) ||
						(R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) != 800)) {
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

			_vertLine1.postInit();
			_vertLine1.setup(826, 7, 1);
			_vertLine1.setPosition(Common::Point(112, 67));
			_vertLine1._numFrames = 1;
			_vertLine1.animate(ANIM_MODE_2);

			_vertLine2.postInit();
			_vertLine2.setup(826, 5, 1);
			_vertLine2.setPosition(Common::Point(158, 67));
			_vertLine2._numFrames = 5;
			_vertLine2.animate(ANIM_MODE_2);

			_vertLine3.postInit();
			_vertLine3.setup(826, 6, 1);
			_vertLine3.setPosition(Common::Point(206, 67));
			_vertLine3._numFrames = 1;
			_vertLine3.animate(ANIM_MODE_2);

			_vertLine4.postInit();
			_vertLine4.setup(826, 8, 1);
			_vertLine4.setPosition(Common::Point(158, 84));
			_vertLine4._numFrames = 1;
			_vertLine4.animate(ANIM_MODE_2);

			_vertLine5.postInit();
			_vertLine5.setup(826, 4, 1);
			_vertLine5.setPosition(Common::Point(161, 110));
			break;
		case 5:
			R2_GLOBALS._player.disableControl();
			if (_menuId == 4) {
				_menuId = 0;

				_vertLine1.remove();
				_vertLine2.remove();
				_vertLine3.remove();
				_vertLine4.remove();
				_vertLine5.remove();

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
	if ((action != CURSOR_USE) || (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) != 850))
		return NamedHotspot::startAction(action, event);
	else {
		Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 851;
		scene->setAction(&scene->_sequenceManager1, scene, 851, &R2_GLOBALS._player, &scene->_fiber, NULL);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

bool Scene850::LiftDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 202;
		scene->setAction(&scene->_sequenceManager1, scene, 202, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::SickBayDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 204;
		scene->setAction(&scene->_sequenceManager1, scene, 204, &R2_GLOBALS._player, this, NULL);
		return true;
	}
}

bool Scene850::Clamp::startAction(CursorType action, Event &event) {
	if (!R2_GLOBALS.getFlag(7))
		return false;
	else if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);
	else {
		Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_spark.postInit();
		scene->_sceneMode = 850;
		scene->setAction(&scene->_sequenceManager1, scene, 850, &R2_GLOBALS._player, this,
			&scene->_spark, NULL);
		return true;
	}
}

bool Scene850::Panel::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || R2_GLOBALS.getFlag(7))
		return SceneActor::startAction(action, event);
	else {
		Scene850 *scene = (Scene850 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 852;
		scene->setAction(&scene->_sequenceManager1, scene, 852, &R2_GLOBALS._player,
			this, &scene->_spark, NULL);
		return true;
	}
}

/*--------------------------------------------------------------------------*/

void Scene850::postInit(SceneObjectList *OwnerList) {
	loadScene(850);
	SceneExt::postInit();

	_liftDoor.postInit();
	_liftDoor.setup(850, 2, 1);
	_liftDoor.setPosition(Common::Point(188, 79));
	_liftDoor.setDetails(850, 3, -1, -1, 1, (SceneItem *)NULL);

	_sickBayDoor.postInit();
	_sickBayDoor.setup(850, 3, 1);
	_sickBayDoor.setPosition(Common::Point(62, 84));
	_sickBayDoor.setDetails(850, 9, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_CLAMP) == 850) {
		_clamp.postInit();
		_clamp.setup(850, 5, 1);
		_clamp.setPosition(Common::Point(242, 93));
		_clamp.fixPriority(81);
		_clamp.animate(ANIM_MODE_2, NULL);
		_clamp.setDetails(850, 27, -1, -1, 1, (SceneItem *)NULL);
	}

	_panel.postInit();
	_panel.setVisage(850);

	if (R2_GLOBALS.getFlag(7))
		_panel.setFrame(7);

	_panel.setPosition(Common::Point(232, 119));
	_panel.fixPriority(82);
	_panel.setDetails(850, 24, -1, -1, 1, (SceneItem *)NULL);

	if (R2_INVENTORY.getObjectScene(R2_OPTICAL_FIBER) == 850) {
		_fiber.postInit();
		_fiber.setup(850, 6, 1);
		_fiber.setPosition(Common::Point(280, 87));
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
		_spark.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 851:
		R2_INVENTORY.setObjectScene(R2_OPTICAL_FIBER, 1);
		_fiber.remove();
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

/*--------------------------------------------------------------------------
 * Scene 900 - Lander Bay 2 - Crane Controls
 *
 *--------------------------------------------------------------------------*/

Scene900::Button::Button() {
	_buttonId = 0;
}

void Scene900::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_buttonId);
}


void Scene900::Button::initButton(int buttonId) {
	_buttonId = buttonId;
	postInit();
	setDetails(900, -1, -1, -1, 2, (SceneItem *) NULL);
}

Scene900::Scene900() {
	_controlsScreenNumber = 0;
	_magnetChangeAmount.x = 0;
	_magnetChangeAmount.y = 0;
}

void Scene900::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_controlsScreenNumber);
	s.syncAsSint16LE(_magnetChangeAmount.x);
	s.syncAsSint16LE(_magnetChangeAmount.y);
}

bool Scene900::Button::startAction(CursorType action, Event &event) {
	Scene900 *scene = (Scene900 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_USE) {
		R2_GLOBALS._sound2.play(14);
		switch (_buttonId) {
		case 2:
			if (scene->_controlsScreenNumber == 1) {
				scene->_sceneMode = 2;
				scene->signal();
			} else if (scene->_controlsScreenNumber == 2) {
				if (!R2_GLOBALS._tractorField) {
					scene->_aSound1.play(30);
					setup(900, 3, 11);
					R2_GLOBALS._tractorField = true;
					if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS == 700)) && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70) && (scene->_cable._animateMode != ANIM_MODE_6)) {
						scene->_cable.animate(ANIM_MODE_6, NULL);
					} else {
						if (((scene->_electromagnet._percent * 49) / 100) + scene->_electromagnet._position.x == scene->_cable._position.x) {
							if (scene->_cable._position.x == 166 - (R2_GLOBALS._electromagnetZoom / 15)) {
								R2_GLOBALS._player.disableControl();
								scene->_sceneMode = 4;
								scene->_cable._moveDiff.y = (scene->_cable._position.y - (scene->_electromagnet._position.y + ((scene->_electromagnet._percent * 3) / 10) - 2)) / 9;
								Common::Point pt(scene->_electromagnet._position.x + ((scene->_electromagnet._percent * 49) / 100), scene->_electromagnet._position.y + ((scene->_electromagnet._percent * 3) / 10) - 2);
								NpcMover *mover = new NpcMover();
								scene->_cable.addMover(mover, &pt, this);
								scene->_cable.animate(ANIM_MODE_6, NULL);
							}
						}
					}
				} else {
					scene->_aSound1.play(53);
					setup(900, 3, 9);
					R2_GLOBALS._tractorField = false;

					if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (scene->_cable._frame < 8) && (scene->_cable._animateMode != ANIM_MODE_5)) {
							scene->_cable.animate(ANIM_MODE_5, NULL);
					} else if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 700) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (scene->_cable._frame < 8)) {
						R2_GLOBALS._cableAttached = false;
						if (scene->_cable._animateMode != 5) {
							R2_GLOBALS._player.disableControl();
							scene->_sceneMode = 5;
							scene->_cable.animate(ANIM_MODE_5, NULL);
							scene->_cable._moveDiff.y = (166 - scene->_cable._position.y) / 9;
							Common::Point pt(scene->_cable._position.x, 166 - (R2_GLOBALS._electromagnetZoom / 15));
							NpcMover *mover = new NpcMover();
							scene->_cable.addMover(mover, &pt, this);
						}
					}
				}
			}
			return true;
			break;
		case 3:
			if (scene->_controlsScreenNumber == 1) {
				scene->_sceneMode = 3;
				scene->signal();
			}
			return true;
			break;
		case 4:
			if ((scene->_magnetChangeAmount.y == 0) && (scene->_magnetChangeAmount.x == 0) && (R2_GLOBALS._electromagnetZoom != 0)) {
				scene->_aSound1.play(38);
				scene->_magnetChangeAmount.y = -5;
			}
			return true;
			break;
		case 5:
			if ((scene->_magnetChangeAmount.y == 0) && (scene->_magnetChangeAmount.x == 0) && (R2_GLOBALS._electromagnetZoom < 135)) {
				scene->_aSound1.play(38);
				scene->_magnetChangeAmount.y = 5;
			}
			return true;
			break;
		case 6:
			if ((scene->_magnetChangeAmount.y == 0) && (scene->_magnetChangeAmount.x == 0) && (R2_GLOBALS._electromagnetChangeAmount > -10)) {
				scene->_aSound1.play(38);
				scene->_magnetChangeAmount.x = -5;
			}
			return true;
			break;
		case 7:
			if ((scene->_magnetChangeAmount.y == 0) && (scene->_magnetChangeAmount.x == 0) && (R2_GLOBALS._electromagnetChangeAmount < 20)) {
				scene->_aSound1.play(38);
				scene->_magnetChangeAmount.x = 5;
			}
			return true;
			break;
		case 8:
			SceneItem::display(5, 11, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			return true;
			break;
		case 9:
			SceneItem::display(5, 12, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			return true;
			break;
		default:
			if (scene->_controlsScreenNumber == 1) {
				R2_GLOBALS._player.disableControl();
				scene->_button2.remove();
				scene->_button3.remove();
				scene->_button4.remove();
				scene->_button5.remove();
				scene->_button6.remove();
				scene->_button7.remove();
				R2_GLOBALS._sound2.play(37);
				scene->_sceneMode = 901;
				scene->setAction(&scene->_sequenceManager1, scene, 901, &scene->_controls, this ,NULL);
			} else if ((scene->_controlsScreenNumber == 2) || (scene->_controlsScreenNumber == 3)) {
				scene->_sceneMode = 1;
				scene->signal();
			}

			return true;
			break;
		}
	} else if (action == CURSOR_LOOK) {
		SceneItem::display(900, ((_buttonId == 2) && (scene->_controlsScreenNumber == 2)) ? 21 : _buttonId + 11,
			SET_WIDTH, 280, SET_X, 160,  SET_POS_MODE, 1, SET_Y, 20, SET_EXT_BGCOLOR, 7, LIST_END);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene900::postInit(SceneObjectList *OwnerList) {
	g_globals->gfxManager()._bounds.moveTo(Common::Point(0, 0));
	loadScene(900);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(34);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_controls.postInit();
	_controls.setDetails(900, 3, -1, -1, 1, (SceneItem *) NULL);

	_magnetChangeAmount.x = 0;
	_magnetChangeAmount.y = 0;

	_electromagnet.postInit();
	_electromagnet.fixPriority(1);
	_electromagnet.setup(900, 1, 2);
	_electromagnet.setPosition(Common::Point(89, 0));
	_electromagnet._effect = EFFECT_SHADED;
	_electromagnet.setDetails(900, 6, -1, 8, 1, (SceneItem *) NULL);

	if ((R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) != 1) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 1)) {
		_cable.postInit();
		_cable.setPosition(Common::Point(0, 0));
		_cable.fixPriority(1);

		if (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) {
			if (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) != 700) {
				_cable.setup(901, 3, 2);
			} else if (R2_GLOBALS._tractorField && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70)) {
				_cable.setup(901, 2, 1);
			} else {
				_cable.setup(901, 2, 8);
			}
			_cable.setPosition(Common::Point(171, 145));
			_cable.setDetails(700, -1, -1, -1, 1, (SceneItem *) NULL);
		} else {
			_cable.setDetails(700, -1, -1, -1, 1, (SceneItem *) NULL);
			if (!R2_GLOBALS._cableAttached) {
				_cable.setup(901, 1, 8);
				// Original set two times the same values: skipped
				_cable.setPosition(Common::Point((((100  - ((CABLE700_X * 350) / 100)) * 49) / 100) + ((CABLE700_Y * _electromagnet._percent * 6) / 100) + 89, 166 - (CABLE700_X / 3)));
				_cable.changeZoom(((100 - ((CABLE700_X * 350) / 100) + 52) / 10) * 10);
			}
		}
	}

	_background.setDetails(Rect(0, 0, 320, 200), 900, 0, -1, -1, 1, NULL);
	_sceneMode = 900;
	setAction(&_sequenceManager1, this, 900, &_controls, NULL);
}

void Scene900::remove() {
	if (_sceneMode != 901)
		R2_GLOBALS._sound1.play(10);

	SceneExt::remove();
}

void Scene900::signal() {
	switch (_sceneMode) {
	case 1:
		_controlsScreenNumber = 1;
		R2_GLOBALS._sound2.play(37);

		_button2.remove();
		_button3.remove();
		_button4.remove();
		_button5.remove();
		_button6.remove();
		_button7.remove();

		_button2.initButton(2);
		_button2.setup(900, 2, 1);
		_button2.setPosition(Common::Point(36, 166));

		_button3.initButton(3);
		_button3.setup(900, 2, 5);
		_button3.setPosition(Common::Point(117, 166));
		break;
	case 2:
		_controlsScreenNumber = 2;

		_button2.remove();
		_button3.remove();

		_button2.initButton(2);
		if (!R2_GLOBALS._tractorField)
			_button2.setup(900, 3, 9);
		else
			_button2.setup(900, 3, 11);
		_button2.setPosition(Common::Point(36, 166));

		_button4.initButton(5);
		_button4.setup(900, 3, 3);
		_button4.setPosition(Common::Point(76, 134));

		_button5.initButton(4);
		_button5.setup(900, 3, 7);
		_button5.setPosition(Common::Point(76, 156));

		_button6.initButton(6);
		_button6.setup(900, 3, 1);
		_button6.setPosition(Common::Point(55, 144));

		_button7.initButton(7);
		_button7.setup(900, 3, 5);
		_button7.setPosition(Common::Point(99, 144));

		break;
	case 3:
		_controlsScreenNumber = 3;

		_button2.remove();
		_button3.remove();
		_button4.remove();
		_button5.remove();
		_button6.remove();
		_button7.remove();

		_button2.initButton(8);
		_button2.setup(900, 4, 1);
		_button2.setPosition(Common::Point(36, 166));

		_button3.initButton(9);
		_button3.setup(900, 4, 5);
		_button3.setPosition(Common::Point(117, 166));
		break;
	case 4:
		_sceneMode = 0;
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		R2_GLOBALS._cableAttached = true;
		break;
	case 900:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;

		_controls.setup(900, 1, 1);

		_button1.initButton(1);
		_button1.setup(900, 1, 3);
		_button1.setPosition(Common::Point(77, 168));

		_sceneMode = 1;
		signal();
		break;
	case 901:
		R2_GLOBALS._sceneManager.changeScene(700);
		break;
	case 5:
		_sceneMode = 0;
		// fall through
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene900::dispatch() {
	if (_magnetChangeAmount.y != 0) {
		if (_magnetChangeAmount.y < 0) {
			R2_GLOBALS._electromagnetZoom--;
			++_magnetChangeAmount.y;
		} else {
			++R2_GLOBALS._electromagnetZoom;
			_magnetChangeAmount.y--;
		}
	}

	if (_magnetChangeAmount.x != 0) {
		if (_magnetChangeAmount.x < 0) {
			R2_GLOBALS._electromagnetChangeAmount--;
			++_magnetChangeAmount.x;
		} else {
			++R2_GLOBALS._electromagnetChangeAmount;
			_magnetChangeAmount.x--;
		}
	}

	if (R2_GLOBALS._sceneObjects->contains(&_cable)) {
		if (R2_GLOBALS._tractorField && (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (R2_GLOBALS._electromagnetChangeAmount == 20) && (R2_GLOBALS._electromagnetZoom == 70)) {
			if ((_cable._frame > 1) && (_cable._animateMode != ANIM_MODE_6))
				_cable.animate(ANIM_MODE_6, NULL);
		} else {
			if ((_cable._frame < 8) && (_cable._animateMode != ANIM_MODE_5) && !R2_GLOBALS._cableAttached && (R2_INVENTORY.getObjectScene(R2_CABLE_HARNESS) == 0) && (R2_INVENTORY.getObjectScene(R2_ATTRACTOR_CABLE_HARNESS) == 700) && (_sceneMode != 4))
				_cable.animate(ANIM_MODE_5, NULL);
		}
	}

	_electromagnet.changeZoom(100 - ((R2_GLOBALS._electromagnetZoom * 70) / 100));
	_electromagnet.setPosition(Common::Point(((_electromagnet._percent * R2_GLOBALS._electromagnetChangeAmount * 6) / 100) + 89, R2_GLOBALS._electromagnetZoom));

	if ((R2_GLOBALS._sceneObjects->contains(&_cable)) && R2_GLOBALS._cableAttached && (!_cable._mover) && (_cable._animateMode == ANIM_MODE_NONE)) {
		_cable.setPosition(Common::Point(_electromagnet._position.x + ((_electromagnet._percent * 49) / 100), _electromagnet._position.y + ((_electromagnet._percent * 3) / 10)));
		if (R2_GLOBALS._electromagnetZoom >= 75) {
			_cable.setup(901, 1, 1);
			_cable.changeZoom(((_electromagnet._percent + 52) / 10) * 10);
		} else {
			_cable.setup(901, 5, 1);
			_cable.changeZoom(((_electromagnet._percent / 10) * 10) + 30);
		}
	}
	Scene::dispatch();
}

} // End of namespace Ringworld2

} // End of namespace TsAGE
