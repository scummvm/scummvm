/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#include "backends/keymapper/action.h"
#include "backends/keymapper/keymap.h"
#include "backends/keymapper/standard-actions.h"
#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/random.h"

#include "freescape/freescape.h"
#include "freescape/games/driller/driller.h"
#include "freescape/language/8bitDetokeniser.h"

namespace Freescape {

enum {
	kDrillerNoRig = 0,
	kDrillerRigInPlace = 1,
	kDrillerRigOutOfPlace = 2,
	kDrillerRigNoGas = 3
};

DrillerEngine::DrillerEngine(OSystem *syst, const ADGameDescription *gd) : FreescapeEngine(syst, gd) {

	if (!Common::parseBool(ConfMan.get("automatic_drilling"), _useAutomaticDrilling))
		error("Failed to parse bool from automatic_drilling option");

	if (isDOS())
		initDOS();
	else if (isAmiga() || isAtariST())
		initAmigaAtari();
	else if (isSpectrum())
		initZX();
	else if (isCPC())
		initCPC();
	else if (isC64())
		initC64();

	_playerHeightNumber = 1;
	_playerHeightMaxNumber = 3;

	_angleRotations.push_back(5);
	_angleRotations.push_back(10);
	_angleRotations.push_back(15);
	_angleRotations.push_back(30);
	_angleRotations.push_back(45);
	_angleRotations.push_back(90);

	_playerWidth = 12;
	_playerDepth = 32;
	_stepUpDistance = 64;

	_initialTankEnergy = 48;
	_initialTankShield = 50;
	_initialJetEnergy = 29;
	_initialJetShield = 34;

	_maxEnergy = 63;
	_maxShield = 63;

	Math::Vector3d drillBaseOrigin = Math::Vector3d(0, 0, 0);
	Math::Vector3d drillBaseSize = Math::Vector3d(3, 2, 3);
	_drillBase = new GeometricObject(kCubeType, 0, 0, drillBaseOrigin, drillBaseSize, nullptr, nullptr, nullptr, FCLInstructionVector(), "");
	assert(!_drillBase->isDestroyed() && !_drillBase->isInvisible());

	if (isDemo()) {
		_demoMode = !_disableDemoMode; // Most of the driller demos are non-interactive
		_angleRotationIndex = 0;
	}

	_endArea = 127;
	_endEntrance = 0;

	_soundIndexShoot = 1;
	_soundIndexCollide = -1;
	_soundIndexFall = 3;
	_soundIndexClimb = -1;
	_soundIndexMenu = -1;
	_soundIndexStart = 9;
	_soundIndexAreaChange = 5;

	_soundIndexNoShield = 20;
	_soundIndexNoEnergy = 20;
	_soundIndexFallen = 20;
	_soundIndexTimeout = 20;
	_soundIndexForceEndGame = 20;
	_soundIndexCrushed = 20;
}

DrillerEngine::~DrillerEngine() {
	delete _drillBase;
}

void DrillerEngine::initKeymaps(Common::Keymap *engineKeyMap, Common::Keymap *infoScreenKeyMap, const char *target) {
	FreescapeEngine::initKeymaps(engineKeyMap, infoScreenKeyMap, target);
	Common::Action *act;

	if (!(isAmiga() || isAtariST())) {
		act = new Common::Action("SAVE", _("Save Game"));
		act->setCustomEngineActionEvent(kActionSave);
		act->addDefaultInputMapping("s");
		infoScreenKeyMap->addAction(act);

		act = new Common::Action("LOAD", _("Load Game"));
		act->setCustomEngineActionEvent(kActionLoad);
		act->addDefaultInputMapping("l");
		infoScreenKeyMap->addAction(act);

		act = new Common::Action("QUIT", _("Quit Game"));
		act->setCustomEngineActionEvent(kActionEscape);
		if (isSpectrum())
			act->addDefaultInputMapping("1");
		else
			act->addDefaultInputMapping("ESCAPE");
		infoScreenKeyMap->addAction(act);

		act = new Common::Action("TOGGLESOUND", _("Toggle Sound"));
		act->setCustomEngineActionEvent(kActionToggleSound);
		act->addDefaultInputMapping("t");
		infoScreenKeyMap->addAction(act);
	}

	act = new Common::Action("ROTL", _("Rotate Left"));
	act->setCustomEngineActionEvent(kActionRotateLeft);
	act->addDefaultInputMapping("q");
	engineKeyMap->addAction(act);

	act = new Common::Action("ROTR", _("Rotate Right"));
	act->setCustomEngineActionEvent(kActionRotateRight);
	act->addDefaultInputMapping("w");
	engineKeyMap->addAction(act);

	// I18N: STEP SIZE: Measures the size of one movement in the direction you are facing (1-250 standard distance units (SDUs))
	act = new Common::Action("INCSTEPSIZE", _("Increase Step Size"));
	act->setCustomEngineActionEvent(kActionIncreaseStepSize);
	act->addDefaultInputMapping("s");
	engineKeyMap->addAction(act);

	// I18N: STEP SIZE: Measures the size of one movement in the direction you are facing (1-250 standard distance units (SDUs))
	act = new Common::Action("DECSTEPSIZE", _("Decrease Step Size"));
	act->setCustomEngineActionEvent(kActionDecreaseStepSize);
	act->addDefaultInputMapping("x");
	engineKeyMap->addAction(act);

	act = new Common::Action("RISE", _("Rise/Fly up"));
	act->setCustomEngineActionEvent(kActionRiseOrFlyUp);
	act->addDefaultInputMapping("JOY_B");
	act->addDefaultInputMapping("r");
	engineKeyMap->addAction(act);

	act = new Common::Action("LOWER", _("Lower/Fly down"));
	act->setCustomEngineActionEvent(kActionLowerOrFlyDown);
	act->addDefaultInputMapping("JOY_Y");
	act->addDefaultInputMapping("f");
	engineKeyMap->addAction(act);

	// I18N: drilling rig is an in game item
	act = new Common::Action("DEPLOY", _("Deploy drilling rig"));
	act->setCustomEngineActionEvent(kActionDeployDrillingRig);
	act->addDefaultInputMapping("JOY_LEFT_SHOULDER");
	act->addDefaultInputMapping("d");
	engineKeyMap->addAction(act);

	// I18N: drilling rig is an in game item
	act = new Common::Action("COLLECT", _("Collect drilling rig"));
	act->setCustomEngineActionEvent(kActionCollectDrillingRig);
	act->addDefaultInputMapping("c");
	act->addDefaultInputMapping("JOY_RIGHT_SHOULDER");
	engineKeyMap->addAction(act);
}

void DrillerEngine::gotoArea(uint16 areaID, int entranceID) {
	int prevAreaID = _currentArea ? _currentArea->getAreaID(): -1;
	debugC(1, kFreescapeDebugMove, "Jumping to area: %d, entrance: %d", areaID, entranceID);

	if (!_areaMap.contains(areaID)) {
		assert(isDOS() && isDemo());
		// Not included in the demo, abort area change
		return;
	}
	_currentArea = _areaMap[areaID];
	_currentArea->show();

	if (entranceID > 0 || areaID == 127) {
		traverseEntrance(entranceID);
	} else if (entranceID == 0) {
		int newPos = -1;
		// FIX: The next check will abort changing the current another
		// area if the player is too close to the corners
		if ((_position.z() < 100 && _position.x() > 3900) ||
			(_position.z() > 3900 && _position.x() < 100) ||
			(_position.z() < 100 && _position.x()  < 100) ||
			(_position.z() > 3900 && _position.x() > 3900)) {
				assert(prevAreaID > 0);
				_currentArea = _areaMap[prevAreaID];
				return;
		}
		if (_position.z() < 200 || _position.z() >= 3800) {
			if (_position.z() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(2, newPos);
		} else if(_position.x() < 200 || _position.x() >= 3800)  {
			if (_position.x() < 200)
				newPos = 4000;
			else
				newPos = 100;
			_position.setValue(0, newPos);
		} else
			error("Invalid movement across areas");
		assert(newPos != -1);
		_sensors = _currentArea->getSensors();
	} else if (entranceID == -1)
		debugC(1, kFreescapeDebugMove, "Loading game, no change in position");
	else
		error("Invalid area change!");

	_lastPosition = _position;
	_gameStateVars[0x1f] = 0;

	if (areaID == _startArea && entranceID == _startEntrance) {
		playSound(_soundIndexStart, true);
	} else if (areaID == 127) {
		assert(entranceID == 0);
		_pitch = 335;
		_flyMode = true; // Avoid falling
		// Show the number of completed areas
		_areaMap[127]->_name.replace(0, 3, Common::String::format("%4d", _gameStateVars[32]));
	} else
		playSound(_soundIndexAreaChange, false);

	debugC(1, kFreescapeDebugMove, "starting player position: %f, %f, %f", _position.x(), _position.y(), _position.z());
	clearTemporalMessages();
	// Ignore sky/ground fields
	_gfx->_keyColor = 0;
	_gfx->setColorRemaps(&_currentArea->_colorRemaps);

	swapPalette(areaID);

	if (isDOS() || isAmiga() || isAtariST()) {
		_currentArea->_skyColor = 0;
		_currentArea->_usualBackgroundColor = 0;
	} else if (isCPC()) {
		_currentArea->_usualBackgroundColor = 1;
		_currentArea->_skyColor = 1;
	}

	resetInput();
}

void DrillerEngine::loadAssets() {
	FreescapeEngine::loadAssets();
	if (_areaMap.contains(18)) {
		/*
		We are going to inject a small script in the
		last area to force the game to end:
		IF COLLIDED? THEN
		IF VAR!=? (v32, 18) THEN END ENDIF
		GOTO (127, 0)
		*/

		FCLInstructionVector instructions;
		Common::Array<uint16> conditionArray;

		conditionArray.push_back(0xb);
		conditionArray.push_back(0x20);
		conditionArray.push_back(0x12);
		conditionArray.push_back(0x12);
		conditionArray.push_back(0x7f);
		conditionArray.push_back(0x0);

		Common::String conditionSource = detokenise8bitCondition(conditionArray, instructions, false);
		debugC(1, kFreescapeDebugParser, "%s", conditionSource.c_str());
		_areaMap[18]->_conditions.push_back(instructions);
		_areaMap[18]->_conditionSources.push_back(conditionSource);
	}

	_timeoutMessage = _messagesList[14];
	_noShieldMessage = _messagesList[15];
	_noEnergyMessage = _messagesList[16];
	_fallenMessage = _messagesList[17];
	_forceEndGameMessage = _messagesList[18];
	// Small extra feature: allow player to be crushed in Driller
	_crushedMessage = "CRUSHED!";
}

void DrillerEngine::drawInfoMenu() {
	PauseToken pauseToken = pauseEngine();
	if (_savedScreen) {
		_savedScreen->free();
		delete _savedScreen;
	}
	_savedScreen = _gfx->getScreenshot();

	uint32 color = _gfx->_texturePixelFormat.ARGBToColor(0x00, 0x00, 0x00, 0x00);
	Graphics::Surface *surface = new Graphics::Surface();
	surface->create(_screenW, _screenH, _gfx->_texturePixelFormat);
	surface->fillRect(_fullscreenViewArea, color);

	uint32 black = _gfx->_texturePixelFormat.ARGBToColor(0xFF, 0x00, 0x00, 0x00);
	surface->fillRect(_viewArea, black);

	switch (_renderMode) {
		case Common::kRenderCGA:
		case Common::kRenderHercG:
			color = 1;
			break;
		case Common::kRenderZX:
			color = 6;
			break;
		case Common::kRenderCPC:
			color = _gfx->_underFireBackgroundColor;
			break;
		default:
			color = 14;
	}
	uint8 r, g, b;

	_gfx->readFromPalette(color, r, g, b);
	if (isAmiga() || isAtariST()) {
		r = 0xFF;
		g = 0xFF;
		b = 0x55;
	}

	uint32 front = _gfx->_texturePixelFormat.ARGBToColor(0xFF, r, g, b);

	Common::String areaName = _currentArea->_name;
	areaName.trim();

	drawStringInSurface(Common::String::format("%10s : %s", "sector", areaName.c_str()), 59, 25, front, black, surface);
	Common::String rigStatus;
	Common::String gasFound;
	Common::String perTapped;
	Common::String gasTapped;

	switch (_drillStatusByArea[_currentArea->getAreaID()]) {
		case kDrillerNoRig:
			rigStatus = _currentArea->_gasPocketRadius > 0 ? "Unpositioned" : "Not required";
			gasFound = "-";
			perTapped = "-";
			gasTapped = "-";
			break;
		case kDrillerRigInPlace:
		case kDrillerRigOutOfPlace:
			rigStatus = "Positioned";
			gasFound = Common::String::format("%d CFT", _drillMaxScoreByArea[_currentArea->getAreaID()]);
			perTapped = Common::String::format("%d %%", _drillSuccessByArea[_currentArea->getAreaID()]);
			gasTapped = Common::String::format("%d", uint32(_drillSuccessByArea[_currentArea->getAreaID()] * _drillMaxScoreByArea[_currentArea->getAreaID()]) / 100);
			break;
		case kDrillerRigNoGas:
			rigStatus = "Positioned";
			gasFound = "none";
			perTapped = "none";
			gasTapped = "zero";
			break;
		default:
			error("Invalid drill status");
			break;
	}

	drawStringInSurface(Common::String::format("%10s : %s", "rig status", rigStatus.c_str()), 59, 33, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas found", gasFound.c_str()), 59, 41, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "% tapped", perTapped.c_str()), 59, 49, front, black, surface);
	drawStringInSurface(Common::String::format("%10s : %s", "gas tapped", gasTapped.c_str()), 59, 57, front, black, surface);

	drawStringInSurface(Common::String::format("%13s : %d", "total sectors", 18), 84, 73, front, black, surface);
	drawStringInSurface(Common::String::format("%13s : %d", "safe sectors", _gameStateVars[32]), 84, 81, front, black, surface);

	if (isDOS() || isCPC()) {
		drawStringInSurface("l-load s-save esc-terminate", 53, 97, front, black, surface);
		drawStringInSurface("t-toggle sound on/off", 76, 105, front, black, surface);
	} else if (isSpectrum()) {
		drawStringInSurface("l-load s-save 1-abort", 76, 97, front, black, surface);
		drawStringInSurface("any other key-continue", 76, 105, front, black, surface);
	} else if (isAmiga() || isAtariST())
		drawStringInSurface("press any key to continue", 66, 97, front, black, surface);

	Texture *menuTexture = _gfx->createTexture(surface);
	Common::Event event;
	bool cont = true;
	while (!shouldQuit() && cont) {
		while (_eventManager->pollEvent(event)) {

			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionLoad) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					loadGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (event.customType == kActionSave) {
					_gfx->setViewport(_fullscreenViewArea);
					_eventManager->purgeKeyboardEvents();
					saveGameDialog();
					_gfx->setViewport(_viewArea);
				} else if (isDOS() && event.customType == kActionToggleSound) {
					// TODO
				} else if ((isDOS() || isCPC() || isSpectrum()) && event.customType == kActionEscape) {
					_forceEndGame = true;
					cont = false;
				} else
					cont = false;
				break;
			case Common::EVENT_KEYDOWN:
					cont = false;
				break;
			case Common::EVENT_SCREEN_CHANGED:
				_gfx->computeScreenViewport();
				// TODO: properly refresh screen
				break;
			case Common::EVENT_RBUTTONDOWN:
			// fallthrough
			case Common::EVENT_LBUTTONDOWN:
				if (g_system->hasFeature(OSystem::kFeatureTouchscreen))
					cont = false;
				break;
			default:
				break;
			}
		}
		_gfx->clear(0, 0, 0, true);
		drawFrame();
		if (surface)
			_gfx->drawTexturedRect2D(_fullscreenViewArea, _fullscreenViewArea, menuTexture);

		_gfx->flipBuffer();
		g_system->updateScreen();
		g_system->delayMillis(15); // try to target ~60 FPS
	}

	_savedScreen->free();
	delete _savedScreen;
	_savedScreen = nullptr;
	surface->free();
	delete surface;
	delete menuTexture;
	pauseToken.clear();
}

Math::Vector3d getProjectionToPlane(const Math::Vector3d &vect, const Math::Vector3d normal) {
	assert (normal.length() == 1);
	// Formula: return p - n * (n . p)
	Math::Vector3d result = vect;
	result -= normal * normal.dotProduct(vect);
	return result;
}

void DrillerEngine::pressedKey(const int keycode) {
	if (keycode == kActionRotateLeft) {
		rotate(-_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == kActionRotateRight) {
		rotate(_angleRotations[_angleRotationIndex], 0);
	} else if (keycode == kActionIncreaseStepSize) {
		increaseStepSize();
	} else if (keycode == kActionDecreaseStepSize) {
		decreaseStepSize();
	} else if (keycode == kActionRiseOrFlyUp) {
		rise();
	} else if (keycode == kActionLowerOrFlyDown) {
		lower();
	} else if (keycode == kActionDeployDrillingRig) {
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		clearTemporalMessages();
		Common::Point gasPocket = _currentArea->_gasPocketPosition;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[12], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		Math::Vector3d drill = drillPosition();
		debugC(1, kFreescapeDebugMove, "Current position at %f %f %f", _position.x(), _position.y(), _position.z());
		debugC(1, kFreescapeDebugMove, "Trying to adding drill at %f %f %f", drill.x(), drill.y(), drill.z());
		debugC(1, kFreescapeDebugMove, "with pitch: %f and yaw %f", _pitch, _yaw);

		if (!checkDrill(drill)) {
			insertTemporaryMessage(_messagesList[4], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;
		const Math::Vector3d gasPocket3D(gasPocket.x, drill.y(), gasPocket.y);
		float distanceToPocket = (gasPocket3D - drill).length();
		float success = _useAutomaticDrilling ? 100.0 : 100.0 * (1.0 - distanceToPocket / _currentArea->_gasPocketRadius);
		insertTemporaryMessage(_messagesList[3], _countdown - 2);
		addDrill(drill, success > 0);
		if (success <= 0) {
			insertTemporaryMessage(_messagesList[9], _countdown - 4);
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigNoGas;
			return;
		}
		Common::String maxScoreMessage = _messagesList[5];
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		maxScoreMessage.replace(2, 6, Common::String::format("%d", maxScore));
		insertTemporaryMessage(maxScoreMessage, _countdown - 4);
		Common::String successMessage = _messagesList[6];
		successMessage.replace(0, 4, Common::String::format("%d", int(success)));
		while (successMessage.size() < 14)
			successMessage += " ";
		insertTemporaryMessage(successMessage, _countdown - 6);
		_drillSuccessByArea[_currentArea->getAreaID()] = uint32(success);
		_gameStateVars[k8bitVariableScore] += uint32(maxScore * uint32(success)) / 100;

		if (success >= 50.0) {
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigInPlace;
			_gameStateVars[32]++;
		} else
			_drillStatusByArea[_currentArea->getAreaID()] = kDrillerRigOutOfPlace;
		executeMovementConditions();
	} else if (keycode == kActionCollectDrillingRig) {
		if (isDOS() && isDemo()) // No support for drilling here yet
			return;
		uint32 gasPocketRadius = _currentArea->_gasPocketRadius;
		clearTemporalMessages();
		if (gasPocketRadius == 0) {
			insertTemporaryMessage(_messagesList[2], _countdown - 2);
			return;
		}

		if (_flyMode) {
			insertTemporaryMessage(_messagesList[8], _countdown - 2);
			return;
		}

		if (!drillDeployed(_currentArea)) {
			insertTemporaryMessage(_messagesList[13], _countdown - 2);
			return;
		}

		if (_gameStateVars[k8bitVariableEnergy] < 5) {
			insertTemporaryMessage(_messagesList[7], _countdown - 2);
			return;
		}

		_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 5;

		uint16 areaID = _currentArea->getAreaID();
		if (_drillStatusByArea[areaID] > 0) {
			if (_drillStatusByArea[areaID] == kDrillerRigInPlace)
				_gameStateVars[32]--;
			_drillStatusByArea[areaID] = kDrillerNoRig;
		}
		removeDrill(_currentArea);
		insertTemporaryMessage(_messagesList[10], _countdown - 2);
		int maxScore = _drillMaxScoreByArea[_currentArea->getAreaID()];
		uint32 success = _drillSuccessByArea[_currentArea->getAreaID()];
		uint32 scoreToRemove = uint32(maxScore * success) / 100;
		assert(scoreToRemove <= uint32(_gameStateVars[k8bitVariableScore]));
		_gameStateVars[k8bitVariableScore] -= scoreToRemove;
		executeMovementConditions();
	}
}

Math::Vector3d DrillerEngine::drillPosition() {
	Math::Vector3d position = _position;
	position.setValue(1, position.y() - _playerHeight);
	position = position + 300 * getProjectionToPlane(_cameraFront, Math::Vector3d(0, 1, 0));

	Object *obj = (GeometricObject *)_areaMap[255]->objectWithID(255); // Drill base
	assert(obj);
	position.setValue(0, position.x() - 128);
	position.setValue(2, position.z() - 128);
	return position;
}

bool DrillerEngine::drillDeployed(Area *area) {
	return (area->objectWithID(255) != nullptr);
}

bool DrillerEngine::checkDrill(const Math::Vector3d position) {
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	origin.setValue(0, origin.x() + 128);
	origin.setValue(1, origin.y() - 5);
	origin.setValue(2, origin.z() + 128);

	_drillBase->setOrigin(origin);
	if (_currentArea->checkCollisions(_drillBase->_boundingBox).empty())
		return false;

	origin.setValue(0, origin.x() - 128);
	origin.setValue(2, origin.z() - 128);

	id = 255;
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	origin.setValue(1, origin.y() + 6);
	obj->setOrigin(origin);

	// This bounding box is too large and can result in the drill to float next to a wall
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() + 15);
	obj->setOrigin(origin);

	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	origin.setValue(1, origin.y() - 10);
	heightLastObject = obj->getSize().y();
	delete obj;

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);
	delete obj;

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	if (!_currentArea->checkCollisions(obj->_boundingBox).empty())
		return false;

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);
	delete obj;
	return true;
}


void DrillerEngine::addSkanner(Area *area) {
	debugC(1, kFreescapeDebugParser, "Adding skanner to area: %d", area->getAreaID());
	GeometricObject *obj = nullptr;
	int16 id;

	id = 248;
	// If first object is already added, do not re-add any
	if (area->objectWithID(id) != nullptr)
		return;

	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 249;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);

	id = 250;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->makeInvisible();
	area->addObject(obj);
}

void DrillerEngine::addDrill(const Math::Vector3d position, bool gasFound) {
	// int drillObjectIDs[8] = {255, 254, 253, 252, 251, 250, 248, 247};
	GeometricObject *obj = nullptr;
	Math::Vector3d origin = position;

	int16 id;
	int heightLastObject;

	id = 255;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	// offset.setValue(1, offset.y() + obj->getSize().y());
	obj->makeVisible();
	_currentArea->addObject(obj);

	heightLastObject = obj->getSize().y();

	id = 254;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	// Set position for object
	origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	obj = (GeometricObject *)obj->duplicate();
	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	id = 253;
	debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
	obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
	assert(obj);
	obj = (GeometricObject *)obj->duplicate();

	origin.setValue(0, origin.x() + obj->getSize().x() / 5);
	origin.setValue(1, origin.y() + heightLastObject);
	origin.setValue(2, origin.z() + obj->getSize().z() / 5);

	obj->setOrigin(origin);
	obj->makeVisible();
	_currentArea->addObject(obj);

	// Undo offset
	// origin.setValue(0, origin.x() - obj->getSize().x() / 5);
	heightLastObject = obj->getSize().y();
	// origin.setValue(2, origin.z() - obj->getSize().z() / 5);

	if (gasFound) {
		id = 252;
		debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
		obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
		assert(obj);
		obj = (GeometricObject *)obj->duplicate();
		origin.setValue(0, origin.x() + obj->getSize().x());
		origin.setValue(1, origin.y() + heightLastObject);
		origin.setValue(2, origin.z() + obj->getSize().z());
		obj->setOrigin(origin);
		assert(obj);
		obj->makeVisible();
		_currentArea->addObject(obj);
		heightLastObject = obj->getSize().y();

		id = 251;
		debugC(1, kFreescapeDebugParser, "Adding object %d to room structure", id);
		obj = (GeometricObject *)_areaMap[255]->objectWithID(id);
		assert(obj);
		obj = (GeometricObject *)obj->duplicate();
		origin.setValue(1, origin.y() + heightLastObject);
		obj->setOrigin(origin);
		assert(obj);
		obj->makeVisible();
		_currentArea->addObject(obj);
	}
}

void DrillerEngine::removeDrill(Area *area) {
	for (int16 id = 251; id < 256; id++) {
		if (id > 252)
			assert(area->objectWithID(id));

		if (area->objectWithID(id))
			area->removeObject(id);
	}
}

void DrillerEngine::initGameState() {
	FreescapeEngine::initGameState();

	for (auto &it : _areaMap) {
		if (_drillStatusByArea[it._key] != kDrillerNoRig)
			removeDrill(it._value);
		_drillStatusByArea[it._key] = kDrillerNoRig;
		if (it._key != 255) {
			addSkanner(it._value);
			_drillMaxScoreByArea[it._key] = (10 + _rnd->getRandomNumber(89)) * 1000;
		}
		_drillSuccessByArea[it._key] = 0;
	}

	_gameStateVars[k8bitVariableEnergy] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShield] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerTank] = _initialTankEnergy;
	_gameStateVars[k8bitVariableShieldDrillerTank] = _initialTankShield;

	_gameStateVars[k8bitVariableEnergyDrillerJet] = _initialJetEnergy;
	_gameStateVars[k8bitVariableShieldDrillerJet] = _initialJetShield;

	_playerHeightNumber = 1;
	_demoIndex = 0;
	_demoEvents.clear();

	// Start playing music, if any, in any supported format
	playMusic("Matt Gray - The Best Of Reformation - 07 Driller Theme");
}

bool DrillerEngine::checkIfGameEnded() {
	if (isDemo() && _demoMode)
		if (_demoData[_demoIndex + 1] == 0x5f)
			return true;

	FreescapeEngine::checkIfGameEnded();
	return false;
}

void DrillerEngine::endGame() {
	FreescapeEngine::endGame();

	if (!_endGamePlayerEndArea)
		return;

	if (_gameStateVars[32] == 18) { // All areas are complete
		insertTemporaryMessage(_messagesList[19], _countdown - 2);
		_gameStateVars[32] = 0;  // Avoid repeating the message
	}

	if (_endGameKeyPressed) {
		_gameStateControl = kFreescapeGameStateRestart;
	}

	_endGameKeyPressed = false;
}

bool DrillerEngine::onScreenControls(Common::Point mouse) {
	if (_moveFowardArea.contains(mouse)) {
		move(kForwardMovement, _scaleVector.x(), 20.0);
		return true;
	} else if (_moveLeftArea.contains(mouse)) {
		move(kLeftMovement, _scaleVector.y(), 20.0);
		return true;
	} else if (_moveRightArea.contains(mouse)) {
		move(kRightMovement, _scaleVector.y(), 20.0);
		return true;
	} else if (_moveBackArea.contains(mouse)) {
		move(kBackwardMovement, _scaleVector.x(), 20.0);
		return true;
	} else if (_moveUpArea.contains(mouse)) {
		rise();
		return true;
	} else if (_moveDownArea.contains(mouse)) {
		lower();
		return true;
	} else if (_deployDrillArea.contains(mouse)) {
		pressedKey(kActionDeployDrillingRig);
		return true;
	} else if (_infoScreenArea.contains(mouse)) {
		drawInfoMenu();
		return true;
	} else if (_saveGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		saveGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	} else if (_loadGameArea.contains(mouse)) {
		_gfx->setViewport(_fullscreenViewArea);
		loadGameDialog();
		_gfx->setViewport(_viewArea);
		return true;
	}
	return false;
}

void DrillerEngine::drawSensorShoot(Sensor *sensor) {
	if (_gameStateControl == kFreescapeGameStatePlaying) {
		// Avoid playing new sounds, so the endgame can progress
		playSound(_soundIndexHit, true);
	}

	Math::Vector3d target;
	target = _position;
	target.y() = target.y() - _playerHeight;
	target.x() = target.x() - 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

	target = _position;
	target.y() = target.y() - _playerHeight;
	target.x() = target.x() + 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

	target = _position;
	target.y() = target.y() + _playerHeight;
	target.x() = target.x() - 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);

	target = _position;
	target.y() = target.y() + _playerHeight;
	target.x() = target.x() + 5;
	_gfx->renderSensorShoot(1, sensor->getOrigin(), target, _viewArea);
}

void DrillerEngine::updateTimeVariables() {
	int seconds, minutes, hours;
	getTimeFromCountdown(seconds, minutes, hours);

	if (_lastMinute != minutes) {
		_lastMinute = minutes;
		if (_gameStateVars[k8bitVariableEnergy] > 0)
			_gameStateVars[k8bitVariableEnergy] = _gameStateVars[k8bitVariableEnergy] - 1;
		_gameStateVars[0x1e] += 1;
		_gameStateVars[0x1f] += 1;
		executeLocalGlobalConditions(false, true, false); // Only execute "on collision" room/global conditions
	}
}

void DrillerEngine::drawCompass(Graphics::Surface *surface, int x, int y, double degrees, double magnitude, uint32 color) {
	double fov = 60;
	degrees = degrees + fov;
	if (degrees >= 360)
		degrees = degrees - 360;

	const double degtorad = (M_PI * 2) / 360;
	double w = magnitude * cos(-degrees * degtorad);
	double h = magnitude * sin(-degrees * degtorad);

	surface->drawLine(x, y, x+(int)w, y+(int)h, color);


	degrees = degrees - fov;
	if (degrees < 0)
		degrees = degrees + 360;

	w = magnitude * cos(-degrees * degtorad);
	h = magnitude * sin(-degrees * degtorad);

	surface->drawLine(x, y, x+(int)w, y+(int)h, color);
	//surface->drawLine(x, y, x+(int)-w, y+(int)h, color);
}


Common::Error DrillerEngine::saveGameStreamExtended(Common::WriteStream *stream, bool isAutosave) {
	for (auto &it : _areaMap) { // All but skip area 255
		if (it._key == 255)
			continue;
		stream->writeUint16LE(it._key);
		stream->writeUint32LE(_drillStatusByArea[it._key]);
		stream->writeUint32LE(_drillMaxScoreByArea[it._key]);
		stream->writeUint32LE(_drillSuccessByArea[it._key]);
	}

	return Common::kNoError;
}

Common::Error DrillerEngine::loadGameStreamExtended(Common::SeekableReadStream *stream) {
	for (uint i = 0; i < _areaMap.size() - 1; i++) { // All except area 255
		uint16 key = stream->readUint16LE();
		assert(key != 255);
		assert(_areaMap.contains(key));
		_drillStatusByArea[key] = stream->readUint32LE();
		if (_drillStatusByArea[key] == kDrillerNoRig)
			if (drillDeployed(_areaMap[key]))
				removeDrill(_areaMap[key]);

		_drillMaxScoreByArea[key] = stream->readUint32LE();
		_drillSuccessByArea[key] = stream->readUint32LE();
	}

	return Common::kNoError;
}

} // End of namespace Freescape
