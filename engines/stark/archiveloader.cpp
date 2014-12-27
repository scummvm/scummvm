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

namespace Stark {

ArchiveLoader::LoadedArchive::LoadedArchive(const Common::String& archiveName) :
		_filename(archiveName) {
	if (!_xarc.open(archiveName)) {
		error("Unable to open archive '%s'", archiveName.c_str());
	}

	_root = importResources();
}

ArchiveLoader::LoadedArchive::~LoadedArchive() {
	delete _root;
}

Resource *ArchiveLoader::LoadedArchive::importResources() {
	// Find the XRC file
	Common::ArchiveMemberList members;
	_xarc.listMatchingMembers(members, "*.xrc");
	if (members.size() == 0) {
		error("No resource tree in archive '%s'", _filename.c_str());
	}
	if (members.size() > 1) {
		error("Too many resource scripts in archive '%s'", _filename.c_str());
	}

	// Open the XRC file
	Common::SeekableReadStream *stream = _xarc.createReadStreamForMember(members.front()->getName());

	// Import the resource tree
	Resource *root = XRCReader::importTree(stream);

	delete stream;

	return root;
}

void ArchiveLoader::load(const Common::String &archiveName) {
	if (hasArchive(archiveName)) {
		// Already loaded
		return;
	}
	_archives.push_back(LoadedArchive(archiveName));
}

void ArchiveLoader::unload(const Common::String &archiveName) {
	for (uint i = 0; i < _archives.size(); i++) {
		if (_archives[i].getFilename() == archiveName) {
			_archives.remove_at(i);
			return;
		}
	}

	error("The archive with name '%s' is not loaded.", archiveName.c_str());
}

Common::ReadStream *ArchiveLoader::getFile(const Common::String &fileName, const Common::String &archiveName) {
	LoadedArchive &archive = findArchive(archiveName);
	XARCArchive &xarc = archive.getXArc();
	return xarc.createReadStreamForMember(fileName);
}

Resource *ArchiveLoader::getRoot(const Common::String &archiveName) {
	LoadedArchive &archive = findArchive(archiveName);
	return archive.getRoot();
}

bool ArchiveLoader::hasArchive(const Common::String &archiveName) {
	for (uint i = 0; i < _archives.size(); i++) {
		if (_archives[i].getFilename() == archiveName) {
			return true;
		}
	}

	return false;
}

ArchiveLoader::LoadedArchive &ArchiveLoader::findArchive(const Common::String &archiveName) {
	for (uint i = 0; i < _archives.size(); i++) {
		if (_archives[i].getFilename() == archiveName) {
			return _archives[i];
		}
	}

	error("The archive with name '%s' is not loaded.", archiveName.c_str());
}

} // End of namespace Stark
