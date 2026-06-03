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

#ifndef COMMON_FORMATS_SPECTRUM_TAPE_H
#define COMMON_FORMATS_SPECTRUM_TAPE_H

#include "common/array.h"
#include "common/archive.h"
#include "common/str.h"

namespace Common {

class SeekableReadStream;

struct SpectrumTapeBlock {
	byte id;
	Array<byte> data;
	Array<byte> tap;
};

typedef Array<SpectrumTapeBlock> SpectrumTapeBlocks;

bool parseSpectrumTape(SeekableReadStream &stream, SpectrumTapeBlocks &blocks);

class SpectrumTapeArchive : public Archive {
public:
	SpectrumTapeArchive() {}
	SpectrumTapeArchive(const SpectrumTapeBlocks &blocks);

	void addFile(const Path &path, const Array<byte> &data);

	bool hasFile(const Path &path) const override;
	int listMembers(ArchiveMemberList &list) const override;
	const ArchiveMemberPtr getMember(const Path &path) const override;
	SeekableReadStream *createReadStreamForMember(const Path &path) const override;

private:
	struct ArchiveFile {
		Path name;
		Array<byte> data;
	};

	Array<ArchiveFile> _files;
};

} // End of namespace Common

#endif
