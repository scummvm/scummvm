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
#include "common/std/memory.h"
#include "common/std/vector.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

class Bitmap;

// TODO: research old version differences
enum SpriteFileVersion {
	kSprfVersion_Undefined = 0,
	kSprfVersion_Uncompressed = 4,
	kSprfVersion_Compressed = 5,
	kSprfVersion_Last32bit = 6,
	kSprfVersion_64bit = 10,
	kSprfVersion_HighSpriteLimit = 11,
	kSprfVersion_StorageFormats = 12,
	kSprfVersion_Current = kSprfVersion_StorageFormats
};

enum SpriteIndexFileVersion {
	kSpridxfVersion_Initial = 1,
	kSpridxfVersion_Last32bit = 2,
	kSpridxfVersion_64bit = 10,
	kSpridxfVersion_HighSpriteLimit = 11,
	kSpridxfVersion_Current = kSpridxfVersion_HighSpriteLimit
};

// Instructions to how the sprites are allowed to be stored
enum SpriteStorage {
	// When possible convert the sprite into another format for less disk space
	// e.g. save 16/32-bit images as 8-bit colormaps with palette
	kSprStore_OptimizeForSize = 0x01
};

// Format in which the sprite's pixel data is stored
enum SpriteFormat {
	kSprFmt_Undefined = 0, // undefined, or keep as-is
	// Encoded as a 8-bit colormap with palette of 24-bit RGB values
	kSprFmt_PaletteRgb888 = 32,
	// Encoded as a 8-bit colormap with palette of 32-bit ARGB values
	kSprFmt_PaletteArgb8888 = 33,
	// Encoded as a 8-bit colormap with palette of 16-bit RGB565 values
	kSprFmt_PaletteRgb565 = 34
};

enum SpriteCompression {
	kSprCompress_None = 0,
	kSprCompress_RLE,
	kSprCompress_LZW,
	kSprCompress_Deflate
};

typedef int32_t sprkey_t;

// SpriteFileIndex contains sprite file's table of contents
struct SpriteFileIndex {
	int SpriteFileIDCheck = 0; // tag matching sprite file and index file
	std::vector<int16_t> Widths;
	std::vector<int16_t> Heights;
	std::vector<soff_t>  Offsets;

	inline size_t GetCount() const {
		return Offsets.size();
	}
	inline sprkey_t GetLastSlot() const {
		return (sprkey_t)GetCount() - 1;
	}
};

// Invidual sprite data header (as read from the file)
struct SpriteDatHeader {
	int BPP = 0; // color depth (bytes per pixel); or input format
	SpriteFormat SFormat = kSprFmt_Undefined; // storage format
	uint32_t PalCount = 0; // palette length, if applicable to storage format
	SpriteCompression Compress = kSprCompress_None; // compression type
	int Width = 0; // sprite's width
	int Height = 0; // sprite's height

	SpriteDatHeader() = default;
	SpriteDatHeader(int bpp, SpriteFormat sformat = kSprFmt_Undefined,
		uint32_t pal_count = 0, SpriteCompression compress = kSprCompress_None,
		int w = 0, int h = 0) : BPP(bpp), SFormat(sformat), PalCount(pal_count),
		Compress(compress), Width(w), Height(h) {
	}
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

	int         GetStoreFlags() const;
	// Tells if bitmaps in the file are compressed
	SpriteCompression GetSpriteCompression() const;
	// Tells the highest known sprite index
	sprkey_t    GetTopmostSprite() const;

	// Loads sprite index file
	bool        LoadSpriteIndexFile(const String &filename, int expectedFileID,
		soff_t spr_initial_offs, sprkey_t topmost, std::vector<Size> &metrics);
	// Rebuilds sprite index from the main sprite file
	HError      RebuildSpriteIndex(Stream *in, sprkey_t topmost,
		std::vector<Size> &metrics);

	// Loads an image data and creates a ready bitmap
	HError      LoadSprite(sprkey_t index, Bitmap *&sprite);
	// Loads a raw sprite element data into the buffer, stores header info separately
	HError      LoadRawData(sprkey_t index, SpriteDatHeader &hdr, std::vector<uint8_t> &data);

private:
	// Seek stream to sprite
	void        SeekToSprite(sprkey_t index);

	// Internal sprite reference
	struct SpriteRef {
		soff_t Offset = 0; // data offset
		size_t RawSize = 0; // file size of element, in bytes
		// TODO: RawSize is currently unused, due to incompleteness of spriteindex format
	};

	// Array of sprite references
	std::vector<SpriteRef> _spriteData;
	std::unique_ptr<Stream> _stream; // the sprite stream
	SpriteFileVersion _version = kSprfVersion_Current;
	int _storeFlags = 0; // storage flags, specify how sprites may be stored
	SpriteCompression _compress = kSprCompress_None; // sprite compression typ
	sprkey_t _curPos; // current stream position (sprite slot)
};

// SpriteFileWriter class writes a sprite file in a requested format.
// Start using it by calling Begin, write ready bitmaps or copy raw sprite data
// over slot by slot, then call Finalize to let it close the format correctly.
class SpriteFileWriter {
public:
	SpriteFileWriter(std::unique_ptr<Stream> &out);
	~SpriteFileWriter() {
	}

	// Get the sprite index, accumulated after write
	const SpriteFileIndex &GetIndex() const {
		return _index;
	}

	// Initializes new sprite file format;
	// store_flags are SpriteStorage;
	// optionally hint how many sprites will be written.
	void Begin(int store_flags, SpriteCompression compress, sprkey_t last_slot = -1);
	// Writes a bitmap into file, compressing if necessary
	void WriteBitmap(Bitmap *image);
	// Writes an empty slot marker
	void WriteEmptySlot();
	// Writes a raw sprite data without any additional processing
	void WriteRawData(const SpriteDatHeader &hdr, const uint8_t *data, size_t data_sz);
	// Finalizes current format; no further writing is possible after this
	void Finalize();

private:
	// Writes prepared image data in a proper file format, following explicit data_bpp rule
	void WriteSpriteData(const SpriteDatHeader &hdr,
		const uint8_t *im_data, size_t im_data_sz, int im_bpp,
		const uint32_t palette[256]);

	std::unique_ptr<Stream> &_out;
	int _storeFlags = 0;
	SpriteCompression _compress = kSprCompress_None;
	soff_t _lastSlotPos = -1; // last slot save position in file
	// sprite index accumulated on write for reporting back to user
	SpriteFileIndex _index;
	// compression buffer
	std::vector<uint8_t> _membuf;
};

// Saves all sprites to file; fills in index data for external use.
// TODO: refactor to be able to save main file and index file separately (separate function for gather data?)
// Accepts available sprites as pairs of bool and Bitmap pointer, where boolean value
// tells if sprite exists and Bitmap pointer may be null;
// If a sprite's bitmap is missing, it will try reading one from the input file stream.
int SaveSpriteFile(const String &save_to_file,
	const std::vector<std::pair<bool, Bitmap *> > &sprites,
	SpriteFile *read_from_file, // optional file to read missing sprites from
	int store_flags, SpriteCompression compress, SpriteFileIndex &index);
// Saves sprite index table in a separate file
extern int SaveSpriteIndex(const String &filename, const SpriteFileIndex &index);

} // namespace Shared
} // namespace AGS
} // namespace AGS3

#endif
