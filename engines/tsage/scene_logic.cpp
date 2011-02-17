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
 * $URL: https://scummvm-misc.svn.sourceforge.net/svnroot/scummvm-misc/trunk/engines/tsage/scene_logic.cpp $
 * $Id: scene_logic.cpp 232 2011-02-12 11:56:38Z dreammaster $
 *
 */

#include "tsage/scene_logic.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"

namespace tSage {

Scene *SceneFactory::createScene(int sceneNumber) {
	switch (sceneNumber) {
	// Kziniti Palace (Introduction)
	case 10: return new Scene10();
	// Outer Space (Introduction)
	case 15: return new Scene15();
	// Cut-scenes for Ch'mee house in distance
	case 20: return new Scene20();
	// Outside Ch'mee residence
	case 30: return new Scene30();
	// Chmeee Home
	case 40: return new Scene40();
	// By Flycycles
	case 50: return new Scene50();
	// Title screen
	case 1000: return new Scene1000();

	default:
		error("Unknown scene number - %d", sceneNumber);
		break;
	}
}

/*--------------------------------------------------------------------------*/

//	Common::Array<int> _actions;

DisplayHotspot::DisplayHotspot(int regionId, ...) {
	_sceneRegionId = regionId;

	// Load up the actions
	va_list va;
	va_start(va, regionId);

	int param = va_arg(va, int);
	while (param != LIST_END) {
		_actions.push_back(param);
		param = va_arg(va, int);
	}

	va_end(va);
}

bool DisplayHotspot::performAction(int action) {
	for (uint i = 0; i < _actions.size(); i += 3) {
		if (_actions[i] == action) {
			display(_actions[i + 1], _actions[i + 2], SET_WIDTH, 200, SET_EXT_BGCOLOUR, 7, LIST_END);
			return true;
		}
	}

	return false;
}

/*--------------------------------------------------------------------------
 * Scene 10 - Kziniti Palace (Introduction)
 *
 *--------------------------------------------------------------------------*/

void Scene10::Scene10_Action1::signal() {
	Scene10 *parent = (Scene10 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(6);
		break;
	case 1:
		_globals->_scenePalette.addRotation(240, 254, -1);
		parent->_stripManager.start(10, this);
		break;
	case 2:
		parent->_speakerSText.setTextPos(Common::Point(20, 20));
		parent->_speakerSText._colour1 = 10;
		parent->_speakerSText._textWidth = 160;
		parent->_stripManager.start(11, this, parent);
		break;
	case 3:
		parent->_object2.flag100();
		parent->_object3.flag100();
		parent->_object3.setAction(NULL);
		parent->_object4.animate(ANIM_MODE_5, this);
		break;
	case 4:
	case 9:
		parent->_object1.animate(ANIM_MODE_5, this);
		break;
	case 5:
		parent->_object2.setStrip(3);
		parent->_object2.setFrame(1);
		parent->_object2.setPosition(Common::Point(240, 51));
		parent->_object2.unflag100();
		
		parent->_object3.setStrip(6);
		parent->_object3.setFrame(1);
		parent->_object3.setPosition(Common::Point(200, 76));
		parent->_object3._numFrames = 20;
		parent->_object3.unflag100();

		parent->_stripManager.start(12, this, parent);
		break;
	case 6:
		parent->_object2.flag100();
		parent->_object3.flag100();
		parent->_object1.animate(ANIM_MODE_6, this);
		break;
	case 7:
		parent->_object3.unflag100();
		parent->_object3.setStrip2(5);
		parent->_object3._numFrames = 10;
		parent->_object3.setPosition(Common::Point(180, 87));
		parent->_object3.setAction(&parent->_action2);

		parent->_object2.setStrip(4);
		parent->_object2.setFrame(1);
		parent->_object2.setPosition(Common::Point(204, 59));
		parent->_object2.unflag100();

		parent->_stripManager.start(13, this, parent);
		break;
	case 8:
		parent->_object2.flag100();
		parent->_object3.flag100();
		parent->_object4.animate(ANIM_MODE_6, this);
		break;
	case 10:
		_globals->_soundHandler.proc1(this);
		break;
	case 11:
		_globals->_scenePalette.clearListeners();
		_globals->_sceneManager.changeScene(15);
		break;
	}
}

void Scene10::Scene10_Action2::signal() {
	Scene10 *parent = (Scene10 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(179));
		break;
	case 1:
		parent->_object3.setFrame(1);
		parent->_object3.animate(ANIM_MODE_5, this);
		_actionIndex = 0;
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene10::postInit(SceneObjectList *OwnerList) {
	loadScene(10);
	setZoomPercents(0, 100, 200, 100);
	
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_speakerSText._speakerName = "STEXT";
	_speakerQText._speakerName = "QTEXT";
	_speakerSText._hideObjects = false;
	_speakerQText._hideObjects = false;
	_speakerQText.setTextPos(Common::Point(140, 120));
	_speakerQText._colour1 = 4;
	_speakerQText._textWidth = 160;
	_speakerSText.setTextPos(Common::Point(20, 20));
	_speakerSText._colour1 = 7;
	_speakerSText._textWidth = 320;

	_stripManager.setCallback(this);

	_object1.postInit();
	_object1.setVisage(10);
	_object1.setPosition(Common::Point(232, 90));
	_object1.setPriority2(1);

	_object2.postInit();
	_object2.setVisage(10);
	_object2.setStrip(4);
	_object2.setFrame(1);
	_object2.setPosition(Common::Point(204, 59));
	_object2.setPriority2(198);

	_object3.postInit();
	_object3.setVisage(10);
	_object3.setStrip2(5);
	_object3.setPosition(Common::Point(180, 87));
	_object3.setPriority2(196);
	_object3.setAction(&_action2);

	_object4.postInit();
	_object4.setVisage(10);
	_object4.setStrip(2);
	_object4.setPosition(Common::Point(0, 209));
	_object4.animate(ANIM_MODE_1, NULL);

	_object5.postInit();
	_object5.setVisage(11);
	_object5.setPosition(Common::Point(107, 146));
	_object5.animate(ANIM_MODE_2, NULL);
	_object5._numFrames = 5;

	_object6.postInit();
	_object6.setVisage(11);
	_object6.setStrip(2);
	_object6.setPosition(Common::Point(287, 149));
	_object6.animate(ANIM_MODE_2, NULL);
	_object6._numFrames = 5;

	_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
	_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

	setAction(&_action1);
	_globals->_soundHandler.startSound(5);
}

void Scene10::stripCallback(int v) {
	switch (v) {
	case 1:
		_object2.animate(ANIM_MODE_7, -1, NULL);
		break;
	case 2:
		_object2.animate(ANIM_MODE_NONE);
		break;
	case 3:
		_object2.animate(ANIM_MODE_7, -1, NULL);
		_object3.animate(ANIM_MODE_5, NULL);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 15 - Outer Space (Introduction)
 *
 *--------------------------------------------------------------------------*/

void Scene15::Scene15_Action1::signal() {
	Scene15 *parent = (Scene15 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1:
		SceneItem::display(15, 0, SET_Y, 20, SET_FONT, 2, SET_BG_COLOUR, -1, SET_EXT_BGCOLOUR, 7,
				SET_WIDTH, 320, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(300);
		break;
	case 2: {
		SceneItem::display(15, 1, SET_Y, 20, SET_FONT, 2, SET_BG_COLOUR, -1, SET_EXT_BGCOLOUR, 7,
				SET_WIDTH, 320, SET_KEEP_ONSCREEN, 1, LIST_END);
		parent->_object1.postInit();
		parent->_object1.setVisage(15);
		parent->_object1.setPosition(Common::Point(160, -10));
		parent->_object1.animate(ANIM_MODE_2, NULL);
		Common::Point pt(160, 100);
		NpcMover *mover = new NpcMover();
		parent->_object1.addMover(mover, &pt, this);
		parent->_soundHandler.startSound(7);
		break;
	}
	case 3:
		SceneItem::display(0, 0);
		_globals->_sceneManager.changeScene(20);
		break;
	}			
}

void Scene15::Scene15_Action1::dispatch() {
	Scene15 *parent = (Scene15 *)_globals->_sceneManager._scene;

	if (parent->_object1._position.y < 100)
		parent->_object1.changeZoom(100 - parent->_object1._position.y);
	Action::dispatch();
}

/*--------------------------------------------------------------------------*/

void Scene15::postInit(SceneObjectList *OwnerList) {
	loadScene(15);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);
	_globals->_soundHandler.startSound(6);
	setAction(&_action1);
}

/*--------------------------------------------------------------------------
 * Scene 20 - Cut-scenes where House Chmeee is in the distance
 *
 *--------------------------------------------------------------------------*/

void Scene20::Scene20_Action1::signal() {
	Scene20 *parent = (Scene20 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1:
		parent->_stripManager.start(20, this);
		break;
	case 2:
		_globals->_soundHandler.proc1(this);
		break;
	case 3:
		_globals->_sceneManager._FadeMode = FADEMODE_GRADUAL;
		_globals->_sceneManager.changeScene(30);	// First game scene
		break;
	default:
		break;
	}
}

void Scene20::Scene20_Action2::signal() {
	Scene20 *parent = (Scene20 *)_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(10);
		break;
	case 1:
		SceneItem::display(20, 1, SET_WIDTH, 200, SET_Y, 20, SET_X, 160, SET_KEEP_ONSCREEN, true, 
			SET_EXT_BGCOLOUR, 4, LIST_END);
		setDelay(120);
		break;
	case 2: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(455, 77);
		_globals->_player.addMover(mover, &pt, this);
		ObjectMover2 *mover2 = new ObjectMover2();
		parent->_sceneObject2.addMover(mover2, 5, 10, &_globals->_player);
		ObjectMover2 *mover3 = new ObjectMover2();
		parent->_sceneObject3.addMover(mover3, 10, 15, &_globals->_player);
		break;
	}
	case 3: {
		npcMover = new NpcMover();
		Common::Point pt(557, 100);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		npcMover = new NpcMover();
		Common::Point pt(602, 90);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 5: {
		npcMover = new NpcMover();
		Common::Point pt(618, 90);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 6: {
		npcMover = new NpcMover();
		Common::Point pt(615, 81);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 7: {
		npcMover = new NpcMover();
		Common::Point pt(588, 79);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 8:
		parent->_sound.proc4();
		parent->_sound.proc1(this);
		break;
	case 9:
		SceneItem::display(0, 0, LIST_END);
		_globals->_sceneManager._FadeMode = FADEMODE_GRADUAL;
		_globals->_sceneManager.changeScene(40);
		break;
	default:
		break;
	}
}

void Scene20::Scene20_Action3::signal() {
	Scene20 *parent = (Scene20 *)_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1: {
		npcMover = new NpcMover();
		Common::Point pt(615, 81);
		_globals->_player.addMover(npcMover, &pt, this);
		ObjectMover2 *mover1 = new ObjectMover2();
		parent->_sceneObject2.addMover(mover1, 5, 10, &_globals->_player);
		ObjectMover2 *mover2 = new ObjectMover2();
		parent->_sceneObject3.addMover(mover2, 20, 25, &_globals->_player);
		break;
	}
	case 2: {
		npcMover = new NpcMover();
		Common::Point pt(618, 90);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 3: {
		_globals->_player._moveDiff = Common::Point(10, 10);
		parent->_sceneObject2._moveDiff = Common::Point(10, 10);
		parent->_sceneObject3._moveDiff = Common::Point(10, 10);
		npcMover = new NpcMover();
		Common::Point pt(445, 132);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		npcMover = new NpcMover();
		Common::Point pt(151, 137);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 5: {
		npcMover = new NpcMover();
		Common::Point pt(-15, 137);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 6:
		parent->_sound.startSound(60, this, 127);
		_globals->_soundHandler.proc4();
		break;
	case 7:
		_globals->_sceneManager._FadeMode = FADEMODE_GRADUAL;
		_globals->_sceneManager.changeScene(90);
		break;
	default:
		break;
	}
}

void Scene20::Scene20_Action4::signal() {
	Scene20 *parent = (Scene20 *)_globals->_sceneManager._scene;
	NpcMover *npcMover;

	switch (_actionIndex++) {
	case 0:
		setDelay(60);
		break;
	case 1: {
		npcMover = new NpcMover();
		Common::Point pt(486, 134);
		_globals->_player.addMover(npcMover, &pt, this);
		ObjectMover2 *mover1 = new ObjectMover2();
		parent->_sceneObject2.addMover(mover1, 20, 35, &_globals->_player);
		break;
	}
	case 2: {
		_globals->_player._moveDiff = Common::Point(12, 12);
		parent->_sceneObject2._moveDiff = Common::Point(12, 12);
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(486, 134);
		parent->_sceneObject3.addMover(mover1, &pt, this);
		NpcMover *mover2 = new NpcMover();
		pt = Common::Point(-15, 134);
		_globals->_player.addMover(mover2, &pt, NULL);
		NpcMover *mover3 = new NpcMover();
		pt = Common::Point(-15, 134);
		parent->_sceneObject2.addMover(mover3, &pt, NULL);
		break;
	}
	case 3: {
		parent->_sceneObject3._moveDiff = Common::Point(20, 20);
		npcMover = new NpcMover();
		Common::Point pt(320, 134);
		parent->_sceneObject3.addMover(npcMover, &pt, this);
		break;
	}
	case 4: {
		parent->_sound.startSound(28);
		parent->_sceneObject4.postInit();
		parent->_sceneObject4.setVisage(21);
		parent->_sceneObject4.setStrip(3);
		parent->_sceneObject4.setPosition(Common::Point(parent->_sceneObject3._position.x - 36,
			parent->_sceneObject3._position.y - 1));
		parent->_sceneObject4._moveDiff.x = 48;

		ObjectMover3 *mover = new ObjectMover3();
		parent->_sceneObject4.addMover(mover, &parent->_sceneObject2, 4, this);
		break;
	}
	case 5: {
		parent->_sound.startSound(42);
		parent->_sceneObject4.remove();
		parent->_sceneObject2.setVisage(21);
		parent->_sceneObject2.setStrip(1);
		parent->_sceneObject2.setFrame(1);
		parent->_sceneObject2.animate(ANIM_MODE_5, NULL);
		
		parent->_sceneObject2._moveDiff.x = 4;
		NpcMover *mover1 = new NpcMover();
		Common::Point pt(parent->_sceneObject2._position.x - 12, parent->_sceneObject2._position.y + 5);
		parent->_sceneObject2.addMover(mover1, &pt, NULL);

		parent->_sceneObject5.postInit();
		parent->_sceneObject5.setVisage(21);
		parent->_sceneObject5.setStrip(3);
		parent->_sceneObject5.setPosition(Common::Point(parent->_sceneObject3._position.x - 36,
			parent->_sceneObject3._position.y - 1));
		parent->_sceneObject5._moveDiff.x = 48;

		ObjectMover3 *mover = new ObjectMover3();
		parent->_sceneObject5.addMover(mover, &_globals->_player, 4, this);
		break;
	}
	case 6: {
		parent->_sound.startSound(42);
		parent->_sceneObject2.setStrip(2);
		parent->_sceneObject2.animate(ANIM_MODE_2, NULL);
		
		parent->_sceneObject5.remove();
		_globals->_player.setVisage(21);
		_globals->_player.setStrip(1);
		_globals->_player.setFrame(1);
		_globals->_player.animate(ANIM_MODE_5, this);
		_globals->_player._moveDiff.x = 4;

		npcMover = new NpcMover();
		Common::Point pt(_globals->_player._position.x - 25, _globals->_player._position.y + 5);
		_globals->_player.addMover(npcMover, &pt, this);
		break;
	}
	case 7:
		_globals->_player.setStrip(2);
		_globals->_player.animate(ANIM_MODE_2, NULL);
		parent->_sound.startSound(77, this, 127);
		break;
	case 8:
		_globals->_game.endGame(20, 0);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene20::Scene20() {
}

void Scene20::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerGameText);
	_speakerQText._npc = &_globals->_player;

	if (_globals->_sceneManager._previousScene == 30) {
		_globals->_player.postInit();
		_globals->_player.setVisage(20);
		_globals->_player.setPosition(Common::Point(405, 69));
		_globals->_player._moveDiff = Common::Point(10, 10);
		_globals->_player.animate(ANIM_MODE_1, NULL);

		_sceneObject2.postInit();
		_sceneObject2.setVisage(20);
		_sceneObject2.setPosition(Common::Point(400, 69));
		_sceneObject2.animate(ANIM_MODE_1, NULL);
		
		_sceneObject3.postInit();
		_sceneObject3.setVisage(20);
		_sceneObject3.setPosition(Common::Point(395, 69));
		_sceneObject3.animate(ANIM_MODE_1, NULL);

		_sceneObject2._moveDiff = Common::Point(10, 10);
		_sceneObject3._moveDiff = Common::Point(10, 10);
		_globals->_soundHandler.startSound(20);
		_sound.startSound(21);
		_sound.proc5(1);
		setAction(&_action2);

		_sceneBounds = Rect(320, 0, 640, 200);
	} else if (_globals->_sceneManager._previousScene == 60) {
		_globals->_player.postInit();
		_globals->_player.setVisage(2640);
		_globals->_player.animate(ANIM_MODE_NONE, NULL);
		_globals->_player.setStrip2(1);
		_globals->_player.setFrame2(4);
		_globals->_player.setPriority2(200);
		_globals->_player.setPosition(Common::Point(425, 233));

		setAction(&_action1);
		_speakerQText.setTextPos(Common::Point(350, 20));
		_speakerQText._textWidth = 260;
		_speakerGameText.setTextPos(Common::Point(350, 20));
		_speakerGameText._textWidth = 260;
		
		_globals->_soundHandler.startSound(8);
		_sceneBounds = Rect(320, 0, 640, 200);
	} else {
		_sound.startSound(30);
		_globals->_player.postInit();
		_globals->_player.setVisage(20);
		_globals->_player.setPosition(Common::Point(588, 79));
		_globals->_player._moveDiff = Common::Point(5, 5);
		_globals->_player.setPriority2(50);
		_globals->_player.animate(ANIM_MODE_1, NULL);

		_sceneObject2.postInit();
		_sceneObject2.setVisage(20);
		_sceneObject2.setPosition(Common::Point(583, 79));
		_sceneObject2.animate(ANIM_MODE_1, NULL);

		_sceneObject3.postInit();
		_sceneObject3.setVisage(20);
		_sceneObject3.setStrip(2);
		_sceneObject2.setPosition(Common::Point(595, 79));
		_sceneObject2.animate(ANIM_MODE_1, NULL);

		if ((_globals->getFlag(120) && _globals->getFlag(116)) || 
				(_globals->getFlag(117) && _globals->getFlag(119))) {
			setAction(&_action3);
		} else if (_globals->getFlag(104)) {
			_sceneMode = 21;
			setAction(&_sequenceManager, this, 21, &_globals->_player, &_sceneObject2, NULL);
		} else {
			_sceneObject3._moveDiff = Common::Point(8, 8);
			setAction(&_action4);
		}

		_sceneBounds.centre(_globals->_player._position.x, _globals->_player._position.y);
	}

	_globals->_player.disableControl();
	loadScene(20);
}

void Scene20::signal() {
	if (_sceneMode == 21)
		_globals->_sceneManager.changeScene(90);
}

/*--------------------------------------------------------------------------
 * Scene 30 - First game scene (Outside Ch'mee house)
 *
 *--------------------------------------------------------------------------*/

void Scene30::Scene30_beamAction::signal() {
	Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		// Disable control and move player to the doorway beam
		_globals->_player.disableControl();
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}

	case 1:
		// Perform the animation of player raising hand
		_globals->_player.setVisage(31);
		_globals->_player.setStrip(1);
		_globals->_player.setFrame(1);
		_globals->_player.animate(ANIM_MODE_5, this);
		break;

	case 2:
		// Hide the beam and lower the player's hand
		parent->_sound.startSound(10, NULL, 127);
		_globals->_player.animate(ANIM_MODE_6, this);
		parent->_beam.remove();
		break;

	case 3: {
		// Bring the Kzin to the doorway
		_globals->_player.setVisage(0);
		_globals->_player.animate(ANIM_MODE_1, NULL);
		_globals->_player.setStrip(7);
		parent->_kzin.postInit();
		parent->_kzin.setVisage(2801);
		parent->_kzin.animate(ANIM_MODE_1, NULL);
		parent->_kzin.setObjectWrapper(new SceneObjectWrapper());
		parent->_kzin.setPosition(Common::Point(334, 1));
		NpcMover *mover = new NpcMover();
		Common::Point pt(158, 170);
		parent->_kzin.addMover(mover, &pt, this);
		_globals->_sceneItems.push_front(&parent->_kzin);
		break;
	}

	case 4:
		// Open the door
		parent->_sound.startSound(11, NULL, 127);
		parent->_door.animate(ANIM_MODE_5, this);
		break;

	case 5:
		// Run the Kzin's talk sequence
		parent->_sound.startSound(13, NULL, 127);
		_globals->_soundHandler.startSound(12, NULL, 127);
		parent->_stripManager.start((parent->_sceneMode == 0) ? 30 : 37, this);
		break;

	case 6:
		// Slight delay
		setDelay(3);
		break;

	case 7:
		// Re-activate player control
		parent->_sceneMode = 31;
		parent->_kzin.setAction(&parent->_kzinAction);
		_globals->_player.enableControl();

		// End this action
		remove();
		break;

	default:
		break;
	}
}

void Scene30::Scene30_kzinAction::signal() {
	Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(1200);
		break;
	case 1:
		_globals->_soundHandler.proc2(0);
		_globals->_player.disableControl();
		setAction(&parent->_sequenceManager, _globals->_sceneManager._scene, 31, &parent->_kzin, &parent->_door, NULL);
		break;
	case 2:
		_globals->_player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

void Scene30::Scene30_ringAction::signal() {
	Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {	
		_globals->_player.disableControl();
		parent->_kzin.setAction(NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	
	case 1:
		_globals->_player.checkAngle(&parent->_kzin);
		parent->_stripManager.start(32, this);
		break;

	case 2: {
		_globals->_player.animate(ANIM_MODE_1, NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(143, 177);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}

	case 3:
		parent->_sound.startSound(11, NULL, 127);
		parent->_door.animate(ANIM_MODE_6, this);
		break;

	case 4: {
		parent->_sound.startSound(13, NULL, 127);
		NpcMover *kzinMover = new NpcMover();
		Common::Point pt(354, 5);
		parent->_kzin.addMover(kzinMover, &pt, this);
		NpcMover *playerMover = new NpcMover();
		pt = Common::Point(335, 36);
		_globals->_player.addMover(playerMover, &pt, this);
		break;
	}

	case 5:
		break;

	case 6:
		_globals->_sceneManager.changeScene(20);
		break;

	default:
		break;
	}
}

void Scene30::Scene30_talkAction::signal() {
	Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		_globals->_player.disableControl();
		parent->_kzin.setAction(NULL);
		NpcMover *mover = new NpcMover();
		Common::Point pt(114, 198);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		_globals->_player.checkAngle(&parent->_kzin);
		parent->_stripManager.start(34, this);
		break;
	case 2:
		setDelay(5);
		break;
	case 3:
		parent->_kzin.setAction(&parent->_kzinAction);
		_globals->_player.enableControl();
		remove();
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene30::Scene30_kzin::doAction(int action) {
	Scene30 *parent = (Scene30 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		display2(30, 12);
		break;
	case OBJECT_SCANNER:
		display2(30, 11);
		break;
	case OBJECT_RING:
		_globals->_inventory._ring._sceneNumber = 30;
		parent->setAction(&parent->_ringAction);
		break;
	case CURSOR_LOOK:
		display2(30, 6);
		break;
	case CURSOR_USE:
		display2(30, 10);
		break;
	case CURSOR_TALK:
		_globals->_player.disableControl();
		parent->setAction(&parent->_talkAction);
		break;
	default:
		SceneObject::doAction(action);	
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene30::Scene30():
	_groundHotspot(9, OBJECT_SCANNER, 50, 17, CURSOR_LOOK, 30, 3, CURSOR_USE, 30, 8, LIST_END),
	_wallsHotspot(8, OBJECT_SCANNER, 50, 13, CURSOR_LOOK, 30, 0, CURSOR_USE, 30, 7, LIST_END),
	_courtyardHotspot(0, CURSOR_LOOK, 30, 4, LIST_END),
	_treeHotspot(10, OBJECT_SCANNER, 40, 39, CURSOR_LOOK, 30, 5, CURSOR_USE, 30, 9, LIST_END) {
}

void Scene30::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	// Add the speaker classes to the strip manager
	_stripManager.addSpeaker(&_speakerQL);
	_stripManager.addSpeaker(&_speakerSR);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerQText);
	_speakerSText._npc = &_kzin;
	_speakerQText._npc = &_globals->_player;
	

	// Setup player
	_globals->_player.postInit();
	_globals->_player.setVisage(0);
	_globals->_player.animate(ANIM_MODE_1);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setStrip(7);
	_globals->_player.setFrame(1);
	_globals->_player.setPosition(Common::Point(114, 198));
	_globals->_player.changeZoom(75);
	_globals->_player.enableControl();

	// Set up beam object
	_beam.postInit();
	_beam.setVisage(31);
	_beam.setStrip(2);
	_beam.setPosition(Common::Point(124, 178));
	_beam.setPriority2(188);

	// Set up door object
	_door.postInit();
	_door.setVisage(30);
	_door.setPosition(Common::Point(150, 183));

	// Final processing and add of scene items
	_courtyardHotspot.setBounds(Rect(0, 0, 320, 200));

	// Add the objects and hotspots to the scene
	_globals->_sceneItems.addItems(&_beam, &_wallsHotspot, &_door, &_treeHotspot, &_groundHotspot, 
		&_courtyardHotspot, NULL);

	// Load the scene data
	loadScene(30);
	_sceneMode = 0;
}

void Scene30::signal() {
	if (_sceneMode == 31) {
		// Re-activate beam if the Kzin goes back inside
		_beam.postInit();
		_beam.setVisage(31);
		_beam.setStrip(2);
		_beam.setPosition(Common::Point(124, 178));
		_beam.setPriority2(188);
		_globals->_sceneItems.push_front(&_beam);
		_globals->_player.enableControl();
	} else if (_sceneMode == 32) {
		_globals->_player.disableControl();
		_sceneMode = 31;
		setAction(&_sequenceManager, _globals->_sceneManager._scene, 31, &_kzin, &_door, NULL);
	}
}

/*--------------------------------------------------------------------------
 * Scene 40 - Chmeee Home
 *
 *--------------------------------------------------------------------------*/

void Scene40::Scene40_Action1::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(120);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		parent->_stripManager.start(40, this);
		break;
	case 2:
		parent->_doorway.postInit();
		parent->_doorway.setVisage(46);
		parent->_doorway.setPosition(Common::Point(305, 61));
		parent->_doorway.animate(ANIM_MODE_5, this);
		parent->_soundHandler.startSound(25);
		break;
	case 3:
		parent->_doorway.flag100();
		parent->_dyingKzin.setPosition(Common::Point(296, 62));
		_globals->_player.animate(ANIM_MODE_5, NULL);
		parent->_object1.setVisage(43);
		parent->_object1.setStrip(3);
		parent->_object1.animate(ANIM_MODE_5, NULL);
		parent->_object2.flag100();
		parent->_object3.flag100();
		parent->_stripManager.start(45, this);
		break;
	case 4:
		parent->_object2.remove();
		parent->_object3.remove();
		parent->_assassin.setVisage(42);
		parent->_assassin.setStrip(2);
		parent->_assassin.setFrame(1);
		parent->_assassin.setPosition(Common::Point(13, 171));
		parent->_assassin.animate(ANIM_MODE_5, this);
		parent->_soundHandler.startSound(25);
		break;
	case 5:
		parent->_doorway.unflag100();
		parent->_doorway.setVisage(42);
		parent->_doorway.setStrip(3);
		parent->_doorway.setFrame(1);
		parent->_doorway.setPosition(Common::Point(41, 144));
		parent->_assassin.animate(ANIM_MODE_6, NULL);
		setDelay(6);
		break;
	case 6:
		parent->_doorway.setPosition(Common::Point(178, 101));
		setDelay(6);
		break;
	case 7:
		parent->_doorway.setPosition(Common::Point(271, 69));
		setDelay(6);
		break;
	case 8:
		parent->_doorway.remove();
		parent->_dyingKzin.animate(ANIM_MODE_5, this);
		break;
	case 9: {
		parent->_dyingKzin.setStrip(1);
		parent->_dyingKzin.setFrame(1);
		parent->_dyingKzin._moveDiff.y = 15;
		parent->_dyingKzin.animate(ANIM_MODE_5, NULL);
		Common::Point pt(223, 186);
		NpcMover *mover = new NpcMover();
		parent->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 10: {
		parent->_soundHandler.startSound(27);
		Common::Point pt(223, 184);
		NpcMover *mover = new NpcMover();
		parent->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 11: {
		Common::Point pt(223, 186);
		NpcMover *mover = new NpcMover();
		parent->_dyingKzin.addMover(mover, &pt, this);
		break;
	}
	case 12: {
		_globals->_soundHandler.startSound(26);
		_globals->_player._uiEnabled = true;
		parent->_assassin.setVisage(42);
		parent->_assassin.setPosition(Common::Point(4, 191));
		parent->_assassin.setStrip(1);
		parent->_assassin.animate(ANIM_MODE_1, NULL);
		Common::Point pt(230, 187);
		NpcMover *mover = new NpcMover();
		parent->_assassin.addMover(mover, &pt, this);
		break;
	}
	case 13:
		setDelay(180);
		break;
	case 14:
		parent->_assassin.setVisage(45);
		parent->_assassin.setStrip(1);
		parent->_assassin.setFrame(1);
		parent->_assassin.animate(ANIM_MODE_5, this);
		parent->_soundHandler.startSound(28);
		break;
	case 15:
		_globals->_player.disableControl();
		parent->_object1.setVisage(40);
		parent->_object1.setStrip(4);
		parent->_object1.setFrame(1);
		parent->_object1.animate(ANIM_MODE_5, NULL);
		_globals->_player.setVisage(40);
		_globals->_player.setStrip(2);
		_globals->_player.setFrame(1);
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 16:
		_globals->_soundHandler.startSound(77, this);
		break;
	case 17:
		_globals->_game.endGame(40, 20);
		remove();
		break;
	}
}

void Scene40::Scene40_Action2::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		if (parent->_assassin._position.x < 229)
			_actionIndex = 0;
		setDelay(1);
		break;
	case 1:
		parent->_assassin.animate(ANIM_MODE_NONE, NULL);
		_globals->_player.setStrip(2);
		_globals->_player.setFrame(1);
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		parent->_soundHandler.startSound(28);
		parent->_doorway.postInit();
		parent->_doorway.setVisage(16);
		parent->_doorway.setStrip2(6);
		parent->_doorway.setPriority2(200);
		parent->_doorway.setPosition(Common::Point(159, 191));
		parent->_doorway._moveDiff = Common::Point(40, 40);
		parent->_doorway._field7A = 60;
		parent->_doorway.animate(ANIM_MODE_5, NULL);

		Common::Point pt(271, 165);
		NpcMover *mover = new NpcMover();
		parent->_doorway.addMover(mover, &pt, this);
		break;
	}
	case 3:
		parent->_doorway.remove();
		parent->_assassin.setVisage(44);
		parent->_assassin._frame = 1;
		parent->_assassin.animate(ANIM_MODE_5, this);
		parent->_soundHandler.startSound(29);
		_globals->_inventory._infoDisk._sceneNumber = 40;
		break;
	case 4:
		_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 5: {
		_globals->_player.setVisage(0);
		_globals->_player.animate(ANIM_MODE_1, NULL);
		_globals->_player.setStrip(1);
		Common::Point pt(230, 195);
		PlayerMover *mover = new PlayerMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 6: {
		_globals->_player.setStrip(7);
		parent->_object1.setVisage(2806);
		parent->_object1.animate(ANIM_MODE_1, NULL);
		SceneObjectWrapper *wrapper = new SceneObjectWrapper();
		parent->_object1.setObjectWrapper(wrapper);
		Common::Point pt(200, 190);
		NpcMover *mover = new NpcMover();
		parent->_object1.addMover(mover, &pt, this);
		break;
	}
	case 7:
		parent->_stripManager.start(44, this);
		break;
	case 8: {
		Common::Point pt(170, 260);
		NpcMover *mover = new NpcMover();
		parent->_object1.addMover(mover, &pt, this);
		break;
	}
	case 9:
		parent->_dyingKzin.setAction(&parent->_action7);
		parent->_object1.remove();
		_globals->_stripNum = 88;
		_globals->_events.setCursor(CURSOR_WALK);
		_globals->_player.enableControl();
		parent->_assassin.setAction(&parent->_action8);
		break;
	}
}

void Scene40::Scene40_Action3::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		_globals->_player.setAction(NULL);
		_globals->_stripNum = 99;
		_globals->_player.disableControl();
		Common::Point pt(240, 195);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		_globals->_player.setVisage(5010);
		_globals->_player._strip = 2;
		_globals->_player._frame = 1;
		_globals->_player.animate(ANIM_MODE_4, 5, 1, this);
		break;
	case 2:
		parent->_assassin.setStrip(2);
		parent->_assassin.setFrame(1);
		_globals->_inventory._infoDisk._sceneNumber = 1;
		_globals->_player.animate(ANIM_MODE_6, this);
		break;
	case 3:
		_globals->_player.setVisage(0);
		_globals->_player.animate(ANIM_MODE_1, NULL);
		_globals->_player.setStrip(7);
		_globals->_stripNum = 88;
		_globals->_player.enableControl();
		remove();
		break;
	}
}

void Scene40::Scene40_Action4::signal() {
	switch (_actionIndex++) {
	case 0: {
		Common::Point pt(178, 190);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		_globals->_stripNum = 88;
		_globals->_player.enableControl();
		break;
	}
}

void Scene40::Scene40_Action5::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: 
		setDelay(_globals->_randomSource.getRandomNumber(120));
		break;
	case 1:
		parent->_object2.animate(ANIM_MODE_8, 1, this);
		_actionIndex = 0;
	}
}

void Scene40::Scene40_Action6::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		parent->_object1.postInit();
		parent->_object1.setVisage(16);
		parent->_object1.setStrip2(6);
		parent->_object1.setPosition(Common::Point(313, 53));
		parent->_object1._field7A = 60;

		Common::Point pt(141, 194);
		NpcMover *mover = new NpcMover();
		parent->_object1.addMover(mover, &pt, this);
		parent->_object1.animate(ANIM_MODE_5, NULL);

		parent->_doorway.postInit();
		parent->_doorway.setVisage(46);
		parent->_doorway.setPosition(Common::Point(305, 61));
		parent->_doorway.animate(ANIM_MODE_5, this);
		parent->_soundHandler.startSound(25);
		break;
	}
	case 1:
		parent->_soundHandler.startSound(28);
		parent->_doorway.setPosition(Common::Point(148, 74));
		parent->_doorway.setFrame(1);
		parent->_doorway.setStrip(2);
		parent->_doorway.animate(ANIM_MODE_5, this);
		break;
	case 2:
		remove();
		break;
	}
}

void Scene40::Scene40_Action7::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(_globals->_randomSource.getRandomNumber(500));
		break;
	case 1:
		parent->_object7.postInit();
		parent->_object7.setVisage(46);

		if (_globals->_randomSource.getRandomNumber(32767) >= 16384) {
			parent->_object7.setStrip(3);
			parent->_object7.setPosition(Common::Point(15, 185));
		} else {
			parent->_object7.setPosition(Common::Point(305, 61));
			parent->_object7.setFrame(15);
		}
		break;
	case 2:
		parent->_object7.remove();
		_actionIndex = 0;
		setDelay(60);
		break;
	}
}

void Scene40::Scene40_Action8::signal() {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setDelay(300);
		break;
	case 1:
		_globals->_player.disableControl();
		
		if ((_globals->_player._position.y >= 197) || (_globals->_player._visage)) {
			_actionIndex = 1;
			setDelay(30);
		} else {
			parent->_doorway.postInit();
			parent->_doorway.setVisage(16);
			parent->_doorway.setStrip2(6);
			parent->_doorway.setPriority2(200);
			parent->_doorway._field7A = 60;

			if (_globals->_player._position.x >= 145) {
				parent->_doorway.setPriority2(-1);
				parent->_doorway.setPosition(Common::Point(6, 157));
			} else {
				parent->_doorway.setPosition(Common::Point(313, 53));
			}

			parent->_doorway._moveDiff = Common::Point(40, 40);
			Common::Point pt(_globals->_player._position.x, _globals->_player._position.y - 18);
			NpcMover *mover = new NpcMover();
			parent->_doorway.addMover(mover, &pt, this);
			parent->_doorway.animate(ANIM_MODE_5, NULL);
		}
		break;
	case 2:
		parent->_doorway.remove();
		_globals->_player.setVisage(40);
		_globals->_player.setStrip(2);
		_globals->_player.setFrame(1);
		_globals->_player.animate(ANIM_MODE_5, this);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene40::Scene40_DyingKzin::doAction(int action) {
	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(40, 43);
		break;
	case CURSOR_CROSSHAIRS:
		SceneItem::display2(40, 44); 
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 12);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 18);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene40::Scene40_Assassin::doAction(int action) {
	Scene40 *parent = (Scene40 *)_globals->_sceneManager._scene;

	switch (action) {
	case CURSOR_CROSSHAIRS:
		if (parent->_assassin._visage == 44)
			SceneItem::display2(40, 21);
		else {
			_globals->_player.disableControl();
			Common::Point pt(230, 187);
			NpcMover *mover = new NpcMover();
			addMover(mover, &pt, NULL);
		}
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, (parent->_assassin._visage == 44) ? 22 : 23);
		break;
	case CURSOR_LOOK:
		if (parent->_assassin._visage != 44)
			SceneItem::display2(40, 13);
		else
			SceneItem::display2(40, (_globals->_inventory._infoDisk._sceneNumber == 1) ? 19 : 14);
		break;
	case CURSOR_USE:
		if (parent->_assassin._visage != 44)
			SceneItem::display2(40, 15);
		else if (_globals->_inventory._infoDisk._sceneNumber == 1)
			SceneItem::display2(40, 19);
		else {
			_globals->_player.disableControl();
			setAction(&parent->_action3);
		}
		break;
	case CURSOR_TALK:
		SceneItem::display2(40, 38);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

void Scene40::Scene40_Item2::doAction(int action) {
	switch (action) {
	case CURSOR_CROSSHAIRS:
		SceneItem::display2(40, 35);
		_globals->_events.setCursor(CURSOR_WALK);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, 34);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 8);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 36);
		break;
	case CURSOR_TALK:
		SceneItem::display2(40, 37);
		break;
	default:
		SceneItem::doAction(action);
		break;
	}
}

void Scene40::Scene40_Item6::doAction(int action) {
	switch (action) {
	case CURSOR_CROSSHAIRS:
		SceneItem::display2(40, 25);
		_globals->_events.setCursor(CURSOR_WALK);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(40, 42);
		break;
	case CURSOR_LOOK:
		SceneItem::display2(40, 6);
		break;
	case CURSOR_USE:
		SceneItem::display2(40, 36);
		break;
	default:
		SceneItem::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene40::Scene40():
	_item1(2, OBJECT_SCANNER, 40, 24, CURSOR_CROSSHAIRS, 40, 25, CURSOR_LOOK, 40, 7, CURSOR_USE, 40, 16, LIST_END),
	_item3(5, OBJECT_SCANNER, 40, 26, CURSOR_CROSSHAIRS, 40, 27, CURSOR_LOOK, 40, 9, CURSOR_USE, 40, 17, LIST_END),
	_item4(6, OBJECT_SCANNER, 40, 31, CURSOR_CROSSHAIRS, 40, 32, CURSOR_LOOK, 40, 5, CURSOR_USE, 40, 33, LIST_END),
	_item5(0, CURSOR_LOOK, 40, 11, LIST_END),
	_item7(4, OBJECT_SCANNER, 40, 26, CURSOR_CROSSHAIRS, 40, 27, CURSOR_LOOK, 40, 9, CURSOR_USE, 40, 17, LIST_END),
	_item8(8, OBJECT_SCANNER, 40, 39, CURSOR_CROSSHAIRS, 40, 40, CURSOR_LOOK, 40, 3, CURSOR_USE, 40, 41, LIST_END) {
}

void Scene40::postInit(SceneObjectList *OwnerList) {
	loadScene(40);
	Scene::postInit();

	setZoomPercents(0, 100, 200, 100);
	_globals->_stripNum = 99;

	_stripManager.addSpeaker(&_speakerQR);
	_stripManager.addSpeaker(&_speakerSL);
	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	_stripManager.addSpeaker(&_speakerGameText);

	_speakerGameText._colour1 = 9;
	_speakerGameText.setTextPos(Common::Point(160, 30));
	_speakerQText._npc = &_globals->_player;
	_speakerSText._npc = &_object1;

	_globals->_player.postInit();
	_globals->_player.setVisage(0);
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player.setPosition(Common::Point(130, 220));
	_globals->_player.disableControl();

	if (_globals->_sceneManager._previousScene == 20) {
		_globals->_soundHandler.startSound(24);
		_globals->_player.setVisage(43);
		
		_object1.postInit();
		_object1.setVisage(41);
		_object1.setPosition(Common::Point(105, 220));
		_object2.postInit();
		_object2.setVisage(41);
		_object2.setStrip(6);
		_object2.setPriority2(200);
		_object2.setPosition(Common::Point(94, 189));
		_object2.setAction(&_action5);

		_object3.postInit();
		_object3.setVisage(41);
		_object3.setStrip(5);
		_object3.setPriority2(205);
		_object3.setPosition(Common::Point(110, 186));
		_object3._numFrames = 2;
		_object3.animate(ANIM_MODE_8, NULL, NULL);
		
		_assassin.postInit();
		_assassin.setPosition(Common::Point(-40, 191));
		_globals->_sceneItems.push_back(&_assassin);

		_dyingKzin.postInit();
		_dyingKzin.setVisage(40);
		_dyingKzin.setStrip(6);
		_dyingKzin.setPosition(Common::Point(-90, 65));
		_dyingKzin.setPriority2(170);

		setAction(&_action1);
	} else {
		_doorway.postInit();
		_doorway.setVisage(46);
		_doorway.setPosition(Common::Point(148, 74));
		_doorway.setStrip(2);
		_doorway.setFrame(_doorway.getFrameCount());
		
		_dyingKzin.postInit();
		_dyingKzin.setVisage(40);
		_dyingKzin.setPosition(Common::Point(205, 183));
		_dyingKzin.setPriority2(170);
		_dyingKzin._frame = 9;
		_dyingKzin.setAction(&_action7);

		_assassin.postInit();
		_assassin.setVisage(44);
		_assassin.setPosition(Common::Point(230, 187));
		_assassin.setAction(&_action8);

		if (_globals->_inventory._infoDisk._sceneNumber == 40) {
			_assassin.setStrip(1);
			_assassin.setFrame(_assassin.getFrameCount());
		} else {
			_assassin.setStrip(2);
		}

		_globals->_sceneItems.push_back(&_assassin);
		_globals->_player.setPosition(Common::Point(170, 220));

		setAction(&_action4);
	}

	_item5.setBounds(Rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT));
	_item6._sceneRegionId = 3;
	_item2._sceneRegionId = 7;
	
	_globals->_sceneItems.addItems(&_dyingKzin, &_item8, &_item1, &_item2, &_item3, &_item4, 
			&_item6, &_item7, &_item5, NULL);
}

void Scene40::signal() {
	if (_sceneMode == 41)
		_globals->_sceneManager.changeScene(50);
}

void Scene40::dispatch() {
	if ((_globals->_stripNum == 88) && (_globals->_player._position.y >= 197)) {
		_globals->_player.disableControl();
		_globals->_stripNum = 0;
		_globals->_player.setAction(NULL);
		_sceneMode = 41;
		setAction(&_sequenceManager, this, 41, &_globals->_player, NULL);

		if (_globals->_sceneManager._previousScene == 20) {
			_dyingKzin.setAction(&_action6);
		}
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 50 - By Flycycles
 *
 *--------------------------------------------------------------------------*/

void Scene50::Scene50_Action1::signal() {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		setAction(&parent->_sequenceManager, this, 54, &_globals->_player, NULL);
		break;
	case 1:
		_globals->_events.setCursor(CURSOR_WALK);
		parent->_stripManager.start(63, this);
		break;
	case 2:
		if (parent->_stripManager._field2E8 != 107) {
			_globals->_player.enableControl();
			remove();
		} else {
			Common::Point pt(282, 139);
			NpcMover *mover = new NpcMover();
			_globals->_player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		_globals->_stripNum = -1;
		_globals->_sceneManager.changeScene(60);
		break;
	}
}

void Scene50::Scene50_Action2::signal() {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		parent->_stripManager.start(66, this);
		break;
	case 1: {
		Common::Point pt(141, 142);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 2:
		_globals->_sceneManager.changeScene(40);
		remove();
		break;
	}
}

void Scene50::Scene50_Action3::signal() {
	switch (_actionIndex++) {
	case 0: {
		_globals->_player.disableControl();
		Common::Point pt(136, 185);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 1:
		_globals->_sceneManager.changeScene(60);
		remove();
		break;
	}
}		

/*--------------------------------------------------------------------------*/

void Scene50::Scene50_Object1::doAction(int action) {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 20);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 19); 
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 4);
		break;
	case CURSOR_USE:
		SceneItem::display2(50, 21);
		break;
	case CURSOR_TALK:
		_globals->_player.disableControl();
		parent->_sceneMode = 52;
		parent->setAction(&parent->_sequenceManager, parent, 52, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Scene50_Object2::doAction(int action) {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10); 
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		_globals->_stripNum = 50;
		parent->setAction(&parent->_action3);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Scene50_Object3::doAction(int action) {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10); 
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		SceneItem::display2(50, 8);
		break;
	case CURSOR_TALK:
		_globals->_player.disableControl();
		parent->_sceneMode = 52;
		parent->setAction(&parent->_sequenceManager, parent, 52, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

void Scene50::Scene50_Object4::doAction(int action) {
	Scene50 *parent = (Scene50 *)_globals->_sceneManager._scene;

	switch (action) {
	case OBJECT_STUNNER:
		SceneItem::display2(50, 11);
		break;
	case OBJECT_SCANNER:
		SceneItem::display2(50, 10); 
		break;
	case CURSOR_LOOK:
		SceneItem::display2(50, 1);
		break;
	case OBJECT_INFODISK:
	case CURSOR_USE:
		_globals->_player.disableControl();
		_globals->_stripNum = 0;
		parent->_sceneMode = 51;
		parent->setAction(&parent->_sequenceManager, parent, 51, &_globals->_player, NULL);
		break;
	default:
		SceneHotspot::doAction(action);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene50::Scene50(): 
		_item0(0, CURSOR_LOOK, 50, 3, LIST_END),
		_item1(0, OBJECT_SCANNER, 50, 15, CURSOR_USE, 50, 16, CURSOR_LOOK, 50, 3, LIST_END),
		_item2(0, CURSOR_LOOK, 50, 7, LIST_END),
		_item3(8, OBJECT_STUNNER, 50, 14, OBJECT_SCANNER, 50, 13, CURSOR_LOOK, 50, 3, LIST_END),
		_item4(9, OBJECT_SCANNER, 40, 39, OBJECT_STUNNER, 40, 40, CURSOR_USE, 40, 41, CURSOR_LOOK, 50, 5, LIST_END),
		_item5(10, OBJECT_SCANNER, 50, 17, OBJECT_STUNNER, 50, 18, CURSOR_LOOK, 50, 6, CURSOR_USE, 30, 8, LIST_END) {
}

void Scene50::postInit(SceneObjectList *OwnerList) {
	loadScene(50);
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	_stripManager.addSpeaker(&_speakerQText);
	_stripManager.addSpeaker(&_speakerSText);
	
	_globals->_player.postInit();
	_globals->_player.setVisage(0);
	_globals->_player.animate(ANIM_MODE_1, NULL);
	_globals->_player.setObjectWrapper(new SceneObjectWrapper());
	_globals->_player._canWalk = false;
	_globals->_player.changeZoom(75);
	_globals->_player._moveDiff.y = 3;

	if (_globals->_sceneManager._previousScene == 40) {
		_globals->_player.setPosition(Common::Point(128, 123));
	} else if (_globals->_stripNum == 50) {
		_globals->_player.setPosition(Common::Point(136, 185));
	} else {
		_globals->_player.setPosition(Common::Point(270, 143));
	}

	_object2.postInit();
	_object2.setVisage(2331);
	_object2.setStrip(6);
	_object2.setPosition(Common::Point(136, 192));
	_object2.setPriority2(200);

	_object3.postInit();
	_object3.setVisage(2337);
	_object3.setStrip(6);
	_object3.setPosition(Common::Point(260, 180));
	_object3.setPriority2(200);

	_object4.postInit();
	_object4.setVisage(2331);
	_object4.setStrip(6);
	_object4.setPosition(Common::Point(295, 144));
	_object4.setPriority2(178);

	_globals->_sceneItems.addItems(&_object2, &_object3, &_object4, NULL);

	if (!_globals->getFlag(101)) {
		_globals->_player.disableControl();
		_globals->setFlag(101);
		setAction(&_action1);
	} else {
		_globals->_player.enableControl();

		if (_globals->_sceneManager._previousScene == 40) {
			_globals->_player.disableControl();
			_sceneMode = 54;
			setAction(&_sequenceManager, this, 54, &_globals->_player, NULL);
		}
	}

	_item0.setBounds(Rect(200, 0, 320, 200));
	_globals->_sceneItems.addItems(&_item3, &_item4, &_item5, &_item0, NULL);
	_doorwayRect = Rect(80, 108, 160, 112);
}

void Scene50::signal() {
	switch (_sceneMode) {
	case 51:
		_globals->_sceneManager.changeScene(60);
		break;
	case 55:
		_globals->_sceneManager.changeScene(40);
		break;
	case 52:
	case 54:
		_globals->_player.enableControl();
		break;
	}
}

void Scene50::dispatch() {
	if ((_sceneMode != 55) && _doorwayRect.contains(_globals->_player._position)) {
		// Player in house doorway, start player moving to within
		_globals->_player.disableControl();
		_sceneMode = 55;
		Common::Point pt(89, 111);
		NpcMover *mover = new NpcMover();
		_globals->_player.addMover(mover, &pt, this);
	}
}

/*--------------------------------------------------------------------------
 * Scene 1000 - Title Screen
 *
 *--------------------------------------------------------------------------*/

void Scene1000::Scene1000_Action1::signal() {
	Scene1000 *parent = (Scene1000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		parent->_object4.postInit();
		parent->_object4.setVisage(1001);
		parent->_object4._frame = 1;
		parent->_object4.setStrip2(5);
		parent->_object4.changeZoom(100);
		parent->_object4.animate(ANIM_MODE_2, NULL);
		parent->_object4.setPosition(Common::Point(403, 163));
		setDelay(90);
		break;
	case 2: {
		SceneItem::display(0, 0);
		parent->_object4.remove();
		parent->_object1.changeZoom(-1);
		NpcMover *mover = new NpcMover();
		Common::Point pt(180, 100);
		parent->_object1.addMover(mover, &pt, this);
		break;
	}
	case 3:
		_globals->_sceneManager.changeScene(1400);
		break;
	}

}

void Scene1000::Scene1000_Action2::signal() {
	switch (_actionIndex++) {
	case 0:
		_globals->_player.disableControl();
		setDelay(10);
		break;
	case 1:
		SceneItem::display(1000, 0, SET_Y, 20, SET_FONT, 2, SET_BG_COLOUR, -1,
				SET_EXT_BGCOLOUR, 35, SET_WIDTH, 200, SET_KEEP_ONSCREEN, 1, LIST_END);
		setDelay(180);
		break;
	case 2:
		SceneItem::display(0, 0);
		_globals->_sceneManager.changeScene(2000);
		break;
	default:
		break;
	}
}

void Scene1000::Scene1000_Action3::signal() {
	Scene1000 *parent = (Scene1000 *)_globals->_sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		_globals->_sceneManager._scene->loadBackground(0, 0);
		setDelay(60);
		break;
	case 1: {
		NpcMover *mover = new NpcMover();
		Common::Point pt(158, 31);
		parent->_object3.addMover(mover, &pt, this);
		break;
	}
	case 2:
	case 3:
		setDelay(60);
		break;
	case 4:
		_globals->_player.unflag100();
		setDelay(240);
		break;
	case 5: {
		// Intro.txt file presence is used to allow user option to skip the introduction
		_globals->_player.enableControl();
		Common::InSaveFile *in = g_system->getSavefileManager()->openForLoading("Intro.txt");
		if (!in) {
			// File not present, so create it
			Common::OutSaveFile *out = g_system->getSavefileManager()->openForSaving("Intro.txt");
			out->finalize();
			delete out;
			setDelay(1);
		} else {
			delete in;

			// Prompt user for whether to start play or watch introduction
			if (MessageDialog::show2(WATCH_INTRO_MSG, START_PLAY_BTN_STRING, INTRODUCTION_BTN_STRING) == 0) {
				_actionIndex = 20;
				_globals->_soundHandler.proc1(this);
			} else {
				setDelay(1);
			}

			_globals->_player.disableControl();
		}
		break;
	}
	case 6: {
		parent->_object3.remove();
		_globals->_player.setStrip2(2);
		NpcMover *mover = new NpcMover();
		Common::Point pt(480, 100);
		_globals->_player.addMover(mover, &pt, this);
		break;
	}
	case 7:
		_globals->_scenePalette.loadPalette(1002);
		_globals->_scenePalette.refresh();
		_globals->_scenePalette.addRotation(80, 95, -1);
		parent->_object3.postInit();
		parent->_object3.setVisage(1002);
		parent->_object3.setStrip(1);
		parent->_object3.setPosition(Common::Point(284, 122));
		parent->_object3.changeZoom(1);

		zoom(true);
		setDelay(200);
		break;
	case 8:
		zoom(false);
		setDelay(10);
		break;
	case 9:
		parent->_object3.setStrip(2);
		parent->_object3.setPosition(Common::Point(285, 155));

		zoom(true);
		setDelay(400);
		break;
	case 10:
		zoom(false);
		setDelay(10);
		break;
	case 11:
		parent->_object3.setStrip(3);
		parent->_object3.setPosition(Common::Point(279, 172));

		zoom(true);
		setDelay(240);
		break;
	case 12:
		zoom(false);
		setDelay(10);
		break;
	case 13:
		parent->_object3.setStrip(4);
		parent->_object3.setPosition(Common::Point(270, 128));

		zoom(true);
		setDelay(300);
		break;
	case 14:
		zoom(false);
		setDelay(10);
		break;
	case 15:
		parent->_object3.setStrip(1);
		parent->_object3.setFrame(2);
		parent->_object3.setPosition(Common::Point(283, 137));

		zoom(true);
		setDelay(300);
		break;
	case 16:
		zoom(false);
		setDelay(10);
		break;
	case 17:
		parent->_object3.setStrip(5);
		parent->_object3.setFrame(1);
		parent->_object3.setPosition(Common::Point(292, 192));
		
		zoom(true);
		setDelay(300);
		break;
	case 18:
		zoom(false);
		_globals->_scenePalette.clearListeners();
		_globals->_soundHandler.proc1(this);
		break;
	case 19:
		_globals->_sceneManager.changeScene(10);
		break;
	case 20:
		_globals->_sceneManager.changeScene(30);
		break;
	default:
		break;
	}		
}

void Scene1000::Scene1000_Action3::zoom(bool up) {
	Scene1000 *parent = (Scene1000 *)_globals->_sceneManager._scene;

	if (up) {
		while ((parent->_object3._percent < 100) && !_vm->shouldQuit()) {
			parent->_object3.changeZoom(parent->_object3._percent + 5);
			_globals->_sceneObjects->draw();
			_globals->_events.delay(1);
		}
	} else {
		while ((parent->_object3._percent > 0) && !_vm->shouldQuit()) {
			parent->_object3.changeZoom(parent->_object3._percent - 5);
			_globals->_sceneObjects->draw();
			_globals->_events.delay(1);
		}
	}
}

/*--------------------------------------------------------------------------*/

void Scene1000::postInit(SceneObjectList *OwnerList) {
	Scene::postInit();
	setZoomPercents(0, 100, 200, 100);

	if (_globals->_sceneManager._previousScene == 2000) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(1, 1);
		_object1.setPosition(Common::Point(120, 180));

		setAction(&_action2);

		_globals->_sceneManager._scene->_sceneBounds.centre(_object1._position.x, _object1._position.y);
		_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
		
		_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;
		_globals->_soundHandler.startSound(114);
	} else if (_globals->_sceneManager._previousScene == 2222) {
		setZoomPercents(150, 10, 180, 100);
		_object1.postInit();
		_object1.setVisage(1001);
		_object1._strip = 7;
		_object1.animate(ANIM_MODE_2, 0);
		_object1._moveDiff = Common::Point(2, 2);
		_object1.setPosition(Common::Point(120, 180));
		
		_globals->_sceneManager._scene->_sceneBounds.centre(_object1._position.x, _object1._position.y);
		_globals->_sceneManager._scene->_sceneBounds.contain(_globals->_sceneManager._scene->_backgroundBounds);
		_globals->_sceneOffset.x = (_globals->_sceneManager._scene->_sceneBounds.left / 160) * 160;

		setAction(&_action1);
	} else {
		_globals->_soundHandler.startSound(4);
		setZoomPercents(0, 10, 30, 100);
		_object3.postInit();
		_object3.setVisage(1050);
		_object3.changeZoom(-1);
		_object3.setPosition(Common::Point(158, 0));
		
		_globals->_player.postInit();
		_globals->_player.setVisage(1050);
		_globals->_player.setStrip(3);
		_globals->_player.setPosition(Common::Point(160, 191));
		_globals->_player._moveDiff.x = 12;
		_globals->_player.flag100();
		_globals->_player.disableControl();

		_globals->_sceneManager._scene->_sceneBounds.centre(_object3._position.x, _object3._position.y);

		setAction(&_action3);
	}

	loadScene(1000);
}

/*--------------------------------------------------------------------------*/


} // End of namespace tSage
