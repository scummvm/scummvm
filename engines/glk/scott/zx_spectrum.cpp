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

/*
 * Based on ScottFree interpreter version 1.14 developed by Swansea
 * University Computer Society without disassembly of any other game
 * drivers, only of game databases as permitted by EEC law (for purposes
 * of compatibility).
 *
 * Licensed under GPLv2
 *
 * https://github.com/angstsmurf/spatterlight/tree/master/terps/scott
 */

#include "common/formats/spectrum_tape.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "glk/scott/zx_spectrum.h"

namespace Glk {
namespace Scott {

bool extractZXSpectrumTapeData(Common::SeekableReadStream &stream, ZXSpectrumTapeData &tape) {
	Common::SpectrumTapeBlocks blocks;
	if (!Common::parseSpectrumTape(stream, blocks))
		return false;

	Common::SpectrumTapeArchive archive(blocks);
	Common::ArchiveMemberList files;
	archive.listMembers(files);

	tape.screen.clear();
	tape.code.clear();
	for (const Common::ArchiveMemberPtr &file : files) {
		Common::ScopedPtr<Common::SeekableReadStream> payload(file->createReadStream());
		if (!payload)
			continue;

		Common::Array<byte> data;
		data.resize(payload->size());
		if (!data.empty() && payload->read(data.data(), data.size()) != data.size())
			return false;

		if (file->getPathInArchive().baseName().hasSuffixIgnoreCase(".scr"))
			tape.screen = data;
		else if (data.size() >= tape.code.size())
			tape.code = data;
	}

	return !tape.code.empty();
}

Common::String computeZXSpectrumTapeCodeMD5(Common::SeekableReadStream &stream, uint32 *codeSize) {
	ZXSpectrumTapeData tape;
	if (!extractZXSpectrumTapeData(stream, tape))
		return Common::String();

	if (codeSize)
		*codeSize = tape.code.size();

	Common::MemoryReadStream codeStream(tape.code.data(), tape.code.size());
	return Common::computeStreamMD5AsString(codeStream);
}

} // End of namespace Scott
} // End of namespace Glk
