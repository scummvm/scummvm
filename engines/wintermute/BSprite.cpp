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
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/StringUtil.h"
#include "engines/wintermute/PathUtil.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BSurface.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BFrame.h"
#include "engines/wintermute/BSound.h"
#include "engines/wintermute/BSubFrame.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSprite, false)

//////////////////////////////////////////////////////////////////////
CBSprite::CBSprite(CBGame *inGame, CBObject *Owner): CBScriptHolder(inGame) {
	_editorAllFrames = false;
	_owner = Owner;
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////
CBSprite::~CBSprite() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::SetDefaults() {
	_currentFrame = -1;
	_looping = false;
	_lastFrameTime = 0;
	_filename = NULL;
	_finished = false;
	_changed = false;
	_paused = false;
	_continuous = false;
	_moveX = _moveY = 0;

	_editorMuted = false;
	_editorBgFile = NULL;
	_editorBgOffsetX = _editorBgOffsetY = 0;
	_editorBgAlpha = 0xFF;
	_streamed = false;
	_streamedKeepLoaded = false;

	SetName("");

	_precise = true;
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::Cleanup() {
	CBScriptHolder::Cleanup();

	for (int i = 0; i < _frames.GetSize(); i++)
		delete _frames[i];
	_frames.RemoveAll();

	SAFE_DELETE_ARRAY(_editorBgFile);

	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha) {
	GetCurrentFrame(ZoomX, ZoomY);
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return S_OK;

	// move owner if allowed to
	if (_changed && _owner && _owner->_movable) {
		_owner->_posX += _moveX;
		_owner->_posY += _moveY;
		_owner->AfterMove();

		X = _owner->_posX;
		Y = _owner->_posY;
	}

	// draw frame
	return Display(X, Y, Register, ZoomX, ZoomY, Alpha);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::LoadFile(const char *Filename, int LifeTime, TSpriteCacheType CacheType) {
	CBFile *File = Game->_fileManager->OpenFile(Filename);
	if (!File) {
		Game->LOG(0, "CBSprite::LoadFile failed for file '%s'", Filename);
		if (Game->_dEBUG_DebugMode) return LoadFile("invalid_debug.bmp", LifeTime, CacheType);
		else return LoadFile("invalid.bmp", LifeTime, CacheType);
	} else {
		Game->_fileManager->CloseFile(File);
		File = NULL;
	}

	HRESULT ret;

	AnsiString ext = PathUtil::GetExtension(Filename);
	if (StringUtil::StartsWith(Filename, "savegame:", true) || StringUtil::CompareNoCase(ext, "bmp") || StringUtil::CompareNoCase(ext, "tga") || StringUtil::CompareNoCase(ext, "png") || StringUtil::CompareNoCase(ext, "jpg")) {
		CBFrame *frame = new CBFrame(Game);
		CBSubFrame *subframe = new CBSubFrame(Game);
		subframe->SetSurface(Filename, true, 0, 0, 0, LifeTime, true);
		if (subframe->_surface == NULL) {
			Game->LOG(0, "Error loading simple sprite '%s'", Filename);
			ret = E_FAIL;
			delete frame;
			delete subframe;
		} else {
			CBPlatform::SetRect(&subframe->_rect, 0, 0, subframe->_surface->GetWidth(), subframe->_surface->GetHeight());
			frame->_subframes.Add(subframe);
			_frames.Add(frame);
			_currentFrame = 0;
			ret = S_OK;
		}
	} else {
		byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
		if (Buffer) {
			if (FAILED(ret = LoadBuffer(Buffer, true, LifeTime, CacheType))) Game->LOG(0, "Error parsing SPRITE file '%s'", Filename);
			delete [] Buffer;
		}
	}

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);


	return ret;
}



TOKEN_DEF_START
TOKEN_DEF(CONTINUOUS)
TOKEN_DEF(SPRITE)
TOKEN_DEF(LOOPING)
TOKEN_DEF(FRAME)
TOKEN_DEF(NAME)
TOKEN_DEF(PRECISE)
TOKEN_DEF(EDITOR_MUTED)
TOKEN_DEF(STREAMED_KEEP_LOADED)
TOKEN_DEF(STREAMED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_BG_FILE)
TOKEN_DEF(EDITOR_BG_OFFSET_X)
TOKEN_DEF(EDITOR_BG_OFFSET_Y)
TOKEN_DEF(EDITOR_BG_ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::LoadBuffer(byte  *Buffer, bool Complete, int LifeTime, TSpriteCacheType CacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(CONTINUOUS)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(LOOPING)
	TOKEN_TABLE(FRAME)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(PRECISE)
	TOKEN_TABLE(EDITOR_MUTED)
	TOKEN_TABLE(STREAMED_KEEP_LOADED)
	TOKEN_TABLE(STREAMED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_BG_FILE)
	TOKEN_TABLE(EDITOR_BG_OFFSET_X)
	TOKEN_TABLE(EDITOR_BG_OFFSET_Y)
	TOKEN_TABLE(EDITOR_BG_ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	CBParser parser(Game);

	Cleanup();


	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_SPRITE) {
			Game->LOG(0, "'SPRITE' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	int frame_count = 1;
	CBFrame *frame;
	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_CONTINUOUS:
			parser.ScanStr((char *)params, "%b", &_continuous);
			break;

		case TOKEN_EDITOR_MUTED:
			parser.ScanStr((char *)params, "%b", &_editorMuted);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_LOOPING:
			parser.ScanStr((char *)params, "%b", &_looping);
			break;

		case TOKEN_PRECISE:
			parser.ScanStr((char *)params, "%b", &_precise);
			break;

		case TOKEN_STREAMED:
			parser.ScanStr((char *)params, "%b", &_streamed);
			if (_streamed && LifeTime == -1) {
				LifeTime = 500;
				CacheType = CACHE_ALL;
			}
			break;

		case TOKEN_STREAMED_KEEP_LOADED:
			parser.ScanStr((char *)params, "%b", &_streamedKeepLoaded);
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_EDITOR_BG_FILE:
			if (Game->_editorMode) {
				SAFE_DELETE_ARRAY(_editorBgFile);
				_editorBgFile = new char[strlen((char *)params) + 1];
				if (_editorBgFile) strcpy(_editorBgFile, (char *)params);
			}
			break;

		case TOKEN_EDITOR_BG_OFFSET_X:
			parser.ScanStr((char *)params, "%d", &_editorBgOffsetX);
			break;

		case TOKEN_EDITOR_BG_OFFSET_Y:
			parser.ScanStr((char *)params, "%d", &_editorBgOffsetY);
			break;

		case TOKEN_EDITOR_BG_ALPHA:
			parser.ScanStr((char *)params, "%d", &_editorBgAlpha);
			_editorBgAlpha = std::min(_editorBgAlpha, 255);
			_editorBgAlpha = std::max(_editorBgAlpha, 0);
			break;

		case TOKEN_FRAME: {
			int FrameLifeTime = LifeTime;
			if (CacheType == CACHE_HALF && frame_count % 2 != 1) FrameLifeTime = -1;

			frame = new CBFrame(Game);

			if (FAILED(frame->LoadBuffer(params, FrameLifeTime, _streamedKeepLoaded))) {
				delete frame;
				Game->LOG(0, "Error parsing frame %d", frame_count);
				return E_FAIL;
			}

			_frames.Add(frame);
			frame_count++;
			if (_currentFrame == -1) _currentFrame = 0;
		}
		break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SPRITE definition");
		return E_FAIL;
	}
	_canBreak = !_continuous;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CBSprite::Reset() {
	if (_frames.GetSize() > 0) _currentFrame = 0;
	else _currentFrame = -1;

	KillAllSounds();

	_lastFrameTime = 0;
	_finished = false;
	_moveX = _moveY = 0;
}


//////////////////////////////////////////////////////////////////////
bool CBSprite::GetCurrentFrame(float ZoomX, float ZoomY) {
	//if(_owner && _owner->_freezable && Game->_state == GAME_FROZEN) return true;

	if (_currentFrame == -1) return false;

	uint32 timer;
	if (_owner && _owner->_freezable) timer = Game->_timer;
	else timer = Game->_liveTimer;

	int LastFrame = _currentFrame;

	// get current frame
	if (!_paused && !_finished && timer >= _lastFrameTime + _frames[_currentFrame]->_delay && _lastFrameTime != 0) {
		if (_currentFrame < _frames.GetSize() - 1) {
			_currentFrame++;
			if (_continuous) _canBreak = (_currentFrame == _frames.GetSize() - 1);
		} else {
			if (_looping) {
				_currentFrame = 0;
				_canBreak = true;
			} else {
				_finished = true;
				_canBreak = true;
			}
		}

		_lastFrameTime = timer;
	}

	_changed = (LastFrame != _currentFrame || (_looping && _frames.GetSize() == 1));

	if (_lastFrameTime == 0) {
		_lastFrameTime = timer;
		_changed = true;
		if (_continuous) _canBreak = (_currentFrame == _frames.GetSize() - 1);
	}

	if (_changed) {
		_moveX = _frames[_currentFrame]->_moveX;
		_moveY = _frames[_currentFrame]->_moveY;

		if (ZoomX != 100 || ZoomY != 100) {
			_moveX = (int)((float)_moveX * (float)(ZoomX / 100.0f));
			_moveY = (int)((float)_moveY * (float)(ZoomY / 100.0f));
		}
	}

	return _changed;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Display(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode) {
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return S_OK;

	// on change...
	if (_changed) {
		if (_frames[_currentFrame]->_killSound) {
			KillAllSounds();
		}
		ApplyEvent("FrameChanged");
		_frames[_currentFrame]->OneTimeDisplay(_owner, Game->_editorMode && _editorMuted);
	}

	// draw frame
	return _frames[_currentFrame]->Draw(X - Game->_offsetX, Y - Game->_offsetY, Register, ZoomX, ZoomY, _precise, Alpha, _editorAllFrames, Rotate, BlendMode);
}


//////////////////////////////////////////////////////////////////////////
CBSurface *CBSprite::GetSurface() {
	// only used for animated textures for 3D models
	if (_currentFrame < 0 || _currentFrame >= _frames.GetSize()) return NULL;
	CBFrame *Frame = _frames[_currentFrame];
	if (Frame && Frame->_subframes.GetSize() > 0) {
		CBSubFrame *Subframe = Frame->_subframes[0];
		if (Subframe) return Subframe->_surface;
		else return NULL;
	} else return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBSprite::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;

	CBPlatform::SetRectEmpty(Rect);
	for (int i = 0; i < _frames.GetSize(); i++) {
		RECT frame;
		RECT temp;
		CBPlatform::CopyRect(&temp, Rect);
		_frames[i]->GetBoundingRect(&frame, X, Y, ScaleX, ScaleY);
		CBPlatform::UnionRect(Rect, &temp, &frame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "SPRITE {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "LOOPING=%s\n", _looping ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CONTINUOUS=%s\n", _continuous ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PRECISE=%s\n", _precise ? "TRUE" : "FALSE");
	if (_streamed) {
		Buffer->PutTextIndent(Indent + 2, "STREAMED=%s\n", _streamed ? "TRUE" : "FALSE");

		if (_streamedKeepLoaded)
			Buffer->PutTextIndent(Indent + 2, "STREAMED_KEEP_LOADED=%s\n", _streamedKeepLoaded ? "TRUE" : "FALSE");
	}

	if (_editorMuted)
		Buffer->PutTextIndent(Indent + 2, "EDITOR_MUTED=%s\n", _editorMuted ? "TRUE" : "FALSE");

	if (_editorBgFile) {
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_FILE=\"%s\"\n", _editorBgFile);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_OFFSET_X=%d\n", _editorBgOffsetX);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_OFFSET_Y=%d\n", _editorBgOffsetY);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_ALPHA=%d\n", _editorBgAlpha);
	}

	CBScriptHolder::SaveAsText(Buffer, Indent + 2);

	int i;

	// scripts
	for (i = 0; i < _scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}


	for (i = 0; i < _frames.GetSize(); i++) {
		_frames[i]->SaveAsText(Buffer, Indent + 2);
	}

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Persist(CBPersistMgr *PersistMgr) {
	CBScriptHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_canBreak));
	PersistMgr->Transfer(TMEMBER(_changed));
	PersistMgr->Transfer(TMEMBER(_paused));
	PersistMgr->Transfer(TMEMBER(_continuous));
	PersistMgr->Transfer(TMEMBER(_currentFrame));
	PersistMgr->Transfer(TMEMBER(_editorAllFrames));
	PersistMgr->Transfer(TMEMBER(_editorBgAlpha));
	PersistMgr->Transfer(TMEMBER(_editorBgFile));
	PersistMgr->Transfer(TMEMBER(_editorBgOffsetX));
	PersistMgr->Transfer(TMEMBER(_editorBgOffsetY));
	PersistMgr->Transfer(TMEMBER(_editorMuted));
	PersistMgr->Transfer(TMEMBER(_finished));

	_frames.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_lastFrameTime));
	PersistMgr->Transfer(TMEMBER(_looping));
	PersistMgr->Transfer(TMEMBER(_moveX));
	PersistMgr->Transfer(TMEMBER(_moveY));
	PersistMgr->Transfer(TMEMBER(_owner));
	PersistMgr->Transfer(TMEMBER(_precise));
	PersistMgr->Transfer(TMEMBER(_streamed));
	PersistMgr->Transfer(TMEMBER(_streamedKeepLoaded));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, const char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetFrame") == 0) {
		Stack->CorrectParams(1);
		int Index = Stack->Pop()->GetInt(-1);
		if (Index < 0 || Index >= _frames.GetSize()) {
			Script->RuntimeError("Sprite.GetFrame: Frame index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushNative(_frames[Index], true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteFrame") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		if (Val->IsInt()) {
			int Index = Val->GetInt(-1);
			if (Index < 0 || Index >= _frames.GetSize()) {
				Script->RuntimeError("Sprite.DeleteFrame: Frame index %d is out of range.", Index);
			}
		} else {
			CBFrame *Frame = (CBFrame *)Val->GetNative();
			for (int i = 0; i < _frames.GetSize(); i++) {
				if (_frames[i] == Frame) {
					if (i == _currentFrame) _lastFrameTime = 0;
					delete _frames[i];
					_frames.RemoveAt(i);
					break;
				}
			}
		}
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Reset") == 0) {
		Stack->CorrectParams(0);
		Reset();
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AddFrame") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();
		const char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->SetSurface(Filename))) {
				Sub->SetDefaultRect();
				Frame->_subframes.Add(Sub);
			} else delete Sub;
		}
		_frames.Add(Frame);

		Stack->PushNative(Frame, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InsertFrame") == 0) {
		Stack->CorrectParams(2);
		int Index = Stack->Pop()->GetInt();
		if (Index < 0) Index = 0;

		CScValue *Val = Stack->Pop();
		const char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->SetSurface(Filename))) Frame->_subframes.Add(Sub);
			else delete Sub;
		}

		if (Index >= _frames.GetSize()) _frames.Add(Frame);
		else _frames.InsertAt(Index, Frame);

		Stack->PushNative(Frame, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pause") == 0) {
		Stack->CorrectParams(0);
		_paused = true;
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Play") == 0) {
		Stack->CorrectParams(0);
		_paused = false;
		Stack->PushNULL();
		return S_OK;
	}

	else return CBScriptHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSprite::ScGetProperty(const char *Name) {
	_scValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		_scValue->SetString("sprite");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFrames (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumFrames") == 0) {
		_scValue->SetInt(_frames.GetSize());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CurrentFrame") == 0) {
		_scValue->SetInt(_currentFrame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		_scValue->SetBool(_precise);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Looping") == 0) {
		_scValue->SetBool(_looping);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Owner (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Owner") == 0) {
		if (_owner == NULL) _scValue->SetNULL();
		else _scValue->SetNative(_owner, true);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finished (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Finished") == 0) {
		_scValue->SetBool(_finished);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Paused (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Paused") == 0) {
		_scValue->SetBool(_paused);
		return _scValue;
	}

	else return CBScriptHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::ScSetProperty(const char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "CurrentFrame") == 0) {
		_currentFrame = Value->GetInt(0);
		if (_currentFrame >= _frames.GetSize() || _currentFrame < 0) {
			_currentFrame = -1;
		}
		_lastFrameTime = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		_precise = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Looping") == 0) {
		_looping = Value->GetBool();
		return S_OK;
	}

	else return CBScriptHolder::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CBSprite::ScToString() {
	return "[sprite]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::KillAllSounds() {
	for (int i = 0; i < _frames.GetSize(); i++) {
		if (_frames[i]->_sound) _frames[i]->_sound->Stop();
	}
	return S_OK;
}

} // end of namespace WinterMute
