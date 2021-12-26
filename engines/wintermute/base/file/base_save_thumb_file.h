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
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_SAVETHUMBFILE_H
#define WINTERMUTE_BASE_SAVETHUMBFILE_H


#include "engines/wintermute/base/file/base_file.h"

namespace Wintermute {

//TODO: Get rid of this
class BaseSaveThumbFile : public BaseFile {
public:
	BaseSaveThumbFile();
	~BaseSaveThumbFile() override;
	bool seek(uint32 pos, int whence = SEEK_SET) override;
	bool read(void *buffer, uint32 size) override;
	bool close() override;
	bool open(const Common::String &filename) override;
private:
	byte *_data;
};

} // End of namespace Wintermute

#endif
