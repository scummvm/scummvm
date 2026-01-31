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

#include "audio/audiostream.h"
#include "audio/decoders/wave.h"
#include "common/archive.h"
#include "common/compression/installshieldv3_archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/macresman.h"
#include "common/language.h"
#include "common/compression/stuffit.h"
#include "graphics/paletteman.h"
#include "engines/util.h"
#include "image/bmp.h"

#include "private/decompiler.h"
#include "private/grammar.h"
#include "private/private.h"
#include "private/savegame.h"
#include "private/tokens.h"

namespace Private {

PrivateEngine *g_private = nullptr;
extern int parse(const char *);

PrivateEngine::PrivateEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr), _videoDecoder(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0), _frameImage(nullptr),
	  _framePalette(nullptr),
	  _videoSubtitles(nullptr), _sfxSubtitles(false), _useSubtitles(false),
	  _defaultCursor(nullptr),
	  _screenW(640), _screenH(480) {
	_highlightMasks = false;
	_rnd = new Common::RandomSource("private");

	// Global object for external reference
	g_private = this;

	// Setting execution
	_nextSetting = "";
	_currentSetting = "";
	_pausedSetting = "";
	_pausedMovieName = "";
	_modified = false;
	_mode = -1;
	_toTake = false;
	_haveTakenItem = false;

	// Movies
	_nextMovie = "";
	_currentMovie = "";
	_nextVS = "";
	_repeatedMovieExit = "";

	// Save and load
	_saveGameMask.clear();
	_loadGameMask.clear();

	// Interface
	_framePath = "inface/general/inface2.bmp";

	// Police
	resetPoliceBust();
	_sirenSound = "po/audio/posfx002.wav";

	// General sounds
	_globalAudioPath = "global/audio/";
	_noStopSounds = false;

	// Radios and phone
	_policeRadioArea.clear();
	_AMRadioArea.clear();
	_phoneArea.clear();
	_AMRadio.path = "inface/radio/comm_/";
	_AMRadio.sound = &_AMRadioSound;
	_policeRadio.path = "inface/radio/police/";
	_policeRadio.sound = &_policeRadioSound;
	_phonePrefix = "inface/telephon/";

	// Dossiers
	_dossierPage = 0;
	_dossierSuspect = 0;
	_dossierPageMask.clear();
	_dossierNextSuspectMask.clear();
	_dossierPrevSuspectMask.clear();
	_dossierNextSheetMask.clear();
	_dossierPrevSheetMask.clear();

	// Diary
	_diaryLocPrefix = "inface/diary/loclist/";
	_currentDiaryPage = -1;

	// Safe
	_safeNumberPath = "sg/search_s/sgsaf%d.bmp";
	for (uint d = 0 ; d < 3; d++) {
		_safeDigitArea[d].clear();
		_safeDigitRect[d] = Common::Rect(0, 0);
	}

	// Timer
	clearTimer();
}

PrivateEngine::~PrivateEngine() {
	destroyVideo();
	destroySubtitles();

	delete _compositeSurface;
	if (_frameImage != nullptr) {
		_frameImage->free();
		delete _frameImage;
	}
	if (_mframeImage != nullptr) {
		_mframeImage->free();
		delete _mframeImage;
	}
	free(_framePalette);

	delete _rnd;
	delete _image;

	delete Gen::g_vm;
	delete Settings::g_setts;

	delete _defaultCursor;
	for (uint i = 0; i < _cursors.size(); i++) {
		if (_cursors[i].winCursorGroup == nullptr) {
			delete _cursors[i].cursor;
		}
		delete _cursors[i].winCursorGroup;
	}

	for (MaskList::const_iterator it = _masks.begin(); it != _masks.end(); ++it) {
		const MaskInfo &m = *it;
		if (m.surf != nullptr) {
			m.surf->free();
			delete m.surf;
		}
	}

	if (_phoneArea.surf != nullptr) {
		_phoneArea.surf->free();
		delete _phoneArea.surf;
	}

	for (uint i = 0; i < ARRAYSIZE(_safeDigitArea); i++) {
		if (_safeDigitArea[i].surf != nullptr) {
			_safeDigitArea[i].surf->free();
			delete _safeDigitArea[i].surf;
		}
	}

	for (RectList::iterator it = _rects.begin(); it != _rects.end(); ++it) {
		Common::Rect *r = (*it);
		delete r;
	}
}

void PrivateEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath, 0, 10);
}

Common::SeekableReadStream *PrivateEngine::loadAssets() {

	Common::File *test = new Common::File();

	if (isDemo() && test->open("SUPPORT/ASSETS/DEMOGAME.WIN"))
		return test;

	if (isDemo() && test->open("SUPPORT/DEMOGAME.MAC"))
		return test;
	if (test->open("SUPPORT/ASSETS/GAME.WIN"))
		return test;
	if (test->open("SUPPORT/GAME.MAC"))
		return test;

	delete test;

	if (_platform == Common::kPlatformMacintosh) {
		Common::ScopedPtr<Common::Archive> macInstaller(loadMacInstaller());
		if (macInstaller) {
			const char *macFileName = isDemo() ? "demogame.mac" : "game.mac";
			Common::SeekableReadStream *file = macInstaller->createReadStreamForMember(macFileName);
			if (file != nullptr) {
				return file;
			}
		}
	}

	Common::InstallShieldV3 installerArchive;
	if (!installerArchive.open("SUPPORT/ASSETS.Z"))
		error("Failed to open SUPPORT/ASSETS.Z");
	// if the full game is used
	if (!isDemo()) {
		if (installerArchive.hasFile("GAME.DAT"))
			return installerArchive.createReadStreamForMember("GAME.DAT");
		if (installerArchive.hasFile("GAME.WIN"))
			return installerArchive.createReadStreamForMember("GAME.WIN");
		error("Unknown version");
		return nullptr;
	}

	// if the demo from archive.org is used
	if (installerArchive.hasFile("GAME.TXT"))
		return installerArchive.createReadStreamForMember("GAME.TXT");

	// if the demo from the full retail CDROM is used
	if (installerArchive.hasFile("DEMOGAME.DAT"))
		return installerArchive.createReadStreamForMember("DEMOGAME.DAT");
	if (installerArchive.hasFile("DEMOGAME.WIN"))
		return installerArchive.createReadStreamForMember("DEMOGAME.WIN");

	error("Unknown version");
	return nullptr;
}

Common::Archive *PrivateEngine::loadMacInstaller() {
	const char *fileName;
	if (_language == Common::JA_JPN) {
		fileName = "xn--16jc8na7ay6a0eyg9e5nud0e4525d";
	} else if (isDemo()) {
		fileName = "Private Eye Demo Installer";
	} else {
		fileName = "Private Eye Installer";
	}

	Common::SeekableReadStream *archiveFile = Common::MacResManager::openFileOrDataFork(fileName);
	if (archiveFile == nullptr) {
		return nullptr;
	}

	// createStuffItArchive() takes ownership of incoming stream, even on failure
	return createStuffItArchive(archiveFile, true);
}

Common::Error PrivateEngine::run() {

	// Only enable if subtitles are available
	if (!Common::parseBool(ConfMan.get("subtitles"), _useSubtitles))
		warning("Failed to parse bool from subtitles options");

	if (!Common::parseBool(ConfMan.get("sfxSubtitles"), _sfxSubtitles))
		warning("Failed to parse bool from sfxSubtitles options");

	if (!Common::parseBool(ConfMan.get("highlightMasks"), _shouldHighlightMasks))
		warning("Failed to parse bool from highlightMasks options");

	if (!_useSubtitles && _sfxSubtitles) {
		warning("SFX subtitles are enabled, but no subtitles will be shown");
	}

	_language = Common::parseLanguage(ConfMan.get("language"));
	_platform = Common::parsePlatform(ConfMan.get("platform"));

	Common::SeekableReadStream *file = loadAssets();
	// Read assets file
	const uint32 fileSize = file->size();
	char *buf = (char *)malloc(fileSize + 1);
	file->read(buf, fileSize);
	buf[fileSize] = '\0';

	Decompiler decomp(buf, fileSize, _platform == Common::kPlatformMacintosh);
	free(buf);

	Common::String scripts = decomp.getResult();
	debugC(1, kPrivateDebugCode, "code:\n%s", scripts.c_str());

	// Initialize stuff
	Gen::g_vm = new Gen::VM();
	Settings::g_setts = new Settings::SettingMaps();

	initFuncs();
	parse(scripts.c_str());
	delete file;
	if (maps.constants.size() == 0)
		error("Failed to parse game script");
	initializeWallSafeValue();

	// Initialize graphics
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_screenW, _screenH, &_pixelFormat);
	_transparentColor = 250;

	_screenRect = Common::Rect(0, 0, _screenW, _screenH);
	loadCursors();
	changeCursor("default");
	_origin = Common::Point(0, 0);
	_image = new Image::BitmapDecoder();
	_compositeSurface = new Graphics::ManagedSurface();
	_compositeSurface->create(_screenW, _screenH, _pixelFormat);
	_compositeSurface->setTransparentColor(_transparentColor);

	_currentDiaryPage = -1;

	// Load the game frame once
	byte *palette;
	bool isNewPalette;
	_frameImage = decodeImage(_framePath, &palette, &isNewPalette);
	if (isNewPalette) {
		free(palette);
		palette = nullptr;
	}
	_mframeImage = decodeImage(_framePath, &palette, &isNewPalette);

	_framePalette = (byte *) malloc(3*256);
	memcpy(_framePalette, palette, 3*256);
	if (isNewPalette) {
		free(palette);
		palette = nullptr;
	}

	byte *initialPalette;
	bool isNewInitialPalette;
	Graphics::Surface *surf = decodeImage("inface/general/inface1.bmp", &initialPalette, &isNewInitialPalette);
	_compositeSurface->setPalette(initialPalette, 0, 256);
	surf->free();
	delete surf;
	_image->destroy();
	if (isNewInitialPalette) {
		free(initialPalette);
		initialPalette = nullptr;
	}

	// Main event loop
	Common::Event event;
	Common::Point mousePos;
	_videoDecoder = nullptr;
	_pausedVideo = nullptr;
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else {
		_nextSetting = getGoIntroSetting();
	}

	_needToDrawScreenFrame = false;

	while (!shouldQuit()) {
		bool mouseMoved = false;
		checkTimer();
		checkPhoneCall();

		while (_system->getEventManager()->pollEvent(event)) {
			mousePos = _system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSkip) {
					if (!_timerSkipSetting.empty()) {
						skipTimer();
					} else {
						skipVideo();
					}
				}
				break;

			case Common::EVENT_SCREEN_CHANGED:
				adjustSubtitleSize();
				break;
			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (selectDossierNextSuspect(mousePos))
					break;
				if (selectDossierPrevSuspect(mousePos))
					break;
				if (selectDossierNextSheet(mousePos))
					break;
				if (selectDossierPrevSheet(mousePos))
					break;
				if (selectDossierPage(mousePos))
					break;
				if (selectSafeDigit(mousePos))
					break;
				if (selectDiaryNextPage(mousePos))
					break;
				if (selectDiaryPrevPage(mousePos))
					break;
				if (selectLocation(mousePos))
					break;
				if (selectMemory(mousePos)) {
					_needToDrawScreenFrame = true;
					break;
				}
				if (selectSkipMemoryVideo(mousePos))
					break;

				if (selectPhoneArea(mousePos))
					 break;
				if (selectPoliceRadioArea(mousePos))
					break;
				if (selectAMRadioArea(mousePos))
					break;
				if (selectLoadGame(mousePos))
					break;
				if (selectSaveGame(mousePos))
					break;
				if (_nextSetting.empty())
					if (selectMask(mousePos))
						break;
				if (_nextSetting.empty())
					if (selectExit(mousePos))
						break;
				selectPauseGame(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				mouseMoved = true;
				updateCursor(mousePos);
				break;

			default:
				break;
			}
		}

		checkPoliceBust();

		// Movies
		if (!_nextMovie.empty()) {
			clearTimer();
			_videoDecoder = new Video::SmackerDecoder();
			playVideo(_nextMovie);
			_currentMovie = _nextMovie;
			_nextMovie = "";
			updateCursor(mousePos);
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			if (_videoDecoder->getCurFrame() == 0) {
				stopSounds();
			}

			if (_videoDecoder->endOfVideo()) {
				delete _videoDecoder;
				_videoDecoder = nullptr;
				destroySubtitles();
				_currentMovie = "";
			} else if (!_videoDecoder->needsUpdate() && mouseMoved) {
				_system->updateScreen();
			} else if (_videoDecoder->needsUpdate()) {
				drawScreen();
			}
			_system->delayMillis(5); // Yield to the system
			continue;
		}

		if (!_nextSetting.empty()) {
			clearTimer();
			debugC(1, kPrivateDebugFunction, "Executing %s", _nextSetting.c_str());
			clearAreas();
			_currentSetting = _nextSetting;
			Settings::g_setts->load(_nextSetting);
			_nextSetting = "";
			_currentVS = "";
			Gen::g_vm->run();

			// Draw the screen once the VM has processed the last setting.
			// This prevents the screen from flickering images as VM settings
			// are executed. Fixes the previous screen from being displayed
			// when a video finishes playing.
			if (_nextSetting.empty()) {
				if (!_nextVS.empty() && _currentVS.empty() && _currentSetting == getMainDesktopSetting()) {
					loadImage(_nextVS, 160, 120);
					_currentVS = _nextVS;
				}

				updateCursor(mousePos);
				drawScreen();
			}
		}

		_system->updateScreen();
		_system->delayMillis(10);
		updateSubtitles();
	}
	return Common::kNoError;
}

void PrivateEngine::initFuncs() {
	for (const Private::FuncTable *fnc = funcTable; fnc->name; fnc++) {
		Common::String name(fnc->name);
		_functions.setVal(name, (void *)fnc->func);
	}
}

void PrivateEngine::clearAreas() {
	for (MaskList::const_iterator it = _masks.begin(); it != _masks.end(); ++it) {
		const MaskInfo &m = *it;
		if (m.surf != nullptr) {
			m.surf->free();
			delete m.surf;
		}
	}

	_exits.clear();
	_masks.clear();
	_highlightMasks = false;
	_locationMasks.clear();
	_memoryMasks.clear();

	_loadGameMask.clear();
	_saveGameMask.clear();
	_policeRadioArea.clear();
	_AMRadioArea.clear();
	if (_phoneArea.surf != nullptr) {
		_phoneArea.surf->free();
		delete _phoneArea.surf;
	}
	_phoneArea.clear();
	_dossierPageMask.clear();
	_dossierNextSuspectMask.clear();
	_dossierPrevSuspectMask.clear();
	_dossierNextSheetMask.clear();
	_dossierPrevSheetMask.clear();
	_diaryNextPageExit.clear();
	_diaryPrevPageExit.clear();

	for (uint d = 0 ; d < 3; d++) {
		if (_safeDigitArea[d].surf) {
			_safeDigitArea[d].surf->free();
			delete _safeDigitArea[d].surf;
		}
		_safeDigitArea[d].clear();
		_safeDigitRect[d] = Common::Rect(0, 0);
	}
}

void PrivateEngine::resetPoliceBust() {
	_policeBustEnabled = false;
	_policeSirenPlayed = false;
	_numberOfClicks = 0;
	_numberClicksAfterSiren = 0;
	_policeBustMovieIndex = 0;
	_policeBustMovie = "";
	_policeBustPreviousSetting = "";
}

void PrivateEngine::startPoliceBust() {
	_policeBustEnabled = true;
	_policeSirenPlayed = false;

	// Calculate two click counts:
	// 1. the number of clicks until the siren warning
	// 2. the number of clicks after the siren warning until the bust
	// This logic was extracted from the executable.
	int policeIndex = maps.variables.getVal(getPoliceIndexVariable())->u.val;
	if (policeIndex > 20) {
		policeIndex = 21;
	}
	int r = _rnd->getRandomNumber(11);
	int numberOfClicks = r + ((policeIndex * 14) / -21) + 16;
	_numberClicksAfterSiren = _rnd->getRandomNumber(6) + 3;
	if ((numberOfClicks - _numberClicksAfterSiren) <= 2) {
		_numberOfClicks = 2;
	} else {
		_numberOfClicks = numberOfClicks - _numberClicksAfterSiren;
	}
}

void PrivateEngine::stopPoliceBust() {
	_policeBustEnabled = false;
}

void PrivateEngine::wallSafeAlarm() {
	// This logic was extracted from the executable.
	// It skips the siren and randomly alters the number of clicks
	// until the police arrive. This may increase or decrease the
	// number of clicks, but there will always be at least 1 left.

	_policeSirenPlayed = true;
	int r1 = _rnd->getRandomNumber(3);
	int r2 = _rnd->getRandomNumber(3);
	if (r1 + r2 + 1 <= _numberOfClicks) {
		r1 = _rnd->getRandomNumber(3);
		r2 = _rnd->getRandomNumber(3);
		_numberOfClicks = r1 + r2 + 1;
	}
}

void PrivateEngine::completePoliceBust() {
	if (!_policeBustPreviousSetting.empty()) {
		_nextSetting = _policeBustPreviousSetting;
	}

	int policeIndex = maps.variables.getVal(getPoliceIndexVariable())->u.val;
	if (policeIndex > 13) {
		return;
	}

	// Set kPoliceArrived. This flag is cleared by the wall safe alarm.
	Symbol *policeArrived = maps.variables.getVal(getPoliceArrivedVariable());
	setSymbol(policeArrived, 1);

	// Select the movie for BustMovie() to play
	_policeBustMovie =
		Common::String::format("po/animatio/spoc%02dxs.smk",
			kPoliceBustVideos[_policeBustMovieIndex]);

	// Play audio on the second bust movie
	if (kPoliceBustVideos[_policeBustMovieIndex] == 2) {
		Common::String s("global/transiti/audio/spoc02VO.wav");
		stopSounds();
		playForegroundSound(s);
		changeCursor("default");
		waitForSoundsToStop();
	}

	// Cycle to the next movie and wrap around
	_policeBustMovieIndex = (_policeBustMovieIndex + 1) % ARRAYSIZE(kPoliceBustVideos);

	_nextSetting = getPOGoBustMovieSetting();
}

void PrivateEngine::checkPoliceBust() {
	if (_mode != 1) {
		return;
	}

	if (!_policeBustEnabled) {
		return;
	}

	if (_numberOfClicks >= 0) {
		return;
	}

	if (!_policeSirenPlayed) {
		// Play siren
		playForegroundSound(_sirenSound);

		_policeSirenPlayed = true;
		_numberOfClicks = _numberClicksAfterSiren;
	} else {
		// Bust Marlowe.
		// The original seems to record _currentSetting instead of
		// _nextSetting, but that causes a click to do nothing if it
		// triggers a police bust that doesn't do anything except for
		// restoring the current scene.
		if (!_nextSetting.empty()) {
			_policeBustPreviousSetting = _nextSetting;
		} else {
			_policeBustPreviousSetting = _currentSetting;
		}
		// The next setting is indeed kPoliceBustFromMO, even though it
		// occurs from all locations and is unrelated to Marlowe's office.
		// According to comments in the game script, Marlowe's office
		// originally required a special mode but it was later removed.
		// Apparently the developers didn't rename the setting.
		_nextSetting = getPoliceBustFromMOSetting();
		_policeBustEnabled = false;
	}
}

void PrivateEngine::updateCursor(Common::Point mousePos) {
	// If a function returns true then it changed the cursor.
	if (cursorPhoneArea(mousePos)) {
		return;
	}
	if (cursorSafeDigit(mousePos)) {
		return;
	}
	if (cursorMask(mousePos)) {
		return;
	}
	if (cursorExit(mousePos)) {
		return;
	}
	if (cursorPauseMovie(mousePos)) {
		return;
	}
	changeCursor("default");
}

bool PrivateEngine::cursorExit(Common::Point mousePos) {
	mousePos = mousePos - _origin;

	int rs = 100000000;
	int cs = 0;
	Common::String cursor;

	for (ExitList::const_iterator it = _exits.begin(); it != _exits.end(); ++it) {
		const ExitInfo &e = *it;
		cs = e.rect.width() * e.rect.height();

		if (e.rect.contains(mousePos)) {
			if (cs < rs && !e.cursor.empty()) {
				rs = cs;
				cursor = e.cursor;
			}
		}
	}

	if (!cursor.empty()) {
		changeCursor(cursor);
		return true;
	}

	return false;
}

bool PrivateEngine::cursorSafeDigit(Common::Point mousePos) {
	if (_safeDigitArea[0].surf == nullptr) {
		return false;
	}

	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0) {
		return false;
	}

	for (uint i = 0; i < 3; i++) {
		MaskInfo &m = _safeDigitArea[i];
		if (m.surf != nullptr) {
			if (_safeDigitRect[i].contains(mousePos) && !m.cursor.empty()) {
				changeCursor(m.cursor);
				return true;
			}
		}
	}

	return false;
}

bool PrivateEngine::inMask(Graphics::Surface *surf, Common::Point mousePos) {
	if (surf == nullptr)
		return false;

	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

	if (mousePos.x > surf->w || mousePos.y > surf->h)
		return false;

	return (surf->getPixel(mousePos.x, mousePos.y) != _transparentColor);
}

bool PrivateEngine::cursorMask(Common::Point mousePos) {
	bool inside = false;
	for (MaskList::const_iterator it = _masks.begin(); it != _masks.end(); ++it) {
		const MaskInfo &m = *it;

		bool inArea = m.useBoxCollision ? m.box.contains(mousePos) : inMask(m.surf, mousePos);
		if (inArea) {
			if (!m.cursor.empty()) { // TODO: check this
				inside = true;
				changeCursor(m.cursor);
				break;
			}
		}
	}
	return inside;
}

bool PrivateEngine::cursorPauseMovie(Common::Point mousePos) {
	if (_mode == 1) {
		uint32 tol = 15;
		Common::Rect window(_origin.x - tol, _origin.y - tol, _screenW - _origin.x + tol, _screenH - _origin.y + tol);
		if (!window.contains(mousePos)) {
			changeCursor("default");
			return true;
		}
	}
	return false;
}

Common::String PrivateEngine::getPauseMovieSetting() {
	return getSymbolName("kPauseMovie", "k3");
}

Common::String PrivateEngine::getGoIntroSetting() {
	return getSymbolName("kGoIntro", "k1");
}

Common::String PrivateEngine::getAlternateGameVariable() {
	return getSymbolName("kAlternateGame", "k2");
}

Common::String PrivateEngine::getMainDesktopSetting() {
	return getSymbolName("kMainDesktop", "k183", "k45");
}

Common::String PrivateEngine::getDiaryTOCSetting() {
	return getSymbolName("kDiaryTOC", "k185");
}

Common::String PrivateEngine::getDiaryMiddleSetting() {
	return getSymbolName("kDiaryMiddle", "k186");
}

Common::String PrivateEngine::getDiaryLastPageSetting() {
	return getSymbolName("kDiaryLastPage", "k187");
}

Common::String PrivateEngine::getPoliceIndexVariable() {
	return getSymbolName("kPoliceIndex", "k0");
}

Common::String PrivateEngine::getPOGoBustMovieSetting() {
	return getSymbolName("kPOGoBustMovie", "k7");
}

Common::String PrivateEngine::getPoliceBustFromMOSetting() {
	return getSymbolName("kPoliceBustFromMO", "k6");
}

Common::String PrivateEngine::getListenToPhoneSetting() {
	return getSymbolName("kListenToPhone", "k9");
}

Common::String PrivateEngine::getWallSafeValueVariable() {
	return getSymbolName("kWallSafeValue", "k3");
}

Common::String PrivateEngine::getPoliceArrivedVariable() {
	return getSymbolName("kPoliceArrived", "k7");
}

Common::String PrivateEngine::getBeenDowntownVariable() {
	return getSymbolName("kBeenDowntown", "k8");
}

Common::String PrivateEngine::getPoliceStationLocation() {
	return getSymbolName("kLocationPO", "k12");
}

Common::String PrivateEngine::getExitCursor() {
	return getSymbolName("kExit", "k5");
}

Common::String PrivateEngine::getInventoryCursor() {
	return getSymbolName("kInventory", "k7");
}

const char *PrivateEngine::getSymbolName(const char *name, const char *strippedName, const char *demoName) {
	if (_platform == Common::kPlatformWindows) {
		if (_language == Common::EN_USA ||
			_language == Common::JA_JPN ||
			_language == Common::KO_KOR ||
			_language == Common::RU_RUS) {
			return name;
		}
	}

	if (demoName != nullptr && isDemo()) {
		return demoName;
	}

	return strippedName;
}

bool PrivateEngine::isSlotActive(const SubtitleSlot &slot) {
	return slot.subs != nullptr && _mixer->isSoundHandleActive(slot.handle);
}

bool PrivateEngine::isSfxSubtitle(const Video::Subtitles *subs) {
	if (!subs)
		return false;
	return subs->isSfx();
}

void PrivateEngine::selectPauseGame(Common::Point mousePos) {
	if (_mode == 1) {
		uint32 tol = 15;
		Common::Rect window(_origin.x - tol, _origin.y - tol, _screenW - _origin.x + tol, _screenH - _origin.y + tol);
		if (!window.contains(mousePos)) {
			// Pause game and return to desktop
			if (_pausedSetting.empty()) {
				if (!_nextSetting.empty())
					_pausedSetting = _nextSetting;
				else
					_pausedSetting = _currentSetting;

				_nextSetting = getPauseMovieSetting();
				if (_videoDecoder) {
					_videoDecoder->pauseVideo(true);
					_pausedVideo = _videoDecoder;
					_pausedMovieName = _currentMovie;
				}
				if (_videoSubtitles || _voiceSlot.subs || _sfxSlot.subs) {
					_system->hideOverlay();
				}

				_pausedBackgroundSoundName = _bgSound.name;

				_compositeSurface->fillRect(_screenRect, 0);
				_compositeSurface->setPalette(_framePalette, 0, 256);
				_origin = Common::Point(kOriginZero[0], kOriginZero[1]);
				drawMask(_frameImage);
				_origin = Common::Point(kOriginOne[0], kOriginOne[1]);
			}
		}
	}
}

void PrivateEngine::resumeGame() {
	_nextSetting = _pausedSetting;
	_pausedSetting = "";
	_mode = 1;
	_origin = Common::Point(kOriginOne[0], kOriginOne[1]);

	if (_pausedVideo != nullptr) {
		_videoDecoder = _pausedVideo;
		_pausedVideo = nullptr;

		// restore the name we saved in selectPauseGame
		if (!_pausedMovieName.empty()) {
			_currentMovie = _pausedMovieName;
			_pausedMovieName.clear();
		}
	}

	// always reload subtitles if a movie is active
	// we do this unconditionally because the casebook might have loaded
	// different subtitles while we were paused
	if (!_currentMovie.empty()) {
		loadSubtitles(convertPath(_currentMovie), kSubtitleVideo);
	}

	if (_videoDecoder) {
		_videoDecoder->pauseVideo(false);
		_needToDrawScreenFrame = true;
	}

	if (!_pausedBackgroundSoundName.empty()) {
		playBackgroundSound(_pausedBackgroundSoundName);
		_pausedBackgroundSoundName.clear();
	}
	// force draw the subtitle once
	// the screen was likely wiped by the pause menu
	// to account for the subtitle which was already rendered and we wiped the screen before it finished we must
	// force the subtitle system to ignore its cache and redraw the text.
	// calling adjustSubtitleSize() makes the next drawSubtitle call perform a full redraw
	// automatically, so we don't need to pass 'true'
	adjustSubtitleSize();
	if (_videoSubtitles || _voiceSlot.subs || _sfxSlot.subs) {
		_system->showOverlay(false);
		_system->clearOverlay();

		// redraw video subtitles
		if (_videoDecoder && _videoSubtitles)
			_videoSubtitles->drawSubtitle(_videoDecoder->getTime(), false, _sfxSubtitles);

		// draw all remaining active subtitles
		if (isSlotActive(_voiceSlot)) {
			uint32 time = _mixer->getElapsedTime(_voiceSlot.handle).msecs();
			_voiceSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
		} else if (isSlotActive(_sfxSlot)) {
			uint32 time = _mixer->getElapsedTime(_sfxSlot.handle).msecs();
			_sfxSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
		}
	}
}


bool PrivateEngine::selectExit(Common::Point mousePos) {
	mousePos = mousePos - _origin;

	Common::String ns = "";
	int rs = 100000000;
	for (ExitList::const_iterator it = _exits.begin(); it != _exits.end(); ++it) {
		const ExitInfo &e = *it;
		int cs = e.rect.width() * e.rect.height();
		//debug("Testing exit %s %d", e.nextSetting->c_str(), cs);
		if (e.rect.contains(mousePos)) {
			//debug("Inside! %d %d", cs, rs);
			if (cs < rs && !e.nextSetting.empty()) { // TODO: check this
				// an item was not taken
				if (_toTake) {
					playForegroundSound(_takeLeaveSound, getLeaveSound());
					_toTake = false;
				}

				//debug("Found Exit %s %d", e.nextSetting->c_str(), cs);
				rs = cs;
				ns = e.nextSetting;
			}
		}
	}
	if (!ns.empty()) {
		if (_mode == 1) {
			_numberOfClicks--; // count click only if it hits a hotspot
		}
		_nextSetting = ns;
		_highlightMasks = false;
		return true;
	}
	return false;
}

bool PrivateEngine::selectMask(Common::Point mousePos) {
	Common::String ns;
	for (MaskList::const_iterator it = _masks.begin(); it != _masks.end(); ++it) {
		const MaskInfo &m = *it;
		//debug("Testing mask %s", m.nextSetting->c_str());
		if (inMask(m.surf, mousePos)) {
			//debug("Inside!");
			if (!m.nextSetting.empty()) { // TODO: check this
				//debug("Found Mask %s", m.nextSetting->c_str());
				ns = m.nextSetting;
			}

			if (m.flag1 != nullptr) { // TODO: check this
				// an item was taken
				if (_toTake) {
					addInventory(m.inventoryItem, *(m.flag1->name));
					playForegroundSound(_takeLeaveSound, getTakeSound());
					_toTake = false;
					_haveTakenItem = true;
				}
			}

			if (m.flag2 != nullptr) {
				setSymbol(m.flag2, 1);
			}
			break;
		}
	}
	if (!ns.empty()) {
		if (_mode == 1) {
			_numberOfClicks--; // count click only if it hits a hotspot
		}
		_nextSetting = ns;
		_highlightMasks = false;
		return true;
	}
	return false;
}

bool PrivateEngine::selectLocation(const Common::Point &mousePos) {
	if (_locationMasks.size() == 0) {
		return false;
	}

	uint i = 0;
	int totalLocations = 0;
	for (auto &it : maps.locationList) {
		const Private::Symbol *sym = maps.locations.getVal(it);
		if (sym->u.val) {
			if (_locationMasks[i].box.contains(mousePos)) {
				bool diaryPageSet = false;
				for (uint j = 0; j < _diaryPages.size(); j++) {
					if (_diaryPages[j].locationID == totalLocations + 1) {
						_currentDiaryPage = j;
						diaryPageSet = true;
						break;
					}
				}

				// Prevent crash if there are no memories for this location
				if (!diaryPageSet) {
					return true;
				}

				_nextSetting = _locationMasks[i].nextSetting;

				return true;
			}
			i++;
		}
		totalLocations++;
	}

	return false;
}

bool PrivateEngine::selectDiaryNextPage(Common::Point mousePos) {
	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

	if (_diaryNextPageExit.rect.contains(mousePos)) {
		_currentDiaryPage++;
		_nextSetting = _diaryNextPageExit.nextSetting;

		playForegroundSound(getPaperShuffleSound());

		return true;
	}

	return false;
}

bool PrivateEngine::selectDiaryPrevPage(Common::Point mousePos) {
	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

	if (_diaryPrevPageExit.rect.contains(mousePos)) {
		_currentDiaryPage--;
		_nextSetting = _diaryPrevPageExit.nextSetting;

		playForegroundSound(getPaperShuffleSound());

		return true;
	}

	return false;
}

bool PrivateEngine::selectMemory(const Common::Point &mousePos) {
	for (uint i = 0; i < _memoryMasks.size(); i++) {
		if (inMask(_memoryMasks[i].surf, mousePos)) {
			clearAreas();
			_nextMovie = _diaryPages[_currentDiaryPage].memories[i].movie;
			_nextSetting = getDiaryMiddleSetting();
			return true;
		}
	}

	return false;
}

void PrivateEngine::addMemory(const Common::String &path) {
	size_t index = path.findLastOf('\\');
	Common::String location = path.substr(index + 2, 2);

	Common::String imagePath;

	// Paths to the global folder have a different pattern from other paths
	if (path.contains("global")) {
		if (path.contains("spoc00xs")) {
			imagePath = "inface/diary/ss_icons/global/transiti/ipoc00.bmp";
		} else {
			imagePath = "inface/diary/ss_icons/global/transiti/animatio/mo/imo" + path.substr(index + 4, 3) + ".bmp";
		}
	} else {
		// First letter after the last \ is an s, which isn't needed; next 2 are location; and the next 3 are what image to use
		imagePath = "inface/diary/ss_icons/" + location + "/i" + location + path.substr(index + 4, 3) + ".bmp";
	}

	if (!Common::File::exists(convertPath(imagePath))) {
		return;
	}

	MemoryInfo memory;
	memory.movie = path;
	memory.image = imagePath;

	for (int i = _diaryPages.size() - 1; i >= 0; i--) {
		if (_diaryPages[i].locationName == location) {
			if (_diaryPages[i].memories.size() == 6) {
				DiaryPage diaryPage;
				diaryPage.locationName = location;
				diaryPage.locationID = _diaryPages[i].locationID;
				diaryPage.memories.push_back(memory);
				_diaryPages.insert_at(i + 1, diaryPage);
				return;
			}
			_diaryPages[i].memories.push_back(memory);

			return;
		}
	}

	DiaryPage diaryPage;
	diaryPage.locationName = location;
	diaryPage.locationID = -1;

	uint locationIndex = 0;
	for (auto &it : maps.locationList) {
		Private::Symbol *sym = maps.locations.getVal(it);
		locationIndex++;

		Common::String currentLocation = it.substr(9);
		if (it.size() <= 3) {
			if (it == "k0") {
				currentLocation = "mo";
			} else if (it == "k1") {
				currentLocation = "is";
			} else if (it == "k2") {
				currentLocation = "mw";
			} else if (it == "k3") {
				currentLocation = "cs";
			} else if (it == "k4") {
				currentLocation = "cw";
			} else if (it == "k5") {
				currentLocation = "ts";
			} else if (it == "k6") {
				currentLocation = "bo";
			} else if (it == "k7") {
				currentLocation = "gz";
			} else if (it == "k8") {
				currentLocation = "sg";
			} else if (it == "k9") {
				currentLocation = "da";
			} else if (it == "k10") {
				currentLocation = "dl";
			} else if (it == "k11") {
				currentLocation = "vn";
			} else if (it == "k12") {
				currentLocation = "po";
			} else if (it == "k13") {
				currentLocation = "dc";
			} else
				error("Unknown location symbol %s", it.c_str());
		}

		currentLocation.toLowercase();
		if (currentLocation == location) {
			// Ensure that the location is marked as visited.
			// Police station video spoc00xs can be played before the
			// police station has been visited if the player has not
			// been busted by the police yet.
			setLocationAsVisited(sym);

			diaryPage.locationID = locationIndex;
			break;
		}
	}
	assert(diaryPage.locationID != -1);

	diaryPage.memories.push_back(memory);

	for (int i = _diaryPages.size() - 1; i >= 0; i--) {
		if (_diaryPages[i].locationID < diaryPage.locationID) {
			_diaryPages.insert_at(i + 1, diaryPage);
			return;
		}
	}

	_diaryPages.insert_at(0, diaryPage);
}

bool PrivateEngine::inInventory(const Common::String &bmp) const {
	for (InvList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->diaryImage == bmp)
			return true;
	}
	return false;
}

void PrivateEngine::addInventory(const Common::String &bmp, Common::String &flag) {
	// set game flag
	if (!flag.empty()) {
		Symbol *sym = maps.lookupVariable(&flag);
		setSymbol(sym, 1);
	}

	// add to casebook
	if (!inInventory(bmp)) {
		InventoryItem i;
		i.diaryImage = bmp;
		i.flag = flag;
		inventory.push_back(i);
	}
}

void PrivateEngine::removeInventory(const Common::String &bmp) {
	for (InvList::iterator it = inventory.begin(); it != inventory.end(); ++it) {
		if (it->diaryImage == bmp) {
			// clear game flag
			if (!it->flag.empty()) {
				Symbol *sym = maps.lookupVariable(&(it->flag));
				setSymbol(sym, 0);
			}
			// remove from casebook
			inventory.erase(it);
			break;
		}
	}
}

void PrivateEngine::removeRandomInventory() {
	// This logic was extracted from the executable.
	// Examples:
	//   0-3 items:  0 items removed
	//   4-6 items:  1 item removed
	//   7-10 items: 2 items removed
	uint numberOfItemsToRemove = (inventory.size() * 30) / 100;
	for (uint i = 0; i < numberOfItemsToRemove; i++) {
		uint indexToRemove = _rnd->getRandomNumber(inventory.size() - 1);
		uint index = 0;
		for (InvList::iterator it = inventory.begin(); it != inventory.end(); ++it) {
			if (index == indexToRemove) {
				removeInventory(it->diaryImage);
				break;
			}
			index++;
		}
	}
}

bool PrivateEngine::selectAMRadioArea(Common::Point mousePos) {
	if (_AMRadioArea.surf == nullptr)
		return false;

	if (inMask(_AMRadioArea.surf, mousePos)) {
		playRadio(_AMRadio, false);
		return true;
	}
	return false;
}

bool PrivateEngine::selectPoliceRadioArea(Common::Point mousePos) {
	if (_policeRadioArea.surf == nullptr)
		return false;

	if (inMask(_policeRadioArea.surf, mousePos)) {
		playRadio(_policeRadio, true);
		return true;
	}
	return false;
}

void PrivateEngine::addDossier(Common::String &page1, Common::String &page2) {
	// Each dossier page can only be added once.
	// Do this even when loading games to fix saves with duplicates.
	for (uint i = 0; i < _dossiers.size(); i++) {
		if (_dossiers[i].page1 == page1) {
			return;
		}
	}

	DossierInfo d;
	d.page1 = page1;
	d.page2 = page2;
	_dossiers.push_back(d);
}

void PrivateEngine::loadDossier() {
	int x = 40;
	int y = 30;

	MaskInfo m;
	DossierInfo d = _dossiers[_dossierSuspect];

	if (_dossierPage == 0) {
		m.surf = loadMask(d.page1, x, y, true);
	} else if (_dossierPage == 1) {
		m.surf = loadMask(d.page2, x, y, true);
	} else {
		error("Invalid page");
	}

	m.cursor = "default";
	_dossierPageMask = m;
	_masks.push_back(m); // not push_front, as this occurs after DossierChgSheet
}

bool PrivateEngine::selectDossierPage(Common::Point mousePos) {
	if (_dossierPageMask.surf == nullptr) {
		return false;
	}

	if (inMask(_dossierPageMask.surf, mousePos)) {
		return true;
	}
	return false;
}

bool PrivateEngine::selectDossierNextSuspect(Common::Point mousePos) {
	if (_dossierNextSuspectMask.surf == nullptr)
		return false;

	if (inMask(_dossierNextSuspectMask.surf, mousePos)) {
		if ((_dossierSuspect + 1) < _dossiers.size()) {
			playForegroundSound(getPaperShuffleSound());
			_dossierSuspect++;
			_dossierPage = 0;
			
			// reload kDossierOpen
			_nextSetting = _currentSetting;
		}
		return true;
	}
	return false;
}

bool PrivateEngine::selectDossierPrevSheet(Common::Point mousePos) {
	if (_dossierNextSheetMask.surf == nullptr)
		return false;

	if (inMask(_dossierPrevSheetMask.surf, mousePos)) {
		if (_dossierPage == 1) {
			playForegroundSound(getPaperShuffleSound());
			_dossierPage = 0;
			
			// reload kDossierOpen
			_nextSetting = _currentSetting;
		}
		return true;
	}
	return false;
}

bool PrivateEngine::selectDossierNextSheet(Common::Point mousePos) {
	if (_dossierNextSheetMask.surf == nullptr)
		return false;

	if (inMask(_dossierNextSheetMask.surf, mousePos)) {
		DossierInfo m = _dossiers[_dossierSuspect];
		if (_dossierPage == 0 && !m.page2.empty()) {
			playForegroundSound(getPaperShuffleSound());
			_dossierPage = 1;
			
			// reload kDossierOpen
			_nextSetting = _currentSetting;
		}
		return true;
	}
	return false;
}

bool PrivateEngine::selectDossierPrevSuspect(Common::Point mousePos) {
	if (_dossierPrevSuspectMask.surf == nullptr)
		return false;

	if (inMask(_dossierPrevSuspectMask.surf, mousePos)) {
		if (_dossierSuspect > 0) {
			playForegroundSound(getPaperShuffleSound());
			_dossierSuspect--;
			_dossierPage = 0;
			
			// reload kDossierOpen
			_nextSetting = _currentSetting;
		}
		return true;
	}
	return false;
}

void PrivateEngine::addRadioClip(
	Radio &radio, const Common::String &name, int priority,
	int disabledPriority1, bool exactPriorityMatch1,
	int disabledPriority2, bool exactPriorityMatch2,
	const Common::String &flagName, int flagValue) {

	// lookup radio clip by name
	RadioClip *clip = nullptr;
	for (uint i = 0; i < radio.clips.size(); i++) {
		if (radio.clips[i].name == name) {
			clip = &radio.clips[i];
			break;
		}
	}

	// add clip if new
	if (clip == nullptr) {
		RadioClip newClip;
		newClip.name = name;
		newClip.played = false;
		newClip.priority = priority;
		newClip.disabledPriority1 = disabledPriority1;
		newClip.exactPriorityMatch1 = exactPriorityMatch1;
		newClip.disabledPriority2 = disabledPriority2;
		newClip.exactPriorityMatch2 = exactPriorityMatch2;
		newClip.flagName = flagName;
		newClip.flagValue = flagValue;
		radio.clips.push_back(newClip);
		clip = &radio.clips[radio.clips.size() - 1];
	}

	// disable other clips based on the clip's priority
	disableRadioClips(radio, clip->priority);
}

void PrivateEngine::initializeAMRadioChannels(uint clipCount) {
	Radio &radio = _AMRadio;
	assert(clipCount < radio.clips.size());

	// clear all channels
	for (uint i = 0; i < ARRAYSIZE(radio.channels); i++) {
		radio.channels[i] = -1;
	}

	// build array of playable clip indexes (up to clipCount)
	Common::Array<uint> playableClips;
	for (uint i = 0; i < clipCount; i++) {
		if (!radio.clips[i].played) {
			playableClips.push_back(i);
		}
	}

	// place the highest priority clips in the channels (up to two)
	uint channelCount;
	switch (playableClips.size()) {
	case 0: channelCount = 0; break;
	case 1: channelCount = 1; break;
	case 2: channelCount = 1; break;
	case 3: channelCount = 1; break;
	default: channelCount = 2; break;
	}
	uint channel = 0;
	uint end = 0;
	while (channel < channelCount) {
		channel++;
		if (channel < playableClips.size()) {
			uint start = channel;
			uint remainingClips = playableClips.size() - start;
			while (remainingClips--) {
				RadioClip &clip1 = radio.clips[playableClips[start]];
				RadioClip &clip2 = radio.clips[playableClips[end]];
				if (clip1.priority < clip2.priority) {
					SWAP(playableClips[start], playableClips[end]);
				}
				start++;
			}
		}
		radio.channels[channel - 1] = playableClips[end];
		end++;
	}

	// build another array of playable clip indexes, starting at clipCount
	Common::Array<uint> morePlayableClips;
	for (uint i = clipCount; i < radio.clips.size(); i++) {
		if (!radio.clips[i].played) {
			morePlayableClips.push_back(i);
		}
	}

	// shuffle second array
	if (!morePlayableClips.empty()) {
		for (uint i = morePlayableClips.size() - 1; i > 0; i--) {
			uint n = _rnd->getRandomNumber(i);
			SWAP(morePlayableClips[i], morePlayableClips[n]);
		}
	}

	// install some of the clips from the second array into channels, starting
	// at the end of the channel array to keep the highest priority clips.
	uint copyCount = morePlayableClips.size();
	if (playableClips.size() <= 3) { // not morePlayableClips
		copyCount = MIN<uint>(copyCount, 2);
	} else {
		copyCount = MIN<uint>(copyCount, 1);
	}
	for (uint i = 0; i < copyCount; i++) {
		radio.channels[2 - i] = morePlayableClips[i];
	}

	// shuffle channels
	for (uint i = ARRAYSIZE(radio.channels) - 1; i > 0; i--) {
		uint n = _rnd->getRandomNumber(i);
		SWAP(radio.channels[i], radio.channels[n]);
	}
}

void PrivateEngine::initializePoliceRadioChannels() {
	Radio &radio = _policeRadio;

	// clear all channels
	for (uint i = 0; i < ARRAYSIZE(radio.channels); i++) {
		radio.channels[i] = -1;
	}

	// build array of playable clip indexes
	Common::Array<uint> playableClips;
	for (uint i = 0; i < radio.clips.size(); i++) {
		if (!radio.clips[i].played) {
			playableClips.push_back(i);
		}
	}

	// place the highest priority clips in the channels (up to three)
	uint channelCount = MIN<uint>(playableClips.size(), ARRAYSIZE(radio.channels));
	uint channel = 0;
	uint end = 0;
	while (channel < channelCount) {
		channel++;
		if (channel < playableClips.size()) {
			uint start = channel;
			uint remainingClips = playableClips.size() - start;
			while (remainingClips--) {
				RadioClip &clip1 = radio.clips[playableClips[start]];
				RadioClip &clip2 = radio.clips[playableClips[end]];
				if (clip1.priority < clip2.priority) {
					SWAP(playableClips[start], playableClips[end]);
				}
				start++;
			}
		}
		radio.channels[channel - 1] = playableClips[end];
		end++;
	}
}

void PrivateEngine::disableRadioClips(Radio &radio, int priority) {
	for (uint i = 0; i < radio.clips.size(); i++) {
		RadioClip &clip = radio.clips[i];
		if (clip.played) {
			continue;
		}

		if (clip.disabledPriority1) {
			if ((clip.exactPriorityMatch1 && priority == clip.disabledPriority1) ||
				(!clip.exactPriorityMatch1 && priority <= clip.disabledPriority1)) {
				clip.played = true;
			}
		}
		if (clip.disabledPriority2) {
			if ((clip.exactPriorityMatch2 && priority == clip.disabledPriority2) ||
				(!clip.exactPriorityMatch2 && priority <= clip.disabledPriority2)) {
				clip.played = true;
			}
		}
	}
}

void PrivateEngine::playRadio(Radio &radio, bool randomlyDisableClips) {
	// if radio is already playing then turn it off
	if (isSoundPlaying(*(radio.sound))) {
		stopForegroundSounds();
		return;
	}

	// search channels for first available clip
	for (uint i = 0; i < ARRAYSIZE(radio.channels); i++) {
		// skip empty channels
		if (radio.channels[i] == -1) {
			continue;
		}

		// verify that clip hasn't been already been played
		RadioClip &clip = radio.clips[radio.channels[i]];
		radio.channels[i] = -1;
		if (clip.played) {
			continue;
		}

		// the police radio randomly disables clips (!)
		if (randomlyDisableClips) {
			uint r = _rnd->getRandomNumber(9);
			if (r < 3) {
				clip.played = true;
				break; // play radio.wav
			}
		}

		// play the clip
		Common::String sound = radio.path + clip.name + ".wav";
		stopForegroundSounds();
		playForegroundSound(*(radio.sound), sound);
		clip.played = true;
		if (!clip.flagName.empty()) {
			Symbol *flag = maps.lookupVariable(&(clip.flagName));
			setSymbol(flag, clip.flagValue);
		}
		return;
	}

	// play default radio sound
	stopForegroundSounds();
	playForegroundSound(*(radio.sound), "inface/radio/radio.wav");
}

void PrivateEngine::addPhone(const Common::String &name, bool once, int startIndex, int endIndex, const Common::String &flagName, int flagValue) {
	// lookup phone clip by name and index range
	PhoneInfo *phone = nullptr;
	for (PhoneList::iterator it = _phones.begin(); it != _phones.end(); ++it) {
		if (it->name == name && it->startIndex == startIndex && it->endIndex == endIndex) {
			phone = &(*it);
			break;
		}
	}

	// add or update phone clip
	if (phone == nullptr) {
		PhoneInfo newPhone;
		newPhone.name = name;
		newPhone.once = once;
		newPhone.startIndex = startIndex;
		newPhone.endIndex = endIndex;
		newPhone.flagName = flagName;
		newPhone.flagValue = flagValue;
		newPhone.status = kPhoneStatusWaiting;
		newPhone.callCount = 0;
		newPhone.soundIndex = 0;
		// add single clip or a range of clips that occur in a random order
		if (startIndex == endIndex) {
			Common::String sound = name + ".wav";
			newPhone.sounds.push_back(sound);
		} else {
			for (int i = startIndex; i <= endIndex; i++) {
				Common::String sound = Common::String::format("%s%02d.wav", name.c_str(), i);
				newPhone.sounds.push_back(sound);
			}
			// shuffle
			for (uint i = newPhone.sounds.size() - 1; i > 0; i--) {
				uint n = _rnd->getRandomNumber(i);
				SWAP<Common::String>(newPhone.sounds[i], newPhone.sounds[n]);
			}
		}
		_phones.push_back(newPhone);
	} else {
		// update an available phone clip's state if its sounds haven't been played yet
		if (phone->soundIndex < phone->sounds.size()) {
			// reset the call count
			phone->callCount = 0;

			// the first PhoneClip() call does not cause the phone clip to ring,
			// but the second call does. if a phone clip has multiple sounds and
			// one has been answered then its status changes to waiting so that
			// the next PhoneClip() call will make the next sound available.
			if (phone->status == kPhoneStatusWaiting) {
				phone->status = kPhoneStatusAvailable;
			} else if (phone->status == kPhoneStatusAnswered) {
				phone->status = kPhoneStatusWaiting;
			}
		}
	}
}

void PrivateEngine::initializePhoneOnDesktop() {
	// any phone clips that were missed, or left ringing, are available
	// unless they are phone clips that only occur once.
	for (PhoneList::iterator it = _phones.begin(); it != _phones.end(); ++it) {
		if (!it->once && (it->status == kPhoneStatusCalling || it->status == kPhoneStatusMissed)) {
			it->status = kPhoneStatusAvailable;
		}
	}
}

void PrivateEngine::checkPhoneCall() {
	if (_phoneArea.surf == nullptr) {
		return;
	}

	if (isSoundPlaying()) {
		return;
	}

	// any phone clips that were calling have been missed
	for (PhoneList::iterator it = _phones.begin(); it != _phones.end(); ++it) {
		if (it->status == kPhoneStatusCalling) {
			it->status = kPhoneStatusMissed;
		}
	}

	// get the next available phone clip
	PhoneInfo *phone = nullptr;
	for (PhoneList::iterator it = _phones.begin(); it != _phones.end(); ++it) {
		if (it->status == kPhoneStatusAvailable &&
			it->soundIndex < it->sounds.size() &&
			it->callCount < (it->once ? 1 : 3)) {
			phone = &(*it);
			break;
		}
	}
	if (phone == nullptr) {
		return;
	}

	phone->status = kPhoneStatusCalling;
	phone->callCount++;
	playForegroundSound(_phoneCallSound, _phonePrefix + "phone.wav");
}

bool PrivateEngine::cursorPhoneArea(Common::Point mousePos) {
	if (_phoneArea.surf == nullptr) {
		return false;
	}

	if (!isSoundPlaying(_phoneCallSound)) {
		return false;
	}

	if (inMask(_phoneArea.surf, mousePos)) {
		changeCursor(_phoneArea.cursor);
		return true;
	}

	return false;
}

bool PrivateEngine::selectPhoneArea(Common::Point mousePos) {
	if (_phoneArea.surf == nullptr) {
		return false;
	}

	if (!isSoundPlaying(_phoneCallSound)) {
		return false;
	}

	if (inMask(_phoneArea.surf, mousePos)) {
		// get phone clip to answer
		PhoneInfo *phone = nullptr;
		for (PhoneList::iterator it = _phones.begin(); it != _phones.end(); ++it) {
			if (it->status == kPhoneStatusCalling) {
				phone = &(*it);
				break;
			}
		}
		if (phone == nullptr) {
			return true;
		}

		// phone clip has been answered, select sound
		phone->status = kPhoneStatusAnswered;
		Common::String sound = _phonePrefix + phone->sounds[phone->soundIndex];
		phone->soundIndex++;

		// -100 indicates that the variable should be decremented
		Symbol *flag = maps.lookupVariable(&(phone->flagName));
		if (phone->flagValue == -100) {
			setSymbol(flag, flag->u.val - 1);
		} else {
			setSymbol(flag, phone->flagValue);
		}

		stopForegroundSounds(); // stop phone ringing
		playForegroundSound(sound);
		_nextSetting = getListenToPhoneSetting();
		changeCursor("default");
		return true;
	}
	return false;
}

void PrivateEngine::initializeWallSafeValue() {
	if (isDemo()) {
		return;
	}

	// initialize to a random value that is not the combination
	Private::Symbol *sym = maps.variables.getVal(getWallSafeValueVariable());
	int value;
	do {
		value = _rnd->getRandomNumber(999);
	} while (value == 426);
	sym->u.val = value;
}

bool PrivateEngine::selectSafeDigit(Common::Point mousePos) {
	if (_safeDigitArea[0].surf == nullptr)
		return false;

	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

	for (uint d = 0 ; d < 3; d ++)
		if (_safeDigitRect[d].contains(mousePos)) {
			incrementSafeDigit(d);
			_nextSetting = _safeDigitArea[d].nextSetting;
			return true;
		}

	return false;
}

void PrivateEngine::addSafeDigit(uint32 d, Common::Rect *rect) {

	MaskInfo m;
	_safeDigitRect[d] = *rect;
	int digitValue = getSafeDigit(d);
	m.surf = loadMask(Common::String::format(_safeNumberPath.c_str(), digitValue), _safeDigitRect[d].left, _safeDigitRect[d].top, true);
	m.cursor = getExitCursor();
	m.nextSetting = _currentSetting;
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	_safeDigitArea[d] = m;
}

int PrivateEngine::getSafeDigit(uint32 d) {
	assert(d < 3);

	Private::Symbol *sym = maps.variables.getVal(getWallSafeValueVariable());
	int value = sym->u.val;

	byte digits[3];
	digits[0] = value / 100;
	digits[1] = (value / 10) % 10;
	digits[2] = value % 10;

	return digits[d];
}

void PrivateEngine::incrementSafeDigit(uint32 d) {
	assert(d < 3);

	Private::Symbol *sym = maps.variables.getVal(getWallSafeValueVariable());
	int value = sym->u.val;

	byte digits[3];
	digits[0] = value / 100;
	digits[1] = (value / 10) % 10;
	digits[2] = value % 10;

	digits[d] = (digits[d] + 1) % 10;
	
	sym->u.val = (100 * digits[0]) + (10 * digits[1]) + digits[2];
}

bool PrivateEngine::selectLoadGame(Common::Point mousePos) {
	if (_loadGameMask.surf == nullptr)
		return false;

	if (inMask(_loadGameMask.surf, mousePos)) {
		loadGameDialog();
		return true;
	}
	return false;
}

bool PrivateEngine::selectSaveGame(Common::Point mousePos) {
	if (_saveGameMask.surf == nullptr)
		return false;

	if (inMask(_saveGameMask.surf, mousePos)) {
		saveGameDialog();
		return true;
	}
	return false;
}

bool PrivateEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

void PrivateEngine::restartGame() {
	debugC(1, kPrivateDebugFunction, "restartGame");

	Common::String alternateGameVariableName = getAlternateGameVariable();
	for (NameList::iterator it = maps.variableList.begin(); it != maps.variableList.end(); ++it) {
		Private::Symbol *sym = maps.variables.getVal(*it);
		if (*(sym->name) != alternateGameVariableName)
			sym->u.val = 0;
	}

	// Police Bust
	resetPoliceBust();

	// Diary
	for (NameList::iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		Private::Symbol *sym = maps.locations.getVal(*it);
		sym->u.val = 0;
	}
	inventory.clear();
	_toTake = false;
	_haveTakenItem = false;
	_dossiers.clear();
	_diaryPages.clear();

	// Sounds
	_AMRadio.clear();
	_policeRadio.clear();
	_phones.clear();
	_pausedBackgroundSoundName.clear();

	// Movies
	_repeatedMovieExit = "";
	_playedMovies.clear();
	destroyVideo();

	// Pause
	_pausedSetting = "";
	_pausedMovieName.clear();

	// VSPicture
	_nextVS = "";

	// Wall Safe
	initializeWallSafeValue();

	// Timer
	clearTimer();
}

Common::Error PrivateEngine::loadGameStream(Common::SeekableReadStream *stream) {
	// We don't want to continue with any sound or videos from a previous game
	stopSounds();
	destroyVideo();
	_pausedMovieName.clear();

	debugC(1, kPrivateDebugFunction, "loadGameStream");

	// Read and validate metadata header
	SavegameMetadata meta;
	if (!readSavegameMetadata(stream, meta)) {
		return Common::kReadingFailed;
	}

	// Log unexpected language or platform
	if (meta.language != _language) {
		warning("Save language %d different than game %d", meta.language, _language);
	}
	if (meta.platform != _platform) {
		warning("Save platform  %d different than game %d", meta.platform, _platform);
	}

	Common::Serializer s(stream, nullptr);
	int val;

	for (NameList::iterator it = maps.variableList.begin(); it != maps.variableList.end(); ++it) {
		s.syncAsUint32LE(val);
		Private::Symbol *sym = maps.variables.getVal(*it);
		sym->u.val = val;
	}

	// Diary
	for (NameList::iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		s.syncAsUint32LE(val);
		Private::Symbol *sym = maps.locations.getVal(*it);
		sym->u.val = val;
	}

	// Inventory
	inventory.clear();
	uint32 size = stream->readUint32LE();
	for (uint32 i = 0; i < size; ++i) {
		InventoryItem inv;
		inv.diaryImage = stream->readString();
		inv.flag = stream->readString();
		inventory.push_back(inv);
	}
	_toTake = (stream->readByte() == 1);
	_haveTakenItem = (stream->readByte() == 1);

	// Diary pages
	_diaryPages.clear();
	uint32 diaryPagesSize = stream->readUint32LE();
	for (uint32 i = 0; i < diaryPagesSize; i++) {
		DiaryPage diaryPage;
		diaryPage.locationName = stream->readString();
		diaryPage.locationID = stream->readUint32LE();

		uint32 memoriesSize = stream->readUint32LE();
		for (uint32 j = 0; j < memoriesSize; j++) {
			MemoryInfo memory;
			memory.image = stream->readString();
			memory.movie = stream->readString();
			diaryPage.memories.push_back(memory);
		}

		_diaryPages.push_back(diaryPage);
	}

	// Dossiers
	_dossiers.clear();
	size = stream->readUint32LE();
	for (uint32 i = 0; i < size; ++i) {
		Common::String page1 = stream->readString();
		Common::String page2 = stream->readString();
		addDossier(page1, page2);
	}

	// Police Bust
	_policeBustEnabled = (stream->readByte() == 1);
	_policeSirenPlayed = (stream->readByte() == 1);
	_numberOfClicks = stream->readSint32LE();
	_numberClicksAfterSiren = stream->readSint32LE();
	_policeBustMovieIndex = stream->readSint32LE();
	_policeBustMovie = stream->readString();
	_policeBustPreviousSetting = stream->readString();

	// Radios
	Radio *radios[] = { &_AMRadio, &_policeRadio };
	for (uint r = 0; r < ARRAYSIZE(radios); r++) {
		Radio *radio = radios[r];
		radio->clear();

		size = stream->readUint32LE();
		for (uint32 i = 0; i < size; ++i) {
			RadioClip clip;
			clip.name = stream->readString();
			clip.played = (stream->readByte() == 1);
			clip.priority = stream->readSint32LE();
			clip.disabledPriority1 = stream->readSint32LE();
			clip.exactPriorityMatch1 = (stream->readByte() == 1);
			clip.disabledPriority2 = stream->readSint32LE();
			clip.exactPriorityMatch2 = (stream->readByte() == 1);
			clip.flagName = stream->readString();
			clip.flagValue = stream->readSint32LE();
			radio->clips.push_back(clip);
		}
		for (uint i = 0; i < ARRAYSIZE(radio->channels); i++) {
			radio->channels[i] = stream->readSint32LE();
		}
	}

	size = stream->readUint32LE();
	_phones.clear();
	for (uint32 j = 0; j < size; ++j) {
		PhoneInfo p;
		p.name = stream->readString();
		p.once = (stream->readByte() == 1);
		p.startIndex = stream->readSint32LE();
		p.endIndex = stream->readSint32LE();
		p.flagName = stream->readString();
		p.flagValue = stream->readSint32LE();
		p.status = (PhoneStatus)stream->readByte();
		p.callCount = stream->readSint32LE();
		p.soundIndex = stream->readUint32LE();
		uint32 phoneSoundsSize = stream->readUint32LE();
		for (uint32 i = 0; i < phoneSoundsSize; i++) {
			p.sounds.push_back(stream->readString());
		}
		_phones.push_back(p);
	}

	// Played media
	_repeatedMovieExit = stream->readString();
	_playedMovies.clear();
	size = stream->readUint32LE();
	for (uint32 i = 0; i < size; ++i) {
		_playedMovies.setVal(stream->readString(), true);
	}

	// VSPicture
	_nextVS = stream->readString();

	// Paused setting
	_pausedSetting = stream->readString();

	// Restore a movie that was playing
	_currentMovie = stream->readString();
	/* int currentTime = */ stream->readUint32LE();

	if (!_currentMovie.empty()) {
		_videoDecoder = new Video::SmackerDecoder();
		playVideo(_currentMovie);
		_videoDecoder->pauseVideo(true);
		// TODO: implement seek
	}

	if (_pausedSetting.empty())
		_nextSetting = getMainDesktopSetting();
	else
		_nextSetting = getPauseMovieSetting();

	// Sounds
	if (meta.version >= 4) {
		_pausedBackgroundSoundName = stream->readString();
	} else {
		_pausedBackgroundSoundName.clear();
	}

	return Common::kNoError;
}

Common::Error PrivateEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	debugC(1, kPrivateDebugFunction, "saveGameStream(%d)", isAutosave);
	if (isAutosave)
		return Common::kNoError;

	// Metadata
	SavegameMetadata meta;
	meta.version = kCurrentSavegameVersion;
	meta.language = _language;
	meta.platform = _platform;
	writeSavegameMetadata(stream, meta);

	// Variables
	for (NameList::const_iterator it = maps.variableList.begin(); it != maps.variableList.end(); ++it) {
		const Private::Symbol *sym = maps.variables.getVal(*it);
		stream->writeUint32LE(sym->u.val);
	}

	// Diary
	for (NameList::const_iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		const Private::Symbol *sym = maps.locations.getVal(*it);
		stream->writeUint32LE(sym->u.val);
	}

	stream->writeUint32LE(inventory.size());
	for (InvList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		stream->writeString(it->diaryImage);
		stream->writeByte(0);
		stream->writeString(it->flag);
		stream->writeByte(0);
	}
	stream->writeByte(_toTake ? 1 : 0);
	stream->writeByte(_haveTakenItem ? 1 : 0);

	stream->writeUint32LE(_diaryPages.size());
	for (uint i = 0; i < _diaryPages.size(); i++) {
		stream->writeString(_diaryPages[i].locationName);
		stream->writeByte(0);

		stream->writeUint32LE(_diaryPages[i].locationID);
		stream->writeUint32LE(_diaryPages[i].memories.size());

		for (uint j = 0; j < _diaryPages[i].memories.size(); j++) {
			stream->writeString(_diaryPages[i].memories[j].image);
			stream->writeByte(0);
			stream->writeString(_diaryPages[i].memories[j].movie);
			stream->writeByte(0);
		}
	}

	// Dossiers
	stream->writeUint32LE(_dossiers.size());
	for (DossierArray::const_iterator it = _dossiers.begin(); it != _dossiers.end(); ++it) {
		stream->writeString(it->page1.c_str());
		stream->writeByte(0);

		if (!it->page2.empty())
			stream->writeString(it->page2.c_str());
		stream->writeByte(0);
	}

	// Police Bust
	stream->writeByte(_policeBustEnabled ? 1 : 0);
	stream->writeByte(_policeSirenPlayed ? 1 : 0);
	stream->writeSint32LE(_numberOfClicks);
	stream->writeSint32LE(_numberClicksAfterSiren);
	stream->writeSint32LE(_policeBustMovieIndex);
	stream->writeString(_policeBustMovie);
	stream->writeByte(0);
	stream->writeString(_policeBustPreviousSetting);
	stream->writeByte(0);

	// Radios
	Radio *radios[] = { &_AMRadio, &_policeRadio };
	for (uint r = 0; r < ARRAYSIZE(radios); r++) {
		Radio *radio = radios[r];
		stream->writeUint32LE(radio->clips.size());
		for (uint i = 0; i < radio->clips.size(); i++) {
			RadioClip &clip = radio->clips[i];
			stream->writeString(clip.name);
			stream->writeByte(0);
			stream->writeByte(clip.played ? 1 : 0);
			stream->writeSint32LE(clip.priority);
			stream->writeSint32LE(clip.disabledPriority1);
			stream->writeByte(clip.exactPriorityMatch1 ? 1 : 0);
			stream->writeSint32LE(clip.disabledPriority2);
			stream->writeByte(clip.exactPriorityMatch2 ? 1 : 0);
			stream->writeString(clip.flagName);
			stream->writeByte(0);
			stream->writeSint32LE(clip.flagValue);
		}
		for (uint i = 0; i < ARRAYSIZE(radio->channels); i++) {
			stream->writeSint32LE(radio->channels[i]);
		}
	}

	// Phone
	stream->writeUint32LE(_phones.size());
	for (PhoneList::const_iterator it = _phones.begin(); it != _phones.end(); ++it) {
		stream->writeString(it->name);
		stream->writeByte(0);
		stream->writeByte(it->once ? 1 : 0);
		stream->writeSint32LE(it->startIndex);
		stream->writeSint32LE(it->endIndex);
		stream->writeString(it->flagName);
		stream->writeByte(0);
		stream->writeSint32LE(it->flagValue);
		stream->writeByte(it->status);
		stream->writeSint32LE(it->callCount);
		stream->writeUint32LE(it->soundIndex);
		stream->writeUint32LE(it->sounds.size());
		for (uint i = 0; i < it->sounds.size(); i++) {
			stream->writeString(it->sounds[i]);
			stream->writeByte(0);
		}
	}

	// Played media
	stream->writeString(_repeatedMovieExit);
	stream->writeByte(0);

	stream->writeUint32LE(_playedMovies.size());
	for (PlayedMediaTable::const_iterator it = _playedMovies.begin(); it != _playedMovies.end(); ++it) {
		stream->writeString(it->_key);
		stream->writeByte(0);
	}

	// VSPicture
	stream->writeString(_nextVS);
	stream->writeByte(0);

	// In case the game was saved during a pause
	stream->writeString(_pausedSetting);
	stream->writeByte(0);

	// If we were playing a movie
	stream->writeString(_currentMovie);
	stream->writeByte(0);
	if (_videoDecoder)
		stream->writeUint32LE(_videoDecoder->getCurFrame());
	else
		stream->writeUint32LE(0);

	// Sounds
	stream->writeString(_pausedBackgroundSoundName);
	stream->writeByte(0);

	return Common::kNoError;
}

Common::Path PrivateEngine::convertPath(const Common::String &name) {
	Common::String path(name);
	Common::String s1("\\");
	Common::String s2("/");

	while (path.contains(s1))
		Common::replace(path, s1, s2);

	s1 = Common::String("\"");
	s2 = Common::String("");

	Common::replace(path, s1, s2);
	Common::replace(path, s1, s2);

	path.toLowercase();
	return Common::Path(path);
}

void PrivateEngine::playBackgroundSound(const Common::String &name) {
	playSound(_bgSound, name, true);
}

void PrivateEngine::playForegroundSound(const Common::String &name) {
	// stop sound if already playing. for example, the wall safe alarm.
	for (uint i = 0; i < ARRAYSIZE(_fgSounds); i++) {
		if (_fgSounds[i].name == name) {
			if (isSoundPlaying(_fgSounds[i])) {
				stopSound(_fgSounds[i]);
				break;
			}
		}
	}

	// play using the first available sound
	for (uint i = 0; i < ARRAYSIZE(_fgSounds); i++) {
		if (!isSoundPlaying(_fgSounds[i])) {
			playSound(_fgSounds[i], name, false);
			break;
		}
	}
}

void PrivateEngine::playForegroundSound(Sound &sound, const Common::String &name) {
	playSound(sound, name, false);
}

void PrivateEngine::playSound(Sound &sound, const Common::String &name, bool loop) {
	sound.name = name;

	Common::Path path = convertPath(name);
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(path);

	if (file == nullptr) {
		error("unable to find sound file %s", path.toString().c_str());
	}

	Audio::LoopingAudioStream *stream = new Audio::LoopingAudioStream(Audio::makeWAVStream(file, DisposeAfterUse::YES), loop ? 0 : 1);
	
	_mixer->stopHandle(sound.handle);
	_mixer->playStream(Audio::Mixer::kSFXSoundType, &sound.handle, stream, -1, Audio::Mixer::kMaxChannelVolume);

	loadSubtitles(path, kSubtitleAudio, &sound);
}

void PrivateEngine::stopForegroundSounds() {
	for (uint i = 0; i < ARRAYSIZE(_fgSounds); i++) {
		stopSound(_fgSounds[i]);
	}
	stopSound(_phoneCallSound);
	stopSound(_AMRadioSound);
	stopSound(_policeRadioSound);
	stopSound(_takeLeaveSound);
}

void PrivateEngine::stopSounds() {
	stopSound(_bgSound);
	stopForegroundSounds();
}

void PrivateEngine::stopSound(Sound &sound) {
	_mixer->stopHandle(sound.handle);
	if (_voiceSlot.handle == sound.handle && _voiceSlot.subs) {
		delete _voiceSlot.subs;
		_voiceSlot.subs = nullptr;
	}
	if (_sfxSlot.handle == sound.handle && _sfxSlot.subs) {
		delete _sfxSlot.subs;
		_sfxSlot.subs = nullptr;
	}
	sound.name.clear();
}

bool PrivateEngine::isSoundPlaying() {
	return _mixer->isSoundIDActive(-1);
}

bool PrivateEngine::isSoundPlaying(Sound &sound) {
	return _mixer->isSoundHandleActive(sound.handle);
}

void PrivateEngine::waitForSoundsToStop() {
	while (isSoundPlaying()) {
		// since this is a blocking wait loop, the main engine loop in run() is not called until this loop finishes
		// we must manually update and draw subtitles here otherwise sounds
		// played via fSyncSound will play audio but show no subtitles.
		updateSubtitles();
		if (consumeEvents()) {
			stopSounds();
			return;
		}
	}

	uint32 i = 100;
	while (i--) { // one second extra
		if (consumeEvents()) {
			stopSounds();
			return;
		}
	}
}

// returns true if interrupted by user or engine quitting
bool PrivateEngine::consumeEvents() {
	if (shouldQuit()) {
		return true;
	}

	Common::Event event;
	while (_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_RETURN_TO_LAUNCHER:
		case Common::EVENT_QUIT:
			return true;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (event.customType == kActionSkip) {
				return true;
			}
			break;

		default:
			break;;
		}
	}

	_system->updateScreen();
	_system->delayMillis(10);
	return false;
}

void PrivateEngine::adjustSubtitleSize() {
	debugC(1, kPrivateDebugFunction, "%s()", __FUNCTION__);
	if (!_videoSubtitles && !_voiceSlot.subs && !_sfxSlot.subs) return;
	// calculate layout first then apply to both active sounds and video subtitled sound
	// Subtitle positioning constants (as percentages of screen height)
	const int HORIZONTAL_MARGIN = 20;
	const float BOTTOM_MARGIN_PERCENT = 0.009f;  // ~20px at 2160p
	const float MAIN_MENU_HEIGHT_PERCENT = 0.093f;  // ~200px at 2160p
	const float ALTERNATE_MODE_HEIGHT_PERCENT = 0.102f;  // ~220px at 2160p
	const float DEFAULT_HEIGHT_PERCENT = 0.074f;  // ~160px at 2160p

	// Font sizing constants (as percentage of screen height)
	const int MIN_FONT_SIZE = 8;
	const float BASE_FONT_SIZE_PERCENT = 0.023f;  // ~50px at 2160p

	int16 h = _system->getOverlayHeight();
	int16 w = _system->getOverlayWidth();

	int bottomMargin = int(h * BOTTOM_MARGIN_PERCENT);
	int topOffset = 0;

	// If we are in the main menu, we need to adjust the position of the subtitles
	if (_mode == 0) {
		topOffset = int(h * MAIN_MENU_HEIGHT_PERCENT);
	} else if (_mode == -1) {
		topOffset = int(h * ALTERNATE_MODE_HEIGHT_PERCENT);
	} else {
		topOffset = int(h * DEFAULT_HEIGHT_PERCENT);
	}
	Common::Rect rect(HORIZONTAL_MARGIN, h - topOffset, w - HORIZONTAL_MARGIN, h - bottomMargin);
	int fontSize = MAX(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));

	// apply to video subtitles
	if (_videoSubtitles) {
		_videoSubtitles->setBBox(rect);
		_videoSubtitles->setColor(0xff, 0xff, 0x80);
		_videoSubtitles->setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
		_videoSubtitles->setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
	}

	// apply to all active audio subtitles
	if (_voiceSlot.subs) {
		_voiceSlot.subs->setBBox(rect);
		_voiceSlot.subs->setColor(0xff, 0xff, 0x80);
		_voiceSlot.subs->setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
		_voiceSlot.subs->setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
	}
	if (_sfxSlot.subs) {
		_sfxSlot.subs->setBBox(rect);
		_sfxSlot.subs->setColor(0xff, 0xff, 0x80);
		_sfxSlot.subs->setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
		_sfxSlot.subs->setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
	}
}

Common::Path PrivateEngine::getSubtitlePath(const Common::String &soundName) {
	// call convertPath to fix slashes, make lowercase etc.
	Common::Path path = convertPath(soundName);

	// add extension and replace '/' with '_' (audio/file -> audio_file)
	Common::String subPathStr = path.toString() + ".srt";
	subPathStr.replace('/', '_');

	// get language code
	Common::String language(Common::getLanguageCode(_language));
	if (language == "us")
		language = "en";

	// construct full path: subtitles/language/subPathStr
	Common::Path subPath = "subtitles";
	subPath = subPath.appendComponent(language);
	subPath = subPath.appendComponent(subPathStr);

	return subPath;
}

void PrivateEngine::loadSubtitles(const Common::Path &path, SubtitleType type, Sound *sound) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, path.toString().c_str());
	if (!_useSubtitles)
		return;

	Common::Path subPath = getSubtitlePath(path.toString());
	debugC(1, kPrivateDebugFunction, "Loading subtitles from %s", subPath.toString().c_str());

	// instantiate and load on heap once
	Video::Subtitles *newSub = new Video::Subtitles();
	newSub->loadSRTFile(subPath);

	// if the subtitle failed loading we should return
	if (!newSub->isLoaded()) {
		delete newSub;
		return;
	}
	if (type == kSubtitleVideo) {
		if (_videoSubtitles)
			delete _videoSubtitles;
		_videoSubtitles = newSub;
	}
	else if (type == kSubtitleAudio) {
		if (!sound) {
			warning("PrivateEngine::loadSubtitles: Audio type requested but no Sound provided");
			delete newSub;
			return;
		}

		bool isSfx = isSfxSubtitle(newSub);

		if (isSfx) {
			// if voice is currently playing, ignore incoming sfx
			if (isSlotActive(_voiceSlot)) {
				delete newSub;
				return;
			}

			// load sfx (overwrites any previous sfx)
			if (_sfxSlot.subs)
				delete _sfxSlot.subs;

			_sfxSlot.handle = sound->handle;
			_sfxSlot.subs = newSub;

		} else {
			// voice always loads and takes priority
			if (_voiceSlot.subs)
				delete _voiceSlot.subs;

			_voiceSlot.handle = sound->handle;
			_voiceSlot.subs = newSub;
		}
	}
	// we skip clearing the overlay because updateSubtitle() handles it in the main loop
	// if we clear here as well then minor flickering occurs
	adjustSubtitleSize();
}

void PrivateEngine::updateSubtitles() {
	if (!_useSubtitles)
		return;

	// remove subtitles for sounds that finished playing
	if (_voiceSlot.subs && !_mixer->isSoundHandleActive(_voiceSlot.handle)) {
		delete _voiceSlot.subs;
		_voiceSlot.subs = nullptr;
	}

	if (_sfxSlot.subs && !_mixer->isSoundHandleActive(_sfxSlot.handle)) {
		delete _sfxSlot.subs;
		_sfxSlot.subs = nullptr;
	}

	if (_voiceSlot.subs) {
		// if voice is active draw voice only
		uint32 time = _mixer->getElapsedTime(_voiceSlot.handle).msecs();
		_voiceSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
	} else if (_sfxSlot.subs) {
		// if voice is empty draw sfx
		uint32 time = _mixer->getElapsedTime(_sfxSlot.handle).msecs();
		_sfxSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
	}
}

void PrivateEngine::destroySubtitles() {
	if (_voiceSlot.subs) {
		delete _voiceSlot.subs;
		_voiceSlot.subs = nullptr;
	}
	if (_sfxSlot.subs) {
		delete _sfxSlot.subs;
		_sfxSlot.subs = nullptr;
	}
	if (_videoSubtitles) {
		delete _videoSubtitles;
		_videoSubtitles = nullptr;
	}
	_system->hideOverlay();
}

void PrivateEngine::playVideo(const Common::String &name) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());

	Common::Path path = convertPath(name);
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(path);

	if (!file)
		error("unable to find video file %s", path.toString().c_str());

	if (!_videoDecoder->loadStream(file))
		error("unable to load video %s", path.toString().c_str());

	loadSubtitles(path, kSubtitleVideo);
	_videoDecoder->start();

	// set the view screen based on the video, unless playing from diary
	if (_currentSetting != getDiaryMiddleSetting()) {
		Common::String videoViewScreen = getVideoViewScreen(name);
		if (!videoViewScreen.empty()) {
			_nextVS = videoViewScreen;
		}
	}
}

Common::String PrivateEngine::getVideoViewScreen(Common::String video) {
	video = convertPath(video).toString();

	// find the separator
	const char *separators[] = { "/animatio/", "/" };
	size_t separatorPos = Common::String::npos;
	size_t separatorLength = Common::String::npos;
	for (uint i = 0; i < ARRAYSIZE(separators); i++) {
		separatorPos = video.find(separators[i]);
		if (separatorPos != Common::String::npos) {
			separatorLength = strlen(separators[i]);
			break;
		}
	}
	if (separatorPos == Common::String::npos) {
		return "";
	}

	// find the video suffix. these suffixes are from the executable.
	size_t suffixPos = Common::String::npos;
	const char *suffixes[] = { "ys.smk", "xs.smk", "a.smk", "s.smk", ".smk" };
	for (uint i = 0; i < ARRAYSIZE(suffixes); i++) {
		if (video.hasSuffix(suffixes[i])) {
			suffixPos = video.size() - strlen(suffixes[i]);
			break;
		}
	}
	if (suffixPos == Common::String::npos) {
		return "";
	}

	// build the view screen picture name
	Common::String picture = Common::String::format(
		"\"inface/views/%s/%s.bmp\"",
		video.substr(0, separatorPos).c_str(),
		video.substr(separatorPos + separatorLength, suffixPos - (separatorPos + separatorLength)).c_str());

	// not every video has a picture
	if (!Common::File::exists(convertPath(picture))) {
		return "";
	}

	return picture;
}

void PrivateEngine::skipVideo() {
	if (_videoDecoder == nullptr || _videoDecoder->isPaused()) {
		return;
	}

	delete _videoDecoder;
	_videoDecoder = nullptr;
	destroySubtitles();
	_currentMovie = "";
}

void PrivateEngine::destroyVideo() {
	if (_videoDecoder != _pausedVideo) {
		delete _pausedVideo;
	}
	delete _videoDecoder;
	_videoDecoder = nullptr;
	_pausedVideo = nullptr;
	destroySubtitles();
}

Graphics::Surface *PrivateEngine::decodeImage(const Common::String &name, byte **palette, bool *isNewPalette) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());
	Common::Path path = convertPath(name);
	Common::ScopedPtr<Common::SeekableReadStream> file(Common::MacResManager::openFileOrDataFork(path));
	if (!file)
		error("unable to load image %s", name.c_str());

	_image->loadStream(*file);
	const Graphics::Surface *oldImage = _image->getSurface();
	Graphics::Surface *newImage;

	const byte *oldPalette = _image->getPalette().data();
	byte *currentPalette;

	uint16 ncolors = _image->getPalette().size();
	if (ncolors < 256 || path.toString('/').hasPrefix("intro")) { // For some reason, requires color remapping
		currentPalette = (byte *) malloc(3*256);
		drawScreen();
		_system->getPaletteManager()->grabPalette(currentPalette, 0, 256);
		newImage = oldImage->convertTo(_pixelFormat, currentPalette);
		remapImage(ncolors, oldImage, oldPalette, newImage, currentPalette);
		*palette = currentPalette;
		*isNewPalette = true;
	} else {
		currentPalette = const_cast<byte *>(oldPalette);
		newImage = oldImage->convertTo(_pixelFormat, currentPalette);
		*palette = currentPalette;
		*isNewPalette = false;
	}

	// Most images store the transparent color (green) in color 250, except for
	// those in Mavis' apartment. Our engine assumes that all images share the
	// the same transparent color number (250), so if this image stores it in
	// a different palette entry then swap it with 250.
	uint32 maskTransparentColor = findMaskTransparentColor(currentPalette, _transparentColor);
	if (maskTransparentColor != _transparentColor) {
		swapImageColors(newImage, currentPalette, maskTransparentColor, _transparentColor);
	}

	return newImage;
}

void PrivateEngine::remapImage(uint16 ncolors, const Graphics::Surface *oldImage, const byte *oldPalette, Graphics::Surface *newImage, const byte *currentPalette) {
	debugC(1, kPrivateDebugFunction, "%s(..)", __FUNCTION__);
	byte paletteMap[256];
	// Run through every color in old palette
	for (int i = 0; i != ncolors; ++i) {
		byte r0 = oldPalette[3 * i + 0];
		byte g0 = oldPalette[3 * i + 1];
		byte b0 = oldPalette[3 * i + 2];

		// Find the closest color in current palette
		int closest_distance = 10000;
		int closest_j = 0;
		for (int j = 0; j != 256; ++j) {
			byte r1 = currentPalette[3 * j + 0];
			byte g1 = currentPalette[3 * j + 1];
			byte b1 = currentPalette[3 * j + 2];

			int distance = (MAX(r0, r1) - MIN(r0, r1))
						+ (MAX(g0, g1) - MIN(g0, g1))
						+ (MAX(b0, b1) - MIN(b0, b1));

			if (distance < closest_distance) {
				closest_distance = distance;
				closest_j = j;
			}
		}
		paletteMap[i] = closest_j;
	}

	const byte *src = (const byte*) oldImage->getPixels();
	byte *dst = (byte *) newImage->getPixels();

	int pitch = oldImage->pitch;
	for (int y = 0; y != oldImage->h; ++y) {
		for (int x = 0; x != oldImage->w; ++x) {
			dst[y * pitch + x] = paletteMap[src[y * pitch + x]];
		}
	}
}

uint32 PrivateEngine::findMaskTransparentColor(const byte *palette, uint32 defaultColor) {
	// Green is used for the transparent color in masks. It is not always
	// the same shade of green, and it is not always the same palette
	// index in the bitmap image. It appears that the original searched
	// each bitmap's palette for the nearest match to RGB 00 FF 00.
	// Some masks use 00 FC 00. Green is usually color 250 in masks,
	// but it is color 2 in the masks in Mavis' apartment.
	uint32 transparentColor = defaultColor;
	for (uint32 c = 0; c < 256; c++) {
		byte r = palette[3 * c + 0];
		byte g = palette[3 * c + 1];
		byte b = palette[3 * c + 2];
		if (r == 0 && b == 0) {
			if (g == 0xff) {
				// exact match, stop scanning
				transparentColor = c;
				break;
			}
			if (g == 0xfc) {
				// almost green, keep scanning
				transparentColor = c;
			}
		}
	}
	return transparentColor;
}

// swaps two colors in an image
void PrivateEngine::swapImageColors(Graphics::Surface *image, byte *palette, uint32 a, uint32 b) {
	SWAP(palette[3 * a + 0], palette[3 * b + 0]);
	SWAP(palette[3 * a + 1], palette[3 * b + 1]);
	SWAP(palette[3 * a + 2], palette[3 * b + 2]);

	for (int y = 0; y < image->h; y++) {
		for (int x = 0; x < image->w; x++) {
			uint32 pixel = image->getPixel(x, y);
			if (pixel == a) {
				image->setPixel(x, y, b);
			} else if (pixel == b) {
				image->setPixel(x, y, a);
			}
		}
	}
}

void PrivateEngine::loadImage(const Common::String &name, int x, int y) {
	debugC(1, kPrivateDebugFunction, "%s(%s,%d,%d)", __FUNCTION__, name.c_str(), x, y);
	byte *palette;
	bool isNewPalette;
	Graphics::Surface *surf = decodeImage(name, &palette, &isNewPalette);
	_compositeSurface->setPalette(palette, 0, 256);
	_compositeSurface->setTransparentColor(_transparentColor);
	_compositeSurface->transBlitFrom(*surf, _origin + Common::Point(x, y), _transparentColor);
	surf->free();
	delete surf;
	_image->destroy();
	if (isNewPalette) {
		free(palette);
	}
}

void PrivateEngine::fillRect(uint32 color, Common::Rect rect) {
	debugC(1, kPrivateDebugFunction, "%s(%d,..)", __FUNCTION__, color);
	rect.translate(_origin.x, _origin.y);
	_compositeSurface->fillRect(rect, color);
}

void PrivateEngine::drawScreenFrame(const byte *newPalette) {
	debugC(1, kPrivateDebugFunction, "%s(..)", __FUNCTION__);
	remapImage(256, _frameImage, _framePalette, _mframeImage, newPalette);
	_system->copyRectToScreen(_mframeImage->getPixels(), _mframeImage->pitch, 0, 0, _screenW, _screenH);
}

void PrivateEngine::loadMaskAndInfo(MaskInfo *m, const Common::String &name, int x, int y, bool drawn) {
	m->surf = new Graphics::Surface();
	m->surf->create(_screenW, _screenH, _pixelFormat);
	m->surf->fillRect(_screenRect, _transparentColor);
	byte *palette;
	bool isNewPalette;
	Graphics::Surface *csurf = decodeImage(name, &palette, &isNewPalette);

	uint32 hdiff = 0;
	uint32 wdiff = 0;

	if (x + csurf->h > _screenH)
		hdiff = x + csurf->h - _screenH;
	if (y + csurf->w > _screenW)
		wdiff = y + csurf->w - _screenW;

	Common::Rect crect(csurf->w - wdiff, csurf->h - hdiff);
	m->surf->copyRectToSurface(*csurf, x, y, crect);
	m->box = Common::Rect(x, y, x + csurf->w, y + csurf->h);

	if (drawn) {
		_compositeSurface->setPalette(palette, 0, 256);
		_compositeSurface->setTransparentColor(_transparentColor);
		drawMask(m->surf);
	}

	csurf->free();
	delete csurf;
	_image->destroy();

	if (isNewPalette) {
		free(palette);
	}
}

Graphics::Surface *PrivateEngine::loadMask(const Common::String &name, int x, int y, bool drawn) {
	debugC(1, kPrivateDebugFunction, "%s(%s,%d,%d,%d)", __FUNCTION__, name.c_str(), x, y, drawn);
	if (_shouldHighlightMasks && name.contains("\\decision\\"))
		_highlightMasks = true;

	MaskInfo m;
	loadMaskAndInfo(&m, name, x, y, drawn);
	return m.surf;
}

void PrivateEngine::drawMask(Graphics::Surface *surf) {
	_compositeSurface->transBlitFrom(*surf, _origin, _transparentColor);
}

void drawCircle(Graphics::ManagedSurface *surface, int x, int y, int radius, int color) {
	int cx = 0;
	int cy = radius;
	int df = 1 - radius;
	int d_e = 3;
	int d_se = -2 * radius + 5;

	do {
		surface->setPixel(x + cx, y + cy, color);
		surface->setPixel(x - cx, y + cy, color);
		surface->setPixel(x + cx, y - cy, color);
		surface->setPixel(x - cx, y - cy, color);
		surface->setPixel(x + cy, y + cx, color);
		surface->setPixel(x - cy, y + cx, color);
		surface->setPixel(x + cy, y - cx, color);
		surface->setPixel(x - cy, y - cx, color);

		if (df < 0) {
			df += d_e;
			d_e += 2;
			d_se += 2;
		} else {
			df += d_se;
			d_e += 2;
			d_se += 4;
			cy--;
		}
		cx++;
	} while (cx <= cy);

	for (int i = -radius; i <= radius; i++) {
		surface->setPixel(x + i, y, color);
		surface->setPixel(x, y + i, color);
	}
}

void PrivateEngine::drawScreen() {
	if (_videoDecoder && !_videoDecoder->isPaused()) {
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Common::Point center((_screenW - _videoDecoder->getWidth()) / 2, (_screenH - _videoDecoder->getHeight()) / 2);

		if (_needToDrawScreenFrame && _videoDecoder->getCurFrame() >= 0) {
			const byte *videoPalette = _videoDecoder->getPalette();
			_system->getPaletteManager()->setPalette(videoPalette, 0, 256);
			drawScreenFrame(videoPalette);
			_needToDrawScreenFrame = false;
		} else if (_videoDecoder->hasDirtyPalette()) {
			const byte *videoPalette = _videoDecoder->getPalette();
			_system->getPaletteManager()->setPalette(videoPalette, 0, 256);

			if (_mode == 1) {
				drawScreenFrame(videoPalette);
			}
		}

		// No use of _compositeSurface, we write the frame directly to the screen in the expected position
		_system->copyRectToScreen(frame->getPixels(), frame->pitch, center.x, center.y, frame->w, frame->h);
	} else {
		byte newPalette[256 * 3];
		_compositeSurface->grabPalette(newPalette, 0, 256);
		_system->getPaletteManager()->setPalette(newPalette, 0, 256);

		if (_mode == 1) {
			// We can reuse newPalette
			_system->getPaletteManager()->grabPalette((byte *) &newPalette, 0, 256);
			drawScreenFrame((byte *) &newPalette);
		}

		if (_highlightMasks) {
			byte redIndex = 0;
			int min_dist = 1000 * 1000;
			for (int i = 0; i < 256; ++i) {
				int r = newPalette[i * 3 + 0];
				int g = newPalette[i * 3 + 1];
				int b = newPalette[i * 3 + 2];
				int dist = (255 - r) * (255 - r) + g * g + b * b;
				if (dist < min_dist) {
					min_dist = dist;
					redIndex = i;
				}
			}

			for (MaskList::const_iterator it = _masks.begin(); it != _masks.end(); ++it) {
				const MaskInfo &m = *it;
				if (m.surf == nullptr) continue;

				long sumX = 0;
				long sumY = 0;
				int count = 0;

				for (int sx = 0; sx < m.surf->w; ++sx) {
					for (int sy = 0; sy < m.surf->h; ++sy) {
						if (m.surf->getPixel(sx, sy) != _transparentColor) {
							sumX += sx;
							sumY += sy;
							count++;
						}
					}
				}

				if (count > 0) {
					int centerX = sumX / count;
					int centerY = sumY / count;

					drawCircle(_compositeSurface, centerX + _origin.x, centerY + _origin.y, 7, redIndex);
				}
			}
		}

		Common::Rect w(_origin.x, _origin.y, _screenW - _origin.x, _screenH - _origin.y);
		Graphics::Surface sa = _compositeSurface->getSubArea(w);
		_system->copyRectToScreen(sa.getPixels(), sa.pitch, _origin.x, _origin.y, sa.w, sa.h);
	}

	// audio subtitles are handled in updateSubtitles() in the main loop so only draw video subtitles here
	if (_videoSubtitles && _videoDecoder && !_videoDecoder->isPaused())
		_videoSubtitles->drawSubtitle(_videoDecoder->getTime(), false, _sfxSubtitles);

	_system->updateScreen();
}

void PrivateEngine::pauseEngineIntern(bool pause) {
	Engine::pauseEngineIntern(pause);

	// If we are unpausing (returning from quit dialog, etc.)
	if (!pause) {
		// reset the overlay
		_system->showOverlay(false);
		_system->clearOverlay();

		// force draw the subtitle once
		// the screen was likely wiped by the dialog/menu
		// to account for the subtitle which was already rendered and we wiped the screen before it finished we must
		// force the subtitle system to ignore its cache and redraw the text.
		// calling adjustSubtitleSize() makes the next drawSubtitle call perform a full redraw
		// automatically, so we don't need to pass 'true'.
		adjustSubtitleSize();
		if (_videoDecoder && _videoSubtitles)
			_videoSubtitles->drawSubtitle(_videoDecoder->getTime(), false, _sfxSubtitles);

		// draw all remaining active subtitles
		if (isSlotActive(_voiceSlot)) {
			uint32 time = _mixer->getElapsedTime(_voiceSlot.handle).msecs();
			_voiceSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
		} else if (isSlotActive(_sfxSlot)) {
			uint32 time = _mixer->getElapsedTime(_sfxSlot.handle).msecs();
			_sfxSlot.subs->drawSubtitle(time, false, _sfxSubtitles);
		}
	}
}

bool PrivateEngine::getRandomBool(uint p) {
	uint r = _rnd->getRandomNumber(100);
	return (r <= p);
}

Common::String PrivateEngine::getPaperShuffleSound() {
	uint r = _rnd->getRandomNumber(6);
	return Common::String::format("%sglsfx0%d.wav", _globalAudioPath.c_str(), kPaperShuffleSound[r]);
}

Common::String PrivateEngine::getTakeSound() {
	if (isDemo())
		return (_globalAudioPath + "mvo007.wav");

	// Only the first four sounds are available when taking the first item.
	const char *sounds[7] = {
		"mvo007.wav",
		"mvo003.wav",
		"took1.wav",
		"took2.wav",
		"took3.wav",
		"took4.wav",
		"took5.wav"
	};
	uint r = _rnd->getRandomNumber(_haveTakenItem ? 6 : 3);
	return _globalAudioPath + sounds[r];
}

Common::String PrivateEngine::getTakeLeaveSound() {
	uint r = _rnd->getRandomNumber(1);
	if (r == 0) {
		return (_globalAudioPath + "mvo001.wav");
	} else {
		return (_globalAudioPath + "mvo006.wav");
	}
}

Common::String PrivateEngine::getLeaveSound() {
	if (isDemo())
		return (_globalAudioPath + "mvo008.wav");

	// The last sound is only available after going to the police station.
	const char *sounds[7] = {
		"mvo008.wav",
		"mvo004.wav",
		"left1.wav",
		"left2.wav",
		"left3.wav",
		"left4.wav",
		"left5.wav" // "I've had enough trouble with the police"
	};
	Private::Symbol *beenDowntown = maps.variables.getVal(getBeenDowntownVariable());
	uint r = _rnd->getRandomNumber(beenDowntown->u.val ? 6 : 5);
	return _globalAudioPath + sounds[r];
}

// Timer

void PrivateEngine::setTimer(uint32 delay, const Common::String &setting, const Common::String &skipSetting) {
	_timerSetting = setting;
	_timerSkipSetting = skipSetting;
	_timerStartTime = _system->getMillis();
	_timerDelay = delay;
}

void PrivateEngine::clearTimer() {
	_timerSetting.clear();
	_timerSkipSetting.clear();
	_timerStartTime = 0;
	_timerDelay = 0;
}

void PrivateEngine::skipTimer() {
	_nextSetting = _timerSkipSetting;
	clearTimer();
}

void PrivateEngine::checkTimer() {
	if (_timerSetting.empty()) {
		return;
	}
	
	uint32 now = _system->getMillis();
	if (now - _timerStartTime >= _timerDelay) {
		_nextSetting = _timerSetting;
		clearTimer();
	}
}

// Diary

void PrivateEngine::loadLocations(const Common::Rect &rect) {
	// Locations are displayed in the order they are visited.
	// maps.locations and maps.locationList contain all locations.
	// A non-zero symbol value indicates that a location has been
	// visited and the order in which it was visited.

	// Create an array of visited locations, sorted by order visited
	Common::Array<const Symbol *> visitedLocations;
	Common::HashMap<const Symbol *, int> locationIDs;
	int locationID = 1; // one-based for image file names
	for (NameList::const_iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		const Private::Symbol *sym = maps.locations.getVal(*it);
		if (sym->u.val != 0) {
			visitedLocations.push_back(sym);
			locationIDs[sym] = locationID;
		}
		locationID++;
	}
	Common::sort(visitedLocations.begin(), visitedLocations.end(), [](const Symbol *a, const Symbol *b) {
		return a->u.val < b->u.val;
	});

	// Load the sorted visited locations
	int16 offset = 54;
	for (uint i = 0; i < visitedLocations.size(); i++) {
		const Private::Symbol *sym = visitedLocations[i];
		Common::String s =
			Common::String::format("%sdryloc%d.bmp", _diaryLocPrefix.c_str(), locationIDs[sym]);

		MaskInfo m;
		loadMaskAndInfo(&m, s, rect.left + 90, rect.top + offset, true);
		m.cursor = getExitCursor();
		m.nextSetting = getDiaryMiddleSetting();
		m.flag1 = nullptr;
		m.flag2 = nullptr;
		m.useBoxCollision = true;
		_masks.push_front(m);
		_locationMasks.push_back(m);
		offset += 26;
	}
}

void PrivateEngine::loadInventory(uint32 x, const Common::Rect &r1, const Common::Rect &r2) {
	int16 offset = 0;
	for (InvList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		Graphics::Surface *surface = loadMask(it->diaryImage, r1.left, r1.top + offset, true);
		surface->free();
		delete surface;
		offset += 20;
	}
}

void PrivateEngine::loadMemories(const Common::Rect &rect, uint rightPageOffset, uint verticalOffset) {
	if (_currentDiaryPage < 0 ||_currentDiaryPage >= (int)_diaryPages.size())
		return;

	Common::String s = Common::String::format("inface/diary/loctabs/drytab%d.bmp", _diaryPages[_currentDiaryPage].locationID);
	loadImage(s, 0, 0);

	uint memoriesLoaded = 0;
	uint currentVerticalOffset = 0;
	uint horizontalOffset = 0;

	for (uint i = 0; i < _diaryPages[_currentDiaryPage].memories.size(); i++) {
		MaskInfo m;
		m.surf = loadMask(_diaryPages[_currentDiaryPage].memories[i].image, rect.left + horizontalOffset, rect.top + currentVerticalOffset, true);
		m.cursor = getExitCursor();
		m.nextSetting = getDiaryMiddleSetting();
		m.flag1 = nullptr;
		m.flag2 = nullptr;
		_masks.push_front(m);
		_memoryMasks.push_back(m);

		currentVerticalOffset += verticalOffset;
		memoriesLoaded++;

		if (memoriesLoaded == 3) {
			horizontalOffset = rightPageOffset;
			currentVerticalOffset = 0;
		}
	}
}

void PrivateEngine::setLocationAsVisited(Symbol *location) {
	if (location->u.val == 0) {
		// visited locations have non-zero values.
		// set to an incrementing value to record the order visited.
		int maxLocationValue = getMaxLocationValue();
		setSymbol(location, maxLocationValue + 1);
	}
}

int PrivateEngine::getMaxLocationValue() {
	int maxValue = 0;
	for (SymbolMap::iterator it = maps.locations.begin(); it != maps.locations.end(); ++it) {
		Symbol *s = it->_value;
		maxValue = MAX(maxValue, s->u.val);
	}
	return maxValue;
}

bool PrivateEngine::selectSkipMemoryVideo(Common::Point mousePos) {
	// this is mode 2 in the original, but we don't use kGoThumbnailMovie
	if (_mode == 0 && _videoDecoder != nullptr && _currentSetting == getDiaryMiddleSetting()) {
		const uint32 tol = 15;
		const Common::Point origin(kOriginOne[0], kOriginOne[1]);
		const Common::Rect window(origin.x - tol, origin.y - tol, _screenW - origin.x + tol, _screenH - origin.y + tol);
		if (!window.contains(mousePos)) {
			skipVideo();
			return true;
		}
	}
	return false;
}

} // End of namespace Private
