
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

#include "bladerunner/actor.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/chapters.h"
#include "bladerunner/clues.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/image.h"
#include "bladerunner/mouse.h"
#include "bladerunner/outtake.h"
#include "bladerunner/scene.h"
#include "bladerunner/script/init.h"
#include "bladerunner/script/script.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vqa_decoder.h"

#include "common/array.h"
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
	_playerLosesControlCounter = 0;

	_script = new Script(this);
	_settings = new Settings(this);
}

BladeRunnerEngine::~BladeRunnerEngine() {
	// delete _sliceRenderer;
	// delete _sliceAnimations;
	// delete _settings;
	// delete _script;
	// delete _scene;
	// delete[] _gameVars;
	// delete _gameFlags;
	// delete _gameInfo;
	// delete _clues;
	// delete _chapters;
	// delete _audioSpeech;
	// delete _audioPlayer;
	// delete _ambientSounds;

	// _surface1.free();
	// _surface2.free();

	// delete[] _zBuffer1;
	// delete[] _zBuffer2;

	delete _settings;
	delete _script;
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

bool BladeRunnerEngine::startup(bool hasSavegames) {
	bool r;

	_surface1.create(640, 480, createRGB555());

	r = openArchive("STARTUP.MIX");
	if (!r)
		return false;

	// TODO: Timer

	_gameInfo = new GameInfo(this);
	if (!_gameInfo)
		return false;

	r = _gameInfo->open("GAMEINFO.DAT");
	if (!r)
		return false;

	// TODO: Create graphics surfaces 1-4

	// TODO: Allocate audio cache

	if (hasSavegames) {
		if (!loadSplash()) {
			return false;
		}
	}

	// TODO: World waypoints

	// TODO: Cover waypoints

	// TODO: Flee waypoints

	_gameVars = new int[_gameInfo->getGlobalVarCount()];

	// TODO: Actor AI DLL init

	// Seed rand

	// TODO: Sine and cosine lookup tables for intervals of 1.0, 4.0, and 12.0

	// TODO: View

	// TODO: Screen Index

	_gameFlags = new GameFlags();
	_gameFlags->setFlagCount(_gameInfo->getFlagCount());

	// TODO: Items

	// Setup sound output

	_audioPlayer = new AudioPlayer(this);

	// TODO: Audio: Music

	_audioSpeech = new AudioSpeech(this);

	_ambientSounds = new AmbientSounds(this);

	// TODO: Read BLADE.INI

	_chapters = new Chapters(this);
	if (!_chapters)
		return false;

	if (!openArchive("MUSIC.MIX"))
		return false;

	if (!openArchive("SFX.MIX"))
		return false;

	if (!openArchive("SPCHSFX.TLK"))
		return false;

	// TODO: Video overlays

	// TODO: Proper ZBuf class
	_zBuffer1 = new uint16[640 * 480];
	_zBuffer2 = new uint16[640 * 480];

	int actorCount = (int)_gameInfo->getActorCount();
	assert(actorCount < 99);
	for (int i = 0; i != actorCount; ++i) {
		_actors[i] = new Actor(this, i);
	}
	_voiceoverActor = new Actor(this, 99);
	_playerActor = _actors[_gameInfo->getPlayerId()];

	// TODO: set _playerActor countdown timer 6

	// TODO: Set actor ids (redundant?)

	// TODO: Police Maze

	_textActorNames = new TextResource(this);
	if (!_textActorNames->open("ACTORS"))
		return false;

	_textCrimes = new TextResource(this);
	if (!_textCrimes->open("CRIMES"))
		return false;

	_textCluetype = new TextResource(this);
	if (!_textCluetype->open("CLUETYPE"))
		return false;

	_textKIA = new TextResource(this);
	if (!_textKIA->open("KIA"))
		return false;

	_textSpindest = new TextResource(this);
	if (!_textSpindest->open("SPINDEST"))
		return false;

	_textVK = new TextResource(this);
	if (!_textVK->open("VK"))
		return false;

	_textOptions = new TextResource(this);
	if (!_textOptions->open("OPTIONS"))
		return false;

	// TODO: Dialogue Menu (DLGMENU.TRE)

	// TODO: SDB

	// TODO: KIA

	// TODO: Spinner Interface

	// TODO: Elevators

	// TODO: Scores

	// TODO: Font

	// TODO: KIA6PT.FON

	for (int i = 0; i != 43; ++i) {
		Shape *shape = new Shape(this);
		shape->readFromContainer("SHAPES.SHP", i);
		_shapes.push_back(shape);
	}

	// TODO: Esper

	// TODO: VK

	_mouse = new Mouse(this);
	// _mouse->setCursorPosition(320, 240);
	_mouse->setCursor(0);

	_sliceAnimations = new SliceAnimations(this);
	r = _sliceAnimations->open("INDEX.DAT");
	if (!r)
		return false;

	// TODO: Support cdframes

	r = _sliceAnimations->openHDFrames();
	if (!r)
		return false;

	r = _sliceAnimations->openCoreAnim();
	if (!r)
		return false;

	_sliceRenderer = new SliceRenderer(this);

	_clues = new Clues(this, "CLUES", _gameInfo->getClueCount());

	// TODO: Scene
	_scene = new Scene(this);

	// Load INIT.DLL
	ScriptInit initScript(this);
	initScript.SCRIPT_Initialize_Game();

	// TODO: Load AI-ACT1.DLL

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

	// TODO: Write BLADE.INI

	// TODO: Shutdown VK

	// TODO: Shutdown Esper

	delete _mouse;
	_mouse = 0;

	for (uint i = 0; i != _shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	// TODO: Shutdown Scene
	delete _scene;

	if (_chapters) {
		if (_chapters->hasOpenResources())
			_chapters->closeResources();
		delete _chapters;
		_chapters = 0;
	}

	delete _clues;
	_clues = 0;

	delete _sliceRenderer;
	_sliceRenderer = 0;

	delete _sliceAnimations;
	_sliceAnimations = 0;

	delete _textActorNames;
	_textActorNames = 0;

	delete _textCrimes;
	_textCrimes = 0;

	delete _textCluetype;
	_textCluetype = 0;

	delete _textKIA;
	_textKIA = 0;

	delete _textSpindest;
	_textSpindest = 0;

	delete _textVK;
	_textVK = 0;

	delete _textOptions;
	_textOptions = 0;

	// TODO: Delete dialogue menu

	delete _ambientSounds;

	// TODO: Delete overlays

	delete _audioSpeech;

	// TODO: Delete Audio: Music

	delete _audioPlayer;

	// Shutdown sound output

	if (isArchiveOpen("MUSIC.MIX"))
		closeArchive("MUSIC.MIX");

	if (isArchiveOpen("SFX.MIX"))
		closeArchive("SFX.MIX");

	if (isArchiveOpen("SPCHSFX.TLK"))
		closeArchive("SPCHSFX.TLK");

	// TODO: Delete KIA6PT.FON

	// TODO: Delete Items

	delete _gameFlags;
	_gameFlags = 0;

	// TODO: Delete View

	// TODO: Delete Screen Index

	// TODO: Delete sine and cosine lookup tables

	// TODO: Unload AI dll

	delete[] _gameVars;
	_gameVars = 0;

	// TODO: Delete World waypoints

	// TODO: Delete Cover waypoints

	// TODO: Delete Flee waypoints

	// TODO: Delete Scores

	// TODO: Delete Elevators

	// TODO: Delete Spinner Interface

	// TODO: Delete KIA

	// TODO: Delete SDB

	// TODO: Delete unknown stuff

	// TODO: Delete actors

	// TODO: Delete proper ZBuf class
	delete[] _zBuffer1;
	_zBuffer1 = 0;

	delete[] _zBuffer2;
	_zBuffer2 = 0;

	delete _gameInfo;
	_gameInfo = 0;

	// TODO: Delete graphics surfaces here
	_surface1.free();
	_surface2.free();

	if (isArchiveOpen("STARTUP.MIX"))
		closeArchive("STARTUP.MIX");

	// TODO: Delete MIXArchives here

	// TODO: Delete Timer
}

bool BladeRunnerEngine::loadSplash() {
	Image img(this);
	if (!img.open("SPLASH.IMG"))
		return false;

	img.copyToSurface(&_surface1);

	_system->copyRectToScreen(_surface1.getPixels(), _surface1.pitch, 0, 0, _surface1.w, _surface1.h);
	_system->updateScreen();

	return false;
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

			// HACK to draw McCoy
			_sliceRenderer->setView(_scene->_view);
			_playerActor->draw();

			// TODO: Draw items (drawing works in Replicant)
			// TODO: Draw item pickup (understood, drawing works in Replicant)
			// TODO: Draw dialogue menu

			Common::Point p = _eventMan->getMousePos();
			_mouse->draw(_surface2, p.x, p.y);

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

	playerLosesControl();

	do {
		gameTick();
	} while (_audioSpeech->isPlaying());

	playerGainsControl();
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

bool BladeRunnerEngine::playerHasControl() {
	return _playerLosesControlCounter == 0;
}

void BladeRunnerEngine::playerLosesControl() {
	if (++_playerLosesControlCounter == 1) {
		_mouse->disable();
	}
	debug("Player Lost Control (%d)", _playerLosesControlCounter);
}

void BladeRunnerEngine::playerGainsControl() {
	if (_playerLosesControlCounter == 0) {
		warning("Unbalanced call to BladeRunnerEngine::playerGainsControl");
	}

	if (_playerLosesControlCounter > 0)
		--_playerLosesControlCounter;

	debug("Player Gained Control (%d)", _playerLosesControlCounter);

	if (_playerLosesControlCounter == 0) {
		_mouse->enable();
	}
}

void BladeRunnerEngine::ISez(const char *str) {
	debug("\t%s", str);
}

} // End of namespace BladeRunner
