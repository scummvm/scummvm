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

#include "fullpipe/fullpipe.h"

#include "fullpipe/objects.h"
#include "fullpipe/ngiarchive.h"
#include "fullpipe/statics.h"
#include "fullpipe/gameobj.h"

namespace Fullpipe {

StaticANIObject::StaticANIObject() {
	_shadowsOn = 1;
	_field_30 = 0;
	_field_34 = 1;
	_initialCounter = 0;
	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_movementObj = 0;
	_staticsObj = 0;
	_flags = 0;
	_callback1 = 0;
	_callback2 = 0;
	_sceneId = -1;
	_someDynamicPhaseIndex = -1;
}

bool StaticANIObject::load(MfcArchive &file) {
	GameObject::load(file);

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		Statics *st = new Statics();
	  
		st->load(file);
		_staticsList.push_back(st);
	}

	count = file.readUint16LE();
	debug(7, "Movements: %d", count);

	for (int i = 0; i < count; i++) {
	  	int movNum = file.readUint16LE();

		char *movname = genFileName(_id, movNum, "mov");

		Common::SeekableReadStream *f = g_fullpipe->_currArchive->createReadStreamForMember(movname);

		Movement *mov = new Movement();

		MfcArchive archive(f);

		mov->load(archive, this);

		_movements.push_back(mov);

		delete f;
		free(movname);
	}

	Common::Point pt;
	if (count) { // We have movements
		((Movement *)_movements[0])->getCurrDynamicPhaseXY(pt);
	} else {
		pt.x = pt.y = 100;
	}

	setOXY(pt.x, pt.y);

	return true;
}

void StaticANIObject::setOXY(int x, int y) {
	_ox = x;
	_oy = y;
	
	if (_movementObj)
		_movementObj->setOXY(x, y);
}

Statics *StaticANIObject::getStaticsById(int itemId) {
	for (uint i = 0; i < _staticsList.size(); i++)
		if (((Statics *)_staticsList[i])->_staticsId == itemId)
			return (Statics *)_staticsList[i];

	return 0;
}

Movement *StaticANIObject::getMovementById(int itemId) {
	for (uint i = 0; i < _movements.size(); i++)
		if (((Movement *)_movements[i])->_id == itemId)
			return (Movement *)_movements[i];

	return 0;
}

Statics *StaticANIObject::addStatics(Statics *ani) {
	warning("STUB: StaticANIObject::addStatics");

	return 0;
}

Statics::Statics() {
	_staticsId = 0;
	_picture = 0;
}

bool Statics::load(MfcArchive &file) {
	DynamicPhase::load(file);

	_staticsId = file.readUint16LE();

	_stringObj = file.readPascalString();
	debug(7, "statics: <%s>", _stringObj);

	_picture = new Picture();
	_picture->load(file);

	return true;
}

Movement::Movement() {
	_lastFrameSpecialFlag = 0;
	_flipFlag = 0;
	_updateFlag1 = 0;
	_staticsObj1 = 0;
	_staticsObj2 = 0;
	_mx = 0;
	_my = 0;
	_m2x = 0;
	_m2y = 0;
	_field_50 = 1;
	_field_78 = 0;
	_framePosOffsets = 0;
	_field_84 = 0;
	_currDynamicPhase = 0;
	_field_8C = 0;
	_currDynamicPhaseIndex = 0;
	_field_94 = 0;
	_currMovementObj = 0;
	_counter = 0;
	_counterMax = 83;
}

bool Movement::load(MfcArchive &file) {
	warning("STUB: Movement::load");
	return true;
}
bool Movement::load(MfcArchive &file, StaticANIObject *ani) {
	GameObject::load(file);

	int dynCount = file.readUint16LE();

	debug(7, "dynCount: %d  _id: %d", dynCount, _id);
	if (dynCount != 0xffff || _id == MV_MAN_TURN_LU) {
		_framePosOffsets = (Common::Point **)calloc(dynCount + 2, sizeof(Common::Point *));

		for (int i = 0; i < dynCount + 2; i++)
			_framePosOffsets[i] = new Common::Point();

		for (int i = 0; i < dynCount; i++) {
			DynamicPhase *ph = new DynamicPhase();
			ph->load(file);

			_dynamicPhases.push_back(ph);

			_framePosOffsets[i]->x = ph->_x;
			_framePosOffsets[i]->y = ph->_y;
		}

		int staticsid = file.readUint16LE();

		_staticsObj1 = ani->getStaticsById(staticsid);

		if (!_staticsObj1 && (staticsid & 0x4000)) {
			Statics *s = ani->getStaticsById(staticsid ^ 0x4000);
			_staticsObj1 = ani->addStatics(s);
		}

		_mx = file.readUint32LE();
		_my = file.readUint32LE();

		staticsid = file.readUint16LE();

		_staticsObj2 = ani->getStaticsById(staticsid);

		if (!_staticsObj2 && (staticsid & 0x4000)) {
			Statics *s = ani->getStaticsById(staticsid ^ 0x4000);
			_staticsObj2 = ani->addStatics(s);
		}

		_m2x = file.readUint32LE();
		_m2y = file.readUint32LE();

		if (_staticsObj2) {
			_dynamicPhases.push_back(_staticsObj2);

			_framePosOffsets[_dynamicPhases.size() - 1]->x = _m2x;
			_framePosOffsets[_dynamicPhases.size() - 1]->y = _m2y;
		}

	} else {
		int movid = file.readUint16LE();

		_currMovementObj = ani->getMovementById(movid);
		_staticsObj1 = 0;
		_staticsObj2 = 0;

		initStatics(ani);
	}

	if (_staticsObj1 && _staticsObj2) {
		if ((_staticsObj1->_staticsId ^ _staticsObj2->_staticsId) & 0x4000)
			_flipFlag = 1;
	}

	if (g_fullpipe->_gameProjectVersion >= 8)
		_field_50 = file.readUint32LE();

	if (g_fullpipe->_gameProjectVersion < 12)
		_counterMax = 83;
	else
		_counterMax = file.readUint32LE();

	_counter = 0;
	updateCurrDynamicPhase();

	return true;
}

Common::Point *Movement::getCurrDynamicPhaseXY(Common::Point &p) {
	p.x = _currDynamicPhase->_x;
	p.y = _currDynamicPhase->_y;

	return &p;
}

void Movement::initStatics(StaticANIObject *ani) {
	warning("STUB: Movement::initStatics");
}

void Movement::updateCurrDynamicPhase() {
	if (_dynamicPhases.size() == 0)
		return;

	if (_dynamicPhases[_currDynamicPhaseIndex]) {
		_currDynamicPhase = (DynamicPhase *)_dynamicPhases[_currDynamicPhaseIndex];
	}
}

DynamicPhase::DynamicPhase() {
	_someX = 0;
	_rect = 0;
	_field_7C = 0;
	_flags = 0;
	_someY = 0;
}

bool DynamicPhase::load(MfcArchive &file) {
	StaticPhase::load(file);

	_field_7C = file.readUint16LE();
	_rect = new Common::Rect();
	_rect->left = file.readUint32LE();
	_rect->top = file.readUint32LE();
	_rect->right = file.readUint32LE();
	_rect->bottom = file.readUint32LE();

	assert (g_fullpipe->_gameProjectVersion >= 1);

	_someX = file.readUint32LE();
	_someY = file.readUint32LE();

	assert (g_fullpipe->_gameProjectVersion >= 12);

	_flags = file.readUint32LE();

	return true;
}

StaticPhase::StaticPhase() {
	_field_6A = 1;
	_initialCountdown = 0;
	_countdown = 0;
	_field_68 = 0;
	_exCommand = 0;
}

bool StaticPhase::load(MfcArchive &file) {
	Picture::load(file);

	_initialCountdown = file.readUint16LE();
	_field_6A = file.readUint16LE();
	
	if (g_fullpipe->_gameProjectVersion >= 12) {
		_exCommand = (ExCommand *)file.readClass();

		return true;
	}

	assert (g_fullpipe->_gameProjectVersion >= 12);

	return true;
}

} // End of namespace Fullpipe
