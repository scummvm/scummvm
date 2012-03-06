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
#include "AdGame.h"
#include "AdResponseBox.h"
#include "BParser.h"
#include "BSurfaceStorage.h"
#include "UIButton.h"
#include "UIWindow.h"
#include "BDynBuffer.h"
#include "BFontStorage.h"
#include "BFont.h"
#include "AdResponse.h"
#include "ScScript.h"
#include "ScStack.h"
#include "BSprite.h"
#include "BFileManager.h"
#include "utils.h"
#include "PlatformSDL.h"
#include "common/str.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdResponseBox, false)

//////////////////////////////////////////////////////////////////////////
CAdResponseBox::CAdResponseBox(CBGame *inGame): CBObject(inGame) {
	m_Font = m_FontHover = NULL;

	m_Window = NULL;
	m_ShieldWindow = new CUIWindow(Game);

	m_Horizontal = false;
	CBPlatform::SetRectEmpty(&m_ResponseArea);
	m_ScrollOffset = 0;
	m_Spacing = 0;

	m_WaitingScript = NULL;
	m_LastResponseText = NULL;
	m_LastResponseTextOrig = NULL;

	m_VerticalAlign = VAL_BOTTOM;
	m_Align = TAL_LEFT;
}


//////////////////////////////////////////////////////////////////////////
CAdResponseBox::~CAdResponseBox() {

	SAFE_DELETE(m_Window);
	SAFE_DELETE(m_ShieldWindow);
	SAFE_DELETE_ARRAY(m_LastResponseText);
	SAFE_DELETE_ARRAY(m_LastResponseTextOrig);

	if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
	if (m_FontHover) Game->m_FontStorage->RemoveFont(m_FontHover);

	ClearResponses();
	ClearButtons();

	m_WaitingScript = NULL;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::ClearResponses() {
	for (int i = 0; i < m_Responses.GetSize(); i++) {
		delete m_Responses[i];
	}
	m_Responses.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::ClearButtons() {
	for (int i = 0; i < m_RespButtons.GetSize(); i++) {
		delete m_RespButtons[i];
	}
	m_RespButtons.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::InvalidateButtons() {
	for (int i = 0; i < m_RespButtons.GetSize(); i++) {
		m_RespButtons[i]->m_Image = NULL;
		m_RespButtons[i]->m_Cursor = NULL;
		m_RespButtons[i]->m_Font = NULL;
		m_RespButtons[i]->m_FontHover = NULL;
		m_RespButtons[i]->m_FontPress = NULL;
		m_RespButtons[i]->SetText("");
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::CreateButtons() {
	ClearButtons();

	m_ScrollOffset = 0;
	for (int i = 0; i < m_Responses.GetSize(); i++) {
		CUIButton *btn = new CUIButton(Game);
		if (btn) {
			btn->m_Parent = m_Window;
			btn->m_SharedFonts = btn->m_SharedImages = true;
			btn->m_SharedCursors = true;
			// iconic
			if (m_Responses[i]->m_Icon) {
				btn->m_Image = m_Responses[i]->m_Icon;
				if (m_Responses[i]->m_IconHover)   btn->m_ImageHover = m_Responses[i]->m_IconHover;
				if (m_Responses[i]->m_IconPressed) btn->m_ImagePress = m_Responses[i]->m_IconPressed;

				btn->SetCaption(m_Responses[i]->m_Text);
				if (m_Cursor) btn->m_Cursor = m_Cursor;
				else if (Game->m_ActiveCursor) btn->m_Cursor = Game->m_ActiveCursor;
			}
			// textual
			else {
				btn->SetText(m_Responses[i]->m_Text);
				btn->m_Font = (m_Font == NULL) ? Game->m_SystemFont : m_Font;
				btn->m_FontHover = (m_FontHover == NULL) ? Game->m_SystemFont : m_FontHover;
				btn->m_FontPress = btn->m_FontHover;
				btn->m_Align = m_Align;

				if (Game->m_TouchInterface)
					btn->m_FontHover = btn->m_Font;


				if (m_Responses[i]->m_Font) btn->m_Font = m_Responses[i]->m_Font;

				btn->m_Width = m_ResponseArea.right - m_ResponseArea.left;
				if (btn->m_Width <= 0) btn->m_Width = Game->m_Renderer->m_Width;
			}
			btn->SetName("response");
			btn->CorrectSize();

			// make the responses touchable
			if (Game->m_TouchInterface)
				btn->m_Height = std::max(btn->m_Height, 50);

			//btn->SetListener(this, btn, m_Responses[i]->m_ID);
			btn->SetListener(this, btn, i);
			btn->m_Visible = false;
			m_RespButtons.Add(btn);

			if (m_ResponseArea.bottom - m_ResponseArea.top < btn->m_Height) {
				Game->LOG(0, "Warning: Response '%s' is too high to be displayed within response box. Correcting.", m_Responses[i]->m_Text);
				m_ResponseArea.bottom += (btn->m_Height - (m_ResponseArea.bottom - m_ResponseArea.top));
			}
		}
	}
	m_Ready = false;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdResponseBox::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing RESPONSE_BOX file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(RESPONSE_BOX)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(FONT_HOVER)
TOKEN_DEF(FONT)
TOKEN_DEF(AREA)
TOKEN_DEF(HORIZONTAL)
TOKEN_DEF(SPACING)
TOKEN_DEF(WINDOW)
TOKEN_DEF(CURSOR)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(VERTICAL_ALIGN)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(RESPONSE_BOX)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(FONT_HOVER)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(AREA)
	TOKEN_TABLE(HORIZONTAL)
	TOKEN_TABLE(SPACING)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(VERTICAL_ALIGN)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END


	byte *params;
	int cmd;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_RESPONSE_BOX) {
			Game->LOG(0, "'RESPONSE_BOX' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_WINDOW:
			SAFE_DELETE(m_Window);
			m_Window = new CUIWindow(Game);
			if (!m_Window || FAILED(m_Window->LoadBuffer(params, false))) {
				SAFE_DELETE(m_Window);
				cmd = PARSERR_GENERIC;
			} else if (m_ShieldWindow) m_ShieldWindow->m_Parent = m_Window;
			break;

		case TOKEN_FONT:
			if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
			m_Font = Game->m_FontStorage->AddFont((char *)params);
			if (!m_Font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_HOVER:
			if (m_FontHover) Game->m_FontStorage->RemoveFont(m_FontHover);
			m_FontHover = Game->m_FontStorage->AddFont((char *)params);
			if (!m_FontHover) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_AREA:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_ResponseArea.left, &m_ResponseArea.top, &m_ResponseArea.right, &m_ResponseArea.bottom);
			break;

		case TOKEN_HORIZONTAL:
			parser.ScanStr((char *)params, "%b", &m_Horizontal);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "center") == 0) m_Align = TAL_CENTER;
			else if (scumm_stricmp((char *)params, "right") == 0) m_Align = TAL_RIGHT;
			else m_Align = TAL_LEFT;
			break;

		case TOKEN_VERTICAL_ALIGN:
			if (scumm_stricmp((char *)params, "top") == 0) m_VerticalAlign = VAL_TOP;
			else if (scumm_stricmp((char *)params, "center") == 0) m_VerticalAlign = VAL_CENTER;
			else m_VerticalAlign = VAL_BOTTOM;
			break;

		case TOKEN_SPACING:
			parser.ScanStr((char *)params, "%d", &m_Spacing);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_CURSOR:
			SAFE_DELETE(m_Cursor);
			m_Cursor = new CBSprite(Game);
			if (!m_Cursor || FAILED(m_Cursor->LoadFile((char *)params))) {
				SAFE_DELETE(m_Cursor);
				cmd = PARSERR_GENERIC;
			}
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in RESPONSE_BOX definition");
		return E_FAIL;
	}

	if (m_Window) {
		for (int i = 0; i < m_Window->m_Widgets.GetSize(); i++) {
			if (!m_Window->m_Widgets[i]->m_ListenerObject)
				m_Window->m_Widgets[i]->SetListener(this, m_Window->m_Widgets[i], 0);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "RESPONSE_BOX\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "AREA { %d, %d, %d, %d }\n", m_ResponseArea.left, m_ResponseArea.top, m_ResponseArea.right, m_ResponseArea.bottom);

	if (m_Font && m_Font->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", m_Font->m_Filename);
	if (m_FontHover && m_FontHover->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_HOVER=\"%s\"\n", m_FontHover->m_Filename);

	if (m_Cursor && m_Cursor->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", m_Cursor->m_Filename);

	Buffer->PutTextIndent(Indent + 2, "HORIZONTAL=%s\n", m_Horizontal ? "TRUE" : "FALSE");

	switch (m_Align) {
	case TAL_LEFT:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "TEXT_ALIGN=\"%s\"\n", "center");
		break;
	}

	switch (m_VerticalAlign) {
	case VAL_TOP:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "top");
		break;
	case VAL_BOTTOM:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "bottom");
		break;
	case VAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "VERTICAL_ALIGN=\"%s\"\n", "center");
		break;
	}

	Buffer->PutTextIndent(Indent + 2, "SPACING=%d\n", m_Spacing);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// window
	if (m_Window) m_Window->SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Display() {
	RECT rect = m_ResponseArea;
	if (m_Window) {
		CBPlatform::OffsetRect(&rect, m_Window->m_PosX, m_Window->m_PosY);
		//m_Window->Display();
	}

	int xxx, yyy, i;

	xxx = rect.left;
	yyy = rect.top;

	// shift down if needed
	if (!m_Horizontal) {
		int total_height = 0;
		for (i = 0; i < m_RespButtons.GetSize(); i++) total_height += (m_RespButtons[i]->m_Height + m_Spacing);
		total_height -= m_Spacing;

		switch (m_VerticalAlign) {
		case VAL_BOTTOM:
			if (yyy + total_height < rect.bottom)
				yyy = rect.bottom - total_height;
			break;

		case VAL_CENTER:
			if (yyy + total_height < rect.bottom)
				yyy += ((rect.bottom - rect.top) - total_height) / 2;
			break;

		case VAL_TOP:
			// do nothing
			break;
		}
	}

	// prepare response buttons
	bool scroll_needed = false;
	for (i = m_ScrollOffset; i < m_RespButtons.GetSize(); i++) {
		if ((m_Horizontal && xxx + m_RespButtons[i]->m_Width > rect.right)
		        || (!m_Horizontal && yyy + m_RespButtons[i]->m_Height > rect.bottom)) {

			scroll_needed = true;
			m_RespButtons[i]->m_Visible = false;
			break;
		}

		m_RespButtons[i]->m_Visible = true;
		m_RespButtons[i]->m_PosX = xxx;
		m_RespButtons[i]->m_PosY = yyy;

		if (m_Horizontal) {
			xxx += (m_RespButtons[i]->m_Width + m_Spacing);
		} else {
			yyy += (m_RespButtons[i]->m_Height + m_Spacing);
		}
	}

	// show appropriate scroll buttons
	if (m_Window) {
		m_Window->ShowWidget("prev", m_ScrollOffset > 0);
		m_Window->ShowWidget("next", scroll_needed);
	}

	// go exclusive
	if (m_ShieldWindow) {
		m_ShieldWindow->m_PosX = m_ShieldWindow->m_PosY = 0;
		m_ShieldWindow->m_Width = Game->m_Renderer->m_Width;
		m_ShieldWindow->m_Height = Game->m_Renderer->m_Height;

		m_ShieldWindow->Display();
	}

	// display window
	if (m_Window) m_Window->Display();


	// display response buttons
	for (i = m_ScrollOffset; i < m_RespButtons.GetSize(); i++) {
		m_RespButtons[i]->Display();
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->m_Type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->m_Name, "prev") == 0) {
			m_ScrollOffset--;
		} else if (scumm_stricmp(obj->m_Name, "next") == 0) {
			m_ScrollOffset++;
		} else if (scumm_stricmp(obj->m_Name, "response") == 0) {
			if (m_WaitingScript) m_WaitingScript->m_Stack->PushInt(m_Responses[param2]->m_ID);
			HandleResponse(m_Responses[param2]);
			m_WaitingScript = NULL;
			Game->m_State = GAME_RUNNING;
			((CAdGame *)Game)->m_StateEx = GAME_NORMAL;
			m_Ready = true;
			InvalidateButtons();
			ClearResponses();
		} else return CBObject::Listen(param1, param2);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_Font));
	PersistMgr->Transfer(TMEMBER(m_FontHover));
	PersistMgr->Transfer(TMEMBER(m_Horizontal));
	PersistMgr->Transfer(TMEMBER(m_LastResponseText));
	PersistMgr->Transfer(TMEMBER(m_LastResponseTextOrig));
	m_RespButtons.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_ResponseArea));
	m_Responses.Persist(PersistMgr);
	PersistMgr->Transfer(TMEMBER(m_ScrollOffset));
	PersistMgr->Transfer(TMEMBER(m_ShieldWindow));
	PersistMgr->Transfer(TMEMBER(m_Spacing));
	PersistMgr->Transfer(TMEMBER(m_WaitingScript));
	PersistMgr->Transfer(TMEMBER(m_Window));

	PersistMgr->Transfer(TMEMBER_INT(m_VerticalAlign));
	PersistMgr->Transfer(TMEMBER_INT(m_Align));

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::WeedResponses() {
	CAdGame *AdGame = (CAdGame *)Game;

	for (int i = 0; i < m_Responses.GetSize(); i++) {
		switch (m_Responses[i]->m_ResponseType) {
		case RESPONSE_ONCE:
			if (AdGame->BranchResponseUsed(m_Responses[i]->m_ID)) {
				delete m_Responses[i];
				m_Responses.RemoveAt(i);
				i--;
			}
			break;

		case RESPONSE_ONCE_GAME:
			if (AdGame->GameResponseUsed(m_Responses[i]->m_ID)) {
				delete m_Responses[i];
				m_Responses.RemoveAt(i);
				i--;
			}
			break;
		}
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CAdResponseBox::SetLastResponseText(char *Text, char *TextOrig) {
	CBUtils::SetString(&m_LastResponseText, Text);
	CBUtils::SetString(&m_LastResponseTextOrig, TextOrig);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::HandleResponse(CAdResponse *Response) {
	SetLastResponseText(Response->m_Text, Response->m_TextOrig);

	CAdGame *AdGame = (CAdGame *)Game;

	switch (Response->m_ResponseType) {
	case RESPONSE_ONCE:
		AdGame->AddBranchResponse(Response->m_ID);
		break;

	case RESPONSE_ONCE_GAME:
		AdGame->AddGameResponse(Response->m_ID);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::GetNextAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = 0; i < Objects.GetSize(); i++) {
				if (Objects[i] == CurrObject) {
					if (i < Objects.GetSize() - 1) return Objects[i + 1];
					else break;
				}
			}
		}
		return Objects[0];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
CBObject *CAdResponseBox::GetPrevAccessObject(CBObject *CurrObject) {
	CBArray<CUIObject *, CUIObject *> Objects;
	GetObjects(Objects, true);

	if (Objects.GetSize() == 0) return NULL;
	else {
		if (CurrObject != NULL) {
			for (int i = Objects.GetSize() - 1; i >= 0; i--) {
				if (Objects[i] == CurrObject) {
					if (i > 0) return Objects[i - 1];
					else break;
				}
			}
		}
		return Objects[Objects.GetSize() - 1];
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdResponseBox::GetObjects(CBArray<CUIObject *, CUIObject *>& Objects, bool InteractiveOnly) {
	for (int i = 0; i < m_RespButtons.GetSize(); i++) {
		Objects.Add(m_RespButtons[i]);
	}
	if (m_Window) m_Window->GetWindowObjects(Objects, InteractiveOnly);

	return S_OK;
}

} // end of namespace WinterMute
