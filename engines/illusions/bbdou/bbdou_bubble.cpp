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
 */

#include "illusions/bbdou/illusions_bbdou.h"
#include "illusions/bbdou/bbdou_bubble.h"
#include "illusions/actor.h"
#include "illusions/camera.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"

namespace Illusions {

BbdouBubble::BbdouBubble(IllusionsEngine_BBDOU *vm, BbdouSpecialCode *bbdou)
	: _vm(vm), _bbdou(bbdou) {
}

BbdouBubble::~BbdouBubble() {
}

void BbdouBubble::init() {

	static const uint32 kObjectIds3[] = {
		0x0004003B, 0x0004003C, 0x0004003D, 0x0004003E,
		0x0004003F, 0x00040040, 0x00040041, 0x00040042,
		0x00040043, 0x00040044, 0x00040045, 0x00040046,
		0x00040047, 0x00040048, 0x00040049, 0x0004004A,
		0x0004004B, 0x0004004C, 0x0004004D, 0x0004004E,
		0x0004004F, 0x00040050, 0x00040051, 0x00040052,
		0x00040053, 0x00040054, 0x00040055, 0x00040056,
		0x00040057, 0x00040058, 0x00040059, 0x0004005A
	};

	static const uint32 kObjectIds2[] = {
		0x0004001B, 0x0004001C, 0x0004001D, 0x0004001E,
		0x0004001F, 0x00040020, 0x00040021, 0x00040022,
		0x00040023, 0x00040024, 0x00040025, 0x00040026,
		0x00040027, 0x00040028, 0x00040029, 0x0004002A,
		0x0004002B, 0x0004002C, 0x0004002D, 0x0004002E,
		0x0004002F, 0x00040030, 0x00040031, 0x00040032,
		0x00040033, 0x00040034, 0x00040035, 0x00040036,
		0x00040037, 0x00040038, 0x00040039, 0x0004003A
	};

	_objectId1414 = 0x4005B;
	_objectId1418 = 0x4005C;

	for (uint i = 0; i < 32; ++i)
		_objectIds[i] = kObjectIds3[i];

	for (uint i = 0; i < 32; ++i) {
		_items[i]._objectId = kObjectIds2[i];
		_items[i]._enabled = 0;
		_items[i]._position.x = 0;
		_items[i]._position.y = 0;
		_items[i]._sequenceId = 0;
	}

	_currItem0 = 0;
	_prevItem0 = 0;
	_someItem0 = 0;
	_pt1.x = 0;
	_pt1.y = 0;
	_pt2.x = 0;
	_pt2.y = 0;

}

void BbdouBubble::addItem0(uint32 sequenceId1, uint32 sequenceId2, uint32 progResKeywordId,
	uint32 namedPointId, int16 count, uint32 *namedPointIds) {
	Item0 item0;
	item0._sequenceId1 = sequenceId1;
	item0._sequenceId2 = sequenceId2;
	item0._progResKeywordId = progResKeywordId;
	item0._baseNamedPointId = namedPointId;
	item0._count = count;
	for (int16 i = 0; i < count; ++i)
		item0._namedPointIds[i] = FROM_LE_32(namedPointIds[i]);
	item0._objectId = 0;
	item0._pt.x = 0;
	item0._pt.y = 0;
	_item0s.push_back(item0);
}

void BbdouBubble::show() {

	if (_prevItem0) {
		hide();
	}

	_prevItem0 = _currItem0;
	_currItem0 = 0;

	calcBubbles(_pt1, _pt2);

	Control *control = _vm->_dict->getObjectControl(_prevItem0->_objectId);
	control->setActorPosition(_pt2);
	control->startSequenceActor(0x60057, 2, 0);
	control->startSequenceActor(_prevItem0->_sequenceId1, 2, 0);
	control->appearActor();
	control->deactivateObject();

	for (uint i = 0; i < 32; ++i) {
		if (_items[i]._enabled == 1) {
			Control *subControl = _vm->_dict->getObjectControl(_items[i]._objectId);
			subControl->setActorPosition(_items[i]._position);
			subControl->startSequenceActor(_items[i]._sequenceId, 2, 0);
		}
	}

}

void BbdouBubble::hide() {
	_someItem0 = _prevItem0;
	_prevItem0 = 0;
	if (_someItem0) {
		Control *control = _vm->_dict->getObjectControl(_someItem0->_objectId);
		control->startSequenceActor(_someItem0->_sequenceId2, 2, 0);
		for (uint i = 0; i < 32; ++i) {
			Control *subControl = _vm->_dict->getObjectControl(_objectIds[i]);
			subControl->stopActor();
			subControl->disappearActor();
		}
		for (uint i = 0; i < 32; ++i) {
			Control *subControl = _vm->_dict->getObjectControl(_items[i]._objectId);
			subControl->stopActor();
			subControl->disappearActor();
		}
	}
}

void BbdouBubble::setup(int16 minCount, Common::Point pt1, Common::Point pt2, uint32 progResKeywordId) {
	for (uint i = 0; i < 32; ++i)
		_items[i]._enabled = 0;
	int16 maxCount = 32;
	for (uint i = 0; i < _item0s.size(); ++i) {
		Item0 *item0 = &_item0s[i];
		if (item0->_count < maxCount && item0->_count >= minCount &&
			(!progResKeywordId || item0->_progResKeywordId == progResKeywordId)) {
			maxCount = item0->_count;
			_currItem0 = item0;
		}
	}
	_pt1 = pt1;
	_pt2 = pt2;
	_currItem0->_pt = pt2;
	_currItem0->_objectId = _objectId1414;
	if (_prevItem0 && _prevItem0->_objectId == _currItem0->_objectId)
		_currItem0->_objectId = _objectId1418;
}

uint32 BbdouBubble::addItem(uint positionIndex, uint32 sequenceId) {
	for (uint i = 0; i < 32; ++i) {
		Item141C *item = &_items[i];
		if (!item->_enabled) {
			Common::Point itemPos = _vm->getNamedPointPosition(_currItem0->_namedPointIds[positionIndex]);
			Common::Point basePos = _vm->getNamedPointPosition(_currItem0->_baseNamedPointId);
			item->_enabled = 1;
			item->_sequenceId = sequenceId;
			item->_position.x = itemPos.x + _currItem0->_pt.x - basePos.x;
			item->_position.y = itemPos.y + _currItem0->_pt.y - basePos.y;
			return item->_objectId;
		}
	}
	return 0;
}

void BbdouBubble::calcBubbles(Common::Point &pt1, Common::Point &pt2) {
	const int kSequenceIdsCount = 10;
	const float kDistanceBetweenPoints = 30.0;
	static const uint32 kSequenceIds[] = {
		0x00060042, 0x00060043, 0x00060044, 0x00060045, 0x00060046,
		0x00060047, 0x00060048, 0x00060049, 0x0006004A, 0x0006004B
	};
	static const int kIndexTbl[kSequenceIdsCount] = {4, 0, 8, 2, 6, 5, 1, 9, 3, 7};

	int sequenceCounters[kSequenceIdsCount];
	bool swapY;
	int centerX, centerY;
	float currentAngle, radius;

	for (int i = 0; i < 32; ++i) {
		Control *control = _vm->_dict->getObjectControl(_objectIds[i]);
		control->startSequenceActor(0x00060056, 2, 0);
	}

	for (int i = 0; i < kSequenceIdsCount; ++i)
		sequenceCounters[i] = 0;

	if (pt2.y >= pt1.y) {
		swapY = true;
		if (pt1.x == pt2.x)
			pt2.x = pt2.x + 20;
	} else {
		swapY = false;
		if (pt1.y == pt2.y)
			pt2.y = pt2.y + 20;
	}

	if (swapY) {
		centerX = (pt2.x * pt2.x - (pt2.y - pt1.y) * (pt2.y - pt1.y) - pt1.x * pt1.x) / (2 * (pt2.x - pt1.x));
		centerY = pt2.y;
		radius = ABS(pt2.x - centerX);
	} else {
		centerX = pt2.x;
		centerY = (pt2.y * pt2.y - (pt2.x - pt1.x) * (pt2.x - pt1.x) - pt1.y * pt1.y) / (2 * (pt2.y - pt1.y));
		radius = ABS(pt2.y - centerY);
	}

	const float fullDistance = sqrt((pt2.y - pt1.y) * (pt2.y - pt1.y) + (pt2.x - pt1.x) * (pt2.x - pt1.x));
	const float arcAngle = 2 * asin(CLIP(0.5 * fullDistance / radius, -1.0, 1.0));
	const float arcLength = arcAngle * radius;
	int pointsCount = (int)(arcLength / kDistanceBetweenPoints);
	float partAngle = ABS(kDistanceBetweenPoints / radius);

	for (int i = 0; i < pointsCount; ++i)
		++sequenceCounters[kIndexTbl[i % kSequenceIdsCount]];

	if (!swapY) {
		if (pt2.y < pt1.y) {
			currentAngle = M_PI * 0.5;
		} else {
			currentAngle = M_PI * 1.5;
			partAngle = -partAngle;
		}
		if (pt2.x < pt1.x)
			partAngle = -partAngle;
	} else {
		if (pt2.x <= pt1.x) {
			currentAngle = M_PI;
		} else {
			currentAngle = 0.0;
			partAngle = -partAngle;
		}
		if (pt2.y > pt1.y)
			partAngle = -partAngle;
	}

	int index = kSequenceIdsCount - 1;
	float angleStep = partAngle / (float)pointsCount * 0.5;
	float angleIncr = (float)(pointsCount / 2) * angleStep + partAngle;

	if (pointsCount > 32)
		pointsCount = 32;

	for (int i = 0; i < pointsCount; ++i) {

		currentAngle += angleIncr;
		angleIncr -= angleStep;

		Common::Point newPoint(
			centerX + _vm->getRandom(8) - 2 + (int)(cos(currentAngle) * radius),
			centerY + _vm->getRandom(8) - 2 - (int)(sin(currentAngle) * radius));

		Control *control = _vm->_dict->getObjectControl(_objectIds[i]);

		for (; index >= 0; --index) {
			if (sequenceCounters[index] > 0) {
				--sequenceCounters[index];
				control->setActorPosition(newPoint);
				control->startSequenceActor(kSequenceIds[index], 2, 0);
				control->appearActor();
				control->deactivateObject();
				break;
			}
		}

	}

}

} // End of namespace Illusions
