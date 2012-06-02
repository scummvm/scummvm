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
#include "engines/wintermute/Base/file/BResourceFile.h"
#include "engines/wintermute/Base/BResources.h"

namespace WinterMute {

//////////////////////////////////////////////////////////////////////////
CBResourceFile::CBResourceFile(CBGame *inGame): CBFile(inGame) {
	_data = NULL;
}


//////////////////////////////////////////////////////////////////////////
CBResourceFile::~CBResourceFile() {
	Close();
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Open(const Common::String &filename) {
	Close();

	if (CBResources::GetFile(filename.c_str(), _data, _size)) {
		_pos = 0;
		return S_OK;
	}
	return E_FAIL;
}

//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Close() {
	_data = NULL;
	_pos = 0;
	_size = 0;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Read(void *buffer, uint32 size) {
	if (!_data || _pos + size > _size) return E_FAIL;

	memcpy(buffer, (byte *)_data + _pos, size);
	_pos += size;

	return S_OK;
}


//////////////////////////////////////////////////////////////////////////
HRESULT CBResourceFile::Seek(uint32 pos, TSeek origin) {
	if (!_data) return E_FAIL;

	int32 newPos = 0;

	switch (origin) {
	case SEEK_TO_BEGIN:
		newPos = pos;
		break;
	case SEEK_TO_END:
		newPos = _size + pos;
		break;
	case SEEK_TO_CURRENT:
		newPos = _pos + pos;
		break;
	}

	if (newPos < 0 || newPos > _size) return E_FAIL;
	else _pos = newPos;

	return S_OK;
}

} // end of namespace WinterMute
