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
 */

#include "bbvs/bbvs.h"
#include "bbvs/dialogs.h"
#include "bbvs/gamemodule.h"
#include "bbvs/graphics.h"
#include "bbvs/sound.h"
#include "bbvs/spritemodule.h"
#include "bbvs/minigames/minigame.h"
#include "bbvs/minigames/bbairguitar.h"
#include "bbvs/minigames/bbant.h"
#include "bbvs/minigames/bbloogie.h"
#include "bbvs/minigames/bbtennis.h"
#include "bbvs/minigames/minigame.h"

#include "audio/audiostream.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/fs.h"
#include "common/timer.h"
#include "engines/util.h"
#include "graphics/cursorman.h"
#include "graphics/font.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"

namespace Bbvs {

static const BBPoint kInventorySlotPositions[] = {
	{ 66, 191}, { 94, 217}, {192, 217}, {159, 213}, {228,  49},
	{137,  49}, {168, 165}, {101,  55}, {177,  46}, {165, 165},
	{202,  74}, {141,  53}, {164, 164}, {165,  78}, {167,  71},
	{142, 188}, {171, 100}, {250, 216}, {200,  72}, {200,  72},
	{101,  82}, { 67,  93}, {133,  87}, {123, 220}, {199, 129},
	{188, 192}, {102,  82}, {188, 192}, { 99, 170}, { 68, 126},
	{159, 130}, {102, 116}, {207, 157}, {130, 141}, {236, 100},
	{102, 197}, {141, 186}, {200, 102}, {221, 220}, {222, 188},
	{135,  93}, {134, 145}, { 96, 224}, {128, 224}, {160, 224},
	{192, 224}, {224, 224}, {240, 224}, {256, 224}, {  0,   0}
};
	
static const BBRect kVerbRects[6] = {
	{-32,  -2, 19, 27}, {-33, -33, 19, 27}, { 12,  -2, 19, 27},
	{ 13, -33, 19, 27}, {-10,   8, 19, 27}, {-11, -49, 19, 27}
};

static const int8 kWalkTurnTbl[] = {
	7, 9, 4, 8, 6, 10, 5, 11
};

static const int8 kWalkAnimTbl[32] = {
	 3,  0,  0,  0,  2,  1,  1,  1,
	15, 12, 14, 13,  0,  0,  0,  0,
	 7,  9,  4,  8,  6, 10,  5, 11,
	 3,  0,  2,  1, 15, 12, 14, 13
};

static const int8 kTurnInfo[8][8] = {
	{ 0,  1,  1,  1,  1, -1, -1, -1},
	{-1,  0,  1,  1,  1,  1, -1, -1},
	{-1, -1,  0,  1,  1,  1,  1, -1},
	{-1, -1, -1,  0,  1,  1,  1,  1},
	{ 1, -1, -1, -1,  0,  1,  1,  1},
	{ 1,  1, -1, -1, -1,  0,  1,  1},
	{ 1,  1,  1, -1, -1, -1,  0,  1},
	{ 1,  1,  1,  1, -1, -1, -1,  0}
};

static const byte kTurnTbl[] = {
	2, 6, 4, 0, 2, 6, 4, 0,
	3, 1, 5, 7, 0, 0, 0, 0
};

static const int kAfterVideoSceneNum[] = {
	 0, 43, 23, 12,  4, 44,  2,
	16,  4,  4,  4, 44, 12, 44
};

const int kMainMenu = 44;
const int kCredits  = 45;

bool WalkArea::contains(const Common::Point &pt) const {
	return Common::Rect(x, y, x + width, y + height).contains(pt);
}

BbvsEngine::BbvsEngine(OSystem *syst, const ADGameDescription *gd) :
	Engine(syst), _gameDescription(gd) {
	
	_random = new Common::RandomSource("bbvs");
	
	Engine::syncSoundSettings();

}

BbvsEngine::~BbvsEngine() {

	delete _random;

}

void BbvsEngine::newGame() {
	_currInventoryItem = -1;
	_newSceneNum = 32;
}

void BbvsEngine::continueGameFromQuickSave() {
	_bootSaveSlot = 0;
}

void BbvsEngine::setNewSceneNum(int newSceneNum) {
	_newSceneNum = newSceneNum;
}

Common::Error BbvsEngine::run() {

	_isSaveAllowed = false;
	_hasSnapshot = false;

	initGraphics(320, 240, false);

	_screen = new Screen(_system);
	_gameModule = new GameModule();
	_spriteModule = new SpriteModule();
	_sound = new SoundMan();
	
	allocSnapshot();
	memset(_easterEggInput, 0, sizeof(_easterEggInput));
	
	_gameTicks = 0;
	_playVideoNumber = 0;
	_bootSaveSlot = -1;
	
	memset(_inventoryItemStatus, 0, sizeof(_inventoryItemStatus));
	memset(_gameVars, 0, sizeof(_gameVars));
	memset(_sceneVisited, 0, sizeof(_sceneVisited));

	_mouseX = 160;
	_mouseY = 120;
	_mouseButtons = 0;

	_currVerbNum = kVerbLook;
	_currInventoryItem = -1;
	_currTalkObjectIndex = -1;
	_currSceneNum = 0;
	_newSceneNum = 31;

	if (ConfMan.hasKey("save_slot"))
		_bootSaveSlot = ConfMan.getInt("save_slot");

	while (!shouldQuit()) {
		updateEvents();
		if (_currSceneNum < kMainMenu || _newSceneNum > 0 || _bootSaveSlot >= 0)
			updateGame();
		else if (_currSceneNum == kMainMenu)
			runMainMenu();
		else if (_currSceneNum == kCredits &&
			(_mouseButtons & kAnyButtonClicked)) {
			_mouseButtons &= ~kAnyButtonClicked;
			_newSceneNum = kMainMenu;
		}
		if (_playVideoNumber > 0) {
			playVideo(_playVideoNumber);
			_playVideoNumber = 0;
		}
	}
	
	writeContinueSavegame();

	freeSnapshot();

	delete _sound;
	delete _spriteModule;
	delete _gameModule;
	delete _screen;

	return Common::kNoError;
}

bool BbvsEngine::hasFeature(EngineFeature f) const {
	return
		(f == kSupportsRTL) ||
		(f == kSupportsLoadingDuringRuntime) ||
		(f == kSupportsSavingDuringRuntime);
}

void BbvsEngine::updateEvents() {
	Common::Event event;

	while (_eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYDOWN:
			_keyCode = event.kbd.keycode;
			break;
		case Common::EVENT_KEYUP:
			checkEasterEgg(event.kbd.ascii);
			_keyCode = Common::KEYCODE_INVALID;
			break;
		case Common::EVENT_MOUSEMOVE:
  			_mouseX = event.mouse.x;
  			_mouseY = event.mouse.y;
  			break;
		case Common::EVENT_LBUTTONDOWN:
			_mouseButtons |= kLeftButtonClicked;
			_mouseButtons |= kLeftButtonDown;
  			break;
		case Common::EVENT_LBUTTONUP:
			_mouseButtons &= ~kLeftButtonDown;
  			break;
		case Common::EVENT_RBUTTONDOWN:
			_mouseButtons |= kRightButtonClicked;
			_mouseButtons |= kRightButtonDown;
  			break;
		case Common::EVENT_RBUTTONUP:
			_mouseButtons &= ~kRightButtonDown;
  			break;
		case Common::EVENT_QUIT:
			quitGame();
			break;
		default:
			break;
		}
	}
}

int BbvsEngine::getRandom(int max) {
	return max == 0 ? 0 : _random->getRandomNumber(max - 1);
}

void BbvsEngine::drawDebugInfo() {
#if 0
	Graphics::Surface *s = _screen->_surface;
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont);
	for (int i = 0; i < _walkAreasCount; ++i) {
		WalkArea *walkArea = &_walkAreas[i];
		Common::Rect r(walkArea->x, walkArea->y, walkArea->x + walkArea->width, walkArea->y + walkArea->height);
		s->frameRect(r, 255);
		Common::String text = Common::String::format("%d", i);
		font->drawString(s, text, r.left + 1, r.top + 1, 100, 11);
	}
#endif
}

void BbvsEngine::drawScreen() {
	drawDebugInfo();
	_screen->copyToScreen();
}

void BbvsEngine::updateGame() {
	int inputTicks;

	if (_gameTicks > 0) {
		int currTicks = _system->getMillis();
		inputTicks = (currTicks - _gameTicks) / 17;
		_gameTicks = currTicks - (currTicks - _gameTicks) % 17;
	} else {
		inputTicks = 1;
		_gameTicks = _system->getMillis();
	}
	
	if (inputTicks > 20) {
		inputTicks = 20;
		_gameTicks = _system->getMillis();
	}
	
	if (inputTicks == 0)
		return;
		
	if (_mouseX >= 320 || _mouseY >= 240) {
		_mouseY = -1;
		_mouseX = -1;
	}
	
	bool done;

	do {
		done = !update(_mouseX, _mouseY, _mouseButtons, _keyCode);
		_mouseButtons &= ~kLeftButtonClicked;
		_mouseButtons &= ~kRightButtonClicked;
		_keyCode = Common::KEYCODE_INVALID;
	} while (--inputTicks && _playVideoNumber == 0 && _gameTicks > 0 && !done);
	
	if (!done && _playVideoNumber == 0 && _gameTicks > 0) {
		DrawList drawList;
		buildDrawList(drawList);
		_screen->drawDrawList(drawList, _spriteModule);
		drawScreen();
	}
	
	_system->delayMillis(10);

}

bool BbvsEngine::evalCondition(Conditions &conditions) {
	bool result = true;
	for (int i = 0; i < 8 && result; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondSceneObjectVerb:
			result = _activeItemType == KITSceneObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectVerb:
			result = _activeItemType == kITBgObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondSceneObjectInventory:
			result = _activeItemType == KITSceneObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectInventory:
			result = _activeItemType == kITBgObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondHasInventoryItem:
			result = _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			result = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			result = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			result = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			result = condition.value2 == _prevSceneNum;
			break;
		case kCondIsCurrTalkObject:
			result = condition.value2 == _currTalkObjectIndex;
			break;
		case kCondIsDialogItem:
			result = _activeItemType == kITDialog &&
				condition.value1 == _activeItemIndex;
			break;
		case kCondIsCameraNum:
			result = condition.value1 == _currCameraNum;
			break;
		case kCondIsNotPrevSceneNum:
			result = condition.value2 != _prevSceneNum;
			break;
		case kCondIsButtheadAtBgObject:
			result = _buttheadObject &&
				_gameModule->getBgObject(condition.value2)->rect.contains(_buttheadObject->x >> 16, _buttheadObject->y >> 16);
			break;
		case kCondIsNotSceneVisited:
			result = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			result = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondUnused:
		case kCondDialogItem0:
		case kCondIsCameraNumTransition:
			result = false;
			break;
		}
	}
	return result;
}

bool BbvsEngine::evalCameraCondition(Conditions &conditions, int value) {
	bool result = true;
	for (int i = 0; i < 8 && result; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondHasInventoryItem:
			result =  _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			result = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			result = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			result = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			result = condition.value2 == _prevSceneNum;
			break;
		case kCondIsNotPrevSceneNum:
			result = condition.value2 != _prevSceneNum;
			break;
		case kCondIsNotSceneVisited:
			result = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			result = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondIsCameraNumTransition:
			result = condition.value1 == _currCameraNum &&
				condition.value2 == value;
			break;
		case kCondUnused:
		case kCondSceneObjectVerb:
		case kCondBgObjectVerb:
		case kCondSceneObjectInventory:
		case kCondBgObjectInventory:
		case kCondIsCurrTalkObject:
		case kCondIsDialogItem:
		case kCondIsCameraNum:
		case kCondDialogItem0:
		case kCondIsButtheadAtBgObject:
			result = false;
			break;
		default:
			break;
		}
	}
	return result;
}

int BbvsEngine::evalDialogCondition(Conditions &conditions) {
	int result = -1;
	bool success = false;
	for (int i = 0; i < 8; ++i) {
		const Condition &condition = conditions.conditions[i];
		switch (condition.cond) {
		case kCondSceneObjectVerb:
			success = _activeItemType == KITSceneObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectVerb:
			success = _activeItemType == kITBgObject &&
				condition.value1 == _currVerbNum &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondSceneObjectInventory:
			success = _activeItemType == KITSceneObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondBgObjectInventory:
			success = _activeItemType == kITBgObject &&
				_currVerbNum == kVerbInvItem &&
				condition.value1 == _currInventoryItem &&
				condition.value2 == _activeItemIndex;
			break;
		case kCondHasInventoryItem:
			success = _inventoryItemStatus[condition.value1] != 0;
			break;
		case kCondHasNotInventoryItem:
			success = _inventoryItemStatus[condition.value1] == 0;
			break;
		case kCondIsGameVar:
			success = _gameVars[condition.value2] != 0;
			break;
		case kCondIsNotGameVar:
			success = _gameVars[condition.value2] == 0;
			break;
		case kCondIsPrevSceneNum:
			success = condition.value2 == _prevSceneNum;
			break;
		case kCondIsCurrTalkObject:
			success = condition.value2 == _currTalkObjectIndex;
			break;
		case kCondIsDialogItem:
			result = condition.value1;
			break;
		case kCondIsCameraNum:
			success = condition.value1 == _currCameraNum;
			break;
		case kCondIsNotPrevSceneNum:
			success = condition.value2 != _prevSceneNum;
			break;
		case kCondIsButtheadAtBgObject:
			success = _buttheadObject &&
				_gameModule->getBgObject(condition.value2)->rect.contains(_buttheadObject->x >> 16, _buttheadObject->y >> 16);
			break;
		case kCondIsNotSceneVisited:
			success = _sceneVisited[_currSceneNum] == 0;
			break;
		case kCondIsSceneVisited:
			success = _sceneVisited[_currSceneNum] != 0;
			break;
		case kCondDialogItem0:
			return 0;
		case kCondUnused:
		case kCondIsCameraNumTransition:
			success = false;
			break;
		}
		if (!success)
			return -1;
	}
	return result;
}

void BbvsEngine::evalActionResults(ActionResults &results) {
	for (int i = 0; i < 8; ++i) {
		const ActionResult &result = results.actionResults[i];
		switch (result.kind) {
		case kActResAddInventoryItem:
			_inventoryItemStatus[result.value1] = 1;
			_currVerbNum = kVerbInvItem;
			_currInventoryItem = result.value1;
			break;
		case kActResRemoveInventoryItem:
			_inventoryItemStatus[result.value1] = 0;
			if (result.value1 == _currInventoryItem)
				_currInventoryItem = -1;
			if (_currVerbNum == kVerbInvItem)
				_currVerbNum = kVerbLook;
			break;
		case kActResSetGameVar:
			_gameVars[result.value2] = 1;
			break;
		case kActResUnsetGameVar:
			_gameVars[result.value2] = 0;
			break;
		case kActResStartDialog:
			_gameState = kGSDialog;
			break;
		case kActResChangeScene:
			_newSceneNum = result.value2;
			break;
		}
	}
}

void BbvsEngine::updateBackgroundSounds() {
	for (int i = 0; i < _gameModule->getSceneSoundsCount(); ++i) {
		SceneSound *sceneSound = _gameModule->getSceneSound(i);
		bool isActive = evalCondition(sceneSound->conditions);
		debug(5, "bgSound(%d) isActive: %d; soundNum: %d", i, isActive, sceneSound->soundNum);
		if (isActive && !_backgroundSoundsActive[i]) {
			playSound(sceneSound->soundNum, true);
			_backgroundSoundsActive[i] = 1;
		} else if (!isActive && _backgroundSoundsActive[i]) {
			stopSound(sceneSound->soundNum);
			_backgroundSoundsActive[i] = 0;
		}
	}
} 

void BbvsEngine::loadScene(int sceneNum) {
	debug(0, "BbvsEngine::loadScene() sceneNum: %d", sceneNum);
	
	Common::String sprFilename = Common::String::format("vnm/vspr%04d.vnm", sceneNum);
	Common::String gamFilename = Common::String::format("vnm/game%04d.vnm", sceneNum);

	_screen->clear();

	_spriteModule->load(sprFilename.c_str());
	_gameModule->load(gamFilename.c_str());
	
	Palette palette = _spriteModule->getPalette();
	_screen->setPalette(palette);

	// Preload sounds
	for (uint i = 0; i < _gameModule->getPreloadSoundsCount(); ++i) {
		Common::String filename = Common::String::format("snd/snd%05d.aif", _gameModule->getPreloadSound(i));
		_sound->loadSound(filename);
	}

	if (sceneNum >= kMainMenu) {
		DrawList drawList;
		drawList.add(_gameModule->getBgSpriteIndex(0), 0, 0, 0);
		_screen->drawDrawList(drawList, _spriteModule);
		drawScreen();
	}

}

void BbvsEngine::initScene(bool sounds) {

	stopSpeech();
	stopSounds();
	_sound->unloadSounds();

	_gameState = kGSScene;
	_prevSceneNum = _currSceneNum;
	_sceneVisited[_currSceneNum] = 1;
	_mouseCursorSpriteIndex = 0;
	_verbPos.x = -1;
	_verbPos.y = -1;
	_activeItemType = kITEmpty;
	_activeItemIndex = 0;
	_cameraPos.x = 0;
	_cameraPos.y = 0;
	_newCameraPos.x = 0;
	_newCameraPos.y = 0;
	_inventoryButtonIndex = -1;
	_currTalkObjectIndex = -1;
	_currCameraNum = 0;
	_walkMousePos.x = -1;
	_walkMousePos.y = -1;
	_currAction = 0;
	_currActionCommandIndex = -1;
	_currActionCommandTimeStamp = 0;
	_dialogSlotCount = 0;
	_buttheadObject = 0;
	_beavisObject = 0;

	memset(_backgroundSoundsActive, 0, sizeof(_backgroundSoundsActive));

	memset(_sceneObjects, 0, sizeof(_sceneObjects));
	for (int i = 0; i < kSceneObjectsCount; ++i) {
		_sceneObjects[i].walkDestPt.x = -1;
		_sceneObjects[i].walkDestPt.y = -1;
	}

	memset(_dialogItemStatus, 0, sizeof(_dialogItemStatus));

	_sceneObjectActions.clear();

	loadScene(_newSceneNum);
	_currSceneNum = _newSceneNum;
	_newSceneNum = 0;
	
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i)
		_sceneObjects[i].sceneObjectDef = _gameModule->getSceneObjectDef(i);
		
	for (int i = 0; i < _gameModule->getSceneObjectInitsCount(); ++i) {
		SceneObjectInit *soInit = _gameModule->getSceneObjectInit(i);
		if (evalCondition(soInit->conditions)) {
			SceneObject *sceneObject = &_sceneObjects[soInit->sceneObjectIndex];
			sceneObject->anim = _gameModule->getAnimation(soInit->animIndex);
			sceneObject->animIndex = soInit->animIndex;
			sceneObject->frameIndex = sceneObject->anim->frameCount - 1;
			sceneObject->frameTicks = 1;
			sceneObject->x = soInit->x << 16;
			sceneObject->y = soInit->y << 16;
		}
	}

	if (_gameModule->getButtheadObjectIndex() >= 0) {
		_buttheadObject = &_sceneObjects[_gameModule->getButtheadObjectIndex()];
		// Search for the Beavis object
		for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i)
			if (!strcmp(_sceneObjects[i].sceneObjectDef->name, "Beavis")) {
				_beavisObject = &_sceneObjects[i];
				break;
			}
	}

	updateSceneObjectsTurnValue();

	updateWalkableRects();

	_currCameraNum = 0;
	if (_buttheadObject) {
		int minDistance = 0xFFFFFF;
		for (int cameraNum = 0; cameraNum < 4; ++cameraNum) {
			CameraInit *cameraInit = _gameModule->getCameraInit(cameraNum);
			int curDistance = ABS(cameraInit->cameraPos.x - (int)(_buttheadObject->x >> 16) + 160);
			if (curDistance < minDistance) {
				minDistance = curDistance;
				_currCameraNum = cameraNum;
			}
		}
	}
	
	_cameraPos = _gameModule->getCameraInit(_currCameraNum)->cameraPos;
	_newCameraPos = _cameraPos;
	
	_walkAreaActions.clear();
	for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
		Action *action = _gameModule->getAction(i);
		for (int j = 0; j < 8; ++j)
			if (action->conditions.conditions[j].cond == kCondIsButtheadAtBgObject)
				_walkAreaActions.push_back(action);
	}

	_mouseCursorSpriteIndex = 0;

	_activeItemIndex = 0;
	_activeItemType = kITEmpty;
	
	for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
		Action *action = _gameModule->getAction(i);
		if (evalCondition(action->conditions)) {
			_gameState = kGSWait;
			_currAction = action;
			for (uint j = 0; j < action->actionCommands.size(); ++j) {
				ActionCommand *actionCommand = &action->actionCommands[j];
				if (actionCommand->cmd == kActionCmdSetCameraPos) {
					_currCameraNum = actionCommand->param;
					_cameraPos = _gameModule->getCameraInit(_currCameraNum)->cameraPos;
					_newCameraPos = _cameraPos;
					break;
				}
			}
			break;
		}
	}
	
	if (sounds)
		updateBackgroundSounds();

}

bool BbvsEngine::changeScene() {

	writeContinueSavegame();
	
	if (_newSceneNum >= 27 && _newSceneNum <= 30) {
		// Run minigames
		stopSpeech();
		stopSounds();
		_sceneVisited[_currSceneNum] = 1;
		if (runMinigame(_newSceneNum - 27)) {
			SWAP(_currSceneNum, _newSceneNum);
		}
	} else if (_newSceneNum >= 31 && _newSceneNum <= 43) {
		// Play video
		stopSpeech();
		stopSounds();
		_sceneVisited[_currSceneNum] = 1;
		_playVideoNumber = _newSceneNum - 30;
		_currSceneNum = _newSceneNum;
		_newSceneNum = kAfterVideoSceneNum[_playVideoNumber];
	} else if (_newSceneNum >= 100 && _currSceneNum == kCredits) {
		// Play secret video
		stopSounds();
		_playVideoNumber = _newSceneNum;
		_currSceneNum = 49;
		_newSceneNum = kCredits;
	} else {
		// Normal scene
		initScene(true);
	}

	return true;
	
}

bool BbvsEngine::update(int mouseX, int mouseY, uint mouseButtons, Common::KeyCode keyCode) {

	if (_bootSaveSlot >= 0) {
		loadGameState(_bootSaveSlot);
		_gameTicks = 0;
		_bootSaveSlot = -1;
		return false;
	}
	
	if (_newSceneNum != 0) {
		_gameTicks = 0;
		return changeScene();
	}

	_mousePos.x = mouseX + _cameraPos.x;
	_mousePos.y = mouseY + _cameraPos.y;

	switch (_gameState) {

	case kGSScene:
		_isSaveAllowed = true;
		saveSnapshot();
		if (mouseButtons & kRightButtonDown) {
			_verbPos = _mousePos;
			if (_mousePos.x - _cameraPos.x < 33)
				_verbPos.x = _cameraPos.x + 33;
			if (_verbPos.x - _cameraPos.x > 287)
				_verbPos.x = _cameraPos.x + 287;
			if (_verbPos.y - _cameraPos.y < 51)
				_verbPos.y = _cameraPos.y + 51;
			if (_verbPos.y - _cameraPos.y > 208)
				_verbPos.y = _cameraPos.y + 208;
			_gameState = kGSVerbs;
		} else {
			switch (keyCode) {
			case Common::KEYCODE_SPACE:
			case Common::KEYCODE_i:
				_inventoryButtonIndex = -1;
				_gameState = kGSInventory;
				return true;
			case Common::KEYCODE_l:
				_currVerbNum = kVerbLook;
				break;
			case Common::KEYCODE_t:
				_currVerbNum = kVerbTalk;
				break;
			case Common::KEYCODE_u:
				_currVerbNum = kVerbUse;
				break;
			case Common::KEYCODE_w:
				_currVerbNum = kVerbWalk;
				break;
			default:
				break;
			}
			updateScene(mouseButtons & kLeftButtonClicked);
			updateCommon();
		}
		break;

	case kGSInventory:
		_isSaveAllowed = true;
		saveSnapshot();
		if (mouseButtons & kRightButtonClicked)
			_currVerbNum = kVerbUse;
		switch (keyCode) {
		case Common::KEYCODE_SPACE:
		case Common::KEYCODE_i:
			_gameState = kGSScene;
			stopSpeech();
			return true;
		case Common::KEYCODE_l:
			_currVerbNum = kVerbLook;
			break;
		case Common::KEYCODE_u:
			_currVerbNum = kVerbUse;
			break;
		default:
			break;
		}
		updateInventory(mouseButtons & kLeftButtonClicked);
		break;
	
	case kGSVerbs:
		_isSaveAllowed = false;
		updateVerbs();
		if (!(mouseButtons & kRightButtonDown)) {
			if (_currVerbNum == kVerbShowInv) {
				_inventoryButtonIndex = -1;
				_gameState = kGSInventory;
			} else {
				_gameState = kGSScene;
			}
		}
		break;
	
	case kGSWait:
	case kGSWaitDialog:
		_isSaveAllowed = false;
		_activeItemType = kITEmpty;
		_activeItemIndex = 0;
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(9);
		if (keyCode == Common::KEYCODE_ESCAPE)
			skipCurrAction();
		else
			updateCommon();
		break;
	
	case kGSDialog:
		_isSaveAllowed = true;
		saveSnapshot();
		updateDialog(mouseButtons & kLeftButtonClicked);
		updateCommon();
		break;
	
	}

	return true;
}

void BbvsEngine::buildDrawList(DrawList &drawList) {

	if (_gameState == kGSInventory) {

		// Inventory background
		drawList.add(_gameModule->getGuiSpriteIndex(15), 0, 0, 0);

		// Inventory button
		if (_inventoryButtonIndex == 0)
			drawList.add(_gameModule->getGuiSpriteIndex(18 + 0), 97, 13, 1);
		else if (_inventoryButtonIndex == 1)
			drawList.add(_gameModule->getGuiSpriteIndex(18 + 1), 135, 15, 1);
		else if (_inventoryButtonIndex == 2)
			drawList.add(_gameModule->getGuiSpriteIndex(18 + 2), 202, 13, 1);

		// Inventory items
		int currItem = -1;
		if (_currVerbNum == kVerbInvItem)
			currItem = _currInventoryItem;
		for (int i = 0; i < 50; ++i)
			if (_inventoryItemStatus[i] && currItem != i)
				drawList.add(_gameModule->getInventoryItemSpriteIndex(i * 2), kInventorySlotPositions[i].x, kInventorySlotPositions[i].y, 1);

	} else {

		// Scene objects
		for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
			SceneObject *sceneObject = &_sceneObjects[i];
			Animation *anim = sceneObject->anim;
			if (anim) {
				drawList.add(anim->frameSpriteIndices[sceneObject->frameIndex],
					(sceneObject->x >> 16) - _cameraPos.x, (sceneObject->y >> 16) - _cameraPos.y,
					sceneObject->y >> 16);
			}
		}

		// Background objects
		for (int i = 0; i < _gameModule->getBgSpritesCount(); ++i)
			drawList.add(_gameModule->getBgSpriteIndex(i), -_cameraPos.x, -_cameraPos.y, _gameModule->getBgSpritePriority(i));

		if (_gameState == kGSVerbs) {
			// Verbs icon background
			for (int i = 0; i < 6; ++i) {
				if (i != 4) {
					int index = (i == _activeItemIndex) ? 17 : 16;
					drawList.add(_gameModule->getGuiSpriteIndex(index), _verbPos.x + kVerbRects[i].x - _cameraPos.x,
						_verbPos.y + kVerbRects[i].y - _cameraPos.y, 499);
				}
			}
			// Verbs background
			drawList.add(_gameModule->getGuiSpriteIndex(13), _verbPos.x - _cameraPos.x,
				_verbPos.y - _cameraPos.y, 500);
			// Selected inventory item		
			if (_currInventoryItem >= 0) {
				drawList.add(_gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem), _verbPos.x - _cameraPos.x,
					_verbPos.y - _cameraPos.y + 27, 500);
			}
		}

		if (_gameState == kGSDialog) {
			// Dialog background
			drawList.add(_gameModule->getGuiSpriteIndex(14), 0, 0, 500);
			// Dialog icons
			int iconX = 16;
			for (int i = 0; i < 50; ++i)
				if (_dialogItemStatus[i]) {
					drawList.add(_gameModule->getDialogItemSpriteIndex(i), iconX, 36, 501);
					iconX += 32;
				}
		}

	}

	// Mouse cursor
	if (_mouseCursorSpriteIndex > 0 && _mousePos.x >= 0)
		drawList.add(_mouseCursorSpriteIndex, _mousePos.x - _cameraPos.x, _mousePos.y - _cameraPos.y, 1000);

}

void BbvsEngine::updateVerbs() {

	_activeItemIndex = 99;

	if (_mousePos.x < 0) {
		_mouseCursorSpriteIndex = 0;
		return;
	}
	
	for (int i = 0; i < 6; ++i) {
		const BBRect &verbRect = kVerbRects[i];
		const int16 x = _verbPos.x + verbRect.x;
		const int16 y = _verbPos.y + verbRect.y;
		if (Common::Rect(x, y, x + verbRect.width, y + verbRect.height).contains(_mousePos)) {
			if (i != kVerbInvItem || _currInventoryItem >= 0) {
				_currVerbNum = i;
				_activeItemIndex = i;
			}
			break;
		}
	}
	
	switch (_currVerbNum) {
	case kVerbLook:
	case kVerbUse:
	case kVerbTalk:
	case kVerbWalk:
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(2 * _currVerbNum);
		break;
	case kVerbInvItem:
		_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem);
		break;
	case kVerbShowInv:
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(8);
		break;
	}

}

void BbvsEngine::updateDialog(bool clicked) {

	if (_mousePos.x < 0) {
		_mouseCursorSpriteIndex = 0;
		_activeItemType = 0;
		return;
	}

	if (_mousePos.y > 32) {
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(10);
		_activeItemIndex = 0;
		_activeItemType = kITEmpty;
		if (clicked)
			_gameState = kGSScene;
		return;
	}
	
	int slotX = (_mousePos.x - _cameraPos.x) / 32;

	if (slotX >= _dialogSlotCount) {
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(4);
		_activeItemType = kITEmpty;
		_activeItemIndex = 0;
		return;
	}

	_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(5);
	_activeItemType = kITDialog;
	
	// Find the selected dialog item index
	for (int i = 0; i < 50 && slotX >= 0; ++i) {
		if (_dialogItemStatus[i]) {
			--slotX;
			_activeItemIndex = i;
		}
	}
	
	// Select the dialog item action if it was clicked
	if (clicked) {
		for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
			Action *action = _gameModule->getAction(i);
			if (evalCondition(action->conditions)) {
				_mouseCursorSpriteIndex = 0;
				_gameState = kGSWaitDialog;
				_currAction = action;
				break;
			}
		}
	}

}

void BbvsEngine::updateInventory(bool clicked) {
	
	Common::Rect kInvButtonRects[3] = {
		Common::Rect(97, 13, 97 + 20, 13 + 26),
		Common::Rect(135, 15, 135 + 46, 15 + 25),
		Common::Rect(202, 13, 202 + 20, 13 + 26)};

	if (_mousePos.x < 0) {
		_mouseCursorSpriteIndex = 0;
		_activeItemType = 0;
		return;
	}

	if (_currVerbNum != kVerbLook && _currVerbNum != kVerbUse && _currVerbNum != kVerbInvItem)
		_currVerbNum = kVerbUse;
	
	const int16 mx = _mousePos.x - _cameraPos.x;
	const int16 my = _mousePos.y - _cameraPos.y;
	
	// Check inventory exit left/right edge of screen
	if (mx < 40 || mx > 280) {
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(10);
		_activeItemIndex = 0;
		_activeItemType = kITEmpty;
		if (clicked) {
			_gameState = kGSScene;
			stopSpeech();
		}
		return;
	}

	// Check hovered/clicked inventory button	
	_inventoryButtonIndex = -1;
	if (kInvButtonRects[0].contains(mx, my)) {
		_inventoryButtonIndex = 0;
		if (clicked)
			_currVerbNum = kVerbLook;
	} else if (kInvButtonRects[2].contains(mx, my)) {
		_inventoryButtonIndex = 2;
		if (clicked)
			_currVerbNum = kVerbUse;
	} else if (kInvButtonRects[1].contains(mx, my)) {
		_inventoryButtonIndex = 1;
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(10);
		_activeItemIndex = 0;
		_activeItemType = kITEmpty;
		if (clicked) {
			_gameState = kGSScene;
			stopSpeech();
		}
		return;
	}

	// Find hovered/clicked inventory item

	int currItem = -1;
	
	if (_currVerbNum == kVerbInvItem)
		currItem = _currInventoryItem;
		
	_activeItemType = kITEmpty;

	for (int i = 0; i < 50; ++i) {
		if (_inventoryItemStatus[i] && i != currItem) {
			InventoryItemInfo *info = _gameModule->getInventoryItemInfo(i);
			const int16 sx = kInventorySlotPositions[i].x + info->xOffs;
			const int16 sy = kInventorySlotPositions[i].y + info->yOffs;
			if (Common::Rect(sx, sy, sx + info->width, sy + info->height).contains(mx, my)) {
				_activeItemType = kITInvItem;
				_activeItemIndex = i;
				break;
			}
		}
	}

	// Update mouse cursor and select inventory item if clicked

	if (_activeItemType == kITInvItem) {
		if (clicked) {
			if (_currVerbNum == kVerbLook) {
				stopSpeech();
				playSpeech(_activeItemIndex + 10000);
			} else if (_currVerbNum == kVerbUse) {
				_currInventoryItem = _activeItemIndex;
				_currVerbNum = kVerbInvItem;
				_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _activeItemIndex);
			} else if (_currVerbNum == kVerbInvItem) {
				if ((_currInventoryItem == 22 && _activeItemIndex == 39) ||
					(_currInventoryItem == 39 && _activeItemIndex == 22)) {
					_inventoryItemStatus[22] = 0;
					_inventoryItemStatus[39] = 0;
					_inventoryItemStatus[40] = 1;
					_currVerbNum = kVerbInvItem;
					_currInventoryItem = 40;
					_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(40);
				}
				if ((_currInventoryItem == 25 && _activeItemIndex == 26) ||
					(_currInventoryItem == 26 && _activeItemIndex == 25)) {
					_inventoryItemStatus[26] = 0;
					_inventoryItemStatus[25] = 0;
					_inventoryItemStatus[27] = 1;
					_currVerbNum = kVerbInvItem;
					_currInventoryItem = 27;
					_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(27);
				}
			}
		} else {
			if (_currVerbNum == kVerbLook)
				_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(1);
			else if (_currVerbNum == kVerbUse)
				_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(3);
			else if (_currVerbNum == kVerbInvItem)
				_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem + 1);
		}
	} else {
		if (_currVerbNum >= kVerbInvItem)
			_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem);
		else
			_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(2 * _currVerbNum);
	}

}

void BbvsEngine::updateScene(bool clicked) {

	if (_mousePos.x < 0) {
		_mouseCursorSpriteIndex = 0;
		_activeItemType = kITNone;
		return;
	}

	int lastPriority = 0;

	_activeItemType = kITEmpty;

	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *sceneObject = &_sceneObjects[i];
		if (sceneObject->anim) {
			Common::Rect frameRect = sceneObject->anim->frameRects1[sceneObject->frameIndex];
			const int objY = sceneObject->y >> 16;
			frameRect.translate(sceneObject->x >> 16, objY);
			if (lastPriority <= objY && frameRect.width() > 0 && frameRect.contains(_mousePos)) {
				lastPriority = objY;
				_activeItemIndex = i;
				_activeItemType = KITSceneObject;
			}
		}
	}
	
	for (int i = 0; i < _gameModule->getBgObjectsCount(); ++i) {
		BgObject *bgObject = _gameModule->getBgObject(i);
		if (lastPriority <= bgObject->rect.bottom && bgObject->rect.contains(_mousePos)) {
			lastPriority = bgObject->rect.bottom;
			_activeItemIndex = i;
			_activeItemType = kITBgObject;
		}
	}

	if (_currVerbNum >= kVerbInvItem)
		_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem);
	else
		_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(2 * _currVerbNum);

	bool checkMore = true;

	if (_activeItemType == KITSceneObject || _activeItemType == kITBgObject) {
		for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
			Action *action = _gameModule->getAction(i);
			if (evalCondition(action->conditions)) {
				checkMore = false;
				if (clicked) {
					_mouseCursorSpriteIndex = 0;
					_gameState = kGSWait;
					_currAction = action;
					if (_currVerbNum == kVerbTalk)
						_currTalkObjectIndex = _activeItemIndex;
					if (_buttheadObject) {
						_buttheadObject->walkDestPt.x = -1;
						_buttheadObject->walkCount = 0;
					}
				} else {
					if (_currVerbNum >= kVerbInvItem)
						_mouseCursorSpriteIndex = _gameModule->getInventoryItemSpriteIndex(2 * _currInventoryItem + 1);
					else
						_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(2 * _currVerbNum + 1);
				}
				break;
			}
		}
	}

	// Test scroll arrow left
	if (checkMore && _buttheadObject && _buttheadObject->anim && _mousePos.x - _cameraPos.x < 16 && _currCameraNum > 0) {
		--_currCameraNum;
		for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
			Action *action = _gameModule->getAction(i);
			if (evalCameraCondition(action->conditions, _currCameraNum + 1)) {
				checkMore = false;
				if (clicked) {
					_mouseCursorSpriteIndex = 0;
					_gameState = kGSWait;
					_currAction = action;
					_buttheadObject->walkDestPt.x = -1;
					_buttheadObject->walkCount = 0;
				} else {
					_activeItemType = kITScroll;
					_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(12);
				}
				break;
			}
		}
		++_currCameraNum;
	}

	// Test scroll arrow right
	if (checkMore && _buttheadObject && _buttheadObject->anim && _mousePos.x - _cameraPos.x >= 304 && _currCameraNum < 4) {
		++_currCameraNum;
		for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
			Action *action = _gameModule->getAction(i);
			if (evalCameraCondition(action->conditions, _currCameraNum - 1)) {
				checkMore = false;
				if (clicked) {
					_mouseCursorSpriteIndex = 0;
					_gameState = kGSWait;
					_currAction = action;
					_buttheadObject->walkDestPt.x = -1;
					_buttheadObject->walkCount = 0;
				} else {
					_activeItemType = kITScroll;
					_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(11);
				}
				break;
			}
		}
		--_currCameraNum;
	}

	if (checkMore && _buttheadObject && _buttheadObject->anim) {
		_walkMousePos = _mousePos;

		while (1) {
			int foundIndex = -1;

			for (int i = 0; i < _walkableRectsCount; ++i)
				if (_walkableRects[i].contains(_walkMousePos)) {
					foundIndex = i;
					break;
				}

			if (foundIndex >= 0) {
				if (_walkMousePos.y != _mousePos.y)
					_walkMousePos.y = _walkableRects[foundIndex].top;
				break;
			} else {
				_walkMousePos.y += 4;
				if (_walkMousePos.y >= 240)
					break;
			}

		}

		if (_beavisObject->anim) {
			Common::Rect frameRect = _beavisObject->anim->frameRects2[_beavisObject->frameIndex];
			frameRect.translate(_beavisObject->x >> 16, (_beavisObject->y >> 16) + 1);
			if (!frameRect.isEmpty() && frameRect.contains(_walkMousePos))
				_walkMousePos.y = frameRect.bottom;
		}

		if (_walkMousePos.y < 240 && canButtheadWalkToDest(_walkMousePos)) {
			if (clicked) {
				_buttheadObject->walkDestPt = _walkMousePos;
				_buttheadObject->walkCount = 0;
			}
			for (int i = 0; i < _gameModule->getSceneExitsCount(); ++i) {
				SceneExit *sceneExit = _gameModule->getSceneExit(i);
				if (sceneExit->rect.contains(_walkMousePos.x, _walkMousePos.y)) {
					_activeItemIndex = i;
					_activeItemType = kITSceneExit;
					_mouseCursorSpriteIndex = _gameModule->getGuiSpriteIndex(10);
				}
			}
		} else {
			_walkMousePos.x = -1;
			_walkMousePos.y = -1;
		}

	}

}

bool BbvsEngine::performActionCommand(ActionCommand *actionCommand) {
	debug(5, "BbvsEngine::performActionCommand() cmd: %d", actionCommand->cmd);

	switch (actionCommand->cmd) {

	case kActionCmdStop:
		stopSpeech();
		return false;

	case kActionCmdWalkObject:
		{			
			SceneObject *sceneObject = &_sceneObjects[actionCommand->sceneObjectIndex];
			debug(5, "[%s] walks from (%d, %d) to (%d, %d)", sceneObject->sceneObjectDef->name,
				sceneObject->x >> 16, sceneObject->y >> 16, actionCommand->walkDest.x, actionCommand->walkDest.y);
			walkObject(sceneObject, actionCommand->walkDest, actionCommand->param);
		}
		return true;

	case kActionCmdMoveObject:
		{
			SceneObject *sceneObject = &_sceneObjects[actionCommand->sceneObjectIndex];
			sceneObject->x = actionCommand->walkDest.x << 16;
			sceneObject->y = actionCommand->walkDest.y << 16;
			sceneObject->xIncr = 0;
			sceneObject->yIncr = 0;
			sceneObject->walkCount = 0;
		}
		return true;

	case kActionCmdAnimObject:
		{
			SceneObject *sceneObject = &_sceneObjects[actionCommand->sceneObjectIndex];
			if (actionCommand->param == 0) {
				sceneObject->anim = 0;
				sceneObject->animIndex = 0;
				sceneObject->frameTicks = 0;
				sceneObject->frameIndex = 0;
			} else if (actionCommand->timeStamp != 0 || sceneObject->anim != _gameModule->getAnimation(actionCommand->param)) {
				sceneObject->animIndex = actionCommand->param;
				sceneObject->anim = _gameModule->getAnimation(actionCommand->param);
				sceneObject->frameIndex = sceneObject->anim->frameCount - 1;
				sceneObject->frameTicks = 1;
			}
		}
		return true;

	case kActionCmdSetCameraPos:
		_currCameraNum = actionCommand->param;
		_newCameraPos = _gameModule->getCameraInit(_currCameraNum)->cameraPos;
		updateBackgroundSounds();
		return true;

	case kActionCmdPlaySpeech:
		playSpeech(actionCommand->param);
		return true;

	case kActionCmdPlaySound:
		playSound(actionCommand->param);
		return true;

	case kActionCmdStartBackgroundSound:
		{
			const uint soundIndex = _gameModule->getSceneSoundIndex(actionCommand->param);
			if (!_backgroundSoundsActive[soundIndex]) {
				_backgroundSoundsActive[soundIndex] = 1;
				playSound(actionCommand->param, true);
			}
		}
		return true;

	case kActionCmdStopBackgroundSound:
		{
			const uint soundIndex = _gameModule->getSceneSoundIndex(actionCommand->param);
			_backgroundSoundsActive[soundIndex] = 0;
			stopSound(actionCommand->param);
		}
		return true;

	default:
		return true;

	}

}

bool BbvsEngine::processCurrAction() {
	bool actionsFinished = false;
	
	if (_sceneObjectActions.size() == 0) {
	
		for (uint i = 0; i < _currAction->actionCommands.size(); ++i) {
			ActionCommand *actionCommand = &_currAction->actionCommands[i];
			if (actionCommand->timeStamp != 0)
				break;
				
			if (actionCommand->cmd == kActionCmdMoveObject || actionCommand->cmd == kActionCmdAnimObject) {
				SceneObjectAction *sceneObjectAction = 0;
				// See if there's already an entry for the SceneObject
				for (uint j = 0; j < _sceneObjectActions.size(); ++j)
					if (_sceneObjectActions[j].sceneObjectIndex == actionCommand->sceneObjectIndex) {
						sceneObjectAction = &_sceneObjectActions[j];
						break;
					}
				// If not, add one
				if (!sceneObjectAction) {
					SceneObjectAction newSceneObjectAction;
					newSceneObjectAction.sceneObjectIndex = actionCommand->sceneObjectIndex;
					_sceneObjectActions.push_back(newSceneObjectAction);
					sceneObjectAction = &_sceneObjectActions.back();
				}
				if (actionCommand->cmd == kActionCmdMoveObject) {
					sceneObjectAction->walkDest = actionCommand->walkDest;
				} else {
					sceneObjectAction->animationIndex = actionCommand->param;
				}
			}
			
			if (actionCommand->cmd == kActionCmdSetCameraPos) {
				_currCameraNum = actionCommand->param;
				_newCameraPos = _gameModule->getCameraInit(actionCommand->param)->cameraPos;
			}

		}
		
		// Delete entries for SceneObjects without anim
		for (uint i = 0; i < _sceneObjectActions.size();) {
			if (!_sceneObjects[_sceneObjectActions[i].sceneObjectIndex].anim)
				_sceneObjectActions.remove_at(i);
			else
				++i;
		}

		// Prepare affected scene objects
		for (uint i = 0; i < _sceneObjectActions.size(); ++i) {
			_sceneObjects[_sceneObjectActions[i].sceneObjectIndex].walkCount = 0;
			_sceneObjects[_sceneObjectActions[i].sceneObjectIndex].turnCount = 0;
		}

	}

	actionsFinished = true;

	// Update SceneObject actions (walk and turn)
	for (uint i = 0; i < _sceneObjectActions.size(); ++i) {
		SceneObjectAction *soAction = &_sceneObjectActions[i];
		SceneObject *sceneObject = &_sceneObjects[soAction->sceneObjectIndex];
		if (sceneObject->walkDestPt.x != -1) {
			debug(5, "waiting for walk to finish");
			actionsFinished = false;
		} else if ((int16)(sceneObject->x >> 16) != soAction->walkDest.x || (int16)(sceneObject->y >> 16) != soAction->walkDest.y) {
			debug(5, "starting to walk");
			sceneObject->walkDestPt = soAction->walkDest;
			actionsFinished = false;
		} else if (sceneObject->walkCount == 0 && sceneObject->turnCount == 0) {
			debug(5, "not walking");
			for (int turnCount = 0; turnCount < 8; ++turnCount)
				if (sceneObject->sceneObjectDef->animIndices[kWalkTurnTbl[turnCount]] == soAction->animationIndex && sceneObject->turnValue != turnCount) {
					sceneObject->turnCount = turnCount | 0x80;
					break;
				}
		}
		if (sceneObject->turnCount)
			actionsFinished = false;
	}

	if (actionsFinished)
		_sceneObjectActions.clear();

	return actionsFinished;
}

void BbvsEngine::skipCurrAction() {
	ActionCommands &actionCommands = _currAction->actionCommands;
	while (_currAction && _newSceneNum == 0)
		updateCommon();
	for (uint i = 0; i < actionCommands.size(); ++i)
		if (actionCommands[i].cmd == kActionCmdPlaySound)
			stopSound(actionCommands[i].param);
	_system->delayMillis(250);
	_gameTicks = 0;
}

void BbvsEngine::updateCommon() {

	if (_currAction) {

		bool doActionCommands = true;

		if (_currActionCommandTimeStamp == 0) {
			doActionCommands = processCurrAction();
			_currActionCommandIndex = 0;
		}

		if (doActionCommands) {

			ActionCommand *actionCommand = &_currAction->actionCommands[_currActionCommandIndex];
			
			while (actionCommand->timeStamp == _currActionCommandTimeStamp &&
				_currActionCommandIndex < (int)_currAction->actionCommands.size()) {
				if (!performActionCommand(actionCommand)) {
					_gameState = kGSScene;
					evalActionResults(_currAction->results);
					if (_gameState == kGSDialog)
						updateDialogConditions();
					_currAction = 0;
					_currActionCommandTimeStamp = 0;
					_currActionCommandIndex = -1;
					updateSceneObjectsTurnValue();
					updateWalkableRects();
					break;
				}
				actionCommand = &_currAction->actionCommands[++_currActionCommandIndex];
			}

			if (_currAction) {
				++_currActionCommandTimeStamp;
			} else {
				_activeItemIndex = 0;
				_mouseCursorSpriteIndex = 0;
				_activeItemType = kITEmpty;
				for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
					Action *action = _gameModule->getAction(i);
					if (evalCondition(action->conditions)) {
						_gameState = kGSWait;
						_currAction = action;
					}
				}
			}

		}

	}

	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *sceneObject = &_sceneObjects[i];

		if (sceneObject->walkDestPt.x != -1) {
			if (sceneObject->walkCount == 0) {
				debug(5, "[%s] needs to walk", sceneObject->sceneObjectDef->name);
				startWalkObject(sceneObject);
				if (sceneObject->walkCount == 0) {
					debug(5, "no walk possible");
					sceneObject->walkDestPt.x = -1;
					sceneObject->walkDestPt.y = -1;
					sceneObject->xIncr = 0;
					sceneObject->yIncr = 0;
				}
			}
			updateWalkObject(sceneObject);
		}
		
		if (sceneObject->walkCount > 0 && sceneObject->turnCount == 0) {
			debug(5, "walk step, xIncr: %d, yIncr: %d", sceneObject->xIncr, sceneObject->yIncr);
			sceneObject->x += sceneObject->xIncr;
			sceneObject->y += sceneObject->yIncr;
			--sceneObject->walkCount;
		} else if (sceneObject->turnCount != 0) {
			debug(5, "need turn, turnCount: %d", sceneObject->turnCount);
			turnObject(sceneObject);
		}

		if (sceneObject == _buttheadObject && sceneObject->walkDestPt.x != -1) {
			for (uint j = 0; j < _walkAreaActions.size(); ++j) {
				if (_walkAreaActions[j] != _currAction && evalCondition(_walkAreaActions[j]->conditions)) {
					_sceneObjectActions.clear();
					_gameState = kGSWait;
					_currAction = _walkAreaActions[j];
					_currActionCommandTimeStamp = 0;
					_currActionCommandIndex = -1;
					for (int k = 0; k < _gameModule->getSceneObjectDefsCount(); ++k) {
						SceneObject *sceneObject2 = &_sceneObjects[k];
						sceneObject2->walkDestPt.x = -1;
						sceneObject2->walkDestPt.y = -1;
						sceneObject2->walkCount = 0;
					}
					break;
				}
			}
		}

		if (sceneObject->anim && --sceneObject->frameTicks == 0) {
			if (++sceneObject->frameIndex >= sceneObject->anim->frameCount)
				sceneObject->frameIndex = 0;
			sceneObject->frameTicks = sceneObject->anim->frameTicks[sceneObject->frameIndex];
		}

	}

	if (!_currAction && _buttheadObject) {
		int16 buttheadX = _buttheadObject->x >> 16;
		int16 buttheadY = _buttheadObject->y >> 16;
		CameraInit *cameraInit = _gameModule->getCameraInit(_currCameraNum);
		for (int i = 0; i < 8; ++i) {
			if (cameraInit->rects[i].contains(buttheadX, buttheadY)) {
				int newCameraNum = cameraInit->cameraLinks[i];
				if (_currCameraNum != newCameraNum) {
					int prevCameraNum = _currCameraNum;
					_currCameraNum = newCameraNum;
					_newCameraPos = _gameModule->getCameraInit(newCameraNum)->cameraPos;
					for (int j = 0; j < _gameModule->getActionsCount(); ++j) {
						Action *action = _gameModule->getAction(j);
						if (evalCameraCondition(action->conditions, prevCameraNum)) {
							_gameState = kGSWait;
							_currAction = action;
							_mouseCursorSpriteIndex = 0;
							_buttheadObject->walkDestPt.x = -1;
							_buttheadObject->walkCount = 0;
							break;
						}
					}
					updateBackgroundSounds();
				}
			}
		}
	}
	
	if (_cameraPos.x < _newCameraPos.x)
		++_cameraPos.x;
	if (_cameraPos.x > _newCameraPos.x)
		--_cameraPos.x;
	if (_cameraPos.y < _newCameraPos.y)
		++_cameraPos.y;
	if (_cameraPos.y > _newCameraPos.y)
		--_cameraPos.y;
	
	// Check if Butthead is inside a scene exit
	if (_newSceneNum == 0 && !_currAction && _buttheadObject) {
		int16 buttheadX = _buttheadObject->x >> 16;
		int16 buttheadY = _buttheadObject->y >> 16;
		for (int i = 0; i < _gameModule->getSceneExitsCount(); ++i) {
			SceneExit *sceneExit = _gameModule->getSceneExit(i);
			if (sceneExit->rect.contains(buttheadX, buttheadY)) {
				_newSceneNum = sceneExit->newModuleNum;
				break;
			}
		}
	}

}

void BbvsEngine::startWalkObject(SceneObject *sceneObject) {
	const int kMaxDistance = 0xFFFFFF;

	if (_buttheadObject != sceneObject && _beavisObject != sceneObject)
		return;
	
	initWalkAreas(sceneObject);
	_sourceWalkAreaPt.x = sceneObject->x >> 16;
	_sourceWalkAreaPt.y = sceneObject->y >> 16;

	_sourceWalkArea = getWalkAreaAtPos(_sourceWalkAreaPt);
	if (!_sourceWalkArea)
		return;

	_destWalkAreaPt = sceneObject->walkDestPt;

	_destWalkArea = getWalkAreaAtPos(_destWalkAreaPt);
	if (!_destWalkArea)
		return;
		
	if (_sourceWalkArea != _destWalkArea) {
		_currWalkDistance = kMaxDistance;
		walkFindPath(_sourceWalkArea, 0);
		_destWalkAreaPt = _currWalkDistance == kMaxDistance ? _sourceWalkAreaPt : _finalWalkPt;
	}

	walkObject(sceneObject, _destWalkAreaPt, sceneObject->sceneObjectDef->walkSpeed);
	
}

void BbvsEngine::updateWalkObject(SceneObject *sceneObject) {
	int animIndex;
	
	if (sceneObject->walkCount > 0 && (sceneObject->xIncr != 0 || sceneObject->yIncr != 0)) {
		if (ABS(sceneObject->xIncr) <= ABS(sceneObject->yIncr))
			sceneObject->turnValue = sceneObject->yIncr >= 0 ? 0 : 4;
		else
			sceneObject->turnValue = sceneObject->xIncr >= 0 ? 6 : 2;
		animIndex = sceneObject->sceneObjectDef->animIndices[kWalkAnimTbl[sceneObject->turnValue]];
		sceneObject->turnCount = 0;
		sceneObject->turnTicks = 0;
	} else {
		animIndex = sceneObject->sceneObjectDef->animIndices[kWalkTurnTbl[sceneObject->turnValue]];
	}

	Animation *anim = 0;
	if (animIndex > 0)
		anim = _gameModule->getAnimation(animIndex);
	
	if (sceneObject->anim != anim) {
		if (anim) {
			sceneObject->anim = anim;
			sceneObject->animIndex = animIndex;
			sceneObject->frameTicks = 1;
			sceneObject->frameIndex = anim->frameCount - 1;
		} else {
			sceneObject->anim = 0;
			sceneObject->animIndex = 0;
			sceneObject->frameTicks = 0;
			sceneObject->frameIndex = 0;
		}
	}

}

void BbvsEngine::walkObject(SceneObject *sceneObject, const Common::Point &destPt, int walkSpeed) {
	int deltaX = destPt.x - (sceneObject->x >> 16);
	int deltaY = destPt.y - (sceneObject->y >> 16);
	float distance = sqrt((double)(deltaX * deltaX + deltaY * deltaY));
	// NOTE The original doesn't have this check but without it the whole pathfinding breaks
	if (distance > 0.0) {
		sceneObject->walkCount = distance / ((((float)ABS(deltaX) / distance) + 1.0) * ((float)walkSpeed / 120));
		sceneObject->xIncr = ((float)deltaX / sceneObject->walkCount) * 65536.0;
		sceneObject->yIncr = ((float)deltaY / sceneObject->walkCount) * 65536.0;
		sceneObject->x = (sceneObject->x & 0xFFFF0000) | 0x8000;
		sceneObject->y = (sceneObject->y & 0xFFFF0000) | 0x8000;
	} else
		sceneObject->walkCount = 0;
}

void BbvsEngine::turnObject(SceneObject *sceneObject) {
	if (sceneObject->turnTicks > 0) {
		--sceneObject->turnTicks;
	} else {
		int turnDir = kTurnInfo[sceneObject->turnValue][sceneObject->turnCount & 0x7F];
		if (turnDir) {
			sceneObject->turnValue = (sceneObject->turnValue + turnDir) & 7;
			int turnAnimIndex = sceneObject->sceneObjectDef->animIndices[kWalkTurnTbl[sceneObject->turnValue]];
			if (turnAnimIndex) {
				Animation *anim = _gameModule->getAnimation(turnAnimIndex);
				if (anim) {
					sceneObject->anim = anim;
					sceneObject->animIndex = turnAnimIndex;
					sceneObject->turnTicks = 4;
					sceneObject->frameTicks = 1;
					sceneObject->frameIndex = anim->frameCount - 1;
				}
			}
		} else {
			sceneObject->turnCount = 0;
		}
	}
}

int BbvsEngine::rectSubtract(const Common::Rect &rect1, const Common::Rect &rect2, Common::Rect *outRects) {
	int count = 0;
	Common::Rect workRect = rect1.findIntersectingRect(rect2);
	if (!workRect.isEmpty()) {
		count = 0;
		outRects[count] = Common::Rect(rect2.width(), workRect.top - rect2.top);
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, rect2.top);
			++count;
		}
		outRects[count] = Common::Rect(workRect.left - rect2.left, workRect.height());
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, workRect.top);
			++count;
		}
		outRects[count] = Common::Rect(rect2.right - workRect.right, workRect.height());
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(workRect.right, workRect.top);
			++count;
		}
		outRects[count] = Common::Rect(rect2.width(), rect2.bottom - workRect.bottom);
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, workRect.bottom);
			++count;
		}
	} else {
		outRects[0] = rect2;
		count = 1;
	}
	return count;
}

WalkInfo *BbvsEngine::addWalkInfo(int16 x, int16 y, int delta, int direction, int16 midPtX, int16 midPtY, int walkAreaIndex) {
	WalkInfo *walkInfo = &_walkInfos[_walkInfosCount++];
	walkInfo->walkAreaIndex = walkAreaIndex;
	walkInfo->direction = direction;
	walkInfo->x = x;
	walkInfo->y = y;
	walkInfo->delta = delta;
	walkInfo->midPt.x = midPtX;
	walkInfo->midPt.y = midPtY;
	return walkInfo;
}

void BbvsEngine::initWalkAreas(SceneObject *sceneObject) {
	int16 objX = sceneObject->x >> 16;
	int16 objY = sceneObject->y >> 16;
	Common::Rect rect;
	bool doRect = false;
	Common::Rect *workWalkableRects;

	if (_buttheadObject == sceneObject && _beavisObject->anim) {
		rect = _beavisObject->anim->frameRects2[_beavisObject->frameIndex];
		rect.translate(_beavisObject->x >> 16, 1 + (_beavisObject->y >> 16));
		doRect = !rect.isEmpty();
	} else if (_buttheadObject->anim) {
		rect = _buttheadObject->anim->frameRects2[_buttheadObject->frameIndex];
		rect.translate(_buttheadObject->x >> 16, 1 + (_buttheadObject->y >> 16));
		doRect = !rect.isEmpty();
	}

	workWalkableRects = _walkableRects;

	_walkAreasCount = _walkableRectsCount;

	if (doRect && !rect.contains(objX, objY)) {
		_walkAreasCount = 0;
		for (int i = 0; i < _walkableRectsCount; ++i)
			_walkAreasCount += rectSubtract(rect, _walkableRects[i], &_tempWalkableRects1[_walkAreasCount]);
		workWalkableRects = _tempWalkableRects1;
	}

	for (int i = 0; i < _walkAreasCount; ++i) {
		_walkAreas[i].x = workWalkableRects[i].left;
		_walkAreas[i].y = workWalkableRects[i].top;
		_walkAreas[i].width = workWalkableRects[i].width();
		_walkAreas[i].height = workWalkableRects[i].height();
		_walkAreas[i].checked = false;
		_walkAreas[i].linksCount = 0;
	}

	_walkInfosCount = 0;

	// Find connections between the walkRects

	for (int i = 0; i < _walkAreasCount; ++i) {
		WalkArea *walkArea1 = &_walkAreas[i];
		int xIter = walkArea1->x + walkArea1->width;
		int yIter = walkArea1->y + walkArea1->height;

		for (int j = 0; j < _walkAreasCount; ++j) {
			WalkArea *walkArea2 = &_walkAreas[j];

			if (i == j)
				continue;

			if (walkArea2->y == yIter) {
				int wa1x = MAX(walkArea1->x, walkArea2->x);
				int wa2x = MIN(walkArea2->x + walkArea2->width, xIter);
				if (wa2x > wa1x) {
					debug(5, "WalkArea %d connected to %d by Y", i, j);
					WalkInfo *walkInfo1 = addWalkInfo(wa1x, yIter - 1, wa2x - wa1x, 0, wa1x + (wa2x - wa1x) / 2, yIter - 1, i);
					WalkInfo *walkInfo2 = addWalkInfo(wa1x, yIter, wa2x - wa1x, 0, wa1x + (wa2x - wa1x) / 2, yIter, j);
					walkArea1->linksD1[walkArea1->linksCount] = walkInfo1;
					walkArea1->linksD2[walkArea1->linksCount] = walkInfo2;
					walkArea1->links[walkArea1->linksCount++] = walkArea2;
					walkArea2->linksD1[walkArea2->linksCount] = walkInfo2;
					walkArea2->linksD2[walkArea2->linksCount] = walkInfo1;
					walkArea2->links[walkArea2->linksCount++] = walkArea1;
				}
			}

			if (walkArea2->x == xIter) {
				int wa1y = MAX(walkArea1->y, walkArea2->y);
				int wa2y = MIN(walkArea2->y + walkArea2->height, yIter);
				if (wa2y > wa1y) {
					debug(5, "WalkArea %d connected to %d by X", i, j);
					WalkInfo *walkInfo1 = addWalkInfo(xIter - 1, wa1y, wa2y - wa1y, 1, xIter - 1, wa1y + (wa2y - wa1y) / 2, i);
					WalkInfo *walkInfo2 = addWalkInfo(xIter, wa1y, wa2y - wa1y, 1, xIter, wa1y + (wa2y - wa1y) / 2, j);
					walkArea1->linksD1[walkArea1->linksCount] = walkInfo1;
					walkArea1->linksD2[walkArea1->linksCount] = walkInfo2;
					walkArea1->links[walkArea1->linksCount++] = walkArea2;
					walkArea2->linksD1[walkArea2->linksCount] = walkInfo2;
					walkArea2->linksD2[walkArea2->linksCount] = walkInfo1;
					walkArea2->links[walkArea2->linksCount++] = walkArea1;
				}
			}

		}

	}

}

WalkArea *BbvsEngine::getWalkAreaAtPos(const Common::Point &pt) {
	for (int i = 0; i < _walkAreasCount; ++i) {
		WalkArea *walkArea = &_walkAreas[i];
		if (walkArea->contains(pt))
			return walkArea;
	}
	return 0;
}

bool BbvsEngine::canButtheadWalkToDest(const Common::Point &destPt) {
	Common::Point srcPt;

	_walkReachedDestArea = false;
	initWalkAreas(_buttheadObject);
	srcPt.x = _buttheadObject->x >> 16;
	srcPt.y = _buttheadObject->y >> 16;
	_sourceWalkArea = getWalkAreaAtPos(srcPt);
	if (_sourceWalkArea) {
		_destWalkArea = getWalkAreaAtPos(destPt);
		if (_destWalkArea)
			canWalkToDest(_sourceWalkArea, 0);
	}
	return _walkReachedDestArea;
}

void BbvsEngine::canWalkToDest(WalkArea *walkArea, int infoCount) {
	
	if (_destWalkArea == walkArea) {
		_walkReachedDestArea = true;
		return;
	}
	
	if (_gameModule->getFieldC() <= 320 || infoCount <= 20) {
		walkArea->checked = true;
		for (int linkIndex = 0; linkIndex < walkArea->linksCount; ++linkIndex) {
			if (!walkArea->links[linkIndex]->checked) {
				canWalkToDest(walkArea->links[linkIndex], infoCount + 2);
				if (_walkReachedDestArea)
					break;
			}
		}
		walkArea->checked = false;
	}

}

bool BbvsEngine::walkTestLineWalkable(const Common::Point &sourcePt, const Common::Point &destPt, WalkInfo *walkInfo) {
	const float ptDeltaX = destPt.x - sourcePt.x;
	const float ptDeltaY = destPt.y - sourcePt.y;
	const float wDeltaX = walkInfo->x - sourcePt.x;
	const float wDeltaY = walkInfo->y - sourcePt.y;
	if (destPt.x == sourcePt.x)
		return true;
	if (walkInfo->direction) {
		const float nDeltaY = wDeltaX * ptDeltaY / ptDeltaX + (float)sourcePt.y - (float)walkInfo->y;
		return (nDeltaY >= 0.0) && (nDeltaY < (float)walkInfo->delta);
	} else {
		const float nDeltaX = wDeltaY / ptDeltaX * ptDeltaY + (float)sourcePt.x - (float)walkInfo->x;
		return (nDeltaX >= 0.0) && (nDeltaX < (float)walkInfo->delta);
	}
	return false;
}

void BbvsEngine::walkFindPath(WalkArea *sourceWalkArea, int infoCount) {
	if (_destWalkArea == sourceWalkArea) {
		walkFoundPath(infoCount);
	} else if (_gameModule->getFieldC() <= 320 || infoCount <= 20) {
		sourceWalkArea->checked = true;
		for (int linkIndex = 0; linkIndex < sourceWalkArea->linksCount; ++linkIndex) {
			if (!sourceWalkArea->links[linkIndex]->checked) {
				_walkInfoPtrs[infoCount + 0] = sourceWalkArea->linksD1[linkIndex];
				_walkInfoPtrs[infoCount + 1] = sourceWalkArea->linksD2[linkIndex];
				walkFindPath(sourceWalkArea->links[linkIndex], infoCount + 2);
			}
		}
		sourceWalkArea->checked = false;
	}
}

int BbvsEngine::calcDistance(const Common::Point &pt1, const Common::Point &pt2) {
	return (int)sqrt((double)(pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
}

void BbvsEngine::walkFoundPath(int count) {
	debug(5, "BbvsEngine::walkFoundPath(%d)", count);
	
	Common::Point midPt = _sourceWalkAreaPt;
	int totalMidPtDistance = 0;
	
	if (count > 0) {
		Common::Point lastMidPt;
		int halfCount = (count + 1) >> 1;
		for (int i = 0; i < halfCount; ++i) {
			lastMidPt = midPt;
			midPt = _walkInfoPtrs[i * 2]->midPt;
			totalMidPtDistance += calcDistance(midPt, lastMidPt);
		}
	}

	int distance = calcDistance(midPt, _destWalkAreaPt) + totalMidPtDistance;

	debug(5, "BbvsEngine::walkFoundPath() distance: %d; _currWalkDistance: %d", distance, _currWalkDistance);

	if (distance >= _currWalkDistance)
		return;
		
	debug(5, "BbvsEngine::walkFoundPath() distance smaller");

	_currWalkDistance = distance;

	Common::Point destPt = _destWalkAreaPt, newDestPt;
	
	while (1) {

		int index = 0;
		if (count > 0) {
			do {
				if (!walkTestLineWalkable(_sourceWalkAreaPt, destPt, _walkInfoPtrs[index]))
					break;
				++index;
			} while (index < count);
		}

		if (index == count)
			break;

		WalkInfo *walkInfo = _walkInfoPtrs[--count];
		destPt.x = walkInfo->x;
		destPt.y = walkInfo->y;
		
		if (walkInfo->direction) {
			newDestPt.x = walkInfo->x;
			newDestPt.y = walkInfo->y + walkInfo->delta - 1;
		} else {
			newDestPt.x = walkInfo->x + walkInfo->delta - 1;
			newDestPt.y = walkInfo->y;
		}

		if ((newDestPt.x - _destWalkAreaPt.x) * (newDestPt.x - _destWalkAreaPt.x) +
			(newDestPt.y - _destWalkAreaPt.y) * (newDestPt.y - _destWalkAreaPt.y) <
			(destPt.x - _destWalkAreaPt.x) * (destPt.x - _destWalkAreaPt.x) +
			(destPt.y - _destWalkAreaPt.y) * (destPt.y - _destWalkAreaPt.y))
			destPt = newDestPt;

	}

	debug(5, "BbvsEngine::walkFoundPath() destPt: (%d, %d)", destPt.x, destPt.y);

	_finalWalkPt = destPt;

	debug(5, "BbvsEngine::walkFoundPath() OK");

}

void BbvsEngine::updateWalkableRects() {
	// Go through all walkable rects and subtract all scene object rects
	Common::Rect *rectsList1 = _tempWalkableRects1;
	Common::Rect *rectsList2 = _gameModule->getWalkRects();
	_walkableRectsCount = _gameModule->getWalkRectsCount();
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *sceneObject = &_sceneObjects[i];
		Animation *anim = sceneObject->anim;
		if (anim && _buttheadObject != sceneObject && _beavisObject != sceneObject) {
			Common::Rect rect = sceneObject->anim->frameRects2[sceneObject->frameIndex];
			rect.translate(sceneObject->x >> 16, sceneObject->y >> 16);
			int count = _walkableRectsCount;
			_walkableRectsCount = 0;
			for (int j = 0; j < count; ++j)
				_walkableRectsCount += rectSubtract(rect, rectsList2[j], &rectsList1[_walkableRectsCount]);
			if (rectsList1 == _tempWalkableRects1) {
				rectsList1 = _tempWalkableRects2;
				rectsList2 = _tempWalkableRects1;
			} else {
				rectsList1 = _tempWalkableRects1;
				rectsList2 = _tempWalkableRects2;
			}
		}
	}
	for (int i = 0; i < _walkableRectsCount; ++i)
		_walkableRects[i] = rectsList2[i];
}

void BbvsEngine::updateSceneObjectsTurnValue() {
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *sceneObject = &_sceneObjects[i];
		sceneObject->turnValue = 0;
		for (int j = 0; j < 12; ++j) {
			if (sceneObject->sceneObjectDef->animIndices[j] == sceneObject->animIndex) {
				sceneObject->turnValue = kTurnTbl[j];
				break;
			}
		}
	}
}

void BbvsEngine::updateDialogConditions() {
	_dialogSlotCount = 0;
	memset(_dialogItemStatus, 0, sizeof(_dialogItemStatus));
	for (int i = 0; i < _gameModule->getActionsCount(); ++i) {
		Action *action = _gameModule->getAction(i);
		int slotIndex = evalDialogCondition(action->conditions);
		if (slotIndex >= 0) {
			_dialogItemStatus[slotIndex] = 1;
			++_dialogSlotCount;
		}
	}
}

void BbvsEngine::playSpeech(int soundNum) {
	debug(5, "playSpeech(%0d)", soundNum);
	Common::String sndFilename = Common::String::format("snd/snd%05d.aif", soundNum);
	Common::File *fd = new Common::File();
	fd->open(sndFilename);
	Audio::AudioStream *audioStream = Audio::makeAIFFStream(fd, DisposeAfterUse::YES);
	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechSoundHandle, audioStream);

}

void BbvsEngine::stopSpeech() {
	_mixer->stopHandle(_speechSoundHandle);
}

void BbvsEngine::playSound(uint soundNum, bool loop) {
	debug(5, "playSound(%0d)", soundNum);
	for (uint i = 0; i < _gameModule->getPreloadSoundsCount(); ++i)
		if (_gameModule->getPreloadSound(i) == soundNum) {
			_sound->playSound(i, loop);
			break;
		}
}

void BbvsEngine::stopSound(uint soundNum) {
	for (uint i = 0; i < _gameModule->getPreloadSoundsCount(); ++i)
		if (_gameModule->getPreloadSound(i) == soundNum) {
			_sound->stopSound(i);
			break;
		}
}

void BbvsEngine::stopSounds() {
	_sound->stopAllSounds();
}

bool BbvsEngine::runMinigame(int minigameNum) {
	debug(0, "BbvsEngine::runMinigame() minigameNum: %d", minigameNum);
	
	bool fromMainGame = _currSceneNum != kMainMenu;
	
	_sound->unloadSounds();
		
	Minigame *minigame = 0;
	
	switch (minigameNum) {
	case kMinigameBbLoogie:
		minigame = new MinigameBbLoogie(this);
		break;
	case kMinigameBbTennis:
		minigame = new MinigameBbTennis(this);
		break;
	case kMinigameBbAnt:
		minigame = new MinigameBbAnt(this);
		break;
	case kMinigameBbAirGuitar:
		minigame = new MinigameBbAirGuitar(this);
		break;
	default:
		error("Incorrect minigame number %d", minigameNum);
		break;
	}
	
	int minigameResult = minigame->run(fromMainGame);
	
	delete minigame;

	// Check if the prinicpal was hit with a megaloogie in the loogie minigame
	if (minigameNum == 0 && minigameResult == 1)
		_gameVars[42] = 1;

#if 0
	//DEBUG Fake it :)
	if (minigameNum == 0)
		_gameVars[42] = 1;
#endif

	return true;
}

void BbvsEngine::runMainMenu() {
	MainMenu *mainMenu = new MainMenu(this);
	mainMenu->runModal();
	delete mainMenu;
}

void BbvsEngine::checkEasterEgg(char key) {

	static const char * const kEasterEggStrings[] = {
		"BOIDUTS",
		"YNNIF",
		"SKCUS",
		"NAMTAH"
	};
	
	static const int kEasterEggLengths[] = {
		7, 5, 5, 6
	};
	
	if (_currSceneNum == kCredits) {
		memcpy(&_easterEggInput[1], &_easterEggInput[0], 6);
		_easterEggInput[0] = key;
		for (int i = 0; i < ARRAYSIZE(kEasterEggStrings); ++i) {
			if (!scumm_strnicmp(kEasterEggStrings[i], _easterEggInput, kEasterEggLengths[i])) {
				_easterEggInput[0] = 0;
				_newSceneNum = 100 + i;
				break;
			}
		}
	}

}

} // End of namespace Bbvs
