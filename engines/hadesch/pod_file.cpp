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
 * Copyright 2020 Google
 *
 */
#include "common/debug.h"
#include "common/file.h"
#include "common/memstream.h"

#include "hadesch/hadesch.h"
#include "hadesch/pod_file.h"

namespace Hadesch {

PodFile::PodFile(const Common::String &debugName) {
	_debugName = debugName;
}

Common::String PodFile::getDebugName() const {
	return _debugName;
}

bool PodFile::openStore(const Common::SharedPtr<Common::SeekableReadStream> &parentStream) {
	byte buf[16];
	if (!parentStream) {
		return false;
	}

	if (parentStream->read(buf, 12) != 12) {
		return false;
	}

	if (memcmp(buf, "Pod File\0\0\0\0", 12) != 0 &&
	    memcmp(buf, "Pod\0file\0\0\0\0", 12) != 0 &&
	    memcmp(buf, "Pod\0\0\0\0\0\0\0\0\0", 12) != 0) {
		return false;
	}

	const uint32 numFiles = parentStream->readUint32LE();
	uint32 offset = 16 + 16 * numFiles;

	_descriptions.resize(numFiles);

	for (uint i = 0; i < _descriptions.size(); ++i) {
		parentStream->read(buf, 12);
		buf[12] = '\0';
		const uint32 size = parentStream->readUint32LE();

		_descriptions[i].name = (const char *) buf;
		_descriptions[i].offset = offset;
		_descriptions[i].size = size;
		offset += size;
	}

	_file = parentStream;

	return true;
}

bool PodFile::openStore(const Common::String &name) {
	Common::SharedPtr<Common::File> file(new Common::File());
	if (name.empty() || !file->open(name)) {
		return false;
	}

	return openStore(file);
}

// It's tempting to use substream but substream is not thread safe
Common::SeekableReadStream *memSubstream(Common::SharedPtr<Common::SeekableReadStream> file,
					 uint32 offset, uint32 size) {
	byte *contents = (byte *) malloc(size);
	if (!contents)
		return nullptr;
	file->seek(offset);
	file->read(contents, size);
	return new Common::MemoryReadStream(contents, size, DisposeAfterUse::YES);
}

Common::SeekableReadStream *PodFile::getFileStream(const Common::String &name) const {
	for (uint j = 0; j < _descriptions.size(); ++j) {
		const Description &desc = _descriptions[j];
		if (desc.name.compareToIgnoreCase(name) == 0) {
			return memSubstream(
				_file, desc.offset, desc.size);
		}
	}
	debugC(kHadeschDebugResources, "PodFile: %s not found", name.c_str());
	return nullptr;
}

Common::Array <PodImage> PodFile::loadImageArray() const {
  	Common::Array <PodImage> pis;

  	for (int idx = 1; ; idx++) {
		PodImage pi;
		if (!pi.loadImage(*this, idx)) {
			break;
		}
		pis.push_back(pi);
	}

	return pis;
}

} // End of namespace Hadesch
