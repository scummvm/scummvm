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

#ifndef HARVESTER_RESOURCES_H
#define HARVESTER_RESOURCES_H

#include "common/archive.h"
#include "common/array.h"

namespace Harvester {

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	void reset();

	bool hasFile(const Common::String &path) const;
	Common::SeekableReadStream *openFile(const Common::String &path) const;
	bool loadFile(const Common::String &path, Common::Array<byte> &data) const;

	Common::String normalizeResourcePath(const Common::String &path) const;
	void mountArchive(const Common::String &name, Common::Archive *archive, int priority = 10, bool autoFree = true);

private:
	Common::SearchSet _search;
};

} // End of namespace Harvester

#endif // HARVESTER_RESOURCES_H
