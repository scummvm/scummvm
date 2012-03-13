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
#include "BPackage.h"
#include "BGame.h"
#include "BFileManager.h"
#include "common/file.h"

namespace WinterMute {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
CBPackage::CBPackage(CBGame *inGame): CBBase(inGame) {
	m_File = NULL;
	m_Name = NULL;
	m_CD = 0;
	m_Priority = 0;
	m_BoundToExe = false;
}


//////////////////////////////////////////////////////////////////////////
CBPackage::~CBPackage() {
	if (m_Name) delete [] m_Name;
	CloseFilePointer(m_File);
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPackage::Open() {
	if (m_File) return S_OK;
	else {
		m_File = GetFilePointer();
		return m_File ? S_OK : E_FAIL;
	}
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPackage::Close() {
	delete m_File;
	m_File = NULL;
	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBPackage::Read(Common::File *file, uint32 offset, byte *buffer, uint32 size) {
	HRESULT ret;
	if (FAILED(ret = Open())) return ret;
	else {
		if (file->seek(offset, SEEK_SET)) return E_FAIL;
		if (file->read(buffer, size) != 1) return E_FAIL;
		else return S_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
Common::File *CBPackage::GetFilePointer() {
	Common::File *file = Game->m_FileManager->OpenPackage(m_Name);
	if (!file) {
		Game->m_FileManager->RequestCD(m_CD, m_Name, "");
		file = Game->m_FileManager->OpenPackage(m_Name);
	}
	return file;
}

//////////////////////////////////////////////////////////////////////////
void CBPackage::CloseFilePointer(Common::File*& file) {
	delete file;
	file = NULL;
}

} // end of namespace WinterMute
