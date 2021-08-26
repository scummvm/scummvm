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
#include "audio/decoders/raw.h"
#include "common/archive.h"
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/file.h"
#include "common/memstream.h"
#include "common/savefile.h"
#include "common/str.h"
#include "common/system.h"
#include "common/timer.h"
#include "common/tokenizer.h"
#include "engines/util.h"
#include "image/bmp.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

Hotspots *g_parsedHots;
ArcadeShooting g_parsedArc;

MVideo::MVideo(Common::String _path, Common::Point _position, bool _transparent, bool _scaled, bool _loop) {
	path = _path;
	position = _position;
	scaled = _scaled;
	transparent = _transparent;
	loop = _loop;
}

const static char *levelVariables[] = {
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
	NULL};

extern int parse_mis(const char *);
extern int parse_arc(const char *);

HypnoEngine::HypnoEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0),
	  _nextHotsToAdd(nullptr), _nextHotsToRemove(nullptr),
	  _screenW(640), _screenH(480) {
	_rnd = new Common::RandomSource("hypno");

	_defaultCursor = "";
	// Add quit level
	Hotspot q;
	q.type = MakeMenu;
	Action *a = new Quit();
	q.actions.push_back(a);
	Level quit;
	Hotspots hs;
	hs.push_back(q);
	quit.scene.hots = hs;
	_levels["<quit>"] = quit;
}

HypnoEngine::~HypnoEngine() {
	// Dispose your resources
	delete _rnd;
}

void HypnoEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 10);
}

void HypnoEngine::parseScene(Common::String prefix, Common::String filename) {
	debug("Parsing %s", filename.c_str());
	filename = convertPath(filename);
	if (!prefix.empty())
		filename = prefix + "/" + filename;
	Common::File *test = new Common::File();
	assert(isDemo());
	assert(test->open(filename.c_str()));

	const uint32 fileSize = test->size();
	char *buf = (char *)malloc(fileSize + 1);
	test->read(buf, fileSize);
	buf[fileSize] = '\0';
	parse_mis(buf);
	Level level;
	level.scene.prefix = prefix;
	level.scene.hots = *g_parsedHots;
	_levels[filename] = level;
	debug("Loaded hots size: %d", g_parsedHots->size());
}

void HypnoEngine::loadLib(Common::String filename, LibData &r) {
	debug("Loading %s", filename.c_str());
	Common::File libfile;
	assert(libfile.open(filename));
	uint32 i = 0;
	Common::String entry = "<>";
	FileData f;
	f.data.push_back(0);
	byte b;
	uint32 start;
	uint32 size;
	uint32 pos;

	do {
		f.name = filename + "/";
		f.data.clear();
		for (i = 0; i < 12; i++) {
			b = libfile.readByte();
			if (b != 0x96 && b != 0x0)
				f.name += tolower(char(b));
		}
		debug("name: %s", f.name.c_str());
		start = libfile.readUint32LE();
		size = libfile.readUint32LE();
		debug("field: %x", libfile.readUint32LE());

		pos = libfile.pos();
		libfile.seek(start);

		for (i = 0; i < size; i++) {
			b = libfile.readByte();
			if (b != '\n')
				b = b ^ 0xfe;
			f.data.push_back(b);
		}
		debug("size: %d", f.data.size());
		libfile.seek(pos);
		if (size > 0)
			r.push_back(f);

	} while (size > 0);
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

ShootSequence HypnoEngine::parseShootList(Common::String name, Common::String data) {
	Common::StringTokenizer tok(data, " ,\t");

	Common::String t;
	Common::String n;
	ShootInfo si;
	ShootSequence seq;
	while (!tok.empty()) {
		t = tok.nextToken();
		if (t[0] == '\n')
			continue;
		n = tok.nextToken();
		//debug("t: %s, n: %s", t.c_str(), n.c_str());
		if (t == "Z")
			break;

		Common::replace(n, "\nS", "");
		Common::replace(n, "\nZ\n", "");
		si.name = n;
		si.timestamp = atoi(t.c_str());
		seq.push_back(si);
		debug("%d -> %s", si.timestamp, si.name.c_str());
	}
	return seq;
}

void HypnoEngine::parseArcadeShooting(Common::String prefix, Common::String name, Common::String data) {
	parse_arc(data.c_str());
	Level level;
	level.arcade = g_parsedArc;
	level.arcade.prefix = prefix;
	_levels[name] = level;
	g_parsedArc.background.clear();
	g_parsedArc.player.clear();
	g_parsedArc.shoots.clear();
}

void HypnoEngine::loadAssets() { error("not implemented"); }

Common::Error HypnoEngine::run() {
	_language = Common::parseLanguage(ConfMan.get("language"));
	_platform = Common::parsePlatform(ConfMan.get("platform"));

	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(640, 480));
	modes.push_back(Graphics::Mode(320, 200));
	initGraphicsModes(modes);

	// Initialize graphics
	initGraphics(_screenW, _screenH, nullptr);
	_pixelFormat = g_system->getScreenFormat();
	if (_pixelFormat == Graphics::PixelFormat::createFormatCLUT8())
		return Common::kUnsupportedColorMode;

	_compositeSurface = new Graphics::ManagedSurface();
	_compositeSurface->create(_screenW, _screenH, _pixelFormat);

	// Main event loop
	Common::Event event;
	Common::Point mousePos;
	/*int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	} else {
		_nextSetting = getGoIntroSetting();
	}*/
	loadAssets();
	_nextLevel = "<start>";
	while (!shouldQuit()) {
		_defaultCursor = "";
		_prefixDir = "";
		resetLevelState();
		_videosPlaying.clear();
		if (!_nextLevel.empty()) {
			debug("Executing level %s", _nextLevel.c_str());
			_currentLevel = _nextLevel;
			_nextLevel = "";
			runLevel(_currentLevel);
		}
	}
	return Common::kNoError;
}

void HypnoEngine::runLevel(Common::String name) {
	assert(_levels.contains(name));
	stopSound();
	_music = "";

	disableCursor();

	if (!_levels[name].trans.level.empty()) {
		_nextLevel = _levels[name].trans.level;
		for (Filenames::iterator it = _levels[name].trans.intros.begin(); it != _levels[name].trans.intros.end(); ++it) {
			MVideo v(*it, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}

	} else if (!_levels[name].arcade.background.empty()) {
		_prefixDir = _levels[name].arcade.prefix;
		if (!_levels[name].arcade.intro.empty()) {
			MVideo v(_levels[name].arcade.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		changeScreenMode("arcade");
		runArcade(_levels[name].arcade);
	} else if (!_levels[name].puzzle.name.empty()) {
		if (!_levels[name].arcade.intro.empty()) {
			MVideo v(_levels[name].arcade.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		runPuzzle(_levels[name].puzzle);
	} else {
		_prefixDir = _levels[name].scene.prefix;
		if (!_levels[name].scene.intro.empty()) {
			MVideo v(_levels[name].scene.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		changeScreenMode("scene");
		runScene(_levels[name].scene);
	}
}

void HypnoEngine::runScene(Scene scene) {
	_refreshConversation = false;
	_conversation.clear();
	Common::Event event;
	Common::Point mousePos;
	Common::List<uint32> videosToRemove;

	stack.clear();
	_nextHotsToAdd = &scene.hots;
	defaultCursor();

	while (!shouldQuit() && _nextLevel.empty()) {

		while (g_system->getEventManager()->pollEvent(event)) {
			mousePos = g_system->getEventManager()->getMousePos();
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {
						if (it->decoder)
							skipVideo(*it);
					}
					_videosPlaying.clear();

					if (!stack.empty()) {
						runMenu(*stack.back());
						drawScreen();
					}
				}

				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RETURN_TO_LAUNCHER:
				break;

			case Common::EVENT_RBUTTONDOWN:
				if (stack.empty())
					break;
				if (!_conversation.empty()) {
					rightClickedConversation(mousePos);
					break;
				}
				break;

			case Common::EVENT_LBUTTONDOWN:
				if (stack.empty())
					break;
				if (!_conversation.empty()) {
					leftClickedConversation(mousePos);
					break;
				}
				if (!_nextHotsToAdd || !_nextHotsToRemove)
					clickedHotspot(mousePos);
				break;

			case Common::EVENT_MOUSEMOVE:
				// Reset cursor to default
				//changeCursor("default");
				// The following functions will return true
				// if the cursor is changed
				if (!_conversation.empty())
					break;

				if (hoverHotspot(mousePos)) {
				} else
					defaultCursor();
				break;

			default:
				break;
			}
		}

		if (_refreshConversation && !_conversation.empty() && _nextSequentialVideoToPlay.empty()) {
			showConversation();
			drawScreen();
			_refreshConversation = false;
			_videosPlaying.clear();
		}

		// Movies
		if (_nextSequentialVideoToPlay.size() > 0 && _videosPlaying.empty()) {
			playVideo(*_nextSequentialVideoToPlay.begin());
			_videosPlaying.push_back(*_nextSequentialVideoToPlay.begin());
			_nextSequentialVideoToPlay.remove_at(0);
		}
		uint32 i = 0;
		videosToRemove.clear();
		for (Videos::iterator it = _videosPlaying.begin(); it != _videosPlaying.end(); ++it) {

			if (it->decoder) {
				if (it->decoder->endOfVideo()) {
					if (it->loop) {
						it->decoder->rewind();
						it->decoder->start();
					} else {
						it->decoder->close();
						delete it->decoder;
						it->decoder = nullptr;
						videosToRemove.push_back(i);
					}

				} else if (it->decoder->needsUpdate()) {
					updateScreen(*it);
				}
			}
			i++;
		}
		if (videosToRemove.size() > 0) {

			for (Common::List<uint32>::iterator it = videosToRemove.begin(); it != videosToRemove.end(); ++it) {
				debug("removing %d from %d size", *it, _videosPlaying.size());
				_videosPlaying.remove_at(*it);
			}

			// Nothing else to play
			if (_videosPlaying.size() == 0 && _nextSequentialVideoToPlay.size() == 0) {
				if (!_conversation.empty())
					_refreshConversation = true;
				else if (!stack.empty()) {
					runMenu(*stack.back());
					drawScreen();
				}
			}
		}

		if (_videosPlaying.size() > 0 || _nextSequentialVideoToPlay.size() > 0) {
			drawScreen();
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

		if (_music.empty() && !scene.sound.empty()) {
			_music = scene.sound;
			playSound(_music, 0);
		}

		if (checkLevelCompleted())
			_nextLevel = "mis/demo.mis";

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void HypnoEngine::runIntro(MVideo &video) {
	Common::Event event;
	stopSound();
	playVideo(video);

	while (!shouldQuit() && video.decoder) {
		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE) {
					skipVideo(video);
				}
				break;

			default:
				break;
			}
		}

		if (video.decoder) {
			if (video.decoder->endOfVideo()) {
				skipVideo(video);
			} else if (video.decoder->needsUpdate()) {
				updateScreen(video);
				drawScreen();
			}
		}

		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void HypnoEngine::runPuzzle(Puzzle puzzle) { error("Not implemented"); }

//Actions

void HypnoEngine::runMenu(Hotspots hs) {
	const Hotspot h = *hs.begin();
	assert(h.type == MakeMenu);

	debug("hotspot actions size: %d", h.actions.size());
	for (Actions::const_iterator itt = h.actions.begin(); itt != h.actions.end(); ++itt) {
		Action *action = *itt;
		if (typeid(*action) == typeid(Quit))
			runQuit((Quit *)action);
		else if (typeid(*action) == typeid(Background))
			runBackground((Background *)action);
		else if (typeid(*action) == typeid(Overlay))
			runOverlay((Overlay *)action);
		else if (typeid(*action) == typeid(Ambient))
			runAmbient((Ambient *)action);

		//else if (typeid(*action) == typeid(Mice))
		//	runMice(h, (Mice*) action);
	}

	//if (h.stype == "SINGLE_RUN")
	//	loadImage("int_main/mainbutt.smk", 0, 0);
	if (h.stype == "AUTO_BUTTONS" && _conversation.empty())
		loadImage("int_main/resume.smk", 0, 0, true);
}

void HypnoEngine::runBackground(Background *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y, false);
}

void HypnoEngine::runOverlay(Overlay *a) {
	Common::Point origin = a->origin;
	loadImage(a->path, origin.x, origin.y, false);
}

void HypnoEngine::runMice(Mice *a) {
	changeCursor(a->path, a->index);
}

void HypnoEngine::runEscape(Escape *a) {
	_nextHotsToRemove = stack.back();
}

void HypnoEngine::runCutscene(Cutscene *a) {
	stopSound();
	_music = "";
	_nextSequentialVideoToPlay.push_back(MVideo(a->path, Common::Point(0, 0), false, true, false));
}

void HypnoEngine::runGlobal(Global *a) {
	if (a->command == "TURNON")
		_levelState[a->variable] = 1;
	else if (a->command == "TURNOFF")
		_levelState[a->variable] = 0;
	else
		error("Invalid command %s", a->command.c_str());
}

void HypnoEngine::runPlay(Play *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;

	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runAmbient(Ambient *a) {
	Common::Point origin = a->origin;
	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, a->fullscreen, a->flag == "/LOOP"));
	}
}

void HypnoEngine::runWalN(WalN *a) {
	if (a->condition.size() > 0 && !_levelState[a->condition])
		return;
	Common::Point origin = a->origin;
	if (a->flag == "/BITMAP")
		loadImage(a->path, origin.x, origin.y, false);
	else {
		_nextSequentialVideoToPlay.push_back(MVideo(a->path, a->origin, false, false, false));
	}
}

void HypnoEngine::runQuit(Quit *a) {
	quitGame();
}

void HypnoEngine::runChangeLevel(ChangeLevel *a) {
	_nextLevel = a->level;
}

void HypnoEngine::runTalk(Talk *a) {
	debug("adding TALK line!");
	_conversation.push_back(a);
	_refreshConversation = true;
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

		debug("hotspot clicked actions size: %d", selected.actions.size());
		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(ChangeLevel))
				runChangeLevel((ChangeLevel *)action);
			if (typeid(*action) == typeid(Escape))
				runEscape((Escape *)action);
			else if (typeid(*action) == typeid(Cutscene))
				runCutscene((Cutscene *)action);
			else if (typeid(*action) == typeid(Play))
				runPlay((Play *)action);
			else if (typeid(*action) == typeid(WalN))
				runWalN((WalN *)action);
			else if (typeid(*action) == typeid(Global))
				runGlobal((Global *)action);
			else if (typeid(*action) == typeid(Talk))
				runTalk((Talk *)action);
			else if (typeid(*action) == typeid(Quit))
				runQuit((Quit *)action);
			else if (typeid(*action) == typeid(Palette))
				debug("runPalette unimplemented");
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
		//debug("Hovered over %d %d %d %d!", selected.rect.left, selected.rect.top, selected.rect.bottom, selected.rect.right);

		//debug("hotspot actions size: %d", h.actions.size());
		for (Actions::const_iterator itt = selected.actions.begin(); itt != selected.actions.end(); ++itt) {
			Action *action = *itt;
			if (typeid(*action) == typeid(Mice))
				runMice((Mice *)action);
		}
		return true;
	}
	return false;
}

void HypnoEngine::loadImage(const Common::String &name, int x, int y, bool transparent) {
	Graphics::Surface *surf = decodeFrame(name, 0);
	drawImage(*surf, x, y, transparent);
}

void HypnoEngine::drawImage(Graphics::Surface &surf, int x, int y, bool transparent) {
	if (transparent) {
		_compositeSurface->transBlitFrom(surf, Common::Point(x, y), surf.getPixel(0, 0));
	}
	else
		_compositeSurface->blitFrom(surf, Common::Point(x, y));
}

Graphics::Surface *HypnoEngine::decodeFrame(const Common::String &name, int n, bool convert) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	Video::SmackerDecoder vd;
	if (!vd.loadStream(file))
		error("unable to load video %s", path.c_str());

	//debug("decoding %s with %d frames", path.c_str(), vd.getFrameCount());

	for (int f = 0; f < n; f++)
		vd.decodeNextFrame();

	const Graphics::Surface *frame = vd.decodeNextFrame();
	Graphics::Surface *rframe;
	//debug("decoding: %s %d %d", name.c_str(), frame->h, frame->w);
	if (convert) {
		rframe = frame->convertTo(_pixelFormat, vd.getPalette());
	} else {
		rframe = frame->convertTo(frame->format, vd.getPalette());
		//rframe->create(frame->w, frame->h, frame->format);
		//rframe->copyRectToSurface(frame->getPixels(), frame->pitch, 0, 0, frame->w, frame->h);
	}

	return rframe;
}

void HypnoEngine::changeScreenMode(Common::String mode) {
	if (mode == "scene") {
		_screenW = 640;
		_screenH = 480;

		initGraphics(_screenW, _screenH, nullptr);

		_compositeSurface->free();
		delete _compositeSurface;

		_compositeSurface = new Graphics::ManagedSurface();
		_compositeSurface->create(_screenW, _screenH, _pixelFormat);

		_transparentColor = _pixelFormat.RGBToColor(0, 0x82, 0);
		_compositeSurface->setTransparentColor(_transparentColor);

	} else if (mode == "arcade") {
		_screenW = 320;
		_screenH = 200;

		initGraphics(_screenW, _screenH, nullptr);

		_compositeSurface->free();
		delete _compositeSurface;

		_compositeSurface = new Graphics::ManagedSurface();
		_compositeSurface->create(_screenW, _screenH, _pixelFormat);

		_transparentColor = _pixelFormat.RGBToColor(0, 0, 0);
		_compositeSurface->setTransparentColor(_transparentColor);
	} else
		error("Unknown screen mode %s", mode.c_str());
}

void HypnoEngine::updateScreen(MVideo &video) {
	const Graphics::Surface *frame = video.decoder->decodeNextFrame();
	video.currentFrame = frame;
	if (frame->h == 0 || frame->w == 0 || video.decoder->getPalette() == nullptr)
		return;

	Graphics::Surface *sframe, *cframe;

	if (video.scaled) {
		sframe = frame->scale(_screenW, _screenH);
		cframe = sframe->convertTo(_pixelFormat, video.decoder->getPalette());
	} else
		cframe = frame->convertTo(_pixelFormat, video.decoder->getPalette());

	if (video.transparent)
		_compositeSurface->transBlitFrom(*cframe, video.position, _transparentColor);
	else
		_compositeSurface->blitFrom(*cframe, video.position);

	if (video.scaled) {
		sframe->free();
		delete sframe;
	}

	cframe->free();
	delete cframe;
}

void HypnoEngine::drawScreen() {
	g_system->copyRectToScreen(_compositeSurface->getPixels(), _compositeSurface->pitch, 0, 0, _screenW, _screenH);
	g_system->updateScreen();
}

// Video handling

void HypnoEngine::playVideo(MVideo &video) {
	//debugC(1, kPrivateDebugFunction, "%s(%s)", __FUNCTION__, name.c_str());
	debug("video.path: %s", video.path.c_str());
	Common::File *file = new Common::File();
	Common::String path = convertPath(video.path);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	assert(video.decoder == nullptr);
	video.decoder = new Video::SmackerDecoder();

	if (!video.decoder->loadStream(file))
		error("unable to load video %s", path.c_str());
	video.decoder->start();
}

void HypnoEngine::skipVideo(MVideo &video) {
	video.decoder->close();
	delete video.decoder;
	video.decoder = nullptr;
	//_currentMovie = "";
}

// Sound handling

void HypnoEngine::playSound(Common::String name, uint32 loops) {

	name = convertPath(name);
	if (!_prefixDir.empty())
		name = _prefixDir + "/" + name;
	debug("trying to play %s", name.c_str());
	ByteArray *raw;
	Audio::LoopingAudioStream *stream;

	for (LibData::iterator it = _soundFiles.begin(); it != _soundFiles.end(); ++it) {
		if (it->name == name) {
			raw = new ByteArray(it->data);
			stream = new Audio::LoopingAudioStream(Audio::makeRawStream(raw->data(), raw->size(), 22050, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), loops);
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
			break;
		}
	}
}

void HypnoEngine::stopSound() {
	debugC(1, kHypnoDebugFunction, "%s()", __FUNCTION__);
	_mixer->stopHandle(_soundHandle);
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
