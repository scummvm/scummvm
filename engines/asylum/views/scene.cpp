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
		_ws->actors[a]->tickValue = _vm->getTick();

	// TODO: init action list

	_titleLoaded = false;
}

void Scene::initialize() {
	_vm->setGameFlag(kGameFlagScriptProcessing);

	_playerActorIdx = 0;

	if (_ws->numBarriers > 0) {
		int32 priority = 0x0FFB;
		for (int32 b = 0; b < _ws->numBarriers; b++) {
			Barrier *barrier  = _ws->barriers[b];
			barrier->priority = priority;
			barrier->flags &= ~kBarrierFlagC000;
			priority -= 4;
		}
	}

	Cursor::create(_cursor, _resPack, _ws->curMagnifyingGlass);

	_ws->sceneRectIdx = 0;
	_vm->screen()->clearScreen(); // XXX was clearGraphicsInQueue()
	_ws->motionStatus = 1;

	Actor *actor = getActor();
	actor->boundingRect.bottom = actor->y2;
	actor->boundingRect.right  = actor->x2 * 2;

	_ws->boundingRect = Common::Rect(195,
			115,
			445 - actor->boundingRect.right,
			345 - actor->boundingRect.bottom);

	actor->flags |= 1;
	actor->updateStatus(kActorStatusEnabled);

	if (_ws->numActors > 1) {
		for (int32 a = 1; a < _ws->numActors; a++) {
			Actor *act = _ws->actors[a];
			act->flags |= 1;
			act->direction = 1;
			getActor(a)->updateStatus(kActorStatusEnabled);
			act->x1 -= act->x2;
			act->y1 -= act->y2;
			act->boundingRect.bottom = act->y2;
			act->boundingRect.right  = 2 * act->x2;
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

	_vm->tempTick07 = 1;

	// TODO sceneRectChangedFlag = 1;

	actor->tickValue= _vm->getTick();
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

	// TODO we can probably dispose of the title resource once the
	// _titleLoaded flag has been set, as opposed to in the
	// scene destructor
#ifdef SHOW_SCENE_LOADING
	delete _title;
#endif
}

Actor* Scene::getActor(int index) {
	return _ws->actors[(index != -1) ? index : _playerActorIdx];
}

void Scene::enterScene() {
#ifdef SHOW_SCENE_LOADING
	if (!_titleLoaded) {
		_title = new SceneTitle(this);
		// disable input polling
		_actions->_allowInput = false;
	} else {
#endif
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
#ifdef SHOW_SCENE_LOADING
	}
#endif
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
#ifdef SHOW_SCENE_LOADING
	if (!_titleLoaded) {
		_title->update(_vm->getTick());
		if (_title->loadingComplete()) {
			_titleLoaded = true;
			enterScene();
		}
		return;
	}
#endif
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
#ifdef SHOW_SCENE_TIMES
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
				if (act->direction == 2) {
					if (_cursor->position().y - actorPos.bottom > 10)
						dir = 3;
				} else {
					if (act->direction == 4) {
						if (actorPos.left - _cursor->position().x > 10)
							dir = 3;
					} else {
						dir = 3;
					}
				}
			} else {
				if (act->direction == 1) {
					if (_cursor->position().y - actorPos.top > 10)
						dir = 2;
				} else {
					if (act->direction == 3) {
						if (actorPos.bottom - _cursor->position().y > 10)
							dir = 2;
					} else {
						dir = 2;
					}
				}
			}
		} else {
			if (act->direction) {
				if (act->direction == 2) {
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
				if (act->direction == 3) {
					if (_cursor->position().x - actorPos.left > 10)
						dir = 4;
				} else {
					if (act->direction == 5) {
						if (actorPos.right - _cursor->position().x > 10)
							dir = 4;
					} else {
						dir = 4;
					}
				}
			}
		} else {
			if (act->direction == 1) {
				if (_cursor->position().x - actorPos.left > 10)
					dir = 0;
			} else {
				if (act->direction == 7) {
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
		if (act->direction) {
			if (act->direction == 6) {
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
		if (act->direction == 5) {
			if (actorPos.bottom - _cursor->position().y > 10)
				dir = 6;
		} else {
			if (act->direction == 7) {
				if (_cursor->position().y - actorPos.top > 10)
					dir = 6;
			} else {
				dir = 6;
			}
		}
		done = true;
	}

	if (!done && act->direction == 4) {
		if (_cursor->position().x - actorPos.right <= 10)
			done = true;
		if (!done)
			dir = 5;
	}

	if (!done && (act->direction != 6 || _cursor->position().y - actorPos.bottom > 10))
		dir = 5;

	handleMouseUpdate(dir, actorPos);

	if (dir >= 0) {
		if (act->status == 1 || act->status == 12)
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
		if (act->status == 1 || act->status == 12) {
			if (direction >= 0) {
				newGraphicResourceId = _ws->curScrollUp + direction;
				_cursor->set(newGraphicResourceId, 0, 2);
			}
		}
	}

	if (act->status == 6 || act->status == 10) {
		newGraphicResourceId = _ws->curHand;
		_cursor->set(newGraphicResourceId, 0, 2);
	} else {
		if (act->field_638) {
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
			if (act->reaction[0]) {
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
				targetUpdateType = getActor(targetIdx)->status;
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
		Barrier *b = _ws->barriers[i];
		if (_ws->isBarrierOnScreen(i))
			if (b->polyIdx)
				if (hitTestPixel(b->resourceId, b->frameIdx, pt.x, pt.y, b->flags & 0x1000)) {
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
		if (LOBYTE(_ws->actions[result]->actionType) & 8) {
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
	if (act->frameNum >= act->frameCount)
		hitFrame = 2 * act->frameNum - act->frameCount - 1;
	else
		hitFrame = act->frameNum;

	return hitTestPixel(act->graphicResourceId,
		hitFrame,
		pt.x - act->x - actPos.x,
		pt.y - act->y - actPos.y,
		(act->direction >= 0));
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
	for (int32 a = 0; a < _ws->numActors; a++)
		getActor(a)->update();
}

void Scene::updateBarriers() {
	//Screen *screen = _vm->screen();

	int32 barriersCount  = (int32)_ws->barriers.size();
	//int  startTickCount = 0;
	bool canPlaySound   = false;

	if (barriersCount > 0) {
		for (int32 b = 0; b < barriersCount; b++) {
			Barrier *barrier = _ws->barriers[b];

			if (barrier->field_3C == 4) {
				if (_ws->isBarrierVisible(b)) {
					int32 flag = barrier->flags;
					if (flag & 0x20) {
						if (_vm->getTick() - barrier->tickCount >= 0x3E8 / barrier->field_B4) {
							barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
							barrier->tickCount = _vm->getTick();
							canPlaySound       = true;
						}
					} else if (flag & 0x10) {
						uint32 frameIdx  = barrier->frameIdx;
						int equalZero = frameIdx == 0;
						if (!frameIdx) {
							if (_vm->getTick() - barrier->tickCount >= 1000 * barrier->tickCount2) {
								if (vm()->getRandom(barrier->field_C0) == 1) {
									if (barrier->field_68C[0]) {
										// TODO: fix this, and find a better way to get frame count
										// Sometimes we get wrong random resource id

										barrier->resourceId = barrier->getRandomId();
										GraphicResource *gra = new GraphicResource(_resPack, barrier->resourceId);
										barrier->frameCount  = gra->getFrameCount();
										delete gra;
									}
									barrier->frameIdx++;
								}
								barrier->tickCount = _vm->getTick();
								canPlaySound       = true;
							}
							frameIdx  = barrier->frameIdx;
							equalZero = frameIdx == 0;
						}

						if (!equalZero) {
							if (_vm->getTick() - barrier->tickCount >= 0x3E8 / barrier->field_B4) {
								barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
								barrier->tickCount = _vm->getTick();
								canPlaySound = true;
							}
						}
					} else if (flag & 8) {
						if (_vm->getTick() - barrier->tickCount >= 0x3E8 / barrier->field_B4) {
							uint32 frameIdx = barrier->frameIdx + 1;
							if (frameIdx < barrier->frameCount - 1) {
								if (barrier->field_688 == 1) {
									// TODO: get global x, y positions
								}
							} else {
								barrier->flags &= ~kBarrierFlag8;
								if (barrier->field_688 == 1) {
									// TODO: reset global x, y positions
								}
							}
							barrier->frameIdx = frameIdx;
						}
					} else if ((flag & 0xFF) & 8) { // check this
						if (_vm->getTick() - barrier->tickCount >= 1000 * barrier->tickCount2) {
							if (vm()->getRandom(barrier->field_C0) == 1) { // TODO: THIS ISNT WORKING
								barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
								barrier->tickCount = _vm->getTick();
								canPlaySound = true;
							}
						}
					} else if (!((flag & 0xFFFF) & 6)) {
						if (_vm->getTick() - barrier->tickCount >= 0x3E8 / barrier->field_B4 && (flag & 0x10000)) {
							uint32 frameIdx = barrier->frameIdx - 1;
							if (frameIdx <= 0) {
								barrier->flags &= ~kBarrierFlag10000;
								if (barrier->field_688 == 1) {
									// TODO: reset global x, y positions
								}
								barrier->tickCount = _vm->getTick();
								canPlaySound = true;
							}
							if (barrier->field_688 == 1) {
								// TODO: get global x, y positions
							}
							barrier->frameIdx = frameIdx;
						} else if (_vm->getTick() - barrier->tickCount >= 0x3E8 / barrier->field_B4) {
							if ((flag & 0xFF) & 2) {
								if (barrier->frameIdx == barrier->frameCount - 1) {
									barrier->frameIdx--;
									barrier->flags = ((flag & 0xFF) & 0xFD) | 4;
								} else {
									barrier->frameIdx++;
								}
							} else if ((flag & 0xFF) & 4) {
								if (barrier->frameIdx) {
									barrier->frameIdx--;
								} else {
									barrier->frameIdx++;
									barrier->flags = ((flag & 0xFF) & 0xFB) | 2;
								}
							}
						}
					}

					flag = barrier->flags;
					flag &= 0x40000;
					if (flag != 0) {
						if (barrier->frameIdx == barrier->frameCount - 1) {
							if (barrier->field_B4 <= 15) {
								barrier->field_B4 -= 2;
								if (barrier->field_B4 < 0)
									barrier->field_B4 = 0;
							} else {
								barrier->field_B4 = 15;
							}
							if (!barrier->field_B4)
								barrier->flags &= 0xFFFEF1C7;
						}
					}
				}

				if (canPlaySound) {
					barrier->updateSoundItems(_vm->sound());
					barrier->stopSound();
				}

				// TODO: get sound functions according with scene
			}
		}
	}
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
				int loflag = LOBYTE(snd->flags);
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

// ----------------------------------
// ---------- DRAW REGION -----------
// ----------------------------------

int Scene::drawScene() {

	_vm->screen()->clearGraphicsInQueue();

	if (_skipDrawScene) {
		_vm->screen()->clearScreen();
	} else {
		// Draw scene background
		GraphicFrame *bg = _bgResource->getFrame(0);

		_vm->screen()->copyToBackBuffer(
			((byte *)bg->surface.pixels) + _ws->yTop * bg->surface.w + _ws->xLeft,
			bg->surface.w,
			0,
			0,
			640,
			480);

		drawActorsAndBarriers();
		queueActorUpdates();
		queueBarrierUpdates();

		// TODO: draw main actor stuff

		_vm->screen()->drawGraphicsInQueue();
	}

	return 1;
}

void Scene::drawActorsAndBarriers() {
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
		act->priority = 3;
		if (act->field_944 == 1 || act->field_944 == 4)
			act->priority = 1;
		else {
			act->field_938 = 1;
			act->field_934 = 0;
			pt.x = act->x1 + act->x2;
			pt.y = act->y1 + act->y2;

			actorRegPt = act->boundingRect.bottom + act->boundingRect.right + 4;

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
				bool actInBar   = bar->boundingRect.contains(act->boundingRect);
				bool intersects = false;

				// TODO verify that my funky LOBYTE macro actually
				// works the way I assume it should :P
				if (!actInBar) {
					if (LOBYTE(bar->flags) & 0x20)
						if (!(LOBYTE(bar->flags) & 0x80))
							// XXX not sure if this will work, as it's
							// supposed to set 0x40 to the lobyte...
							bar->flags |= 0x40;
					continue;
				}

				if (bar->flags & 2) {
					// TODO refactor
					if (bar->field_74 || bar->field_78 ||
						bar->field_7C || bar->field_80)
						intersects = (pt.y > bar->field_78 + (bar->field_80 - bar->field_78) * (pt.x - bar->field_74) / (bar->field_7C - bar->field_74)) == 0;
					else
						intersects = true;
				} else {
					if (bar->flags & 0x40) {
						PolyDefinitions *poly = &_polygons->entries[bar->polyIdx];
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
				if (LOBYTE(bar->flags) & 0x80 || intersects) {
					if (LOBYTE(bar->flags) & 0x20)
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
							act->field_3C = barIdx;
							act->flags |= 2;
						}
					}
				} else {
					if (intersects) {
						// XXX assuming the following:
						// "if ( *(int *)((char *)&scene.characters[0].priority + v18) < *(v12_barrierPtr + 35) )"
						// is the same as what I'm comparing :P
						if (act->priority < bar->priority) {
							act->field_934 = 1;
							act->priority = bar->priority + 3;
							// TODO there's a block of code here that seems
							// to loop through the CharacterUpdateItems and do some
							// priority adjustment. Since I'm not using CharacterUpdateItems as of yet,
							// I'm not sure what to do here
							// The loop seems to occur if:
							// (a) there are still character items to process
							// (b) sceneNumber != 2 && actor->field_944 != 1
						}
					} else {
						if (act->priority > bar->priority || act->priority == 1) {
							act->field_934 = 1;
							act->priority = bar->priority - 1;
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

int Scene::queueActorUpdates() {
	if (_ws->numActors > 0) {
		Common::Point pt;
		for (int32 a = 0; a < _ws->numActors; a++) {
			Actor *actor = _ws->actors[a];

			if ((actor->flags & 0xFF) & 1) { // check this mask
				getActorPosition(actor, &pt);
				//pt.x += actor->x;
				//pt.y += actor->y;

				int32 frameNum = actor->frameNum;
				if (actor->frameNum >= actor->frameCount) {
					frameNum = 2 * actor->frameCount - actor->frameNum - 1;
				}

				if ((actor->flags & 0xFF) & 2) {
					// TODO: sub_40AC10



				} else {
					// TODO: get flag value from character_DeadSarah_sub_40A140
					_vm->screen()->addGraphicToQueue(actor->graphicResourceId, frameNum, pt.x, pt.y, ((actor->direction < 5) - 1) & 2, actor->field_96C, actor->priority);
				}
			}
		}
	}

	return 1;
}

int Scene::queueBarrierUpdates() {
	int32 barriersCount = (int32)_ws->barriers.size();

	if (barriersCount > 0) {
		for (int32 b = 0; b < barriersCount; b++) {
			Barrier *barrier = _ws->barriers[b];

			if (!(barrier->flags & 4) && !((barrier->flags & 0xFF) & 0x40)) {
				if (_ws->isBarrierOnScreen(b)) {
					//TODO: need to do something here yet

					if (barrier->field_67C <= 0 || barrier->field_67C >= 4) { // TODO: still missing a condition for game quality config
						_vm->screen()->addGraphicToQueue(barrier->resourceId, barrier->frameIdx, barrier->x, barrier->y, (barrier->flags >> 11) & 2, barrier->field_67C - 3, barrier->priority);
					} else {
						// TODO: Do Cross Fade
						// parameters: barrier->resourceId, barrier->frameIdx, barrier->x, barrier->y, _ws->backgroundImage, _ws->xLeft, _ws->yTop, 0, 0, barrier->field_67C - 1
						_vm->screen()->addGraphicToQueue(barrier->resourceId, barrier->frameIdx, barrier->x, barrier->y, 0, 0, 0);
					}
				}
			}
		}
	}

	return 1;
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
		Barrier *b = _ws->barriers[p];

		if (b->flags & 0x20) {
			surface.create(b->boundingRect.right - b->boundingRect.left + 1,
			               b->boundingRect.bottom - b->boundingRect.top + 1,
			               1);
			surface.frameRect(b->boundingRect, 0x22);
			copyToBackBufferClipped(&surface, b->x, b->y);
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
			surface.create(a->boundingRect.right - a->boundingRect.left + 1,
			               a->boundingRect.bottom - a->boundingRect.top + 1,
			               1);
			surface.frameRect(a->boundingRect, 0x22);
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

//////////////////////////////////////////////////////////////////////////
// SceneTitle
//////////////////////////////////////////////////////////////////////////
SceneTitle::SceneTitle(Scene *scene): _scene(scene) {
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

SceneTitle::~SceneTitle() {
	delete _bg;
	delete _progress;
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
