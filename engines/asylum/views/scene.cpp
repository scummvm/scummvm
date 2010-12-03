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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "asylum/views/scene.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/script.h"
#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/savegame.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

#include "asylum/views/menu.h"
#include "asylum/views/scenetitle.h"

#include "asylum/asylum.h"
#include "asylum/respack.h"
#include "asylum/staticres.h"

#include "common/file.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugPolygons;
int g_debugObjects;
int g_debugScrolling;

Scene::Scene(AsylumEngine *engine): _vm(engine),
	_polygons(NULL), _ws(NULL) {

	// Initialize data
	_packId = kResourcePackInvalid;
	_playerIndex = 0;

	_hitAreaChapter7Counter = 0;
	_chapter5FrameIndex = 0;

	g_debugPolygons  = 0;
	g_debugObjects  = 0;
	g_debugScrolling = 0;
}

Scene::~Scene() {
	// Unload the associated resources
	getResource()->unload(_packId);

	// Clear script queue
	getScript()->reset();

	delete _polygons;
	delete _ws;

	// Zero passed pointers
	_vm = NULL;
}

void Scene::enter(ResourcePackId packId) {
	_vm->setGameFlag(kGameFlagScriptProcessing);

	getCursor()->hide();

	_playerIndex = 0;

	// Load the scene data
	load(packId);

	// Set wheel indices
	_ws->setWheelObjects();

	// Adjust object priority
	if (_ws->objects.size() > 0) {
		int32 priority = 4091;

		for (uint32 i = 0; i < _ws->objects.size(); i++) {
			Object *object  = _ws->objects[i];
			object->setPriority(priority);
			object->flags &= ~kObjectFlagC000;
			priority -= 4;
		}
	}

	// Set the cursor to magnifying glass
	getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass], 0, kCursorAnimationNone);
	getCursor()->show();

	// Clear the graphic queue
	getScreen()->clearGraphicsInQueue();

	_ws->sceneRectIdx = 0;
	_ws->motionStatus = 1;

	// Update current player bounding rectangle
	Actor *player = getActor();
	Common::Rect *boundingRect = player->getBoundingRect();
	boundingRect->bottom = (int16)player->getPoint2()->y;
	boundingRect->right  = (int16)(player->getPoint2()->x * 2);

	// Adjust scene bounding rect
	_ws->boundingRect = Common::Rect(195, 115, 445 - boundingRect->right, 345 - boundingRect->bottom);

	// Hide actor
	player->hide();
	player->enable();

	// Update current player coordinates
	player->getPoint1()->x -= player->getPoint2()->x;
	player->getPoint1()->y -= player->getPoint2()->y;

	// Update all other actors
	if (_ws->actors.size() > 1) {
		for (uint32 i = 1; i < _ws->actors.size(); i++) {
			Actor *actor = _ws->actors[i];

			actor->hide();
			actor->setDirection(kDirectionNO);
			actor->enable();

			actor->getPoint1()->x -= actor->getPoint2()->x;
			actor->getPoint1()->y -= actor->getPoint2()->y;

			actor->getBoundingRect()->bottom = (int16)actor->getPoint2()->y;
			actor->getBoundingRect()->right  = (int16)(2 * actor->getPoint2()->x);
		}
	}

	// Queue scene script
	if (_ws->scriptIndex)
		getScript()->queueScript(_ws->scriptIndex, 0);

	// Clear the graphic queue (FIXME: not sure why we need to do this again)
	getScreen()->clearGraphicsInQueue();

	// Load transparency tables
	getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);
	getScreen()->selectTransTable(1);

	// Setup font
	getText()->loadFont(_ws->font1);

	// Preload graphics (we are just showing the loading screen
	preload();

	// Play scene intro dialog
	playIntroSpeech();

	// Set actor type
	_ws->actorType = actorType[_ws->chapter];

	// Play intro music
	if (_ws->musicCurrentResourceIndex != kMusicStopped && _ws->chapter != kChapter1)
		getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, _ws->musicCurrentResourceIndex));
	else
		getSound()->playMusic(kResourceNone, 0);

	// Update global values
	_vm->lastScreenUpdate = 1;
	getSharedData()->setFlag(kFlagScene1, true);

	player->setLastScreenUpdate(_vm->screenUpdateCount);
	player->enable();

	if (_ws->chapter == kChapter9) {
		changePlayer(1);
		_ws->nextPlayer = kActorInvalid;
	}
}

void Scene::load(ResourcePackId packId) {
	// Setup resource manager
	_packId = packId;
	getResource()->setMusicPackId(packId);

	char filename[10];
	sprintf(filename, SCENE_FILE_MASK, _packId);

	char sceneTag[6];
	Common::File* fd = new Common::File;

	if (!Common::File::exists(filename))
		error("Scene file doesn't exist %s", filename);

	fd->open(filename);

	if (!fd->isOpen())
		error("Failed to load scene file %s", filename);

	fd->read(sceneTag, 6);

	if (Common::String(sceneTag, 6) != "DFISCN")
		error("The file isn't recognized as scene %s", filename);

	_ws = new WorldStats(_vm);
	_ws->load(fd);

	_polygons = new Polygons(fd);

	getScript()->load(fd);

	fd->close();
	delete fd;

	_vm->resetFlags();
	_ws->field_120 = -1;

	int32 tick = _vm->getTick();
	for (uint32 a = 0; a < _ws->actors.size(); a++)
		_ws->actors[a]->setLastScreenUpdate(tick);

	getCursor()->show();
}

//////////////////////////////////////////////////////////////////////////
// Event handling
//////////////////////////////////////////////////////////////////////////
bool Scene::handleEvent(const AsylumEvent &evt) {
	switch ((uint32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();

	case EVENT_ASYLUM_ACTIVATE:
	case Common::EVENT_RBUTTONUP:
		activate();
		break;

	case EVENT_ASYLUM_UPDATE:
		return update();

	case Common::EVENT_KEYDOWN:
		if (evt.kbd.flags & Common::KBD_CTRL)
			_isCTRLPressed = true;

		return key(evt);

	case Common::EVENT_KEYUP:
		if (!(evt.kbd.flags & Common::KBD_CTRL))
			_isCTRLPressed = false;
		break;

	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_MBUTTONDOWN:
		return clickDown(evt);
	}

	return false;
}

void Scene::activate() {
	Actor *player = getActor();

	if (player->getStatus() == kActorStatus1)
		player->updateStatus(kActorStatusEnabled);

	if (player->getStatus() == kActorStatus12)
		player->updateStatus(kActorStatus14);
}

bool Scene::init() {
	if (getSharedData()->getFlag((kFlag3))) { // this flag is set during an encounter
		getSharedData()->setFlag(kFlag3, false);

		// The original test for flag 1001 but doesn't use the result
		return true;
	}

	getCursor()->set(_ws->cursorResources[kCursorResourceScrollUp], 0, kCursorAnimationNone);
	_ws->coordinates[0] = -1;
	getScreen()->clear();
	getText()->loadFont(_ws->font1);

	ResourceId paletteResource = _ws->actions[getActor()->getActionIndex3()]->paletteResourceId;
	if (!paletteResource)
		paletteResource = _ws->currentPaletteId;

	getScreen()->setPalette(paletteResource);
	getScreen()->setGammaLevel(paletteResource, 0);
	getScreen()->makeGreyPalette();
	getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);
	getScreen()->selectTransTable(1);

	getCursor()->show();

	return true;
}

bool Scene::update() {
	if (getEncounter()->getFlag1()) {
		getEncounter()->setFlag1(false);

		// Enable player
		getActor()->updateStatus(kActorStatusEnabled);
	}

	uint32 ticks = _vm->getTick();

	if (!getSharedData()->getFlag(kFlagRedraw)) {
		if (updateScreen())
			return true;

		getSharedData()->setFlag(kFlagRedraw, true);
	}

	if (ticks > getSharedData()->getNextScreenUpdate()) {
		if (getSharedData()->getFlag(kFlagRedraw)) {
			if (getSharedData()->getMatteBarHeight() <= 0)
				getScreen()->copyBackBufferToScreen();
			else
				error("[Scene::update] Not implemented!");

			// Original also sets an unused value to 0
			getSharedData()->setData(39, getSharedData()->getData(39) ^ 1);

			getSharedData()->setFlag(kFlagRedraw, false);
			getSharedData()->setNextScreenUpdate(ticks + 55);
			++_vm->screenUpdateCount;
		}
	}

	return true;
}

bool Scene::key(const AsylumEvent &evt) {
	// TODO add support for debug commands

	//////////////////////////////////////////////////////////////////////////
	// Check for keyboard shortcuts
	if (evt.kbd.ascii == Config.keyShowVersion) {
		// TODO show version!

		return true;
	}

	if (evt.kbd.ascii == Config.keyQuickLoad) {
		getSaveLoad()->quickLoad();

		return true;
	}

	if (evt.kbd.ascii == Config.keyQuickSave) {
		getSaveLoad()->quickSave();

		return true;
	}

	if (evt.kbd.ascii == Config.keySwitchToSara) {
		if (getCursor()->isHidden() || _ws->chapter != kChapter9)
			return true;

		getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2206)]->scriptIndex, _playerIndex);

		return true;
	}

	if (evt.kbd.ascii == Config.keySwitchToGrimwall) {
		if (getCursor()->isHidden() || _ws->chapter != kChapter9)
			return true;

		getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2207)]->scriptIndex, _playerIndex);

		return true;
	}

	if (evt.kbd.ascii == Config.keySwitchToOlmec) {
		if (getCursor()->isHidden() || _ws->chapter != kChapter9)
			return true;

		getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2208)]->scriptIndex, _playerIndex);

		return true;
	}

	switch (evt.kbd.keycode) {
	default:
		break;

	case Common::KEYCODE_BACKSPACE:
		// TODO add support for debug commands
		warning("[Scene::key] debug command handling not implemented!");
		break;

	case Common::KEYCODE_RETURN:
		// TODO add support for debug commands
		warning("[Scene::key] debug command handling not implemented!");
		break;

	case Common::KEYCODE_ESCAPE:
		// TODO add support for debug commands

		if (getSpeech()->getSoundResourceId()) {
			getScene()->stopSpeech();
		} else {
			if (getCursor()->isHidden())
				break;

			_vm->switchEventHandler(_vm->menu());
		}
		break;

	case Common::KEYCODE_LEFTBRACKET:
		if (evt.kbd.ascii != 123)
			break;
		// Fallback to next case (we got a left brace)

	case Common::KEYCODE_p:
	case Common::KEYCODE_q:
	case Common::KEYCODE_r:
	case Common::KEYCODE_s:
	case Common::KEYCODE_t:
	case Common::KEYCODE_u:
	case Common::KEYCODE_v:
	case Common::KEYCODE_w:
	case Common::KEYCODE_x:
	case Common::KEYCODE_y:
	case Common::KEYCODE_z:
		if (speak(evt.kbd.keycode)) {
			_vm->lastScreenUpdate = _vm->screenUpdateCount;
			getActor()->setLastScreenUpdate(_vm->screenUpdateCount);
		}
		break;

	case Common::KEYCODE_TAB:
		warning("[Scene::key] Screenshot function not implemented!");
		break;
	}

	return true;
}

bool Scene::clickDown(const AsylumEvent &evt) {
	_vm->lastScreenUpdate = 0;

	if (getSharedData()->getFlag(kFlag2)) {
		stopSpeech();

		return true;
	}

	Actor *player = getActor();
	switch (evt.type) {
	default:
		break;

	case Common::EVENT_RBUTTONDOWN:
		if (getSpeech()->getSoundResourceId())
			stopSpeech();

		if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10) {
			player->updateStatus(kActorStatusEnabled);
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
		} else if (player->getStatus() != kActorStatusDisabled) {
			player->updateStatus(kActorStatus1);
		}
		break;

	case Common::EVENT_MBUTTONDOWN:
		if (player->getStatus() != kActorStatusDisabled) {
			if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10)
				player->updateStatus(kActorStatusEnabled);
			else
				player->updateStatus(kActorStatus6);
		}
		break;

	case Common::EVENT_LBUTTONDOWN:
		if (getCursor()->getState() & kCursorStateRight)
			break;

		if (getSpeech()->getSoundResourceId())
			stopSpeech();

		if (player->getStatus() == kActorStatusDisabled)
			break;

		if (player->getField638()) {
			if (hitTestPlayer()) {
				player->setField638(0);
				return true;
			}

			HitType type = kHitNone;
			int32 res = hitTestScene(type);

			if (res == -1)
				getSpeech()->playIndexed(2);
			else
				handleHit(res, type);

			return true;
		}

		if (!hitTestPlayer() || player->getStatus() >= kActorStatus11 || !player->getReaction(0)) {
			if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10) {
				playerReaction();
			} else {
				HitType type = kHitNone;
				int32 res = hitTest(type);
				if (res != -1)
					handleHit(res, type);
			}
			return true;
		}

		if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10) {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
			player->updateStatus(kActorStatusEnabled);
		} else {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 2));
			player->updateStatus(kActorStatus6);
		}
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Scene update
//////////////////////////////////////////////////////////////////////////
bool Scene::updateScreen() {
	// Original has a frame counter (for showing fps)

	if (updateScene())
		return true;

	if (Config.performance <= 4) {
		// TODO when Config.performance <= 4, we need to skip drawing frames to screen

		if (drawScene())
			return true;

	} else {
		if (drawScene())
			return true;
	}

	getActor()->drawNumber();

	// Original handle all debug commands here (we do it as part of each update command)

	if (getSharedData()->getFlag(kFlagScene1)) {
		getScreen()->clear();

		getScreen()->setupPaletteAndStartFade(0, 0, 0);
		updateScene();
		drawScene();
		getScreen()->copyBackBufferToScreen();

		getScreen()->stopFadeAndSetPalette(getWorld()->currentPaletteId, 100, 10);
		drawScene();
		getScreen()->copyBackBufferToScreen();

		getSharedData()->setFlag(kFlagScene1, false);
	}

	if (getSpeech()->getSoundResourceId() != 0) {
		if (getSound()->isPlaying(getSpeech()->getSoundResourceId())) {
			getSpeech()->prepareSpeech();
		} else {
			getSpeech()->resetResourceIds();
			_vm->clearGameFlag(kGameFlag219);
		}
	}

	if (getWorld()->chapter == kChapter5) {
		if (_vm->isGameFlagSet(kGameFlag249))
			drawChapter5();
	}

	return false;
}


bool Scene::updateScene() {
#ifdef DEBUG_SCENE_TIMES
#define MESURE_TICKS(func) { \
	int32 startTick =_vm->getTick(); \
	func(); \
	debugC(kDebugLevelScene, #func " Time: %d", _vm->getTick() - startTick); \
}
#else
#define MESURE_TICKS(func) func();
#endif

	// Update each part of the scene
	if (getSharedData()->getMatteBarHeight() != 170 || getSharedData()->getMattePlaySound()) {
		MESURE_TICKS(updateMouse);
		MESURE_TICKS(updateActors);
		MESURE_TICKS(updateObjects);
		MESURE_TICKS(updateAmbientSounds);
		MESURE_TICKS(updateMusic);
		MESURE_TICKS(updateAdjustScreen);

		// Update Debug
		if (g_debugPolygons)
			debugShowPolygons();
		if (g_debugObjects)
			debugShowObjects();
	}

	return getScript()->process();
}

void Scene::updateMouse() {
	Actor *player = getActor();
	Common::Point mouse = getCursor()->position();

	Common::Point pt;
	player->adjustCoordinates(&pt);

	Common::Rect actorRect;
	if (_ws->chapter != kChapter2 || _playerIndex != 10) {
		actorRect.left   = pt.x + 20;
		actorRect.top    = pt.y;
		actorRect.right  = (int16)(pt.x + 2 * player->getPoint2()->x);
		actorRect.bottom = (int16)(pt.y + player->getPoint2()->y);
	} else {
		actorRect.left   = pt.x + 50;
		actorRect.top    = pt.y + 60;
		actorRect.right  = (int16)(pt.x + getActor(10)->getPoint2()->x + 10);
		actorRect.bottom = (int16)(pt.y + getActor(10)->getPoint2()->y - 20);
	}

	ActorDirection newDirection = kDirectionInvalid;

	if (mouse.x < actorRect.left) {
		if (mouse.y >= actorRect.top) {
			if (mouse.y > actorRect.bottom) {
				if (player->getDirection() == kDirectionO) {
					if ((mouse.y - actorRect.bottom) > 10)
						newDirection = kDirectionSO;
				} else {
					if (player->getDirection() == kDirectionS) {
						if ((actorRect.left - mouse.x) > 10)
							newDirection = kDirectionSO;
					} else {
						newDirection = kDirectionSO;
					}
				}
			} else {
				if (player->getDirection() == kDirectionNO) {
					if ((mouse.y - actorRect.top) > 10)
						newDirection = kDirectionO;
				} else {
					if (player->getDirection() == kDirectionSO) {
						if ((actorRect.bottom - mouse.y) > 10)
							newDirection = kDirectionO;
					} else {
						newDirection = kDirectionO;
					}
				}
			}
		} else {
			if (player->getDirection() != kDirectionN) {
				if (player->getDirection() == kDirectionO) {
					if ((actorRect.top - mouse.y) > 10)
						newDirection = kDirectionNO;
				} else {
					newDirection = kDirectionNO;
				}
			} else {
				if ((actorRect.left - mouse.x) > 10)
					newDirection = kDirectionNO;
			}
		}

	} else if (mouse.x <= actorRect.right) {
		if (mouse.y >= actorRect.top) {
			if (mouse.y > actorRect.bottom) {
				if (player->getDirection() == kDirectionSO) {
					if ((mouse.x - actorRect.left) > 10)
						newDirection = kDirectionS;
				} else {
					if (player->getDirection() == kDirectionSE) {
						if ((actorRect.right - mouse.x) > 10)
							newDirection = kDirectionS;
					} else {
						newDirection = kDirectionS;
					}
				}
			}
		} else {
			if (player->getDirection() == kDirectionNO) {
				if ((mouse.x - actorRect.left) > 10)
					newDirection = kDirectionN;
			} else {
				if (player->getDirection() == kDirectionNE) {
					if ((actorRect.right - mouse.x) > 10)
						newDirection = kDirectionN;
				} else {
					newDirection = kDirectionN;
				}
			}
		}
	} else if (mouse.y < actorRect.top) {
		if (player->getDirection() != kDirectionN) {
			if (player->getDirection() == kDirectionE) {
				if ((actorRect.top - mouse.y) > 10)
					newDirection = kDirectionNE;
			} else {
				newDirection = kDirectionNE;
			}
		} else {
			if ((mouse.x - actorRect.right) > 10)
				newDirection = kDirectionNE;
		}
	} else if (mouse.y <= actorRect.bottom) {
		if (player->getDirection() == kDirectionSE) {
			if ((actorRect.bottom - mouse.y) > 10)
				newDirection = kDirectionE;
		} else {
			if (player->getDirection() == kDirectionNE) {
				if ((mouse.y - actorRect.top) > 10)
					newDirection = kDirectionE;
			} else {
				newDirection = kDirectionE;
			}
		}
	} else if ( player->getDirection() == kDirectionS) {
		if ((mouse.x - actorRect.right) > 10)
			newDirection = kDirectionSE;
	} else if ((player->getDirection() != kDirectionE || (mouse.y - actorRect.bottom) > 10)) {
		newDirection = kDirectionSE;
	}

	updateCursor(newDirection, actorRect);

	if (newDirection >= kDirectionN)
		if (player->getStatus() == kActorStatus1 || player->getStatus() == kActorStatus12)
			player->updateFromDirection(newDirection);
}


void Scene::updateActors() {
	if (!_ws)
		error("[Scene::updateActors] WorldStats not initialized properly!");

	for (uint32 i = 0; i < _ws->actors.size(); i++)
		_ws->actors[i]->update();
}

void Scene::updateObjects() {
	if (!_ws)
		error("[Scene::updateObjects] WorldStats not initialized properly!");

	for (uint32 i = 0; i < _ws->objects.size(); i++)
		_ws->objects[i]->update();
}

void Scene::updateAmbientSounds() {
	if (!_ws)
		error("[Scene::updateAmbientSounds] WorldStats not initialized properly!");

	if (Config.performance <= 3)
		return;

	// The original loops for each actor, but the volume calculation is always the same

	for (int32 i = 0; i < _ws->numAmbientSounds; i++) {
		bool processSound = true;
		AmbientSoundItem *snd = &_ws->ambientSounds[i];
		uint32 *ambientTick = getSharedData()->getAmbientTick(i);

		for (int32 f = 0; f < 6; f++) {
			GameFlag gameFlag = snd->flagNum[f];
			if (gameFlag >= 0) {
				if (_vm->isGameFlagNotSet(gameFlag)) {
					processSound = false;
					break;
				}
			} else {
				if (_vm->isGameFlagSet((GameFlag)-gameFlag)) {
					processSound = false;
					break;
				}
			}
		}
		if (processSound) {
			if (_vm->sound()->isPlaying(snd->resourceId)) {

				if (snd->field_0) {
					int32 volume = Config.ambientVolume + getSound()->calculateVolumeAdjustement(snd->x, snd->y, snd->attenuation, snd->delta);

					if (volume <= 0) {
						if (volume < -10000)
							volume = -10000;

						getSound()->setVolume(snd->resourceId, volume);
					} else {
						getSound()->setVolume(snd->resourceId, 0);
					}
				}

			} else {
				int32 panning = (snd->field_0) ? getSound()->calculatePanningAtPoint(snd->x, snd->y) : 0;

				int32 volume = 0;
				if (snd->field_0)
					volume = getSound()->calculateVolumeAdjustement(snd->x, snd->y, snd->attenuation, snd->delta);
				else
					volume = -pow((double)snd->delta, 2);

				volume += Config.ambientVolume;


				if (LOBYTE(snd->flags) & 1) {

					getSound()->playSound(snd->resourceId, true, volume, panning);

				} else if (LOBYTE(snd->flags) & 2) {
					if (_vm->getRandom(10000) < 10) {
						if (snd->field_0) {
							getSound()->playSound(snd->resourceId, false, volume, panning);
						} else {
							int32 tmpVol = volume + _vm->getRandom(500) * ((((_vm->getRandom(100) >= 50) - 1) & 2) - 1);

							if (tmpVol <= -10000)
								tmpVol = -10000;

							if (tmpVol >= 0)
								tmpVol = 0;
							else if (tmpVol <= -10000)
								tmpVol = -10000;

							getSound()->playSound(snd->resourceId, false, tmpVol, _vm->getRandom(20001) - 10000);
						}
					}
				} else if (LOBYTE(snd->flags) & 4) {
					if (*ambientTick > _vm->getTick()) {
						if (snd->nextTick >= 0)
							*ambientTick = 60000 * snd->nextTick + _vm->getTick();
						else
							*ambientTick = _vm->getTick() - 1000 * snd->nextTick;

						getSound()->playSound(snd->resourceId, false, volume, panning);
					}
				} else if (LOBYTE(snd->flags) & 8) {
					if (_vm->getGameFlagByIndex(85 + i)) {
						getSound()->playSound(snd->resourceId, false, volume, panning);
						_vm->setGameFlagByIndex(85 + i);
					}

				}
			}
		} else {
			if (_vm->sound()->isPlaying(snd->resourceId))
				_vm->sound()->stop(snd->resourceId);
		}
	}
}

void Scene::updateMusic() {
	//warning("[Scene::updateMusic] not implemented!");
}

void Scene::updateAdjustScreen() {
	if (g_debugScrolling) {
		debugScreenScrolling();
	} else {
		updateCoordinates();
	}
}

void Scene::updateCoordinates() {
	Actor *act = getActor();
	int32 newXLeft = -1;
	int32 newYTop  = -1;
	Common::Rect b = _ws->boundingRect;

	if (_ws->motionStatus == 1) {
		int32 posX = act->getPoint1()->x - _ws->xLeft;
		int32 posY = act->getPoint1()->y - _ws->yTop;

		if (posX < b.left || posX > b.right) {
			int32 newRBounds = posX - b.right;
			newXLeft = newRBounds + _ws->xLeft;
			_ws->xLeft += newRBounds;
		}

		if (posY < b.top || posY > b.bottom) {
			int32 newBBounds = posY - b.bottom;
			newYTop = newBBounds + _ws->yTop;
			_ws->yTop += newBBounds;
		}

		if (newXLeft < 0)
			newXLeft = _ws->xLeft = 0;

		if (newXLeft > _ws->width - 640)
			newXLeft = _ws->xLeft = _ws->width - 640;

		if (newYTop < 0)
			newYTop = _ws->yTop = 0;

		if (newYTop > _ws->height - 480)
			newYTop = _ws->yTop = _ws->height - 480;
	} else {
		// TODO
	}

	uint8 rectIndex = _ws->sceneRectIdx;
	b = _ws->sceneRects[rectIndex];

	if (newXLeft < b.left)
		newXLeft = _ws->xLeft = b.left;

	if (newYTop < b.top)
		newYTop = _ws->yTop = b.top;

	if (newXLeft + 639 > b.right)
		newXLeft = _ws->xLeft = b.right - 639;

	if (newYTop + 479 > b.bottom)
		newYTop = _ws->yTop = b.bottom - 479;
}

void Scene::updateCursor(ActorDirection direction, Common::Rect rect) {
	HitType type = kHitNone;
	Actor *player = getActor();
	int16 rightLimit = rect.right - 10;
	Common::Point mouse = getCursor()->position();

	if (getEncounter()->getFlag6()) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceTalkNPC])
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC]);

		return;
	}

	if (getCursor()->getState() & kCursorStateRight) {
		if (player->getStatus() == kActorStatus1 || player->getStatus() == kActorStatus12) {

			ResourceId resourceId =_ws->cursorResources[direction];

			if (direction >= kDirectionN && getCursor()->graphicResourceId != resourceId)
				getCursor()->set(resourceId);
		}

		return;
	}

	if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceHand])
			getCursor()->set(_ws->cursorResources[kCursorResourceHand]);

		return;
	}

	if (player->getField638()) {
		if (mouse.x >= rect.left && mouse.x <= rightLimit && mouse.y >= rect.top  && mouse.y <= rect.bottom && hitTestPlayer()) {

			ResourceId id = _ws->cursorResourcesAlternate[player->getField638() + 31];
			if (getCursor()->graphicResourceId != id)
				getCursor()->set(id, 0, kCursorAnimationNone);

		} else {
			if (hitTestScene(type) == -1) {
				ResourceId id = _ws->cursorResourcesAlternate[player->getField638() + 31];
				if (getCursor()->graphicResourceId != id)
					getCursor()->set(id, 0, kCursorAnimationNone);
			} else {
				ResourceId id = _ws->cursorResourcesAlternate[player->getField638() + 47];
				uint32 frameCount = GraphicResource::getFrameCount(_vm, id);
				if (getCursor()->graphicResourceId != id)
					getCursor()->set(id, 0, (CursorAnimation)(((frameCount <= 1) - 1) & 2));
			}
		}

		return;
	}

	if (mouse.x >= rect.left && mouse.x <= rightLimit && mouse.y >= rect.top  && mouse.y <= rect.bottom && hitTestPlayer()) {
		if (player->getReaction(0)) {
			if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceGrabPointer])
				getCursor()->set(_ws->cursorResources[kCursorResourceGrabPointer]);

			return;
		}
	}

	int32 index = hitTest(type);
	if (index == -1) {
		if (_ws->chapter != kChapter2 || _playerIndex != 10) {
			if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->animation)
				getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
		} else {
			if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->animation)
				getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC2]);
		}

		return;
	}

	int32 actionType = 0;
	switch (type) {
	default:
		error("[Scene::updateCursor] Invalid hit type!");
		break;

	case kHitActionArea:
		actionType = _ws->actions[index]->actionType;
		break;

	case kHitObject:
		actionType = _ws->objects[index]->actionType;
		break;

	case kHitActor:
		actionType = getActor(index)->actionType;
		break;
	}

	if (actionType & kActionTypeFind) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->animation != kCursorAnimationMirror)
			getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
	} else if (actionType & kActionTypeTalk) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceTalkNPC])
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC]);
	} else if (actionType & kActionTypeGrab) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceHand])
			getCursor()->set(_ws->cursorResources[kCursorResourceHand]);
	} else if (actionType & kActionType16) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->animation != kCursorAnimationMirror)
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC2]);
	} else if (_ws->chapter != kChapter2 && _playerIndex != 10) {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->animation)
			getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
	} else {
		if (getCursor()->graphicResourceId != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->animation)
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC2]);
	}
}

//////////////////////////////////////////////////////////////////////////
// HitTest
//////////////////////////////////////////////////////////////////////////
int32 Scene::hitTest(HitType &type) {
	type = kHitNone;

	int32 targetIdx = hitTestObject();
	if (targetIdx == -1) {

		targetIdx = hitTestActionArea();
		if (targetIdx == -1) {

			targetIdx = hitTestActor();
			type = kHitActor;

		} else {
			type = kHitActionArea;
		}
	} else {
		type = kHitObject;
	}

	return targetIdx;
}

int32 Scene::hitTestScene(HitType &type) {
	if (!_ws)
		error("[Scene::hitTestScene] WorldStats not initialized properly!");

	const Common::Point pt = getCursor()->position();

	int32 top  = pt.x + _ws->xLeft;
	int32 left = pt.y + _ws->yTop;
	type = kHitNone;

	int32 index = findActionArea(kActionAreaType2, Common::Point(top, left));
	if (index != -1) {
		if (_ws->actions[index]->actionType & kActionType8) {
			type = kHitActionArea;
			return index;
		}

		index = -1;
	}

	// Check objects
	for (uint i = 0; i < _ws->objects.size(); i++) {
		Object *object = _ws->objects[i];

		if (object->isOnScreen() && object->actionType & kActionType8) {
			if (hitTestPixel(object->getResourceId(),
			                 object->getFrameIndex(),
							 top - object->x,
							 left - object->y,
			                 object->flags & kObjectFlag1000)) {
				type = kHitObject;
				return i;
			}
		}
	}

	// Check actors
	for (uint i = 0; i < _ws->actors.size(); i++) {
		Actor *actor = _ws->actors[i];

		if (actor->actionType & kActionType8) {
			uint32 frameIndex = (actor->getFrameIndex() >= actor->getFrameCount() ? 2 * actor->getFrameCount() - (actor->getFrameIndex() + 1) : actor->getFrameIndex());

			if (hitTestPixel(actor->getResourceId(),
				             frameIndex,
				             top - actor->getPoint()->x - actor->getPoint1()->x,
							 left - actor->getPoint()->y - actor->getPoint1()->y,
				             actor->getDirection() >= kDirectionSE)) {
				type = kHitActor;
				return i;
			}
		}
	}

	return -1;
}

int32 Scene::hitTestActionArea() {
	const Common::Point pt = getCursor()->position();

	int32 targetIdx = findActionArea(kActionAreaType2, Common::Point(_ws->xLeft + pt.x, _ws->yTop + pt.y));

	if ( targetIdx == -1 || !(_ws->actions[targetIdx]->actionType & (kActionTypeFind | kActionTypeTalk | kActionTypeGrab | kActionType16)))
		return -1;

	return targetIdx;
}

ActorIndex Scene::hitTestActor() {
	const Common::Point mouse = getCursor()->position();

	if (_ws->actors.size() == 0)
		return -1;

	// Check actors 13 to 20
	if (_ws->actors.size() >= 20) {
		for (uint i = 13; i < 21; i++) {
			Actor *actor = getActor(i);

			if (!actor->isOnScreen() || !actor->actionType)
				continue;

			Common::Rect rect = GraphicResource::getFrameRect(_vm, getActor(12)->getResourceId(), 0);

			int32 x = _ws->xLeft + mouse.x - (actor->getPoint1()->x + actor->getPoint()->x);
			int32 y = _ws->yTop  + mouse.y - (actor->getPoint1()->y + actor->getPoint()->y);

			if (x > (rect.left - 20)
				&& x < (rect.width() + rect.left + 20)
				&& y > (rect.top - 20)
				&& y < (rect.height() + rect.top + 20))
				return i;
		}
	}

	// Check Actor 11
	if (_ws->actors.size() >= 11) {
		Actor *actor11 = getActor(11);
		if (actor11->isOnScreen() && actor11->actionType) {
			int x = mouse.x + _ws->xLeft - actor11->getPoint1()->x;
			int y = mouse.y + _ws->yTop  - actor11->getPoint1()->y;

			if (actor11->getBoundingRect()->contains(x, y))
				return 11;
		}
	}

	switch (_ws->chapter) {
	default:
		break;

	case kChapter8:
		if (_ws->actors.size() <= 7)
			error("[Scene::hitTestActor] Not enough actors to check (chapter 8 - checking actors 1-7)!");

		for (uint i = 1; i < 7; i++) {
			Actor *actor = getActor(i);

			if (!actor->isVisible() || !actor->actionType)
				continue;

			int32 x = _ws->xLeft + mouse.x - (actor->getPoint1()->x + actor->getPoint()->x);
			int32 y = _ws->yTop  + mouse.y - (actor->getPoint1()->y + actor->getPoint()->y);

			if (x > 300 && x < 340 && y > 220 && y < 260)
				return i;
		}
		break;

	case kChapter11:
		if (_ws->actors.size() <= 1)
			error("[Scene::hitTestActor] Not enough actors to check (chapter 11 - checking actor 1)!");

		if (getActor(1)->isOnScreen() && getActor(1)->actionType) {
			Actor *actor = getActor(1);

			int32 x = _ws->xLeft + mouse.x - (actor->getPoint1()->x + actor->getPoint()->x);
			int32 y = _ws->yTop  + mouse.y - (actor->getPoint1()->y + actor->getPoint()->y);

			Common::Rect rect = GraphicResource::getFrameRect(_vm, actor->getResourceId(), 0);

			if (x > (rect.left - 10)
			 && x < (rect.width() + rect.left + 10)
			 && y > (rect.top - 10)
			 && y < (rect.height() + rect.top + 10))
				return 1;
		}

		if (_ws->actors.size() <= 15)
			error("[Scene::hitTestActor] Not enough actors to check (chapter 11 - checking actors 10-15)!");

		for (uint i = 10; i < 15; i++) {
			Actor *actor = getActor(i);

			if (!actor->isOnScreen() || !actor->actionType)
				continue;

			Common::Rect rect = GraphicResource::getFrameRect(_vm, actor->getResourceId(), 0);

			int32 x = _ws->xLeft + mouse.x - (actor->getPoint1()->x + actor->getPoint()->x);
			int32 y = _ws->yTop  + mouse.y - (actor->getPoint1()->y + actor->getPoint()->y);

			if (x > (rect.left - 10)
			 && x < (rect.width() + rect.left + 10)
			 && y > (rect.top - 10)
			 && y < (rect.height() + rect.top + 10))
				return i;
		}
		break;
	}

	//////////////////////////////////////////////////////////////////////////
	// Default check
	for (int i = _ws->actors.size() - 1; i >= 0 ; i--) {
	Actor *actor = getActor(i);

	int32 hitFrame;
	if (actor->getFrameIndex() >= actor->getFrameCount())
		hitFrame = 2 * actor->getFrameIndex() - actor->getFrameCount() - 1;
	else
		hitFrame = actor->getFrameIndex();

	if (hitTestPixel(actor->getResourceId(),
					 hitFrame,
					 _ws->xLeft - actor->getPoint()->x - actor->getPoint1()->x,
					 _ws->yTop  - actor->getPoint()->y - actor->getPoint1()->y,
					 actor->getDirection() >= kDirectionSE))
		return i;
	}

	return -1;
}

bool Scene::hitTestPlayer() {
	const Common::Point pt = getCursor()->position();

	Actor *player = getActor();
	Common::Point point;

	player->adjustCoordinates(&point);

	uint32 frameIndex = (player->getFrameIndex() >= player->getFrameCount() ? 2 * player->getFrameCount() - (player->getFrameIndex() + 1) : player->getFrameIndex());

	return hitTestPixel(player->getResourceId(),
	                    frameIndex,
	                    pt.x - player->getPoint()->x - point.x,
	                    pt.y - player->getPoint()->y - point.y,
	                    player->getDirection() >= kDirectionSE);
}

int32 Scene::hitTestObject() {
	if (!_ws)
		error("[Scene::hitTestObject] WorldStats not initialized properly!");

	const Common::Point pt = getCursor()->position();

	for (uint32 i = 0; i < _ws->objects.size(); i++) {
		Object *object = _ws->objects[i];
		if (object->isOnScreen() && object->actionType)
			if (hitTestPixel(object->getResourceId(),
			                 object->getFrameIndex(),
			                 _ws->xLeft + pt.x - object->x,
			                 _ws->yTop + pt.y - object->y,
			                 object->flags & kObjectFlag1000))
				return i;
	}

	return -1;
}

bool Scene::hitTestPixel(ResourceId resourceId, int32 frame, int16 x, int16 y, bool flipped) {
	Common::Rect rect = GraphicResource::getFrameRect(_vm, resourceId, frame);

	// TODO we need to test each pixel of the surface!

	return rect.contains(x, y);
}

//////////////////////////////////////////////////////////////////////////
// Hit actions
//////////////////////////////////////////////////////////////////////////
void Scene::handleHit(int32 index, HitType type) {
	switch (type) {
	default:
		break;

	case kHitActionArea:
		if (!getScript()->isInQueue(_ws->actions[index]->scriptIndex))
			getScript()->queueScript(_ws->actions[index]->scriptIndex, _playerIndex);

		switch (_ws->chapter) {
		default:
			break;

		case kChapter2:
			hitAreaChapter2(_ws->actions[index]->id);
			break;

		case kChapter7:
			hitAreaChapter7(_ws->actions[index]->id);
			break;

		case kChapter11:
			hitAreaChapter11(_ws->actions[index]->id);
			break;
		}
		break;

	case kHitObject: {
		Object *object = _ws->objects[index];

		if (object->getSoundResourceId()) {
			if (getSound()->isPlaying(object->getSoundResourceId())) {
				getSound()->stop(object->getSoundResourceId());
				object->setSoundResourceId(kResourceNone);
			}
		}

		if (!getScript()->isInQueue(object->getScriptIndex()))
			getScript()->queueScript(object->getScriptIndex(), _playerIndex);

		// Original executes special script hit functions, but since there is none defined, we can skip this part
		}
		break;

	case kHitActor: {
		Actor *actor = _ws->actors[index];

		if (actor->actionType & (kActionTypeFind | kActionType16)) {

			if (getScript()->isInQueue(actor->getScriptIndex()))
				getScript()->queueScript(actor->getScriptIndex(), _playerIndex);

		} else if (actor->actionType & kActionTypeTalk) {

			if (getSound()->isPlaying(actor->getSoundResourceId())) {
				if (actor->getStatus() != kActorStatusEnabled)
					actor->updateStatus(kActorStatusEnabled);

				getSound()->stop(actor->getSoundResourceId());
				actor->setSoundResourceId(kResourceNone);
			}

			if (getScript()->isInQueue(actor->getScriptIndex()))
				getScript()->queueScript(actor->getScriptIndex(), _playerIndex);
		}

		switch (_ws->chapter) {
		default:
			break;

		case kChapter2:
			hitActorChapter2(index);
			break;

		case kChapter11:
			hitActorChapter11(index);
			break;
		}
		}
		break;
	}
}

void Scene::playerReaction() {
	const Common::Point mouse = getCursor()->position();
	Common::Point point;
	Actor *player = getActor();

	player->adjustCoordinates(&point);

	uint32 maxIndex = 0;
	for (maxIndex = 0; maxIndex < 8; maxIndex++) {
		if (!player->getReaction(maxIndex))
			break;
	}

	maxIndex -= 1;

	player->setField638(0);

	if (maxIndex > 0) {
		for (uint32 i = 0; i < maxIndex; i++) {
			Common::Point ret = _vm->getSinCosValues(maxIndex, i);
			int32 x = point.x + player->getPoint2()->x + ret.x;
			int32 y = point.y + player->getPoint2()->y / 2 - ret.y;

			if (mouse.x >= x && mouse.x <= (x + 40) && mouse.y >= y && mouse.y <= (y + 40)) {
				// Handle reaction
				getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 4));

				if (_ws->chapter == kChapter9) {
					switch (i) {
					default:
						player->setField638(player->getReaction(i));
						break;

					case 0:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2206)]->scriptIndex, _playerIndex);
						break;

					case 1:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2207)]->scriptIndex, _playerIndex);
						break;

					case 2:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2208)]->scriptIndex, _playerIndex);
						break;
					}
				} else {
					player->setField638(player->getReaction(i));
				}
				break;
			}
		}
	}

	player->updateStatus(kActorStatusEnabled);
	getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
}

void Scene::hitAreaChapter2(int32 id) {
	if (id == 783)
		getActor()->setField638(6);
}

void Scene::hitAreaChapter7(int32 id) {
	switch (id) {
	default:
		break;

	case 1088:
		if (_isCTRLPressed)
			_vm->setGameFlag(kGameFlag1144);
		break;

	case 2504:
		if (++_hitAreaChapter7Counter > 20) {
			_vm->setGameFlag(kGameFlag1108);
			getActor(1)->setPosition(570, 225, kDirectionN, 0);
			getActor(1)->show();
		}
		break;
	}
}

void Scene::hitAreaChapter11(int32 id) {
	if (id == 1670)
		_ws->field_E849C = 666;
}


void Scene::hitActorChapter2(ActorIndex index) {
	Actor *player = getActor();

	if (player->getStatus() != kActorStatus14 && player->getStatus() != kActorStatus12)
		return;

	if (index == 11) {
		player->faceTarget(index + 9, kDirectionFromActor);
		player->updateStatus(kActorStatus15);

		Actor *actor11 = getActor(index);

		Common::Point pointPlayer(player->getPoint1()->x + player->getPoint2()->x, player->getPoint1()->y + player->getPoint2()->y);
		Common::Point pointActor11(actor11->getPoint1()->x + actor11->getPoint2()->x, actor11->getPoint1()->y + actor11->getPoint2()->y);

		if (Actor::distance(pointPlayer, pointActor11) < 150) {
			if (actor11->getStatus() == kActorStatus12)
				actor11->updateStatus(kActorStatus18);

			if (actor11->getStatus() == kActorStatusEnabled)
				actor11->updateStatus(kActorStatus14);
		}

		getSharedData()->setData(38, index);

	} else if (index > 12) {
		player->faceTarget(index + 9, kDirectionFromActor);
		player->updateStatus(kActorStatus15);
		getSharedData()->setData(38, index);
	}
}

void Scene::hitActorChapter11(ActorIndex index) {
	if (_ws->field_E848C < 3)
		_ws->field_E849C = index;
}


//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
void Scene::playIntroSpeech() {
	ResourceId resourceId;

	switch (_packId) {
	default:
		resourceId = (ResourceId)_packId;
		break;

	case kResourcePackCourtyardAndChapel:
		resourceId = getSpeech()->playScene(4, 3);
		break;

	case kResourcePackCave:
		resourceId = getSpeech()->playScene(4, 6);
		break;

	case kResourcePackLaboratory:
		resourceId = getSpeech()->playScene(4, 7);
		break;
	}

	getScreen()->clear();
	getScreen()->setupPaletteAndStartFade(0, 0, 0);

	do {
		// Poll events (this ensure we don't freeze the screen)
		Common::Event ev;
		_vm->getEventManager()->pollEvent(ev);

	} while (getSound()->isPlaying(resourceId));
}

void Scene::stopSpeech() {
	if (_vm->isGameFlagSet(kGameFlag219)) {
		if (getSpeech()->getSoundResourceId() != kResourceNone && getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			getSound()->stopAll(getSpeech()->getSoundResourceId());
		else if (getSpeech()->getTick())
			getSpeech()->setTick(_vm->getTick());
	}
}

bool Scene::speak(Common::KeyCode code) {
#define GET_INDEX(val) ((((long long)val >> 32) ^ abs((int)val) & 1) == ((long long)val >> 32))

	int32 index = -1;

	switch (code) {
	default:
		break;

	case Common::KEYCODE_p:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = GET_INDEX(_vm->getRandom(RAND_MAX));
			break;

		case 1:
		case 2:
		case 3:
			index = 1;
			break;
		}
		break;

	case Common::KEYCODE_q:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 3 - GET_INDEX(_vm->getRandom(RAND_MAX));
			break;

		case 1:
		case 2:
		case 3:
			index = 2;
			break;
		}
		break;

	case Common::KEYCODE_r:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 2;
			break;

		case 1:
		case 2:
		case 3:
			index = 4;
			break;
		}
		break;

	case Common::KEYCODE_s:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 5;
			break;

		case 1:
		case 2:
		case 3:
			index = 3;
			break;
		}
		break;

	case Common::KEYCODE_t:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 6;
			break;

		case 1:
		case 2:
		case 3:
			index = 4;
			break;
		}
		break;

	case Common::KEYCODE_u:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 7;
			break;

		case 1:
		case 2:
			index = 5;
			break;
		}
		break;

	case Common::KEYCODE_v:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 8;
			break;

		case 1:
		case 2:
			index = 6;
			break;
		}
		break;

	case Common::KEYCODE_w:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 9;
			break;

		case 1:
		case 2:
			index = 7;
			break;
		}
		break;

	case Common::KEYCODE_x:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 10;
			break;

		case 1:
		case 2:
			index = 8;
			break;
		}
		break;

	case Common::KEYCODE_y:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 11;
			break;

		case 1:
		case 2:
			index = 9;
			break;
		}
		break;

	case Common::KEYCODE_z:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 13 - GET_INDEX(_vm->getRandom(65536));
			break;

		case 1:
		case 2:
			index = 10;
			break;
		}
		break;

	case Common::KEYCODE_LEFTBRACKET:
		switch (_ws->actorType) {
		default:
			break;

		case 0:
			index = 15 - GET_INDEX(_vm->getRandom(RAND_MAX));
			break;

		case 2:
			index = 12 - GET_INDEX(_vm->getRandom(RAND_MAX));
			break;
		}
		break;
	}

	if (getSpeech()->getSoundResourceId() && getSound()->isPlaying(getSpeech()->getSoundResourceId()))
		return false;

	if (index == -1)
		return false;

	getSpeech()->playPlayer(index);

	return true;

#undef GET_INDEX
}

bool Scene::pointIntersectsRect(Common::Point point, Common::Rect rect) {
	if (rect.top || rect.left || rect.bottom || rect.right) {
		Common::Rational res((rect.bottom - rect.top) * (point.x - rect.left), rect.right - rect.left);

		return (bool)(point.y > rect.top ? 1 + res.toInt() : res.toInt());
	}

	return true;
}

bool Scene::rectIntersect(int32 x, int32 y, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3) {
	return (x >= x3 && y >= y3 && x1 >= x2 && y1 >= y2);
}

void Scene::adjustCoordinates(Common::Point *point) {
	point->x = _ws->xLeft + getCursor()->position().x;
	point->y = _ws->yTop  + getCursor()->position().y;
}

Actor* Scene::getActor(ActorIndex index) {
	if (!_ws)
		error("[Scene::getActor] WorldStats not initialized properly!");

	ActorIndex computedIndex =  (index != -1) ? index : _playerIndex;

	if (computedIndex < 0 || computedIndex >= (int16)_ws->actors.size())
		error("[Scene::getActor] Invalid actor index: %d ([0-%d] allowed)", computedIndex, _ws->actors.size() - 1);

	return _ws->actors[computedIndex];
}

bool Scene::updateSceneCoordinates(int32 tX, int32 tY, int32 A0, bool checkSceneCoords, int32 *param) {
	if (!_ws)
		error("[Scene::updateSceneCoordinates] WorldStats not initialized properly!");

	Common::Rect *sr = &_ws->sceneRects[_ws->sceneRectIdx];

	int32 *targetX = &_ws->coordinates[0];
	int32 *targetY = &_ws->coordinates[1];
	int32 *coord3  = &_ws->coordinates[2];

	*targetX = tX;
	*targetY = tY;

	*coord3 = A0;

	// Adjust coordinates
	if (checkSceneCoords)
		if (*targetX + 640 > _ws->width)
			*targetX = _ws->width - 640;

	if (*targetX < sr->left)
		*targetX = sr->left;

	if (*targetY < sr->top)
		*targetY = sr->top;

	if (*targetX + 640 > sr->right)
		*targetX = sr->right - 640;

	if (*targetY + 480 < sr->bottom)
		*targetY = sr->bottom - 480;

	if (checkSceneCoords)
		if (*targetY + 480 > _ws->height)
			*targetY = _ws->height - 480;

	// Adjust scene offsets & coordinates
	getSharedData()->setSceneOffset(0);
	getSharedData()->setSceneXLeft(_ws->xLeft);
	getSharedData()->setSceneYTop(_ws->yTop);

	int32 diffX = *targetX - _ws->xLeft;
	int32 diffY = *targetY - _ws->yTop;

	if (abs(diffX) <= abs(diffY)) {
		if (_ws->yTop > *targetY)
			*coord3 = -*coord3;

		getSharedData()->setSceneOffsetAdd(Common::Rational(*coord3, diffY) * diffX);

		if (param != NULL && abs(diffY) <= abs(*coord3)) {
			*targetX = -1;
			*param = 0;
			return true;
		}
	} else {
		if (_ws->xLeft > *targetX)
			*coord3 = -*coord3;

		getSharedData()->setSceneOffsetAdd(Common::Rational(*coord3, diffX) * diffY);

		if (param != NULL && abs(diffX) <= abs(*coord3)) {
			*targetX = -1;
			return true;
		}
	}

	return false;
}


int32 Scene::findActionArea(ActionAreaType type, const Common::Point pt) {
	if (!_ws)
		error("[Scene::findActionArea] WorldStats not initialized properly!");

	if (!_polygons)
		error("[Scene::findActionArea] Polygons not initialized properly!");

	switch (type) {
	default:
		return type - 2;

	case kActionAreaType1:
		if (_ws->actions.size() < 1)
			return -1;

		for (int32 i = _ws->actions.size() - 1; i >= 0; i--) {
			ActionArea *area = _ws->actions[i];

			bool found = false;

			// Iterate over flagNum
			for (uint32 j = 0; j < 10; j++) {
				if (!area->flagNums[j])
					break;                 // We stop as soon as a flag is 0

				bool flagSet = false;
				if (area->flagNums[j] <= 0)
					flagSet = _vm->isGameFlagNotSet((GameFlag)-area->flagNums[j]);
				else
					flagSet = _vm->isGameFlagNotSet((GameFlag)area->flagNums[j]);

				if (!flagSet) {
					found = true;
					break;
				}
			}

			if (!found && _polygons->entries[area->polygonIndex].contains(pt))
				return i;
		}
		break;

	case kActionAreaType2:
		if (_ws->actions.size() < 1)
			return -1;

		for (int32 i = _ws->actions.size() - 1; i >= 0; i--) {
			ActionArea *area = _ws->actions[i];

			bool found = false;

			// Iterate over flagNum
			for (uint32 j = 0; j < 10; j++) {
				if (!area->flagNums[j])
					continue;                 // We skip over null flags

				bool flagSet = false;
				if (area->flagNums[j] <= 0)
					flagSet = _vm->isGameFlagNotSet((GameFlag)-area->flagNums[j]);
				else
					flagSet = _vm->isGameFlagNotSet((GameFlag)area->flagNums[j]);

				if (!flagSet) {
					found = true;
					break;
				}
			}

			if (!found && _polygons->entries[area->polygonIndex].contains(pt))
				return i;
		}
		break;
	}

	return -1;
}

void Scene::changePlayer(ActorIndex index) {
	error("[Scene::changePlayer] not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Scene drawing
//////////////////////////////////////////////////////////////////////////
void Scene::preload() {
	if (!Config.showSceneLoading)
		return;

	SceneTitle *title = new SceneTitle(_vm);
	title->load();

	do {
		title->update(_vm->getTick());

		getScreen()->copyBackBufferToScreen();
		g_system->updateScreen();

		g_system->delayMillis(10);

		// Poll events (this ensure we don't freeze the screen)
		Common::Event ev;
		_vm->getEventManager()->pollEvent(ev);

	} while (!title->loadingComplete());

	delete title;
}

bool Scene::drawScene() {
	if (!_ws)
		error("[Scene::drawScene] WorldStats not initialized properly!");

	_vm->screen()->clearGraphicsInQueue();

	if (getSharedData()->getFlag(kFlagRedraw)) {
		_vm->screen()->clear();
		return false;
	}

	// Draw scene background
	getScreen()->draw(_ws->backgroundImage, 0, -_ws->xLeft, -_ws->yTop, 0, false);

	// Draw actors on the update list
	buildUpdateList();
	processUpdateList();

	if (_ws->chapter == kChapter11)
		checkVisibleActorsPriority();

	// Queue updates
	for (uint32 i = 0; i < _ws->actors.size(); i++)
		_ws->actors[i]->draw();

	for (uint32 i = 0; i < _ws->objects.size(); i++)
		_ws->objects[i]->draw();

	Actor *player = getActor();
	if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10)
		player->updateAndDraw();
	else
		player->setNumberFlag01(0);

	_vm->screen()->drawGraphicsInQueue();

	return false;
}

bool Scene::updateListCompare(const UpdateItem &item1, const UpdateItem &item2) {
	return (item1.priority - item2.priority < 0) ? false : true;
}

void Scene::buildUpdateList() {
	if (!_ws)
		error("[Scene::buildUpdateList] WorldStats not initialized properly!");

	_updateList.clear();

	for (uint32 i = 0; i < _ws->actors.size(); i++) {
		Actor *actor = _ws->actors[i];

		if (actor->isVisible()) {
			UpdateItem item;
			item.index = i;
			item.priority = actor->getPoint1()->y + actor->getPoint2()->y;

			_updateList.push_back(item);
		}
	}

	// Sort the list (the original uses qsort, so we may have to revert to that if our sort isn't behaving the same)
	Common::sort(_updateList.begin(), _updateList.end(), &Scene::updateListCompare);
}

void Scene::processUpdateList() {
	if (!_ws)
		error("[Scene::processUpdateList] WorldStats not initialized properly!");

	for (uint32 i = 0; i < _updateList.size(); i++) {
		Actor *actor = getActor(_updateList[i].index);
		int32 priority = _updateList[i].priority;
		Common::Point point;

		// Check priority
		if (priority < 0) {
			actor->setPriority(abs(priority));
			continue;
		}

		actor->setPriority(3);

		if (actor->getField944() == 1 || actor->getField944() == 4) {
			actor->setPriority(1);
		} else {
			actor->setField938(1);
			actor->setField934(0);
			point.x = actor->getPoint1()->x + actor->getPoint2()->x;
			point.y = actor->getPoint1()->y + actor->getPoint2()->y;

			int32 bottomRight = actor->getBoundingRect()->bottom + actor->getPoint1()->y + 4;

			if (_ws->chapter == kChapter11 && _updateList[i].index != getPlayerIndex())
				bottomRight += 20;

			// Our actor rect
			Common::Rect actorRect(actor->getPoint1()->x, actor->getPoint1()->y, actor->getPoint1()->x + actor->getBoundingRect()->right, bottomRight);

			// Process objects
			for (uint32 j = 0; j < _ws->objects.size(); j++) {
				Object *object = _ws->objects[i];

				// Skip hidden objects
				if (!object->isOnScreen())
					continue;

				// Rect for the object
				Common::Rect objectRect(object->x, object->y, object->x + object->getBoundingRect()->right, object->y + object->getBoundingRect()->bottom);

				// Check that the rects are contained
				if (!objectRect.contains(actorRect)) {
					if (BYTE1(object->flags) & kObjectFlag20)
						if (!(BYTE1(object->flags) & kObjectFlag80))
							object->flags = BYTE1(object->flags) | kObjectFlag40;
					continue;
				}

				// Check if it intersects with either the object rect or the related polygon
				bool intersects = false;
				if (object->flags & kObjectFlag2) {
					intersects = pointIntersectsRect(point, *object->getRect());
				} else {
					if (object->flags & kObjectFlag40) {
						PolyDefinitions *poly = &_polygons->entries[object->getPolygonIndex()];
						if (point.x > 0 && point.y > 0 && poly->count() > 0)
							intersects = poly->contains(point);
						else
							warning ("[drawActorsAndObjects] trying to find intersection of uninitialized point");
					}
				}

				// Adjust object flags
				if (BYTE1(object->flags) & kObjectFlag80 || intersects) {
					if (BYTE1(object->flags) & kObjectFlag20)
						object->flags = (BYTE1(object->flags) & 0xBF) | kObjectFlag80;
				} else {
					if (BYTE1(object->flags) & kObjectFlag20) {
						object->flags = BYTE1(object->flags) | kObjectFlag40;
					}
				}

				if (object->flags & kObjectFlag4) {
					if (intersects && LOBYTE(actor->flags) & kActorFlagMasked) {
						error("[Scene::processUpdateList] Assigning mask to masked character [%s]", actor->getName());
					} else {
						object->adjustCoordinates(&point);
						actor->setObjectIndex(j);
						actor->flags |= kActorFlagMasked;
					}
				} else {
					if (intersects) {
						if (actor->getPriority() < object->getPriority()) {
							actor->setField934(1);
							actor->setPriority(object->getPriority() + 3);

							if (_updateList[i].index > _updateList[0].index) {
								error("[Scene::processUpdateList] list update not implemented!");
							}
						}
					} else {
						if (actor->getPriority() > object->getPriority() || actor->getPriority() == 1) {
							actor->setField934(1);
							actor->setPriority(object->getPriority() - 1);

							if (_updateList[i].index > _updateList[0].index) {
								error("[Scene::processUpdateList] list update not implemented!");
							}
						}
					}
				}
			} // end processing objects

			// Update all other actors
			for (uint32 k = 0; k < _updateList.size(); k++) {
				Actor *actor2 = getActor(_updateList[k].index);

				if (actor2->isVisible() && actor2->getField944() != 1 && actor2->getField944() != 4 && _updateList[k].index != _updateList[i].index) {

					Common::Rect actor2Rect(actor2->getPoint1()->x, actor2->getPoint1()->y, actor2->getPoint1()->x + actor2->getBoundingRect()->right, actor2->getPoint1()->y + actor2->getBoundingRect()->bottom);

					if (actor2Rect.contains(actorRect)) {

						// Inferior
						if ((actor2->getPoint1()->y + actor2->getPoint2()->y) > (actor->getPoint1()->y + actor->getPoint2()->y)) {
							if (actor->getPriority() <= actor2->getPriority()) {
								if (actor->getField934() || actor2->getNumberValue01()) {
									if (!actor2->getNumberValue01())
										actor2->setPriority(actor->getPriority() - 1);
								} else {
									actor->setPriority(actor2->getPriority() + 1);
								}
							}
						}

						// Superior
						if ((actor2->getPoint1()->y + actor2->getPoint2()->y) < (actor->getPoint1()->y + actor->getPoint2()->y)) {
							if (actor->getPriority() >= actor2->getPriority()) {
								if (actor->getField934() || actor2->getNumberValue01()) {
									if (!actor2->getNumberValue01())
										actor2->setPriority(actor->getPriority() + 1);
								} else {
									actor->setPriority(actor2->getPriority() - 1);
								}
							}
						}
					}
				}
			}

			if (actor->getField974())
				getActor(actor->getField980())->setPriority(-actor->getPriority());
		}
	} // end processing actors


	// Go through the list from the end
	if (_updateList.size() > 1) {
		for (int i = _ws->actors.size() - 1; i >= 0; --i) {
			Actor *actor = _ws->actors[i];

			// Skip hidden actors
			if (!actor->isVisible())
				continue;

			if (actor->getField944() != 1 && actor->getField944() != 4) {
				error("[Scene::processUpdateList] list update not implemented!");
			}
		}
	}
}

void Scene::checkVisibleActorsPriority() {
	for (uint i = 2; i < 9; i++)
		if (getActor(i)->isVisible())
			adjustActorPriority(i);

	for (uint i = 16; i < 18; i++)
		if (getActor(i)->isVisible())
			adjustActorPriority(i);
}

void Scene::adjustActorPriority(ActorIndex index) {
	Actor* actor0 = getActor(0);
	Actor* actor = getActor(index);

	if (rectIntersect(actor0->getPoint1()->x,
	                  actor0->getPoint1()->y,
	                  actor0->getPoint1()->x + actor0->getBoundingRect()->right,
	                  actor0->getPoint1()->y + actor0->getBoundingRect()->bottom + 4,
					  actor->getPoint1()->x,
					  actor->getPoint1()->y,
					  actor->getPoint1()->x + actor0->getBoundingRect()->right,
					  actor->getPoint1()->y + actor0->getBoundingRect()->bottom)) {
		if (actor->getPriority() < actor0->getPriority())
			actor0->setPriority(actor->getPriority());
	}
}

void Scene::drawChapter5() {
	if (getSharedData()->getFlag(kFlagSkipDrawScene))
		return;

	for (uint y = 0; y < 512; y = y + 64) {
		for (uint x = 0; x < 704; x = x + 64) {
			getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 58), _chapter5FrameIndex, x + (_ws->xLeft % 64) / 8, y + (_ws->yTop % 64) / 8, 0);
		}
	}

	_chapter5FrameIndex = (_chapter5FrameIndex + 1) % GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, 58));
}

//////////////////////////////////////////////////////////////////////////
// Debug
//////////////////////////////////////////////////////////////////////////
void Scene::debugScreenScrolling() {
	if (!_ws)
		error("[Scene::debugScreenScrolling] WorldStats not initialized properly!");

	Common::Rect rect = GraphicResource::getFrameRect(_vm, _ws->backgroundImage, 0);

	// Horizontal scrolling
	if (getCursor()->position().x < SCREEN_EDGES && _ws->xLeft >= SCROLL_STEP)
		_ws->xLeft -= SCROLL_STEP;
	else if (getCursor()->position().x > 640 - SCREEN_EDGES && _ws->xLeft <= rect.width() - 640 - SCROLL_STEP)
		_ws->xLeft += SCROLL_STEP;

	// Vertical scrolling
	if (getCursor()->position().y < SCREEN_EDGES && _ws->yTop >= SCROLL_STEP)
		_ws->yTop -= SCROLL_STEP;
	else if (getCursor()->position().y > 480 - SCREEN_EDGES && _ws->yTop <= rect.height() - 480 - SCROLL_STEP)
		_ws->yTop += SCROLL_STEP;
}

// WALK REGION DEBUG
void Scene::debugShowWalkRegion(PolyDefinitions *poly) {
	Graphics::Surface surface;
	surface.create(poly->boundingRect.right - poly->boundingRect.left + 1,
	               poly->boundingRect.bottom - poly->boundingRect.top + 1,
	               1);

	// Draw all lines in Polygon
	for (uint32 i = 0; i < poly->count(); i++) {
		surface.drawLine(
		    poly->points[i].x - poly->boundingRect.left,
		    poly->points[i].y - poly->boundingRect.top,
		    poly->points[(i+1) % poly->count()].x - poly->boundingRect.left,
		    poly->points[(i+1) % poly->count()].y - poly->boundingRect.top, 0x3A);
	}

	getScreen()->copyToBackBufferClipped(&surface, poly->boundingRect.left, poly->boundingRect.top);

	surface.free();
}

// POLYGONS DEBUG
void Scene::debugShowPolygons() {
	if (!_polygons)
		error("[Scene::debugShowPolygons] Polygons not initialized properly!");

	for (int32 p = 0; p < _polygons->numEntries; p++) {
		Graphics::Surface surface;
		PolyDefinitions poly = _polygons->entries[p];
		surface.create(poly.boundingRect.right - poly.boundingRect.left + 1,
		               poly.boundingRect.bottom - poly.boundingRect.top + 1,
		               1);

		// Draw all lines in Polygon
		for (uint32 i = 0; i < poly.count(); i++) {
			surface.drawLine(
			    poly.points[i].x - poly.boundingRect.left,
			    poly.points[i].y - poly.boundingRect.top,
			    poly.points[(i+1) % poly.count()].x - poly.boundingRect.left,
			    poly.points[(i+1) % poly.count()].y - poly.boundingRect.top, 0xFF);
		}

		getScreen()->copyToBackBufferClipped(&surface, poly.boundingRect.left, poly.boundingRect.top);

		surface.free();
	}
}

// OBJECT DEBUGGING
void Scene::debugShowObjects() {
	if (!_ws)
		error("[Scene::debugShowObjects] WorldStats not initialized properly!");

	for (uint32 p = 0; p < _ws->objects.size(); p++) {
		Graphics::Surface surface;
		Object *object = _ws->objects[p];

		if (object->flags & 0x20) {
			surface.create(object->getBoundingRect()->right - object->getBoundingRect()->left + 1,
			               object->getBoundingRect()->bottom - object->getBoundingRect()->top + 1,
			               1);
			surface.frameRect(*object->getBoundingRect(), 0x22);
			getScreen()->copyToBackBufferClipped(&surface, object->x, object->y);
		}

		surface.free();
	}
}

// ACTOR DEBUGGING
void Scene::debugShowActors() {
	for (uint32 p = 0; p < _ws->actors.size(); p++) {
		Graphics::Surface surface;
		Actor *a = _ws->actors[p];

		if (a->flags & 2) {
			surface.create(a->getBoundingRect()->right - a->getBoundingRect()->left + 1,
			               a->getBoundingRect()->bottom - a->getBoundingRect()->top + 1,
			               1);
			surface.frameRect(*a->getBoundingRect(), 0x22);
			getScreen()->copyToBackBufferClipped(&surface, a->getPoint()->x, a->getPoint()->y);
		}

		surface.free();
	}
}

} // end of namespace Asylum
