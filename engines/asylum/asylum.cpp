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

#include "backends/keymapper/keymapper.h"

#include "common/debug-channels.h"
#include "common/rect.h"

#include "engines/util.h"

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

namespace Asylum {

// inventory ring is a circle of radius 80 centered at (-20, 20)
static const int16 inventoryRingPoints[36][2] = {
	{ -20,  100},
	{ -20,  100}, { -20,  -60},
	{ -20,  100}, { -89,  -20}, {  49,  -20},
	{ -20,  100}, {-100,   20}, { -20,  -60}, {  60,   20},
	{ -20,  100}, { -96,   45}, { -67,  -45}, {  27,  -45}, {  56,   45},
	{ -20,  100}, { -89,   60}, { -89,  -20}, { -20,  -60}, {  49,  -20}, {  49,   60},
	{ -20,  100}, { -82,   70}, { -98,    3}, { -56,  -51}, {  13,  -53}, {  57,   -1}, {  45,   67},
	{ -20,  100}, { -77,   77}, {-100,   20}, { -77,  -37}, { -20,  -60}, {  37,  -37}, {  60,   20}, {  37,   77}
};

// first 8 triangular numbers
static const uint32 inventoryRingOffsets[8] = {0, 1, 3, 6, 10, 15, 21, 28};

AsylumEngine::AsylumEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), _gameDescription(gd),
	_console(NULL), _cursor(NULL), _encounter(NULL), _menu(NULL), _reaction(NULL), _resource(NULL), _savegame(NULL),
	_scene(NULL), _screen(NULL), _script(NULL), _special(NULL), _speech(NULL), _sound(NULL), _text(NULL),
	_video(NULL), _handler(NULL), _puzzles(NULL) {

	// Init data
	memset(&_gameFlags, 0, sizeof(_gameFlags));
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
	SearchMan.addSubDirectoryMatching(gameDataDir, "data/vids");
	SearchMan.addSubDirectoryMatching(gameDataDir, "data/music");

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

	_previousScene = NULL;

	delete _rnd;

	// Zero passed pointers
	_gameDescription = NULL;
}

bool AsylumEngine::hasFeature(EngineFeature f) const {
	return (f == kSupportsReturnToLauncher);
}

Common::Error AsylumEngine::run() {
	// Initialize the graphics
	initGraphics(640, 480);

	// Create debugger. It requires GFX to be initialized
	_console   = new Console(this);
	setDebugger(_console);

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

	// Set the current cd number (necessary for proper SharedSound resource pack initialization)
	int32 cdNumber = _resource->getCdNumber();

	switch (sceneId) {
	default:
		_resource->setCdNumber(-1);
		break;

	case kResourcePackTowerCells:
	case kResourcePackInnocentAbandoned:
	case kResourcePackCourtyardAndChapel:
		_resource->setCdNumber(1);
		break;

	case kResourcePackCircusOfFools:
	case kResourcePackCave:
	case kResourcePackMansion:
	case kResourcePackLaboratory:
	case kResourcePackHive:
		_resource->setCdNumber(2);
		break;

	case kResourcePackMorgueAndCemetery:
	case kResourcePackLostVillage:
	case kResourcePackMaze:
	case kResourcePackGauntlet:
	case kResourcePackMorgansLastGame:
		_resource->setCdNumber(3);
		break;
	}

	if (_resource->getCdNumber() != cdNumber)
		_resource->clearSharedSoundCache();

	_resource->clearMusicCache();

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
	if (!_menu || !_special || !_puzzles)
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

			int8 skip = 0;
			do {
				// Poll events (this ensures we don't freeze the screen)
				Common::Event ev;
				while (_eventMan->pollEvent(ev)) {
					switch (ev.type) {
					case Common::EVENT_LBUTTONDOWN:
					case Common::EVENT_KEYDOWN:
						skip = true;
						break;
					default:
						break;
					}
				}

				_system->updateScreen();
				_system->delayMillis(100);

			} while (_sound->isPlaying(introSpeech) && !skip);

			if (_sound->isPlaying(introSpeech)) {
				_sound->stop(introSpeech);
			}
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
	Common::Keymapper *const keymapper = _eventMan->getKeymapper();

	while (_eventMan->pollEvent(ev)) {
		keymapper->setEnabled(_handler == _scene || (_handler == _menu && !_menu->isEditingSavegameName()));
		switch (ev.type) {
		default:
			break;

		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			// Handle custom actions
			if (_handler)
				_handler->handleEvent(ev);
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
	if (!_video || !_sound || !_menu || !_script || !_screen)
		error("[AsylumEngine::processDelayedEvents] Subsystems not initialized properly!");

	// check for a delayed scene change
	if (_delayedSceneIndex != kResourcePackInvalid && isGameFlagNotSet(kGameFlagScriptProcessing)) {
		ResourcePackId sceneIndex = _delayedSceneIndex;
		_delayedSceneIndex = kResourcePackInvalid;

		// Reset actor and script queue
		_script->resetQueue();
		_script->reset();
		if (_scene)
			_scene->getActor(0)->updateStatus(kActorStatusDisabled);

		// Fade screen to black
		_screen->paletteFade(0, 75, 8);
		_screen->clear();

		// Stop all sounds & music
		_sound->stopMusic();
		_sound->stopAll();

		// Switch the scene
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

Common::Point AsylumEngine::getInventoryRingPoint(uint32 nPoints, uint32 index) const {
	if (!_scene)
		error("[AsylumEngine::getInventoryRingPoint] Subsystems not initialized properly!");

	const int16 (*pointPtr)[2];
	if (_scene->worldstats()->chapter == kChapter11)
		pointPtr = &inventoryRingPoints[inventoryRingOffsets[7] + index + 3];
	else
		pointPtr = &inventoryRingPoints[inventoryRingOffsets[nPoints - 1] + index];

	return Common::Point((*pointPtr)[0], (*pointPtr)[1]);
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
		s.syncAsUint32LE(_gameFlags[i]);

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
void AsylumEngine::setGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] |= 1 << (flag % 32);
}

void AsylumEngine::clearGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] &= ~(1 << (flag % 32));
}

void AsylumEngine::toggleGameFlag(GameFlag flag) {
	_gameFlags[flag / 32] ^= 1 << (flag % 32);
}

bool AsylumEngine::isGameFlagSet(GameFlag flag) const {
	return ((1 << (flag % 32)) & _gameFlags[flag / 32]) >> (flag % 32) != 0;
}

bool AsylumEngine::isGameFlagNotSet(GameFlag flag) const {
	return ((1 << (flag % 32)) & _gameFlags[flag / 32]) >> (flag % 32) == 0;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
bool AsylumEngine::rectContains(const int16 (*rectPtr)[4], const Common::Point &p) const {
	return ((*rectPtr)[0] <= p.x) && (p.x < (*rectPtr)[2]) && ((*rectPtr)[1] <= p.y) && (p.y < (*rectPtr)[3]);
}

} // namespace Asylum
