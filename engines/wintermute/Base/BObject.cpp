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
#include "engines/wintermute/Base/BObject.h"
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BStringTable.h"
#include "engines/wintermute/Base/BSprite.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBObject, false)

//////////////////////////////////////////////////////////////////////
CBObject::CBObject(CBGame *inGame): CBScriptHolder(inGame) {
	_posX = _posY = 0;
	_movable = true;
	_zoomable = true;
	_registrable = true;
	_shadowable = true;
	_rotatable = false;
	_is3D = false;

	_alphaColor = 0;
	_scale = -1;
	_relativeScale = 0;

	_scaleX = -1;
	_scaleY = -1;

	_ready = true;

	_soundEvent = NULL;

	_iD = Game->GetSequence();

	CBPlatform::SetRectEmpty(&_rect);
	_rectSet = false;

	_cursor = NULL;
	_activeCursor = NULL;
	_sharedCursors = false;

	_sFX = NULL;
	_sFXStart = 0;
	_sFXVolume = 100;
	_autoSoundPanning = true;

	_editorAlwaysRegister = false;
	_editorSelected = false;

	_editorOnly = false;

	_rotate = 0.0f;
	_rotateValid = false;
	_relativeRotate = 0.0f;

	for (int i = 0; i < 7; i++) _caption[i] = NULL;
	_saveState = true;

	_nonIntMouseEvents = false;

	// sound FX
	_sFXType = SFX_NONE;
	_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;

	_blendMode = BLEND_NORMAL;
}


//////////////////////////////////////////////////////////////////////
CBObject::~CBObject() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::cleanup() {
	if (Game && Game->_activeObject == this) Game->_activeObject = NULL;

	CBScriptHolder::cleanup();
	delete[] _soundEvent;
	_soundEvent = NULL;

	if (!_sharedCursors) {
		delete _cursor;
		delete _activeCursor;
		_cursor = NULL;
		_activeCursor = NULL;
	}
	delete _sFX;
	_sFX = NULL;

	for (int i = 0; i < 7; i++) {
		delete[] _caption[i];
		_caption[i] = NULL;
	}

	_sFXType = SFX_NONE;
	_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBObject::setCaption(const char *caption, int Case) { // TODO: rename Case to something usefull
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7) return;

	delete[] _caption[Case - 1];
	_caption[Case - 1] = new char[strlen(caption) + 1];
	if (_caption[Case - 1]) {
		strcpy(_caption[Case - 1], caption);
		Game->_stringTable->Expand(&_caption[Case - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBObject::getCaption(int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7 || _caption[Case - 1] == NULL) return "";
	else return _caption[Case - 1];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::listen(CBScriptHolder *param1, uint32 param2) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::scCallMethod(CScScript *script, CScStack *stack, CScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SkipTo") == 0) {
		stack->CorrectParams(2);
		_posX = stack->Pop()->GetInt();
		_posY = stack->Pop()->GetInt();
		afterMove();
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Caption") == 0) {
		stack->CorrectParams(1);
		stack->PushString(getCaption(stack->Pop()->GetInt()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetCursor") == 0) {
		stack->CorrectParams(1);
		if (SUCCEEDED(setCursor(stack->Pop()->GetString()))) stack->PushBool(true);
		else stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveCursor") == 0) {
		stack->CorrectParams(0);
		if (!_sharedCursors) {
			delete _cursor;
			_cursor = NULL;
		} else {
			_cursor = NULL;

		}
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCursor") == 0) {
		stack->CorrectParams(0);
		if (!_cursor || !_cursor->_filename) stack->PushNULL();
		else stack->PushString(_cursor->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCursorObject") == 0) {
		stack->CorrectParams(0);
		if (!_cursor) stack->PushNULL();
		else stack->PushNative(_cursor, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasCursor") == 0) {
		stack->CorrectParams(0);

		if (_cursor) stack->PushBool(true);
		else stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetCaption") == 0) {
		stack->CorrectParams(2);
		setCaption(stack->Pop()->GetString(), stack->Pop()->GetInt());
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadSound") == 0) {
		stack->CorrectParams(1);
		const char *Filename = stack->Pop()->GetString();
		if (SUCCEEDED(playSFX(Filename, false, false)))
			stack->PushBool(true);
		else
			stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlaySound") == 0) {
		stack->CorrectParams(3);

		const char *Filename;
		bool Looping;
		uint32 LoopStart;

		CScValue *val1 = stack->Pop();
		CScValue *val2 = stack->Pop();
		CScValue *val3 = stack->Pop();

		if (val1->_type == VAL_BOOL) {
			Filename = NULL;
			Looping = val1->GetBool();
			LoopStart = val2->GetInt();
		} else {
			if (val1->IsNULL()) Filename = NULL;
			else Filename = val1->GetString();
			Looping = val2->IsNULL() ? false : val2->GetBool();
			LoopStart = val3->GetInt();
		}

		if (FAILED(playSFX(Filename, Looping, true, NULL, LoopStart))) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySoundEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlaySoundEvent") == 0) {
		stack->CorrectParams(2);

		const char *Filename;
		const char *EventName;

		CScValue *val1 = stack->Pop();
		CScValue *val2 = stack->Pop();

		if (val2->IsNULL()) {
			Filename = NULL;
			EventName = val1->GetString();
		} else {
			Filename = val1->GetString();
			EventName = val2->GetString();
		}

		if (FAILED(playSFX(Filename, false, true, EventName))) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopSound") == 0) {
		stack->CorrectParams(0);

		if (FAILED(stopSFX())) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseSound") == 0) {
		stack->CorrectParams(0);

		if (FAILED(pauseSFX())) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeSound") == 0) {
		stack->CorrectParams(0);

		if (FAILED(resumeSFX())) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSoundPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsSoundPlaying") == 0) {
		stack->CorrectParams(0);

		if (_sFX && _sFX->isPlaying()) stack->PushBool(true);
		else stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSoundPosition") == 0) {
		stack->CorrectParams(1);

		uint32 Time = stack->Pop()->GetInt();
		if (FAILED(setSFXTime(Time))) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundPosition") == 0) {
		stack->CorrectParams(0);

		if (!_sFX) stack->PushInt(0);
		else stack->PushInt(_sFX->getPositionTime());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSoundVolume") == 0) {
		stack->CorrectParams(1);

		int Volume = stack->Pop()->GetInt();
		if (FAILED(setSFXVolume(Volume))) stack->PushBool(false);
		else stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundVolume") == 0) {
		stack->CorrectParams(0);

		if (!_sFX) stack->PushInt(_sFXVolume);
		else stack->PushInt(_sFX->getVolume());
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SoundFXNone
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXNone") == 0) {
		stack->CorrectParams(0);
		_sFXType = SFX_NONE;
		_sFXParam1 = 0;
		_sFXParam2 = 0;
		_sFXParam3 = 0;
		_sFXParam4 = 0;
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXEcho
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXEcho") == 0) {
		stack->CorrectParams(4);
		_sFXType = SFX_ECHO;
		_sFXParam1 = (float)stack->Pop()->GetFloat(0); // Wet/Dry Mix [%] (0-100)
		_sFXParam2 = (float)stack->Pop()->GetFloat(0); // Feedback [%] (0-100)
		_sFXParam3 = (float)stack->Pop()->GetFloat(333.0f); // Left Delay [ms] (1-2000)
		_sFXParam4 = (float)stack->Pop()->GetFloat(333.0f); // Right Delay [ms] (1-2000)
		stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXReverb
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXReverb") == 0) {
		stack->CorrectParams(4);
		_sFXType = SFX_REVERB;
		_sFXParam1 = (float)stack->Pop()->GetFloat(0); // In Gain [dB] (-96 - 0)
		_sFXParam2 = (float)stack->Pop()->GetFloat(0); // Reverb Mix [dB] (-96 - 0)
		_sFXParam3 = (float)stack->Pop()->GetFloat(1000.0f); // Reverb Time [ms] (0.001 - 3000)
		_sFXParam4 = (float)stack->Pop()->GetFloat(0.001f); // HighFreq RT Ratio (0.001 - 0.999)
		stack->PushNULL();

		return S_OK;
	}

	else return CBScriptHolder::scCallMethod(script, stack, thisStack, name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBObject::scGetProperty(const char *name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Type") == 0) {
		_scValue->SetString("object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Caption") == 0) {
		_scValue->SetString(getCaption(1));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_scValue->SetInt(_posX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_scValue->SetInt(_posY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Height") == 0) {
		_scValue->SetInt(getHeight());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Ready (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Ready") == 0) {
		_scValue->SetBool(_ready);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Movable") == 0) {
		_scValue->SetBool(_movable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Registrable") == 0 || strcmp(name, "Interactive") == 0) {
		_scValue->SetBool(_registrable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Zoomable") == 0 || strcmp(name, "Scalable") == 0) {
		_scValue->SetBool(_zoomable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotatable") == 0) {
		_scValue->SetBool(_rotatable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaColor") == 0) {
		_scValue->SetInt((int)_alphaColor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "BlendMode") == 0) {
		_scValue->SetInt((int)_blendMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale") == 0) {
		if (_scale < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scale);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleX") == 0) {
		if (_scaleX < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scaleX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleY") == 0) {
		if (_scaleY < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scaleY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeScale") == 0) {
		_scValue->SetFloat((double)_relativeScale);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotate") == 0) {
		if (!_rotateValid) _scValue->SetNULL();
		else _scValue->SetFloat((double)_rotate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeRotate") == 0) {
		_scValue->SetFloat((double)_relativeRotate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Colorable") == 0) {
		_scValue->SetBool(_shadowable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundPanning") == 0) {
		_scValue->SetBool(_autoSoundPanning);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SaveState") == 0) {
		_scValue->SetBool(_saveState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NonIntMouseEvents") == 0) {
		_scValue->SetBool(_nonIntMouseEvents);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccCaption") == 0) {
		_scValue->SetNULL();
		return _scValue;
	}

	else return CBScriptHolder::scGetProperty(name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::scSetProperty(const char *name, CScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Caption") == 0) {
		setCaption(value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_posX = value->GetInt();
		afterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_posY = value->GetInt();
		afterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Movable") == 0) {
		_movable = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Registrable") == 0 || strcmp(name, "Interactive") == 0) {
		_registrable = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Zoomable") == 0 || strcmp(name, "Scalable") == 0) {
		_zoomable = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotatable") == 0) {
		_rotatable = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaColor") == 0) {
		_alphaColor = (uint32)value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "BlendMode") == 0) {
		int i = value->GetInt();
		if (i < BLEND_NORMAL || i >= NUM_BLEND_MODES) i = BLEND_NORMAL;
		_blendMode = (TSpriteBlendMode)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale") == 0) {
		if (value->IsNULL()) _scale = -1;
		else _scale = (float)value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleX") == 0) {
		if (value->IsNULL()) _scaleX = -1;
		else _scaleX = (float)value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleY") == 0) {
		if (value->IsNULL()) _scaleY = -1;
		else _scaleY = (float)value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeScale") == 0) {
		_relativeScale = (float)value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotate") == 0) {
		if (value->IsNULL()) {
			_rotate = 0.0f;
			_rotateValid = false;
		} else {
			_rotate = (float)value->GetFloat();
			_rotateValid = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeRotate") == 0) {
		_relativeRotate = (float)value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Colorable") == 0) {
		_shadowable = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundPanning") == 0) {
		_autoSoundPanning = value->GetBool();
		if (!_autoSoundPanning) resetSoundPan();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SaveState") == 0) {
		_saveState = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NonIntMouseEvents") == 0) {
		_nonIntMouseEvents = value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccCaption") == 0) {
		return S_OK;
	}

	else return CBScriptHolder::scSetProperty(name, value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBObject::scToString() {
	return "[object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::showCursor() {
	if (_cursor) return Game->DrawCursor(_cursor);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::saveAsText(CBDynBuffer *buffer, int indent) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::persist(CBPersistMgr *persistMgr) {
	CBScriptHolder::persist(persistMgr);

	for (int i = 0; i < 7; i++) persistMgr->transfer(TMEMBER(_caption[i]));
	persistMgr->transfer(TMEMBER(_activeCursor));
	persistMgr->transfer(TMEMBER(_alphaColor));
	persistMgr->transfer(TMEMBER(_autoSoundPanning));
	persistMgr->transfer(TMEMBER(_cursor));
	persistMgr->transfer(TMEMBER(_sharedCursors));
	persistMgr->transfer(TMEMBER(_editorAlwaysRegister));
	persistMgr->transfer(TMEMBER(_editorOnly));
	persistMgr->transfer(TMEMBER(_editorSelected));
	persistMgr->transfer(TMEMBER(_iD));
	persistMgr->transfer(TMEMBER(_is3D));
	persistMgr->transfer(TMEMBER(_movable));
	persistMgr->transfer(TMEMBER(_posX));
	persistMgr->transfer(TMEMBER(_posY));
	persistMgr->transfer(TMEMBER(_relativeScale));
	persistMgr->transfer(TMEMBER(_rotatable));
	persistMgr->transfer(TMEMBER(_scale));
	persistMgr->transfer(TMEMBER(_sFX));
	persistMgr->transfer(TMEMBER(_sFXStart));
	persistMgr->transfer(TMEMBER(_sFXVolume));
	persistMgr->transfer(TMEMBER(_ready));
	persistMgr->transfer(TMEMBER(_rect));
	persistMgr->transfer(TMEMBER(_rectSet));
	persistMgr->transfer(TMEMBER(_registrable));
	persistMgr->transfer(TMEMBER(_shadowable));
	persistMgr->transfer(TMEMBER(_soundEvent));
	persistMgr->transfer(TMEMBER(_zoomable));

	persistMgr->transfer(TMEMBER(_scaleX));
	persistMgr->transfer(TMEMBER(_scaleY));

	persistMgr->transfer(TMEMBER(_rotate));
	persistMgr->transfer(TMEMBER(_rotateValid));
	persistMgr->transfer(TMEMBER(_relativeRotate));

	persistMgr->transfer(TMEMBER(_saveState));
	persistMgr->transfer(TMEMBER(_nonIntMouseEvents));

	persistMgr->transfer(TMEMBER_INT(_sFXType));
	persistMgr->transfer(TMEMBER(_sFXParam1));
	persistMgr->transfer(TMEMBER(_sFXParam2));
	persistMgr->transfer(TMEMBER(_sFXParam3));
	persistMgr->transfer(TMEMBER(_sFXParam4));


	persistMgr->transfer(TMEMBER_INT(_blendMode));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::setCursor(const char *filename) {
	if (!_sharedCursors) {
		delete _cursor;
		_cursor = NULL;
	}

	_sharedCursors = false;
	_cursor = new CBSprite(Game);
	if (!_cursor || FAILED(_cursor->loadFile(filename))) {
		delete _cursor;
		_cursor = NULL;
		return E_FAIL;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::setActiveCursor(const char *filename) {
	delete _activeCursor;
	_activeCursor = new CBSprite(Game);
	if (!_activeCursor || FAILED(_activeCursor->loadFile(filename))) {
		delete _activeCursor;
		_activeCursor = NULL;
		return E_FAIL;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBObject::getHeight() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::handleMouse(TMouseEvent event, TMouseButton button) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::handleKeypress(Common::Event *event, bool printable) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::handleMouseWheel(int delta) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::playSFX(const char *filename, bool looping, bool playNow, const char *eventName, uint32 loopStart) {
	// just play loaded sound
	if (filename == NULL && _sFX) {
		if (Game->_editorMode || _sFXStart) {
			_sFX->setVolume(_sFXVolume);
			_sFX->setPositionTime(_sFXStart);
			if (!Game->_editorMode) _sFXStart = 0;
		}
		if (playNow) {
			setSoundEvent(eventName);
			if (loopStart) _sFX->setLoopStart(loopStart);
			return _sFX->play(looping);
		} else return S_OK;
	}

	if (filename == NULL) return E_FAIL;

	// create new sound
	delete _sFX;

	_sFX = new CBSound(Game);
	if (_sFX && SUCCEEDED(_sFX->setSound(filename, SOUND_SFX, true))) {
		_sFX->setVolume(_sFXVolume);
		if (_sFXStart) {
			_sFX->setPositionTime(_sFXStart);
			_sFXStart = 0;
		}
		_sFX->ApplyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
		if (playNow) {
			setSoundEvent(eventName);
			if (loopStart) _sFX->setLoopStart(loopStart);
			return _sFX->play(looping);
		} else return S_OK;
	} else {
		delete _sFX;
		_sFX = NULL;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::stopSFX(bool deleteSound) {
	if (_sFX) {
		_sFX->stop();
		if (deleteSound) {
			delete _sFX;
			_sFX = NULL;
		}
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::pauseSFX() {
	if (_sFX) return _sFX->pause();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::resumeSFX() {
	if (_sFX) return _sFX->resume();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::setSFXTime(uint32 time) {
	_sFXStart = time;
	if (_sFX && _sFX->isPlaying()) return _sFX->setPositionTime(time);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::setSFXVolume(int Volume) {
	_sFXVolume = Volume;
	if (_sFX) return _sFX->setVolume(Volume);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::updateSounds() {
	if (_soundEvent) {
		if (_sFX && !_sFX->isPlaying()) {
			applyEvent(_soundEvent);
			setSoundEvent(NULL);
		}
	}

	if (_sFX) updateOneSound(_sFX);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::updateOneSound(CBSound *sound) {
	HRESULT Ret = S_OK;

	if (sound) {
		if (_autoSoundPanning)
			Ret = sound->setPan(Game->_soundMgr->posToPan(_posX  - Game->_offsetX, _posY - Game->_offsetY));

		Ret = sound->ApplyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
	}
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::resetSoundPan() {
	if (!_sFX) return S_OK;
	else {
		return _sFX->setPan(0.0f);
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::getExtendedFlag(const char *flagName) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::isReady() {
	return _ready;
}


//////////////////////////////////////////////////////////////////////////
void CBObject::setSoundEvent(const char *eventName) {
	delete[] _soundEvent;
	_soundEvent = NULL;
	if (eventName) {
		_soundEvent = new char[strlen(eventName) + 1];
		if (_soundEvent) strcpy(_soundEvent, eventName);
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::afterMove() {
	return S_OK;
}

} // end of namespace WinterMute
