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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "bbvs/bbvs.h"
#include "bbvs/gamemodule.h"

namespace Bbvs {

static const int8 kTurnInfo[8][8] = {
	{ 0,  1,  1,  1,  1, -1, -1, -1},
	{-1,  0,  1,  1,  1,  1, -1, -1},
	{-1, -1,  0,  1,  1,  1,  1, -1},
	{-1, -1, -1,  0,  1,  1,  1,  1},
	{ 1, -1, -1, -1,  0,  1,  1,  1},
	{ 1,  1, -1, -1, -1,  0,  1,  1},
	{ 1,  1,  1, -1, -1, -1,  0,  1},
	{ 1,  1,  1,  1, -1, -1, -1,  0}
};

static const int8 kWalkAnimTbl[32] = {
	 3,  0,  0,  0,  2,  1,  1,  1,
	15, 12, 14, 13,  0,  0,  0,  0,
	 7,  9,  4,  8,  6, 10,  5, 11,
	 3,  0,  2,  1, 15, 12, 14, 13
};

void BbvsEngine::startWalkObject(SceneObject *sceneObject) {
	if (_buttheadObject != sceneObject && _beavisObject != sceneObject)
		return;

	initWalkAreas(sceneObject);
	_sourceWalkAreaPt.x = sceneObject->x / 65536;
	_sourceWalkAreaPt.y = sceneObject->y / 65536;

	_sourceWalkArea = getWalkAreaAtPos(_sourceWalkAreaPt);
	if (!_sourceWalkArea)
		return;

	_destWalkAreaPt = sceneObject->walkDestPt;

	_destWalkArea = getWalkAreaAtPos(_destWalkAreaPt);
	if (!_destWalkArea)
		return;

	if (_sourceWalkArea != _destWalkArea) {
		_currWalkDistance = kMaxDistance;
		walkFindPath(_sourceWalkArea, 0);
		_destWalkAreaPt = _currWalkDistance == kMaxDistance ? _sourceWalkAreaPt : _finalWalkPt;
	}

	walkObject(sceneObject, _destWalkAreaPt, sceneObject->sceneObjectDef->walkSpeed);

}

void BbvsEngine::updateWalkObject(SceneObject *sceneObject) {
	int animIndex;

	if (sceneObject->walkCount > 0 && (sceneObject->xIncr != 0 || sceneObject->yIncr != 0)) {
		if (ABS(sceneObject->xIncr) <= ABS(sceneObject->yIncr))
			sceneObject->turnValue = sceneObject->yIncr >= 0 ? 0 : 4;
		else
			sceneObject->turnValue = sceneObject->xIncr >= 0 ? 6 : 2;
		animIndex = sceneObject->sceneObjectDef->animIndices[kWalkAnimTbl[sceneObject->turnValue]];
		sceneObject->turnCount = 0;
		sceneObject->turnTicks = 0;
	} else {
		animIndex = sceneObject->sceneObjectDef->animIndices[kWalkTurnTbl[sceneObject->turnValue]];
	}

	Animation *anim = 0;
	if (animIndex > 0)
		anim = _gameModule->getAnimation(animIndex);

	if (sceneObject->anim != anim) {
		if (anim) {
			sceneObject->anim = anim;
			sceneObject->animIndex = animIndex;
			sceneObject->frameTicks = 1;
			sceneObject->frameIndex = anim->frameCount - 1;
		} else {
			sceneObject->anim = 0;
			sceneObject->animIndex = 0;
			sceneObject->frameTicks = 0;
			sceneObject->frameIndex = 0;
		}
	}

}

void BbvsEngine::walkObject(SceneObject *sceneObject, const Common::Point &destPt, int walkSpeed) {
	int deltaX = destPt.x - (sceneObject->x / 65536);
	int deltaY = destPt.y - (sceneObject->y / 65536);
	float distance = (float)sqrt((double)(deltaX * deltaX + deltaY * deltaY));
	// NOTE The original doesn't have this check but without it the whole pathfinding breaks
	if (distance > 0.0f) {
		sceneObject->walkCount = (int)(distance / ((((float)ABS(deltaX) / distance) + 1.0f) * ((float)walkSpeed / 120)));
		sceneObject->xIncr = (int)(((float)deltaX / sceneObject->walkCount) * 65536.0f);
		sceneObject->yIncr = (int)(((float)deltaY / sceneObject->walkCount) * 65536.0f);
		sceneObject->x = (sceneObject->x & 0xFFFF0000) | 0x8000;
		sceneObject->y = (sceneObject->y & 0xFFFF0000) | 0x8000;
	} else
		sceneObject->walkCount = 0;
}

void BbvsEngine::turnObject(SceneObject *sceneObject) {
	if (sceneObject->turnTicks > 0) {
		--sceneObject->turnTicks;
	} else {
		int turnDir = kTurnInfo[sceneObject->turnValue][sceneObject->turnCount & 0x7F];
		if (turnDir) {
			sceneObject->turnValue = (sceneObject->turnValue + turnDir) & 7;
			int turnAnimIndex = sceneObject->sceneObjectDef->animIndices[kWalkTurnTbl[sceneObject->turnValue]];
			if (turnAnimIndex) {
				Animation *anim = _gameModule->getAnimation(turnAnimIndex);
				if (anim) {
					sceneObject->anim = anim;
					sceneObject->animIndex = turnAnimIndex;
					sceneObject->turnTicks = 4;
					sceneObject->frameTicks = 1;
					sceneObject->frameIndex = anim->frameCount - 1;
				}
			}
		} else {
			sceneObject->turnCount = 0;
		}
	}
}

int BbvsEngine::rectSubtract(const Common::Rect &rect1, const Common::Rect &rect2, Common::Rect *outRects) {
	int count = 0;
	Common::Rect workRect = rect1.findIntersectingRect(rect2);
	if (!workRect.isEmpty()) {
		count = 0;
		outRects[count] = Common::Rect(rect2.width(), workRect.top - rect2.top);
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, rect2.top);
			++count;
		}
		outRects[count] = Common::Rect(workRect.left - rect2.left, workRect.height());
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, workRect.top);
			++count;
		}
		outRects[count] = Common::Rect(rect2.right - workRect.right, workRect.height());
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(workRect.right, workRect.top);
			++count;
		}
		outRects[count] = Common::Rect(rect2.width(), rect2.bottom - workRect.bottom);
		if (!outRects[count].isEmpty()) {
			outRects[count].translate(rect2.left, workRect.bottom);
			++count;
		}
	} else {
		outRects[0] = rect2;
		count = 1;
	}
	return count;
}

WalkInfo *BbvsEngine::addWalkInfo(int16 x, int16 y, int delta, int direction, int16 midPtX, int16 midPtY, int walkAreaIndex) {
	WalkInfo *walkInfo = &_walkInfos[_walkInfosCount++];
	walkInfo->walkAreaIndex = walkAreaIndex;
	walkInfo->direction = direction;
	walkInfo->x = x;
	walkInfo->y = y;
	walkInfo->delta = delta;
	walkInfo->midPt.x = midPtX;
	walkInfo->midPt.y = midPtY;
	return walkInfo;
}

void BbvsEngine::initWalkAreas(SceneObject *sceneObject) {
	int16 objX = sceneObject->x / 65536;
	int16 objY = sceneObject->y / 65536;
	Common::Rect rect;
	bool doRect = false;
	Common::Rect *workWalkableRects;

	if (_buttheadObject == sceneObject && _beavisObject->anim) {
		rect = _beavisObject->anim->frameRects2[_beavisObject->frameIndex];
		rect.translate(_beavisObject->x / 65536, 1 + (_beavisObject->y / 65536));
		doRect = !rect.isEmpty();
	} else if (_buttheadObject->anim) {
		rect = _buttheadObject->anim->frameRects2[_buttheadObject->frameIndex];
		rect.translate(_buttheadObject->x / 65536, 1 + (_buttheadObject->y / 65536));
		doRect = !rect.isEmpty();
	}

	workWalkableRects = _walkableRects;

	_walkAreasCount = _walkableRectsCount;

	if (doRect && !rect.contains(objX, objY)) {
		_walkAreasCount = 0;
		for (int i = 0; i < _walkableRectsCount; ++i)
			_walkAreasCount += rectSubtract(rect, _walkableRects[i], &_tempWalkableRects1[_walkAreasCount]);
		workWalkableRects = _tempWalkableRects1;
	}

	for (int i = 0; i < _walkAreasCount; ++i) {
		_walkAreas[i].x = workWalkableRects[i].left;
		_walkAreas[i].y = workWalkableRects[i].top;
		_walkAreas[i].width = workWalkableRects[i].width();
		_walkAreas[i].height = workWalkableRects[i].height();
		_walkAreas[i].checked = false;
		_walkAreas[i].linksCount = 0;
	}

	_walkInfosCount = 0;

	// Find connections between the walkRects

	for (int i = 0; i < _walkAreasCount; ++i) {
		WalkArea *walkArea1 = &_walkAreas[i];
		int xIter = walkArea1->x + walkArea1->width;
		int yIter = walkArea1->y + walkArea1->height;

		for (int j = 0; j < _walkAreasCount; ++j) {
			WalkArea *walkArea2 = &_walkAreas[j];

			if (i == j)
				continue;

			if (walkArea2->y == yIter) {
				int wa1x = MAX(walkArea1->x, walkArea2->x);
				int wa2x = MIN(walkArea2->x + walkArea2->width, xIter);
				if (wa2x > wa1x) {
					debug(5, "WalkArea %d connected to %d by Y", i, j);
					WalkInfo *walkInfo1 = addWalkInfo(wa1x, yIter - 1, wa2x - wa1x, 0, wa1x + (wa2x - wa1x) / 2, yIter - 1, i);
					WalkInfo *walkInfo2 = addWalkInfo(wa1x, yIter, wa2x - wa1x, 0, wa1x + (wa2x - wa1x) / 2, yIter, j);
					walkArea1->linksD1[walkArea1->linksCount] = walkInfo1;
					walkArea1->linksD2[walkArea1->linksCount] = walkInfo2;
					walkArea1->links[walkArea1->linksCount++] = walkArea2;
					walkArea2->linksD1[walkArea2->linksCount] = walkInfo2;
					walkArea2->linksD2[walkArea2->linksCount] = walkInfo1;
					walkArea2->links[walkArea2->linksCount++] = walkArea1;
				}
			}

			if (walkArea2->x == xIter) {
				int wa1y = MAX(walkArea1->y, walkArea2->y);
				int wa2y = MIN(walkArea2->y + walkArea2->height, yIter);
				if (wa2y > wa1y) {
					debug(5, "WalkArea %d connected to %d by X", i, j);
					WalkInfo *walkInfo1 = addWalkInfo(xIter - 1, wa1y, wa2y - wa1y, 1, xIter - 1, wa1y + (wa2y - wa1y) / 2, i);
					WalkInfo *walkInfo2 = addWalkInfo(xIter, wa1y, wa2y - wa1y, 1, xIter, wa1y + (wa2y - wa1y) / 2, j);
					walkArea1->linksD1[walkArea1->linksCount] = walkInfo1;
					walkArea1->linksD2[walkArea1->linksCount] = walkInfo2;
					walkArea1->links[walkArea1->linksCount++] = walkArea2;
					walkArea2->linksD1[walkArea2->linksCount] = walkInfo2;
					walkArea2->linksD2[walkArea2->linksCount] = walkInfo1;
					walkArea2->links[walkArea2->linksCount++] = walkArea1;
				}
			}

		}

	}

}

WalkArea *BbvsEngine::getWalkAreaAtPos(const Common::Point &pt) {
	for (int i = 0; i < _walkAreasCount; ++i) {
		WalkArea *walkArea = &_walkAreas[i];
		if (walkArea->contains(pt))
			return walkArea;
	}
	return 0;
}

bool BbvsEngine::canButtheadWalkToDest(const Common::Point &destPt) {
	Common::Point srcPt;

	_walkReachedDestArea = false;
	initWalkAreas(_buttheadObject);
	srcPt.x = _buttheadObject->x / 65536;
	srcPt.y = _buttheadObject->y / 65536;
	_sourceWalkArea = getWalkAreaAtPos(srcPt);
	if (_sourceWalkArea) {
		_destWalkArea = getWalkAreaAtPos(destPt);
		if (_destWalkArea)
			canWalkToDest(_sourceWalkArea, 0);
	}
	return _walkReachedDestArea;
}

void BbvsEngine::canWalkToDest(WalkArea *walkArea, int infoCount) {

	if (_destWalkArea == walkArea) {
		_walkReachedDestArea = true;
		return;
	}

	if (_gameModule->getFieldC() <= 320 || infoCount <= 20) {
		walkArea->checked = true;
		for (int linkIndex = 0; linkIndex < walkArea->linksCount; ++linkIndex) {
			if (!walkArea->links[linkIndex]->checked) {
				canWalkToDest(walkArea->links[linkIndex], infoCount + 2);
				if (_walkReachedDestArea)
					break;
			}
		}
		walkArea->checked = false;
	}

}

bool BbvsEngine::walkTestLineWalkable(const Common::Point &sourcePt, const Common::Point &destPt, WalkInfo *walkInfo) {
	const float ptDeltaX = MAX<float>(destPt.x - sourcePt.x, 1.0f);
	const float ptDeltaY = destPt.y - sourcePt.y;
	const float wDeltaX = walkInfo->x - sourcePt.x;
	const float wDeltaY = walkInfo->y - sourcePt.y;
	if (walkInfo->direction) {
		const float nDeltaY = wDeltaX * ptDeltaY / ptDeltaX + (float)sourcePt.y - (float)walkInfo->y;
		return (nDeltaY >= 0.0f) && (nDeltaY < (float)walkInfo->delta);
	} else {
		const float nDeltaX = wDeltaY / ptDeltaX * ptDeltaY + (float)sourcePt.x - (float)walkInfo->x;
		return (nDeltaX >= 0.0f) && (nDeltaX < (float)walkInfo->delta);
	}
	return false;
}

void BbvsEngine::walkFindPath(WalkArea *sourceWalkArea, int infoCount) {
	if (_destWalkArea == sourceWalkArea) {
		walkFoundPath(infoCount);
	} else if (_gameModule->getFieldC() <= 320 || infoCount <= 20) {
		sourceWalkArea->checked = true;
		for (int linkIndex = 0; linkIndex < sourceWalkArea->linksCount; ++linkIndex) {
			if (!sourceWalkArea->links[linkIndex]->checked) {
				_walkInfoPtrs[infoCount + 0] = sourceWalkArea->linksD1[linkIndex];
				_walkInfoPtrs[infoCount + 1] = sourceWalkArea->linksD2[linkIndex];
				walkFindPath(sourceWalkArea->links[linkIndex], infoCount + 2);
			}
		}
		sourceWalkArea->checked = false;
	}
}

int BbvsEngine::calcDistance(const Common::Point &pt1, const Common::Point &pt2) {
	return (int)sqrt((double)(pt1.x - pt2.x) * (pt1.x - pt2.x) + (pt1.y - pt2.y) * (pt1.y - pt2.y));
}

void BbvsEngine::walkFoundPath(int count) {
	debug(5, "BbvsEngine::walkFoundPath(%d)", count);

	Common::Point midPt = _sourceWalkAreaPt;
	int totalMidPtDistance = 0;

	if (count > 0) {
		Common::Point lastMidPt;
		int halfCount = (count + 1) >> 1;
		for (int i = 0; i < halfCount; ++i) {
			lastMidPt = midPt;
			midPt = _walkInfoPtrs[i * 2]->midPt;
			totalMidPtDistance += calcDistance(midPt, lastMidPt);
		}
	}

	int distance = calcDistance(midPt, _destWalkAreaPt) + totalMidPtDistance;

	debug(5, "BbvsEngine::walkFoundPath() distance: %d; _currWalkDistance: %d", distance, _currWalkDistance);

	if (distance >= _currWalkDistance)
		return;

	debug(5, "BbvsEngine::walkFoundPath() distance smaller");

	_currWalkDistance = distance;

	Common::Point destPt = _destWalkAreaPt, newDestPt;

	while (1) {

		int index = 0;
		if (count > 0) {
			do {
				if (!walkTestLineWalkable(_sourceWalkAreaPt, destPt, _walkInfoPtrs[index]))
					break;
				++index;
			} while (index < count);
		}

		if (index == count)
			break;

		WalkInfo *walkInfo = _walkInfoPtrs[--count];
		destPt.x = walkInfo->x;
		destPt.y = walkInfo->y;

		if (walkInfo->direction) {
			newDestPt.x = walkInfo->x;
			newDestPt.y = walkInfo->y + walkInfo->delta - 1;
		} else {
			newDestPt.x = walkInfo->x + walkInfo->delta - 1;
			newDestPt.y = walkInfo->y;
		}

		if ((newDestPt.x - _destWalkAreaPt.x) * (newDestPt.x - _destWalkAreaPt.x) +
			(newDestPt.y - _destWalkAreaPt.y) * (newDestPt.y - _destWalkAreaPt.y) <
			(destPt.x - _destWalkAreaPt.x) * (destPt.x - _destWalkAreaPt.x) +
			(destPt.y - _destWalkAreaPt.y) * (destPt.y - _destWalkAreaPt.y))
			destPt = newDestPt;

	}

	debug(5, "BbvsEngine::walkFoundPath() destPt: (%d, %d)", destPt.x, destPt.y);

	_finalWalkPt = destPt;

	debug(5, "BbvsEngine::walkFoundPath() OK");

}

void BbvsEngine::updateWalkableRects() {
	// Go through all walkable rects and subtract all scene object rects
	Common::Rect *rectsList1 = _tempWalkableRects1;
	Common::Rect *rectsList2 = _gameModule->getWalkRects();
	_walkableRectsCount = _gameModule->getWalkRectsCount();
	for (int i = 0; i < _gameModule->getSceneObjectDefsCount(); ++i) {
		SceneObject *sceneObject = &_sceneObjects[i];
		Animation *anim = sceneObject->anim;
		if (anim && _buttheadObject != sceneObject && _beavisObject != sceneObject) {
			Common::Rect rect = sceneObject->anim->frameRects2[sceneObject->frameIndex];
			rect.translate(sceneObject->x / 65536, sceneObject->y / 65536);
			int count = _walkableRectsCount;
			_walkableRectsCount = 0;
			for (int j = 0; j < count; ++j)
				_walkableRectsCount += rectSubtract(rect, rectsList2[j], &rectsList1[_walkableRectsCount]);
			if (rectsList1 == _tempWalkableRects1) {
				rectsList1 = _tempWalkableRects2;
				rectsList2 = _tempWalkableRects1;
			} else {
				rectsList1 = _tempWalkableRects1;
				rectsList2 = _tempWalkableRects2;
			}
		}
	}
	for (int i = 0; i < _walkableRectsCount; ++i)
		_walkableRects[i] = rectsList2[i];
}

} // End of namespace Bbvs
