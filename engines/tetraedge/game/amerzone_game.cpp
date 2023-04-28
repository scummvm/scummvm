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
_speedX(0.0f), _speedY(0.0f), _isInDrag(false), _musicOn(false), _edgeButtonRolloverCount(0),
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
		_videoMusic.stop();
	}
	_prevWarpY = _warpY;
	_warpY = nullptr;

	Application *app = g_engine->getApplication();
	TeCore *core = g_engine->getCore();

	// TODO: There is a bunch of stuff here to cache the warp zone.
	// Just reload each time for now.
	if (!_warpY) {
		_warpY = new TeWarp();
		_warpY->setRotation(app->frontLayout().rotation());
		_warpY->init();
		float fov = 60.0f; //TODO: g_engine->getCore()->fileFlagSystemFlagsContains("HD") ? 60.0f : 45.0f;
		_warpY->setFov((float)(fov * M_PI / 180.0));
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
	_yAngleMin = 45.0f - _orientationY;
	_yAngleMax = _orientationY + 55.0f;

	dotpos = sceneXml.rfind('.');
	Common::String sceneLua = sceneXml.substr(0, dotpos);
	sceneLua += ".lua";
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
	if (!_running)
		return;
	if (_warpX)
		_warpX->render();
	if (_warpY)
		_warpY->render();
}

void AmerzoneGame::enter() {
	Application *app = g_engine->getApplication();
	// TODO:
	//_puzzleDisjoncteur.setState(5);
	_inGameGui.load("GUI/InGame.lua");

	TeLayout *inGame = _inGameGui.layoutChecked("inGame");
	app->frontLayout().addChild(inGame);
	// DocumentsBrowser and Inventory get added as children of InventoryMenu
	_inventoryMenu.load();
	app->frontLayout().addChild(&_inventoryMenu);

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
	app->frontLayout().addChild(&_dialog2);
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
	_warpX->setRotation(app->frontLayout().rotation());
	_warpX->init();
	float fov = 60.0f; //TODO: g_engine->getCore()->fileFlagSystemFlagsContains("HD") ? 60.0f : 45.0f;
	_warpX->setFov((float)(fov * M_PI / 180.0));
	_warpX->setVisible(true, false);
	_luaContext.create();
	_luaScript.attachToContext(&_luaContext);

	// Game also sets up fade sprites, which is set up in Game.

	_running = true;
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

void AmerzoneGame::isInDrag(bool inDrag) {
	const Common::Point mousePt = g_engine->getInputMgr()->lastMousePos();
	if (inDrag != _isInDrag) {
		_isInDrag = inDrag;
		g_system->lockMouse(inDrag);
		if (inDrag) {
			// Start drag operation
			_mouseDragStart = mousePt;
			_mouseDragLast = mousePt;
			_decelAnimX.stop();
			_decelAnimY.stop();
			_dragTimer.stop();
			_dragTimer.start();
		} else {
			// Finish drag operation
			_dragTimer.timeElapsed();
			Application *app = g_engine->getApplication();
			TeVector3f32 mouseDir(mousePt.x - _mouseDragLast.x, mousePt.y - _mouseDragLast.y, 0);
			if (app->inverseLook())
				mouseDir = mouseDir * -1.0f;
			const TeMatrix4x4 layoutRot = app->frontLayout().rotation().toTeMatrix();
			TeVector3f32 dest = layoutRot * mouseDir;
			dest.x() /= 2;
			dest.y() /= 2;
			_speedX = CLIP(dest.x(), -10000.0f, 10000.0f);
			_speedY = CLIP(dest.y(), -10000.0f, 10000.0f);
			startDecelerationAnim();
		}
	}
}

void AmerzoneGame::leave(bool flag) {
	_inGameGui.unload();
	_question2.unload();
	Application *app = g_engine->getApplication();
	app->frontLayout().removeChild(&_dialog2);
	_dialog2.unload();
	if (_warpX) {
		delete _warpX;
		_warpX = nullptr;
	}
	if (_warpY) {
		saveBackup("save.xml");
	}
	app->frontLayout().removeChild(&_inventoryMenu);
	_inventoryMenu.unload();

	// TODO: game does this.. doesn't this leak?
	_warpY = nullptr;
	_prevWarpY = nullptr;

	// TODO: Game goes through a list of (cached?) warps here to clean up.
	warning("TODO: Finish AmerzoneGame::leave");

	_notifier.unload();
	_luaContext.destroy();
	_running = false;
	_playedTimer.stop();
	_videoMusic.stop();
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
	_warpY->setMouseLeftUpForMakers();
	TeVector3f32 offset = TeVector3f32(pt - _mouseDragStart);
	if (offset.length() > 20.0f)
		_warpY->checkObjectEvents();
	isInDrag(false);
	return false;
}

bool AmerzoneGame::onMouseLeftDown(const Common::Point &pt) {
	isInDrag(true);
	return false;
}

bool AmerzoneGame::onObjectClick(const Common::String &obj) {
	error("TODO: Implement AmerzoneGame::onObjectClick");
}

bool AmerzoneGame::onPuzzleEnterAnimLoadTime() {
	error("TODO: Implement AmerzoneGame::onPuzzleEnterAnimLoadTime");
}

void AmerzoneGame::optimizeWarpResources() {
	// Note: original calls this OptimizeWarpRessources
	warning("TODO: Implement AmerzoneGame::optimizeWarpResources");
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

	float roundedAngle = angle - (int)(angle / 360.0f) * 360;
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

	_orientationY = CLIP(angle, -55.0f, 45.0f);
}

void AmerzoneGame::showPuzzle(int puzzleNo, int puzParam1, int puzParam2) {
	_puzzleNo = puzzleNo;
	_puzParam1 = puzParam1;
	_puzParam2 = puzParam2;
	onPuzzleEnterAnimLoadTime();
}


void AmerzoneGame::speedX(const float &speed) {
	_speedX = CLIP(speed, -10000.0f, 10000.0f);
}

void AmerzoneGame::speedY(const float &speed) {
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

void AmerzoneGame::startDecelerationAnim() {
	_decelAnimX.stop();
	_decelAnimY.stop();

	Common::Array<float> curve;
	curve.push_back(0);
	curve.push_back(0.35f);
	curve.push_back(0.68f);
	curve.push_back(0.85f);
	curve.push_back(0.93f);
	curve.push_back(0.97f);
	curve.push_back(1);

	_decelAnimX.setCurve(curve);
	_decelAnimX._duration = 400;
	_decelAnimX._startVal = _speedX;
	_decelAnimX._endVal = 0;
	_decelAnimX._callbackObj = this;
	_decelAnimX._callbackMethod = &AmerzoneGame::speedX;
	_decelAnimX.play();

	_decelAnimY.setCurve(curve);
	_decelAnimY._duration = 400;
	_decelAnimY._startVal = _speedY;
	_decelAnimY._endVal = 0;
	_decelAnimY._callbackObj = this;
	_decelAnimY._callbackMethod = &AmerzoneGame::speedY;
	_decelAnimY.play();
}

void AmerzoneGame::update() {
	TeInputMgr *inputMgr = g_engine->getInputMgr();

	//if (!inputMgr->>isLeftDown())
	//     isInDrag(false);

	Application *app = g_engine->getApplication();
	if (!app->compassLook()) {
		if (_isInDrag) {
			TeVector2s32 mousePos = TeVector2s32(inputMgr->lastMousePos());
			TeVector3f32 offset = TeVector3f32(mousePos - _mouseDragLast);
			TeMatrix4x4 orientLayoutMatrix = app->frontLayout().rotation().toTeMatrix();
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

	// Rotate x around the Y axis (spinning left/right on the spot)
	TeQuaternion xRot = TeQuaternion::fromAxisAndAngle(TeVector3f32(0, 1, 0), (float)(_orientationX * M_PI) / 180);
	// Rotate y around the axis perpendicular to the x rotation
	TeVector3f32 yRotAxis = TeVector3f32(1, 0, 0);
	xRot.inverse().transform(yRotAxis);
	TeQuaternion yRot = TeQuaternion::fromAxisAndAngle(yRotAxis, (float)(_orientationY * M_PI) / 180);

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
	Application *app = g_engine->getApplication();
	_videoMusic.stop();
	if (_musicOn)
		app->music().play();
	_running = true;
	_luaScript.execute("OnMovieFinished", vidPath);
	return false;
}

} // end namespace Tetraedge
