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

#ifndef MTROPOLIS_VFS_H
#define MTROPOLIS_VFS_H

#include "common/archive.h"
#include "common/hash-str.h"

namespace MTropolis {

struct VirtualFileSystemLayout {
	VirtualFileSystemLayout();

	struct PathJunction {
		Common::String _srcPath;
		Common::String _destPath;
	};

	struct ArchiveJunction {
		Common::String _archiveName;
		Common::Archive *_archive;
	};

	char _pathSeparator;
	Common::Path _workspaceRoot;

	Common::Array<PathJunction> _pathJunctions;
	Common::Array<ArchiveJunction> _archiveJunctions;
	Common::Array<Common::String> _exclusions;
};

class VirtualFileSystem : public Common::Archive {
public:
	explicit VirtualFileSystem(const VirtualFileSystemLayout &layout);

	bool hasFile(const Common::Path &path) const override;
	bool isPathDirectory(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SeekableReadStream *createReadStreamForMember(const Common::Path &path) const override;

	Common::SeekableReadStream *createReadStreamForMemberAltStream(const Common::Path &path, Common::AltStreamType altStreamType) const override;

	int listMatchingMembers(Common::ArchiveMemberList &list, const Common::Path &pattern, bool matchPathComponents = false) const override;

	char getPathSeparator() const override;

private:
	struct VirtualFile {
		Common::Path _virtualPath;
		Common::ArchiveMemberPtr _archiveMember;

		Common::ArchiveMemberPtr _vfsArchiveMember;
	};

	struct TempLayoutFile {
		Common::String _expandedPath;
		Common::String _expandedPathCanonical;
		Common::ArchiveMemberPtr _archiveMember;
	};

	class VFSArchiveMember : public Common::ArchiveMember {
	public:
		VFSArchiveMember(const VirtualFile *virtualFile, char pathSeparator);

		Common::SeekableReadStream *createReadStream() const override;
		Common::SeekableReadStream *createReadStreamForAltStream(Common::AltStreamType altStreamType) const override;

		Common::String getName() const override;

		Common::Path getPathInArchive() const override;
		Common::String getFileName() const override;
		bool isDirectory() const override;
		void listChildren(Common::ArchiveMemberList &childList, const char *pattern) const override;
		Common::U32String getDisplayName() const override;
		bool isInMacArchive() const override;

	private:
		const VirtualFile *_virtualFile;
		char _pathSeparator;
	};

	const VirtualFile *getVirtualFile(const Common::Path &path) const;
	static bool sortVirtualFiles(const VirtualFile &a, const VirtualFile &b);

	Common::String canonicalizePath(const Common::Path &path) const;

	char _pathSeparator;
	Common::Path _workspaceRoot;

	Common::Array<VirtualFile> _virtualFiles;
	Common::HashMap<Common::String, uint> _pathToVirtualFileIndex;
};

} // End of namespace MTropolis

#endif
