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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "kyra/kyra_v2.h"
#include "kyra/wsamovie.h"

namespace Kyra {

void KyraEngine_v2::clearAnimObjects() {
	memset(_animObjects, 0, sizeof(_animObjects));
	
	_animObjects[0].index = 0;
	_animObjects[0].type = 0;
	_animObjects[0].enabled = 1;
	_animObjects[0].flags = 0x800;
	_animObjects[0].width = 32;
	_animObjects[0].height = 49;
	_animObjects[0].width2 = 4;
	_animObjects[0].height2 = 10;
	
	for (int i = 1; i < 11; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 2;
	}
	
	for (int i = 11; i <= 40; ++i) {
		_animObjects[i].index = i;
		_animObjects[i].type = 1;
		_animObjects[i].flags = 0x800;
		_animObjects[i].width = 16;
		_animObjects[i].height = 16;
	}
}

KyraEngine_v2::AnimObj *KyraEngine_v2::initAnimList(AnimObj *list, AnimObj *entry) {
	entry->nextObject = list;
	return entry;
}

KyraEngine_v2::AnimObj *KyraEngine_v2::addToAnimListSorted(AnimObj *list, AnimObj *add) {
	if (add->yPos1 <= list->yPos1) {
		add->nextObject = list;
		return add;
	}

	AnimObj *cur = list;
	AnimObj *prev = list;
	while (add->yPos1 > cur->yPos1) {
		AnimObj *temp = cur->nextObject;
		if (!temp)
			break;
		prev = cur;
		cur = temp;
	}
	
	if (add->yPos1 <= cur->yPos1) {
		prev->nextObject = add;
		add->nextObject = cur;
	} else {
		cur->nextObject = add;
		add->nextObject = 0;
	}
	return list;
}

KyraEngine_v2::AnimObj *KyraEngine_v2::deleteAnimListEntry(AnimObj *list, AnimObj *entry) {
	if (!list)
		return 0;

	AnimObj *old = 0;
	AnimObj *cur = list;

	while (true) {
		if (cur == entry)
			break;
		if (!cur->nextObject)
			break;
		old = cur;
		cur = cur->nextObject;
	}

	if (cur == list) {
		if (!cur->nextObject)
			return 0;
		cur = cur->nextObject;
		return cur;
	}

	if (!cur->nextObject) {
		if (!old)
			return 0;
		old->nextObject = 0;
		return list;
	}

	if (cur != entry)
		return list;

	old->nextObject = entry->nextObject;
	return list;
}

void KyraEngine_v2::drawAnimObjects() {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;
		
		int x = curObject->xPos2 - (_screen->getScreenDim(2)->sx << 3);
		int y = curObject->yPos2 - _screen->getScreenDim(2)->sy;
		int layer = 7;
		
		if (curObject->flags & 0x800) {
			if (curObject->animFlags)
				layer = 0;
			else
				layer = getDrawLayer(curObject->xPos1, curObject->yPos1);
		}
		curObject->flags |= 0x800;

		if (curObject->index)
			drawSceneAnimObject(curObject, x, y, layer);
		else
			drawCharacterAnimObject(curObject, x, y, layer);
	}
}

void KyraEngine_v2::refreshAnimObjects(int force) {
	for (AnimObj *curObject = _animList; curObject; curObject = curObject->nextObject) {
		if (!curObject->enabled)
			continue;
		if (!curObject->needRefresh && !force)
			continue;

		int x = curObject->xPos2 - curObject->width2;
		if (x < 0)
			x = 0;
		if (x >= 320)
			x = 319;
		int y = curObject->yPos2 - curObject->height2;
		if (y < 0)
			y = 0;
		if (y >= 143)
			y = 142;

		int width = curObject->width + curObject->width2 + 8;
		int height = curObject->height + curObject->height2*2;
		if (width + x > 320)
			width -= width + x - 322;
		if (height + y > 143)
			height -= height + y - 144;
	
		_screen->hideMouse();
		_screen->copyRegion(x, y, x, y, width, height, 2, 0, Screen::CR_CLIPPED);
		_screen->showMouse();
		
		curObject->needRefresh = false;
	}
}

void KyraEngine_v2::refreshAnimObjectsIfNeed() {
	for (AnimObj *curEntry = _animList; curEntry; curEntry = curEntry->nextObject) {
		if (curEntry->enabled && curEntry->needRefresh) {
			restorePage3();
			drawAnimObjects();
			refreshAnimObjects(0);
			_screen->updateScreen();
			return;
		}
	}
}

void KyraEngine_v2::updateCharacterAnim(int) {
	Character *c = &_mainCharacter;
	AnimObj *animState = _animObjects;

	animState->needRefresh = 1;
	animState->unk8 = 1;

	if (c->facing >= 1 && c->facing <= 3)
		animState->flags |= 1;
	else if (c->facing >= 5 && c->facing <= 7)
		animState->flags &= ~1;

	animState->xPos2 = animState->xPos1 = c->x1;
	animState->yPos2 = animState->yPos1 = c->y1;
	animState->shapePtr = _defaultShapeTable[c->animFrame];
	animState->shapeIndex1 = animState->shapeIndex2 = c->animFrame;

	int xAdd = _shapeDescTable[c->animFrame-9].xAdd;
	int yAdd = _shapeDescTable[c->animFrame-9].yAdd;

	_charScaleX = _charScaleY = getScale(c->x1, c->y1);

	animState->xPos2 += (xAdd * _charScaleX) >> 8;
	animState->yPos2 += (yAdd * _charScaleY) >> 8;
	animState->width2 = 8;
	animState->height2 = 10;

	_animList = deleteAnimListEntry(_animList, animState);
	if (_animList)
		_animList = addToAnimListSorted(_animList, animState);
	else
		_animList = initAnimList(_animList, animState);

	updateCharPal(1);
}

void KyraEngine_v2::updateSceneAnim(int anim, int newFrame) {
	AnimObj *animObject = &_animObjects[1+anim];
	if (!animObject->enabled)
		return;
	
	animObject->needRefresh = 1;
	animObject->unk8 = 1;
	animObject->flags = 0;
	
	if (_sceneAnims[anim].flags & 2)
		animObject->flags |= 0x800;
	else
		animObject->flags &= ~0x800;
	
	if (_sceneAnims[anim].flags & 4)
		animObject->flags |= 1;
	else
		animObject->flags &= ~1;
	
	if (_sceneAnims[anim].flags & 0x20) {
		animObject->shapePtr = _sceneShapeTable[newFrame];
		animObject->shapeIndex2 = 0xFFFF;
		animObject->shapeIndex3 = 0xFFFF;
		animObject->animNum = 0xFFFF;		
	} else {
		animObject->shapePtr = 0;
		animObject->shapeIndex3 = newFrame;
		animObject->animNum = anim;
	}
	
	animObject->xPos1 = _sceneAnims[anim].x;
	animObject->yPos1 = _sceneAnims[anim].y;
	animObject->xPos2 = _sceneAnims[anim].x2;
	animObject->yPos2 = _sceneAnims[anim].y2;
	
	if (_sceneAnims[anim].flags & 2) {
		_animList = deleteAnimListEntry(_animList, animObject);
		if (!_animList)
			_animList = initAnimList(_animList, animObject);
		else
			_animList = addToAnimListSorted(_animList, animObject);
	}
}

void KyraEngine_v2::drawSceneAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (obj->type == 1) {
		if (obj->shapeIndex1 == 0xFFFF)
			return;
		int scale = getScale(obj->xPos1, obj->yPos1);
		_screen->drawShape(2, getShapePtr(obj->shapeIndex1), x, y, 2, obj->flags | 4, layer, scale, scale);
		return;
	}

	if (obj->shapePtr) {
		_screen->drawShape(2, obj->shapePtr, x, y, 2, obj->flags, layer);
	} else {
		if (obj->shapeIndex3 == 0xFFFF || obj->animNum == 0xFFFF)
			return;

		int flags = 0x4000;
		if (obj->flags & 0x800)
			flags |= 0x8000;

		if (_sceneAnims[obj->animNum].wsaFlag) {
			x = y = 0;
		} else {
			x = obj->xPos2;
			y = obj->yPos2;
		}

		_sceneAnimMovie[obj->animNum]->setX(x);
		_sceneAnimMovie[obj->animNum]->setY(y);
		_sceneAnimMovie[obj->animNum]->setDrawPage(2);
		_sceneAnimMovie[obj->animNum]->displayFrame(obj->shapeIndex3, int(flags | layer), 0, 0);
	}
}

void KyraEngine_v2::drawCharacterAnimObject(AnimObj *obj, int x, int y, int layer) {
	if (_drawNoShapeFlag || obj->shapeIndex1 == 0xFFFF)
		return;
	_screen->drawShape(2, getShapePtr(obj->shapeIndex1), x, y, 2, obj->flags | 4, layer, _charScaleX, _charScaleY);
}

} // end of namespace Kyra
