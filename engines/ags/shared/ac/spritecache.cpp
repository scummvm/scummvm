//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
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

#include "ac/common.h" // quit
#include "ac/gamestructdefines.h"
#include "ac/spritecache.h"
#include "core/assetmanager.h"
#include "debug/out.h"
#include "gfx/bitmap.h"
#include "util/compress.h"
#include "util/file.h"
#include "util/stream.h"

using namespace AGS::Common;

// [IKM] We have to forward-declare these because their implementations are in the Engine
extern void initialize_sprite(int);
extern void pre_save_sprite(int);
extern void get_new_size_for_sprite(int, int, int, int &, int &);

#define START_OF_LIST -1
#define END_OF_LIST   -1

const char *spindexid = "SPRINDEX";

// TODO: should not be part of SpriteCache, but rather some asset management class?
const String SpriteCache::DefaultSpriteFileName = "acsprset.spr";
const String SpriteCache::DefaultSpriteIndexName = "sprindex.dat";


SpriteInfo::SpriteInfo()
    : Flags(0)
    , Width(0)
    , Height(0)
{
}

SpriteCache::SpriteData::SpriteData()
    : Offset(0)
    , Size(0)
    , Flags(0)
    , Image(nullptr)
{
}

SpriteCache::SpriteData::~SpriteData()
{
    // TODO: investigate, if it's actually safe/preferred to delete bitmap here
    // (some of these bitmaps may be assigned from outside of the cache)
}


SpriteCache::SpriteCache(std::vector<SpriteInfo> &sprInfos)
    : _sprInfos(sprInfos)
{
    _compressed = false;
    Init();
}

SpriteCache::~SpriteCache()
{
    Reset();
}

size_t SpriteCache::GetCacheSize() const
{
    return _cacheSize;
}

size_t SpriteCache::GetLockedSize() const
{
    return _lockedSize;
}

size_t SpriteCache::GetMaxCacheSize() const
{
    return _maxCacheSize;
}

sprkey_t SpriteCache::GetSpriteSlotCount() const
{
    return _spriteData.size();
}

sprkey_t SpriteCache::FindTopmostSprite() const
{
    sprkey_t topmost = -1;
    for (sprkey_t i = 0; i < static_cast<sprkey_t>(_spriteData.size()); ++i)
        if (DoesSpriteExist(i))
            topmost = i;
    return topmost;
}

void SpriteCache::SetMaxCacheSize(size_t size)
{
    _maxCacheSize = size;
}

void SpriteCache::Init()
{
    _cacheSize = 0;
    _lockedSize = 0;
    _maxCacheSize = (size_t)DEFAULTCACHESIZE_KB * 1024;
    _liststart = -1;
    _listend = -1;
    _lastLoad = -2;
}

void SpriteCache::Reset()
{
    _stream.reset();
    // TODO: find out if it's safe to simply always delete _spriteData.Image with array element
    for (size_t i = 0; i < _spriteData.size(); ++i)
    {
        if (_spriteData[i].Image)
        {
            delete _spriteData[i].Image;
            _spriteData[i].Image = nullptr;
        }
    }
    _spriteData.clear();

    _mrulist.clear();
    _mrubacklink.clear();

    Init();
}

void SpriteCache::SetSprite(sprkey_t index, Bitmap *sprite)
{
    if (index < 0 || EnlargeTo(index) != index)
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: unable to use index %d", index);
        return;
    }
    if (!sprite)
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetSprite: attempt to assign nullptr to index %d", index);
        return;
    }
    _spriteData[index].Image = sprite;
    _spriteData[index].Flags = SPRCACHEFLAG_LOCKED; // NOT from asset file
    _spriteData[index].Offset = 0;
    _spriteData[index].Size = 0;
#ifdef DEBUG_SPRITECACHE
    Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SetSprite: (external) %d", index);
#endif
}

void SpriteCache::SetEmptySprite(sprkey_t index, bool as_asset)
{
    if (index < 0 || EnlargeTo(index) != index)
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SetEmptySprite: unable to use index %d", index);
        return;
    }
    if (as_asset)
        _spriteData[index].Flags = SPRCACHEFLAG_ISASSET;
    RemapSpriteToSprite0(index);
}

void SpriteCache::SubstituteBitmap(sprkey_t index, Common::Bitmap *sprite)
{
    if (!DoesSpriteExist(index))
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SubstituteBitmap: attempt to set for non-existing sprite %d", index);
        return;
    }
    _spriteData[index].Image = sprite;
#ifdef DEBUG_SPRITECACHE
    Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "SubstituteBitmap: %d", index);
#endif
}

void SpriteCache::RemoveSprite(sprkey_t index, bool freeMemory)
{
    if (freeMemory)
        delete _spriteData[index].Image;
    InitNullSpriteParams(index);
#ifdef DEBUG_SPRITECACHE
    Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemoveSprite: %d", index);
#endif
}

sprkey_t SpriteCache::EnlargeTo(sprkey_t topmost)
{
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

sprkey_t SpriteCache::GetFreeIndex()
{
    for (size_t i = MIN_SPRITE_INDEX; i < _spriteData.size(); ++i)
    {
        // slot empty
        if (!DoesSpriteExist(i))
        {
            _sprInfos[i] = SpriteInfo();
            _spriteData[i] = SpriteData();
            return i;
        }
    }
    // enlarge the sprite bank to find a free slot and return the first new free slot
    return EnlargeTo(_spriteData.size());
}

bool SpriteCache::SpriteData::DoesSpriteExist() const
{
    return (Image != nullptr) || // HAS loaded bitmap
        ((Flags & SPRCACHEFLAG_ISASSET) != 0); // OR found in the game resources
}

bool SpriteCache::SpriteData::IsAssetSprite() const
{
    return (Flags & SPRCACHEFLAG_ISASSET) != 0; // found in game resources
}

bool SpriteCache::SpriteData::IsExternalSprite() const
{
    return (Image != nullptr) &&  // HAS loaded bitmap
        ((Flags & SPRCACHEFLAG_ISASSET) == 0) && // AND NOT found in game resources
        ((Flags & SPRCACHEFLAG_REMAPPED) == 0); // AND was NOT remapped to another sprite
}

bool SpriteCache::SpriteData::IsLocked() const
{
    return (Flags & SPRCACHEFLAG_LOCKED) != 0;
}

bool SpriteCache::DoesSpriteExist(sprkey_t index) const
{
    return index >= 0 && (size_t)index < _spriteData.size() && _spriteData[index].DoesSpriteExist();
}

Bitmap *SpriteCache::operator [] (sprkey_t index)
{
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

    if (_liststart < 0)
    {
        _liststart = index;
        _listend = index;
        _mrulist[index] = END_OF_LIST;
        _mrubacklink[index] = START_OF_LIST;
    } 
    else if (_listend != index)
    {
        // this is the oldest element being bumped to newest, so update start link
        if (index == _liststart)
        {
            _liststart = _mrulist[index];
            _mrubacklink[_liststart] = START_OF_LIST;
        }
        // already in list, link previous to next
        else if (_mrulist[index] > 0)
        {
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

void SpriteCache::DisposeOldest()
{
    if (_liststart < 0)
        return;

    sprkey_t sprnum = _liststart;

    if ((_spriteData[sprnum].Image != nullptr) && !_spriteData[sprnum].IsLocked())
    {
        // Free the memory
        // Sprites that are not from the game resources should not normally be in a MRU list;
        // if such is met here there's something wrong with the internal cache logic!
        if (!_spriteData[sprnum].IsAssetSprite())
        {
            quitprintf("SpriteCache::DisposeOldest: attempted to remove sprite %d that was added externally or does not exist", sprnum);
        }
        _cacheSize -= _spriteData[sprnum].Size;

        delete _spriteData[sprnum].Image;
        _spriteData[sprnum].Image = nullptr;
    }

    if (_liststart == _listend)
    {
        // there was one huge sprite, removing it has now emptied the cache completely
        if (_cacheSize > 0)
        {
            Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "SPRITE CACHE ERROR: Sprite cache should be empty, but still has %d bytes", _cacheSize);
        }
        _mrulist[_liststart] = 0;
        _liststart = -1;
        _listend = -1;
    }
    else
    {
        sprkey_t oldstart = _liststart;
        _liststart = _mrulist[_liststart];
        _mrulist[oldstart] = 0;
        _mrubacklink[_liststart] = START_OF_LIST;
        if (oldstart == _liststart)
        {
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

void SpriteCache::DisposeAll()
{
    _liststart = -1;
    _listend = -1;
    for (size_t i = 0; i < _spriteData.size(); ++i)
    {
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

void SpriteCache::Precache(sprkey_t index)
{
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

sprkey_t SpriteCache::GetDataIndex(sprkey_t index)
{
    return (_spriteData[index].Flags & SPRCACHEFLAG_REMAPPED) == 0 ? index : 0;
}

void SpriteCache::SeekToSprite(sprkey_t index)
{
    // If we didn't just load the previous sprite, seek to it
    if (index - 1 != _lastLoad)
        _stream->Seek(_spriteData[index].Offset, kSeekBegin);
}

size_t SpriteCache::LoadSprite(sprkey_t index)
{
    int hh = 0;

    while (_cacheSize > _maxCacheSize)
    {
        DisposeOldest();
        hh++;
        if (hh > 1000)
        {
            Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "RUNTIME CACHE ERROR: STUCK IN FREE_UP_MEM; RESETTING CACHE");
            DisposeAll();
        }
    }

    if (index < 0 || (size_t)index >= _spriteData.size())
        quit("sprite cache array index out of bounds");

    sprkey_t load_index = GetDataIndex(index);
    SeekToSprite(load_index);

    int coldep = _stream->ReadInt16();

    if (coldep == 0)
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Error, "LoadSprite: asked to load sprite %d (for slot %d) which does not exist.", load_index, index);
        _lastLoad = load_index;
        return 0;
    }

    int wdd = _stream->ReadInt16();
    int htt = _stream->ReadInt16();
    // update the stored width/height
    _sprInfos[index].Width = wdd;
    _sprInfos[index].Height = htt;

    _spriteData[index].Image = BitmapHelper::CreateBitmap(wdd, htt, coldep * 8);
    if (_spriteData[index].Image == nullptr)
    {
        Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Warn, "LoadSprite: failed to init sprite %d, remapping to sprite 0.", index);
        RemapSpriteToSprite0(index);
        return 0;
    }

    Bitmap *image = _spriteData[index].Image;
    if (this->_compressed) 
    {
        _stream->ReadInt32(); // skip data size
        UnCompressSprite(image, _stream.get());
    }
    else
    {
        if (coldep == 1)
        {
            for (hh = 0; hh < htt; hh++)
                _stream->ReadArray(&image->GetScanLineForWriting(hh)[0], coldep, wdd);
        }
        else if (coldep == 2)
        {
            for (hh = 0; hh < htt; hh++)
                _stream->ReadArrayOfInt16((int16_t*)&image->GetScanLineForWriting(hh)[0], wdd);
        }
        else
        {
            for (hh = 0; hh < htt; hh++)
                _stream->ReadArrayOfInt32((int32_t*)&image->GetScanLineForWriting(hh)[0], wdd);
        }
    }

    _lastLoad = load_index;

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
    size_t size = _sprInfos[index].Width * _sprInfos[index].Height * coldep;
    _spriteData[index].Size = size;
    _cacheSize += size;

#ifdef DEBUG_SPRITECACHE
    Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "Loaded %d, size now %u KB", index, _cacheSize / 1024);
#endif

    return size;
}

void SpriteCache::RemapSpriteToSprite0(sprkey_t index)
{
    _sprInfos[index].Flags = _sprInfos[0].Flags;
    _sprInfos[index].Width = _sprInfos[0].Width;
    _sprInfos[index].Height = _sprInfos[0].Height;
    _spriteData[index].Image = nullptr;
    _spriteData[index].Offset = _spriteData[0].Offset;
    _spriteData[index].Size = _spriteData[0].Size;
    _spriteData[index].Flags |= SPRCACHEFLAG_REMAPPED;
#ifdef DEBUG_SPRITECACHE
    Debug::Printf(kDbgGroup_SprCache, kDbgMsg_Debug, "RemapSpriteToSprite0: %d", index);
#endif
}

const char *spriteFileSig = " Sprite File ";

void SpriteCache::CompressSprite(Bitmap *sprite, Stream *out)
{
    const int depth = sprite->GetBPP();
    if (depth == 1)
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cpackbitl(&sprite->GetScanLineForWriting(y)[0], sprite->GetWidth(), out);
    }
    else if (depth == 2)
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cpackbitl16((unsigned short *)&sprite->GetScanLine(y)[0], sprite->GetWidth(), out);
    }
    else
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cpackbitl32((unsigned int *)&sprite->GetScanLine(y)[0], sprite->GetWidth(), out);
    }
}

void SpriteCache::UnCompressSprite(Bitmap *sprite, Stream *in)
{
    const int depth = sprite->GetBPP();
    if (depth == 1)
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cunpackbitl(&sprite->GetScanLineForWriting(y)[0], sprite->GetWidth(), in);
    }
    else if (depth == 2)
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cunpackbitl16((unsigned short*)&sprite->GetScanLineForWriting(y)[0], sprite->GetWidth(), in);
    }
    else
    {
        for (int y = 0; y < sprite->GetHeight(); y++)
            cunpackbitl32((unsigned int*)&sprite->GetScanLineForWriting(y)[0], sprite->GetWidth(), in);
    }
}

int SpriteCache::SaveToFile(const char *filename, bool compressOutput, SpriteFileIndex &index)
{
    Stream *output = Common::File::CreateFile(filename);
    if (output == nullptr)
        return -1;

    if (compressOutput)
    {
        // re-open the file so that it can be seeked
        delete output;
        output = File::OpenFile(filename, Common::kFile_Open, Common::kFile_ReadWrite); // CHECKME why mode was "r+" here?
        if (output == nullptr)
            return -1;
    }

    int spriteFileIDCheck = (int)time(nullptr);

    // sprite file version
    output->WriteInt16(kSprfVersion_Current);

    output->WriteArray(spriteFileSig, strlen(spriteFileSig), 1);

    output->WriteInt8(compressOutput ? 1 : 0);
    output->WriteInt32(spriteFileIDCheck);

    sprkey_t lastslot = FindTopmostSprite();
    output->WriteInt32(lastslot);

    // allocate buffers to store the indexing info
    sprkey_t numsprits = lastslot + 1;
    std::vector<int16_t> spritewidths, spriteheights;
    std::vector<soff_t> spriteoffs;
    spritewidths.resize(numsprits);
    spriteheights.resize(numsprits);
    spriteoffs.resize(numsprits);

    const int memBufferSize = 100000;
    char *memBuffer = new char[memBufferSize];

    for (sprkey_t i = 0; i <= lastslot; ++i)
    {
        spriteoffs[i] = output->GetPosition();

        // if compressing uncompressed sprites, load the sprite into memory
        if ((_spriteData[i].Image == nullptr) && (this->_compressed != compressOutput))
            (*this)[i];

        if (_spriteData[i].Image != nullptr)
        {
            // image in memory -- write it out
            pre_save_sprite(i);
            Bitmap *image = _spriteData[i].Image;
            int bpss = image->GetColorDepth() / 8;
            spritewidths[i] = image->GetWidth();
            spriteheights[i] = image->GetHeight();
            output->WriteInt16(bpss);
            output->WriteInt16(spritewidths[i]);
            output->WriteInt16(spriteheights[i]);

            if (compressOutput)
            {
                size_t lenloc = output->GetPosition();
                // write some space for the length data
                output->WriteInt32(0);

                CompressSprite(image, output);

                size_t fileSizeSoFar = output->GetPosition();
                // write the length of the compressed data
                output->Seek(lenloc, kSeekBegin);
                output->WriteInt32((fileSizeSoFar - lenloc) - 4);
                output->Seek(0, kSeekEnd);
            }
            else
            {
                output->WriteArray(image->GetDataForWriting(), spritewidths[i] * bpss, spriteheights[i]);
            }
            continue;
        }

        if (_spriteData[i].Offset == 0)
        {
            // sprite doesn't exist
            output->WriteInt16(0); // colour depth
            spritewidths[i] = 0;
            spriteheights[i] = 0;
            spriteoffs[i] = 0;
            continue;
        }

        // not in memory -- seek to it in the source file
        sprkey_t load_index = GetDataIndex(i);
        SeekToSprite(load_index);
        _lastLoad = load_index;

        short colDepth = _stream->ReadInt16();
        output->WriteInt16(colDepth);

        if (colDepth == 0)
            continue;

        if (this->_compressed != compressOutput)
        {
            // shouldn't be able to get here
            delete [] memBuffer;
            delete output;
            return -2;
        }

        // and copy the data across
        int width = _stream->ReadInt16();
        int height = _stream->ReadInt16();

        spritewidths[i] = width;
        spriteheights[i] = height;

        output->WriteInt16(width);
        output->WriteInt16(height);

        int sizeToCopy;
        if (this->_compressed)
        {
            sizeToCopy = _stream->ReadInt32();
            output->WriteInt32(sizeToCopy);
        }
        else
        {
            sizeToCopy = width * height * (int)colDepth;
        }

        while (sizeToCopy > memBufferSize)
        {
            _stream->ReadArray(memBuffer, memBufferSize, 1);
            output->WriteArray(memBuffer, memBufferSize, 1);
            sizeToCopy -= memBufferSize;
        }

        _stream->ReadArray(memBuffer, sizeToCopy, 1);
        output->WriteArray(memBuffer, sizeToCopy, 1);
    }

    delete [] memBuffer;
    delete output;

    index.SpriteFileIDCheck = spriteFileIDCheck;
    index.LastSlot = lastslot;
    index.SpriteCount = numsprits;
    index.Widths = spritewidths;
    index.Heights = spriteheights;
    index.Offsets = spriteoffs;
    return 0;
}

int SpriteCache::SaveSpriteIndex(const char *filename, const SpriteFileIndex &index)
{
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
    if (index.SpriteCount > 0)
    {
        out->WriteArrayOfInt16(&index.Widths.front(), index.Widths.size());
        out->WriteArrayOfInt16(&index.Heights.front(), index.Heights.size());
        out->WriteArrayOfInt64(&index.Offsets.front(), index.Offsets.size());
    }
    delete out;
    return 0;
}

HError SpriteCache::InitFile(const char *filename, const char *sprindex_filename)
{
    SpriteFileVersion vers;
    char buff[20];
    soff_t spr_initial_offs = 0;
    int spriteFileID = 0;

    _stream.reset(Common::AssetManager::OpenAsset(filename));
    if (_stream == nullptr)
        return new Error(String::FromFormat("Failed to open spriteset file '%s'.", filename));

    spr_initial_offs = _stream->GetPosition();

    vers = (SpriteFileVersion)_stream->ReadInt16();
    // read the "Sprite File" signature
    _stream->ReadArray(&buff[0], 13, 1);

    if (vers < kSprfVersion_Uncompressed || vers > kSprfVersion_Current)
    {
        _stream.reset();
        return new Error(String::FromFormat("Unsupported spriteset format (requested %d, supported %d - %d).", vers, kSprfVersion_Uncompressed, kSprfVersion_Current));
    }

    // unknown version
    buff[13] = 0;
    if (strcmp(buff, spriteFileSig))
    {
        _stream.reset();
        return new Error("Uknown spriteset format.");
    }

    if (vers == kSprfVersion_Uncompressed)
    {
        this->_compressed = false;
    }
    else if (vers == kSprfVersion_Compressed)
    {
        this->_compressed = true;
    }
    else if (vers >= kSprfVersion_Last32bit)
    {
        this->_compressed = (_stream->ReadInt8() == 1);
        spriteFileID = _stream->ReadInt32();
    }

    if (vers < kSprfVersion_Compressed)
    {
        // skip the palette
        _stream->Seek(256 * 3); // sizeof(RGB) * 256
    }

    sprkey_t topmost;
    if (vers < kSprfVersion_HighSpriteLimit)
        topmost = _stream->ReadInt16();
    else
        topmost = _stream->ReadInt32();
    if (vers < kSprfVersion_Uncompressed)
        topmost = 200;

    EnlargeTo(topmost);

    // if there is a sprite index file, use it
    if (LoadSpriteIndexFile(sprindex_filename, spriteFileID, spr_initial_offs, topmost))
    {
        // Succeeded
        return HError::None();
    }

    // Failed, index file is invalid; index sprites manually
    return RebuildSpriteIndex(_stream.get(), topmost, vers);
}

HError SpriteCache::RebuildSpriteIndex(AGS::Common::Stream *in, sprkey_t topmost, SpriteFileVersion vers)
{
    for (sprkey_t i = 0; i <= topmost; ++i)
    {
        _spriteData[i].Offset = in->GetPosition();
        _spriteData[i].Flags = 0;

        int coldep = in->ReadInt16();

        if (coldep == 0)
        {
            // Empty slot
            if (i > 0)
                InitNullSpriteParams(i);

            if (in->EOS())
                break;
            continue;
        }

        if (in->EOS())
            break;

        if ((size_t)i >= _spriteData.size())
            break;

        _spriteData[i].Flags = SPRCACHEFLAG_ISASSET;
        _spriteData[i].Image = nullptr;

        int wdd = in->ReadInt16();
        int htt = in->ReadInt16();
        _sprInfos[i].Width = wdd;
        _sprInfos[i].Height = htt;
        get_new_size_for_sprite(i, wdd, htt, _sprInfos[i].Width, _sprInfos[i].Height);

        size_t spriteDataSize;
        if (vers == kSprfVersion_Compressed)
        {
            spriteDataSize = in->ReadInt32();
        }
        else if (vers >= kSprfVersion_Last32bit)
        {
            spriteDataSize = this->_compressed ? in->ReadInt32() : wdd * coldep * htt;
        }
        else
        {
            spriteDataSize = wdd * coldep * htt;
        }
        in->Seek(spriteDataSize);
    }
    return HError::None();
}

bool SpriteCache::LoadSpriteIndexFile(const char *filename, int expectedFileID, soff_t spr_initial_offs, sprkey_t topmost)
{
    Stream *fidx = Common::AssetManager::OpenAsset(filename);
    if (fidx == nullptr) 
    {
        return false;
    }

    char buffer[9];
    // check "SPRINDEX" id
    fidx->ReadArray(&buffer[0], strlen(spindexid), 1);
    buffer[8] = 0;
    if (strcmp(buffer, spindexid))
    {
        delete fidx;
        return false;
    }
    // check version
    SpriteIndexFileVersion vers = (SpriteIndexFileVersion)fidx->ReadInt32();
    if (vers < kSpridxfVersion_Initial || vers > kSpridxfVersion_Current)
    {
        delete fidx;
        return false;
    }
    if (vers >= kSpridxfVersion_Last32bit)
    {
        if (fidx->ReadInt32() != expectedFileID)
        {
            delete fidx;
            return false;
        }
    }

    sprkey_t topmost_index = fidx->ReadInt32();
    // end index+1 should be the same as num sprites
    if (fidx->ReadInt32() != topmost_index + 1)
    {
        delete fidx;
        return false;
    }

    if (topmost_index != topmost)
    {
        delete fidx;
        return false;
    }

    sprkey_t numsprits = topmost_index + 1;
    short *rspritewidths = new short[numsprits];
    short *rspriteheights = new short[numsprits];
    soff_t *spriteoffs = new soff_t[numsprits];

    fidx->ReadArrayOfInt16(&rspritewidths[0], numsprits);
    fidx->ReadArrayOfInt16(&rspriteheights[0], numsprits);
    if (vers <= kSpridxfVersion_Last32bit)
    {
        for (sprkey_t i = 0; i < numsprits; ++i)
            spriteoffs[i] = fidx->ReadInt32();
    }
    else // large file support
    {
        fidx->ReadArrayOfInt64(spriteoffs, numsprits);
    }

    for (sprkey_t i = 0; i <= topmost_index; ++i)
    {
        if (spriteoffs[i] != 0)
        {
            _spriteData[i].Flags = SPRCACHEFLAG_ISASSET;
            _spriteData[i].Offset = spriteoffs[i] + spr_initial_offs;
            get_new_size_for_sprite(i, rspritewidths[i], rspriteheights[i], _sprInfos[i].Width, _sprInfos[i].Height);
        }
        else if (i > 0)
        {
            InitNullSpriteParams(i);
        }
    }

    delete [] rspritewidths;
    delete [] rspriteheights;
    delete [] spriteoffs;
    delete fidx;
    return true;
}

void SpriteCache::DetachFile()
{
    _stream.reset();
    _lastLoad = -2;
}

int SpriteCache::AttachFile(const char *filename)
{
    _stream.reset(Common::AssetManager::OpenAsset((char *)filename));
    if (_stream == nullptr)
        return -1;
    return 0;
}

bool SpriteCache::IsFileCompressed() const
{
    return _compressed;
}
