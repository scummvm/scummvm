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

#ifndef HOLLYWOOD_IMAGE_TRANSLATION_H
#define HOLLYWOOD_IMAGE_TRANSLATION_H

#include "common/archive.h"
#include "common/hash-str.h"
#include "common/ptr.h"

#include "hollywood/resource.h"

namespace Hollywood {

// Applies extracted image overlays while preserving the original files on disk.
class ImageTranslationArchive : public Common::MemcachingCaseInsensitiveArchive {
public:
	ImageTranslationArchive(Common::Archive *archive);
	bool load(const Common::Path &originalTextArchive);

	bool hasFile(const Common::Path &path) const override;
	int listMembers(Common::ArchiveMemberList &list) const override;
	const Common::ArchiveMemberPtr getMember(const Common::Path &path) const override;
	Common::SharedArchiveContents readContentsForPath(const Common::Path &path) const override;

private:
	enum Layout {
		kBlocks,
		kPixels,
		kPalette
	};

	struct Overlay {
		Common::String source;
		Common::String baseName;
		byte chunk;
		Layout layout;
		byte paletteChunk;
		uint16 x, y;
	};

	bool readOriginalChunk(Common::SeekableReadStream &file, const ResourceChunkTable &table,
		uint index, Common::Array<byte> &data) const;
	void remapPalette(Common::Array<byte> &pixels, const Common::Array<byte> &originalPalette,
		const byte *translatedPalette) const;
	bool applyImage(const Overlay &overlay, Common::SeekableReadStream &file,
		const ResourceChunkTable &table, Common::Array<byte> &data) const;

	Common::Archive *_archive; // Owned by TextTranslation.
	Common::StringMap _sources;
	Common::Array<Overlay> _overlays;
};

} // End of namespace Hollywood

#endif // HOLLYWOOD_IMAGE_TRANSLATION_H
