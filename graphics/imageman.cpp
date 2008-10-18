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
 * $URL$
 * $Id$
 */

#include "graphics/imagedec.h"
#include "graphics/imageman.h"
#include "graphics/surface.h"

#include "common/unzip.h"

DECLARE_SINGLETON(Graphics::ImageManager);

namespace Graphics {

ImageManager::ImageManager() {
}

ImageManager::~ImageManager() {
	for (Iterator pos = _surfaces.begin(); pos != _surfaces.end(); ++pos) {
		(*pos)->surface->free();
		delete (*pos)->surface;
		delete *pos;
		*pos = 0;
	}
	_surfaces.clear();
}

bool ImageManager::addArchive(const Common::String &name) {
	Common::Archive *arch = 0;

	if (name.hasSuffix(".zip")) {
#ifdef USE_ZLIB
		Common::ZipArchive *zip = new Common::ZipArchive(name);
		if (!zip || !zip->isOpen())
			return false;

		arch = zip;
#else
		return false;
#endif
	} else {
		Common::FSDirectory *dir = new Common::FSDirectory(name);
		if (!dir || !dir->getFSNode().isDirectory())	
			return false;

		arch = dir;
	}

	_archives.add(name, Common::ArchivePtr(arch));
	return true;
}

void ImageManager::removeArchive(const Common::String &name) {
	if (_archives.hasArchive(name))
		_archives.remove(name);
}

bool ImageManager::registerSurface(const Common::String &name, Surface *surf) {
	if (getSurface(name)) {
		return false;
	}

	Entry *newHandle = new Entry;
	if (!newHandle)
		return false;

	if (!surf)
		surf = ImageDecoder::loadFile(name);

	if (!surf) {
		Common::SeekableReadStream *stream = _archives.openFile(name);
		if (stream) {
			surf = ImageDecoder::loadFile(*stream);
			delete stream;
		}
	}

	if (!surf)
		return false;

	newHandle->surface = surf;
	newHandle->name = name;
	_surfaces.push_back(newHandle);

	return true;
}

bool ImageManager::unregisterSurface(const Common::String &name) {
	Iterator pos = searchHandle(name);
	if (pos == _surfaces.end()) {
		// no surface handle it as success
		return true;
	}

	(*pos)->surface->free();
	delete (*pos)->surface;
	delete *pos;
	*pos = 0;

	_surfaces.erase(pos);

	return true;
}

Surface * ImageManager::getSurface(const Common::String &name) {
	Iterator pos = searchHandle(name);
	if (pos == _surfaces.end()) {
		// no surface handle it as success
		return 0;
	}
	return (*pos)->surface;
}

ImageManager::Iterator ImageManager::searchHandle(const Common::String &name) {
	Iterator pos = _surfaces.begin();
	while (pos != _surfaces.end()) {
		if ((*pos)->name == name)
			break;
		++pos;
	}
	return pos;
}
} // end of namespace Graphics
