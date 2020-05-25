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

FileMgr::~FileMgr() {
	debug("FileMgr::dtor");
	closeAll();
}

bool FileMgr::openStore(const Common::String &name) {
	Common::ScopedPtr<Common::File> file(new Common::File());
	if (name.empty() || !file->open(name) || file->readUint32BE() != MKTAG('S', 't', 'O', 'R')) {
		return false;
	}

	const uint32 tableOffset = file->readUint32LE();
	if (!file->seek(tableOffset)) {
		return false;
	}

	_stores.push_back(Store());
	Store &store = _stores.back();
	store.descriptions.resize(file->readUint32LE());

	for (uint i = 0; i < store.descriptions.size(); ++i) {
		file->skip(4);
		store.descriptions[i].offset = file->readUint32LE();
		store.descriptions[i].size = file->readUint32LE();
	}

	for (uint i = 0; i < store.descriptions.size(); ++i) {
		char ch;
		while ((ch = file->readByte()) != 0) {
			store.descriptions[i].name += ch;
		}
	}
	store.file = file.release();

	debug("FileMgr: opened store %s (files count: %d)", name.c_str(), store.descriptions.size());

	return true;
}

void FileMgr::closeStore(const Common::String &name) {
	for (uint i = 0; i < _stores.size(); ++i) {
		if (_stores[i].file->getName() == name) {
			delete _stores[i].file;
			_stores.remove_at(i);
			return;
		}
	}

}

void FileMgr::closeAll() {
	debug("FileMgr::closeAll");
	for (uint i = 0; i < _stores.size(); ++i) {
		delete _stores[i].file;
	}
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
	for (uint i = 0; i < _stores.size(); ++i) {
		for (uint j = 0; j < _stores[i].descriptions.size(); ++j) {
			const Description &desc = _stores[i].descriptions[j];
			if (desc.name.compareToIgnoreCase(name) == 0) {
				return new Common::SafeSeekableSubReadStream(_stores[i].file, desc.offset, desc.offset + desc.size);
			}

		}
	}
	debugC(kPetkaDebugResources, "FileMgr: %s not found", name.c_str());
	return nullptr;
}

} // End of namespace Petka
