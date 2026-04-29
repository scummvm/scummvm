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
	_isFloppy = false;
	_floppySuspectsOff = _floppyHintBlockOff = _floppyNoteIndexOff = 0;
	_floppyGalleryOff = _floppyTextOff = _floppyKDTextOff = 0;
	_floppySolvedOff = 0;
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

	// Floppy `M*.BIN` uses a completely different header layout from
	// the CD release. Verified via Ghidra of `EEM.EXE` floppy:
	//
	//   `_ReadMystery_Floppy @ 22dc:0178` parses M0..M54 into pointers
	//    held in a global table at 28da:3c87+. The header offsets are:
	//
	//     header[+0..+1]   ???
	//     header[+2..+3]   ???
	//     header[+4..+5]   pointer → SUSPECTS section
	//                      (count byte, then 0xb-byte entries; entry[+4] =
	//                       pic ID, entry[+10] = recolor flag)
	//                      (`_FloppySuspectsPtr` @ 28da:3c8b)
	//     header[+6..+7]   pointer → ???       (28da:3c9f)
	//     header[+8..+9]   pointer → NOTES section
	//                      (7-byte entries indexed by clue ID; used by
	//                       floppy `_DrawNotes` @ 15e0:01e8)
	//                      (`DAT_28da_3c9b`)
	//     header[+0xa..+b] pointer → GALLERY-PORTRAITS section
	//                      (count byte, then variable-length entries
	//                       `5 + name_len` bytes; entry[+0..+1] = u16
	//                       picID, entry[+4] = name length)
	//                      (`_FloppyGalleryPtr` @ 28da:3c87, count =
	//                       `_FloppyNumSuspects` @ 28da:004b)
	//     header[+0xc..+d] pointer → TEXT block (alibi text base; used in
	//                       floppy `_DisplayAlibi` @ 1d40:00df)
	//     header[+0x10..1] pointer → KDTextIndex (`_FloppyKDTextIndexPtr`
	//                       @ 28da:3c93)
	//     header[+0x12..3] pointer → ???       (28da:3c8f)
	//
	//   There is NO fixed-offset numSites / numSuspects / numCONSITEs
	//   field — counts are stored as the FIRST byte of each section.
	//   The CD release refactored this into a flat header at fixed
	//   offsets; our `Mystery::load` here parses the CD layout.
	//
	// Detect the variant from the first u16: CD M0 starts with `0x003e`
	// (initOffset = 62), floppy M0 starts with `0x2286` (a section
	// pointer near end-of-file). When the first u16 is too high to be
	// a CD `_initOffset`, parse as floppy.
	if (readU16(0) > 0x100) {
		_isFloppy = true;
		// Section-pointer header verified via Ghidra of floppy
		// `_ReadMystery_Floppy @ 22dc:0178`,
		// `_DoSiteLoop_Floppy @ 1652:03a3`,
		// and `FUN_1fed_07ed` (BigMap site iteration):
		//
		//   header[+4]   → SITES section
		//                  count byte + 0xb-byte entries; entry[+4] =
		//                  pic ID for BigMap marker, [+6..7] = u16 X,
		//                  [+8..9] = u16 Y, [+10] = recolor flag.
		//   header[+6]   → SITE INDEX (array of u16 offsets to per-site
		//                  data structs; site[+0]=picOff,
		//                  site[+2]=clueBlockOff, site[+8]=speakerInfo)
		//   header[+8]   → NOTES (7-byte entries / clue ID)
		//   header[+0xa] → SUSPECTS / GALLERY portraits
		//   header[+0xc] → TEXT block base
		//   header[+0x10] → KDTextIndex
		//   header[+0x12] → SOLVED CLUE CHAIN
		_floppySuspectsOff  = readU16(0x04);  // SITES
		_floppyHintBlockOff = readU16(0x06);  // SITE INDEX
		_floppyNoteIndexOff = readU16(0x08);
		_floppyGalleryOff   = readU16(0x0a);  // SUSPECTS
		_floppyTextOff      = readU16(0x0c);
		_floppyKDTextOff    = readU16(0x10);
		_floppySolvedOff    = readU16(0x12);

		// header[+0] (the first u16) holds the InitBlock byte offset on
		// floppy too — verified at `FUN_19bb_042f` where `*DAT_28da_3ca5`
		// (deref'd as int *) reads the first u16 of the buffer and uses
		// it as `cVar1 = *(buffer + initOffset)` (caseType byte).
		_initOffset = readU16(0x00);

		// Counts: first byte of each section. Verified at
		// `FUN_1fed_07ed` (`uVar3 = *_FloppySuspectsPtr` then iterates)
		// and `FUN_154e_0045` (`DAT_28da_004b = *DAT_28da_3c87`).
		const byte *sitesSec = (_floppySuspectsOff < _data.size())
								? _data.data() + _floppySuspectsOff : nullptr;
		const byte *susSec   = (_floppyGalleryOff < _data.size())
								? _data.data() + _floppyGalleryOff : nullptr;
		_numSites    = sitesSec ? *sitesSec : 0;
		_numSuspects = susSec   ? *susSec   : 0;
		_numCONSITEs = 0;
		_numCOFFSITEs = 0;

		// Point CD-shaped accessor offsets at the floppy equivalents
		// so existing accessors return the right base for floppy:
		//   siteIndexEntry() → floppy site index (header[+6])
		//   noteIndex()       → floppy notes (header[+8])
		//   galleryData()     → floppy suspects (header[+0xa])
		//   textAt()          → floppy text block (header[+0xc])
		//   kdTextIndex()     → floppy KDTextIndex (header[+0x10])
		//   solvedClueBlock() → floppy solved chain (header[+0x12])
		// Per-section LAYOUTS still differ from CD, so consumers
		// walking entries need `isFloppy()` branches.
		_siteIndexOffset = _floppyHintBlockOff;
		_noteOffset      = _floppyNoteIndexOff;
		_galleryOffset   = _floppyGalleryOff;
		_textOffset      = _floppyTextOff;
		_kdTextOffset    = _floppyKDTextOff;
		_solvedOffset    = _floppySolvedOff;
		_hintOffset      = _floppyHintBlockOff;

		debugC(1, kDebugMystery,
			   "Mystery::load(%u) floppy: sites=0x%04x siteIdx=0x%04x "
			   "notes=0x%04x suspects=0x%04x text=0x%04x kd=0x%04x "
			   "solved=0x%04x  numSites=%u numSuspects=%u",
			   num, _floppySuspectsOff, _floppyHintBlockOff,
			   _floppyNoteIndexOff, _floppyGalleryOff, _floppyTextOff,
			   _floppyKDTextOff, _floppySolvedOff,
			   _numSites, _numSuspects);
		return true;
	}

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

	// Defensive clamp. The floppy mystery file format uses a different
	// header layout (verified by comparing M0.BIN: CD has `numSites =
	// readU16(0x14) = 3`; floppy has `readU16(0x14) = 0x1925`,
	// obviously not a site count). Without a clamp, downstream loops
	// over `_onSites` / `_visitedSite` (capacity 20) blow past the
	// array end. Until the floppy format is fully supported, cap at
	// the array capacity so the engine fails gracefully.
	if (_numSites > kVisitedSiteCap)
		_numSites = kVisitedSiteCap;

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
	// Floppy site index uses 2-byte (u16) entries — verified at
	// `_DoSiteLoop_Floppy @ 1652:03d2` reading `*(int *)
	// ((int)_FloppySiteIndexPtr + siteNum * 2)`. CD uses 6-byte rows.
	const uint stride = _isFloppy ? 2 : 6;
	const uint off = _siteIndexOffset + siteNum * stride;
	if (off + stride > _data.size())
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
	if (_isFloppy) {
		// Floppy: hotspot table sits inside the per-site sub-blob.
		// `site_data[+4..5]` is a u16 file offset to a header byte
		// (count) + N×8-byte rectangles (x1, y1, x2, y2 as u16s) —
		// verified at `FUN_22dc_0b80 @ 22dc:0b80` (the click hit-test
		// loop reads `*(byte *)(buf + site_data[+4])` for the count
		// then `FUN_14c9_0039(... buf + site_data[+4] + 1 + i*8)`
		// for each rectangle).
		const byte *site = siteData(siteNum);
		if (!site || (size_t)(site - _data.data()) + 6 > _data.size())
			return nullptr;
		const uint16 hotspotOff = READ_LE_UINT16(site + 4);
		if (hotspotOff == 0 || hotspotOff + 1 > _data.size())
			return nullptr;
		return _data.data() + hotspotOff + 1;
	}
	const byte *idx = siteIndexEntry(siteNum);
	if (!idx)
		return nullptr;
	const uint16 hotspotOff = READ_LE_UINT16(idx + 4);
	if (hotspotOff >= _data.size())
		return nullptr;
	return _data.data() + hotspotOff;
}

uint16 Mystery::hotspotCount(uint siteNum) const {
	if (_isFloppy) {
		const byte *site = siteData(siteNum);
		if (!site || (size_t)(site - _data.data()) + 6 > _data.size())
			return 0;
		const uint16 hotspotOff = READ_LE_UINT16(site + 4);
		if (hotspotOff == 0 || hotspotOff >= _data.size())
			return 0;
		return (uint16)_data[hotspotOff];
	}
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
	// CD entries are 4 bytes (`u16 textOff; u16 points`); floppy
	// entries are 7 bytes (`u16 ?; u16 jakeOff; u16 jennyOff; u8
	// score`) — verified at `FUN_22dc_05c8 @ 22dc:0843` reading
	// `*(int *)(notes + idx*7 + 2)` (Jake) / `+4` (Jenny).
	if (_galleryOffset <= _noteOffset)
		return 0;
	const uint stride = _isFloppy ? 7 : 4;
	return (uint16)((_galleryOffset - _noteOffset) / stride);
}

const byte *Mystery::kdTextIndex() const {
	if (!isLoaded() || _kdTextOffset >= _data.size())
		return nullptr;
	return _data.data() + _kdTextOffset;
}

const byte *Mystery::mapEntry(uint siteNum) const {
	if (!isLoaded() || siteNum >= _numSites)
		return nullptr;
	if (_isFloppy) {
		// Floppy SITES section: byte[0] = count, then 11-byte entries.
		// Verified at `FUN_1fed_07ed` (BigMap site iteration) where
		// `pcVar2 = _FloppySuspectsPtr` (header[+4]) and the loop reads
		// `*(int *)(pcVar2 + i*0xb + 7)` (X) and `*(int *)(pcVar2 + i*0xb
		// + 9)` (Y) — the +7/+9 offsets are 1-based because pcVar2[0]
		// holds the count, so entry stride 11 starts at byte 1.
		const uint off = _floppySuspectsOff + 1 + siteNum * 11;
		if (off + 11 > _data.size())
			return nullptr;
		return _data.data() + off;
	}
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
