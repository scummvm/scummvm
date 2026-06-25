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

/// Mystery file M<n>.BIN. CD header layout from _ReadMystery @ 2404:008f:
///   word[0]      InitBlock offset
///   word[2]      MapData offset
///   word[3]      SiteIndex offset
///   word[4]      TextBlock offset
///   word[5]      NoteIndex offset
///   word[6]      GalleryData offset
///   word[7]      KDTextIndex offset
///   word[8]      SolvedClues offset
///   word[9]      HintBlock offset
///   word[10]     NumSites
///   word[13]     NumSuspects (low byte)
///   word[14]     NumCONSITEs
///   word[15]     NumCOFFSITEs
///   word[16..20] AChain (5 words)
///   word[21..25] BChain
///   word[26..30] CChain
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

	/// Load M<num>.BIN and reset per-mystery state. The Mac release stores
	/// cases in the indexed MysteryData container instead of loose files.
	bool load(uint num, class Common::RandomSource *rng = nullptr,
			  bool macintosh = false);

	void clear();

	bool isLoaded() const { return !_data.empty(); }
	bool usesCompactMacData() const {
		return _isMacintosh && !_isMacintoshLooseScripts;
	}

	uint number() const { return _number; }
	uint16 numSites() const { return _numSites; }
	uint8  numSuspects() const { return _numSuspects; }
	uint8  numCONSITEs() const { return _numCONSITEs; }
	uint8  numCOFFSITEs() const { return _numCOFFSITEs; }

	/// InitBlock (case briefing) at mystery + word[0].
	const byte *initBlock() const;

	/// GalleryData. DOS CD uses 0x46-byte entries; floppy/Mac use compact
	/// variable-stride entries accessed through floppySuspectEntry().
	const byte *galleryData() const;

	/// Compact variable-stride suspect record for floppy/Mac. Returns nullptr
	/// on CD or out-of-range. Layout: u16 picID, u16 alibiMarker
	/// (0xFFFF=guilty), u8 count, count bytes of name/clue data.
	const byte *floppySuspectEntry(uint suspectIdx) const;

	/// NoteIndex array. EEM1 CD: 4 bytes/entry (u16 textOff + u16 pts).
	/// Floppy: 7 bytes/entry. EEM2/London CD: 2 bytes/entry. 
	const byte *noteIndex() const;

	uint16 noteIndexCount() const;

	/// Raw byte size of the CD NoteIndex section [_noteOffset, _galleryOffset).
	/// The notebook divides this by the variant stride (EEM1 4 / London 2) to
	/// get the clue count, since `noteIndexCount()` assumes the 4-byte stride.
	uint noteSectionSize() const;

	/// True when clueId has a notebook text entry. Floppy dialog records
	/// may be spoken-only with zero notebook offset, skipped by _DrawNotes_Floppy.
	bool noteHasNotebookText(uint clueId) const;

	/// KDTextIndex; first u16s are TextBlock offsets for host hint lines.
	const byte *kdTextIndex() const;

	/// HintBlock (_KDHelp @ 1560:010a). Per-clue hint TextBlock offsets
	/// indexed by _aChain[i].
	const byte *hintBlock() const;

	/// Entry @p i of the required-clue chain. Returns 0xFFFF when out of
	/// range. Walked by `_KDHelp` to find unfound clues for hints.
	uint16 aChain(uint i) const {
		return i < kChainLen ? _aChain[i] : 0xFFFF;
	}

	/// Entry @p slot of hint chain @p chainIdx (0 = A, 1 = B, 2 = C; header
	/// words 16-20 / 21-25 / 26-30). EEM2 `_DoKDHelp @ 15c1:020b` walks all
	/// three chains × 5 slots; EEM1 `_KDHelp` only chain A slots 0..1.
	uint16 hintChain(uint chainIdx, uint slot) const {
		if (slot >= kChainLen)
			return 0xFFFF;
		switch (chainIdx) {
		case 0: return _aChain[slot];
		case 1: return _bChain[slot];
		case 2: return _cChain[slot];
		default: return 0xFFFF;
		}
	}

	/// MapData entry for siteNum: 14 bytes; first u16 = sitepic, +4..7 = (x, y).
	const byte *mapEntry(uint siteNum) const;

	/// SiteIndex entry for siteNum (6 bytes per site on CD).
	const byte *siteIndexEntry(uint siteNum) const;

	/// SiteData (sitepic, travel, hotspot count, ...) per SiteIndex[siteNum].
	const byte *siteData(uint siteNum) const;

	/// Floppy ANI.BIN per-site animation block. Layout:
	/// u8 cycleCount, cycleCount x {u8 start, u8 end},
	/// u8 animCount, animCount x {u8 animId, u16 x, u8 y}.
	const byte *floppySiteAnimData(uint siteNum) const;

	/// Hotspot rectangle array for siteNum (14 bytes each: x1,y1,x2,y2 + clue).
	const byte *hotspots(uint siteNum) const;

	uint16 hotspotCount(uint siteNum) const;

	/// NUL-terminated string at TextBlock + offset.
	const char *textAt(uint16 offset) const;

	/// Pointer at byte @p offset within the mystery blob, or null if out
	/// of range. Used to chase ClueBlock pointers stored in hotspot data.
	const byte *blobAt(uint32 offset) const {
		return offset < _data.size() ? _data.data() + offset : nullptr;
	}

	uint32 dataSize() const { return (uint32)_data.size(); }

	void syncState(Common::Serializer &s);

	/// _GetSelectedPoints @ 1df2:00bd.
	int selectedPoints() const;

	/// _GetFoundPoints @ 1df2:0098 — sum of top 5 found notebook entries.
	int foundPoints() const;

	bool solvedCheck() const { return selectedPoints() > 99; }

	/// EEM2/London `_SolvedCheck @ 1ea1:0b1a`. London dropped EEM1's points
	/// model for set matching: the three hint chains (_aChain/_bChain/_cChain,
	/// = Ghidra g_dwSolveSet1..3, mystery header words 16/21/26) are the
	/// accepted answer sets of up to 5 clue ids. A set is satisfied when every
	/// slot is either 0xFFFF (wildcard / unused) or has its accuse-selected
	/// flag (_noteSelected, = g_awSelectedClue) set; the accusation is solved
	/// if ANY set is satisfied. An all-wildcard set is unused and never counts.
	bool londonSolved() const;

	/// EEM2/London `_GetMinRemaining @ 1ea1:1056`. Same three sets as
	/// londonSolved() but tested against _cluesFound (clues discovered in the
	/// world, = _CluesFound): returns the fewest clues still to find across the
	/// usable sets — 0 means a full answer set has been discovered and the
	/// accusation can begin. Returns 5 when no relevant clue has been found yet.
	int minCluesRemaining() const;

	/// _WITCH @ 1df2:089f. GalleryData[i*0x46 + 0x02] == 0xFFFF marks the
	/// guilty suspect; innocent suspects store their alibi TextBlock offset.
	bool isGuilty(uint suspectIdx) const;

	/// TextBlock offset of suspect's alibi text. 0xFFFF for guilty suspect.
	uint16 alibiTextOffset(uint suspectIdx) const;

	/// Win-clueblock at `MysteryIndex[+0x10]` = `_solvedOffset`. Used by
	/// `_DisplayCorrect` @ 1df2:0769 (`_DisplayClue(_Mystery + MysteryIndex[+0x10], 0)`).
	const byte *solvedClueBlock() const;

	/// Per-mystery runtime state, zeroed at load time.
	uint8  _cluesFound[kCluesFoundCap]   = {};
	uint8  _noteSelected[kCluesFoundCap] = {};
	uint16 _hotSpotsSeen[kHotSpotsCap]   = {};
	uint16 _inGallery[kGalleryCap]       = {};
	uint8  _newOrder[kGalleryCap]        = {};
	uint16 _visitedSite[kVisitedSiteCap] = {};
	uint16 _onSites[kVisitedSiteCap]     = {};
	bool   _sawCOFFSITEs = false;
	bool   _sawCONSITEs  = false;
	uint8  _seenCOFFSITEs = 0;
	uint8  _seenCONSITEs  = 0;
	bool   _sawHelpHint  = false;
	bool   _solvedPuzzle = false;
	bool   _firstTry     = true;
	uint16 _searchLocationNumber = 0xFFFF;
	uint16 _siteNumber           = 0xFFFF;
	uint16 _lastSite             = 0xFFFF;
	uint16 _pendingSiteJump      = 0;
	uint16 _siteReturnDepth      = 0;
	uint16 _siteReturnStack[kVisitedSiteCap] = {};

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

	bool   _isFloppy = false;
	bool   _isMacintosh = false;
	bool   _isMacintoshLooseScripts = false;
	uint16 _floppySuspectsOff = 0;   ///< header[+4]    suspects
	uint16 _floppyHintBlockOff = 0;  ///< header[+6]    hint -> clue table
	uint16 _floppyNoteIndexOff = 0;  ///< header[+8]    notes (7B/clue)
	uint16 _floppyGalleryOff = 0;    ///< header[+0xa]  gallery portraits
	uint16 _floppyTextOff = 0;       ///< header[+0xc]  text block
	uint16 _floppyKDTextOff = 0;     ///< header[+0x10] KDTextIndex
	uint16 _floppySolvedOff = 0;     ///< header[+0x12] solved clue chain
	Common::Array<byte> _floppySiteAnimData;
	uint16 _floppySiteAnimSiteOff[kVisitedSiteCap] = {};

	uint16 readU16(uint offset) const;
	void loadFloppySiteAnimData();
};

} // End of namespace EEM

#endif
