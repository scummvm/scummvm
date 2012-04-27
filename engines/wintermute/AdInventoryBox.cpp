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
	CBPlatform::SetRectEmpty(&_itemsArea);
	_scrollOffset = 0;
	_spacing = 0;
	_itemWidth = _itemHeight = 50;
	_scrollBy = 1;

	_window = NULL;
	_closeButton = NULL;

	_hideSelected = false;

	_visible = false;
	_exclusive = false;
}


//////////////////////////////////////////////////////////////////////////
CAdInventoryBox::~CAdInventoryBox() {
	Game->UnregisterObject(_window);
	_window = NULL;

	SAFE_DELETE(_closeButton);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::Listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "close") == 0) {
			_visible = false;
		} else if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset -= _scrollBy;
			_scrollOffset = std::max(_scrollOffset, 0);
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset += _scrollBy;
		} else return CBObject::Listen(param1, param2);
		break;
	}

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::Display() {
	CAdGame *AdGame = (CAdGame *)Game;

	if (!_visible) return S_OK;

	int ItemsX, ItemsY;
	ItemsX = floor((float)((_itemsArea.right - _itemsArea.left + _spacing) / (_itemWidth + _spacing)));
	ItemsY = floor((float)((_itemsArea.bottom - _itemsArea.top + _spacing) / (_itemHeight + _spacing)));

	if (_window) {
		_window->EnableWidget("prev", _scrollOffset > 0);
		_window->EnableWidget("next", _scrollOffset + ItemsX * ItemsY < AdGame->_inventoryOwner->GetInventory()->_takenItems.GetSize());
	}


	if (_closeButton) {
		_closeButton->_posX = _closeButton->_posY = 0;
		_closeButton->_width = Game->_renderer->_width;
		_closeButton->_height = Game->_renderer->_height;

		_closeButton->Display();
	}


	// display window
	RECT rect = _itemsArea;
	if (_window) {
		CBPlatform::OffsetRect(&rect, _window->_posX, _window->_posY);
		_window->Display();
	}

	// display items
	if (_window && _window->_alphaColor != 0) Game->_renderer->_forceAlphaColor = _window->_alphaColor;
	int yyy = rect.top;
	for (int j = 0; j < ItemsY; j++) {
		int xxx = rect.left;
		for (int i = 0; i < ItemsX; i++) {
			int ItemIndex = _scrollOffset + j * ItemsX + i;
			if (ItemIndex >= 0 && ItemIndex < AdGame->_inventoryOwner->GetInventory()->_takenItems.GetSize()) {
				CAdItem *item = AdGame->_inventoryOwner->GetInventory()->_takenItems[ItemIndex];
				if (item != ((CAdGame *)Game)->_selectedItem || !_hideSelected) {
					item->Update();
					item->Display(xxx, yyy);
				}
			}

			xxx += (_itemWidth + _spacing);
		}
		yyy += (_itemHeight + _spacing);
	}
	if (_window && _window->_alphaColor != 0) Game->_renderer->_forceAlphaColor = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::LoadFile(char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CAdInventoryBox::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

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

	_exclusive = false;
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
			SAFE_DELETE(_window);
			_window = new CUIWindow(Game);
			if (!_window || FAILED(_window->LoadBuffer(params, false))) {
				SAFE_DELETE(_window);
				cmd = PARSERR_GENERIC;
			} else Game->RegisterObject(_window);
			break;

		case TOKEN_AREA:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_itemsArea.left, &_itemsArea.top, &_itemsArea.right, &_itemsArea.bottom);
			break;

		case TOKEN_EXCLUSIVE:
			parser.ScanStr((char *)params, "%b", &_exclusive);
			break;

		case TOKEN_HIDE_SELECTED:
			parser.ScanStr((char *)params, "%b", &_hideSelected);
			break;

		case TOKEN_ALWAYS_VISIBLE:
			parser.ScanStr((char *)params, "%b", &always_visible);
			break;

		case TOKEN_SPACING:
			parser.ScanStr((char *)params, "%d", &_spacing);
			break;

		case TOKEN_ITEM_WIDTH:
			parser.ScanStr((char *)params, "%d", &_itemWidth);
			break;

		case TOKEN_ITEM_HEIGHT:
			parser.ScanStr((char *)params, "%d", &_itemHeight);
			break;

		case TOKEN_SCROLL_BY:
			parser.ScanStr((char *)params, "%d", &_scrollBy);
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

	if (_exclusive) {
		SAFE_DELETE(_closeButton);
		_closeButton = new CUIButton(Game);
		if (_closeButton) {
			_closeButton->SetName("close");
			_closeButton->SetListener(this, _closeButton, 0);
			_closeButton->_parent = _window;
		}
	}

	_visible = always_visible;

	if (_window) {
		for (int i = 0; i < _window->_widgets.GetSize(); i++) {
			if (!_window->_widgets[i]->_listenerObject)
				_window->_widgets[i]->SetListener(this, _window->_widgets[i], 0);
		}
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "INVENTORY_BOX\n");
	Buffer->PutTextIndent(Indent, "{\n");

	Buffer->PutTextIndent(Indent + 2, "NAME=\"%s\"\n", _name);
	Buffer->PutTextIndent(Indent + 2, "CAPTION=\"%s\"\n", GetCaption());

	Buffer->PutTextIndent(Indent + 2, "AREA { %d, %d, %d, %d }\n", _itemsArea.left, _itemsArea.top, _itemsArea.right, _itemsArea.bottom);

	Buffer->PutTextIndent(Indent + 2, "EXCLUSIVE=%s\n", _exclusive ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "HIDE_SELECTED=%s\n", _hideSelected ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "ALWAYS_VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	Buffer->PutTextIndent(Indent + 2, "SPACING=%d\n", _spacing);
	Buffer->PutTextIndent(Indent + 2, "ITEM_WIDTH=%d\n", _itemWidth);
	Buffer->PutTextIndent(Indent + 2, "ITEM_HEIGHT=%d\n", _itemHeight);
	Buffer->PutTextIndent(Indent + 2, "SCROLL_BY=%d\n", _scrollBy);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// window
	if (_window) _window->SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent + 2, "\n");

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CAdInventoryBox::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_closeButton));
	PersistMgr->Transfer(TMEMBER(_hideSelected));
	PersistMgr->Transfer(TMEMBER(_itemHeight));
	PersistMgr->Transfer(TMEMBER(_itemsArea));
	PersistMgr->Transfer(TMEMBER(_itemWidth));
	PersistMgr->Transfer(TMEMBER(_scrollBy));
	PersistMgr->Transfer(TMEMBER(_scrollOffset));
	PersistMgr->Transfer(TMEMBER(_spacing));
	PersistMgr->Transfer(TMEMBER(_visible));
	PersistMgr->Transfer(TMEMBER(_window));
	PersistMgr->Transfer(TMEMBER(_exclusive));

	return S_OK;
}

} // end of namespace WinterMute
