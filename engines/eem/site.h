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

namespace EEM {

class EEMEngine;
class Mystery;

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

	EEMEngine *_vm;
	Mystery *_mystery;
	bool _showHotspots = true;     ///< Toggle outlines with V key.
	int _lastSiteAnim = -1;        ///< Last site we played the arrival on.
	int _snapshotSite = -1;        ///< Site number the snapshot belongs to.
	Graphics::ManagedSurface _bgSnapshot;
	uint32 _lastTickMs = 0;        ///< Last frame-pump tick in ms.
};

} // End of namespace EEM

#endif
