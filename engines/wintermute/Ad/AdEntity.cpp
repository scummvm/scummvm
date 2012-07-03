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
#include "engines/wintermute/Ad/AdEntity.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BActiveRect.h"
#include "engines/wintermute/Base/BSurfaceStorage.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Ad/AdGame.h"
#include "engines/wintermute/Ad/AdScene.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Ad/AdWaypointGroup.h"
#include "engines/wintermute/Base/BFontStorage.h"
#include "engines/wintermute/Base/BFont.h"
#include "engines/wintermute/Ad/AdSentence.h"
#include "engines/wintermute/Base/BRegion.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/Base/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/utils/utils.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/video/VidTheoraPlayer.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdEntity, false)

//////////////////////////////////////////////////////////////////////////
CAdEntity::CAdEntity(CBGame *inGame): CAdTalkHolder(inGame) {
	_type = OBJECT_ENTITY;
	_subtype = ENTITY_NORMAL;
	_region = NULL;
	_item = NULL;

	_walkToX = _walkToY = 0;
	_walkToDir = DI_NONE;

	_theora = NULL;
}


//////////////////////////////////////////////////////////////////////////
CAdEntity::~CAdEntity() {
	Game->UnregisterObject(_region);

	delete _theora;
	_theora = NULL;

	delete[] _item;
	_item = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::loadFile(const char *filename) {
	byte *buffer = Game->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		Game->LOG(0, "CAdEntity::LoadFile failed for file '%s'", filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (FAILED(ret = loadBuffer(buffer, true))) Game->LOG(0, "Error parsing ENTITY file '%s'", filename);


	delete [] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(ENTITY)
TOKEN_DEF(SPRITE)
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
TOKEN_DEF(EVENTS)
TOKEN_DEF(FONT)
TOKEN_DEF(TALK_SPECIAL)
TOKEN_DEF(TALK)
TOKEN_DEF(CURSOR)
TOKEN_DEF(REGION)
TOKEN_DEF(BLOCKED_REGION)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(SOUND_START_TIME)
TOKEN_DEF(SOUND_VOLUME)
TOKEN_DEF(SOUND_PANNING)
TOKEN_DEF(SOUND)
TOKEN_DEF(SUBTYPE)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PROPERTY)
TOKEN_DEF(WAYPOINTS)
TOKEN_DEF(IGNORE_ITEMS)
TOKEN_DEF(ROTABLE)
TOKEN_DEF(ROTATABLE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(SCALE)
TOKEN_DEF(RELATIVE_SCALE)
TOKEN_DEF(ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(ITEM)
TOKEN_DEF(WALK_TO_X)
TOKEN_DEF(WALK_TO_Y)
TOKEN_DEF(WALK_TO_DIR)
TOKEN_DEF(SAVE_STATE)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::loadBuffer(byte *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(ENTITY)
	TOKEN_TABLE(SPRITE)
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
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TALK_SPECIAL)
	TOKEN_TABLE(TALK)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(REGION)
	TOKEN_TABLE(BLOCKED_REGION)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(SOUND_START_TIME)
	TOKEN_TABLE(SOUND_VOLUME)
	TOKEN_TABLE(SOUND_PANNING)
	TOKEN_TABLE(SOUND)
	TOKEN_TABLE(SUBTYPE)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PROPERTY)
	TOKEN_TABLE(WAYPOINTS)
	TOKEN_TABLE(IGNORE_ITEMS)
	TOKEN_TABLE(ROTABLE)
	TOKEN_TABLE(ROTATABLE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(SCALE)
	TOKEN_TABLE(RELATIVE_SCALE)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(ITEM)
	TOKEN_TABLE(WALK_TO_X)
	TOKEN_TABLE(WALK_TO_Y)
	TOKEN_TABLE(WALK_TO_DIR)
	TOKEN_TABLE(SAVE_STATE)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	if (complete) {
		if (parser.GetCommand((char **)&buffer, commands, (char **)&params) != TOKEN_ENTITY) {
			Game->LOG(0, "'ENTITY' keyword expected.");
			return E_FAIL;
		}
		buffer = params;
	}

	CAdGame *adGame = (CAdGame *)Game;
	CBSprite *spr = NULL;
	int ar = 0, ag = 0, ab = 0, alpha = 0;
	while ((cmd = parser.GetCommand((char **)&buffer, commands, (char **)&params)) > 0) {
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

		case TOKEN_SPRITE: {
			delete _sprite;
			_sprite = NULL;
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->loadFile((char *)params))) cmd = PARSERR_GENERIC;
			else _sprite = spr;
		}
		break;

		case TOKEN_TALK: {
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->loadFile((char *)params, adGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSprites.Add(spr);
		}
		break;

		case TOKEN_TALK_SPECIAL: {
			spr = new CBSprite(Game, this);
			if (!spr || FAILED(spr->loadFile((char *)params, adGame->_texTalkLifeTime))) cmd = PARSERR_GENERIC;
			else _talkSpritesEx.Add(spr);
		}
		break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_ITEM:
			setItem((char *)params);
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

		case TOKEN_CURSOR:
			delete _cursor;
			_cursor = new CBSprite(Game);
			if (!_cursor || FAILED(_cursor->loadFile((char *)params))) {
				delete _cursor;
				_cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr((char *)params, "%b", &_editorSelected);
			break;

		case TOKEN_REGION: {
			if (_region) Game->UnregisterObject(_region);
			_region = NULL;
			CBRegion *rgn = new CBRegion(Game);
			if (!rgn || FAILED(rgn->loadBuffer(params, false))) cmd = PARSERR_GENERIC;
			else {
				_region = rgn;
				Game->RegisterObject(_region);
			}
		}
		break;

		case TOKEN_BLOCKED_REGION: {
			delete _blockRegion;
			_blockRegion = NULL;
			delete _currentBlockRegion;
			_currentBlockRegion = NULL;
			CBRegion *rgn = new CBRegion(Game);
			CBRegion *crgn = new CBRegion(Game);
			if (!rgn || !crgn || FAILED(rgn->loadBuffer(params, false))) {
				delete _blockRegion;
				_blockRegion = NULL;
				delete _currentBlockRegion;
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
			_wptGroup = NULL;
			delete _currentWptGroup;
			_currentWptGroup = NULL;
			CAdWaypointGroup *wpt = new CAdWaypointGroup(Game);
			CAdWaypointGroup *cwpt = new CAdWaypointGroup(Game);
			if (!wpt || !cwpt || FAILED(wpt->loadBuffer(params, false))) {
				delete _wptGroup;
				_wptGroup = NULL;
				delete _currentWptGroup;
				_currentWptGroup = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				_wptGroup = wpt;
				_currentWptGroup = cwpt;
				_currentWptGroup->Mimic(_wptGroup);
			}
		}
		break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_SUBTYPE: {
			if (scumm_stricmp((char *)params, "sound") == 0) {
				delete _sprite;
				_sprite = NULL;
				if (Game->_editorMode) {
					spr = new CBSprite(Game, this);
					if (!spr || FAILED(spr->loadFile("entity_sound.sprite"))) cmd = PARSERR_GENERIC;
					else _sprite = spr;
				}
				if (Game->_editorMode) _editorOnly = true;
				_zoomable = false;
				_rotatable = false;
				_registrable = Game->_editorMode;
				_shadowable = false;
				_subtype = ENTITY_SOUND;
			}
		}
		break;

		case TOKEN_SOUND:
			playSFX((char *)params, false, false);
			break;

		case TOKEN_SOUND_START_TIME:
			parser.ScanStr((char *)params, "%d", &_sFXStart);
			break;

		case TOKEN_SOUND_VOLUME:
			parser.ScanStr((char *)params, "%d", &_sFXVolume);
			break;

		case TOKEN_SOUND_PANNING:
			parser.ScanStr((char *)params, "%b", &_autoSoundPanning);
			break;

		case TOKEN_SAVE_STATE:
			parser.ScanStr((char *)params, "%b", &_saveState);
			break;

		case TOKEN_PROPERTY:
			parseProperty(params, false);
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

		case TOKEN_WALK_TO_X:
			parser.ScanStr((char *)params, "%d", &_walkToX);
			break;

		case TOKEN_WALK_TO_Y:
			parser.ScanStr((char *)params, "%d", &_walkToY);
			break;

		case TOKEN_WALK_TO_DIR: {
			int i;
			parser.ScanStr((char *)params, "%d", &i);
			if (i < 0) i = 0;
			if (i >= NUM_DIRECTIONS) i = DI_NONE;
			_walkToDir = (TDirection)i;
		}
		break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in ENTITY definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading ENTITY definition");
		if (spr) delete spr;
		return E_FAIL;
	}

	if (_region && _sprite) {
		Game->LOG(0, "Warning: Entity '%s' has both sprite and region.", _name);
	}

	updatePosition();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	_alphaColor = DRGBA(ar, ag, ab, alpha);
	_state = STATE_READY;

	if (_item && ((CAdGame *)Game)->IsItemTaken(_item)) _active = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::display() {
	if (_active) {
		updateSounds();

		uint32 Alpha;
		if (_alphaColor != 0) Alpha = _alphaColor;
		else Alpha = _shadowable ? ((CAdGame *)Game)->_scene->getAlphaAt(_posX, _posY) : 0xFFFFFFFF;

		float ScaleX, ScaleY;
		GetScale(&ScaleX, &ScaleY);

		float Rotate;
		if (_rotatable) {
			if (_rotateValid) Rotate = _rotate;
			else Rotate = ((CAdGame *)Game)->_scene->getRotationAt(_posX, _posY) + _relativeRotate;
		} else Rotate = 0.0f;


		bool Reg = _registrable;
		if (_ignoreItems && ((CAdGame *)Game)->_selectedItem) Reg = false;

		if (_region && (Reg || _editorAlwaysRegister)) {
			Game->_renderer->_rectList.Add(new CBActiveRect(Game, _registerAlias, _region, Game->_offsetX, Game->_offsetY));
		}

		displaySpriteAttachments(true);
		if (_theora && (_theora->isPlaying() || _theora->isPaused())) {
			_theora->display(Alpha);
		} else if (_currentSprite) {
			_currentSprite->display(_posX,
			                        _posY,
			                        (Reg || _editorAlwaysRegister) ? _registerAlias : NULL,
			                        ScaleX,
			                        ScaleY,
			                        Alpha,
			                        Rotate,
			                        _blendMode);
		}
		displaySpriteAttachments(false);

		if (_partEmitter) _partEmitter->display(_region);

	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::update() {
	_currentSprite = NULL;

	if (_state == STATE_READY && _animSprite) {
		delete _animSprite;
		_animSprite = NULL;
	}

	// finished playing animation?
	if (_state == STATE_PLAYING_ANIM && _animSprite != NULL && _animSprite->_finished) {
		_state = STATE_READY;
		_currentSprite = _animSprite;
	}

	if (_sentence && _state != STATE_TALKING) _sentence->finish();

	// default: stand animation
	if (!_currentSprite) _currentSprite = _sprite;

	switch (_state) {
		//////////////////////////////////////////////////////////////////////////
	case STATE_PLAYING_ANIM:
		_currentSprite = _animSprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_READY:
		if (!_animSprite)
			_currentSprite = _sprite;
		break;

		//////////////////////////////////////////////////////////////////////////
	case STATE_TALKING: {
		_sentence->update();
		if (_sentence->_currentSprite) _tempSprite2 = _sentence->_currentSprite;

		bool TimeIsUp = (_sentence->_sound && _sentence->_soundStarted && (!_sentence->_sound->isPlaying() && !_sentence->_sound->isPaused())) || (!_sentence->_sound && _sentence->_duration <= Game->_timer - _sentence->_startTime);
		if (_tempSprite2 == NULL || _tempSprite2->_finished || (/*_tempSprite2->_looping &&*/ TimeIsUp)) {
			if (TimeIsUp) {
				_sentence->finish();
				_tempSprite2 = NULL;
				_state = STATE_READY;
			} else {
				_tempSprite2 = getTalkStance(_sentence->getNextStance());
				if (_tempSprite2) {
					_tempSprite2->Reset();
					_currentSprite = _tempSprite2;
				}
				((CAdGame *)Game)->AddSentence(_sentence);
			}
		} else {
			_currentSprite = _tempSprite2;
			((CAdGame *)Game)->AddSentence(_sentence);
		}
	}
	break;
	default: // Silence unhandled enum-warning
		break;
	}


	if (_currentSprite) {
		_currentSprite->GetCurrentFrame(_zoomable ? ((CAdGame *)Game)->_scene->getZoomAt(_posX, _posY) : 100);
		if (_currentSprite->_changed) {
			_posX += _currentSprite->_moveX;
			_posY += _currentSprite->_moveY;
		}
	}

	updateBlockRegion();
	_ready = (_state == STATE_READY);

	if (_theora) {
		int OffsetX, OffsetY;
		Game->GetOffset(&OffsetX, &OffsetY);
		_theora->_posX = _posX - OffsetX;
		_theora->_posY = _posY - OffsetY;

		_theora->update();
		if (_theora->isFinished()) {
			_theora->stop();
			delete _theora;
			_theora = NULL;
		}
	}

	updatePartEmitter();
	updateSpriteAttachments();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "StopSound") == 0 && _subtype == ENTITY_SOUND) {
		stack->correctParams(0);

		if (FAILED(stopSFX(false))) stack->pushBool(false);
		else stack->pushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlayTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlayTheora") == 0) {
		stack->correctParams(4);
		const char *filename = stack->pop()->getString();
		bool looping = stack->pop()->getBool(false);
		CScValue *valAlpha = stack->pop();
		int startTime = stack->pop()->getInt();

		delete _theora;
		_theora = new CVidTheoraPlayer(Game);
		if (_theora && SUCCEEDED(_theora->initialize(filename))) {
			if (!valAlpha->isNULL())    _theora->setAlphaImage(valAlpha->getString());
			_theora->play(VID_PLAY_POS, 0, 0, false, false, looping, startTime, _scale >= 0.0f ? _scale : -1.0f, _sFXVolume);
			//if(m_Scale>=0) m_Theora->m_PlayZoom = m_Scale;
			stack->pushBool(true);
		} else {
			script->RuntimeError("Entity.PlayTheora - error playing video '%s'", filename);
			stack->pushBool(false);
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopTheora") == 0) {
		stack->correctParams(0);
		if (_theora) {
			_theora->stop();
			delete _theora;
			_theora = NULL;
			stack->pushBool(true);
		} else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsTheoraPlaying") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPlaying()) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseTheora") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPlaying()) {
			_theora->pause();
			stack->pushBool(true);
		} else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeTheora
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeTheora") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPaused()) {
			_theora->resume();
			stack->pushBool(true);
		} else stack->pushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsTheoraPaused
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsTheoraPaused") == 0) {
		stack->correctParams(0);
		if (_theora && _theora->isPaused()) stack->pushBool(true);
		else stack->pushBool(false);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// CreateRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "CreateRegion") == 0) {
		stack->correctParams(0);
		if (!_region) {
			_region = new CBRegion(Game);
			Game->RegisterObject(_region);
		}
		if (_region) stack->pushNative(_region, true);
		else stack->pushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteRegion
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteRegion") == 0) {
		stack->correctParams(0);
		if (_region) {
			Game->UnregisterObject(_region);
			_region = NULL;
			stack->pushBool(true);
		} else stack->pushBool(false);

		return S_OK;
	}

	else return CAdTalkHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CAdEntity::scGetProperty(const char *name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->setString("entity");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Item") == 0) {
		if (_item) _scValue->setString(_item);
		else _scValue->setNULL();

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Subtype (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Subtype") == 0) {
		if (_subtype == ENTITY_SOUND)
			_scValue->setString("sound");
		else
			_scValue->setString("normal");

		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToX") == 0) {
		_scValue->setInt(_walkToX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToY") == 0) {
		_scValue->setInt(_walkToY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToDirection") == 0) {
		_scValue->setInt((int)_walkToDir);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Region (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Region") == 0) {
		if (_region) _scValue->setNative(_region, true);
		else _scValue->setNULL();
		return _scValue;
	}

	else return CAdTalkHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::scSetProperty(const char *name, CScValue *value) {

	//////////////////////////////////////////////////////////////////////////
	// Item
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Item") == 0) {
		setItem(value->getString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToX") == 0) {
		_walkToX = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToY") == 0) {
		_walkToY = value->getInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// WalkToDirection
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "WalkToDirection") == 0) {
		int dir = value->getInt();
		if (dir >= 0 && dir < NUM_DIRECTIONS) _walkToDir = (TDirection)dir;
		return S_OK;
	}

	else return CAdTalkHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CAdEntity::scToString() {
	return "[entity object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "ENTITY {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	if (_subtype == ENTITY_SOUND)
		buffer->putTextIndent(indent + 2, "SUBTYPE=\"SOUND\"\n");
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());
	buffer->putTextIndent(indent + 2, "ACTIVE=%s\n", _active ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "X=%d\n", _posX);
	buffer->putTextIndent(indent + 2, "Y=%d\n", _posY);
	buffer->putTextIndent(indent + 2, "SCALABLE=%s\n", _zoomable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "INTERACTIVE=%s\n", _registrable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "COLORABLE=%s\n", _shadowable ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "EDITOR_SELECTED=%s\n", _editorSelected ? "TRUE" : "FALSE");
	if (_ignoreItems)
		buffer->putTextIndent(indent + 2, "IGNORE_ITEMS=%s\n", _ignoreItems ? "TRUE" : "FALSE");
	if (_rotatable)
		buffer->putTextIndent(indent + 2, "ROTATABLE=%s\n", _rotatable ? "TRUE" : "FALSE");

	if (!_autoSoundPanning)
		buffer->putTextIndent(indent + 2, "SOUND_PANNING=%s\n", _autoSoundPanning ? "TRUE" : "FALSE");

	if (!_saveState)
		buffer->putTextIndent(indent + 2, "SAVE_STATE=%s\n", _saveState ? "TRUE" : "FALSE");

	if (_item && _item[0] != '\0') buffer->putTextIndent(indent + 2, "ITEM=\"%s\"\n", _item);

	buffer->putTextIndent(indent + 2, "WALK_TO_X=%d\n", _walkToX);
	buffer->putTextIndent(indent + 2, "WALK_TO_Y=%d\n", _walkToY);
	if (_walkToDir != DI_NONE)
		buffer->putTextIndent(indent + 2, "WALK_TO_DIR=%d\n", (int)_walkToDir);

	int i;

	for (i = 0; i < _scripts.GetSize(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	if (_subtype == ENTITY_NORMAL && _sprite && _sprite->_filename)
		buffer->putTextIndent(indent + 2, "SPRITE=\"%s\"\n", _sprite->_filename);

	if (_subtype == ENTITY_SOUND && _sFX && _sFX->_soundFilename) {
		buffer->putTextIndent(indent + 2, "SOUND=\"%s\"\n", _sFX->_soundFilename);
		buffer->putTextIndent(indent + 2, "SOUND_START_TIME=%d\n", _sFXStart);
		buffer->putTextIndent(indent + 2, "SOUND_VOLUME=%d\n", _sFXVolume);
	}


	if (D3DCOLGetR(_alphaColor) != 0 || D3DCOLGetG(_alphaColor) != 0 ||  D3DCOLGetB(_alphaColor) != 0)
		buffer->putTextIndent(indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", D3DCOLGetR(_alphaColor), D3DCOLGetG(_alphaColor), D3DCOLGetB(_alphaColor));

	if (D3DCOLGetA(_alphaColor) != 0)
		buffer->putTextIndent(indent + 2, "ALPHA = %d\n", D3DCOLGetA(_alphaColor));

	if (_scale >= 0)
		buffer->putTextIndent(indent + 2, "SCALE = %d\n", (int)_scale);

	if (_relativeScale != 0)
		buffer->putTextIndent(indent + 2, "RELATIVE_SCALE = %d\n", (int)_relativeScale);

	if (_font && _font->_filename)
		buffer->putTextIndent(indent + 2, "FONT=\"%s\"\n", _font->_filename);

	if (_cursor && _cursor->_filename)
		buffer->putTextIndent(indent + 2, "CURSOR=\"%s\"\n", _cursor->_filename);

	CAdTalkHolder::saveAsText(buffer, indent + 2);

	if (_region) _region->saveAsText(buffer, indent + 2);

	if (_scProp) _scProp->saveAsText(buffer, indent + 2);

	CAdObject::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CAdEntity::getHeight() {
	if (_region && !_sprite) {
		return _region->_rect.bottom - _region->_rect.top;
	} else {
		if (_currentSprite == NULL) _currentSprite = _sprite;
		return CAdObject::getHeight();
	}
}


//////////////////////////////////////////////////////////////////////////
void CAdEntity::updatePosition() {
	if (_region && !_sprite) {
		_posX = _region->_rect.left + (_region->_rect.right - _region->_rect.left) / 2;
		_posY = _region->_rect.bottom;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::persist(CBPersistMgr *persistMgr) {
	CAdTalkHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_item));
	persistMgr->transfer(TMEMBER(_region));
	//persistMgr->transfer(TMEMBER(_sprite));
	persistMgr->transfer(TMEMBER_INT(_subtype));
	_talkSprites.persist(persistMgr);
	_talkSpritesEx.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_walkToX));
	persistMgr->transfer(TMEMBER(_walkToY));
	persistMgr->transfer(TMEMBER_INT(_walkToDir));

	persistMgr->transfer(TMEMBER(_theora));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdEntity::setItem(const char *itemName) {
	CBUtils::SetString(&_item, itemName);
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdEntity::setSprite(const char *filename) {
	bool setCurrent = false;
	if (_currentSprite == _sprite) {
		_currentSprite = NULL;
		setCurrent = true;
	}

	delete _sprite;
	_sprite = NULL;
	CBSprite *spr = new CBSprite(Game, this);
	if (!spr || FAILED(spr->loadFile(filename))) {
		delete _sprite;
		_sprite = NULL;
		return E_FAIL;
	} else {
		_sprite = spr;
		_currentSprite = _sprite;
		return S_OK;
	}
}

} // end of namespace WinterMute
