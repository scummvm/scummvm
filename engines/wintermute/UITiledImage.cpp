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
#include "engines/wintermute/UITiledImage.h"
#include "engines/wintermute/BSurface.h"
#include "engines/wintermute/BDynBuffer.h"
#include "engines/wintermute/BParser.h"
#include "engines/wintermute/BGame.h"
#include "engines/wintermute/BSubFrame.h"
#include "engines/wintermute/BFileManager.h"
#include "engines/wintermute/PlatformSDL.h"

namespace WinterMute {

IMPLEMENT_PERSISTENT(CUITiledImage, false)

//////////////////////////////////////////////////////////////////////////
CUITiledImage::CUITiledImage(CBGame *inGame): CBObject(inGame) {
	_image = NULL;

	CBPlatform::SetRectEmpty(&_upLeft);
	CBPlatform::SetRectEmpty(&_upMiddle);
	CBPlatform::SetRectEmpty(&_upRight);
	CBPlatform::SetRectEmpty(&_middleLeft);
	CBPlatform::SetRectEmpty(&_middleMiddle);
	CBPlatform::SetRectEmpty(&_middleRight);
	CBPlatform::SetRectEmpty(&_downLeft);
	CBPlatform::SetRectEmpty(&_downMiddle);
	CBPlatform::SetRectEmpty(&_downRight);
}


//////////////////////////////////////////////////////////////////////////
CUITiledImage::~CUITiledImage() {
	delete _image;
	_image = NULL;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::Display(int X, int Y, int Width, int Height) {
	if (!_image) return E_FAIL;

	int tile_width = _middleMiddle.right - _middleMiddle.left;
	int tile_height = _middleMiddle.bottom - _middleMiddle.top;

	int nu_columns = (Width - (_middleLeft.right - _middleLeft.left) - (_middleRight.right - _middleRight.left)) / tile_width;
	int nu_rows = (Height - (_upMiddle.bottom - _upMiddle.top) - (_downMiddle.bottom - _downMiddle.top)) / tile_height;

	int col, row;

	Game->_renderer->StartSpriteBatch();

	// top left/right
	_image->_surface->DisplayTrans(X,                                                       Y, _upLeft);
	_image->_surface->DisplayTrans(X + (_upLeft.right - _upLeft.left) + nu_columns * tile_width, Y, _upRight);

	// bottom left/right
	_image->_surface->DisplayTrans(X,                                                       Y + (_upMiddle.bottom - _upMiddle.top) + nu_rows * tile_height, _downLeft);
	_image->_surface->DisplayTrans(X + (_upLeft.right - _upLeft.left) + nu_columns * tile_width, Y + (_upMiddle.bottom - _upMiddle.top) + nu_rows * tile_height, _downRight);

	// left/right
	int yyy = Y + (_upMiddle.bottom - _upMiddle.top);
	for (row = 0; row < nu_rows; row++) {
		_image->_surface->DisplayTrans(X,                                                       yyy, _middleLeft);
		_image->_surface->DisplayTrans(X + (_middleLeft.right - _middleLeft.left) + nu_columns * tile_width, yyy, _middleRight);
		yyy += tile_width;
	}

	// top/bottom
	int xxx = X + (_upLeft.right - _upLeft.left);
	for (col = 0; col < nu_columns; col++) {
		_image->_surface->DisplayTrans(xxx, Y, _upMiddle);
		_image->_surface->DisplayTrans(xxx, Y + (_upMiddle.bottom - _upMiddle.top) + nu_rows * tile_height, _downMiddle);
		xxx += tile_width;
	}

	// tiles
	yyy = Y + (_upMiddle.bottom - _upMiddle.top);
	for (row = 0; row < nu_rows; row++) {
		xxx = X + (_upLeft.right - _upLeft.left);
		for (col = 0; col < nu_columns; col++) {
			_image->_surface->DisplayTrans(xxx, yyy, _middleMiddle);
			xxx += tile_width;
		}
		yyy += tile_width;
	}

	Game->_renderer->EndSpriteBatch();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::LoadFile(const char *Filename) {
	byte *Buffer = Game->_fileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUITiledImage::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	_filename = new char [strlen(Filename) + 1];
	strcpy(_filename, Filename);

	if (FAILED(ret = LoadBuffer(Buffer, true))) Game->LOG(0, "Error parsing TILED_IMAGE file '%s'", Filename);


	delete [] Buffer;

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
HRESULT CUITiledImage::LoadBuffer(byte  *Buffer, bool Complete) {
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

	byte *params;
	int cmd;
	CBParser parser(Game);
	bool HTiles = false, VTiles = false;
	int H1 = 0, H2 = 0, H3 = 0;
	int V1 = 0, V2 = 0, V3 = 0;

	if (Complete) {
		if (parser.GetCommand((char **)&Buffer, commands, (char **)&params) != TOKEN_TILED_IMAGE) {
			Game->LOG(0, "'TILED_IMAGE' keyword expected.");
			return E_FAIL;
		}
		Buffer = params;
	}

	while ((cmd = parser.GetCommand((char **)&Buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_TEMPLATE:
			if (FAILED(LoadFile((char *)params))) cmd = PARSERR_GENERIC;
			break;

		case TOKEN_IMAGE:
			delete _image;
			_image = new CBSubFrame(Game);
			if (!_image || FAILED(_image->SetSurface((char *)params))) {
				delete _image;
				_image = NULL;
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_UP_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_upLeft.left, &_upLeft.top, &_upLeft.right, &_upLeft.bottom);
			break;

		case TOKEN_UP_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_upRight.left, &_upRight.top, &_upRight.right, &_upRight.bottom);
			break;

		case TOKEN_UP_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_upMiddle.left, &_upMiddle.top, &_upMiddle.right, &_upMiddle.bottom);
			break;

		case TOKEN_DOWN_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_downLeft.left, &_downLeft.top, &_downLeft.right, &_downLeft.bottom);
			break;

		case TOKEN_DOWN_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_downRight.left, &_downRight.top, &_downRight.right, &_downRight.bottom);
			break;

		case TOKEN_DOWN_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_downMiddle.left, &_downMiddle.top, &_downMiddle.right, &_downMiddle.bottom);
			break;

		case TOKEN_MIDDLE_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_middleLeft.left, &_middleLeft.top, &_middleLeft.right, &_middleLeft.bottom);
			break;

		case TOKEN_MIDDLE_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_middleRight.left, &_middleRight.top, &_middleRight.right, &_middleRight.bottom);
			break;

		case TOKEN_MIDDLE_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &_middleMiddle.left, &_middleMiddle.top, &_middleMiddle.right, &_middleMiddle.bottom);
			break;

		case TOKEN_HORIZONTAL_TILES:
			parser.ScanStr((char *)params, "%d,%d,%d", &H1, &H2, &H3);
			HTiles = true;
			break;

		case TOKEN_VERTICAL_TILES:
			parser.ScanStr((char *)params, "%d,%d,%d", &V1, &V2, &V3);
			VTiles = true;
			break;

		case TOKEN_EDITOR_PROPERTY:
			ParseEditorProperty(params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		Game->LOG(0, "Syntax error in TILED_IMAGE definition");
		return E_FAIL;
	}
	if (cmd == PARSERR_GENERIC) {
		Game->LOG(0, "Error loading TILED_IMAGE definition");
		return E_FAIL;
	}

	if (VTiles && HTiles) {
		// up row
		CBPlatform::SetRect(&_upLeft,   0,     0, H1,       V1);
		CBPlatform::SetRect(&_upMiddle, H1,    0, H1 + H2,    V1);
		CBPlatform::SetRect(&_upRight,  H1 + H2, 0, H1 + H2 + H3, V1);

		// middle row
		CBPlatform::SetRect(&_middleLeft,   0,     V1, H1,       V1 + V2);
		CBPlatform::SetRect(&_middleMiddle, H1,    V1, H1 + H2,    V1 + V2);
		CBPlatform::SetRect(&_middleRight,  H1 + H2, V1, H1 + H2 + H3, V1 + V2);

		// down row
		CBPlatform::SetRect(&_downLeft,   0,     V1 + V2, H1,       V1 + V2 + V3);
		CBPlatform::SetRect(&_downMiddle, H1,    V1 + V2, H1 + H2,    V1 + V2 + V3);
		CBPlatform::SetRect(&_downRight,  H1 + H2, V1 + V2, H1 + H2 + H3, V1 + V2 + V3);
	}

	// default
	if (_image && _image->_surface) {
		int Width = _image->_surface->GetWidth() / 3;
		int Height = _image->_surface->GetHeight() / 3;

		if (CBPlatform::IsRectEmpty(&_upLeft))   CBPlatform::SetRect(&_upLeft,   0,       0, Width,   Height);
		if (CBPlatform::IsRectEmpty(&_upMiddle)) CBPlatform::SetRect(&_upMiddle, Width,   0, 2 * Width, Height);
		if (CBPlatform::IsRectEmpty(&_upRight))  CBPlatform::SetRect(&_upRight,  2 * Width, 0, 3 * Width, Height);

		if (CBPlatform::IsRectEmpty(&_middleLeft))   CBPlatform::SetRect(&_middleLeft,   0,       Height, Width,   2 * Height);
		if (CBPlatform::IsRectEmpty(&_middleMiddle)) CBPlatform::SetRect(&_middleMiddle, Width,   Height, 2 * Width, 2 * Height);
		if (CBPlatform::IsRectEmpty(&_middleRight))  CBPlatform::SetRect(&_middleRight,  2 * Width, Height, 3 * Width, 2 * Height);

		if (CBPlatform::IsRectEmpty(&_downLeft))   CBPlatform::SetRect(&_downLeft,   0,       2 * Height, Width,   3 * Height);
		if (CBPlatform::IsRectEmpty(&_downMiddle)) CBPlatform::SetRect(&_downMiddle, Width,   2 * Height, 2 * Width, 3 * Height);
		if (CBPlatform::IsRectEmpty(&_downRight))  CBPlatform::SetRect(&_downRight,  2 * Width, 2 * Height, 3 * Width, 3 * Height);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "TILED_IMAGE\n");
	Buffer->PutTextIndent(Indent, "{\n");

	if (_image && _image->_surfaceFilename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", _image->_surfaceFilename);

	int H1, H2, H3;
	int V1, V2, V3;

	H1 = _upLeft.right;
	H2 = _upMiddle.right - _upMiddle.left;
	H3 = _upRight.right - _upRight.left;

	V1 = _upLeft.bottom;
	V2 = _middleLeft.bottom - _middleLeft.top;
	V3 = _downLeft.bottom - _downLeft.top;


	Buffer->PutTextIndent(Indent + 2, "VERTICAL_TILES { %d, %d, %d }\n", V1, V2, V3);
	Buffer->PutTextIndent(Indent + 2, "HORIZONTAL_TILES { %d, %d, %d }\n", H1, H2, H3);

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CUITiledImage::CorrectSize(int *Width, int *Height) {
	int tile_width = _middleMiddle.right - _middleMiddle.left;
	int tile_height = _middleMiddle.bottom - _middleMiddle.top;

	int nu_columns = (*Width - (_middleLeft.right - _middleLeft.left) - (_middleRight.right - _middleRight.left)) / tile_width;
	int nu_rows = (*Height - (_upMiddle.bottom - _upMiddle.top) - (_downMiddle.bottom - _downMiddle.top)) / tile_height;

	*Width  = (_middleLeft.right - _middleLeft.left) + (_middleRight.right - _middleRight.left) + nu_columns * tile_width;
	*Height = (_upMiddle.bottom - _upMiddle.top) + (_downMiddle.bottom - _downMiddle.top) + nu_rows * tile_height;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(_downLeft));
	PersistMgr->Transfer(TMEMBER(_downMiddle));
	PersistMgr->Transfer(TMEMBER(_downRight));
	PersistMgr->Transfer(TMEMBER(_image));
	PersistMgr->Transfer(TMEMBER(_middleLeft));
	PersistMgr->Transfer(TMEMBER(_middleMiddle));
	PersistMgr->Transfer(TMEMBER(_middleRight));
	PersistMgr->Transfer(TMEMBER(_upLeft));
	PersistMgr->Transfer(TMEMBER(_upMiddle));
	PersistMgr->Transfer(TMEMBER(_upRight));

	return S_OK;
}

} // end of namespace WinterMute
