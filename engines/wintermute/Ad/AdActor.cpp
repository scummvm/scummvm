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

	SetDefaultAnimNames();
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::SetDefaultAnimNames() {
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
HRESULT CAdActor::loadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->readWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdActor::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = loadBuffer(Buffer, true))) Game->LOG(0, "Error parsing ACTOR file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CAdActor::loadBuffer(byte  *Buffer, bool Complete) {
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

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_ACTOR) {
			Game->LOG(0, "'ACTOR' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	CAdGame *AdGame = (CAdGame *)Game;
	CAdSpriteSet *spr = NULL;
	int ar = 0, ag = 0, ab = 0, alpha = 0;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &_posX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &_posY);
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
			parser.ScanStr((char *)params, "%b", &_zoomable);
			break;

		case TOKEN_ROTABLE:
		case TOKEN_ROTATABLE:
			parser.ScanStr((char *)params, "%b", &_rotatable);
			break;

		case TOKEN_REGISTRABLE:
		case TOKEN_INTERACTIVE:
			parser.ScanStr((char *)params, "%b", &_registrable);
			break;

		case TOKEN_SHADOWABLE:
		case TOKEN_COLORABLE:
			parser.ScanStr((char *)params, "%b", &_shadowable);
			break;

		case TOKEN_ACTIVE:
			parser.ScanStr((char *)params, "%b", &_active);
			break;

		case TOKEN_WALK:
			delete _walkSprite;
			_walkSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, AdGame->_texWalkLifeTime, CACHE_HALF))) cmd = PARSERR_GENERIC;
			else _walkSprite = spr;
			break;

		case TOKEN_TALK:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, AdGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSprites.Add(spr);
			break;

		case TOKEN_TALK_SPECIAL:
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, AdGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSpritesEx.Add(spr);
			break;

		case TOKEN_STAND:
			delete _standSprite;
			_standSprite = NULL;
			spr = new CAdSpriteSet(Game, this);
			if (!spr || FAILED(spr->loadBuffer(params, true, AdGame->_texStandLifeTime))) cmd = PARSERR_GENERIC;
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
			parser.ScanStr((char *)params, "%d", &_sFXVolume);
			break;

		case TOKEN_SCALE: {
			int s;
			parser.ScanStr((char *)params, "%d", &s);
			_scale = (float)s;

		}
		break;

		case TOKEN_RELATIVE_SCALE: {
			int s;
			parser.ScanStr((char *)params, "%d", &s);
			_relativeScale = (float)s;

		}
		break;

		case TOKEN_SOUND_PANNING:
			parser.ScanStr((char *)params, "%b", &_autoSoundPanning);
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
				_currentBlockRegion->Mimic(_blockRegion);
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
				_currentWptGroup->Mimic(_wptGroup);
			}
		}
		break;

		case TOKEN_IGNORE_ITEMS:
			parser.ScanStr((char *)params, "%b", &_ignoreItems);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
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
void CAdActor::TurnTo(TDirection dir) {
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
void CAdActor::GoTo(int X, int Y, TDirection AfterWalkDir) {
	_afterWalkDir = AfterWalkDir;
	if (X == _targetPoint->x && Y == _targetPoint->y && _state == STATE_FOLLOWING_PATH) return;

	_path->Reset();
	_path->SetReady(false);

	_targetPoint->x = X;
	_targetPoint->y = Y;

	((CAdGame *)Game)->_scene->correctTargetPoint(_posX, _posY, &_targetPoint->x, &_targetPoint->y, true, this);

	_state = STATE_SEARCHING_PATH;

}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::display() {
	if (_active) updateSounds();

	uint32 Alpha;
	if (_alphaColor != 0) Alpha = _alphaColor;
	else Alpha = _shadowable ? ((CAdGame *)Game)->_scene->getAlphaAt(_posX, _posY, true) : 0xFFFFFFFF;

	float ScaleX, ScaleY;
	GetScale(&ScaleX, &ScaleY);


	float Rotate;
	if (_rotatable) {
		if (_rotateValid) Rotate = _rotate;
		else Rotate = ((CAdGame *)Game)->_scene->getRotationAt(_posX, _posY) + _relativeRotate;
	} else Rotate = 0.0f;

	if (_active) displaySpriteAttachments(true);

	if (_currentSprite && _active) {
		bool Reg = _registrable;
		if (_ignoreItems && ((CAdGame *)Game)->_selectedItem) Reg = false;

		_currentSprite->display(_posX,
		                        _posY,
		                        Reg ? _registerAlias : NULL,
		                        ScaleX,
		                        ScaleY,
		                        Alpha,
		                        Rotate,
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
				CAdSpriteSet *Anim = GetAnimByName(_idleAnimName);
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
					CAdSpriteSet *Anim = GetAnimByName(_turnLeftAnimName);
					if (Anim) _tempSprite2 = Anim->getSprite(_dir);
				}

				if (_tempSprite2) {
					_tempSprite2->Reset();
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
					CAdSpriteSet *Anim = GetAnimByName(_turnRightAnimName);
					if (Anim) _tempSprite2 = Anim->getSprite(_dir);
				}

				if (_tempSprite2) {
					_tempSprite2->Reset();
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
		if (_path->_ready) FollowPath();
		break;


		//////////////////////////////////////////////////////////////////////////
	case STATE_FOLLOWING_PATH:
		GetNextStep();
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
					_tempSprite2->Reset();
					_currentSprite = _tempSprite2;
					((CAdGame *)Game)->AddSentence(_sentence);
				}
			}
		} else {
			_currentSprite = _tempSprite2;
			((CAdGame *)Game)->AddSentence(_sentence);
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
					CAdSpriteSet *Anim = GetAnimByName(_idleAnimName);
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
void CAdActor::FollowPath() {
	// skip current position
	_path->GetFirst();
	while (_path->GetCurrent() != NULL) {
		if (_path->GetCurrent()->x != _posX || _path->GetCurrent()->y != _posY) break;
		_path->GetNext();
	}

	// are there points to follow?
	if (_path->GetCurrent() != NULL) {
		_state = STATE_FOLLOWING_PATH;;
		InitLine(CBPoint(_posX, _posY), *_path->GetCurrent());
	} else {
		if (_afterWalkDir != DI_NONE) TurnTo(_afterWalkDir);
		else _state = STATE_READY;
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::GetNextStep() {
	if (_walkSprite) {
		_currentSprite = _walkSprite->getSprite(_dir);
	} else {
		CAdSpriteSet *Anim = GetAnimByName(_walkAnimName);
		if (Anim) _currentSprite = Anim->getSprite(_dir);
	}

	if (!_currentSprite) return;

	_currentSprite->GetCurrentFrame(_zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100, _zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100);
	if (!_currentSprite->_changed) return;


	int MaxStepX, MaxStepY;
	MaxStepX = abs(_currentSprite->_moveX);
	MaxStepY = abs(_currentSprite->_moveY);

	MaxStepX = MAX(MaxStepX, MaxStepY);
	MaxStepX = MAX(MaxStepX, 1);

	while (_pFCount > 0 && MaxStepX >= 0) {
		_pFX += _pFStepX;
		_pFY += _pFStepY;

		_pFCount--;
		MaxStepX--;
	}

	if (((CAdGame *)Game)->_scene->isBlockedAt(_pFX, _pFY, true, this)) {
		if (_pFCount == 0) {
			_state = _nextState;
			_nextState = STATE_READY;
			return;
		}
		GoTo(_targetPoint->x, _targetPoint->y);
		return;
	}


	_posX = (int)_pFX;
	_posY = (int)_pFY;

	afterMove();


	if (_pFCount == 0) {
		if (_path->GetNext() == NULL) {
			_posX = _targetPoint->x;
			_posY = _targetPoint->y;

			_path->Reset();
			if (_afterWalkDir != DI_NONE) TurnTo(_afterWalkDir);
			else {
				_state = _nextState;
				_nextState = STATE_READY;
			}
		} else InitLine(CBPoint(_posX, _posY), *_path->GetCurrent());
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdActor::InitLine(CBPoint StartPt, CBPoint EndPt) {
	_pFCount = MAX((abs(EndPt.x - StartPt.x)) , (abs(EndPt.y - StartPt.y)));

	_pFStepX = (double)(EndPt.x - StartPt.x) / _pFCount;
	_pFStepY = (double)(EndPt.y - StartPt.y) / _pFCount;

	_pFX = StartPt.x;
	_pFY = StartPt.y;

	int angle = (int)(atan2((double)(EndPt.y - StartPt.y), (double)(EndPt.x - StartPt.x)) * (180 / 3.14));

	_nextState = STATE_FOLLOWING_PATH;

	TurnTo(AngleToDirection(angle));
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GoTo / GoToAsync
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GoTo") == 0 || strcmp(Name, "GoToAsync") == 0) {
		Stack->CorrectParams(2);
		int X = Stack->Pop()->GetInt();
		int Y = Stack->Pop()->GetInt();
		GoTo(X, Y);
		if (strcmp(Name, "GoToAsync") != 0) Script->WaitForExclusive(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoToObject / GoToObjectAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoToObject") == 0 || strcmp(Name, "GoToObjectAsync") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		if (!Val->IsNative()) {
			Script->RuntimeError("actor.%s method accepts an entity refrence only", Name);
			Stack->PushNULL();
			return S_OK;
		}
		CAdObject *Obj = (CAdObject *)Val->GetNative();
		if (!Obj || Obj->_type != OBJECT_ENTITY) {
			Script->RuntimeError("actor.%s method accepts an entity refrence only", Name);
			Stack->PushNULL();
			return S_OK;
		}
		CAdEntity *Ent = (CAdEntity *)Obj;
		if (Ent->_walkToX == 0 && Ent->_walkToY == 0) GoTo(Ent->_posX, Ent->_posY);
		else GoTo(Ent->_walkToX, Ent->_walkToY, Ent->_walkToDir);
		if (strcmp(Name, "GoToObjectAsync") != 0) Script->WaitForExclusive(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnTo / TurnToAsync
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnTo") == 0 || strcmp(Name, "TurnToAsync") == 0) {
		Stack->CorrectParams(1);
		int dir;
		CScValue *val = Stack->Pop();

		// turn to object?
		if (val->IsNative() && Game->ValidObject((CBObject *)val->GetNative())) {
			CBObject *obj = (CBObject *)val->GetNative();
			int angle = (int)(atan2((double)(obj->_posY - _posY), (double)(obj->_posX - _posX)) * (180 / 3.14));
			dir = (int)AngleToDirection(angle);
		}
		// otherwise turn to direction
		else dir = val->GetInt();

		if (dir >= 0 && dir < NUM_DIRECTIONS) {
			TurnTo((TDirection)dir);
			if (strcmp(Name, "TurnToAsync") != 0) Script->WaitForExclusive(this);
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsWalking
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsWalking") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(_state == STATE_FOLLOWING_PATH);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MergeAnims
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MergeAnims") == 0) {
		Stack->CorrectParams(1);
		Stack->PushBool(SUCCEEDED(MergeAnims(Stack->Pop()->GetString())));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// UnloadAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "UnloadAnim") == 0) {
		Stack->CorrectParams(1);
		const char *AnimName = Stack->Pop()->GetString();

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
		Stack->PushBool(Found);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasAnim
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasAnim") == 0) {
		Stack->CorrectParams(1);
		const char *AnimName = Stack->Pop()->GetString();
		Stack->PushBool(GetAnimByName(AnimName) != NULL);
		return S_OK;
	}

	else return CAdTalkHolder::scCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdActor::scGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Direction") == 0) {
		_scValue->SetInt(_dir);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("actor");
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkAnimName") == 0) {
		_scValue->SetString(_talkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkAnimName") == 0) {
		_scValue->SetString(_walkAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IdleAnimName") == 0) {
		_scValue->SetString(_idleAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnLeftAnimName") == 0) {
		_scValue->SetString(_turnLeftAnimName);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnRightAnimName") == 0) {
		_scValue->SetString(_turnRightAnimName);
		return _scValue;
	}

	else return CAdTalkHolder::scGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::scSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Direction
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Direction") == 0) {
		int dir = Value->GetInt();
		if (dir >= 0 && dir < NUM_DIRECTIONS) _dir = (TDirection)dir;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TalkAnimName") == 0) {
		if (Value->IsNULL()) _talkAnimName = "talk";
		else _talkAnimName = Value->GetString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "WalkAnimName") == 0) {
		if (Value->IsNULL()) _walkAnimName = "walk";
		else _walkAnimName = Value->GetString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IdleAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IdleAnimName") == 0) {
		if (Value->IsNULL()) _idleAnimName = "idle";
		else _idleAnimName = Value->GetString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnLeftAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnLeftAnimName") == 0) {
		if (Value->IsNULL()) _turnLeftAnimName = "turnleft";
		else _turnLeftAnimName = Value->GetString();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// TurnRightAnimName
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TurnRightAnimName") == 0) {
		if (Value->IsNULL()) _turnRightAnimName = "turnright";
		else _turnRightAnimName = Value->GetString();
		return S_OK;
	}

	else return CAdTalkHolder::scSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdActor::scToString() {
	return "[actor object]";
}


//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::getTalkStance(const char *Stance) {
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
		return GetTalkStanceOld(Stance);

	// new way
	CBSprite *Ret = NULL;

	// do we have an animation with this name?
	CAdSpriteSet *Anim = GetAnimByName(Stance);
	if (Anim) Ret = Anim->getSprite(_dir);

	// not - get a random talk
	if (!Ret) {
		CBArray<CAdSpriteSet *, CAdSpriteSet *> TalkAnims;
		for (int i = 0; i < _anims.GetSize(); i++) {
			if (_talkAnimName.compareToIgnoreCase(_anims[i]->_name) == 0)
				TalkAnims.Add(_anims[i]);
		}

		if (TalkAnims.GetSize() > 0) {
			int rnd = g_wintermute->randInt(0, TalkAnims.GetSize() - 1);
			Ret = TalkAnims[rnd]->getSprite(_dir);
		} else {
			if (_standSprite) Ret = _standSprite->getSprite(_dir);
			else {
				Anim = GetAnimByName(_idleAnimName);
				if (Anim) Ret = Anim->getSprite(_dir);
			}
		}
	}
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
CBSprite *CAdActor::GetTalkStanceOld(const char *Stance) {
	CBSprite *ret = NULL;

	if (Stance != NULL) {
		// search special stances
		for (int i = 0; i < _talkSpritesEx.GetSize(); i++) {
			if (scumm_stricmp(_talkSpritesEx[i]->_name, Stance) == 0) {
				ret = _talkSpritesEx[i]->getSprite(_dir);
				break;
			}
		}
		if (ret == NULL) {
			// search generic stances
			for (int i = 0; i < _talkSprites.GetSize(); i++) {
				if (scumm_stricmp(_talkSprites[i]->_name, Stance) == 0) {
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
TDirection CAdActor::AngleToDirection(int Angle) {
	TDirection ret = DI_DOWN;;

	if (Angle > -112 && Angle <= -67) ret = DI_UP;
	else if (Angle > -67  && Angle <= -22)  ret = DI_UPRIGHT;
	else if (Angle > -22  && Angle <= 22)   ret = DI_RIGHT;
	else if (Angle > 22   && Angle <= 67)   ret = DI_DOWNRIGHT;
	else if (Angle > 67   && Angle <= 112)  ret = DI_DOWN;
	else if (Angle > 112  && Angle <= 157)  ret = DI_DOWNLEFT;
	else if ((Angle > 157 && Angle <= 180) || (Angle >= -180 && Angle <= -157)) ret = DI_LEFT;
	else if (Angle > -157 && Angle <= -112) ret = DI_UPLEFT;

	return ret;
}


//////////////////////////////////////////////////////////////////////////
int CAdActor::getHeight() {
	// if no current sprite is set, set some
	if (_currentSprite == NULL) {
		if (_standSprite) _currentSprite = _standSprite->getSprite(_dir);
		else {
			CAdSpriteSet *Anim = GetAnimByName(_idleAnimName);
			if (Anim) _currentSprite = Anim->getSprite(_dir);
		}
	}
	// and get height
	return CAdTalkHolder::getHeight();
}


//////////////////////////////////////////////////////////////////////////
CAdSpriteSet *CAdActor::GetAnimByName(const Common::String &animName) {
	for (int i = 0; i < _anims.GetSize(); i++) {
		if (animName.compareToIgnoreCase(_anims[i]->_name) == 0)
			return _anims[i];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::MergeAnims(const char *AnimsFilename) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ANIMATION)
	TOKEN_TABLE_END


	byte *FileBuffer = Game->_fileManager->readWholeFile(AnimsFilename);
	if (FileBuffer == NULL) {
		Game->LOG(0, "CAdActor::MergeAnims failed for file '%s'", AnimsFilename);
		return E_FAIL;
	}

	byte *Buffer = FileBuffer;
	byte *params;
	int cmd;
	CBParser parser(Game);

	HRESULT Ret = S_OK;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
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
	delete [] FileBuffer;
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdActor::PlayAnim(const char *Filename) {
	// if we have an anim with this name, use it
	CAdSpriteSet *Anim = GetAnimByName(Filename);
	if (Anim) {
		_animSprite2 = Anim->getSprite(_dir);
		if (_animSprite2) {
			_animSprite2->Reset();
			_state = STATE_PLAYING_ANIM_SET;
			return S_OK;
		}
	}
	// otherwise call the standard handler
	return CAdTalkHolder::playAnim(Filename);
}

} // end of namespace WinterMute
