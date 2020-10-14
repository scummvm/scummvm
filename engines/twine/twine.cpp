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
#include "graphics/palette.h"
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

/** Engine current version */
static const char *ENGINE_VERSION = "0.2.0";

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
	debug("Starting twine");
	syncSoundSettings();
	initGraphics(SCREEN_WIDTH, SCREEN_HEIGHT);
	allocVideoMemory();
	initAll();
	initEngine();
	_music->stopTrackMusic();
	_music->stopMidiMusic();
	return Common::kNoError;
}

bool TwinEEngine::hasFeature(EngineFeature f) const {
	return false;
}

void TwinEEngine::allocVideoMemory() {
	const size_t videoBufferSize = (SCREEN_WIDTH * SCREEN_HEIGHT) * sizeof(uint8);
	workVideoBuffer = (uint8 *)malloc(videoBufferSize);
	frontVideoBuffer = (uint8 *)malloc(videoBufferSize);

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

	return 0; // English
}

#define ConfGetOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? ConfMan.get(key) : Common::String(defaultVal))
#define ConfGetIntOrDefault(key, defaultVal) (ConfMan.hasKey(key) ? atoi(ConfMan.get(key).c_str()) : (defaultVal))

void TwinEEngine::initConfigurations() {
	// TODO: use existing entries for some of the settings - like volume and so on.

	Common::String language = ConfGetOrDefault("Language", Common::getLanguageDescription(_gameLang));
	cfgfile.LanguageId = getLanguageTypeIndex(language.c_str()) + 1;

	Common::String languageCD = ConfGetOrDefault("LanguageCD", "None");
	cfgfile.LanguageCDId = getLanguageTypeIndex(languageCD.c_str()) + 1;

	cfgfile.FlagDisplayText = ConfGetOrDefault("FlagDisplayText", "ON") == "ON";
	cfgfile.FlagKeepVoice = ConfGetOrDefault("FlagKeepVoice", "OFF") == "ON";
	const Common::String midiType = ConfGetOrDefault("MidiType", "auto");
	if (midiType == "auto") {
		Common::File midiHqr;
		if (midiHqr.exists(Resources::HQR_MIDI_MI_WIN_FILE)) {
			cfgfile.MidiType = 1;
		} else {
			cfgfile.MidiType = 0;
		}
	} else if (midiType == "midi") {
		cfgfile.MidiType = 1;
	} else {
		cfgfile.MidiType = 0;
	}
	cfgfile.Version = ConfGetIntOrDefault("Version", EUROPE_VERSION);
	cfgfile.FullScreen = ConfGetIntOrDefault("FullScreen", 1) == 1;
	cfgfile.UseCD = ConfGetIntOrDefault("UseCD", 0);
	cfgfile.Sound = ConfGetIntOrDefault("Sound", 0);
	cfgfile.Movie = ConfGetIntOrDefault("Movie", 0);
	cfgfile.CrossFade = ConfGetIntOrDefault("CrossFade", 0);
	cfgfile.Fps = ConfGetIntOrDefault("Fps", DEFAULT_FRAMES_PER_SECOND);
	cfgfile.Debug = ConfGetIntOrDefault("Debug", 0);
	cfgfile.UseAutoSaving = ConfGetIntOrDefault("UseAutoSaving", 0);
	cfgfile.AutoAgressive = ConfGetIntOrDefault("CombatAuto", 0);
	cfgfile.ShadowMode = ConfGetIntOrDefault("Shadow", 0);
	cfgfile.SceZoom = ConfGetIntOrDefault("SceZoom", 0);
	cfgfile.WallCollision = ConfGetIntOrDefault("WallCollision", 0);
}

void TwinEEngine::initEngine() {
	// getting configuration file
	initConfigurations();

	// Show engine information
	debug("TwinEngine v%s", ENGINE_VERSION);
	debug("(c)2002 The TwinEngine team.");
	debug("(c)2020 The ScummVM team.");
	debug("Refer to the credits for further details.");
	debug("Released under the terms of the GNU GPL license version 2 (or, at your opinion, any later). See COPYING file.");
	debug("The original Little Big Adventure game is:");
	debug("(c)1994 by Adeline Software International, All Rights Reserved.");

	_screens->clearScreen();

	// Toggle fullscreen if Fullscreen flag is set
	toggleFullscreen();

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

	_menu->mainMenu();
}

void TwinEEngine::initMCGA() {
	_redraw->drawInGameTransBox = 1;
}

void TwinEEngine::initSVGA() {
	_redraw->drawInGameTransBox = 0;
}

void TwinEEngine::initAll() {
	_grid->blockBuffer = (uint8 *)malloc(64 * 64 * 25 * 2 * sizeof(uint8));
	_animations->animBuffer1 = _animations->animBuffer2 = (uint8 *)malloc(5000 * sizeof(uint8));
	memset(_menu->itemAngle, 256, sizeof(_menu->itemAngle)); // reset inventory items angles

	_redraw->bubbleSpriteIndex = SPRITEHQR_DIAG_BUBBLE_LEFT;
	_redraw->bubbleActor = -1;
	_text->showDialogueBubble = 1;

	_text->currentTextBank = -1;
	_menu->currMenuTextIndex = -1;
	_menu->currMenuTextBank = -1;
	_actor->autoAgressive = 1;

	_scene->sceneHero = &_scene->sceneActors[0];

	_redraw->renderLeft = 0;
	_redraw->renderTop = 0;
	_redraw->renderRight = SCREEN_TEXTLIMIT_RIGHT;
	_redraw->renderBottom = SCREEN_TEXTLIMIT_BOTTOM;
	// Set clip to fullscreen by default, allows main menu to render properly after load
	_interface->resetClip();

	rightMouse = 0;
	leftMouse = 0;

	_resources->initResources();

	initSVGA();
}

int TwinEEngine::getRandomNumber(uint max) {
	return _rnd.getRandomNumber(max);
}

void TwinEEngine::freezeTime() {
	if (!isTimeFreezed)
		saveFreezedTime = lbaTime;
	isTimeFreezed++;
}

void TwinEEngine::unfreezeTime() {
	--isTimeFreezed;
	if (isTimeFreezed == 0)
		lbaTime = saveFreezedTime;
}

void TwinEEngine::processActorSamplePosition(int32 actorIdx) {
	int32 channelIdx;
	ActorStruct *actor = &_scene->sceneActors[actorIdx];
	channelIdx = _sound->getActorChannel(actorIdx);
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
	loopCurrentKey = _keyboard.skipIntro;

	_debug->processDebug(loopCurrentKey);

	if (_menuOptions->canShowCredits != 0) {
		// TODO: if current music playing != 8, than play_track(8);
		if (_keyboard.skipIntro != 0) {
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
		if (_keyboard.skipIntro == 1 && _scene->sceneHero->life > 0 && _scene->sceneHero->entity != -1 && !_scene->sceneHero->staticFlags.bIsHidden) {
			freezeTime();
			if (_menu->giveupMenu()) {
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				freezeTime();
				_gameState->saveGame(); // auto save game
				quitGame = 0;
				cfgfile.Quit = 0;
				unfreezeTime();
				return 0;
			}
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// Process options menu - Press F6
		if (loopCurrentKey == 0x40) {
			int tmpLangCD = cfgfile.LanguageCDId;
			freezeTime();
			_sound->pauseSamples();
			_menu->OptionsMenuSettings[5] = 15;
			cfgfile.LanguageCDId = 0;
			_text->initTextBank(0);
			_menu->optionsMenu();
			cfgfile.LanguageCDId = tmpLangCD;
			_text->initTextBank(_text->currentTextBank + 3);
			//TODO: play music
			_sound->resumeSamples();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// inventory menu
		loopInventoryItem = -1;
		if (loopCurrentKey == Keys::InventoryMenu && _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == kManual) {
			freezeTime();
			_menu->processInventoryMenu();

			switch (loopInventoryItem) {
			case kiHolomap:
				warning("Use Inventory [kiHolomap] not implemented!\n");
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
				_screens->fadeToBlack(_screens->paletteRGB);
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
				_screens->fadeToBlack(_screens->paletteRGBCustom);
				_screens->clearScreen();
				flip();
				setPalette(_screens->paletteRGB);
				_screens->lockPalette = 1;
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
				int32 tmpLanguageCDIdx;
				tmpLanguageCDIdx = cfgfile.LanguageCDId;
				unfreezeTime();
				_redraw->redrawEngineActions(1);
				freezeTime();
				cfgfile.LanguageCDId = 0;
				_text->initTextBank(2);
				_text->textClipFull();
				_text->setFontCrossColor(15);
				_text->drawTextFullscreen(162);
				_text->textClipSmall();
				cfgfile.LanguageCDId = tmpLanguageCDIdx;
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

		// Process behaviour menu - Press CTRL and F1..F4 Keys
		if ((loopCurrentKey == Keys::BehaviourMenu
		  || loopCurrentKey == Keys::QuickBehaviourNormal
		  || loopCurrentKey == Keys::QuickBehaviourAthletic
		  || loopCurrentKey == Keys::QuickBehaviourAggressive
		  || loopCurrentKey == Keys::QuickBehaviourDiscreet)
		  && _scene->sceneHero->entity != -1 && _scene->sceneHero->controlMode == kManual) {
			if (loopCurrentKey == Keys::QuickBehaviourNormal) {
				_actor->heroBehaviour = HeroBehaviourType::kNormal;
			} else if (loopCurrentKey == Keys::QuickBehaviourAthletic) {
				_actor->heroBehaviour = HeroBehaviourType::kAthletic;
			} else if (loopCurrentKey == Keys::QuickBehaviourAggressive) {
				_actor->heroBehaviour = HeroBehaviourType::kAggressive;
			} else if (loopCurrentKey == Keys::QuickBehaviourDiscreet) {
				_actor->heroBehaviour = HeroBehaviourType::kDiscrete;
			}
			freezeTime();
			_menu->processBehaviourMenu();
			unfreezeTime();
			_redraw->redrawEngineActions(1);
		}

		// use Proto-Pack
		if (loopCurrentKey == Keys::UseProtoPack && _gameState->gameFlags[InventoryItems::kiProtoPack] == 1) {
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

		// Press Enter to Recenter Screen
		if ((loopPressedKey & 2) && !disableScreenRecenter) {
			_grid->newCameraX = _scene->sceneActors[_scene->currentlyFollowedActor].x >> 9;
			_grid->newCameraY = _scene->sceneActors[_scene->currentlyFollowedActor].y >> 8;
			_grid->newCameraZ = _scene->sceneActors[_scene->currentlyFollowedActor].z >> 9;
			_redraw->reqBgRedraw = 1;
		}

		// TODO: draw holomap

		// Process Pause - Press P
		if (loopCurrentKey == Keys::Pause) {
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
			} while (_keyboard.skipIntro != 0x19 && !_keyboard.pressedKey);
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
							_redraw->reqBgRedraw = 1;
							_screens->lockPalette = 1;
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

			_redraw->reqBgRedraw = 1;
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

	_redraw->reqBgRedraw = 1;
	_screens->lockPalette = 1;
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
	_keyboard.skipIntro = 0;
	do {
		readKeys();
		if (_keyboard.skipIntro == 1) {
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

void TwinEEngine::setPalette(uint8 *palette) {
	g_system->getPaletteManager()->setPalette(palette, 0, 256);
	flip();
}

void TwinEEngine::fadeBlackToWhite() {
#if 0
	SDL_Color colorPtr[256];
	SDL_UpdateRect(screen, 0, 0, 0, 0);
	for (int32 i = 0; i < 256; i += 3) {
		memset(colorPtr, i, sizeof(colorPtr));
		SDL_SetPalette(screen, SDL_PHYSPAL, colorPtr, 0, 256);
	}
#endif
}

void TwinEEngine::flip() {
	g_system->copyRectToScreen(frontVideoBuffer, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
}

void TwinEEngine::copyBlockPhys(int32 left, int32 top, int32 right, int32 bottom) {
	g_system->copyRectToScreen(frontVideoBuffer, SCREEN_WIDTH, left, top, right - left + 1, bottom - top + 1);
	g_system->updateScreen();
}

void TwinEEngine::crossFade(uint8 *buffer, uint8 *palette) {
	// TODO: implement cross fading
	g_system->copyRectToScreen(buffer, SCREEN_WIDTH, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	g_system->updateScreen();
}

void TwinEEngine::toggleFullscreen() {
	_redraw->reqBgRedraw = 1;
	_system->setFeatureState(OSystem::kFeatureFullscreenMode, cfgfile.FullScreen);
	cfgfile.FullScreen = !cfgfile.FullScreen;
}

/** Pressed key map - scanCodeTab1 */
static const uint8 pressedKeyMap[] = {
    0x48, // 0
    0x50,
    0x4B,
    0x4D,
    0x47,
    0x49,
    0x51,
    0x4F, // 7

    0x39, // 8
    0x1C,
    0x1D,
    0x38,
    0x53,
    0x2A,
    0x36, // 14

    0x3B, // 15
    0x3C,
    0x3D,
    0x3E,
    0x3F,
    0x40, // LBAKEY_F6
    0x41,
    0x42,
    0x43,
    0x44,
    0x57,
    0x58,
    0x2A,
    0x0, // 28
};
static_assert(ARRAYSIZE(pressedKeyMap) == 29, "Expected size of key map");

/** Pressed key char map - scanCodeTab2 */
static const uint16 pressedKeyCharMap[] = {
    0x0100, // up
    0x0200, // down
    0x0400, // left
    0x0800, // right
    0x0500, // home
    0x0900, // pageup
    0x0A00, // pagedown
    0x0600, // end

    0x0101, // space bar
    0x0201, // enter
    0x0401, // ctrl
    0x0801, // alt
    0x1001, // del
    0x2001, // left shift
    0x2001, // right shift

    0x0102, // F1
    0x0202, // F2
    0x0402, // F3
    0x0802, // F4
    0x1002, // F5
    0x2002, // F6
    0x4002, // F7
    0x8002, // F8

    0x0103, // F9
    0x0203, // F10
    0x0403, // ?
    0x0803, // ?
    0x00FF, // left shift
    0x00FF,
    0x0,
    0x0,
};
static_assert(ARRAYSIZE(pressedKeyCharMap) == 31, "Expected size of key char map");

/** Handle keyboard pressed keys */
void TwinEEngine::readKeys() {
	if (shouldQuit()) {
		_keyboard.skipIntro = 1;
		_keyboard.skippedKey = 1;
		return;
	}
	_keyboard.skippedKey = 0;
	_keyboard.skipIntro = 0;
	int32 localKey = 0;

	Common::Event event;
	while (g_system->getEventManager()->pollEvent(event)) {
		switch (event.type) {
		case Common::EVENT_KEYUP:
			_keyboard.pressedKey = 0;
			break;
		case Common::EVENT_KEYDOWN: {
			switch (event.kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				localKey = 0x1;
				break;
			case Common::KEYCODE_SPACE:
				localKey = Keys::ExecuteBehaviourAction;
				break;
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				localKey = Keys::RecenterScreenOnTwinsen; // TODO: depends on the context
				break;
			case Common::KEYCODE_LSHIFT:
			case Common::KEYCODE_RSHIFT:
				localKey = Keys::InventoryMenu;
				break;
			case Common::KEYCODE_LALT:
			case Common::KEYCODE_RALT:
				localKey = 0x38;
				break;
			case Common::KEYCODE_LCTRL:
			case Common::KEYCODE_RCTRL:
				localKey = Keys::BehaviourMenu;
				break;
			case Common::KEYCODE_PAGEUP:
				localKey = 0x49;
				break;
			case Common::KEYCODE_p: // pause
				localKey = Keys::Pause;
				break;
			case Common::KEYCODE_h: // holomap
				localKey = Keys::OpenHolomap;
				break;
			case Common::KEYCODE_j:
				localKey = Keys::UseProtoPack;
				break;
			case Common::KEYCODE_w: // Especial key to do the action
				localKey = 0x11;
				break;
			case Common::KEYCODE_F1:
				localKey = Keys::QuickBehaviourNormal;
				break;
			case Common::KEYCODE_F2:
				localKey = Keys::QuickBehaviourAthletic;
				break;
			case Common::KEYCODE_F3:
				localKey = Keys::QuickBehaviourAggressive;
				break;
			case Common::KEYCODE_F4:
				localKey = Keys::QuickBehaviourDiscreet;
				break;
			case Common::KEYCODE_F6:
				localKey = Keys::OptionsMenu;
				break;
			case Common::KEYCODE_F12:
				toggleFullscreen();
				break;
			default:
				break;
			}
			if (cfgfile.Debug) {
				switch (event.kbd.keycode) {
				case Common::KEYCODE_r: // next room
					localKey = Keys::NextRoom;
					break;
				case Common::KEYCODE_f: // previous room
					localKey = Keys::PreviousRoom;
					break;
				case Common::KEYCODE_t: // apply celling grid
					localKey = Keys::ApplyCellingGrid;
					break;
				case Common::KEYCODE_g: // increase celling grid index
					localKey = Keys::IncreaseCellingGridIndex;
					break;
				case Common::KEYCODE_b: // decrease celling grid index
					localKey = Keys::DecreaseCellingGridIndex;
					break;
				default:
					break;
				}
			}
			break;
		}
		case Common::EVENT_LBUTTONDOWN:
			leftMouse = 1;
			break;
		case Common::EVENT_RBUTTONDOWN:
			rightMouse = 1;
			break;
		default:
			break;
		}
	}

	int32 size = 0;
	uint8 *keyboard = nullptr; // TODO: SDL_GetKeyState(&size);
	for (int32 j = 0; j < size; j++) {
		if (keyboard[j]) {
			switch (j) {
			case Common::KEYCODE_RETURN:
			case Common::KEYCODE_KP_ENTER:
				localKey = Keys::RecenterScreenOnTwinsen; // TODO: depends on the context
				break;
			case Common::KEYCODE_SPACE:
				localKey = Keys::ExecuteBehaviourAction;
				break;
			case Common::KEYCODE_UP:
			case Common::KEYCODE_KP8:
				localKey = Keys::MoveForward;
				break;
			case Common::KEYCODE_DOWN:
			case Common::KEYCODE_KP2:
				localKey = Keys::MoveBackward;
				break;
			case Common::KEYCODE_LEFT:
			case Common::KEYCODE_KP4:
				localKey = Keys::TurnLeft;
				break;
			case Common::KEYCODE_RIGHT:
			case Common::KEYCODE_KP6:
				localKey = Keys::TurnRight;
				break;
			case Common::KEYCODE_LCTRL:
			case Common::KEYCODE_RCTRL:
				localKey = Keys::BehaviourMenu;
				break;
			/*case Common::KEYCODE_LSHIFT:
			case Common::KEYCODE_RSHIFT:
				localKey = 0x36;
				break;*/
			case Common::KEYCODE_LALT:
			case Common::KEYCODE_RALT:
				localKey = 0x38;
				break;
			case Common::KEYCODE_F1:
				localKey = Keys::QuickBehaviourNormal;
				break;
			case Common::KEYCODE_F2:
				localKey = Keys::QuickBehaviourAthletic;
				break;
			case Common::KEYCODE_F3:
				localKey = Keys::QuickBehaviourAggressive;
				break;
			case Common::KEYCODE_F4:
				localKey = Keys::QuickBehaviourDiscreet;
				break;
			default:
				break;
			}
			if (cfgfile.Debug) {
				switch (keyboard[j]) {
				// change grid camera
				case Common::KEYCODE_s:
					localKey = 0x1F;
					break;
				case Common::KEYCODE_x:
					localKey = 0x2D;
					break;
				case Common::KEYCODE_z:
					localKey = 0x2C;
					break;
				case Common::KEYCODE_c:
					localKey = 0x2E;
					break;
				}
			}
		}

		int find = 0;
		bool found = false;
		for (int i = 0; i < 28; i++) {
			if (pressedKeyMap[i] == localKey) {
				find = i;
				found = true;
				break;
			}
		}

		if (found) {
			int16 temp = pressedKeyCharMap[find];
			uint8 temp2 = temp & 0x00FF;

			if (temp2 == 0) {
				// pressed valid keys
				if (!(localKey & 0x80)) {
					_keyboard.pressedKey |= (temp & 0xFF00) >> 8;
				} else {
					_keyboard.pressedKey &= -((temp & 0xFF00) >> 8);
				}
			}
			// pressed inactive keys
			else {
				_keyboard.skippedKey |= (temp & 0xFF00) >> 8;
			}
		}

		//if (!found) {
		_keyboard.skipIntro = localKey;
		//}
	}
}

void TwinEEngine::drawText(int32 x, int32 y, const char *string, int32 center) {
#if 0 // TODO
	SDL_Color white = {0xFF, 0xFF, 0xFF, 0};
	SDL_Color *forecol = &white;
	SDL_Rect rectangle;
	Graphics::Surface *text = TTF_RenderText_Solid(font, string, *forecol);

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
	mouseData->X = point.x;
	mouseData->Y = point.y;
	mouseData->left = leftMouse;
	mouseData->right = rightMouse;
	leftMouse = 0;
	rightMouse = 0;
}

} // namespace TwinE
