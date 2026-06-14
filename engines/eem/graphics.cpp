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
#include "common/savefile.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "graphics/cursorman.h"
#include "graphics/managed_surface.h"

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"

namespace EEM {

// HelpData @ 29be:00c8, read by _InterfaceHelp @ 1560:0205. 5-byte entries:
// u8 count, then up to 2 u16 picIds. Verified bytes:
//   entry 0 (PDA / gallery HELP button): count=2, picIds = 0x0063, 0x01ae
//   entry 1:                              count=2, picIds = 0x0192, 0x01b1
// Only entry 0 is reachable from the PDA notebook (rect 1) and the gallery
// (rect 1) — both call _InterfaceHelp(0).
const uint16 kHelpPics[][2] = {
	{ 0x0063, 0x01ae },
	{ 0x0192, 0x01b1 },
};

// kBalloonInsetTable: 52 entries x 10 bytes @ 29be:0875 (CD) / 2608:05f9
// (floppy), indexed by (bubNum & 0x7F). Used by _DisplayClue to position
// _WordWrap text inside the balloon AND by _DisplayHotspotClue_Floppy
// @ 22dc:05c8 to position the "click to continue" indicator. Layout per entry:
//   +0..1 = text X inset
//   +2..3 = text Y inset
//   +4..5 = wrap width
//   +6..7 = "more / end" indicator X within the balloon
//   +8..9 = "more / end" indicator Y within the balloon
struct BalloonInsets {
	uint16 x;
	uint16 y;
	uint16 w;
	uint16 indDX;
	uint16 indDY;
};
const BalloonInsets kBalloonInsetTable[] = {
	{ 6, 4, 142, 150,  6 }, { 6, 4, 142, 150, 14 }, { 6, 4, 142, 150, 22 }, { 6, 4, 142, 150, 40 },
	{ 6, 4, 142, 150, 50 }, { 6, 4, 142, 150, 70 }, { 6, 4, 142, 150, 86 },
	{ 6, 4, 224, 233,  6 }, { 6, 4, 224, 233, 14 }, { 6, 4, 224, 233, 23 }, { 6, 4, 224, 233, 41 },
	{ 6, 4, 224, 233, 59 }, { 6, 4, 224, 233, 68 }, { 6, 4, 224, 233, 77 },
	{ 6, 4, 291, 300,  6 }, { 6, 4, 291, 300, 14 }, { 6, 4, 291, 300, 23 }, { 6, 4, 291, 300, 32 },
	{ 6, 4, 291, 300, 50 }, { 6, 4, 291, 300, 69 }, { 6, 4, 291, 300, 77 },
	{ 5, 4, 155, 150, 14 }, { 5, 4, 155, 150, 25 }, { 5, 4, 155, 150, 35 }, { 5, 4, 155, 150, 52 },
	{ 5, 4, 155, 150, 61 }, { 5, 4, 155, 150, 78 }, { 5, 4, 155, 150, 81 },
	{ 5, 4, 237, 233, 15 }, { 5, 4, 237, 233, 24 }, { 5, 4, 237, 233, 42 }, { 5, 4, 237, 233, 50 },
	{ 5, 4, 237, 233, 69 }, { 5, 4, 237, 233, 80 }, { 5, 4, 237, 233, 80 },
	{ 3, 4, 155, 150, 15 }, { 3, 4, 155, 150, 24 }, { 3, 4, 155, 150, 34 }, { 3, 4, 155, 150, 40 },
	{ 3, 4, 155, 150, 61 }, { 3, 4, 155, 150, 82 }, { 3, 4, 155, 150, 88 },
	{ 5, 4, 238, 232, 16 }, { 5, 4, 238, 232, 25 }, { 5, 4, 238, 232, 34 }, { 5, 4, 238, 232, 51 },
	{ 5, 4, 238, 232, 66 }, { 5, 4, 238, 232, 71 }, { 5, 4, 238, 232, 95 },
	{ 5, 8, 158, 160, 45 }, { 5, 8, 176, 176, 50 }, { 8, 7, 142, 142, 71 }
};

struct BalloonFamily {
	uint16 first;
	uint16 last;
};

const BalloonFamily kBalloonFamilies[] = {
	{ 0x00, 0x06 },
	{ 0x07, 0x0d },
	{ 0x0e, 0x14 },
	{ 0x15, 0x1b },
	{ 0x1c, 0x22 },
	{ 0x23, 0x29 },
	{ 0x2a, 0x30 },
	{ 0x31, 0x31 },
	{ 0x32, 0x32 },
	{ 0x33, 0x33 },
};

bool findBalloonFamily(uint16 balloonId, uint16 &first, uint16 &last) {
	for (uint i = 0; i < ARRAYSIZE(kBalloonFamilies); i++) {
		if (balloonId >= kBalloonFamilies[i].first &&
			balloonId <= kBalloonFamilies[i].last) {
			first = kBalloonFamilies[i].first;
			last  = kBalloonFamilies[i].last;
			return true;
		}
	}
	first = last = balloonId;
	return false;
}

// indDY is the artist-intended last text line, not just the indicator Y:
// families 3, 4, 6 and singletons have shadow/tail decoration below indDY.
uint getBalloonLineCapacity(uint16 balloonId, int lineH) {
	const uint idx = balloonId & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable) || lineH <= 0)
		return 0;

	const BalloonInsets &insets = kBalloonInsetTable[idx];
	return MAX<uint>(1, ((int)insets.indDY - (int)insets.y) / lineH + 1);
}

bool EEMEngine::floppyHotspotSearched(uint siteIdx, uint hotspotIdx) const {
	// FUN_22dc_096c @ 22dc:096c: walks per-site dialog records at
	// site_data[+6] to skip hotspotIdx hotspots, then returns _TextSeen for
	// the selected hotspot's searched text index.
	const byte *site = _mystery.siteData(siteIdx);
	if (!site)
		return false;
	const uint16 dlgListOff = READ_LE_UINT16(site + 6);
	const byte *bufBase = _mystery.blobAt(0);
	const uint32 dsz = _mystery.dataSize();
	if (!bufBase || dlgListOff == 0 || dlgListOff >= dsz)
		return false;
	uint32 off = dlgListOff;
	for (uint h = 0; h < hotspotIdx; h++) {
		if (off + 10 >= dsz)
			return false;
		const uint32 mainLen = 11u + (uint)bufBase[off + 10];
		off += mainLen;
		if (off >= dsz)
			return false;
		const uint contCount = (uint)(bufBase[off] & 0x7F);
		off += 1;
		for (uint c = 0; c < contCount; c++) {
			if (off + 10 >= dsz)
				return false;
			off += 11u + (uint)bufBase[off + 10];
			if (off >= dsz)
				return false;
		}
	}
	if (off + 10 >= dsz)
		return false;
	const uint32 mainLen = 11u + (uint)bufBase[off + 10];
	const uint32 contFlagsOff = off + mainLen;
	if (contFlagsOff >= dsz)
		return false;
	uint32 searchedRecOff = off;
	if ((bufBase[contFlagsOff] & 0x7F) != 0)
		searchedRecOff = contFlagsOff + 1;
	if (searchedRecOff + 11 >= dsz || bufBase[searchedRecOff + 10] == 0)
		return false;
	const uint8 textIdx = bufBase[searchedRecOff + 11] & 0x7F;
	return textIdx < EEM::Mystery::kCluesFoundCap &&
		   _mystery._cluesFound[textIdx] != 0;
}

void EEMEngine::doHelp() {
	// Floppy per-mystery H<n>.BIN hint files. Loader FUN_1503_0001 @ 1503:0001
	// (format string "h%d.bin" @ 2608:0154), consumer FUN_1503_01a5 @ 1503:01a5.
	// Format:
	//   byte numChainHints; numChainHints × { byte siteIdx; byte hotspotIdx; }
	//   byte numExtraHints; numExtraHints × { byte siteIdx; byte hotspotIdx; }
	//   asciiz str1, str2, str3 (post-solve, score >= 100)
	// Selection: any chain hotspot unsearched -> str1; else any extra
	// unsearched -> str2; else selectedPoints() >= 100 -> str3.
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
		while ((uint)((const byte *)p - hd) < hsz && *p != 0)
			p++;
		if ((uint)((const byte *)p - hd) >= hsz)
			return;
		str2 = p + 1;
		p = str2;
		while ((uint)((const byte *)p - hd) < hsz && *p != 0)
			p++;
		if ((uint)((const byte *)p - hd) >= hsz)
			return;
		str3 = p + 1;

		const char *chosen = nullptr;
		bool anyChainUnseen = false;
		for (uint i = 0; i < chainCount; i++) {
			const uint8 siteIdx    = hd[off + i * 2 + 0];
			const uint8 hotspotIdx = hd[off + i * 2 + 1];
			if (!floppyHotspotSearched(siteIdx, hotspotIdx)) {
				anyChainUnseen = true;
				break;
			}
		}
		bool anyExtraUnseen = false;
		if (!anyChainUnseen) {
			for (uint i = 0; i < extraCount; i++) {
				const uint8 siteIdx    = hd[extraStart + i * 2 + 0];
				const uint8 hotspotIdx = hd[extraStart + i * 2 + 1];
				if (!floppyHotspotSearched(siteIdx, hotspotIdx)) {
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

		// _GetKDTextBalloon @ 1df2:0105 (floppy FUN_1d40_009f) indexes the
		// per-character table at 2608:0c14 by the literal byte. Bytes at
		// 2608:0c44 (= 0xc14 + '0') give the '0'..'9' -> balloon-id mapping:
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
		balloonIdx = fitBalloonToText((uint16)balloonIdx, text) & 0x7F;
		Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
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
			ms.transBlitFrom(balloon.surface,
							 Common::Point(0x21, balloonY),
							 (uint32)(byte)(balloon.flags >> 8));
		}
		uint16 bx = 5;
		uint16 by = 4;
		uint16 bw = 142;
		getBalloonInsets(balloonIdx, bx, by, bw);
		_font.drawWordWrapped(&ms, 0x21 + bx, balloonY + by,
							  MAX<int>(8, (int)bw), text, 0);
		g_system->copyRectToScreen(ms.getPixels(), ms.pitch, 0, 0, kScreenWidth, kScreenHeight);
		g_system->updateScreen();

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

	// Mirrors _KDHelp @ 1560:010a. Walks the first two _AChain entries
	// (the puzzle's required-clue chain — the "spine" of evidence the
	// player must collect):
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
	//       // Generic KD hint: KDTextIndex[+0xe] (first time) /
	//       // KDTextIndex[+0x10] (second time, toggled by _SawHelpHint).
	//       // If no chain hint was ever defined, render the "no hints"
	//       // sentinel instead.
	//       _DisplayHint(...);
	//   }
	//
	// SMART per-puzzle hint: partner points at whichever chain clue the
	// player hasn't found yet, only falling back to the generic line once
	// every chain hint has been triggered.
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
		// Second arm of _KDHelp (1560:0152-019b): generic KD hint fallback.
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
		// Else: original would render NoHints string @ 29be:00d3; we bail.
	}

	if (chosenText == 0xFFFF) {
		debugC(1, kDebugScript, "doHelp: no hint available");
		return;
	}

	const Common::String raw  = _mystery.textAt(chosenText);
	Common::String text = parseString(raw, _playerName, _partner);

	// Render as a speech-balloon overlay, mirroring _DisplayHint @ 1560:0009:
	//
	//   _GetKDTextBalloon(text, &bub);             // first-char dispatch
	//   _GetBalloon(bub);                          // load balloon pic
	//   y = (h < 0x4e) ? (0x50 - h) >> 1 : 1;      // vertical centre
	//   _AddPicBackground(balloon, 0x21, y);       // overlay on screen
	//   _WordWrap(0x21+tbl[bub].x, y+tbl[bub].y,   // text inside balloon
	//             tbl[bub].w, text, -1, color=0);
	//   _SayKDDigital(snd);                        // partner voice
	//   _Wait();
	//
	// BG is the caller's CURRENT screen (site / PDA / gallery), not a cleared
	// scratch.
	Graphics::ManagedSurface ms(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	ms.clear();
	{
		Graphics::Surface *cur = g_system->lockScreen();
		if (cur) {
			ms.simpleBlitFrom(*cur);
			g_system->unlockScreen();
		}
	}

	// Balloon shape dispatch via _GetKDTextBalloon @ 1df2:0105 — based on
	// the first char of the parsed text. Digits select a specific balloon
	// variant; non-digit defaults to 0x17. The digit, when present, is
	// THEN consumed from the displayed text — mirrors _DisplayAlibi
	// @ 1df2:0145's `str = pbVar7 + 1` advance after reading `*str` for
	// bindx. _GetKDTextBalloon itself doesn't strip it (1df2:0105 just
	// reads `*str`), so the caller has to.
	const byte firstChar =
		text.empty() ? (byte)0 : (byte)text[0];
	uint16 bubNum = getKDTextBalloon(firstChar);
	if (firstChar >= '0' && firstChar <= '9')
		text.deleteChar(0);
	bubNum = fitBalloonToText(bubNum, text);
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

	uint16 tx = 5, ty = 4, tw = 155;
	getBalloonInsets(bubNum, tx, ty, tw);
	_font.drawWordWrapped(&ms, balloonX + tx, balloonY + ty, tw, text,
						  haveBalloon ? 0 : 0xF);

	g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();

	// _DisplayHint @ 1560:0009 plays _SayKDDigital(soundnum) — a
	// partner-specific voice line keyed to which hint type fired:
	//   10 = first chain hint, 11 = second chain hint,
	//    7 = generic KD (first), 8 = generic KD (second).
	if (_audio && _mystery.kdTextIndex() && soundNum > 0)
		_audio->sayKDDigital(_mystery.kdTextIndex(), (uint)soundNum,
							 _partner);

	waitForInput(60000);
}

void EEMEngine::doInterfaceHelp(uint num) {
	// Mirrors _InterfaceHelp(num) @ 1560:0205. The original walks
	// HelpData @ 29be:00c8 (5-byte entries: u8 count, then up to 2 u16
	// picIds), _GetPictures each one, blits via _Rect_Move_Mask(0, 0, ...)
	// (a MASKED blit on top of the existing screen — transparent pixels
	// show the caller's BG), and waits for click / key. ESC at 1560:02b3
	// skips to the end. The function hides the cursor at the top
	// (MOV [0x3a00], 0 @ 1560:0216 + _RemoveMouse @ 1000:542f at
	// 1560:021c) and restores it at the tail (_DrawMouse @ 1000:5429
	// at 1560:02e8). See kHelpPics comment for HelpData decoding.
	if (num >= ARRAYSIZE(kHelpPics))
		return;

	debugC(1, kDebugScript, "doInterfaceHelp(%u): showing pics 0x%x, 0x%x",
		   num, kHelpPics[num][0], kHelpPics[num][1]);

	// Snapshot caller's screen once: each PIC overlays the same clean BG.
	Graphics::ManagedSurface bg(kScreenWidth, kScreenHeight,
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

		// transBlitFrom transp = pic.flags >> 8 matches _Rect_Move_Mask param_10
		// @ 1000:03fc. Explicit (0,0) destPos: no-arg overload stretches to fill.
		Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(bg);
		const byte transp = (byte)(pic.flags >> 8);
		scratch.transBlitFrom(pic.surface, Common::Point(0, 0),
							  (uint32)transp);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, kScreenWidth, kScreenHeight);
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
	if (bg && bg->w == kScreenWidth && bg->h == kScreenHeight) {
		_partnerEraseBg.create(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		_partnerEraseBg.simpleBlitFrom(*bg);
	} else {
		_partnerEraseBg.free();
	}
}

uint16 EEMEngine::fitBalloonToText(uint16 bubNum,
								   const Common::String &text) {
	// Opt-in via "fit_dialog_balloons", CD only (floppy table unvalidated).
	if (isFloppy() || !ConfMan.getBool("fit_dialog_balloons"))
		return bubNum;

	const uint16 originalId = bubNum & 0x7F;
	if (bubNum == 0xFFFF || text.empty() || !_font.isLoaded() ||
		originalId >= ARRAYSIZE(kBalloonInsetTable))
		return bubNum;

	const BalloonInsets &originalInsets = kBalloonInsetTable[originalId];
	const int lineH = _font.getFontHeight();
	const uint originalCapacity = getBalloonLineCapacity(originalId, lineH);
	if (originalCapacity == 0)
		return bubNum;

	Common::Array<Common::String> lines;
	_font.wordWrapText(text, MAX<int>(8, (int)originalInsets.w), lines);
	if (lines.empty())
		return bubNum;

	const uint usedLines = lines.size();
	if (usedLines > originalCapacity)
		return bubNum;

	uint16 familyFirst = 0;
	uint16 familyLast = 0;
	if (!findBalloonFamily(originalId, familyFirst, familyLast))
		return bubNum;

	uint16 chosenId = originalId;
	uint chosenCapacity = originalCapacity;
	debug(
		   "fitBalloonToText: original 0x%02x, usedLines=%u, capacity=%u, family=0x%02x-0x%02x",
		   (int)originalId, usedLines, originalCapacity,
		   (int)familyFirst, (int)familyLast);
	while (chosenId > familyFirst) {
		if (chosenCapacity < usedLines)
			break;

		const uint16 candidateId = chosenId - 1;
		if (candidateId >= ARRAYSIZE(kBalloonInsetTable) ||
			kBalloonInsetTable[candidateId].w != originalInsets.w)
			break;
		const uint candidateCapacity =
			getBalloonLineCapacity(candidateId, lineH);
		if (candidateCapacity < usedLines)
			break;
		chosenId = candidateId;
		chosenCapacity = candidateCapacity;
	}

	if (chosenId == originalId)
		return bubNum;

	debug(
		   "fitBalloonToText: 0x%02x -> 0x%02x (%u lines, capacity %u -> %u)",
		   (int)originalId, (int)chosenId, usedLines,
		   originalCapacity, chosenCapacity);
	return (bubNum & 0x80) | chosenId;
}

bool EEMEngine::getBalloonInsets(uint16 bubNum, uint16 &xInset,
								  uint16 &yInset, uint16 &textW) const {
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	xInset = kBalloonInsetTable[idx].x;
	yInset = kBalloonInsetTable[idx].y;
	textW  = kBalloonInsetTable[idx].w;
	return true;
}

bool EEMEngine::getBalloonIndicatorPos(uint16 bubNum, uint16 &dx,
										uint16 &dy) const {
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kBalloonInsetTable))
		return false;
	dx = kBalloonInsetTable[idx].indDX;
	dy = kBalloonInsetTable[idx].indDY;
	return true;
}

void EEMEngine::drawFloppyBubbleIndicator(Graphics::ManagedSurface &dst,
										   uint16 bubNum, int ballX, int ballY,
										   bool endIndicator) {
	// Mirrors _DisplayHotspotClue_Floppy @ 22dc:08c0 (end-of-record) and
	// @ 22dc:08aa (mid-pagination). Both grab a pre-loaded PIC:
	//   DAT_28da_3034 = PIC 0xa0  "more pages" indicator
	//   DAT_28da_3030 = PIC 0xa1  "end" indicator
	// and stamp it at (ballX + insetTable[bubNum].indDX,
	//                  ballY + insetTable[bubNum].indDY) via _AddPicBackground.
	uint16 dx = 0;
	uint16 dy = 0;
	if (!getBalloonIndicatorPos(bubNum, dx, dy))
		return;
	const uint picId = endIndicator ? 0xa1 : 0xa0;
	Picture pic;
	if (!_picsArchive.getPicture(picId, pic))
		return;
	const int x = ballX + (int)dx;
	const int y = ballY + (int)dy;
	dst.transBlitFrom(pic.surface, Common::Point(x, y),
					  (uint32)(byte)(pic.flags >> 8));
}

} // End of namespace EEM
