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
#include "bladerunner/audio_mixer.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/chapters.h"
#include "bladerunner/combat.h"
#include "bladerunner/crimes_database.h"
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
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/screen_effects.h"
#include "bladerunner/set.h"
#include "bladerunner/script/ai_script.h"
#include "bladerunner/script/init_script.h"
#include "bladerunner/script/kia_script.h"
#include "bladerunner/script/scene_script.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/suspects_database.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/ui/elevator.h"
#include "bladerunner/ui/esper.h"
#include "bladerunner/ui/kia.h"
#include "bladerunner/ui/spinner.h"
#include "bladerunner/ui/vk.h"
#include "bladerunner/vqa_decoder.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "graphics/pixelformat.h"

namespace BladeRunner {

BladeRunnerEngine::BladeRunnerEngine(OSystem *syst, const ADGameDescription *desc)
	: Engine(syst),
	  _rnd("bladerunner") {

	_windowIsActive = true;
	_gameIsRunning = true;
	_playerLosesControlCounter = 0;

	_playerActorIdle = false;
	_playerDead = false;
	_speechSkipped = false;
	_gameOver = false;
	_gameAutoSave = 0;
	_gameIsLoading = false;
	_sceneIsLoading = false;

	_walkingActorId = -1;

	_walkSoundId = -1;
	_walkSoundVolume = 0;
	_walkSoundBalance = 0;

	_crimesDatabase = nullptr;

	switch (desc->language) {
	case Common::EN_ANY:
		this->_languageCode = "E";
		break;
	case Common::DE_DEU:
		this->_languageCode = "G";
		break;
	case Common::FR_FRA:
		this->_languageCode = "F";
		break;
	case Common::IT_ITA:
		this->_languageCode = "I";
		break;
	case Common::RU_RUS:
		this->_languageCode = "R";
		break;
	case Common::ES_ESP:
		this->_languageCode = "S";
		break;
	default:
		this->_languageCode = "E";
	}

	_screenEffects           = nullptr;
	_combat                  = nullptr;
	_actorDialogueQueue      = nullptr;
	_settings                = nullptr;
	_itemPickup              = nullptr;
	_lights                  = nullptr;
	_obstacles               = nullptr;
	_sceneScript             = nullptr;
	_gameInfo                = nullptr;
	_waypoints               = nullptr;
	_gameVars                = nullptr;
	_view                    = nullptr;
	_sceneObjects            = nullptr;
	_gameFlags               = nullptr;
	_items                   = nullptr;
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
	_spinner                 = nullptr;
	_elevator                = nullptr;
	_mainFont                = nullptr;
	_mouse                   = nullptr;
	_sliceAnimations         = nullptr;
	_sliceRenderer           = nullptr;
	_crimesDatabase          = nullptr;
	_scene                   = nullptr;
	_aiScripts               = nullptr;
	for (int i = 0; i != kActorCount; ++i) {
		_actors[i] = nullptr;
	}
}

BladeRunnerEngine::~BladeRunnerEngine() {
}

bool BladeRunnerEngine::hasFeature(EngineFeature f) const {
	return f == kSupportsRTL;
}

Common::Error BladeRunnerEngine::run() {
	Graphics::PixelFormat format = createRGB555();
	initGraphics(640, 480, &format);

	_system->showMouse(true);

	if (!startup()) {
		shutdown();
		return Common::Error(Common::kUnknownError, "Failed to initialize resources");
	}

#if BLADERUNNER_DEBUG_GAME
	{
#else
	if (warnUserAboutUnsupportedGame()) {
#endif
		init2();

		/* TODO: Check for save games and enter KIA */
		gameLoop();
	}

	shutdown();

	return Common::kNoError;
}

bool BladeRunnerEngine::startup(bool hasSavegames) {
	// These are static objects in original game

	_screenEffects = new ScreenEffects(this, 0x8000);

	_combat = new Combat(this);

	// TODO: end credits

	_actorDialogueQueue = new ActorDialogueQueue(this);

	_settings = new Settings(this);

	_itemPickup = new ItemPickup(this);

	_lights = new Lights(this);

	// TODO: outtake player - but this is done bit differently

	// TODO: police maze

	_obstacles = new Obstacles(this);

	// TODO: slice renderer shadow

	// TODO: voight-kampf script

	_sceneScript = new SceneScript(this);

	// This is the original startup in the game

	bool r;

	_surfaceFront.create(640, 480, createRGB555());
	_surfaceBack.create(640, 480, createRGB555());
	_surface4.create(640, 480, createRGB555());

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

	// TODO: Create datetime - not used

	// TODO: Create graphics surfaces 1-4

	// TODO: Allocate audio cache

	if (hasSavegames) {
		if (!loadSplash()) {
			return false;
		}
	}

	_waypoints = new Waypoints(this, _gameInfo->getWaypointCount());

	// TODO: Cover waypoints

	// TODO: Flee waypoints

	_gameVars = new int[_gameInfo->getGlobalVarCount()];

	// TODO: Actor AI DLL init

	// Seed rand

	// TODO: Sine and cosine lookup tables for intervals of 1.0, 4.0, and 12.0

	_view = new View();

	_sceneObjects = new SceneObjects(this, _view);

	_gameFlags = new GameFlags();
	_gameFlags->setFlagCount(_gameInfo->getFlagCount());

	_items = new Items(this);

	_audioMixer = new AudioMixer(this);

	_audioPlayer = new AudioPlayer(this);

	_music = new Music(this);

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

	_overlays = new Overlays(this);
	_overlays->init();

	_zbuffer = new ZBuffer();
	_zbuffer->init(640, 480);

	int actorCount = (int)_gameInfo->getActorCount();
	assert(actorCount < kActorCount);
	for (int i = 0; i != actorCount; ++i) {
		_actors[i] = new Actor(this, i);
		_actors[i]->setup(i);
	}
	_actors[kActorVoiceOver] = new Actor(this, kActorVoiceOver);
	_playerActor = _actors[_gameInfo->getPlayerId()];

	_playerActor->setFPS(15);

	// TODO: set _playerActor countdown timer 6

	// TODO: Set actor ids (redundant?)

	// TODO: Police Maze

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

	// TODO: Scores

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
	// _mouse->setCursorPosition(320, 240);
	_mouse->setCursor(0);

	_sliceAnimations = new SliceAnimations(this);
	r = _sliceAnimations->open("INDEX.DAT");
	if (!r)
		return false;

	// TODO: Support cdframes

	r = _sliceAnimations->openHDFrames();
	if (!r) {
		return false;
	}

	r = _sliceAnimations->openCoreAnim();
	if (!r) {
		return false;
	}

	_sliceRenderer = new SliceRenderer(this);
	_sliceRenderer->setScreenEffects(_screenEffects);

	_crimesDatabase = new CrimesDatabase(this, "CLUES", _gameInfo->getClueCount());

	// TODO: Scene
	_scene = new Scene(this);

	// Load INIT.DLL
	InitScript initScript(this);
	initScript.SCRIPT_Initialize_Game();

	// TODO: Load AI-ACT1.DLL
	_aiScripts = new AIScripts(this, actorCount);

	initChapterAndScene();

	return true;
}

void BladeRunnerEngine::initChapterAndScene() {
	// TODO: Init actors...
	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_aiScripts->initialize(i);
	}

	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_actors[i]->changeAnimationMode(kAnimationModeIdle);
	}

	for (int i = 1, end = _gameInfo->getActorCount(); i != end; ++i) { // skip first actor, probably player
		_actors[i]->movementTrackNext(true);
	}

	_settings->setChapter(1);
	_settings->setNewSetAndScene(_gameInfo->getInitialSetId(), _gameInfo->getInitialSceneId());
//	_settings->setNewSetAndScene(52, 52);
}

void BladeRunnerEngine::shutdown() {
	_mixer->stopAll();

	// TODO: Write BLADE.INI

	delete _vk;
	_vk = nullptr;

	delete _esper;
	_esper = nullptr;

	delete _mouse;
	_mouse = nullptr;

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
		_chapters = nullptr;
	}

	delete _crimesDatabase;
	_crimesDatabase = nullptr;

	delete _sliceRenderer;
	_sliceRenderer = nullptr;

	delete _sliceAnimations;
	_sliceAnimations = nullptr;

	delete _textActorNames;
	_textActorNames = nullptr;

	delete _textCrimes;
	_textCrimes = nullptr;

	delete _textClueTypes;
	_textClueTypes = nullptr;

	delete _textKIA;
	_textKIA = nullptr;

	delete _textSpinnerDestinations;
	_textSpinnerDestinations = nullptr;

	delete _textVK;
	_textVK = nullptr;

	delete _textOptions;
	_textOptions = nullptr;

	delete _dialogueMenu;
	_dialogueMenu = nullptr;

	delete _ambientSounds;
	_ambientSounds = nullptr;

	delete _overlays;
	_overlays = nullptr;

	delete _audioSpeech;
	_audioSpeech = nullptr;

	delete _music;
	_music = nullptr;

	delete _audioPlayer;
	_audioPlayer = nullptr;

	delete _audioMixer;
	_audioMixer = nullptr;

	if (isArchiveOpen("MUSIC.MIX")) {
		closeArchive("MUSIC.MIX");
	}

	if (isArchiveOpen("SFX.MIX")) {
		closeArchive("SFX.MIX");
	}

	if (isArchiveOpen("SPCHSFX.TLK")) {
		closeArchive("SPCHSFX.TLK");
	}

	if (_mainFont) {
		_mainFont->close();
		delete _mainFont;
		_mainFont = nullptr;
	}

	delete _items;
	_items = nullptr;

	delete _gameFlags;
	_gameFlags = nullptr;

	delete _view;
	_view = nullptr;

	delete _sceneObjects;
	_sceneObjects = nullptr;

	// TODO: Delete sine and cosine lookup tables

	delete _aiScripts;
	_aiScripts = nullptr;

	delete[] _gameVars;
	_gameVars = nullptr;

	delete _waypoints;
	_waypoints = nullptr;

	// TODO: Delete Cover waypoints

	// TODO: Delete Flee waypoints

	// TODO: Delete Scores

	delete _elevator;
	_elevator = nullptr;

	delete _spinner;
	_spinner = nullptr;

	delete _kia;
	_kia = nullptr;

	delete _suspectsDatabase;
	_suspectsDatabase = nullptr;

	// TODO: Delete datetime - not used

	int actorCount = (int)_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		delete _actors[i];
		_actors[i] = nullptr;
	}
	delete _actors[kActorVoiceOver];
	_actors[kActorVoiceOver] = nullptr;

	_playerActor = nullptr;

	delete _zbuffer;
	_zbuffer = nullptr;

	delete _gameInfo;
	_gameInfo = nullptr;

	// TODO: Delete graphics surfaces here
	_surface4.free();
	_surfaceBack.free();
	_surfaceFront.free();

	if (isArchiveOpen("STARTUP.MIX")) {
		closeArchive("STARTUP.MIX");
	}

	// TODO: Delete MIXArchives here

	// TODO: Delete Timer



	// These are static objects in original game

	delete _zbuffer;
	_zbuffer = nullptr;

	delete _itemPickup;
	_itemPickup = nullptr;

	delete _obstacles;
	_obstacles = nullptr;

	delete _actorDialogueQueue;
	_actorDialogueQueue = nullptr;

	delete _combat;
	_combat = nullptr;

	delete _screenEffects;
	_screenEffects = nullptr;

	delete _lights;
	_lights = nullptr;

	delete _settings;
	_settings = nullptr;

	delete _sceneScript;
	_sceneScript = nullptr;
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

bool BladeRunnerEngine::init2() {
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
		/* TODO: check player death */
		gameTick();
	} while (_gameIsRunning);
}

#if BLADERUNNER_DEBUG_RENDERING

void drawBBox(Vector3 start, Vector3 end, View *view, Graphics::Surface *surface, int color) {
	Vector3 bfl = view->calculateScreenPosition(Vector3(start.x, start.y, start.z));
	Vector3 bfr = view->calculateScreenPosition(Vector3(start.x, end.y, start.z));
	Vector3 bbr = view->calculateScreenPosition(Vector3(end.x, end.y, start.z));
	Vector3 bbl = view->calculateScreenPosition(Vector3(end.x, start.y, start.z));

	Vector3 tfl = view->calculateScreenPosition(Vector3(start.x, start.y, end.z));
	Vector3 tfr = view->calculateScreenPosition(Vector3(start.x, end.y, end.z));
	Vector3 tbr = view->calculateScreenPosition(Vector3(end.x, end.y, end.z));
	Vector3 tbl = view->calculateScreenPosition(Vector3(end.x, start.y, end.z));

	surface->drawLine(bfl.x, bfl.y, bfr.x, bfr.y, color);
	surface->drawLine(bfr.x, bfr.y, bbr.x, bbr.y, color);
	surface->drawLine(bbr.x, bbr.y, bbl.x, bbl.y, color);
	surface->drawLine(bbl.x, bbl.y, bfl.x, bfl.y, color);

	surface->drawLine(tfl.x, tfl.y, tfr.x, tfr.y, color);
	surface->drawLine(tfr.x, tfr.y, tbr.x, tbr.y, color);
	surface->drawLine(tbr.x, tbr.y, tbl.x, tbl.y, color);
	surface->drawLine(tbl.x, tbl.y, tfl.x, tfl.y, color);

	surface->drawLine(bfl.x, bfl.y, tfl.x, tfl.y, color);
	surface->drawLine(bfr.x, bfr.y, tfr.x, tfr.y, color);
	surface->drawLine(bbr.x, bbr.y, tbr.x, tbr.y, color);
	surface->drawLine(bbl.x, bbl.y, tbl.x, tbl.y, color);
}
#endif

void BladeRunnerEngine::gameTick() {
	handleEvents();

	if (_gameIsRunning && _windowIsActive) {
		// TODO: Only run if not in Kia, script, nor AI
		if (!_sceneScript->isInsideScript() && !_aiScripts->isInsideScript()) {
			_settings->openNewScene();
		}

		// TODO: Autosave

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

		// TODO: Scores

		_actorDialogueQueue->tick();
		if (_scene->didPlayerWalkIn()) {
			_sceneScript->playerWalkedIn();
		}
		bool inDialogueMenu = _dialogueMenu->isVisible();
		if (!inDialogueMenu) {
			// TODO: actors combat-tick
		}

		// TODO: Gun range announcements
		_zbuffer->clean();

		_ambientSounds->tick();

		bool backgroundChanged = false;
		int frame = _scene->advanceFrame();
		if (frame >= 0) {
			_sceneScript->sceneFrameAdvanced(frame);
			backgroundChanged = true;
		}
		(void)backgroundChanged;
		blit(_surfaceBack, _surfaceFront);

		// TODO: remove zbuffer draw
		// _surfaceFront.copyRectToSurface(_zbuffer->getData(), 1280, 0, 0, 640, 480);

		_overlays->tick();

		if (!inDialogueMenu) {
			actorsUpdate();
		}

		if (_settings->getNewScene() == -1 || _sceneScript->isInsideScript() || _aiScripts->isInsideScript()) {
			_sliceRenderer->setView(*_view);

			// Tick and draw all actors in current set
			int setId = _scene->getSetId();
			for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
				if (_actors[i]->getSetId() == setId) {
					// TODO: remove this limitation
					if (i == kActorMcCoy
						|| i == kActorRunciter
						|| i == kActorOfficerLeary
						|| i == kActorMaggie) {
						Common::Rect screenRect;
						if (_actors[i]->tick(backgroundChanged, &screenRect)) {
							_zbuffer->mark(screenRect);
						}
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

			_mouse->tick(p.x, p.y);
			_mouse->draw(_surfaceFront, p.x, p.y);

			// TODO: Process AUD

			if (_walkSoundId >= 0) {
				const char *name = _gameInfo->getSfxTrack(_walkSoundId);
				_audioPlayer->playAud(name, _walkSoundVolume, _walkSoundBalance, _walkSoundBalance, 50, 0);
				_walkSoundId = -1;
			}

#if BLADERUNNER_DEBUG_RENDERING
			//draw scene objects
			int count = _sceneObjects->_count;
			if (count > 0) {
				for (int i = 0; i < count; i++) {
					SceneObjects::SceneObject *sceneObject = &_sceneObjects->_sceneObjects[_sceneObjects->_sceneObjectsSortedByDistance[i]];

					BoundingBox *bbox = &sceneObject->boundingBox;
					Vector3 a, b;
					bbox->getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
					Vector3 pos = _view->calculateScreenPosition(0.5 * (a + b));
					int color;

					switch (sceneObject->sceneObjectType) {
					case kSceneObjectTypeActor:
						color = 0b111110000000000;
						drawBBox(a, b, _view, &_surfaceFront, color);
						_mainFont->drawColor(_textActorNames->getText(sceneObject->sceneObjectId - kSceneObjectOffsetActors), _surfaceFront, pos.x, pos.y, color);
						break;
					case kSceneObjectTypeItem:
						char itemText[40];
						drawBBox(a, b, _view, &_surfaceFront, color);
						sprintf(itemText, "item %i", sceneObject->sceneObjectId - kSceneObjectOffsetItems);
						_mainFont->drawColor(itemText, _surfaceFront, pos.x, pos.y, color);
						break;
					case kSceneObjectTypeObject:
						color = 0b011110111101111;
						//if (sceneObject->_isObstacle)
						//	color += 0b100000000000000;
						if (sceneObject->isClickable) {
							color = 0b000001111100000;
						}
						drawBBox(a, b, _view, &_surfaceFront, color);
						_mainFont->drawColor(_scene->objectGetName(sceneObject->sceneObjectId - kSceneObjectOffsetObjects), _surfaceFront, pos.x, pos.y, color);
						break;
					}
					_surfaceFront.frameRect(sceneObject->screenRectangle, color);
				}
			}

			//draw regions
			for (int i = 0; i < 10; i++) {
				Regions::Region *region = &_scene->_regions->_regions[i];
				if (!region->present) continue;
				_surfaceFront.frameRect(region->rectangle, 0b000000000011111);
			}

			for (int i = 0; i < 10; i++) {
				Regions::Region *region = &_scene->_exits->_regions[i];
				if (!region->present) continue;
				_surfaceFront.frameRect(region->rectangle, 0b111111111111111);
			}

			//draw walkboxes
			for (int i = 0; i < _scene->_set->_walkboxCount; i++) {
				Set::Walkbox *walkbox = &_scene->_set->_walkboxes[i];

				for (int j = 0; j < walkbox->vertexCount; j++) {
					Vector3 start = _view->calculateScreenPosition(walkbox->vertices[j]);
					Vector3 end = _view->calculateScreenPosition(walkbox->vertices[(j + 1) % walkbox->vertexCount]);
					_surfaceFront.drawLine(start.x, start.y, end.x, end.y, 0b111111111100000);
					Vector3 pos = _view->calculateScreenPosition(0.5 * (start + end));
					_mainFont->drawColor(walkbox->name, _surfaceFront, pos.x, pos.y, 0b111111111100000);
				}
			}

			// draw lights
			for (int i = 0; i < (int)_lights->_lights.size(); i++) {
				Light *light = _lights->_lights[i];
				Matrix4x3 m = light->_matrix;
				m = invertMatrix(m);
				//todo do this properly
				Vector3 posOrigin = m * Vector3(0.0f, 0.0f, 0.0f);
				float t = posOrigin.y;
				posOrigin.y = posOrigin.z;
				posOrigin.z = -t;

				Vector3 posTarget = m * Vector3(0.0f, 0.0f, -100.0f);
				t = posTarget.y;
				posTarget.y = posTarget.z;
				posTarget.z = -t;

				Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
				int colorR = (light->_color.r * 31.0f);
				int colorG = (light->_color.g * 31.0f);
				int colorB = (light->_color.b * 31.0f);
				int color = (colorR << 10) + (colorG << 5) + colorB;

				drawBBox(posOrigin - size, posOrigin + size, _view, &_surfaceFront, color);

				Vector3 posOriginT = _view->calculateScreenPosition(posOrigin);
				Vector3 posTargetT = _view->calculateScreenPosition(posTarget);
				_surfaceFront.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);
				_mainFont->drawColor(light->_name, _surfaceFront, posOriginT.x, posOriginT.y, color);
			}

			//draw waypoints
			for(int i = 0; i < _waypoints->_count; i++) {
				Waypoints::Waypoint *waypoint = &_waypoints->_waypoints[i];
				if(waypoint->setId != _scene->getSetId())
					continue;
				Vector3 pos = waypoint->position;
				Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
				int color = 0b111111111111111;
				drawBBox(pos - size, pos + size, _view, &_surfaceFront, color);
				Vector3 spos = _view->calculateScreenPosition(pos);
				char waypointText[40];
				sprintf(waypointText, "waypoint %i", i);
				_mainFont->drawColor(waypointText, _surfaceFront, spos.x, spos.y, color);
			}
#endif
#if BLADERUNNER_DEBUG_RENDERING
			//draw aesc
			for (uint i = 0; i < _screenEffects->_entries.size(); i++) {
				ScreenEffects::Entry &entry = _screenEffects->_entries[i];
				int j = 0;
				for (int y = 0; y < entry.height; y++) {
					for (int x = 0; x < entry.width; x++) {
						Common::Rect r((entry.x + x) * 2, (entry.y + y) * 2, (entry.x + x) * 2 + 2, (entry.y + y) * 2 + 2);

						int ec = entry.data[j++];
						Color256 color = entry.palette[ec];
						int bladeToScummVmConstant = 256 / 16;

						Graphics::PixelFormat _pixelFormat = createRGB555();
						int color555 = _pixelFormat.RGBToColor(
							CLIP(color.r * bladeToScummVmConstant, 0, 255),
							CLIP(color.g * bladeToScummVmConstant, 0, 255),
							CLIP(color.b * bladeToScummVmConstant, 0, 255));
						_surfaceFront.fillRect(r, color555);
					}
				}
			}
#endif

			blitToScreen(_surfaceFront);
			_system->delayMillis(10);
		}
	}
}

void BladeRunnerEngine::actorsUpdate() {
	int actorCount = (int)_gameInfo->getActorCount();
	int setId = _scene->getSetId();

	//TODO: original game updates every non-visible characters by updating only one character in one frame
	if (setId != 89 || _gameVars[1] != 4 || _gameFlags->query(670) != 1 || !_aiScripts->isInsideScript()) {
		for (int i = 0; i < actorCount; i++) {
			Actor *actor = _actors[i];
			if (actor->getSetId() == setId) {
				_aiScripts->update(i);
				actor->countdownTimersUpdate();
			}
		}
	}
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
	if (event.kbd.keycode == Common::KEYCODE_RETURN) {
		_speechSkipped = true;
	}

	// TODO:
	if (!playerHasControl() /*|| ActorInWalkingLoop*/) {
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

	//TODO: scores
	switch (event.kbd.keycode) {
		case Common::KEYCODE_TAB:
			_kia->openLastOpened();
			break;
		case Common::KEYCODE_ESCAPE:
			_kia->open(kKIASectionSettings);
			break;
		case Common::KEYCODE_SPACE:
			// TODO: combat::switchCombatMode(&Combat);
			break;
		default:
			break;
	}
}

void BladeRunnerEngine::handleKeyDown(Common::Event &event) {
	//TODO:
	if (!playerHasControl() /* || ActorWalkingLoop || ActorSpeaking || VqaIsPlaying */) {
		return;
	}

	if (_kia->isOpen()) {
		_kia->handleKeyDown(event.kbd);
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

	//TODO: scores

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

void BladeRunnerEngine::handleMouseAction(int x, int y, bool buttonLeft, bool buttonDown) {
	if (!playerHasControl() || _mouse->isDisabled()) {
		return;
	}

	if (_kia->isOpen()) {
		if (buttonDown) {
			_kia->handleMouseDown(x, y, buttonLeft);
		} else {
			_kia->handleMouseUp(x, y, buttonLeft);
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
			_esper->handleMouseDown(x, y, buttonLeft);
		} else {
			_esper->handleMouseUp(x, y, buttonLeft);
		}
		return;
	}

	if (_vk->isOpen()) {
		if (buttonDown) {
			_vk->handleMouseDown(x, y, buttonLeft);
		} else {
			_vk->handleMouseUp(x, y, buttonLeft);
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

	if (_dialogueMenu->waitingForInput()) {
		if (buttonLeft && !buttonDown) {
			_dialogueMenu->mouseUp();
		}
		return;
	}

	if (buttonLeft && !buttonDown) {
		Vector3 scenePosition = _mouse->getXYZ(x, y);

		bool isClickable;
		bool isObstacle;
		bool isTarget;

		int sceneObjectId = _sceneObjects->findByXYZ(&isClickable, &isObstacle, &isTarget, scenePosition.x, scenePosition.y, scenePosition.z, true, false, true);
		int exitIndex = _scene->_exits->getRegionAtXY(x, y);

		if ((sceneObjectId < 0 || sceneObjectId > 73) && exitIndex >= 0) {
			handleMouseClickExit(x, y, exitIndex);
			return;
		}

		int regionIndex = _scene->_regions->getRegionAtXY(x, y);
		if (regionIndex >= 0) {
			handleMouseClickRegion(x, y, regionIndex);
			return;
		}

		if (sceneObjectId == -1) {
			handleMouseClickEmpty(x, y, scenePosition);
			return;
		} else if (sceneObjectId >= 0 && sceneObjectId <= 73) {
			handleMouseClickActor(x, y, sceneObjectId);
			return;
		} else if (sceneObjectId >= 74 && sceneObjectId <= 197) {
			handleMouseClickItem(x, y, sceneObjectId - 74);
			return;
		} else if (sceneObjectId >= 198 && sceneObjectId <= 293) {
			handleMouseClick3DObject(x, y, sceneObjectId - 198, isClickable, isTarget);
			return;
		}
	}
	if (!buttonLeft && buttonDown) {
		// TODO: stop walking && switch combat mode
	}

}

void BladeRunnerEngine::handleMouseClickExit(int x, int y, int exitIndex) {
	debug("clicked on exit %d %d %d", exitIndex, x, y);
	_sceneScript->clickedOnExit(exitIndex);
}

void BladeRunnerEngine::handleMouseClickRegion(int x, int y, int regionIndex) {
	debug("clicked on region %d %d %d", regionIndex, x, y);
	_sceneScript->clickedOn2DRegion(regionIndex);
}

void BladeRunnerEngine::handleMouseClick3DObject(int x, int y, int objectId, bool isClickable, bool isTarget) {
	const char *objectName = _scene->objectGetName(objectId);
	debug("Clicked on object %s", objectName);
	_sceneScript->clickedOn3DObject(objectName, false);
}

void BladeRunnerEngine::handleMouseClickEmpty(int x, int y, Vector3 &mousePosition) {
	bool sceneMouseClick = _sceneScript->mouseClick(x, y);

	if (sceneMouseClick) {
		return;
	}

	bool isRunning;
	debug("Clicked on nothing %f, %f, %f", mousePosition.x, mousePosition.y, mousePosition.z);
	_playerActor->loopWalkToXYZ(mousePosition, 0, false, false, false, &isRunning);
}

void BladeRunnerEngine::handleMouseClickItem(int x, int y, int itemId) {
	debug("Clicked on item %d", itemId);
	_sceneScript->clickedOnItem(itemId, false);
}

void BladeRunnerEngine::handleMouseClickActor(int x, int y, int actorId) {
	debug("Clicked on actor %d", actorId);
	bool t = _sceneScript->clickedOnActor(actorId);
	if (!_combat->isActive() && !t) {
		_aiScripts->clickedByPlayer(actorId);
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
		return false;
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

	debug("closeArchive: Archive %s not open.", name.c_str());
	return false;
}

bool BladeRunnerEngine::isArchiveOpen(const Common::String &name) const {
	for (int i = 0; i != kArchiveCount; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name)
			return true;
	}

	return false;
}

Common::SeekableReadStream *BladeRunnerEngine::getResourceStream(const Common::String &name) {
	for (int i = 0; i != kArchiveCount; ++i) {
		if (!_archives[i].isOpen()) {
			continue;
		}
		if (false) {
			debug("getResource: Searching archive %s for %s.", _archives[i].getName().c_str(), name.c_str());
		}
		Common::SeekableReadStream *stream = _archives[i].createReadStreamForMember(name);
		if (stream) {
			return stream;
		}
	}

	debug("getResource: Resource %s not found.", name.c_str());
	return nullptr;
}

bool BladeRunnerEngine::playerHasControl() {
	return _playerLosesControlCounter == 0;
}

void BladeRunnerEngine::playerLosesControl() {
	if (++_playerLosesControlCounter == 1) {
		_mouse->disable();
	}
	// debug("Player Lost Control (%d)", _playerLosesControlCounter);
}

void BladeRunnerEngine::playerGainsControl() {
	if (_playerLosesControlCounter == 0) {
		warning("Unbalanced call to BladeRunnerEngine::playerGainsControl");
	}

	if (_playerLosesControlCounter > 0)
		--_playerLosesControlCounter;

	// debug("Player Gained Control (%d)", _playerLosesControlCounter);

	if (_playerLosesControlCounter == 0) {
		_mouse->enable();
	}
}

void BladeRunnerEngine::ISez(const char *str) {
	debug("\t%s", str);
}

void BladeRunnerEngine::blitToScreen(const Graphics::Surface &src) {
	_system->copyRectToScreen(src.getPixels(), src.pitch, 0, 0, src.w, src.h);
	_system->updateScreen();
}

void blit(const Graphics::Surface &src, Graphics::Surface &dst) {
	dst.copyRectToSurface(src.getPixels(), src.pitch, 0, 0, src.w, src.h);
}

} // End of namespace BladeRunner
