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

#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/mystery.h"
#include "eem/site.h"

namespace EEM {

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

	// Capture whether this is the first time the player enters this
	// site BEFORE we mark it visited — `_DoSiteLoop @ 168d:03f4`
	// uses the same check to decide whether to play the arrival
	// dialog: `if (_VisitedSite[_SiteNumber] == 0) _DisplayClue(...)`.
	const bool firstVisit = (siteNum < Mystery::kVisitedSiteCap)
							 && (_mystery->_visitedSite[siteNum] == 0);

	_mystery->_siteNumber = siteNum;
	debugC(1, kDebugSite, "Entering site %u (%u hotspots)",
		   siteNum, _mystery->hotspotCount(siteNum));

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
				if (clueBlock)
					_vm->displayClue(clueBlock);
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
					if (_vm->areYouSure())
						return;
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

	auto blitFrame = [](Graphics::ManagedSurface &dst, const Picture &p,
						int x, int y, byte transp) {
		const int w = p.surface.w, h = p.surface.h;
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
	};

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
	// Each frame waits one `_CheckFrameRate` tick — we use 80 ms which
	// matches the original's ~12 FPS pacing.
	Animation kd;
	if (_vm->getAni().loadAnimation(kKDAni, kd) && !kd.empty()) {
		for (uint frameIdx = 0;
			 frameIdx < kd.size() && !_vm->shouldQuit();
			 frameIdx++) {
			const Picture &fr = kd[frameIdx];
			const byte transp = (byte)(fr.flags >> 8);
			const int destX = -(int)fr.miscflags;
			const int destY = kKDY - (int)fr.rowoff;

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

// Mask-aware blit from a Picture into a Graphics::Surface.
static void blitMaskedSurface(Graphics::Surface *screen,
							  const Picture &p, int x, int y) {
	if (!screen)
		return;
	const byte transp = (byte)(p.flags >> 8);
	for (int row = 0; row < p.surface.h; row++) {
		const int dstY = y + row;
		if (dstY < 0 || dstY >= screen->h) continue;
		const byte *src = (const byte *)p.surface.getBasePtr(0, row);
		byte *dst = (byte *)screen->getBasePtr(0, dstY);
		for (int col = 0; col < p.surface.w; col++) {
			const int dstX = x + col;
			if (dstX < 0 || dstX >= screen->w) continue;
			if (src[col] != transp)
				dst[dstX] = src[col];
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

	const uint32 kFramePeriodMs = 100; // ~10 FPS, in line with `_CheckFrameRate`.

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
		const uint frameIdx = (uint)((tickMs / kFramePeriodMs) % anim.size());
		blitMaskedSurface(screen, anim[frameIdx], x, y);
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
	auto rotate = [&](uint8 start, uint8 end) {
		if (end <= start) return;
		const uint count = (uint)end - (uint)start + 1;
		byte buf[256 * 3];
		g_system->getPaletteManager()->grabPalette(buf, start, count);
		// Save first triplet, shift, restore at end.
		byte savedR = buf[0], savedG = buf[1], savedB = buf[2];
		for (uint i = 0; i + 1 < count; i++) {
			buf[i * 3 + 0] = buf[(i + 1) * 3 + 0];
			buf[i * 3 + 1] = buf[(i + 1) * 3 + 1];
			buf[i * 3 + 2] = buf[(i + 1) * 3 + 2];
		}
		buf[(count - 1) * 3 + 0] = savedR;
		buf[(count - 1) * 3 + 1] = savedG;
		buf[(count - 1) * 3 + 2] = savedB;
		g_system->getPaletteManager()->setPalette(buf, start, count);
	};
	for (uint i = 0; i < _colorCycles.size(); i++) {
		rotate(_colorCycles[i].start, _colorCycles[i].end);
	}
	// Hotspot marching ants — always cycled.
	rotate(0xF9, 0xFE);
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
	// Verbatim copy of the bytes Ghidra dumped at 29be:021c so we
	// don't need to ship the original data segment.
	static const uint16 kWaitAnims[][6] = {
		{ 0x00, 0x0a, 0x06, 0x06, 0x50, 0x50 }, // 0
		{ 0x03, 0x0c, 0x06, 0x06, 0x50, 0x50 }, // 1
		{ 0x01, 0x0b, 0x06, 0x06, 0x50, 0x50 }, // 2
		{ 0x04, 0x0d, 0x06, 0x06, 0x50, 0x50 }, // 3
		{ 0x02, 0x10, 0x06, 0x06, 0x50, 0x50 }, // 4
		{ 0x05, 0x05, 0x06, 0x06, 0x50, 0x50 }, // 5
		{ 0x06, 0x06, 0x06, 0x06, 0x50, 0x50 }, // 6
		{ 0x00, 0x00, 0x23, 0x6f, 0x38, 0x88 }, // 7 — special pos
		{ 0x07, 0xb1, 0x39, 0xc8, 0x88, 0xae }, // 8 — likely junk; 0xb1 anim id is suspect
		{ 0x9d, 0xbe, 0xa3, 0xae, 0xb8, 0xbe }  // 9 — likely junk
	};

	const byte *site = _mystery->siteData(siteNum);
	if (!site)
		return;
	const uint16 speaker = READ_LE_UINT16(site + 8);
	if (speaker >= ARRAYSIZE(kWaitAnims))
		return;

	const uint8 partner = _vm->getPartnerIndex();
	const uint  animId  = kWaitAnims[speaker][0 + partner];
	const int   x       = (int)(int16)kWaitAnims[speaker][2 + partner];
	const int   y       = (int)(int16)kWaitAnims[speaker][4 + partner];

	Animation anim;
	if (!_vm->getAni().loadAnimation(animId, anim) || anim.empty())
		return;

	// `_UpdateAnimations @ 172b:09c1` advances the partner's frame
	// every `_CheckFrameRate` tick. We pick the frame from a global
	// 100 ms clock so the partner cycles in sync with the animated
	// drops.
	const uint32 kFramePeriodMs = 100;
	const uint frameIdx = (uint)((tickMs / kFramePeriodMs) % anim.size());
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	blitMaskedSurface(screen, anim[frameIdx], x, y);
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
		if (clueBlock)
			_vm->displayClue(clueBlock);
	}
	// Caller (`SiteScreen::run`) re-renders the site after this returns.
}

} // End of namespace EEM
