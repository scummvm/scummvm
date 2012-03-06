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
	m_Image = NULL;

	CBPlatform::SetRectEmpty(&m_UpLeft);
	CBPlatform::SetRectEmpty(&m_UpMiddle);
	CBPlatform::SetRectEmpty(&m_UpRight);
	CBPlatform::SetRectEmpty(&m_MiddleLeft);
	CBPlatform::SetRectEmpty(&m_MiddleMiddle);
	CBPlatform::SetRectEmpty(&m_MiddleRight);
	CBPlatform::SetRectEmpty(&m_DownLeft);
	CBPlatform::SetRectEmpty(&m_DownMiddle);
	CBPlatform::SetRectEmpty(&m_DownRight);
}


//////////////////////////////////////////////////////////////////////////
CUITiledImage::~CUITiledImage() {
	SAFE_DELETE(m_Image);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::Display(int X, int Y, int Width, int Height) {
	if (!m_Image) return E_FAIL;

	int tile_width = m_MiddleMiddle.right - m_MiddleMiddle.left;
	int tile_height = m_MiddleMiddle.bottom - m_MiddleMiddle.top;

	int num_columns = (Width - (m_MiddleLeft.right - m_MiddleLeft.left) - (m_MiddleRight.right - m_MiddleRight.left)) / tile_width;
	int num_rows = (Height - (m_UpMiddle.bottom - m_UpMiddle.top) - (m_DownMiddle.bottom - m_DownMiddle.top)) / tile_height;

	int col, row;

	Game->m_Renderer->StartSpriteBatch();

	// top left/right
	m_Image->m_Surface->DisplayTrans(X,                                                       Y, m_UpLeft);
	m_Image->m_Surface->DisplayTrans(X + (m_UpLeft.right - m_UpLeft.left) + num_columns * tile_width, Y, m_UpRight);

	// bottom left/right
	m_Image->m_Surface->DisplayTrans(X,                                                       Y + (m_UpMiddle.bottom - m_UpMiddle.top) + num_rows * tile_height, m_DownLeft);
	m_Image->m_Surface->DisplayTrans(X + (m_UpLeft.right - m_UpLeft.left) + num_columns * tile_width, Y + (m_UpMiddle.bottom - m_UpMiddle.top) + num_rows * tile_height, m_DownRight);

	// left/right
	int yyy = Y + (m_UpMiddle.bottom - m_UpMiddle.top);
	for (row = 0; row < num_rows; row++) {
		m_Image->m_Surface->DisplayTrans(X,                                                       yyy, m_MiddleLeft);
		m_Image->m_Surface->DisplayTrans(X + (m_MiddleLeft.right - m_MiddleLeft.left) + num_columns * tile_width, yyy, m_MiddleRight);
		yyy += tile_width;
	}

	// top/bottom
	int xxx = X + (m_UpLeft.right - m_UpLeft.left);
	for (col = 0; col < num_columns; col++) {
		m_Image->m_Surface->DisplayTrans(xxx, Y, m_UpMiddle);
		m_Image->m_Surface->DisplayTrans(xxx, Y + (m_UpMiddle.bottom - m_UpMiddle.top) + num_rows * tile_height, m_DownMiddle);
		xxx += tile_width;
	}

	// tiles
	yyy = Y + (m_UpMiddle.bottom - m_UpMiddle.top);
	for (row = 0; row < num_rows; row++) {
		xxx = X + (m_UpLeft.right - m_UpLeft.left);
		for (col = 0; col < num_columns; col++) {
			m_Image->m_Surface->DisplayTrans(xxx, yyy, m_MiddleMiddle);
			xxx += tile_width;
		}
		yyy += tile_width;
	}

	Game->m_Renderer->EndSpriteBatch();

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::LoadFile(char *Filename) {
	byte *Buffer = Game->m_FileManager->ReadWholeFile(Filename);
	if (Buffer == NULL) {
		Game->LOG(0, "CUITiledImage::LoadFile failed for file '%s'", Filename);
		return E_FAIL;
	}

	HRESULT ret;

	m_Filename = new char [strlen(Filename) + 1];
	strcpy(m_Filename, Filename);

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
			SAFE_DELETE(m_Image);
			m_Image = new CBSubFrame(Game);
			if (!m_Image || FAILED(m_Image->SetSurface((char *)params))) {
				SAFE_DELETE(m_Image);
				cmd = PARSERR_GENERIC;
			}
			break;

		case TOKEN_UP_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_UpLeft.left, &m_UpLeft.top, &m_UpLeft.right, &m_UpLeft.bottom);
			break;

		case TOKEN_UP_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_UpRight.left, &m_UpRight.top, &m_UpRight.right, &m_UpRight.bottom);
			break;

		case TOKEN_UP_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_UpMiddle.left, &m_UpMiddle.top, &m_UpMiddle.right, &m_UpMiddle.bottom);
			break;

		case TOKEN_DOWN_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_DownLeft.left, &m_DownLeft.top, &m_DownLeft.right, &m_DownLeft.bottom);
			break;

		case TOKEN_DOWN_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_DownRight.left, &m_DownRight.top, &m_DownRight.right, &m_DownRight.bottom);
			break;

		case TOKEN_DOWN_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_DownMiddle.left, &m_DownMiddle.top, &m_DownMiddle.right, &m_DownMiddle.bottom);
			break;

		case TOKEN_MIDDLE_LEFT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_MiddleLeft.left, &m_MiddleLeft.top, &m_MiddleLeft.right, &m_MiddleLeft.bottom);
			break;

		case TOKEN_MIDDLE_RIGHT:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_MiddleRight.left, &m_MiddleRight.top, &m_MiddleRight.right, &m_MiddleRight.bottom);
			break;

		case TOKEN_MIDDLE_MIDDLE:
			parser.ScanStr((char *)params, "%d,%d,%d,%d", &m_MiddleMiddle.left, &m_MiddleMiddle.top, &m_MiddleMiddle.right, &m_MiddleMiddle.bottom);
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
		CBPlatform::SetRect(&m_UpLeft,   0,     0, H1,       V1);
		CBPlatform::SetRect(&m_UpMiddle, H1,    0, H1 + H2,    V1);
		CBPlatform::SetRect(&m_UpRight,  H1 + H2, 0, H1 + H2 + H3, V1);

		// middle row
		CBPlatform::SetRect(&m_MiddleLeft,   0,     V1, H1,       V1 + V2);
		CBPlatform::SetRect(&m_MiddleMiddle, H1,    V1, H1 + H2,    V1 + V2);
		CBPlatform::SetRect(&m_MiddleRight,  H1 + H2, V1, H1 + H2 + H3, V1 + V2);

		// down row
		CBPlatform::SetRect(&m_DownLeft,   0,     V1 + V2, H1,       V1 + V2 + V3);
		CBPlatform::SetRect(&m_DownMiddle, H1,    V1 + V2, H1 + H2,    V1 + V2 + V3);
		CBPlatform::SetRect(&m_DownRight,  H1 + H2, V1 + V2, H1 + H2 + H3, V1 + V2 + V3);
	}

	// default
	if (m_Image && m_Image->m_Surface) {
		int Width = m_Image->m_Surface->GetWidth() / 3;
		int Height = m_Image->m_Surface->GetHeight() / 3;

		if (CBPlatform::IsRectEmpty(&m_UpLeft))   CBPlatform::SetRect(&m_UpLeft,   0,       0, Width,   Height);
		if (CBPlatform::IsRectEmpty(&m_UpMiddle)) CBPlatform::SetRect(&m_UpMiddle, Width,   0, 2 * Width, Height);
		if (CBPlatform::IsRectEmpty(&m_UpRight))  CBPlatform::SetRect(&m_UpRight,  2 * Width, 0, 3 * Width, Height);

		if (CBPlatform::IsRectEmpty(&m_MiddleLeft))   CBPlatform::SetRect(&m_MiddleLeft,   0,       Height, Width,   2 * Height);
		if (CBPlatform::IsRectEmpty(&m_MiddleMiddle)) CBPlatform::SetRect(&m_MiddleMiddle, Width,   Height, 2 * Width, 2 * Height);
		if (CBPlatform::IsRectEmpty(&m_MiddleRight))  CBPlatform::SetRect(&m_MiddleRight,  2 * Width, Height, 3 * Width, 2 * Height);

		if (CBPlatform::IsRectEmpty(&m_DownLeft))   CBPlatform::SetRect(&m_DownLeft,   0,       2 * Height, Width,   3 * Height);
		if (CBPlatform::IsRectEmpty(&m_DownMiddle)) CBPlatform::SetRect(&m_DownMiddle, Width,   2 * Height, 2 * Width, 3 * Height);
		if (CBPlatform::IsRectEmpty(&m_DownRight))  CBPlatform::SetRect(&m_DownRight,  2 * Width, 2 * Height, 3 * Width, 3 * Height);
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::SaveAsText(CBDynBuffer *Buffer, int Indent) {
	Buffer->PutTextIndent(Indent, "TILED_IMAGE\n");
	Buffer->PutTextIndent(Indent, "{\n");

	if (m_Image && m_Image->m_SurfaceFilename)
		Buffer->PutTextIndent(Indent + 2, "IMAGE=\"%s\"\n", m_Image->m_SurfaceFilename);

	int H1, H2, H3;
	int V1, V2, V3;

	H1 = m_UpLeft.right;
	H2 = m_UpMiddle.right - m_UpMiddle.left;
	H3 = m_UpRight.right - m_UpRight.left;

	V1 = m_UpLeft.bottom;
	V2 = m_MiddleLeft.bottom - m_MiddleLeft.top;
	V3 = m_DownLeft.bottom - m_DownLeft.top;


	Buffer->PutTextIndent(Indent + 2, "VERTICAL_TILES { %d, %d, %d }\n", V1, V2, V3);
	Buffer->PutTextIndent(Indent + 2, "HORIZONTAL_TILES { %d, %d, %d }\n", H1, H2, H3);

	// editor properties
	CBBase::SaveAsText(Buffer, Indent + 2);

	Buffer->PutTextIndent(Indent, "}\n");
	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
void CUITiledImage::CorrectSize(int *Width, int *Height) {
	int tile_width = m_MiddleMiddle.right - m_MiddleMiddle.left;
	int tile_height = m_MiddleMiddle.bottom - m_MiddleMiddle.top;

	int num_columns = (*Width - (m_MiddleLeft.right - m_MiddleLeft.left) - (m_MiddleRight.right - m_MiddleRight.left)) / tile_width;
	int num_rows = (*Height - (m_UpMiddle.bottom - m_UpMiddle.top) - (m_DownMiddle.bottom - m_DownMiddle.top)) / tile_height;

	*Width  = (m_MiddleLeft.right - m_MiddleLeft.left) + (m_MiddleRight.right - m_MiddleRight.left) + num_columns * tile_width;
	*Height = (m_UpMiddle.bottom - m_UpMiddle.top) + (m_DownMiddle.bottom - m_DownMiddle.top) + num_rows * tile_height;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CUITiledImage::Persist(CBPersistMgr *PersistMgr) {
	CBObject::Persist(PersistMgr);

	PersistMgr->Transfer(TMEMBER(m_DownLeft));
	PersistMgr->Transfer(TMEMBER(m_DownMiddle));
	PersistMgr->Transfer(TMEMBER(m_DownRight));
	PersistMgr->Transfer(TMEMBER(m_Image));
	PersistMgr->Transfer(TMEMBER(m_MiddleLeft));
	PersistMgr->Transfer(TMEMBER(m_MiddleMiddle));
	PersistMgr->Transfer(TMEMBER(m_MiddleRight));
	PersistMgr->Transfer(TMEMBER(m_UpLeft));
	PersistMgr->Transfer(TMEMBER(m_UpMiddle));
	PersistMgr->Transfer(TMEMBER(m_UpRight));

	return S_OK;
}

} // end of namespace WinterMute
