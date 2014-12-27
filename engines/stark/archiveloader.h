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

#include "common/array.h"
#include "common/str.h"

#include "engines/stark/archive.h"

namespace Stark {

class Resource;

/**
 * XARC Archive loader.
 *
 * Maintains a list of opened archive files.
 * Loads the resources from the XRC tree.
 */
class ArchiveLoader {

public:
	/** Load a Xarc archive, and add it to the managed archives list */
	void load(const Common::String &archiveName);

	/** Unload a Xarc archive, and add it to the managed archives list */
	void unload(const Common::String &archiveName);

	/** Retrieve a file from a specified archive */
	Common::ReadStream *getFile(const Common::String &fileName, const Common::String &archiveName);

	/** Get the resource tree root for an archive */
	Resource *getRoot(const Common::String &archiveName);

private:
	class LoadedArchive {
	public:
		LoadedArchive(const Common::String &archiveName);
		~LoadedArchive();

		Common::String &getFilename() { return _filename; }
		XARCArchive &getXArc() { return _xarc; }
		Resource *getRoot() { return _root; }

	private:
		Resource *importResources();

		Common::String _filename;
		XARCArchive _xarc;
		Resource *_root;
	};

	bool hasArchive(const Common::String &archiveName);
	LoadedArchive &findArchive(const Common::String &archiveName);

	Common::Array<LoadedArchive> _archives;
};

} // End of namespace Stark

#endif // STARK_ARCHIVE_LOADER_H
