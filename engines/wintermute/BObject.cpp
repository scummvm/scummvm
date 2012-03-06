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
#include "engines/wintermute/BObject.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BSoundMgr.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBObject, false)

//////////////////////////////////////////////////////////////////////
CBObject::CBObject(CBGame *inGame): CBScriptHolder(inGame) {
	m_PosX = m_PosY = 0;
	m_Movable = true;
	m_Zoomable = true;
	m_Registrable = true;
	m_Shadowable = true;
	m_Rotatable = false;
	m_Is3D = false;

	m_AlphaColor = 0;
	m_Scale = -1;
	m_RelativeScale = 0;

	m_ScaleX = -1;
	m_ScaleY = -1;

	m_Ready = true;

	m_SoundEvent = NULL;

	m_ID = Game->GetSequence();

	CBPlatform::SetRectEmpty(&m_Rect);
	m_RectSet = false;

	m_Cursor = NULL;
	m_ActiveCursor = NULL;
	m_SharedCursors = false;

	m_SFX = NULL;
	m_SFXStart = 0;
	m_SFXVolume = 100;
	m_AutoSoundPanning = true;

	m_EditorAlwaysRegister = false;
	m_EditorSelected = false;

	m_EditorOnly = false;

	m_Rotate = 0.0f;
	m_RotateValid = false;
	m_RelativeRotate = 0.0f;

	for (int i = 0; i < 7; i++) m_Caption[i] = NULL;
	m_SaveState = true;

	m_NonIntMouseEvents = false;

	// sound FX
	m_SFXType = SFX_NONE;
	m_SFXParam1 = m_SFXParam2 = m_SFXParam3 = m_SFXParam4 = 0;

	m_BlendMode = BLEND_NORMAL;
}


//////////////////////////////////////////////////////////////////////
CBObject::~CBObject() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::Cleanup() {
	if (Game && Game->m_ActiveObject == this) Game->m_ActiveObject = NULL;

	CBScriptHolder::Cleanup();
	delete[] m_SoundEvent;
	m_SoundEvent = NULL;

	if (!m_SharedCursors) {
		delete m_Cursor;
		delete m_ActiveCursor;
		m_Cursor = NULL;
		m_ActiveCursor = NULL;
	}
	delete m_SFX;
	m_SFX = NULL;

	for (int i = 0; i < 7; i++) {
		delete[] m_Caption[i];
		m_Caption[i] = NULL;
	}

	m_SFXType = SFX_NONE;
	m_SFXParam1 = m_SFXParam2 = m_SFXParam3 = m_SFXParam4 = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBObject::SetCaption(char *Caption, int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7) return;

	delete[] m_Caption[Case - 1];
	m_Caption[Case - 1] = new char[strlen(Caption) + 1];
	if (m_Caption[Case - 1]) {
		strcpy(m_Caption[Case - 1], Caption);
		Game->m_StringTable->Expand(&m_Caption[Case - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBObject::GetCaption(int Case) {
	if (Case == 0) Case = 1;
	if (Case < 1 || Case > 7 || m_Caption[Case - 1] == NULL) return "";
	else return m_Caption[Case - 1];
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::Listen(CBScriptHolder *param1, uint32 param2) {
	return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SkipTo") == 0) {
		Stack->CorrectParams(2);
		m_PosX = Stack->Pop()->GetInt();
		m_PosY = Stack->Pop()->GetInt();
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
		if (!m_SharedCursors) {
			delete m_Cursor;
			m_Cursor = NULL;
		} else {
			m_Cursor = NULL;
	
		}
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCursor") == 0) {
		Stack->CorrectParams(0);
		if (!m_Cursor || !m_Cursor->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_Cursor->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetCursorObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_Cursor) Stack->PushNULL();
		else Stack->PushNative(m_Cursor, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HasCursor") == 0) {
		Stack->CorrectParams(0);

		if (m_Cursor) Stack->PushBool(true);
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
		char *Filename = Stack->Pop()->GetString();
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

		char *Filename;
		bool Looping;
		uint32 LoopStart;

		CScValue *val1 = Stack->Pop();
		CScValue *val2 = Stack->Pop();
		CScValue *val3 = Stack->Pop();

		if (val1->m_Type == VAL_BOOL) {
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

		char *Filename;
		char *EventName;

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

		if (m_SFX && m_SFX->IsPlaying()) Stack->PushBool(true);
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

		if (!m_SFX) Stack->PushInt(0);
		else Stack->PushInt(m_SFX->GetPositionTime());
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

		if (!m_SFX) Stack->PushInt(m_SFXVolume);
		else Stack->PushInt(m_SFX->GetVolume());
		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SoundFXNone
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXNone") == 0) {
		Stack->CorrectParams(0);
		m_SFXType = SFX_NONE;
		m_SFXParam1 = 0;
		m_SFXParam2 = 0;
		m_SFXParam3 = 0;
		m_SFXParam4 = 0;
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXEcho
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXEcho") == 0) {
		Stack->CorrectParams(4);
		m_SFXType = SFX_ECHO;
		m_SFXParam1 = (float)Stack->Pop()->GetFloat(0); // Wet/Dry Mix [%] (0-100)
		m_SFXParam2 = (float)Stack->Pop()->GetFloat(0); // Feedback [%] (0-100)
		m_SFXParam3 = (float)Stack->Pop()->GetFloat(333.0f); // Left Delay [ms] (1-2000)
		m_SFXParam4 = (float)Stack->Pop()->GetFloat(333.0f); // Right Delay [ms] (1-2000)
		Stack->PushNULL();

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXReverb
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundFXReverb") == 0) {
		Stack->CorrectParams(4);
		m_SFXType = SFX_REVERB;
		m_SFXParam1 = (float)Stack->Pop()->GetFloat(0); // In Gain [dB] (-96 - 0)
		m_SFXParam2 = (float)Stack->Pop()->GetFloat(0); // Reverb Mix [dB] (-96 - 0)
		m_SFXParam3 = (float)Stack->Pop()->GetFloat(1000.0f); // Reverb Time [ms] (0.001 - 3000)
		m_SFXParam4 = (float)Stack->Pop()->GetFloat(0.001f); // HighFreq RT Ratio (0.001 - 0.999)
		Stack->PushNULL();

		return S_OK;
	}

	else return CBScriptHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBObject::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("object");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Caption") == 0) {
		m_ScValue->SetString(GetCaption(1));
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "X") == 0) {
		m_ScValue->SetInt(m_PosX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		m_ScValue->SetInt(m_PosY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Height") == 0) {
		m_ScValue->SetInt(GetHeight());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Ready (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Ready") == 0) {
		m_ScValue->SetBool(m_Ready);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Movable") == 0) {
		m_ScValue->SetBool(m_Movable);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Registrable") == 0 || strcmp(Name, "Interactive") == 0) {
		m_ScValue->SetBool(m_Registrable);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Zoomable") == 0 || strcmp(Name, "Scalable") == 0) {
		m_ScValue->SetBool(m_Zoomable);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotatable") == 0) {
		m_ScValue->SetBool(m_Rotatable);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		m_ScValue->SetInt((int)m_AlphaColor);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "BlendMode") == 0) {
		m_ScValue->SetInt((int)m_BlendMode);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		if (m_Scale < 0) m_ScValue->SetNULL();
		else m_ScValue->SetFloat((double)m_Scale);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleX") == 0) {
		if (m_ScaleX < 0) m_ScValue->SetNULL();
		else m_ScValue->SetFloat((double)m_ScaleX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleY") == 0) {
		if (m_ScaleY < 0) m_ScValue->SetNULL();
		else m_ScValue->SetFloat((double)m_ScaleY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeScale") == 0) {
		m_ScValue->SetFloat((double)m_RelativeScale);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotate") == 0) {
		if (!m_RotateValid) m_ScValue->SetNULL();
		else m_ScValue->SetFloat((double)m_Rotate);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeRotate") == 0) {
		m_ScValue->SetFloat((double)m_RelativeRotate);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Colorable") == 0) {
		m_ScValue->SetBool(m_Shadowable);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundPanning") == 0) {
		m_ScValue->SetBool(m_AutoSoundPanning);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveState") == 0) {
		m_ScValue->SetBool(m_SaveState);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NonIntMouseEvents") == 0) {
		m_ScValue->SetBool(m_NonIntMouseEvents);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccCaption") == 0) {
		m_ScValue->SetNULL();
		return m_ScValue;
	}

	else return CBScriptHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ScSetProperty(char *Name, CScValue *Value) {
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
		m_PosX = Value->GetInt();
		AfterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Y") == 0) {
		m_PosY = Value->GetInt();
		AfterMove();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Movable") == 0) {
		m_Movable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Registrable") == 0 || strcmp(Name, "Interactive") == 0) {
		m_Registrable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Zoomable") == 0 || strcmp(Name, "Scalable") == 0) {
		m_Zoomable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotatable") == 0) {
		m_Rotatable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {
		m_AlphaColor = (uint32)Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "BlendMode") == 0) {
		int i = Value->GetInt();
		if (i < BLEND_NORMAL || i >= NUM_BLEND_MODES) i = BLEND_NORMAL;
		m_BlendMode = (TSpriteBlendMode)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Scale") == 0) {
		if (Value->IsNULL()) m_Scale = -1;
		else m_Scale = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleX") == 0) {
		if (Value->IsNULL()) m_ScaleX = -1;
		else m_ScaleX = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ScaleY") == 0) {
		if (Value->IsNULL()) m_ScaleY = -1;
		else m_ScaleY = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeScale") == 0) {
		m_RelativeScale = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Rotate") == 0) {
		if (Value->IsNULL()) {
			m_Rotate = 0.0f;
			m_RotateValid = false;
		} else {
			m_Rotate = (float)Value->GetFloat();
			m_RotateValid = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "RelativeRotate") == 0) {
		m_RelativeRotate = (float)Value->GetFloat();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Colorable") == 0) {
		m_Shadowable = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SoundPanning") == 0) {
		m_AutoSoundPanning = Value->GetBool();
		if (!m_AutoSoundPanning) ResetSoundPan();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SaveState") == 0) {
		m_SaveState = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NonIntMouseEvents") == 0) {
		m_NonIntMouseEvents = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AccCaption") == 0) {
		return S_OK;
	}

	else return CBScriptHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CBObject::ScToString() {
	return "[object]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ShowCursor() {
	if (m_Cursor) return Game->DrawCursor(m_Cursor);
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::Persist(CBPersistMgr *PersistMgr) {
	CBScriptHolder::Persist(PersistMgr);

	for (int i = 0; i < 7; i++) PersistMgr->Transfer(TMEMBER(m_Caption[i]));
	PersistMgr->Transfer(TMEMBER(m_ActiveCursor));
	PersistMgr->Transfer(TMEMBER(m_AlphaColor));
	PersistMgr->Transfer(TMEMBER(m_AutoSoundPanning));
	PersistMgr->Transfer(TMEMBER(m_Cursor));
	PersistMgr->Transfer(TMEMBER(m_SharedCursors));
	PersistMgr->Transfer(TMEMBER(m_EditorAlwaysRegister));
	PersistMgr->Transfer(TMEMBER(m_EditorOnly));
	PersistMgr->Transfer(TMEMBER(m_EditorSelected));
	PersistMgr->Transfer(TMEMBER(m_ID));
	PersistMgr->Transfer(TMEMBER(m_Is3D));
	PersistMgr->Transfer(TMEMBER(m_Movable));
	PersistMgr->Transfer(TMEMBER(m_PosX));
	PersistMgr->Transfer(TMEMBER(m_PosY));
	PersistMgr->Transfer(TMEMBER(m_RelativeScale));
	PersistMgr->Transfer(TMEMBER(m_Rotatable));
	PersistMgr->Transfer(TMEMBER(m_Scale));
	PersistMgr->Transfer(TMEMBER(m_SFX));
	PersistMgr->Transfer(TMEMBER(m_SFXStart));
	PersistMgr->Transfer(TMEMBER(m_SFXVolume));
	PersistMgr->Transfer(TMEMBER(m_Ready));
	PersistMgr->Transfer(TMEMBER(m_Rect));
	PersistMgr->Transfer(TMEMBER(m_RectSet));
	PersistMgr->Transfer(TMEMBER(m_Registrable));
	PersistMgr->Transfer(TMEMBER(m_Shadowable));
	PersistMgr->Transfer(TMEMBER(m_SoundEvent));
	PersistMgr->Transfer(TMEMBER(m_Zoomable));

	PersistMgr->Transfer(TMEMBER(m_ScaleX));
	PersistMgr->Transfer(TMEMBER(m_ScaleY));

	PersistMgr->Transfer(TMEMBER(m_Rotate));
	PersistMgr->Transfer(TMEMBER(m_RotateValid));
	PersistMgr->Transfer(TMEMBER(m_RelativeRotate));

	PersistMgr->Transfer(TMEMBER(m_SaveState));
	PersistMgr->Transfer(TMEMBER(m_NonIntMouseEvents));

	PersistMgr->Transfer(TMEMBER_INT(m_SFXType));
	PersistMgr->Transfer(TMEMBER(m_SFXParam1));
	PersistMgr->Transfer(TMEMBER(m_SFXParam2));
	PersistMgr->Transfer(TMEMBER(m_SFXParam3));
	PersistMgr->Transfer(TMEMBER(m_SFXParam4));


	PersistMgr->Transfer(TMEMBER_INT(m_BlendMode));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetCursor(char *Filename) {
	if (!m_SharedCursors) {
		delete m_Cursor;
		m_Cursor = NULL;
	}

	m_SharedCursors = false;
	m_Cursor = new CBSprite(Game);
	if (!m_Cursor || FAILED(m_Cursor->LoadFile(Filename))) {
		delete m_Cursor;
		m_Cursor = NULL;
		return E_FAIL;
	} else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetActiveCursor(char *Filename) {
	delete m_ActiveCursor;
	m_ActiveCursor = new CBSprite(Game);
	if (!m_ActiveCursor || FAILED(m_ActiveCursor->LoadFile(Filename))) {
		delete m_ActiveCursor;
		m_ActiveCursor = NULL;
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
bool CBObject::HandleKeypress(SDL_Event *event) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::HandleMouseWheel(int Delta) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::PlaySFX(char *Filename, bool Looping, bool PlayNow, char *EventName, uint32 LoopStart) {
	// just play loaded sound
	if (Filename == NULL && m_SFX) {
		if (Game->m_EditorMode || m_SFXStart) {
			m_SFX->SetVolume(m_SFXVolume);
			m_SFX->SetPositionTime(m_SFXStart);
			if (!Game->m_EditorMode) m_SFXStart = 0;
		}
		if (PlayNow) {
			SetSoundEvent(EventName);
			if (LoopStart) m_SFX->SetLoopStart(LoopStart);
			return m_SFX->Play(Looping);
		} else return S_OK;
	}

	if (Filename == NULL) return E_FAIL;

	// create new sound
	delete m_SFX;

	m_SFX = new CBSound(Game);
	if (m_SFX && SUCCEEDED(m_SFX->SetSound(Filename, SOUND_SFX, true))) {
		m_SFX->SetVolume(m_SFXVolume);
		if (m_SFXStart) {
			m_SFX->SetPositionTime(m_SFXStart);
			m_SFXStart = 0;
		}
		m_SFX->ApplyFX(m_SFXType, m_SFXParam1, m_SFXParam2, m_SFXParam3, m_SFXParam4);
		if (PlayNow) {
			SetSoundEvent(EventName);
			if (LoopStart) m_SFX->SetLoopStart(LoopStart);
			return m_SFX->Play(Looping);
		} else return S_OK;
	} else {
		delete m_SFX;
		m_SFX = NULL;
		return E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::StopSFX(bool DeleteSound) {
	if (m_SFX) {
		m_SFX->Stop();
		if (DeleteSound) {
			delete m_SFX;
			m_SFX = NULL;
		}
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::PauseSFX() {
	if (m_SFX) return m_SFX->Pause();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ResumeSFX() {
	if (m_SFX) return m_SFX->Resume();
	else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetSFXTime(uint32 Time) {
	m_SFXStart = Time;
	if (m_SFX && m_SFX->IsPlaying()) return m_SFX->SetPositionTime(Time);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::SetSFXVolume(int Volume) {
	m_SFXVolume = Volume;
	if (m_SFX) return m_SFX->SetVolume(Volume);
	else return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::UpdateSounds() {
	if (m_SoundEvent) {
		if (m_SFX && !m_SFX->IsPlaying()) {
			ApplyEvent(m_SoundEvent);
			SetSoundEvent(NULL);
		}
	}

	if (m_SFX) UpdateOneSound(m_SFX);

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::UpdateOneSound(CBSound *Sound) {
	HRESULT Ret = S_OK;

	if (Sound) {
		if (m_AutoSoundPanning)
			Ret = Sound->SetPan(Game->m_SoundMgr->PosToPan(m_PosX  - Game->m_OffsetX, m_PosY - Game->m_OffsetY));

		Ret = Sound->ApplyFX(m_SFXType, m_SFXParam1, m_SFXParam2, m_SFXParam3, m_SFXParam4);
	}
	return Ret;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::ResetSoundPan() {
	if (!m_SFX) return S_OK;
	else {
		return m_SFX->SetPan(0.0f);
	}
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::GetExtendedFlag(char *FlagName) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CBObject::IsReady() {
	return m_Ready;
}


//////////////////////////////////////////////////////////////////////////
void CBObject::SetSoundEvent(char *EventName) {
	delete[] m_SoundEvent;
	m_SoundEvent = NULL;
	if (EventName) {
		m_SoundEvent = new char[strlen(EventName) + 1];
		if (m_SoundEvent) strcpy(m_SoundEvent, EventName);
	}
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBObject::AfterMove() {
	return S_OK;
}

} // end of namespace WinterMute
