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
#include "engines/wintermute/UIWindow.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BActiveRect.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BKeyboardState.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/UIButton.h"
#include "engines/wintermute/UIEdit.h"
#include "engines/wintermute/UIText.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BViewport.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIWindow, false)

//////////////////////////////////////////////////////////////////////////
CUIWindow::CUIWindow(CBGame *inGame): CUIObject(inGame) {
	CBPlatform::SetRectEmpty(&m_TitleRect);
	CBPlatform::SetRectEmpty(&m_DragRect);
	m_TitleAlign = TAL_LEFT;
	m_Transparent = false;

	m_BackInactive = NULL;
	m_FontInactive = NULL;
	m_ImageInactive = NULL;

	m_Type = UI_WINDOW;
	m_CanFocus = true;

	m_Dragging = false;
	m_DragFrom.x = m_DragFrom.y = 0;

	m_Mode = WINDOW_NORMAL;
	m_ShieldWindow = NULL;
	m_ShieldButton = NULL;

	m_FadeColor = 0x00000000;
	m_FadeBackground = false;

	m_Ready = true;
	m_IsMenu = false;
	m_InGame = false;

	m_ClipContents = false;
	m_Viewport = NULL;

	m_PauseMusic = true;
}


//////////////////////////////////////////////////////////////////////////
CUIWindow::~CUIWindow() {
	Close();
	Cleanup();
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::Cleanup() {
	delete m_ShieldWindow;
	delete m_ShieldButton;
	delete m_Viewport;
	m_ShieldWindow = NULL;
	m_ShieldButton = NULL;
	m_Viewport = NULL;

	if (m_BackInactive) delete m_BackInactive;
	if (!m_SharedFonts && m_FontInactive) Game->m_FontStorage->RemoveFont(m_FontInactive);
	if (!m_SharedImages && m_ImageInactive) delete m_ImageInactive;

	for (int i = 0; i < m_Widgets.GetSize(); i++) delete m_Widgets[i];
	m_Widgets.RemoveAll();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Display(int OffsetX, int OffsetY) {
	// go exclusive
	if (m_Mode == WINDOW_EXCLUSIVE || m_Mode == WINDOW_SYSTEM_EXCLUSIVE) {
		if (!m_ShieldWindow) m_ShieldWindow = new CUIWindow(Game);
		if (m_ShieldWindow) {
			m_ShieldWindow->m_PosX = m_ShieldWindow->m_PosY = 0;
			m_ShieldWindow->m_Width = Game->m_Renderer->m_Width;
			m_ShieldWindow->m_Height = Game->m_Renderer->m_Height;

			m_ShieldWindow->Display();
		}
	} else if (m_IsMenu) {
		if (!m_ShieldButton) {
			m_ShieldButton = new CUIButton(Game);
			m_ShieldButton->SetName("close");
			m_ShieldButton->SetListener(this, m_ShieldButton, 0);
			m_ShieldButton->m_Parent = this;
		}
		if (m_ShieldButton) {
			m_ShieldButton->m_PosX = m_ShieldButton->m_PosY = 0;
			m_ShieldButton->m_Width = Game->m_Renderer->m_Width;
			m_ShieldButton->m_Height = Game->m_Renderer->m_Height;

			m_ShieldButton->Display();
		}
	}

	if (!m_Visible) return S_OK;

	if (m_FadeBackground) Game->m_Renderer->FadeToColor(m_FadeColor);

	if (m_Dragging) {
		m_PosX += (Game->m_MousePos.x - m_DragFrom.x);
		m_PosY += (Game->m_MousePos.y - m_DragFrom.y);

		m_DragFrom.x = Game->m_MousePos.x;
		m_DragFrom.y = Game->m_MousePos.y;
	}

	if (!m_FocusedWidget || (!m_FocusedWidget->m_CanFocus || m_FocusedWidget->m_Disable || !m_FocusedWidget->m_Visible)) {
		MoveFocus();
	}

	bool PopViewport = false;
	if (m_ClipContents) {
		if (!m_Viewport) m_Viewport = new CBViewport(Game);
		if (m_Viewport) {
			m_Viewport->SetRect(m_PosX + OffsetX, m_PosY + OffsetY, m_PosX + m_Width + OffsetX, m_PosY + m_Height + OffsetY);
			Game->PushViewport(m_Viewport);
			PopViewport = true;
		}
	}


	CUITiledImage *back = m_Back;
	CBSprite *image = m_Image;
	CBFont *font = m_Font;

	if (!IsFocused()) {
		if (m_BackInactive) back = m_BackInactive;
		if (m_ImageInactive) image = m_ImageInactive;
		if (m_FontInactive) font = m_FontInactive;
	}

	if (m_AlphaColor != 0) Game->m_Renderer->m_ForceAlphaColor = m_AlphaColor;
	if (back) back->Display(m_PosX + OffsetX, m_PosY + OffsetY, m_Width, m_Height);
	if (image) image->Draw(m_PosX + OffsetX, m_PosY + OffsetY, m_Transparent ? NULL : this);

	if (!CBPlatform::IsRectEmpty(&m_TitleRect) && font && m_Text) {
		font->DrawText((byte  *)m_Text, m_PosX + OffsetX + m_TitleRect.left, m_PosY + OffsetY + m_TitleRect.top, m_TitleRect.right - m_TitleRect.left, m_TitleAlign, m_TitleRect.bottom - m_TitleRect.top);
	}

	if (!m_Transparent && !image) Game->m_Renderer->m_RectList.Add(new CBActiveRect(Game, this, NULL, m_PosX + OffsetX, m_PosY + OffsetY, m_Width, m_Height, 100, 100, false));

	for (int i = 0; i < m_Widgets.GetSize(); i++) {
		m_Widgets[i]->Display(m_PosX + OffsetX, m_PosY + OffsetY);
	}

	if (m_AlphaColor != 0) Game->m_Renderer->m_ForceAlphaColor = 0;

	if (PopViewport) Game->PopViewport();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIWindow::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing WINDOW file '%s'", Filename);

	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(WINDOW)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(BACK_INACTIVE)
TOKEN_DEF(BACK)
TOKEN_DEF(IMAGE_INACTIVE)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_INACTIVE)
TOKEN_DEF(FONT)
TOKEN_DEF(TITLE_ALIGN)
TOKEN_DEF(TITLE_RECT)
TOKEN_DEF(TITLE)
TOKEN_DEF(DRAG_RECT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(FADE_ALPHA)
TOKEN_DEF(FADE_COLOR)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(BUTTON)
TOKEN_DEF(STATIC)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(MENU)
TOKEN_DEF(IN_GAME)
TOKEN_DEF(CLIP_CONTENTS)
TOKEN_DEF(PAUSE_MUSIC)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(EDIT)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(BACK_INACTIVE)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(IMAGE_INACTIVE)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_INACTIVE)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TITLE_ALIGN)
	TOKEN_TABLE(TITLE_RECT)
	TOKEN_TABLE(TITLE)
	TOKEN_TABLE(DRAG_RECT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(FADE_ALPHA)
	TOKEN_TABLE(FADE_COLOR)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(BUTTON)
	TOKEN_TABLE(STATIC)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(MENU)
	TOKEN_TABLE(IN_GAME)
	TOKEN_TABLE(CLIP_CONTENTS)
	TOKEN_TABLE(PAUSE_MUSIC)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(EDIT)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	int FadeR = 0, FadeG = 0, FadeB = 0, FadeA = 0;
	int ar = 0, ag = 0, ab = 0, alpha = 0;

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_WINDOW) {
			Game->LOG(0, "'WINDOW' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while (cmd >= PARSERR_TOKENNOTFOUND && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) >= PARSERR_TOKENNOTFOUND) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			SetName((char *)params);
			break;

		case TOKEN_CAPTION:
			SetCaption((char *)params);
			break;

		case TOKEN_BACK:
			delete m_Back;
			m_Back = new CUITiledImage(Game);
			if (!m_Back || FAILED(m_Back->LoadFile((char *)params))) {
				delete m_Back;
				m_Back = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_INACTIVE:
			delete m_BackInactive;
			m_BackInactive = new CUITiledImage(Game);
			if (!m_BackInactive || FAILED(m_BackInactive->LoadFile((char *)params))) {
				delete m_BackInactive;
				m_BackInactive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete m_Image;
			m_Image = new CBSprite(Game);
			if (!m_Image || FAILED(m_Image->LoadFile((char *)params))) {
				delete m_Image;
				m_Image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_INACTIVE:
			delete m_ImageInactive,
			m_ImageInactive = new CBSprite(Game);
			if (!m_ImageInactive || FAILED(m_ImageInactive->LoadFile((char *)params))) {
				delete m_ImageInactive;
				m_ImageInactive = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_FONT:
			if (m_Font) Game->m_FontStorage->RemoveFont(m_Font);
			m_Font = Game->m_FontStorage->AddFont((char *)params);
			if (!m_Font) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_INACTIVE:
			if (m_FontInactive) Game->m_FontStorage->RemoveFont(m_FontInactive);
			m_FontInactive = Game->m_FontStorage->AddFont((char *)params);
			if (!m_FontInactive) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TITLE:
			SetText((char *)params);
			Game->m_StringTable->Expand(&m_Text);
			break;

		case TOKEN_TITLE_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) m_TitleAlign = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) m_TitleAlign = TAL_RIGHT;
			else m_TitleAlign = TAL_CENTER;
			break;

		case TOKEN_TITLE_RECT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_TitleRect.left, &m_TitleRect.top, &m_TitleRect.right, &m_TitleRect.bottom);
			break;

		case TOKEN_DRAG_RECT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_DragRect.left, &m_DragRect.top, &m_DragRect.right, &m_DragRect.bottom);
			break;

		case TOKEN_X:
			parser.ScanStr((char *)params, "%d", &m_PosX);
			break;

		case TOKEN_Y:
			parser.ScanStr((char *)params, "%d", &m_PosY);
			break;

		case TOKEN_WIDTH:
			parser.ScanStr((char *)params, "%d", &m_Width);
			break;

		case TOKEN_HEIGHT:
			parser.ScanStr((char *)params, "%d", &m_Height);
			break;

		case TOKEN_CURSOR:
			delete m_Cursor;
			m_Cursor = new CBSprite(Game);
			if (!m_Cursor || FAILED(m_Cursor->LoadFile((char *)params))) {
				delete m_Cursor;
				m_Cursor = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BUTTON: {
			CUIButton *btn = new CUIButton(Game);
			if (!btn || FAILED(btn->LoadBuffer(params, false))) {
				delete btn;
				btn = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				btn->m_Parent = this;
				m_Widgets.Add(btn);
			}
		}
		break;

		case TOKEN_STATIC: {
			CUIText *text = new CUIText(Game);
			if (!text || FAILED(text->LoadBuffer(params, false))) {
				delete text;
				text = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				text->m_Parent = this;
				m_Widgets.Add(text);
			}
		}
		break;

		case TOKEN_EDIT: {
			CUIEdit *edit = new CUIEdit(Game);
			if (!edit || FAILED(edit->LoadBuffer(params, false))) {
				delete edit;
				edit = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				edit->m_Parent = this;
				m_Widgets.Add(edit);
			}
		}
		break;

		case TOKEN_WINDOW: {
			CUIWindow *win = new CUIWindow(Game);
			if (!win || FAILED(win->LoadBuffer(params, false))) {
				delete win;
				win = NULL;
				cmd = PARSERR_GENERIC;
			} else {
				win->m_Parent = this;
				m_Widgets.Add(win);
			}
		}
		break;


		case TOKEN_TRANSPARENT:
			parser.ScanStr((char *)params, "%b", &m_Transparent);
			break;

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.ScanStr((char *)params, "%b", &m_ParentNotify);
			break;

		case TOKEN_PAUSE_MUSIC:
			parser.ScanStr((char *)params, "%b", &m_PauseMusic);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &m_Disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &m_Visible);
			break;

		case TOKEN_MENU:
			parser.ScanStr((char *)params, "%b", &m_IsMenu);
			break;

		case TOKEN_IN_GAME:
			parser.ScanStr((char *)params, "%b", &m_InGame);
			break;

		case TOKEN_CLIP_CONTENTS:
			parser.ScanStr((char *)params, "%b", &m_ClipContents);
			break;

		case TOKEN_FADE_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &FadeR, &FadeG, &FadeB);
			m_FadeBackground = true;
			break;

		case TOKEN_FADE_ALPHA:
			parser.ScanStr((char *)params, "%d", &FadeA);
			m_FadeBackground = true;
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.ScanStr((char *)params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.ScanStr((char *)params, "%d", &alpha);
			break;


		default:
			if (FAILED(Game->WindowLoadHook(this, (char **)&Buffer, (char **)params))) {
				cmd = PARSERR_GENERIC;
			}
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in WINDOW definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading WINDOW definition");
		return E_FAIL;
	}

	CorrectSize();

	if (alpha != 0 && ar == 0 && ag == 0 && ab == 0) {
		ar = ag = ab = 255;
	}
	m_AlphaColor = DRGBA(ar, ag, ab, alpha);

	if (m_FadeBackground) m_FadeColor = DRGBA(FadeR, FadeG, FadeB, FadeA);

	m_FocusedWidget = NULL;

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "WINDOW\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (m_Back && m_Back->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK=\"%s\"\n", m_Back->m_Filename);
	if (m_BackInactive && m_BackInactive->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_INACTIVE=\"%s\"\n", m_BackInactive->m_Filename);

	if (m_Image && m_Image->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", m_Image->m_Filename);
	if (m_ImageInactive && m_ImageInactive->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_INACTIVE=\"%s\"\n", m_ImageInactive->m_Filename);

	if (m_Font && m_Font->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", m_Font->m_Filename);
	if (m_FontInactive && m_FontInactive->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_INACTIVE=\"%s\"\n", m_FontInactive->m_Filename);

	if (m_Cursor && m_Cursor->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", m_Cursor->m_Filename);

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (m_Text)
		Buffer->PutTextIndent(Indent + 2, "TITLE=\"%s\"\n", m_Text);

	switch (m_TitleAlign) {
	case TAL_LEFT:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "left");
		break;
	case TAL_RIGHT:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "right");
		break;
	case TAL_CENTER:
		Buffer->PutTextIndent(Indent + 2, "TITLE_ALIGN=\"%s\"\n", "center");
		break;
	}

	if (!CBPlatform::IsRectEmpty(&m_TitleRect)) {
		Buffer->PutTextIndent(Indent + 2, "TITLE_RECT { %d, %d, %d, %d }\n", m_TitleRect.left, m_TitleRect.top, m_TitleRect.right, m_TitleRect.bottom);
	}

	if (!CBPlatform::IsRectEmpty(&m_DragRect)) {
		Buffer->PutTextIndent(Indent + 2, "DRAG_RECT { %d, %d, %d, %d }\n", m_DragRect.left, m_DragRect.top, m_DragRect.right, m_DragRect.bottom);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", m_PosX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", m_PosY);
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", m_Width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", m_Height);

	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", m_Disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", m_Visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", m_ParentNotify ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "TRANSPARENT=%s\n", m_Transparent ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PAUSE_MUSIC=%s\n", m_PauseMusic ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "MENU=%s\n", m_IsMenu ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "IN_GAME=%s\n", m_InGame ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CLIP_CONTENTS=%s\n", m_ClipContents ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (m_FadeBackground) {
		Buffer->PutTextIndent(Indent + 2, "FADE_COLOR { %d, %d, %d }\n", D3DCOLGetR(m_FadeColor), D3DCOLGetG(m_FadeColor), D3DCOLGetB(m_FadeColor));
		Buffer->PutTextIndent(Indent + 2, "FADE_ALPHA=%d\n", D3DCOLGetA(m_FadeColor));
	}

	Buffer->PutTextIndent(Indent + 2, "ALPHA_COLOR { %d, %d, %d }\n", D3DCOLGetR(m_AlphaColor), D3DCOLGetG(m_AlphaColor), D3DCOLGetB(m_AlphaColor));
	Buffer->PutTextIndent(Indent + 2, "ALPHA=%d\n", D3DCOLGetA(m_AlphaColor));

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	// controls
	for (int i = 0; i < m_Widgets.GetSize(); i++)
		m_Widgets[i]->SaveAsText(Buffer, Indent + 2);


	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::EnableWidget(char *Name, bool Enable) {
	for (int i = 0; i < m_Widgets.GetSize(); i++) {
		if (scumm_stricmp(m_Widgets[i]->m_Name, Name) == 0) m_Widgets[i]->m_Disable = !Enable;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ShowWidget(char *Name, bool Visible) {
	for (int i = 0; i < m_Widgets.GetSize(); i++) {
		if (scumm_stricmp(m_Widgets[i]->m_Name, Name) == 0) m_Widgets[i]->m_Visible = Visible;
	}
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// GetWidget / GetControl
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "GetWidget") == 0 || strcmp(Name, "GetControl") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		if (val->GetType() == VAL_INT) {
			int widget = val->GetInt();
			if (widget < 0 || widget >= m_Widgets.GetSize()) Stack->PushNULL();
			else Stack->PushNative(m_Widgets[widget], true);
		} else {
			for (int i = 0; i < m_Widgets.GetSize(); i++) {
				if (scumm_stricmp(m_Widgets[i]->m_Name, val->GetString()) == 0) {
					Stack->PushNative(m_Widgets[i], true);
					return S_OK;
				}
			}
			Stack->PushNULL();
		}

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetInactiveFont") == 0) {
		Stack->CorrectParams(1);

		if (m_FontInactive) Game->m_FontStorage->RemoveFont(m_FontInactive);
		m_FontInactive = Game->m_FontStorage->AddFont(Stack->Pop()->GetString());
		Stack->PushBool(m_FontInactive != NULL);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetInactiveImage") == 0) {
		Stack->CorrectParams(1);

		delete m_ImageInactive;
		m_ImageInactive = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!m_ImageInactive || FAILED(m_ImageInactive->LoadFile(Filename))) {
			delete m_ImageInactive;
			m_ImageInactive = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInactiveImage") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageInactive || !m_ImageInactive->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_ImageInactive->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetInactiveImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetInactiveImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageInactive) Stack->PushNULL();
		else Stack->PushNative(m_ImageInactive, true);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// Close
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Close") == 0) {
		Stack->CorrectParams(0);
		Stack->PushBool(SUCCEEDED(Close()));
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoExclusive") == 0) {
		Stack->CorrectParams(0);
		GoExclusive();
		Script->WaitFor(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GoSystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GoSystemExclusive") == 0) {
		Stack->CorrectParams(0);
		GoSystemExclusive();
		Script->WaitFor(this);
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Center
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Center") == 0) {
		Stack->CorrectParams(0);
		m_PosX = (Game->m_Renderer->m_Width - m_Width) / 2;
		m_PosY = (Game->m_Renderer->m_Height - m_Height) / 2;
		Stack->PushNULL();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadFromFile
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "LoadFromFile") == 0) {
		Stack->CorrectParams(1);

		CScValue *Val = Stack->Pop();
		Cleanup();
		if (!Val->IsNULL()) {
			Stack->PushBool(SUCCEEDED(LoadFile(Val->GetString())));
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateButton
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateButton") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIButton *Btn = new CUIButton(Game);
		if (!Val->IsNULL()) Btn->SetName(Val->GetString());
		Stack->PushNative(Btn, true);

		Btn->m_Parent = this;
		m_Widgets.Add(Btn);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateStatic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateStatic") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIText *Sta = new CUIText(Game);
		if (!Val->IsNULL()) Sta->SetName(Val->GetString());
		Stack->PushNative(Sta, true);

		Sta->m_Parent = this;
		m_Widgets.Add(Sta);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateEditor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateEditor") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIEdit *Edi = new CUIEdit(Game);
		if (!Val->IsNULL()) Edi->SetName(Val->GetString());
		Stack->PushNative(Edi, true);

		Edi->m_Parent = this;
		m_Widgets.Add(Edi);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// CreateWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "CreateWindow") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		CUIWindow *Win = new CUIWindow(Game);
		if (!Val->IsNULL()) Win->SetName(Val->GetString());
		Stack->PushNative(Win, true);

		Win->m_Parent = this;
		m_Widgets.Add(Win);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteControl / DeleteButton / DeleteStatic / DeleteEditor / DeleteWindow
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "DeleteControl") == 0 || strcmp(Name, "DeleteButton") == 0 || strcmp(Name, "DeleteStatic") == 0 || strcmp(Name, "DeleteEditor") == 0 || strcmp(Name, "DeleteWindow") == 0) {
		Stack->CorrectParams(1);
		CScValue *val = Stack->Pop();
		CUIObject *obj = (CUIObject *)val->GetNative();

		for (int i = 0; i < m_Widgets.GetSize(); i++) {
			if (m_Widgets[i] == obj) {
				delete m_Widgets[i];
				m_Widgets.RemoveAt(i);
				if (val->GetType() == VAL_VARIABLE_REF) val->SetNULL();
			}
		}
		Stack->PushNULL();
		return S_OK;
	} else if SUCCEEDED(Game->WindowScriptMethodHook(this, Script, Stack, Name)) return S_OK;

	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIWindow::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("window");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumWidgets / NumControls (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "NumWidgets") == 0 || strcmp(Name, "NumControls") == 0) {
		m_ScValue->SetInt(m_Widgets.GetSize());
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Exclusive") == 0) {
		m_ScValue->SetBool(m_Mode == WINDOW_EXCLUSIVE);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SystemExclusive") == 0) {
		m_ScValue->SetBool(m_Mode == WINDOW_SYSTEM_EXCLUSIVE);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Menu") == 0) {
		m_ScValue->SetBool(m_IsMenu);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InGame") == 0) {
		m_ScValue->SetBool(m_InGame);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseMusic") == 0) {
		m_ScValue->SetBool(m_PauseMusic);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClipContents") == 0) {
		m_ScValue->SetBool(m_ClipContents);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Transparent") == 0) {
		m_ScValue->SetBool(m_Transparent);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeColor") == 0) {
		m_ScValue->SetInt((int)m_FadeColor);
		return m_ScValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// Name
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Name") == 0) {
		SetName(Value->GetString());
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Menu
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Menu") == 0) {
		m_IsMenu = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InGame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "InGame") == 0) {
		m_InGame = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseMusic
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PauseMusic") == 0) {
		m_PauseMusic = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ClipContents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "ClipContents") == 0) {
		m_ClipContents = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Transparent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Transparent") == 0) {
		m_Transparent = Value->GetBool();
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// FadeColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "FadeColor") == 0) {
		m_FadeColor = (uint32)Value->GetInt();
		m_FadeBackground = (m_FadeColor != 0);
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Exclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Exclusive") == 0) {
		if (Value->GetBool())
			GoExclusive();
		else {
			Close();
			m_Visible = true;
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SystemExclusive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SystemExclusive") == 0) {
		if (Value->GetBool())
			GoSystemExclusive();
		else {
			Close();
			m_Visible = true;
		}
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CUIWindow::ScToString() {
	return "[window]";
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::HandleKeypress(SDL_Event *event) {
//TODO
#if 0
	if (event->type == SDL_KEYDOWN && event->key.keysym.scancode == SDL_SCANCODE_TAB) {
		return SUCCEEDED(MoveFocus(!CBKeyboardState::IsShiftDown()));
	} else {
		if (m_FocusedWidget) return m_FocusedWidget->HandleKeypress(event);
		else return false;
	}
#endif
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool CUIWindow::HandleMouseWheel(int Delta) {
	if (m_FocusedWidget) return m_FocusedWidget->HandleMouseWheel(Delta);
	else return false;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::HandleMouse(TMouseEvent Event, TMouseButton Button) {
	HRESULT res = CUIObject::HandleMouse(Event, Button);

	// handle window dragging
	if (!CBPlatform::IsRectEmpty(&m_DragRect)) {
		// start drag
		if (Event == MOUSE_CLICK && Button == MOUSE_BUTTON_LEFT) {
			RECT DragRect = m_DragRect;
			int OffsetX, OffsetY;
			GetTotalOffset(&OffsetX, &OffsetY);
			CBPlatform::OffsetRect(&DragRect, m_PosX + OffsetX, m_PosY + OffsetY);

			if (CBPlatform::PtInRect(&DragRect, Game->m_MousePos)) {
				m_DragFrom.x = Game->m_MousePos.x;
				m_DragFrom.y = Game->m_MousePos.y;
				m_Dragging = true;
			}
		}
		// end drag
		else if (m_Dragging && Event == MOUSE_RELEASE && Button == MOUSE_BUTTON_LEFT) {
			m_Dragging = false;
		}
	}

	return res;
}



//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_BackInactive));
	PersistMgr->Transfer(TMEMBER(m_ClipContents));
	PersistMgr->Transfer(TMEMBER(m_DragFrom));
	PersistMgr->Transfer(TMEMBER(m_Dragging));
	PersistMgr->Transfer(TMEMBER(m_DragRect));
	PersistMgr->Transfer(TMEMBER(m_FadeBackground));
	PersistMgr->Transfer(TMEMBER(m_FadeColor));
	PersistMgr->Transfer(TMEMBER(m_FontInactive));
	PersistMgr->Transfer(TMEMBER(m_ImageInactive));
	PersistMgr->Transfer(TMEMBER(m_InGame));
	PersistMgr->Transfer(TMEMBER(m_IsMenu));
	PersistMgr->Transfer(TMEMBER_INT(m_Mode));
	PersistMgr->Transfer(TMEMBER(m_ShieldButton));
	PersistMgr->Transfer(TMEMBER(m_ShieldWindow));
	PersistMgr->Transfer(TMEMBER_INT(m_TitleAlign));
	PersistMgr->Transfer(TMEMBER(m_TitleRect));
	PersistMgr->Transfer(TMEMBER(m_Transparent));
	PersistMgr->Transfer(TMEMBER(m_Viewport));
	PersistMgr->Transfer(TMEMBER(m_PauseMusic));

	m_Widgets.Persist(PersistMgr);

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::MoveFocus(bool Forward) {
	int i;
	bool found = false;
	for (i = 0; i < m_Widgets.GetSize(); i++) {
		if (m_Widgets[i] == m_FocusedWidget) {
			found = true;
			break;
		}
	}
	if (!found) m_FocusedWidget = NULL;

	if (!m_FocusedWidget) {
		if (m_Widgets.GetSize() > 0) i = 0;
		else return S_OK;
	}

	int NumTries = 0;
	bool done = false;

	while (NumTries <= m_Widgets.GetSize()) {
		if (m_Widgets[i] != m_FocusedWidget && m_Widgets[i]->m_CanFocus && m_Widgets[i]->m_Visible && !m_Widgets[i]->m_Disable) {
			m_FocusedWidget = m_Widgets[i];
			done = true;
			break;
		}

		if (Forward) {
			i++;
			if (i >= m_Widgets.GetSize()) i = 0;
		} else {
			i--;
			if (i < 0) i = m_Widgets.GetSize() - 1;
		}
		NumTries++;
	}

	return done ? S_OK : E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GoExclusive() {
	if (m_Mode == WINDOW_EXCLUSIVE) return S_OK;

	if (m_Mode == WINDOW_NORMAL) {
		m_Ready = false;
		m_Mode = WINDOW_EXCLUSIVE;
		m_Visible = true;
		m_Disable = false;
		Game->FocusWindow(this);
		return S_OK;
	} else return E_FAIL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GoSystemExclusive() {
	if (m_Mode == WINDOW_SYSTEM_EXCLUSIVE) return S_OK;

	MakeFreezable(false);

	m_Mode = WINDOW_SYSTEM_EXCLUSIVE;
	m_Ready = false;
	m_Visible = true;
	m_Disable = false;
	Game->FocusWindow(this);

	Game->Freeze(m_PauseMusic);
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Close() {
	if (m_Mode == WINDOW_SYSTEM_EXCLUSIVE) {
		Game->Unfreeze();
	}

	m_Mode = WINDOW_NORMAL;
	m_Visible = false;
	m_Ready = true;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->m_Type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->m_Name, "close") == 0) Close();
		else return CBObject::Listen(param1, param2);
		break;
	default:
		return CBObject::Listen(param1, param2);
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIWindow::MakeFreezable(bool Freezable) {
	for (int i = 0; i < m_Widgets.GetSize(); i++)
		m_Widgets[i]->MakeFreezable(Freezable);

	CBObject::MakeFreezable(Freezable);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIWindow::GetWindowObjects(CBArray<CUIObject *, CUIObject *>& Objects, bool InteractiveOnly) {
	for (int i = 0; i < m_Widgets.GetSize(); i++) {
		CUIObject *Control = m_Widgets[i];
		if (Control->m_Disable && InteractiveOnly) continue;

		switch (Control->m_Type) {
		case UI_WINDOW:
			((CUIWindow *)Control)->GetWindowObjects(Objects, InteractiveOnly);
			break;

		case UI_BUTTON:
		case UI_EDIT:
			Objects.Add(Control);
			break;

		default:
			if (!InteractiveOnly) Objects.Add(Control);
		}
	}
	return S_OK;
}

} // end of namespace WinterMute
