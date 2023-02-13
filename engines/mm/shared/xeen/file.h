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

#ifndef MM_SHARED_XEEN_FILE_H
#define MM_SHARED_XEEN_FILE_H

#include "common/file.h"
#include "common/serializer.h"

namespace MM {
namespace Shared {
namespace Xeen {


/**
 * Derived file class
 */
class File : public Common::File {
private:
public:
#ifdef ENABLE_XEEN
	/**
	 * Sets which archive is used by default
	 */
	static void setCurrentArchive(int ccMode);
#endif

	/**
	 * Synchronizes a boolean array as a bitfield set
	 */
	static void syncBitFlags(Common::Serializer &s, bool *startP, bool *endP);
public:
	File() : Common::File() {
	}
	File(const Common::String &filename);
	File(const Common::String &filename, Common::Archive &archive);
#ifdef ENABLE_XEEN
	File(const Common::String &filename, int ccMode);
#endif
	~File() override {}

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	bool open(const Common::Path &filename) override;

	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	bool open(const Common::Path &filename, Common::Archive &archive) override;
#ifdef ENABLE_XEEN
	/**
	 * Opens the given file, throwing an error if it can't be opened
	 */
	virtual bool open(const Common::String &filename, int ccMode);
#endif

	/**
	 * Opens the given file
	 */
	bool open(const Common::FSNode &node) override {
		return Common::File::open(node);
	}

	/**
	 * Opens the given file
	 */
	bool open(SeekableReadStream *stream, const Common::String &name) override {
		return Common::File::open(stream, name);
	}

	/**
	 * Reads in a null terminated string
	 */
	Common::String readString();

	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename);

#ifdef ENABLE_XEEN
	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @param	ccMode		Archive to use
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename, int ccMode);
#endif

	/**
	 * Checks if a given file exists
	 *
	 * @param	filename	the file to check for
	 * @param	archive		Archive to use
	 * @return	true if the file exists, false otherwise
	 */
	static bool exists(const Common::String &filename, Common::Archive &archive);
};

} // namespace Xeen
} // namespace Shared
} // namespace MM

#endif
