/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005 The ScummVM project
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
 * $Header$
 *
 */

#include "kyra/kyra.h"
#include "kyra/screen.h"
#include "kyra/animator.h"
#include "kyra/sprites.h"

#include "common/system.h"

namespace Kyra {
ScreenAnimator::ScreenAnimator(KyraEngine *vm, OSystem *system) {
	_vm = vm;
	_screen = vm->screen();
	_initOk = false;
	_updateScreen = false;
	_system = system;
	_screenObjects = _actors = _items = _sprites = _objectQueue = 0;
	_noDrawShapesFlag = 0;
	
	memset(&_kyragemFadingState, 0, sizeof(_kyragemFadingState));
	_kyragemFadingState.gOffset = 0x13;
	_kyragemFadingState.bOffset = 0x13;
}

ScreenAnimator::~ScreenAnimator() {
	close();
}

void ScreenAnimator::init(int actors_, int items_, int sprites_) {
	debug(9, "ScreenAnimator::init(%d, %d, %d)", actors_, items_, sprites_);
	_screenObjects = new AnimObject[actors_ + items_ + sprites_];
	assert(_screenObjects);
	memset(_screenObjects, 0, sizeof(AnimObject) * (actors_ + items_ + sprites_));
	_actors = _screenObjects;
	_sprites = &_screenObjects[actors_];
	_items = &_screenObjects[actors_ + items_];
	_initOk = true;
}

void ScreenAnimator::close() {
	debug(9, "ScreenAnimator::close()");
	if (_initOk) {
		_initOk = false;
		delete [] _screenObjects;
		_screenObjects = _actors = _items = _sprites = _objectQueue = 0;
	}
}

void ScreenAnimator::preserveAllBackgrounds() {
	debug(9, "ScreenAnimator::preserveAllBackgrounds()");
	uint8 curPage = _screen->_curPage;
	_screen->_curPage = 2;

	AnimObject *curObject = _objectQueue;
	while (curObject) {
		if (curObject->active && !curObject->unk1) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	_screen->_curPage = curPage;
}

void ScreenAnimator::flagAllObjectsForBkgdChange() {
	debug(9, "ScreenAnimator::flagAllObjectsForBkgdChange()");
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->bkgdChangeFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::flagAllObjectsForRefresh() {
	debug(9, "ScreenAnimator::flagAllObjectsForRefresh()");
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->refreshFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::restoreAllObjectBackgrounds() {
	debug(9, "ScreenAnimator::restoreAllObjectBackground()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		if (curObject->active && !curObject->unk1) {
			preserveOrRestoreBackground(curObject, true);
			curObject->x2 = curObject->x1;
			curObject->y2 = curObject->y1;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void ScreenAnimator::preserveAnyChangedBackgrounds() {
	debug(9, "ScreenAnimator::preserveAnyChangedBackgrounds()");
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;
	
	while (curObject) {
		if (curObject->active && !curObject->unk1 && curObject->bkgdChangeFlag) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	
	_screen->_curPage = 0;
}

void ScreenAnimator::preserveOrRestoreBackground(AnimObject *obj, bool restore) {
	debug(9, "ScreenAnimator::preserveOrRestoreBackground(0x%X, restore)", obj, restore);
	int x = 0, y = 0, width = obj->width << 3, height = obj->height;
	
	if (restore) {
		x = obj->x2;
		y = obj->y2;
	} else {
		x = obj->x1;
		y = obj->y1;
	}
	
	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;
	
	int temp;
	
	temp = x + width;
	if (temp >= 319) {
		x = 319 - width;
	}
	temp = y + height;
	if (temp >= 136) {
		y = 136 - height;
	}
	
	if (restore) {
		_screen->copyBlockToPage(_screen->_curPage, x, y, width, height, obj->background);
	} else {
		_screen->copyRegionToBuffer(_screen->_curPage, x, y, width, height, obj->background);
	}
}

void ScreenAnimator::prepDrawAllObjects() {
	debug(9, "ScreenAnimator::prepDrawAllObjects()");
	AnimObject *curObject = _objectQueue;
	int drawPage = 2;
	int flagUnk1 = 0, flagUnk2 = 0, flagUnk3 = 0;
	if (_noDrawShapesFlag)
		return;
	if (_vm->_brandonStatusBit & 0x20)
		flagUnk1 = 0x200;
	if (_vm->_brandonStatusBit & 0x40)
		flagUnk2 = 0x4000;

	while (curObject) {
		if (curObject->active) {
			int xpos = curObject->x1;
			int ypos = curObject->y1;
			
			int drawLayer = 0;
			if (!(curObject->flags & 0x800)) {
				drawLayer = 7;
			} else if (curObject->unk1) {
				drawLayer = 0;
			} else {
				drawLayer = _vm->_sprites->getDrawLayer(curObject->drawY);
			}
			
			// talking head functionallity
			if (_vm->_talkingCharNum != -1) {
				const int16 baseAnimFrameTable1[] = { 0x11, 0x35, 0x59, 0x00, 0x00, 0x00 };
				const int16 baseAnimFrameTable2[] = { 0x15, 0x39, 0x5D, 0x00, 0x00, 0x00 };
				const int8 xOffsetTable1[] = { 2, 4, 0, 5, 2, 0, 0, 0 };
				const int8 xOffsetTable2[] = { 6, 4, 8, 3, 6, 0, 0, 0 };
				const int8 yOffsetTable1[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				const int8 yOffsetTable2[] = { 0, 8, 1, 1, 0, 0, 0, 0 };
				if (curObject->index == 0 || curObject->index <= 4) {
					int shapesIndex = 0;
					if (curObject->index == _vm->_charSayUnk3) {
						shapesIndex = _vm->_currHeadShape + baseAnimFrameTable1[curObject->index];
					} else {
						shapesIndex = baseAnimFrameTable2[curObject->index];
						int temp2 = 0;
						if (curObject->index == 2) {
							if (_vm->_characterList[2].sceneId == 77 || _vm->_characterList[2].sceneId == 86) {
								temp2 = 1;
							} else {
								temp2 = 0;
							}
						} else {
							temp2 = 1;
						}
						
						if (!temp2) {
							shapesIndex = -1;
						}
					}
						
					xpos = curObject->x1;
					ypos = curObject->y1;
						
					int tempX = 0, tempY = 0;
					if (curObject->flags & 0x1) {
						tempX = (xOffsetTable1[curObject->index] * _vm->_brandonScaleX) >> 8;
						tempY = yOffsetTable1[curObject->index];
					} else {
						tempX = (xOffsetTable2[curObject->index] * _vm->_brandonScaleX) >> 8;
						tempY = yOffsetTable2[curObject->index];
					}
					tempY = (tempY * _vm->_brandonScaleY) >> 8;
					xpos += tempX;
					ypos += tempY;
					
					if (_vm->_scaleMode && _vm->_brandonScaleX != 256) {
						++xpos;
					}
					
					if (curObject->index == 0 && shapesIndex != -1) {
						if (!(_vm->_brandonStatusBit & 2)) {
							flagUnk3 = 0x100;
							if ((flagUnk1 & 0x200) || (flagUnk2 & 0x4000)) {
								flagUnk3 = 0;
							}
							
							int tempFlags = 0;
							if (flagUnk3 & 0x100) {
								tempFlags = curObject->flags & 1;
								tempFlags |= 0x800 | flagUnk1 | 0x100;
							}
							
							if (!(flagUnk3 & 0x100) && (flagUnk2 & 0x4000)) {
								tempFlags = curObject->flags & 1;
								tempFlags |= 0x900 | flagUnk1 | 0x4000;
								_screen->drawShape(drawPage, _vm->_shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 4, _vm->_brandonPoisonFlagsGFX, int(1), int(_vm->_brandonInvFlag), drawLayer, _vm->_brandonScaleX, _vm->_brandonScaleY);
							} else {
								if (!(flagUnk2 & 0x4000)) {
									tempFlags = curObject->flags & 1;
									tempFlags |= 0x900 | flagUnk1;
								}
								
								_screen->drawShape(drawPage, _vm->_shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 4, _vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _vm->_brandonScaleX, _vm->_brandonScaleY);
							}
						}
					} else {
						if (shapesIndex != -1) {
							int tempFlags = 0;
							if (curObject->flags & 1) {
								tempFlags = 1;
							}
							_screen->drawShape(drawPage, _vm->_shapes[4+shapesIndex], xpos, ypos, 2, tempFlags | 0x800, drawLayer); 							
						}
					}
				}
			}
			
			xpos = curObject->x1;
			ypos = curObject->y1;
			
			curObject->flags |= 0x800;
			if (curObject->index == 0) {
				flagUnk3 = 0x100;
				
				if (flagUnk1 & 0x200 || flagUnk2 & 0x4000) {
					flagUnk3 = 0;
				}
				
				if (_vm->_brandonStatusBit & 2) {
					curObject->flags &= 0xFFFFFFFE;
				}
				
				if (!_vm->_scaleMode) {
					if (flagUnk3 & 0x100) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x100, (uint8*)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer);
					} else if (flagUnk3 & 0x4000) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4000, int(_vm->_brandonInvFlag), drawLayer);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1, drawLayer);
					}
				} else {
					if (flagUnk3 & 0x100) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x104, (uint8*)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _vm->_brandonScaleX, _vm->_brandonScaleY);
					} else if (flagUnk3 & 0x4000) {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4004, int(_vm->_brandonInvFlag), drawLayer, _vm->_brandonScaleX, _vm->_brandonScaleY);
					} else {
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | flagUnk1 | 0x4, drawLayer, _vm->_brandonScaleX, _vm->_brandonScaleY);
					}
				}
			} else {
				if (curObject->index >= 16 && curObject->index <= 27) {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | 4, drawLayer, (int)_vm->_scaleTable[curObject->drawY], (int)_vm->_scaleTable[curObject->drawY]);
				} else {
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags, drawLayer);
				}
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void ScreenAnimator::copyChangedObjectsForward(int refreshFlag) {
	debug(9, "ScreenAnimator::copyChangedObjectsForward(%d)", refreshFlag);
	AnimObject *curObject = _objectQueue;

	while (curObject) {
		if (curObject->active) {
			if (curObject->refreshFlag || refreshFlag) {
				int xpos = 0, ypos = 0, width = 0, height = 0;
				xpos = curObject->x1 - (curObject->width2+1);
				ypos = curObject->y1 - curObject->height2;
				width = (curObject->width + ((curObject->width2)>>3)+1)<<3;
				height = curObject->height + curObject->height2*2;
				
				if (xpos < 8) {
					xpos = 8;
				} else if (xpos + width > 312) {
					width = 312 - xpos;
				}
				
				if (ypos < 8) {
					ypos = 8;
				} else if (ypos + height > 136) {
					height = 136 - ypos;
				}
				
				_screen->copyRegion(xpos, ypos, xpos, ypos, width, height, 2, 0, Screen::CR_CLIPPED);
				curObject->refreshFlag = 0;
				_updateScreen = true;
			}
		}
		curObject = curObject->nextAnimObject;
	}
	if (_updateScreen) {
		_screen->updateScreen();
		_updateScreen = false;
	}
}

void ScreenAnimator::updateAllObjectShapes() {
	debug(9, "ScreenAnimator::updateAllObjectShapes()");
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	copyChangedObjectsForward(0);
}

void ScreenAnimator::animRemoveGameItem(int index) {
	debug(9, "ScreenAnimator::animRemoveGameItem(%d)", index);
	restoreAllObjectBackgrounds();
	
	AnimObject *animObj = &_items[index];
	animObj->sceneAnimPtr = 0;
	animObj->animFrameNumber = -1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;	
	updateAllObjectShapes();
	animObj->active = 0;
	
	objectRemoveQueue(_objectQueue, animObj);
}

void ScreenAnimator::animAddGameItem(int index, uint16 sceneId) {
	debug(9, "ScreenAnimator::animRemoveGameItem(%d, %d)", index, sceneId);
	restoreAllObjectBackgrounds();
	assert(sceneId < _vm->_roomTableSize);
	Room *currentRoom = &_vm->_roomTable[sceneId];
	AnimObject *animObj = &_items[index];
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = currentRoom->itemsYPos[index];
	animObj->sceneAnimPtr = _vm->_shapes[220+currentRoom->itemsTable[index]];
	animObj->animFrameNumber = -1;
	animObj->x1 = currentRoom->itemsXPos[index];
	animObj->y1 = currentRoom->itemsYPos[index];
	animObj->x1 -= _vm->fetchAnimWidth(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]) >> 1;
	animObj->y1 -= _vm->fetchAnimHeight(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]);
	animObj->x2 = animObj->x1;
	animObj->y2 = animObj->y1;
	animObj->width2 = 0;
	animObj->height2 = 0;
	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

void ScreenAnimator::animAddNPC(int character) {
	debug(9, "ScreenAnimator::animAddNPC(%d)", character);
	restoreAllObjectBackgrounds();
	AnimObject *animObj = &_actors[character];
	const Character *ch = &_vm->_characterList[character];
	
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _vm->_shapes[4+ch->currentAnimFrame];
	animObj->x1 = animObj->x2 = ch->x1 + _vm->_defaultShapeTable[ch->currentAnimFrame-7].xOffset;
	animObj->y1 = animObj->y2 = ch->y1 + _vm->_defaultShapeTable[ch->currentAnimFrame-7].yOffset;
	if (ch->facing >= 1 && ch->facing <= 3) {
		animObj->flags |= 1;
	} else if (ch->facing >= 5 && ch->facing <= 7) {
		animObj->flags &= 0xFFFFFFFE;
	}	
	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

AnimObject *ScreenAnimator::objectRemoveQueue(AnimObject *queue, AnimObject *rem) {
	debug(9, "ScreenAnimator::objectRemoveQueue(0x%X, 0x%X)", queue, rem);
	AnimObject *cur = queue;
	AnimObject *prev = queue;

	while (cur != rem && cur) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (cur == queue) {
		if (!cur)
			return 0;
		return cur->nextAnimObject;
	}
	
	if (!cur->nextAnimObject) {
		if (cur == rem) {
			if (!prev) {
				return 0;
			} else {
				prev->nextAnimObject = 0;
			}
		}
	} else {
		if (cur == rem) {
			prev->nextAnimObject = rem->nextAnimObject;
		}
	}
	
	return queue;
}

AnimObject *ScreenAnimator::objectAddHead(AnimObject *queue, AnimObject *head) {
	debug(9, "ScreenAnimator::objectAddHead(0x%X, 0x%X)", queue, head);
	head->nextAnimObject = queue;
	return head;
}

AnimObject *ScreenAnimator::objectQueue(AnimObject *queue, AnimObject *add) {
	debug(9, "ScreenAnimator::objectQueue(0x%X, 0x%X)", queue, add);
	if (add->drawY <= queue->drawY || !queue) {
		add->nextAnimObject = queue;
		return add;
	}
	AnimObject *cur = queue;
	AnimObject *prev = queue;
	while (add->drawY > cur->drawY) {
		AnimObject *temp = cur->nextAnimObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (add->drawY <= cur->drawY) {
		prev->nextAnimObject = add;
		add->nextAnimObject = cur;
	} else {
		cur->nextAnimObject = add;
		add->nextAnimObject = 0;
	}
	return queue;
}

void ScreenAnimator::addObjectToQueue(AnimObject *object) {
	debug(9, "ScreenAnimator::addObjectToQueue(0x%X)", object);
	if (!_objectQueue) {
		_objectQueue = objectAddHead(0, object);
	} else {
		_objectQueue = objectQueue(_objectQueue, object);
	}
}

void ScreenAnimator::refreshObject(AnimObject *object) {
	debug(9, "ScreenAnimator::refreshObject(0x%X)", object);
	_objectQueue = objectRemoveQueue(_objectQueue, object);
	if (_objectQueue) {
		_objectQueue = objectQueue(_objectQueue, object);
	} else {
		_objectQueue = objectAddHead(0, object);
	}
}

void ScreenAnimator::updateKyragemFading() {
	static const uint8 kyraGemPalette[0x28] = {
		0x3F, 0x3B, 0x38, 0x34, 0x32, 0x2F, 0x2C, 0x29, 0x25, 0x22,
		0x1F, 0x1C, 0x19, 0x16, 0x12, 0x0F, 0x0C, 0x0A, 0x06, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	
	if (_system->getMillis() < _kyragemFadingState.timerCount)
		return;
	
	_kyragemFadingState.timerCount = _system->getMillis() + 4 * _vm->tickLength();
	int palPos = 684;
	for (int i = 0; i < 20; ++i) {
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.rOffset];
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.gOffset];
		_screen->_currentPalette[palPos++] = kyraGemPalette[i + _kyragemFadingState.bOffset];
	}
	_screen->setScreenPalette(_screen->_currentPalette);
	_updateScreen = true;
	switch (_kyragemFadingState.nextOperation) {
		case 0:
			--_kyragemFadingState.bOffset;
			if (_kyragemFadingState.bOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 1;
			break;

		case 1:
			++_kyragemFadingState.rOffset;
			if (_kyragemFadingState.rOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 2;
			break;

		case 2:
			--_kyragemFadingState.gOffset;
			if (_kyragemFadingState.gOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 3;
			break;
		
		case 3:
			++_kyragemFadingState.bOffset;
			if (_kyragemFadingState.bOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 4;
			break;
		
		case 4:
			--_kyragemFadingState.rOffset;
			if (_kyragemFadingState.rOffset >= 1)
				return;
			_kyragemFadingState.nextOperation = 5;
			break;
		
		case 5:
			++_kyragemFadingState.gOffset;
			if (_kyragemFadingState.gOffset < 19)
				return;
			_kyragemFadingState.nextOperation = 0;
			break;
			
		default:
			break;
	}
	
	_kyragemFadingState.timerCount = _system->getMillis() + 120 * _vm->tickLength();
}
} // end of namespace Kyra
