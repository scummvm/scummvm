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
	  _screenW(640), _screenH(480) {
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

	// Safe
	_safeNumberPath = "sg/search_s/sgsaf%d.bmp";
	for (uint d = 0 ; d < 3; d++) {
		_safeDigitArea[d].clear();
		_safeDigit[d] = 0;
		_safeDigitRect[d] = Common::Rect(0, 0);
	}
}

PrivateEngine::~PrivateEngine() {
	// Dispose your resources here
	delete _frameImage;
	delete _rnd;

	delete Gen::g_vm;
	delete Settings::g_setts;
}

void PrivateEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath, 0, 10);
}

Common::SeekableReadStream *PrivateEngine::loadAssets() {

	Common::File *test = new Common::File();
	Common::SeekableReadStream *file = nullptr;

	if (isDemo() && test->open("SUPPORT/ASSETS/DEMOGAME.WIN"))
		return test;

	if (isDemo() && test->open("SUPPORT/DEMOGAME.MAC"))
		return test;
	if (test->open("SUPPORT/ASSETS/GAME.WIN"))
		return test;
	if (test->open("SUPPORT/GAME.MAC"))
		return test;

	delete test;

	if (_platform == Common::kPlatformMacintosh && _language == Common::JA_JPN)
		file = Common::MacResManager::openFileOrDataFork("xn--16jc8na7ay6a0eyg9e5nud0e4525d");
	else
		file = Common::MacResManager::openFileOrDataFork(isDemo() ? "Private Eye Demo Installer" : "Private Eye Installer");
	if (file) {
		Common::Archive *s = createStuffItArchive(file, true);
		Common::SeekableReadStream *file2 = nullptr;
		if (s)
			file2 = s->createReadStreamForMember(isDemo() ? "demogame.mac" : "game.mac");
		// file2 is enough to keep valid reference
		delete file;
		if (file2)
			return file2;
	}

	if (!_installerArchive.open("SUPPORT/ASSETS.Z"))
		error("Failed to open SUPPORT/ASSETS.Z");
	// if the full game is used
	if (!isDemo()) {
		if (_installerArchive.hasFile("GAME.DAT"))
			return _installerArchive.createReadStreamForMember("GAME.DAT");
		if (_installerArchive.hasFile("GAME.WIN"))
			return _installerArchive.createReadStreamForMember("GAME.WIN");
		error("Unknown version");
		return nullptr;
	}

	// if the demo from archive.org is used
	if (_installerArchive.hasFile("GAME.TXT"))
		return _installerArchive.createReadStreamForMember("GAME.TXT");

	// if the demo from the full retail CDROM is used
	if (_installerArchive.hasFile("DEMOGAME.DAT"))
		return _installerArchive.createReadStreamForMember("DEMOGAME.DAT");
	if (_installerArchive.hasFile("DEMOGAME.WIN"))
		return _installerArchive.createReadStreamForMember("DEMOGAME.WIN");

	error("Unknown version");
	return nullptr;
}

Common::Error PrivateEngine::run() {

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

	// Initialize graphics
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_screenW, _screenH, &_pixelFormat);
	_transparentColor = 250;

	_safeColor = _pixelFormat.RGBToColor(65, 65, 65);
	_screenRect = Common::Rect(0, 0, _screenW, _screenH);
	changeCursor("default");
	_origin = Common::Point(0, 0);
	_image = new Image::BitmapDecoder();
	_compositeSurface = new Graphics::ManagedSurface();
	_compositeSurface->create(_screenW, _screenH, _pixelFormat);
	_compositeSurface->setTransparentColor(_transparentColor);

	// Load the game frame once
	byte *palette;
	_frameImage = decodeImage(_framePath, nullptr);
	_mframeImage = decodeImage(_framePath, &palette);

	_framePalette = (byte *) malloc(3*256);
	memcpy(_framePalette, palette, 3*256);

	byte *initialPalette;
	Graphics::Surface *surf = decodeImage("inface/general/inface1.bmp", &initialPalette);
	_compositeSurface->setPalette(initialPalette, 0, 256);
	surf->free();
	delete surf;
	_image->destroy();

	// Main event loop
	Common::Event event;
	Common::Point mousePos;
	_videoDecoder = nullptr;
	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else {
		_nextSetting = getGoIntroSetting();
	}

	while (!shouldQuit()) {
		checkPhoneCall();

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _videoDecoder) {
					skipVideo();
				}
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
				// Reset cursor to default
				changeCursor("default");
				// The following functions will return true
				// if the cursor is changed
				if (cursorPauseMovie(mousePos)) {
				} else if (cursorMask(mousePos)) {
				} else
					cursorExit(mousePos);
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
			continue;
		}

		if (!_nextVS.empty() && _currentVS.empty() && (_currentSetting == getMainDesktopSetting())) {
			loadImage(_nextVS, 160, 120);
			drawScreen();
			_currentVS = _nextVS;
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			if (_videoDecoder->getCurFrame() == 0)
				stopSound(true);
			if (_videoDecoder->endOfVideo()) {
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
				_currentMovie = "";
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
			changeCursor("default");
			drawScreen();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
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
	_exits.clear();
	_masks.clear();

	if (_loadGameMask.surf)
		_loadGameMask.surf->free();
	delete _loadGameMask.surf;
	_loadGameMask.clear();

	if (_saveGameMask.surf)
		_saveGameMask.surf->free();
	delete _saveGameMask.surf;
	_saveGameMask.clear();

	if (_policeRadioArea.surf)
		_policeRadioArea.surf->free();
	delete _policeRadioArea.surf;
	_policeRadioArea.clear();

	if (_AMRadioArea.surf)
		_AMRadioArea.surf->free();
	delete _AMRadioArea.surf;
	_AMRadioArea.clear();

	if (_phoneArea.surf)
		_phoneArea.surf->free();
	delete _phoneArea.surf;
	_phoneArea.clear();

	if (_dossierNextSuspectMask.surf)
		_dossierNextSuspectMask.surf->free();
	delete _dossierNextSuspectMask.surf;
	_dossierNextSuspectMask.clear();

	if (_dossierPrevSuspectMask.surf)
		_dossierPrevSuspectMask.surf->free();
	delete _dossierPrevSuspectMask.surf;
	_dossierPrevSuspectMask.clear();

	if (_dossierNextSheetMask.surf)
		_dossierNextSheetMask.surf->free();
	delete _dossierNextSheetMask.surf;
	_dossierNextSheetMask.clear();

	if (_dossierPrevSheetMask.surf)
		_dossierPrevSheetMask.surf->free();
	delete _dossierPrevSheetMask.surf;
	_dossierPrevSheetMask.clear();

	for (uint d = 0 ; d < 3; d++) {
		if (_safeDigitArea[d].surf)
			_safeDigitArea[d].surf->free();
		delete _safeDigitArea[d].surf;
		_safeDigitArea[d].clear();
		_safeDigit[d] = 0;
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

		if (inMask(m.surf, mousePos)) {
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
			return true;
		}
	}
	return false;
}

Common::String PrivateEngine::getPauseMovieSetting() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kPauseMovie";

	return "k3";
}

Common::String PrivateEngine::getGoIntroSetting() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR || _language == Common::JA_JPN) && _platform != Common::kPlatformMacintosh)
		return "kGoIntro";

	return "k1";
}

Common::String PrivateEngine::getAlternateGameVariable() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kAlternateGame";

	return "k2";
}

Common::String PrivateEngine::getMainDesktopSetting() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kMainDesktop";

	if (isDemo())
		return "k45";

	return "k183";
}

Common::String PrivateEngine::getPoliceIndexVariable() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kPoliceIndex";

	return "k0";
}

Common::String PrivateEngine::getPOGoBustMovieSetting() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kPOGoBustMovie";

	return "k7";
}

Common::String PrivateEngine::getPoliceBustFromMOSetting() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kPoliceBustFromMO";

	return "k6";
}

Common::String PrivateEngine::getWallSafeValueVariable() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kWallSafeValue";

	return "k3";
}

Common::String PrivateEngine::getExitCursor() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kExit";

	return "k5";
}

Common::String PrivateEngine::getInventoryCursor() {
	if ((_language == Common::EN_USA || _language == Common::RU_RUS || _language == Common::KO_KOR) && _platform != Common::kPlatformMacintosh)
		return "kInventory";

	return "k7";
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
	if (_videoDecoder) {
		_videoDecoder->pauseVideo(false);
		const byte *videoPalette = g_private->_videoDecoder->getPalette();
		g_system->getPaletteManager()->setPalette(videoPalette, 0, 256);
		drawScreenFrame(videoPalette);
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
				setSymbol(m.flag1, 1);
				// an item was taken
				if (_toTake) {
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
	}
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
		const PhoneInfo &i = _phone.back();
		setSymbol(i.flag, i.val);
		Common::String sound = _phonePrefix + i.sound + ".wav";
		playSound(sound, 1, true, false);
		_phone.pop_back();
	}
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

bool PrivateEngine::selectSafeDigit(Common::Point mousePos) {
	if (_safeDigitArea[0].surf == nullptr)
		return false;

	mousePos = mousePos - _origin;
	if (mousePos.x < 0 || mousePos.y < 0)
		return false;

	for (uint d = 0 ; d < 3; d ++)
		if (_safeDigitRect[d].contains(mousePos)) {
			_safeDigit[d] = (_safeDigit[d] + 1) % 10;
			renderSafeDigit(d);
			Private::Symbol *sym = maps.variables.getVal(getWallSafeValueVariable());
			sym->u.val = 100*_safeDigit[0] + 10*_safeDigit[1] + _safeDigit[2];
			return true;
		}

	return false;
}

void PrivateEngine::addSafeDigit(uint32 d, Common::Rect *rect) {

	MaskInfo m;
	_safeDigitRect[d] = *rect;
	fillRect(_safeColor, _safeDigitRect[d]);
	m.surf = loadMask(Common::String::format(_safeNumberPath.c_str(), _safeDigit[d]), _safeDigitRect[d].left, _safeDigitRect[d].top, true);
	m.cursor = g_private->getExitCursor();
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	_safeDigitArea[d] = m;
	drawScreen();
}


void PrivateEngine::renderSafeDigit(uint32 d) {

	if (_safeDigitArea[d].surf != nullptr) {
		_safeDigitArea[d].surf->free();
		delete _safeDigitArea[d].surf;
		_safeDigitArea[d].clear();
	}
	fillRect(_safeColor, _safeDigitRect[d]);
	MaskInfo m;
	m.surf = loadMask(Common::String::format(_safeNumberPath.c_str(), _safeDigit[d]), _safeDigitRect[d].left, _safeDigitRect[d].top, true);
	m.cursor = g_private->getExitCursor();
	m.nextSetting = "";
	m.flag1 = nullptr;
	m.flag2 = nullptr;
	_safeDigitArea[d] = m;
	drawScreen();
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

	// Sounds
	_AMRadio.clear();
	_policeRadio.clear();
	_phone.clear();
	_playedPhoneClips.clear();

	// Movies
	_repeatedMovieExit = "";
	_playedMovies.clear();

	// Pause
	_pausedSetting = "";

	// VSPicture
	_nextVS = "";
}

Common::Error PrivateEngine::loadGameStream(Common::SeekableReadStream *stream) {
	// We don't want to continue with any sound from a previous game
	stopSound(true);

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

	// Dossiers
	_dossiers.clear();
	size = stream->readUint32LE();
	DossierInfo m;
	for (uint32 i = 0; i < size; ++i) {
		m.page1 = stream->readString();
		m.page2 = stream->readString();
		_dossiers.push_back(m);
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
}

bool PrivateEngine::isSoundActive() {
	return _mixer->isSoundIDActive(-1);
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
	_videoDecoder->start();
}

void PrivateEngine::skipVideo() {
	_videoDecoder->close();
	delete _videoDecoder;
	_videoDecoder = nullptr;
	_currentMovie = "";
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

Graphics::Surface *PrivateEngine::decodeImage(const Common::String &name, byte **palette) {
	debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());
	Common::Path path = convertPath(name);
	Common::ScopedPtr<Common::SeekableReadStream> file(Common::MacResManager::openFileOrDataFork(path));
	if (!file)
		error("unable to load image %s", name.c_str());

	_image->loadStream(*file);
	const Graphics::Surface *oldImage = _image->getSurface();
	Graphics::Surface *newImage;

	const byte *oldPalette = _image->getPalette();
	byte *currentPalette;

	uint16 ncolors = _image->getPaletteColorCount();
	if (ncolors < 256 || path.toString('/').hasPrefix("intro")) { // For some reason, requires color remapping
		currentPalette = (byte *) malloc(3*256);
		drawScreen();
		g_system->getPaletteManager()->grabPalette(currentPalette, 0, 256);
		newImage = oldImage->convertTo(_pixelFormat, currentPalette);
		remapImage(ncolors, oldImage, oldPalette, newImage, currentPalette);
	} else {
		currentPalette = const_cast<byte *>(oldPalette);
		newImage = oldImage->convertTo(_pixelFormat, currentPalette);
	}

	if (palette != nullptr) {
		*palette = currentPalette;
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

void PrivateEngine::loadImage(const Common::String &name, int x, int y) {
	debugC(1, kPrivateDebugFunction, "%s(%s,%d,%d)", __FUNCTION__, name.c_str(), x, y);
	byte *palette;
	Graphics::Surface *surf = decodeImage(name, &palette);
	_compositeSurface->setPalette(palette, 0, 256);
	_compositeSurface->setTransparentColor(_transparentColor);
	_compositeSurface->transBlitFrom(*surf, _origin + Common::Point(x, y), _transparentColor);
	surf->free();
	delete surf;
	_image->destroy();
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

Graphics::Surface *PrivateEngine::loadMask(const Common::String &name, int x, int y, bool drawn) {
	debugC(1, kPrivateDebugFunction, "%s(%s,%d,%d,%d)", __FUNCTION__, name.c_str(), x, y, drawn);
	Graphics::Surface *surf = new Graphics::Surface();
	surf->create(_screenW, _screenH, _pixelFormat);
	surf->fillRect(_screenRect, _transparentColor);
	byte *palette;
	Graphics::Surface *csurf = decodeImage(name, &palette);

	uint32 hdiff = 0;
	uint32 wdiff = 0;

	if (x + csurf->h > _screenH)
		hdiff = x + csurf->h - _screenH;
	if (y + csurf->w > _screenW)
		wdiff = y + csurf->w - _screenW;

	Common::Rect crect(csurf->w - wdiff, csurf->h - hdiff);
	surf->copyRectToSurface(*csurf, x, y, crect);

	if (drawn) {
		_compositeSurface->setPalette(palette, 0, 256);
		_compositeSurface->setTransparentColor(_transparentColor);
		drawMask(surf);
	}

	csurf->free();
	delete csurf;
	_image->destroy();

	return surf;
}

void PrivateEngine::drawMask(Graphics::Surface *surf) {
	_compositeSurface->transBlitFrom(*surf, _origin, _transparentColor);
}

void PrivateEngine::drawScreen() {
	if (_videoDecoder && !_videoDecoder->isPaused()) {
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Common::Point center((_screenW - _videoDecoder->getWidth()) / 2, (_screenH - _videoDecoder->getHeight()) / 2);
		const byte *videoPalette = nullptr;

		if (_videoDecoder->hasDirtyPalette()) {
			videoPalette = _videoDecoder->getPalette();
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

		Common::Rect w(_origin.x, _origin.y, _screenW - _origin.x, _screenH - _origin.y);
		Graphics::Surface sa = _compositeSurface->getSubArea(w);
		g_system->copyRectToScreen(sa.getPixels(), sa.pitch, _origin.x, _origin.y, sa.w, sa.h);
	}
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
	return g_system->getTimerManager()->installTimerProc(&timerCallback, delay, (void *)ns, "timerCallback");
}

void PrivateEngine::removeTimer() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
}

// Diary

void PrivateEngine::loadLocations(const Common::Rect &rect) {
	uint32 i = 0;
	int16 offset = 44;
	for (NameList::const_iterator it = maps.locationList.begin(); it != maps.locationList.end(); ++it) {
		const Private::Symbol *sym = maps.locations.getVal(*it);
		i++;
		if (sym->u.val) {
			offset = offset + 22;
			Common::String s =
				Common::String::format("%sdryloc%d.bmp", _diaryLocPrefix.c_str(), i);

			Graphics::Surface *surface = loadMask(s, rect.left + 120, rect.top + offset, true);
			delete surface;
		}
	}
}

void PrivateEngine::loadInventory(uint32 x, const Common::Rect &r1, const Common::Rect &r2) {
	int16 offset = 0;
	for (NameList::const_iterator it = inventory.begin(); it != inventory.end(); ++it) {
		offset = offset + 22;
		Graphics::Surface *surface = loadMask(*it, r1.left, r1.top + offset, true);
		delete surface;
	}
}

} // End of namespace Private
