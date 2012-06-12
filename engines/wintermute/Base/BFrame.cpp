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
#include "engines/wintermute/Base/BParser.h"
#include "engines/wintermute/Base/BFrame.h"
#include "engines/wintermute/Base/BGame.h"
#include "engines/wintermute/Base/BDynBuffer.h"
#include "engines/wintermute/Base/BSoundMgr.h"
#include "engines/wintermute/Base/BSound.h"
#include "engines/wintermute/Base/BSubFrame.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/Base/scriptables/ScValue.h"
#include "engines/wintermute/Base/scriptables/ScScript.h"
#include "engines/wintermute/Base/scriptables/ScStack.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBFrame, false)

//////////////////////////////////////////////////////////////////////
CBFrame::CBFrame(CBGame *inGame): CBScriptable(inGame, true) {
	_delay = 0;
	_moveX = _moveY = 0;

	_sound = NULL;
	_killSound = false;

	_editorExpanded = false;
	_keyframe = false;
}


//////////////////////////////////////////////////////////////////////
CBFrame::~CBFrame() {
	delete _sound;
	_sound = NULL;

	int i;

	for (i = 0; i < _subframes.GetSize(); i++) delete _subframes[i];
	_subframes.RemoveAll();

	for (i = 0; i < _applyEvent.GetSize(); i++) {
		delete[] _applyEvent[i];
		_applyEvent[i] = NULL;
	}
	_applyEvent.RemoveAll();
}


//////////////////////////////////////////////////////////////////////
HRESULT CBFrame::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, bool Precise, uint32 Alpha, bool AllFrames, float Rotate, TSpriteBlendMode BlendMode) {
	HRESULT res;

	for (int i = 0; i < _subframes.GetSize(); i++) {
		res = _subframes[i]->Draw(X, Y, Register, ZoomX, ZoomY, Precise, Alpha, Rotate, BlendMode);
		if (FAILED(res)) return res;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::OneTimeDisplay(CBObject *Owner, bool Muted) {
	if (_sound && !Muted) {
		if (Owner) Owner->UpdateOneSound(_sound);
		_sound->Play();
		/*
		if (Game->_state == GAME_FROZEN) {
		    _sound->Pause(true);
		}
		*/
	}
	if (Owner) {
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			Owner->ApplyEvent(_applyEvent[i]);
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
	bool custo_trans = false;
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
			parser.ScanStr(params, "%d", &_delay);
			break;

		case TOKEN_IMAGE:
			surface_file = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.ScanStr(params, "%d,%d,%d", &r, &g, &b);
			custo_trans = true;
			break;

		case TOKEN_RECT:
			parser.ScanStr(params, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			break;

		case TOKEN_HOTSPOT:
			parser.ScanStr(params, "%d,%d", &HotspotX, &HotspotY);
			break;

		case TOKEN_MOVE:
			parser.ScanStr(params, "%d,%d", &_moveX, &_moveY);
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
			parser.ScanStr(params, "%b", &_editorExpanded);
			break;

		case TOKEN_KILL_SOUND:
			parser.ScanStr(params, "%b", &_killSound);
			break;

		case TOKEN_SUBFRAME: {
			CBSubFrame *subframe = new CBSubFrame(Game);
			if (!subframe || FAILED(subframe->LoadBuffer((byte *)params, LifeTime, KeepLoaded))) {
				delete subframe;
				cmd = PARSERR_GENERIC;
			} else _subframes.Add(subframe);
		}
		break;

		case TOKEN_SOUND: {
			if (_sound) {
				delete _sound;
				_sound = NULL;
			}
			_sound = new CBSound(Game);
			if (!_sound || FAILED(_sound->SetSound(params, SOUND_SFX, false))) {
				if (Game->_soundMgr->_soundAvailable) Game->LOG(0, "Error loading sound '%s'.", params);
				delete _sound;
				_sound = NULL;
			}
		}
		break;

		case TOKEN_APPLY_EVENT: {
			char *Event = new char[strlen(params) + 1];
			strcpy(Event, params);
			_applyEvent.Add(Event);
		}
		break;

		case TOKEN_KEYFRAME:
			parser.ScanStr(params, "%b", &_keyframe);
			break;

		case TOKEN_DECORATION:
			parser.ScanStr(params, "%b", &Decoration);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty((byte *)params, false);
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
		if (custo_trans) sub->SetSurface(surface_file, false, r, g, b, LifeTime, KeepLoaded);
		else sub->SetSurface(surface_file, true, 0, 0, 0, LifeTime, KeepLoaded);

		if (!sub->_surface) {
			delete sub;
			Game->LOG(0, "Error loading SUBFRAME");
			return E_FAIL;
		}

		sub->_alpha = DRGBA(ar, ag, ab, alpha);
		if (custo_trans) sub->_transparent = DRGBA(r, g, b, 0xFF);
	}

	if (CBPlatform::IsRectEmpty(&rect)) sub->SetDefaultRect();
	else sub->_rect = rect;

	sub->_hotspotX = HotspotX;
	sub->_hotspotY = HotspotY;
	sub->_2DOnly = Is2DOnly;
	sub->_3DOnly = Is3DOnly;
	sub->_decoration = Decoration;
	sub->_mirrorX = MirrorX;
	sub->_mirrorY = MirrorY;


	sub->_editorSelected = editor_selected;
	_subframes.InsertAt(0, sub);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CBFrame::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;
	CBPlatform::SetRectEmpty(Rect);

	RECT SubRect;

	for (int i = 0; i < _subframes.GetSize(); i++) {
		_subframes[i]->GetBoundingRect(&SubRect, X, Y, ScaleX, ScaleY);
		CBPlatform::UnionRect(Rect, Rect, &SubRect);
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "FRAME {\n");
	Buffer->PutTextIndent(Indent + 2, "DELAY = %d\n", _delay);

	if (_moveX != 0 || _moveY != 0)
		Buffer->PutTextIndent(Indent + 2, "MOVE {%d, %d}\n", _moveX, _moveY);

	if (_sound && _sound->_soundFilename)
		Buffer->PutTextIndent(Indent + 2, "SOUND=\"%s\"\n", _sound->_soundFilename);

	Buffer->PutTextIndent(Indent + 2, "KEYFRAME=%s\n", _keyframe ? "TRUE" : "FALSE");

	if (_killSound)
		Buffer->PutTextIndent(Indent + 2, "KILL_SOUND=%s\n", _killSound ? "TRUE" : "FALSE");

	if (_editorExpanded)
		Buffer->PutTextIndent(Indent + 2, "EDITOR_EXPANDED=%s\n", _editorExpanded ? "TRUE" : "FALSE");

	if (_subframes.GetSize() > 0) _subframes[0]->SaveAsText(Buffer, Indent, false);

	int i;
	for (i = 1; i < _subframes.GetSize(); i++) {
		_subframes[i]->SaveAsText(Buffer, Indent + 2);
	}

	for (i = 0; i < _applyEvent.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "APPLY_EVENT=\"%s\"\n", _applyEvent[i]);
	}

	CBBase::SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::Persist(CBPersistMgr *PersistMgr) {
	CBScriptable::Persist(PersistMgr);

	_applyEvent.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(_delay));
	PersistMgr->Transfer(TMEMBER(_editorExpanded));
	PersistMgr->Transfer(TMEMBER(_keyframe));
	PersistMgr->Transfer(TMEMBER(_killSound));
	PersistMgr->Transfer(TMEMBER(_moveX));
	PersistMgr->Transfer(TMEMBER(_moveY));
	PersistMgr->Transfer(TMEMBER(_sound));
	_subframes.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// GetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetSound") == 0) {
		Stack->CorrectParams(0);

		if (_sound && _sound->_soundFilename) Stack->PushString(_sound->_soundFilename);
		else Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetSound") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		delete _sound;
		_sound = NULL;

		if (!Val->IsNULL()) {
			_sound = new CBSound(Game);
			if (!_sound || FAILED(_sound->SetSound(Val->GetString(), SOUND_SFX, false))) {
				Stack->PushBool(false);
				delete _sound;
				_sound = NULL;
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
		if (Index < 0 || Index >= _subframes.GetSize()) {
			Script->RuntimeError("Frame.GetSubframe: Subframe index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushNative(_subframes[Index], true);

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
			if (Index < 0 || Index >= _subframes.GetSize()) {
				Script->RuntimeError("Frame.DeleteSubframe: Subframe index %d is out of range.", Index);
			}
		} else {
			CBSubFrame *Sub = (CBSubFrame *)Val->GetNative();
			for (int i = 0; i < _subframes.GetSize(); i++) {
				if (_subframes[i] == Sub) {
					delete _subframes[i];
					_subframes.RemoveAt(i);
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
		const char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBSubFrame *Sub = new CBSubFrame(Game);
		if (Filename != NULL) {
			Sub->SetSurface(Filename);
			Sub->SetDefaultRect();
		}
		_subframes.Add(Sub);

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
		const char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBSubFrame *Sub = new CBSubFrame(Game);
		if (Filename != NULL) {
			Sub->SetSurface(Filename);
		}

		if (Index >= _subframes.GetSize()) _subframes.Add(Sub);
		else _subframes.InsertAt(Index, Sub);

		Stack->PushNative(Sub, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetSubframe") == 0) {
		Stack->CorrectParams(1);
		int Index = Stack->Pop()->GetInt(-1);
		if (Index < 0 || Index >= _applyEvent.GetSize()) {
			Script->RuntimeError("Frame.GetEvent: Event index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushString(_applyEvent[Index]);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddEvent") == 0) {
		Stack->CorrectParams(1);
		const char *Event = Stack->Pop()->GetString();
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			if (scumm_stricmp(_applyEvent[i], Event) == 0) {
				Stack->PushNULL();
				return S_OK;
			}
		}
		_applyEvent.Add(Event);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteEvent") == 0) {
		Stack->CorrectParams(1);
		const char *Event = Stack->Pop()->GetString();
		for (int i = 0; i < _applyEvent.GetSize(); i++) {
			if (scumm_stricmp(_applyEvent[i], Event) == 0) {
				delete [] _applyEvent[i];
				_applyEvent.RemoveAt(i);
				break;
			}
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->ScCallMethod(Script, Stack, ThisStack, Name);
		else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
	}
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBFrame::ScGetProperty(const char *Name) {
	if (!_scValue) _scValue = new CScValue(Game);
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("frame");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Delay") == 0) {
		_scValue->SetInt(_delay);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Keyframe") == 0) {
		_scValue->SetBool(_keyframe);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KillSounds") == 0) {
		_scValue->SetBool(_killSound);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveX") == 0) {
		_scValue->SetInt(_moveX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveY") == 0) {
		_scValue->SetInt(_moveY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumSubframes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumSubframes") == 0) {
		_scValue->SetInt(_subframes.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumEvents (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumEvents") == 0) {
		_scValue->SetInt(_applyEvent.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->ScGetProperty(Name);
		else return CBScriptable::ScGetProperty(Name);
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBFrame::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Delay") == 0) {
		_delay = MAX(0, Value->GetInt());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Keyframe") == 0) {
		_keyframe = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "KillSounds") == 0) {
		_killSound = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveX") == 0) {
		_moveX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MoveY") == 0) {
		_moveY = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.GetSize() == 1) return _subframes[0]->ScSetProperty(Name, Value);
		else return CBScriptable::ScSetProperty(Name, Value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *CBFrame::ScToString() {
	return "[frame]";
}

} // end of namespace WinterMute
