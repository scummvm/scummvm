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
#include "common/compression/installshield_cab.h"
#include "common/fs.h"
#include "common/ptr.h"
#include "common/stream.h"
#include "hollywood/runaway/archive.h"

namespace Hollywood {
namespace {

class RunawayInstallerArchive : public Common::MemcachingCaseInsensitiveArchive {
public:
	RunawayInstallerArchive(Common::Archive *cabinet, const Common::FSNode &text, const Common::FSNode &lipSync) :
			MemcachingCaseInsensitiveArchive(128 * 1024 * 1024), _cabinet(cabinet), _text(text), _lipSync(lipSync),
			_hasLipSync(lipSync.exists() && !lipSync.isDirectory()) {}

	bool hasFile(const Common::Path &path) const override {
		return path.equalsIgnoreCase("RESOURCE.003") || (_hasLipSync && path.equalsIgnoreCase("RESOURCE.004")) || _cabinet->hasFile(path);
	}

	int listMembers(Common::ArchiveMemberList &list) const override {
		Common::ArchiveMemberList members;
		_cabinet->listMembers(members);
		for (const auto &member : members)
			list.push_back(getMember(member->getPathInArchive()));
		list.push_back(getMember("RESOURCE.003"));
		if (_hasLipSync && !_cabinet->hasFile("RESOURCE.004")) {
			list.push_back(getMember("RESOURCE.004"));
			return members.size() + 2;
		}
		return members.size() + 1;
	}

	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override {
		return hasFile(path) ? Common::ArchiveMemberPtr(new Common::GenericArchiveMember(path, *this)) :
			Common::ArchiveMemberPtr();
	}

	Common::SharedArchiveContents readContentsForPath(const Common::Path &path) const override {
		// The installer renames the loose text and optional lip-sync files.
		const bool lipSync = _hasLipSync && path.equalsIgnoreCase("RESOURCE.004");
		if (path.equalsIgnoreCase("RESOURCE.003") || lipSync) {
			Common::SeekableReadStream *stream = (lipSync ? _lipSync : _text).createReadStream();
			return stream ? Common::SharedArchiveContents::bypass(stream) : Common::SharedArchiveContents();
		}

		Common::ScopedPtr<Common::SeekableReadStream> stream(_cabinet->createReadStreamForMember(path));
		if (!stream || stream->size() < 0 || stream->size() > UINT32_MAX)
			return Common::SharedArchiveContents();
		// Graphics and speech repeatedly reopen the same large cabinet members.
		// Retain the decompressed data in memory, shared by independent seekable streams.
		const uint32 size = stream->size();
		byte *data = new byte[size];
		if (stream->read(data, size) != size || stream->err()) {
			delete[] data;
			return Common::SharedArchiveContents();
		}
		return Common::SharedArchiveContents(data, size);
	}

private:
	Common::ScopedPtr<Common::Archive> _cabinet;
	Common::FSNode _text, _lipSync;
	bool _hasLipSync;
};

} // End of anonymous namespace

Common::Archive *makeRunawayInstallerArchive(const Common::FSNode &directory) {
	const Common::FSNode text = directory.getChild("data2.ps");
	if (!text.exists() || text.isDirectory())
		return nullptr;
	Common::Archive *cabinet = Common::makeInstallShieldArchive(directory.getChild("data1.cab"));
	return cabinet ? new RunawayInstallerArchive(cabinet, text, directory.getChild("data3.ps")) : nullptr;
}

} // End of namespace Hollywood
