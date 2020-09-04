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

#include "ultima/shared/engine/data_archive.h"
#include "common/config-manager.h"
#include "common/file.h"
#include "common/translation.h"
#include "common/unzip.h"

namespace Ultima {
namespace Shared {

#define DATA_FILENAME "ultima.dat"

class UltimaDataArchiveMember : public Common::ArchiveMember {
private:
	Common::SharedPtr<Common::ArchiveMember> _member;
	Common::String _publicFolder;
	Common::String _innerfolder;
public:
	UltimaDataArchiveMember(Common::SharedPtr<Common::ArchiveMember> member,
		const Common::String &subfolder) :
		_member(member), _publicFolder("data/"), _innerfolder(subfolder) {
	}
	~UltimaDataArchiveMember() override {}
	Common::SeekableReadStream *createReadStream() const override {
		return _member->createReadStream();
	}
	Common::String getName() const override {
		Common::String name = _member->getName();
		assert(name.hasPrefixIgnoreCase(_innerfolder));
		return _publicFolder + Common::String(name.c_str() + _innerfolder.size());
	}
	Common::String getDisplayName() const override {
		return _member->getDisplayName();
	}
};

/*-------------------------------------------------------------------*/

bool UltimaDataArchive::load(const Common::String &subfolder,
		int reqMajorVersion, int reqMinorVersion, Common::U32String &errorMsg) {
	Common::Archive *dataArchive = nullptr;
	Common::File f;

#ifndef RELEASE_BUILD
	Common::FSNode folder;
	if (ConfMan.hasKey("extrapath")) {
		if ((folder = Common::FSNode(ConfMan.get("extrapath"))).exists()
				&& (folder = folder.getChild("files")).exists()
				&& (folder = folder.getChild(subfolder)).exists()) {
			f.open(folder.getChild("version.txt"));
		}
	}

#endif
	if (!f.isOpen()) {
		if (!Common::File::exists(DATA_FILENAME) ||
			(dataArchive = Common::makeZipArchive(DATA_FILENAME)) == 0 ||
			!f.open(Common::String::format("%s/version.txt", subfolder.c_str()), *dataArchive)) {
			delete dataArchive;
			errorMsg = Common::U32String::format(_("Could not locate engine data %s"), DATA_FILENAME);
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
		archive = new UltimaDataArchiveProxy(folder);
	else
#endif
		archive = new UltimaDataArchive(dataArchive, subfolder);

	SearchMan.add("data", archive);
	return true;
}

/*-------------------------------------------------------------------*/

bool UltimaDataArchive::hasFile(const Common::String &name) const {
	if (!name.hasPrefixIgnoreCase(_publicFolder))
		return false;

	Common::String realFilename = innerToPublic(name);
	return _zip->hasFile(realFilename);
}

int UltimaDataArchive::listMatchingMembers(Common::ArchiveMemberList &list, const Common::String &pattern) const {
	Common::String patt = pattern;
	if (pattern.hasPrefixIgnoreCase(_publicFolder))
		patt = innerToPublic(pattern);

	// Get any matching files
	Common::ArchiveMemberList innerList;
	int result = _zip->listMatchingMembers(innerList, patt);

	// Modify the results to change the filename
	for (Common::ArchiveMemberList::iterator it = innerList.begin();
			it != innerList.end(); ++it) {
		Common::ArchiveMemberPtr member = Common::ArchiveMemberPtr(
			new UltimaDataArchiveMember(*it, _innerfolder));
		list.push_back(member);		
	}

	return result;
}

int UltimaDataArchive::listMembers(Common::ArchiveMemberList &list) const {
	Common::ArchiveMemberList innerList;
	int result = _zip->listMembers(innerList);

	// Modify the results to change the filename
	for (Common::ArchiveMemberList::iterator it = innerList.begin();
		it != innerList.end(); ++it) {
		Common::ArchiveMemberPtr member = Common::ArchiveMemberPtr(
			new UltimaDataArchiveMember(*it, _innerfolder));
		list.push_back(member);
	}

	return result;
}

const Common::ArchiveMemberPtr UltimaDataArchive::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *UltimaDataArchive::createReadStreamForMember(const Common::String &name) const {
	if (hasFile(name)) {
		Common::String filename = innerToPublic(name);
		return _zip->createReadStreamForMember(filename);
	}

	return nullptr;
}

/*-------------------------------------------------------------------*/

#ifndef RELEASE_BUILD

const Common::ArchiveMemberPtr UltimaDataArchiveProxy::getMember(const Common::String &name) const {
	if (!hasFile(name))
		return Common::ArchiveMemberPtr();

	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(name, this));
}

Common::SeekableReadStream *UltimaDataArchiveProxy::createReadStreamForMember(const Common::String &name) const {
	if (hasFile(name))
		return getNode(name).createReadStream();

	return nullptr;
}

Common::FSNode UltimaDataArchiveProxy::getNode(const Common::String &name) const {
	Common::String remainingName = name.substr(_publicFolder.size());
	Common::FSNode node = _folder;
	size_t pos;
	
	while ((pos = remainingName.findFirstOf('/')) != Common::String::npos) {
		node = node.getChild(remainingName.substr(0, pos));
		if (!node.exists())
			return node;

		remainingName = remainingName.substr(pos + 1);
	}

	if (!remainingName.empty())
		node = node.getChild(remainingName);

	return node;	
}

#endif

} // End of namespace Shared
} // End of namespace Ultima
