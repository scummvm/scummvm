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

Hotspots *g_parsedHots;
ArcadeShooting g_parsedArc;
//Settings g_settings;

const static char* levelVariables[] = {
	"GS_NONE",
	"GS_SCTEXT",
	"GS_AMBIENT",
	"GS_MUSIC",
	"GS_VOLUME",
	"GS_MOUSESPEED",
	"GS_MOUSEON",
	"GS_LEVELCOMPLETE",
	"GS_LEVELWON",
	"GS_CHEATS",
	"GS_SWITCH0",
	"GS_SWITCH1",
	"GS_SWITCH2",
	"GS_SWITCH3",
	"GS_SWITCH4",
	"GS_SWITCH5",
	"GS_SWITCH6",
	"GS_SWITCH7",
	"GS_SWITCH8",
	"GS_SWITCH9",
	"GS_SWITCH10",
	"GS_SWITCH11",
	"GS_SWITCH12",
	"GS_COMBATJSON",
	"GS_COMBATLEVEL",
	"GS_PUZZLELEVEL",
	NULL
};

extern int parse_mis(const char *);
extern int parse_arc(const char *);

HypnoEngine::HypnoEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr), _videoDecoder(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0), 
	  _nextHotsToAdd(nullptr), _nextHotsToRemove(nullptr),
	  _screenW(640), _screenH(480) {
	_rnd = new Common::RandomSource("hypno");
}

HypnoEngine::~HypnoEngine() {
	// Dispose your resources
	delete _rnd;
}

void HypnoEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 10);
}

void HypnoEngine::loadMis(Common::String filename) {
    filename = convertPath(filename);
	Common::File *test = new Common::File();
	assert(isDemo());
    assert(test->open(filename.c_str()));

	const uint32 fileSize = test->size();
	char *buf = (char *)malloc(fileSize + 1);
	test->read(buf, fileSize);
	buf[fileSize] = '\0';
	parse_mis(buf);
	Level level;
	level.hots = *g_parsedHots; 
	_levels[filename] = level;
	debug("Loaded hots size: %d", g_parsedHots->size());
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
			if (cont && it[i] != 0x96 && it[i] != 0x0) {
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

void HypnoEngine::resetLevelState() {
	uint32 i = 0;
	while (levelVariables[i]) {
		_levelState[levelVariables[i]] = 0;
		i++;
	}
}

bool HypnoEngine::checkLevelCompleted() {
	return _levelState["GS_LEVELCOMPLETE"];
}

void HypnoEngine::loadAssets() {

	
	LibData files = loadLib("C_MISC/MISSIONS.LIB");
	uint32 i = 0;
	uint32 j = 0;
	uint32 k = 0;

	Common::String mis;

	debug("file: %s",files.filenames[j].c_str());
	for (i = 0; i < files.data.size(); i++) {
		mis += files.data[i];
		//debugN("%x/%c, ", files.data[i], files.data[i]);
		if (files.data[i] == 'X') {
			break;
			//if (j == files.filenames.size()-1) {
			//	debug("Finished at %d from %d", i, files.data.size()); 
			//	for (k = i; k < files.data.size(); k++)
			//		debugN("%c", files.data[k]);
			//	break;
			//}

			//j++;
			debugN("\n************file: %s**************",files.filenames[j].c_str());
		}
	}
	parse_arc(mis.c_str());
	{ 
		Level level;
		level.arcade = g_parsedArc;  
		_levels[files.filenames[0]] = level;
		debug("%s", level.arcade.background.c_str());
	}

	{ // quit level
		Hotspot q;
		q.type = MakeMenu;
		Action *a = new Quit();
		q.actions.push_back(a);
		Level level;
		Hotspots quit;
		quit.push_back(q);
		level.hots = quit;  
		_levels["mis/quit.mis"] = level;
	}

	// Read assets from mis files
	loadMis("mis/demo.mis");
	_levels["mis/demo.mis"].intros.push_back("demo/dcine1.smk");
	_levels["mis/demo.mis"].intros.push_back("demo/dcine2.smk");
	_levels["mis/demo.mis"].hots[1].setting = "C1.MI_";
	_levels["mis/demo.mis"].hots[2].setting = "mis/alley.mis";
	_levels["mis/demo.mis"].hots[5].setting = "mis/order.mis";


	loadMis("mis/order.mis");
	_levels["mis/order.mis"].hots[1].setting = "mis/quit.mis";
	loadMis("mis/alley.mis");
	_levels["mis/alley.mis"].intros.push_back("demo/aleyc01s.smk");

	//loadMis("mis/shoctalk.mis");

}

Common::Error HypnoEngine::run() {
	_language = Common::parseLanguage(ConfMan.get("language"));
	_platform = Common::parsePlatform(ConfMan.get("platform"));

	//return Common::kNoError;

	// Initialize graphics
	initGraphics(_screenW, _screenH, nullptr);
	_pixelFormat = g_system->getScreenFormat();
	if (_pixelFormat == Graphics::PixelFormat::createFormatCLUT8())
		return Common::kUnsupportedColorMode;

	_transparentColor = _pixelFormat.RGBToColor(0, 0x82, 0);
	screenRect = Common::Rect(0, 0, _screenW, _screenH);
	//changeCursor("default");
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
	loadAssets();
	_nextSetting = "mis/demo.mis";
	while (!shouldQuit()) {
		resetLevelState();
		if (!_nextSetting.empty()) {
			debug("Executing setting %s", _nextSetting.c_str());
			_currentSetting = _nextSetting;
			_nextSetting = "";
			runLevel(_currentSetting);
		}
			
	}
	return Common::kNoError;
}

void HypnoEngine::runLevel(Common::String name) {
	assert(_levels.contains(name));
	if (_levels[name].hots.size() == 0)
		runArcade(_levels[name].arcade);
	else
		runScene(_levels[name].hots, _levels[name].intros);

}

void HypnoEngine::runArcade(ArcadeShooting arc) {
	Common::Event event;
	Common::Point mousePos;
	
	_nextMoviesToPlay.push_back(arc.background);
	_nextMoviesPositions.push_back(Common::Point(0, 0));
	_nextMoviesScales.push_back(true);
	
	changeCursor("mouse/cursor1.smk", 0);

	while (!shouldQuit()) {
		
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				//if (!_nextHotsToAdd || !_nextHotsToRemove)
				// 	clickedHotspot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				break;

			default:
				break;
			}
		}

		// Movies
		if (_nextMoviesToPlay.size() > 0 && _currentMovie.empty()) {
			debug("start playing %s", _nextMoviesToPlay.front().c_str());
			//removeTimer();
			assert(_nextMoviesToPlay.size() == _nextMoviesPositions.size());
			_videoDecoder = new Video::SmackerDecoder();
			_currentMovie = _nextMoviesToPlay.front();
			_moviePosition = _nextMoviesPositions.front();
			_movieScale = _nextMoviesScales.front();
			playVideo(_currentMovie);
			_nextMoviesToPlay.pop_front();
			_nextMoviesPositions.pop_front();
			_nextMoviesScales.pop_front();
			continue;
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			if (_videoDecoder->getCurFrame() == 0)
				stopSound(true);
			if (_videoDecoder->endOfVideo()) {
				debug("video still playing");
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
				_currentMovie = "";

			} else if (_videoDecoder->needsUpdate()) {
				debug("updating screen");
				drawScreen();
				g_system->delayMillis(10);
			}
			continue;
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void HypnoEngine::runScene(Hotspots hots, Movies intros) {
	Common::Event event;
	Common::Point mousePos;
	
	stack.clear();
	_nextHotsToAdd = &hots;
	_nextMoviesToPlay = intros;
	for (uint32 i = 0; i < _nextMoviesToPlay.size(); i++) {
		_nextMoviesPositions.push_back(Common::Point(0, 0));
		_nextMoviesScales.push_back(true);
	}
	
	changeCursor("mouse/cursor1.smk", 0);

	while (!shouldQuit() && _nextSetting.empty()) {
		
		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE && _videoDecoder) {
					skipVideo();
					if (!stack.empty()) { 
						runMenu(*stack.back());
						drawScreen();
					}
				}

				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (!_nextHotsToAdd || !_nextHotsToRemove)
				 	clickedHotspot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				// Reset cursor to default
				//changeCursor("default");
				// The following functions will return true
				// if the cursor is changed
				if (hoverHotspot(mousePos)) {
				} else
					changeCursor("mouse/cursor1.smk", 0);
				break;

			default:
				break;
			}
		}

		// Movies
		if (_nextMoviesToPlay.size() > 0 && _currentMovie.empty()) {
			debug("start playing %s", _nextMoviesToPlay.front().c_str());
			//removeTimer();
			assert(_nextMoviesToPlay.size() == _nextMoviesPositions.size());
			_videoDecoder = new Video::SmackerDecoder();
			_currentMovie = _nextMoviesToPlay.front();
			_moviePosition = _nextMoviesPositions.front();
			_movieScale = _nextMoviesScales.front();
			playVideo(_currentMovie);
			_nextMoviesToPlay.pop_front();
			_nextMoviesPositions.pop_front();
			_nextMoviesScales.pop_front();
			continue;
		}

		if (_videoDecoder && !_videoDecoder->isPaused()) {
			debug("video decoder active!");
			if (_videoDecoder->getCurFrame() == 0)
				stopSound(true);
			if (_videoDecoder->endOfVideo()) {
				debug("video still playing");
				_videoDecoder->close();
				delete _videoDecoder;
				_videoDecoder = nullptr;
				_currentMovie = "";

				// refresh current scene
				if (!stack.empty()) { 
					runMenu(*stack.back());
					drawScreen();
				}

			} else if (_videoDecoder->needsUpdate()) {
				debug("updating screen");
				drawScreen();
				g_system->delayMillis(10);
			}
			continue;
		}

		if (_nextHotsToRemove) {
			debug("Removing a hotspot list!");
			stack.pop_back();
			runMenu(*stack.back());
			_nextHotsToRemove = NULL;
			drawScreen();
		} else if (_nextHotsToAdd) {
			debug("Adding a hotspot list!");
			//clearAreas();
			stack.push_back(_nextHotsToAdd);
			runMenu(*stack.back());
			_nextHotsToAdd = NULL;
			drawScreen();
		}

		if (checkLevelCompleted())
			_nextSetting = "mis/demo.mis";

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

//Actions

void HypnoEngine::runMenu(Hotspots hs) {
	const Hotspot h = *hs.begin();
	assert(h.type == MakeMenu);

	debug("hotspot actions size: %d", h.actions.size());
	for (Actions::const_iterator itt = h.actions.begin(); itt != h.actions.end(); ++itt) {
		Action *action = *itt;
		if (typeid(*action) == typeid(Quit))
			runQuit(h, (Quit*) action);
		else if (typeid(*action) == typeid(Background))
			runBackground(h, (Background*) action);
		else if (typeid(*action) == typeid(Overlay))
			runOverlay(h, (Overlay*) action);
		//else if (typeid(*action) == typeid(Mice))
		//	runMice(h, (Mice*) action);
	}

	//if (h.stype == "SINGLE_RUN")
	//	loadImage("int_main/mainbutt.smk", 0, 0);
	if (h.stype == "AUTO_BUTTONS")
		loadImage("int_main/resume.smk", 0, 0);
}

void HypnoEngine::runBackground(const Hotspot h, Background *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y);
}

void HypnoEngine::runOverlay(const Hotspot h, Overlay *a) {
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y);
}

void HypnoEngine::runMice(const Hotspot h, Mice *a) {
    changeCursor(a->path, a->index);
}

void HypnoEngine::runEscape(const Hotspot h, Escape *a) {
    _nextHotsToRemove = stack.back();
}

void HypnoEngine::runCutscene(const Hotspot h, Cutscene *a) {
    _nextMoviesToPlay.push_back(a->path);
	_nextMoviesPositions.push_back(Common::Point(0, 0));
	_nextMoviesScales.push_back(true);
}

void HypnoEngine::runGlobal(const Hotspot h, Global *a) {
    if (a->command == "TURNON")
		_levelState[a->variable] = 1;
	else if (a->command == "TURNOFF")
		_levelState[a->variable] = 0;
	else
		error("Invalid command %s", a->command.c_str());
}

void HypnoEngine::runPlay(const Hotspot h, Play *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;

	if (a->flag == "BITMAP")
			loadImage(a->path, origin.x, origin.y);
	else { 
    	_nextMoviesToPlay.push_back(a->path);
		_nextMoviesPositions.push_back(origin);
		_nextMoviesScales.push_back(false);
	}
}

void HypnoEngine::runWalN(const Hotspot h, WalN *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;
	if (a->flag == "BITMAP")
			loadImage(a->path, origin.x, origin.y);
	else { 
    	_nextMoviesToPlay.push_back(a->path);
		_nextMoviesPositions.push_back(origin);
		_nextMoviesScales.push_back(false);
	}
}

void HypnoEngine::runQuit(const Hotspot h, Quit *a) {
    quitGame();
}

// Hotspots

void HypnoEngine::clickedHotspot(Common::Point mousePos) {
	debug("clicked in %d %d", mousePos.x, mousePos.y);
	Hotspots *hots = stack.back();
	Hotspot selected;
	bool found = false;
	int rs = 100000000;
	int cs = 0;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		cs = h.rect.width() * h.rect.height();
		if (h.rect.contains(mousePos)) {
			if (cs < rs) {
				selected = h;
				found = true;
				rs = cs;
			}
		}
	}
	if (found) {
		//debug("Hotspot found! %x", selected.smenu);
		if (selected.smenu) {
			debug("SMenu found!");
			assert(selected.smenu->size() > 0);
			_nextHotsToAdd = selected.smenu;
		}

		if (!selected.setting.empty())
			_nextSetting = selected.setting;

		debug("hotspot clicked actions size: %d", selected.actions.size());
		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(Escape))
				runEscape(selected, (Escape*) action);
			if (typeid(*action) == typeid(Cutscene))
				runCutscene(selected, (Cutscene*) action);
			if (typeid(*action) == typeid(Play))
				runPlay(selected, (Play*) action);
			if (typeid(*action) == typeid(WalN))
				runWalN(selected, (WalN*) action);
			if (typeid(*action) == typeid(Global))
				runGlobal(selected, (Global*) action);
		}

	}
}

bool HypnoEngine::hoverHotspot(Common::Point mousePos) {
	if (stack.empty())
		return false;

	Hotspots *hots = stack.back();
	Hotspot selected;
	bool found = false;
	int rs = 100000000;
	int cs = 0;
	for (Hotspots::const_iterator it = hots->begin(); it != hots->end(); ++it) {
		const Hotspot h = *it;
		if (h.type != MakeHotspot)
			continue;

		cs = h.rect.width() * h.rect.height();
		if (h.rect.contains(mousePos)) {
			if (cs < rs) {
				selected = h;
				found = true;
				rs = cs;
			}
		}
	}
	if (found) {
		debug("Hovered over %d %d %d %d!", selected.rect.left, selected.rect.top, selected.rect.bottom, selected.rect.right);

		//debug("hotspot actions size: %d", h.actions.size());
		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(Mice))
				runMice(selected, (Mice*) action);
		}
		return true;
	}
	return false;
}

void HypnoEngine::loadImage(const Common::String &name, int x, int y) {
	Graphics::Surface *surf = decodeFrame(name, 0);
	_compositeSurface->transBlitFrom(*surf, Common::Point(x, y), _transparentColor);
}

Graphics::Surface *HypnoEngine::decodeFrame(const Common::String &name, int n, bool convert) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	Video::SmackerDecoder vd;
	if (!vd.loadStream(file))
		error("unable to load video %s", path.c_str());

    for(int f = 0; f < n; f++) 
		vd.decodeNextFrame();

	const Graphics::Surface *frame = vd.decodeNextFrame();
	Graphics::Surface *rframe;

	if (convert) {
		rframe = frame->convertTo(_pixelFormat, vd.getPalette());
	} else {
		rframe = frame->convertTo(frame->format, vd.getPalette());
		//rframe->create(frame->w, frame->h, frame->format);
		//rframe->copyRectToSurface(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
	}

	return rframe;
}


void HypnoEngine::drawScreen() {
	Graphics::ManagedSurface *surface = _compositeSurface;

	if (_videoDecoder && !_videoDecoder->isPaused()) {
		const Graphics::Surface *frame = _videoDecoder->decodeNextFrame();
		Graphics::Surface *sframe, *cframe;

		if (_movieScale) {
			sframe = frame->scale(_screenW, _screenH);
			cframe = sframe->convertTo(_pixelFormat, _videoDecoder->getPalette());
		} else
			cframe = frame->convertTo(_pixelFormat, _videoDecoder->getPalette());

		debug("Move position: %d %d", _moviePosition.x, _moviePosition.y);
		surface->blitFrom(*cframe, _moviePosition);

		if (_movieScale) { 
			sframe->free();
			delete sframe;
		}

		cframe->free();
		delete cframe;
	}

	g_system->copyRectToScreen(surface->getPixels(), surface->pitch, 0, 0, _screenW, _screenH);
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
