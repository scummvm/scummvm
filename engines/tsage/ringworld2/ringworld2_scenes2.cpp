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
#include "tsage/ringworld2/ringworld2_scenes2.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 2000 - Maze
 *
 *--------------------------------------------------------------------------*/
void Scene2000::initPlayer() {
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");

	warning("initPlayer: %d", _mazePlayerMode);
	switch (_mazePlayerMode) {
	case 0:
		R2_GLOBALS._player.setStrip(5);
		if (_exit1._enabled) {
			if (_exit2._enabled)
				R2_GLOBALS._player.setPosition(Common::Point(140, 129));
			else
				R2_GLOBALS._player.setPosition(Common::Point(20, 129));
		} else
			R2_GLOBALS._player.setPosition(Common::Point(245, 129));
		R2_GLOBALS._player.enableControl();
		warning("EnableControl, with 2 arguments?");
		break;
	case 1:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2001;
		else
			_sceneMode = 2021;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2002;
		else
			_sceneMode = 2022;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2000;
		else
			_sceneMode = 2020;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2005;
		else
			_sceneMode = 2025;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 5:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2004;
		else
			_sceneMode = 2024;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2009;
		else
			_sceneMode = 2029;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2008;
		else
			_sceneMode = 2028;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2013;
		else
			_sceneMode = 2033;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2012;
		else
			_sceneMode = 2032;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2016;
		else
			_sceneMode = 2036;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == 1)
			_sceneMode = 2038;
		else
			_sceneMode = 2040;
		setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
	for (int i = 0; i < 11; i++) {
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == R2_GLOBALS._v56605[3 + i])
			_objList1[i].show();
	}

	if ((R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) && (R2_GLOBALS._v56605[1] == R2_GLOBALS._v56605[2])) {
		_object1.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_object1.setup(20, 5, 1);
			_object1.setDetails(9002, 0, 4, 3, 1, NULL);
		} else {
			_object1.setup(2008, 5, 1);
			_object1.setDetails(9001, 0, 5, 3, 1, NULL);
		}
		if (_exit1._enabled) {
			if (_exit2._enabled)
				_object1.setPosition(Common::Point(180, 128));
			else
				_object1.setPosition(Common::Point(75, 128));
		} else
			_object1.setPosition(Common::Point(300, 128));
	}
}

void Scene2000::initExits() {
	_exit1._enabled = true;
	_exit2._enabled = true;
	_exit3._enabled = false;
	_exit4._enabled = false;
	_exit5._enabled = false;

	_exit1._insideArea = false;
	_exit2._insideArea = false;
	_exit3._insideArea = false;
	_exit4._insideArea = false;
	_exit5._insideArea = false;

	_exit1._moving = false;
	_exit2._moving = false;
	_exit3._moving = false;
	_exit4._moving = false;
	_exit5._moving = false;

	for (int i = 0; i < 11; i++)
		_objList1[i].hide();

	_object1.remove();

	warning("initExits: %d", R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]);

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 3:
	case 10:
	case 16:
	case 21:
		_exit5._enabled = true;
		_exit5._bounds.set(61, 68, 90, 125);
		_exit5.setDest(Common::Point(92, 129));
		_exit5._cursorNum = EXITCURSOR_W;
		break;
	case 4:
	case 12:
	case 25:
	case 34:
		_exit5._enabled = true;
		_exit5._bounds.set(230, 68, 259, 125);
		_exit5.setDest(Common::Point(244, 129));
		_exit5._cursorNum = EXITCURSOR_E;
		break;
	default:
		break;
	}

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] - 1) {
	case 0:
	case 6:
	case 13:
	case 18:
	case 22:
	case 27:
	case 30:
		_exit1._enabled = false;
		loadScene(2225);
		R2_GLOBALS._walkRegions.load(2225);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 1:
	case 19:
		_exit3._enabled = true;
		_exit3._bounds.set(71, 130, 154, 168);
		_exit3.setDest(Common::Point(94, 129));
		_exit3._cursorNum = EXITCURSOR_SE;
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
		R2_GLOBALS._walkRegions.load(2000);
		_exit2._enabled = false;
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1900)
			_mazePlayerMode = 2;
		else if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		R2_GLOBALS._sceneManager._previousScene = 2000;
		break;
	case 7:
	case 29:
		_exit4._enabled = true;
		_exit4._bounds.set(138, 83, 211, 125);
		_exit4.setDest(Common::Point(129, 188));
		_exit4._cursorNum = EXITCURSOR_NW;
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
		_exit3._enabled = true;
		_exit3._bounds.set(78, 130, 148, 168);
		_exit3.setDest(Common::Point(100, 129));
		_exit3._cursorNum = EXITCURSOR_SE;
		loadScene(2075);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 14:
		_exit3._enabled = true;
		_exit3._bounds.set(160, 130, 248, 168);
		_exit3.setDest(Common::Point(225, 129));
		_exit3._cursorNum = EXITCURSOR_SW;
		loadScene(2325);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 16:
	case 31:
		_exit4._enabled = true;
		_exit4._bounds.set(122, 83, 207, 125);
		_exit4.setDest(Common::Point(210, 129));
		_exit4._cursorNum = EXITCURSOR_NW;
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
		_exit4._enabled = true;
		_exit4._bounds.set(108, 83, 128, 184);
		_exit4.setDest(Common::Point(135, 129));
		_exit4._cursorNum = CURSOR_INVALID;
		loadScene(2275);
		R2_GLOBALS._walkRegions.load(2000);
		if (!_exitingFlag)
			_mazePlayerMode = 0;
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 2000;
		break;
	case 28:
		_exit3._enabled = true;
		_exit3._bounds.set(171, 130, 241, 168);
		_exit3.setDest(Common::Point(218, 129));
		_exit3._cursorNum = EXITCURSOR_SW;
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
		warning("TODO: Check sub_22D26");
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, scene);
		break;
		}
	case 1:
		scene->_objList1[_state].setPosition(Common::Point(340, 127));
		--R2_GLOBALS._v56605[4 + _state];
		_actionIndex = 0;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._v56605[4] == 1)
				_actionIndex = 10;
			break;
		case 2:
			if (R2_GLOBALS._v56605[6] == 7)
				_actionIndex = 10;
			break;
		case 4:
			if (R2_GLOBALS._v56605[8] == 14)
				_actionIndex = 10;
			break;
		case 6:
			if (R2_GLOBALS._v56605[10] == 19)
				_actionIndex = 10;
			break;
		case 7:
			if (R2_GLOBALS._v56605[11] == 23)
				_actionIndex = 10;
			break;
		default:
			break;
		}
		
		if (R2_GLOBALS._v56605[3 + _state] == R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex])
			scene->_objList1[_state].show();
		else
			scene->_objList1[_state].hide();
		
		signal();
		break;
	case 5: {
		_actionIndex = 6;
		Common::Point pt(340, 127);
		warning("TODO: Check sub_22D26");
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, this);
		break;
		}
	case 6:
		scene->_objList1[_state].setPosition(Common::Point(-20, 127));
		++R2_GLOBALS._v56605[3 + _state];
		_actionIndex = 5;
		switch (_state - 1) {
		case 0:
			if (R2_GLOBALS._v56605[4] == 5)
				_actionIndex = 15;
			break;
		case 2:
			if (R2_GLOBALS._v56605[6] == 13)
				_actionIndex = 15;
			break;
		case 4:
			if (R2_GLOBALS._v56605[8] == 16)
				_actionIndex = 15;
			break;
		case 6:
			if (R2_GLOBALS._v56605[10] == 22)
				_actionIndex = 15;
			break;
		case 7:
			if (R2_GLOBALS._v56605[11] == 27)
				_actionIndex = 15;
			break;
		default:
			break;
		}

		if (R2_GLOBALS._v56605[3 + _state] == R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex])
			scene->_objList1[_state].show();
		else
			scene->_objList1[_state].hide();

		signal();
		break;
	case 10: {
		Common::Point pt(290, 127);
		warning("TODO: Check sub_22D26");
		NpcMover *mover = new NpcMover();
		scene->_objList1[_state].addMover(mover, &pt, this);
		_actionIndex = 11;
		break;
		}
	case 11:
		if (_state == 1)
			scene->_objList1[0].setStrip(1);
		else if (_state == 5)
			scene->_objList1[4].setStrip(1);
		setDelay(600);
		_actionIndex = 12;
		break;
	case 12:
		if (_state == 1)
			scene->_objList1[0].setStrip(2);
		else if (_state == 5)
			scene->_objList1[4].setStrip(2);
		scene->_objList1[_state].setStrip(1);
		_actionIndex = 5;
		signal();
		break;
	case 15:
		if ((R2_GLOBALS._v56605[3 + _state] == 13) || (R2_GLOBALS._v56605[3 + _state] == 22) || (R2_GLOBALS._v56605[3 + _state] == 27)) {
			Common::Point pt(30, 127);
			warning("TODO: Check sub_22D26");
			NpcMover *mover = new NpcMover();
			scene->_objList1[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		} else {
			Common::Point pt(120, 127);
			warning("TODO: Check sub_22D26");
			NpcMover *mover = new NpcMover();
			scene->_objList1[_state].addMover(mover, &pt, this);
			_actionIndex = 16;
		}
		break;
	case 16:
		if (_state == 1)
			scene->_objList1[2].setStrip(2);
		else if (_state == 8)
			scene->_objList1[9].setStrip(2);
		setDelay(600);
		_actionIndex = 17;
		break;
	case 17:
		if (_state == 1)
			scene->_objList1[2].setStrip(1);
		else if (_state == 8)
			scene->_objList1[9].setStrip(1);
		scene->_objList1[_state].setStrip(2);
		_actionIndex = 0;
		break;
	case 99:
		error("99");
		break;
	default:
		break;
	}
}

void Scene2000::Exit1::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;
	warning("exit1");

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");
	scene->_sceneMode = 10;

	warning("TODO: Check sub_22D26");
	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);

	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene2000::Exit2::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;
	warning("exit2");

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");
	scene->_sceneMode = 11;

	warning("TODO: Check sub_22D26");
	Common::Point pt(330, 129);
	NpcMover *mover = new NpcMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2000::Exit3::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;
	warning("exit13");

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");
	scene->_sceneMode = 12;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 2:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 8;
		break;
	case 11:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 17;
		break;
	case 15:
		scene->_mazePlayerMode = 8;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 24;
		break;
	case 20:
		scene->_mazePlayerMode = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 30;
		break;
	case 26:
		scene->_mazePlayerMode = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 32;
		break;
	case 29:
		scene->_mazePlayerMode = 11;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 29;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 4:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2003, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2023, &R2_GLOBALS._player, NULL);
		break;
	case 6:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2007, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2027, &R2_GLOBALS._player, NULL);
		break;
	case 8:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2011, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2031, &R2_GLOBALS._player, NULL);
		break;
	case 11:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->_sceneMode = 2039;
		else
			scene->_sceneMode = 2041;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		break;

	default:
		break;
	}
}
void Scene2000::Exit4::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;
	warning("exit4");

	scene->_exitingFlag = true;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");
	scene->_sceneMode = 13;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 8:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 2;
		break;
	case 17:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 11;
		break;
	case 24:
		scene->_mazePlayerMode = 9;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 15;
		break;
	case 30:
		scene->_mazePlayerMode = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 20;
		break;
	case 32:
		scene->_mazePlayerMode = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 26;
		break;
	default:
		break;
	}

	switch (scene->_mazePlayerMode) {
	case 5:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2006, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2026, &R2_GLOBALS._player, NULL);
		break;
	case 7:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2010, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2030, &R2_GLOBALS._player, NULL);
		break;
	case 9:
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2014, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2034, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

void Scene2000::Exit5::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;
	warning("exit5");

	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with arguments?");
	scene->_sceneMode = 14;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 3:
		scene->_mazePlayerMode = 1;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		scene->_mazePlayerMode = 7;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		scene->_mazePlayerMode = 8;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 12:
		scene->_mazePlayerMode = 3;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 16:
		scene->_mazePlayerMode = 4;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 21:
		scene->_mazePlayerMode = 5;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 25:
		scene->_mazePlayerMode = 2;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 34:
		scene->_mazePlayerMode = 6;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	default:
		break;
	}
}

void Scene2000::postInit(SceneObjectList *OwnerList) {
	SceneExt::postInit();
	loadScene(2000);

	if (R2_GLOBALS._sceneManager._previousScene != -1) {
		R2_GLOBALS._v56605[1] = 21;
		R2_GLOBALS._v56605[2] = 21;
	}
	if ((R2_GLOBALS._player._characterScene[R2_GLOBALS._player._characterIndex] != R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex]) && (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] != 2350)) {
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 0;
	}
	_exitingFlag = false;

	_exit1.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2000);
	_exit1.setDest(Common::Point(14, 129));
	_exit2.setDetails(Rect(305, 100, 320, 140), EXITCURSOR_E, 2000);
	_exit2.setDest(Common::Point(315, 129));
	_exit3.setDetails(Rect(71, 130, 154, 168), EXITCURSOR_S, 2000);
	_exit3.setDest(Common::Point(94, 129));
	_exit4.setDetails(Rect(138, 83, 211, 125), EXITCURSOR_N, 2000);
	_exit4.setDest(Common::Point(188, 128));
	_exit5.setDetails(Rect(61, 68, 90, 125), EXITCURSOR_W, 2000);
	_exit5.setDest(Common::Point(92, 129));

	R2_GLOBALS._sound1.play(200);
	initExits();
	g_globals->_sceneManager._fadeMode = FADEMODE_IMMEDIATE;
	
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
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
		_objList1[i].postInit();

	_objList1[0].setVisage(2000);
	_objList1[0].setStrip(2);
	_objList1[0].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[1].setVisage(2001);
	_objList1[1].setStrip(2);
	_objList1[1].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[2].setVisage(2003);
	_objList1[2].setStrip(1);
	_objList1[2].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[3].setVisage(2007);
	_objList1[3].setStrip(2);
	_objList1[3].setDetails(2001, 12, -1, -1, 1, NULL);

	_objList1[4].setVisage(2004);
	_objList1[4].setStrip(2);
	_objList1[4].setDetails(2001, 19, -1, -1, 1, NULL);

	_objList1[5].setVisage(2003);
	_objList1[5].setStrip(2);
	_objList1[5].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[6].setVisage(2000);
	_objList1[6].setStrip(1);
	_objList1[6].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[7].setVisage(2000);
	_objList1[7].setStrip(2);
	_objList1[7].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[8].setVisage(2000);
	_objList1[8].setStrip(2);
	_objList1[8].setDetails(2001, 0, -1, -1, 1, NULL);

	_objList1[9].setVisage(2006);
	_objList1[9].setStrip(1);
	_objList1[9].setDetails(2001, 6, -1, -1, 1, NULL);

	_objList1[10].setVisage(2007);
	_objList1[10].setStrip(1);
	_objList1[10].setDetails(2001, 12, -1, -1, 1, NULL);

	for (int i = 0; i < 11; i++) {
		_objList1[i].animate(ANIM_MODE_1, NULL);
		_objList1[i]._moveDiff.x = 3;
		_objList1[i]._moveRate = 8;
		_objList1[i].hide();
		switch (i - 1) {
		case 0:
			if (R2_GLOBALS._v56605[3 + i]  == 1)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 5)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 2:
			if (R2_GLOBALS._v56605[3 + i]  == 7)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 13)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 4:
			if (R2_GLOBALS._v56605[3 + i]  == 14)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 16)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 6:
			if (R2_GLOBALS._v56605[3 + i]  == 19)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 22)
				--R2_GLOBALS._v56605[3 + i];
			break;
		case 8:
			if (R2_GLOBALS._v56605[3 + i]  == 23)
				++R2_GLOBALS._v56605[3 + i];
			else if (R2_GLOBALS._v56605[3 + i]  == 27)
				--R2_GLOBALS._v56605[3 + i];
			break;
		default:
			break;
		}
		switch (R2_GLOBALS._v56605[3 + i] - 1) {
		case 0:
		case 6:
		case 13:
		case 18:
		case 22:
		case 27:
		case 30:
			_objList1[i].setPosition(Common::Point(265, 127));
			break;
		case 5:
		case 12:
		case 17:
		case 21:
		case 26:
			_objList1[i].setPosition(Common::Point(55, 127));
			break;
		default:
			_objList1[i].setPosition(Common::Point(160, 127));
			break;
		}
	}
	_objList1[1].setAction(&_action2);
	_objList1[3].setAction(&_action5);
	_objList1[5].setAction(&_action4);
	_objList1[8].setAction(&_action1);

	initPlayer();

	_item1.setDetails(Rect(0, 0, 320, 200), 2000, 0, -1, 23, 1, NULL);
}

void Scene2000::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene2000::signal() {
	switch (_sceneMode) {
	case 10:
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == 6)
			g_globals->_sceneManager.changeScene(1900);
		else {
			_mazePlayerMode = 1;
			--R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
		}
	break;
	case 11:
		switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
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
			++R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			initExits();
			initPlayer();
			break;
		}
		break;
	case 12:
	case 13:
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
			break;
		}
		break;
	case 2039:
	case 2041:
		g_globals->_sceneManager.changeScene(2350);
		break;
	default:
		break;
	}
}

void Scene2000::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN) && 
			(R2_GLOBALS._events.getCursor() == CURSOR_CROSSHAIRS)) {
		warning("TODO: Check sub_22D26");
		
		Common::Point pt(event.mousePos.x, 129);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt, this);

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
bool Scene2350::Actor2::startAction(CursorType action, Event &event) {
	if (action != R2_6)
		return(SceneActor::startAction(action, event));
	return true;
}

bool Scene2350::Actor3::startAction(CursorType action, Event &event) {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	if ((action == R2_20) && (R2_GLOBALS.getFlag(74))) {
		R2_GLOBALS._player.disableControl();
		scene->_actor1.postInit();
		scene->_sceneMode = 2355;
		scene->setAction(&scene->_sequenceManager, scene, 2355, &R2_GLOBALS._player, &scene->_actor1, NULL);
		return true;
	}

	return(SceneActor::startAction(action, event));
}

void Scene2350::ExitUp::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;
	
	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 12;
	if (R2_GLOBALS._player._characterIndex == 1)
		scene->setAction(&scene->_sequenceManager, scene, 2350, &R2_GLOBALS._player, NULL);
	else
		scene->setAction(&scene->_sequenceManager, scene, 2352, &R2_GLOBALS._player, NULL);
}
	
void Scene2350::ExitWest::changeScene() {
	Scene2350 *scene = (Scene2350 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 11;

	Common::Point pt(-10, 129);
	warning("TODO: Check sub_22D26");
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
		R2_GLOBALS._player._characterScene[2] = 2350;

	_exitUp.setDetails(Rect(25, 83, 93, 125), EXITCURSOR_NW, 2350);
	_exitUp.setDest(Common::Point(80, 129));
	_exitWest.setDetails(Rect(0, 100, 14, 140), EXITCURSOR_W, 2350);
	_exitWest.setDest(Common::Point(14, 129));

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

	if (R2_GLOBALS._player._characterIndex == 1) {
		R2_GLOBALS._player.setup(2008, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	} else {
		R2_GLOBALS._player.setup(20, 3, 1);
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	}

	if (R2_GLOBALS._player._characterScene[1] == R2_GLOBALS._player._characterScene[2]) {
		_actor2.postInit();
		if (R2_GLOBALS._player._characterIndex == 1) {
			_actor2.setup(20, 5, 1);
			_actor2.setDetails(9002, 0, 4, 3, 1, NULL);
		} else {
			_actor2.setup(2008, 5, 1);
			_actor2.setDetails(9001, 0, 5, 3, 1,  NULL);
		}
		_actor2.setPosition(Common::Point(135, 128));
	}
	_actor3.postInit();
	_actor4.postInit();

	if (R2_INVENTORY.getObjectScene(20) == 2350) {
		_actor3.hide();
		_actor4.hide();
	} else {
		_actor3.setup(2350, 0, 1);
		_actor3.setPosition(Common::Point(197, 101));
		_actor3.setDetails(2000, 12, -1, -1, 1, NULL);
		_actor3.fixPriority(10);
		_actor4.setup(2350, 1, 2);
		_actor4.setPosition(Common::Point(199, 129));
		_actor4.setDetails(2000, 12, -1, -1, 1, NULL);
		_actor4.fixPriority(10);
	}
	_item1.setDetails(Rect(0, 0, 320, 200), 2000, 9, -1, -1, 1, NULL);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
		if (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] == 34) {
			if (R2_GLOBALS._player._characterIndex == 1)
				_sceneMode = 2351;
			else 
				_sceneMode = 2353;
			setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		} else {
			_sceneMode = 10;
			R2_GLOBALS._player.setPosition(Common::Point(-20, 129));
			Common::Point pt(20, 129);
			warning("TODO: Check sub_22D26");
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
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 34;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 12:
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 29;
		g_globals->_sceneManager.changeScene(2000);
		break;
	case 20:
		_sceneMode = 21;
		_stripManager.start(712, this);
		break;
	case 21:
		R2_GLOBALS._player.disableControl();
		R2_INVENTORY.setObjectScene(36, 1);
		_sceneMode = 2354;
		setAction(&_sequenceManager, this, 2354, &R2_GLOBALS._player, NULL);
		break;
	case 2354:
		R2_INVENTORY.setObjectScene(20, 2350);
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

void Scene2350::process(Event &event) {
	if ((R2_GLOBALS._player._canWalk) && (event.eventType != EVENT_BUTTON_DOWN) && 
			(R2_GLOBALS._events.getCursor() == CURSOR_CROSSHAIRS)){
		Common::Point pt(event.mousePos.x, 129);
		PlayerMover *mover = new PlayerMover();
		BF_GLOBALS._player.addMover(mover, &pt);
		event.handled = true;
	}
	Scene::process(event);
}

} // End of namespace Ringworld2
} // End of namespace TsAGE
