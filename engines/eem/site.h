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

#include "common/rect.h"
#include "common/scummsys.h"

#include "graphics/managed_surface.h"

#include "eem/resource.h"  // Picture / Animation

namespace EEM {

class EEMEngine;
class Mystery;

/// Pick the frame index to render at `tickMs` for animation
/// `seqnum`. Walks the script registered in `kAnimScripts` (mirrors
/// `_UpdateAnimations @ 172b:09c1`'s looping path) at one entry per
/// 100 ms tick, wrapping on the script's 0x80 terminator. Falls
/// back to flipbook (`tick % numFrames`) when no script is
/// registered. `numFrames` is the underlying ANI.DBD entry's cell
/// count — used both for the fallback path and to clamp script
/// values that point past the asset.
uint partnerFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs);

/// BigMap-overview partner frame: plays the count-up 0..8 once (the
/// "unfold the map" pose), then loops the `_BigMapWaitSeq` hold
/// (9,9,9,9,10,9,9,9,9). Mirrors `_DoBigMap @ 20fe:09e7`'s two-phase
/// dispatch — the slot starts on script 0x14 (count-up @ 29be:196a)
/// and on the 0x80 terminator the slot's script pointer is rewritten
/// to `_BigMapWaitSeq @ 29be:1574`. `partnerFrameAtTick` can't model
/// that swap on its own (it always loops), so without this helper the
/// unfold cycles forever instead of resting on the open-map pose.
/// `elapsedMs` is the time since the BigMap was opened.
uint bigMapPartnerFrameAtTick(uint numFrames, uint32 elapsedMs);

/// BigMap-detail (zoomed view) partner frame. Same two-phase shape as
/// `bigMapPartnerFrameAtTick`, but the original `_DoMapScreen @ 20fe:120b`
/// uses script 0x13 (count-up 0..7 @ 29be:1992) for the unfold and
/// swaps the slot pointer to `_SmallMapWaitSeq @ 29be:1548` (an
/// 18-frame hold of cell 7 with a single cell-10 fidget) on the
/// terminator (`MOV [BX+0x789f],0x1548` at 20fe:1390). `elapsedMs`
/// is the time since the detail screen was opened.
uint bigMapDetailPartnerFrameAtTick(uint numFrames, uint32 elapsedMs);

/// Anchor-aware masked blit. Mirrors the per-frame anchor offset
/// math in `_UpdateAnimations @ 172b:09c1`:
/// `blit_x = anchor_x - frame.miscflags`, `blit_y = anchor_y -
/// frame.rowoff`. Use for any animation rendered through the
/// `_NewAnimation` path in the original (partner sprites, animated
/// drops, briefing animations) — without it, frames with non-zero
/// per-cell anchors (e.g. anim 0x14 BigMap walk-cycle's miscflags
/// = -2 shift) "shake in place" instead of translating across
/// the screen as they're meant to.
void blitAnimFrameAnchored(Graphics::Surface *screen, const Picture &p,
						   int anchorX, int anchorY);

/// One hotspot (search rectangle) within a site, 14 bytes on disk.
struct Hotspot {
	int16  x1, y1, x2, y2;     ///< rectangle in screen coordinates
	uint16 clueOffset;          ///< +8: byte offset of ClueBlock in the mystery file
	uint16 hotspotIndex;        ///< +10: 1-based hotspot ordinal within the site
	uint16 extra;               ///< +12: unknown (zero in M0..M54)

	Common::Rect rect() const { return Common::Rect(x1, y1, x2, y2); }
};

/**
 * Site / scene controller.
 *
 * Walks the mystery's SiteIndex to render one site at a time, polls hotspots
 * for the player's search clicks, and dispatches clue display. Mirrors the
 * site loop driven by `_DrawSearchButtons` @ 2404:0a8f and `_SearchButtons`
 * @ 2404:0bfb.
 */
class SiteScreen {
public:
	SiteScreen(EEMEngine *vm, Mystery *mystery)
		: _vm(vm), _mystery(mystery) {}

	/** Enter site @p siteNum. Renders BG + hotspots; runs the input loop. */
	void enter(uint siteNum);

	/// Run the per-mystery loop: site -> map -> next site, ESC exits.
	void run();

private:
	void renderBackground(uint siteNum);
	void renderHotspots(uint siteNum);
	int  hotspotAtPoint(uint siteNum, int x, int y) const;
	void onHotspotClicked(uint siteNum, uint hotIdx);

	/// Play the partner's site-arrival sequence once `_LastSite !=
	/// _SiteNumber`. Mirrors `_EnterSiteAnim @ 1000:9b21` — animation
	/// 6 (Jake) / 14 (Jenny) skateboards in from the right edge along
	/// the bottom, then animation 7 / 15 slides KD in from the left.
	void enterSiteAnim();

	/// Draw the persistent in-site partner sprite (Jake or Jenny
	/// standing/idling) at the position from `_WaitAnims` @ 29be:021c.
	/// Mirrors the `_GetAnimation` + `_NewAnimation` block at the tail
	/// of `_DoSiteLoop @ 168d:03f4`. `tickMs` selects which frame of
	/// the partner's animation to render; in the original, frames
	/// advance per `_CheckFrameRate` tick via `_UpdateAnimations`.
	void renderPartner(uint siteNum, uint32 tickMs);

	/// Draw the per-site `_AddDrop` static decorations (Loop 2).
	/// `_DoSiteLoop` runs this loop with bound siteData[+0x4] and
	/// 6-byte entries at siteData[+0xc]: {picId, x, y}. These never
	/// animate so they go in the BG snapshot.
	void renderStaticDrops(uint siteNum);

	/// Draw the per-site animated NPCs (Loop 1) at the current tick.
	/// `_DoSiteLoop` registers each via `_NewAnimation` (siteData[+0xa]
	/// entries at siteData[+0x48]: {animId (-1 = ColorCycle), x, y})
	/// and `_UpdateAnimations @ 172b:09c1` advances frame indices each
	/// tick. We use a millis-based frame index so all NPCs cycle in
	/// step with the global clock.
	void renderAnimatedDrops(uint siteNum, uint32 tickMs);

	/// Snapshot the post-BG, post-static-drops screen so the per-tick
	/// frame pump can restore it without reloading PICS.DBD entries.
	void captureBgSnapshot();

	/// Restore the snapshot taken at `captureBgSnapshot` time.
	void restoreBgSnapshot();

	/// Scan the site's Loop 1 entries for ColorCycle entries (animId
	/// == -1) and cache their (start, end) palette ranges. Called from
	/// `enter()`. Mirrors `_DoSiteLoop @ 168d:03f4`'s init scan.
	void scanColorCycles(uint siteNum);

	/// Rotate cached ColorCycle palette ranges (and 0xf9..0xfe for
	/// hotspot marching ants) one step. Mirrors the original's per-tick
	/// `_ColorCycle(start, end)` calls inside `_DoSiteLoop`'s main
	/// loop. ScummVM's palette manager grabs current 8-bit RGB and
	/// writes back the rotated values.
	void applyColorCycles();

	EEMEngine *_vm;
	Mystery *_mystery;
	bool _showHotspots = true;     ///< Toggle outlines with V key.
	int _lastSiteAnim = -1;        ///< Last site we played the arrival on.
	int _snapshotSite = -1;        ///< Site number the snapshot belongs to.
	Graphics::ManagedSurface _bgSnapshot;
	uint32 _lastTickMs = 0;        ///< Last frame-pump tick in ms.

	/// Wall-clock timestamp at which the partner's wait animation
	/// "started" (or last restarted). The site loop renders the
	/// wait sprite at `partnerFrameAtTick(animId, ..., now -
	/// _waitPhaseAnchor)` so the script position is RELATIVE to
	/// this anchor, not the global clock. Bump it on:
	///   - entry to a new site (mirrors `_NewAnimation` setting
	///     the slot's frame index to 0xffff at site setup, see
	///     `_DoSiteLoop @ 168d:0436`)
	///   - return from a one-shot kdAnim (mirrors `_PlayAnimation
	///     @ 172b:1f5d` writing 0xffff to the resumed slot's frame
	///     index when state=4 chains back to WaitHandle)
	/// Without this, the wait anim "snaps" to wherever the global
	/// clock dictates each time the partner reappears, instead of
	/// resuming from script[0].
	uint32 _waitPhaseAnchor = 0;

	/// Per-site cached ColorCycle ranges. Up to 5 (matching the
	/// original's 5-slot animation table).
	struct ColorCycleRange { uint8 start, end; };
	Common::Array<ColorCycleRange> _colorCycles;
};

} // End of namespace EEM

#endif
