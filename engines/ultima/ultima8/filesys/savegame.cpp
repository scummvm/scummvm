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

#include "ultima/ultima8/misc/pent_include.h"

#include "ultima/ultima8/filesys/savegame.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/zip_file.h"

namespace Ultima8 {

Savegame::Savegame(IDataSource *ds) {
	zipfile = new ZipFile(ds);
}

Savegame::~Savegame() {
	delete zipfile;
}

uint32 Savegame::getVersion() {
	IDataSource *ids = getDataSource("VERSION");
	if (!ids || ids->getSize() != 4) return 0;

	uint32 version = ids->read4();
	delete ids;

	return version;
}

std::string Savegame::getDescription() {
	return zipfile->getComment();
}

IDataSource *Savegame::getDataSource(const std::string &name) {
	uint32 size;
	uint8 *data = zipfile->getObject(name, &size);
	return new IBufferDataSource(data, size, false, true);
}

} // End of namespace Ultima8
