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

#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 2000 - Spill Mountains
 *
 *--------------------------------------------------------------------------*/

void Scene2000::initPlayer() {
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._player.disableControl();

	switch (_mazePlayerMode) {
	case 0:
		R2_GLOBALS._player.setStrip(5);
		if (_westExit._enabled) {
			if (_eastExit._enabled)
				R2_GLOBALS._player.setPosition(Common::Point(140, 129));
			else
				R2_GLOBALS._player.setPosition(Common::Point(20, 129));
		} else
			R2_GLOBALS._player.setPosition(Common::Point(245, 129));
		R2_GLOBALS._player.enableControl();
		break;
	case 1:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2001;
		else
			_sceneMode = 2021;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2002;
		else
			_sceneMode = 2022;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2000;
		else
			_sceneMode = 2020;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2005;
		else
			_sceneMode = 2025;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 5:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2004;
		else
			_sceneMode = 2024;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2009;
		else
			_sceneMode = 2029;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2008;
		else
			_sceneMode = 2028;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2013;
		else
			_sceneMode = 2033;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2012;
		else
			_sceneMode = 2032;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2016;
		else
			_sceneMode = 2036;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_sceneMode = 2038;
		else
			_sceneMode = 2040;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
	for (int i = 0; i < 11; i++) {
		if (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] == R2_GLOBALS._spillLocation[3 + i])
			_persons[i].show();
	}

	if ((R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER])
			&& (R2_GLOBALS._spillLocation[R2_QUINN] == R2_GLOBALS._spillLocation[R2_SEEKER])) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			// Seeker is in room with Quinn
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			// Quinn is in room with Seeker
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		if (_westExit._enabled) {
			if (_eastExit._enabled)
				_companion.setPosition(Common::Point(180, 128));
			else
				_companion.setPosition(Common::Point(75, 128));
		} else
			_companion.setPosition(Common::Point(300, 128));
	}
}

void Scene2000::initExits() {
	_westExit._enabled = true;
	_eastExit._enabled = true;
	_southExit._enabled = false;
	_northExit._enabled = false;
	_doorExit._enabled = false;

	_westExit._insideArea = false;
	_eastExit._insideArea = false;
	_southExit._insideArea = false;
	_northExit._insideArea = false;
	_doorExit._insideArea = false;

	_westExit._moving = false;
	_eastExit._moving = false;
	_southExit._moving = false;
	_northExit._moving = false;
	_doorExit._moving = false;

	for (int i = 0; i < 11; i++)
		_persons[i].hide();

	_companion.remove();

	switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex]) {
	case 3:
	case 10:
	case 16:
	case 21:
		_doorExit._enabled = true;
		_doorExit._bounds.set(61, 68, 90, 125);
		_doorExit.setDest(Common::Point(92, 129));
		_doorExit._cursorNum = EXITCURSOR_W;
		break;
	case 4:
	case 12:
	case 25:
	case 34:
		_doorExit._enabled = true;
		_doorExit._bounds.set(230, 68, 259, 125);
		_doorExit.setDest(Common::Point(244, 129));
		_doorExit._cursorNum = EXITCURSOR_E;
		break;
	default:
		break;
	}

	switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] - 1) {
	case 0:
	case 6:
	case 13:
	case 18:
	case 22:
	case 27:
	case 30:
		_westExit._enabled = false;
		loadScene(2225);
		R2_GLOBALS._walkRegions.load(2225);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 1:
	case 19:
		_southExit._enabled = true;
		_southExit._bounds.set(71, 130, 154, 168);
		_southExit.setDest(Common::Point(94, 129));
		_southExit._cursorNum = EXITCURSOR_SE;
		loadScene(2300);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._walkRegions.load(2000);
		break;
	case 2:
	case 9:
	case 15:
	case 20:
		loadScene(2150);
		R2_GLOBALS._walkRegions.load(2000);
		switch(R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
		case 2400:
			_mazePlayerMode = 1;
			break;
		case 2425:
		case 2430:
		case 2435:
		case 2450:
			_mazePlayerMode = 3;
			break;
		default:
			if (!_exitingFlag)
				_mazePlayerMode = 0;
			break;
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 3:
	case 11:
	case 24:
	case 33:
		loadScene(2175);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
			if (!_exitingFlag)
				_mazePlayerMode = 0;
		} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2350)
			_mazePlayerMode = 1;
		else
			_mazePlayerMode = 10;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 4:
	case 8:
		loadScene(2000);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1900)
			_mazePlayerMode = 1;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 5:
	case 12:
	case 17:
	case 21:
	case 26:
		loadScene(2200);
		R2_GLOBALS._walkRegions.load(2200);
		_eastExit._enabled = false;
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1900)
			_mazePlayerMode = 2;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 7:
	case 29:
		_northExit._enabled = true;
		_northExit._bounds.set(138, 83, 211, 125);
		_northExit.setDest(Common::Point(188, 129));
		_northExit._cursorNum = EXITCURSOR_NW;
		loadScene(2250);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2500)
			_mazePlayerMode = 1;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 10:
	case 25:
		_southExit._enabled = true;
		_southExit._bounds.set(78, 130, 148, 168);
		_southExit.setDest(Common::Point(100, 129));
		_southExit._cursorNum = EXITCURSOR_SE;
		loadScene(2075);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 14:
		_southExit._enabled = true;
		_southExit._bounds.set(160, 130, 248, 168);
		_southExit.setDest(Common::Point(225, 129));
		_southExit._cursorNum = EXITCURSOR_SW;
		loadScene(2325);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 16:
	case 31:
		_northExit._enabled = true;
		_northExit._bounds.set(122, 83, 207, 125);
		_northExit.setDest(Common::Point(210, 129));
		_northExit._cursorNum = EXITCURSOR_NW;
		loadScene(2125);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2400)
			_mazePlayerMode = 2;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 23:
		_northExit._enabled = true;
		_northExit._bounds.set(108, 83, 184, 125);
		_northExit.setDest(Common::Point(135, 129));
		_northExit._cursorNum = EXITCURSOR_NE;
		loadScene(2275);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 28:
		_southExit._enabled = true;
		_southExit._bounds.set(171, 130, 241, 168);
		_southExit.setDest(Common::Point(218, 129));
		_southExit._cursorNum = EXITCURSOR_SW;
		loadScene(2050);
		R2_GLOBALS._walkRegions.load(2000);
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2350)
			_mazePlayerMode = 11;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 32:
		loadScene(2025);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	default:
		break;
	}
	_exitingFlag = false;
	R2_GLOBALS._uiElements.show();
}

void Scene2000::Action1::signal() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 0: {
		_actionIndex = 1;
		Common::Point pt(-20, 127);
		NpcMover *mover = new NpcMover();
		scene->_persons[_state].addMover(mover, &pt, scene);
		break;
		}
	case 1:
		scene->_persons[_state].setPosition(Common::Point(340, 127));
		--R2_GLOBALS._spillLocation[4 + _state];
		_actionIndex = 0;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._spillLocation[4] == 1)
				_actionIndex = 10;
			break;
		case 2:
			if (R2_GLOBALS._spillLocation[6] == 7)
				_actionIndex = 10;
			break;
		case 4:
			if (R2_GLOBALS._spillLocation[8] == 14)
				_actionIndex = 10;
			break;
		case 6:
			if (R2_GLOBALS._spillLocation[10] == 19)
				_actionIndex = 10;
			break;
		case 7:
			if (R2_GLOBALS._spillLocation[11] == 23)
				_actionIndex = 10;
			break;
		default:
			break;
		}

		if (R2_GLOBALS._spillLocation[3 + _state] == R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex])
			scene->_persons[_state].show();
		else
			scene->_persons[_state].hide();

		signal();
		break;
	case 5: {
		_actionIndex = 6;
		Common::Point pt(340, 127);
		NpcMover *mover = new NpcMover();
		scene->_persons[_state].addMover(mover, &pt, this);
		break;
		}
	case 6:
		scene->_persons[_state].setPosition(Common::Point(-20, 127));
		++R2_GLOBALS._spillLocation[3 + _state];
		_actionIndex = 5;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._spillLocation[4] == 5)
				_actionIndex = 15;
			break;
		case 2:
			if (R2_GLOBALS._spillLocation[6] == 13)
				_actionIndex = 15;
			break;
		case 4:
			if (R2_GLOBALS._spillLocation[8] == 16)
				_actionIndex = 15;
			break;
		case 6:
			if (R2_GLOBALS._spillLocation[10] == 22)
				_actionIndex = 15;
			break;
		case 7:
			if (R2_GLOBALS._spillLocation[11] == 27)
				_actionIndex = 15;
			break;
		default:
			break;
		}

		if (R2_GLOBALS._spillLocation[3 + _state] == R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex])
			scene->_persons[_state].show();
		else
			scene->_persons[_state].hide();

		signal();
		break;
	case 10: {
		Common::Point pt(290, 127);
		NpcMover *mover = new NpcMover();
		scene->_persons[_state].addMover(mover, &pt, this);
		_actionIndex = 11;
		break;
		}
	case 11:
		if (_state == 1)
			scene->_persons[0].setStrip(1);
		else if (_state == 5)
			scene->_persons[4].setStrip(1);
		setDelay(600);
		_actionIndex = 12;
		break;
	case 12:
		if (_state == 1)
			scene->_persons[0].setStrip(2);
		else if (_state == 5)
			scene->_persons[4].setStrip(2);
		scene->_persons[_state].setStrip(1);
		_actionIndex = 5;
		signal();
		break;
	case 15:
		if ((R2_GLOBALS._spillLocation[3 + _state] == 13) || (R2_GLOBALS._spillLocation[3 + _state] == 22) || (R2_GLOBALS._spillLocation[3 + _state] == 27)) {
			Common::Point pt(30, 127);
			NpcMover *mover = new NpcMover();
			scene->_persons[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		} else {
			Common::Point pt(120, 127);
			NpcMover *mover = new NpcMover();
			scene->_persons[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		}
		break;
	case 16:
		if (_state == 1)
			scene->_persons[2].setStrip(2);
		else if (_state == 8)
			scene->_persons[9].setStrip(2);
		setDelay(600);
		_actionIndex = 17;
		break;
	case 17:
		if (_state == 1)
			scene->_persons[2].setStrip(1);
		else if (_state == 8)
			scene->_persons[9].setStrip(1);
		scene->_persons[_state].setStrip(2);
		_actionIndex = 0;
		break;
	case 99:
		error("99");
		break;
	default:
		break;
	}
}

void Scene2000::WestExit::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 10;

	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2000::EastExit::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 11;

	Common::Point pt(330, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2000::SouthExit::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 12;

	switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex]) {
	case 2:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 8;
		break;
	case 11:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 17;
		break;
	case 15:
		scene->_mazePlayerMode = 8;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 24;
		break;
	case 20:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 30;
		break;
	case 26:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 32;
		break;
	case 29:
		scene->_mazePlayerMode = 11;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 29;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 4:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2003, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2023, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2007, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2027, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2011, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2031, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->_sceneMode = 2039;
		else
			scene->_sceneMode = 2041;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		break;

	default:
		break;
	}
}

void Scene2000::NorthExit::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_exitingFlag = true;
	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 13;

	switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex]) {
	case 8:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 2;
		break;
	case 17:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 11;
		break;
	case 24:
		scene->_mazePlayerMode = 9;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 15;
		break;
	case 30:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 20;
		break;
	case 32:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 26;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 5:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2006, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2026, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2010, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2030, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2014, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2034, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

void Scene2000::DoorExit::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl(CURSOR_ARROW);
	scene->_sceneMode = 14;

	switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex]) {
	case 3:
		scene->_mazePlayerMode = 1;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		scene->_mazePlayerMode = 7;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		scene->_mazePlayerMode = 8;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 12:
		scene->_mazePlayerMode = 3;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 16:
		scene->_mazePlayerMode = 4;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 21:
		scene->_mazePlayerMode = 5;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 25:
		scene->_mazePlayerMode = 2;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 34:
		scene->_mazePlayerMode = 6;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

Scene2000::Scene2000(): SceneExt() {
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._spillLocation[R2_QUINN] = 21;
		R2_GLOBALS._spillLocation[R2_SEEKER] = 21;
	}
	if ((R2_GLOBALS._player._characterScene[R2_GLOBALS._player._characterIndex]
			!= R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex])
			&& (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] != 2350)) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 0;
	}

	_exitingFlag = false;
	_mazePlayerMode = 0;
}

void Scene2000::postInit(SceneObjectList *OwnerList) {
	_westExit.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2000);
	_westExit.setDest(Common::Point(14, 129));
	_eastExit.setDetails(Rect(305, 100, 320, 140), EXITCURSOR_E, 2000);
	_eastExit.setDest(Common::Point(315, 129));
	_southExit.setDetails(Rect(71, 130, 154, 168), EXITCURSOR_S, 2000);
	_southExit.setDest(Common::Point(94, 129));
	_northExit.setDetails(Rect(138, 83, 211, 125), EXITCURSOR_N, 2000);
	_northExit.setDest(Common::Point(188, 129));
	_doorExit.setDetails(Rect(61, 68, 90, 125), EXITCURSOR_W, 2000);
	_doorExit.setDest(Common::Point(92, 129));

	R2_GLOBALS._sound1.play(200);
	initExits();
	g_globals->_sceneManager._fadeMode = FADEMODE_IMMEDIATE;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	_action1._state = 8;
	_action2._state = 1;
	_action3._state = 5;
	_action4._state = 7;
	_action5._state = 3;

	for (int i = 0; i < 11; i++)
		_persons[i].postInit();

	_persons[0].setVisage(2000);
	_persons[0].setStrip(2);
	_persons[0].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[1].setVisage(2001);
	_persons[1].setStrip(2);
	_persons[1].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[2].setVisage(2003);
	_persons[2].setStrip(1);
	_persons[2].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[3].setVisage(2007);
	_persons[3].setStrip(2);
	_persons[3].setDetails(2001, 12, -1, -1, 1, (SceneItem *)NULL);

	_persons[4].setVisage(2004);
	_persons[4].setStrip(2);
	_persons[4].setDetails(2001, 19, -1, -1, 1, (SceneItem *)NULL);

	_persons[5].setVisage(2003);
	_persons[5].setStrip(2);
	_persons[5].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[6].setVisage(2000);
	_persons[6].setStrip(1);
	_persons[6].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[7].setVisage(2000);
	_persons[7].setStrip(2);
	_persons[7].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[8].setVisage(2000);
	_persons[8].setStrip(2);
	_persons[8].setDetails(2001, 0, -1, -1, 1, (SceneItem *)NULL);

	_persons[9].setVisage(2006);
	_persons[9].setStrip(1);
	_persons[9].setDetails(2001, 6, -1, -1, 1, (SceneItem *)NULL);

	_persons[10].setVisage(2007);
	_persons[10].setStrip(1);
	_persons[10].setDetails(2001, 12, -1, -1, 1, (SceneItem *)NULL);

	for (int i = 0; i < 11; i++) {
		_persons[i].animate(ANIM_MODE_1, NULL);
		_persons[i]._moveDiff.x = 3;
		_persons[i]._moveRate = 8;
		_persons[i].hide();
		switch (i - 1) {
		case 0:
			if (R2_GLOBALS._spillLocation[3 + i]  == 1)
				++R2_GLOBALS._spillLocation[3 + i];
			else if (R2_GLOBALS._spillLocation[3 + i]  == 5)
				--R2_GLOBALS._spillLocation[3 + i];
			break;
		case 2:
			if (R2_GLOBALS._spillLocation[3 + i]  == 7)
				++R2_GLOBALS._spillLocation[3 + i];
			else if (R2_GLOBALS._spillLocation[3 + i]  == 13)
				--R2_GLOBALS._spillLocation[3 + i];
			break;
		case 4:
			if (R2_GLOBALS._spillLocation[3 + i]  == 14)
				++R2_GLOBALS._spillLocation[3 + i];
			else if (R2_GLOBALS._spillLocation[3 + i]  == 16)
				--R2_GLOBALS._spillLocation[3 + i];
			break;
		case 6:
			if (R2_GLOBALS._spillLocation[3 + i]  == 19)
				++R2_GLOBALS._spillLocation[3 + i];
			else if (R2_GLOBALS._spillLocation[3 + i]  == 22)
				--R2_GLOBALS._spillLocation[3 + i];
			break;
		case 8:
			if (R2_GLOBALS._spillLocation[3 + i]  == 23)
				++R2_GLOBALS._spillLocation[3 + i];
			else if (R2_GLOBALS._spillLocation[3 + i]  == 27)
				--R2_GLOBALS._spillLocation[3 + i];
			break;
		default:
			break;
		}
		switch (R2_GLOBALS._spillLocation[3 + i] - 1) {
		case 0:
		case 6:
		case 13:
		case 18:
		case 22:
		case 27:
		case 30:
			_persons[i].setPosition(Common::Point(265, 127));
			break;
		case 5:
		case 12:
		case 17:
		case 21:
		case 26:
			_persons[i].setPosition(Common::Point(55, 127));
			break;
		default:
			_persons[i].setPosition(Common::Point(160, 127));
			break;
		}
	}
	_persons[1].setAction(&_action2);
	_persons[3].setAction(&_action5);
	_persons[5].setAction(&_action4);
	_persons[8].setAction(&_action1);

	initPlayer();

	_background.setDetails(Rect(0, 0, 320, 200), 2000, 0, -1, 23, 1, NULL);

	SceneExt::postInit();
}

void Scene2000::remove() {
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2000::signal() {
	switch (_sceneMode) {
	case 10:
		// Leaving left-hand side of scene
		if (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] == 6)
			g_globals->_sceneManager.changeScene(1900);
		else {
			_mazePlayerMode = 1;
			--R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
		}
	break;
	case 11:
		// Leaving right-hand side of scene
		switch (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex]) {
		case 5:
			g_globals->_sceneManager.changeScene(1900);
			break;
		case 30:
			g_globals->_sceneManager.changeScene(2500);
			break;
		case 34:
			g_globals->_sceneManager.changeScene(2350);
			break;
		default:
			_mazePlayerMode = 2;
			++R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
			break;
		}
		break;
	case 12:
	case 13:
		// Top/bottom scene exits
		initExits();
		initPlayer();
		break;
	case 14:
		switch (_mazePlayerMode - 1) {
		case 0:
			g_globals->_sceneManager.changeScene(2450);
			break;
		case 1:
			g_globals->_sceneManager.changeScene(2440);
			break;
		case 2:
			g_globals->_sceneManager.changeScene(2435);
			break;
		case 3:
			g_globals->_sceneManager.changeScene(2430);
			break;
		case 4:
			g_globals->_sceneManager.changeScene(2425);
			break;
		case 5:
			g_globals->_sceneManager.changeScene(2525);
			break;
		case 6:
			g_globals->_sceneManager.changeScene(2530);
			break;
		case 7:
			g_globals->_sceneManager.changeScene(2535);
			break;
		default:
			R2_GLOBALS._player.enableControl(CURSOR_WALK);
			break;
		}
		break;
	case 2039:
	case 2041:
		g_globals->_sceneManager.changeScene(2350);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene2000::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN) &&
			(R2_GLOBALS._events.getCursor() == CURSOR_CROSSHAIRS)) {

		Common::Point pt(event.mousePos.x, 129);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);

		event.handled = true;
	}
	Scene::process(event);
}

void Scene2000::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsByte(_exitingFlag);
	s.syncAsSint16LE(_mazePlayerMode);
}

/*--------------------------------------------------------------------------
 * Scene 2350 - Balloon Launch Platform
 *
 *--------------------------------------------------------------------------*/

bool Scene2350::Companion::startAction(CursorType action, Event &event) {
	if (action != R2_SENSOR_PROBE)
		return(SceneActor::startAction(action, event));
	return true;
}

bool Scene2350::Balloon::startAction(CursorType action, Event &event) {
	if ((action == R2_REBREATHER_TANK) && (R2_GLOBALS.getFlag(74))) {
		Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_person.postInit();
		scene->_sceneMode = 2355;
		scene->setAction(&scene->_sequenceManager, scene, 2355, &R2_GLOBALS._player, &scene->_person, NULL);
		return true;
	}

	return(SceneActor::startAction(action, event));
}

void Scene2350::ExitUp::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;
	_enabled = false;

	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 12;
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		scene->setAction(&scene->_sequenceManager, scene, 2350, &R2_GLOBALS._player, NULL);
	else
		scene->setAction(&scene->_sequenceManager, scene, 2352, &R2_GLOBALS._player, NULL);
}

void Scene2350::ExitWest::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_CROSSHAIRS);
	scene->_sceneMode = 11;

	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2350::postInit(SceneObjectList *OwnerList) {
	loadScene(2350);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	_stripManager.addSpeaker(&_pharishaSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 2350;

	_exitUp.setDetails(Rect(25, 83, 93, 125), EXITCURSOR_NW, 2350);
	_exitUp.setDest(Common::Point(80, 129));
	_exitWest.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2350);
	_exitWest.setDest(Common::Point(14, 129));

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1,  (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(135, 128));
	}
	_balloon.postInit();
	_harness.postInit();

	if (R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 2350) {
		_balloon.hide();
		_harness.hide();
	} else {
		_balloon.setup(2350, 0, 1);
		_balloon.setPosition(Common::Point(197, 101));
		_balloon.setDetails(2000, 12, -1, -1, 1, (SceneItem *)NULL);
		_balloon.fixPriority(10);
		_harness.setup(2350, 1, 2);
		_harness.setPosition(Common::Point(199, 129));
		_harness.setDetails(2000, 12, -1, -1, 1, (SceneItem *)NULL);
		_harness.fixPriority(10);
	}
	_background.setDetails(Rect(0, 0, 320, 200), 2000, 9, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		if (R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] != 34) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_sceneMode = 2351;
			else
				_sceneMode = 2353;
			setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		} else {
			_sceneMode = 10;
			R2_GLOBALS._player.setPosition(Common::Point(-20, 129));
			Common::Point pt(20, 129);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);

		}
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(100, 129));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2350;
}

void Scene2350::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2350::signal() {
	switch (_sceneMode) {
	case 11:
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 34;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 12:
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 29;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		_sceneMode = 21;
		_stripManager.start(712, this);
		break;
	case 21:
		R2_GLOBALS._player.disableControl();
		R2_INVENTORY.setObjectScene(R2_FLUTE, 1);
		_sceneMode = 2354;
		setAction(&_sequenceManager, this, 2354, &R2_GLOBALS._player, NULL);
		break;
	case 2354:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 2350);
		g_globals->_sceneManager.changeScene(2900);
		break;
	case 2355:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(711, this);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2400 - Spill Mountains: Large empty room
 *
 *--------------------------------------------------------------------------*/

void Scene2400::WestExit::changeScene() {
	Scene2400 *scene = (Scene2400 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 10;

	Common::Point pt(-10, 150);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2400::EastExit::changeScene() {
	Scene2400 *scene = (Scene2400 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(330, 150);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2400::postInit(SceneObjectList *OwnerList) {
	loadScene(2400);
	SceneExt::postInit();
	_westExit.setDetails(Rect(0, 125, 14, 165), EXITCURSOR_W, 2000);
	_westExit.setDest(Common::Point(14, 150));
	_eastExit.setDetails(Rect(305, 125, 320, 165), EXITCURSOR_E, 2000);
	_eastExit.setDest(Common::Point(315, 150));
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._spillLocation[R2_QUINN] == 16) {
		_sceneMode = 2400;
		setAction(&_sequenceManager, this, 2400, &R2_GLOBALS._player, NULL);
	} else {
		_sceneMode = 2401;
		setAction(&_sequenceManager, this, 2401, &R2_GLOBALS._player, NULL);
	}
}

void Scene2400::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._spillLocation[R2_QUINN] = 16;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 11:
		R2_GLOBALS._spillLocation[R2_QUINN] = 17;
		g_globals->_sceneManager.changeScene(2000);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2425 - Spill Mountains: The Hall Of Records
 *
 *--------------------------------------------------------------------------*/

bool Scene2425::RopeDest1::startAction(CursorType action, Event &event) {
	if ((action == R2_CURSOR_ROPE) && (!R2_GLOBALS.getFlag(84))) {
		Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2426;
		scene->setAction(&scene->_sequenceManager, scene, 2426, &R2_GLOBALS._player, &scene->_rope, NULL);
		R2_GLOBALS.setFlag(84);
		return true;
	} else if (action == R2_CURSOR_ROPE) {
		R2_GLOBALS._events.setCursor(CURSOR_USE);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		return NamedHotspot::startAction(CURSOR_USE, event);
	} else
		return NamedHotspot::startAction(action, event);
}

bool Scene2425::RopeDest2::startAction(CursorType action, Event &event) {
	if ((action == R2_CURSOR_ROPE) && (R2_GLOBALS.getFlag(84))) {
		Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2427;
		scene->setAction(&scene->_sequenceManager, scene, 2427, &R2_GLOBALS._player, &scene->_rope, NULL);
		R2_GLOBALS.clearFlag(84);
		return true;
	} else if (action == R2_CURSOR_ROPE) {
		R2_GLOBALS._events.setCursor(CURSOR_USE);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		return NamedHotspot::startAction(CURSOR_USE, event);
	} else
		return NamedHotspot::startAction(action, event);
}

bool Scene2425::Crevasse::startAction(CursorType action, Event &event) {
	if (action != R2_CURSOR_ROPE)
		return NamedHotspot::startAction(action, event);
	else {
		Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS.getFlag(84)) {
			scene->_sceneMode = 20;
			scene->setAction(&scene->_sequenceManager, scene, 2427, &R2_GLOBALS._player, &scene->_rope, NULL);
			R2_GLOBALS.clearFlag(84);
		} else {
			scene->_sceneMode = 2425;
			scene->setAction(&scene->_sequenceManager, scene, 2425, &R2_GLOBALS._player, &scene->_rope, NULL);
		}
		return true;
	}
}

bool Scene2425::Background::startAction(CursorType action, Event &event) {
	if (action != R2_CURSOR_ROPE)
		return NamedHotspot::startAction(action, event);
	else {
		R2_GLOBALS._events.setCursor(CURSOR_USE);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		return NamedHotspot::startAction(CURSOR_USE, event);
	}
}

bool Scene2425::Rope::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			R2_GLOBALS._events.setCursor(R2_CURSOR_ROPE);
			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
	} else if (R2_GLOBALS._events.getCursor() == R2_CURSOR_ROPE)
		return false;
	else
		return SceneActor::startAction(action, event);
}

bool Scene2425::Pictographs::startAction(CursorType action, Event &event) {
	if (action != R2_CURSOR_ROPE)
		return SceneActor::startAction(action, event);
	else {
		R2_GLOBALS._events.setCursor(CURSOR_USE);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		return SceneActor::startAction(CURSOR_USE, event);
	}
}

void Scene2425::SouthEastExit::changeScene() {
	Scene2425 *scene = (Scene2425 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(340, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}


void Scene2425::postInit(SceneObjectList *OwnerList) {
	loadScene(2425);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._player._characterIndex = R2_SEEKER;
		R2_GLOBALS._sceneManager._previousScene = 2000;
	}

	R2_GLOBALS._sound1.play(200);
	_southEastExit.setDetails(Rect(270, 136, 319, 168), EXITCURSOR_SE, 2000);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_pictographs1.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_pictographs1.setup(20, 5, 1);
			_pictographs1.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_pictographs1.setup(2008, 5, 1);
			_pictographs1.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_pictographs1.setPosition(Common::Point(250, 185));
	}

	_rope.postInit();
	if (R2_GLOBALS._sceneManager._previousScene == 2455)
		_rope.setup(2426, 1, 1);
	else
		_rope.setup(2426, 1, 2);

	_rope.setPosition(Common::Point(290, 9));
	_rope.fixPriority(20);
	_rope.setDetails(2455, 12, -1, -1, 1, (SceneItem *)NULL);
	_ropeDest1.setDetails(Rect(225, 52, 248, 65), 2425, -1, -1, -1, 1, NULL);
	_ropeDest2.setDetails(Rect(292, 81, 316, 94), 2425, -1, -1, -1, 1, NULL);

	_pictographs2._sceneRegionId = 11;
	_pictographs2._resNum = 2425;
	_pictographs2._lookLineNum = 3;
	_pictographs2._talkLineNum = -1;
	_pictographs2._useLineNum = 6;
	g_globals->_sceneItems.push_back(&_pictographs2);

	_crevasse.setDetails(12, 2425, 7, -1, 9);
	_background.setDetails(Rect(0, 0, 320, 200), 2425, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();
	switch (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
	case 2000: {
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(340, 200));

		Common::Point pt(280, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 2425:
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(280, 150));
		signal();
		break;
	case 2455:
		_sceneMode = 2428;
		setAction(&_sequenceManager, this, 2428, &R2_GLOBALS._player, &_rope, NULL);
		break;
	default:
		R2_GLOBALS._player.setPosition(Common::Point(280, 150));
		R2_GLOBALS._player.setStrip(8);
		R2_GLOBALS._player.enableControl();
		break;
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2425;
}

void Scene2425::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2425::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		_sceneMode = 2425;
		setAction(&_sequenceManager, this, 2425, &R2_GLOBALS._player, &_rope, NULL);
		break;
	case 2425:
		g_globals->_sceneManager.changeScene(2455);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2430 - Spill Mountains: Bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2430::Companion::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2430::GunPowder::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (R2_GLOBALS._player._characterIndex != 2))
		return SceneActor::startAction(action, event);

	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2430;
	scene->setAction(&scene->_sequenceManager, scene, 2430, &R2_GLOBALS._player, &scene->_gunPowder, NULL);
	return true;
}

bool Scene2430::OilLamp::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (R2_GLOBALS._player._characterIndex != 2))
		return SceneActor::startAction(action, event);

	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2435;
	scene->setAction(&scene->_sequenceManager, scene, 2435, &R2_GLOBALS._player, &scene->_oilLamp, NULL);
	return true;
}

void Scene2430::SouthExit::changeScene() {
	Scene2430 *scene = (Scene2430 *)R2_GLOBALS._sceneManager._scene;

	scene->_sceneMode = 0;
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(108, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2430::postInit(SceneObjectList *OwnerList) {
	loadScene(2430);
	SceneExt::postInit();
	_southExit.setDetails(Rect(68, 155, 147, 168), EXITCURSOR_S, 2000);
	_southExit.setDest(Common::Point(108, 160));

	if (R2_INVENTORY.getObjectScene(R2_GUNPOWDER) == 2430) {
		_gunPowder.postInit();
		_gunPowder.setup(2435, 1, 5);
		_gunPowder.setPosition(Common::Point(205, 119));
		_gunPowder.fixPriority(152);
		_gunPowder.setDetails(2430, 51, -1, 53, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_3) == 2435) {
		_oilLamp.postInit();
		_oilLamp.setup(2435, 1, 1);
		_oilLamp.setPosition(Common::Point(31, 65));
		_oilLamp.setDetails(2430, 48, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(100, 200));

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(189, 137));
		R2_GLOBALS._walkRegions.disableRegion(4);
	}

	_furnishings.setDetails(Rect(9, 58, 63, 92), 2430, 6, -1, -1, 1, NULL);
	_rug1.setDetails(Rect(20, 89, 127, 107), 2430,  9, -1, 11, 1, NULL);
	_mirror.setDetails(Rect(49, 7, 60, 27), 2430, 12, 13, 14, 1, NULL);
	_garments.setDetails(Rect(69, 10, 95, 72), 2430, 15, -1, 14, 1, NULL);
	_post.setDetails(Rect(198, 4, 222, 146), 2430, 30, 31, 32, 1, NULL);
	_bed.setDetails(Rect(155, 40, 304, 120), 2430, 21, -1, 23, 1, NULL);
	_towel.setDetails(Rect(249, 3, 261, 39), 2430, 24, 25, -1, 1, NULL);
	_bottles1.setDetails(Rect(279, 13, 305, 34), 2430, 33, -1, 18, 1, NULL);
	_bottles2.setDetails(Rect(11, 30, 37, 45), 2430, 33, -1, 18, 1, NULL);
	_clothesPile1.setDetails(Rect(116, 104, 148, 111), 2430, 39, -1, -1, 1, NULL);
	_clothesPile2.setDetails(Rect(66, 77, 84, 83), 2430, 39, -1, -1, 1, NULL);
	_rug2.setDetails(Rect(117, 118, 201, 141), 2430, 9, -1, 11, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2430;
		Common::Point pt(108, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(105, 145));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2430::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2430:
		_gunPowder.remove();
		R2_INVENTORY.setObjectScene(R2_GUNPOWDER, 2);
		R2_GLOBALS._player.enableControl();
		break;
	case 2435:
		_oilLamp.remove();
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_3, 2);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2435 - Spill Mountains: Throne room
 *
 *--------------------------------------------------------------------------*/

bool Scene2435::Companion::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2435::Astor::startAction(CursorType action, Event &event) {
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_SAPPHIRE_BLUE:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS.setFlag(82);
		scene->_stripManager.start(603, scene);
		return true;
	case R2_ANCIENT_SCROLLS:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS.setFlag(82);
		scene->_stripManager.start(602, scene);
		R2_INVENTORY.setObjectScene(R2_ANCIENT_SCROLLS, 2000);
		return true;
	case CURSOR_TALK:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		if ((R2_GLOBALS._player._characterIndex == R2_QUINN) || (R2_GLOBALS.getFlag(82))) {
			scene->_stripManager.start(605, scene);
			return true;
		} else if (R2_INVENTORY.getObjectScene(R2_ANCIENT_SCROLLS) == 2) {
			scene->_stripManager.start(601, scene);
			return true;
		} else {
			R2_GLOBALS.setFlag(82);
			scene->_stripManager.start(600, scene);
			return true;
		}
	default:
		return SceneActor::startAction(action, event);
	}
}

void Scene2435::SouthExit::changeScene() {
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(175, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

}

void Scene2435::postInit(SceneObjectList *OwnerList) {
	loadScene(2435);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(201);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_pharishaSpeaker);
	_southExit.setDetails(Rect(142, 155, 207, 167), EXITCURSOR_S, 2000);
	_southExit.setDest(Common::Point(175, 160));
	_astor.postInit();
	_astor.setup(2005, 3, 1);
	_astor.setPosition(Common::Point(219, 106));
	_astor.setDetails(2001, 25, 26, -1, 1, (SceneItem *)NULL);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(175, 200));
	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(107, 145));
		R2_GLOBALS._walkRegions.disableRegion(2);
	}

	_leftWindow.setDetails(Rect(52, 44, 96, 82), 2430, 3, -1, 5, 1, NULL);
	_rightWindow.setDetails(Rect(117, 36, 161, 74), 2430, 3, -1, 5, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		_sceneMode = 10;
		Common::Point pt(175, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2450) {
		_sceneMode = 30;
		Common::Point pt(175, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2435;
	R2_GLOBALS._spillLocation[1 + R2_GLOBALS._player._characterIndex] = 12;
}

void Scene2435::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2435::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 30:
		R2_GLOBALS._player._characterScene[R2_QUINN] = 2435;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 2435;
		R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 2435;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 2435;
		R2_GLOBALS._spillLocation[R2_QUINN] = 12;
		R2_GLOBALS._spillLocation[R2_SEEKER] = 12;
		R2_GLOBALS.setFlag(81);
		_sceneMode = 2436;
		R2_GLOBALS._player.setStrip(7);
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_companion.setVisage(20);
		else
			_companion.setVisage(2008);
		setAction(&_sequenceManager, this, 2436, &_companion, NULL);
		break;
	case 2436:
		R2_GLOBALS._walkRegions.disableRegion(2);
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		_stripManager.start(709, this);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2440 - Spill Mountains: Another bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2440::Companion::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene2440::OilLamp::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_SEEKER)) {
		Scene2440 *scene = (Scene2440 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2440;
		scene->setAction(&scene->_sequenceManager, scene, 2440, &R2_GLOBALS._player, &scene->_oilLamp, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene2440::SouthEastExit::changeScene() {
	Scene2440 *scene = (Scene2440 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(210, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2440::postInit(SceneObjectList *OwnerList) {
	loadScene(2440);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	// Fix exit cursor, the original was using NW
	_southEastExit.setDetails(Rect(172, 155, 250, 167), EXITCURSOR_SE, 2000);
	_southEastExit.setDest(Common::Point(210, 160));
	if (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_2) == 2440) {
		_oilLamp.postInit();
		_oilLamp.setup(2435, 1, 1);
		_oilLamp.setPosition(Common::Point(94, 80));
		_oilLamp.fixPriority(106);
		_oilLamp.setDetails(2430, 48, -1, -1, 1, (SceneItem *)NULL);
	}
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(210, 200));
	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9002, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(38, 119));
	}

	_garments.setDetails(Rect(125, 25, 142, 73), 2430, 15, -1, 14, 1, NULL);
	_bedspread.setDetails(Rect(124, 78, 237, 120), 2430, 36, -1, 38, 1, NULL);
	_post.setDetails(Rect(250, 3, 265, 133), 2430, 30, 31, 32, 1, NULL);
	_rug.setDetails(Rect(91, 117, 203, 140), 2430, 9, -1, 11, 1, NULL);
	_furnishings.setDetails(Rect(48, 78, 103, 112), 2430, 6, -1, -1, 1, NULL);
	_bottles.setDetails(Rect(48, 31, 73, 52), 2430, 33, -1, 18, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2440;
		Common::Point pt(210, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2440::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2440::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2440:
		_oilLamp.remove();
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_2, 2);
		// fall through
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2445 - Spill Mountains:
 *
 *--------------------------------------------------------------------------*/

void Scene2445::postInit(SceneObjectList *OwnerList) {
	loadScene(2445);
	SceneExt::postInit();
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.setVisage(10);
	R2_GLOBALS._player.setPosition(Common::Point(160, 140));
	R2_GLOBALS._player.disableControl();
}

void Scene2445::signal() {
	R2_GLOBALS._player.enableControl();
}

/*--------------------------------------------------------------------------
 * Scene 2450 - Spill Mountains: Another bedroom
 *
 *--------------------------------------------------------------------------*/

bool Scene2450::Parker::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS._player._characterIndex == R2_QUINN)) {
		Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2452;
		scene->setAction(&scene->_sequenceManager, scene, 2452, &R2_GLOBALS._player, &scene->_parker, NULL);
		return true;
	}
	return SceneActor::startAction(action, event);
}

bool Scene2450::CareTaker::startAction(CursorType action, Event &event) {
	if (action == CURSOR_TALK) {
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._stripModifier < 3) {
			Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

			++R2_GLOBALS._stripModifier;
			scene->_sceneMode = 20;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_stripManager.start(699 + (R2_GLOBALS._stripModifier * 2), scene);
			else
				scene->_stripManager.start(700 + (R2_GLOBALS._stripModifier * 2), scene);
		}
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene2450::SouthWestExit::changeScene() {
	if ((R2_GLOBALS._player._characterIndex == R2_SEEKER) || (R2_GLOBALS.getFlag(61))) {
		Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;

		_enabled = false;
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		Common::Point pt(-10, 180);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, scene);
	} else {
		_moving = false;
		SceneItem::display(2450, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		Common::Point pt(60, 140);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, NULL);
	}
}

void Scene2450::postInit(SceneObjectList *OwnerList) {
	loadScene(2450);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1900;
		R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1900;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1900;
	}
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_caretakerSpeaker);

	if (R2_GLOBALS.getFlag(72)) {
		_southWestExit.setDetails(Rect(0, 143, 47, 168), EXITCURSOR_SW, 2000);
		_southWestExit.setDest(Common::Point(10, 160));
	}

	if (!R2_GLOBALS.getFlag(61)) {
		_parker.postInit();
		_parker.setVisage(2009);
		_parker.setPosition(Common::Point(190, 119));
		_parker.fixPriority(50);
		_parker.setDetails(2450, 0, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	switch (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) {
	case 1900:
		R2_GLOBALS._stripModifier = 0;
		R2_GLOBALS._player._characterScene[R2_QUINN] = 2450;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 2450;
		R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 2450;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 2450;
		R2_GLOBALS._player.setup(2450, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(126, 101));
		setAction(&_sequenceManager, this, 2450, &R2_GLOBALS._player, NULL);
		break;
	case 2000:
		_sceneMode = 2451;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS._player._characterScene[R2_SEEKER] == 2450) {
				_companion.postInit();
				_companion.setup(20, 6, 1);
				_companion.setPosition(Common::Point(240, 120));
				_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
			}
			setAction(&_sequenceManager, this, 2451, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 2450;
			R2_GLOBALS._player._characterScene[R2_SEEKER] = 2450;
			if (R2_GLOBALS._player._characterScene[R2_QUINN] == 2450) {
				_companion.postInit();
				if (R2_GLOBALS.getFlag(61))
					_companion.setup(2008, 6, 1);
				else
					_companion.setup(10, 6, 1);
				_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
				_companion.setPosition(Common::Point(106, 111));
			}
			setAction(&_sequenceManager, this, 2456, &R2_GLOBALS._player, NULL);
		}
		break;
	case 2450:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.postInit();
			if (R2_GLOBALS.getFlag(61)) {
				R2_GLOBALS._player.setup(2008, 6, 1);
			} else {
				R2_GLOBALS._player.setup(10, 6, 1);
			}
			R2_GLOBALS._player.setPosition(Common::Point(106, 111));
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			if (R2_GLOBALS.getFlag(72)) {
				if (R2_GLOBALS._player._characterScene[R2_SEEKER] == 2450) {
					_companion.postInit();
					_companion.setup(20, 6, 1);
					_companion.setPosition(Common::Point(240, 120));
					_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
				}
			} else {
				_companion.postInit();
				_companion.setup(20, 8, 1);
				_companion.setPosition(Common::Point(93, 158));
				_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);

				_careTaker.postInit();
				_careTaker.setup(2001, 7, 1);
				_careTaker.setPosition(Common::Point(34, 153));
				_careTaker.setDetails(2001, 40, -1, -1, 1, (SceneItem *)NULL);

				_southWestExit._enabled = false;
			}
		} else {
			R2_GLOBALS._player.postInit();
			R2_GLOBALS._player.setup(20, 8, 1);
			R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(93, 158));
			if (R2_GLOBALS.getFlag(72)) {
				if (R2_GLOBALS._player._characterScene[R2_QUINN] == 2450) {
					_companion.postInit();
					if (R2_GLOBALS.getFlag(61)) {
						_companion.setup(2008, 6, 1);
					} else {
						_companion.setup(10, 6, 1);
					}
					_companion.setPosition(Common::Point(106, 111));
					_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
				}
			} else {
				_companion.postInit();
				if (R2_GLOBALS.getFlag(61)) {
					_companion.setup(2008, 6, 1);
				} else {
					_companion.setup(10, 6, 1);
				}
				_companion.setPosition(Common::Point(106, 111));
				_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);

				_careTaker.postInit();
				_careTaker.setup(2001, 7, 1);
				_careTaker.setPosition(Common::Point(34, 153));
				_careTaker.setDetails(2001, 40, -1, -1, 1, (SceneItem *)NULL);

				_southWestExit._enabled = false;
			}
		}
		R2_GLOBALS._player.enableControl();
		if (!R2_GLOBALS.getFlag(72)) {
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	default:
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(61)) {
				R2_GLOBALS._player.setup(2008, 3, 1);
			} else {
				R2_GLOBALS._player.setup(10, 3, 1);
			}
			R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		} else {
			R2_GLOBALS._player.setVisage(20);
			R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
		}
		R2_GLOBALS._player.setPosition(Common::Point(100, 130));
		R2_GLOBALS._player.enableControl();
		break;
	}
	_post.setDetails(Rect(174, 4, 199, 123), 2430, 30, 31, 32, 1, NULL);
	_bedspread.setDetails(Rect(67, 73, 207, 121), 2430, 36, -1, 38, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);
}

void Scene2450::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2450::signal() {
	switch (_sceneMode) {
	case 10:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		if (R2_GLOBALS._stripModifier == 3) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._stripModifier = 4;
			_sceneMode = 2454;
			setAction(&_sequenceManager, this, 2454, &_careTaker, NULL);
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
			if (R2_GLOBALS._stripModifier < 4)
				R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 30:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 2455;
		setAction(&_sequenceManager, this, 2455, &_companion, NULL);
		break;
	case 31:
		R2_GLOBALS.setFlag(61);
		g_globals->_sceneManager.changeScene(2435);
		break;
	case 2451:
		R2_GLOBALS._player.enableControl();
		break;
	case 2452:
		R2_GLOBALS.setFlag(61);
		_parker.remove();
		R2_GLOBALS._player.enableControl();
		if (!R2_GLOBALS.getFlag(72)) {
			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 2453:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(700, this);
		break;
	case 2454:
		_southWestExit._enabled = true;
		R2_GLOBALS.setFlag(72);
		_careTaker.remove();
		if (R2_GLOBALS.getFlag(61)) {
			g_globals->_sceneManager.changeScene(2435);
		} else {
			_sceneMode = 31;
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				setAction(&_sequenceManager, this, 2452, &R2_GLOBALS._player, &_parker, NULL);
			} else {
				setAction(&_sequenceManager, this, 2452, &_companion, &_parker, NULL);
			}
		}
		break;
	case 2455:
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 2450;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 2000;
		R2_GLOBALS._spillLocation[R2_SEEKER] = 3;
		_companion.remove();
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	default:
		_companion.postInit();
		_companion.setDetails(9002, 0, 4, 3, 2, (SceneItem *)NULL);
		_careTaker.postInit();
		_careTaker.setDetails(2001, 40, -1, -1, 2, (SceneItem *)NULL);
		_sceneMode = 2453;
		setAction(&_sequenceManager, this, 2453, &_careTaker, &_companion, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2455 - Spill Mountains: Inside crevasse
 *
 *--------------------------------------------------------------------------*/

bool Scene2455::Lamp::startAction(CursorType action, Event &event) {
	if (action == R2_GLASS_DOME) {
		if ((R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_2) == 2455) || (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_3) == 2455)) {
			Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2458;
			scene->_pool._lookLineNum = 9;
			scene->_lamp.remove();
			scene->_scrithKey.postInit();
			scene->_scrithKey.setDetails(2455, 16, 1, -1, 2, (SceneItem *)NULL);
			scene->setAction(&scene->_sequenceManager, scene, 2458, &R2_GLOBALS._player, &scene->_pool, &scene->_scrithKey, NULL);
			return true;
		}
	}

	return SceneActor::startAction(action, event);
}

bool Scene2455::Pool::startAction(CursorType action, Event &event) {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_ALCOHOL_LAMP_2:
		if (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_3) != 2455) {
			R2_GLOBALS._player.disableControl();
			scene->_lamp.postInit();
			scene->_lamp.setup(2456, 3, 3);
			scene->_lamp.setPosition(Common::Point(162, 165));
			scene->_lamp.setDetails(2455, 15, 1, -1, 2, (SceneItem *)NULL);
			scene->_sceneMode = 11;
			scene->setAction(&scene->_sequenceManager, scene, 2457, &R2_GLOBALS._player, &scene->_pool, NULL);
			return true;
		}
		break;
	case R2_ALCOHOL_LAMP_3:
		if (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_2) != 2455) {
			R2_GLOBALS._player.disableControl();
			scene->_lamp.postInit();
			scene->_lamp.setup(2456, 3, 3);
			scene->_lamp.setPosition(Common::Point(162, 165));
			scene->_lamp.setDetails(2455, 15, 1, -1, 2, (SceneItem *)NULL);
			scene->_sceneMode = 12;
			scene->setAction(&scene->_sequenceManager, scene, 2457, &R2_GLOBALS._player, &scene->_pool, NULL);
			return true;
		}
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene2455::ScrithKey::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2459;
		scene->setAction(&scene->_sequenceManager, scene, 2459, &R2_GLOBALS._player, &scene->_scrithKey, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene2455::NorthExit::changeScene() {
	Scene2455 *scene = (Scene2455 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 2461;
	scene->setAction(&scene->_sequenceManager, scene, 2461, &R2_GLOBALS._player, NULL);
}

void Scene2455::postInit(SceneObjectList *OwnerList) {
	loadScene(2455);
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_INVENTORY.setObjectScene(R2_GLASS_DOME, 2);
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_3, 2);
	}

	R2_GLOBALS._sound1.play(200);
	_northExit.setDetails(Rect(0, 0, 320, 15), EXITCURSOR_N, 2425);

	if (R2_INVENTORY.getObjectScene(R2_GLASS_DOME) == 2455) {
		_scrithKey.postInit();
		_scrithKey.setup(2456, 3, 1);
		_scrithKey.setPosition(Common::Point(176, 165));
		_scrithKey.setDetails(2455, 16, 1, -1, 1, (SceneItem *)NULL);
	} else if ((R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_3) == 2455) ||
			(R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_2) == 2455)) {
		_lamp.postInit();
		_lamp.setup(2456, 3, 3);
		_lamp.setPosition(Common::Point(162, 165));
		_lamp.setDetails(2455, 15, 1, -1, 1, (SceneItem *)NULL);
	}

	_pool.postInit();
	if (R2_INVENTORY.getObjectScene(R2_GLASS_DOME) == 2455) {
		_pool.setup(2456, 3, 2);
		_pool.setDetails(2455, 9, 1, -1, 1, (SceneItem *)NULL);
	} else {
		if ((R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_3) != 2455) && (R2_INVENTORY.getObjectScene(R2_ALCOHOL_LAMP_2) != 2455))
			_pool.setup(2455, 1, 1);
		else
			_pool.setup(2456, 1, 1);
		_pool.setDetails(2455, 3, 1, -1, 1, (SceneItem *)NULL);
	}
	_pool.setPosition(Common::Point(162, 165));
	_pool.fixPriority(20);
	if (R2_INVENTORY.getObjectScene(R2_GLASS_DOME) != 2455)
		_pool.animate(ANIM_MODE_2, NULL);

	R2_GLOBALS._player.postInit();
	_background.setDetails(Rect(0, 0, 320, 200), 2455, 0, 1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2425) {
		_sceneMode = 2460;
		setAction(&_sequenceManager, this, 2460, &R2_GLOBALS._player, NULL);
	} else {
		R2_GLOBALS._player.setup(2455, 2, 9);
		R2_GLOBALS._player.setPosition(Common::Point(118, 165));
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2455;
}

void Scene2455::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2455::signal() {
	switch (_sceneMode) {
	case 10:
	// No break on purpose
	case 2461:
		g_globals->_sceneManager.changeScene(2425);
		break;
	case 11:
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_2, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 12:
		R2_INVENTORY.setObjectScene(R2_ALCOHOL_LAMP_3, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2458:
		R2_INVENTORY.setObjectScene(R2_GLASS_DOME, 2455);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2459:
		_scrithKey.remove();
		R2_INVENTORY.setObjectScene(R2_SCRITH_KEY, 2);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2500 - Spill Mountains: Large Ledge
 *
 *--------------------------------------------------------------------------*/

void Scene2500::WestExit::changeScene() {
	Scene2500 *scene = (Scene2500 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(20, 105);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2500::postInit(SceneObjectList *OwnerList) {
	loadScene(2500);
	SceneExt::postInit();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 3100)
		R2_GLOBALS._uiElements._active = false;

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_webbsterSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 2000;

	_westExit.setDetails(Rect(30, 50, 85, 105), EXITCURSOR_W, 2000);
	_westExit.setDest(Common::Point(84, 104));
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(11);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
	} else {
		R2_GLOBALS._player.setVisage(21);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	}

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(21, 3, 1);
			_companion.setDetails(9002, 1, -1, -1, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 3, 1);
			_companion.changeZoom(50);
			_companion.setDetails(9001, 0, -1, -1, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(141, 94));
	}

	_background.setDetails(Rect(0, 0, 320, 200), 2430, 0, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		_sceneMode = 10;
		R2_GLOBALS._player.setPosition(Common::Point(20, 105));
		Common::Point pt(95, 105);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 3100) {
		_sceneMode = 2500;
		_quinn.postInit();
		_ship.postInit();
		setAction(&_sequenceManager, this, 2500, &R2_GLOBALS._player, &_quinn, &_ship, NULL);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
	R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2500;
}

void Scene2500::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 2501;
		setAction(&_sequenceManager, this, 2501, &R2_GLOBALS._player, &_quinn, &_ship, NULL);
		break;
	case 2500:
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(900, this);
		break;
	case 2501:
		g_globals->_sceneManager.changeScene(1000);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2525 - Furnace room
 *
 *--------------------------------------------------------------------------*/

bool Scene2525::StopCock::startAction(CursorType action, Event &event) {
	if ((action == R2_REBREATHER_TANK) && (!R2_GLOBALS.getFlag(74))) {
		Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2526;
		scene->setAction(&scene->_sequenceManager, scene, 2526, &R2_GLOBALS._player, NULL);
		return true;
	}

	return SceneItem::startAction(action, event);
}

bool Scene2525::GlassDome::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2525;
		scene->setAction(&scene->_sequenceManager, scene, 2525, &R2_GLOBALS._player, &scene->_glassDome, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
	}
	return true;
}

void Scene2525::SouthExit::changeScene() {
	Scene2525 *scene = (Scene2525 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(R2_GLOBALS._player._position.x, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2525::postInit(SceneObjectList *OwnerList) {
	loadScene(2525);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(200);
	R2_GLOBALS._sound2.play(207);

	_southExit.setDetails(Rect(86, 155, 228, 168), EXITCURSOR_S, 2000);

	if (R2_INVENTORY.getObjectScene(R2_GLASS_DOME) == 2525) {
		_glassDome.postInit();
		_glassDome.setup(2435, 1, 2);
		_glassDome.setPosition(Common::Point(78, 155));
		_glassDome.fixPriority(155);
		_glassDome.setDetails(2525, 27, -1, -1, 1, (SceneItem *)NULL);
	}

	_compressor.postInit();
	_compressor.setup(2525, 1, 1);
	_compressor.setPosition(Common::Point(183, 114));
	_compressor.setDetails(2525, 15, -1, -1, 1, (SceneItem *)NULL);
	_compressor.animate(ANIM_MODE_2, NULL);
	_compressor._numFrames = 3;

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(209, 162));

		R2_GLOBALS._walkRegions.disableRegion(4);
	}

	_stopcock.setDetails(Rect(125, 73, 140, 86), 2525, 6, -1, -1, 1, NULL);
	_pipes1.setDetails(Rect(137, 11, 163, 72), 2525, 12, -1, -1, 1, NULL);
	_pipes2.setDetails(Rect(204, 20, 234, 78), 2525, 12, -1, -1, 1, NULL);
	_machine.setDetails(Rect(102, 62, 230, 134), 2525, 0, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2525, 24, -1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2525;
		R2_GLOBALS._player.setPosition(Common::Point(160, 200));
		Common::Point pt(160, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(160, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2525::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene2525::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2525:
		_glassDome.remove();
		R2_INVENTORY.setObjectScene(R2_GLASS_DOME, 2);
		R2_GLOBALS._player.enableControl();
		break;
	case 2526:
		R2_GLOBALS.setFlag(74);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2530 - Spill Mountains: Well
 *
 *--------------------------------------------------------------------------*/

bool Scene2530::Flask::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2530;
		scene->setAction(&scene->_sequenceManager, scene, 2530, &R2_GLOBALS._player, &scene->_flask, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
	}

	return true;
}

bool Scene2530::Crank::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS.getFlag(73))
			SceneItem::display(2530, 35, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2532;
			scene->setAction(&scene->_sequenceManager, scene, 2532, &R2_GLOBALS._player, &scene->_crank, NULL);
		}
	} else {
		if (R2_GLOBALS.getFlag(73)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2533;
			scene->setAction(&scene->_sequenceManager, scene, 2533, &R2_GLOBALS._player, &scene->_crank, NULL);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 2531;
			scene->setAction(&scene->_sequenceManager, scene, 2531, &R2_GLOBALS._player, &scene->_crank, NULL);
		}
	}

	return true;
}

void Scene2530::SouthExit::changeScene() {
	Scene2530 *scene = (Scene2530 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(108, 200);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2530::postInit(SceneObjectList *OwnerList) {
	loadScene(2530);
	SceneExt::postInit();

	_southExit.setDetails(Rect(68, 155, 147, 168), EXITCURSOR_S, 2000);
	_southExit.setDest(Common::Point(108, 160));

	if (R2_INVENTORY.getObjectScene(R2_PURE_GRAIN_ALCOHOL) == 2530) {
		_flask.postInit();
		_flask.setup(2435, 1, 3);
		_flask.setPosition(Common::Point(299, 80));
		_flask.fixPriority(80);
		_flask.setDetails(2530, 28, -1, -1, 1, (SceneItem *)NULL);
	}

	_crank.postInit();
	if (R2_GLOBALS.getFlag(73)) {
		_crank.setup(2531, 4, 2);
		_crank.setPosition(Common::Point(154, 130));
	} else {
		_crank.setup(2531, 4, 1);
		_crank.setPosition(Common::Point(173, 131));
	}
	_crank.setDetails(2530, 22, -1, -1, 1, (SceneItem *)NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(100, 200));

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(20, 130));
		R2_GLOBALS._walkRegions.disableRegion(1);
	}

	_crank2.setDetails(Rect(108, 90, 135, 205), 2530, 22, -1, -1, 1, NULL);
	_rope.setDetails(Rect(115, 112, 206, 130), 2530, 25, -1, 27, 1, NULL);
	_shelf.setDetails(Rect(256, 64, 311, 85), 2530, 31, -1, 33, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2530, 0, 1, -1, 1, NULL);

	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2530;
		Common::Point pt(108, 150);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(105, 145));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2530::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2530:
		R2_INVENTORY.setObjectScene(R2_PURE_GRAIN_ALCOHOL, 2);
		_flask.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 2531:
	// No break on purpose
	case 2532:
		R2_GLOBALS.setFlag(73);
		R2_GLOBALS._player.enableControl();
		break;
	case 2533:
		R2_GLOBALS.clearFlag(73);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2535 - Spill Mountains: Tannery
 *
 *--------------------------------------------------------------------------*/

bool Scene2535::RebreatherTank::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();

		scene->_sceneMode = (R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 2535) ? 2536 : 2537;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode,
			&R2_GLOBALS._player, &scene->_rebreatherTank, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
	}

	return true;
}

bool Scene2535::TannerMask::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2535;
		scene->setAction(&scene->_sequenceManager, scene, 2535, &R2_GLOBALS._player, &scene->_tannerMask, NULL);
	} else {
		SceneItem::display(2530, 33, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
	}

	return true;
}

void Scene2535::SouthExit::changeScene() {
	Scene2535 *scene = (Scene2535 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_ARROW);
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;
	Common::Point pt(210, 200);
	PlayerMover *mover = new PlayerMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2535::postInit(SceneObjectList *OwnerList) {
	loadScene(2535);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS.setFlag(73);
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 2535);
	}
	_southExit.setDetails(Rect(172, 155, 250, 167), EXITCURSOR_S, 2000);
	_southExit.setDest(Common::Point(210, 160));

	if (R2_INVENTORY.getObjectScene(R2_TANNER_MASK) == 2535) {
		_tannerMask.postInit();
		_tannerMask.setup(2435, 1, 4);
		_tannerMask.setPosition(Common::Point(47, 74));
		_tannerMask.fixPriority(74);
		_tannerMask.setDetails(2535, 21, -1, -1, 1, (SceneItem *)NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 2535) {
		_rebreatherTank.postInit();
		_rebreatherTank.setup(2535, 3, 1);
		_rebreatherTank.setPosition(Common::Point(203, 131));
		_rebreatherTank.setDetails(3, 20, -1, -1, 1, (SceneItem *)NULL);
		R2_GLOBALS._walkRegions.disableRegion(6);
	}

	if ((R2_INVENTORY.getObjectScene(R2_REBREATHER_TANK) == 0) && (R2_GLOBALS.getFlag(73))) {
		_rebreatherTank.postInit();
		_rebreatherTank.setup(2536, 1, 2);
		_rebreatherTank.setPosition(Common::Point(164, 133));
		_rebreatherTank.setDetails(3, 20, -1, -1, 1, (SceneItem *)NULL);
	}

	if (R2_GLOBALS.getFlag(73)) {
		_rope.postInit();
		_rope.setup(2536, 1, 1);
		_rope.setPosition(Common::Point(160, 130));
		_rope.fixPriority(122);
		_rope.setDetails(2535, 37, -1, -1, 1, (SceneItem *)NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(2008);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setVisage(20);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}
	R2_GLOBALS._player.setPosition(Common::Point(210, 200));

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
		_companion.postInit();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_companion.setup(20, 5, 1);
			_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *)NULL);
		} else {
			_companion.setup(2008, 5, 1);
			_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *)NULL);
		}
		_companion.setPosition(Common::Point(245, 115));
		R2_GLOBALS._walkRegions.disableRegion(2);
	}

	_roof.setDetails(Rect(96, 3, 215, 33), 2535, 3, 6, 5, 1, NULL);
	_skin1.setDetails(Rect(4, 43, 40, 101), 2535, 6, 7, 8, 1, NULL);
	_skin2.setDetails(Rect(55, 13, 140, 89), 2535, 6, 7, 8, 1, NULL);
	_skin3.setDetails(Rect(144, 23, 216, 76), 2535, 6, 7, 8, 1, NULL);
	_skin4.setDetails(Rect(227, 8, 307, 99), 2535, 6, 7, 8, 1, NULL);
	_depression.setDetails(Rect(116, 111, 201, 132), 2535, 18, 19, 20, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2535, 0, 1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2535;
		Common::Point pt(210, 150);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(210, 150));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2535::signal() {
	switch (_sceneMode) {
	case 11:
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 2535:
		R2_INVENTORY.setObjectScene(R2_TANNER_MASK, R2_SEEKER);
		_tannerMask.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 2536:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 0);
		R2_GLOBALS._walkRegions.enableRegion(6);
		if (!R2_GLOBALS.getFlag(73)) {
			_rebreatherTank.remove();
			R2_GLOBALS._player.enableControl();
		} else {
			_sceneMode = 20;
			_rebreatherTank.show();
			_rebreatherTank.setup(2536, 1, 2);
			_rebreatherTank.setDetails(3, 20, -1, -1, 3, (SceneItem *)NULL);
			_rebreatherTank.setPosition(Common::Point(164, 150));
			_rebreatherTank.fixPriority(130);

			_rebreatherTank._moveDiff.y = 1;
			ADD_MOVER(_rebreatherTank, 164, 133);
		}
		break;
	case 2537:
		_rebreatherTank.remove();
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		R2_GLOBALS._player.enableControl();
		break;
	case 20:
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2600 - Spill Mountains: Exit
 *
 *--------------------------------------------------------------------------*/

Scene2600::Scene2600(): SceneExt() {
	_rotation = NULL;
}

void Scene2600::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	SYNC_POINTER(_rotation);
}

void Scene2600::postInit(SceneObjectList *OwnerList) {
	loadScene(2600);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	R2_GLOBALS._sound1.fadeSound(214);
	R2_GLOBALS._sound2.play(215);
	_rotation = R2_GLOBALS._scenePalette.addRotation(176, 191, 1);
	_rotation->setDelay(3);
	_rotation->_countdown = 1;
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	_sceneMode = 2600;
	R2_GLOBALS._player.setAction(&_sequenceManager, this, 2600, &R2_GLOBALS._player, NULL);
}

void Scene2600::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.fadeOut2(NULL);
	R2_GLOBALS._uiElements._visible = true;
//	_rotation->remove();
	SceneExt::remove();
}

void Scene2600::signal() {
	if (_sceneMode == 2600)
		g_globals->_sceneManager.changeScene(3800);
}

/*--------------------------------------------------------------------------
 * Scene 2700 - Outer Forest
 *
 *--------------------------------------------------------------------------*/

Scene2700::Scene2700(): SceneExt() {
	_areaMode = _moveMode = _stripNumber = 0;

	_walkRect1.set(70, 122, 90, 132);
	_walkRect2.set(150, 122, 160, 132);
	_walkRect3.set(90, 142, 130, 157);
	_walkRect4.set(175, 137, 200, 147);
	_walkRect5.set(280, 127, 300, 137);
	_walkRect6.set(240, 157, 265, 167);
}

void Scene2700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_areaMode);
	s.syncAsSint16LE(_moveMode);
	s.syncAsSint16LE(_stripNumber);
}

void Scene2700::Action1::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_ghoulHome6.animate(ANIM_MODE_5, NULL);
}

void Scene2700::Action2::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(300 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_ghoulHome7.animate(ANIM_MODE_5, NULL);
}

void Scene2700::Action3::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(450 + R2_GLOBALS._randomSource.getRandomNumber(450));
	scene->_ghoulHome8.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2700::Action4::signal() {
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	setDelay(300 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_ghoulHome9.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2700::SouthExit::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_moveMode = 2703;
		switch (scene->_areaMode) {
		case 0:
		// No break on purpose
		case 6:
			scene->_sceneMode = 2703;
			scene->setAction(&scene->_sequenceManager, scene, 2703, &R2_GLOBALS._player, NULL);
			break;
		case 1:	{
			Common::Point pt(80, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 4:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 5:	{
			Common::Point pt(235, 132);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2700::EastExit::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_moveMode = 2704;
		switch (scene->_areaMode) {
		case 0:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 1:	{
			Common::Point pt(80, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(155, 137);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 4:	{
			Common::Point pt(235, 132);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 5:
			scene->_sceneMode = 2704;
			scene->setAction(&scene->_sequenceManager, scene, 2704, &R2_GLOBALS._player, NULL);
			break;
		case 6:	{
			Common::Point pt(140, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2700::postInit(SceneObjectList *OwnerList) {
	loadScene(2700);
	SceneExt::postInit();
	R2_GLOBALS._sound1.stop();
	R2_GLOBALS._sound2.stop();

	_southExit.setDetails(Rect(135, 160, 185, 168), SHADECURSOR_DOWN);
	_eastExit.setDetails(Rect(300, 90, 320, 135), EXITCURSOR_E);

	_ghoulHome6.postInit();
	_ghoulHome6.setup(2700, 1, 1);
	_ghoulHome6.setPosition(Common::Point(140, 29));
	_ghoulHome6.setAction(&_action1);

	_ghoulHome7.postInit();
	_ghoulHome7.setup(2700, 2, 1);
	_ghoulHome7.setPosition(Common::Point(213, 32));
	_ghoulHome7.setAction(&_action2);

	_ghoulHome8.postInit();
	_ghoulHome8.setup(2700, 3, 1);
	_ghoulHome8.setPosition(Common::Point(17, 39));
	_ghoulHome8.setAction(&_action3);

	_ghoulHome9.postInit();
	_ghoulHome9.setup(2700, 5, 1);
	_ghoulHome9.setPosition(Common::Point(17, 71));
	_ghoulHome9.setAction(&_action4);

	_ghoulHome1.setDetails(Rect(52, 38, 68, 60), 2700, 4, -1, 6, 1, NULL);
	_ghoulHome2.setDetails(Rect(113, 22, 127, 33), 2700, 4, -1, 6, 1, NULL);
	_ghoulHome3.setDetails(Rect(161, 44, 170, 52), 2700, 4, -1, 6, 1, NULL);
	_ghoulHome4.setDetails(Rect(221, 19, 233, 31), 2700, 4, -1, 6, 1, NULL);
	_ghoulHome5.setDetails(Rect(235, 59, 250, 75), 2700, 4, -1, 6, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2700, 4, -1, 6, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_FLUTE) == 0)
		R2_GLOBALS._sound1.changeSound(234);

	if (R2_GLOBALS._sceneManager._previousScene == 2750) {
		_sceneMode = 2702;
		_areaMode = 5;
		setAction(&_sequenceManager, this, 2702, &R2_GLOBALS._player, NULL);
	} else {
		_areaMode = 0;
		if (R2_GLOBALS._sceneManager._previousScene == 3900) {
			_sceneMode = 2701;
			setAction(&_sequenceManager, this, 2701, &R2_GLOBALS._player, NULL);
		} else {
			R2_GLOBALS._player.setStrip(3);
			R2_GLOBALS._player.setPosition(Common::Point(164, 160));
			R2_GLOBALS._player.enableControl();
		}
	}
}

void Scene2700::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_moveMode) {
		case 1:
			switch (_areaMode) {
			case 0:
			case 2:
			case 4:
			case 6:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2705, &R2_GLOBALS._player, NULL);
				break;
			case 3: {
				_sceneMode = _moveMode;
				_areaMode = 1;
				Common::Point pt(80, 127);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 1
				break;
			}
			break;
		case 2:
			switch (_areaMode) {
			case 0:
			case 1:
			case 6:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 3:
			case 4: {
				_sceneMode = _moveMode;
				_areaMode = 2;
				Common::Point pt(155, 127);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 2
				break;
			}
			break;
		case 3:
			switch (_areaMode) {
			case 0:
			case 1:
			case 2:
			case 4:
			case 6: {
				_sceneMode = _moveMode;
				_areaMode = 3;
				Common::Point pt(115, 152);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 5:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default: // includes case 3
				break;
			}
			break;
		case 4:
			switch (_areaMode) {
			case 0:
			case 1:
			case 6:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4:
			case 5:
				_sceneMode = _moveMode;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2704, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 5:
			switch (_areaMode) {
			case 0:
			case 1:
			case 6:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4: {
				_sceneMode = _moveMode;
				_areaMode = 5;
				Common::Point pt(285, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			default: // includes case 5
				break;
			}
			break;
		case 6:
			switch (_areaMode) {
			case 0:
			case 3: {
				_sceneMode = _moveMode;
				_areaMode = 6;
				Common::Point pt(250, 162);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 1:
			case 2:
			case 4:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 5:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2703:
			switch (_areaMode) {
			case 0:
			case 3:
			case 6:
				_sceneMode = _moveMode;
				setAction(&_sequenceManager, this, 2703, &R2_GLOBALS._player, NULL);
				break;
			case 1:
			case 2:
			case 4:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 5:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2704:
			switch (_areaMode) {
			case 0:
			case 1:
			case 6:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2706, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 3:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2709, &R2_GLOBALS._player, NULL);
				break;
			case 4:
			case 5:
				_sceneMode = _moveMode;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2704, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		case 2710:
			switch (_areaMode) {
			case 0:
			case 1:
			case 3:
				_areaMode = 3;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2707, &R2_GLOBALS._player, NULL);
				break;
			case 2:
			case 5: {
				_sceneMode = _moveMode;
				Common::Point pt(164, 160);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				break;
				}
			case 4:
				_areaMode = 4;
				R2_GLOBALS._player.setAction(&_sequenceManager, this, 2708, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 11:
		R2_INVENTORY.setObjectScene(R2_FLUTE, 0);
		R2_GLOBALS._player.disableControl();
		_areaMode = 0;
		_sceneMode = 2700;
		setAction(&_sequenceManager, this, 2700, &_nej, NULL);
		break;
	case 12:
		R2_GLOBALS._sound1.play(234);
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_sceneMode = 2711;
		_stripManager.start(_stripNumber, this);
		break;
	case 13:
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_sceneMode = 2712;
		_stripManager.start(_stripNumber, this);
		break;
	case 14:
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_sceneMode = 2713;
		_stripManager.start(_stripNumber, this);
		break;
	case 15:
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_sceneMode = 11;
		_stripManager.start(_stripNumber, this);
		break;
	case 2700:
		_nej.remove();
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	case 2703:
		g_globals->_sceneManager.changeScene(3900);
		break;
	case 2704:
		g_globals->_sceneManager.changeScene(2750);
		break;
	case 2710:
		// Start of Nej assault
		_stripNumber = 1200;
		_sceneMode = 12;
		_nej.postInit();
		setAction(&_sequenceManager, this, 2710, &R2_GLOBALS._player, &_nej, NULL);
		break;
	case 2711:
		R2_GLOBALS._player.disableControl();
		_stripNumber = 1201;
		_sceneMode = 13;
		setAction(&_sequenceManager, this, 2711, &R2_GLOBALS._player, &_nej, NULL);
		break;
	case 2712:
		R2_GLOBALS._player.disableControl();
		_stripNumber = 1202;
		_sceneMode = 14;
		setAction(&_sequenceManager, this, 2712, &R2_GLOBALS._player, &_nej, NULL);
		break;
	case 2713:
		R2_GLOBALS._player.disableControl();
		_stripNumber = 1203;
		_sceneMode = 15;
		setAction(&_sequenceManager, this, 2713, &R2_GLOBALS._player, &_nej, NULL);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	}
}
void Scene2700::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN)) {
		if (R2_GLOBALS._events.getCursor() == R2_FLUTE) {
			if (R2_GLOBALS._player._bounds.contains(event.mousePos)) {
				_sceneMode = 10;
				_moveMode = 2710;
				R2_GLOBALS._player.disableControl();
				R2_GLOBALS._events.setCursor(CURSOR_WALK);

				switch (_areaMode) {
				case 0: {
					_sceneMode = 2710;
					Common::Point pt(164, 160);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 1: {
					Common::Point pt(80, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 2: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 3: {
					Common::Point pt(140, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 4: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 5: {
					Common::Point pt(235, 132);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 6: {
					Common::Point pt(205, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				default:
					break;
				}
			} else {
				SceneItem::display(2700, 3, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
			}

			event.handled = true;
		} else if (R2_GLOBALS._events.getCursor() == CURSOR_WALK) {
			if (_walkRect1.contains(event.mousePos)) {
				if (!_walkRect1.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 1;
				}
			} else if (_walkRect2.contains(event.mousePos)) {
				if (!_walkRect2.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 2;
				}
			} else if (_walkRect3.contains(event.mousePos)) {
				if (!_walkRect3.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 3;
				}
			} else if (_walkRect4.contains(event.mousePos)) {
				if (!_walkRect4.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 4;
				}
			} else if (_walkRect5.contains(event.mousePos)) {
				if (!_walkRect5.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 5;
				}
			} else if (_walkRect6.contains(event.mousePos)) {
				if (!_walkRect6.contains(R2_GLOBALS._player._position)) {
					event.handled = true;
					_sceneMode = 10;
					_moveMode = 6;
				}
			} else {
				event.handled = true;
				R2_GLOBALS._player.updateAngle(Common::Point(event.mousePos.x, event.mousePos.y));
			}
			if (_sceneMode == 10) {
				R2_GLOBALS._player.disableControl();
				switch (_areaMode) {
				case 0:
					if (_moveMode >= 6) {
						Common::Point pt(205, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(140, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 1: {
					Common::Point pt(80, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 2: {
					Common::Point pt(155, 137);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 3:
					if (_moveMode == 1) {
						Common::Point pt(80, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else if (_moveMode == 6) {
						Common::Point pt(140, 162);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(155, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 4:
					if (_moveMode == 5) {
						Common::Point pt(235, 132);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					} else {
						Common::Point pt(155, 137);
						NpcMover *mover = new NpcMover();
						R2_GLOBALS._player.addMover(mover, &pt, this);
					}
					break;
				case 5: {
					Common::Point pt(235, 132);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				case 6: {
					Common::Point pt(140, 162);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
					break;
					}
				default:
					break;
				}
			}
		}
	}
	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 2750 - Inner Forest
 *
 *--------------------------------------------------------------------------*/

Scene2750::Scene2750(): SceneExt() {
	_areaMode = _moveMode = _stripNumber = 0;
}

void Scene2750::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_areaMode);
	s.syncAsSint16LE(_moveMode);
	s.syncAsSint16LE(_stripNumber);
}

void Scene2750::Action1::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex) {
	case 1:
		setDelay(60 + R2_GLOBALS._randomSource.getRandomNumber(240));
		_actionIndex = 2;
		scene->_bird2.show();
		scene->_bird2.animate(ANIM_MODE_8, 1, NULL);
		break;
	case 2:
		setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(600));
		_actionIndex = 0;
		scene->_bird2.show();
		scene->_bird1.animate(ANIM_MODE_2, NULL);
		break;
	default:
		setDelay(30);
		_actionIndex = 1;
		scene->_bird1.animate(ANIM_MODE_6, NULL);
		scene->_folliage1.animate(ANIM_MODE_8, 1, NULL);
		break;
	}
}

void Scene2750::Action2::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_folliage2.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action3::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (scene->_folliage3._position.x <= 320) {
		setDelay(1800 + R2_GLOBALS._randomSource.getRandomNumber(600));
	} else {
		setDelay(60);
		scene->_folliage3.setPosition(Common::Point(-10, 25));
		Common::Point pt(330, 45);
		NpcMover *mover = new NpcMover();
		scene->_folliage3.addMover(mover, &pt, NULL);
	}
}

void Scene2750::Action4::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_folliage4.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action5::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_folliage5.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action6::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_folliage6.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::Action7::signal() {
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	setDelay(600 + R2_GLOBALS._randomSource.getRandomNumber(300));
	scene->_folliage7.animate(ANIM_MODE_8, 1, NULL);
}

void Scene2750::WestExit::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_moveMode = 2752;
		switch (scene->_areaMode) {
		case 1:	{
			scene->_sceneMode = 2752;
			scene->setAction(&scene->_sequenceManager, scene, 2752, &R2_GLOBALS._player, NULL);
			break;
			}
		case 2:	{
			Common::Point pt(140, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			Common::Point pt(210, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2750::EastExit::process(Event &event) {
	SceneArea::process(event);
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._player._canWalk) && (_bounds.contains(event.mousePos))) {
		Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 10;
		scene->_moveMode = 2753;
		switch (scene->_areaMode) {
		case 1:	{
			Common::Point pt(140, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 2:	{
			Common::Point pt(210, 142);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			break;
			}
		case 3:	{
			scene->_sceneMode = 2753;
			scene->setAction(&scene->_sequenceManager, scene, 2753, &R2_GLOBALS._player, NULL);
			break;
			}
		default:
			break;
		}
	}
}

void Scene2750::postInit(SceneObjectList *OwnerList) {
	loadScene(2750);
	R2_GLOBALS._sound2.stop();
	SceneExt::postInit();
	_westExit.setDetails(Rect(0, 90, 20, 135), EXITCURSOR_W);
	_eastExit.setDetails(Rect(300, 90, 320, 135), EXITCURSOR_E);

	_walkRect1.set(30, 127, 155, 147);
	_walkRect2.set(130, 142, 210, 167);
	_walkRect3.set(-1, 137, 290, 147);

	if (R2_INVENTORY.getObjectScene(R2_FLUTE) == 0) {
		R2_GLOBALS._sound1.changeSound(235);
		_fire.postInit();
		_fire.setup(2751, 1, 1);
		_fire.setPosition(Common::Point(104, 158));
		_fire.animate(ANIM_MODE_2, NULL);
	}

	_bird1.postInit();
	_bird1.setup(2750, 1, 1);
	_bird1.setPosition(Common::Point(188, 34));
	_bird1.animate(ANIM_MODE_2, NULL);
	_bird1._numFrames = 16;

	_folliage1.postInit();
	_folliage1.setup(2700, 4, 1);
	_folliage1.setPosition(Common::Point(188, 37));
	_folliage1.fixPriority(26);

	_bird2.postInit();
	_bird2.setup(2750, 2, 1);
	_bird2.setPosition(Common::Point(188, 34));
	_bird2.hide();

	_bird1.setAction(&_action1);

	_folliage2.postInit();
	_folliage2.setup(2750, 3, 1);
	_folliage2.setPosition(Common::Point(9, 167));
	_folliage2.fixPriority(252);
	_folliage2.setAction(&_action2);

	_folliage3.postInit();
	_folliage3.setup(2750, 4, 1);
	_folliage3.setPosition(Common::Point(-10, 25));
	_folliage3.animate(ANIM_MODE_1, NULL);
	_folliage3.setStrip2(4);
	_folliage3._moveRate = 20;
	_folliage3.setAction(&_action3);

	_folliage4.postInit();
	_folliage4.fixPriority(26);
	_folliage4.setup(2750, 5, 1);
	_folliage4.setPosition(Common::Point(258, 33));
	_folliage4.setAction(&_action4);

	_folliage5.postInit();
	_folliage5.fixPriority(26);
	_folliage5.setup(2750, 6, 1);
	_folliage5.setPosition(Common::Point(61, 38));
	_folliage5.setAction(&_action5);

	_folliage6.postInit();
	_folliage6.fixPriority(26);
	_folliage6.setup(2750, 7, 1);
	_folliage6.setPosition(Common::Point(69, 37));
	_folliage6.setAction(&_action6);

	_folliage7.postInit();
	_folliage7.fixPriority(26);
	_folliage7.setup(2750, 8, 1);
	_folliage7.setPosition(Common::Point(80, 35));
	_folliage7.setAction(&_action7);

	_ghoulHome1.setDetails(Rect(29, 50, 35, 56), 2750, 3, -1, 5, 1, NULL);
	_ghoulHome2.setDetails(Rect(47, 36, 54, 42), 2750, 3, -1, 5, 1, NULL);
	_ghoulHome3.setDetails(Rect(193, 21, 206, 34), 2750, 3, -1, 5, 1, NULL);
	_ghoulHome4.setDetails(Rect(301, 18, 315, 32), 2750, 3, -1, 5, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 2700, 0, -1, 2, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);

	if (R2_INVENTORY.getObjectScene(R2_FLUTE) == 0) {
		_nej.postInit();
		_nej.setup(2752, 5, 1);
		_nej.animate(ANIM_MODE_NONE, NULL);
		_nej.setPosition(Common::Point(101, 148));
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._sceneManager._previousScene == 2700) {
		if (R2_INVENTORY.getObjectScene(R2_FLUTE) == 0) {
			R2_GLOBALS._player.setVisage(2752);
			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			R2_GLOBALS._player.setPosition(Common::Point(81, 165));

			R2_GLOBALS._events.setCursor(CURSOR_ARROW);
			_stripNumber = 1204;
			_sceneMode = 11;
			_stripManager.start(_stripNumber, this);
		} else {
			_sceneMode = 2750;
			_areaMode = 1;
			R2_GLOBALS._player.setAction(&_sequenceManager, this, 2750, &R2_GLOBALS._player, NULL);
		}
	} else if (R2_GLOBALS._sceneManager._previousScene == 2800) {
		_sceneMode = 2751;
		_areaMode = 3;
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2751, &R2_GLOBALS._player, NULL);
	} else {
		_areaMode = 1;
		R2_GLOBALS._player.setPosition(Common::Point(90, 137));
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.enableControl();
	}
}

void Scene2750::signal() {
	switch (_sceneMode) {
	case 10:
		switch (_moveMode) {
		case 1:
			switch (_areaMode) {
			case 2: {
				_sceneMode = _moveMode;
				_areaMode = 1;
				Common::Point pt(90, 137);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				_areaMode = 2;
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2: {
			_sceneMode = _moveMode;
			_areaMode = 2;
			Common::Point pt(170, 162);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
			}
			break;
		case 3:
			switch (_areaMode) {
			case 1: {
				_areaMode = 2;
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2: {
				_sceneMode = _moveMode;
				_areaMode = 3;
				Common::Point pt(270, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2752:
			switch (_areaMode) {
			case 1:
				_sceneMode = _moveMode;
				setAction(&_sequenceManager, this, 2752, &R2_GLOBALS._player, NULL);
				break;
			case 2: {
				_areaMode = 1;
				Common::Point pt(20, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				_areaMode = 2;
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
			break;
		case 2753:
			switch (_areaMode) {
			case 1: {
				_areaMode = 2;
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2: {
				_areaMode = 3;
				Common::Point pt(300, 132);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3:
				_sceneMode = _moveMode;
				setAction(&_sequenceManager, this, 2753, &R2_GLOBALS._player, NULL);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		break;
	case 11:
	// No break on purpose
	case 2753:
		g_globals->_sceneManager.changeScene(2800);
		break;
	case 2752:
		g_globals->_sceneManager.changeScene(2700);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	}
}

void Scene2750::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN)
			&& (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN)) {
		if (_walkRect1.contains(event.mousePos)) {
			if (!_walkRect1.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_moveMode = 1;
			}
		} else if (_walkRect2.contains(event.mousePos)) {
			if (!_walkRect2.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_moveMode = 2;
			}
		} else if (_walkRect3.contains(event.mousePos)) {
			if (!_walkRect3.contains(R2_GLOBALS._player._position)) {
				event.handled = true;
				_sceneMode = 10;
				_moveMode = 3;
			}
		} else {
			event.handled = true;
			R2_GLOBALS._player.updateAngle(Common::Point(event.mousePos.x, event.mousePos.y));
		}

		if (_sceneMode == 10) {
			R2_GLOBALS._player.disableControl();
			switch (_areaMode) {
			case 1: {
				Common::Point pt(140, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 2:
				if (_moveMode == 1) {
					Common::Point pt(140, 142);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
				} else {
					Common::Point pt(210, 142);
					NpcMover *mover = new NpcMover();
					R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			case 3: {
				Common::Point pt(210, 142);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
				}
				break;
			default:
				break;
			}
		}
	}
	Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 2800 - Guard post
 *
 *--------------------------------------------------------------------------*/

Scene2800::Scene2800(): SceneExt() {
	_stripNumber = 0;
}

void Scene2800::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_stripNumber);
}

bool Scene2800::Outpost::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_GLOBALS.getFlag(47))) {
		Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 2805;
		scene->setAction(&scene->_sequenceManager, scene, 2805, &R2_GLOBALS._player, NULL);
		return true;
	} else
		return SceneHotspot::startAction(action, event);
}

bool Scene2800::Guard::startAction(CursorType action, Event &event) {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (action == CURSOR_TALK) {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		R2_GLOBALS.setFlag(47);
		scene->_stripNumber = 1205;
		scene->_sceneMode = 2803;
		scene->_stripManager.start(scene->_stripNumber, scene);
		return true;
	} else if (action == R2_SONIC_STUNNER) {
		R2_GLOBALS._events.setCursor(CURSOR_ARROW);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS.setFlag(47);
		scene->_sceneMode = 10;
		scene->setAction(&scene->_sequenceManager, scene, 2802, &R2_GLOBALS._player, &scene->_nej, &scene->_guard, NULL);
		return true;
	} else
		return SceneActor::startAction(action, event);
}

void Scene2800::Action1::signal() {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._position.x <= 320) {
		setDelay(120);
		Common::Point pt(330, 25);
		NpcMover *mover = new NpcMover();
		scene->_bird.addMover(mover, &pt, NULL);
	} else {
		setDelay(1800 + R2_GLOBALS._randomSource.getRandomNumber(600));
		scene->_bird.setPosition(Common::Point(-10, 45));
	}
}

void Scene2800::Action2::signal() {
	Scene2800 *scene = (Scene2800 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(240);
		R2_GLOBALS._sound1.changeSound(240);
		R2_GLOBALS._sound2.stop();
		break;
	case 1:
		_object2.postInit();
		_object2.setVisage(2800);
		_object2.setStrip(1);
		_object2._numFrames = 8;
		_object2._moveRate = 8;
		_object2.changeZoom(100);
		_object2.setPosition(Common::Point(1, 1));
		_object2.show();
		_object2.animate(ANIM_MODE_5, this);
		break;
	case 2:
		R2_GLOBALS._sound2.play(130);
		_object2.setVisage(2800);
		_object2.setStrip(7);

		_object3.postInit();
		_object3.setVisage(2800);
		_object3.setStrip(3);
		_object3._numFrames = 8;
		_object3._moveRate = 8;
		_object3.changeZoom(100);
		_object3.setPosition(Common::Point(300, 104));
		_object3.show();
		_object3.animate(ANIM_MODE_5, this);
		break;
	case 3:
		R2_GLOBALS._sound1.play(241);
		_object4.postInit();
		_object4.setVisage(2800);
		_object4.setStrip(2);
		_object4._numFrames = 4;
		_object4._moveRate = 4;
		_object4.changeZoom(100);
		_object4.setPosition(Common::Point(300, 104));
		_object4.fixPriority(105);
		_object4.show();
		_object4.animate(ANIM_MODE_5, this);
		break;
	case 4:
		setDelay(18);
		_object4.setStrip(4);
		scene->_guard.setVisage(2800);
		scene->_guard.setStrip(5);
		scene->_guard.setFrame(1);
		scene->_guard._numFrames = 5;
		scene->_guard._moveRate = 5;
		scene->_guard.setPosition(Common::Point(300, 104));
		scene->_guard.fixPriority(110);
		scene->_guard.changeZoom(100);
		scene->_guard.show();
		break;
	case 5:
		scene->_guard.animate(ANIM_MODE_5, this);
		break;
	case 6: {
		scene->_guard.changeZoom(-1);
		scene->_guard.setVisage(3107);
		scene->_guard.animate(ANIM_MODE_1, NULL);
		scene->_guard.setStrip(3);
		scene->_guard.setPosition(Common::Point(297, 140));
		scene->_guard._numFrames = 10;
		scene->_guard._moveRate = 10;
		scene->_guard._moveDiff = Common::Point(3, 2);

		Common::Point pt(297, 160);
		NpcMover *mover = new NpcMover();
		scene->_guard.addMover(mover, &pt, this);
		break;
		}
	case 7: {
		scene->_guard.changeZoom(75);
		scene->_guard.updateAngle(R2_GLOBALS._player._position);

		Common::Point pt(105, 82);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 8: {
		R2_GLOBALS._player._numFrames = 8;
		R2_GLOBALS._player._moveRate = 8;
		R2_GLOBALS._player.animate(ANIM_MODE_2, NULL);
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setStrip(2);
		R2_GLOBALS._player.changeZoom(-1);

		Common::Point pt(79, 100);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 9: {
		R2_GLOBALS._player._numFrames = 10;
		R2_GLOBALS._player._moveRate = 10;
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());

		Common::Point pt(64, 100);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 10: {
		R2_GLOBALS._player.fixPriority(124);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);

		Common::Point pt(160, 124);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 11: {
		R2_GLOBALS._player.fixPriority(-1);

		Common::Point pt(160, 160);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 12: {
		Common::Point pt(270, 160);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 13:
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		scene->_stripNumber = 1207;
		scene->_stripManager.start(scene->_stripNumber, this);
		break;
	case 14: {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.fixPriority(110);

		Common::Point pt(288, 140);
		PlayerMover *mover = new PlayerMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		break;
		}
	case 15:
		setDelay(18);
		scene->_guard.updateAngle(R2_GLOBALS._player._position);
		R2_GLOBALS._player.setVisage(2800);
		R2_GLOBALS._player.setStrip(6);
		R2_GLOBALS._player.setFrame(1);
		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.setPosition(Common::Point(300, 104));
		R2_GLOBALS._player._numFrames = 5;
		R2_GLOBALS._player._moveRate = 5;
		break;
	case 16:
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 17:
		setDelay(6);
		_object4.setStrip(2);
		_object4.setFrame(11);
		R2_GLOBALS._player.hide();
		// fall through
	case 18:
		R2_GLOBALS._sound1.play(241);
		_object4.animate(ANIM_MODE_6, this);
		break;
	case 19:
		_object4.remove();
		_object3.animate(ANIM_MODE_6, this);
		break;
	case 20:
		setDelay(6);
		_object3.remove();
		_object2.setStrip(1);
		_object2.setFrame(19);
		break;
	case 21:
		setDelay(150);
		R2_GLOBALS._sound1.play(269);
		R2_GLOBALS._sound2.stop();
		break;
	case 22:
		scene->_sceneMode = 12;
		_object2.animate(ANIM_MODE_6, scene);
		break;
	default:
		break;
	}
}

void Scene2800::postInit(SceneObjectList *OwnerList) {
	loadScene(2800);
	setZoomPercents(100, 50, 124, 75);
	R2_GLOBALS._sound1.stop();
	R2_GLOBALS._sound2.stop();
	SceneExt::postInit();

	_bird.postInit();
	_bird.setup(2750, 4, 1);
	_bird.setPosition(Common::Point(-10, 25));
	_bird.animate(ANIM_MODE_1, NULL);
	_bird.setStrip2(4);
	_bird._moveRate = 20;
	_bird.setAction(&_action1);

	_lightBar.postInit();
	_lightBar.setup(2802, 1, 1);
	_lightBar.setPosition(Common::Point(116, 80));
	_lightBar.fixPriority(111);
	_lightBar.animate(ANIM_MODE_2, NULL);
	_lightBar._numFrames = 6;

	if (!R2_GLOBALS.getFlag(47)) {
		_guard.postInit();
		_guard.setVisage(3105);
		_guard.setStrip(3);
		_guard.setFrame(1);
		_guard.setZoom(50);
		_guard._moveDiff = Common::Point(2, 1);
		_guard.setPosition(Common::Point(122, 82));
		_guard.animate(ANIM_MODE_NONE, NULL);
		_guard.setDetails(2800, -1, -1, -1, 1, (SceneItem *)NULL);
	}

	_background.setDetails(Rect(0, 0, 320, 200), 2800, -1, -1, -1, 1, NULL);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_nejSpeaker);
	_stripManager.addSpeaker(&_guardSpeaker);

	if (R2_INVENTORY.getObjectScene(R2_FLUTE) == 0) {
		R2_GLOBALS._sound1.fadeSound(237);
		if (R2_GLOBALS.getFlag(47)) {
			_outpost.setDetails(Rect(76, 45, 155, 90), 2800, 3, -1, -1, 2, NULL);
		} else {
			_nej.postInit();
			_nej.setup(2752, 5, 1);
			_nej.animate(ANIM_MODE_NONE, NULL);
			_nej.changeZoom(100);
			_nej._moveDiff = Common::Point(2, 1);
			_nej.setPosition(Common::Point(101, 148));
		}
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(19);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.changeZoom(100);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_INVENTORY.getObjectScene(R2_FLUTE) != 0) {
		_sceneMode = 2800;
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2800, &R2_GLOBALS._player, NULL);
	} else if (R2_GLOBALS.getFlag(47)) {
		R2_GLOBALS._player.setVisage(3110);
		R2_GLOBALS._player.changeZoom(-1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		R2_GLOBALS._player.setPosition(Common::Point(160, 124));
		R2_GLOBALS._player.enableControl();
	} else {
		_sceneMode = 2801;
		R2_GLOBALS._player.setAction(&_sequenceManager, this, 2801, &R2_GLOBALS._player,
			&_nej, &_guard, NULL);
	}
}

void Scene2800::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._sound1.play(238);
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripNumber = 1206;
		_sceneMode = 2804;
		_stripManager.start(_stripNumber, this);
		break;
	case 11:
		_nej.remove();
		_bird.setAction(NULL);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		_outpost.setDetails(Rect(76, 45, 155, 90), 2800, 3, -1, -1, 2, NULL);
		break;
	case 12:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sound2.fadeOut2(NULL);
		g_globals->_sceneManager.changeScene(1000);
		break;
	case 2800:
		g_globals->_sceneManager.changeScene(2750);
		break;
	case 2801:
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 2803:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 10;
		setAction(&_sequenceManager, this, 2803, &R2_GLOBALS._player, &_nej, &_guard, NULL);
		break;
	case 2804:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 11;
		setAction(&_sequenceManager, this, 2804, &R2_GLOBALS._player, &_nej, NULL);
		break;
	case 2805:
		_bird.remove();
		setAction(&_action2);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2900 - Balloon Cutscene
 *
 *--------------------------------------------------------------------------*/

bool Scene2900::Scenery::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		SceneItem::display2(1, 5 + R2_GLOBALS._randomSource.getRandomNumber(5));
		break;
	case CURSOR_LOOK:
		SceneItem::display2(2900, 0);
		break;
	case CURSOR_TALK:
		SceneItem::display2(1, 10 + R2_GLOBALS._randomSource.getRandomNumber(5));
		break;
	default:
		break;
	}

	event.handled = true;
	return true;
}

bool Scene2900::ControlPanel::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		SceneItem::display2(2900, 5);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(2900, 3);
		break;
	case CURSOR_TALK:
		SceneItem::display2(2900, 4);
		break;
	default:
		break;
	}

	event.handled = true;
	return true;
}

bool Scene2900::Altimeter::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE:
		SceneItem::display2(2900, 8);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(2900, 6);
		break;
	case CURSOR_TALK:
		SceneItem::display2(2900, 4);
		break;
	default:
		break;
	}

	event.handled = true;
	return true;
}

bool Scene2900::KnobLeft::startAction(CursorType action, Event &event) {
	Scene2900 *scene = (Scene2900 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_majorMinorFlag || scene->_altitudeChanging ||
				scene->_xAmount != scene->_xComparison) {
			// Let your altitude stablize first
			SceneItem::display2(2900, 17);
		} else if (R2_GLOBALS._balloonAltitude / 48 == 0) {
			// Maximum altitude
			SceneItem::display2(2900, 15);
		} else {
			// Increase altitude
			R2_GLOBALS._sound2.fadeSound(282);
			scene->_altitudeChanging = true;
			scene->_altitudeMajorChange = -1;
			scene->_xComparison = 100 - ((R2_GLOBALS._balloonAltitude / 48) - 1) * 25;
		}
		break;

	case CURSOR_LOOK:
		SceneItem::display2(2900, 9);
		break;

	case CURSOR_TALK:
		SceneItem::display2(2900, 4);
		break;

	default:
		break;
	}

	event.handled = true;
	return true;
}

bool Scene2900::KnobRight::startAction(CursorType action, Event &event) {
	switch (action) {
	case CURSOR_USE: {
		Scene2900 *scene = (Scene2900 *)R2_GLOBALS._sceneManager._scene;

		if (scene->_majorMinorFlag || scene->_altitudeChanging ||
				scene->_xAmount != scene->_xComparison) {
			// Let your altitude stablize first
			SceneItem::display2(2900, 17);
		} else if (R2_GLOBALS._balloonAltitude / 48 >= 3) {
			// Altitude is too low - cannot land here
			SceneItem::display2(2900, 16);
		} else {
			// Decrease altitude
			R2_GLOBALS._sound2.fadeSound(212);
			scene->_altitudeChanging = true;
			scene->_altitudeMajorChange = 1;
			scene->_xComparison = 100 - ((R2_GLOBALS._balloonAltitude / 48) + 1) * 25;
		}
		break;
		}

	case CURSOR_LOOK:
		SceneItem::display2(2900, 12);
		break;

	case CURSOR_TALK:
		SceneItem::display2(2900, 4);
		break;

	default:
		break;
	}

	event.handled = true;
	return true;
}

bool Scene2900::Skip::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE) {
		Scene2900 *scene = (Scene2900 *)R2_GLOBALS._sceneManager._scene;

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 12;
		scene->signal();
	}

	return true;
}

/*------------------------------------------------------------------------*/

void Scene2900::Action1::signal() {
	Scene2900 *scene = (Scene2900 *)R2_GLOBALS._sceneManager._scene;
	setDelay(3);

	if (!scene->_majorMinorFlag && !scene->_altitudeChanging) {
		scene->_fadeCounter = 2;
		scene->_controlsActiveChanging = false;
	} else if (scene->_majorMinorFlag) {
		R2_GLOBALS._sound2.fadeOut2(NULL);
	} else if (scene->_fadeCounter == 0) {
		R2_GLOBALS._sound2.fadeOut2(NULL);
	} else if (!scene->_controlsActiveChanging) {
		scene->_knobLeftContent.hide();
		scene->_knobRightContent.hide();
		scene->_controlsActiveChanging = true;
	} else {
		--scene->_fadeCounter;
		scene->_knobLeftContent.show();
		scene->_knobRightContent.show();
		scene->_controlsActiveChanging = false;
	}
}

/*------------------------------------------------------------------------*/

Scene2900::Map::Map() {
	_mapWidth = _mapHeight = 0;
	_resNum = 0;
	_xV = _yV = 0;
	_bounds = Rect(40, 0, 280, 150);
}

void Scene2900::Map::load(int resNum) {
	byte *data = g_resourceManager->getResource(RES_BITMAP, resNum, 9999);

	_resNum = resNum;
	_xV = _yV = 0;
	_mapWidth = READ_LE_UINT16(data);
	_mapHeight = READ_LE_UINT16(data + 2);

	DEALLOCATE(data);
}

Common::Point Scene2900::Map::setPosition(const Common::Point &pos, bool initialFlag) {
	Common::Point p = pos;
	Rect updateRect;

	if (p.x >= 0) {
		int xRight = p.x + _bounds.width();
		if (xRight > _mapWidth) {
			p.x = _mapWidth - _bounds.width();
		}
	} else {
		p.x = 0;
	}

	if (p.y >= 0) {
		int yBottom = p.y + _bounds.height();
		if (yBottom > _mapHeight) {
			p.y = _mapHeight - _bounds.height();
		}
	} else {
		p.y = 0;
	}

	if ((p.x != 0 || p.y != 0) && !initialFlag) {
		moveArea(updateRect, _xV - p.x, _yV - p.y);
		redraw(&updateRect);
	} else {
		redraw();
	}

	_xV = p.x;
	_yV = p.y;
	return Common::Point(_xV, _yV);
}

void Scene2900::Map::redraw(Rect *updateRect) {
	int xHalfCount = _mapWidth / 160;
	int yHalfCount = _mapHeight / 100;
	int rlbNum = 0;

	Rect blockRect(0, 0, 160, 100);
	Rect screenRect = _bounds;
	screenRect.translate(_xV - _bounds.left, _yV - _bounds.top);

	Rect modifyRect;
	if (updateRect)
		modifyRect = *updateRect;

	for (int xCtr = 0; xCtr < xHalfCount; ++xCtr) {
		for (int yCtr = 0; yCtr < yHalfCount; ++yCtr, ++rlbNum) {
			blockRect.moveTo(160 * xCtr, 100 * yCtr);
			if (blockRect.intersects(screenRect)) {
				// The block of the map is at least partially on-screen, so needs drawing
				blockRect.translate(_bounds.left - _xV, _bounds.top - _yV);
				byte *data = g_resourceManager->getResource(RES_BITMAP, _resNum, rlbNum);

				drawBlock(data, blockRect.left, blockRect.top, _bounds, modifyRect);

				DEALLOCATE(data);
			}
		}
	}
}

void Scene2900::Map::synchronize(Serializer &s) {
	s.syncAsUint16LE(_resNum);
	if (s.isLoading())
		load(_resNum);

	s.syncAsSint16LE(_xV);
	s.syncAsSint16LE(_yV);
	_bounds.synchronize(s);
}

int Scene2900::Map::adjustRect(Common::Rect &r1, const Common::Rect &r2) {
	if (r2.contains(r1))
		return 0;
	if (!r2.intersects(r1))
		return 1;

	if (r1.top >= r2.top && r1.bottom <= r2.bottom) {
		if (r1.left >= r2.left && r1.left < r2.right) {
			r1.left = r2.right - 1;
			return 1;
		}
		if (r1.right > r2.left && r1.right <= r2.right) {
			r1.right = r2.left + 1;
			return 1;
		}
	}

	if (r1.left < r2.left || r1.right > r2.right)
		return -1;

	if (r1.top >= r2.top && r1.top < r2.bottom) {
		r1.top = r2.bottom - 1;
		return 1;
	}
	if (r1.bottom > r2.top && r1.bottom <= r2.bottom) {
		r1.bottom = r2.top + 1;
		return 1;
	}

	return -1;
}

void Scene2900::Map::drawBlock(const byte *data, int xp, int yp,
		const Rect &bounds, const Rect &updateRect) {
	Rect blockRect(xp, yp, xp + 160, yp + 100);
	const byte *src = data;

	if (blockRect.intersects(bounds)) {
		blockRect.clip(bounds);

		if (adjustRect(blockRect, updateRect) != 0) {
			int width = blockRect.width();
			int height = blockRect.height();
			src += (blockRect.top - yp) * 160 + blockRect.left - xp;

			GfxSurface &surface = R2_GLOBALS._sceneManager._scene->_backSurface;
			Graphics::Surface s = surface.lockSurface();

			for (int yCtr = 0; yCtr < height; ++yCtr, src += 160) {
				byte *destP = (byte *)s.getBasePtr(blockRect.left, blockRect.top + yCtr);
				Common::copy(src, src + width, destP);
			}

			surface.unlockSurface();
			R2_GLOBALS.gfxManager().copyFrom(surface, blockRect, blockRect);
		}
	}
}

void Scene2900::Map::moveArea(Rect &r, int xAmt, int yAmt) {
	Rect tempRect = r;
	tempRect.translate(xAmt, yAmt);

	if (tempRect.intersects(r)) {
		int xpSrc, xpDest, width;
		int ypSrc, ypDest, height;
		if (xAmt >= 0) {
			xpSrc = tempRect.left;
			width = tempRect.width() - xAmt;
			xpDest = tempRect.left + xAmt;
		} else {
			xpSrc = tempRect.left - xAmt;
			width = tempRect.width() + xAmt;
			xpDest = tempRect.left;
		}

		if (yAmt > 0) {
			height = tempRect.height() - yAmt;
			ypDest = tempRect.top + yAmt + (height - 1);
			ypSrc = tempRect.top + (height - 1);

			for (int yCtr = 0; yCtr < height; ++yCtr, --ypSrc, --ypDest) {
				moveLine(xpSrc, ypSrc, xpDest, ypDest, width);
			}
		} else {
			ypSrc = tempRect.top - yAmt;
			height = tempRect.height() + yAmt;
			ypDest = tempRect.top;

			for (int yCtr = 0; yCtr < height; ++yCtr, ++ypSrc, ++ypDest) {
				moveLine(xpSrc, ypSrc, xpDest, ypDest, width);
			}
		}
	} else {
		r = Rect(0, 0, 0, 0);
	}
}

void Scene2900::Map::moveLine(int xpSrc, int ypSrc, int xpDest, int ypDest, int width) {
	byte buffer[SCREEN_WIDTH];
	assert(width <= SCREEN_WIDTH);

	GfxSurface &surface = R2_GLOBALS.gfxManager().getSurface();
	Graphics::Surface s = surface.lockSurface();

	byte *srcP = (byte *)s.getBasePtr(xpSrc, ypSrc);
	byte *destP = (byte *)s.getBasePtr(xpDest, ypDest);
	Common::copy(srcP, srcP + width, &buffer[0]);
	Common::copy(&buffer[0], &buffer[width], destP);

	surface.unlockSurface();
}

/*------------------------------------------------------------------------*/

Scene2900::Scene2900(): SceneExt() {
	_controlsActiveChanging = false;
	_altitudeChanging = false;
	_majorMinorFlag = false;
	_balloonLocation = Common::Point(550, 550);
	_altitudeMinorChange = 0;
	_altitudeMajorChange = 0;
	_balloonScreenPos = Common::Point(160, 100);
	_newAltitude = 0;
	_xAmount = 100;
	_xComparison = 100;
	_fadeCounter = 0;
	_paletteReloadNeeded = false;
}

void Scene2900::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_controlsActiveChanging);
	s.syncAsSint16LE(_altitudeChanging);
	s.syncAsSint16LE(_majorMinorFlag);
	s.syncAsSint16LE(_altitudeMinorChange);
	s.syncAsSint16LE(_altitudeMajorChange);
	s.syncAsSint16LE(_balloonLocation.x);
	s.syncAsSint16LE(_balloonLocation.y);
	s.syncAsSint16LE(_balloonScreenPos.x);
	s.syncAsSint16LE(_balloonScreenPos.y);
	s.syncAsSint16LE(_newAltitude);
	s.syncAsSint16LE(_xAmount);
	s.syncAsSint16LE(_xComparison);
	s.syncAsSint16LE(_fadeCounter);
	s.syncAsSint16LE(_paletteReloadNeeded);

	_map.synchronize(s);
}

void Scene2900::postInit(SceneObjectList *OwnerList) {
	R2_GLOBALS._uiElements._active = false;
	// TODO: Determine correct colors
	R2_GLOBALS._gfxColors.foreground = 228;
	R2_GLOBALS._fontColors.background = 12;
	R2_GLOBALS._fontColors.foreground = 22;

	_map.load(2950);

	loadScene(2900);
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_leftEdge.setup(2900, 6, 1, 22, 0, 25);
	_rightEdge.setup(2900, 6, 1, 280, 0, 25);
	_knob.setup(2900, 1, 3, 228, 199, 25);

	_altimeterContent.postInit();
	_altimeterContent.setVisage(2900);
	_altimeterContent.setStrip(2);
	_altimeterContent.setFrame(1);
	_altimeterContent.fixPriority(10);

	_knobLeftContent.postInit();
	_knobLeftContent.setVisage(2900);
	_knobLeftContent.setStrip(1);
	_knobLeftContent.setFrame(1);
	_knobLeftContent.setPosition(Common::Point(209, 199));
	_knobLeftContent.fixPriority(200);

	_knobRightContent.postInit();
	_knobRightContent.setVisage(2900);
	_knobRightContent.setStrip(1);
	_knobRightContent.setFrame(2);
	_knobRightContent.setPosition(Common::Point(247, 199));
	_knobRightContent.fixPriority(200);

	// Set up hotspots
	_scenery.setDetails(Rect(0, 0, 320, 150), 2900, -1, -1, -1, 1, (SceneItem *)NULL);
	_controlPanel.setDetails(Rect(0, 150, 320, 200), 2900, -1, -1, -1, 1, (SceneItem *)NULL);
	_altimeter.setDetails(Rect(42, 164, 129, 190), 2900, -1, -1, -1, 2, (SceneItem *)NULL);
	_knobLeft.setDetails(Rect(165, 160, 228, 200), 2900, -1, -1, -1, 2, (SceneItem *)NULL);
	_knobRight.setDetails(Rect(228, 160, 285, 200), 2900, -1, -1, -1, 2, (SceneItem *)NULL);
	_skip.postInit();
	_skip.setBounds(185, 50, 200, 0);
	R2_GLOBALS._sceneItems.push_front(&_skip);

	// For ScummVM, we're introducing a Skip button, since the scene is a pain
	_skipText._color1 = R2_GLOBALS._scenePalette._colors.foreground;
	_skipText._color2 = R2_GLOBALS._scenePalette._colors.background;
	_skipText.setPosition(Common::Point(0, 185));
	_skipText.setup("Skip");

	setAction(&_action1);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setVisage(2900);
	R2_GLOBALS._player.setStrip2(3);
	R2_GLOBALS._player.setFrame2(1);
	R2_GLOBALS._player.fixPriority(15);
	R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._sceneManager._previousScene == 2350 &&
			R2_GLOBALS._balloonPosition.x == 0 && R2_GLOBALS._balloonPosition.y == 0) {
		R2_GLOBALS._balloonAltitude = 5;
		_map.setPosition(Common::Point(_balloonLocation.x - 120, _balloonLocation.y - 100));
		_sceneMode = 10;

		R2_GLOBALS._player.changeZoom(100);
		R2_GLOBALS._player.setPosition(Common::Point(160, 180));
		ADD_PLAYER_MOVER(160, 100);

		_altimeterContent.setPosition(Common::Point(9, 189));
	} else {
		_balloonLocation.x = R2_GLOBALS._balloonPosition.x + 120;
		_balloonLocation.y = R2_GLOBALS._balloonPosition.y + 100;

		if ((R2_GLOBALS._balloonAltitude % 8) == 0)
			_balloonLocation.x -= 70;
		else if ((R2_GLOBALS._balloonAltitude % 8) == 7)
			_balloonLocation.x += 70;

		if (_balloonLocation.x <= 120)
			_balloonScreenPos.x = _balloonLocation.x + 40;
		else if (_balloonLocation.x >= 680)
			_balloonScreenPos.x = _balloonLocation.x - 520;

		if ((R2_GLOBALS._balloonAltitude / 6) == 5)
			_balloonLocation.y -= 50;
		if (_balloonLocation.y <= 100)
			_balloonScreenPos.y = _balloonLocation.y;

		_xAmount = _xComparison = 100 - (R2_GLOBALS._balloonAltitude / 48) * 25;
		_map.setPosition(Common::Point(_balloonLocation.x - 120, _balloonLocation.y - 100));
		_sceneMode = 11;

		R2_GLOBALS._player.changeZoom(_xAmount);
		R2_GLOBALS._player.setPosition(_balloonScreenPos);
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;

		_altimeterContent.setPosition(Common::Point(109 - _xAmount, 189));
	}

	R2_GLOBALS._sound1.play(211);
}

void Scene2900::remove() {
	// TODO: Figure out correct colors
	R2_GLOBALS._gfxColors.foreground = 59;
	R2_GLOBALS._fontColors.background = 4;
	R2_GLOBALS._fontColors.foreground = 15;

	R2_GLOBALS._scenePalette.loadPalette(0);
	R2_GLOBALS._scenePalette.setEntry(255, 255, 255, 255);

	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._sound2.stop();

	R2_GLOBALS._interfaceY = UI_INTERFACE_Y;
	SceneExt::remove();
}

void Scene2900::signal() {
	switch (_sceneMode) {
	case 10:
		_sceneMode = 11;
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 12:
		R2_GLOBALS._sceneManager.changeScene(2600);
		break;
	default:
		break;
	}
}

void Scene2900::dispatch() {
	if (_sceneMode == 11) {
		_balloonLocation.x += balloonData[R2_GLOBALS._balloonAltitude].x;
		_balloonLocation.y += balloonData[R2_GLOBALS._balloonAltitude].y;
		_altitudeMinorChange = balloonData[R2_GLOBALS._balloonAltitude].v3;

		if (_altitudeMinorChange == 0) {
			_majorMinorFlag = false;
		} else {
			_majorMinorFlag = true;
			_altitudeChanging = false;
			_xComparison = 100 - ((R2_GLOBALS._balloonAltitude / 48) + _altitudeMinorChange) * 25;
		}

		// Zooming/altitude balloon change
		if (_xAmount == _xComparison) {
			_majorMinorFlag = false;
		} else {
			if (!_majorMinorFlag) {
				_xAmount = _xAmount - _altitudeMajorChange;
			} else {
				_xAmount = _xAmount - _altitudeMinorChange;
			}

			if (_altitudeMinorChange == -1 || _altitudeMajorChange == -1) {
				if (_altimeterContent._frame == 1) {
					_altimeterContent.setFrame2(10);
				} else {
					_altimeterContent.setFrame2(_altimeterContent._frame - 1);
				}
			} else if (_altitudeMinorChange == -1 || _altitudeMajorChange == 1) {
				if (_altimeterContent._frame == 10)
					_altimeterContent.setFrame2(1);
				else
					_altimeterContent.setFrame2(_altimeterContent._frame + 1);
			}

			_altimeterContent.setPosition(Common::Point(109 - _xAmount, 189));
			R2_GLOBALS._player.changeZoom(_xAmount);
		}

		if (!_paletteReloadNeeded) {
			R2_GLOBALS._scenePalette.loadPalette(2950);
			R2_GLOBALS._scenePalette.refresh();
		}

		R2_GLOBALS._balloonPosition = _map.setPosition(
			Common::Point(_balloonLocation.x - 120, _balloonLocation.y - 100), !_paletteReloadNeeded);
		_paletteReloadNeeded = true;

		if (_balloonLocation.x <= 120)
			_balloonScreenPos.x = _balloonLocation.x + 40;
		else if (_balloonLocation.x >= 680)
			_balloonScreenPos.x = _balloonLocation.x - 520;

		if (_balloonLocation.y <= 100)
			_balloonScreenPos.y = _balloonLocation.y;

		R2_GLOBALS._player.setPosition(_balloonScreenPos);

		if ((_balloonLocation.x % 100) == 50 && (_balloonLocation.y % 100) == 50 && !_majorMinorFlag) {
			// At an altitude change point, so calculate new altitude
			_newAltitude = R2_GLOBALS._balloonAltitude;
			if (_altitudeChanging) {
				_newAltitude += _altitudeMajorChange * 48;
				_altitudeChanging = false;
			}

			if (balloonData[R2_GLOBALS._balloonAltitude].x > 0) {
				++_newAltitude;
			} else if (balloonData[R2_GLOBALS._balloonAltitude].x < 0) {
				--_newAltitude;
			}

			if (balloonData[R2_GLOBALS._balloonAltitude].y > 0) {
				_newAltitude -= 8;
			} else if (balloonData[R2_GLOBALS._balloonAltitude].y < 0) {
				_newAltitude += 8;
			}

			if (balloonData[R2_GLOBALS._balloonAltitude].v3 > 0) {
				_newAltitude += 48;
			} else if (balloonData[R2_GLOBALS._balloonAltitude].v3 < 0) {
				_newAltitude -= 48;
			}

			assert(_newAltitude < 193);
			R2_GLOBALS._balloonAltitude = _newAltitude;
			if (R2_GLOBALS._balloonAltitude == 189) {
				// Finally reached landing point
				_sceneMode = 12;
				R2_GLOBALS._player.disableControl();

				ADD_MOVER(R2_GLOBALS._player, 160, -10);
			}
		}
	}

	Scene::dispatch();
}

void Scene2900::refreshBackground(int xAmount, int yAmount) {
	SceneExt::refreshBackground(xAmount, yAmount);

	_map.redraw();
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
