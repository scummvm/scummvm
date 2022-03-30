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
// SpriteFile class handles sprite file parsing and streaming sprites.
// SpriteFileWriter manages writing sprites into the output stream one by one,
// accumulating index information, and may therefore be suitable for a variety
// of situations.
//
//=============================================================================

#ifndef AGS_SHARED_AC_SPRITE_FILE_H
#define AGS_SHARED_AC_SPRITE_FILE_H

#include "ags/shared/core/types.h"
#include "ags/lib/std/memory.h"
#include "ags/lib/std/vector.h"
#include "ags/shared/util/stream.h"
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
	std::vector<int16_t> Widths;
	std::vector<int16_t> Heights;
	std::vector<soff_t>  Offsets;

	inline size_t GetCount() const { return Offsets.size(); }
	inline sprkey_t GetLastSlot() const { return (sprkey_t)GetCount() - 1; }
};

// SpriteFile opens a sprite file for reading, reports general information,
// and lets read sprites in any order.
class SpriteFile {
public:
	// Standart sprite file and sprite index names
	static const char *DefaultSpriteFileName;
	static const char *DefaultSpriteIndexName;

	SpriteFile();
	// Loads sprite reference information and inits sprite stream
	HError      OpenFile(const String &filename, const String &sprindex_filename,
		std::vector<Size> &metrics);
	// Closes stream; no reading will be possible unless opened again
	void        Close();

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

	// Loads an image data and creates a ready bitmap
	HError      LoadSprite(sprkey_t index, Bitmap *&sprite);
	// Loads an image data into the buffer, reports the bitmap metrics and color depth
	HError      LoadSpriteData(sprkey_t index, Size &metric, int &bpp, std::vector<uint8_t> &data);

private:
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

// SpriteFileWriter class writes a sprite file in a requested format.
// Start using it by calling Begin, write ready bitmaps or copy raw sprite data
// over slot by slot, then call Finalize to let it close the format correctly.
class SpriteFileWriter {
public:
	SpriteFileWriter(std::unique_ptr<Stream> &out);
	~SpriteFileWriter() {}

    // Get the sprite index, accumulated after write
    const SpriteFileIndex &GetIndex() const { return _index; }

    // Initializes new sprite file format
    void Begin(bool compress, sprkey_t last_slot = -1);
    // Writes a bitmap into file, compressing if necessary
    void WriteBitmap(Bitmap *image);
    // Writes an empty slot marker
    void WriteEmptySlot();
    // Writes a raw sprite data without additional processing
	void WriteSpriteData(const uint8_t *pbuf, size_t len, int w, int h, int bpp);
    // Finalizes current format; no further writing is possible after this
    void Finalize();

private:
    std::unique_ptr<Stream> &_out;
    bool _compress = false;
    soff_t _lastSlotPos = -1; // last slot save position in file
    // sprite index accumulated on write for reporting back to user
    SpriteFileIndex _index;
    // compression buffer
	std::vector<uint8_t> _membuf;
};

// Saves all sprites to file; fills in index data for external use
// TODO: refactor to be able to save main file and index file separately (separate function for gather data?)
int SaveSpriteFile(const String &save_to_file,
    const std::vector<Bitmap*> &sprites, // available sprites (may contain nullptrs)
    SpriteFile *read_from_file, // optional file to read missing sprites from
    bool compressOutput, SpriteFileIndex &index);
// Saves sprite index table in a separate file
int SaveSpriteIndex(const String &filename, const SpriteFileIndex &index);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
