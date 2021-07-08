/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

//=============================================================================
//
// sprite caching system
//
//=============================================================================

#ifdef _MANAGED
// ensure this doesn't get compiled to .NET IL
#pragma unmanaged
#pragma warning (disable: 4996 4312)  // disable deprecation warnings
#endif

#include "common/system.h"
#include "ags/lib/std/algorithm.h"
#include "ags/shared/ac/common.h" // quit
#include "ags/shared/ac/game_struct_defines.h"
#include "ags/shared/ac/sprite_cache.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/gfx/bitmap.h"
#include "ags/shared/util/compress.h"
#include "ags/shared/util/file.h"
#include "ags/shared/util/stream.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

// [IKM] We have to forward-declare these because their implementations are in the Engine
extern void initialize_sprite(int);
extern void pre_save_sprite(Bitmap *image);
extern void get_new_size_for_sprite(int, int, int, int &, int &);

#define START_OF_LIST -1
#define END_OF_LIST   -1

const char *spindexid = "SPRINDEX";

// TODO: should not be part of SpriteCache, but rather some asset management class?
const char *const SpriteFile::DefaultSpriteFileName = "acsprset.spr";
const char *const SpriteFile::DefaultSpriteIndexName = "sprindex.dat";

SpriteInfo::SpriteInfo()
	: Flags(0)
	, Width(0)
	, Height(0) {
}

SpriteCache::SpriteData::SpriteData()
	: Size(0)
	, Flags(0)
	, Image(nullptr) {
}

SpriteCache::SpriteData::~SpriteData() {
	// TODO: investigate, if it's actually safe/preferred to delete bitmap here
	// (some of these bitmaps may be assigned from outside of the cache)
}


SpriteFile::SpriteFile() {
	_compressed = false;
	_curPos = -2;
}

SpriteCache::SpriteCache(std::vector<SpriteInfo> &sprInfos)
	: _sprInfos(sprInfos) {
	Init();
}

SpriteCache::~SpriteCache() {
	Reset();
}

size_t SpriteCache::GetCacheSize() const {
	return _cacheSize;
}

size_t SpriteCache::GetLockedSize() const {
	return _lockedSize;
}

size_t SpriteCache::GetMaxCacheSize() const {
	return _maxCacheSize;
}

size_t SpriteCache::GetSpriteSlotCount() const {
	return _spriteData.size();
}

sprkey_t SpriteFile::FindTopmostSprite(const std::vector<Bitmap *> &sprites) {
	sprkey_t topmost = -1;
	for (sprkey_t i = 0; i < static_cast<sprkey_t>(sprites.size()); ++i)
		if (sprites[i])
			topmost = i;
	return topmost;
}

void SpriteCache::SetMaxCacheSize(size_t size) {
	_maxCacheSize = size;
}

void SpriteCache::Init() {
	_cacheSize = 0;
	_lockedSize = 0;
	_maxCacheSize = (size_t)DEFAULTCACHESIZE_KB * 1024;
	_liststart = -1;
	_listend = -1;
}

void SpriteCache::Reset() {
	_file.Reset();
	// TODO: find out if it's safe to simply always delete _spriteData.Image with array element
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (_spriteData[i].Image) {
			delete _spriteData[i].Image;
			_spriteData[i].Image = nullptr;
		}
	}
	_spriteData.clear();

	_mrulist.clear();
	_mrubacklink.clear();

	Init();
}

void SpriteCache::SetSprite(sprkey_t index, Bitmap *sprite) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: unable to use index %d", index);
		return;
	}
	if (!sprite) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: attempt to assign nullptr to index %d", index);
		return;
	}
	_spriteData[index].Image = sprite;
	_spriteData[index].Flags = SPRCACHEFLAG_LOCKED; // NOT from asset file
	_spriteData[index].Size = 0;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SetSprite: (external) %d", index);
#endif
}

void SpriteCache::SetEmptySprite(sprkey_t index, bool as_asset) {
	if (index < 0 || EnlargeTo(index) != index) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetEmptySprite: unable to use index %d", index);
		return;
	}
	if (as_asset)
		_spriteData[index].Flags = SPRCACHEFLAG_ISASSET;
	RemapSpriteToSprite0(index);
}

void SpriteCache::SubstituteBitmap(sprkey_t index, Bitmap *sprite) {
	if (!DoesSpriteExist(index)) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SubstituteBitmap: attempt to set for non-existing sprite %d", index);
		return;
	}
	_spriteData[index].Image = sprite;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SubstituteBitmap: %d", index);
#endif
}

void SpriteCache::RemoveSprite(sprkey_t index, bool freeMemory) {
	if (freeMemory)
		delete _spriteData[index].Image;
	InitNullSpriteParams(index);
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemoveSprite: %d", index);
#endif
}

sprkey_t SpriteCache::EnlargeTo(sprkey_t topmost) {
	if (topmost < 0 || topmost > MAX_SPRITE_INDEX)
		return -1;
	if ((size_t)topmost < _spriteData.size())
		return topmost;

	size_t newsize = topmost + 1;
	_sprInfos.resize(newsize);
	_spriteData.resize(newsize);
	_mrulist.resize(newsize);
	_mrubacklink.resize(newsize);
	return topmost;
}

sprkey_t SpriteCache::GetFreeIndex() {
	for (size_t i = MIN_SPRITE_INDEX; i < _spriteData.size(); ++i) {
		// slot empty
		if (!DoesSpriteExist(i)) {
			_sprInfos[i] = SpriteInfo();
			_spriteData[i] = SpriteData();
			return i;
		}
	}
	// enlarge the sprite bank to find a free slot and return the first new free slot
	return EnlargeTo(_spriteData.size());
}

bool SpriteCache::SpriteData::DoesSpriteExist() const {
	return (Image != nullptr) || // HAS loaded bitmap
		((Flags & SPRCACHEFLAG_ISASSET) != 0); // OR found in the game resources
}

bool SpriteCache::SpriteData::IsAssetSprite() const {
	return (Flags & SPRCACHEFLAG_ISASSET) != 0; // found in game resources
}

bool SpriteCache::SpriteData::IsExternalSprite() const {
	return (Image != nullptr) &&  // HAS loaded bitmap
		((Flags & SPRCACHEFLAG_ISASSET) == 0) && // AND NOT found in game resources
		((Flags & SPRCACHEFLAG_REMAPPED) == 0); // AND was NOT remapped to another sprite
}

bool SpriteCache::SpriteData::IsLocked() const {
	return (Flags & SPRCACHEFLAG_LOCKED) != 0;
}

bool SpriteCache::DoesSpriteExist(sprkey_t index) const {
	return index >= 0 && (size_t)index < _spriteData.size() && _spriteData[index].DoesSpriteExist();
}

Bitmap *SpriteCache::operator [] (sprkey_t index) {
	// invalid sprite slot
	if (index < 0 || (size_t)index >= _spriteData.size())
		return nullptr;

	// Externally added sprite, don't put it into MRU list
	if (_spriteData[index].IsExternalSprite())
		return _spriteData[index].Image;

	// Sprite exists in file but is not in mem, load it
	if ((_spriteData[index].Image == nullptr) && _spriteData[index].IsAssetSprite())
		LoadSprite(index);

	// Locked sprite that shouldn't be put into MRU list
	if (_spriteData[index].IsLocked())
		return _spriteData[index].Image;

	if (_liststart < 0) {
		_liststart = index;
		_listend = index;
		_mrulist[index] = END_OF_LIST;
		_mrubacklink[index] = START_OF_LIST;
	} else if (_listend != index) {
		// this is the oldest element being bumped to newest, so update start link
		if (index == _liststart) {
			_liststart = _mrulist[index];
			_mrubacklink[_liststart] = START_OF_LIST;
		}
		// already in list, link previous to next
		else if (_mrulist[index] > 0) {
			_mrulist[_mrubacklink[index]] = _mrulist[index];
			_mrubacklink[_mrulist[index]] = _mrubacklink[index];
		}

		// set this as the newest element in the list
		_mrulist[index] = END_OF_LIST;
		_mrulist[_listend] = index;
		_mrubacklink[index] = _listend;
		_listend = index;
	}

	return _spriteData[index].Image;
}

void SpriteCache::DisposeOldest() {
	if (_liststart < 0)
		return;

	sprkey_t sprnum = _liststart;

	if ((_spriteData[sprnum].Image != nullptr) && !_spriteData[sprnum].IsLocked()) {
		// Free the memory
		// Sprites that are not from the game resources should not normally be in a MRU list;
		// if such is met here there's something wrong with the internal cache logic!
		if (!_spriteData[sprnum].IsAssetSprite()) {
			quitprintf("SpriteCache::DisposeOldest: attempted to remove sprite %d that was added externally or does not exist", sprnum);
		}
		_cacheSize -= _spriteData[sprnum].Size;

		delete _spriteData[sprnum].Image;
		_spriteData[sprnum].Image = nullptr;
	}

	if (_liststart == _listend) {
		// there was one huge sprite, removing it has now emptied the cache completely
		if (_cacheSize > 0) {
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SPRITE CACHE ERROR: Sprite cache should be empty, but still has %d bytes", _cacheSize);
		}
		_mrulist[_liststart] = 0;
		_liststart = -1;
		_listend = -1;
	} else {
		sprkey_t oldstart = _liststart;
		_liststart = _mrulist[_liststart];
		_mrulist[oldstart] = 0;
		_mrubacklink[_liststart] = START_OF_LIST;
		if (oldstart == _liststart) {
			// Somehow, we have got a recursive link to itself, so we
			// the game will freeze (since it is not actually freeing any
			// memory)
			// There must be a bug somewhere causing this, but for now
			// let's just reset the cache
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: CACHE INCONSISTENT: RESETTING\n\tAt size %d (of %d), start %d end %d  fwdlink=%d",
				_cacheSize, _maxCacheSize, oldstart, _listend, _liststart);
			DisposeAll();
		}
	}

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "DisposeOldest: disposed %d, size now %d KB", sprnum, _cacheSize / 1024);
#endif
}

void SpriteCache::DisposeAll() {
	_liststart = -1;
	_listend = -1;
	for (size_t i = 0; i < _spriteData.size(); ++i) {
		if (!_spriteData[i].IsLocked() && // not locked
			_spriteData[i].IsAssetSprite()) // sprite from game resource
		{
			delete _spriteData[i].Image;
			_spriteData[i].Image = nullptr;
		}
		_mrulist[i] = 0;
		_mrubacklink[i] = 0;
	}
	_cacheSize = _lockedSize;
}

void SpriteCache::Precache(sprkey_t index) {
	if (index < 0 || (size_t)index >= _spriteData.size())
		return;

	soff_t sprSize = 0;

	if (_spriteData[index].Image == nullptr)
		sprSize = LoadSprite(index);
	else if (!_spriteData[index].IsLocked())
		sprSize = _spriteData[index].Size;

	// make sure locked sprites can't fill the cache
	_maxCacheSize += sprSize;
	_lockedSize += sprSize;

	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "Precached %d", index);
#endif
}

sprkey_t SpriteCache::GetDataIndex(sprkey_t index) {
	return (_spriteData[index].Flags & SPRCACHEFLAG_REMAPPED) == 0 ? index : 0;
}

void SpriteFile::SeekToSprite(sprkey_t index) {
	// If we didn't just load the previous sprite, seek to it
	if (index != _curPos) {
		_stream->Seek(_spriteData[index].Offset, kSeekBegin);
		_curPos = index;
	}
}

size_t SpriteCache::LoadSprite(sprkey_t index) {
	int hh = 0;

	while (_cacheSize > _maxCacheSize) {
		DisposeOldest();
		hh++;
		if (hh > 1000) {
			Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: STUCK IN FREE_UP_MEM; RESETTING CACHE");
			DisposeAll();
		}
	}

	if (index < 0 || (size_t)index >= _spriteData.size())
		quit("sprite cache array index out of bounds");

	sprkey_t load_index = GetDataIndex(index);
	Bitmap *image;
	HError err = _file.LoadSprite(load_index, image);
	if (!image) {
		Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn,
			"LoadSprite: failed to load sprite %d:\n%s\n - remapping to sprite 0.", index,
			err ? err->FullMessage().GetCStr() : "Sprite does not exist.");
		RemapSpriteToSprite0(index);
		return 0;
	}

	// update the stored width/height
	_sprInfos[index].Width = image->GetWidth();
	_sprInfos[index].Height = image->GetHeight();
	_spriteData[index].Image = image;

	// Stop it adding the sprite to the used list just because it's loaded
	// TODO: this messy hack is required, because initialize_sprite calls operator[]
	// which puts the sprite to the MRU list.
	_spriteData[index].Flags |= SPRCACHEFLAG_LOCKED;

	// TODO: this is ugly: asks the engine to convert the sprite using its own knowledge.
	// And engine assigns new bitmap using SpriteCache::SubstituteBitmap().
	// Perhaps change to the callback function pointer?
	initialize_sprite(index);

	if (index != 0)  // leave sprite 0 locked
		_spriteData[index].Flags &= ~SPRCACHEFLAG_LOCKED;

	// we need to store this because the main program might
	// alter spritewidth/height if it resizes stuff
	size_t size = _sprInfos[index].Width * _sprInfos[index].Height *
		_spriteData[index].Image->GetBPP();
	_spriteData[index].Size = size;
	_cacheSize += size;

#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "Loaded %d, size now %zu KB", index, _cacheSize / 1024);
#endif

	return size;
}

void SpriteCache::RemapSpriteToSprite0(sprkey_t index) {
	_sprInfos[index].Flags = _sprInfos[0].Flags;
	_sprInfos[index].Width = _sprInfos[0].Width;
	_sprInfos[index].Height = _sprInfos[0].Height;
	_spriteData[index].Image = nullptr;
	_spriteData[index].Size = _spriteData[0].Size;
	_spriteData[index].Flags |= SPRCACHEFLAG_REMAPPED;
#ifdef DEBUG_SPRITECACHE
	Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemapSpriteToSprite0: %d", index);
#endif
}

const char *spriteFileSig = " Sprite File ";

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

int SpriteCache::SaveToFile(const String &filename, bool compressOutput, SpriteFileIndex &index) {
	std::vector<Bitmap *> sprites;
	for (const auto &data : _spriteData) {
		// NOTE: this is a horrible hack:
		// because Editor expects slightly different RGB order, it swaps colors
		// when loading them (call to initialize_sprite), so here we basically
		// unfix that fix to save the data in a way that engine will expect.
		// TODO: perhaps adjust the editor to NOT need this?!
		pre_save_sprite(data.Image);
		sprites.push_back(data.Image);
	}
	return _file.SaveToFile(filename, sprites, &_file, compressOutput, index);
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

HError SpriteCache::InitFile(const String &filename, const String &sprindex_filename) {
	std::vector<Size> metrics;
	HError err = _file.OpenFile(filename, sprindex_filename, metrics);
	if (!err)
		return err;

	// Initialize sprite infos
	size_t newsize = metrics.size();
	_sprInfos.resize(newsize);
	_spriteData.resize(newsize);
	_mrulist.resize(newsize);
	_mrubacklink.resize(newsize);
	for (size_t i = 0; i < metrics.size(); ++i) {
		if (!metrics[i].IsNull()) {
			// Existing sprite
			_spriteData[i].Flags = SPRCACHEFLAG_ISASSET;
			_spriteData[i].Image = nullptr;
			get_new_size_for_sprite(i, metrics[i].Width, metrics[i].Height, _sprInfos[i].Width, _sprInfos[i].Height);
		} else {
			// Handle empty slot: remap to sprite 0
			if (i > 0) // FIXME: optimize
				InitNullSpriteParams(i);
		}
	}
	return HError::None();
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

void SpriteCache::DetachFile() {
	_file.Reset();
}

bool SpriteFile::IsFileCompressed() const {
	return _compressed;
}

sprkey_t SpriteFile::GetTopmostSprite() const {
	return (sprkey_t)_spriteData.size() - 1;
}

void SpriteFile::Reset() {
	_stream.reset();
	_curPos = -2;
}

HAGSError SpriteFile::LoadSprite(sprkey_t index, Shared::Bitmap *&sprite) {
	sprite = nullptr;
	if (index < 0 || (size_t)index >= _spriteData.size())
		new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

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
	if (index < 0 || (size_t)index >= _spriteData.size())
		new Error(String::FromFormat("LoadSprite: slot index %d out of bounds (%d - %d).",
			index, 0, _spriteData.size() - 1));

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

HAGSError SpriteFile::OpenFile(const String &filename, const String &sprindex_filename,
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

} // namespace AGS3
