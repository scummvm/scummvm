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

//=============================================================================
//
// SpriteFile class handles sprite file loading and streaming.
//
//=============================================================================

#ifndef AGS_SHARED_AC_SPRITE_FILE_H
#define AGS_SHARED_AC_SPRITE_FILE_H

#include "ags/shared/core/types.h"
#include "ags/lib/std/vector.h"
#include "ags/globals.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Bitmap;

// TODO: research old version differences
enum SpriteFileVersion {
	kSprfVersion_Uncompressed = 4,
	kSprfVersion_Compressed = 5,
	kSprfVersion_Last32bit = 6,
	kSprfVersion_64bit = 10,
	kSprfVersion_HighSpriteLimit = 11,
	kSprfVersion_Current = kSprfVersion_HighSpriteLimit
};

enum SpriteIndexFileVersion {
	kSpridxfVersion_Initial = 1,
	kSpridxfVersion_Last32bit = 2,
	kSpridxfVersion_64bit = 10,
	kSpridxfVersion_HighSpriteLimit = 11,
	kSpridxfVersion_Current = kSpridxfVersion_HighSpriteLimit
};

typedef int32_t sprkey_t;

// SpriteFileIndex contains sprite file's table of contents
struct SpriteFileIndex {
	int SpriteFileIDCheck = 0; // tag matching sprite file and index file
	sprkey_t LastSlot = -1;
	size_t SpriteCount = 0u;
	std::vector<int16_t> Widths;
	std::vector<int16_t> Heights;
	std::vector<soff_t>  Offsets;
};


class SpriteFile {
public:
	// Standart sprite file and sprite index names
	static const char *DefaultSpriteFileName;
	static const char *DefaultSpriteIndexName;

	SpriteFile();
	// Loads sprite reference information and inits sprite stream
	HError      OpenFile(const String &filename, const String &sprindex_filename,
		std::vector<Size> &metrics);
	void        Reset();

	// Tells if bitmaps in the file are compressed
	bool        IsFileCompressed() const;
	// Tells the highest known sprite index
	sprkey_t    GetTopmostSprite() const;

	// Loads sprite index file
	bool        LoadSpriteIndexFile(const String &filename, int expectedFileID,
		soff_t spr_initial_offs, sprkey_t topmost, std::vector<Size> &metrics);
	// Rebuilds sprite index from the main sprite file
	HError      RebuildSpriteIndex(Stream *in, sprkey_t topmost, SpriteFileVersion vers,
		std::vector<Size> &metrics);

	HError      LoadSprite(sprkey_t index, Bitmap *&sprite);
	HError      LoadSpriteData(sprkey_t index, Size &metric, int &bpp, std::vector<char> &data);

	// Saves all sprites to file; fills in index data for external use
	// TODO: refactor to be able to save main file and index file separately (separate function for gather data?)
	static int  SaveToFile(const String &save_to_file,
		const std::vector<Bitmap *> &sprites, // available sprites (may contain nullptrs)
		SpriteFile *read_from_file, // optional file to read missing sprites from
		bool compressOutput, SpriteFileIndex &index);
	// Saves sprite index table in a separate file
	static int  SaveSpriteIndex(const String &filename, const SpriteFileIndex &index);

private:
	// Finds the topmost occupied slot index. Warning: may be slow.
	static sprkey_t FindTopmostSprite(const std::vector<Bitmap *> &sprites);
	// Seek stream to sprite
	void        SeekToSprite(sprkey_t index);

	// Internal sprite reference
	struct SpriteRef {
		soff_t Offset = 0; // data offset
		size_t Size = 0;   // cache size of element, in bytes
	};

	// Array of sprite references
	std::vector<SpriteRef> _spriteData;
	std::unique_ptr<Stream> _stream; // the sprite stream
	bool _compressed; // are sprites compressed
	sprkey_t _curPos; // current stream position (sprite slot)
};

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
