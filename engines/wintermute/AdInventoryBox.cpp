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
#include "AdInventoryBox.h"
#include "AdInventory.h"
#include "AdItem.h"
#include "BGame.h"
#include "BParser.h"
#include "BFileManager.h"
#include "BViewport.h"
#include "BDynBuffer.h"
#include "UIButton.h"
#include "UIWindow.h"
#include "PlatformSDL.h"
#include "common/str.h"
#include <cmath>


namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdInventoryBox, false)

//////////////////////////////////////////////////////////////////////////
CAdInventoryBox::CAdInventoryBox(CBGame *inGame): CBObject(inGame) {
	CBPlatform::SetRectEmpty(&m_ItemsArea);
	m_ScrollOffset = 0;
	m_Spacing = 0;
	m_ItemWidth = m_ItemHeight = 50;
	m_ScrollBy = 1;

	m_Window = NULL;
	m_CloseButton = NULL;

	m_HideSelected = false;

	m_Visible = false;
	m_Exclusive = false;
}


//////////////////////////////////////////////////////////////////////////
CAdInventoryBox::~CAdInventoryBox() {
	Game->UnregisterObject(m_Window);
	m_Window = NULL;

	SAFE_DELETE(m_CloseButton);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->m_Type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->m_Name, "close") == 0) {
			m_Visible = false;
		} else if (scumm_stricmp(obj->m_Name, "prev") == 0) {
			m_ScrollOffset -= m_ScrollBy;
			m_ScrollOffset = std::max(m_ScrollOffset, 0);
		} else if (scumm_stricmp(obj->m_Name, "next") == 0) {
			m_ScrollOffset += m_ScrollBy;
		} else return CBObject::Listen(param1, param2);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::Display() {
	CAdGame *AdGame = (CAdGame *)Game;

	if (!m_Visible) return S_OK;

	int ItemsX, ItemsY;
	ItemsX = floor((float)((m_ItemsArea.right - m_ItemsArea.left + m_Spacing) / (m_ItemWidth + m_Spacing)));
	ItemsY = floor((float)((m_ItemsArea.bottom - m_ItemsArea.top + m_Spacing) / (m_ItemHeight + m_Spacing)));

	if (m_Window) {
		m_Window->EnableWidget("prev", m_ScrollOffset > 0);
		m_Window->EnableWidget("next", m_ScrollOffset + ItemsX * ItemsY < AdGame->m_InventoryOwner->GetInventory()->m_TakenItems.GetSize());
	}


	if (m_CloseButton) {
		m_CloseButton->m_PosX = m_CloseButton->m_PosY = 0;
		m_CloseButton->m_Width = Game->m_Renderer->m_Width;
		m_CloseButton->m_Height = Game->m_Renderer->m_Height;

		m_CloseButton->Display();
	}


	// display window
	RECT rect = m_ItemsArea;
	if (m_Window) {
		CBPlatform::OffsetRect(&rect, m_Window->m_PosX, m_Window->m_PosY);
		m_Window->Display();
	}

	// display items
	if (m_Window && m_Window->m_AlphaColor != 0) Game->m_Renderer->m_ForceAlphaColor = m_Window->m_AlphaColor;
	int yyy = rect.top;
	for (int j = 0; j < ItemsY; j++) {
		int xxx = rect.left;
		for (int i = 0; i < ItemsX; i++) {
			int ItemIndex = m_ScrollOffset + j * ItemsX + i;
			if (ItemIndex >= 0 && ItemIndex < AdGame->m_InventoryOwner->GetInventory()->m_TakenItems.GetSize()) {
				CAdItem *item = AdGame->m_InventoryOwner->GetInventory()->m_TakenItems[ItemIndex];
				if (item != ((CAdGame *)Game)->m_SelectedItem || !m_HideSelected) {
					item->Update();
					item->Display(xxx, yyy);
				}
			}

			xxx += (m_ItemWidth + m_Spacing);
		}
		yyy += (m_ItemHeight + m_Spacing);
	}
	if (m_Window && m_Window->m_AlphaColor != 0) Game->m_Renderer->m_ForceAlphaColor = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdInventoryBox::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing INVENTORY_BOX file '%s'", Filename);


	delete [] Buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(INVENTORY_BOX)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(WINDOW)
TOKEN_DEF(EXCLUSIVE)
TOKEN_DEF(ALWAYS_VISIBLE)
TOKEN_DEF(AREA)
TOKEN_DEF(SPACING)
TOKEN_DEF(ITEM_WIDTH)
TOKEN_DEF(ITEM_HEIGHT)
TOKEN_DEF(SCROLL_BY)
TOKEN_DEF(NAME)
TOKEN_DEF(CAPTION)
TOKEN_DEF(HIDE_SELECTED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::LoadBuffer(byte  *Buffer, bool Complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(INVENTORY_BOX)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(WINDOW)
	TOKEN_TABLE(EXCLUSIVE)
	TOKEN_TABLE(ALWAYS_VISIBLE)
	TOKEN_TABLE(AREA)
	TOKEN_TABLE(SPACING)
	TOKEN_TABLE(ITEM_WIDTH)
	TOKEN_TABLE(ITEM_HEIGHT)
	TOKEN_TABLE(SCROLL_BY)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(CAPTION)
	TOKEN_TABLE(HIDE_SELECTED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd = 2;
	CBParser parser(Game);
	bool always_visible = false;

	m_Exclusive = false;
	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_INVENTORY_BOX) {
			Game->LOG(0, "'INVENTORY_BOX' keyword expected.");
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

		case TOKEN_WINDOW:
			SAFE_DELETE(m_Window);
			m_Window = new CUIWindow(Game);
			if (!m_Window || FAILED(m_Window->LoadBuffer(params, false))) {
				SAFE_DELETE(m_Window);
				cmd = PARSERR_GENERIC;
			} else Game->RegisterObject(m_Window);
			break;

		case TOKEN_AREA:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_ItemsArea.left, &m_ItemsArea.top, &m_ItemsArea.right, &m_ItemsArea.bottom);
			break;

		case TOKEN_EXCLUSIVE:
			parser.ScanStr((char *)params, "%b", &m_Exclusive);
			break;

		case TOKEN_HIDE_SELECTED:
			parser.ScanStr((char *)params, "%b", &m_HideSelected);
			break;

		case TOKEN_ALWAYS_VISIBLE:
			parser.ScanStr((char *)params, "%b", &always_visible);
			break;

		case TOKEN_SPACING:
			parser.ScanStr((char *)params, "%d", &m_Spacing);
			break;

		case TOKEN_ITEM_WIDTH:
			parser.ScanStr((char *)params, "%d", &m_ItemWidth);
			break;

		case TOKEN_ITEM_HEIGHT:
			parser.ScanStr((char *)params, "%d", &m_ItemHeight);
			break;

		case TOKEN_SCROLL_BY:
			parser.ScanStr((char *)params, "%d", &m_ScrollBy);
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in INVENTORY_BOX definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading INVENTORY_BOX definition");
		return E_FAIL;
	}

	if (m_Exclusive) {
		SAFE_DELETE(m_CloseButton);
		m_CloseButton = new CUIButton(Game);
		if (m_CloseButton) {
			m_CloseButton->SetName("close");
			m_CloseButton->SetListener(this, m_CloseButton, 0);
			m_CloseButton->m_Parent = m_Window;
		}
	}

	m_Visible = always_visible;

	if (m_Window) {
		for (int i = 0; i < m_Window->m_Widgets.GetSize(); i++) {
			if (!m_Window->m_Widgets[i]->m_ListenerObject)
				m_Window->m_Widgets[i]->SetListener(this, m_Window->m_Widgets[i], 0);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "INVENTORY_BOX\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", m_Name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "AREA { %d, %d, %d, %d }\n", m_ItemsArea.left, m_ItemsArea.top, m_ItemsArea.right, m_ItemsArea.bottom);

	Buffer->PutTextIndent(Indent + 2, "EXCLUSIVE=%s\n", m_Exclusive ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "HIDE_SELECTED=%s\n", m_HideSelected ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "ALWAYS_VISIBLE=%s\n", m_Visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "SPACING=%d\n", m_Spacing);
	Buffer->PutTextIndent(Indent + 2, "ITEM_WIDTH=%d\n", m_ItemWidth);
	Buffer->PutTextIndent(Indent + 2, "ITEM_HEIGHT=%d\n", m_ItemHeight);
	Buffer->PutTextIndent(Indent + 2, "SCROLL_BY=%d\n", m_ScrollBy);

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
HRESULT CAdInventoryBox::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_CloseButton));
	PersistMgr->Transfer(TMEMBER(m_HideSelected));
	PersistMgr->Transfer(TMEMBER(m_ItemHeight));
	PersistMgr->Transfer(TMEMBER(m_ItemsArea));
	PersistMgr->Transfer(TMEMBER(m_ItemWidth));
	PersistMgr->Transfer(TMEMBER(m_ScrollBy));
	PersistMgr->Transfer(TMEMBER(m_ScrollOffset));
	PersistMgr->Transfer(TMEMBER(m_Spacing));
	PersistMgr->Transfer(TMEMBER(m_Visible));
	PersistMgr->Transfer(TMEMBER(m_Window));
	PersistMgr->Transfer(TMEMBER(m_Exclusive));

	return S_OK;
}

} // end of namespace WinterMute
