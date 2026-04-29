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

// EEM — graphics helpers (KDGRAPH.C + KDHELP.C, the latter merged because
// it has only three functions). Animation playback, balloon-table lookup,
// and the help-screen primitives that share the same balloon machinery.

namespace EEM {

// `_InterfaceHelp(num)` @ 1560:0205 reads `HelpData @ 29be:00c8` (5-byte
// entries: u8 count, then up to 2 u16 picIds). Verified bytes:
//   entry 0 (PDA / gallery HELP button): count=2, picIds = 0x0063, 0x01ae
//   entry 1: count=2, picIds = 0x0192, 0x01b1
// Only entry 0 is reachable from the PDA notebook (rect 1) and the
// gallery (rect 1) — both call `_InterfaceHelp(0)`.
const uint16 kHelpPics[][2] = {
	{ 0x0063, 0x01ae },
	{ 0x0192, 0x01b1 },
};

// 52-entry, 10-bytes-each balloon-metadata table at `29be:0875`.
// Used at 1df2:0aef-0af9 (accuse hint) and `_DisplayClue` to position
// `_WordWrap` text inside the balloon. Only +0/+2/+4 are read by
// `getBalloonInsets`:
//   +0..1 = text X inset, +2..3 = Y inset, +4..5 = max wrap width
// (+6/+8 = balloon h / tail offset, both unused for text layout).
struct BalloonInsets { uint16 x; uint16 y; uint16 w; };
const BalloonInsets kBalloonInsetTable[] = {
	{ 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 },
	{ 6, 4, 142 }, { 6, 4, 142 }, { 6, 4, 142 },
	{ 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 },
	{ 6, 4, 224 }, { 6, 4, 224 }, { 6, 4, 224 },
	{ 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 },
	{ 6, 4, 291 }, { 6, 4, 291 }, { 6, 4, 291 },
	{ 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 },
	{ 5, 4, 155 }, { 5, 4, 155 }, { 5, 4, 155 },
	{ 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 },
	{ 5, 4, 237 }, { 5, 4, 237 }, { 5, 4, 237 },
	{ 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 },
	{ 3, 4, 155 }, { 3, 4, 155 }, { 3, 4, 155 },
	{ 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 },
	{ 5, 4, 238 }, { 5, 4, 238 }, { 5, 4, 238 },
	{ 5, 8, 158 }, { 5, 8, 176 }, { 8, 7, 142 }
};

// Floppy KDHelp hotspot-searched check. Mirrors
// `FUN_22dc_096c @ 22dc:096c`: walks the per-site dialog records at
// `site_data[+6]` to skip `hotspotIdx` hotspots, then returns the
// `_cluesFound` flag for that hotspot's first text index.
static bool floppyHotspotSearched(EEM::Mystery &mystery, uint siteIdx,
								   uint hotspotIdx) {
	const byte *site = mystery.siteData(siteIdx);
	if (!site)
		return false;
	const uint16 dlgListOff = READ_LE_UINT16(site + 6);
	const byte *bufBase = mystery.blobAt(0);
	if (!bufBase || dlgListOff == 0 || dlgListOff >= mystery.dataSize())
		return false;
	uint32 off = dlgListOff;
	for (uint h = 0; h < hotspotIdx; h++) {
		const byte *rec = bufBase + off;
		off += 11u + (uint)rec[10];
		if (off >= mystery.dataSize())
			return false;
		const uint contCount = (uint)(bufBase[off] & 0x7F);
		off += 1;
		for (uint c = 0; c < contCount; c++) {
			const byte *cr = bufBase + off;
			off += 11u + (uint)cr[10];
			if (off >= mystery.dataSize())
				return false;
		}
	}
	if (off + 11 >= mystery.dataSize())
		return false;
	const byte *mainRec = bufBase + off;
	const uint8 textIdx = mainRec[11] & 0x7F;
	return textIdx < EEM::Mystery::kCluesFoundCap &&
		   mystery._cluesFound[textIdx] != 0;
}

void EEMEngine::doHelp() {
	// Floppy uses a totally different hint mechanism — per-mystery
	// `H<n>.BIN` data files (one per case). Format verified at
	// `FUN_1503_0001 @ 1503:0001` (loader, format string at
	// `2608:0154` = "h%d.bin") + `FUN_1503_01a5 @ 1503:01a5`
	// (consumer):
	//   byte 0 = numChainHints
	//   numChainHints × { byte siteIdx; byte hotspotIdx; }
	//   byte = numExtraHints
	//   numExtraHints × { byte siteIdx; byte hotspotIdx; }
	//   asciiz string 1  ("[balloon-digit]Let's go to <site>...")
	//   asciiz string 2  (alternate hint)
	//   asciiz string 3  (post-solve hint, used when score ≥ 100)
	// Selection logic: if any chain hotspot is unsearched → string 1.
	// Else if any extra hotspot is unsearched → string 2. Else if
	// `selectedPoints() ≥ 100` → string 3.
	if (isFloppy() && _mystery.isLoaded()) {
		const Common::String filename = Common::String::format("H%u.BIN",
															   _mystery.number());
		Common::File hf;
		if (!hf.open(Common::Path(filename))) {
			warning("doHelp: cannot open %s", filename.c_str());
			return;
		}
		const uint32 hsz = hf.size();
		Common::Array<byte> hbuf;
		hbuf.resize(hsz);
		if (hf.read(hbuf.data(), hsz) != hsz)
			return;
		const byte *hd = hbuf.data();

		const uint chainCount = hd[0];
		uint off = 1;
		uint chainEnd = off + chainCount * 2;
		if (chainEnd >= hsz)
			return;
		const uint extraCount = hd[chainEnd];
		uint extraStart = chainEnd + 1;
		uint extraEnd = extraStart + extraCount * 2;
		if (extraEnd >= hsz)
			return;
		// Three NUL-terminated strings follow.
		const char *str1 = (const char *)(hd + extraEnd);
		const char *str2 = nullptr;
		const char *str3 = nullptr;
		const char *p = str1;
		while ((uint)((const byte *)p - hd) < hsz && *p != 0) p++;
		if ((uint)((const byte *)p - hd) >= hsz) return;
		str2 = p + 1;
		p = str2;
		while ((uint)((const byte *)p - hd) < hsz && *p != 0) p++;
		if ((uint)((const byte *)p - hd) >= hsz) return;
		str3 = p + 1;

		const char *chosen = nullptr;
		bool anyChainUnseen = false;
		for (uint i = 0; i < chainCount; i++) {
			const uint8 siteIdx    = hd[off + i * 2 + 0];
			const uint8 hotspotIdx = hd[off + i * 2 + 1];
			if (!floppyHotspotSearched(_mystery, siteIdx, hotspotIdx)) {
				anyChainUnseen = true;
				break;
			}
		}
		bool anyExtraUnseen = false;
		if (!anyChainUnseen) {
			for (uint i = 0; i < extraCount; i++) {
				const uint8 siteIdx    = hd[extraStart + i * 2 + 0];
				const uint8 hotspotIdx = hd[extraStart + i * 2 + 1];
				if (!floppyHotspotSearched(_mystery, siteIdx, hotspotIdx)) {
					anyExtraUnseen = true;
					break;
				}
			}
		}
		if (anyChainUnseen)
			chosen = str1;
		else if (anyExtraUnseen)
			chosen = str2;
		else if (_mystery.selectedPoints() >= 100)
			chosen = str3;
		if (!chosen || *chosen == 0)
			return;

		// Strip leading balloon-digit byte. `_GetKDTextBalloon @
		// 1df2:0105` (= floppy `FUN_1d40_009f`) doesn't take the
		// digit's *value* — it indexes the per-character table at
		// `2608:0c14` by the literal byte, so '0'..'9' map to a
		// non-trivial balloon-id sequence. Verified bytes at
		// `2608:0c44` (= 0xc14 + '0'):
		//   '0'→0x15, '1'→0x16, '2'→0x17, '3'→0x18, '4'→0x19,
		//   '5'→0x1a, '6'→0x1c, '7'→0x1d, '8'→0x1e, '9'→0x0a.
		// Without this map the previous (digit - '0') version asked
		// `getBalloonInsets` for balloon 0 (text width 142) instead of
		// the correct balloon 21 (text width 155), which is why the
		// hint bubble rendered narrower than the original.
		static const uint8 kFloppyDigitToBalloon[10] = {
			0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1c, 0x1d, 0x1e, 0x0a
		};
		uint balloonIdx = 0x17;
		const char *txt = chosen;
		if (*txt >= '0' && *txt <= '9') {
			balloonIdx = kFloppyDigitToBalloon[(int)(*txt - '0')];
			txt++;
		}

		Common::String text = parseString(Common::String(txt),
										   _playerName, _partner);
		Graphics::ManagedSurface ms(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		ms.clear();
		{
			Graphics::Surface *cur = g_system->lockScreen();
			if (cur) {
				ms.simpleBlitFrom(*cur);
				g_system->unlockScreen();
			}
		}
		Picture balloon;
		const bool haveBalloon = _balloonArchive.size() > balloonIdx &&
			_balloonArchive.loadEntry(balloonIdx, balloon);
		uint16 balloonY = 1;
		if (haveBalloon) {
			const uint h = (uint)balloon.surface.h;
			if (h < 0x4e)
				balloonY = (uint16)((0x50 - h) >> 1);
			const byte transp = (byte)(balloon.flags >> 8);
			for (int row = 0; row < balloon.surface.h && balloonY + row < 200;
				 row++) {
				const byte *src =
					(const byte *)balloon.surface.getBasePtr(0, row);
				byte *dst = (byte *)ms.getBasePtr(0x21, balloonY + row);
				for (int col = 0; col < balloon.surface.w && 0x21 + col < 320;
					 col++) {
					if (src[col] != transp)
						dst[col] = src[col];
				}
			}
		}
		uint16 bx = 5;
		uint16 by = 4;
		uint16 bw = 142;
		getBalloonInsets(balloonIdx, bx, by, bw);
		_font.drawWordWrapped(&ms, 0x21 + bx, balloonY + by,
							  MAX<int>(8, (int)bw), text, 0);
		g_system->copyRectToScreen(ms.getPixels(), ms.pitch, 0, 0, 320, 200);
		g_system->updateScreen();

		// Wait for click — KD hint dismisses on any input.
		while (!shouldQuit()) {
			Common::Event ev;
			bool advance = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					ev.type == Common::EVENT_LBUTTONDOWN ||
					ev.type == Common::EVENT_KEYDOWN) {
					advance = true;
					break;
				}
			}
			if (advance)
				break;
			g_system->updateScreen();
			g_system->delayMillis(10);
		}
		return;
	}

	// Mirrors `_KDHelp @ 1560:010a`. The original walks the first two
	// entries of `_AChain` (the puzzle's required-clue chain — the
	// "spine" of evidence the player must collect):
	//
	//   for (i = 0; i < 2; i++) {
	//       if (_AChain[i] != -1 && _HintBlock[i] != -1 &&
	//           _CluesFound[_AChain[i]] == 0) {
	//           _DisplayHint(TextBlock + _HintBlock[i], i + 10);
	//           shown++; break;
	//       }
	//       if (_HintBlock[i] != -1) defined++;
	//   }
	//   if (!shown) {
	//       // Fall back to the generic KD hint: KDTextIndex[+0xe]
	//       // (first time) / KDTextIndex[+0x10] (second time, toggled
	//       // by _SawHelpHint). If neither chain entry had a hint
	//       // defined, show the global "no hints" sentinel instead.
	//       _DisplayHint(...);
	//   }
	//
	// So this is a SMART per-puzzle hint: the partner points the
	// player at whichever chain clue they haven't yet found, only
	// falling back to the generic "let's keep looking" line when
	// every chain hint has been triggered already.
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	const byte *kd  = _mystery.kdTextIndex();
	const byte *hb  = _mystery.hintBlock();
	if (!kd)
		return;

	uint16 chosenText = 0xFFFF;
	int    soundNum   = 0;
	bool   anyHintDefined = false;

	if (hb) {
		for (uint i = 0; i < 2; i++) {
			const uint16 chainClue = _mystery.aChain(i);
			if (chainClue == 0xFFFF)
				continue;
			const uint16 hintOff = READ_LE_UINT16(hb + i * 2);
			if (hintOff == 0xFFFF)
				continue;
			anyHintDefined = true;
			if (chainClue < Mystery::kCluesFoundCap &&
				_mystery._cluesFound[chainClue] == 0) {
				chosenText = hintOff;
				soundNum   = (int)i + 10;
				break;
			}
		}
	}

	if (chosenText == 0xFFFF) {
		// No unfound chain clue had a hint to give — fall back to the
		// generic KD hint (or the "no hints defined" sentinel if the
		// chain has no hints at all). Mirrors the second arm of
		// `_KDHelp` (1560:0152-019b).
		if (anyHintDefined) {
			const uint16 hintFirst  = READ_LE_UINT16(kd + 0x0e);
			const uint16 hintSecond = READ_LE_UINT16(kd + 0x10);
			if (!_mystery._sawHelpHint && hintFirst != 0xFFFF) {
				chosenText = hintFirst;
				soundNum   = 7;
				_mystery._sawHelpHint = true;
			} else if (hintSecond != 0xFFFF) {
				chosenText = hintSecond;
				soundNum   = 8;
			}
		}
		// Else: keep chosenText == 0xFFFF — original would render
		// `NoHints` (a "There are no hints defined for this Mystery"
		// string at 29be:00d3); we just bail.
	}

	if (chosenText == 0xFFFF) {
		debugC(1, kDebugScript, "doHelp: no hint available");
		return;
	}

	const Common::String raw  = _mystery.textAt(chosenText);
	Common::String text = parseString(raw, _playerName, _partner);

	// Render as a speech-balloon overlay, exactly mirroring
	// `_DisplayHint @ 1560:0009`:
	//
	//   _GetKDTextBalloon(text, &bub);             // first-char dispatch
	//   _GetBalloon(bub);                           // load balloon pic
	//   y = (h < 0x4e) ? (0x50 - h) >> 1 : 1;       // vertical centre
	//   _AddPicBackground(balloon, 0x21, y);        // overlay on screen
	//   _WordWrap(0x21+tbl[bub].x, y+tbl[bub].y,   // text inside balloon
	//             tbl[bub].w, text, -1, color=0);
	//   _SayKDDigital(snd);                          // partner voice
	//   _Wait();
	//
	// The balloon BG is the caller's CURRENT screen — site / PDA /
	// gallery — not a cleared scratch.
	Graphics::ManagedSurface ms(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	ms.clear();
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			ms.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
	}

	// Balloon shape dispatch via `_GetKDTextBalloon @ 1df2:0105` —
	// based on the first char of the parsed text. Digits select a
	// specific balloon variant; non-digit defaults to `0x17`. The
	// digit, when present, is THEN consumed from the displayed
	// text — mirrors `_DisplayAlibi @ 1df2:0145`'s `str = pbVar7 + 1`
	// advance after using `*str` for `bindx`. Without this the hint
	// renders like "1Try checking the kitchen..." with a stray
	// leading digit. `_GetKDTextBalloon` itself doesn't strip it
	// (verified at 1df2:0105 — it just reads `*str`), so the caller
	// has to.
	const byte firstChar =
		text.empty() ? (byte)0 : (byte)text[0];
	const uint16 bubNum = getKDTextBalloon(firstChar);
	if (firstChar >= '0' && firstChar <= '9')
		text.deleteChar(0);
	Picture balloon;
	const bool haveBalloon =
		_balloonArchive.size() > (bubNum & 0x7F) &&
		_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

	const int balloonX = 0x21;
	int balloonY = 1;
	if (haveBalloon && balloon.surface.h < 0x4e)
		balloonY = (0x50 - balloon.surface.h) / 2;

	if (haveBalloon) {
		const byte transp = (byte)(balloon.flags >> 8);
		ms.transBlitFrom(balloon.surface,
						 Common::Point(balloonX, balloonY),
						 (uint32)transp);
	}

	// Balloon-relative text insets from the table at `29be:0875`
	// (10 bytes per entry: x, y, max-width, ...).
	uint16 tx = 5, ty = 4, tw = 155;
	getBalloonInsets(bubNum, tx, ty, tw);
	_font.drawWordWrapped(&ms, balloonX + tx, balloonY + ty, tw, text,
						  haveBalloon ? 0 : 0xF);

	g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();

	// `_DisplayHint @ 1560:0009` plays `_SayKDDigital(soundnum)` —
	// partner-specific voice line keyed to which hint type fired (10
	// = first chain hint, 11 = second, 7 / 8 = generic KD).
	if (_audio && _mystery.kdTextIndex() && soundNum > 0)
		_audio->sayKDDigital(_mystery.kdTextIndex(), (uint)soundNum,
							 _partner);

	waitForInput(60000);
}

void EEMEngine::doInterfaceHelp(uint num) {
	// Mirrors `_InterfaceHelp(num)` @ 1560:0205. The original walks
	// `HelpData @ 29be:00c8` (5-byte entries: u8 count, then up to 2
	// u16 picIds), `_GetPicture`s each one, blits it via
	// `_Rect_Move_Mask(0, 0, ...)` (a MASKED blit on top of the
	// existing screen — transparent pixels show the caller's BG), and
	// waits for click / key. ESC at `1560:02b3` skips to end. The
	// function also hides the cursor at the top (`MOV [0x3a00], 0` at
	// 1560:0216 + `_RemoveMouse @ 1000:542f` at 1560:021c) and
	// restores it at the tail (`_DrawMouse @ 1000:5429` at 1560:02e8).
	//
	// `kHelpPics` lives at file scope above; see comment there for the
	// HelpData decoding.
	if (num >= ARRAYSIZE(kHelpPics))
		return;

	debugC(1, kDebugScript, "doInterfaceHelp(%u): showing pics 0x%x, 0x%x",
		   num, kHelpPics[num][0], kHelpPics[num][1]);

	// Snapshot the caller's screen ONCE so each help PIC overlays the
	// same clean BG. Without this, after the first PIC is dismissed the
	// second snapshot would include the first PIC's pixels and the two
	// would composite together — same gotcha as the setup-screen help
	// loop fix in `doSetup`.
	Graphics::ManagedSurface bg(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			bg.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
	}

	const bool wasShown = CursorMan.isVisible();
	CursorMan.showMouse(false);

	for (uint i = 0; i < 2; i++) {
		const uint16 picId = kHelpPics[num][i];
		Picture pic;
		if (!_picsArchive.getPicture(picId, pic)) {
			warning("doInterfaceHelp: getPicture(0x%x) failed", picId);
			continue;
		}
		debugC(1, kDebugScript, "doInterfaceHelp: pic 0x%x = %dx%d flags=0x%x",
			   picId, pic.surface.w, pic.surface.h, pic.flags);

		// Compose a 320x200 frame from the clean BG snapshot and overlay
		// the help pic with `transBlitFrom` — `Graphics::ManagedSurface`'s
		// masked blit (transparent colour = the pic's `flags >> 8`,
		// matching `_Rect_Move_Mask`'s param_10 at 1000:03fc). Pass an
		// explicit `destPos` of (0, 0) — the no-destPos overload at
		// managed_surface.cpp:738 scales src to fill `this`'s rect,
		// stretching the help PIC to 320x200 instead of placing it at
		// native size. The original `_Rect_Move_Mask` passes destX=0,
		// destY=0 with copy-width = pic[+4] (= `pic.surface.w`) and
		// copy-height = pic[+2] (= `pic.surface.h`) — i.e. native size,
		// not stretched.
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(bg);
		const byte transp = (byte)(pic.flags >> 8);
		scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
							  (uint32)transp);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		bool escape = false;
		while (!shouldQuit() && !escape) {
			Common::Event ev;
			bool advance = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					if (wasShown)
						CursorMan.showMouse(true);
					return;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					advance = true;
					break;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
						escape = true;
					else
						advance = true;
					break;
				}
			}
			if (advance || escape)
				break;
			g_system->updateScreen();
			g_system->delayMillis(15);
		}
		if (escape)
			break;
	}

	if (wasShown)
		CursorMan.showMouse(true);
}

void EEMEngine::setPartnerEraseBg(const Graphics::ManagedSurface *bg) {
	if (bg && bg->w == 320 && bg->h == 200) {
		_partnerEraseBg.create(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		_partnerEraseBg.simpleBlitFrom(*bg);
	} else {
		_partnerEraseBg.free();
	}
}

bool EEMEngine::getBalloonInsets(uint16 bubNum, uint16 &xInset,
								  uint16 &yInset, uint16 &textW) const {
	// `kBalloonInsetTable` lives at file scope above; see comment there.
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	xInset = kBalloonInsetTable[idx].x;
	yInset = kBalloonInsetTable[idx].y;
	textW  = kBalloonInsetTable[idx].w;
	return true;
}

} // End of namespace EEM
