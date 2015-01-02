/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
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

#include "engines/stark/archiveloader.h"

#include "engines/stark/xrcreader.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"

namespace Stark {

ArchiveLoader::LoadedArchive::LoadedArchive(const Common::String& archiveName) :
		_filename(archiveName),
		_root(nullptr),
		_useCount(0) {
	if (!_xarc.open(archiveName)) {
		error("Unable to open archive '%s'", archiveName.c_str());
	}
}

ArchiveLoader::LoadedArchive::~LoadedArchive() {
	// Resource lifecycle update
	_root->onPreDestroy();

	delete _root;
}

void ArchiveLoader::LoadedArchive::importResources() {
	// Import the resource tree
	_root = XRCReader::importTree(&_xarc);
}

ArchiveLoader::~ArchiveLoader() {
	for (LoadedArchiveList::iterator it = _archives.begin(); it != _archives.end(); it++) {
		delete *it;
	}
}

bool ArchiveLoader::load(const Common::String &archiveName) {
	if (hasArchive(archiveName)) {
		// Already loaded
		return false;
	}

	LoadedArchive *archive = new LoadedArchive(archiveName);
	_archives.push_back(archive);

	archive->importResources();

	return true;
}

void ArchiveLoader::unloadUnused() {
	for (LoadedArchiveList::iterator it = _archives.begin(); it != _archives.end(); it++) {
		if (!(*it)->isInUse()) {
			delete *it;
			_archives.erase(it);
		}
	}
}

Common::ReadStream *ArchiveLoader::getFile(const Common::String &fileName, const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	XARCArchive &xarc = archive->getXArc();
	return xarc.createReadStreamForMember(fileName);
}

bool ArchiveLoader::returnRoot(const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	archive->decUsage();

	return !archive->isInUse();
}

bool ArchiveLoader::hasArchive(const Common::String &archiveName) {
	for (LoadedArchiveList::iterator it = _archives.begin(); it != _archives.end(); it++) {
		if ((*it)->getFilename() == archiveName) {
			return true;
		}
	}

	return false;
}

ArchiveLoader::LoadedArchive *ArchiveLoader::findArchive(const Common::String &archiveName) {
	for (LoadedArchiveList::iterator it = _archives.begin(); it != _archives.end(); it++) {
		if ((*it)->getFilename() == archiveName) {
			return *it;
		}
	}

	error("The archive with name '%s' is not loaded.", archiveName.c_str());
}

Common::String ArchiveLoader::buildArchiveName(Level *level, Location *location) {
	Common::String archive;

	if (!location) {
		switch (level->getSubType()) {
		case 1:
			archive = Common::String::format("%s/%s.xarc", level->getName().c_str(), level->getName().c_str());
			break;
		case 2:
			archive = Common::String::format("%02x/%02x.xarc", level->getIndex(), level->getIndex());
			break;
		default:
			error("Unknown level archive type %d", level->getSubType());
		}
	} else {
		archive = Common::String::format("%02x/%02x/%02x.xarc", level->getIndex(), location->getIndex(), location->getIndex());
	}

	return archive;
}

} // End of namespace Stark
