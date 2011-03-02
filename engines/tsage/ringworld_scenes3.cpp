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
 * $URL$
 * $Id$
 *
 */

#include "tsage/ringworld_scenes3.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

/*--------------------------------------------------------------------------
 * Scene 2000 - Cockpit cutscenes
 *
 *--------------------------------------------------------------------------*/

void Scene2000::Action1::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2075, this);
		break;
	case 2:
		setDelay(4);
		break;
	case 3:
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(1250);
		break;
	}
}

void Scene2000::Action2::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object2.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		if (_globals->_randomSource.getRandomNumber(4) >= 2)
			_actionIndex = 0;
		break;
	case 2:
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = _globals->_randomSource.getRandomNumber(1);
		break;
	}
}

void Scene2000::Action3::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object6.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object6.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action4::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object4.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		break;
	case 1:
		scene->_object4.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(179) + 60);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action5::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_object3.animate(ANIM_MODE_5, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(125) + 300);
		break;
	case 1:
		scene->_object3.animate(ANIM_MODE_6, NULL);
		setDelay(_globals->_randomSource.getRandomNumber(125) + 300);
		_actionIndex = 0;
		break;
	}
}

void Scene2000::Action6::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(130);
		break;
	case 1:
		scene->_soundHandler2.startSound(79);
		scene->_stripManager.start(2000, this);
		break;
	case 2:
		_globals->_soundHandler.startSound(81);
		scene->_object6.postInit();
		scene->_object6.setVisage(2003);
		scene->_object6.setAction(NULL);
		scene->_object6.setStrip2(2);
		scene->_object6.setPosition(Common::Point(184, 137));
		scene->_object6.animate(ANIM_MODE_5, this);
		break;
	case 3:
		scene->_stripManager.start(95, this);
		break;
	case 4:
		scene->_object6.animate(ANIM_MODE_6, this);
		break;
	case 5:
		_globals->_soundHandler.startSound(80);
		scene->_object6.remove();
		_globals->_sceneManager.changeScene(1001);
		break;
	}
}

void Scene2000::Action7::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2072, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2222);
		break;
	}
}

void Scene2000::Action8::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2073, this);
		break;
	case 2:
		setDelay(10);
		break;
	case 3:
		_globals->_stripNum = 2005;
		_globals->_sceneManager.changeScene(1000);
		break;
	}
}

void Scene2000::Action9::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2074, this);
		break;
	case 2:
		setDelay(3);
		break;
	case 3:
		_globals->_stripNum = 2008;
		_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action10::signal() {
	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		error("Old stuff");
		break;
	case 2:
		SceneItem::display(2000, 17, SET_Y, 20, SET_X, 110, SET_FONT, 2, SET_BG_COLOUR, -1,
			SET_FG_COLOUR, 17, SET_WIDTH, 200, SET_POS_MODE, 0, SET_KEEP_ONSCREEN, 1, LIST_END);
		break;
	case 3:
		SceneItem::display(0, 0);
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(9999);
		break;
	}
}

void Scene2000::Action11::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		error("Old stuff");
		break;
	case 1:
		scene->_stripManager.start(2076, this);
		break;
	case 2:
		scene->_stripManager.start(2077, this);
		break;
	case 3:
		_globals->_stripNum = 0;
		_globals->_sceneManager.changeScene(1400);
		break;
	}
}

void Scene2000::Action12::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		scene->_stripManager.start(2020, this);
		break;
	case 2:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 3:
		_globals->_sceneManager.changeScene(2300);
		break;
	}
}

void Scene2000::Action13::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2078, this);
		break;
	case 2:
		SceneItem::display(0, 0);
		_globals->_stripNum = 2751;
		_globals->_sceneManager.changeScene(1500);
		break;
	}
}

void Scene2000::Action14::signal() {
	Scene2000 *scene = (Scene2000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(30);
		break;
	case 1:
		scene->_stripManager.start(2070, this);
		break;
	case 2:
		setDelay(60);
		break;
	case 3:
		_globals->_soundHandler.startSound(99);
		scene->_object8.unflag100();
		scene->_object8.animate(ANIM_MODE_5, this);
		break;
	case 4:
		_globals->_soundHandler.startSound(12);
		scene->_object8.setStrip(2);
		scene->_object8.setFrame(1);
		scene->_object9.unflag100();
		scene->_object10.unflag100();
		setDelay(60);
		break;
	case 5:
		scene->_stripManager.start(2001, this, scene);
		break;
	case 6:
		_globals->_soundHandler.proc1(false);
		scene->_object8.setStrip(1);
		scene->_object8.setFrame(scene->_object8.getFrameCount());
		scene->_object8.animate(ANIM_MODE_6, this);

		scene->_object9.remove();
		scene->_object10.remove();
		break;
	case 7:
		_globals->_soundHandler.startSound(111);
		scene->_object8.remove();
		setDelay(5);
		break;
	case 8:
		scene->_stripManager.start(2071, this);
		break;
	case 9:
		_globals->_stripNum = 1250;
		_globals->_sceneManager.changeScene(1000);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene2000::postInit(SceneObjectList *OwnerList) {
	loadScene(2000);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);
	_stripManager.addSpeaker(&_speakerHText);

	_speakerQText._npc = &_object2;
	_speakerSText._npc = &_object3;
	_speakerMText._npc = &_object6;
	_speakerHText._npc = &_object6;
	_stripManager.setCallback(this);
	
	_object3.postInit();
	_object3.setVisage(2002);
	_object3.setPosition(Common::Point(65, 199));
	
	_object4.postInit();
	_object4.setVisage(2002);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(125, 199));
	_object4.setAction(&_action4);

	_object2.postInit();
	_object2.setVisage(2001);
	_object2.setPosition(Common::Point(43, 115));
	_object2.setAction(&_action2);

	_globals->_player.disableControl();

	_object6.postInit();
	_object6.setVisage(2003);
	_object6.setPosition(Common::Point(267, 170));
	_object6.setAction(&_action3);

	_object8.postInit();
	_object8.setVisage(2005);
	_object8.setPosition(Common::Point(169, 133));
	_object8.setPriority(133);
	_object8.flag100();

	_object9.postInit();
	_object9.setVisage(2005);
	_object9.setStrip2(3);
	_object9.setFrame(4);
	_object9.setPosition(Common::Point(136, 86));
	_object9.setPriority2(190);
	_object9.flag100();

	_object10.postInit();
	_object10.setVisage(2005);
	_object10.setStrip2(5);
	_object10.setFrame(4);
	_object10.setPosition(Common::Point(202, 86));
	_object10.setPriority2(195);
	_object10.flag100();

	switch (_globals->_sceneManager._previousScene) {
	case 1000:
		setAction(&_action7);
		break;
	case 1001:
		_object6.remove();
		setAction(&_action12);
		break;
	case 1500:
		setAction(&_action13);
		break;
	case 2200:
		_globals->_soundHandler.startSound(111);
		setAction(&_action14);
		break;
	case 2222:
		_globals->_soundHandler.startSound(115);
		setAction(&_action8);
		break;
	case 3500:
		setAction(&_action11);
		break;
	default:
		_object6.remove();
		_globals->_soundHandler.startSound(80);
		setAction(&_action6);
		break;
	}

	_soundHandler1.startSound(78);
	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

void Scene2000::stripCallback(int v) {
	switch (v) {
	case 0:
		_object9.setStrip(3);
		_object9.animate(ANIM_MODE_7, NULL);
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		break;
	case 1:
		_object10.setStrip(5);
		_object10.animate(ANIM_MODE_7, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		_object9.animate(ANIM_MODE_NONE, NULL);
		break;
	case 3:
		_object10.setStrip(6);
		_object10.setFrame(1);
		_object10.animate(ANIM_MODE_5, NULL);
		_object9.setStrip(4);
		_object9.setFrame(1);
		_object9.animate(ANIM_MODE_5, NULL);
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 2100 - Cockpit
 *
 *--------------------------------------------------------------------------*/

void Scene2100::Action1::signal() {
}

void Scene2100::Action2::signal() {
}

void Scene2100::Action3::signal() {
}

void Scene2100::Action4::signal() {
}

void Scene2100::Action5::signal() {
}

void Scene2100::Action6::signal() {
}

void Scene2100::Action7::signal() {
}

void Scene2100::Action8::signal() {
}

void Scene2100::Action9::signal() {
}

void Scene2100::Action10::signal() {
}

void Scene2100::Action11::signal() {
}

void Scene2100::Action12::signal() {
}

void Scene2100::Action13::signal() {
}

void Scene2100::Action14::signal() {
}

void Scene2100::Action15::signal() {
}

void Scene2100::Action16::signal() {
}

void Scene2100::Action17::signal() {
}

/*--------------------------------------------------------------------------*/
/*
void Scene2100::Object1::doAction(int action) {
	Scene2100 *scene = (Scene2100 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_LOOK:
		SceneItem::display2(2100, 3);
		break;
	case CURSOR_USE:
		if (_globals->getFlag(13))
			SceneItem::display2(2100, 29);
		else {
			_globals->_player.disableControl();
			scene->setAction(&_action4);
		}
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}
*/

/*--------------------------------------------------------------------------*/

Scene2100::Scene2100(): 
		_hotspot1(0, 2100, 2, LIST_END) {
}

void Scene2100::postInit(SceneObjectList *OwnerList) {
	loadScene(2100);
	Scene::postInit();
	setZoomPercents(60, 80, 200, 100);

	_stripManager.addSpeaker(&_speakerMText);
	_stripManager.addSpeaker(&_speakerMR);
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerSAL);
	_stripManager.addSpeaker(&_speakerHText);
	_stripManager.addSpeaker(&_speakerGameText);
	_speakerMText._npc = &_object10;
	_speakerQText._npc = &_globals->_player;
	_speakerSText._npc = &_object9;
	
	_object8.postInit();
	_object8.setVisage(2100);
	_object8.animate(ANIM_MODE_NONE, NULL);
	_object8.setPosition(Common::Point(157, 57));
	_object8.setPriority(5);

	_object2.postInit();
	_object2.setVisage(2101);
	_object2._frame = 1;
	_object2.animate(ANIM_MODE_2, NULL);
	_object2.setPosition(Common::Point(53, 44));
	_object2.changeZoom(100);
	_object2.setPriority2(1);

	_object3.postInit();
	_object3.setVisage(2101);
	_object3._frame = 1;
	_object3._strip = 3;
	_object3.animate(ANIM_MODE_8, 0, NULL);
	_object3.setPosition(Common::Point(274, 52));
	_object3.changeZoom(100);
	_object3.setPriority2(1);

	_object4.postInit();
	_object4.setVisage(2101);
	_object4._frame = 1;
	_object4._strip = 4;
	_object4.animate(ANIM_MODE_8, 0, NULL);
	_object4.setPosition(Common::Point(219, 141));
	_object4.changeZoom(100);
	_object4.setPriority2(160);

	_object5.postInit();
	_object5.setVisage(2101);
	_object5._frame = 1;
	_object5._strip = 5;
	_object5.setPriority2(175);
	_object5.animate(ANIM_MODE_8, 0, NULL);
	_object5.setPosition(Common::Point(97, 142));
	_object5.changeZoom(100);

	_object6.postInit();
	_object6.setVisage(2101);
	_object6._frame = 1;
	_object6._strip = 6;
	_object6.animate(ANIM_MODE_NONE, NULL);
	_object6.setPosition(Common::Point(133, 46));
	_object6.changeZoom(100);
	_object6.setPriority2(1);

	_object7.postInit();
	_object7.setVisage(2101);
	_object7._frame = 1;
	_object7._strip = 7;
	_object7.animate(ANIM_MODE_8, 0, NULL);
	_object7.setPosition(Common::Point(20, 45));
	_object7.changeZoom(100);
	_object7.setPriority2(1);

	_object1.postInit();
	_object1.setVisage(2101);
	_object1._frame = 1;
	_object1._strip = 7;
	_object1.animate(ANIM_MODE_8, 0, NULL);
	_object1.setPosition(Common::Point(88, 41));
	_object1.changeZoom(100);
	_object1.setPriority2(1);

	_hotspot4.setBounds(Rect(139, 74, 173, 96));
	_hotspot3.setBounds(Rect(71, 100, 91, 135));
	_hotspot7.setBounds(Rect(100, 97, 216, 130));
	_hotspot6.setBounds(Rect(13, 124, 94, 168));
	_hotspot5.setBounds(Rect(217, 141, 307, 155));
	_hotspot8.setBounds(Rect(14, 90, 46, 107));
	_hotspot1.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));

	if (!_globals->getFlag(36) && !_globals->getFlag(70) && !_globals->getFlag(43)) {
		_object10.postInit();
		_object10.setPosition(Common::Point(246, 156));
		_object10.animate(ANIM_MODE_NONE, NULL);
		_object10.changeZoom(100);
		_object10.setPriority2(156);
		_object10.setVisage(2107);
		_object10.setStrip(1);
		_object10.setAction(&_action2);
		_globals->_sceneItems.push_back(&_object10);
	}

	if (!_globals->getFlag(59) && !_globals->getFlag(70) && !_globals->getFlag(37) && !_globals->getFlag(114)) {
		_object9.postInit();
		_object9.setPosition(Common::Point(150, 100));
		_object9.animate(ANIM_MODE_NONE, NULL);
		_object9.changeZoom(100);
		_object9.setPriority2(113);
		_object9.setAction(&_action3);
		_globals->_sceneItems.push_back(&_object9);
	}

	_globals->_sceneItems.addItems(&_hotspot8, &_hotspot4, &_hotspot3, &_hotspot2, &_hotspot7, &_hotspot6,
		&_hotspot5, &_object7, &_object8, &_object1, &_object2, &_object3, &_object4, &_object5, &_object6,
		&_hotspot1, NULL);

	// TODO: Load visages

	_globals->_player.postInit();
	_globals->_player.setVisage(_globals->getFlag(13) ? 2170 : 0);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player._moveDiff.x = 4;
	_globals->_player.changeZoom(-1);
	_globals->_player.disableControl();
	_field1800 = 0;

	switch (_globals->_sceneManager._previousScene) {
	case 2120:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_object8.setPriority2(-1);
		_globals->_player.setPriority2(-1);
		_globals->_player.setPosition(Common::Point(80, 66));
		_globals->_player.enableControl();
		break;
	case 2150:
		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));
		_sceneMode = 2104;
		setAction(&_sequenceManager, this, 2104, &_globals->_player, &_object8, NULL);
		break;
	case 2222:
		if (_globals->_sceneObjects->contains(&_object10))
			_object10.remove();

		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(144, 55));
		
		_object9.setVisage(2806);
		_object9.changeZoom(-1);
		_object9.setPosition(Common::Point(158, 55));
		_object9.setPriority2(1);
		_object9.setAction(NULL);
		_object9.setObjectWrapper(new SceneObjectWrapper());
		_object9.animate(ANIM_MODE_1, NULL);
		_object9.setStrip(3);
		setAction(&_action12);
		break;
	case 2320:
		if (_globals->_stripNum == 2321) {
			if (_globals->_sceneObjects->contains(&_object10))
				_object10.remove();
			
			_globals->_player.setPriority2(1);
			_globals->_player.setPosition(Common::Point(144, 55));

			_object9.postInit();
			_object9.setVisage(2806);
			_object9.setStrip(1);
			_object9.changeZoom(-1);
			_object9.setPosition(Common::Point(158, 55));
			_object9.setPriority2(1);
			_object9.setAction(NULL);
			_object9.setObjectWrapper(new SceneObjectWrapper());
			_object9.animate(ANIM_MODE_1, NULL);

			setAction(&_action12);
		} else if (_globals->_stripNum == 6100) {
			_globals->_player.setPosition(Common::Point(157, 56));
			_globals->_player.setPriority2(1);

			_object11.postInit();
			_object11.setVisage(2102);
			_object11.setPosition(Common::Point(160, 199));
			_object11.flag100();

			setAction(&_action14);
		} else {
			_globals->_player.disableControl();
			_globals->_player.setPosition(Common::Point(157, 56));
			_sceneMode = 2104;

			setAction(&_sequenceManager, this, 2104, &_globals->_player, &_object8, NULL);
		}
		break;
	case 3700:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		Scene::setZoomPercents(80, 75, 100, 90);

		if (_globals->_sceneObjects->contains(&_object9))
			_object9.remove();
		
		_globals->_player._angle = 225;
		_globals->_player.setStrip(6);
		_globals->_player.setFrame(1);
		_globals->_player.setPriority2(-1);
		_globals->_player.setPosition(Common::Point(272, 127));

		_object10.setPosition(Common::Point(246, 156));
		_object10.setPriority2(156);
		_sceneMode = 2105;
		setAction(&_sequenceManager, this, 2105, &_object10, NULL);
		break;
	case 4250:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_globals->clearFlag(43);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);

		_object11.postInit();
		_object11.setVisage(2102);
		_object11.setPosition(Common::Point(160, 199));
		_object11.flag100();

		_sceneMode = 2107;
		setAction(&_sequenceManager, this, 2107, &_object11, NULL);
		break;
	case 5000:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_sceneObjects->contains(&_object9))
			_object9.remove();

		_globals->_player.setStrip(3);
		_globals->_player.setFrame(1);
		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));

		_object10.setPosition(Common::Point(246, 156));
		_object10.setPriority2(156);

		setAction(&_action5);
		break;
	case 5100:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);
		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_field1800 = 1;

		_object11.postInit();
		_object11.setVisage(2102);
		_object11.setPosition(Common::Point(160, 199));
		_object11.flag100();

		_globals->_inventory._stasisBox._sceneNumber = 0;
		setAction(&_action9);
		break;
	case 7000:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_inventory._stasisBox2._sceneNumber == 1) {
			_globals->_player.setPriority2(1);
			_globals->_player.setPosition(Common::Point(157, 56));
			
			_object11.postInit();
			_object11.setVisage(2102);
			_object11.setPosition(Common::Point(160, 199));
			_object11.flag100();
			_globals->clearFlag(15);
			_globals->clearFlag(109);
			_globals->clearFlag(72);

			setAction(&_action17);
		} else {
			_globals->_player.setVisage(2104);
			_globals->_player.setFrame(1);
			_globals->_player.setPosition(Common::Point(65, 149));
			_globals->_player.setPriority2(152);
			_globals->_player.setStrip(2);

			_field1800 = 1;
			setAction(&_action16);
		}
		break;
	case 7600:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		if (_globals->_sceneObjects->contains(&_object9))
			_object9.remove();

		_globals->_player.setPriority2(1);
		_globals->_player.setPosition(Common::Point(157, 56));

		setAction(&_action8);
		break;
	case 8100:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_sceneMode = 2106;
		setAction(&_sequenceManager, this, 2106, NULL);
		break;
	case 9750:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player.setVisage(2104);
		_globals->_player.setFrame(1);
		_globals->_player.setPosition(Common::Point(65, 149));
		_globals->_player.setPriority2(152);
		_globals->_player.setStrip(2);
		
		_object11.postInit();
		_object11.setVisage(2102);
		_object11.setPosition(Common::Point(160, 199));
		_object11.flag100();

		_sceneMode = 2103;
		setAction(&_sequenceManager, this, 2103, &_object11, NULL);
		break;
	default:
		_globals->_soundHandler.startSound(160);
		_globals->_soundHandler.proc5(true);

		_globals->_player._uiEnabled = true;
		break;
	}

	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
}

} // End of namespace tSage
