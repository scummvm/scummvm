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

#include "tsage/ringworld2/ringworld2_vampire.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 1950 - Flup Tube Corridor Maze
 *
 *--------------------------------------------------------------------------*/

Scene1950::KeypadWindow::KeypadWindow() {
	_buttonIndex = 0;
}

void Scene1950::KeypadWindow::synchronize(Serializer &s) {
	SceneArea::synchronize(s);

	s.syncAsSint16LE(_buttonIndex);
}

Scene1950::KeypadWindow::KeypadButton::KeypadButton() {
	_buttonIndex = 0;
	_pressed = false;
	_toggled = false;
}

void Scene1950::KeypadWindow::KeypadButton::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_buttonIndex);
	s.syncAsSint16LE(_pressed);
	s.syncAsSint16LE(_toggled);
}

void Scene1950::KeypadWindow::KeypadButton::init(int indx) {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_buttonIndex = indx;
	_pressed = false;
	_toggled = false;

	postInit();
	setup(1971, 2, 1);
	fixPriority(249);
	setPosition(Common::Point(((_buttonIndex % 4) * 22) + 127, ((_buttonIndex / 4) * 19) + 71));
	scene->_sceneAreas.push_front(this);
}

void Scene1950::KeypadWindow::KeypadButton::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_USE)
			&& (_bounds.contains(event.mousePos)) && !_pressed) {
		R2_GLOBALS._sound2.play(227);
		if (!_toggled) {
			setFrame(2);
			_toggled = true;
		} else {
			setFrame(1);
			_toggled = false;
		}
		_pressed = true;
		event.handled = true;
	}

	if ((event.eventType == EVENT_BUTTON_UP) && _pressed) {
		_pressed = false;
		event.handled = true;
		Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;
		scene->doButtonPress(_buttonIndex);
	}
}

bool Scene1950::KeypadWindow::KeypadButton::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;
	return SceneActor::startAction(action, event);
}

void Scene1950::KeypadWindow::remove() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;
	for (_buttonIndex = 0; _buttonIndex < 16; ++_buttonIndex) {
		scene->_sceneAreas.remove(&_buttons[_buttonIndex]);
		_buttons[_buttonIndex].remove();
	}

	ModalWindow::remove();

	if (!R2_GLOBALS.getFlag(37))
		R2_GLOBALS._sound2.play(278);

	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_eastExit._enabled = true;

	if (!R2_GLOBALS.getFlag(37)) {
		if (R2_GLOBALS.getFlag(36)) {
			scene->_sceneMode = 1964;
			scene->setAction(&scene->_sequenceManager, scene, 1964, &R2_GLOBALS._player, NULL);
		} else {
			scene->_sceneMode = 1965;
			scene->setAction(&scene->_sequenceManager, scene, 1965, &R2_GLOBALS._player, NULL);
		}
	}
}

void Scene1950::KeypadWindow::setup2(int visage, int stripFrameNum, int frameNum, int posX, int posY) {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._mover)
		R2_GLOBALS._player.addMover(NULL);
	R2_GLOBALS._player._canWalk = false;

	ModalWindow::setup2(visage, stripFrameNum, frameNum, posX, posY);

	_object1.fixPriority(248);
	scene->_eastExit._enabled = false;
	setup3(1950, 27, 28, 27);

	for (_buttonIndex = 0; _buttonIndex < 16; _buttonIndex++)
		_buttons[_buttonIndex].init(_buttonIndex);
}

void Scene1950::KeypadWindow::setup3(int resNum, int lookLineNum, int talkLineNum, int useLineNum) {
	// Copy of Scene1200::LaserPanel::proc13()
	_areaActor.setDetails(resNum, lookLineNum, talkLineNum, useLineNum, 2, (SceneItem *) NULL);
}

/*--------------------------------------------------------------------------*/

bool Scene1950::Keypad::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (R2_GLOBALS.getFlag(37)))
		return SceneHotspot::startAction(action, event);

	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS.getFlag(36)) {
		scene->_sceneMode = 1962;
		scene->setAction(&scene->_sequenceManager, scene, 1962, &R2_GLOBALS._player, NULL);
	} else {
		scene->_sceneMode = 1963;
		scene->setAction(&scene->_sequenceManager, scene, 1963, &R2_GLOBALS._player, NULL);
	}
	return true;
}

bool Scene1950::Door::startAction(CursorType action, Event &event) {
	if (action != R2_SCRITH_KEY)
		return SceneActor::startAction(action, event);

	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	R2_INVENTORY.setObjectScene(R2_SCRITH_KEY, 0);
	scene->_sceneMode = 1958;
	scene->setAction(&scene->_sequenceManager, scene, 1958, &R2_GLOBALS._player, &scene->_door, NULL);
	return true;
}

bool Scene1950::Scrolls::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) != 1950))
		return SceneActor::startAction(action, event);

	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 1968;
	scene->setAction(&scene->_sequenceManager, scene, 1968, &R2_GLOBALS._player, NULL);

	return true;
}

bool Scene1950::Gem::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (!R2_GLOBALS.getFlag(37)))
		return SceneActor::startAction(action, event);

	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 1967;
	scene->setAction(&scene->_sequenceManager, scene, 1967, &R2_GLOBALS._player, NULL);

	return true;
}

/*--------------------------------------------------------------------------*/

Scene1950::Vampire::Vampire() {
	_deadPosition = Common::Point(0, 0);
	_deltaX = 0;
	_deltaY = 0;
	_vampireMode = 0;
}

void Scene1950::Vampire::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_deadPosition.x);
	s.syncAsSint16LE(_deadPosition.y);
	s.syncAsSint16LE(_deltaX);
	s.syncAsSint16LE(_deltaY);
	s.syncAsSint16LE(_vampireMode);
}

void Scene1950::Vampire::signal() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	switch (_vampireMode) {
	case 19: {
		_vampireMode = 0;
		setVisage(1960);
		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			setStrip(2);
		else
			setStrip(1);

		NpcMover *mover = new NpcMover();
		addMover(mover, &scene->_vampireDestPos, scene);
		}
		break;
	case 20: {
		// Non fatal shot
		_vampireMode = 19;
		R2_GLOBALS._player.setVisage(22);
		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			R2_GLOBALS._player.setStrip(1);
		else
			R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._shotsRequired--;

		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			_deadPosition.x = _position.x + 10;
		else
			_deadPosition.x = _position.x - 10;
		_deadPosition.y = _position.y - 4;

		setVisage(1961);

		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			setStrip(2);
		else
			setStrip(1);

		animate(ANIM_MODE_2, NULL);
		Common::Point pt = _deadPosition;
		PlayerMover *mover = new PlayerMover();
		addMover(mover, &pt, this);

		R2_GLOBALS._player.enableControl();
		}
		break;
	case 21: {
		// Fatal shot
		R2_GLOBALS._player.setVisage(22);
		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			R2_GLOBALS._player.setStrip(1);
		else
			R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

		setVisage(1961);
		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			setStrip(4);
		else
			setStrip(3);
		setDetails(1950, 15, -1, 17, 2, (SceneItem *) NULL);
		addMover(NULL);
		_numFrames = 8;
		R2_GLOBALS._sound2.play(226);
		animate(ANIM_MODE_5, NULL);
		fixPriority(10);

		R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._isAlive = false;
		R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._shotsRequired--;
		R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._position = _position;
		_deltaX = (_position.x - R2_GLOBALS._player._position.x) / 2;
		_deltaY = (_position.y - R2_GLOBALS._player._position.y) / 2;

		byte vampireCount = 0;
		for (byte i = 0; i < 18; ++i) {
			if (!R2_GLOBALS._vampireData[i]._isAlive)
				++vampireCount;
		}

		if (vampireCount == 18) {
			R2_GLOBALS.setFlag(36);
			_vampireMode = 23;
			Common::Point pt(R2_GLOBALS._player._position.x + _deltaX, R2_GLOBALS._player._position.y + _deltaY);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if (vampireCount == 1) {
			_vampireMode = 22;
			Common::Point pt(R2_GLOBALS._player._position.x + _deltaX, R2_GLOBALS._player._position.y + _deltaY);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_WALK);
		}

		if (R2_GLOBALS._flubMazeEntryDirection == 3)
			scene->_eastExit._enabled = true;
		else
			scene->_westExit._enabled = true;

		scene->_vampireActive = false;
		}
		break;
	case 22:
		SceneItem::display(1950, 18, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	case 23:
		SceneItem::display(1950, 25, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		scene->_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
		scene->setAction(&scene->_sequenceManager, scene, 1960, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

bool Scene1950::Vampire::startAction(CursorType action, Event &event) {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	if (!R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._isAlive ||
			(action != R2_PHOTON_STUNNER))
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._vampireData[scene->_vampireIndex - 1]._shotsRequired <= 1)
		_vampireMode = 21;
	else
		_vampireMode = 20;

	R2_GLOBALS._player.setVisage(25);
	if (R2_GLOBALS._flubMazeEntryDirection == 3)
		R2_GLOBALS._player.setStrip(2);
	else
		R2_GLOBALS._player.setStrip(1);
	R2_GLOBALS._player.animate(ANIM_MODE_5, this);
	R2_GLOBALS._sound3.play(99);

	return true;
}

/*--------------------------------------------------------------------------*/

void Scene1950::NorthExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 1;
	scene->_sceneMode = 11;

	Common::Point pt(160, 127);
	PlayerMover *mover = new PlayerMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1950::UpExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 2;
	scene->_sceneMode = 12;

	if (!scene->_upExitStyle) {
		if (R2_GLOBALS.getFlag(36))
			scene->setAction(&scene->_sequenceManager, scene, 1953, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1970, &R2_GLOBALS._player, NULL);
	} else {
		if (R2_GLOBALS.getFlag(36))
			scene->setAction(&scene->_sequenceManager, scene, 1952, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1969, &R2_GLOBALS._player, NULL);
	}
}

void Scene1950::EastExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 3;
	scene->_sceneMode = 13;

	if (scene->_vampireActive)
		R2_GLOBALS._player.animate(ANIM_MODE_9);

	Common::Point pt(340, 160);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1950::DownExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 4;
	scene->_sceneMode = 14;

	if (R2_GLOBALS.getFlag(36))
		scene->setAction(&scene->_sequenceManager, scene, 1956, &R2_GLOBALS._player, NULL);
	else
		scene->setAction(&scene->_sequenceManager, scene, 1973, &R2_GLOBALS._player, NULL);
}

void Scene1950::SouthExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 5;
	scene->_sceneMode = 15;

	Common::Point pt(160, 213);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1950::WestExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 6;

	if (R2_GLOBALS._flubMazeArea == 2) {
		// In the very first corridor area after the Scrith Door
		if ((R2_GLOBALS.getFlag(36)) && (R2_INVENTORY.getObjectScene(R2_SAPPHIRE_BLUE) == 2) && (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) == 2)) {
			scene->_sceneMode = 1961;
			Common::Point pt(-20, 160);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
		} else {
			if (!R2_GLOBALS.getFlag(36))
				SceneItem::display(1950, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			if ((R2_INVENTORY.getObjectScene(R2_SAPPHIRE_BLUE) == 1950) || (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) == 1950))
				SceneItem::display(1950, 34, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			scene->_sceneMode = 0;
			Common::Point pt(30, 160);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
		}
	} else {
		if (scene->_vampireActive)
			R2_GLOBALS._player.animate(ANIM_MODE_9);

		scene->_sceneMode = 16;
		Common::Point pt(-20, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	}
}

void Scene1950::ShaftExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 0;
	scene->_sceneMode = 1951;
	scene->setAction(&scene->_sequenceManager, scene, 1951, &R2_GLOBALS._player, NULL);
}

void Scene1950::DoorExit::changeScene() {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	R2_GLOBALS._flubMazeEntryDirection = 3;
	if (R2_GLOBALS._player._visage == 22) {
		scene->_sceneMode = 1975;
		scene->setAction(&scene->_sequenceManager, scene, 1975, &R2_GLOBALS._player, NULL);
	} else {
		SceneItem::display(1950, 22, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		R2_GLOBALS._flubMazeEntryDirection = 0;
		scene->_sceneMode = 0;
		Common::Point pt(250, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
		_enabled = true;
	}
}

/*--------------------------------------------------------------------------*/

Scene1950::Scene1950() {
	_upExitStyle = false;
	_removeFlag = false;
	_vampireActive = false;
	_vampireDestPos = Common::Point(0, 0);
	_vampireIndex = 0;
}

void Scene1950::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_upExitStyle);
	s.syncAsSint16LE(_removeFlag);
	s.syncAsSint16LE(_vampireActive);
	s.syncAsSint16LE(_vampireDestPos.x);
	s.syncAsSint16LE(_vampireDestPos.y);
	s.syncAsSint16LE(_vampireIndex);
}

void Scene1950::initArea() {
	_northExit._enabled = false;
	_upExit._enabled = false;
	_eastExit._enabled = false;
	_downExit._enabled = false;
	_southExit._enabled = false;
	_westExit._enabled = false;
	_shaftExit._enabled = false;
	_doorExit._enabled = false;
	_northExit._insideArea = false;
	_upExit._insideArea = false;
	_eastExit._insideArea = false;
	_downExit._insideArea = false;
	_southExit._insideArea = false;
	_westExit._insideArea = false;
	_shaftExit._insideArea = false;
	_doorExit._insideArea = false;
	_northExit._moving = false;
	_upExit._moving = false;
	_eastExit._moving = false;
	_downExit._moving = false;
	_southExit._moving = false;
	_westExit._moving = false;
	_shaftExit._moving = false;
	_doorExit._moving = false;
	_upExitStyle = false;

	switch (R2_GLOBALS._flubMazeArea - 1) {
	case 0:
		loadScene(1948);
		break;
	case 1:
	// No break on purpose
	case 8:
	// No break on purpose
	case 10:
	// No break on purpose
	case 12:
	// No break on purpose
	case 16:
	// No break on purpose
	case 19:
	// No break on purpose
	case 23:
	// No break on purpose
	case 30:
	// No break on purpose
	case 44:
	// No break on purpose
	case 72:
	// No break on purpose
	case 74:
	// No break on purpose
	case 86:
	// No break on purpose
	case 96:
	// No break on purpose
	case 103:
		loadScene(1950);
		break;
	case 2:
	// No break on purpose
	case 29:
		loadScene(1965);
		break;
	case 3:
	// No break on purpose
	case 9:
	// No break on purpose
	case 11:
	// No break on purpose
	case 15:
	// No break on purpose
	case 24:
	// No break on purpose
	case 39:
	// No break on purpose
	case 45:
	// No break on purpose
	case 71:
	// No break on purpose
	case 73:
	// No break on purpose
	case 75:
	// No break on purpose
	case 79:
	// No break on purpose
	case 85:
	// No break on purpose
	case 87:
	// No break on purpose
	case 95:
		loadScene(1955);
		break;
	case 4:
	// No break on purpose
	case 6:
	// No break on purpose
	case 13:
	// No break on purpose
	case 27:
	// No break on purpose
	case 41:
	// No break on purpose
	case 48:
	// No break on purpose
	case 50:
	// No break on purpose
	case 54:
	// No break on purpose
	case 76:
	// No break on purpose
	case 80:
	// No break on purpose
	case 90:
	// No break on purpose
	case 104:
		loadScene(1975);
		break;
	case 5:
	// No break on purpose
	case 7:
	// No break on purpose
	case 14:
	// No break on purpose
	case 28:
	// No break on purpose
	case 32:
	// No break on purpose
	case 47:
	// No break on purpose
	case 53:
		loadScene(1997);
		break;
	case 17:
	// No break on purpose
	case 20:
	// No break on purpose
	case 25:
	// No break on purpose
	case 31:
	// No break on purpose
	case 33:
	// No break on purpose
	case 46:
		loadScene(1995);
		break;
	case 18:
	// No break on purpose
	case 22:
	// No break on purpose
	case 26:
	// No break on purpose
	case 36:
	// No break on purpose
	case 38:
	// No break on purpose
	case 43:
	// No break on purpose
	case 51:
	// No break on purpose
	case 70:
	// No break on purpose
	case 78:
	// No break on purpose
	case 84:
	// No break on purpose
	case 89:
	// No break on purpose
	case 101:
		loadScene(1970);
		break;
	case 21:
	// No break on purpose
	case 34:
	// No break on purpose
	case 57:
	// No break on purpose
	case 58:
	// No break on purpose
	case 59:
	// No break on purpose
	case 62:
	// No break on purpose
	case 65:
		loadScene(1980);
		break;
	case 35:
	// No break on purpose
	case 61:
	// No break on purpose
	case 77:
	// No break on purpose
	case 83:
		loadScene(1982);
		break;
	case 37:
	// No break on purpose
	case 52:
	// No break on purpose
	case 82:
	// No break on purpose
	case 88:
	// No break on purpose
	case 92:
	// No break on purpose
	case 97:
	// No break on purpose
	case 100:
		loadScene(1962);
		break;
	case 40:
	// No break on purpose
	case 102:
		loadScene(1960);
		break;
	case 42:
	// No break on purpose
	case 55:
	// No break on purpose
	case 60:
	// No break on purpose
	case 66:
	// No break on purpose
	case 68:
	// No break on purpose
	case 69:
	// No break on purpose
	case 93:
	// No break on purpose
	case 98:
		loadScene(1990);
		break;
	case 49:
	// No break on purpose
	case 81:
	// No break on purpose
	case 91:
	// No break on purpose
	case 94:
	// No break on purpose
	case 99:
		loadScene(1967);
		break;
	case 56:
	// No break on purpose
	case 63:
	// No break on purpose
	case 64:
	// No break on purpose
	case 67:
		loadScene(1985);
		_upExitStyle = true;
		break;
	default:
		break;
	}

	if (R2_GLOBALS._flubMazeArea != 1)
		R2_GLOBALS._walkRegions.load(1950);

	switch (R2_GLOBALS._flubMazeArea - 1) {
	case 0:
		_shaftExit._enabled = true;
		if ((R2_INVENTORY.getObjectScene(R2_SCRITH_KEY) == 0) && (R2_INVENTORY.getObjectScene(R2_SAPPHIRE_BLUE) == 1950))
			_doorExit._enabled = true;
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
		R2_GLOBALS._walkRegions.disableRegion(5);
		R2_GLOBALS._walkRegions.disableRegion(6);
		break;
	case 1:
	// No break on purpose
	case 2:
	// No break on purpose
	case 3:
	// No break on purpose
	case 8:
	// No break on purpose
	case 9:
	// No break on purpose
	case 10:
	// No break on purpose
	case 11:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 16:
	// No break on purpose
	case 19:
	// No break on purpose
	case 23:
	// No break on purpose
	case 24:
	// No break on purpose
	case 29:
	// No break on purpose
	case 30:
	// No break on purpose
	case 39:
	// No break on purpose
	case 40:
	// No break on purpose
	case 44:
	// No break on purpose
	case 45:
	// No break on purpose
	case 71:
	// No break on purpose
	case 72:
	// No break on purpose
	case 73:
	// No break on purpose
	case 74:
	// No break on purpose
	case 75:
	// No break on purpose
	case 79:
	// No break on purpose
	case 85:
	// No break on purpose
	case 86:
	// No break on purpose
	case 87:
	// No break on purpose
	case 95:
	// No break on purpose
	case 96:
	// No break on purpose
	case 102:
	// No break on purpose
	case 103:
		_eastExit._enabled = true;
		_westExit._enabled = true;
		break;
	case 4:
	// No break on purpose
	case 6:
	// No break on purpose
	case 13:
	// No break on purpose
	case 17:
	// No break on purpose
	case 20:
	// No break on purpose
	case 25:
	// No break on purpose
	case 27:
	// No break on purpose
	case 31:
	// No break on purpose
	case 33:
	// No break on purpose
	case 37:
	// No break on purpose
	case 41:
	// No break on purpose
	case 46:
	// No break on purpose
	case 48:
	// No break on purpose
	case 50:
	// No break on purpose
	case 52:
	// No break on purpose
	case 54:
	// No break on purpose
	case 76:
	// No break on purpose
	case 80:
	// No break on purpose
	case 82:
	// No break on purpose
	case 88:
	// No break on purpose
	case 90:
	// No break on purpose
	case 92:
	// No break on purpose
	case 97:
	// No break on purpose
	case 100:
	// No break on purpose
	case 104:
		_westExit._enabled = true;
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(9);
		break;
	case 5:
	// No break on purpose
	case 7:
	// No break on purpose
	case 14:
	// No break on purpose
	case 18:
	// No break on purpose
	case 22:
	// No break on purpose
	case 26:
	// No break on purpose
	case 28:
	// No break on purpose
	case 32:
	// No break on purpose
	case 36:
	// No break on purpose
	case 38:
	// No break on purpose
	case 43:
	// No break on purpose
	case 47:
	// No break on purpose
	case 49:
	// No break on purpose
	case 51:
	// No break on purpose
	case 53:
	// No break on purpose
	case 70:
	// No break on purpose
	case 78:
	// No break on purpose
	case 81:
	// No break on purpose
	case 84:
	// No break on purpose
	case 89:
	// No break on purpose
	case 91:
	// No break on purpose
	case 94:
	// No break on purpose
	case 99:
	// No break on purpose
	case 101:
		_eastExit._enabled = true;
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(7);
		R2_GLOBALS._walkRegions.disableRegion(13);
		break;
	default:
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(7);
		R2_GLOBALS._walkRegions.disableRegion(9);
		R2_GLOBALS._walkRegions.disableRegion(13);
		break;
	}

	_northDoorway.remove();
	_northDoorway.removeObject();
	_southDoorway.remove();

	switch (R2_GLOBALS._flubMazeArea - 4) {
	case 0:
	// No break on purpose
	case 3:
	// No break on purpose
	case 16:
	// No break on purpose
	case 22:
	// No break on purpose
	case 24:
	// No break on purpose
	case 32:
	// No break on purpose
	case 33:
	// No break on purpose
	case 45:
	// No break on purpose
	case 46:
	// No break on purpose
	case 48:
	// No break on purpose
	case 51:
	// No break on purpose
	case 56:
	// No break on purpose
	case 59:
	// No break on purpose
	case 67:
	// No break on purpose
	case 68:
	// No break on purpose
	case 70:
	// No break on purpose
	case 73:
	// No break on purpose
	case 82:
	// No break on purpose
	case 90:
		_northExit._enabled = true;
		_northDoorway.setup(1950, (R2_GLOBALS._flubMazeArea % 2) + 1, 1, 160, 137, 25);
		//visage,strip,frame,px,py,priority,effect
		_southDoorway.postInit();
		_southDoorway.setVisage(1950);
		_southDoorway.setStrip((((R2_GLOBALS._flubMazeArea - 1) / 35) % 2) + 1);
		_southDoorway.setFrame(2);
		_southDoorway.setPosition(Common::Point(160, 167));
		_southDoorway.fixPriority(220);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
		break;
	case 7:
	// No break on purpose
	case 10:
	// No break on purpose
	case 23:
	// No break on purpose
	case 29:
	// No break on purpose
	case 31:
	// No break on purpose
	case 39:
	// No break on purpose
	case 40:
	// No break on purpose
	case 52:
	// No break on purpose
	case 53:
	// No break on purpose
	case 55:
	// No break on purpose
	case 63:
	// No break on purpose
	case 65:
	// No break on purpose
	case 66:
	// No break on purpose
	case 75:
	// No break on purpose
	case 77:
	// No break on purpose
	case 81:
	// No break on purpose
	case 87:
	// No break on purpose
	case 89:
	// No break on purpose
	case 97:
		_southExit._enabled = true;

		_southDoorway.postInit();
		_southDoorway.setVisage(1950);
		_southDoorway.setStrip((((R2_GLOBALS._flubMazeArea - 1) / 35) % 2) + 1);
		_southDoorway.setFrame(3);
		_southDoorway.setPosition(Common::Point(160, 167));
		_southDoorway.fixPriority(220);
		break;
	case 58:
	// No break on purpose
	case 74:
	// No break on purpose
	case 80:
		_northExit._enabled = true;
		_southExit._enabled = true;

		_northDoorway.setup(1950, (R2_GLOBALS._flubMazeArea % 2) + 1, 1, 160, 137, 25);

		_southDoorway.postInit();
		_southDoorway.setVisage(1950);
		_southDoorway.setStrip((((R2_GLOBALS._flubMazeArea - 1) / 35) % 2) + 1);
		_southDoorway.setFrame(3);
		_southDoorway.setPosition(Common::Point(160, 167));
		_southDoorway.fixPriority(220);
		R2_GLOBALS._walkRegions.disableRegion(3);
		R2_GLOBALS._walkRegions.disableRegion(4);
		break;
	default:
		_southDoorway.postInit();
		_southDoorway.setVisage(1950);
		_southDoorway.setStrip(((R2_GLOBALS._flubMazeArea - 1) / 35) % 2 + 1);
		_southDoorway.setFrame(2);
		_southDoorway.setPosition(Common::Point(160, 167));
		_southDoorway.fixPriority(220);
		break;
	}

	switch (R2_GLOBALS._flubMazeArea - 3) {
	case 0:
	// No break on purpose
	case 3:
	// No break on purpose
	case 5:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 18:
	// No break on purpose
	case 19:
	// No break on purpose
	case 23:
	// No break on purpose
	case 26:
	// No break on purpose
	case 27:
	// No break on purpose
	case 29:
	// No break on purpose
	case 30:
	// No break on purpose
	case 31:
	// No break on purpose
	case 32:
	// No break on purpose
	case 44:
	// No break on purpose
	case 45:
	// No break on purpose
	case 51:
	// No break on purpose
	case 55:
	// No break on purpose
	case 56:
	// No break on purpose
	case 57:
	// No break on purpose
	case 60:
	// No break on purpose
	case 63:
		_upExit._enabled = true;
		break;
	case 54:
	// No break on purpose
	case 61:
	// No break on purpose
	case 62:
	// No break on purpose
	case 65:
		_upExit._enabled = true;
		// fall through
	case 35:
	// No break on purpose
	case 38:
	// No break on purpose
	case 40:
	// No break on purpose
	case 47:
	// No break on purpose
	case 50:
	// No break on purpose
	case 53:
	// No break on purpose
	case 58:
	// No break on purpose
	case 64:
	// No break on purpose
	case 66:
	// No break on purpose
	case 67:
	// No break on purpose
	case 79:
	// No break on purpose
	case 80:
	// No break on purpose
	case 86:
	// No break on purpose
	case 89:
	// No break on purpose
	case 90:
	// No break on purpose
	case 91:
	// No break on purpose
	case 92:
	// No break on purpose
	case 95:
	// No break on purpose
	case 96:
	// No break on purpose
	case 97:
	// No break on purpose
	case 98:
	// No break on purpose
	case 100:
		_downExit._enabled = true;
		R2_GLOBALS._walkRegions.disableRegion(4);
		R2_GLOBALS._walkRegions.disableRegion(5);
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(10);
		R2_GLOBALS._walkRegions.disableRegion(11);
	default:
		break;
	}
	R2_GLOBALS._uiElements.draw();
}

void Scene1950::enterArea() {
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	_vampire.remove();
	_door.remove();
	_scrolls.remove();

	_vampireActive = false;
	_vampireIndex = 0;

	// Certain areas have a vampire in them
	switch (R2_GLOBALS._flubMazeArea) {
	case 10:
		_vampireIndex = 1;
		break;
	case 13:
		_vampireIndex = 2;
		break;
	case 16:
		_vampireIndex = 3;
		break;
	case 17:
		_vampireIndex = 4;
		break;
	case 24:
		_vampireIndex = 5;
		break;
	case 25:
		_vampireIndex = 6;
		break;
	case 31:
		_vampireIndex = 7;
		break;
	case 40:
		_vampireIndex = 8;
		break;
	case 45:
		_vampireIndex = 9;
		break;
	case 46:
		_vampireIndex = 10;
		break;
	case 73:
		_vampireIndex = 11;
		break;
	case 75:
		_vampireIndex = 12;
		break;
	case 80:
		_vampireIndex = 13;
		break;
	case 87:
		_vampireIndex = 14;
		break;
	case 88:
		_vampireIndex = 15;
		break;
	case 96:
		_vampireIndex = 16;
		break;
	case 97:
		_vampireIndex = 17;
		break;
	case 104:
		_vampireIndex = 18;
		break;
	default:
		break;
	}

	if (_vampireIndex != 0) {
		_vampire.postInit();
		_vampire._numFrames = 6;
		_vampire._moveRate = 6;
		_vampire._moveDiff = Common::Point(3, 2);
		_vampire._effect = EFFECT_SHADED;

		if (!R2_GLOBALS._vampireData[_vampireIndex - 1]._isAlive) {
			// Show vampire ashes
			_vampire.setPosition(Common::Point(R2_GLOBALS._vampireData[_vampireIndex - 1]._position));
			_vampire.animate(ANIM_MODE_NONE, NULL);
			_vampire.addMover(NULL);
			_vampire.setVisage(1961);
			_vampire.setStrip(4);
			_vampire.setFrame(10);
			_vampire.fixPriority(10);
			_vampire.setDetails(1950, 15, -1, 17, 2, (SceneItem *) NULL);
		} else {
			// Start the vampire
			_vampire.setVisage(1960);
			_vampire.setPosition(Common::Point(160, 130));
			_vampire.animate(ANIM_MODE_2, NULL);
			_vampire.setDetails(1950, 12, -1, 14, 2, (SceneItem *) NULL);
			_vampireActive = true;
		}
	}
	if ((R2_GLOBALS._flubMazeArea == 1) && (R2_INVENTORY.getObjectScene(R2_SCRITH_KEY) != 0)) {
		// Show doorway at the right hand side of the very first flub corridor
		_door.postInit();
		_door.setVisage(1948);
		_door.setStrip(3);
		_door.setPosition(Common::Point(278, 155));
		_door.fixPriority(100);
		_door.setDetails(1950, 19, 20, 23, 2, (SceneItem *) NULL);
	}

	if (R2_GLOBALS._flubMazeArea == 102) {
		R2_GLOBALS._walkRegions.load(1951);
		R2_GLOBALS._walkRegions.disableRegion(1);
		R2_GLOBALS._walkRegions.disableRegion(5);
		R2_GLOBALS._walkRegions.disableRegion(6);
		R2_GLOBALS._walkRegions.disableRegion(7);

		_cube.postInit();
		_cube.setVisage(1970);
		_cube.setStrip(1);
		if (R2_GLOBALS.getFlag(37))
			_cube.setFrame(3);
		else
			_cube.setFrame(1);
		_cube.setPosition(Common::Point(193, 158));
		_cube.setDetails(1950, 3, 4, 5, 2, (SceneItem *) NULL);

		_pulsingLights.postInit();
		_pulsingLights.setVisage(1970);
		_pulsingLights.setStrip(3);
		_pulsingLights.animate(ANIM_MODE_2, NULL);
		_pulsingLights._numFrames = 6;
		_pulsingLights.setPosition(Common::Point(194, 158));
		_pulsingLights.fixPriority(159);

		_keypad.setDetails(Rect(188, 124, 199, 133), 1950, 27, 28, -1, 2, NULL);

		if (R2_INVENTORY.getObjectScene(R2_SAPPHIRE_BLUE) == 1950) {
			_gem.postInit();
			_gem.setVisage(1970);
			_gem.setStrip(1);
			_gem.setFrame(2);
			_gem.fixPriority(160);
		}

		if (R2_GLOBALS.getFlag(37)) {
			_gem.setPosition(Common::Point(192, 118));
			_gem.setDetails(1950, 9, 4, -1, 2, (SceneItem *) NULL);
		} else {
			_containmentField.postInit();
			_containmentField.setVisage(1970);
			_containmentField.setStrip(4);
			_containmentField._numFrames = 4;
			_containmentField.animate(ANIM_MODE_8, 0, NULL);
			_containmentField.setPosition(Common::Point(192, 121));
			_containmentField.fixPriority(159);
			_containmentField.setDetails(1950, 6, 7, 8, 2, (SceneItem *) NULL);

			_gem.setPosition(Common::Point(192, 109));
			_gem.setDetails(1950, 9, 7, 8, 2, (SceneItem *) NULL);
		}

		_scrolls.postInit();
		_scrolls.setVisage(1972);
		_scrolls.setStrip(1);
		_scrolls.setPosition(Common::Point(76, 94));
		_scrolls.fixPriority(25);
		_scrolls.setDetails(1950, 30, -1, -1, 2, (SceneItem *) NULL);
		if (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) == 2)
			_scrolls.setFrame(2);
		else
			_scrolls.setFrame(1);

		_removeFlag = true;
	} else if (_removeFlag) {
		_cube.remove();
		_containmentField.remove();
		_gem.remove();
		_pulsingLights.remove();
		_scrolls.remove();

		R2_GLOBALS._sceneItems.remove(&_background);
		_background.setDetails(Rect(0, 0, 320, 200), 1950, 0, 1, 2, 2, NULL);

		_removeFlag = false;
	}

	switch (R2_GLOBALS._flubMazeEntryDirection) {
	case 0:
		_sceneMode = 1950;
		if (R2_INVENTORY.getObjectScene(R2_SCRITH_KEY) == 0)
			// The original uses CURSOR_ARROW. CURSOR_WALK is much more coherent
			R2_GLOBALS._player.enableControl(CURSOR_WALK);
		else
			setAction(&_sequenceManager, this, 1950, &R2_GLOBALS._player, NULL);

		break;
	case 1: {
		_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
		R2_GLOBALS._player.setPosition(Common::Point(160, 213));
		Common::Point pt(160, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2:
		_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
		if (R2_GLOBALS.getFlag(36))
			setAction(&_sequenceManager, this, 1957, &R2_GLOBALS._player, NULL);
		else
			setAction(&_sequenceManager, this, 1974, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		// Entering from the left
		if (!_vampireActive) {
			_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
			R2_GLOBALS._player.setPosition(Common::Point(-20, 160));
			Common::Point pt(30, 160);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			_sceneMode = 18;
			_eastExit._enabled = false;
			_vampireDestPos = Common::Point(60, 152);
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;

			_vampire.setStrip(2);
			NpcMover *mover = new NpcMover();
			_vampire.addMover(mover, &_vampireDestPos, this);

			R2_GLOBALS._player.setPosition(Common::Point(-20, 160));
			Common::Point pt2(30, 160);
			NpcMover *mover2 = new NpcMover();
			R2_GLOBALS._player.addMover(mover2, &pt2, NULL);
		}
		break;
	case 4:
		_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
		if (!_upExitStyle) {
			if (R2_GLOBALS.getFlag(36))
				setAction(&_sequenceManager, this, 1955, &R2_GLOBALS._player, NULL);
			else
				setAction(&_sequenceManager, this, 1972, &R2_GLOBALS._player, NULL);
		} else {
			if (R2_GLOBALS.getFlag(36))
				setAction(&_sequenceManager, this, 1954, &R2_GLOBALS._player, NULL);
			else
				setAction(&_sequenceManager, this, 1971, &R2_GLOBALS._player, NULL);
		}
		break;
	case 5: {
		_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
		R2_GLOBALS._player.setPosition(Common::Point(160, 127));
		Common::Point pt(160, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 6:
		// Entering from the right
		if (!_vampireActive) {
			_sceneMode = R2_GLOBALS._flubMazeEntryDirection;
			if (R2_GLOBALS._flubMazeArea == 1) {
				setAction(&_sequenceManager, this, 1961, &R2_GLOBALS._player, NULL);
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(340, 160));
				Common::Point pt(289, 160);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			}
		} else {
			_sceneMode = 17;
			_westExit._enabled = false;
			_vampireDestPos = Common::Point(259, 152);

			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;

			_vampire.setStrip(1);
			NpcMover *mover = new NpcMover();
			_vampire.addMover(mover, &_vampireDestPos, this);

			R2_GLOBALS._player.setPosition(Common::Point(340, 160));
			Common::Point pt2(289, 160);
			NpcMover *mover2 = new NpcMover();
			R2_GLOBALS._player.addMover(mover2, &pt2, NULL);
		}
		break;
	default:
		break;
	}
}

void Scene1950::doButtonPress(int indx) {
	Scene1950 *scene = (Scene1950 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();

	int prevIndex = indx - 1;
	if ((indx / 4) == (prevIndex / 4)) {
		if (prevIndex < 0)
			prevIndex = 3;
	} else {
		prevIndex += 4;
	}

	assert(prevIndex >= 0 && prevIndex < 16);
	if (!_KeypadWindow._buttons[prevIndex]._toggled) {
		_KeypadWindow._buttons[prevIndex].setFrame(2);
		_KeypadWindow._buttons[prevIndex]._toggled = true;
	} else {
		_KeypadWindow._buttons[prevIndex].setFrame(1);
		_KeypadWindow._buttons[prevIndex]._toggled = false;
	}

	prevIndex = indx + 1;
	if ((indx / 4) == (prevIndex / 4)) {
		if (prevIndex >  15)
			prevIndex = 12;
	} else {
		prevIndex -= 4;
	}

	assert(prevIndex >= 0 && prevIndex < 16);
	if (!_KeypadWindow._buttons[prevIndex]._toggled) {
		_KeypadWindow._buttons[prevIndex].setFrame(2);
		_KeypadWindow._buttons[prevIndex]._toggled = true;
	} else {
		_KeypadWindow._buttons[prevIndex].setFrame(1);
		_KeypadWindow._buttons[prevIndex]._toggled = false;
	}

	prevIndex = indx - 4;
	if (prevIndex < 0)
		prevIndex += 16;

	assert(prevIndex >= 0 && prevIndex < 16);
	if (!_KeypadWindow._buttons[prevIndex]._toggled) {
		_KeypadWindow._buttons[prevIndex].setFrame(2);
		_KeypadWindow._buttons[prevIndex]._toggled = true;
	} else {
		_KeypadWindow._buttons[prevIndex].setFrame(1);
		_KeypadWindow._buttons[prevIndex]._toggled = false;
	}

	prevIndex = indx + 4;
	if (prevIndex > 15)
		prevIndex -= 16;

	assert(prevIndex >= 0 && prevIndex < 16);
	if (!_KeypadWindow._buttons[prevIndex]._toggled) {
		_KeypadWindow._buttons[prevIndex].setFrame(2);
		_KeypadWindow._buttons[prevIndex]._toggled = true;
	} else {
		_KeypadWindow._buttons[prevIndex].setFrame(1);
		_KeypadWindow._buttons[prevIndex]._toggled = false;
	}

	// Check whether all the buttons are highlighted
	int cpt = 0;
	for (prevIndex = 0; prevIndex < 16; prevIndex++) {
		if (_KeypadWindow._buttons[prevIndex]._toggled)
			++cpt;
	}

	if (cpt != 16) {
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
	} else {
		R2_GLOBALS.setFlag(37);
		_sceneMode = 24;
		setAction(&_sequenceManager, scene, 1976, NULL);
	}
}

void Scene1950::postInit(SceneObjectList *OwnerList) {
	_upExitStyle = false;
	_removeFlag = false;
	_vampireActive = false;
	_vampireIndex = 0;
	if (R2_GLOBALS._sceneManager._previousScene == 300)
		R2_GLOBALS._flubMazeArea = 103;

	initArea();
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(105);

	_northExit.setDetails(Rect(130, 46, 189, 135), SHADECURSOR_UP, 1950);
	_northExit.setDest(Common::Point(160, 145));

	_upExit.setDetails(Rect(208, 0, 255, 73), EXITCURSOR_N, 1950);
	_upExit.setDest(Common::Point(200, 151));

	_eastExit.setDetails(Rect(305, 95, 320, 147), EXITCURSOR_E, 1950);
	_eastExit.setDest(Common::Point(312, 160));

	_downExit.setDetails(Rect(208, 99, 255, 143), EXITCURSOR_S, 1950);
	_downExit.setDest(Common::Point(200, 151));

	_southExit.setDetails(Rect(113, 154, 206, 168), SHADECURSOR_DOWN, 1950);
	_southExit.setDest(Common::Point(160, 165));

	_westExit.setDetails(Rect(0, 95, 14, 147), EXITCURSOR_W, 1950);
	_westExit.setDest(Common::Point(7, 160));

	_shaftExit.setDetails(Rect(72, 54, 120, 128), EXITCURSOR_NW, 1950);
	_shaftExit.setDest(Common::Point(120, 140));

	_doorExit.setDetails(Rect(258, 60, 300, 145), EXITCURSOR_NE, 1950);
	_doorExit.setDest(Common::Point(268, 149));

	R2_GLOBALS._player.postInit();
	if ( (R2_INVENTORY.getObjectScene(R2_TANNER_MASK) == 0) && (R2_INVENTORY.getObjectScene(R2_PURE_GRAIN_ALCOHOL) == 0)
		&& (R2_INVENTORY.getObjectScene(R2_SOAKED_FACEMASK) == 0) && (!R2_GLOBALS.getFlag(36)) )
		R2_GLOBALS._player.setVisage(22);
	else
		R2_GLOBALS._player.setVisage(20);

	R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	_background.setDetails(Rect(0, 0, 320, 200), 1950, 0, 1, 2, 1, NULL);

	enterArea();
}

void Scene1950::remove() {
	R2_GLOBALS._sound1.stop();
	R2_GLOBALS._sound2.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1950::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._flubMazeArea += 7;
		initArea();
		enterArea();
		break;
	case 12:
		// Moving up a ladder within the Flub maze
		R2_GLOBALS._flubMazeArea += 35;
		initArea();
		enterArea();
		break;
	case 1975:
		SceneItem::display(1950, 21, SET_WIDTH, 280, SET_X, 160, SET_POS_MODE, 1,
			SET_Y, 20, SET_EXT_BGCOLOR, 7, LIST_END);
		// fall through
	case 13:
		// Moving east within the Flub maze
		++R2_GLOBALS._flubMazeArea;
		initArea();
		enterArea();
		break;
	case 14:
		// Moving down a ladder within the Flub maze
		R2_GLOBALS._flubMazeArea -= 35;
		initArea();
		enterArea();
		break;
	case 15:
		R2_GLOBALS._flubMazeArea -= 7;
		initArea();
		enterArea();
		break;
	case 16:
		// Moving west within the Flub maze
	// No break on purpose
	case 1961:
		--R2_GLOBALS._flubMazeArea;
		initArea();
		enterArea();
		break;
	case 17: {
		_sceneMode = 13;
		R2_GLOBALS._flubMazeEntryDirection = 3;
		_vampireActive = false;
		R2_GLOBALS._player.disableControl(CURSOR_WALK);
		R2_GLOBALS._player._canWalk = true;
		R2_GLOBALS._player.setVisage(22);
		R2_GLOBALS._player.animate(ANIM_MODE_9);
		Common::Point pt(340, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		Common::Point pt2(289, 160);
		NpcMover *mover2 = new NpcMover();
		_vampire.addMover(mover2, &pt2, NULL);
		}
		break;
	case 18: {
		_sceneMode = 16;
		R2_GLOBALS._flubMazeEntryDirection = 6;
		_vampireActive = false;
		R2_GLOBALS._player.disableControl(CURSOR_WALK);
		R2_GLOBALS._player._canWalk = true;
		R2_GLOBALS._player.setVisage(22);
		R2_GLOBALS._player.animate(ANIM_MODE_9);
		Common::Point pt(-20, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		Common::Point pt2(30, 160);
		NpcMover *mover2 = new NpcMover();
		_vampire.addMover(mover2, &pt2, NULL);
		}
		break;
	case 24:
		_KeypadWindow.remove();
		_sceneMode = 1966;
		_cube.setFrame(3);
		setAction(&_sequenceManager, this, 1966, &_containmentField, &_gem, NULL);
		break;
	case 1951:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sceneManager.changeScene(1945);
		break;
	case 1958:
		SceneItem::display(1950, 24, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		_doorExit._enabled = true;
		break;
	case 1959:
		R2_INVENTORY.setObjectScene(R2_SOAKED_FACEMASK, 0);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		_doorExit._enabled = true;
		break;
	case 1962:
	// No break on purpose
	case 1963:
		R2_GLOBALS._player.enableControl();
		_KeypadWindow.setup2(1971, 1, 1, 160, 135);
		break;
	case 1964:
	// No break on purpose
	case 1965:
		if (!R2_GLOBALS.getFlag(37))
			SceneItem::display(1950, 26, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);

		R2_GLOBALS._player.enableControl();
		break;
	case 1966:
		_containmentField.remove();
		if (R2_GLOBALS.getFlag(36)) {
			_sceneMode = 1964;
			setAction(&_sequenceManager, this, 1964, &R2_GLOBALS._player, NULL);
		} else {
			_sceneMode = 1965;
			setAction(&_sequenceManager, this, 1965, &R2_GLOBALS._player, NULL);
		}
		_gem.setDetails(1950, 9, -1, -1, 2, (SceneItem *) NULL);
		break;
	case 1967: {
		_sceneMode = 0;
		R2_INVENTORY.setObjectScene(R2_SAPPHIRE_BLUE, 2);
		_gem.remove();
		if (R2_GLOBALS.getFlag(36))
			R2_GLOBALS._player.setVisage(20);
		else
			R2_GLOBALS._player.setVisage(22);

		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		// This is a hack to work around a pathfinding issue. original destination is (218, 165)
		Common::Point pt(128, 165);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 1968:
		R2_GLOBALS._player.enableControl();
		R2_INVENTORY.setObjectScene(R2_ANCIENT_SCROLLS, 2);
		_scrolls.setFrame(2);
		if (R2_GLOBALS.getFlag(36))
			R2_GLOBALS._player.setVisage(20);
		else
			R2_GLOBALS._player.setVisage(22);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	}
}

void Scene1950::process(Event &event) {
	if ( (event.eventType == EVENT_BUTTON_DOWN)
			&& (R2_GLOBALS._player._uiEnabled)
			&& (R2_GLOBALS._events.getCursor() == R2_SOAKED_FACEMASK)
			&& (R2_GLOBALS._player._bounds.contains(event.mousePos))
			&& (R2_INVENTORY.getObjectScene(R2_SCRITH_KEY) == 0)) {
		event.handled = true;
		R2_GLOBALS._player.disableControl();
		_shaftExit._enabled = false;
		_doorExit._enabled = false;
		_sceneMode = 1959;
		setAction(&_sequenceManager, this, 1959, &R2_GLOBALS._player, NULL);
	}

	Scene::process(event);
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
