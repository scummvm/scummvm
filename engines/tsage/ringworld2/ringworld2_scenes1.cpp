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

#include "graphics/cursorman.h"

#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes1.h"

namespace TsAGE {

namespace Ringworld2 {

/*--------------------------------------------------------------------------
 * Scene 1000 - Cutscene scene
 *
 *--------------------------------------------------------------------------*/

Scene1000::Scene1000(): SceneExt() {
	R2_GLOBALS._sceneManager._hasPalette = false;
	R2_GLOBALS._uiElements._active = false;
	_gameTextSpeaker._displayMode = 9;
	_forceCheckAnimationFl = false;
	_animCounter = 0;
}

void Scene1000::postInit(SceneObjectList *OwnerList) {
	loadBlankScene();
	SceneExt::postInit();

	_stripManager.addSpeaker(&_gameTextSpeaker);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 300:
		_sceneMode = R2_GLOBALS.getFlag(57) ? 40 : 0;
		break;
	case 1010:
		_sceneMode = 30;
		break;
	case 1100:
		_sceneMode = 10;
		break;
	case 1530:
		_sceneMode = 20;
		break;
	case 2500:
		_sceneMode = 100;
		break;
	case 2800:
		_sceneMode = 70;
		break;
	case 3100:
		if (R2_GLOBALS._player._oldCharacterScene[R2_QUINN] == 1000)
			_sceneMode = 90;
		else
			_sceneMode = 80;
		break;
	case 3500:
		_sceneMode = 50;
		break;
	case 3700:
		_sceneMode = 60;
		break;
	default:
		_sceneMode = 999;
		break;
	}

	R2_GLOBALS._uiElements._active = false;
	setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
}

void Scene1000::remove() {
	R2_GLOBALS._scenePalette.loadPalette(0);
	R2_GLOBALS._scenePalette.setEntry(255, 0xff, 0xff, 0xff);
	SceneExt::remove();
}

void Scene1000::signal() {
	ScenePalette scenePalette1, scenePalette2;
	uint32 black = 0;

	switch (_sceneMode++) {
	case 0:
		// TODO: Determine correct colors
		R2_GLOBALS._gfxColors.foreground = 191;
		R2_GLOBALS._gfxColors.background = 144;
		R2_GLOBALS._fontColors.background = 224;
		R2_GLOBALS._fontColors.foreground = 119;

		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer.load(5, this);
		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;

		_animationPlayer.dispatch();
		_forceCheckAnimationFl = true;

		R2_GLOBALS._scenePalette.fade((const byte *)&black, true, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound1.play(67);
		break;

	case 1:
		R2_GLOBALS._sound1.fadeOut2(NULL);

		// TODO: Determine correct colors
		R2_GLOBALS._gfxColors.foreground = 191;
		R2_GLOBALS._gfxColors.background = 144;
		R2_GLOBALS._fontColors.background = 224;
		R2_GLOBALS._fontColors.foreground = 119;

		R2_GLOBALS._scenePalette.loadPalette(0);
		loadScene(9999);

		R2_GLOBALS._player.setup(1140, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(160, 100));
		R2_GLOBALS._player.show();

		_animCounter = 0;
		_stripManager.start(29, this);
		break;

	case 2:
		if (R2_GLOBALS._speechSubtitles & SPEECH_TEXT)
			setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
		else {
			if (++_animCounter < 3)
				_sceneMode = 2;

			setAction(&_sequenceManager1, this, 2, &R2_GLOBALS._player, NULL);
		}
		break;

	case 3:
		// TODO: Determine correct colors
		R2_GLOBALS._gfxColors.foreground = 191;
		R2_GLOBALS._gfxColors.background = 144;
		R2_GLOBALS._fontColors.background = 224;
		R2_GLOBALS._fontColors.foreground = 119;

		for (int percent = 100; percent >= 0; percent -= 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(7, this);
		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;

		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound2.play(81);
		R2_GLOBALS._sound1.play(80);
		break;

	case 4:
		// TODO: Determine correct colors
		R2_GLOBALS._gfxColors.foreground = 191;
		R2_GLOBALS._gfxColors.background = 144;
		R2_GLOBALS._fontColors.background = 224;
		R2_GLOBALS._fontColors.foreground = 119;

		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sceneManager.changeScene(1100);
		break;

	case 10:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(6, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound1.play(55);
		break;

	case 11:
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;

	case 20:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(8, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 21:
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(1530);
		break;

	case 30:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(17, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound2.play(91);
		break;

	case 31:
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS.setFlag(51);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;

	case 40:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(18, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound2.play(90);
		break;

	case 41:
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(1010);
		break;

	case 50:
		R2_GLOBALS._sound2.play(306);
		for (int percent = 100; percent >= 0; percent -= 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(13, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 51:
		R2_GLOBALS._sound2.stop();
		R2_GLOBALS._sound2.play(307);
		R2_GLOBALS._sound1.play(308);

		for (int percent = 100; percent >= 0; percent -= 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(14, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 52:
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(3350);
		break;

	case 60:
		R2_GLOBALS._sound1.play(333);

		for (int percent = 100; percent >= 0; percent -= 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(12, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 61:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(160);
		break;

	case 70:
		R2_GLOBALS._sound2.play(113);
		for (int percent = 100; percent >= 0; percent -= 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(9, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 71:
	case 81:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(3100);
		break;

	case 80:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(10, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound1.play(242);
		R2_GLOBALS._sound2.play(286);
		break;

	case 90:
		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(11, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);

		R2_GLOBALS._sound1.play(277);
		break;

	case 91:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._player._characterIndex = R2_SEEKER;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 3100;
		R2_GLOBALS._sceneManager.changeScene(2500);
		break;

	case 100:
		R2_GLOBALS._sound1.play(304);
		R2_GLOBALS._sound2.play(82);

		_animationPlayer._paletteMode = ANIMPALMODE_NONE;
		_animationPlayer._objectMode = ANIMOBJMODE_2;
		_animationPlayer.load(19, this);

		R2_GLOBALS._scenePalette.loadPalette(_animationPlayer._subData._palData, 0, 256);
		R2_GLOBALS._sceneManager._hasPalette = false;
		_animationPlayer.dispatch();

		_forceCheckAnimationFl = true;
		R2_GLOBALS._scenePalette.fade((const byte *)&black, 1, 0);
		for (int percent = 0; percent < 100; percent += 5)
			R2_GLOBALS._scenePalette.fade((const byte *)&black, true, percent);
		break;

	case 101:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_GLOBALS._sound2.fadeOut2(NULL);
		R2_GLOBALS._scenePalette.loadPalette(0);
		R2_GLOBALS._sceneManager.changeScene(3500);
		break;
	}
}

void Scene1000::dispatch() {
	if (_forceCheckAnimationFl) {
		if (_animationPlayer.isCompleted()) {
			_forceCheckAnimationFl = false;
			_animationPlayer.close();
			_animationPlayer.remove();

			if (_sceneMode == 52)
				_animationPlayer._endAction = this;
		} else
			_animationPlayer.dispatch();
	}

	Scene::dispatch();
}


/*--------------------------------------------------------------------------
 * Scene 1010 - Cutscene: A pixel lost in space!
 *
 *--------------------------------------------------------------------------*/

void Scene1010::postInit(SceneObjectList *OwnerList) {
	loadScene(1010);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	setZoomPercents(100, 1, 160, 100);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setObjectWrapper(NULL);
	R2_GLOBALS._player.setPosition(Common::Point(30, 264));
	R2_GLOBALS._player.changeZoom(-1);
	R2_GLOBALS._player.disableControl();

	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);

	if (R2_GLOBALS.getFlag(57))
		_sceneMode = 1;
	else {
		R2_GLOBALS._sound1.play(89);
		_sceneMode = 0;
	}
}

void Scene1010::signal() {
	switch (_sceneMode) {
	case 1: {
		_sceneMode = 2;
		R2_GLOBALS._player.setup(1010, 2, 1);
		R2_GLOBALS._player.setPosition(Common::Point(297, 101));
		Common::Point pt(30, 264);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2:
		_sceneMode = 3;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		R2_GLOBALS._player.hide();
		break;
	case 3:
		if (R2_GLOBALS.getFlag(57))
			R2_GLOBALS._sceneManager.changeScene(1500);
		else
			R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	default: {
		_sceneMode = 2;
		R2_GLOBALS._player.setup(1010, 1, 1);
		Common::Point pt(297, 101);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1020 - Cutscene - trip in space 2
 *
 *--------------------------------------------------------------------------*/

void Scene1020::postInit(SceneObjectList *OwnerList) {
	loadScene(1020);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene == 1010)
		_sceneBounds = Rect(160, 0, SCREEN_WIDTH + 160, 200);

	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;
	R2_GLOBALS._player.postInit();

	if (R2_GLOBALS._sceneManager._previousScene == 1010) {
		R2_GLOBALS._player.setPosition(Common::Point(500, 100));
		R2_GLOBALS._player.setup(1020, 1, 1);
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(0, 100));
		R2_GLOBALS._player.setup(1020, 2, 1);
	}

	R2_GLOBALS._player.setObjectWrapper(NULL);
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();
	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);

	if (R2_GLOBALS._sceneManager._previousScene == 1010)
		_sceneMode = 0;
	else
		_sceneMode = 10;
}

void Scene1020::signal() {
	switch (_sceneMode) {
	case 0: {
		_sceneMode = 1;
		R2_GLOBALS._player.show();
		R2_GLOBALS._player.setPosition(Common::Point(347, 48));
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
		R2_GLOBALS._player.setZoom(0);
		Common::Point pt(392, 41);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 1:
		_sceneMode = 2;
		R2_GLOBALS._player.setZoom(100);
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		_sceneMode = 3;
		R2_GLOBALS._player._moveDiff = Common::Point(30, 15);
		Common::Point pt(-15, 149);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		_sceneMode = 4;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 4:
		R2_GLOBALS.setFlag(51);
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 10: {
		_sceneMode = 11;
		R2_GLOBALS._player.setPosition(Common::Point(25, 133));
		R2_GLOBALS._player._moveDiff = Common::Point(30, 15);
		R2_GLOBALS._player.setZoom(100);
		Common::Point pt(355, 60);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 11:
		R2_GLOBALS._player.setPosition(Common::Point(355, 57));
		_sceneMode = 12;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 12: {
		R2_GLOBALS._player.setPosition(Common::Point(355, 60));
		_sceneMode = 13;
		R2_GLOBALS._player._moveDiff = Common::Point(3, 1);
		Common::Point pt(347, 48);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 13:
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		if (R2_GLOBALS._player._percent < 1)
			_sceneMode = 14;
		break;
	case 14:
		R2_GLOBALS._sceneManager.changeScene(1010);
		break;
	default:
		break;
	}
}

void Scene1020::dispatch() {
	if (_sceneMode == 1) {
		R2_GLOBALS._player.setZoom(R2_GLOBALS._player._percent + 1);
		if (R2_GLOBALS._player._percent > 10)
			R2_GLOBALS._player._moveDiff.x = 3;
		if (R2_GLOBALS._player._percent > 20)
			R2_GLOBALS._player._moveDiff.x = 4;
	}

	if ((_sceneMode == 13) && (R2_GLOBALS._player._percent != 0)) {
		R2_GLOBALS._player.setZoom(R2_GLOBALS._player._percent - 2);
		if (R2_GLOBALS._player._percent < 80)
			R2_GLOBALS._player._moveDiff.x = 2;
		if (R2_GLOBALS._player._percent < 70)
			R2_GLOBALS._player._moveDiff.x = 1;
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1100 - Canyon
 *
 *--------------------------------------------------------------------------*/

Scene1100::Scene1100() {
	_nextStripNum = 0;
	_paletteRefreshStatus = 0;
}

void Scene1100::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_nextStripNum);
	s.syncAsSint16LE(_paletteRefreshStatus);
}

bool Scene1100::Seeker::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS.getFlag(52)) {
		// The trouper is dead
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->_nextStripNum = 327;
		else
			scene->_nextStripNum = 328;
		scene->_sceneMode = 53;
		scene->setAction(&scene->_sequenceManager1, scene, 1122, &R2_GLOBALS._player, NULL);
	} else {
		// The trouper is not dead
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 55;
		if (R2_GLOBALS._stripModifier >= 3) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_stripManager.start3(329, scene, R2_GLOBALS._stripManager_lookupList);
			else
				scene->_stripManager.start3(330, scene, R2_GLOBALS._stripManager_lookupList);
		} else {
			++R2_GLOBALS._stripModifier;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_stripManager.start3(304, scene, R2_GLOBALS._stripManager_lookupList);
			else
				scene->_stripManager.start3(308, scene, R2_GLOBALS._stripManager_lookupList);
		}
	}
	return true;
}

bool Scene1100::Trooper::startAction(CursorType action, Event &event) {
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_NEGATOR_GUN:
		if (_visage == 1105) {
			// Trooper wears the stasis shield
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1114;
			scene->setAction(&scene->_sequenceManager1, scene, 1114, &R2_GLOBALS._player, &scene->_trooper, NULL);
			return true;
		} else
			return SceneActor::startAction(action, event);
		break;
	case R2_SONIC_STUNNER:
	// No break on purpose
	case R2_PHOTON_STUNNER:
		if (_visage == 1105) {
			// If trooper wears the stasis shield
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				scene->_sceneMode = 1112;
				scene->setAction(&scene->_sequenceManager1, scene, 1112, &R2_GLOBALS._player, &scene->_trooper, NULL);
			} else {
				scene->_sceneMode = 1115;
				scene->setAction(&scene->_sequenceManager1, scene, 1115, &R2_GLOBALS._player, &scene->_trooper, NULL);
			}
			return true;
		} else if (_strip == 2) {
			// Trooper wears his black uniform
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1113;
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->setAction(&scene->_sequenceManager1, scene, 1113, &R2_GLOBALS._player, &scene->_trooper, NULL);
			else
				scene->setAction(&scene->_sequenceManager1, scene, 1118, &R2_GLOBALS._player, &scene->_trooper, NULL);
			return true;
		} else
			return SceneActor::startAction(action, event);
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

bool Scene1100::Chief::startAction(CursorType action, Event &event) {
	// CHECKME: Flag 54 is never set. Guess: the flag means "Chief is dead"
	if ((action == CURSOR_TALK) && (!R2_GLOBALS.getFlag(54)) && (R2_GLOBALS.getFlag(52))) {
		// Talk to chief after the trooper dies
		Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

		scene->_nextStripNum = 0;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 53;
		scene->setAction(&scene->_sequenceManager1, scene, 1122, &R2_GLOBALS._player, NULL);
		return true;
	}

	return SceneActor::startAction(action, event);
}

void Scene1100::postInit(SceneObjectList *OwnerList) {
	if ((R2_GLOBALS._sceneManager._previousScene == 300) || (R2_GLOBALS._sceneManager._previousScene == 1100))
		loadScene(1150);
	else
		loadScene(1100);

	if ((R2_GLOBALS._sceneManager._previousScene == 1000) && (!R2_GLOBALS.getFlag(44)))
		R2_GLOBALS._uiElements._active = false;

	if (R2_GLOBALS._player._characterScene[R2_QUINN] == 1100)
		R2_GLOBALS._sceneManager._previousScene = 1100;

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._uiElements._active = false;

	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1000;

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_seekerSpeaker);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_chiefSpeaker);

	scalePalette(65, 65, 65);

	_cloud.postInit();
	_cloud.setup(1100, 1, 1);
	_cloud.fixPriority(10);

	R2_GLOBALS._scrollFollower = NULL;

	_fuana1.setDetails(Rect(56, 47, 68, 83), 1100, 7, -1, -1, 1, NULL);
	_fauna2.setDetails(Rect(167, 132, 183, 167), 1100, 7, -1, -1, 1, NULL);
	_bouldersBlockingCave.setDetails(Rect(26, 112, 87, 145), 1100, 13, -1, -1, 1, NULL);
	_trail.setDetails(Rect(4, 70, 79, 167), 1100, 16, -1, -1, 1, NULL);

	R2_GLOBALS._sound1.stop();

	if (R2_GLOBALS._sceneManager._previousScene == 300) {
		if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
			R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player._characterScene[R2_QUINN] = 1100;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 1100;
		_cloud.setPosition(Common::Point(150, 30));
		R2_GLOBALS._sound1.play(93);
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.hide();
		R2_GLOBALS._player.disableControl();

		_seeker.postInit();
		_seeker.hide();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_seeker.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		else
			_seeker.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);

		_chief.postInit();
		_chief.setup(1113, 3, 1);
		_chief.setPosition(Common::Point(181, 125));
		_chief.fixPriority(110);

		if (R2_GLOBALS.getFlag(54))
			_chief.setDetails(1100, 4, -1, -1, 1, (SceneItem *) NULL);
		else
			_chief.setDetails(1100, 3, -1, -1, 1, (SceneItem *) NULL);

		_trooper.postInit();
		// Trooper wears his stasis shield
		_trooper.setup(1105, 3, 1);
		_trooper.setPosition(Common::Point(312, 165));
		_trooper._numFrames = 5;
		_trooper.setDetails(1100, 22, 23, 24, 1, (SceneItem *) NULL);

		_ship.postInit();
		_ship.setup(1512, 1, 1);
		_ship.setPosition(Common::Point(187, -25));
		_ship.fixPriority(48);
		_ship._moveDiff.y = 1;
		_ship.setDetails(1100, 37, -1, -1, 1, (SceneItem *) NULL);

		_sceneMode = 20;

		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
	} else if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		_cloud.setPosition(Common::Point(50, 30));
		_paletteRefreshStatus = 0;
		_palette1.loadPalette(1101);
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player._effect = EFFECT_SHADOW_MAP;
		R2_GLOBALS._player._shadowMap = _shadowPaletteMap;
		R2_GLOBALS._player.setup(1102, 3, 2);
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setPosition(Common::Point(111,-20));
		R2_GLOBALS._player.fixPriority(150);
		R2_GLOBALS._player._moveRate = 30;
		R2_GLOBALS._player._moveDiff = Common::Point(16, 2);

		_rightLandslide.setup2(1104, 2, 1, 175, 125, 102, EFFECT_SHADED);
		_purplePlant.setup2(1102, 5, 1, 216, 167, 1, EFFECT_NONE);

		_leftImpacts.postInit();
		_leftImpacts.setup(1113, 2, 1);
		_leftImpacts.setPosition(Common::Point(67, 151));
		_leftImpacts.fixPriority(255);

		_shipFormation.postInit();
		_shipFormation.setup(1102, 6, 1);
		_shipFormation._moveRate = 30;
		_shipFormation._moveDiff.x = 2;

		_shipFormationShadow.postInit();
		_shipFormationShadow.setup(1102, 6, 2);
		_shipFormationShadow._moveRate = 30;
		_shipFormationShadow._moveDiff.x = 2;
		_shipFormationShadow._effect = EFFECT_SHADOW_MAP;
		_shipFormationShadow._shadowMap = _shadowPaletteMap;

		R2_GLOBALS._sound1.play(86);

		_sceneMode = 0;

		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
	} else {
		_cloud.setPosition(Common::Point(180, 30));
		if (R2_GLOBALS.getFlag(52))
			// Trooper is dead
			R2_GLOBALS._sound1.play(98);
		else
			// Trooper is alive
			R2_GLOBALS._sound1.play(95);

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);

		_seeker.postInit();

		if (R2_GLOBALS.getFlag(52)) {
			// Trooper is dead
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				R2_GLOBALS._player.setup(19, 7, 1);
				_seeker.setup(29, 6, 1);
			} else {
				R2_GLOBALS._player.setup(29, 7, 1);
				_seeker.setup(19, 6, 1);
			}
			R2_GLOBALS._player.setPosition(Common::Point(140, 124));
			_seeker.setPosition(Common::Point(237, 134));
			R2_GLOBALS._player.enableControl();
		} else {
			// Trooper is alive
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				R2_GLOBALS._player.setup(1107, 2, 1);
				_seeker.setup(1107, 4, 1);
				R2_GLOBALS._player.setPosition(Common::Point(247, 169));
				_seeker.setPosition(Common::Point(213, 169));
			} else {
				R2_GLOBALS._player.setup(1107, 4, 1);
				_seeker.setup(1107, 2, 1);
				R2_GLOBALS._player.setPosition(Common::Point(213, 169));
				_seeker.setPosition(Common::Point(247, 169));
			}
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
		}

		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_seeker.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		else
			_seeker.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);

		_chief.postInit();
		_chief.setup(1113, 3, 1);
		_chief.setPosition(Common::Point(181, 125));
		_chief.fixPriority(110);

		if (R2_GLOBALS.getFlag(54))
			_chief.setDetails(1100, 4, -1, -1, 1, (SceneItem *) NULL);
		else
			_chief.setDetails(1100, 3, -1, -1, 1, (SceneItem *) NULL);

		if (!R2_GLOBALS.getFlag(52)) {
			// If trooper is alive, initialize him
			_trooper.postInit();
			if (R2_GLOBALS.getFlag(53))
				// Trooper wears his black uniform
				_trooper.setup(1106, 2, 4);
			else
				// Trooper wears a stasis shield
				_trooper.setup(1105, 4, 4);

			_trooper.setPosition(Common::Point(17, 54));
			_trooper._numFrames = 5;

			if (R2_GLOBALS.getFlag(53))
				// Trooper isn't wearing the stasis shield
				_trooper.setDetails(1100, 28, -1, -1, 1, (SceneItem *) NULL);
			else
				_trooper.setDetails(1100, 22, 23, 24, 1, (SceneItem *) NULL);

			_trooper.fixPriority(200);
		}
		_ship.postInit();
		_ship.setup(1512, 1, 1);
		_ship.setPosition(Common::Point(187, 45));
		_ship.fixPriority(48);
		_ship._moveDiff.y = 1;
		_ship.setDetails(1100, 37, -1, -1, 1, (SceneItem *) NULL);
	}

	_boulders.setDetails(Rect(123, 69, 222, 105), 1100, 13, -1, -1, 1, NULL);
	_sky.setDetails(Rect(0, 0, 480, 46), 1100, 0, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, 480, 200), 1100, 40, 41, 42, 1, NULL);
}

void Scene1100::remove() {
	R2_GLOBALS._scrollFollower = &R2_GLOBALS._player;
	if (_sceneMode > 20)
		R2_GLOBALS._sound1.fadeOut2(NULL);
	g_globals->gfxManager()._bounds.moveTo(Common::Point(0, 0));
	R2_GLOBALS._uiElements._active = true;
	SceneExt::remove();
}

void Scene1100::signal() {
	switch (_sceneMode++) {
	case 0:
		_shipFormation.setPosition(Common::Point(350, 20));
		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 1:{
		Common::Point pt(-150, 20);
		NpcMover *mover = new NpcMover();
		_shipFormation.addMover(mover, &pt, this);
		_shipFormationShadow.setPosition(Common::Point(350, 55));

		Common::Point pt2(-150, 55);
		NpcMover *mover2 = new NpcMover();
		_shipFormationShadow.addMover(mover2, &pt2, NULL);
		}
		break;
	case 2:
		_shipFormation.remove();
		_shipFormationShadow.remove();
		_shotImpact1.postInit();
		_shotImpact2.postInit();
		_shotImpact3.postInit();
		_shotImpact4.postInit();
		_shotImpact5.postInit();
		_laserShot.postInit();
		setAction(&_sequenceManager1, this, 1102, &_shotImpact1, &_shotImpact2, &_shotImpact3, &_shotImpact4, &_shotImpact5, &_laserShot, NULL);
		break;
	case 3: {
		R2_GLOBALS._sound2.play(84);
		R2_GLOBALS._player.setPosition(Common::Point(-50, 126));
		Common::Point pt(350, 226);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 4:
		_chief.postInit();
		_chief.show();
		setAction(&_sequenceManager1, this, 1101, &_chief, &_laserShot, NULL);
		break;
	case 5:
		_runningGuy1.postInit();
		_runningGuy1._effect = EFFECT_SHADED2;
		_runningGuy1.setup(1103, 3, 1);
		_runningGuy1._moveRate = 30;

		_runningGuy2.postInit();
		_runningGuy2._effect = EFFECT_SHADED2;
		_runningGuy2.setup(1103, 4, 1);
		_runningGuy2._moveRate = 25;

		_runningGuy1.setAction(&_sequenceManager2, this, 1109, &_runningGuy1, &_runningGuy2, NULL);
		break;
	case 6: {
		_runningGuy1.remove();
		_runningGuy2.remove();
		R2_GLOBALS._player.setPosition(Common::Point(-50, 136));
		R2_GLOBALS._sound2.play(84);
		Common::Point pt(350, 236);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 7:
		setAction(&_sequenceManager1, this, 1103, &_chief, &_laserShot, NULL);
		break;
	case 8:
		R2_GLOBALS._player._effect = EFFECT_NONE;
		_animation.postInit();
		setAction(&_sequenceManager1, this, 1105, &R2_GLOBALS._player, &_laserShot, &_animation, &_chief, NULL);
		break;
	case 9:
		_rightLandslide.copySceneToBackground();

		_runningGuy3.postInit();
		_runningGuy3.setup(1103, 2, 1);
		_runningGuy3._moveRate = 30;
		_runningGuy3.setAction(&_sequenceManager3, this, 1107, &_runningGuy3, NULL);
		break;
	case 10:
		_runningGuy1.postInit();
		_runningGuy1.setup(1103, 1, 1);
		_runningGuy1._moveRate = 15;
		_runningGuy1.setAction(&_sequenceManager2, this, 1108, &_runningGuy1, NULL);
		break;
	case 11: {
		setAction(&_sequenceManager1, this, 1106, &_animation, &_laserShot, &_leftImpacts, NULL);

		R2_GLOBALS._player._effect = EFFECT_SHADOW_MAP;
		R2_GLOBALS._player.setup(1102, 3, 2);
		R2_GLOBALS._player.setPosition(Common::Point(-50, 131));
		R2_GLOBALS._sound2.play(84);
		Common::Point pt(350, 231);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 12:
	// Really nothing
		break;
	case 13:
		_trooper.postInit();
		R2_GLOBALS._scrollFollower = &_trooper;

		_animation.setup(1100, 2, 1);
		_animation.setPosition(Common::Point(408, 121));

		_laserShot.setup(1100, 3, 5);
		_laserShot.setPosition(Common::Point(409, 121));

		setAction(&_sequenceManager1, this, 1104, &_trooper, NULL);
		break;
	case 14:
		setAction(&_sequenceManager1, this, 1100, &_animation, &_laserShot, NULL);
		break;
	case 15:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 20: {
		Common::Point pt(187, -13);
		NpcMover *mover = new NpcMover();
		_ship.addMover(mover, &pt, this);
		}
		break;
	case 21: {
		R2_GLOBALS._sound2.play(92);
		_trooper.animate(ANIM_MODE_5, NULL);
		Common::Point pt(187, 45);
		NpcMover *mover = new NpcMover();
		_ship.addMover(mover, &pt, this);
		}
		break;
	case 22:
		setAction(&_sequenceManager1, this, 1110, &_seeker, &R2_GLOBALS._player, NULL);
		break;
	case 23:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(312, this);
		R2_GLOBALS._player.setAction(&_sequenceManager1, this, 1119, &R2_GLOBALS._player, NULL);
		break;
	case 24:
		if (!_stripManager._endHandler)
			R2_GLOBALS._player.disableControl();
		break;
	case 25:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._stripManager_lookupList[9] = 1;
		R2_GLOBALS._stripManager_lookupList[10] = 1;
		R2_GLOBALS._stripManager_lookupList[11] = 1;
		R2_GLOBALS._sound1.play(95);
		setAction(&_sequenceManager1, this, 1111, &_trooper, &R2_GLOBALS._player, &_seeker, NULL);
		break;
	case 26:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripManager.start(302, this);
		break;
	case 27:
		R2_GLOBALS._player.disableControl();
		setAction(&_sequenceManager1, this, 1120, &_seeker, &R2_GLOBALS._player, NULL);
		break;
	case 28:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripManager.start(303, this);
		break;
	case 29:
	case 50:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 51:
		// Trooper no longer wears a statis shield
		R2_GLOBALS.setFlag(53);
		_trooper.setDetails(1100, 28, -1, -1, 3, (SceneItem *) NULL);

		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 52:
		// Trooper is shot to death
		R2_GLOBALS._sound1.play(98);
		R2_GLOBALS.setFlag(52);
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1116;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			setAction(&_sequenceManager1, this, 1116, &R2_GLOBALS._player, NULL);
			_seeker.setAction(&_sequenceManager2, NULL, 1123, &_seeker, NULL);
		} else {
			setAction(&_sequenceManager1, this, 1124, &R2_GLOBALS._player, NULL);
			_seeker.setAction(&_sequenceManager2, NULL, 1117, &_seeker, NULL);
		}
		break;
	case 53:
		_sceneMode = 54;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (_nextStripNum == 0) {
			R2_GLOBALS.setFlag(55);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_stripManager.start(318, this);
			else
				_stripManager.start(323, this);
		} else
			_stripManager.start3(_nextStripNum, this, _stripManager._lookupList);

		break;
	case 54:
		if (_stripManager._exitMode == 1) {
			R2_GLOBALS._player.disableControl();
			_sceneMode = 1125;
			setAction(&_sequenceManager1, this, 1125, &R2_GLOBALS._player, &_seeker, NULL);
		} else
			R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 55:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 99:
		R2_GLOBALS._player._characterScene[R2_QUINN] = 300;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 300;
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 1112:
		_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(313, this, _stripManager._lookupList);
		break;
	case 1113:
		_sceneMode = 52;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS._sound1.play(96);
		_stripManager.start3(316, this, _stripManager._lookupList);
		break;
	case 1114:
		_sceneMode = 51;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(315, this, _stripManager._lookupList);
		break;
	case 1115:
		_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start3(314, this, _stripManager._lookupList);
		break;
	case 1116:
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		R2_GLOBALS._stripManager_lookupList[9] = 1;
		R2_GLOBALS._stripManager_lookupList[10] = 1;
		R2_GLOBALS._stripManager_lookupList[11] = 1;
		break;
	case 1125: {
		_sceneMode = 99;
		R2_GLOBALS._sound2.play(100);
		R2_GLOBALS._sound1.play(101);
		Common::Point pt(187, -13);
		NpcMover *mover = new NpcMover();
		_ship.addMover(mover, &pt, this);
		}
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene1100::dispatch() {
	// WORKAROUND: This fixes a problem with an overhang that gets blasted re-appearing
	if (_animation._frame > 5 && _sceneMode == 13) {
		_animation._endFrame = 9;
		if (_animation._frame == 9)
			// Use one of the scene's background scene objects to copy the scene to the background.
			// This fixes the problem with the cliff overhang still appearing during the cutscene
			_rightLandslide.copySceneToBackground();
	}

	if ((g_globals->_sceneObjects->contains(&_laserShot)) && (_laserShot._visage == 1102) && (_laserShot._strip == 4) && (_laserShot._frame == 1) && (_laserShot._flags & OBJFLAG_HIDING)) {
		if (_paletteRefreshStatus == 1) {
			_paletteRefreshStatus = 2;
			R2_GLOBALS._scenePalette.refresh();
		}
	} else {
		if (_paletteRefreshStatus == 2)
			R2_GLOBALS._scenePalette.refresh();
		_paletteRefreshStatus = 1;
	}

	Scene::dispatch();

	if (R2_GLOBALS._player._bounds.contains(_runningGuy1._position))
		_runningGuy1._shade = 3;
	else
		_runningGuy1._shade = 0;

	if (R2_GLOBALS._player._bounds.contains(_runningGuy2._position))
		_runningGuy2._shade = 3;
	else
		_runningGuy2._shade = 0;

	if (R2_GLOBALS._player._bounds.contains(_runningGuy3._position))
		_runningGuy3._shade = 3;
	else
		_runningGuy3._shade = 0;
}

void Scene1100::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
		R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::saveCharacter(characterIndex);
}

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
		if (_field414 == 0) {
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

/*--------------------------------------------------------------------------
 * Scene 1337 - Card game
 *
 *--------------------------------------------------------------------------*/

Scene1337::Card::Card() {
	_cardId = 0;
	_stationPos = Common::Point(0, 0);
}

void Scene1337::Card::synchronize(Serializer &s) {
	warning("STUBBED: Card::synchronize()");
}

bool Scene1337::Card::isIn(Common::Point pt) {
	if ((_stationPos.x > pt.x) || (_stationPos.x + 24 < pt.x))
		return false;

	if ((_stationPos.y > pt.y) || (_stationPos.y + 24 < pt.y))
		return false;

	return true;
}

Scene1337::GameBoardSide::GameBoardSide() {
	_card1Pos = Common::Point(0, 0);
	_card2Pos = Common::Point(0, 0);
	_card3Pos = Common::Point(0, 0);
	_card4Pos = Common::Point(0, 0);
	_frameNum = 0;
}

void Scene1337::GameBoardSide::synchronize(Serializer &s) {
	warning("STUBBED: GameBoardSide::synchronize()");
}

Scene1337::Scene1337() {
	_autoplay = false;
	_cardsAvailableNumb = 0;
	_currentDiscardIndex = 0;

	for (int i = 0; i < 100; i++)
		_availableCardsPile[i] = 0;

	_shuffleEndedFl = false;
	_currentPlayerNumb = 0;
	_actionIdx1 = 0;
	_actionIdx2 = 0;
	_showPlayerTurn = false;
	_displayHelpFl = false;
	_winnerId = -1;
	_instructionsDisplayedFl = false;
	_instructionsWaitCount = 0;

	_delayedFunction = nullptr;
	_actionCard1 = nullptr;
	_actionCard2 = nullptr;
	_actionCard3 = nullptr;

	_cursorCurRes = 0;
	_cursorCurStrip = 0;
	_cursorCurFrame = 0;
}

void Scene1337::synchronize(Serializer &s) {
	warning("STUBBED: Scene1337::synchronize()");
}

void Scene1337::Action1337::waitFrames(int32 frameCount) {
	uint32 firstFrameNumber = g_globals->_events.getFrameNumber();
	uint32 curFrame = firstFrameNumber;
	uint32 destFrame = firstFrameNumber + frameCount;

	while ((curFrame < destFrame) && !g_vm->shouldQuit()) {
		TsAGE::Event event;
		g_globals->_events.getEvent(event);
		curFrame = g_globals->_events.getFrameNumber();
	}

	// CHECKME: The original is calling _eventManager.waitEvent();
}

/**
 * Display instructions
 */
void Scene1337::Action1::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 1: {
		scene->actionDisplay(1331, 6, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1331, 7, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 8, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[1]._outpostStation[0]._cardId = 2;
		scene->_gameBoardSide[1]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[1]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[1]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[1]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[1]._outpostStation[0]._cardId);
		scene->_gameBoardSide[1]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[1]._outpostStation[0]);

		scene->_gameBoardSide[1]._outpostStation[1]._cardId = 3;
		scene->_gameBoardSide[1]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[1]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[1]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[1]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[1]._outpostStation[1]._cardId);
		scene->_gameBoardSide[1]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[1]._outpostStation[1]);

		scene->_gameBoardSide[2]._outpostStation[0]._cardId = 4;
		scene->_gameBoardSide[2]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[0]._cardId);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[0]);

		scene->_gameBoardSide[3]._outpostStation[0]._cardId = 5;
		scene->_gameBoardSide[3]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[0]._cardId);
		scene->_gameBoardSide[3]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[0]);

		scene->_gameBoardSide[3]._outpostStation[1]._cardId = 6;
		scene->_gameBoardSide[3]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[1]._cardId);
		scene->_gameBoardSide[3]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[1]);

		scene->_gameBoardSide[3]._outpostStation[2]._cardId = 7;
		scene->_gameBoardSide[3]._outpostStation[2]._card.postInit();
		scene->_gameBoardSide[3]._outpostStation[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setPosition(scene->_gameBoardSide[3]._outpostStation[2]._stationPos, 0);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setStrip(2);
		scene->_gameBoardSide[3]._outpostStation[2]._card.setFrame(scene->_gameBoardSide[3]._outpostStation[2]._cardId);
		scene->_gameBoardSide[3]._outpostStation[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[3]._outpostStation[2]);

		scene->_gameBoardSide[0]._outpostStation[0]._cardId = 8;
		scene->_gameBoardSide[0]._outpostStation[0]._card.postInit();
		scene->_gameBoardSide[0]._outpostStation[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setPosition(scene->_gameBoardSide[0]._outpostStation[0]._stationPos, 0);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setStrip(2);
		scene->_gameBoardSide[0]._outpostStation[0]._card.setFrame(scene->_gameBoardSide[0]._outpostStation[0]._cardId);
		scene->_gameBoardSide[0]._outpostStation[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[0]._outpostStation[0]);

		scene->_gameBoardSide[0]._outpostStation[1]._cardId = 9;
		scene->_gameBoardSide[0]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[0]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[0]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[0]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[0]._outpostStation[1]._cardId);
		scene->_gameBoardSide[0]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[0]._outpostStation[1]);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 9, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = 2;
		scene->_gameBoardSide[2]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[1]._cardId);
		scene->_gameBoardSide[2]._outpostStation[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);

		scene->_gameBoardSide[2]._outpostStation[2]._cardId = 3;
		scene->_gameBoardSide[2]._outpostStation[2]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[2]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[2]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[2]._cardId);
		scene->_gameBoardSide[2]._outpostStation[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[2]);

		scene->_gameBoardSide[2]._outpostStation[3]._cardId = 5;
		scene->_gameBoardSide[2]._outpostStation[3]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[3]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[3]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[3]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[3]._cardId);
		scene->_gameBoardSide[2]._outpostStation[3]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[3]);

		scene->_gameBoardSide[2]._outpostStation[4]._cardId = 6;
		scene->_gameBoardSide[2]._outpostStation[4]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[4]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[4]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[4]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[4]._cardId);
		scene->_gameBoardSide[2]._outpostStation[4]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[4]);

		scene->_gameBoardSide[2]._outpostStation[5]._cardId = 7;
		scene->_gameBoardSide[2]._outpostStation[5]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[5]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[5]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[5]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[5]._cardId);
		scene->_gameBoardSide[2]._outpostStation[5]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[5]);

		scene->_gameBoardSide[2]._outpostStation[6]._cardId = 8;
		scene->_gameBoardSide[2]._outpostStation[6]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[6]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[6]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[6]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[6]._cardId);
		scene->_gameBoardSide[2]._outpostStation[6]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[6]);

		scene->_gameBoardSide[2]._outpostStation[7]._cardId = 9;
		scene->_gameBoardSide[2]._outpostStation[7]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[7]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[7]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setStrip(2);
		scene->_gameBoardSide[2]._outpostStation[7]._card.setFrame(scene->_gameBoardSide[2]._outpostStation[7]._cardId);
		scene->_gameBoardSide[2]._outpostStation[7]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[7]);

		scene->_aSound1.play(62);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(120);
		scene->_gameBoardSide[2]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[1]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[2]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[3]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[4]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[5]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[6]._card.remove();
		scene->_gameBoardSide[2]._outpostStation[7]._card.remove();

		scene->_gameBoardSide[1]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[1]._outpostStation[1]._card.remove();

		scene->_gameBoardSide[3]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[3]._outpostStation[1]._card.remove();
		scene->_gameBoardSide[3]._outpostStation[2]._card.remove();

		scene->_gameBoardSide[0]._outpostStation[0]._card.remove();
		scene->_gameBoardSide[0]._outpostStation[1]._card.remove();

		scene->_stockPile.setup(1332, 5, 1);
		scene->_stockPile.setPosition(Common::Point(165, 95));
		scene->_stockPile.setPriority(110);
		scene->_stockPile._effect = EFFECT_SHADED;
		scene->_stockPile.show();

		scene->_gameBoardSide[1]._handCard[0]._card.postInit();
		scene->_gameBoardSide[1]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[0]._card.setPosition(scene->_gameBoardSide[1]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[0]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[1]._handCard[1]._card.postInit();
		scene->_gameBoardSide[1]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[1]._card.setPosition(scene->_gameBoardSide[1]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[1]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[1]._handCard[2]._card.postInit();
		scene->_gameBoardSide[1]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[2]._card.setPosition(scene->_gameBoardSide[1]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[2]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[2]._card.fixPriority(170);

		scene->_gameBoardSide[2]._handCard[0]._cardId = 30;
		scene->_gameBoardSide[2]._handCard[0]._card.postInit();
		scene->_gameBoardSide[2]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[0]._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[0]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[0]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[0]);

		scene->_gameBoardSide[2]._handCard[1]._cardId = 16;
		scene->_gameBoardSide[2]._handCard[1]._card.postInit();
		scene->_gameBoardSide[2]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[1]._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[1]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[1]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[1]);

		scene->_gameBoardSide[2]._handCard[2]._cardId = 1;
		scene->_gameBoardSide[2]._handCard[2]._card.postInit();
		scene->_gameBoardSide[2]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[2]._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[2]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[2]._card.fixPriority(170);
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[2]);

		scene->_gameBoardSide[3]._handCard[0]._card.postInit();
		scene->_gameBoardSide[3]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[0]._card.setPosition(scene->_gameBoardSide[3]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[0]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[3]._handCard[1]._card.postInit();
		scene->_gameBoardSide[3]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[1]._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[1]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[3]._handCard[2]._card.postInit();
		scene->_gameBoardSide[3]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[2]._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[2]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[2]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[0]._card.postInit();
		scene->_gameBoardSide[0]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[0]._card.setPosition(scene->_gameBoardSide[0]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[0]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[0]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[1]._card.postInit();
		scene->_gameBoardSide[0]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[1]._card.setPosition(scene->_gameBoardSide[0]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[1]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[1]._card.fixPriority(170);

		scene->_gameBoardSide[0]._handCard[2]._card.postInit();
		scene->_gameBoardSide[0]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[2]._card.setPosition(scene->_gameBoardSide[0]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[0]._handCard[2]._card.setFrame(2);
		scene->_gameBoardSide[0]._handCard[2]._card.fixPriority(170);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 10, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);
		scene->_animatedCard._card.show();
		scene->_aSound2.play(61);

		Common::Point pt(91, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);
		}
		break;
	case 2: {
		scene->_gameBoardSide[2]._handCard[3]._cardId = 2;
		scene->_gameBoardSide[2]._handCard[3]._card.postInit();
		scene->_gameBoardSide[2]._handCard[3]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[3]._card.setPosition(scene->_gameBoardSide[2]._handCard[3]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[3]._card.setStrip(1);
		scene->_gameBoardSide[2]._handCard[3]._card.setFrame(2);
		scene->_gameBoardSide[2]._handCard[3]._card.fixPriority(170);

		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._handCard[3]);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 12, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = 1;
		scene->_gameBoardSide[2]._outpostStation[1]._card.postInit();
		scene->_gameBoardSide[2]._outpostStation[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._outpostStation[1]._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_gameBoardSide[2]._outpostStation[1]._card.hide();

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[2]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[2]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[2]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._outpostStation[1]._stationPos, this);
		}
		break;
	case 3: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);
		scene->_aSound1.play(59);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 13, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._outpostStation[1]._cardId = scene->_gameBoardSide[2]._handCard[3]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[3]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[3]._card._frame);

		scene->_gameBoardSide[2]._handCard[3]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[3]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[3]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._outpostStation[1]._stationPos, this);
		}
		break;
	case 4: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._outpostStation[1]);
		scene->_aSound1.play(59);

		scene->_discardPile._cardId = 1;
		scene->_discardPile._card.hide();

		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._outpostStation[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 5: {
		scene->_animatedCard._card.hide();

		scene->_discardPile._card.postInit();
		scene->_discardPile._card.setVisage(1332);
		scene->_discardPile._card.setPosition(scene->_discardPile._stationPos, 0);
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 14, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_gameBoardSide[2]._delayCard._card.postInit();
		scene->_gameBoardSide[2]._delayCard._card.setVisage(1332);
		scene->_gameBoardSide[2]._delayCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_gameBoardSide[2]._delayCard._card.hide();

		scene->_gameBoardSide[3]._handCard[2]._cardId = 0;
		scene->_gameBoardSide[3]._handCard[2].remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 6: {
		scene->_animatedCard._card.hide();
		scene->_gameBoardSide[2]._delayCard._cardId = 21;
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._delayCard);
		scene->_aSound1.play(57);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(60);
		scene->actionDisplay(1331, 15, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		int tmpVal = 15;
		int i = -1;

		for (i = 0; i <= 7; i++) {
			tmpVal += 29;

			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 90), 0);
			scene->_upperDisplayCard[i].setStrip(3);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 90), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[0].setFrame(1);
		scene->_upperDisplayCard[1].setFrame(3);
		scene->_upperDisplayCard[2].setFrame(6);
		scene->_upperDisplayCard[3].setFrame(8);
		scene->_upperDisplayCard[4].setFrame(9);
		scene->_upperDisplayCard[5].setFrame(10);
		scene->_upperDisplayCard[6].setFrame(11);
		scene->_upperDisplayCard[7].setFrame(12);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();
		scene->_upperDisplayCard[4].remove();
		scene->_upperDisplayCard[5].remove();
		scene->_upperDisplayCard[6].remove();
		scene->_upperDisplayCard[7].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();
		scene->_lowerDisplayCard[4].remove();
		scene->_lowerDisplayCard[5].remove();
		scene->_lowerDisplayCard[6].remove();
		scene->_lowerDisplayCard[7].remove();

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._delayCard._cardId;

		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 7: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();

		scene->_gameBoardSide[2]._delayCard._card.postInit();
		scene->_gameBoardSide[2]._delayCard._card.setVisage(1332);
		scene->_gameBoardSide[2]._delayCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_gameBoardSide[2]._delayCard._card.hide();

		scene->_gameBoardSide[3]._handCard[1]._cardId = 0;
		scene->_gameBoardSide[3]._handCard[1].remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 8: {
		scene->_animatedCard._card.hide();
		scene->_gameBoardSide[2]._delayCard._cardId = 14;
		scene->setAnimationInfo(&scene->_gameBoardSide[2]._delayCard);
		scene->_aSound1.play(57);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 16, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		int tmpVal = 72;
		int i = -1;

		for (i = 0; i <= 3; i++) {
			tmpVal += 29;
			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 71), 0);
			scene->_upperDisplayCard[i].setStrip(3);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 71), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[0].setFrame(2);
		scene->_upperDisplayCard[1].setFrame(5);
		scene->_upperDisplayCard[2].setFrame(7);
		scene->_upperDisplayCard[3].setFrame(15);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);
		scene->actionDisplay(1331, 17, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		tmpVal = 72;
		for (i = 4; i <= 7; i++) {
			tmpVal += 29;

			scene->_upperDisplayCard[i].postInit();
			scene->_upperDisplayCard[i].setVisage(1332);
			scene->_upperDisplayCard[i].setPosition(Common::Point(tmpVal, 100), 0);
			scene->_upperDisplayCard[i].setStrip(4);
			scene->_upperDisplayCard[i].fixPriority(190);

			scene->_lowerDisplayCard[i].postInit();
			scene->_lowerDisplayCard[i].setVisage(1332);
			scene->_lowerDisplayCard[i].setPosition(Common::Point(tmpVal, 100), 0);
			scene->_lowerDisplayCard[i].setStrip(7);
			scene->_lowerDisplayCard[i].setFrame(1);
			scene->_lowerDisplayCard[i].fixPriority(180);
		}

		scene->_upperDisplayCard[4].setFrame(1);
		scene->_upperDisplayCard[5].setFrame(5);
		scene->_upperDisplayCard[6].setFrame(7);
		scene->_upperDisplayCard[7].setFrame(3);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();
		scene->_upperDisplayCard[4].remove();
		scene->_upperDisplayCard[5].remove();
		scene->_upperDisplayCard[6].remove();
		scene->_upperDisplayCard[7].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();
		scene->_lowerDisplayCard[4].remove();
		scene->_lowerDisplayCard[5].remove();
		scene->_lowerDisplayCard[6].remove();
		scene->_lowerDisplayCard[7].remove();

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._handCard[0]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[0]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[0]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[0]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[2]._delayCard._stationPos, this);
		}
		break;
	case 9: {
		scene->_aSound1.play(58);
		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard.remove();
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._delayCard._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 10: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);

		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1331, 18, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		scene->_upperDisplayCard[0].postInit();
		scene->_upperDisplayCard[0].setVisage(1332);
		scene->_upperDisplayCard[0].setPosition(Common::Point(131, 71), 0);
		scene->_upperDisplayCard[0].fixPriority(190);
		scene->_upperDisplayCard[0].setStrip(3);
		scene->_upperDisplayCard[0].setFrame(4);

		scene->_lowerDisplayCard[0].postInit();
		scene->_lowerDisplayCard[0].setVisage(1332);
		scene->_lowerDisplayCard[0].setPosition(Common::Point(131, 71), 0);
		scene->_lowerDisplayCard[0].setStrip(7);
		scene->_lowerDisplayCard[0].setFrame(1);
		scene->_lowerDisplayCard[0].fixPriority(180);

		scene->_upperDisplayCard[1].postInit();
		scene->_upperDisplayCard[1].setVisage(1332);
		scene->_upperDisplayCard[1].setPosition(Common::Point(160, 71), 0);
		scene->_upperDisplayCard[1].fixPriority(190);
		scene->_upperDisplayCard[1].setStrip(3);
		scene->_upperDisplayCard[1].setFrame(16);

		scene->_lowerDisplayCard[1].postInit();
		scene->_lowerDisplayCard[1].setVisage(1332);
		scene->_lowerDisplayCard[1].setPosition(Common::Point(160, 71), 0);
		scene->_lowerDisplayCard[1].setStrip(7);
		scene->_lowerDisplayCard[1].setFrame(1);
		scene->_lowerDisplayCard[1].fixPriority(180);

		scene->_upperDisplayCard[2].postInit();
		scene->_upperDisplayCard[2].setVisage(1332);
		scene->_upperDisplayCard[2].setPosition(Common::Point(131, 100), 0);
		scene->_upperDisplayCard[2].fixPriority(190);
		scene->_upperDisplayCard[2].setStrip(4);
		scene->_upperDisplayCard[2].setFrame(4);

		scene->_lowerDisplayCard[2].postInit();
		scene->_lowerDisplayCard[2].setVisage(1332);
		scene->_lowerDisplayCard[2].setPosition(Common::Point(131, 100), 0);
		scene->_lowerDisplayCard[2].setStrip(7);
		scene->_lowerDisplayCard[2].setFrame(1);
		scene->_lowerDisplayCard[2].fixPriority(180);

		scene->_upperDisplayCard[3].postInit();
		scene->_upperDisplayCard[3].setVisage(1332);
		scene->_upperDisplayCard[3].setPosition(Common::Point(160, 100), 0);
		scene->_upperDisplayCard[3].fixPriority(190);
		scene->_upperDisplayCard[3].setStrip(4);
		scene->_upperDisplayCard[3].setFrame(2);

		scene->_lowerDisplayCard[3].postInit();
		scene->_lowerDisplayCard[3].setVisage(1332);
		scene->_lowerDisplayCard[3].setPosition(Common::Point(160, 100), 0);
		scene->_lowerDisplayCard[3].setStrip(7);
		scene->_lowerDisplayCard[3].setFrame(1);
		scene->_lowerDisplayCard[3].fixPriority(180);

		R2_GLOBALS._sceneObjects->draw();

		waitFrames(240);

		scene->_upperDisplayCard[0].remove();
		scene->_upperDisplayCard[1].remove();
		scene->_upperDisplayCard[2].remove();
		scene->_upperDisplayCard[3].remove();

		scene->_lowerDisplayCard[0].remove();
		scene->_lowerDisplayCard[1].remove();
		scene->_lowerDisplayCard[2].remove();
		scene->_lowerDisplayCard[3].remove();

		scene->_currentPlayerArrow.setFrame(1);
		scene->_currentPlayerArrow.show();
		scene->_currentPlayerArrow.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 19, 159, 10, 1, 220, 0, 7, 0, 154, 154);

		scene->_currentPlayerArrow.hide();

		scene->actionDisplay(1331, 20, 159, 10, 1, 220, 0, 7, 0, 154, 154);
		scene->actionDisplay(1331, 21, 159, 10, 1, 220, 0, 7, 0, 154, 154);

		scene->_discardPile._cardId = scene->_gameBoardSide[2]._handCard[1]._cardId;

		scene->_animatedCard._card.setStrip(scene->_gameBoardSide[2]._handCard[1]._card._strip);
		scene->_animatedCard._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._card._frame);
		scene->_animatedCard._card.animate(ANIM_MODE_NONE, NULL);

		scene->_gameBoardSide[2]._handCard[1]._cardId = 0;
		scene->_gameBoardSide[2]._handCard[1]._card.remove();

		scene->_animatedCard._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_discardPile._stationPos, this);
		}
		break;
	case 11: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._sceneObjects->draw();

		scene->actionDisplay(1331, 22, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		int i = -1;
		for (i = 0; i <= 3; i ++) {
			scene->_gameBoardSide[3]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[3]._handCard[i]._card.remove();

			scene->_gameBoardSide[2]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[2]._handCard[i]._card.remove();

			scene->_gameBoardSide[0]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[0]._handCard[i]._card.remove();

			scene->_gameBoardSide[1]._handCard[i]._cardId = 0;
			scene->_gameBoardSide[1]._handCard[i]._card.remove();
		}

		for (i = 0; i <= 7; i++) {
			scene->_gameBoardSide[3]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[3]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[2]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[2]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[0]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[0]._outpostStation[i]._card.remove();

			scene->_gameBoardSide[1]._outpostStation[i]._cardId = 0;
			scene->_gameBoardSide[1]._outpostStation[i]._card.remove();
		}

		scene->_gameBoardSide[2]._delayCard._cardId = 0;
		scene->_gameBoardSide[2]._delayCard._card.remove();

		scene->_discardPile._cardId = 0;
		scene->_discardPile._card.remove();

		scene->_stockPile.remove();
		}
	// No break on purpose
	case 0:
		R2_GLOBALS._sceneObjects->draw();
		signal();
		break;
	case 12:
		scene->suggestInstructions();
		remove();
		break;
	default:
		break;
	}
}

/**
 * Shuffle cards animation
 */
void Scene1337::Action2::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		scene->_shuffleAnimation._card.postInit();
		scene->_shuffleAnimation._card.setVisage(1332);
		scene->_shuffleAnimation._card.setStrip(8);
		scene->_shuffleAnimation._card.setFrame(1);
		scene->_shuffleAnimation._card.fixPriority(300);
		scene->_shuffleAnimation._card.setPosition(Common::Point(156, 108));

		scene->_discardPile._card.remove();
		scene->_discardPile._cardId = 0;

		scene->_aSound1.play(60);
		scene->_shuffleAnimation._card.animate(ANIM_MODE_5, this);
		break;
	case 1:
		scene->_shuffleAnimation._card.setFrame(1);

		scene->_aSound1.play(60);
		scene->_shuffleAnimation._card.animate(ANIM_MODE_5, this);
		break;
	case 2: {
		Common::Point pt(156, 108);
		NpcMover *mover = new NpcMover();
		scene->_shuffleAnimation._card.addMover(mover, &pt, this);
		}
		break;
	case 3:
		scene->_shuffleAnimation._card.remove();
		scene->_stockPile.setup(1332, 5, 1);
		scene->_stockPile.setPosition(Common::Point(162, 95));
		scene->_stockPile.setPriority(110);
		scene->_stockPile._effect = EFFECT_SHADED;
		scene->_stockPile.show();
		scene->_shuffleEndedFl = true;
		break;
	default:
		break;
	}
}

/**
 * Deal cards
 */
void Scene1337::Action3::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);

	switch (_actionIndex++) {
	case 0: {
		scene->_animatedCard._card._moveDiff = Common::Point(30, 30);
		scene->_animatedCard._card.setVisage(1332);
		scene->_animatedCard._card.setStrip(5);
		scene->_animatedCard._card.setFrame(1);
		scene->_animatedCard._card.fixPriority(400);
		scene->_animatedCard._card.animate(ANIM_MODE_2, NULL);
		scene->_aSound2.play(61);

		Common::Point pt(283, 146);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_animatedCard._card.show();
		scene->_gameBoardSide[1]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 1: {
		scene->_gameBoardSide[1]._handCard[0]._card.postInit();
		scene->_gameBoardSide[1]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[0]._card.setPosition(scene->_gameBoardSide[1]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[0]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(10, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 2: {
		scene->_gameBoardSide[2]._handCard[0]._card.postInit();
		scene->_gameBoardSide[2]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[0]._card.setPosition(scene->_gameBoardSide[2]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[0]._card.fixPriority(170);
		if (scene->_gameBoardSide[2]._handCard[0]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[0]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[0]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[0]._card.setFrame(scene->_gameBoardSide[2]._handCard[0]._cardId);
		}
		scene->_aSound2.play(61);

		Common::Point pt(14, 14);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 3: {
		scene->_gameBoardSide[3]._handCard[0]._card.postInit();
		scene->_gameBoardSide[3]._handCard[0]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[0]._card.setPosition(scene->_gameBoardSide[3]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[0]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[0]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(280, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 4: {
		scene->_gameBoardSide[0]._handCard[0]._card.postInit();
		scene->_gameBoardSide[0]._handCard[0]._card._moveDiff = Common::Point(30,30);
		scene->_gameBoardSide[0]._handCard[0]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[0]._card.setPosition(scene->_gameBoardSide[0]._handCard[0]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[0]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[0]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[0]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(283, 124);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[1]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 5: {
		scene->_gameBoardSide[1]._handCard[1]._card.postInit();
		scene->_gameBoardSide[1]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[1]._card.setPosition(scene->_gameBoardSide[1]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[1]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(37, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 6: {
		scene->_gameBoardSide[2]._handCard[1]._card.postInit();
		scene->_gameBoardSide[2]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[1]._card.setPosition(scene->_gameBoardSide[2]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[1]._card.fixPriority(170);

		if (scene->_gameBoardSide[2]._handCard[1]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[1]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[1]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[1]._card.setFrame(scene->_gameBoardSide[2]._handCard[1]._cardId);
		}

		scene->_aSound2.play(61);

		Common::Point pt(14, 36);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 7: {
		scene->_gameBoardSide[3]._handCard[1]._card.postInit();
		scene->_gameBoardSide[3]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[1]._card.setPosition(scene->_gameBoardSide[3]._handCard[1]._stationPos);
		scene->_gameBoardSide[3]._handCard[1]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[1]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(253, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 8: {
		scene->_gameBoardSide[0]._handCard[1]._card.postInit();
		scene->_gameBoardSide[0]._handCard[1]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[0]._handCard[1]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[1]._card.setPosition(scene->_gameBoardSide[0]._handCard[1]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[1]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[1]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[1]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(283, 102);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[1]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 9: {
		scene->_gameBoardSide[1]._handCard[2]._card.postInit();
		scene->_gameBoardSide[1]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[1]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[1]._handCard[2]._card.setPosition(scene->_gameBoardSide[1]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[1]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[1]._handCard[2]._card.setFrame(4);
		scene->_gameBoardSide[1]._handCard[2]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(64, 174);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[2]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 10: {
		scene->_gameBoardSide[2]._handCard[2]._card.postInit();
		scene->_gameBoardSide[2]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[2]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[2]._handCard[2]._card.setPosition(scene->_gameBoardSide[2]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[2]._handCard[2]._card.fixPriority(170);

		if (scene->_gameBoardSide[2]._handCard[2]._cardId > 25) {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(4);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId - 25);
		} else if (scene->_gameBoardSide[2]._handCard[2]._cardId > 9) {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(3);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId - 9);
		} else {
			scene->_gameBoardSide[2]._handCard[2]._card.setStrip(2);
			scene->_gameBoardSide[2]._handCard[2]._card.setFrame(scene->_gameBoardSide[2]._handCard[2]._cardId);
		}

		scene->_aSound2.play(61);

		Common::Point pt(14, 58);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[3]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 11: {
		scene->_gameBoardSide[3]._handCard[2]._card.postInit();
		scene->_gameBoardSide[3]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[3]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[3]._handCard[2]._card.setPosition(scene->_gameBoardSide[3]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[3]._handCard[2]._card.setStrip(1);
		scene->_gameBoardSide[3]._handCard[2]._card.setFrame(3);
		scene->_gameBoardSide[3]._handCard[2]._card.fixPriority(170);
		scene->_aSound2.play(61);

		Common::Point pt(226, 5);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);

		scene->_gameBoardSide[0]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
		}
		break;
	case 12:
		scene->_gameBoardSide[0]._handCard[2]._card.postInit();
		scene->_gameBoardSide[0]._handCard[2]._card._moveDiff = Common::Point(30, 30);
		scene->_gameBoardSide[0]._handCard[2]._card.setVisage(1332);
		scene->_gameBoardSide[0]._handCard[2]._card.setPosition(scene->_gameBoardSide[0]._handCard[2]._stationPos, 0);
		scene->_gameBoardSide[0]._handCard[2]._card.setStrip(5);
		scene->_gameBoardSide[0]._handCard[2]._card.setFrame(1);
		scene->_gameBoardSide[0]._handCard[2]._card.fixPriority(170);
		scene->_animatedCard._card.hide();
	default:
		break;
	}

	if (_actionIndex > 12) {
		scene->_currentPlayerNumb = 0;
		R2_GLOBALS._sceneObjects->draw();
		scene->actionDisplay(1330, 0, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		scene->handleNextTurn();
	} else if (_actionIndex >= 1) {
		scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
		scene->_cardsAvailableNumb--;
	}
}

/**
 * Action used to handle the other players' turn
 */
void Scene1337::Action4::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95), 0);
			scene->_animatedCard._card.show();
			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;

			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else {
			// Self call, forcing next actionIndex
			signal();
		}
		break;
	case 1:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card1Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[0]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 2:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card2Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[1]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 3:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card3Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[2]);

		scene->_animatedCard._card.hide();
		if ( (scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._cardId == 0)
		  && (!scene->isStationCard(scene->_gameBoardSide[scene->_currentPlayerNumb]._delayCard._cardId))) {
			if (scene->_cardsAvailableNumb < 0)
				scene->shuffleCards();
			scene->_animatedCard._card.setPosition(Common::Point(162, 95));
			scene->_animatedCard._card.show();

			scene->_aSound2.play(61);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos, this);

			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._cardId = scene->_availableCardsPile[scene->_cardsAvailableNumb];
			scene->_availableCardsPile[scene->_cardsAvailableNumb] = 0;
			scene->_cardsAvailableNumb--;
			if (scene->_cardsAvailableNumb < 0)
				scene->_stockPile.remove();
		} else
			signal();
		break;
	case 4:
		if ( (scene->_animatedCard._card._position.x == scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos.x)
		  && (scene->_animatedCard._card._position.y == scene->_gameBoardSide[scene->_currentPlayerNumb]._card4Pos.y) ) {
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.postInit();
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card._moveDiff = Common::Point(30, 30);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setVisage(1332);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setPosition(scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._stationPos, 0);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setStrip(1);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.setFrame(scene->_gameBoardSide[scene->_currentPlayerNumb]._frameNum);
			scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]._card.fixPriority(170);
		}

		if ((R2_GLOBALS._debugCardGame) || (scene->_currentPlayerNumb == 2))
			scene->setAnimationInfo(&scene->_gameBoardSide[scene->_currentPlayerNumb]._handCard[3]);

		scene->_animatedCard._card.hide();
		switch (scene->_currentPlayerNumb) {
		case 0:
			scene->handlePlayer0();
			break;
		case 1:
			scene->handlePlayer1();
			break;
		case 2:
			scene->handleAutoplayPlayer2();
			break;
		case 3:
			scene->handlePlayer3();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

/**
 * Animations for discarding a card
 */
void Scene1337::Action5::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard1->_cardId;
		scene->_currentDiscardIndex--;
		if (!g_globals->_sceneObjects->contains(&scene->_discardPile._card)) {
			// The first discarded card makes the pile appear
			scene->_discardPile._card.postInit();
			scene->_discardPile._card.hide();
			scene->_discardPile._card.setVisage(1332);
			scene->_discardPile._card.setPosition(scene->_discardPile._stationPos, 0);
			scene->_discardPile._card.fixPriority(170);
		}

		scene->_discardPile._cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		Common::Point pt(128, 95);
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &pt, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(&scene->_discardPile);
		scene->_aSound2.play(61);
		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

/**
 * Animations for playing a platform card
 */
void Scene1337::Action6::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = 1;
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos);
		scene->_actionCard2->_card.fixPriority(170);

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(59);
		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

/**
 * Upgrade platform to station by playing a station card on top of it
 */
void Scene1337::Action7::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(59);
		scene->_discardedPlatformCard._cardId = 1;
		scene->_discardedPlatformCard._stationPos = scene->_actionCard2->_stationPos;
		scene->_discardedPlatformCard._card.postInit();
		scene->_discardedPlatformCard._card.hide();
		scene->_discardedPlatformCard._card._flags = OBJFLAG_HIDING;

		scene->discardCard(&scene->_discardedPlatformCard);
		break;
	default:
		break;
	}
}

void Scene1337::Action8::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;

		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

// Play delay card
void Scene1337::Action9::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_actionCard2->_card.fixPriority(170);

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(57);

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}

		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

// Counter a trick with a card
void Scene1337::Action10::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard3->_card.postInit();
		scene->_actionCard3->_card.hide();
		scene->_actionCard3->_card.setVisage(1332);
		scene->_actionCard3->_card.setPosition(scene->_actionCard3->_stationPos, 0);
		scene->_actionCard3->_card.fixPriority(170);
		scene->_actionCard3->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		if (scene->_actionCard1 == &scene->_selectedCard) {
			scene->setCursorData(5, 1, 4);
			scene->subC4CEC();
		}

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();
		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard3->_stationPos, this);
		}
		break;
	case 1: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard3);
		scene->_aSound1.play(57);

		bool found = false;
		int indexFound = -1;

		switch (scene->_actionIdx1) {
		case 0:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				if (scene->_gameBoardSide[0]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 1:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				if (scene->_gameBoardSide[1]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 2:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				if (scene->_gameBoardSide[2]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		case 3:
			for (indexFound = 0; indexFound < 3; indexFound++) {
				if (scene->_gameBoardSide[3]._handCard[indexFound]._cardId == 29) {
					found = true;
					break;
				}
			}
			break;
		default:
			break;
		}

		bool found2 = false;

		if (found) {
			switch (scene->_actionIdx1) {
			case 0:
				scene->subC51A0(&scene->_gameBoardSide[0]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			case 1:
				scene->subC51A0(&scene->_gameBoardSide[1]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			case 2:
				scene->subC4CD2();
				if (MessageDialog::show(USE_INTERCEPTOR, NO_MSG, YES_MSG) == 0)
					scene->subC4CEC();
				else {
					scene->subC51A0(&scene->_gameBoardSide[2]._handCard[indexFound], scene->_actionCard3);
					found2 = true;
				}
				break;
			case 3:
				scene->subC51A0(&scene->_gameBoardSide[3]._handCard[indexFound], scene->_actionCard3);
				found2 = true;
				break;
			default:
				break;
			}
		}

		if (!found2)
			break;

		if (scene->_actionIdx1 == 2) {
			int j = 0;
			for (int i = 0; i <= 7; i++) {
				if (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0)
					++j;
			}

			if (j <= 1) {
				for (int i = 0; i <= 7; i++) {
					if (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0) {
						scene->_actionCard2 = &scene->_gameBoardSide[2]._outpostStation[i];
						break;
					}
				}
			} else {
				scene->subC4CD2();

				found2 = false;
				while (!found2) {
					scene->actionDisplay(1330, 130, 159, 10, 1, 200, 0, 7, 0, 154, 154);

					// Wait for a mouse or keypress
					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					for (int i = 0; i <= 7; i++) {
						if (scene->_gameBoardSide[2]._outpostStation[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[2]._outpostStation[i]._cardId != 0)) {
							scene->_actionCard2 = &scene->_gameBoardSide[2]._outpostStation[0];
							found2 = true;
							break;
						}
					}
				}
				scene->subC4CEC();
			}
		}

		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;
		scene->_actionCard2->_cardId = 0;
		scene->_actionCard2->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard3->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		scene->discardCard(scene->_actionCard3);
		break;
	default:
		break;
	}
}

// Use trick (card #25 - thieft ?) and pick a card from the opponent
void Scene1337::Action11::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_actionCard2->_card.postInit();
		scene->_actionCard2->_card.hide();
		scene->_actionCard2->_card.setVisage(1332);
		scene->_actionCard2->_card.setPosition(scene->_actionCard2->_stationPos, 0);
		scene->_actionCard2->_card.fixPriority(170);
		scene->_actionCard2->_cardId = 25;

		if (scene->_actionIdx1 == 2) {
			scene->_animatedCard._card.setPosition(scene->_actionCard2->_stationPos, 0);
			scene->setCursorData(5, 1, 4);
		} else {
			scene->_actionCard1->_cardId = 0;
			scene->_actionCard1->_card.remove();
			scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		}
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1: {
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(57);

		bool found = false;
		bool noAction = true;

		int i = -1;

		switch (scene->_actionIdx2) {
		case 0:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[0]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getFreeHandCard(scene->_actionIdx1) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[0]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[0]._emptyStationPos;
				if (scene->_actionIdx1 != 0) {
					int tmpVal = scene->getFreeHandCard(scene->_actionIdx1);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx1]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		case 1:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[1]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getFreeHandCard(scene->_actionIdx1) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[1]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[1]._emptyStationPos;
				if (scene->_actionIdx1 != 1) {
					int tmpVal = scene->getFreeHandCard(scene->_actionIdx1);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx1]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		case 2:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[2]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getFreeHandCard(scene->_actionIdx1) != -1)) {
				scene->subC4CD2();
				if (MessageDialog::show(USE_DOUBLE_AGENT, NO_MSG, YES_MSG) == 0)
					scene->subC4CEC();
				else {
					scene->subC4CEC();
					scene->_actionCard1 = &scene->_gameBoardSide[2]._handCard[i];
					scene->_actionCard2 = &scene->_gameBoardSide[2]._emptyStationPos;
					if (scene->_actionIdx1 != 2) {
						int tmpVal = scene->getFreeHandCard(scene->_actionIdx1);
						scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx1]._handCard[tmpVal];
					}
					scene->_actionItem.setAction(&scene->_action12);
					noAction = false;
				}
			}
			break;
		case 3:
			for (i = 0; i <= 3; i++) {
				if (scene->_gameBoardSide[3]._handCard[i]._cardId == 27) {
					found = true;
					break;
				}
			}

			if ((found) && (scene->getFreeHandCard(scene->_actionIdx1) != -1)) {
				scene->_actionCard1 = &scene->_gameBoardSide[3]._handCard[i];
				scene->_actionCard2 = &scene->_gameBoardSide[3]._emptyStationPos;
				if (scene->_actionIdx1 != 3) {
					int tmpVal = scene->getFreeHandCard(scene->_actionIdx1);
					scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx1]._handCard[tmpVal];
				}
				scene->_actionItem.setAction(&scene->_action12);
				noAction = false;
			}
			break;
		default:
			break;
		}

		if (!noAction)
			return;

		if (scene->_actionIdx1 == 2) {
			int count = 0;
			if (scene->_actionIdx2 != 2) {
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[scene->_actionIdx2]._handCard[i]._cardId == 0)
						++count;
				}
			}

			if (count > 1) {
				scene->subC4CD2();

				found = false;
				while (!found) {
					switch (scene->_actionIdx2) {
					case 0:
						scene->actionDisplay(1330, 131, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 1:
						scene->actionDisplay(1330, 132, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 3:
						scene->actionDisplay(1330, 133, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					default:
						break;
					}

					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					found = false;

					if (scene->_actionIdx2 != 2) {
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[scene->_actionIdx2]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[scene->_actionIdx2]._handCard[i]._cardId != 0)) {
								scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx2]._handCard[i];
								found = true;
								break;
							}
						}
					}
				} // while
				scene->_displayHelpFl = true;
				scene->subC4CEC();
			} else if (scene->_actionIdx2 != 2) {
				int tmpVal = scene->getFreeHandCard(scene->_actionIdx2);
				scene->_actionCard3 = &scene->_gameBoardSide[scene->_actionIdx2]._handCard[tmpVal];
			}
		}

		scene->_actionCard1->_card.postInit();
		scene->_actionCard1->_card.hide();
		scene->_actionCard1->_card.setVisage(1332);
		scene->_actionCard1->_card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_actionCard1->_card.fixPriority(170);
		scene->_actionCard1->_card.setStrip2(1);
		scene->_actionCard1->_cardId = scene->_actionCard3->_cardId;

		scene->_actionCard3->_cardId = 0;
		scene->_actionCard3->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard3->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard1->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		switch (scene->_actionIdx1) {
		case 0:
			scene->_actionCard1->_card.setFrame2(2);
			scene->_actionCard1->_card.show();
			break;
		case 1:
			scene->_actionCard1->_card.setFrame2(4);
			scene->_actionCard1->_card.show();
			break;
		case 3:
			scene->_actionCard1->_card.setFrame2(3);
			scene->_actionCard1->_card.show();
			break;
		default:
			scene->setAnimationInfo(scene->_actionCard1);
			break;
		}

		scene->_currentPlayerNumb--;
		scene->_showPlayerTurn = false;
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

// Pick a card in opponent hand
void Scene1337::Action12::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0:
		signal();
		break;
	case 1: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex++;
		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;
		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();
		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 2:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		if (scene->_actionIdx2 == 2) {
			int count = 0;
			int i = -1;
			switch (scene->_actionIdx1) {
			case 0:
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[0]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			case 1:
				for (i = 0; i <= 3; i++) {
					// The original game was counting in the hand of player 3, which is obviously wrong
					if (scene->_gameBoardSide[1]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			case 3:
				for (i = 0; i <= 3; i++) {
					if (scene->_gameBoardSide[3]._handCard[i]._cardId != 0)
						++count;
				}
				break;
			default:
				break;
			}

			if (count > 1) {
				scene->subC4CD2();

				bool found = false;

				while (!found) {
					switch (scene->_actionIdx1) {
					case 0:
						scene->actionDisplay(1330, 131, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 1:
						scene->actionDisplay(1330, 132, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					case 3:
						scene->actionDisplay(1330, 133, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						break;
					default:
						break;
					}

					Event event;
					while (!g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN | EVENT_KEYPRESS) && !g_vm->shouldQuit()) {
						g_globals->_scenePalette.signalListeners();
						R2_GLOBALS._sceneObjects->draw();
						g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
					}

					scene->_selectedCard._stationPos = event.mousePos;

					if (scene->_actionIdx1 == 0) {
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[0]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[0]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[0]._handCard[i];
								break;
							}
						}
					}

					if (scene->_actionIdx1 == 3) {
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[3]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[3]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[3]._handCard[i];
								break;
							}
						}
					}

					if (scene->_actionIdx1 == 1) {
						for (i = 0; i <= 3; i++) {
							if (scene->_gameBoardSide[1]._handCard[i].isIn(scene->_selectedCard._stationPos) && (scene->_gameBoardSide[1]._handCard[i]._cardId != 0)) {
								found = true;
								scene->_actionCard3 = &scene->_gameBoardSide[1]._handCard[i];
								break;
							}
						}
					}
				}
				scene->subC4CEC();
			} else if (scene->_actionIdx1 != 1) {
				switch (scene->_actionIdx1) {
				case 0:
					scene->_actionCard3 = &scene->_gameBoardSide[0]._handCard[scene->getFreeHandCard(0)];
					break;
				case 3:
					scene->_actionCard3 = &scene->_gameBoardSide[3]._handCard[scene->getFreeHandCard(3)];
					break;
				default:
					break;
				}
			} else {
				scene->_actionCard3 = &scene->_gameBoardSide[1]._handCard[scene->getFreeHandCard(1)];
			}

			scene->_actionCard1->_card.postInit();
			scene->_actionCard1->_card.hide();
			scene->_actionCard1->_card.setVisage(1332);
			scene->_actionCard1->_card.setPosition(scene->_actionCard1->_stationPos);
			scene->_actionCard1->_card.fixPriority(170);
			scene->_actionCard1->_card.setStrip2(1);
			scene->_actionCard1->_cardId = scene->_actionCard3->_cardId;

			scene->_actionCard3->_cardId = 0;
			scene->_actionCard3->_card.remove();

			scene->_animatedCard._card.setPosition(scene->_actionCard3->_stationPos);
			scene->_animatedCard._card.show();
			scene->_aSound1.play(57);

			NpcMover *mover = new NpcMover();
			scene->_animatedCard._card.addMover(mover, &scene->_actionCard1->_stationPos, this);
		}
		break;
	case 3:
		scene->_animatedCard._card.hide();
		switch (scene->_actionIdx2) {
		case 0:
			scene->_actionCard1->_card.setFrame2(2);
			scene->_actionCard1->_card.show();
			break;
		case 1:
			scene->_actionCard1->_card.setFrame2(4);
			scene->_actionCard1->_card.show();
			break;
		case 3:
			scene->_actionCard1->_card.setFrame2(3);
			scene->_actionCard1->_card.show();
			break;
		default:
			scene->setAnimationInfo(scene->_actionCard1);
			break;
		}
		scene->discardCard(scene->_actionCard2);
		scene->handleNextTurn();
		break;
	default:
		break;
	}
}

void Scene1337::Action13::signal() {
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;

	switch (_actionIndex++) {
	case 0: {
		scene->_availableCardsPile[scene->_currentDiscardIndex] = scene->_actionCard2->_cardId;
		scene->_currentDiscardIndex--;

		scene->_actionCard2->_cardId = scene->_actionCard1->_cardId;

		scene->_actionCard1->_cardId = 0;
		scene->_actionCard1->_card.remove();

		scene->_animatedCard._card.setPosition(scene->_actionCard1->_stationPos, 0);
		scene->_animatedCard._card.show();

		NpcMover *mover = new NpcMover();
		scene->_animatedCard._card.addMover(mover, &scene->_actionCard2->_stationPos, this);
		}
		break;
	case 1:
		scene->_animatedCard._card.hide();
		scene->setAnimationInfo(scene->_actionCard2);
		scene->_aSound1.play(58);
		signal();
		break;
	case 2:
		scene->discardCard(scene->_actionCard2);
		break;
	default:
		break;
	}
}

void Scene1337::postInit(SceneObjectList *OwnerList) {
// In the original, may be found in subPostInit.
// Without it, enableControl asserts
	loadScene(1330);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
//

	// Hide the user interface
	BF_GLOBALS._interfaceY = SCREEN_HEIGHT;
	R2_GLOBALS._uiElements._visible = false;

	R2_GLOBALS._player.enableControl();
	R2_GLOBALS._player._canWalk = false;
	R2_GLOBALS._player._uiEnabled = false;

	_delayedFunction = nullptr;

	_actionCard1 = nullptr;
	_actionCard2 = nullptr;
	_actionCard3 = nullptr;

	_gameBoardSide[2]._handCard[0]._stationPos = Common::Point(10, 174);
	_gameBoardSide[2]._handCard[1]._stationPos = Common::Point(37, 174);
	_gameBoardSide[2]._handCard[2]._stationPos = Common::Point(64, 174);
	_gameBoardSide[2]._handCard[3]._stationPos = Common::Point(91, 174);

	_gameBoardSide[2]._outpostStation[0]._stationPos = Common::Point(119, 174);
	_gameBoardSide[2]._outpostStation[1]._stationPos = Common::Point(119, 148);
	_gameBoardSide[2]._outpostStation[2]._stationPos = Common::Point(119, 122);
	_gameBoardSide[2]._outpostStation[3]._stationPos = Common::Point(145, 122);
	_gameBoardSide[2]._outpostStation[4]._stationPos = Common::Point(171, 122);
	_gameBoardSide[2]._outpostStation[5]._stationPos = Common::Point(171, 148);
	_gameBoardSide[2]._outpostStation[6]._stationPos = Common::Point(171, 174);
	_gameBoardSide[2]._outpostStation[7]._stationPos = Common::Point(145, 174);

	_gameBoardSide[2]._delayCard._stationPos = Common::Point(199, 174);

	_gameBoardSide[2]._emptyStationPos._stationPos = Common::Point(145, 148);

	_gameBoardSide[2]._card1Pos = Common::Point(10, 174);
	_gameBoardSide[2]._card2Pos = Common::Point(37, 174);
	_gameBoardSide[2]._card3Pos = Common::Point(64, 174);
	_gameBoardSide[2]._card4Pos = Common::Point(91, 174);
	_gameBoardSide[2]._frameNum = 2;

	_gameBoardSide[3]._handCard[0]._stationPos = Common::Point(14, 14);
	_gameBoardSide[3]._handCard[1]._stationPos = Common::Point(14, 36);
	_gameBoardSide[3]._handCard[2]._stationPos = Common::Point(14, 58);
	_gameBoardSide[3]._handCard[3]._stationPos = Common::Point(14, 80);

	_gameBoardSide[3]._outpostStation[0]._stationPos = Common::Point(37, 66);
	_gameBoardSide[3]._outpostStation[1]._stationPos = Common::Point(63, 66);
	_gameBoardSide[3]._outpostStation[2]._stationPos = Common::Point(89, 66);
	_gameBoardSide[3]._outpostStation[3]._stationPos = Common::Point(89, 92);
	_gameBoardSide[3]._outpostStation[4]._stationPos = Common::Point(89, 118);
	_gameBoardSide[3]._outpostStation[5]._stationPos = Common::Point(63, 118);
	_gameBoardSide[3]._outpostStation[6]._stationPos = Common::Point(37, 118);
	_gameBoardSide[3]._outpostStation[7]._stationPos = Common::Point(37, 92);

	_gameBoardSide[3]._delayCard._stationPos = Common::Point(37, 145);

	_gameBoardSide[3]._emptyStationPos._stationPos = Common::Point(63, 92);

	_gameBoardSide[3]._card1Pos = Common::Point(14, 14);
	_gameBoardSide[3]._card2Pos = Common::Point(14, 36);
	_gameBoardSide[3]._card3Pos = Common::Point(14, 58);
	_gameBoardSide[3]._card4Pos = Common::Point(14, 80);
	_gameBoardSide[3]._frameNum = 3;

	_gameBoardSide[0]._handCard[0]._stationPos = Common::Point(280, 5);
	_gameBoardSide[0]._handCard[1]._stationPos = Common::Point(253, 5);
	_gameBoardSide[0]._handCard[2]._stationPos = Common::Point(226, 5);
	_gameBoardSide[0]._handCard[3]._stationPos = Common::Point(199, 5);

	_gameBoardSide[0]._outpostStation[0]._stationPos = Common::Point(171, 16);
	_gameBoardSide[0]._outpostStation[1]._stationPos = Common::Point(171, 42);
	_gameBoardSide[0]._outpostStation[2]._stationPos = Common::Point(171, 68);
	_gameBoardSide[0]._outpostStation[3]._stationPos = Common::Point(145, 68);
	_gameBoardSide[0]._outpostStation[4]._stationPos = Common::Point(119, 68);
	_gameBoardSide[0]._outpostStation[5]._stationPos = Common::Point(119, 42);
	_gameBoardSide[0]._outpostStation[6]._stationPos = Common::Point(119, 16);
	_gameBoardSide[0]._outpostStation[7]._stationPos = Common::Point(145, 16);

	_gameBoardSide[0]._delayCard._stationPos = Common::Point(91, 16);

	_gameBoardSide[0]._emptyStationPos._stationPos = Common::Point(145, 42);

	_gameBoardSide[0]._card1Pos = Common::Point(280, 5);
	_gameBoardSide[0]._card2Pos = Common::Point(253, 5);
	_gameBoardSide[0]._card3Pos = Common::Point(226, 5);
	_gameBoardSide[0]._card4Pos = Common::Point(199, 5);
	_gameBoardSide[0]._frameNum = 2;

	_gameBoardSide[1]._handCard[0]._stationPos = Common::Point(283, 146);
	_gameBoardSide[1]._handCard[1]._stationPos = Common::Point(283, 124);
	_gameBoardSide[1]._handCard[2]._stationPos = Common::Point(283, 102);
	_gameBoardSide[1]._handCard[3]._stationPos = Common::Point(283, 80);

	_gameBoardSide[1]._outpostStation[0]._stationPos = Common::Point(253, 122);
	_gameBoardSide[1]._outpostStation[1]._stationPos = Common::Point(227, 122);
	_gameBoardSide[1]._outpostStation[2]._stationPos = Common::Point(201, 122);
	_gameBoardSide[1]._outpostStation[3]._stationPos = Common::Point(201, 96);
	_gameBoardSide[1]._outpostStation[4]._stationPos = Common::Point(201, 70);
	_gameBoardSide[1]._outpostStation[5]._stationPos = Common::Point(227, 70);
	_gameBoardSide[1]._outpostStation[6]._stationPos = Common::Point(253, 70);
	_gameBoardSide[1]._outpostStation[7]._stationPos = Common::Point(253, 96);

	_gameBoardSide[1]._delayCard._stationPos = Common::Point(253, 43);

	_gameBoardSide[1]._emptyStationPos._stationPos = Common::Point(227, 96);

	_gameBoardSide[1]._card1Pos = Common::Point(283, 146);
	_gameBoardSide[1]._card2Pos = Common::Point(283, 124);
	_gameBoardSide[1]._card3Pos = Common::Point(283, 102);
	_gameBoardSide[1]._card4Pos = Common::Point(283, 80);
	_gameBoardSide[1]._frameNum = 4;

	subPostInit();

	_stockPile.postInit();
}

void Scene1337::remove() {
	if (R2_GLOBALS._v57709 > 1) {
		subD1917();
		subD1940(false);
	}

	R2_GLOBALS._uiElements._active = true;
	R2_GLOBALS._uiElements._visible = true;
	SceneExt::remove();
}

void Scene1337::process(Event &event) {
	if (event.eventType == EVENT_BUTTON_DOWN) {
		if (event.btnState == BTNSHIFT_RIGHT) {
			updateCursorId(R2_GLOBALS._mouseCursorId, true);
			event.handled = true;
		} else if (_delayedFunction) {
			FunctionPtrType tmpFctPtr = _delayedFunction;
			_delayedFunction = nullptr;
			(this->*tmpFctPtr)();
			event.handled = true;
		}
	} else if (event.eventType == EVENT_KEYPRESS) {
		if (event.kbd.keycode == Common::KEYCODE_SPACE) {
			if (_delayedFunction) {
				FunctionPtrType tmpFctPtr = _delayedFunction;
				_delayedFunction = nullptr;
				(this->*tmpFctPtr)();
				event.handled = true;
			}
		} else
			warning("Fixme: Find proper keycode value");
	}

	if (!event.handled)
		Scene::process(event);
}

void Scene1337::dispatch() {
	if (!_instructionsDisplayedFl) {
		++_instructionsWaitCount;
		if (_instructionsWaitCount == 4) {
			_instructionsDisplayedFl = true;
			suggestInstructions();
		}
	}

	// The following code is in the original in sceneHandler::process(),
	// which is terrible as it's checked in every scene of the game.
	setCursorData(5, _cursorCurStrip, _cursorCurFrame);
	//

	Scene::dispatch();
}

void Scene1337::actionDisplay(int resNum, int lineNum, int x, int y, int keepOnScreen, int width, int textMode, int fontNum, int colFG, int colBGExt, int colFGExt) {
	// TODO: Check if it's normal that arg5 is unused and replaced by an hardcoded 0 value
	// May hide an original bug

	SceneItem::display(resNum, lineNum, SET_X, x, SET_Y, y, SET_KEEP_ONSCREEN, 0,
		               SET_WIDTH, width, SET_POS_MODE, -1, SET_TEXT_MODE, textMode,
					   SET_FONT, fontNum, SET_FG_COLOR, colFG, SET_EXT_BGCOLOR, colBGExt,
					   SET_EXT_FGCOLOR, colFGExt, LIST_END);
}

void Scene1337::setAnimationInfo(Card *card) {
	if (!card)
		return;

	if (card->_cardId > 25) {
		card->_card.setStrip2(4);
		card->_card.setFrame(card->_cardId - 25);
	} else if (card->_cardId > 9) {
		card->_card.setStrip2(3);
		card->_card.setFrame(card->_cardId - 9);
	} else {
		card->_card.setStrip2(2);
		card->_card.setFrame(card->_cardId);
	}

	card->_card.show();
	R2_GLOBALS._sceneObjects->draw();
}

void Scene1337::handleNextTurn() {
	switch (_winnerId) {
	case -1:
		++_currentPlayerNumb;
		if (_currentPlayerNumb > 3)
			_currentPlayerNumb = 0;

		if (_showPlayerTurn) {
			_currentPlayerArrow.show();
			switch (_currentPlayerNumb) {
			case 0:
				_currentPlayerArrow.setStrip(3);
				break;
			case 1:
				_currentPlayerArrow.setStrip(4);
				break;
			case 2:
				subD1975(174, 107);
				_currentPlayerArrow.setStrip(1);
				break;
			case 3:
				subC4CEC();
				_currentPlayerArrow.setStrip(2);
				break;
			default:
				break;
			}

			if (!_autoplay)
				_delayedFunction = &Scene1337::handlePlayerTurn;
			else
				handlePlayerTurn();
		} else {
			handlePlayerTurn();
		}
		break;
	case 0:
		_aSound2.play(62);
		actionDisplay(1330, 135, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 121, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 122, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 123, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	case 1:
		_aSound2.play(62);
		actionDisplay(1330, 151, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		actionDisplay(1330, 118, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 119, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 120, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	case 2:
		_aSound2.play(62);
		actionDisplay(1330, 134, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		actionDisplay(1330, 124, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 126, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 125, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		break;
	case 3:
		_aSound2.play(62);
		actionDisplay(1330, 150, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		actionDisplay(1330, 115, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		actionDisplay(1330, 116, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		R2_GLOBALS._sceneObjects->draw();
		actionDisplay(1330, 117, 159, 134, 1, 200, 0, 7, 0, 105, 105);
		break;
	default:
		break;
	}

	if (_winnerId != -1)
		R2_GLOBALS._sceneManager.changeScene(125);

}

void Scene1337::handlePlayerTurn() {
	if (_showPlayerTurn)
		_currentPlayerArrow.hide();

	switch (_currentPlayerNumb) {
	case 2:
		subC4CD2();
		if (_displayHelpFl)
			actionDisplay(1330, 114, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		_displayHelpFl = false;
	// No break on purpose
	case 0:
	// No break on purpose
	case 1:
	// No break on purpose
	case 3:
		_actionItem.setAction(&_action4);
	default:
		break;
	}

	_showPlayerTurn = true;

}

bool Scene1337::isStationCard(int cardId) {
	switch (cardId) {
	case 10:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 17:
	// No break on purpose
	case 18:
	// No break on purpose
	case 19:
	// No break on purpose
	case 20:
	// No break on purpose
	case 21:
		return true;
	default:
		return false;
	}
}

bool Scene1337::isStopConstructionCard(int cardId) {
	switch (cardId) {
	case 11:
	// No break on purpose
	case 14:
	// No break on purpose
	case 16:
	// No break on purpose
	case 24:
		return true;
	default:
		return false;
	}
}

int Scene1337::getStationId(int playerId, int handCardId) {
	if ((_gameBoardSide[playerId]._handCard[handCardId]._cardId > 1) && (_gameBoardSide[playerId]._handCard[handCardId]._cardId <= 9))
		return handCardId;

	return -1;
}

int Scene1337::findPlatformCardInHand(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 1)
			return i;
	}

	return -1;
}

int Scene1337::findCard13InHand(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 13)
			return i;
	}

	return -1;
}

int Scene1337::checkThieftCard(int playerId) {
	for (int i = 0; i <= 3; i++) {
		if (_gameBoardSide[playerId]._handCard[i]._cardId == 25)
			return i;
	}

	return -1;
}

int Scene1337::isDelayCard(int cardId) {
	switch (cardId) {
	case 11:
	// No break on purpose
	case 14:
	// No break on purpose
	case 16:
	// No break on purpose
	case 24:
		return cardId;
		break;
	default:
		return -1;
		break;
	}
}

int Scene1337::getStationCardId(int cardId) {
	switch (cardId) {
	case 10:
	// No break on purpose
	case 12:
	// No break on purpose
	case 15:
	// No break on purpose
	case 17:
	// No break on purpose
	case 18:
	// No break on purpose
	case 19:
	// No break on purpose
	case 20:
	// No break on purpose
	case 21:
		return cardId;
	default:
		return -1;
	}
}

void Scene1337::handlePlayer01Discard(int playerId) {
	switch (playerId) {
	case 0:
		for (int i = 0; i <= 3; i++) {
			if (getStationCardId(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (isDelayCard(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId > 1) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 9)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId >= 26) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 33)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 25) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 13) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}
		break;
	case 1:
		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId >= 26) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 33)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if ((_gameBoardSide[playerId]._handCard[i]._cardId > 1) && (_gameBoardSide[playerId]._handCard[i]._cardId <= 9)) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (getStationCardId(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (isDelayCard(_gameBoardSide[playerId]._handCard[i]._cardId) != -1) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 25) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		for (int i = 0; i <= 3; i++) {
			if (_gameBoardSide[playerId]._handCard[i]._cardId == 13) {
				discardCard(&_gameBoardSide[playerId]._handCard[i]);
				return;
			}
		}

		break;
	default:
		break;
	}
}

void Scene1337::playThieftCard(int playerId, Card *card, int victimId) {
	_actionIdx1 = playerId;
	_actionIdx2 = victimId;

	int randIndx;

	for (;;) {
		randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);
		if (_gameBoardSide[victimId]._handCard[randIndx]._cardId != 0)
			break;
	}

	_actionCard1 = card;
	_actionCard2 = &_gameBoardSide[victimId]._emptyStationPos;
	_actionCard3 = &_gameBoardSide[victimId]._handCard[randIndx];

	_actionItem.setAction(&_action11);
}

int Scene1337::getPreventionCardId(int cardId) {
	int retVal;

	switch (cardId) {
	case 10:
		retVal = 2;
		break;
	case 12:
		retVal = 3;
		break;
	case 15:
		retVal = 5;
		break;
	case 17:
		retVal = 9;
		break;
	case 18:
		retVal = 6;
		break;
	case 19:
		retVal = 4;
		break;
	case 20:
		retVal = 8;
		break;
	case 21:
		retVal = 7;
		break;
	default:
		retVal = -1;
	}

	return retVal;
}

bool Scene1337::isAttackPossible(int victimId, int cardId) {
	if (victimId < 0 || victimId >= ARRAYSIZE(_gameBoardSide))
		error("Scene1337::isAttackPossible() victimId:%d out of range 0 to %d", victimId, ARRAYSIZE(_gameBoardSide)-1);

	for (int i = 0; i <= 7; i++) {
		if (_gameBoardSide[victimId]._outpostStation[i]._cardId != 0) {
			if (getPreventionCardId(cardId) == _gameBoardSide[victimId]._outpostStation[i]._cardId)
				return false;
		}
	}
	return true;
}

int Scene1337::getPlayerWithOutpost(int playerId) {
	int randPlayerId = R2_GLOBALS._randomSource.getRandomNumber(3);

	for (int i = 0; i <= 3; i++) {
		if (randPlayerId != playerId) {
			for (int j = 0; j <= 7; j++) {
				if (_gameBoardSide[randPlayerId]._outpostStation[j]._cardId != 0)
					return randPlayerId;
			}
		}

		if (playerId == 1) {
			randPlayerId--;
			if (randPlayerId < 0)
				randPlayerId = 3;
		} else {
			++randPlayerId;
			if (randPlayerId > 3)
				randPlayerId = 0;
		}
	}

	return -1;
}

bool Scene1337::checkAntiDelayCard(int delayCardId, int cardId) {
	if ((delayCardId == 11) && (cardId == 26))
		return true;

	if ((delayCardId == 14) && (cardId == 30))
		return true;

	if ((delayCardId == 16) && (cardId == 32))
		return true;

	if ((delayCardId == 24) && (cardId == 28))
		return true;

	return false;
}

void Scene1337::playStationCard(Card *station, Card *platform) {
	_actionCard1 = station;
	_actionCard2 = platform;
	_actionItem.setAction(&_action7);
}

int Scene1337::getFreeHandCard(int playerId) {
	if ( (_gameBoardSide[playerId]._handCard[0]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[1]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[2]._cardId == 0)
	  && (_gameBoardSide[playerId]._handCard[3]._cardId == 0))
	  return -1;

	int randIndx;
	for (;;) {
		randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);
		if (_gameBoardSide[playerId]._handCard[randIndx]._cardId == 0)
			break;
	}

	return randIndx;
}

void Scene1337::playPlatformCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action6);
}

void Scene1337::playDelayCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action9);
}

void Scene1337::playAntiDelayCard(Card *card, Card *dest) {
	_actionCard1 = card;
	_actionCard2 = dest;

	_actionItem.setAction(&_action8);

	handleNextTurn();
}


Scene1337::Card *Scene1337::getStationCard(int playerId) {
	for (int i = 0; i <= 7; i++) {
		if ((_gameBoardSide[playerId]._outpostStation[i]._cardId >= 1) && (_gameBoardSide[playerId]._outpostStation[i]._cardId <= 9))
			return &_gameBoardSide[playerId]._outpostStation[i];
	}

	return nullptr;
}

void Scene1337::playCounterTrickCard(Card *card, int playerId) {
	_actionCard1 = card;
	_actionCard2 = getStationCard(playerId);
	_actionCard3 = &_gameBoardSide[playerId]._emptyStationPos;
	_actionIdx1 = playerId;
	_actionItem.setAction(&_action10);
	handleNextTurn();
}

void Scene1337::discardCard(Card *card) {
	_actionCard1 = card;

	_actionItem.setAction(&_action5);
}

void Scene1337::subC4CD2() {
	if (R2_GLOBALS._v57709 > 0) {
		subD1917();
		subD1940(false); // _v5780C--
	}
}

void Scene1337::subC4CEC() {
	if (R2_GLOBALS._v57709 == 0) {
		subD18F5();
		subD1940(true); // _v5780C++
	}
}

void Scene1337::subC51A0(Card *subObj1, Card *subObj2) {
	_actionCard1 = subObj1;
	_actionCard2 = subObj2;

	_actionItem.setAction(&_action13);
}

void Scene1337::displayDialog(int dialogNumb) {
	switch (dialogNumb - 1) {
	case 0:
		actionDisplay(1330, 53, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 1:
		actionDisplay(1330, 57, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 2:
		actionDisplay(1330, 58, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 3:
		actionDisplay(1330, 59, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 4:
		actionDisplay(1330, 60, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 5:
		actionDisplay(1330, 61, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 6:
		actionDisplay(1330, 62, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 7:
		actionDisplay(1330, 63, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 8:
		actionDisplay(1330, 64, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 9:
		actionDisplay(1330, 65, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 10:
		actionDisplay(1330, 67, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 11:
		actionDisplay(1330, 69, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 12:
		actionDisplay(1330, 71, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 72, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 73, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 13:
		actionDisplay(1330, 79, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 14:
		actionDisplay(1330, 81, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 15:
		actionDisplay(1330, 83, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 16:
		actionDisplay(1330, 85, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 17:
		actionDisplay(1330, 87, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 18:
		actionDisplay(1330, 89, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 19:
		actionDisplay(1330, 91, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 20:
		actionDisplay(1330, 93, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 23:
		actionDisplay(1330, 95, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 24:
		actionDisplay(1330, 97, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 25:
		actionDisplay(1330, 104, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 26:
		actionDisplay(1330, 105, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 106, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 27:
		actionDisplay(1330, 110, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 28:
		actionDisplay(1330, 108, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		actionDisplay(1330, 109, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 29:
		actionDisplay(1330, 111, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	case 31:
		actionDisplay(1330, 112, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		break;
	default:
		break;
	}
}

void Scene1337::subPostInit() {
	R2_GLOBALS._v57709 = 0;
	R2_GLOBALS._v5780C = 0;
	updateCursorId(1, false);
	subD1940(true); // _v5780C++
	subD18F5();

//	loadScene(1330);
//	SceneExt::postInit();

	R2_GLOBALS._scenePalette.addRotation(224, 235, 1);

	_availableCardsPile[0] = 1;
	_availableCardsPile[1] = 1;
	_availableCardsPile[2] = 1;
	_availableCardsPile[3] = 1;
	_availableCardsPile[4] = 1;
	_availableCardsPile[5] = 1;
	_availableCardsPile[6] = 1;
	_availableCardsPile[7] = 1;
	_availableCardsPile[8] = 26;
	_availableCardsPile[9] = 2;
	_availableCardsPile[10] = 2;
	_availableCardsPile[11] = 2;
	_availableCardsPile[12] = 2;
	_availableCardsPile[13] = 2;
	_availableCardsPile[14] = 26;
	_availableCardsPile[15] = 3;
	_availableCardsPile[16] = 3;
	_availableCardsPile[17] = 3;
	_availableCardsPile[18] = 3;
	_availableCardsPile[19] = 3;
	_availableCardsPile[20] = 28;
	_availableCardsPile[21] = 4;
	_availableCardsPile[22] = 4;
	_availableCardsPile[23] = 4;
	_availableCardsPile[24] = 4;
	_availableCardsPile[25] = 4;
	_availableCardsPile[26] = 28;
	_availableCardsPile[27] = 5;
	_availableCardsPile[28] = 5;
	_availableCardsPile[29] = 5;
	_availableCardsPile[30] = 5;
	_availableCardsPile[31] = 5;
	_availableCardsPile[32] = 30;
	_availableCardsPile[33] = 6;
	_availableCardsPile[34] = 6;
	_availableCardsPile[35] = 6;
	_availableCardsPile[36] = 6;
	_availableCardsPile[37] = 6;
	_availableCardsPile[38] = 30;
	_availableCardsPile[39] = 7;
	_availableCardsPile[40] = 7;
	_availableCardsPile[41] = 7;
	_availableCardsPile[42] = 7;
	_availableCardsPile[43] = 7;
	_availableCardsPile[44] = 32;
	_availableCardsPile[45] = 8;
	_availableCardsPile[46] = 8;
	_availableCardsPile[47] = 8;
	_availableCardsPile[48] = 8;
	_availableCardsPile[49] = 8;
	_availableCardsPile[50] = 32;
	_availableCardsPile[51] = 9;
	_availableCardsPile[52] = 9;
	_availableCardsPile[53] = 9;
	_availableCardsPile[54] = 9;
	_availableCardsPile[55] = 9;
	_availableCardsPile[56] = 10;
	_availableCardsPile[57] = 11;
	_availableCardsPile[58] = 12;
	_availableCardsPile[59] = 13;
	_availableCardsPile[60] = 13;
	_availableCardsPile[61] = 14;
	_availableCardsPile[62] = 15;
	_availableCardsPile[63] = 16;
	_availableCardsPile[64] = 17;
	_availableCardsPile[65] = 18;
	_availableCardsPile[66] = 19;
	_availableCardsPile[67] = 20;
	_availableCardsPile[68] = 21;
	_availableCardsPile[69] = 26;
	_availableCardsPile[70] = 28;
	_availableCardsPile[71] = 24;
	_availableCardsPile[72] = 25;
	_availableCardsPile[73] = 25;
	_availableCardsPile[74] = 25;
	_availableCardsPile[75] = 25;
	_availableCardsPile[76] = 26;
	_availableCardsPile[77] = 26;
	_availableCardsPile[78] = 26;
	_availableCardsPile[79] = 27;
	_availableCardsPile[80] = 27;
	_availableCardsPile[81] = 28;
	_availableCardsPile[82] = 28;
	_availableCardsPile[83] = 28;
	_availableCardsPile[84] = 29;
	_availableCardsPile[85] = 29;
	_availableCardsPile[86] = 29;
	_availableCardsPile[87] = 30;
	_availableCardsPile[88] = 30;
	_availableCardsPile[89] = 30;
	_availableCardsPile[90] = 30;
	_availableCardsPile[91] = 32;
	_availableCardsPile[92] = 1;
	_availableCardsPile[93] = 32;
	_availableCardsPile[94] = 32;
	_availableCardsPile[95] = 32;
	_availableCardsPile[96] = 1;
	_availableCardsPile[97] = 1;
	_availableCardsPile[98] = 1;
	_availableCardsPile[99] = 0;

	_cardsAvailableNumb = 98;
	_currentDiscardIndex = 98; // CHECKME: Would make more sense at pos 99

	_discardPile._cardId = 0;
	_discardPile._stationPos = Common::Point(128, 95);

	_stockCard._cardId = 0;
	_stockCard._stationPos = Common::Point(162, 95);

	_selectedCard._cardId = 0;

	_animatedCard._card.postInit();
	_animatedCard._card.setVisage(1332);
	_animatedCard._card.setStrip(5);
	_animatedCard._card.setFrame(1);
	_animatedCard._card._moveDiff = Common::Point(10, 10);
	_animatedCard._card.fixPriority(400);
	_animatedCard._card.setPosition(Common::Point(128, 95), 0);
	_animatedCard._card.animate(ANIM_MODE_2, NULL);
	_animatedCard._card.hide();

	_currentPlayerArrow.postInit();
	_currentPlayerArrow.setVisage(1334);
	_currentPlayerArrow.setStrip(1);
	_currentPlayerArrow.setFrame(1);
	_currentPlayerArrow._numFrames = 12;
	_currentPlayerArrow.fixPriority(500);
	_currentPlayerArrow.setPosition(Common::Point(174, 107), 0);
	_currentPlayerArrow.animate(ANIM_MODE_2, NULL);
	_currentPlayerArrow.hide();

	_showPlayerTurn = true;
	_displayHelpFl = false;
	_winnerId = -1;

	_helpIcon.postInit();
	_helpIcon.setup(9531, 1, 1);
	_helpIcon.setPosition(Common::Point(249, 168));
	_helpIcon.setPriority(155);
	_helpIcon._effect = EFFECT_NONE;
	_helpIcon.show();

	_autoplay = false;
	_instructionsDisplayedFl = false;
	_instructionsWaitCount = 0;
}

void Scene1337::suggestInstructions() {
	if (R2_GLOBALS._v57709 > 0)
		subD1917();

	if (MessageDialog::show(NEED_INSTRUCTIONS, NO_MSG, YES_MSG) == 0) {
		if (R2_GLOBALS._v57709 == 0)
			subD18F5();
		dealCards();
	} else {
		if (R2_GLOBALS._v57709 == 0)
			subD18F5();
		displayInstructions();
	}
}

void Scene1337::displayInstructions() {
	_actionItem.setAction(&_action1);
}

void Scene1337::shuffleCards() {
	R2_GLOBALS._sceneObjects->draw();

	// Remove holes in card pile
	for (int i = 0; i <= 98; i++) {
		if (_availableCardsPile[i] == 0) {
			for (int j = i + 1; j <= 98; j ++) {
				if (_availableCardsPile[j] != 0) {
					_availableCardsPile[i] = _availableCardsPile[j];
					_availableCardsPile[j] = 0;
					break;
				}
			}
		}
	}

	// Compute the number of available cards
	for (int i = 0; i <= 99; i ++) {
		if (_availableCardsPile[i] == 0) {
			// CHECKME: This will fail if i == 0, which shouldn't happen
			// as we don't shuffle cards when no card is available.
			_cardsAvailableNumb = i - 1;
			_currentDiscardIndex = 98;  // CHECKME: Would make more sense at pos 99
			break;
		}
	}

	for (int i = 0; i < 2000; i ++) {
		int randIndx = R2_GLOBALS._randomSource.getRandomNumber(_cardsAvailableNumb);
		int swap = _availableCardsPile[0];
		_availableCardsPile[0] = _availableCardsPile[randIndx];
		_availableCardsPile[randIndx] = swap;
	}

	_shuffleEndedFl = false;

	// Shuffle cards
	_animatedCard._card.setAction(&_action2);

	while(!_shuffleEndedFl && !g_vm->shouldQuit()) {
		g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
		g_globals->_scenePalette.signalListeners();
		R2_GLOBALS._sceneObjects->draw();
		g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
	}
}

void Scene1337::dealCards() {
	_animatedCard._card._moveDiff = Common::Point(30, 30);
	shuffleCards();

	// Deal cards
	_actionItem.setAction(&_action3);
}

void Scene1337::showOptionsDialog() {
	// Display menu with "Auto Play", "New Game", "Quit" and "Continue"
	OptionsDialog::show();
}

void Scene1337::handleClick(int arg1, Common::Point pt) {
	int curReg = R2_GLOBALS._sceneRegions.indexOf(g_globals->_events._mousePos);

	if (arg1 == 3) {
		bool found = false;
		int i;
		for (i = 0; i <= 7; i++) {
			if ( _gameBoardSide[2]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[0]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[1]._outpostStation[i].isIn(pt)
			  || _gameBoardSide[3]._outpostStation[i].isIn(pt) ) {
				found = true;
				break;
			}
		}

		if (found) {
			switch (curReg) {
			case 5:
				if (_gameBoardSide[2]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[2]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 20, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				if (_gameBoardSide[3]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[3]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 22, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				if (_gameBoardSide[0]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[0]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 21, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				if (_gameBoardSide[1]._outpostStation[i]._cardId != 0)
					displayDialog(_gameBoardSide[1]._outpostStation[i]._cardId);
				else
					actionDisplay(1330, 23, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		} else if ( _gameBoardSide[2]._delayCard.isIn(pt)
			  || _gameBoardSide[0]._delayCard.isIn(pt)
			  || _gameBoardSide[1]._delayCard.isIn(pt)
			  || _gameBoardSide[3]._delayCard.isIn(pt) ) {
			switch (curReg) {
			case 5:
				if (_gameBoardSide[2]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[2]._delayCard._cardId);
				else
					actionDisplay(1330, 10, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				if (_gameBoardSide[3]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[3]._delayCard._cardId);
				else
					actionDisplay(1330, 16, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				if (_gameBoardSide[0]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[0]._delayCard._cardId);
				else
					actionDisplay(1330, 13, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				if (_gameBoardSide[1]._delayCard._cardId != 0)
					displayDialog(_gameBoardSide[1]._delayCard._cardId);
				else
					actionDisplay(1330, 18, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		} else if (_discardPile.isIn(pt)) {
			if (_discardPile._cardId != 0)
				displayDialog(_discardPile._cardId);
			else
				actionDisplay(1330, 7, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		} else if (_helpIcon._bounds.contains(pt))
			actionDisplay(1330, 43, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if (_stockCard.isIn(pt))
			actionDisplay(1330, 4, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ( (_gameBoardSide[2]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[3]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[0]._emptyStationPos.isIn(pt))
			     || (_gameBoardSide[1]._emptyStationPos.isIn(pt)) )
			actionDisplay(1330, 32, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if (_gameBoardSide[2]._handCard[0].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[0]._cardId);
		else if (_gameBoardSide[2]._handCard[1].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[1]._cardId);
		else if (_gameBoardSide[2]._handCard[2].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[2]._cardId);
		else if (_gameBoardSide[2]._handCard[3].isIn(pt))
			displayDialog(_gameBoardSide[2]._handCard[3]._cardId);
		else if ((curReg >= 6) && (curReg <= 9))
			actionDisplay(1330, 29, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ((curReg >= 11) && (curReg <= 14))
			actionDisplay(1330, 31, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else if ((curReg >= 16) && (curReg <= 19))
			actionDisplay(1330, 30, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else {
			switch (curReg) {
			case 0:
				actionDisplay(1330, 2, 159, 134, 1, 200, 0, 7, 0, 105, 105);
				break;
			case 5:
				actionDisplay(1330, 25, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 10:
				actionDisplay(1330, 27, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 15:
				actionDisplay(1330, 26, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 20:
				actionDisplay(1330, 28, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 21:
				actionDisplay(1330, 24, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				break;
			}
		}
	}

	if (arg1 != 1)
		return;

	for (int i = 0; i <= 7; i++) {
		if (_gameBoardSide[2]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[2]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			case 1:
				actionDisplay(1330, 54, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				actionDisplay(1330, 34, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			}
			return;
		}
		if (_gameBoardSide[0]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[0]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			default:
				actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				break;
			}
			return;
		}
		if (_gameBoardSide[1]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[1]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
				break;
			default:
				actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
				break;
			}
			return;
		}
		if (_gameBoardSide[3]._outpostStation[i].isIn(pt)) {
			switch (_gameBoardSide[3]._outpostStation[i]._cardId) {
			case 0:
				actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
				break;
			default:
				actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
				break;
			}
			return;
		}
	}

	if (_gameBoardSide[2]._delayCard.isIn(pt)) {
		// The original uses _gameBoardSide[0], which is obviously a bug.
		if (_gameBoardSide[2]._delayCard._cardId != 0)
			actionDisplay(1330, 39, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		return;
	}
	if (_gameBoardSide[3]._delayCard.isIn(pt)) {
		if (_gameBoardSide[3]._delayCard._cardId != 0)
			actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		else
			actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);

		return;
	}
	if (_gameBoardSide[1]._delayCard.isIn(pt)) {
		if (_gameBoardSide[1]._delayCard._cardId != 0)
			actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		else
			actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);

		return;
	}
	if (_gameBoardSide[0]._delayCard.isIn(pt)) {
		if (_gameBoardSide[0]._delayCard._cardId != 0)
			actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		else
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);

		return;
	}
	if (_gameBoardSide[3]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
		return;
	}
	if (_gameBoardSide[1]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
		return;
	}
	if (_gameBoardSide[0]._emptyStationPos.isIn(pt)) {
		actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
		return;
	}

	if (_helpIcon._bounds.contains(pt)) {
		showOptionsDialog();
		return;
	}

	if (_discardPile.isIn(pt))
		actionDisplay(1330, 9, 159, 10, 1, 200, 0, 7, 0, 154, 154);
	else if (_stockCard.isIn(pt))
		actionDisplay(1330, 5, 159, 10, 1, 200, 0, 7, 0, 154, 154);
	else {
		switch (curReg) {
		case 0:
			actionDisplay(1330, 3, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		case 6:
		// no break on purpose
		case 7:
		// no break on purpose
		case 8:
		// no break on purpose
		case 9:
			actionDisplay(1330, 145, 20, 99, 1, 136, 0, 7, 0, 172, 172);
			break;
		case 10:
			actionDisplay(1330, 147, 20, 99, 1, 136, 0, 7, 0, 172, 172);
			break;
		case 11:
		// no break on purpose
		case 12:
		// no break on purpose
		case 13:
		// no break on purpose
		case 14:
			actionDisplay(1330, 1, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		case 16:
		// no break on purpose
		case 17:
		// no break on purpose
		case 18:
		// no break on purpose
		case 19:
			actionDisplay(1330, 144, 300, 99, 1, 136, 0, 7, 0, 117, 117);
			break;
		case 20:
			actionDisplay(1330, 146, 300, 99, 1, 136, 0, 7, 0, 117, 117);
			break;
		default:
			actionDisplay(1330, 11, 159, 10, 1, 200, 0, 7, 0, 154, 154);
			break;
		}
	}
}

void Scene1337::handlePlayer0() {
	if (_gameBoardSide[0]._delayCard._cardId != 0) {
		switch (_gameBoardSide[0]._delayCard._cardId) {
		case 10:
		//No break on purpose
		case 12:
		//No break on purpose
		case 15:
		//No break on purpose
		case 17:
		//No break on purpose
		case 18:
		//No break on purpose
		case 19:
		//No break on purpose
		case 20:
		//No break on purpose
		case 21:
			discardCard(&_gameBoardSide[0]._delayCard);
			break;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[0]._delayCard._cardId, _gameBoardSide[0]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[0]._delayCard);
					return;
				}
			}

			break;
		}
	}

	for (int i = 0; i <= 3; i++) {
		int tmpVal = getStationId(0, i);

		if (tmpVal != -1) {
			bool stationAlreadyPresentFl = false;
			for (int j = 0; j <= 7; j++) {
				if (_gameBoardSide[0]._outpostStation[j]._cardId == _gameBoardSide[0]._handCard[tmpVal]._cardId) {
					stationAlreadyPresentFl = true;
					break;
				}
			}

			if (!stationAlreadyPresentFl) {
				for (int j = 0; j <= 7; j++) {
					if ((_gameBoardSide[0]._outpostStation[j]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[0]._delayCard._cardId)) {
						int stationCount = 0;
						for (int k = 0; k <= 7; k++) {
							if ((_gameBoardSide[0]._outpostStation[k]._cardId > 1) && (_gameBoardSide[0]._outpostStation[k]._cardId <= 9)) {
								++stationCount;
							}
						}

						if (stationCount == 7)
							_winnerId = 0;

						playStationCard(&_gameBoardSide[0]._handCard[tmpVal], &_gameBoardSide[0]._outpostStation[j]);
						return;
					}
				}
			}
		}
	}

	int tmpVal = findPlatformCardInHand(0);

	if (tmpVal != -1) {
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[0]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[0]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[0]._handCard[tmpVal], &_gameBoardSide[0]._outpostStation[i]);
				return;
			}
		}
	}

	int card13Id = findCard13InHand(0);
	if (card13Id != -1) {
		for (int i = 0; i <= 7; i++) {
			if (_gameBoardSide[2]._outpostStation[i]._cardId != 0) {
				playCounterTrickCard(&_gameBoardSide[0]._handCard[card13Id], 2);
				return;
			}
		}
	}

	int thieftId = checkThieftCard(0);
	if (thieftId != -1) {
		if ( (_gameBoardSide[2]._handCard[0]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[1]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[2]._cardId != 0)
		  || (_gameBoardSide[2]._handCard[3]._cardId != 0) ) {
			playThieftCard(0, &_gameBoardSide[0]._handCard[thieftId], 2);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if ((isDelayCard(_gameBoardSide[0]._handCard[i]._cardId) != -1)
		&&  (_gameBoardSide[2]._delayCard._cardId == 0)
		&&  isAttackPossible(2, _gameBoardSide[0]._handCard[i]._cardId)) {
			playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[2]._delayCard);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if ((getStationCardId(_gameBoardSide[0]._handCard[i]._cardId) != -1)
		&&  (_gameBoardSide[2]._delayCard._cardId == 0)
		&& isAttackPossible(2, _gameBoardSide[0]._handCard[i]._cardId)) {
			playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[2]._delayCard);
			return;
		}
	}

	card13Id = findCard13InHand(0);
	int victimPlayerId = getPlayerWithOutpost(0);

	if ((card13Id != -1) && (victimPlayerId != -1)) {
		playCounterTrickCard(&_gameBoardSide[0]._handCard[card13Id], victimPlayerId);
		return;
	}

	thieftId = checkThieftCard(0);
	if (thieftId != -1) {
		if ( (_gameBoardSide[1]._handCard[0]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[1]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[2]._cardId != 0)
		  || (_gameBoardSide[1]._handCard[3]._cardId != 0) ) {
			playThieftCard(0, &_gameBoardSide[0]._handCard[thieftId], 1);
			return;
		}
	}

	for (int i = 0; i <= 3; i++) {
		if (getStationCardId(_gameBoardSide[0]._handCard[i]._cardId) != -1) {
			if ((_gameBoardSide[1]._delayCard._cardId == 0) && isAttackPossible(1, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[1]._delayCard);
				return;
			}

			if ((_gameBoardSide[3]._delayCard._cardId == 0) && isAttackPossible(3, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[3]._delayCard);
				return;
			}
		}
	}

	for (int i = 0; i <= 3; i++) {
		tmpVal = isDelayCard(_gameBoardSide[0]._handCard[i]._cardId);
		if (tmpVal != -1) {
			if ((_gameBoardSide[1]._delayCard._cardId == 0) && isAttackPossible(1, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[1]._delayCard);
				return;
			}

			if ((_gameBoardSide[3]._delayCard._cardId == 0) && isAttackPossible(3, _gameBoardSide[0]._handCard[i]._cardId)) {
				playDelayCard(&_gameBoardSide[0]._handCard[i], &_gameBoardSide[3]._delayCard);
				return;
			}
		}
	}

	handlePlayer01Discard(0);
}

void Scene1337::handlePlayer1() {
	if (this->_gameBoardSide[1]._delayCard._cardId != 0) {
		switch (_gameBoardSide[1]._delayCard._cardId) {
		case 10:
			// No break on purpose
		case 12:
			// No break on purpose
		case 15:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			discardCard(&_gameBoardSide[1]._delayCard);
			return;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[1]._delayCard._cardId, _gameBoardSide[1]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[1]._handCard[i], &_gameBoardSide[1]._delayCard);
					return;
				}
			}
			break;
		}
	}

	for (int i = 0; i <= 3; i++) {
		int tmpIndx = getStationId(1, i);
		if (tmpIndx == -1)
			break;

		int tmpVal = 0;
		for (int j = 0; j <= 7; j++) {
			if (_gameBoardSide[1]._outpostStation[j]._cardId == _gameBoardSide[1]._handCard[tmpIndx]._cardId) {
				tmpVal = 1;
				break;
			}
		}

		if (tmpVal == 0)
			break;

		for (int j = 0; j <= 7; j++) {
			if ((_gameBoardSide[1]._outpostStation[j]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[1]._delayCard._cardId)) {
				int stationCount = 0;
				for (int k = 0; k <= 7; k++) {
					if ((_gameBoardSide[1]._outpostStation[k]._cardId > 1) && (_gameBoardSide[1]._outpostStation[k]._cardId <= 9))
						++stationCount;
				}

				if (stationCount == 7)
					_winnerId = 1;

				playStationCard(&_gameBoardSide[1]._handCard[tmpIndx], &_gameBoardSide[1]._outpostStation[j]);
				return;
			}
		}
	}

	int normalCardId = findPlatformCardInHand(1);
	if (normalCardId != -1) {
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[1]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[1]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[1]._handCard[normalCardId], &_gameBoardSide[1]._outpostStation[i]);
				return;
			}
		}
	}

	int card13Id = findCard13InHand(1);
	int tmpVal2 = getPlayerWithOutpost(1);

	if ((card13Id != -1) && (tmpVal2 != -1)) {
		playCounterTrickCard(&_gameBoardSide[1]._handCard[card13Id], tmpVal2);
		return;
	}

	int thieftId = checkThieftCard(1);
	if (thieftId != -1) {
		int playerIdFound = -1;
		int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);
		for (int i = 0; i <= 3; i++) {
			if (rndVal != 1) {
				if (  (_gameBoardSide[rndVal]._handCard[0]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[1]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[2]._cardId != 0)
					|| (_gameBoardSide[rndVal]._handCard[3]._cardId == 0)) {
						playerIdFound = rndVal;
						break;
				}
			}
			// The original was only updating in the rndVal block,
			// which was a bug as the checks were stopping at this point
			rndVal--;
			if (rndVal < 0)
				rndVal = 3;
		}

		if (playerIdFound != -1) {
			playThieftCard(1, &_gameBoardSide[1]._handCard[thieftId], playerIdFound);
			return;
		}
	}

	int count = -1;
	int i;
	for (i = 0; i <= 3; i++) {
		int tmpVal = isDelayCard(_gameBoardSide[1]._handCard[i]._cardId);
		if (tmpVal != -1) {
			int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);

			for (int j = 0; j <= 3; j++) {
				//CHECKME: tmpVal or rndVal?
				// FIXME: This is probably meant to be rndVal, but not clear...
				if (tmpVal < 0 || tmpVal >= ARRAYSIZE(_gameBoardSide))
					error("Scene1337::handlePlayer1() tmpVal:%d out of range 0 to %d", tmpVal, ARRAYSIZE(_gameBoardSide)-1);

				if (tmpVal != 1) {
					if ((_gameBoardSide[tmpVal]._delayCard._cardId == 0) && isAttackPossible(tmpVal, _gameBoardSide[1]._handCard[i]._cardId))
						count = tmpVal;
				}

				if (count < 0 || count >= ARRAYSIZE(_gameBoardSide))
					error("Scene1337::handlePlayer1() count:%d out of range 0 to %d", count, ARRAYSIZE(_gameBoardSide)-1);

				if (count != -1) {
					playDelayCard(&_gameBoardSide[1]._handCard[i], &_gameBoardSide[count]._delayCard);
					return;
				} else {
					rndVal--;
					if (rndVal < 0)
						rndVal = 3;
				}
			}
		}
	}

	int j;
	for (j = 0; j <= 3; j++) {
		if (getStationCardId(_gameBoardSide[1]._handCard[j]._cardId) != -1) {
			count = -1;
			int rndVal = R2_GLOBALS._randomSource.getRandomNumber(3);
			for (int l = 0; l <= 3; l++) {
				if (rndVal != 1) {
					if ((_gameBoardSide[rndVal]._delayCard._cardId == 0) && (_gameBoardSide[1]._handCard[j]._cardId == 1))
						count = rndVal;
				}
				if (count != -1) {
					playDelayCard(&_gameBoardSide[1]._handCard[j], &_gameBoardSide[count]._delayCard);
					return;
				} else {
					rndVal--;
					if (rndVal < 0)
						rndVal = 3;
				}
			}
		}
	}

	handlePlayer01Discard(1);
}

void Scene1337::handlePlayer3() {
	if (_gameBoardSide[3]._delayCard._cardId != 0) {
		switch (_gameBoardSide[3]._delayCard._cardId) {
		case 10:
			// No break on purpose
		case 12:
			// No break on purpose
		case 15:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			discardCard(&_gameBoardSide[3]._delayCard);
			return;
		default:
			for (int i = 0; i <= 3; i++) {
				if (checkAntiDelayCard(_gameBoardSide[3]._delayCard._cardId, _gameBoardSide[3]._handCard[i]._cardId)) {
					playAntiDelayCard(&_gameBoardSide[3]._handCard[i], &_gameBoardSide[3]._delayCard);
					return;
				}
			}
			break;
		}
	}

	int randIndx = R2_GLOBALS._randomSource.getRandomNumber(3);

	if (_gameBoardSide[3]._handCard[randIndx]._cardId == 1) {
		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[3]._outpostStation[i]._cardId == 0) && !isStopConstructionCard(_gameBoardSide[3]._delayCard._cardId)) {
				playPlatformCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[3]._outpostStation[i]);
				return;
			}
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId <= 9) {
		for (int i = 0; i <= 7; i++) {
			if (_gameBoardSide[3]._outpostStation[i]._cardId == _gameBoardSide[3]._handCard[randIndx]._cardId) {
				discardCard(&_gameBoardSide[3]._handCard[randIndx]);
				return;
			}
		}

		for (int i = 0; i <= 7; i++) {
			if ((_gameBoardSide[3]._outpostStation[i]._cardId == 1) && !isStopConstructionCard(_gameBoardSide[3]._delayCard._cardId)) {
				int stationCount = 0;
				for (int j = 0; j <= 7; j++) {
					if ((_gameBoardSide[3]._outpostStation[j]._cardId > 1) && (_gameBoardSide[3]._outpostStation[j]._cardId <= 9))
						++stationCount;
				}

				if (stationCount == 7)
					_winnerId = 3;

				playStationCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[3]._outpostStation[i]);
				return;
			}
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId == 13) {
		int victimId = getPlayerWithOutpost(3);

		if (victimId != -1) {
			playCounterTrickCard(&_gameBoardSide[3]._handCard[randIndx], victimId);
			return;
		}
	} else if (_gameBoardSide[3]._handCard[randIndx]._cardId == 25) {
		int victimId = -1;
		int tmpRandIndx = R2_GLOBALS._randomSource.getRandomNumber(3);

		for (int i = 0; i <= 3; i++) {
			if (  (tmpRandIndx != 3)
				&& (  (_gameBoardSide[tmpRandIndx]._handCard[0]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[1]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[2]._cardId != 0)
				|| (_gameBoardSide[tmpRandIndx]._handCard[3]._cardId != 0) )) {
					victimId = tmpRandIndx;
					break;
			}

			++tmpRandIndx;
			if (tmpRandIndx > 3)
				tmpRandIndx = 0;
		}

		if (victimId != -1) {
			playThieftCard(3, &_gameBoardSide[3]._handCard[randIndx], victimId);
			return;
		}
	} else {
		switch (_gameBoardSide[3]._handCard[randIndx]._cardId) {
		case 10:
			// No break on purpose
		case 11:
			// No break on purpose
		case 12:
			// No break on purpose
		case 14:
			// No break on purpose
		case 15:
			// No break on purpose
		case 16:
			// No break on purpose
		case 17:
			// No break on purpose
		case 18:
			// No break on purpose
		case 19:
			// No break on purpose
		case 20:
			// No break on purpose
		case 21:
			// No break on purpose
		case 24: {
			int victimId = -1;
			int tmpRandIndx = R2_GLOBALS._randomSource.getRandomNumber(3);

			for (int i = 0; i <= 3; i++) {
				if (tmpRandIndx != 3) {
					if ((_gameBoardSide[tmpRandIndx]._delayCard._cardId == 0)
					&&  isAttackPossible(tmpRandIndx, _gameBoardSide[3]._handCard[randIndx]._cardId))
						victimId = tmpRandIndx;
				}

				++tmpRandIndx;
				if (tmpRandIndx > 3)
					tmpRandIndx = 0;

				if (victimId != -1)
					break;
			}

			if (victimId != -1) {
				// Useless second identical check skipped
				playDelayCard(&_gameBoardSide[3]._handCard[randIndx], &_gameBoardSide[victimId]._delayCard);
				return;
			}
				 }
		default:
			break;
		}
	}

	discardCard(&_gameBoardSide[3]._handCard[randIndx]);
}

void Scene1337::handleAutoplayPlayer2() {
	if (getStationCardId(this->_gameBoardSide[2]._delayCard._cardId) == -1)
		_delayedFunction = &Scene1337::handlePlayer2;
	else
		discardCard(&_gameBoardSide[2]._delayCard);
}

void Scene1337::handlePlayer2() {
	_selectedCard._stationPos = g_globals->_events._mousePos;

	if (R2_GLOBALS._v57810 == 200) {
		// Hand
		int i;
		for (i = 0; i < 4; i++) {
			if ((_gameBoardSide[2]._handCard[i].isIn(_selectedCard._stationPos)) && (_gameBoardSide[2]._handCard[i]._cardId != 0)) {
				Card *handcard = &_gameBoardSide[2]._handCard[i];
				_selectedCard._cardId = handcard->_cardId;
				_selectedCard._stationPos = handcard->_stationPos;
				//warning("_selectedCard._actorName = handcard->_actorName;");
				//warning("_selectedCard._fieldE = handcard->_fieldE;");
				//warning("_selectedCard._field10 = handcard->_field10;");
				//warning("_selectedCard._field12 = handcard->_field12;");
				//warning("_selectedCard._field14 = handcard->_field14;");
				//warning("_selectedCard._field16 = handcard->_field16;");
				_selectedCard._sceneRegionId = handcard->_sceneRegionId;
				_selectedCard._position = handcard->_position;
				_selectedCard._yDiff = handcard->_yDiff;
				_selectedCard._bounds = handcard->_bounds;
				_selectedCard._resNum = handcard->_resNum;
				_selectedCard._lookLineNum = handcard->_lookLineNum;
				_selectedCard._talkLineNum = handcard->_talkLineNum;
				_selectedCard._useLineNum = handcard->_useLineNum;
				_selectedCard._action = handcard->_action;
				//warning("_selectedCard._field0 = handcard->_field0;");
				_selectedCard._card._updateStartFrame = handcard->_card._updateStartFrame;
				_selectedCard._card._walkStartFrame = handcard->_card._walkStartFrame;
				// _field2E is named _field3C in R2R
				_selectedCard._card._oldPosition = handcard->_card._oldPosition;
				_selectedCard._card._percent = handcard->_card._percent;
				_selectedCard._card._priority = handcard->_card._priority;
				_selectedCard._card._angle = handcard->_card._angle;
				_selectedCard._card._flags = handcard->_card._flags;
				_selectedCard._card._xe = handcard->_card._xe;
				_selectedCard._card._xs = handcard->_card._xs;
				_selectedCard._card._paneRects[0] = handcard->_card._paneRects[0];
				_selectedCard._card._paneRects[1] = handcard->_card._paneRects[1];
				_selectedCard._card._visage = handcard->_card._visage;
				_selectedCard._card._objectWrapper = handcard->_card._objectWrapper;
				_selectedCard._card._strip = handcard->_card._strip;
				_selectedCard._card._animateMode = handcard->_card._animateMode;
				_selectedCard._card._frame = handcard->_card._frame;
				_selectedCard._card._endFrame = handcard->_card._endFrame;
				// _field68 is named _field76 in R2R
				_selectedCard._card._loopCount = handcard->_card._loopCount;
				_selectedCard._card._frameChange = handcard->_card._frameChange;
				_selectedCard._card._numFrames = handcard->_card._numFrames;
				_selectedCard._card._regionIndex = handcard->_card._regionIndex;
				_selectedCard._card._mover = handcard->_card._mover;
				_selectedCard._card._moveDiff = handcard->_card._moveDiff;
				_selectedCard._card._moveRate = handcard->_card._moveRate;
				_selectedCard._card._actorDestPos = handcard->_card._actorDestPos;
				_selectedCard._card._endAction = handcard->_card._endAction;
				_selectedCard._card._regionBitList = handcard->_card._regionBitList;
				// _selectedCard._object1._actorName = handcard->_object1._actorName;
				//warning("_selectedCard._card._fieldE = handcard->_card._fieldE;");
				//warning("_selectedCard._card._field10 = handcard->_card._field10;");
				//warning("_selectedCard._card._field12 = handcard->_card._field12;");
				//warning("_selectedCard._card._field14 = handcard->_card._field14;");
				//warning("_selectedCard._card._field16 = handcard->_card._field16;");

				_gameBoardSide[2]._handCard[i]._cardId = 0;
				_gameBoardSide[2]._handCard[i]._card.remove();
				break;
			}
		}

		if (i == 4) {
			handleClick(1, _selectedCard._stationPos);
			handleAutoplayPlayer2();
			return;
		} else {
			setCursorData(1332, _selectedCard._card._strip, _selectedCard._card._frame);
			R2_GLOBALS._sceneObjects->draw();
		}
	} else if (R2_GLOBALS._v57810 == 300) {
		// Eye
		handleClick(3, _selectedCard._stationPos);
		handleAutoplayPlayer2();
		return;
	} else {
		// The original code is calling a function full of dead code.
		// Only this message remains after a cleanup.
		MessageDialog::show(WRONG_ANSWER_MSG, OK_BTN_STRING);
		//
		handleAutoplayPlayer2();
		return;
	}

	Event event;
	bool found;
	for (;;) {
		if ( ((g_globals->_events.getEvent(event, EVENT_BUTTON_DOWN)) && (event.btnState == BTNSHIFT_RIGHT))
			|| (g_globals->_events.getEvent(event, EVENT_KEYPRESS)) ){
			_selectedCard._stationPos = g_globals->_events._mousePos;
			found = false;

			for (int i = 0; i <= 3; i ++) {
				if (_gameBoardSide[2]._handCard[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[2]._handCard[i]._cardId == 0) {
						_gameBoardSide[2]._handCard[i]._cardId = _selectedCard._cardId;
						_gameBoardSide[2]._handCard[i]._card.postInit();
						_gameBoardSide[2]._handCard[i]._card.hide();
						_gameBoardSide[2]._handCard[i]._card.setVisage(1332);
						_gameBoardSide[2]._handCard[i]._card.setPosition(_gameBoardSide[2]._handCard[i]._stationPos, 0);
						_gameBoardSide[2]._handCard[i]._card.fixPriority(170);
						setAnimationInfo(&_gameBoardSide[2]._handCard[i]);
						setCursorData(5, 1, 4);
						_currentPlayerNumb--;
						_showPlayerTurn = false;
						handleNextTurn();
						return;
					} else {
						actionDisplay(1330, 127, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						found = true;
					}
					break;
				}
			}

			if (!found) {
				if (_discardPile.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					discardCard(&_selectedCard);
					return;
				} else if (_selectedCard._cardId == 1) {
					bool isInCardFl = false;
					int i;
					for (i = 0; i <= 7; i++) {
						if (_gameBoardSide[2]._outpostStation[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							isInCardFl = true;
							break;
						}
					}

					if ((isInCardFl) && (_gameBoardSide[2]._outpostStation[i]._cardId == 0)) {
						if (isDelayCard(_gameBoardSide[2]._delayCard._cardId) != -1) {
							actionDisplay(1330, 55, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							playPlatformCard(&_selectedCard, &_gameBoardSide[2]._outpostStation[i]);
							return;
						}
					} else {
						actionDisplay(1330, 56, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if (_selectedCard._cardId <= 9) {
					bool isInCardFl = false;
					int i;
					for (i = 0; i <= 7; i++) {
						if (_gameBoardSide[2]._outpostStation[i].isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							isInCardFl = true;
							break;
						}
					}
					if ((isInCardFl) && (_gameBoardSide[2]._outpostStation[i]._cardId == 1)) {
						isInCardFl = false;
						for (int j = 0; j <= 7; j++) {
							if (_selectedCard._cardId == _gameBoardSide[2]._outpostStation[j]._cardId) {
								isInCardFl = true;
								break;
							}
						}
						if (isInCardFl) {
							// This station is already in place
							actionDisplay(1330, 34, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (isDelayCard(_gameBoardSide[2]._delayCard._cardId) != -1) {
							// You must eliminate your delay before you can play a station
							actionDisplay(1330, 35, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							int stationCount = 0;
							for (int k = 0; k <= 7; k++) {
								if ((_gameBoardSide[2]._outpostStation[k]._cardId > 1) && (_gameBoardSide[2]._outpostStation[k]._cardId <= 9))
									++stationCount;
							}

							if (stationCount == 7)
								_winnerId = 2;

							playStationCard(&_selectedCard, &_gameBoardSide[2]._outpostStation[i]);
							return;
						}
					} else {
						actionDisplay(1330, 37, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if ((_selectedCard._cardId == 26) || (_selectedCard._cardId == 30) ||(_selectedCard._cardId == 32) || (_selectedCard._cardId == 28)) {
					// Check anti-delay card
					if (_gameBoardSide[2]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
						actionDisplay(1330, 42, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (checkAntiDelayCard(_gameBoardSide[2]._delayCard._cardId, _selectedCard._cardId)) {
						playAntiDelayCard(&_selectedCard, &_gameBoardSide[2]._delayCard);
						return;
					} else {
						if (_gameBoardSide[2]._delayCard._cardId != 0) {
							switch (_gameBoardSide[2]._delayCard._cardId) {
							case 11:
								actionDisplay(1330, 68, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 14:
								actionDisplay(1330, 80, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 16:
								actionDisplay(1330, 84, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							case 24:
								actionDisplay(1330, 96, 159, 10, 1, 200, 0, 7, 0, 154, 154);
								break;
							default:
								break;
							}
						} else {
							actionDisplay(1330, 41, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					}
				} else if ((getStationCardId(_selectedCard._cardId) == -1) && (isDelayCard(_selectedCard._cardId) == -1)) {
					if (_selectedCard._cardId == 13) {
						if (_gameBoardSide[0]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[0]._outpostStation[k]._cardId != 0) {
									playCounterTrickCard(&_selectedCard, 0);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (_gameBoardSide[3]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[3]._outpostStation[k]._cardId != 0) {
									playCounterTrickCard(&_selectedCard, 3);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else if (_gameBoardSide[1]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							for (int k = 0; k <= 7; k++) {
								if (_gameBoardSide[1]._outpostStation[k]._cardId == 0) {
									playCounterTrickCard(&_selectedCard, 1);
									return;
								}
							}
							actionDisplay(1330, 74, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						} else {
							actionDisplay(1330, 128, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					} else if (_selectedCard._cardId == 25) {
						if (_gameBoardSide[0]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[0]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[0]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 0);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						} else if (_gameBoardSide[1]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[1]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[1]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 1);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						}

						if (_gameBoardSide[3]._emptyStationPos.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
							if ( (_gameBoardSide[3]._handCard[0]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[1]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[2]._cardId != 0)
								|| (_gameBoardSide[3]._handCard[3]._cardId != 0) ) {
									int k;
									for (k = 0; k <= 3; k++){
										if (_gameBoardSide[2]._handCard[k]._cardId == 0)
											break;
									}
									playThieftCard(2, &_gameBoardSide[2]._handCard[k], 3);
									return;
							} else {
								actionDisplay(1330, 99, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							}
						} else {
							actionDisplay(1330, 129, 159, 10, 1, 200, 0, 7, 0, 154, 154);
						}
					} else if (_selectedCard._cardId == 29) {
						// Interceptor cards are used to prevent collision
						actionDisplay(1330, 136, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (_selectedCard._cardId == 27) {
						actionDisplay(1330, 137, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					}
				} else if (_gameBoardSide[0]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[0]._delayCard._cardId != 0) {
						actionDisplay(1330, 15, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(0, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[0]._delayCard);
						return;
					}
				} else if (_gameBoardSide[3]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[3]._delayCard._cardId != 0) {
						actionDisplay(1330, 17, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(3, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[3]._delayCard);
						return;
					}
				} else if (_gameBoardSide[1]._delayCard.isIn(Common::Point(_selectedCard._stationPos.x + 12, _selectedCard._stationPos.y + 12))) {
					if (_gameBoardSide[1]._delayCard._cardId != 0) {
						actionDisplay(1330, 19, 159, 10, 1, 200, 0, 7, 0, 154, 154);
					} else if (!isAttackPossible(1, _selectedCard._cardId)) {
						switch (_selectedCard._cardId) {
						case 10:
							actionDisplay(1330, 66, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 12:
							actionDisplay(1330, 70, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 15:
							actionDisplay(1330, 82, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 17:
							actionDisplay(1330, 86, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 18:
							actionDisplay(1330, 88, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 19:
							actionDisplay(1330, 90, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 20:
							actionDisplay(1330, 92, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						case 21:
							actionDisplay(1330, 94, 159, 10, 1, 200, 0, 7, 0, 154, 154);
							break;
						default:
							break;
						}
					} else {
						playDelayCard(&_selectedCard, &_gameBoardSide[1]._delayCard);
						return;
					}
				} else {
					actionDisplay(1330, 38, 159, 10, 1, 200, 0, 7, 0, 154, 154);
				}
			}
		} else {
			g_globals->_scenePalette.signalListeners();
			R2_GLOBALS._sceneObjects->draw();
			g_globals->_events.delay(g_globals->_sceneHandler->_delayTicks);
		}

		g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
	}
}

void Scene1337::updateCursorId(int cursorId, bool updateFl) {
	if ((R2_GLOBALS._v57709 != 0) || (R2_GLOBALS._v5780C != 0))
		return;

	R2_GLOBALS._mouseCursorId = cursorId;

	if (updateFl) {
		R2_GLOBALS._mouseCursorId++;

		if (R2_GLOBALS._mouseCursorId < 1)
			R2_GLOBALS._mouseCursorId = 2;

		if (R2_GLOBALS._mouseCursorId > 2)
			R2_GLOBALS._mouseCursorId = 1;
	}

	// The original was using an intermediate function to call setCursorData.
	// It has been removed to improve readability
	if (R2_GLOBALS._mouseCursorId == 1) {
		R2_GLOBALS._v57810 = 200;
		setCursorData(5, 1, 4);
	} else if (R2_GLOBALS._mouseCursorId == 2) {
		R2_GLOBALS._v57810 = 300;
		setCursorData(5, 1, 5);
	} else {
		R2_GLOBALS._v57810 = 0;
		setCursorData(5, 0, 0);
	}
}

void Scene1337::setCursorData(int resNum, int rlbNum, int frameNum) {
	_cursorCurRes = resNum;
	_cursorCurStrip = rlbNum;
	_cursorCurFrame = frameNum;

	if (!frameNum) {
		// Should be a hardcoded cursor displaying only a dot.
		// FIXME: Use another cursor when possible
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
	} else {
		// TODO: The original was using some ressource caching, which was useless and complex
		// and which has been removed. This cursor behavior clearly made intensive use of this caching...
		// We now have to find a way to cache these cursor pointers and avoid loading them multiple times per seconds
		uint size;
		byte *cursor = g_resourceManager->getSubResource(resNum, rlbNum, frameNum, &size);
		// Decode the cursor
		GfxSurface s = surfaceFromRes(cursor);

		Graphics::Surface surface = s.lockSurface();
		const byte *cursorData = (const byte *)surface.getPixels();
		CursorMan.replaceCursor(cursorData, surface.w, surface.h, s._centroid.x, s._centroid.y, s._transColor);
		s.unlockSurface();

		DEALLOCATE(cursor);
	}
}

void Scene1337::subD18F5() {
	if (R2_GLOBALS._v57709 == 0)
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

	++R2_GLOBALS._v57709;
}

void Scene1337::subD1917() {
	if (R2_GLOBALS._v57709 != 0) {
		R2_GLOBALS._v57709--;
		if (R2_GLOBALS._v57709 != 0) {
			// The original was using an intermediate function to call setCursorData.
			// It has been removed to improve readability
			setCursorData(5, _cursorCurStrip, _cursorCurFrame);
		}
	}
}

void Scene1337::subD1940(bool flag) {
	if (flag)
		++R2_GLOBALS._v5780C;
	else if (R2_GLOBALS._v5780C != 0)
		--R2_GLOBALS._v5780C;
}

void Scene1337::subD1975(int arg1, int arg2) {
	warning("STUBBED lvl2 Scene1337::subD1975()");
}

void Scene1337::OptionsDialog::show() {
	OptionsDialog *dlg = new OptionsDialog();
	dlg->draw();

	// Show the dialog
	GfxButton *btn = dlg->execute(NULL);

	// Figure out the new selected character
	if (btn == &dlg->_quitGame)
		R2_GLOBALS._sceneManager.changeScene(125);
	else if (btn == &dlg->_restartGame)
		R2_GLOBALS._sceneManager.changeScene(1330);

	// Remove the dialog
	dlg->remove();
	delete dlg;
}

Scene1337::OptionsDialog::OptionsDialog() {
	// Set the elements text
	Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;
	_autoplay.setText(scene->_autoplay ? AUTO_PLAY_ON : AUTO_PLAY_OFF);
	_restartGame.setText(START_NEW_CARD_GAME);
	_quitGame.setText(QUIT_CARD_GAME);
	_continueGame.setText(CONTINUE_CARD_GAME);

	// Set position of the elements
	_autoplay._bounds.moveTo(5, 2);
	_restartGame._bounds.moveTo(5, _autoplay._bounds.bottom + 2);
	_quitGame._bounds.moveTo(5, _restartGame._bounds.bottom + 2);
	_continueGame._bounds.moveTo(5, _quitGame._bounds.bottom + 2);

	// Add the items to the dialog
	addElements(&_autoplay, &_restartGame, &_quitGame, &_continueGame, NULL);

	// Set the dialog size and position
	frame();
	_bounds.collapse(-6, -6);
	setCenter(160, 100);
}

GfxButton *Scene1337::OptionsDialog::execute(GfxButton *defaultButton) {
	_gfxManager.activate();

	// Event loop
	GfxButton *selectedButton = NULL;

	bool breakFlag = false;
	while (!g_vm->shouldQuit() && !breakFlag) {
		Event event;
		while (g_globals->_events.getEvent(event) && !breakFlag) {
			// Adjust mouse positions to be relative within the dialog
			event.mousePos.x -= _gfxManager._bounds.left;
			event.mousePos.y -= _gfxManager._bounds.top;

			for (GfxElementList::iterator i = _elements.begin(); i != _elements.end(); ++i) {
				if ((*i)->process(event))
					selectedButton = static_cast<GfxButton *>(*i);
			}

			if (selectedButton == &_autoplay) {
				// Toggle Autoplay
				selectedButton = NULL;
				Scene1337 *scene = (Scene1337 *)R2_GLOBALS._sceneManager._scene;
				scene->_autoplay = !scene->_autoplay;

				_autoplay.setText(scene->_autoplay ? AUTO_PLAY_ON : AUTO_PLAY_OFF);
				_autoplay.draw();
			} else if (selectedButton) {
				breakFlag = true;
				break;
			} else if (!event.handled) {
				if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE)) {
					selectedButton = NULL;
					breakFlag = true;
					break;
				}
			}
		}

		g_system->delayMillis(10);
		GLOBALS._screenSurface.updateScreen();
	}

	_gfxManager.deactivate();
	return selectedButton;
}

/*--------------------------------------------------------------------------
 * Scene 1500 - Cutscene: Ship landing
 *
 *--------------------------------------------------------------------------*/

void Scene1500::postInit(SceneObjectList *OwnerList) {
	loadScene(1500);
	R2_GLOBALS._uiElements._active = false;
	setZoomPercents(170, 13, 240, 100);
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	scalePalette(65, 65, 65);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.disableControl();

	_starshipShadow.postInit();
	_starshipShadow.setup(1401, 1, 1);
	_starshipShadow._effect = EFFECT_SHADOW_MAP;
	_starshipShadow.fixPriority(10);
	_starshipShadow._shadowMap = _shadowPaletteMap;

	_starship.postInit();
	_starship.setup(1400, 1, 1);
	_starship._moveDiff = Common::Point(1, 1);
	_starship._linkedActor = &_starshipShadow;

	if (R2_GLOBALS._sceneManager._previousScene != 1010) {
		_smallShipShadow.postInit();
		_smallShipShadow.setup(1401, 2, 1);
		_smallShipShadow._effect = EFFECT_SHADOW_MAP;
		_smallShipShadow.fixPriority(10);
		_smallShipShadow._shadowMap = _shadowPaletteMap;

		_smallShip.postInit();
		_smallShip._moveRate = 30;
		_smallShip._moveDiff = Common::Point(1, 1);
		_smallShip._linkedActor = &_smallShipShadow;
	}

	if (R2_GLOBALS._sceneManager._previousScene == 300) {
		_starship.setPosition(Common::Point(189, 139), 5);

		_smallShip.setup(1400, 1, 2);
		_smallShip.setPosition(Common::Point(148, 108), 0);

		_sceneMode = 20;
		R2_GLOBALS._sound1.play(110);
	} else if (R2_GLOBALS._sceneManager._previousScene == 1550) {
		_starship.setPosition(Common::Point(189, 139), 5);

		_smallShip.setup(1400, 2, 1);
		_smallShip.changeZoom(-1);
		_smallShip.setPosition(Common::Point(298, 258), 5);

		_sceneMode = 10;
		R2_GLOBALS._sound1.play(106);
	} else {
		_starship.setPosition(Common::Point(289, 239), -30);
		_sceneMode = 0;
		R2_GLOBALS._sound1.play(102);
	}

	signal();
}

void Scene1500::remove() {
	R2_GLOBALS._uiElements._active = true;
	R2_GLOBALS._uiElements._visible = true;

	SceneExt::remove();
}

void Scene1500::signal() {
	switch(_sceneMode++) {
	case 0:
		R2_GLOBALS.setFlag(25);
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
	// No break on purpose
	case 1:
		if (_starship._yDiff < 50) {
			_starship.setPosition(Common::Point(289, 239), _starship._yDiff + 1);
			_sceneMode = 1;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 2: {
		Common::Point pt(189, 139);
		NpcMover *mover = new NpcMover();
		_starship.addMover(mover, &pt, this);
		}
		break;
	case 3:
		if (_starship._yDiff > 5) {
			_starship.setPosition(Common::Point(189, 139), _starship._yDiff - 1);
			_sceneMode = 3;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 13:
		R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	// No break on purpose
	case 4:
		R2_GLOBALS._sceneManager.changeScene(300);
		break;
	case 10:
	// No break on purpose
	case 20:
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 11: {
		Common::Point pt(148, 108);
		NpcMover *mover = new NpcMover();
		_smallShip.addMover(mover, &pt, this);
		}
		break;
	case 12:
		setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
		break;
	case 21: {
		Common::Point pt(-2, -42);
		NpcMover *mover = new NpcMover();
		_smallShip.addMover(mover, &pt, NULL);
		signal();
		}
		break;
	case 22:
		if (_starship._yDiff < 50) {
			_starship.setPosition(Common::Point(189, 139), _starship._yDiff + 1);
			_sceneMode = 22;
		}
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	case 23: {
		Common::Point pt(-13, -61);
		NpcMover *mover = new NpcMover();
		_starship.addMover(mover, &pt, this);
		}
		break;
	case 24:
		R2_GLOBALS._sceneManager.changeScene(1550);
		break;
	default:
		break;
	}
}

void Scene1500::dispatch() {
	if (_sceneMode > 10) {
		float yDiff = sqrt((float) (_smallShip._position.x * _smallShip._position.x) + (_smallShip._position.y * _smallShip._position.y));
		if (yDiff > 6)
			_smallShip.setPosition(_smallShip._position, (int) yDiff);
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1525 - Cutscene - Ship
 *
 *--------------------------------------------------------------------------*/

void Scene1525::postInit(SceneObjectList *OwnerList) {
	loadScene(1525);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();

	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._sceneManager._previousScene == 525)
		R2_GLOBALS._player.setup(1525, 1, 1);
	else
		R2_GLOBALS._player.setup(1525, 1, 16);
	R2_GLOBALS._player.setPosition(Common::Point(244, 148));
	R2_GLOBALS._player.disableControl();

	_sceneMode = 0;
	setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
}

void Scene1525::signal() {
	switch (_sceneMode++) {
	case 0:
		if (R2_GLOBALS._sceneManager._previousScene == 525)
			setAction(&_sequenceManager, this, 1525, &R2_GLOBALS._player, NULL);
		else
			setAction(&_sequenceManager, this, 1526, &R2_GLOBALS._player, NULL);
		break;
	case 1:
		setAction(&_sequenceManager, this, 2, &R2_GLOBALS._player, NULL);
		break;
	case 2:
		if (R2_GLOBALS._sceneManager._previousScene == 1530)
			R2_GLOBALS._sceneManager.changeScene(1550);
		else
			R2_GLOBALS._sceneManager.changeScene(1530);
	default:
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1530 - Cutscene - Crashing on Rimwall
 *
 *--------------------------------------------------------------------------*/

void Scene1530::postInit(SceneObjectList *OwnerList) {
	if (R2_GLOBALS._sceneManager._previousScene == 1000)
		loadScene(1650);
	else if (R2_GLOBALS._sceneManager._previousScene == 1580)
		loadScene(1550);
	else
		loadScene(1530);

	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == 1000) {
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.hide();
		R2_GLOBALS._player.disableControl();

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(538, this);
		R2_GLOBALS._sound1.play(114);

		_sceneMode = 3;
	} else if (R2_GLOBALS._sceneManager._previousScene == 1580) {
		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player.setup(1516, 6, 1);
		R2_GLOBALS._player.setPosition(Common::Point(160, 125));
		R2_GLOBALS._player._moveRate = 30;
		R2_GLOBALS._player._moveDiff = Common::Point(4, 1);

		_leftReactor.postInit();
		_leftReactor.setup(1516, 7, 1);
		_leftReactor.setPosition(Common::Point(121, 41));
		_leftReactor.animate(ANIM_MODE_2, NULL);

		_rightReactor.postInit();
		_rightReactor.setup(1516, 8, 1);
		_rightReactor.setPosition(Common::Point(107, 116));
		_rightReactor.animate(ANIM_MODE_2, NULL);

		R2_GLOBALS._player.disableControl();
		Common::Point pt(480, 75);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		R2_GLOBALS._sound1.play(111);

		_sceneMode = 1;
	} else {
		_seeker.postInit();
		_seeker._effect = EFFECT_SHADED;

		R2_GLOBALS._player.postInit();
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.disableControl();

		setAction(&_sequenceManager, this, 1530, &R2_GLOBALS._player, &_seeker, NULL);

		_sceneMode = 2;
	}

}

void Scene1530::signal() {
	switch (_sceneMode - 1) {
	case 0:
		R2_GLOBALS._sceneManager.changeScene(1000);
		break;
	case 1:
		R2_GLOBALS._sceneManager.changeScene(1525);
		break;
	case 2:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 4;
		R2_GLOBALS._player.show();
		setAction(&_sequenceManager, this, 1650, &R2_GLOBALS._player, NULL);
		break;
	case 3:
		R2_GLOBALS._sceneManager.changeScene(1700);
		break;
	default:
		break;
	}
}

void Scene1530::dispatch() {
	int16 x = R2_GLOBALS._player._position.x;
	int16 y = R2_GLOBALS._player._position.y;

	_leftReactor.setPosition(Common::Point(x - 39, y - 85));
	_rightReactor.setPosition(Common::Point(x - 53, y - 9));

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1550 - Spaceport
 *
 *--------------------------------------------------------------------------*/

Scene1550::Junk::Junk() {
	_junkNumber = 0;
}

void Scene1550::Junk::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_junkNumber);
}

bool Scene1550::Junk::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_visage == 1561) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 40;
			Common::Point pt(_position.x + 5, _position.y + 20);
			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &pt, scene);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case CURSOR_LOOK:
		if (_visage == 1561) {
			switch (_frame) {
			case 2:
				SceneItem::display(1550, 23, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			case 3:
				SceneItem::display(1550, 26, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			case 4:
				SceneItem::display(1550, 35, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			default:
				break;
			}
		} else {
			switch ((((_strip - 1) * 5) + _frame) % 3) {
			case 0:
				SceneItem::display(1550, 62, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			case 1:
				SceneItem::display(1550, 53, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			case 2:
				SceneItem::display(1550, 76, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
				break;
			default:
				break;
			}
		}
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

Scene1550::ShipComponent::ShipComponent() {
	_componentId = 0;
}

void Scene1550::ShipComponent::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_componentId);
}

bool Scene1550::ShipComponent::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (_componentId == 8) {
			scene->_dontExit = true;
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1576;
			else
				scene->_sceneMode = 1584;
			// strcpy(scene->_shipComponents[7]._actorName, 'hatch');
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[7], NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case CURSOR_LOOK:
		if (_componentId == 8)
			SceneItem::display(1550, 75, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		else if (_frame == 1)
			SceneItem::display(1550, 70, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		else
			SceneItem::display(1550, 71, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		return true;
		break;
	case R2_FUEL_CELL:
		scene->_dontExit = true;
		if (_componentId == 6) {
			R2_GLOBALS._player.disableControl();
			scene->_wreckage2.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1574;
			else
				scene->_sceneMode = 1582;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[5], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_GYROSCOPE:
		scene->_dontExit = true;
		if (_componentId == 3) {
			R2_GLOBALS._player.disableControl();
			scene->_wreckage2.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1571;
			else
				scene->_sceneMode = 1581;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[2], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_GUIDANCE_MODULE:
		scene->_dontExit = true;
		if (_componentId == 1) {
			R2_GLOBALS._player.disableControl();
			scene->_wreckage2.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1569;
			else
				scene->_sceneMode = 1579;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[0], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_THRUSTER_VALVE:
		scene->_dontExit = true;
		if (_componentId == 4) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1572;
			scene->_wreckage2.postInit();
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[3], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_RADAR_MECHANISM:
		scene->_dontExit = true;
		if (_componentId == 2) {
			R2_GLOBALS._player.disableControl();
			scene->_wreckage2.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1570;
			else
				scene->_sceneMode = 1580;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[1], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_IGNITOR:
		scene->_dontExit = true;
		if (_componentId == 5) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1573;
			scene->_wreckage2.postInit();
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[4], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	case R2_BATTERY:
		scene->_dontExit = true;
		if (_componentId == 7) {
			R2_GLOBALS._player.disableControl();
			scene->_wreckage2.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				scene->_sceneMode = 1575;
			else
				scene->_sceneMode = 1583;
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_shipComponents[6], &scene->_wreckage2, NULL);
			return true;
		}
		return SceneActor::startAction(action, event);
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

void Scene1550::ShipComponent::setupShipComponent(int componentId) {
	_componentId = componentId;
	postInit();
	setup(1517, _componentId, 1);

	switch (_componentId) {
	case 1:
		if (R2_INVENTORY.getObjectScene(R2_GUIDANCE_MODULE) == 0)
			setFrame(5);
		setPosition(Common::Point(287, 85));
		break;
	case 2:
		if (R2_INVENTORY.getObjectScene(R2_RADAR_MECHANISM) == 0)
			setFrame(5);
		setPosition(Common::Point(248, 100));
		break;
	case 3:
		if (R2_INVENTORY.getObjectScene(R2_GYROSCOPE) == 0)
			setFrame(5);
		setPosition(Common::Point(217, 85));
		break;
	case 4:
		if (R2_INVENTORY.getObjectScene(R2_THRUSTER_VALVE) == 0)
			setFrame(5);
		setPosition(Common::Point(161, 121));
		break;
	case 5:
		if (R2_INVENTORY.getObjectScene(R2_IGNITOR) == 0)
			setFrame(5);
		setPosition(Common::Point(117, 121));
		break;
	case 6:
		if (R2_INVENTORY.getObjectScene(R2_FUEL_CELL) == 0)
			setFrame(5);
		setPosition(Common::Point(111, 85));
		break;
	case 7:
		if (R2_INVENTORY.getObjectScene(R2_BATTERY) == 0)
			setFrame(5);
		setPosition(Common::Point(95, 84));
		break;
	case 8: {
		setup(1516, 1, 1);
		setPosition(Common::Point(201, 45));
		Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
		if ((scene->_sceneMode == 1577) || (scene->_sceneMode == 1578))
			hide();
		fixPriority(92);
		setDetails(1550, 70, -1, -1, 2, (SceneItem *) NULL);
		}
		break;
	default:
		break;
	}

	fixPriority(92);
	setDetails(1550, 70, -1, -1, 2, (SceneItem *)NULL);
}

Scene1550::DishControlsWindow::DishControl::DishControl() {
	_controlId = 0;
}

void Scene1550::DishControlsWindow::DishControl::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_controlId);
}

bool Scene1550::DishControlsWindow::DishControl::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (_controlId) {
	case 1:
		// Button control
		if (scene->_dish._frame == 5) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 25;
			if (scene->_walkway._frame == 1) {
				scene->setAction(&scene->_sequenceManager1, scene, 1560, &scene->_walkway, NULL);
				R2_GLOBALS.setFlag(20);
				setFrame(2);
			} else {
				scene->setAction(&scene->_sequenceManager1, scene, 1561, &scene->_walkway, NULL);
				R2_GLOBALS.clearFlag(20);
				setFrame(1);
			}
			scene->_dishControlsWindow.remove();
		}
		break;
	case 2:
		// Lever control
		R2_GLOBALS._player.disableControl();
		if (scene->_dish._frame == 1) {
			scene->_sceneMode = 23;
			scene->setAction(&scene->_sequenceManager1, scene, 1560, this, NULL);
		} else {
			if (scene->_walkway._frame == 1)
				scene->_sceneMode = 24;
			else
				scene->_sceneMode = 22;
			scene->setAction(&scene->_sequenceManager1, scene, 1561, this, NULL);
		}
	break;
	default:
		break;
	}
	return true;
}

void Scene1550::DishControlsWindow::remove() {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	_button.remove();
	_lever.remove();

	ModalWindow::remove();

	if ((scene->_sceneMode >= 20) && (scene->_sceneMode <= 29))
		return;

	R2_GLOBALS._player.disableControl();
	if (scene->_walkway._frame == 1) {
		scene->_sceneMode = 1559;
		scene->setAction(&scene->_sequenceManager1, scene, 1559, &R2_GLOBALS._player, NULL);
	} else {
		scene->_sceneMode = 1562;
		scene->setAction(&scene->_sequenceManager1, scene, 1562, &R2_GLOBALS._player, NULL);
	}
}

void Scene1550::DishControlsWindow::setup2(int visage, int stripFrameNum, int frameNum,
		int posX, int posY) {
	// Call inherited setup
	ModalWindow::setup2(visage, stripFrameNum, frameNum, posX, posY);

	// Further setup
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	setup3(1550, 67, -1, -1);
	_button.postInit();
	_button._controlId = 1;
	if (scene->_walkway._frame == 1)
		_button.setup(1559, 3, 1);
	else
		_button.setup(1559, 3, 2);
	_button.setPosition(Common::Point(142, 79));
	_button.fixPriority(251);
	_button.setDetails(1550, 68, -1, -1, 2, (SceneItem *) NULL);

	_lever.postInit();
	_lever._numFrames = 5;
	_lever._controlId = 2;
	if (scene->_dish._frame == 1)
		_lever.setup(1559, 2, 1);
	else
		_lever.setup(1559, 2, 5);
	_lever.setPosition(Common::Point(156, 103));
	_lever.fixPriority(251);
	_lever.setDetails(1550, 69, -1, -1, 2, (SceneItem *) NULL);
}

bool Scene1550::WorkingShip::startAction(CursorType action, Event &event) {
	return SceneHotspot::startAction(action, event);
}

bool Scene1550::Wreckage::startAction(CursorType action, Event &event) {
	return SceneActor::startAction(action, event);
}

bool Scene1550::Companion::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneMode = 80;
	scene->signal();

	return true;
}

bool Scene1550::AirBag::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	R2_GLOBALS._player.disableControl();
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	scene->_dontExit = true;
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		scene->_sceneMode = 1552;
	else
		scene->_sceneMode = 1588;

	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_airbag, NULL);
	return true;
}

bool Scene1550::Joystick::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		scene->_sceneMode = 50;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->_stripManager.start(518, scene);
		else
			scene->_stripManager.start(520, scene);
		return true;
		break;
	case CURSOR_LOOK:
		SceneItem::display(1550, 41, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
	}
}

bool Scene1550::Gyroscope::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		scene->_sceneMode = 1555;
	else
		scene->_sceneMode = 1589;

	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
		&scene->_gyroscope, NULL);
	return true;
}

bool Scene1550::DiagnosticsDisplay::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();
	scene->_dontExit = true;
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		scene->_sceneMode = 1586;
	else
		scene->_sceneMode = 1587;

	scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player,
		&scene->_diagnosticsDisplay, NULL);
	return true;
}

bool Scene1550::DishTower::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 1585;
		scene->setAction(&scene->_sequenceManager1, scene, 1585, &R2_GLOBALS._player, NULL);
	} else {
		R2_GLOBALS._player.disableControl();
		switch(scene->_dishMode) {
		case 0:
			scene->_dish.fixPriority(168);
			scene->_walkway.fixPriority(125);
			scene->_sceneMode = 1558;
			scene->setAction(&scene->_sequenceManager1, scene, 1558, &R2_GLOBALS._player, NULL);
			break;
		case 1:
			return SceneActor::startAction(action, event);
			break;
		case 2:
			scene->_dishMode = 1;
			scene->_sceneMode = 1563;
			scene->setAction(&scene->_sequenceManager1, scene, 1563, &R2_GLOBALS._player, &scene->_walkway, NULL);
			break;
		default:
			break;
		}
	}
	return true;

}

bool Scene1550::Dish::startAction(CursorType action, Event &event) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (scene->_dishMode != 2)
			return SceneActor::startAction(action, event);

		if (R2_INVENTORY.getObjectScene(R2_BATTERY) == 1550) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1564;
			scene->setAction(&scene->_sequenceManager1, scene, 1564, &R2_GLOBALS._player, NULL);
		} else
			SceneItem::display(1550, 64, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		return true;
		break;
	case CURSOR_LOOK:
		if (scene->_dishMode != 2)
			return SceneActor::startAction(action, event);

		if (R2_INVENTORY.getObjectScene(R2_BATTERY) == 1550) {
			SceneItem::display(1550, 74, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		} else
			SceneItem::display(1550, 64, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene1550::Scene1550() {
	_dontExit = false;
	_wallType = 0;
	_dishMode = 0;
	_sceneResourceId = 0;
	_walkRegionsId = 0;
}

void Scene1550::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_dontExit);
	s.syncAsByte(_wallType);
	s.syncAsSint16LE(_dishMode);
	s.syncAsSint16LE(_sceneResourceId);
	s.syncAsSint16LE(_walkRegionsId);
}

void Scene1550::postInit(SceneObjectList *OwnerList) {
	if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 9) &&
			(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11))
		// Exiting the intact spaceship
		loadScene(1234);
	else
		// Normal scene entry
		loadScene(1550);
	SceneExt::postInit();

	scalePalette(65, 65, 65);
	setZoomPercents(30, 75, 170, 100);
	_sceneResourceId = 1550;
	_walkRegionsId = 0;

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS.setFlag(16);

	if ((R2_GLOBALS._player._characterScene[R2_QUINN] != 1550) && (R2_GLOBALS._player._characterScene[R2_QUINN] != 1580)) {
		R2_GLOBALS._player._characterScene[R2_QUINN] = 1550;
		R2_GLOBALS._player._characterScene[R2_SEEKER] = 1550;
	}

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player._effect = EFFECT_SHADED2;

	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		R2_GLOBALS._player.setup(1500, 3, 1);
	else
		R2_GLOBALS._player.setup(1505, 3, 1);

	R2_GLOBALS._player._moveDiff = Common::Point(5, 3);

	if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 9) &&
			(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11))
		R2_GLOBALS._player.setPosition(Common::Point(157, 135));
	else
		R2_GLOBALS._player.setPosition(Common::Point(160, 100));

	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	R2_GLOBALS._player.disableControl();

	_wallType = 0;
	_companion.changeZoom(-1);
	R2_GLOBALS._player.changeZoom(-1);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1530:
		R2_GLOBALS._stripModifier = 0;
	// No break on purpose
	case 300:
	// No break on purpose
	case 1500:
	// No break on purpose
	case 3150:
		R2_GLOBALS._sound1.play(105);
		break;
	case 1580:
		// Leaving intact ship
		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1580) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);

			_dontExit = true;

			_wreckage2.postInit();
			_shipComponents[7].setupShipComponent(8);
			_shipComponents[7].hide();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_sceneMode = 1577;
			else
				_sceneMode = 1578;

			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_wreckage2, &_shipComponents[7], NULL);
			R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1550;
		} else {
			R2_GLOBALS._player.enableControl();
		}
		break;
	default:
		break;
	}

	enterArea();

	_intactHull1.setDetails(16, 1550, 10, -1, -1);
	_intactHull2.setDetails(24, 1550, 10, -1, -1);
	_background.setDetails(Rect(0, 0, 320, 200), 1550, 0, 1, -1, 1, NULL);

	if ((R2_GLOBALS._sceneManager._previousScene == 1500) && (R2_GLOBALS.getFlag(16))) {
		_sceneMode = 70;
		if (!R2_GLOBALS._sceneObjects->contains(&_companion))
			_companion.postInit();

		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_companion.setVisage(1505);
		else
			_companion.setVisage(1500);

		_companion.changeZoom(77);
		_companion.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);

		assert(_walkRegionsId >= 1550);
		R2_GLOBALS._walkRegions.disableRegion(k5A750[_walkRegionsId - 1550]);

		setAction(&_sequenceManager1, this, 1590, &_companion, NULL);
	} else if ((_sceneMode != 1577) && (_sceneMode != 1578))
		R2_GLOBALS._player.enableControl();
}

void Scene1550::signal() {
	switch (_sceneMode) {
	case 1:
	// No break on purpose
	case 3:
	// No break on purpose
	case 5:
	// No break on purpose
	case 7:
		_dontExit = false;
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	case 20:
	// No break on purpose
	case 21:
	// No break on purpose
	case 25:
	// No break on purpose
	case 1563:
		// Show the communication dish controls window
		R2_GLOBALS.clearFlag(20);
		_dishControlsWindow.setup2(1559, 1, 1, 160, 125);
		R2_GLOBALS._player.enableControl();
		_sceneMode = 0;
		break;
	case 22:
		_dishControlsWindow.remove();
		_sceneMode = 24;
		setAction(&_sequenceManager1, this, 1561, &_walkway, NULL);
		R2_GLOBALS.clearFlag(20);
		break;
	case 23:
		_dishControlsWindow.remove();
		_sceneMode = 20;
		setAction(&_sequenceManager1, this, 1566, &_dish, &_dishTowerShadow, NULL);
		R2_GLOBALS.setFlag(19);
		break;
	case 24:
		_dishControlsWindow.remove();
		_sceneMode = 21;
		setAction(&_sequenceManager1, this, 1567, &_dish, &_dishTowerShadow, NULL);
		R2_GLOBALS.clearFlag(19);
		break;
	case 30:
	// No break on purpose
	case 1556:
	// No break on purpose
	case 1557:
	// Nothing on purpose
		break;
	case 40: {
			_sceneMode = 41;
			Common::Point pt(_junk[0]._position.x, _junk[0]._position.y + 20);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 41:
		_sceneMode = 42;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setup(1502, 8, 1);
		} else {
			R2_GLOBALS._player.changeZoom(R2_GLOBALS._player._percent + 14);
			R2_GLOBALS._player.setup(1516, 4, 1);
		}
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;

	case 42: {
		_sceneMode = 43;
		int junkRegionIndex = R2_GLOBALS._scene1550JunkLocations[_junk[0]._junkNumber + 3];
		R2_GLOBALS._walkRegions.enableRegion(scene1550JunkRegions[junkRegionIndex]);

		switch (_junk[0]._frame) {
		case 1:
			R2_INVENTORY.setObjectScene(R2_JOYSTICK, R2_GLOBALS._player._characterIndex);
			break;
		case 2:
			R2_INVENTORY.setObjectScene(R2_FUEL_CELL, R2_GLOBALS._player._characterIndex);
			break;
		case 3:
			R2_INVENTORY.setObjectScene(R2_GUIDANCE_MODULE, R2_GLOBALS._player._characterIndex);
			break;
		case 4:
			R2_INVENTORY.setObjectScene(R2_RADAR_MECHANISM, R2_GLOBALS._player._characterIndex);
			break;
		case 5:
			R2_INVENTORY.setObjectScene(R2_BATTERY, R2_GLOBALS._player._characterIndex);
			break;
		case 6:
			R2_INVENTORY.setObjectScene(R2_DIAGNOSTICS_DISPLAY, R2_GLOBALS._player._characterIndex);
			break;
		default:
			break;
		}

		_junk[0].remove();
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	}
	case 43:
		R2_GLOBALS._scene1550JunkLocations[_junk[0]._junkNumber + 2] = 0;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			R2_GLOBALS._player.setVisage(1500);
		else {
			R2_GLOBALS._player.changeZoom(-1);
			R2_GLOBALS._player.setVisage(1505);
		}
		R2_GLOBALS._player.animate(ANIM_MODE_1, this);
		R2_GLOBALS._player.setStrip(8);
		R2_GLOBALS._player.enableControl();
		break;
	case 50:
		// Removed (useless ?) call to sub_1D227
		++_sceneMode;
		setAction(&_sequenceManager1, this, 1591, &R2_GLOBALS._player, NULL);
		if (g_globals->_sceneObjects->contains(&_companion))
			signal();
		else {
			_companion.postInit();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_companion.setVisage(1505);
			else
				_companion.setVisage(1500);
			_companion.changeZoom(77);
			_companion.setAction(&_sequenceManager2, this, 1590, &_companion, NULL);
			_companion.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
		}
		break;
	case 51:
		++_sceneMode;
		break;
	case 52:
		_companion.changeZoom(-1);
		_sceneMode = 1592;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			setAction(&_sequenceManager1, this, 1592, &R2_GLOBALS._player, &_companion,
				&_junk[0], &_joystick, NULL);
		else
			setAction(&_sequenceManager1, this, 1593, &R2_GLOBALS._player, &_companion,
				&_junk[0], &_joystick, NULL);
		break;
	case 61:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		_dishMode = 2;
		break;
	case 62:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		if (_dishMode == 2) {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
		}
		break;
	case 70:
		R2_GLOBALS._s1550PlayerArea[R2_SEEKER] = R2_GLOBALS._s1550PlayerArea[R2_QUINN];
		//R2_GLOBALS._s1550PlayerAreas[0] = 1;
		_sceneMode = 60;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(500, this);
		break;
	case 80:
		if (R2_GLOBALS.getFlag(16)) {
			_sceneMode = 60;
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._stripModifier >= 3) {
				if (R2_GLOBALS._player._characterIndex == R2_QUINN)
					_stripManager.start(572, this);
				else
					_stripManager.start(573, this);
			} else {
				++R2_GLOBALS._stripModifier;
				if (R2_GLOBALS._player._characterIndex == R2_QUINN)
					_stripManager.start(499 + R2_GLOBALS._stripModifier, this);
				else
					_stripManager.start(502 + R2_GLOBALS._stripModifier, this);
			}
		} else {
			_sceneMode = 60;
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			if (R2_GLOBALS._stripModifier >= 4) {
				if (R2_GLOBALS._player._characterIndex == R2_QUINN)
					_stripManager.start(572, this);
				else
					_stripManager.start(573, this);
			} else {
				++R2_GLOBALS._stripModifier;
				if (R2_GLOBALS._player._characterIndex == R2_QUINN)
					_stripManager.start(563 + R2_GLOBALS._stripModifier, this);
				else
					_stripManager.start(567 + R2_GLOBALS._stripModifier, this);
			}
		}
		break;
	case 1550:
		R2_GLOBALS._sceneManager.changeScene(1525);
		break;
	case 1552:
	// No break on purpose
	case 1588:
		R2_INVENTORY.setObjectScene(R2_AIRBAG, R2_GLOBALS._player._characterIndex);
		_airbag.remove();
		_dontExit = false;
		R2_GLOBALS._player.enableControl();
		break;
	case 1553:
		R2_GLOBALS._sceneManager.changeScene(1575);
		break;
	case 1554:
		R2_GLOBALS._player.enableControl();
		_dontExit = false;
		break;
	case 1555:
	// No break on purpose
	case 1589:
		R2_INVENTORY.setObjectScene(R2_GYROSCOPE, R2_GLOBALS._player._characterIndex);
		_gyroscope.remove();
		R2_GLOBALS._player.enableControl();
		break;
	case 1558:
		_dish.fixPriority(124);
		_dishMode = 1;
		_dishControlsWindow.setup2(1559, 1, 1, 160, 125);
		R2_GLOBALS._player.enableControl();
		break;
	case 1559:
		_dish.fixPriority(168);
		_walkway.fixPriority(169);
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.changeZoom(-1);
		_dishMode = 0;
		R2_GLOBALS._player.enableControl();
		break;
	case 1562:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		_dishMode = 2;
		break;
	case 1564:
		R2_INVENTORY.setObjectScene(R2_BATTERY, 1);
		_sceneMode = 1565;
		setAction(&_sequenceManager1, this, 1565, &R2_GLOBALS._player, NULL);
		break;
	case 1565:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 1569:
	// No break on purpose
	case 1579:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_GUIDANCE_MODULE, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1570:
	// No break on purpose
	case 1580:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_RADAR_MECHANISM, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1571:
	// No break on purpose
	case 1581:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_GYROSCOPE, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1572:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_THRUSTER_VALVE, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1573:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_IGNITOR, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1574:
	// No break on purpose
	case 1582:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_FUEL_CELL, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1575:
	// No break on purpose
	case 1583:
		_dontExit = false;
		_wreckage2.remove();
		R2_INVENTORY.setObjectScene(R2_BATTERY, 0);
		R2_GLOBALS._player.enableControl();
		break;
	case 1576:
	// No break on purpose
	case 1584:
		R2_GLOBALS._sceneManager.changeScene(1580);
		R2_GLOBALS._player.enableControl();
		break;
	case 1577:
	// No break on purpose
	case 1578:
		_sceneMode = 0;
		_wreckage2.remove();
		_dontExit = false;
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl();
		break;
	case 1585:
		SceneItem::display(1550, 66, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		R2_GLOBALS._player.enableControl();
		break;
	case 1586:
	// No break on purpose
	case 1587:
		R2_INVENTORY.setObjectScene(R2_DIAGNOSTICS_DISPLAY, R2_GLOBALS._player._characterIndex);
		_diagnosticsDisplay.remove();
		_dontExit = false;
		R2_GLOBALS._player.enableControl();
		break;
	case 1592:
		_joystick.remove();
		R2_INVENTORY.setObjectScene(R2_JOYSTICK, 1);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._s1550PlayerArea[R2_SEEKER] = R2_GLOBALS._s1550PlayerArea[R2_QUINN];
		} else {
			R2_GLOBALS._s1550PlayerArea[R2_QUINN] = R2_GLOBALS._s1550PlayerArea[R2_SEEKER];
		}
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	default:
		_sceneMode = 62;
		setAction(&_sequenceManager1, this, 1, &R2_GLOBALS._player, NULL);
		break;
	}
}

void Scene1550::process(Event &event) {
	if ((!R2_GLOBALS._player._canWalk) && (R2_GLOBALS._events.getCursor() == R2_NEGATOR_GUN) && (event.eventType == EVENT_BUTTON_DOWN) && (this->_screenNumber == 1234)) {
		int curReg = R2_GLOBALS._sceneRegions.indexOf(event.mousePos);
		if (curReg == 0)
			_dontExit = true;
		else if (((R2_GLOBALS._player._position.y < 90) && (event.mousePos.y > 90)) || ((R2_GLOBALS._player._position.y > 90) && (event.mousePos.y < 90)))
			_dontExit = true;
		else
			_dontExit = false;

		if ((curReg == 13) || (curReg == 14))
			_dontExit = false;
	}

	Scene::process(event);
}

void Scene1550::dispatch() {
	Scene::dispatch();

	// Arrays related to this scene are all hacky in the origina: they are based on the impossibility to use Miranda
	assert ((R2_GLOBALS._player._characterIndex == R2_QUINN) || (R2_GLOBALS._player._characterIndex == R2_SEEKER));

	if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 15) && (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 16)) {
		R2_GLOBALS._player._shade = 0;

		// NOTE: Original game contains a switch based on an uninitialized variable.
		// We're leaving this code here, but ifdef'ed out, in case we can ever figure out
		// what the original programmers intended the value to come from
#if 0
		int missingVariable = 0;
		switch (missingVariable) {
		case 144:
		// No break on purpose
		case 146:
			_dish._frame = 5;
			R2_GLOBALS._player._shade = 3;
			break;
		case 148:
		// No break on purpose
		case 149:
			_dish._frame = 1;
		// No break on purpose
		case 147:
		// No break on purpose
		case 150:
			R2_GLOBALS._player._shade = 3;
			break;
		default:
			break;
		}
#endif
	}

	if (_dontExit)
		return;

	switch (R2_GLOBALS._player.getRegionIndex() - 11) {
	case 0:
	// No break on purpose
	case 5:
		// Exiting the top of the screen
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1;
		_dontExit = true;
		--R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y;

		enterArea();

		R2_GLOBALS._player.setPosition(Common::Point( 160 - (((((160 - R2_GLOBALS._player._position.x) * 100) / 108) * 172) / 100), 145));
		if (R2_GLOBALS._player._position.x < 160) {
			Common::Point pt(R2_GLOBALS._player._position.x + 5, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if (R2_GLOBALS._player._position.x <= 160) { // the check is really in the original...
			Common::Point pt(R2_GLOBALS._player._position.x, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			Common::Point pt(R2_GLOBALS._player._position.x - 5, 135);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 1:
		// Exiting the bottom of the screen
		R2_GLOBALS._player.disableControl();
		_sceneMode = 3;
		_dontExit = true;
		++R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y;

		enterArea();

		R2_GLOBALS._player.setPosition(Common::Point( 160 - (((((160 - R2_GLOBALS._player._position.x) * 100) / 172) * 108) / 100), 19));
		if (R2_GLOBALS._player._position.x < 160) {
			Common::Point pt(R2_GLOBALS._player._position.x + 5, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if (R2_GLOBALS._player._position.x <= 160) { // the check is really in the original...
			Common::Point pt(R2_GLOBALS._player._position.x, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else {
			Common::Point pt(R2_GLOBALS._player._position.x - 5, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 2:
		// Exiting the right of the screen
		R2_GLOBALS._player.disableControl();
		_sceneMode = 5;
		_dontExit = true;
		++R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x;

		enterArea();

		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 9) && (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11) && (R2_GLOBALS._player._position.y > 50) && (R2_GLOBALS._player._position.y < 135)) {
			if (R2_GLOBALS._player._position.y >= 85) {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 10));
				Common::Point pt(R2_GLOBALS._player._position.x + 30, R2_GLOBALS._player._position.y + 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 10));
				Common::Point pt(R2_GLOBALS._player._position.x + 30, R2_GLOBALS._player._position.y - 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y));
			Common::Point pt(R2_GLOBALS._player._position.x + 10, R2_GLOBALS._player._position.y);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	case 3:
		// Exiting to the left of the screen
		R2_GLOBALS._player.disableControl();
		_sceneMode = 7;
		_dontExit = true;
		--R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x;

		enterArea();

		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 24) && (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11)) {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y / 2));
			Common::Point pt(265, 29);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		} else if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 9) && (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11) && (R2_GLOBALS._player._position.y > 50) && (R2_GLOBALS._player._position.y < 135)) {
			if (R2_GLOBALS._player._position.y >= 85) {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y + 10));
				Common::Point pt(R2_GLOBALS._player._position.x - 30, R2_GLOBALS._player._position.y + 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y - 10));
				Common::Point pt(R2_GLOBALS._player._position.x - 30, R2_GLOBALS._player._position.y - 20);
				NpcMover *mover = new NpcMover();
				R2_GLOBALS._player.addMover(mover, &pt, this);
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(320 - R2_GLOBALS._player._position.x, R2_GLOBALS._player._position.y));
			Common::Point pt(R2_GLOBALS._player._position.x - 10, R2_GLOBALS._player._position.y);
			NpcMover *mover = new NpcMover();
			R2_GLOBALS._player.addMover(mover, &pt, this);
		}
		break;
	default:
		break;
	}
}

void Scene1550::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
		R2_GLOBALS._sound1.fadeOut2(NULL);

	SceneExt::saveCharacter(characterIndex);
}

void Scene1550::Wall::setupWall(int frameNumber, int strip) {
	Scene1550 *scene = (Scene1550 *)R2_GLOBALS._sceneManager._scene;

	postInit();
	if (scene->_wallType == 2)
		setup(1551, strip, frameNumber);
	else
		setup(1554, strip, frameNumber);

	switch (strip) {
	case 0:
		switch (frameNumber - 1) {
		case 0:
			setup(1551, 1, 1);
			setPosition(Common::Point(30, 67));
			break;
		case 1:
			setup(1560, 1, 5);
			setPosition(Common::Point(141, 54));
			break;
		case 2:
			setup(1560, 2, 5);
			setPosition(Common::Point(178, 54));
			break;
		case 3:
			setup(1560, 2, 1);
			setPosition(Common::Point(289, 67));
			break;
		case 4:
			setup(1560, 2, 2);
			setPosition(Common::Point(298, 132));
			break;
		case 5:
			setup(1560, 1, 2);
			setPosition(Common::Point(21, 132));
			break;
		case 6:
			setup(1560, 2, 4);
			setPosition(Common::Point(285, 123));
			break;
		case 7:
			setup(1560, 1, 3);
			setPosition(Common::Point(30, 111));
			break;
		case 8:
			setup(1560, 2, 3);
			setPosition(Common::Point(289, 111));
			break;
		case 9:
			setup(1560, 1, 4);
			setPosition(Common::Point(34, 123));
			break;
		default:
			break;
		}
		fixPriority(1);
		break;
	case 1:
		if (frameNumber == 3) {
			setup(1553, 3, 1);
			setPosition(Common::Point(48, 44));
			fixPriority(2);
		} else {
			fixPriority(1);
			setPosition(Common::Point(32, 17));
		}

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 3:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 2:
		fixPriority(1);
		switch (frameNumber) {
		case 4:
			setup(1553, 4, 1);
			setPosition(Common::Point(48, 168));
			break;
		case 5:
			setup(1553, 3, 2);
			setPosition(Common::Point(20, 168));
			fixPriority(250);
			break;
		default:
			setPosition(Common::Point(28, 116));
			break;
		}

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 4:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 5:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 3:
		switch (frameNumber) {
		case 2:
			fixPriority(2);
			if (scene->_wallType == 2)
				setup(1553, 2, 1);
			else
				setup(1556, 2, 1);
			setPosition(Common::Point(160, 44));
			break;
		case 3:
			fixPriority(2);
			setup(1553, 5, 1);
			setPosition(Common::Point(178, 44));
			break;
		default:
			fixPriority(1);
			setPosition(Common::Point(160, 17));
			break;
		}

		if (frameNumber == 1)
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
		else
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
		break;
	case 4:
		if (frameNumber == 2) {
			fixPriority(250);
			if (scene->_wallType == 2)
				setup(1553, 1, 1);
			else
				setup(1556, 1, 1);
		} else {
			fixPriority(2);
		}

		if (frameNumber != 1)
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);

		setPosition(Common::Point(160, 168));
		break;
	case 5:
		fixPriority(1);
		setPosition(Common::Point(287, 17));

		switch (frameNumber) {
		case 2:
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
			break;
		case 3:
			setDetails(1550, 6, -1, -1, 2, (SceneItem *) NULL);
			break;
		default:
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
			break;
		}
		break;
	case 6:
		fixPriority(1);
		setPosition(Common::Point(291, 116));

		if (frameNumber == 2)
			setDetails(1550, 3, -1, -1, 2, (SceneItem *) NULL);
		else
			setDetails(1550, 72, -1, -1, 2, (SceneItem *) NULL);
		break;
	default:
		break;
	}

}

void Scene1550::enterArea() {
	_walkRegionsId = 0;
	_dishMode = 0;

	_wallCorner1.remove();
	_westWall.remove();
	_northWall.remove();
	_southWall.remove();
	_wallCorner2.remove();
	_eastWall.remove();

	for (int i = 0; i < 8; ++i)
		_junk[i].remove();

	_wreckage.remove();

	for (int i = 0; i < 8; ++i)
		_shipComponents[i].remove();

	_airbag.remove();
	_joystick.remove();
	_gyroscope.remove();
	_wreckage4.remove();
	_diagnosticsDisplay.remove();

	if ((_sceneMode != 1577) && (_sceneMode != 1578))
		_wreckage2.remove();

	_wreckage3.remove();
	_companion.remove();
	_dish.remove();
	_dishTowerShadow.remove();
	_dishTower.remove();
	_walkway.remove();

	// Set up of special walk regions for certain areas
	switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y) {
	case 0:
		switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x) {
		case 3:
			R2_GLOBALS._walkRegions.load(1554);
			_walkRegionsId = 1554;
			break;
		case 4:
			R2_GLOBALS._walkRegions.load(1553);
			_walkRegionsId = 1553;
			break;
		default:
			break;
		}
		break;
	case 3:
	// No break on purpose
	case 4:
		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 23) || (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x)) {
			if (!R2_GLOBALS.getFlag(16)) {
				R2_GLOBALS._walkRegions.load(1559);
				_walkRegionsId = 1559;
			}
		}
		break;
	case 7:
		switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x) {
		case 10:
			R2_GLOBALS._walkRegions.load(1555);
			_walkRegionsId = 1555;
			break;
		case 11:
			R2_GLOBALS._walkRegions.load(1556);
			_walkRegionsId = 1556;
			break;
		default:
			break;
		}
		break;
	case 11:
		switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x) {
		case 24:
			R2_GLOBALS._walkRegions.load(1558);
			_walkRegionsId = 1558;
			break;
		case 25:
			R2_GLOBALS._walkRegions.load(1557);
			_walkRegionsId = 1557;
			break;
		default:
			break;
		}
		break;
	case 16:
		switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x) {
		case 2:
			R2_GLOBALS._walkRegions.load(1552);
			_walkRegionsId = 1552;
			break;
		case 3:
			R2_GLOBALS._walkRegions.load(1551);
			_walkRegionsId = 1551;
			break;
		case 15:
			R2_GLOBALS._walkRegions.load(1575);
			_walkRegionsId = 1575;
		default:
			break;
		}
		break;
	default:
		break;
	}

	int varA = 0;

	// This section handles checks if the ARM spacecraft have not yet seized
	// control of Lance of Truth.
	if (!R2_GLOBALS.getFlag(16)) {
		switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y - 2) {
		case 0:
			switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x - 22) {
				case 0:
					varA = 1553;
					_northWall.setupWall(6, 0);
					break;
				case 1:
				// No break on purpose
				case 2:
				// No break on purpose
				case 3:
				// No break on purpose
				case 4:
					varA = 1553;
					break;
				case 5:
					varA = 1553;
					_northWall.setupWall(6, 0);
					break;
				default:
					break;
			}
			break;
		case 1:
		// No break on purpose
		case 2:
			switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x - 21) {
			case 0:
				varA = 1550;
				_northWall.setupWall(9, 0);
				break;
			case 1:
				varA = 1552;
				_northWall.setupWall(10, 0);
				break;
			case 2:
			// No break on purpose
			case 3:
			// No break on purpose
			case 4:
			// No break on purpose
			case 5:
				varA = 1552;
				break;
			case 6:
				varA = 1552;
				_northWall.setupWall(7, 0);
				break;
			case 7:
				varA = 1550;
				_northWall.setupWall(8, 0);
				break;
			default:
				break;
			}
			break;
		case 3:
			switch (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x - 21) {
			case 0:
				varA = 1550;
				_northWall.setupWall(4, 0);
				break;
			case 1:
				varA = 1550;
				_northWall.setupWall(3, 0);
				break;
			case 2:
			// No break on purpose
			case 3:
			// No break on purpose
			case 4:
			// No break on purpose
			case 5:
				varA = 1551;
				break;
			case 6:
				varA = 1550;
				_northWall.setupWall(2, 0);
				break;
			case 7:
				varA = 1550;
				_northWall.setupWall(1, 0);
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y > 0) &&
				(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x <= 29) &&
				((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x < 20) ||
				(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y > 7))) {
			// In an area where the cutscene can be triggered, so start it
			R2_GLOBALS.setFlag(16);
			R2_GLOBALS._sceneManager.changeScene(1500);
		}
	}

	if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == 9) &&
			(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 11)) {
		if (_screenNumber != 1234) {
			R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;
			loadScene(1234);
			R2_GLOBALS._sceneManager._hasPalette = false;
			_wallType = 0;
		}
	} else if (_screenNumber == 1234) {
		R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;
		loadScene(1550);
		R2_GLOBALS._sceneManager._hasPalette = false;
	}

	if (_screenNumber == 1234)
		_walkRegionsId = 1576;

	if (_wallType == 0) {
		_wallType = 1;
	} else {
		if (_wallType == 2) {
			_wallType = 3;
		} else {
			_wallType = 2;
		}

		if (R2_GLOBALS._sceneManager._sceneNumber == 1550){
#if 0
			warning("Mouse_hideIfNeeded()");
			warning("gfx_set_pane_p");
			for (int i = 3; i != 168; ++i) {
				warning("sub294D2(4, i, 312,  var14C)");
				warning("missing for loop, to be implemented");
				warning("gfx_draw_slice");
			}
			warning("gfx_flip_screen()");
			warning("gfx_set_pane_p()");
#endif
			R2_GLOBALS._sceneManager._fadeMode = FADEMODE_IMMEDIATE;

			if (varA == 0) {
				if (_sceneResourceId != 1550) {
					g_globals->_scenePalette.loadPalette(1550);
					R2_GLOBALS._sceneManager._hasPalette = true;
				}
			} else {
				g_globals->_scenePalette.loadPalette(varA);
				R2_GLOBALS._sceneManager._hasPalette = true;
			}

			if (R2_GLOBALS._sceneManager._hasPalette)
				_sceneResourceId = varA;

//			warning("sub_2C429()");
		}
	}

	// Scene setup dependent on the type of cell specified in the scene map
	switch (scene1550AreaMap[(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y * 30) +
		R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x]) {
	case 0:
		// Standard cell
		if (_walkRegionsId == 0) {
			R2_GLOBALS._walkRegions.load(1550);
			_walkRegionsId = 1550;
		}
		break;
	case 1:
		// North end of the spaceport
		if (_walkRegionsId == 0) {
			R2_GLOBALS._walkRegions.load(1560);
			_walkRegionsId = 1560;
		}
		_wallCorner1.setupWall(2, 1);
		_northWall.setupWall(1, 3);
		_wallCorner2.setupWall(2, 5);
		break;
	case 2:
		R2_GLOBALS._walkRegions.load(1561);
		_walkRegionsId = 1561;
		_wallCorner1.setupWall(2, 1);
		_westWall.setupWall(2, 2);
		_northWall.setupWall(1, 3);
		_wallCorner2.setupWall(2, 5);
		break;
	case 3:
		R2_GLOBALS._walkRegions.load(1562);
		_walkRegionsId = 1562;
		_wallCorner1.setupWall(2, 1);
		_northWall.setupWall(1, 3);
		_wallCorner2.setupWall(2, 5);
		_eastWall.setupWall(2, 6);
		break;
	case 4:
		R2_GLOBALS._walkRegions.load(1563);
		_walkRegionsId = 1563;
		_northWall.setupWall(2, 3);
		break;
	case 5:
		R2_GLOBALS._walkRegions.load(1564);
		_walkRegionsId = 1564;
		_southWall.setupWall(2, 4);
		break;
	case 6:
		R2_GLOBALS._walkRegions.load(1565);
		_walkRegionsId = 1565;
		_wallCorner1.setupWall(1, 1);
		_westWall.setupWall(1, 2);
		_northWall.setupWall(3, 3);
		break;
	case 7:
		R2_GLOBALS._walkRegions.load(1566);
		_walkRegionsId = 1566;
		_wallCorner1.setupWall(1, 1);
		_westWall.setupWall(1, 2);
		_northWall.setupWall(2, 4);
		break;
	case 8:
		R2_GLOBALS._walkRegions.load(1567);
		_walkRegionsId = 1567;
		_westWall.setupWall(5, 2);
		break;
	case 9:
		R2_GLOBALS._walkRegions.load(1568);
		_walkRegionsId = 1568;
		_westWall.setupWall(4, 2);
		break;
	case 10:
		R2_GLOBALS._walkRegions.load(1569);
		_walkRegionsId = 1569;
		_wallCorner1.setupWall(3, 1);
		break;
	case 11:
		R2_GLOBALS._walkRegions.load(1570);
		_walkRegionsId = 1570;
		_wallCorner1.setupWall(1, 1);
		_westWall.setupWall(1, 2);
		break;
	case 12:
		R2_GLOBALS._walkRegions.load(1571);
		_walkRegionsId = 1571;
		_wallCorner2.setupWall(1, 5);
		_eastWall.setupWall(1, 6);
		break;
	case 13:
		R2_GLOBALS._walkRegions.load(1572);
		_walkRegionsId = 1572;
		_wallCorner1.setupWall(1, 1);
		_westWall.setupWall(1, 2);
		_southWall.setupWall(1, 4);
		break;
	case 14:
		R2_GLOBALS._walkRegions.load(1573);
		_walkRegionsId = 1573;
		_southWall.setupWall(1, 4);
		_wallCorner2.setupWall(1, 5);
		_eastWall.setupWall(1, 6);
		break;
	case 15:
		// South wall
		R2_GLOBALS._walkRegions.load(1574);
		_walkRegionsId = 1574;
		_southWall.setupWall(1, 4);
		break;
	case 16:
		R2_GLOBALS._walkRegions.load(1570);
		_walkRegionsId = 1570;
		_wallCorner1.setupWall(2, 1);
		_westWall.setupWall(2, 2);
		break;
	case 17:
		R2_GLOBALS._walkRegions.load(1570);
		_walkRegionsId = 1570;
		_wallCorner1.setupWall(2, 1);
		_westWall.setupWall(3, 2);
		break;
	case 18:
		R2_GLOBALS._walkRegions.load(1571);
		_walkRegionsId = 1571;
		_wallCorner2.setupWall(2, 5);
		_eastWall.setupWall(2, 6);
		break;
	case 19:
		R2_GLOBALS._walkRegions.load(1571);
		_walkRegionsId = 1571;
		_wallCorner2.setupWall(2, 5);
		_eastWall.setupWall(3, 6);
		break;
	default:
		break;
	}

	int di = 0;
	int tmpIdx = 0;
	for (int i = 0; i < 127 * 4; i += 4) {
		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == R2_GLOBALS._scene1550JunkLocations[i]) &&
				(R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == R2_GLOBALS._scene1550JunkLocations[i + 1]) &&
				(R2_GLOBALS._scene1550JunkLocations[i + 2] != 0)) {
			tmpIdx = R2_GLOBALS._scene1550JunkLocations[i + 3];
			_junk[di].postInit();
			_junk[di]._effect = EFFECT_SHADED2;
			_junk[di]._shade = 0;
			//_junk[di]._junkState = tmpIdx;
			_junk[di]._junkNumber = i;
			_junk[di].setDetails(1550, 62, -1, 63, 2, (SceneItem *) NULL);
			if (R2_GLOBALS._scene1550JunkLocations[i + 2] == 41) {
				_junk[di].changeZoom(-1);
				_junk[di].setPosition(Common::Point(150, 70));
				_junk[di].setup(1562, 1, 1);

				R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[2]);
				R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[3]);
				R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[6]);
				R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[7]);

				if (R2_INVENTORY.getObjectScene(R2_JOYSTICK) == 1550) {
					_joystick.postInit();
					_joystick.setup(1562, 3, 1);
					_joystick.setPosition(Common::Point(150, 70));
					_joystick.fixPriority(10);
					_joystick.setDetails(1550, 41, -1, 42, 2, (SceneItem *) NULL);
				}
			} else {
				if (R2_GLOBALS._scene1550JunkLocations[i + 2] > 40) {
					_junk[di].changeZoom(100);
					_junk[di].setup(1561, 1, R2_GLOBALS._scene1550JunkLocations[i + 2] - 40);
				} else {
					_junk[di].changeZoom(-1);
					_junk[di].setup(1552, ((R2_GLOBALS._scene1550JunkLocations[i + 2] - 1) / 5) + 1, ((R2_GLOBALS._scene1550JunkLocations[i + 2] - 1) % 5) + 1);
				}
				_junk[di].setPosition(Common::Point(scene1550JunkX[tmpIdx], scene1550JunkY[tmpIdx]));
				if (scene1550JunkRegions[tmpIdx] != 0)
					R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[tmpIdx]);
				di++;
			}
		}
	}

	// Loop for detecting and setting up certain special areas within the map
	for (int i = 0; i < 15 * 3; i += 3) {
		if ((R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].x == scene1550SpecialAreas[i])
				&& (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == scene1550SpecialAreas[i + 1])) {
			int areaType = scene1550SpecialAreas[i + 2];
			switch (areaType) {
			case 1:
				if (!R2_GLOBALS.getFlag(16)) {
					_wreckage2.postInit();
					if (R2_GLOBALS._s1550PlayerArea[R2_GLOBALS._player._characterIndex].y == 3)
						_wreckage2.setup(1555, 2, 1);
					else
						_wreckage2.setup(1555, 1, 1);
					_wreckage2.setPosition(Common::Point(150, 100));
					_wreckage2.fixPriority(92);
					_wreckage2.setDetails(1550, 73, -1, -1, 2, (SceneItem *) NULL);
				}
				break;
			case 2:
				_dish.postInit();
				if (R2_GLOBALS.getFlag(19))
					_dish.setup(1556, 3, 5);
				else
					_dish.setup(1556, 3, 1);
				_dish.changeZoom(95);
				_dish.setPosition(Common::Point(165, 83));
				_dish.fixPriority(168);
				_dish.setDetails(1550, 17, -1, 19, 2, (SceneItem *) NULL);

				_dishTower.postInit();
				_dishTower.setup(1556, 4, 1);
				_dishTower.setPosition(Common::Point(191, 123));
				_dishTower.changeZoom(95);
				_dishTower.setDetails(1550, 65, -1, 66, 2, (SceneItem *) NULL);

				_dishTowerShadow.postInit();
				_dishTowerShadow._numFrames = 5;
				if (R2_GLOBALS.getFlag(19))
					_dishTowerShadow.setup(1556, 8, 5);
				else
					_dishTowerShadow.setup(1556, 8, 1);

				_dishTowerShadow.setPosition(Common::Point(156, 151));
				_dishTowerShadow.fixPriority(10);

				_walkway.postInit();
				if (R2_GLOBALS.getFlag(20))
					_walkway.setup(1558, 3, 10);
				else
					_walkway.setup(1558, 3, 1);

				_walkway.setPosition(Common::Point(172, 48));
				_walkway.fixPriority(169);
				R2_GLOBALS._walkRegions.disableRegion(scene1550JunkRegions[15]);
				break;
			case 3:
				_wreckage.postInit();
				_wreckage.setup(1550, 1, 1);
				_wreckage.setPosition(Common::Point(259, 55));
				_wreckage.fixPriority(133);
				_wreckage.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 1, 2);
				_wreckage2.setPosition(Common::Point(259, 133));
				_wreckage2.fixPriority(105);
				_wreckage2.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				if (R2_INVENTORY.getObjectScene(R2_GYROSCOPE) == 1550) {
					_gyroscope.postInit();
					_gyroscope.setup(1550, 7, 2);
					_gyroscope.setPosition(Common::Point(227, 30));
					_gyroscope.fixPriority(130);
					_gyroscope.setDetails(1550, 29, -1, 63, 2, (SceneItem *) NULL);
				}
				break;
			case 4:
				_wreckage.postInit();
				_wreckage.setup(1550, 1, 4);
				_wreckage.setPosition(Common::Point(76, 131));
				_wreckage.fixPriority(10);
				_wreckage.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 1, 3);
				_wreckage2.setPosition(Common::Point(76, 64));
				_wreckage2.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				if (R2_INVENTORY.getObjectScene(R2_DIAGNOSTICS_DISPLAY) == 1550) {
					_diagnosticsDisplay.postInit();
					_diagnosticsDisplay.setup(1504, 4, 1);
					_diagnosticsDisplay.setPosition(Common::Point(49, 35));
					_diagnosticsDisplay.animate(ANIM_MODE_2, NULL);
					_diagnosticsDisplay._numFrames = 4;
					_diagnosticsDisplay.fixPriority(65);
					_diagnosticsDisplay.setDetails(1550, 14, 15, 63, 2, (SceneItem *) NULL);
				}
				if (R2_INVENTORY.getObjectScene(R2_AIRBAG) == 1550) {
					_airbag.postInit();
					_airbag.setup(1550, 7, 1);
					_airbag.setPosition(Common::Point(45, 44));
					_airbag.fixPriority(150);
					_airbag.setDetails(1550, 44, -1, 63, 2, (SceneItem *) NULL);
				}
				break;
			case 5:
				_wreckage.postInit();
				_wreckage.setup(1550, 2, 4);
				_wreckage.setPosition(Common::Point(243, 131));
				_wreckage.fixPriority(10);
				_wreckage.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 2, 3);
				_wreckage2.setPosition(Common::Point(243, 64));
				_wreckage2.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 6:
				_wreckage.postInit();
				_wreckage.setup(1550, 2, 1);
				_wreckage.setPosition(Common::Point(60, 55));
				_wreckage.fixPriority(133);
				_wreckage.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 2, 2);
				_wreckage2.setPosition(Common::Point(60, 133));
				_wreckage2.fixPriority(106);
				_wreckage2.setDetails(1550, 9, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 7:
				_wreckage.postInit();
				_wreckage.setup(1550, 3, 1);
				_wreckage.setPosition(Common::Point(281, 132));
				_wreckage.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 8:
				_wreckage.postInit();
				_wreckage.setup(1550, 3, 2);
				_wreckage.setPosition(Common::Point(57, 96));
				_wreckage.fixPriority(70);
				_wreckage.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 3, 3);
				_wreckage2.setPosition(Common::Point(145, 88));
				_wreckage2.fixPriority(55);
				_wreckage2.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage3.postInit();
				_wreckage3.setup(1550, 3, 4);
				_wreckage3.setPosition(Common::Point(64, 137));
				_wreckage3.fixPriority(115);
				_wreckage3.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage4.postInit();
				_wreckage4.setup(1550, 5, 1);
				_wreckage4.setPosition(Common::Point(60, 90));
				_wreckage4.fixPriority(45);
				break;
			case 9:
				_wreckage.postInit();
				_wreckage.setup(1550, 4, 2);
				_wreckage.setPosition(Common::Point(262, 96));
				_wreckage.fixPriority(70);
				_wreckage.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage2.postInit();
				_wreckage2.setup(1550, 4, 3);
				_wreckage2.setPosition(Common::Point(174, 88));
				_wreckage2.fixPriority(55);
				_wreckage2.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage3.postInit();
				_wreckage3.setup(1550, 4, 4);
				_wreckage3.setPosition(Common::Point(255, 137));
				_wreckage3.fixPriority(115);
				_wreckage3.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);

				_wreckage4.postInit();
				_wreckage4.setup(1550, 6, 1);
				_wreckage4.setPosition(Common::Point(259, 90));
				_wreckage4.fixPriority(45);
				break;
			case 10:
				_wreckage.postInit();
				_wreckage.setup(1550, 4, 1);
				_wreckage.setPosition(Common::Point(38, 132));
				_wreckage.setDetails(1550, 56, -1, -1, 2, (SceneItem *) NULL);
				break;
			case 12:
				// Intact ship
				_shipComponents[7].setupShipComponent(8);
				_shipComponents[0].setupShipComponent(1);
				_shipComponents[1].setupShipComponent(2);
				_shipComponents[2].setupShipComponent(3);
				_shipComponents[3].setupShipComponent(4);
				_shipComponents[4].setupShipComponent(5);
				_shipComponents[5].setupShipComponent(6);
				_shipComponents[6].setupShipComponent(7);
				break;
			default:
				break;
			}
		}
	}

	if (R2_GLOBALS._s1550PlayerArea[R2_QUINN] == R2_GLOBALS._s1550PlayerArea[R2_SEEKER]) {
		_companion.postInit();
		_companion._effect = EFFECT_SHADED2;
		_companion.changeZoom(-1);

		assert((_walkRegionsId >= 1550) && (_walkRegionsId <= 2008));
		R2_GLOBALS._walkRegions.disableRegion(k5A750[_walkRegionsId - 1550]);
		_companion.setPosition(Common::Point(scene1550JunkX[k5A76D[_walkRegionsId - 1550]], scene1550JunkY[k5A76D[_walkRegionsId - 1550]] + 8));
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS._player._characterScene[R2_SEEKER] == 1580) {
				_companion.setup(1516, 3, 17);
				_companion.setPosition(Common::Point(272, 94));
				_companion.fixPriority(91);
				_companion.changeZoom(100);
				_companion.setDetails(1550, -1, -1, -1, 5, &_shipComponents[7]);
			} else {
				_companion.setup(1505, 6, 1);
				_companion.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
			}
		} else {
			if (R2_GLOBALS._player._characterScene[R2_QUINN] == 1580) {
				_companion.setup(1516, 2, 14);
				_companion.setPosition(Common::Point(276, 97));
				_companion.fixPriority(91);
				_companion.changeZoom(100);
				_companion.setDetails(1550, -1, -1, -1, 5, &_shipComponents[7]);
			} else {
				_companion.setup(1500, 6, 1);
				_companion.setDetails(1550, -1, -1, -1, 2, (SceneItem *) NULL);
			}
		}
	}
	R2_GLOBALS._uiElements.updateInventory();
}

/*--------------------------------------------------------------------------
 * Scene 1575 - Spaceport - unused ship scene
 *
 *--------------------------------------------------------------------------*/

Scene1575::Button::Button() {
	_buttonId = 0;
	_pressed = false;
}

void Scene1575::Button::synchronize(Serializer &s) {
	NamedHotspot::synchronize(s);

	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_pressed);
}

void Scene1575::Button::process(Event &event) {
	Scene1575 *scene = (Scene1575 *)R2_GLOBALS._sceneManager._scene;
	bool isInBounds = _bounds.contains(event.mousePos);
	CursorType cursor = R2_GLOBALS._events.getCursor();

	if ((event.eventType == EVENT_BUTTON_DOWN && cursor == CURSOR_USE && isInBounds) ||
		(_pressed && _buttonId != 1 && event.eventType == EVENT_BUTTON_UP && isInBounds)) {
		// Button pressed
		_pressed = true;
		Common::Point pos = scene->_actor1._position;
		event.handled = true;

		if (!R2_GLOBALS.getFlag(18) || _buttonId <= 1 || _buttonId >= 6) {
			switch (_buttonId) {
			case 1:
				if (R2_GLOBALS.getFlag(18)) {
					scene->_actor14.hide();
					scene->_actor15.hide();
					R2_GLOBALS.clearFlag(18);
				} else if ((scene->_actor12._position.x == 85) && (scene->_actor12._position.y == 123)) {
					scene->_actor14.show();
					scene->_actor15.show();
					R2_GLOBALS.setFlag(18);
				} else {
					SceneItem::display("That's probably not a good thing, ya know!");
				}
				break;
			case 2:
				if (scene->_field41A < 780) {
					if (pos.x > 54)
						pos.x -= 65;
					pos.x += 2;
					scene->_field41A += 2;

					for (int i = 0; i < 17; i++)
						scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x + 2, scene->_arrActor[i]._position.y));

					scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x + 2, scene->_actor13._position.y));
					scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x + 2, scene->_actor12._position.y));
					scene->_actor1.setPosition(Common::Point(pos.x, pos.y));
					scene->_actor2.setPosition(Common::Point(pos.x + 65, pos.y));
					scene->_actor3.setPosition(Common::Point(pos.x + 130, pos.y));
				}
				break;
			case 3:
				if (scene->_field41A > 0) {
					if (pos.x < -8)
						pos.x += 65;

					pos.x -= 2;
					scene->_field41A -= 2;
					for (int i = 0; i < 17; i++)
						scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x - 2, scene->_arrActor[i]._position.y));

					scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x - 2, scene->_actor13._position.y));
					scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x - 2, scene->_actor12._position.y));
					scene->_actor1.setPosition(Common::Point(pos.x, pos.y));
					scene->_actor2.setPosition(Common::Point(pos.x + 65, pos.y));
					scene->_actor3.setPosition(Common::Point(pos.x + 130, pos.y));
				}
				break;
			case 4: {
					if (pos.y < 176) {
						++pos.y;
						for (int i = 0; i < 17; ++i)
							scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x, scene->_arrActor[i]._position.y + 1));

						scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x, scene->_actor13._position.y + 1));
						scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x, scene->_actor12._position.y + 1));
						scene->_actor1.setPosition(Common::Point(pos.x, pos.y));
						scene->_actor2.setPosition(Common::Point(pos.x + 65, pos.y));
						scene->_actor3.setPosition(Common::Point(pos.x + 130, pos.y));
					}
				}
				break;
			case 5: {
					if (pos.y > 145) {
						--pos.y;
						for (int i = 0; i < 17; ++i)
							scene->_arrActor[i].setPosition(Common::Point(scene->_arrActor[i]._position.x, scene->_arrActor[i]._position.y - 1));

						scene->_actor13.setPosition(Common::Point(scene->_actor13._position.x, scene->_actor13._position.y - 1));
						scene->_actor12.setPosition(Common::Point(scene->_actor12._position.x, scene->_actor12._position.y - 1));
						scene->_actor1.setPosition(Common::Point(pos.x, pos.y));
						scene->_actor2.setPosition(Common::Point(pos.x + 65, pos.y));
						scene->_actor3.setPosition(Common::Point(pos.x + 130, pos.y));
					}
				}
				break;
			case 6:
				R2_GLOBALS._sceneManager.changeScene(1550);
				break;
			default:
				break;
			}

			int j = 0;
			for (int i = 0; i < 17; i++) {
				if (scene->_arrActor[i]._bounds.contains(85, 116))
					j = i;
			}

			if (scene->_actor13._bounds.contains(85, 116))
				j = 18;

			if (scene->_actor12._bounds.contains(85, 116))
				j = 19;

			if (j)
				scene->_actor11.show();
			else
				scene->_actor11.hide();
		} else {
			SceneItem::display("Better not move the laser while it's firing!");
		}
	} else {
		_pressed = false;
	}
}

bool Scene1575::Button::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return false;
	return SceneHotspot::startAction(action, event);
}

void Scene1575::Button::initButton(int buttonId) {
	_buttonId = buttonId;
	_pressed = false;
	EventHandler::postInit();

	switch (_buttonId) {
	case 1:
		setDetails(Rect(53, 165, 117, 190), -1, -1, -1, 2, 1, NULL);
		break;
	case 2:
		setDetails(Rect(151, 142, 189, 161), -1, -1, -1, 2, 1, NULL);
		break;
	case 3:
		setDetails(Rect(225, 142, 263, 161), -1, -1, -1, 2, 1, NULL);
		break;
	case 4:
		setDetails(Rect(188, 122, 226, 140), -1, -1, -1, 2, 1, NULL);
		break;
	case 5:
		setDetails(Rect(188, 162, 226, 180), -1, -1, -1, 2, 1, NULL);
		break;
	case 6:
		setDetails(Rect(269, 169, 301, 185), -1, -1, -1, 2, 1, NULL);
		break;
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*/

Scene1575::Scene1575() {
	_field412 = 0;
	_field414 = 390;
	_field416 = 0;
	_field418 = 0;
	_field41A = 0;
}

void Scene1575::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_field412);
	s.syncAsSint16LE(_field414);
	s.syncAsSint16LE(_field416);
	s.syncAsSint16LE(_field418);
	s.syncAsSint16LE(_field41A);
}

double hypotenuse(double v1, double v2) {
	return sqrt(v1 * v1 + v2 * v2);
}

void Scene1575::postInit(SceneObjectList *OwnerList) {
	loadScene(1575);
	R2_GLOBALS._uiElements._active = false;
	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	_actor1.postInit();
	_actor1.setup(1575, 1, 1);
	_actor1.setPosition(Common::Point(54, 161));
	_actor1.fixPriority(5);

	_actor2.postInit();
	_actor2.setup(1575, 1, 1);
	_actor2.setPosition(Common::Point(119, 161));
	_actor2.fixPriority(5);

	_actor3.postInit();
	_actor3.setup(1575, 1, 1);
	_actor3.setPosition(Common::Point(184, 161));
	_actor3.fixPriority(5);

	for (int i = 0; i < 17; i++) {
		_arrActor[i].postInit();
		_arrActor[i].setup(1575, 2, k5A7F6[3 * i + 2]);

		double v1 = hypotenuse(2.0, 3 - k5A7F6[3 * i]);
		v1 += hypotenuse(2.0, 3 - k5A7F6[3 * i + 1]);
		int yp = (int)(sqrt(v1) * 75.0 / 17.0 - 161.0);

		int angle = R2_GLOBALS._gfxManagerInstance.getAngle(
			Common::Point(3, 16), Common::Point(k5A7F6[3 * i], k5A7F6[3 * i + 1]));
		int xp = angle * 78 / 9 - 319;

		_arrActor[i].setPosition(Common::Point(xp, yp));
		_arrActor[i].fixPriority(6);
	}

	_actor4.postInit();
	_actor4.setup(1575, 3, 1);
	_actor4.setPosition(Common::Point(48, 81));

	_actor5.postInit();
	_actor5.setup(1575, 3,1);
	_actor5.setPosition(Common::Point(121, 81));

	_actor6.postInit();
	_actor6.setup(1575, 3, 2);
	_actor6.setPosition(Common::Point(203, 80));

	_actor7.postInit();
	_actor7.setup(1575, 3, 2);
	_actor7.setPosition(Common::Point(217, 80));

	_actor8.postInit();
	_actor8.setup(1575, 3, 2);
	_actor8.setPosition(Common::Point(231, 80));

	_actor9.postInit();
	_actor9.setup(1575, 3, 2);
	_actor9.setPosition(Common::Point(273, 91));

	_actor10.postInit();
	_actor10.setup(1575, 3, 2);
	_actor10.setPosition(Common::Point(287, 91));

	// Initialize buttons
	_button1.initButton(1);
	_button2.initButton(2);
	_button3.initButton(3);
	_button4.initButton(4);
	_button5.initButton(5);
	_button6.initButton(6);

	_actor11.postInit();
	_actor11.setup(1575, 4, 2);
	_actor11.setPosition(Common::Point(84, 116));
	_actor11.hide();

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();

	do {
		_field412 = R2_GLOBALS._randomSource.getRandomNumber(20) - 10;
		_field414 = R2_GLOBALS._randomSource.getRandomNumber(20) - 10;
	} while ((_field412) && (_field414));

	if (_field412 < 0)
		_actor4.hide();

	if (_field414 < 0)
		_actor5.hide();

	_field416 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	_field418 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;

	_actor13.postInit();
	_actor13.setup(1575, 2, 4);

	// TODO
	warning("TODO: another immense pile of floating operations");

	_actor12.postInit();
	_actor12.fixPriority(12);

	if (R2_GLOBALS.getFlag(17)) {
		_actor13.setPosition(Common::Point(_actor13._position.x + 5, _actor13._position.y));
		_actor12.setPosition(Common::Point(_actor12._position.x + 5, _actor12._position.y));
	}

	_actor14.postInit();
	_actor14.setup(1575, 5, 1);
	_actor14.setPosition(Common::Point(85, 176));
	_actor14.fixPriority(7);
	_actor14.hide();

	_actor15.postInit();
	_actor15.setup(1575, 5, 2);
	_actor15.setPosition(Common::Point(85, 147));
	_actor15.fixPriority(7);
	_actor15.hide();
}

void Scene1575::remove() {
	SceneExt::remove();
	R2_GLOBALS._uiElements._active = true;
}

void Scene1575::signal() {
	R2_GLOBALS._player.enableControl();
}

void Scene1575::process(Event &event) {
	Scene::process(event);

	g_globals->_sceneObjects->recurse(SceneHandler::dispatchObject);
}

void Scene1575::dispatch() {
	if (_field412 <= 0) {
		++_field412;
		if (_field412 == 0) {
			_actor4.show();
			_field412 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	} else {
		_field412--;
		if (_field412 ==0) {
			_actor4.hide();
			_field412 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	}

	if (_field414 <= 0) {
		++_field414;
		if (_field414 == 0) {
			_actor5.show();
			_field414 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	} else {
		_field414--;
		if (_field414 == 0) {
			_actor5.hide();
			_field414 = R2_GLOBALS._randomSource.getRandomNumber(9) + 1;
		}
	}

	if (_field416 == 0) {
		switch(R2_GLOBALS._randomSource.getRandomNumber(3)) {
		case 0:
			_actor6.hide();
			_actor7.hide();
			_actor8.hide();
			break;
		case 1:
			_actor6.show();
			_actor7.hide();
			_actor8.hide();
			break;
		case 2:
			_actor6.show();
			_actor7.show();
			_actor8.hide();
			break;
		case 3:
			_actor6.show();
			_actor7.show();
			_actor8.show();
			break;
		default:
			break;
		}
		_field416 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	} else {
		--_field416;
	}

	if (_field418 == 0) {
		switch(R2_GLOBALS._randomSource.getRandomNumber(2)) {
		case 0:
			_actor9.hide();
			_actor10.hide();
			break;
		case 1:
			_actor9.show();
			_actor10.hide();
			break;
		case 2:
			_actor9.show();
			_actor10.show();
			break;
		default:
			break;
		}
		_field418 = R2_GLOBALS._randomSource.getRandomNumber(4) + 1;
	} else {
		_field418--;
	}
	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1580 - Inside wreck
 *
 *--------------------------------------------------------------------------*/

Scene1580::Scene1580() {
}

void Scene1580::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
}

bool Scene1580::JoystickPlug::startAction(CursorType action, Event &event) {
	if (action == R2_JOYSTICK) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

		R2_INVENTORY.setObjectScene(R2_JOYSTICK, 1580);
		R2_GLOBALS._sceneItems.remove(&scene->_joystickPlug);
		scene->_joystick.postInit();
		scene->_joystick.setup(1580, 1, 4);
		scene->_joystick.setPosition(Common::Point(159, 163));
		scene->_joystick.setDetails(1550, 78, -1, -1, 2, (SceneItem *) NULL);

		scene->_arrActor[5].remove();

		return true;
	}

	return SceneHotspot::startAction(action, event);
}

bool Scene1580::ScreenSlot::startAction(CursorType action, Event &event) {
	if (action == R2_DIAGNOSTICS_DISPLAY) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

		R2_INVENTORY.setObjectScene(R2_DIAGNOSTICS_DISPLAY, 1580);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneItems.remove(&scene->_screenSlot);

		scene->_screen.postInit();
		scene->_screen.setup(1580, 1, 1);
		scene->_screen.setPosition(Common::Point(124, 108));
		scene->_screen.fixPriority(10);

		if (R2_INVENTORY.getObjectScene(R2_JOYSTICK) == 1580)
			scene->_screen.setDetails(1550, 14, -1, -1, 5, &scene->_joystick);
		else
			scene->_screen.setDetails(1550, 14, -1, -1, 2, (SceneItem *)NULL);

		scene->_screenDisplay.postInit();
		scene->_screenDisplay.setup(1580, 3, 1);
		scene->_screenDisplay.setPosition(Common::Point(124, 109));
		scene->_screenDisplay.fixPriority(20);
		//scene->_field412 = 1;
		scene->_sceneMode = 10;
		scene->setAction(&scene->_sequenceManager, scene, 1, &R2_GLOBALS._player, NULL);

		return true;
	}

	return SceneHotspot::startAction(action, event);
}

bool Scene1580::Joystick::startAction(CursorType action, Event &event) {
	if ( (action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(R2_DIAGNOSTICS_DISPLAY) == 1580)
		&& (R2_INVENTORY.getObjectScene(R2_FUEL_CELL) == 0) && (R2_INVENTORY.getObjectScene(R2_GUIDANCE_MODULE) == 0)
		&& (R2_INVENTORY.getObjectScene(R2_RADAR_MECHANISM) == 0) && (R2_INVENTORY.getObjectScene(R2_GYROSCOPE) == 0)
		&& (R2_INVENTORY.getObjectScene(R2_THRUSTER_VALVE) == 0) && (R2_INVENTORY.getObjectScene(R2_IGNITOR) == 0)) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;
		scene->_sceneMode = 31;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->_stripManager.start(536, scene);
		else
			scene->_stripManager.start(537, scene);

		return true;
	}

	return SceneActor::startAction(action, event);
}

bool Scene1580::Screen::startAction(CursorType action, Event &event) {
	if ((action == CURSOR_USE) && (R2_INVENTORY.getObjectScene(R2_BROKEN_DISPLAY) == 1580)) {
		Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

		R2_INVENTORY.setObjectScene(R2_BROKEN_DISPLAY, R2_GLOBALS._player._characterIndex);
		scene->_screenSlot.setDetails(Rect(69, 29, 177, 108), 1550, 82, -1, -1, 2, NULL);
		scene->_screenDisplay.remove();
		remove();
		return true;
	}

	return SceneActor::startAction(action, event);
}

bool Scene1580::StorageCompartment::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._sceneItems.remove(&scene->_storageCompartment);
	scene->_sceneMode = 0;
	animate(ANIM_MODE_5, scene);

	return true;
}

bool Scene1580::HatchButton::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	setFrame(2);
	scene->_sceneMode = 20;
	scene->setAction(&scene->_sequenceManager, scene, 2, &R2_GLOBALS._player, NULL);

	return true;
}

bool Scene1580::ThrusterValve::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_INVENTORY.setObjectScene(R2_THRUSTER_VALVE, 1);
			remove();
			return true;
		}
		break;
	case R2_COM_SCANNER:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(529, scene);
		return true;
		break;
	case R2_COM_SCANNER_2:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(527, scene);
		return true;
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

bool Scene1580::Ignitor::startAction(CursorType action, Event &event) {
	Scene1580 *scene = (Scene1580 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_INVENTORY.setObjectScene(R2_IGNITOR, 1);
			remove();
			return true;
		}
		break;
	case R2_COM_SCANNER:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(529, scene);
		return true;
		break;
	case R2_COM_SCANNER_2:
		scene->_sceneMode = 30;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		scene->_stripManager.start(527, scene);
		return true;
		break;
	default:
		break;
	}

	return SceneActor::startAction(action, event);
}

void Scene1580::postInit(SceneObjectList *OwnerList) {
	loadScene(1580);
	R2_GLOBALS._sceneManager._fadeMode = FADEMODE_GRADUAL;
	SceneExt::postInit();

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_sceneMode = 0;

	R2_GLOBALS._player.disableControl();
	if (R2_INVENTORY.getObjectScene(R2_JOYSTICK) == 1580) {
		_joystick.postInit();
		_joystick.setup(1580, 1, 4);
		_joystick.setPosition(Common::Point(159, 163));
		_joystick.setDetails(1550, 78, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_joystickPlug.setDetails(Rect(141, 148, 179, 167), 1550, 79, -1, -1, 1, NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_BROKEN_DISPLAY) == 1580) {
		_screen.postInit();
		_screen.setup(1580, 1, 1);
		_screen.setPosition(Common::Point(124, 108));
		_screen.fixPriority(10);
		_screen.setDetails(1550, 13, -1, -1, 1, (SceneItem *) NULL);

		_screenDisplay.postInit();
		_screenDisplay.setup(1580, 1, 3);
		_screenDisplay.setPosition(Common::Point(124, 96));
		_screenDisplay.fixPriority(20);
	} else if (R2_INVENTORY.getObjectScene(R2_DIAGNOSTICS_DISPLAY) == 1580) {
		_screen.postInit();
		_screen.setup(1580, 1, 1);
		_screen.setPosition(Common::Point(124, 108));
		_screen.fixPriority(10);
		_screen.setDetails(1550, 14, -1, -1, 1, (SceneItem *) NULL);

		_screenDisplay.postInit();
		_screenDisplay.setup(1580, 3, 1);
		_screenDisplay.setPosition(Common::Point(124, 109));
		_screenDisplay.fixPriority(20);

		_sceneMode = 10;
	} else {
		_screenSlot.setDetails(Rect(69, 29, 177, 108), 1550, 82, -1, -1, 1, NULL);
	}

	_storageCompartment.postInit();
	if (R2_GLOBALS.getFlag(58) == 0) {
		_storageCompartment.setup(1580, 5, 1);
		_storageCompartment.setDetails(1550, 80, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_storageCompartment.setup(1580, 5, 6);
	}

	_storageCompartment.setPosition(Common::Point(216, 108));
	_storageCompartment.fixPriority(100);

	_hatchButton.postInit();
	_hatchButton.setup(1580, 4, 1);
	_hatchButton.setPosition(Common::Point(291, 147));
	_hatchButton.fixPriority(100);
	_hatchButton.setDetails(1550, 81, -1, -1, 1, (SceneItem *) NULL);

	if (R2_INVENTORY.getObjectScene(R2_THRUSTER_VALVE) == 1580) {
		_thrusterValve.postInit();
		_thrusterValve.setup(1580, 6, 2);
		_thrusterValve.setPosition(Common::Point(222, 108));
		_thrusterValve.fixPriority(50);
		_thrusterValve.setDetails(1550, 32, -1, 34, 1, (SceneItem *) NULL);
	}

	if (R2_INVENTORY.getObjectScene(R2_IGNITOR) == 1580) {
		_ignitor.postInit();
		_ignitor.setup(1580, 6, 1);
		_ignitor.setPosition(Common::Point(195, 108));
		_ignitor.fixPriority(50);
		_ignitor.setDetails(1550, 38, -1, 34, 1, (SceneItem *) NULL);
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 1550, 50, -1, -1, 1, NULL);
}

void Scene1580::signal() {
	switch (_sceneMode++) {
	case 10:
		_screenDisplay.animate(ANIM_MODE_5, this);
		break;
	case 11:
		_screenDisplay.setup(1580, 1, 2);
		_screenDisplay.setPosition(Common::Point(124, 94));

		if (R2_INVENTORY.getObjectScene(R2_GYROSCOPE) != 0) {
			_arrActor[0].postInit();
			_arrActor[0].setup(1580, 2, 1);
			_arrActor[0].setPosition(Common::Point(138, 56));
		}

		if (R2_INVENTORY.getObjectScene(R2_RADAR_MECHANISM) != 0) {
			_arrActor[1].postInit();
			_arrActor[1].setup(1580, 2, 2);
			_arrActor[1].setPosition(Common::Point(140, 66));
		}

		if (R2_INVENTORY.getObjectScene(R2_IGNITOR) != 0) {
			_arrActor[2].postInit();
			_arrActor[2].setup(1580, 2, 3);
			_arrActor[2].setPosition(Common::Point(142, 85));
		}

		if (R2_INVENTORY.getObjectScene(R2_THRUSTER_VALVE) != 0) {
			_arrActor[3].postInit();
			_arrActor[3].setup(1580, 2, 4);
			_arrActor[3].setPosition(Common::Point(142, 92));
		}

		if (R2_INVENTORY.getObjectScene(R2_GUIDANCE_MODULE) != 0) {
			_arrActor[4].postInit();
			_arrActor[4].setup(1580, 2, 5);
			_arrActor[4].setPosition(Common::Point(108, 54));
		}

		if (R2_INVENTORY.getObjectScene(R2_JOYSTICK) != 1580) {
			_arrActor[5].postInit();
			_arrActor[5].setup(1580, 2, 6);
			_arrActor[5].setPosition(Common::Point(110, 64));
		}

		if (R2_INVENTORY.getObjectScene(R2_BATTERY) != 0) {
			_arrActor[6].postInit();
			_arrActor[6].setup(1580, 2, 7);
			_arrActor[6].setPosition(Common::Point(108, 80));
		}

		if (R2_INVENTORY.getObjectScene(R2_FUEL_CELL) != 0) {
			_arrActor[7].postInit();
			_arrActor[7].setup(1580, 2, 8);
			_arrActor[7].setPosition(Common::Point(111, 92));
		}

		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 20:
		R2_GLOBALS._sceneManager.changeScene(1550);
		break;
	case 31:
		R2_GLOBALS._sceneManager.changeScene(1530);
		break;
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1625 - Miranda being questioned
 *
 *--------------------------------------------------------------------------*/

bool Scene1625::Wire::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();

	scene->_sceneMode = 1631;
	scene->_mirandaMouth.postInit();
	scene->setAction(&scene->_sequenceManager, scene, 1631, &scene->_mirandaMouth, &scene->_wire, NULL);
	return true;
}

Scene1625::Scene1625() {
}

void Scene1625::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
}

void Scene1625::postInit(SceneObjectList *OwnerList) {
	loadScene(1625);
	R2_GLOBALS._player._characterIndex = R2_MIRANDA;
	SceneExt::postInit();

	_stripManager.addSpeaker(&_mirandaSpeaker);
	_stripManager.addSpeaker(&_tealSpeaker);
	_stripManager.addSpeaker(&_soldierSpeaker);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();

	_wire.postInit();
	_wire.setup(1626, 2, 1);
	_wire.setPosition(Common::Point(206, 133));
	_wire.setDetails(1625, 0, -1, -1, 1, (SceneItem *) NULL);

	_wristRestraints.postInit();
	_wristRestraints.setup(1625, 8, 1);
	_wristRestraints.setPosition(Common::Point(190, 131));
	_wristRestraints.setDetails(1625, 6, -1, 2, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._player._oldCharacterScene[R2_MIRANDA] == 1625) {
		if (!R2_GLOBALS.getFlag(83)) {
			_glass.postInit();
			_glass.setup(1626, 4, 1);
			_glass.setPosition(Common::Point(96, 166));
			_glass.setDetails(1625, -1, -1, -1, 1, (SceneItem *) NULL);
		}
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
	} else {
		_teal.postInit();
		_teal.fixPriority(10);

		_tealRightArm.postInit();

		R2_GLOBALS._player.disableControl();
		_sceneMode = 1625;
		setAction(&_sequenceManager, this, 1625, &_teal, &_tealRightArm, NULL);
	}

	R2_GLOBALS._sound1.play(245);
	_background.setDetails(Rect(0, 0, 320, 200), 1625, 12, -1, -1, 1, NULL);
	R2_GLOBALS._player._oldCharacterScene[R2_MIRANDA] = 1625;
	R2_GLOBALS._player._characterScene[R2_MIRANDA] = 1625;
}

void Scene1625::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1625::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.disableControl();
		_glass.postInit();
		_glass.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
		_sceneMode = 1626;
		setAction(&_sequenceManager, this, 1626, &_tealHead, &_glass, NULL);
		break;
	case 12:
		MessageDialog::show(DONE_MSG, OK_BTN_STRING);
		break;
	case 14:
		_tealHead.postInit();
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(831, this);
		break;
	case 99:
		R2_GLOBALS._player.disableControl();
		switch (_stripManager._exitMode) {
		case 1:
			_sceneMode = 1627;
			setAction(&_sequenceManager, this, 1627, &_mirandaMouth, &_glass, NULL);
			break;
		case 2:
			_sceneMode = 1629;
			setAction(&_sequenceManager, this, 1629, &_tealHead, &_wristRestraints, NULL);
			break;
		case 4:
			R2_GLOBALS._player._oldCharacterScene[R2_MIRANDA] = 3150;
			R2_GLOBALS._player._characterScene[R2_MIRANDA] = 3150;
			R2_GLOBALS._player._characterIndex = R2_QUINN;
			R2_GLOBALS._sceneManager.changeScene(R2_GLOBALS._player._characterScene[R2_QUINN]);
			break;
		case 5:
			_sceneMode = 1628;
			_tealHead.remove();
			setAction(&_sequenceManager, this, 1628, &_mirandaMouth, &_glass, NULL);
			break;
		case 6:
			_glass.postInit();
			_glass.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
			_sceneMode = 1632;
			setAction(&_sequenceManager, this, 1632, &_glass, NULL);
			break;
		case 7:
			_sceneMode = 1633;
			setAction(&_sequenceManager, this, 1633, &_glass, NULL);
			break;
		case 8:
			_sceneMode = 1635;
			setAction(&_sequenceManager, this, 1635, &_mirandaMouth, &_wristRestraints, NULL);
			break;
		case 9:
			_glass.postInit();
			_glass.setDetails(1625, -1, -1, -1, 2, (SceneItem *) NULL);
			_sceneMode = 1634;
			setAction(&_sequenceManager, this, 1634, &_mirandaMouth, &_wristRestraints, NULL);
			break;
		case 3:
		// No break on purpose
		default:
			_sceneMode = 1630;
			_tealHead.remove();
			setAction(&_sequenceManager, this, 1630, &_teal, &_tealRightArm, NULL);
			break;
		}
		//_field412 = _stripManager._field2E8;
		_stripManager._currObj44Id = 0;
		break;
	case 1625:
		_tealHead.postInit();
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));
		_sceneMode = 10;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(800, this);
		break;
	case 1626:
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));
		_tealHead.show();

		_mirandaMouth.postInit();
		_mirandaMouth.setup(1627, 3, 1);
		_mirandaMouth.setPosition(Common::Point(196, 65));

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(832, this);
		break;
	case 1627:
		_mirandaMouth.setup(1627, 3, 1);
		_mirandaMouth.setPosition(Common::Point(196, 65));
		_mirandaMouth.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(833, this);
		break;
	case 1628:
		R2_GLOBALS.setFlag(83);
		_tealHead.postInit();
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));

		_mirandaMouth.setup(1627, 3, 1);
		_mirandaMouth.setPosition(Common::Point(196, 65));
		_mirandaMouth.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(834, this);
		break;
	case 1629:
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));
		_tealHead.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(805, this);
		break;
	case 1630:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = true;
		break;
	case 1631:
		_mirandaMouth.setup(1627, 3, 1);
		_mirandaMouth.setPosition(Common::Point(196, 65));
		_mirandaMouth.show();

		_wire.remove();

		_teal.postInit();
		_teal.fixPriority(10);

		_tealRightArm.postInit();

		R2_INVENTORY.setObjectScene(R2_SUPERCONDUCTOR_WIRE, 3);
		_sceneMode = 14;

		setAction(&_sequenceManager, this, 1625, &_teal, &_tealRightArm, NULL);
		break;
	case 1632:
		_tealHead.setup(1627, 1, 1);
		_tealHead.setPosition(Common::Point(68, 68));
		_tealHead.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(835, this);
		break;
	case 1633:
		_glass.remove();
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(818, this);
		break;
	case 1634:
		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(836, this);
		break;
	case 1635:
		_mirandaMouth.setup(1627, 3, 1);
		_mirandaMouth.setPosition(Common::Point(196, 65));
		_mirandaMouth.show();

		_sceneMode = 99;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(818, this);
		break;
	default:
		break;
	}
}

void Scene1625::process(Event &event) {
	if ((event.eventType == EVENT_KEYPRESS) && (event.kbd.keycode == Common::KEYCODE_ESCAPE))
		event.handled = true;
	else
		Scene::process(event);
}

/*--------------------------------------------------------------------------
 * Scene 1700 - Rim
 *
 *--------------------------------------------------------------------------*/

Scene1700::Scene1700() {
	_walkFlag = 0;
}

void Scene1700::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_walkFlag);
}

bool Scene1700::RimTransport::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 4;

	Common::Point pt(271, 90);
	PlayerMover *mover = new PlayerMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);

	return true;
}

bool Scene1700::Companion::startAction(CursorType action, Event &event) {
	if (action != CURSOR_TALK)
		return SceneActor::startAction(action, event);

	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;
	scene->_sceneMode = 30;
	scene->signal();

	return true;
}

void Scene1700::NorthExit::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 1;

	Common::Point pt(R2_GLOBALS._player._position.x, 0);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::SouthExit::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 2;

	Common::Point pt(R2_GLOBALS._player._position.x, 170);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::WestExit::changeScene() {
	Scene1700 *scene = (Scene1700 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	_moving = false;
	scene->_sceneMode = 6;

	Common::Point pt(0, R2_GLOBALS._player._position.y);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1700::enterArea() {
	Rect tmpRect;
	R2_GLOBALS._walkRegions.load(1700);

	_slabWest.remove();
	_slabEast.remove();
	_slabShadowWest.remove();
	_slabShadowEast.remove();
	_westPlatform.remove();
	_rimTransportDoor.remove();
	_rimTransport.remove();

	if (_sceneMode != 40) {
		_ledgeHopper.remove();
		_hatch.remove();
	}

	// The original had manual code here to redraw the background manually when
	// changing areas within the scene. Which seems to be totally redundant.

	if (_sceneMode != 40 && R2_GLOBALS._rimLocation == 0) {
		// Crashed ledge hopper
		_ledgeHopper.postInit();
		_ledgeHopper.setup(1701, 1, 1);
		_ledgeHopper.setPosition(Common::Point(220, 137));
		_ledgeHopper.setDetails(1700, 6, -1, -1, 2, (SceneItem *) NULL);
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(12);
	}

	if ((R2_GLOBALS._rimLocation + 2) % 4 == 0) {
		// The slabs forming the bottom of the regular rings the rim transport travels through
		_slabWest.postInit();
		_slabWest.setup(1700, 1, 1);
		_slabWest.setPosition(Common::Point(222, 82));
		_slabWest.setDetails(100, -1, -1, -1, 2, (SceneItem *) NULL);

		_slabShadowWest.postInit();
		_slabShadowWest.setup(1700, 2, 1);
		_slabShadowWest.setPosition(Common::Point(177, 82));
		_slabShadowWest.fixPriority(0);

		_slabShadowEast.postInit();
		_slabShadowEast.setup(1700, 2, 2);
		_slabShadowEast.setPosition(Common::Point(332, 96));
		_slabShadowEast.fixPriority(0);

		_slabEast.postInit();
		_slabEast.setup(1700, 1, 2);
		_slabEast.setPosition(Common::Point(424, 84));

		R2_GLOBALS._walkRegions.disableRegion(11);
	}

	if ((R2_GLOBALS._rimLocation + 399) % 800 == 0) {
		// Enable west exit to lift
		_westPlatform.postInit();
		_westPlatform.setup(1700, 3, 2);
		_westPlatform.setPosition(Common::Point(51, 141));
		_westPlatform.fixPriority(0);
		_westPlatform.setDetails(100, -1, -1, -1, 2, (SceneItem *) NULL);

		_westExit._enabled = true;
	} else {
		R2_GLOBALS._walkRegions.disableRegion(1);
		_westExit._enabled = false;
	}

	if (  ((!R2_GLOBALS.getFlag(15)) && ((R2_GLOBALS._rimLocation == 25) || (R2_GLOBALS._rimLocation == -3)))
		 || ((R2_GLOBALS.getFlag(15)) && (R2_GLOBALS._rimLocation == R2_GLOBALS._rimTransportLocation))
		 ) {
		// Rim transport vechile located
		R2_GLOBALS._rimTransportLocation = R2_GLOBALS._rimLocation;
		if (!R2_GLOBALS.getFlag(15))
			_walkFlag = true;

		_rimTransport.postInit();
		_rimTransport.setup(1700, 3, 1);
		_rimTransport.setPosition(Common::Point(338, 150));
		_rimTransport.setDetails(1700, 9, -1, -1, 2, (SceneItem *) NULL);
		_rimTransport.fixPriority(15);

		_rimTransportDoor.postInit();
		_rimTransportDoor.setup(1700, 4, 1);
		_rimTransportDoor.setPosition(Common::Point(312, 106));
		_rimTransportDoor.fixPriority(130);
	}
}

void Scene1700::postInit(SceneObjectList *OwnerList) {
	loadScene(1700);
	SceneExt::postInit();
	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._sceneManager._previousScene = 1530;

	scalePalette(65, 65, 65);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_northExit.setDetails(Rect(94, 0, 319, 12), EXITCURSOR_N, 1700);
	_southExit.setDetails(Rect(0, 161, 319, 168), EXITCURSOR_S, 1700);
	_westExit.setDetails(Rect(0, 0, 12, 138), EXITCURSOR_W, 1800);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.setPosition(Common::Point(0, 0));
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setVisage(1501);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 1);
	} else {
		R2_GLOBALS._player.setVisage(1506);
		R2_GLOBALS._player._moveDiff = Common::Point(3, 1);
	}

	_companion.postInit();
	_companion.animate(ANIM_MODE_1, NULL);
	_companion.setObjectWrapper(new SceneObjectWrapper());

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		_companion.setVisage(1506);
		_companion._moveDiff = Common::Point(3, 1);
		_companion.setDetails(9002, 1, -1, -1, 1, (SceneItem *) NULL);
	} else {
		_companion.setVisage(1501);
		_companion._moveDiff = Common::Point(2, 1);
		_companion.setDetails(9001, 1, -1, -1, 1, (SceneItem *) NULL);
	}

	R2_GLOBALS._sound1.play(134);

	_playerShadow.postInit();
	_playerShadow.fixPriority(10);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_playerShadow.setVisage(1112);
	else
		_playerShadow.setVisage(1111);

	_playerShadow._effect = EFFECT_SHADOW_MAP;
	_playerShadow._shadowMap = _shadowPaletteMap;
	R2_GLOBALS._player._linkedActor = &_playerShadow;

	_companionShadow.postInit();
	_companionShadow.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_companionShadow.setVisage(1111);
	else
		_companionShadow.setVisage(1112);

	_companionShadow._effect = EFFECT_SHADOW_MAP;
	_companionShadow._shadowMap = _shadowPaletteMap;
	_companion._linkedActor = &_companionShadow;

	R2_GLOBALS._sound1.play(134);

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1530:
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.hide();
		_companion.hide();

		_hatch.postInit();
		_hatch.hide();

		_ledgeHopper.postInit();
		_ledgeHopper.setup(1701, 1, 1);
		_ledgeHopper.setPosition(Common::Point(220, 137));
		_ledgeHopper.setDetails(1700, 6, -1, -1, 1, (SceneItem *) NULL);

		_playerShadow.hide();
		_companionShadow.hide();
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripManager.start(539, this);
		_sceneMode = 40;
		break;
	case 1750: {
		R2_GLOBALS._player.setPosition(Common::Point(282, 121));
		_companion.setPosition(Common::Point(282, 139));
		_sceneMode = 8;
		Common::Point pt(262, 101);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		Common::Point pt2(262, 119);
		NpcMover *mover2 = new NpcMover();
		_companion.addMover(mover2, &pt2, this);
		}
		break;
	case 1800: {
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.setPosition(Common::Point(0, 86));
		_companion.setPosition(Common::Point(0, 64));
		_sceneMode = 7;
		R2_GLOBALS._player.setObjectWrapper(NULL);
		R2_GLOBALS._player._strip = 1;
		Common::Point pt(64, 86);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);
		_companion.setObjectWrapper(NULL);
		_companion._strip = 1;
		Common::Point pt2(77, 64);
		NpcMover *mover2 = new NpcMover();
		_companion.addMover(mover2, &pt2, NULL);
		}
		break;
	default:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setPosition(Common::Point(109, 160));
			_companion.setPosition(Common::Point(156, 160));
			R2_GLOBALS._walkRegions.disableRegion(15);
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(156, 160));
			_companion.setPosition(Common::Point(109, 160));
			R2_GLOBALS._walkRegions.disableRegion(17);
		}
		_sceneMode = 50;
		setAction(&_sequenceManager, this, 1, &R2_GLOBALS._player, NULL);
		break;
	}
	R2_GLOBALS._player._characterScene[R2_QUINN] = 1700;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1700;
	R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1700;
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1700;

	enterArea();

	_surface.setDetails(1, 1700, 3, -1, -1);
	_background.setDetails(Rect(0, 0, 480, 200), 1700, 0, -1, -1, 1, NULL);
}

void Scene1700::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1700::signal() {
	switch (_sceneMode) {
	case 1: {
		_sceneMode = 3;
		if (R2_GLOBALS._rimLocation < 2400)
			++R2_GLOBALS._rimLocation;
		enterArea();
		R2_GLOBALS._player.setPosition(Common::Point(235 - (((((235 - R2_GLOBALS._player._position.x) * 100) / 103) * 167) / 100), 170));
		Common::Point pt(R2_GLOBALS._player._position.x, 160);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);

		if (R2_GLOBALS._player._position.x < 132) {
			_companion.setPosition(Common::Point(156, 170));
			Common::Point pt2(156, 160);
			NpcMover *mover2 = new NpcMover();
			_companion.addMover(mover2, &pt2, NULL);
			R2_GLOBALS._walkRegions.disableRegion(15);
		} else {
			_companion.setPosition(Common::Point(109, 170));
			Common::Point pt3(109, 160);
			NpcMover *mover3 = new NpcMover();
			_companion.addMover(mover3, &pt3, NULL);
			R2_GLOBALS._walkRegions.disableRegion(17);
		}
		}
		break;
	case 2: {
		_sceneMode = 3;
		if (R2_GLOBALS._rimLocation > -2400)
			--R2_GLOBALS._rimLocation;
		enterArea();
		R2_GLOBALS._player.setPosition(Common::Point(235 - (((((235 - R2_GLOBALS._player._position.x) * 100) / 167) * 103) / 100), 0));
		Common::Point pt(R2_GLOBALS._player._position.x, 10);
		NpcMover *mover = new NpcMover();
		R2_GLOBALS._player.addMover(mover, &pt, this);

		if (R2_GLOBALS._player._position.x >= 171) {
			_companion.setPosition(Common::Point(155, 0));
			Common::Point pt2(155, 10);
			NpcMover *mover2 = new NpcMover();
			_companion.addMover(mover2, &pt2, NULL);
			R2_GLOBALS._walkRegions.disableRegion(15);
		} else {
			_companion.setPosition(Common::Point(188, 0));
			Common::Point pt3(188, 10);
			NpcMover *mover3 = new NpcMover();
			_companion.addMover(mover3, &pt3, NULL);
			R2_GLOBALS._walkRegions.disableRegion(17);
		}
		}
		break;
	case 3:
		if (!_walkFlag) {
			R2_GLOBALS._player.enableControl(CURSOR_WALK);
		} else {
			R2_GLOBALS.setFlag(15);
			_walkFlag = false;
			_sceneMode = 31;
			R2_GLOBALS._events.setCursor(CURSOR_WALK);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN)
				_stripManager.start(542, this);
			else
				_stripManager.start(543, this);
		}
		break;
	case 4: {
		_sceneMode = 5;
		Common::Point pt(271, 90);
		PlayerMover *mover = new PlayerMover();
		_companion.addMover(mover, &pt, NULL);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			setAction(&_sequenceManager, this, 1700, &R2_GLOBALS._player, &_rimTransportDoor, NULL);
		else
			setAction(&_sequenceManager, this, 1701, &R2_GLOBALS._player, &_rimTransportDoor, NULL);
		}
		break;
	case 5:
		R2_GLOBALS._sceneManager.changeScene(1750);
		break;
	case 6:
		R2_GLOBALS._sceneManager.changeScene(1800);
		break;
	case 7:
		R2_GLOBALS._player.setObjectWrapper(new SceneObjectWrapper());
		R2_GLOBALS._player._strip = 1;
		_companion.setObjectWrapper(new SceneObjectWrapper());
		_companion._strip = 1;
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		R2_GLOBALS._walkRegions.disableRegion(14);
		break;
	case 8:
		R2_GLOBALS._player._strip = 2;
		_companion._strip = 1;
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		R2_GLOBALS._walkRegions.disableRegion(12);
		break;
	case 30:
		_sceneMode = 31;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			_stripManager.start(540, this);
		else
			_stripManager.start(541, this);
		break;
	case 31:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 40:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1704;
		setAction(&_sequenceManager, this, 1704, &R2_GLOBALS._player, &_companion,
			&_hatch, &_ledgeHopper, &_playerShadow, &_companionShadow, NULL);
		break;
	case 50:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			R2_GLOBALS._walkRegions.disableRegion(15);
		else
			R2_GLOBALS._walkRegions.disableRegion(17);

		R2_GLOBALS._player.enableControl();
		break;
	case 1704:
		R2_GLOBALS._sound1.play(134);
		R2_GLOBALS._walkRegions.disableRegion(15);
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._walkRegions.disableRegion(12);
		R2_GLOBALS._player.fixPriority(-1);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1750 - Maintaiance Vechile
 *
 *--------------------------------------------------------------------------*/

Scene1750::Button::Button() {
	_buttonId = 0;
}

void Scene1750::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_buttonId);
}

bool Scene1750::Button::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1750 *scene = (Scene1750 *)R2_GLOBALS._sceneManager._scene;

	switch (_buttonId) {
	case 1:
		// Forward button
		show();
		scene->_backwardButton.hide();
		if (scene->_speed < 0)
			scene->_speed = -scene->_speed;
		scene->_direction = 1;
		break;
	case 2:
		// Backwards button
		show();
		scene->_forwardButton.hide();
		if (scene->_speed > 0)
			scene->_speed = -scene->_speed;
		scene->_direction = -1;
		break;
	case 3:
		// Exit button
		if (scene->_rotation->_idxChange == 0) {
			show();
			R2_GLOBALS._sceneManager.changeScene(1700);
		} else {
			scene->_speed = 0;
			scene->_speedSlider._moveRate = 20;
			scene->_forwardButton._moveDiff.y = 1;
			Common::Point pt(286, 143);
			NpcMover *mover = new NpcMover();
			scene->_speedSlider.addMover(mover, &pt, NULL);
		}
	default:
		break;
	}

	return true;
}

/*------------------------------------------------------------------------*/

Scene1750::SpeedSlider::SpeedSlider() {
	_incrAmount = 0;
	_xp = 0;
	_ys = 0;
	_height = 0;
	_thumbHeight = 0;
	_mouseDown = false;
}

void Scene1750::SpeedSlider::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_incrAmount);
	s.syncAsSint16LE(_xp);
	s.syncAsSint16LE(_ys);
	s.syncAsSint16LE(_height);
	s.syncAsSint16LE(_thumbHeight);
	s.syncAsSint16LE(_mouseDown);
}

void Scene1750::SpeedSlider::setupSlider(int incrAmount, int xp, int ys, int height, int thumbHeight) {
	_mouseDown = false;
	_incrAmount = incrAmount;
	_xp = xp;
	_ys = ys;
	_height = height;
	_thumbHeight = thumbHeight;

	postInit();
	setup(1750, 1, 1);
	fixPriority(255);
	setPosition(Common::Point(_xp, _ys + ((_height * (incrAmount - 1)) / (_thumbHeight - 1))));
}

void Scene1750::SpeedSlider::calculateSlider() {
	Scene1750 *scene = (Scene1750 *)R2_GLOBALS._sceneManager._scene;

	int tmpVar = (_height / (_thumbHeight - 1)) / 2;
	int tmpVar2 = ((_position.y - _ys + tmpVar) * _thumbHeight) / (_height + 2 * tmpVar);

	setPosition(Common::Point(_xp, _ys + ((_height * tmpVar2) / (_thumbHeight - 1))));
	scene->_speed = scene->_direction * tmpVar2;
}

void Scene1750::SpeedSlider::process(Event &event) {
	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_USE) &&
			(_bounds.contains(event.mousePos))) {
		_mouseDown = true;
		event.eventType = EVENT_NONE;
	}

	if ((event.eventType == EVENT_BUTTON_UP) && _mouseDown) {
		_mouseDown = false;
		event.handled = true;
		addMover(NULL);
		calculateSlider();
	}

	if (_mouseDown) {
		event.handled = true;
		if (event.mousePos.y >= _ys) {
			if (_ys + _height >= event.mousePos.y)
				setPosition(Common::Point(_xp, event.mousePos.y));
			else
				setPosition(Common::Point(_xp, _ys + _height));
		} else {
			setPosition(Common::Point(_xp, _ys));
		}
	}
}

bool Scene1750::SpeedSlider::startAction(CursorType action, Event &event) {
	if (action == CURSOR_USE)
		return SceneActor::startAction(action, event);

	return false;
}

/*------------------------------------------------------------------------*/

Scene1750::Scene1750() {
	_direction = 0;
	_speedCurrent = 0;
	_speed = 0;
	_speedDelta = 0;
	_rotationSegment = 0;
	_rotationSegCurrent = 0;
	_newRotation = 0;

	_rotation = nullptr;
}

void Scene1750::synchronize(Serializer &s) {
	SceneExt::synchronize(s);
	SYNC_POINTER(_rotation);

	s.syncAsSint16LE(_direction);
	s.syncAsSint16LE(_speedCurrent);
	s.syncAsSint16LE(_speed);
	s.syncAsSint16LE(_speedDelta);
	s.syncAsSint16LE(_rotationSegment);
	s.syncAsSint16LE(_rotationSegCurrent);
	s.syncAsSint16LE(_newRotation);
}

void Scene1750::postInit(SceneObjectList *OwnerList) {
	loadScene(1750);
	R2_GLOBALS._sound1.play(115);
	R2_GLOBALS._uiElements._active = false;

	SceneExt::postInit();
	R2_GLOBALS._interfaceY = SCREEN_HEIGHT;

	R2_GLOBALS._player._characterScene[R2_QUINN] = 1750;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1750;
	R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1750;
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1750;

	_rotation = R2_GLOBALS._scenePalette.addRotation(224, 254, 1);
	_rotation->setDelay(0);
	_rotation->_idxChange = 0;
	_rotation->_countdown = 2;

	switch ((R2_GLOBALS._rimLocation + 2) % 4) {
	case 0:
		_rotation->_currIndex = 247;
		break;
	case 1:
		_rotation->_currIndex = 235;
		break;
	case 2:
		_rotation->_currIndex = 239;
		break;
	case 3:
		_rotation->_currIndex = 243;
		break;
	default:
		break;
	}

	byte tmpPal[768];

	for (int i = 224; i < 255; i++) {
		int tmpIndex = _rotation->_currIndex - 224;
		if (tmpIndex > 254)
			tmpIndex -= 31;
		tmpPal[3 * i] = R2_GLOBALS._scenePalette._palette[3 * tmpIndex];
		tmpPal[(3 * i) + 1] = R2_GLOBALS._scenePalette._palette[(3 * tmpIndex) + 1];
		tmpPal[(3 * i) + 2] = R2_GLOBALS._scenePalette._palette[(3 * tmpIndex) + 2];
	}

	for (int i = 224; i < 255; i++) {
		R2_GLOBALS._scenePalette._palette[3 * i] = tmpPal[3 * i];
		R2_GLOBALS._scenePalette._palette[(3 * i) + 1] = tmpPal[(3 * i) + 1];
		R2_GLOBALS._scenePalette._palette[(3 * i) + 2] = tmpPal[(3 * i) + 2];
	}

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();
	R2_GLOBALS._player.enableControl();

	_radarSweep.postInit();
	_radarSweep.setup(1750, 3, 1);
	_radarSweep.setPosition(Common::Point(49, 185));
	_radarSweep.fixPriority(7);
	_radarSweep.setDetails(1750, 30, -1, -1, 1, (SceneItem *) NULL);

	_scannerIcon.postInit();
	_scannerIcon.setup(1750, 2, 1);
	_scannerIcon.setPosition(Common::Point(35, ((_rotation->_currIndex - 218) % 4) + ((R2_GLOBALS._rimLocation % 800) * 4) - 1440));
	_scannerIcon.fixPriority(8);

	_redLights.postInit();
	_redLights.setup(1750, 1, 4);

	int tmpVar = ABS(_scannerIcon._position.y - 158) / 100;

	if (tmpVar >= 8)
		_redLights.hide();
	else if (_scannerIcon._position.y <= 158)
		_redLights.setPosition(Common::Point(137, (tmpVar * 7) + 122));
	else
		_redLights.setPosition(Common::Point(148, (tmpVar * 7) + 122));

	_speedSlider.setupSlider(1, 286, 143, 41, 15);
	_speedSlider.setDetails(1750, 24, 1, -1, 1, (SceneItem *) NULL);

	_forwardButton.postInit();
	_forwardButton._buttonId = 1;
	_forwardButton.setup(1750, 1, 2);
	_forwardButton.setPosition(Common::Point(192, 140));
	_forwardButton.setDetails(1750, 18, 1, -1, 1, (SceneItem *) NULL);

	_backwardButton.postInit();
	_backwardButton._buttonId = 2;
	_backwardButton.setup(1750, 1, 3);
	_backwardButton.setPosition(Common::Point(192, 163));
	_backwardButton.setDetails(1750, 18, 1, -1, 1, (SceneItem *) NULL);
	_backwardButton.hide();

	_exitButton.postInit();
	_exitButton._buttonId = 3;
	_exitButton.setup(1750, 1, 5);
	_exitButton.setPosition(Common::Point(230, 183));
	_exitButton.setDetails(1750, 27, 1, -1, 1, (SceneItem *) NULL);

	_direction = 1;		// Forward by default
	_speedDelta = 0;
	_speedCurrent = 0;
	_speed = 0;
	_rotationSegment = ((_rotation->_currIndex - 218) / 4) % 4;

	_redLightsDescr.setDetails(Rect(129, 112, 155, 175), 1750, 21, -1, -1, 1, NULL);
	_greenLights.setDetails(Rect(93, 122, 126, 172), 1750, 15, -1, -1, 1, NULL);
	_frontView.setDetails(Rect(3, 3, 157, 99), 1750, 9, -1, -1, 1, NULL);
	_rearView.setDetails(Rect(162, 3, 316, 99), 1750, 12, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 1750, 6, 1, -1, 1, NULL);
}

void Scene1750::remove() {
	if (R2_GLOBALS._rimLocation == 2400)
		R2_GLOBALS._rimLocation = 2399;

	if (R2_GLOBALS._rimLocation == -2400)
		R2_GLOBALS._rimLocation = -2399;

	R2_GLOBALS._rimTransportLocation = R2_GLOBALS._rimLocation;

	SceneExt::remove();
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._uiElements._active = true;
}

void Scene1750::signal() {
	R2_GLOBALS._player.enableControl();
}

void Scene1750::process(Event &event) {
	Scene::process(event);
	if (!event.handled)
		_speedSlider.process(event);
}

void Scene1750::dispatch() {
	if (_rotation) {
		if (!_speedDelta && (_speed != _speedCurrent)) {
			if (_speedCurrent >= _speed)
				--_speedCurrent;
			else
				++_speedCurrent;

			_speedDelta = 21 - ABS(_speedCurrent);
		}

		if (_speedDelta == 1) {
			if (_speedCurrent == 0) {
				_radarSweep.show();
				_rotation->_idxChange = 0;
			} else {
				if (_rotation->_idxChange == 0)
					_radarSweep.hide();

				if (_speedCurrent < -12) {
					_rotation->setDelay(15 - ABS(_speedCurrent));
					_rotation->_idxChange = -2;
				} else if (_speedCurrent < 0) {
					_rotation->setDelay(10 - ABS(_speedCurrent));
					_rotation->_idxChange = -1;
				} else if (_speedCurrent < 11) {
					_rotation->setDelay(10 - _speedCurrent);
					_rotation->_idxChange = 1;
				} else {
					_rotation->setDelay(15 - _speedCurrent);
					_rotation->_idxChange = 2;
				}
			}
		}

		if (_speedDelta)
			--_speedDelta;

		_rotationSegCurrent = _rotationSegment;
		_rotationSegment = ((_rotation->_currIndex - 218) / 4) % 4;

		if ((_rotationSegCurrent + 1) == _rotationSegment || (_rotationSegCurrent - 3)  == _rotationSegment) {
			if (R2_GLOBALS._rimLocation < 2400) {
				++R2_GLOBALS._rimLocation;
			}
		}

		if ((_rotationSegCurrent - 1) == _rotationSegment || (_rotationSegCurrent + 3) == _rotationSegment) {
			if (R2_GLOBALS._rimLocation > -2400) {
				--R2_GLOBALS._rimLocation;
			}
		}

		if (_rotation->_currIndex != _newRotation) {
			// Handle setting the position of the lift icon in the scanner display
			_newRotation = _rotation->_currIndex;
			_scannerIcon.setPosition(Common::Point(35, ((_rotation->_currIndex - 218) % 4) +
				((R2_GLOBALS._rimLocation % 800) * 4) - 1440));
		}
	}

	int v = ABS(_scannerIcon._position.y - 158) / 100;
	if (v < 8) {
		// Show how close the user is to the lift on the second column of lights
		_redLights.show();
		_redLights.setPosition(Common::Point((_scannerIcon._position.y <= 158) ? 137 : 148,
			v * 7 + 122));
	} else {
		_redLights.hide();
	}
}

/*--------------------------------------------------------------------------
 * Scene 1800 - Rim Lift Exterior
 *
 *--------------------------------------------------------------------------*/

Scene1800::Scene1800() {
	_locationMode = 0;
}

void Scene1800::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_locationMode);
}

bool Scene1800::Background::startAction(CursorType action, Event &event) {
	if ((action != R2_COM_SCANNER) && (action != R2_COM_SCANNER_2))
		return false;

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS._rimLocation == 1201) {
			scene->_stripManager.start(548, this);
		} else if (R2_GLOBALS.getFlag(66)) {
			return false;
		} else {
			scene->_stripManager.start(546, this);
		}
	} else {
		if (R2_GLOBALS._rimLocation == 1201) {
			scene->_stripManager.start(549, this);
		} else if (R2_GLOBALS.getFlag(66)) {
			return false;
		} else {
			scene->_stripManager.start(547, this);
		}
	}

	R2_GLOBALS.setFlag(66);
	return true;
}

bool Scene1800::Lever::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if (!R2_GLOBALS.getFlag(14))
		return false;

	if (R2_GLOBALS._player._characterIndex != R2_QUINN)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;
	R2_GLOBALS._player.disableControl();

	if (_frame == 1) {
		R2_GLOBALS.setFlag(64);
		scene->_sceneMode = 1810;
		scene->setAction(&scene->_sequenceManager, scene, 1810, &R2_GLOBALS._player, &scene->_lever, &scene->_leftStaircase, &scene->_rightStaircase, NULL);
	} else {
		R2_GLOBALS.clearFlag(64);
		scene->_sceneMode = 1811;
		scene->setAction(&scene->_sequenceManager, scene, 1811, &R2_GLOBALS._player, &scene->_lever, &scene->_leftStaircase, &scene->_rightStaircase, NULL);
	}
	return true;
}

bool Scene1800::Doors::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
		// Seeker trying to force open the door
		R2_GLOBALS._player.disableControl();
		if (scene->_locationMode >= 2) {
			if (R2_GLOBALS.getFlag(14)) {
				// Allow door to close
				scene->_sceneMode = 1809;
				scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_doors, NULL);
				R2_GLOBALS.clearFlag(14);
			} else {
				// Force open door
				scene->_sceneMode = 1808;
				scene->setAction(&scene->_sequenceManager, scene, 1808, &R2_GLOBALS._player, &scene->_doors, NULL);
				R2_GLOBALS.setFlag(14);
			}
		} else {
			// Seeker failing to force open doors
			scene->_sceneMode = 1813;
			// Original was using 1813 in setAction too, but it somewhat broken.
			// Seeker goes 2 pixels to high, hiding behind the door
			scene->setAction(&scene->_sequenceManager, scene, 1808, &R2_GLOBALS._player, &scene->_doors, NULL);
		}
	} else if (R2_GLOBALS.getFlag(14)) {
		return SceneActor::startAction(action, event);
	} else {
		// Quinn trying to force open doors
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 1812;
		scene->setAction(&scene->_sequenceManager, scene, 1812, &R2_GLOBALS._player, NULL);
	}

	return true;
}

bool Scene1800::PassengerDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	if (_position.x < 160) {
		if (scene->_leftStaircase._frame == 1) {
			return SceneActor::startAction(action, event);
		} else {
			R2_GLOBALS.setFlag(29);
			R2_GLOBALS._player.disableControl();
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 1;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &scene->_companion, &scene->_doors, NULL);
					R2_GLOBALS.clearFlag(14);
				} else {
					scene->_sceneMode = 1804;
					scene->setAction(&scene->_sequenceManager, scene, 1804, &R2_GLOBALS._player, &scene->_companion, &scene->_leftDoor, NULL);
				}
			} else {
				if (R2_GLOBALS.getFlag(14)) {
					scene->_sceneMode = 1;
					scene->setAction(&scene->_sequenceManager, scene, 1809, &scene->_doors, NULL);
					R2_GLOBALS.clearFlag(14);
				} else {
					scene->_sceneMode = 1805;
					scene->setAction(&scene->_sequenceManager, scene, 1805, &R2_GLOBALS._player, &scene->_companion, &scene->_leftDoor, NULL);
				}
			}
		}
	} else if (scene->_leftStaircase._frame == 1) {
		return SceneActor::startAction(action, event);
	} else {
		R2_GLOBALS.clearFlag(29);
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(14)) {
				scene->_sceneMode = 2;
				scene->setAction(&scene->_sequenceManager, scene, 1809, &scene->_companion, &scene->_doors, NULL);
				R2_GLOBALS.clearFlag(14);
			} else {
				scene->_sceneMode = 1806;
				scene->setAction(&scene->_sequenceManager, scene, 1806, &R2_GLOBALS._player, &scene->_companion, &scene->_rightDoor, NULL);
			}
		} else {
			if (R2_GLOBALS.getFlag(14)) {
				scene->_sceneMode = 2;
				scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_doors, NULL);
				R2_GLOBALS.clearFlag(14);
			} else {
				scene->_sceneMode = 1807;
				scene->setAction(&scene->_sequenceManager, scene, 1807, &R2_GLOBALS._player, &scene->_companion, &scene->_rightDoor, NULL);
			}
		}
	}

	return true;
}

void Scene1800::SouthExit::changeScene() {
	Scene1800 *scene = (Scene1800 *)R2_GLOBALS._sceneManager._scene;

	_enabled = false;
	R2_GLOBALS._events.setCursor(CURSOR_WALK);
	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS.getFlag(14)) {
		scene->_sceneMode = 3;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 1809, &scene->_companion, &scene->_doors, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1809, &R2_GLOBALS._player, &scene->_doors, NULL);
		R2_GLOBALS.clearFlag(14);
	} else {
		scene->_sceneMode = 1802;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			scene->setAction(&scene->_sequenceManager, scene, 1802, &R2_GLOBALS._player, &scene->_companion, NULL);
		else
			scene->setAction(&scene->_sequenceManager, scene, 1803, &R2_GLOBALS._player, &scene->_companion, NULL);
	}
}

void Scene1800::postInit(SceneObjectList *OwnerList) {
	loadScene(1800);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(116);
	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	if (R2_GLOBALS._sceneManager._previousScene == -1)
		R2_GLOBALS._rimLocation = 1201;

	// Set the mode based on whether this is the "correct" lift or not
	if (R2_GLOBALS._rimLocation == 1201)
		_locationMode = 2;
	else
		_locationMode = 0;

	scalePalette(65, 65, 65);
	_southExit.setDetails(Rect(0, 160, 319, 168), EXITCURSOR_S, 1800);
	_background.setDetails(Rect(0, 0, 320, 200), -1, -1, -1, -1, 1, NULL);

	_lever.postInit();
	_lever.setup(1801, 4, 1);
	_lever.setPosition(Common::Point(170, 124));
	_lever.setDetails(1800, 13, 14, 15, 1, (SceneItem *) NULL);

	_doors.postInit();
	_doors.setup(1801, 3, 1);
	_doors.setPosition(Common::Point(160, 139));
	_doors.setDetails(1800, 6, -1, -1, 1, (SceneItem *) NULL);

	_leftDoor.postInit();
	_leftDoor.setup(1800, 1, 1);
	_leftDoor.setPosition(Common::Point(110, 78));
	_leftDoor.fixPriority(135);
	_leftDoor.setDetails(1800, 20, -1, -1, 1, (SceneItem *) NULL);

	_rightDoor.postInit();
	_rightDoor.setup(1800, 2, 1);
	_rightDoor.setPosition(Common::Point(209, 78));
	_rightDoor.fixPriority(135);
	_rightDoor.setDetails(1800, 20, -1, -1, 1, (SceneItem *) NULL);

	_leftStaircase.postInit();
	if ((_locationMode != 1) && (_locationMode != 3) && (!R2_GLOBALS.getFlag(64)))
		_leftStaircase.setup(1801, 2, 1);
	else
		_leftStaircase.setup(1801, 2, 10);
	_leftStaircase.setPosition(Common::Point(76, 142));
	_leftStaircase.setDetails(1800, 3, -1, -1, 1, (SceneItem *) NULL);

	_rightStaircase.postInit();
	if ((_locationMode != 1) && (_locationMode != 3) && (!R2_GLOBALS.getFlag(64)))
		_rightStaircase.setup(1801, 1, 1);
	else
		_rightStaircase.setup(1801, 1, 10);
	_rightStaircase.setPosition(Common::Point(243, 142));
	_rightStaircase.setDetails(1800, 3, -1, -1, 1, (SceneItem *) NULL);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		// Standard Quinn setup
		R2_GLOBALS._player.setVisage(1503);
		R2_GLOBALS._player._moveDiff = Common::Point(2, 2);
	} else {
		// Seeker setup dependent on whether he's holding the doors or not
		if (R2_GLOBALS.getFlag(14)) {
			R2_GLOBALS._player.animate(ANIM_MODE_NONE, NULL);
			R2_GLOBALS._player.setObjectWrapper(NULL);
			R2_GLOBALS._player.setup(1801, 5, 12);
			R2_GLOBALS._player.setPosition(Common::Point(160, 139));
			R2_GLOBALS._walkRegions.disableRegion(9);
			_doors.hide();
		} else {
			R2_GLOBALS._player.setVisage(1507);
		}
		R2_GLOBALS._player._moveDiff = Common::Point(4, 2);
	}

	_companion.postInit();
	_companion.animate(ANIM_MODE_1, NULL);
	_companion.setObjectWrapper(new SceneObjectWrapper());
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		if (R2_GLOBALS.getFlag(14)) {
			_companion.animate(ANIM_MODE_NONE, NULL);
			_companion.setObjectWrapper(NULL);
			_companion.setup(1801, 5, 12);

			R2_GLOBALS._walkRegions.disableRegion(9);
			_doors.hide();
		} else {
			_companion.setup(1507, 1, 1);
			_companion.setPosition(Common::Point(180, 160));
		}
		_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
		_companion._moveDiff = Common::Point(4, 2);
	} else {
		_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);
		_companion.setVisage(1503);
		_companion._moveDiff = Common::Point(2, 2);
	}

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1800) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			R2_GLOBALS._player.setPosition(Common::Point(114, 150));
			R2_GLOBALS._player.setStrip(5);
			if (R2_GLOBALS.getFlag(14)) {
				_companion.setPosition(Common::Point(160, 139));
				R2_GLOBALS._walkRegions.disableRegion(8);
			} else {
				_companion.setPosition(Common::Point(209, 150));
				_companion.setStrip(6);
				R2_GLOBALS._walkRegions.disableRegion(8);
			}
		} else {
			if (R2_GLOBALS.getFlag(14)) {
				R2_GLOBALS._player.setup(1801, 5, 12);
				R2_GLOBALS._player.setPosition(Common::Point(160, 139));
			} else {
				R2_GLOBALS._player.setPosition(Common::Point(209, 150));
				R2_GLOBALS._player.setStrip(6);
			}
			_companion.setPosition(Common::Point(114, 150));
			_companion.setStrip(5);
			R2_GLOBALS._walkRegions.disableRegion(10);
			R2_GLOBALS._walkRegions.disableRegion(11);
		}
	} else if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player.setPosition(Common::Point(140, 160));
		_companion.setPosition(Common::Point(180, 160));
	} else {
		R2_GLOBALS._player.setPosition(Common::Point(180, 160));
		_companion.setPosition(Common::Point(140, 160));
	}

	_playerShadow.postInit();
	_playerShadow.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_playerShadow.setVisage(1111);
	else
		_playerShadow.setVisage(1110);

	_playerShadow._effect = EFFECT_SHADOW_MAP;
	_playerShadow._shadowMap = _shadowPaletteMap;

	R2_GLOBALS._player._linkedActor = &_playerShadow;

	_companionShadow.postInit();
	_companionShadow.fixPriority(10);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_companionShadow.setVisage(1110);
	else
		_companionShadow.setVisage(1111);

	_companionShadow._effect = EFFECT_SHADOW_MAP;
	_companionShadow._shadowMap = _shadowPaletteMap;

	_companion._linkedActor = &_companionShadow;

	R2_GLOBALS._player._characterScene[R2_QUINN] = 1800;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1800;

	_elevatorContents.setDetails(Rect(128, 95, 190, 135), 1800, 10, -1, -1, 1, NULL);
	_elevator.setDetails(Rect(95, 3, 223, 135), 1800, 0, -1, -1, 1, NULL);

	// Original was calling _item3.setDetails(Rect(1800, 11, 24, 23), 25, -1, -1, -1, 1, NULL);
	// This is *wrong*. The following statement is a wild guess based on good common sense
	_surface.setDetails(11, 1800, 23, 24, 25);
	_secBackground.setDetails(Rect(0, 0, 320, 200), 1800, 17, -1, 19, 1, NULL);

	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1800) {
		if ((R2_GLOBALS.getFlag(14)) && (R2_GLOBALS._player._characterIndex == R2_SEEKER)) {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_WALK);
		}
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1850) {
		if (R2_GLOBALS.getFlag(29)) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				_sceneMode = 1814;
				setAction(&_sequenceManager, this, 1814, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
			} else {
				_sceneMode = 1815;
				setAction(&_sequenceManager, this, 1815, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
			}
		} else if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1816;
			setAction(&_sequenceManager, this, 1816, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
		} else {
			_sceneMode = 1817;
			setAction(&_sequenceManager, this, 1817, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
		}
	} else if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		_sceneMode = 1800;
		setAction(&_sequenceManager, this, 1800, &R2_GLOBALS._player, &_companion, NULL);
	} else {
		_sceneMode = 1801;
		setAction(&_sequenceManager, this, 1801, &R2_GLOBALS._player, &_companion, NULL);
	}

	R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1800;
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1800;
}

void Scene1800::signal() {
	switch (_sceneMode) {
	case 1:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1804;
			setAction(&_sequenceManager, this, 1804, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
		} else {
			_sceneMode = 1805;
			setAction(&_sequenceManager, this, 1805, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
		}
		break;
	case 2:
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_sceneMode = 1806;
			setAction(&_sequenceManager, this, 1806, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
		} else {
			_sceneMode = 1807;
			setAction(&_sequenceManager, this, 1807, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
		}
		break;
	case 3:
		_sceneMode = 1802;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			setAction(&_sequenceManager, this, 1802, &R2_GLOBALS._player, &_companion, NULL);
		else
			setAction(&_sequenceManager, this, 1803, &R2_GLOBALS._player, &_companion, NULL);
		break;
	case 10:
	// No break on purpose
	case 11:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 12:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		R2_GLOBALS._player._canWalk = false;
		break;
	case 13:
		_sceneMode = 14;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS._player.setup(1801, 7, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_8, 0, NULL);
		_stripManager.start(550, this);
		break;
	case 14:
		_sceneMode = 15;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.setup(1801, 6, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_6, this);
		break;
	case 15:
		R2_GLOBALS._player.setup(1503, 4, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	// Cases 23 and 24 have been added to fix missing hardcoded logic in the original,
	// when Seeker tries to open the door
	case 23:
		_sceneMode = 24;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		R2_GLOBALS._player.setup(1801, 5, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_8, 0, NULL);
		_stripManager.start(550, this);
		break;
	case 24:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._player.setup(1507, 4, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.enableControl(CURSOR_USE);

		_doors.setup(1801, 3, 1);
		_doors.setPosition(Common::Point(160, 139));
		_doors.setDetails(1800, 6, -1, -1, 1, (SceneItem *) NULL);
		_doors.show();

		R2_GLOBALS._player._position.y += 2;
		R2_GLOBALS._player.show();
		break;
	case 1800:
		R2_GLOBALS._walkRegions.disableRegion(8);
		if (R2_GLOBALS.getFlag(63))
			R2_GLOBALS._player.enableControl(CURSOR_USE);
		else {
			_sceneMode = 10;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_stripManager.start(544, this);
		}
		break;
	case 1801:
		R2_GLOBALS._walkRegions.disableRegion(10);
		R2_GLOBALS._walkRegions.disableRegion(11);
		R2_GLOBALS.setFlag(63);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	case 1802:
		R2_GLOBALS.clearFlag(14);
		R2_GLOBALS._sceneManager.changeScene(1700);
		break;
	case 1804:
	// No break on purpose
	case 1805:
	// No break on purpose
	case 1806:
	// No break on purpose
	case 1807:
		R2_GLOBALS.clearFlag(14);
		R2_GLOBALS._sceneManager.changeScene(1850);
		break;
	case 1808:
		_sceneMode = 12;
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripManager.start(553, this);
		break;
	case 1812:
		_sceneMode = 13;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	// Case 1813 has been added to fix Seeker missing animation in the original game
	case 1813:
		_sceneMode = 23;
		R2_GLOBALS._player.animate(ANIM_MODE_5, this);
		break;
	case 1814:
	// No break on purpose
	case 1815:
		R2_GLOBALS._walkRegions.disableRegion(10);
		R2_GLOBALS._walkRegions.disableRegion(11);
		R2_GLOBALS._player.enableControl();
		break;
	case 1816:
	// No break on purpose
	case 1817:
		R2_GLOBALS._walkRegions.disableRegion(8);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene1800::saveCharacter(int characterIndex) {
	if (R2_GLOBALS._player._characterIndex == R2_MIRANDA)
		R2_GLOBALS._sound1.fadeOut2(NULL);

	SceneExt::saveCharacter(characterIndex);
}

/*--------------------------------------------------------------------------
 * Scene 1850 - Rim Lift Interior
 *
 *--------------------------------------------------------------------------*/

bool Scene1850::Button::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	Scene1850 *scene = (Scene1850 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		scene->_sceneMode = 1852;
		if (R2_GLOBALS.getFlag(32))
			scene->setAction(&scene->_sequenceManager1, scene, 1871, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 1852, &R2_GLOBALS._player, NULL);
	} else if (R2_GLOBALS.getFlag(30)) {
		scene->_seqNumber = 1;
		scene->_sceneMode = 1860;

		if (R2_GLOBALS.getFlag(32))
			scene->setAction(&scene->_sequenceManager1, scene, 1860, &R2_GLOBALS._player, &scene->_robot, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 1859, &R2_GLOBALS._player, &scene->_robot, NULL);

		R2_GLOBALS.clearFlag(30);
	} else {
		scene->_sceneMode = 1853;

		if (R2_GLOBALS.getFlag(32))
			scene->setAction(&scene->_sequenceManager1, scene, 1872, &R2_GLOBALS._player, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 1853, &R2_GLOBALS._player, NULL);
	}

	return true;
}

bool Scene1850::Robot::startAction(CursorType action, Event &event) {
	Scene1850 *scene = (Scene1850 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case CURSOR_USE:
		if ((R2_GLOBALS._player._characterIndex != R2_SEEKER) || R2_GLOBALS.getFlag(33) || R2_GLOBALS.getFlag(30))
			return SceneActor::startAction(action, event);

		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 1857;

		if (R2_GLOBALS.getFlag(32))
			scene->setAction(&scene->_sequenceManager1, scene, 1858, &R2_GLOBALS._player, &scene->_robot, NULL);
		else
			scene->setAction(&scene->_sequenceManager1, scene, 1857, &R2_GLOBALS._player, &scene->_robot, NULL);

		R2_GLOBALS.setFlag(30);
		return true;
		break;
	case CURSOR_LOOK:
		if (R2_GLOBALS.getFlag(34))
			SceneItem::display(1850, 2, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		else
			SceneItem::display(1850, 1, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);

		return true;
		break;
	case R2_AIRBAG:
		if (R2_GLOBALS._player._characterIndex == R2_SEEKER) {
			if (R2_GLOBALS.getFlag(70)) {
				R2_GLOBALS._player.disableControl();
				scene->_sceneMode = 30;

				R2_GLOBALS._events.setCursor(CURSOR_WALK);
				scene->_stripManager.start(558, scene);

				return true;
			} else {
				return SceneActor::startAction(action, event);
			}
		} else if (R2_GLOBALS.getFlag(30)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1875;
			scene->_airbag.postInit();

			if (R2_GLOBALS.getFlag(32))
				scene->setAction(&scene->_sequenceManager1, scene, 1876,
					&R2_GLOBALS._player, &scene->_airbag, NULL);
			else
				scene->setAction(&scene->_sequenceManager1, scene, 1875,
					&R2_GLOBALS._player, &scene->_airbag, NULL);

			return true;
		} else if (R2_GLOBALS.getFlag(70)) {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 20;
			R2_GLOBALS._events.setCursor(CURSOR_WALK);
			scene->_stripManager.start(557, scene);
			R2_GLOBALS.setFlag(69);

			return true;
		} else {
			return SceneActor::startAction(action, event);
		}
		break;
	case R2_REBREATHER_TANK:
		if (R2_INVENTORY.getObjectScene(R2_AIRBAG) == 1850) {
			if (R2_GLOBALS.getFlag(30))
				return SceneActor::startAction(action, event);

			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1878;
			scene->setAction(&scene->_sequenceManager1, scene, 1878, &R2_GLOBALS._player,
				&scene->_robot, &scene->_airbag, NULL);
		}

		return true;
		break;
	default:
		return SceneActor::startAction(action, event);
		break;
	}
}

bool Scene1850::Door::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	if (R2_GLOBALS.getFlag(32)) {
		SceneItem::display(3240, 4, 0, 280, 1, 160, 9, 1, 2, 20, 7, 7, LIST_END);
		return true;
	}

	Scene1850 *scene = (Scene1850 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	if (scene->_sceneMode == 1851)
		R2_GLOBALS._player._effect = EFFECT_SHADED;

	if (_position.x >= 160)
		R2_GLOBALS.setFlag(29);
	else
		R2_GLOBALS.clearFlag(29);

	if ((R2_GLOBALS._player._characterIndex == R2_SEEKER) && (R2_GLOBALS.getFlag(30))) {
		if (_position.x >= 160)
			scene->_seqNumber = 3;
		else
			scene->_seqNumber = 2;

		scene->_sceneMode = 1860;

		if (R2_GLOBALS.getFlag(32)) {
			scene->setAction(&scene->_sequenceManager1, scene, 1860, &R2_GLOBALS._player, &scene->_robot, NULL);
		} else {
			scene->setAction(&scene->_sequenceManager1, scene, 1859, &R2_GLOBALS._player, &scene->_robot, NULL);
		}
	} else {
		scene->_sceneMode = 11;
		if (_position.x >= 160) {
			scene->setAction(&scene->_sequenceManager1, scene, 1866, &R2_GLOBALS._player, &scene->_rightDoor, NULL);
		} else {
			scene->setAction(&scene->_sequenceManager1, scene, 1865, &R2_GLOBALS._player, &scene->_leftDoor, NULL);
		}
	}

	return true;
}

bool Scene1850::DisplayScreen::startAction(CursorType action, Event &event) {
	if ((action != CURSOR_USE) || (_position.y != 120))
		return SceneHotspot::startAction(action, event);

	Scene1850 *scene = (Scene1850 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl();
	scene->_sceneMode = 1881;

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		scene->setAction(&scene->_sequenceManager1, scene, 1881, &R2_GLOBALS._player, NULL);
	} else {
		scene->setAction(&scene->_sequenceManager1, scene, 1880, &R2_GLOBALS._player, NULL);
	}

	return true;
}

/*------------------------------------------------------------------------*/

Scene1850::Scene1850() {
	_sceneMode = 0;
	_shadeCountdown = 0;
	_shadeDirection = 0;
	_shadeChanging = false;
	_seqNumber = 0;
}

void Scene1850::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_sceneMode);
	s.syncAsSint16LE(_shadeCountdown);
	s.syncAsSint16LE(_shadeDirection);
	s.syncAsSint16LE(_shadeChanging);
	s.syncAsSint16LE(_seqNumber);
	s.syncAsSint16LE(_playerDest.x);
	s.syncAsSint16LE(_playerDest.y);
}

void Scene1850::postInit(SceneObjectList *OwnerList) {
	loadScene(1850);

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] != 1850)
		R2_GLOBALS.clearFlag(31);

	_palette1.loadPalette(0);

	if (R2_GLOBALS.getFlag(31)) {
		_sceneMode = 1850;
		g_globals->_scenePalette.loadPalette(1850);
	} else {
		_sceneMode = 1851;
		g_globals->_scenePalette.loadPalette(1851);
	}

	SceneExt::postInit();

	if (R2_GLOBALS._sceneManager._previousScene == 3150)
		R2_GLOBALS._sound1.play(116);

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_shadeChanging = false;
	_seqNumber = 0;
	_playerDest = Common::Point(0, 0);

	R2_GLOBALS._player._characterScene[R2_QUINN] = 1850;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1850;

	_button.setDetails(Rect(101, 56, 111, 63), 1850, 19, -1, -1, 1, NULL);

	_leftDoor.postInit();
	_leftDoor.setup(1850, 3, 1);
	_leftDoor.setPosition(Common::Point(66, 102));
	_leftDoor.setDetails(1850, 22, -1, -1, 1, (SceneItem *) NULL);

	_rightDoor.postInit();
	_rightDoor.setup(1850, 2, 1);
	_rightDoor.setPosition(Common::Point(253, 102));
	_rightDoor.setDetails(1850, 22, -1, -1, 1, (SceneItem *) NULL);

	R2_GLOBALS._walkRegions.disableRegion(1);

	_robot.postInit();

	if (R2_GLOBALS.getFlag(34)) {
		R2_GLOBALS._walkRegions.disableRegion(2);
		_robot.setup(1851, 4, 3);
	} else if (R2_GLOBALS.getFlag(30)) {
		_robot.setup(1851, 2, 2);
	} else {
		R2_GLOBALS._walkRegions.disableRegion(5);
		if (R2_GLOBALS.getFlag(33)) {
			R2_GLOBALS._walkRegions.disableRegion(2);
			_robot.setup(1851, 1, 3);
		} else {
			_robot.setup(1851, 2, 1);
		}
	}

	_robot.setPosition(Common::Point(219, 130));
	_robot.fixPriority(114);
	_robot.setDetails(1850, -1, -1, -1, 1, (SceneItem *) NULL);

	R2_GLOBALS._player.postInit();

	_companion.postInit();
	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		_companion.setDetails(9002, 0, 4, 3, 1, (SceneItem *) NULL);
	} else {
		_companion.setDetails(9001, 0, 5, 3, 1, (SceneItem *) NULL);
	}

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1850) {
		R2_GLOBALS._player._effect = EFFECT_SHADED2;
		_companion._effect = EFFECT_SHADED2;
		if (R2_GLOBALS.getFlag(31)) {
			R2_GLOBALS._player._shade = 0;
			_companion._shade = 0;
		} else {
			R2_GLOBALS._player._shade = 6;
			_companion._shade = 6;
		}

		if (R2_INVENTORY.getObjectScene(R2_AIRBAG) == 1850) {
			_airbag.postInit();
			if (R2_GLOBALS.getFlag(34)) {
				_airbag.setup(1851, 4, 2);
				_airbag.fixPriority(114);
			} else {
				_airbag.setup(1851, 4, 1);
			}

			_airbag.setPosition(Common::Point(179, 113));

			if ((_robot._strip == 1) && (_robot._frame == 3)){
				_airbag.hide();
			}

			_airbag.setDetails(1850, 6, -1, -1, 1, (SceneItem *) NULL);
		}

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(32)) {
				R2_GLOBALS._player.setVisage(1511);
				_companion.setVisage(1508);

				_screen.postInit();
				_screen.setup(1853, 3, 1);
				_screen.setPosition(Common::Point(122, 113));
				_screen.fixPriority(114);
				_screen._effect = EFFECT_SHADED2;
				_screen.setDetails(1850, 28, -1, -1, 2, (SceneItem *) NULL);

				_helmet.postInit();
				_helmet.setup(1853, 3, 2);
				_helmet.setPosition(Common::Point(139, 111));
				_helmet.fixPriority(114);
				_helmet._effect = EFFECT_SHADED2;
				_helmet.setDetails(1850, 29, -1, -1, 2, (SceneItem *) NULL);

				if (R2_GLOBALS.getFlag(31)) {
					_screen._shade = 0;
					_helmet._shade = 0;
				} else {
					_screen._shade = 6;
					_helmet._shade = 6;
				}
			} else {
				R2_GLOBALS._player.setVisage(1500);
				_companion.setVisage(1505);
			}
		} else if (R2_GLOBALS.getFlag(32)) {
			// Not Quinn, flag 32
			R2_GLOBALS._player.setVisage(1508);
			_companion.setVisage(1511);

			_screen.postInit();
			_screen.setup(1853, 3, 1);
			_screen.setPosition(Common::Point(122, 113));
			_screen.fixPriority(114);
			_screen._effect = EFFECT_SHADED2;
			_screen.setDetails(1850, 30, -1, -1, 2, (SceneItem *) NULL);

			_helmet.postInit();
			_helmet.setup(1853, 3, 2);
			_helmet.setPosition(Common::Point(139, 111));
			_helmet.fixPriority(114);
			_helmet._effect = EFFECT_SHADED2;
			_helmet.setDetails(1850, 28, -1, -1, 1, (SceneItem *) NULL);

			if (R2_GLOBALS.getFlag(31)) {
				_screen._shade = 0;
				_helmet._shade = 0;
			} else {
				_screen._shade = 6;
				_helmet._shade = 6;
			}
		} else {
			// Not Quinn, nor flag 32
			R2_GLOBALS._player.setVisage(1505);
			_companion.setVisage(1500);
		}

		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setStrip(3);
		R2_GLOBALS._player.setPosition(Common::Point(80, 114));

		_companion.animate(ANIM_MODE_1, NULL);
		_companion.setObjectWrapper(new SceneObjectWrapper());
		_companion.setStrip(3);
		_companion.setPosition(Common::Point(180, 96));

		if (R2_GLOBALS.getFlag(30)) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				_companion.animate(ANIM_MODE_NONE, NULL);
				_companion.setObjectWrapper(NULL);
				if (R2_GLOBALS.getFlag(32)) {
					_companion.setup(1854, 1, 3);
				} else {
					_companion.setup(1854, 2, 3);
				}

				_companion.setPosition(Common::Point(164, 106));
			} else {
				_companion.animate(ANIM_MODE_NONE, NULL);
				_companion.setObjectWrapper(NULL);
				if (R2_GLOBALS.getFlag(32)) {
					R2_GLOBALS._player.setup(1854, 1, 3);
				} else {
					R2_GLOBALS._player.setup(1854, 2, 3);
				}

				R2_GLOBALS._player.setPosition(Common::Point(164, 106));
			}
		}

		R2_GLOBALS._player.enableControl();
	} else { // R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] != 1850
		R2_GLOBALS._player._effect = EFFECT_SHADED;
		_companion._effect = EFFECT_SHADED;
		R2_GLOBALS._player.disableControl();
		_sceneMode = 10;
		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (R2_GLOBALS.getFlag(29)) {
				setAction(&_sequenceManager1, this, 1863, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
			} else {
				setAction(&_sequenceManager1, this, 1861, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
			}
		} else {
			if (R2_GLOBALS.getFlag(29)) {
				setAction(&_sequenceManager1, this, 1864, &R2_GLOBALS._player, &_companion, &_rightDoor, NULL);
			} else {
				setAction(&_sequenceManager1, this, 1862, &R2_GLOBALS._player, &_companion, &_leftDoor, NULL);
			}
		}
	}

	if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
		_companion._moveDiff = Common::Point(5, 3);
	} else {
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
		_companion._moveDiff = Common::Point(3, 2);
	}

	_displayScreen.postInit();
	_displayScreen.setup(1850, 1, 1);

	if (R2_GLOBALS.getFlag(62)) {
		_displayScreen.setPosition(Common::Point(159, 120));
	} else {
		_displayScreen.setPosition(Common::Point(159, 184));
	}

	_displayScreen.fixPriority(113);

	if (R2_GLOBALS.getFlag(34)) {
		_displayScreen.setDetails(1850, 25, -1, -1, 4, &_robot);
	} else {
		_displayScreen.setDetails(1850, 25, -1, -1, 2, (SceneItem *) NULL);
	}

	if (!R2_GLOBALS.getFlag(62)) {
		_displayScreen.hide();
	}

	_background.setDetails(Rect(0, 0, 320, 200), 1850, 16, -1, -1, 1, NULL);

	R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1850;
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1850;
}

void Scene1850::remove() {
	g_globals->_scenePalette.loadPalette(0);

	R2_GLOBALS._scenePalette._palette[765] = 255;
	R2_GLOBALS._scenePalette._palette[766] = 255;
	R2_GLOBALS._scenePalette._palette[767] = 255;

	SceneExt::remove();
}

void Scene1850::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player._effect = EFFECT_SHADED2;
		R2_GLOBALS._player._shade = 6;

		_companion._effect = EFFECT_SHADED2;
		_companion._shade = 6;

		R2_GLOBALS._walkRegions.disableRegion(5);

		if (R2_GLOBALS.getFlag(68)) {
			R2_GLOBALS._player.enableControl();
		} else {
			R2_GLOBALS.setFlag(68);
			_sceneMode = 20;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_stripManager.start(554, this);
		}
		break;
	case 11:
		R2_GLOBALS.clearFlag(30);
		R2_GLOBALS._sceneManager.changeScene(1800);
		break;
	case 15:
		_sceneMode = 16;
		break;
	case 16:
		_sceneMode = 1870;
		setAction(&_sequenceManager1, this, 1870, &R2_GLOBALS._player, &_companion,
			&_screen, &_helmet, NULL);
		break;
	case 20:
		R2_GLOBALS._player.enableControl(CURSOR_TALK);
		break;
	case 21:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1877;
		setAction(&_sequenceManager1, this, 1877, &R2_GLOBALS._player, &_companion,
			&_robot, NULL);
		break;
	case 30:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1882;
		setAction(&_sequenceManager1, this, 1882, &R2_GLOBALS._player, NULL);
		break;
	case 1852:
	// No break on purpose:
	case 1853:
		if (_sceneMode == 1851) { // At this point, SceneMode can't be equal to 1851 => dead code
			R2_GLOBALS.setFlag(31);
			_palette1.loadPalette(1850);
			_sceneMode = 1850;
		} else {
			R2_GLOBALS.clearFlag(31);
			_palette1.loadPalette(1851);
			_sceneMode = 1851;
		}

		_shadeChanging = true;
		if (R2_GLOBALS.getFlag(30)) {
			_displayScreen.setAction(&_sequenceManager2, NULL, 1867, &_displayScreen, NULL);
		} else if (R2_GLOBALS.getFlag(34)) {
			if (R2_GLOBALS.getFlag(62)) {
				R2_GLOBALS.clearFlag(62);
				_displayScreen.setAction(&_sequenceManager2, this, 1851, &_displayScreen, NULL);
			} else {
				R2_GLOBALS.setFlag(62);
				_displayScreen.setAction(&_sequenceManager2, this, 1850, &_displayScreen, NULL);
			}
		} else if (R2_GLOBALS.getFlag(33)) {
				R2_GLOBALS.setFlag(62);
				R2_GLOBALS.setFlag(34);
				R2_GLOBALS._walkRegions.disableRegion(2);

				_airbag.postInit();
				_airbag.setDetails(1850, 6, -1, -1, 5, &_robot);

				_sceneMode = 1879;

				_displayScreen.setAction(&_sequenceManager2, this, 1879, &_robot,
					&_displayScreen, &_airbag, NULL);
		} else {
			_displayScreen.setAction(&_sequenceManager2, NULL, 1867, &_displayScreen, NULL);
		}

		if (R2_GLOBALS.getFlag(34))
			R2_GLOBALS._scenePalette.addFader(_palette1._palette, 256, 5, NULL);
		else
			R2_GLOBALS._scenePalette.addFader(_palette1._palette, 256, 5, this);

		if (_sceneMode == 1851)
			_shadeDirection = -20;
		else
			_shadeDirection = 20;

		_shadeCountdown = 20;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			if (_sceneMode == 1879)
				_sceneMode = 1854;

			if (R2_GLOBALS.getFlag(32)) {
				setAction(&_sequenceManager1, NULL, 1873, &R2_GLOBALS._player, NULL);
			} else {
				setAction(&_sequenceManager1, NULL, 1854, &R2_GLOBALS._player, NULL);
			}
		} else {
			if (_sceneMode == 1879)
				_sceneMode = 1855;

			if (R2_GLOBALS.getFlag(32)) {
				setAction(&_sequenceManager1, NULL, 1874, &R2_GLOBALS._player, NULL);
			} else {
				setAction(&_sequenceManager1, NULL, 1855, &R2_GLOBALS._player, NULL);
			}
		}
		break;
	case 1857:
		if (R2_GLOBALS.getFlag(69)) {
			R2_GLOBALS._player.enableControl();
			R2_GLOBALS._player._canWalk = false;
		} else {
			_sceneMode = 1858;
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			_stripManager.start(555, this);
			R2_GLOBALS.setFlag(69);
		}
		break;
	case 1858:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1859;
		setAction(&_sequenceManager1, this, 1859, &R2_GLOBALS._player, &_robot, NULL);
		R2_GLOBALS.clearFlag(30);
		break;
	case 1859:
		R2_GLOBALS.setFlag(70);
		_sceneMode = 20;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(575, this);
		break;
	case 1860:
		if (_playerDest.x != 0) {
			R2_GLOBALS._player.enableControl();

			PlayerMover *mover = new PlayerMover();
			R2_GLOBALS._player.addMover(mover, &_playerDest, this);

			_playerDest = Common::Point(0, 0);
		}

		switch (_seqNumber) {
		case 1:
			_sceneMode = 1853;
			if (R2_GLOBALS.getFlag(32)) {
				setAction(&_sequenceManager1, this, 1872, &R2_GLOBALS._player, NULL);
			} else {
				setAction(&_sequenceManager1, this, 1853, &R2_GLOBALS._player, NULL);
			}
			break;
		case 2:
			_sceneMode = 11;
			setAction(&_sequenceManager1, this, 1865, &R2_GLOBALS._player, &_leftDoor, NULL);
			break;
		case 3:
			_sceneMode = 11;
			setAction(&_sequenceManager1, this, 1866, &R2_GLOBALS._player, &_rightDoor, NULL);
			break;
		default:
			break;
		}

		_seqNumber = 0;
		break;
	case 1870:
		R2_GLOBALS._walkRegions.disableRegion(5);
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1);
		R2_GLOBALS.setFlag(32);
		R2_GLOBALS._player.enableControl(CURSOR_WALK);
		break;
	case 1875:
		R2_INVENTORY.setObjectScene(R2_AIRBAG, 1850);
		_sceneMode = 21;
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_stripManager.start(561, this);
		break;
	case 1877:
		_screen.postInit();
		_screen._effect = EFFECT_SHADED2;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_screen.setDetails(1850, 28, -1, -1, 2, (SceneItem *)NULL);
		} else {
			_screen.setDetails(1850, 30, -1, -1, 2, (SceneItem *)NULL);
		}

		_helmet.postInit();
		_helmet._effect = EFFECT_SHADED2;

		if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
			_helmet.setDetails(1850, 29, -1, -1, 2, (SceneItem *)NULL);
		} else {
			_helmet.setDetails(1850, 28, -1, -1, 2, (SceneItem *)NULL);
		}

		if (R2_GLOBALS.getFlag(31)) {
			_screen._shade = 0;
			_helmet._shade = 0;
		} else {
			_screen._shade = 6;
			_helmet._shade = 6;
		}

		R2_GLOBALS.clearFlag(30);
		_sceneMode = 15;
		setAction(&_sequenceManager1, this, 1869, &R2_GLOBALS._player, &_screen, NULL);
		_companion.setAction(&_sequenceManager2, this, 1868, &_companion, &_helmet, NULL);
		break;
	case 1878:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 1850);
		R2_GLOBALS.setFlag(33);
		R2_GLOBALS._walkRegions.disableRegion(2);
		R2_GLOBALS._player.enableControl();
		break;
	case 1879:
		R2_GLOBALS._player.enableControl();
		break;
	case 1881:
		R2_GLOBALS._sceneManager.changeScene(1875);
		break;
	case 1882:
		R2_INVENTORY.setObjectScene(R2_AIRBAG, 1);
		R2_GLOBALS._player.enableControl();
		break;
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

void Scene1850::process(Event &event) {
	if ( (event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_WALK)
			&& (R2_GLOBALS._player._characterIndex == R2_SEEKER) && (R2_GLOBALS.getFlag(30))) {
		_playerDest = event.mousePos;
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1860;
		if (R2_GLOBALS.getFlag(32)) {
			setAction(&_sequenceManager1, this, 1860, &R2_GLOBALS._player, &_robot, NULL);
		} else {
			setAction(&_sequenceManager1, this, 1859, &R2_GLOBALS._player, &_robot, NULL);
		}
		R2_GLOBALS.clearFlag(30);
		event.handled = true;
	}

	Scene::process(event);
}

void Scene1850::dispatch() {
	if (_shadeChanging) {
		_shadeCountdown--;
		if (_shadeCountdown == 0)
			_shadeChanging = false;

		if (_shadeDirection >= 0) {
			R2_GLOBALS._player._shade = (_shadeCountdown * 6) / _shadeDirection;
		} else {
			R2_GLOBALS._player._shade = ((_shadeCountdown * 6) / _shadeDirection) + 6;
		}
		R2_GLOBALS._player._flags |= OBJFLAG_PANES;

		_companion._shade = R2_GLOBALS._player._shade;
		_companion._flags |= OBJFLAG_PANES;

		_screen._shade = R2_GLOBALS._player._shade;
		_screen._flags |= OBJFLAG_PANES;

		_helmet._shade = R2_GLOBALS._player._shade;
		_helmet._flags |= OBJFLAG_PANES;
	}

	if (R2_GLOBALS.getFlag(32)) {
		_screen.setPosition(Common::Point(_displayScreen._position.x - 37, _displayScreen._position.y - 71));
		_helmet.setPosition(Common::Point(_displayScreen._position.x - 20, _displayScreen._position.y - 73));
	}

	if (R2_INVENTORY.getObjectScene(R2_AIRBAG) == 1850) {
		_airbag.setPosition(Common::Point(_displayScreen._position.x + 20, _displayScreen._position.y - 71));
	}

	Scene::dispatch();
}

/*--------------------------------------------------------------------------
 * Scene 1875 - Rim Lift Computer
 *
 *--------------------------------------------------------------------------*/

Scene1875::Button::Button() {
	_buttonId = 0;
	_buttonDown = false;
}

void Scene1875::Button::synchronize(Serializer &s) {
	SceneActor::synchronize(s);

	s.syncAsSint16LE(_buttonId);
	s.syncAsSint16LE(_buttonDown);
}

void Scene1875::Button::doButtonPress() {
	Scene1875 *scene = (Scene1875 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._sound1.play(227);

	int newFrameNumber;
	switch (_buttonId) {
	case 3:
		if ((scene->_map._frame == 1) && (scene->_button1._strip == 2)) {
			R2_GLOBALS._player.disableControl();
			R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
			scene->_sceneMode = 10;
			scene->_stripManager.start(576, scene);
		} else {
			R2_GLOBALS._player.disableControl();
			scene->_sceneMode = 1890;
			scene->_rimPosition.postInit();
			scene->setAction(&scene->_sequenceManager, scene, 1890, &scene->_rimPosition, NULL);
		}
		break;
	case 4:
		newFrameNumber = scene->_map._frame + 1;
		if (newFrameNumber > 6)
			newFrameNumber = 1;
		scene->_map.setFrame(newFrameNumber);
		break;
	case 5:
		newFrameNumber = scene->_map._frame - 1;
		if (newFrameNumber < 1)
			newFrameNumber = 6;
		scene->_map.setFrame(newFrameNumber);
		break;
	default:
		break;
	}
}

void Scene1875::Button::initButton(int buttonId) {
	postInit();
	_buttonId = buttonId;
	_buttonDown = false;
	setVisage(1855);

	if (_buttonId == 1)
		setStrip(2);
	else
		setStrip(1);

	setFrame(_buttonId);
	switch (_buttonId) {
	case 1:
		setPosition(Common::Point(20, 144));
		break;
	case 2:
		setPosition(Common::Point(82, 144));
		break;
	case 3:
		setPosition(Common::Point(136, 144));
		break;
	case 4:
		setPosition(Common::Point(237, 144));
		break;
	case 5:
		setPosition(Common::Point(299, 144));
		break;
	default:
		break;
	}

	setDetails(1875, 6, 1, -1, 2, (SceneItem *) NULL);
}

void Scene1875::Button::process(Event &event) {
	if (!R2_GLOBALS._player._uiEnabled || event.handled)
		return;

	Scene1875 *scene = (Scene1875 *)R2_GLOBALS._sceneManager._scene;

	if ((event.eventType == EVENT_BUTTON_DOWN) && (R2_GLOBALS._events.getCursor() == CURSOR_USE)
			&& (_bounds.contains(event.mousePos)) && !_buttonDown) {
		setStrip(2);
		switch (_buttonId) {
		case 1:
			R2_GLOBALS._sound2.play(227);
			scene->_button2.setStrip(1);
			break;
		case 2:
			R2_GLOBALS._sound2.play(227);
			scene->_button1.setStrip(1);
			break;
		default:
			break;
		}
		_buttonDown = true;
		event.handled = true;
	}

	if ((event.eventType == EVENT_BUTTON_UP) && _buttonDown) {
		if ((_buttonId == 3) || (_buttonId == 4) || (_buttonId == 5)) {
			setStrip(1);
			doButtonPress();
		}
		_buttonDown = false;
		event.handled = true;
	}
}

void Scene1875::postInit(SceneObjectList *OwnerList) {
	loadScene(1875);
	SceneExt::postInit();

	R2_GLOBALS._player._characterScene[R2_QUINN] = 1875;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1875;

	_stripManager.addSpeaker(&_quinnSpeaker);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_button1.initButton(1);
	_button2.initButton(2);
	_button3.initButton(3);
	_button4.initButton(4);
	_button5.initButton(5);

	_map.postInit();
	_map.setup(1855, 4, 1);
	_map.setPosition(Common::Point(160, 116));

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.hide();

	if (R2_GLOBALS._sceneManager._previousScene == 1625) {
		R2_GLOBALS._sound1.play(122);
		R2_GLOBALS._player.disableControl();
		_sceneMode = 11;
		_rimPosition.postInit();
		setAction(&_sequenceManager, this, 1892, &_rimPosition, NULL);
	} else {
		if (R2_GLOBALS._sceneManager._previousScene == 3150) {
			R2_GLOBALS._sound1.play(116);
		}

		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
	}

	_screen.setDetails(Rect(43, 14, 275, 122), 1875, 9, 1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 1875, 3, -1, -1, 1, NULL);

	R2_GLOBALS._player._characterScene[R2_QUINN] = 1875;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1875;
	R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1875;
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1875;
}

void Scene1875::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._player.disableControl();
		_sceneMode = 1891;
		_rimPosition.postInit();
		setAction(&_sequenceManager, this, 1891, &_rimPosition, NULL);
		break;
	case 11:
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);
		_sceneMode = 1892;
		_stripManager.start(577, this);
		break;
	case 1890:
		_rimPosition.remove();
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	case 1891:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneManager.changeScene(1625);
		break;
	case 1892:
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._sceneManager.changeScene(1900);
		break;
	default:
		R2_GLOBALS._player.enableControl();
		R2_GLOBALS._player._canWalk = false;
		break;
	}
}

void Scene1875::process(Event &event) {
	Scene::process(event);

	_button1.process(event);
	_button2.process(event);
	_button3.process(event);
	_button4.process(event);
	_button5.process(event);
}

/*--------------------------------------------------------------------------
 * Scene 1900 - Spill Mountains Elevator Exit
 *
 *--------------------------------------------------------------------------*/

bool Scene1900::LiftDoor::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneActor::startAction(action, event);

	if ((_frame != 1) || (R2_GLOBALS._player._characterIndex != R2_SEEKER)) {
		if (R2_GLOBALS._player._characterIndex == R2_QUINN)
			return SceneActor::startAction(action, event);
		else
			return true;
	}

	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.enableControl(CURSOR_USE);

	if (_position.x >= 160) {
		scene->_sceneMode = 1905;
		scene->setAction(&scene->_sequenceManager1, scene, 1905, &R2_GLOBALS._player, &scene->_rightDoor, NULL);
	} else {
		R2_GLOBALS.setFlag(29);
		scene->_sceneMode = 1904;
		scene->setAction(&scene->_sequenceManager1, scene, 1904, &R2_GLOBALS._player, &scene->_leftDoor, NULL);
	}

	return true;
}

void Scene1900::WestExit::changeScene() {
	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 10;

	Common::Point pt(-10, 135);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1900::EastExit::changeScene() {
	Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 11;

	Common::Point pt(330, 135);
	NpcMover *mover = new NpcMover();
	R2_GLOBALS._player.addMover(mover, &pt, scene);
}

void Scene1900::postInit(SceneObjectList *OwnerList) {
	loadScene(1900);
	SceneExt::postInit();

	// Debug message, skipped

	if (R2_GLOBALS._sceneManager._previousScene == -1) {
		R2_GLOBALS._sceneManager._previousScene = 1925;
		R2_GLOBALS._player._characterIndex = R2_SEEKER;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1925;
	}

	if (R2_GLOBALS._sceneManager._previousScene != 1875)
		R2_GLOBALS._sound1.play(200);

	_stripManager.setColors(60, 255);
	_stripManager.setFontNumber(3);
	_stripManager.addSpeaker(&_seekerSpeaker);

	_westExit.setDetails(Rect(0, 105, 14, 145), EXITCURSOR_W, 2000);
	_westExit.setDest(Common::Point(14, 135));

	_eastExit.setDetails(Rect(305, 105, 320, 145), EXITCURSOR_E, 2000);
	_eastExit.setDest(Common::Point(315, 135));

	R2_GLOBALS._player.postInit();
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		R2_GLOBALS._player.setup(2008, 3, 1);
	else
		R2_GLOBALS._player.setup(20, 3, 1);
	R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		R2_GLOBALS._player._moveDiff = Common::Point(3, 2);
	else
		R2_GLOBALS._player._moveDiff = Common::Point(5, 3);
	R2_GLOBALS._player.disableControl();

	if (R2_GLOBALS._sceneManager._previousScene != 1925)
		R2_GLOBALS.clearFlag(29);

	_leftDoor.postInit();
	_leftDoor.setup(1901, 1, 1);
	_leftDoor.setPosition(Common::Point(95, 109));
	_leftDoor.fixPriority(100);

	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_leftDoor.setDetails(1900, 0, 1, 2, 1, (SceneItem *) NULL);
	else
		_leftDoor.setDetails(1900, 0, 1, -1, 1, (SceneItem *) NULL);

	_rightDoor.postInit();
	_rightDoor.setup(1901, 2, 1);
	_rightDoor.setPosition(Common::Point(225, 109));
	_rightDoor.fixPriority(100);
	if (R2_GLOBALS._player._characterIndex == R2_QUINN)
		_rightDoor.setDetails(1900, 0, 1, 2, 1, (SceneItem *) NULL);
	else
		_rightDoor.setDetails(1900, 0, 1, -1, 1, (SceneItem *) NULL);

	if (R2_GLOBALS._sceneManager._previousScene != 1875) {
		_leftDoorFrame.postInit();
		_leftDoorFrame.setup(1945, 6, 1);
		_leftDoorFrame.setPosition(Common::Point(96, 109));
		_leftDoorFrame.fixPriority(80);

		_rightDoorFrame.postInit();
		_rightDoorFrame.setup(1945, 6, 2);
		_rightDoorFrame.setPosition(Common::Point(223, 109));
		_rightDoorFrame.fixPriority(80);
	}

	if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1875) {
		R2_GLOBALS._player._characterIndex = R2_QUINN;
		_companion.postInit();
		_sceneMode = 20;
		R2_GLOBALS._player.setAction(&_sequenceManager1, NULL, 1901, &R2_GLOBALS._player, &_leftDoor, NULL);
		_companion.setAction(&_sequenceManager2, this, 1900, &_companion, &_rightDoor, NULL);
	} else if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 1925) {
		if (R2_GLOBALS.getFlag(29)) {
			R2_GLOBALS.clearFlag(29);
			_leftDoor.hide();

			R2_GLOBALS._player.setStrip(6);
			R2_GLOBALS._player.setPosition(Common::Point(90, 106));
			_sceneMode = 1906;
			setAction(&_sequenceManager1, this, 1906, &R2_GLOBALS._player, &_leftDoor, NULL);
		} else {
			_rightDoor.hide();
			R2_GLOBALS._player.setStrip(5);
			R2_GLOBALS._player.setPosition(Common::Point(230, 106));
			_sceneMode = 1907;
			setAction(&_sequenceManager1, this, 1907, &R2_GLOBALS._player, &_rightDoor, NULL);
		}

		if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
			_companion.postInit();
			_companion.setPosition(Common::Point(30, 110));
			R2_GLOBALS._walkRegions.disableRegion(1);
			_companion.setup(2008, 3, 1);
			_companion.setDetails(9001, 0, -1, -1, 1, (SceneItem *) NULL);
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1900;
	} else {
		if (R2_GLOBALS._player._characterScene[R2_QUINN] == R2_GLOBALS._player._characterScene[R2_SEEKER]) {
			_companion.postInit();
			_companion.setPosition(Common::Point(30, 110));
			R2_GLOBALS._walkRegions.disableRegion(1);
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				_companion.setup(20, 3, 1);
				_companion.setDetails(9002, 1, -1, -1, 1, (SceneItem *) NULL);
			} else {
				_companion.setup(2008, 3, 1);
				_companion.setDetails(9001, 0, -1, -1, 1, (SceneItem *) NULL);
			}
		}

		if (R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] == 2000) {
			if (R2_GLOBALS._player._characterIndex == R2_QUINN) {
				if (R2_GLOBALS._spillLocation[R2_QUINN] == 5) {
					_sceneMode = 1902;
					setAction(&_sequenceManager1, this, 1902, &R2_GLOBALS._player, NULL);
				} else {
					_sceneMode = 1903;
					setAction(&_sequenceManager1, this, 1903, &R2_GLOBALS._player, NULL);
				}
			} else {
				if (R2_GLOBALS._spillLocation[R2_SEEKER] == 5) {
					_sceneMode = 1908;
					setAction(&_sequenceManager1, this, 1908, &R2_GLOBALS._player, NULL);
				} else {
					_sceneMode = 1909;
					setAction(&_sequenceManager1, this, 1909, &R2_GLOBALS._player, NULL);
				}
			}
		} else {
			R2_GLOBALS._player.setPosition(Common::Point(160, 135));
			R2_GLOBALS._player.enableControl();
		}
		R2_GLOBALS._player._oldCharacterScene[R2_GLOBALS._player._characterIndex] = 1900;
	}

	_elevator.setDetails(Rect(77, 2, 240, 103), 1900, 6, -1, -1, 1, NULL);
	_background.setDetails(Rect(0, 0, 320, 200), 1900, 3, -1, -1, 1, NULL);
}

void Scene1900::remove() {
	R2_GLOBALS._sound1.fadeOut(NULL);
	SceneExt::remove();
}

void Scene1900::signal() {
	switch (_sceneMode) {
	case 10:
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 5;
		R2_GLOBALS._sceneManager.changeScene(2000);
		break;
	case 11:
		R2_GLOBALS._spillLocation[R2_GLOBALS._player._characterIndex] = 6;
		R2_GLOBALS._sceneManager.changeScene(2000);
		break;
	case 20:
		++_sceneMode;
		R2_GLOBALS._events.setCursor(CURSOR_WALK);
		_stripManager.start(1300, this);
		break;
	case 21:
		++_sceneMode;
		R2_GLOBALS._player.disableControl();
		if (R2_GLOBALS._player._action)
			R2_GLOBALS._player._action->_endHandler = this;
		else
			signal();
		break;
	case 22:
		_sceneMode = 1910;
		_companion.setAction(&_sequenceManager2, this, 1910, &_companion, NULL);
		break;
	case 1904:
		R2_GLOBALS._scene1925CurrLevel = -3;
	// No break on purpose
	case 1905:
		R2_GLOBALS._player.disableControl(CURSOR_WALK);
		R2_GLOBALS._sceneManager.changeScene(1925);
		break;
	case 1910:
		R2_INVENTORY.setObjectScene(R2_REBREATHER_TANK, 2535);
		R2_GLOBALS._player.disableControl(CURSOR_WALK);
		R2_GLOBALS._player._oldCharacterScene[R2_QUINN] = 1900;
		R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1900;
		R2_GLOBALS._sceneManager.changeScene(2450);
		break;
	case 1906:
		R2_GLOBALS._scene1925CurrLevel = -3;
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl();
		break;
	}
}

/*--------------------------------------------------------------------------
 * Scene 1925 - Spill Mountains Elevator Shaft
 *
 *--------------------------------------------------------------------------*/

Scene1925::Scene1925() {
	_newSceneMode = 0;
	for (int i = 0; i < 5; i++)
		_levelResNum[i] = 0;
}

void Scene1925::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_newSceneMode);
	for (int i = 0; i < 5; i++)
		s.syncAsSint16LE(_levelResNum[i]);
}

bool Scene1925::Button::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100))
		return SceneHotspot::startAction(action, event);

	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20))
		scene->_sceneMode = 1928;
	else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200))
		scene->_sceneMode = 1929;
	else
		scene->_sceneMode = 1930;

	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player,
		&scene->_door, NULL);
	return true;
}

bool Scene1925::Ladder::startAction(CursorType action, Event &event) {
	if ((!R2_GLOBALS.getFlag(29)) || (action != CURSOR_USE))
		return SceneHotspot::startAction(action, event);

	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_westExit._enabled = false;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode,
			&R2_GLOBALS._player, &scene->_door, NULL);
		return true;
	}

	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20) && (event.mousePos.y >= 30)) {
		scene->_sceneMode = 1931;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200) && (event.mousePos.y < 140)) {
		scene->_sceneMode = 1932;
	} else if ( (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		         || ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))
				 ) && (event.mousePos.y >= 100)) {
		scene->_sceneMode = 1926;
	} else if ( (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		         || ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))
				 ) && (event.mousePos.y < 60)) {
		scene->_sceneMode = 1927;
	} else {
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);

	return true;
}

void Scene1925::ExitUp::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_westExit._enabled = false;
		scene->_newSceneMode = 1927;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode,
			&R2_GLOBALS._player, &scene->_door, NULL);
		return;
	}

	if (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		|| ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))) {
		scene->_sceneMode = 1927;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 200)) {
		scene->_sceneMode = 1932;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
	else {
		scene->_sceneMode = 1932;
		scene->signal();
	}
}

void Scene1925::ExitDown::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 110) && (R2_GLOBALS._player._position.y == 100)) {
		scene->_westExit._enabled = false;
		scene->_newSceneMode = 1926;
		scene->_sceneMode = 1925;
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode,
			&R2_GLOBALS._player, &scene->_door, NULL);
		return;
	}

	if (   ((R2_GLOBALS._player._position.x == 112) && (R2_GLOBALS._player._position.y == 101))
		|| ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 110))) {
		scene->_sceneMode = 1926;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 20)) {
		scene->_sceneMode = 1931;
	}

	if (scene->_sceneMode == 0) {
		scene->_sceneMode = 1931;
		scene->signal();
	} else
		scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::WestExit::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 1921;
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::EastExit::changeScene() {
	Scene1925 *scene = (Scene1925 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_WALK);
	scene->_sceneMode = 1920;
	scene->setAction(&scene->_sequenceManager, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1925::changeLevel(bool upFlag) {
	if (R2_GLOBALS._scene1925CurrLevel < 0)
		R2_GLOBALS._scene1925CurrLevel = 3;

	if (upFlag) {
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 200));
		++R2_GLOBALS._scene1925CurrLevel;
	} else {
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 20));
		R2_GLOBALS._scene1925CurrLevel--;
	}

	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -1:
		R2_GLOBALS._sceneManager.changeScene(1945);
		return;
		break;
	case 3:
		loadScene(_levelResNum[4]);
		_button.setDetails(Rect(133, 68, 140, 77), 1925, 3, -1, 5, 2, NULL);
		_door.setDetails(1925, 0, 1, 2, 2, (SceneItem *) NULL);
		_door.show();
		break;
	case 512:
		R2_GLOBALS._scene1925CurrLevel = 508;
	// No break on purpose
	default:
		loadScene(_levelResNum[(R2_GLOBALS._scene1925CurrLevel % 4)]);
		R2_GLOBALS._sceneItems.remove(&_button);
		R2_GLOBALS._sceneItems.remove(&_door);
		_door.hide();
		break;
	}

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
}

void Scene1925::postInit(SceneObjectList *OwnerList) {
	_levelResNum[0] = 1930;
	_levelResNum[1] = 1935;
	_levelResNum[2] = 1940;
	_levelResNum[3] = 1935;
	_levelResNum[4] = 1925;

	if (R2_GLOBALS.getFlag(29)) {
		if ((R2_GLOBALS._scene1925CurrLevel == -3) || (R2_GLOBALS._scene1925CurrLevel == 3))
			loadScene(_levelResNum[4]);
		else
			loadScene(_levelResNum[R2_GLOBALS._scene1925CurrLevel % 4]);
	} else {
		R2_GLOBALS._scene1925CurrLevel = -2;
		loadScene(1920);
	}
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(220);
	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.disableControl();
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1925;
	R2_GLOBALS._player._characterIndex = R2_SEEKER;

	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -2:
		_eastExit.setDetails(Rect(203, 44, 247, 111), EXITCURSOR_E, 1925);
		_ladder.setDetails(Rect(31, 3, 45, 167), 1925, 6, -1, 8, 1, NULL);
		break;
	case 3:
		_door.setDetails(1925, 0, 1, 2, 1, (SceneItem *) NULL);
		_button.setDetails(Rect(133, 68, 140, 77), 1925, 3, -1, 5, 1, NULL);
	// No break on purpose
	case -3:
		_westExit.setDetails(Rect(83, 38, 128, 101), EXITCURSOR_W, 1925);
	// No break on purpose
	default:
		_exitUp.setDetails(Rect(128, 0, 186, 10), EXITCURSOR_N, 1925);
		_exitDown.setDetails(Rect(128, 160, 190, 167), EXITCURSOR_S, 1925);
		_ladder.setDetails(Rect(141, 11, 167, 159),	1925, 6, -1, -1, 1, NULL);
		break;
	}

	_door.postInit();
	_door.setup(1925, 5, 1);
	_door.setPosition(Common::Point(128, 35));
	_door.hide();

	if (R2_GLOBALS._scene1925CurrLevel == 3)
		_door.show();

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	switch (R2_GLOBALS._scene1925CurrLevel) {
	case -2:
		R2_GLOBALS._player.setup(20, 6, 1);
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setPosition(Common::Point(224, 109));
		break;
	case -3:
		_door.hide();
		R2_GLOBALS._player.setup(20, 5, 1);
		R2_GLOBALS._player.setPosition(Common::Point(110, 100));
		break;
	case 0:
		R2_GLOBALS._player.setup(1925, 1, 1);
		R2_GLOBALS._player.setPosition(Common::Point(154, 200));
		break;
	default:
		R2_GLOBALS._player.setup(1925, 1, 3);
		R2_GLOBALS._player.setPosition(Common::Point(154, 110));
		break;
	}

	R2_GLOBALS._player._canWalk = false;
	_newSceneMode = 0;
	R2_GLOBALS._sceneManager._previousScene = 1925;
	_background.setDetails(Rect(27, 0, 292, 200), 1925, 9, -1, -1, 1, NULL);
}

void Scene1925::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	R2_GLOBALS._player._oldCharacterScene[R2_SEEKER] = 1925;
	SceneExt::remove();
}

void Scene1925::signal() {
	switch (_sceneMode) {
	case 1920:
	// No break on purpose
	case 1921:
	// No break on purpose
	case 1928:
	// No break on purpose
	case 1929:
	// No break on purpose
	case 1930:
		R2_GLOBALS._scene1925CurrLevel = -3;
		R2_GLOBALS._sceneManager.changeScene(1900);
		break;
	case 1926:
	// No break on purpose
	case 1931:
		// Change level, down
		changeLevel(false);
		break;
	case 1927:
	// No break on purpose
	case 1932:
		// Change level, up
		changeLevel(true);
		break;
	case 1925:
		_westExit._enabled = false;
		if (_newSceneMode != 0) {
			_sceneMode = _newSceneMode;
			_newSceneMode = 0;
			setAction(&_sequenceManager, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
	// No break on purpose
	default:
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		break;
	}

	R2_GLOBALS._player._canWalk = false;
}

/*--------------------------------------------------------------------------
 * Scene 1945 - Spill Mountains Shaft Bottom
 *
 *--------------------------------------------------------------------------*/

Scene1945::Scene1945() {
	_nextSceneMode1 = 0;
	_nextSceneMode2 = 0;
	_lampUsed = CURSOR_NONE;
}

void Scene1945::synchronize(Serializer &s) {
	SceneExt::synchronize(s);

	s.syncAsSint16LE(_nextSceneMode1);
	s.syncAsSint16LE(_nextSceneMode2);
	s.syncAsSint16LE(_lampUsed);
}

bool Scene1945::Ice::startAction(CursorType action, Event &event) {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	switch (action) {
	case R2_GUNPOWDER:
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142))
			scene->_sceneMode = 1942;
		else {
			scene->_sceneMode = 1940;
			scene->_nextSceneMode1 = 1942;
		}
		// At this point the original check if _sceneMode != 0. Skipped.
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, &scene->_gunpowder, NULL);
		return true;
		break;
	case CURSOR_USE:
		R2_GLOBALS._player.disableControl(CURSOR_USE);
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50) && (event.mousePos.x >= 130)) {
			scene->_sceneMode = 1940;
			// At this point the original check if _sceneMode != 0. Skipped.
			scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
			return true;
		} else {
			R2_GLOBALS._player.enableControl(CURSOR_USE);
			R2_GLOBALS._player._canWalk = false;
			if (event.mousePos.x > 130)
				scene->_ice.setDetails(1945, 3, -1, -1, 3, (SceneItem *) NULL);
			else
				scene->_ice.setDetails(1945, 3, -1, 5, 3, (SceneItem *) NULL);
		}
	// No break on purpose
	default:
		return SceneHotspot::startAction(action, event);
		break;
	}
}

bool Scene1945::Ladder::startAction(CursorType action, Event &event) {
	if (action != CURSOR_USE)
		return SceneHotspot::startAction(action, event);

	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 221) && (R2_GLOBALS._player._position.y == 142)) {
		scene->_sceneMode = 1949;
		scene->_nextSceneMode1 = 1947;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1947;
	} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50)
			&& (event.mousePos.y >= 30)) {
		scene->_sceneMode = 1940;
	} else {
		R2_GLOBALS._player.enableControl(CURSOR_USE);
		R2_GLOBALS._player._canWalk = false;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);

	return true;
}

bool Scene1945::Gunpowder::startAction(CursorType action, Event &event) {
	if ((action == R2_ALCOHOL_LAMP_3) || (action == R2_ALCOHOL_LAMP_2)) {
		Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

		scene->_lampUsed = action;
		R2_GLOBALS._player.disableControl();
		scene->_sceneMode = 0;
		if ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) {
			scene->_sceneMode= 1947;
			scene->_nextSceneMode1 = 1943;
		} else if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50)) {
			scene->_sceneMode = 1940;
			scene->_nextSceneMode1 = 1943;
		} else {
			scene->_sceneMode = 1949;
			scene->_nextSceneMode1 = 1947;
			scene->_nextSceneMode2 = 1943;
		}
		// At this point the original check if _sceneMode != 0. Skipped.
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
		return true;
	} else {
		return SceneActor::startAction(action, event);
	}
}

void Scene1945::ExitUp::changeScene() {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 221) && (R2_GLOBALS._player._position.y == 142)) {
		scene->_sceneMode = 1949;
		scene->_nextSceneMode1 = 1947;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1947;
	}

	if (scene->_sceneMode == 0) {
		scene->_sceneMode = 1941;
		signal();
	} else {
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
	}
}

void Scene1945::CorridorExit::changeScene() {
	Scene1945 *scene = (Scene1945 *)R2_GLOBALS._sceneManager._scene;

	_moving = false;
	R2_GLOBALS._player.disableControl(CURSOR_USE);
	scene->_sceneMode = 0;

	if ((R2_GLOBALS._player._position.x == 154) && (R2_GLOBALS._player._position.y == 50)) {
		scene->_sceneMode = 1940;
		scene->_nextSceneMode1 = 1945;
	} else if (  ((R2_GLOBALS._player._position.x == 197) && (R2_GLOBALS._player._position.y == 158))
		      || ((R2_GLOBALS._player._position.x == 191) && (R2_GLOBALS._player._position.y == 142)) ) {
		scene->_sceneMode = 1945;
	}

	if (scene->_sceneMode != 0)
		scene->setAction(&scene->_sequenceManager1, scene, scene->_sceneMode, &R2_GLOBALS._player, NULL);
}

void Scene1945::postInit(SceneObjectList *OwnerList) {
	loadScene(1945);
	SceneExt::postInit();
	R2_GLOBALS._sound1.play(220);

	R2_GLOBALS._player.postInit();
	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
	R2_GLOBALS._player._characterScene[R2_SEEKER] = 1945;
	R2_GLOBALS._player._characterIndex = R2_SEEKER;

	_exitUp.setDetails(Rect(128, 0, 186, 10), EXITCURSOR_N, 1945);
	_corridorExit.setDetails(Rect(238, 144, 274, 167), EXITCURSOR_E, 1945);

	_ladder.setDetails(Rect(141, 3, 167, 126), 1945, 9, -1, -1, 1, NULL);

	if (!R2_GLOBALS.getFlag(43)) {
		_corridorExit._enabled = false;
		_gunpowder.postInit();
		_gunpowder.setup(1945, 4, 1);
		_gunpowder.setPosition(Common::Point(253, 169));
		_gunpowder.fixPriority(150);

		if (R2_GLOBALS.getFlag(42))
			_gunpowder.setDetails(1945, 15, -1, -1, 1, (SceneItem *) NULL);
		else
			_gunpowder.hide();

		_coveringIce.postInit();
		_coveringIce.setup(1945, 8, 1);
		_coveringIce.setPosition(Common::Point(253, 169));
		_coveringIce.fixPriority(130);

		_alcoholLamp.postInit();
		_alcoholLamp.setup(1945, 3, 1);
		_alcoholLamp.hide();
	} else {
		_corridorExit._enabled = true;
	}

	switch (R2_GLOBALS._sceneManager._previousScene) {
	case 1925:
		R2_GLOBALS._scene1925CurrLevel = 0;
		R2_GLOBALS.clearFlag(29);
		R2_GLOBALS._player.setup(1925, 1, 10);
		R2_GLOBALS._player.setPosition(Common::Point(154, 50));
		break;
	case 1950:
		_sceneMode = 1944;
		setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		R2_GLOBALS._player.disableControl(CURSOR_USE);
		break;
	default:
		R2_GLOBALS._player.animate(ANIM_MODE_1, NULL);
		R2_GLOBALS._player.setup(20, 5, 1);
		R2_GLOBALS._player.setPosition(Common::Point(191, 142));
		break;
	}

	R2_GLOBALS._player._canWalk = false;
	_nextSceneMode1 = 0;
	_nextSceneMode2 = 0;

	_ice.setDetails(11, 1945, 3, -1, 5);
	_hole.setDetails(Rect(238, 144, 274, 167), 1945, 0, -1, 2, 1, NULL);
	_ice2.setDetails(Rect(27, 3, 292, 167), 1945, 3, -1, -1, 1, NULL);
}

void Scene1945::remove() {
	R2_GLOBALS._sound1.fadeOut2(NULL);
	SceneExt::remove();
}

void Scene1945::signal() {
	switch (_sceneMode) {
	case 1940:
		if (_nextSceneMode1 == 1943) {
			_sceneMode = _nextSceneMode1;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_alcoholLamp, NULL);
		} else {
			_sceneMode = 1946;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
		break;
	case 1941:
		if (_nextSceneMode1 == 0) {
			R2_GLOBALS._scene1925CurrLevel = 0;
			R2_GLOBALS.setFlag(29);
			R2_GLOBALS._sceneManager.changeScene(1925);
		} else {
			_sceneMode = _nextSceneMode1;
			_nextSceneMode1 = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	case 1942:
		R2_INVENTORY.setObjectScene(R2_GUNPOWDER, 0);
		_gunpowder.setDetails(1945, 15, -1, -1, 2, (SceneItem *) NULL);
		R2_GLOBALS.setFlag(42);
		break;
	case 1943:
		R2_GLOBALS._sound1.fadeOut2(NULL);
		R2_INVENTORY.setObjectScene(_lampUsed, 0);
		_sceneMode = 1948;
		setAction(&_sequenceManager1, this, _sceneMode, &_gunpowder, &_alcoholLamp, &_coveringIce, NULL);
		R2_GLOBALS._player.setAction(&_sequenceManager2, NULL, 1941, &R2_GLOBALS._player, NULL);
		return;
	case 1944:
		break;
	case 1945:
		R2_GLOBALS._sceneManager.changeScene(1950);
		return;
	case 1946:
		if (_nextSceneMode1 == 1942) {
			_sceneMode = _nextSceneMode1;
			_nextSceneMode1 = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_gunpowder, NULL);
			return;
		}
		_sceneMode = 0;
		break;
	case 1947:
		if (_nextSceneMode1 == 1943) {
			_sceneMode = _nextSceneMode1;
			_nextSceneMode1 = 1948;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_alcoholLamp, NULL);
		} else {
			_sceneMode = 1941;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	case 1948:
		R2_GLOBALS._sound1.play(220);
		_corridorExit._enabled = true;
		R2_GLOBALS._sceneItems.remove(&_gunpowder);
		R2_GLOBALS.clearFlag(42);
		R2_GLOBALS.setFlag(43);
		_nextSceneMode1 = 1940;
	// No break on purpose
	case 1949:
		_sceneMode = _nextSceneMode1;
		if (_nextSceneMode2 == 1943) {
			_nextSceneMode1 = _nextSceneMode2;
			_nextSceneMode2 = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, &_alcoholLamp, NULL);
		} else {
			_nextSceneMode1 = 0;
			setAction(&_sequenceManager1, this, _sceneMode, &R2_GLOBALS._player, NULL);
		}
		return;
	default:
		break;
	}

	R2_GLOBALS._player.enableControl(CURSOR_USE);
	R2_GLOBALS._player._canWalk = false;
}

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
	// No break on purpose
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
	// No break on purpose
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
