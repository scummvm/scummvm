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
#include "common/str.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "engines/util.h"
#include "graphics/managed_surface.h"
#include "graphics/palette.h"
#include "graphics/pixelformat.h"
#include "graphics/surface.h"
#include "gui/debugger.h"
#include "twine/actor.h"
#include "twine/animations.h"
#include "twine/collision.h"
#include "twine/debug.h"
#include "twine/debug_grid.h"
#include "twine/debug_scene.h"
#include "twine/extra.h"
#include "twine/flamovies.h"
#include "twine/gamestate.h"
#include "twine/grid.h"
#include "twine/holomap.h"
#include "twine/hqrdepack.h"
#include "twine/interface.h"
#include "twine/keyboard.h"
#include "twine/menu.h"
#include "twine/menuoptions.h"
#include "twine/movements.h"
#include "twine/music.h"
#include "twine/redraw.h"
#include "twine/renderer.h"
#include "twine/resources.h"
#include "twine/scene.h"
#include "twine/screens.h"
#include "twine/script_life.h"
#include "twine/script_move.h"
#include "twine/sound.h"
#include "twine/text.h"

namespace TwinE {

TwinEEngine::TwinEEngine(OSystem *system, Common::Language language, uint32 flags)
    : Engine(system), _gameLang(language), _gameFlags(flags), _rnd("twine") {
	setDebugger(new GUI::Debugger());
	_actor = new Actor(this);
	_animations = new Animations(this);
	_collision = new Collision(this);
	_extra = new Extra(this);
	_gameState = new GameState(this);
	_grid = new Grid(this);
	_movements = new Movements(this);
	_hqrdepack = new HQRDepack(this);
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
	delete _hqrdepack;
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
	delete _debug;
	delete _debugScene;
}

Common::Error TwinEEngine::run() {
	/** Engine current version */
	const char *ENGINE_VERSION = "0.2.2";

	// Show engine information
	debug("TwinEngine v%s", ENGINE_VERSION);
	debug("(c)2002 The TwinEngine team.");
	debug("(c)2020 The ScummVM team.");
	debug("Refer to the credits for further details.");
	debug("Released under the terms of the GNU GPL license version 2 (or, at your opinion, any later). See COPYING file.");
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
	while (!shouldQuit()) {
		_menu->run();
	}
	_music->stopTrackMusic();
	_music->stopMidiMusic();
	return Common::kNoError;
}

bool TwinEEngine::hasFeature(EngineFeature f) const {
	return false;
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
	cfgfile.CrossFade = ConfGetIntOrDefault("CrossFade", 0);
	cfgfile.Fps = ConfGetIntOrDefault("Fps", DEFAULT_FRAMES_PER_SECOND);
	cfgfile.Debug = ConfGetIntOrDefault("Debug", 0) == 1;
	cfgfile.UseAutoSaving = ConfGetIntOrDefault("UseAutoSaving", 0);
	cfgfile.AutoAgressive = ConfGetIntOrDefault("CombatAuto", 0);
	cfgfile.ShadowMode = ConfGetIntOrDefault("Shadow", 0);
	cfgfile.SceZoom = ConfGetIntOrDefault("SceZoom", 0);
	cfgfile.WallCollision = ConfGetIntOrDefault("WallCollision", 0);
}

void TwinEEngine::initEngine() {
	// getting configuration file
	initConfigurations();

	_screens->clearScreen();

	// Check if LBA CD-Rom is on drive
	_music->initCdrom();

	// Display company logo
	_screens->adelineLogo();

	// verify game version screens
	if (cfgfile.Version == EUROPE_VERSION) {
		// Little Big Adventure screen
		_screens->loadImageDelay(RESSHQR_LBAIMG, 3);
		// Electronic Arts Logo
		_screens->loadImageDelay(RESSHQR_EAIMG, 2);
	} else if (cfgfile.Version == USA_VERSION) {
		// Relentless screen
		_screens->loadImageDelay(RESSHQR_RELLENTIMG, 3);
		// Electronic Arts Logo
		_screens->loadImageDelay(RESSHQR_EAIMG, 2);
	} else if (cfgfile.Version == MODIFICATION_VERSION) {
		// Modification screen
		_screens->loadImageDelay(RESSHQR_RELLENTIMG, 2);
	}

	_flaMovies->playFlaMovie(FLA_DRAGON3);

	_screens->loadMenuImage();
}

void TwinEEngine::initMCGA() {
	_redraw->drawInGameTransBox = true;
}

void TwinEEngine::initSVGA() {
	_redraw->drawInGameTransBox = false;
}

void TwinEEngine::initAll() {
	_grid->blockBuffer = (uint8 *)malloc(64 * 64 * 25 * 2 * sizeof(uint8));
	_animations->animBuffer1 = _animations->animBuffer2 = (uint8 *)malloc(5000 * sizeof(uint8));
	memset(_menu->itemAngle, 256, sizeof(_menu->itemAngle)); // reset inventory items angles

	_redraw->bubbleSpriteIndex = SPRITEHQR_DIAG_BUBBLE_LEFT;

	_scene->sceneHero = &_scene->sceneActors[0];

	_redraw->renderRight = SCREEN_TEXTLIMIT_RIGHT;
	_redraw->renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
	// Set clip to fullscreen by default, allows main menu to render properly after load
	_interface->resetClip();

	_resources->initResources();

	initSVGA();
}

int TwinEEngine::getRandomNumber(uint max) {
	return _rnd.getRandomNumber(max);
}

void TwinEEngine::freezeTime() {
	if (!isTimeFreezed) {
		saveFreezedTime = lbaTime;
	}
	isTimeFreezed++;
}

void TwinEEngine::unfreezeTime() {
	--isTimeFreezed;
	if (isTimeFreezed == 0) {
		lbaTime = saveFreezedTime;
	}
}

void TwinEEngine::processActorSamplePosition(int32 actorIdx) {
	const ActorStruct *actor = &_scene->sceneActors[actorIdx];
	const int32 channelIdx = _sound->getActorChannel(actorIdx);
	_sound->setSamplePosition(channelIdx, actor->x, actor->y, actor->z);
}

int32 TwinEEngine::runGameEngine() { // mainLoopInteration
	readKeys();

	if (_scene->needChangeScene > -1) {
		_scene->changeScene();
	}

	previousLoopPressedKey = loopPressedKey;
	_keyboard.key = _keyboard.pressedKey;
	loopPressedKey = _keyboard.skippedKey;
	loopCurrentKey = _keyboard.internalKeyCode;

	_debug->processDebug(loopCurrentKey);

	if (_menuOptions->canShowCredits != 0) {
		// TODO: if current music playing != 8, than play_track(8);
		if (_keyboard.internalKeyCode != 0) {
			return 0;
		}
		if (_keyboard.pressedKey != 0) {
			return 0;
		}
		if (_keyboard.skippedKey != 0) {
			return 0;
		}
	} else {
		// Process give up menu - Press ESC
		if (_keyboard.internalKeyCode == 1 && _scene->sceneHero->life > 0 && _scene->sceneHero->entity != -1 && !_scene->sceneHero->staticFlags.bIsHidden) {
			freezeTime();
			if (_menu->giveupMenu()) {
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				freezeTime();
				_gameState->saveGame(); // auto save game
				quitGame = 0;
				unfreezeTime();
				return 0;
			}
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		if (loopCurrentKey == twineactions[TwinEActionType::OptionsMenu].localKey) {
			freezeTime();
			_sound->pauseSamples();
			_menu->OptionsMenuSettings[MenuSettings_FirstButton] = 15; // TODO: why? - where is the reset? kReturnGame
			_text->initTextBank(0);
			_menu->optionsMenu();
			_text->initTextBank(_text->currentTextBank + 3);
			// TODO: play music
			_sound->resumeSamples();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// inventory menu
		loopInventoryItem = -1;
		if (loopCurrentKey == twineactions[TwinEActionType::InventoryMenu].localKey && _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == kManual) {
			freezeTime();
			_menu->processInventoryMenu();

			switch (loopInventoryItem) {
			case kiHolomap:
				_holomap->processHolomap();
				_screens->lockPalette = true;
				warning("Use inventory [kiHolomap] not implemented!\n");
				break;
			case kiMagicBall:
				if (_gameState->usingSabre == 1) {
					_actor->initModelActor(0, 0);
				}
				_gameState->usingSabre = 0;
				break;
			case kiUseSabre:
				if (_scene->sceneHero->body != InventoryItems::kiUseSabre) {
					if (_actor->heroBehaviour == kProtoPack) {
						_actor->setBehaviour(kNormal);
					}
					_actor->initModelActor(InventoryItems::kiUseSabre, 0);
					_animations->initAnim(kSabreUnknown, 1, 0, 0);

					_gameState->usingSabre = 1;
				}
				break;
			case kiBookOfBu: {
				_screens->fadeToBlack(_screens->paletteRGBA);
				_screens->loadImage(RESSHQR_INTROSCREEN1IMG);
				_text->initTextBank(2);
				_text->newGameVar4 = 0;
				_text->textClipFull();
				_text->setFontCrossColor(15);
				int32 tmpFlagDisplayText = cfgfile.FlagDisplayText;
				cfgfile.FlagDisplayText = 1;
				_text->drawTextFullscreen(161);
				cfgfile.FlagDisplayText = tmpFlagDisplayText;
				_text->textClipSmall();
				_text->newGameVar4 = 1;
				_text->initTextBank(_text->currentTextBank + 3);
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
				ActorStruct *pinguin = &_scene->sceneActors[_scene->mecaPinguinIdx];

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
			} break;
			case kiBonusList: {
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				freezeTime();
				_text->initTextBank(2);
				_text->textClipFull();
				_text->setFontCrossColor(15);
				_text->drawTextFullscreen(162);
				_text->textClipSmall();
				_text->initTextBank(_text->currentTextBank + 3);
			} break;
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
		if ((loopCurrentKey == twineactions[TwinEActionType::BehaviourMenu].localKey ||
		     loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourNormal].localKey ||
		     loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourAthletic].localKey ||
		     loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourAggressive].localKey ||
		     loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourDiscreet].localKey) &&
		    _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == kManual) {
			if (loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourNormal].localKey) {
				_actor->heroBehaviour = HeroBehaviourType::kNormal;
			} else if (loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourAthletic].localKey) {
				_actor->heroBehaviour = HeroBehaviourType::kAthletic;
			} else if (loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourAggressive].localKey) {
				_actor->heroBehaviour = HeroBehaviourType::kAggressive;
			} else if (loopCurrentKey == twineactions[TwinEActionType::QuickBehaviourDiscreet].localKey) {
				_actor->heroBehaviour = HeroBehaviourType::kDiscrete;
			}
			freezeTime();
			_menu->processBehaviourMenu();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// use Proto-Pack
		if (loopCurrentKey == twineactions[TwinEActionType::UseProtoPack].localKey && _gameState->gameFlags[InventoryItems::kiProtoPack] == 1) {
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
		if ((loopPressedKey & 2) && !disableScreenRecenter) {
			_grid->newCameraX = _scene->sceneActors[_scene->currentlyFollowedActor].x >> 9;
			_grid->newCameraY = _scene->sceneActors[_scene->currentlyFollowedActor].y >> 8;
			_grid->newCameraZ = _scene->sceneActors[_scene->currentlyFollowedActor].z >> 9;
			_redraw->reqBgRedraw = true;
		}

		// Draw holomap
		if (loopCurrentKey == 35 && _gameState->gameFlags[InventoryItems::kiHolomap] == 1 && !_gameState->gameFlags[GAMEFLAG_INVENTORY_DISABLED]) {
			freezeTime();
			//TestRestoreModeSVGA(1);
			_holomap->processHolomap();
			_screens->lockPalette = true;
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// Process Pause
		if (loopCurrentKey == twineactions[TwinEActionType::Pause].localKey) {
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
			} while (_keyboard.internalKeyCode != 0x19 && !_keyboard.pressedKey);
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}
	}

	loopActorStep = _movements->getRealValue(&loopMovePtr);
	if (!loopActorStep) {
		loopActorStep = 1;
	}

	_movements->setActorAngle(0, -256, 5, &loopMovePtr);
	disableScreenRecenter = 0;

	_scene->processEnvironmentSound();

	// Reset HitBy state
	for (int32 a = 0; a < _scene->sceneNumActors; a++) {
		_scene->sceneActors[a].hitBy = -1;
	}

	_extra->processExtras();

	for (int32 a = 0; a < _scene->sceneNumActors; a++) {
		ActorStruct *actor = &_scene->sceneActors[a];

		if (!actor->dynamicFlags.bIsDead) {
			if (actor->life == 0) {
				if (a == 0) { // if its hero who died
					_animations->initAnim(kLandDeath, 4, 0, 0);
					actor->controlMode = 0;
				} else {
					_sound->playSample(37, getRandomNumber(2000) + 3096, 1, actor->x, actor->y, actor->z, a);

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
							_sound->playSample(0x25, rnd, 1, actor->x, actor->y, actor->z, a);
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
								actor->controlMode = 0;
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

							_scene->heroPositionType = kReborn;

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

							_gameState->saveGame();
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
		ActorStruct *actor = &_scene->sceneActors[_scene->currentlyFollowedActor];
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

bool TwinEEngine::gameEngineLoop() { // mainLoop
	uint32 start;

	_redraw->reqBgRedraw = true;
	_screens->lockPalette = true;
	_movements->setActorAngle(0, -256, 5, &loopMovePtr);

	while (quitGame == -1) {
		start = g_system->getMillis();

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

void TwinEEngine::delaySkip(uint32 time) {
	uint32 startTicks = _system->getMillis();
	uint32 stopTicks = 0;
	_keyboard.internalKeyCode = 0;
	do {
		readKeys();
		if (_keyboard.internalKeyCode == 1) {
			break;
		}
		if (shouldQuit()) {
			break;
		}
		stopTicks = _system->getMillis() - startTicks;
		_system->delayMillis(1);
		//lbaTime++;
	} while (stopTicks <= time);
}

void TwinEEngine::setPalette(const uint32 *palette) {
#if 1
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
#else
	frontVideoBuffer.setPalette(palette, 0, 256);
	workVideoBuffer.setPalette(palette, 0, 256);
#endif
	flip();
}

void TwinEEngine::flip() {
	g_system->copyRectToScreen(frontVideoBuffer.getPixels(), frontVideoBuffer.pitch, 0, 0, frontVideoBuffer.w, frontVideoBuffer.h);
	g_system->updateScreen();
}

void TwinEEngine::copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom) {
	assert(left < right);
	assert(top < bottom);
#if 0
	// TODO: fix this - looks like the palette includes a color key at pos 0
	g_system->copyRectToScreen(frontVideoBuffer.getPixels(), frontVideoBuffer.pitch, left, top, right - left + 1, bottom - top + 1);
	g_system->updateScreen();
#else
	flip();
#endif
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

/** Pressed key char map - scanCodeTab2 */
static const struct KeyProperties {
	uint8 high;
	bool pressed;
	uint8 key;
} pressedKeyCharMap[] = {
    {0x01, false, 0x48}, // up
    {0x02, false, 0x50}, // down
    {0x04, false, 0x4B}, // left
    {0x08, false, 0x4D}, // right
    {0x05, false, 0x47}, // home
    {0x09, false, 0x49}, // pageup
    {0x0A, false, 0x51}, // pagedown
    {0x06, false, 0x4F}, // end
    {0x01, true,  0x39}, // space bar
    {0x02, true,  0x1C}, // enter
    {0x04, true,  0x1D}, // ctrl
    {0x08, true,  0x38}, // alt
    {0x10, true,  0x53}, // del
    {0x20, true,  0x2A}, // left shift
    {0x20, true,  0x36}, // right shift
    {0x01, true,  0x3B}, // F1
    {0x02, true,  0x3C}, // F2
    {0x04, true,  0x3D}, // F3
    {0x08, true,  0x3E}, // F4
    {0x10, true,  0x3F}, // F5
    {0x20, true,  0x40}, // F6
    {0x40, true,  0x41}, // F7
    {0x80, true,  0x42}, // F8
    {0x01, true,  0x43}, // F9
    {0x02, true,  0x44}, // F10
    {0x04, true,  0x57}, // ?
    {0x08, true,  0x58}, // ?
    {0x00, true,  0x2A}, // left shift
    {0x00, true,  0x00},
    {0x01, false, 0x01}, // esc
    {0x00, false, 0x00}};
static_assert(ARRAYSIZE(pressedKeyCharMap) == 31, "Expected size of key char map");

void TwinEEngine::readKeys() {
	if (shouldQuit()) {
		_keyboard.internalKeyCode = 1;
		_keyboard.skippedKey = 1;
		return;
	}
	_keyboard.skippedKey = 0;
	_keyboard.internalKeyCode = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		uint8 localKey = 0;
		switch (event.type) {
		case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
			_keyboard.actionStates[event.customType] = false;
			localKey = twineactions[event.customType].localKey;
			break;
		case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
			if (!cfgfile.Debug) {
				switch (event.customType) {
				case TwinEActionType::NextRoom:
				case TwinEActionType::PreviousRoom:
				case TwinEActionType::ApplyCellingGrid:
				case TwinEActionType::IncreaseCellingGridIndex:
				case TwinEActionType::DecreaseCellingGridIndex:
					break;
				default:
					localKey = twineactions[event.customType].localKey;
					_keyboard.actionStates[event.customType] = true;
					break;
				}
			} else {
				localKey = twineactions[event.customType].localKey;
				_keyboard.actionStates[event.customType] = true;
			}
			break;
		case Common::EVENT_LBUTTONDOWN:
			leftMouse = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			rightMouse = 1;
			break;
		default:
			break;
		}

		if (localKey == 0) {
			continue;
		}

		for (int i = 0; i < ARRAYSIZE(pressedKeyCharMap); i++) {
			if (pressedKeyCharMap[i].key == localKey) {
				if (pressedKeyCharMap[i].pressed) {
					if (event.type == Common::EVENT_CUSTOM_ENGINE_ACTION_END) {
						_keyboard.pressedKey &= ~pressedKeyCharMap[i].high;
					} else {
						_keyboard.pressedKey |= pressedKeyCharMap[i].high;
					}
				} else {
					_keyboard.skippedKey |= pressedKeyCharMap[i].high;
				}
				break;
			}
		}
		_keyboard.internalKeyCode = localKey;
	}
}

void TwinEEngine::drawText(int32 x, int32 y, const char *string, int32 center) {
#if 0 // TODO
	SDL_Color white = {0xFF, 0xFF, 0xFF, 0};
	SDL_Color *forecol = &white;
	SDL_Rect rectangle;
	Graphics::ManagedSurface *text = TTF_RenderText_Solid(font, string, *forecol);

	if (center) {
		rectangle.x = x - (text->w / 2);
	} else {
		rectangle.x = x;
	}
	rectangle.y = y - 2;
	rectangle.w = text->w;
	rectangle.h = text->h;

	SDL_BlitSurface(text, NULL, screenBuffer, &rectangle);
	SDL_FreeSurface(text);
#endif
}

void TwinEEngine::getMousePositions(MouseStatusStruct *mouseData) {
	Common::Point point = g_system->getEventManager()->getMousePos();
	mouseData->x = point.x;
	mouseData->y = point.y;
	mouseData->left = leftMouse;
	mouseData->right = rightMouse;
	leftMouse = 0;
	rightMouse = 0;
}

} // namespace TwinE
