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

#include "asylum/puzzles/boardkeyhidesto.h"
#include "asylum/puzzles/boardsalvation.h"
#include "asylum/puzzles/boardyouth.h"
#include "asylum/puzzles/clock.h"
#include "asylum/puzzles/fisherman.h"
#include "asylum/puzzles/hivecontrol.h"
#include "asylum/puzzles/hivemachine.h"
#include "asylum/puzzles/lock.h"
#include "asylum/puzzles/morguedoor.h"
#include "asylum/puzzles/pipes.h"
#include "asylum/puzzles/puzzle11.h"
#include "asylum/puzzles/tictactoe.h"
#include "asylum/puzzles/timemachine.h"
#include "asylum/puzzles/vcr.h"
#include "asylum/puzzles/wheel.h"
#include "asylum/puzzles/writings.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/sound.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/scene.h"
#include "asylum/views/menu.h"
#include "asylum/views/video.h"

#include "asylum/console.h"
#include "asylum/respack.h"

#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/events.h"
#include "common/EventRecorder.h"

#include "engines/util.h"

namespace Asylum {

AsylumEngine::AsylumEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), _gameDescription(gd),
	_console(NULL), _cursor(NULL), _encounter(NULL), _menu(NULL), _reaction(NULL), _resource(NULL), _savegame(NULL),
	_scene(NULL), _screen(NULL), _script(NULL), _sound(NULL), _text(NULL), _video(NULL), _handler(NULL) {

	// Init data
	memset(&_gameFlags, 0, sizeof(_gameFlags));
	memset(&_puzzles, 0, sizeof(_puzzles));
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
	delete _cursor;
	delete _scene;
	delete _encounter;
	delete _reaction;
	delete _savegame;
	delete _screen;
	delete _script;
	delete _sound;
	delete _text;
	delete _video;
	delete _menu;
	delete _resource;
	delete _console;

	// Cleanup puzzles
	for (uint i = 0; i < ARRAYSIZE(_puzzles); i++)
		delete _puzzles[i];

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
	_reaction  = new Reaction(this);
	_savegame  = new Savegame(this);
	_screen    = new Screen(this);
	_script    = new ScriptManager(this);
	_sound     = new Sound(this, _mixer);
	_special   = new Special(this);
	_speech    = new Speech(this);
	_text      = new Text(this);
	_video     = new VideoPlayer(this, _mixer);
	initPuzzles();

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
	if (!_cursor || !_screen)
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
	_cursor->hide();

	// Cleanup
	memset(&_gameFlags, 0, sizeof(_gameFlags));
	delete _scene;
	_scene = NULL;
	// TODO reset puzzle data
	// TODO reset encounter data
	_script->resetQueue();

	_data.point.x = -1;
	_data.point.y = -1;

	reset();

	_introPlayed = false;

	_screen->clear();
	_sound->playMusic(kResourceNone, 0);

	startGame(kResourcePackTowerCells, kStartGamePlayIntro);
}

void AsylumEngine::reset() {
	// Set game as started
	_menu->setGameStarted();

	// Reset puzzles
	for (uint32 i = 0; i < ARRAYSIZE(_puzzles); i++)
		if (_puzzles[i] != NULL)
			_puzzles[i]->reset();

	// FIXME reset shared actor data
}

void AsylumEngine::playIntro() {
	if (!_video || !_screen)
		error("[AsylumEngine::playIntro] Subsystems not initialized properly!");

	updateReverseStereo();

	if (!_introPlayed) {
		_cursor->hide();

		if (!Config.showIntro) {
			if (_scene->worldstats()->chapter == kChapter1)
				_sound->playMusic(MAKE_RESOURCE(kResourcePackMusic, _scene->worldstats()->musicCurrentResourceIndex));
		} else {
			_sound->playMusic(kResourceNone, 0);

			_video->play(1, _menu);
			// FIXME: the cursor is still shown after the video ends

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
				// Poll events (this ensure we don't freeze the screen)
				Common::Event ev;
				_eventMan->pollEvent(ev);

			} while (_sound->isPlaying(introSpeech));

		}

		_introPlayed = true;
	}

	_cursor->show();

	_savegame->loadMoviesViewed();

	// Switch to scene event handling
	switchEventHandler(_scene);
}

void AsylumEngine::handleEvents() {
	if (!_console || !_video || !_screen || !_sound || !_menu)
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
	if (!_video || !_sound || !_menu)
		error("[AsylumEngine::processDelayedEvents] Subsystems not initialized properly!");

	// check for a delayed scene change
	if (_delayedSceneIndex != kResourcePackInvalid && isGameFlagNotSet(kGameFlagScriptProcessing)) {
		ResourcePackId sceneIndex = _delayedSceneIndex;

		// Reset delayed scene
		_delayedSceneIndex = kResourcePackInvalid;

		_scene->getActor(0)->updateStatus(kActorStatusDisabled);
		_script->reset();

		_sound->stopMusic();
		_sound->stopAll();

		switchScene(sceneIndex);
	}

	// Check for delayed video
	if (_delayedVideoIndex != -1 && isGameFlagNotSet(kGameFlagScriptProcessing)) {
		int32 index = _delayedVideoIndex;
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

EventHandler *AsylumEngine::getPuzzle(uint32 index) {
	if (index > ARRAYSIZE(_puzzles))
		error("[AsylumEngine::getPuzzleEventHandler] Invalid index (was: %d - max: %d)", index, ARRAYSIZE(_puzzles));

	if (_puzzles[index] == NULL)
		error("[AsylumEngine::getPuzzleEventHandler] This puzzle doesn't have an event handler! (index: %d)", index);

	return (EventHandler *)_puzzles[index];
}

void AsylumEngine::initPuzzles() {
	_puzzles[0] = new PuzzleVCR(this);
	_puzzles[1] = new PuzzlePipes(this);
	_puzzles[2] = new PuzzleTicTacToe(this);
	_puzzles[3] = new PuzzleLock(this);
	_puzzles[4] = NULL;// No event handler for Puzzle 5
	_puzzles[5] = new PuzzleWheel(this);
	_puzzles[6] = new PuzzleBoardSalvation(this);
	_puzzles[7] = new PuzzleBoardYouth(this);
	_puzzles[8] = new PuzzleBoardKeyHidesTo(this);
	_puzzles[9] = new PuzzleWritings(this);
	_puzzles[10] = new Puzzle11(this);
	_puzzles[11] = new PuzzleMorgueDoor(this);
	_puzzles[12] = new PuzzleClock(this);
	_puzzles[13] = new PuzzleTimeMachine(this);
	_puzzles[14] = new PuzzleFisherman(this);
	_puzzles[15] = new PuzzleHiveMachine(this);
	_puzzles[16] = new PuzzleHiveControl(this);
}

void AsylumEngine::initSinCosTables(double a2, int32 a3, int32 a4) {
	uint32 offset = 0;
	uint32 baseStep = 1;

	do {
		if (baseStep >= 1) {
			int32 baseAngle = 90;
			int32 step = baseStep;

			int32 *val = &_sinCosTables[2 * offset];

			do {
				double angle = (double)(baseAngle % 360) * 3.141592653589 * 0.005555555555555556;

				*val       = cos(angle) * a2 - (a3 / 2);
				*(val + 1) = sin(angle) * a2 - (a4 / 2);

				baseAngle += 360 / baseStep;
				val += 2;
				--step;
			} while (step);

			offset += baseStep;
		}

		++baseStep;

	} while (baseStep <= 8);
}

int32 AsylumEngine::computeSinCosOffset(int32 val) {
	int32 offset = 0;
	for (int32 i = val; i > 0; --i)
		offset += i;

	return offset - val;
}

Common::Point AsylumEngine::getSinCosValues(int32 index1, int32 index2) {
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
	error("[AsylumEngine::saveLoadWithSerializer] Not implemented!");
}

//////////////////////////////////////////////////////////////////////////
// Game flags
//////////////////////////////////////////////////////////////////////////
#define FLAG_MASK 0xFFFFFFE0

void AsylumEngine::setGameFlagByIndex(int32 index) {
	_gameFlags[index] = 1;
}

int32 AsylumEngine::getGameFlagByIndex(int32 index) {
	return _gameFlags[index];
}

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

void AsylumEngine::resetFlags() {
	memset(&_gameFlags[130], 0, 60);
}

} // namespace Asylum
