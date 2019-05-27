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

	static const uint32 kTrailObjectIds[] = {
		0x0004003B, 0x0004003C, 0x0004003D, 0x0004003E,
		0x0004003F, 0x00040040, 0x00040041, 0x00040042,
		0x00040043, 0x00040044, 0x00040045, 0x00040046,
		0x00040047, 0x00040048, 0x00040049, 0x0004004A,
		0x0004004B, 0x0004004C, 0x0004004D, 0x0004004E,
		0x0004004F, 0x00040050, 0x00040051, 0x00040052,
		0x00040053, 0x00040054, 0x00040055, 0x00040056,
		0x00040057, 0x00040058, 0x00040059, 0x0004005A
	};

	static const uint32 kIconObjectIds[] = {
		0x0004001B, 0x0004001C, 0x0004001D, 0x0004001E,
		0x0004001F, 0x00040020, 0x00040021, 0x00040022,
		0x00040023, 0x00040024, 0x00040025, 0x00040026,
		0x00040027, 0x00040028, 0x00040029, 0x0004002A,
		0x0004002B, 0x0004002C, 0x0004002D, 0x0004002E,
		0x0004002F, 0x00040030, 0x00040031, 0x00040032,
		0x00040033, 0x00040034, 0x00040035, 0x00040036,
		0x00040037, 0x00040038, 0x00040039, 0x0004003A
	};

	_bubbleObjectId1 = 0x4005B;
	_bubbleObjectId2 = 0x4005C;

	for (uint i = 0; i < 32; ++i) {
		_trailObjectIds[i] = kTrailObjectIds[i];
	}

	for (uint i = 0; i < 32; ++i) {
		_icons[i]._objectId = kIconObjectIds[i];
		_icons[i]._enabled = false;
		_icons[i]._position.x = 0;
		_icons[i]._position.y = 0;
		_icons[i]._sequenceId = 0;
	}

	_currBubbleStyle = 0;
	_showingBubbleStyle = 0;
	_hidingBubbleStyle = 0;
	_sourcePt.x = 0;
	_sourcePt.y = 0;
	_destPt.x = 0;
	_destPt.y = 0;

}

void BbdouBubble::addBubbleStyle(uint32 showSequenceId, uint32 hideSequenceId, uint32 progResKeywordId,
	uint32 namedPointId, int16 count, uint32 *namedPointIds) {
	BubbleStyle style;
	style._showSequenceId = showSequenceId;
	style._hideSequenceId = hideSequenceId;
	style._progResKeywordId = progResKeywordId;
	style._baseNamedPointId = namedPointId;
	style._count = count;
	for (int16 i = 0; i < count; ++i) {
		style._namedPointIds[i] = FROM_LE_32(namedPointIds[i]);
	}
	style._objectId = 0;
	style._position.x = 0;
	style._position.y = 0;
	_bubbleStyles.push_back(style);
}

void BbdouBubble::show() {
	
	if (_showingBubbleStyle) {
		hide();
	}

	_showingBubbleStyle = _currBubbleStyle;
	_currBubbleStyle = 0;

	calcBubbleTrail(_sourcePt, _destPt);

	Control *bubbleControl = _vm->_dict->getObjectControl(_showingBubbleStyle->_objectId);
	bubbleControl->setActorPosition(_destPt);
	bubbleControl->startSequenceActor(0x60057, 2, 0);
	bubbleControl->startSequenceActor(_showingBubbleStyle->_showSequenceId, 2, 0);
	bubbleControl->appearActor();
	bubbleControl->deactivateObject();

	for (uint i = 0; i < 32; ++i) {
		if (_icons[i]._enabled) {
			Control *iconControl = _vm->_dict->getObjectControl(_icons[i]._objectId);
			iconControl->setActorPosition(_icons[i]._position);
			iconControl->startSequenceActor(_icons[i]._sequenceId, 2, 0);
		}
	}

}

void BbdouBubble::hide() {
	_hidingBubbleStyle = _showingBubbleStyle;
	_showingBubbleStyle = 0;
	if (_hidingBubbleStyle) {
		Control *bubbleControl = _vm->_dict->getObjectControl(_hidingBubbleStyle->_objectId);
		bubbleControl->startSequenceActor(_hidingBubbleStyle->_hideSequenceId, 2, 0);
		for (uint i = 0; i < 32; ++i) {
			Control *trailControl = _vm->_dict->getObjectControl(_trailObjectIds[i]);
			trailControl->stopActor();
			trailControl->disappearActor();
		}
		for (uint i = 0; i < 32; ++i) {
			Control *iconControl = _vm->_dict->getObjectControl(_icons[i]._objectId);
			iconControl->stopActor();
			iconControl->disappearActor();
		}
	}
}

void BbdouBubble::selectBubbleStyle(int16 minCount, Common::Point sourcePt, Common::Point destPt, uint32 progResKeywordId) {
	for (uint i = 0; i < 32; ++i) {
		_icons[i]._enabled = false;
	}
	int16 maxCount = 32;
	for (uint i = 0; i < _bubbleStyles.size(); ++i) {
		BubbleStyle *style = &_bubbleStyles[i];
		if (style->_count < maxCount && style->_count >= minCount &&
			(progResKeywordId == 0 || progResKeywordId == style->_progResKeywordId)) {
			maxCount = style->_count;
			_currBubbleStyle = style;
		}
	}
	_sourcePt = sourcePt;
	_destPt = destPt;
	_currBubbleStyle->_position = destPt;
	_currBubbleStyle->_objectId = _bubbleObjectId1;
	if (_showingBubbleStyle && _showingBubbleStyle->_objectId == _currBubbleStyle->_objectId)
		_currBubbleStyle->_objectId = _bubbleObjectId2;
}

uint32 BbdouBubble::addBubbleIcon(uint positionIndex, uint32 sequenceId) {
	for (uint i = 0; i < 32; ++i) {
		BubbleIcon *icon = &_icons[i];
		if (!icon->_enabled) {
			Common::Point itemPos = _vm->getNamedPointPosition(_currBubbleStyle->_namedPointIds[positionIndex]);
			Common::Point basePos = _vm->getNamedPointPosition(_currBubbleStyle->_baseNamedPointId);
			icon->_enabled = true;
			icon->_sequenceId = sequenceId;
			icon->_position.x = itemPos.x + _currBubbleStyle->_position.x - basePos.x;
			icon->_position.y = itemPos.y + _currBubbleStyle->_position.y - basePos.y;
			return icon->_objectId;
		}
	}
	return 0;
}

void BbdouBubble::calcBubbleTrail(Common::Point &sourcePt, Common::Point &destPt) {
	const int kSequenceIdsCount = 10;
	const float kDistanceBetweenPoints = 30.0;
	static const uint32 kBubbleTrailSequenceIds[] = {
		0x00060042, 0x00060043, 0x00060044, 0x00060045, 0x00060046,
		0x00060047, 0x00060048, 0x00060049, 0x0006004A, 0x0006004B
	};
	static const int kIndexTbl[kSequenceIdsCount] = {4, 0, 8, 2, 6, 5, 1, 9, 3, 7};

	int sequenceCounters[kSequenceIdsCount];
	bool swapY;
	int centerX, centerY;
	float currentAngle, radius;

	for (int i = 0; i < 32; ++i) {
		Control *control = _vm->_dict->getObjectControl(_trailObjectIds[i]);
		control->startSequenceActor(0x00060056, 2, 0);
	}

	for (int i = 0; i < kSequenceIdsCount; ++i) {
		sequenceCounters[i] = 0;
	}

	if (destPt.y >= sourcePt.y) {
		swapY = true;
		if (sourcePt.x == destPt.x)
			destPt.x = destPt.x + 20;
	} else {
		swapY = false;
		if (sourcePt.y == destPt.y)
			destPt.y = destPt.y + 20;
	}

	if (swapY) {
		centerX = (destPt.x * destPt.x - (destPt.y - sourcePt.y) * (destPt.y - sourcePt.y) - sourcePt.x * sourcePt.x) / (2 * (destPt.x - sourcePt.x));
		centerY = destPt.y;
		radius = ABS(destPt.x - centerX);
	} else {
		centerX = destPt.x;
		centerY = (destPt.y * destPt.y - (destPt.x - sourcePt.x) * (destPt.x - sourcePt.x) - sourcePt.y * sourcePt.y) / (2 * (destPt.y - sourcePt.y));
		radius = ABS(destPt.y - centerY);
	}

	const float fullDistance = sqrt((destPt.y - sourcePt.y) * (destPt.y - sourcePt.y) + (destPt.x - sourcePt.x) * (destPt.x - sourcePt.x));
	const float arcAngle = 2 * asin(CLIP(0.5 * fullDistance / radius, -1.0, 1.0));
	const float arcLength = arcAngle * radius;
	int pointsCount = (int)(arcLength / kDistanceBetweenPoints);
	float partAngle = ABS(kDistanceBetweenPoints / radius);

	for (int i = 0; i < pointsCount; ++i) {
		++sequenceCounters[kIndexTbl[i % kSequenceIdsCount]];
	}

	if (!swapY) {
		if (destPt.y < sourcePt.y) {
			currentAngle = (float)M_PI * 0.5F;
		} else {
			currentAngle = (float)M_PI * 1.5F;
			partAngle = -partAngle;
		}
		if (destPt.x < sourcePt.x)
			partAngle = -partAngle;
	} else {
		if (destPt.x <= sourcePt.x) {
			currentAngle = (float)M_PI;
		} else {
			currentAngle = 0.0;
			partAngle = -partAngle;
		}
		if (destPt.y > sourcePt.y)
			partAngle = -partAngle;
	}

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

		Control *trailControl = _vm->_dict->getObjectControl(_trailObjectIds[i]);

		for (int index = kSequenceIdsCount - 1; index >= 0; --index) {
			if (sequenceCounters[index] > 0) {
				--sequenceCounters[index];
				trailControl->setActorPosition(newPoint);
				trailControl->startSequenceActor(kBubbleTrailSequenceIds[index], 2, 0);
				trailControl->appearActor();
				trailControl->deactivateObject();
				break;
			}
		}

	}

}

} // End of namespace Illusions
