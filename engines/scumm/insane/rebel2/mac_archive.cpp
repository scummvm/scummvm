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
#include "common/array.h"
#include "scumm/file.h"
#include "scumm/insane/rebel2/mac_archive.h"

namespace Scumm {

// RA2 uses the SCUMM Mac container format, but its members are flat names
// rather than the DOS paths used by its movies, fonts and sounds.
class Rebel2MacArchive : public Common::Archive {
public:
	Rebel2MacArchive(const ScummEngine *vm, const Common::Path &filename) : _vm(vm), _filename(filename) {}
	bool load();
	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

private:
	const ScummEngine *_vm;
	Common::Path _filename;
	Common::Array<Common::String> _members;
};

bool Rebel2MacArchive::load() {
	ScummFile file(_vm);
	if (!file.open(_filename) || file.size() < 8)
		return false;

	const uint32 offset = file.readUint32BE();
	const uint32 length = file.readUint32BE();
	if (length % 40 || uint64(offset) + length > uint64(file.size()) || !file.seek(offset))
		return false;

	// Index the names before mounting the archive. hasFile() must not open
	// the bundle through SearchMan, which will also search this archive.
	for (uint32 i = 0; i < length / 40; ++i) {
		const uint32 start = file.readUint32BE();
		const uint32 size = file.readUint32BE();
		char name[33];
		if (file.read(name, 32) != 32 || uint64(start) + size > uint64(file.size()))
			return false;
		name[32] = 0;
		if (!name[0])
			return false;
		_members.push_back(name);
	}
	return !file.err();
}

bool Rebel2MacArchive::hasFile(const Common::Path &path) const {
	const Common::String name = path.baseName();
	for (const Common::String &member : _members) {
		if (member.equalsIgnoreCase(name))
			return true;
	}
	return false;
}

int Rebel2MacArchive::listMembers(Common::ArchiveMemberList &list) const {
	for (const Common::String &member : _members)
		list.push_back(Common::ArchiveMemberPtr(new Common::GenericArchiveMember(member, *this)));
	return _members.size();
}

const Common::ArchiveMemberPtr Rebel2MacArchive::getMember(const Common::Path &path) const {
	if (!hasFile(path))
		return Common::ArchiveMemberPtr();
	return Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this));
}

Common::SeekableReadStream *Rebel2MacArchive::createReadStreamForMember(const Common::Path &path) const {
	if (!hasFile(path))
		return nullptr;

	ScummFile *file = new ScummFile(_vm);
	if (file->open(_filename) && file->openSubFile(Common::Path(path.baseName())))
		return file;
	delete file;
	return nullptr;
}

Common::Archive *createRebel2MacArchive(const ScummEngine *vm, const Common::Path &filename) {
	Rebel2MacArchive *archive = new Rebel2MacArchive(vm, filename);
	if (archive->load())
		return archive;
	delete archive;
	return nullptr;
}

} // End of namespace Scumm
