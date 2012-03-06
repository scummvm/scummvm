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
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/UIButton.h"
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BActiveRect.h"
#include "engines/wintermute/BFontStorage.h"
#include "engines/wintermute/BFont.h"
#include "engines/wintermute/BStringTable.h"
#include "engines/wintermute/BSprite.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"
#include "engines/wintermute/scriptables/ScValue.h"
#include "engines/wintermute/scriptables/ScScript.h"
#include "engines/wintermute/scriptables/ScStack.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUIButton, false)

//////////////////////////////////////////////////////////////////////////
CUIButton::CUIButton(CBGame *inGame): CUIObject(inGame) {
	m_BackPress = m_BackHover = m_BackDisable = m_BackFocus = NULL;

	m_FontHover = m_FontPress = m_FontDisable = m_FontFocus = NULL;

	m_ImageDisable = m_ImagePress = m_ImageHover = m_ImageFocus = NULL;

	m_Align = TAL_CENTER;

	m_Hover = m_Press = false;

	m_Type = UI_BUTTON;

	m_CanFocus = false;
	m_StayPressed = false;

	m_OneTimePress = false;
	m_CenterImage = false;

	m_PixelPerfect = false;
}


//////////////////////////////////////////////////////////////////////////
CUIButton::~CUIButton() {
	if (m_BackPress) delete m_BackPress;
	if (m_BackHover) delete m_BackHover;
	if (m_BackDisable) delete m_BackDisable;
	if (m_BackFocus) delete m_BackFocus;

	if (!m_SharedFonts) {
		if (m_FontHover)   Game->m_FontStorage->RemoveFont(m_FontHover);
		if (m_FontPress)   Game->m_FontStorage->RemoveFont(m_FontPress);
		if (m_FontDisable) Game->m_FontStorage->RemoveFont(m_FontDisable);
		if (m_FontFocus)   Game->m_FontStorage->RemoveFont(m_FontFocus);
	}

	if (!m_SharedImages) {
		if (m_ImageHover)   delete m_ImageHover;
		if (m_ImagePress)   delete m_ImagePress;
		if (m_ImageDisable) delete m_ImageDisable;
		if (m_ImageFocus)   delete m_ImageFocus;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUIButton::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing BUTTON file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(BUTTON)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(DISABLED)
TOKEN_DEF(VISIBLE)
TOKEN_DEF(FOCUSABLE)
TOKEN_DEF(BACK_HOVER)
TOKEN_DEF(BACK_PRESS)
TOKEN_DEF(BACK_DISABLE)
TOKEN_DEF(BACK_FOCUS)
TOKEN_DEF(BACK)
TOKEN_DEF(CENTER_IMAGE)
TOKEN_DEF(IMAGE_HOVER)
TOKEN_DEF(IMAGE_PRESS)
TOKEN_DEF(IMAGE_DISABLE)
TOKEN_DEF(IMAGE_FOCUS)
TOKEN_DEF(IMAGE)
TOKEN_DEF(FONT_HOVER)
TOKEN_DEF(FONT_PRESS)
TOKEN_DEF(FONT_DISABLE)
TOKEN_DEF(FONT_FOCUS)
TOKEN_DEF(FONT)
TOKEN_DEF(TEXT_ALIGN)
TOKEN_DEF(TEXT)
TOKEN_DEF(X)
TOKEN_DEF(Y)
TOKEN_DEF(WIDTH)
TOKEN_DEF(HEIGHT)
TOKEN_DEF(CURSOR)
TOKEN_DEF(NAME)
TOKEN_DEF(EVENTS)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(CAPTION)
TOKEN_DEF(PARENT_NOTIFY)
TOKEN_DEF(PRESSED)
TOKEN_DEF(PIXEL_PERFECT)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(BUTTON)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(DISABLED)
	TOKEN_TABLE(VISIBLE)
	TOKEN_TABLE(FOCUSABLE)
	TOKEN_TABLE(BACK_HOVER)
	TOKEN_TABLE(BACK_PRESS)
	TOKEN_TABLE(BACK_DISABLE)
	TOKEN_TABLE(BACK_FOCUS)
	TOKEN_TABLE(BACK)
	TOKEN_TABLE(CENTER_IMAGE)
	TOKEN_TABLE(IMAGE_HOVER)
	TOKEN_TABLE(IMAGE_PRESS)
	TOKEN_TABLE(IMAGE_DISABLE)
	TOKEN_TABLE(IMAGE_FOCUS)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(FONT_HOVER)
	TOKEN_TABLE(FONT_PRESS)
	TOKEN_TABLE(FONT_DISABLE)
	TOKEN_TABLE(FONT_FOCUS)
	TOKEN_TABLE(FONT)
	TOKEN_TABLE(TEXT_ALIGN)
	TOKEN_TABLE(TEXT)
	TOKEN_TABLE(X)
	TOKEN_TABLE(Y)
	TOKEN_TABLE(WIDTH)
	TOKEN_TABLE(HEIGHT)
	TOKEN_TABLE(CURSOR)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(EVENTS)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(PARENT_NOTIFY)
	TOKEN_TABLE(PRESSED)
	TOKEN_TABLE(PIXEL_PERFECT)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_BUTTON) {
			Game->LOG(0, "'BUTTON' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while (cmd > 0 && (cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
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

		case TOKEN_BACK_HOVER:
			delete m_BackHover;
			m_BackHover = new CUITiledImage(Game);
			if (!m_BackHover || FAILED(m_BackHover->LoadFile((char *)params))) {
				delete m_BackHover;
				m_BackHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_PRESS:
			delete m_BackPress;
			m_BackPress = new CUITiledImage(Game);
			if (!m_BackPress || FAILED(m_BackPress->LoadFile((char *)params))) {
				delete m_BackPress;
				m_BackPress = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_DISABLE:
			delete m_BackDisable;
			m_BackDisable = new CUITiledImage(Game);
			if (!m_BackDisable || FAILED(m_BackDisable->LoadFile((char *)params))) {
				delete m_BackDisable;
				m_BackDisable = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_BACK_FOCUS:
			delete m_BackFocus;
			m_BackFocus = new CUITiledImage(Game);
			if (!m_BackFocus || FAILED(m_BackFocus->LoadFile((char *)params))) {
				delete m_BackFocus;
				m_BackFocus = NULL;
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

		case TOKEN_IMAGE_HOVER:
			delete m_ImageHover;
			m_ImageHover = new CBSprite(Game);
			if (!m_ImageHover || FAILED(m_ImageHover->LoadFile((char *)params))) {
				delete m_ImageHover;
				m_ImageHover = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_PRESS:
			delete m_ImagePress;
			m_ImagePress = new CBSprite(Game);
			if (!m_ImagePress || FAILED(m_ImagePress->LoadFile((char *)params))) {
				delete m_ImagePress;
				m_ImagePress = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_DISABLE:
			delete m_ImageDisable;
			m_ImageDisable = new CBSprite(Game);
			if (!m_ImageDisable || FAILED(m_ImageDisable->LoadFile((char *)params))) {
				delete m_ImageDisable;
				m_ImageDisable = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE_FOCUS:
			delete m_ImageFocus;
			m_ImageFocus = new CBSprite(Game);
			if (!m_ImageFocus || FAILED(m_ImageFocus->LoadFile((char *)params))) {
				delete m_ImageFocus;
				m_ImageFocus = NULL;
				cmd = PARSERR_GENERIC;
			}
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

		case TOKEN_FONT_PRESS:
			if (m_FontPress) Game->m_FontStorage->RemoveFont(m_FontPress);
			m_FontPress = Game->m_FontStorage->AddFont((char *)params);
			if (!m_FontPress) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_DISABLE:
			if (m_FontDisable) Game->m_FontStorage->RemoveFont(m_FontDisable);
			m_FontDisable = Game->m_FontStorage->AddFont((char *)params);
			if (!m_FontDisable) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_FONT_FOCUS:
			if (m_FontFocus) Game->m_FontStorage->RemoveFont(m_FontFocus);
			m_FontFocus = Game->m_FontStorage->AddFont((char *)params);
			if (!m_FontFocus) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_TEXT:
			SetText((char *)params);
			Game->m_StringTable->Expand(&m_Text);
			break;

		case TOKEN_TEXT_ALIGN:
			if (scumm_stricmp((char *)params, "left") == 0) m_Align = TAL_LEFT;
			else if (scumm_stricmp((char *)params, "right") == 0) m_Align = TAL_RIGHT;
			else m_Align = TAL_CENTER;
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

		case TOKEN_SCRIPT:
			AddScript((char *)params);
			break;

		case TOKEN_PARENT_NOTIFY:
			parser.ScanStr((char *)params, "%b", &m_ParentNotify);
			break;

		case TOKEN_DISABLED:
			parser.ScanStr((char *)params, "%b", &m_Disable);
			break;

		case TOKEN_VISIBLE:
			parser.ScanStr((char *)params, "%b", &m_Visible);
			break;

		case TOKEN_FOCUSABLE:
			parser.ScanStr((char *)params, "%b", &m_CanFocus);
			break;

		case TOKEN_CENTER_IMAGE:
			parser.ScanStr((char *)params, "%b", &m_CenterImage);
			break;

		case TOKEN_PRESSED:
			parser.ScanStr((char *)params, "%b", &m_StayPressed);
			break;

		case TOKEN_PIXEL_PERFECT:
			parser.ScanStr((char *)params, "%b", &m_PixelPerfect);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in BUTTON definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading BUTTON definition");
		return E_FAIL;
	}

	CorrectSize();

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "BUTTON\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "\n");

	if (m_Back && m_Back->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK=\"%s\"\n", m_Back->m_Filename);
	if (m_BackHover && m_BackHover->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_HOVER=\"%s\"\n", m_BackHover->m_Filename);
	if (m_BackPress && m_BackPress->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_PRESS=\"%s\"\n", m_BackPress->m_Filename);
	if (m_BackDisable && m_BackDisable->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_DISABLE=\"%s\"\n", m_BackDisable->m_Filename);
	if (m_BackFocus && m_BackFocus->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "BACK_FOCUS=\"%s\"\n", m_BackFocus->m_Filename);

	if (m_Image && m_Image->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", m_Image->m_Filename);
	if (m_ImageHover && m_ImageHover->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_HOVER=\"%s\"\n", m_ImageHover->m_Filename);
	if (m_ImagePress && m_ImagePress->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_PRESS=\"%s\"\n", m_ImagePress->m_Filename);
	if (m_ImageDisable && m_ImageDisable->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_DISABLE=\"%s\"\n", m_ImageDisable->m_Filename);
	if (m_ImageFocus && m_ImageFocus->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE_FOCUS=\"%s\"\n", m_ImageFocus->m_Filename);

	if (m_Font && m_Font->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT=\"%s\"\n", m_Font->m_Filename);
	if (m_FontHover && m_FontHover->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_HOVER=\"%s\"\n", m_FontHover->m_Filename);
	if (m_FontPress && m_FontPress->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_PRESS=\"%s\"\n", m_FontPress->m_Filename);
	if (m_FontDisable && m_FontDisable->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_DISABLE=\"%s\"\n", m_FontDisable->m_Filename);
	if (m_FontFocus && m_FontFocus->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "FONT_FOCUS=\"%s\"\n", m_FontFocus->m_Filename);

	if (m_Cursor && m_Cursor->m_Filename)
		Buffer->PutTextIndent(Indent + 2, "CURSOR=\"%s\"\n", m_Cursor->m_Filename);


	Buffer->PutTextIndent(Indent + 2, "\n");

	if (m_Text)
		Buffer->PutTextIndent(Indent + 2, "TEXT=\"%s\"\n", m_Text);

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

	Buffer->PutTextIndent(Indent + 2, "\n");

	Buffer->PutTextIndent(Indent + 2, "X=%d\n", m_PosX);
	Buffer->PutTextIndent(Indent + 2, "Y=%d\n", m_PosY);
	Buffer->PutTextIndent(Indent + 2, "WIDTH=%d\n", m_Width);
	Buffer->PutTextIndent(Indent + 2, "HEIGHT=%d\n", m_Height);


	Buffer->PutTextIndent(Indent + 2, "DISABLED=%s\n", m_Disable ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "VISIBLE=%s\n", m_Visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PARENT_NOTIFY=%s\n", m_ParentNotify ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "FOCUSABLE=%s\n", m_CanFocus ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "CENTER_IMAGE=%s\n", m_CenterImage ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PRESSED=%s\n", m_StayPressed ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "PIXEL_PERFECT=%s\n", m_PixelPerfect ? "TRUE" : "FALSE");

	Buffer->PutTextIndent(Indent + 2, "\n");

	// scripts
	for (int i = 0; i < m_Scripts.GetSize(); i++) {
		Buffer->PutTextIndent(Indent + 2, "SCRIPT=\"%s\"\n", m_Scripts[i]->m_Filename);
	}

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CUIButton::CorrectSize() {
	RECT rect;

	CBSprite *img = NULL;
	if (m_Image) img = m_Image;
	else if (m_ImageDisable) img = m_ImageDisable;
	else if (m_ImageHover) img = m_ImageHover;
	else if (m_ImagePress) img = m_ImagePress;
	else if (m_ImageFocus) img = m_ImageFocus;

	if (m_Width <= 0) {
		if (img) {
			img->GetBoundingRect(&rect, 0, 0);
			m_Width = rect.right - rect.left;
		} else m_Width = 100;
	}

	if (m_Height <= 0) {
		if (img) {
			img->GetBoundingRect(&rect, 0, 0);
			m_Height = rect.bottom - rect.top;
		}
	}

	if (m_Text) {
		int text_height;
		if (m_Font) text_height = m_Font->GetTextHeight((byte  *)m_Text, m_Width);
		else text_height = Game->m_SystemFont->GetTextHeight((byte  *)m_Text, m_Width);

		if (text_height > m_Height) m_Height = text_height;
	}

	if (m_Height <= 0) m_Height = 100;

	if (m_Back) m_Back->CorrectSize(&m_Width, &m_Height);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::Display(int OffsetX, int OffsetY) {
	if (!m_Visible) return S_OK;

	CUITiledImage *back = NULL;
	CBSprite *image = NULL;
	CBFont *font = 0;

	//RECT rect;
	//CBPlatform::SetRect(&rect, OffsetX + m_PosX, OffsetY + m_PosY, OffsetX+m_PosX+m_Width, OffsetY+m_PosY+m_Height);
	//m_Hover = (!m_Disable && CBPlatform::PtInRect(&rect, Game->m_MousePos)!=FALSE);
	m_Hover = (!m_Disable && Game->m_ActiveObject == this && (Game->m_Interactive || Game->m_State == GAME_SEMI_FROZEN));

	if ((m_Press && m_Hover && !Game->m_MouseLeftDown) ||
	        m_OneTimePress && CBPlatform::GetTime() - m_OneTimePressTime >= 100) Press();


	if (m_Disable) {
		if (m_BackDisable) back = m_BackDisable;
		if (m_ImageDisable) image = m_ImageDisable;
		if (m_Text && m_FontDisable) font = m_FontDisable;
	} else if (m_Press || m_OneTimePress || m_StayPressed) {
		if (m_BackPress) back = m_BackPress;
		if (m_ImagePress) image = m_ImagePress;
		if (m_Text && m_FontPress) font = m_FontPress;
	} else if (m_Hover) {
		if (m_BackHover) back = m_BackHover;
		if (m_ImageHover) image = m_ImageHover;
		if (m_Text && m_FontHover) font = m_FontHover;
	} else if (m_CanFocus && IsFocused()) {
		if (m_BackFocus) back = m_BackFocus;
		if (m_ImageFocus) image = m_ImageFocus;
		if (m_Text && m_FontFocus) font = m_FontFocus;
	}

	if (!back && m_Back) back = m_Back;
	if (!image && m_Image) image = m_Image;
	if (m_Text && !font) {
		if (m_Font) font = m_Font;
		else font = Game->m_SystemFont;
	}

	int ImageX = OffsetX + m_PosX;
	int ImageY = OffsetY + m_PosY;

	if (image && m_CenterImage) {
		RECT rc;
		image->GetBoundingRect(&rc, 0, 0);
		ImageX += (m_Width - (rc.right - rc.left)) / 2;
		ImageY += (m_Height - (rc.bottom - rc.top)) / 2;
	}

	if (back) back->Display(OffsetX + m_PosX, OffsetY + m_PosY, m_Width, m_Height);
	//if(image) image->Draw(ImageX +((m_Press||m_OneTimePress)&&back?1:0), ImageY +((m_Press||m_OneTimePress)&&back?1:0), NULL);
	if (image) image->Draw(ImageX + ((m_Press || m_OneTimePress) && back ? 1 : 0), ImageY + ((m_Press || m_OneTimePress) && back ? 1 : 0), m_PixelPerfect ? this : NULL);

	if (font && m_Text) {
		int text_offset = (m_Height - font->GetTextHeight((byte  *)m_Text, m_Width)) / 2;
		font->DrawText((byte  *)m_Text, OffsetX + m_PosX + ((m_Press || m_OneTimePress) ? 1 : 0), OffsetY + m_PosY + text_offset + ((m_Press || m_OneTimePress) ? 1 : 0), m_Width, m_Align);
	}

	if (!m_PixelPerfect || !m_Image) Game->m_Renderer->m_RectList.Add(new CBActiveRect(Game, this, NULL, OffsetX + m_PosX, OffsetY + m_PosY, m_Width, m_Height, 100, 100, false));

	// reset unused sprites
	if (m_Image && m_Image != image) m_Image->Reset();
	if (m_ImageDisable && m_ImageDisable != image) m_ImageDisable->Reset();
	if (m_ImageFocus && m_ImageFocus != image) m_ImageFocus->Reset();
	if (m_ImagePress && m_ImagePress != image) m_ImagePress->Reset();
	if (m_ImageHover && m_ImageHover != image) m_ImageHover->Reset();

	m_Press = m_Hover && Game->m_MouseLeftDown && Game->m_CapturedObject == this;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
void CUIButton::Press() {
	ApplyEvent("Press");
	if (m_ListenerObject) m_ListenerObject->Listen(m_ListenerParamObject, m_ListenerParamDWORD);
	if (m_ParentNotify && m_Parent) m_Parent->ApplyEvent(m_Name);

	m_OneTimePress = false;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::ScCallMethod(CScScript *Script, CScStack *Stack, CScStack *ThisStack, char *Name) {
	//////////////////////////////////////////////////////////////////////////
	// SetDisabledFont
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "SetDisabledFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (m_FontDisable) Game->m_FontStorage->RemoveFont(m_FontDisable);
		if (Val->IsNULL()) {
			m_FontDisable = NULL;
			Stack->PushBool(true);
		} else {
			m_FontDisable = Game->m_FontStorage->AddFont(Val->GetString());
			Stack->PushBool(m_FontDisable != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetHoverFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHoverFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (m_FontHover) Game->m_FontStorage->RemoveFont(m_FontHover);
		if (Val->IsNULL()) {
			m_FontHover = NULL;
			Stack->PushBool(true);
		} else {
			m_FontHover = Game->m_FontStorage->AddFont(Val->GetString());
			Stack->PushBool(m_FontHover != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPressedFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (m_FontPress) Game->m_FontStorage->RemoveFont(m_FontPress);
		if (Val->IsNULL()) {
			m_FontPress = NULL;
			Stack->PushBool(true);
		} else {
			m_FontPress = Game->m_FontStorage->AddFont(Val->GetString());
			Stack->PushBool(m_FontPress != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedFont
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFocusedFont") == 0) {
		Stack->CorrectParams(1);
		CScValue *Val = Stack->Pop();

		if (m_FontFocus) Game->m_FontStorage->RemoveFont(m_FontFocus);
		if (Val->IsNULL()) {
			m_FontFocus = NULL;
			Stack->PushBool(true);
		} else {
			m_FontFocus = Game->m_FontStorage->AddFont(Val->GetString());
			Stack->PushBool(m_FontFocus != NULL);
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetDisabledImage") == 0) {
		Stack->CorrectParams(1);

		delete m_ImageDisable;
		m_ImageDisable = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!m_ImageDisable || FAILED(m_ImageDisable->LoadFile(Filename))) {
			delete m_ImageDisable;
			m_ImageDisable = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDisabledImage") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageDisable || !m_ImageDisable->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_ImageDisable->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetDisabledImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetDisabledImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageDisable) Stack->PushNULL();
		else Stack->PushNative(m_ImageDisable, true);

		return S_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetHoverImage") == 0) {
		Stack->CorrectParams(1);

		delete m_ImageHover;
		m_ImageHover = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!m_ImageHover || FAILED(m_ImageHover->LoadFile(Filename))) {
			delete m_ImageHover;
			m_ImageHover = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverImage") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageHover || !m_ImageHover->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_ImageHover->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetHoverImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetHoverImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageHover) Stack->PushNULL();
		else Stack->PushNative(m_ImageHover, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetPressedImage") == 0) {
		Stack->CorrectParams(1);

		delete m_ImagePress;
		m_ImagePress = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!m_ImagePress || FAILED(m_ImagePress->LoadFile(Filename))) {
			delete m_ImagePress;
			m_ImagePress = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetPressedImage") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImagePress || !m_ImagePress->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_ImagePress->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetPressedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetPressedImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImagePress) Stack->PushNULL();
		else Stack->PushNative(m_ImagePress, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "SetFocusedImage") == 0) {
		Stack->CorrectParams(1);

		delete m_ImageFocus;
		m_ImageFocus = new CBSprite(Game);
		char *Filename = Stack->Pop()->GetString();
		if (!m_ImageFocus || FAILED(m_ImageFocus->LoadFile(Filename))) {
			delete m_ImageFocus;
			m_ImageFocus = NULL;
			Stack->PushBool(false);
		} else Stack->PushBool(true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImage
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFocusedImage") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageFocus || !m_ImageFocus->m_Filename) Stack->PushNULL();
		else Stack->PushString(m_ImageFocus->m_Filename);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetFocusedImageObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "GetFocusedImageObject") == 0) {
		Stack->CorrectParams(0);
		if (!m_ImageFocus) Stack->PushNULL();
		else Stack->PushNative(m_ImageFocus, true);

		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Press
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Press") == 0) {
		Stack->CorrectParams(0);

		if (m_Visible && !m_Disable) {
			m_OneTimePress = true;
			m_OneTimePressTime = CBPlatform::GetTime();
		}
		Stack->PushNULL();

		return S_OK;
	}


	else return CUIObject::ScCallMethod(Script, Stack, ThisStack, Name);
}


//////////////////////////////////////////////////////////////////////////
CScValue *CUIButton::ScGetProperty(char *Name) {
	m_ScValue->SetNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "Type") == 0) {
		m_ScValue->SetString("button");
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "TextAlign") == 0) {
		m_ScValue->SetInt(m_Align);
		return m_ScValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Focusable") == 0) {
		m_ScValue->SetBool(m_CanFocus);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pressed") == 0) {
		m_ScValue->SetBool(m_StayPressed);
		return m_ScValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		m_ScValue->SetBool(m_PixelPerfect);
		return m_ScValue;
	}

	else return CUIObject::ScGetProperty(Name);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::ScSetProperty(char *Name, CScValue *Value) {
	//////////////////////////////////////////////////////////////////////////
	// TextAlign
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(Name, "TextAlign") == 0) {
		int i = Value->GetInt();
		if (i < 0 || i >= NUM_TEXT_ALIGN) i = 0;
		m_Align = (TTextAlign)i;
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Focusable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Focusable") == 0) {
		m_CanFocus = Value->GetBool();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// Pressed
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "Pressed") == 0) {
		m_StayPressed = Value->GetBool();
		return S_OK;
	}
	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(Name, "PixelPerfect") == 0) {
		m_PixelPerfect = Value->GetBool();
		return S_OK;
	}

	else return CUIObject::ScSetProperty(Name, Value);
}


//////////////////////////////////////////////////////////////////////////
char *CUIButton::ScToString() {
	return "[button]";
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUIButton::Persist(CBPersistMgr *PersistMgr) {

	CUIObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER_INT(m_Align));
	PersistMgr->Transfer(TMEMBER(m_BackDisable));
	PersistMgr->Transfer(TMEMBER(m_BackFocus));
	PersistMgr->Transfer(TMEMBER(m_BackHover));
	PersistMgr->Transfer(TMEMBER(m_BackPress));
	PersistMgr->Transfer(TMEMBER(m_CenterImage));
	PersistMgr->Transfer(TMEMBER(m_FontDisable));
	PersistMgr->Transfer(TMEMBER(m_FontFocus));
	PersistMgr->Transfer(TMEMBER(m_FontHover));
	PersistMgr->Transfer(TMEMBER(m_FontPress));
	PersistMgr->Transfer(TMEMBER(m_Hover));
	PersistMgr->Transfer(TMEMBER(m_Image));
	PersistMgr->Transfer(TMEMBER(m_ImageDisable));
	PersistMgr->Transfer(TMEMBER(m_ImageFocus));
	PersistMgr->Transfer(TMEMBER(m_ImageHover));
	PersistMgr->Transfer(TMEMBER(m_ImagePress));
	PersistMgr->Transfer(TMEMBER(m_PixelPerfect));
	PersistMgr->Transfer(TMEMBER(m_Press));
	PersistMgr->Transfer(TMEMBER(m_StayPressed));

	if (!PersistMgr->m_Saving) {
		m_OneTimePress = false;
		m_OneTimePressTime = 0;
	}

	return S_OK;
}

} // end of namespace WinterMute
