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

#include "asylum/scene.h"
#include "asylum/sceneres.h"
#include "asylum/shared.h"
#include "asylum/scriptman.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugPolygons;
int g_debugBarriers;

Scene::Scene(uint8 sceneIdx) {
	_sceneIdx		= sceneIdx;
	_sceneResource	= new SceneResource;

	if (_sceneResource->load(_sceneIdx)) {
		_text 		= new Text(Shared.getScreen());
		_resPack 	= new ResourcePack(sceneIdx);
		_speechPack = new ResourcePack(3);

		_sceneResource->getMainActor()->setResourcePack(_resPack);
		
		_text->loadFont(_resPack, _sceneResource->getWorldStats()->commonRes.font1);

		char musPackFileName[10];
		sprintf(musPackFileName, "mus.%03d", sceneIdx);
		_musPack 	= new ResourcePack(musPackFileName);
		_bgResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.backgroundImage);

        _blowUp = 0;
	}

	_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.curMagnifyingGlass);

	_background    = 0;
	_startX        = 0;
	_startY        = 0;
	_leftClick     = false;
	_rightButton   = false;
	_isActive      = false;
    _skipDrawScene = 0;

	g_debugPolygons = 0;
	g_debugBarriers = 0;

    Shared.setGameFlag(183);

    // TODO: do all the rest stuffs in sub at address 40E460
    WorldStats   *worldStats = _sceneResource->getWorldStats();
    _playerActorIdx = 0;

    if (worldStats->numBarriers > 0) {
        uint32 priority = 0x0FFB;
        for (int b=0; b < worldStats->numBarriers; b++) {
            BarrierItem *barrier = &worldStats->barriers[b];
            barrier->priority = priority;
            barrier->flags &= 0xFFFF3FFF;
            priority -= 4;
        }
    }
    worldStats->sceneRectIdx = 0;
    Shared.getScreen()->clearGraphicsInQueue();
    worldStats->motionStatus = 1;
    // TODO: do some rect stuffs from player actor
    // TODO: reset actors flags
}

Scene::~Scene() {
	delete _cursorResource;
	delete _bgResource;
	delete _musPack;
	delete _speechPack;
	delete _resPack;
	delete _text;
	delete _sceneResource;
    delete _blowUp;
 }

void Scene::enterScene() {
	Shared.getScreen()->setPalette(_resPack, _sceneResource->getWorldStats()->commonRes.palette);
	_background = _bgResource->getFrame(0);
	Shared.getScreen()->copyToBackBuffer(
			((byte *)_background->surface.pixels) + _startY * _background->surface.w + _startX, _background->surface.w,
			0, 0, 640, 480);

	_cursorStep		= 1;
	_curMouseCursor	= 0;
	Shared.getScreen()->setCursor(_cursorResource, 0);
	Shared.getScreen()->showCursor();

	// Music testing: play the first music track
	Shared.getSound()->playMusic(_musPack, 0);

	_isActive = true;
	_walking  = false;
}

ActionDefinitions* Scene::getDefaultActionList() {
	if (_sceneResource)
		return getActionList(_sceneResource->getWorldStats()->actionListIdx);
	else
		return 0;
}

ActionDefinitions* Scene::getActionList(int actionListIndex) {
	if ((actionListIndex >= 0) && (actionListIndex < (int)_sceneResource->getWorldStats()->numActions))
		return &_sceneResource->getActionList()->actions[actionListIndex];
	else
		return 0;
}

void Scene::setActorPosition(int actorIndex, int x, int y) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		_sceneResource->getWorldStats()->actors[actorIndex].boundingRect.left = x;
		_sceneResource->getWorldStats()->actors[actorIndex].boundingRect.top  = y;
	}
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		_sceneResource->getMainActor()->_actorX = x;
		_sceneResource->getMainActor()->_actorY = y;
	}
}

void Scene::setActorAction(int actorIndex, int action) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		_sceneResource->getWorldStats()->actors[actorIndex].direction = action;
	}
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		if(_sceneResource->getMainActor())
		_sceneResource->getMainActor()->setActionByIndex(action);  // The action appears to be an index rather than a direct resId
	}
}

void Scene::actorVisible(int actorIndex, bool visible) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors)) {
		if(visible) //	TODO - enums for flags (0x01 is visible)
			_sceneResource->getWorldStats()->actors[actorIndex].flags |= 0x01;
		else
			_sceneResource->getWorldStats()->actors[actorIndex].flags &= 0xFFFFFFFE;
	}

	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		//if(_sceneResource->getMainActor())
		//_sceneResource->getMainActor()->setAction(action);
	}
}

bool Scene::actorVisible(int actorIndex) {
	if ((actorIndex >= 0) && (actorIndex < (int)_sceneResource->getWorldStats()->numActors))
		return _sceneResource->getWorldStats()->actors[actorIndex].flags & 0x01;	//	TODO - enums for flags (0x01 is visible)
	
	// FIXME - Remove this once mainActor uses proper actor info
	if (actorIndex == 0) {
		//if(_sceneResource->getMainActor())
		//_sceneResource->getMainActor()->setAction(action);
	}
	
	return false;
}

void Scene::setScenePosition(int x, int y)
{
	GraphicFrame *bg = _bgResource->getFrame(0);
	_startX = x;
	_startY = y;
	
	if (_startX < 0)
		_startX = 0;
	if (_startX > (bg->surface.w - 640))
		_startX = bg->surface.w - 640;
		
	
	if (_startX < 0)
		_startY = 0;
	if (_startX > (bg->surface.h - 480))
		_startY = bg->surface.h - 480;		
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {

	case Common::EVENT_MOUSEMOVE:
		_mouseX = _ev->mouse.x;
		_mouseY = _ev->mouse.y;
		break;

	case Common::EVENT_LBUTTONUP:
		if (ScriptMan.isInputAllowed())
			_leftClick = true;
		break;

	case Common::EVENT_RBUTTONUP:
		if (ScriptMan.isInputAllowed()) {
			delete _cursorResource;
			// TODO This isn't always going to be the magnifying glass
			// Should check the current pointer region to identify the type
			// of cursor to use
			_cursorResource = new GraphicResource(_resPack, _sceneResource->getWorldStats()->commonRes.curMagnifyingGlass);
			_rightButton    = false;
		}
		break;

	case Common::EVENT_RBUTTONDOWN:
		if (ScriptMan.isInputAllowed())
			_rightButton = true;
		break;
	}

	if (doUpdate || _leftClick)
		update();
}

void Scene::updateCursor() {
	int             action    = _sceneResource->getMainActor()->getCurrentAction();
	CommonResources *cr       = &_sceneResource->getWorldStats()->commonRes;
	uint32          newCursor = 0;

	// Change cursor
	switch (action) {
	case kWalkN:
		newCursor = cr->curScrollUp;
		break;
	case kWalkNE:
		newCursor = cr->curScrollUpRight;
		break;
	case kWalkNW:
		newCursor = cr->curScrollUpLeft;
		break;
	case kWalkS:
		newCursor = cr->curScrollDown;
		break;
	case kWalkSE:
		newCursor = cr->curScrollDownRight;
		break;
	case kWalkSW:
		newCursor = cr->curScrollDownLeft;
		break;
	case kWalkW:
		newCursor = cr->curScrollLeft;
		break;
	case kWalkE:
		newCursor = cr->curScrollRight;
		break;
	}

	if (_cursorResource->getEntryNum() != newCursor) {
		delete _cursorResource;
		_cursorResource = new GraphicResource(_resPack, newCursor);
	}
}

void Scene::animateCursor() {
	_curMouseCursor += _cursorStep;
	if (_curMouseCursor == 0)
		_cursorStep = 1;
	if (_curMouseCursor == _cursorResource->getFrameCount() - 1)
		_cursorStep = -1;

	Shared.getScreen()->setCursor(_cursorResource, _curMouseCursor);
}

// -------------------------------------------
// ---------- PROCESS SCENE REGION -----------
// -------------------------------------------

void Scene::update() {
	if(updateScene())
        return;

    // TODO: check game quality
    drawScene();

    //TODO: other process stuffs from sub 0040AE30
}

int Scene::updateScene() {
    uint32       startTick   = 0;
    WorldStats   *worldStats = _sceneResource->getWorldStats();
    
    // Actors
    startTick = Shared.getMillis();
    for (uint32 a = 0; a < worldStats->numActors; a++) {
        updateActor(a);
    }
    debugC(kDebugLevelScene, "UpdateActors Time: %d", Shared.getMillis() - startTick);

    // Barriers
    startTick = Shared.getMillis();
    updateBarriers(worldStats);
    debugC(kDebugLevelScene, "UpdateBarriers Time: %d", Shared.getMillis() - startTick);

    // Ambient Sounds
    startTick = Shared.getMillis();
    updateAmbientSounds();
    debugC(kDebugLevelScene, "UpdateAmbientSounds Time: %d", Shared.getMillis() - startTick);

    // Music
    startTick = Shared.getMillis();
    updateMusic();
    debugC(kDebugLevelScene, "UpdateMusic Time: %d", Shared.getMillis() - startTick);

    // Adjust Screen
    startTick = Shared.getMillis();
    updateAdjustScreen();
    debugC(kDebugLevelScene, "AdjustScreenStart Time: %d", Shared.getMillis() - startTick);

    if(ScriptMan.processActionList())
        return 1;

    return 0;
}

int Scene::isActorVisible(ActorItem *actor) {
    return actor->flags & 1;
}

void Scene::updateActor(uint32 actorIdx) {
    WorldStats   *worldStats = _sceneResource->getWorldStats();
    ActorItem *actor = &worldStats->actors[actorIdx];
    
    if (isActorVisible(actor)) {
        switch (actor->field_40) {
        
        case 0x10:
            if (worldStats->numChapter == 2) {
                // TODO: updateCharacterSub14()
            } else if (worldStats->numChapter == 1) {
                if (_playerActorIdx == actorIdx) {
                    // TODO: updateActorSub21();
                }
            }
            break;
        case 0x11:
            if (worldStats->numChapter == 2) {
                // TODO: put code here
            } else if (worldStats->numChapter == 11) {
                if (_playerActorIdx == actorIdx) {
                    // TODO: put code here
                }
            }
            break;
        case 0xF:
            if (worldStats->numChapter == 2) {
                // TODO: put code here
            } else if (worldStats->numChapter == 11) {
                // TODO: put code here
            }
            break;
        case 0x12:
            if (worldStats->numChapter == 2) {
                // TODO: put code here
            }
            break;
        case 0x5: {
            uint32 frameNum = actor->frameNum + 1;
            actor->frameNum = frameNum % actor->frameCount;

            if (Shared.getMillis() - actor->tickValue1 > 300) {
                if (rand() % 100 < 50) {
                    // TODO: check sound playing
                }
                actor->tickValue1 = Shared.getMillis();
            }
        }        
            break;
        case 0xC:
            if (worldStats->numChapter == 2) {
                // TODO: put code here
            } else if (worldStats->numChapter == 11) {
                // TODO: put code here
            }
        case 0x1:
            // TODO: do actor direction
            break;
        case 0x2:
        case 0xD:
            // TODO: do actor direction
            break;
        case 0x3:
        case 0x13:
            // TODO: updateCharacterSub05();
            break;
        case 0x7:
            // TODO: something
            break;
        case 0x4:
            if (actor->field_944 != 5) {
                // TODO: updateCharacterSub01_sw(1, actorIdx);
            }
            break;
        case 0xE:
            // TODO: updateCharacterSub02(1, actorIdx);
            break;
        case 0x15:
            // TODO: updateCharacterSub06(1, actorIdx);
            break;
        case 0x9:
            // TODO: updateCharacterSub03(1, actorIdx);
            break;
        case 0x6:
        case 0xA:
            actor->frameNum = (actor->frameNum + 1) % actor->frameCount;
            break;
        case 0x8:
            // TODO: actor sound
            break;
        default:
            break;
        }
    }
}

bool Scene::isBarrierVisible(BarrierItem *barrier) {
    if ((barrier->flags & 0xFF) & 1) {
        for (uint32 f = 0; f < 10; f++) {
            bool   isSet = false;
            uint32 flag  = barrier->gameFlags[f];

            if (flag <= 0) {
                isSet = Shared.isGameFlagNotSet(flag); // -flag
            } else {
                isSet = Shared.isGameFlagSet(flag);
            }

            if(!isSet) {
                return false;
            }
        }

        return true;
    }
    return false;
}


bool Scene::isBarrierOnScreen(BarrierItem *barrier) {
    WorldStats *worldStats = _sceneResource->getWorldStats();
    Common::Rect screenRect = Common::Rect(worldStats->xLeft, worldStats->yTop, worldStats->xLeft + 640, worldStats->yTop + 480);
    Common::Rect barrierRect = barrier->boundingRect;
    barrierRect.translate(barrier->x, barrier->y);
    return isBarrierVisible(barrier) && (barrier->flags & 1) && screenRect.intersects(barrierRect);
}

uint32 Scene::getRandomResId(BarrierItem *barrier) {
    int numRes = 1;
    uint32 rndResId[5];
    for (int i = 0; i < 5; i++) {
        if (barrier->field_68C[i]) {
            rndResId[numRes] = barrier->field_68C[i];
            numRes++;
        }
    }
    return rndResId[rand() % numRes];
}

void Scene::updateBarriers(WorldStats *worldStats) {
    Screen *screen = Shared.getScreen();

    uint barriersCount  = worldStats->barriers.size();
    int  startTickCount = 0;
    bool canPlaySound   = false;

    if (barriersCount > 0) {
        for (uint32 b = 0; b < barriersCount; b++) {
            BarrierItem *barrier = &worldStats->barriers[b];

            if (barrier->field_3C == 4) {
                if (isBarrierVisible(barrier)) {
                    uint32 flag = barrier->flags;
                    if (flag & 0x20) {
                        if (Shared.getMillis() - barrier->tickCount >= 0x3E8 / (barrier->field_B4+1)) {
                            barrier->frameIdx = (barrier->frameIdx + 1) % barrier->frameCount;
                            barrier->tickCount = Shared.getMillis();
                            canPlaySound = true;
                        }
                    } else if (flag & 0x10) {
                        uint32 frameIdx  = barrier->frameIdx;
                        char   equalZero = frameIdx == 0;
                        char   lessZero  = frameIdx < 0;
                        if (!frameIdx) {
                            if (Shared.getMillis() - barrier->tickCount >= 1000 * barrier->tickCount2) {
                                if (rand() % barrier->field_C0 == 1) {
                                    if (barrier->field_68C) {
                                        // TODO: fix this, and find a better way to get frame count
                                        // Sometimes we get wrong random resource id

                                        /*barrier->resId = getRandomResId(barrier);
                                        GraphicResource *gra = new GraphicResource(_resPack, barrier->resId);
                                        barrier->frameCount = gra->getFrameCount();
                                        delete gra; */
                                    }
                                    barrier->frameIdx++;
                                }
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound = true;
                            }
                            frameIdx  = barrier->frameIdx;
                            equalZero = frameIdx == 0;
                            lessZero  = frameIdx < 0;
                        }

                        if (!(lessZero ^ 0 | equalZero)) {
                            // FIXME: we shouldn't increment field_B4 (check why this value came zero sometimes)
                            if (Shared.getMillis() - barrier->tickCount >= 0x3E8 / (barrier->field_B4+1)) {
                                barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound = true;
                            }
                        }
                    } else if (flag & 8) {
                        // FIXME: we shouldn't increment field_B4 (check why this value came zero sometimes)
						if (Shared.getMillis() - barrier->tickCount >= 0x3E8 / (barrier->field_B4+1)) {
							uint32 frameIdx = barrier->frameIdx + 1;
                            if (frameIdx < barrier->frameCount - 1) {
                                if (barrier->field_688 == 1) {
                                    // TODO: get global x, y positions
                                }
                            } else {
                                barrier->flags &= 0xFFFFFFF7;
                                if (barrier->field_688 == 1) {
                                    // TODO: reset global x, y positions
                                }
                            }
                            barrier->frameIdx = frameIdx;
						}
                    } else if ((flag & 0xFF) & 8) { // check this
                        if (Shared.getMillis() - barrier->tickCount >= 1000 * barrier->tickCount2) {
							if (rand() % barrier->field_C0 == 1) { // TODO: THIS ISNT WORKING
								barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound = true;
							}
					    }
					} else if (!((flag & 0xFFFF) & 6)) {
                        // FIXME: we shouldn't increment field_B4 (check why this value came zero sometimes)
                        if ((Shared.getMillis() - barrier->tickCount >= 0x3E8 / (barrier->field_B4+1)) && (flag & 0x10000)) {
                            uint32 frameIdx = barrier->frameIdx - 1;
                            if (frameIdx <= 0) {
                                barrier->flags &= 0xFFFEFFFF;
                                if (barrier->field_688 == 1) {
                                    // TODO: reset global x, y positions
                                }
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound = true;
                            }
                            if (barrier->field_688 == 1) {
                                // TODO: get global x, y positions
                            }
                            barrier->frameIdx = frameIdx;
                        } else if (Shared.getMillis() - barrier->tickCount >= 0x3E8 / (barrier->field_B4+1)) {
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
                                if (barrier->field_B4 < 0) // FIXME: check this
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
                    // TODO: play sounds
                }

                // TODO: get sound functions according with scene
            }
        }
    }
}

void Scene::updateAmbientSounds() {
}

void Scene::updateMusic() {
}

void Scene::updateAdjustScreen() {
}


void Scene::OLD_UPDATE(WorldStats *worldStats) {
	int32 curHotspot = -1;
	int32 curBarrier = -1;
   	MainActor *mainActor = _sceneResource->getMainActor();

	// DEBUG
	// Force the screen to scroll if the mouse approaches the edges
	//debugScreenScrolling(bg);

	// DEBUGGING
	// Check current walk region
	for (uint32 a = 0; a < worldStats->numActions; a++) {
		if (worldStats->actions[a].actionType == 0) {
			ActionArea *area = &worldStats->actions[a];
			PolyDefinitions poly = _sceneResource->getGamePolygons()->polygons[area->polyIdx];
			if (Shared.pointInPoly(&poly, mainActor->_actorX, mainActor->_actorY)) {
				debugShowWalkRegion(&poly);
				//break;
			}
		}
	}
	uint32 newCursor = 0;
	if (!_rightButton) {
		if (_sceneResource->getWorldStats()->actors[0].flags & 0x01) {	// TESTING - only draw if visible flag
			// Check if the character was walking before the right-button
			// was released. If so, change the resource to one where he/she
			// is standing still, facing the last active direction
			if (_walking) {
				int currentAction = mainActor->getCurrentAction();
				if (currentAction > 0)
					mainActor->setAction(currentAction + 5);
				_walking = false;
			}
			mainActor->drawActor();
		}
	} else {
		_walking = true;

		mainActor->walkTo(_mouseX, _mouseY);
		updateCursor();
	}

	if (g_debugPolygons)
		debugShowPolygons();
	if (g_debugBarriers)
		debugShowBarriers();

	// Check if we're within a barrier
	for (uint32 p = 0; p < worldStats->numBarriers; p++) {
		BarrierItem b = worldStats->barriers[p];
		if (b.flags & 0x20) {
			if ((b.boundingRect.left + b.x <= _mouseX + _startX) &&
				(_mouseX + _startX < b.boundingRect.right + b.x) &&
				(b.boundingRect.top + b.y <= _mouseY + _startY) &&
				(_mouseY + _startY < b.boundingRect.bottom + b.y)) {
				animateCursor();
				curBarrier = (int32)p;
				break;
			}
		}
	}

	// FIXME? I'm assigning a higher priority to barriers than polygons. I'm assuming
	// that barriers that overlap polygons will have actions associated with them, and
	// the polygon will be part of a walk/look region (so it's accessible elsewhere).
	// This could be completely wrong, and if so, we just have to check to see which
	// of the barrier/polygon action scripts should be processed first
	if (curBarrier < 0) {
		// Update cursor if it's in a polygon hotspot
		for (uint32 p = 0; p < _sceneResource->getGamePolygons()->numEntries; p++) {
			PolyDefinitions poly = _sceneResource->getGamePolygons()->polygons[p];
			if (poly.boundingRect.contains(_mouseX + _startX, _mouseY + _startY)) {
				if (Shared.pointInPoly(&poly, _mouseX + _startX, _mouseY + _startY)) {
					curHotspot = (int32)p;
					animateCursor();
					break;
				}
			}
		}
	}

	if (_leftClick) {
		_leftClick = false;

		if (curHotspot >= 0) {
			for (uint32 a = 0; a < worldStats->numActions; a++) {
				if (worldStats->actions[a].polyIdx == (uint32)curHotspot) {
					debugC(kDebugLevelScripts, "Hotspot: 0x%X - \"%s\", poly %d, action lists %d/%d, action type %d, sound res %d\n",
							worldStats->actions[a].id, 
							worldStats->actions[a].name,
							worldStats->actions[a].polyIdx,
							worldStats->actions[a].actionListIdx1,
							worldStats->actions[a].actionListIdx2,
							worldStats->actions[a].actionType,
							worldStats->actions[a].soundResId);
					ScriptMan.setScript(&_sceneResource->getActionList()->actions[worldStats->actions[a].actionListIdx1]);
				}
			}
		} else if (curBarrier >= 0) {
			BarrierItem b = worldStats->barriers[curBarrier];
			debugC(kDebugLevelScripts, "%s: action(%d) sound(%d) flags(%d/%d)\n",
				b.name,
				b.actionListIdx,
				b.soundResId,
				b.flags,
				b.flags2);
			ScriptMan.setScript(getActionList(b.actionListIdx));
		}
	}
}

// ----------------------------------
// ---------- DRAW REGION -----------
// ----------------------------------

int Scene::drawScene() {

    Shared.getScreen()->clearGraphicsInQueue();

    if (_skipDrawScene) {
        Shared.getScreen()->clearScreen();
    } else {
        // Draw scene background
        WorldStats *worldStats = _sceneResource->getWorldStats();    
        GraphicFrame *bg = _bgResource->getFrame(0);
        Shared.getScreen()->copyToBackBuffer(((byte *)bg->surface.pixels) + _startY * bg->surface.w + _startX, bg->surface.w, worldStats->xLeft, worldStats->yTop, 640, 480);

        // TODO: prepare Actors and Barriers draw
        // TODO: draw actors
        drawBarriers();
        // TODO: draw main actor stuff

        Shared.getScreen()->drawGraphicsInQueue();
        
        // TODO: we must get rid of this
        OLD_UPDATE(worldStats);
    }

    return 1;
}

int Scene::drawBarriers() {
    WorldStats *worldStats = _sceneResource->getWorldStats();
    uint barriersCount = worldStats->barriers.size();

    if (barriersCount > 0) {
        for (uint32 b = 0; b < barriersCount; b++) {
            BarrierItem *barrier = &worldStats->barriers[b];

            if (!(barrier->flags & 4) && !((barrier->flags & 0xFF) & 0x40)) {
                if (isBarrierOnScreen(barrier)) {
                    //TODO: need to do something here yet

                    if (barrier->field_67C <= 0 || barrier->field_67C >= 4) { // TODO: still missing a condition for game quality config
                        Shared.getScreen()->addGraphicToQueue(barrier->resId, barrier->frameIdx, barrier->x, barrier->y, (barrier->flags >> 11) & 2, barrier->field_67C - 3, barrier->priority);
                    } else {
                        // TODO: Do Cross Fade
                        // parameters: barrier->resId, barrier->frameIdx, barrier->x, barrier->y, worldStats->commonRes.backgroundImage, worldStats->xLeft, worldStats->yTop, 0, 0, barrier->field_67C - 1
                        Shared.getScreen()->addGraphicToQueue(barrier->resId, barrier->frameIdx, barrier->x, barrier->y, 0, 0, 0);
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
	Common::Rect screenRect(_startX, _startY, _startX + 640, _startY + 480);
	Common::Rect animRect(x, y, x + surface->w, y + surface->h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate anim rectangle
		animRect.translate(-_startX, -_startY);

		int startX = animRect.right  == 640 ? 0 : surface->w - animRect.width();
		int startY = animRect.bottom == 480 ? 0 : surface->h - animRect.height();

		if (surface->w > 640)
			startX = _startX;
		if (surface->h > 480)
			startY = _startY;

		Shared.getScreen()->copyToBackBufferWithTransparency(
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
	if (_mouseX < SCREEN_EDGES && _startX >= SCROLL_STEP)
		_startX -= SCROLL_STEP;
	else if (_mouseX > 640 - SCREEN_EDGES && _startX <= bg->surface.w - 640 - SCROLL_STEP)
		_startX += SCROLL_STEP;

	// Vertical scrolling
	if (_mouseY < SCREEN_EDGES && _startY >= SCROLL_STEP)
		_startY -= SCROLL_STEP;
	else if (_mouseY > 480 - SCREEN_EDGES && _startY <= bg->surface.h - 480 - SCROLL_STEP)
		_startY += SCROLL_STEP;
}

// WALK REGION DEBUG
void Scene::debugShowWalkRegion(PolyDefinitions *poly) {
	Graphics::Surface surface;
	surface.create(poly->boundingRect.right - poly->boundingRect.left + 1,
			poly->boundingRect.bottom - poly->boundingRect.top + 1,
			1);

	// Draw all lines in Polygon
	for (uint32 i=0; i < poly->numPoints; i++) {
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
	for (uint32 p = 0; p < _sceneResource->getGamePolygons()->numEntries; p++) {
		Graphics::Surface surface;
		PolyDefinitions poly = _sceneResource->getGamePolygons()->polygons[p];
		surface.create(poly.boundingRect.right - poly.boundingRect.left + 1,
				poly.boundingRect.bottom - poly.boundingRect.top + 1,
				1);
		
		// Draw all lines in Polygon
		for (uint32 i=0; i < poly.numPoints; i++) {
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
	for (uint32 p = 0; p < _sceneResource->getWorldStats()->numBarriers; p++) {
		Graphics::Surface surface;
		BarrierItem b = _sceneResource->getWorldStats()->barriers[p];

		if (b.flags & 0x20) {
			surface.create(b.boundingRect.right - b.boundingRect.left + 1,
					b.boundingRect.bottom - b.boundingRect.top + 1,
					1);
			surface.frameRect(b.boundingRect, 0x22);
			copyToBackBufferClipped(&surface, b.x, b.y);
		}

		surface.free();
	}
}

} // end of namespace Asylum
