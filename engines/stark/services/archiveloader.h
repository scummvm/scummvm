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

#ifndef STARK_SERVICES_ARCHIVE_LOADER_H
#define STARK_SERVICES_ARCHIVE_LOADER_H

#include "common/list.h"
#include "common/str.h"
#include "common/substream.h"
#include "common/util.h"

#include "math/quat.h"
#include "math/vector3d.h"

#include "engines/stark/formats/xarc.h"
#include "engines/stark/resources/object.h"

namespace Stark {

namespace Resources {
class Level;
class Location;
}

/**
 * A read stream with helper functions to read usual data types
 */
class ArchiveReadStream : public Common::SeekableSubReadStream {
public:
	ArchiveReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	virtual ~ArchiveReadStream();

	Common::String readString();
	Common::String readString16();
	Math::Vector3d readVector3();
	Math::Quaternion readQuaternion();
	float readFloat();
};

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
	ArchiveReadStream *getFile(const Common::String &fileName, const Common::String &archiveName);

	/** Get the resource tree root for an archive, and increment the archive use count */
	template <class T>
	T *useRoot(const Common::String &archiveName);

	/** Decrement the root's archive use count */
	bool returnRoot(const Common::String &archiveName);

	/** Build the archive filename for a level or a location */
	Common::String buildArchiveName(Resources::Level *level, Resources::Location *location = nullptr) const;

	/** Retrieve a file relative to a specified archive */
	Common::SeekableReadStream *getExternalFile(const Common::String &fileName, const Common::String &archiveName) const;
	Common::String getExternalFilePath(const Common::String &fileName, const Common::String &archiveName) const;

private:
	class LoadedArchive {
	public:
		explicit LoadedArchive(const Common::String &archiveName);
		~LoadedArchive();

		const Common::String &getFilename() const { return _filename; }
		const Formats::XARCArchive &getXArc() const { return _xarc; }
		Resources::Object *getRoot() const { return _root; }

		void importResources();

		bool isInUse() const { return _useCount > 0; }
		void incUsage() { _useCount++; }
		void decUsage() { _useCount = MAX<int>(_useCount - 1, 0); }

	private:
		uint _useCount;
		Common::String _filename;
		Formats::XARCArchive _xarc;
		Resources::Object *_root;
	};

	typedef Common::List<LoadedArchive *> LoadedArchiveList;

	bool hasArchive(const Common::String &archiveName) const;
	LoadedArchive *findArchive(const Common::String &archiveName) const;

	LoadedArchiveList _archives;
};

template <class T>
T *ArchiveLoader::useRoot(const Common::String &archiveName) {
	LoadedArchive *archive = findArchive(archiveName);
	archive->incUsage();
	return Resources::Object::cast<T>(archive->getRoot());
}

} // End of namespace Stark

#endif // STARK_SERVICES_ARCHIVE_LOADER_H
