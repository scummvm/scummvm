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

	if (vers == kSprfVersion_Uncompressed) {
		this->_compressed = false;
	} else if (vers == kSprfVersion_Compressed) {
		this->_compressed = true;
	} else if (vers >= kSprfVersion_Last32bit) {
		this->_compressed = (_stream->ReadInt8() == 1);
		spriteFileID = _stream->ReadInt32();
	}

	if (vers < kSprfVersion_Compressed) {
		// skip the palette
		_stream->Seek(256 * 3); // sizeof(RGB) * 256
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

void SpriteFile::Reset() {
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
	for (sprkey_t i = 0; i <= topmost; ++i) {
		_spriteData[i].Offset = in->GetPosition();

		int coldep = in->ReadInt16();

		if (coldep == 0) {
			if (in->EOS())
				break;
			continue;
		}

		if (in->EOS())
			break;

		if ((size_t)i >= _spriteData.size())
			break;

		int wdd = in->ReadInt16();
		int htt = in->ReadInt16();
		metrics[i].Width = wdd;
		metrics[i].Height = htt;

		size_t spriteDataSize;
		if (vers == kSprfVersion_Compressed) {
			spriteDataSize = in->ReadInt32();
		} else if (vers >= kSprfVersion_Last32bit) {
			spriteDataSize = this->_compressed ? in->ReadInt32() : wdd * coldep * htt;
		} else {
			spriteDataSize = wdd * coldep * htt;
		}
		in->Seek(spriteDataSize);
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

	int coldep = _stream->ReadInt16();
	if (coldep == 0) { // empty slot, this is normal
		return HError::None();
	}

	int wdd = _stream->ReadInt16();
	int htt = _stream->ReadInt16();
	Bitmap *image = BitmapHelper::CreateBitmap(wdd, htt, coldep * 8);
	if (image == nullptr) {
		return new Error(String::FromFormat("LoadSprite: failed to allocate bitmap %d (%dx%d%d).",
			index, wdd, htt, coldep * 8));
	}

	if (_compressed) {
		size_t data_size = _stream->ReadInt32();
		if (data_size == 0) {
			delete image;
			return new Error(String::FromFormat("LoadSprite: bad compressed data for sprite %d.", index));
		}
		rle_decompress(image, _stream.get());
	} else {
		if (coldep == 1) {
			for (int h = 0; h < htt; ++h)
				_stream->ReadArray(&image->GetScanLineForWriting(h)[0], coldep, wdd);
		} else if (coldep == 2) {
			for (int h = 0; h < htt; ++h)
				_stream->ReadArrayOfInt16((int16_t *)&image->GetScanLineForWriting(h)[0], wdd);
		} else {
			for (int h = 0; h < htt; ++h)
				_stream->ReadArrayOfInt32((int32_t *)&image->GetScanLineForWriting(h)[0], wdd);
		}
	}
	sprite = image;
	_curPos = index + 1; // mark correct pos
	return HError::None();
}

HError SpriteFile::LoadSpriteData(sprkey_t index, Size &metric, int &bpp,
	std::vector<char> &data) {
	metric = Size();
	bpp = 0;
	if (index < 0 || (size_t)index >= _spriteData.size())
		new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

	if (_spriteData[index].Offset == 0)
		return HError::None(); // sprite is not in file

	SeekToSprite(index);
	_curPos = -2; // mark undefined pos

	int coldep = _stream->ReadInt16();
	if (coldep == 0) { // empty slot, this is normal
		metric = Size();
		bpp = 0;
		data.resize(0);
		return HError::None();
	}

	int width = _stream->ReadInt16();
	int height = _stream->ReadInt16();

	size_t data_size;
	if (_compressed)
		data_size = _stream->ReadInt32();
	else
		data_size = width * height * coldep;
	data.resize(data_size);
	_stream->Read(&data[0], data_size);
	metric = Size(width, height);
	bpp = coldep;
	_curPos = index + 1; // mark correct pos
	return HError::None();
}

sprkey_t SpriteFile::FindTopmostSprite(const std::vector<Bitmap *> &sprites) {
	sprkey_t topmost = -1;
	for (sprkey_t i = 0; i < static_cast<sprkey_t>(sprites.size()); ++i)
		if (sprites[i])
			topmost = i;
	return topmost;
}

void SpriteFile::SeekToSprite(sprkey_t index) {
	// If we didn't just load the previous sprite, seek to it
	if (index != _curPos) {
		_stream->Seek(_spriteData[index].Offset, kSeekBegin);
		_curPos = index;
	}
}

int SpriteFile::SaveToFile(const String &save_to_file,
	const std::vector<Bitmap *> &sprites,
	SpriteFile *read_from_file,
	bool compressOutput, SpriteFileIndex &index) {
	std::unique_ptr<Stream> output(File::CreateFile(save_to_file));
	if (output == nullptr)
		return -1;

	int spriteFileIDCheck = g_system->getMillis();

	// sprite file version
	output->WriteInt16(kSprfVersion_Current);

	output->WriteArray(spriteFileSig, strlen(spriteFileSig), 1);

	output->WriteInt8(compressOutput ? 1 : 0);
	output->WriteInt32(spriteFileIDCheck);

	sprkey_t lastslot = read_from_file ? read_from_file->GetTopmostSprite() : 0;
	lastslot = std::max(lastslot, FindTopmostSprite(sprites));
	output->WriteInt32(lastslot);

	// allocate buffers to store the indexing info
	sprkey_t numsprits = lastslot + 1;
	std::vector<int16_t> spritewidths, spriteheights;
	std::vector<soff_t> spriteoffs;
	spritewidths.resize(numsprits);
	spriteheights.resize(numsprits);
	spriteoffs.resize(numsprits);
	std::unique_ptr<Bitmap> temp_bmp; // for disposing temp sprites
	std::vector<char> membuf; // for loading raw sprite data

	const bool diff_compress =
		read_from_file && read_from_file->IsFileCompressed() != compressOutput;

	for (sprkey_t i = 0; i <= lastslot; ++i) {
		soff_t sproff = output->GetPosition();

		Bitmap *image = (size_t)i < sprites.size() ? sprites[i] : nullptr;

		// if compression setting is different, load the sprite into memory
		// (otherwise we will be able to simply copy bytes from one file to another
		if ((image == nullptr) && diff_compress) {
			read_from_file->LoadSprite(i, image);
			temp_bmp.reset(image);
		}

		// if managed to load an image - save it according the new compression settings
		if (image != nullptr) {
			// image in memory -- write it out
			int bpp = image->GetColorDepth() / 8;
			spriteoffs[i] = sproff;
			spritewidths[i] = image->GetWidth();
			spriteheights[i] = image->GetHeight();
			output->WriteInt16(bpp);
			output->WriteInt16(spritewidths[i]);
			output->WriteInt16(spriteheights[i]);

			if (compressOutput) {
				soff_t lenloc = output->GetPosition();
				// write some space for the length data
				output->WriteInt32(0);

				rle_compress(image, output.get());

				soff_t fileSizeSoFar = output->GetPosition();
				// write the length of the compressed data
				output->Seek(lenloc, kSeekBegin);
				output->WriteInt32((fileSizeSoFar - lenloc) - 4);
				output->Seek(0, kSeekEnd);
			} else {
				output->WriteArray(image->GetDataForWriting(), spritewidths[i] * bpp, spriteheights[i]);
			}
			continue;
		} else if (diff_compress) {
			// sprite doesn't exist
			output->WriteInt16(0); // colour depth
			continue;
		}

		// Not in memory - and same compression option;
		// Directly copy the sprite bytes from the input file to the output
		Size metric;
		int bpp;
		read_from_file->LoadSpriteData(i, metric, bpp, membuf);

		output->WriteInt16(bpp);
		if (bpp == 0)
			continue; // empty slot

		spriteoffs[i] = sproff;
		spritewidths[i] = metric.Width;
		spriteheights[i] = metric.Height;
		output->WriteInt16(metric.Width);
		output->WriteInt16(metric.Height);
		if (compressOutput)
			output->WriteInt32(membuf.size());
		if (membuf.size() == 0)
			continue; // bad data?
		output->Write(&membuf[0], membuf.size());
	}

	index.SpriteFileIDCheck = spriteFileIDCheck;
	index.LastSlot = lastslot;
	index.SpriteCount = numsprits;
	index.Widths = spritewidths;
	index.Heights = spriteheights;
	index.Offsets = spriteoffs;
	return 0;
}

int SpriteFile::SaveSpriteIndex(const String &filename, const SpriteFileIndex &index) {
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
	out->WriteInt32(index.LastSlot);
	out->WriteInt32(index.SpriteCount);
	if (index.SpriteCount > 0) {
		out->WriteArrayOfInt16(&index.Widths.front(), index.Widths.size());
		out->WriteArrayOfInt16(&index.Heights.front(), index.Heights.size());
		out->WriteArrayOfInt64(&index.Offsets.front(), index.Offsets.size());
	}
	delete out;
	return 0;
}

} // namespace Shared
} // namespace AGS
} // namespace AGS3
