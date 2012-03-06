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
#include "BGame.h"
#include "BSaveThumbFile.h"
#include "PlatformSDL.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
CBSaveThumbFile::CBSaveThumbFile(CBGame *inGame): CBFile(inGame) {
	m_Data = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBSaveThumbFile::~CBSaveThumbFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSaveThumbFile::Open(Common::String Filename) {
	Close();

	if (scumm_strnicmp(Filename.c_str(), "savegame:", 9) != 0) return E_FAIL;

	char *TempFilename = new char[strlen(Filename.c_str()) - 8];
	strcpy(TempFilename, Filename.c_str() + 9);
	for (int i = 0; i < strlen(TempFilename); i++) {
		if (TempFilename[i] < '0' || TempFilename[i] > '9') {
			TempFilename[i] = '\0';
			break;
		}
	}

	// get slot number from name
	int Slot = atoi(TempFilename);
	delete [] TempFilename;

	char SlotFilename[MAX_PATH + 1];
	Game->GetSaveSlotFilename(Slot, SlotFilename);
	CBPersistMgr *pm = new CBPersistMgr(Game);
	if (!pm) return E_FAIL;

	Game->m_DEBUG_AbsolutePathWarning = false;
	if (FAILED(pm->InitLoad(SlotFilename))) {
		Game->m_DEBUG_AbsolutePathWarning = true;
		delete pm;
		return E_FAIL;
	}
	Game->m_DEBUG_AbsolutePathWarning = true;

	HRESULT res;

	if (pm->m_ThumbnailDataSize != 0) {
		m_Data = new byte[pm->m_ThumbnailDataSize];
		memcpy(m_Data, pm->m_ThumbnailData, pm->m_ThumbnailDataSize);
		m_Size = pm->m_ThumbnailDataSize;
		res = S_OK;
	} else res = E_FAIL;
	delete pm;

	return res;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSaveThumbFile::Close() {
	delete[] m_Data;
	m_Data = NULL;

	m_Pos = 0;
	m_Size = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSaveThumbFile::Read(void *Buffer, uint32 Size) {
	if (!m_Data || m_Pos + Size > m_Size) return E_FAIL;

	memcpy(Buffer, (byte  *)m_Data + m_Pos, Size);
	m_Pos += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBSaveThumbFile::Seek(uint32 Pos, TSeek Origin) {
	if (!m_Data) return E_FAIL;

	uint32 NewPos = 0;

	switch (Origin) {
	case SEEK_TO_BEGIN:
		NewPos = Pos;
		break;
	case SEEK_TO_END:
		NewPos = m_Size + Pos;
		break;
	case SEEK_TO_CURRENT:
		NewPos = m_Pos + Pos;
		break;
	}

	if (NewPos < 0 || NewPos > m_Size) return E_FAIL;
	else m_Pos = NewPos;

	return S_OK;
}

} // end of namespace WinterMute
