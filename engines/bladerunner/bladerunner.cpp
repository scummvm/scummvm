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
#include "bladerunner/actor_dialogue_queue.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_cache.h"
#include "bladerunner/audio_mixer.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/chapters.h"
#include "bladerunner/combat.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/debugger.h"
#include "bladerunner/dialogue_menu.h"
#include "bladerunner/font.h"
#include "bladerunner/game_flags.h"
#include "bladerunner/game_info.h"
#include "bladerunner/image.h"
#include "bladerunner/item_pickup.h"
#include "bladerunner/items.h"
#include "bladerunner/lights.h"
#include "bladerunner/mouse.h"
#include "bladerunner/music.h"
#include "bladerunner/outtake.h"
#include "bladerunner/obstacles.h"
#include "bladerunner/overlays.h"
#include "bladerunner/regions.h"
#include "bladerunner/savefile.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/set.h"
#include "bladerunner/script/ai_script.h"
#include "bladerunner/script/init_script.h"
#include "bladerunner/script/kia_script.h"
#include "bladerunner/script/police_maze.h"
#include "bladerunner/script/scene_script.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/subtitles.h"
#include "bladerunner/suspects_database.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/time.h"
#include "bladerunner/ui/elevator.h"
#include "bladerunner/ui/end_credits.h"
#include "bladerunner/ui/esper.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/scores.h"
#include "bladerunner/ui/spinner.h"
#include "bladerunner/ui/vk.h"
#include "bladerunner/vqa_decoder.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

#include "common/array.h"
#include "common/config-manager.h"
#include "common/error.h"
#include "common/events.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/debug-channels.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "graphics/pixelformat.h"

namespace BladeRunner {

BladeRunnerEngine::BladeRunnerEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst),
	  _rnd("bladerunner") {

	DebugMan.addDebugChannel(kDebugScript, "Script", "Debug the scripts");

	_windowIsActive = true;
	_gameIsRunning  = true;

	_vqaIsPlaying       = false;
	_vqaStopIsRequested = false;

	_actorIsSpeaking           = false;
	_actorSpeakStopIsRequested = false;

	_subtitlesEnabled = false;
	_sitcomMode       = false;
	_shortyMode       = false;

	_playerLosesControlCounter = 0;

	_playerActorIdle = false;
	_playerDead      = false;

	_gameOver        = false;
	_gameAutoSave    = -1;
	_gameIsLoading   = false;
	_sceneIsLoading  = false;

	_runningActorId         = -1;
	_isWalkingInterruptible = false;
	_interruptWalking       = false;

	_walkSoundId      = -1;
	_walkSoundVolume  = 0;
	_walkSoundBalance = 0;

	_crimesDatabase = nullptr;

	switch (desc->language) {
	case Common::EN_ANY:
		_languageCode = "E";
		break;
	case Common::DE_DEU:
		_languageCode = "G";
		break;
	case Common::FR_FRA:
		_languageCode = "F";
		break;
	case Common::IT_ITA:
		_languageCode = "I";
		break;
	case Common::RU_RUS:
		_languageCode = "R";
		break;
	case Common::ES_ESP:
		_languageCode = "S";
		break;
	default:
		_languageCode = "E";
	}

	_screenEffects           = nullptr;
	_combat                  = nullptr;
	_actorDialogueQueue      = nullptr;
	_settings                = nullptr;
	_itemPickup              = nullptr;
	_lights                  = nullptr;
	_obstacles               = nullptr;
	_sceneScript             = nullptr;
	_time                    = nullptr;
	_gameInfo                = nullptr;
	_waypoints               = nullptr;
	_gameVars                = nullptr;
	_cosTable1024            = nullptr;
	_sinTable1024            = nullptr;
	_view                    = nullptr;
	_sceneObjects            = nullptr;
	_gameFlags               = nullptr;
	_items                   = nullptr;
	_audioCache              = nullptr;
	_audioMixer              = nullptr;
	_audioPlayer             = nullptr;
	_music                   = nullptr;
	_audioSpeech             = nullptr;
	_ambientSounds           = nullptr;
	_chapters                = nullptr;
	_overlays                = nullptr;
	_zbuffer                 = nullptr;
	_playerActor             = nullptr;
	_textActorNames          = nullptr;
	_textCrimes              = nullptr;
	_textClueTypes           = nullptr;
	_textKIA                 = nullptr;
	_textSpinnerDestinations = nullptr;
	_textVK                  = nullptr;
	_textOptions             = nullptr;
	_dialogueMenu            = nullptr;
	_suspectsDatabase        = nullptr;
	_kia                     = nullptr;
	_endCredits              = nullptr;
	_spinner                 = nullptr;
	_scores                  = nullptr;
	_elevator                = nullptr;
	_mainFont                = nullptr;
	_subtitles               = nullptr;
	_esper                   = nullptr;
	_vk                      = nullptr;
	_policeMaze              = nullptr;
	_mouse                   = nullptr;
	_sliceAnimations         = nullptr;
	_sliceRenderer           = nullptr;
	_crimesDatabase          = nullptr;
	_scene                   = nullptr;
	_aiScripts               = nullptr;
	for (int i = 0; i != kActorCount; ++i) {
		_actors[i]           = nullptr;
	}
	_debugger                = nullptr;
	walkingReset();

	_actorUpdateCounter = 0;
}

BladeRunnerEngine::~BladeRunnerEngine() {
}

bool BladeRunnerEngine::hasFeature(EngineFeature f) const {
	return
		f == kSupportsRTL ||
		f == kSupportsLoadingDuringRuntime ||
		f == kSupportsSavingDuringRuntime;
}

bool BladeRunnerEngine::canLoadGameStateCurrently() {
	return
		playerHasControl() &&
		!_sceneScript->isInsideScript() &&
		!_aiScripts->isInsideScript() &&
		!_kia->isOpen() &&
		!_spinner->isOpen() &&
		!_vk->isOpen() &&
		!_elevator->isOpen();
}

Common::Error BladeRunnerEngine::loadGameState(int slot) {
	Common::InSaveFile *saveFile = BladeRunner::SaveFileManager::openForLoading(_targetName, slot);
	if (saveFile == nullptr || saveFile->err()) {
		delete saveFile;
		return Common::kReadingFailed;
	}

	BladeRunner::SaveFileHeader header;
	if (!BladeRunner::SaveFileManager::readHeader(*saveFile, header)) {
		error("Invalid savegame");
	}

	loadGame(*saveFile);

	delete saveFile;

	return Common::kNoError;
}

bool BladeRunnerEngine::canSaveGameStateCurrently() {
	return
		playerHasControl() &&
		!_sceneScript->isInsideScript() &&
		!_aiScripts->isInsideScript() &&
		!_kia->isOpen() &&
		!_spinner->isOpen() &&
		!_vk->isOpen() &&
		!_elevator->isOpen();
}

Common::Error BladeRunnerEngine::saveGameState(int slot, const Common::String &desc) {
	Common::OutSaveFile *saveFile = BladeRunner::SaveFileManager::openForSaving(_targetName, slot);
	if (saveFile == nullptr || saveFile->err()) {
		delete saveFile;
		return Common::kReadingFailed;
	}

	Graphics::Surface thumbnail = generateThumbnail();

	BladeRunner::SaveFileHeader header;
	header._name = desc;

	BladeRunner::SaveFileManager::writeHeader(*saveFile, header);

	_time->pause();
	saveGame(*saveFile, thumbnail);
	_time->resume();

	saveFile->finalize();

	thumbnail.free();

	delete saveFile;

	return Common::kNoError;
}

void BladeRunnerEngine::pauseEngineIntern(bool pause) {
	_mixer->pauseAll(pause);
}

Common::Error BladeRunnerEngine::run() {
	Graphics::PixelFormat format = createRGB555();
	initGraphics(640, 480, &format);

	_system->showMouse(true);

	bool hasSavegames = !SaveFileManager::list(_targetName).empty();

	if (!startup(hasSavegames)) {
		shutdown();
		return Common::Error(Common::kUnknownError, "Failed to initialize resources");
	}

	if (warnUserAboutUnsupportedGame()) {
		if (hasSavegames) {
			_kia->_forceOpen = true;
			_kia->open(kKIASectionLoad);
		}
		// TODO: why is game starting new game here when everything is done in startup?
		//  else {
		// 	newGame(1);
		// }

		gameLoop();

		_mouse->disable();

		if (_gameOver) {
			autoSaveGame(4, true);
			_endCredits->show();
		}
	}

	shutdown();

	return Common::kNoError;
}

bool BladeRunnerEngine::startup(bool hasSavegames) {
	// These are static objects in original game

	_screenEffects = new ScreenEffects(this, 0x8000);

	_endCredits = new EndCredits(this);

	_actorDialogueQueue = new ActorDialogueQueue(this);

	_settings = new Settings(this);

	_itemPickup = new ItemPickup(this);

	_lights = new Lights(this);

	// outtake player was initialized here in the original game - but this is done bit differently

	_obstacles = new Obstacles(this);

	_sceneScript = new SceneScript(this);

	_debugger = new Debugger(this);

	// This is the original startup in the game

	_surfaceFront.create(640, 480, createRGB555());
	_surfaceBack.create(640, 480, createRGB555());

	_time = new Time(this);

	// Try to load the SUBTITLES.MIX first, before Startup.MIX
	// allows overriding any identically named resources (such as the original font files and as a bonus also the TRE files for the UI and dialogue menu)
	_subtitles = new Subtitles(this);
	bool r = openArchive("SUBTITLES.MIX");
	if (!r) {
		_subtitles->setSubtitlesSystemInactive(true); // no subtitles support
	}
	_subtitles->init();

	r = openArchive("STARTUP.MIX");
	if (!r)
		return false;

	_gameInfo = new GameInfo(this);
	if (!_gameInfo)
		return false;

	r = _gameInfo->open("GAMEINFO.DAT");
	if (!r) {
		return false;
	}

	if (hasSavegames) {
		if (!loadSplash()) {
			return false;
		}
	}

	_waypoints = new Waypoints(this, _gameInfo->getWaypointCount());

	_combat = new Combat(this);

	_gameVars = new int[_gameInfo->getGlobalVarCount()]();

	// Seed rand

	_cosTable1024 = new Common::CosineTable(1024); // 10-bits = 1024 points for 2*PI;
	_sinTable1024 = new Common::SineTable(1024);

	_view = new View();

	_sceneObjects = new SceneObjects(this, _view);

	_gameFlags = new GameFlags();
	_gameFlags->setFlagCount(_gameInfo->getFlagCount());

	// Assign default values to the ScummVM configuration manager, in case settings are missing
	ConfMan.registerDefault("subtitles", "true");
	// get value from the ScummVM configuration manager
	_subtitlesEnabled = ConfMan.getBool("subtitles");

	_sitcomMode = ConfMan.getBool("sitcom");
	_shortyMode = ConfMan.getBool("shorty");

	_items = new Items(this);

	_audioCache = new AudioCache();

	_audioMixer = new AudioMixer(this);

	_audioPlayer = new AudioPlayer(this);

	_music = new Music(this);

	_audioSpeech = new AudioSpeech(this);

	_ambientSounds = new AmbientSounds(this);

	// BLADE.INI was read here, but it was replaced by ScummVM configuration

	_chapters = new Chapters(this);
	if (!_chapters)
		return false;

	if (!openArchive("MUSIC.MIX"))
		return false;

	if (!openArchive("SFX.MIX"))
		return false;

	if (!openArchive("SPCHSFX.TLK"))
		return false;

	_overlays = new Overlays(this);
	_overlays->init();

	_zbuffer = new ZBuffer();
	_zbuffer->init(640, 480);

	int actorCount = (int)_gameInfo->getActorCount();
	assert(actorCount < kActorCount);
	for (int i = 0; i != actorCount; ++i) {
		_actors[i] = new Actor(this, i);
	}
	_actors[kActorVoiceOver] = new Actor(this, kActorVoiceOver);
	_playerActor = _actors[_gameInfo->getPlayerId()];

	_playerActor->setFPS(15);
	_playerActor->timerStart(6, 200);

	_policeMaze = new PoliceMaze(this);

	_textActorNames = new TextResource(this);
	if (!_textActorNames->open("ACTORS"))
		return false;

	_textCrimes = new TextResource(this);
	if (!_textCrimes->open("CRIMES"))
		return false;

	_textClueTypes = new TextResource(this);
	if (!_textClueTypes->open("CLUETYPE"))
		return false;

	_textKIA = new TextResource(this);
	if (!_textKIA->open("KIA"))
		return false;

	_textSpinnerDestinations = new TextResource(this);
	if (!_textSpinnerDestinations->open("SPINDEST"))
		return false;

	_textVK = new TextResource(this);
	if (!_textVK->open("VK"))
		return false;

	_textOptions = new TextResource(this);
	if (!_textOptions->open("OPTIONS"))
		return false;

	_dialogueMenu = new DialogueMenu(this);
	if (!_dialogueMenu->loadText("DLGMENU"))
		return false;

	_suspectsDatabase = new SuspectsDatabase(this, _gameInfo->getSuspectCount());

	_kia = new KIA(this);

	_spinner = new Spinner(this);

	_elevator = new Elevator(this);

	_scores = new Scores(this);

	_mainFont = new Font(this);
	_mainFont->open("KIA6PT.FON", 640, 480, -1, 0, 0x252D);
	_mainFont->setSpacing(1, 0);

	for (int i = 0; i != 43; ++i) {
		Shape *shape = new Shape(this);
		shape->open("SHAPES.SHP", i);
		_shapes.push_back(shape);
	}

	_esper = new ESPER(this);

	_vk = new VK(this);

	_mouse = new Mouse(this);
	_mouse->setCursor(0);

	_sliceAnimations = new SliceAnimations(this);
	r = _sliceAnimations->open("INDEX.DAT");
	if (!r)
		return false;

	r = _sliceAnimations->openCoreAnim();
	if (!r) {
		return false;
	}

	_sliceRenderer = new SliceRenderer(this);
	_sliceRenderer->setScreenEffects(_screenEffects);

	_crimesDatabase = new CrimesDatabase(this, "CLUES", _gameInfo->getClueCount());

	_scene = new Scene(this);

	// Load INIT.DLL
	InitScript initScript(this);
	initScript.SCRIPT_Initialize_Game();

	// Load AI-ACT1.DLL
	_aiScripts = new AIScripts(this, actorCount);

	initChapterAndScene();

	return true;
}

void BladeRunnerEngine::initChapterAndScene() {
	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_aiScripts->initialize(i);
	}

	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_actors[i]->changeAnimationMode(kAnimationModeIdle);
	}

	for (int i = 1, end = _gameInfo->getActorCount(); i != end; ++i) { // skip first actor, probably player
		_actors[i]->movementTrackNext(true);
	}

	if (ConfMan.hasKey("boot_param")) {
		int param = ConfMan.getInt("boot_param"); // CTTTSSS
		int chapter = param / 1000000;
		param -= chapter * 1000000;
		int set = param / 1000;
		param -= set * 1000;
		int scene = param;

		_settings->setChapter(chapter);
		_settings->setNewSetAndScene(set, scene);
	} else {
		_settings->setChapter(1);
		_settings->setNewSetAndScene(_gameInfo->getInitialSetId(), _gameInfo->getInitialSceneId());
	}
}

void BladeRunnerEngine::shutdown() {
	_mixer->stopAll();

	// BLADE.INI as updated here

	delete _aiScripts;
	_aiScripts = nullptr;

	delete _scene;
	_scene = nullptr;

	delete _crimesDatabase;
	_crimesDatabase = nullptr;

	delete _sliceRenderer;
	_sliceRenderer = nullptr;

	delete _sliceAnimations;
	_sliceAnimations = nullptr;

	delete _mouse;
	_mouse = nullptr;

	delete _vk;
	_vk = nullptr;

	delete _esper;
	_esper = nullptr;

	/// todo
	for (uint i = 0; i != _shapes.size(); ++i) {
		delete _shapes[i];
	}
	_shapes.clear();

	if (_mainFont) {
		_mainFont->close();
		delete _mainFont;
		_mainFont = nullptr;
	}

	delete _scores;
	_scores = nullptr;

	delete _elevator;
	_elevator = nullptr;

	delete _spinner;
	_spinner = nullptr;

	delete _kia;
	_kia = nullptr;

	delete _suspectsDatabase;
	_suspectsDatabase = nullptr;

	delete _dialogueMenu;
	_dialogueMenu = nullptr;

	delete _textOptions;
	_textOptions = nullptr;

	delete _textVK;
	_textVK = nullptr;

	delete _textSpinnerDestinations;
	_textSpinnerDestinations = nullptr;

	delete _textKIA;
	_textKIA = nullptr;

	delete _textClueTypes;
	_textClueTypes = nullptr;

	delete _textCrimes;
	_textCrimes = nullptr;

	delete _textActorNames;
	_textActorNames = nullptr;

	delete _policeMaze;
	_policeMaze = nullptr;

	_playerActor = nullptr;
	delete _actors[kActorVoiceOver];
	_actors[kActorVoiceOver] = nullptr;
	int actorCount = (int)_gameInfo->getActorCount();
	for (int i = 0; i < actorCount; ++i) {
		delete _actors[i];
		_actors[i] = nullptr;
	}

	delete _zbuffer;
	_zbuffer = nullptr;

	delete _overlays;
	_overlays = nullptr;

	if (isArchiveOpen("SPCHSFX.TLK")) {
		closeArchive("SPCHSFX.TLK");
	}

	if (isArchiveOpen("SFX.MIX")) {
		closeArchive("SFX.MIX");
	}

	if (isArchiveOpen("MUSIC.MIX")) {
		closeArchive("MUSIC.MIX");
	}

	if (_chapters) {
		if (_chapters->hasOpenResources())
			_chapters->closeResources();
		delete _chapters;
		_chapters = nullptr;
	}

	delete _ambientSounds;
	_ambientSounds = nullptr;

	delete _audioSpeech;
	_audioSpeech = nullptr;

	delete _music;
	_music = nullptr;

	delete _audioPlayer;
	_audioPlayer = nullptr;

	delete _audioMixer;
	_audioMixer = nullptr;

	delete _audioCache;
	_audioCache = nullptr;

	delete _items;
	_items = nullptr;

	delete _gameFlags;
	_gameFlags = nullptr;

	delete _sceneObjects;
	_sceneObjects = nullptr;

	delete _view;
	_view = nullptr;

	delete _sinTable1024;
	_sinTable1024 = nullptr;
	delete _cosTable1024;
	_cosTable1024 = nullptr;

	delete[] _gameVars;
	_gameVars = nullptr;

	delete _combat;
	_combat = nullptr;

	delete _waypoints;
	_waypoints = nullptr;

	delete _gameInfo;
	_gameInfo = nullptr;

	if (isArchiveOpen("STARTUP.MIX")) {
		closeArchive("STARTUP.MIX");
	}

	if (isArchiveOpen("SUBTITLES.MIX")) {
		closeArchive("SUBTITLES.MIX");
	}
	if (_subtitles) {
		delete _subtitles;
		_subtitles = nullptr;
	}

	delete _time;
	_time = nullptr;

	_surfaceBack.free();
	_surfaceFront.free();

	// These are static objects in original game

	delete _debugger;
	_debugger = nullptr;

	delete _sceneScript;
	_sceneScript = nullptr;

	delete _obstacles;
	_obstacles = nullptr;

	delete _lights;
	_lights = nullptr;

	delete _itemPickup;
	_itemPickup = nullptr;

	delete _settings;
	_settings = nullptr;

	delete _actorDialogueQueue;
	_actorDialogueQueue = nullptr;

	delete _endCredits;
	_endCredits = nullptr;

	delete _screenEffects;
	_screenEffects = nullptr;
}

bool BladeRunnerEngine::loadSplash() {
	Image img(this);
	if (!img.open("SPLASH.IMG")) {
		return false;
	}

	img.copyToSurface(&_surfaceFront);

	blitToScreen(_surfaceFront);

	return true;
}

Common::Point BladeRunnerEngine::getMousePos() const {
	Common::Point p = _eventMan->getMousePos();
	p.x = CLIP(p.x, int16(0), int16(639));
	p.y = CLIP(p.y, int16(0), int16(479));
	return p;
}

bool BladeRunnerEngine::isMouseButtonDown() const {
	return _eventMan->getButtonState() != 0;
}

void BladeRunnerEngine::gameLoop() {
	_gameIsRunning = true;
	do {
		if (_playerDead) {
			playerDied();
			_playerDead = false;
		}
		gameTick();
	} while (_gameIsRunning);
}

void BladeRunnerEngine::gameTick() {
	handleEvents();

	if (!_gameIsRunning || !_windowIsActive) {
		return;
	}

	if (!_kia->isOpen() && !_sceneScript->isInsideScript() && !_aiScripts->isInsideScript()) {
		_settings->openNewScene();
	}

	if (_gameAutoSave >= 0) {
		autoSaveGame(_gameAutoSave, false);
		_gameAutoSave = -1;
	}

	//probably not needed, this version of tick is just loading data from buffer
	//_audioMixer->tick();

	if (_kia->isOpen()) {
		_kia->tick();
		return;
	}

	if (_spinner->isOpen()) {
		_spinner->tick();
		_ambientSounds->tick();
		return;
	}

	if (_esper->isOpen()) {
		_esper->tick();
		return;
	}

	if (_vk->isOpen()) {
		_vk->tick();
		_ambientSounds->tick();
		return;
	}

	if (_elevator->isOpen()) {
		_elevator->tick();
		_ambientSounds->tick();
		return;
	}

	if (_scores->isOpen()) {
		_scores->tick();
		_ambientSounds->tick();
		return;
	}

	_actorDialogueQueue->tick();
	if (_scene->didPlayerWalkIn()) {
		_sceneScript->playerWalkedIn();
	}

	bool inDialogueMenu = _dialogueMenu->isVisible();
	if  (!inDialogueMenu) {
		for (int i = 0; i < (int)_gameInfo->getActorCount(); ++i) {
			_actors[i]->tickCombat();
		}
	}

	_policeMaze->tick();

	_zbuffer->clean();

	_ambientSounds->tick();

	bool backgroundChanged = false;
	int frame = _scene->advanceFrame();
	if (frame >= 0) {
		_sceneScript->sceneFrameAdvanced(frame);
		backgroundChanged = true;
	}
	blit(_surfaceBack, _surfaceFront);

	_overlays->tick();

	if (!inDialogueMenu) {
		actorsUpdate();
	}

	if (_settings->getNewScene() != -1 && !_sceneScript->isInsideScript() && !_aiScripts->isInsideScript()) {
		return;
	}

	_sliceRenderer->setView(_view);

	// Tick and draw all actors in current set
	int setId = _scene->getSetId();
	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		if (_actors[i]->getSetId() == setId) {
			Common::Rect screenRect;
			if (_actors[i]->tick(backgroundChanged, &screenRect)) {
				_zbuffer->mark(screenRect);
			}
		}
	}

	_items->tick();

	_itemPickup->tick();
	_itemPickup->draw();

	Common::Point p = getMousePos();

	if (_dialogueMenu->isVisible()) {
		_dialogueMenu->tick(p.x, p.y);
		_dialogueMenu->draw(_surfaceFront);
	}

	if (_debugger->_viewZBuffer) {
		_surfaceFront.copyRectToSurface(_zbuffer->getData(), 1280, 0, 0, 640, 480);
	}

	_mouse->tick(p.x, p.y);
	_mouse->draw(_surfaceFront, p.x, p.y);

	if (_walkSoundId >= 0) {
		_audioPlayer->playAud(_gameInfo->getSfxTrack(_walkSoundId), _walkSoundVolume, _walkSoundBalance, _walkSoundBalance, 50, 0);
		_walkSoundId = -1;
	}

	if (_debugger->_isDebuggerOverlay) {
		_debugger->drawDebuggerOverlay();
	}

	if (_debugger->_viewObstacles) {
		_obstacles->draw();
	}

	_subtitles->tick(_surfaceFront);

	blitToScreen(_surfaceFront);
	_system->delayMillis(10);
}

void BladeRunnerEngine::actorsUpdate() {
	int actorCount = (int)_gameInfo->getActorCount();
	int setId = _scene->getSetId();

	// what a "nice" last minute fix...
	if ( setId == kSetUG18
	 && _gameVars[kVariableChapter] == 4
	 && _gameFlags->query(kFlagCallWithGuzza)
	 && _aiScripts->isInsideScript()
	) {
		return;
	}

	for (int i = 0; i < actorCount; i++) {
		Actor *actor = _actors[i];
		if (actor->getSetId() == setId || i == _actorUpdateCounter) {
			_aiScripts->update(i);
			actor->timersUpdate();
		}
	}
	++_actorUpdateCounter;
	if (_actorUpdateCounter >= actorCount) {
		_actorUpdateCounter = 0;
	}
}

void BladeRunnerEngine::walkingReset() {
	_mouseClickTimeLast   = 0;
	_mouseClickTimeDiff   = 0;
	_walkingToExitId      = -1;
	_isInsideScriptExit   = false;
	_walkingToRegionId    = -1;
	_isInsideScriptRegion = false;
	_walkingToObjectId    = -1;
	_isInsideScriptObject = false;
	_walkingToItemId      = -1;
	_isInsideScriptItem   = false;
	_walkingToEmpty       = false;
	_walkingToEmptyX      = 0;
	_walkingToEmptyY      = 0;
	_isInsideScriptEmpty  = false;
	_walkingToActorId     = -1;
	_isInsideScriptActor  = false;
}

void BladeRunnerEngine::handleEvents() {
	if (shouldQuit()) {
		_gameIsRunning = false;
		return;
	}

	Common::Event event;
	Common::EventManager *eventMan = _system->getEventManager();
	while (eventMan->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYUP:
			handleKeyUp(event);
			break;
		case Common::EVENT_KEYDOWN:
			handleKeyDown(event);
			break;
		case Common::EVENT_LBUTTONUP:
			handleMouseAction(event.mouse.x, event.mouse.y, true, false);
			break;
		case Common::EVENT_RBUTTONUP:
		case Common::EVENT_MBUTTONUP:
			handleMouseAction(event.mouse.x, event.mouse.y, false, false);
			break;
		case Common::EVENT_LBUTTONDOWN:
			handleMouseAction(event.mouse.x, event.mouse.y, true, true);
			break;
		case Common::EVENT_RBUTTONDOWN:
		case Common::EVENT_MBUTTONDOWN:
			handleMouseAction(event.mouse.x, event.mouse.y, false, true);
			break;
		default:
			; // nothing to do
		}
	}
}

void BladeRunnerEngine::handleKeyUp(Common::Event &event) {
	if (_actorIsSpeaking && event.kbd.keycode == Common::KEYCODE_RETURN) {
		_actorSpeakStopIsRequested = true;
		_actorIsSpeaking = false;

		return;
	}

	if (_vqaIsPlaying) {
		_vqaStopIsRequested = true;
		_vqaIsPlaying = false;

		return;
	}

	if (!playerHasControl() || _isWalkingInterruptible) {
		return;
	}

	if (_kia->isOpen()) {
		_kia->handleKeyUp(event.kbd);
		return;
	}

	if (_spinner->isOpen()) {
		return;
	}

	if (_elevator->isOpen()) {
		return;
	}

	if (_esper->isOpen()) {
		return;
	}

	if (_vk->isOpen()) {
		return;
	}

	if (_dialogueMenu->isOpen()) {
		return;
	}

	if (_scores->isOpen()) {
		return;
	}

	switch (event.kbd.keycode) {
		case Common::KEYCODE_TAB:
			_kia->openLastOpened();
			break;
		case Common::KEYCODE_ESCAPE:
			_kia->open(kKIASectionSettings);
			break;
		case Common::KEYCODE_SPACE:
			_combat->change();
			break;
		default:
			break;
	}
}

void BladeRunnerEngine::handleKeyDown(Common::Event &event) {
	if ((event.kbd.keycode == Common::KEYCODE_d) && (event.kbd.flags & Common::KBD_CTRL)) {
		getDebugger()->attach();
		getDebugger()->onFrame();
		return;
	}

	if (!playerHasControl() || _isWalkingInterruptible || _actorIsSpeaking || _vqaIsPlaying) {
		return;
	}

	if (_kia->isOpen()) {
		_kia->handleKeyDown(event.kbd);
		return;
	}

	if (_spinner->isOpen()) {
		return;
	}

	if (_elevator->isOpen()) {
		return;
	}

	if (_esper->isOpen()) {
		return;
	}

	if (_dialogueMenu->isOpen()) {
		return;
	}

	if (_scores->isOpen()) {
		_scores->handleKeyDown(event.kbd);
		return;
	}

	switch (event.kbd.keycode) {
		case Common::KEYCODE_F1:
			_kia->open(kKIASectionHelp);
			break;
		case Common::KEYCODE_F2:
			_kia->open(kKIASectionSave);
			break;
		case Common::KEYCODE_F3:
			_kia->open(kKIASectionLoad);
			break;
		case Common::KEYCODE_F4:
			_kia->open(kKIASectionCrimes);
			break;
		case Common::KEYCODE_F5:
			_kia->open(kKIASectionSuspects);
			break;
		case Common::KEYCODE_F6:
			_kia->open(kKIASectionClues);
			break;
		case Common::KEYCODE_F10:
			_kia->open(kKIASectionQuit);
			break;
		default:
			break;
	}
}

void BladeRunnerEngine::handleMouseAction(int x, int y, bool mainButton, bool buttonDown) {
	x = CLIP(x, 0, 639);
	y = CLIP(y, 0, 479);

	int timeNow = _time->current();

	if (buttonDown) {
		_mouseClickTimeDiff = timeNow - _mouseClickTimeLast;
		_mouseClickTimeLast = timeNow;
	}

	if (!playerHasControl() || _mouse->isDisabled()) {
		return;
	}

	if (_kia->isOpen()) {
		if (buttonDown) {
			_kia->handleMouseDown(x, y, mainButton);
		} else {
			_kia->handleMouseUp(x, y, mainButton);
		}
		return;
	}

	if (_spinner->isOpen()) {
		if (buttonDown) {
			_spinner->handleMouseDown(x, y);
		} else {
			_spinner->handleMouseUp(x, y);
		}
		return;
	}

	if (_esper->isOpen()) {
		if (buttonDown) {
			_esper->handleMouseDown(x, y, mainButton);
		} else {
			_esper->handleMouseUp(x, y, mainButton);
		}
		return;
	}

	if (_vk->isOpen()) {
		if (buttonDown) {
			_vk->handleMouseDown(x, y, mainButton);
		} else {
			_vk->handleMouseUp(x, y, mainButton);
		}
		return;
	}

	if (_elevator->isOpen()) {
		if (buttonDown) {
			_elevator->handleMouseDown(x, y);
		} else {
			_elevator->handleMouseUp(x, y);
		}
		return;
	}

	if (_scores->isOpen()) {
		if (buttonDown) {
			_scores->handleMouseDown(x, y);
		} else {
			_scores->handleMouseUp(x, y);
		}
		return;
	}

	if (_dialogueMenu->waitingForInput()) {
		if (mainButton && !buttonDown) {
			_dialogueMenu->mouseUp();
		}
		return;
	}

	if (mainButton) {
		Vector3 scenePosition = _mouse->getXYZ(x, y);

		bool isClickable;
		bool isObstacle;
		bool isTarget;

		int sceneObjectId = _sceneObjects->findByXYZ(&isClickable, &isObstacle, &isTarget, scenePosition, true, false, true);
		int exitIndex = _scene->_exits->getRegionAtXY(x, y);
		int regionIndex = _scene->_regions->getRegionAtXY(x, y);

		if ((sceneObjectId < kSceneObjectOffsetActors || sceneObjectId >= kSceneObjectOffsetItems) && exitIndex >= 0) {
			handleMouseClickExit(exitIndex, x, y, buttonDown);
		} else if (regionIndex >= 0) {
			handleMouseClickRegion(regionIndex, x, y, buttonDown);
		} else if (sceneObjectId == -1) {
			handleMouseClickEmpty(x, y, scenePosition, buttonDown);
		} else if (sceneObjectId >= kSceneObjectOffsetActors && sceneObjectId < kSceneObjectOffsetItems) {
			handleMouseClickActor(sceneObjectId - kSceneObjectOffsetActors, mainButton, buttonDown, scenePosition, x, y);
		} else if (sceneObjectId >= kSceneObjectOffsetItems && sceneObjectId < kSceneObjectOffsetObjects) {
			handleMouseClickItem(sceneObjectId - kSceneObjectOffsetItems, buttonDown);
		} else if (sceneObjectId >= kSceneObjectOffsetObjects && sceneObjectId <= 293) {
			handleMouseClick3DObject(sceneObjectId - kSceneObjectOffsetObjects, buttonDown, isClickable, isTarget);
		}
	} else if (buttonDown) {
		if (_playerActor->mustReachWalkDestination()) {
			if (!_isWalkingInterruptible) {
				return;
			}
			_playerActor->stopWalking(false);
			_interruptWalking = true;
		}
		_combat->change();
	}
}

void BladeRunnerEngine::handleMouseClickExit(int exitId, int x, int y, bool buttonDown) {
	if (_isWalkingInterruptible && exitId != _walkingToExitId) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToExitId = exitId;
		return;
	}

	if (buttonDown) {
		return;
	}

	if (_isInsideScriptExit && exitId == _walkingToExitId) {
		_playerActor->run();
		if (_mouseClickTimeDiff <= 10000) {
			_playerActor->increaseFPS();
		}
	} else {
		_walkingToExitId   = exitId;
		_walkingToRegionId = -1;
		_walkingToObjectId = -1;
		_walkingToItemId   = -1;
		_walkingToEmpty    = false;
		_walkingToActorId  = -1;

		_isInsideScriptExit = true;
		_sceneScript->clickedOnExit(exitId);
		_isInsideScriptExit = false;
	}
}

void BladeRunnerEngine::handleMouseClickRegion(int regionId, int x, int y, bool buttonDown) {
	if (_isWalkingInterruptible && regionId != _walkingToRegionId) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToRegionId = regionId;
		return;
	}

	if (buttonDown || _mouse->isInactive()) {
		return;
	}

	if (_isInsideScriptRegion && regionId == _walkingToRegionId) {
		_playerActor->run();
		if (_mouseClickTimeDiff <= 10000) {
			_playerActor->increaseFPS();
		}
	} else {
		_walkingToExitId   = -1;
		_walkingToRegionId = regionId;
		_walkingToObjectId = -1;
		_walkingToItemId   = -1;
		_walkingToEmpty    = false;
		_walkingToActorId  = -1;

		_isInsideScriptRegion = true;
		_sceneScript->clickedOn2DRegion(regionId);
		_isInsideScriptRegion = false;
	}
}

void BladeRunnerEngine::handleMouseClick3DObject(int objectId, bool buttonDown, bool isClickable, bool isTarget) {
	const Common::String &objectName = _scene->objectGetName(objectId);

	if (_isWalkingInterruptible && objectId != _walkingToObjectId) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToObjectId = objectId;
		return;
	}

	if (_mouse->isInactive()) {
		return;
	}

	if (!_combat->isActive()) {
		if (buttonDown || !isClickable) {
			return;
		}

		if (_isInsideScriptObject && objectId == _walkingToObjectId) {
			_playerActor->run();
			if (_mouseClickTimeDiff <= 10000) {
				_playerActor->increaseFPS();
			}
		} else {
			_walkingToExitId   = -1;
			_walkingToRegionId = -1;
			_walkingToObjectId = objectId;
			_walkingToItemId   = -1;
			_walkingToEmpty    = false;
			_walkingToActorId  = -1;

			_isInsideScriptObject = true;
			_sceneScript->clickedOn3DObject(objectName.c_str(), false);
			_isInsideScriptObject = false;
		}
	} else {
		if (!buttonDown || !isTarget) {
			return;
		}
		_playerActor->stopWalking(false);
		_playerActor->faceObject(objectName, false);
		_playerActor->changeAnimationMode(kAnimationModeCombatAttack, false);
		_settings->decreaseAmmo();
		_audioPlayer->playAud(_gameInfo->getSfxTrack(_combat->getHitSound()), 100, 0, 0, 90, 0);

		_mouse->setMouseJitterUp();

		_isInsideScriptObject = true;
		_sceneScript->clickedOn3DObject(objectName.c_str(), true);
		_isInsideScriptObject = false;
	}
}

void BladeRunnerEngine::handleMouseClickEmpty(int x, int y, Vector3 &scenePosition, bool buttonDown) {
	if (_isWalkingInterruptible) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToEmpty = false;
		return;
	}

	_isInsideScriptEmpty = true;
	bool sceneMouseClick = _sceneScript->mouseClick(x, y);
	_isInsideScriptEmpty = false;

	if (sceneMouseClick) {
		return;
	}

	int actorId = Actor::findTargetUnderMouse(this, x, y);
	int itemId = _items->findTargetUnderMouse(x, y);

	if (_combat->isActive() && buttonDown && (actorId > 0 || itemId > 0)) {
		_playerActor->stopWalking(false);
		if (actorId > 0) {
			_playerActor->faceActor(actorId, false);
		} else {
			_playerActor->faceItem(itemId, false);
		}
		_playerActor->changeAnimationMode(kAnimationModeCombatAttack, false);
		_settings->decreaseAmmo();
		_audioPlayer->playAud(_gameInfo->getSfxTrack(_combat->getMissSound()), 100, 0, 0, 90, 0);

		_mouse->setMouseJitterUp();

		if (actorId > 0) {
			_aiScripts->shotAtAndMissed(actorId);
		}
	} else {
		if (buttonDown) {
			return;
		}

		_walkingToExitId   = -1;
		_walkingToRegionId = -1;
		_walkingToObjectId = -1;
		_walkingToItemId   = -1;
		_walkingToEmpty    = true;
		_walkingToActorId  = -1;

		if (_combat->isActive() && (actorId > 0 || itemId > 0)) {
			return;
		}

		int xDist = abs(_walkingToEmptyX - x);
		int yDist = abs(_walkingToEmptyY - y);

		_walkingToEmptyX = x;
		_walkingToEmptyY = y;

		bool inWalkbox = false;
		float altitude = _scene->_set->getAltitudeAtXZ(scenePosition.x, scenePosition.z, &inWalkbox);

		if (!inWalkbox || scenePosition.y >= altitude + 6.0f) {
			return;
		}

		bool shouldRun = _playerActor->isRunning();
		if (_mouseClickTimeDiff <= 10000 && xDist < 10 && yDist < 10) {
			shouldRun = true;
		}

		_playerActor->walkTo(shouldRun, scenePosition, false);

		if (shouldRun && _playerActor->isWalking()) {
			_playerActor->increaseFPS();
		}
	}
}

void BladeRunnerEngine::handleMouseClickItem(int itemId, bool buttonDown) {
	if (_isWalkingInterruptible && itemId != _walkingToItemId) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToItemId = itemId;
		return;
	}

	if (_mouse->isInactive()) {
		return;
	}

	if (!_combat->isActive()) {
		if (buttonDown) {
			return;
		}

		if (_isInsideScriptItem && itemId == _walkingToItemId) {
			_playerActor->run();
			if (_mouseClickTimeDiff <= 10000) {
				_playerActor->increaseFPS();
			}
		} else {
			_walkingToExitId = -1;
			_walkingToRegionId = -1;
			_walkingToObjectId = -1;
			_walkingToItemId = itemId;
			_walkingToEmpty = false;
			_walkingToActorId = -1;

			_isInsideScriptItem = true;
			_sceneScript->clickedOnItem(itemId, false);
			_isInsideScriptItem = false;
		}
	} else {
		if (!buttonDown || !_items->isTarget(itemId) /* || _mouse->isRandomized() */) {
			return;
		}

		_playerActor->stopWalking(false);
		_playerActor->faceItem(itemId, false);
		_playerActor->changeAnimationMode(kAnimationModeCombatAttack, false);
		_settings->decreaseAmmo();
		_audioPlayer->playAud(_gameInfo->getSfxTrack(_combat->getHitSound()), 100, 0, 0, 90, 0);

		_mouse->setMouseJitterUp();

		_isInsideScriptItem = true;
		_sceneScript->clickedOnItem(itemId, true);
		_isInsideScriptItem = false;
	}
}

void BladeRunnerEngine::handleMouseClickActor(int actorId, bool mainButton, bool buttonDown, Vector3 &scenePosition, int x, int y) {
	if (_isWalkingInterruptible && actorId != _walkingToActorId) {
		_isWalkingInterruptible = false;
		_interruptWalking = true;
		walkingReset();
		_walkingToActorId = actorId;
		return;
	}

	if (_mouse->isInactive()) {
		return;
	}

	if (!buttonDown) {
		if (actorId == kActorMcCoy) {
			if (mainButton) {
				if (!_combat->isActive()) {
					_kia->openLastOpened();
				}
			} else if (!_playerActor->mustReachWalkDestination()) {
				_combat->change();
			}
			return;
		}

		if (_isInsideScriptActor && actorId == _walkingToActorId) {
			_playerActor->run();
			if (_mouseClickTimeDiff <= 10000) {
				_playerActor->increaseFPS();
			}
		} else {
			_walkingToExitId = -1;
			_walkingToRegionId = -1;
			_walkingToObjectId = -1;
			_walkingToItemId = -1;
			_walkingToEmpty = false;
			_walkingToActorId = actorId;

			_isInsideScriptActor = true;
			bool processedBySceneScript = _sceneScript->clickedOnActor(actorId);
			_isInsideScriptActor = false;

			if (!_combat->isActive() && !processedBySceneScript) {
				_aiScripts->clickedByPlayer(actorId);
			}
		}
	} else {
		Actor *actor = _actors[actorId];

		if (!_combat->isActive() || actorId == kActorMcCoy || !actor->isTarget() || actor->isRetired() /*|| _mouse->isRandomized()*/) {
			return;
		}
		_playerActor->stopWalking(false);
		_playerActor->faceActor(actorId, false);
		_playerActor->changeAnimationMode(kAnimationModeCombatAttack, false);
		_settings->decreaseAmmo();

		bool missed = _playerActor->isObstacleBetween(actor->getXYZ());

		_audioPlayer->playAud(_gameInfo->getSfxTrack(missed ? _combat->getMissSound() : _combat->getHitSound()), 100, 0, 0, 90, 0);

		_mouse->setMouseJitterUp();

		if (missed) {
			_aiScripts->shotAtAndMissed(actorId);
		} else {
			_isInsideScriptActor = true;
			bool canShoot = _aiScripts->shotAtAndHit(actorId);
			_isInsideScriptActor = false;
			if (!canShoot) {
				_combat->shoot(actorId, scenePosition, x);
			}
		}
	}
}

void BladeRunnerEngine::gameWaitForActive() {
	while (!_windowIsActive) {
		handleEvents();
	}
}

void BladeRunnerEngine::loopActorSpeaking() {
	if (!_audioSpeech->isPlaying()) {
		return;
	}

	playerLosesControl();

	do {
		gameTick();
	} while (_gameIsRunning && _audioSpeech->isPlaying());

	playerGainsControl();
}

void BladeRunnerEngine::outtakePlay(int id, bool noLocalization, int container) {
	Common::String name = _gameInfo->getOuttake(id);

	OuttakePlayer player(this);

	player.play(name, noLocalization, container);
}

bool BladeRunnerEngine::openArchive(const Common::String &name) {
	int i;

	// If archive is already open, return true
	for (i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name) {
			return true;
		}
	}

	// Find first available slot
	for (i = 0; i != kArchiveCount; ++i) {
		if (!_archives[i].isOpen()) {
			break;
		}
	}
	if (i == kArchiveCount) {
		/* TODO: BLADE.EXE retires the least recently used
		 * archive when it runs out of slots. */

		error("openArchive: No more archive slots");
	}

	_archives[i].open(name);
	return _archives[i].isOpen();
}

bool BladeRunnerEngine::closeArchive(const Common::String &name) {
	for (int i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name) {
			_archives[i].close();
			return true;
		}
	}

	warning("closeArchive: Archive %s not open.", name.c_str());
	return false;
}

bool BladeRunnerEngine::isArchiveOpen(const Common::String &name) const {
	for (int i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name)
			return true;
	}

	return false;
}

void BladeRunnerEngine::syncSoundSettings() {
	Engine::syncSoundSettings();

	_subtitlesEnabled = ConfMan.getBool("subtitles");
}

bool BladeRunnerEngine::isSubtitlesEnabled() {
	return _subtitlesEnabled;
}

void BladeRunnerEngine::setSubtitlesEnabled(bool newVal) {
	ConfMan.setBool("subtitles", newVal);
	syncSoundSettings();
}

Common::SeekableReadStream *BladeRunnerEngine::getResourceStream(const Common::String &name) {
	for (int i = 0; i != kArchiveCount; ++i) {
		if (!_archives[i].isOpen()) {
			continue;
		}

		// debug("getResource: Searching archive %s for %s.", _archives[i].getName().c_str(), name.c_str());

		Common::SeekableReadStream *stream = _archives[i].createReadStreamForMember(name);
		if (stream) {
			return stream;
		}
	}

	warning("getResource: Resource %s not found", name.c_str());
	return nullptr;
}

bool BladeRunnerEngine::playerHasControl() {
	return _playerLosesControlCounter == 0;
}

void BladeRunnerEngine::playerLosesControl() {
	if (++_playerLosesControlCounter == 1) {
		_mouse->disable();
	}
}

void BladeRunnerEngine::playerGainsControl() {
	if (_playerLosesControlCounter == 0) {
		warning("Unbalanced call to BladeRunnerEngine::playerGainsControl");
	}

	if (_playerLosesControlCounter > 0)
		--_playerLosesControlCounter;

	if (_playerLosesControlCounter == 0) {
		_mouse->enable();
	}
}

void BladeRunnerEngine::playerDied() {
	playerLosesControl();

	int timeWaitEnd = _time->current() + 5000;
	while (_time->current() < timeWaitEnd) {
		gameTick();
	}

	_actorDialogueQueue->flush(1, false);

	while (_playerLosesControlCounter > 0) {
		playerGainsControl();
	}

	_kia->_forceOpen = true;
	_kia->open(kKIASectionLoad);
}

bool BladeRunnerEngine::saveGame(Common::WriteStream &stream, const Graphics::Surface &thumbnail) {
	if (!playerHasControl() || _sceneScript->isInsideScript() || _aiScripts->isInsideScript()) {
		return false;
	}

	Common::MemoryWriteStreamDynamic memoryStream(DisposeAfterUse::YES);
	SaveFileWriteStream s(memoryStream);

	s.write(thumbnail.getPixels(), SaveFileManager::kThumbnailSize);
	s.writeFloat(1.0f);
	_settings->save(s);
	_scene->save(s);
	_scene->_exits->save(s);
	_scene->_regions->save(s);
	_scene->_set->save(s);
	for (uint i = 0; i != _gameInfo->getGlobalVarCount(); ++i) {
		s.writeInt(_gameVars[i]);
	}
	_music->save(s);
	// _audioPlayer->save(s) // zero func
	// _audioSpeech->save(s) // zero func
	_combat->save(s);
	_gameFlags->save(s);
	_items->save(s);
	_sceneObjects->save(s);
	_ambientSounds->save(s);
	_overlays->save(s);
	_spinner->save(s);
	_scores->save(s);
	_dialogueMenu->save(s);
	_obstacles->save(s);
	_actorDialogueQueue->save(s);
	_waypoints->save(s);

	for (uint i = 0; i != _gameInfo->getActorCount(); ++i) {
		_actors[i]->save(s);

		int animationState, animationFrame, animationStateNext, nextAnimation;
		_aiScripts->queryAnimationState(i, &animationState, &animationFrame, &animationStateNext, &nextAnimation);
		s.writeInt(animationState);
		s.writeInt(animationFrame);
		s.writeInt(animationStateNext);
		s.writeInt(nextAnimation);
	}
	_actors[kActorVoiceOver]->save(s);
	_policeMaze->save(s);
	_crimesDatabase->save(s);

	s.finalize();

	stream.writeUint32LE(memoryStream.size() + 4);
	stream.write(memoryStream.getData(), memoryStream.size());
	stream.flush();

	return true;
}

bool BladeRunnerEngine::loadGame(Common::SeekableReadStream &stream) {
	if (!playerHasControl() || _sceneScript->isInsideScript() || _aiScripts->isInsideScript()) {
		return false;
	}

	SaveFileReadStream s(stream);

	_ambientSounds->removeAllNonLoopingSounds(true);
	_ambientSounds->removeAllLoopingSounds(1);
	_music->stop(2);
	_audioSpeech->stopSpeech();
	_actorDialogueQueue->flush(true, false);
	_screenEffects->_entries.clear();

	int size = s.readInt();

	if (size != s.size() - s.pos() + 4) {
		_gameIsLoading = false;
		return false;
	}

	_gameIsLoading = true;
	_settings->setLoadingGame();
	s.skip(SaveFileManager::kThumbnailSize); // skip the thumbnail
	s.skip(4);// always float 1.0, but never used
	_settings->load(s);
	_scene->load(s);
	_scene->_exits->load(s);
	_scene->_regions->load(s);
	_scene->_set->load(s);
	for (uint i = 0; i != _gameInfo->getGlobalVarCount(); ++i) {
		_gameVars[i] = s.readInt();
	}
	_music->load(s);
	// _audioPlayer->load(s) // zero func
	// _audioSpeech->load(s) // zero func
	_combat->load(s);
	_gameFlags->load(s);
	_items->load(s);
	_sceneObjects->load(s);
	_ambientSounds->load(s);
	_overlays->load(s);
	_spinner->load(s);
	_scores->load(s);
	_dialogueMenu->load(s);
	_obstacles->load(s);
	_actorDialogueQueue->load(s);
	_waypoints->load(s);
	for (uint i = 0; i != _gameInfo->getActorCount(); ++i) {
		_actors[i]->load(s);

		int animationState = s.readInt();
		int animationFrame = s.readInt();
		int animationStateNext = s.readInt();
		int nextAnimation = s.readInt();
		_aiScripts->setAnimationState(i, animationState, animationFrame, animationStateNext, nextAnimation);
	}
	_actors[kActorVoiceOver]->load(s);
	_policeMaze->load(s);
	_crimesDatabase->load(s);

	_gameIsLoading = false;

	_settings->setNewSetAndScene(_settings->getSet(), _settings->getScene());
	_settings->setChapter(_settings->getChapter());

	return true;
}

void BladeRunnerEngine::newGame(int difficulty) {
	_settings->reset();
	_combat->reset();

	for (uint i = 0; i < _gameInfo->getActorCount(); ++i) {
		_actors[i]->setup(i);
	}
	_actors[kActorVoiceOver]->setup(99);

	for (uint i = 0; i < _gameInfo->getSuspectCount(); ++i) {
		_suspectsDatabase->get(i)->reset();
	}

	_gameFlags->clear();

	for (uint i = 0; i < _gameInfo->getGlobalVarCount(); ++i) {
		_gameVars[i] = 0;
	}

	_items->reset();
	_scores->reset();
	_kia->reset();
	_dialogueMenu->clear();
	_scene->_exits->enable();

	if (difficulty >= 0 && difficulty < 3) {
		_settings->setDifficulty(difficulty);
	}

	InitScript initScript(this);
	initScript.SCRIPT_Initialize_Game();
	initChapterAndScene();

	_settings->setStartingGame();
}

void BladeRunnerEngine::autoSaveGame(int textId, bool endgame) {
	TextResource textAutoSave(this);
	if (!textAutoSave.open("AUTOSAVE")) {
		return;
	}

	SaveStateList saveList = BladeRunner::SaveFileManager::list(getTargetName());

	// Find first available save slot
	int slot = -1;
	int maxSlot = -1;
	for (int i = 0; i < (int)saveList.size(); ++i) {
		maxSlot = MAX(maxSlot, saveList[i].getSaveSlot());
		if (saveList[i].getSaveSlot() != i) {
			slot = i;
			break;
		}
	}

	if (slot == -1) {
		slot = maxSlot + 1;
	}
	if (endgame) {
		saveGameState(slot, "END_GAME_STATE");
	} else {
		saveGameState(slot,  textAutoSave.getText(textId));
	}

}

void BladeRunnerEngine::ISez(const Common::String &str) {
	debug("\t%s", str.c_str());
}

void BladeRunnerEngine::blitToScreen(const Graphics::Surface &src) const {
	_system->copyRectToScreen(src.getPixels(), src.pitch, 0, 0, src.w, src.h);
	_system->updateScreen();
}

Graphics::Surface BladeRunnerEngine::generateThumbnail() const {
	Graphics::Surface thumbnail;
	thumbnail.create(640 / 8, 480 / 8, createRGB555());

	for (int y = 0; y < thumbnail.h; ++y) {
		for (int x = 0; x < thumbnail.w; ++x) {
			uint16       *dstPixel = (uint16 *)thumbnail.getBasePtr(x, y);
			const uint16 *srcPixel = (const uint16 *)_surfaceFront.getBasePtr(x * 8, y * 8);

			*dstPixel = *srcPixel;
		}
	}

	return thumbnail;
}

GUI::Debugger *BladeRunnerEngine::getDebugger() {
	return _debugger;
}

Common::String BladeRunnerEngine::getTargetName() const {
	return _targetName;
}

void blit(const Graphics::Surface &src, Graphics::Surface &dst) {
	dst.copyRectToSurface(src.getPixels(), src.pitch, 0, 0, src.w, src.h);
}

} // End of namespace BladeRunner
