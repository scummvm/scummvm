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
 * Scene 2000 - 
 *
 *--------------------------------------------------------------------------*/
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

	scene->_field412 = 1;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with argements?");
	scene->_sceneMode = 10;

	warning("TODO: Check sub_22D26");
	Common::Point pt(-10, 129);
	NpcMover *mover = new NpcMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);

	scene->setAction(&scene->_sequenceManager, scene, 206, &R2_GLOBALS._player, NULL);
}

void Scene2000::Exit2::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_field412 = 1;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with argements?");
	scene->_sceneMode = 11;

	warning("TODO: Check sub_22D26");
	Common::Point pt(330, 129);
	NpcMover *mover = new NpcMover();
	BF_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene2000::Exit3::changeScene() {
	Scene2000 *scene = (Scene2000 *)R2_GLOBALS._sceneManager._scene;

	scene->_field412 = 1;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with argements?");
	scene->_sceneMode = 12;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 2:
		scene->_field414 = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 8;
		break;
	case 11:
		scene->_field414 = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 17;
		break;
	case 15:
		scene->_field414 = 8;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 24;
		break;
	case 20:
		scene->_field414 = 4;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 30;
		break;
	case 26:
		scene->_field414 = 6;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 32;
		break;
	case 29:
		scene->_field414 = 11;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 29;
		break;
	default:
		break;
	}

	switch (scene->_field414) {
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

	scene->_field412 = 1;
	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with argements?");
	scene->_sceneMode = 13;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 8:
		scene->_field414 = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 2;
		break;
	case 17:
		scene->_field414 = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 11;
		break;
	case 24:
		scene->_field414 = 9;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 15;
		break;
	case 30:
		scene->_field414 = 5;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 20;
		break;
	case 32:
		scene->_field414 = 7;
		R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex] = 26;
		break;
	default:
		break;
	}

	switch (scene->_field414) {
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

	scene->_sceneMode = 0;
	R2_GLOBALS._player.disableControl();
	warning("DisableControl, with argements?");
	scene->_sceneMode = 14;

	switch (R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex]) {
	case 3:
		scene->_field414 = 1;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		scene->_field414 = 7;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 10:
		scene->_field414 = 8;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 12:
		scene->_field414 = 3;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 16:
		scene->_field414 = 4;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 21:
		scene->_field414 = 5;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2015, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2035, &R2_GLOBALS._player, NULL);
		break;
	case 25:
		scene->_field414 = 2;
		if (R2_GLOBALS._player._characterIndex == 1)
			scene->setAction(&scene->_sequenceManager, scene, 2017, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 2037, &R2_GLOBALS._player, NULL);
		break;
	case 34:
		scene->_field414 = 6;
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
	_field412 = 0;

	_exit1.setDetails(Rect(0, 100, 14, 140), CURSOR_9, 2000);
	_exit1.setDest(Common::Point(14, 129));
	_exit2.setDetails(Rect(305, 100, 320, 140), CURSOR_10, 2000);
	_exit2.setDest(Common::Point(315, 129));
	_exit3.setDetails(Rect(71, 130, 154, 168), CURSOR_8, 2000);
	_exit3.setDest(Common::Point(94, 129));
	_exit4.setDetails(Rect(138, 83, 211, 125), CURSOR_7, 2000);
	_exit4.setDest(Common::Point(188, 128));
	_exit5.setDetails(Rect(61, 68, 90, 125), CURSOR_9, 2000);
	_exit5.setDest(Common::Point(92, 129));

	R2_GLOBALS._sound1.play(200);
	warning("STUB: sub_D2676");
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

	warning("STUB: sub_D2FBA");

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
			_field414 = 1;
			--R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			warning("STUB: sub_D2676");
			warning("STUB: sub_D2FBA");
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
			_field414 = 2;
			++R2_GLOBALS._v56605[R2_GLOBALS._player._characterIndex];
			warning("STUB: sub_D2676");
			warning("STUB: sub_D2FBA");
			break;
		}
		break;
	case 12:
	case 13:
		warning("STUB: sub_D2676");
		warning("STUB: sub_D2FBA");
		break;
	case 14:
		switch (_field414 - 1) {
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
	if ((R2_GLOBALS._player._canWalk) && (event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_3)) {
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

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
}


} // End of namespace Ringworld2
} // End of namespace TsAGE
