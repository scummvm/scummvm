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
#include "common/file.h"
#include "common/path.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/site.h"

// EEM — clue / briefing pipeline (SCRIPT.C clue parts + KD.C briefing parts).
// Everything that drives a `ClueBlock` through the displayClue / portrait /
// balloon / notebook-side-effect flow plus the case-briefing intro that
// preambles into the same flow.

namespace EEM {

// Picture / animation IDs verified against `_DoChoosePartner @ 1a35:0756`.
// `const` at namespace scope already implies internal linkage in C++,
// so no `static` needed.
const uint kPicChooseBackground = 0x8c; ///< `_GetBackground(0x8c)`
const uint kAniBoy  = 8;                 ///< `_GetAnimation(8)` (Jake)
const uint kAniGirl = 9;                 ///< `_GetAnimation(9)` (Jenny)

// `_DoHappiness @ 172b:27b5`: cursor X picks one of 4 rects; past
// rect 3 is treated as level 4. Verbatim from `29be:030f`.
const Common::Rect kHappyZones[4] = {
	Common::Rect(  0, 0,  70, 200), // far left  — girl very happy, boy neutral
	Common::Rect( 70, 0, 126, 200), // girl's column
	Common::Rect(126, 0, 182, 200), // middle
	Common::Rect(182, 0, 235, 200), // boy's column
};

// On-screen positions verified from `_NewAnimation` calls @ 1a35:07b9 / 07d5.
const int kBoyX  = 0xe2; // 226
const int kBoyY  = 0x62; // 98
const int kGirlX = 0x42; // 66
const int kGirlY = 0x60; // 96

// `_DoHappiness @ 172b:27b5`: each cursor zone swaps the partner's
// sequence script to a more / less "happy" cycle. Boy seqs lifted
// verbatim from `29be:0337` (5 × 0x14 bytes), girl seqs from
// `29be:039b`. Both cycle through 9 frames (the boy/girl anim cells
// contain 10 cells = pairs of "neutral, smile" at increasing intensity).
const uint8 kBoySeqs[5][9] = {
	{ 0,0,0,0,0,0,0,1,0 }, // level 0
	{ 2,2,2,2,2,2,2,3,2 }, // level 1
	{ 4,4,4,4,4,4,4,5,4 }, // level 2
	{ 6,6,6,6,6,6,7,6,6 }, // level 3
	{ 8,8,8,8,8,8,8,8,9 }, // level 4 (cursor past zone 3)
};
const uint8 kGirlSeqs[5][9] = {
	{ 8,9,8,8,8,8,8,8,8 },
	{ 6,6,6,7,6,6,6,6,6 },
	{ 4,4,5,4,4,4,4,4,4 },
	{ 2,2,2,2,2,2,3,2,2 },
	{ 0,0,0,0,0,1,0,0,0 },
};

uint happinessLevel(int x) {
	for (uint i = 0; i < ARRAYSIZE(kHappyZones); i++) {
		if (kHappyZones[i].contains(x, 100))
			return i;
	}
	return 4; // past zone 3 → max level
}

// Lock the framebuffer, masked-blit `p` at (x, y), unlock. The transparent
// colour is the high byte of `p.flags` per `_Rect_Move_Mask @ 1000:03fc`.
void blitMaskedToScreen(const Picture &p, int x, int y) {
	const byte transp = (byte)(p.flags >> 8);
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
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
	g_system->unlockScreen();
}

void EEMEngine::doChoosePartner() {
	// Mirrors _DoChoosePartner @ 1a35:0756. The original places boy + girl
	// animations on a backdrop and polls four click rectangles (two per
	// character) for the player's choice. We approximate by splitting the
	// screen at x=160: left half = girl (Jenny), right half = boy (Jake).
	Picture background;
	if (!_picsArchive.getPicture(kPicChooseBackground, background)) {
		warning("ChoosePartner background (%u) load failed", kPicChooseBackground);
		return;
	}

	Animation boyAnim;
	if (!_aniArchive.loadAnimation(kAniBoy, boyAnim) || boyAnim.empty()) {
		warning("Boy animation (%u) load failed", kAniBoy);
		return;
	}
	Animation girlAnim;
	if (!_aniArchive.loadAnimation(kAniGirl, girlAnim) || girlAnim.empty()) {
		warning("Girl animation (%u) load failed", kAniGirl);
		return;
	}

	setAnmPalette(Common::Path("TITLE.ANM"));

	// `_DoHappiness @ 172b:27b5`: the cursor's X column picks one of 4
	// rects (29be:030f, all full-height); past rect 3 → "level 4". The
	// per-zone sequence scripts (`kBoySeqs` / `kGirlSeqs`) live at file
	// scope above so the gestures match the original beat-for-beat.

	// `_DoChoosePartner` opens with `_SetMousePos(0xa0, 0x96)` so the
	// cursor lands centred between the two partners — start the
	// happiness level from that initial X.
	int curMouseX = 0xa0;
	uint level = happinessLevel(curMouseX);
	uint seqIdx = 0;       // step within the 9-frame seq

	// Initial render — pose 0 of whichever zone the cursor opens in.
	blitAt(background, 0, 0);
	blitAt(girlAnim[kGirlSeqs[level][seqIdx % 9] % girlAnim.size()],
		   kGirlX, kGirlY);
	blitAt(boyAnim [kBoySeqs [level][seqIdx % 9] % boyAnim.size()],
		   kBoyX, kBoyY);
	g_system->updateScreen();

	debugC(1, kDebugGeneral, "ChoosePartner: %u boy frames at (%d,%d), "
		   "%u girl frames at (%d,%d)",
		   (uint)boyAnim.size(), kBoyX, kBoyY,
		   (uint)girlAnim.size(), kGirlX, kGirlY);

	uint32 lastTick = g_system->getMillis();
	while (!shouldQuit()) {
		// Advance through the 9-frame seq at 100 ms — `_CheckFrameRate`
		// cadence. The seq is short and loops; matches the original
		// `_UpdateAnimations` which restarts at curIdx=0 on the 0x80
		// marker. Mirrors `_DoHappiness`'s rewriting of `curIdx = 0xFFFF`
		// when the cursor crosses zones (we restart `seqIdx` instead).
		if (g_system->getMillis() - lastTick > 100) {
			lastTick = g_system->getMillis();
			seqIdx = (seqIdx + 1) % 9;
			blitAt(background, 0, 0);
			blitAt(girlAnim[kGirlSeqs[level][seqIdx % 9] % girlAnim.size()],
				   kGirlX, kGirlY);
			blitAt(boyAnim [kBoySeqs [level][seqIdx % 9] % boyAnim.size()],
				   kBoyX, kBoyY);
			g_system->updateScreen();
		}

		Common::Event ev;
		bool done = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				done = true;
				break;
			}
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				curMouseX = ev.mouse.x;
				const uint newLevel = happinessLevel(curMouseX);
				if (newLevel != level) {
					level = newLevel;
					seqIdx = 0; // restart cycle so the gesture pops
					blitAt(background, 0, 0);
					blitAt(girlAnim[kGirlSeqs[level][seqIdx % 9] % girlAnim.size()],
						   kGirlX, kGirlY);
					blitAt(boyAnim [kBoySeqs [level][seqIdx % 9] % boyAnim.size()],
						   kBoyX, kBoyY);
					g_system->updateScreen();
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				_partner = (ev.mouse.x >= 160) ? 0 : 1;
				debugC(1, kDebugGeneral, "Partner picked: %s",
					   _partner == 0 ? "Jake" : "Jennifer");
				done = true;
				break;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					_partner = 1; done = true; break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					_partner = 0; done = true; break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_RETURN ||
					ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					done = true; break;
				}
			}
		}
		if (done)
			break;
		g_system->updateScreen();
		g_system->delayMillis(20);
	}

	// Mirrors the tail of `_DoChoosePartner @ 1a35:097f` — once the
	// player commits to a partner, load and play their intro VOC
	// (`jen.voc` for Jenny, `jake.voc` for Jake; strings at 29be:0af1 /
	// 29be:0af9) and block on `_WaitForVoiceDone`.
	if (_audio) {
		_audio->playVoc(Common::Path(_partner == 0 ? "JAKE.VOC" : "JEN.VOC"));
		_audio->waitForVoiceDone();
	}
}

void EEMEngine::doInitClues() {
	// Mirrors `_DoInitClues` @ 1a35:0411. The original does:
	//   1. _AllBlack(); _GetBackground(0x52); _GetPalette(0x22);
	//   2. _GetAnimation(gameAni); _NewAnimation(0xcd, 0x6c, ...)
	//      _GetAnimation(bookAni); _NewAnimation(0,    99,   ...)
	//      (case type 1 also: _NewAnimation(0x68, 0x8b, nancyAni))
	//   3. _UpdateAnimations(); _FadeIn();
	//   4. while (frame != gameNum) { _CheckFrameRate(); _UpdateAnimations(); }
	//        — cycles through the entire game animation once. Click skips.
	//   5. _PlayInSequence(seqId, ...) — plays a follow-up sequence based
	//      on partner + case type.
	//   6. _DisplayClue(InitBlock + 2, 1) — the briefing dialogue.
	//   7. _OnSites[startSite] = 1.
	//
	// gameAni / bookAni / nancyAni values verified directly from Ghidra:
	//   gameAni  = 0x17 (Jake) / 0x3b (Jenny)
	//   bookAni  = 0x18 (Jake) / 0x3c (Jenny)
	//   nancyAni = 0x19 (case type 1 only)
	if (!_mystery.isLoaded())
		return;

	const byte *ib = _mystery.initBlock();
	if (!ib)
		return;

	const uint16 startSite = READ_LE_UINT16(ib + 2);
	if (startSite < Mystery::kVisitedSiteCap)
		_mystery._onSites[startSite] = 1;
	_mystery._siteNumber = startSite;
	_mystery._lastSite = startSite;

	setSitePalette(0x22);
	Picture bg;
	if (_picsArchive.getPicture(0x52, bg))
		blitAt(bg, 0, 0);

	const uint gameAni = _partner == 0 ? 0x17 : 0x3b;
	const uint bookAni = _partner == 0 ? 0x18 : 0x3c;
	Animation game, book, nancy;
	const bool haveGame  = _aniArchive.loadAnimation(gameAni, game) && !game.empty();
	const bool haveBook  = _aniArchive.loadAnimation(bookAni, book) && !book.empty();

	const uint16 caseType = READ_LE_UINT16(ib);
	const bool haveNancy = (caseType == 1)
						  && _aniArchive.loadAnimation(0x19, nancy)
						  && !nancy.empty();

	// Step 4 — cycle through the game animation once before the briefing.
	// Mirrors the `while (uVar9 != gameNum)` loop. The original calls
	// `_UpdateAnimations` per `_CheckFrameRate` tick (~10 fps). We use
	// 100 ms ticks for the same cadence. Click / key skips.
	if (haveGame || haveBook || haveNancy) {
		const uint frameCount = haveGame ? game.size() : 8;
		bool skip = false;
		for (uint frame = 0; frame < frameCount && !shouldQuit() && !skip; frame++) {
			// Restore BG + advance frame. The original always uses
			// Jake's anim IDs (0x17/0x18/0x19) as the SCRIPT keys
			// even when Jenny's CELL data is loaded — verified at
			// `_DoInitClues @ 1a35:0507`/`0541` where
			// `_NewAnimation(..., (PicData *)CONCAT22(0x17, ...), ...)`
			// hard-codes the script index to 0x17. So we look up
			// `partnerFrameAtTick(0x17, ...)` regardless of partner.
			// This gives us the correct cadence — book holds on its
			// "thinking" pose (cell 8) for 16 ticks instead of
			// flipbook-cycling, and nancy waits 18 ticks before her
			// late-arrival count-up.
			if (_picsArchive.getPicture(0x52, bg))
				blitAt(bg, 0, 0);
			const uint32 t = frame * 100;
			// All three briefing anims (game/book/nancy) go through
			// the original `_NewAnimation` path so per-frame anchors
			// apply. Use `blitAnimFrameAnchored` against a locked
			// screen surface so the briefing partner / book / nancy
			// translate cleanly between cells instead of pinning at
			// the same top-left.
			Graphics::Surface *scr = g_system->lockScreen();
			if (!scr) {
				skip = true;
				break;
			}
			if (haveGame) {
				const uint f = partnerFrameAtTick(0x17, (uint)game.size(), t);
				blitAnimFrameAnchored(scr, game[f], 0xcd, 0x6c);
			}
			if (haveBook) {
				const uint f = partnerFrameAtTick(0x18, (uint)book.size(), t);
				blitAnimFrameAnchored(scr, book[f], 0, 99);
			}
			if (haveNancy) {
				const uint f = partnerFrameAtTick(0x19, (uint)nancy.size(), t);
				blitAnimFrameAnchored(scr, nancy[f], 0x68, 0x8b);
			}
			g_system->unlockScreen();
			g_system->updateScreen();

			// Wait 100 ms or until input.
			const uint32 wakeup = g_system->getMillis() + 100;
			while (g_system->getMillis() < wakeup && !shouldQuit() && !skip) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_LBUTTONDOWN ||
						ev.type == Common::EVENT_KEYDOWN) {
						skip = true;
						break;
					}
				}
				g_system->delayMillis(10);
			}
		}
	}

	// Composite the final frames (or first frames if skipped) so the BG
	// is in a sensible state when displayClue overlays the speaker.
	if (_picsArchive.getPicture(0x52, bg))
		blitAt(bg, 0, 0);
	if (haveGame)
		blitMaskedToScreen(game[0], 0xcd, 0x6c);
	if (haveBook)
		blitMaskedToScreen(book[0], 0, 99);
	if (haveNancy)
		blitMaskedToScreen(nancy[0], 0x68, 0x8b);
	g_system->updateScreen();

	// Step 5 — `_PlayInSequence(animSeq, 0xcd, animY)` per Ghidra:
	//   Jake (partner=0):
	//     caseType=1 → anim 0x38 at (0xcd, 0x6d)
	//     caseType=2 → anim 0x37 at (0xcd, 0x6c)
	//     caseType=3 → anim 0x39 at (0xcd, 0x6c)
	//   Jenny (partner=1):
	//     caseType=2 → anim 0x3a at (0xcd, 0x6c)
	//     caseType=3 → anim 0x3d at (0xcd, 0x6c)
	// `_PlayInSequence @ 172b:2d03` plays each frame at (sx-w, sy-rowoff)
	// with mask blit, advancing one frame per `_CheckFrameRate` tick.
	uint16 seqAni = 0xFFFF;
	uint16 seqY   = 0x6c;
	if (_partner == 0) {
		switch (caseType) {
		case 1:
			seqAni = 0x38;
			seqY = 0x6d;
			break;
		case 2:
			seqAni = 0x37;
			seqY = 0x6c;
			break;
		case 3:
			seqAni = 0x39;
			seqY = 0x6c;
			break;
		default:
			break;
		}
	} else {
		switch (caseType) {
		case 2:
			seqAni = 0x3a;
			seqY = 0x6c;
			break;
		case 3:
			seqAni = 0x3d;
			seqY = 0x6c;
			break;
		default:
			break;
		}
	}
	if (seqAni != 0xFFFF) {
		Animation seq;
		if (_aniArchive.loadAnimation(seqAni, seq) && !seq.empty()) {
			bool skip = false;
			for (uint frame = 0; frame < seq.size() && !shouldQuit() && !skip;
				 frame++) {
				const Picture &fr = seq[frame];
				// Restore BG + base anim frames so each new frame
				// composites cleanly.
				if (_picsArchive.getPicture(0x52, bg))
					blitAt(bg, 0, 0);
				if (haveGame)
					blitMaskedToScreen(game[frame % game.size()], 0xcd, 0x6c);
				if (haveBook)
					blitMaskedToScreen(book[frame % book.size()], 0, 99);
				if (haveNancy)
					blitMaskedToScreen(nancy[frame % nancy.size()], 0x68, 0x8b);
				// Anchor: original blits at `(sx - frame.width,
				// sy - frame.rowoff)`. `frame.rowoff` is the y-anchor
				// in our PicData. We use width/height directly since
				// loadAnimation places anchor at (0, 0).
				const int dstX = (int)0xcd - (int)fr.surface.w;
				const int dstY = (int)seqY - (int)fr.rowoff;
				blitMaskedToScreen(fr, dstX, dstY);
				g_system->updateScreen();
				const uint32 wakeup = g_system->getMillis() + 100;
				while (g_system->getMillis() < wakeup &&
					   !shouldQuit() && !skip) {
					Common::Event ev;
					while (g_system->getEventManager()->pollEvent(ev)) {
						if (ev.type == Common::EVENT_LBUTTONDOWN ||
							ev.type == Common::EVENT_KEYDOWN) {
							skip = true;
							break;
						}
					}
					g_system->delayMillis(10);
				}
			}
		}
	}

	// `_DoInitClues` plays `phone.voc` (29be:0acc) ONLY when caseType == 2
	// (the "incoming call" briefing variant). Verified at 1a35:05a2 —
	// the gate is `iVar1 == 2 && _VoiceAvailable`. Other case types open
	// straight into the briefing dialogue without it.
	if (caseType == 2 && _audio) {
		_audio->playVoc(Common::Path("PHONE.VOC"));
		_audio->waitForVoiceDone();
	}

	// Step 6 — case briefing dialogue.
	displayClue(ib + 4);
}

/// Mirror `_ParseString` @ 1b66:07c3 — substitute the control bytes that
/// the original engine uses as placeholders. Only the two we encounter most
/// often (player name = 0x80, partner first name = 0x82) are substituted;
/// other 0x8N opcodes are stripped. The original engine also handles
/// hyphenation marks and a hint placeholder (0x89) we ignore for now.
Common::String EEMEngine::parseString(const Common::String &raw,
									  const Common::String &playerName,
									  uint partner) const {
	// Substitution opcodes from `_ParseString` @ 1b66:07c3, jump-table
	// at 1b66:0cbe. Each handler reads `_Partner` (16-bit at 0x7918)
	// and indexes the name table at 29be:0c28 ({Jake, Jennifer, he,
	// she, him, her, his} as far pointers).
	//   0x80 — player's typed name (auto-cap word starts) — uses _PlayerRecord
	//   0x81 — _Partner == 0 ? "Jake"     : "Jennifer"  (chosen detective)
	//   0x82 — _Partner == 0 ? "Jennifer" : "Jake"      (the OTHER one)
	//   0x83 — _Partner == 0 ? "he"       : "she"
	//   0x84 — _Partner == 0 ? "him"      : "her"
	//   0x85 — _Partner == 0 ? "his"      : "her"
	//   0x86..0x88 read a different gender flag at 0x7985 — left alone
	//     until that flag's source is traced.
	//   0x89 — KD hint placeholder (handled by caller).
	const bool isJake = (partner == 0);
	Common::String out;
	for (uint i = 0; i < raw.size(); i++) {
		const byte c = (byte)raw[i];
		switch (c) {
		case 0x80:
			out += playerName;
			break;
		case 0x81:
			out += isJake ? "Jake" : "Jennifer";
			break;
		case 0x82:
			out += isJake ? "Jennifer" : "Jake";
			break;
		case 0x83:
			out += isJake ? "he" : "she";
			break;
		case 0x84:
			out += isJake ? "him" : "her";
			break;
		case 0x85:
			out += isJake ? "his" : "her";
			break;
		case 0x86:
		case 0x87:
		case 0x88:
		case 0x89:
			// Eaten silently — see comment above.
			break;
		case 0:
			return out;
		case '\r':
			break;
		default:
			out += (char)c;
			break;
		}
	}
	return out;
}

void EEMEngine::applyClueSideEffects(const byte *c) {
	for (uint j = 0; j < 5; j++) {
		const uint16 note = READ_LE_UINT16(c + 0x30 + j * 2);
		if (note != 0xFFFF && note < Mystery::kCluesFoundCap)
			_mystery._cluesFound[note] = 1;

		const uint16 galIdx = READ_LE_UINT16(c + 0x26 + j * 2);
		if (galIdx != 0xFFFF && galIdx < Mystery::kGalleryCap) {
			const uint8 phys = _mystery._newOrder[galIdx];
			if (phys < Mystery::kGalleryCap)
				_mystery._inGallery[phys] = 1;
		}

		const uint16 siteIdx = READ_LE_UINT16(c + 0x1c + j * 2);
		if (siteIdx != 0xFFFF) {
			const uint16 siteVal = siteIdx & 0x7FFF;
			if (siteVal < Mystery::kVisitedSiteCap)
				_mystery._onSites[siteVal] = 1;
			if (siteIdx & 0x8000)
				_mystery._sawCONSITEs = true;
		}
	}
}

void EEMEngine::displayClue(const byte *clueBlock) {
	if (!clueBlock || !_mystery.isLoaded())
		return;

	// ClueBlock layout (verified against M0.BIN):
	//   +0..1: number (entry count)
	//   +2..3: pic ID for entry 0 (entry N>0 uses prev entry's last 2 bytes)
	//   +4..:  array of 62-byte entries
	const uint16 number = READ_LE_UINT16(clueBlock);
	debugC(1, kDebugScript, "displayClue: %u entries", number);
	if (number == 0 || number > 32) {
		// number==0 = no briefing (e.g. mystery 0 case-type 4); >32 is a
		// guard against bad pointers.
		return;
	}

	// Snapshot the current screen as the BG so character pics from
	// earlier entries don't stack on top of each other.
	Graphics::ManagedSurface bg(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	bg.clear();
	{
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			for (int row = 0; row < 200; row++) {
				memcpy((byte *)bg.getBasePtr(0, row),
					   (const byte *)screen->getBasePtr(0, row), 320);
			}
			g_system->unlockScreen();
		}
	}

	for (uint i = 0; i < number && !shouldQuit(); i++) {
		// Restore BG before drawing this entry's portrait + balloon.
		g_system->copyRectToScreen(bg.getPixels(), bg.pitch, 0, 0, 320, 200);
		const byte *c = clueBlock + 4 + i * 62;
		// Per-partner fields:
		//   +0..1, +2..3: tx, ty (partner 0)
		//   +4..5, +6..7: tx, ty (partner 1)
		//   +8..9, +10..11: bubText offset for partner 0/1 (rel. TextBlock)
		//   +12..13, +14..15: balloon picture ID for partner 0/1
		//   +16..17, +18..19: bubX, bubY
		//   +0x3a..+0x3b:    KD-anim number (-1 = none)
		// Per `_DisplayClue` @ 2404:05e6: partner 1 uses its own field
		// set ONLY when bubText1 is not -1; otherwise it falls back to
		// the partner 0 fields entirely. Partner 0 always uses field 0.

		// Per-clue partner reaction animation. `_DisplayClue` @
		// 2404:0635-064b checks `clueEntry[+0x3a]` and, when not -1,
		// calls `_DoKDAnim(num)` BEFORE drawing the speaker portrait.
		// This is what surfaces "Jenny takes a picture with a camera"
		// (and the matching Jake gestures) during NPC searches.
		const int16 kdAnimNum = (int16)READ_LE_UINT16(c + 0x3a);
		if (kdAnimNum != -1)
			playKdAnim((uint16)kdAnimNum);

		const bool useP1 = (_partner == 1) &&
			(READ_LE_UINT16(c + 10) != 0xFFFF);
		const uint partner = useP1 ? 1 : 0;
		const uint16 textOff = READ_LE_UINT16(c + 8 + partner * 2);
		const bool hasText = (textOff != 0xFFFF);
		// Partner 1 bubX/bubY at +0x14/+0x16; partner 0 at +0x10/+0x12.
		const uint16 bubX = READ_LE_UINT16(c + (useP1 ? 0x14 : 0x10));
		const uint16 bubY = READ_LE_UINT16(c + (useP1 ? 0x16 : 0x12));
		const uint16 bubNum = READ_LE_UINT16(c + (useP1 ? 0x0E : 0x0C));
		const char *raw   = hasText ? _mystery.textAt(textOff) : "";

		// Speaker portrait. Mirrors `_DisplayClue`'s `pic[clues+i*62-2]`:
		// for entry 0 the pic ID is in the ClueBlock header at +2; for
		// later entries it sits in the previous entry's last 2 bytes.
		// Speaker portrait position uses partner 0 fields (+0..+3) when
		// _partner==0 or when partner 1 falls back; otherwise partner 1
		// fields (+4..+7). Same logic as the original.
		const uint16 charX  = READ_LE_UINT16(c + (useP1 ? 4 : 0));
		const uint16 charY  = READ_LE_UINT16(c + (useP1 ? 6 : 2));
		const uint16 charPicId = (i == 0)
			? READ_LE_UINT16(clueBlock + 2)
			: READ_LE_UINT16(c - 2);
		if (charPicId != 0 && charPicId != 0xFFFF) {
			Picture charPic;
			if (_picsArchive.getPicture(charPicId, charPic) &&
				charX < 320 && charY < 200) {
				const int w = MIN<int>(charPic.surface.w, 320 - charX);
				const int h = MIN<int>(charPic.surface.h, 200 - charY);
				if (w > 0 && h > 0)
					g_system->copyRectToScreen(charPic.surface.getPixels(),
						charPic.surface.pitch, charX, charY, w, h);
			}
		}

		// Substitute control bytes (0x80..0x89) — see `parseString` for
		// the table. 0x81 = chosen detective, 0x82 = the other one.
		const Common::String text = parseString(raw ? raw : "",
												_playerName, _partner);

		// Speech balloon. Mirrors `_GetBalloon` + `_AddPicBackground` in
		// `_DisplayClue`. The original looks up per-balloon text-area
		// metadata in a table at offset 0x875 (within `_DisplayClue`'s
		// segment); we don't have that table decoded yet, so we use a
		// fixed inset of 8 px from the balloon's top-left.
		Picture balloon;
		const uint16 balloonId = bubNum & 0x7F;
		const bool haveBalloon = bubNum != 0xFFFF &&
			_balloonArchive.size() > balloonId &&
			_balloonArchive.loadEntry(balloonId, balloon);

		if (_font.isLoaded() && !text.empty()) {
			// Snapshot the current screen, overlay balloon + text, then
			// copy the changed band back. This preserves the site BG
			// underneath unchanged regions.
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen)
				break;
			Graphics::ManagedSurface scratch(320, 200,
				Graphics::PixelFormat::createFormatCLUT8());
			for (int row = 0; row < 200; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)screen->getBasePtr(0, row), 320);
			}
			g_system->unlockScreen();

			int textX = bubX;
			int textY = bubY;
			int textW = MIN<int>(320 - bubX, 200);
			int copyY = bubY;
			int copyH = _font.getFontHeight() * 4 + 8;

			if (haveBalloon) {
				const int bw = MIN<int>(balloon.surface.w, 320 - bubX);
				const int bh = MIN<int>(balloon.surface.h, 200 - bubY);
				// `_AddPicBackground` passes `pic->miscflags >> 8` as
				// the transparent colour to `_Rect_Move_Mask`. The
				// on-disk u16 at file offset 0 maps to `Picture::flags`.
				const byte transp = (byte)(balloon.flags >> 8);
				// `_GetBalloon @ 172b:1d7d` mirrors the picture horizontally
				// when `(bubNum & 0x80)` is set — used for right-side
				// speakers so the tail points the other way.
				const bool flipBalloon = (bubNum & 0x80) != 0;
				if (bw > 0 && bh > 0) {
					for (int row = 0; row < bh; row++) {
						const byte *src =
							(const byte *)balloon.surface.getBasePtr(0, row);
						byte *dst = (byte *)scratch.getBasePtr(bubX, bubY + row);
						for (int col = 0; col < bw; col++) {
							const int srcCol = flipBalloon
								? (balloon.surface.w - 1 - col)
								: col;
							const byte px = src[srcCol];
							if (px != transp)
								dst[col] = px;
						}
					}
				}
				// Per-balloon metadata from `29be:0875` (52 × 10 bytes,
				// indexed by `bubNum & 0x7F`). The original `_DisplayClue`
				// does `_WordWrap(bubX + table[bub].x, bubY + table[bub].y,
				// table[bub].w, ...)`. `getBalloonInsets` is the shared
				// accessor (defined in `graphics.cpp`); fall back to the
				// (5, 4, 155) entry-23 inset if the lookup fails.
				uint16 bx = 5;
				uint16 by = 4;
				uint16 bw_ = 155;
				getBalloonInsets(balloonId, bx, by, bw_);
				textX = bubX + bx;
				textY = bubY + by;
				textW = bw_;
				copyH = bh;
			} else {
				// No balloon — clear a band so old pixels don't bleed.
				const Common::Rect band(0, bubY, 320,
					MIN<int>(bubY + copyH, 200));
				scratch.fillRect(band, 0);
				copyY = bubY;
			}

			// `_DisplayClue` @ 2404:07fe passes fontColor=0 (palette
			// index 0 of the case-briefing palette 0x22) to `_WordWrap`.
			// Hard-coding 0xF here gave the wrong colour.
			_font.drawWordWrapped(&scratch, textX, textY,
				MAX<int>(8, textW), text, 0);

			g_system->copyRectToScreen(scratch.getBasePtr(0, copyY),
				scratch.pitch, 0, copyY, 320,
				MIN<int>(copyH, 200 - copyY));
			g_system->updateScreen();
		}

		// `_DisplayClue` @ 2404:0833-085a — after the balloon is drawn,
		// spool the per-clue voice. Each ClueEntry stores two 1-based
		// sound indices: `+0x18` for partner=Jenny and `+0x1a` for
		// partner=Jake (verified against 2404:0823-0834). Index 0 / -1
		// = no audio. The original blocks until the line ends; we run
		// async (the wait happens implicitly while the player reads).
		if (_audio) {
			const uint16 voiceJenny = READ_LE_UINT16(c + 0x18);
			const uint16 voiceJake  = READ_LE_UINT16(c + 0x1a);
			const uint16 voice = (_partner == 0) ? voiceJake : voiceJenny;
			if (voice != 0 && voice != 0xFFFF)
				_audio->spoolSound((uint)(voice - 1));
		}

		// Wait for click/key to advance — only if we drew something.
		// ESC skips the entire dialogue rather than just one entry.
		if (hasText || (charPicId != 0 && charPicId != 0xFFFF)) {
			bool advance = false;
			bool skipAll = false;
			while (!advance && !shouldQuit()) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_QUIT ||
						ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
						advance = true;
						break;
					}
					if (ev.type == Common::EVENT_KEYDOWN &&
						ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
						advance = true;
						skipAll = true;
						break;
					}
					if (ev.type == Common::EVENT_LBUTTONDOWN ||
						ev.type == Common::EVENT_KEYDOWN) {
						advance = true;
						break;
					}
				}
				// Tick the screen so the OSystem cursor follows the
				// mouse — ScummVM redraws the cursor overlay only on
				// updateScreen.
				g_system->updateScreen();
				g_system->delayMillis(10);
			}
			if (skipAll) {
				// Apply remaining side-effects without rendering. The
				// original silently runs the state updates even when the
				// player skips ahead.
				for (uint k = i; k < number; k++)
					applyClueSideEffects(clueBlock + 4 + k * 62);
				return;
			}
		}

		applyClueSideEffects(c);
	}
}

bool EEMEngine::areYouSure() {
	// Mirrors `_AreYouSure` @ 1a35:0a5c. Original loads PIC 0x136 for the
	// dialog body and PIC 0x1FD/0x1FE for YES/NO. We render a minimal
	// text dialog that preserves the screen behind it.
	if (!_font.isLoaded())
		return true;

	Graphics::Surface *screen = g_system->lockScreen();
	Graphics::ManagedSurface saved(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	if (screen) {
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)saved.getBasePtr(0, row),
				   (const byte *)screen->getBasePtr(0, row), 320);
		}
		g_system->unlockScreen();
	}

	const Common::Rect dlg(60, 70, 260, 140);
	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	for (int row = 0; row < 200; row++)
		memcpy((byte *)scratch.getBasePtr(0, row),
			   (const byte *)saved.getBasePtr(0, row), 320);
	scratch.fillRect(dlg, 0);
	scratch.frameRect(dlg, 0xF);
	_font.drawString(&scratch, "Are you sure you want to quit?", dlg.left + 8, dlg.top + 8, 320, 0xF);
	_font.drawString(&scratch, "Y - Yes", dlg.left + 16, dlg.top + 36, 320, 0xF);
	_font.drawString(&scratch, "N - No", dlg.left + 100, dlg.top + 36, 320, 0xF);
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();

	bool result = false;
	while (!shouldQuit()) {
		Common::Event ev;
		bool decided = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				result = true;
				decided = true;
				break;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_y ||
					ev.kbd.keycode == Common::KEYCODE_RETURN) {
					result = true; decided = true; break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_n ||
					ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					result = false; decided = true; break;
				}
			}
		}
		if (decided)
			break;
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	// Restore the screen so the caller's UI is intact.
	g_system->copyRectToScreen(saved.getPixels(), saved.pitch, 0, 0, 320, 200);
	g_system->updateScreen();
	return result;
}

} // End of namespace EEM
