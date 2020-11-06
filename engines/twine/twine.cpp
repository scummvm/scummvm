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

#include "twine/twine.h"
#include "common/config-manager.h"
#include "common/debug.h"
#include "common/error.h"
#include "common/events.h"
#include "common/keyboard.h"
#include "common/savefile.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "engines/metaengine.h"
#include "engines/util.h"
#include "graphics/colormasks.h"
#include "graphics/fontman.h"
#include "graphics/font.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/console.h"
#include "twine/debug.h"
#include "twine/debug_grid.h"
#include "twine/debug_scene.h"
#include "twine/extra.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/holomap.h"
#include "twine/hqr.h"
#include "twine/input.h"
#include "twine/interface.h"
#include "twine/menu.h"
#include "twine/menuoptions.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/script_life_v1.h"
#include "twine/script_move_v1.h"
#include "twine/sound.h"
#include "twine/text.h"

namespace TwinE {

ScopedEngineFreeze::ScopedEngineFreeze(TwinEEngine* engine) : _engine(engine) {
	_engine->freezeTime();
}

ScopedEngineFreeze::~ScopedEngineFreeze() {
	_engine->unfreezeTime();
}

TwinEEngine::TwinEEngine(OSystem *system, Common::Language language, uint32 flags, TwineGameType gameType)
    : Engine(system), _gameType(gameType), _gameLang(language), _gameFlags(flags), _rnd("twine") {
	// Add default file directories
	const Common::FSNode gameDataDir(ConfMan.get("path"));
	SearchMan.addSubDirectoryMatching(gameDataDir, "fla");
	SearchMan.addSubDirectoryMatching(gameDataDir, "vox");

	setDebugger(new TwinEConsole(this));
	_actor = new Actor(this);
	_animations = new Animations(this);
	_collision = new Collision(this);
	_extra = new Extra(this);
	_gameState = new GameState(this);
	_grid = new Grid(this);
	_movements = new Movements(this);
	_interface = new Interface(this);
	_menu = new Menu(this);
	_flaMovies = new FlaMovies(this);
	_menuOptions = new MenuOptions(this);
	_music = new Music(this);
	_redraw = new Redraw(this);
	_renderer = new Renderer(this);
	_resources = new Resources(this);
	_scene = new Scene(this);
	_screens = new Screens(this);
	_scriptLife = new ScriptLife(this);
	_scriptMove = new ScriptMove(this);
	_holomap = new Holomap(this);
	_sound = new Sound(this);
	_text = new Text(this);
	_debugGrid = new DebugGrid(this);
	_input = new Input(this);
	_debug = new Debug(this);
	_debugScene = new DebugScene(this);
}

TwinEEngine::~TwinEEngine() {
	delete _actor;
	delete _animations;
	delete _collision;
	delete _extra;
	delete _gameState;
	delete _grid;
	delete _movements;
	delete _interface;
	delete _menu;
	delete _flaMovies;
	delete _music;
	delete _redraw;
	delete _renderer;
	delete _resources;
	delete _scene;
	delete _screens;
	delete _scriptLife;
	delete _scriptMove;
	delete _holomap;
	delete _sound;
	delete _text;
	delete _debugGrid;
	delete _input;
	delete _debug;
	delete _debugScene;
}

Common::Error TwinEEngine::run() {
	debug("Based on TwinEngine v0.2.2");
	debug("(c)2002 The TwinEngine team.");
	debug("(c)2020 The ScummVM team.");
	debug("Refer to the credits for further details.");
	debug("The original Little Big Adventure game is:");
	debug("(c)1994 by Adeline Software International, All Rights Reserved.");

	syncSoundSettings();
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	allocVideoMemory();
	initAll();
	initEngine();
	_sound->stopSamples();
	_screens->copyScreen(frontVideoBuffer, workVideoBuffer);

	_menu->init();

	if (ConfMan.hasKey("save_slot")) {
		const int saveSlot = ConfMan.getInt("save_slot");
		if (saveSlot >= 0 && saveSlot <= 999) {
			Common::Error state = loadGameState(saveSlot);
			if (state.getCode() != Common::kNoError) {
				return state;
			}
		}
	}
	while (!shouldQuit()) {
		readKeys();
		switch (_state) {
		case EngineState::QuitGame: {
			Common::Event event;
			event.type = Common::EVENT_QUIT;
			_system->getEventManager()->pushEvent(event);
			break;
		}
		case EngineState::LoadedGame:
			debug("Loaded game");
			if (_scene->newHeroX == -1) {
				_scene->heroPositionType = ScenePositionType::kNoPosition;
			}
			_text->newGameVar5 = 0;
			_text->textClipSmall();
			_text->newGameVar4 = 1;
			_state = EngineState::GameLoop;
			break;
		case EngineState::GameLoop:
			if (gameEngineLoop()) {
				_menuOptions->showCredits();
			}
			_state = EngineState::Menu;
			break;
		case EngineState::Menu:
			_state = _menu->run();
			break;
		}
	}

	ConfMan.setInt("CombatAuto", _actor->autoAgressive ? 1 : 0);
	ConfMan.setInt("Shadow", cfgfile.ShadowMode);
	ConfMan.setInt("SceZoom", cfgfile.SceZoom ? 1 : 0);
	ConfMan.setInt("PolygonDetails", cfgfile.PolygonDetails);

	_sound->stopSamples();
	_music->stopTrackMusic();
	_music->stopMidiMusic();
	return Common::kNoError;
}

bool TwinEEngine::hasFeature(EngineFeature f) const {
	switch (f) {
	case EngineFeature::kSupportsReturnToLauncher:
	case EngineFeature::kSupportsLoadingDuringRuntime:
	case EngineFeature::kSupportsSavingDuringRuntime:
	case EngineFeature::kSupportsChangingOptionsDuringRuntime:
		return true;
	default:
		break;
	}
	return false;
}

SaveStateList TwinEEngine::getSaveSlots() const {
	return getMetaEngine().listSaves(_targetName.c_str());
}

void TwinEEngine::wipeSaveSlot(int slot) {
	Common::SaveFileManager *saveFileMan = getSaveFileManager();
	const Common::String &saveFile = getSaveStateName(slot);
	saveFileMan->removeSavefile(saveFile);
}

bool TwinEEngine::canSaveGameStateCurrently() { return _scene->currentScene != nullptr; }

Common::Error TwinEEngine::loadGameStream(Common::SeekableReadStream *stream) {
	debug("load game stream");
	if (!_gameState->loadGame(stream)) {
		return Common::Error(Common::kReadingFailed);
	}
	_state = EngineState::LoadedGame;
	return Common::Error(Common::kNoError);
}

Common::Error TwinEEngine::saveGameStream(Common::WriteStream *stream, bool isAutosave) {
	if (!_gameState->saveGame(stream)) {
		return Common::Error(Common::kWritingFailed);
	}
	return Common::Error(Common::kNoError);
}

void TwinEEngine::autoSave() {
	// TODO: scene title, not player name
	saveGameState(getAutosaveSlot(), _gameState->playerName, true);
}

void TwinEEngine::allocVideoMemory() {
	const Graphics::PixelFormat format = Graphics::PixelFormat::createFormatCLUT8();
	workVideoBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, format);
	frontVideoBuffer.create(SCREEN_WIDTH, SCREEN_HEIGHT, format);

	int32 j = 0;
	int32 k = 0;
	for (int32 i = SCREEN_HEIGHT; i > 0; i--) {
		screenLookupTable[j] = k;
		j++;
		k += SCREEN_WIDTH;
	}

	// initVideoVar1 = -1;
}

static int getLanguageTypeIndex(const char *languageName) {
	char buffer[256];
	Common::strlcpy(buffer, languageName, sizeof(buffer));

	char *ptr = strchr(buffer, ' ');
	if (ptr != nullptr) {
		*ptr = '\0';
	}

	const int32 length = ARRAYSIZE(LanguageTypes);
	for (int32 i = 0; i < length; i++) {
		if (!strcmp(LanguageTypes[i].name, buffer)) {
			return i;
		}
	}

	debug("Failed to detect language %s - falling back to english", languageName);
	// select english for the fan translations
	return 0; // English
}

#define ConfGetOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? ConfMan.get(key) : Common::String(defaultVal))
#define ConfGetIntOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? atoi(ConfMan.get(key).c_str()) : (defaultVal))

void TwinEEngine::initConfigurations() {
	// TODO: use existing entries for some of the settings - like volume and so on.

	const char *lng = Common::getLanguageDescription(_gameLang);
	cfgfile.LanguageId = getLanguageTypeIndex(lng);
	cfgfile.Voice = ConfGetOrDefault("Voice", "ON") == "ON";
	cfgfile.FlagDisplayText = ConfGetOrDefault("FlagDisplayText", "ON") == "ON";
	const Common::String midiType = ConfGetOrDefault("MidiType", "auto");
	if (midiType == "None") {
		cfgfile.MidiType = MIDIFILE_NONE;
	} else {
		Common::File midiHqr;
		if (midiHqr.exists(Resources::HQR_MIDI_MI_WIN_FILE)) {
			cfgfile.MidiType = MIDIFILE_WIN;
			debug("Use %s for midi", Resources::HQR_MIDI_MI_WIN_FILE);
		} else if (midiHqr.exists(Resources::HQR_MIDI_MI_DOS_FILE)) {
			cfgfile.MidiType = MIDIFILE_DOS;
			debug("Use %s for midi", Resources::HQR_MIDI_MI_DOS_FILE);
		} else {
			cfgfile.MidiType = MIDIFILE_NONE;
			debug("Could not find midi hqr file");
		}
	}
	cfgfile.Version = ConfGetIntOrDefault("Version", EUROPE_VERSION);
	cfgfile.UseCD = ConfGetIntOrDefault("UseCD", 0);
	cfgfile.Sound = ConfGetIntOrDefault("Sound", 1);
	cfgfile.Movie = ConfGetIntOrDefault("Movie", CONF_MOVIE_FLA);
	cfgfile.Fps = ConfGetIntOrDefault("Fps", DEFAULT_FRAMES_PER_SECOND);
	cfgfile.Debug = ConfGetIntOrDefault("Debug", 0) == 1;

	cfgfile.UseAutoSaving = ConfGetIntOrDefault("UseAutoSaving", 0);
	cfgfile.CrossFade = ConfGetIntOrDefault("CrossFade", 0);
	cfgfile.WallCollision = ConfGetIntOrDefault("WallCollision", 0);

	_actor->autoAgressive = ConfGetIntOrDefault("CombatAuto", 1) == 1;
	cfgfile.ShadowMode = ConfGetIntOrDefault("Shadow", 2);
	cfgfile.SceZoom = ConfGetIntOrDefault("SceZoom", 0) == 0;
	cfgfile.PolygonDetails = ConfGetIntOrDefault("PolygonDetails", 2);
}

void TwinEEngine::initEngine() {
	// getting configuration file
	initConfigurations();

	_screens->clearScreen();

	// Check if LBA CD-Rom is on drive
	_music->initCdrom();

	_input->enableKeyMap(cutsceneKeyMapId);
	// Display company logo
	bool abort = false;
	abort |= _screens->adelineLogo();

	// verify game version screens
	if (!abort && cfgfile.Version == EUROPE_VERSION) {
		// Little Big Adventure screen
		abort |= _screens->loadImageDelay(RESSHQR_LBAIMG, 3);
		if (!abort) {
			// Electronic Arts Logo
			abort |= _screens->loadImageDelay(RESSHQR_EAIMG, 2);
		}
	} else if (!abort && cfgfile.Version == USA_VERSION) {
		// Relentless screen
		abort |= _screens->loadImageDelay(RESSHQR_RELLENTIMG, 3);
		if (!abort) {
			// Electronic Arts Logo
			abort |= _screens->loadImageDelay(RESSHQR_EAIMG, 2);
		}
	} else if (!abort && cfgfile.Version == MODIFICATION_VERSION) {
		// Modification screen
		abort |= _screens->loadImageDelay(RESSHQR_RELLENTIMG, 2);
	}

	if (!abort) {
		_flaMovies->playFlaMovie(FLA_DRAGON3);
	}
	_input->enableKeyMap(uiKeyMapId);

	_screens->loadMenuImage();
}

void TwinEEngine::initMCGA() {
	_redraw->drawInGameTransBox = true;
}

void TwinEEngine::initSVGA() {
	_redraw->drawInGameTransBox = false;
}

void TwinEEngine::initAll() {
	memset(_menu->itemAngle, 0, sizeof(_menu->itemAngle)); // reset inventory items angles

	_redraw->bubbleSpriteIndex = SPRITEHQR_DIAG_BUBBLE_LEFT;

	_scene->sceneHero = _scene->getActor(OWN_ACTOR_SCENE_INDEX);

	_redraw->renderRight = SCREEN_TEXTLIMIT_RIGHT;
	_redraw->renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
	// Set clip to fullscreen by default, allows main menu to render properly after load
	_interface->resetClip();

	_resources->initResources();

	initSVGA();
}

int TwinEEngine::getRandomNumber(uint max) {
	if (max == 0) {
		return 0;
	}
	return _rnd.getRandomNumber(max - 1);
}

void TwinEEngine::freezeTime() {
	if (!isTimeFreezed) {
		saveFreezedTime = lbaTime;
		_pauseToken = pauseEngine();
	}
	isTimeFreezed++;
}

void TwinEEngine::unfreezeTime() {
	--isTimeFreezed;
	if (isTimeFreezed == 0) {
		lbaTime = saveFreezedTime;
		_pauseToken.clear();
	}
}

void TwinEEngine::processActorSamplePosition(int32 actorIdx) {
	const ActorStruct *actor = _scene->getActor(actorIdx);
	const int32 channelIdx = _sound->getActorChannel(actorIdx);
	_sound->setSamplePosition(channelIdx, actor->x, actor->y, actor->z);
}

int32 TwinEEngine::runGameEngine() { // mainLoopInteration
	_input->enableKeyMap(mainKeyMapId);

	readKeys();

	if (_scene->needChangeScene > -1) {
		_scene->changeScene();
	}

	_movements->update();

	_debug->processDebug();

	if (_menuOptions->canShowCredits) {
		// TODO: if current music playing != 8, than play_track(8);
		if (_input->toggleAbortAction()) {
			return 0;
		}
	} else {
		// Process give up menu - Press ESC
		if (_input->toggleAbortAction() && _scene->sceneHero->life > 0 && _scene->sceneHero->entity != -1 && !_scene->sceneHero->staticFlags.bIsHidden) {
			freezeTime();
			const int giveUp = _menu->giveupMenu();
			if (giveUp == kQuitEngine) {
				return 0;
			}
			if (giveUp == 1) {
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				ScopedEngineFreeze freeze(this);
				autoSave();
				quitGame = 0;
				return 0;
			}
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		if (_input->toggleActionIfActive(TwinEActionType::OptionsMenu)) {
			freezeTime();
			_sound->pauseSamples();
			_menu->inGameOptionsMenu();
			// TODO: play music
			_sound->resumeSamples();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// inventory menu
		loopInventoryItem = -1;
		if (_input->isActionActive(TwinEActionType::InventoryMenu) && _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == ControlMode::kManual) {
			freezeTime();
			_menu->processInventoryMenu();

			switch (loopInventoryItem) {
			case kiHolomap:
				_holomap->processHolomap();
				_screens->lockPalette = true;
				warning("Use inventory [kiHolomap] not implemented!\n");
				break;
			case kiMagicBall:
				if (_gameState->usingSabre) {
					_actor->initModelActor(0, 0);
				}
				_gameState->usingSabre = false;
				break;
			case kiUseSabre:
				if (_scene->sceneHero->body != InventoryItems::kiUseSabre) {
					if (_actor->heroBehaviour == kProtoPack) {
						_actor->setBehaviour(kNormal);
					}
					_actor->initModelActor(InventoryItems::kiUseSabre, 0);
					_animations->initAnim(kSabreUnknown, 1, 0, 0);

					_gameState->usingSabre = true;
				}
				break;
			case kiBookOfBu: {
				_screens->fadeToBlack(_screens->paletteRGBA);
				_screens->loadImage(RESSHQR_INTROSCREEN1IMG);
				_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
				_text->newGameVar4 = 0;
				_text->textClipFull();
				_text->setFontCrossColor(15);
				const bool tmpFlagDisplayText = cfgfile.FlagDisplayText;
				cfgfile.FlagDisplayText = true;
				_text->drawTextFullscreen(161);
				cfgfile.FlagDisplayText = tmpFlagDisplayText;
				_text->textClipSmall();
				_text->newGameVar4 = 1;
				_text->initTextBank(_scene->sceneTextBank + 3);
				_screens->fadeToBlack(_screens->paletteRGBACustom);
				_screens->clearScreen();
				flip();
				setPalette(_screens->paletteRGBA);
				_screens->lockPalette = true;
			} break;
			case kiProtoPack:
				if (_gameState->gameFlags[InventoryItems::kiBookOfBu]) {
					_scene->sceneHero->body = 0;
				} else {
					_scene->sceneHero->body = 1;
				}

				if (_actor->heroBehaviour == kProtoPack) {
					_actor->setBehaviour(kNormal);
				} else {
					_actor->setBehaviour(kProtoPack);
				}
				break;
			case kiPinguin: {
				ActorStruct *pinguin = _scene->getActor(_scene->mecaPinguinIdx);

				pinguin->x = _renderer->destX + _scene->sceneHero->x;
				pinguin->y = _scene->sceneHero->y;
				pinguin->z = _renderer->destZ + _scene->sceneHero->z;
				pinguin->angle = _scene->sceneHero->angle;

				_movements->rotateActor(0, 800, pinguin->angle);

				if (!_collision->checkCollisionWithActors(_scene->mecaPinguinIdx)) {
					pinguin->life = 50;
					pinguin->body = -1;
					_actor->initModelActor(0, _scene->mecaPinguinIdx);
					pinguin->dynamicFlags.bIsDead = 0; // &= 0xDF
					pinguin->brickShape = 0;
					_movements->moveActor(pinguin->angle, pinguin->angle, pinguin->speed, &pinguin->move);
					_gameState->gameFlags[InventoryItems::kiPinguin] = 0; // byte_50D89 = 0;
					pinguin->info0 = lbaTime + 1500;
				}
				break;
			}
			case kiBonusList: {
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				freezeTime();
				_text->initTextBank(TextBankId::Inventory_Intro_and_Holomap);
				_text->textClipFull();
				_text->setFontCrossColor(15);
				_text->drawTextFullscreen(162);
				_text->textClipSmall();
				_text->initTextBank(_scene->sceneTextBank + 3);
				break;
			}
			case kiCloverLeaf:
				if (_scene->sceneHero->life < 50) {
					if (_gameState->inventoryNumLeafs > 0) {
						_scene->sceneHero->life = 50;
						_gameState->inventoryMagicPoints = _gameState->magicLevelIdx * 20;
						_gameState->inventoryNumLeafs--;
						_redraw->addOverlay(koInventoryItem, 27, 0, 0, 0, koNormal, 3);
					}
				}
				break;
			}

			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// Process behaviour menu
		if ((_input->isActionActive(TwinEActionType::BehaviourMenu, false) ||
		     _input->isActionActive(TwinEActionType::QuickBehaviourNormal, false) ||
		     _input->isActionActive(TwinEActionType::QuickBehaviourAthletic, false) ||
		     _input->isActionActive(TwinEActionType::QuickBehaviourAggressive, false) ||
		     _input->isActionActive(TwinEActionType::QuickBehaviourDiscreet, false)) &&
		    _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == ControlMode::kManual) {
			if (_input->isActionActive(TwinEActionType::QuickBehaviourNormal, false)) {
				_actor->heroBehaviour = HeroBehaviourType::kNormal;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourAthletic, false)) {
				_actor->heroBehaviour = HeroBehaviourType::kAthletic;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourAggressive, false)) {
				_actor->heroBehaviour = HeroBehaviourType::kAggressive;
			} else if (_input->isActionActive(TwinEActionType::QuickBehaviourDiscreet, false)) {
				_actor->heroBehaviour = HeroBehaviourType::kDiscrete;
			}
			freezeTime();
			_menu->processBehaviourMenu();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// use Proto-Pack
		if (_input->isActionActive(TwinEActionType::UseProtoPack, false) && _gameState->gameFlags[InventoryItems::kiProtoPack] == 1) {
			if (_gameState->gameFlags[InventoryItems::kiBookOfBu]) {
				_scene->sceneHero->body = 0;
			} else {
				_scene->sceneHero->body = 1;
			}

			if (_actor->heroBehaviour == kProtoPack) {
				_actor->setBehaviour(kNormal);
			} else {
				_actor->setBehaviour(kProtoPack);
			}
		}

		// Recenter Screen
		if (_input->isActionActive(TwinEActionType::RecenterScreenOnTwinsen) && !disableScreenRecenter) {
			const ActorStruct *currentlyFollowedActor = _scene->getActor(_scene->currentlyFollowedActor);
			_grid->newCameraX = currentlyFollowedActor->x >> 9;
			_grid->newCameraY = currentlyFollowedActor->y >> 8;
			_grid->newCameraZ = currentlyFollowedActor->z >> 9;
			_redraw->reqBgRedraw = true;
		}

		// Draw holomap
		if (_input->isActionActive(TwinEActionType::OpenHolomap) && _gameState->gameFlags[InventoryItems::kiHolomap] == 1 && !_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED]) {
			freezeTime();
			//TestRestoreModeSVGA(1);
			_holomap->processHolomap();
			_screens->lockPalette = true;
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// Process Pause
		if (_input->toggleActionIfActive(TwinEActionType::Pause)) {
			freezeTime();
			_text->setFontColor(15);
			_text->drawText(5, 446, "Pause"); // no key for pause in Text Bank
			copyBlockPhys(5, 446, 100, 479);
			do {
				readKeys();
				if (shouldQuit()) {
					break;
				}
				g_system->delayMillis(10);
			} while (!_input->toggleActionIfActive(TwinEActionType::Pause));
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}
	}

	loopActorStep = _movements->getRealValue(&loopMovePtr);
	if (!loopActorStep) {
		loopActorStep = 1;
	}

	_movements->setActorAngle(0, -256, 5, &loopMovePtr);
	disableScreenRecenter = false;

	_scene->processEnvironmentSound();

	// Reset HitBy state
	for (int32 a = 0; a < _scene->sceneNumActors; a++) {
		_scene->getActor(a)->hitBy = -1;
	}

	_extra->processExtras();

	for (int32 a = 0; a < _scene->sceneNumActors; a++) {
		ActorStruct *actor = _scene->getActor(a);

		if (!actor->dynamicFlags.bIsDead) {
			if (actor->life == 0) {
				if (a == 0) { // if its hero who died
					_animations->initAnim(kLandDeath, 4, 0, 0);
					actor->controlMode = ControlMode::kNoMove;
				} else {
					_sound->playSample(Samples::Explode, getRandomNumber(2000) + 3096, 1, actor->x, actor->y, actor->z, a);

					if (a == _scene->mecaPinguinIdx) {
						_extra->addExtraExplode(actor->x, actor->y, actor->z);
					}
				}

				if (actor->bonusParameter & 0x1F0 && !(actor->bonusParameter & 1)) {
					_actor->processActorExtraBonus(a);
				}
			}

			_movements->processActorMovements(a);

			actor->collisionX = actor->x;
			actor->collisionY = actor->y;
			actor->collisionZ = actor->z;

			if (actor->positionInMoveScript != -1) {
				_scriptMove->processMoveScript(a);
			}

			_animations->processActorAnimations(a);

			if (actor->staticFlags.bIsZonable) {
				_scene->processActorZones(a);
			}

			if (actor->positionInLifeScript != -1) {
				_scriptLife->processLifeScript(a);
			}

			processActorSamplePosition(a);

			if (quitGame != -1) {
				return quitGame;
			}

			if (actor->staticFlags.bCanDrown) {
				int32 brickSound;
				brickSound = _grid->getBrickSoundType(actor->x, actor->y - 1, actor->z);
				actor->brickSound = brickSound;

				if ((brickSound & 0xF0) == 0xF0) {
					if ((brickSound & 0xF) == 1) {
						if (a) { // all other actors
							int32 rnd = getRandomNumber(2000) + 3096;
							_sound->playSample(Samples::Explode, rnd, 1, actor->x, actor->y, actor->z, a);
							if (actor->bonusParameter & 0x1F0) {
								if (!(actor->bonusParameter & 1)) {
									_actor->processActorExtraBonus(a);
								}
								actor->life = 0;
							}
						} else { // if Hero
							if (_actor->heroBehaviour != 4 || (brickSound & 0x0F) != actor->anim) {
								if (!_actor->cropBottomScreen) {
									_animations->initAnim(kDrawn, 4, 0, 0);
									_renderer->projectPositionOnScreen(actor->x - _grid->cameraX, actor->y - _grid->cameraY, actor->z - _grid->cameraZ);
									_actor->cropBottomScreen = _renderer->projPosY;
								}
								_renderer->projectPositionOnScreen(actor->x - _grid->cameraX, actor->y - _grid->cameraY, actor->z - _grid->cameraZ);
								actor->controlMode = ControlMode::kNoMove;
								actor->life = -1;
								_actor->cropBottomScreen = _renderer->projPosY;
								actor->staticFlags.bCanDrown |= 0x10;
							}
						}
					}
				}
			}

			if (actor->life <= 0) {
				if (!a) { // if its Hero
					if (actor->dynamicFlags.bAnimEnded) {
						if (_gameState->inventoryNumLeafs > 0) { // use clover leaf automaticaly
							_scene->sceneHero->x = _scene->newHeroX;
							_scene->sceneHero->y = _scene->newHeroY;
							_scene->sceneHero->z = _scene->newHeroZ;

							_scene->needChangeScene = _scene->currentSceneIdx;
							_gameState->inventoryMagicPoints = _gameState->magicLevelIdx * 20;

							_grid->newCameraX = (_scene->sceneHero->x >> 9);
							_grid->newCameraY = (_scene->sceneHero->y >> 8);
							_grid->newCameraZ = (_scene->sceneHero->z >> 9);

							_scene->heroPositionType = ScenePositionType::kReborn;

							_scene->sceneHero->life = 50;
							_redraw->reqBgRedraw = true;
							_screens->lockPalette = true;
							_gameState->inventoryNumLeafs--;
							_actor->cropBottomScreen = 0;
						} else { // game over
							_gameState->inventoryNumLeafsBox = 2;
							_gameState->inventoryNumLeafs = 1;
							_gameState->inventoryMagicPoints = _gameState->magicLevelIdx * 20;
							_actor->heroBehaviour = _actor->previousHeroBehaviour;
							actor->angle = _actor->previousHeroAngle;
							actor->life = 50;

							if (_scene->previousSceneIdx != _scene->currentSceneIdx) {
								_scene->newHeroX = -1;
								_scene->newHeroY = -1;
								_scene->newHeroZ = -1;
								_scene->currentSceneIdx = _scene->previousSceneIdx;
							}

							autoSave();
							_gameState->processGameoverAnimation();
							quitGame = 0;
							return 0;
						}
					}
				} else {
					_actor->processActorCarrier(a);
					actor->dynamicFlags.bIsDead = 1;
					actor->entity = -1;
					actor->zone = -1;
				}
			}

			if (_scene->needChangeScene != -1) {
				return 0;
			}
		}
	}

	// recenter screen automatically
	if (!disableScreenRecenter && !_debugGrid->useFreeCamera) {
		ActorStruct *actor = _scene->getActor(_scene->currentlyFollowedActor);
		_renderer->projectPositionOnScreen(actor->x - (_grid->newCameraX << 9),
		                                   actor->y - (_grid->newCameraY << 8),
		                                   actor->z - (_grid->newCameraZ << 9));
		if (_renderer->projPosX < 80 || _renderer->projPosX > 539 || _renderer->projPosY < 80 || _renderer->projPosY > 429) {
			_grid->newCameraX = ((actor->x + 0x100) >> 9) + (((actor->x + 0x100) >> 9) - _grid->newCameraX) / 2;
			_grid->newCameraY = actor->y >> 8;
			_grid->newCameraZ = ((actor->z + 0x100) >> 9) + (((actor->z + 0x100) >> 9) - _grid->newCameraZ) / 2;

			if (_grid->newCameraX >= 64) {
				_grid->newCameraX = 63;
			}

			if (_grid->newCameraZ >= 64) {
				_grid->newCameraZ = 63;
			}

			_redraw->reqBgRedraw = true;
		}
	}

	_redraw->redrawEngineActions(_redraw->reqBgRedraw);

	// workaround to fix hero redraw after drowning
	if (_actor->cropBottomScreen && _redraw->reqBgRedraw == 1) {
		_scene->sceneHero->staticFlags.bIsHidden = 1;
		_redraw->redrawEngineActions(1);
		_scene->sceneHero->staticFlags.bIsHidden = 0;
	}

	_scene->needChangeScene = -1;
	_redraw->reqBgRedraw = 0;

	return 0;
}

bool TwinEEngine::gameEngineLoop() {
	_redraw->reqBgRedraw = true;
	_screens->lockPalette = true;
	_movements->setActorAngle(0, -256, 5, &loopMovePtr);

	while (quitGame == -1) {
		uint32 start = g_system->getMillis();

		while (g_system->getMillis() < start + cfgfile.Fps) {
			if (runGameEngine()) {
				return true;
			}
			g_system->delayMillis(10);
		}
		lbaTime++;
		if (shouldQuit()) {
			break;
		}
	}
	return false;
}

bool TwinEEngine::delaySkip(uint32 time) {
	uint32 startTicks = _system->getMillis();
	uint32 stopTicks = 0;
	do {
		readKeys();
		if (_input->toggleAbortAction()) {
			return true;
		}
		if (shouldQuit()) {
			return true;
		}
		stopTicks = _system->getMillis() - startTicks;
		_system->delayMillis(1);
		//lbaTime++;
	} while (stopTicks <= time);
	return false;
}

void TwinEEngine::setPalette(const uint32 *palette) {
	uint8 pal[NUMOFCOLORS * 3];
	uint8 *out = pal;
	const uint8 *in = (const uint8 *)palette;
	for (int i = 0; i < NUMOFCOLORS; i++) {
		out[0] = in[0];
		out[1] = in[1];
		out[2] = in[2];
		out += 3;
		in += 4;
	}
	g_system->getPaletteManager()->setPalette(pal, 0, 256);
	flip();
}

void TwinEEngine::flip() {
	g_system->copyRectToScreen(frontVideoBuffer.getPixels(), frontVideoBuffer.pitch, 0, 0, frontVideoBuffer.w, frontVideoBuffer.h);
	g_system->updateScreen();
}

void TwinEEngine::copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom) {
	assert(left <= right);
	assert(top <= bottom);
	int32 width = right - left + 1;
	int32 height = bottom - top + 1;
	if (left + width > SCREEN_WIDTH) {
		width = SCREEN_WIDTH - left;
	}
	if (top + height > SCREEN_HEIGHT) {
		height = SCREEN_HEIGHT - top;
	}
	if (width <= 0 || height <= 0) {
		return;
	}
	g_system->copyRectToScreen(frontVideoBuffer.getBasePtr(left, top), frontVideoBuffer.pitch, left, top, width, height);
	g_system->updateScreen();
}

void TwinEEngine::crossFade(const Graphics::ManagedSurface &buffer, const uint32 *palette) {
	Graphics::ManagedSurface backupSurface;
	Graphics::ManagedSurface newSurface;
	Graphics::ManagedSurface tempSurface;
	Graphics::ManagedSurface surfaceTable;

	Graphics::PixelFormat fmt(4, 8, 8, 8, 8, 24, 16, 8, 0);
	backupSurface.create(frontVideoBuffer.w, frontVideoBuffer.h, fmt);
	newSurface.create(frontVideoBuffer.w, frontVideoBuffer.h, fmt);
	tempSurface.create(frontVideoBuffer.w, frontVideoBuffer.h, Graphics::PixelFormat::createFormatCLUT8());
	tempSurface.setPalette(palette, 0, 256);

	surfaceTable.create(frontVideoBuffer.w, frontVideoBuffer.h, fmt);

	backupSurface.transBlitFrom(frontVideoBuffer);
	newSurface.transBlitFrom(tempSurface);

	for (int32 i = 0; i < 8; i++) {
		surfaceTable.blitFrom(backupSurface);
		surfaceTable.transBlitFrom(newSurface, 0, false, 0, i * 32);
		frontVideoBuffer.blitFrom(surfaceTable);
		flip();
		delaySkip(50);
	}

	frontVideoBuffer.blitFrom(newSurface);
	flip();

	backupSurface.free();
	newSurface.free();
	tempSurface.free();
	surfaceTable.free();
}

void TwinEEngine::readKeys() {
	_input->readKeys();
}

void TwinEEngine::drawText(int32 x, int32 y, const char *string, int32 center) {
	const Graphics::Font *font = FontMan.getFontByUsage(Graphics::FontManager::kGUIFont);
	if (!font) {
		return;
	}
	int width = 100;
	const Common::String text(string);
	font->drawString(&frontVideoBuffer, text,
	                 x, y, width,
	                 frontVideoBuffer.format.RGBToColor(255, 255, 255),
	                 center ? Graphics::kTextAlignCenter : Graphics::kTextAlignLeft);
}

} // namespace TwinE
