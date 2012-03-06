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
#include "BResourceFile.h"
#include "BResources.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBResourceFile::CBResourceFile(CBGame *inGame): CBFile(inGame) {
	m_Data = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBResourceFile::~CBResourceFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Open(Common::String Filename) {
	Close();

	if (CBResources::GetFile(Filename.c_str(), m_Data, m_Size)) {
		m_Pos = 0;
		return S_OK;
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Close() {
	m_Data = NULL;
	m_Pos = 0;
	m_Size = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Read(void *Buffer, uint32 Size) {
	if (!m_Data || m_Pos + Size > m_Size) return E_FAIL;

	memcpy(Buffer, (byte  *)m_Data + m_Pos, Size);
	m_Pos += Size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Seek(uint32 Pos, TSeek Origin) {
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
