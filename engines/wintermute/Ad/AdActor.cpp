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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#include "engines/wintermute/dcgf.h"
#include "engines/wintermute/dctypes.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Ad/AdActor.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Ad/AdSpriteSet.h"
#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Ad/AdPath.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/wintermute.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdActor, false)


//////////////////////////////////////////////////////////////////////////
CAdActor::CAdActor(CBGame *inGame): CAdTalkHolder(inGame) {
	_path = new CAdPath(Game);

	_type = OBJECT_ACTOR;
	_dir = DI_LEFT;

	_walkSprite = NULL;
	_standSprite = NULL;
	_turnLeftSprite = NULL;
	_turnRightSprite = NULL;

	_targetPoint = new CBPoint;
	_afterWalkDir = DI_NONE;

	_animSprite2 = NULL;

	setDefaultAnimNames();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::setDefaultAnimNames() {
	_talkAnimName = "talk";
	_idleAnimName = "idle";
	_walkAnimName = "walk";
	_turnLeftAnimName = "turnleft";
	_turnRightAnimName = "turnright";
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
CAdActor::~CAdActor() {
	delete _path;
	delete _targetPoint;
	_path = NULL;
	_targetPoint = NULL;

	delete _walkSprite;
	delete _standSprite;
	delete _turnLeftSprite;
	delete _turnRightSprite;
	_walkSprite = NULL;
	_standSprite = NULL;
	_turnLeftSprite = NULL;
	_turnRightSprite = NULL;

	_animSprite2 = NULL; // ref only

	for (int i = 0; i < _talkSprites.GetSize(); i++) {
		delete _talkSprites[i];
	}
	_talkSprites.RemoveAll();

	for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
		delete _talkSpritesEx[i];
	}
	_talkSpritesEx.RemoveAll();

	for (int i = 0; i < _anims.GetSize(); i++) {
		delete _anims[i];
		_anims[i] = NULL;
	}
	_anims.RemoveAll();

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdActor::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing ACTOR file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ACTOR)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(NAME)
TOKEN_DEF(SCALABLE)
TOKEN_DEF(REGISTRABLE)
TOKEN_DEF(INTERACTIVE)
TOKEN_DEF(SHADOWABLE)
TOKEN_DEF(COLORABLE)
TOKEN_DEF(ACTIVE)
TOKEN_DEF(WALK)
TOKEN_DEF(STAND)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(TURN_LEFT)
TOKEN_DEF(TURN_RIGHT)
TOKEN_DEF(EVENTS)
TOKEN_DEF(FONT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(SOUND_VOLUME)
TOKEN_DEF(SOUND_PANNING)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(BLOCKED_REGION)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(IGNORE_ITEMS)
TOKEN_DEF(ROTABLE)
TOKEN_DEF(ROTATABLE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(SCALE)
TOKEN_DEF(RELATIVE_SCALE)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(ANIMATION)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ACTOR)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(SCALABLE)
	TOKEN_TABLE(REGISTRABLE)
	TOKEN_TABLE(INTERACTIVE)
	TOKEN_TABLE(SHADOWABLE)
	TOKEN_TABLE(COLORABLE)
	TOKEN_TABLE(ACTIVE)
	TOKEN_TABLE(WALK)
	TOKEN_TABLE(STAND)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(TURN_LEFT)
	TOKEN_TABLE(TURN_RIGHT)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(SOUND_VOLUME)
	TOKEN_TABLE(SOUND_PANNING)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(BLOCKED_REGION)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(IGNORE_ITEMS)
	TOKEN_TABLE(ROTABLE)
	TOKEN_TABLE(ROTATABLE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(RELATIVE_SCALE)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ACTOR) {
			Game->LOG(0, "'ACTOR' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	CAdGame *adGame = (CAdGame *)Game;
	CAdSpriteSet *spr = NULL;
	int ar = 0, ag = 0, ab = 0, alpha = 0;
	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_X:
			parser.scanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.scanStr((char *)params, "%d", &_posY);
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_FONT:
			SetFont((char *)params);
			break;

		case TOKEN_SCALABLE:
			parser.scanStr((char *)params, "%b", &_zoomable);
			break;

		case TOKEN_ROTABLE:
		case TOKEN_ROTATABLE:
			parser.scanStr((char *)params, "%b", &_rotatable);
			break;

		case TOKEN_REGISTRABLE:
		case TOKEN_INTERACTIVE:
			parser.scanStr((char *)params, "%b", &_registrable);
			break;

		case TOKEN_SHADOWABLE:
		case TOKEN_COLORABLE:
			parser.scanStr((char *)params, "%b", &_shadowable);
			break;

		case TOKEN_ACTIVE:
			parser.scanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_WALK:
			delete _walkSprite;
			_walkSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, adGame->_texWalkLifeTime, CACHE_HALF))) cmd = PARSERR_GENERIC;
			else _walkSprite = spr;
			break;

		case TOKEN_TALK:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, adGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSprites.Add(spr);
			break;

		case TOKEN_TALK_SPECIAL:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, adGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSpritesEx.Add(spr);
			break;

		case TOKEN_STAND:
			delete _standSprite;
			_standSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, adGame->_texStandLifeTime))) cmd = PARSERR_GENERIC;
			else _standSprite = spr;
			break;

		case TOKEN_TURN_LEFT:
			delete _turnLeftSprite;
			_turnLeftSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true))) cmd = PARSERR_GENERIC;
			else _turnLeftSprite = spr;
			break;

		case TOKEN_TURN_RIGHT:
			delete _turnRightSprite;
			_turnRightSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true))) cmd = PARSERR_GENERIC;
			else _turnRightSprite = spr;
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_SOUND_VOLUME:
			parser.scanStr((char *)params, "%d", &_sFXVolume);
			break;

		case TOKEN_SCALE: {
			int s;
			parser.scanStr((char *)params, "%d", &s);
			_scale = (float)s;

		}
		break;

		case TOKEN_RELATIVE_SCALE: {
			int s;
			parser.scanStr((char *)params, "%d", &s);
			_relativeScale = (float)s;

		}
		break;

		case TOKEN_SOUND_PANNING:
			parser.scanStr((char *)params, "%b", &_autoSoundPanning);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
			break;

		case TOKEN_BLOCKED_REGION: {
			delete _blockRegion;
			delete _currentBlockRegion;
			_blockRegion = NULL;
			_currentBlockRegion = NULL;
			CBRegion *rgn = new CBRegion(Game);
			CBRegion *crgn = new CBRegion(Game);
			if (!rgn || !crgn || FAILED(rgn->loadBuffer(params, false))) {
				delete _blockRegion;
				delete _currentBlockRegion;
				_blockRegion = NULL;
				_currentBlockRegion = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				_blockRegion = rgn;
				_currentBlockRegion = crgn;
				_currentBlockRegion->mimic(_blockRegion);
			}
		}
		break;

		case TOKEN_WAYPOINTS: {
			delete _wptGroup;
			delete _currentWptGroup;
			_wptGroup = NULL;
			_currentWptGroup = NULL;
			CAdWaypointGroup *wpt = new CAdWaypointGroup(Game);
			CAdWaypointGroup *cwpt = new CAdWaypointGroup(Game);
			if (!wpt || !cwpt || FAILED(wpt->loadBuffer(params, false))) {
				delete _wptGroup;
				delete _currentWptGroup;
				_wptGroup = NULL;
				_currentWptGroup = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				_wptGroup = wpt;
				_currentWptGroup = cwpt;
				_currentWptGroup->mimic(_wptGroup);
			}
		}
		break;

		case TOKEN_IGNORE_ITEMS:
			parser.scanStr((char *)params, "%b", &_ignoreItems);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr((char *)params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		case TOKEN_ANIMATION: {
			CAdSpriteSet *Anim = new CAdSpriteSet(Game, this);
			if (!Anim || FAILED(Anim->loadBuffer(params, false))) cmd = PARSERR_GENERIC;
			else _anims.Add(Anim);
		}
		break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ACTOR definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		if (spr) delete spr;
		Game->LOG(0, "Error loading ACTOR definition");
		return E_FAIL;
	}

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = DRGBA(ar, ag, ab, alpha);
	_state = _nextState = STATE_READY;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::turnTo(TDirection dir) {
	int delta1, delta2, delta3, delta;

	delta1 = dir - _dir;
	delta2 = dir + NUM_DIRECTIONS - _dir;
	delta3 = dir - NUM_DIRECTIONS - _dir;

	delta1 = (abs(delta1) <= abs(delta2)) ? delta1 : delta2;
	delta = (abs(delta1) <= abs(delta3)) ? delta1 : delta3;

	// already there?
	if (abs(delta) < 2) {
		_dir = dir;
		_state = _nextState;
		_nextState = STATE_READY;
		return;
	}

	_targetDir = dir;
	_state = delta < 0 ? STATE_TURNING_LEFT : STATE_TURNING_RIGHT;

	_tempSprite2 = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::goTo(int x, int y, TDirection afterWalkDir) {
	_afterWalkDir = afterWalkDir;
	if (x == _targetPoint->x && y == _targetPoint->y && _state == STATE_FOLLOWING_PATH) return;

	_path->reset();
	_path->setReady(false);

	_targetPoint->x = x;
	_targetPoint->y = y;

	((CAdGame *)Game)->_scene->correctTargetPoint(_posX, _posY, &_targetPoint->x, &_targetPoint->y, true, this);

	_state = STATE_SEARCHING_PATH;

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::display() {
	if (_active) updateSounds();

	uint32 alpha;
	if (_alphaColor != 0) alpha = _alphaColor;
	else alpha = _shadowable ? ((CAdGame *)Game)->_scene->getAlphaAt(_posX, _posY, true) : 0xFFFFFFFF;

	float scaleX, scaleY;
	getScale(&scaleX, &scaleY);


	float rotate;
	if (_rotatable) {
		if (_rotateValid) rotate = _rotate;
		else rotate = ((CAdGame *)Game)->_scene->getRotationAt(_posX, _posY) + _relativeRotate;
	} else rotate = 0.0f;

	if (_active) displaySpriteAttachments(true);

	if (_currentSprite && _active) {
		bool reg = _registrable;
		if (_ignoreItems && ((CAdGame *)Game)->_selectedItem) reg = false;

		_currentSprite->display(_posX,
		                        _posY,
		                        reg ? _registerAlias : NULL,
		                        scaleX,
		                        scaleY,
		                        alpha,
		                        rotate,
		                        _blendMode);

	}

	if (_active) displaySpriteAttachments(false);
	if (_active && _partEmitter) _partEmitter->display();


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::update() {
	_currentSprite = NULL;

	if (_state == STATE_READY) {
		if (_animSprite) {
			delete _animSprite;
			_animSprite = NULL;
		}
		if (_animSprite2) {
			_animSprite2 = NULL;
		}
	}

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL && _animSprite->_finished) {
		_state = _nextState;
		_nextState = STATE_READY;
		_currentSprite = _animSprite;
	}

	if (_state == STATE_PLAYING_ANIM_SET && _animSprite2 != NULL && _animSprite2->_finished) {
		_state = _nextState;
		_nextState = STATE_READY;
		_currentSprite = _animSprite2;
	}

	if (_sentence && _state != STATE_TALKING) _sentence->finish();

	// default: stand animation
	if (!_currentSprite) {
		if (_sprite) _currentSprite = _sprite;
		else {
			if (_standSprite) {
				_currentSprite = _standSprite->getSprite(_dir);
			} else {
				CAdSpriteSet *Anim = getAnimByName(_idleAnimName);
				if (Anim) _currentSprite = Anim->getSprite(_dir);
			}
		}
	}

	bool already_moved = false;

	switch (_state) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		_currentSprite = _animSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM_SET:
		_currentSprite = _animSprite2;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TURNING_LEFT:
		if (_tempSprite2 == NULL || _tempSprite2->_finished) {
			if (_dir > 0) _dir = (TDirection)(_dir - 1);
			else _dir = (TDirection)(NUM_DIRECTIONS - 1);

			if (_dir == _targetDir) {
				_tempSprite2 = NULL;
				_state = _nextState;
				_nextState = STATE_READY;
			} else {
				if (_turnLeftSprite) {
					_tempSprite2 = _turnLeftSprite->getSprite(_dir);
				} else {
					CAdSpriteSet *Anim = getAnimByName(_turnLeftAnimName);
					if (Anim) _tempSprite2 = Anim->getSprite(_dir);
				}

				if (_tempSprite2) {
					_tempSprite2->reset();
					if (_tempSprite2->_looping) _tempSprite2->_looping = false;
				}
				_currentSprite = _tempSprite2;
			}
		} else _currentSprite = _tempSprite2;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_TURNING_RIGHT:
		if (_tempSprite2 == NULL || _tempSprite2->_finished) {
			_dir = (TDirection)(_dir + 1);

			if ((int)_dir >= (int)NUM_DIRECTIONS) _dir = (TDirection)(0);

			if (_dir == _targetDir) {
				_tempSprite2 = NULL;
				_state = _nextState;
				_nextState = STATE_READY;
			} else {
				if (_turnRightSprite) {
					_tempSprite2 = _turnRightSprite->getSprite(_dir);
				} else {
					CAdSpriteSet *Anim = getAnimByName(_turnRightAnimName);
					if (Anim) _tempSprite2 = Anim->getSprite(_dir);
				}

				if (_tempSprite2) {
					_tempSprite2->reset();
					if (_tempSprite2->_looping) _tempSprite2->_looping = false;
				}
				_currentSprite = _tempSprite2;
			}
		} else _currentSprite = _tempSprite2;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_SEARCHING_PATH:
		// keep asking scene for the path
		if (((CAdGame *)Game)->_scene->getPath(CBPoint(_posX, _posY), *_targetPoint, _path, this))
			_state = STATE_WAITING_PATH;
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_WAITING_PATH:
		// wait until the scene finished the path
		if (_path->_ready) followPath();
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_FOLLOWING_PATH:
		getNextStep();
		already_moved = true;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		_sentence->update(_dir);
		if (_sentence->_currentSprite) _tempSprite2 = _sentence->_currentSprite;

		bool TimeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) || (!_sentence->_sound && _sentence->_duration <= Game->_timer - _sentence->_startTime);
		if (_tempSprite2 == NULL || _tempSprite2->_finished || (/*_tempSprite2->_looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				_sentence->finish();
				_tempSprite2 = NULL;
				_state = _nextState;
				_nextState = STATE_READY;
			} else {
				_tempSprite2 = getTalkStance(_sentence->getNextStance());
				if (_tempSprite2) {
					_tempSprite2->reset();
					_currentSprite = _tempSprite2;
					((CAdGame *)Game)->addSentence(_sentence);
				}
			}
		} else {
			_currentSprite = _tempSprite2;
			((CAdGame *)Game)->addSentence(_sentence);
		}
	}
	break;

	//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!_animSprite && !_animSprite2) {
			if (_sprite) _currentSprite = _sprite;
			else {
				if (_standSprite) {
					_currentSprite = _standSprite->getSprite(_dir);
				} else {
					CAdSpriteSet *Anim = getAnimByName(_idleAnimName);
					if (Anim) _currentSprite = Anim->getSprite(_dir);
				}
			}
		}
		break;
	default:
		error("AdActor::Update - Unhandled enum");
	}


	if (_currentSprite && !already_moved) {
		_currentSprite->GetCurrentFrame(_zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100, _zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100);
		if (_currentSprite->_changed) {
			_posX += _currentSprite->_moveX;
			_posY += _currentSprite->_moveY;
			afterMove();
		}
	}

	//Game->QuickMessageForm("%s", _currentSprite->_filename);

	updateBlockRegion();
	_ready = (_state == STATE_READY);

	updatePartEmitter();
	updateSpriteAttachments();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::followPath() {
	// skip current position
	_path->getFirst();
	while (_path->getCurrent() != NULL) {
		if (_path->getCurrent()->x != _posX || _path->getCurrent()->y != _posY) break;
		_path->getNext();
	}

	// are there points to follow?
	if (_path->getCurrent() != NULL) {
		_state = STATE_FOLLOWING_PATH;;
		initLine(CBPoint(_posX, _posY), *_path->getCurrent());
	} else {
		if (_afterWalkDir != DI_NONE) turnTo(_afterWalkDir);
		else _state = STATE_READY;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::getNextStep() {
	if (_walkSprite) {
		_currentSprite = _walkSprite->getSprite(_dir);
	} else {
		CAdSpriteSet *Anim = getAnimByName(_walkAnimName);
		if (Anim) _currentSprite = Anim->getSprite(_dir);
	}

	if (!_currentSprite) return;

	_currentSprite->GetCurrentFrame(_zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100, _zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100);
	if (!_currentSprite->_changed) return;


	int maxStepX, maxStepY;
	maxStepX = abs(_currentSprite->_moveX);
	maxStepY = abs(_currentSprite->_moveY);

	maxStepX = MAX(maxStepX, maxStepY);
	maxStepX = MAX(maxStepX, 1);

	while (_pFCount > 0 && maxStepX >= 0) {
		_pFX += _pFStepX;
		_pFY += _pFStepY;

		_pFCount--;
		maxStepX--;
	}

	if (((CAdGame *)Game)->_scene->isBlockedAt(_pFX, _pFY, true, this)) {
		if (_pFCount == 0) {
			_state = _nextState;
			_nextState = STATE_READY;
			return;
		}
		goTo(_targetPoint->x, _targetPoint->y);
		return;
	}


	_posX = (int)_pFX;
	_posY = (int)_pFY;

	afterMove();


	if (_pFCount == 0) {
		if (_path->getNext() == NULL) {
			_posX = _targetPoint->x;
			_posY = _targetPoint->y;

			_path->reset();
			if (_afterWalkDir != DI_NONE) turnTo(_afterWalkDir);
			else {
				_state = _nextState;
				_nextState = STATE_READY;
			}
		} else initLine(CBPoint(_posX, _posY), *_path->getCurrent());
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::initLine(CBPoint startPt, CBPoint endPt) {
	_pFCount = MAX((abs(endPt.x - startPt.x)) , (abs(endPt.y - startPt.y)));

	_pFStepX = (double)(endPt.x - startPt.x) / _pFCount;
	_pFStepY = (double)(endPt.y - startPt.y) / _pFCount;

	_pFX = startPt.x;
	_pFY = startPt.y;

	int angle = (int)(atan2((double)(endPt.y - startPt.y), (double)(endPt.x - startPt.x)) * (180 / 3.14));

	_nextState = STATE_FOLLOWING_PATH;

	turnTo(angleToDirection(angle));
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GoTo / GoToAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GoTo") == 0 || strcmp(name, "GoToAsync") == 0) {
		stack->correctParams(2);
		int X = stack->pop()->getInt();
		int Y = stack->pop()->getInt();
		goTo(X, Y);
		if (strcmp(name, "GoToAsync") != 0) script->waitForExclusive(this);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToObject / GoToObjectAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GoToObject") == 0 || strcmp(name, "GoToObjectAsync") == 0) {
		stack->correctParams(1);
		CScValue *Val = stack->pop();
		if (!Val->isNative()) {
			script->runtimeError("actor.%s method accepts an entity refrence only", name);
			stack->pushNULL();
			return S_OK;
		}
		CAdObject *Obj = (CAdObject *)Val->getNative();
		if (!Obj || Obj->_type != OBJECT_ENTITY) {
			script->runtimeError("actor.%s method accepts an entity refrence only", name);
			stack->pushNULL();
			return S_OK;
		}
		CAdEntity *Ent = (CAdEntity *)Obj;
		if (Ent->_walkToX == 0 && Ent->_walkToY == 0) goTo(Ent->_posX, Ent->_posY);
		else goTo(Ent->_walkToX, Ent->_walkToY, Ent->_walkToDir);
		if (strcmp(name, "GoToObjectAsync") != 0) script->waitForExclusive(this);
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnTo / TurnToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnTo") == 0 || strcmp(name, "TurnToAsync") == 0) {
		stack->correctParams(1);
		int dir;
		CScValue *val = stack->pop();

		// turn to object?
		if (val->isNative() && Game->validObject((CBObject *)val->getNative())) {
			CBObject *obj = (CBObject *)val->getNative();
			int angle = (int)(atan2((double)(obj->_posY - _posY), (double)(obj->_posX - _posX)) * (180 / 3.14));
			dir = (int)angleToDirection(angle);
		}
		// otherwise turn to direction
		else dir = val->getInt();

		if (dir >= 0 && dir < NUM_DIRECTIONS) {
			turnTo((TDirection)dir);
			if (strcmp(name, "TurnToAsync") != 0) script->waitForExclusive(this);
		}
		stack->pushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsWalking") == 0) {
		stack->correctParams(0);
		stack->pushBool(_state == STATE_FOLLOWING_PATH);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MergeAnims
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MergeAnims") == 0) {
		stack->correctParams(1);
		stack->pushBool(SUCCEEDED(mergeAnims(stack->pop()->getString())));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "UnloadAnim") == 0) {
		stack->correctParams(1);
		const char *AnimName = stack->pop()->getString();

		bool Found = false;
		for (int i = 0; i < _anims.GetSize(); i++) {
			if (scumm_stricmp(_anims[i]->_name, AnimName) == 0) {
				// invalidate sprites in use
				if (_anims[i]->containsSprite(_tempSprite2)) _tempSprite2 = NULL;
				if (_anims[i]->containsSprite(_currentSprite)) _currentSprite = NULL;
				if (_anims[i]->containsSprite(_animSprite2)) _animSprite2 = NULL;

				delete _anims[i];
				_anims[i] = NULL;
				_anims.RemoveAt(i);
				i--;
				Found = true;
			}
		}
		stack->pushBool(Found);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasAnim") == 0) {
		stack->correctParams(1);
		const char *AnimName = stack->pop()->getString();
		stack->pushBool(getAnimByName(AnimName) != NULL);
		return S_OK;
	}

	else return CAdTalkHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdActor::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Direction") == 0) {
		_scValue->setInt(_dir);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Type") == 0) {
		_scValue->setString("actor");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkAnimName") == 0) {
		_scValue->setString(_talkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkAnimName") == 0) {
		_scValue->setString(_walkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IdleAnimName") == 0) {
		_scValue->setString(_idleAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnLeftAnimName") == 0) {
		_scValue->setString(_turnLeftAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnRightAnimName") == 0) {
		_scValue->setString(_turnRightAnimName);
		return _scValue;
	}

	else return CAdTalkHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Direction") == 0) {
		int dir = value->getInt();
		if (dir >= 0 && dir < NUM_DIRECTIONS) _dir = (TDirection)dir;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TalkAnimName") == 0) {
		if (value->isNULL()) _talkAnimName = "talk";
		else _talkAnimName = value->getString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkAnimName") == 0) {
		if (value->isNULL()) _walkAnimName = "walk";
		else _walkAnimName = value->getString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IdleAnimName") == 0) {
		if (value->isNULL()) _idleAnimName = "idle";
		else _idleAnimName = value->getString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnLeftAnimName") == 0) {
		if (value->isNULL()) _turnLeftAnimName = "turnleft";
		else _turnLeftAnimName = value->getString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "TurnRightAnimName") == 0) {
		if (value->isNULL()) _turnRightAnimName = "turnright";
		else _turnRightAnimName = value->getString();
		return S_OK;
	}

	else return CAdTalkHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdActor::scToString() {
	return "[actor object]";
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::getTalkStance(const char *stance) {
	// forced stance?
	if (_forcedTalkAnimName && !_forcedTalkAnimUsed) {
		_forcedTalkAnimUsed = true;
		delete _animSprite;
		_animSprite = new CBSprite(Game, this);
		if (_animSprite) {
			HRESULT res = _animSprite->loadFile(_forcedTalkAnimName);
			if (FAILED(res)) {
				Game->LOG(res, "CAdActor::GetTalkStance: error loading talk sprite (object:\"%s\" sprite:\"%s\")", _name, _forcedTalkAnimName);
				delete _animSprite;
				_animSprite = NULL;
			} else return _animSprite;
		}
	}

	// old way
	if (_talkSprites.GetSize() > 0 || _talkSpritesEx.GetSize() > 0)
		return getTalkStanceOld(stance);

	// new way
	CBSprite *ret = NULL;

	// do we have an animation with this name?
	CAdSpriteSet *Anim = getAnimByName(stance);
	if (Anim) ret = Anim->getSprite(_dir);

	// not - get a random talk
	if (!ret) {
		CBArray<CAdSpriteSet *, CAdSpriteSet *> TalkAnims;
		for (int i = 0; i < _anims.GetSize(); i++) {
			if (_talkAnimName.compareToIgnoreCase(_anims[i]->_name) == 0)
				TalkAnims.Add(_anims[i]);
		}

		if (TalkAnims.GetSize() > 0) {
			int rnd = g_wintermute->randInt(0, TalkAnims.GetSize() - 1);
			ret = TalkAnims[rnd]->getSprite(_dir);
		} else {
			if (_standSprite) ret = _standSprite->getSprite(_dir);
			else {
				Anim = getAnimByName(_idleAnimName);
				if (Anim) ret = Anim->getSprite(_dir);
			}
		}
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::getTalkStanceOld(const char *stance) {
	CBSprite *ret = NULL;

	if (stance != NULL) {
		// search special stances
		for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(_talkSpritesEx[i]->_name, stance) == 0) {
				ret = _talkSpritesEx[i]->getSprite(_dir);
				break;
			}
		}
		if (ret == NULL) {
			// search generic stances
			for (int i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_name, stance) == 0) {
					ret = _talkSprites[i]->getSprite(_dir);
					break;
				}
			}
		}
	}

	// not a valid stance? get a random one
	if (ret == NULL) {
		if (_talkSprites.GetSize() < 1) ret = _standSprite->getSprite(_dir);
		else {
			// TODO: remember last
			int rnd = g_wintermute->randInt(0, _talkSprites.GetSize() - 1);
			ret = _talkSprites[rnd]->getSprite(_dir);
		}
	}

	return ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::persist(CBPersistMgr *persistMgr) {
	CAdTalkHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER_INT(_dir));
	persistMgr->transfer(TMEMBER(_path));
	persistMgr->transfer(TMEMBER(_pFCount));
	persistMgr->transfer(TMEMBER(_pFStepX));
	persistMgr->transfer(TMEMBER(_pFStepY));
	persistMgr->transfer(TMEMBER(_pFX));
	persistMgr->transfer(TMEMBER(_pFY));
	persistMgr->transfer(TMEMBER(_standSprite));
	_talkSprites.persist(persistMgr);
	_talkSpritesEx.persist(persistMgr);
	persistMgr->transfer(TMEMBER_INT(_targetDir));
	persistMgr->transfer(TMEMBER_INT(_afterWalkDir));
	persistMgr->transfer(TMEMBER(_targetPoint));
	persistMgr->transfer(TMEMBER(_turnLeftSprite));
	persistMgr->transfer(TMEMBER(_turnRightSprite));
	persistMgr->transfer(TMEMBER(_walkSprite));

	persistMgr->transfer(TMEMBER(_animSprite2));
	persistMgr->transfer(TMEMBER(_talkAnimName));
	persistMgr->transfer(TMEMBER(_idleAnimName));
	persistMgr->transfer(TMEMBER(_walkAnimName));
	persistMgr->transfer(TMEMBER(_turnLeftAnimName));
	persistMgr->transfer(TMEMBER(_turnRightAnimName));

	_anims.persist(persistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
TDirection CAdActor::angleToDirection(int angle) {
	TDirection ret = DI_DOWN;;

	if (angle > -112 && angle <= -67) ret = DI_UP;
	else if (angle > -67  && angle <= -22)  ret = DI_UPRIGHT;
	else if (angle > -22  && angle <= 22)   ret = DI_RIGHT;
	else if (angle > 22   && angle <= 67)   ret = DI_DOWNRIGHT;
	else if (angle > 67   && angle <= 112)  ret = DI_DOWN;
	else if (angle > 112  && angle <= 157)  ret = DI_DOWNLEFT;
	else if ((angle > 157 && angle <= 180) || (angle >= -180 && angle <= -157)) ret = DI_LEFT;
	else if (angle > -157 && angle <= -112) ret = DI_UPLEFT;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
int CAdActor::getHeight() {
	// if no current sprite is set, set some
	if (_currentSprite == NULL) {
		if (_standSprite) _currentSprite = _standSprite->getSprite(_dir);
		else {
			CAdSpriteSet *Anim = getAnimByName(_idleAnimName);
			if (Anim) _currentSprite = Anim->getSprite(_dir);
		}
	}
	// and get height
	return CAdTalkHolder::getHeight();
}


//////////////////////////////////////////////////////////////////////////
CAdSpriteSet *CAdActor::getAnimByName(const Common::String &animName) {
	for (int i = 0; i < _anims.GetSize(); i++) {
		if (animName.compareToIgnoreCase(_anims[i]->_name) == 0)
			return _anims[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::mergeAnims(const char *animsFilename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END


	byte *fileBuffer = Game->_fileManager->readWholeFile(animsFilename);
	if (fileBuffer == NULL) {
		Game->LOG(0, "CAdActor::MergeAnims failed for file '%s'", animsFilename);
		return E_FAIL;
	}

	byte *buffer = fileBuffer;
	byte *params;
	int cmd;
	CBParser parser(Game);

	HRESULT Ret = S_OK;

	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_ANIMATION: {
			CAdSpriteSet *Anim = new CAdSpriteSet(Game, this);
			if (!Anim || FAILED(Anim->loadBuffer(params, false))) {
				cmd = PARSERR_GENERIC;
				Ret = E_FAIL;
			} else _anims.Add(Anim);
		}
		break;
		}
	}
	delete [] fileBuffer;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::playAnim(const char *filename) {
	// if we have an anim with this name, use it
	CAdSpriteSet *Anim = getAnimByName(filename);
	if (Anim) {
		_animSprite2 = Anim->getSprite(_dir);
		if (_animSprite2) {
			_animSprite2->reset();
			_state = STATE_PLAYING_ANIM_SET;
			return S_OK;
		}
	}
	// otherwise call the standard handler
	return CAdTalkHolder::playAnim(filename);
}

} // end of namespace WinterMute
