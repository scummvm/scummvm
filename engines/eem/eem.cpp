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
const uint kPalEAKids          = 0x25;
const uint kPalHighScore       = 0x27;

// Save format. Used by `saveGameState` / `loadGameState`.
const uint32 kSaveMagic = MKTAG('E', 'E', 'M', '0');
const byte   kSaveVer   = 3;  ///< v2: _mysteriesSolved tracker; v3: player name

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
				if (!_mystery.isLoaded())
					break;
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

void EEMEngine::doSiteLoop() {
	// Mirrors the per-mystery site loop. SiteScreen::run() handles
	// hotspot clicks plus M (map), N (notebook), G (gallery), A (accuse),
	// Tab (next site), ESC (exit).
	SiteScreen screen(this, &_mystery);
	screen.run();
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
