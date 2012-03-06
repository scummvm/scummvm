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

#include "dcgf.h"
#include "BParser.h"
#include "BSubFrame.h"
#include "BActiveRect.h"
#include "BDynBuffer.h"
#include "BSurface.h"
#include "ScValue.h"
#include "ScStack.h"
#include "BGame.h"
#include "PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CBSubFrame, false)

//////////////////////////////////////////////////////////////////////////
CBSubFrame::CBSubFrame(CBGame *inGame): CBScriptable(inGame, true) {
	m_Surface = NULL;
	m_HotspotX = m_HotspotY = 0;
	m_Alpha = 0xFFFFFFFF;
	m_Transparent = 0xFFFF00FF;

	CBPlatform::SetRectEmpty(&m_Rect);

	m_EditorSelected = false;

	m_SurfaceFilename = NULL;
	m_CKDefault = true;
	m_CKRed = m_CKBlue = m_CKGreen = 0;
	m_LifeTime = -1;
	m_KeepLoaded = false;

	m_2DOnly = m_3DOnly = false;
	m_Decoration = false;

	m_MirrorX = m_MirrorY = false;
}


//////////////////////////////////////////////////////////////////////////
CBSubFrame::~CBSubFrame() {
	if (m_Surface) Game->m_SurfaceStorage->RemoveSurface(m_Surface);
	delete[] m_SurfaceFilename;
	m_SurfaceFilename = NULL;
}


TOKEN_DEF_START
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(RECT)
TOKEN_DEF(HOTSPOT)
TOKEN_DEF(2D_ONLY)
TOKEN_DEF(3D_ONLY)
TOKEN_DEF(DECORATION)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(MIRROR_X)
TOKEN_DEF(MIRROR_Y)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::LoadBuffer(byte  *Buffer, int LifeTime, bool KeepLoaded) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(RECT)
	TOKEN_TABLE(HOTSPOT)
	TOKEN_TABLE(2D_ONLY)
	TOKEN_TABLE(3D_ONLY)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(MIRROR_X)
	TOKEN_TABLE(MIRROR_Y)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	CBParser parser(Game);
	RECT rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	bool custom_trans = false;
	CBPlatform::SetRectEmpty(&rect);
	char *surface_file = NULL;

	delete m_Surface;
	m_Surface = NULL;

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, &params)) > 0) {
		switch (cmd) {
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
			parser.ScanStr(params, "%d,%d", &m_HotspotX, &m_HotspotY);
			break;

		case TOKEN_2D_ONLY:
			parser.ScanStr(params, "%b", &m_2DOnly);
			break;

		case TOKEN_3D_ONLY:
			parser.ScanStr(params, "%b", &m_3DOnly);
			break;

		case TOKEN_MIRROR_X:
			parser.ScanStr(params, "%b", &m_MirrorX);
			break;

		case TOKEN_MIRROR_Y:
			parser.ScanStr(params, "%b", &m_MirrorY);
			break;

		case TOKEN_DECORATION:
			parser.ScanStr(params, "%b", &m_Decoration);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.ScanStr(params, "%b", &m_EditorSelected);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty((byte  *)params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in SUBFRAME definition");
		return E_FAIL;
	}

	if (surface_file != NULL) {
		if (custom_trans) SetSurface(surface_file, false, r, g, b, LifeTime, KeepLoaded);
		else SetSurface(surface_file, true, 0, 0, 0, LifeTime, KeepLoaded);
	}

	m_Alpha = DRGBA(ar, ag, ab, alpha);
	if (custom_trans) m_Transparent = DRGBA(r, g, b, 0xFF);

	/*
	if(m_Surface == NULL)
	{
	    Game->LOG(0, "Error parsing sub-frame. Image not set.");
	    return E_FAIL;
	}
	*/
	if (CBPlatform::IsRectEmpty(&rect)) SetDefaultRect();
	else m_Rect = rect;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::Draw(int X, int Y, CBObject *Register, float ZoomX, float ZoomY, bool Precise, uint32 Alpha, float Rotate, TSpriteBlendMode BlendMode) {
	if (!m_Surface) return S_OK;

	if (Register != NULL && !m_Decoration) {
		if (ZoomX == 100 && ZoomY == 100) {
			Game->m_Renderer->m_RectList.Add(new CBActiveRect(Game, Register, this, X - m_HotspotX + m_Rect.left, Y - m_HotspotY + m_Rect.top, m_Rect.right - m_Rect.left, m_Rect.bottom - m_Rect.top, ZoomX, ZoomY, Precise));
		} else {
			Game->m_Renderer->m_RectList.Add(new CBActiveRect(Game, Register, this, (int)(X - (m_HotspotX + m_Rect.left) * (ZoomX / 100)), (int)(Y - (m_HotspotY + m_Rect.top) * (ZoomY / 100)), (int)((m_Rect.right - m_Rect.left) * (ZoomX / 100)), (int)((m_Rect.bottom - m_Rect.top) * (ZoomY / 100)), ZoomX, ZoomY, Precise));
		}
	}
	if (Game->m_SuspendedRendering) return S_OK;

	HRESULT res;

	//if(Alpha==0xFFFFFFFF) Alpha = m_Alpha; // TODO: better (combine owner's and self alpha)
	if (m_Alpha != 0xFFFFFFFF) Alpha = m_Alpha;

	if (Rotate != 0.0f) {
		res = m_Surface->DisplayTransform((int)(X - m_HotspotX * (ZoomX / 100)), (int)(Y - m_HotspotY * (ZoomY / 100)), m_HotspotX, m_HotspotY, m_Rect, ZoomX, ZoomY, Alpha, Rotate, BlendMode, m_MirrorX, m_MirrorY);
	} else {
		if (ZoomX == 100 && ZoomY == 100) res = m_Surface->DisplayTrans(X - m_HotspotX, Y - m_HotspotY, m_Rect, Alpha, BlendMode, m_MirrorX, m_MirrorY);
		else res = m_Surface->DisplayTransZoom((int)(X - m_HotspotX * (ZoomX / 100)), (int)(Y - m_HotspotY * (ZoomY / 100)), m_Rect, ZoomX, ZoomY, Alpha, BlendMode, m_MirrorX, m_MirrorY);
	}

	return res;
}


//////////////////////////////////////////////////////////////////////////
bool CBSubFrame::GetBoundingRect(LPRECT Rect, int X, int Y, float ScaleX, float ScaleY) {
	if (!Rect) return false;

	float RatioX = ScaleX / 100.0f;
	float RatioY = ScaleY / 100.0f;

	CBPlatform::SetRect(Rect,
	                    X - m_HotspotX * RatioX,
	                    Y - m_HotspotY * RatioY,
	                    X - m_HotspotX * RatioX + (m_Rect.right - m_Rect.left)*RatioX,
	                    Y - m_HotspotY * RatioY + (m_Rect.bottom - m_Rect.top)*RatioY);
	return true;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::SaveAsText(CBDynBuffer *Buffer, int Indent, bool Complete) {
	if (Complete)
		Buffer->PutTextIndent(Indent, "SUBFRAME {\n");

	if (m_Surface && m_Surface->m_Filename != NULL)
		Buffer->PutTextIndent(Indent + 2, "IMAGE = \"%s\"\n", m_Surface->m_Filename);

	if (m_Transparent != 0xFFFF00FF)
		Buffer->PutTextIndent(Indent + 2, "TRANSPARENT { %d,%d,%d }\n", D3DCOLGetR(m_Transparent), D3DCOLGetG(m_Transparent), D3DCOLGetB(m_Transparent));

	RECT rect;
	CBPlatform::SetRectEmpty(&rect);
	if (m_Surface) CBPlatform::SetRect(&rect, 0, 0, m_Surface->GetWidth(), m_Surface->GetHeight());
	if (!CBPlatform::EqualRect(&rect, &m_Rect))
		Buffer->PutTextIndent(Indent + 2, "RECT { %d,%d,%d,%d }\n", m_Rect.left, m_Rect.top, m_Rect.right, m_Rect.bottom);

	if (m_HotspotX != 0 || m_HotspotY != 0)
		Buffer->PutTextIndent(Indent + 2, "HOTSPOT {%d, %d}\n", m_HotspotX, m_HotspotY);

	if (m_Alpha != 0xFFFFFFFF) {
		Buffer->PutTextIndent(Indent + 2, "ALPHA_COLOR { %d,%d,%d }\n", D3DCOLGetR(m_Alpha), D3DCOLGetG(m_Alpha), D3DCOLGetB(m_Alpha));
		Buffer->PutTextIndent(Indent + 2, "ALPHA = %d\n", D3DCOLGetA(m_Alpha));
	}

	if (m_MirrorX)
		Buffer->PutTextIndent(Indent + 2, "MIRROR_X=%s\n", m_MirrorX ? "TRUE" : "FALSE");

	if (m_MirrorY)
		Buffer->PutTextIndent(Indent + 2, "MIRROR_Y=%s\n", m_MirrorY ? "TRUE" : "FALSE");

	if (m_2DOnly)
		Buffer->PutTextIndent(Indent + 2, "2D_ONLY=%s\n", m_2DOnly ? "TRUE" : "FALSE");

	if (m_3DOnly)
		Buffer->PutTextIndent(Indent + 2, "3D_ONLY=%s\n", m_3DOnly ? "TRUE" : "FALSE");

	if (m_Decoration)
		Buffer->PutTextIndent(Indent + 2, "DECORATION=%s\n", m_Decoration ? "TRUE" : "FALSE");

	if (m_EditorSelected)
		Buffer->PutTextIndent(Indent + 2, "EDITOR_SELECTED=%s\n", m_EditorSelected ? "TRUE" : "FALSE");

	CBBase::SaveAsText(Buffer, Indent + 2);


	if (Complete)
		Buffer->PutTextIndent(Indent, "}\n\n");

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CBSubFrame::SetDefaultRect() {
	if (m_Surface) {
		CBPlatform::SetRect(&m_Rect, 0, 0, m_Surface->GetWidth(), m_Surface->GetHeight());
	} else CBPlatform::SetRectEmpty(&m_Rect);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::Persist(CBPersistMgr *PersistMgr) {

	CBScriptable::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_2DOnly));
	PersistMgr->Transfer(TMEMBER(m_3DOnly));
	PersistMgr->Transfer(TMEMBER(m_Alpha));
	PersistMgr->Transfer(TMEMBER(m_Decoration));
	PersistMgr->Transfer(TMEMBER(m_EditorSelected));
	PersistMgr->Transfer(TMEMBER(m_HotspotX));
	PersistMgr->Transfer(TMEMBER(m_HotspotY));
	PersistMgr->Transfer(TMEMBER(m_Rect));

	PersistMgr->Transfer(TMEMBER(m_SurfaceFilename));
	PersistMgr->Transfer(TMEMBER(m_CKDefault));
	PersistMgr->Transfer(TMEMBER(m_CKRed));
	PersistMgr->Transfer(TMEMBER(m_CKGreen));
	PersistMgr->Transfer(TMEMBER(m_CKBlue));
	PersistMgr->Transfer(TMEMBER(m_LifeTime));

	PersistMgr->Transfer(TMEMBER(m_KeepLoaded));
	PersistMgr->Transfer(TMEMBER(m_MirrorX));
	PersistMgr->Transfer(TMEMBER(m_MirrorY));
	PersistMgr->Transfer(TMEMBER(m_Transparent));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {

	//////////////////////////////////////////////////////////////////////////
	// GetImage
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetImage") == 0) {
		Stack->CorrectParams(0);

		if (!m_SurfaceFilename) Stack->PushNULL();
		else Stack->PushString(m_SurfaceFilename);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetImage") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (Val->IsNULL()) {
			if (m_Surface) Game->m_SurfaceStorage->RemoveSurface(m_Surface);
			delete[] m_SurfaceFilename;
			m_SurfaceFilename = NULL;
			Stack->PushBool(true);
		} else {
			char *Filename = Val->GetString();
			if (SUCCEEDED(SetSurface(Filename))) {
				SetDefaultRect();
				Stack->PushBool(true);
			} else Stack->PushBool(false);
		}

		return S_OK;
	}

	else return CBScriptable::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CBSubFrame::ScGetProperty(char *Name) {
	if (!m_ScValue) m_ScValue = new CScValue(Game);
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("subframe");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "AlphaColor") == 0) {

		m_ScValue->SetInt((int)m_Alpha);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TransparentColor (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TransparentColor") == 0) {
		m_ScValue->SetInt((int)m_Transparent);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is2DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Is2DOnly") == 0) {
		m_ScValue->SetBool(m_2DOnly);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is3DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Is3DOnly") == 0) {
		m_ScValue->SetBool(m_3DOnly);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MirrorX") == 0) {
		m_ScValue->SetBool(m_MirrorX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MirrorY") == 0) {
		m_ScValue->SetBool(m_MirrorY);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Decoration") == 0) {
		m_ScValue->SetBool(m_Decoration);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HotspotX") == 0) {
		m_ScValue->SetInt(m_HotspotX);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HotspotY") == 0) {
		m_ScValue->SetInt(m_HotspotY);
		return m_ScValue;
	}

	else return CBScriptable::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "AlphaColor") == 0) {
		m_Alpha = (uint32)Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is2DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Is2DOnly") == 0) {
		m_2DOnly = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Is3DOnly
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Is3DOnly") == 0) {
		m_3DOnly = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MirrorX") == 0) {
		m_MirrorX = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MirrorY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "MirrorY") == 0) {
		m_MirrorY = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Decoration
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Decoration") == 0) {
		m_Decoration = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HotspotX") == 0) {
		m_HotspotX = Value->GetInt();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HotspotY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "HotspotY") == 0) {
		m_HotspotY = Value->GetInt();
		return S_OK;
	}

	else return CBScriptable::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CBSubFrame::ScToString() {
	return "[subframe]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::SetSurface(char *Filename, bool default_ck, byte ck_red, byte ck_green, byte ck_blue, int LifeTime, bool KeepLoaded) {
	if (m_Surface) {
		Game->m_SurfaceStorage->RemoveSurface(m_Surface);
		m_Surface = NULL;
	}

	delete[] m_SurfaceFilename;
	m_SurfaceFilename = NULL;

	m_Surface = Game->m_SurfaceStorage->AddSurface(Filename, default_ck, ck_red, ck_green, ck_blue, LifeTime, KeepLoaded);
	if (m_Surface) {
		m_SurfaceFilename = new char[strlen(Filename) + 1];
		strcpy(m_SurfaceFilename, Filename);

		m_CKDefault = default_ck;
		m_CKRed = ck_red;
		m_CKGreen = ck_green;
		m_CKBlue = ck_blue;
		m_LifeTime = LifeTime;
		m_KeepLoaded = KeepLoaded;

		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSubFrame::SetSurfaceSimple() {
	if (!m_SurfaceFilename) {
		m_Surface = NULL;
		return S_OK;
	}
	m_Surface = Game->m_SurfaceStorage->AddSurface(m_SurfaceFilename, m_CKDefault, m_CKRed, m_CKGreen, m_CKBlue, m_LifeTime, m_KeepLoaded);
	if (m_Surface) return S_OK;
	else return E_FAIL;
}

} // end of namespace WinterMute
