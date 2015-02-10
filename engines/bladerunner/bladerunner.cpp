
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

#include "bladerunner/bladerunner.h"

#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/chapters.h"
#include "bladerunner/clues.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/image.h"
#include "bladerunner/outtake.h"
#include "bladerunner/scene.h"
#include "bladerunner/script/init.h"
#include "bladerunner/script/script.h"
#include "bladerunner/settings.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vqa_decoder.h"

#include "common/error.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/pixelformat.h"

namespace BladeRunner {

BladeRunnerEngine::BladeRunnerEngine(OSystem *syst)
	: Engine(syst),
	  _rnd("bladerunner")
{
	_windowIsActive = true;
	_gameIsRunning  = true;

	_ambientSounds = new AmbientSounds(this);
	_audioPlayer = new AudioPlayer(this);
	_audioSpeech = new AudioSpeech(this);
	_chapters = nullptr;
	_clues = nullptr;
	_gameInfo = nullptr;
	_gameFlags = new GameFlags();
	_gameVars = nullptr;
	_scene = new Scene(this);
	_script = new Script(this);
	_settings = new Settings(this);
	_sliceAnimations = new SliceAnimations(this);
	_sliceRenderer = new SliceRenderer(this);

	_zBuffer1 = nullptr;
	_zBuffer2 = nullptr;

	_actorNames = nullptr;
}

BladeRunnerEngine::~BladeRunnerEngine() {
	delete _actorNames;

	delete _sliceRenderer;
	delete _sliceAnimations;
	delete _settings;
	delete _script;
	delete _scene;
	delete[] _gameVars;
	delete _gameFlags;
	delete _gameInfo;
	delete _clues;
	delete _chapters;
	delete _audioSpeech;
	delete _audioPlayer;
	delete _ambientSounds;

	_surface1.free();
	_surface2.free();

	delete[] _zBuffer1;
	delete[] _zBuffer2;
}

bool BladeRunnerEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsRTL;
}

Common::Error BladeRunnerEngine::run() {
	Graphics::PixelFormat format = createRGB555();
	initGraphics(640, 480, true, &format);

	_system->showMouse(true);

	if (!startup()) {
		shutdown();
		return Common::Error(Common::kUnknownError, "Failed to initialize resources");
	}

	if (warnUserAboutUnsupportedGame()) {
		init2();

		/* TODO: Check for save games and enter KIA */
		gameLoop();
	}

	shutdown();

	return Common::kNoError;
}

bool BladeRunnerEngine::startup() {
	bool r;

	_surface1.create(640, 480, createRGB555());

	r = openArchive("STARTUP.MIX");
	if (!r)
		return false;

	loadSplash();

	_gameInfo = new GameInfo(this);
	if (!_gameInfo)
		return false;

	r = _gameInfo->open("GAMEINFO.DAT");
	if (!r)
		return false;

	_gameFlags->setFlagCount(_gameInfo->getFlagCount());

	_gameVars = new int[_gameInfo->getGlobalVarCount()];

	_chapters = new Chapters(this);
	if (!_chapters)
		return false;

	r = openArchive("MUSIC.MIX");
	if (!r)
		return false;

	r = openArchive("SFX.MIX");
	if (!r)
		return false;

	r = openArchive("SPCHSFX.TLK");
	if (!r)
		return false;

	r = _sliceAnimations->open("INDEX.DAT");
	if (!r)
		return false;

	r = _sliceAnimations->openCoreAnim();
	if (!r)
		return false;

	r = _sliceAnimations->openHDFrames();
	if (!r)
		return false;

	_zBuffer1 = new uint16[640 * 480];
	_zBuffer2 = new uint16[640 * 480];

	_actorNames = new TextResource(this);
	_actorNames->open("ACTORS");

	_clues = new Clues(this, "CLUES", _gameInfo->getClueCount());

	ScriptInit initScript(this);
	initScript.SCRIPT_Initialize_Game();

	initChapterAndScene();

	return true;
}

void BladeRunnerEngine::initChapterAndScene() {
	// TODO: Init actors...

	_settings->setChapter(1);
	_settings->setNewSetAndScene(_gameInfo->getInitialSetId(), _gameInfo->getInitialSceneId());
}

void BladeRunnerEngine::shutdown() {
	_mixer->stopAll();

	if (_chapters) {
		if (_chapters->hasOpenResources())
			_chapters->closeResources();
		delete _chapters;
		_chapters = 0;
	}

	if (isArchiveOpen("MUSIC.MIX"))
		closeArchive("MUSIC.MIX");

	if (isArchiveOpen("SFX.MIX"))
		closeArchive("SFX.MIX");

	if (isArchiveOpen("SPCHSFX.TLK"))
		closeArchive("SPCHSFX.TLK");

	if (isArchiveOpen("STARTUP.MIX"))
		closeArchive("STARTUP.MIX");
}

void BladeRunnerEngine::loadSplash() {
	Image img(this);
	if (!img.open("SPLASH.IMG"))
		return;

	img.copyToSurface(&_surface1);

	_system->copyRectToScreen(_surface1.getPixels(), _surface1.pitch, 0, 0, _surface1.w, _surface1.h);
	_system->updateScreen();
}

bool BladeRunnerEngine::init2() {
	return true;
}

void BladeRunnerEngine::gameLoop() {
	_gameIsRunning = true;
	do {
		/* TODO: check player death */
		gameTick();
	} while (_gameIsRunning && !shouldQuit());
}

void BladeRunnerEngine::gameTick() {
	handleEvents();

	if (_gameIsRunning && _windowIsActive) {
		// TODO: Only run if not in Kia, script, nor AI
		_settings->openNewScene();

		// TODO: Autosave
		// TODO: Kia
		// TODO: Spinner
		// TODO: Esper
		// TODO: VK
		// TODO: Elevators
		// TODO: Scores
		// TODO: Call Script_Player_Walked_In if applicable
		// TODO: Gun range announcements
		// TODO: ZBUF repair dirty rects

		_ambientSounds->tick();

		bool backgroundChanged = false;
		int frame = _scene->advanceFrame(_surface1, _zBuffer1);
		if (frame >= 0) {
			_script->SceneFrameAdvanced(frame);
			backgroundChanged = true;
		}
		(void)backgroundChanged;
		_surface2.copyFrom(_surface1);
		memcpy(_zBuffer2, _zBuffer1, 640*480*2);

		// TODO: Render overlays (mostly in Replicant)
		// TODO: Tick Actor AI and Timers (timers in Replicant)

		if (_settings->getNewScene() == -1 || _script->_inScriptCounter /* || in_ai */) {

			// TODO: Tick and draw all actors in current set (drawing works in Replicant)

			// Hardcode McCoy in place to test the slice renderer
			Vector3 pos = _scene->_actorStartPosition;
			Vector3 draw_pos(pos.x, -pos.z, pos.y + 2);
			float facing = -1.570796f;

			_sliceRenderer->setView(_scene->_view);
			_sliceRenderer->setupFrame(19, 1, draw_pos, facing);
			_sliceRenderer->drawFrame(_surface2, _zBuffer2);

			// TODO: Draw items (drawing works in Replicant)
			// TODO: Draw item pickup (understood, drawing works in Replicant)
			// TODO: Draw dialogue menu
			// TODO: Draw mouse (understood)
			// TODO: Process AUD (audio in Replicant)
			// TODO: Footstep sound

			_system->copyRectToScreen((const byte *)_surface2.getBasePtr(0, 0), _surface2.pitch, 0, 0, 640, 480);
			_system->updateScreen();
			_system->delayMillis(10);
		}
	}
}

void BladeRunnerEngine::handleEvents() {
	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
	}
}

void BladeRunnerEngine::loopActorSpeaking() {
	if (!_audioSpeech->isPlaying())
		return;

	// playerLosesControl();

	do {
		gameTick();
	} while (_audioSpeech->isPlaying());

	// playerGainsControl();
}

void BladeRunnerEngine::outtakePlay(int id, bool noLocalization, int container) {
	Common::String name = _gameInfo->getOuttake(id);

	OuttakePlayer player(this);

	player.play(name, noLocalization, container);
}

bool BladeRunnerEngine::openArchive(const Common::String &name) {
	uint i;

	// If archive is already open, return true
	for (i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name)
			return true;
	}

	// Find first available slot
	for (i = 0; i != kArchiveCount; ++i) {
		if (!_archives[i].isOpen())
			break;
	}
	if (i == kArchiveCount) {
		/* TODO: BLADE.EXE retires the least recently used
		 * archive when it runs out of slots. */

		error("openArchive: No more archive slots");
		return false;
	}

	_archives[i].open(name);
	return _archives[i].isOpen();
}

bool BladeRunnerEngine::closeArchive(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (_archives[i].isOpen() &&_archives[i].getName() == name) {
			_archives[i].close();
			return true;
		}
	}

	debug("closeArchive: Archive %s not open.", name.c_str());
	return false;
}

bool BladeRunnerEngine::isArchiveOpen(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (_archives[i].isOpen() &&_archives[i].getName() == name)
			return true;
	}

	return false;
}

Common::SeekableReadStream *BladeRunnerEngine::getResourceStream(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (!_archives[i].isOpen())
			continue;

		if (false)
			debug("getResource: Searching archive %s for %s.", _archives[i].getName().c_str(), name.c_str());
		Common::SeekableReadStream *stream = _archives[i].createReadStreamForMember(name);
		if (stream)
			return stream;
	}

	debug("getResource: Resource %s not found.", name.c_str());
	return 0;
}

void BladeRunnerEngine::ISez(const char *str) {
	debug("\t%s", str);
}


} // End of namespace BladeRunner
