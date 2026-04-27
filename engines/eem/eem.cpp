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
#include "common/debug-channels.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/path.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "engines/util.h"

#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/site.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "graphics/managed_surface.h"

namespace EEM {

namespace {
const uint kPalSize = 768;     ///< 256 colors * 3 bytes
const uint kNumSitePals = 40;  ///< SITEPALS holds 40 palettes (40 * 768 = 30720)

// Picture / palette IDs from the original code (1-based picture IDs).
const uint kPicEAKidsLogo      = 0x54;  ///< _ShowEAKids: GetPicture(0x54)
const uint kPicHighScoreLogo   = 0x20c; ///< _ShowHScoreLogo: GetPicture(0x20c)
const uint kPicChooseBackground = 0x8c; ///< _DoChoosePartner: GetBackground(0x8c)
const uint kPalEAKids          = 0x25;
const uint kPalHighScore       = 0x27;

// Animation IDs (0-based per ANI.DBX). _DoChoosePartner uses GetAnimation(8/9).
const uint kAniBoy  = 8;
const uint kAniGirl = 9;

// On-screen positions for the boy and girl partner sprites, from
// _DoChoosePartner: NewAnimation(0xe2, 0x62, ...) and (0x42, 0x60, ...).
const int kBoyX  = 0xe2; // 226
const int kBoyY  = 0x62; // 98
const int kGirlX = 0x42; // 66
const int kGirlY = 0x60; // 96

// 11x16 mouse cursor — replaces the DOS hardware cursor wired in by
// _InitMouse @ 152d:018b (INT 33h). The original game sets the cursor
// visible/hidden via _MouseCursor; we leave it on once the screens
// that need it (ChoosePartner, CaseSelection, sites) are reached.
//   0 = transparent, 1 = black outline, 2 = white fill
const byte kCursorBitmap[11 * 16] = {
	1,1,0,0,0,0,0,0,0,0,0,
	1,2,1,0,0,0,0,0,0,0,0,
	1,2,2,1,0,0,0,0,0,0,0,
	1,2,2,2,1,0,0,0,0,0,0,
	1,2,2,2,2,1,0,0,0,0,0,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,2,2,2,2,2,1,0,0,0,
	1,2,2,2,2,2,2,2,1,0,0,
	1,2,2,2,2,2,2,2,2,1,0,
	1,2,2,2,2,2,2,2,2,2,1,
	1,2,2,2,2,2,1,0,0,0,0,
	1,2,1,0,1,2,2,1,0,0,0,
	1,1,0,0,1,2,2,1,0,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,1,2,2,1,0,0,
	0,0,0,0,0,0,1,2,2,1,0
};
const byte kCursorPalette[] = {
	0x00, 0x00, 0x00, // 0 — transparent (key)
	0x00, 0x00, 0x00, // 1 — outline
	0xFF, 0xFF, 0xFF  // 2 — fill
};
} // anonymous namespace

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _rng("eem"),
	  _playerName("Detective"),
	  _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle), _partner(0) {
}

EEMEngine::~EEMEngine() {
}

Common::Error EEMEngine::run() {
	// _SetMode13X @ 1000:0358 enters VGA mode 13h (320x200x256).
	initGraphics(320, 200);

	if (!openArchives())
		return Common::Error(Common::kReadingFailed, "EEM archive open failed");

	if (!loadSitePalettes())
		return Common::Error(Common::kReadingFailed, "SITEPALS load failed");

	// _LoadFont @ 1b66:023c — main 8 px bitmap font.
	if (!_font.load(Common::Path("FONT.FNT")))
		warning("FONT.FNT failed to load; text will not render");

	// _InitMouse @ 152d:018b in the original — install our 11x16 arrow,
	// using palette index 0 as the transparency key. The cursor is left
	// hidden through the opening anims and switched on at NewPlayer /
	// ChoosePartner where the player actually clicks.
	CursorMan.replaceCursor(kCursorBitmap, 11, 16, 0, 0, 0);
	CursorMan.replaceCursorPalette(kCursorPalette, 0, 3);
	CursorMan.showMouse(false);

	// _AllBlack @ 172b:0d4b paints the screen black before the first handler.
	byte black[3 * 256] = { 0 };
	g_system->getPaletteManager()->setPalette(black, 0, 256);

	debugC(1, kDebugGeneral, "EEM engine starting");

	// If the user chose "Load" before pressing Play, the framework
	// invokes `loadGameState` which sets up `_mystery` and `_partner`.
	// Honour that by skipping the intros and going straight to the
	// loaded mystery's site loop.
	const int wantedSave = ConfMan.hasKey("save_slot")
		? ConfMan.getInt("save_slot") : -1;
	if (wantedSave >= 0) {
		const Common::Error err = loadGameState(wantedSave);
		if (err.getCode() == Common::kNoError && _mystery.isLoaded()) {
			debugC(1, kDebugGeneral, "Resuming from slot %d at mystery %u",
				   wantedSave, _mystery.number());
			CursorMan.showMouse(true);
			doInitClues();
			// Original screen 0 → screen 1: after the briefing the
			// game opens the map (function at 20fe:120b → _DoBigMap)
			// and only enters a site once the player clicks on one.
			doBigMap();
			if (_mystery.isLoaded())
				doSiteLoop();
			while (!shouldQuit()) {
				doCaseSelection();
				if (!_mystery.isLoaded()) break;
				doInitClues();
				doBigMap();
				if (_mystery.isLoaded())
					doSiteLoop();
			}
			return Common::kNoError;
		}
	}

	// Reproduces _DoOpeningAnims @ 2520:082a (sans audio):
	//   EA Kids logo (PIC) -> HighScore Productions logo (PIC) ->
	//   Storm Software logo (BOLT.ANM) -> 20 character-intro animations
	//   (ANIM01.A .. ANIM20.A) -> TITLE.ANM. Click / any key skips a
	//   single clip; ESC skips the rest of the chain (waitForInput /
	//   playAnm raise `_skipIntro` so each subsequent step bails out).
	_skipIntro = false;
	showEAKidsLogo();
	if (!shouldQuit() && !_skipIntro)
		showHighScoreLogo();
	if (!shouldQuit() && !_skipIntro)
		playAnm(Common::Path("BOLT.ANM"));
	for (int i = 1; i <= 20 && !shouldQuit() && !_skipIntro; i++) {
		Common::String name = Common::String::format("ANIM%02d.A", i);
		playAnm(Common::Path(name));
		// Between anims the original plays a voice clip via _SpoolSound;
		// without audio we still want a beat so each scene reads.
		if (!shouldQuit() && !_skipIntro && i != 20)
			waitForInput(2000);
	}
	if (!shouldQuit() && !_skipIntro)
		playAnm(Common::Path("TITLE.ANM"), 120, /*holdLastFrame=*/true);
	_skipIntro = false;

	// After the title chain, the original goes Title (B) -> screen 8
	// (NewPlayer / saved-record selection) -> screen 9 (ChoosePartner) ->
	// screen A (CaseSelection) -> site loop. We mirror the same order.
	// Mouse stays hidden through the opening anims; show it now for
	// the interactive screens (matches `_MouseCursor = 1` at the tail
	// of `_NewPlayer`).
	CursorMan.showMouse(true);
	if (!shouldQuit())
		doNewPlayer();
	if (!shouldQuit())
		doChoosePartner();
	if (!shouldQuit())
		doCaseSelection();
	if (!shouldQuit() && _mystery.isLoaded()) {
		// Mark the starting site as active and display the case briefing.
		// `_DoInitClues` @ 1a35:0411 — case briefing.
		doInitClues();
		// Original screen 0 → screen 1: after the briefing the game
		// opens the map (function at 20fe:120b → `_DoBigMap`) and only
		// enters a site once the player clicks on one.
		doBigMap();
		if (_mystery.isLoaded())
			doSiteLoop();

		// After a case, loop back to CaseSelection.
		while (!shouldQuit()) {
			doCaseSelection();
			if (!_mystery.isLoaded())
				break;
			doInitClues();
			// Original screen 0 → screen 1: after the briefing the
			// game opens the map (function at 20fe:120b → _DoBigMap)
			// and only enters a site once the player clicks on one.
			doBigMap();
			if (_mystery.isLoaded())
				doSiteLoop();
		}
	}

	debugC(1, kDebugGeneral, "EEM engine exiting");
	return Common::kNoError;
}

bool EEMEngine::openArchives() {
	// _InitGraphicsSystem @ 172b:0145 opens these five .DBD/.DBX pairs.
	if (!_picsArchive.open(Common::Path("PICS.DBD"), Common::Path("PICS.DBX"))) {
		warning("PICS archive missing");
		return false;
	}
	if (!_aniArchive.open(Common::Path("ANI.DBD"), Common::Path("ANI.DBX"))) {
		warning("ANI archive missing");
		return false;
	}
	// SITES + BALLOON are optional for the boot path but needed for site
	// rendering and clue display.
	if (!_sitesArchive.open(Common::Path("SITES.DBD"), Common::Path("SITES.DBX")))
		warning("SITES archive missing — site backgrounds disabled");
	if (!_balloonArchive.open(Common::Path("BALLOON.DBD"), Common::Path("BALLOON.DBX")))
		warning("BALLOON archive missing — clue text will lack balloons");
	// `_GetButton @ 172b:199d` reads from this archive (see strings
	// 'button.dbd' / 'Button.DBX' at 29be:06bf / 29be:04bb). Each
	// per-site map marker (used by `_StampButtons @ 20fe:0d2f` and
	// looked up via MapData[+0]) lives here.
	if (!_buttonArchive.open(Common::Path("BUTTON.DBD"), Common::Path("BUTTON.DBX")))
		warning("BUTTON archive missing — map markers will be unlabelled");
	return true;
}

bool EEMEngine::loadSitePalettes() {
	Common::File f;
	if (!f.open(Common::Path("SITEPALS"))) {
		warning("SITEPALS missing");
		return false;
	}
	_sitePals.resize(f.size());
	if (f.read(_sitePals.data(), _sitePals.size()) != _sitePals.size()) {
		warning("SITEPALS short read");
		return false;
	}
	debugC(1, kDebugGfx, "Loaded %u SITEPALS palettes",
		   (uint)(_sitePals.size() / kPalSize));
	return true;
}

void EEMEngine::setSitePalette(uint num) {
	if (num >= kNumSitePals || _sitePals.size() < (num + 1) * kPalSize) {
		warning("setSitePalette: index %u out of range", num);
		return;
	}
	// SITEPALS stores 6-bit VGA-DAC values (0..63); ScummVM expects 8-bit
	// (0..255), so left-shift by 2 like the original VGA hardware did.
	const byte *src = _sitePals.data() + num * kPalSize;
	byte expanded[kPalSize];
	for (uint i = 0; i < kPalSize; i++)
		expanded[i] = (byte)(src[i] << 2);
	g_system->getPaletteManager()->setPalette(expanded, 0, 256);
}

bool EEMEngine::setAnmPalette(const Common::Path &anmPath) {
	Common::File f;
	if (!f.open(anmPath)) {
		warning("setAnmPalette: cannot open %s", anmPath.toString().c_str());
		return false;
	}
	byte raw[kPalSize];
	if (f.read(raw, kPalSize) != kPalSize) {
		warning("setAnmPalette: short read on %s", anmPath.toString().c_str());
		return false;
	}
	byte expanded[kPalSize];
	for (uint i = 0; i < kPalSize; i++)
		expanded[i] = (byte)(raw[i] << 2);
	g_system->getPaletteManager()->setPalette(expanded, 0, 256);
	return true;
}

void EEMEngine::playAnm(const Common::Path &path, uint frameDelayMs, bool holdLastFrame) {
	ANMDecoder anm;
	if (!anm.open(path)) {
		warning("playAnm: %s missing", path.toString().c_str());
		return;
	}

	byte palette[768];
	anm.getPalette8(palette);
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	const uint16 w = anm.width();
	const uint16 h = anm.height();

	while (!shouldQuit()) {
		const byte *frame = anm.nextFrame();
		if (!frame)
			break;

		g_system->copyRectToScreen(frame, w, 0, 0, w, h);
		g_system->updateScreen();

		// Drain events and let the user skip with click/key. The original
		// uses _CheckFrameRate / _kbhit; we use a simple fixed delay until
		// the frame-rate calibration logic from _GetSpeedRating is wired up.
		// ESC additionally sets `_skipIntro` so the opening-anim chain in
		// run() bails out of the whole sequence instead of advancing to
		// the next clip.
		const uint32 frameStart = g_system->getMillis();
		bool aborted = false;
		while (g_system->getMillis() - frameStart < frameDelayMs && !aborted) {
			Common::Event event;
			while (g_system->getEventManager()->pollEvent(event)) {
				if (event.type == Common::EVENT_QUIT ||
					event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					event.type == Common::EVENT_LBUTTONDOWN) {
					aborted = true;
					break;
				}
				if (event.type == Common::EVENT_KEYDOWN) {
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
						_skipIntro = true;
					aborted = true;
					break;
				}
			}
			g_system->delayMillis(5);
		}
		if (aborted)
			break;
	}

	if (holdLastFrame && !shouldQuit() && !_skipIntro) {
		// Mirror the wait-loop at the end of `_DoOpeningAnims`:
		//   while (!keyDataAvailable) ;
		// We accept either a click or a key.
		while (!shouldQuit()) {
			Common::Event ev;
			bool clicked = false;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER ||
					ev.type == Common::EVENT_LBUTTONDOWN) {
					clicked = true;
					break;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
						_skipIntro = true;
					clicked = true;
					break;
				}
			}
			if (clicked)
				break;
			g_system->updateScreen();
			g_system->delayMillis(20);
		}
	}
}

void EEMEngine::blitAt(const Picture &pic, int x, int y) {
	// Clip against the 320x200 frame buffer.
	const int w = MIN<int>(pic.surface.w, 320 - x);
	const int h = MIN<int>(pic.surface.h, 200 - y);
	if (w <= 0 || h <= 0)
		return;
	g_system->copyRectToScreen(pic.surface.getPixels(), pic.surface.pitch,
							   x, y, w, h);
}

void EEMEngine::waitForInput(uint32 maxMs) {
	// ESC additionally raises `_skipIntro` so the opening-anim chain
	// can fast-forward past the rest of the sequence.
	const uint32 startMs = g_system->getMillis();
	while (!shouldQuit() && (g_system->getMillis() - startMs < maxMs)) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT ||
				event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				event.type == Common::EVENT_LBUTTONDOWN) {
				return;
			}
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					_skipIntro = true;
				return;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void EEMEngine::showEAKidsLogo() {
	// Mirrors _ShowEAKids @ 2520:05f0 (without the color-cycle loop):
	// GetPicture(0x54), MemoryCopy to VGA, GetPalette(0x25), setmany.
	Picture pic;
	if (!_picsArchive.getPicture(kPicEAKidsLogo, pic)) {
		warning("EA Kids logo (%u) load failed", kPicEAKidsLogo);
		return;
	}
	blitAt(pic, 0, 0);
	setSitePalette(kPalEAKids);
	g_system->updateScreen();
	waitForInput(2500);
}

void EEMEngine::showHighScoreLogo() {
	// Mirrors _ShowHScoreLogo @ 2520:0799 (without the wait-loop):
	// GetPicture(0x20c), MemoryCopy to VGA, GetPalette(0x27), FadeIn.
	Picture pic;
	if (!_picsArchive.getPicture(kPicHighScoreLogo, pic)) {
		warning("HighScore logo (%u) load failed", kPicHighScoreLogo);
		return;
	}
	blitAt(pic, 0, 0);
	setSitePalette(kPalHighScore);
	g_system->updateScreen();
	waitForInput(2500);
}

void EEMEngine::doNewPlayer() {
	// Mirrors `_NewPlayer` @ 1c33:0dda. The original draws background
	// 0x104 + character peek pic 0x107, then shows "Please type your
	// name" and accepts up to 12 characters until Enter. We render a
	// minimal version: black screen + prompt.
	if (!_font.isLoaded()) {
		_playerName = "Detective";
		return;
	}

	Common::String name;
	const int maxChars = 12;

	// Mirror the original: load PIC 0x104 as the name-entry backdrop.
	// The original also slides in PIC 0x107 (a peeking character).
	Picture bg;
	const bool haveBG = _picsArchive.getPicture(0x104, bg);

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveBG) {
			const int w = MIN<int>(bg.surface.w, 320);
			const int h = MIN<int>(bg.surface.h, 200);
			for (int row = 0; row < h; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)bg.surface.getBasePtr(0, row), w);
		}
		// Match the original `_NewPlayer`: `_Show_String(rw=0x28, cl=0x50)`
		// for the prompt, then `_ShowChar(0x50, x, …)` for typed input.
		// (rw=row=y, cl=col=x.) Prompt at (y=40, x=80), input at (y=80, x=80).
		_font.drawString(&scratch, "Please type your name:", 80, 40, 240, 0xF);
		Common::String shown = name + "_";
		_font.drawString(&scratch, shown, 80, 80, 240, 0xF);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};
	draw();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_RETURN) {
				if (name.empty())
					name = "Detective";
				_playerName = name;
				return;
			}
			if (k == Common::KEYCODE_ESCAPE) {
				_playerName = "Detective";
				return;
			}
			if (k == Common::KEYCODE_BACKSPACE) {
				if (!name.empty()) {
					name.deleteLastChar();
					dirty = true;
				}
				continue;
			}
			if (ev.kbd.ascii >= ' ' && ev.kbd.ascii < 127 &&
				(int)name.size() < maxChars) {
				name += (char)ev.kbd.ascii;
				dirty = true;
			}
		}
		if (dirty)
			draw();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
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
	blitAt(background, 0, 0);
	blitAt(girlAnim[0], kGirlX, kGirlY);
	blitAt(boyAnim[0], kBoyX, kBoyY);
	g_system->updateScreen();

	debugC(1, kDebugGeneral, "ChoosePartner: %u boy frames at (%d,%d), "
		   "%u girl frames at (%d,%d)",
		   (uint)boyAnim.size(), kBoyX, kBoyY,
		   (uint)girlAnim.size(), kGirlX, kGirlY);

	uint frame = 0;
	uint32 lastTick = g_system->getMillis();
	while (!shouldQuit()) {
		// Advance frame at ~5 Hz so the animations cycle gently.
		if (g_system->getMillis() - lastTick > 200) {
			lastTick = g_system->getMillis();
			frame++;
			blitAt(background, 0, 0);
			blitAt(girlAnim[frame % girlAnim.size()], kGirlX, kGirlY);
			blitAt(boyAnim[frame % boyAnim.size()], kBoyX, kBoyY);
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
}

void EEMEngine::doCaseSelection() {
	// Mirrors `_CaseSelection` @ 1c33:0a87. The original draws PIC 0x41
	// (chooser background) plus a centred "Book %d" / "Challenge Book"
	// header at (y=12) and then calls `_DoChoose(list)` to render the
	// menu via `DrawList` @ 1c33:040d at (_TextBox+3, DAT_29be_0d02) =
	// (61, 35), 12 rows × 10 px line height. The menu list itself is
	// the static array at 29be:0d6a (verified via `push 0x0d6a` at
	// 1c33:1ab4). Strings are at 29be:0ef4 onwards. Layout:
	//   list[0]  = "----------------------------------"
	//   list[1]  = "         Choose A Mystery"
	//   list[2..10] = alternating menu items + separators
	// Five selectable items: Choose A Mystery / Practice Mystery /
	// See ScrapBook 1/2/3.
	const uint kMaxMystery = 54;

	enum MenuPick {
		kPickChoose = 0,
		kPickPractice,
		kPickScrap1,
		kPickScrap2,
		kPickScrap3,
		kNumPicks
	};
	const char *kPickLabel[kNumPicks] = {
		"         Choose A Mystery",
		"         Practice Mystery",
		"         See ScrapBook 1",
		"         See ScrapBook 2",
		"         See ScrapBook 3"
	};
	// ScrapBooks aren't implemented yet — grey them so the player can't
	// stop on them, mirroring the original `_Greys` mask.
	const bool kPickEnabled[kNumPicks] = { true, true, false, false, false };
	uint pick = kPickChoose;

	const char *kSeparator = "----------------------------------";

	// Click rectangles from the original `_DoChoose` @ 1c33:0514 — each
	// `_InRect(_MouseX, _MouseY, addr, 0x29be)` reads one 4×u16 rect at
	// the listed offset in segment 29be ({x1, y1, x2, y2}). We use
	// `Common::Rect` (left/top/right/bottom) which also gives us
	// `contains(x, y)` for hit testing.
	const Common::Rect kOkRect      ( 12,  63,  41,  87); // 29be:0cd8 confirm
	const Common::Rect kHelpRect    ( 12, 100,  41, 124); // 29be:0ce0 help
	const Common::Rect kExitRect    ( 12, 137,  41, 161); // 29be:0ce8 cancel
	const Common::Rect kUpArrowRect (240,  31, 250,  43); // 29be:0cf0 scroll up
	const Common::Rect kDnArrowRect (240, 148, 250, 159); // 29be:0cf8 scroll dn
	const Common::Rect kListRect    ( 58,  35, 238, 158); // 29be:0d00 list panel

	// The original `_NewPlayer` set `_MouseCursor = 1` on exit; the
	// chain of screens after it expects the cursor to stay visible.
	// Reassert here in case anything between hid it.
	CursorMan.showMouse(true);

	// Mirrors `_CaseSelection`: load PIC 0x41 as the chooser backdrop.
	Picture caseBg;
	const bool haveCaseBg = _picsArchive.getPicture(0x41, caseBg);

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveCaseBg) {
			const int w = MIN<int>(caseBg.surface.w, 320);
			const int h = MIN<int>(caseBg.surface.h, 200);
			for (int row = 0; row < h; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)caseBg.surface.getBasePtr(0, row), w);
			}
		}
		if (_font.isLoaded()) {
			// `DrawList` @ 1c33:040d coordinates: `_TextBox + 3` for x
			// and `DAT_29be_0d02` for y. `_TextBox` @ 29be:0d00 holds
			// {x=58, y=35, x2=238, y2=158}. Matches the blue panel.
			const int kListX  = 58 + 3;
			const int kListW  = 238 - kListX;
			const int kListY0 = 35;
			const int kLineH  = 10;

			// Top centred "Book %d" / "Challenge Book" title — sprintf
			// format strings at 29be:0deb / 29be:0dfa shown via
			// `_Show_String(0xc, (0xba - width)/2 + 0x3c, …)` in the
			// original. We don't track challenge tier yet so always
			// show "Book 1".
			const Common::String book = "Book 1";
			const int titleW = _font.getStringWidth(book);
			const int titleX = (0xba - titleW) / 2 + 0x3c;
			_font.drawString(&scratch, book, titleX, 12, 320, 0xF);

			// Render 11 list rows: separator + menu item pairs.
			//   row 0  separator
			//   row 1  Choose A Mystery
			//   row 2  separator
			//   row 3  Practice Mystery
			//   ...
			//   row 9  See ScrapBook 3
			//   row 10 separator
			for (int r = 0; r < 11; r++) {
				const int y = kListY0 + r * kLineH;
				if ((r & 1) == 0) {
					_font.drawString(&scratch, kSeparator, kListX, y, kListW, 0x7);
					continue;
				}
				const uint mp = (uint)(r >> 1);
				const bool isSel  = (mp == pick);
				const byte color  = isSel        ? 0xF :
									kPickEnabled[mp] ? 0x7 : 0x8;
				_font.drawString(&scratch, kPickLabel[mp], kListX, y, kListW, color);
			}
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	auto pickPrev = [&]() {
		for (int i = 0; i < (int)kNumPicks; i++) {
			pick = (pick == 0) ? (uint)(kNumPicks - 1) : pick - 1;
			if (kPickEnabled[pick])
				break;
		}
	};
	auto pickNext = [&]() {
		for (int i = 0; i < (int)kNumPicks; i++) {
			pick = (pick + 1) % kNumPicks;
			if (kPickEnabled[pick])
				break;
		}
	};

	draw();

	bool exitChosen = false;
	while (!shouldQuit()) {
		Common::Event ev;
		bool confirmed = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// OK / EXIT / HELP buttons (rectangles from `_DoChoose`).
				if (kOkRect.contains(ev.mouse.x, ev.mouse.y)) {
					confirmed = true;
					break;
				}
				if (kExitRect.contains(ev.mouse.x, ev.mouse.y)) {
					exitChosen = true;
					confirmed = true;
					break;
				}
				if (kHelpRect.contains(ev.mouse.x, ev.mouse.y)) {
					// HELP placeholder — original calls `_DisplayHint`;
					// our help screen is wired to `H` later in the flow.
					continue;
				}
				// List panel: click on a non-separator row selects the
				// menu entry under the cursor.
				if (kListRect.contains(ev.mouse.x, ev.mouse.y)) {
					const int kLineH = 10;
					const int row = (ev.mouse.y - kListRect.top) / kLineH;
					if ((row & 1) == 1) {
						const uint mp = (uint)(row >> 1);
						if (mp < kNumPicks && kPickEnabled[mp]) {
							pick = mp;
							draw();
							continue;
						}
					}
				}
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				exitChosen = true;
				confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_RETURN) {
				confirmed = true;
				break;
			}
			if (k == Common::KEYCODE_UP || k == Common::KEYCODE_LEFT) {
				pickPrev();
				draw();
				continue;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_RIGHT ||
				k == Common::KEYCODE_TAB) {
				pickNext();
				draw();
				continue;
			}
		}
		if (confirmed) {
			draw();
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (shouldQuit())
		return;

	if (exitChosen) {
		_mystery.clear();
		_nextScreen = kScreenInvalid;
		return;
	}

	// "Practice Mystery" is the tutorial → mystery 0.
	if (pick == kPickPractice) {
		if (!_mystery.load(0, &_rng)) {
			warning("doCaseSelection: failed to load practice mystery");
			_mystery.clear();
		}
		return;
	}

	if (pick != kPickChoose) {
		// ScrapBooks aren't implemented; bail back to the menu loop.
		_mystery.clear();
		return;
	}

	// "Choose A Mystery" sub-screen: pick a specific case from the
	// 55-mystery roster. The original opens a different list here;
	// we approximate with the tier-aware numeric chooser we used
	// before. Default to the first unsolved mystery.
	uint sel = 0;
	for (uint i = 0; i <= kMaxMystery; i++) {
		if (i < sizeof(_mysteriesSolved) && !_mysteriesSolved[i]) {
			sel = i;
			break;
		}
	}

	auto drawSubmenu = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveCaseBg) {
			const int w = MIN<int>(caseBg.surface.w, 320);
			const int h = MIN<int>(caseBg.surface.h, 200);
			for (int row = 0; row < h; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)caseBg.surface.getBasePtr(0, row), w);
		}
		if (_font.isLoaded()) {
			const int kListX  = 61;
			const int kListW  = 238 - kListX;
			const int kListY0 = 35;
			const int kLineH  = 10;
			const int kVisible = 12;
			int top = (int)sel - kVisible / 2;
			if (top < 0) top = 0;
			if (top + kVisible > (int)kMaxMystery + 1)
				top = (int)kMaxMystery + 1 - kVisible;
			for (int r = 0; r < kVisible; r++) {
				const int idx = top + r;
				if (idx > (int)kMaxMystery)
					break;
				char marker = ' ';
				if ((uint)idx < sizeof(_mysteriesSolved)) {
					if (_mysteriesSolved[idx] == 2) marker = '*';
					else if (_mysteriesSolved[idx] == 1) marker = '+';
				}
				const char arrow = ((uint)idx == sel) ? '>' : ' ';
				_font.drawString(&scratch,
								 Common::String::format("%c %c Mystery %d", arrow, marker, idx),
								 kListX, kListY0 + r * kLineH, kListW, 0xF);
			}
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	drawSubmenu();
	bool confirmed = false;
	while (!confirmed && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Same `_DoChoose` rectangles as the top-level menu.
				if (kOkRect.contains(ev.mouse.x, ev.mouse.y)) {
					confirmed = true;
					break;
				}
				if (kExitRect.contains(ev.mouse.x, ev.mouse.y)) {
					_mystery.clear();
					return;
				}
				if (kUpArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					sel = (sel == 0) ? kMaxMystery : sel - 1;
					drawSubmenu();
					continue;
				}
				if (kDnArrowRect.contains(ev.mouse.x, ev.mouse.y)) {
					sel = (sel >= kMaxMystery) ? 0 : sel + 1;
					drawSubmenu();
					continue;
				}
				if (kListRect.contains(ev.mouse.x, ev.mouse.y)) {
					// Pick the row under the cursor.
					const int kLineH = 10;
					const int kVisible = 12;
					int top = (int)sel - kVisible / 2;
					if (top < 0) top = 0;
					if (top + kVisible > (int)kMaxMystery + 1)
						top = (int)kMaxMystery + 1 - kVisible;
					const int row = (ev.mouse.y - kListRect.top) / kLineH;
					const int idx = top + row;
					if (idx >= 0 && idx <= (int)kMaxMystery) {
						sel = (uint)idx;
						drawSubmenu();
					}
					continue;
				}
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			if (k == Common::KEYCODE_ESCAPE) {
				_mystery.clear();
				return;
			}
			if (k == Common::KEYCODE_RETURN) {
				confirmed = true;
				break;
			}
			if (k >= Common::KEYCODE_0 && k <= Common::KEYCODE_9) {
				sel = (uint)(k - Common::KEYCODE_0);
				drawSubmenu();
				continue;
			}
			if (k == Common::KEYCODE_DOWN || k == Common::KEYCODE_TAB) {
				sel = (sel >= kMaxMystery) ? 0 : sel + 1;
				drawSubmenu();
				continue;
			}
			if (k == Common::KEYCODE_UP) {
				sel = (sel == 0) ? kMaxMystery : sel - 1;
				drawSubmenu();
				continue;
			}
			if (k == Common::KEYCODE_PAGEDOWN) {
				sel = (sel + 10 > kMaxMystery) ? kMaxMystery : sel + 10;
				drawSubmenu();
				continue;
			}
			if (k == Common::KEYCODE_PAGEUP) {
				sel = (sel < 10) ? 0 : sel - 10;
				drawSubmenu();
				continue;
			}
			if (k == Common::KEYCODE_HOME) { sel = 0; drawSubmenu(); continue; }
			if (k == Common::KEYCODE_END)  { sel = kMaxMystery; drawSubmenu(); continue; }
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}

	if (!_mystery.load(sel, &_rng)) {
		warning("doCaseSelection: failed to load mystery %u", sel);
		_mystery.clear();
		return;
	}
	debugC(1, kDebugMystery, "Mystery %u loaded; %u sites, %u suspects",
		   sel, _mystery.numSites(), _mystery.numSuspects());
}

void EEMEngine::doInitClues() {
	// Mirrors `_DoInitClues` @ 1a35:0411. Sets BG 0x52 + palette 0x22,
	// blits the Goblindroid game and book first frames, then displays
	// the case briefing ClueBlock at InitBlock + 4. Marks the starting
	// site (InitBlock word[1]) on `_OnSites`.
	if (!_mystery.isLoaded())
		return;

	const byte *ib = _mystery.initBlock();
	if (!ib)
		return;

	const uint16 startSite = READ_LE_UINT16(ib + 2);
	if (startSite < Mystery::kVisitedSiteCap)
		_mystery._onSites[startSite] = 1;
	// Mirror the original: at briefing time the player isn't actually
	// at any site yet — they pick from the map next. Set _siteNumber
	// to the start site so the map opens centred on the only initially
	// accessible location and the post-map site loop has a sensible
	// resume point.
	_mystery._siteNumber = startSite;
	_mystery._lastSite = startSite;

	setSitePalette(0x22);
	Picture bg;
	if (_picsArchive.getPicture(0x52, bg))
		blitAt(bg, 0, 0);

	const uint gameAni = _partner == 0 ? 0x17 : 0x3b;
	const uint bookAni = _partner == 0 ? 0x18 : 0x3c;
	Animation game, book;
	if (_aniArchive.loadAnimation(gameAni, game) && !game.empty())
		blitAt(game[0], 0xcd, 0x6c);
	if (_aniArchive.loadAnimation(bookAni, book) && !book.empty())
		blitAt(book[0], 0, 99);

	// Case type 1 also places "Nancy" (a third character) at (0x68, 0x8b)
	// per `_DoInitClues`.
	const uint16 caseType = READ_LE_UINT16(ib);
	if (caseType == 1) {
		Animation nancy;
		if (_aniArchive.loadAnimation(0x19, nancy) && !nancy.empty())
			blitAt(nancy[0], 0x68, 0x8b);
	}

	displayClue(ib + 4);
}

void EEMEngine::doSiteLoop() {
	// Mirrors the per-mystery site loop. SiteScreen::run() handles
	// hotspot clicks plus M (map), N (notebook), G (gallery), A (accuse),
	// Tab (next site), ESC (exit).
	SiteScreen screen(this, &_mystery);
	screen.run();
}

/// Mirror `_ParseString` @ 1b66:07c3 — substitute the control bytes that
/// the original engine uses as placeholders. Only the two we encounter most
/// often (player name = 0x80, partner first name = 0x82) are substituted;
/// other 0x8N opcodes are stripped. The original engine also handles
/// hyphenation marks and a hint placeholder (0x89) we ignore for now.
static Common::String parseString(const Common::String &raw,
								  const Common::String &playerName,
								  uint partner) {
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
		// Per `_DisplayClue` @ 2404:05e6: partner 1 uses its own field
		// set ONLY when bubText1 is not -1; otherwise it falls back to
		// the partner 0 fields entirely. Partner 0 always uses field 0.
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
			if (!screen) break;
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
				// Per-balloon metadata table at 29be:0875 — 10-byte
				// entries indexed by `(bubNum & 0x7f)`. Layout:
				//   +0..1 textX inset, +2..3 textY inset, +4..5 textWidth.
				// All entries use textX=6, textY=4 so we hard-code those
				// constants; textWidth is read live from the table.
				textX = bubX + 6;
				textY = bubY + 4;
				textW = bw - 12;
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

void EEMEngine::doNotebook() {
	// Mirrors `_DrawNotes` @ 161e:01d0 + `_HandleNoteButton`. We list every
	// found clue with its NoteIndex point value and let the player toggle
	// "selected" with number keys 1..9 (paged in groups of 9). The total
	// points of selected clues feed `_SolvedCheck` during accuse.
	if (!_font.isLoaded())
		return;

	int page = 0;
	const int kPerPage = 9;

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		_font.drawString(&scratch, "NOTEBOOK", 8, 4, 320, 0xF);
		_font.drawString(&scratch, Common::String::format("pts: %d", _mystery.selectedPoints()), 200, 4, 320, 0xF);

		// Build a list of found-clue indices.
		Common::Array<uint> found;
		for (uint i = 0; i < Mystery::kCluesFoundCap; i++)
			if (_mystery._cluesFound[i])
				found.push_back(i);
		const int total = (int)found.size();
		const int pages = MAX<int>(1, (total + kPerPage - 1) / kPerPage);
		page = MIN<int>(page, pages - 1);

		_font.drawString(&scratch, Common::String::format("page %d/%d", page + 1, pages), 200, 16, 320, 0xF);

		const byte *ni = _mystery.noteIndex();
		const uint16 niCount = _mystery.noteIndexCount();
		int y = 4 + _font.getFontHeight() * 2 + 4;
		for (int slot = 0; slot < kPerPage; slot++) {
			const int idx = page * kPerPage + slot;
			if (idx >= total)
				break;
			const uint clueId = found[idx];
			Common::String text;
			int pts = 0;
			if (ni && clueId < niCount) {
				const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
				const uint16 ptsRaw  = READ_LE_UINT16(ni + clueId * 4 + 2);
				pts = (int)(int16)ptsRaw;
				const Common::String raw = _mystery.textAt(textOff);
				text = parseString(raw, _playerName, _partner);
			}
			if (text.empty())
				text = Common::String::format("clue %u", clueId);

			const char selMark = _mystery._noteSelected[clueId] ? '*' : ' ';
			Common::String line = Common::String::format(
				"%d [%c] (%d pts) %s", slot + 1, selMark, pts, text.c_str());
			const int used = _font.drawWordWrapped(&scratch, 8, y, 304, line, 0xF);
			y += used + 2;
			if (y >= 192) break;
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	draw();
	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool exit  = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) { exit = true; break; }
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) { exit = true; break; }
				if (ev.kbd.keycode >= Common::KEYCODE_1 && ev.kbd.keycode <= Common::KEYCODE_9) {
					const int slot = (int)(ev.kbd.keycode - Common::KEYCODE_1);
					Common::Array<uint> found;
					for (uint i = 0; i < Mystery::kCluesFoundCap; i++)
						if (_mystery._cluesFound[i])
							found.push_back(i);
					const int idx = page * kPerPage + slot;
					if (idx < (int)found.size()) {
						const uint clueId = found[idx];
						_mystery._noteSelected[clueId] ^= 1;
						dirty = true;
					}
				} else if (ev.kbd.keycode == Common::KEYCODE_TAB ||
						   ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					page++; dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					if (page > 0) page--;
					dirty = true;
				}
			}
		}
		if (exit) break;
		if (dirty) draw();
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::doGallery() {
	// Mirrors `_DrawGallery` @ 158f:0046. The original loops `_NumSuspects`
	// gallery entries (0x46 = 70 bytes each in `_GalleryData`); the first
	// u16 of each entry is the PIC picture ID for that suspect. We render
	// them in a row across the screen.
	if (!_mystery.isLoaded())
		return;

	const byte *gd = _mystery.galleryData();
	if (!gd) {
		warning("doGallery: no GalleryData in mystery %u", _mystery.number());
		return;
	}

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();

	// Use PIC 0x3f as the gallery backdrop, matching `_DoAccuseGallery`.
	Picture galBg;
	if (_picsArchive.getPicture(0x3f, galBg)) {
		const int w = MIN<int>(galBg.surface.w, 320);
		const int h = MIN<int>(galBg.surface.h, 200);
		for (int row = 0; row < h; row++) {
			memcpy((byte *)scratch.getBasePtr(0, row),
				   (const byte *)galBg.surface.getBasePtr(0, row), w);
		}
	}

	if (_font.isLoaded())
		_font.drawString(&scratch, "GALLERY", 8, 4, 320, 0xF);

	const uint8 num = _mystery.numSuspects();
	int slotX = 8;
	const int slotY = 24;
	const int slotStep = 320 / MAX<uint8>(1, num);
	for (uint i = 0; i < num; i++) {
		const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
		if (picId == 0)
			continue;
		Picture portrait;
		if (!_picsArchive.getPicture(picId, portrait))
			continue;
		const int placeX = slotX + (slotStep - portrait.surface.w) / 2;
		const int placeY = slotY;
		const int w = MIN<int>(portrait.surface.w, 320 - placeX);
		const int h = MIN<int>(portrait.surface.h, 200 - placeY);
		if (w > 0 && h > 0) {
			for (int row = 0; row < h; row++) {
				memcpy((byte *)scratch.getBasePtr(placeX, placeY + row),
					   (const byte *)portrait.surface.getBasePtr(0, row), w);
			}
		}
		// Suspect number + discovered marker under the portrait.
		if (_font.isLoaded()) {
			const bool discovered = (i < Mystery::kGalleryCap) &&
									_mystery._inGallery[i];
			Common::String label = Common::String::format("%u%s",
				i + 1, discovered ? " *" : "");
			_font.drawString(&scratch, label, placeX + 4, placeY + h + 2, 320, 0xF);
		}
		slotX += slotStep;
	}

	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
	waitForInput(60000);
}

void EEMEngine::doBigMap() {
	// Two-stage flow that mirrors the original screen-1 wrapper at
	// 20fe:120b and `_DoBigMap @ 20fe:09e7`:
	//
	//   STAGE 1 — Overview. PIC 0x42 + site icons drawn via the
	//   `_DrawBigMapButtons` algorithm at BigMap coords MapData[+4/+6].
	//   The original `_DoBigMap` returns sx/sy = (mouseX*2 - 0x74,
	//   mouseY*2 - 0x55) when the player clicks inside `BigMapWindow`,
	//   which is the scroll position into the SmallMap.
	//
	//   STAGE 2 — Detail zoom. PIC 0x43 frame + a 0xe9 × 0xab viewport
	//   into BIGMAP.PIC at (2, 2), drawn by `DrawMap @ 20fe:1058` with
	//   the (sx, sy) returned from stage 1. Site icons are stamped at
	//   SmallMap coords MapData[+8/+0xa] via `_StampButtons`. Click on
	//   a site icon → travel.
	//
	// MapData entry layout (14 bytes), verified directly from the
	// disassembly of `_DrawBigMapButtons @ 20fe:0877` (`PUSH ES:[BX+4]`
	// for X, `PUSH ES:[BX+6]` for Y, `CMP ES:[BX+0xc], 0` for crime)
	// and `_StampButtons @ 20fe:0d2f` (`MOV AX, ES:[BX+8]`,
	// `MOV AX, ES:[BX+0xa]`):
	//   +0..3   ??? (not yet decoded)
	//   +4..5   BigMap X
	//   +6..7   BigMap Y
	//   +8..9   SmallMap X
	//   +0xa..b SmallMap Y
	//   +0xc..d crime-flag

	if (!_mystery.isLoaded())
		return;

	CursorMan.showMouse(true);

	// `_GetPalette(0x24)` per `_DoBigMap @ 20fe:09e7`.
	setSitePalette(0x24);

	const Common::Rect kSetupRect(0xc7, 0x12, 0xc7 + 0x32, 0x12 + 0xa); // approx; original from globals
	(void)kSetupRect; // not yet wired into our overlay

	// ------------------------------------------------------------------
	// STAGE 1 — Overview: PIC 0x42 + clickable site icons.
	// ------------------------------------------------------------------

	auto drawOverview = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		Picture frame;
		if (_picsArchive.getPicture(0x42, frame)) {
			const int w = MIN<int>(frame.surface.w, 320);
			const int h = MIN<int>(frame.surface.h, 200);
			for (int row = 0; row < h; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)frame.surface.getBasePtr(0, row), w);
		}

		// Marker PICs from `_main @ 1a35:0f59`. Three globals are filled
		// once at boot via `_GetPicture` (1-based IDs → entries N-1):
		//   _DoneMarker  = PIC 0x20d  (already-searched site)
		//   _SiteMarker  = PIC 0xc5   (default available site)
		//   _CrimeMarker = PIC 0xc6   (crime-scene flag set)
		// Picked per-site by `_DrawBigMapButtons @ 20fe:0877`:
		//   1. SaveSiteComplete[i] → DoneMarker
		//   2. else MapData[+0xc] != 0 → CrimeMarker
		//   3. else SiteMarker
		Picture done, normal, crimeM;
		const bool haveDone   = _picsArchive.getPicture(0x20d, done);
		const bool haveNormal = _picsArchive.getPicture(0xc5,  normal);
		const bool haveCrime  = _picsArchive.getPicture(0xc6,  crimeM);

		auto blitMarker = [&](const Picture &m, int x, int y) {
			const byte transp = (byte)(m.flags >> 8);
			for (int row = 0; row < m.surface.h; row++) {
				const int dstY = y + row;
				if (dstY < 0 || dstY >= 200) continue;
				const byte *src = (const byte *)m.surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < m.surface.w; col++) {
					const int dstX = x + col;
					if (dstX < 0 || dstX >= 320) continue;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
		};

		for (uint i = 0; i < _mystery.numSites(); i++) {
			if (!_mystery._onSites[i] && i != _mystery._siteNumber)
				continue;
			const byte *entry = _mystery.mapEntry(i);
			if (!entry)
				continue;
			const uint16 mx    = READ_LE_UINT16(entry + 0x4);
			const uint16 my    = READ_LE_UINT16(entry + 0x6);
			const uint16 crime = READ_LE_UINT16(entry + 0xc);
			const bool   done_ = (i < Mystery::kVisitedSiteCap)
								  && _mystery._visitedSite[i];

			const Picture *m = nullptr;
			if (done_ && haveDone)            m = &done;
			else if (crime != 0 && haveCrime) m = &crimeM;
			else if (haveNormal)              m = &normal;

			if (m)
				blitMarker(*m, (int)mx, (int)my);
			else {
				// Fallback if the markers couldn't be loaded.
				const Common::Rect mark(mx - 3, my - 3, mx + 4, my + 4);
				scratch.fillRect(mark, 0x0F);
			}
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	drawOverview();

	// Static rectangles read directly from the binary at the labelled
	// addresses (29be:0x1596 onwards). Format is {x1, y1, x2, y2}.
	const Common::Rect kBigMapWindow   (  0,   0, 247, 192); // 29be:1596
	const Common::Rect kSetupBtnRect   (252,   4, 315,  42); // 29be:15ce

	bool wantZoom = false;
	int  zoomX = 0, zoomY = 0;
	while (!shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN &&
				ev.kbd.keycode == Common::KEYCODE_ESCAPE)
				return;
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// SetupButtonRect → `_NextScreen = 6` (the original's
				// settings screen). We use it as "back to menu":
				// abandon the current mystery and return to case
				// selection.
				if (kSetupBtnRect.contains(ev.mouse.x, ev.mouse.y)) {
					_mystery.clear();
					_nextScreen = kScreenInvalid;
					return;
				}
				// Click in the BigMapWindow → zoom. Original formula:
				//   sx = mouseX*2 - 0x74; sy = mouseY*2 - 0x55
				if (kBigMapWindow.contains(ev.mouse.x, ev.mouse.y)) {
					int sx = ev.mouse.x * 2;
					int sy = ev.mouse.y * 2;
					sx = (sx < 0x75) ? 0 : sx - 0x74;
					sy = (sy < 0x56) ? 0 : sy - 0x55;
					zoomX = sx;
					zoomY = sy;
					wantZoom = true;
					break;
				}
			}
		}
		if (wantZoom)
			break;
		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	if (!wantZoom)
		return;

	// ------------------------------------------------------------------
	// STAGE 2 — Detail zoom: PIC 0x43 frame + scrollable BIGMAP.PIC
	// viewport at (2, 2), 0xe9 × 0xab. Click on a stamped icon → travel.
	// ------------------------------------------------------------------

	Common::File f;
	if (!f.open(Common::Path("BIGMAP.PIC"))) {
		warning("doBigMap: BIGMAP.PIC missing for detail view");
		return;
	}
	const uint16 mapH = f.readUint16LE();
	const uint16 mapW = f.readUint16LE();
	if (mapW == 0 || mapH == 0)
		return;
	Common::Array<byte> mapPixels((uint32)mapW * mapH);
	if (f.read(mapPixels.data(), mapPixels.size()) != mapPixels.size()) {
		warning("doBigMap: short read on BIGMAP.PIC for detail view");
		return;
	}

	const int kMapWinW = 0xe9; // 233
	const int kMapWinH = 0xab; // 171
	const int kMapWinX = 2;
	const int kMapWinY = 2;

	int scrollX = MAX<int>(0, MIN<int>(mapW - kMapWinW, zoomX));
	int scrollY = MAX<int>(0, MIN<int>(mapH - kMapWinH, zoomY));

	auto drawDetail = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		Picture frame;
		if (_picsArchive.getPicture(0x43, frame)) {
			const int w = MIN<int>(frame.surface.w, 320);
			const int h = MIN<int>(frame.surface.h, 200);
			for (int row = 0; row < h; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)frame.surface.getBasePtr(0, row), w);
		}

		const int copyW = MIN<int>(mapW - scrollX, kMapWinW);
		const int copyH = MIN<int>(mapH - scrollY, kMapWinH);
		for (int row = 0; row < copyH; row++) {
			memcpy((byte *)scratch.getBasePtr(kMapWinX, kMapWinY + row),
				   mapPixels.data() + (scrollY + row) * mapW + scrollX,
				   copyW);
		}

		// Stamped site buttons. `_StampButtons @ 20fe:0d2f` does:
		//   button = _GetButton(MapData[+0])      // BUTTON.DBD entry
		//   destX  = MapData[+8],  destY = MapData[+0xa]
		// then bakes the button PIC into the map bitmap. Each button
		// sprite carries the site name baked in. We blit them on top
		// of the BIGMAP.PIC viewport at the same SmallMap coords.
		for (uint i = 0; i < _mystery.numSites(); i++) {
			if (!_mystery._onSites[i] && i != _mystery._siteNumber)
				continue;
			const byte *entry = _mystery.mapEntry(i);
			if (!entry) continue;
			const uint16 buttonId = READ_LE_UINT16(entry + 0x0);
			const uint16 mx       = READ_LE_UINT16(entry + 0x8);
			const uint16 my       = READ_LE_UINT16(entry + 0xa);

			Picture button;
			if (!_buttonArchive.loadEntry(buttonId, button))
				continue;
			const int sx = (int)mx - scrollX + kMapWinX;
			const int sy = (int)my - scrollY + kMapWinY;
			const byte transp = (byte)(button.flags >> 8);

			// Crop blit against the viewport.
			const int x0 = MAX<int>(sx, kMapWinX);
			const int y0 = MAX<int>(sy, kMapWinY);
			const int x1 = MIN<int>(sx + button.surface.w, kMapWinX + kMapWinW);
			const int y1 = MIN<int>(sy + button.surface.h, kMapWinY + kMapWinH);
			for (int row = y0; row < y1; row++) {
				const byte *src = (const byte *)button.surface.getBasePtr(0, row - sy);
				byte *dst = (byte *)scratch.getBasePtr(0, row);
				for (int col = x0; col < x1; col++) {
					const byte px = src[col - sx];
					if (px != transp)
						dst[col] = px;
				}
			}
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	drawDetail();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
					return;  // exit detail back to caller (site loop / engine)
				const int kStep = 16;
				if (ev.kbd.keycode == Common::KEYCODE_LEFT) {
					scrollX = MAX<int>(0, scrollX - kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT) {
					scrollX = MIN<int>(MAX<int>(0, mapW - kMapWinW),
						scrollX + kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_UP) {
					scrollY = MAX<int>(0, scrollY - kStep);
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_DOWN) {
					scrollY = MIN<int>(MAX<int>(0, mapH - kMapWinH),
						scrollY + kStep);
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Scroll arrows + slider rects live in `SmallMapButtons`
				// at 29be:0x159e (six 8-byte rects in order: Y-up, Y-down,
				// X-left, X-right, right-panel, top-right) plus the
				// dedicated `XSliderRect @ 29be:15d6` and
				// `YSliderRect @ 29be:15de`. Format {x1,y1,x2,y2}.
				const Common::Rect kArrowYUp   (237,   2, 247,  11);
				const Common::Rect kArrowYDown (237, 163, 247, 172);
				const Common::Rect kArrowXLeft (  2, 175,  12, 185);
				const Common::Rect kArrowXRight(224, 175, 234, 185);
				const Common::Rect kXSlider    ( 15, 175, 221, 185);
				const Common::Rect kYSlider    (237,  14, 247, 160);
				const Common::Rect kSetupBtn   (252,   4, 315,  42);

				const int kArrowStep = 16;
				const int kSliderRange = mapW - kMapWinW;
				const int kSliderRangeY = mapH - kMapWinH;

				if (kSetupBtn.contains(ev.mouse.x, ev.mouse.y)) {
					// Setup button on detail too — `_NextScreen = 6` in
					// the original. We treat it the same way: bail back
					// to case selection.
					_mystery.clear();
					_nextScreen = kScreenInvalid;
					return;
				}
				if (kArrowYUp.contains(ev.mouse.x, ev.mouse.y)) {
					scrollY = MAX<int>(0, scrollY - kArrowStep);
					dirty = true;
				} else if (kArrowYDown.contains(ev.mouse.x, ev.mouse.y)) {
					scrollY = MIN<int>(MAX<int>(0, kSliderRangeY),
						scrollY + kArrowStep);
					dirty = true;
				} else if (kArrowXLeft.contains(ev.mouse.x, ev.mouse.y)) {
					scrollX = MAX<int>(0, scrollX - kArrowStep);
					dirty = true;
				} else if (kArrowXRight.contains(ev.mouse.x, ev.mouse.y)) {
					scrollX = MIN<int>(MAX<int>(0, kSliderRange),
						scrollX + kArrowStep);
					dirty = true;
				} else if (kXSlider.contains(ev.mouse.x, ev.mouse.y)) {
					// Click on X slider track → jump scrollX so the
					// click position maps proportionally into the map.
					if (kSliderRange > 0) {
						const int t = ev.mouse.x - kXSlider.left;
						const int tw = kXSlider.width();
						scrollX = MAX<int>(0, MIN<int>(kSliderRange,
							t * kSliderRange / MAX<int>(1, tw)));
						dirty = true;
					}
				} else if (kYSlider.contains(ev.mouse.x, ev.mouse.y)) {
					if (kSliderRangeY > 0) {
						const int t = ev.mouse.y - kYSlider.top;
						const int th = kYSlider.height();
						scrollY = MAX<int>(0, MIN<int>(kSliderRangeY,
							t * kSliderRangeY / MAX<int>(1, th)));
						dirty = true;
					}
				} else if (ev.mouse.x >= kMapWinX &&
						   ev.mouse.x < kMapWinX + kMapWinW &&
						   ev.mouse.y >= kMapWinY &&
						   ev.mouse.y < kMapWinY + kMapWinH) {
					// Hit-test the per-site button at its actual bbox
					// (`_StampButtons` records the rect at SmallMap +8/+0xa
					// with the button PIC's width/height).
					for (uint i = 0; i < _mystery.numSites(); i++) {
						if (!_mystery._onSites[i] &&
							i != _mystery._siteNumber)
							continue;
						const byte *entry = _mystery.mapEntry(i);
						if (!entry) continue;
						const uint16 buttonId = READ_LE_UINT16(entry + 0x0);
						const uint16 mx       = READ_LE_UINT16(entry + 0x8);
						const uint16 my       = READ_LE_UINT16(entry + 0xa);
						Picture button;
						int bw = 16, bh = 16;
						if (_buttonArchive.loadEntry(buttonId, button)) {
							bw = button.surface.w;
							bh = button.surface.h;
						}
						const int sx = (int)mx - scrollX + kMapWinX;
						const int sy = (int)my - scrollY + kMapWinY;
						if (ev.mouse.x >= sx && ev.mouse.x < sx + bw &&
							ev.mouse.y >= sy && ev.mouse.y < sy + bh) {
							_mystery._lastSite = _mystery._siteNumber;
							_mystery._siteNumber = (uint16)i;
							return;
						}
					}
				}
			}
		}
		if (dirty)
			drawDetail();
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void EEMEngine::doHelp() {
	// `_KDHelp` reads two hint TextBlock offsets from `_KDTextIndex`:
	//   word @ +0xe : first-time hint
	//   word @ +0x10: second-time hint (cycles back to first if missing)
	// `_SawHelpHint` toggles between them.
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	const byte *kd = _mystery.kdTextIndex();
	if (!kd)
		return;

	const uint16 hintFirst  = READ_LE_UINT16(kd + 0x0e);
	const uint16 hintSecond = READ_LE_UINT16(kd + 0x10);
	uint16 use = _mystery._sawHelpHint && hintSecond != 0xFFFF ? hintSecond : hintFirst;
	if (use == 0xFFFF) {
		debugC(1, kDebugScript, "doHelp: no hint configured");
		return;
	}
	if (!_mystery._sawHelpHint && hintFirst != 0xFFFF)
		_mystery._sawHelpHint = true;

	const Common::String raw = _mystery.textAt(use);
	const Common::String text = parseString(raw, _playerName, _partner);

	Graphics::ManagedSurface scratch(320, 200,
		Graphics::PixelFormat::createFormatCLUT8());
	scratch.clear();
	_font.drawString(&scratch, "HELP", 8, 4, 320, 0xF);
	_font.drawWordWrapped(&scratch, 8, 24, 304, text, 0xF);
	g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
							   0, 0, 320, 200);
	g_system->updateScreen();
	waitForInput(60000);
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

void EEMEngine::doAccuse() {
	if (!_mystery.isLoaded())
		return;

	// Mirrors `_DoAccuseGallery` @ 1df2:0a31. Render gallery + prompt,
	// accept either keyboard 1..N or a click on a suspect's portrait.
	const uint8 num = _mystery.numSuspects();
	if (num == 0)
		return;

	const byte *gd = _mystery.galleryData();
	const int slotStep = 320 / MAX<uint8>(1, num);
	const int slotY    = 24;

	// Mirrors `_DoAccuseGallery`: load PIC 0x3f as the accuse backdrop.
	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);

	auto drawGallery = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveAccuseBg) {
			const int w = MIN<int>(accuseBg.surface.w, 320);
			const int h = MIN<int>(accuseBg.surface.h, 200);
			for (int row = 0; row < h; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)accuseBg.surface.getBasePtr(0, row), w);
			}
		}
		if (_font.isLoaded())
			_font.drawString(&scratch, "ACCUSE", 8, 4, 320, 0xF);

		for (uint i = 0; i < num; i++) {
			if (!gd) continue;
			const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
			if (picId == 0)
				continue;
			Picture portrait;
			if (!_picsArchive.getPicture(picId, portrait))
				continue;
			const int placeX = i * slotStep +
							   (slotStep - portrait.surface.w) / 2;
			const int placeY = slotY;
			const int w = MIN<int>(portrait.surface.w, 320 - placeX);
			const int h = MIN<int>(portrait.surface.h, 200 - placeY);
			if (w > 0 && h > 0) {
				for (int row = 0; row < h; row++)
					memcpy((byte *)scratch.getBasePtr(placeX, placeY + row),
						   (const byte *)portrait.surface.getBasePtr(0, row), w);
			}
			if (_font.isLoaded()) {
				Common::String label = Common::String::format("%u", i + 1);
				_font.drawString(&scratch, label, placeX + 4, placeY + h + 2, 320, 0xF);
			}
		}
		if (_font.isLoaded()) {
			_font.drawString(&scratch, "Click a suspect or press 1..N - ESC to cancel", 8, 180, 320, 0xF);
		}
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};
	drawGallery();

	int picked = -1;
	while (picked < 0 && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
					return;
				const int k = (int)ev.kbd.keycode;
				if (k >= Common::KEYCODE_1 && k <= Common::KEYCODE_9) {
					const int idx = k - Common::KEYCODE_1;
					if (idx < num)
						picked = idx;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				const int slot = ev.mouse.x / slotStep;
				if (slot >= 0 && slot < (int)num &&
					ev.mouse.y >= slotY && ev.mouse.y < slotY + 120)
					picked = slot;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	if (picked < 0)
		return;

	// Real chain evaluation: sum point values of clues the player marked
	// "selected" in the notebook. Mirrors `_SolvedCheck` @ 1df2:00ec.
	const int points = _mystery.selectedPoints();
	const bool guessedRight = _mystery.solvedCheck();
	debugC(1, kDebugScript, "doAccuse: picked=%d selectedPts=%d -> %s",
		   picked, points, guessedRight ? "correct" : "wrong");

	// If the player hasn't marked any evidence yet, give them a hint
	// rather than an instant fail. Mirrors the original "We're not ready
	// to solve this mystery yet..." string at 29be:10f0.
	if (points == 0 && _font.isLoaded()) {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		_font.drawWordWrapped(&scratch, 16, 80, 288,
			"We're not ready to solve this mystery yet. "
			"Let's keep investigating until we have some "
			"more solid evidence to make our case! "
			"(Press N in the site screen to mark clues.)",
			0xF);
		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
		waitForInput(15000);
		return;
	}

	// Pick the ending based on the chain. For a correct accusation the
	// original would call `_DisplayClue(_Mystery + AChain[0])`, play
	// SCRAPBK.ANI and save progress. We load the matching `E<n>.BIN`
	// ending text and render its pages with prev/next navigation.
	const int endingNum = guessedRight ? picked : 0;
	const Common::String fname = Common::String::format("E%d.BIN", endingNum);
	Common::File f;
	if (!f.open(Common::Path(fname))) {
		warning("doAccuse: %s missing", fname.c_str());
		return;
	}

	// E<n>.BIN format (verified against `_DisplayEndingPage` @ 1df2:044c):
	//   u16 numPages
	//   per page (10 bytes header + NUL-string):
	//     u16 picNum
	//     u16 x1, y1, x2, y2  (story rect)
	//     bytes[] NUL-terminated text
	const uint32 fileLen = f.size();
	Common::Array<byte> blob(fileLen);
	if (f.read(blob.data(), fileLen) != fileLen)
		return;
	const byte *e = blob.data();
	const uint16 pages = READ_LE_UINT16(e);

	uint pageIdx = 0;

	while (!shouldQuit()) {
		// Walk to pageIdx.
		uint pos = 2;
		uint cur = 0;
		while (cur < pageIdx && pos + 10 < fileLen) {
			const char *t = (const char *)(e + pos + 10);
			pos += 10 + strlen(t) + 1;
			cur++;
		}
		if (pos + 10 >= fileLen)
			break;

		const uint16 picNum = READ_LE_UINT16(e + pos + 0);
		const uint16 x1     = READ_LE_UINT16(e + pos + 2);
		const uint16 y1     = READ_LE_UINT16(e + pos + 4);
		const uint16 x2     = READ_LE_UINT16(e + pos + 6);
		const uint16 y2     = READ_LE_UINT16(e + pos + 8);
		const char *raw     = (const char *)(e + pos + 10);
		const Common::String txt = parseString(raw, _playerName, _partner);

		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		// Page background.
		if (picNum != 0) {
			Picture bg;
			if (_picsArchive.getPicture(picNum, bg)) {
				const int w = MIN<int>(bg.surface.w, 320);
				const int h = MIN<int>(bg.surface.h, 200);
				for (int row = 0; row < h; row++) {
					memcpy((byte *)scratch.getBasePtr(0, row),
						   (const byte *)bg.surface.getBasePtr(0, row), w);
				}
			}
		}

		if (_font.isLoaded()) {
			Common::String banner = "Not enough evidence";
			if (guessedRight)
				banner = _mystery._firstTry ? "CORRECT - FIRST TRY!" : "CORRECT!";
			_font.drawString(&scratch, banner, 8, 4, 320, 0xF);
			_font.drawString(&scratch, Common::String::format("Evidence: %d/100  Suspect: %d",
									   points, picked + 1), 8, 16, 320, 0xF);
			const int wrapW = MAX<int>(16, x2 - x1);
			const int wrapY = MAX<int>(28, (int)y1);
			(void)y2;
			_font.drawWordWrapped(&scratch, x1, wrapY, wrapW, txt, 0xF);
			_font.drawString(&scratch, Common::String::format("page %u/%u  (Left/Right or click)",
									   pageIdx + 1, pages), 8, 188, 320, 0xF);
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		// Page navigation.
		bool advance = false;
		bool back    = false;
		bool exit    = false;
		while (!advance && !back && !exit && !shouldQuit()) {
			Common::Event ev;
			while (g_system->getEventManager()->pollEvent(ev)) {
				if (ev.type == Common::EVENT_QUIT ||
					ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
					exit = true; break;
				}
				if (ev.type == Common::EVENT_LBUTTONDOWN) {
					advance = true; break;
				}
				if (ev.type == Common::EVENT_KEYDOWN) {
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE)
						exit = true;
					else if (ev.kbd.keycode == Common::KEYCODE_LEFT)
						back = true;
					else
						advance = true;
					break;
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(15);
		}
		if (exit) break;
		if (advance) {
			if (pageIdx + 1 >= pages) break;
			pageIdx++;
		} else if (back) {
			if (pageIdx > 0) pageIdx--;
		}
	}

	// Mirror `_DisplayCorrect`'s scrap-book animation + solved tracking +
	// auto-save (the original calls `_SavePlayerRecord` after a win).
	if (guessedRight) {
		const uint mn = _mystery.number();
		if (mn < sizeof(_mysteriesSolved)) {
			_mysteriesSolved[mn] = _mystery._firstTry ? 2 : 1;
		}
		playAnm(Common::Path("SCRAPBK.ANI"), 120, true);

		// Auto-save into slot 0 (the engine's quicksave slot).
		const Common::String desc = Common::String::format(
			"%s — solved mystery %u", _playerName.c_str(), mn);
		Common::Error err = saveGameState(0, desc, true);
		if (err.getCode() != Common::kNoError)
			warning("auto-save after solve failed: %s",
					err.getDesc().c_str());
	} else {
		_mystery._firstTry = false;
	}
}

// -------------------- save / load --------------------

namespace {
const uint32 kSaveMagic = MKTAG('E', 'E', 'M', '0');
const byte   kSaveVer   = 3;  ///< v2: _mysteriesSolved tracker; v3: player name
} // anonymous namespace

bool EEMEngine::hasFeature(EngineFeature f) const {
	// We support saving any time but loading only at startup (via the
	// `--save-slot=N` resume path or a slot picked from the launcher).
	// Runtime loads would replace `_mystery._data` while pointers into
	// it are alive on the stack inside `displayClue` etc.
	return f == kSupportsSavingDuringRuntime ||
		   f == kSupportsReturnToLauncher;
}

bool EEMEngine::canLoadGameStateCurrently(Common::U32String *) {
	return false;  // Loading is startup-only.
}

bool EEMEngine::canSaveGameStateCurrently(Common::U32String *) {
	return _mystery.isLoaded();
}

Common::Error EEMEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	Common::OutSaveFile *out = getSaveFileManager()->openForSaving(getSaveStateName(slot));
	if (!out)
		return Common::kCreatingFileFailed;

	out->writeUint32BE(kSaveMagic);
	out->writeByte(kSaveVer);

	// Header: description + ScummVM extended save metadata are appended
	// automatically when `EngineFeature::kSavesUseExtendedFormat` is set;
	// our save body just carries the engine state.
	(void)desc;
	(void)isAutosave;

	uint16 mysteryNum = (uint16)_mystery.number();
	out->writeUint16LE(mysteryNum);
	out->writeByte(_partner);
	out->write(_mysteriesSolved, sizeof(_mysteriesSolved));

	// v3: persist the player name so save-slot resume restores it.
	out->writeUint16LE((uint16)_playerName.size());
	out->writeString(_playerName);

	debugC(1, kDebugGeneral,
		   "Saved slot %d: mystery=%u partner=%u name=%s autosave=%d",
		   slot, mysteryNum, _partner, _playerName.c_str(), isAutosave ? 1 : 0);

	Common::Serializer s(nullptr, out);
	s.setVersion(kSaveVer);
	_mystery.syncState(s);

	out->finalize();
	delete out;
	return Common::kNoError;
}

Common::Error EEMEngine::loadGameState(int slot) {
	Common::InSaveFile *in = getSaveFileManager()->openForLoading(getSaveStateName(slot));
	if (!in)
		return Common::kReadingFailed;

	if (in->readUint32BE() != kSaveMagic) {
		delete in;
		return Common::kUnknownError;
	}
	const byte ver = in->readByte();
	if (ver > kSaveVer) {
		delete in;
		return Common::kUnknownError;
	}

	const uint16 mysteryNum = in->readUint16LE();
	_partner = in->readByte();
	if (ver >= 2)
		in->read(_mysteriesSolved, sizeof(_mysteriesSolved));
	else
		memset(_mysteriesSolved, 0, sizeof(_mysteriesSolved));

	if (ver >= 3) {
		const uint16 nameLen = in->readUint16LE();
		Common::String name;
		for (uint16 i = 0; i < nameLen && i < 64; i++)
			name += (char)in->readByte();
		_playerName = name.empty() ? Common::String("Detective") : name;
	}

	if (!_mystery.load(mysteryNum, &_rng)) {
		_mystery.clear();
		delete in;
		return Common::kReadingFailed;
	}

	Common::Serializer s(in, nullptr);
	s.setVersion(ver);
	_mystery.syncState(s);

	delete in;
	debugC(1, kDebugGeneral,
		   "Loaded slot %d: mystery=%u partner=%u name=%s",
		   slot, mysteryNum, _partner, _playerName.c_str());
	return Common::kNoError;
}

void EEMEngine::screenDriver() {
	// Placeholder for the eventual dispatch table (one entry per ScreenId).
	// run() currently calls handlers directly until the title path lands.
}

} // End of namespace EEM
