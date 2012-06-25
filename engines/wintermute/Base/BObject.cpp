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
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::Cleanup() {
	if (Game && Game->_activeObject == this) Game->_activeObject = NULL;

	CBScriptHolder::Cleanup();
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
void CBObject::SetCaption(const char *Caption, int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7) return;

	delete[] _caption[Case - 1];
	_caption[Case - 1] = new char[strlen(Caption) + 1];
	if (_caption[Case - 1]) {
		strcpy(_caption[Case - 1], Caption);
		Game->_stringTable->Expand(&_caption[Case - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBObject::GetCaption(int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7 || _caption[Case - 1] == NULL) return "";
	else return _caption[Case - 1];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::Listen(CBScriptHolder *param1, uint32 param2) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::scCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SkipTo") == 0) {
		Stack->CorrectParams(2);
		_posX = Stack->Pop()->GetInt();
		_posY = Stack->Pop()->GetInt();
		AfterMove();
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Caption") == 0) {
		Stack->CorrectParams(1);
		Stack->PushString(GetCaption(Stack->Pop()->GetInt()));

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetCursor") == 0) {
		Stack->CorrectParams(1);
		if (SUCCEEDED(SetCursor(Stack->Pop()->GetString()))) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RemoveCursor") == 0) {
		Stack->CorrectParams(0);
		if (!_sharedCursors) {
			delete _cursor;
			_cursor = NULL;
		} else {
			_cursor = NULL;

		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCursor") == 0) {
		Stack->CorrectParams(0);
		if (!_cursor || !_cursor->_filename) Stack->PushNULL();
		else Stack->PushString(_cursor->_filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCursorObject") == 0) {
		Stack->CorrectParams(0);
		if (!_cursor) Stack->PushNULL();
		else Stack->PushNative(_cursor, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasCursor") == 0) {
		Stack->CorrectParams(0);

		if (_cursor) Stack->PushBool(true);
		else Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetCaption") == 0) {
		Stack->CorrectParams(2);
		SetCaption(Stack->Pop()->GetString(), Stack->Pop()->GetInt());
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadSound") == 0) {
		Stack->CorrectParams(1);
		const char *Filename = Stack->Pop()->GetString();
		if (SUCCEEDED(PlaySFX(Filename, false, false)))
			Stack->PushBool(true);
		else
			Stack->PushBool(false);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlaySound") == 0) {
		Stack->CorrectParams(3);

		const char *Filename;
		bool Looping;
		uint32 LoopStart;

		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		CScValue *val3 = Stack->Pop();

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

		if (FAILED(PlaySFX(Filename, Looping, true, NULL, LoopStart))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySoundEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PlaySoundEvent") == 0) {
		Stack->CorrectParams(2);

		const char *Filename;
		const char *EventName;

		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();

		if (val2->IsNULL()) {
			Filename = NULL;
			EventName = val1->GetString();
		} else {
			Filename = val1->GetString();
			EventName = val2->GetString();
		}

		if (FAILED(PlaySFX(Filename, false, true, EventName))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "StopSound") == 0) {
		Stack->CorrectParams(0);

		if (FAILED(StopSFX())) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseSound") == 0) {
		Stack->CorrectParams(0);

		if (FAILED(PauseSFX())) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ResumeSound") == 0) {
		Stack->CorrectParams(0);

		if (FAILED(ResumeSFX())) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSoundPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "IsSoundPlaying") == 0) {
		Stack->CorrectParams(0);

		if (_sFX && _sFX->IsPlaying()) Stack->PushBool(true);
		else Stack->PushBool(false);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSoundPosition") == 0) {
		Stack->CorrectParams(1);

		uint32 Time = Stack->Pop()->GetInt();
		if (FAILED(SetSFXTime(Time))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSoundPosition") == 0) {
		Stack->CorrectParams(0);

		if (!_sFX) Stack->PushInt(0);
		else Stack->PushInt(_sFX->GetPositionTime());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetSoundVolume") == 0) {
		Stack->CorrectParams(1);

		int Volume = Stack->Pop()->GetInt();
		if (FAILED(SetSFXVolume(Volume))) Stack->PushBool(false);
		else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSoundVolume") == 0) {
		Stack->CorrectParams(0);

		if (!_sFX) Stack->PushInt(_sFXVolume);
		else Stack->PushInt(_sFX->GetVolume());
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SoundFXNone
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXNone") == 0) {
		Stack->CorrectParams(0);
		_sFXType = SFX_NONE;
		_sFXParam1 = 0;
		_sFXParam2 = 0;
		_sFXParam3 = 0;
		_sFXParam4 = 0;
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXEcho
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXEcho") == 0) {
		Stack->CorrectParams(4);
		_sFXType = SFX_ECHO;
		_sFXParam1 = (float)Stack->Pop()->GetFloat(0); // Wet/Dry Mix [%] (0-100)
		_sFXParam2 = (float)Stack->Pop()->GetFloat(0); // Feedback [%] (0-100)
		_sFXParam3 = (float)Stack->Pop()->GetFloat(333.0f); // Left Delay [ms] (1-2000)
		_sFXParam4 = (float)Stack->Pop()->GetFloat(333.0f); // Right Delay [ms] (1-2000)
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXReverb
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXReverb") == 0) {
		Stack->CorrectParams(4);
		_sFXType = SFX_REVERB;
		_sFXParam1 = (float)Stack->Pop()->GetFloat(0); // In Gain [dB] (-96 - 0)
		_sFXParam2 = (float)Stack->Pop()->GetFloat(0); // Reverb Mix [dB] (-96 - 0)
		_sFXParam3 = (float)Stack->Pop()->GetFloat(1000.0f); // Reverb Time [ms] (0.001 - 3000)
		_sFXParam4 = (float)Stack->Pop()->GetFloat(0.001f); // HighFreq RT Ratio (0.001 - 0.999)
		Stack->PushNULL();

		return S_OK;
	}

	else return CBScriptHolder::scCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBObject::scGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Caption") == 0) {
		_scValue->SetString(GetCaption(1));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "X") == 0) {
		_scValue->SetInt(_posX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		_scValue->SetInt(_posY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		_scValue->SetInt(GetHeight());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Ready (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Ready") == 0) {
		_scValue->SetBool(_ready);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Movable") == 0) {
		_scValue->SetBool(_movable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Registrable") == 0 || strcmp(Name, "Interactive") == 0) {
		_scValue->SetBool(_registrable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Zoomable") == 0 || strcmp(Name, "Scalable") == 0) {
		_scValue->SetBool(_zoomable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotatable") == 0) {
		_scValue->SetBool(_rotatable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		_scValue->SetInt((int)_alphaColor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "BlendMode") == 0) {
		_scValue->SetInt((int)_blendMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		if (_scale < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scale);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleX") == 0) {
		if (_scaleX < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scaleX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleY") == 0) {
		if (_scaleY < 0) _scValue->SetNULL();
		else _scValue->SetFloat((double)_scaleY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeScale") == 0) {
		_scValue->SetFloat((double)_relativeScale);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotate") == 0) {
		if (!_rotateValid) _scValue->SetNULL();
		else _scValue->SetFloat((double)_rotate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeRotate") == 0) {
		_scValue->SetFloat((double)_relativeRotate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Colorable") == 0) {
		_scValue->SetBool(_shadowable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundPanning") == 0) {
		_scValue->SetBool(_autoSoundPanning);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveState") == 0) {
		_scValue->SetBool(_saveState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NonIntMouseEvents") == 0) {
		_scValue->SetBool(_nonIntMouseEvents);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccCaption") == 0) {
		_scValue->SetNULL();
		return _scValue;
	}

	else return CBScriptHolder::scGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::scSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Caption") == 0) {
		SetCaption(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "X") == 0) {
		_posX = Value->GetInt();
		AfterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		_posY = Value->GetInt();
		AfterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Movable") == 0) {
		_movable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Registrable") == 0 || strcmp(Name, "Interactive") == 0) {
		_registrable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Zoomable") == 0 || strcmp(Name, "Scalable") == 0) {
		_zoomable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotatable") == 0) {
		_rotatable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		_alphaColor = (uint32)Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "BlendMode") == 0) {
		int i = Value->GetInt();
		if (i < BLEND_NORMAL || i >= NUM_BLEND_MODES) i = BLEND_NORMAL;
		_blendMode = (TSpriteBlendMode)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		if (Value->IsNULL()) _scale = -1;
		else _scale = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleX") == 0) {
		if (Value->IsNULL()) _scaleX = -1;
		else _scaleX = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleY") == 0) {
		if (Value->IsNULL()) _scaleY = -1;
		else _scaleY = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeScale") == 0) {
		_relativeScale = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotate") == 0) {
		if (Value->IsNULL()) {
			_rotate = 0.0f;
			_rotateValid = false;
		} else {
			_rotate = (float)Value->GetFloat();
			_rotateValid = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeRotate") == 0) {
		_relativeRotate = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Colorable") == 0) {
		_shadowable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundPanning") == 0) {
		_autoSoundPanning = Value->GetBool();
		if (!_autoSoundPanning) ResetSoundPan();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveState") == 0) {
		_saveState = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NonIntMouseEvents") == 0) {
		_nonIntMouseEvents = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccCaption") == 0) {
		return S_OK;
	}

	else return CBScriptHolder::scSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
const char *CBObject::scToString() {
	return "[object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ShowCursor() {
	if (_cursor) return Game->DrawCursor(_cursor);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SaveAsText(CBDynBuffer *Buffer, int Indent) {
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
HRESULT CBObject::SetCursor(const char *Filename) {
	if (!_sharedCursors) {
		delete _cursor;
		_cursor = NULL;
	}

	_sharedCursors = false;
	_cursor = new CBSprite(Game);
	if (!_cursor || FAILED(_cursor->LoadFile(Filename))) {
		delete _cursor;
		_cursor = NULL;
		return E_FAIL;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetActiveCursor(const char *Filename) {
	delete _activeCursor;
	_activeCursor = new CBSprite(Game);
	if (!_activeCursor || FAILED(_activeCursor->LoadFile(Filename))) {
		delete _activeCursor;
		_activeCursor = NULL;
		return E_FAIL;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
int CBObject::GetHeight() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::HandleMouse(TMouseEvent Event, TMouseButton Button) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::HandleKeypress(Common::Event *event, bool printable) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::HandleMouseWheel(int Delta) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::PlaySFX(const char *Filename, bool Looping, bool PlayNow, const char *EventName, uint32 LoopStart) {
	// just play loaded sound
	if (Filename == NULL && _sFX) {
		if (Game->_editorMode || _sFXStart) {
			_sFX->SetVolume(_sFXVolume);
			_sFX->SetPositionTime(_sFXStart);
			if (!Game->_editorMode) _sFXStart = 0;
		}
		if (PlayNow) {
			SetSoundEvent(EventName);
			if (LoopStart) _sFX->SetLoopStart(LoopStart);
			return _sFX->Play(Looping);
		} else return S_OK;
	}

	if (Filename == NULL) return E_FAIL;

	// create new sound
	delete _sFX;

	_sFX = new CBSound(Game);
	if (_sFX && SUCCEEDED(_sFX->SetSound(Filename, SOUND_SFX, true))) {
		_sFX->SetVolume(_sFXVolume);
		if (_sFXStart) {
			_sFX->SetPositionTime(_sFXStart);
			_sFXStart = 0;
		}
		_sFX->ApplyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
		if (PlayNow) {
			SetSoundEvent(EventName);
			if (LoopStart) _sFX->SetLoopStart(LoopStart);
			return _sFX->Play(Looping);
		} else return S_OK;
	} else {
		delete _sFX;
		_sFX = NULL;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::StopSFX(bool DeleteSound) {
	if (_sFX) {
		_sFX->Stop();
		if (DeleteSound) {
			delete _sFX;
			_sFX = NULL;
		}
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::PauseSFX() {
	if (_sFX) return _sFX->Pause();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ResumeSFX() {
	if (_sFX) return _sFX->Resume();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetSFXTime(uint32 Time) {
	_sFXStart = Time;
	if (_sFX && _sFX->IsPlaying()) return _sFX->SetPositionTime(Time);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetSFXVolume(int Volume) {
	_sFXVolume = Volume;
	if (_sFX) return _sFX->SetVolume(Volume);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::UpdateSounds() {
	if (_soundEvent) {
		if (_sFX && !_sFX->IsPlaying()) {
			ApplyEvent(_soundEvent);
			SetSoundEvent(NULL);
		}
	}

	if (_sFX) UpdateOneSound(_sFX);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::UpdateOneSound(CBSound *Sound) {
	HRESULT Ret = S_OK;

	if (Sound) {
		if (_autoSoundPanning)
			Ret = Sound->SetPan(Game->_soundMgr->posToPan(_posX  - Game->_offsetX, _posY - Game->_offsetY));

		Ret = Sound->ApplyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
	}
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ResetSoundPan() {
	if (!_sFX) return S_OK;
	else {
		return _sFX->SetPan(0.0f);
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::GetExtendedFlag(const char *FlagName) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::IsReady() {
	return _ready;
}


//////////////////////////////////////////////////////////////////////////
void CBObject::SetSoundEvent(const char *EventName) {
	delete[] _soundEvent;
	_soundEvent = NULL;
	if (EventName) {
		_soundEvent = new char[strlen(EventName) + 1];
		if (_soundEvent) strcpy(_soundEvent, EventName);
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::AfterMove() {
	return S_OK;
}

} // end of namespace WinterMute
