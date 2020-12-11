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

#ifndef ULTIMA_SHARED_CORE_FILE_H
#define ULTIMA_SHARED_CORE_FILE_H

#include "common/file.h"
#include "common/str.h"

namespace Ultima {
namespace Shared {

/**
 * Simple ScummVM File descendent that throws a wobbly if the file it tries to open isn't present
 */
class File : public Common::File {
private:
	int32 _filesize;
public:
	File() : Common::File(), _filesize(-1) {}
	File(const Common::String &name);

	/**
	 * Open the file with the given filename, by searching SearchMan.
	 * @param	name	the name of the file to open
	 */
	bool open(const Common::String &name) override;

	/**
	 * Open the file with the given filename from within the given archive.
	 * @param	name	the name of the file to open
	 * @param	archive		the archive in which to search for the file
	 */
	bool open(const Common::String &name, Common::Archive &archive) override;

	/**
	 * Open the file corresponding to the give node.
	 * @param   node        the node to consider.
	 */
	bool open(const Common::FSNode &node) override;

	/**
	 * 'Open' the given stream. That is, we just wrap around it
	 * @param	stream		a pointer to a SeekableReadStream, or 0
	 * @param	name		a string describing the 'file' corresponding to stream
	 */
	bool open(SeekableReadStream *stream, const Common::String &name) override;

	/**
	 * Close the currently open file
	 */
	void close() override;

	/**
	 * Differing eof that returns true when pos == size as well as beyond
	 */
	bool eof();
};

} // End of namespace Shared
} // End of namespace Ultima

#endif
