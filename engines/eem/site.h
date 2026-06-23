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
/// at `kFramePeriodMs` per entry; wraps
/// on the script's 0x80 terminator. Falls back to flipbook (`tick % numFrames`)
/// when no script is registered. `numFrames` is the ANI.DBD entry's cell count,
/// used both for the fallback and to clamp script values past the asset.
uint partnerFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs);

/// Like partnerFrameAtTick but plays the script ONCE and holds the final frame.
uint oneShotFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs);

/// Total time for one full play of a one-shot gesture (frame count * period).
uint32 oneShotDurationMs(uint16 seqnum, uint numFrames);

/// Select the EEM2 ("London") animation-script table inside `findAnimScript`.
/// EEM2 ships its own `_AnimationSequences`; many partner/KD scripts differ
/// from EEM1's, so the engine must use the EEM2 sequences for that variant.
void setLondonAnimScripts(bool enabled);

/// bigMapPartnerFrameAtTick: overview-map partner walk. EEM1 (11-frame anim):
/// count-up 0..8 once, then idle `_BigMapWaitSeq`. London's anim 0x14/0x12 has
/// 12 frames, so it uses a longer entrance and an idle that reaches frame 11.
uint bigMapPartnerFrameAtTick(uint numFrames, uint32 elapsedMs, bool london);

/// bigMapDetailPartnerFrameAtTick: zoomed-view partner frame. Same two-phase shape
/// as `bigMapPartnerFrameAtTick`.
uint bigMapDetailPartnerFrameAtTick(uint numFrames, uint32 elapsedMs);

/// blitAnimFrameAnchored: mask-blit at (anchorX - frame.miscflags,
/// anchorY - frame.rowoff).
void blitAnimFrameAnchored(Graphics::Surface *screen, const Picture &p,
						   int anchorX, int anchorY);

struct MacSpritePaletteMap {
	byte white;
	byte black;
};

/// Mac UI/sprite art is authored with color 0 = white and 0xff = black,
/// but several site ColorTables swap those endpoint slots.
MacSpritePaletteMap getMacSpritePaletteMap();
byte mapMacSpriteColor(byte color, const MacSpritePaletteMap &paletteMap);
void blitMacMaskedSurface(Graphics::Surface *dst, const Picture &p,
						  int x, int y, bool flipX = false);
void blitMacMaskedSurface(Graphics::Surface *dst, const Picture &p,
						  int x, int y, bool flipX,
						  const MacSpritePaletteMap &paletteMap);
void blitMacAnimFrameAnchored(Graphics::Surface *dst, const Picture &p,
							  int anchorX, int anchorY);
void blitMacAnimFrameAnchored(Graphics::Surface *dst, const Picture &p,
							  int anchorX, int anchorY,
							  const MacSpritePaletteMap &paletteMap);

/// Rotate one VGA palette range by one slot (START→END direction).
void cyclePaletteRange(uint8 start, uint8 end);

/// Rotate one VGA palette range by one slot in the OPPOSITE direction
/// (END→START).
void cyclePaletteRangeReverse(uint8 start, uint8 end);

/// Load the 6-step yellow marching-ants ramp into palette 0xF9..0xFE
void applyHotspotGlowPalette();

/// One hotspot (search rectangle) within a site, 14 bytes on disk.
struct Hotspot {
	int16  x1, y1, x2, y2;     ///< rectangle in screen coordinates
	uint16 clueOffset;          ///< +8: byte offset of ClueBlock in the mystery file
	uint16 hotspotIndex;        ///< +10: zero-based mystery-wide seen ordinal
	uint16 extra;               ///< +12: CD cursor ID for _SwitchMouse; shipped data uses 0

	Common::Rect rect() const { return Common::Rect(x1, y1, x2, y2); }
};

/// Site / scene controller. 
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
	/// EEM2 cursor id at CD hotspot row +0xc (0 for floppy / EEM1).
	int  hotspotCursorId(uint siteNum, int idx) const;
	void updateHotspotCursor(uint siteNum, int x, int y);
	void onHotspotClicked(uint siteNum, uint hotIdx);
	/// Show a site clue, then autosave the profile if @p forceSave is set (e.g.
	/// a hotspot was newly marked seen) or it revealed a new notebook clue
	/// (`_cluesFound` 0->1). Matches EEM1's per-clue persistence.
	void displayClueAndAutosave(const byte *clueBlock, bool forceSave = false);
	void initImpatienceCounter();
	bool checkImpatienceCounter();
	void notePartnerActivity();
	bool playLondonTravelAnimation(uint fromSite, uint toSite);

	/// Partner site-arrival sequence
	bool enterSiteAnim();

	/// renderPartner: persistent in-site partner sprite
	void renderPartner(uint siteNum, uint32 tickMs);

	/// Resolve the partner idle wait-anim (animId + screen anchor) for a site.
	/// Shared by renderPartner and the clue gesture's idle-resume. Returns
	/// false when the site has no usable speaker/partner entry.
	bool partnerIdleAnimParams(uint siteNum, uint16 &animId, int &x, int &y);

	/// Floppy active speaker pose 
	bool renderFloppyHotspotPartnerPose(uint siteNum);

	void renderStaticDrops(uint siteNum);
	void renderFloppyDrops(uint siteNum);
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
	/// cache (start, end) palette ranges.
	void scanColorCycles(uint siteNum);

	/// applyColorCycles: rotate cached ColorCycle ranges
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

	/// Wall-clock anchor for partner wait anim phase
	uint32 _waitPhaseAnchor = 0;

	/// Per-site cached ColorCycle ranges (up to 5, matching original 5-slot anim table).
	struct ColorCycleRange { uint8 start, end; };
	Common::Array<ColorCycleRange> _colorCycles;
};

} // End of namespace EEM

#endif
