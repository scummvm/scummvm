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
	_floppySiteAnimData.clear();
	memset(_floppySiteAnimSiteOff, 0, sizeof(_floppySiteAnimSiteOff));
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
	_seenCOFFSITEs = _seenCONSITEs = 0;
	_firstTry = true;
	_searchLocationNumber = _siteNumber = 0xFFFF;
	_lastSite = 0x1B;
	_pendingSiteJump = 0;
	_siteReturnDepth = 0;
	memset(_siteReturnStack, 0, sizeof(_siteReturnStack));
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

	// Floppy M*.BIN uses a different header layout from CD.
	// _ReadMystery_Floppy @ 22dc:0178. Section-pointer header:
	//   header[+0]    InitBlock byte offset (caseType byte at *(buf+initOff))
	//   header[+4]    SITES: count byte + 11-byte entries
	//                 (entry[+4]=picID, [+6..7]=u16 X, [+8..9]=u16 Y, [+10]=recolor)
	//   header[+6]    SITE INDEX (array of u16 offsets to per-site structs)
	//   header[+8]    NOTES (7-byte entries / clue ID; _DrawNotes_Floppy @ 15e0:01e8)
	//   header[+0xa]  GALLERY portraits: count byte + variable `5+nameLen` entries
	//                 (entry[+0..1]=u16 picID, [+4]=nameLen)
	//   header[+0xc]  TEXT block (alibi base; _DisplayAlibi_Floppy @ 1d40:00df)
	//   header[+0x10] KDTextIndex
	//   header[+0x12] SOLVED CLUE CHAIN
	// Counts live in the FIRST BYTE of each section, not the header.
	// Detect via first u16: CD M0 = 0x003e (initOffset), floppy M0 = 0x2286
	// (section pointer near EOF).
	if (readU16(0) > 0x100) {
		_isFloppy = true;
		_floppySuspectsOff  = readU16(0x04);  // SITES
		_floppyHintBlockOff = readU16(0x06);  // SITE INDEX
		_floppyNoteIndexOff = readU16(0x08);
		_floppyGalleryOff   = readU16(0x0a);  // SUSPECTS
		_floppyTextOff      = readU16(0x0c);
		_floppyKDTextOff    = readU16(0x10);
		_floppySolvedOff    = readU16(0x12);
		_initOffset = readU16(0x00);

		const byte *sitesSec = (_floppySuspectsOff < _data.size())
								? _data.data() + _floppySuspectsOff : nullptr;
		const byte *susSec   = (_floppyGalleryOff < _data.size())
								? _data.data() + _floppyGalleryOff : nullptr;
		_numSites    = sitesSec ? *sitesSec : 0;
		_numSuspects = susSec   ? *susSec   : 0;
		_numCONSITEs = 0;
		_numCOFFSITEs = 0;

		// Point CD-shaped accessors at the floppy equivalents.
		// Per-section LAYOUTS still differ, so consumers walking entries
		// need `isFloppy()` branches.
		_siteIndexOffset = _floppyHintBlockOff;
		_noteOffset      = _floppyNoteIndexOff;
		_galleryOffset   = _floppyGalleryOff;
		_textOffset      = _floppyTextOff;
		_kdTextOffset    = _floppyKDTextOff;
		_solvedOffset    = _floppySolvedOff;
		_hintOffset      = _floppyHintBlockOff;

		// Per-mystery runtime state — mirrors _ReadMystery_Floppy @ 22dc:0178.
		// _newOrder uses identity mapping [0..N-1] so dialog byte9 (1-based
		// logical idx) maps to _inGallery[idx - 1].
		memset(_cluesFound, 0, sizeof(_cluesFound));
		memset(_noteSelected, 0, sizeof(_noteSelected));
		memset(_hotSpotsSeen, 0, sizeof(_hotSpotsSeen));
		memset(_inGallery, 0, sizeof(_inGallery));
		(void)rng;
		for (uint i = 0; i < kGalleryCap; i++)
			_newOrder[i] = (uint8)i;
		memset(_visitedSite, 0, sizeof(_visitedSite));
		memset(_onSites, 0, sizeof(_onSites));
		_sawCOFFSITEs = _sawCONSITEs = _sawHelpHint = _solvedPuzzle = false;
		_seenCOFFSITEs = _seenCONSITEs = 0;
		_firstTry = true;
		_searchLocationNumber = _siteNumber = 0xFFFF;
		_lastSite = 0x1B;
		_pendingSiteJump = 0;
		_siteReturnDepth = 0;
		memset(_siteReturnStack, 0, sizeof(_siteReturnStack));
		loadFloppySiteAnimData();

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

	if (_numSites > kVisitedSiteCap)
		_numSites = kVisitedSiteCap;

	for (uint i = 0; i < kChainLen; i++) {
		_aChain[i] = readU16((16 + i) * 2);
		_bChain[i] = readU16((21 + i) * 2);
		_cChain[i] = readU16((26 + i) * 2);
	}

	memset(_cluesFound, 0, sizeof(_cluesFound));
	memset(_noteSelected, 0, sizeof(_noteSelected));
	memset(_hotSpotsSeen, 0, sizeof(_hotSpotsSeen));
	memset(_inGallery, 0, sizeof(_inGallery));
	(void)rng;
	for (uint i = 0; i < kGalleryCap; i++)
		_newOrder[i] = (uint8)i;
	memset(_visitedSite, 0, sizeof(_visitedSite));
	memset(_onSites, 0, sizeof(_onSites));
	_sawCOFFSITEs = _sawCONSITEs = _sawHelpHint = _solvedPuzzle = false;
	_seenCOFFSITEs = _seenCONSITEs = 0;
	_firstTry = true;
	_searchLocationNumber = _siteNumber = 0xFFFF;
	_lastSite = 0x1B; // _ReadMystery _LastSite sentinel.
	_pendingSiteJump = 0;
	_siteReturnDepth = 0;
	memset(_siteReturnStack, 0, sizeof(_siteReturnStack));

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
	// Floppy site index: 2-byte u16 entries (_DoSiteLoop_Floppy @ 1652:03d2).
	// CD: 6-byte rows.
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

const byte *Mystery::floppySiteAnimData(uint siteNum) const {
	if (!_isFloppy || siteNum >= kVisitedSiteCap)
		return nullptr;
	const uint16 off = _floppySiteAnimSiteOff[siteNum];
	if (off == 0 || off >= _floppySiteAnimData.size())
		return nullptr;
	return _floppySiteAnimData.data() + off;
}

void Mystery::loadFloppySiteAnimData() {
	_floppySiteAnimData.clear();
	memset(_floppySiteAnimSiteOff, 0, sizeof(_floppySiteAnimSiteOff));

	Common::File f;
	if (!f.open(Common::Path("ANI.BIN"))) {
		warning("Mystery::loadFloppySiteAnimData: ANI.BIN missing");
		return;
	}

	const int32 size = f.size();
	if (size <= 0 || size > 0xFFFF) {
		warning("Mystery::loadFloppySiteAnimData: invalid ANI.BIN size %d",
				size);
		return;
	}

	_floppySiteAnimData.resize((uint)size);
	if (f.read(_floppySiteAnimData.data(), (uint32)size) != (uint32)size) {
		warning("Mystery::loadFloppySiteAnimData: short ANI.BIN read");
		_floppySiteAnimData.clear();
		return;
	}

	const uint tableOff = _number * 2;
	if (tableOff + 2 > _floppySiteAnimData.size())
		return;
	uint32 pos = READ_LE_UINT16(_floppySiteAnimData.data() + tableOff);
	if (pos == 0 || pos >= _floppySiteAnimData.size())
		return;

	for (uint site = 0; site < _numSites && site < kVisitedSiteCap; site++) {
		const uint32 start = pos;
		if (pos + 1 > _floppySiteAnimData.size())
			break;
		const uint cycles = _floppySiteAnimData[pos++];
		if (pos + cycles * 2 + 1 > _floppySiteAnimData.size()) {
			warning("Mystery::loadFloppySiteAnimData: malformed cycles "
					"for mystery %u site %u", _number, site);
			break;
		}
		pos += cycles * 2;

		const uint anims = _floppySiteAnimData[pos++];
		if (pos + anims * 4 > _floppySiteAnimData.size()) {
			warning("Mystery::loadFloppySiteAnimData: malformed anims "
					"for mystery %u site %u", _number, site);
			break;
		}
		_floppySiteAnimSiteOff[site] = (uint16)start;
		pos += anims * 4;
	}

	debugC(1, kDebugMystery,
		   "Mystery::loadFloppySiteAnimData(%u): base=0x%04x sites=%u",
		   _number, READ_LE_UINT16(_floppySiteAnimData.data() + tableOff),
		   _numSites);
}

const byte *Mystery::hotspots(uint siteNum) const {
	if (_isFloppy) {
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
	// NoteIndex runs from _noteOffset to start of GalleryData.
	// CD entries: 4 bytes (u16 textOff; u16 points).
	// Floppy entries: 7 bytes (u16 ?; u16 jakeOff; u16 jennyOff; u8 score)
	if (_galleryOffset <= _noteOffset)
		return 0;
	const uint stride = _isFloppy ? 7 : 4;
	return (uint16)((_galleryOffset - _noteOffset) / stride);
}

uint Mystery::noteSectionSize() const {
	if (!isLoaded() || _galleryOffset <= _noteOffset)
		return 0;
	return _galleryOffset - _noteOffset;
}

bool Mystery::noteHasNotebookText(uint clueId) const {
	const byte *ni = noteIndex();
	const uint16 cnt = noteIndexCount();
	if (!ni || clueId >= cnt)
		return false;

	if (!_isFloppy)
		return true;

	// _DrawNotes_Floppy @ 15e0:01e8 skips rows with notes[idx*7..idx*7+1] == 0.
	// Spoken-only dialog records are marked seen but have no notebook text.
	return READ_LE_UINT16(ni + clueId * 7) != 0;
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
		// Floppy SITES section (FUN_1fed_07ed): byte[0]=count, then 11-byte
		// entries starting at byte 1.
		//   +4 picID, +6..7 u16 X, +8..9 u16 Y, +10 recolor.
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

const byte *Mystery::floppySuspectEntry(uint suspectIdx) const {
	// Floppy gallery section (_DrawGallery_Floppy @ 154e:00b6).
	// byte[0]=numSuspects, then per suspect a `5 + nameLen` record:
	//   u16 +0 picID (BUTTON.DBD entry)
	//   u16 +2 alibi marker (0xFFFF=guilty; else hi byte indexes TEXT_BLOCK)
	//   u8  +4 nameLen
	//   u8  +5.. name string
	if (!_isFloppy || !isLoaded())
		return nullptr;
	const byte *gd = _data.data() + _galleryOffset;
	if (gd + 1 > _data.data() + _data.size())
		return nullptr;
	const uint8 numSus = gd[0];
	if (suspectIdx >= numSus)
		return nullptr;
	const byte *p = gd + 1;
	const byte *bufEnd = _data.data() + _data.size();
	for (uint i = 0; i < suspectIdx; i++) {
		if (p + 5 > bufEnd)
			return nullptr;
		p += 5 + (uint)p[4];
	}
	if (p + 5 > bufEnd)
		return nullptr;
	return p;
}

bool Mystery::isGuilty(uint suspectIdx) const {
	if (_isFloppy) {
		const byte *e = floppySuspectEntry(suspectIdx);
		return e && READ_LE_UINT16(e + 2) == 0xFFFF;
	}
	const byte *gd = galleryData();
	if (!gd || suspectIdx >= _numSuspects)
		return false;
	const uint16 off = READ_LE_UINT16(gd + suspectIdx * 0x46 + 0x02);
	return off == 0xFFFF;
}

uint16 Mystery::alibiTextOffset(uint suspectIdx) const {
	if (_isFloppy) {
		const byte *e = floppySuspectEntry(suspectIdx);
		if (!e)
			return 0xFFFF;
		const uint16 alibi = READ_LE_UINT16(e + 2);
		if (alibi == 0xFFFF)
			return 0xFFFF;
		const uint8 idx = (uint8)(alibi >> 8);
		const uint32 base = _textOffset;
		if ((uint32)idx * 2 + 2 > _data.size() - base)
			return 0xFFFF;
		return READ_LE_UINT16(_data.data() + base + (uint32)idx * 2);
	}
	const byte *gd = galleryData();
	if (!gd || suspectIdx >= _numSuspects)
		return 0xFFFF;
	return READ_LE_UINT16(gd + suspectIdx * 0x46 + 0x02);
}

const byte *Mystery::hintBlock() const {
	// Header word[9] _hintOffset (_KDHelp @ 1560:010a). Each u16 is a
	// TextBlock offset for the corresponding _AChain entry, or 0xFFFF.
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
	if (_isFloppy) {
		// _GetSelectedPoints_Floppy @ 1d40:0c23: per-note score (note +6)
		// for each TextSeen clue, sort descending, sum top 5.
		uint8 scores[Mystery::kCluesFoundCap] = {};
		uint scoreCount = 0;
		const uint maxIdx = MIN<uint>(cnt, kCluesFoundCap);
		for (uint i = 0; i < maxIdx; i++) {
			if (_cluesFound[i] == 0)
				continue;
			scores[scoreCount++] = ni[i * 7 + 6];
		}
		// Partial selection sort for the top 5 scores.
		const uint topN = MIN<uint>(5u, scoreCount);
		for (uint k = 0; k < topN; k++) {
			uint best = k;
			for (uint j = k + 1; j < scoreCount; j++) {
				if (scores[j] > scores[best])
					best = j;
			}
			if (best != k) {
				uint8 tmp = scores[k];
				scores[k] = scores[best];
				scores[best] = tmp;
			}
		}
		int total = 0;
		for (uint k = 0; k < topN; k++)
			total += scores[k];
		return total;
	}
	int total = 0;
	for (uint i = 0; i < cnt && i < kCluesFoundCap; i++) {
		if (!_noteSelected[i])
			continue;
		// CD NoteIndex entry: 4 bytes = u16 textOff + u16 points (at +2).
		const uint16 pts = READ_LE_UINT16(ni + i * 4 + 2);
		total += (int)(int16)pts;
	}
	return total;
}
// `_SolvedCheck @ 1ea1:0b1a`. Reuses the already-loaded hint chains
// (_aChain/_bChain/_cChain) as the three answer sets and the shared
// _noteSelected accuse-selection array — see londonSolved() doc in the
// header. EEM1/floppy keep the points model via solvedCheck().
bool Mystery::londonSolved() const {
	for (uint chain = 0; chain < 3; chain++) {
		int remaining = (int)kChainLen;
		int wild = 0;
		for (uint slot = 0; slot < kChainLen; slot++) {
			const uint16 clue = hintChain(chain, slot);
			if (clue == 0xFFFF) {
				remaining--;
				wild++;
			} else if (clue < kCluesFoundCap && _noteSelected[clue]) {
				remaining--;
			}
		}
		if (wild == (int)kChainLen)
			continue;            // all-wildcard set is unused
		if (remaining == 0)
			return true;         // every slot wildcard-or-selected
	}
	return false;
}
// `_GetMinRemaining @ 1ea1:1056`. Parallel to londonSolved() but tests the
// three answer sets against _cluesFound (discovered in the world). Tracks
// whether any non-wildcard answer clue has been found at all; if none, the
// player has nothing relevant yet (returns kChainLen).
int Mystery::minCluesRemaining() const {
	int best = (int)kChainLen;
	int foundReal = 0;
	for (uint chain = 0; chain < 3; chain++) {
		int remaining = (int)kChainLen;
		int wild = 0;
		for (uint slot = 0; slot < kChainLen; slot++) {
			const uint16 clue = hintChain(chain, slot);
			if (clue == 0xFFFF) {
				remaining--;
				wild++;
			} else if (clue < kCluesFoundCap && _cluesFound[clue]) {
				remaining--;
				foundReal++;
			}
		}
		if (wild == (int)kChainLen)
			remaining = (int)kChainLen;   // unused set must not lower the min
		if (remaining < best)
			best = remaining;
	}
	if (foundReal == 0)
		return (int)kChainLen;
	return best;
}

int Mystery::foundPoints() const {
	const byte *ni = noteIndex();
	const uint16 cnt = noteIndexCount();
	if (!ni || cnt == 0)
		return 0;

	uint16 scores[Mystery::kCluesFoundCap] = {};
	uint scoreCount = 0;
	const uint maxIdx = MIN<uint>(cnt, kCluesFoundCap);
	for (uint i = 0; i < maxIdx; i++) {
		if (_cluesFound[i] == 0)
			continue;
		const uint16 pts = _isFloppy ? ni[i * 7 + 6]
									 : READ_LE_UINT16(ni + i * 4 + 2);
		scores[scoreCount++] = pts;
	}

	const uint topN = MIN<uint>(5u, scoreCount);
	for (uint k = 0; k < topN; k++) {
		uint best = k;
		for (uint j = k + 1; j < scoreCount; j++) {
			if (scores[j] > scores[best])
				best = j;
		}
		if (best != k) {
			const uint16 tmp = scores[k];
			scores[k] = scores[best];
			scores[best] = tmp;
		}
	}

	int total = 0;
	for (uint k = 0; k < topN; k++)
		total += scores[k];
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
	if (s.isLoading())
		_pendingSiteJump = 0;
	s.syncAsUint16LE(_siteReturnDepth);
	s.syncArray(_siteReturnStack, kVisitedSiteCap,
				Common::Serializer::Uint16LE);
	if (_siteReturnDepth > kVisitedSiteCap)
		_siteReturnDepth = 0;
	s.syncAsByte(_seenCOFFSITEs);
	s.syncAsByte(_seenCONSITEs);
}

} // End of namespace EEM
