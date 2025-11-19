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
#include "private/tokens.h"

namespace Private {

PrivateEngine *g_private = nullptr;
extern int parse(const char *);

PrivateEngine::PrivateEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr), _videoDecoder(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0), _frameImage(nullptr),
	  _framePalette(nullptr), _maxNumberClicks(0), _sirenWarning(0),
	  _subtitles(nullptr), _sfxSubtitles(false), _useSubtitles(false),
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
	_modified = false;
	_mode = -1;
	_toTake = false;

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
	_policeBustEnabled = false;
	_policeBustSetting = "";
	_numberClicks = 0;
	_sirenSound = "po/audio/posfx002.wav";

	// General sounds
	_globalAudioPath = "global/audio/";
	_noStopSounds = false;

	// Radios and phone
	_policeRadioArea.clear();
	_AMRadioArea.clear();
	_phoneArea.clear();
	// TODO: use this as a default sound for radio
	_infaceRadioPath = "inface/radio/";
	_phonePrefix = "inface/telephon/";
	_phoneCallSound = "phone.wav";

	// Dossiers
	_dossierPage = 0;
	_dossierSuspect = 0;
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
}

PrivateEngine::~PrivateEngine() {
	destroyVideo();

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
		checkPhoneCall();

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				if (event.customType == kActionSkip) {
					skipVideo();
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
				else if (selectDossierPrevSuspect(mousePos))
					break;
				else if (selectDossierNextSheet(mousePos))
					break;
				else if (selectDossierPrevSheet(mousePos))
					break;
				else if (selectSafeDigit(mousePos))
					break;
				else if (selectDiaryNextPage(mousePos))
					break;
				else if (selectDiaryPrevPage(mousePos))
					break;
				else if (selectLocation(mousePos))
					break;
				else if (selectMemory(mousePos)) {
					_needToDrawScreenFrame = true;
					break;
				}

				selectPauseGame(mousePos);
				selectPhoneArea(mousePos);
				selectPoliceRadioArea(mousePos);
				selectAMRadioArea(mousePos);
				selectLoadGame(mousePos);
				selectSaveGame(mousePos);
				if (_nextSetting.empty())
					selectMask(mousePos);
				if (_nextSetting.empty())
					selectExit(mousePos);
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
			removeTimer();
			_videoDecoder = new Video::SmackerDecoder();
			playVideo(_nextMovie);
			_currentMovie = _nextMovie;
			_nextMovie = "";
			updateCursor(mousePos);
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			if (_videoDecoder->getCurFrame() == 0) {
				stopSound(true);
			}

			if (_videoDecoder->endOfVideo()) {
				delete _videoDecoder;
				_videoDecoder = nullptr;
				if (_subtitles != nullptr) {
					delete _subtitles;
					_subtitles = nullptr;
					g_system->hideOverlay();
				}
				_currentMovie = "";
			} else if (!_videoDecoder->needsUpdate() && mouseMoved) {
				g_system->updateScreen();
			} else if (_videoDecoder->needsUpdate()) {
				drawScreen();
			}
			g_system->delayMillis(5); // Yield to the system
			continue;
		}

		if (!_nextSetting.empty()) {
			removeTimer();
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

		g_system->updateScreen();
		g_system->delayMillis(10);
		if (_subtitles != nullptr) {
			if (_mixer->isSoundHandleActive(_fgSoundHandle)) {
				_subtitles->drawSubtitle(_mixer->getElapsedTime(_fgSoundHandle).msecs(), false, _sfxSubtitles);
			} else {
				delete _subtitles;
				_subtitles = nullptr;
				g_system->hideOverlay();
			}
		}
	}
	removeTimer();
	return Common::kNoError;
}

void PrivateEngine::ignoreEvents() {
	Common::Event event;
	g_system->getEventManager()->pollEvent(event);
	g_system->updateScreen();
	g_system->delayMillis(10);
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
	_phoneArea.clear();
	_dossierNextSuspectMask.clear();
	_dossierPrevSuspectMask.clear();
	_dossierNextSheetMask.clear();
	_dossierPrevSheetMask.clear();

	for (uint d = 0 ; d < 3; d++) {
		if (_safeDigitArea[d].surf) {
			_safeDigitArea[d].surf->free();
			delete _safeDigitArea[d].surf;
		}
		_safeDigitArea[d].clear();
		_safeDigitRect[d] = Common::Rect(0, 0);
	}
}

void PrivateEngine::startPoliceBust() {
	// This logic was extracted from the binary
	int policeIndex = maps.variables.getVal(getPoliceIndexVariable())->u.val;
	int r = _rnd->getRandomNumber(0xc);
	if (policeIndex > 0x14) {
		policeIndex = 0x15;
	}
	_maxNumberClicks = r + 0x10 + (policeIndex * 0xe) / -0x15;
	_sirenWarning = _rnd->getRandomNumber(0x7) + 3;
	_numberClicks = 0;
	if (_sirenWarning >= _maxNumberClicks)
		_sirenWarning = _maxNumberClicks - 1;
}

void PrivateEngine::checkPoliceBust() {
	if (!_policeBustEnabled)
		return;

	if (_numberClicks < _sirenWarning)
		return;

	if (_numberClicks == _sirenWarning) {
		stopSound(true);
		playSound(_sirenSound, 0, false, false);
		_numberClicks++; // Won't execute again
		return;
	}

	if (_numberClicks == _maxNumberClicks + 1) {
		uint policeIndex = maps.variables.getVal(getPoliceIndexVariable())->u.val;
		_policeBustSetting = _currentSetting;
		if (policeIndex <= 13) {
			_nextSetting = getPOGoBustMovieSetting();
		} else {
			_nextSetting = getPoliceBustFromMOSetting();
		}
		clearAreas();
		_policeBustEnabled = false;
	}
}

void PrivateEngine::updateCursor(Common::Point mousePos) {
	// If a function returns true then it changed the cursor.
	if (cursorPauseMovie(mousePos)) {
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
	changeCursor("default");
}

bool PrivateEngine::cursorExit(Common::Point mousePos) {
	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

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

bool PrivateEngine::inBox(const Common::Rect &box, Common::Point mousePos) {
	return box.contains(mousePos);
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
	if (_mode == 1 && !_policeBustEnabled) {
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

void PrivateEngine::selectPauseGame(Common::Point mousePos) {
	if (_mode == 1 && !_policeBustEnabled) {
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
				}
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
	}

	if (_videoDecoder) {
		_videoDecoder->pauseVideo(false);
		_needToDrawScreenFrame = true;
	}
}


void PrivateEngine::selectExit(Common::Point mousePos) {
	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return;

	Common::String ns = "";
	int rs = 100000000;
	int cs = 0;
	for (ExitList::const_iterator it = _exits.begin(); it != _exits.end(); ++it) {
		const ExitInfo &e = *it;
		cs = e.rect.width() * e.rect.height();
		//debug("Testing exit %s %d", e.nextSetting->c_str(), cs);
		if (e.rect.contains(mousePos)) {
			//debug("Inside! %d %d", cs, rs);
			if (cs < rs && !e.nextSetting.empty()) { // TODO: check this
				// an item was not taken
				if (_toTake) {
					playSound(getLeaveSound(), 1, false, false);
					_toTake = false;
				}

				//debug("Found Exit %s %d", e.nextSetting->c_str(), cs);
				rs = cs;
				ns = e.nextSetting;
			}
		}
	}
	if (!ns.empty()) {
		_numberClicks++; // count click only if it hits a hotspot
		_nextSetting = ns;
		_highlightMasks = false;
	}
}

void PrivateEngine::selectMask(Common::Point mousePos) {
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
					if (!inInventory(m.inventoryItem))
						inventory.push_back(m.inventoryItem);
					setSymbol(m.flag1, 1);
					playSound(getTakeSound(), 1, false, false);
					_toTake = false;
				}
			}

			if (m.flag2 != nullptr) {
				setSymbol(m.flag2, 1);
			}
			break;
		}
	}
	if (!ns.empty()) {
		_numberClicks++; // count click only if it hits a hotspot
		_nextSetting = ns;
		_highlightMasks = false;
	}
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
			if (inBox(_locationMasks[i].box, mousePos)) {
				bool diaryPageSet = false;
				for (uint j = 0; j < _diaryPages.size(); j++) {
					if (_diaryPages[j].locationID == totalLocations + 1) {
						_currentDiaryPage = j;
						diaryPageSet = true;
						break;
					}
				}

				_numberClicks++;

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
		const Private::Symbol *sym = maps.locations.getVal(it);
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
		if (sym->u.val && currentLocation == location) {
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
	for (NameList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		if (*it == bmp)
			return true;
	}
	return false;
}

void PrivateEngine::selectAMRadioArea(Common::Point mousePos) {
	if (_AMRadioArea.surf == nullptr)
		return;

	if (_AMRadio.empty())
		return;

	if (inMask(_AMRadioArea.surf, mousePos)) {
		Common::String sound = _infaceRadioPath + "comm_/" + _AMRadio.back() + ".wav";
		playSound(sound, 1, false, false);
		_AMRadio.pop_back();
	}
}

void PrivateEngine::selectPoliceRadioArea(Common::Point mousePos) {
	if (_policeRadioArea.surf == nullptr)
		return;

	if (_policeRadio.empty())
		return;

	if (inMask(_policeRadioArea.surf, mousePos)) {
		Common::String sound = _infaceRadioPath + "police/" + _policeRadio.back() + ".wav";
		playSound(sound, 1, false, false);
		_policeRadio.pop_back();
	}
}

void PrivateEngine::checkPhoneCall() {
	if (_phoneArea.surf == nullptr)
		return;

	if (_phone.empty())
		return;

	if (!_mixer->isSoundHandleActive(_fgSoundHandle))
		playSound(_phonePrefix + _phoneCallSound, 1, false, false);
}

void PrivateEngine::selectPhoneArea(Common::Point mousePos) {
	if (_phoneArea.surf == nullptr)
		return;

	if (_phone.empty())
		return;

	if (inMask(_phoneArea.surf, mousePos)) {
		const PhoneInfo &i = _phone.front();
		// -100 indicates that the variable should be decremented
		if (i.val == -100) {
			setSymbol(i.flag, i.flag->u.val - 1);
		} else {
			setSymbol(i.flag, i.val);
		}
		Common::String sound = _phonePrefix + i.sound + ".wav";
		playSound(sound, 1, true, false);
		_phone.pop_front();
		_nextSetting = getListenToPhoneSetting();
	}
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

	DossierInfo m = _dossiers[_dossierSuspect];

	if (_dossierPage == 0) {
		loadImage(m.page1, x, y);
	} else if (_dossierPage == 1) {
		loadImage(m.page2, x, y);
	} else {
		error("Invalid page");
	}
}

bool PrivateEngine::selectDossierNextSuspect(Common::Point mousePos) {
	if (_dossierNextSuspectMask.surf == nullptr)
		return false;

	if (inMask(_dossierNextSuspectMask.surf, mousePos)) {
		if ((_dossierSuspect + 1) < _dossiers.size()) {
			playSound(getPaperShuffleSound(), 1, false, false);
			_dossierSuspect++;
			_dossierPage = 0;
			loadDossier();
			drawMask(_dossierNextSuspectMask.surf);
			drawMask(_dossierPrevSuspectMask.surf);
			drawScreen();
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
			playSound(getPaperShuffleSound(), 1, false, false);
			_dossierPage = 0;
			loadDossier();
			drawMask(_dossierNextSuspectMask.surf);
			drawMask(_dossierPrevSuspectMask.surf);
			drawScreen();
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
			playSound(getPaperShuffleSound(), 1, false, false);
			_dossierPage = 1;
			loadDossier();
			drawMask(_dossierNextSuspectMask.surf);
			drawMask(_dossierPrevSuspectMask.surf);
			drawScreen();
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
			playSound(getPaperShuffleSound(), 1, false, false);
			_dossierSuspect--;
			_dossierPage = 0;
			loadDossier();
			drawMask(_dossierNextSuspectMask.surf);
			drawMask(_dossierPrevSuspectMask.surf);
			drawScreen();
		}
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
	m.cursor = g_private->getExitCursor();
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

void PrivateEngine::selectLoadGame(Common::Point mousePos) {
	if (_loadGameMask.surf == nullptr)
		return;

	if (inMask(_loadGameMask.surf, mousePos)) {
		loadGameDialog();
	}
}

void PrivateEngine::selectSaveGame(Common::Point mousePos) {
	if (_saveGameMask.surf == nullptr)
		return;

	if (inMask(_saveGameMask.surf, mousePos)) {
		saveGameDialog();
	}
}

bool PrivateEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

void PrivateEngine::restartGame() {
	debugC(1, kPrivateDebugFunction, "restartGame");

	for (NameList::iterator it = maps.variableList.begin(); it != maps.variableList.end(); ++it) {
		Private::Symbol *sym = maps.variables.getVal(*it);
		if (*(sym->name) != getAlternateGameVariable())
			sym->u.val = 0;
	}

	// Diary
	for (NameList::iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		Private::Symbol *sym = maps.locations.getVal(*it);
		sym->u.val = 0;
	}
	inventory.clear();
	_dossiers.clear();
	_diaryPages.clear();

	// Sounds
	_AMRadio.clear();
	_policeRadio.clear();
	_phone.clear();
	_playedPhoneClips.clear();

	// Movies
	_repeatedMovieExit = "";
	_playedMovies.clear();
	destroyVideo();

	// Pause
	_pausedSetting = "";

	// VSPicture
	_nextVS = "";

	// Wall Safe
	initializeWallSafeValue();
}

Common::Error PrivateEngine::loadGameStream(Common::SeekableReadStream *stream) {
	// We don't want to continue with any sound or videos from a previous game
	stopSound(true);
	destroyVideo();

	Common::Serializer s(stream, nullptr);
	debugC(1, kPrivateDebugFunction, "loadGameStream");
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
		inventory.push_back(stream->readString());
	}

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

	// Radios
	size = stream->readUint32LE();
	_AMRadio.clear();

	for (uint32 i = 0; i < size; ++i) {
		_AMRadio.push_back(stream->readString());
	}

	size = stream->readUint32LE();
	_policeRadio.clear();

	for (uint32 i = 0; i < size; ++i) {
		_policeRadio.push_back(stream->readString());
	}

	size = stream->readUint32LE();
	_phone.clear();
	PhoneInfo p;
	Common::String name;
	for (uint32 j = 0; j < size; ++j) {
		p.sound = stream->readString();
		name = stream->readString();
		p.flag = maps.lookupVariable(&name);
		p.val = stream->readUint32LE();
		_phone.push_back(p);
	}

	// Played media
	_repeatedMovieExit = stream->readString();
	_playedMovies.clear();
	size = stream->readUint32LE();
	for (uint32 i = 0; i < size; ++i) {
		_playedMovies.setVal(stream->readString(), true);
	}

	_playedPhoneClips.clear();
	size = stream->readUint32LE();
	for (uint32 i = 0; i < size; ++i) {
		_playedPhoneClips.setVal(stream->readString(), true);
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

	return Common::kNoError;
}

Common::Error PrivateEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	debugC(1, kPrivateDebugFunction, "saveGameStream(%d)", isAutosave);
	if (isAutosave)
		return Common::kNoError;

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
	for (NameList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		stream->writeString(*it);
		stream->writeByte(0);
	}

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

	// Radios
	stream->writeUint32LE(_AMRadio.size());
	for (SoundList::const_iterator it = _AMRadio.begin(); it != _AMRadio.end(); ++it) {
		stream->writeString(*it);
		stream->writeByte(0);
	}
	stream->writeUint32LE(_policeRadio.size());
	for (SoundList::const_iterator it = _policeRadio.begin(); it != _policeRadio.end(); ++it) {
		stream->writeString(*it);
		stream->writeByte(0);
	}

	stream->writeUint32LE(_phone.size());
	for (PhoneList::const_iterator it = _phone.begin(); it != _phone.end(); ++it) {
		stream->writeString(it->sound);
		stream->writeByte(0);
		stream->writeString(*it->flag->name);
		stream->writeByte(0);
		stream->writeUint32LE(it->val);
	}

	// Played media
	stream->writeString(_repeatedMovieExit);
	stream->writeByte(0);

	stream->writeUint32LE(_playedMovies.size());
	for (PlayedMediaTable::const_iterator it = _playedMovies.begin(); it != _playedMovies.end(); ++it) {
		stream->writeString(it->_key);
		stream->writeByte(0);
	}

	stream->writeUint32LE(_playedPhoneClips.size());
	for (PlayedMediaTable::const_iterator it = _playedPhoneClips.begin(); it != _playedPhoneClips.end(); ++it) {
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

void PrivateEngine::playSound(const Common::String &name, uint loops, bool stopOthers, bool background) {
	debugC(1, kPrivateDebugFunction, "%s(%s,%d,%d,%d)", __FUNCTION__, name.c_str(), loops, stopOthers, background);

	Common::Path path = convertPath(name);
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(path);

	if (!file)
		error("unable to find sound file %s", path.toString().c_str());

	Audio::LoopingAudioStream *stream;
	stream = new Audio::LoopingAudioStream(Audio::makeWAVStream(file, DisposeAfterUse::YES), loops);
	if (stopOthers) {
		stopSound(true);
	}

	Audio::SoundHandle *sh = nullptr;
	if (background) {
		_mixer->stopHandle(_bgSoundHandle);
		sh = &_bgSoundHandle;
	} else {
		_mixer->stopHandle(_fgSoundHandle);
		sh = &_fgSoundHandle;
	}

	_mixer->playStream(Audio::Mixer::kSFXSoundType, sh, stream, -1, Audio::Mixer::kMaxChannelVolume);
	loadSubtitles(path);
}

bool PrivateEngine::isSoundActive() {
	return _mixer->isSoundIDActive(-1);
}

void PrivateEngine::waitForSoundToStop() {
	while (g_private->isSoundActive())
		g_private->ignoreEvents();

	uint32 i = 100;
	while (i--) // one second extra
		g_private->ignoreEvents();
}

void PrivateEngine::adjustSubtitleSize() {
	debugC(1, kPrivateDebugFunction, "%s()", __FUNCTION__);
	if (_subtitles) {
		// Subtitle positioning constants (as percentages of screen height)
		const int HORIZONTAL_MARGIN = 20;
		const float BOTTOM_MARGIN_PERCENT = 0.009f;  // ~20px at 2160p
		const float MAIN_MENU_HEIGHT_PERCENT = 0.093f;  // ~200px at 2160p
		const float ALTERNATE_MODE_HEIGHT_PERCENT = 0.102f;  // ~220px at 2160p
		const float DEFAULT_HEIGHT_PERCENT = 0.074f;  // ~160px at 2160p

		// Font sizing constants (as percentage of screen height)
		const int MIN_FONT_SIZE = 8;
		const float BASE_FONT_SIZE_PERCENT = 0.023f;  // ~50px at 2160p

		int16 h = g_system->getOverlayHeight();
		int16 w = g_system->getOverlayWidth();

		int bottomMargin = int(h * BOTTOM_MARGIN_PERCENT);

		// If we are in the main menu, we need to adjust the position of the subtitles
		if (_mode == 0) {
			int topOffset = int(h * MAIN_MENU_HEIGHT_PERCENT);
			_subtitles->setBBox(Common::Rect(HORIZONTAL_MARGIN,
											h - topOffset,
											w - HORIZONTAL_MARGIN,
											h - bottomMargin));
		} else if (_mode == -1) {
			int topOffset = int(h * ALTERNATE_MODE_HEIGHT_PERCENT);
			_subtitles->setBBox(Common::Rect(HORIZONTAL_MARGIN,
											h - topOffset,
											w - HORIZONTAL_MARGIN,
											h - bottomMargin));
		} else {
			int topOffset = int(h * DEFAULT_HEIGHT_PERCENT);
			_subtitles->setBBox(Common::Rect(HORIZONTAL_MARGIN,
											h - topOffset,
											w - HORIZONTAL_MARGIN,
											h - bottomMargin));
		}

		int fontSize = MAX(MIN_FONT_SIZE, int(h * BASE_FONT_SIZE_PERCENT));
		_subtitles->setColor(0xff, 0xff, 0x80);
		_subtitles->setFont("LiberationSans-Regular.ttf", fontSize, Video::Subtitles::kFontStyleRegular);
		_subtitles->setFont("LiberationSans-Italic.ttf", fontSize, Video::Subtitles::kFontStyleItalic);
	}
}

void PrivateEngine::loadSubtitles(const Common::Path &path) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, path.toString().c_str());
	if (!_useSubtitles)
		return;

	Common::String subPathStr = path.toString() + ".srt";
	subPathStr.toLowercase();
	subPathStr.replace('/', '_');
	Common::String language(Common::getLanguageCode(_language));
	if (language == "us")
		language = "en";

	Common::Path subPath = "subtitles";
	subPath = subPath.appendComponent(language);
	subPath = subPath.appendComponent(subPathStr);
	debugC(1, kPrivateDebugFunction, "Loading subtitles from %s", subPath.toString().c_str());

	if (_subtitles != nullptr) {
		delete _subtitles;
		_subtitles = nullptr;
		g_system->hideOverlay();
	}

	_subtitles = new Video::Subtitles();
	_subtitles->loadSRTFile(subPath);
	if (!_subtitles->isLoaded()) {
		delete _subtitles;
		_subtitles = nullptr;
		return;
	}

	g_system->showOverlay(false);
	g_system->clearOverlay();
	adjustSubtitleSize();
}
void PrivateEngine::playVideo(const Common::String &name) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());
	//stopSound(true);
	Common::Path path = convertPath(name);
	Common::SeekableReadStream *file = Common::MacResManager::openFileOrDataFork(path);

	if (!file)
		error("unable to find video file %s", path.toString().c_str());

	if (!_videoDecoder->loadStream(file))
		error("unable to load video %s", path.toString().c_str());

	loadSubtitles(path);
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
	if (_subtitles != nullptr) {
		delete _subtitles;
		_subtitles = nullptr;
		g_system->hideOverlay();
	}
	_currentMovie = "";
}

void PrivateEngine::destroyVideo() {
	if (_videoDecoder != _pausedVideo) {
		delete _pausedVideo;
	}
	delete _videoDecoder;
	_videoDecoder = nullptr;
	_pausedVideo = nullptr;
	if (_subtitles != nullptr) {
		delete _subtitles;
		_subtitles = nullptr;
		g_system->hideOverlay();
	}
}

void PrivateEngine::stopSound(bool all) {
	debugC(1, kPrivateDebugFunction, "%s(%d)", __FUNCTION__, all);

	if (all) {
		_mixer->stopHandle(_fgSoundHandle);
		_mixer->stopHandle(_bgSoundHandle);
	} else {
		_mixer->stopHandle(_fgSoundHandle);
	}
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
		g_system->getPaletteManager()->grabPalette(currentPalette, 0, 256);
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
	g_system->copyRectToScreen(_mframeImage->getPixels(), _mframeImage->pitch, 0, 0, _screenW, _screenH);
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
			g_system->getPaletteManager()->setPalette(videoPalette, 0, 256);
			drawScreenFrame(videoPalette);
			_needToDrawScreenFrame = false;
		} else if (_videoDecoder->hasDirtyPalette()) {
			const byte *videoPalette = _videoDecoder->getPalette();
			g_system->getPaletteManager()->setPalette(videoPalette, 0, 256);

			if (_mode == 1) {
				drawScreenFrame(videoPalette);
			}
		}

		// No use of _compositeSurface, we write the frame directly to the screen in the expected position
		g_system->copyRectToScreen(frame->getPixels(), frame->pitch, center.x, center.y, frame->w, frame->h);
	} else {
		byte newPalette[256 * 3];
		_compositeSurface->grabPalette(newPalette, 0, 256);
		g_system->getPaletteManager()->setPalette(newPalette, 0, 256);

		if (_mode == 1) {
			// We can reuse newPalette
			g_system->getPaletteManager()->grabPalette((byte *) &newPalette, 0, 256);
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
		g_system->copyRectToScreen(sa.getPixels(), sa.pitch, _origin.x, _origin.y, sa.w, sa.h);
	}

	if (_subtitles && _videoDecoder && !_videoDecoder->isPaused())
		_subtitles->drawSubtitle(_videoDecoder->getTime(), false, _sfxSubtitles);
	g_system->updateScreen();
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

	uint r = _rnd->getRandomNumber(4) + 1;
	return Common::String::format("%stook%d.wav", _globalAudioPath.c_str(), r);
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

	uint r = _rnd->getRandomNumber(4) + 1;
	return Common::String::format("%sleft%d.wav", _globalAudioPath.c_str(), r);
}

Common::String PrivateEngine::getRandomPhoneClip(const char *clip, int i, int j) {
	uint r = i + _rnd->getRandomNumber(j - i);
	return Common::String::format("%s%02d", clip, r);
}

// Timers
static void timerCallback(void *refCon) {
	g_private->removeTimer();
	g_private->_nextSetting = *(Common::String *)refCon;
}

bool PrivateEngine::installTimer(uint32 delay, Common::String *ns) {
	return g_system->getTimerManager()->installTimerProc(&timerCallback, delay, ns, "timerCallback");
}

void PrivateEngine::removeTimer() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
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
	Common::sort(visitedLocations.begin(), visitedLocations.end(), [&locationIDs](const Symbol *a, const Symbol *b) {
		if (a->u.val != b->u.val) {
			return a->u.val < b->u.val;
		} else {
			// backwards compatibility for older saves files that stored 1
			// for visited locations and displayed them in a fixed order.
			return locationIDs[a] < locationIDs[b];
		}
	});

	// Load the sorted visited locations
	int16 offset = 54;
	for (uint i = 0; i < visitedLocations.size(); i++) {
		const Private::Symbol *sym = visitedLocations[i];
		Common::String s =
			Common::String::format("%sdryloc%d.bmp", _diaryLocPrefix.c_str(), locationIDs[sym]);

		MaskInfo m;
		loadMaskAndInfo(&m, s, rect.left + 90, rect.top + offset, true);
		m.cursor = g_private->getExitCursor();
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
	for (NameList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		Graphics::Surface *surface = loadMask(*it, r1.left, r1.top + offset, true);
		surface->free();
		delete surface;
		offset += 20;
	}
}

void PrivateEngine::loadMemories(const Common::Rect &rect, uint rightPageOffset, uint verticalOffset) {
	if (_currentDiaryPage < 0)
		return;

	Common::String s = Common::String::format("inface/diary/loctabs/drytab%d.bmp", _diaryPages[_currentDiaryPage].locationID);
	loadImage(s, 0, 0);

	uint memoriesLoaded = 0;
	uint currentVerticalOffset = 0;
	uint horizontalOffset = 0;

	for (uint i = 0; i < _diaryPages[_currentDiaryPage].memories.size(); i++) {
		MaskInfo m;
		m.surf = loadMask(_diaryPages[_currentDiaryPage].memories[i].image, rect.left + horizontalOffset, rect.top + currentVerticalOffset, true);
		m.cursor = g_private->getExitCursor();
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

int PrivateEngine::getMaxLocationValue() {
	int maxValue = 0;
	for (SymbolMap::iterator it = maps.locations.begin(); it != maps.locations.end(); ++it) {
		Symbol *s = it->_value;
		maxValue = MAX(maxValue, s->u.val);
	}
	return maxValue;
}

} // End of namespace Private
