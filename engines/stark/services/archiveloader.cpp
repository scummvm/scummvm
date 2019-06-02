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

#include "engines/stark/services/archiveloader.h"

#include "engines/stark/formats/xrc.h"
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
	_root = Formats::XRCReader::importTree(&_xarc);
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
			it = _archives.erase(it);
			it--;
		}
	}
}

ArchiveReadStream *ArchiveLoader::getFile(const Common::String &fileName, const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	const Formats::XARCArchive &xarc = archive->getXArc();

	Common::SeekableReadStream *stream = xarc.createReadStreamForMember(fileName);
	if (!stream) {
		return nullptr;
	}

	return new ArchiveReadStream(stream);
}

bool ArchiveLoader::returnRoot(const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	archive->decUsage();

	return !archive->isInUse();
}

bool ArchiveLoader::hasArchive(const Common::String &archiveName) const {
	for (LoadedArchiveList::const_iterator it = _archives.begin(); it != _archives.end(); it++) {
		if ((*it)->getFilename() == archiveName) {
			return true;
		}
	}

	return false;
}

ArchiveLoader::LoadedArchive *ArchiveLoader::findArchive(const Common::String &archiveName) const {
	for (LoadedArchiveList::const_iterator it = _archives.begin(); it != _archives.end(); it++) {
		if ((*it)->getFilename() == archiveName) {
			return *it;
		}
	}

	error("The archive with name '%s' is not loaded.", archiveName.c_str());
}

Common::String ArchiveLoader::buildArchiveName(Resources::Level *level, Resources::Location *location) const {
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
			error("Unknown level type %d", level->getSubType());
		}
	} else {
		archive = Common::String::format("%02x/%02x/%02x.xarc", level->getIndex(), location->getIndex(), location->getIndex());
	}

	return archive;
}

Common::String ArchiveLoader::getExternalFilePath(const Common::String &fileName, const Common::String &archiveName) const {
	static const char separator = '/';

	// Build a path of the type 45/00/
	Common::String filePath = archiveName;
	while (filePath.lastChar() != separator && !filePath.empty()) {
		filePath.deleteLastChar();
	}
	filePath += "xarc/" + fileName;

	return filePath;
}

Common::SeekableReadStream *ArchiveLoader::getExternalFile(const Common::String &fileName, const Common::String &archiveName) const {
	Common::String filePath = getExternalFilePath(fileName, archiveName);
	return SearchMan.createReadStreamForMember(filePath);
}

ArchiveReadStream::ArchiveReadStream(
		Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream) :
		SeekableSubReadStream(parentStream, 0, parentStream->size(), disposeParentStream) {
}

ArchiveReadStream::~ArchiveReadStream() {
}

Common::String ArchiveReadStream::readString() {
	// Read the string length
	uint32 length = readUint32LE();

	// Read the string
	char *data = new char[length + 1];
	read(data, length);
	data[length] = '\0';

	Common::String string(data);
	delete[] data;

	return string;
}

Common::String ArchiveReadStream::readString16() {
	// Read the string length
	uint16 length = readUint16LE();

	// Read the string
	char *data = new char[length + 1];
	read(data, length);
	data[length] = '\0';

	Common::String string(data);
	delete[] data;

	return string;
}

Math::Vector3d ArchiveReadStream::readVector3() {
	Math::Vector3d v;
	v.readFromStream(this);
	return v;
}

Math::Quaternion ArchiveReadStream::readQuaternion() {
	Math::Quaternion q;
	q.readFromStream(this);
	return q;
}

float ArchiveReadStream::readFloat() {
	float f;
	read(&f, sizeof(float));
	return f;
}

} // End of namespace Stark
