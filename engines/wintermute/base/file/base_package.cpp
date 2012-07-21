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
#include "engines/wintermute/base/file/base_package.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "common/file.h"
#include "common/stream.h"

namespace WinterMute {
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
BasePackage::BasePackage(BaseGame *inGame): BaseClass(inGame) {
	_file = NULL;
	_name = NULL;
	_cD = 0;
	_priority = 0;
	_boundToExe = false;
}


//////////////////////////////////////////////////////////////////////////
BasePackage::~BasePackage() {
	if (_name) delete [] _name;
	closeFilePointer(_file);
}


//////////////////////////////////////////////////////////////////////////
bool BasePackage::open() {
	if (_file) return STATUS_OK;
	else {
		_file = getFilePointer();
		return _file ? STATUS_OK : STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BasePackage::close() {
	delete _file;
	_file = NULL;
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BasePackage::read(Common::SeekableReadStream *file, uint32 offset, byte *buffer, uint32 size) {
	bool ret;
	if (DID_FAIL(ret = open())) return ret;
	else {
		if (file->seek(offset, SEEK_SET)) return STATUS_FAILED;
		if (file->read(buffer, size) != 1) return STATUS_FAILED;
		else return STATUS_OK;
	}
}

//////////////////////////////////////////////////////////////////////////
Common::SeekableReadStream *BasePackage::getFilePointer() {
	Common::File *file = _gameRef->_fileManager->openPackage(_name);
	if (!file) {
		_gameRef->_fileManager->requestCD(_cD, _name, "");
		file = _gameRef->_fileManager->openPackage(_name);
	}
	return file;
}

//////////////////////////////////////////////////////////////////////////
void BasePackage::closeFilePointer(Common::SeekableReadStream *&file) {
	delete file;
	file = NULL;
}

} // end of namespace WinterMute
