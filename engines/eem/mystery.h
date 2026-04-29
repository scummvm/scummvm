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

#ifndef EEM_MYSTERY_H
#define EEM_MYSTERY_H

#include "common/array.h"
#include "common/path.h"
#include "common/scummsys.h"
#include "common/serializer.h"
#include "common/str.h"

namespace EEM {

/**
 * One mystery (case file) loaded from `M<n>.BIN`.
 *
 * Mirrors the layout established by `_ReadMystery` @ 2404:008f:
 *
 *   word[0]  = InitBlock byte offset
 *   word[2]  = MapData byte offset
 *   word[3]  = SiteIndex byte offset
 *   word[4]  = TextBlock byte offset
 *   word[5]  = NoteIndex byte offset
 *   word[6]  = GalleryData byte offset
 *   word[7]  = KDTextIndex byte offset
 *   word[8]  = SolvedClues byte offset
 *   word[9]  = HintBlock byte offset
 *   word[10] = NumSites + start of MysteryStats
 *   word[13] = NumSuspects (low byte)
 *   word[14] = NumCONSITEs
 *   word[15] = NumCOFFSITEs
 *   word[16..20] = AChain (5 words)
 *   word[21..25] = BChain
 *   word[26..30] = CChain
 *
 * Per-mystery state is reset every time a mystery is loaded; chains and
 * indices are pointers into the in-memory `_data` blob.
 */
class Mystery {
public:
	static const uint kNumChains       = 3;
	static const uint kChainLen        = 5;
	static const uint kCluesFoundCap   = 100;
	static const uint kHotSpotsCap     = 100;
	static const uint kGalleryCap      = 5;
	static const uint kVisitedSiteCap  = 20;

	Mystery() = default;
	~Mystery() = default;

	/// Load `M<num>.BIN` and reset per-mystery state. Returns false on error.
	bool load(uint num, class Common::RandomSource *rng = nullptr);

	/// Drop the loaded mystery and zero per-mystery state. Safe to call
	/// at any time; `isLoaded()` returns false afterward.
	void clear();

	/// True once `load()` succeeded and offsets are valid.
	bool isLoaded() const { return !_data.empty(); }

	uint number() const { return _number; }
	uint16 numSites() const { return _numSites; }
	uint8  numSuspects() const { return _numSuspects; }
	uint8  numCONSITEs() const { return _numCONSITEs; }
	uint8  numCOFFSITEs() const { return _numCOFFSITEs; }

	/// Pointer to the InitBlock (case briefing).
	/// _InitBlock @ 2d5d:?? = mystery + word[0] in `_ReadMystery`.
	const byte *initBlock() const;

	/// Pointer to the GalleryData; one 0x46-byte entry per suspect.
	/// First u16 of each entry is the PIC picture ID for that suspect.
	const byte *galleryData() const;

	/// Pointer to the NoteIndex array (4 bytes per entry: u16 textOff + u16 pts).
	const byte *noteIndex() const;

	/// Number of entries in NoteIndex.
	uint16 noteIndexCount() const;

	/// Pointer to the KDTextIndex; first u16s are TextBlock offsets for
	/// host hint lines.
	const byte *kdTextIndex() const;

	/// Pointer to the HintBlock; per-clue hint TextBlock offsets indexed
	/// by `_aChain[i]` (the Nth required clue). Mirrors the
	/// `_HintBlock` global read in `_KDHelp @ 1560:010a`.
	const byte *hintBlock() const;

	/// Read entry @p i from `_aChain` (the required-clue chain). Returns
	/// 0xFFFF when no entry exists. Used by `_KDHelp` to walk unfound
	/// clues for hints.
	uint16 aChain(uint i) const {
		return i < kChainLen ? _aChain[i] : 0xFFFF;
	}

	/// Pointer to the MapData entry for site @p siteNum (14 bytes per
	/// entry; first u16 = sitepic, +4..7 = (x, y) on the big map).
	const byte *mapEntry(uint siteNum) const;

	/// Pointer to the SiteIndex entry for site @p siteNum (6 bytes per site).
	const byte *siteIndexEntry(uint siteNum) const;

	/// Pointer to the SiteData (sitepic, travel, hotspot count, ...)
	/// referenced by SiteIndex[@p siteNum].
	const byte *siteData(uint siteNum) const;

	/// Pointer to the hotspot rectangle array for site @p siteNum.
	/// Each rect is 14 bytes: x1, y1, x2, y2, then 6 bytes of clue data.
	const byte *hotspots(uint siteNum) const;

	/// Number of hotspots in site @p siteNum.
	uint16 hotspotCount(uint siteNum) const;

	/// Pointer to a NUL-terminated string at TextBlock+@p offset.
	const char *textAt(uint16 offset) const;

	/// Pointer at byte offset @p offset within the mystery blob, or null
	/// if out of range. Used to chase ClueBlock pointers stored in
	/// hotspot data.
	const byte *blobAt(uint32 offset) const {
		return offset < _data.size() ? _data.data() + offset : nullptr;
	}

	/// Total mystery blob size in bytes (for bounds checks).
	uint32 dataSize() const { return (uint32)_data.size(); }

	/// Synchronize the per-mystery runtime state for save/load. The fixed
	/// arrays serialize first, then the booleans and counters.
	void syncState(Common::Serializer &s);

	/// Sum of point values of every selected notebook entry. Mirrors
	/// `_GetSelectedPoints` @ 1df2:00bd.
	int selectedPoints() const;

	/// True when `selectedPoints() > 99`. Mirrors `_SolvedCheck`.
	bool solvedCheck() const { return selectedPoints() > 99; }

	/// True iff suspect @p suspectIdx is the case's guilty party. The
	/// guilty marker is `GalleryData[suspectIdx * 0x46 + 0x02] ==
	/// 0xFFFF` — innocent suspects store their alibi text offset there;
	/// the guilty suspect uses the sentinel. Verified at `_WITCH @
	/// 1df2:089f` (`if (psVar1->field_0x2 == -1) _DisplayCorrect();
	/// else _DisplayAlibi(...)`).
	bool isGuilty(uint suspectIdx) const;

	/// TextBlock offset of suspect @p suspectIdx's alibi text. Returns
	/// 0xFFFF for the guilty suspect (no alibi).
	uint16 alibiTextOffset(uint suspectIdx) const;

	/// Pointer to the win-clueblock (`MysteryIndex[+0x10]` =
	/// `_solvedOffset`). Mirrors `_DisplayCorrect`'s
	/// `_DisplayClue(_Mystery + MysteryIndex[+0x10], 0)` at 1df2:0769.
	const byte *solvedClueBlock() const;

	/// Per-mystery runtime state, zeroed at load time.
	uint8  _cluesFound[kCluesFoundCap]   = {};
	uint8  _noteSelected[kCluesFoundCap] = {};  ///< Mirror `_NoteSelected`
	uint16 _hotSpotsSeen[kHotSpotsCap]   = {};
	uint16 _inGallery[kGalleryCap]       = {};
	uint8  _newOrder[kGalleryCap]        = {};
	uint16 _visitedSite[kVisitedSiteCap] = {};
	uint16 _onSites[kVisitedSiteCap]     = {};
	bool   _sawCOFFSITEs = false;
	bool   _sawCONSITEs  = false;
	bool   _sawHelpHint  = false;
	bool   _solvedPuzzle = false;
	bool   _firstTry     = true;
	uint16 _searchLocationNumber = 0xFFFF;
	uint16 _siteNumber           = 0xFFFF;
	uint16 _lastSite             = 0xFFFF;

private:
	Common::Array<byte> _data;
	uint   _number = 0;

	uint16 _initOffset      = 0;
	uint16 _mapOffset       = 0;
	uint16 _siteIndexOffset = 0;
	uint16 _textOffset      = 0;
	uint16 _noteOffset      = 0;
	uint16 _galleryOffset   = 0;
	uint16 _kdTextOffset    = 0;
	uint16 _solvedOffset    = 0;
	uint16 _hintOffset      = 0;

	uint16 _numSites    = 0;
	uint8  _numSuspects = 0;
	uint8  _numCONSITEs = 0;
	uint8  _numCOFFSITEs = 0;

	uint16 _aChain[kChainLen] = {};
	uint16 _bChain[kChainLen] = {};
	uint16 _cChain[kChainLen] = {};

	// Floppy variant uses a completely different header (see comment
	// in `Mystery::load`). When `_isFloppy` is true, the CD-shaped
	// `_initOffset / _siteIndexOffset / etc.` fields are unset and the
	// floppy section pointers below are populated from the floppy
	// header offsets verified at `_ReadMystery_Floppy @ 22dc:0178`.
	bool   _isFloppy = false;
	uint16 _floppySuspectsOff = 0;   ///< header[+4]  → suspects
	uint16 _floppyHintBlockOff = 0;  ///< header[+6]  → hint→clue table
	uint16 _floppyNoteIndexOff = 0;  ///< header[+8]  → notes (7B/clue)
	uint16 _floppyGalleryOff = 0;    ///< header[+0xa] → gallery portraits
	uint16 _floppyTextOff = 0;       ///< header[+0xc] → text block
	uint16 _floppyKDTextOff = 0;     ///< header[+0x10] → KDTextIndex
	uint16 _floppySolvedOff = 0;     ///< header[+0x12] → solved clue chain

	uint16 readU16(uint offset) const;
};

} // End of namespace EEM

#endif
