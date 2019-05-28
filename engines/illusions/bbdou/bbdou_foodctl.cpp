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
#include "illusions/bbdou/bbdou_foodctl.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/textdrawer.h"
#include "illusions/time.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

BbdouFoodCtl::BbdouFoodCtl(IllusionsEngine_BBDOU *vm)
	: _vm(vm) {
}

BbdouFoodCtl::~BbdouFoodCtl() {
}

void BbdouFoodCtl::placeFood(uint totalRoundsCount, uint maxRequestedFoodCount) {
	_totalRoundsCount = totalRoundsCount;
	_maxRequestedFoodCount = maxRequestedFoodCount;
	_requestedFoodCount = 0;
	_requestedFoodIndex = 0;
	placeActors();
}

void BbdouFoodCtl::addFood(uint32 propertyId) {
	_foodPropertyIds[_requestedFoodCount++] = propertyId;
}

void BbdouFoodCtl::requestFirstFood() {
	_requestedFoodIndex = 1;
	_vm->_scriptResource->_properties.set(_foodPropertyIds[0], true);
}

void BbdouFoodCtl::requestNextFood() {
	uint32 propertyId = _foodPropertyIds[_requestedFoodIndex++];
	_vm->_scriptResource->_properties.set(propertyId, true);
}

void BbdouFoodCtl::nextRound() {
	--_totalRoundsCount;
}

bool BbdouFoodCtl::hasReachedRequestedFoodCount() {
	return _requestedFoodIndex > _requestedFoodCount;
}

bool BbdouFoodCtl::hasRoundFinished() {
	return _totalRoundsCount == 0 || _requestedFoodCount > _maxRequestedFoodCount;
}

void BbdouFoodCtl::serveFood() {
	uint32 foodSequenceId = getFoodSequenceId();
	uint32 studentObjectId = getCurrentStudentObjectId();
	uint32 foodObjectId = _foodItems[_servedFoodCount++].objectId;
	Control *foodControl = _vm->_dict->getObjectControl(foodObjectId);
	foodControl->startSequenceActor(foodSequenceId, 2, 0);
	foodControl->linkToObject(studentObjectId, _servedFoodCount);
}

void BbdouFoodCtl::resetFood() {
	for (uint i = 0; i < _servedFoodCount; ++i) {
		Control *control = _vm->_dict->getObjectControl(_foodItems[i].objectId);
		control->unlinkObject();
		_foodItems[i].value = 0;
	}
	_servedFoodCount = 0;
	resetFoodControls();
}

void BbdouFoodCtl::placeActors() {
	static const uint32 kFoodSequenceIds[] = {
		0x00060932, 0x00060933, 0x00060934,
		0x00060935, 0x00060936, 0x00060937
	};
	for (uint i = 0; i < kFoodCount; ++i) {
		uint32 objectId = _vm->_controls->newTempObjectId();
		_vm->_controls->placeActor(0x00050119, Common::Point(0, 0), 0x00060931, objectId, 0);
		Control *control = _vm->_dict->getObjectControl(objectId);
		control->deactivateObject();
		control->setPriority(i + 10);
		control->startSequenceActor(kFoodSequenceIds[(i + 1) % 6], 2, 0);
		_foodItems[i].objectId = objectId;
		_foodItems[i].value = 0;
	}
	_servedFoodCount = 0;
	resetFoodControls();
}

void BbdouFoodCtl::resetFoodControls() {
	Common::Point pos(-100, 32);
	for (uint i = 0; i < kFoodCount; ++i) {
		Control *control = _vm->_dict->getObjectControl(_foodItems[i].objectId);
		control->setActorPosition(pos);
		pos.y += 20;
	}
}

uint32 BbdouFoodCtl::getFoodSequenceId() {
	if (_vm->_scriptResource->_properties.get(0x000E014A))
		return 0x60932;
	if (_vm->_scriptResource->_properties.get(0x000E014B))
		return 0x60933;
	if (_vm->_scriptResource->_properties.get(0x000E014C))
		return 0x60934;
	if (_vm->_scriptResource->_properties.get(0x000E014D))
		return 0x60935;
	if (_vm->_scriptResource->_properties.get(0x000E014E))
		return 0x60936;
	if (_vm->_scriptResource->_properties.get(0x000E014F))
		return 0x60937;
	return 0;
}

uint32 BbdouFoodCtl::getCurrentStudentObjectId() {
	if (_vm->_scriptResource->_properties.get(0x000E0146))
		return 0x40077;
	if (_vm->_scriptResource->_properties.get(0x000E0147))
		return 0x40255;
	if (_vm->_scriptResource->_properties.get(0x000E0148))
		return 0x40256;
	if (_vm->_scriptResource->_properties.get(0x000E0149))
		return 0x40257;
	return 0;
}

} // End of namespace Illusions
