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
	m_EditorAllFrames = false;
	m_Owner = Owner;
	SetDefaults();
}


//////////////////////////////////////////////////////////////////////
CBSprite::~CBSprite() {
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::SetDefaults() {
	m_CurrentFrame = -1;
	m_Looping = false;
	m_LastFrameTime = 0;
	m_Filename = NULL;
	m_Finished = false;
	m_Changed = false;
	m_Paused = false;
	m_Continuous = false;
	m_MoveX = m_MoveY = 0;

	m_EditorMuted = false;
	m_EditorBgFile = NULL;
	m_EditorBgOffsetX = m_EditorBgOffsetY = 0;
	m_EditorBgAlpha = 0xFF;
	m_Streamed = false;
	m_StreamedKeepLoaded = false;

	SetName("");

	m_Precise = true;
}


//////////////////////////////////////////////////////////////////////////
void CBSprite::Cleanup() {
	CBScriptHolder::Cleanup();

	for (int i = 0; i < m_Frames.GetSize(); i++)
		delete m_Frames[i];
	m_Frames.RemoveAll();

	SAFE_DELETE_ARRAY(m_EditorBgFile);

	SetDefaults();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha) {
	GetCurrentFrame(ZoomX, ZoomY);
	if (m_CurrentFrame < 0 || m_CurrentFrame >= m_Frames.GetSize()) return S_OK;

	// move owner if allowed to
	if (m_Changed && m_Owner && m_Owner->m_Movable) {
		m_Owner->m_PosX += m_MoveX;
		m_Owner->m_PosY += m_MoveY;
		m_Owner->AfterMove();

		X = m_Owner->m_PosX;
		Y = m_Owner->m_PosY;
	}

	// draw frame
	return Display(X, Y, Register, ZoomX, ZoomY, Alpha);
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::LoadFile(char *Filename, int LifeTime, TSpriteCacheType CacheType) {
	CBFile *File = Game->m_FileManager->OpenFile(Filename);
	if (!File) {
		Game->LOG(0, "CBSprite::LoadFile failed for file '%s'", Filename);
		if (Game->m_DEBUG_DebugMode) return LoadFile("invalid_debug.bmp", LifeTime, CacheType);
		else return LoadFile("invalid.bmp", LifeTime, CacheType);
	} else {
		Game->m_FileManager->CloseFile(File);
		File = NULL;
	}

	HRESULT ret;

	AnsiString ext = PathUtil::GetExtension(Filename);
	if (StringUtil::StartsWith(Filename, "savegame:", true) || StringUtil::CompareNoCase(ext, ".bmp") || StringUtil::CompareNoCase(ext, ".tga") || StringUtil::CompareNoCase(ext, ".png") || StringUtil::CompareNoCase(ext, ".jpg")) {
		CBFrame *frame = new CBFrame(Game);
		CBSubFrame *subframe = new CBSubFrame(Game);
		subframe->SetSurface(Filename, true, 0, 0, 0, LifeTime, true);
		if (subframe->m_Surface == NULL) {
			Game->LOG(0, "Error loading simple sprite '%s'", Filename);
			ret = E_FAIL;
			delete frame;
			delete subframe;
		} else {
			CBPlatform::SetRect(&subframe->m_Rect, 0, 0, subframe->m_Surface->GetWidth(), subframe->m_Surface->GetHeight());
			frame->m_Subframes.Add(subframe);
			m_Frames.Add(frame);
			m_CurrentFrame = 0;
			ret = S_OK;
		}
	} else {
		byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
		if (Buffer) {
			if (FAILED(ret = LoadBuffer(Buffer, true, LifeTime, CacheType))) Game->LOG(0, "Error parsing SPRITE file '%s'", Filename);
			delete [] Buffer;
		}
	}

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);


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
			parser.ScanStr((char *)params, "%b", &m_Continuous);
			break;

		case TOKEN_EDITOR_MUTED:
			parser.ScanStr((char *)params, "%b", &m_EditorMuted);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_LOOPING:
			parser.ScanStr((char *)params, "%b", &m_Looping);
			break;

		case TOKEN_PRECISE:
			parser.ScanStr((char *)params, "%b", &m_Precise);
			break;

		case TOKEN_STREAMED:
			parser.ScanStr((char *)params, "%b", &m_Streamed);
			if (m_Streamed && LifeTime == -1) {
				LifeTime = 500;
				CacheType = CACHE_ALL;
			}
			break;

		case TOKEN_STREAMED_KEEP_LOADED:
			parser.ScanStr((char *)params, "%b", &m_StreamedKeepLoaded);
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_EDITOR_BG_FILE:
			if (Game->m_EditorMode) {
				SAFE_DELETE_ARRAY(m_EditorBgFile);
				m_EditorBgFile = new char[strlen((char *)params) + 1];
				if (m_EditorBgFile) strcpy(m_EditorBgFile, (char *)params);
			}
			break;

		case TOKEN_EDITOR_BG_OFFSET_X:
			parser.ScanStr((char *)params, "%d", &m_EditorBgOffsetX);
			break;

		case TOKEN_EDITOR_BG_OFFSET_Y:
			parser.ScanStr((char *)params, "%d", &m_EditorBgOffsetY);
			break;

		case TOKEN_EDITOR_BG_ALPHA:
			parser.ScanStr((char *)params, "%d", &m_EditorBgAlpha);
			m_EditorBgAlpha = std::min(m_EditorBgAlpha, 255);
			m_EditorBgAlpha = std::max(m_EditorBgAlpha, 0);
			break;

		case TOKEN_FRAME: {
			int FrameLifeTime = LifeTime;
			if (CacheType == CACHE_HALF && frame_count % 2 != 1) FrameLifeTime = -1;

			frame = new CBFrame(Game);

			if (FAILED(frame->LoadBuffer(params, FrameLifeTime, m_StreamedKeepLoaded))) {
				delete frame;
				Game->LOG(0, "Error parsing frame %d", frame_count);
				return E_FAIL;
			}

			m_Frames.Add(frame);
			frame_count++;
			if (m_CurrentFrame == -1) m_CurrentFrame = 0;
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
	m_CanBreak = !m_Continuous;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
void CBSprite::Reset() {
	if (m_Frames.GetSize() > 0) m_CurrentFrame = 0;
	else m_CurrentFrame = -1;

	KillAllSounds();

	m_LastFrameTime = 0;
	m_Finished = false;
	m_MoveX = m_MoveY = 0;
}


//////////////////////////////////////////////////////////////////////
bool CBSprite::GetCurrentFrame(float ZoomX, float ZoomY) {
	//if(m_Owner && m_Owner->m_Freezable && Game->m_State == GAME_FROZEN) return true;

	if (m_CurrentFrame == -1) return false;

	uint32 timer;
	if (m_Owner && m_Owner->m_Freezable) timer = Game->m_Timer;
	else timer = Game->m_LiveTimer;

	int LastFrame = m_CurrentFrame;

	// get current frame
	if (!m_Paused && !m_Finished && timer >= m_LastFrameTime + m_Frames[m_CurrentFrame]->m_Delay && m_LastFrameTime != 0) {
		if (m_CurrentFrame < m_Frames.GetSize() - 1) {
			m_CurrentFrame++;
			if (m_Continuous) m_CanBreak = (m_CurrentFrame == m_Frames.GetSize() - 1);
		} else {
			if (m_Looping) {
				m_CurrentFrame = 0;
				m_CanBreak = true;
			} else {
				m_Finished = true;
				m_CanBreak = true;
			}
		}

		m_LastFrameTime = timer;
	}

	m_Changed = (LastFrame != m_CurrentFrame || (m_Looping && m_Frames.GetSize() == 1));

	if (m_LastFrameTime == 0) {
		m_LastFrameTime = timer;
		m_Changed = true;
		if (m_Continuous) m_CanBreak = (m_CurrentFrame == m_Frames.GetSize() - 1);
	}

	if (m_Changed) {
		m_MoveX = m_Frames[m_CurrentFrame]->m_MoveX;
		m_MoveY = m_Frames[m_CurrentFrame]->m_MoveY;

		if (ZoomX != 100 || ZoomY != 100) {
			m_MoveX = (int)((float)m_MoveX * (float)(ZoomX / 100.0f));
			m_MoveY = (int)((float)m_MoveY * (float)(ZoomY / 100.0f));
		}
	}

	return m_Changed;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Display(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode) {
	if (m_CurrentFrame < 0 || m_CurrentFrame >= m_Frames.GetSize()) return S_OK;

	// on change...
	if (m_Changed) {
		if (m_Frames[m_CurrentFrame]->m_KillSound) {
			KillAllSounds();
		}
		ApplyEvent("FrameChanged");
		m_Frames[m_CurrentFrame]->OneTimeDisplay(m_Owner, Game->m_EditorMode && m_EditorMuted);
	}

	// draw frame
	return m_Frames[m_CurrentFrame]->Draw(X - Game->m_OffsetX, Y - Game->m_OffsetY, Register, ZoomX, ZoomY, m_Precise, Alpha, m_EditorAllFrames, Rotate, BlendMode);
}


//////////////////////////////////////////////////////////////////////////
CBSurface *CBSprite::GetSurface() {
	// only used for animated textures for 3D models
	if (m_CurrentFrame < 0 || m_CurrentFrame >= m_Frames.GetSize()) return NULL;
	CBFrame *Frame = m_Frames[m_CurrentFrame];
	if (Frame && Frame->m_Subframes.GetSize() > 0) {
		CBSubFrame *Subframe = Frame->m_Subframes[0];
		if (Subframe) return Subframe->m_Surface;
		else return NULL;
	} else return NULL;
}

//////////////////////////////////////////////////////////////////////////
bool CBSprite::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;

	CBPlatform::SetRectEmpty(Rect);
	for (int i = 0; i < m_Frames.GetSize(); i++) {
		RECT frame;
		RECT temp;
		CBPlatform::CopyRect(&temp, Rect);
		m_Frames[i]->GetBoundingRect(&frame, X, Y, ScaleX, ScaleY);
		CBPlatform::UnionRect(Rect, &temp, &frame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "SPRITE {\n");
	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "LOOPING=%s\n", m_Looping ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CONTINUOUS=%s\n", m_Continuous ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PRECISE=%s\n", m_Precise ? "TRUE" : "FALSE");
	if (m_Streamed) {
		Buffer->PutTextIndent(Indent + 2, "STREAMED=%s\n", m_Streamed ? "TRUE" : "FALSE");

		if (m_StreamedKeepLoaded)
			Buffer->PutTextIndent(Indent + 2, "STREAMED_KEEP_LOADED=%s\n", m_StreamedKeepLoaded ? "TRUE" : "FALSE");
	}

	if (m_EditorMuted)
		Buffer->PutTextIndent(Indent + 2, "EDITOR_MUTED=%s\n", m_EditorMuted ? "TRUE" : "FALSE");

	if (m_EditorBgFile) {
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_FILE=\"%s\"\n", m_EditorBgFile);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_OFFSET_X=%d\n", m_EditorBgOffsetX);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_OFFSET_Y=%d\n", m_EditorBgOffsetY);
		Buffer->PutTextIndent(Indent + 2, "EDITOR_BG_ALPHA=%d\n", m_EditorBgAlpha);
	}

	CBScriptHolder::SaveAsText(Buffer, Indent + 2);

	int i;

	// scripts
	for (i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}


	for (i = 0; i < m_Frames.GetSize(); i++) {
		m_Frames[i]->SaveAsText(Buffer, Indent + 2);
	}

	Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::Persist(CBPersistMgr *PersistMgr) {
	CBScriptHolder::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_CanBreak));
	PersistMgr->Transfer(TMEMBER(m_Changed));
	PersistMgr->Transfer(TMEMBER(m_Paused));
	PersistMgr->Transfer(TMEMBER(m_Continuous));
	PersistMgr->Transfer(TMEMBER(m_CurrentFrame));
	PersistMgr->Transfer(TMEMBER(m_EditorAllFrames));
	PersistMgr->Transfer(TMEMBER(m_EditorBgAlpha));
	PersistMgr->Transfer(TMEMBER(m_EditorBgFile));
	PersistMgr->Transfer(TMEMBER(m_EditorBgOffsetX));
	PersistMgr->Transfer(TMEMBER(m_EditorBgOffsetY));
	PersistMgr->Transfer(TMEMBER(m_EditorMuted));
	PersistMgr->Transfer(TMEMBER(m_Finished));

	m_Frames.Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_LastFrameTime));
	PersistMgr->Transfer(TMEMBER(m_Looping));
	PersistMgr->Transfer(TMEMBER(m_MoveX));
	PersistMgr->Transfer(TMEMBER(m_MoveY));
	PersistMgr->Transfer(TMEMBER(m_Owner));
	PersistMgr->Transfer(TMEMBER(m_Precise));
	PersistMgr->Transfer(TMEMBER(m_Streamed));
	PersistMgr->Transfer(TMEMBER(m_StreamedKeepLoaded));


	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetFrame") == 0) {
		Stack->CorrectParams(1);
		int Index = Stack->Pop()->GetInt(-1);
		if (Index < 0 || Index >= m_Frames.GetSize()) {
			Script->RuntimeError("Sprite.GetFrame: Frame index %d is out of range.", Index);
			Stack->PushNULL();
		} else Stack->PushNative(m_Frames[Index], true);
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
			if (Index < 0 || Index >= m_Frames.GetSize()) {
				Script->RuntimeError("Sprite.DeleteFrame: Frame index %d is out of range.", Index);
			}
		} else {
			CBFrame *Frame = (CBFrame *)Val->GetNative();
			for (int i = 0; i < m_Frames.GetSize(); i++) {
				if (m_Frames[i] == Frame) {
					if (i == m_CurrentFrame) m_LastFrameTime = 0;
					delete m_Frames[i];
					m_Frames.RemoveAt(i);
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
		char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->SetSurface(Filename))) {
				Sub->SetDefaultRect();
				Frame->m_Subframes.Add(Sub);
			} else delete Sub;
		}
		m_Frames.Add(Frame);

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
		char *Filename = NULL;
		if (!Val->IsNULL()) Filename = Val->GetString();

		CBFrame *Frame = new CBFrame(Game);
		if (Filename != NULL) {
			CBSubFrame *Sub = new CBSubFrame(Game);
			if (SUCCEEDED(Sub->SetSurface(Filename))) Frame->m_Subframes.Add(Sub);
			else delete Sub;
		}

		if (Index >= m_Frames.GetSize()) m_Frames.Add(Frame);
		else m_Frames.InsertAt(Index, Frame);

		Stack->PushNative(Frame, true);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pause") == 0) {
		Stack->CorrectParams(0);
		m_Paused = true;
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Play") == 0) {
		Stack->CorrectParams(0);
		m_Paused = false;
		Stack->PushNULL();
		return S_OK;
	}

	else return CBScriptHolder::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSprite::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("sprite");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFrames (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumFrames") == 0) {
		m_ScValue->SetInt(m_Frames.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CurrentFrame") == 0) {
		m_ScValue->SetInt(m_CurrentFrame);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		m_ScValue->SetBool(m_Precise);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Looping") == 0) {
		m_ScValue->SetBool(m_Looping);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Owner (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Owner") == 0) {
		if (m_Owner == NULL) m_ScValue->SetNULL();
		else m_ScValue->SetNative(m_Owner, true);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finished (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Finished") == 0) {
		m_ScValue->SetBool(m_Finished);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Paused (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Paused") == 0) {
		m_ScValue->SetBool(m_Paused);
		return m_ScValue;
	}

	else return CBScriptHolder::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSprite::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "CurrentFrame") == 0) {
		m_CurrentFrame = Value->GetInt(0);
		if (m_CurrentFrame >= m_Frames.GetSize() || m_CurrentFrame < 0) {
			m_CurrentFrame = -1;
		}
		m_LastFrameTime = 0;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		m_Precise = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Looping") == 0) {
		m_Looping = Value->GetBool();
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
	for (int i = 0; i < m_Frames.GetSize(); i++) {
		if (m_Frames[i]->m_Sound) m_Frames[i]->m_Sound->Stop();
	}
	return S_OK;
}

} // end of namespace WinterMute
