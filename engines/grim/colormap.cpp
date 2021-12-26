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

#include "common/stream.h"

#include "engines/grim/colormap.h"
#include "engines/grim/resource.h"

namespace Grim {

// Load a colormap from the given data.
CMap::CMap(const Common::String &fileName, Common::SeekableReadStream *data) :
		Object(), _fname(fileName) {
	uint32 tag = data->readUint32BE();
	if (tag != MKTAG('C','M','P',' '))
		error("Invalid magic loading colormap");

	data->seek(64, SEEK_SET);
	data->read(_colors, sizeof(_colors));
}

CMap::~CMap() {
	if (g_resourceloader)
		g_resourceloader->uncacheColormap(this);
}

bool CMap::operator==(const CMap &c) const {
	if (_fname != c._fname) {
		return false;
	}

	return true;
}


} // end of namespace Grim

