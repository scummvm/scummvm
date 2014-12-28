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

#ifndef STARK_ARCHIVE_LOADER_H
#define STARK_ARCHIVE_LOADER_H

#include "common/list.h"
#include "common/str.h"
#include "common/util.h"

#include "engines/stark/archive.h"
#include "engines/stark/resources/resource.h"

namespace Stark {

class Level;
class Location;

/**
 * XARC Archive loader.
 *
 * Maintains a list of opened archive files.
 * Loads the resources from the XRC tree.
 */
class ArchiveLoader {

public:
	~ArchiveLoader();

	/** Load a Xarc archive, and add it to the managed archives list */
	bool load(const Common::String &archiveName);

	/** Unload all the unused Xarc archives */
	void unloadUnused();

	/** Retrieve a file from a specified archive */
	Common::ReadStream *getFile(const Common::String &fileName, const Common::String &archiveName);

	/** Get the resource tree root for an archive, and increment the archive use count */
	template <class T>
	T *useRoot(const Common::String &archiveName);

	/** Decrement the root's archive use count */
	bool returnRoot(const Common::String &archiveName);

	/** Build the archive filename for a level or a location */
	Common::String buildArchiveName(Level *level, Location *location = nullptr);

private:
	class LoadedArchive {
	public:
		LoadedArchive(const Common::String &archiveName);
		~LoadedArchive();

		Common::String &getFilename() { return _filename; }
		XARCArchive &getXArc() { return _xarc; }
		Resource *getRoot() { return _root; }

		bool isInUse() { return _useCount > 0; }
		void incUsage() { _useCount++; }
		void decUsage() { _useCount = MAX<int>(_useCount - 1, 0); }

	private:
		Resource *importResources();

		uint _useCount;
		Common::String _filename;
		XARCArchive _xarc;
		Resource *_root;
	};

	typedef Common::List<LoadedArchive *> LoadedArchiveList;

	bool hasArchive(const Common::String &archiveName);
	LoadedArchive *findArchive(const Common::String &archiveName);

	LoadedArchiveList _archives;
};

template <class T>
T *ArchiveLoader::useRoot(const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	archive->incUsage();

	Resource *root = archive->getRoot();

	if (root->getType() != T::TYPE) {
		error("Wrong root type for archive '%s' found '%s', expected '%s'",
				archiveName.c_str(), root->getType().getName(), ResourceType(T::TYPE).getName());
	}

	return (T *)root;
}

template <>
Resource *ArchiveLoader::useRoot<Resource>(const Common::String &archiveName);

} // End of namespace Stark

#endif // STARK_ARCHIVE_LOADER_H
