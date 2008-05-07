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

DECLARE_SINGLETON(Graphics::ImageManager);

namespace Graphics {
ImageManager::ImageManager() : _surfaces()
#ifdef USE_ZLIB
, _archives()
#endif
{
}

ImageManager::~ImageManager() {
	for (Iterator pos = _surfaces.begin(); pos != _surfaces.end(); ++pos) {
		(*pos)->surface->free();
		delete (*pos)->surface;
		delete *pos;
		*pos = 0;
	}
	_surfaces.clear();
#ifdef USE_ZLIB
	for (ZipIterator pos2 = _archives.begin(); pos2 != _archives.end(); ++pos2) {
		unzClose(pos2->file);
	}
	_archives.clear();
#endif
}

bool ImageManager::addArchive(const Common::String &name) {
#ifdef USE_ZLIB
	unzFile newFile = unzOpen(name.c_str());
	if (!newFile)
		return false;
	Archive arch;
	arch.file = newFile;
	arch.filename = name;
	_archives.push_back(arch);
#endif
	return true;
}

void ImageManager::remArchive(const Common::String &name) {
#ifdef USE_ZLIB
	for (ZipIterator pos = _archives.begin(); pos != _archives.end(); ++pos) {
		if (pos->filename.compareToIgnoreCase(name) == 0) {
			unzClose(pos->file);
			_archives.erase(pos);
			break;
		}
	}
#endif
}

bool ImageManager::registerSurface(const Common::String &name, Surface *surf) {
	if (getSurface(name)) {
		return false;
	}

	Entry *newHandle = new Entry;
	if (!newHandle)
		return false;

	if (!surf) {
		surf = ImageDecoder::loadFile(name);
		if (!surf) {
#ifdef USE_ZLIB
			ZipIterator file = _archives.end();
			for (ZipIterator pos = _archives.begin(); pos != _archives.end(); ++pos) {
				if (unzLocateFile(pos->file, name.c_str(), 2) == UNZ_OK) {
					file = pos;
					break;
				}
			}

			if (file == _archives.end())
				return false;

			unz_file_info fileInfo;
			unzOpenCurrentFile(file->file);
			unzGetCurrentFileInfo(file->file, &fileInfo, NULL, 0, NULL, 0, NULL, 0);
			uint8 *buffer = new uint8[fileInfo.uncompressed_size];
			assert(buffer);
			unzReadCurrentFile(file->file, buffer, fileInfo.uncompressed_size);
			unzCloseCurrentFile(file->file);
			Common::MemoryReadStream stream(buffer, fileInfo.uncompressed_size);
			surf = ImageDecoder::loadFile(stream);
			delete[] buffer;

			if (!surf)
				return false;
#else
			return false;
#endif
		}
	}

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
