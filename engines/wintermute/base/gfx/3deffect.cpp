/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

/*
 * This file is based on WME.
 * http://dead-code.org/redir.php?target=wme
 * Copyright (c) 2003-2013 Jan Nedoma and contributors
 */

#include "common/crc.h"

#include "engines/wintermute/base/base_named_object.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/base/gfx/3deffect.h"
#include "engines/wintermute/utils/utils.h"

namespace Wintermute {

//////////////////////////////////////////////////////////////////////////
Effect3D::Effect3D(BaseGame *inGame) : BaseClass(inGame) {
	_effectHash = 0xFFFFFFFF;
}

//////////////////////////////////////////////////////////////////////////
Effect3D::~Effect3D() {
	_effectHash = 0xFFFFFFFF;
}

//////////////////////////////////////////////////////////////////////////
bool Effect3D::invalidateDeviceObjects() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Effect3D::restoreDeviceObjects() {
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool Effect3D::createFromFile(const Common::String &filename) {
	uint32 size;
	byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename, &size);
	if (!buffer) {
		return false;
	}

	_filename = filename;

	Common::CRC32 crc;
	_effectHash = crc.crcFast(buffer, size);

	delete[] buffer;

	return true;
}

} // namespace Wintermute
