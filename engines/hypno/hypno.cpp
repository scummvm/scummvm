/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
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
#include "engines/util.h"
#include "image/bmp.h"

#include "hypno/hypno.h"
#include "hypno/grammar.h"

namespace Hypno {

Hotspots *hots;
Settings setts;

extern int parse(const char *);

HypnoEngine::HypnoEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr), _videoDecoder(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0), _frame(nullptr),
	  _maxNumberClicks(0), _sirenWarning(0), _screenW(640), _screenH(480) {
	_rnd = new Common::RandomSource("hypno");
}

HypnoEngine::~HypnoEngine() {
	// Dispose your resources here
	delete _frame;
	delete _rnd;
}

void HypnoEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 10);
}

void HypnoEngine::loadMis(Common::String filename) {
    filename = convertPath(filename);
	Common::File *test = new Common::File();
	Common::SeekableReadStream *file = NULL;
	assert(isDemo());
    assert(test->open(filename.c_str()));

	const uint32 fileSize = test->size();
	char *buf = (char *)malloc(fileSize + 1);
	test->read(buf, fileSize);
	buf[fileSize] = '\0';
	parse(buf);
	setts[filename] = *hots;
	debug("Loaded hots size: %d", hots->size());
}

LibData HypnoEngine::loadLib(char *filename) {
	Common::File libfile;
    assert(libfile.open(filename));
	const uint32 fileSize = libfile.size();
	byte *buf = (byte *)malloc(fileSize + 1);
	byte *it = buf;
	libfile.read(buf, fileSize);
	uint32 i = 0;
	uint32 j = 0;
	bool cont = false;
	Common::String entry;
	LibData r;

	while (true) {
		cont = true;
		entry.clear();
		for (j = 0; j < 24; j++) {
			if (cont && it[i] != 0x96 && it[i] != NULL) {
				entry += char(it[i]);
			}
			else
				cont = false;
			i++;			
		}
		if (!cont && entry.size() > 0) {
			debug("Found %s", entry.c_str());
			r.filenames.push_back(entry);
		}
		else {
			break;
		}

	}

	while (it[i] != '\n')
	    i++;

	//ByteArray *files = new ByteArray();

	for (; i < fileSize; i++) {
		if (it[i] != '\n')
			it[i] = it[i] ^ 0xfe;
		r.data.push_back(it[i]);
	}
	
	return r;
}

Common::Error HypnoEngine::run() {
	_language = Common::parseLanguage(ConfMan.get("language"));
	_platform = Common::parsePlatform(ConfMan.get("platform"));
    /*
	LibData files = loadLib("C_MISC/MISSIONS.LIB");
	uint32 i = 0;
	uint32 j = 0;
	uint32 k = 0;

	debug("file: %s",files.filenames[j].c_str());
	for (i = 0; i < files.data.size(); i++) {
		debugN("%c", files.data[i]);
		if (files.data[i] == 'X') {
			if (j == files.filenames.size()-1) {
				debug("Finished at %d from %d", i, files.data.size()); 
				for (k = i; k < files.data.size(); k++)
					debugN("%c", files.data[k]);
				break;
			}

			j++;
			debugN("\n************file: %s**************",files.filenames[j].c_str());
		}
	}
	*/	
	// Read assets file
	loadMis("mis/demo.mis");
	loadMis("mis/order.mis");
	loadMis("mis/alley.mis");
	//loadMis("MIS/SHOCTALK.MIS");

	// Initialize graphics
	initGraphics(_screenW, _screenH, nullptr);
	_pixelFormat = g_system->getScreenFormat();
	if (_pixelFormat == Graphics::PixelFormat::createFormatCLUT8())
		return Common::kUnsupportedColorMode;

	_transparentColor = _pixelFormat.RGBToColor(0, 255, 0);
	_safeColor = _pixelFormat.RGBToColor(65, 65, 65);
	screenRect = Common::Rect(0, 0, _screenW, _screenH);
	//changeCursor("default");
	_origin = Common::Point(0, 0);
	_image = new Image::BitmapDecoder();
	_compositeSurface = new Graphics::ManagedSurface();
	_compositeSurface->create(_screenW, _screenH, _pixelFormat);
	_compositeSurface->setTransparentColor(_transparentColor);

	// Main event loop
	Common::Event event;
	Common::Point mousePos;
	_videoDecoder = nullptr;
	/*int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else {
		_nextSetting = getGoIntroSetting();
	}*/
	Common::String logoIntro = "DEMO/DCINE1.SMK";
	Common::String movieIntro = "DEMO/DCINE2.SMK";
	_nextMovie = logoIntro;

	while ((_nextMovie != "" || _currentMovie != "") && !shouldQuit()) {
			while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _videoDecoder)
					// Ugly hack to play two intro movies.
					if (_currentMovie == logoIntro)
						_nextMovie = movieIntro;
					else
					    skipVideo();

				break;

			default:
				break;
			}
		}

		// Movies
		if (!_nextMovie.empty()) {
			_videoDecoder = new Video::SmackerDecoder();
			playVideo(_nextMovie);
			_currentMovie = _nextMovie;
			_nextMovie = "";
			continue;
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			if (_videoDecoder->getCurFrame() == 0)
				stopSound(true);
			if (_videoDecoder->endOfVideo()) {
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
				// Ugly hack to play two intro movies.
				if (_currentMovie == logoIntro)
					_nextMovie = movieIntro;
				else 
					_nextMovie = "";
			} else if (_videoDecoder->needsUpdate()) {
				drawScreen();
				g_system->delayMillis(10);
			}
			continue;
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}

	//_nextSetting = "mis/alley.mis";
	_nextSetting = "mis/demo.mis";
	changeCursor("mouse/cursor1.smk", 0);

	while (!shouldQuit()) {
		
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _videoDecoder)
					skipVideo();

				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				// if (selectDossierNextSuspect(mousePos))
				// 	break;
				// else if (selectDossierPrevSuspect(mousePos))
				// 	break;
				// else if (selectDossierNextSheet(mousePos))
				// 	break;
				// else if (selectDossierPrevSheet(mousePos))
				// 	break;
				// else if (selectSafeDigit(mousePos))
				// 	break;

				// selectPauseMovie(mousePos);
				// selectPhoneArea(mousePos);
				// selectPoliceRadioArea(mousePos);
				// selectAMRadioArea(mousePos);
				// selectLoadGame(mousePos);
				// selectSaveGame(mousePos);
				// if (_nextSetting.empty())
				// 	selectMask(mousePos);
				// if (_nextSetting.empty())
				// 	selectExit(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				// Reset cursor to default
				//changeCursor("default");
				// The following functions will return true
				// if the cursor is changed
				//if (cursorPauseMovie(mousePos)) {
				//} else if (cursorMask(mousePos)) {
				//} else
				//	cursorExit(mousePos);
				break;

			default:
				break;
			}
		}

		// Movies
		if (!_nextMovie.empty()) {
			//removeTimer();
			_videoDecoder = new Video::SmackerDecoder();
			playVideo(_nextMovie);
			_currentMovie = _nextMovie;
			_nextMovie = "";
			continue;
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
				g_system->delayMillis(10);
			}
			continue;
		}

		if (!_nextSetting.empty()) {

			debug("Executing %s", _nextSetting.c_str());
			//clearAreas();
			_currentSetting = _nextSetting;
			//Settings::g_setts->load(_nextSetting);
			prepareHotspots(setts[_currentSetting]);
			_nextSetting = "";
			//Gen::g_vm->run();
			//changeCursor("default");
			drawScreen();
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
	return Common::kNoError;
}

void HypnoEngine::prepareHotspots(Hotspots hs) {
	for (Hotspots::const_iterator it = hs.begin(); it != hs.end(); ++it) {
		const Hotspot h = *it;
		debug("hotspot type: %d action size: %d", it->type, it->actions.size());
		for (Actions::const_iterator itt = it->actions.begin(); itt != it->actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(Background))
				runBackground(h, (Background*) action);
			else if (typeid(*action) == typeid(Overlay))
				runOverlay(h, (Overlay*) action);
			//else if (typeid(*action) == typeid(Mice))
			//	runMice(h, (Mice*) action);
		}	
	}
}

void HypnoEngine::runBackground(const Hotspot h, Background *a) {
	Common::Point origin = a->origin;
	if (a->condition.size() == 0)
		loadImage(a->path, origin.x, origin.y);
}

void HypnoEngine::runOverlay(const Hotspot h, Overlay *a) {
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y);
}

void HypnoEngine::runMice(const Hotspot h, Mice *a) {
	if (h.type == MakeMenu)
    	changeCursor(a->path, a->index);
}

void HypnoEngine::loadImage(const Common::String &name, int x, int y) {
	Graphics::Surface *surf = decodeFrame(name, 0);
	_compositeSurface->transBlitFrom(*surf, _origin + Common::Point(x, y), _transparentColor);
}

Graphics::Surface *HypnoEngine::decodeFrame(const Common::String &name, int n, bool convert) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	_videoDecoder = new Video::SmackerDecoder();
	if (!_videoDecoder->loadStream(file))
		error("unable to load video %s", path.c_str());

    for(int f = 0; f < n; f++) 
		_videoDecoder->decodeNextFrame();

	const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
	Graphics::Surface *rframe;

	if (convert) {
		rframe = frame->convertTo(_pixelFormat, _videoDecoder->getPalette());
	} else {
		rframe = frame->convertTo(frame->format, _videoDecoder->getPalette());
		//rframe->create(frame->w, frame->h, frame->format);
		//rframe->copyRectToSurface(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
	}

	delete _videoDecoder;
	_videoDecoder = nullptr;

	return rframe;
}


void HypnoEngine::drawScreen() {
	Graphics::ManagedSurface *surface = _compositeSurface;

	if (_videoDecoder && !_videoDecoder->isPaused()) {
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Graphics::Surface *sframe = frame->scale(_screenW, _screenH);
		Graphics::Surface *cframe = sframe->convertTo(_pixelFormat, _videoDecoder->getPalette());
		Common::Point center(0, 0);
		surface->blitFrom(*cframe, center);

		sframe->free();
		delete sframe;

		cframe->free();
		delete cframe;
	}

	Common::Rect w(_origin.x, _origin.y, _screenW - _origin.x, _screenH - _origin.y);
	Graphics::Surface sa = surface->getSubArea(w);
	g_system->copyRectToScreen(sa.getPixels(), sa.pitch, _origin.x, _origin.y, sa.w, sa.h);
	g_system->updateScreen();
}

// Video handling

void HypnoEngine::playVideo(const Common::String &name) {
	//debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	if (!_videoDecoder->loadStream(file))
		error("unable to load video %s", path.c_str());
	_videoDecoder->start();
}

void HypnoEngine::skipVideo() {
	_videoDecoder->close();
	delete _videoDecoder;
	_videoDecoder = nullptr;	
	_currentMovie = "";
}

// Sound handling

void HypnoEngine::stopSound(bool all) {
	// debugC(1, kPrivateDebugFunction, "%s(%d)", __FUNCTION__, all);

	// if (all) {
	// 	_mixer->stopHandle(_fgSoundHandle);
	// 	_mixer->stopHandle(_bgSoundHandle);
	// } else {
	// 	_mixer->stopHandle(_fgSoundHandle);
	// }
}

// Path handling

Common::String HypnoEngine::convertPath(const Common::String &name) {
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
	return path;
}

} // End of namespace Hypno
