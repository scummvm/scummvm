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

#include "asylum/system/config.h"

#include "asylum/staticres.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugPolygons;
int g_debugBarriers;
int g_debugScrolling;

Scene::Scene(uint8 sceneIdx, AsylumEngine *engine): _vm(engine) {
	_sceneIdx = sceneIdx;

	char filename[10];
	sprintf(filename, SCENE_FILE_MASK, sceneIdx);

	char sceneTag[6];
	Common::File* fd = new Common::File;

	if (!fd->exists(filename))
		error("Scene file doesn't exist %s", filename);

	fd->open(filename);

	if (!fd->isOpen())
		error("Failed to load scene file %s", filename);

	fd->read(sceneTag, 6);

	if (Common::String(sceneTag, 6) != "DFISCN")
		error("The file isn't recognized as scene %s", filename);

	_ws = new WorldStats(fd, this);
	// jump to game polygons data
	fd->seek(0xE8686);
	_polygons = new Polygons(fd);
	// jump to action list data
	fd->seek(0xE868E + _polygons->size * _polygons->numEntries);
	_actions = new ActionList(fd, this);

	fd->close();
	delete fd;

	_speech = new Speech(this);
	_resPack = new ResourcePack(sceneIdx);

	// TODO
	// This will have to be re-initialized elsewhere due to
	// the title screen overwriting the font
	_vm->text()->loadFont(_resPack, _ws->font1);

	char musPackFileName[10];
	sprintf(musPackFileName, MUSIC_FILE_MASK, sceneIdx);
	_musPack = new ResourcePack(musPackFileName);

	_bgResource    = new GraphicResource(_resPack, _ws->backgroundImage);
	//_blowUp        = 0;
	_cursor        = 0;
	_background    = 0;
	_leftClick     = false;
	_rightButton   = false;
	_isActive      = false;
	_skipDrawScene = 0;

	g_debugPolygons  = 0;
	g_debugBarriers  = 0;
	g_debugScrolling = 0;

	_globalX = _globalY = 0;

	// TODO figure out what field_120 is used for
	_ws->field_120 = -1;

	for (int32 a = 0; a < _ws->numActors; a++)
		_ws->actors[a]->setTickValue(_vm->getTick());

	// TODO: init action list

	_title = new SceneTitle(this);
	_titleLoaded = false;
}

void Scene::initialize() {
	_vm->setGameFlag(kGameFlagScriptProcessing);

	_playerActorIdx = 0;

	if (_ws->numBarriers > 0) {
		int32 priority = 0x0FFB;
		for (int32 b = 0; b < _ws->numBarriers; b++) {
			Barrier *barrier  = _ws->barriers[b];
			barrier->setPriority(priority);
			barrier->flags &= ~kBarrierFlagC000;
			priority -= 4;
		}
	}

	Cursor::create(_cursor, _resPack, _ws->curMagnifyingGlass);

	_ws->sceneRectIdx = 0;
	_vm->screen()->clearScreen(); // XXX was clearGraphicsInQueue()
	_ws->motionStatus = 1;

	Actor *actor = getActor();
	Common::Rect *boundinRect = actor->getBoundingRect();
	boundinRect->bottom = actor->y2;
	boundinRect->right  = actor->x2 * 2;

	_ws->boundingRect = Common::Rect(195,
			115,
			445 - boundinRect->right,
			345 - boundinRect->bottom);

	actor->flags |= 1;
	actor->updateStatus(kActorStatusEnabled);

	if (_ws->numActors > 1) {
		for (int32 a = 1; a < _ws->numActors; a++) {
			Actor *act = _ws->actors[a];
			act->flags |= 1;
			act->setDirection(1);
			getActor(a)->updateStatus(kActorStatusEnabled);
			act->x1 -= act->x2;
			act->y1 -= act->y2;
			boundinRect->bottom = act->y2;
			boundinRect->right  = 2 * act->x2;
		}
	}

	int32 actionIdx = _ws->actionListIdx;
	if (actionIdx)
		_actions->queueScript(actionIdx, 0);

	// XXX not sure why we need to do this again
	_vm->screen()->clearScreen();
	// TODO loadTransTables(3, field_64/68/7C)
	// TODO setTransTable(1)
	_vm->text()->loadFont(_resPack, _ws->font1);
	// TODO preloadGraphics() .text:00410F10
	// TODO sound_sub(sceneNumber) .text:0040E750
	_ws->actorType = actorType[_ws->numChapter];

	startMusic();

	_vm->globalTickValue = 1;

	// TODO sceneRectChangedFlag = 1;

	actor->setTickValue(_vm->getTick());
	// XXX This initialization was already done earlier,
	// so I'm not sure why we need to do it again. Investigate.
	actor->updateDirection();

	if (_ws->numChapter == 9) {
		// TODO changeActorIndex(1); .text:00405140
		_ws->field_E860C = -1;
	}
}

void Scene::startMusic() {
	// TODO musicCacheOk check as part of if
	int musicId = 0;
	if (_ws->musicCurrentResourceId != -666 && _ws->numChapter != 1)
		musicId = _ws->musicResourceId - 0x7FFE0000;
	_vm->sound()->playMusic(_musPack, musicId);

}

Scene::~Scene() {
	delete _ws;
	delete _polygons;
	delete _actions;

	delete _cursor;
	delete _bgResource;
	delete _musPack;
	delete _resPack;
	//delete _blowUp;

	delete _title;
}

Actor* Scene::getActor(int index) {
	return _ws->actors[(index != -1) ? index : _playerActorIdx];
}

void Scene::enterScene() {
	if (Config.showSceneLoading && !_titleLoaded) {
		_title->load();
		// disable input polling
		//_actions->_allowInput = false;
	} else {
		_vm->screen()->setPalette(_resPack, _ws->currentPaletteId);
		_background = _bgResource->getFrame(0);
		_vm->screen()->copyToBackBuffer(
			((byte *)_background->surface.pixels) + _ws->yTop * _background->surface.w + _ws->xLeft, _background->surface.w,
			0, 0, 640, 480);

		// FIXME
		// I don't know that this is the right way to initialize the cursor
		// when the scene is started. Check against the original to see
		// when the cursor is initialized, and then how it reacts to the
		// show_cursor opcode
		Cursor::create(_cursor, _resPack, _ws->curMagnifyingGlass);
		_cursor->show();

		startMusic();

		_walking  = false;
	}

	_isActive = true;
}

void Scene::setScenePosition(int x, int y) {
	GraphicFrame *bg = _bgResource->getFrame(0);
	//_startX = x;
	//_startY = y;
	_ws->targetX = x;
	_ws->targetY = y;

	if (_ws->targetX < 0)
		_ws->targetX = 0;
	if (_ws->targetX > (bg->surface.w - 640))
		_ws->targetX = bg->surface.w - 640;


	if (_ws->targetY < 0)
		_ws->targetY = 0;
	if (_ws->targetY > (bg->surface.h - 480))
		_ws->targetY = bg->surface.h - 480;
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {

	case Common::EVENT_MOUSEMOVE:
		if (_cursor)
			_cursor->move(_ev->mouse.x, _ev->mouse.y);
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
			Cursor::create(_cursor, _resPack, _ws->curMagnifyingGlass);
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
	if (doUpdate) // || _leftClick)
		update();
}

// -------------------------------------------
// ---------- PROCESS SCENE REGION -----------
// -------------------------------------------

void Scene::update() {
	if (Config.showSceneLoading) {
		if (!_titleLoaded) {
			_title->update(_vm->getTick());
			if (_title->loadingComplete()) {
				_titleLoaded = true;
				enterScene();
			}
			return;
		}
	}

	if (updateScene())
		return;

	// TODO: check game quality
	drawScene();

	//TODO: other process stuffs from sub 0040AE30

	if (_speech->_soundResourceId != 0) {
		if (_vm->sound()->isPlaying(_speech->_soundResourceId)) {
			_speech->prepareSpeech();
		} else {
			_speech->_textResourceId = 0;
			_speech->_soundResourceId = 0;
			_vm->clearGameFlag(kGameFlag219);
		}
}
}

int Scene::updateScene() {
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
	MESURE_TICKS(updateBarriers);
	MESURE_TICKS(updateAmbientSounds);
	MESURE_TICKS(updateMusic);
	MESURE_TICKS(updateScreen);

	// Update Debug
	if (g_debugPolygons)
		debugShowPolygons();
	if (g_debugBarriers)
		debugShowBarriers();

	if (_actions->process())
		return 1;

	return 0;
}

void Scene::updateMouse() {
	Common::Rect  actorPos;
	Common::Point pt;
	Actor *act = getActor();

	// The code below was from
	// .text:0040A1B0 getCharacterScreenPosition()
	// which was only ever called at this point, so
	// inlining it for simplicity
	pt.x = act->x1 -_ws->xLeft;
	pt.y = act->y1 -_ws->yTop;

	if (_sceneIdx != 2 || _playerActorIdx != 10) {
		actorPos.left   = pt.x + 20;
		actorPos.top    = pt.y;
		actorPos.right  = pt.x + 2 * act->x2;
		actorPos.bottom = pt.y + act->y2;
	} else {
		actorPos.left   = pt.x + 50;
		actorPos.top    = pt.y + 60;
		actorPos.right  = pt.x + getActor(10)->x2 + 10;
		actorPos.bottom = pt.y + getActor(10)->y2 - 20;
	}

	int  dir = -1;
	bool done = false;

	if (_cursor->position().x < actorPos.left) {
		if (_cursor->position().y >= actorPos.top) {
			if (_cursor->position().y > actorPos.bottom) {
				if (act->getDirection() == 2) {
					if (_cursor->position().y - actorPos.bottom > 10)
						dir = 3;
				} else {
					if (act->getDirection() == 4) {
						if (actorPos.left - _cursor->position().x > 10)
							dir = 3;
					} else {
						dir = 3;
					}
				}
			} else {
				if (act->getDirection() == 1) {
					if (_cursor->position().y - actorPos.top > 10)
						dir = 2;
				} else {
					if (act->getDirection() == 3) {
						if (actorPos.bottom - _cursor->position().y > 10)
							dir = 2;
					} else {
						dir = 2;
					}
				}
			}
		} else {
			if (act->getDirection()) {
				if (act->getDirection() == 2) {
					if (actorPos.top - _cursor->position().y > 10)
						dir = 1;
				} else {
					dir = 1;
				}
			} else {
				if (actorPos.left - _cursor->position().x > 10)
					dir = 1;
			}
		}
		done = true;
	}

	if (!done && _cursor->position().x <= actorPos.right) {
		if (_cursor->position().y >= actorPos.top) {
			if (_cursor->position().y > actorPos.bottom) {
				if (act->getDirection() == 3) {
					if (_cursor->position().x - actorPos.left > 10)
						dir = 4;
				} else {
					if (act->getDirection() == 5) {
						if (actorPos.right - _cursor->position().x > 10)
							dir = 4;
					} else {
						dir = 4;
					}
				}
			}
		} else {
			if (act->getDirection() == 1) {
				if (_cursor->position().x - actorPos.left > 10)
					dir = 0;
			} else {
				if (act->getDirection() == 7) {
					if (actorPos.right - _cursor->position().x > 10)
						dir = 0;
				} else {
					dir = 0;
				}
			}
		}
		done = true;
	}

	if (!done && _cursor->position().y < actorPos.top) {
		if (act->getDirection()) {
			if (act->getDirection() == 6) {
				if (actorPos.top - _cursor->position().y > 10)
					dir = 7;
			} else {
				dir = 7;
			}
		} else {
			if (_cursor->position().x - actorPos.right > 10)
				dir = 7;
		}
		done = true;
	}

	if (!done && _cursor->position().y <= actorPos.bottom) {
		if (act->getDirection() == 5) {
			if (actorPos.bottom - _cursor->position().y > 10)
				dir = 6;
		} else {
			if (act->getDirection() == 7) {
				if (_cursor->position().y - actorPos.top > 10)
					dir = 6;
			} else {
				dir = 6;
			}
		}
		done = true;
	}

	if (!done && act->getDirection() == 4) {
		if (_cursor->position().x - actorPos.right <= 10)
			done = true;
		if (!done)
			dir = 5;
	}

	if (!done && (act->getDirection() != 6 || _cursor->position().y - actorPos.bottom > 10))
		dir = 5;

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
	if (0 && _cursor->graphicResourceId != _ws->curTalkNPC)
		_cursor->set(_ws->curTalkNPC, 0, 2);

	Actor *act = getActor(); // get the player actor reference

	// XXX field_11 seems to have something to do with
	// whether the event manager is handling a right mouse down
	// event
	if (_cursor->field_11 & 2) {
		if (act->getStatus() == 1 || act->getStatus() == 12) {
			if (direction >= 0) {
				newGraphicResourceId = _ws->curScrollUp + direction;
				_cursor->set(newGraphicResourceId, 0, 2);
			}
		}
	}

	if (act->getStatus() == 6 || act->getStatus() == 10) {
		newGraphicResourceId = _ws->curHand;
		_cursor->set(newGraphicResourceId, 0, 2);
	} else {
		if (act->getField638()) {
			if (_cursor->position().x >= rect.left && _cursor->position().x <= rlimit &&
				_cursor->position().y >= rect.top  && _cursor->position().y <= rect.bottom &&
				hitTestActor(_cursor->position())) {
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
				newGraphicResourceId = hitTestScene(_cursor->position(), type);
				if (newGraphicResourceId != -1) {
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
		int32 targetIdx = hitTest(_cursor->position(), type);

		//printf ("Mouse X(%d)/Y(%d) = %d\n", _cursor->position().x, _cursor->position().y, type);
		if (_cursor->position().x >= rect.left && _cursor->position().x <= rlimit &&
			_cursor->position().y >= rect.top  && _cursor->position().y <= rect.bottom &&
			hitTestActor(_cursor->position())) {
			if (act->getReaction(0)) {
				_cursor->set(_ws->curGrabPointer, 0, 2);
				return;
			}
		}
		if (targetIdx == -1) {
			if (_ws->numChapter != 2 || _playerActorIdx != 10) {
				if (_cursor->flags)
					_cursor->set(_ws->curMagnifyingGlass, 0, 2);
			} else {
				if (_cursor->flags)
					_cursor->set(_ws->curTalkNPC2, 0, 2);
			}
		} else {
			int32 targetUpdateType = 0;
			switch (type) {
			case kHitActionArea:
				targetUpdateType = _ws->actions[targetIdx]->actionType;
				break;
			case kHitBarrier:
				targetUpdateType = _ws->barriers[targetIdx]->actionType;
				break;
			case kHitActor:
				targetUpdateType = getActor(targetIdx)->getStatus();
				break;
			default:
				// TODO LOBYTE(hitType)
				break;
			}

			if (targetUpdateType & 1 && _cursor->flags != 2) {
				_cursor->set(_ws->curMagnifyingGlass, 0, 2);
			} else {
				if (targetUpdateType & 4) {
					_cursor->set(_ws->curHand, 0, 2);
				} else {
					if (targetUpdateType & 2) {
						_cursor->set(_ws->curTalkNPC, 0, 2);
					} else {
						if (targetUpdateType & 0x10 && _cursor->flags != 2) {
							_cursor->set(_ws->curTalkNPC2, 0, 2);
						} else {
							if (_ws->numChapter != 2 && _playerActorIdx != 10) {
								_cursor->set(_ws->curMagnifyingGlass, 0, 0);
							} else {
								if (_cursor->flags)
									_cursor->set(_ws->curTalkNPC2, 0, 2);
							}
						}
					}
				}
			}
		}
	}


}

int32 Scene::hitTestBarrier(const Common::Point pt) {
	int32 targetIdx = -1;
	for (int32 i = 0; i < _ws->numBarriers; i++) {
		Barrier *barrier = _ws->barriers[i];
		if (barrier->isOnScreen())
			if (barrier->getPolygonIndex())
				if (hitTestPixel(barrier->getResourceId(), barrier->getFrameIndex(), pt.x, pt.y, barrier->flags & 0x1000)) {
					targetIdx = i;
					break;
				}
	}
	return targetIdx;
}

int32 Scene::hitTest(const Common::Point pt, HitType &type) {
	type = kHitNone;
	int32 targetIdx = hitTestBarrier(pt);
	if (targetIdx == -1) {
		targetIdx = hitTestActionArea(pt);
		if (targetIdx == -1) {
			targetIdx = hitTestActor(pt);
			if (targetIdx != -1)
				type = kHitActor;
		} else {
			type = kHitActionArea;
		}
	} else {
		type = kHitBarrier;
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
	// TODO
	// This is a VERY loose implementation of the target
	// function, as this doesn't do any of the flag checking
	// the original did
	int32 targetIdx = -1;
	for (int32 i = 0; i < _ws->numActions; i++) {
		ActionArea *a = _ws->actions[i];
		PolyDefinitions p = _polygons->entries[a->polyIdx];
		if (p.contains(pt.x, pt.y)) {
			targetIdx = i;
			break;
		}
	}
	return targetIdx;
}

int32 Scene::hitTestScene(const Common::Point pt, HitType &type) {
	int32 top  = pt.x + _ws->xLeft;
	int32 left = pt.y + _ws->yTop;
	type = kHitNone;

	int32 result = findActionArea(Common::Point(top, left));

	if (result != -1) {
		if (LO_BYTE(_ws->actions[result]->actionType) & 8) {
			type = kHitActionArea;
			return result;
		}
	}

	// TODO barrier and actor checks

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
		pt.x - act->x - actPos.x,
		pt.y - act->y - actPos.y,
		(act->getDirection() >= 0));
}

bool Scene::hitTestPixel(ResourceId resourceId, int32 frame, int16 x, int16 y, bool flipped) {
	GraphicResource *gra = new GraphicResource(_resPack, resourceId);
	GraphicFrame    *fra = gra->getFrame(frame);

	// TODO this gets a bit funky with the "flipped" calculations for x intersection
	// The below is a pretty basic intersection test for proof of concept

	Common::Rect rect;
	rect.top    = fra->x;
	rect.left   = fra->y;
	rect.right  = fra->x + fra->surface.w;
	rect.bottom = fra->y + fra->surface.h;

	delete gra;

	return rect.contains(x, y);
}

void Scene::changePlayerActorIndex(ActorIndex index) {
	error("[Scene::changePlayerActorIndex] not implemented");
}

void Scene::updateActors() {
	for (uint32 i = 0; i < _ws->actors.size(); i++)
		_ws->actors[i]->update();
}

void Scene::updateBarriers() {
	for (uint32 i = 0; i < _ws->barriers.size(); i++)
		_ws->barriers[i]->update();
}

void Scene::updateAmbientSounds() {
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
				int loflag = LO_BYTE(snd->flags);
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
							_vm->sound()->playSound(snd->resourceId, false, volume, panning, false);
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
							_vm->sound()->playSound(snd->resourceId, 0, tmpVol, vm()->getRandom(20001) - 10000);
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
				_vm->sound()->stopSound(snd->resourceId);
		}
	}
}

int32 Scene::calculateVolumeAdjustment(AmbientSoundItem *snd, Actor *act) {
	//int32 x, y;
	if (snd->field_10) {
		/* FIXME properly handle global x/y
		if (g_object_x == -1) {
			x = snd->x - act->x1 - act->x2;
			y = snd->y - act->y1 - act->y2;
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
}

void Scene::updateScreen() {
	if (g_debugScrolling) { // DEBUG ScreenScrolling
		debugScreenScrolling(_bgResource->getFrame(0));
	} else {
		updateAdjustScreen();
	}
}

void Scene::updateAdjustScreen() {
	Actor *act = getActor();
	int32 newXLeft = -1;
	int32 newYTop  = -1;
	Common::Rect b = _ws->boundingRect;

	if (_ws->motionStatus == 1) {
		int32 posX = act->x1 - _ws->xLeft;
		int32 posY = act->y1 - _ws->yTop;

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

bool Scene::updateSceneCoordinates(int32 targetX, int32 targetY, int32 A0, bool checkSceneCoords, int32 *param) {
	Common::Rect *sr = &_ws->sceneRects[_ws->sceneRectIdx];

	_ws->targetX = targetX;
	_ws->targetY = targetY;

	_ws->field_A0 = A0;

	// Adjust coordinates
	if (checkSceneCoords)
		if (_ws->targetX + 640 > _ws->width)
			_ws->targetX = _ws->width - 640;

	if (_ws->targetX < sr->left)
		_ws->targetX = sr->left;

	if (_ws->targetY < sr->top)
		_ws->targetY = sr->top;

	if (_ws->targetX + 640 > sr->right)
		_ws->targetX = sr->right - 640;

	if (_ws->targetY + 480 < sr->bottom)
		_ws->targetY = sr->bottom - 480;

	if (checkSceneCoords)
		if (_ws->targetY + 480 > _ws->height)
			_ws->targetY = _ws->height - 480;

	// Adjust scene offsets & coordinates
	_sceneOffset = 0;
	_sceneXLeft = _ws->xLeft;
	_sceneYTop  = _ws->yTop;

	int32 diffX = _ws->targetX - _ws->xLeft;
	int32 diffY = _ws->targetY - _ws->yTop;

	if (abs(diffX) <= abs(diffY)) {
		if (_ws->yTop > _ws->targetY)
			_ws->field_A0 = -_ws->field_A0;

		_sceneOffsetAdd = Common::Rational(_ws->field_A0, diffY) * diffX;

		if (param != NULL && abs(diffY) <= abs(_ws->field_A0)) {
			_ws->targetX = -1;
			*param = 0;
			return true;
		}
	} else {
		if (_ws->xLeft > _ws->targetX)
			_ws->field_A0 = -_ws->field_A0;

		_sceneOffsetAdd = Common::Rational(_ws->field_A0, diffX) * diffY;

		if (param != NULL && abs(diffX) <= abs(_ws->field_A0)) {
			_ws->targetX = -1;
			return true;
		}
	}

	return false;
}

void Scene::adjustCoordinates(int32 x, int32 y, Common::Point *point) {
	if (!point)
		error("[Scene::adjustCoordinates] Invalid point parameter!");

	point->x = x - _ws->xLeft;
	point->y = y - _ws->yTop;
}

//////////////////////////////////////////////////////////////////////////
// Scene drawing
//////////////////////////////////////////////////////////////////////////
int Scene::drawScene() {
	_vm->screen()->clearGraphicsInQueue();

	if (_skipDrawScene) {
		_vm->screen()->clearScreen();
	} else {
		// Draw scene background
		_vm->screen()->draw(_bgResource, 0, -_ws->xLeft, -_ws->yTop, 0);

		// Draw actors on the update list
		buildUpdateList();
		drawUpdateList();

		if (_ws->numChapter == 11)
			checkVisibleActorsPriority();

		// Queue updates
		for (uint32 i = 0; i < _ws->actors.size(); i++)
			_ws->actors[i]->draw();

		for (uint32 i = 0; i < _ws->barriers.size(); i++)
			_ws->barriers[i]->draw();

		Actor *player = getActor();
		if (player->getStatus() == kActorStatus6 || player->getStatus() == kActorStatus10)
			player->update_40DE20();
		else
			player->setNumberFlag01(0);

		_vm->screen()->drawGraphicsInQueue();
	}

	return 1;
}

bool Scene::updateListCompare(const UpdateItem &item1, const UpdateItem &item2) {
	return item1.priority - item2.priority;
}

void Scene::checkVisibleActorsPriority() {
	error("[Scene::checkVisibleActorsPriority] not implemented");
}

void Scene::adjustActorPriority(ActorIndex index) {
	error("[Scene::adjustActorPriority] not implemented");
}

void Scene::buildUpdateList() {
	_updateList.clear();

	for (uint32 i = 0; i < _ws->actors.size(); i++) {
		Actor *actor = _ws->actors[i];

		if (actor->isVisible()) {
			UpdateItem item;
			item.index = i;
			item.priority = actor->y1 + actor->y2;

			_updateList.push_back(item);
		}
	}

	// Sort the list (the original uses qsort, so we may have to revert to that if our sort isn't behaving the same)
	Common::sort(_updateList.begin(), _updateList.end(), &Scene::updateListCompare);
}

void Scene::drawUpdateList() {
	// TODO this is supposed to be looping through
	// a collection of CharacterUpdateItems. Since
	// we're only on scene 1 atm, and there is only one
	// character, this will have to do :P
	for (int32 i = 0; i < _ws->numActors; i++) {
		int actorRegPt = 0;
		Actor *act = _ws->actors[i];
		Common::Point pt;

		// XXX Since we're not using CharacterUpdateItems,
		// the actor priority is never going to be changed.
		// Need to investigate if this is an issue
		/*
		if (act->priority < 0)
			act->priority = abs(act->priority);
			continue;
		*/
		act->setPriority(3);
		if (act->getField944() == 1 || act->getField944() == 4)
			act->setPriority(1);
		else {
			act->setField938(1);
			act->setField934(0);
			pt.x = act->x1 + act->x2;
			pt.y = act->y1 + act->y2;

			actorRegPt = act->getBoundingRect()->bottom + act->getBoundingRect()->right + 4;

			// TODO special case for scene 11
			// Not sure if we're checking the scene index
			// or the scene number though :P
			/*
			if (_sceneIdx == 11) {
				// FIXME this probably won't work
				if (act != this->getActor())
					actorRegPt += 20;
			}
			*/

			// XXX from .text:0040a4d1
			for (int32 barIdx = 0; barIdx < _ws->numBarriers; barIdx++) {
				Barrier *bar    = _ws->barriers[barIdx];
				bool actInBar   = bar->getBoundingRect()->contains(*act->getBoundingRect());
				bool intersects = false;

				// TODO verify that my funky LOBYTE macro actually
				// works the way I assume it should :P
				if (!actInBar) {
					if (LO_BYTE(bar->flags) & 0x20)
						if (!(LO_BYTE(bar->flags) & 0x80))
							// XXX not sure if this will work, as it's
							// supposed to set 0x40 to the lobyte...
							bar->flags |= 0x40;
					continue;
				}

				if (bar->flags & 2) {
					// TODO refactor
					if (bar->getField74() || bar->getField78() ||
						bar->getField7C() || bar->getField80())
						intersects = (pt.y > bar->getField78() + (bar->getField80() - bar->getField78()) * (pt.x - bar->getField74()) / (bar->getField7C() - bar->getField74())) == 0;
					else
						intersects = true;
				} else {
					if (bar->flags & 0x40) {
						PolyDefinitions *poly = &_polygons->entries[bar->getPolygonIndex()];
						if (pt.x > 0 && pt.y > 0 && poly->numPoints > 0)
							intersects = poly->contains(pt.x, pt.y);
						else
							;//warning ("[drawActorsAndBarriers] trying to find intersection of uninitialized point");
					}
					// XXX the original has an else case here that
					// assigns intersects the value of the
					// flags & 2 check, which doesn't make any sense since
					// that path would never have been taken if code
					// execution had made it's way here.
				}
				if (LO_BYTE(bar->flags) & 0x80 || intersects) {
					if (LO_BYTE(bar->flags) & 0x20)
						// XXX not sure if this will work, as it's
						// supposed to set this value on the lobyte...
						bar->flags &= 0xBF | 0x80;
					else
						// XXX another lobyte assignment...
						bar->flags |= 0x40;
						// TODO label jump up a few lines here. Investigate...
				}
				if (bar->flags & 4) {
					if (intersects) {
						if(act->flags & 2)
							;//warning ("[drawActorsAndBarriers] Assigning mask to masked character [%s]", bar->name);
						else {
							// TODO there's a call to sub_40ac10 that does
							// a point calculation, but the result doesn't appear to
							// ever be used, and the object passed in as a parameter
							// isn't updated
							act->setBarrierIndex(barIdx);
							act->flags |= 2;
						}
					}
				} else {
					if (intersects) {
						// XXX assuming the following:
						// "if ( *(int *)((char *)&scene.characters[0].priority + v18) < *(v12_barrierPtr + 35) )"
						// is the same as what I'm comparing :P
						if (act->getPriority() < bar->getPriority()) {
							act->setField934(1);
							act->setPriority(bar->getPriority() + 3);
							// TODO there's a block of code here that seems
							// to loop through the CharacterUpdateItems and do some
							// priority adjustment. Since I'm not using CharacterUpdateItems as of yet,
							// I'm not sure what to do here
							// The loop seems to occur if:
							// (a) there are still character items to process
							// (b) sceneNumber != 2 && actor->field_944 != 1
						}
					} else {
						if (act->getPriority() > bar->getPriority() || act->getPriority() == 1) {
							act->setField934(1);
							act->setPriority(bar->getPriority() - 1);
							// TODO another character update loop
							// This time it looks like there's another
							// intersection test, and more updates
							// to field_934 and field_944, then
							// priority updates
						}
					}
				}
			} // end for (barriers)
		}
	} // end for (actors)

}

void Scene::getActorPosition(Actor *actor, Common::Point *pt) {
	pt->x = actor->x1 - _ws->xLeft;
	pt->y = actor->y1 - _ws->yTop;
}
// ----------------------------------
// ---------- SCREEN REGION -----------
// ----------------------------------

void Scene::copyToBackBufferClipped(Graphics::Surface *surface, int x, int y) {
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
	// Horizontal scrolling
	if (_cursor->position().x < SCREEN_EDGES && _ws->xLeft >= SCROLL_STEP)
		_ws->xLeft -= SCROLL_STEP;
	else if (_cursor->position().x > 640 - SCREEN_EDGES && _ws->xLeft <= bg->surface.w - 640 - SCROLL_STEP)
		_ws->xLeft += SCROLL_STEP;

	// Vertical scrolling
	if (_cursor->position().y < SCREEN_EDGES && _ws->yTop >= SCROLL_STEP)
		_ws->yTop -= SCROLL_STEP;
	else if (_cursor->position().y > 480 - SCREEN_EDGES && _ws->yTop <= bg->surface.h - 480 - SCROLL_STEP)
		_ws->yTop += SCROLL_STEP;
}

// WALK REGION DEBUG
void Scene::debugShowWalkRegion(PolyDefinitions *poly) {
	Graphics::Surface surface;
	surface.create(poly->boundingRect.right - poly->boundingRect.left + 1,
	               poly->boundingRect.bottom - poly->boundingRect.top + 1,
	               1);

	// Draw all lines in Polygon
	for (int32 i = 0; i < poly->numPoints; i++) {
		surface.drawLine(
		    poly->points[i].x - poly->boundingRect.left,
		    poly->points[i].y - poly->boundingRect.top,
		    poly->points[(i+1) % poly->numPoints].x - poly->boundingRect.left,
		    poly->points[(i+1) % poly->numPoints].y - poly->boundingRect.top, 0x3A);
	}

	copyToBackBufferClipped(&surface, poly->boundingRect.left, poly->boundingRect.top);

	surface.free();
}

// POLYGONS DEBUG
void Scene::debugShowPolygons() {
	for (int32 p = 0; p < _polygons->numEntries; p++) {
		Graphics::Surface surface;
		PolyDefinitions poly = _polygons->entries[p];
		surface.create(poly.boundingRect.right - poly.boundingRect.left + 1,
		               poly.boundingRect.bottom - poly.boundingRect.top + 1,
		               1);

		// Draw all lines in Polygon
		for (int32 i = 0; i < poly.numPoints; i++) {
			surface.drawLine(
			    poly.points[i].x - poly.boundingRect.left,
			    poly.points[i].y - poly.boundingRect.top,
			    poly.points[(i+1) % poly.numPoints].x - poly.boundingRect.left,
			    poly.points[(i+1) % poly.numPoints].y - poly.boundingRect.top, 0xFF);
		}

		copyToBackBufferClipped(&surface, poly.boundingRect.left, poly.boundingRect.top);

		surface.free();
	}
}

// BARRIER DEBUGGING
void Scene::debugShowBarriers() {
	for (int32 p = 0; p < _ws->numBarriers; p++) {
		Graphics::Surface surface;
		Barrier *barrier = _ws->barriers[p];

		if (barrier->flags & 0x20) {
			surface.create(barrier->getBoundingRect()->right - barrier->getBoundingRect()->left + 1,
			               barrier->getBoundingRect()->bottom - barrier->getBoundingRect()->top + 1,
			               1);
			surface.frameRect(*barrier->getBoundingRect(), 0x22);
			copyToBackBufferClipped(&surface, barrier->x, barrier->y);
		}

		surface.free();
	}
}

// BARRIER DEBUGGING
void Scene::debugShowActors() {
	for (int32 p = 0; p < _ws->numActors; p++) {
		Graphics::Surface surface;
		Actor *a = _ws->actors[p];

		if (a->flags & 2) {
			surface.create(a->getBoundingRect()->right - a->getBoundingRect()->left + 1,
			               a->getBoundingRect()->bottom - a->getBoundingRect()->top + 1,
			               1);
			surface.frameRect(*a->getBoundingRect(), 0x22);
			copyToBackBufferClipped(&surface, a->x, a->y);
		}

		surface.free();
	}
}

void Scene::updatePlayerChapter9(int32 param) {
	error("[Scene::updatePlayerChapter9] not implemented!");
}

ResourceId Scene::playSpeech(int32 a1) {
	error("[Scene::playSpeech] not implemented!");
}

ResourceId Scene::playSpeech(int32 a1, int32 a2) {
	error("[Scene::playSpeech] not implemented!");
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

//////////////////////////////////////////////////////////////////////////
// SceneTitle
//////////////////////////////////////////////////////////////////////////
SceneTitle::SceneTitle(Scene *scene): _scene(scene) {
	_bg = NULL;
	_progress = NULL;
}

SceneTitle::~SceneTitle() {
	delete _bg;
	delete _progress;
}

void SceneTitle::load() {
	_start = _scene->vm()->getTick();

	_bg = new GraphicResource(_scene->_resPack, _scene->_ws->sceneTitleGraphicResourceId);
	_scene->vm()->screen()->setPalette(_scene->_resPack, _scene->_ws->sceneTitlePaletteResourceId);

	ResourcePack *pack = new ResourcePack(0x12);
	_progress = new GraphicResource(pack, 0x80120011);
	_spinnerProgress = 0;
	_spinnerFrame = 0;

	_scene->vm()->text()->loadFont(pack, 0x80120012);

	delete pack;

	_done = false;
	_showMouseState = g_system->showMouse(false);
}

void SceneTitle::update(int32 tick) {

	// XXX This is not from the original. It's just some
	// arbitrary math to throttle the progress indicator.
	// In the game, the scene loading progress indicated
	// how far the engine had progressed in terms of buffering
	// the various scene resource.
	// Since we don't actually buffer content like the original,
	// but load on demand from offset/length within a ResourcePack,
	// the progress indicator is effectively useless.
	// It's just in here as "eye candy" :P
	if ((tick - _start) % 500 > 100)
		_spinnerProgress += 20;

	GraphicFrame *bgFrame = _bg->getFrame(0);

	_scene->vm()->screen()->copyToBackBuffer(
		((byte *)bgFrame->surface.pixels),
		bgFrame->surface.w,
		0, 0, 640, 480);

	ResourceId resourceId = _scene->getSceneIndex() - 4 + 1811;
	int32 resWidth = _scene->vm()->text()->getResTextWidth(resourceId);
	_scene->vm()->text()->drawResTextCentered(320 - resWidth * 24, 30, resWidth, resourceId);

	GraphicFrame *frame = _progress->getFrame(_spinnerFrame);

	_scene->vm()->screen()->copyRectToScreenWithTransparency(
		((byte*)frame->surface.pixels),
		frame->surface.w,
		frame->x - 290 + _spinnerProgress,
		frame->y,
		frame->surface.w,
		frame->surface.h);

	_spinnerFrame++;

	if (_spinnerFrame > _progress->getFrameCount() - 1)
		_spinnerFrame = 0;

    if (_spinnerProgress > 590) {
		_done = true;
        g_system->showMouse(_showMouseState);
    }
}

} // end of namespace Asylum
