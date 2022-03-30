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
#include "ags/lib/std/algorithm.h"
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


SpriteFile::SpriteFile() {
	_compressed = false;
	_curPos = -2;
}

HError SpriteFile::OpenFile(const String &filename, const String &sprindex_filename,
	std::vector<Size> &metrics) {
	SpriteFileVersion vers;
	char buff[20];
	soff_t spr_initial_offs = 0;
	int spriteFileID = 0;

	_stream.reset(_GP(AssetMgr)->OpenAsset(filename));
	if (_stream == nullptr)
		return new Error(String::FromFormat("Failed to open spriteset file '%s'.", filename.GetCStr()));

	spr_initial_offs = _stream->GetPosition();

	vers = (SpriteFileVersion)_stream->ReadInt16();
	// read the "Sprite File" signature
	_stream->ReadArray(&buff[0], 13, 1);

	if (vers < kSprfVersion_Uncompressed || vers > kSprfVersion_Current) {
		_stream.reset();
		return new Error(String::FromFormat("Unsupported spriteset format (requested %d, supported %d - %d).", vers, kSprfVersion_Uncompressed, kSprfVersion_Current));
	}

	// unknown version
	buff[13] = 0;
	if (strcmp(buff, spriteFileSig)) {
		_stream.reset();
		return new Error("Uknown spriteset format.");
	}

	if (vers < kSprfVersion_Compressed) {
		_compressed = false;
		// skip the palette
		_stream->Seek(256 * 3); // sizeof(RGB) * 256
	} else if (vers == kSprfVersion_Compressed) {
		_compressed = true;
	} else if (vers >= kSprfVersion_Last32bit) {
		_compressed = (_stream->ReadInt8() == 1);
		spriteFileID = _stream->ReadInt32();
	}

	sprkey_t topmost;
	if (vers < kSprfVersion_HighSpriteLimit)
		topmost = (uint16_t)_stream->ReadInt16();
	else
		topmost = _stream->ReadInt32();
	if (vers < kSprfVersion_Uncompressed)
		topmost = 200;

	_spriteData.resize(topmost + 1);
	metrics.resize(topmost + 1);

	// if there is a sprite index file, use it
	if (LoadSpriteIndexFile(sprindex_filename, spriteFileID,
		spr_initial_offs, topmost, metrics)) {
		// Succeeded
		return HError::None();
	}

	// Failed, index file is invalid; index sprites manually
	return RebuildSpriteIndex(_stream.get(), topmost, vers, metrics);
}

void SpriteFile::Close() {
	_stream.reset();
	_curPos = -2;
}

bool SpriteFile::IsFileCompressed() const {
	return _compressed;
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

HError SpriteFile::RebuildSpriteIndex(Stream *in, sprkey_t topmost,
	SpriteFileVersion vers, std::vector<Size> &metrics) {
	topmost = std::min(topmost, (sprkey_t)_spriteData.size() - 1);
	for (sprkey_t i = 0; !in->EOS() && (i <= topmost); ++i) {
		_spriteData[i].Offset = in->GetPosition();
		int bpp = in->ReadInt16();
		if (bpp == 0) continue; // empty slot
		int w = in->ReadInt16();
		int h = in->ReadInt16();
		metrics[i].Width = w;
		metrics[i].Height = h;
		size_t data_sz = _compressed ? in->ReadInt32() : w * h * bpp;
		in->Seek(data_sz); // skip image data
	}
	return HError::None();
}

HError SpriteFile::LoadSprite(sprkey_t index, Shared::Bitmap *&sprite) {
	sprite = nullptr;
	if (index < 0 || (size_t)index >= _spriteData.size())
		new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

	if (_spriteData[index].Offset == 0)
		return HError::None(); // sprite is not in file

	SeekToSprite(index);
	_curPos = -2; // mark undefined pos

	int bpp = _stream->ReadInt16();
	if (bpp == 0) { // empty slot, this is normal
		return HError::None();
	}
	int w = _stream->ReadInt16();
	int h = _stream->ReadInt16();
	Bitmap *image = BitmapHelper::CreateBitmap(w, h, bpp * 8);
	if (image == nullptr) {
		return new Error(String::FromFormat("LoadSprite: failed to allocate bitmap %d (%dx%d%d).",
			index, w, h, bpp * 8));
	}

	if (_compressed) {
		size_t data_size = _stream->ReadInt32();
		if (data_size == 0) {
			delete image;
			return new Error(String::FromFormat("LoadSprite: bad compressed data for sprite %d.", index));
		}
		rle_decompress(image, _stream.get());
		// TODO: test that not more than data_size was read!
	} else {
		switch (bpp) {
		case 1: _stream->Read(image->GetDataForWriting(), w * h); break;
		case 2: _stream->ReadArrayOfInt16(
			reinterpret_cast<int16_t *>(image->GetDataForWriting()), w *h); break;
		case 4: _stream->ReadArrayOfInt32(
			reinterpret_cast<int32_t *>(image->GetDataForWriting()), w *h); break;
		default: assert(0); break;
		}
	}

	sprite = image;
	_curPos = index + 1; // mark correct pos
	return HError::None();
}

HError SpriteFile::LoadRawData(sprkey_t index, SpriteDatHeader &hdr, std::vector<uint8_t> &data) {
	hdr = SpriteDatHeader();
	data.resize(0);
	if (index < 0 || (size_t)index >= _spriteData.size())
		new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

	if (_spriteData[index].Offset == 0)
		return HError::None(); // sprite is not in file

	SeekToSprite(index);
	_curPos = -2; // mark undefined pos

	int bpp = _stream->ReadInt16();
	if (bpp == 0) { // empty slot, this is normal
		return HError::None();
	}
	int w = _stream->ReadInt16();
	int h = _stream->ReadInt16();
	size_t data_size = w * h * bpp;
	if (_compressed) {
		data_size = _stream->ReadInt32() + sizeof(int32_t);
		_stream->Seek(-4);
	}
	data.resize(data_size);
	_stream->Read(&data[0], data_size);
	hdr = SpriteDatHeader(bpp, w, h);

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

// Finds the topmost occupied slot index. Warning: may be slow.
static sprkey_t FindTopmostSprite(const std::vector<Bitmap *> &sprites) {
	sprkey_t topmost = -1;
	for (sprkey_t i = 0; i < static_cast<sprkey_t>(sprites.size()); ++i)
		if (sprites[i])
			topmost = i;
	return topmost;
}

int SaveSpriteFile(const String &save_to_file,
	const std::vector<Bitmap *> &sprites,
	SpriteFile *read_from_file,
	bool compressOutput, SpriteFileIndex &index) {
	std::unique_ptr<Stream> output(File::CreateFile(save_to_file));
	if (output == nullptr)
		return -1;

	sprkey_t lastslot = read_from_file ? read_from_file->GetTopmostSprite() : 0;
	lastslot = std::max(lastslot, FindTopmostSprite(sprites));

	SpriteFileWriter writer(output);
	writer.Begin(compressOutput, lastslot);

	std::unique_ptr<Bitmap> temp_bmp; // for disposing temp sprites
	std::vector<uint8_t> membuf; // for loading raw sprite data

	const bool diff_compress =
		read_from_file && read_from_file->IsFileCompressed() != compressOutput;

	for (sprkey_t i = 0; i <= lastslot; ++i) {
		Bitmap *image = (size_t)i < sprites.size() ? sprites[i] : nullptr;

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

void SpriteFileWriter::Begin(bool compressed, sprkey_t last_slot) {
	if (!_out) return;
	_index.SpriteFileIDCheck = g_system->getMillis();
	_compress = compressed;

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
	const SpriteDatHeader hdr(bpp, w, h);
	if (_compress) {
		MemoryStream mems(_membuf, kStream_Write);
		rle_compress(image, &mems);
		// write image data as a plain byte array
		WriteSpriteData(hdr, &_membuf[0], _membuf.size(), 1);
		_membuf.clear();
	} else {
		WriteSpriteData(hdr, image->GetData(), w * h * bpp, bpp);
	}
}

void SpriteFileWriter::WriteSpriteData(const SpriteDatHeader &hdr,
	const uint8_t *im_data, size_t im_data_sz, int im_bpp) {
	// Add index entry and write resulting data to the stream
	soff_t sproff = _out->GetPosition();
	_index.Offsets.push_back(sproff);
	_index.Widths.push_back(hdr.Width);
	_index.Heights.push_back(hdr.Height);
	_out->WriteInt16(hdr.BPP);
	_out->WriteInt16(hdr.Width);
	_out->WriteInt16(hdr.Height);
	// if not compressed, then the data size is supposed to be calculated
	// from the image metrics
	if (_compress)
		_out->WriteInt32(im_data_sz);
	switch (im_bpp) {
	case 1: _out->Write(im_data, im_data_sz); break;
	case 2: _out->WriteArrayOfInt16(reinterpret_cast<const int16_t *>(im_data),
		im_data_sz / sizeof(int16_t)); break;
	case 4: _out->WriteArrayOfInt32(reinterpret_cast<const int32_t *>(im_data),
		im_data_sz / sizeof(int32_t)); break;
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
	_out->WriteInt16(hdr.BPP);
	_out->WriteInt16(hdr.Width);
	_out->WriteInt16(hdr.Height);
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
