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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/paletteman.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/mystery.h"
#include "eem/site.h"

namespace EEM {

const uint kSiteBackendActionFlushPriority = 100;

class SiteBackendActionObserverRegistration {
public:
	SiteBackendActionObserverRegistration(Common::EventObserver *observer)
		: _dispatcher(g_system->getEventManager()->getEventDispatcher()),
		  _observer(observer) {
		if (_dispatcher)
			_dispatcher->registerObserver(_observer, kSiteBackendActionFlushPriority, false);
	}

	~SiteBackendActionObserverRegistration() {
		if (_dispatcher)
			_dispatcher->unregisterObserver(_observer);
	}

private:
	Common::EventDispatcher *_dispatcher;
	Common::EventObserver *_observer;
};

const uint kLondonTravelMatrixDim = 60;
const uint kLondonTravelFrameDelayMs = 120;
const uint16 kLondonTravelLastSiteSentinel = 0x1b;

bool londonTravelSitePic(const Mystery *mystery, uint siteNum, uint16 &sitePic) {
	if (!mystery || !mystery->isLoaded())
		return false;

	if (siteNum < mystery->numSites()) {
		const byte *sd = mystery->siteData(siteNum);
		if (!sd)
			return false;
		sitePic = READ_LE_UINT16(sd);
		return true;
	}

	if (siteNum == kLondonTravelLastSiteSentinel || siteNum == 0xffff) {
		sitePic = kLondonTravelLastSiteSentinel;
		return true;
	}

	return false;
}

static bool paletteEntryIsWhite(const byte *rgb) {
	return rgb[0] >= 0xFC && rgb[1] >= 0xFC && rgb[2] >= 0xFC;
}

static bool paletteEntryIsBlack(const byte *rgb) {
	return rgb[0] <= 0x03 && rgb[1] <= 0x03 && rgb[2] <= 0x03;
}

static byte findPaletteEndpoint(const byte *palette, bool wantWhite,
								byte preferred, byte alternate,
								byte fallback) {
	bool (*matches)(const byte *) = wantWhite ? paletteEntryIsWhite
											  : paletteEntryIsBlack;
	if (matches(palette + preferred * 3))
		return preferred;
	if (matches(palette + alternate * 3))
		return alternate;
	for (uint i = 0; i < 256; i++) {
		if (matches(palette + i * 3))
			return (byte)i;
	}
	return fallback;
}

MacSpritePaletteMap getMacSpritePaletteMap() {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	MacSpritePaletteMap map;
	map.white = findPaletteEndpoint(palette, true, 0x00, 0xFF, 0x00);
	map.black = findPaletteEndpoint(palette, false, 0xFF, 0x00, 0xFF);
	return map;
}

byte mapMacSpriteColor(byte color, const MacSpritePaletteMap &paletteMap) {
	if (color == 0x00)
		return paletteMap.white;
	if (color == 0xFF)
		return paletteMap.black;
	return color;
}

// Masked blit using `transp` = high byte of `pic.flags` (`_Rect_Move_Mask @ 1000:03fc`).
void blitFrame(Graphics::ManagedSurface &dst, const Picture &p,
			   int x, int y, byte transp) {
	dst.transBlitFrom(p.surface, Common::Point(x, y), (uint32)transp);
}

// Masked top-left blit onto a locked screen surface. Clips both src and
// dst against the screen, then delegates to copyRectToSurfaceWithKey
void keyBlitToScreen(Graphics::Surface *screen, const Picture &p,
							int x, int y) {
	if (!screen || p.surface.empty())
		return;
	const Common::Rect dst = Common::Rect(x, y, x + p.surface.w,
										  y + p.surface.h)
		.findIntersectingRect(Common::Rect(screen->w, screen->h));
	if (dst.isEmpty())
		return;
	const Common::Rect src(dst.left - x, dst.top - y,
						   dst.right - x, dst.bottom - y);
	screen->copyRectToSurfaceWithKey(p.surface, dst.left, dst.top,
									 src, (uint32)(byte)(p.flags >> 8));
}

void blitMaskedSurface(Graphics::Surface *screen, const Picture &p,
					   int x, int y) {
	keyBlitToScreen(screen, p, x, y);
}

void blitMacMaskedSurface(Graphics::Surface *dst, const Picture &p,
						  int x, int y, bool flipX,
						  const MacSpritePaletteMap &paletteMap) {
	if (!dst || p.surface.empty())
		return;

	const Common::Rect dstRect =
		Common::Rect(x, y, x + p.surface.w, y + p.surface.h)
			.findIntersectingRect(Common::Rect(dst->w, dst->h));
	if (dstRect.isEmpty())
		return;

	const byte transp = (byte)(p.flags >> 8);
	for (int row = dstRect.top; row < dstRect.bottom; row++) {
		const int srcY = row - y;
		const byte *src = (const byte *)p.surface.getBasePtr(0, srcY);
		byte *out = (byte *)dst->getBasePtr(dstRect.left, row);
		for (int col = dstRect.left; col < dstRect.right; col++, out++) {
			const int relX = col - x;
			const int srcX = flipX ? (p.surface.w - 1 - relX) : relX;
			const byte color = src[srcX];
			if (color != transp)
				*out = mapMacSpriteColor(color, paletteMap);
		}
	}
}

void blitMacMaskedSurface(Graphics::Surface *dst, const Picture &p,
						  int x, int y, bool flipX) {
	const MacSpritePaletteMap paletteMap = getMacSpritePaletteMap();
	blitMacMaskedSurface(dst, p, x, y, flipX, paletteMap);
}

void remapMacSurfaceEndpoints(Graphics::ManagedSurface &surface,
							  const MacSpritePaletteMap &paletteMap) {
	for (int y = 0; y < surface.h; y++) {
		byte *row = (byte *)surface.getBasePtr(0, y);
		for (int x = 0; x < surface.w; x++)
			row[x] = mapMacSpriteColor(row[x], paletteMap);
	}
}

// `_UpdateAnimations @ 172b:09c1`
void blitAnimFrameAnchored(Graphics::Surface *screen, const Picture &p,
						   int anchorX, int anchorY) {
	keyBlitToScreen(screen, p,
					anchorX - (int)(int16)p.miscflags,
					anchorY - (int)(int16)p.rowoff);
}

void blitMacAnimFrameAnchored(Graphics::Surface *dst, const Picture &p,
							  int anchorX, int anchorY,
							  const MacSpritePaletteMap &paletteMap) {
	blitMacMaskedSurface(dst, p,
						 anchorX - (int)(int16)p.miscflags,
						 anchorY - (int)(int16)p.rowoff,
						 false, paletteMap);
}

void blitMacAnimFrameAnchored(Graphics::Surface *dst, const Picture &p,
							  int anchorX, int anchorY) {
	const MacSpritePaletteMap paletteMap = getMacSpritePaletteMap();
	blitMacAnimFrameAnchored(dst, p, anchorX, anchorY, paletteMap);
}

bool readHotspotRect(const byte *r, bool mac, Common::Rect &rect) {
	if (mac) {
		const int16 top = (int16)READ_LE_UINT16(r + 0);
		const int16 left = (int16)READ_LE_UINT16(r + 2);
		const int16 bottom = (int16)READ_LE_UINT16(r + 4);
		const int16 right = (int16)READ_LE_UINT16(r + 6);
		if (right <= left || bottom <= top)
			return false;
		rect = Common::Rect(left, top, right, bottom);
		return true;
	}

	const int16 x1 = (int16)READ_LE_UINT16(r + 0);
	const int16 y1 = (int16)READ_LE_UINT16(r + 2);
	const int16 x2 = (int16)READ_LE_UINT16(r + 4);
	const int16 y2 = (int16)READ_LE_UINT16(r + 6);
	if (x2 <= x1 || y2 <= y1)
		return false;
	rect = Common::Rect(x1, y1, x2, y2);
	return true;
}

Common::Rect siteControlRect(const EEMEngine *vm, const Common::Rect &rect) {
	return (vm && vm->isMacintosh()) ? vm->scaleRect(rect) : rect;
}

// `_ColorCycle @ 172b:2015` — rotate `_fpal[start..end]` by one slot:
// save [start], shift [start..end-1] = [start+1..end], restore saved at
// [end], then re-upload via `_Set_Palette`. 
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
// `_OpenColorCycle @ 2520:04f7`
void cyclePaletteRangeReverse(uint8 start, uint8 end) {
	if (end <= start)
		return;
	const uint count = (uint)end - (uint)start + 1;
	byte buf[256 * 3];
	g_system->getPaletteManager()->grabPalette(buf, start, count);
	const uint last = count - 1;
	const byte savedR = buf[last * 3 + 0];
	const byte savedG = buf[last * 3 + 1];
	const byte savedB = buf[last * 3 + 2];
	for (uint i = last; i > 0; i--) {
		buf[i * 3 + 0] = buf[(i - 1) * 3 + 0];
		buf[i * 3 + 1] = buf[(i - 1) * 3 + 1];
		buf[i * 3 + 2] = buf[(i - 1) * 3 + 2];
	}
	buf[0] = savedR;
	buf[1] = savedG;
	buf[2] = savedB;
	g_system->getPaletteManager()->setPalette(buf, start, count);
}

void applyHotspotGlowPalette() {
	static const byte kAntsGlow[6 * 3] = {
		0x40, 0x40, 0x00, // F9 — dim
		0x80, 0x80, 0x00, // FA
		0xC0, 0xC0, 0x00, // FB
		0xFF, 0xFF, 0x40, // FC — peak
		0xC0, 0xC0, 0x00, // FD
		0x80, 0x80, 0x00, // FE
	};
	g_system->getPaletteManager()->setPalette(kAntsGlow, 0xF9, 6);
}

// `_WaitAnims @ 29be:021c`. 12 bytes per entry, indexed by `siteData[+8]`:
//   +0..1 anim Jake, +2..3 anim Jenny,
//   +4..5 x    Jake, +6..7 x    Jenny,
//   +8..9 y    Jake, +10..11 y    Jenny.
// 7 entries; past entry 6 is `_SiteButtons` rect data.
const uint16 kWaitAnims[7][6] = {
	{ 0x00, 0x0a, 0x06, 0x06, 0x50, 0x50 }, // 0
	{ 0x03, 0x0c, 0x06, 0x06, 0x50, 0x50 }, // 1
	{ 0x01, 0x0b, 0x06, 0x06, 0x50, 0x50 }, // 2
	{ 0x04, 0x0d, 0x06, 0x06, 0x50, 0x50 }, // 3
	{ 0x02, 0x10, 0x06, 0x06, 0x50, 0x50 }, // 4
	{ 0x05, 0x05, 0x06, 0x06, 0x50, 0x50 }, // 5
	{ 0x06, 0x06, 0x06, 0x06, 0x50, 0x50 }, // 6
};

const uint16 kKdAnimTable[6][6] = {
	{ 0x03, 0x0c, 6, 6, 80, 80 }, // 0 — speaker idx 1 wait anim
	{ 0x01, 0x0b, 6, 6, 80, 80 }, // 1 — same as PDA idle
	{ 0x04, 0x0d, 6, 6, 80, 80 }, // 2
	{ 0x02, 0x10, 6, 6, 80, 80 }, // 3 — same as gallery
	{ 0x05, 0x05, 6, 6, 80, 80 }, // 4 — same anim both partners
	{ 0x06, 0x06, 6, 6, 80, 80 }, // 5 — same anim both partners
};

// EEM2 `_DoKDAnim @ 1717:05bf` table @ 2bca:0238 
const uint16 kKdAnimTableLondon[6][6] = {
	{ 0x03, 0x0c, 3, 2, 66, 65 }, // 0
	{ 0x01, 0x0b, 2, 2, 78, 78 }, // 1
	{ 0x04, 0x0d, 2, 2, 78, 78 }, // 2
	{ 0x02, 0x10, 2, 2, 78, 78 }, // 3
	{ 0x05, 0x55, 2, 2, 78, 78 }, // 4 — Jenny uses 0x55
	{ 0x06, 0x2d, 2, 2, 78, 78 }, // 5 — Jenny uses 0x2d
};

// EEM2 `_WaitAnims @ 2bca:022c`.
const uint16 kWaitAnimsLondon[7][6] = {
	{ 0x00, 0x0a, 2, 2, 78, 78 }, // 0
	{ 0x03, 0x0c, 3, 2, 66, 65 }, // 1
	{ 0x01, 0x0b, 2, 2, 78, 78 }, // 2
	{ 0x04, 0x0d, 2, 2, 78, 78 }, // 3
	{ 0x02, 0x10, 2, 2, 78, 78 }, // 4
	{ 0x05, 0x55, 2, 2, 78, 78 }, // 5
	{ 0x06, 0x2d, 2, 2, 78, 78 }, // 6
};

// EEM2 Mac site loop uses its own `_WaitAnims` A5 table
// (`lea (-0x2040,A5)` in FUN_0000d042; data @ 0x153e). These are native
// Mac 512x384 anchors, not a scale transform of the DOS table.
const uint16 kMacWaitAnimsLondon[7][6] = {
	{ 0x00, 0x0a,  0, 3, 150, 150 }, // 0
	{ 0x03, 0x0c,  1, 3, 120, 128 }, // 1
	{ 0x01, 0x0b, 13, 5, 151, 151 }, // 2
	{ 0x04, 0x0d,  2, 5, 148, 152 }, // 3
	{ 0x02, 0x10, 20, 7, 146, 152 }, // 4
	{ 0x05, 0x1b, (uint16)-3, 4, 151, 154 }, // 5
	{ 0x06, 0x1a,  5, 5, 149, 152 }, // 6
};

// Animation script table (`_AnimationSequences @ 29be:22d4`)
// Script byte format:
//   0x80         = restart (loop back to index 0; terminator for one-shots)
//   0x81 N       = jump to byte N (not used in partner subset)
//   other        = frame index to render this tick
//
// Repeated frames are the original's "frame-hold" mechanism: per-tick
// walk advances exactly one entry, so K repeats hold the frame for
// K * `kFramePeriodMs` ≈ K * 140 ms (e.g. [0,0,0,0,0,0,0,0,0,2] →
// nine ticks of frame 0, one tick of frame 2 = "blink with long
// idle hold"). Same scripts serve wait anims (looping) and kd-clue
// reactions (state-4 one-shot — see `_PlayAnimation`); state field
// in the slot differentiates them. `seqnum == animId` per
// `_PlayAnimation @ 172b:1f5d` push order.
// Length counts only playable frames; trailing 0x80 is not a frame.
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
	// 0x02 (29be:18aa) — Jake gallery: brief wave, long hold, second wave, hold.
	{ 0x02, 26, { 0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,0,0,0,0,0 } },
	// 0x03 (29be:18e0) — Jake "lift, hold, lower" gesture
	{ 0x03,  9, { 0,1,2,3,2,2,2,1,0 } },
	// 0x04 (29be:18f4) — Jake bigger gesture (camera flash-style)
	{ 0x04, 13, { 0,1,2,3,4,5,4,4,4,3,2,1,0 } },
	// 0x05 (29be:1910) — Jake/Jenny shared (speaker 5): held idle, peak
	{ 0x05, 13, { 0,0,0,1,2,3,2,1,0,0,0,0,0 } },
	// 0x06 (29be:192c) — empty (byte 0 = 0x80, immediate END).
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
	// 0x10 (29be:1956, alias of 0x09) — Jenny gallery wait.
	{ 0x10,  9, { 0,0,0,1,0,0,0,0,0 } },
	// 0x11 (29be:1992) — Jenny BigMap entrance count-up 0..7 (one-shot).
	{ 0x11,  8, { 0,1,2,3,4,5,6,7 } },
	// 0x12 (29be:197e) — Jake BigMap entrance count-down 8..0 (one-shot).
	{ 0x12,  9, { 8,7,6,5,4,3,2,1,0 } },
	// 0x13 (29be:1992, alias of 0x11) — Jake BigMap idle walk-cycle.
	{ 0x13,  8, { 0,1,2,3,4,5,6,7 } },
	// 0x14 (29be:196a) — BigMap idle walk-cycle 0..8.
	{ 0x14,  9, { 0,1,2,3,4,5,6,7,8 } },
	// 0x15 (29be:185e, alias of 0x00) — Jake CaseSelection greeter.
	{ 0x15, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// 0x16 (29be:185e, alias of 0x00) — Jenny CaseSelection greeter.
	{ 0x16, 10, { 0,0,0,0,0,0,0,0,0,2 } },
	// Site / drop scripts ≤28 frames (`_AnimationSequences @ 29be:22d4`).
	// 0x1b (29be:192e, alias of 0x07) — walk-cycle 0..9.
	{ 0x1b, 10, { 0,1,2,3,4,5,6,7,8,9 } },
	// 0x1c (29be:21a8) — short 6-frame count-up.
	{ 0x1c,  6, { 0,1,2,3,4,5 } },
	// 0x1d (29be:21a8, alias of 0x1c).
	{ 0x1d,  6, { 0,1,2,3,4,5 } },
	// 0x21 (29be:1b86) — paired-step idle bob.
	{ 0x21, 14, { 0,0,0,1,1,2,2,3,3,3,2,2,1,1 } },
	// 0x25 (29be:218a) — 3-frame trigger.
	{ 0x25,  3, { 0,1,2 } },
	// 0x26 (29be:1d3e) — count-up 0..17 (18 frames).
	{ 0x26, 18, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17 } },
	// 0x27 (29be:1d64) — count-up 0..11 (12 frames).
	{ 0x27, 12, { 0,1,2,3,4,5,6,7,8,9,10,11 } },
	// 0x2a (29be:1e50) — 5-step micro-anim.
	{ 0x2a,  5, { 0,1,2,2,3 } },
	// 0x2e (29be:21ce) — count-up 0..12 (13 frames).
	{ 0x2e, 13, { 0,1,2,3,4,5,6,7,8,9,10,11,12 } },
	// 0x2f (29be:21ea) — count-up 0..22 (23 frames).
	{ 0x2f, 23, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
				  20,21,22 } },
	// 0x32 (29be:219c) — count-up 0..4 (5 frames).
	{ 0x32,  5, { 0,1,2,3,4 } },
	// 0x33 (29be:2192) — count-up 0..3 (4 frames).
	{ 0x33,  4, { 0,1,2,3 } },
	// 0x34 (29be:219c, alias of 0x32).
	{ 0x34,  5, { 0,1,2,3,4 } },
	// 0x35 (29be:21b6) — count-up 0..10 (11 frames).
	{ 0x35, 11, { 0,1,2,3,4,5,6,7,8,9,10 } },
};

struct AnimScriptLong {
	uint16 seqnum;
	uint16 len;
	const uint8 *frames;
};

// Briefing animations — `_DoInitClues @ 1a35:0411` always uses anim ID
// 0x17 (game) / 0x18 (book) / 0x19 (nancy) regardless of partner
const uint8 kScript17[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29
};
const uint8 kScript18[] = {
	0,1,2,3,4,5,6,7,8,8,8,8,8,8,8,8,
	8,8,8,8,8,8,8,9,10,11,12,13,14,15
};
const uint8 kScript19[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,2,3,4,5,6,7,8,9,10,11,12
};

// Site / NPC drop scripts
const uint8 kScript1a[] = {
	0,1,2,3,4,5,6,7,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	1,2,3,4,5,6,7,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	7,6,5,4,3,2,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript1e[] = {
	0,1,2,3,3,3,3,4,4,3,4,4,4,4,4,3,
	5,5,5,5,5,5,5,5,4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,6,5,6,5,7,7,7,7,7,7,
	7,8,7,7,7,7,7,8,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript1f[] = {
	0,1,2,3,4,5,
	0,0,0,0,
	1,2,3,4,5,
	0,0,0,0,0,
	6,7,8,8,8,7,6,7,8,8,8,7,
	6,7,8,8,8,7,6,7,8,8,8,7,
	6,
	0,0,0,0,0
};

const uint8 kScript20[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	20,21,22,23,24,25,26,27,28,29,30,31,32,33
};

const uint8 kScript22[] = {
	0,
	1,1,1,1,1,
	2,2,2,2,2,
	3,3,3,3,3,
	4,4,4,4,
	5,5,5,5,
	6,6,6,6,
	7,7,7,7,
	8,8,8,8,
	9,9,9,9,
	10,10,10,10,10,
	11,11,11,11,11,
	12,12,12,12,12,12,
	13,13,13,13,13,
	14,14,14,14,
	15,15,15,15,
	16,16,16,16,16,16,16,
	17,17,17,17,
	18,18,18,18,
	19,19,19,19,
	20,20,20,20,
	21,21,21,21,
	22,22,22,22,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript23[] = {
	0,1,1,1,1,1,1,
	2,3,4,3,2,
	5,5,5,
	2,3,4,3,3,3,3,3,
	0,0,0,0,0,0
};

const uint8 kScript24[] = {
	0,0,1,1,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript28[] = {
	0,1,1,2,2,
	3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
	2,2,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript29[] = {
	0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,
	11,11,12,12,13,13,14,14,15,15,16,16,17,17,18,18,19,19,
	20,20,21,21,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript2b[] = {
	0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,
	4,4,4,4,5,5,5,5,6,6,6,6,7,7,7,7,
	8,8,8,8,9,9,9,9,10,10,10,10,11,11,11,11
};

const uint8 kScript2c[] = {
	0,1,2,3,4,5,
	0,
	6,7,8,9,10,10,10,10,10,10,
	11,11,11,
	12,13,14,15,
	0,
	16,17,18,19,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript2d[] = {
	0,0,0,0,0,0,0,0,
	1,1,1,1,1,1,1,1,
	2,2,2,2,2,2,2,2,
	3,3,3,3,3,3,3,3,
	4,4,4,4,4,4,4,4,
	5,5,5,5,5,5,5,5,
	6,6,6,6,6,6,6,6,
	7,7,7,7,7,7,7,7,
	8,8,8,8,8,8,8,8,
	9,9,9,9,9,9,9,9,
	10,10,10,10,10,10,10,10,
	11,11,11,11,11,11,11,11
};

const uint8 kScript30[] = {
	0,0,
	1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	0,0,0,0,0,0,0,0,0,0,
	19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,
	5,4,4,3,3,2,2,1,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const uint8 kScript31[] = {
	0,0,0,1,1,1,
	0,0,0,1,1,1,
	2,2,2,3,3,3,
	2,2,2,3,3,3,
	4,4,4,5,5,5,
	4,4,4,5,5,5,
	3,3,3,2,2,2,
	3,3,3,2,2,2,
	1,1,1,
	0,0,0,
	1,1,1
};

const uint8 kScript36[] = {
	0,1,2,3,4,5,6,7,8,
	1,2,3,4,5,6,7,8,
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
	8,7,6,5,4,3,2,1,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

const AnimScriptLong kAnimScriptsLong[] = {
	{ 0x17, 30,  kScript17 },
	{ 0x18, 30,  kScript18 },
	{ 0x19, 30,  kScript19 },
	{ 0x1a, 77,  kScript1a },
	{ 0x1e, 76,  kScript1e },
	{ 0x1f, 50,  kScript1f },
	{ 0x20, 34,  kScript20 },
	{ 0x22, 115, kScript22 },
	{ 0x23, 29,  kScript23 },
	{ 0x24, 58,  kScript24 },
	{ 0x28, 45,  kScript28 },
	{ 0x29, 58,  kScript29 },
	{ 0x2b, 48,  kScript2b },
	{ 0x2c, 54,  kScript2c },
	{ 0x2d, 96,  kScript2d },
	{ 0x30, 86,  kScript30 },
	{ 0x31, 57,  kScript31 },
	{ 0x36, 60,  kScript36 },
};

// `_PatientSequence` and `_ImpatientSequence` are standalone script
// pointers. CD has the data but never calls the switchers; floppy calls 
// them from `_DoSiteLoop_Floppy` (via `_Switch2Patient` / `_Switch2Impatient`). 
// We intentionally enable the same switch for both builds.
const uint8 kPatientSequence[]   = { 0,0,0,0,0,0,0,0,0,2 };
const uint8 kImpatientSequence[] = { 0,1,0,1,0,1,0,1,2,1 };

// Test-shortened impatience delay. The original stores an hour-rounded
// wall-clock value via DOS gettime; this keeps the same reset/switch
// behavior but makes the feature observable during normal testing.
const uint32 kImpatienceDelayMs = 60 * 1000;

// EEM2 ("Eagle Eye Mysteries in London") animation scripts. EEM2 ships its own
// `_AnimationSequences @ 2bca:2e2e` (read from EEM2CD.EXE); essentially every
// non-trivial partner / KD / site script differs from EEM1's — e.g. 0x18 is a
// plain count-up 0..16 in EEM2 vs "0..8, hold, 9..15" in EEM1 (`kScript18`),
// 0x14 is 0..10 vs 0..8. Using EEM1's scripts on EEM2's cells plays the wrong
// frame sequence (visible corruption). These override the EEM1 tables when the
// London variant is active; any seqnum not listed falls through to the shared
// EEM1 scripts below. Only the seqnums that actually differ are listed.
//
// NOTE: EEM2 scripts 0x27/0x2e/0x30 end with a `0x81 N` jump (loop back to
// entry N) rather than a 0x80 restart. `frameFromScriptAtTick` has no jump
// support (EEM1 never used it), so the flat frame list is stored: correct for a
// one-shot play-through, but a looped play replays the intro instead of just
// the post-jump tail. Acceptable for these site NPC fidgets; revisit if needed.
const uint8 kScript06London[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,11,11,5,6,7,8,9,10,
	11,11,11,5,6,7,8,9,10,11,11,11,5,4,3,2,1,0,
};
const uint8 kScript1dLondon[] = {
	0,1,0,1,1,0,0,2,3,4,5,6,7,8,9,1,1,1,1,0,
	0,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
	2,2,2,2,2,2,
};
const uint8 kScript1eLondon[] = {
	0,1,0,1,0,1,1,0,0,2,2,2,2,3,3,4,4,5,6,6,
	4,3,2,1,0,1,0,1,0,1,1,0,0,
};
const uint8 kScript1fLondon[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0,1,1,1,1,2,1,3,3,4,5,6,5,6,4,1,1,1,
};
const uint8 kScript23London[] = {
	0,0,0,0,1,1,2,2,3,3,3,4,4,3,3,3,4,4,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
const uint8 kScript25London[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
	19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,19,
};
const uint8 kScript27London[] = {  // 0x81 jump->22
	0,0,0,0,0,0,0,0,0,0,1,0,1,2,3,2,4,5,6,6,
	7,6,8,6,7,7,7,7,8,
};
const uint8 kScript2bLondon[] = {
	0,1,0,1,1,1,0,1,2,2,2,2,3,4,3,4,3,2,1,0,
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
const uint8 kScript2dLondon[] = {
	0,1,2,3,4,5,6,7,8,9,10,11,11,11,5,6,7,8,9,10,
	11,11,11,5,6,7,8,9,10,11,11,11,5,4,3,2,1,0,
};
const uint8 kScript31London[] = {
	32,32,32,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
	17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,
	37,38,39,40,41,42,43,44,45,46,32,32,32,32,32,32,32,32,32,32,
	32,32,
};
const AnimScript kAnimScriptsLondon[] = {
	{ 0x01,  6, { 0,1,2,3,4,5 } },                                  // PDA idle
	{ 0x03, 15, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 } },           // gesture
	{ 0x04, 23, { 0,1,2,3,4,5,5,5,6,5,5,5,5,6,5,5,5,5,4,3,2,1,0 } },// big gesture
	{ 0x05, 11, { 0,1,2,3,4,4,4,3,2,1,0 } },
	{ 0x0b,  6, { 0,1,2,3,4,5 } },                                  // Jenny PDA
	{ 0x0c, 15, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14 } },           // Jenny gesture
	{ 0x0d, 23, { 0,1,2,3,4,5,5,5,6,5,5,5,5,6,5,5,5,5,4,3,2,1,0 } },
	{ 0x0e, 22, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,20 } },
	{ 0x14, 11, { 0,1,2,3,4,5,6,7,8,9,10 } },                       // BigMap walk
	{ 0x18, 17, { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16 } },     // case-intro entrance
	{ 0x19,  2, { 0,1 } },
	{ 0x1a,  4, { 0,1,2,3 } },
	{ 0x1b, 12, { 0,0,1,1,2,2,3,3,4,4,5,5 } },
	{ 0x1c, 19, { 11,11,11,0,1,2,3,4,5,6,7,8,9,10,11,11,11,11,11 } },
	{ 0x20, 19, { 0,1,1,2,3,3,0,0,1,1,1,0,3,2,0,1,4,4,4 } },
	{ 0x21,  5, { 0,0,1,1,2 } },
	{ 0x22, 10, { 0,0,0,1,0,1,0,1,1,1 } },
	{ 0x24, 18, { 0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 } },
	{ 0x26, 20, { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,2,1,3 } },
	{ 0x28, 16, { 0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7 } },
	{ 0x29, 14, { 0,1,2,3,4,4,3,4,3,4,3,2,1,0 } },
	{ 0x2a, 23, { 0,0,0,1,2,3,2,1,0,0,0,0,1,2,3,2,1,0,1,2,3,2,1 } },
	{ 0x2c, 22, { 15,15,15,15,15,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 } },
	{ 0x2e, 16, { 0,0,0,1,2,3,4,5,6,7,8,9,10,11,12,12 } },          // 0x81 jump->15
	{ 0x2f,  6, { 0,1,2,3,4,5 } },
	{ 0x30, 26, { 18,18,18,18,18,18,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,18 } }, // 0x81 jump->25
	{ 0x32, 27, { 0,0,0,0,1,1,0,0,0,1,0,0,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1 } },
	{ 0x33, 27, { 4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,0,1,2,3,4,4,4,0,1,2,3 } },
	{ 0x34,  4, { 0,1,2,3 } },
	{ 0x35, 21, { 9,9,9,9,9,9,9,9,9,9,9,9,0,1,2,3,4,5,6,7,8 } },
	{ 0x36, 12, { 0,0,0,0,0,1,2,3,4,5,6,6 } },
	{ 0x55, 11, { 0,1,2,3,4,4,4,3,2,1,0 } },                        // KD Jenny reaction-4
};
const AnimScriptLong kAnimScriptsLondonLong[] = {
	{ 0x06, 38, kScript06London },
	{ 0x1d, 46, kScript1dLondon },
	{ 0x1e, 33, kScript1eLondon },
	{ 0x1f, 38, kScript1fLondon },
	{ 0x23, 39, kScript23London },
	{ 0x25, 59, kScript25London },
	{ 0x27, 29, kScript27London },
	{ 0x2b, 39, kScript2bLondon },
	{ 0x2d, 38, kScript2dLondon },
	{ 0x31, 62, kScript31London },
};

// Set true for the London variant so findAnimScript uses the EEM2 tables.
bool g_londonAnimScripts = false;

void setLondonAnimScripts(bool enabled) {
	g_londonAnimScripts = enabled;
}

struct AnimScriptRef {
	const uint8 *frames;
	uint16 len;
};
AnimScriptRef findAnimScript(uint16 seqnum) {
	if (g_londonAnimScripts) {
		for (uint i = 0; i < ARRAYSIZE(kAnimScriptsLondon); i++) {
			if (kAnimScriptsLondon[i].seqnum == seqnum) {
				AnimScriptRef r;
				r.frames = kAnimScriptsLondon[i].frames;
				r.len = kAnimScriptsLondon[i].len;
				return r;
			}
		}
		for (uint i = 0; i < ARRAYSIZE(kAnimScriptsLondonLong); i++) {
			if (kAnimScriptsLondonLong[i].seqnum == seqnum) {
				AnimScriptRef r;
				r.frames = kAnimScriptsLondonLong[i].frames;
				r.len = kAnimScriptsLondonLong[i].len;
				return r;
			}
		}
	}
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

// Original frame period from `_InitFrameCounter @ 1a35:01ae`:
const uint kFramePeriodMs = 140;

uint frameFromScriptAtTick(const uint8 *frames, uint len,
								  uint numFrames, uint32 tickMs) {
	if (!frames || len == 0)
		return numFrames > 0 ? (uint)((tickMs / kFramePeriodMs) % numFrames) : 0;
	const uint scriptIdx = (uint)((tickMs / kFramePeriodMs) % len);
	const uint frame     = frames[scriptIdx];
	return (numFrames > 0) ? MIN<uint>(frame, numFrames - 1) : 0;
}

// Looping path of `_UpdateAnimations`: walk the script one entry per
// `_CheckFrameRate` tick (`kFramePeriodMs` ~= 140 ms), wrap on 0x80.
uint partnerFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs) {
	const AnimScriptRef s = findAnimScript(seqnum);
	return frameFromScriptAtTick(s.frames, s.len, numFrames, tickMs);
}

uint oneShotFrameAtTick(uint16 seqnum, uint numFrames, uint32 tickMs) {
	const AnimScriptRef s = findAnimScript(seqnum);
	const uint tick = (uint)(tickMs / kFramePeriodMs);
	if (!s.frames || s.len == 0)
		return numFrames > 0 ? MIN<uint>(tick, numFrames - 1) : 0;
	const uint scriptIdx = MIN<uint>(tick, (uint)s.len - 1);
	const uint frame = s.frames[scriptIdx];
	return numFrames > 0 ? MIN<uint>(frame, numFrames - 1) : 0;
}

uint32 oneShotDurationMs(uint16 seqnum, uint numFrames) {
	const AnimScriptRef s = findAnimScript(seqnum);
	const uint count = (s.frames && s.len) ? (uint)s.len : numFrames;
	return (uint32)count * kFramePeriodMs;
}

// Play `unfold` once, then loop `waitSeq` forever. Mirrors the
// original's slot-script-swap idiom
uint oneShotThenLoopFrameAtTick(const uint8 *unfold, uint unfoldLen,
									   const uint8 *waitSeq, uint waitSeqLen,
									   uint numFrames, uint32 elapsedMs) {
	const uint tick = elapsedMs / kFramePeriodMs;
	const uint frame = (tick < unfoldLen)
		? unfold[tick]
		: waitSeq[(tick - unfoldLen) % waitSeqLen];
	return (numFrames > 0) ? MIN<uint>(frame, numFrames - 1) : 0;
}

uint bigMapPartnerFrameAtTick(uint numFrames, uint32 elapsedMs, bool london) {
	if (london) {
		static const uint8 kUnfoldL[]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
		static const uint8 kWaitSeqL[] = { 10, 11, 10, 10, 10, 10, 10, 10, 10 };
		return oneShotThenLoopFrameAtTick(kUnfoldL, ARRAYSIZE(kUnfoldL),
										  kWaitSeqL, ARRAYSIZE(kWaitSeqL),
										  numFrames, elapsedMs);
	}
	// EEM1 (11-frame anim): entrance {0..8}, idle `_BigMapWaitSeq` {9,..,10,..}.
	static const uint8 kUnfold[]  = { 0, 1, 2, 3, 4, 5, 6, 7, 8 };
	static const uint8 kWaitSeq[] = { 9, 9, 9, 9, 10, 9, 9, 9, 9 };
	return oneShotThenLoopFrameAtTick(kUnfold, ARRAYSIZE(kUnfold),
									  kWaitSeq, ARRAYSIZE(kWaitSeq),
									  numFrames, elapsedMs);
}

uint bigMapDetailPartnerFrameAtTick(uint numFrames, uint32 elapsedMs) {
	static const uint8 kUnfold[]  = { 0, 1, 2, 3, 4, 5, 6, 7 };
	static const uint8 kWaitSeq[] = {
		7, 7, 7, 10, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
	};
	return oneShotThenLoopFrameAtTick(kUnfold, ARRAYSIZE(kUnfold),
									  kWaitSeq, ARRAYSIZE(kWaitSeq),
									  numFrames, elapsedMs);
}

bool SiteScreen::playLondonTravelAnimation(uint fromSite, uint toSite) {
	if (!_vm || !_mystery || !_vm->isLondon())
		return false;

	uint16 fromPic = 0;
	uint16 toPic = 0;
	if (!londonTravelSitePic(_mystery, fromSite, fromPic) ||
		!londonTravelSitePic(_mystery, toSite, toPic))
		return false;
	if (fromPic >= kLondonTravelMatrixDim || toPic >= kLondonTravelMatrixDim)
		return false;

	Common::File matrix;
	if (!matrix.open(Common::Path("TRAVEL.BIN"))) {
		warning("London travel: TRAVEL.BIN missing");
		return false;
	}

	const uint32 matrixOff = (uint32)fromPic * kLondonTravelMatrixDim + toPic;
	if (!matrix.seek(matrixOff)) {
		warning("London travel: seek to matrix offset %u failed", matrixOff);
		return false;
	}

	byte travelKind = 0;
	if (matrix.read(&travelKind, 1) != 1) {
		warning("London travel: short read at matrix offset %u", matrixOff);
		return false;
	}
	if (travelKind == 0)
		return false;
	if (travelKind > 3) {
		warning("London travel: invalid matrix value %u for pic %u -> %u",
				travelKind, fromPic, toPic);
		return false;
	}

	const uint partnerSuffix = (travelKind == 3) ? 0 : _vm->getPartnerIndex();
	const Common::String name = Common::String::format("TRAVEL%u%u.ANM",
		(uint)travelKind - 1, partnerSuffix);

	debugC(1, kDebugSite,
		   "London travel: site %u/pic %u -> site %u/pic %u, kind=%u, anim=%s",
		   fromSite, fromPic, toSite, toPic, travelKind, name.c_str());
	_vm->startLondonTravelMusic(travelKind);
	fadeCurrentPaletteToBlack();
	_vm->playAnm(Common::Path(name), kLondonTravelFrameDelayMs,
				 /* holdLastFrame= */ false, /* fadeIn= */ true,
				 /* setSkipIntroOnEsc= */ false);
	_vm->stopMusic();
	return true;
}

void SiteScreen::enter(uint siteNum, bool resetPartnerMood) {
	if (!_mystery || !_mystery->isLoaded()) {
		warning("SiteScreen::enter: no mystery loaded");
		return;
	}
	if (siteNum >= _mystery->numSites()) {
		warning("SiteScreen::enter: site %u out of range (max %u)",
				siteNum, _mystery->numSites());
		return;
	}

	_waitPhaseAnchor = g_system->getMillis();
	if (resetPartnerMood) {
		_partnerWaitMood = kPartnerWaitDefault;
		initImpatienceCounter();
	}

	const bool firstVisit = (siteNum < Mystery::kVisitedSiteCap)
							 && (_mystery->_visitedSite[siteNum] == 0);

	_mystery->_siteNumber = siteNum;
	debugC(1, kDebugSite, "Entering site %u (%u hotspots)",
		   siteNum, _mystery->hotspotCount(siteNum));

	const byte *sd = _mystery->siteData(siteNum);
	const bool playArrival = _vm->shouldPlaySiteArrival(siteNum);

	if (playArrival) {
		if (_vm->isLondon()) {
			bool showedApproach = false;
			const uint16 approachId = sd ? READ_LE_UINT16(sd + 2) : 0xffff;
			if (firstVisit && approachId != 0xffff)
				showedApproach = _vm->doLondonApproach(approachId);
			if (!showedApproach)
				playLondonTravelAnimation(_mystery->_lastSite, siteNum);
		} else {
			_vm->startTravelMusic();
		}
	}

	const bool compactSite = _vm->isFloppy() ||
							 (_mystery && _mystery->usesCompactMacData());
	uint16 sitepic = 0;
	if (sd) {
		if (compactSite) {
			const uint16 dropsOff = READ_LE_UINT16(sd);
			const byte *drops = _mystery->blobAt(dropsOff);
			if (drops)
				sitepic = (uint16)drops[0];
		} else {
			sitepic = READ_LE_UINT16(sd);
		}
	}
	_vm->setSitePaletteForSite(sitepic);

	applyHotspotGlowPalette();

	renderBackground(siteNum);

	if (playArrival) {
		if (compactSite)
			renderFloppyDrops(siteNum);
		else
			renderStaticDrops(siteNum);
		renderAnimatedDrops(siteNum, g_system->getMillis());
		const bool skippedArrival = enterSiteAnim();
		_vm->markSiteArrivalPlayed(siteNum);
		if (!_vm->isFloppy() && !_vm->isLondon()) {
			if (skippedArrival)
				_vm->stopMusic();
			else
				_vm->waitForMusicDone();
		}
		renderBackground(siteNum);
	}

	if (compactSite)
		renderFloppyDrops(siteNum);
	else
		renderStaticDrops(siteNum);

	captureBgSnapshot();
	_snapshotSite = (int)siteNum;

	scanColorCycles(siteNum);

	const uint32 now = g_system->getMillis();
	renderAnimatedDrops(siteNum, now);
	renderPartner(siteNum, now);

	renderHotspots(siteNum);
	g_system->updateScreen();

	// First-visit dialog. `_DoSiteLoop @ 168d:03f4`:
	//   if (_VisitedSite[_SiteNumber] == 0) {
	//       _DisplayClue(_Mystery + SiteIndex[siteNum*6 + 2], 1);
	//       _VisitedSite[_SiteNumber] = 1;
	//   }
	// SiteIndex[+2..+3] = byte offset of entry-clue ClueBlock.
	if (firstVisit && !(_mystery && _mystery->usesCompactMacData())) {
		const byte *idx = _mystery->siteIndexEntry(siteNum);
		if (idx) {
			const uint16 clueOff = READ_LE_UINT16(idx + 2);
			if (clueOff != 0xFFFF) {
				const byte *clueBlock = _mystery->blobAt(clueOff);
				if (clueBlock)
					displayClueAndAutosave(clueBlock);
			}
		}
		if (siteNum < Mystery::kVisitedSiteCap)
			_mystery->_visitedSite[siteNum] = 1;
		// Dialog overlay leaves portrait/balloon residue; refresh & re-snapshot.
		renderBackground(siteNum);
		if (compactSite)
			renderFloppyDrops(siteNum);
		else
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

void SiteScreen::initImpatienceCounter() {
	_impatientDeadlineMs = g_system->getMillis() + kImpatienceDelayMs;
}

bool SiteScreen::checkImpatienceCounter() {
	const uint32 now = g_system->getMillis();
	const bool impatient = (int32)(now - _impatientDeadlineMs) >= 0;
	if (impatient)
		initImpatienceCounter();
	return impatient;
}

void SiteScreen::notePartnerActivity() {
	const bool wasImpatient = _partnerWaitMood == kPartnerWaitImpatient;
	_partnerWaitMood = kPartnerWaitPatient;
	initImpatienceCounter();
	if (wasImpatient)
		debugC(1, kDebugSite, "Partner impatience: reset to patient");
}

bool SiteScreen::notifyEvent(const Common::Event &event) {
	if (event.type != Common::EVENT_CUSTOM_BACKEND_ACTION_START)
		return false;

	if (_snapshotSite < 0 || g_system->isOverlayVisible())
		return false;

	syncCompositedScreen();
	g_system->updateScreen();
	return false;
}

void SiteScreen::run() {
	if (!_mystery || !_mystery->isLoaded())
		return;

	uint cur = _mystery->_siteNumber;
	if (cur >= _mystery->numSites())
		cur = 0;
	_mystery->_pendingSiteJump = 0;
	if (_mystery->_siteReturnDepth > Mystery::kVisitedSiteCap)
		_mystery->_siteReturnDepth = 0;
	_snapshotSite = -1;
	const Common::Rect pdaSiteRect =
		siteControlRect(_vm, kPdaSiteRect);
	const Common::Rect pdaPartnerFootMapRect =
		siteControlRect(_vm, kPdaPartnerFootMapRect);
	const Common::Rect pdaPartnerHeadHintRect =
		siteControlRect(_vm, kPdaPartnerHeadHintRect);
	SiteBackendActionObserverRegistration backendActionRegistration(this);
	enter(cur);
	Common::Point mouse = g_system->getEventManager()->getMousePos();
	updateHotspotCursor(cur, mouse.x, mouse.y);

	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				_vm->setHotspotMouseCursor(false);
				_vm->stopMusic();
				return;

			case Common::EVENT_MOUSEMOVE:
				updateHotspotCursor(cur, event.mouse.x, event.mouse.y);
				break;

			case Common::EVENT_LBUTTONDOWN: {
				if (pdaSiteRect.contains(event.mouse.x, event.mouse.y)) {
					notePartnerActivity();
					_vm->setHotspotMouseCursor(false);
					_vm->setNextScreen(kScreenNotebook);
					_vm->stopMusic();
					return;
				}
				if (pdaPartnerFootMapRect.contains(event.mouse.x,
												   event.mouse.y)) {
					notePartnerActivity();
					_vm->setHotspotMouseCursor(false);
					if (_vm->isLondon() && _mystery->_siteReturnDepth != 0) {
						const uint16 depth = --_mystery->_siteReturnDepth;
						const uint16 returnSite = _mystery->_siteReturnStack[depth];
						_mystery->_siteReturnStack[depth] = 0;
						if (returnSite < _mystery->numSites()) {
							debugC(1, kDebugSite,
								   "London: returning from site %u to site %u",
								   cur, returnSite);
							_mystery->_lastSite = (uint16)cur;
							_mystery->_siteNumber = returnSite;
							cur = returnSite;
							enter(cur);
							mouse = g_system->getEventManager()->getMousePos();
							updateHotspotCursor(cur, mouse.x, mouse.y);
							break;
						}
						warning("London site return target %u out of range",
								returnSite);
					}
					// CD: _NextScreen=1, floppy=2.
					_vm->setNextScreen(_vm->isFloppy() ? kScreenMapAlt
													   : kScreenMap);
					_vm->stopMusic();
					return;
				}
				if (pdaPartnerHeadHintRect.contains(event.mouse.x,
													event.mouse.y)) {
					_vm->setHotspotMouseCursor(false);
					_vm->doHelp();
					notePartnerActivity();
					enter(cur, false);
					// Re-evaluate cursor against CURRENT pointer position.
					mouse = g_system->getEventManager()->getMousePos();
					updateHotspotCursor(cur, mouse.x, mouse.y);
					break;
				}
				const int idx = hotspotAtPoint(cur, event.mouse.x, event.mouse.y);
				if (idx >= 0) {
					_vm->setHotspotMouseCursor(false);
					_mystery->_pendingSiteJump = 0;
					onHotspotClicked(cur, (uint)idx);
					notePartnerActivity();
					const uint16 jumpSite = _mystery->_pendingSiteJump;
					_mystery->_pendingSiteJump = 0;
					if (_vm->isLondon() && jumpSite != 0) {
						if (jumpSite < _mystery->numSites()) {
							if (_mystery->_siteReturnDepth < Mystery::kVisitedSiteCap) {
								_mystery->_siteReturnStack[_mystery->_siteReturnDepth++] =
									(uint16)cur;
							} else {
								warning("London site return stack full; "
										"jumping without return site");
							}
							debugC(1, kDebugSite,
								   "London: hotspot jump from site %u to site %u",
								   cur, jumpSite);
							_mystery->_lastSite = (uint16)cur;
							_mystery->_siteNumber = jumpSite;
							cur = jumpSite;
							enter(cur);
						} else {
							warning("London hotspot jump target %u out of range",
									jumpSite);
							enter(cur, false);
						}
					} else {
						enter(cur, false);
					}
					// Use CURRENT pointer position (click pos still in rect).
					mouse = g_system->getEventManager()->getMousePos();
					updateHotspotCursor(cur, mouse.x, mouse.y);
				} else {
					notePartnerActivity();
				}
				break;
			}

			case Common::EVENT_KEYDOWN:
				notePartnerActivity();
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_vm->setHotspotMouseCursor(false);
					_vm->openMainMenuDialog();
					enter(cur, false);
					mouse = g_system->getEventManager()->getMousePos();
					updateHotspotCursor(cur, mouse.x, mouse.y);
				}
				break;

			default:
				break;
			}
		}

		if (!_mystery || !_mystery->isLoaded()) {
			_vm->stopMusic();
			return;
		}

		const uint32 now = g_system->getMillis();
		if (_snapshotSite == (int)cur &&
			now - _lastTickMs >= kFramePeriodMs) {
			if (checkImpatienceCounter()) {
				_partnerWaitMood = kPartnerWaitImpatient;
				debugC(1, kDebugSite, "Partner impatience: switched to impatient");
			}
			restoreBgSnapshot();
			renderAnimatedDrops(cur, now);
			renderPartner(cur, now);
			renderHotspots(cur);
			applyColorCycles();
			_lastTickMs = now;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}
// `_EnterSiteAnim @ 1000:9b21`. Two phases (partner-dependent):
//   Phase 1 — skateboard scroll: anim 6 (Jake) / 0xe (Jenny).
//             Slides from (320-w, 199-h) leftward off-screen.
//   Phase 2 — KD slide-in: anim 7 (Jake) / 0xf (Jenny).
//             Slides from x=-w at y=0x8b/0x8e until x=0.
bool SiteScreen::enterSiteAnim() {
	if (!_vm || !_mystery)
		return false;
	const uint8 partner = _vm->getPartnerIndex();
	const uint kSkateAni = (partner == 0) ? 6  : 0xe;
	const uint kKDAni    = (partner == 0) ? 7  : 0xf;
	const int baseKDY = (partner == 0) ? 0x8b : 0x8e;
	const int kKDY = _vm->isMacintosh() ? _vm->scaleY(baseKDY) : baseKDY;
	const int sw = _vm->screenWidth();
	const int sh = _vm->screenHeight();

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return false;
	Graphics::ManagedSurface bg(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());
	bg.simpleBlitFrom(*screen);
	g_system->unlockScreen();
	const bool mac = _vm->isMacintosh();
	MacSpritePaletteMap macPaletteMap = {0x00, 0xFF};
	if (mac)
		macPaletteMap = getMacSpritePaletteMap();

	if (_vm->isLondon()) {
		const uint animId = (partner == 0) ? 7 : 0xf;
		int anchorX = mac ? (int)(int16)kMacWaitAnimsLondon[0][2 + partner] : 0;
		int anchorY = mac ? (int)(int16)kMacWaitAnimsLondon[0][4 + partner]
						  : ((partner == 0) ? 0x50 : 0x4e);
		bool alignToIdleBottom = false;
		int idleBottom = 0;
		Animation anim;
		if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
			return false;
		if (mac) {
			uint16 idleAnimId = 0;
			int idleX = 0;
			int idleY = 0;
			Animation idleAnim;
			if (partnerIdleAnimParams(_mystery->_siteNumber, idleAnimId,
									  idleX, idleY) &&
				_vm->getAni().loadAnimation(idleAnimId, idleAnim) &&
				!idleAnim.empty()) {
				const uint idleFrame =
					partnerFrameAtTick(idleAnimId, (uint)idleAnim.size(), 0);
				if (idleFrame < idleAnim.size()) {
					const Picture &idle = idleAnim[idleFrame];
					anchorX = idleX;
					anchorY = idleY;
					idleBottom = idleY - (int)(int16)idle.rowoff +
								 idle.surface.h;
					alignToIdleBottom = true;
				}
			}
		}
		for (uint frameIdx = 0;
			 frameIdx < anim.size() && !_vm->shouldQuit();
			 frameIdx++) {
			const Picture &frame = anim[frameIdx];
			const int frameY = alignToIdleBottom
				? idleBottom - frame.surface.h + (int)(int16)frame.rowoff
				: anchorY;
			Graphics::ManagedSurface scratch(sw, sh,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.simpleBlitFrom(bg);
			if (mac)
				blitMacAnimFrameAnchored(scratch.surfacePtr(), frame,
										 anchorX, frameY, macPaletteMap);
			else
				blitAnimFrameAnchored(scratch.surfacePtr(), frame,
									  anchorX, frameY);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, sw, sh);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					return true;
				}
			}
			g_system->delayMillis(kFramePeriodMs);
		}
		return false;
	}

	Animation skate;
	if (_vm->getAni().loadAnimation(kSkateAni, skate) && !skate.empty()) {
		const int spriteH = skate[0].surface.h;
		const int spriteW = skate[0].surface.w;
		int x = (sw - spriteW) & ~3;            // 4-px aligned (mode-X)
		const int y = (_vm->isMacintosh() ? sh : 199) - spriteH;
		const byte transp = (byte)(skate[0].flags >> 8);
		uint frameIdx = 0;
		int distSinceTick = 0;
		const int kStep = _vm->isMacintosh() ? _vm->scaleX(4) : 4;
		const int kFrameTicks = _vm->isMacintosh() ? _vm->scaleX(0xc) : 0xc;

		while (x + spriteW > 0 && !_vm->shouldQuit()) {
			Graphics::ManagedSurface scratch(sw, sh,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.simpleBlitFrom(bg);
			if (mac)
				blitMacMaskedSurface(scratch.surfacePtr(), skate[frameIdx],
									 x, y, false, macPaletteMap);
			else
				blitFrame(scratch, skate[frameIdx], x, y, transp);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, sw, sh);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					return true; // user-skip
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

	Animation kd;
	if (_vm->getAni().loadAnimation(kKDAni, kd) && !kd.empty()) {
		for (uint frameIdx = 0;
			 frameIdx < kd.size() && !_vm->shouldQuit();
			 frameIdx++) {
			const Picture &fr = kd[frameIdx];
			const byte transp = (byte)(fr.flags >> 8);
			const int destX = -(int)(int16)fr.miscflags;
			const int destY = kKDY - (int)(int16)fr.rowoff;

			Graphics::ManagedSurface scratch(sw, sh,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.simpleBlitFrom(bg);
			if (mac)
				blitMacMaskedSurface(scratch.surfacePtr(), fr, destX, destY,
									 false, macPaletteMap);
			else
				blitFrame(scratch, fr, destX, destY, transp);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, sw, sh);
			g_system->updateScreen();

			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_KEYDOWN ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					return true;
				}
			}
			g_system->delayMillis(80);
		}
	}
	return false;
}

void SiteScreen::renderStaticDrops(uint siteNum) {
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
		uint16 picId = READ_LE_UINT16(site + dropOff + 0);
		const int16  x     = (int16)READ_LE_UINT16(site + dropOff + 2);
		const int16  y     = (int16)READ_LE_UINT16(site + dropOff + 4);
		if (_vm->isLondon() && picId == 0x140 &&
			_vm->getPartnerIndex() == kPartnerJake)
			picId = 0x141;
		if (picId == 0)
			continue;
		Picture pic;
		if (!_vm->getPics().getPicture(picId, pic))
			continue;
		blitMaskedSurface(screen, pic, x, y);
	}

	g_system->unlockScreen();
}

void SiteScreen::renderFloppyDrops(uint siteNum) {
	if (!_mystery)
		return;
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const bool mac = _vm && _vm->isMacintosh();
	const uint16 dropsOff = READ_LE_UINT16(site);
	const byte *drops = _mystery->blobAt(dropsOff);
	if (!drops)
		return;
	const uint8 count = drops[1];

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	MacSpritePaletteMap macPaletteMap = {0x00, 0xFF};
	if (mac)
		macPaletteMap = getMacSpritePaletteMap();
	for (uint i = 0; i < count; i++) {
		const byte *e = drops + 2 + i * (mac ? 6 : 5);
		const uint16 picID = READ_LE_UINT16(e + 0);
		const int16  x     = (int16)READ_LE_UINT16(e + 2);
		const int16  y     = mac ? (int16)READ_LE_UINT16(e + 4)
								  : (int16)e[4];
		if (picID == 0)
			continue;
		Picture pic;
		if (!_vm->getPics().getPicture((uint)picID, pic))
			continue;
		if (mac)
			blitMacMaskedSurface(screen, pic, x, y, false, macPaletteMap);
		else
			blitMaskedSurface(screen, pic, x, y);
	}
	g_system->unlockScreen();
}

void SiteScreen::renderAnimatedDrops(uint siteNum, uint32 tickMs) {
	if (!_mystery)
		return;

	if (_vm && _vm->isMacintosh())
		return;

	if (_vm && _vm->isFloppy()) {
		const byte *siteAnim = _mystery->floppySiteAnimData(siteNum);
		if (!siteAnim)
			return;
		const uint cycles = siteAnim[0];
		const byte *animList = siteAnim + 1 + cycles * 2;
		const uint animCount = animList[0];
		animList++;
		if (animCount == 0)
			return;

		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen)
			return;

		const uint maxAnims = MIN<uint>(animCount, 4);
		for (uint i = 0; i < maxAnims; i++) {
			const byte *e = animList + i * 4;
			const uint animId = e[0];
			const int16 x = (int16)READ_LE_UINT16(e + 1);
			const int16 y = (int16)e[3];
			Animation anim;
			if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
				continue;
			const uint frameIdx = partnerFrameAtTick((uint16)animId,
													  (uint)anim.size(),
													  tickMs);
			blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
		}

		g_system->unlockScreen();
		return;
	}

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
		blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
	}

	g_system->unlockScreen();
}

void SiteScreen::scanColorCycles(uint siteNum) {
	_colorCycles.clear();
	if (!_mystery)
		return;

	if (_vm && _vm->isMacintosh())
		return;

	if (_vm && _vm->isFloppy()) {
		const byte *siteAnim = _mystery->floppySiteAnimData(siteNum);
		if (!siteAnim)
			return;
		const uint cycles = siteAnim[0];
		for (uint i = 0; i < cycles && _colorCycles.size() < 5; i++) {
			ColorCycleRange r;
			r.start = siteAnim[1 + i * 2];
			r.end = siteAnim[1 + i * 2 + 1];
			if (r.end > r.start)
				_colorCycles.push_back(r);
		}
		return;
	}

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
	for (uint i = 0; i < _colorCycles.size(); i++) {
		cyclePaletteRange(_colorCycles[i].start, _colorCycles[i].end);
	}
	cyclePaletteRange(0xF9, 0xFE);
}

void SiteScreen::captureBgSnapshot() {
	const int sw = _vm ? _vm->screenWidth() : kScreenWidth;
	const int sh = _vm ? _vm->screenHeight() : kScreenHeight;
	_bgSnapshot.create(sw, sh, Graphics::PixelFormat::createFormatCLUT8());
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen) {
		_snapshotSite = -1;
		return;
	}
	_bgSnapshot.simpleBlitFrom(*screen);
	g_system->unlockScreen();
}

void SiteScreen::restoreBgSnapshot() {
	const int sw = _vm ? _vm->screenWidth() : kScreenWidth;
	const int sh = _vm ? _vm->screenHeight() : kScreenHeight;
	if (_bgSnapshot.w != sw || _bgSnapshot.h != sh)
		return;
	g_system->copyRectToScreen(_bgSnapshot.getPixels(), _bgSnapshot.pitch,
							   0, 0, sw, sh);
}

void SiteScreen::syncCompositedScreen() {
	const int sw = _vm ? _vm->screenWidth() : kScreenWidth;
	const int sh = _vm ? _vm->screenHeight() : kScreenHeight;
	Graphics::ManagedSurface snapshot(sw, sh,
		Graphics::PixelFormat::createFormatCLUT8());

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	snapshot.simpleBlitFrom(*screen);
	g_system->unlockScreen();

	g_system->copyRectToScreen(snapshot.getPixels(), snapshot.pitch,
							   0, 0, sw, sh);
}

bool SiteScreen::partnerIdleAnimParams(uint siteNum, uint16 &animId,
									   int &x, int &y) {
	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return false;
	const uint8 partner = _vm->getPartnerIndex();
	if (_mystery->usesCompactMacData()) {
		const uint16 spkOff = READ_LE_UINT16(site + 8);
		const byte *spk = _mystery->blobAt(spkOff);
		if (!spk)
			return false;
		const uint poseOff = partner == 0 ? 0 : 6;
		animId = READ_LE_UINT16(spk + poseOff + 0);
		x      = (int)READ_LE_UINT16(spk + poseOff + 2);
		y      = (int)READ_LE_UINT16(spk + poseOff + 4);
	} else if (_vm->isFloppy()) {
		const uint16 spkOff = READ_LE_UINT16(site + 8);
		const byte *spk = _mystery->blobAt(spkOff);
		if (!spk)
			return false;
		if (partner == 0) {
			animId = READ_LE_UINT16(spk + 0);
			x      = (int)READ_LE_UINT16(spk + 2);
			y      = (int)spk[4];
		} else {
			animId = READ_LE_UINT16(spk + 5);
			x      = (int)READ_LE_UINT16(spk + 7);
			y      = (int)spk[9];
		}
	} else {
		const uint16 speaker = READ_LE_UINT16(site + 8);
		const uint16 (*waitTable)[6] = kWaitAnims;
		if (_vm->isLondon())
			waitTable = _vm->isMacintosh()
				? kMacWaitAnimsLondon : kWaitAnimsLondon;
		if (speaker >= ARRAYSIZE(kWaitAnims))
			return false;
		animId = waitTable[speaker][0 + partner];
		x      = (int)(int16)waitTable[speaker][2 + partner];
		y      = (int)(int16)waitTable[speaker][4 + partner];
	}
	return true;
}

void SiteScreen::renderPartner(uint siteNum, uint32 tickMs) {
	uint16 animId;
	int    x;
	int    y;
	if (!partnerIdleAnimParams(siteNum, animId, x, y))
		return;

	Animation anim;
	if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
		return;

	const uint32 elapsed = (tickMs >= _waitPhaseAnchor)
							? (tickMs - _waitPhaseAnchor)
							: tickMs;
	uint frameIdx = 0;
	if (_partnerWaitMood == kPartnerWaitImpatient) {
		frameIdx = frameFromScriptAtTick(kImpatientSequence,
										 ARRAYSIZE(kImpatientSequence),
										 (uint)anim.size(), elapsed);
	} else if (_partnerWaitMood == kPartnerWaitPatient) {
		frameIdx = frameFromScriptAtTick(kPatientSequence,
										 ARRAYSIZE(kPatientSequence),
										 (uint)anim.size(), elapsed);
	} else {
		frameIdx = partnerFrameAtTick((uint16)animId,
									  (uint)anim.size(), elapsed);
	}
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	if (_vm->isMacintosh())
		blitMacAnimFrameAnchored(screen, anim[frameIdx], x, y);
	else
		blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
	g_system->unlockScreen();
}

bool SiteScreen::renderFloppyHotspotPartnerPose(uint siteNum) {
	if (!_vm || !_mystery ||
		(!_vm->isFloppy() && !_mystery->usesCompactMacData()))
		return false;

	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return false;

	const uint16 spkOff = READ_LE_UINT16(site + 8);
	const bool mac = _vm->isMacintosh();
	const uint poseOff = (_vm->getPartnerIndex() == kPartnerJake)
		? (mac ? 0x30 : 0x28) : (mac ? 0x36 : 0x2d);
	const uint poseSize = mac ? 6 : 5;
	if ((uint32)spkOff + poseOff + poseSize > _mystery->dataSize())
		return false;

	const byte *pose = _mystery->blobAt((uint32)spkOff + poseOff);
	if (!pose)
		return false;

	const uint16 animId = READ_LE_UINT16(pose + 0);
	const int x = (int)READ_LE_UINT16(pose + 2);
	const int y = mac ? (int)READ_LE_UINT16(pose + 4) : (int)pose[4];

	Animation anim;
	if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
		return false;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return false;

	const uint frameIdx = partnerFrameAtTick(animId, (uint)anim.size(), 0);
	if (mac)
		blitMacAnimFrameAnchored(screen, anim[frameIdx], x, y);
	else
		blitAnimFrameAnchored(screen, anim[frameIdx], x, y);
	g_system->unlockScreen();
	return true;
}

void SiteScreen::renderBackground(uint siteNum) {
	// Mac art uses index 0 = white / 0xFF = black, but most site ColorTables
	// store those endpoints swapped. Backgrounds are copied straight to the
	// screen, so normalize their endpoints to the active palette first (sprites
	// already do this through blitMacMaskedSurface).
	const bool mac = _vm->isMacintosh();
	const MacSpritePaletteMap macMap =
		mac ? getMacSpritePaletteMap() : MacSpritePaletteMap();

	Picture frame;
	if (_vm->getPics().loadEntry(0x3d, frame)) {
		if (mac)
			remapMacSurfaceEndpoints(frame.surface, macMap);
		g_system->copyRectToScreen(frame.surface.getPixels(),
								   frame.surface.pitch,
								   0, 0, frame.surface.w, frame.surface.h);
	}

	const byte *site = _mystery->siteData(siteNum);
	uint16 sitepic = 0;
	if (site) {
		if (_vm->isFloppy() || _mystery->usesCompactMacData()) {
			const uint16 dropsOff = READ_LE_UINT16(site);
			const byte *drops = _mystery->blobAt(dropsOff);
			if (drops)
				sitepic = (uint16)drops[0];
		} else {
			sitepic = READ_LE_UINT16(site);
		}
	}
	Picture scene;
	bool haveScene = false;
	if (sitepic < _vm->getSites().size())
		haveScene = _vm->getSites().loadEntry(sitepic, scene);
	if (!haveScene)
		haveScene = _vm->getPics().getPicture(sitepic + 1, scene);
	if (haveScene) {
		// `_Rect_Move(0, 0, h, ..., 0x42, 0x14, 48000, h, w)`.
		const int x = _vm->scaleX(0x42);
		const int y = _vm->scaleY(0x14);
		const int w = MIN<int>(scene.surface.w, _vm->screenWidth() - x);
		const int h = MIN<int>(scene.surface.h, _vm->screenHeight() - y);
		if (w > 0 && h > 0) {
			if (mac)
				remapMacSurfaceEndpoints(scene.surface, macMap);
			g_system->copyRectToScreen(scene.surface.getPixels(),
									   scene.surface.pitch, x, y, w, h);
		}
	}
}

void bumpHotspotEdgeColor(byte &color) {
	const byte next = (byte)(color + 1);
	color = (next > 0xFE) ? (byte)0xF9 : next;
}

byte currentWhitePaletteIndex(byte fallback) {
	byte palette[256 * 3];
	g_system->getPaletteManager()->grabPalette(palette, 0, 256);
	for (uint i = 0; i < 256; i++) {
		const byte *rgb = palette + i * 3;
		if (rgb[0] >= 0xFC && rgb[1] >= 0xFC && rgb[2] >= 0xFC)
			return (byte)i;
	}
	return fallback;
}

void SiteScreen::renderHotspots(uint siteNum) {
	if (ConfMan.getBool("hide_highlight_boxes"))
		return;

	const byte *spots = _mystery->hotspots(siteNum);
	const uint16 count = _mystery->hotspotCount(siteNum);
	if (!spots)
		return;

	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;

	const uint32 tickMs = g_system->getMillis();

	// CD hotspot row = 14 bytes:
	//   +0..1 x1, +2..3 y1, +4..5 x2, +6..7 y2   (rect, screen coords)
	//   +8..9   clueOffset    (u16 byte offset to ClueBlock in mystery)
	//   +0xa..b hotspotIndex  (zero-based mystery-wide seen ordinal)
	//   +0xc..d extra         (CD cursor ID for `_SwitchMouse`; shipped = 0)
	// Seen key = the +0xa ordinal (so unrelated hotspots on later sites
	// don't inherit the first site's seen state after travel/reload).
	// Floppy = 8-byte plain rect only; searched state is derived by
	// walking the dialog record list, like `_HotspotSearched_Floppy`.
	const bool compact = _vm &&
		(_vm->isFloppy() || (_mystery && _mystery->usesCompactMacData()));
	const bool floppy = _vm && _vm->isFloppy();
	const bool mac = _vm && _vm->isMacintosh();
	const uint stride = compact ? 8 : 14;
	// The floppy SITEPALS has at least one searchable site where 0xFF is
	// yellow. The CD corrected that palette data; for floppy, draw with an
	// existing white entry from the current palette instead of changing it.
	const byte searchedColor = floppy ? currentWhitePaletteIndex(0xFF) : 0xFF;
	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * stride;
		Common::Rect rect;
		if (!readHotspotRect(r, mac, rect))
			continue;
		rect = rect.findIntersectingRect(Common::Rect(screen->w, screen->h));
		if (rect.isEmpty())
			continue;
		bool seen = false;
		if (compact) {
			seen = _vm->floppyHotspotSearched(siteNum, i);
		} else {
			const uint seenKey = READ_LE_UINT16(r + 0xa);
			seen = seenKey < Mystery::kHotSpotsCap &&
				   _mystery->_hotSpotsSeen[seenKey];
		}
		if (seen) {
			screen->frameRect(rect, searchedColor);
		} else {
			byte color = (byte)(0xF9 + ((i + (tickMs / 80)) & 0x07) % 6);
			// Top edge
			for (int x = rect.left; x < rect.right; x++) {
				if (x >= 0 && x < screen->w && rect.top >= 0 && rect.top < screen->h)
					*(byte *)screen->getBasePtr(x, rect.top) = color;
				bumpHotspotEdgeColor(color);
			}
			// Right edge
			for (int y = rect.top; y < rect.bottom; y++) {
				if (rect.right - 1 >= 0 && rect.right - 1 < screen->w && y >= 0 && y < screen->h)
					*(byte *)screen->getBasePtr(rect.right - 1, y) = color;
				bumpHotspotEdgeColor(color);
			}
			// Bottom edge
			for (int x = rect.right - 1; x >= rect.left; x--) {
				if (x >= 0 && x < screen->w && rect.bottom - 1 >= 0 && rect.bottom - 1 < screen->h)
					*(byte *)screen->getBasePtr(x, rect.bottom - 1) = color;
				bumpHotspotEdgeColor(color);
			}
			// Left edge
			for (int y = rect.bottom - 1; y >= rect.top; y--) {
				if (rect.left >= 0 && rect.left < screen->w && y >= 0 && y < screen->h)
					*(byte *)screen->getBasePtr(rect.left, y) = color;
				bumpHotspotEdgeColor(color);
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

	const bool compact = _vm &&
		(_vm->isFloppy() || (_mystery && _mystery->usesCompactMacData()));
	const uint stride = compact ? 8 : 14;
	const bool mac = _vm && _vm->isMacintosh();
	for (uint i = 0; i < count; i++) {
		const byte *r = spots + i * stride;
		Common::Rect rect;
		if (readHotspotRect(r, mac, rect) && rect.contains(x, y))
			return (int)i;
	}
	return -1;
}

// CD hotspot row +0xc..d: cursor id for `_SwitchMouse` (EEM1 ships 0; EEM2
// uses 2/3 examine, etc.). Floppy rows are 8-byte rects with no cursor field.
int SiteScreen::hotspotCursorId(uint siteNum, int idx) const {
	if (idx < 0 || (_vm && (_vm->isFloppy() ||
		(_mystery && _mystery->usesCompactMacData()))))
		return 0;
	const byte *spots = _mystery->hotspots(siteNum);
	if (!spots || (uint)idx >= _mystery->hotspotCount(siteNum))
		return 0;
	return (int)READ_LE_UINT16(spots + idx * 14 + 0xc);
}

void SiteScreen::updateHotspotCursor(uint siteNum, int x, int y) {
	if (!_vm)
		return;
	const int idx = hotspotAtPoint(siteNum, x, y);
	const bool siteControl = siteControlRect(_vm, kPdaSiteRect).contains(x, y) ||
							 siteControlRect(_vm, kPdaPartnerFootMapRect).contains(x, y) ||
							 siteControlRect(_vm, kPdaPartnerHeadHintRect).contains(x, y);
	if (_vm->isLondon()) {
		// EEM2 gives some hotspots their own cursor shape; every other
		// interactive zone (the site/foot/head controls, or a hotspot with no
		// custom cursor) falls back to the red-outline highlight.
		const int cursorId = idx >= 0 ? hotspotCursorId(siteNum, idx) : 0;
		if (cursorId > 0)
			_vm->setSiteHotspotCursorId(cursorId);
		else if (idx >= 0 || siteControl)
			_vm->setInteractiveMouseCursor(true);
		else
			_vm->setSiteHotspotCursorId(0);
		return;
	}
	_vm->setHotspotMouseCursor(siteControl || idx >= 0);
}

void SiteScreen::displayClueAndAutosave(const byte *clueBlock, bool forceSave) {
	byte before[Mystery::kCluesFoundCap];
	memcpy(before, _mystery->_cluesFound, sizeof(before));

	// Idle wait-anim for `displayClue` to resume after a gesture's one-shot.
	uint16 idleId = 0;
	int idleX = 0, idleY = 0;
	const bool hasIdle =
		partnerIdleAnimParams(_mystery->_siteNumber, idleId, idleX, idleY);

	_vm->setPartnerEraseBg(&_bgSnapshot);
	_vm->setPartnerIdleAnim(hasIdle, idleId, idleX, idleY);
	_vm->displayClue(clueBlock);
	_vm->setPartnerIdleAnim(false, 0, 0, 0);
	_vm->setPartnerEraseBg(nullptr);

	bool save = forceSave;
	for (uint i = 0; !save && i < Mystery::kCluesFoundCap; i++)
		save = !before[i] && _mystery->_cluesFound[i];

	if (save) {
		const Common::Error err = _vm->saveProfile(_vm->playerName());
		if (err.getCode() != Common::kNoError)
			warning("auto-save after clue failed: %s",
					err.getDesc().c_str());
	}
}

void SiteScreen::onHotspotClicked(uint siteNum, uint hotIdx) {
	debugC(1, kDebugSite, "Site %u: hotspot %u clicked", siteNum, hotIdx);

	// Floppy: 8-byte rects only (no clue metadata @ +0xa/+8). Dialog
	// records live in a separate list @ `site_data[+6]`.
	if (_vm->isFloppy() || _mystery->usesCompactMacData()) {
		if (hotIdx < Mystery::kHotSpotsCap)
			_mystery->_hotSpotsSeen[hotIdx] = 1;
		_mystery->_searchLocationNumber = (uint16)hotIdx;

		// Snapshot `_cluesFound` before dialog → autosave on new clue.
		// Floppy side-effect path is `displayFloppyDialogRecords`
		// (clues.cpp), not `displayClue` → autosave must be duplicated.
		byte before[Mystery::kCluesFoundCap];
		memcpy(before, _mystery->_cluesFound, sizeof(before));

		restoreBgSnapshot();
		const uint32 now = g_system->getMillis();
		renderAnimatedDrops(siteNum, now);
		if (!renderFloppyHotspotPartnerPose(siteNum))
			renderPartner(siteNum, now);
		g_system->updateScreen();

		_vm->setPartnerEraseBg(&_bgSnapshot);
		_vm->displayFloppyHotspotDialog(siteNum, hotIdx);
		_vm->setPartnerEraseBg(nullptr);

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
				warning("auto-save after floppy clue failed: %s",
						err.getDesc().c_str());
		}
		return;
	}

	// `_DoSiteLoop @ 168d:03f4` (after _DisplayClue):
	//   _HotSpotsSeen[hotspot[+0xa] * 2] = _HotSpotComplete;
	// Seen key = hotspotIndex @ +0xa (mystery-wide ordinal, NOT local row).
	const byte *spots = _mystery->hotspots(siteNum);
	uint hotOrdinal = hotIdx; // fallback to array index
	if (spots) {
		hotOrdinal = READ_LE_UINT16(spots + hotIdx * 14 + 0xa);
	}
	bool newlySeen = false;
	if (hotOrdinal < Mystery::kHotSpotsCap) {
		newlySeen = _mystery->_hotSpotsSeen[hotOrdinal] == 0;
		_mystery->_hotSpotsSeen[hotOrdinal] = 1;
	}
	_mystery->_searchLocationNumber = (uint16)hotIdx;

	// Bytes 8..9 of each 14-byte hotspot rect = byte offset to ClueBlock.
	// `displayClue` handles side effects (`_AddNotebook` for ClueEntry
	// +0x30..+0x39, gallery +0x26..+0x2f, onsite +0x1c..+0x25).
	if (spots) {
		const uint16 clueOff = READ_LE_UINT16(spots + hotIdx * 14 + 8);
		debugC(2, kDebugSite, "  hotspot %u -> clue offset 0x%04x",
			   hotIdx, clueOff);
		const byte *clueBlock = _mystery->blobAt(clueOff);
		if (clueBlock)
			displayClueAndAutosave(clueBlock, /* forceSave= */ newlySeen);
	}
}
// `_DoKDAnim(num) @ 168d:028a` + `_PlayAnimation @ 172b:1f46`:
//   _SuspendAnimation(WaitHandle);
//   anim = kKdAnimTable[num].anim[partner]   (@ 29be:0228)
//   x    = kKdAnimTable[num].x[partner]
//   y    = kKdAnimTable[num].y[partner]
//   _PlayAnimation(anim, x, y, WaitHandle)
//     -> registers a state-4 (one-shot) animation slot and lets
//        `_UpdateAnimations` walk the script until 0x80, then
//        frees the slot and re-activates `WaitHandle`.
bool EEMEngine::loadKdAnim(uint16 num, Animation &anim, int &px, int &py,
						   uint16 &animId) {
	if (num >= ARRAYSIZE(kKdAnimTable))
		return false;

	const uint16 (*kdTable)[6] = kKdAnimTable;
	if (isLondon()) {
		// EEM2 Mac FUN_0000ce6e indexes `_WaitAnims + 1`
		// (`lea (-0x2034,A5)`), so KD gestures share the Mac idle anchors
		// for speaker rows 1..6.
		kdTable = isMacintosh()
			? kMacWaitAnimsLondon + 1 : kKdAnimTableLondon;
	}
	const uint partner = (_partner == kPartnerJake) ? 0 : 1;
	animId = kdTable[num][partner];
	px     = (int)kdTable[num][2 + partner];
	py     = (int)kdTable[num][4 + partner];

	if (!_aniArchive.loadAnimation(animId, anim) || anim.empty()) {
		warning("loadKdAnim(%u): anim %u failed to load", num, animId);
		return false;
	}
	return true;
}
} // End of namespace EEM
