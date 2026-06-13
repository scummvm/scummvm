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

// Clue / briefing pipeline (SCRIPT.C + KD.C).

namespace EEM {

// _DoChoosePartner @ 1a35:0756.
const uint kPicChooseBackground = 0x8c; // _GetBackground(0x8c)
const uint kAniJake  = 8;
const uint kAniJenny = 9;

// _DoHappiness @ 172b:27b5 — cursor X picks one of 4 rects @ 29be:030f.
// Past rect 3 = level 4. Constexpr (Point, w, h) form to avoid a global
// constructor (-Wglobal-constructors).
constexpr Common::Rect kHappyZones[4] = {
	Common::Rect(Common::Point(  0, 0),  70, 200), // far left — Jenny very happy, Jake neutral
	Common::Rect(Common::Point( 70, 0),  56, 200), // Jenny's column
	Common::Rect(Common::Point(126, 0),  56, 200), // middle
	Common::Rect(Common::Point(182, 0),  53, 200), // Jake's column
};

// _NewAnimation positions @ 1a35:07b9 / 07d5.
const int kJakeX  = 0xe2; // 226
const int kJakeY  = 0x62; // 98
const int kJennyX = 0x42; // 66
const int kJennyY = 0x60; // 96

uint markClueBlockNotebookEntries(Mystery &mystery, const byte *clueBlock) {
	if (!clueBlock)
		return 0;

	const uint16 number = READ_LE_UINT16(clueBlock);
	if (number == 0 || number > 32)
		return 0;

	uint marked = 0;
	for (uint i = 0; i < number; i++) {
		const byte *entry = clueBlock + 4 + i * 62;
		for (uint j = 0; j < 5; j++) {
			const uint16 note = READ_LE_UINT16(entry + 0x30 + j * 2);
			if (note != 0xFFFF && note < Mystery::kCluesFoundCap &&
				mystery._cluesFound[note] == 0) {
				mystery._cluesFound[note] = 1;
				marked++;
			}
		}
	}
	return marked;
}

// _DoHappiness @ 172b:27b5 — per-zone sequence scripts.
// Jake seqs @ 29be:0337 (5 × 0x14 bytes), Jenny seqs @ 29be:039b. 9 frames each;
// the anim cells contain 10 cells = pairs of (neutral, smile) at 5 intensities.
const uint8 kJakeSeqs[5][9] = {
	{ 0,0,0,0,0,0,0,1,0 }, // level 0
	{ 2,2,2,2,2,2,2,3,2 }, // level 1
	{ 4,4,4,4,4,4,4,5,4 }, // level 2
	{ 6,6,6,6,6,6,7,6,6 }, // level 3
	{ 8,8,8,8,8,8,8,8,9 }, // level 4 (cursor past zone 3)
};
const uint8 kJennySeqs[5][9] = {
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

// Masked blit: transparent colour = high byte of p.flags.
// _AddPicBackground @ 172b:0ed4 pushes `word ptr ES:[BX] >> 8` as
// _Rect_Move_Mask's mask byte (the on-disk u16 at file offset 0 maps to
// Picture::flags).
void blitMaskedToScreen(const Picture &p, int x, int y) {
	Graphics::Surface *screen = g_system->lockScreen();
	if (!screen)
		return;
	const Common::Rect dst = Common::Rect(x, y, x + p.surface.w,
										  y + p.surface.h)
		.findIntersectingRect(Common::Rect(screen->w, screen->h));
	if (!dst.isEmpty()) {
		const Common::Rect src(dst.left - x, dst.top - y,
							   dst.right - x, dst.bottom - y);
		screen->copyRectToSurfaceWithKey(p.surface, dst.left, dst.top,
										 src, (uint32)(byte)(p.flags >> 8));
	}
	g_system->unlockScreen();
}

void blitRawToScreen(const Picture &p, int x, int y) {
	const int w = MIN<int>(p.surface.w, kScreenWidth - x);
	const int h = MIN<int>(p.surface.h, kScreenHeight - y);
	if (x < 0 || y < 0 || w <= 0 || h <= 0)
		return;

	g_system->copyRectToScreen(p.surface.getPixels(), p.surface.pitch,
							   x, y, w, h);
}

// _DoChoosePartner @ 1a35:0756. The original places Jake + Jenny animations
// on a backdrop and polls four click rectangles (two per character); we
// approximate with a single split at x=160 (left=Jenny, right=Jake).
void EEMEngine::doChoosePartner() {
	Picture background;
	if (!_picsArchive.getPicture(kPicChooseBackground, background)) {
		warning("ChoosePartner background (%u) load failed", kPicChooseBackground);
		return;
	}

	Animation jakeAnim;
	if (!_aniArchive.loadAnimation(kAniJake, jakeAnim) || jakeAnim.empty()) {
		warning("Jake animation (%u) load failed", kAniJake);
		return;
	}
	Animation jennyAnim;
	if (!_aniArchive.loadAnimation(kAniJenny, jennyAnim) || jennyAnim.empty()) {
		warning("Jenny animation (%u) load failed", kAniJenny);
		return;
	}

	setAnmPalette(Common::Path("TITLE.ANM"));

	// _DoChoosePartner opens with _SetMousePos(0xa0, 0x96).
	int curMouseX = 0xa0;
	uint level = happinessLevel(curMouseX);
	uint seqIdx = 0;

	blitAt(background, 0, 0);
	blitAt(jennyAnim[kJennySeqs[level][seqIdx % 9] % jennyAnim.size()],
		   kJennyX, kJennyY);
	blitAt(jakeAnim [kJakeSeqs [level][seqIdx % 9] % jakeAnim.size()],
		   kJakeX, kJakeY);
	g_system->updateScreen();

	debugC(1, kDebugGeneral, "ChoosePartner: %u Jake frames at (%d,%d), "
		   "%u Jenny frames at (%d,%d)",
		   (uint)jakeAnim.size(), kJakeX, kJakeY,
		   (uint)jennyAnim.size(), kJennyX, kJennyY);

	uint32 lastTick = g_system->getMillis();
	while (!shouldQuit()) {
		// 100ms tick: matches _CheckFrameRate's ~10 fps cadence. The seq
		// is short and loops; _UpdateAnimations restarts at curIdx=0 on
		// the 0x80 marker. _DoHappiness rewrites curIdx=0xFFFF on zone
		// change (here we restart seqIdx).
		if (g_system->getMillis() - lastTick > 100) {
			lastTick = g_system->getMillis();
			seqIdx = (seqIdx + 1) % 9;
			blitAt(background, 0, 0);
			blitAt(jennyAnim[kJennySeqs[level][seqIdx % 9] % jennyAnim.size()],
				   kJennyX, kJennyY);
			blitAt(jakeAnim [kJakeSeqs [level][seqIdx % 9] % jakeAnim.size()],
				   kJakeX, kJakeY);
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
					blitAt(jennyAnim[kJennySeqs[level][seqIdx % 9] % jennyAnim.size()],
						   kJennyX, kJennyY);
					blitAt(jakeAnim [kJakeSeqs [level][seqIdx % 9] % jakeAnim.size()],
						   kJakeX, kJakeY);
					g_system->updateScreen();
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				_partner = (ev.mouse.x >= 160) ? kPartnerJake : kPartnerJenny;
				debugC(1, kDebugGeneral, "Partner picked: %s",
					   _partner == kPartnerJake ? "Jake" : "Jennifer");
				done = true;
				break;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					_partner = kPartnerJenny; done = true; break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					_partner = kPartnerJake; done = true; break;
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

	// Partner intro VOC (jake.voc / jen.voc @ 29be:0af9 / 29be:0af1),
	// tail of _DoChoosePartner @ 1a35:097f.
	if (_audio) {
		if (isFloppy()) {
			// Floppy _DoChoosePartner_Floppy @ 19bb:0a8e calls
			// _LoadSoundName_Floppy(0x14) (slot 20); per-partner tables at
			// 2608:0f0e / 2608:0f76 resolve to m-0113sl.voc (Jake) or
			// f-0140sl.voc (Jenny).
			_audio->playFloppyVoiceSlot(0x14, _partner);
		} else {
			_audio->playVoc(Common::Path(
				(_partner == kPartnerJake) ? "JAKE.VOC" : "JEN.VOC"));
		}
		_audio->waitForVoiceDone();
	}
}

// _DoInitClues @ 1a35:0411. Sequence:
//   1. BG 0x52, palette 0x22
//   2. anims: game @ (0xcd, 0x6c), book @ (0, 99), nancy @ (0x68, 0x8b) on caseType=1
//   3. cycle game anim once (click skips)
//   4. _PlayInSequence per partner+caseType
//   5. _DisplayClue(InitBlock + 2, 1) — briefing dialogue
//   6. _OnSites[startSite] = 1
// Anim IDs: gameAni = 0x17 (Jake) / 0x3b (Jenny);
// bookAni = 0x18 / 0x3c; nancyAni = 0x19 (caseType 1 only).
void EEMEngine::doInitClues() {
	if (!_mystery.isLoaded())
		return;

	const byte *ib = _mystery.initBlock();
	if (!ib)
		return;

	// CD InitBlock: u16 caseType; u16 startSite; <clue block>.
	// Floppy InitBlock (FUN_19bb_042f): u8 caseType; u8 nSubjects;
	// subjects[]; u8 nDialog; dialog_records[]. No startSite.
	const bool floppy = isFloppy();
	const uint16 caseType = floppy ? (uint16)ib[0] : READ_LE_UINT16(ib);

	if (!floppy) {
		const uint16 startSite = READ_LE_UINT16(ib + 2);
		if (startSite < Mystery::kVisitedSiteCap)
			_mystery._onSites[startSite] = 1;
		_mystery._siteNumber = startSite;
		_mystery._lastSite = startSite;
	} else {
		// Floppy _DoMapScreen @ 1fed:1060 (FUN_1fed_07ed) walks every
		// site unconditionally — mirror that by marking all visible.
		const uint sites = _mystery.numSites();
		for (uint s = 0; s < sites && s < Mystery::kVisitedSiteCap; s++)
			_mystery._onSites[s] = 1;
		_mystery._siteNumber = 0;
		_mystery._lastSite = 0;
	}

	// Case-briefing palette. EEM1 `_DoInitClues` uses SITEPALS index 0x22;
	// EEM2 `_DoInitClues` @ 1abf:03b3 does `_GetBackground(0x52)` then
	// `_GetPalette(0x39)` (its 63-entry SITEPALS. shifts the UI palettes).
	// The briefing partner animation is an ANI sprite drawn under the screen
	// palette, so the same index fixes both the background and the anim.
	setSitePalette(isLondon() ? 0x39 : 0x22);
	Picture bg;
	const bool haveBriefingBg = _picsArchive.getPicture(0x52, bg);
	if (haveBriefingBg)
		blitAt(bg, 0, 0);

	const uint gameAni = _partner == kPartnerJake ? 0x17 : 0x3b;
	const uint bookAni = _partner == kPartnerJake ? 0x18 : 0x3c;
	Animation game, book, nancy;
	const bool haveGame  = _aniArchive.loadAnimation(gameAni, game) && !game.empty();
	const bool haveBook  = _aniArchive.loadAnimation(bookAni, book) && !book.empty();

	const bool haveNancy = (caseType == 1)
						  && _aniArchive.loadAnimation(0x19, nancy)
						  && !nancy.empty();

	// Cycle the game animation once at _CheckFrameRate cadence (~7 fps, 140 ms
	// per tick; _InitFrameCounter @ 1a35:01ae). The original loop @ 1a35:0507
	// makes `gameNum - 1` _UpdateAnimations calls, each advancing every
	// registered slot by one script tick. _DoInitClues @ 1a35:0507/0541
	// hard-codes Jake's script IDs (0x17/0x18/0x19) regardless of partner, so
	// we look up scripts by those IDs unconditionally.
	if (haveGame || haveBook || haveNancy) {
		const uint kCheckFrameRateMs = 140;
		const uint baseFrames = haveGame ? game.size() : 8;
		// `gameNum - 1` ticks: scriptIdx 0..gameNum-2.
		const uint frameCount = (baseFrames > 0) ? baseFrames - 1 : 0;
		bool skip = false;
		for (uint frame = 0; frame < frameCount && !shouldQuit() && !skip; frame++) {
			if (haveBriefingBg)
				blitAt(bg, 0, 0);
			const uint32 t = frame * kCheckFrameRateMs;
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

			// ESC interrupts voice/spool so audio doesn't bleed into the MAP.
			const uint32 wakeup = g_system->getMillis() + kCheckFrameRateMs;
			while (g_system->getMillis() < wakeup && !shouldQuit() && !skip) {
				Common::Event ev;
				while (g_system->getEventManager()->pollEvent(ev)) {
					if (ev.type == Common::EVENT_KEYDOWN &&
						ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
						interruptAudio(/* stopMusicToo= */ false);
						skip = true;
						break;
					}
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

	// _VidramRectCopy(0, 0x5a, 0x28, 0x6d, 16000, 48000/32000): freeze the
	// lower-left book/Nancy band the original bakes into BG buffers before
	// clearing registered animations. Width is in mode-X cols (0x28 = 160px).
	// Intentionally drops the right-side game anim; _PlayInSequence redraws
	// that character over a clean BG next.
	Graphics::ManagedSurface briefingBase(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	briefingBase.clear();
	if (haveBriefingBg) {
		briefingBase.simpleBlitFrom(bg.surface);
	} else {
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			briefingBase.simpleBlitFrom(*screen);
			g_system->unlockScreen();
		}
	}
	{
		const int preserveX = 0;
		const int preserveY = 0x5a;
		const int preserveW = 0x28 * 4;
		const int preserveH = 0x6d;
		const Common::Rect preserveRect(preserveX, preserveY,
										preserveX + preserveW,
										preserveY + preserveH);
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			briefingBase.simpleBlitFrom(*screen, preserveRect,
										Common::Point(preserveX, preserveY));
			g_system->unlockScreen();
		}
	}

	// Setup voice — _DoInitClues @ 1a35:061d runs this BEFORE _PlayInSequence
	// (i.e. the phone rings, the player hears it ring out, and only then
	// does Jake/Jenny reach for the receiver):
	//   CD:     caseType 2 -> PHONE.VOC then _WaitForVoiceDone
	//   Floppy (_DoInitClues_Floppy @ 19bb:042f):
	//     caseType 2 -> slot 0xc (PHONESL.VOC)
	//     caseType 3 -> slot 3   (NEWSCAN.VOC, news-anchor variant)
	// While the voice plays the screen continues showing the game anim's
	// last frame (saved into briefingBase above).
	if (_audio) {
		if (caseType == 2) {
			if (floppy)
				_audio->playFloppyVoiceSlot(0x0c, _partner);
			else
				_audio->playVoc(Common::Path("PHONE.VOC"));
			_audio->waitForVoiceDone();
		} else if (caseType == 3 && floppy) {
			_audio->playFloppyVoiceSlot(0x03, _partner);
			_audio->waitForVoiceDone();
		}
	}

	// _PlayInSequence @ 172b:2d03. Anim selection per partner + caseType:
	//   Jake:  caseType 1 -> 0x38 @ (0xcd, 0x6d)
	//          caseType 2 -> 0x37 @ (0xcd, 0x6c)
	//          caseType 3 -> 0x39 @ (0xcd, 0x6c)
	//   Jenny: caseType 2 -> 0x3a @ (0xcd, 0x6c)
	//          caseType 3 -> 0x3d @ (0xcd, 0x6c)
	uint16 seqAni = 0xFFFF;
	uint16 seqY   = 0x6c;
	if (_partner == kPartnerJake) {
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
				g_system->copyRectToScreen(briefingBase.getPixels(),
										   briefingBase.pitch, 0, 0,
										   kScreenWidth, kScreenHeight);
				// _PlayInSequence @ 172b:2d35-2d50:
				//   dstX = sx - cell[+0x8]   ; signed X anchor (miscflags)
				//   dstY = sy - cell[+0x6]   ; signed Y anchor (rowoff)
				// asm `SUB AX, ES:[BX+0x8]` — NOT the cell width.
				const int dstX = (int)0xcd - (int)(int16)fr.miscflags;
				const int dstY = (int)seqY - (int)(int16)fr.rowoff;
				blitMaskedToScreen(fr, dstX, dstY);
				g_system->updateScreen();
				const uint32 wakeup = g_system->getMillis() + 100;
				while (g_system->getMillis() < wakeup &&
					   !shouldQuit() && !skip) {
					Common::Event ev;
					while (g_system->getEventManager()->pollEvent(ev)) {
						if (ev.type == Common::EVENT_KEYDOWN &&
							ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
							interruptAudio(/* stopMusicToo= */ false);
							skip = true;
							break;
						}
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

	// Briefing dialogue. CD: clue block @ ib+4 (after caseType,startSite).
	// Floppy: dialog records dispatched via FUN_22dc_05c8 @ 22dc:05c8
	// (record size = 11 + textCount bytes).
	if (floppy) {
		displayFloppyBriefing(ib);
	} else {
		const byte *briefingClues = ib + 4;
		// _DisplayClue calls _AddNotebook for each ClueEntry note list at
		// +0x30..+0x39. Mark starting notes before the first PDA visit.
		const uint marked = markClueBlockNotebookEntries(_mystery, briefingClues);
		if (marked != 0)
			debugC(1, kDebugScript,
				   "doInitClues: marked %u CD briefing notebook entries",
				   marked);
		displayClue(briefingClues);
	}
}

// _ParseString @ 1b66:07c3 (jump table @ 1b66:0cbe). Each handler reads
// _Partner (u16 @ 0x7918) and indexes the name table @ 29be:0c28
// ({Jake, Jennifer, he, she, him, her, his} as far pointers).
//   0x80 player name (auto-cap word starts, uses _PlayerRecord)
//   0x81 _Partner == 0 ? "Jake"     : "Jennifer"  (chosen detective)
//   0x82 _Partner == 0 ? "Jennifer" : "Jake"      (the OTHER one)
//   0x83 _Partner == 0 ? "he"       : "she"
//   0x84 _Partner == 0 ? "him"      : "her"
//   0x85 _Partner == 0 ? "his"      : "her"
//   0x86..0x88 mirror 0x83..0x85 but branch on a separate gender flag
//     (DAT_29be_7985, handlers @ 1b66:0ad2/0b41/0bb0). That flag has no
//     writers anywhere in EEMCD.EXE — only the three handlers read it —
//     and no shipping mystery text (CD or floppy) contains the 0x86/0x87
//     /0x88 bytes inside parseString-formatted strings. The opcodes are
//     dead in the original engine and dead in our data, so we drop them
//     silently (which matches the always-0 flag path in DOS: he/him/his).
//   0x89 KD hint placeholder (caller handles).
Common::String EEMEngine::parseString(const Common::String &raw,
									  const Common::String &playerName,
									  uint partner) const {
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
			// Stubbed suspect-gender pronouns; the DOS flag they branch
			// on is never written and no mystery msg uses these bytes
			// (see jumptable comment above).
			break;
		case 0x89:
			// KD hint placeholder (caller handles before this point).
			break;
		case 0:
			return out;
		case '\r':
			break;
		case '^':
			// `_WordWrap @ 1b03:0456` treats `^` as a forced line
			// break (sets `cur_width = max_width`, forcing the next
			// loop turn to wrap at the previous space and skip the
			// `^` itself). Without this conversion the caret falls
			// through the default case and renders as a literal,
			// pushing the line past the balloon's visual edge — the
			// "bubbles aren't large enough" symptom. Promote it to
			// `\n` so `Font::wordWrapText` (which honours
			// embedded newlines) picks the same break point the
			// original engine did.
			out += '\n';
			break;
		default:
			out += (char)c;
			break;
		}
	}

	// Strip leading spaces at the start of each emitted line. Mirrors
	// `_DoWordWrap @ 1b66:04a7`, which advances past spaces at the
	// start of every output line via `for (; str[last] == ' '; last++)`.
	// ~60% of mystery-text strings carry 1-2 leading spaces in the data
	// of the original WordWrap
	// discards them, so we do the same before the text reaches
	// `Font::wordWrapText` (which only trims at wrap-induced line
	// boundaries, not at start-of-input or after an embedded '\n').
	Common::String cleaned;
	bool atLineStart = true;
	for (uint i = 0; i < out.size(); i++) {
		const char ch = out[i];
		if (atLineStart && ch == ' ')
			continue;
		cleaned += ch;
		atLineStart = (ch == '\n');
	}
	return cleaned;
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

// ClueBlock layout:
//   +0..1: number (entry count; 0 = no briefing)
//   +2..3: pic ID for entry 0; entry N>0 uses (entry-1).lastWord
//   +4..:  array of 62-byte entries
void EEMEngine::displayClue(const byte *clueBlock) {
	if (!clueBlock || !_mystery.isLoaded())
		return;

	const uint16 number = READ_LE_UINT16(clueBlock);
	debugC(1, kDebugScript, "displayClue: %u entries", number);
	// number == 0 = no briefing (e.g. mystery 0 case-type 4); >32 = bad ptr.
	if (number == 0 || number > 32)
		return;

	// Snapshot BG so per-entry character pics don't stack.
	Graphics::ManagedSurface bg(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	bg.clear();
	{
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			bg.simpleBlitFrom(*screen);
			g_system->unlockScreen();
		}
	}

	// ClueEntry layout (62 bytes):
	//   +0..1, +2..3:   p0 tx, ty       +4..5, +6..7:   p1 tx, ty
	//   +8..9, +10..11: bubText offset p0/p1 (rel. TextBlock; -1 = none)
	//   +12..13, +14..15: balloon pic ID p0/p1
	//   +0x10/+0x12: bubX, bubY (partner 0)
	//   +0x14/+0x16: bubX, bubY (partner 1)
	//   +0x18: Jenny voice (1-based)
	//   +0x1a: Jake  voice (1-based)
	//   +0x30..+0x39: 5 notebook entries (-1 terminated)
	//   +0x3a..+0x3b: KD-anim number (-1 = none)
	// _DisplayClue @ 2404:05e6: partner 1 uses its own field set only when
	// bubText1 != -1; otherwise falls back to partner 0 fields entirely.
	// Partner 0 always uses field 0.
	for (uint i = 0; i < number && !shouldQuit(); i++) {
		g_system->copyRectToScreen(bg.getPixels(), bg.pitch, 0, 0, kScreenWidth, kScreenHeight);
		const byte *c = clueBlock + 4 + i * 62;

		// _DisplayClue @ 2404:0635-064b: _DoKDAnim(num) runs before the
		// speaker portrait.
		const int16 kdAnimNum = (int16)READ_LE_UINT16(c + 0x3a);
		if (kdAnimNum != -1) {
			playKdAnim((uint16)kdAnimNum);
			// _UpdateAnimations @ 172b:09c1 reactivates the wait anim.
			g_system->copyRectToScreen(bg.getPixels(), bg.pitch,
									   0, 0, kScreenWidth, kScreenHeight);
		}

		const bool useP1 = (_partner == kPartnerJenny) &&
			(READ_LE_UINT16(c + 10) != 0xFFFF);
		const uint partner = useP1 ? 1 : 0;
		const uint16 textOff = READ_LE_UINT16(c + 8 + partner * 2);
		const bool hasText = (textOff != 0xFFFF);
		// Partner 1 bubX/bubY at +0x14/+0x16; partner 0 at +0x10/+0x12.
		const uint16 bubX = READ_LE_UINT16(c + (useP1 ? 0x14 : 0x10));
		const uint16 bubY = READ_LE_UINT16(c + (useP1 ? 0x16 : 0x12));
		const uint16 bubNum = READ_LE_UINT16(c + (useP1 ? 0x0E : 0x0C));
		const char *raw   = hasText ? _mystery.textAt(textOff) : "";

		// Speaker portrait: pic[clues + i*62 - 2]. Entry 0 ID is in
		// ClueBlock +2; entries N>0 read (entry-1)+0x3c (last word).
		const uint16 charX  = READ_LE_UINT16(c + (useP1 ? 4 : 0));
		const uint16 charY  = READ_LE_UINT16(c + (useP1 ? 6 : 2));
		const uint16 charPicId = (i == 0)
			? READ_LE_UINT16(clueBlock + 2)
			: READ_LE_UINT16(c - 2);
		if (charPicId != 0 && charPicId != 0xFFFF) {
			Picture charPic;
			if (_picsArchive.getPicture(charPicId, charPic) &&
				charX < kScreenWidth && charY < kScreenHeight) {
				blitMaskedToScreen(charPic, charX, charY);
			}
		}

		const Common::String text = parseString(raw ? raw : "",
												_playerName, _partner);

		// Speech balloon: _GetBalloon + _AddPicBackground.
		const uint16 fittedBubNum = fitBalloonToText(bubNum, text);
		Picture balloon;
		const uint16 balloonId = fittedBubNum & 0x7F;
		const bool haveBalloon = bubNum != 0xFFFF &&
			_balloonArchive.size() > balloonId &&
			_balloonArchive.loadEntry(balloonId, balloon);

		if (_font.isLoaded() && !text.empty()) {
			Graphics::Surface *screen = g_system->lockScreen();
			if (!screen)
				break;
			Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.simpleBlitFrom(*screen);
			g_system->unlockScreen();

			int textX = bubX;
			int textY = bubY;
			int textW = MIN<int>(kScreenWidth - bubX, 200);
			int copyY = bubY;
			int copyH = _font.getFontHeight() * 4 + 8;

			if (haveBalloon) {
				const int bw = MIN<int>(balloon.surface.w, kScreenWidth - bubX);
				const int bh = MIN<int>(balloon.surface.h, kScreenHeight - bubY);
				// _AddPicBackground: transparent colour = pic->miscflags >> 8.
				const byte transp = (byte)(balloon.flags >> 8);
				// _GetBalloon @ 172b:1d7d mirrors horizontally when
				// (bubNum & 0x80) — for right-side speakers.
				const bool flipBalloon = (fittedBubNum & 0x80) != 0;
				if (bw > 0 && bh > 0) {
					scratch.transBlitFrom(balloon.surface,
										  Common::Point(bubX, bubY),
										  transp, flipBalloon);
				}
				// Per-balloon insets at 29be:0875 (52 x 10 bytes,
				// indexed by bubNum & 0x7F). _DisplayClue does
				// _WordWrap(bubX + table[bub].x, bubY + table[bub].y,
				//           table[bub].w, ...).
				// Fallback (5, 4, 155) = entry-23 inset.
				uint16 insetX = 5;
				uint16 insetY = 4;
				uint16 insetW = 155;
				getBalloonInsets(balloonId, insetX, insetY, insetW);
				textX = bubX + insetX;
				textY = bubY + insetY;
				textW = insetW;
				copyH = bh;
			} else {
				// No balloon: clear band.
				const Common::Rect band(0, bubY, kScreenWidth,
					MIN<int>(bubY + copyH, kScreenHeight));
				scratch.fillRect(band, 0);
				copyY = bubY;
			}

			// _DisplayClue @ 2404:07fe passes fontColor=0 (palette
			// index 0 of case-briefing palette 0x22).
			_font.drawWordWrapped(&scratch, textX, textY,
				MAX<int>(8, textW), text, 0);

			g_system->copyRectToScreen(scratch.getBasePtr(0, copyY),
				scratch.pitch, 0, copyY, kScreenWidth,
				MIN<int>(copyH, kScreenHeight - copyY));
			g_system->updateScreen();
		}

		// _DisplayClue @ 2404:0833-085a — per-clue voice gate. The gate is on
		// the Jenny slot regardless of partner; entries with +0x18 == 0 but
		// +0x1a set are text-only.
		if (_audio) {
			const uint16 voiceJenny = READ_LE_UINT16(c + 0x18);
			if (voiceJenny != 0 && voiceJenny != 0xFFFF) {
				const uint16 voiceJake = READ_LE_UINT16(c + 0x1a);
				const uint16 voice = (_partner == kPartnerJake) ? voiceJake : voiceJenny;
				if (voice != 0 && voice != 0xFFFF)
					_audio->spoolSound((uint)(voice - 1));
			}
		}

		// Wait for click/key. ESC skips entire dialog; Return / KP-Enter /
		// Space advance one entry.
		if (hasText || (charPicId != 0 && charPicId != 0xFFFF)) {
			// Clear any leftover highlighted-cursor state from the site loop.
			setInteractiveMouseCursor(false);
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
					if (ev.type == Common::EVENT_MOUSEMOVE) {
						setInteractiveMouseCursor(false);
						continue;
					}
					if (ev.type == Common::EVENT_KEYDOWN &&
						ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
						advance = true;
						skipAll = true;
						// Cut voice + spool only (keep MIDI).
						interruptAudio(/* stopMusicToo= */ false);
						break;
					}
					if (ev.type == Common::EVENT_LBUTTONDOWN) {
						advance = true;
						break;
					}
					if (ev.type == Common::EVENT_KEYDOWN &&
						(ev.kbd.keycode == Common::KEYCODE_RETURN ||
						 ev.kbd.keycode == Common::KEYCODE_KP_ENTER ||
						 ev.kbd.keycode == Common::KEYCODE_SPACE)) {
						advance = true;
						break;
					}
				}
				g_system->updateScreen();
				g_system->delayMillis(10);
			}
			if (skipAll) {
				// Apply remaining side-effects without rendering.
				for (uint k = i; k < number; k++)
					applyClueSideEffects(clueBlock + 4 + k * 62);
				return;
			}
		}

		applyClueSideEffects(c);
	}

	// _StopTheVoice @ 1ff1:0283 effect (voice only, keep MIDI). Diverges
	// from _DisplayClue @ 2404:05e6 which lets voice bleed past dismissal.
	interruptAudio(/* stopMusicToo= */ false);
}

bool EEMEngine::floppyDialogWaitForClick() {
	// Drain pending events so a stale keystroke doesn't auto-advance.
	Common::Event drain;
	while (g_system->getEventManager()->pollEvent(drain)) {}
	setInteractiveMouseCursor(false);
	const uint32 minVisibleMs = 250;
	const uint32 startedAt = g_system->getMillis();
	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return true;  // skip
			if (ev.type == Common::EVENT_MOUSEMOVE) {
				setInteractiveMouseCursor(false);
				continue;
			}
			if (g_system->getMillis() - startedAt < minVisibleMs)
				continue;
			if (ev.type == Common::EVENT_KEYDOWN &&
				ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
				interruptAudio(/* stopMusicToo= */ false);
				return true;  // skip
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN ||
				ev.type == Common::EVENT_KEYDOWN)
				return false;  // advance one page
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	return true;
}

void EEMEngine::displayFloppyDialogRecords(const byte *rec, uint count,
											uint lastIndicator) {
	// Render `count` consecutive floppy dialog records starting at
	// `rec`. Per `FUN_22dc_05c8 @ 22dc:05c8`, each record is:
	//   u16 picID    @ +0     (character portrait, 0 = skip pic)
	//   u16 picX     @ +2
	//   u8  picY     @ +4
	//   u8  balloon  @ +5     (low 7 bits = balloon idx, +0x80 = mirror)
	//   u16 ballX    @ +6
	//   u8  ballY    @ +8
	//   u8  sound    @ +9     (high bit = play voice, low 7 bits = slot)
	//   u8  textCount@ +10
	//   u8  textIdx[]@ +11    (1 byte per — low 7 bits = NOTES idx)
	// NOTES text offsets are absolute byte offsets into the mystery
	// buffer; read via mystery.blobAt(noteEntry[+2..3]) for Jake,
	// +4..5 for Jenny.
	if (!rec || !isFloppy() || !_font.isLoaded() || count == 0)
		return;

	const byte *notes   = _mystery.noteIndex();
	const byte *bufBase = _mystery.blobAt(0);
	if (!notes || !bufBase)
		return;

	// Snapshot BG for between-bubble restores.
	Graphics::ManagedSurface bg(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	{
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			bg.simpleBlitFrom(*screen);
			g_system->unlockScreen();
		}
	}

	const uint32 dsz       = _mystery.dataSize();
	const uint32 notesBase = (uint32)(notes - bufBase);

	// _DisplayHotspotClue_Floppy @ 22dc:05c8 writes _TextSeen_Floppy[idx]=1
	// inline per text. Pre-mark all up front so ESC-fast-forward still
	// records every clue in the notebook.
	{
		const byte *r = rec;
		for (uint i = 0; i < count; i++) {
			const uint8 tc = r[10];
			for (uint t = 0; t < tc; t++) {
				const uint8 idx = r[11 + t] & 0x7f;
				if (idx < Mystery::kCluesFoundCap)
					_mystery._cluesFound[idx] = 1;
			}
			r += 11 + tc;
		}
	}

	for (uint i = 0; i < count && !shouldQuit(); i++) {
		const uint16 picID    = READ_LE_UINT16(rec + 0);
		const uint16 picX     = READ_LE_UINT16(rec + 2);
		const uint8  picY     = rec[4];
		const uint8  balByte  = rec[5];
		const uint16 ballX    = READ_LE_UINT16(rec + 6);
		const uint8  ballY    = rec[8];
		const uint8  textCount= rec[10];

		// byte 9: high bit = play voice slot.
		const bool playedRecordVoice = (rec[9] & 0x80) != 0 && _audio;
		if (playedRecordVoice) {
			const uint slot = rec[9] & 0x7f;
			_audio->playFloppyVoiceSlot(slot, _partner);
		}
		// _DisplayHotspotClue_Floppy @ 22dc:0908..0922 suspect-found:
		//   if ((rec[9] & 0x80) == 0 && rec[9] != 0)
		//     _InGallery_Floppy[_GalleryShuffleSeed_Floppy[rec[9]]] = 1;
		// _GalleryShuffleSeed (DS:0x2d65) overlaps _NewOrder (DS:0x2d66)
		// by one byte: GalleryShuffleSeed[i+1] == NewOrder[i]. So:
		// _inGallery[_newOrder[b9 - 1]].
		const uint8 b9 = rec[9];
		if ((b9 & 0x80) == 0 && b9 != 0) {
			const uint logicalIdx = (uint)b9 - 1;
			if (logicalIdx < Mystery::kGalleryCap) {
				const uint8 slot = _mystery._newOrder[logicalIdx];
				if (slot < Mystery::kGalleryCap)
					_mystery._inGallery[slot] = 1;
			}
		}

		Common::String fitText;
		Common::String fitPage;
		uint16 fitXIns = 0;
		uint16 fitYIns = 0;
		uint16 fitWidth = 142;
		getBalloonInsets(balByte, fitXIns, fitYIns, fitWidth);
		uint fitTextLines = 0;
		for (uint t = 0; t < textCount; t++) {
			const uint8 idxByte = rec[11 + t];
			const uint8 idx = idxByte & 0x7f;
			const uint32 noteAbs = notesBase + (uint32)idx * 7;
			if (noteAbs + 6 > dsz)
				continue;
			const uint16 textOff = (_partner == kPartnerJake)
				? READ_LE_UINT16(notes + idx * 7 + 2)
				: READ_LE_UINT16(notes + idx * 7 + 4);
			if (textOff >= dsz)
				continue;
			const char *linePtr = (const char *)(bufBase + textOff);
			uint32 lineLen = 0;
			while (textOff + lineLen < dsz && linePtr[lineLen] != 0)
				lineLen++;
			Common::String raw(linePtr, lineLen);
			Common::String parsed = parseString(raw, _playerName, _partner);
			if (!parsed.empty()) {
				if (!fitPage.empty())
					fitPage += '\n';
				fitPage += parsed;
			}
			const bool continuePage =
				(idxByte & 0x80) != 0 && t + 1 < textCount;
			if (!continuePage) {
				Common::Array<Common::String> wrapped;
				_font.wordWrapText(fitPage, MAX<int>(8, (int)fitWidth),
					wrapped);
				if (wrapped.size() > fitTextLines ||
					(wrapped.size() == fitTextLines &&
					 fitPage.size() > fitText.size())) {
					fitText = fitPage;
					fitTextLines = wrapped.size();
				}
				fitPage.clear();
			}
		}

		// Pre-load balloon picture + insets once per record (constant
		// across all paginated text indices).
		const uint16 fittedBalByte = fitBalloonToText(balByte, fitText);
		Picture balloon;
		const uint16 balloonId  = fittedBalByte & 0x7F;
		const bool   flipBall   = (fittedBalByte & 0x80) != 0;
		const bool   haveBalloon = balByte != 0xFF &&
			_balloonArchive.size() > balloonId &&
			_balloonArchive.loadEntry(balloonId, balloon);
		uint16 textWidth = 142;
		uint16 textXIns  = 6;
		uint16 textYIns  = 4;
		if (haveBalloon)
			getBalloonInsets(balloonId, textXIns, textYIns, textWidth);
		const int textX = ballX + textXIns;
		const int lineH    = _font.getFontHeight();

		// FUN_22dc_05c8 pagination via local_1c (set from previous text's
		// flag bit): 0 = fresh page (redraw balloon, Y at top),
		// 1 = continuation (append below previous lines).
		bool firstPage  = true;
		int  cursorY    = ballY + textYIns;
		bool skipAll    = false;

		for (uint t = 0; t < textCount && !shouldQuit() && !skipAll; t++) {
			const uint8 idxByte = rec[11 + t];
			const uint8 idx     = idxByte & 0x7f;
			if (idx < Mystery::kCluesFoundCap)
				_mystery._cluesFound[idx] = 1;
			const uint32 noteAbs = notesBase + (uint32)idx * 7;
			if (noteAbs + 6 > dsz)
				break;
			const uint16 textOff = (_partner == kPartnerJake)
				? READ_LE_UINT16(notes + idx * 7 + 2)
				: READ_LE_UINT16(notes + idx * 7 + 4);
			if (textOff >= dsz)
				break;
			const char *linePtr = (const char *)(bufBase + textOff);
			uint32 lineLen = 0;
			while (textOff + lineLen < dsz && linePtr[lineLen] != 0)
				lineLen++;
			Common::String raw(linePtr, lineLen);
			const Common::String text =
				parseString(raw, _playerName, _partner);

			// Render this text page.
			Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.simpleBlitFrom(*bg.surfacePtr());

			if (firstPage) {
				// Original only redraws balloon + portrait on local_1c == 0.
				if (picID != 0 && picID != 0xFFFF) {
					Picture pic;
					if (_picsArchive.getPicture(picID, pic)) {
						scratch.transBlitFrom(pic.surface,
											  Common::Point(picX, picY),
											  (uint32)(byte)(pic.flags >> 8));
					}
				}
				if (haveBalloon) {
					const byte transp = (byte)(balloon.flags >> 8);
					scratch.transBlitFrom(balloon.surface,
										  Common::Point(ballX, ballY),
										  transp, flipBall);
				}
				cursorY = ballY + textYIns;
			}

			Common::Array<Common::String> lines;
			_font.wordWrapText(text, MAX<int>(8, (int)textWidth), lines);
			for (uint l = 0; l < lines.size(); l++) {
				_font.drawString(&scratch, lines[l], textX,
								  cursorY + (int)l * lineH,
								  MAX<int>(8, (int)textWidth), 0);
			}
			cursorY += (int)lines.size() * lineH;

			// Pagination for next text idx is driven by this text's high bit.
			const bool textHighBit = (idxByte & 0x80) != 0;
			const bool isLastText  = (t + 1 == textCount);
			const bool isLastRec   = (i + 1 == count);

			// "Click to continue" indicator: PIC 0xa0 ("more" arrow) or
			// PIC 0xa1 (end). _DisplayHotspotClue_Floppy @ 22dc:08aa
			// (mid-page) / @ 22dc:08c0 (end-of-record). lastIndicator == 0
			// matches the original param_2 == 0 "no indicator" case.
			bool waitNeeded   = false;
			bool drawArrow    = false;
			bool useEndPic    = false;
			if (!isLastText) {
				if (textHighBit) {
					firstPage = false;
				} else {
					waitNeeded = true;
					drawArrow  = true;          // PIC 0xa0
					useEndPic  = false;
				}
			} else {
				waitNeeded = true;
				if (!isLastRec) {
					// More records follow → param_2 = 1 → PIC 0xa0.
					drawArrow = true;
					useEndPic = false;
				} else {
					if (lastIndicator == 1) {
						drawArrow = true;
						useEndPic = false;
					} else if (lastIndicator == 2) {
						drawArrow = true;
						useEndPic = true;       // PIC 0xa1
					}
				}
			}

			if (drawArrow) {
				drawFloppyBubbleIndicator(scratch, balloonId, ballX,
										   ballY, useEndPic);
			}

			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, kScreenWidth, kScreenHeight);
			g_system->updateScreen();

			if (waitNeeded) {
				if (floppyDialogWaitForClick()) {
					skipAll = true;
					break;
				}
				if (!isLastText && !textHighBit)
					firstPage = true;
			}
		}
		if (skipAll) {
			if (playedRecordVoice)
				_audio->stopVoice();
			return;
		}

		if (playedRecordVoice)
			_audio->stopVoice();

		rec += 11 + textCount;
	}
}

void EEMEngine::displayFloppyBriefing(const byte *initBlock) {
	// FUN_19bb_042f @ 19bb:042f: walks nDialog = ib[2 + nSubjects] records
	// starting at ib + 3 + nSubjects, dispatching each to FUN_22dc_05c8.
	if (!initBlock || !isFloppy())
		return;
	const uint8 nSubjects = initBlock[1];
	const uint8 nDialog   = initBlock[2 + nSubjects];
	const byte *rec       = initBlock + 3 + nSubjects;
	displayFloppyDialogRecords(rec, nDialog);
}

void EEMEngine::displayFloppyHotspotDialog(uint siteNum, uint hotIdx) {
	// FUN_22dc_0b80 @ 22dc:0b80 + FUN_1652_00e6 @ 1652:00e6 +
	// FUN_1652_006c @ 1652:006c. site_data[+6..7] -> per-hotspot list:
	//   for each hotspot:
	//     main record (11 + textCount bytes)
	//     u8 contFlags  (low 7 = cont count, high bit = partner pose)
	//     contCount x { record (11 + textCount bytes) }
	if (!_mystery.isLoaded() || !isFloppy())
		return;
	const byte *site = _mystery.siteData(siteNum);
	if (!site)
		return;
	const uint16 dlgListOff = READ_LE_UINT16(site + 6);
	const byte *bufBase = _mystery.blobAt(0);
	if (!bufBase || dlgListOff == 0 || dlgListOff >= _mystery.dataSize())
		return;
	uint32 off = dlgListOff;
	for (uint h = 0; h < hotIdx; h++) {
		const byte *rec = bufBase + off;
		off += 11 + rec[10];
		const uint contCount = bufBase[off] & 0x7F;
		off += 1;
		for (uint c = 0; c < contCount; c++) {
			const byte *cr = bufBase + off;
			off += 11 + cr[10];
		}
	}
	if (off >= _mystery.dataSize())
		return;
	// Snapshot clean site BG and restore between main + continuation calls
	// so each displayFloppyDialogRecords sees a bubble-free background.
	Graphics::ManagedSurface siteBG(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	{
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			siteBG.simpleBlitFrom(*screen);
			g_system->unlockScreen();
		}
	}
	const byte *mainRec = bufBase + off;
	const uint mainLen = 11u + (uint)mainRec[10];
	uint contCount = 0;
	uint contFlagsByte = 0;
	if (off + mainLen < _mystery.dataSize()) {
		contFlagsByte = bufBase[off + mainLen];
		contCount = contFlagsByte & 0x7F;
	}
	// _HandleHotspotClick_Floppy @ 1652:00e6 derives param_2:
	//   contFlagsByte == 0  -> 0 (no indicator)
	//   high bit set        -> 1 (PIC 0xa0, "more")
	//   low 7 bits non-zero -> 2 (PIC 0xa1, alt end)
	uint mainIndicator = 0;
	if (contFlagsByte != 0) {
		mainIndicator = (contFlagsByte & 0x80) ? 1 : 2;
	}
	displayFloppyDialogRecords(mainRec, 1, mainIndicator);
	if (contCount == 0)
		return;
	const uint32 contOff = off + mainLen + 1;
	if (contOff >= _mystery.dataSize())
		return;
	// Wipe main bubble so the continuation chain snapshots a clean BG.
	g_system->copyRectToScreen(siteBG.getPixels(), siteBG.pitch,
							   0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();
	// _DisplayDialogContinuations_Floppy @ 1652:006c: lastIndicator=0
	// means no indicator on the final continuation.
	displayFloppyDialogRecords(bufBase + contOff, contCount, 0);
}

bool EEMEngine::areYouSure() {
	Graphics::Surface *screen = g_system->lockScreen();
	Graphics::ManagedSurface saved(kScreenWidth, kScreenHeight,
		Graphics::PixelFormat::createFormatCLUT8());
	if (screen) {
		saved.simpleBlitFrom(*screen);
		g_system->unlockScreen();
	}

	// _AreYouSure @ 1a35:0a5c (CD) / FUN_19bb_0b43 (floppy):
	//   PIC 0x136 = dialog body; PIC 0x1fd/0x1fe = YES/NO pressed buttons.
	//   YES hit: (x+0x0c, y+0x23) .. (x+0x20, y+0x32)
	//   NO  hit: (x+0x60, y+0x23) .. (x+0x74, y+0x32)
	Picture dialogPic;
	Picture yesPic;
	Picture noPic;
	const bool haveOriginalDialog =
		_picsArchive.getPicture(0x136, dialogPic) &&
		_picsArchive.getPicture(0x1fd, yesPic) &&
		_picsArchive.getPicture(0x1fe, noPic);

	Common::Rect yesRect;
	Common::Rect noRect;
	int yesX = 0;
	int yesY = 0;
	int noX = 0;
	int noY = 0;

	if (haveOriginalDialog) {
		const int x = (kScreenWidth - dialogPic.surface.w) / 2;
		const int y = (kScreenHeight - dialogPic.surface.h) / 2;
		yesX = x + 0x0c;
		yesY = y + 0x23;
		noX = x + 0x60;
		noY = y + 0x23;
		yesRect = Common::Rect(yesX, yesY, x + 0x20 + 1, y + 0x32 + 1);
		noRect = Common::Rect(noX, noY, x + 0x74 + 1, y + 0x32 + 1);
		blitMaskedToScreen(dialogPic, x, y);
	} else if (_font.isLoaded()) {
		const Common::Rect dlg(60, 70, 260, 140);
		yesRect = Common::Rect(dlg.left + 16, dlg.top + 34,
							   dlg.left + 84, dlg.top + 54);
		noRect = Common::Rect(dlg.left + 100, dlg.top + 34,
							  dlg.left + 160, dlg.top + 54);

		Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.simpleBlitFrom(saved);
		scratch.fillRect(dlg, 0);
		scratch.frameRect(dlg, 0xF);
		_font.drawString(&scratch,
			isSpanish() ? "Estas seguro que quieres salir?"
						: "Are you sure you want to quit?",
			dlg.left + 8, dlg.top + 8, kScreenWidth, 0xF);
		_font.drawString(&scratch,
			isSpanish() ? "S - Si" : "Y - Yes",
			dlg.left + 16, dlg.top + 36, kScreenWidth, 0xF);
		_font.drawString(&scratch, "N - No", dlg.left + 100,
						 dlg.top + 36, kScreenWidth, 0xF);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, kScreenWidth, kScreenHeight);
	} else {
		return true;
	}
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
				// Spanish prompt is "S - Si": accept Y and S.
				if (ev.kbd.keycode == Common::KEYCODE_y ||
					ev.kbd.keycode == Common::KEYCODE_s ||
					ev.kbd.keycode == Common::KEYCODE_RETURN) {
					result = true; decided = true; break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_n ||
					ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					result = false; decided = true; break;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				if (yesRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (haveOriginalDialog) {
						blitRawToScreen(yesPic, yesX, yesY);
						g_system->updateScreen();
						g_system->delayMillis(90);
					}
					result = true; decided = true; break;
				}
				if (noRect.contains(ev.mouse.x, ev.mouse.y)) {
					if (haveOriginalDialog) {
						blitRawToScreen(noPic, noX, noY);
						g_system->updateScreen();
						g_system->delayMillis(90);
					}
					result = false; decided = true; break;
				}
			}
		}
		if (decided)
			break;
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	g_system->copyRectToScreen(saved.getPixels(), saved.pitch, 0, 0, kScreenWidth, kScreenHeight);
	g_system->updateScreen();
	return result;
}

} // End of namespace EEM
