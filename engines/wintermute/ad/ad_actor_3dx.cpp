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

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/util.h"
#include "math/utils.h"

#include "engines/wintermute/ad/ad_actor_3dx.h"
#include "engines/wintermute/ad/ad_attach_3dx.h"
#include "engines/wintermute/ad/ad_entity.h"
#include "engines/wintermute/ad/ad_game.h"
#include "engines/wintermute/ad/ad_path.h"
#include "engines/wintermute/ad/ad_path3d.h"
#include "engines/wintermute/ad/ad_scene.h"
#include "engines/wintermute/ad/ad_scene_geometry.h"
#include "engines/wintermute/ad/ad_sentence.h"
#include "engines/wintermute/ad/ad_waypoint_group.h"
#include "engines/wintermute/base/base_active_rect.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_region.h"
#include "engines/wintermute/base/base_surface_storage.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/base/gfx/3dshadow_volume.h"
#include "engines/wintermute/base/gfx/opengl/base_render_opengl3d.h"
#include "engines/wintermute/base/gfx/xmodel.h"
#include "engines/wintermute/base/gfx/xmath.h"
#include "engines/wintermute/base/gfx/3dutils.h"
#include "engines/wintermute/base/particles/part_emitter.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(AdActor3DX, false)

//////////////////////////////////////////////////////////////////////////
AdActor3DX::AdActor3DX(BaseGame *inGame) : AdObject3D(inGame) {
	_targetPoint3D = DXVector3(0.0f, 0.0f, 0.0f);
	_targetPoint2D = new BasePoint;

	_targetAngle = 0.0f;
	_afterWalkAngle = -1.0f;

	_path3D = new AdPath3D(inGame);
	_path2D = new AdPath(inGame);

	_talkAnimName = Common::String("talk");

	_idleAnimName = Common::String("idle");

	_walkAnimName = Common::String("walk");

	_turnLeftAnimName = Common::String("turnleft");

	_turnRightAnimName = Common::String("turnright");

	_talkAnimChannel = 0;

	_gameRef->_renderer3D->enableShadows();

	_directWalkMode = DIRECT_WALK_NONE;
	_directTurnMode = DIRECT_TURN_NONE;
	_directWalkVelocity = 0.0f;
	_directTurnVelocity = 0.0f;

	_defaultTransTime = 200;
	_defaultStopTransTime = 200;
	_stateAnimChannel = -1;

	_goToTolerance = 2;

	_partOffset = DXVector3(0.0f, 0.0f, 0.0f);
}

//////////////////////////////////////////////////////////////////////////
AdActor3DX::~AdActor3DX() {
	// delete attachments
	for (uint32 i = 0; i < _attachments.size(); i++) {
		delete _attachments[i];
	}
	_attachments.clear();

	// delete transition times
	for (uint32 i = 0; i < _transitionTimes.size(); i++) {
		delete _transitionTimes[i];
	}
	_transitionTimes.clear();

	delete _path2D;
	_path2D = nullptr;
	delete _path3D;
	_path3D = nullptr;

	delete _targetPoint2D;
	_targetPoint2D = nullptr;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::update() {
	if (!_xmodel) {
		return true;
	}

	if (_gameRef->_state == GAME_FROZEN) {
		return true;
	}

	AdGame *adGame = (AdGame *)_gameRef;

	if (_state == STATE_READY && _stateAnimChannel >= 0 && _xmodel) {
		_stateAnimChannel = -1;
	}

	if (_sentence && _state != STATE_TALKING) {
		_sentence->finish();
		delete _sentence;
		_sentence = nullptr;

		// kill talking anim
		if (_talkAnimChannel > 0)
			_xmodel->stopAnim(_talkAnimChannel, _defaultStopTransTime);
	}

	// update state
	switch (_state) {
	//////////////////////////////////////////////////////////////////////////
	case STATE_DIRECT_CONTROL:
		if (_directWalkMode == DIRECT_WALK_NONE && _directTurnMode == DIRECT_TURN_NONE) {
			_state = _nextState;
			_nextState = STATE_READY;
		} else {
			// set animation
			if (_directWalkMode != DIRECT_WALK_NONE) {
				// disabled in original code
			} else if (_directTurnMode != DIRECT_TURN_NONE) {
				if (!_directTurnAnim.empty()) {
					_xmodel->playAnim(0, _directTurnAnim, _defaultTransTime, false, _defaultStopTransTime);
				} else {
					_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
				}
			}

			// move and/or turn
			float turnVel = _directTurnVelocity == 0.0f ? _angVelocity : _directTurnVelocity;

			if (_directTurnMode == DIRECT_TURN_CW) {
				_angle += turnVel * (float)_gameRef->_deltaTime / 1000.f;
				_angle = BaseUtils::normalizeAngle(_angle);
			}

			if (_directTurnMode == DIRECT_TURN_CCW) {
				_angle -= turnVel * (float)_gameRef->_deltaTime / 1000.f;
				_angle = BaseUtils::normalizeAngle(_angle);
			}

			float walkVel = _directWalkVelocity == 0.0f ? _velocity : _directWalkVelocity;
			DXVector3 newPos = _posVector;
			if (_directWalkMode == DIRECT_WALK_FW) {
				newPos._x += -sinf(degToRad(_angle)) * walkVel * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
				newPos._z += -cosf(degToRad(_angle)) * walkVel * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
			}

			if (_directWalkMode == DIRECT_WALK_BK) {
				newPos._x -= -sinf(degToRad(_angle)) * walkVel * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
				newPos._z -= -cosf(degToRad(_angle)) * walkVel * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
			}

			AdScene *scene = ((AdGame *)_gameRef)->_scene;

			if (scene && scene->_geom) {
				bool canWalk = false;

				if (scene->_2DPathfinding) {
					DXMatrix newWorldMat;
					getMatrix(&newWorldMat, &newPos);

					int32 newX, newY;
					convert3DTo2D(&newWorldMat, &newX, &newY);
					canWalk = !scene->isBlockedAt(newX, newY, false, this);
				} else {
					canWalk = scene->_geom->directPathExists(&_posVector, &newPos);
				}

				if (canWalk) {
					if (!_directWalkAnim.empty()) {
						_xmodel->playAnim(0, _directWalkAnim, _defaultTransTime, false, _defaultStopTransTime);
					} else {
						_xmodel->playAnim(0, _walkAnimName, _defaultTransTime, false, _defaultStopTransTime);
					}

					_posVector = newPos;
				} else {
					_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
				}
			}
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_TURNING:
		if (_turningLeft) {
			_xmodel->playAnim(0, _turnLeftAnimName, _defaultTransTime, false, _defaultStopTransTime);
		} else {
			_xmodel->playAnim(0, _turnRightAnimName, _defaultTransTime, false, _defaultStopTransTime);
		}

		if (turnToStep(_angVelocity)) {
			_state = _nextState;
			_nextState = STATE_READY;
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_SEARCHING_PATH:
		// keep asking scene for the path
		_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);

		if (adGame->_scene->_2DPathfinding) {
			if (adGame->_scene->getPath(BasePoint(_posX, _posY), *_targetPoint2D, _path2D, this)) {
				_state = STATE_WAITING_PATH;
			}
		} else {
			if (adGame->_scene->_geom->getPath(_posVector, _targetPoint3D, _path3D))
				_state = STATE_WAITING_PATH;
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_WAITING_PATH:
		// wait until the scene finished the path
		_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
		if (adGame->_scene->_2DPathfinding) {
			if (_path2D->_ready) {
				followPath2D();
			}
		} else {
			if (_path3D->_ready) {
				followPath3D();
			}
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_FOLLOWING_PATH:
		if (adGame->_scene->_2DPathfinding) {
			getNextStep2D();
		} else {
			getNextStep3D();
		}

		_xmodel->playAnim(0, _walkAnimName, _defaultTransTime, false, _defaultStopTransTime);
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		if (!_sentence)
			break;

		_sentence->update();

		if (_sentence->_currentSkelAnim) {
			_tempSkelAnim = _sentence->_currentSkelAnim;
		}

		bool timeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) ||
		                (!_sentence->_sound && _sentence->_duration <= _gameRef->getTimer()->getTime() - _sentence->_startTime);
		if (_tempSkelAnim == nullptr || !_xmodel->isAnimPending(0, _tempSkelAnim) || timeIsUp) {
			if (timeIsUp) {
				_sentence->finish();
				_tempSkelAnim = nullptr;
				_state = _nextState;
				_nextState = STATE_READY;

				if (_talkAnimChannel > 0)
					_xmodel->stopAnim(_talkAnimChannel, _defaultStopTransTime);
			} else {
				_tempSkelAnim = _sentence->getNextStance();
				if (_tempSkelAnim)
					_xmodel->playAnim(0, _tempSkelAnim, _defaultTransTime, true, _defaultStopTransTime);
				else {
					if (_xmodel->getAnimationSetByName(_talkAnimName))
						_xmodel->playAnim(_talkAnimChannel, _talkAnimName, _defaultTransTime, false, _defaultStopTransTime);
					else
						_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
				}

				((AdGame *)_gameRef)->addSentence(_sentence);
			}
		} else {
			if (_tempSkelAnim) {
				_xmodel->playAnim(0, _tempSkelAnim, _defaultTransTime, false, _defaultStopTransTime);
			}

			((AdGame *)_gameRef)->addSentence(_sentence);
		}
		break;
	}

	case STATE_PLAYING_ANIM:
		if (_stateAnimChannel != 0) {
			_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
		}
		break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		_xmodel->playAnim(0, _idleAnimName, _defaultTransTime, false, _defaultStopTransTime);
		break;

	case STATE_IDLE:
	case STATE_TURNING_LEFT:
	case STATE_TURNING_RIGHT:
	case STATE_PLAYING_ANIM_SET:
	case STATE_NONE:
		break;
	} // switch(_state)

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && !_xmodel->isAnimPending(_stateAnimChannel)) {
		_state = _nextState;
		_nextState = STATE_READY;
	}

	updateBlockRegion();
	_ready = (_state == STATE_READY);

	// setup 2D position
	int origX = _posX;
	int origY = _posY;

	bool ret = AdObject3D::update();

	if (origX != _posX || origY != _posY) {
		afterMove();
	}

	if (_xmodel) {
		_xmodel->update();

		if (_shadowModel) {
			_shadowModel->update();
		}
	}

	updateAttachments();
	updatePartEmitter();

	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::display() {
	if (!_xmodel) {
		return true;
	}

	updateSounds();
	setupLights();

	_gameRef->_renderer3D->setSpriteBlendMode(_blendMode);

	if (_hasAmbientLightColor) {
		_gameRef->_renderer3D->setAmbientLightColor(_ambientLightColor);
	}

	TShadowType shadowType = _gameRef->getMaxShadowType(this);

	if (shadowType == SHADOW_STENCIL) {
		displayShadowVolume();
	} else if (shadowType > SHADOW_NONE) {
		DXVector3 lightPos = DXVector3(_shadowLightPos._x * _scale3D,
									   _shadowLightPos._y * _scale3D,
									   _shadowLightPos._z * _scale3D);
		_gameRef->_renderer3D->displayShadow(this, &lightPos, true);
	}

	_gameRef->_renderer3D->setSpriteBlendMode(_blendMode);
	_gameRef->_renderer3D->setWorldTransform(_worldMatrix);

	bool res = _xmodel->render();

	if (_registrable) {
		_gameRef->_renderer->addRectToList(new BaseActiveRect(_gameRef, this, _xmodel,
		                                                      _xmodel->_boundingRect.left,
		                                                      _xmodel->_boundingRect.top,
		                                                      _xmodel->_boundingRect.right - _xmodel->_boundingRect.left,
		                                                      _xmodel->_boundingRect.bottom - _xmodel->_boundingRect.top,
		                                                      true));
	}

	displayAttachments(true);

	if (_hasAmbientLightColor) {
		_gameRef->_renderer3D->setDefaultAmbientLightColor();
	}

	if (_active && _partEmitter) {
		_gameRef->_renderer3D->setup2D();
		_partEmitter->display();
	}

	// this functionality appearently was removed in either WME lite
	// or in the ScummVM port. It might have been replaced as well
	// not sure what to do about it right now
	// accessibility
	//	if (_gameRef->_accessMgr->GetActiveObject() == this) {
	//		_gameRef->_accessMgr->SetHintRect(&_xmodel->m_BoundingRect);
	//	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::renderModel() {
	if (!_xmodel) {
		return true;
	}

	_gameRef->_renderer3D->setWorldTransform(_worldMatrix);

	if (_shadowModel) {
		_shadowModel->render();
	} else {
		_xmodel->render();
	}

	displayAttachments(false);
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::displayShadowVolume() {
	DXVector3 pos;
	DXVector3 target;
	DXVector3 lightVector;
	float extrusionDepth;

	if (!_xmodel) {
		return false;
	}

	_gameRef->_renderer3D->setWorldTransform(_worldMatrix);

	DXVector3 lightPos = DXVector3(_shadowLightPos._x * _scale3D,
	                               _shadowLightPos._y * _scale3D,
								   _shadowLightPos._z * _scale3D);
	pos = _posVector + lightPos;
	target = _posVector;

	lightVector = pos - target;
	extrusionDepth = DXVec3Length(&lightVector) * 1.5f;
	DXVec3Normalize(&lightVector, &lightVector);

	getShadowVolume()->setColor(_shadowColor);

	getShadowVolume()->reset();

	XModel *shadowModel;
	if (_shadowModel) {
		shadowModel = _shadowModel;
	} else {
		shadowModel = _xmodel;
	}

	shadowModel->updateShadowVol(getShadowVolume(), &_worldMatrix, &lightVector, extrusionDepth);

	DXMatrix origWorld;
	_gameRef->_renderer3D->getWorldTransform(&origWorld);

	// handle the attachments
	for (uint32 i = 0; i < _attachments.size(); i++) {
		AdAttach3DX *at = _attachments[i];

		if (!at->_active) {
			continue;
		}

		DXMatrix *boneMat = _xmodel->getBoneMatrix(at->getParentBone().c_str());
		if (!boneMat) {
			continue;
		}

		DXMatrix viewMat;
		DXMatrixMultiply(&viewMat, boneMat, &_worldMatrix);

		at->displayShadowVol(&viewMat, &lightVector, extrusionDepth, true);
	}

	// restore model's world matrix and render the shadow volume
	_gameRef->_renderer3D->setWorldTransform(origWorld);

	getShadowVolume()->renderToStencilBuffer();

	// finally display all the shadows rendered into stencil buffer
	getShadowVolume()->renderToScene();

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::updateAttachments() {
	for (uint32 i = 0; i < _attachments.size(); i++) {
		if (_attachments[i]->_active) {
			_attachments[i]->update();
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::displayAttachments(bool registerObjects) {
	if (!_xmodel) {
		return false;
	}

	if (_attachments.size() == 0) {
		return true;
	}

	DXMatrix origView;
	_gameRef->_renderer3D->getWorldTransform(&origView);

	for (uint32 i = 0; i < _attachments.size(); i++) {
		AdAttach3DX *at = _attachments[i];
		if (!at->_active) {
			continue;
		}

		DXMatrix *boneMat = _xmodel->getBoneMatrix(at->getParentBone().c_str());
		if (!boneMat) {
			continue;
		}

		DXMatrix viewMat;
		DXMatrixMultiply(&viewMat, boneMat, &origView);

		at->displayAttachable(&viewMat, registerObjects);
	}

	_gameRef->_renderer3D->setWorldTransform(origView);

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::turnTo(float angle) {
	_turningLeft = prepareTurn(angle);

	if (_targetAngle == _angle) { // no need to turn
		_state = _nextState;
		_nextState = STATE_READY;
	} else {
		_state = STATE_TURNING;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::goTo3D(DXVector3 targetPos, float targetAngle) {
	_afterWalkAngle = targetAngle;

	if (_targetPoint3D == targetPos && _state == STATE_FOLLOWING_PATH) {
		return;
	}

	_path3D->reset();
	_path3D->setReady(false);

	_targetPoint3D = targetPos;

	_state = STATE_SEARCHING_PATH;
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::goTo2D(int x, int y, float targetAngle) {
	_afterWalkAngle = targetAngle;

	if (x == _targetPoint2D->x && y == _targetPoint2D->y && _state == STATE_FOLLOWING_PATH) {
		return;
	}

	_path2D->reset();
	_path2D->setReady(false);

	_targetPoint2D->x = x;
	_targetPoint2D->y = y;

	((AdGame *)_gameRef)->_scene->correctTargetPoint(_posX, _posY, &_targetPoint2D->x, &_targetPoint2D->y, true, this);

	_state = STATE_SEARCHING_PATH;
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::followPath3D() {
	_path3D->getFirst();

	// are there points to follow?
	if (_path3D->getCurrent() != nullptr) {
		_state = STATE_FOLLOWING_PATH;
		initLine3D(_posVector, *_path3D->getCurrent(), true);
	} else {
		if (_afterWalkAngle != -1.0f) {
			turnTo(_afterWalkAngle);
		} else {
			_state = STATE_READY;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::getNextStep3D() {
	if (_angle != _targetAngle)
		turnToStep(_angVelocity);

	DXVector3 newPos = _posVector;
	newPos._x += -sinf(degToRad(_targetAngle)) * _velocity * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
	newPos._z += -cosf(degToRad(_targetAngle)) * _velocity * _scale3D * (float)_gameRef->_deltaTime / 1000.f;

	DXVector3 origVec, newVec;
	DXVector3 *currentPos = _path3D->getCurrent();

	if (currentPos != nullptr) {
		origVec = *currentPos - _posVector;
		newVec = *currentPos - newPos;
	}

	if (currentPos == nullptr || DXVec3Length(&origVec) < DXVec3Length(&newVec)) {
		if (currentPos != nullptr) {
			_posVector = *currentPos;
		}

		if (_path3D->getNext() == nullptr) {
			_path3D->reset();
			if (_afterWalkAngle != -1.0f) {
				turnTo(_afterWalkAngle);
			} else {
				_state = _nextState;
				_nextState = STATE_READY;
			}
		} else {
			initLine3D(_posVector, *_path3D->getCurrent(), false);
		}
	} else {
		_posVector = newPos;
	}
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::initLine3D(DXVector3 startPt, DXVector3 endPt, bool firstStep) {
	if (firstStep) {
		_nextState = STATE_FOLLOWING_PATH;
		turnTo(radToDeg(-atan2(endPt._z - startPt._z, endPt._x - startPt._x)) - 90);
	} else {
		_turningLeft = prepareTurn(radToDeg(-atan2(endPt._z - startPt._z, endPt._x - startPt._x)) - 90);
	}
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::getNextStep2D() {
	AdGame *adGame = (AdGame *)_gameRef;

	if (!adGame || !adGame->_scene || !adGame->_scene->_geom || !_path2D || !_path2D->getCurrent()) {
		_state = _nextState;
		_nextState = STATE_READY;
		return;
	}

	if (_angle != _targetAngle) {
		turnToStep(_angVelocity);
	}

	DXVector3 newPos = _posVector;
	newPos._x += -sinf(degToRad(_targetAngle)) * _velocity * _scale3D * (float)_gameRef->_deltaTime / 1000.f;
	newPos._z += -cosf(degToRad(_targetAngle)) * _velocity * _scale3D * (float)_gameRef->_deltaTime / 1000.f;

	DXVector3 currentPoint;
	adGame->_scene->_geom->convert2Dto3DTolerant(_path2D->getCurrent()->x,
	                                                      _path2D->getCurrent()->y,
	                                                      &currentPoint);

	DXVector3 origVec, newVec;

	origVec = currentPoint - _posVector;
	newVec = currentPoint - newPos;

	if (DXVec3Length(&origVec) < DXVec3Length(&newVec)) {
		_posVector = currentPoint;

		if (_path2D->getNext() == nullptr) {
			_path2D->reset();

			if (_afterWalkAngle != -1.0f) {
				turnTo(_afterWalkAngle);
			} else {
				_state = _nextState;
				_nextState = STATE_READY;
			}
		} else {
			adGame->_scene->_geom->convert2Dto3DTolerant(_path2D->getCurrent()->x,
			                                                      _path2D->getCurrent()->y,
			                                                      &currentPoint);
			initLine3D(_posVector, currentPoint, false);
		}
	} else
		_posVector = newPos;
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::followPath2D() {
	AdGame *adGameRef = (AdGame *)_gameRef;

	// skip current position
	_path2D->getFirst();

	while (_path2D->getCurrent() != nullptr) {
		if (_path2D->getCurrent()->x != _posX || _path2D->getCurrent()->y != _posY) {
			break;
		}

		_path2D->getNext();
	}

	// are there points to follow?
	if (_path2D->getCurrent() != nullptr) {
		_state = STATE_FOLLOWING_PATH;

		DXVector3 currentPoint;
		adGameRef->_scene->_geom->convert2Dto3DTolerant(_path2D->getCurrent()->x,
		                                                         _path2D->getCurrent()->y,
		                                                         &currentPoint);

		initLine3D(_posVector, currentPoint, true);
	} else {
		if (_afterWalkAngle != -1.0f) {
			turnTo(_afterWalkAngle);
		} else {
			_state = STATE_READY;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::prepareTurn(float targetAngle) {
	bool turnLeft;

	_angle = BaseUtils::normalizeAngle(_angle);
	targetAngle = BaseUtils::normalizeAngle(targetAngle);

	if (_angle == targetAngle) {
		_targetAngle = _angle;
		return true;
	}

	float delta1, delta2, delta3, delta;

	delta1 = targetAngle - _angle;
	delta2 = targetAngle + 360 - _angle;
	delta3 = targetAngle - 360 - _angle;

	delta1 = (fabs(delta1) <= fabs(delta2)) ? delta1 : delta2;
	delta = (fabs(delta1) <= fabs(delta3)) ? delta1 : delta3;

	_targetAngle = _angle + delta;
	turnLeft = (delta < 0);

	return turnLeft;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::turnToStep(float velocity) {
	if (_turningLeft) {
		_angle -= velocity * (float)_gameRef->_deltaTime / 1000.f;
		if (_angle < _targetAngle) {
			_angle = _targetAngle;
		}
	} else {
		_angle += velocity * (float)_gameRef->_deltaTime / 1000.f;
		if (_angle > _targetAngle) {
			_angle = _targetAngle;
		}
	}

	// done turning?
	if (_angle == _targetAngle) {
		_angle = BaseUtils::normalizeAngle(_angle);
		_targetAngle = _angle;
		return true;
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::loadFile(const char *filename) {
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);

	if (buffer == nullptr) {
		_gameRef->LOG(0, "AdActor3DX::LoadFile failed for file '%s'", filename);
		return false;
	}

	setFilename(filename);

	bool ret = loadBuffer(buffer, true);

	if (!ret) {
		_gameRef->LOG(0, "Error parsing ACTOR3D file '%s'", filename);
	}

	delete[] buffer;

	return ret;
}

TOKEN_DEF_START
	TOKEN_DEF(ACTOR3DX)
	TOKEN_DEF(X)
	TOKEN_DEF(Y)
	TOKEN_DEF(Z)
	TOKEN_DEF(ANGLE)
	TOKEN_DEF(VELOCITY)
	TOKEN_DEF(ANGULAR_VELOCITY)
	TOKEN_DEF(TEMPLATE)
	TOKEN_DEF(NAME)
	TOKEN_DEF(REGISTRABLE)
	TOKEN_DEF(INTERACTIVE)
	TOKEN_DEF(ACTIVE)
	TOKEN_DEF(MODEL)
	TOKEN_DEF(EVENTS)
	TOKEN_DEF(FONT)
	TOKEN_DEF(CURSOR)
	TOKEN_DEF(DROP_TO_FLOOR)
	TOKEN_DEF(SCRIPT)
	TOKEN_DEF(CAPTION)
	TOKEN_DEF(PROPERTY)
	TOKEN_DEF(ANIMATION)
	TOKEN_DEF(EDITOR_PROPERTY)
	TOKEN_DEF(SHADOW_IMAGE)
	TOKEN_DEF(SHADOW_SIZE)
	TOKEN_DEF(SIMPLE_SHADOW)
	TOKEN_DEF(SHADOW_COLOR)
	TOKEN_DEF(SHADOW_MODEL)
	TOKEN_DEF(SHADOW_TYPE)
	TOKEN_DEF(LIGHT_POSITION)
	TOKEN_DEF(SHADOW)
	TOKEN_DEF(SCALE)
	TOKEN_DEF(DRAW_BACKFACES)
	TOKEN_DEF(BLOCKED_REGION)
	TOKEN_DEF(WAYPOINTS)
	TOKEN_DEF(EFFECT_FILE)
	TOKEN_DEF(EFFECT)
	TOKEN_DEF(MATERIAL)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(ACTOR3DX)
		TOKEN_TABLE(X)
		TOKEN_TABLE(Y)
		TOKEN_TABLE(Z)
		TOKEN_TABLE(ANGLE)
		TOKEN_TABLE(VELOCITY)
		TOKEN_TABLE(ANGULAR_VELOCITY)
		TOKEN_TABLE(TEMPLATE)
		TOKEN_TABLE(NAME)
		TOKEN_TABLE(REGISTRABLE)
		TOKEN_TABLE(INTERACTIVE)
		TOKEN_TABLE(ACTIVE)
		TOKEN_TABLE(MODEL)
		TOKEN_TABLE(EVENTS)
		TOKEN_TABLE(FONT)
		TOKEN_TABLE(CURSOR)
		TOKEN_TABLE(DROP_TO_FLOOR)
		TOKEN_TABLE(SCRIPT)
		TOKEN_TABLE(CAPTION)
		TOKEN_TABLE(PROPERTY)
		TOKEN_TABLE(ANIMATION)
		TOKEN_TABLE(EDITOR_PROPERTY)
		TOKEN_TABLE(SHADOW_IMAGE)
		TOKEN_TABLE(SHADOW_SIZE)
		TOKEN_TABLE(SIMPLE_SHADOW)
		TOKEN_TABLE(SHADOW_COLOR)
		TOKEN_TABLE(SHADOW_MODEL)
		TOKEN_TABLE(SHADOW_TYPE)
		TOKEN_TABLE(LIGHT_POSITION)
		TOKEN_TABLE(SHADOW)
		TOKEN_TABLE(SCALE)
		TOKEN_TABLE(DRAW_BACKFACES)
		TOKEN_TABLE(BLOCKED_REGION)
		TOKEN_TABLE(WAYPOINTS)
		TOKEN_TABLE(EFFECT)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ACTOR3DX) {
			_gameRef->LOG(0, "'ACTOR3DX' keyword expected.");
			return false;
		}

		buffer = params;
	}

	delete _xmodel;
	_xmodel = nullptr;
	delete _shadowModel;
	_shadowModel = nullptr;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (!loadFile((char *)params)) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%f", &_posVector._x);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%f", &_posVector._y);
			break;

		case TOKEN_Z:
			parser.scanStr((char *)params, "%f", &_posVector._z);
			break;

		case TOKEN_ANGLE:
			parser.scanStr((char *)params, "%f", &_angle);
			BaseUtils::normalizeAngle(_angle);
			break;

		case TOKEN_SHADOW_SIZE:
			parser.scanStr((char *)params, "%f", &_shadowSize);
			_shadowSize = MAX(_shadowSize, 0.0f);
			break;

		case TOKEN_SIMPLE_SHADOW: {
			bool simpleShadow;
			parser.scanStr((char *)params, "%b", &simpleShadow);
			if (simpleShadow) {
				_shadowType = SHADOW_SIMPLE;
			}

			break;
		}

		case TOKEN_SHADOW_COLOR: {
			int r, g, b, a;
			parser.scanStr((char *)params, "%d,%d,%d,%d", &r, &g, &b, &a);
			_shadowColor = BYTETORGBA(r, g, b, a);

			break;
		}

		case TOKEN_LIGHT_POSITION:
			parser.scanStr((char *)params, "%f,%f,%f", &_shadowLightPos._x, &_shadowLightPos._y, &_shadowLightPos._z);
			break;

		case TOKEN_SHADOW: {
			bool shadowEnabled;
			parser.scanStr((char *)params, "%b", &shadowEnabled);
			if (!shadowEnabled) {
				_shadowType = SHADOW_NONE;
			}

			break;
		}

		case TOKEN_DRAW_BACKFACES:
			parser.scanStr((char *)params, "%b", &_drawBackfaces);
			break;

		case TOKEN_VELOCITY:
			parser.scanStr((char *)params, "%f", &_velocity);
			break;

		case TOKEN_ANGULAR_VELOCITY:
			parser.scanStr((char *)params, "%f", &_angVelocity);
			break;

		case TOKEN_SCALE:
			parser.scanStr((char *)params, "%f", &_scale3D);
			_scale3D /= 100.0f;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_FONT:
			setFont((char *)params);
			break;

		case TOKEN_REGISTRABLE:
		case TOKEN_INTERACTIVE:
			parser.scanStr((char *)params, "%b", &_registrable);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_DROP_TO_FLOOR:
			parser.scanStr((char *)params, "%b", &_dropToFloor);
			break;

		case TOKEN_SHADOW_TYPE: {
			char *typeName = (char *)params;
			if (scumm_stricmp(typeName, "none") == 0) {
				_shadowType = SHADOW_NONE;
			} else if (scumm_stricmp(typeName, "simple") == 0) {
				_shadowType = SHADOW_SIMPLE;
			} else if (scumm_stricmp(typeName, "flat") == 0) {
				_shadowType = SHADOW_FLAT;
			} else if (scumm_stricmp(typeName, "stencil") == 0) {
				_shadowType = SHADOW_STENCIL;
			} else {
				_shadowType = (TShadowType)atoi(typeName);
				if (_shadowType < 0) {
					_shadowType = SHADOW_NONE;
				}
				if (_shadowType > SHADOW_STENCIL) {
					_shadowType = SHADOW_STENCIL;
				}
			}
			break;
		}

		case TOKEN_MODEL:
			if (!_xmodel) {
				_xmodel = new XModel(_gameRef, this);

				if (!_xmodel || !_xmodel->loadFromFile((char *)params)) {
					delete _xmodel;
					_xmodel = nullptr;
					cmd = PARSERR_GENERIC;
				}
			} else {
				if (!_xmodel->mergeFromFile((char *)params)) {
					cmd = PARSERR_GENERIC;
				}
			}
			break;

		case TOKEN_SHADOW_MODEL:
			if (_xmodel) {
				delete _shadowModel;
				_shadowModel = new XModel(_gameRef, this);

				if (!_shadowModel || !_shadowModel->loadFromFile((char *)params, _xmodel)) {
					delete _shadowModel;
					_shadowModel = nullptr;
					cmd = PARSERR_GENERIC;
				}
			} else {
				_gameRef->LOG(0, "Error: a MODEL= line must precede shadow model assignment (file: %s)", getFilename());
			}
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new BaseSprite(_gameRef);
			if (!_cursor || !_cursor->loadFile((char *)params)) {
				delete _cursor;
				_cursor = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_PROPERTY:
			parseProperty((char *)params, false);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty((char *)params, false);
			break;

		case TOKEN_ANIMATION:
			if (_xmodel) {
				_xmodel->parseAnim(params);
			} else {
				_gameRef->LOG(0, "Error: a MODEL= line must precede any animation definitions (file: %s)", getFilename());
			}
			break;

		case TOKEN_EFFECT:
			if (_xmodel)
				parseEffect(params);
			else
				_gameRef->LOG(0, "Error: a MODEL= line must precede any effect definitions (file: %s)", getFilename());
			break;

		case TOKEN_SHADOW_IMAGE:
			if (_shadowImage)
				_gameRef->_surfaceStorage->removeSurface(_shadowImage);
			_shadowImage = nullptr;

			_shadowImage = _gameRef->_surfaceStorage->addSurface((char *)params);
			break;

		case TOKEN_BLOCKED_REGION: {
			delete _blockRegion;
			_blockRegion = nullptr;
			delete _currentBlockRegion;
			_currentBlockRegion = nullptr;
			BaseRegion *rgn = new BaseRegion(_gameRef);
			BaseRegion *crgn = new BaseRegion(_gameRef);
			if (!rgn || !crgn || !rgn->loadBuffer((char *)params, false)) {
				delete rgn;
				delete crgn;
				cmd = PARSERR_GENERIC;
			} else {
				_blockRegion = rgn;
				_currentBlockRegion = crgn;
				_currentBlockRegion->mimic(_blockRegion);
			}
			break;
		}

		case TOKEN_WAYPOINTS: {
			delete _wptGroup;
			_wptGroup = nullptr;
			delete _currentWptGroup;
			_currentWptGroup = nullptr;
			AdWaypointGroup *wpt = new AdWaypointGroup(_gameRef);
			AdWaypointGroup *cwpt = new AdWaypointGroup(_gameRef);
			if (!wpt || !cwpt || !wpt->loadBuffer((char *)params, false)) {
				delete wpt;
				delete cwpt;
				cmd = PARSERR_GENERIC;
			} else {
				_wptGroup = wpt;
				_currentWptGroup = cwpt;
				_currentWptGroup->mimic(_wptGroup);
			}
			break;
		}

		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in ACTOR3DX definition");
		return false;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading ACTOR3DX definition");
		return false;
	}

	if (!_xmodel) {
		_gameRef->LOG(0, "Error: No model has been loaded for 3D actor");
		return false;
	}

	_state = _nextState = STATE_READY;

	return true;
}

//////////////////////////////////////////////////////////////////////////
float AdActor3DX::dirToAngle(TDirection dir) {
	switch (dir) {
	case DI_UP:
		return 180.0f;
	case DI_UPRIGHT:
		return 225.0f;
	case DI_RIGHT:
		return 270.0f;
	case DI_DOWNRIGHT:
		return 315.0f;
	case DI_DOWN:
		return 0.0f;
	case DI_DOWNLEFT:
		return 45.0f;
	case DI_LEFT:
		return 90.0f;
	case DI_UPLEFT:
		return 135.0f;
	case DI_NONE:
		return -1.0f;
	default:
		return 0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////
TDirection AdActor3DX::angleToDir(float angle) {
	if (angle >= 337.0f || angle < 22.0f)
		return DI_DOWN;
	if (angle >= 22.0f && angle < 67.0f)
		return DI_DOWNLEFT;
	if (angle >= 67.0f && angle < 112.0f)
		return DI_LEFT;
	if (angle >= 112.0f && angle < 157.0f)
		return DI_UPLEFT;
	if (angle >= 157.0f && angle < 202.0f)
		return DI_UP;
	if (angle >= 202.0f && angle < 247.0f)
		return DI_UPRIGHT;
	if (angle >= 247.0f && angle < 292.0f)
		return DI_RIGHT;
	if (angle >= 292.0f && angle < 337.0f)
		return DI_DOWNRIGHT;

	return DI_NONE;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::playAnim3DX(const char *name, bool setState) {
	return playAnim3DX(0, name, setState);
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::playAnim3DX(int channel, const char *name, bool setState) {
	if (!_xmodel) {
		return false;
	}

	bool res = _xmodel->playAnim(channel, name, _defaultTransTime, true, _defaultStopTransTime);
	if (res && setState) {
		_state = STATE_PLAYING_ANIM;
		_stateAnimChannel = channel;
	}

	return res;
}

//////////////////////////////////////////////////////////////////////////
void AdActor3DX::talk(const char *text, const char *sound, uint32 duration, const char *stances, TTextAlign align) {
	AdObject::talk(text, sound, duration, stances, align);
}

//////////////////////////////////////////////////////////////////////////
int32 AdActor3DX::getHeight() {
	if (!_xmodel) {
		return 0;
	} else {
		return _posY - _xmodel->_boundingRect.top - 5;
	}
}

//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// PlayAnim / PlayAnimAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "PlayAnim") == 0 || strcmp(name, "PlayAnimAsync") == 0) {
		bool async = strcmp(name, "PlayAnimAsync") == 0;
		stack->correctParams(1);
		if (!playAnim3DX(stack->pop()->getString(), true)) {
			stack->pushBool(false);
		} else {
			if (!async)
				script->waitFor(this);
			stack->pushBool(true);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopAnim") == 0) {
		stack->correctParams(1);
		int transTime = stack->pop()->getInt(_defaultStopTransTime);
		bool ret = false;
		if (_xmodel) {
			ret = _xmodel->stopAnim(0, transTime);
		}
		stack->pushBool(ret);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopAnimChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopAnimChannel") == 0) {
		stack->correctParams(2);
		int channel = stack->pop()->getInt();
		int transTime = stack->pop()->getInt();
		bool ret = false;
		if (_xmodel) {
			ret = _xmodel->stopAnim(channel, transTime);
		}

		stack->pushBool(ret);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayAnimChannel / PlayAnimChannelAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayAnimChannel") == 0 || strcmp(name, "PlayAnimChannelAsync") == 0) {
		bool async = strcmp(name, "PlayAnimChannelAsync") == 0;

		stack->correctParams(2);
		int channel = stack->pop()->getInt();
		const char *animName = stack->pop()->getString();
		if (!playAnim3DX(channel, animName, !async)) {
			stack->pushBool(false);
		} else {
			if (!async) {
				script->waitFor(this);
			}

			stack->pushBool(true);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAnimPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsAnimPlaying") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		const char *animName;
		if (val->isNULL()) {
			animName = nullptr;
		} else {
			animName = val->getString();
		}

		if (_xmodel) {
			stack->pushBool(_xmodel->isAnimPending(0, animName));
		} else {
			stack->pushBool(false);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsAnimChannelPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsAnimChannelPlaying") == 0) {
		stack->correctParams(2);
		int channel = stack->pop()->getInt(0);
		ScValue *val = stack->pop();
		const char *animName;
		if (val->isNULL()) {
			animName = nullptr;
		} else {
			animName = val->getString();
		}

		if (_xmodel) {
			stack->pushBool(_xmodel->isAnimPending(channel, animName));
		} else {
			stack->pushBool(false);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddAttachment / AddMesh
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddAttachment") == 0 || strcmp(name, "AddMesh") == 0) {
		if (strcmp(name, "AddMesh") == 0)
			_gameRef->LOG(0, "Warning: AddMesh is now obsolete, use AddAttachment");

		stack->correctParams(3);
		const char *filename = stack->pop()->getString();
		const char *attachName = stack->pop()->getString();
		const char *boneName = stack->pop()->getString();

		if (!_xmodel) {
			stack->pushBool(false);
		} else {
			if (!_xmodel->getBoneMatrix(boneName)) {
				script->runtimeError("Bone '%s' cannot be found", boneName);
				stack->pushBool(false);
			} else {
				AdAttach3DX *at = new AdAttach3DX(_gameRef, this);
				if (!at || !at->init(filename, attachName, boneName)) {
					script->runtimeError("Error adding attachment");
					delete at;
					stack->pushBool(false);
				} else {
					bool isSet = false;
					for (uint32 i = 0; i < _attachments.size(); i++) {
						if (scumm_stricmp(_attachments[i]->getName(), attachName) == 0) {
							delete _attachments[i];
							_attachments[i] = at;
							isSet = true;
							break;
						}
					}
					if (!isSet) {
						_attachments.add(at);
					}

					stack->pushBool(true);
				}
			}
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveAttachment / RemoveMesh
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveAttachment") == 0 || strcmp(name, "RemoveMesh") == 0) {
		if (strcmp(name, "RemoveMesh") == 0) {
			_gameRef->LOG(0, "Warning: RemoveMesh is now obsolete, use RemoveAttachment");
		}

		stack->correctParams(1);
		const char *attachmentName = stack->pop()->getString();

		if (!_xmodel) {
			stack->pushBool(false);
		} else {
			bool isFound = false;
			for (uint32 i = 0; i < _attachments.size(); i++) {
				if (scumm_stricmp(_attachments[i]->getName(), attachmentName) == 0) {
					delete _attachments[i];
					_attachments.remove_at(i);
					isFound = true;
					break;
				}
			}
			stack->pushBool(isFound);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAttachment
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAttachment") == 0) {
		stack->correctParams(1);
		const char *attachmentName = stack->pop()->getString();

		if (!_xmodel) {
			stack->pushNULL();
		} else {
			bool isFound = false;
			for (uint32 i = 0; i < _attachments.size(); i++) {
				if (scumm_stricmp(_attachments[i]->getName(), attachmentName) == 0) {
					stack->pushNative(_attachments[i], true);
					isFound = true;
					break;
				}
			}
			if (!isFound)
				stack->pushNULL();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoTo3D / GoTo3DAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoTo3D") == 0 || strcmp(name, "GoTo3DAsync") == 0) {
		stack->correctParams(3);
		DXVector3 pos;
		pos._x = stack->pop()->getFloat();
		pos._y = stack->pop()->getFloat();
		pos._z = stack->pop()->getFloat();
		goTo3D(pos);

		if (strcmp(name, "GoTo3DAsync") != 0) {
			script->waitForExclusive(this);
		}

		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoTo / GoToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoTo") == 0 || strcmp(name, "GoToAsync") == 0) {
		stack->correctParams(2);
		int x = stack->pop()->getInt();
		int y = stack->pop()->getInt();
		AdGame *adGame = (AdGame *)_gameRef;

		if (isGoToNeeded(x, y)) {
			if (adGame->_scene && adGame->_scene->_2DPathfinding) {
				goTo2D(x, y);

				if (strcmp(name, "GoToAsync") != 0) {
					script->waitForExclusive(this);
				}
			} else {
				if (adGame->_scene && adGame->_scene->_geom) {
					DXVector3 pos;
					if (adGame->_scene->_geom->convert2Dto3DTolerant(x, y, &pos)) {
						goTo3D(pos);
						if (strcmp(name, "GoToAsync") != 0) {
							script->waitForExclusive(this);
						}
					}
				}
			}
		} else {
			if (_path2D) {
				_path2D->reset();
			}
			if (_path3D) {
				_path3D->reset();
			}
		}

		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToObject / GoToObjectAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoToObject") == 0 || strcmp(name, "GoToObjectAsync") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (!val->isNative()) {
			script->runtimeError("actor.%s method accepts an entity reference only", name);
			stack->pushNULL();
			return true;
		}
		AdObject *obj = (AdObject *)val->getNative();

		if (!obj || obj->getType() != OBJECT_ENTITY) {
			script->runtimeError("actor.%s method accepts an entity reference only", name);
			stack->pushNULL();
			return true;
		}

		AdEntity *ent = (AdEntity *)obj;

		AdGame *adGame = (AdGame *)_gameRef;

		bool goToNeeded = true;

		if (ent->getWalkToX() == 0 && ent->getWalkToY() == 0) {
			goToNeeded = isGoToNeeded(ent->_posX, ent->_posY);
		} else {
			goToNeeded = isGoToNeeded(ent->getWalkToX(), ent->getWalkToY());
		}

		if (!goToNeeded) {
			// no goto needed, but we still want to turn
			if (ent->getWalkToX() != 0 || ent->getWalkToY() != 0) {
				turnTo(dirToAngle(ent->getWalkToDir()));
				if (strcmp(name, "GoToObjectAsync") != 0) {
					script->waitForExclusive(this);
				}
			}

			if (_path2D) {
				_path2D->reset();
			}
			if (_path3D) {
				_path3D->reset();
			}

			stack->pushNULL();
			return true;
		}

		if (adGame->_scene->_2DPathfinding) {
			if (ent->getWalkToX() == 0 && ent->getWalkToY() == 0) {
				goTo2D(ent->_posX, ent->_posY);
			} else {
				goTo2D(ent->getWalkToX(), ent->getWalkToY(), dirToAngle(ent->getWalkToDir()));
			}

			if (strcmp(name, "GoToObjectAsync") != 0) {
				script->waitForExclusive(this);
			}
		} else {
			if (adGame->_scene->_geom) {
				DXVector3 pos;

				if (adGame->_scene->_geom->convert2Dto3DTolerant(ent->getWalkToX(), ent->getWalkToY(), &pos)) {
					if (ent->getWalkToX() == 0 && ent->getWalkToY() == 0) {
						goTo3D(pos);
					} else {
						goTo3D(pos, dirToAngle(ent->getWalkToDir()));
					}

					if (strcmp(name, "GoToObjectAsync") != 0) {
						script->waitForExclusive(this);
					}
				}
			}
		}
		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnTo / TurnToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnTo") == 0 || strcmp(name, "TurnToAsync") == 0) {
		stack->correctParams(1);
		int dir;
		ScValue *val = stack->pop();

		float angle = 0.0;

		// turn to object?
		if (val->isNative() && _gameRef->validObject((BaseObject *)val->getNative())) {
			BaseObject *obj = (BaseObject *)val->getNative();
			DXVector3 objPos;
			((AdGame *)_gameRef)->_scene->_geom->convert2Dto3D(obj->_posX, obj->_posY, &objPos);
			angle = radToDeg(-atan2(objPos._z - _posVector._z, objPos._x - _posVector._x)) - 90;
		} else {
			// otherwise turn to direction
			dir = val->getInt();
			angle = dirToAngle((TDirection)dir);
		}

		if (_path2D) {
			_path2D->reset();
		}

		if (_path3D) {
			_path3D->reset();
		}

		turnTo(angle);
		if (strcmp(name, "TurnToAsync") != 0) {
			script->waitForExclusive(this);
		}

		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnToAngle / TurnToAngleAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnToAngle") == 0 || strcmp(name, "TurnToAngleAsync") == 0) {
		stack->correctParams(1);
		float angle = stack->pop()->getFloat();

		if (_path2D) {
			_path2D->reset();
		}

		if (_path3D) {
			_path3D->reset();
		}

		turnTo(angle);
		if (strcmp(name, "TurnToAngleAsync") != 0) {
			script->waitForExclusive(this);
		}

		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsWalking") == 0) {
		stack->correctParams(0);
		stack->pushBool(_state == STATE_FOLLOWING_PATH);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectWalk / DirectWalkBack
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DirectWalk") == 0 || strcmp(name, "DirectWalkBack") == 0) {
		stack->correctParams(2);

		ScValue *valVelocity = stack->pop();
		ScValue *valAnim = stack->pop();

		if (!valVelocity->isNULL()) {
			_directWalkVelocity = valVelocity->getFloat();
		} else {
			_directWalkVelocity = 0.0f;
		}

		if (!valAnim->isNULL()) {
			_directWalkAnim = valAnim->getString();
		}

		_state = STATE_DIRECT_CONTROL;

		if (strcmp(name, "DirectWalk") == 0) {
			_directWalkMode = DIRECT_WALK_FW;
		} else {
			_directWalkMode = DIRECT_WALK_BK;
		}

		stack->pushBool(true);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectWalkStop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DirectWalkStop") == 0) {
		stack->correctParams(0);
		_directWalkMode = DIRECT_WALK_NONE;
		stack->pushBool(true);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectTurnLeft / DirectTurnRight
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DirectTurnLeft") == 0 || strcmp(name, "DirectTurnRight") == 0) {
		stack->correctParams(2);

		ScValue *valVelocity = stack->pop();
		ScValue *valAnim = stack->pop();

		if (!valVelocity->isNULL()) {
			_directTurnVelocity = valVelocity->getFloat();
		} else {
			_directTurnVelocity = 0.0f;
		}

		if (!valAnim->isNULL()) {
			_directTurnAnim = valAnim->getString();
		}

		_state = STATE_DIRECT_CONTROL;

		if (strcmp(name, "DirectTurnLeft") == 0) {
			_directTurnMode = DIRECT_TURN_CCW;
		} else {
			_directTurnMode = DIRECT_TURN_CW;
		}

		stack->pushBool(true);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectTurnStop
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DirectTurnStop") == 0) {
		stack->correctParams(0);
		_directTurnMode = DIRECT_TURN_NONE;
		stack->pushBool(true);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetTexture
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetTexture") == 0) {
		stack->correctParams(2);
		const char *materialName = stack->pop()->getString();
		const char *textureFilename = stack->pop()->getString();

		if (_xmodel && _xmodel->setMaterialSprite(materialName, textureFilename)) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetTheoraTexture
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetTheoraTexture") == 0) {
		stack->correctParams(2);
		const char *materialName = stack->pop()->getString();
		const char *theoraFilename = stack->pop()->getString();

		if (_xmodel && _xmodel->setMaterialTheora(materialName, theoraFilename)) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEffect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEffect") == 0) {
		stack->correctParams(2);
		/*const char *materialName =*/ stack->pop()->getString();
		/*const char *effectFilename =*/ stack->pop()->getString();

		warning("AdActor3DX::scCallMethod D3DX effects are not supported");
		//if (_xmodel && _xmodel->setMaterialEffect(materialName, effectFilename)) {
		if (_xmodel) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveEffect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveEffect") == 0) {
		stack->correctParams(1);
		/*const char *materialName =*/ stack->pop()->getString();
		stack->pop();

		warning("AdActor3DX::scCallMethod D3DX effects are not supported");
		// if (_xmodel && _xodel->removeMaterialEffect(materialName)) {
		if (_xmodel) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEffectParam
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEffectParam") == 0) {
		stack->correctParams(3);
		/*const char *materialName =*/ stack->pop()->getString();
		/*const char *paramName =*/ stack->pop()->getString();
		/*ScValue *val =*/ stack->pop();

		warning("AdActor3DX::scCallMethod D3DX effects are not supported");
		// if (_xmodel && _xmodel->setMaterialEffectParam(materialName, paramName, val)) {
		if (_xmodel) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEffectParamVector
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEffectParamVector") == 0) {
		stack->correctParams(6);
		/*const char *materialName =*/ stack->pop()->getString();
		/*const char *paramName =*/ stack->pop()->getString();
		/*float x =*/ stack->pop()->getFloat();
		/*float y =*/ stack->pop()->getFloat();
		/*float z =*/ stack->pop()->getFloat();
		/*float w =*/ stack->pop()->getFloat();

		//if (_xmodel && _xmodel->setMaterialEffectParam(materialName, paramName, DXVector4(x, y, z, w))) {
		warning("AdActor3DX::scCallMethod D3DX effects are not supported");
		if (_xmodel) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetEffectParamColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetEffectParamColor") == 0) {
		stack->correctParams(3);
		/*const char *materialName =*/ stack->pop()->getString();
		/*const char *paramName =*/ stack->pop()->getString();
		/*uint32 color =*/ stack->pop()->getInt();

		//float r = RGBCOLGetR(color) / 255.0f;
		//float g = RGBCOLGetG(color) / 255.0f;
		//float b = RGBCOLGetB(color) / 255.0f;
		//float a = RGBCOLGetA(color) / 255.0f;

		//if (_xmodel && _xmodel->setMaterialEffectParam(materialName, paramName, DXVector4(r, g, b, a))) {
		warning("AdActor3DX::scCallMethod D3DX effects are not supported");
		if (_xmodel) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// MergeAnims
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MergeAnims") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();

		stack->pushBool(mergeAnimations(filename));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadAnim") == 0) {
		stack->correctParams(1);
		const char *animName = stack->pop()->getString();

		stack->pushBool(unloadAnimation(animName));
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetAnimTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetAnimTransitionTime") == 0) {
		stack->correctParams(3);
		const char *animFrom = stack->pop()->getString();
		const char *animTo = stack->pop()->getString();
		int time = stack->pop()->getInt();

		bool found = false;
		for (uint32 i = 0; i < _transitionTimes.size(); i++) {
			BaseAnimationTransitionTime *trans = _transitionTimes[i];
			if (!trans->_animFrom.empty() && !trans->_animTo.empty() && trans->_animFrom.compareToIgnoreCase(animFrom) == 0 && trans->_animTo.compareToIgnoreCase(animTo) == 0) {
				found = true;
				if (time < 0) {
					delete trans;
					_transitionTimes.remove_at(i);
				} else
					trans->_time = (uint32)time;
				break;
			}
		}

		if (!found && time >= 0) {
			BaseAnimationTransitionTime *trans = new BaseAnimationTransitionTime(animFrom, animTo, (uint32)time);
			_transitionTimes.add(trans);
		}

		stack->pushNULL();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetAnimTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetAnimTransitionTime") == 0) {
		stack->correctParams(2);
		const char *animFrom = stack->pop()->getString();
		const char *animTo = stack->pop()->getString();

		int time = -1;
		for (uint32 i = 0; i < _transitionTimes.size(); i++) {
			BaseAnimationTransitionTime *trans = _transitionTimes[i];

			if (!trans->_animFrom.empty() && !trans->_animTo.empty() && trans->_animFrom.compareToIgnoreCase(animFrom) == 0 && trans->_animTo.compareToIgnoreCase(animTo) == 0) {
				time = trans->_time;
				break;
			}
		}
		stack->pushInt(time);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateParticleEmitterBone
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateParticleEmitterBone") == 0) {
		stack->correctParams(4);
		const char *boneName = stack->pop()->getString();
		float offsetX = stack->pop()->getFloat();
		float offsetY = stack->pop()->getFloat();
		float offsetZ = stack->pop()->getFloat();

		PartEmitter *emitter = createParticleEmitter(boneName, DXVector3(offsetX, offsetY, offsetZ));
		if (emitter) {
			stack->pushNative(_partEmitter, true);
		} else {
			stack->pushNULL();
		}

		return true;
	} else {
		return AdObject3D::scCallMethod(script, stack, thisStack, name);
	}
}

//////////////////////////////////////////////////////////////////////////
ScValue *AdActor3DX::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("actor3dx");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TalkAnimName") {
		_scValue->setString(_talkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimChannel
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TalkAnimChannel") {
		_scValue->setInt(_talkAnimChannel);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (name == "WalkAnimName") {
		_scValue->setString(_talkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (name == "IdleAnimName") {
		_scValue->setString(_idleAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TurnLeftAnimName") {
		_scValue->setString(_turnLeftAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (name == "TurnRightAnimName") {
		_scValue->setString(_turnRightAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectionAngle / DirAngle
	//////////////////////////////////////////////////////////////////////////
	else if (name == "DirectionAngle" || name == "DirAngle") {
		_scValue->setFloat(_angle);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Direction") {
		_scValue->setInt(angleToDir(_angle));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AnimTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AnimTransitionTime") {
		_scValue->setInt(_defaultTransTime);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AnimStopTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AnimStopTransitionTime") {
		_scValue->setInt(_defaultStopTransTime);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToTolerance
	//////////////////////////////////////////////////////////////////////////
	else if (name == "GoToTolerance") {
		_scValue->setInt(_goToTolerance);
		return _scValue;
	}

	else {
		return AdObject3D::scGetProperty(name);
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "TalkAnimName") == 0) {
		if (value->isNULL()) {
			_talkAnimName = "talk";
		} else {
			_talkAnimName = value->getString();
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimChannel
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkAnimChannel") == 0) {
		_talkAnimChannel = value->getInt();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkAnimName") == 0) {
		if (value->isNULL()) {
			_walkAnimName = "walk";
		} else {
			_walkAnimName = value->getString();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IdleAnimName") == 0) {
		if (value->isNULL()) {
			_idleAnimName = "idle";
		} else {
			_idleAnimName = value->getString();
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnLeftAnimName") == 0) {
		if (value->isNULL()) {
			_turnLeftAnimName = "turnleft";
		} else {
			_turnLeftAnimName = value->getString();
		}

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnRightAnimName") == 0) {
		if (value->isNULL()) {
			_turnRightAnimName = "turnright";
		} else {
			_turnRightAnimName = value->getString();
		}
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// DirectionAngle / DirAngle
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DirectionAngle") == 0 || strcmp(name, "DirAngle") == 0) {
		_angle = value->getFloat();
		BaseUtils::normalizeAngle(_angle);
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Direction") == 0) {
		_angle = dirToAngle((TDirection)value->getInt());
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// AnimTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AnimTransitionTime") == 0) {
		_defaultTransTime = value->getInt();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// AnimStopTransitionTime
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AnimStopTransitionTime") == 0) {
		_defaultStopTransTime = value->getInt();
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToTolerance
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoToTolerance") == 0) {
		_goToTolerance = value->getInt();
		return true;
	}

	else {
		return AdObject3D::scSetProperty(name, value);
	}
}

//////////////////////////////////////////////////////////////////////////
const char *AdActor3DX::scToString() {
	return "[actor3dx object]";
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::persist(BasePersistenceManager *persistMgr) {
	AdObject3D::persist(persistMgr);

	persistMgr->transferPtr(TMEMBER(_path3D));
	persistMgr->transferPtr(TMEMBER(_path2D));
	persistMgr->transferAngle(TMEMBER(_targetAngle));
	persistMgr->transferVector3d(TMEMBER(_targetPoint3D));
	persistMgr->transferPtr(TMEMBER(_targetPoint2D));
	persistMgr->transferBool(TMEMBER(_turningLeft));
	persistMgr->transferFloat(TMEMBER(_afterWalkAngle));

	persistMgr->transferString(TMEMBER(_talkAnimName));
	persistMgr->transferString(TMEMBER(_idleAnimName));
	persistMgr->transferString(TMEMBER(_walkAnimName));
	persistMgr->transferString(TMEMBER(_turnLeftAnimName));
	persistMgr->transferString(TMEMBER(_turnRightAnimName));

	// direct controls
	persistMgr->transferSint32(TMEMBER_INT(_directWalkMode));
	persistMgr->transferSint32(TMEMBER_INT(_directTurnMode));
	persistMgr->transferString(TMEMBER(_directWalkAnim));
	persistMgr->transferString(TMEMBER(_directTurnAnim));
	persistMgr->transferFloat(TMEMBER(_directWalkVelocity));
	persistMgr->transferFloat(TMEMBER(_directTurnVelocity));

	// new for X
	persistMgr->transferUint32(TMEMBER(_defaultTransTime));
	_attachments.persist(persistMgr);
	persistMgr->transferSint32(TMEMBER(_stateAnimChannel));

	persistMgr->transferSint32(TMEMBER(_goToTolerance));

	persistMgr->transferUint32(TMEMBER(_defaultStopTransTime));

	if (persistMgr->getIsSaving()) {
		int32 numItems = _transitionTimes.size();
		persistMgr->transferSint32(TMEMBER(numItems));
		for (uint32 i = 0; i < _transitionTimes.size(); i++) {
			_transitionTimes[i]->persist(persistMgr);
		}
	} else {
		int32 numItems = _transitionTimes.size();
		persistMgr->transferSint32(TMEMBER(numItems));
		for (int i = 0; i < numItems; i++) {
			BaseAnimationTransitionTime *trans = new BaseAnimationTransitionTime();
			trans->persist(persistMgr);
			_transitionTimes.add(trans);
		}
	}

	persistMgr->transferSint32(TMEMBER(_talkAnimChannel));

	persistMgr->transferString(TMEMBER(_partBone));
	persistMgr->transferVector3d(TMEMBER(_partOffset));

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::invalidateDeviceObjects() {
	if (_xmodel)
		_xmodel->invalidateDeviceObjects();
	if (_shadowModel)
		_shadowModel->invalidateDeviceObjects();

	for (uint32 i = 0; i < _attachments.size(); i++) {
		_attachments[i]->invalidateDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::restoreDeviceObjects() {
	if (_xmodel) {
		_xmodel->restoreDeviceObjects();
	}

	if (_shadowModel) {
		_shadowModel->restoreDeviceObjects();
	}

	for (uint32 i = 0; i < _attachments.size(); i++) {
		_attachments[i]->restoreDeviceObjects();
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::mergeAnimations(const char *filename) {
	if (!_xmodel) {
		return false;
	}

	bool res = _xmodel->mergeFromFile(filename);
	if (!res) {
		_gameRef->LOG(res, "Error: MergeAnims failed for file '%s'", filename);
		return res;
	}

	AnsiString path = PathUtil::getDirectoryName(filename);
	AnsiString name = PathUtil::getFileNameWithoutExtension(filename);
	AnsiString animExtFile = PathUtil::combine(path, name + ".anim");

	if (BaseFileManager::getEngineInstance()->hasFile(animExtFile)) {
		return mergeAnimations2(animExtFile.c_str());
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::mergeAnimations2(const char *filename) {
	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END

	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		return false;
	}

	byte *bufferOrig = buffer;

	byte *params;
	int cmd;
	BaseParser parser;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ANIMATION:
			if (!_xmodel->parseAnim(params)) {
				cmd = PARSERR_GENERIC;
			}
		}
	}
	delete[] bufferOrig;

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in animation definition file");
		return false;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading animation definition file");
		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::unloadAnimation(const char *animName) {
	if (_xmodel) {
		return _xmodel->unloadAnimation(animName);
	} else {
		return false;
	}
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::isGoToNeeded(int x, int y) {
	if (ABS(x - _posX) <= _goToTolerance && ABS(y - _posY) <= _goToTolerance) {
		return false;
	} else {
		return true;
	}
}

//////////////////////////////////////////////////////////////////////////
uint32 AdActor3DX::getAnimTransitionTime(char *from, char *to) {
	for (uint32 i = 0; i < _transitionTimes.size(); i++) {
		BaseAnimationTransitionTime *trans = _transitionTimes[i];
		if (!trans->_animFrom.empty() && !trans->_animTo.empty() && trans->_animFrom.compareToIgnoreCase(from) == 0 && trans->_animTo.compareToIgnoreCase(to) == 0) {
			return trans->_time;
		}
	}

	return _defaultTransTime;
}

//////////////////////////////////////////////////////////////////////////
PartEmitter *AdActor3DX::createParticleEmitter(bool followParent, int offsetX, int offsetY) {
	_partBone.clear();
	return AdObject::createParticleEmitter(followParent, offsetX, offsetY);
}

//////////////////////////////////////////////////////////////////////////
PartEmitter *AdActor3DX::createParticleEmitter(const char *boneName, DXVector3 offset) {
	_partBone = boneName;
	_partOffset = offset;
	return AdObject::createParticleEmitter(true);
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::updatePartEmitter() {
	if (!_partEmitter) {
		return false;
	}

	if (_partBone.empty()) {
		return AdObject::updatePartEmitter();
	}

	AdGame *adGame = (AdGame *)_gameRef;

	if (!adGame->_scene || !adGame->_scene->_geom) {
		return false;
	}

	DXVector3 bonePos;
	getBonePosition3D(_partBone.c_str(), &bonePos, &_partOffset);
	int32 x = 0, y = 0;
	static_cast<AdGame *>(_gameRef)->_scene->_geom->convert3Dto2D(&bonePos, &x, &y);

	_partEmitter->_posX = x - _gameRef->_renderer->_drawOffsetX;
	_partEmitter->_posY = y - _gameRef->_renderer->_drawOffsetY;

	return _partEmitter->update();
}

//////////////////////////////////////////////////////////////////////////
bool AdActor3DX::parseEffect(byte *buffer) {
	warning("AdActor3DX::parseEffect D3DX effect are not implemented");

	TOKEN_TABLE_START(commands)
		TOKEN_TABLE(MATERIAL)
		TOKEN_TABLE(EFFECT_FILE)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	char *effectFile = nullptr;
	char *material = nullptr;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_EFFECT_FILE:
			BaseUtils::setString(&effectFile, (char *)params);
			break;

		case TOKEN_MATERIAL:
			BaseUtils::setString(&material, (char *)params);
			break;
		}
	}

	if (cmd != PARSERR_EOF) {
		return false;
	}

	if (effectFile && material) {
		// TODO: Implement
	}

	delete[] effectFile;
	delete[] material;

	return true;
}

} // namespace Wintermute
