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

#include "harvester/resources.h"

#include "common/stream.h"

namespace Harvester {

ResourceManager::ResourceManager() {
	reset();
}

ResourceManager::~ResourceManager() {
	_search.clear();
}

void ResourceManager::reset() {
	_search.clear();
	_search.add("harvester-loose-files", &SearchMan, 0, false);
}

Common::String ResourceManager::normalizeResourcePath(const Common::String &path) const {
	Common::String normalized(path);

	for (uint i = 0; i < normalized.size(); ++i) {
		if (normalized[i] == '\\')
			normalized.setChar('/', i);
	}

	while (normalized.hasPrefix("./"))
		normalized.erase(0, 2);

	if (normalized.size() >= 3 && normalized[1] == ':' && normalized[2] == '/')
		normalized.erase(0, 3);

	while (!normalized.empty() && normalized[0] == '/')
		normalized.erase(0, 1);

	return normalized;
}

bool ResourceManager::hasFile(const Common::String &path) const {
	const Common::String normalized = normalizeResourcePath(path);
	return !normalized.empty() && _search.hasFile(Common::Path(normalized, '/'));
}

Common::SeekableReadStream *ResourceManager::openFile(const Common::String &path) const {
	const Common::String normalized = normalizeResourcePath(path);
	if (normalized.empty())
		return nullptr;

	return _search.createReadStreamForMember(Common::Path(normalized, '/'));
}

bool ResourceManager::loadFile(const Common::String &path, Common::Array<byte> &data) const {
	Common::ScopedPtr<Common::SeekableReadStream> stream(openFile(path));
	if (!stream)
		return false;

	data.resize(stream->size());
	if (!data.empty())
		stream->read(data.data(), data.size());

	return true;
}

void ResourceManager::mountArchive(const Common::String &name, Common::Archive *archive, int priority, bool autoFree) {
	_search.add(name, archive, priority, autoFree);
}

} // End of namespace Harvester
