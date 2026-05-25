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

#ifndef EEM_SITE_H
#define EEM_SITE_H

#include "common/events.h"
#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/managed_surface.h"

#include "eem/resource.h"  // Picture / Animation

namespace EEM {

class EEMEngine;
class Mystery;

/// partnerFrameAtTick: frame index for `seqnum` at `tickMs`. Walks `kAnimScripts`
/// at `kFramePeriodMs` (~140 ms = `_CheckFrameRate` cadence) per entry; wraps
/// on the script's 0x80 terminator. Falls back to flipbook (`tick % numFrames`)
/// when no script is registered. `numFrames` is the ANI.DBD entry's cell count,
/// used both for the fallback and to clamp script values past the asset.
/// Mirrors the looping path of `_UpdateAnimations @ 172b:09c1`.
uint partnerFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs);

/// bigMapPartnerFrameAtTick: count-up 0..8 once, then loop `_BigMapWaitSeq`
/// (9,9,9,9,10,9,9,9,9). Mirrors `_DoBigMap @ 20fe:09e7` two-phase swap from
/// script 0x14 (count-up @ 29be:196a) to `_BigMapWaitSeq @ 29be:1574`.
uint bigMapPartnerFrameAtTick(uint numFrames, uint32 elapsedMs);

/// bigMapDetailPartnerFrameAtTick: zoomed-view partner frame. Same two-phase shape
/// as `bigMapPartnerFrameAtTick`. `_DoMapScreen @ 20fe:120b` runs script 0x13
/// (count-up 0..7 @ 29be:1992) then swaps to `_SmallMapWaitSeq @ 29be:1548`
/// (`MOV [BX+0x789f],0x1548` at 20fe:1390).
uint bigMapDetailPartnerFrameAtTick(uint numFrames, uint32 elapsedMs);

/// blitAnimFrameAnchored: mask-blit at (anchorX - frame.miscflags,
/// anchorY - frame.rowoff). Mirrors per-frame anchor math in
/// `_UpdateAnimations @ 172b:09c1`. Both anchors are SIGNED int16
/// (e.g. anim 0x14 BigMap walk-cycle has miscflags = -2 per cell, so
/// the sprite translates across the screen as it cycles). Use for any
/// animation rendered through `_NewAnimation` in the original — partner
/// sprites, animated drops, briefing animations. Transparency comes
/// from `flags >> 8` (NOT miscflags).
void blitAnimFrameAnchored(Graphics::Surface *screen, const Picture &p,
						   int anchorX, int anchorY);

/// Rotate one VGA palette range by one slot (START→END direction).
/// Mirrors `_ColorCycle`. Used by per-site Loop-1 ColorCycle entries,
/// hotspot marching ants (0xF9..0xFE), and the BigMap marker shine.
void cyclePaletteRange(uint8 start, uint8 end);

/// Rotate one VGA palette range by one slot in the OPPOSITE direction
/// (END→START): save END, shift every entry up by one (END-1 → END, ...),
/// wrap saved END to START. Mirrors `_OpenColorCycle @ 2520:04f7` (CD) /
/// `_ReverseColorCycle_Floppy`. Used by opening-anim logos (EA Kids, etc.)
/// where the cycle shifts END→START rather than START→END.
void cyclePaletteRangeReverse(uint8 start, uint8 end);

/// One hotspot (search rectangle) within a site, 14 bytes on disk.
struct Hotspot {
	int16  x1, y1, x2, y2;     ///< rectangle in screen coordinates
	uint16 clueOffset;          ///< +8: byte offset of ClueBlock in the mystery file
	uint16 hotspotIndex;        ///< +10: zero-based mystery-wide seen ordinal
	uint16 extra;               ///< +12: CD cursor ID for _SwitchMouse; shipped data uses 0

	Common::Rect rect() const { return Common::Rect(x1, y1, x2, y2); }
};

/// Site / scene controller. Mirrors `_DrawSearchButtons @ 2404:0a8f` /
/// `_SearchButtons @ 2404:0bfb` site loop.
class SiteScreen : public Common::EventObserver {
public:
	SiteScreen(EEMEngine *vm, Mystery *mystery)
		: _vm(vm), _mystery(mystery) {}

	/** Enter site @p siteNum. Renders BG + hotspots; runs the input loop. */
	void enter(uint siteNum, bool resetPartnerMood = true);

	/// Run the per-mystery loop: site -> map -> next site, ESC exits.
	void run();

private:
	bool notifyEvent(const Common::Event &event) override;

	void renderBackground(uint siteNum);
	void renderHotspots(uint siteNum);
	int  hotspotAtPoint(uint siteNum, int x, int y) const;
	void updateHotspotCursor(uint siteNum, int x, int y);
	void onHotspotClicked(uint siteNum, uint hotIdx);
	void initImpatienceCounter();
	bool checkImpatienceCounter();
	void notePartnerActivity();

	/// Partner site-arrival sequence (when `_LastSite != _SiteNumber`).
	/// Mirrors `_EnterSiteAnim @ 1000:9b21`: anim 6/14 (Jake/Jenny) skateboards
	/// in from right, then anim 7/15 slides KD in from left. Returns true if skipped.
	bool enterSiteAnim();

	/// renderPartner: persistent in-site partner sprite at `_WaitAnims @ 29be:021c`.
	/// Mirrors `_GetAnimation` + `_NewAnimation` tail of `_DoSiteLoop @ 168d:03f4`.
	void renderPartner(uint siteNum, uint32 tickMs);

	/// renderStaticDrops: `_AddDrop` static decorations (Loop 2).
	/// siteData[+0x4] count, siteData[+0xc] entries (6 bytes: {picId, x, y}).
	void renderStaticDrops(uint siteNum);

	/// renderFloppyDrops: floppy variant. `*site_data`→drops; drops[1]=count;
	/// entries at drops+2 are 5 bytes ({u16 X, u16 Y, byte picID}); PIC loaded as picID-1.
	/// Per `_DoSiteLoop_Floppy @ 1652:0418` and `FUN_16e2_18eb`.
	void renderFloppyDrops(uint siteNum);

	/// renderAnimatedDrops: per-site animated NPCs (Loop 1) at current tick.
	/// `_DoSiteLoop` registers each via `_NewAnimation` (siteData[+0xa] count,
	/// siteData[+0x48] entries: {animId (-1=ColorCycle), x, y}); frames advance via
	/// `_UpdateAnimations @ 172b:09c1`.
	void renderAnimatedDrops(uint siteNum, uint32 tickMs);

	/// Snapshot the post-BG, post-static-drops screen so the per-tick
	/// frame pump can restore it without reloading PICS.DBD entries.
	void captureBgSnapshot();

	/// Restore the snapshot taken at `captureBgSnapshot` time.
	void restoreBgSnapshot();

	/// Push pixels written through `lockScreen()` back through the backend's
	/// normal screen-copy path. The SDL/OpenGL screenshot code captures the
	/// presented backend buffer, so site frames must not leave partner/NPC
	/// sprites only in the locked software surface.
	void syncCompositedScreen();

	/// scanColorCycles: scan Loop 1 for ColorCycle entries (animId == -1),
	/// cache (start, end) palette ranges. Mirrors `_DoSiteLoop @ 168d:03f4` init scan.
	void scanColorCycles(uint siteNum);

	/// applyColorCycles: rotate cached ColorCycle ranges + 0xf9..0xfe (hotspot ants)
	/// one step. Mirrors per-tick `_ColorCycle(start, end)` calls in `_DoSiteLoop`.
	void applyColorCycles();

	EEMEngine *_vm;
	Mystery *_mystery;
	enum PartnerWaitMood {
		kPartnerWaitDefault,
		kPartnerWaitPatient,
		kPartnerWaitImpatient
	};
	int _snapshotSite = -1;        ///< Site number the snapshot belongs to.
	Graphics::ManagedSurface _bgSnapshot;
	uint32 _lastTickMs = 0;        ///< Last frame-pump tick in ms.
	uint32 _impatientDeadlineMs = 0; ///< Test-shortened impatience deadline.
	PartnerWaitMood _partnerWaitMood = kPartnerWaitDefault;

	/// Wall-clock anchor for partner wait anim phase: rendered as
	/// `partnerFrameAtTick(animId, ..., now - _waitPhaseAnchor)`.
	/// Bumped on site entry (`_NewAnimation` writes 0xffff at `_DoSiteLoop @ 168d:0436`)
	/// and on return from a one-shot kdAnim (`_PlayAnimation @ 172b:1f5d` state=4).
	uint32 _waitPhaseAnchor = 0;

	/// Per-site cached ColorCycle ranges (up to 5, matching original 5-slot anim table).
	struct ColorCycleRange { uint8 start, end; };
	Common::Array<ColorCycleRange> _colorCycles;
};

} // End of namespace EEM

#endif
