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
#include "fullpipe/messagequeue.h"

#include "fullpipe/gameobj.h"
#include "fullpipe/objectnames.h"

namespace Fullpipe {

CStepArray::CStepArray() {
	_points = 0;
	_maxPointIndex = 0;
	_currPointIndex = 0;
	_pointsCount = 0;
	_isEos = 0;
}

CStepArray::~CStepArray() {
	if (_pointsCount) {
		for (int i = 0; i < _pointsCount; i++)
			delete _points[i];

		delete _points;

		_points = 0;
	}
}

void CStepArray::clear() {
	_currPointIndex = 0;
	_maxPointIndex = 0;
	_isEos = 0;

	for (int i = 0; i < _pointsCount; i++) {
		_points[i]->x = 0;
		_points[i]->y = 0;
	}
}

StaticANIObject::StaticANIObject() {
	_shadowsOn = 1;
	_field_30 = 0;
	_field_34 = 1;
	_initialCounter = 0;
	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_movement = 0;
	_statics = 0;
	_flags = 0;
	_callback1 = 0;
	_callback2 = 0;
	_sceneId = -1;
	_someDynamicPhaseIndex = -1;
}

bool StaticANIObject::load(MfcArchive &file) {
	debug(5, "StaticANIObject::load()");

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
	
	if (_movement)
		_movement->setOXY(x, y);
}

void StaticANIObject::clearFlags() {
	_flags = 0;

	deleteFromGlobalMessageQueue();
	_messageQueueId = 0;
	_movement = 0;
	_statics = 0;
	_animExFlag = 0;
	_counter = 0;
	_messageNum = 0;
	_stepArray.clear();
}

void StaticANIObject::deleteFromGlobalMessageQueue() {
	while (_messageQueueId) {
		if (g_fullpipe->_globalMessageQueueList->getMessageQueueById(_messageQueueId)) {
			if (!isIdle())
				return;

			g_fullpipe->_globalMessageQueueList->deleteQueueById(_messageQueueId);
		} else {
			_messageQueueId = 0;
		}
	}
}

bool StaticANIObject::isIdle() {
	if (_messageQueueId) {
		MessageQueue *m = g_fullpipe->_globalMessageQueueList->getMessageQueueById(_messageQueueId);

		if (m && m->getFlags() & 1)
			return false;
	}

	return true;
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

Movement *StaticANIObject::getMovementByName(char *name) {
	for (uint i = 0; i < _movements.size(); i++)
		if (!strcmp(((Movement *)_movements[i])->_objectName, name))
			return (Movement *)_movements[i];

	return 0;
}

void Movement::draw(bool flipFlag, int angle) {
	warning("STUB: Movement::draw(%d, %d)", flipFlag, angle);
}


void StaticANIObject::loadMovementsPixelData() {
	for (uint i = 0; i < _movements.size(); i++)
		((Movement *)_movements[i])->loadPixelData();
}

Statics *StaticANIObject::addReverseStatics(Statics *st) {
	Statics *res = getStaticsById(st->_staticsId ^ 0x4000);

	if (!res) {
		res = new Statics(st, true);

		_staticsList.push_back(res);
	}

	return res;
}

void StaticANIObject::draw() {
	warning("STUB: StaticANIObject::draw()");
}

void StaticANIObject::draw2() {
	debug(0, "StatciANIObject::draw2()");

	if ((_flags & 4) && (_flags & 0x10)) {
		if (_movement) {
			_movement->draw(1, 0);
		} else {
			Common::Point point;

			_statics->getSomeXY(point);

			_statics->draw(_ox - point.x, _oy - point.y, 1, 0);
		}
	}
}

Statics::Statics() {
	_staticsId = 0;
	_picture = 0;
	_staticsName = 0;
}

Statics::Statics(Statics *src, bool reverse) : DynamicPhase(src, reverse) {
	_staticsId = src->_staticsId;

	if (reverse) {
		_staticsId ^= 0x4000;
		int newlen = strlen(src->_staticsName) + strlen(sO_MirroredTo) + 1;
		_staticsName = (char *)calloc(newlen, 1);

		snprintf(_staticsName, newlen, "%s%s", sO_MirroredTo, src->_staticsName);
	} else {
		_staticsName = (char *)calloc(strlen(src->_staticsName) + 1, 1);
		strncpy(_staticsName, src->_staticsName, strlen(src->_staticsName) + 1);
	}

	_memfilename = (char *)calloc(strlen(src->_memfilename) + 1, 1);
	strncpy(_memfilename, src->_memfilename, strlen(src->_memfilename) + 1);

	_picture = new Picture();
}

bool Statics::load(MfcArchive &file) {
	debug(5, "Statics::load()");

	DynamicPhase::load(file);

	_staticsId = file.readUint16LE();

	_staticsName = file.readPascalString();
	debug(7, "statics: <%s> id: %d (%x)", transCyrillic((byte *)_staticsName), _staticsId, _staticsId);

	_picture = new Picture();
	_picture->load(file);

	return true;
}

Common::Point *Statics::getSomeXY(Common::Point &p) {
	p.x = _someX;
	p.y = _someY;

	return &p;
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
			_staticsObj1 = ani->addReverseStatics(s);
		}

		_mx = file.readUint32LE();
		_my = file.readUint32LE();

		staticsid = file.readUint16LE();

		_staticsObj2 = ani->getStaticsById(staticsid);

		if (!_staticsObj2 && (staticsid & 0x4000)) {
			Statics *s = ani->getStaticsById(staticsid ^ 0x4000);
			_staticsObj2 = ani->addReverseStatics(s);
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

void Movement::loadPixelData() {
	Movement *mov = this;
	for (Movement *i = _currMovementObj; i; i = i->_currMovementObj)
		mov = i;

	for (uint i = 0; i < _dynamicPhases.size(); i++) {
		if ((Statics *)_dynamicPhases[i] != mov->_staticsObj2 || !(mov->_staticsObj2->_staticsId & 0x4000) )
			((Statics *)_dynamicPhases[i])->getPixelData();
	}

	if (!(mov->_staticsObj1->_staticsId & 0x4000))
		mov->_staticsObj1->getPixelData();
}

DynamicPhase::DynamicPhase() {
	_someX = 0;
	_rect = 0;
	_field_7C = 0;
	_dynFlags = 0;
	_someY = 0;
}

DynamicPhase::DynamicPhase(DynamicPhase *src, bool reverse) {
	_field_7C = src->_field_7C;
	_rect = new Common::Rect();

	if (reverse) {
		if (!src->_bitmap)
			src->init();

		_bitmap = src->_bitmap->reverseImage();
		_data = _bitmap->_pixels;
		_dataSize = src->_dataSize;

		if (g_fullpipe->_currArchive) {
			_field_14 = 0;
			_libHandle = g_fullpipe->_currArchive;
		}

		_flags |= 1;

		_someX = src->_someX;
		_someY = src->_someY;
	} else {
		_field_14 = src->_field_14;
		_field_8 = src->_field_8;
		_flags = src->_flags;

		_memfilename = (char *)calloc(strlen(src->_memfilename) + 1, 1);
		strncpy(_memfilename, src->_memfilename, strlen(src->_memfilename) + 1);
		_dataSize = src->_dataSize;
		_field_10 = src->_field_10;
		_libHandle = src->_libHandle;

		_bitmap = src->_bitmap;
		if (_bitmap)
			_field_54 = 1;

		_someX = src->_someX;
		_someY = src->_someY;
	}

	_rect->top = src->_rect->top;
	_rect->bottom = src->_rect->bottom;
	_rect->left = src->_rect->left;
	_rect->right = src->_rect->right;

	_width = src->_width;
	_height = src->_height;
	_field_7C = src->_field_7C;

	if (src->getExCommand())
		_exCommand = new ExCommand(src->getExCommand());
	else
		_exCommand = 0;

	_initialCountdown = src->_initialCountdown;
	_field_6A = src->_field_6A;
	_dynFlags = src->_dynFlags;

	setPaletteData(getPaletteData());

	copyMemoryObject2(src);
}

bool DynamicPhase::load(MfcArchive &file) {
	debug(5, "DynamicPhase::load()");

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

	_dynFlags = file.readUint32LE();

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
	debug(5, "StaticPhase::load()");

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
