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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "backends/keymapper/keymapper.h"

#include "common/achievements.h"
#include "common/debug-channels.h"
#include "common/rect.h"
#include "common/translation.h"

#include "engines/util.h"

#include "gui/message.h"

#include "asylum/asylum.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
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

AsylumEngine::AsylumEngine(OSystem *system, const ADGameDescription *gd) : Engine(system), _gameDescription(gd),
	_console(nullptr), _cursor(nullptr), _encounter(nullptr), _menu(nullptr), _resource(nullptr), _savegame(nullptr),
	_scene(nullptr), _screen(nullptr), _script(nullptr), _special(nullptr), _speech(nullptr), _sound(nullptr), _text(nullptr),
	_video(nullptr), _handler(nullptr), _puzzles(nullptr) {

	// Init data
	resetFlags();
	_introPlayed = false;
	_tickOffset = 0;

	screenUpdateCount = 0;
	lastScreenUpdate  = 0;

	// Debug
	_delayedSceneIndex = kResourcePackInvalid;
	_delayedVideoIndex = -1;
	_previousScene = nullptr;

	// Add default search directories
	const Common::FSNode gamePath(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gamePath, "vids");
	SearchMan.addSubDirectoryMatching(gamePath, "music");

	// Initialize random number source
	_rnd = new Common::RandomSource("asylum");
}

AsylumEngine::~AsylumEngine() {
	_handler = nullptr;

	delete _cursor;
	delete _scene;
	delete _encounter;
	delete _puzzles;
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

	_previousScene = nullptr;

	delete _rnd;

	// Zero passed pointers
	_gameDescription = nullptr;
}

Common::Error AsylumEngine::run() {
	// Initialize the graphics
	initGraphics(640, 480);

	// Create debugger. It requires GFX to be initialized
	_console   = new Console(this);
	setDebugger(_console);

	// Create resource manager
	_resource  = new ResourceManager(this);
	_resource->setCdNumber(1);

	// Create all game classes
	_encounter = new Encounter(this);
	_cursor    = new Cursor(this);
	_puzzles   = new Puzzles(this);
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
	if (checkGameVersion("Demo")) {
		if (!isAltDemo())
			_video->play(0, nullptr);
		restart();
	} else {
		int saveSlot = ConfMan.hasKey("save_slot") ? ConfMan.getInt("save_slot") : -1;
		bool noError = false;

		if (saveSlot >= 0 && saveSlot < SAVEGAME_COUNT) {
			if (loadGameState(saveSlot).getCode() != Common::kNoError)
				warning("[AsylumEngine::run] Could not load savegame in slot %d", saveSlot);
			else
				noError = true;
		}

		if (!noError)
			_handler = _menu;
		else
			menu()->setLoadingDuringStartup();

		// Load config
		Config.read();
	}

	// Setup mixer
	syncSoundSettings();

	// Set up achievements system
	Common::String gameTarget = ConfMan.getActiveDomainName();
	AchMan.setActiveDomain(getMetaEngine()->getAchievementsInfo(gameTarget));

	// Send init event to our default event handler
	AsylumEvent initEvt(EVENT_ASYLUM_INIT);
	if (_handler)
		_handler->handleEvent(initEvt);

	// Start running event loop
	while (!shouldQuit()) {
		handleEvents();

		_system->delayMillis(10);

		screen()->processPaletteFadeQueue();

		_system->updateScreen();

		if (_scene)
			checkAchievements();
	}

	// Stop all sounds & music
	_sound->stopMusic();
	_sound->stopAll();

	return Common::kNoError;
}

bool AsylumEngine::startGame(ResourcePackId sceneId, StartGameType type) {
	if (!_cursor || !_screen || !_savegame)
		error("[AsylumEngine::startGame] Subsystems not initialized properly!");

	if (type == kStartGameLoad && !_savegame->isCompatible()) {
		// I18N: Warn user about loading potentially incompatible saved game
		Common::U32String message = _("WARNING: Attempt to load saved game from a previous version: Version %s / Build %d");
		GUI::MessageDialog dialog(Common::U32String::format(message, _savegame->getVersion(), _savegame->getBuild()), _("Load anyway"), _("Cancel"));

		if (dialog.runModal() != GUI::kMessageOK) {
			_menu->setDword455C80(false);
			return false;
		}
	}

	// Load the default mouse cursor
	// Original sets the cursor to a rotating disc
	_cursor->set(MAKE_RESOURCE(sceneId, 10));
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
		// Stop all sounds & music
		_sound->stopMusic();
		_sound->stopAll();
		_savegame->load();
		_scene->enterLoad();
		updateReverseStereo();
		break;

	case kStartGameScene:
		_scene->enter(sceneId);
		break;
	}

	_cursor->show();
	return true;
}

void AsylumEngine::restart() {
	if (!_cursor || !_script)
		error("[AsylumEngine::restart] Subsystems not initialized properly!");

	_cursor->hide();

	// Cleanup
	resetFlags();
	delete _scene;
	_scene = nullptr;
	delete _encounter;
	_encounter = new Encounter(this);
	_script->resetQueue();

	_data.setGlobalPoint(Common::Point(-1, -1));

	reset();

	_introPlayed = false;

	_screen->clear();
	_sound->playMusic(kResourceNone, 0);

	(void)startGame(kResourcePackTowerCells, kStartGamePlayIntro);
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
		if (!Config.showIntro && !checkGameVersion("Demo")) {
			if (_scene->worldstats()->chapter == kChapter1)
				_sound->playMusic(MAKE_RESOURCE(kResourcePackMusic, _scene->worldstats()->musicCurrentResourceIndex));
		} else {
			_sound->playMusic(kResourceNone, 0);

			_video->play(1, checkGameVersion("Demo") ? nullptr : _menu);

			if (_scene->worldstats()->musicCurrentResourceIndex != kMusicStopped)
				_sound->playMusic(MAKE_RESOURCE(kResourcePackMusic, _scene->worldstats()->musicCurrentResourceIndex));

			_screen->clear();

			setGameFlag(kGameFlag4);
			setGameFlag(kGameFlag12);

			// Play the intro speech: it is played after the intro video over a black background,
			// and the game is "locked" until the speech is completed.
			ResourceId introSpeech = MAKE_RESOURCE(checkGameVersion("Demo") ? kResourcePackShared : kResourcePackSound, 7);
			_sound->playSound(introSpeech);

			bool skip = false;
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
			_scene->getActor(0)->changeStatus(kActorStatusDisabled);

		// Fade screen to black
		_screen->paletteFade(0, 75, 8);
		_screen->clear();

		// Stop all sounds & music
		_sound->stopMusic();
		_sound->stopAll();

		resetFlags();

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
	if (handler == nullptr && !checkGameVersion("Demo"))
		warning("[AsylumEngine::switchMessageHandler] NULL handler parameter (shouldn't happen outside of debug commands)!");

	// De-init previous handler
	if (_handler != nullptr) {
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
			_previousScene = nullptr;
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
	if (_handler == nullptr)
		error("[AsylumEngine::notify] Invalid handler parameter (cannot be NULL)!");

	AsylumEvent evt(type, param1, param2);
	_handler->handleEvent(evt);
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

bool AsylumEngine::areGameFlagsSet(uint from, uint to) const {
	while (from <= to)
		if (isGameFlagNotSet((GameFlag)from++))
			return false;

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Steam achievements
//////////////////////////////////////////////////////////////////////////
void AsylumEngine::unlockAchievement(const Common::String &id) {
	AchMan.setAchievement(id);
}

void AsylumEngine::checkAchievements() {
	switch (_scene->worldstats()->chapter) {
	default:
		return;

	case kChapter2:
		if (isGameFlagSet(kGameFlag128) && !isGameFlagSet(kGameFlag3189)) {
			unlockAchievement("ASYLUM_HIDE_AND_SEEK");
			setGameFlag(kGameFlag3189);
		}
		break;

	case kChapter3:
		if (isGameFlagSet(kGameFlag86) && !isGameFlagSet(kGameFlag3386))
			setGameFlag(kGameFlag3386);
		if (isGameFlagSet(kGameFlag87) && !isGameFlagSet(kGameFlag3387))
			setGameFlag(kGameFlag3387);
		if (isGameFlagSet(kGameFlag88) && !isGameFlagSet(kGameFlag3388))
			setGameFlag(kGameFlag3388);

		if (areGameFlagsSet(kGameFlag3386, kGameFlag3388) && !isGameFlagSet(kGameFlag3389)) {
			unlockAchievement("ASYLUM_DANCE");
			setGameFlag(kGameFlag3389);
		}
		break;

	case kChapter5:
		if (!isGameFlagSet(kGameFlag3351) && areGameFlagsSet(kGameFlag284, kGameFlag289)) {
			unlockAchievement("ASYLUM_PASSWORD");
			setGameFlag(kGameFlag3351);
		}
		break;

	case kChapter6:
		if (!isGameFlagSet(kGameFlag3754) && isGameFlagSet(kGameFlagSolveHiveMachine) && !isGameFlagSet(kGameFlag3755)) {
			unlockAchievement("ASYLUM_MELODY");
			setGameFlag(kGameFlag3755);
		}
		break;

	case kChapter8:
		if (!isGameFlagSet(kGameFlag3842) && areGameFlagsSet(kGameFlag3810, kGameFlag3823)) {
			unlockAchievement("ASYLUM_SOCIAL");
			setGameFlag(kGameFlag3842);
		}

		if (!isGameFlagSet(kGameFlag3843) && isGameFlagSet(kGameFlag899)) {
			unlockAchievement("ASYLUM_SORT");
			setGameFlag(kGameFlag3843);
		}
		break;
	}
}

//////////////////////////////////////////////////////////////////////////
// Save/Load
//////////////////////////////////////////////////////////////////////////
bool AsylumEngine::canLoadGameStateCurrently() {
	return (!checkGameVersion("Demo")
		&& (_handler == _scene || _handler == _menu)
		&& !speech()->getSoundResourceId());
}

bool AsylumEngine::canSaveGameStateCurrently() {
	return (!checkGameVersion("Demo")
		&& (_handler == _scene)
		&& !speech()->getSoundResourceId());
}

bool AsylumEngine::canSaveAutosaveCurrently() {
	return canSaveGameStateCurrently()
		&& (scene()->getActor()->getStatus() == kActorStatusEnabled);
}

Common::Error AsylumEngine::loadGameState(int slot) {
	savegame()->loadList();
	savegame()->setIndex(slot);
	if (savegame()->hasSavegame(slot))
		return startGame(savegame()->getScenePack(), AsylumEngine::kStartGameLoad) ? Common::kNoError : Common::kReadingFailed;
	else
		return Common::kReadingFailed;
}

Common::Error AsylumEngine::saveGameState(int slot, const Common::String &desc, bool isAutosave) {
	savegame()->loadList();
	savegame()->setIndex(slot);
	savegame()->setName(slot, desc);
	savegame()->save();

	return Common::kNoError;
}

//////////////////////////////////////////////////////////////////////////
// Misc
//////////////////////////////////////////////////////////////////////////
bool AsylumEngine::rectContains(const int16 (*rectPtr)[4], const Common::Point &p) const {
	return ((*rectPtr)[0] <= p.x) && (p.x < (*rectPtr)[2]) && ((*rectPtr)[1] <= p.y) && (p.y < (*rectPtr)[3]);
}

} // namespace Asylum
