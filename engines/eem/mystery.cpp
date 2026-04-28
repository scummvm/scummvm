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

#include "common/debug.h"
#include "common/file.h"
#include "common/random.h"
#include "common/serializer.h"
#include "common/textconsole.h"

#include "eem/detection.h"
#include "eem/mystery.h"

namespace EEM {

uint16 Mystery::readU16(uint offset) const {
	if (offset + 2 > _data.size())
		return 0;
	return READ_LE_UINT16(_data.data() + offset);
}

void Mystery::clear() {
	_data.clear();
	_number = 0;
	_initOffset = _mapOffset = _siteIndexOffset = _textOffset = 0;
	_noteOffset = _galleryOffset = _kdTextOffset = 0;
	_solvedOffset = _hintOffset = 0;
	_numSites = 0;
	_numSuspects = _numCONSITEs = _numCOFFSITEs = 0;
	memset(_aChain, 0, sizeof(_aChain));
	memset(_bChain, 0, sizeof(_bChain));
	memset(_cChain, 0, sizeof(_cChain));
	memset(_cluesFound, 0, sizeof(_cluesFound));
	memset(_noteSelected, 0, sizeof(_noteSelected));
	memset(_hotSpotsSeen, 0, sizeof(_hotSpotsSeen));
	memset(_inGallery, 0, sizeof(_inGallery));
	memset(_newOrder, 0, sizeof(_newOrder));
	memset(_visitedSite, 0, sizeof(_visitedSite));
	memset(_onSites, 0, sizeof(_onSites));
	_sawCOFFSITEs = _sawCONSITEs = _sawHelpHint = _solvedPuzzle = false;
	_firstTry = true;
	_searchLocationNumber = _siteNumber = 0xFFFF;
	_lastSite = 0x1B;
}

bool Mystery::load(uint num, Common::RandomSource *rng) {
	const Common::String fname = Common::String::format("M%u.BIN", num);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("Mystery::load: cannot open %s", fname.c_str());
		return false;
	}

	const int32 size = f.size();
	if (size <= 64) {
		warning("Mystery::load: %s too small (%d bytes)", fname.c_str(), size);
		return false;
	}

	// Stage to a temporary buffer so a short read leaves the previous
	// mystery state intact instead of half-clobbering `_data`.
	Common::Array<byte> staging(size);
	if (f.read(staging.data(), size) != (uint32)size) {
		warning("Mystery::load: short read on %s", fname.c_str());
		return false;
	}
	f.close();

	_data = staging;
	_number = num;

	// Header is 16-bit-word indexed (matches `int *piVar1 = __Mystery; piVar1[N]`).
	_initOffset      = readU16(0  * 2);
	_mapOffset       = readU16(2  * 2);
	_siteIndexOffset = readU16(3  * 2);
	_textOffset      = readU16(4  * 2);
	_noteOffset      = readU16(5  * 2);
	_galleryOffset   = readU16(6  * 2);
	_kdTextOffset    = readU16(7  * 2);
	_solvedOffset    = readU16(8  * 2);
	_hintOffset      = readU16(9  * 2);

	_numSites    = readU16(10 * 2);
	_numSuspects = (uint8)readU16(13 * 2);
	_numCONSITEs = (uint8)readU16(14 * 2);
	_numCOFFSITEs = (uint8)readU16(15 * 2);

	for (uint i = 0; i < kChainLen; i++) {
		_aChain[i] = readU16((16 + i) * 2);
		_bChain[i] = readU16((21 + i) * 2);
		_cChain[i] = readU16((26 + i) * 2);
	}

	// Per-mystery runtime state — `_ReadMystery` zeroes these at load.
	memset(_cluesFound, 0, sizeof(_cluesFound));
	memset(_noteSelected, 0, sizeof(_noteSelected));
	memset(_hotSpotsSeen, 0, sizeof(_hotSpotsSeen));
	memset(_inGallery, 0, sizeof(_inGallery));
	// `_NewOrder` in the original randomly cycles the gallery positions
	// per playthrough. For consistency between clue side-effects (which
	// write to `_inGallery[_newOrder[galIdx]]`) and gallery rendering
	// (which iterates logical indices), we keep the identity mapping.
	// If the original's randomized positioning is required later, both
	// the side-effect path AND the rendering path need to use it together.
	(void)rng;
	for (uint i = 0; i < kGalleryCap; i++)
		_newOrder[i] = (uint8)i;
	memset(_visitedSite, 0, sizeof(_visitedSite));
	memset(_onSites, 0, sizeof(_onSites));
	_sawCOFFSITEs = _sawCONSITEs = _sawHelpHint = _solvedPuzzle = false;
	_firstTry = true;
	_searchLocationNumber = _siteNumber = 0xFFFF;
	_lastSite = 0x1B; // Sentinel matching _ReadMystery's `_LastSite = 0x1b`.

	debugC(1, kDebugMystery, "Loaded %s (%d B): %u sites, %u suspects, "
		   "CON=%u COFF=%u, init=0x%04x site=0x%04x text=0x%04x",
		   fname.c_str(), size, _numSites, _numSuspects,
		   _numCONSITEs, _numCOFFSITEs,
		   _initOffset, _siteIndexOffset, _textOffset);
	return true;
}

const byte *Mystery::siteIndexEntry(uint siteNum) const {
	if (!isLoaded() || siteNum >= _numSites)
		return nullptr;
	const uint off = _siteIndexOffset + siteNum * 6;
	if (off + 6 > _data.size())
		return nullptr;
	return _data.data() + off;
}

const byte *Mystery::siteData(uint siteNum) const {
	const byte *idx = siteIndexEntry(siteNum);
	if (!idx)
		return nullptr;
	const uint16 dataOff = READ_LE_UINT16(idx);
	if (dataOff >= _data.size())
		return nullptr;
	return _data.data() + dataOff;
}

const byte *Mystery::hotspots(uint siteNum) const {
	const byte *idx = siteIndexEntry(siteNum);
	if (!idx)
		return nullptr;
	const uint16 hotspotOff = READ_LE_UINT16(idx + 4);
	if (hotspotOff >= _data.size())
		return nullptr;
	return _data.data() + hotspotOff;
}

uint16 Mystery::hotspotCount(uint siteNum) const {
	const byte *site = siteData(siteNum);
	if (!site || (size_t)(site - _data.data()) + 8 > _data.size())
		return 0;
	return READ_LE_UINT16(site + 6);
}

const char *Mystery::textAt(uint16 offset) const {
	if (!isLoaded())
		return "";
	const uint pos = _textOffset + offset;
	if (pos >= _data.size())
		return "";
	return (const char *)(_data.data() + pos);
}

const byte *Mystery::initBlock() const {
	if (!isLoaded() || _initOffset >= _data.size())
		return nullptr;
	return _data.data() + _initOffset;
}

const byte *Mystery::galleryData() const {
	if (!isLoaded() || _galleryOffset >= _data.size())
		return nullptr;
	return _data.data() + _galleryOffset;
}

const byte *Mystery::noteIndex() const {
	if (!isLoaded() || _noteOffset >= _data.size())
		return nullptr;
	return _data.data() + _noteOffset;
}

uint16 Mystery::noteIndexCount() const {
	if (!isLoaded())
		return 0;
	// NoteIndex runs from _noteOffset to the start of GalleryData.
	if (_galleryOffset <= _noteOffset)
		return 0;
	return (uint16)((_galleryOffset - _noteOffset) / 4);
}

const byte *Mystery::kdTextIndex() const {
	if (!isLoaded() || _kdTextOffset >= _data.size())
		return nullptr;
	return _data.data() + _kdTextOffset;
}

const byte *Mystery::mapEntry(uint siteNum) const {
	if (!isLoaded() || siteNum >= _numSites)
		return nullptr;
	const uint off = _mapOffset + siteNum * 14;
	if (off + 14 > _data.size())
		return nullptr;
	return _data.data() + off;
}

bool Mystery::isGuilty(uint suspectIdx) const {
	// `_WITCH @ 1df2:089f`: `if (GalleryData[i*0x46 + 0x02] == -1)
	// _DisplayCorrect(); else _DisplayAlibi(...)`. Innocent suspects
	// store their alibi-text TextBlock offset at +0x02; the guilty
	// one stores the sentinel 0xFFFF.
	const byte *gd = galleryData();
	if (!gd || suspectIdx >= _numSuspects)
		return false;
	const uint16 off = READ_LE_UINT16(gd + suspectIdx * 0x46 + 0x02);
	return off == 0xFFFF;
}

uint16 Mystery::alibiTextOffset(uint suspectIdx) const {
	const byte *gd = galleryData();
	if (!gd || suspectIdx >= _numSuspects)
		return 0xFFFF;
	return READ_LE_UINT16(gd + suspectIdx * 0x46 + 0x02);
}

const byte *Mystery::hintBlock() const {
	// Header word at index 9 (`_hintOffset`) — used by `_KDHelp @
	// 1560:010a`'s per-chain-clue hint table. Each pair-of-bytes is
	// a TextBlock offset for the corresponding `_AChain` entry, or
	// `0xFFFF` if no hint is defined for that chain position.
	if (!isLoaded() || _hintOffset == 0 || _hintOffset >= _data.size())
		return nullptr;
	return _data.data() + _hintOffset;
}

const byte *Mystery::solvedClueBlock() const {
	if (!isLoaded() || _solvedOffset == 0 || _solvedOffset >= _data.size())
		return nullptr;
	return _data.data() + _solvedOffset;
}

int Mystery::selectedPoints() const {
	const byte *ni = noteIndex();
	const uint16 cnt = noteIndexCount();
	if (!ni || cnt == 0)
		return 0;
	int total = 0;
	for (uint i = 0; i < cnt && i < kCluesFoundCap; i++) {
		if (!_noteSelected[i])
			continue;
		// Each NoteIndex entry is 4 bytes: u16 textOff + u16 points.
		const uint16 pts = READ_LE_UINT16(ni + i * 4 + 2);
		total += (int)(int16)pts;
	}
	return total;
}

void Mystery::syncState(Common::Serializer &s) {
	s.syncBytes(_cluesFound, kCluesFoundCap);
	s.syncBytes(_noteSelected, kCluesFoundCap);
	s.syncArray(_hotSpotsSeen, kHotSpotsCap, Common::Serializer::Uint16LE);
	s.syncArray(_inGallery,    kGalleryCap,  Common::Serializer::Uint16LE);
	s.syncBytes(_newOrder, kGalleryCap);
	s.syncArray(_visitedSite, kVisitedSiteCap, Common::Serializer::Uint16LE);
	s.syncArray(_onSites,     kVisitedSiteCap, Common::Serializer::Uint16LE);
	s.syncAsByte(_sawCOFFSITEs);
	s.syncAsByte(_sawCONSITEs);
	s.syncAsByte(_sawHelpHint);
	s.syncAsByte(_solvedPuzzle);
	s.syncAsByte(_firstTry);
	s.syncAsUint16LE(_searchLocationNumber);
	s.syncAsUint16LE(_siteNumber);
	s.syncAsUint16LE(_lastSite);
}

} // End of namespace EEM
