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
#include "asylum/shared.h"
#include "asylum/scriptman.h"
#include "asylum/actor.h"

namespace Asylum {

#define SCREEN_EDGES 40
#define SCROLL_STEP 10

int g_debugPolygons;
int g_debugBarriers;

Scene::Scene(uint8 sceneIdx) {
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

	if (Common::String(sceneTag,6) != "DFISCN")
		error("The file isn't recognized as scene %s", filename);

	_ws = new WorldStats(fd);
	// jump to game polygons data
	fd->seek(0xE8686);
	_polygons = new Polygons(fd);
	// jump to action list data
	fd->seek(0xE868E + _polygons->size * _polygons->numEntries);
	_actions = new ActionList(fd);

	fd->close();
	delete fd;

	_text 		= new Text(Shared.getScreen());
	_resPack 	= new ResourcePack(sceneIdx);
	_speechPack = new ResourcePack(3); // FIXME are all scene speech packs the same?

	// FIXME
	// Is there EVER more than one actor enabled for a scene? I can't
	// remember, so I guess a playthrough is in order :P
	// Either way, this is kinda dumb
	for (uint8 i = 0; i < _ws->numActors; i++)
		_ws->actors[i].setResourcePack(_resPack);

	_text->loadFont(_resPack, _ws->commonRes.font1);

	char musPackFileName[10];
	sprintf(musPackFileName, "mus.%03d", sceneIdx);
	_musPack = new ResourcePack(musPackFileName);

	_bgResource    = new GraphicResource(_resPack, _ws->commonRes.backgroundImage);
	_blowUp        = 0;
	_cursor        = new Cursor(_resPack);
	_background    = 0;
	_leftClick     = false;
	_rightButton   = false;
	_isActive      = false;
    _skipDrawScene = 0;

	g_debugPolygons = 0;
	g_debugBarriers = 0;

    Shared.setGameFlag(183);

    // TODO: do all the rest stuffs in sub at address 40E460
    _playerActorIdx = 0;

    if (_ws->numBarriers > 0) {
        uint32 priority = 0x0FFB;
        for (uint32 b = 0; b < _ws->numBarriers; b++) {
            Barrier *barrier  = &_ws->barriers[b];
            barrier->priority = priority;
            barrier->flags &= 0xFFFF3FFF;
            priority -= 4;
        }
    }

    _ws->sceneRectIdx = 0;
    Shared.getScreen()->clearGraphicsInQueue();
    _ws->motionStatus = 1;

    // TODO: do some rect stuffs from player actor
    // TODO: reset actors flags
}

Scene::~Scene() {
	delete _ws;
	delete _polygons;
	delete _actions;

	delete _cursor;
	delete _bgResource;
	delete _musPack;
	delete _speechPack;
	delete _resPack;
	delete _text;
    delete _blowUp;
}

Actor* Scene::getActor() {
	return &_ws->actors[_playerActorIdx];
}

void Scene::enterScene() {
    WorldStats *ws = _ws;
	Shared.getScreen()->setPalette(_resPack, ws->commonRes.palette);
	_background = _bgResource->getFrame(0);
	Shared.getScreen()->copyToBackBuffer(
            ((byte *)_background->surface.pixels) + ws->targetY * _background->surface.w + ws->targetX, _background->surface.w,
			0, 0, 640, 480);

	_cursor->load(_ws->commonRes.curMagnifyingGlass);
	_cursor->show();

	// Music testing: play the first music track
	Shared.getSound()->playMusic(_musPack, 0);

	_isActive = true;
	_walking  = false;
}

ActionDefinitions* Scene::getDefaultActionList() {
	getActionList(_ws->actionListIdx);
}

ActionDefinitions* Scene::getActionList(int actionListIndex) {
	if ((actionListIndex >= 0) && (actionListIndex < (int)_ws->numActions))
		return &actions()->entries[actionListIndex];
	else
		return 0;
}

void Scene::setScenePosition(int x, int y)
{
    WorldStats *ws = _ws;
	GraphicFrame *bg = _bgResource->getFrame(0);
	//_startX = x;
	//_startY = y;
    ws->targetX = x;
    ws->targetY = y;
	
	if (ws->targetX < 0)
		ws->targetX = 0;
	if (ws->targetX > (bg->surface.w - 640))
		ws->targetX = bg->surface.w - 640;
		
	
	if (ws->targetY < 0)
		ws->targetY = 0;
	if (ws->targetY > (bg->surface.h - 480))
		ws->targetY = bg->surface.h - 480;		
}

void Scene::handleEvent(Common::Event *event, bool doUpdate) {
	_ev = event;

	switch (_ev->type) {

	case Common::EVENT_MOUSEMOVE:
		_cursor->setCoords(_ev->mouse.x, _ev->mouse.y);
		break;

	case Common::EVENT_LBUTTONUP:
		if (ScriptMan.allowInput)
			_leftClick = true;
		break;

	case Common::EVENT_RBUTTONUP:
		if (ScriptMan.allowInput) {
			// TODO This isn't always going to be the magnifying glass
			// Should check the current pointer region to identify the type
			// of cursor to use
			_cursor->load(_ws->commonRes.curMagnifyingGlass);
			_rightButton    = false;
		}
		break;

	case Common::EVENT_RBUTTONDOWN:
		if (ScriptMan.allowInput)
			_rightButton = true;
		break;
	}

	if (doUpdate || _leftClick)
		update();
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
	uint32     startTick   = 0;
    WorldStats *worldStats = _ws;
    
    // Mouse
    startTick = Shared.getMillis();
    updateMouse();
    debugC(kDebugLevelScene, "UpdateMouse Time: %d", Shared.getMillis() - startTick);

    // Actors
    startTick = Shared.getMillis();
    for (uint32 a = 0; a < worldStats->numActors; a++)
        updateActor(a);
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
    //startTick = Shared.getMillis();
    // FIXME
    // Commented out the (incomplete) update screen code because once the
    // actor's x1/y1 values are properly set, the temp code causes a crash
    // Have to finish implementing the method I guess :P
    //updateAdjustScreen();
    //debugC(kDebugLevelScene, "AdjustScreenStart Time: %d", Shared.getMillis() - startTick);

    if(ScriptMan.processActionList())
        return 1;

    return 0;
}

void Scene::updateMouse() {
	Common::Rect actorPos;

	if (_sceneIdx != 2 || _playerActorIdx != 10) {
		actorPos.top    = getActor()->y;
		actorPos.left   = getActor()->x + 20;
		actorPos.right  = getActor()->x + 2 + getActor()->x2;
		actorPos.bottom = getActor()->y + getActor()->y2;
	} else {
		actorPos.top    = getActor()->y + 60;
		actorPos.left   = getActor()->x + 50;
		actorPos.right  = getActor()->x + getActor()->x2 + 10;
		actorPos.bottom = getActor()->y + getActor()->y2 - 20;
	}

	int  dir = -1;
	bool done = false;

	if (_cursor->x() < actorPos.left) {
		if (_cursor->y() >= actorPos.top) {
			if (_cursor->y() > actorPos.bottom) {
				if (getActor()->direction == 2) {
					if (_cursor->y() - actorPos.bottom > 10)
						dir = 3;
				} else {
					if (getActor()->direction == 4) {
						if (actorPos.left - _cursor->x() > 10)
							dir = 3;
					} else {
						dir = 3;
					}
				}
			} else {
				if (getActor()->direction == 1) {
					if (_cursor->y() - actorPos.top > 10)
						dir = 2;
				} else {
					if (getActor()->direction == 3) {
						if (actorPos.bottom - _cursor->y() > 10)
							dir = 2;
					} else {
						dir = 2;
					}
				}
			}
		} else {
			if (getActor()->direction) {
				if (getActor()->direction == 2) {
					if (actorPos.top - _cursor->y() > 10)
						dir = 1;
				} else {
					dir = 1;
				}
			} else {
				if (actorPos.left - _cursor->x() > 10)
					dir = 1;
			}
		}
		done = true;
	}

	if (!done && _cursor->x() <= actorPos.right) {
		if (_cursor->y() >= actorPos.top) {
			if (_cursor->y() > actorPos.bottom) {
				if (getActor()->direction == 3) {
					if (_cursor->x() - actorPos.left > 10)
						dir = 4;
				} else {
					if (getActor()->direction == 5) {
						if (actorPos.right - _cursor->x() > 10)
							dir = 4;
					} else {
						dir = 4;
					}
				}
			}
		} else {
			if (getActor()->direction == 1) {
				if (_cursor->x() - actorPos.left > 10)
					dir = 0;
			} else {
				if (getActor()->direction == 7) {
					if (actorPos.right - _cursor->x() > 10)
						dir = 0;
				} else {
					dir = 0;
				}
			}
		}
		done = true;
	}

	if (!done && _cursor->y() < actorPos.top) {
		if (getActor()->direction) {
			if (getActor()->direction == 6) {
				if (actorPos.top - _cursor->y() > 10)
					dir = 7;
			} else {
				dir = 7;
			}
		} else {
			if (_cursor->x() - actorPos.right > 10)
				dir = 7;
		}
		done = true;
	}

	if (!done && _cursor->y() <= actorPos.bottom) {
		if (getActor()->direction == 5) {
			if (actorPos.bottom - _cursor->y() > 10)
				dir = 6;
		} else {
			if (getActor()->direction == 7) {
				if (_cursor->y() - actorPos.top > 10)
					dir = 6;
			} else {
				dir = 6;
			}
		}
		done = true;
	}

	if (!done && getActor()->direction == 4) {
		if (_cursor->x() - actorPos.right <= 10)
			done = true;
		if (!done)
			dir = 5;
	}

	if (!done && (getActor()->direction != 6 || _cursor->y() - actorPos.bottom > 10)) {
		dir = 5;
	}

	//printf("Current Dir %d -- New Dir %d\n", actor->direction, dir);
}

void Scene::updateActor(uint32 actorIdx) {
    WorldStats *ws    = _ws;
    Actor      *actor = getActor();
    
    if (actor->visible()) {
    	// printf("Actor field_40 = 0x%02X\n", actor->field_40);

        switch (actor->field_40) {
        
        case 0x10:
            if (ws->numChapter == 2) {
                // TODO: updateCharacterSub14()
            } else if (ws->numChapter == 1) {
                if (_playerActorIdx == actorIdx) {
                    // TODO: updateActorSub21();
                }
            }
            break;
        case 0x11:
            if (ws->numChapter == 2) {
                // TODO: put code here
            } else if (ws->numChapter == 11) {
                if (_playerActorIdx == actorIdx) {
                    // TODO: put code here
                }
            }
            break;
        case 0xF:
            if (ws->numChapter == 2) {
                // TODO: put code here
            } else if (ws->numChapter == 11) {
                // TODO: put code here
            }
            break;
        case 0x12:
            if (ws->numChapter == 2) {
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
            if (ws->numChapter == 2) {
                // TODO: put code here
            } else if (ws->numChapter == 11) {
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

void Scene::updateBarriers(WorldStats *worldStats) {
    Screen *screen = Shared.getScreen();

    uint barriersCount  = worldStats->barriers.size();
    int  startTickCount = 0;
    bool canPlaySound   = false;

    if (barriersCount > 0) {
        for (uint32 b = 0; b < barriersCount; b++) {
            Barrier *barrier = &worldStats->barriers[b];

            if (barrier->field_3C == 4) {
                if (barrier->visible()) {
                    uint32 flag = barrier->flags;
                    if (flag & 0x20) {
                        if (barrier->field_B4 && (Shared.getMillis() - barrier->tickCount >= 0x3E8 / barrier->field_B4)) {
                            barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
                            barrier->tickCount = Shared.getMillis();
                            canPlaySound       = true;
                        }
                    } else if (flag & 0x10) {
                        uint32 frameIdx  = barrier->frameIdx;
                        char   equalZero = frameIdx == 0;
                        char   lessZero  = frameIdx < 0;
                        if (!frameIdx) {
                            if (Shared.getMillis() - barrier->tickCount >= 1000 * barrier->tickCount2) {
                                if (rand() % barrier->field_C0 == 1) {
                                    if (barrier->field_68C[0]) {
                                        // TODO: fix this, and find a better way to get frame count
                                        // Sometimes we get wrong random resource id

                                        barrier->resId = barrier->getRandomId();
                                        GraphicResource *gra = new GraphicResource(_resPack, barrier->resId);
                                        barrier->frameCount  = gra->getFrameCount();
                                        delete gra;
                                    }
                                    barrier->frameIdx++;
                                }
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound       = true;
                            }
                            frameIdx  = barrier->frameIdx;
                            equalZero = frameIdx == 0;
                            lessZero  = frameIdx < 0;
                        }

                        if (!(lessZero ^ 0 | equalZero)) {
                            // FIXME: we shouldn't increment field_B4 (check why this value came zero sometimes)
                            if (barrier->field_B4 && (Shared.getMillis() - barrier->tickCount >= 0x3E8 / barrier->field_B4)) {
                                barrier->frameIdx  = (barrier->frameIdx + 1) % barrier->frameCount;
                                barrier->tickCount = Shared.getMillis();
                                canPlaySound = true;
                            }
                        }
                    } else if (flag & 8) {
                        // FIXME: we shouldn't increment field_B4 (check why this value came zero sometimes)
						if (barrier->field_B4 && (Shared.getMillis() - barrier->tickCount >= 0x3E8 / barrier->field_B4)) {
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
                        if (barrier->field_B4 && (Shared.getMillis() - barrier->tickCount >= 0x3E8 / barrier->field_B4) && (flag & 0x10000)) {
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
                        } else if (barrier->field_B4 && (Shared.getMillis() - barrier->tickCount >= 0x3E8 / barrier->field_B4)) {
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
	WorldStats *ws = _ws;

	int v5, v6, v7, v15, v16;
	int v1 = -1;
	int v0 = -1;

	if (ws->motionStatus == 1) {
		v5 = getActor()->x1 - ws->xLeft;
		v7 = getActor()->y1 - ws->yTop;
		if (v5 < ws->boundingRect.left || v5 > ws->boundingRect.right) {
			v15 = ws->boundingRect.left - ws->boundingRect.right;
			v1 = v15 + ws->xLeft;
			ws->xLeft += v15;
		}
		if (v7 < ws->boundingRect.top || v7 > ws->boundingRect.bottom) {
			v16 = v7 - ws->boundingRect.bottom;
			v0 = v16 + ws->yTop;
			ws->yTop += v16;
		}
		if (v1 < 0)
			v1 = ws->xLeft = 0;
		if (v1 > ws->width - 640) {
			v1 = ws->width - 640;
			ws->xLeft = v1;
		}
		if (v0 < 0)
			v0 = ws->yTop = 0;
		if (v0 > ws->height - 480) {
			v0 = ws->height - 480;
			ws->yTop = v0;
		}
	} else {
		if (ws->motionStatus == 2 || ws->motionStatus == 5) {
			if (ws->motionStatus != 3) {
				// TODO
				/*
				 __asm
				{
				  fld     flt_543514
				  fadd    flt_543518
				  fstp    flt_543514
				  fild    scene.field_98
				  fsubr   flt_54350C
				  fild    scene.field_9C
				  fsubr   flt_543510
				}
				v12 = abs(_ftol());
				if ( v12 <= abs(_ftol()) )
				{
				  v2 = scene.field_9C;
				  if ( scene.field_9C != scene.yTop )
				  {
					__asm
					{
					  fld     flt_543514
					  fadd    flt_54350C
					}
					v14 = _ftol();
					v1 = v14;
					scene.xLeft = v14;
				  }
				  v4 = scene.field_A0;
				  v0 += scene.field_A0;
				  scene.yTop = v0;
				  v3 = v0;
				}
				else
				{
				  v2 = scene.field_98;
				  if ( scene.field_98 != scene.xLeft )
				  {
					__asm
					{
					  fld     flt_543514
					  fadd    flt_543510
					}
					v13 = _ftol();
					v0 = v13;
					scene.yTop = v13;
				  }
				  v4 = scene.field_A0;
				  v1 += scene.field_A0;
				  scene.xLeft = v1;
				  v3 = v1;
				}
				if ( abs(v3 - v2) <= abs(v4) )
				{
				  scene.field_88 = 3;
				  scene.field_98 = -1;
				}
			  }
			*/
			}
		}
	}
	/*
	v9 = 16 * scene.sceneRectIndex;
	if ( v1 < *(LONG *)((char *)&scene.sceneRects[0].left + v9) )
	{
	v1 = *(LONG *)((char *)&scene.sceneRects[0].left + v9);
	scene.xLeft = *(LONG *)((char *)&scene.sceneRects[0].left + v9);
	}
	if ( v0 < *(LONG *)((char *)&scene.sceneRects[0].top + v9) )
	{
	v0 = *(LONG *)((char *)&scene.sceneRects[0].top + v9);
	scene.yTop = *(LONG *)((char *)&scene.sceneRects[0].top + v9);
	}
	v10 = *(LONG *)((char *)&scene.sceneRects[0].right + v9);
	if ( v1 + 639 > v10 )
	{
	v1 = v10 - 639;
	scene.xLeft = v10 - 639;
	}
	result = *(LONG *)((char *)&scene.sceneRects[0].bottom + v9);
	if ( v0 + 479 > result )
	{
	v0 = result - 479;
	scene.yTop = result - 479;
	}
	if ( v17 != v1 || v18 != v0 )
	dword_44E1EC = 2

	 */
}


void Scene::OLD_UPDATE(WorldStats *worldStats) {
	int32 curHotspot = -1;
	int32 curBarrier = -1;

	// DEBUGGING
	// Check current walk region
	for (uint32 a = 0; a < worldStats->numActions; a++) {
		if (worldStats->actions[a].actionType == 0) {
			ActionArea *area = &worldStats->actions[a];
			PolyDefinitions poly = _polygons->entries[area->polyIdx];
			if (poly.contains(getActor()->x, getActor()->y)) {
				debugShowWalkRegion(&poly);
				//break;
			}
		}
	}

	if (!_rightButton) {
		if (_ws->actors[0].flags & 0x01) {	// TESTING - only draw if visible flag
			// Check if the character was walking before the right-button
			// was released. If so, change the resource to one where he/she
			// is standing still, facing the last active direction
			if (_walking) {
				if (getActor()->currentAction > 0)
					getActor()->setAction(getActor()->currentAction + 5);
				_walking = false;
			}
			getActor()->drawActor();
		}
	} else {
		_walking = true;

		getActor()->walkTo(_cursor->x(), _cursor->y());
		_cursor->update(&_ws->commonRes, getActor()->direction);
	}

	if (g_debugPolygons)
		debugShowPolygons();
	if (g_debugBarriers)
		debugShowBarriers();

	// Check if we're within a barrier
	for (uint32 p = 0; p < worldStats->numBarriers; p++) {
		Barrier b = worldStats->barriers[p];
		if (b.flags & 0x20) {
            if ((b.boundingRect.left + b.x <= _cursor->x() + worldStats->targetX) &&
				(_cursor->x() + worldStats->targetX < b.boundingRect.right + b.x) &&
				(b.boundingRect.top + b.y <= _cursor->y() + worldStats->targetY) &&
				(_cursor->y() + worldStats->targetY < b.boundingRect.bottom + b.y)) {
				_cursor->animate();
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
		for (uint32 p = 0; p < _polygons->numEntries; p++) {
			PolyDefinitions poly = _polygons->entries[p];
			if (poly.boundingRect.contains(_cursor->x() + worldStats->targetX, _cursor->y() + worldStats->targetY)) {
				if (poly.contains(_cursor->x() + worldStats->targetX, _cursor->y() + worldStats->targetY)) {
					curHotspot = (int32)p;
					_cursor->animate();
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
					ScriptMan.setScript(&_actions->entries[worldStats->actions[a].actionListIdx1]);
				}
			}
		} else if (curBarrier >= 0) {
			Barrier b = worldStats->barriers[curBarrier];
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
        WorldStats   *ws = _ws;
        GraphicFrame *bg = _bgResource->getFrame(0);
        Shared.getScreen()->copyToBackBuffer(
        		((byte *)bg->surface.pixels) + ws->targetY * bg->surface.w + ws->targetX, bg->surface.w,
        		ws->xLeft,
        		ws->yTop,
        		640,
        		480);

        // DEBUG
	    // Force the screen to scroll if the mouse approaches the edges
	    //debugScreenScrolling(bg);

        // TODO: prepare Actors and Barriers draw
        // TODO: draw actors
        drawBarriers();
        // TODO: draw main actor stuff

        Shared.getScreen()->drawGraphicsInQueue();
        
        // TODO: we must get rid of this
        OLD_UPDATE(ws);
    }

    return 1;
}

int Scene::drawBarriers() {
    WorldStats *worldStats = _ws;
    uint barriersCount = worldStats->barriers.size();

    if (barriersCount > 0) {
        for (uint32 b = 0; b < barriersCount; b++) {
            Barrier *barrier = &worldStats->barriers[b];

            if (!(barrier->flags & 4) && !((barrier->flags & 0xFF) & 0x40)) {
                if (barrier->onscreen()) {
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
    WorldStats *ws = _ws;

    Common::Rect screenRect(ws->targetX, ws->targetY, ws->targetX + 640, ws->targetY + 480);
	Common::Rect animRect(x, y, x + surface->w, y + surface->h);
	animRect.clip(screenRect);

	if (!animRect.isEmpty()) {
		// Translate anim rectangle
		animRect.translate(-ws->targetX, -ws->targetY);

		int startX = animRect.right  == 640 ? 0 : surface->w - animRect.width();
		int startY = animRect.bottom == 480 ? 0 : surface->h - animRect.height();

		if (surface->w > 640)
			startX = ws->targetX;
		if (surface->h > 480)
			startY = ws->targetY;

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
    WorldStats *ws = _ws;

	// Horizontal scrolling
	if (_cursor->x() < SCREEN_EDGES && ws->targetX >= SCROLL_STEP)
		ws->targetX -= SCROLL_STEP;
	else if (_cursor->x() > 640 - SCREEN_EDGES && ws->targetX <= bg->surface.w - 640 - SCROLL_STEP)
		ws->targetX += SCROLL_STEP;

	// Vertical scrolling
	if (_cursor->y() < SCREEN_EDGES && ws->targetY >= SCROLL_STEP)
		ws->targetY -= SCROLL_STEP;
	else if (_cursor->y() > 480 - SCREEN_EDGES && ws->targetY <= bg->surface.h - 480 - SCROLL_STEP)
		ws->targetY += SCROLL_STEP;
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
	for (uint32 p = 0; p < _polygons->numEntries; p++) {
		Graphics::Surface surface;
		PolyDefinitions poly = _polygons->entries[p];
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
	for (uint32 p = 0; p < _ws->numBarriers; p++) {
		Graphics::Surface surface;
		Barrier b = _ws->barriers[p];

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
