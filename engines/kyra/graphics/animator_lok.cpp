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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "kyra/engine/kyra_lok.h"
#include "kyra/graphics/screen.h"
#include "kyra/graphics/animator_lok.h"
#include "kyra/engine/sprites.h"

namespace Kyra {

Animator_LoK::Animator_LoK(KyraEngine_LoK *vm, OSystem *system) {
	_vm = vm;
	_screen = vm->screen();
	_initOk = false;
	_system = system;
	_screenObjects = _actors = _items = _sprites = _objectQueue = nullptr;
	_noDrawShapesFlag = 0;
	_brandonDrawFrame = 0;
	_brandonScaleX = _brandonScaleY = _brandonAnimSeqSizeWidth = _brandonAnimSeqSizeHeight = 0;

	_actorBkgBackUp[0] = new uint8[_screen->getRectSize(8, 69)]();
	_actorBkgBackUp[1] = new uint8[_screen->getRectSize(8, 69)]();
}

Animator_LoK::~Animator_LoK() {
	close();
	delete[] _actorBkgBackUp[0];
	delete[] _actorBkgBackUp[1];
}

void Animator_LoK::init(int actors_, int items_, int sprites_) {
	_screenObjects = new AnimObject[actors_ + items_ + sprites_]();
	assert(_screenObjects);
	_actors = _screenObjects;
	_sprites = &_screenObjects[actors_];
	_items = &_screenObjects[actors_ + items_];
	_brandonDrawFrame = 113;

	_initOk = true;
}

void Animator_LoK::close() {
	if (_initOk) {
		_initOk = false;
		delete[] _screenObjects;
		_screenObjects = _actors = _items = _sprites = _objectQueue = nullptr;
	}
}

void Animator_LoK::initAnimStateList() {
	AnimObject *animStates = _screenObjects;
	animStates[0].index = 0;
	animStates[0].active = 1;
	animStates[0].flags = 0x800;
	animStates[0].background = _actorBkgBackUp[0];
	animStates[0].rectSize = _screen->getRectSize(4, 48);
	animStates[0].width = 4;
	animStates[0].height = 48;
	animStates[0].width2 = 4;
	animStates[0].height2 = 3;

	for (int i = 1; i <= 4; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0x800;
		animStates[i].background = _actorBkgBackUp[1];
		animStates[i].rectSize = _screen->getRectSize(4, 64);
		animStates[i].width = 4;
		animStates[i].height = 48;
		animStates[i].width2 = 4;
		animStates[i].height2 = 3;
	}

	for (int i = 5; i < 16; ++i) {
		animStates[i].index = i;
		animStates[i].active = 0;
		animStates[i].flags = 0;
	}

	for (int i = 16; i < 28; ++i) {
		animStates[i].index = i;
		animStates[i].flags = 0;
		animStates[i].background = _vm->_shapes[345 + i];
		animStates[i].rectSize = _screen->getRectSize(3, 24);
		animStates[i].width = 3;
		animStates[i].height = 16;
		animStates[i].width2 = 0;
		animStates[i].height2 = 0;
	}
}

void Animator_LoK::preserveAllBackgrounds() {
	uint8 curPage = _screen->_curPage;
	_screen->_curPage = 2;

	AnimObject *curObject = _objectQueue;
	while (curObject) {
		if (curObject->active && !curObject->disable) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}
	_screen->_curPage = curPage;
}

void Animator_LoK::flagAllObjectsForBkgdChange() {
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->bkgdChangeFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void Animator_LoK::flagAllObjectsForRefresh() {
	AnimObject *curObject = _objectQueue;
	while (curObject) {
		curObject->refreshFlag = 1;
		curObject = curObject->nextAnimObject;
	}
}

void Animator_LoK::restoreAllObjectBackgrounds() {
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;

	while (curObject) {
		if (curObject->active && !curObject->disable) {
			preserveOrRestoreBackground(curObject, true);
			curObject->x2 = curObject->x1;
			curObject->y2 = curObject->y1;
		}
		curObject = curObject->nextAnimObject;
	}

	_screen->_curPage = 0;
}

void Animator_LoK::preserveAnyChangedBackgrounds() {
	AnimObject *curObject = _objectQueue;
	_screen->_curPage = 2;

	while (curObject) {
		if (curObject->active && !curObject->disable && curObject->bkgdChangeFlag) {
			preserveOrRestoreBackground(curObject, false);
			curObject->bkgdChangeFlag = 0;
		}
		curObject = curObject->nextAnimObject;
	}

	_screen->_curPage = 0;
}

void Animator_LoK::preserveOrRestoreBackground(AnimObject *obj, bool restore) {
	int x = 0, y = 0, width = obj->width, height = obj->height;

	if (restore) {
		x = obj->x2 >> 3;
		y = obj->y2;
	} else {
		x = obj->x1 >> 3;
		y = obj->y1;
	}

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	int temp;

	temp = x + width;
	if (temp >= 39)
		x = 39 - width;
	temp = y + height;
	if (temp >= 136)
		y = 136 - height;

	if (restore)
		_screen->copyBlockToPage(_screen->_curPage, x << 3, y, width << 3, height, obj->background);
	else
		_screen->copyRegionToBuffer(_screen->_curPage, x << 3, y, width << 3, height, obj->background);
}

void Animator_LoK::prepDrawAllObjects() {
	AnimObject *curObject = _objectQueue;
	int drawPage = 2;
	int invisibilityDrawFlag = 0, blurDrawFlag = 0, fadeDrawFlag = 0;
	if (_noDrawShapesFlag)
		return;
	if (_vm->_brandonStatusBit & 0x20)
		invisibilityDrawFlag = Screen::kDRAWSHP_PREDATOR;
	if (_vm->_brandonStatusBit & 0x40)
		blurDrawFlag = Screen::kDRAWSHP_MORPH;

	while (curObject) {
		if (curObject->active) {
			int xpos = curObject->x1;
			int ypos = curObject->y1;

			int drawLayer = 0;
			if (!(curObject->flags & Screen::kDRAWSHP_PRIORITY))
				drawLayer = 7;
			else if (curObject->disable)
				drawLayer = 0;
			else
				drawLayer = _vm->_sprites->getDrawLayer(curObject->drawY);

			// Talking head functionallity
			// DOS Floppy, Amiga and Mac work differently than DOS Talkie. PC-98 and FM-Towns are similar to DOS Talkie, but also slightly different.
			bool fmTownsOrPC98 = (_vm->gameFlags().platform == Common::kPlatformFMTowns || _vm->gameFlags().platform == Common::kPlatformPC98);
			if (_vm->_talkingCharNum != -1 && (fmTownsOrPC98 || (_vm->gameFlags().isTalkie ?
				(_vm->_currentCharacter->currentAnimFrame != 88 || curObject->index != 0) : (_brandonScaleX == 0x100 || !_vm->_scaleMode)))) {
				const int16 baseAnimFrameTable1[] = { 0x11, 0x35, 0x59, 0x00, 0x00 };
				const int16 baseAnimFrameTable2[] = { 0x15, 0x39, 0x5D, 0x00, 0x00 };
				const int8 xOffsetTable1[] = { 2, 4, 0, 5, 2, };
				const int8 xOffsetTable2[] = { 6, 4, 8, 3, 6, };
				const int8 yOffsetTable[] = { 0, 8, 1, 1, 0, };
				if (curObject->index <= 4) {
					int shapesIndex = 0;
					if (curObject->index == _vm->_talkHeadAnimCharNum)
						shapesIndex = _vm->_currHeadShape + baseAnimFrameTable1[curObject->index];
					else if (curObject->index != 2 || _vm->_characterList[2].sceneId == 77 || _vm->_characterList[2].sceneId == 86)
						shapesIndex = baseAnimFrameTable2[curObject->index];
					else
						shapesIndex = -1;

					xpos = curObject->x1;
					ypos = curObject->y1;

					int tempX = 0, tempY = 0;
					if (curObject->flags & Screen::kDRAWSHP_XFLIP) {
						tempX = xOffsetTable1[curObject->index];
						tempY = yOffsetTable[curObject->index];
					} else {
						tempX = xOffsetTable2[curObject->index];
						tempY = yOffsetTable[curObject->index];
					}

					if (_vm->gameFlags().isTalkie || fmTownsOrPC98) {
						tempX = (tempX * _brandonScaleX) >> 8;
						tempY = (tempY * _brandonScaleY) >> 8;
						if (_vm->_scaleMode && _brandonScaleX != 0x100)
							++tempX;
					}

					xpos += tempX;
					ypos += tempY;

					if (curObject->index == 0 && shapesIndex != -1) {
						if (!(_vm->_brandonStatusBit & 2)) {
							fadeDrawFlag = Screen::kDRAWSHP_FADE;
							if ((invisibilityDrawFlag & Screen::kDRAWSHP_PREDATOR) || (blurDrawFlag & Screen::kDRAWSHP_MORPH))
								fadeDrawFlag = 0;

							int tempFlags = 0;
							if (fadeDrawFlag & Screen::kDRAWSHP_FADE) {
								tempFlags = curObject->flags & Screen::kDRAWSHP_XFLIP;
								tempFlags |= (Screen::kDRAWSHP_PRIORITY | invisibilityDrawFlag | Screen::kDRAWSHP_FADE);
							}

							if (!(fadeDrawFlag & Screen::kDRAWSHP_FADE) && (blurDrawFlag & Screen::kDRAWSHP_MORPH)) {
								tempFlags = curObject->flags & Screen::kDRAWSHP_XFLIP;
								tempFlags |= (Screen::kDRAWSHP_PRIORITY | Screen::kDRAWSHP_FADE | invisibilityDrawFlag | Screen::kDRAWSHP_MORPH);
								_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | Screen::kDRAWSHP_SCALE, _vm->_brandonPoisonFlagsGFX, int(1), int(_vm->_brandonInvFlag), drawLayer, _brandonScaleX, _brandonScaleY);
							} else {
								if (!(blurDrawFlag & Screen::kDRAWSHP_MORPH)) {
									tempFlags = curObject->flags & Screen::kDRAWSHP_XFLIP;
									tempFlags |= (Screen::kDRAWSHP_PRIORITY | Screen::kDRAWSHP_FADE | invisibilityDrawFlag);
								}

								_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | Screen::kDRAWSHP_SCALE, _vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _brandonScaleX, _brandonScaleY);
							}
						}
					} else {
						if (shapesIndex != -1) {
							int tempFlags = 0;
							if (curObject->flags & Screen::kDRAWSHP_XFLIP)
								tempFlags = Screen::kDRAWSHP_XFLIP;
							_screen->drawShape(drawPage, _vm->_shapes[shapesIndex], xpos, ypos, 2, tempFlags | Screen::kDRAWSHP_PRIORITY, drawLayer);
						}
					}
				}
			}

			xpos = curObject->x1;
			ypos = curObject->y1;

			curObject->flags |= Screen::kDRAWSHP_PRIORITY;
			if (curObject->index == 0) {
				fadeDrawFlag = Screen::kDRAWSHP_FADE;

				if (invisibilityDrawFlag & Screen::kDRAWSHP_PREDATOR || blurDrawFlag & Screen::kDRAWSHP_MORPH)
					fadeDrawFlag = 0;

				if (_vm->_brandonStatusBit & 2)
					curObject->flags &= ~Screen::kDRAWSHP_XFLIP;

				if (!_vm->_scaleMode) {
					if (fadeDrawFlag & Screen::kDRAWSHP_FADE)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag | Screen::kDRAWSHP_FADE, (uint8 *)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer);
					else if (blurDrawFlag & Screen::kDRAWSHP_MORPH)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag | Screen::kDRAWSHP_MORPH, int(_vm->_brandonInvFlag), drawLayer);
					else
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag, drawLayer);
				} else {
					if (fadeDrawFlag & Screen::kDRAWSHP_FADE)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag | Screen::kDRAWSHP_FADE | Screen::kDRAWSHP_SCALE, (uint8 *)_vm->_brandonPoisonFlagsGFX, int(1), drawLayer, _brandonScaleX, _brandonScaleY);
					else if (blurDrawFlag & Screen::kDRAWSHP_MORPH)
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag | Screen::kDRAWSHP_MORPH | Screen::kDRAWSHP_SCALE, int(_vm->_brandonInvFlag), drawLayer, _brandonScaleX, _brandonScaleY);
					else
						_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | invisibilityDrawFlag | Screen::kDRAWSHP_SCALE, drawLayer, _brandonScaleX, _brandonScaleY);
				}
			} else {
				if (curObject->index >= 16 && curObject->index <= 27)
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags | Screen::kDRAWSHP_SCALE, drawLayer, (int)_vm->_scaleTable[curObject->drawY], (int)_vm->_scaleTable[curObject->drawY]);
				else
					_screen->drawShape(drawPage, curObject->sceneAnimPtr, xpos, ypos, 2, curObject->flags, drawLayer);
			}
		}
		curObject = curObject->nextAnimObject;
	}
}

void Animator_LoK::copyChangedObjectsForward(int refreshFlag, bool refreshScreen) {
	for (AnimObject *curObject = _objectQueue; curObject; curObject = curObject->nextAnimObject) {
		if (curObject->active) {
			if (curObject->refreshFlag || refreshFlag) {
				int xpos = 0, ypos = 0, width = 0, height = 0;
				xpos = (curObject->x1 >> 3) - (curObject->width2 >> 3) - 1;
				ypos = curObject->y1 - curObject->height2;
				width = curObject->width + (curObject->width2 >> 3) + 2;
				height = curObject->height + curObject->height2 * 2;

				if (xpos < 1)
					xpos = 1;
				else if (xpos > 39)
					continue;

				if (xpos + width > 39)
					width = 39 - xpos;

				if (ypos < 8)
					ypos = 8;
				else if (ypos > 136)
					continue;

				if (ypos + height > 136)
					height = 136 - ypos;

				_screen->copyRegion(xpos << 3, ypos, xpos << 3, ypos, width << 3, height, 2, 0);
				curObject->refreshFlag = 0;
			}
		}
	}

	if (refreshScreen)
		_screen->updateScreen();
}

void Animator_LoK::updateAllObjectShapes(bool refreshScreen) {
	restoreAllObjectBackgrounds();
	preserveAnyChangedBackgrounds();
	prepDrawAllObjects();
	copyChangedObjectsForward(0, refreshScreen);
}

void Animator_LoK::animRemoveGameItem(int index) {
	restoreAllObjectBackgrounds();

	AnimObject *animObj = &_items[index];
	animObj->sceneAnimPtr = nullptr;
	animObj->animFrameNumber = -1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	updateAllObjectShapes();
	animObj->active = 0;

	objectRemoveQueue(_objectQueue, animObj);
}

void Animator_LoK::animAddGameItem(int index, uint16 sceneId) {
	restoreAllObjectBackgrounds();
	assert(sceneId < _vm->_roomTableSize);
	Room *currentRoom = &_vm->_roomTable[sceneId];
	AnimObject *animObj = &_items[index];
	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = currentRoom->itemsYPos[index];
	animObj->sceneAnimPtr = _vm->_shapes[216 + currentRoom->itemsTable[index]];
	animObj->animFrameNumber = -1;
	animObj->x1 = currentRoom->itemsXPos[index];
	animObj->y1 = currentRoom->itemsYPos[index];
	animObj->x1 -= fetchAnimWidth(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]) >> 1;
	animObj->y1 -= fetchAnimHeight(animObj->sceneAnimPtr, _vm->_scaleTable[animObj->drawY]);
	animObj->x2 = animObj->x1;
	animObj->y2 = animObj->y1;
	animObj->width2 = 0;
	animObj->height2 = 0;
	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

void Animator_LoK::animAddNPC(int character) {
	restoreAllObjectBackgrounds();
	AnimObject *animObj = &_actors[character];
	const Character *ch = &_vm->_characterList[character];

	animObj->active = 1;
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _vm->_shapes[ch->currentAnimFrame];
	animObj->x1 = animObj->x2 = ch->x1 + _vm->_defaultShapeTable[ch->currentAnimFrame - 7].xOffset;
	animObj->y1 = animObj->y2 = ch->y1 + _vm->_defaultShapeTable[ch->currentAnimFrame - 7].yOffset;

	if (ch->facing >= 1 && ch->facing <= 3)
		animObj->flags |= 1;
	else if (ch->facing >= 5 && ch->facing <= 7)
		animObj->flags &= 0xFFFFFFFE;

	_objectQueue = objectQueue(_objectQueue, animObj);
	preserveAnyChangedBackgrounds();
	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
}

Animator_LoK::AnimObject *Animator_LoK::objectRemoveQueue(AnimObject *queue, AnimObject *rem) {
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
			return nullptr;
		return cur->nextAnimObject;
	}

	if (!cur->nextAnimObject) {
		if (cur == rem) {
			if (!prev)
				return nullptr;
			else
				prev->nextAnimObject = nullptr;
		}
	} else {
		if (cur == rem)
			prev->nextAnimObject = rem->nextAnimObject;
	}

	return queue;
}

Animator_LoK::AnimObject *Animator_LoK::objectAddHead(AnimObject *queue, AnimObject *head) {
	head->nextAnimObject = queue;
	return head;
}

Animator_LoK::AnimObject *Animator_LoK::objectQueue(AnimObject *queue, AnimObject *add) {
	if (!queue || add->drawY <= queue->drawY) {
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
		add->nextAnimObject = nullptr;
	}
	return queue;
}

void Animator_LoK::addObjectToQueue(AnimObject *object) {
	if (!_objectQueue)
		_objectQueue = objectAddHead(nullptr, object);
	else
		_objectQueue = objectQueue(_objectQueue, object);
}

void Animator_LoK::refreshObject(AnimObject *object) {
	_objectQueue = objectRemoveQueue(_objectQueue, object);
	if (_objectQueue)
		_objectQueue = objectQueue(_objectQueue, object);
	else
		_objectQueue = objectAddHead(nullptr, object);
}

void Animator_LoK::makeBrandonFaceMouse() {
	Common::Point mouse = _vm->getMousePos();
	if (mouse.x >= _vm->_currentCharacter->x1)
		_vm->_currentCharacter->facing = 3;
	else
		_vm->_currentCharacter->facing = 5;
	animRefreshNPC(0);
	updateAllObjectShapes();
}

int16 Animator_LoK::fetchAnimWidth(const uint8 *shape, int16 mult) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;
	return (((int16)READ_LE_UINT16((shape + 3))) * mult) >> 8;
}

int16 Animator_LoK::fetchAnimHeight(const uint8 *shape, int16 mult) {
	if (_vm->gameFlags().useAltShapeHeader)
		shape += 2;
	return (int16)(((int8)*(shape + 2)) * mult) >> 8;
}

void Animator_LoK::setBrandonAnimSeqSize(int width, int height) {
	restoreAllObjectBackgrounds();
	_brandonAnimSeqSizeWidth = _actors[0].width;
	_brandonAnimSeqSizeHeight = _actors[0].height;
	_actors[0].width = width + 1;
	_actors[0].height = height;
	preserveAllBackgrounds();
}

void Animator_LoK::resetBrandonAnimSeqSize() {
	restoreAllObjectBackgrounds();
	_actors[0].width = _brandonAnimSeqSizeWidth;
	_actors[0].height = _brandonAnimSeqSizeHeight;
	preserveAllBackgrounds();
}

void Animator_LoK::animRefreshNPC(int character) {
	AnimObject *animObj = &_actors[character];
	Character *ch = &_vm->characterList()[character];

	animObj->refreshFlag = 1;
	animObj->bkgdChangeFlag = 1;
	int facing = ch->facing;
	if (facing >= 1 && facing <= 3)
		animObj->flags |= 1;
	else if (facing >= 5 && facing <= 7)
		animObj->flags &= 0xFFFFFFFE;

	animObj->drawY = ch->y1;
	animObj->sceneAnimPtr = _vm->shapes()[ch->currentAnimFrame];
	animObj->animFrameNumber = ch->currentAnimFrame;
	if (character == 0) {
		if (_vm->brandonStatus() & 10) {
			animObj->animFrameNumber = 88;
			ch->currentAnimFrame = 88;
		}
		if (_vm->brandonStatus() & 2) {
			animObj->animFrameNumber = _brandonDrawFrame;
			ch->currentAnimFrame = _brandonDrawFrame;
			animObj->sceneAnimPtr = _vm->shapes()[_brandonDrawFrame];
			if (_vm->_brandonStatusBit0x02Flag) {
				++_brandonDrawFrame;
				// TODO: check this
				// UPDATE: From DOS floppy disasm: _brandonDrawFrame > 122 --> Test where this actually occurs
				if (_brandonDrawFrame >= 122) {
					_brandonDrawFrame = 113;
					_vm->_brandonStatusBit0x02Flag = 0;
				}
			}
		}
	}

	int xOffset = _vm->_defaultShapeTable[ch->currentAnimFrame - 7].xOffset;
	int yOffset = _vm->_defaultShapeTable[ch->currentAnimFrame - 7].yOffset;

	if (_vm->_scaleMode) {
		animObj->x1 = ch->x1;
		animObj->y1 = ch->y1;

		int newScale = _vm->_scaleTable[ch->y1];
		_brandonScaleX = newScale;
		_brandonScaleY = newScale;

		animObj->x1 += (_brandonScaleX * xOffset) >> 8;
		animObj->y1 += (_brandonScaleY * yOffset) >> 8;
	} else {
		animObj->x1 = ch->x1 + xOffset;
		animObj->y1 = ch->y1 + yOffset;
	}
	animObj->width2 = 4;
	animObj->height2 = 3;

	refreshObject(animObj);
}

void Animator_LoK::setCharacterDefaultFrame(int character) {
	static const uint16 initFrameTable[] = {
		7, 41, 77, 0, 0
	};
	assert(character < ARRAYSIZE(initFrameTable));
	Character *edit = &_vm->characterList()[character];
	edit->sceneId = 0xFFFF;
	edit->facing = 0;
	edit->currentAnimFrame = initFrameTable[character];
	// edit->unk6 = 1;
}

void Animator_LoK::setCharactersHeight() {
	static const int8 initHeightTable[] = {
		48, 40, 48, 47, 56,
		44, 42, 47, 38, 35,
		40
	};
	for (int i = 0; i < 11; ++i)
		_vm->characterList()[i].height = initHeightTable[i];
}

} // End of namespace Kyra
