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

#include "common/math.h"

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/game/lua_binds.h"
#include "tetraedge/te/te_core.h"
#include "tetraedge/te/te_input_mgr.h"
#include "tetraedge/te/te_renderer.h"
#include "tetraedge/te/te_scene_warp.h"
#include "tetraedge/te/te_sound_manager.h"
#include "tetraedge/te/te_warp.h"

namespace Tetraedge {

AmerzoneGame::AmerzoneGame() : Tetraedge::Game(), _orientationX(0.0f), _orientationY(0.0f),
_speedX(0.0f), _speedY(0.0f), _isInDrag(false), _edgeButtonRolloverCount(0),
_warpX(nullptr), _warpY(nullptr), _prevWarpY(nullptr) {

}

void AmerzoneGame::addToBag(const Common::String &objname) {
	inventory().addObject(objname);
	// TODO: set this once _puzzleDisjoncteur is created
	//if (objname == "A_Fil_cuivre_jour")
	//	_puzzleDisjoncteur.addState(2);

	_notifier.push("<section style=\"center\" /><color r=\"0\" g=\"0\" b=\"0\"/><font file=\"Common/Fonts/Arial_r_16.tef\" />" + inventory().objectName(objname), "");
}

void AmerzoneGame::changeSpeedToMouseDirection() {
	error("TODO: Implement AmerzoneGame::changeSpeedToMouseDirection");
}

bool AmerzoneGame::changeWarp(const Common::String &zone, const Common::String &scene, bool fadeFlag) {
	if (_warpY) {
		_luaScript.execute("OnWarpLeave");
		_warpY->markerValidatedSignal().add(this, &AmerzoneGame::onObjectClick);
		_warpY->animFinishedSignal().add(this, &AmerzoneGame::onAnimationFinished);
		saveBackup("save.xml");
		_music.stop();
	}
	_prevWarpY = _warpY;
	_warpY = nullptr;

	Application *app = g_engine->getApplication();
	TeCore *core = g_engine->getCore();

	// TODO: There is a bunch of stuff here to cache the warp zone.
	// Just reload each time for now.
	if (!_warpY) {
		_warpY = new TeWarp();
		_warpY->setRotation(app->frontOrientationLayout().rotation());
		_warpY->init();
		float fov = 45.0f; //TODO: g_engine->getCore()->fileFlagSystemFlagsContains("HD") ? 60.0f : 45.0f;
		_warpY->setFov(fov);
	}
	_warpY->load(zone, false);
	_warpY->setVisible(true, false);
	TeWarp::debug = false;
	_warpY->activeMarkers(app->permanentHelp());
	_warpY->animFinishedSignal().add(this, &AmerzoneGame::onAnimationFinished);
	_luaContext.removeGlobal("OnWarpEnter");
	_luaContext.removeGlobal("OnWarpLeave");
	_luaContext.removeGlobal("OnWarpObjectHit");
	_luaContext.removeGlobal("OnMovieFinished");
	_luaContext.removeGlobal("OnAnimationFinished");
	_luaContext.removeGlobal("OnDialogFinished");
	_luaContext.removeGlobal("OnDocumentClosed");
	_luaContext.removeGlobal("OnPuzzleWon");
	Common::String sceneXml = zone;
	size_t dotpos = sceneXml.rfind('.');
	if (dotpos != Common::String::npos)
		sceneXml = sceneXml.substr(0, dotpos);
	sceneXml += ".xml";
	TeSceneWarp sceneWarp;
	sceneWarp.load(sceneXml, _warpY, false);

	_xAngleMin = FLT_MAX;
	_xAngleMax = FLT_MAX;
	_yAngleMax = 45.0 - _orientationY;
	_yAngleMin = _orientationY + 55.0;

	dotpos = sceneXml.rfind('.');
	Common::String sceneLua = sceneXml.substr(0, dotpos);
	sceneLua += ".xml";
	_luaScript.load(core->findFile(sceneLua));
	_luaScript.execute();
	_luaScript.execute("OnWarpEnter");
	if (fadeFlag) {
        startChangeWarpAnim();
	} else {
        onChangeWarpAnimFinished();
	}
	return false;
}

void AmerzoneGame::draw() {
	error("TODO: Implement AmerzoneGame::draw");
}

void AmerzoneGame::enter() {
	Application *app = g_engine->getApplication();
	// TODO:
	//_puzzleDisjoncteur.setState(5);
	_inGameGui.load("GUI/InGame.lua");

	TeLayout *inGame = _inGameGui.layoutChecked("inGame");
	// Note:
	app->frontOrientationLayout().addChild(inGame);
	_inventoryMenu.load();
	app->frontOrientationLayout().addChild(&_inventoryMenu);

	TeButtonLayout *invbtn = _inGameGui.buttonLayoutChecked("inventoryButton");
	invbtn->onMouseClickValidated().add(this, &AmerzoneGame::onInventoryButtonValidated);
	TeButtonLayout *helpbtn = _inGameGui.buttonLayoutChecked("helpButton");
	helpbtn->onMouseClickValidated().add(this, &AmerzoneGame::onHelpButtonValidated);
	if (app->permanentHelp()) {
		helpbtn->setVisible(false);
	}
	TeButtonLayout *skipvidbtn = _inGameGui.buttonLayoutChecked("skipVideoButton");
	skipvidbtn->setVisible(false);
	skipvidbtn->onMouseClickValidated().add(this, &AmerzoneGame::onSkipVideoButtonValidated);
	TeSpriteLayout *vid = _inGameGui.spriteLayoutChecked("video");
	vid->_tiledSurfacePtr->_frameAnim.onStop().add(this, &Game::onVideoFinished);
	vid->setVisible(false);
	_dialog2.load();
	app->frontOrientationLayout().addChild(&_dialog2);
	_question2.load();

	TeInputMgr *inputMgr = g_engine->getInputMgr();
	inputMgr->_mouseMoveSignal.add(this, &AmerzoneGame::onMouseMove);
	inputMgr->_mouseLUpSignal.add(this, &AmerzoneGame::onMouseLeftUp);
	inputMgr->_mouseLDownSignal.add(this, &AmerzoneGame::onMouseLeftDown);

	_orientationX = 0;
	_orientationY = 0;
	_isInDrag = false;
	_speedX = 0;
	_speedY = 0;

	_notifier.load();
	_warpX = new TeWarp();
	_warpX->setRotation(app->frontOrientationLayout().rotation());
	_warpX->init();
	_warpX->setVisible(true, false);
	_luaContext.create();
	_luaScript.attachToContext(&_luaContext);

	warning("TODO: Finish AmerzoneGame::enter");

	_playedTimer.start();
	_edgeButtonRolloverCount = 0;

	initLoadedBackupData();
}

void AmerzoneGame::finishGame() {
	// Skip the animations of the original.
	// This is more like OnGameFinishedRotateAnimFinished.
	leave(true);
	Application *app = g_engine->getApplication();
	app->mainMenu().enter();
}

// This is actually GameWarp::Load
void AmerzoneGame::initLoadedBackupData() {
	_luaContext.destroy();
	_luaContext.create();
	_luaContext.addBindings(LuaBinds::LuaOpenBinds);
	Application *app = g_engine->getApplication();
	if (!_loadName.empty()) {
		error("TODO: finish AmerzoneGame::initLoadedBackupData for direct load");
	}
	changeWarp(app->firstWarpPath(), app->firstScene(), true);
}

void AmerzoneGame::leave(bool flag) {
	error("TODO: Implement AmerzoneGame::leave");
}

bool AmerzoneGame::onChangeWarpAnimFinished() {
	if (_prevWarpY) {
		// TODO: remove callback from movement3
		_prevWarpY->setVisible(false, true);
		_prevWarpY->clear();
		_prevWarpY = nullptr;
		// TODO: set some sprite not visible here.
		error("TODO: Finish AmerzoneGame::onChangeWarpAnimFinished");
	}
	_warpY->markerValidatedSignal().remove(this, &AmerzoneGame::onObjectClick);
	optimizeWarpResources();
	return false;
}

bool AmerzoneGame::onHelpButtonValidated() {
	g_engine->getSoundManager()->playFreeSound("Sounds/SFX/Clic_prec-suiv.ogg", 1.0f, "sfx");

	bool active = true;
	TeWarp::debug = TeWarp::debug == false;
	if (!TeWarp::debug && !g_engine->getApplication()->permanentHelp())
		active = false;

	_warpY->activeMarkers(active);
	return false;
}

bool AmerzoneGame::onAnimationFinished(const Common::String &anim) {
	error("TODO: Implement AmerzoneGame::onAnimationFinished");
}

bool AmerzoneGame::onMouseLeftUp(const Common::Point &pt) {
	error("TODO: Implement AmerzoneGame::onMouseLeftUp");
}

bool AmerzoneGame::onMouseLeftDown(const Common::Point &pt) {
	error("TODO: Implement AmerzoneGame::onMouseLeftDown");
}

bool AmerzoneGame::onObjectClick(const Common::String &obj) {
	error("TODO: Implement AmerzoneGame::onObjectClick");
}

void AmerzoneGame::optimizeWarpResources() {
	error("TODO: Implement AmerzoneGame::optimizeWarpResources");
}

void AmerzoneGame::setAngleX(float angle) {
	float diff = angle - _orientationX;
	float distFromMin = _xAngleMin - diff;
	if (distFromMin < 0)
		angle += distFromMin;
	float distFromMax = diff + _xAngleMax;
	if (distFromMax < 0)
		angle -= distFromMax;

	diff = angle - _orientationX;
	_xAngleMin -= diff;
	_xAngleMax += diff;

	float roundedAngle = (int)(angle / 360.0f) * 360;
	_orientationX = roundedAngle;
	if (roundedAngle > 360.0f || roundedAngle < -360.0f)
		_orientationX = 0;
}

void AmerzoneGame::setAngleY(float angle) {
	float diff = angle - _orientationY;
	float distFromMin = _yAngleMin - diff;
	if (distFromMin < 0)
		angle += distFromMin;
	float distFromMax = diff + _yAngleMax;
	if (distFromMax < 0)
		angle -= distFromMax;

	diff = angle - _orientationY;
	_yAngleMin -= diff;
	_yAngleMax += diff;

	if (angle < -55.0f)
		_orientationY = -55.0f;
	else if (_orientationY > 45.0f)
		_orientationY = 45.0f;
}

void AmerzoneGame::speedX(float speed) {
	_speedX = CLIP(speed, -10000.0f, 10000.0f);
}

void AmerzoneGame::speedY(float speed) {
	_speedY = CLIP(speed, -10000.0f, 10000.0f);
}

void AmerzoneGame::startChangeWarpAnim() {
	_warpX->update();
	_warpY->update();
	if (_prevWarpY == nullptr) {
		onChangeWarpAnimFinished();
	} else {
		TeRenderer *renderer = g_engine->getRenderer();
		renderer->clearBuffer(TeRenderer::ColorBuffer);
		renderer->clearBuffer(TeRenderer::DepthBuffer);
		if (_warpX)
			_warpX->render();
		_prevWarpY->render();

		// This is a much simpler version of what the original does
		// as it reuses the fade code.
		g_engine->getApplication()->captureFade();
		_prevWarpY->unloadTextures();
		g_engine->getApplication()->visualFade().animateFadeWithZoom();
	}
}

void AmerzoneGame::update() {
	TeInputMgr *inputMgr = g_engine->getInputMgr();

	// TODO:
	// if (!inputMgr->isLeftDown())
	//     isInDrag(false);

	Application *app = g_engine->getApplication();
	if (!app->compassLook()) {
		if (_isInDrag) {
			TeVector2s32 mousePos = TeVector2s32(inputMgr->lastMousePos());
			TeVector3f32 offset = TeVector3f32(mousePos - _mouseDragLast);
			TeMatrix4x4 orientLayoutMatrix = app->frontOrientationLayout().rotation().toTeMatrix();
			TeVector3f32 rotOffset = orientLayoutMatrix * offset;
			if (app->inverseLook()) {
				setAngleX(_orientationX + rotOffset.x() / 2);
				setAngleY(_orientationY - rotOffset.y() / 2);
			} else {
				setAngleX(_orientationX - rotOffset.x() / 2);
				setAngleY(_orientationY + rotOffset.y() / 2);
			}
			_mouseDragLast = inputMgr->lastMousePos();
		} else {
			if (_edgeButtonRolloverCount > 0) {
				changeSpeedToMouseDirection();
			}
			float dragtime = (float)(_dragTimer.timeElapsed() / 1000000.0);
			setAngleX(_orientationX - _speedX * dragtime);
			setAngleY(_orientationY + _speedY * dragtime);
		}
	} else {
		// Compass stuff happens here in the game, but it's
		// not fully implemented - the TeCompass class is just
		// stubs.
		error("TODO: Implement compass support in AmerzoneGame::update.");
	}

	if (_warpY) {
		TeVector2s32 mousePos = TeVector2s32(inputMgr->lastMousePos());
		TeVector3f32 offset = TeVector3f32(mousePos - _mouseDragStart);
		if (offset.length() > 20.0f)
			_warpY->setMouseLeftUpForMakers();
	}

	TeQuaternion xRot = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), (float)(_orientationX * M_PI) / 180);
	TeQuaternion yRot = TeQuaternion::fromAxisAndAngle(TeVector3f32(1, 0, 0), (float)(_orientationY * M_PI) / 180);

	if (_warpX)
		_warpX->rotateCamera(xRot * yRot);
	if (_warpY)
		_warpY->rotateCamera(xRot * yRot);
	if (_warpX)
		_warpX->update();
	if (_warpY)
		_warpY->update();

}

bool AmerzoneGame::onDialogFinished(const Common::String &val) {
	_luaScript.execute("OnDialogFinished", val);
	return false;
}

bool AmerzoneGame::onVideoFinished() {
	_inGameGui.buttonLayoutChecked("videoBackgroundButton")->setVisible(false);
	_inGameGui.buttonLayoutChecked("skipVideoButton")->setVisible(false);
	TeSpriteLayout *video = _inGameGui.spriteLayoutChecked("video");
	Common::String vidPath = video->_tiledSurfacePtr->loadedPath();
	video->setVisible(false);
	_music.stop();
	// TODO:
	//Application *app = g_engine->getApplication();
	//if (app->musicOn()) {
	//	app->music().play();
	//}
	_running = true;
	_luaScript.execute("OnMovieFinished", vidPath);
	return false;
}

} // end namespace Tetraedge
