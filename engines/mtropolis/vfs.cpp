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

#include "mtropolis/vfs.h"

#include "common/punycode.h"

namespace MTropolis {

VirtualFileSystemLayout::VirtualFileSystemLayout() : _pathSeparator('/') {
}

VirtualFileSystem::VirtualFileSystem(const VirtualFileSystemLayout &layout) : _pathSeparator(layout._pathSeparator), _workspaceRoot(layout._workspaceRoot) {
	Common::Array<TempLayoutFile> tempLayoutFiles;
	Common::HashMap<Common::String, uint> pathToTLF;
	Common::HashMap<Common::String, Common::String> canonicalPathToVirtualPath;
	Common::HashMap<Common::String, uint> canonicalPathToTLF;

	for (const VirtualFileSystemLayout::ArchiveJunction &arcJunction : layout._archiveJunctions) {
		Common::String prefix = arcJunction._archiveName + _pathSeparator;
		Common::ArchiveMemberList arcMemberList;

		arcJunction._archive->listMembers(arcMemberList);

		for (const Common::ArchiveMemberPtr &arcMember : arcMemberList) {
			TempLayoutFile tlf;
			tlf._archiveMember = arcMember;
			tlf._expandedPath = (prefix + arcMember->getPathInArchive().toString(_pathSeparator));
			tlf._expandedPathCanonical = canonicalizePath(Common::Path(tlf._expandedPath, _pathSeparator));

			Common::HashMap<Common::String, uint>::const_iterator indexIt = pathToTLF.find(tlf._expandedPath);

			if (indexIt != pathToTLF.end())
				tempLayoutFiles[indexIt->_value] = tlf;
			else {
				pathToTLF[tlf._expandedPath] = tempLayoutFiles.size();
				tempLayoutFiles.push_back(tlf);
			}
		}
	}

	for (const VirtualFileSystemLayout::PathJunction &pjunc : layout._pathJunctions) {
		Common::String destPathFile = canonicalizePath(Common::Path(pjunc._destPath, _pathSeparator));
		Common::String destPathDir = destPathFile + _pathSeparator;

		for (uint i = 0; i < tempLayoutFiles.size(); i++) {
			const TempLayoutFile &tlf = tempLayoutFiles[i];

			if (tlf._expandedPathCanonical == destPathFile || tlf._expandedPathCanonical.hasPrefix(destPathDir)) {
				Common::String translatedPath = pjunc._srcPath + tlf._expandedPath.substr(destPathFile.size());

				Common::String canonicalPath = canonicalizePath(Common::Path(translatedPath, _pathSeparator));

				canonicalPathToTLF[canonicalPath] = i;
				canonicalPathToVirtualPath[canonicalPath] = translatedPath;
			}
		}
	}

	for (const Common::String &excl : layout._exclusions) {
		Common::String canonicalPath = canonicalizePath(Common::Path(excl, _pathSeparator));

		canonicalPathToTLF.erase(canonicalPath);
	}

	_virtualFiles.reserve(canonicalPathToTLF.size());

	for (const Common::HashMap<Common::String, uint>::Node &ctofNode : canonicalPathToTLF) {
		Common::HashMap<Common::String, Common::String>::const_iterator ctovIt = canonicalPathToVirtualPath.find(ctofNode._key);

		assert(ctovIt != canonicalPathToVirtualPath.end());

		VirtualFile vf;
		vf._archiveMember = tempLayoutFiles[ctofNode._value]._archiveMember;
		vf._virtualPath = Common::Path(ctovIt->_value, _pathSeparator);

		_virtualFiles.push_back(vf);
	}

	Common::sort(_virtualFiles.begin(), _virtualFiles.end(), sortVirtualFiles);

	for (uint i = 0; i < _virtualFiles.size(); i++) {
		VirtualFile &vf = _virtualFiles[i];

		_pathToVirtualFileIndex[canonicalizePath(vf._virtualPath)] = i;
		vf._vfsArchiveMember = Common::ArchiveMemberPtr(new VFSArchiveMember(&vf, _pathSeparator));
	}
}

bool VirtualFileSystem::hasFile(const Common::Path &path) const {
	return getVirtualFile(path) != nullptr;
}

bool VirtualFileSystem::isPathDirectory(const Common::Path &path) const {
	const VirtualFile *vf = getVirtualFile(path);
	if (vf)
		return vf->_archiveMember->isDirectory();

	return false;
}

int VirtualFileSystem::listMembers(Common::ArchiveMemberList &list) const {
	int numMembers = 0;
	for (const VirtualFile &vf : _virtualFiles) {
		list.push_back(vf._vfsArchiveMember);
		numMembers++;
	}

	return numMembers;
}

const Common::ArchiveMemberPtr VirtualFileSystem::getMember(const Common::Path &path) const {
	const VirtualFile *vf = getVirtualFile(path);
	if (vf)
		return vf->_vfsArchiveMember;

	return nullptr;
}

Common::SeekableReadStream *VirtualFileSystem::createReadStreamForMember(const Common::Path &path) const {
	const VirtualFile *vf = getVirtualFile(path);
	if (vf)
		return vf->_archiveMember->createReadStream();

	return nullptr;
}

Common::SeekableReadStream *VirtualFileSystem::createReadStreamForMemberAltStream(const Common::Path &path, Common::AltStreamType altStreamType) const {
	const VirtualFile *vf = getVirtualFile(path);
	if (vf)
		return vf->_archiveMember->createReadStreamForAltStream(altStreamType);

	return nullptr;
}

char VirtualFileSystem::getPathSeparator() const {
	return _pathSeparator;
}

int VirtualFileSystem::listMatchingMembers(Common::ArchiveMemberList &list, const Common::Path &pattern, bool matchPathComponents) const {
	Common::ArchiveMemberList allNames;
	listMembers(allNames);

	Common::String patternString = pattern.toString(getPathSeparator());
	int matches = 0;

	char pathSepString[2] = {getPathSeparator(), '\0'};

	const char *wildcardExclusions = matchPathComponents ? NULL : pathSepString;

	Common::ArchiveMemberList::const_iterator it = allNames.begin();
	for (; it != allNames.end(); ++it) {
		if ((*it)->getPathInArchive().toString(_pathSeparator).matchString(patternString, true, wildcardExclusions)) {
			list.push_back(*it);
			matches++;
		}
	}

	return matches;
}

const VirtualFileSystem::VirtualFile *VirtualFileSystem::getVirtualFile(const Common::Path &path) const {
	Common::HashMap<Common::String, uint>::const_iterator it = _pathToVirtualFileIndex.find(canonicalizePath(path));

	if (it == _pathToVirtualFileIndex.end())
		return nullptr;

	return &_virtualFiles[it->_value];
}

bool VirtualFileSystem::sortVirtualFiles(const VirtualFile &a, const VirtualFile &b) {
	return a._virtualPath < b._virtualPath;
}

Common::String VirtualFileSystem::canonicalizePath(const Common::Path &path) const {
	Common::StringArray components = path.splitComponents();

	for (Common::String &component : components) {
		if (Common::punycode_hasprefix(component))
			component = Common::punycode_decode(component).encode(Common::kUtf8);
	}

	Common::String result = Common::Path::joinComponents(components).toString(_pathSeparator);
	result.toLowercase();

	return result;
}

VirtualFileSystem::VFSArchiveMember::VFSArchiveMember(const VirtualFile *virtualFile, char pathSeparator) : _virtualFile(virtualFile), _pathSeparator(pathSeparator) {
}

Common::SeekableReadStream *VirtualFileSystem::VFSArchiveMember::createReadStream() const {
	return _virtualFile->_archiveMember->createReadStream();
}

Common::SeekableReadStream *VirtualFileSystem::VFSArchiveMember::createReadStreamForAltStream(Common::AltStreamType altStreamType) const {
	return _virtualFile->_archiveMember->createReadStreamForAltStream(altStreamType);
}

Common::String VirtualFileSystem::VFSArchiveMember::getName() const {
	return _virtualFile->_virtualPath.toString(_pathSeparator);
}

Common::Path VirtualFileSystem::VFSArchiveMember::getPathInArchive() const {
	return _virtualFile->_virtualPath;
}

Common::String VirtualFileSystem::VFSArchiveMember::getFileName() const {
	return _virtualFile->_archiveMember->getFileName();
}

bool VirtualFileSystem::VFSArchiveMember::isDirectory() const {
	return _virtualFile->_archiveMember->isDirectory();
}

void VirtualFileSystem::VFSArchiveMember::listChildren(Common::ArchiveMemberList &childList, const char *pattern) const {
	return _virtualFile->_archiveMember->listChildren(childList, pattern);
}

Common::U32String VirtualFileSystem::VFSArchiveMember::getDisplayName() const {
	return _virtualFile->_archiveMember->getDisplayName();
}

bool VirtualFileSystem::VFSArchiveMember::isInMacArchive() const {
	return _virtualFile->_archiveMember->isInMacArchive();
}

} // End of namespace MTropolis
