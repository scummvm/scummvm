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
#include "fullpipe/messages.h"

#include "fullpipe/constants.h"
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

StaticANIObject::StaticANIObject(StaticANIObject *src) : GameObject(src) {
	_shadowsOn = src->_shadowsOn;
	_field_30 = src->_field_30;
	_field_34 = 1;
	_initialCounter = 0;

	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_someDynamicPhaseIndex = -1;
	_sceneId = src->_sceneId;
	_callback1 = src->_callback1;
	_callback2 = src->_callback2;

	for (uint i = 0; i < src->_staticsList.size(); i++)
		_staticsList.push_back(new Statics((Statics *)src->_staticsList[i], 0));

	_movement = 0;
	_statics = 0;

	for (uint i = 0; i < src->_movements.size(); i++) {
		Movement *mov;
		if (((Movement *)src->_movements[i])->_currMovement) {
			mov = new Movement(getMovementById(src->getMovementIdById(((Movement *)src->_movements[i])->_id)), this);
			mov->_id = ((Movement *)src->_movements[i])->_id;
		} else {
			mov = new Movement(((Movement *)src->_movements[i]), 0, -1, this);
		}

		_movements.push_back(mov);
	}
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

int StaticANIObject::getMovementIdById(int itemId) {
	for (uint i = 0; i < _movements.size(); i++) {
		Movement *mov = (Movement *)_movements[i];
		if (mov->_currMovement) {
			if (mov->_id == itemId)
				return mov->_id;
			if (mov->_currMovement->_id == itemId)
				return mov->_id;
		}
	}

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
	if (_flags & 4 == 0)
		return;

	Common::Point point;
	Common::Rect rect;

	debug(0, "StaticANIObject::draw()");

	if (_shadowsOn && g_fullpipe->_currentScene && g_fullpipe->_currentScene->_shadows
		&& (getCurrDimensions(point)->x != 1 || getCurrDimensions(point)->y != 1)) {

		DynamicPhase *dyn;

		if (!_movement || _flags & 0x20 )
			dyn = _statics;
		else
			dyn = _movement->_currDynamicPhase;

		if (dyn->getDynFlags() & 4) {
			rect = *dyn->_rect;

			DynamicPhase *shd = g_fullpipe->_currentScene->_shadows->findSize(rect.width(), rect.height());
			if (shd) {
				shd->getDimensions(&point);
				int midx = _ox - shd->_x / 2 - dyn->_someX;
				int midy = _oy - shd->_y / 2 - dyn->_someY + rect.bottom - 3;
				int shdy =  shd->_y;

				int px;
				if (!_movement || _flags & 0x20 )
					px = _statics->getCenter(&point)->x;
				else
					px = _movement->getCenter(&point)->x;

				if (_shadowsOn != 1)
					midy = _shadowsOn - shdy / 2;

				shd->draw(px + midx, midy, 0, 0);
			}
		}
	}

	int angle = 0;
	if (_field_30 & 0xC000) {
		if (_field_30 & 0x8000)
			angle = -(_field_30 ^ 0x8000);
		else
			angle = _field_30 ^ 0x4000;
	}

	if (!_movement || (_flags & 0x20)) {
		_statics->getSomeXY(point);
		_statics->_x = _ox - point.x;
		_statics->_y = _oy - point.y;
		_statics->draw(_statics->_x, _statics->_y, 0, angle);
	} else {
		_movement->draw(0, angle);
	}
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

MovTable *StaticANIObject::countMovements() {
	CGameVar *preloadSubVar = g_fullpipe->getGameLoaderGameVar()->getSubVarByName(getName())->getSubVarByName("PRELOAD");

	if (preloadSubVar || preloadSubVar->getSubVarsCount() == 0)
		return 0;

	MovTable *movTable = new MovTable;

	movTable->count = _movements.size();
	movTable->movs = (int16 *)calloc(_movements.size(), sizeof(int16));

	for (uint i = 0; i < _movements.size(); i++) {
		GameObject *obj = (GameObject *)_movements[i];
		movTable->movs[i] = 2;

		for (CGameVar *sub = preloadSubVar->_subVars; sub; sub = sub->_nextVarObj) {
			if (scumm_stricmp(obj->getName(), sub->_varName) == 0) {
				movTable->movs[i] = 1;
				break;
			}
		}
	}

	return movTable;
}

void StaticANIObject::setSpeed(int speed) {
	warning("STUB: StaticANIObject::setSpeed(%d)", speed);
}

void StaticANIObject::initMovements() {
	for (uint i = 0; i < _movements.size(); i++)
		((Movement *)_movements[i])->removeFirstPhase();
}

Common::Point *StaticANIObject::getCurrDimensions(Common::Point &p) {
	Picture *pic;

	if (_movement)
		pic = _movement->_currDynamicPhase;
	else
		pic = _statics;

	if (pic) {
		Common::Point point;

		pic->getDimensions(&point);
		p.x = point.x;
		p.y = point.y;
	} else {
		p.x = 0;
		p.y = 0;
	}

	return &p;
}

void StaticANIObject::update(int counterdiff) {
	warning("STUB: StaticANIObject::update(%d)", counterdiff);
}

bool StaticANIObject::setPicAniInfo(PicAniInfo *picAniInfo) {
	if (!(picAniInfo->type & 3)) {
		warning("StaticANIObject::setPicAniInfo(): Wrong type: %d", picAniInfo->type);

		return false;
	}

	if (picAniInfo->type & 3) {
		setOXY(picAniInfo->ox, picAniInfo->oy);
		_priority = picAniInfo->priority;
		_field_4 = picAniInfo->field_8;
		setFlags(picAniInfo->flags);
		_field_8 = picAniInfo->field_24;
	}

	if (picAniInfo->type & 1) {
		_messageQueueId = picAniInfo->type >> 16;

		if (picAniInfo->staticsId)
			_statics = getStaticsById(picAniInfo->staticsId);
		else
			_statics = 0;

		if (picAniInfo->movementId) {
			_movement = getMovementById(picAniInfo->movementId);
			if (_movement)
				_movement->setDynamicPhaseIndex(picAniInfo->dynamicPhaseIndex);
		} else {
			_movement = 0;
		}

		setSomeDynamicPhaseIndex(picAniInfo->someDynamicPhaseIndex);
	}

	return true;
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

Common::Point *Statics::getCenter(Common::Point *p) {
	Common::Rect rect;

	rect = *_rect;

	if (_staticsId & 0x4000) {
		Common::Point point;

		getDimensions(&point);
		rect.moveTo(point.x - _rect->right, _rect->top);
	}

	p->x = rect.left + _rect->width() / 2;
	p->y = rect.top + _rect->height() / 2;

	return p;
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
	_currMovement = 0;
	_counter = 0;
	_counterMax = 83;
}

Movement::Movement(Movement *src, StaticANIObject *ani) {
	_lastFrameSpecialFlag = 0;
	_flipFlag = src->_flipFlag;
	_updateFlag1 = src->_updateFlag1;
	_staticsObj1 = 0;
	_staticsObj2 = 0;
	_mx = 0;
	_my = 0;
	_m2x = 0;
	_m2y = 0;

	_field_78 = 0;
	_framePosOffsets = 0;
	_field_84 = 0;
	_currDynamicPhase = 0;
	_field_8C = 0;
	_currDynamicPhaseIndex = src->_currDynamicPhaseIndex;
	_field_94 = 0;

	_currMovement = src;
	_ox = src->_ox;
	_oy = src->_oy;

	initStatics(ani);

	_counterMax = src->_counterMax;
	_counter = src->_counter;
	_field_50 = src->_field_50;

	updateCurrDynamicPhase();
}

Movement::Movement(Movement *src, int *flag1, int flag2, StaticANIObject *ani) {
	warning("STUB: Movement(src, %p, %d, ani)", (void *)flag1, flag2);
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

		_currMovement = ani->getMovementById(movid);
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

Common::Point *Movement::getDimensionsOfPhase(Common::Point *p, int phaseIndex) {
	int idx = phaseIndex;

	if (idx == -1)
		idx = _currDynamicPhaseIndex;

	DynamicPhase *dyn;

	if (_currMovement)
		dyn = (DynamicPhase *)_currMovement->_dynamicPhases[idx];
	else
		dyn = (DynamicPhase *)_dynamicPhases[idx];

	Common::Point point;

	dyn->getDimensions(&point);

	*p = point;

	return p;
}

void Movement::initStatics(StaticANIObject *ani) {
	if (!_currMovement)
		return;

	_staticsObj2 = ani->addReverseStatics(_currMovement->_staticsObj2);
	_staticsObj1 = ani->addReverseStatics(_currMovement->_staticsObj1);
	
	_mx = _currMovement->_mx;
	_my = _currMovement->_my;

	_currMovement->setDynamicPhaseIndex(_currMovement->_updateFlag1 != 0 ? 1 : 0);

	Common::Point point;

	int x1 = _currMovement->_staticsObj1->getDimensions(&point)->x - _mx;

	_mx = x1 - _currMovement->_currDynamicPhase->getDimensions(&point)->x;

	_currMovement->setDynamicPhaseIndex(_currMovement->_currDynamicPhaseIndex);

	_m2x = _currMovement->_m2x;
	_m2y = _currMovement->_m2y;
	_currMovement->gotoLastFrame();

	x1 = _currMovement->_staticsObj2->getDimensions(&point)->x;
	_m2x = _currMovement->_currDynamicPhase->getDimensions(&point)->x - _m2x - x1;
}

void Movement::updateCurrDynamicPhase() {
	if (_dynamicPhases.size() == 0)
		return;

	if (_dynamicPhases[_currDynamicPhaseIndex]) {
		_currDynamicPhase = (DynamicPhase *)_dynamicPhases[_currDynamicPhaseIndex];
	}
}

void Movement::setDynamicPhaseIndex(int index) {
	while (_currDynamicPhaseIndex < index)
		gotoNextFrame(0, 0);

	while (_currDynamicPhaseIndex > index)
		gotoPrevFrame(0, 0);
}

void Movement::loadPixelData() {
	Movement *mov = this;
	for (Movement *i = _currMovement; i; i = i->_currMovement)
		mov = i;

	for (uint i = 0; i < _dynamicPhases.size(); i++) {
		if ((Statics *)_dynamicPhases[i] != mov->_staticsObj2 || !(mov->_staticsObj2->_staticsId & 0x4000) )
			((Statics *)_dynamicPhases[i])->getPixelData();
	}

	if (!(mov->_staticsObj1->_staticsId & 0x4000))
		mov->_staticsObj1->getPixelData();
}

void Movement::removeFirstPhase() {
	if (_updateFlag1) {
		if (!_currDynamicPhaseIndex)
			gotoNextFrame(0, 0);

		if (!_currMovement) {
			_dynamicPhases.remove_at(0);

			for (uint i = 0; i < _dynamicPhases.size(); i++) {
				_framePosOffsets[i - 1]->x = _framePosOffsets[i]->x;
				_framePosOffsets[i - 1]->y = _framePosOffsets[i]->y;
			}
		}
		_currDynamicPhaseIndex--;
	}

	updateCurrDynamicPhase();
	_updateFlag1 = 0;
}

void Movement::gotoNextFrame(int callback1, int callback2) {
	debug(0, "Movement::gotoNextFrame(%d, %d)", callback1, callback2);

	if (!callback2) {
		if (_currMovement) {
			if ((uint)_currDynamicPhaseIndex == _currMovement->_dynamicPhases.size() - 1
				&& !(((DynamicPhase *)(_currMovement->_dynamicPhases.back()))->_countdown)) {
				return;
			}
		} else if ((uint)_currDynamicPhaseIndex == _dynamicPhases.size() - 1
				   && !(((DynamicPhase *)(_dynamicPhases.back()))->_countdown)) {
			return;
		}
	}

	if (_currDynamicPhase->_countdown) {
		_currDynamicPhase->_countdown--;
		return;
	}

	Common::Point point;

	getCurrDynamicPhaseXY(point);
	_ox -= point.x;
	_oy -= point.y;

	int deltax = 0;

	if (_currMovement)
		deltax = _currMovement->getDimensionsOfPhase(&point, _currDynamicPhaseIndex)->x;

	int oldDynIndex = _currDynamicPhaseIndex;

	if (callback2)
		; //callback2(&_currDynamicPhaseIndex);
	else
		_currDynamicPhaseIndex++;

	if (_currMovement) {
		if (_currMovement->_dynamicPhases.size() <= (uint)_currDynamicPhaseIndex)
			_currDynamicPhaseIndex = _currMovement->_dynamicPhases.size() - 1;
		if (_currDynamicPhaseIndex < 0)
			_currDynamicPhaseIndex = 0;
		if (_currMovement->_framePosOffsets) {
			if (callback1) {
				point = *_currMovement->_framePosOffsets[_currDynamicPhaseIndex];
				//callback1(_currDynamicPhaseIndex, &point, _ox, _oy);

				_ox += deltax - point.x;
				_oy += point.y;

				_ox -= _currMovement->getDimensionsOfPhase(&point, _currDynamicPhaseIndex)->x;
			} else if (oldDynIndex >= _currDynamicPhaseIndex) {
				while (oldDynIndex > _currDynamicPhaseIndex) {
					_ox += deltax;
					deltax = _currMovement->getDimensionsOfPhase(&point, oldDynIndex)->x;

					_ox += _currMovement->_framePosOffsets[oldDynIndex]->x;
					_oy -= _currMovement->_framePosOffsets[oldDynIndex]->y;
					oldDynIndex--;

					_ox -= _currMovement->getDimensionsOfPhase(&point, oldDynIndex)->x;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _currDynamicPhaseIndex; i++) {
					_ox += deltax;
					deltax = _currMovement->getDimensionsOfPhase(&point, i)->x;
					_ox -= _currMovement->_framePosOffsets[i]->x;
					_oy += _currMovement->_framePosOffsets[i]->y;
					_ox -= _currMovement->getDimensionsOfPhase(&point, i)->x;
				}
			}
		}
	} else {
		if (_dynamicPhases.size() <= (uint)_currDynamicPhaseIndex)
			_currDynamicPhaseIndex = _dynamicPhases.size() - 1;
		if (_currDynamicPhaseIndex < 0)
			_currDynamicPhaseIndex = 0;

		if (_framePosOffsets) {
			if (callback1) {
				point.x = _framePosOffsets[_currDynamicPhaseIndex]->x;
				point.y = _framePosOffsets[_currDynamicPhaseIndex]->y;

				//callback1(_currDynamicPhaseIndex, &point, _ox, _oy);
				_ox += point.x;
				_oy += point.y;
			} else if (oldDynIndex >= _currDynamicPhaseIndex) {
				for (int i = oldDynIndex; i > _currDynamicPhaseIndex; i--) {
					_ox -= _framePosOffsets[i]->x;
					_oy -= _framePosOffsets[i]->y;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _currDynamicPhaseIndex; i++) {
					_ox += _framePosOffsets[i]->x;
					_oy += _framePosOffsets[i]->y;
				}
			}
		}
	}

	updateCurrDynamicPhase();
	getCurrDynamicPhaseXY(point);
	_ox += point.x;
	_oy += point.y;

	_currDynamicPhase->_countdown = _currDynamicPhase->_initialCountdown;
}

void Movement::gotoPrevFrame(int callback1, int callback2) {
	warning("STUB: Movement::gotoPrevFrame()");
}

void Movement::gotoLastFrame() {
	if (_currMovement) {
		while ((uint)_currDynamicPhaseIndex != _currMovement->_dynamicPhases.size() - 1)
			gotoNextFrame(0, 0);
	} else {
		while ((uint)_currDynamicPhaseIndex != _dynamicPhases.size() - 1)
			gotoNextFrame(0, 0);
	}
}

Common::Point *Movement::getCenter(Common::Point *p) {
	Common::Rect rect;

	rect = *_currDynamicPhase->_rect;

	if (_currMovement) {
		Common::Point point;

		getDimensionsOfPhase(&point, _currDynamicPhaseIndex);

		rect.moveTo(point.x - _currDynamicPhase->_rect->right, _currDynamicPhase->_rect->top);
	}

	p->x = rect.left + _currDynamicPhase->_rect->width() / 2;
	p->y = rect.top + _currDynamicPhase->_rect->height() / 2;

	return p;
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

	*_rect = *src->_rect;

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
