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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "asylum/asylum.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/reaction.h"
#include "asylum/resources/script.h"
#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

#include "asylum/puzzles/puzzles.h"

#include "asylum/system/cursor.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"
#include "asylum/views/menu.h"
#include "asylum/views/video.h"

#include "asylum/respack.h"

#include "common/debug-channels.h"
#include "common/EventRecorder.h"

#include "engines/util.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), _gameDescription(gd),
	_console(NULL), _cursor(NULL), _encounter(NULL), _menu(NULL), _reaction(NULL), _resource(NULL), _savegame(NULL),
	_scene(NULL), _screen(NULL), _script(NULL), _special(NULL), _speech(NULL), _sound(NULL), _text(NULL),
	_video(NULL), _handler(NULL) {

	// Init data
	memset(&_gameFlags, 0, sizeof(_gameFlags));
	memset(&_sinCosTables, 0, sizeof(_sinCosTables));
	_introPlayed = false;
	_tickOffset = 0;

	screenUpdateCount = 0;
	lastScreenUpdate  = 0;

	// Debug
	_delayedSceneIndex = kResourcePackInvalid;
	_delayedVideoIndex = -1;
	_previousScene = NULL;

	// Add default search directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "data");
	SearchMan.addSubDirectoryMatching(gameDataDir, "vids");
	SearchMan.addSubDirectoryMatching(gameDataDir, "music");

	// Initialize custom debug levels
	DebugMan.addDebugChannel(kDebugLevelMain,      "Main",      "Generic debug level");
	DebugMan.addDebugChannel(kDebugLevelResources, "Resources", "Resources debugging");
	DebugMan.addDebugChannel(kDebugLevelSprites,   "Sprites",   "Sprites debugging");
	DebugMan.addDebugChannel(kDebugLevelInput,     "Input",     "Input events debugging");
	DebugMan.addDebugChannel(kDebugLevelMenu,      "Menu",      "Menu debugging");
	DebugMan.addDebugChannel(kDebugLevelScripts,   "Scripts",   "Scripts debugging");
	DebugMan.addDebugChannel(kDebugLevelSound,     "Sound",     "Sound debugging");
	DebugMan.addDebugChannel(kDebugLevelSavegame,  "Savegame",  "Saving & restoring game debugging");
	DebugMan.addDebugChannel(kDebugLevelScene,     "Scene",     "Scene process and draw debugging");
	DebugMan.addDebugChannel(kDebugLevelObjects,   "Objects",   "Debug Object Objects");

	// Initialize random number source
	_rnd = new Common::RandomSource("asylum");
}

AsylumEngine::~AsylumEngine() {
	_handler = NULL;

	delete _cursor;
	delete _scene;
	delete _encounter;
	delete _puzzles;
	delete _reaction;
	delete _savegame;
	delete _screen;
	delete _script;
	delete _special;
	delete _speech;
	delete _sound;
	delete _text;
	delete _video;
	delete _menu;
	delete _resource;
	delete _console;

	_previousScene = NULL;

	delete _rnd;

	// Zero passed pointers
	_gameDescription = NULL;
}

Common::Error AsylumEngine::run() {
	// Initialize the graphics
	initGraphics(640, 480, true);

	// Create debugger. It requires GFX to be initialized
	_console   = new Console(this);

	// Create resource manager
	_resource  = new ResourceManager();
	_resource->setCdNumber(1);

	// Create all game classes
	_encounter = new Encounter(this);
	_cursor    = new Cursor(this);
	_puzzles   = new Puzzles(this);
	_reaction  = new Reaction(this);
	_savegame  = new Savegame(this);
	_screen    = new Screen(this);
	_script    = new ScriptManager(this);
	_sound     = new Sound(this, _mixer);
	_special   = new Special(this);
	_speech    = new Speech(this);
	_text      = new Text(this);
	_video     = new VideoPlayer(this, _mixer);

	// Init tables
	initSinCosTables(80.0, 40, 40);

	// Create main menu
	_menu  = new Menu(this);
	_handler = _menu;

    // Load config
    Config.read();

	// Setup mixer
	syncSoundSettings();

	// Send init event to our default event handler
	AsylumEvent initEvt(EVENT_ASYLUM_INIT);
	if (_handler)
		_handler->handleEvent(initEvt);

	// Start running event loop
	while (!shouldQuit()) {
		handleEvents();

		_system->delayMillis(10);

		_system->updateScreen();
	}

	return Common::kNoError;
}

void AsylumEngine::startGame(ResourcePackId sceneId, StartGameType type) {
	if (!_cursor || !_screen || !_savegame)
		error("[AsylumEngine::startGame] Subsystems not initialized properly!");

	// Load the default mouse cursor
	_cursor->set(MAKE_RESOURCE(kResourcePackSound, 14), 0, kCursorAnimationNone);
	_cursor->hide();

	// Clear the graphic list
	_screen->clearGraphicsInQueue();

	// Reset scene (this ensures the current resource pack is closed as in the original)
	delete _scene;
	_scene = new Scene(this);
	_handler = _scene;

	// Original checks for the current cd (we have all data files on disc, so this is not needed)
	switch (type) {
	default:
		error("[AsylumEngine::startGame] Invalid start game type!");

	case kStartGamePlayIntro:
		_scene->enter(sceneId);
		playIntro();
		break;

	case kStartGameLoad:
		if (_savegame->load()) {
			_scene->enterLoad();
			updateReverseStereo();
			switchEventHandler(_scene);
		}
		break;

	case kStartGameScene:
		_scene->enter(sceneId);
		break;
	}

	_cursor->show();
}

void AsylumEngine::restart() {
	if (!_cursor || !_script)
		error("[AsylumEngine::restart] Subsystems not initialized properly!");

	_cursor->hide();

	// Cleanup
	memset(&_gameFlags, 0, sizeof(_gameFlags));
	delete _scene;
	_scene = NULL;
	delete _encounter;
	_encounter = new Encounter(this);
	_script->resetQueue();

	_data.setGlobalPoint(Common::Point(-1, -1));

	reset();

	_introPlayed = false;

	_screen->clear();
	_sound->playMusic(kResourceNone, 0);

	startGame(kResourcePackTowerCells, kStartGamePlayIntro);
}

void AsylumEngine::reset() {
	if (!_menu)
		error("[AsylumEngine::reset] Subsystems not initialized properly!");

	// Set game as started
	_menu->setGameStarted();

	// Reset puzzles
	_puzzles->reset();

	// Reset shared data
	_data.reset();

	// Reset special palette info
	_special->reset(true);
}

void AsylumEngine::playIntro() {
	if (!_video || !_screen)
		error("[AsylumEngine::playIntro] Subsystems not initialized properly!");

	updateReverseStereo();

	if (!_introPlayed) {
		_cursor->hide();
		_cursor->setForceHide(true);
		if (!Config.showIntro) {
			if (_scene->worldstats()->chapter == kChapter1)
				_sound->playMusic(MAKE_RESOURCE(kResourcePackMusic, _scene->worldstats()->musicCurrentResourceIndex));
		} else {
			_sound->playMusic(kResourceNone, 0);

			_video->play(1, _menu);

			if (_scene->worldstats()->musicCurrentResourceIndex != kMusicStopped)
				_sound->playMusic(MAKE_RESOURCE(kResourcePackMusic, _scene->worldstats()->musicCurrentResourceIndex));

			_screen->clear();

			setGameFlag(kGameFlag4);
			setGameFlag(kGameFlag12);

			// Play the intro speech: it is played after the intro video over a black background,
			// and the game is "locked" until the speech is completed.
			ResourceId introSpeech = MAKE_RESOURCE(kResourcePackSound, 7);
			_sound->playSound(introSpeech);

			do {
				// Poll events (this ensures we don't freeze the screen)
				Common::Event ev;
				_eventMan->pollEvent(ev);

				_system->delayMillis(100);

			} while (_sound->isPlaying(introSpeech));

		}
		_cursor->setForceHide(false);
		_introPlayed = true;
	}

	_cursor->show();

	_savegame->loadMoviesViewed();

	// Switch to scene event handling
	switchEventHandler(_scene);
}

void AsylumEngine::handleEvents() {
	if (!_console || !_video || !_screen || !_sound || !_menu || !_cursor)
		error("[AsylumEngine::handleEvents] Subsystems not initialized properly!");

	// Show the debugger if required
	_console->onFrame();

	AsylumEvent ev;
	while (_eventMan->pollEvent(ev)) {
		switch (ev.type) {
		default:
			break;

		case Common::EVENT_KEYDOWN:
			if ((ev.kbd.flags & Common::KBD_CTRL) && ev.kbd.keycode == Common::KEYCODE_d) {
				_console->attach();
				break;
			}

			// Handle key events
			if (_handler)
				_handler->handleEvent(ev);
			break;

		case Common::EVENT_KEYUP:
			// Handle key events
			if (_handler)
				_handler->handleEvent(ev);
			break;

		case Common::EVENT_MOUSEMOVE:
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_LBUTTONUP:
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MBUTTONUP:
		case Common::EVENT_MBUTTONDOWN:
			// Handle mouse events
			_cursor->setState(ev);

			if (_handler)
				_handler->handleEvent(ev);
			break;

		case Common::EVENT_QUIT:
			quitGame();
			break;

		// TODO handle cases where we receive a midi or music event
		}
	}

	// Animate cursor
	_cursor->animate();

	// Send update event to our event handler
	AsylumEvent updateEvt = AsylumEvent(EVENT_ASYLUM_UPDATE);
	if (_handler)
		_handler->handleEvent(updateEvt);

	// Handle debug events
	processDelayedEvents();
}

void AsylumEngine::processDelayedEvents() {
	if (!_video || !_sound || !_menu || !_script)
		error("[AsylumEngine::processDelayedEvents] Subsystems not initialized properly!");

	// check for a delayed scene change
	if (_delayedSceneIndex != kResourcePackInvalid && isGameFlagNotSet(kGameFlagScriptProcessing)) {
		ResourcePackId sceneIndex = _delayedSceneIndex;
		_delayedSceneIndex = kResourcePackInvalid;

		// Reset script queue
		_script->resetQueue();

		_sound->stopMusic();
		_sound->stopAll();

		switchScene(sceneIndex);
	}

	// Check for delayed video
	if (_delayedVideoIndex != -1 && isGameFlagNotSet(kGameFlagScriptProcessing)) {
		uint32 index = (uint32)_delayedVideoIndex;
		_delayedVideoIndex = -1;

		_video->play(index, _handler);
	}
}

//////////////////////////////////////////////////////////////////////////
// Message handlers
//////////////////////////////////////////////////////////////////////////
void AsylumEngine::switchEventHandler(EventHandler *handler) {
	if (handler == NULL)
		warning("[AsylumEngine::switchMessageHandler] NULL handler parameter (shouldn't happen outside of debug commands)!");

	// De-init previous handler
	if (_handler != NULL) {
		AsylumEvent deinit(EVENT_ASYLUM_DEINIT);
		_handler->handleEvent(deinit);
	}

	//////////////////////////////////////////////////////////////////////////
	// DEBUG - If a previous scene is found, replace the current scene by this one
	if (handler == _scene) {
		if (_previousScene) {
			delete _scene;
			_scene = _previousScene;
			handler = _scene;
			_previousScene = NULL;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// replace message handler
	_handler = handler;

	// Init new handler
	AsylumEvent init(EVENT_ASYLUM_INIT);
	if (_handler)
		_handler->handleEvent(init);
}

void AsylumEngine::notify(AsylumEventType type, int32 param1, int32 param2) {
	if (_handler == NULL)
		error("[AsylumEngine::notify] Invalid handler parameter (cannot be NULL)!");

	AsylumEvent evt(type, param1, param2);
	_handler->handleEvent(evt);
}


void AsylumEngine::initSinCosTables(double a2, int32 a3, int32 a4) {
	uint32 offset = 0;
	uint32 baseStep = 1;

	do {
		if (baseStep >= 1) {
			uint32 baseAngle = 90;
			int32 step = baseStep;

			int16 *val = &_sinCosTables[2 * offset];

			do {
				double angle = (double)(baseAngle % 360) * 3.141592653589 * 0.005555555555555556;

				*val       = (int16)(cos(angle) * a2 - (a3 / 2.0));
				*(val + 1) = (int16)(sin(angle) * a2 - (a4 / 2.0));

				baseAngle += 360 / baseStep;
				val += 2;
				--step;
			} while (step);

			offset += baseStep;
		}

		++baseStep;

	} while (baseStep <= 8);
}

int32 AsylumEngine::computeSinCosOffset(int32 val) const {
	int32 offset = 0;
	for (int32 i = val; i > 0; --i)
		offset += i;

	return offset - val;
}

Common::Point AsylumEngine::getSinCosValues(int32 index1, int32 index2) {
	if (!_scene)
		error("[AsylumEngine::getSinCosValues] Subsystems not initialized properly!");

	Common::Point values;

	if (_scene->worldstats()->chapter == kChapter11) {
		int32 offset = computeSinCosOffset(8) + index2 + 3;
		values.x = _sinCosTables[2 * offset];
		values.y = _sinCosTables[1];
	} else {
		int32 offset = computeSinCosOffset(index1) + index2;
		values.x = _sinCosTables[2 * offset];
		values.y = _sinCosTables[2 * offset + 1];
	}

	return values;
}

void AsylumEngine::updateReverseStereo() {
	if (_scene && _scene->worldstats())
		_scene->worldstats()->reverseStereo = Config.reverseStereo;
}

void AsylumEngine::saveLoadWithSerializer(Common::Serializer &s) {
	if (!_script)
		error("[AsylumEngine::saveLoadWithSerializer] Subsystems not initialized properly!");

	// Game flags
	for (uint32 i = 0; i < ARRAYSIZE(_gameFlags); i++)
		s.syncAsSint32LE(_gameFlags[i]);

	// The original has the script data in the middle of other shared data,
	// so to be compatible with original savegames, we want to save it in
	// the proper order
	_data.saveLoadAmbientSoundData(s);

	// Original skips two elements
	// (original has one unused, one used for debugging screen update counts)
	s.skip(8);

	// Script queue
	_script->saveQueue(s);

	// Shared data (the rest of it)
	_data.saveLoadWithSerializer(s);
}

//////////////////////////////////////////////////////////////////////////
// Game flags
//////////////////////////////////////////////////////////////////////////
#define FLAG_MASK 0xFFFFFFE0

void AsylumEngine::setGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] |= 1 << (flag % FLAG_MASK);
}

void AsylumEngine::clearGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] &= ~(1 << (uint32)(flag % FLAG_MASK));
}

void AsylumEngine::toggleGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] ^= 1 << (uint32)(flag % FLAG_MASK);
}

bool AsylumEngine::isGameFlagSet(GameFlag flag) const {
	return ((1 << (flag % FLAG_MASK)) & (unsigned int)_gameFlags[flag / 32]) >> (flag % FLAG_MASK) != 0;
}

bool AsylumEngine::isGameFlagNotSet(GameFlag flag) const {
	return ((1 << (flag % FLAG_MASK)) & (unsigned int)_gameFlags[flag / 32]) >> (flag % FLAG_MASK) == 0;
}

} // namespace Asylum
