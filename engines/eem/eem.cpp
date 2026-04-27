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

	auto blitMaskedAt = [&](const Picture &p, int x, int y) {
		const byte transp = (byte)(p.flags >> 8);
		Graphics::Surface *screen = g_system->lockScreen();
		if (!screen) return;
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
		g_system->unlockScreen();
	};

	// Step 4 — cycle through the game animation once before the briefing.
	// Mirrors the `while (uVar9 != gameNum)` loop. The original calls
	// `_UpdateAnimations` per `_CheckFrameRate` tick (~10 fps). We use
	// 100 ms ticks for the same cadence. Click / key skips.
	if (haveGame || haveBook || haveNancy) {
		const uint frameCount = haveGame ? game.size() : 8;
		bool skip = false;
		for (uint frame = 0; frame < frameCount && !shouldQuit() && !skip; frame++) {
			// Restore BG + advance frame.
			if (_picsArchive.getPicture(0x52, bg))
				blitAt(bg, 0, 0);
			if (haveGame)
				blitMaskedAt(game[frame % game.size()], 0xcd, 0x6c);
			if (haveBook)
				blitMaskedAt(book[frame % book.size()], 0, 99);
			if (haveNancy)
				blitMaskedAt(nancy[frame % nancy.size()], 0x68, 0x8b);
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
		blitMaskedAt(game[0], 0xcd, 0x6c);
	if (haveBook)
		blitMaskedAt(book[0], 0, 99);
	if (haveNancy)
		blitMaskedAt(nancy[0], 0x68, 0x8b);
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
		case 1: seqAni = 0x38; seqY = 0x6d; break;
		case 2: seqAni = 0x37; seqY = 0x6c; break;
		case 3: seqAni = 0x39; seqY = 0x6c; break;
		default: break;
		}
	} else {
		switch (caseType) {
		case 2: seqAni = 0x3a; seqY = 0x6c; break;
		case 3: seqAni = 0x3d; seqY = 0x6c; break;
		default: break;
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
					blitMaskedAt(game[frame % game.size()], 0xcd, 0x6c);
				if (haveBook)
					blitMaskedAt(book[frame % book.size()], 0, 99);
				if (haveNancy)
					blitMaskedAt(nancy[frame % nancy.size()], 0x68, 0x8b);
				// Anchor: original blits at `(sx - frame.width,
				// sy - frame.rowoff)`. `frame.rowoff` is the y-anchor
				// in our PicData. We use width/height directly since
				// loadAnimation places anchor at (0, 0).
				const int dstX = (int)0xcd - (int)fr.surface.w;
				const int dstY = (int)seqY - (int)fr.rowoff;
				blitMaskedAt(fr, dstX, dstY);
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

	// Step 6 — case briefing dialogue.
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
				// Per-balloon metadata table verified from 29be:0875 —
				// 10-byte entries indexed by `(bubNum & 0x7f)`. Layout:
				//   +0..1 textX inset, +2..3 textY inset, +4..5 width,
				//   +6..7 height, +8..9 tail offset.
				// 52 entries total; insets vary (3, 5, 6, or 8 px).
				// The original `_DisplayClue` does:
				//   _WordWrap(bubX + table[bubNum].x, bubY + table[bubNum].y,
				//             table[bubNum].w, ...);
				static const struct { uint16 x, y, w; } kBalloonTable[] = {
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
				const uint kBalloonTableSize = sizeof(kBalloonTable) /
											   sizeof(kBalloonTable[0]);
				const uint balloonIdx = balloonId < kBalloonTableSize
										? balloonId : 0;
				const auto &bm = kBalloonTable[balloonIdx];
				textX = bubX + bm.x;
				textY = bubY + bm.y;
				textW = bm.w;
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
	// Mirrors `_DoNotebook @ 161e:0500` + `_DrawNotes @ 161e:01d0` +
	// `_HandleNoteButton @ 161e:03cb`.
	//
	// Layout (verified from Ghidra labels in 29be:013f / 29be:0147):
	//   _NotebookRect = (78, 12, 288, 152)   — note display rectangle.
	//   _NoteButtons (11 entries, 8 bytes each, at 29be:0147):
	//     [0]  (134, 174, 155, 190)  decorative — `_HandleNoteButton(0)`
	//                                returns immediately (i-1 unsigned > 9).
	//     [1]  (93,  174, 115, 190)  → `_InterfaceHelp(0)` (handler 0x3f9)
	//     [2]  (157, 174, 178, 190)  → handler 0x477   (page nav)
	//     [3]  (5,   80,  44, 110)   → `_KDHelp` (host hint, 0x403)
	//     [4]  (180, 174, 201, 190)  → solve / accuse  (0x436)
	//     [5]  (204, 174, 224, 190)  → `_NextScreen = 5` (gallery, 0x489)
	//     [6]  (226, 174, 247, 190)  → handler 0x4ab
	//     [7]  (7,   177,  57, 200)  → handler 0x480   (back to map)
	//     [8]  (35,  111,  56, 136)  → `_NextScreen = 3` (site)
	//     [9]  (0, 0, 0, 0)          → same exit as [8]
	//     [10] (66,  79, 267, 174)   → `_InterfaceHelp(0)` (note area)
	//   Background: PIC 0x3f.
	//   Partner anim: anim 1 (Jake) / 0xb (Jenny) at (5, 80).
	if (!_mystery.isLoaded() || !_font.isLoaded())
		return;

	// Button rects from `_NoteButtons @ 29be:0147` matched to handler
	// addresses via the jump table at `_HandleNoteButton + 0xec` (i.e.
	// 161e:04ec). Decoded handlers (i = rect_index, dispatch = handler[i-1]):
	//   rect 0 (134,155) → no handler (i-1 underflows; original treats
	//                      this as a decorative/no-op slot)
	//   rect 1 (93,115)  → 0x03f9 = `_InterfaceHelp(0)`           (HELP)
	//   rect 2 (157,178) → 0x0477 = `_NextScreen = 5`             (GALLERY)
	//   rect 3 (5,80)    → 0x0403 = `_KDHelp`                     (host hint)
	//   rect 4 (180,201) → 0x0436 = `_SolvedCheck` -> NextScreen=7 (SOLVE)
	//   rect 5 (204,224) → 0x0489 = `_EraseNotes` + `_DrawNotes`  (PAGE NEXT)
	//   rect 6 (226,247) → 0x04ab = decrement CurrentPage + redraw (PAGE PREV)
	//   rect 7 (7,177)   → 0x0480 = `_NextScreen = 2`             (MAP)
	//   rect 8 (35,111)  → 0x03ed = `_NextScreen = 3`             (SITE)
	//   rect 9 (0,0)     → 0x03ed = same as rect 8
	//   rect 10 (66,79)  → 0x03f9 = `_InterfaceHelp(0)`           (note-area help)
	const Common::Rect kNotebookRect(78, 12, 288, 152);
	const Common::Rect kBtnHelp1   ( 93, 174, 115, 190);  // [1] HELP
	const Common::Rect kBtnGallery (157, 174, 178, 190);  // [2] GALLERY
	const Common::Rect kBtnPartner (  5,  80,  44, 110);  // [3] KD HELP
	const Common::Rect kBtnAccuse  (180, 174, 201, 190);  // [4] SOLVE
	const Common::Rect kBtnPageNext(204, 174, 224, 190);  // [5] PAGE NEXT
	const Common::Rect kBtnPagePrev(226, 174, 247, 190);  // [6] PAGE PREV
	const Common::Rect kBtnMap     (  7, 177,  57, 200);  // [7] MAP
	const Common::Rect kBtnSite    ( 35, 111,  56, 136);  // [8] SITE
	const Common::Rect kNoteArea   ( 66,  79, 267, 174);  // [10] note area

	CursorMan.showMouse(true);

	int page = 0;
	int hoveredNoteSlot = -1;

	// Build a list of found-clue indices, identical ordering to the
	// original's iteration through `_CluesFound[]`.
	auto buildFound = [&]() {
		Common::Array<uint> found;
		for (uint i = 0; i < Mystery::kCluesFoundCap; i++)
			if (_mystery._cluesFound[i])
				found.push_back(i);
		return found;
	};

	auto draw = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		// PIC 0x3f frame.
		Picture frame;
		if (_picsArchive.getPicture(0x3f, frame)) {
			const int w = MIN<int>(frame.surface.w, 320);
			const int h = MIN<int>(frame.surface.h, 200);
			for (int row = 0; row < h; row++)
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)frame.surface.getBasePtr(0, row), w);
		}

		// Partner sprite at (5, 80). Anim 1 for Jake, 0xb (11) for Jenny.
		const uint partnerAnim = (_partner == 0) ? 1 : 0xb;
		Animation partnerAni;
		if (_aniArchive.loadAnimation(partnerAnim, partnerAni) && !partnerAni.empty()) {
			const uint32 now = g_system->getMillis();
			const uint frameIdx = (uint)((now / 100) % partnerAni.size());
			const Picture &fr = partnerAni[frameIdx];
			const byte transp = (byte)(fr.flags >> 8);
			for (int row = 0; row < fr.surface.h; row++) {
				const int dstY = 80 + row;
				if (dstY < 0 || dstY >= 200) continue;
				const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < fr.surface.w; col++) {
					const int dstX = 5 + col;
					if (dstX < 0 || dstX >= 320) continue;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
		}

		// Notes — `_DrawNotes` walks `_NoteIndex` for the current page,
		// rendering each found clue's text inside `_NotebookRect` with
		// word-wrap. Selected clues are highlighted (color 0x3c in the
		// original's case-briefing palette).
		const Common::Array<uint> found = buildFound();
		const byte *ni = _mystery.noteIndex();
		const uint16 niCount = _mystery.noteIndexCount();

		const int kRectX = kNotebookRect.left;
		const int kRectY = kNotebookRect.top;
		const int kRectW = kNotebookRect.width();
		const int kRectH = kNotebookRect.height();

		// Walk forward to the start clue of the current page.
		// Each page renders as many clues as fit in `kRectH`.
		int clueCursor = 0;
		Common::Array<int> pageStarts;
		pageStarts.push_back(0);
		{
			const int lineH = _font.getFontHeight() + 1;
			int y = kRectY;
			while (clueCursor < (int)found.size()) {
				const uint clueId = found[clueCursor];
				Common::String txt;
				if (ni && clueId < niCount) {
					const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
					txt = parseString(_mystery.textAt(textOff),
									  _playerName, _partner);
				}
				// Measure height by wrapping the text without drawing.
				Common::Array<Common::String> wrapped;
				_font.wordWrapText(txt, kRectW, wrapped);
				const int h = (int)wrapped.size() * lineH;
				if (y + h + 7 > kRectY + kRectH) {
					// Page break before this clue.
					y = kRectY;
					pageStarts.push_back(clueCursor);
				}
				y += h + 7;
				clueCursor++;
			}
			if (page >= (int)pageStarts.size())
				page = (int)pageStarts.size() - 1;
			if (page < 0)
				page = 0;
		}

		// Track per-slot rectangles so the click handler can map a
		// click in `kNoteArea` back to a clue index.
		Common::Array<Common::Rect> slotRects;
		Common::Array<uint> slotClues;

		const int startClue = (page < (int)pageStarts.size())
								? pageStarts[page] : 0;
		const int endClue   = (page + 1 < (int)pageStarts.size())
								? pageStarts[page + 1] : (int)found.size();

		int y = kRectY;
		for (int i = startClue; i < endClue; i++) {
			const uint clueId = found[i];
			Common::String txt;
			if (ni && clueId < niCount) {
				const uint16 textOff = READ_LE_UINT16(ni + clueId * 4);
				txt = parseString(_mystery.textAt(textOff),
								  _playerName, _partner);
			}
			if (txt.empty())
				txt = Common::String::format("clue %u", clueId);
			// Per `_DrawNotes @ 161e:01d0`: text uses
			// `_NoteUnselectedColor` (0x5c=cyan) for unselected and 0x3c
			// (light yellow-white) for selected. Both contrast cleanly
			// with the PDA screen's natural blue, so we draw text
			// directly on PIC 0x3f without an extra fill rectangle —
			// matches the original design.
			Common::Array<Common::String> wrapped;
			_font.wordWrapText(txt, kRectW, wrapped);
			const int lineH = _font.getFontHeight() + 1;
			const int h = (int)wrapped.size() * lineH;
			const byte color = _mystery._noteSelected[clueId] ? 0x3C : 0x5C;
			for (uint li = 0; li < wrapped.size(); li++) {
				_font.drawString(&scratch, wrapped[li], kRectX,
								 y + (int)li * lineH, kRectW, color);
			}
			slotRects.push_back(Common::Rect(kRectX, y,
											  kRectX + kRectW, y + h));
			slotClues.push_back(clueId);
			y += h + 7;
		}

		// Page indicator + selected-points counter directly on PIC.
		_font.drawString(&scratch, Common::String::format("p%d/%d",
								   page + 1, (int)pageStarts.size()),
						 270, 4, 320, 0x5C);
		_font.drawString(&scratch, Common::String::format("%d pts",
								   _mystery.selectedPoints()),
						 270, 14, 320, 0x5C);
		(void)hoveredNoteSlot;

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();

		// Stash slot info on the captures so the click handler below
		// can use it via the closure.
		_notebookSlotRects = slotRects;
		_notebookSlotClues = slotClues;
	};

	draw();

	uint32 lastDraw = g_system->getMillis();

	while (!shouldQuit()) {
		Common::Event ev;
		bool dirty = false;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				exitFlag = true;
				break;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitFlag = true;
					break;
				}
				if (ev.kbd.keycode == Common::KEYCODE_LEFT ||
					ev.kbd.keycode == Common::KEYCODE_PAGEUP) {
					if (page > 0) page--;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_RIGHT ||
						   ev.kbd.keycode == Common::KEYCODE_PAGEDOWN ||
						   ev.kbd.keycode == Common::KEYCODE_TAB) {
					page++;
					dirty = true;
				} else if (ev.kbd.keycode == Common::KEYCODE_h) {
					doHelp();
					dirty = true;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// Test buttons in the order the original would —
				// button 0 / 9 are dead zones, so check the actionable
				// rects directly. Earlier rects "win" when overlapping
				// (matches `_FindButton`).
				if (kBtnSite.contains(ev.mouse.x, ev.mouse.y)) {
					exitFlag = true;
					break;  // back to site
				}
				if (kBtnMap.contains(ev.mouse.x, ev.mouse.y)) {
					doBigMap();
					exitFlag = true;
					break;
				}
				if (kBtnPartner.contains(ev.mouse.x, ev.mouse.y)) {
					doHelp();              // _KDHelp = host hint
					dirty = true;
					continue;
				}
				if (kBtnAccuse.contains(ev.mouse.x, ev.mouse.y)) {
					doAccuse();
					exitFlag = true;
					break;
				}
				if (kBtnGallery.contains(ev.mouse.x, ev.mouse.y)) {
					doGallery();
					dirty = true;
					continue;
				}
				if (kBtnHelp1.contains(ev.mouse.x, ev.mouse.y)) {
					// rect 1 → `_InterfaceHelp(0)`: walks `HelpData[0]` and
					// blits PICs 0x63 / 0x1ae fullscreen for click-through.
					doInterfaceHelp(0);
					dirty = true;
					continue;
				}
				if (kBtnPagePrev.contains(ev.mouse.x, ev.mouse.y)) {
					if (page > 0) page--;
					dirty = true;
					continue;
				}
				if (kBtnPageNext.contains(ev.mouse.x, ev.mouse.y)) {
					page++;
					dirty = true;
					continue;
				}
				if (kNoteArea.contains(ev.mouse.x, ev.mouse.y)) {
					// Toggle the selection on whichever clue's text
					// the click landed in. The original calls
					// `_InterfaceHelp` here; that's the help screen,
					// not selection — selection is in the Accuse
					// screen. We use the area for selection because
					// keyboard 1..9 toggling is awkward, and the
					// resulting `_NoteSelected` state is what
					// `_SolvedCheck` reads.
					for (uint i = 0; i < _notebookSlotRects.size(); i++) {
						if (_notebookSlotRects[i].contains(ev.mouse.x,
														   ev.mouse.y)) {
							const uint clueId = _notebookSlotClues[i];
							_mystery._noteSelected[clueId] ^= 1;
							dirty = true;
							break;
						}
					}
					continue;
				}
			}
		}
		if (exitFlag)
			break;

		const uint32 now = g_system->getMillis();
		// Re-render every 100 ms so the partner sprite cycles frames.
		if (dirty || now - lastDraw >= 100) {
			draw();
			lastDraw = now;
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
}

void EEMEngine::doGallery() {
	// Mirrors `_DoGallery @ 158f:065b` and `_DrawGallery @ 158f:0046`.
	// Verified directly from the disassembly:
	//   * Background: PIC 0x3f (same as PDA).
	//   * Partner sprite at (5, 0x50): anim 2 (Jake) / 0x10 (Jenny).
	//     `_NewAnimation(5, 0x50, ...)`. NOTE: gallery uses anim 2/0x10,
	//     PDA uses 1/0xb — different sprites.
	//   * Five fixed slot positions at `29be:0x116` (4 bytes per slot,
	//     `{u16 x, u16 y}`):
	//         slot 0 = ( 83,  14)   slot 3 = (119,  90)
	//         slot 1 = (155,  14)   slot 4 = (191,  90)
	//         slot 2 = (227,  14)
	//   * For each logical suspect i in 0..NumSuspects-1:
	//         picId   = `*(u16 *)(_GalleryData + i * 0x46)` (entry +0).
	//         visible = `_InGallery[_NewOrder[i]] != 0`.
	//         drawX   = positions[_NewOrder[i]].x
	//         drawY   = positions[_NewOrder[i]].y + (0x48 - pic.height)
	//     So portraits are BOTTOM-aligned to baselines 0x48 + pos.y.
	//   * Click on portrait via `_SearchSuspects` → `MoreInfo(i)` shows
	//     the suspect detail page. ESC returns to PDA.
	//   * Frame-cycled @ 100ms via `_CheckFrameRate` + `_UpdateAnimations`
	//     + `_GizmoColorCycle`.
	if (!_mystery.isLoaded())
		return;

	const byte *gd = _mystery.galleryData();
	if (!gd) {
		warning("doGallery: no GalleryData in mystery %u", _mystery.number());
		return;
	}

	CursorMan.showMouse(true);

	struct Slot { int x; int y; };
	static const Slot kGallerySlots[5] = {
		{  83,  14 }, // 0
		{ 155,  14 }, // 1
		{ 227,  14 }, // 2
		{ 119,  90 }, // 3
		{ 191,  90 }  // 4
	};

	// Pre-load static elements once.
	Picture galBg;
	const bool haveBg = _picsArchive.getPicture(0x3f, galBg);

	// Gallery partner anim — `_DoGallery` calls `_GetAnimation(uVar6)` with
	// uVar6 = 2 (Jake) / 0x10 (Jenny). Different from PDA (1 / 0xb).
	const uint partnerAnim = (_partner == 0) ? 2 : 0x10;
	Animation partnerAni;
	const bool havePartner = _aniArchive.loadAnimation(partnerAnim, partnerAni)
							  && !partnerAni.empty();

	const uint8 num = _mystery.numSuspects();

	// Cache slot rects for click hit-testing.
	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect; // logical suspect index in [0, num)
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++) {
		slotSuspect[i] = -1;
	}

	auto drawFrame = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();

		if (haveBg) {
			const int bw = MIN<int>(galBg.surface.w, 320);
			const int bh = MIN<int>(galBg.surface.h, 200);
			for (int row = 0; row < bh; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)galBg.surface.getBasePtr(0, row), bw);
			}
		}

		// Partner sprite frame @ (5, 0x50).
		if (havePartner) {
			const uint32 now = g_system->getMillis();
			const uint frameIdx = (uint)((now / 100) % partnerAni.size());
			const Picture &fr = partnerAni[frameIdx];
			const byte transp = (byte)(fr.flags >> 8);
			const int px = 5, py = 0x50;
			for (int row = 0; row < fr.surface.h; row++) {
				const int dstY = py + row;
				if (dstY < 0 || dstY >= 200) continue;
				const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < fr.surface.w; col++) {
					const int dstX = px + col;
					if (dstX < 0 || dstX >= 320) continue;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
		}

		// Portraits — `_DrawGallery @ 158f:0046` walks suspects 0..N-1
		// and only renders those flagged in `_InGallery[NewOrder[i]]`.
		// Undiscovered slots are left empty in the original. We render
		// a darkened placeholder + "?" so the player has visual feedback
		// that suspects exist but are still unknown.
		uint discoveredCount = 0;
		for (uint i = 0; i < num && i < Mystery::kGalleryCap; i++) {
			slotRects[i] = Common::Rect();
			slotSuspect[i] = -1;

			const uint8 phys = _mystery._newOrder[i];
			if (phys >= 5)
				continue;
			const Slot &s = kGallerySlots[phys];

			const bool discovered = _mystery._inGallery[phys] != 0;
			if (discovered) {
				const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
				Picture portrait;
				if (picId == 0 ||
					!_picsArchive.getPicture(picId, portrait))
					continue;

				const int placeX = s.x;
				const int placeY = s.y + (0x48 - portrait.surface.h);
				const byte transp = (byte)(portrait.flags >> 8);
				const int w = MIN<int>(portrait.surface.w, 320 - placeX);
				const int h = MIN<int>(portrait.surface.h, 200 - placeY);
				if (w <= 0 || h <= 0)
					continue;
				for (int row = 0; row < h; row++) {
					const int dstY = placeY + row;
					if (dstY < 0) continue;
					const byte *src =
						(const byte *)portrait.surface.getBasePtr(0, row);
					byte *dst = (byte *)scratch.getBasePtr(0, dstY);
					for (int col = 0; col < w; col++) {
						const int dstX = placeX + col;
						if (src[col] != transp)
							dst[dstX] = src[col];
					}
				}
				slotRects[i] = Common::Rect(placeX, placeY,
											 placeX + w, placeY + h);
				slotSuspect[i] = (int)i;
				discoveredCount++;
			} else {
				// Undiscovered placeholder — small framed "?" box at
				// (s.x, s.y) sized 0x40 × 0x48 (typical portrait size).
				const int phW = 0x40, phH = 0x48;
				const int phX = s.x, phY = s.y;
				if (phX + phW <= 320 && phY + phH <= 200) {
					scratch.fillRect(Common::Rect(phX, phY,
						phX + phW, phY + phH), 0x20);
					scratch.frameRect(Common::Rect(phX, phY,
						phX + phW, phY + phH), 0x5C);
					if (_font.isLoaded()) {
						_font.drawString(&scratch, "?",
							phX + phW / 2 - 3,
							phY + phH / 2 - 4, phW, 0x5C);
					}
				}
			}
		}
		(void)discoveredCount;

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	drawFrame();
	uint32 lastDraw = g_system->getMillis();

	while (!shouldQuit()) {
		Common::Event ev;
		bool exitFlag = false;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN) {
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					exitFlag = true;
					break;
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				// PDA bottom-bar buttons mirror `_NoteButtons @ 29be:0147`.
				// `_DoGallery @ 158f:065b` shares the SAME button table
				// with `_DoNotebook` (both call `_FindButton` against the
				// 11-entry table at 0x147). `_HandleGalleryButton @
				// 158f:05c0` dispatches via a different jump table
				// (158f:0645). Verified gallery button mapping:
				//   rect 0 (134,155) → 0x05ef = `_NextScreen = 4` (NOTEBOOK)
				//   rect 1 (93,115)  → 0x0625 = `_InterfaceHelp` (HELP)
				//   rect 2 (157,178) → 0x0638 = generic exit (no-op)
				//   rect 3 (5,80)    → 0x061e = `_KDHelp` (host hint)
				//   rect 4 (180,201) → 0x05ff = `_SolvedCheck` -> SOLVE
				//   rect 5 (204,224) → 0x0638 = generic exit
				//   rect 6 (226,247) → 0x0638 = generic exit
				//   rect 7 (7,177)   → 0x05f7 = `_NextScreen = 2` (MAP)
				//   rect 8 (35,111)  → 0x05e4 = `_NextScreen = 3` (SITE)
				const Common::Rect kBtnSite    ( 35, 111,  56, 136); // [8] SITE
				const Common::Rect kBtnMap     (  7, 177,  57, 200); // [7] MAP
				const Common::Rect kBtnAccuse  (180, 174, 201, 190); // [4] SOLVE
				const Common::Rect kBtnNotebook(134, 174, 155, 190); // [0] NOTEBOOK (back to PDA notes)
				const Common::Rect kBtnHelp    ( 93, 174, 115, 190); // [1] HELP
				const Common::Rect kBtnPartner (  5,  80,  44, 110); // [3] KD HELP
				if (kBtnSite.contains(ev.mouse.x, ev.mouse.y)) {
					exitFlag = true; break;
				}
				if (kBtnMap.contains(ev.mouse.x, ev.mouse.y)) {
					doBigMap();
					exitFlag = true; break;
				}
				if (kBtnAccuse.contains(ev.mouse.x, ev.mouse.y)) {
					doAccuse();
					exitFlag = true; break;
				}
				if (kBtnNotebook.contains(ev.mouse.x, ev.mouse.y)) {
					// Already came from notebook; exiting returns to it.
					exitFlag = true; break;
				}
				if (kBtnHelp.contains(ev.mouse.x, ev.mouse.y)) {
					// Gallery rect 1 → `_InterfaceHelp(0)` per jmp table at
					// 158f:0625 (HandleGalleryButton). Same picture sequence
					// as the notebook HELP button.
					doInterfaceHelp(0);
					lastDraw = 0;
					continue;
				}
				if (kBtnPartner.contains(ev.mouse.x, ev.mouse.y)) {
					doHelp();
					lastDraw = 0;
					continue;
				}
				// `_SearchSuspects` walks the per-slot rects and returns
				// the suspect index. We mirror that with cached rects.
				bool clicked = false;
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0) continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						// `MoreInfo(i)` — show the suspect detail page.
						// Mirrors `MoreInfo @ 158f:0419`:
						//   _RefreshGalleryBackground();
						//   _GetPicture(*(u16*)(gd + i*0x46));
						//   _AddPicBackground(pic, 0x94, 0xf);
						//   _DrawGalleryNotes(gd + i*0x46);
						//   loop until ESC or button click.
						// Suspect data layout (verified against M1):
						//   +0..1: picId (used here AND for gallery slot)
						//   +8..9: number of clues for this suspect
						//   +0xa..??: array of u16 clue IDs (terminated
						//             by 0xFFFF if shorter than count).
						const uint suspectIdx = (uint)slotSuspect[i];
						const byte *suspect = gd + suspectIdx * 0x46;
						const uint16 detailPic =
							READ_LE_UINT16(suspect + 0);
						const uint16 clueCount =
							READ_LE_UINT16(suspect + 8);

						Graphics::ManagedSurface ms(320, 200,
							Graphics::PixelFormat::createFormatCLUT8());
						ms.clear();
						if (haveBg) {
							const int bw = MIN<int>(galBg.surface.w, 320);
							const int bh = MIN<int>(galBg.surface.h, 200);
							for (int row = 0; row < bh; row++) {
								memcpy((byte *)ms.getBasePtr(0, row),
									   (const byte *)galBg.surface.getBasePtr(0, row), bw);
							}
						}
						// Full suspect picture at (0x94, 0xf).
						Picture detail;
						if (_picsArchive.getPicture(detailPic, detail)) {
							const byte transp =
								(byte)(detail.flags >> 8);
							const int dx = 0x94, dy = 0x0f;
							const int dw = MIN<int>(detail.surface.w, 320 - dx);
							const int dh = MIN<int>(detail.surface.h, 200 - dy);
							for (int row = 0; row < dh; row++) {
								const byte *src =
									(const byte *)detail.surface.getBasePtr(0, row);
								byte *dst =
									(byte *)ms.getBasePtr(0, dy + row);
								for (int col = 0; col < dw; col++) {
									if (src[col] != transp)
										dst[dx + col] = src[col];
								}
							}
						}
						// Suspect's clue notes inside _GalleryNoteRect
						// = (78, 93, 288, 152), per 29be:0100. Cyan text
						// renders directly on the PDA's natural blue
						// screen — matches `_DrawGalleryNotes @ 158f:01f4`.
						const int rx = 78, ry = 93;
						const int rw = 288 - 78, rh = 152 - 93;

						const byte *ni = _mystery.noteIndex();
						const uint16 niCount = _mystery.noteIndexCount();
						int yPos = ry;
						const int lineH = _font.getFontHeight() + 1;
						bool drewAny = false;
						for (uint k = 0; k < clueCount && k < 30; k++) {
							const uint16 clueId =
								READ_LE_UINT16(suspect + 0xa + k * 2);
							if (clueId == 0xFFFF) break;
							if (clueId >= Mystery::kCluesFoundCap ||
								!_mystery._cluesFound[clueId])
								continue;
							if (!ni || clueId >= niCount) continue;
							const uint16 textOff =
								READ_LE_UINT16(ni + clueId * 4);
							Common::String txt =
								parseString(_mystery.textAt(textOff),
											_playerName, _partner);
							if (txt.empty()) continue;
							const byte color =
								_mystery._noteSelected[clueId] ? 0x3C : 0x5C;
							const int hLine = _font.drawWordWrapped(
								&ms, rx, yPos, rw, txt, color);
							yPos += hLine + 7;
							drewAny = true;
							if (yPos + lineH > ry + rh) break;
						}
						if (!drewAny && _font.isLoaded()) {
							_font.drawString(&ms,
								"No clues yet for this suspect.",
								rx, ry, rw, 0x5C);
						}
						// Header / footer text.
						if (_font.isLoaded()) {
							_font.drawString(&ms, "SUSPECT FILE",
											  rx, ry - 11, rw, 0x3C);
							_font.drawString(&ms, "(click / ESC: back)",
											  rx, ry + rh + 2, rw, 0x3C);
						}
						g_system->copyRectToScreen(ms.getPixels(),
							ms.pitch, 0, 0, 320, 200);
						g_system->updateScreen();

						// Wait for click or ESC. Drain the queued
						// LBUTTONDOWN that triggered this MoreInfo first
						// so we don't immediately accept it as the
						// dismiss event.
						g_system->delayMillis(150);
						{
							Common::Event drain;
							while (g_system->getEventManager()->pollEvent(drain)) {
								if (drain.type == Common::EVENT_QUIT ||
									drain.type == Common::EVENT_RETURN_TO_LAUNCHER)
									return;
							}
						}
						bool back = false;
						while (!back && !shouldQuit()) {
							Common::Event e2;
							while (g_system->getEventManager()->pollEvent(e2)) {
								if (e2.type == Common::EVENT_LBUTTONDOWN ||
									(e2.type == Common::EVENT_KEYDOWN &&
									 (e2.kbd.keycode == Common::KEYCODE_ESCAPE ||
									  e2.kbd.keycode == Common::KEYCODE_RETURN))) {
									back = true;
									break;
								}
								if (e2.type == Common::EVENT_QUIT ||
									e2.type == Common::EVENT_RETURN_TO_LAUNCHER)
									return;
							}
							g_system->delayMillis(20);
						}
						// Force gallery redraw immediately so the
						// player isn't left looking at the dismissed
						// MoreInfo screen until the next 100 ms tick.
						drawFrame();
						lastDraw = g_system->getMillis();
						clicked = true;
						break;
					}
				}
				(void)clicked;
			}
		}
		if (exitFlag) break;

		const uint32 now = g_system->getMillis();
		if (now - lastDraw >= 100) {
			drawFrame();
			lastDraw = now;
		}
		g_system->delayMillis(15);
	}
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

void EEMEngine::doInterfaceHelp(uint num) {
	// Mirrors `_InterfaceHelp(num)` @ 1560:0205. The original walks
	// `HelpData @ 29be:00c8` (5-byte entries: u8 count, then up to 2
	// u16 picIds), `_GetPicture`s each one, blits it fullscreen via
	// `_Rect_Move_Mask(0, 0, ...)`, and waits for click / key. ESC ends
	// the cycle; any other input advances to the next pic.
	//
	// Verified from Ghidra HelpData bytes:
	//   entry 0 (PDA / gallery HELP button): count=2, picIds = 0x0063, 0x01ae
	//   entry 1: count=2, picIds = 0x0192, 0x01b1
	// Only entry 0 is reachable from the PDA notebook (rect 1) and the
	// gallery (rect 1) — both call `_InterfaceHelp(0)`.
	static const uint16 kHelpPics[][2] = {
		{ 0x0063, 0x01ae },
		{ 0x0192, 0x01b1 },
	};
	if (num >= ARRAYSIZE(kHelpPics))
		return;

	debugC(1, kDebugScript, "doInterfaceHelp(%u): showing pics 0x%x, 0x%x",
		   num, kHelpPics[num][0], kHelpPics[num][1]);

	for (uint i = 0; i < 2; i++) {
		const uint16 picId = kHelpPics[num][i];
		Picture pic;
		if (!_picsArchive.getPicture(picId, pic)) {
			warning("doInterfaceHelp: getPicture(0x%x) failed", picId);
			continue;
		}
		debugC(1, kDebugScript, "doInterfaceHelp: pic 0x%x = %dx%d flags=0x%x",
			   picId, pic.surface.w, pic.surface.h, pic.flags);

		// Compose a 320x200 frame (cleared) and blit the help pic at (0,0)
		// with the original's masked-blit semantics: pixels equal to the
		// pic's sub-mode (high byte of `pic[0]`, see `_Rect_Move_Mask`
		// param_10 at 1000:03fc) are treated as transparent and skipped.
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		const byte transp = (byte)(pic.flags >> 8);
		const int w = MIN<int>(pic.surface.w, 320);
		const int h = MIN<int>(pic.surface.h, 200);
		for (int row = 0; row < h; row++) {
			const byte *src = (const byte *)pic.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, row);
			for (int col = 0; col < w; col++) {
				if (src[col] != transp)
					dst[col] = src[col];
			}
		}
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
}

uint16 EEMEngine::getKDTextBalloon(byte firstChar) const {
	// Mirrors `_GetKDTextBalloon @ 1df2:0105`:
	//   if ((ctype[firstChar] & 2) == 0)  bub = *(u16*)29be:1068 = 0x17
	//   else                              bub = *(u16*)(29be:0fe6+0x1e+c*2)
	// `ctype` is Borland's `_ctype_` array at `29be:2be1`. Bit 1 (0x02) is
	// set only for digits '0'..'9' (verified by reading the table — '0'..'9'
	// each map to byte 0x02; everything else has bit 1 clear).
	// Lookup table at 29be:1064 (= 29be:0fe6 + 0x1e + '0'*2):
	//   '0'→0x15  '1'→0x16  '2'→0x17  '3'→0x18  '4'→0x19
	//   '5'→0x1a  '6'→0x20  '7'→0x21  '8'→0x22  '9'→0x1e
	// Note `*(u16*)29be:1068` (= entry for '2') is the same byte the
	// non-digit fallback returns — the original encodes the constant by
	// reusing the digit-2 slot.
	if (firstChar < '0' || firstChar > '9')
		return 0x17;
	static const uint16 kDigitBalloons[10] = {
		0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x20, 0x21, 0x22, 0x1e
	};
	return kDigitBalloons[firstChar - '0'];
}

void EEMEngine::setPartnerEraseBg(const Graphics::ManagedSurface *bg) {
	if (bg && bg->w == 320 && bg->h == 200) {
		_partnerEraseBg.create(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		for (int row = 0; row < 200; row++) {
			memcpy((byte *)_partnerEraseBg.getBasePtr(0, row),
				   (const byte *)bg->getBasePtr(0, row), 320);
		}
	} else {
		_partnerEraseBg.free();
	}
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
	// Six valid kdAnimNum entries (0..5). Verified bytes from
	// `29be:0228`. Layout per entry: { animJake, animJenny, xJake,
	// xJenny, yJake, yJenny }. Position is (6, 80) in every entry.
	static const uint16 kKdAnimTable[6][6] = {
		{ 0x03, 0x0c, 6, 6, 80, 80 }, // 0 — speaker idx 1 wait anim
		{ 0x01, 0x0b, 6, 6, 80, 80 }, // 1 — same as PDA idle
		{ 0x04, 0x0d, 6, 6, 80, 80 }, // 2
		{ 0x02, 0x10, 6, 6, 80, 80 }, // 3 — same as gallery
		{ 0x05, 0x05, 6, 6, 80, 80 }, // 4 — same anim both partners
		{ 0x06, 0x06, 6, 6, 80, 80 }, // 5 — same anim both partners
	};
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

	// Sequence-script lookup. Entries copied verbatim from
	// `_AnimationSequences @ 29be:22d4` walked through to the next 0x80.
	// Each script is a u16[] of frame indices terminated by 0x80; we
	// don't yet handle 0x81 jumps (none of the kdAnim sequences use
	// them — verified). seqnum == animId for these calls (per
	// `_PlayAnimation` 172b:1f5d push order).
	struct Script {
		uint16 seqnum;
		uint8 len;
		uint8 frames[20];  // long enough for any kdAnim script
	};
	static const Script kScripts[] = {
		// seqnum 1 (29be:188a) — head bob
		{ 0x01, 15, { 0,1,2,0,1,0,2,1,0,1,0,1,2,1,0 } },
		// seqnum 2 (29be:18aa) — short blip then long pause
		{ 0x02, 16, { 0,1,2,1,0,0,0,0,0,0,0,0,0,0,0,0 } },
		// seqnum 3 (29be:18e0) — Jake "lift, hold, lower" gesture
		{ 0x03,  9, { 0,1,2,3,2,2,2,1,0 } },
		// seqnum 4 (29be:18f4) — bigger gesture (camera flash-style)
		{ 0x04, 13, { 0,1,2,3,4,5,4,4,4,3,2,1,0 } },
		// seqnum 5 (29be:1910) — held idle with a single peak
		{ 0x05, 13, { 0,0,0,1,2,3,2,1,0,0,0,0,0 } },
		// seqnum 6 (29be:192c) — empty (immediate END)
		{ 0x06,  0, { 0 } },
		// seqnum 0xb (29be:188a, same as 1) — Jenny PDA idle
		{ 0x0b, 15, { 0,1,2,0,1,0,2,1,0,1,0,1,2,1,0 } },
		// seqnum 0xc (29be:18e0, same as 3) — Jenny "take a picture"
		{ 0x0c,  9, { 0,1,2,3,2,2,2,1,0 } },
		// seqnum 0xd (29be:18f4, same as 4) — Jenny big gesture
		{ 0x0d, 13, { 0,1,2,3,4,5,4,4,4,3,2,1,0 } },
		// seqnum 0x10 (29be:1956) — Jenny short anim
		{ 0x10,  9, { 0,0,0,1,0,0,0,0,0 } },
	};
	const uint8 *frames = nullptr;
	uint frameCount = 0;
	for (uint i = 0; i < ARRAYSIZE(kScripts); i++) {
		if (kScripts[i].seqnum == animId) {
			frames = kScripts[i].frames;
			frameCount = kScripts[i].len;
			break;
		}
	}
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
		const int w = MIN<int>(fr.surface.w, 320 - px);
		const int h = MIN<int>(fr.surface.h, 200 - py);
		for (int row = 0; row < h; row++) {
			const int dstY = py + row;
			if (dstY < 0) continue;
			const byte *src = (const byte *)fr.surface.getBasePtr(0, row);
			byte *dst = (byte *)scratch.getBasePtr(0, dstY);
			for (int col = 0; col < w; col++) {
				const int dstX = px + col;
				if (dstX < 0) continue;
				if (src[col] != transp)
					dst[dstX] = src[col];
			}
		}
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

bool EEMEngine::getBalloonInsets(uint16 bubNum, uint16 &xInset,
								  uint16 &yInset, uint16 &textW) const {
	// 52-entry, 10-bytes-each balloon-metadata table at `29be:0875`.
	// Used at 1df2:0aef-0af9 (accuse hint) and `_DisplayClue` to position
	// `_WordWrap` text inside the balloon. Only +0/+2/+4 are read here:
	//   +0..1 = text X inset, +2..3 = Y inset, +4..5 = max wrap width
	// (+6/+8 = balloon h / tail offset, both unused for text layout).
	static const struct { uint16 x, y, w; } kTable[] = {
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
	const uint idx = bubNum & 0x7F;
	if (idx >= ARRAYSIZE(kTable))
		return false;
	xInset = kTable[idx].x;
	yInset = kTable[idx].y;
	textW  = kTable[idx].w;
	return true;
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

	// Mirrors `_DoAccuseGallery @ 1df2:0a31`:
	//   1. Show KD's hint balloon (KDTextIndex[+8] text).
	//   2. `_GetBackground(0x3f)` — same backdrop as PDA / gallery.
	//   3. `_DrawGallery()` — renders portraits at the standard 5 slots
	//      (positions verified at 29be:0x116, bottom-aligned baseline 0x48).
	//   4. Click loop dispatching on `_NoteButtons` (same table as PDA)
	//      with a separate `_HandleAccuseNoteButton` jump table.
	const uint8 num = _mystery.numSuspects();
	if (num == 0)
		return;

	const byte *gd = _mystery.galleryData();

	// Verbatim from 29be:0x116 — same five suspect slot positions as
	// `_DrawGallery @ 158f:0046`.
	struct Slot { int x; int y; };
	static const Slot kGallerySlots[5] = {
		{  83,  14 }, { 155,  14 }, { 227,  14 },
		{ 119,  90 }, { 191,  90 }
	};

	Picture accuseBg;
	const bool haveAccuseBg = _picsArchive.getPicture(0x3f, accuseBg);

	Common::Array<Common::Rect> slotRects;
	Common::Array<int> slotSuspect;
	slotRects.resize(num);
	slotSuspect.resize(num);
	for (uint i = 0; i < num; i++)
		slotSuspect[i] = -1;

	int highlighted = 0;
	auto drawGallery = [&]() {
		Graphics::ManagedSurface scratch(320, 200,
			Graphics::PixelFormat::createFormatCLUT8());
		scratch.clear();
		if (haveAccuseBg) {
			const int bw = MIN<int>(accuseBg.surface.w, 320);
			const int bh = MIN<int>(accuseBg.surface.h, 200);
			for (int row = 0; row < bh; row++) {
				memcpy((byte *)scratch.getBasePtr(0, row),
					   (const byte *)accuseBg.surface.getBasePtr(0, row), bw);
			}
		}

		for (uint i = 0; i < num && i < Mystery::kGalleryCap; i++) {
			slotRects[i] = Common::Rect();
			slotSuspect[i] = -1;
			if (!gd) continue;
			const uint8 phys = _mystery._newOrder[i];
			if (phys >= 5) continue;
			// `_DrawGallery @ 158f:00b9` skips suspects whose
			// `_InGallery[phys]` flag is 0 — that's the original gate
			// (some suspects only become visible after being met or
			// stay hidden after a wrong accusation removes them).
			if (_mystery._inGallery[phys] == 0) continue;
			const Slot &s = kGallerySlots[phys];

			const uint16 picId = READ_LE_UINT16(gd + i * 0x46);
			if (picId == 0) continue;
			Picture portrait;
			if (!_picsArchive.getPicture(picId, portrait))
				continue;

			const int placeX = s.x;
			const int placeY = s.y + (0x48 - portrait.surface.h);
			const byte transp = (byte)(portrait.flags >> 8);
			const int w = MIN<int>(portrait.surface.w, 320 - placeX);
			const int h = MIN<int>(portrait.surface.h, 200 - placeY);
			if (w <= 0 || h <= 0) continue;
			for (int row = 0; row < h; row++) {
				const int dstY = placeY + row;
				if (dstY < 0) continue;
				const byte *src =
					(const byte *)portrait.surface.getBasePtr(0, row);
				byte *dst = (byte *)scratch.getBasePtr(0, dstY);
				for (int col = 0; col < w; col++) {
					const int dstX = placeX + col;
					if (src[col] != transp)
						dst[dstX] = src[col];
				}
			}
			slotRects[i] = Common::Rect(placeX, placeY,
										 placeX + w, placeY + h);
			slotSuspect[i] = (int)i;
		}

		// Highlight indicator. The original moves the mouse cursor
		// to the centre of the highlighted suspect via `_PutMouseInRect`
		// (1df2:0b8e) — we draw a 1px outline in palette index 0xFE
		// (within the marching-ants cycle range 0xF9..0xFE) which is
		// unambiguously visible under any palette without warping the
		// player's cursor.
		if (highlighted >= 0 && highlighted < (int)slotRects.size() &&
			!slotRects[highlighted].isEmpty()) {
			Common::Rect r = slotRects[highlighted];
			r.grow(1);
			scratch.frameRect(r, 0xFE);
		}

		g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
								   0, 0, 320, 200);
		g_system->updateScreen();
	};

	// Step 1 — KD hint balloon. Mirrors `_DoAccuseGallery @ 1df2:0a31`
	// (1df2:0a4c-1df2:0afe):
	//   text  = TextBlock + KDTextIndex[+8]               (1df2:0a4c-0a57)
	//   bub   = _GetKDTextBalloon(text[0])                (1df2:0a6d)
	//   GetBalloon(bub)                                   (1df2:0a7c)
	//   y     = (h < 0x4e) ? (0x50 - h) >> 1 : 1          (1df2:0a8b-0aa5)
	//   AddPicBackground(pic, 0x21, y)                    (1df2:0aab)
	//   WordWrap(0x21+tbl[bub].x, y+tbl[bub].y, tbl[bub].w, text, color=0)
	//     tbl @ 29be:0875, 10-byte entries (1df2:0ad6-0af1)
	const byte *kdIdx = _mystery.kdTextIndex();
	if (kdIdx) {
		const int16 textOff = (int16)READ_LE_UINT16(kdIdx + 8);
		if (textOff != -1) {
			const char *raw = _mystery.textAt((uint16)textOff);
			Common::String hint =
				parseString(raw ? raw : "", _playerName, _partner);
			if (!hint.empty()) {
				// First-char dispatch via getKDTextBalloon (1df2:0105).
				// Note: we pass the *parsed* first char; the original
				// reads it BEFORE `_ParseString`, but the player-name /
				// partner-name substitutions never start with digits, so
				// the dispatch result is the same either way.
				const byte firstChar =
					hint.empty() ? (byte)0 : (byte)hint[0];
				const uint16 bubNum = getKDTextBalloon(firstChar);
				Picture balloon;
				const bool haveBalloon =
					_balloonArchive.size() > (bubNum & 0x7F) &&
					_balloonArchive.loadEntry(bubNum & 0x7F, balloon);

				// 1df2:0a8b-1df2:0aa5: y = (h < 0x4e) ? (0x50-h)>>1 : 1
				const int balloonX = 0x21;
				int balloonY = 1;
				if (haveBalloon && balloon.surface.h < 0x4e)
					balloonY = (0x50 - balloon.surface.h) / 2;

				Graphics::ManagedSurface ms(320, 200,
					Graphics::PixelFormat::createFormatCLUT8());
				ms.clear();
				if (haveAccuseBg) {
					const int bw = MIN<int>(accuseBg.surface.w, 320);
					const int bh = MIN<int>(accuseBg.surface.h, 200);
					for (int row = 0; row < bh; row++) {
						memcpy((byte *)ms.getBasePtr(0, row),
							   (const byte *)accuseBg.surface.getBasePtr(0, row), bw);
					}
				}
				// Masked balloon blit — `_Rect_Move_Mask` (1000:03fc)
				// skips pixels equal to `pic[0] >> 8`.
				if (haveBalloon) {
					const byte transp = (byte)(balloon.flags >> 8);
					const int bw = MIN<int>(balloon.surface.w, 320 - balloonX);
					const int bh = MIN<int>(balloon.surface.h, 200 - balloonY);
					for (int row = 0; row < bh; row++) {
						const byte *src = (const byte *)balloon.surface.getBasePtr(0, row);
						byte *dst = (byte *)ms.getBasePtr(balloonX, balloonY + row);
						for (int col = 0; col < bw; col++) {
							if (src[col] != transp)
								dst[col] = src[col];
						}
					}
				}
				// Inset table @ 29be:0875 — 1df2:0acb pushes color=0.
				uint16 tx = 5, ty = 4, tw = 155;
				getBalloonInsets(bubNum, tx, ty, tw);
				if (_font.isLoaded()) {
					_font.drawWordWrapped(&ms, balloonX + tx,
										  balloonY + ty, tw, hint,
										  haveBalloon ? 0 : 0xF);
				}
				g_system->copyRectToScreen(ms.getPixels(), ms.pitch,
					0, 0, 320, 200);
				g_system->updateScreen();
				waitForInput(8000);
			}
		}
	}

	// Helper to find the next "alive" slot (one whose `_inGallery[phys]`
	// flag is still set so a portrait was actually drawn). Mirrors the
	// way the original wraps DI past empty slots.
	auto nextLiveSlot = [&](int from, int dir) -> int {
		const int n = (int)slotRects.size();
		if (n <= 0) return 0;
		for (int step = 1; step <= n; step++) {
			int idx = (from + dir * step) % n;
			if (idx < 0) idx += n;
			if (!slotRects[idx].isEmpty())
				return idx;
		}
		return from;
	};
	if (slotRects[highlighted].isEmpty())
		highlighted = nextLiveSlot(highlighted, +1);

	drawGallery();

	// Wait-for-pick loop. Mirrors `_DoAccuseGallery` 1df2:0b26-1df2:0bc8:
	//   * `_CheckFrameRate` + `_UpdateAnimations` per tick (1df2:0b2a-0b33)
	//   * 5-entry input dispatch table @ 1df2:0bc9:
	//       0x09 (TAB)   → handler 0x0b94 (cycle highlight)
	//       0x0d (Enter) → handler 0x0b72 (pick = _SearchSuspects)
	//       0x4b (LEFT)  → handler 0x0b94
	//       0x4d (RIGHT) → handler 0x0b94
	//       0xFFFF (mb)  → handler 0x0b72
	//   * 0x0b94: `INC DI` + wraparound + `_PutMouseInRect(&Guys[DI])`,
	//     i.e. advance highlight and warp cursor (1df2:0b94-0bb1).
	//   * 0x0b72: `_SearchSuspects` (158f:0584) — mouse-rect hit-test;
	//     if non-0xFFFF, pick that suspect.
	// We don't warp the cursor (unfriendly under SDL); instead the
	// highlight is drawn as a 1px outline and Enter picks it.
	int picked = -1;
	uint32 lastTick = g_system->getMillis();
	bool dirty = false;
	while (picked < 0 && !shouldQuit()) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER)
				return;
			if (ev.type == Common::EVENT_KEYDOWN) {
				switch (ev.kbd.keycode) {
				case Common::KEYCODE_ESCAPE:
					return;
				case Common::KEYCODE_TAB:
				case Common::KEYCODE_RIGHT:
					highlighted = nextLiveSlot(highlighted, +1);
					dirty = true;
					break;
				case Common::KEYCODE_LEFT:
					// 1df2:0b94 increments DI for LEFT too — but a
					// keyboard-driven UX is friendlier with separate
					// directions, so we mirror Right=+1 / Left=-1.
					highlighted = nextLiveSlot(highlighted, -1);
					dirty = true;
					break;
				case Common::KEYCODE_RETURN:
				case Common::KEYCODE_KP_ENTER:
					if (highlighted >= 0 &&
						highlighted < (int)slotRects.size() &&
						!slotRects[highlighted].isEmpty()) {
						picked = highlighted;
					}
					break;
				default: {
					const int k = (int)ev.kbd.keycode;
					if (k >= Common::KEYCODE_1 && k <= Common::KEYCODE_9) {
						const int idx = k - Common::KEYCODE_1;
						if (idx < num &&
							!slotRects[idx].isEmpty())
							picked = idx;
					}
					break;
				}
				}
			}
			if (ev.type == Common::EVENT_LBUTTONDOWN) {
				for (uint i = 0; i < slotRects.size(); i++) {
					if (slotSuspect[i] < 0) continue;
					if (slotRects[i].contains(ev.mouse.x, ev.mouse.y)) {
						picked = (int)i;
						break;
					}
				}
			}
		}
		// 100 ms tick — the original calls `_UpdateAnimations` per
		// `_CheckFrameRate` (1df2:0b33). The accuse screen has no
		// animations registered, so the tick is just a redraw cadence.
		// We still re-render whenever the highlight moves (`dirty`).
		const uint32 now = g_system->getMillis();
		if (dirty || now - lastTick >= 100) {
			drawGallery();
			lastTick = now;
			dirty = false;
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
