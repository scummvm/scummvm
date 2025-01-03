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

#include "asylum/views/scene.h"

#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/inventory.h"
#include "asylum/resources/object.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/script.h"
#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

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

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugActors;
int g_debugObjects;
int g_debugPolygons;
int g_debugPolygonIndex;
int g_debugSceneRects;
int g_debugScrolling;

Scene::Scene(AsylumEngine *engine): _vm(engine),
	_polygons(nullptr), _ws(nullptr) {

	// Initialize data
	_packId = kResourcePackInvalid;

	_hitAreaChapter7Counter = 0;
	_isCTRLPressed = false;
	_chapter5RainFrameIndex = 0;

	_musicVolume = 0;
	_frameCounter = 0;

	_keyState = 0;
	_rightButtonDown = false;

	_savedScreen.create(640, 480, Graphics::PixelFormat::createFormatCLUT8());

	_debugShowVersion = false;
	g_debugActors = 0;
	g_debugObjects  = 0;
	g_debugPolygons  = 0;
	g_debugPolygonIndex = 0;
	g_debugSceneRects = 0;
	g_debugScrolling = 0;
}

Scene::~Scene() {
	// Unload the associated resources
	getResource()->unload(_packId);

	// Clear script queue
	getScript()->reset();

	_savedScreen.free();

	delete _polygons;
	delete _ws;
}

void Scene::enter(ResourcePackId packId) {
	_vm->setGameFlag(kGameFlagScriptProcessing);

	getCursor()->hide();

	getSharedData()->setPlayerIndex(0);

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

	player->show(); // flag |= 1 (show actor)
	player->enable();

	// Update current player coordinates
	player->getPoint1()->x -= player->getPoint2()->x;
	player->getPoint1()->y -= player->getPoint2()->y;

	// Update all other actors
	if (_ws->actors.size() > 1) {
		for (uint32 i = 1; i < _ws->actors.size(); i++) {
			Actor *actor = _ws->actors[i];

			actor->show();
			actor->setDirection(kDirectionNW);
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

void Scene::enterLoad() {
	if (!_ws)
		error("[Scene::enterLoad] WorldStats not initialized properly");

	_vm->setGameFlag(kGameFlagScriptProcessing);
	getScreen()->clearGraphicsInQueue();

	// Setup scene bounding rect
	_ws->boundingRect.left   = 195;
	_ws->boundingRect.top    = 115;
	_ws->boundingRect.right  = 445 - getActor()->getBoundingRect()->right;
	_ws->boundingRect.bottom = 345 - getActor()->getBoundingRect()->bottom;

	// Setup transparency table
	getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);
	getScreen()->selectTransTable(1);
	getText()->loadFont(_ws->font1);

	preload();

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

	// Play intro music
	if (_ws->musicCurrentResourceIndex != kMusicStopped)
		getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, _ws->musicCurrentResourceIndex));
	else
		getSound()->playMusic(kResourceNone, 0);

	// Palette fade
	getScreen()->paletteFade(0, 75, 8);
	getScreen()->clear();

	getSharedData()->setFlag(kFlagScene1, true);
	_vm->lastScreenUpdate = 1;
	getActor()->setLastScreenUpdate(_vm->screenUpdateCount);
}

void Scene::load(ResourcePackId packId) {
	// Setup resource manager
	_packId = packId;
	getResource()->setMusicPackId(packId);

	char filename[10];
	snprintf(filename, 10, SCENE_FILE_MASK, _packId);

	char sceneTag[6];
	Common::File *fd = new Common::File;

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

	if (_vm->checkGameVersion("Demo"))
		fd->seek(0x1D72E, SEEK_SET);

	_polygons = new Polygons(fd);

	if (_vm->checkGameVersion("Demo"))
		fd->seek(3 * 4, SEEK_CUR);

	ScriptManager *script = getScript();
	script->resetAll();
	script->load(fd);

	fd->close();
	delete fd;

	getSharedData()->resetAmbientFlags();
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
	switch ((int32)evt.type) {
	default:
		break;

	case EVENT_ASYLUM_INIT:
		return init();

	case EVENT_ASYLUM_ACTIVATE:
		activate();
		break;

	case EVENT_ASYLUM_UPDATE:
		return update();

	case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
		return actionDown((AsylumAction)evt.customType);

	case Common::EVENT_CUSTOM_ENGINE_ACTION_END:
		return actionUp((AsylumAction)evt.customType);

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
		return getCursor()->isHidden() ? false : clickDown(evt);

	case Common::EVENT_RBUTTONUP:
		_rightButtonDown = false;
		activate();
		break;
	}

	return false;
}

void Scene::activate() {
	Actor *player = getActor();

	if (player->getStatus() == kActorStatusWalking)
		player->changeStatus(kActorStatusEnabled);

	if (player->getStatus() == kActorStatusWalking2)
		player->changeStatus(kActorStatusEnabled2);
}

bool Scene::init() {
	if (!_ws)
		error("[Scene::init] WorldStats not initialized properly");

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
	getScreen()->setGammaLevel(paletteResource);
	getScreen()->loadGrayPalette();
	getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);
	getScreen()->selectTransTable(1);

	getCursor()->show();

	return true;
}

bool Scene::update() {
	if (getEncounter()->shouldEnablePlayer()) {
		getEncounter()->setShouldEnablePlayer(false);

		// Enable player
		getActor()->changeStatus(kActorStatusEnabled);
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
				getEncounter()->drawScreen();

			// Original also sets an unused value to 0
			getSharedData()->setEventUpdate(getSharedData()->getEventUpdate() ^ 1);

			getSharedData()->setFlag(kFlagRedraw, false);
			getSharedData()->setNextScreenUpdate(ticks + 55 / Config.animationsSpeed);
			++_vm->screenUpdateCount;
		}
	}

	return true;
}

bool Scene::actionDown(AsylumAction a) {
	Actor *player = getActor();

	switch (a) {
	case kAsylumActionShowVersion:
		_debugShowVersion = !_debugShowVersion;
		break;

	case kAsylumActionQuickLoad:
		if (!_vm->checkGameVersion("Demo"))
			getSaveLoad()->quickLoad();
		break;

	case kAsylumActionQuickSave:
		if (!_vm->checkGameVersion("Demo"))
			getSaveLoad()->quickSave();
		break;

	case kAsylumActionSwitchToSarah:
	case kAsylumActionSwitchToGrimwall:
	case kAsylumActionSwitchToOlmec:
		if (getCursor()->isHidden() || _ws->chapter != kChapter9)
			return true;

		getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2206 + a - kAsylumActionSwitchToSarah)]->scriptIndex,
								 getSharedData()->getPlayerIndex());
		break;

	case kAsylumActionOpenInventory:
		if (getActor()->inventory[0] && getActor()->getStatus() == kActorStatusEnabled && !getActor()->inventory.getSelectedItem()) {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 2));
			getActor()->changeStatus(kActorStatusShowingInventory);
		} else if (getActor()->getStatus() == kActorStatusShowingInventory || getActor()->getStatus() == kActorStatus10) {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
			getActor()->changeStatus(kActorStatusEnabled);
		}
		break;

	case kAsylumActionShowMenu:
		if (getSpeech()->getSoundResourceId()) {
			getScene()->stopSpeech();
		} else {
			if (getCursor()->isHidden())
				break;

			if (!_vm->checkGameVersion("Demo")) {
				_savedScreen.copyFrom(*getScreen()->getSurface());
				memcpy(_savedPalette, getScreen()->getPalette(), sizeof(_savedPalette));
				_vm->switchEventHandler(_vm->menu());
			}
		}
		break;

	case kAsylumActionMoveUp:
		if (player->getStatus() != kActorStatusDisabled) {
			player->changeStatus(kActorStatusWalking);
		}
		_keyState |= kWalkUp;
		break;

	case kAsylumActionMoveDown:
		if (player->getStatus() != kActorStatusDisabled) {
			player->changeStatus(kActorStatusWalking);
		}
		_keyState |= kWalkDown;
		break;

	case kAsylumActionMoveLeft:
		if (player->getStatus() != kActorStatusDisabled) {
			player->changeStatus(kActorStatusWalking);
		}
		_keyState |= kWalkLeft;
		break;

	case kAsylumActionMoveRight:
		if (player->getStatus() != kActorStatusDisabled) {
			player->changeStatus(kActorStatusWalking);
		}
		_keyState |= kWalkRight;
		break;

	default:
		break;
	}

	return true;
}

bool Scene::actionUp(AsylumAction a) {
	byte lastKeyState = _keyState;

	switch (a) {
	case kAsylumActionMoveUp:
		_keyState &= ~kWalkUp;
		break;

	case kAsylumActionMoveDown:
		_keyState &= ~kWalkDown;
		break;

	case kAsylumActionMoveLeft:
		_keyState &= ~kWalkLeft;
		break;

	case kAsylumActionMoveRight:
		_keyState &= ~kWalkRight;
		break;

	default:
		break;
	}

	if (lastKeyState && !_keyState)
		activate();

	return true;
}

bool Scene::key(const AsylumEvent &evt) {
	if (!_ws)
		error("[Scene::key] WorldStats not initialized properly");

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

	case Common::KEYCODE_LEFTBRACKET:
		if (evt.kbd.ascii != 123)
			break;
		// fallthrough

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
	}

	return true;
}

bool Scene::clickDown(const AsylumEvent &evt) {
	if (g_debugScrolling) {
		g_debugScrolling = 0;
		getActor()->setPosition(_ws->xLeft + evt.mouse.x, _ws->yTop + evt.mouse.y, getActor()->getDirection(), getActor()->getFrameIndex());
		return true;
	}

	_vm->lastScreenUpdate = 0;

	if (getSharedData()->getFlag(kFlag2)) {
		stopSpeech();

		return true;
	}

	Actor *player = getActor();
	player->setLastScreenUpdate(_vm->screenUpdateCount);

	switch (evt.type) {
	default:
		break;

	case Common::EVENT_RBUTTONDOWN:
		if (getSpeech()->getSoundResourceId())
			stopSpeech();

		if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10) {
			player->changeStatus(kActorStatusEnabled);
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
		} else if (player->getStatus() != kActorStatusDisabled) {
			player->changeStatus(kActorStatusWalking);
		}

		_rightButtonDown = true;
		break;

	case Common::EVENT_LBUTTONDOWN:
		if (_rightButtonDown || _keyState)
			break;

		if (getSpeech()->getSoundResourceId())
			stopSpeech();

		if (player->getStatus() == kActorStatusDisabled)
			break;

		if (player->inventory.getSelectedItem()) {
			if (hitTestPlayer()) {
				player->inventory.selectItem(0);
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

		if (!hitTestPlayer() || player->getStatus() >= kActorStatus11 || !player->inventory[0]) {
			if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10) {
				clickInventory();
			} else {
				HitType type = kHitNone;
				int32 res = hitTest(type);
				if (res != -1)
					handleHit(res, type);
			}
			return true;
		}

		if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10) {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
			player->changeStatus(kActorStatusEnabled);
		} else {
			getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 2));
			player->changeStatus(kActorStatusShowingInventory);
		}
		break;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
// Scene update
//////////////////////////////////////////////////////////////////////////
bool Scene::updateScreen() {
	_frameCounter++;

	if (updateScene())
		return true;

#if 0
	if (Config.performance <= 4) {
		// TODO: implement skip drawing frames to screen
	} else {
#endif
		if (drawScene())
			return true;
	//}

	getActor()->drawNumber();

	// Original handle all debug commands here (we do it as part of each update command)
	if (_debugShowVersion) {
		getText()->setPosition(Common::Point(0, 0));
		getText()->loadFont(_ws->font1);
		getText()->draw(Common::String::format("Version %s / Build %d",
												getSaveLoad()->getVersion(),
												getSaveLoad()->getBuild()).c_str());
	}

	if (getSharedData()->getFlag(kFlagScene1)) {
		getScreen()->clear();

		getScreen()->stopPaletteFade(0, 0, 0);
		updateScene();
		drawScene();
		getScreen()->copyBackBufferToScreen();

		getScreen()->stopPaletteFadeAndSet(getWorld()->currentPaletteId, 100, 10);
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
			drawRain();
	}

	return false;
}


bool Scene::updateScene() {
#ifdef DEBUG_SCENE_TIMES
#define MEASURE_TICKS(func) { \
	int32 startTick =_vm->getTick(); \
	func(); \
	debugC(kDebugLevelScene, "[Scene] " #func " - Time: %d", _vm->getTick() - startTick); \
}
#else
#define MEASURE_TICKS(func) func();
#endif

	// Update each part of the scene
	if (getSharedData()->getMatteBarHeight() != 170 || getSharedData()->getMattePlaySound()) {
		MEASURE_TICKS(updateMouse);
		MEASURE_TICKS(updateActors);
		MEASURE_TICKS(updateObjects);
		MEASURE_TICKS(updateAmbientSounds);
		MEASURE_TICKS(updateMusic);
		MEASURE_TICKS(updateAdjustScreen);
	}

	return getScript()->process();
}

void Scene::updateMouse() {
	Actor *player = getActor();
	Common::Point mouse = getCursor()->position();

	Common::Point pt;
	player->adjustCoordinates(&pt);

	Common::Rect actorRect;
	if (_ws->chapter != kChapter2 || getSharedData()->getPlayerIndex() != 10) {
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

	if (_keyState) {
		if (_keyState & kWalkLeft) {
			if (_keyState & kWalkUp) {
				newDirection = kDirectionNW;
			} else if (_keyState & kWalkDown) {
				newDirection = kDirectionSW;
			} else {
				newDirection = kDirectionW;
			}
		} else if (_keyState & kWalkRight) {
			if (_keyState & kWalkUp) {
				newDirection = kDirectionNE;
			} else if (_keyState & kWalkDown) {
				newDirection = kDirectionSE;
			} else {
				newDirection = kDirectionE;
			}
		} else if (_keyState & kWalkUp) {
			newDirection = kDirectionN;
		} else if (_keyState & kWalkDown) {
			newDirection = kDirectionS;
		}

		updateCursor(newDirection, actorRect);

		if (newDirection >= kDirectionN)
			if (player->getStatus() == kActorStatusWalking || player->getStatus() == kActorStatusWalking2)
				player->changeDirection(newDirection);
		return;
	}

	if (mouse.x < actorRect.left) {
		if (mouse.y >= actorRect.top) {
			if (mouse.y > actorRect.bottom) {
				if (player->getDirection() == kDirectionW) {
					if ((mouse.y - actorRect.bottom) > 10)
						newDirection = kDirectionSW;
				} else {
					if (player->getDirection() == kDirectionS) {
						if ((actorRect.left - mouse.x) > 10)
							newDirection = kDirectionSW;
					} else {
						newDirection = kDirectionSW;
					}
				}
			} else {
				if (player->getDirection() == kDirectionNW) {
					if ((mouse.y - actorRect.top) > 10)
						newDirection = kDirectionW;
				} else {
					if (player->getDirection() == kDirectionSW) {
						if ((actorRect.bottom - mouse.y) > 10)
							newDirection = kDirectionW;
					} else {
						newDirection = kDirectionW;
					}
				}
			}
		} else {
			if (player->getDirection() != kDirectionN) {
				if (player->getDirection() == kDirectionW) {
					if ((actorRect.top - mouse.y) > 10)
						newDirection = kDirectionNW;
				} else {
					newDirection = kDirectionNW;
				}
			} else {
				if ((actorRect.left - mouse.x) > 10)
					newDirection = kDirectionNW;
			}
		}

	} else if (mouse.x <= actorRect.right) {
		if (mouse.y >= actorRect.top) {
			if (mouse.y > actorRect.bottom) {
				if (player->getDirection() == kDirectionSW) {
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
			if (player->getDirection() == kDirectionNW) {
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
	} else if (player->getDirection() == kDirectionS) {
		if ((mouse.x - actorRect.right) > 10)
			newDirection = kDirectionSE;
	} else if ((player->getDirection() != kDirectionE || (mouse.y - actorRect.bottom) > 10)) {
		newDirection = kDirectionSE;
	}

	updateCursor(newDirection, actorRect);

	if (newDirection >= kDirectionN)
		if (player->getStatus() == kActorStatusWalking || player->getStatus() == kActorStatusWalking2)
			player->changeDirection(newDirection);
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

	for (uint32 i = 0; i < _ws->numAmbientSounds; i++) {
		bool processSound = true;
		AmbientSoundItem *snd = &_ws->ambientSounds[i];
		uint32 ambientTick = getSharedData()->getAmbientTick(i);

		for (int32 f = 0; f < 6; f++) {
			int32 gameFlag = snd->flagNum[f];

			if (!gameFlag)
				break;

			if (gameFlag >= 0) {
				if (_vm->isGameFlagNotSet((GameFlag)gameFlag)) {
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
					int32 volume = Config.ambientVolume + getSound()->calculateVolumeAdjustement(snd->point, snd->attenuation, snd->delta);

					if (volume <= 0) {
						if (volume < -10000)
							volume = -10000;

						getSound()->setVolume(snd->resourceId, volume);
					} else {
						getSound()->setVolume(snd->resourceId, 0);
					}
				}

			} else {
				int32 panning = (snd->field_0) ? getSound()->calculatePanningAtPoint(snd->point) : 0;

				int32 volume = 0;
				if (snd->field_0)
					volume = getSound()->calculateVolumeAdjustement(snd->point, snd->attenuation, snd->delta);
				else
					volume = -(int32)pow((double)snd->delta, 2);

				volume += Config.ambientVolume;


				if (LOBYTE(snd->flags) & 1) {

					getSound()->playSound(snd->resourceId, true, volume, panning);

				} else if (LOBYTE(snd->flags) & 2) {
					if (_vm->getRandom(10000) < 10) {
						if (snd->field_0) {
							getSound()->playSound(snd->resourceId, false, volume, panning);
						} else {
							int32 tmpVol = volume + (int32)_vm->getRandom(500) * ((_vm->getRandom(100) >= 50) ? -1 : 1);

							if (tmpVol >= 0)
								tmpVol = 0;
							else if (tmpVol <= -10000)
								tmpVol = -10000;

							getSound()->playSound(snd->resourceId, false, tmpVol, _vm->getRandom(20001) - 10000);
						}
					}
				} else if (LOBYTE(snd->flags) & 4) {
					if (ambientTick < _vm->getTick()) {
						if (snd->nextTick >= 0)
							getSharedData()->setAmbientTick(i, (uint32)((int32)_vm->getTick() + snd->nextTick * 60000));
						else
							getSharedData()->setAmbientTick(i, (uint32)((int32)_vm->getTick() - snd->nextTick * 1000));

						getSound()->playSound(snd->resourceId, false, volume, panning);
					}
				} else if (LOBYTE(snd->flags) & 8) {
					if (!getSharedData()->getAmbientFlag(i)) {
						getSound()->playSound(snd->resourceId, false, volume, panning);
						getSharedData()->setAmbientFlag(i, 1);
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
	if (!getWorld()->musicFlag)
		return;

	if (getWorld()->musicCurrentResourceIndex != kMusicStopped) {
		switch (getWorld()->musicStatus) {
		default:
			break;

		case 1:
			if (getWorld()->musicResourceIndex == kMusicStopped) {
				getWorld()->musicCurrentResourceIndex = kMusicStopped;
				getWorld()->musicStatus = 0;
				getSound()->playMusic(kResourceNone, 0);
			} else {
				getWorld()->musicCurrentResourceIndex = getWorld()->musicResourceIndex;
				getWorld()->musicStatus = getWorld()->musicStatusExt;
				getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicResourceIndex));
			}
			getWorld()->musicResourceIndex = kMusicStopped;
			getWorld()->musicStatusExt = 0;
			getWorld()->musicFlag = 0;
			break;

		case 2:
			_musicVolume = getSound()->getMusicVolume();
			getWorld()->musicStatus = 4;
			break;

		case 4:
			_musicVolume -= 150;
			if (_musicVolume > -2500) {
				getSound()->setMusicVolume(_musicVolume);
				break;
			}

			_musicVolume = -10000;
			getWorld()->musicCurrentResourceIndex = kMusicStopped;

			if (getWorld()->musicResourceIndex == kMusicStopped) {
				getWorld()->musicStatus = 0;
				getSound()->playMusic(kResourceNone, 0);
				getWorld()->musicResourceIndex = kMusicStopped;
				getWorld()->musicStatusExt = 0;
				getWorld()->musicFlag = 0;
			} else {
				getWorld()->musicStatus = 8;
				getSound()->playMusic(kResourceNone, 0);
				getWorld()->musicCurrentResourceIndex = getWorld()->musicResourceIndex;
				_musicVolume = -2500;
				getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicResourceIndex), _musicVolume);
			}
			break;

		case 8:
			_musicVolume += 150;
			if (_musicVolume < Config.musicVolume) {
				getSound()->setMusicVolume(_musicVolume);
				break;
			}

			getSound()->setMusicVolume(Config.musicVolume);
			getWorld()->musicStatus = getWorld()->musicStatusExt;
			getWorld()->musicResourceIndex = kMusicStopped;
			getWorld()->musicStatusExt = 0;
			getWorld()->musicFlag = 0;
			break;
		}
	} else if (getWorld()->musicResourceIndex != kMusicStopped) {
		switch (getWorld()->musicStatusExt) {
		default:
			break;

		case 1:
			getWorld()->musicCurrentResourceIndex = getWorld()->musicResourceIndex;
			getWorld()->musicStatus = 1;
			getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicResourceIndex));
			getWorld()->musicResourceIndex = kMusicStopped;
			getWorld()->musicStatusExt = 0;
			getWorld()->musicFlag = 0;
			break;

		case 2:
			_musicVolume = -10000;
			getSound()->setMusicVolume(_musicVolume);
			getWorld()->musicCurrentResourceIndex = getWorld()->musicResourceIndex;
			getWorld()->musicStatus = 8;
			getSound()->playMusic(MAKE_RESOURCE(kResourcePackMusic, getWorld()->musicResourceIndex), _musicVolume);
			break;
		}
	} else {
		getWorld()->musicFlag = 0;
	}
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
	int16 xLeft = _ws->xLeft, oxLeft = xLeft;
	int16 yTop  = _ws->yTop, oyTop = yTop;
	int16 posX = act->getPoint1()->x - _ws->xLeft;
	int16 posY = act->getPoint1()->y - _ws->yTop;
	Common::Rect boundingRect = _ws->boundingRect;

	switch (_ws->motionStatus) {
	default:
		break;

	case 1:
		if (posX < boundingRect.left) {
			xLeft = (posX - boundingRect.left) + _ws->xLeft;
			_ws->xLeft += posX - boundingRect.left;
		} else if (posX > boundingRect.right) {
			xLeft = (posX - boundingRect.right) + _ws->xLeft;
			_ws->xLeft += posX - boundingRect.right;
		}

		if (posY < boundingRect.top) {
			yTop = (posY - boundingRect.top) + _ws->yTop;
			_ws->yTop += posY - boundingRect.top;
		} else if (posY > boundingRect.bottom) {
			yTop = (posY - boundingRect.bottom) + _ws->yTop;
			_ws->yTop += posY - boundingRect.bottom;
		}

		if (xLeft < 0)
			xLeft = _ws->xLeft = 0;

		if (xLeft > (_ws->width - 640))
			xLeft = _ws->xLeft = _ws->width - 640;

		if (yTop < 0)
			yTop = _ws->yTop = 0;

		if (yTop > (_ws->height - 480))
			yTop = _ws->yTop = _ws->height - 480;

		break;

	case 2:
	case 5: {
		getSharedData()->setSceneOffset(getSharedData()->getSceneOffset() + getSharedData()->getSceneOffsetAdd());

		int32 coord1 = 0;
		int32 coord2 = 0;

		if (abs(getSharedData()->getSceneCoords().x - _ws->coordinates[0]) <= abs(getSharedData()->getSceneCoords().y - _ws->coordinates[1])) {
			coord1 = _ws->coordinates[1];
			coord2 = yTop;

			if (_ws->coordinates[1] != _ws->yTop)
				xLeft = _ws->xLeft = getSharedData()->getSceneOffset() + getSharedData()->getSceneCoords().x;

			yTop = _ws->coordinates[2] + _ws->yTop;
			_ws->yTop += _ws->coordinates[2];
		} else {
			coord1 = _ws->coordinates[0];
			coord2 = xLeft;

			if (_ws->coordinates[0] != _ws->xLeft)
				yTop = _ws->yTop = getSharedData()->getSceneOffset() + getSharedData()->getSceneCoords().y;

			xLeft = _ws->coordinates[2] + _ws->xLeft;
			_ws->xLeft += _ws->coordinates[2];
		}

		if (abs(coord2 - coord1) <= abs(_ws->coordinates[2])) {
			_ws->motionStatus = 3;
			_ws->coordinates[0] = -1;
		}

		}
		break;
	}

	// Update scene coordinates
	Common::Rect sceneRect = _ws->sceneRects[_ws->sceneRectIdx];

	if (xLeft < sceneRect.left)
		xLeft =_ws->xLeft =sceneRect.left;

	if (yTop < sceneRect.top)
		yTop = _ws->yTop = sceneRect.top;

	if ((xLeft + 639) > sceneRect.right)
		xLeft = _ws->xLeft = sceneRect.right - 639;

	if ((yTop + 479) > sceneRect.bottom)
		yTop = _ws->yTop = sceneRect.bottom - 479;

	// XXX dword_44E1EC is set to 2 at this point if the scene coordinates
	// have changed, but that variable is never used anywhere else
	if ((_ws->motionStatus == 2 || _ws->motionStatus == 5) && (oxLeft != _ws->xLeft || oyTop != _ws->yTop))
		debugC(kDebugLevelScene, "[Scene::updateCoordinates] (%d, %d) ~> (%d, %d), motionStatus = %d",
				_ws->xLeft, _ws->yTop, _ws->coordinates[0], _ws->coordinates[1], _ws->motionStatus);
}

void Scene::updateCursor(ActorDirection direction, const Common::Rect &rect) {
	HitType type = kHitNone;
	Actor *player = getActor();
	int16 rightLimit = rect.right - 10;
	Common::Point mouse = getCursor()->position();

	if (getSharedData()->getFlag(kFlagIsEncounterRunning)) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceTalkNPC])
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC]);

		return;
	}

	if (_rightButtonDown || _keyState) {
		if (player->getStatus() == kActorStatusWalking || player->getStatus() == kActorStatusWalking2) {

			if (direction >= kDirectionN) {
				ResourceId resourceId =_ws->cursorResources[direction];

				if (getCursor()->getResourceId() != resourceId)
					getCursor()->set(resourceId);
			}
		}

		return;
	}

	if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceHand])
			getCursor()->set(_ws->cursorResources[kCursorResourceHand]);

		return;
	}

	if (player->inventory.getSelectedItem()) {
		if (mouse.x >= rect.left && mouse.x <= rightLimit && mouse.y >= rect.top  && mouse.y <= rect.bottom && hitTestPlayer()) {

			ResourceId id = _ws->inventoryCursorsNormal[player->inventory.getSelectedItem() - 1];
			if (getCursor()->getResourceId() != id)
				getCursor()->set(id, 0, kCursorAnimationNone);

		} else {
			if (hitTestScene(type) == -1) {
				ResourceId id = _ws->inventoryCursorsNormal[player->inventory.getSelectedItem() - 1];
				if (getCursor()->getResourceId() != id)
					getCursor()->set(id, 0, kCursorAnimationNone);
			} else {
				ResourceId id = _ws->inventoryCursorsBlinking[player->inventory.getSelectedItem() - 1];
				uint32 frameCount = GraphicResource::getFrameCount(_vm, id);
				if (getCursor()->getResourceId() != id)
					getCursor()->set(id, 0, (frameCount <= 1) ? kCursorAnimationNone : kCursorAnimationMirror);
			}
		}

		return;
	}

	if (mouse.x >= rect.left && mouse.x <= rightLimit && mouse.y >= rect.top  && mouse.y <= rect.bottom && hitTestPlayer()) {
		if (player->inventory[0]) {
			if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceGrabPointer])
				getCursor()->set(_ws->cursorResources[kCursorResourceGrabPointer]);

			return;
		}
	}

	int32 index = hitTest(type);
	if (index == -1) {
		if (_ws->chapter != kChapter2 || getSharedData()->getPlayerIndex() != 10) {
			if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->getAnimation())
				getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
		} else {
			if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->getAnimation())
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
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->getAnimation() != kCursorAnimationMirror)
			getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
	} else if (actionType & kActionTypeTalk) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceTalkNPC])
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC]);
	} else if (actionType & kActionTypeGrab) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceHand])
			getCursor()->set(_ws->cursorResources[kCursorResourceHand]);
	} else if (actionType & kActionType16) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->getAnimation() != kCursorAnimationMirror)
			getCursor()->set(_ws->cursorResources[kCursorResourceTalkNPC2]);
	} else if (_ws->chapter != kChapter2 && getSharedData()->getPlayerIndex() != 10) {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceMagnifyingGlass] || getCursor()->getAnimation())
			getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass]);
	} else {
		if (getCursor()->getResourceId() != _ws->cursorResources[kCursorResourceTalkNPC2] || getCursor()->getAnimation())
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

	int16 top  = pt.x + _ws->xLeft;
	int16 left = pt.y + _ws->yTop;
	type = kHitNone;

	int32 index = findActionArea(kActionAreaType2, Common::Point(top, left));
	if (index != -1) {
		if (_ws->actions[index]->actionType & kActionType8) {
			type = kHitActionArea;
			return index;
		}
	}

	// Check objects
	for (uint i = 0; i < _ws->objects.size(); i++) {
		Object *object = _ws->objects[i];

		if (object->isOnScreen() && (object->actionType & kActionType8)) {
			if (hitTestPixel(object->getResourceId(),
			                 object->getFrameIndex(),
			                 top - object->x,
			                 left - object->y,
			                 (bool)(object->flags & kObjectFlag1000))) {
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
				             top  - (actor->getPoint()->x + actor->getPoint1()->x),
				             left - (actor->getPoint()->y + actor->getPoint1()->y),
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

	if (targetIdx == -1 || !(_ws->actions[targetIdx]->actionType & (kActionTypeFind | kActionTypeTalk | kActionTypeGrab | kActionType16)))
		return -1;

	return targetIdx;
}

ActorIndex Scene::hitTestActor() {
	if (!_ws)
		error("[Scene::hitTestActor] WorldStats not initialized properly!");

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
			Common::Point pt = mouse + Common::Point(_ws->xLeft, _ws->yTop) - *actor11->getPoint1();

			if (actor11->getBoundingRect()->contains(pt))
				return 11;
		}
	}

	switch (_ws->chapter) {
	default:
		break;

	case kChapter8:
		if (_ws->actors.size() < 7)
			error("[Scene::hitTestActor] Not enough actors to check (chapter 8 - checking actors 1-6)!");

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

		if (!actor->isOnScreen() || !actor->actionType)
			continue;

		uint32 hitFrame;
		if (actor->getFrameIndex() >= actor->getFrameCount())
			hitFrame = 2 * actor->getFrameIndex() - (actor->getFrameCount() + 1);
		else
			hitFrame = actor->getFrameIndex();

		if (hitTestPixel(actor->getResourceId(),
						 hitFrame,
						 _ws->xLeft + mouse.x - (actor->getPoint1()->x + actor->getPoint()->x),
						 _ws->yTop  + mouse.y - (actor->getPoint1()->y + actor->getPoint()->y),
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
	                    pt.x - (player->getPoint()->x + point.x),
	                    pt.y - (player->getPoint()->y + point.y),
	                    player->getDirection() >= kDirectionSE);
}

int32 Scene::hitTestObject() {
	if (!_ws)
		error("[Scene::hitTestObject] WorldStats not initialized properly!");

	const Common::Point pt = getCursor()->position();

	for (int32 i = _ws->objects.size() - 1; i >= 0; i--) {
		Object *object = _ws->objects[i];
		if (object->isOnScreen() && object->actionType)
			if (hitTestPixel(object->getResourceId(),
			                 object->getFrameIndex(),
			                 _ws->xLeft + pt.x - object->x,
			                 _ws->yTop + pt.y - object->y,
			                 (bool)(object->flags & kObjectFlag1000)))
				return i;
	}

	return -1;
}

bool Scene::hitTestPixel(ResourceId resourceId, uint32 frameIndex, int16 x, int16 y, bool flipped) {
	if (x < 0 || y < 0)
		return false;

	GraphicResource *resource = new GraphicResource(_vm, resourceId);
	GraphicFrame *frame = resource->getFrame(frameIndex);
	Common::Rect frameRect = frame->getRect();

	// Check y coordinates
	if (y < frameRect.top || y >= frameRect.bottom)
		goto cleanup;

	// Compute left/right x coordinates, using flipped value
	int32 left, right;
	if (flipped) {
		if (getScreen()->getFlag() == -1) {
			left = resource->getData().maxWidth - frameRect.right;
			right = resource->getData().maxWidth - frameRect.left;
		} else {
			left = frameRect.left + 2 * (getScreen()->getFlag() - (frameRect.right / 2));
			right = x;
		}
	} else {
		left = frameRect.left;
		right = frameRect.right;
	}

	// Check x coordinates
	if (x < left || x >= right)
		goto cleanup;

	// Check pixel value
	byte *pixel;
	if (flipped) {
		pixel = (byte *)frame->surface.getBasePtr((left - x) + frame->getWidth() - 1, y - frame->y);
	} else {
		pixel = (byte *)frame->surface.getBasePtr(x - left, y - frame->y);
	}

	if (*pixel == 0)
		goto cleanup;

	delete resource;

	return true;

cleanup:
	delete resource;

	return false;
}

//////////////////////////////////////////////////////////////////////////
// Hit actions
//////////////////////////////////////////////////////////////////////////
void Scene::handleHit(int32 index, HitType type) {
	if (!_ws)
		error("[Scene::handleHit] WorldStats not initialized properly!");

	switch (type) {
	default:
		break;

	case kHitActionArea:
		if (!getScript()->isInQueue(_ws->actions[index]->scriptIndex)) {
			debugC(kDebugLevelScripts, "[Script] Queuing Script idx: %d from kHitActionArea (idx: %d, name: '%s')",
					_ws->actions[index]->scriptIndex, index, _ws->actions[index]->name);
			getScript()->queueScript(_ws->actions[index]->scriptIndex, getSharedData()->getPlayerIndex());
		}

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

		if (!getScript()->isInQueue(object->getScriptIndex())) {
			debugC(kDebugLevelScripts, "[Script] Queuing Script idx: %d from kHitObject (id: %d, name: '%s')",
								object->getScriptIndex(), object->getId(), object->getName());

			getScript()->queueScript(object->getScriptIndex(), getSharedData()->getPlayerIndex());
		}

		// Original executes special script hit functions, but since there is none defined, we can skip this part
		}
		break;

	case kHitActor: {
		Actor *actor = _ws->actors[index];

		if (actor->actionType & (kActionTypeFind | kActionType16)) {

			if (!getScript()->isInQueue(actor->getScriptIndex())) {
				debugC(kDebugLevelScripts, "[Script] Queuing Script idx: %d from kHitActor (id: %d, name: '%s')",
						actor->getScriptIndex(), index, actor->getName());
				getScript()->queueScript(actor->getScriptIndex(), getSharedData()->getPlayerIndex());
			}

		} else if (actor->actionType & kActionTypeTalk) {

			if (getSound()->isPlaying(actor->getSoundResourceId())) {
				if (actor->getStatus() != kActorStatusEnabled)
					actor->changeStatus(kActorStatusEnabled);

				getSound()->stop(actor->getSoundResourceId());
				actor->setSoundResourceId(kResourceNone);
			}

			if (!getScript()->isInQueue(actor->getScriptIndex())) {
				debugC(kDebugLevelScripts, "[Script] Queuing Script idx: %d from kActionTypeTalk (actor idx: %d)",
										actor->getScriptIndex(), getSharedData()->getPlayerIndex());
				getScript()->queueScript(actor->getScriptIndex(), getSharedData()->getPlayerIndex());
			}
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

void Scene::clickInventory() {
	const Common::Point mouse = getCursor()->position();
	Common::Point point;
	Actor *player = getActor();

	player->adjustCoordinates(&point);

	uint count = player->inventory.find();

	player->inventory.selectItem(0);

	if (count > 0) {
		for (uint32 i = 0; i < count; i++) {
			Common::Point ringPoint = Inventory::getInventoryRingPoint(_vm, count, i);
			int32 x = point.x + player->getPoint2()->x + ringPoint.x;
			int32 y = point.y + player->getPoint2()->y / 2 - ringPoint.y;

			if (mouse.x >= x && mouse.x <= (x + 40) && mouse.y >= y && mouse.y <= (y + 40)) {
				getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 4));

				if (_ws->chapter == kChapter9) {
					switch (i) {
					default:
						player->inventory.selectItem(player->inventory[i]);
						break;

					case 0:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2206)]->scriptIndex, getSharedData()->getPlayerIndex());
						break;

					case 1:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2207)]->scriptIndex, getSharedData()->getPlayerIndex());
						break;

					case 2:
						getScript()->queueScript(_ws->actions[_ws->getActionAreaIndexById(2208)]->scriptIndex, getSharedData()->getPlayerIndex());
						break;
					}
				} else {
					player->inventory.selectItem(player->inventory[i]);
				}
				break;
			}
		}
	}

	player->changeStatus(kActorStatusEnabled);
	getSound()->playSound(MAKE_RESOURCE(kResourcePackSound, 5));
}

void Scene::hitAreaChapter2(int32 id) {
	if (id == 783)
		getActor()->inventory.selectItem(6);
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
	if (!_ws)
		error("[Scene::hitAreaChapter11] WorldStats not initialized properly!");

	if (id == 1670)
		_ws->field_E849C = 666;
}


void Scene::hitActorChapter2(ActorIndex index) {
	if (!_ws)
		error("[Scene::hitActorChapter2] WorldStats not initialized properly!");

	Actor *player = getActor();

	if (player->getStatus() != kActorStatusEnabled2 && player->getStatus() != kActorStatusWalking2)
		return;

	if (index == 11) {
		player->faceTarget((uint32)index, kDirectionFromActor);
		player->changeStatus(kActorStatusAttacking);

		Actor *actor11 = getActor(index);

		Common::Point pointPlayer(player->getPoint1()->x + player->getPoint2()->x, player->getPoint1()->y + player->getPoint2()->y);
		Common::Point pointActor11(actor11->getPoint1()->x + actor11->getPoint2()->x, actor11->getPoint1()->y + actor11->getPoint2()->y);

		if (Actor::euclidianDistance(pointPlayer, pointActor11) < 150) {
			if (actor11->getStatus() == kActorStatusWalking2)
				actor11->changeStatus(kActorStatus18);

			if (actor11->getStatus() == kActorStatusEnabled)
				actor11->changeStatus(kActorStatusEnabled2);
		}

		getSharedData()->setChapter2ActorIndex(index);

	} else if (index > 12) {
		player->faceTarget((uint32)(index + 9), kDirectionFromActor);
		player->changeStatus(kActorStatusAttacking);
		getSharedData()->setChapter2ActorIndex(index);
	}
}

void Scene::hitActorChapter11(ActorIndex index) {
	if (!_ws)
		error("[Scene::hitActorChapter11] WorldStats not initialized properly!");

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
	getScreen()->stopPaletteFade(0, 0, 0);

	do {
		// Poll events (this ensure we don't freeze the screen)
		Common::Event ev;
		_vm->getEventManager()->pollEvent(ev);

		g_system->updateScreen();
		g_system->delayMillis(100);

	} while (getSound()->isPlaying(resourceId));
}

void Scene::stopSpeech() {
	if (_vm->isGameFlagNotSet(kGameFlag219)) {
		if (getSpeech()->getSoundResourceId() != kResourceNone && getSound()->isPlaying(getSpeech()->getSoundResourceId()))
			getSound()->stopAll(getSpeech()->getSoundResourceId());
		else if (getSpeech()->getTick())
			getSpeech()->setTick(_vm->getTick());
	}
}

bool Scene::speak(Common::KeyCode code) {
	if (!_ws)
		error("[Scene::speak] WorldStats not initialized properly!");

#define GET_INDEX() ((int)abs((double)_vm->getRandom(RAND_MAX)) & 1)

	int32 index = -1;

	switch (code) {
	default:
		break;

	case Common::KEYCODE_p:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = GET_INDEX();
			break;

		case kActorSarah:
		case kActorCyclops:
		case kActorAztec:
			index = 1;
			break;
		}
		break;

	case Common::KEYCODE_q:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 3 - GET_INDEX();
			break;

		case kActorSarah:
		case kActorCyclops:
		case kActorAztec:
			index = 2;
			break;
		}
		break;

	case Common::KEYCODE_r:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 2;
			break;

		case kActorSarah:
		case kActorCyclops:
		case kActorAztec:
			index = 4;
			break;
		}
		break;

	case Common::KEYCODE_s:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 5;
			break;

		case kActorSarah:
		case kActorCyclops:
		case kActorAztec:
			index = 3;
			break;
		}
		break;

	case Common::KEYCODE_t:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 6;
			break;

		case kActorSarah:
		case kActorCyclops:
		case kActorAztec:
			index = 4;
			break;
		}
		break;

	case Common::KEYCODE_u:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 7;
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 5;
			break;
		}
		break;

	case Common::KEYCODE_v:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 8;
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 6;
			break;
		}
		break;

	case Common::KEYCODE_w:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 9;
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 7;
			break;
		}
		break;

	case Common::KEYCODE_x:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 10;
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 8;
			break;
		}
		break;

	case Common::KEYCODE_y:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 11;
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 9;
			break;
		}
		break;

	case Common::KEYCODE_z:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 13 - GET_INDEX();
			break;

		case kActorSarah:
		case kActorCyclops:
			index = 10;
			break;
		}
		break;

	case Common::KEYCODE_LEFTBRACKET:
		switch (_ws->actorType) {
		default:
			break;

		case kActorMax:
			index = 15 - GET_INDEX();
			break;

		case kActorCyclops:
			index = 12 - GET_INDEX();
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

bool Scene::pointBelowLine(const Common::Point &point, const Common::Rect &rect) const {
	if (rect.top || rect.left || rect.bottom || rect.right) {
		Common::Rational res(rect.height() * (point.x - rect.left), rect.width());

		return (bool)(point.y > (rect.top + res.toInt()));
	}

	return true;
}

bool Scene::rectIntersect(int32 x, int32 y, int32 x1, int32 y1, int32 x2, int32 y2, int32 x3, int32 y3) const {
	return (x <= x3 && x1 >= x2) && (y <= y3 && y1 >= y2);
}

void Scene::adjustCoordinates(Common::Point *point) {
	if (!_ws)
		error("[Scene::adjustCoordinates] WorldStats not initialized properly!");

	point->x = _ws->xLeft + getCursor()->position().x;
	point->y = _ws->yTop  + getCursor()->position().y;
}

Actor *Scene::getActor(ActorIndex index) {
	if (!_ws)
		error("[Scene::getActor] WorldStats not initialized properly!");

	ActorIndex computedIndex =  (index != kActorInvalid) ? index : getSharedData()->getPlayerIndex();

	if (computedIndex < 0 || computedIndex >= (int16)_ws->actors.size())
		error("[Scene::getActor] Invalid actor index: %d ([0-%d] allowed)", computedIndex, _ws->actors.size() - 1);

	return _ws->actors[computedIndex];
}

bool Scene::updateSceneCoordinates(int32 tX, int32 tY, int32 A0, bool checkSceneCoords, int32 *param) {
	if (!_ws)
		error("[Scene::updateSceneCoordinates] WorldStats not initialized properly!");

	Common::Rect *sr = &_ws->sceneRects[_ws->sceneRectIdx];

	int16 *targetX = &_ws->coordinates[0];
	int16 *targetY = &_ws->coordinates[1];
	int16 *coord3  = &_ws->coordinates[2];

	*targetX = (int16)tX;
	*targetY = (int16)tY;

	*coord3 = (int16)A0;

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

	if (*targetY + 480 > sr->bottom)
		*targetY = sr->bottom - 480;

	if (checkSceneCoords)
		if (*targetY + 480 > _ws->height)
			*targetY = _ws->height - 480;

	// Adjust scene offsets & coordinates
	getSharedData()->setSceneOffset(0);
	getSharedData()->setSceneCoords(Common::Point(_ws->xLeft, _ws->yTop));

	int32 diffX = *targetX - _ws->xLeft;
	int32 diffY = *targetY - _ws->yTop;

	if (abs(diffX) <= abs(diffY)) {
		if (_ws->yTop > *targetY)
			*coord3 = -*coord3;

		if (diffY)
			getSharedData()->setSceneOffsetAdd((int16)Common::Rational(*coord3 * diffX, diffY).toInt());

		if (param != nullptr && abs(diffY) <= abs(*coord3)) {
			*targetX = -1;
			*param = 0;
			return true;
		}
	} else {
		if (_ws->xLeft > *targetX)
			*coord3 = -*coord3;

		getSharedData()->setSceneOffsetAdd((int16)Common::Rational(*coord3 * diffY, diffX).toInt());

		if (param != nullptr && abs(diffX) <= abs(*coord3)) {
			*targetX = -1;
			return true;
		}
	}

	return false;
}


int32 Scene::findActionArea(ActionAreaType type, const Common::Point &pt, bool highlight) {
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

			if (!(area->flags & 1))
				continue;

			if (g_debugPolygons && highlight) {
				// Highlight each polygon as it gets checked for action
				debugHighlightPolygon(area->polygonIndex);
			}

			bool found = false;

			// Iterate over flagNum
			for (uint32 j = 0; j < 10; j++) {
				if (!area->flagNums[j])
					break;                 // We stop as soon as a flag is 0

				bool flagSet = false;
				if (area->flagNums[j] <= 0)
					flagSet = _vm->isGameFlagNotSet((GameFlag)-area->flagNums[j]);
				else
					flagSet = _vm->isGameFlagSet((GameFlag)area->flagNums[j]);

				if (!flagSet) {
					found = true;
					break;
				}
			}

			if (!found && _polygons->get(area->polygonIndex).contains(pt))
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
					continue;    // We skip over null flags

				bool flagSet = false;
				if (area->flagNums[j] <= 0)
					flagSet = _vm->isGameFlagNotSet((GameFlag)-area->flagNums[j]);
				else
					flagSet = _vm->isGameFlagSet((GameFlag)area->flagNums[j]);

				if (!flagSet) {
					found = true;
					break;
				}
			}

			if (!found && _polygons->get(area->polygonIndex).contains(pt))
				return i;
		}
		break;
	}

	return -1;
}

void Scene::changePlayer(ActorIndex index) {
	if (!_ws)
		error("[Scene::changePlayer] WorldStats not initialized properly!");

	switch (index) {
	default:
		if (_ws->chapter == kChapter9) {
			changePlayerUpdate(index);

			getActor(index)->show();
		}

		getSharedData()->setPlayerIndex(index);
		break;

	case 1:
		if (_ws->chapter == kChapter9) {
			changePlayerUpdate(index);

			getScreen()->setPalette(_ws->graphicResourceIds[0]);
			_ws->currentPaletteId = _ws->graphicResourceIds[0];
			getScreen()->setGammaLevel(_ws->graphicResourceIds[0]);
			_vm->setGameFlag(kGameFlag635);
			_vm->clearGameFlag(kGameFlag636);
			_vm->clearGameFlag(kGameFlag637);

			getActor(index)->show();
		}

		getSharedData()->setPlayerIndex(index);
		break;

	case 2:
		if (_ws->chapter == kChapter9) {
			changePlayerUpdate(index);

			getScreen()->setPalette(_ws->graphicResourceIds[1]);
			_ws->currentPaletteId = _ws->graphicResourceIds[1];
			getScreen()->setGammaLevel(_ws->graphicResourceIds[1]);
			_vm->setGameFlag(kGameFlag636);
			_vm->clearGameFlag(kGameFlag635);
			_vm->clearGameFlag(kGameFlag637);

			getActor(index)->show();
		}

		getSharedData()->setPlayerIndex(index);
		break;

	case 3:
		if (_ws->chapter == kChapter9) {
			changePlayerUpdate(index);

			getScreen()->setPalette(_ws->graphicResourceIds[2]);
			_ws->currentPaletteId = _ws->graphicResourceIds[2];
			getScreen()->setGammaLevel(_ws->graphicResourceIds[2]);
			_vm->setGameFlag(kGameFlag637);
			_vm->clearGameFlag(kGameFlag635);
			_vm->clearGameFlag(kGameFlag636);

			getActor(index)->show();
		}

		getActor(index)->show();
		getSharedData()->setPlayerIndex(index);
		break;

	case 666:
		getScreen()->setupTransTables(3, _ws->graphicResourceIds[50], _ws->graphicResourceIds[49], _ws->graphicResourceIds[48]);

		// Save scene data
		getSharedData()->saveCursorResources((ResourceId *)&_ws->cursorResources, sizeof(_ws->cursorResources));
		getSharedData()->saveSceneFonts(_ws->font1, _ws->font2, _ws->font3);
		getSharedData()->saveSmallCursor(_ws->smallCurUp, _ws->smallCurDown);
		getSharedData()->saveEncounterFrameBackground(_ws->encounterFrameBg);

		// Setup new values
		for (uint32 i = 0; i < 11; i++)
			_ws->cursorResources[i] = _ws->graphicResourceIds[20 + i];

		_ws->font1 = _ws->graphicResourceIds[35];
		_ws->font2 = _ws->graphicResourceIds[37];
		_ws->font3 = _ws->graphicResourceIds[36];

		_ws->smallCurUp = _ws->graphicResourceIds[33];
		_ws->smallCurDown = _ws->graphicResourceIds[34];
		_ws->encounterFrameBg = _ws->graphicResourceIds[32];
		break;

	case 667:
		getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);

		// Load scene data
		getSharedData()->loadCursorResources((ResourceId *)&_ws->cursorResources, sizeof(_ws->cursorResources));
		getSharedData()->loadSceneFonts(&_ws->font1, &_ws->font2, &_ws->font3);
		getSharedData()->loadSmallCursor(&_ws->smallCurUp, &_ws->smallCurDown);
		getSharedData()->loadEncounterFrameBackground(&_ws->encounterFrameBg);

		// Reset cursor
		getCursor()->set(_ws->cursorResources[kCursorResourceMagnifyingGlass], 0, kCursorAnimationNone);
		break;

	case 668:
		getActor(11)->setPosition(2300, 100, kDirectionN, 0);
		getSharedData()->setChapter2Counter(6, 0);
		getSharedData()->setFlag(kFlag1, false);
		break;
	}
}

void Scene::changePlayerUpdate(ActorIndex index) {
	Actor *actor = getActor(index);
	Actor *player = getActor();

	actor->setPosition(player->getPoint1()->x + player->getPoint2()->x, player->getPoint1()->y + player->getPoint2()->y, player->getDirection(), 0);
	player->hide();

	actor->inventory.copyFrom(player->inventory);
}

//////////////////////////////////////////////////////////////////////////
// Scene drawing
//////////////////////////////////////////////////////////////////////////
void Scene::preload() {
	if (!Config.showSceneLoading || _vm->checkGameVersion("Demo"))
		return;

	SceneTitle *title = new SceneTitle(_vm);
	getCursor()->hide();
	title->load();

	do {
		title->update(_vm->getTick());

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

	if (getSharedData()->getFlag(kFlagSkipDrawScene)) {
		_vm->screen()->fillRect(0, 0, 640, 480, 0);
		getCursor()->hide();

		return false;
	}

	// Draw scene background
	getScreen()->draw(_ws->backgroundImage, 0, Common::Point(-_ws->xLeft, -_ws->yTop), kDrawFlagNone, false);

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
	if (player->getStatus() == kActorStatusShowingInventory || player->getStatus() == kActorStatus10)
		player->drawInventory();
	else
		player->setNumberFlag01(0);

	_vm->screen()->drawGraphicsInQueue();

	// Show debug information
	if (g_debugScrolling)
		debugScreenScrolling();
	if (g_debugActors)
		debugShowActors();
	if (g_debugPolygons)
		debugShowPolygons();
	if (g_debugPolygonIndex)
		debugHighlightPolygon(g_debugPolygonIndex);
	if (g_debugObjects)
		debugShowObjects();
	if (g_debugSceneRects)
		debugShowSceneRects();

	return false;
}

bool Scene::updateListCompare(const UpdateItem &item1, const UpdateItem &item2) {
	return (item1.priority < item2.priority);
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
			Common::Point sum = *actor->getPoint1() + *actor->getPoint2();

			int16 bottomRight = actor->getPoint1()->y + actor->getBoundingRect()->bottom + 4;

			if (_ws->chapter == kChapter11 && _updateList[i].index != getSharedData()->getPlayerIndex())
				bottomRight += 20;

			// Our actor rect
			Common::Rect actorRect(actor->getPoint1()->x, actor->getPoint1()->y, actor->getPoint1()->x + actor->getBoundingRect()->right, bottomRight);

			// Process objects
			for (uint32 j = 0; j < _ws->objects.size(); j++) {
				Object *object = _ws->objects[j];

				// Skip hidden objects
				if (!object->isOnScreen())
					continue;

				// Check that the rects are contained
				if (!rectIntersect(object->x,
				                   object->y,
				                   object->x + object->getBoundingRect()->right,
				                   object->y + object->getBoundingRect()->bottom,
				                   actor->getPoint1()->x,
				                   actor->getPoint1()->y,
				                   actor->getPoint1()->x + actor->getBoundingRect()->right,
				                   bottomRight)) {

					if ((BYTE1(object->flags) & kObjectFlag20) && !(BYTE1(object->flags) & kObjectFlag80))
						BYTE1(object->flags) = BYTE1(object->flags) | kObjectFlag40;

					continue;
				}

				// Check if it intersects with either the object rect or the related polygon
				bool isMasked = false;
				if (object->flags & kObjectFlag2) {
					isMasked = !pointBelowLine(sum, *object->getRect());
				} else if (object->flags & kObjectFlag40) {
					Polygon poly = _polygons->get(object->getPolygonIndex());
					isMasked = poly.contains(sum);
				}

				// Adjust object flags
				if ((BYTE1(object->flags) & kObjectFlag80) || isMasked) {
					if (BYTE1(object->flags) & kObjectFlag20)
						BYTE1(object->flags) = (BYTE1(object->flags) & kObjectFlagBF) | kObjectFlag80;
				} else {
					if (BYTE1(object->flags) & kObjectFlag20) {
						BYTE1(object->flags) = BYTE1(object->flags) | kObjectFlag40;
					}
				}

				if (LOBYTE(object->flags) & kObjectFlag4) {
					if (isMasked) {
						if (actor->flags & kActorFlagMasked)
							error("[Scene::processUpdateList] Assigning mask to masked character (%s)", actor->getName());

						// We are masked by the object!
						actor->setObjectIndex(j);
						actor->flags |= kActorFlagMasked;
					}
				} else {
					if (isMasked) {
						if (actor->getPriority() < object->getPriority()) {
							actor->setField934(1);
							actor->setPriority(object->getPriority() + 3);

							if (i > 0) {
								// Update actor priority up to current index
								for (uint32 k = 0;  k < i; k++) {
									Actor *previousActor = getWorld()->actors[_updateList[k].index];

									if (getWorld()->chapter != kChapter2 || previousActor->getField944() != 1) {
										if (previousActor->getPriority() == actor->getPriority())
											actor->setPriority(actor->getPriority() - 1);
									}
								}
							}
						}
					} else {
						if (actor->getPriority() > object->getPriority() || actor->getPriority() == 1) {
							actor->setField934(1);
							actor->setPriority(object->getPriority() - 1);

							if (i > 0) {

								// Update actor priority up to current index
								for (uint32 k = 0;  k < i; k++) {
									Actor *previousActor = getWorld()->actors[_updateList[k].index];

									if (previousActor->getField944() != 1 && previousActor->getField944() != 4) {
										Actor *actorCheck = getWorld()->actors[k];

										if (rectIntersect(actorCheck->getPoint1()->x,
										                  actorCheck->getPoint1()->y,
										                  actorCheck->getPoint1()->x + actorCheck->getBoundingRect()->bottom,
										                  actorCheck->getPoint1()->y + actorCheck->getBoundingRect()->right,
										                  actor->getPoint1()->x,
										                  actor->getPoint1()->y,
										                  actor->getPoint1()->x + actor->getBoundingRect()->right,
										                  bottomRight)) {
											if (previousActor->getPriority() == actor->getPriority())
												actor->setPriority(actor->getPriority() - 1);
										}
									}
								}
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

			if (actor->shouldInvertPriority())
				getActor(actor->getNextActorIndex())->setPriority(-actor->getPriority());
		}
	} // end processing actors


	// Go through the list from the end
	if (_updateList.size() > 1) {
		for (int i = _updateList.size() - 1; i >= 0; --i) {
			// Get the actor
			Actor *actor1 = getActor(_updateList[i].index);

			// Skip hidden actors
			if (!actor1->isVisible())
				continue;

			if (actor1->getField944() != 1 && actor1->getField944() != 4) {
				// Process all previous actors
				if (i > 0) {
					for (int j = i - 1; j >= 0; --j) {
						Actor *actor2 = getActor(_updateList[j].index);

						if (actor2->getField944() != 1 && actor2->getField944() != 4) {

							// Process priorities
							if (actor2->getPriority() <= actor1->getPriority()) {
								if (rectIntersect(actor1->getPoint1()->x,
								                  actor1->getPoint1()->y,
								                  actor1->getPoint1()->x + actor1->getBoundingRect()->right,
								                  actor1->getPoint1()->y + actor1->getBoundingRect()->bottom,
								                  actor2->getPoint1()->x,
								                  actor2->getPoint1()->y,
								                  actor2->getPoint1()->x + actor2->getBoundingRect()->right,
								                  actor2->getPoint1()->y + actor2->getBoundingRect()->bottom)) {
									actor2->setPriority(actor1->getPriority() + 1);
								}
							}
						}
					}
				}
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
	Actor *actor0 = getActor(0);
	Actor *actor  = getActor(index);

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

void Scene::drawRain() {
	if (!_ws)
		error("[Scene::drawRain] WorldStats not initialized properly!");

	if (getSharedData()->getFlag(kFlagSkipDrawScene))
		return;

	for (int16 y = 0; y < 512; y = y + 64) {
		for (int16 x = 0; x < 704; x = x + 64) {
			getScreen()->draw(MAKE_RESOURCE(kResourcePackShared, 58), (uint32)_chapter5RainFrameIndex, Common::Point(x + (_ws->xLeft % 64) / 8, y + (_ws->yTop % 64) / 8));
		}
	}

	_chapter5RainFrameIndex = (_chapter5RainFrameIndex + 1) % (int32)GraphicResource::getFrameCount(_vm, MAKE_RESOURCE(kResourcePackShared, 58));
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
	else if (getCursor()->position().x > (640 - SCREEN_EDGES) && _ws->xLeft <= (rect.width() - (640 + SCROLL_STEP)))
		_ws->xLeft += SCROLL_STEP;

	// Vertical scrolling
	if (getCursor()->position().y < SCREEN_EDGES && _ws->yTop >= SCROLL_STEP)
		_ws->yTop -= SCROLL_STEP;
	else if (getCursor()->position().y > (480 - SCREEN_EDGES) && _ws->yTop <= (rect.height() - (480 + SCROLL_STEP)))
		_ws->yTop += SCROLL_STEP;
}

// WALK REGION DEBUG
void Scene::debugShowWalkRegion(Polygon *poly) {
	Graphics::Surface surface;
	surface.create((uint16)poly->boundingRect.width() + 1,
	               (uint16)poly->boundingRect.height() + 1,
	               Graphics::PixelFormat::createFormatCLUT8());

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

	for (uint32 p = 0; p < _polygons->size(); p++)
		debugShowPolygon(p);
}

void Scene::debugShowPolygon(uint32 index, uint32 color) {
	if (!_polygons)
		error("[Scene::debugShowPolygon] Polygons not initialized properly");

	if (index >= _polygons->size() - 1)
		return;

	Graphics::Surface surface;
	Polygon poly = _polygons->get(index);
	surface.create((uint16)poly.boundingRect.width() + 1,
		           (uint16)poly.boundingRect.height() + 1,
		           Graphics::PixelFormat::createFormatCLUT8());

	// Draw all lines in Polygon
	for (uint32 i = 0; i < poly.count(); i++) {
		surface.drawLine(poly.points[i].x - poly.boundingRect.left,
			                poly.points[i].y - poly.boundingRect.top,
			                poly.points[(i+1) % poly.count()].x - poly.boundingRect.left,
			                poly.points[(i+1) % poly.count()].y - poly.boundingRect.top,
			                color);
	}

	getScreen()->copyToBackBufferClipped(&surface, poly.boundingRect.left, poly.boundingRect.top);

	surface.free();
}

void Scene::debugHighlightPolygon(uint32 index) {
	debugShowPolygon(index, 0x12);
	getScreen()->copyBackBufferToScreen();
	g_system->updateScreen();
}

// SCENE RECTS DEBUG
void Scene::debugShowSceneRects() {
	if (!_ws)
		error("[Scene::debugShowObjects] WorldStats not initialized properly!");

	for (uint32 i = 0; i < ARRAYSIZE(_ws->sceneRects); i++)
		getScreen()->drawRect(_ws->sceneRects[i]);
}

// OBJECT DEBUGGING
void Scene::debugShowObjects() {
	if (!_ws)
		error("[Scene::debugShowObjects] WorldStats not initialized properly!");

	for (uint32 p = 0; p < _ws->objects.size(); p++) {
		Graphics::Surface surface;
		Object *object = _ws->objects[p];

		if (object->isOnScreen()) {
			surface.create((uint16)object->getBoundingRect()->width() + 1,
			               (uint16)object->getBoundingRect()->height() + 1,
			               Graphics::PixelFormat::createFormatCLUT8());
			surface.frameRect(*object->getBoundingRect(), 0x22);
			getScreen()->copyToBackBufferClipped(&surface, object->x, object->y);
		}

		surface.free();
	}
}

// ACTOR DEBUGGING
void Scene::debugShowActors() {
	if (!_ws)
		error("[Scene::debugShowActors] WorldStats not initialized properly!");

	for (uint32 p = 0; p < _ws->actors.size(); p++) {
		Graphics::Surface surface;
		Actor *a = _ws->actors[p];

		if (a->isOnScreen()) {
			surface.create((uint16)a->getBoundingRect()->width() + 1,
			               (uint16)a->getBoundingRect()->height() + 1,
			               Graphics::PixelFormat::createFormatCLUT8());
			surface.frameRect(*a->getBoundingRect(), 0x128);
			getScreen()->copyToBackBufferClipped(&surface, a->getPoint1()->x, a->getPoint1()->y);
		}

		surface.free();
	}
}

} // end of namespace Asylum
