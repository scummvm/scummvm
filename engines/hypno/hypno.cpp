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
#include "engines/advancedDetector.h"
#include "engines/util.h"
#include "image/bmp.h"

#include "hypno/grammar.h"
#include "hypno/hypno.h"

namespace Hypno {

Hotspots *g_parsedHots;
ArcadeShooting *g_parsedArc;
HypnoEngine *g_hypno;

HypnoEngine::HypnoEngine(OSystem *syst, const ADGameDescription *gd)
	: Engine(syst), _gameDescription(gd), _image(nullptr),
	  _compositeSurface(nullptr), _transparentColor(0),
	  _nextHotsToAdd(nullptr), _nextHotsToRemove(nullptr),
	  _levelId(0), _skipLevel(false), _health(0), _maxHealth(0),
	  _playerFrameIdx(0), _playerFrameSep(0), _refreshConversation(false),
	  _countdown(0), _timerStarted(false), _score(0), _lives(0),
	  _defaultCursor(""), _defaultCursorIdx(0),  _skipDefeatVideo(false),
	  _background(nullptr), _masks(nullptr), _musicRate(0),
	  _additionalVideo(nullptr), _ammo(0), _maxAmmo(0),
	  _doNotStopSounds(false), _screenW(0), _screenH(0) { // Every games initializes its own resolution
	_rnd = new Common::RandomSource("hypno");
	_checkpoint = "";

	if (gd->extra)
		_variant = gd->extra;
	else
		_variant = "FullGame";
	g_hypno = this;
	g_parsedArc = new ArcadeShooting();
	_language = Common::parseLanguage(ConfMan.get("language"));
	_platform = Common::parsePlatform(ConfMan.get("platform"));
	if (!Common::parseBool(ConfMan.get("cheats"), _cheatsEnabled))
		error("Failed to parse bool from cheats options");

	if (!Common::parseBool(ConfMan.get("infiniteHealth"), _infiniteHealthCheat))
		error("Failed to parse bool from cheats options");

	if (!Common::parseBool(ConfMan.get("infiniteAmmo"), _infiniteAmmoCheat))
		error("Failed to parse bool from cheats options");

	if (!Common::parseBool(ConfMan.get("restored"), _restoredContentEnabled))
		error("Failed to parse bool from restored options");
	// Add quit level
	Hotspot q(MakeMenu);
	Action *a = new Quit();
	q.actions.push_back(a);
	Scene *quit = new Scene();
	Hotspots hs;
	hs.push_back(q);
	quit->hots = hs;
	_levels["<quit>"] = quit;
	resetStatistics();
}

HypnoEngine::~HypnoEngine() {
	// Deallocate actions
	// for (Levels::iterator it = _levels.begin(); it != _levels.end(); ++it) {
	// 	Level level = (*it)._value;
	// 	for (Hotspots::iterator itt = level.scene.hots.begin(); itt != level.scene.hots.end(); ++itt) {
	// 		Hotspot hot = *itt;
	// 		for (Actions::iterator ittt = hot.actions.begin(); ittt != hot.actions.end(); ++ittt)
	// 			delete (*ittt);
	// 	}
	// }

	delete _rnd;
	_compositeSurface->free();
	delete _compositeSurface;

	delete g_parsedArc;
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
Common::String HypnoEngine::findNextLevel(const Common::String &level) { error("Function \"%s\" not implemented", __FUNCTION__); }

Common::Error HypnoEngine::run() {
	Graphics::ModeList modes;
	modes.push_back(Graphics::Mode(640, 480));
	modes.push_back(Graphics::Mode(320, 200));
	initGraphicsModes(modes);

	// Initialize graphics
	_pixelFormat = Graphics::PixelFormat::createFormatCLUT8();
	initGraphics(_screenW, _screenH, &_pixelFormat);

	_compositeSurface = new Graphics::ManagedSurface();
	_compositeSurface->create(_screenW, _screenH, _pixelFormat);

	// Main event loop
	loadAssets();

	int saveSlot = ConfMan.getInt("save_slot");
	if (saveSlot >= 0) { // load the savegame
		loadGameState(saveSlot);
	}

	assert(!_nextLevel.empty());
	while (!shouldQuit()) {
		debug("nextLevel: %s", _nextLevel.c_str());
		_prefixDir = "";
		_videosPlaying.clear();
		_videosLooping.clear();
		if (!_nextLevel.empty()) {
			_currentLevel = findNextLevel(_nextLevel);
			_nextLevel = "";
			_arcadeMode = "";
			runLevel(_currentLevel);
		} else
			g_system->delayMillis(300);
	}
	return Common::kNoError;
}

void HypnoEngine::runLevel(Common::String &name) {
	if (!_levels.contains(name))
		error("Level %s cannot be found", name.c_str());

	_prefixDir = _levels[name]->prefix;
	stopSound();
	_music.clear();

	// Play intros
	disableCursor();

	if (_levels[name]->playMusicDuringIntro && !_levels[name]->music.empty()) {
		playSound(_levels[name]->music, 0, _levels[name]->musicRate);
		_doNotStopSounds = true;
	}

	debug("Number of videos to play: %d", _levels[name]->intros.size());
	for (Filenames::iterator it = _levels[name]->intros.begin(); it != _levels[name]->intros.end(); ++it) {
		MVideo v(*it, Common::Point(0, 0), false, true, false);
		runIntro(v);
	}

	_doNotStopSounds = false;

	if (_levels[name]->type == TransitionLevel) {
		debugC(1, kHypnoDebugScene, "Executing transition level %s", name.c_str());
		runTransition((Transition *)_levels[name]);
	} else if (_levels[name]->type == ArcadeLevel) {
		debugC(1, kHypnoDebugArcade, "Executing arcade level %s", name.c_str());
		changeScreenMode("320x200");
		ArcadeShooting *arc = (ArcadeShooting *)_levels[name];
		runBeforeArcade(arc);
		runArcade(arc);
		runAfterArcade(arc);
	} else if (_levels[name]->type == CodeLevel) {
		debugC(1, kHypnoDebugScene, "Executing hardcoded level %s", name.c_str());
		// Resolution depends on the game
		runCode((Code *)_levels[name]);
	} else if (_levels[name]->type == SceneLevel) {
		debugC(1, kHypnoDebugScene, "Executing scene level %s with next level: %s", name.c_str(), _levels[name]->levelIfWin.c_str());
		runScene((Scene *)_levels[name]);
	} else {
		error("Invalid level %s", name.c_str());
	}
}

void HypnoEngine::runIntros(Videos &videos) {
	debugC(1, kHypnoDebugScene, "Starting run intros with %d videos!", videos.size());
	Common::Event event;
	if (!_doNotStopSounds)
		stopSound();
	bool skip = false;
	int clicked[3] = {-1, -1, -1};
	int clicks = 0;

	for (Videos::iterator it = videos.begin(); it != videos.end(); ++it) {
		playVideo(*it);
	}

	while (!shouldQuit()) {
		while (g_system->getEventManager()->pollEvent(event)) {
			// Events
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode == Common::KEYCODE_ESCAPE)
					skip = true;
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (videos.size() == 1) {
					int first = (clicks - 2) % 3;
					int last = clicks % 3;
					clicked[last] = videos[0].decoder->getCurFrame();
					if (clicks >= 2 && clicked[last] - clicked[first] <= 10)
						skip = true;
					clicks++;
				}
				break;

			default:
				break;
			}
		}
		if (skip) {
			for (Videos::iterator it = videos.begin(); it != videos.end(); ++it) {
				if (it->decoder)
					skipVideo(*it);
			}
			videos.clear();
		}

		bool playing = false;
		for (Videos::iterator it = videos.begin(); it != videos.end(); ++it) {
			assert(!it->loop);
			if (it->decoder) {
				if (it->decoder->endOfVideo()) {
					it->decoder->close();
					delete it->decoder;
					it->decoder = nullptr;
				} else {
					playing = true;
					if (it->decoder->needsUpdate()) {
						updateScreen(*it);
						drawScreen();
					}
				}
			}
		}
		if (!playing) {
			debugC(1, kHypnoDebugScene, "Not playing anymore!");
			break;
		}
		g_system->updateScreen();
		g_system->delayMillis(10);
	}
}

void HypnoEngine::runIntro(MVideo &video) {
	Videos tmp;
	tmp.push_back(video);
	runIntros(tmp);
}

void HypnoEngine::runCode(Code *code) { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::showCredits() { error("Function \"%s\" not implemented", __FUNCTION__); }
void HypnoEngine::loadGame(const Common::String &nextLevel, int score, int puzzleDifficulty, int combatDifficulty) {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void HypnoEngine::loadFonts() {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void HypnoEngine::drawString(const Filename &name, const Common::String &str, int x, int y, int w, uint32 c) {
	error("Function \"%s\" not implemented", __FUNCTION__);
}

void HypnoEngine::loadImage(const Common::String &name, int x, int y, bool transparent, bool palette, int frameNumber) {

	debugC(1, kHypnoDebugMedia, "%s(%s, %d, %d, %d)", __FUNCTION__, name.c_str(), x, y, transparent);
	Graphics::Surface *surf;
	if (palette) {
		byte *array;
		surf = decodeFrame(name, frameNumber, &array);
		loadPalette(array, 0, 256);
	} else
		surf = decodeFrame(name, frameNumber);

	drawImage(*surf, x, y, transparent);

	surf->free();
	delete surf;
}

void HypnoEngine::drawImage(Graphics::Surface &surf, int x, int y, bool transparent) {
	if (transparent) {
		_compositeSurface->transBlitFrom(surf, Common::Point(x, y), _transparentColor);
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
		uint32 size = file->size();
		byte *data = (byte *)malloc(size);
		file->seek(0);
		file->read(data, size);
		data[0] = 'S';
		data[1] = 'M';
		data[2] = 'K';
		file->close();
		delete file;
		file = (Common::File *)new Common::MemoryReadStream(data, size, DisposeAfterUse::YES);
	} else
		file->seek(0);

	return file;
}

Graphics::Surface *HypnoEngine::decodeFrame(const Common::String &name, int n, byte **palette) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(name);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find video file %s", path.c_str());

	file = fixSmackerHeader(file);

	HypnoSmackerDecoder vd;
	if (!vd.loadStream(file))
		error("unable to load video %s", path.c_str());

	for (int f = 0; f < n; f++)
		vd.decodeNextFrame();

	const Graphics::Surface *frame = vd.decodeNextFrame();
	Graphics::Surface *rframe = frame->convertTo(frame->format, vd.getPalette());
	if (palette != nullptr) {
		byte *newPalette = (byte *)malloc(3 * 256);
		memcpy(newPalette, vd.getPalette(), 3 * 256);
		*palette = newPalette;
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

	HypnoSmackerDecoder vd;
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
	if (mode == "640x480") {
		if (_screenW == 640 && _screenH == 480)
			return;

		_screenW = 640;
		_screenH = 480;

		initGraphics(_screenW, _screenH, &_pixelFormat);

		_compositeSurface->free();
		delete _compositeSurface;

		_compositeSurface = new Graphics::ManagedSurface();
		_compositeSurface->create(_screenW, _screenH, _pixelFormat);

		_compositeSurface->setTransparentColor(_transparentColor);

	} else if (mode == "320x200") {
		if (_screenW == 320 && _screenH == 200)
			return;

		_screenW = 320;
		_screenH = 200;

		initGraphics(_screenW, _screenH, &_pixelFormat);

		_compositeSurface->free();
		delete _compositeSurface;

		_compositeSurface = new Graphics::ManagedSurface();
		_compositeSurface->create(_screenW, _screenH, _pixelFormat);

		_compositeSurface->setTransparentColor(_transparentColor);
	} else
		error("Unknown screen mode %s", mode.c_str());
}

void HypnoEngine::loadPalette(const Common::String &fname) {
	Common::File *file = new Common::File();
	Common::String path = convertPath(fname);
	if (!_prefixDir.empty())
		path = _prefixDir + "/" + path;

	if (!file->open(path))
		error("unable to find palette file %s", path.c_str());

	debugC(1, kHypnoDebugMedia, "Loading palette from %s", path.c_str());
	byte *videoPalette = (byte *)malloc(file->size());
	file->read(videoPalette, file->size());
	g_system->getPaletteManager()->setPalette(videoPalette + 8, 0, 256);
}

void HypnoEngine::loadPalette(const byte *palette, uint32 offset, uint32 size) {
	debugC(1, kHypnoDebugMedia, "Loading palette from byte array with offset %d and size %d", offset, size);
	g_system->getPaletteManager()->setPalette(palette, offset, size);
}


byte *HypnoEngine::getPalette(uint32 idx) {
	byte *videoPalette = (byte *)malloc(3);
	g_system->getPaletteManager()->grabPalette(videoPalette, idx, 1);
	return videoPalette;
}

void HypnoEngine::updateVideo(MVideo &video) {
	video.decoder->decodeNextFrame();
}

void HypnoEngine::updateScreen(MVideo &video) {
	const Graphics::Surface *frame = video.decoder->decodeNextFrame();
	bool dirtyPalette = video.decoder->hasDirtyPalette();

	if (frame->h == 0 || frame->w == 0 || video.decoder->getPalette() == nullptr)
		return;

	const byte *videoPalette = nullptr;
	if (video.scaled && dirtyPalette) {
		debugC(1, kHypnoDebugMedia, "Updating palette at frame %d", video.decoder->getCurFrame());
		videoPalette = video.decoder->getPalette();
		g_system->getPaletteManager()->setPalette(videoPalette, 0, 256);
	}

	if (video.scaled) {
		Graphics::Surface *sframe = frame->scale(_screenW, _screenH);

		if (video.transparent) {
			_compositeSurface->transBlitFrom(*sframe, video.position, _transparentColor);
		} else {
			_compositeSurface->blitFrom(*sframe, video.position);
		}

		sframe->free();
		delete sframe;
	} else {
		if (video.transparent) {
			_compositeSurface->transBlitFrom(*frame, video.position, _transparentColor);
		} else {
			_compositeSurface->blitFrom(*frame, video.position);
		}
	}
}

void HypnoEngine::drawScreen() {
	g_system->copyRectToScreen(_compositeSurface->getPixels(), _compositeSurface->pitch, 0, 0, _screenW, _screenH);
	g_system->updateScreen();
	g_system->delayMillis(10);
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

	if (video.decoder != nullptr) {
		debugC(1, kHypnoDebugMedia, "Restarting %s!!!!", video.path.c_str());
		delete video.decoder;
	}
	// error("Video %s was not previously closed and deallocated", video.path.c_str());

	video.decoder = new HypnoSmackerDecoder();

	if (!video.decoder->loadStream(file))
		error("unable to load video %s", path.c_str());

	debugC(1, kHypnoDebugMedia, "audio track count: %d", video.decoder->getAudioTrackCount());
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

void HypnoEngine::playSound(const Common::String &filename, uint32 loops, uint32 sampleRate) {
	debugC(1, kHypnoDebugMedia, "%s(%s, %d, %d)", __FUNCTION__, filename.c_str(), loops, sampleRate);
	Common::String name = convertPath(filename);

	Audio::LoopingAudioStream *stream = nullptr;
	Common::File *file = new Common::File();
	if (file->open(name)) {
		stream = new Audio::LoopingAudioStream(Audio::makeRawStream(file, sampleRate, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), loops);
		_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
	} else {
		if (!_prefixDir.empty())
			name = _prefixDir + "/" + name;
		if (file->open(name)) {
			stream = new Audio::LoopingAudioStream(Audio::makeRawStream(file, sampleRate, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES), loops);
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_soundHandle, stream, -1, Audio::Mixer::kMaxChannelVolume);
		} else
			debugC(1, kHypnoDebugMedia, "%s not found!", name.c_str());
	}
}

void HypnoEngine::stopSound() {
	debugC(1, kHypnoDebugMedia, "%s()", __FUNCTION__);
	_mixer->stopAll();
	//_mixer->stopHandle(_soundHandle);
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
static void alarmCallback(void *refCon) {
	g_system->getTimerManager()->removeTimerProc(&alarmCallback);
	Common::String *level = (Common::String *)refCon;
	g_hypno->_nextLevel = *level;
	delete level;
}

static void countdownCallback(void *refCon) {
	g_hypno->_countdown = g_hypno->_countdown - 1;
}

bool HypnoEngine::startAlarm(uint32 delay, Common::String *ns) {
	return g_system->getTimerManager()->installTimerProc(&alarmCallback, delay, (void *)ns, "alarm");
}

bool HypnoEngine::startCountdown(uint32 delay) {
	_countdown = delay;
	_timerStarted = true;
	uint32 oneSecond = 1000000;
	return g_system->getTimerManager()->installTimerProc(&countdownCallback, oneSecond, 0x0, "countdown");
}

void HypnoEngine::removeTimers() {
	_timerStarted = false;
	g_system->getTimerManager()->removeTimerProc(&alarmCallback);
	g_system->getTimerManager()->removeTimerProc(&countdownCallback);
}

} // End of namespace Hypno
