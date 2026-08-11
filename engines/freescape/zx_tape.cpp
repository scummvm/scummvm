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

#include "freescape/zx_tape.h"

#include "common/file.h"
#include "common/formats/spectrum_tape.h"
#include "common/fs.h"
#include "common/md5.h"
#include "common/memstream.h"
#include "engines/advancedDetector.h"

namespace Freescape {

bool extractZxSpectrumTapeFiles(Common::SeekableReadStream &stream, const char *prefix, ZxTapeFileList &files) {
	files.clear();

	Common::SpectrumTapeBlocks blocks;
	if (!Common::parseSpectrumTape(stream, blocks))
		return false;

	Common::Array<byte> title;
	Common::Array<byte> border;
	Common::Array<byte> data;

	for (const Common::SpectrumTapeBlock &block : blocks) {
		if (block.tap.size() >= 2) {
			Common::Array<byte> body;
			body.assign(block.tap.begin() + 1, block.tap.end() - 1);
			if (body.size() == 6912) {
				title = border;
				border = body;
			} else if (body.size() >= data.size()) {
				data = body;
			}
		}
	}

	const struct {
		const char *suffix;
		const Common::Array<byte> &payload;
	} outputs[] = {
		{ "title", title },
		{ "border", border },
		{ "data", data }
	};
	for (uint i = 0; i < ARRAYSIZE(outputs); ++i) {
		if (!outputs[i].payload.empty()) {
			ZxTapeFile file;
			file.name = Common::Path(Common::String::format("%s.zx.%s", prefix, outputs[i].suffix), Common::Path::kNoSeparator);
			file.data = outputs[i].payload;
			files.push_back(file);
		}
	}

	return !data.empty();
}

bool matchZxSpectrumTapeFiles(const ZxTapeFileList &files, const ADGameDescription &desc, uint md5Bytes) {
	bool matched = desc.platform == Common::kPlatformZX;

	for (const ADGameFileDescription *fileDesc = desc.filesDescriptions; matched && fileDesc->fileName; ++fileDesc) {
		bool fileMatched = false;
		Common::Path fileName(fileDesc->fileName, Common::Path::kNoSeparator);
		for (uint i = 0; !fileMatched && i < files.size(); ++i) {
			fileMatched = files[i].name.equalsIgnoreCase(fileName) &&
				(fileDesc->fileSize == AD_NO_SIZE || fileDesc->fileSize == files[i].data.size());
			if (fileMatched && fileDesc->md5) {
				Common::MemoryReadStream stream(files[i].data.data(), files[i].data.size());
				fileMatched = Common::computeStreamMD5AsString(stream, md5Bytes) == fileDesc->md5;
			}
		}
		matched = fileMatched;
	}

	return matched;
}

Common::Archive *makeZxSpectrumTapeArchive(const ADGameDescription &desc, const Common::Path &gamePath) {
	Common::Archive *archive = nullptr;

	Common::FSList files;
	if (desc.platform == Common::kPlatformZX && Common::FSNode(gamePath).getChildren(files, Common::FSNode::kListFilesOnly)) {
		for (const Common::FSNode &node : files) {
			Common::File file;
			Common::String name = node.getName();
			if ((name.hasSuffixIgnoreCase(".tap") || name.hasSuffixIgnoreCase(".tzx")) && file.open(node)) {
				ZxTapeFileList tapeFiles;
				if (extractZxSpectrumTapeFiles(file, desc.gameId, tapeFiles) && matchZxSpectrumTapeFiles(tapeFiles, desc)) {
					Common::SpectrumTapeArchive *tapeArchive = new Common::SpectrumTapeArchive();
					for (const ZxTapeFile &tapeFile : tapeFiles)
						tapeArchive->addFile(tapeFile.name, tapeFile.data);
					archive = tapeArchive;
					break;
				}
			}
		}
	}

	return archive;
}

} // End of namespace Freescape
