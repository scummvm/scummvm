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

#ifndef ULTIMA8_FILESYS_SAVEGAMEWRITER_H
#define ULTIMA8_FILESYS_SAVEGAMEWRITER_H

#include "ultima8/std/string.h"

namespace Ultima8 {

class ODataSource;
class OAutoBufferDataSource;

class SavegameWriter {
public:
	explicit SavegameWriter(ODataSource *ds);
	virtual ~SavegameWriter();

	//! write the savegame's description.
	bool writeDescription(const std::string &desc);

	//! write the savegame's global version
	bool writeVersion(uint32 version);

	//! write a file to the savegame
	//! \param name name of the file
	//! \param data the data
	//! \param size (in bytes) of data
	virtual bool writeFile(const char *name, const uint8 *data, uint32 size);

	//! write a file to the savegame from an OAutoBufferDataSource
	//! \param name name of the file
	//! \param buf the OBufferDataSource to save
	bool writeFile(const char *name, OAutoBufferDataSource *buf);

	//! finish savegame
	bool finish();

protected:
	ODataSource *ds;
	std::string comment;
	void *zipfile;
};

} // End of namespace Ultima8

#endif
