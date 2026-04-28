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

#include "eem/audio.h"
#include "eem/detection.h"
#include "eem/eem.h"
#include "eem/music.h"
#include "eem/site.h"

#include "common/config-manager.h"
#include "common/savefile.h"
#include "common/serializer.h"

#include "graphics/managed_surface.h"

namespace EEM {

const uint kPalSize = 768;     ///< 256 colors * 3 bytes
const uint kNumSitePals = 40;  ///< SITEPALS holds 40 palettes (40 * 768 = 30720)

// Picture / palette IDs from the original code (1-based picture IDs).
const uint kPicEAKidsLogo      = 0x54;  ///< _ShowEAKids: GetPicture(0x54)
const uint kPicHighScoreLogo   = 0x20c; ///< _ShowHScoreLogo: GetPicture(0x20c)
const uint kPalEAKids          = 0x25;
const uint kPalHighScore       = 0x27;

// Save body version, used by the `Common::Serializer` inside
// `saveGameStream`/`loadGameStream`. The framework's extended-save
// header (description / thumbnail / playtime) is appended/parsed
// separately by `Engine::saveGameState` / `MetaEngine::readSavegameHeader`,
// so we don't need a magic word or our own metadata fields.
const byte kSaveBodyVer = 3;

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

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _rng("eem"),
	  _playerName("Detective"),
	  _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle), _partner(0) {
}

EEMEngine::~EEMEngine() {
	delete _audio;
	delete _music;
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

	// MIDI music player. Mirrors `_InitMIDI @ 20a2:013a`. Constructed
	// here (after `initGraphics` so the OSystem's timer/mixer is up).
	_music = new MusicPlayer();

	// Digital audio (VOC + spool). Mirrors `_InitDrivers @ 1ff1:0368`
	// which `_AIL_register_driver`s SBDIG.ADV / PASDIG.ADV alongside
	// the MIDI driver.
	_audio = new AudioPlayer(this);
	_audio->setVoiceEnabled(_voiceOn);
	syncSoundSettings();

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
	// Honour that by skipping the intros and dropping into the screen-
	// driver loop just past the briefing — the loaded mystery already
	// knows its current site, so we resume at MAP (the original's
	// post-briefing state, set by handler 0 at 1a35:0e1d).
	const int wantedSave = ConfMan.hasKey("save_slot")
		? ConfMan.getInt("save_slot") : -1;
	bool resumed = false;
	if (wantedSave >= 0) {
		const Common::Error err = loadGameState(wantedSave);
		if (err.getCode() == Common::kNoError && _mystery.isLoaded()) {
			debugC(1, kDebugGeneral, "Resuming from slot %d at mystery %u",
				   wantedSave, _mystery.number());
			CursorMan.showMouse(true);
			_nextScreen = kScreenMap;
			resumed = true;
		}
	}

	// Skip the entire intro chain (logos + anims + name entry +
	// partner pick) when resuming a saved profile — the partner is
	// already known, the player has already named themselves, and the
	// loaded mystery's site loop is what they want to see again.
	if (resumed)
		goto screen_loop;

	// Reproduces _DoOpeningAnims @ 2520:082a:
	//   EA Kids logo (PIC) -> HighScore Productions logo (PIC) ->
	//   Storm Software logo (BOLT.ANM) -> [music starts] -> 20
	//   character-intro animations (ANIM01.A..ANIM20.A) -> [music
	//   restarts] -> TITLE.ANM. Click / any key skips a single clip;
	//   ESC skips the rest of the chain (waitForInput / playAnm raise
	//   `_skipIntro` so each subsequent step bails out).
	//
	// Music timing (verified at 2520:0883 and 2520:0918):
	//   - The three logos and `_InitMysterySounds(0x3c)` all run BEFORE
	//     any `_MIDIPlayFile` call — those segments are voice-only.
	//   - Theme starts with `_LoopMIDI = 0x7fff` right before the
	//     ANIM01..ANIM20 loop (2520:0883).
	//   - After the loop the original calls `_CleanMysterySounds` and
	//     then `_MIDIPlayFile("theme.xmi")` again with `_LoopMIDI =
	//     0xffff` (2520:0918) to restart the theme for TITLE.ANM.
	//   - `_StopMIDI()` runs on keypress at the title screen
	//     (2520:094c).
	_skipIntro = false;
	showEAKidsLogo();
	if (!shouldQuit() && !_skipIntro)
		showHighScoreLogo();
	// Storm Software logo: voice + animation. The original at
	// `_ShowStormLogo @ 2520:0707` calls `_LoadSoundName("thunder.voc")`
	// (29be:177d) and passes the buffer to `OpenDifferenceAnimation_Sound`
	// so the thunder roar plays alongside the lightning bolt.
	if (!shouldQuit() && !_skipIntro) {
		if (_audio)
			_audio->playVoc(Common::Path("THUNDER.VOC"));
		playAnm(Common::Path("BOLT.ANM"));
		if (_audio)
			_audio->stopVoice();
	}
	// `_InitMysterySounds(0x3c)` at 2520:086a — load M60.SDX/SDB so
	// `_SpoolSound(uVar3 - 1)` between the ANIM01..ANIM20 anims has
	// data to draw from.
	if (!shouldQuit() && !_skipIntro && _audio)
		_audio->initMysterySounds(60);
	// Theme begins HERE — after the three silent logos, before the
	// character-intro reel.
	if (!shouldQuit() && !_skipIntro && _music)
		_music->playFile(Common::Path("THEME.XMI"), /*loop=*/true);
	for (int i = 1; i <= 20 && !shouldQuit() && !_skipIntro; i++) {
		Common::String name = Common::String::format("ANIM%02d.A", i);
		playAnm(Common::Path(name));
		// `_SpoolSound(uVar3 - 1)` at 2520:08c2 — the per-character VO
		// plays AFTER each anim except the last (`if (uVar3 != 0x14)`
		// at 2520:08a8). Original blocks until done; we run async and
		// wait so the next anim doesn't start before the line ends.
		if (!shouldQuit() && !_skipIntro && i != 20 && _audio) {
			_audio->spoolSound((uint)(i - 1));
			_audio->waitForSpoolDone();
		}
	}
	// `_CleanMysterySounds` at 2520:0903 — release M60 before the title.
	if (_audio)
		_audio->cleanMysterySounds();
	// Restart the theme for TITLE.ANM — matches the second
	// `_MIDIPlayFile("theme.xmi")` call at 2520:0918.
	if (!shouldQuit() && !_skipIntro && _music)
		_music->playFile(Common::Path("THEME.XMI"), /*loop=*/true);
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

	// Stop the title music — the original `_NewPlayer / _DoChoosePartner`
	// screens have no music until the briefing's `_PlayInSequence` /
	// per-mystery `_StartTravelMusic` kicks in.
	if (_music)
		_music->stop();
	// Profile pick (or fresh creation) — `screen8_handler @ 1c33:1012`.
	// `doProfilePicker` lists existing profiles via `listProfiles()`
	// and falls through to `doNewPlayer` if none exist or the user
	// picks "[New Player]".
	if (!shouldQuit())
		doProfilePicker();
	if (!shouldQuit())
		doChoosePartner();

	// Now drop into the screen-driver state machine — same pattern as
	// `_ScreenDriver @ 1a35:0dc1` + the per-screen handlers in the
	// table at 1a35:0e5e. The original sets `_NextScreen` either
	// directly (e.g. `_DisplayCorrect` writes 12 = ACTION) or via the
	// jumptable handlers (e.g. handler 0 calls `_DoInitClues` then
	// writes 1 = MAP). The handlers here mirror that exactly: each
	// case body runs the screen and updates `_nextScreen` for the next
	// iteration. Sentinel `kScreenInvalid` (0xFFFF) ends the loop —
	// same as the original's table-end marker.
	//
	// `_DoChoosePartner @ 1a35:099d` sets `_NextScreen = 0xc` (= the
	// original `_ActionScreen` — "Choose A Mystery / Practice Mystery /
	// See ScrapBook 1..3"). In our port that menu lives inside
	// `doCaseSelection`, which already mirrors `_ActionScreen`'s 5-entry
	// list (verified against `ActionNames @ 29be:0d6a`) and rolls the
	// individual-mystery picker (`_CaseSelection @ 1c33:0a87`) into the
	// same flow. So we route straight to `kScreenChooseMystery`; the
	// `kScreenAction` (= 0xc) state still exists for the post-win path
	// (`_DisplayCorrect @ 1df2:0895` writes 0xc) but its handler also
	// dispatches `doCaseSelection`. The previous `doActionScreen` was a
	// synthetic stub ("What now?" / "Solve a Mystery" / "Look at My
	// Books") whose strings are nowhere in the binary — confirmed by a
	// `search_strings` for "What" returning zero matches.
	//
	// Mid-mystery profile resume: if the profile picker loaded a
	// save whose `hasMystery` flag was set, `_mystery.isLoaded()` is
	// true here and the player just re-picked their partner. Drop
	// straight to MAP rather than the action menu so they don't have
	// to walk back through the case picker (which would
	// `_mystery.load()` fresh and discard their site / clue
	// progress). The original has no equivalent — it persists only
	// profile-level state via `_PlayerRecord`, not in-progress
	// mysteries — so this is a ScummVM-only ergonomics improvement.
	if (!shouldQuit() && !resumed)
		_nextScreen = _mystery.isLoaded() ? kScreenMap : kScreenChooseMystery;
screen_loop:
	while (!shouldQuit() && _nextScreen != kScreenInvalid) {
		const ScreenId current = (ScreenId)_nextScreen;
		debugC(1, kDebugGeneral, "screenDriver: id=%d", (int)current);

		switch (current) {
		case kScreenAction:
			// Post-mystery menu. The original's `_ActionScreen @
			// 1c33:195b` shows the 5-entry "Choose A Mystery /
			// Practice / ScrapBook" picker; `doCaseSelection` is
			// our port of that exact menu (plus the individual-case
			// sub-picker the original handles in `_CaseSelection @
			// 1c33:0a87`). After the player picks, fall through to
			// the same routing the `kScreenChooseMystery` case uses.
			// Reachable from `_DisplayCorrect`'s 0xc write after a
			// solve (see `ui.cpp` `_nextScreen = kScreenAction`).
			doCaseSelection();
			_nextScreen = _mystery.isLoaded() ? kScreenInitClues
											  : kScreenInvalid;
			break;

		case kScreenChooseMystery:
			// Handler 10 at 1a35:0e0e calls `_DoChooseMystery` which
			// presets `_NextScreen = 0` (INIT_CLUES) before
			// `_CaseSelection`. Same dispatch as `kScreenAction`.
			doCaseSelection();
			_nextScreen = _mystery.isLoaded() ? kScreenInitClues
											  : kScreenInvalid;
			break;

		case kScreenInitClues:
			// Handler 0 at 1a35:0e14 runs `_PreLoad` + `_DoInitClues`
			// then writes `_NextScreen = 1` (MAP).
			doInitClues();
			_nextScreen = _mystery.isLoaded() ? kScreenMap
											  : kScreenChooseMystery;
			break;

		case kScreenMap:
		case kScreenMapAlt:
			// Handler 1/2 at 1a35:0e25 calls `_DoMapScreen @
			// 20fe:120b` which manages its own `_NextScreen` writes —
			// 3 (a site was clicked), 6 (setup), or 0xffff (quit).
			// Our `doBigMap` keeps the original's "click site, then
			// enter the site loop" behaviour inline; once it returns
			// the natural next state is SITE.
			doBigMap();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenChooseMystery;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenSite:
			// Handler 3 at 1a35:0e2c calls `_DoSiteLoop @
			// 168d:03f4`. Our `doSiteLoop` is a complete loop —
			// notebook / gallery / accuse / map are dispatched
			// inline within `SiteScreen::run`. The accusation tail
			// in `doAccuse` (see ui.cpp) writes the next screen:
			// kScreenAction on win (matches `_DisplayCorrect @
			// 1df2:0895` writing 0xc) or kScreenSite on lose
			// (matches `_DisplayAlibi @ 1df2:043f` snapping back
			// via `_LastScreen`).
			doSiteLoop();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenChooseMystery;
			else if (_nextScreen == current)
				_nextScreen = kScreenInvalid;  // user quit
			break;

		case kScreenSetup:
			// Handler 6 at 1a35:0e48 calls `_DoSetup @ 1f78:044e`.
			// Reachable via the BigMap setup button which writes
			// `_NextScreen = 6` (verified at 20fe:0c33). The
			// original sets `_NextScreen = _LastScreen` on entry,
			// then the toggle UI returns when ESC / Back is hit;
			// `doSetup` sets `_nextScreen` itself.
			doSetup();
			break;

		default:
			warning("screenDriver: unhandled screen id %d", (int)current);
			_nextScreen = kScreenInvalid;
			break;
		}
		_lastScreen = current;
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

void EEMEngine::doSiteLoop() {
	// Mirrors the per-mystery site loop. SiteScreen::run() handles
	// hotspot clicks plus M (map), N (notebook), G (gallery), A (accuse),
	// Tab (next site), ESC (exit).
	SiteScreen screen(this, &_mystery);
	screen.run();
}

void EEMEngine::startTravelMusic() {
	// Mirrors `_StartTravelMusic @ 20a2:0595`:
	//
	//   for (num = _SiteNumber; num > 4; num -= 5) {}
	//   if (_MIDIAvailable && _MusicEnabled) {
	//       if (_IsMIDIPlaying()) _StopMIDI();
	//       _MIDIPlay(num);
	//   }
	//
	// Five travel tracks: MUS00000.XMI .. MUS00004.XMI, picked by
	// `_SiteNumber % 5`. The original always loops travel music (the
	// `_LoopMIDI` global isn't reset between site changes).
	if (!_music || !_mystery.isLoaded())
		return;
	const uint num = _mystery._siteNumber % 5;
	_music->playMus(num, /*loop=*/true);
}

void EEMEngine::syncSoundSettings() {
	Engine::syncSoundSettings();
	if (_music)
		_music->syncVolume();
}

bool EEMEngine::hasFeature(EngineFeature f) const {
	// We support saving any time but loading only at startup (via the
	// `--save-slot=N` resume path or a slot picked from the launcher).
	// Runtime loads would replace `_mystery._data` while pointers into
	// it are alive on the stack inside `displayClue` etc.
	return f == kSupportsSavingDuringRuntime ||
		   f == kSupportsReturnToLauncher;
}

bool EEMEngine::canLoadGameStateCurrently(Common::U32String *) {
	// Loading mid-mystery would replace `_mystery._data` while
	// pointers into it are alive on the stack inside `displayClue`
	// etc. Profile picking still works via `loadProfile` from the
	// menu screens before a mystery loads.
	return !_mystery.isLoaded();
}

bool EEMEngine::canSaveGameStateCurrently(Common::U32String *) {
	// Profile saves (no mystery loaded) are always OK; mid-mystery
	// snapshots only after the active case has fully initialised.
	return true;
}

Common::Error EEMEngine::saveGameStream(Common::WriteStream *stream,
										 bool isAutosave) {
	(void)isAutosave;

	// Body header: one byte version. `Common::Serializer::setVersion`
	// alone doesn't write/read the version — we emit it explicitly so
	// `loadGameStream` knows which fields are present. Older saves
	// (v1) lack `_chainStage`; newer ones include it.
	Common::Serializer s(nullptr, stream);
	s.setVersion(kSaveBodyVer);
	byte ver = kSaveBodyVer;
	s.syncAsByte(ver);

	// Profile-level state — mirrors the original `_PlayerRecord` body
	// at `2d5d:3f6a` (159 bytes, written by `_SavePlayerRecord @
	// 1c33:034f`). The `_PlayerRecord` layout is:
	//   +0x00..+0x0b : player name (12 chars, null-padded)
	//   +0x0c..+0x1f : random ID bytes used by `_GenerateFilename`
	//                  (29be:0dbf "C:\EEMCDSAV\%s.PLR") — irrelevant to
	//                  ScummVM saves which key on slot, not filename.
	//   +0x20..+0x28 : derived 8-char .PLR basename — likewise unused.
	//   +0x2d        : voice-enable flag (`DAT_2d5d_3f97`, default 1).
	//   +0x2f        : chain stage (`DAT_2d5d_3f99`, 1=A, 2=B, 3=C —
	//                  `_DisplayCorrect` advances it once every case
	//                  in the current set is solved).
	//   +0x31..+0xa6 : `mysteriesSolved[55]` u16 (0=unsolved, 1=solved,
	//                  2=solved on first try) — `_DisplayCorrect`
	//                  writes 1 always, 2 when `_FirstTry != 0`.
	//
	// We persist the gameplay-meaningful subset (name + solved table +
	// partner) and skip the filename-derivation bytes. The voice /
	// chain-stage fields aren't yet wired into the C++ port; we save
	// space for them so they slot in without a version bump.
	s.syncString(_playerName);
	s.syncBytes(_mysteriesSolved, sizeof(_mysteriesSolved));
	s.syncAsByte(_partner);
	// v2+: chain-stage tier (1=Junior, 2=Senior, 3=Master).
	s.syncAsByte(_chainStage);
	// v3+: voice on/off flag (DAT_2d5d_3f97).
	s.syncAsByte(_voiceOn);

	// ScummVM-only extension: persist the in-progress mystery so the
	// player can resume mid-case. The original engine has no such
	// notion — `_LoadGame @ 2404:0dc7` simply loads a fresh mystery,
	// it doesn't preserve site progress. The flag lets a profile save
	// stay valid even when no mystery is loaded (e.g. fresh profile).
	bool hasMystery = _mystery.isLoaded();
	s.syncAsByte(hasMystery);
	if (hasMystery) {
		uint16 mysteryNum = (uint16)_mystery.number();
		s.syncAsUint16LE(mysteryNum);
		_mystery.syncState(s);
	}

	debugC(1, kDebugGeneral,
		   "Saved profile name=%s partner=%u stage=%u mystery=%d autosave=%d",
		   _playerName.c_str(), _partner, _chainStage,
		   hasMystery ? (int)_mystery.number() : -1,
		   isAutosave ? 1 : 0);
	return Common::kNoError;
}

Common::Error EEMEngine::loadGameStream(Common::SeekableReadStream *stream) {
	Common::Serializer s(stream, nullptr);
	byte ver = 0;
	s.syncAsByte(ver);
	if (ver > kSaveBodyVer) {
		warning("loadGameStream: save body version %u newer than %u — refusing",
				ver, kSaveBodyVer);
		return Common::kReadingFailed;
	}
	s.setVersion(ver);

	s.syncString(_playerName);
	if (_playerName.empty())
		_playerName = "Detective";

	s.syncBytes(_mysteriesSolved, sizeof(_mysteriesSolved));
	s.syncAsByte(_partner);
	s.syncAsByte(_chainStage);
	s.syncAsByte(_voiceOn);
	if (_audio)
		_audio->setVoiceEnabled(_voiceOn);

	bool hasMystery = false;
	s.syncAsByte(hasMystery);
	if (hasMystery) {
		uint16 mysteryNum = 0;
		s.syncAsUint16LE(mysteryNum);
		if (!_mystery.load(mysteryNum, &_rng)) {
			_mystery.clear();
			return Common::kReadingFailed;
		}
		// `_ReadMystery @ 2404:008f` calls `_InitMysterySounds` at the
		// tail (2404:0298) so the SDB index is in place for clue and
		// partner-speech spool sounds.
		if (_audio)
			_audio->initMysterySounds(mysteryNum);
		_mystery.syncState(s);
	} else {
		_mystery.clear();
	}

	debugC(1, kDebugGeneral,
		   "Loaded profile name=%s partner=%u mystery=%d",
		   _playerName.c_str(), _partner,
		   _mystery.isLoaded() ? (int)_mystery.number() : -1);
	return Common::kNoError;
}

SaveStateList EEMEngine::listProfiles() const {
	// Mirrors `_findfirst("*.PLR")` in `screen8_handler @ 1c33:1012`.
	// We disable autosave (`getAutosaveSlot()` returns -1) but a
	// pre-existing slot-0 file from a previous run, or one written by
	// another engine using the same target, would still show up here
	// and pollute the picker. Filter it out so the picker treats slot
	// 0 as if it didn't exist — matching the original which never
	// has an autosave concept.
	SaveStateList saves = getMetaEngine()->listSaves(_targetName.c_str());
	for (uint i = 0; i < saves.size(); ) {
		if (saves[i].getSaveSlot() == 0)
			saves.remove_at(i);
		else
			i++;
	}
	return saves;
}

Common::Error EEMEngine::saveProfile(const Common::String &name) {
	if (name.empty())
		return Common::kCreatingFileFailed;

	const SaveStateList saves = listProfiles();

	// Slot lookup by description: if a save with this profile name
	// already exists, overwrite it. Same as Wetlands' `saveProfile`.
	int slot = -1;
	for (auto &s : saves) {
		if (s.getDescription() == name) {
			slot = s.getSaveSlot();
			break;
		}
	}

	// New profile — pick the lowest unused slot. The MetaEngine caps
	// us at 99 by default (`getMaximumSaveSlot`); 25 was the DOS
	// original's limit (`screen8_handler` walks `*.PLR` up to 25
	// entries in `local_8c[0x19][2]`).
	if (slot < 0) {
		const int maxSlot = getMetaEngine()->getMaximumSaveSlot();
		Common::Array<bool> used(maxSlot + 1);
		for (auto &s : saves) {
			const int sl = s.getSaveSlot();
			if (sl >= 0 && sl <= maxSlot)
				used[sl] = true;
		}
		for (int i = 0; i <= maxSlot; i++) {
			if (!used[i]) {
				slot = i;
				break;
			}
		}
		if (slot < 0)
			return Common::kCreatingFileFailed;
	}

	_playerName = name;
	debugC(1, kDebugGeneral, "saveProfile(%s) -> slot %d",
		   name.c_str(), slot);
	return saveGameState(slot, name, /*isAutosave=*/false);
}

bool EEMEngine::loadProfile(const Common::String &name) {
	if (name.empty())
		return false;

	const SaveStateList saves = listProfiles();
	for (auto &s : saves) {
		if (s.getDescription() == name) {
			const Common::Error err = loadGameState(s.getSaveSlot());
			if (err.getCode() == Common::kNoError) {
				debugC(1, kDebugGeneral, "loadProfile(%s) <- slot %d",
					   name.c_str(), s.getSaveSlot());
				return true;
			}
			break;
		}
	}
	debugC(1, kDebugGeneral, "loadProfile(%s) — no matching slot",
		   name.c_str());
	return false;
}

void EEMEngine::screenDriver() {
	// Placeholder for the eventual dispatch table (one entry per ScreenId).
	// run() currently calls handlers directly until the title path lands.
}

} // End of namespace EEM
