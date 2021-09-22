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
#include "engines/util.h"
#include "image/bmp.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

Hotspots *g_parsedHots;
ArcadeShooting g_parsedArc;
HypnoEngine *g_hypno;

MVideo::MVideo(Common::String _path, Common::Point _position, bool _transparent, bool _scaled, bool _loop) {
	path = _path;
	position = _position;
	scaled = _scaled;
	transparent = _transparent;
	loop = _loop;
}

HypnoEngine::HypnoEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0),
	  _nextHotsToAdd(nullptr), _nextHotsToRemove(nullptr), _font(nullptr),
	  _screenW(640), _screenH(480) {
	_rnd = new Common::RandomSource("hypno");

	g_hypno = this;
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
	// Deallocate actions
	for (Levels::iterator it = _levels.begin(); it != _levels.end(); ++it) {
		Level level = (*it)._value;
		for (Hotspots::iterator itt = level.scene.hots.begin(); itt != level.scene.hots.end(); ++itt) {
			Hotspot hot = *itt;
			for (Actions::iterator ittt = hot.actions.begin(); ittt != hot.actions.end(); ++ittt)
				delete (*ittt);
		}
	}

	delete _rnd;
	_compositeSurface->free();
	delete _compositeSurface;
}

void HypnoEngine::initializePath(const Common::FSNode &gamePath) {
	SearchMan.addDirectory(gamePath.getPath(), gamePath, 0, 10);
}

LibFile *HypnoEngine::loadLib(const Filename &prefix, const Filename &filename, bool encrypted) {
	LibFile *lib = new LibFile();
	SearchMan.add(filename, (Common::Archive *)lib, 0, true);
	if (!lib->open(prefix, filename, encrypted)) {
		return nullptr;
	}
	_archive.push_back(lib);
	return lib;
}

void HypnoEngine::loadAssets() { error("Function \"%s\" not implemented", __FUNCTION__); }

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
	loadAssets();
	_nextLevel = "<start>";
	while (!shouldQuit()) {
		_defaultCursor = "";
		_prefixDir = "";
		_videosPlaying.clear();
		if (!_nextLevel.empty()) {
			_currentLevel = _nextLevel;
			_nextLevel = "";
			runLevel(_currentLevel);
		}
	}
	return Common::kNoError;
}

void HypnoEngine::runLevel(Common::String &name) {
	if (!_levels.contains(name))
		error("Level %s cannot be found", name.c_str());
	stopSound();
	_music.clear();

	disableCursor();

	if (!_levels[name].trans.level.empty()) {
		debugC(1, kHypnoDebugScene, "Executing transition level %s", name.c_str());
		runTransition(_levels[name].trans);

	} else if (!_levels[name].arcade.background.empty()) {
		debugC(1, kHypnoDebugArcade, "Executing arcade level %s", name.c_str());
		_prefixDir = _levels[name].arcade.prefix;
		if (!_levels[name].arcade.intro.empty()) {
			MVideo v(_levels[name].arcade.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		changeScreenMode("arcade");
		runArcade(_levels[name].arcade);
	} else if (!_levels[name].puzzle.name.empty()) {
		debugC(1, kHypnoDebugScene, "Executing puzzle level %s", name.c_str());
		if (!_levels[name].arcade.intro.empty()) {
			MVideo v(_levels[name].arcade.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		runPuzzle(_levels[name].puzzle);
	} else {
		debugC(1, kHypnoDebugScene, "Executing scene level %s", name.c_str());
		resetSceneState();
		_prefixDir = _levels[name].scene.prefix;
		if (!_levels[name].scene.intro.empty()) {
			MVideo v(_levels[name].scene.intro, Common::Point(0, 0), false, true, false);
			runIntro(v);
		}
		changeScreenMode("scene");
		runScene(_levels[name].scene);
	}
}

void HypnoEngine::runIntro(MVideo &video) {
	Common::Event event;
	stopSound();
	disableCursor();
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

void HypnoEngine::runPuzzle(Puzzle puzzle) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::showCredits() { error("Function \"%s\" not implemented", __FUNCTION__); }

void HypnoEngine::loadImage(const Common::String &name, int x, int y, bool transparent) {
	debugC(1, kHypnoDebugMedia, "%s(%s, %d, %d, %d)", __FUNCTION__, name.c_str(), x, y, transparent);
	Graphics::Surface *surf = decodeFrame(name, 0);
	drawImage(*surf, x, y, transparent);
}

void HypnoEngine::drawImage(Graphics::Surface &surf, int x, int y, bool transparent) {
	if (transparent) {
		_compositeSurface->transBlitFrom(surf, Common::Point(x, y), surf.getPixel(0, 0));
	} else
		_compositeSurface->blitFrom(surf, Common::Point(x, y));
}

Common::File *HypnoEngine::fixSmackerHeader(Common::File *file) {
	Common::String magic;
	magic += file->readByte();
	magic += file->readByte();
	magic += file->readByte();
	magic += file->readByte();

	if (magic == "HYP2") {
		ByteArray *data = new ByteArray();
		data->push_back('S');
		data->push_back('M');
		data->push_back('K');
		data->push_back('2');
		while (!file->eos()) {
			data->push_back(file->readByte());
		}
		file->close();
		delete file;
		file = (Common::File *) new Common::MemoryReadStream(data->data(), data->size());
	} else 
		file->seek(0);

	return file;
}

Graphics::Surface *HypnoEngine::decodeFrame(const Common::String &name, int n, bool convert) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	file = fixSmackerHeader(file);

	Video::SmackerDecoder vd;
	if (!vd.loadStream(file))
		error("unable to load video %s", path.c_str());

	for (int f = 0; f < n; f++)
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

Frames HypnoEngine::decodeFrames(const Common::String &name) {
	Frames frames;
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	file = fixSmackerHeader(file);

	Video::SmackerDecoder vd;
	if (!vd.loadStream(file))
		error("unable to load video %s", path.c_str());

	const Graphics::Surface *frame = nullptr;
	Graphics::Surface *rframe = nullptr;

	while (!vd.endOfVideo()) {
		frame = vd.decodeNextFrame();
		rframe = frame->convertTo(_pixelFormat, vd.getPalette());
		frames.push_back(rframe);
	}
	return frames;
}

void HypnoEngine::changeScreenMode(const Common::String &mode) {
	debugC(1, kHypnoDebugMedia, "%s(%s)", __FUNCTION__, mode.c_str());
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
	debugC(1, kHypnoDebugMedia, "%s(%s)", __FUNCTION__, video.path.c_str());
	Common::File *file = new Common::File();
	Common::String path = convertPath(video.path);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	file = fixSmackerHeader(file);

	if (video.decoder != nullptr)
		error("Video %s was not previously closed and deallocated", video.path.c_str());

	video.decoder = new Video::SmackerDecoder();

	if (!video.decoder->loadStream(file))
		error("unable to load video %s", path.c_str());
	video.decoder->start();
}

void HypnoEngine::skipVideo(MVideo &video) {
	if (!video.decoder)
		return;
	debugC(1, kHypnoDebugMedia, "%s()", __FUNCTION__);
	video.decoder->close();
	delete video.decoder;
	video.decoder = nullptr;
}

// Sound handling

void HypnoEngine::playSound(const Common::String &filename, uint32 loops) {
	debugC(1, kHypnoDebugMedia, "%s(%s, %d)", __FUNCTION__, filename.c_str(), loops);
	Common::String name = convertPath(filename);
	if (!_prefixDir.empty())
		name = _prefixDir + "/" + name;

	Audio::LoopingAudioStream *stream = nullptr;
	Common::File *file = new Common::File();
	if (file->open(name)) {
		stream = new Audio::LoopingAudioStream(Audio::makeRawStream(file, 22050, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), loops);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
	}
}

void HypnoEngine::stopSound() {
	debugC(1, kHypnoDebugMedia, "%s()", __FUNCTION__);
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

// Timers
static void timerCallback(void *refCon) {
	g_hypno->removeTimer();
	Common::String *level = (Common::String *)refCon;
	g_hypno->_nextLevel = *level;
	delete level;
}

bool HypnoEngine::installTimer(uint32 delay, Common::String *ns) {
	return g_system->getTimerManager()->installTimerProc(&timerCallback, delay, (void *)ns, "timerCallback");
}

void HypnoEngine::removeTimer() {
	g_system->getTimerManager()->removeTimerProc(&timerCallback);
}

} // End of namespace Hypno

