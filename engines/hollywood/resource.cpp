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

#include "hollywood/resource.h"

#include "common/file.h"
#include "common/substream.h"

namespace Hollywood {

void ResourceChunkTable::clear() {
	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i) {
		offsets[i] = 0;
		sizes[i] = 0;
	}
}

bool ResourceChunkTable::isValidChunk(uint index) const {
	return index < HollywoodEngine::kResourceChunkCount && sizes[index] != 0;
}

bool ResourceManager::readChunkTable(const Common::Path &fileName, ResourceChunkTable &table) const {
	Common::File file;
	if (!file.open(fileName)) {
		table.clear();
		return false;
	}

	return readChunkTable(file, table);
}

Common::SeekableReadStream *ResourceManager::createChunkReadStream(const Common::Path &fileName, uint index) const {
	if (index >= HollywoodEngine::kResourceChunkCount)
		return nullptr;

	Common::File *file = new Common::File();
	if (!file->open(fileName)) {
		delete file;
		return nullptr;
	}

	ResourceChunkTable table;
	if (!readChunkTable(*file, table) || !table.isValidChunk(index)) {
		delete file;
		return nullptr;
	}

	const uint32 begin = table.offsets[index];
	const uint32 fileSize = file->size();
	if (begin > fileSize || table.sizes[index] > fileSize - begin) {
		delete file;
		return nullptr;
	}

	const uint32 end = begin + table.sizes[index];
	return new Common::SeekableSubReadStream(file, begin, end, DisposeAfterUse::YES);
}

bool ResourceManager::readChunkTable(Common::SeekableReadStream &stream, ResourceChunkTable &table) const {
	table.clear();

	if (stream.size() < HollywoodEngine::kResourceChunkCount * 8)
		return false;

	stream.seek(0);
	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i)
		table.offsets[i] = stream.readUint32LE();

	for (int i = 0; i < HollywoodEngine::kResourceChunkCount; ++i)
		table.sizes[i] = stream.readUint32LE();

	return !stream.err();
}

} // End of namespace Hollywood
