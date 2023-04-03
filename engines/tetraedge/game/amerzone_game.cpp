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

#include "tetraedge/tetraedge.h"
#include "tetraedge/game/application.h"
#include "tetraedge/game/amerzone_game.h"
#include "tetraedge/te/te_input_mgr.h"

namespace Tetraedge {

AmerzoneGame::AmerzoneGame() : Tetraedge::Game(), _orientationX(0.0f), _orientationY(0.0f),
_speedX(0.0f), _speedY(0.0f), _isInDrag(false), _edgeButtonRolloverCount(0),
_warpX(nullptr), _warpY(nullptr) {

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
	error("TODO: Implement AmerzoneGame::changeWarp");
	return false;
}

void AmerzoneGame::draw() {
	error("TODO: Implement AmerzoneGame::draw");
}

void AmerzoneGame::enter() {
	// TODO:
	//_puzzleDisjoncteur.setState(5);
	_inGameGui.load("GUI/InGame.lua");

	error("TODO: Implement AmerzoneGame::enter");
}

void AmerzoneGame::finishGame() {
	// Skip the animations of the original.
	// This is more like OnGameFinishedRotateAnimFinished.
	leave(true);
	Application *app = g_engine->getApplication();
	app->mainMenu().enter();
}

void AmerzoneGame::initLoadedBackupData() {

}

void AmerzoneGame::leave(bool flag) {
	error("TODO: Implement AmerzoneGame::leave");
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
