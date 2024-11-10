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

#include "common/archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/compression/unzip.h"
#include "common/engine_data.h"

namespace Common {

class DataArchiveMember : public Common::ArchiveMember {
private:
	Common::SharedPtr<Common::ArchiveMember> _member;
	Common::Path _publicFolder;
	Common::Path _innerfolder;
public:
	DataArchiveMember(Common::SharedPtr<Common::ArchiveMember> member,
		const Common::Path &subfolder, const Common::Path &publicFolder) :
			_member(member), _publicFolder(publicFolder), _innerfolder(subfolder) {
	}
	~DataArchiveMember() override {
	}
	Common::SeekableReadStream *createReadStream() const override {
		return _member->createReadStream();
	}
	Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override {
		return nullptr;
	}
	Common::Path getPathInArchive() const override {
		Common::Path name = _member->getPathInArchive();
		return _publicFolder.join(name.relativeTo(_innerfolder));
	}
	Common::U32String getDisplayName() const override {
		return _member->getDisplayName();
	}
	Common::String getFileName() const override { return _member->getFileName(); }
	Common::String getName() const override { return getPathInArchive().toString(); }
};

/**
 * The data archive class encapsulates access to a specific subfolder
 * for the game within the engine data zip file as a generic "data" folder
 */
class DataArchive : public Common::Archive {
private:
	Common::Archive *_zip;
	Common::Path _publicFolder;
	Common::Path _innerfolder;

	Common::Path publicToInner(const Common::Path &filename) const {
		assert(filename.isRelativeTo(_publicFolder));
		return _innerfolder.join(filename.relativeTo(_publicFolder));
	}
public:
	DataArchive(Common::Archive *zip, const Common::Path &subfolder, bool useDataPrefix) :
		_zip(zip), _publicFolder(useDataPrefix ? "data/" : ""), _innerfolder(subfolder) {
	}
	~DataArchive() override {
		delete _zip;
	}

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::Path &path) const override;

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override;

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::Path &path)
		const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(
		const Common::Path &path) const override;
};

#ifndef RELEASE_BUILD

/**
 * The data archive proxy class is used for debugging purposes to access engine data
 * files when the create_mm folder is in the search path. It will allow for
 * local mucking around with the data files and committing changes without having to
 * recreate the data file every time a change is made. mm.dat then just has
 * to be recreated prior to a release or when the changes are completed and stable
 */
class DataArchiveProxy : public Common::Archive {
	friend class DataArchive;
private:
	Common::FSNode _folder;
	const Common::Path _publicFolder;

	/**
	 * Gets a file node from the passed filename
	 */
	Common::FSNode getNode(const Common::Path &name) const;
public:
	DataArchiveProxy(const Common::FSNode &folder, bool useDataPrefix) :
		_folder(folder), _publicFolder(useDataPrefix ? "data/" : "") {}
	~DataArchiveProxy() override {}

	/**
	 * Check if a member with the given name is present in the Archive.
	 * Patterns are not allowed, as this is meant to be a quick File::exists()
	 * replacement.
	 */
	bool hasFile(const Common::Path &path) const override {
		return path.isRelativeTo(_publicFolder) && getNode(path).exists();
	}

	/**
	 * Add all members of the Archive to list.
	 * Must only append to list, and not remove elements from it.
	 *
	 * @return the number of names added to list
	 */
	int listMembers(Common::ArchiveMemberList &list) const override {
		return 0;
	}

	/**
	 * Returns a ArchiveMember representation of the given file.
	 */
	const Common::ArchiveMemberPtr getMember(const Common::Path &path)
		const override;

	/**
	 * Create a stream bound to a member with the specified name in the
	 * archive. If no member with this name exists, 0 is returned.
	 * @return the newly created input stream
	 */
	Common::SeekableReadStream *createReadStreamForMember(
		const Common::Path &path) const override;
};

#endif

bool DataArchive::hasFile(const Common::Path &path) const {
	if (!path.isRelativeTo(_publicFolder))
		return false;

	Common::Path realFilename = publicToInner(path);
	return _zip->hasFile(realFilename);
}

int DataArchive::listMembers(Common::ArchiveMemberList &list) const {
	Common::ArchiveMemberList innerList;
	int result = _zip->listMembers(innerList);

	// Modify the results to change the filename
	for (Common::ArchiveMemberList::iterator it = innerList.begin();
		it != innerList.end(); ++it) {
		Common::ArchiveMemberPtr member = Common::ArchiveMemberPtr(
			new DataArchiveMember(*it, _innerfolder, _publicFolder));
		list.push_back(member);
	}

	return result;
}

const Common::ArchiveMemberPtr DataArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *DataArchive::createReadStreamForMember(const Common::Path &path) const {
	if (hasFile(path)) {
		Common::Path filename = publicToInner(path);
		return _zip->createReadStreamForMember(filename);
	}

	return nullptr;
}

/*------------------------------------------------------------------------*/

#ifndef RELEASE_BUILD

const Common::ArchiveMemberPtr DataArchiveProxy::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *DataArchiveProxy::createReadStreamForMember(const Common::Path &path) const {
	if (hasFile(path))
		return getNode(path).createReadStream();

	return nullptr;
}

Common::FSNode DataArchiveProxy::getNode(const Common::Path &name) const {
	Common::Path remainingName = name.relativeTo(_publicFolder);
	Common::FSNode node = _folder;

	Common::StringArray components = remainingName.splitComponents();

	if (components.empty()) {
		return node;
	}

	for(Common::StringArray::const_iterator it = components.begin(); it != components.end() - 1; it++) {
		node = node.getChild(*it);
		if (!node.exists())
			return node;
	}

	node = node.getChild(*(components.end() - 1));

	return node;
}

#endif

/*------------------------------------------------------------------------*/

bool load_engine_data(const Common::Path &datFilename, const Common::String &subfolder, int reqMajorVersion,
		int reqMinorVersion, Common::U32String &errorMsg, bool useDataPrefix) {
	Common::Archive *dataArchive = nullptr;
	Common::File f;

#ifndef RELEASE_BUILD
	Common::FSNode folder;
	if (ConfMan.hasKey("extrapath")) {
		if ((folder = Common::FSNode(ConfMan.getPath("extrapath"))).exists()
			&& (folder = folder.getChild("files")).exists()
			&& (folder = folder.getChild(subfolder)).exists()) {
			f.open(folder.getChild("version.txt"));
		}
	}

#endif
	if (!f.isOpen()) {
		Common::String versionFile = subfolder.empty() ? "version.txt" :
			Common::String::format("%s/version.txt", subfolder.c_str());

		if (!Common::File::exists(datFilename) ||
			(dataArchive = Common::makeZipArchive(datFilename)) == 0 ||
			!f.open(Common::Path(versionFile), *dataArchive)) {
			delete dataArchive;
			errorMsg = Common::U32String::format(_("Could not locate engine data %s"), datFilename.toString().c_str());
			return false;
		}
	}

	// Validate the version
	char buffer[5];
	f.read(buffer, 4);
	buffer[4] = '\0';

	int major = 0, minor = 0;
	if (buffer[1] == '.') {
		major = buffer[0] - '0';
		minor = atoi(&buffer[2]);
	}

	if (major != reqMajorVersion || minor != reqMinorVersion) {
		delete dataArchive;
		errorMsg = Common::U32String::format(_("Out of date engine data. Expected %d.%d, but got version %d.%d"),
			reqMajorVersion, reqMinorVersion, major, minor);
		return false;
	}

	// It was all validated correctly
	Common::Archive *archive;
#ifndef RELEASE_BUILD
	if (!dataArchive)
		archive = new DataArchiveProxy(folder, useDataPrefix);
	else
#endif
		archive = new DataArchive(dataArchive, Common::Path(subfolder), useDataPrefix);

	SearchMan.add("data", archive);
	return true;
}

} // namespace Common
