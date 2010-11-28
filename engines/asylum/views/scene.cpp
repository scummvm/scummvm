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

#include "asylum/resources/actionlist.h"
#include "asylum/resources/actor.h"
#include "asylum/resources/encounters.h"
#include "asylum/resources/object.h"
#include "asylum/resources/polygons.h"
#include "asylum/resources/special.h"
#include "asylum/resources/worldstats.h"

#include "asylum/system/config.h"
#include "asylum/system/cursor.h"
#include "asylum/system/graphics.h"
#include "asylum/system/screen.h"
#include "asylum/system/speech.h"
#include "asylum/system/text.h"

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

Scene::Scene(AsylumEngine *engine): _vm(engine), _ev(NULL),
	_actions(NULL), _special(NULL), _speech(NULL), _title(NULL), _polygons(NULL), _ws(NULL) {

	// Initialize data
	_packId = kResourcePackInvalid;
	_playerActorIdx = 0;
	_titleLoaded = false;
	_walking = false;
	_leftClick = false;
	_rightButton = false;
	_isActive = false;
	_globalDirection = kDirectionN;

	// Graphics
	_bgResource = NULL;
	_background = NULL;
}

Scene::~Scene() {
	// Unload the associated resources
	getResource()->unload(_packId);

	delete _actions;
	//delete _blowUp;
	delete _special;
	delete _speech;
	delete _title;
	delete _polygons;
	delete _ws;

	delete _bgResource;
	_background = NULL;

	// Zero passed pointers
	_vm = NULL;
	_ev = NULL;
}

void Scene::enter(ResourcePackId packId) {
	_vm->setGameFlag(kGameFlagScriptProcessing);

	getCursor()->hide();

	_playerActorIdx = 0;

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
	getCursor()->set(_ws->curMagnifyingGlass);
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
		_actions->queueScript(_ws->scriptIndex, 0);

	// Clear the graphic queue (FIXME: not sure why we need to do this again)
	getScreen()->clearGraphicsInQueue();

	// Load transparency tables
	getScreen()->setupTransTables(3, _ws->cellShadeMask1, _ws->cellShadeMask2, _ws->cellShadeMask3);
	getScreen()->selectTransTable(1);

	// Setup font
	getText()->loadFont(_ws->font1);

	// Play scene intro dialog
	playIntroSpeech();

	// Set actor type
	_ws->actorType = actorType[_ws->chapter];

	// Play intro music
	ResourceId musicId = kResourceNone;
	if (_ws->musicCurrentResourceIndex != kMusicStopped && _ws->chapter != kChapter1)
		musicId = MAKE_RESOURCE(kResourcePackMusic, _ws->musicCurrentResourceIndex);

	getSound()->playMusic(musicId);

	// Update global values
	_vm->lastScreenUpdate = 1;
	getSharedData()->setFlag(kFlagScene1, true);

	player->setLastScreenUpdate(_vm->screenUpdateCount);
	player->enable();

	if (_ws->chapter == kChapter9) {
		changePlayer(1);
		_ws->nextPlayer = kActorInvalid;
	}

	activate();
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

	_ws = new WorldStats(fd, this);
	_polygons = new Polygons(fd);
	_actions = new ActionList(_vm);
	_actions->load(fd);

	fd->close();
	delete fd;

	_speech = new Speech(_vm);

	// TODO
	// This will have to be re-initialized elsewhere due to
	// the title screen overwriting the font
	_vm->text()->loadFont(_ws->font1);

	_bgResource    = new GraphicResource(_vm, _ws->backgroundImage);
	//_blowUp        = 0;
	_background    = NULL;
	_leftClick     = false;
	_rightButton   = false;
	_isActive      = false;
	//_skipDrawScene = false;

	g_debugPolygons  = 0;
	g_debugObjects  = 0;
	g_debugScrolling = 0;

	//_globalX = _globalY = 0;

	// TODO figure out what field_120 is used for
	_ws->field_120 = -1;

	for (uint32 a = 0; a < _ws->actors.size(); a++)
		_ws->actors[a]->setLastScreenUpdate(_vm->getTick());

	// TODO: init action list

	_title = new SceneTitle(_vm);
	_titleLoaded = false;

	_special = new Special(_vm);
}

Actor* Scene::getActor(ActorIndex index) {
	if (!_ws)
		error("[Scene::getActor] WorldStats not initialized properly!");

	ActorIndex computedIndex =  (index != -1) ? index : _playerActorIdx;

	if (computedIndex < 0 || computedIndex >= (int16)_ws->actors.size())
		error("[Scene::getActor] Invalid actor index: %d ([0-%d] allowed)", computedIndex, _ws->actors.size() - 1);

	return _ws->actors[computedIndex];
}

void Scene::setScenePosition(int x, int y) {
	if (!_ws)
		error("[Scene::setScenePosition] WorldStats not initialized properly!");

	if (!_bgResource)
		error("[Scene::setScenePosition] Scene resources not initialized properly!");

	GraphicFrame *bg = _bgResource->getFrame(0);
	//_startX = x;
	//_startY = y;

	int32 *targetX = &_ws->coordinates[0];
	int32 *targetY = &_ws->coordinates[1];

	*targetX = x;
	*targetY = y;

	if (*targetX < 0)
		*targetX = 0;
	if (*targetX > (bg->surface.w - 640))
		*targetX = bg->surface.w - 640;


	if (*targetY < 0)
		*targetY = 0;
	if (*targetY > (bg->surface.h - 480))
		*targetY = bg->surface.h - 480;
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	if (!_ws)
		error("[Scene::handleEvent] WorldStats not initialized properly!");

	if (!_title)
		error("[Scene::handleEvent] Scene title not initialized properly!");

	_ev = event;

	switch (_ev->type) {

	case Common::EVENT_MOUSEMOVE:
		if (getCursor())
			getCursor()->move(_ev->mouse.x, _ev->mouse.y);
		break;

	case Common::EVENT_LBUTTONUP:
		//if (_actions->doesAllowInput())
			_leftClick = true;
		break;

	case Common::EVENT_RBUTTONUP:
		//if (_actions->doesAllowInput()) {
			// TODO This isn't always going to be the magnifying glass
			// Should check the current pointer region to identify the type
			// of cursor to use
			getCursor()->set(_ws->curMagnifyingGlass);
			_rightButton    = false;
		//}
		break;

	case Common::EVENT_RBUTTONDOWN:
		//if (_actions->doesAllowInput())
			_rightButton = true;
		break;

	default:
		break;

	}

	// FIXME just updating because a left click event
	// is caught causes animation speeds to change. This needs
	// to be addressed
	if (!doUpdate)
		return;

	if (Config.showSceneLoading) {
		if (!_titleLoaded) {
			_title->update(_vm->getTick());
			if (_title->loadingComplete()) {
				_titleLoaded = true;
                activate();
			}
			return;
		}
	}

	if (update())
		return;

	// TODO: check game quality
	drawScene();

	//TODO: other process stuffs from sub 0040AE30


	if (_speech->getSoundResourceId() != 0) {
		if (_vm->sound()->isPlaying(_speech->getSoundResourceId())) {
			_speech->prepareSpeech();
		} else {
			_speech->resetResourceIds();
			_vm->clearGameFlag(kGameFlag219);
		}
	}
}

void Scene::activate() {
	if (!_ws)
		error("[Scene::activate] WorldStats not initialized properly!");

	if (!_bgResource)
		error("[Scene::activate] Scene resources not initialized properly!");

	//////////////////////////////////////////////////////////////////////////
	// FIXME: get rid of this?

	_isActive = true;
	getScreen()->setPalette(_ws->currentPaletteId);
	getScreen()->draw(_ws->backgroundImage, 0, _ws->xLeft, _ws->yTop, 0);
}

bool Scene::update() {
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
	MESURE_TICKS(updateMouse);
	MESURE_TICKS(updateActors);
	MESURE_TICKS(updateObjects);
	MESURE_TICKS(updateAmbientSounds);
	MESURE_TICKS(updateMusic);
	MESURE_TICKS(updateScreen);

	// Update Debug
	if (g_debugPolygons)
		debugShowPolygons();
	if (g_debugObjects)
		debugShowObjects();

	return _actions->process();
}

void Scene::updateMouse() {
	Common::Rect  actorPos;
	Common::Point pt;
	Actor *act = getActor();

	// The code below was from
	// .text:0040A1B0 getCharacterScreenPosition()
	// which was only ever called at this point, so
	// inlining it for simplicity
	pt.x = (int16)(act->getPoint1()->x -_ws->xLeft);
	pt.y = (int16)(act->getPoint1()->y -_ws->yTop);

	if (_packId != 2 || _playerActorIdx != 10) {
		actorPos.left   = pt.x + 20;
		actorPos.top    = pt.y;
		actorPos.right  = (int16)(pt.x + 2 * act->getPoint2()->x);
		actorPos.bottom = (int16)(pt.y + act->getPoint2()->y);
	} else {
		actorPos.left   = pt.x + 50;
		actorPos.top    = pt.y + 60;
		actorPos.right  = (int16)(pt.x + getActor(10)->getPoint2()->x + 10);
		actorPos.bottom = (int16)(pt.y + getActor(10)->getPoint2()->y - 20);
	}

	ActorDirection dir = kDirectionInvalid;
	bool done = false;

	if (getCursor()->position().x < actorPos.left) {
		if (getCursor()->position().y >= actorPos.top) {
			if (getCursor()->position().y > actorPos.bottom) {
				if (act->getDirection() == 2) {
					if (getCursor()->position().y - actorPos.bottom > 10)
						dir = kDirectionSO;
				} else {
					if (act->getDirection() == kDirectionS) {
						if (actorPos.left - getCursor()->position().x > 10)
							dir = kDirectionSO;
					} else {
						dir = kDirectionSO;
					}
				}
			} else {
				if (act->getDirection() == 1) {
					if (getCursor()->position().y - actorPos.top > 10)
						dir = kDirectionO;
				} else {
					if (act->getDirection() == kDirectionSO) {
						if (actorPos.bottom - getCursor()->position().y > 10)
							dir = kDirectionO;
					} else {
						dir = kDirectionO;
					}
				}
			}
		} else {
			if (act->getDirection()) {
				if (act->getDirection() == kDirectionO) {
					if (actorPos.top - getCursor()->position().y > 10)
						dir = kDirectionNO;
				} else {
					dir = kDirectionNO;
				}
			} else {
				if (actorPos.left - getCursor()->position().x > 10)
					dir = kDirectionNO;
			}
		}
		done = true;
	}

	if (!done && getCursor()->position().x <= actorPos.right) {
		if (getCursor()->position().y >= actorPos.top) {
			if (getCursor()->position().y > actorPos.bottom) {
				if (act->getDirection() == kDirectionSO) {
					if (getCursor()->position().x - actorPos.left > 10)
						dir = kDirectionS;
				} else {
					if (act->getDirection() == kDirectionSE) {
						if (actorPos.right - getCursor()->position().x > 10)
							dir = kDirectionS;
					} else {
						dir = kDirectionS;
					}
				}
			}
		} else {
			if (act->getDirection() == kDirectionNO) {
				if (getCursor()->position().x - actorPos.left > 10)
					dir = kDirectionN;
			} else {
				if (act->getDirection() == kDirectionNE) {
					if (actorPos.right - getCursor()->position().x > 10)
						dir = kDirectionN;
				} else {
					dir = kDirectionN;
				}
			}
		}
		done = true;
	}

	if (!done && getCursor()->position().y < actorPos.top) {
		if (act->getDirection()) {
			if (act->getDirection() == kDirectionE) {
				if (actorPos.top - getCursor()->position().y > 10)
					dir = kDirectionNE;
			} else {
				dir = kDirectionNE;
			}
		} else {
			if (getCursor()->position().x - actorPos.right > 10)
				dir = kDirectionNE;
		}
		done = true;
	}

	if (!done && getCursor()->position().y <= actorPos.bottom) {
		if (act->getDirection() == kDirectionSE) {
			if (actorPos.bottom - getCursor()->position().y > 10)
				dir = kDirectionE;
		} else {
			if (act->getDirection() == kDirectionNE) {
				if (getCursor()->position().y - actorPos.top > 10)
					dir = kDirectionE;
			} else {
				dir = kDirectionE;
			}
		}
		done = true;
	}

	if (!done && act->getDirection() == kDirectionS) {
		if (getCursor()->position().x - actorPos.right <= 10)
			done = true;
		if (!done)
			dir = kDirectionSE;
	}

	if (!done && (act->getDirection() != kDirectionE || getCursor()->position().y - actorPos.bottom > 10))
		dir = kDirectionSE;

	handleMouseUpdate(dir, actorPos);

	if (dir >= 0) {
		if (act->getStatus() == 1 || act->getStatus() == 12)
			act->setDirection(dir);
	}
}

void Scene::handleMouseUpdate(int direction, Common::Rect rect) {
	int16 rlimit = rect.right - 10;
	ResourceId newGraphicResourceId;
	HitType type = kHitNone;

	// TODO if encounter_flag03
	if (0 && getCursor()->graphicResourceId != _ws->curTalkNPC)
		getCursor()->set(_ws->curTalkNPC, 0, 2);

	Actor *act = getActor(); // get the player actor reference

	// XXX field_11 seems to have something to do with
	// whether the event manager is handling a right mouse down
	// event
	if (getCursor()->field_11 & 2) {
		if (act->getStatus() == 1 || act->getStatus() == 12) {
			if (direction >= 0) {
				newGraphicResourceId = (ResourceId)(_ws->curScrollUp + direction);
				getCursor()->set(newGraphicResourceId, 0, 2);
			}
		}
	}

	if (act->getStatus() == 6 || act->getStatus() == 10) {
		newGraphicResourceId = _ws->curHand;
		getCursor()->set(newGraphicResourceId, 0, 2);
	} else {
		if (act->getField638()) {
			if (getCursor()->position().x >= rect.left && getCursor()->position().x <= rlimit &&
				getCursor()->position().y >= rect.top  && getCursor()->position().y <= rect.bottom &&
				hitTestActor(getCursor()->position())) {
				// TODO LOTS of work here, because apparently we need to use
				// field_638 as an index into _ws->field_D6AC8, which is not
				// yet defined as part of worldstats, but according to IDA, is:
				// 000D6A88 field_D6A88     dd 16 dup(?)
				// so this is an array that is initialized as part of the scene
				// loading process. Need to investigate further ...
				warning("Something...");
			} else {
				// TODO pass a reference to hitType so it can be populated by
				// hitTestScene
				newGraphicResourceId = hitTestScene(getCursor()->position(), type);
				if (newGraphicResourceId != (ResourceId)-1) {
					warning ("Can't set mouse cursor, field_D6AC8 not handled ... yet");
					// TODO
					// check if _ws->field_D6AC8[act->field_638] != newGraphicResourceId
					// if false, set mouse cursor
				} else {
					// TODO _ws->field_D6B08 stuff, then set cursor
					warning ("Can't set mouse cursor, field_D6B08 not handled ... yet");
				}
			}
			return; // return result;
		}
		int32 targetIdx = hitTest(getCursor()->position(), type);

		//printf ("Mouse X(%d)/Y(%d) = %d\n", getCursor()->position().x, getCursor()->position().y, type);
		if (getCursor()->position().x >= rect.left && getCursor()->position().x <= rlimit &&
			getCursor()->position().y >= rect.top  && getCursor()->position().y <= rect.bottom &&
			hitTestActor(getCursor()->position())) {
			if (act->getReaction(0)) {
				getCursor()->set(_ws->curGrabPointer, 0, 2);
				return;
			}
		}
		if (targetIdx == -1) {
			if (_ws->chapter != kChapter2 || _playerActorIdx != 10) {
				if (getCursor()->flags)
					getCursor()->set(_ws->curMagnifyingGlass, 0, 2);
			} else {
				if (getCursor()->flags)
					getCursor()->set(_ws->curTalkNPC2, 0, 2);
			}
		} else {
			int32 targetUpdateType = 0;
			switch (type) {
			case kHitActionArea:
				targetUpdateType = _ws->actions[targetIdx]->actionType;
				break;
			case kHitObject:
				targetUpdateType = _ws->objects[targetIdx]->actionType;
				break;
			case kHitActor:
				targetUpdateType = getActor(targetIdx)->getStatus();
				break;
			default:
				// TODO LOBYTE(hitType)
				break;
			}

			if (targetUpdateType & 1 && getCursor()->flags != 2) {
				getCursor()->set(_ws->curMagnifyingGlass, 0, 2);
			} else {
				if (targetUpdateType & 4) {
					getCursor()->set(_ws->curHand, 0, 2);
				} else {
					if (targetUpdateType & 2) {
						getCursor()->set(_ws->curTalkNPC, 0, 2);
					} else {
						if (targetUpdateType & 0x10 && getCursor()->flags != 2) {
							getCursor()->set(_ws->curTalkNPC2, 0, 2);
						} else {
							if (_ws->chapter != kChapter2 && _playerActorIdx != 10) {
								getCursor()->set(_ws->curMagnifyingGlass, 0, 0);
							} else {
								if (getCursor()->flags)
									getCursor()->set(_ws->curTalkNPC2, 0, 2);
							}
						}
					}
				}
			}
		}
	}


}

int32 Scene::hitTestObject(const Common::Point pt) {
	if (!_ws)
		error("[Scene::hitTestObject] WorldStats not initialized properly!");

	int32 targetIdx = -1;
	for (uint32 i = 0; i < _ws->objects.size(); i++) {
		Object *object = _ws->objects[i];
		if (object->isOnScreen())
			if (object->getPolygonIndex())
				if (hitTestPixel(object->getResourceId(), object->getFrameIndex(), pt.x, pt.y, object->flags & 0x1000)) {
					targetIdx = i;
					break;
				}
	}
	return targetIdx;
}

int32 Scene::hitTest(const Common::Point pt, HitType &type) {
	type = kHitNone;
	int32 targetIdx = hitTestObject(pt);
	if (targetIdx == -1) {
		targetIdx = hitTestActionArea(pt);
		if (targetIdx == -1) {
			if (hitTestActor(pt)) {
				targetIdx = _playerActorIdx;
				type = kHitActor;
			}
		} else {
			type = kHitActionArea;
		}
	} else {
		type = kHitObject;
	}
	return targetIdx;
}

int32 Scene::hitTestActionArea(const Common::Point pt) {
	int32 targetIdx = findActionArea(Common::Point(_ws->xLeft + pt.x, _ws->yTop + pt.y));

	if ( targetIdx == -1 || !(_ws->actions[targetIdx]->actionType & 0x17))
		targetIdx = -1;

	return targetIdx;
}

int32 Scene::findActionArea(const Common::Point pt) {
	if (!_ws)
		error("[Scene::findActionArea] WorldStats not initialized properly!");

	if (!_polygons)
		error("[Scene::findActionArea] Polygons not initialized properly!");

	// TODO
	// This is a VERY loose implementation of the target
	// function, as this doesn't do any of the flag checking
	// the original did
	int32 targetIdx = -1;
	for (uint32 i = 0; i < _ws->actions.size(); i++) {
		ActionArea *a = _ws->actions[i];
		PolyDefinitions p = _polygons->entries[a->polyIdx];
		if (p.contains(pt.x, pt.y)) {
			targetIdx = i;
			break;
		}
	}
	return targetIdx;
}

int32 Scene::isInActionArea(const Common::Point &pt, ActionArea *area) {
	error("[Scene::isInActionArea] Not implemented!");
}

ResourceId Scene::hitTestScene(const Common::Point pt, HitType &type) {
	if (!_ws)
		error("[Scene::hitTestScene] WorldStats not initialized properly!");

	int32 top  = pt.x + _ws->xLeft;
	int32 left = pt.y + _ws->yTop;
	type = kHitNone;

	ResourceId result = (ResourceId)findActionArea(Common::Point(top, left));

	if (result != (ResourceId)-1) {
		if (LOBYTE(_ws->actions[result]->actionType) & 8) {
			type = kHitActionArea;
			return result;
		}
	}

	// TODO object and actor checks

	return result;
}

bool Scene::hitTestActor(const Common::Point pt) {
	Actor *act = getActor();
	Common::Point actPos;
	getActorPosition(act, &actPos);

	int32 hitFrame;
	if (act->getFrameIndex() >= act->getFrameCount())
		hitFrame = 2 * act->getFrameIndex() - act->getFrameCount() - 1;
	else
		hitFrame = act->getFrameIndex();

	return hitTestPixel(act->getResourceId(),
		hitFrame,
		pt.x - act->getPoint()->x - actPos.x,
		pt.y - act->getPoint()->y - actPos.y,
		(act->getDirection() >= 0));
}

bool Scene::hitTestPixel(ResourceId resourceId, int32 frame, int16 x, int16 y, bool flipped) {
	// TODO this gets a bit funky with the "flipped" calculations for x intersection
	// The below is a pretty basic intersection test for proof of concept
	return GraphicResource::getFrameRect(_vm, resourceId, frame).contains(x, y);
}

void Scene::changePlayer(ActorIndex index) {
	error("[Scene::changePlayer] not implemented");
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

	for (int32 i = 0; i < _ws->numAmbientSound; i++) {
		bool processSound = true;
		int panning = 0;
		int volume  = 0;
		AmbientSoundItem *snd = &_ws->ambientSounds[i];

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
					// TODO optimize
					// This adjustment only uses the actor at
					// index zero, but it's supposed to loop through
					// all available actors as well (I think)
					volume = calculateVolumeAdjustment(snd, getActor(0));
					if (volume <= 0) {
						if (volume < -10000)
							volume = -10000;
						// TODO setSoundVolume(snd->resourceId, volume);
					} else
						; // TODO setSoundVolume(snd->resourceId, 0);
				}
			} else {
				int loflag = BYTE1(snd->flags);
				if (snd->field_0) {
					; // TODO calculate panning at point
				} else {
					panning = 0;
				}
				if (snd->field_0 == 0) {
					volume = -(snd->field_C ^ 2);
				} else {
					volume = calculateVolumeAdjustment(snd, getActor(0));
					volume += Config.ambientVolume;
				}
				if (loflag & 2) {
					int tmpVol = volume;
					if (vm()->getRandom(10000) < 10) {
						if (snd->field_0) {
							_vm->sound()->playSound(snd->resourceId, false, volume, panning);
						} else {
							// FIXME will this even work?
							tmpVol += (vm()->getRandom(500)) * (((vm()->getRandom(100) >= 50) - 1) & 2) - 1;
							if (tmpVol <= -10000)
								volume = -10000;
							if (volume >= 0)
								tmpVol = 0;
							else
								if (tmpVol <= -10000)
									tmpVol = -10000;
							getSound()->playSound(snd->resourceId, false, tmpVol, vm()->getRandom(20001) - 10000);
						}
					}
				} else {
					if (loflag & 4) {
						// TODO panning array stuff
					}
				}
			}
		} else {
			if (_vm->sound()->isPlaying(snd->resourceId))
				_vm->sound()->stop(snd->resourceId);
		}
	}
}

int32 Scene::calculateVolumeAdjustment(AmbientSoundItem *snd, Actor *act) {
	//int32 x, y;
	if (snd->field_10) {
		/* FIXME properly handle global x/y
		if (g_object_x == -1) {
			x = snd->x - act->getPoint1()->x - act->getPoint2()->x;
			y = snd->y - act->getPoint1()->y - act->getPoint2()->y;
		} else {
			x = snd->x - g_object_x;
			y = snd->y - g_object_y;
		}
		*/

		// FIXME vol = sub_432CA0(x ^ 2 + y ^ 2);
		// Just assigning an arbitrary value for the
		// time being
		int vol = 5000;
		if (100 / snd->field_C)
			vol = vol / (100 / snd->field_C);
		else
			vol = snd->field_C;
		vol = (vol - snd->field_C) ^ 2;
		if (vol <= 10000)
			vol = -vol;
		else
			vol = -10000;

		return vol;
	} else {
		return -(snd->field_C ^ 2);
	}
}

void Scene::updateMusic() {
	//warning("[Scene::updateMusic] not implemented!");
}

void Scene::updateScreen() {
	if (g_debugScrolling) { // DEBUG ScreenScrolling
		debugScreenScrolling(_bgResource->getFrame(0));
	} else {
		updateAdjustScreen();
	}
}

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

	// TODO do palette fade and wait until sound is done
	warning("[Scene::playIntroSpeech] Missing palette fade and wait!");
}

void Scene::updateAdjustScreen() {
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

void Scene::adjustCoordinates(int32 x, int32 y, Common::Point *point) {
	if (!point)
		error("[Scene::adjustCoordinates] Invalid point parameter!");

	if (!_ws)
		error("[Scene::adjustCoordinates] WorldStats not initialized properly!");

	point->x = x;// - _ws->xLeft;
	point->y = y;// - _ws->yTop;
}

//////////////////////////////////////////////////////////////////////////
// Scene drawing
//////////////////////////////////////////////////////////////////////////
int Scene::drawScene() {
	if (!_ws)
		error("[Scene::drawScene] WorldStats not initialized properly!");

	_vm->screen()->clearGraphicsInQueue();

	if (getSharedData()->getSkipDrawScene()) {
		_vm->screen()->clear();
	} else {
		// Draw scene background
		_vm->screen()->draw(_ws->backgroundImage, 0, -_ws->xLeft, -_ws->yTop, 0);

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
	}

	return 1;
}

bool Scene::updateListCompare(const UpdateItem &item1, const UpdateItem &item2) {
	return item1.priority - item2.priority;
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
						adjustCoordinates(object->x, object->y, &point);
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
	error("[Scene::checkVisibleActorsPriority] not implemented");
}

void Scene::adjustActorPriority(ActorIndex index) {
	error("[Scene::adjustActorPriority] not implemented");
}

//////////////////////////////////////////////////////////////////////////
// Helpers
//////////////////////////////////////////////////////////////////////////
bool Scene::pointIntersectsRect(Common::Point point, Common::Rect rect) {
	if (rect.top || rect.left || rect.bottom || rect.right) {
		Common::Rational res((rect.bottom - rect.top) * (point.x - rect.left), rect.right - rect.left);

		return (bool)(point.y > rect.top ? 1 + res.toInt() : res.toInt());
	}

	return true;
}

void Scene::getActorPosition(Actor *actor, Common::Point *pt) {
	if (!_ws)
		error("[Scene::getActorPosition] WorldStats not initialized properly!");

	pt->x = actor->getPoint1()->x - _ws->xLeft;
	pt->y = actor->getPoint1()->y - _ws->yTop;
}
// ----------------------------------
// ---------- SCREEN REGION -----------
// ----------------------------------

void Scene::copyToBackBufferClipped(Graphics::Surface *surface, int x, int y) {
	if (!_ws)
		error("[Scene::copyToBackBufferClipped] WorldStats not initialized properly!");

	Common::Rect screenRect(_ws->xLeft, _ws->yTop, _ws->xLeft + 640, _ws->yTop + 480);
	Common::Rect animRect(x, y, x + surface->w, y + surface->h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate anim rectangle
		animRect.translate(-(int16)_ws->xLeft, -(int16)_ws->yTop);

		int startX = animRect.right  == 640 ? 0 : surface->w - animRect.width();
		int startY = animRect.bottom == 480 ? 0 : surface->h - animRect.height();

		if (surface->w > 640)
			startX = _ws->xLeft;
		if (surface->h > 480)
			startY = _ws->yTop;

		_vm->screen()->copyToBackBufferWithTransparency(
		    ((byte*)surface->pixels) +
		    startY * surface->pitch +
		    startX * surface->bytesPerPixel,
		    surface->pitch,
		    animRect.left,
		    animRect.top,
		    animRect.width(),
		    animRect.height());
	}
}

// ----------------------------------
// ---------- DEBUG REGION -----------
// ----------------------------------

void Scene::debugScreenScrolling(GraphicFrame *bg) {
	if (!_ws)
		error("[Scene::debugScreenScrolling] WorldStats not initialized properly!");

	// Horizontal scrolling
	if (getCursor()->position().x < SCREEN_EDGES && _ws->xLeft >= SCROLL_STEP)
		_ws->xLeft -= SCROLL_STEP;
	else if (getCursor()->position().x > 640 - SCREEN_EDGES && _ws->xLeft <= bg->surface.w - 640 - SCROLL_STEP)
		_ws->xLeft += SCROLL_STEP;

	// Vertical scrolling
	if (getCursor()->position().y < SCREEN_EDGES && _ws->yTop >= SCROLL_STEP)
		_ws->yTop -= SCROLL_STEP;
	else if (getCursor()->position().y > 480 - SCREEN_EDGES && _ws->yTop <= bg->surface.h - 480 - SCROLL_STEP)
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

	copyToBackBufferClipped(&surface, poly->boundingRect.left, poly->boundingRect.top);

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

		copyToBackBufferClipped(&surface, poly.boundingRect.left, poly.boundingRect.top);

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
			copyToBackBufferClipped(&surface, object->x, object->y);
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
			copyToBackBufferClipped(&surface, a->getPoint()->x, a->getPoint()->y);
		}

		surface.free();
	}
}

int Scene::processActor(int *x, int *param) {
	error("[Scene::processActor] not implemented!");
}

void  Scene::updatePalette(int32 param) {
	error("[Scene::updatePalette] not implemented!");
}

void Scene::makeGreyPalette() {
	error("[Scene::makeGreyPalette] not implemented!");
}

void Scene::resetActor0() {
	error("[Scene::resetActor0] not implemented!");
}

} // end of namespace Asylum
