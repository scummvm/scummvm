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

#include "common/debug.h"
#include "common/file.h"
#include "common/substream.h"

#include "petka/petka.h"
#include "petka/file_mgr.h"

namespace Petka {

bool FileMgr::openStore(const Common::String &name) {
	Common::SharedPtr<Common::File> file(new Common::File());
	if (name.empty() || !file->open(name) || file->readUint32BE() != MKTAG('S', 't', 'O', 'R')) {
		return false;
	}

	const uint32 tableOffset = file->readUint32LE();
	if (!file->seek(tableOffset)) {
		return false;
	}

	const uint32 tableSize = file->size() - file->pos();

	Common::ScopedPtr<Common::SeekableReadStream> stream(file->readStream(tableSize));
	if (stream->size() != (int)tableSize)
		return false;

	_stores.push_back(Store());

	Store &store = _stores.back();
	store.file = file;
	store.descriptions.resize(stream->readUint32LE());

	for (auto &description : store.descriptions) {
		stream->skip(4);
		description.offset = stream->readUint32LE();
		description.size = stream->readUint32LE();
	}

	for (auto &description : store.descriptions) {
		char ch;
		while ((ch = stream->readByte()) != '\0') {
			description.name += ch;
		}
	}

	debugC(kPetkaDebugResources, "FileMgr: opened store %s (files count: %d)", name.c_str(), store.descriptions.size());
	return true;
}

void FileMgr::closeStore(const Common::String &name) {
	for (auto it = _stores.begin(); it != _stores.end(); ++it) {
		if (it->file->getName() == name) {
			_stores.erase(it);
			return;
		}
	}
}

void FileMgr::closeAll() {
	_stores.clear();
}

static Common::String formPath(Common::String name) {
	for (uint i = 0; i < name.size(); ++i) {
		if (name[i] == '\\') {
			name.setChar('/', i);
		}
	}
	return name;
}

Common::SeekableReadStream *FileMgr::getFileStream(const Common::String &name) {
	Common::ScopedPtr<Common::File> file(new Common::File());
	if (file->open(formPath(name))) {
		debugC(kPetkaDebugResources, "FileMgr: %s is opened from game directory", name.c_str());
		return file.release();
	}

	for (auto &store : _stores) {
		for (auto &resource : store.descriptions) {
			if (resource.name.compareToIgnoreCase(name) == 0)
				return new Common::SafeSeekableSubReadStream(store.file.get(), resource.offset, resource.offset + resource.size);
		}
	}

	debugC(kPetkaDebugResources, "FileMgr: %s not found", name.c_str());
	return nullptr;
}

} // End of namespace Petka
