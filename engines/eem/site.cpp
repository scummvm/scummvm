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
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/paletteman.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/mystery.h"
#include "eem/site.h"

namespace EEM {

// Masked blit a Picture into a ManagedSurface. Pixels equal to `transp`
// (the high byte of `pic.flags`, per `_Rect_Move_Mask @ 1000:03fc`) are
// skipped. Used by `enterSiteAnim` for both skateboard + KD slide-in
// passes; the surface is the in-memory frame buffer that gets pushed
// to the screen each tick.
void blitFrame(Graphics::ManagedSurface &dst, const Picture &p,
			   int x, int y, byte transp) {
	const int w = p.surface.w;
	const int h = p.surface.h;
	for (int row = 0; row < h; row++) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= 200)
			continue;
		const byte *src = (const byte *)p.surface.getBasePtr(0, row);
		byte *out = (byte *)dst.getBasePtr(0, dstY);
		for (int col = 0; col < w; col++) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= 320)
				continue;
			if (src[col] != transp)
				out[dstX] = src[col];
		}
	}
}

// Mask-aware blit from a Picture into a `Graphics::Surface` (the
// locked framebuffer). Same pixel-mask semantics as `blitFrame`.
// Used by hotspot/NPC rendering inside `SiteScreen::renderHotspots`
// and `renderStaticDrops`.
void blitMaskedSurface(Graphics::Surface *screen, const Picture &p,
					   int x, int y) {
	// Top-left semantics. Used for static drops (`_AddDrop @
	// 172b:1a77` which calls `_Rect_Move_Mask(..., x, y, ...)` with
	// the raw (x, y) and ignores per-frame anchors) and any other
	// non-animated overlay. Animation rendering should go through
	// `blitAnimFrameAnchored` instead so per-frame anchor offsets
	// (miscflags = X, rowoff = Y) apply correctly.
	if (!screen)
		return;
	const byte transp = (byte)(p.flags >> 8);
	for (int row = 0; row < p.surface.h; row++) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= screen->h)
			continue;
		const byte *src = (const byte *)p.surface.getBasePtr(0, row);
		byte *dst = (byte *)screen->getBasePtr(0, dstY);
		for (int col = 0; col < p.surface.w; col++) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= screen->w)
				continue;
			if (src[col] != transp)
				dst[dstX] = src[col];
		}
	}
}

void blitAnimFrameAnchored(Graphics::Surface *screen, const Picture &p,
						   int anchorX, int anchorY) {
	// `_UpdateAnimations @ 172b:09c1` blits each animation frame at
	// `(anchor_x - puVar5[4], anchor_y - puVar5[3])` where puVar5[3]/[4]
	// are the per-frame `rowoff` / `miscflags` values from the
	// 16-byte PicData header. Both are SIGNED 16-bit anchor offsets
	// — when frames have varying anchors (anim 0x14 BigMap walk-
	// cycle has miscflags = -2 per cell, anim 0x07 has rowoff up to
	// 61), the sprite actually translates across the screen as it
	// cycles through cells. Without this, the partner "shakes in
	// place" instead of walking. (Transparency still comes from
	// `flags >> 8`, verified at the `_Rect_Move_Mask(..., *thePic >>
	// 8)` call — NOT from miscflags as an earlier comment claimed.)
	if (!screen)
		return;
	const int blitX = anchorX - (int)(int16)p.miscflags;
	const int blitY = anchorY - (int)(int16)p.rowoff;
	const byte transp = (byte)(p.flags >> 8);
	for (int row = 0; row < p.surface.h; row++) {
		const int dstY = blitY + row;
		if (dstY < 0 || dstY >= screen->h)
			continue;
		const byte *src = (const byte *)p.surface.getBasePtr(0, row);
		byte *dst = (byte *)screen->getBasePtr(0, dstY);
		for (int col = 0; col < p.surface.w; col++) {
			const int dstX = blitX + col;
			if (dstX < 0 || dstX >= screen->w)
				continue;
			if (src[col] != transp)
				dst[dstX] = src[col];
		}
	}
}

// Rotate one VGA palette range by one slot. Mirrors `_ColorCycle @
// 172b:2015` — used by both the per-site Loop-1 ColorCycle entries and
// the always-on hotspot marching-ants range 0xF9..0xFE.
void cyclePaletteRange(uint8 start, uint8 end) {
	if (end <= start)
		return;
	const uint count = (uint)end - (uint)start + 1;
	byte buf[256 * 3];
	g_system->getPaletteManager()->grabPalette(buf, start, count);
	const byte savedR = buf[0];
	const byte savedG = buf[1];
	const byte savedB = buf[2];
	for (uint i = 0; i + 1 < count; i++) {
		buf[i * 3 + 0] = buf[(i + 1) * 3 + 0];
		buf[i * 3 + 1] = buf[(i + 1) * 3 + 1];
		buf[i * 3 + 2] = buf[(i + 1) * 3 + 2];
	}
	buf[(count - 1) * 3 + 0] = savedR;
	buf[(count - 1) * 3 + 1] = savedG;
	buf[(count - 1) * 3 + 2] = savedB;
	g_system->getPaletteManager()->setPalette(buf, start, count);
}

// Per-speaker partner-position table verified against `_WaitAnims @
// 29be:021c`. 12 bytes per entry, indexed by `siteData[+8]`. Layout:
//   +0..1 anim Jake, +2..3 anim Jenny,
//   +4..5 x    Jake, +6..7 x    Jenny,
//   +8..9 y    Jake, +10..11 y    Jenny.
// Seven valid entries — anything past entry 6 in the binary is
// `_SiteButtons` rect data that follows the table in memory.
const uint16 kWaitAnims[7][6] = {
	{ 0x00, 0x0a, 0x06, 0x06, 0x50, 0x50 }, // 0
	{ 0x03, 0x0c, 0x06, 0x06, 0x50, 0x50 }, // 1
	{ 0x01, 0x0b, 0x06, 0x06, 0x50, 0x50 }, // 2
	{ 0x04, 0x0d, 0x06, 0x06, 0x50, 0x50 }, // 3
	{ 0x02, 0x10, 0x06, 0x06, 0x50, 0x50 }, // 4
	{ 0x05, 0x05, 0x06, 0x06, 0x50, 0x50 }, // 5
	{ 0x06, 0x06, 0x06, 0x06, 0x50, 0x50 }, // 6
};

// `_DoKDAnim` lookup table. Six valid kdAnimNum entries (0..5)
// verified from `29be:0228`. Layout per entry: { animJake, animJenny,
// xJake, xJenny, yJake, yJenny }. Position is (6, 80) in every entry.
const uint16 kKdAnimTable[6][6] = {
	{ 0x03, 0x0c, 6, 6, 80, 80 }, // 0 — speaker idx 1 wait anim
	{ 0x01, 0x0b, 6, 6, 80, 80 }, // 1 — same as PDA idle
	{ 0x04, 0x0d, 6, 6, 80, 80 }, // 2
	{ 0x02, 0x10, 6, 6, 80, 80 }, // 3 — same as gallery
	{ 0x05, 0x05, 6, 6, 80, 80 }, // 4 — same anim both partners
	{ 0x06, 0x06, 6, 6, 80, 80 }, // 5 — same anim both partners
};

// Animation script table. Mirrors `_AnimationSequences @ 29be:22d4`
// (a 55-entry table of far ptrs, each pointing to a u16-frame-index
// stream terminated by 0x80; 0x81 marks a jump that we don't see in
// the partner subset and so don't yet implement).
//
// `_NewAnimation @ 172b:06e1` reads the script via
// `_AnimationSequences[anim_id]` and stores the pointer in
// `DAT_2d5d_3eaf[i*0xb]`. `_UpdateAnimations @ 172b:09c1` then walks
// it one entry per `_CheckFrameRate` tick (~100 ms): the value at
// `script[index]` is the frame to render; 0x80 resets index to 0
// (loop). So a script like `[0,0,0,0,0,0,0,0,0,2]` renders nine ticks
// of frame 0 then one tick of frame 2 → the natural "blink with long
// idle hold" cadence.
//
// We use the same scripts for the wait anims (`renderPartner`) AND
// the kd-clue reaction anims (`playKdAnim`), since both call
// `_NewAnimation` in the original — only the state field differs (1
// = looping, 4 = one-shot). seqnum == animId per `_PlayAnimation`
// 172b:1f5d push order.
//
// Each entry was read directly from the EXE via Ghidra; cross-checked
// against `_NewAnimation`'s read. Frame counts include only the
// playable frames (the trailing 0x80 is the terminator, not a frame).
struct AnimScript {
	uint16 seqnum;
	uint8 len;
	uint8 frames[28];  // longest partner-subset script is 26 frames (anim 2)
};
const AnimScript kAnimScripts[] = {
	// 0x00 (29be:185e) — Jake speaker-0 wait: nine idle, one blink, loop
	{ 0x00, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// 0x01 (29be:188a) — Jake PDA idle: alternating head bob with peak
	{ 0x01, 15, { 0,1,2,0,1,0,2,1,0,1,0,1,2,1,0 } },
	// 0x02 (29be:18aa) — Jake gallery: brief wave, long hold, second
	// wave, hold (CONFIRMED 26 frames — earlier table was truncated to
	// 16 which dropped the second wave cycle).
	{ 0x02, 26, { 0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0 } },
	// 0x03 (29be:18e0) — Jake "lift, hold, lower" gesture
	{ 0x03,  9, { 0,1,2,3,2,2,2,1,0 } },
	// 0x04 (29be:18f4) — Jake bigger gesture (camera flash-style)
	{ 0x04, 13, { 0,1,2,3,4,5,4,4,4,3,2,1,0 } },
	// 0x05 (29be:1910) — Jake/Jenny shared (speaker 5): held idle, peak
	{ 0x05, 13, { 0,0,0,1,2,3,2,1,0,0,0,0,0 } },
	// 0x06 (29be:192c) — speaker 6 partner: empty (immediate END,
	// renders nothing — verified at 29be:192c byte 0 = 0x80)
	{ 0x06,  0, { 0 } },
	// 0x07 (29be:192e) — Jake walk-cycle (10 frames: 0..9)
	{ 0x07, 10, { 0,1,2,3,4,5,6,7,8,9 } },
	// 0x08 (29be:1944) — Jake stand-still with very late blink
	{ 0x08,  8, { 0,0,0,0,0,0,0,1 } },
	// 0x09 (29be:1956) — short blip animation
	{ 0x09,  9, { 0,0,0,1,0,0,0,0,0 } },
	// 0x0a — Jenny speaker-0 wait (alias of 0x00 in the binary)
	{ 0x0a, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// 0x0b — Jenny PDA idle (alias of 0x01)
	{ 0x0b, 15, { 0,1,2,0,1,0,2,1,0,1,0,1,2,1,0 } },
	// 0x0c — Jenny "take a picture" (alias of 0x03)
	{ 0x0c,  9, { 0,1,2,3,2,2,2,1,0 } },
	// 0x0d — Jenny big gesture (alias of 0x04)
	{ 0x0d, 13, { 0,1,2,3,4,5,4,4,4,3,2,1,0 } },
	// 0x0e — alias of 0x06 (empty)
	{ 0x0e,  0, { 0 } },
	// 0x0f — alias of 0x09
	{ 0x0f,  9, { 0,0,0,1,0,0,0,0,0 } },
	// 0x10 — Jenny gallery wait (alias of 0x09 — verified at 29be:1956)
	{ 0x10,  9, { 0,0,0,1,0,0,0,0,0 } },
	// 0x11 (29be:1992) — Jenny entrance count-up: 0..7. Used by
	// `_DoBigMap` when `_LastScreen == 2` (BigMap entrance one-shot).
	{ 0x11,  8, { 0,1,2,3,4,5,6,7 } },
	// 0x12 (29be:197e) — Jake entrance count-down 8..0. Used by
	// `_DoBigMap` (entrance one-shot, partner-specific exit cell).
	{ 0x12,  9, { 8,7,6,5,4,3,2,1,0 } },
	// 0x13 (29be:1992, alias of 0x11) — Jake walk-cycle 0..7,
	// looped during BigMap idle.
	{ 0x13,  8, { 0,1,2,3,4,5,6,7 } },
	// 0x14 (29be:196a) — BigMap idle walk-cycle 0..8 (9 cells),
	// partner shifts feet while you pick a site.
	{ 0x14,  9, { 0,1,2,3,4,5,6,7,8 } },
	// 0x15 (29be:185e, alias of 0x00) — Jake CaseSelection greeter:
	// nine idle, one blink, loop. Same blink cadence as the site
	// loop's wait anim (animID 0x00).
	{ 0x15, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// 0x16 (29be:185e, alias of 0x00) — Jenny CaseSelection greeter,
	// same blink script as 0x15.
	{ 0x16, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// Briefing animations — `_DoInitClues @ 1a35:0411` calls
	// `_NewAnimation(..., (PicData *)CONCAT22(0x17, ...), 1, ...)`
	// for the game animation (always anim ID 0x17 — even Jenny's
	// briefing reuses Jake's SCRIPT, even though the loaded ANI.DBD
	// cells come from her partner-specific entry 0x3b). Same pattern
	// for book (0x18 always) and nancy (0x19 always).
	//
	// AnimScript len was 28 — these scripts overflow that. Bump
	// `frames[]` is fine because we just need to fit 30 frames per
	// briefing entry. We size `frames` to 36 so all five scripts fit
	// (longest is 0x18 at 30 frames).
};
static_assert(true, "see kAnimScriptsLong below for >28-frame scripts");

// Scripts longer than 28 frames live here so the main `kAnimScripts`
// table can keep its tight `frames[28]` storage (the lookup in
// `findAnimScript` checks both arrays). Used for the briefing
// animations whose original scripts run 30 frames each.
struct AnimScriptLong {
	uint16 seqnum;
	uint8 len;
	uint8 frames[36];
};
const AnimScriptLong kAnimScriptsLong[] = {
	// 0x17 (29be:221a) — briefing game count-up 0..29 (30 frames),
	// drives the per-tick frame walk of the game piece animation
	// during `_DoInitClues`.
	{ 0x17, 30, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
				  20,21,22,23,24,25,26,27,28,29 } },
	// 0x18 (29be:2296) — briefing book: counts up to cell 8 then
	// holds for 16 ticks (the "thinking" pose) then count up 9..15.
	{ 0x18, 30, { 0,1,2,3,4,5,6,7,8,8,8,8,8,8,8,8,
				  8,8,8,8,8,8,8,9,10,11,12,13,14,15 } },
	// 0x19 (29be:2258) — briefing nancy: 18 idle ticks then
	// count-up 1..12 (the late-arriving sidekick pose).
	{ 0x19, 30, { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
				  1,2,3,4,5,6,7,8,9,10,11,12 } },
};

// Look up the script for `seqnum`. Returns the frame array + length,
// or `(nullptr, 0)` if no script is known — caller falls back to
// flipbook cycling so unknown anims still animate (just without idle
// holds).
struct AnimScriptRef {
	const uint8 *frames;
	uint8 len;
};
static AnimScriptRef findAnimScript(uint16 seqnum) {
	for (uint i = 0; i < ARRAYSIZE(kAnimScripts); i++) {
		if (kAnimScripts[i].seqnum == seqnum) {
			AnimScriptRef r;
			r.frames = kAnimScripts[i].frames;
			r.len = kAnimScripts[i].len;
			return r;
		}
	}
	for (uint i = 0; i < ARRAYSIZE(kAnimScriptsLong); i++) {
		if (kAnimScriptsLong[i].seqnum == seqnum) {
			AnimScriptRef r;
			r.frames = kAnimScriptsLong[i].frames;
			r.len = kAnimScriptsLong[i].len;
			return r;
		}
	}
	AnimScriptRef r;
	r.frames = nullptr;
	r.len = 0;
	return r;
}

void auditPartnerAnims(EEMEngine *vm) {
	// Cross-check every registered partner-subset script against the
	// underlying ANI.DBD entry it references. If the script asks for
	// a frame past the anim's actual frame count, the visible result
	// is "missing frames" — that's the user-reported symptom we
	// want to catch and fix here, not paper over with a clamp.
	if (!vm)
		return;
	DBDArchive &ani = vm->getAni();

	// Helper: audit one (id, frames, len) tuple — log a warning if
	// the script asks for a frame the ANI.DBD entry doesn't have.
	struct Walker {
		static void check(DBDArchive &ani, uint16 id, const uint8 *frames, uint8 len) {
			if (len == 0)
				return;
			Animation a;
			if (!ani.loadAnimation(id, a) || a.empty()) {
				debugC(1, kDebugSite,
					   "auditPartnerAnims: anim 0x%02x failed to load", id);
				return;
			}
			uint maxRequested = 0;
			for (uint j = 0; j < len; j++)
				if (frames[j] > maxRequested)
					maxRequested = frames[j];
			if (maxRequested >= a.size()) {
				warning("anim 0x%02x: script wants frame %u but ANI.DBD has "
						"only %u — frames will be clamped (verify script "
						"reading from `_AnimationSequences[0x%02x]` against "
						"Ghidra)",
						id, maxRequested, (uint)a.size(), id);
			} else {
				debugC(2, kDebugSite,
					   "anim 0x%02x: %u cells, script max=%u, len=%u",
					   id, (uint)a.size(), maxRequested, len);
			}
		}
	};

	for (uint i = 0; i < ARRAYSIZE(kAnimScripts); i++)
		Walker::check(ani, kAnimScripts[i].seqnum,
					  kAnimScripts[i].frames, kAnimScripts[i].len);
	for (uint i = 0; i < ARRAYSIZE(kAnimScriptsLong); i++)
		Walker::check(ani, kAnimScriptsLong[i].seqnum,
					  kAnimScriptsLong[i].frames, kAnimScriptsLong[i].len);

	// Per-frame anchor-offset audit. The original `_UpdateAnimations
	// @ 172b:09c1` blits each frame at `(anchor_x - frame.miscflags,
	// anchor_y - frame.rowoff)`. Our `blitMaskedSurface` ignores
	// those offsets — it treats the WaitAnims (anchor_x, anchor_y)
	// as the top-left. That's fine when every frame has
	// `miscflags == 0 && rowoff == 0`; if any frame has a non-zero
	// anchor, the partner sprite jumps between cells. Log the
	// offending IDs so we know whether to plumb anchors through
	// `partnerFrameAtTick` callers.
	// Audit covers every animID we register a script for — the
	// partner-subset (0x00..0x16, used by the wait anims, kd-clue
	// reactions, BigMap, Notebook, Gallery, CaseSelection greeter)
	// AND the briefing-subset (0x17..0x19, the
	// game/book/nancy anims driven by `_DoInitClues @ 1a35:0411`).
	const uint16 partnerIds[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
								   0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
								   0x0d, 0x0f, 0x10, 0x11, 0x12, 0x13,
								   0x14, 0x15, 0x16, 0x17, 0x18, 0x19 };
	for (uint i = 0; i < ARRAYSIZE(partnerIds); i++) {
		const uint16 id = partnerIds[i];
		Animation a;
		if (!ani.loadAnimation(id, a) || a.empty())
			continue;
		bool anyAnchor = false;
		int rowMin = 0, rowMax = 0, miscMin = 0, miscMax = 0;
		for (uint f = 0; f < a.size(); f++) {
			const Picture &fr = a[f];
			const int sRow  = (int)(int16)fr.rowoff;
			const int sMisc = (int)(int16)fr.miscflags;
			if (sRow != 0 || sMisc != 0) {
				if (!anyAnchor) {
					rowMin = rowMax = sRow;
					miscMin = miscMax = sMisc;
					anyAnchor = true;
				} else {
					rowMin = MIN(rowMin, sRow);
					rowMax = MAX(rowMax, sRow);
					miscMin = MIN(miscMin, sMisc);
					miscMax = MAX(miscMax, sMisc);
				}
			}
		}
		if (anyAnchor) {
			// `_UpdateAnimations @ 172b:09c1` reads these as signed
			// 16-bit values via `puVar5[3]/[4]`, so log them with
			// the sign preserved — earlier the log printed unsigned
			// 65534 instead of -2.
			debugC(1, kDebugSite,
				   "anim 0x%02x: per-frame anchor (rowoff [%d..%d], "
				   "miscflags [%d..%d]) — handled by "
				   "`blitAnimFrameAnchored`",
				   id, rowMin, rowMax, miscMin, miscMax);
		}
	}
}

// Pick the frame index to render at `tickMs` for the looping
// animation `seqnum` whose underlying ANI.DBD entry has `numFrames`
// frames. Mirrors the looping path of `_UpdateAnimations`: walk the
// script one entry per ~100 ms `_CheckFrameRate` tick, wrap on the
// 0x80 terminator. Exposed (non-static) so the BigMap, CaseSelection
// greeter, Notebook, and Gallery render paths in `ui.cpp` can use the
// same cadence — without this, every off-site partner rendering
// flipbook-cycles ALL cells of the ANI entry (no idle holds, no
// timing variations) which is the user-reported "constantly looping"
// symptom.
uint partnerFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs) {
	const AnimScriptRef s = findAnimScript(seqnum);
	const uint kFramePeriodMs = 100;
	if (!s.frames || s.len == 0)
		return numFrames > 0 ? (uint)((tickMs / kFramePeriodMs) % numFrames) : 0;
	const uint scriptIdx = (uint)((tickMs / kFramePeriodMs) % s.len);
	const uint frame     = s.frames[scriptIdx];
	// The script can in theory request a frame that's outside the
	// animation's actual frame count (a misencoded script). Clamp so
	// we don't read past `anim[]` in the caller.
	return (numFrames > 0) ? MIN<uint>(frame, numFrames - 1) : 0;
}

// Generic "play `unfold` once, then loop `waitSeq` forever" walker.
// Mirrors the original's slot-script-swap idiom: the entrance script
// runs to its 0x80 terminator, then the slot's script pointer is
// rewritten to a looping wait sequence (e.g. `_BigMapWaitSeq @
// 29be:1574`, `_SmallMapWaitSeq @ 29be:1548`). `partnerFrameAtTick`
// can't model that swap on its own (it always wraps on the same
// script), hence this helper.
static uint oneShotThenLoopFrameAtTick(const uint8 *unfold, uint unfoldLen,
									   const uint8 *waitSeq, uint waitSeqLen,
									   uint numFrames, uint32 elapsedMs) {
	const uint kFramePeriodMs = 100;
	const uint tick = elapsedMs / kFramePeriodMs;
	const uint frame = (tick < unfoldLen)
		? unfold[tick]
		: waitSeq[(tick - unfoldLen) % waitSeqLen];
	return (numFrames > 0) ? MIN<uint>(frame, numFrames - 1) : 0;
}

uint bigMapPartnerFrameAtTick(uint numFrames, uint32 elapsedMs) {
	// Script 0x14 @ 29be:196a (count-up 0..8, 0x80) → on terminator,
	// `_DoBigMap` swaps to `_BigMapWaitSeq` @ 29be:1574
	// (9,9,9,9,10,9,9,9,9, 0x80) — open-map hold with a fidget.
	static const uint8 kUnfold[]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	static const uint8 kWaitSeq[] = { 9, 9, 9, 9, 10, 9, 9, 9, 9 };
	return oneShotThenLoopFrameAtTick(kUnfold, ARRAYSIZE(kUnfold),
									  kWaitSeq, ARRAYSIZE(kWaitSeq),
									  numFrames, elapsedMs);
}

uint bigMapDetailPartnerFrameAtTick(uint numFrames, uint32 elapsedMs) {
	// Script 0x13 @ 29be:1992 (count-up 0..7, 0x80) → on terminator,
	// `_DoMapScreen @ 20fe:1390` swaps to `_SmallMapWaitSeq` @ 29be:1548
	// (18 entries: hold cell 7 with a single cell-10 fidget) — fidget
	// every ~1.8 s.
	static const uint8 kUnfold[]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	static const uint8 kWaitSeq[] = {
		7, 7, 7, 10, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	return oneShotThenLoopFrameAtTick(kUnfold, ARRAYSIZE(kUnfold),
									  kWaitSeq, ARRAYSIZE(kWaitSeq),
									  numFrames, elapsedMs);
}

void SiteScreen::enter(uint siteNum) {
	if (!_mystery || !_mystery->isLoaded()) {
		warning("SiteScreen::enter: no mystery loaded");
		return;
	}
	if (siteNum >= _mystery->numSites()) {
		warning("SiteScreen::enter: site %u out of range (max %u)",
				siteNum, _mystery->numSites());
		return;
	}

	// Reset the wait-anim phase so the partner starts fresh from
	// script[0] when entering. Mirrors `_DoSiteLoop @ 168d:0436`
	// where `_NewAnimation` sets the new slot's frame index to
	// 0xffff (= -1, becomes 0 on the first `_UpdateAnimations`
	// tick).
	_waitPhaseAnchor = g_system->getMillis();

	// Capture whether this is the first time the player enters this
	// site BEFORE we mark it visited — `_DoSiteLoop @ 168d:03f4`
	// uses the same check to decide whether to play the arrival
	// dialog: `if (_VisitedSite[_SiteNumber] == 0) _DisplayClue(...)`.
	const bool firstVisit = (siteNum < Mystery::kVisitedSiteCap)
							 && (_mystery->_visitedSite[siteNum] == 0);

	_mystery->_siteNumber = siteNum;
	debugC(1, kDebugSite, "Entering site %u (%u hotspots)",
		   siteNum, _mystery->hotspotCount(siteNum));

	// `_DoTravel @ 168d:02da` calls `_StartTravelMusic` after the
	// destination is set. We do the same here so the music swaps as
	// the player moves between sites.
	_vm->startTravelMusic();

	// Palette: original `_BuildBackground` calls `GetPalette(sitenum + 1)`
	// where sitenum is the global SITES.DBD index (= the per-mystery
	// `sitepic` field), not the per-mystery site index.
	const byte *sd = _mystery->siteData(siteNum);
	const uint16 sitepic = sd ? READ_LE_UINT16(sd) : 0;
	_vm->setSitePaletteForSite(sitepic);

	renderBackground(siteNum);

	// `_DoSiteLoop @ 168d:03f4` plays `_EnterSiteAnim` whenever
	// `_LastSite != _SiteNumber`. We track the last site we *played*
	// the arrival on so re-entries (after notebook/map/etc.) don't
	// repeat the animation.
	if ((int)siteNum != _lastSiteAnim) {
		enterSiteAnim();
		_lastSiteAnim = (int)siteNum;
		// Re-paint the BG; the arrival animation drew on top of it.
		renderBackground(siteNum);
	}

	// Stop the travel music explicitly. `_DoSiteLoop @ 168d:06c0`
	// waits for the one-shot travel track to play out and then calls
	// `_StopMIDI()` before the interactive site phase begins —
	// blocking the engine while it spins. We just stop now so the
	// site investigation runs without music (matches the original
	// silent-investigation phase).
	_vm->stopMusic();

	// Static drops (Loop 2 from `_DoSiteLoop`) — no animation, baked
	// into the BG snapshot the run() pump uses to restore.
	renderStaticDrops(siteNum);

	// Snapshot the static layers so per-tick animation re-blits don't
	// have to re-load PIC 0x43, the SITES.DBD scene, or each
	// `_AddDrop` PIC every frame.
	captureBgSnapshot();
	_snapshotSite = (int)siteNum;

	// Cache ColorCycle palette ranges for this site so the per-tick
	// frame pump can rotate them. Mirrors the init scan at the top of
	// `_DoSiteLoop @ 168d:03f4`.
	scanColorCycles(siteNum);

	// Animated NPCs (Loop 1) and the persistent partner sit on top of
	// the snapshot. Initial frame goes at tickMs=now.
	const uint32 now = g_system->getMillis();
	renderAnimatedDrops(siteNum, now);
	renderPartner(siteNum, now);

	renderHotspots(siteNum);
	g_system->updateScreen();

	// First-visit dialog. `_DoSiteLoop @ 168d:03f4` does:
	//   if (_VisitedSite[_SiteNumber] == 0) {
	//       _DisplayClue(_Mystery + SiteIndex[siteNum*6 + 2], 1);
	//       _VisitedSite[_SiteNumber] = 1;
	//   }
	// `SiteIndex[+2..+3]` is the byte offset (within the mystery
	// buffer) of a ClueBlock that holds the partner's arrival
	// dialogue. We've kept the +2 field undocumented up to now —
	// this confirms it's the entry-clue offset.
	if (firstVisit) {
		const byte *idx = _mystery->siteIndexEntry(siteNum);
		if (idx) {
			const uint16 clueOff = READ_LE_UINT16(idx + 2);
			if (clueOff != 0xFFFF) {
				const byte *clueBlock = _mystery->blobAt(clueOff);
				if (clueBlock) {
					// See onHotspotClicked — supply a partner-less BG
					// so KD-anim playback (e.g. the partner's arrival
					// camera gesture) doesn't ghost over the resting
					// idle frame.
					_vm->setPartnerEraseBg(&_bgSnapshot);
					_vm->displayClue(clueBlock);
					_vm->setPartnerEraseBg(nullptr);
				}
			}
		}
		if (siteNum < Mystery::kVisitedSiteCap)
			_mystery->_visitedSite[siteNum] = 1;
		// The dialog overlay will have left the screen with portrait /
		// balloon residues; refresh the site so the player returns to
		// a clean state. Re-build the snapshot too.
		renderBackground(siteNum);
		renderStaticDrops(siteNum);
		captureBgSnapshot();
		_snapshotSite = (int)siteNum;
		const uint32 nowAfter = g_system->getMillis();
		renderAnimatedDrops(siteNum, nowAfter);
		renderPartner(siteNum, nowAfter);
		renderHotspots(siteNum);
		g_system->updateScreen();
	} else if (siteNum < Mystery::kVisitedSiteCap) {
		_mystery->_visitedSite[siteNum] = 1;
	}
}

void SiteScreen::run() {
	if (!_mystery || !_mystery->isLoaded())
		return;

	// The caller (run() in eem.cpp) is responsible for bringing the
	// player into a site via the map first, so `_siteNumber` is
	// already set to the destination they picked. Resuming a save
	// also restores `_siteNumber`. Start there instead of forcing
	// site 0 each time.
	uint cur = _mystery->_siteNumber;
	if (cur >= _mystery->numSites())
		cur = 0;
	enter(cur);

	while (!_vm->shouldQuit()) {
		Common::Event event;
		bool exitRequested = false;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				return;

			case Common::EVENT_LBUTTONDOWN: {
				// On-screen UI buttons. `_DoSiteLoop @ 168d:03f4` calls
				//   _FindButton(&SiteButtons, 2, MouseX, MouseY)
				// where `SiteButtons` is two 8-byte rectangles at
				// 29be:0x274 (verified via 168d:0729-0848):
				//   Button 0: (35, 111) - (56, 136)  → notebook
				//                                       (`_NextScreen = 4`)
				//   Button 1: (7, 177)  - (57, 200)  → map
				//                                       (`_NextScreen = 1`)
				// Test the buttons before falling through to hotspots so
				// a click on the PDA / map icon doesn't accidentally
				// trigger a hotspot underneath.
				const Common::Rect kBtnNotebook(35, 111, 56, 136);
				const Common::Rect kBtnMap     ( 7, 177, 57, 200);
				// Partner area — port-only enhancement so the player
				// can click the host sprite for a hint, mirroring the
				// PDA's rect-3 / gallery's rect-3 behaviour. The
				// original site loop's `_FindButton(&SiteButtons, 2,
				// ...)` only checks notebook + map, but the same
				// partner-click → `_KDHelp` shortcut is wired in
				// `_HandleNoteButton[3]` (0x0403) and
				// `_HandleGalleryButton[3]` (0x061e). Rect matches
				// the PDA / gallery `kBtnPartner` (5, 80, 44, 110).
				const Common::Rect kBtnPartner ( 5,  80, 44, 110);
				if (kBtnNotebook.contains(event.mouse.x, event.mouse.y)) {
					_vm->doNotebook();
					enter(cur);
					break;
				}
				if (kBtnMap.contains(event.mouse.x, event.mouse.y)) {
					_vm->doBigMap();
					if (_mystery->_siteNumber < _mystery->numSites())
						cur = _mystery->_siteNumber;
					enter(cur);
					break;
				}
				if (kBtnPartner.contains(event.mouse.x, event.mouse.y)) {
					_vm->doHelp();
					enter(cur);
					break;
				}
				const int idx = hotspotAtPoint(cur, event.mouse.x, event.mouse.y);
				if (idx >= 0) {
					onHotspotClicked(cur, (uint)idx);
					// Restore the site BG after the clue overlay.
					enter(cur);
				}
				break;
			}

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					if (_vm->areYouSure()) {
						// Mirrors `_DoSiteLoop @ 168d:07b7` ESC path:
						// `_NextScreen = 1` (back to MAP) after the
						// areYouSure confirm. Without explicitly
						// writing it here, the run() loop would see
						// _nextScreen unchanged and treat it as a
						// quit-engine signal — abandoning the case.
						// Going to MAP keeps the case alive so the
						// player can continue from a different site.
						_vm->setNextScreen(kScreenMap);
						return;
					}
					enter(cur);
					break;
				case Common::KEYCODE_m:
					_vm->doBigMap();
					// Either way the map covered the site — re-render.
					if (_mystery->_siteNumber < _mystery->numSites())
						cur = _mystery->_siteNumber;
					enter(cur);
					break;
				case Common::KEYCODE_n:
					_vm->doNotebook();
					enter(cur);
					break;
				case Common::KEYCODE_g:
					_vm->doGallery();
					enter(cur);
					break;
				case Common::KEYCODE_a:
					_vm->doAccuse();
					exitRequested = true;
					break;
				case Common::KEYCODE_h:
					_vm->doHelp();
					enter(cur);
					break;
				case Common::KEYCODE_v:
					_showHotspots = !_showHotspots;
					enter(cur);
					break;
				case Common::KEYCODE_r:
					// Restart the mystery from scratch (mirrors `_ReloadMystery`).
					if (_mystery->load(_mystery->number())) {
						if (_vm->_audio)
							_vm->_audio->initMysterySounds(_mystery->number());
						cur = 0;
						enter(cur);
					}
					break;
				case Common::KEYCODE_QUESTION:
				case Common::KEYCODE_F1: {
					if (_vm->getFont().isLoaded()) {
						Graphics::ManagedSurface help(320, 200,
							Graphics::PixelFormat::createFormatCLUT8());
						help.clear();
						const EEMFont &fnt = _vm->getFont();
						int y = 8;
						const char *lines[] = {
							"EAGLE EYE MYSTERIES — keys",
							"",
							"  click   search a hotspot",
							"  V       toggle hotspot outlines",
							"  M       map (travel between sites)",
							"  N       notebook (mark evidence with 1..9)",
							"  G       gallery (suspect portraits)",
							"  H       hint from the case host",
							"  A       accuse a suspect",
							"  R       restart current mystery",
							"  Tab     next site (cycle)",
							"  F5      save / load (ScummVM dialog)",
							"  ? / F1  this help",
							"  Esc     quit (with confirm)",
							"",
							"Notebook: select evidence with 1..9.",
							"Selected-points > 99 wins the case."
						};
						for (uint i = 0; i < sizeof(lines)/sizeof(lines[0]); i++) {
							fnt.drawString(&help, lines[i], 8, y, 320, 0xF);
							y += fnt.getFontHeight() + 1;
						}
						g_system->copyRectToScreen(help.getPixels(),
							help.pitch, 0, 0, 320, 200);
						g_system->updateScreen();
						_vm->waitForInput(60000);
						enter(cur);
					}
					break;
				}
				case Common::KEYCODE_TAB:
					_mystery->_lastSite = cur;
					cur = (cur + 1) % _mystery->numSites();
					enter(cur);
					break;
				default:
					break;
				}
				break;

			default:
				break;
			}
		}
		if (exitRequested)
			return;

		// Per-tick frame pump (mirrors `_CheckFrameRate` +
		// `_UpdateAnimations` at the top of `_DoSiteLoop`'s main loop).
		// Restore the static BG snapshot, redraw animated NPCs +
		// partner at the current frame, then re-render hotspots on
		// top. We tick at 100 ms (~10 FPS) which is in the same ball
		// park as the original.
		const uint32 now = g_system->getMillis();
		if (_snapshotSite == (int)cur && now - _lastTickMs >= 100) {
			restoreBgSnapshot();
			renderAnimatedDrops(cur, now);
			renderPartner(cur, now);
			renderHotspots(cur);
			// Per-tick palette rotation for ColorCycle entries +
			// hotspot marching ants. Matches `_ColorCycle(start, end)`
			// calls inside `_DoSiteLoop @ 168d:03f4`'s main loop.
			applyColorCycles();
			_lastTickMs = now;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void SiteScreen::enterSiteAnim() {
	// Mirrors `_EnterSiteAnim @ 1000:9b21`. Two phases, both partner
	// dependent:
	//   Phase 1 — skateboard scroll: anim 6 (Jake) / 0xe (Jenny). Sprite
	//             starts at (320 - sprite_w, 199 - sprite_h) and slides
	//             left until off-screen.
	//   Phase 2 — KD slide-in: anim 7 (Jake) / 0xf (Jenny). Sprite enters
	//             from x = -sprite_w at y = 0x8b (Jake) / 0x8e (Jenny)
	//             and slides until x = 0.
	// Original cycles frames every `_MoveSkateBoardPixels` worth of
	// motion (a runtime-calibrated speed value); we use a fixed 4 px
	// per tick which feels close to the DOS pacing.
	if (!_vm || !_mystery)
		return;
	const uint8 partner = _vm->getPartnerIndex();
	const uint kSkateAni = (partner == 0) ? 6  : 0xe;
	const uint kKDAni    = (partner == 0) ? 7  : 0xf;
	const int  kKDY      = (partner == 0) ? 0x8b : 0x8e;

	// Snapshot the current screen so we can restore between frames.
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	Graphics::ManagedSurface bg(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < 200; row++) {
		memcpy((byte *)bg.getBasePtr(0, row),
			   (const byte *)screen->getBasePtr(0, row), 320);
	}
	g_system->unlockScreen();

	// Phase 1 — skateboard scroll. `_GetAnimation(6 | 0xe)`.
	Animation skate;
	if (_vm->getAni().loadAnimation(kSkateAni, skate) && !skate.empty()) {
		// `iVar4 = 199 - sprite_h`, `uVar5 = 320 - sprite_w` from the
		// original; sprite_h/w come from the FIRST frame.
		const int spriteH = skate[0].surface.h;
		const int spriteW = skate[0].surface.w;
		int x = (320 - spriteW) & ~3;            // 4-px aligned (mode-X)
		const int y = 199 - spriteH;
		const byte transp = (byte)(skate[0].flags >> 8);
		uint frameIdx = 0;
		int distSinceTick = 0;
		const int kStep = 4;            // _MoveSkateBoardPixels analogue
		const int kFrameTicks = 0xc;    // original switches frame at 12 px

		while (x + spriteW > 0 && !_vm->shouldQuit()) {
			Graphics::ManagedSurface scratch(320, 200,
				Graphics::PixelFormat::createFormatCLUT8());
			for (int row = 0; row < 200; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)bg.getBasePtr(0, row), 320);
			}
			blitFrame(scratch, skate[frameIdx], x, y, transp);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, 320, 200);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					return; // user-skip — bail out of the animation
				}
			}

			x -= kStep;
			distSinceTick += kStep;
			if (distSinceTick >= kFrameTicks) {
				frameIdx = (frameIdx + 1) % skate.size();
				distSinceTick = 0;
			}
			g_system->delayMillis(40);
		}
	}

	// Phase 2 — KD slide-in. From `_EnterSiteAnim` each frame is blitted
	// at its OWN anchor offsets (the sprite "walks in" because the
	// frame-by-frame anchors decrease as the animation progresses):
	//   destX = -frame.miscflags    (on-disk byte 8 = anchor X)
	//   destY = kKDY - frame.rowoff (on-disk byte 6 = anchor Y)
	// Both anchors are SIGNED int16 (mirrors `blitAnimFrameAnchored` /
	// `_UpdateAnimations @ 172b:09c1`). The original's `-pPVar8->width`
	// negation wraps in 16-bit on DOS, so a frame with `miscflags = -2`
	// (0xFFFE) lands at destX = +2 in the original. Without the int16
	// re-cast our 32-bit negation produces destX = -65534 and the
	// second-to-last frame (which has a non-zero anchor in anim 7/0xf)
	// clips entirely off-screen, leaving a one-tick partner-less gap.
	// Each frame waits one `_CheckFrameRate` tick — we use 80 ms which
	// matches the original's ~12 FPS pacing.
	Animation kd;
	if (_vm->getAni().loadAnimation(kKDAni, kd) && !kd.empty()) {
		for (uint frameIdx = 0;
			 frameIdx < kd.size() && !_vm->shouldQuit();
			 frameIdx++) {
			const Picture &fr = kd[frameIdx];
			const byte transp = (byte)(fr.flags >> 8);
			const int destX = -(int)(int16)fr.miscflags;
			const int destY = kKDY - (int)(int16)fr.rowoff;

			Graphics::ManagedSurface scratch(320, 200,
				Graphics::PixelFormat::createFormatCLUT8());
			for (int row = 0; row < 200; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)bg.getBasePtr(0, row), 320);
			}
			blitFrame(scratch, fr, destX, destY, transp);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, 320, 200);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					return;
				}
			}
			g_system->delayMillis(80);
		}
	}
}

void SiteScreen::renderStaticDrops(uint siteNum) {
	// Loop 2 from `_DoSiteLoop @ 168d:03f4`:
	//   bound: siteData[+0x4]   (verified at 168d:05c0:
	//          `MOV ES:[BX+0x4], DI; CMP ES:[BX+0x4], DI`)
	//   per entry at siteData[+0xc + i*6]: {picId, x, y}
	//   each → `_AddDrop(picId, x, y)` (`_AddDrop @ 172b:1a77`):
	//   loads PIC `picId-1` from PICS.DBD and blits with miscflags
	//   high-byte as the transparent colour. These NEVER cycle, so
	//   they belong to the BG snapshot.
	if (!_mystery)
		return;
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const uint16 numStatic = READ_LE_UINT16(site + 0x4);
	if (numStatic == 0 || numStatic > 16)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	for (uint i = 0; i < numStatic; i++) {
		const uint dropOff = 0xc + i * 6;
		const uint16 picId = READ_LE_UINT16(site + dropOff + 0);
		const int16  x     = (int16)READ_LE_UINT16(site + dropOff + 2);
		const int16  y     = (int16)READ_LE_UINT16(site + dropOff + 4);
		if (picId == 0)
			continue;
		Picture pic;
		if (!_vm->getPics().getPicture(picId, pic))
			continue;
		blitMaskedSurface(screen, pic, x, y);
	}

	g_system->unlockScreen();
}

void SiteScreen::renderAnimatedDrops(uint siteNum, uint32 tickMs) {
	// Loop 1 from `_DoSiteLoop @ 168d:03f4`:
	//   bound: siteData[+0xa]
	//   per entry at siteData[+0x48 + i*6]: {animId, x, y}
	//   animId == -1 → `_ColorCycle(x, y)` palette range (handled
	//                  in the run() loop's frame pump as palette
	//                  rotation; not yet implemented).
	//   else → `_GetAnimation(animId)` + `_NewAnimation` then
	//          `_UpdateAnimations @ 172b:09c1` walks a sequence
	//          script (entries are frame indices; 0x80 = end-of-loop,
	//          0x81 = jump command). We don't have the sequence-script
	//          structure decoded yet, so for now we cycle through the
	//          raw animation frames in order using a global tick.
	if (!_mystery)
		return;
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const uint16 numAnims = READ_LE_UINT16(site + 0xa);
	if (numAnims == 0 || numAnims > 16)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	for (uint i = 0; i < numAnims; i++) {
		const uint dropOff = 0x48 + i * 6;
		const int16 animId = (int16)READ_LE_UINT16(site + dropOff + 0);
		if (animId < 0)
			continue;
		const int16 x = (int16)READ_LE_UINT16(site + dropOff + 2);
		const int16 y = (int16)READ_LE_UINT16(site + dropOff + 4);
		Animation anim;
		if (!_vm->getAni().loadAnimation((uint)animId, anim) || anim.empty())
			continue;
		const uint frameIdx = partnerFrameAtTick((uint16)animId,
												  (uint)anim.size(), tickMs);
		// Animated drops go through `_NewAnimation` in the original,
		// so `_UpdateAnimations` applies per-frame anchor offsets —
		// route through the anchored blitter.
		blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
	}

	g_system->unlockScreen();
}

void SiteScreen::scanColorCycles(uint siteNum) {
	// `_DoSiteLoop @ 168d:03f4` walks Loop 1 entries (siteData[+0xa]
	// count, 6-byte entries at siteData[+0x48]) and stores each entry
	// with `animId == -1` into a 5-slot table:
	//   start palette idx = entry +2
	//   end   palette idx = entry +4
	// We mirror the layout exactly. Up to 5 entries are tracked (the
	// original's `[unaff_BP + -0x12]` and `[unaff_BP + -0x1c]` arrays
	// are 5 × u16 each).
	_colorCycles.clear();
	if (!_mystery)
		return;
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const uint16 numAnims = READ_LE_UINT16(site + 0xa);
	for (uint i = 0; i < numAnims && _colorCycles.size() < 5; i++) {
		const int16 animId = (int16)READ_LE_UINT16(site + 0x48 + i * 6);
		if (animId != -1)
			continue;
		const uint16 startPal = READ_LE_UINT16(site + 0x48 + i * 6 + 2);
		const uint16 endPal   = READ_LE_UINT16(site + 0x48 + i * 6 + 4);
		ColorCycleRange r;
		r.start = (uint8)startPal;
		r.end   = (uint8)endPal;
		if (r.end > r.start)
			_colorCycles.push_back(r);
	}
}

void SiteScreen::applyColorCycles() {
	// `_ColorCycle @ 172b:2015` rotates `_fpal[start..end]` by one
	// palette slot — saves [start], shifts [start..end-1] = [start+1..
	// end], restores saved at [end] — then re-uploads via `_Set_Palette`.
	// We do the same against ScummVM's palette manager. Always rotate
	// 0xf9..0xfe for hotspot marching ants (the `_ColorCycle(0xf9,
	// 0xfe)` call at the bottom of `_DoSiteLoop`'s main loop).
	for (uint i = 0; i < _colorCycles.size(); i++) {
		cyclePaletteRange(_colorCycles[i].start, _colorCycles[i].end);
	}
	// Hotspot marching ants — always cycled.
	cyclePaletteRange(0xF9, 0xFE);
}

void SiteScreen::captureBgSnapshot() {
	_bgSnapshot.create(320, 200, Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen) {
		_snapshotSite = -1;
		return;
	}
	for (int row = 0; row < 200; row++) {
		memcpy((byte *)_bgSnapshot.getBasePtr(0, row),
			   (const byte *)screen->getBasePtr(0, row), 320);
	}
	g_system->unlockScreen();
}

void SiteScreen::restoreBgSnapshot() {
	if (_bgSnapshot.w != 320 || _bgSnapshot.h != 200)
		return;
	g_system->copyRectToScreen(_bgSnapshot.getPixels(), _bgSnapshot.pitch,
							   0, 0, 320, 200);
}

void SiteScreen::renderPartner(uint siteNum, uint32 tickMs) {
	// `_DoSiteLoop @ 168d:03f4` reads `siteData[+8]` as the speaker
	// table index, then for each (speaker × partner) loads
	//   anim  = WaitAnims[speakerIdx].anim[partner]
	//   x     = WaitAnims[speakerIdx].x[partner]
	//   y     = WaitAnims[speakerIdx].y[partner]
	// from the table at `_WaitAnims @ 29be:021c`. Each entry is
	// 12 bytes / 6 u16:
	//   +0..1 anim Jake, +2..3 anim Jenny,
	//   +4..5 x    Jake, +6..7 x    Jenny,
	//   +8..9 y    Jake, +10..11 y    Jenny.
	// `kWaitAnims` lives at file scope above; we cap rendering at
	// `speaker < 7` since anything past entry 6 is the `_SiteButtons`
	// rect data that follows the table in the binary.
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const uint16 speaker = READ_LE_UINT16(site + 8);
	if (speaker >= ARRAYSIZE(kWaitAnims)) {
		warning("renderPartner: site %u has speakerIdx=%u out of range",
				siteNum, speaker);
		return;
	}

	const uint8 partner = _vm->getPartnerIndex();
	const uint  animId  = kWaitAnims[speaker][0 + partner];
	const int   x       = (int)(int16)kWaitAnims[speaker][2 + partner];
	const int   y       = (int)(int16)kWaitAnims[speaker][4 + partner];

	Animation anim;
	if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
		return;

	// `_UpdateAnimations @ 172b:09c1` walks the per-anim script (from
	// `_AnimationSequences[seqnum]`) one entry per `_CheckFrameRate`
	// tick (~100 ms): render `script[index]`, advance, wrap on 0x80.
	// That's how the original gets long idle holds with brief blinks
	// — naive flipbook cycling (`tick % nFrames`) loses those pauses
	// and makes the partner constantly fidget. `partnerFrameAt` picks
	// the right frame; if no script is registered for this anim it
	// falls back to flipbook so unknown anims still move.
	// Use the relative phase anchor instead of the raw `tickMs` so
	// the wait anim resumes from script[0] after each kdAnim
	// one-shot ends — matching the original's `_PlayAnimation @
	// 172b:1f5d` resetting the resumed slot's frame index to
	// 0xffff. Without this, the wait anim snaps mid-cycle every
	// time we return from a clue display.
	const uint32 elapsed = (tickMs >= _waitPhaseAnchor)
							? (tickMs - _waitPhaseAnchor)
							: tickMs;
	const uint frameIdx = partnerFrameAtTick((uint16)animId,
											  (uint)anim.size(), elapsed);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	// Partner sprite — anchor-aware blit so per-frame `miscflags` /
	// `rowoff` apply (e.g. the BigMap walk-cycle's -2 px shift).
	blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
	g_system->unlockScreen();
}

void SiteScreen::renderBackground(uint siteNum) {
	// Mirrors `_BuildBackground(sitepic, 0x42, 0x14)` as called from
	// `_DoSiteLoop @ 168d:03f4` and `_DisplayCorrect`. The original:
	//   1. Loads frame via `_GetFromDB(_PicIndex, 0x3d)` — that's
	//      DBI entry 0x3d (0-based). Note this is NOT the same as
	//      `_GetPicture(0x3d)` which would index entry 0x3c. Our
	//      `loadEntry(0x3d)` matches the original directly.
	//   2. Loads SITES.DBD entry by `sitepic` (0-based, from
	//      SiteData[+0..+1]).
	//   3. `_Rect_Move(... x, y, 48000, ...)` composes the scene at
	//      (x, y) = (0x42, 0x14) = (66, 20) on top of the frame.
	//   4. `_GetPalette(sitepic + 1)` — per-site palette.
	Picture frame;
	if (_vm->getPics().loadEntry(0x3d, frame)) {
		g_system->copyRectToScreen(frame.surface.getPixels(),
								   frame.surface.pitch,
								   0, 0, frame.surface.w, frame.surface.h);
	}

	// `_BuildBackground @ 172b:13e2` calls
	//   `_GetFromDB(_siteFile, &_SiteDBIndex, sitenum)`
	// with the value at SiteData[+0] passed straight through. The
	// `_GetFromDB` callee uses that as a **0-based** index into
	// SITES.DBD (verified at 172b:14c8: `MOV BX, [BP+0x6]; IMUL BX, BX, 0xa`
	// — the dbi entry stride is 10 bytes, no -1 adjustment). Our
	// previous `loadEntry(sitepic - 1)` was off by one, which is why
	// the tutorial mystery rendered scenes from neighbouring cases.
	const byte *site = _mystery->siteData(siteNum);
	const uint16 sitepic = site ? READ_LE_UINT16(site) : 0;
	Picture scene;
	bool haveScene = false;
	if (sitepic < _vm->getSites().size())
		haveScene = _vm->getSites().loadEntry(sitepic, scene);
	if (!haveScene)
		haveScene = _vm->getPics().getPicture(sitepic + 1, scene);
	if (haveScene) {
		// Hard-coded composition position from `_BuildBackground`:
		//   `_Rect_Move(0, 0, h, ..., 0x42, 0x14, 48000, h, w)`.
		const int x = 0x42;
		const int y = 0x14;
		const int w = MIN<int>(scene.surface.w, 320 - x);
		const int h = MIN<int>(scene.surface.h, 200 - y);
		if (w > 0 && h > 0)
			g_system->copyRectToScreen(scene.surface.getPixels(),
									   scene.surface.pitch, x, y, w, h);
	}
}

void SiteScreen::renderHotspots(uint siteNum) {
	// HUD overlay: site number, found clues, selected points. Drawn at
	// the BOTTOM of the screen so the scene's top row stays visible —
	// 320x200 mode 13h has a small bottom strip that the original engine
	// uses for tool buttons; we repurpose it for the HUD.
	if (_vm->getFont().isLoaded()) {
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			uint cluesFound = 0;
			for (uint i = 0; i < Mystery::kCluesFoundCap; i++)
				if (_mystery->_cluesFound[i])
					cluesFound++;
			Common::String hud = Common::String::format(
				"Site %u  Clues %u  Pts %d   M N G H A R V Tab ?",
				siteNum, cluesFound, _mystery->selectedPoints());
			const int hudY = 192;
			screen->fillRect(Common::Rect(0, hudY, 320, 200), 0);
			Graphics::ManagedSurface mgr(320, 9,
				Graphics::PixelFormat::createFormatCLUT8());
			mgr.clear();
			_vm->getFont().drawString(&mgr, hud, 4, 0, 320, 0x0F);
			for (int row = 0; row < 8; row++) {
				memcpy((byte *)screen->getBasePtr(0, hudY + row),
					   (const byte *)mgr.getBasePtr(0, row), 320);
			}
			g_system->unlockScreen();
		}
	}

	// Hotspot outlines (`_DrawSearchButtons`): toggle via V.
	if (!_showHotspots)
		return;

	const byte *spots = _mystery->hotspots(siteNum);
	const uint16 count = _mystery->hotspotCount(siteNum);
	if (!spots)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	// Mirrors `_DrawSearchButtons @ 2404:0a8f`:
	//   for each hotspot:
	//     if `_Sawit(theSite, loc)` (= _SaveBuffer[hotspot[+0xa]] != 0)
	//       _DrawRect(rect)        // outline in cycling colors 0xF9..0xFE
	//     else
	//       _DrawSolidRect(rect)   // outline in solid white 0xFF
	// `_DrawRect`'s cycling colors produce a "marching ants" effect that
	// makes already-searched hotspots visually distinct without hiding
	// them. We approximate the cycling by rotating the start color via
	// the global tick.
	const uint32 tickMs = g_system->getMillis();
	const byte cyclePhase = (byte)((tickMs / 80) & 0x07);  // 0..7

	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * 14;
		const int16 x1 = (int16)READ_LE_UINT16(r + 0);
		const int16 y1 = (int16)READ_LE_UINT16(r + 2);
		const int16 x2 = (int16)READ_LE_UINT16(r + 4);
		const int16 y2 = (int16)READ_LE_UINT16(r + 6);
		const Common::Rect rect(MAX<int>(0, x1), MAX<int>(0, y1),
								MIN<int>(screen->w, x2),
								MIN<int>(screen->h, y2));
		const bool seen = (i < Mystery::kHotSpotsCap)
						   && _mystery->_hotSpotsSeen[i];
		if (!seen) {
			// `_DrawSolidRect` — solid white outline (color 0xFF).
			screen->frameRect(rect, 0xFF);
		} else {
			// `_DrawRect` — cycling colors 0xF9..0xFE on each pixel of
			// the outline. We approximate per-pixel cycling with a
			// per-rect phase shift so the rects look animated. Start
			// color is rotated via the global clock.
			const byte palette[6] = { 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE };
			byte color = palette[cyclePhase % 6];
			// Top edge
			for (int x = rect.left; x < rect.right; x++) {
				if (x >= 0 && x < screen->w && rect.top >= 0 && rect.top < screen->h)
					*(byte *)screen->getBasePtr(x, rect.top) = color;
				color = palette[(color - 0xF9 + 1) % 6];
			}
			// Right edge
			for (int y = rect.top; y < rect.bottom; y++) {
				if (rect.right - 1 >= 0 && rect.right - 1 < screen->w && y >= 0 && y < screen->h)
					*(byte *)screen->getBasePtr(rect.right - 1, y) = color;
				color = palette[(color - 0xF9 + 1) % 6];
			}
			// Bottom edge
			for (int x = rect.right - 1; x >= rect.left; x--) {
				if (x >= 0 && x < screen->w && rect.bottom - 1 >= 0 && rect.bottom - 1 < screen->h)
					*(byte *)screen->getBasePtr(x, rect.bottom - 1) = color;
				color = palette[(color - 0xF9 + 1) % 6];
			}
			// Left edge
			for (int y = rect.bottom - 1; y >= rect.top; y--) {
				if (rect.left >= 0 && rect.left < screen->w && y >= 0 && y < screen->h)
					*(byte *)screen->getBasePtr(rect.left, y) = color;
				color = palette[(color - 0xF9 + 1) % 6];
			}
		}
	}

	g_system->unlockScreen();
}

int SiteScreen::hotspotAtPoint(uint siteNum, int x, int y) const {
	const byte *spots = _mystery->hotspots(siteNum);
	const uint16 count = _mystery->hotspotCount(siteNum);
	if (!spots)
		return -1;

	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * 14;
		const int16 x1 = (int16)READ_LE_UINT16(r + 0);
		const int16 y1 = (int16)READ_LE_UINT16(r + 2);
		const int16 x2 = (int16)READ_LE_UINT16(r + 4);
		const int16 y2 = (int16)READ_LE_UINT16(r + 6);
		if (x >= x1 && x < x2 && y >= y1 && y < y2)
			return (int)i;
	}
	return -1;
}

void SiteScreen::onHotspotClicked(uint siteNum, uint hotIdx) {
	debugC(1, kDebugSite, "Site %u: hotspot %u clicked", siteNum, hotIdx);

	// `_DoSiteLoop @ 168d:03f4` (after _DisplayClue):
	//   _HotSpotsSeen[hotspot[+0xa] * 2] = _HotSpotComplete;
	// The "seen" key is the hotspotIndex field (+0xa) — the 1-based
	// ordinal — NOT the array index. Two hotspots can share an ordinal
	// across sites (e.g., a partner's clue you can re-read), so this
	// matters for cross-site state.
	const byte *spots = _mystery->hotspots(siteNum);
	uint hotOrdinal = hotIdx; // fallback to array index
	if (spots) {
		hotOrdinal = READ_LE_UINT16(spots + hotIdx * 14 + 0xa);
	}
	if (hotOrdinal < Mystery::kHotSpotsCap)
		_mystery->_hotSpotsSeen[hotOrdinal] = 1;
	if (hotIdx < Mystery::kHotSpotsCap)
		_mystery->_hotSpotsSeen[hotIdx] = 1;  // also mark by array idx for our render
	_mystery->_searchLocationNumber = (uint16)hotIdx;

	// Bytes 8..9 of each 14-byte hotspot rect = byte offset within the
	// mystery blob pointing at a ClueBlock. Verified against M0.BIN:
	// site 0 hotspot 0 -> 0x0502 -> "Hi! I think somebody's playing a
	// trick on us...". `displayClue` runs the entry's side effects
	// (`_AddNotebook` for ClueEntry +0x30..+0x39, gallery +0x26..+0x2f,
	// onsite +0x1c..+0x25) so we don't need to touch `_cluesFound` here.
	if (spots) {
		const uint16 clueOff = READ_LE_UINT16(spots + hotIdx * 14 + 8);
		debugC(2, kDebugSite, "  hotspot %u -> clue offset 0x%04x",
			   hotIdx, clueOff);
		const byte *clueBlock = _mystery->blobAt(clueOff);
		if (clueBlock) {
			// Snapshot `_cluesFound` BEFORE the clue display so we
			// can detect if any new clue was actually collected
			// (vs. a re-read of an already-found clue) — only worth
			// auto-saving when the player makes progress.
			byte before[Mystery::kCluesFoundCap];
			memcpy(before, _mystery->_cluesFound, sizeof(before));
			// Hand the engine our partner-less backdrop so that
			// `_DoKDAnim` / `playKdAnim` (the camera-style reaction
			// animation that fires when a ClueEntry has +0x3a != -1)
			// can erase the partner's resting frame between cells
			// instead of compositing over it. Cleared after the call
			// so accuse / briefing contexts fall back to their own
			// snapshots.
			_vm->setPartnerEraseBg(&_bgSnapshot);
			_vm->displayClue(clueBlock);
			_vm->setPartnerEraseBg(nullptr);
			// Auto-save when a new clue is found. The original
			// engine has no autosave (saving is a manual SETUP
			// button, `_SaveGame @ 2404:0c87`); we add the autosave
			// here so the player never loses mystery progress.
			// Detected via 0→1 transition in `_cluesFound[]` (set
			// by `applyClueSideEffects` inside `displayClue`).
			bool foundNewClue = false;
			for (uint i = 0; i < Mystery::kCluesFoundCap; i++) {
				if (!before[i] && _mystery->_cluesFound[i]) {
					foundNewClue = true;
					break;
				}
			}
			if (foundNewClue) {
				const Common::Error err =
					_vm->saveProfile(_vm->playerName());
				if (err.getCode() != Common::kNoError)
					warning("auto-save after clue failed: %s",
							err.getDesc().c_str());
			}
		}
	}
	// Caller (`SiteScreen::run`) re-renders the site after this returns.
}

void EEMEngine::playKdAnim(uint16 num) {
	// Mirrors `_DoKDAnim(num) @ 168d:028a` + `_PlayAnimation @ 172b:1f46`:
	//   _SuspendAnimation(WaitHandle);
	//   anim   = WaitAnims[1+num].anim[partner]   (table @ 29be:0228)
	//   x      = WaitAnims[1+num].x[partner]
	//   y      = WaitAnims[1+num].y[partner]
	//   _PlayAnimation(anim, x, y, WaitHandle)
	//     → registers a state-4 (one-shot) animation slot and lets
	//       `_UpdateAnimations` walk the sequence script until 0x80,
	//       then frees this slot and re-activates `WaitHandle`.
	// Our port renders the partner's idle inline in each redraw rather
	// than via a slot system, so we play the one-shot synchronously here
	// (blocking) and resume normal idle rendering when the caller
	// returns. That matches the user-visible effect: the partner's
	// gesture (Jenny taking a picture, etc.) finishes before the
	// speaker portrait + speech balloon appear.
	//
	// `kKdAnimTable` and `kAnimScripts` live at file scope above.
	if (num >= ARRAYSIZE(kKdAnimTable))
		return;

	const uint partner = (_partner == 0) ? 0 : 1;
	const uint16 animId = kKdAnimTable[num][partner];
	const int    px     = (int)kKdAnimTable[num][2 + partner];
	const int    py     = (int)kKdAnimTable[num][4 + partner];

	Animation anim;
	if (!_aniArchive.loadAnimation(animId, anim) || anim.empty()) {
		warning("playKdAnim(%u): anim %u failed to load", num, animId);
		return;
	}

	// `_DoKDAnim` (168d:028a) calls `_PlayAnimation` with state=4 (one-
	// shot), which `_UpdateAnimations` walks until it sees the 0x80
	// terminator and then frees the slot. The same script the
	// site-loop wait anim uses (looping) is what the one-shot plays
	// through ONCE here.
	const AnimScriptRef s = findAnimScript(animId);
	const uint8 *frames = s.frames;
	uint frameCount     = s.len;
	if (frameCount == 0) {
		// Fallback: linear playback through anim cells (better than
		// nothing if a future kdAnim references an unscripted anim).
		frameCount = (uint)anim.size();
	}

	// Erase-source for between-frame redraw. Prefer the partner-less
	// backdrop the caller stashed via `setPartnerEraseBg` (e.g. the
	// site's `_bgSnapshot`, which has the static drops + frame but no
	// partner sprite). Without that, fall back to whatever's currently
	// on screen — which works for full-screen contexts (PDA / accuse /
	// briefing) where there is no separate idle partner overlay to
	// erase, but produces visible "ghosting" against the site's idle
	// partner cell at (6, 80) because it has the resting pose baked in.
	Graphics::ManagedSurface bg(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	if (_partnerEraseBg.w == 320 && _partnerEraseBg.h == 200) {
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)bg.getBasePtr(0, row),
				   (const byte *)_partnerEraseBg.getBasePtr(0, row), 320);
		}
	} else {
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen)
			return;
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)bg.getBasePtr(0, row),
				   (const byte *)screen->getBasePtr(0, row), 320);
		}
		g_system->unlockScreen();
	}

	for (uint i = 0; i < frameCount && !shouldQuit(); i++) {
		const uint frameIdx = frames ? (uint)frames[i] : i;
		if (frameIdx >= anim.size())
			continue;
		const Picture &fr = anim[frameIdx];
		const byte transp = (byte)(fr.flags >> 8);

		// Restore BG, then masked-blit the next frame.
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)bg.getBasePtr(0, row), 320);
		}
		// Anchor-aware: kdAnim cells (0x03/0x04/0x0c/0x0d ...) have
		// non-zero per-frame `miscflags`/`rowoff` (anim 0x03 has
		// rowoff up to 9, anim 0x04 has miscflags = -2). Without
		// applying those, the camera-flash gesture pop-up appears
		// at a fixed pixel rather than translating across cells.
		(void)transp;  // anchored blitter recomputes from p.flags
		blitAnimFrameAnchored(scratch.surfacePtr(), fr, px, py);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		// One frame per `_CheckFrameRate` tick. The original calibrates
		// this to ~10 fps; 100 ms matches what the rest of the engine
		// uses for partner / NPC frame cycling.
		const uint32 wakeup = g_system->getMillis() + 100;
		while (g_system->getMillis() < wakeup && !shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				// Drain events but don't allow skipping mid-animation —
				// the speaker portrait + balloon haven't been drawn yet
				// and a click would otherwise eat the upcoming clue.
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
					return;
			}
			g_system->delayMillis(10);
		}
	}

	// Restore BG so the next caller (speaker portrait blit) starts clean.
	g_system->copyRectToScreen(bg.getPixels(), bg.pitch, 0, 0, 320, 200);
	g_system->updateScreen();
}
} // End of namespace EEM
