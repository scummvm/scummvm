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

#include "tsage/ringworld2/ringworld2_airduct.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 1200 - Air Ducts Maze
 *
 *--------------------------------------------------------------------------*/

Scene1200::Scene1200() {
	_nextCrawlDirection = 0;
	_field414 = 0;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;
	_fixupMaze = false;
}

void Scene1200::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_nextCrawlDirection);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
	s.syncAsSint16LE(_fixupMaze);
}

Scene1200::LaserPanel::LaserPanel() {
}

void Scene1200::LaserPanel::Jumper::init(int state) {
	_state = state;

	SceneActor::postInit();
	setup(1003, 1, 1);
	fixPriority(255);

	switch (_state) {
	case 1:
		switch (R2_GLOBALS._ductMazePanel1State) {
		case 1:
			setFrame2(2);
			setPosition(Common::Point(129, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(135, 95));
			break;
		default:
			break;
		}
		break;
	case 2:
		switch (R2_GLOBALS._ductMazePanel2State) {
		case 1:
			setFrame2(2);
			setPosition(Common::Point(152, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(158, 122));
			break;
		case 3:
			setFrame2(3);
			setPosition(Common::Point(135, 122));
			break;
		default:
			break;
		}
		break;
	case 3:
		switch (R2_GLOBALS._ductMazePanel3State) {
		case 1:
			setFrame2(3);
			setPosition(Common::Point(158, 95));
			break;
		case 2:
			setFrame2(2);
			setPosition(Common::Point(175, 101));
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	setDetails(1200, 12, -1, -1, 2, (SceneItem *) NULL);
}

bool Scene1200::LaserPanel::Jumper::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._sound2.play(260);
	switch (_state) {
	case 1:
		if (R2_GLOBALS._ductMazePanel1State == 1) {
			R2_GLOBALS._ductMazePanel1State = 2;
			setFrame2(3);
			setPosition(Common::Point(135, 95));
		} else {
			R2_GLOBALS._ductMazePanel1State = 1;
			setFrame2(2);
			setPosition(Common::Point(129, 101));
		}
		break;
	case 2:
		++R2_GLOBALS._ductMazePanel2State;
		if (R2_GLOBALS._ductMazePanel2State == 4)
			R2_GLOBALS._ductMazePanel2State = 1;

		switch (R2_GLOBALS._ductMazePanel2State) {
		case 1:
			setFrame2(2);
			setPosition(Common::Point(152, 101));
			break;
		case 2:
			setFrame2(3);
			setPosition(Common::Point(158, 122));
			break;
		case 3:
			setFrame2(3);
			setPosition(Common::Point(135, 122));
			break;
		default:
			break;
		}
		break;
	case 3:
		if (R2_GLOBALS._ductMazePanel3State == 1) {
			R2_GLOBALS._ductMazePanel3State = 2;
			setFrame2(2);
			setPosition(Common::Point(175, 101));
		} else {
			R2_GLOBALS._ductMazePanel3State = 1;
			setFrame2(3);
			setPosition(Common::Point(158, 95));
		}
		break;
	default:
		break;
	}

	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;
	scene->_field418 = 0;

	if ((R2_GLOBALS._ductMazePanel1State == 1) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 1))
		scene->_field418 = 1;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 1))
		scene->_field418 = 2;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 2))
		scene->_field418 = 3;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 3) && (R2_GLOBALS._ductMazePanel3State == 1))
		scene->_field418 = 4;

	return true;
}

void Scene1200::LaserPanel::postInit(SceneObjectList *OwnerList) {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;

	scene->_field41A = 1;
	R2_GLOBALS._events.setCursor(CURSOR_USE);
	setup2(1003, 1, 1, 100, 40);
	setup3(1200, 11, -1, -1);
	R2_GLOBALS._sound2.play(259);
	_jumper1.init(1);
	_jumper2.init(2);
	_jumper3.init(3);

	R2_GLOBALS._player._canWalk = false;
}

void Scene1200::LaserPanel::remove() {
	Scene1200 *scene = (Scene1200 *)R2_GLOBALS._sceneManager._scene;

	scene->_field41A = 0;
	scene->_sceneAreas.remove(&_jumper1);
	scene->_sceneAreas.remove(&_jumper2);
	scene->_sceneAreas.remove(&_jumper3);
	_jumper1.remove();
	_jumper2.remove();
	_jumper3.remove();

	ModalWindow::remove();
	R2_GLOBALS._player._canWalk = true;
}

void Scene1200::postInit(SceneObjectList *OwnerList) {
	loadScene(1200);
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene < 3200)
		R2_GLOBALS._sound1.play(257);

	_nextCrawlDirection = CRAWL_EAST;
	_field414 = 0;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;

	if ((R2_GLOBALS._ductMazePanel1State == 1) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 1))
		_field418 = 1;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 1))
		_field418 = 2;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 1) && (R2_GLOBALS._ductMazePanel3State == 2))
		_field418 = 3;
	else if ((R2_GLOBALS._ductMazePanel1State == 2) && (R2_GLOBALS._ductMazePanel2State == 3) && (R2_GLOBALS._ductMazePanel3State == 1))
		_field418 = 4;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player.setup(3156, 1, 6);
	R2_GLOBALS._player.setPosition(Common::Point(160, 70));
	R2_GLOBALS._player._numFrames = 10;
	R2_GLOBALS._player._oldCharacterScene[R2_MIRANDA] = 1200;

	_actor1.postInit();
	_actor1.hide();

	_mazeUI.setDisplayBounds(Rect(110, 20, 210, 120));

	_mazeUI.postInit();
	_mazeUI.load(1);
	_mazeUI.setMazePosition(R2_GLOBALS._ventCellPos);

	R2_GLOBALS._player.enableControl();
	_item1.setDetails(Rect(0, 0, 320, 200), 1200, 0, 1, 2, 1, NULL);
}

void Scene1200::signal() {
	switch (_sceneMode++) {
	case 1:
	// No break on purpose
	case 1200:
	// No break on purpose
	case 1201:
	// No break on purpose
	case 1202:
	// No break on purpose
	case 1203:
		R2_GLOBALS._player.enableControl();
		// CHECKME: The original is calling _eventManager.waitEvent();
		_sceneMode = 2;
		break;
	case 10:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(1);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 11:
	// No break on purpose
	case 21:
	// No break on purpose
	case 31:
	// No break on purpose
	case 41:
		_field416 = 0;
		break;
	case 12:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 1, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 13:
	// No break on purpose
	case 16:
	// No break on purpose
	case 23:
	// No break on purpose
	case 26:
	// No break on purpose
	case 33:
	// No break on purpose
	case 36:
	// No break on purpose
	case 43:
	// No break on purpose
	case 46:
		R2_GLOBALS._player.setFrame(4);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 15:
	// No break on purpose
	case 25:
	// No break on purpose
	case 35:
	// No break on purpose
	case 45:
		_field414 = 20;
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 20:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 22:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 2, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 30:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 32:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 3, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 40:
		_field416 = 1;
		_field414 = 6;
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.setStrip(4);
		R2_GLOBALS._player.setFrame(5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 42:
		_field414 = 14;
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player.setup(3155, 4, 4);
		R2_GLOBALS._player.setPosition(Common::Point(160, 70));
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		break;
	case 50:
	// No break on purpose
	case 55:
	// No break on purpose
	case 60:
		R2_GLOBALS._player.setup(3156, 5, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 51:
	// No break on purpose
	case 56:
	// No break on purpose
	case 117:
		R2_GLOBALS._player.setup(3157, 1, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 52:
	// No break on purpose
	case 82:
	// No break on purpose
	case 118:
		R2_GLOBALS._player.setup(3156, 3, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 57:
	// No break on purpose
	case 91:
	// No break on purpose
	case 96:
		R2_GLOBALS._player.setup(3157, 2, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 58:
	// No break on purpose
	case 92:
	// No break on purpose
	case 122:
		R2_GLOBALS._player.setup(3156, 2, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 61:
		R2_GLOBALS._player.setup(3157, 4, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 62:
	// No break on purpose
	case 72:
	// No break on purpose
	case 98:
		R2_GLOBALS._player.setup(3156, 4, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 70:
	// No break on purpose
	case 75:
	// No break on purpose
	case 80:
		R2_GLOBALS._player.setup(3156, 6, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 71:
	// No break on purpose
	case 76:
	// No break on purpose
	case 97:
		R2_GLOBALS._player.setup(3157, 3, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 77:
	// No break on purpose
	case 111:
	// No break on purpose
	case 116:
		R2_GLOBALS._player.setup(3157, 4, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 78:
	// No break on purpose
	case 102:
	// No break on purpose
	case 112:
		R2_GLOBALS._player.setup(3156, 1, 6);
		_sceneMode = 1;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 81:
		R2_GLOBALS._player.setup(3157, 2, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 90:
	// No break on purpose
	case 95:
	// No break on purpose
	case 100:
		R2_GLOBALS._player.setup(3156, 7, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 101:
		R2_GLOBALS._player.setup(3157, 1, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 110:
	// No break on purpose
	case 115:
	// No break on purpose
	case 120:
		R2_GLOBALS._player.setup(3156, 8, 1);
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 121:
		R2_GLOBALS._player.setup(3157, 3, 5);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	default:
		// CHECKME: The original is walling _eventManager.waitEvent();
		_sceneMode = 2;
		break;
	}
}

void Scene1200::process(Event &event) {
	if (_field414 != 0)
		return;

	Scene::process(event);

	if (!R2_GLOBALS._player._canWalk)
		return;

	if (event.eventType == EVENT_BUTTON_DOWN) {
		Common::Point cellPos = R2_GLOBALS._ventCellPos;
		_mazeUI.pixelToCellXY(cellPos);

		int cellId = _mazeUI.getCellFromPixelXY(event.mousePos);
		switch (R2_GLOBALS._events.getCursor()) {
		case CURSOR_WALK:
			event.handled = true;
			if ((event.mousePos.x > 179) && (event.mousePos.x < 210) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				startCrawling(CRAWL_EAST);

			if ((event.mousePos.x > 109) && (event.mousePos.x < 140) && (event.mousePos.y > 50) && (event.mousePos.y < 89))
				startCrawling(CRAWL_WEST);

			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 89) && (event.mousePos.y < 120))
				startCrawling(CRAWL_SOUTH);

			if ((event.mousePos.x > 140) && (event.mousePos.x < 179) && (event.mousePos.y > 19) && (event.mousePos.y < 50))
				startCrawling(CRAWL_NORTH);
			break;
		case CURSOR_USE:
			if (cellId > 36) {
				if ( ((cellPos.x == 3)  && (cellPos.y == 33))
					|| ((cellPos.x == 7)  && (cellPos.y == 33))
					|| ((cellPos.x == 33) && (cellPos.y == 41))
					|| ((cellPos.x == 5)  && (cellPos.y == 5))
					|| ((cellPos.x == 13) && (cellPos.y == 21))
					|| ((cellPos.x == 17) && (cellPos.y == 21))
					|| ((cellPos.x == 17) && (cellPos.y == 5))
					|| ((cellPos.x == 17) && (cellPos.y == 9))
					|| ((cellPos.x == 29) && (cellPos.y == 17))
					|| ((cellPos.x == 33) && (cellPos.y == 17))
					|| ((cellPos.x == 35) && (cellPos.y == 17))
					|| ((cellPos.x == 41) && (cellPos.y == 21)) ) {
					_laserPanel.postInit();
					event.handled = true;
				}
			}

			if ((cellId == 1) || (cellId == 4) || (cellId == 11) || (cellId == 14)) {
				if ( ((cellPos.x == 3)  && (cellPos.y == 9))
				  || ((cellPos.x == 11)  && (cellPos.y == 27))
				  || ((cellPos.x == 17)  && (cellPos.y == 7))
				  || ((cellPos.x == 17)  && (cellPos.y == 27))
				  || ((cellPos.x == 17)  && (cellPos.y == 33))
				  || (cellPos.x == 33) ) {
					switch (cellPos.x) {
					case 3:
						R2_GLOBALS._sceneManager.changeScene(3150);
						break;
					case 33:
						if (R2_GLOBALS._scientistConvIndex >= 4)
							R2_GLOBALS._sceneManager.changeScene(3250);
						else
							SceneItem::display(1200, 6, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, LIST_END);
						break;
					default:
						SceneItem::display(1200, 5, 0, 280, 1, 160, 9, 1, 2, 20, 7, 154, LIST_END);
						break;
					}
					event.handled = true;
				}
			}
			break;
		case CURSOR_LOOK:
			if ((cellId == 1) || (cellId == 4) || (cellId == 11) || (cellId == 14)) {
				event.handled = true;
				switch (cellPos.x) {
				case 3:
					// It was your cell.
					SceneItem::display(1200, 8, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
					break;
				case 9:
						R2_GLOBALS._sceneManager.changeScene(3240);
						break;
				case 11:
					if (cellPos.y == 27)
						R2_GLOBALS._sceneManager.changeScene(3210);
					else
						// A vent grill
						SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
					break;
				case 17:
					switch (cellPos.y) {
					case 5:
						R2_GLOBALS._sceneManager.changeScene(3230);
						break;
					case 21:
						R2_GLOBALS._sceneManager.changeScene(3220);
						break;
					case 33:
						R2_GLOBALS._sceneManager.changeScene(3200);
						break;
					default:
						// A vent grill
						SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
						break;
					}
					break;
				case 33:
					R2_GLOBALS._sceneManager.changeScene(3245);
					break;
				default:
					SceneItem::display(1200, 10, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
					break;
				}
			}
			if (cellId > 36) {
				// "An anti-pest laser"
				event.handled = true;
				SceneItem::display(1200, 9, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			}
			break;
		case CURSOR_TALK:
			event.handled = true;
			break;
		default:
			return;
		}
	} else if (event.eventType == EVENT_KEYPRESS) {
		if (_field414) {
			event.handled = false;
			return;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_KP8:
		case Common::KEYCODE_UP:
			startCrawling(CRAWL_NORTH);
			break;
		case Common::KEYCODE_KP4:
		case Common::KEYCODE_LEFT:
			startCrawling(CRAWL_WEST);
			break;
		case Common::KEYCODE_KP6:
		case Common::KEYCODE_RIGHT:
			startCrawling(CRAWL_EAST);
			break;
		case Common::KEYCODE_KP2:
		case Common::KEYCODE_DOWN:
			startCrawling(CRAWL_SOUTH);
			break;
		default:
			event.handled = false;
			return;
			break;
		}
	} else
		return;
}

void Scene1200::dispatch() {
	Rect tmpRect;
	Scene::dispatch();

	if (_fixupMaze) {
		_mazeUI.setMazePosition(R2_GLOBALS._ventCellPos);
		//_mazeUI.draw();
		_fixupMaze = false;
	}

	if (_field414 != 0) {
		tmpRect.set(110, 20, 210, 120);
		_field414--;

		switch (_nextCrawlDirection) {
		case CRAWL_EAST:
			R2_GLOBALS._ventCellPos.x += 2;
			break;
		case CRAWL_WEST:
			R2_GLOBALS._ventCellPos.x -= 2;
			break;
		case CRAWL_SOUTH:
			R2_GLOBALS._ventCellPos.y += 2;
			break;
		case CRAWL_NORTH:
			R2_GLOBALS._ventCellPos.y -= 2;
			break;
		default:
			break;
		}

		_mazeUI.setMazePosition(R2_GLOBALS._ventCellPos);
		//_mazeUI.draw();

		if (_field416 != 0) {
			switch(_nextCrawlDirection) {
			case CRAWL_EAST:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x - 2, R2_GLOBALS._player._position.y));
				break;
			case CRAWL_WEST:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x + 2, R2_GLOBALS._player._position.y));
				break;
			case CRAWL_SOUTH:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 2));
				break;
			case CRAWL_NORTH:
				R2_GLOBALS._player.setPosition(Common::Point(R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 2));
				break;
			default:
				break;
			}
		}
		if (_field414 == 0) {
			if (_field416 == 0)
				R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			signal();
		}
	}
}

void Scene1200::saveCharacter(int characterIndex) {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::saveCharacter(characterIndex);
}

void Scene1200::startCrawling(CrawlDirection dir) {
	Common::Point cellPos = R2_GLOBALS._ventCellPos;
	_mazeUI.pixelToCellXY(cellPos);

	switch (dir) {
	case CRAWL_EAST:
		if ( ((_mazeUI.getCellFromPixelXY(Common::Point(200, 50)) > 36) || (_mazeUI.getCellFromPixelXY(Common::Point(200, 88)) > 36))
			&& ( ((cellPos.x == 3) && (cellPos.y == 33) && (_field418 != 4))
				|| ((cellPos.x == 13) && (cellPos.y == 21) && (_field418 != 2))
				|| ((cellPos.x == 29) && (cellPos.y == 17) && (_field418 != 1))
				|| ((cellPos.x == 33) && (cellPos.y == 41)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1200;
			setAction(&_sequenceManager, this, 1200, &_actor1, NULL);
		} else if (_mazeUI.getCellFromPixelXY(Common::Point(200, 69)) == 36) {
			switch (_nextCrawlDirection) {
			case CRAWL_EAST:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 15;
				else
					_sceneMode = 10;
				break;
			case CRAWL_WEST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 76;
				else
					_sceneMode = 75;
				break;
			case CRAWL_SOUTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 101;
				else
					_sceneMode = 100;
				break;
			case CRAWL_NORTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 111;
				else
					_sceneMode = 110;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_nextCrawlDirection = 1;
			signal();
		}
		break;
	case CRAWL_WEST:
		if ( ((_mazeUI.getCellFromPixelXY(Common::Point(120, 50)) > 36) || (_mazeUI.getCellFromPixelXY(Common::Point(120, 88)) > 36))
			&& ( ((cellPos.x == 7) && (cellPos.y == 33) && (_field418 != 4))
				|| ((cellPos.x == 17) && (cellPos.y == 21) && (_field418 != 2))
				|| ((cellPos.x == 33) && (cellPos.y == 17) && (_field418 != 1))
				|| ((cellPos.x == 5) && (cellPos.y == 5)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1201;
			setAction(&_sequenceManager, this, 1201, &_actor1, NULL);
		} else if (_mazeUI.getCellFromPixelXY(Common::Point(120, 69)) == 36) {
			switch (_nextCrawlDirection) {
			case CRAWL_EAST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 56;
				else
					_sceneMode = 55;
				break;
			case CRAWL_WEST:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 25;
				else
					_sceneMode = 20;
				break;
			case CRAWL_SOUTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 91;
				else
					_sceneMode = 90;
				break;
			case CRAWL_NORTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 121;
				else
					_sceneMode = 120;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_nextCrawlDirection = 2;
			signal();
		}
		break;
	case CRAWL_SOUTH:
		if ( ((_mazeUI.getCellFromPixelXY(Common::Point(140, 110)) > 36) || (_mazeUI.getCellFromPixelXY(Common::Point(178, 110)) > 36))
			&& ( ((cellPos.x == 17) && (cellPos.y == 5) && (_field418 != 3))
				|| ((cellPos.x == 41) && (cellPos.y == 21)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1203;
			setAction(&_sequenceManager, this, 1203, &_actor1, NULL);
		} else if (_mazeUI.getCellFromPixelXY(Common::Point(160, 110)) == 36) {
			switch (_nextCrawlDirection) {
			case CRAWL_EAST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 51;
				else
					_sceneMode = 50;
				break;
			case CRAWL_WEST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 81;
				else
					_sceneMode = 80;
				break;
			case CRAWL_SOUTH:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 35;
				else
					_sceneMode = 30;
				break;
			case CRAWL_NORTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 116;
				else
					_sceneMode = 115;
				break;
			default:
				break;
			}
			R2_GLOBALS._player.disableControl();
			_nextCrawlDirection = 3;
			signal();
		}
		break;
	case CRAWL_NORTH:
		if ( ((_mazeUI.getCellFromPixelXY(Common::Point(140, 30)) > 36) || (_mazeUI.getCellFromPixelXY(Common::Point(178, 30)) > 36))
			&& ( ((cellPos.x == 17) && (cellPos.y == 9) && (_field418 != 3))
				|| ((cellPos.x == 35) && (cellPos.y == 17)) )
				)	{
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1202;
			setAction(&_sequenceManager, this, 1202, &_actor1, NULL);
		} else if (_mazeUI.getCellFromPixelXY(Common::Point(160, 30)) == 36) {
			switch (_nextCrawlDirection) {
			case CRAWL_EAST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 61;
				else
					_sceneMode = 60;
				break;
			case CRAWL_WEST:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 71;
				else
					_sceneMode = 70;
				break;
			case CRAWL_SOUTH:
				if (R2_GLOBALS._player._visage == 3156)
					_sceneMode = 96;
				else
					_sceneMode = 95;
				break;
			case CRAWL_NORTH:
				if (R2_GLOBALS._player._visage == 3155)
					_sceneMode = 45;
				else
					_sceneMode = 40;
				break;
			default:
				_sceneMode = 1;
				R2_GLOBALS._player.setup(3156, 4, 6);
				break;
			}
			R2_GLOBALS._player.disableControl();
			_nextCrawlDirection = 4;
			signal();
		}
		break;
	default:
		break;
	}
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
