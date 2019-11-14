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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
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

#include "engines/wintermute/ui/ui_tiled_image.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/base_renderer.h"
#include "engines/wintermute/platform_osystem.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(UITiledImage, false)

//////////////////////////////////////////////////////////////////////////
UITiledImage::UITiledImage(BaseGame *inGame) : BaseObject(inGame) {
	_image = nullptr;

	_upLeft.setEmpty();
	_upMiddle.setEmpty();
	_upRight.setEmpty();
	_middleLeft.setEmpty();
	_middleMiddle.setEmpty();
	_middleRight.setEmpty();
	_downLeft.setEmpty();
	_downMiddle.setEmpty();
	_downRight.setEmpty();
}


//////////////////////////////////////////////////////////////////////////
UITiledImage::~UITiledImage() {
	delete _image;
	_image = nullptr;
}


//////////////////////////////////////////////////////////////////////////
bool UITiledImage::display(int x, int y, int width, int height) {
	if (!_image) {
		return STATUS_FAILED;
	}

	int tileWidth = _middleMiddle.right - _middleMiddle.left;
	int tileHeight = _middleMiddle.bottom - _middleMiddle.top;

	int nuColumns = (width - (_middleLeft.right - _middleLeft.left) - (_middleRight.right - _middleRight.left)) / tileWidth;
	int nuRows = (height - (_upMiddle.bottom - _upMiddle.top) - (_downMiddle.bottom - _downMiddle.top)) / tileHeight;

	_gameRef->_renderer->startSpriteBatch();

	// top left/right
	_image->_surface->displayTrans(x,                                                       y, _upLeft);
	_image->_surface->displayTrans(x + (_upLeft.right - _upLeft.left) + nuColumns * tileWidth, y, _upRight);

	// bottom left/right
	_image->_surface->displayTrans(x,                                                       y + (_upMiddle.bottom - _upMiddle.top) + nuRows * tileHeight, _downLeft);
	_image->_surface->displayTrans(x + (_upLeft.right - _upLeft.left) + nuColumns * tileWidth, y + (_upMiddle.bottom - _upMiddle.top) + nuRows * tileHeight, _downRight);

	// left/right
	if (nuRows > 0) {
		int yyy = y + (_upMiddle.bottom - _upMiddle.top);
		_image->_surface->displayTiled(x, yyy, _middleLeft, 1, nuRows);
		_image->_surface->displayTiled(x + (_middleLeft.right - _middleLeft.left) + nuColumns * tileWidth, yyy, _middleRight, 1, nuRows);
	}

	// top/bottom
	if (nuColumns > 0) {
		int xxx = x + (_upLeft.right - _upLeft.left);
		_image->_surface->displayTiled(xxx, y, _upMiddle, nuColumns, 1);
		_image->_surface->displayTiled(xxx, y + (_upMiddle.bottom - _upMiddle.top) + nuRows * tileHeight, _downMiddle, nuColumns, 1);
	}

	// tiles
	if (nuRows > 0 && nuColumns > 0) {
		int yyy = y + (_upMiddle.bottom - _upMiddle.top);
		int xxx = x + (_upLeft.right - _upLeft.left);
		_image->_surface->displayTiled(xxx, yyy, _middleMiddle, nuColumns, nuRows);
	}

	_gameRef->_renderer->endSpriteBatch();

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool UITiledImage::loadFile(const char *filename) {
	char *buffer = (char *)BaseFileManager::getEngineInstance()->readWholeFile(filename);
	if (buffer == nullptr) {
		_gameRef->LOG(0, "UITiledImage::LoadFile failed for file '%s'", filename);
		return STATUS_FAILED;
	}

	bool ret;

	setFilename(filename);

	if (DID_FAIL(ret = loadBuffer(buffer, true))) {
		_gameRef->LOG(0, "Error parsing TILED_IMAGE file '%s'", filename);
	}


	delete[] buffer;

	return ret;
}


TOKEN_DEF_START
TOKEN_DEF(TILED_IMAGE)
TOKEN_DEF(TEMPLATE)
TOKEN_DEF(IMAGE)
TOKEN_DEF(UP_LEFT)
TOKEN_DEF(UP_RIGHT)
TOKEN_DEF(UP_MIDDLE)
TOKEN_DEF(DOWN_LEFT)
TOKEN_DEF(DOWN_RIGHT)
TOKEN_DEF(DOWN_MIDDLE)
TOKEN_DEF(MIDDLE_LEFT)
TOKEN_DEF(MIDDLE_RIGHT)
TOKEN_DEF(MIDDLE_MIDDLE)
TOKEN_DEF(VERTICAL_TILES)
TOKEN_DEF(HORIZONTAL_TILES)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////////
bool UITiledImage::loadBuffer(char *buffer, bool complete) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(TILED_IMAGE)
	TOKEN_TABLE(TEMPLATE)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(UP_LEFT)
	TOKEN_TABLE(UP_RIGHT)
	TOKEN_TABLE(UP_MIDDLE)
	TOKEN_TABLE(DOWN_LEFT)
	TOKEN_TABLE(DOWN_RIGHT)
	TOKEN_TABLE(DOWN_MIDDLE)
	TOKEN_TABLE(MIDDLE_LEFT)
	TOKEN_TABLE(MIDDLE_RIGHT)
	TOKEN_TABLE(MIDDLE_MIDDLE)
	TOKEN_TABLE(VERTICAL_TILES)
	TOKEN_TABLE(HORIZONTAL_TILES)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;
	bool hTiles = false, vTiles = false;
	int h1 = 0, h2 = 0, h3 = 0;
	int v1 = 0, v2 = 0, v3 = 0;

	if (complete) {
		if (parser.getCommand(&buffer, commands, &params) != TOKEN_TILED_IMAGE) {
			_gameRef->LOG(0, "'TILED_IMAGE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	while ((cmd = parser.getCommand(&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (DID_FAIL(loadFile(params))) {
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new BaseSubFrame(_gameRef);
			if (!_image || DID_FAIL(_image->setSurface(params))) {
				delete _image;
				_image = nullptr;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_UP_LEFT:
			parser.scanStr(params, "%d,%d,%d,%d", &_upLeft.left, &_upLeft.top, &_upLeft.right, &_upLeft.bottom);
			break;

		case TOKEN_UP_RIGHT:
			parser.scanStr(params, "%d,%d,%d,%d", &_upRight.left, &_upRight.top, &_upRight.right, &_upRight.bottom);
			break;

		case TOKEN_UP_MIDDLE:
			parser.scanStr(params, "%d,%d,%d,%d", &_upMiddle.left, &_upMiddle.top, &_upMiddle.right, &_upMiddle.bottom);
			break;

		case TOKEN_DOWN_LEFT:
			parser.scanStr(params, "%d,%d,%d,%d", &_downLeft.left, &_downLeft.top, &_downLeft.right, &_downLeft.bottom);
			break;

		case TOKEN_DOWN_RIGHT:
			parser.scanStr(params, "%d,%d,%d,%d", &_downRight.left, &_downRight.top, &_downRight.right, &_downRight.bottom);
			break;

		case TOKEN_DOWN_MIDDLE:
			parser.scanStr(params, "%d,%d,%d,%d", &_downMiddle.left, &_downMiddle.top, &_downMiddle.right, &_downMiddle.bottom);
			break;

		case TOKEN_MIDDLE_LEFT:
			parser.scanStr(params, "%d,%d,%d,%d", &_middleLeft.left, &_middleLeft.top, &_middleLeft.right, &_middleLeft.bottom);
			break;

		case TOKEN_MIDDLE_RIGHT:
			parser.scanStr(params, "%d,%d,%d,%d", &_middleRight.left, &_middleRight.top, &_middleRight.right, &_middleRight.bottom);
			break;

		case TOKEN_MIDDLE_MIDDLE:
			parser.scanStr(params, "%d,%d,%d,%d", &_middleMiddle.left, &_middleMiddle.top, &_middleMiddle.right, &_middleMiddle.bottom);
			break;

		case TOKEN_HORIZONTAL_TILES:
			parser.scanStr(params, "%d,%d,%d", &h1, &h2, &h3);
			hTiles = true;
			break;

		case TOKEN_VERTICAL_TILES:
			parser.scanStr(params, "%d,%d,%d", &v1, &v2, &v3);
			vTiles = true;
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;

		default:
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in TILED_IMAGE definition");
		return STATUS_FAILED;
	}
	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading TILED_IMAGE definition");
		return STATUS_FAILED;
	}

	if (vTiles && hTiles) {
		// up row
		_upLeft.setRect(0, 0, h1, v1);
		_upMiddle.setRect(h1, 0, h1 + h2, v1);
		_upRight.setRect(h1 + h2, 0, h1 + h2 + h3, v1);

		// middle row
		_middleLeft.setRect(0, v1, h1, v1 + v2);
		_middleMiddle.setRect(h1, v1, h1 + h2, v1 + v2);
		_middleRight.setRect(h1 + h2, v1, h1 + h2 + h3, v1 + v2);

		// down row
		_downLeft.setRect(0, v1 + v2, h1, v1 + v2 + v3);
		_downMiddle.setRect(h1, v1 + v2, h1 + h2, v1 + v2 + v3);
		_downRight.setRect(h1 + h2, v1 + v2, h1 + h2 + h3, v1 + v2 + v3);
	}

	// default
	if (_image && _image->_surface) {
		int width = _image->_surface->getWidth() / 3;
		int height = _image->_surface->getHeight() / 3;

		if (_upLeft.isRectEmpty()) {
			_upLeft.setRect(0, 0, width, height);
		}
		if (_upMiddle.isRectEmpty()) {
			_upMiddle.setRect(width, 0, 2 * width, height);
		}
		if (_upRight.isRectEmpty()) {
			_upRight.setRect(2 * width, 0, 3 * width, height);
		}

		if (_middleLeft.isRectEmpty()) {
			_middleLeft.setRect(0, height, width, 2 * height);
		}
		if (_middleMiddle.isRectEmpty()) {
			_middleMiddle.setRect(width, height, 2 * width, 2 * height);
		}
		if (_middleRight.isRectEmpty()) {
			_middleRight.setRect(2 * width, height, 3 * width, 2 * height);
		}

		if (_downLeft.isRectEmpty()) {
			_downLeft.setRect(0, 2 * height, width, 3 * height);
		}
		if (_downMiddle.isRectEmpty()) {
			_downMiddle.setRect(width, 2 * height, 2 * width, 3 * height);
		}
		if (_downRight.isRectEmpty()) {
			_downRight.setRect(2 * width, 2 * height, 3 * width, 3 * height);
		}
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool UITiledImage::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "TILED_IMAGE\n");
	buffer->putTextIndent(indent, "{\n");

	if (_image && _image->getSurfaceFilename()) {
		buffer->putTextIndent(indent + 2, "IMAGE=\"%s\"\n", _image->getSurfaceFilename());
	}

	int h1, h2, h3;
	int v1, v2, v3;

	h1 = _upLeft.right;
	h2 = _upMiddle.right - _upMiddle.left;
	h3 = _upRight.right - _upRight.left;

	v1 = _upLeft.bottom;
	v2 = _middleLeft.bottom - _middleLeft.top;
	v3 = _downLeft.bottom - _downLeft.top;


	buffer->putTextIndent(indent + 2, "VERTICAL_TILES { %d, %d, %d }\n", v1, v2, v3);
	buffer->putTextIndent(indent + 2, "HORIZONTAL_TILES { %d, %d, %d }\n", h1, h2, h3);

	// editor properties
	BaseClass::saveAsText(buffer, indent + 2);

	buffer->putTextIndent(indent, "}\n");
	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
void UITiledImage::correctSize(int32 *width, int32 *height) {
	int tileWidth = _middleMiddle.right - _middleMiddle.left;
	int tileHeight = _middleMiddle.bottom - _middleMiddle.top;

	int nuColumns = (*width - (_middleLeft.right - _middleLeft.left) - (_middleRight.right - _middleRight.left)) / tileWidth;
	int nuRows = (*height - (_upMiddle.bottom - _upMiddle.top) - (_downMiddle.bottom - _downMiddle.top)) / tileHeight;

	*width  = (_middleLeft.right - _middleLeft.left) + (_middleRight.right - _middleRight.left) + nuColumns * tileWidth;
	*height = (_upMiddle.bottom - _upMiddle.top) + (_downMiddle.bottom - _downMiddle.top) + nuRows * tileHeight;
}


//////////////////////////////////////////////////////////////////////////
bool UITiledImage::persist(BasePersistenceManager *persistMgr) {
	BaseObject::persist(persistMgr);

	persistMgr->transferRect32(TMEMBER(_downLeft));
	persistMgr->transferRect32(TMEMBER(_downMiddle));
	persistMgr->transferRect32(TMEMBER(_downRight));
	persistMgr->transferPtr(TMEMBER_PTR(_image));
	persistMgr->transferRect32(TMEMBER(_middleLeft));
	persistMgr->transferRect32(TMEMBER(_middleMiddle));
	persistMgr->transferRect32(TMEMBER(_middleRight));
	persistMgr->transferRect32(TMEMBER(_upLeft));
	persistMgr->transferRect32(TMEMBER(_upMiddle));
	persistMgr->transferRect32(TMEMBER(_upRight));

	return STATUS_OK;
}

} // End of namespace Wintermute
