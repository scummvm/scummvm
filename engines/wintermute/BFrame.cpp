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
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BFrame.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BSoundMgr.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BSubFrame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBFrame, false)

//////////////////////////////////////////////////////////////////////
CBFrame::CBFrame(CBGame *inGame): CBScriptable(inGame, true) {
	m_Delay = 0;
	m_MoveX = m_MoveY = 0;

	m_Sound = NULL;
	m_KillSound = false;

	m_EditorExpanded = false;
	m_Keyframe = false;
}


//////////////////////////////////////////////////////////////////////
CBFrame::~CBFrame() {
	delete m_Sound;
	m_Sound = NULL;

	int i;

	for (i = 0; i < m_Subframes.GetSize(); i++) delete m_Subframes[i];
	m_Subframes.RemoveAll();

	for (i = 0; i < m_ApplyEvent.GetSize(); i++) {
		SAFE_DELETE_ARRAY(m_ApplyEvent[i]);
	}
	m_ApplyEvent.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFrame::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, bool Precise, uint32 Alpha, bool AllFrames, float Rotate, TSpriteBlendMode BlendMode) {
	HRESULT res;

	for (int i = 0; i < m_Subframes.GetSize(); i++) {
		res = m_Subframes[i]->Draw(X, Y, Register, ZoomX, ZoomY, Precise, Alpha, Rotate, BlendMode);
		if (FAILED(res)) return res;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::OneTimeDisplay(CBObject *Owner, bool Muted) {
	if (m_Sound && !Muted) {
		if (Owner) Owner->UpdateOneSound(m_Sound);
		m_Sound->Play();
		/*
		if(Game->m_State==GAME_FROZEN)
		{
		    m_Sound->Pause(true);
		}
		*/
	}
	if (Owner) {
		for (int i = 0; i < m_ApplyEvent.GetSize(); i++) {
			Owner->ApplyEvent(m_ApplyEvent[i]);
		}
	}
	return S_OK;
}



TOKEN_DEF_START
TOKEN_DEF(DELAY)
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(RECT)
TOKEN_DEF(HOTSPOT)
TOKEN_DEF(2D_ONLY)
TOKEN_DEF(3D_ONLY)
TOKEN_DEF(MIRROR_X)
TOKEN_DEF(MIRROR_Y)
TOKEN_DEF(MOVE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(SUBFRAME)
TOKEN_DEF(SOUND)
TOKEN_DEF(KEYFRAME)
TOKEN_DEF(DECORATION)
TOKEN_DEF(APPLY_EVENT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(EDITOR_EXPANDED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(KILL_SOUND)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBFrame::LoadBuffer(byte  *Buffer, int LifeTime, bool KeepLoaded) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(DELAY)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(RECT)
	TOKEN_TABLE(HOTSPOT)
	TOKEN_TABLE(2D_ONLY)
	TOKEN_TABLE(3D_ONLY)
	TOKEN_TABLE(MIRROR_X)
	TOKEN_TABLE(MIRROR_Y)
	TOKEN_TABLE(MOVE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(SUBFRAME)
	TOKEN_TABLE(SOUND)
	TOKEN_TABLE(KEYFRAME)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(APPLY_EVENT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(EDITOR_EXPANDED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(KILL_SOUND)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);
	RECT rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	int HotspotX = 0, HotspotY = 0;
	bool custom_trans = false;
	bool editor_selected = false;
	bool Is2DOnly = false;
	bool Is3DOnly = false;
	bool Decoration = false;
	bool MirrorX = false;
	bool MirrorY = false;
	CBPlatform::SetRectEmpty(&rect);
	char *surface_file = NULL;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_DELAY:
			parser.ScanStr(params, "%d", &m_Delay);
			break;

		case TOKEN_IMAGE:
			surface_file = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			custom_trans = true;
			break;

		case TOKEN_RECT:
			parser.ScanStr(params, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			break;

		case TOKEN_HOTSPOT:
			parser.ScanStr(params, "%d,%d", &HotspotX, &HotspotY);
			break;

		case TOKEN_MOVE:
			parser.ScanStr(params, "%d,%d", &m_MoveX, &m_MoveY);
			break;

		case TOKEN_2D_ONLY:
			parser.ScanStr(params, "%b", &Is2DOnly);
			break;

		case TOKEN_3D_ONLY:
			parser.ScanStr(params, "%b", &Is3DOnly);
			break;

		case TOKEN_MIRROR_X:
			parser.ScanStr(params, "%b", &MirrorX);
			break;

		case TOKEN_MIRROR_Y:
			parser.ScanStr(params, "%b", &MirrorY);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr(params, "%b", &editor_selected);
			break;

		case TOKEN_EDITOR_EXPANDED:
			parser.ScanStr(params, "%b", &m_EditorExpanded);
			break;

		case TOKEN_KILL_SOUND:
			parser.ScanStr(params, "%b", &m_KillSound);
			break;

		case TOKEN_SUBFRAME: {
			CBSubFrame *subframe = new CBSubFrame(Game);
			if (!subframe || FAILED(subframe->LoadBuffer((byte  *)params, LifeTime, KeepLoaded))) {
				delete subframe;
				cmd = PARSERR_GENERIC;
			} else m_Subframes.Add(subframe);
		}
		break;

		case TOKEN_SOUND: {
			if (m_Sound) {
				delete m_Sound;
				m_Sound = NULL;
			}
			m_Sound = new CBSound(Game);
			if (!m_Sound || FAILED(m_Sound->SetSound(params, SOUND_SFX, false))) {
				if (Game->m_SoundMgr->m_SoundAvailable) Game->LOG(0, "Error loading sound '%s'.", params);
				delete m_Sound;
				m_Sound = NULL;
			}
		}
		break;

		case TOKEN_APPLY_EVENT: {
			char *Event = new char[strlen(params) + 1];
			strcpy(Event, params);
			m_ApplyEvent.Add(Event);
		}
		break;

		case TOKEN_KEYFRAME:
			parser.ScanStr(params, "%b", &m_Keyframe);
			break;

		case TOKEN_DECORATION:
			parser.ScanStr(params, "%b", &Decoration);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty((byte  *)params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in FRAME definition");
		return E_FAIL;
	}

	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading FRAME definition");
		return E_FAIL;
	}


	CBSubFrame *sub = new CBSubFrame(Game);
	if (surface_file != NULL) {
		if (custom_trans) sub->SetSurface(surface_file, false, r, g, b, LifeTime, KeepLoaded);
		else sub->SetSurface(surface_file, true, 0, 0, 0, LifeTime, KeepLoaded);

		if (!sub->m_Surface) {
			delete sub;
			Game->LOG(0, "Error loading SUBFRAME");
			return E_FAIL;
		}

		sub->m_Alpha = DRGBA(ar, ag, ab, alpha);
		if (custom_trans) sub->m_Transparent = DRGBA(r, g, b, 0xFF);
	}

	if (CBPlatform::IsRectEmpty(&rect)) sub->SetDefaultRect();
	else sub->m_Rect = rect;

	sub->m_HotspotX = HotspotX;
	sub->m_HotspotY = HotspotY;
	sub->m_2DOnly = Is2DOnly;
	sub->m_3DOnly = Is3DOnly;
	sub->m_Decoration = Decoration;
	sub->m_MirrorX = MirrorX;
	sub->m_MirrorY = MirrorY;


	sub->m_EditorSelected = editor_selected;
	m_Subframes.InsertAt(0, sub);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFrame::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;
	CBPlatform::SetRectEmpty(Rect);

	RECT SubRect;

	for (int i = 0; i < m_Subframes.GetSize(); i++) {
		m_Subframes[i]->GetBoundingRect(&SubRect, X, Y, ScaleX, ScaleY);
		CBPlatform::UnionRect(Rect, Rect, &SubRect);
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "FRAME {\n");
	Buffer->PutTextIndent(Indent + 2, "DELAY = %d\n", m_Delay);

	if (m_MoveX != 0 || m_MoveY != 0)
		Buffer->PutTextIndent(Indent + 2, "MOVE {%d, %d}\n", m_MoveX, m_MoveY);

	if (m_Sound && m_Sound->m_SoundFilename)
		Buffer->PutTextIndent(Indent + 2, "SOUND=\"%s\"\n", m_Sound->m_SoundFilename);

	Buffer->PutTextIndent(Indent + 2, "KEYFRAME=%s\n", m_Keyframe ? "TRUE" : "FALSE");

	if (m_KillSound)
		Buffer->PutTextIndent(Indent + 2, "KILL_SOUND=%s\n", m_KillSound ? "TRUE" : "FALSE");

	if (m_EditorExpanded)
		Buffer->PutTextIndent(Indent + 2, "EDITOR_EXPANDED=%s\n", m_EditorExpanded ? "TRUE" : "FALSE");

	if (m_Subframes.GetSize() > 0) m_Subframes[0]->SaveAsText(Buffer, Indent, false);

	int i;
	for (i = 1; i < m_Subframes.GetSize(); i++) {
		m_Subframes[i]->SaveAsText(Buffer, Indent + 2);
	}

	for (i = 0; i < m_ApplyEvent.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "APPLY_EVENT=\"%s\"\n", m_ApplyEvent[i]);
	}

	CBBase::SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::Persist(CBPersistMgr *PersistMgr) {
	CBScriptable::Persist(PersistMgr);

	m_ApplyEvent.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_Delay));
	PersistMgr->Transfer(TMEMBER(m_EditorExpanded));
	PersistMgr->Transfer(TMEMBER(m_Keyframe));
	PersistMgr->Transfer(TMEMBER(m_KillSound));
	PersistMgr->Transfer(TMEMBER(m_MoveX));
	PersistMgr->Transfer(TMEMBER(m_MoveY));
	PersistMgr->Transfer(TMEMBER(m_Sound));
	m_Subframes.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// GetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetSound") == 0) {
		Stack->CorrectParams(0);

		if (m_Sound && m_Sound->m_SoundFilename) Stack->PushString(m_Sound->m_SoundFilename);
		else Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSound") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		delete m_Sound;
		m_Sound = NULL;

		if (!Val->IsNULL()) {
			m_Sound = new CBSound(Game);
			if (!m_Sound || FAILED(m_Sound->SetSound(Val->GetString(), SOUND_SFX, false))) {
				Stack->PushBool(false);
				delete m_Sound;
				m_Sound = NULL;
			} else Stack->PushBool(true);
		} else Stack->PushBool(true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSubframe
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetSubframe") == 0) {
		Stack->CorrectParams(1);
		int Index = Stack->Pop()->GetInt(-1);
		if (Index < 0 || Index >= m_Subframes.GetSize()) {
			Script->RuntimeError("Frame.GetSubframe: Subframe index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushNative(m_Subframes[Index], true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteSubframe") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		if (Val->IsInt()) {
			int Index = Val->GetInt(-1);
			if (Index < 0 || Index >= m_Subframes.GetSize()) {
				Script->RuntimeError("Frame.DeleteSubframe: Subframe index %d is out of range.", Index);
			}
		} else {
			CBSubFrame *Sub = (CBSubFrame *)Val->GetNative();
			for (int i = 0; i < m_Subframes.GetSize(); i++) {
				if (m_Subframes[i] == Sub) {
					delete m_Subframes[i];
					m_Subframes.RemoveAt(i);
					break;
				}
			}
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddSubframe") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBSubFrame *Sub = new CBSubFrame(Game);
		if (Filename != NULL) {
			Sub->SetSurface(Filename);
			Sub->SetDefaultRect();
		}
		m_Subframes.Add(Sub);

		Stack->PushNative(Sub, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InsertSubframe") == 0) {
		Stack->CorrectParams(2);
		int Index = Stack->Pop()->GetInt();
		if (Index < 0) Index = 0;

		CScValue *Val = Stack->Pop();
		char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBSubFrame *Sub = new CBSubFrame(Game);
		if (Filename != NULL) {
			Sub->SetSurface(Filename);
		}

		if (Index >= m_Subframes.GetSize()) m_Subframes.Add(Sub);
		else m_Subframes.InsertAt(Index, Sub);

		Stack->PushNative(Sub, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSubframe") == 0) {
		Stack->CorrectParams(1);
		int Index = Stack->Pop()->GetInt(-1);
		if (Index < 0 || Index >= m_ApplyEvent.GetSize()) {
			Script->RuntimeError("Frame.GetEvent: Event index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushString(m_ApplyEvent[Index]);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddEvent") == 0) {
		Stack->CorrectParams(1);
		char *Event = Stack->Pop()->GetString();
		for (int i = 0; i < m_ApplyEvent.GetSize(); i++) {
			if (scumm_stricmp(m_ApplyEvent[i], Event) == 0) {
				Stack->PushNULL();
				return S_OK;
			}
		}
		m_ApplyEvent.Add(Event);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteEvent") == 0) {
		Stack->CorrectParams(1);
		char *Event = Stack->Pop()->GetString();
		for (int i = 0; i < m_ApplyEvent.GetSize(); i++) {
			if (scumm_stricmp(m_ApplyEvent[i], Event) == 0) {
				delete [] m_ApplyEvent[i];
				m_ApplyEvent.RemoveAt(i);
				break;
			}
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (m_Subframes.GetSize() == 1) return m_Subframes[0]->ScCallMethod(Script, Stack, ThisStack, Name);
		else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBFrame::ScGetProperty(char *Name) {
	if (!m_ScValue) m_ScValue = new CScValue(Game);
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("frame");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Delay") == 0) {
		m_ScValue->SetInt(m_Delay);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Keyframe") == 0) {
		m_ScValue->SetBool(m_Keyframe);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KillSounds") == 0) {
		m_ScValue->SetBool(m_KillSound);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveX") == 0) {
		m_ScValue->SetInt(m_MoveX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveY") == 0) {
		m_ScValue->SetInt(m_MoveY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumSubframes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumSubframes") == 0) {
		m_ScValue->SetInt(m_Subframes.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumEvents (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumEvents") == 0) {
		m_ScValue->SetInt(m_ApplyEvent.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (m_Subframes.GetSize() == 1) return m_Subframes[0]->ScGetProperty(Name);
		else return CBScriptable::ScGetProperty(Name);
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Delay") == 0) {
		m_Delay = MAX(0, Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Keyframe") == 0) {
		m_Keyframe = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KillSounds") == 0) {
		m_KillSound = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveX") == 0) {
		m_MoveX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveY") == 0) {
		m_MoveY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (m_Subframes.GetSize() == 1) return m_Subframes[0]->ScSetProperty(Name, Value);
		else return CBScriptable::ScSetProperty(Name, Value);
	}
}


//////////////////////////////////////////////////////////////////////////
char *CBFrame::ScToString() {
	return "[frame]";
}

} // end of namespace WinterMute
