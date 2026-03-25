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

/**
 * @file iso9660.h
 * ISO 9660 / High Sierra filesystem reader.
 *
 * Reads files and directories from an ISO 9660 image, or from the data track
 * of an MDS/MDF or CUE/BIN image presented as 2048-byte logical sectors.
 *
 * Supports ISO 9660 Level 1/2, Joliet (Unicode) extensions, and the older
 * High Sierra Format (HSF / ECMA-119:1986) used by some early CD-ROM titles.
 * Rock Ridge extensions are not implemented.
 */

#ifndef COMMON_FORMATS_ISO9660_H
#define COMMON_FORMATS_ISO9660_H

#include "common/array.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/stream.h"

namespace Common {

/**
 * Low-level ISO 9660 filesystem reader.
 *
 * Construct with a SeekableReadStream over the raw sector data.
 * For a plain .iso file the stream IS the file.  For an MDS/MDF image,
 * strip the 16-byte sync header from each raw sector first and pass the
 * resulting 2048-byte-per-sector stream.
 */
class ISO9660FileSystem {
public:
	/**
	 * A single directory entry exposed by the ISO filesystem.
	 */
	struct DirEntry {
		bool   isDir; ///< True if this is a directory.
		String name;  ///< Filename (lowercase, Joliet Unicode name preferred over 8.3 ISO name, version suffix stripped).
		uint32 lba;   ///< Logical Block Address of the extent.
		uint32 size;  ///< Size of the file or directory data in bytes.
	};

	/**
	 * Construct an ISO9660FileSystem from a raw ISO image stream.
	 *
	 * @param stream  Seekable stream of raw 2048-byte sectors. The filesystem
	 *                takes ownership of the pointer.
	 */
	explicit ISO9660FileSystem(SeekableReadStream *stream);

	~ISO9660FileSystem();

	/** Returns true if the ISO was opened and parsed successfully. */
	bool isOpen() const;

	/** Returns the LBA of the root directory (Joliet root if available). */
	uint32 getRootLba() const  { return _nameEncoding == kNameJoliet ? _jolietRootLba  : _rootLba;  }

	/** Returns the byte size of the root directory extent. */
	uint32 getRootSize() const { return _nameEncoding == kNameJoliet ? _jolietRootSize : _rootSize; }

	/** Read and return all entries in the root directory. */
	Array<DirEntry> readRootDirectory() const;

	/** Return a DirEntry representing the root directory. */
	DirEntry getRootEntry() const;

	/** Read and return all entries in the given directory. */
	Array<DirEntry> readDirectory(const DirEntry &dir) const;

	/**
	 * Open a file as a SeekableReadStream.
	 *
	 * The returned stream is safe to use concurrently with other streams
	 * opened from the same ISO9660FileSystem.
	 * The caller takes ownership of the returned stream.
	 *
	 * @return A new SeekableReadStream, or @c nullptr if entry.isDir is
	 *         true or on error.
	 */
	SeekableReadStream *openFile(const DirEntry &entry) const;

private:
	enum NameEncoding { kNameISO9660, kNameJoliet };

	Array<DirEntry> readDirectory(uint32 lba, uint32 size) const;
	bool readSector(uint32 lba, byte *buf) const;
	bool parsePVD();
	bool scanForJoliet();
	Array<DirEntry> parseDir(uint32 lba, uint32 size, NameEncoding encoding) const;
	static String jolietToString(const byte *data, uint8 len);
	static String iso9660ToLower(const byte *data, uint8 len);

	ScopedPtr<SeekableReadStream> _stream;
	uint32 _rootLba,       _rootSize;
	uint32 _jolietRootLba, _jolietRootSize;
	NameEncoding _nameEncoding;
	bool   _hsf;  ///< True if using High Sierra Format instead of ISO 9660
};

} // namespace Common

#endif // COMMON_FORMATS_ISO9660_H
