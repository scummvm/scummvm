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
#include "bladerunner/adq.h"
#include "bladerunner/ambient_sounds.h"
#include "bladerunner/audio_player.h"
#include "bladerunner/audio_speech.h"
#include "bladerunner/chapters.h"
#include "bladerunner/combat.h"
#include "bladerunner/crimes_database.h"
#include "bladerunner/font.h"
#include "bladerunner/gameflags.h"
#include "bladerunner/gameinfo.h"
#include "bladerunner/image.h"
#include "bladerunner/item_pickup.h"
#include "bladerunner/items.h"
#include "bladerunner/lights.h"
#include "bladerunner/mouse.h"
#include "bladerunner/outtake.h"
#include "bladerunner/obstacles.h"
#include "bladerunner/scene.h"
#include "bladerunner/scene_objects.h"
#include "bladerunner/script/init.h"
#include "bladerunner/script/scene.h"
#include "bladerunner/script/ai.h"
#include "bladerunner/settings.h"
#include "bladerunner/shape.h"
#include "bladerunner/slice_animations.h"
#include "bladerunner/slice_renderer.h"
#include "bladerunner/text_resource.h"
#include "bladerunner/vqa_decoder.h"
#include "bladerunner/waypoints.h"
#include "bladerunner/zbuffer.h"

#include "common/array.h"
#include "common/error.h"
#include "common/events.h"
#include "common/system.h"

#include "engines/util.h"

#include "graphics/pixelformat.h"
#include "suspects_database.h"

namespace BladeRunner {

BladeRunnerEngine::BladeRunnerEngine(OSystem *syst)
	: Engine(syst),
	  _rnd("bladerunner") {
	_windowIsActive = true;
	_gameIsRunning = true;
	_playerLosesControlCounter = 0;

	_crimesDatabase = nullptr;
	_sceneScript = new SceneScript(this);
	_settings = new Settings(this);
	_lights = new Lights(this);
	_combat = new Combat(this);
	_adq = new ADQ(this);
	_obstacles = new Obstacles(this);
	_itemPickup = new ItemPickup(this);

	_playerActorIdle = false;
	_playerDead = false;
	_speechSkipped = false;
	_gameOver = false;
	_gameAutoSave = 0;
	_gameIsLoading = false;
	_sceneIsLoading = false;

	_walkSoundId = -1;
	_walkSoundVolume = 0;
	_walkSoundBalance = 0;
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

	delete _zbuffer;

	delete _itemPickup;
	delete _obstacles;
	delete _adq;
	delete _combat;
	delete _lights;
	delete _settings;
	delete _sceneScript;
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

	_zbuffer = new ZBuffer();
	_zbuffer->init(640, 480);

	int actorCount = (int)_gameInfo->getActorCount();
	assert(actorCount < ACTORS_COUNT);
	for (int i = 0; i != actorCount; ++i) {
		_actors[i] = new Actor(this, i);
		_actors[i]->setup(i);
	}
	_actors[VOICEOVER_ACTOR] = new Actor(this, VOICEOVER_ACTOR);
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

	_textCluetype = new TextResource(this);
	if (!_textCluetype->open("CLUETYPE"))
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

	// TODO: Dialogue Menu (DLGMENU.TRE)

	_suspectsDatabase = new SuspectsDatabase(this, _gameInfo->getSuspectsDatabaseSize());

	// TODO: KIA

	// TODO: Spinner Interface

	// TODO: Elevators

	// TODO: Scores

	_mainFont = new Font(this);
	_mainFont->open("KIA6PT.FON", 640, 480, -1, 0, 0x252D);
	_mainFont->setSpacing(1, 0);

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

	_crimesDatabase = new CrimesDatabase(this, "CLUES", _gameInfo->getClueCount());

	// TODO: Scene
	_scene = new Scene(this);

	// Load INIT.DLL
	ScriptInit initScript(this);
	initScript.SCRIPT_Initialize_Game();

	// TODO: Load AI-ACT1.DLL
	_aiScripts = new AIScripts(this, actorCount);

	initChapterAndScene();

	return true;
}

void BladeRunnerEngine::initChapterAndScene() {
	// TODO: Init actors...
	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_aiScripts->Initialize(i);
	}

	for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
		_actors[i]->changeAnimationMode(kAnimationModeIdle);
	}

	for (int i = 1, end = _gameInfo->getActorCount(); i != end; ++i) { // skip first actor, probably player
		_actors[i]->movementTrackNext(true);
	}

	_settings->setChapter(1);
	_settings->setNewSetAndScene(_gameInfo->getInitialSetId(), _gameInfo->getInitialSceneId());
}

void BladeRunnerEngine::shutdown() {
	_mixer->stopAll();

	// TODO: Write BLADE.INI

	// TODO: Shutdown VK

	// TODO: Shutdown Esper

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

	delete _textCluetype;
	_textCluetype = nullptr;

	delete _textKIA;
	_textKIA = nullptr;

	delete _textSpinnerDestinations;
	_textSpinnerDestinations = nullptr;

	delete _textVK;
	_textVK = nullptr;

	delete _textOptions;
	_textOptions = nullptr;

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

	// TODO: Unload AI dll
	delete _aiScripts;
	_aiScripts = nullptr;

	delete[] _gameVars;
	_gameVars = nullptr;

	delete _waypoints;
	_waypoints = nullptr;

	// TODO: Delete Cover waypoints

	// TODO: Delete Flee waypoints

	// TODO: Delete Scores

	// TODO: Delete Elevators

	// TODO: Delete Spinner Interface

	// TODO: Delete KIA

	delete _suspectsDatabase;
	_suspectsDatabase = nullptr;

	// TODO: Delete datetime - not used

	int actorCount = (int)_gameInfo->getActorCount();
	for (int i = 0; i != actorCount; ++i) {
		delete _actors[i];
		_actors[i] = nullptr;
	}
	delete _actors[VOICEOVER_ACTOR];
	_actors[VOICEOVER_ACTOR] = nullptr;

	_playerActor = nullptr;

	delete _zbuffer;
	_zbuffer = nullptr;

	delete _gameInfo;
	_gameInfo = nullptr;

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

	return true;
}

bool BladeRunnerEngine::init2() {
	return true;
}

void BladeRunnerEngine::gameLoop() {
	_gameIsRunning = true;
	do {
		/* TODO: check player death */
		gameTick();
	} while (_gameIsRunning);
}

#if _DEBUG

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
		_settings->openNewScene();

		// TODO: Autosave
		// TODO: Kia
		// TODO: Spinner
		// TODO: Esper
		// TODO: VK
		// TODO: Elevators
		// TODO: Scores

		_adq->tick();
		if (_scene->didPlayerWalkIn()) {
			_sceneScript->PlayerWalkedIn();
		}
		// TODO: Gun range announcements

		_zbuffer->clean();

		_ambientSounds->tick();

		bool backgroundChanged = false;
		int frame = _scene->advanceFrame(_surface1);
		if (frame >= 0) {
			_sceneScript->SceneFrameAdvanced(frame);
			backgroundChanged = true;
		}
		(void)backgroundChanged;
		_surface2.copyFrom(_surface1);
		// TODO: remove zbuffer draw
		//_surface2.copyRectToSurface(_zbuffer->getData(), 1280, 0, 0, 640, 480);

		// TODO: Render overlays

		//if (!dialogueMenu)
			actorsUpdate();

		if (_settings->getNewScene() == -1 || _sceneScript->IsInsideScript() || _aiScripts->IsInsideScript()) {
			_sliceRenderer->setView(*_view);

			// Tick and draw all actors in current set
			int setId = _scene->getSetId();
			for (int i = 0, end = _gameInfo->getActorCount(); i != end; ++i) {
				if (_actors[i]->getSetId() == setId) {
					if (i == 0 || i == 15 || i == 23) { // Currently limited to McCoy, Runciter and Officer Leroy
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

			// TODO: Draw dialogue menu

			Common::Point p = _eventMan->getMousePos();
			_mouse->tick(p.x, p.y);
			_mouse->draw(_surface2, p.x, p.y);

			// TODO: Process AUD
			// TODO: Footstep sound

			if (_walkSoundId >= 0) {
				const char *name = _gameInfo->getSfxTrack(_walkSoundId);
				_audioPlayer->playAud(name, _walkSoundVolume, _walkSoundBalance, _walkSoundBalance, 50, 0);
				_walkSoundId = -1;
			}

#if 0
			//draw scene objects
			int count = _sceneObjects->_count;
			if (count > 0) {
				for (int i = 0; i < count; i++) {
					SceneObject *sceneObject = &_sceneObjects->_sceneObjects[_sceneObjects->_sceneObjectsSortedByDistance[i]];

					BoundingBox *bbox = &sceneObject->_boundingBox;
					Vector3 a, b;
					bbox->getXYZ(&a.x, &a.y, &a.z, &b.x, &b.y, &b.z);
					Vector3 pos = _view->calculateScreenPosition(0.5 * (a + b));
					int color;

					switch (sceneObject->_sceneObjectType) {
					case SceneObjectTypeActor:
						color = 0b111110000000000;
						drawBBox(a, b, _view, &_surface2, color);
						_mainFont->drawColor(_textActorNames->getText(sceneObject->_sceneObjectId - SCENE_OBJECTS_ACTORS_OFFSET), _surface2, pos.x, pos.y, color);
						break;
					case SceneObjectTypeItem:
						char itemText[40];
						drawBBox(a, b, _view, &_surface2, color);
						sprintf(itemText, "item %i", sceneObject->_sceneObjectId - SCENE_OBJECTS_ITEMS_OFFSET);
						_mainFont->drawColor(itemText, _surface2, pos.x, pos.y, color);
						break;
					case SceneObjectTypeObject:
						color = 0b011110111101111;
						//if (sceneObject->_isObstacle)
						//	color += 0b100000000000000;
						if (sceneObject->_isClickable) {
							color = 0b000001111100000;
						}
						drawBBox(a, b, _view, &_surface2, color);
						_mainFont->drawColor(_scene->objectGetName(sceneObject->_sceneObjectId - SCENE_OBJECTS_OBJECTS_OFFSET), _surface2, pos.x, pos.y, color);
						break;
					}
			_surface2.frameRect(sceneObject->_screenRectangle, color);
				}
			}

			//draw regions
			for (int i = 0; i < 10; i++) {
				Region *region = &_scene->_regions->_regions[i];
				if (!region->_present) continue;
				_surface2.frameRect(region->_rectangle, 0b000000000011111);
			}

			for (int i = 0; i < 10; i++) {
				Region *region = &_scene->_exits->_regions[i];
				if (!region->_present) continue;
				_surface2.frameRect(region->_rectangle, 0b111111111111111);
			}


			//draw walkboxes
			for (int i = 0; i < _scene->_set->_walkboxCount; i++) {
				Walkbox *walkbox = &_scene->_set->_walkboxes[i];

				for (int j = 0; j < walkbox->_vertexCount; j++) {
					Vector3 start = _view->calculateScreenPosition(walkbox->_vertices[j]);
					Vector3 end = _view->calculateScreenPosition(walkbox->_vertices[(j + 1) % walkbox->_vertexCount]);
					_surface2.drawLine(start.x, start.y, end.x, end.y, 0b111111111100000);
					Vector3 pos = _view->calculateScreenPosition(0.5 * (start + end));
					_mainFont->drawColor(walkbox->_name, _surface2, pos.x, pos.y, 0b111111111100000);
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

				drawBBox(posOrigin - size, posOrigin + size, _view, &_surface2, color);

				Vector3 posOriginT = _view->calculateScreenPosition(posOrigin);
				Vector3 posTargetT = _view->calculateScreenPosition(posTarget);
				_surface2.drawLine(posOriginT.x, posOriginT.y, posTargetT.x, posTargetT.y, color);
				_mainFont->drawColor(light->_name, _surface2, posOriginT.x, posOriginT.y, color);
			}

			//draw waypoints
			for(int i = 0; i < _waypoints->_count; i++) {
				Waypoint *waypoint = &_waypoints->_waypoints[i];
				if(waypoint->_setId != _scene->getSetId())
					continue;
				Vector3 pos = waypoint->_position;
				Vector3 size = Vector3(5.0f, 5.0f, 5.0f);
				int color = 0b111111111111111;
				drawBBox(pos - size, pos + size, _view, &_surface2, color);
				Vector3 spos = _view->calculateScreenPosition(pos);
				char waypointText[40];
				sprintf(waypointText, "waypoint %i", i);
				_mainFont->drawColor(waypointText, _surface2, spos.x, spos.y, color);
			}
#endif

			_system->copyRectToScreen((const byte *)_surface2.getBasePtr(0, 0), _surface2.pitch, 0, 0, 640, 480);
			_system->updateScreen();
			_system->delayMillis(10);
		}
	}
}

void BladeRunnerEngine::actorsUpdate() {
	int actorCount = (int)_gameInfo->getActorCount();
	int setId = _scene->getSetId();

	//TODO: original game updates every non-visible characters by updating only one character in one frame
	if (setId != 89 || _gameVars[1] != 4 || _gameFlags->query(670) != 1 || !_aiScripts->IsInsideScript()) {
		for (int i = 0; i < actorCount; i++) {
			Actor *actor = _actors[i];
			if (actor->getSetId() == setId) {
				_aiScripts->Update(i);
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
		case Common::EVENT_LBUTTONDOWN:
		case Common::EVENT_RBUTTONDOWN:
			handleMouseClick(event.mouse.x, event.mouse.y);
		default:
			;
		}
	}
}

void BladeRunnerEngine::handleMouseClick(int x, int y) {
	if (!playerHasControl() || _mouse->isDisabled())
		return;

	Vector3 mousePosition = _mouse->getXYZ(x, y);

	int isClickable;
	int isObstacle;
	int isTarget;

	int sceneObjectId = _sceneObjects->findByXYZ(&isClickable, &isObstacle, &isTarget, mousePosition.x, mousePosition.y, mousePosition.z, 1, 0, 1);
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
		bool isRunning;
		_playerActor->loopWalkToXYZ(mousePosition, 0, false, false, false, &isRunning);
		debug("Clicked on nothing %f, %f, %f", mousePosition.x, mousePosition.y, mousePosition.z);
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

void BladeRunnerEngine::handleMouseClickExit(int x, int y, int exitIndex) {
	// clickedOnExit(exitType, x, y);
	debug("clicked on exit %d %d %d", exitIndex, x, y);
	_sceneScript->ClickedOnExit(exitIndex);
}

void BladeRunnerEngine::handleMouseClickRegion(int x, int y, int regionIndex) {
	debug("clicked on region %d %d %d", regionIndex, x, y);
	_sceneScript->ClickedOn2DRegion(regionIndex);
}

void BladeRunnerEngine::handleMouseClick3DObject(int x, int y, int objectId, bool isClickable, bool isTarget) {
	const char *objectName = _scene->objectGetName(objectId);
	debug("Clicked on object %s", objectName);
	_sceneScript->ClickedOn3DObject(objectName, false);
}

void BladeRunnerEngine::handleMouseClickItem(int x, int y, int itemId) {
	debug("Clicked on item %d", itemId);
	_sceneScript->ClickedOnItem(itemId, false);
}

void BladeRunnerEngine::handleMouseClickActor(int x, int y, int actorId) {
	debug("Clicked on actor %d", actorId);
	_sceneScript->ClickedOnActor(actorId);
}

void BladeRunnerEngine::gameWaitForActive() {
	while (!_windowIsActive) {
		handleEvents();
	}
}

void BladeRunnerEngine::loopActorSpeaking() {
	if (!_audioSpeech->isPlaying())
		return;

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
		if (_archives[i].isOpen() && _archives[i].getName() == name) {
			_archives[i].close();
			return true;
		}
	}

	debug("closeArchive: Archive %s not open.", name.c_str());
	return false;
}

bool BladeRunnerEngine::isArchiveOpen(const Common::String &name) {
	for (uint i = 0; i != 10; ++i) {
		if (_archives[i].isOpen() && _archives[i].getName() == name)
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

} // End of namespace BladeRunner
