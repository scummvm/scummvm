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

#include "ultima/ultima8/filesys/file_system.h"

#ifdef BUILTIN_DATA
#include "data/data.h"
#endif

namespace Ultima {
namespace Ultima8 {

void FileSystem::initBuiltinData(bool allowoverride) {
#ifdef BUILTIN_DATA
	allowdataoverride = allowoverride;

	int i = 0;
	while (true) {
		PentagramData::DataFile &file = PentagramData::files[i++];
		if (!file.name) break;

		Std::string vp = "@data/";
		vp += file.name;

		Std::map<Common::String, MemoryFile *>::iterator p = memoryfiles.find(vp);

		if (p != memoryfiles.end()) {
			perr << "Warning: duplicate builtin data file: " << vp
			     << Std::endl;
			continue;
		}

		memoryfiles[vp] = new MemoryFile(file.data, file.size);

	}
#endif
}

} // End of namespace Ultima8
} // End of namespace Ultima
