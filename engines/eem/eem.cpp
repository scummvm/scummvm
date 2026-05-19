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

const uint kPalSize = 768;
const uint kNumSitePals = 40;  // SITEPALS: 40 * 768 = 30720

// 1-based picture/palette IDs.
const uint kPicEAKidsLogo      = 0x54;  // _ShowEAKids
const uint kPicHighScoreLogo   = 0x20c; // _ShowHScoreLogo
const uint kPicStormLogo       = 0x20b; // Floppy storm-logo still
const uint kPalEAKids          = 0x25;
const uint kPalHighScore       = 0x27;
const uint kPalStormLogo       = 0x26;  // Floppy FUN_23d2_0605
const uint kPicMousePointer    = 0x50;  // 0x51 is the wait cursor

const byte kSaveBodyVer = 1;

// Test switch: populate ScrapBook 1 at startup without exposing a game
// option or changing save format. Set false before release.
const bool kDebugPopulateScrapbook1AtStartup = false;

// Fallback 11x16 cursor used if PIC pointer load fails.
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
	0x00, 0x00, 0x00,
	0x00, 0x00, 0x00,
	0xFF, 0xFF, 0xFF
};
const byte kCursorInteractivePalette[] = {
	0x00, 0x00, 0x00,
	0xFF, 0x00, 0x00,
	0xFF, 0xFF, 0xFF
};

void fadeCurrentPaletteToBlack(uint delayMs = 8) {
	byte start[kPalSize];
	byte stepPal[kPalSize];
	g_system->getPaletteManager()->grabPalette(start, 0, 256);

	for (int step = 15; step >= 0; step--) {
		for (uint i = 0; i < kPalSize; i++)
			stepPal[i] = (byte)((uint)start[i] * step / 16);
		g_system->getPaletteManager()->setPalette(stepPal, 0, 256);
		g_system->updateScreen();
		if (delayMs)
			g_system->delayMillis(delayMs);
	}
}

void fadePaletteFromBlack(const byte *target, uint delayMs = 8) {
	byte stepPal[kPalSize];

	for (uint step = 1; step <= 16; step++) {
		for (uint i = 0; i < kPalSize; i++)
			stepPal[i] = (byte)((uint)target[i] * step / 16);
		g_system->getPaletteManager()->setPalette(stepPal, 0, 256);
		g_system->updateScreen();
		if (delayMs)
			g_system->delayMillis(delayMs);
	}
}

void setInteractiveCursorPalette(const Picture &cursor, byte transparent) {
	byte palette[kPalSize];
	bool used[256];
	memset(used, 0, sizeof(used));

	g_system->getPaletteManager()->grabPalette(palette, 0, 256);

	for (int y = 0; y < cursor.surface.h; y++) {
		const byte *src = (const byte *)cursor.surface.getBasePtr(0, y);
		for (int x = 0; x < cursor.surface.w; x++) {
			if (src[x] != transparent)
				used[src[x]] = true;
		}
	}

	int minLuma = 255;
	int maxLuma = 0;
	for (uint i = 0; i < 256; i++) {
		if (!used[i])
			continue;
		const byte *rgb = palette + i * 3;
		const int luma = (rgb[0] * 30 + rgb[1] * 59 + rgb[2] * 11) / 100;
		minLuma = MIN(minLuma, luma);
		maxLuma = MAX(maxLuma, luma);
	}
	const int outlineThreshold = (minLuma + maxLuma) / 2;

	for (uint i = 0; i < 256; i++) {
		if (!used[i])
			continue;
		byte *rgb = palette + i * 3;
		const int luma = (rgb[0] * 30 + rgb[1] * 59 + rgb[2] * 11) / 100;
		if (luma <= outlineThreshold) {
			rgb[0] = 0xFF;
			rgb[1] = 0x00;
			rgb[2] = 0x00;
		} else {
			rgb[0] = 0xFF;
			rgb[1] = 0xFF;
			rgb[2] = 0xFF;
		}
	}

	CursorMan.replaceCursorPalette(palette, 0, 256);
}

void installMouseCursor(DBDArchive &pics, bool interactive) {
	Picture cursor;
	if (pics.getPicture(kPicMousePointer, cursor) && !cursor.surface.empty()) {
		const byte transparent = (byte)(cursor.flags >> 8);
		CursorMan.replaceCursor(cursor.surface.rawSurface(), 0, 0,
								transparent);
		if (interactive)
			setInteractiveCursorPalette(cursor, transparent);
		else
			CursorMan.replaceCursorPalette(nullptr, 0, 0);
		return;
	}

	warning("EEM: mouse cursor PIC 0x%x missing; using fallback cursor",
			kPicMousePointer);
	CursorMan.replaceCursor(kCursorBitmap, 11, 16, 0, 0, 0);
	CursorMan.replaceCursorPalette(interactive ? kCursorInteractivePalette
											   : kCursorPalette,
								   0, 3);
}

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _rng("eem"),
	  _playerName("Detective"),
	  _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle), _partner(0) {
	ConfMan.registerDefault("hide_highlight_boxes", false);
	ConfMan.registerDefault("fit_dialog_balloons", false);
	ConfMan.registerDefault("skip_repeated_cases", false);

	_variant = (gameDesc && gameDesc->extra &&
				Common::String(gameDesc->extra).contains("Floppy"))
				 ? kVariantFloppy : kVariantCD;
	_language = gameDesc ? gameDesc->language : Common::EN_ANY;
}

EEMEngine::~EEMEngine() {
	delete _audio;
	delete _music;
}

void EEMEngine::applyStartupTestOverrides() {
	if (!kDebugPopulateScrapbook1AtStartup)
		return;

	for (uint i = 1; i <= 0x18 && i < sizeof(_mysteriesSolved); i++)
		_mysteriesSolved[i] = 1;

	debugC(1, kDebugGeneral,
		   "startup test override: populated ScrapBook 1 mystery flags");
}

bool EEMEngine::areMysteriesSolved(uint lo, uint hi) const {
	if (hi < lo)
		return false;
	for (uint i = lo; i <= hi; i++) {
		if (i >= sizeof(_mysteriesSolved) || _mysteriesSolved[i] == 0)
			return false;
	}
	return true;
}

void EEMEngine::advanceChainStageAfterSolve(uint mysteryNum) {
	if (mysteryNum == 0 || _chainStage >= 4)
		return;

	uint lo = 0;
	uint hi = 0;
	switch (_chainStage) {
	case 1:
		lo = 1;
		hi = 0x18;
		break;
	case 2:
		lo = 0x19;
		hi = 0x30;
		break;
	case 3:
		lo = 0x31;
		hi = 0x36;
		break;
	default:
		return;
	}

	if (!areMysteriesSolved(lo, hi))
		return;

	const uint oldStage = _chainStage;
	// Book 2 repeats the Book 1 cases; this option keeps the original solve
	// state but jumps the profile's active chain straight to Book 3.
	if (_chainStage == 1 && ConfMan.getBool("skip_repeated_cases"))
		_chainStage = 3;
	else
		_chainStage++;

	debugC(1, kDebugMystery,
		   "chainStage advanced from %u to %u after solving mystery %u",
		   oldStage, _chainStage, mysteryNum);
}

void EEMEngine::applySkipRepeatedCasesOption() {
	if (!ConfMan.getBool("skip_repeated_cases"))
		return;
	if (_mystery.isLoaded())
		return;
	if (_chainStage <= 2 && areMysteriesSolved(1, 0x18)) {
		const uint oldStage = _chainStage;
		_chainStage = 3;
		debugC(1, kDebugMystery,
			   "skip_repeated_cases advanced chainStage from %u to %u",
			   oldStage, _chainStage);
	}
}

Common::Error EEMEngine::run() {
	// _SetMode13X @ 1000:0358 — VGA mode 13h.
	initGraphics(kScreenWidth, kScreenHeight);

	if (!openArchives())
		return Common::Error(Common::kReadingFailed, "EEM archive open failed");

	if (!loadSitePalettes())
		return Common::Error(Common::kReadingFailed, "SITEPALS load failed");

	// _LoadFont @ 1b66:023c.
	if (!_font.load(Common::Path("FONT.FNT")))
		warning("FONT.FNT failed to load; text will not render");

	// _InitMIDI @ 20a2:013a.
	_music = new MusicPlayer(isFloppy());

	// _InitDrivers @ 1ff1:0368 (SBDIG.ADV / PASDIG.ADV).
	_audio = new AudioPlayer(this);
	_audio->setVoiceEnabled(_voiceOn);
	syncSoundSettings();

	// CD `_main @ 1a35:0f59` and floppy `_main_Floppy @ 19bb:1012` both
	// load `_GetPicture(0x50)` as the active mouse pointer before
	// `_InitMouse`. PIC 0x51 is present in both archives but has no
	// executable xrefs and appears to be the wait cursor.
	// CD's `_SwitchMouse` supports swapping to a hotspot cursor ID stored
	// at search record +0x0c, but the shipped CD mystery data only uses
	// cursor 0; floppy search records have no cursor-id field.
	installMouseCursor(_picsArchive, false);
	CursorMan.showMouse(false);

	// _AllBlack @ 172b:0d4b.
	byte black[3 * 256] = { 0 };
	g_system->getPaletteManager()->setPalette(black, 0, 256);

	debugC(1, kDebugGeneral, "EEM engine starting");

	// Resume from save: mystery in progress → MAP (handler 0 @ 1a35:0e1d);
	// otherwise → ACTION.
	const int wantedSave = ConfMan.hasKey("save_slot")
		? ConfMan.getInt("save_slot") : -1;
	bool resumed = false;
	bool skippedIntro = false;
	if (wantedSave >= 0) {
		const Common::Error err = loadGameState(wantedSave);
		if (err.getCode() == Common::kNoError) {
			applyStartupTestOverrides();
			applySkipRepeatedCasesOption();
			CursorMan.showMouse(true);
			if (_mystery.isLoaded()) {
				debugC(1, kDebugGeneral,
					   "Resuming from slot %d at mystery %u",
					   wantedSave, _mystery.number());
				_nextScreen = kScreenMap;
			} else {
				debugC(1, kDebugGeneral,
					   "Resuming profile from slot %d (no mystery — "
					   "→ action screen)", wantedSave);
				_nextScreen = kScreenAction;
			}
			resumed = true;
		}
	}

	if (resumed)
		goto screenLoop;

	// _DoOpeningAnims @ 2520:082a:
	//   EA Kids logo (PIC) -> HighScore logo (PIC) -> Storm logo
	//   (BOLT.ANM) -> [music starts] -> 20 character-intro anims
	//   (ANIM01.A..ANIM20.A) -> [music restarts] -> TITLE.ANM. Click /
	//   any key skips one clip; ESC raises _skipIntro so each step bails
	//   out.
	// Music timing (2520:0883 + 2520:0918):
	//   - The three logos and `_InitMysterySounds(0x3c)` run BEFORE any
	//     `_MIDIPlayFile` — those segments are voice-only.
	//   - Theme starts with `_LoopMIDI = 0x7fff` right before the
	//     ANIM01..ANIM20 loop (2520:0883).
	//   - After the loop the original calls `_CleanMysterySounds` then
	//     `_MIDIPlayFile("theme.xmi")` again with `_LoopMIDI = 0xffff`
	//     (2520:0918) to restart for TITLE.ANM.
	//   - `_StopMIDI()` runs on keypress at the title screen (2520:094c).
	_skipIntro = false;
	if (isFloppy()) {
		// Floppy opening — `FUN_23d2_039c @ 23d2:039c`:
		//   FUN_23d2_0170()  — clear palette
		//   FUN_23d2_004b()  — set up timer
		//   FUN_23d2_050c()  — PIC 0x54 (EA Kids, palette 0x25)
		//   FUN_23d2_06c6()  — PIC 0x20c (High Score, palette 0x27)
		//   FUN_23d2_0605()  — PIC 0x20b (Storm, palette 0x26) AND
		//                      voice slot 25 = "thunder.voc" (via Jake
		//                      voice table 2608:0f0e slot 25 →
		//                      2608:11ac).
		//   _MIDIPlayFile("theme.xmi", loop=1)
		//   _PlayANM(0) — CHAT.ANM (filename table 2608:14fe[0] →
		//                  "chat.anm" at 2608:150a)
		//   _PlayANM(1) — MOVIE.ANM (table[1] → 2608:1513)
		// TITLE.ANM is shown later by screen-0xb handler @ 19bb:0ebc.
		if (!shouldQuit() && !_skipIntro)
			showEAKidsLogo();
		if (!shouldQuit() && !_skipIntro)
			showHighScoreLogo();
		if (!shouldQuit() && !_skipIntro)
			showFloppyStormLogo();
		if (!shouldQuit() && !_skipIntro && _music)
			_music->playFile(Common::Path("THEME.XMI"), /* loop= */ true);
		if (!shouldQuit() && !_skipIntro)
			playAnm(Common::Path("CHAT.ANM"), 120,
					/* holdLastFrame= */ false);
		if (!shouldQuit() && !_skipIntro)
			playAnm(Common::Path("MOVIE.ANM"), 120,
					/* holdLastFrame= */ false);
	} else {
		showEAKidsLogo();
		if (!shouldQuit() && !_skipIntro)
			showHighScoreLogo();
		// Storm Software logo: voice + animation. `_ShowStormLogo @
		// 2520:0707` calls `_LoadSoundName("thunder.voc")` (29be:177d)
		// and passes the buffer to `OpenDifferenceAnimation_Sound` so
		// the thunder roar plays alongside the lightning-bolt BOLT.ANM.
		if (!shouldQuit() && !_skipIntro) {
			if (_audio)
				_audio->playVoc(Common::Path("THUNDER.VOC"));
			playAnm(Common::Path("BOLT.ANM"), 120,
					/* holdLastFrame= */ false, /* fadeIn= */ true);
			waitForInput(1800);
			fadeCurrentPaletteToBlack();
			if (_audio)
				_audio->stopVoice();
		}
		// _InitMysterySounds(0x3c) @ 2520:086a — load M60.SDX/SDB.
		if (!shouldQuit() && !_skipIntro && _audio)
			_audio->initMysterySounds(60);
		if (!shouldQuit() && !_skipIntro && _music)
			_music->playFile(Common::Path("THEME.XMI"), /* loop= */ true);
		for (int i = 1; i <= 20 && !shouldQuit() && !_skipIntro; i++) {
			const bool fadeIn = (i == 1 || i == 5);
			if (i == 5)
				fadeCurrentPaletteToBlack();
			Common::String name = Common::String::format("ANIM%02d.A", i);
			playAnm(Common::Path(name), 120,
					/* holdLastFrame= */ false, fadeIn);
			// _SpoolSound(uVar3 - 1) @ 2520:08c2 — per-anim VO, skipped
			// when uVar3 == 0x14 @ 2520:08a8.
			if (!shouldQuit() && !_skipIntro && i != 20 && _audio) {
				_audio->spoolSound((uint)(i - 1));
				_audio->waitForSpoolDone();
			}
		}
		// _CleanMysterySounds @ 2520:0903.
		if (_audio)
			_audio->cleanMysterySounds();
		// _MIDIPlayFile("theme.xmi") @ 2520:0918.
		if (!shouldQuit() && !_skipIntro && _music)
			_music->playFile(Common::Path("THEME.XMI"), /* loop= */ true);
		if (!shouldQuit() && !_skipIntro)
			playAnm(Common::Path("TITLE.ANM"), 120,
					/* holdLastFrame= */ true, /* fadeIn= */ true);
	}
	skippedIntro = _skipIntro;
	_skipIntro = false;

	if (isFloppy() && !shouldQuit() && !skippedIntro) {
		_nextScreen = kScreenTitle;
		goto screenLoop;
	}

	// Title(B) -> screen 8 (profile) -> 9 (partner) -> C (action) ->
	// A (case selection) -> site loop.
	CursorMan.showMouse(true);

	if (_music)
		_music->stop();
	// screen8_handler @ 1c33:1012.
	if (!shouldQuit())
		doProfilePicker();
	if (!shouldQuit())
		applyStartupTestOverrides();
	if (!shouldQuit())
		applySkipRepeatedCasesOption();
	if (!shouldQuit())
		doChoosePartner();

	// Drop into the screen-driver state machine — same pattern as
	// `_ScreenDriver @ 1a35:0dc1` + the per-screen handler table at
	// 1a35:0e5e. Sentinel `kScreenInvalid` (0xFFFF) ends the loop.
	// `_DoChoosePartner @ 1a35:099d` writes `_NextScreen = 0xc` (the
	// original `_ActionScreen`, which is separate from handler 10's
	// `_DoChooseMystery` / `_CaseSelection`).
	// Mid-mystery resume: if the loaded save had `hasMystery` set,
	// drop straight to MAP rather than the action menu so the player
	// doesn't walk back through the case picker (which would
	// `_mystery.load()` fresh and discard site / clue progress).
	if (!shouldQuit() && !resumed)
		_nextScreen = _mystery.isLoaded() ? kScreenMap : kScreenAction;
screenLoop:
	while (!shouldQuit() && _nextScreen != kScreenInvalid) {
		const ScreenId current = (ScreenId)_nextScreen;
		debugC(1, kDebugGeneral, "screenDriver: id=%d", (int)current);

		switch (current) {
		case kScreenTitle:
			// Floppy handler 0xb _HandleScreen11_Title_Floppy ->
			// _DoTitle_Floppy -> _PlayTitleANM_Floppy(1)=TITLE.ANM.
			// Writes _NextScreen=8 (profile picker).
			_nextScreen = kScreenProfile;
			if (isFloppy()) {
				CursorMan.showMouse(false);
				playAnm(Common::Path("TITLE.ANM"), 120,
						/* holdLastFrame= */ true, /* fadeIn= */ true);
				_skipIntro = false;
				CursorMan.showMouse(true);
			}
			break;

		case kScreenAction:
			// Top-level post-profile / post-mystery menu. `_ActionScreen
			// @ 1c33:195b` shows the 5-entry "Choose A Mystery /
			// Practice / ScrapBook 1..3" picker; writes _NextScreen=0xa
			// only when the player picks "Choose A Mystery".
			_nextScreen = kScreenInvalid;
			doActionScreen();
			if (_nextScreen == kScreenInvalid && _mystery.isLoaded())
				_nextScreen = kScreenInitClues;
			break;

		case kScreenChooseMystery:
			// Handler 10 @ 1a35:0e0e -> _DoChooseMystery (presets
			// _NextScreen=0 INIT_CLUES) -> _CaseSelection.
			_nextScreen = kScreenInvalid;
			doCaseSelection();
			if (_nextScreen == kScreenInvalid && _mystery.isLoaded())
				_nextScreen = kScreenInitClues;
			break;

		case kScreenInitClues:
			// Handler 0 @ 1a35:0e14 -> _PreLoad + _DoInitClues, writes
			// _NextScreen=1 (MAP).
			doInitClues();
			_nextScreen = _mystery.isLoaded() ? kScreenMap
											  : kScreenAction;
			break;

		case kScreenMap:
		case kScreenMapAlt:
			// Handler 1/2 @ 1a35:0e25 -> `_DoMapScreen @ 20fe:120b`
			// (floppy 19bb:0ef3 -> 1fed map code), which manages its
			// own _NextScreen writes: 3 (site clicked), 6 (setup), or
			// 0xffff (quit). After `doBigMap` returns the natural
			// next state is SITE.
			doBigMap();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenSite:
			// Handler 3 @ 1a35:0e2c -> `_DoSiteLoop @ 168d:03f4`
			// (floppy dispatches through 1652). Site writes _NextScreen
			// for PDA / map rather than entering those as nested modals.
			doSiteLoop();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenInvalid;
			break;

		case kScreenNotebook:
			// Handler 4 — PDA notebook screen. Button handler writes
			// 2 (map), 3 (site), 5 (gallery), or 7 (accuse).
			doNotebook();
			if (!_mystery.isLoaded() && _nextScreen != kScreenAction)
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenGallery:
			// Handler 5 — suspect gallery. ESC and the site button
			// write 3, the map button writes 2, the PDA button writes 4.
			doGallery();
			if (!_mystery.isLoaded() && _nextScreen != kScreenAction)
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenSetup:
			// Handler 6 @ 1a35:0e48 -> _DoSetup @ 1f78:044e. Entered
			// from BigMap setup button (_NextScreen=6 @ 20fe:0c33).
			doSetup();
			break;

		case kScreenProfile:
			// Handler 8: CD screen8_handler @ 1c33:1012 ->
			// _NewPlayer; floppy _HandleScreen8_NewPlayer_Floppy @
			// 19bb:0ec2 writes screen 9.
			_nextScreen = kScreenInvalid;
			_mystery.clear();
			doProfilePicker();
			if (!shouldQuit())
				applyStartupTestOverrides();
			if (!shouldQuit())
				applySkipRepeatedCasesOption();
			if (!shouldQuit())
				doChoosePartner();
			if (!shouldQuit())
				_nextScreen = _mystery.isLoaded() ? kScreenMap
												  : kScreenAction;
			break;

		case kScreenAccuse:
			// Handler 7 — accusation flow. Failed accusation returns to
			// `_LastScreen`; a correct solution writes 0xc (ACTION).
			doAccuse();
			if (!_mystery.isLoaded() && _nextScreen != kScreenAction)
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
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

void EEMEngine::setInteractiveMouseCursor(bool active) {
	if (_interactiveMouseCursor == active)
		return;

	_interactiveMouseCursor = active;
	installMouseCursor(_picsArchive, active);
}

void EEMEngine::setHotspotMouseCursor(bool active) {
	setInteractiveMouseCursor(active);
}

bool EEMEngine::openArchives() {
	// _InitGraphicsSystem @ 172b:0145.
	if (!_picsArchive.open(Common::Path("PICS.DBD"), Common::Path("PICS.DBX"))) {
		warning("PICS archive missing");
		return false;
	}
	if (!_aniArchive.open(Common::Path("ANI.DBD"), Common::Path("ANI.DBX"))) {
		warning("ANI archive missing");
		return false;
	}
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

bool EEMEngine::getSitePalette(uint num, byte *out) const {
	if (num >= kNumSitePals || _sitePals.size() < (num + 1) * kPalSize)
		return false;
	// SITEPALS stores 6-bit VGA-DAC values (0..63); ScummVM expects
	// 8-bit (0..255), so left-shift by 2 like the original VGA hardware.
	const byte *src = _sitePals.data() + num * kPalSize;
	for (uint i = 0; i < kPalSize; i++)
		out[i] = (byte)(src[i] << 2);
	return true;
}

void EEMEngine::setSitePalette(uint num) {
	byte expanded[kPalSize];
	if (!getSitePalette(num, expanded)) {
		warning("setSitePalette: index %u out of range", num);
		return;
	}
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

void EEMEngine::interruptAudio(bool stopMusicToo) {
	// Mirrors `_CleanMysterySounds @ 202f:05a5` + `_StopMIDI @
	// 20a2:0512` — both fire when the player aborts the opening-anim
	// chain or dismisses the title (`_DoOpeningAnims` writes
	// `_LoopMIDI = 0; _StopMIDI();` after the title-input loop).
	// Conversation / clue-dialog skip paths pass `stopMusicToo = false`
	// so the site / briefing MIDI keeps going across an ESC — only the
	// per-line voice + spool need to stop.
	if (_audio) {
		_audio->stopVoice();
		_audio->stopSpool();
	}
	if (stopMusicToo && _music)
		_music->stop();
}

void EEMEngine::playAnm(const Common::Path &path, uint frameDelayMs,
						bool holdLastFrame, bool fadeIn) {
	ANMDecoder anm;
	if (!anm.open(path)) {
		warning("playAnm: %s missing", path.toString().c_str());
		return;
	}

	byte palette[768];
	anm.getPalette8(palette);

	const uint16 w = anm.width();
	const uint16 h = anm.height();
	{
		Graphics::Surface *screen = g_system->lockScreen();
		if (screen) {
			if (screen->w >= w && screen->h >= h)
				anm.seedFrameBuffer((const byte *)screen->getBasePtr(0, 0), screen->pitch);
			g_system->unlockScreen();
		}
	}

	bool paletteApplied = false;
	while (!shouldQuit()) {
		const byte *frame = anm.nextFrame();
		if (!frame)
			break;

		g_system->copyRectToScreen(frame, w, 0, 0, w, h);
		if (!paletteApplied) {
			if (fadeIn)
				fadePaletteFromBlack(palette);
			else
				g_system->getPaletteManager()->setPalette(palette, 0, 256);
			paletteApplied = true;
		}
		g_system->updateScreen();

		// Original uses _CheckFrameRate / _kbhit; fixed delay here.
		// ESC sets _skipIntro and interrupts audio.
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
					if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
						_skipIntro = true;
						interruptAudio();
					}
					aborted = true;
					break;
				}
			}
			// Refresh cursor overlay every tick — otherwise
			// the cursor only redraws when the next frame is blitted
			// (~8 Hz at 120 ms), perceived as choppy during long
			// animations like SCRAPBK.ANI.
			g_system->updateScreen();
			g_system->delayMillis(5);
		}
		if (aborted)
			break;
	}

	if (holdLastFrame && !shouldQuit() && !_skipIntro) {
		// _DoOpeningAnims tail: while (!keyDataAvailable).
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
					if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
						_skipIntro = true;
						interruptAudio();
					}
					clicked = true;
					break;
				}
			}
			if (clicked)
				break;
			g_system->updateScreen();
			g_system->delayMillis(20);
		}
		// _DoOpeningAnims @ 2520:0945: _LoopMIDI=0; _StopMIDI().
		if (_music)
			_music->stop();
	}
}

void EEMEngine::blitAt(const Picture &pic, int x, int y) {
	// Clip against the 320x200 frame buffer.
	const int w = MIN<int>(pic.surface.w, kScreenWidth - x);
	const int h = MIN<int>(pic.surface.h, kScreenHeight - y);
	if (w <= 0 || h <= 0)
		return;
	g_system->copyRectToScreen(pic.surface.getPixels(), pic.surface.pitch,
							   x, y, w, h);
}

void EEMEngine::waitForInput(uint32 maxMs) {
	// ESC: _skipIntro + interruptAudio (matches _CleanMysterySounds +
	// _StopMIDI around _DoOpeningAnims title wait).
	// Only Return/KP-Enter/Space/Escape advance.
	setInteractiveMouseCursor(false);
	const uint32 startMs = g_system->getMillis();
	while (!shouldQuit() && (g_system->getMillis() - startMs < maxMs)) {
		Common::Event event;
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_QUIT ||
				event.type == Common::EVENT_RETURN_TO_LAUNCHER ||
				event.type == Common::EVENT_LBUTTONDOWN) {
				return;
			}
			if (event.type == Common::EVENT_MOUSEMOVE) {
				setInteractiveMouseCursor(false);
				continue;
			}
			if (event.type == Common::EVENT_KEYDOWN) {
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					_skipIntro = true;
					interruptAudio();
					return;
				}
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_KP_ENTER ||
					event.kbd.keycode == Common::KEYCODE_SPACE) {
					return;
				}
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

// _OpenColorCycle @ 2520:04f7. Rotate `fpal[start..end]` by one slot:
//   saved = fpal[end]
//   for u = end..start+1: fpal[u] = fpal[u-1]
//   fpal[start] = saved
// If `show`, upload `end - start` entries (fpal[start..end-1]) — note that
// fpal[end] is rotated in memory but intentionally not uploaded each tick.
void openColorCycle(byte *fpal, uint8 start, uint8 end, bool show) {
	if (end <= start)
		return;
	const byte savedR = fpal[end * 3 + 0];
	const byte savedG = fpal[end * 3 + 1];
	const byte savedB = fpal[end * 3 + 2];
	for (uint u = end; u > start; u--) {
		fpal[u * 3 + 0] = fpal[(u - 1) * 3 + 0];
		fpal[u * 3 + 1] = fpal[(u - 1) * 3 + 1];
		fpal[u * 3 + 2] = fpal[(u - 1) * 3 + 2];
	}
	fpal[start * 3 + 0] = savedR;
	fpal[start * 3 + 1] = savedG;
	fpal[start * 3 + 2] = savedB;
	if (show) {
		g_system->getPaletteManager()->setPalette(fpal + start * 3, start,
												   end - start);
	}
}

void EEMEngine::showEAKidsLogo() {
	// _ShowEAKids @ 2520:05f0:
	//   _GetPicture(0x54) + memcpy to 0xa000 (VGA).
	//   _GetPalette(0x25) loads pal 0x25 into _fpal (NOT uploaded to DAC).
	//   FRAME_RATE = 0x19 (25 fps); _InitFrameReg.
	//   for j in 0..1: show = j;
	//     for u in 0..0x37 (= 55):
	//       if (show) wait for next 25-fps tick (abort on key/click).
	//       _OpenColorCycle(0x01, 0x6e, show)   // bg / outer ring shimmer
	//       _OpenColorCycle(0x81, 0xee, show)   // inner gradient shimmer
	//       if (--delay == 0) {
	//         delay = 8;
	//         _OpenColorCycle(0x70, 0x80, show) // mid band
	//       }
	//   if (!abort) {
	//     for i in 0..5: _OpenColorCycle(0x70, 0x80, 1);
	//     for i in 0..0x23: wait one frame;
	//   }
	//   _OpenFadeOut().
	//
	// Pass 1 (j=0, show=0) pre-rolls _fpal 55 frames in memory only — no
	// DAC upload, no frame sync. Pass 2 (j=1, show=1) uploads each shift
	// at 25 fps. Without the pre-roll, the logo first appears at the
	// unrotated palette-0x25 phase instead of the intended "55-shifts-in"
	// phase.
	Picture pic;
	if (!_picsArchive.getPicture(kPicEAKidsLogo, pic)) {
		warning("EA Kids logo (%u) load failed", kPicEAKidsLogo);
		return;
	}
	blitAt(pic, 0, 0);

	// _GetPalette(0x25) — load into our shadow buffer; do not upload.
	// The logo bitmap is on screen but invisible until the first
	// _OpenColorCycle(..., show=1) upload in pass 2 lights it up.
	byte fpal[kPalSize];
	if (!getSitePalette(kPalEAKids, fpal)) {
		warning("EA Kids palette (%u) load failed", kPalEAKids);
		return;
	}

	const uint kFrameMs = 40;  // FRAME_RATE = 0x19 (25 fps).
	bool aborted = false;

	for (uint j = 0; j < 2 && !aborted && !shouldQuit(); j++) {
		const bool show = (j != 0);
		int delayCount = 8;

		for (uint u = 0; u < 0x37 && !aborted && !shouldQuit(); u++) {
			if (show) {
				const uint32 frameEnd = g_system->getMillis() + kFrameMs;
				while (g_system->getMillis() < frameEnd && !aborted) {
					Common::Event ev;
					while (g_system->getEventManager()->pollEvent(ev)) {
						if (ev.type == Common::EVENT_QUIT ||
							ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
							_skipIntro = true;
							return;
						}
						if (ev.type == Common::EVENT_KEYDOWN ||
							ev.type == Common::EVENT_LBUTTONDOWN) {
							if (ev.type == Common::EVENT_KEYDOWN &&
								ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
								_skipIntro = true;
							}
							aborted = true;
							break;
						}
					}
					g_system->delayMillis(5);
				}
			}

			openColorCycle(fpal, 0x01, 0x6e, show);
			openColorCycle(fpal, 0x81, 0xee, show);
			delayCount--;
			if (delayCount == 0) {
				delayCount = 8;
				openColorCycle(fpal, 0x70, 0x80, show);
			}
			if (show)
				g_system->updateScreen();
		}
	}

	if (aborted) {
		fadeCurrentPaletteToBlack();
		return;
	}

	for (uint i = 0; i < 5 && !shouldQuit(); i++)
		openColorCycle(fpal, 0x70, 0x80, true);
	g_system->updateScreen();
	waitForInput(0x23 * kFrameMs);

	// _OpenFadeOut @ 2520:0093 — 16 linear steps from current palette to black.
	fadeCurrentPaletteToBlack();
}

void EEMEngine::showHighScoreLogo() {
	// _ShowHScoreLogo @ 2520:0799: PIC 0x20c + palette 0x27;
	// _OpenFadeIn; 50-tick wait @ 25 fps; _OpenFadeOut.
	Picture pic;
	if (!_picsArchive.getPicture(kPicHighScoreLogo, pic)) {
		warning("HighScore logo (%u) load failed", kPicHighScoreLogo);
		return;
	}
	blitAt(pic, 0, 0);

	// Force black before fade-in to avoid a 1-frame full-logo flash.
	byte target[kPalSize];
	if (!getSitePalette(kPalHighScore, target)) {
		warning("HighScore palette (%u) load failed", kPalHighScore);
		return;
	}
	byte black[kPalSize] = {};
	g_system->getPaletteManager()->setPalette(black, 0, 256);
	g_system->updateScreen();
	fadePaletteFromBlack(target);

	// 50 ticks @ 25 fps.
	waitForInput(2000);

	fadeCurrentPaletteToBlack();
}

void EEMEngine::showFloppyStormLogo() {
	// Floppy storm-logo splash — `FUN_23d2_0605 @ 23d2:0605`:
	//   GetPicture(0x20b); BlitToVGA;
	//   if (sound) { LoadVOC(slot 25 = "thunder.voc"); PlayVOC(...); }
	//   GetPalette(0x26); FadeIn; wait 50 ticks; FadeOut.
	// CD plays `BOLT.ANM` here with `THUNDER.VOC` overlaid; floppy uses
	// a static still + the same VOC.
	Picture pic;
	if (!_picsArchive.getPicture(kPicStormLogo, pic)) {
		warning("Storm logo (%u) load failed", kPicStormLogo);
		return;
	}
	blitAt(pic, 0, 0);

	byte target[kPalSize];
	if (!getSitePalette(kPalStormLogo, target)) {
		warning("Storm palette (%u) load failed", kPalStormLogo);
		return;
	}
	byte black[kPalSize] = {};
	g_system->getPaletteManager()->setPalette(black, 0, 256);
	g_system->updateScreen();

	if (_audio)
		_audio->playVoc(Common::Path("THUNDER.VOC"));

	fadePaletteFromBlack(target);
	waitForInput(2000);
	fadeCurrentPaletteToBlack();

	if (_audio)
		_audio->stopVoice();
}

void EEMEngine::doSiteLoop() {
	// Per-mystery site loop. SiteScreen::run() handles hotspot clicks
	// plus M (map), N (notebook), G (gallery), A (accuse), Tab (next
	// site), ESC (exit).
	SiteScreen screen(this, &_mystery);
	screen.run();
	setHotspotMouseCursor(false);
}

void EEMEngine::startTravelMusic() {
	// _StartTravelMusic @ 20a2:0595:
	//   for (num = _SiteNumber; num > 4; num -= 5) {}
	//   if (_MIDIAvailable && _MusicEnabled) {
	//       if (_IsMIDIPlaying()) _StopMIDI();
	//       _MIDIPlay(num);
	//   }
	// Five travel tracks (MUS00000.XMI..MUS00004.XMI), picked by
	// `_SiteNumber % 5`. ONE-SHOT — `_DoOpeningAnims @ 2520:0945` resets
	// `_LoopMIDI = 0` after the title-screen wait, and the function
	// doesn't write it; combined with `_DoSiteLoop @ 168d:06c0` calling
	// `_StopMIDI()` before the interactive phase, travel music plays
	// ONCE during the entrance animation only.
	if (!_music || !_mystery.isLoaded() || !_voiceOn)
		return;
	const uint num = _mystery._siteNumber % 5;
	_music->playMus(num, /* loop= */ false);
}

void EEMEngine::waitForMusicDone(uint32 maxMs) {
	if (!_music)
		return;

	const uint32 startMs = g_system->getMillis();
	while (_music->isPlaying() && !shouldQuit() &&
		   g_system->getMillis() - startMs < maxMs) {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				stopMusic();
				return;
			}
			if (ev.type == Common::EVENT_KEYDOWN ||
				ev.type == Common::EVENT_LBUTTONDOWN) {
				stopMusic();
				return;
			}
		}
		g_system->updateScreen();
		g_system->delayMillis(20);
	}
	stopMusic();
}

void EEMEngine::stopMusic() {
	if (_music)
		_music->stop();
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
	return !_mystery.isLoaded();
}

bool EEMEngine::canSaveGameStateCurrently(Common::U32String *) {
	return true;
}

Common::Error EEMEngine::saveGameStream(Common::WriteStream *stream,
										 bool isAutosave) {
	(void)isAutosave;

	// Body header: explicit 1-byte version.
	Common::Serializer s(nullptr, stream);
	s.setVersion(kSaveBodyVer);
	byte ver = kSaveBodyVer;
	s.syncAsByte(ver);

	// _PlayerRecord body @ 2d5d:3f6a (159 bytes, written by
	// _SavePlayerRecord @ 1c33:034f). Layout:
	//   +0x00..+0x0b : player name (12 chars, null-padded)
	//   +0x0c..+0x1f : random ID bytes for `_GenerateFilename`
	//                  (29be:0dbf "C:\EEMCDSAV\%s.PLR") — irrelevant to
	//                  ScummVM saves which key on slot, not filename.
	//   +0x20..+0x28 : derived 8-char .PLR basename — likewise unused.
	//   +0x2d        : voice-enable flag (`DAT_2d5d_3f97`, default 1)
	//   +0x2f        : chain stage (`DAT_2d5d_3f99`, 1=A, 2=B, 3=C;
	//                  `_DisplayCorrect` advances once every case in the
	//                  current set is solved)
	//   +0x31..+0xa6 : `mysteriesSolved[55]` u16 (0=unsolved, 1=solved,
	//                  2=solved on first try — `_DisplayCorrect` writes
	//                  1 always, 2 when `_FirstTry != 0`)
	// We persist the gameplay-meaningful subset and skip the original's
	// filename-derivation bytes.
	s.syncString(_playerName);
	s.syncBytes(_mysteriesSolved, sizeof(_mysteriesSolved));
	s.syncAsByte(_partner);
	s.syncAsByte(_chainStage);
	s.syncAsByte(_voiceOn);

	// Mid-case resume: persist in-progress mystery (no equivalent in
	// _LoadGame @ 2404:0dc7).
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
	if (ver != kSaveBodyVer) {
		warning("loadGameStream: unsupported save body version %u (expected %u)",
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
			resetSiteArrivalState();
			return Common::kReadingFailed;
		}
		// `_ReadMystery @ 2404:008f` tail-calls `_InitMysterySounds`
		// (2404:0298) so the SDB index is in place for clue and
		// partner-speech spool sounds. Floppy ships individual
		// `M-XXXX.VOC` files instead of the bundled SDB/SDX archive,
		// so we skip the init there to avoid "missing" warnings;
		// `spoolSound` then no-ops via its `_currentMystery < 0` guard
		// until the per-voice VOC mapping is wired up.
		if (_audio && !isFloppy())
			_audio->initMysterySounds(mysteryNum);
		_mystery.syncState(s);
		if (_mystery._siteNumber < _mystery.numSites())
			setSiteArrivalState(_mystery._siteNumber);
		else
			resetSiteArrivalState();
	} else {
		_mystery.clear();
		resetSiteArrivalState();
	}
	applySkipRepeatedCasesOption();

	debugC(1, kDebugGeneral,
		   "Loaded profile name=%s partner=%u mystery=%d",
		   _playerName.c_str(), _partner,
		   _mystery.isLoaded() ? (int)_mystery.number() : -1);
	return Common::kNoError;
}

SaveStateList EEMEngine::listProfiles() const {
	// _findfirst("*.PLR") in screen8_handler @ 1c33:1012.
	// Filter out slot 0 (autosave) to match the original which
	// has no autosave concept.
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

	// Overwrite on matching description.
	int slot = -1;
	for (auto &s : saves) {
		if (s.getDescription() == name) {
			slot = s.getSaveSlot();
			break;
		}
	}

	// New profile: pick lowest unused slot >=1 (slot 0 is autosave;
	// DOS limit was 25 per screen8_handler local_8c[0x19][2]).
	if (slot < 0) {
		const int maxSlot = getMetaEngine()->getMaximumSaveSlot();
		Common::Array<bool> used(maxSlot + 1);
		for (auto &s : saves) {
			const int sl = s.getSaveSlot();
			if (sl >= 0 && sl <= maxSlot)
				used[sl] = true;
		}
		for (int i = 1; i <= maxSlot; i++) {
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
	return saveGameState(slot, name, /* isAutosave= */ false);
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
}

} // End of namespace EEM
