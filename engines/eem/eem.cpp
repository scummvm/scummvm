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
#include "common/engine_data.h"
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

// 1-based picture/palette IDs.
const uint kPicEAKidsLogo      = 0x54;  // _ShowEAKids
const uint kPicHighScoreLogo   = 0x20c; // _ShowHScoreLogo
const uint kPicStormLogo       = 0x20b; // Floppy storm-logo still
const uint kPalEAKids          = 0x25;
const uint kPalHighScore       = 0x27;
const uint kPalStormLogo       = 0x26;  // Floppy FUN_23d2_0605
const uint kPicMousePointer    = 0x50;  // 0x51 is the wait cursor

// EEM2 cursor table — `_main @ 1abf:0faf` loads seven cursor PICs into
// `_AnimationObjects`-adjacent slots and `_SwitchMouse @ 17ee:2c83` activates
// one by index. Order matches the slot order: 0 arrow, 1 wait, 2/3 examine,
// 4 Jake hand, 5 Jenny hand, 6 approach. Indexed by the site search-record
// cursor id (row +0xc).
const uint16 kLondonCursorPics[7] = {
	0x50, 0x51, 0x206, 0xa1, 0x207, 0x20b, 0x35e
};

const byte kSaveBodyVer = 1;

// Test switch: populate ScrapBook 1 at startup without exposing a game
// option or changing save format. Set false before release.
const bool kDebugPopulateScrapbook1AtStartup = false;

Common::String makeLondonProfileDisplayName(const Common::String &first,
											 const Common::String &last) {
	Common::String cleanFirst = first;
	Common::String cleanLast = last;
	cleanFirst.trim();
	cleanLast.trim();
	if (cleanLast.empty())
		return cleanFirst;
	if (cleanFirst.empty())
		return cleanLast;
	return cleanFirst + " " + cleanLast;
}

Common::String makeLondonProfileKey(const Common::String &first,
									 const Common::String &last) {
	Common::String cleanFirst = first;
	Common::String cleanLast = last;
	cleanFirst.trim();
	cleanLast.trim();
	Common::String key;
	for (uint i = 0; i < cleanFirst.size() && key.size() < 7; i++)
		key += cleanFirst[i];
	for (uint i = 0; i < cleanLast.size() && key.size() < 8; i++)
		key += cleanLast[i];
	for (uint i = 0; i < key.size(); i++) {
		if (key[i] == ' ' || key[i] == '.')
			key.setChar('_', i);
	}
	key.toUppercase();
	return key;
}

Common::String londonProfileKeyFromDisplayName(const Common::String &name) {
	Common::String clean = name;
	clean.trim();
	const size_t split = clean.findFirstOf(' ');
	if (split == Common::String::npos)
		return makeLondonProfileKey(clean, Common::String());

	Common::String first = clean.substr(0, split);
	Common::String last = clean.substr(split + 1);
	last.trim();
	return makeLondonProfileKey(first, last);
}

void fadeCurrentPaletteToBlack(uint delayMs) {
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

void fadePaletteFromBlack(const byte *target, uint delayMs) {
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
	if (!pics.getPicture(kPicMousePointer, cursor) || cursor.surface.empty())
		error("EEM: mouse cursor PIC 0x%x missing", kPicMousePointer);

	const byte transparent = (byte)(cursor.flags >> 8);
	CursorMan.replaceCursor(cursor.surface.rawSurface(), 0, 0, transparent);
	if (interactive)
		setInteractiveCursorPalette(cursor, transparent);
	else
		CursorMan.replaceCursorPalette(nullptr, 0, 0);
}

EEMEngine::EEMEngine(OSystem *syst, const ADGameDescription *gameDesc)
	: Engine(syst), _gameDescription(gameDesc), _rng("eem"),
	  _playerName("Detective"),
	  _lastScreen(kScreenInvalid), _nextScreen(kScreenTitle), _partner(0) {
	ConfMan.registerDefault("hide_highlight_boxes", false);
	ConfMan.registerDefault("fit_dialog_balloons", false);
	ConfMan.registerDefault("skip_repeated_cases", false);
	ConfMan.registerDefault("restored_content", false);

	_variant = (gameDesc && gameDesc->extra &&
				Common::String(gameDesc->extra).contains("Floppy"))
				 ? kVariantFloppy : kVariantCD;
	if (gameDesc && gameDesc->gameId &&
		Common::String(gameDesc->gameId) == "eem2")
		_variant = kVariantLondonCD;
	setLondonAnimScripts(isLondon());
	_language = gameDesc ? gameDesc->language : Common::EN_ANY;
}

EEMEngine::~EEMEngine() {
	delete _audio;
	delete _music;
}

void EEMEngine::applyStartupTestOverrides() {
	if (!kDebugPopulateScrapbook1AtStartup)
		return;

	uint lo = 0, hi = 0;
	if (mysteryTierRange(1, lo, hi)) {
		for (uint i = lo; i <= hi && i < sizeof(_mysteriesSolved); i++)
			_mysteriesSolved[i] = 1;
	}

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

bool EEMEngine::anyMysterySolved(uint lo, uint hi) const {
	for (uint i = lo; i <= hi && i < sizeof(_mysteriesSolved); i++) {
		if (_mysteriesSolved[i] != 0)
			return true;
	}
	return false;
}

bool EEMEngine::mysteryTierRange(uint stage, uint &lo, uint &hi) const {
	if (isLondon()) {
		// EEM2/London: two 25-case books
		// (`_DisplayCorrect @ 1ea1:0619`).
		switch (stage) {
		case 1: lo = 0x01; hi = 0x19; return true;  //  1..25
		case 2: lo = 0x1a; hi = 0x32; return true;  // 26..50
		default: return false;
		}
	}
	// EEM1: Junior 1..24, Senior 25..48, Master 49..54
	// (`_DisplayCorrect @ 1df2:073c`).
	switch (stage) {
	case 1: lo = 0x01; hi = 0x18; return true;  //  1..24
	case 2: lo = 0x19; hi = 0x30; return true;  // 25..48
	case 3: lo = 0x31; hi = 0x36; return true;  // 49..54
	default: return false;
	}
}

void EEMEngine::advanceChainStageAfterSolve(uint mysteryNum) {
	if (mysteryNum == 0 || _chainStage >= 4)
		return;

	uint lo = 0, hi = 0;
	if (!mysteryTierRange(_chainStage, lo, hi))
		return;
	if (!areMysteriesSolved(lo, hi))
		return;

	const uint oldStage = _chainStage;
	if (!isLondon() && _chainStage == 1 &&
		ConfMan.getBool("skip_repeated_cases"))
		_chainStage = 3;
	else
		_chainStage++;

	if (isLondon() && _chainStage == 3)
		_chainStage = 4;

	debugC(1, kDebugMystery,
		   "chainStage advanced from %u to %u after solving mystery %u",
		   oldStage, _chainStage, mysteryNum);
}

void EEMEngine::applySkipRepeatedCasesOption() {
	if (isLondon() || !ConfMan.getBool("skip_repeated_cases"))
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
	initGraphics(kScreenWidth, kScreenHeight);

	if (!isFloppy() && ConfMan.getBool("restored_content")) {
		Common::U32String engineDataError;
		if (!Common::load_engine_data("eem.dat", "eem", 1, 0,
									  engineDataError)) {
			warning("EEM restored content unavailable: %s",
					Common::String(engineDataError).c_str());
		} else {
			_restoredContentDataLoaded = true;
		}
	}

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

	if (isLondon()) {
		runLondonStartup();
		if (!shouldQuit()) {
			CursorMan.showMouse(true);
			applyStartupTestOverrides();
			applySkipRepeatedCasesOption();
			if (_mystery.isLoaded()) {
				_nextScreen = kScreenMap;
			} else {
				doChoosePartner();
				(void)saveProfile(_playerName);
				if (_profileCreatedThisSession) {
					_nextScreen = startLondonTrainingMystery()
						? kScreenInitClues : kScreenInvalid;
				} else {
					_nextScreen = kScreenAction;
				}
			}
		}
		goto screenLoop;
	}

	_skipIntro = false;
	if (isFloppy()) {
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
			if (!shouldQuit() && !_skipIntro && i != 20 && _audio) {
				_audio->spoolSound((uint)(i - 1));
				_audio->waitForSpoolDone();
			}
		}
		if (_audio)
			_audio->cleanMysterySounds();
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

	CursorMan.showMouse(true);

	if (_music)
		_music->stop();

	if (!shouldQuit())
		doProfilePicker();
	if (!shouldQuit())
		applyStartupTestOverrides();
	if (!shouldQuit())
		applySkipRepeatedCasesOption();
	if (!shouldQuit())
		doChoosePartner();

	if (!shouldQuit() && !resumed)
		_nextScreen = _mystery.isLoaded() ? kScreenMap : kScreenAction;
screenLoop:
	while (!shouldQuit() && _nextScreen != kScreenInvalid) {
		const ScreenId current = (ScreenId)_nextScreen;
		debugC(1, kDebugGeneral, "screenDriver: id=%d", (int)current);

		switch (current) {
		case kScreenTitle:
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
			_nextScreen = kScreenInvalid;
			doActionScreen();
			if (_nextScreen == kScreenInvalid && _mystery.isLoaded())
				_nextScreen = kScreenInitClues;
			break;

		case kScreenChooseMystery:
			_nextScreen = kScreenInvalid;
			doCaseSelection();
			if (isLondon())
				stopMusic();
			if (_nextScreen == kScreenInvalid && _mystery.isLoaded())
				_nextScreen = kScreenInitClues;
			break;

		case kScreenInitClues:
			doInitClues();
			_nextScreen = _mystery.isLoaded() ? kScreenMap
											  : kScreenAction;
			break;

		case kScreenMap:
		case kScreenMapAlt:
			doBigMap();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenSite:
			doSiteLoop();
			if (!_mystery.isLoaded())
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenInvalid;
			break;

		case kScreenNotebook:
			doNotebook();
			if (isLondon())
				stopMusic();
			if (!_mystery.isLoaded() && _nextScreen != kScreenAction)
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenGallery:
			doGallery();
			if (isLondon())
				stopMusic();
			if (!_mystery.isLoaded() && _nextScreen != kScreenAction)
				_nextScreen = kScreenAction;
			else if (_nextScreen == current)
				_nextScreen = kScreenSite;
			break;

		case kScreenSetup:
			if (isLondon())
				doSetupLondon();
			else
				doSetup();
			break;

		case kScreenProfile:
			_nextScreen = kScreenInvalid;
			_mystery.clear();
			doProfilePicker();
			if (!shouldQuit())
				applyStartupTestOverrides();
			if (!shouldQuit())
				applySkipRepeatedCasesOption();
			if (!shouldQuit() && (!isLondon() || !_mystery.isLoaded()))
				doChoosePartner();
			if (!shouldQuit() && isLondon() && !_mystery.isLoaded())
				(void)saveProfile(_playerName);
			if (!shouldQuit()) {
				if (_mystery.isLoaded())
					_nextScreen = kScreenMap;
				else if (isLondon() && _profileCreatedThisSession)
					_nextScreen = startLondonTrainingMystery()
						? kScreenInitClues : kScreenInvalid;
				else
					_nextScreen = kScreenAction;
			}
			break;

		case kScreenAccuse:
			doAccuse();
			if (isLondon())
				stopMusic();
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
	// The red-outline highlight replaced any London cursor shape; force the
	// next setSiteHotspotCursorId to reinstall.
	_siteCursorId = -1;
}

void EEMEngine::setHotspotMouseCursor(bool active) {
	if (isLondon()) {
		if (!active)
			setSiteHotspotCursorId(0);
		return;
	}
	setInteractiveMouseCursor(active);
}

void EEMEngine::setSiteHotspotCursorId(int cursorId) {
	if (!isLondon())
		return;
	if (cursorId == 4 && _partner == kPartnerJenny)
		cursorId = 5;
	if (cursorId < 0 || cursorId >= (int)ARRAYSIZE(kLondonCursorPics))
		cursorId = 0;
	if (cursorId == _siteCursorId)
		return;

	Picture cursor;
	if (!_picsArchive.getPicture(kLondonCursorPics[cursorId], cursor) ||
		cursor.surface.empty()) {
		warning("EEM2: cursor %d (PIC 0x%x) missing", cursorId,
				kLondonCursorPics[cursorId]);
		return;
	}
	// Each cursor carries its own transparent colour (flags >> 8) and uses the
	// active screen palette (no separate cursor palette, unlike the EEM1
	// red-outline highlight). Hotspot is the top-left (rowoff/misc are 0).
	const byte transparent = (byte)(cursor.flags >> 8);
	CursorMan.replaceCursor(cursor.surface.rawSurface(), 0, 0, transparent);
	CursorMan.replaceCursorPalette(nullptr, 0, 0);
	_siteCursorId = cursorId;
	// This London cursor replaced the red-outline highlight, if it was active.
	_interactiveMouseCursor = false;
}

bool EEMEngine::openArchives() {
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
	if (!_buttonArchive.open(Common::Path("BUTTON.DBD"), Common::Path("BUTTON.DBX")))
		warning("BUTTON archive missing — map markers will be unlabelled");
	return true;
}

bool EEMEngine::loadSitePalettes() {
	Common::File f;
	const char *palFile = isLondon() ? "SITEPALS." : "SITEPALS";
	if (!f.open(Common::Path(palFile))) {
		warning("%s missing", palFile);
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
	if (_sitePals.size() < (num + 1) * kPalSize)
		return false;
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
	if (_audio) {
		_audio->stopVoice();
		_audio->stopSpool();
	}
	if (stopMusicToo && _music)
		_music->stop();
}

void EEMEngine::playAnm(const Common::Path &path, uint frameDelayMs,
						bool holdLastFrame, bool fadeIn,
						bool setSkipIntroOnEsc) {
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
						if (setSkipIntroOnEsc) {
							_skipIntro = true;
							interruptAudio();
						}
					}
					aborted = true;
					break;
				}
			}
			g_system->updateScreen();
			g_system->delayMillis(5);
		}
		if (aborted)
			break;
	}

	if (holdLastFrame && !shouldQuit() && !_skipIntro) {
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
						if (setSkipIntroOnEsc) {
							_skipIntro = true;
							interruptAudio();
						}
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
		if (_music)
			_music->stop();
	}
}

void EEMEngine::blitAt(const Picture &pic, int x, int y) {
	const int w = MIN<int>(pic.surface.w, kScreenWidth - x);
	const int h = MIN<int>(pic.surface.h, kScreenHeight - y);
	if (w <= 0 || h <= 0)
		return;
	g_system->copyRectToScreen(pic.surface.getPixels(), pic.surface.pitch,
							   x, y, w, h);
}

void EEMEngine::waitForInput(uint32 maxMs) {
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

// _OpenColorCycle @ 2520:04f7. Rotate `fpal[start..end]` by one slot
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
// _ShowEAKids @ 2520:05f0:
void EEMEngine::showEAKidsLogo() {
	Picture pic;
	if (!_picsArchive.getPicture(kPicEAKidsLogo, pic)) {
		warning("EA Kids logo (%u) load failed", kPicEAKidsLogo);
		return;
	}
	blitAt(pic, 0, 0);

	byte fpal[kPalSize];
	if (!getSitePalette(kPalEAKids, fpal)) {
		warning("EA Kids palette (%u) load failed", kPalEAKids);
		return;
	}

	const uint kFrameMs = 40;
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

	fadeCurrentPaletteToBlack();
}

// _ShowHScoreLogo @ 2520:0799: PIC 0x20c + palette 0x27;
void EEMEngine::showHighScoreLogo() {
	Picture pic;
	if (!_picsArchive.getPicture(kPicHighScoreLogo, pic)) {
		warning("HighScore logo (%u) load failed", kPicHighScoreLogo);
		return;
	}
	blitAt(pic, 0, 0);

	byte target[kPalSize];
	if (!getSitePalette(kPalHighScore, target)) {
		warning("HighScore palette (%u) load failed", kPalHighScore);
		return;
	}
	byte black[kPalSize] = {};
	g_system->getPaletteManager()->setPalette(black, 0, 256);
	g_system->updateScreen();
	fadePaletteFromBlack(target);

	waitForInput(2000);

	fadeCurrentPaletteToBlack();
}

void EEMEngine::showLondonLogo(uint picId, uint palId, uint holdMs) {
	Picture pic;
	if (!_picsArchive.getPicture(picId, pic) || pic.surface.empty()) {
		warning("London logo PIC 0x%x load failed", picId);
		return;
	}
	blitAt(pic, 0, 0);

	byte target[kPalSize];
	if (!getSitePalette(palId, target)) {
		warning("London palette 0x%x load failed", palId);
		return;
	}
	byte black[kPalSize] = {};
	g_system->getPaletteManager()->setPalette(black, 0, 256);
	g_system->updateScreen();
	fadePaletteFromBlack(target);

	waitForInput(holdMs);
	fadeCurrentPaletteToBlack();
}

bool EEMEngine::startLondonTrainingMystery() {
	if (_mystery.load(0, &_rng)) {
		resetSiteArrivalState();
		if (_audio)
			_audio->initMysterySounds(0);
		debugC(1, kDebugMystery,
			   "London: training mystery 0 loaded — %u sites, %u suspects",
			   _mystery.numSites(), _mystery.numSuspects());
		return true;
	}

	warning("London: training mystery 0 (M0.BIN) failed to load");
	return false;
}

void EEMEngine::runLondonStartup() {
	const uint32 kHoldForever = 0xFFFFFFFFu;
	CursorMan.showMouse(false);
	_skipIntro = false;
	debugC(1, kDebugGeneral, "EEM2 (London): opening sequence");

	// Two still logos.
	if (!shouldQuit() && !_skipIntro)
		showLondonLogo(0x54, 0x3c, 2500);   // EA Kids
	if (!shouldQuit() && !_skipIntro)
		showLondonLogo(0x20c, 0x3e, 2500);  // publisher logo (FUN_2721_07be)

	// Storm Software — bolt.anm with the thunder roar.
	if (!shouldQuit() && !_skipIntro) {
		if (_audio)
			_audio->playVoc(Common::Path("THUNDER.VOC"));
		playAnm(Common::Path("BOLT.ANM"), 120, /* holdLastFrame= */ false,
				/* fadeIn= */ true);
		if (_audio)
			_audio->stopVoice();
		fadeCurrentPaletteToBlack();
	}

	// Intro movie with its theme (MUS00101.XMI).
	if (!shouldQuit() && !_skipIntro && _music)
		_music->playFile(Common::Path("MUS00101.XMI"), /* loop= */ false);
	if (!shouldQuit() && !_skipIntro)
		playAnm(Common::Path("MOVIE.ANM"), 120, /* holdLastFrame= */ false,
				/* fadeIn= */ true);

	// Animated title (wave.anm) over the looping theme (MUS00102.XMI);
	// a click / key advances to character creation. The original loops
	// wave.anm; we play it once, hold the last frame and wait.
	if (!shouldQuit() && !_skipIntro && _music)
		_music->playFile(Common::Path("MUS00102.XMI"), /* loop= */ true);
	if (!shouldQuit() && !_skipIntro)
		playAnm(Common::Path("WAVE.ANM"), 120, /* holdLastFrame= */ true,
				/* fadeIn= */ true);
	if (!shouldQuit() && !_skipIntro)
		waitForInput(kHoldForever);
	if (_music)
		_music->stop();
	_skipIntro = false;

	// Screen 8 profile selection. If no profile exists or the player chooses
	// the bottom "new player" sentinel, this opens London `_NewPlayer`.
	if (!shouldQuit()) {
		CursorMan.showMouse(true);
		doProfilePicker();
	}

	debugC(1, kDebugGeneral, "EEM2 (London): intro + profile selection done");
}

// `_NewPlayer` @ 1cd3:0f27 — character creation over background PIC 0xc
void EEMEngine::showLondonCharSelect() {
	debugC(1, kDebugGeneral, "EEM2 (London): character creation");

	const Common::Rect kFirstRect(54, 75, 151, 85);
	const Common::Rect kLastRect(167, 75, 266, 85);
	const Common::Rect kMaleBox(110, 116, 120, 122);
	const Common::Rect kFemaleBox(190, 116, 200, 122);
	const uint kMaxFirst = 12, kMaxLast = 20;
	const uint8 kInkColor = 0x22;

	Picture bg;
	const bool haveBg = _picsArchive.getPicture(0xc, bg) && !bg.surface.empty();
	if (!haveBg)
		warning("London: passport background PIC 0xc failed to load");
	const uint8 boxBlankColor = haveBg
		? (uint8)bg.surface.getPixel(kFirstRect.left, kFirstRect.top)
		: 0x38;
	const uint8 boxMarkColor = haveBg
		? (uint8)bg.surface.getPixel(109, 115)
		: 0x22;

	byte pal[kPalSize];
	byte black[kPalSize] = {};
	g_system->getPaletteManager()->setPalette(black, 0, 256);
	g_system->updateScreen();
	const bool havePal = getSitePalette(0, pal);
	if (!havePal)
		warning("London: passport palette 0 failed to load");

	Common::Event staleEvent;
	while (g_system->getEventManager()->pollEvent(staleEvent)) {
		if (staleEvent.type == Common::EVENT_QUIT ||
			staleEvent.type == Common::EVENT_RETURN_TO_LAUNCHER)
			return;
	}

	CursorMan.showMouse(true);
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	_profileCreatedThisSession = false;

	enum { kFieldFirst = 0, kFieldLast = 1, kFieldGender = 2 };
	int field = kFieldFirst;
	Common::String first, last;
	bool female = false;
	bool blink = true, fadedIn = false, done = false, needRedraw = true;
	bool genderReady = false;
	bool genderCanConfirm = true;
	uint32 blinkMs = g_system->getMillis();

	while (!done && !shouldQuit()) {
		if (needRedraw) {
			Graphics::ManagedSurface scratch(kScreenWidth, kScreenHeight,
				Graphics::PixelFormat::createFormatCLUT8());
			scratch.clear();
			if (haveBg)
				scratch.simpleBlitFrom(bg.surface);
			if (getFont().isLoaded()) {
				getFont().drawString(&scratch, first, kFirstRect.left + 2,
									 kFirstRect.top + 1, kFirstRect.width(),
									 kInkColor);
				getFont().drawString(&scratch, last, kLastRect.left + 2,
									 kLastRect.top + 1, kLastRect.width(),
									 kInkColor);
				if (blink && (field == kFieldFirst || field == kFieldLast)) {
					const Common::Rect &fr =
						(field == kFieldFirst) ? kFirstRect : kLastRect;
					const Common::String &buf =
						(field == kFieldFirst) ? first : last;
					const int caretX =
						fr.left + 2 + getFont().getStringWidth(buf);
					Common::Rect caret(caretX, fr.top, caretX + 6,
									   fr.top + 0xb);
					caret.clip(Common::Rect(kScreenWidth, kScreenHeight));
					if (!caret.isEmpty())
						scratch.fillRect(caret, kInkColor);
				}
			}
			if (field == kFieldGender) {
				scratch.fillRect(kMaleBox, boxBlankColor);
				scratch.fillRect(kFemaleBox, boxBlankColor);
				scratch.fillRect(female ? kFemaleBox : kMaleBox,
								 boxMarkColor);
			}
			CursorMan.showMouse(true);
			g_system->copyRectToScreen(scratch.getPixels(), scratch.pitch,
									   0, 0, kScreenWidth, kScreenHeight);
			if (!fadedIn) {
				if (havePal)
					fadePaletteFromBlack(pal);
				fadedIn = true;
			}
			g_system->updateScreen();
			needRedraw = false;
			if (field == kFieldGender)
				genderReady = true;
		}

		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			if (ev.type == Common::EVENT_QUIT ||
				ev.type == Common::EVENT_RETURN_TO_LAUNCHER) {
				CursorMan.showMouse(true);
				g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
				return;
			}
			if (field == kFieldGender && ev.type == Common::EVENT_LBUTTONDOWN) {
				if (!genderReady)
					continue;
				female = ev.mouse.x >= 160;
				done = true;
				needRedraw = true;
				break;
			}
			if (field == kFieldGender && ev.type == Common::EVENT_KEYUP &&
				(ev.kbd.keycode == Common::KEYCODE_RETURN ||
				 ev.kbd.keycode == Common::KEYCODE_KP_ENTER)) {
				genderCanConfirm = true;
				continue;
			}
			if (ev.type != Common::EVENT_KEYDOWN)
				continue;
			const Common::KeyCode k = ev.kbd.keycode;
			const bool enter = (k == Common::KEYCODE_RETURN ||
								k == Common::KEYCODE_KP_ENTER);
			if (field == kFieldFirst || field == kFieldLast) {
				Common::String &buf = (field == kFieldFirst) ? first : last;
				const uint cap = (field == kFieldFirst) ? kMaxFirst : kMaxLast;
				if (enter || k == Common::KEYCODE_TAB) {
					Common::String clean = buf;
					clean.trim();
					if (!clean.empty()) {
						field++;  // advance to last name, then to gender pick
						if (field == kFieldGender) {
							genderReady = false;
							genderCanConfirm = false;
						}
					}
				} else if (k == Common::KEYCODE_BACKSPACE && !buf.empty()) {
					buf.deleteLastChar();
				} else if (ev.kbd.ascii >= ' ' && ev.kbd.ascii < 127 &&
						   buf.size() < cap) {
					buf += (char)ev.kbd.ascii;
				}
			} else {  // gender pick
				if (k == Common::KEYCODE_LEFT) {
					female = false;
					genderCanConfirm = true;
				} else if (k == Common::KEYCODE_RIGHT) {
					female = true;
					genderCanConfirm = true;
				} else if (k == Common::KEYCODE_BACKSPACE) {
					field = kFieldLast;  // back to editing
					genderCanConfirm = true;
				} else if (enter && genderReady && genderCanConfirm) {
					done = true;
				}
			}
			needRedraw = true;
			if (field == kFieldGender && !genderReady)
				break;
		}

		const uint32 now = g_system->getMillis();
		if (field != kFieldGender && now - blinkMs >= 400) {
			blinkMs = now;
			blink = !blink;
			needRedraw = true;
		}
		g_system->updateScreen();
		g_system->delayMillis(15);
	}
	if (shouldQuit()) {
		CursorMan.showMouse(true);
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
		return;
	}

	while (g_system->getEventManager()->pollEvent(staleEvent)) {
		if (staleEvent.type == Common::EVENT_QUIT ||
			staleEvent.type == Common::EVENT_RETURN_TO_LAUNCHER) {
			CursorMan.showMouse(true);
			g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
			return;
		}
	}
	CursorMan.showMouse(true);
	g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	const Common::String displayName =
		makeLondonProfileDisplayName(first, last);
	const Common::String profileKey = makeLondonProfileKey(first, last);

	for (const SaveStateDescriptor &s : listProfiles()) {
		if (londonProfileKeyFromDisplayName(s.getDescription()) != profileKey)
			continue;

		const Common::Error err = loadGameState(s.getSaveSlot());
		if (err.getCode() == Common::kNoError) {
			_profileCreatedThisSession = false;
			debugC(1, kDebugGeneral,
				   "London: existing player key=%s loaded from slot %d (%s)",
				   profileKey.c_str(), s.getSaveSlot(),
				   s.getDescription().c_str());
			return;
		}
		warning("London: failed to load existing profile '%s' at slot %d",
				s.getDescription().c_str(), s.getSaveSlot());
	}

	_playerName = displayName.empty() ? "Detective" : displayName;
	_chainStage = 1;
	_playerFemale = female;
	_voiceOn = true;
	if (_audio)
		_audio->setVoiceEnabled(_voiceOn);
	for (uint i = 0; i < sizeof(_mysteriesSolved); i++)
		_mysteriesSolved[i] = 0;
	_mystery.clear();
	resetSiteArrivalState();
	_profileCreatedThisSession = true;
	const Common::Error err = saveProfile(_playerName);
	if (err.getCode() != Common::kNoError)
		warning("London: failed to save new profile '%s': %s",
				_playerName.c_str(), err.getDesc().c_str());
	debugC(1, kDebugGeneral, "London: new player='%s' key=%s gender=%s",
		   _playerName.c_str(), profileKey.c_str(),
		   female ? "female" : "male");
}
// Floppy storm-logo splash — `23d2:0605`:
void EEMEngine::showFloppyStormLogo() {
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
	SiteScreen screen(this, &_mystery);
	screen.run();
	setHotspotMouseCursor(false);
}

// _StartTravelMusic @ 20a2:0595:
void EEMEngine::startTravelMusic() {
	if (!_music || !_mystery.isLoaded() ||
		(isLondon() ? !_musicOn : !_voiceOn))
		return;
	const uint num = _mystery._siteNumber % 5;
	_music->playMus(num, /* loop= */ false);
}

void EEMEngine::startLondonTravelMusic(uint8 travelKind) {
	static const uint16 kLondonTravelMusic[4][3] = {
		{ 0,  0,  0 },
		{ 3, 22, 25 },
		{ 7, 23, 17 },
		{ 10, 21, 24 },
	};
	if (!_music || !_musicOn || travelKind == 0 ||
		travelKind >= ARRAYSIZE(kLondonTravelMusic))
		return;

	const uint track = kLondonTravelMusic[travelKind][_rng.getRandomNumber(2)];
	_music->playMus(track, /* loop= */ false);
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
	//                  us since saves which key on slot, not filename.
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
	s.syncAsByte(_musicOn);

	byte playerFemale = _playerFemale ? 1 : 0;
	s.syncAsByte(playerFemale);

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
	s.syncAsByte(_musicOn);

	byte playerFemale = 0;
	s.syncAsByte(playerFemale);
	_playerFemale = (playerFemale != 0);

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
	const Common::String londonKey =
		isLondon() ? londonProfileKeyFromDisplayName(name) : Common::String();

	// Overwrite on matching description.
	int slot = -1;
	for (auto &s : saves) {
		if (s.getDescription() == name ||
			(isLondon() &&
			 londonProfileKeyFromDisplayName(s.getDescription()) == londonKey)) {
			slot = s.getSaveSlot();
			break;
		}
	}

	// New profile: pick lowest unused slot >=1 (slot 0 is autosave);
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
	const Common::String londonKey =
		isLondon() ? londonProfileKeyFromDisplayName(name) : Common::String();
	for (auto &s : saves) {
		if (s.getDescription() == name ||
			(isLondon() &&
			 londonProfileKeyFromDisplayName(s.getDescription()) == londonKey)) {
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
