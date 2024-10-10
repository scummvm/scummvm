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

#include "ags/shared/ac/sprite_file.h"
#include "common/std/algorithm.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/compress.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/memory_stream.h"
#include "ags/shared/util/stream.h"

namespace AGS3 {
namespace AGS {
namespace Shared {

static const char *spriteFileSig = " Sprite File ";
static const char *spindexid = "SPRINDEX";

// TODO: should not be part of SpriteFile, but rather some asset management class?
const char *SpriteFile::DefaultSpriteFileName = "acsprset.spr";
const char *SpriteFile::DefaultSpriteIndexName = "sprindex.dat";

// Image buffer pointer, a helper struct that eases switching
// between intermediate buffers when loading, saving or converting an image.
template <typename T> struct ImBufferPtrT {
	T        Buf = nullptr;
	size_t   Size = 0;
	int      BPP = 1; // byte per pixel

	ImBufferPtrT() = default;
	ImBufferPtrT(T buf, size_t sz, int bpp) : Buf(buf), Size(sz), BPP(bpp) {
	}
};
typedef ImBufferPtrT<uint8_t *> ImBufferPtr;
typedef ImBufferPtrT<const uint8_t *> ImBufferCPtr;


// Finds the given color's index in the palette, or returns SIZE_MAX if such color is not there
static size_t lookup_palette(uint32_t col, uint32_t palette[256], uint32_t ncols) {
	for (size_t i = 0; i < ncols; ++i)
		if (palette[i] == col) return i;
	return SIZE_MAX;
}

// Converts a 16/32-bit image into the indexed 8-bit pixel data with palette;
// NOTE: the palette will contain colors in the same format as the source image.
// only succeeds if the total number of colors used in the image is < 257.
static bool CreateIndexedBitmap(const Bitmap *image, std::vector<uint8_t> &dst_data,
	uint32_t palette[256], uint32_t &pal_count) {
	const int src_bpp = image->GetBPP();
	if (src_bpp < 2) { assert(0); return false; }
	const size_t src_size = image->GetWidth() * image->GetHeight() * image->GetBPP();
	const size_t dst_size = image->GetWidth() * image->GetHeight();
	dst_data.resize(dst_size);
	const uint8_t *src = image->GetData(), *src_end = src + src_size;
	uint8_t *dst = &dst_data[0], *dst_end = dst + dst_size;
	pal_count = 0;

	for (; src < src_end && dst < dst_end; src += src_bpp) {
		uint32_t col = 0;
		size_t pal_n = 0;
		switch (src_bpp) {
		case 2:
			col = *((const uint16_t *)src);
			pal_n = lookup_palette(col, palette, pal_count);
			break;
		case 4:
			col = *((const uint32_t *)src);
			pal_n = lookup_palette(col, palette, pal_count);
			break;
		default: assert(0); return false;
		}

		if (pal_n == SIZE_MAX) {
			if (pal_count == 256) return false;
			pal_n = pal_count;
			palette[pal_count++] = col;
		}
		*(dst++) = (uint8_t)pal_n;
	}
	return true;
}

// Unpacks an indexed image's pixel data into the 16/32-bit image;
// NOTE: the palette is expected to contain colors in the same format as the destination.
static void UnpackIndexedBitmap(Bitmap *image, const uint8_t *data, size_t data_size,
	uint32_t *palette, uint32_t pal_count) {
	assert(pal_count > 0);
	if (pal_count == 0) return; // meaningless
	const uint8_t bpp = image->GetBPP();
	const size_t dst_size = image->GetWidth() * image->GetHeight() * image->GetBPP();
	uint8_t *dst = image->GetDataForWriting(), *dst_end = dst + dst_size;

	switch (bpp) {
		case 2:
			for (size_t p = 0; (p < data_size) && (dst < dst_end); ++p, dst += bpp) {
				uint8_t index = data[p];
				assert(index < pal_count);
				uint32_t color = palette[(index < pal_count) ? index : 0];
				*((uint16_t *)dst) = color;
			}
			break;
		case 4:
			for (size_t p = 0; (p < data_size) && (dst < dst_end); ++p, dst += bpp) {
				uint8_t index = data[p];
				assert(index < pal_count);
				uint32_t color = palette[(index < pal_count) ? index : 0];
				*((uint32_t *)dst) = color;
			}
			break;
		default:
			assert(0);
			return;
		}
	}


static inline SpriteFormat PaletteFormatForBPP(int bpp) {
	switch (bpp) {
	case 1: return kSprFmt_PaletteRgb888;
	case 2: return kSprFmt_PaletteRgb565;
	case 4: return kSprFmt_PaletteArgb8888;
	default: return kSprFmt_Undefined;
	}
}

static inline uint8_t GetPaletteBPP(SpriteFormat fmt) {
	switch (fmt) {
	case kSprFmt_PaletteRgb888: return 3;
	case kSprFmt_PaletteArgb8888: return 4;
	case kSprFmt_PaletteRgb565: return 2;
	default: return 0; // means no palette
	}
}

SpriteFile::SpriteFile() {
	_curPos = -2;
}

HError SpriteFile::OpenFile(const String &filename, const String &sprindex_filename,
		std::vector<Size> &metrics) {
	Close();

	char buff[20];
	soff_t spr_initial_offs = 0;
	int spriteFileID = 0;

	_stream.reset(_GP(AssetMgr)->OpenAsset(filename));
	if (_stream == nullptr)
		return new Error(String::FromFormat("Failed to open spriteset file '%s'.", filename.GetCStr()));

	spr_initial_offs = _stream->GetPosition();

	_version = (SpriteFileVersion)_stream->ReadInt16();
	// read the "Sprite File" signature
	_stream->ReadArray(&buff[0], 13, 1);

	if (_version < kSprfVersion_Uncompressed || _version > kSprfVersion_Current) {
		_stream.reset();
		return new Error(String::FromFormat("Unsupported spriteset format (requested %d, supported %d - %d).", _version,
			kSprfVersion_Uncompressed, kSprfVersion_Current));
	}

	// unknown version
	buff[13] = 0;
	if (strcmp(buff, spriteFileSig)) {
		_stream.reset();
		return new Error("Uknown spriteset format.");
	}

	_storeFlags = 0;
	if (_version < kSprfVersion_Compressed) {
		_compress = kSprCompress_None;
		// skip the palette
		_stream->Seek(256 * 3); // sizeof(RGB) * 256
	} else if (_version == kSprfVersion_Compressed) {
		_compress = kSprCompress_RLE;
	} else if (_version >= kSprfVersion_Last32bit) {
		_compress = (SpriteCompression)_stream->ReadInt8();
		spriteFileID = _stream->ReadInt32();
	}

	sprkey_t topmost;
	if (_version < kSprfVersion_HighSpriteLimit)
		topmost = (uint16_t)_stream->ReadInt16();
	else
		topmost = _stream->ReadInt32();
	if (_version < kSprfVersion_Uncompressed)
		topmost = 200;

	_spriteData.resize(topmost + 1);
	metrics.resize(topmost + 1);

	// Version 12+: read global store flags
	if (_version >= kSprfVersion_StorageFormats) {
		_storeFlags = _stream->ReadInt8();
		_stream->ReadInt8(); // reserved
		_stream->ReadInt8();
		_stream->ReadInt8();
	}

	// if there is a sprite index file, use it
	if (LoadSpriteIndexFile(sprindex_filename, spriteFileID,
		spr_initial_offs, topmost, metrics)) {
		// Succeeded
		return HError::None();
	}

	// Failed, index file is invalid; index sprites manually
	return RebuildSpriteIndex(_stream.get(), topmost, metrics);
}

void SpriteFile::Close() {
	_stream.reset();
	_spriteData.clear();
	_version = kSprfVersion_Undefined;
	_storeFlags = 0;
	_compress = kSprCompress_None;
	_curPos = -2;
}

int SpriteFile::GetStoreFlags() const {
	return _storeFlags;
}

SpriteCompression SpriteFile::GetSpriteCompression() const {
	return _compress;
}

sprkey_t SpriteFile::GetTopmostSprite() const {
	return (sprkey_t)_spriteData.size() - 1;
}

bool SpriteFile::LoadSpriteIndexFile(const String &filename, int expectedFileID,
	soff_t spr_initial_offs, sprkey_t topmost, std::vector<Size> &metrics) {
	Stream *fidx = _GP(AssetMgr)->OpenAsset(filename);
	if (fidx == nullptr) {
		return false;
	}

	char buffer[9];
	// check "SPRINDEX" id
	fidx->ReadArray(&buffer[0], strlen(spindexid), 1);
	buffer[8] = 0;
	if (strcmp(buffer, spindexid)) {
		delete fidx;
		return false;
	}
	// check version
	SpriteIndexFileVersion vers = (SpriteIndexFileVersion)fidx->ReadInt32();
	if (vers < kSpridxfVersion_Initial || vers > kSpridxfVersion_Current) {
		delete fidx;
		return false;
	}
	if (vers >= kSpridxfVersion_Last32bit) {
		if (fidx->ReadInt32() != expectedFileID) {
			delete fidx;
			return false;
		}
	}

	sprkey_t topmost_index = fidx->ReadInt32();
	// end index+1 should be the same as num sprites
	if (fidx->ReadInt32() != topmost_index + 1) {
		delete fidx;
		return false;
	}

	if (topmost_index != topmost) {
		delete fidx;
		return false;
	}

	sprkey_t numsprits = topmost_index + 1;
	std::vector<int16_t> rspritewidths; rspritewidths.resize(numsprits);
	std::vector<int16_t> rspriteheights; rspriteheights.resize(numsprits);
	std::vector<soff_t>  spriteoffs; spriteoffs.resize(numsprits);

	fidx->ReadArrayOfInt16(&rspritewidths[0], numsprits);
	fidx->ReadArrayOfInt16(&rspriteheights[0], numsprits);
	if (vers <= kSpridxfVersion_Last32bit) {
		for (sprkey_t i = 0; i < numsprits; ++i)
			spriteoffs[i] = fidx->ReadInt32();
	} else // large file support
	{
		fidx->ReadArrayOfInt64(&spriteoffs[0], numsprits);
	}
	delete fidx;

	for (sprkey_t i = 0; i <= topmost_index; ++i) {
		if (spriteoffs[i] != 0) {
			_spriteData[i].Offset = spriteoffs[i] + spr_initial_offs;
			metrics[i].Width = rspritewidths[i];
			metrics[i].Height = rspriteheights[i];
		}
	}
	return true;
}

static inline void ReadSprHeader(SpriteDatHeader &hdr, Stream *in,
	const SpriteFileVersion ver, SpriteCompression gl_compress) {
	int bpp = in->ReadInt8();
	SpriteFormat sformat = (SpriteFormat)in->ReadInt8();
	// note we MUST read first 2 * int8 before skipping rest
	if (bpp == 0) {
		hdr = SpriteDatHeader(); return;
	} // empty slot
	int pal_count = 0;
	SpriteCompression compress = gl_compress;
	if (ver >= kSprfVersion_StorageFormats) {
		pal_count = (uint8_t)in->ReadInt8() + 1; // saved as (count - 1)
		compress = (SpriteCompression)in->ReadInt8();
	}
	int w = in->ReadInt16();
	int h = in->ReadInt16();
	hdr = SpriteDatHeader(bpp, sformat, pal_count, compress, w, h);
}

HError SpriteFile::RebuildSpriteIndex(Stream *in, sprkey_t topmost,
		std::vector<Size> &metrics) {
	topmost = MIN(topmost, (sprkey_t)_spriteData.size() - 1);
	for (sprkey_t i = 0; !in->EOS() && (i <= topmost); ++i) {
		_spriteData[i].Offset = in->GetPosition();
		SpriteDatHeader hdr;
		ReadSprHeader(hdr, _stream.get(), _version, _compress);
		if (hdr.BPP == 0) continue; // empty slot, this is normal
		int pal_bpp = GetPaletteBPP(hdr.SFormat);
		if (pal_bpp > 0) in->Seek(hdr.PalCount * pal_bpp); // skip palette
		size_t data_sz =
			((_version >= kSprfVersion_StorageFormats) || _compress != kSprCompress_None) ?
			(uint32_t)in->ReadInt32() : hdr.Width * hdr.Height * hdr.BPP;
		in->Seek(data_sz); // skip image data
		metrics[i].Width = hdr.Width;
		metrics[i].Height = hdr.Height;
	}
	return HError::None();
}

HError SpriteFile::LoadSprite(sprkey_t index, Shared::Bitmap *&sprite) {
	sprite = nullptr;
	if (index < 0 || (size_t)index >= _spriteData.size())
		return new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

	if (_spriteData[index].Offset == 0)
		return HError::None(); // sprite is not in file

	SeekToSprite(index);
	_curPos = -2; // mark undefined pos

	SpriteDatHeader hdr;
	ReadSprHeader(hdr, _stream.get(), _version, _compress);
	if (hdr.BPP == 0) return HError::None(); // empty slot, this is normal
	int bpp = hdr.BPP, w = hdr.Width, h = hdr.Height;
	std::unique_ptr<Bitmap> image(BitmapHelper::CreateBitmap(w, h, bpp * 8));
	if (image == nullptr) {
		return new Error(String::FromFormat("LoadSprite: failed to allocate bitmap %d (%dx%d%d).",
			index, w, h, bpp * 8));
	}
	ImBufferPtr im_data(image->GetDataForWriting(), w * h * bpp, bpp);
	// (Optional) Handle storage options, reverse
	std::vector<uint8_t> indexed_buf;
	uint32_t palette[256];
	uint32_t pal_bpp = GetPaletteBPP(hdr.SFormat);
	if (pal_bpp > 0) { // read palette if format assumes one
		switch (pal_bpp) {
		case 2: for (uint32_t i = 0; i < hdr.PalCount; ++i) {
			palette[i] = _stream->ReadInt16();
		}
			  break;
		case 4: for (uint32_t i = 0; i < hdr.PalCount; ++i) {
			palette[i] = _stream->ReadInt32();
		}
			  break;
		default: assert(0); break;
		}
		indexed_buf.resize(w * h);
		im_data = ImBufferPtr(&indexed_buf[0], indexed_buf.size(), 1);
	}
	// (Optional) Decompress the image data into the temp buffer
	size_t in_data_size =
		((_version >= kSprfVersion_StorageFormats) || _compress != kSprCompress_None) ?
		(uint32_t)_stream->ReadInt32() : (w * h * bpp);
	if (hdr.Compress != kSprCompress_None) {
		// TODO: rewrite this to only make a choice once the SpriteFile is initialized
		// and use either function ptr or a decompressing stream class object
		if (in_data_size == 0) {
			return new Error(String::FromFormat("LoadSprite: bad compressed data for sprite %d.", index));
		}
		bool result;
		switch (hdr.Compress) {
		case kSprCompress_RLE: result = rle_decompress(im_data.Buf, im_data.Size, im_data.BPP, _stream.get());
			break;
		case kSprCompress_LZW: result = lzw_decompress(im_data.Buf, im_data.Size, im_data.BPP, _stream.get(), in_data_size);
			break;
		case kSprCompress_Deflate: result = inflate_decompress(im_data.Buf, im_data.Size, im_data.BPP, _stream.get(), in_data_size);
			break;
		default: assert(!"Unsupported compression type!"); result = false; break;
		}
		// TODO: test that not more than data_size was read!
		if (!result) {
			return new Error(String::FromFormat("LoadSprite: failed to decompress pixel array for sprite %d.", index));
		}
	}
	// Otherwise (no compression) read directly
	else {
		switch (im_data.BPP) {
		case 1: _stream->Read(im_data.Buf, im_data.Size);
			break;
		case 2: _stream->ReadArrayOfInt16(
			reinterpret_cast<int16_t *>(im_data.Buf), im_data.Size / sizeof(int16_t));
			break;
		case 4: _stream->ReadArrayOfInt32(
			reinterpret_cast<int32_t *>(im_data.Buf), im_data.Size / sizeof(int32_t));
			break;
		default: assert(0); break;
		}
	}
	// Finally revert storage options
	if (pal_bpp > 0) {
		UnpackIndexedBitmap(image.get(), im_data.Buf, im_data.Size, palette, hdr.PalCount);
	}

	sprite = image.release(); // FIXME: pass unique_ptr in this function
	_curPos = index + 1; // mark correct pos
	return HError::None();
}

HError SpriteFile::LoadRawData(sprkey_t index, SpriteDatHeader &hdr, std::vector<uint8_t> &data) {
	hdr = SpriteDatHeader();
	data.resize(0);
	if (index < 0 || (size_t)index >= _spriteData.size())
		return new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

	if (_spriteData[index].Offset == 0)
		return HError::None(); // sprite is not in file

	SeekToSprite(index);
	_curPos = -2; // mark undefined pos

	ReadSprHeader(hdr, _stream.get(), _version, _compress);
	if (hdr.BPP == 0) return HError::None(); // empty slot, this is normal
	size_t data_size = 0;
	soff_t data_pos = _stream->GetPosition();
	// Optional palette
	size_t pal_size = hdr.PalCount * GetPaletteBPP(hdr.SFormat);
	data_size += pal_size;
	_stream->Seek(pal_size);
	// Pixel data
	if ((_version >= kSprfVersion_StorageFormats) || _compress != kSprCompress_None)
		data_size += (uint32_t)_stream->ReadInt32() + sizeof(uint32_t);
	else
		data_size += hdr.Width * hdr.Height * hdr.BPP;
	// Seek back and read all at once
	data.resize(data_size);
	_stream->Seek(data_pos, kSeekBegin);
	_stream->Read(&data[0], data_size);

	_curPos = index + 1; // mark correct pos
	return HError::None();
}

void SpriteFile::SeekToSprite(sprkey_t index) {
	// If we didn't just load the previous sprite, seek to it
	if (index != _curPos) {
		_stream->Seek(_spriteData[index].Offset, kSeekBegin);
		_curPos = index;
	}
}


// Finds the topmost occupied slot index
static sprkey_t FindTopmostSprite(const std::vector<std::pair<bool, Bitmap *>> &sprites) {
	sprkey_t topmost = -1;
	for (sprkey_t i = 0; i < static_cast<sprkey_t>(sprites.size()); ++i)
		if (sprites[i].first)
			topmost = i;
	return topmost;
}

int SaveSpriteFile(const String &save_to_file,
		const std::vector<std::pair<bool, Bitmap *> > &sprites,
		SpriteFile *read_from_file,
		int store_flags, SpriteCompression compress, SpriteFileIndex &index) {
	std::unique_ptr<Stream> output(File::CreateFile(save_to_file));
	if (output == nullptr)
		return -1;

	sprkey_t lastslot = FindTopmostSprite(sprites);
	SpriteFileWriter writer(output);
	writer.Begin(store_flags, compress, lastslot);

	std::unique_ptr<Bitmap> temp_bmp; // for disposing temp sprites
	std::vector<uint8_t> membuf; // for loading raw sprite data

	const bool diff_compress =
		read_from_file &&
		(read_from_file->GetSpriteCompression() != compress ||
			read_from_file->GetStoreFlags() != store_flags);

	for (sprkey_t i = 0; i <= lastslot; ++i) {
		if (!sprites[i].first) { // empty slot
			writer.WriteEmptySlot();
			continue;
		}

		Bitmap *image = sprites[i].second;
		// if compression setting is different, load the sprite into memory
		// (otherwise we will be able to simply copy bytes from one file to another
		if ((image == nullptr) && diff_compress) {
			read_from_file->LoadSprite(i, image);
			temp_bmp.reset(image);
		}

		// if managed to load an image - save it according the new compression settings
		if (image != nullptr) {
			writer.WriteBitmap(image);
			continue;
		} else if (diff_compress) {
			// sprite doesn't exist
			writer.WriteEmptySlot();
			continue;
		}

		// Not in memory - and same compression option;
		// Directly copy the sprite bytes from the input file to the output
		SpriteDatHeader hdr;
		read_from_file->LoadRawData(i, hdr, membuf);
		if (hdr.BPP == 0) { // empty slot
			writer.WriteEmptySlot();
			continue;
		}
		writer.WriteRawData(hdr, &membuf[0], membuf.size());
	}
	writer.Finalize();

	index = writer.GetIndex();
	return 0;
}

int SaveSpriteIndex(const String &filename, const SpriteFileIndex &index) {
	// write the sprite index file
	Stream *out = File::CreateFile(filename);
	if (!out)
		return -1;
	// write "SPRINDEX" id
	out->WriteArray(spindexid, strlen(spindexid), 1);
	// write version
	out->WriteInt32(kSpridxfVersion_Current);
	out->WriteInt32(index.SpriteFileIDCheck);
	// write last sprite number and num sprites, to verify that
	// it matches the spr file
	out->WriteInt32(index.GetLastSlot());
	out->WriteInt32(index.GetCount());
	if (index.GetCount() > 0) {
		out->WriteArrayOfInt16(&index.Widths[0], index.Widths.size());
		out->WriteArrayOfInt16(&index.Heights[0], index.Heights.size());
		out->WriteArrayOfInt64(&index.Offsets[0], index.Offsets.size());
	}
	delete out;
	return 0;
}

SpriteFileWriter::SpriteFileWriter(std::unique_ptr<Stream> &out) : _out(out) {
}

void SpriteFileWriter::Begin(int store_flags, SpriteCompression compress, sprkey_t last_slot) {
	if (!_out) return;
	_index.SpriteFileIDCheck = g_system->getMillis();
	_storeFlags = store_flags;
	_compress = compress;

	// sprite file version
	_out->WriteInt16(kSprfVersion_Current);
	_out->WriteArray(spriteFileSig, strlen(spriteFileSig), 1);
	_out->WriteInt8(_compress ? 1 : 0);
	_out->WriteInt32(_index.SpriteFileIDCheck);

	// Remember and write provided "last slot" index,
	// but if it's not set (< 0) then we will have to return back later
	// and write correct one; this is done in Finalize().
	_lastSlotPos = _out->GetPosition();
	_out->WriteInt32(last_slot);

	_out->WriteInt8(_storeFlags);
	_out->WriteInt8(0); // reserved
	_out->WriteInt8(0);
	_out->WriteInt8(0);

	if (last_slot >= 0) { // allocate buffers to store the indexing info
		sprkey_t numsprits = last_slot + 1;
		_index.Offsets.reserve(numsprits);
		_index.Widths.reserve(numsprits);
		_index.Heights.reserve(numsprits);
	}
}

void SpriteFileWriter::WriteBitmap(Bitmap *image) {
	if (!_out) return;
	int bpp = image->GetBPP();
	int w = image->GetWidth();
	int h = image->GetHeight();
	ImBufferCPtr im_data(image->GetData(), w * h * bpp, bpp);

	// (Optional) Handle storage options
	std::vector<uint8_t> indexed_buf;
	uint32_t palette[256];
	uint32_t pal_count = 0;
	SpriteFormat sformat = kSprFmt_Undefined;

	if ((_storeFlags & kSprStore_OptimizeForSize) != 0 && (image->GetBPP() > 1)) { // Try to store this sprite as an indexed bitmap
		uint32_t gen_pal_count;
		if (CreateIndexedBitmap(image, indexed_buf, palette, gen_pal_count) && gen_pal_count > 0) { // Test the resulting size, and switch if the paletted image is less
			if (im_data.Size > (indexed_buf.size() + gen_pal_count * image->GetBPP())) {
			im_data = ImBufferCPtr(&indexed_buf[0], indexed_buf.size(), 1);
			sformat = PaletteFormatForBPP(image->GetBPP());
			pal_count = gen_pal_count;
			}
		}
	}
	// (Optional) Compress the image data into the temp buffer
	SpriteCompression compress = kSprCompress_None;
	if (_compress != kSprCompress_Deflate)
		warning("TODO: Deflate not implemented, writing uncompressed BMP");
	else if (_compress != kSprCompress_None) {
		// TODO: rewrite this to only make a choice once the SpriteFile is initialized
		// and use either function ptr or a decompressing stream class object
		compress = _compress;
		VectorStream mems(_membuf, kStream_Write);
		bool result;
		switch (compress) {
		case kSprCompress_RLE: result = rle_compress(im_data.Buf, im_data.Size, im_data.BPP, &mems);
			break;
		case kSprCompress_LZW: result = lzw_compress(im_data.Buf, im_data.Size, im_data.BPP, &mems);
			break;
		case kSprCompress_Deflate: result = deflate_compress(im_data.Buf, im_data.Size, im_data.BPP, &mems);
			break;
		default: assert(!"Unsupported compression type!"); result = false; break;
		}
		// mark to write as a plain byte array
		im_data = result ? ImBufferCPtr(&_membuf[0], _membuf.size(), 1) : ImBufferCPtr();
	}

	// Write the final data
	SpriteDatHeader hdr(bpp, sformat, pal_count, compress, w, h);
	WriteSpriteData(hdr, im_data.Buf, im_data.Size, im_data.BPP, palette);
	_membuf.clear();
}

static inline void WriteSprHeader(const SpriteDatHeader &hdr, Stream *out) {
	out->WriteInt8(hdr.BPP);
	out->WriteInt8(hdr.SFormat);
	out->WriteInt8(hdr.PalCount > 0 ? (uint8_t)(hdr.PalCount - 1) : 0);
	out->WriteInt8(hdr.Compress);
	out->WriteInt16(hdr.Width);
	out->WriteInt16(hdr.Height);
}

void SpriteFileWriter::WriteSpriteData(const SpriteDatHeader &hdr,
	const uint8_t *im_data, size_t im_data_sz, int im_bpp,
	const uint32_t palette[256]) {
	// Add index entry and write resulting data to the stream
	soff_t sproff = _out->GetPosition();
	_index.Offsets.push_back(sproff);
	_index.Widths.push_back(hdr.Width);
	_index.Heights.push_back(hdr.Height);
	WriteSprHeader(hdr, _out.get());
	// write palette, if available
	int pal_bpp = GetPaletteBPP(hdr.SFormat);
	if (pal_bpp > 0) {
		assert(hdr.PalCount > 0);
		switch (pal_bpp) {
		case 2: for (uint32_t i = 0; i < hdr.PalCount; ++i) {
			_out->WriteInt16(palette[i]);
		}
			  break;
		case 4: for (uint32_t i = 0; i < hdr.PalCount; ++i) {
			_out->WriteInt32(palette[i]);
		}
			  break;
		}
	}
	// write the image pixel data
	_out->WriteInt32(im_data_sz);
	switch (im_bpp) {
	case 1: _out->Write(im_data, im_data_sz);
		break;
	case 2: _out->WriteArrayOfInt16(reinterpret_cast<const int16_t *>(im_data),
		im_data_sz / sizeof(int16_t));
		break;
	case 4: _out->WriteArrayOfInt32(reinterpret_cast<const int32_t *>(im_data),
		im_data_sz / sizeof(int32_t));
		break;
	default: assert(0); break;
	}
}

void SpriteFileWriter::WriteEmptySlot() {
	if (!_out) return;
	soff_t sproff = _out->GetPosition();
	_out->WriteInt16(0); // write invalid color depth to mark empty slot
	_index.Offsets.push_back(sproff);
	_index.Widths.push_back(0);
	_index.Heights.push_back(0);
}

void SpriteFileWriter::WriteRawData(const SpriteDatHeader &hdr, const uint8_t *data, size_t data_sz) {
	if (!_out) return;
	soff_t sproff = _out->GetPosition();
	_index.Offsets.push_back(sproff);
	_index.Widths.push_back(hdr.Width);
	_index.Heights.push_back(hdr.Height);
	WriteSprHeader(hdr, _out.get());
	_out->Write(data, data_sz);
}

void SpriteFileWriter::Finalize() {
	if (!_out || _lastSlotPos < 0) return;
	_out->Seek(_lastSlotPos, kSeekBegin);
	_out->WriteInt32(_index.GetLastSlot());
	_out.reset();
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
