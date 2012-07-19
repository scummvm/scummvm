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
#include "engines/wintermute/ad/AdGame.h"
#include "engines/wintermute/ad/AdInventoryBox.h"
#include "engines/wintermute/ad/AdInventory.h"
#include "engines/wintermute/ad/AdItem.h"
#include "engines/wintermute/base/BGame.h"
#include "engines/wintermute/base/BParser.h"
#include "engines/wintermute/base/BFileManager.h"
#include "engines/wintermute/base/BViewport.h"
#include "engines/wintermute/base/BDynBuffer.h"
#include "engines/wintermute/ui/UIButton.h"
#include "engines/wintermute/ui/UIWindow.h"
#include "engines/wintermute/platform_osystem.h"
#include "common/str.h"
#include "common/rect.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CAdInventoryBox, false)

//////////////////////////////////////////////////////////////////////////
CAdInventoryBox::CAdInventoryBox(CBGame *inGame): CBObject(inGame) {
	_itemsArea.setEmpty();
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
	_gameRef->unregisterObject(_window);
	_window = NULL;

	delete _closeButton;
	_closeButton = NULL;
}


//////////////////////////////////////////////////////////////////////////
bool CAdInventoryBox::listen(CBScriptHolder *param1, uint32 param2) {
	CUIObject *obj = (CUIObject *)param1;

	switch (obj->_type) {
	case UI_BUTTON:
		if (scumm_stricmp(obj->_name, "close") == 0) {
			_visible = false;
		} else if (scumm_stricmp(obj->_name, "prev") == 0) {
			_scrollOffset -= _scrollBy;
			_scrollOffset = MAX(_scrollOffset, 0);
		} else if (scumm_stricmp(obj->_name, "next") == 0) {
			_scrollOffset += _scrollBy;
		} else return CBObject::listen(param1, param2);
		break;
	default:
		error("CAdInventoryBox::Listen - Unhandled enum");
		break;
	}

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdInventoryBox::display() {
	CAdGame *adGame = (CAdGame *)_gameRef;

	if (!_visible) return STATUS_OK;

	int itemsX, itemsY;
	itemsX = (int)floor((float)((_itemsArea.right - _itemsArea.left + _spacing) / (_itemWidth + _spacing)));
	itemsY = (int)floor((float)((_itemsArea.bottom - _itemsArea.top + _spacing) / (_itemHeight + _spacing)));

	if (_window) {
		_window->enableWidget("prev", _scrollOffset > 0);
		_window->enableWidget("next", _scrollOffset + itemsX * itemsY < adGame->_inventoryOwner->getInventory()->_takenItems.getSize());
	}


	if (_closeButton) {
		_closeButton->_posX = _closeButton->_posY = 0;
		_closeButton->_width = _gameRef->_renderer->_width;
		_closeButton->_height = _gameRef->_renderer->_height;

		_closeButton->display();
	}


	// display window
	Rect32 rect = _itemsArea;
	if (_window) {
		rect.offsetRect(_window->_posX, _window->_posY);
		_window->display();
	}

	// display items
	if (_window && _window->_alphaColor != 0) _gameRef->_renderer->_forceAlphaColor = _window->_alphaColor;
	int yyy = rect.top;
	for (int j = 0; j < itemsY; j++) {
		int xxx = rect.left;
		for (int i = 0; i < itemsX; i++) {
			int itemIndex = _scrollOffset + j * itemsX + i;
			if (itemIndex >= 0 && itemIndex < adGame->_inventoryOwner->getInventory()->_takenItems.getSize()) {
				CAdItem *item = adGame->_inventoryOwner->getInventory()->_takenItems[itemIndex];
				if (item != ((CAdGame *)_gameRef)->_selectedItem || !_hideSelected) {
					item->update();
					item->display(xxx, yyy);
				}
			}

			xxx += (_itemWidth + _spacing);
		}
		yyy += (_itemHeight + _spacing);
	}
	if (_window && _window->_alphaColor != 0) _gameRef->_renderer->_forceAlphaColor = 0;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdInventoryBox::loadFile(const char *filename) {
	byte *buffer = _gameRef->_fileManager->readWholeFile(filename);
	if (buffer == NULL) {
		_gameRef->LOG(0, "CAdInventoryBox::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	_filename = new char [strlen(filename) + 1];
	strcpy(_filename, filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) _gameRef->LOG(0, "Error parsing INVENTORY_BOX file '%s'", filename);


	delete [] buffer;

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
bool CAdInventoryBox::loadBuffer(byte *buffer, bool complete) {
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
	CBParser parser(_gameRef);
	bool always_visible = false;

	_exclusive = false;
	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_INVENTORY_BOX) {
			_gameRef->LOG(0, "'INVENTORY_BOX' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while (cmd > 0 && (cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_CAPTION:
			setCaption((char *)params);
			break;

		case TOKEN_WINDOW:
			delete _window;
			_window = new CUIWindow(_gameRef);
			if (!_window || DID_FAIL(_window->loadBuffer(params, false))) {
				delete _window;
				_window = NULL;
				cmd = PARSERR_GENERIC;
			} else _gameRef->registerObject(_window);
			break;

		case TOKEN_AREA:
			parser.scanStr((char *)params, "%d,%d,%d,%d", &_itemsArea.left, &_itemsArea.top, &_itemsArea.right, &_itemsArea.bottom);
			break;

		case TOKEN_EXCLUSIVE:
			parser.scanStr((char *)params, "%b", &_exclusive);
			break;

		case TOKEN_HIDE_SELECTED:
			parser.scanStr((char *)params, "%b", &_hideSelected);
			break;

		case TOKEN_ALWAYS_VISIBLE:
			parser.scanStr((char *)params, "%b", &always_visible);
			break;

		case TOKEN_SPACING:
			parser.scanStr((char *)params, "%d", &_spacing);
			break;

		case TOKEN_ITEM_WIDTH:
			parser.scanStr((char *)params, "%d", &_itemWidth);
			break;

		case TOKEN_ITEM_HEIGHT:
			parser.scanStr((char *)params, "%d", &_itemHeight);
			break;

		case TOKEN_SCROLL_BY:
			parser.scanStr((char *)params, "%d", &_scrollBy);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in INVENTORY_BOX definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading INVENTORY_BOX definition");
		return STATUS_FAILED;
	}

	if (_exclusive) {
		delete _closeButton;
		_closeButton = new CUIButton(_gameRef);
		if (_closeButton) {
			_closeButton->setName("close");
			_closeButton->setListener(this, _closeButton, 0);
			_closeButton->_parent = _window;
		}
	}

	_visible = always_visible;

	if (_window) {
		for (int i = 0; i < _window->_widgets.getSize(); i++) {
			if (!_window->_widgets[i]->_listenerObject)
				_window->_widgets[i]->setListener(this, _window->_widgets[i], 0);
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool CAdInventoryBox::saveAsText(CBDynBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "INVENTORY_BOX\n");
	buffer->putTextIndent(indent, "{\n");

	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", _name);
	buffer->putTextIndent(indent + 2, "CAPTION=\"%s\"\n", getCaption());

	buffer->putTextIndent(indent + 2, "AREA { %d, %d, %d, %d }\n", _itemsArea.left, _itemsArea.top, _itemsArea.right, _itemsArea.bottom);

	buffer->putTextIndent(indent + 2, "EXCLUSIVE=%s\n", _exclusive ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "HIDE_SELECTED=%s\n", _hideSelected ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "ALWAYS_VISIBLE=%s\n", _visible ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "SPACING=%d\n", _spacing);
	buffer->putTextIndent(indent + 2, "ITEM_WIDTH=%d\n", _itemWidth);
	buffer->putTextIndent(indent + 2, "ITEM_HEIGHT=%d\n", _itemHeight);
	buffer->putTextIndent(indent + 2, "SCROLL_BY=%d\n", _scrollBy);

	buffer->putTextIndent(indent + 2, "\n");

	// window
	if (_window) _window->saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent + 2, "\n");

	// editor properties
	CBBase::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool CAdInventoryBox::persist(CBPersistMgr *persistMgr) {
	CBObject::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_closeButton));
	persistMgr->transfer(TMEMBER(_hideSelected));
	persistMgr->transfer(TMEMBER(_itemHeight));
	persistMgr->transfer(TMEMBER(_itemsArea));
	persistMgr->transfer(TMEMBER(_itemWidth));
	persistMgr->transfer(TMEMBER(_scrollBy));
	persistMgr->transfer(TMEMBER(_scrollOffset));
	persistMgr->transfer(TMEMBER(_spacing));
	persistMgr->transfer(TMEMBER(_visible));
	persistMgr->transfer(TMEMBER(_window));
	persistMgr->transfer(TMEMBER(_exclusive));

	return STATUS_OK;
}

} // end of namespace WinterMute
