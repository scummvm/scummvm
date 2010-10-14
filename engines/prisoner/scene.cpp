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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "prisoner/prisoner.h"
#include "prisoner/path.h"
#include "prisoner/resource.h"
#include "prisoner/screen.h"

namespace Prisoner {

void PrisonerEngine::setEnterSceneScript(int16 programIndex, int16 scriptIndex) {
	_enterSceneScriptProgramIndex = programIndex;
	_enterSceneScriptIndex = scriptIndex;
	if (scriptIndex == -1) {
		_inventoryBarEnabled = true;
		_inventoryBarFlag = false;
	}
}

void PrisonerEngine::setLeaveSceneScript(int16 programIndex, int16 scriptIndex) {
	_leaveSceneScriptProgramIndex = programIndex;
	_leaveSceneScriptIndex = scriptIndex;
	if (scriptIndex != -1)
		_inventoryBarEnabled = false;
}

bool PrisonerEngine::isEnterSceneScriptFinished() {
	bool finished = false;
	if (_enterSceneScriptIndex == -1) {
		if (!_inventoryBarEnabled) {
			_inventoryBarEnabled = true;
			_inventoryBarFlag = false;
		}
		finished = true;
	} else if (_scriptPrograms[_enterSceneScriptProgramIndex].scripts[_enterSceneScriptIndex].status == kScriptStatusPaused) {
		_enterSceneScriptIndex = -1;
		_inventoryBarEnabled = true;
		_inventoryBarFlag = false;
		setUserInput(true);
		finished = true;
	}
	return finished;
}

bool PrisonerEngine::isLeaveSceneScriptFinished() {
	bool finished = false;
	if (_leaveSceneScriptIndex == -1)
		finished = true;
	else if (_scriptPrograms[_leaveSceneScriptProgramIndex].scripts[_leaveSceneScriptIndex].status == kScriptStatusPaused) {
		_leaveSceneScriptIndex = -1;
		finished = true;
	}
	return finished;
}

void PrisonerEngine::startModuleScript(Common::String &pakName, int16 pakSlot) {
	clearScriptProgram(kModuleScriptProgram);
	loadScriptProgram(pakName, pakSlot, kModuleScriptProgram);
	startScript(kModuleScriptProgram, 0);
	runInitScript(kModuleScriptProgram);
}

void PrisonerEngine::stopModuleScript() {
	stopScriptProgram(kModuleScriptProgram);
	unloadScriptProgram(kModuleScriptProgram);
}

void PrisonerEngine::updateModuleScript() {
	runScripts(kModuleScriptProgram);
}

void PrisonerEngine::checkForSceneChange() {

	//debug("PrisonerEngine::checkForSceneChange() _newModuleIndex = %d; _newSceneIndex = %d", _newModuleIndex, _newSceneIndex);
	if (!isLeaveSceneScriptFinished() || _newSceneIndex == -1 || _newModuleIndex == -1)
		return;

	leaveScene();

	_menuMouseCursorActive = true;
	_menuMouseCursor = kCursorDisk;
	updateMouseCursor();
	// TODO: showMouseCursor(true);

	if (_newModuleIndex == _currModuleIndex) {
		enterScene(_currModuleIndex, _newSceneIndex);
		_newSceneIndex = -1;
		_newModuleIndex = -1;
	} else {
		_currSceneIndex = _newSceneIndex;
		_currModuleIndex = _newModuleIndex;
		_newSceneIndex = -1;
		_newModuleIndex = -1;
		enterScene(_currModuleIndex, -1);
	}

	_menuMouseCursorActive = false;
	_menuMouseCursor = -1;
	updateMouseCursor();

}

void PrisonerEngine::enterScene(int16 moduleIndex, int16 sceneIndex) {

	debug("PrisonerEngine::enterScene(%d, %d)", moduleIndex, sceneIndex);

	//if (moduleIndex == 2 && sceneIndex == 33) gDebugLevel = 8; else gDebugLevel = 0;

	Common::String modulePakName = Common::String::printf("SM%02d", moduleIndex);

	_lockUserInputRefCounter = 0;
	_cameraFollowsActorIndex = -1;
	_updateDirtyRectsFlag = true;
	_muxClearScreenBefore = true;
	_muxClearScreenAfter = true;
	_backgroundNoScrollFlag1 = false;
	_moduleScriptCalled = false;
	_clearBackgroundFlag = false;
	resetBackgroundValues();
	// TODO: resetBackgroundObjectsResourceCacheSlot();
	clearPaletteTasks();
	clearActors();
	_pathSystem->clearPathWalker();
	resetAnimationFrameTicks();
	clearActorFrameSounds();
	_sceneItems.clear(); // NOIMPL: clearSceneItems();
	setEnterSceneScript(-1, -1);
	setLeaveSceneScript(-1, -1);

	if (sceneIndex == -1) {
		startModuleScript(modulePakName, 0);
		initDialog();
		clearInventoryItems();
		_screenTextShowing = false;
		_menuMouseCursorActive = false;
		_menuMouseCursor = -1;
		_zoneMouseCursorActive = false;
		_zoneMouseCursor = -1;
		updateMouseCursor();
		setFontDefaultColors();
	}

	clearScriptProgram(kSceneScriptProgram);
	loadScriptProgram(modulePakName, sceneIndex + 2, kSceneScriptProgram);

	_prevSceneIndex = _currSceneIndex;
	_prevModuleIndex = _currModuleIndex;
	_currSceneIndex = sceneIndex;
	_currModuleIndex = moduleIndex;
	_inScene = true;

	startScript(kSceneScriptProgram, 0);
	runInitScript(kSceneScriptProgram);

	bool backgroundFlag = _backgroundFlag;
	bool updateDirtyRectsFlag = _updateDirtyRectsFlag;
	bool needToUpdatePalette = _needToUpdatePalette;

	_needToUpdatePalette = true;
	_updateDirtyRectsFlag = true;
	_screen->clear();
	// TODO: resetDirtyRects();
	addDirtyRect(0, 0, 640, 480, 1);
	// TODO: setPalette(_blackPalette, 0, 240);
	// TODO: drawDirtyRects(false);
	addDirtyRect(0, 0, 640, 480, 1);
	_pathSystem->buildPathSystem();
	_pathSystem->setPathSystemBuilt(true);
	buildActorSpriteDrawQueue();
	updateBackground(true);
	_screen->setClipRect(0, 82, 639, 397);
	drawSprites(_cameraX, _cameraY);
	_screen->setClipRect(0, 0, 639, 479);
	// TODO: updateDirtyRects();

	_backgroundFlag = backgroundFlag;
	_updateDirtyRectsFlag = updateDirtyRectsFlag;
	_needToUpdatePalette = needToUpdatePalette;

	if (_enterSceneScriptIndex == -1)
		_updateDirtyRectsFlag = true;

	_sceneFlag = false;

	resetFrameValues();

}

void PrisonerEngine::leaveScene() {

	if (!_inScene)
		return;

	_inScene = false;

	_pathSystem->setPathSystemBuilt(false);
	stopScriptProgram(kSceneScriptProgram);
	clearSceneItemActors();
	clearZones();
	unloadScreenTexts();
	clearZoneActions();
	unloadSounds(_newModuleIndex != _currModuleIndex);
	unloadActors();
	// TODO: updatePalStucts();
	// TODO: clearPaletteValues
	unloadBackgroundObjects();
	unloadBackground();
	unloadScriptProgram(kSceneScriptProgram);
	if (_newModuleIndex != _currModuleIndex) {
		// TODO: unloadMusicItems
		stopModuleScript();
		// -> resetModuleScriptVars();
		for (uint i = 0; i < 300; i++)
			_moduleScriptVars[i] = -1;
		unloadDialogPanel();
		unloadInventoryItems();
		// Free all unused resources
		_res->purge();
		/* CHECKME: The original defragments the system memory here,
			I don't think that's neccessary on any supported platform.
		*/
	}

	resetFrameValues();

}

void PrisonerEngine::gotoScene(int16 moduleIndex, int16 sceneIndex) {
	debug(1, "PrisonerEngine::gotoScene(%d, %d)", moduleIndex, sceneIndex);
	_newModuleIndex = moduleIndex;
	_newSceneIndex = sceneIndex;
	stopScriptProgram(_currScriptProgramIndex);
}

void PrisonerEngine::actorEntersScene(int16 prevModuleIndex, int16 prevSceneIndex, int16 nodeIndex,
	Common::String &pakName, int16 pakSlot, int16 frameListIndex, int16 scriptIndex) {

	if (_prevModuleIndex != prevModuleIndex || _prevSceneIndex != prevSceneIndex)
		return;

	PathNode *node = _pathSystem->getNode(nodeIndex);

	if (_mainActorIndex != -1) {
		Actor *actor = &_actors[_mainActorIndex];
		actor->pathNodeIndex = nodeIndex;
		actor->actorSprite->x = node->x;
		actor->actorSprite->y = node->y;
		actor->actorSprite->scale = node->scale;
		setActorAnimation(_mainActorIndex, pakName, pakSlot, frameListIndex, -1);
	}

	updateCameraPosition(node->x, node->y);
	setEnterSceneScript(kSceneScriptProgram, scriptIndex);

	if (scriptIndex != -1) {
		setUserInput(false);
		startScript(kSceneScriptProgram, scriptIndex);
		if (_mainActorIndex != -1) {
			Script *script = &_scriptPrograms[kSceneScriptProgram].scripts[scriptIndex];
			script->status = kScriptStatusAnimation;
			script->actorIndex2 = _mainActorIndex;
		}
	}

}

} // End of namespace Prisoner
