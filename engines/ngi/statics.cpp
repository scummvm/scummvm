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

#include "ngi/ngi.h"

#include "ngi/objects.h"
#include "ngi/ngiarchive.h"
#include "ngi/statics.h"
#include "ngi/messages.h"
#include "ngi/interaction.h"
#include "ngi/motion.h"

#include "ngi/constants.h"
#include "ngi/objectnames.h"

namespace NGI {

StepArray::StepArray() {
	_currPointIndex = 0;
	_isEos = false;
}

void StepArray::clear() {
	_currPointIndex = 0;
	_isEos = false;
	_points.clear();
}

Common::Point StepArray::getCurrPoint() const {
	if (_isEos || !_points.size()) {
		return Common::Point();
	}

	return Common::Point(_points[_currPointIndex].x,
						 _points[_currPointIndex].y);
}

Common::Point StepArray::getPoint(int index, int offset) const {
	if (index == -1)
		index = _currPointIndex;

	if (index + offset > getPointsCount() - 1)
		offset = getPointsCount() - index;

	Common::Point point;

	while (offset >= 1) {
		point.x += _points[index].x;
		point.y += _points[index].y;

		index++;
		offset--;
	}

	return point;
}

bool StepArray::gotoNextPoint() {
	if (_currPointIndex < getPointsCount() - 1) {
		_currPointIndex++;
		return true;
	} else {
		_isEos = true;
		return false;
	}
}

void StepArray::insertPoints(const PointList &points) {
	_points.resize(_currPointIndex + points.size());
	Common::copy(points.begin(), points.end(), _points.begin() + _currPointIndex);
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
	_callback1 = 0; // Really NULL
	_callback2 = 0; // Really NULL
	_sceneId = -1;
	_someDynamicPhaseIndex = -1;

	_field_32 = 0;
	_field_96 = 0;
	_messageNum = 0;
	_objtype = kObjTypeStaticANIObject;
}

StaticANIObject::~StaticANIObject() {
	Common::for_each(_staticsList.begin(), _staticsList.end(), Common::DefaultDeleter<Statics>());
	Common::for_each(_movements.begin(), _movements.end(), Common::DefaultDeleter<Movement>());
	g_nmi->_aniHandler->detachAllObjects();
}

StaticANIObject::StaticANIObject(StaticANIObject *src) : GameObject(src) {
	_shadowsOn = src->_shadowsOn;
	_field_30 = src->_field_30;
	_field_34 = 1;
	_initialCounter = 0;

	_field_32 = 0;
	_field_96 = 0;
	_messageNum = 0;

	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_someDynamicPhaseIndex = -1;
	_sceneId = src->_sceneId;
	_callback1 = src->_callback1;
	_callback2 = src->_callback2;
	_objtype = kObjTypeStaticANIObject;

	for (uint i = 0; i < src->_staticsList.size(); i++)
		_staticsList.push_back(new Statics(src->_staticsList[i], false));

	_movement = 0;
	_statics = 0;

	for (uint i = 0; i < src->_movements.size(); i++) {
		Movement *newmov;

		if (src->_movements[i]->_currMovement) {
			// This is weird code. Logically it should be
			// newmov = new Movement(src->getMovementById(src->getMovementIdById(src->_movements[i]->_id)), this);
			newmov = new Movement(getMovementById(src->getMovementIdById(src->_movements[i]->_id)), this);
			newmov->_id = src->_movements[i]->_id;
		} else {
			newmov = new Movement(src->_movements[i], 0, -1, this);
		}

		_movements.push_back(newmov);
	}
}

bool StaticANIObject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "StaticANIObject::load()");

	GameObject::load(file);

	debugC(6, kDebugXML, "%% <OLDANI %s>", GameObject::toXML().c_str());

	int count = file.readUint16LE();

	for (int i = 0; i < count; i++) {
		Statics *st = new Statics();

		st->load(file);
		_staticsList.push_back(st);
	}

	count = file.readUint16LE();
	debugC(7, kDebugLoading, "Movements: %d", count);

	for (int i = 0; i < count; i++) {
		int movNum = file.readUint16LE();

		Common::String movname = genFileName(_id, movNum, "mov");

		Common::ScopedPtr<Common::SeekableReadStream> f(g_nmi->_currArchive->createReadStreamForMember(movname));

		Movement *mov = new Movement();
		_movements.push_back(mov);

		MfcArchive archive(f.get());

		mov->load(archive, this);
	}

	Common::Point pt;
	if (count) { // We have movements
		pt = _movements[0]->getCurrDynamicPhaseXY();
	} else {
		pt.x = pt.y = 100;
	}

	debugC(6, kDebugXML, "%% </OLDANI>");

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

void StaticANIObject::setFlags40(bool state) {
	if (state) {
		_flags |= 0x40;
	} else {
		if (_flags & 0x40)
			_flags ^= 0x40;
	}
}

void StaticANIObject::deleteFromGlobalMessageQueue() {
	while (_messageQueueId) {
		if (g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId)) {
			if (!isIdle())
				return;

			g_nmi->_globalMessageQueueList->deleteQueueById(_messageQueueId);
		} else {
			_messageQueueId = 0;
		}
	}
}

bool StaticANIObject::queueMessageQueue(MessageQueue *mq) {
	if (_flags & 0x80)
		return false;

	if (isIdle()) {
		deleteFromGlobalMessageQueue();
		_messageQueueId = 0;
		_messageNum = 0;

		if (_flags & 2)
			_flags ^= 2;

		if (mq) {
			_animExFlag = 0;
			if (_movement)
				_messageQueueId = mq->_id;
			else
				mq->sendNextCommand();
		} else {
			_messageQueueId = 0;
		}
	}

	return true;
}

void StaticANIObject::restartMessageQueue(MessageQueue *mq) {
	ExCommand *ex = mq->getExCommandByIndex(0);
	if (ex) {
		while (ex->_messageKind != 1 || ex->_parentId != _id) {
			ex->_parId = 0;
			ex->_excFlags |= 2;
			ex->handleMessage();

			mq->deleteExCommandByIndex(0, 0);

			ex = mq->getExCommandByIndex(0);

			if (!ex)
				return;
		}

		if (ex) {
			startAnim(ex->_messageNum, mq->_id, -1);
			mq->deleteExCommandByIndex(0, 1);
		}
	}
}

MessageQueue *StaticANIObject::getMessageQueue() {
	if (this->_messageQueueId <= 0)
		return 0;

	return g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId);
}

bool StaticANIObject::trySetMessageQueue(int msgNum, int qId) {
	if (_messageQueueId || !msgNum) {
		updateGlobalMessageQueue(qId, _id);
		return false;
	}

	_flags |= 2;

	_messageNum = msgNum;
	_messageQueueId = qId;

	return true;
}

void StaticANIObject::startMQIfIdle(int qId, int flag) {
	MessageQueue *msg = g_nmi->_currentScene->getMessageQueueById(qId);

	if (msg && isIdle() && !(_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(msg, 0, 0);

		mq->setFlags(mq->getFlags() | flag);

		ExCommand *ex = mq->getExCommandByIndex(0);

		if (ex) {
			while (ex->_messageKind != 1 || ex->_parentId != _id) {
				ex->_parId = 0;
				ex->_excFlags |= 2;
				ex->handleMessage();

				mq->deleteExCommandByIndex(0, 0);

				ex = mq->getExCommandByIndex(0);

				if (!ex)
					return;
			}

			if (ex) {
				startAnim(ex->_messageNum, mq->_id, -1);
				mq->deleteExCommandByIndex(0, 1);
			}
		}
	}
}

bool StaticANIObject::isIdle() {
	assert(_objtype == kObjTypeStaticANIObject);

	if (_messageQueueId) {
		MessageQueue *m = g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId);

		if (m && m->getFlags() & 1)
			return false;
	}

	return true;
}

Statics *StaticANIObject::getStaticsById(int itemId) {
	for (uint i = 0; i < _staticsList.size(); i++)
		if (_staticsList[i]->_staticsId == itemId)
			return _staticsList[i];

	return 0;
}

Statics *StaticANIObject::getStaticsByName(const Common::String &name) {
	for (uint i = 0; i < _staticsList.size(); i++)
		if (_staticsList[i]->_staticsName == name)
			return _staticsList[i];

	return 0;
}

Movement *StaticANIObject::getMovementById(int itemId) {
	for (uint i = 0; i < _movements.size(); i++)
		if (_movements[i]->_id == itemId)
			return _movements[i];

	return 0;
}

int StaticANIObject::getMovementIdById(int itemId) const {
	for (uint i = 0; i < _movements.size(); i++) {
		Movement *mov = _movements[i];

		if (mov->_currMovement) {
			if (mov->_id == itemId)
				return mov->_currMovement->_id;

			if (mov->_currMovement->_id == itemId)
				return mov->_id;
		}
	}

	return 0;
}

Movement *StaticANIObject::getMovementByName(const Common::String &name) {
	for (uint i = 0; i < _movements.size(); i++)
		if (_movements[i]->_objectName == name)
			return _movements[i];

	return 0;
}

bool StaticANIObject::isPixelHitAtPos(int x, int y) {
	return getPixelAtPos(x, y, 0, true);
}

bool StaticANIObject::getPixelAtPos(int x, int y, uint32 *pixel, bool hitOnly) {
	bool res = false;
	Picture *pic;

	if (_movement)
		pic = _movement->_currDynamicPhase;
	else
		pic = _statics;

	if (!pic)
		return false;

	int ongoing;
	int xani, yani;
	int oxani, oyani;

	if (_movement)
		ongoing = _movement->_currMovement != 0;
	else
		ongoing = _statics->_staticsId & 0x4000;

	if (_movement) {
		const Common::Point point = _movement->getCurrDynamicPhaseXY();
		xani = point.x;
		yani = point.y;
		oxani = _movement->_ox;
		oyani = _movement->_oy;
	} else {
		const Common::Point point = _statics->getSomeXY();
		xani = point.x;
		yani = point.y;
		oxani = _ox;
		oyani = _oy;
	}

	int xtarget = x - (oxani - xani);
	int ytarget = y - (oyani - yani);

	if (ongoing && _movement)
		xtarget = pic->getDimensions().x - xtarget;

	x = pic->_x;
	y = pic->_y;
	pic->_x = 0;
	pic->_y = 0;

	if (hitOnly)
		return pic->isPixelHitAtPos(xtarget, ytarget);

	if (pic->isPixelHitAtPos(xtarget, ytarget)) {
		*pixel = pic->getPixelAtPos(xtarget, ytarget);

		res = true;
	} else {
		res = false;
	}
	pic->_x = x;
	pic->_y = y;

	return res;
}

void Movement::draw(bool flipFlag, int angle) {
	debugC(3, kDebugDrawing, "Movement::draw(%d, %d)", flipFlag, angle);

	Common::Point point = getCurrDynamicPhaseXY();

	int x = _ox - point.x;
	int y = _oy - point.y;

	if (_currDynamicPhase->getPaletteData().size)
		g_nmi->_globalPalette = &_currDynamicPhase->getPaletteData();

	Common::ScopedPtr<Bitmap> bmp;
	if (_currMovement) {
		bmp.reset(_currDynamicPhase->getPixelData()->reverseImage());
	} else {
		bmp.reset(_currDynamicPhase->getPixelData()->reverseImage(false));
	}

	if (flipFlag) {
		bmp->flipVertical()->drawShaded(1, x, y + 30 + _currDynamicPhase->_rect.bottom, _currDynamicPhase->getPaletteData(), _currDynamicPhase->getAlpha());
	} else if (angle) {
		bmp->drawRotated(x, y, angle, _currDynamicPhase->getPaletteData(), _currDynamicPhase->getAlpha());
	} else {
		bmp->putDib(x, y, _currDynamicPhase->getPaletteData(), _currDynamicPhase->getAlpha());
	}

	if (_currDynamicPhase->_rect.top) {
		if (!_currDynamicPhase->getConvertedBitmap()) {
			//v12 = Picture_getPixelData(v5);
			//v13 = Bitmap_convertTo16Bit565(v12, (unsigned int *)&_currDynamicPhase->rect);
			//_currDynamicPhase->convertedBitmap = v13;
		}

		if (_currDynamicPhase->getConvertedBitmap()) {
			if (_currMovement) {
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 1, LOBYTE(_currDynamicPhase->rect.top));
				bmp.reset(_currDynamicPhase->getConvertedBitmap()->reverseImage());
				bmp->putDib(x, y, _currDynamicPhase->getPaletteData(), _currDynamicPhase->getAlpha());
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 0, 255);
			} else {
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 1, LOBYTE(_currDynamicPhase->rect.top));
				bmp.reset(_currDynamicPhase->getConvertedBitmap()->reverseImage(false));
				bmp->putDib(x, y, _currDynamicPhase->getPaletteData(), _currDynamicPhase->getAlpha());
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 0, 255);
			}
		}
	}
}

void StaticANIObject::loadMovementsPixelData() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->loadPixelData();
}

void StaticANIObject::freeMovementsPixelData() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->freePixelData();
}

Statics *StaticANIObject::addReverseStatics(Statics *st) {
	assert(st);
	Statics *res = getStaticsById(st->_staticsId ^ 0x4000);

	if (!res) {
		res = new Statics(st, true);
		_staticsList.push_back(res);
	}

	return res;
}

void StaticANIObject::draw() {
	if ((_flags & 4) == 0)
		return;

	debugC(6, kDebugDrawing, "StaticANIObject::draw() (%s) [%d] [%d, %d]", transCyrillic(_objectName), _id, _ox, _oy);

	if (_shadowsOn && g_nmi->_currentScene && g_nmi->_currentScene->_shadows
		&& (getCurrDimensions().x != 1 || getCurrDimensions().y != 1)) {

		DynamicPhase *dyn;

		if (!_movement || _flags & 0x20)
			dyn = _statics;
		else
			dyn = _movement->_currDynamicPhase;

		if (!dyn) {
			warning("HACK: StaticANIObject::draw(): dyn is missing");
			return;
		}

		if (dyn->getDynFlags() & 4) {
			const Common::Rect &rect = dyn->_rect;

			DynamicPhase *shd = g_nmi->_currentScene->_shadows->findSize(rect.width(), rect.height());
			if (shd) {
				const Dims dims = shd->getDimensions();
				int midx = _ox - dims.x / 2 - dyn->_someX;
				int midy = _oy - dims.y / 2 - dyn->_someY + rect.bottom - 3;
				int shdw =  dims.y;

				int px;
				if (!_movement || (_flags & 0x20))
					px = _statics->getCenter().x;
				else
					px = _movement->getCenter().x;

				if (_shadowsOn != 1)
					midy = _shadowsOn - shdw / 2;

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
		const Common::Point point = _statics->getSomeXY();
		_statics->_x = _ox - point.x;
		_statics->_y = _oy - point.y;
		_statics->draw(_statics->_x, _statics->_y, 0, angle);
	} else {
		_movement->draw(0, angle);
	}
}

void StaticANIObject::draw2() {
	debugC(6, kDebugDrawing, "StatciANIObject::draw2(): id: (%s) %d [%d, %d]", transCyrillic(_objectName), _id, _ox, _oy);

	if ((_flags & 4) && (_flags & 0x10)) {
		if (_movement) {
			_movement->draw(1, 0);
		} else {
			const Common::Point point = _statics->getSomeXY();
			_statics->draw(_ox - point.x, _oy - point.y, 1, 0);
		}
	}
}

MovTable *StaticANIObject::countMovements() {
	GameVar *preloadSubVar = g_nmi->getGameLoaderGameVar()->getSubVarByName(getName())->getSubVarByName("PRELOAD");

	if (!preloadSubVar || preloadSubVar->getSubVarsCount() == 0)
		return nullptr;

	MovTable *movTable = new MovTable;
	movTable->reserve(_movements.size());
	for (uint i = 0; i < _movements.size(); i++) {
		int16 value = 2;
		for (GameVar *sub = preloadSubVar->_subVars; sub; sub = sub->_nextVarObj) {
			if (scumm_stricmp(_movements[i]->getName().c_str(), sub->_varName.c_str()) == 0) {
				value = 1;
				break;
			}
		}
		movTable->push_back(value);
	}

	return movTable;
}

void StaticANIObject::setSpeed(int speed) {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName(getName())->getSubVarByName("SpeedUp");

	if (!var)
		return;

	for (var = var->_subVars; var; var = var->_nextVarObj) {
		Movement *mov = getMovementById(var->_value.intValue);

		if (mov) {
			if (speed) {
				if (mov->_counterMax == 83)
					mov->_counterMax = 41;
			} else if (mov->_counterMax == 41) {
				mov->_counterMax = 83;
			}
		}
	}

}

void StaticANIObject::setAlpha(int alpha) {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->setAlpha(alpha);

	for (uint i = 0; i < _staticsList.size(); i++)
		_staticsList[i]->setAlpha(alpha);
}

void StaticANIObject::initMovements() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->removeFirstPhase();
}

void StaticANIObject::preloadMovements(MovTable *mt) {
	if (mt) {
		for (uint i = 0; i < _movements.size(); i++) {
			Movement *mov = _movements[i];

			if ((*mt)[i] == 1)
				mov->loadPixelData();
			else if ((*mt)[i] == 2)
				mov->freePixelData();
		}
	}
}

Dims StaticANIObject::getCurrDimensions() const {
	Picture *pic;

	if (_movement)
		pic = _movement->_currDynamicPhase;
	else
		pic = _statics;

	if (pic) {
		return pic->getDimensions();
	}

	return Dims();
}

Common::Point StaticANIObject::getSomeXY() const {
	if (_movement) {
		return _movement->getCurrDynamicPhaseXY();
	}

	if (_statics)
		return _statics->getSomeXY();

	error("No someXY found");
}

void StaticANIObject::update(int counterdiff) {
	int mqid;

	debugC(6, kDebugAnimation, "StaticANIObject::update() (%s) [%d] [%d, %d] fl: %x", transCyrillic(_objectName), _id, _ox, _oy, _flags);

	if (_flags & 2) {
		_messageNum--;
		if (_messageNum)
			return;

		mqid = _messageQueueId;
		_messageQueueId = 0;
		_flags ^= 2;

		updateGlobalMessageQueue(mqid, _id);
		return;
	}

	ExCommand *ex, *newex;

	if (_movement) {
		_movement->_counter += counterdiff;

		if (_movement->_counter < _movement->_counterMax)
			return;

		_movement->_counter = 0;

		if (_flags & 1) {
			if (_counter) {
				_counter--;

				return;
			}

			DynamicPhase *dyn = _movement->_currDynamicPhase;
			if (dyn->_initialCountdown == dyn->_countdown) {

				ex = dyn->getExCommand();
				if (ex && ex->_messageKind != 35) {
					newex = ex->createClone();
					newex->_excFlags |= 2;
					if (newex->_messageKind == 17) {
						newex->_parentId = _id;
						newex->_param = _odelay;
					}
					newex->sendMessage();

					if (!_movement)
						return;
				}
			}

			if (dyn->_initialCountdown == dyn->_countdown && dyn->_field_68 != 0) {
				newex = new ExCommand(_id, 17, dyn->_field_68, 0, 0, 0, 1, 0, 0, 0);
				newex->_excFlags = 2;
				newex->_param = _odelay;
				newex->sendMessage();

				if (!_movement)
					return;
			}

			if (!_movement->gotoNextFrame(_callback1, _callback2)) {
				stopAnim_maybe();
			} else {
				setOXY(_movement->_ox, _movement->_oy);
				_counter = _initialCounter;

				if (dyn->_initialCountdown == dyn->_countdown) {
					ex = dyn->getExCommand();
					if (ex) {
						if (ex->_messageKind == 35) {
							newex = ex->createClone();
							newex->_excFlags |= 2;
							newex->sendMessage();
						}
					}
				}
			}
			if (!_movement)
				return;

			const Common::Point point = _stepArray.getCurrPoint();
			setOXY(point.x + _ox, point.y + _oy);
			_stepArray.gotoNextPoint();
			if (_someDynamicPhaseIndex == _movement->_currDynamicPhaseIndex)
				adjustSomeXY();
		} else if (_flags & 0x20) {
			_flags ^= 0x20;
			_flags |= 1;

			_movement->gotoFirstFrame();

			const Common::Point point = _movement->getCurrDynamicPhaseXY();
			const Common::Point pointS = _statics->getSomeXY();
			_movement->setOXY(_ox + point.x + _movement->_mx - pointS.x,
							  _oy + point.y + _movement->_my - pointS.y);
		}
	} else {
		if (_statics) {
			if (_messageQueueId) {
				if (_statics->_countdown) {
					_statics->_countdown--;
					return;
				}
				mqid = _messageQueueId;
				_messageQueueId = 0;
				updateGlobalMessageQueue(mqid, _id);
			}
		}
	}
}

void StaticANIObject::updateStepPos() {
	Common::Point point;

	int ox = _movement->_ox;
	int oy = _movement->_oy;

	point = _movement->calcSomeXY(1, _someDynamicPhaseIndex);
	int x = point.x;
	int y = point.y;

	point = _stepArray.getPoint(-1, _stepArray.getPointsCount());
	x += point.x;
	y += point.y;

	_statics = _movement->_staticsObj2;
	_movement = 0;

	setOXY(ox + x, oy + y);
}

Common::Point *StaticANIObject::calcNextStep(Common::Point *pRes) {
	if (!_movement) {
		pRes->x = 0;
		pRes->y = 0;

		return pRes;
	}

	Common::Point point = _movement->calcSomeXY(1, _someDynamicPhaseIndex);

	int resX = point.x;
	int resY = point.y;

	int pointN, offset;

	if (_someDynamicPhaseIndex <= 0) {
		pointN = _stepArray.getCurrPointIndex();
		offset = _stepArray.getPointsCount() - _stepArray.getCurrPointIndex();
	} else {
		pointN = _stepArray.getCurrPointIndex();
		offset = 1 - _movement->_currDynamicPhaseIndex + _someDynamicPhaseIndex;
	}

	if (pointN >= 0) {
		point = _stepArray.getPoint(pointN, offset);

		resX += point.x;
		resY += point.y;
	}

	pRes->x = resX;
	pRes->y = resY;

	return pRes;
}

void StaticANIObject::stopAnim_maybe() {
	debugC(2, kDebugAnimation, "StaticANIObject::stopAnim_maybe()");

	if (!(_flags & 1))
		return;

	_flags ^= 1;

	int oid = 0;
	int oldmqid = _messageQueueId;
	Common::Point point;

	if (_movement) {
		setOXY(_movement->_ox, _movement->_oy);

		if (_flags & 0x40) {
			if (!_movement->_currMovement) {
				if (_movement->_currDynamicPhaseIndex)
					goto L11;
L8:
				_statics = _movement->_staticsObj1;
				point = _movement->getCurrDynamicPhaseXY();
				_ox -= point.x;
				_oy -= point.y;

				_ox -= _movement->_mx;
				_oy -= _movement->_my;

				point = _statics->getSomeXY();
				if (_movement->_currMovement) {
					_oy += point.y;
					_ox -= point.x;
					_ox += _statics->getDimensions().x;
				} else {
					_ox += point.x;
					_oy += point.y;
				}
				goto L12;
			}
			if (!_movement->_currDynamicPhaseIndex)
				goto L8;
		}
L11:
		_statics = _movement->_staticsObj2;
L12:
		point = _statics->getSomeXY();

		_statics->_x = _ox - point.x;
		_statics->_y = _oy - point.y;
		oid = _movement->_id;
		_movement = 0;

		ExCommand *ex = new ExCommand(_id, 17, 24, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _odelay;
		ex->_excFlags = 2;
		ex->postMessage();
	}

	int mqid = _messageQueueId;

	if (_animExFlag) {
		_messageQueueId = 0;
		startAnimEx(oid, mqid, -1, -1);
	} else {
		if (_messageQueueId == oldmqid) {
			_messageQueueId = 0;
			if (_field_34 == 1)
				updateGlobalMessageQueue(mqid, _id);
		}
	}
}

void StaticANIObject::adjustSomeXY() {
	if (_movement) {
		Common::Point point;

		point = _movement->calcSomeXY(0, -1);

		int diff = abs(point.y) - abs(point.x);

		point = _movement->calcSomeXY(1, -1);

		if (diff > 0)
			_ox += point.x;
		else
			_oy += point.y;

		_statics = _movement->_staticsObj2;
		_movement = 0;
		_someDynamicPhaseIndex = -1;
	}
}

MessageQueue *StaticANIObject::changeStatics1(int msgNum) {
	g_nmi->_aniHandler->attachObject(_id);

	MessageQueue *mq = g_nmi->_aniHandler->makeQueue(this, msgNum, 0, 0, 0);

	if (!mq)
		return 0;

	if (mq->getCount() <= 0) {
		g_nmi->_globalMessageQueueList->addMessageQueue(mq);

		if (_flags & 1)
			_messageQueueId = mq->_id;
	} else {
		if (!queueMessageQueue(mq)) {
			delete mq;

			return 0;
		}

		g_nmi->_globalMessageQueueList->addMessageQueue(mq);
	}

	return mq;
}

void StaticANIObject::changeStatics2(int objId) {
	_animExFlag = 0;

	deleteFromGlobalMessageQueue();

	if (_movement || _statics) {
		g_nmi->_aniHandler->attachObject(_id);
		g_nmi->_aniHandler->putObjectToStatics(this, objId);
	} else {
		_statics = getStaticsById(objId);
	}

	if (_messageQueueId) {
		if (g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId))
			g_nmi->_globalMessageQueueList->deleteQueueById(_messageQueueId);

		_messageQueueId = 0;
	}
}

void StaticANIObject::hide() {
	if (!_messageQueueId) {
		if (_flags & 4)
			_flags ^= 4;
	}
}

void StaticANIObject::show1(int x, int y, int movId, int mqId) {
	debugC(6, kDebugAnimation, "StaticANIObject::show1(%d, %d, %d, %d)", x, y, movId, mqId);

	if (_messageQueueId)
		return;

	if (movId == -1) {
		_flags |= 4u;
		if (x != -1 && y != -1) {
			setOXY(x, y);
		}

		return;
	}

	Movement *mov = getMovementById(movId);
	if (!mov)
		return;

	if (x != -1 && y != -1) {
		setOXY(x, y);
	}

	_statics = mov->_staticsObj1;

	const Common::Point point = mov->_staticsObj1->getSomeXY();
	_statics->_x = x - point.x;
	_statics->_y = y - point.y;

	_statics->_countdown = _statics->_initialCountdown;

	_flags |= 4;
	_ox = x;
	_oy = y;
	_movement = 0;

	if (mov->_currMovement)
		_flags |= 8;
	else if (_flags & 8)
		_flags ^= 8;

	if (_flags & 1)
		_flags ^= 1;

	_messageQueueId = mqId;
}

void StaticANIObject::show2(int x, int y, int movementId, int mqId) {
	if (movementId == -1) {
		_flags |= 4u;
		return;
	}

	if (!_messageQueueId) {
		_messageQueueId = mqId;

		Movement *mov = getMovementById(movementId);

		if (mov) {
			_statics = mov->_staticsObj1;
			_movement = mov;
			mov->gotoLastFrame();
			mov->setOXY(x, y);
			mov->gotoFirstFrame();

			Common::Point point = mov->getCurrDynamicPhaseXY();
			_statics->_x = mov->_ox - point.x - mov->_mx;
			_statics->_y = mov->_oy - point.y - mov->_my;

			point = _statics->getSomeXY();
			_flags |= 4;
			_ox = _statics->_x + point.x;
			_oy = _statics->_y + point.y;

			if (mov->_currMovement) {
				_flags |= 8;
			} else {
				if (_flags & 8)
					_flags ^= 8;
			}

			if (_flags & 1)
				_flags ^= 1;

			_flags |= 0x20;
		}
	}
}

void StaticANIObject::playIdle() {
	if (isIdle())
		adjustSomeXY();
}

void StaticANIObject::startAnimSteps(int movementId, int messageQueueId, int x, int y, const PointList &points, int someDynamicPhaseIndex) {
	Movement *mov = 0;

	if (!(_flags & 0x80)) {
		if (!_messageQueueId)
			for (uint i = 0; i < _movements.size(); i++) {
				if (_movements[i]->_id == movementId) {
					mov = _movements[i];
					break;
				}
			}
	}

	if (!mov) {
		updateGlobalMessageQueue(messageQueueId, _id);

		return;
	}


	if (_movement || !_statics)
		return;

	Common::Point point = _statics->getSomeXY();

	int newx = _ox - point.x;
	int newy = _oy - point.y;

	_movement = mov;

	if (_flags & 0x40)
		_movement->gotoLastFrame();
	else
		_movement->gotoFirstFrame();

	_stepArray.clear();
	_stepArray.insertPoints(points);

	if (!(_flags & 0x40)) {
		if (!_movement->_currDynamicPhaseIndex) {
			point = _stepArray.getCurrPoint();
			newx += point.x + _movement->_mx;
			newy += point.y + _movement->_my;
			_stepArray.gotoNextPoint();

			ExCommand *ex = _movement->_currDynamicPhase->getExCommand();

			if (ex) {
				if (ex->_messageKind == 35) {
					ExCommand *newEx = ex->createClone();

					newEx->_excFlags |= 2u;
					newEx->sendMessage();
				}
			}
		}
	}

	point = _movement->getCurrDynamicPhaseXY();
	setOXY(point.x + newx, point.y + newy);

	if ((_movement->_staticsObj2->_staticsId >> 8) & 0x40)
		_flags |= 8;
	else
		_flags &= 0xFFF7;

	_flags |= 1;
	_messageQueueId = messageQueueId;
	_movement->_currDynamicPhase->_countdown = _movement->_currDynamicPhase->_initialCountdown;
	_movement->_counter = 0;
	_counter = _initialCounter;
	_someDynamicPhaseIndex = someDynamicPhaseIndex;

	ExCommand *ex = new ExCommand(_id, 17, 23, 0, 0, movementId, 1, 0, 0, 0);

	ex->_param = _odelay;
	ex->_excFlags = 2;
	ex->postMessage();
}

bool StaticANIObject::startAnimEx(int movid, int parId, int flag1, int flag2) {
	bool res = startAnim(movid, parId, -1);
	if (res)
		_animExFlag = 1;

	_someDynamicPhaseIndex = -1;
	return res;
}

bool StaticANIObject::startAnim(int movementId, int messageQueueId, int dynPhaseIdx) {
	if (_flags & 0x80)
		return false;

	debugC(4, kDebugAnimation, "StaticANIObject::startAnim(%d, %d, %d) (%s [%d]) [%d, %d]", movementId, messageQueueId, dynPhaseIdx, transCyrillic(_objectName), _id, _ox, _oy);

	if (_messageQueueId) {
		updateGlobalMessageQueue(messageQueueId, _id);
		return false;
	}

	Movement *mov = 0;

	for (uint i = 0; i < _movements.size(); i++) {
		if (_movements[i]->_id == movementId) {
			mov = _movements[i];
			break;
		}
	}

	if (!mov) {
		updateGlobalMessageQueue(messageQueueId, _id);
		return false;
	}

	if (mov == _movement) {
		_flags |= 1;
		_messageQueueId = messageQueueId;

		return true;
	}

	int newx = _ox;
	int newy = _oy;
	Common::Point point;

	if (_movement) {
		point = _movement->getCurrDynamicPhaseXY();

		newx -= point.x;
		newy -= point.y;

	} else if (_statics) {
		point = _statics->getSomeXY();

		newx -= point.x;
		newy -= point.y;
	}

	_movement = mov;

	_stepArray.clear();

	if (_flags & 0x40)
		_movement->gotoLastFrame();
	else
		_movement->gotoFirstFrame();

	if (!(_flags & 0x40)) {
		if (!_movement->_currDynamicPhaseIndex) {
			point = _stepArray.getCurrPoint();
			newx += point.x + _movement->_mx;
			newy += point.y + _movement->_my;

			_stepArray.gotoNextPoint();

			ExCommand *ex = _movement->_currDynamicPhase->getExCommand();
			if (ex) {
				if (ex->_messageKind == 35) {
					ExCommand *newex = ex->createClone();
					newex->_excFlags |= 2;
					newex->sendMessage();
				}
			}
		}
	}

	point = _movement->getCurrDynamicPhaseXY();
	setOXY(point.x + newx, point.y + newy);

	if (_movement->_staticsObj2->_staticsId & 0x4000)
		_flags |= 8;
	else
		_flags &= 0xFFF7;

	_flags |= 1;

	_messageQueueId = messageQueueId;
	_movement->_currDynamicPhase->_countdown = _movement->_currDynamicPhase->_initialCountdown;
	_movement->_counter = 0;

	_counter = _initialCounter;
	_someDynamicPhaseIndex = dynPhaseIdx;

	_stepArray.clear();

	ExCommand *newex = new ExCommand(_id, 17, 23, 0, 0, movementId, 1, 0, 0, 0);

	newex->_param = _odelay;
	newex->_excFlags = 2;

	newex->postMessage();

	return true;
}

Common::Point *StaticANIObject::calcStepLen(Common::Point *p) {
	if (_movement) {
		Common::Point point;

		point = _movement->calcSomeXY(0, _movement->_currDynamicPhaseIndex);

		p->x = point.x;
		p->y = point.y;

		int idx = _stepArray.getCurrPointIndex() - _movement->_currDynamicPhaseIndex - 1;

		if (idx >= 0) {
			point = _stepArray.getPoint(idx, _movement->_currDynamicPhaseIndex + 2);
			p->x += point.x;
			p->y += point.y;
		}
	} else {
		p->x = 0;
		p->y = 0;
	}

	return p;
}

Statics::Statics() {
	_staticsId = 0;
	_data = nullptr;
}

Statics::Statics(Statics *src, bool reverse) : DynamicPhase(src, reverse) {
	_staticsId = src->_staticsId;

	if (reverse) {
		_staticsId ^= 0x4000;
		_staticsName = sO_MirroredTo + src->_staticsName;
	} else {
		_staticsName = src->_staticsName;
	}

	_memfilename = src->_memfilename;
}

bool Statics::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Statics::load()");

	DynamicPhase::load(file);

	_staticsId = file.readUint16LE();

	_staticsName = file.readPascalString();
	debugC(6, kDebugXML, "%% <STATICS id=\"%s\" name=\"%s\" %s />",
		g_nmi->gameIdToStr(_staticsId).c_str(), transCyrillic(_staticsName), DynamicPhase::toXML().c_str());

	_picture.load(file);

	return true;
}

void Statics::init() {
	Picture::init();

	if (_staticsId & 0x4000) {
		_bitmap.reset(_bitmap->reverseImage());
	}
}

Common::Point Statics::getSomeXY() const {
	return Common::Point(_someX, _someY);
}

Common::Point Statics::getCenter() const {
	Common::Rect rect(_rect);

	if (_staticsId & 0x4000) {
		const Dims dims = getDimensions();
		rect.moveTo(dims.x - _rect.right, _rect.top);
	}

	return Common::Point(rect.left + _rect.width() / 2,
						 rect.top + _rect.height() / 2);
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
	_field_84 = 0;
	_currDynamicPhase = 0;
	_field_8C = 0;
	_currDynamicPhaseIndex = 0;
	_field_94 = 0;
	_currMovement = 0;
	_counter = 0;
	_counterMax = 83;
}

Movement::~Movement() {
	if (!_currMovement) {
		if (_updateFlag1) {
			_dynamicPhases[0]->freePixelData();
			delete _dynamicPhases.remove_at(0);
		}

		// FIXME: At this point, the last entry in _dynamicPhases is invalid
		for (uint i = 0; i < _dynamicPhases.size() - 1; i++) {
			DynamicPhase *phase = _dynamicPhases[i];
			if (phase != _staticsObj1 && phase != _staticsObj2)
				delete phase;
			else
				_dynamicPhases[i]->freePixelData();
		}
	}
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
	_field_84 = 0;
	_currDynamicPhase = 0;
	_field_8C = 0;
	_currDynamicPhaseIndex = src->_currDynamicPhaseIndex;
	_field_94 = 0;

	_somePoint.x = 0;
	_somePoint.y = 0;

	_currMovement = src;
	_ox = src->_ox;
	_oy = src->_oy;

	initStatics(ani);

	_counterMax = src->_counterMax;
	_counter = src->_counter;
	_field_50 = src->_field_50;

	updateCurrDynamicPhase();
}

Movement::Movement(Movement *src, int *oldIdxs, int newSize, StaticANIObject *ani) : GameObject(src) {
	_lastFrameSpecialFlag = 0;
	_updateFlag1 = 1;
	_staticsObj1 = 0;
	_staticsObj2 = 0;
	_mx = 0;
	_my = 0;
	_m2x = 0;
	_m2y = 0;

	_counter = 0;
	_counterMax = 0;

	_field_78 = 0;
	_field_84 = 0;
	_currDynamicPhase = 0;
	_field_8C = 0;
	_currDynamicPhaseIndex = 0;
	_field_94 = 0;

	_somePoint.x = 0;
	_somePoint.y = 0;

	_field_50 = src->_field_50;
	_flipFlag = src->_flipFlag;
	_currMovement = 0;
	_mx = src->_mx;
	_my = src->_my;
	_m2x = src->_m2x;
	_m2y = src->_m2y;

	if (newSize != -1) {
		if (newSize >= (int)src->_dynamicPhases.size() + 1)
			newSize = src->_dynamicPhases.size() + 1;
	} else {
		newSize = src->_dynamicPhases.size();
	}

	if (!newSize) {
		warning("Movement::Movement: newSize = 0");

		return;
	}

	_framePosOffsets.resize(newSize);

	if (oldIdxs) {
		for (int i = 0; i < newSize - 1; i++, oldIdxs++) {
			if (oldIdxs[i] == -1) {
				_dynamicPhases.push_back(src->_staticsObj1);
			} else {
				src->setDynamicPhaseIndex(oldIdxs[i]);

				_dynamicPhases.push_back(src->_currDynamicPhase);

				_framePosOffsets[i].x = src->_framePosOffsets[oldIdxs[i]].x;
				_framePosOffsets[i].y = src->_framePosOffsets[oldIdxs[i]].y;
			}
		}
		_staticsObj1 = dynamic_cast<Statics *>(_dynamicPhases.front());
		_staticsObj2 = dynamic_cast<Statics *>(_dynamicPhases.back());
	} else {
		for (int i = 0; i < newSize; i++) {
			src->setDynamicPhaseIndex(i);

			if (i < newSize - 1)
				_dynamicPhases.push_back(new DynamicPhase(src->_currDynamicPhase, 0));

			_framePosOffsets[i].x = src->_framePosOffsets[i].x;
			_framePosOffsets[i].y = src->_framePosOffsets[i].y;
		}

		_staticsObj1 = ani->getStaticsById(src->_staticsObj1->_staticsId);
		_staticsObj2 = ani->getStaticsById(src->_staticsObj2->_staticsId);

		_dynamicPhases.push_back(_staticsObj2);

		this->_updateFlag1 = src->_updateFlag1;
	}

	updateCurrDynamicPhase();
	removeFirstPhase();

	_counterMax = src->_counterMax;
	_counter = src->_counter;
}

bool Movement::load(MfcArchive &file) {
	warning("STUB: Movement::load");
	return true;
}

bool Movement::load(MfcArchive &file, StaticANIObject *ani) {
	GameObject::load(file);

	int dynCount = file.readUint16LE();

	if (dynCount != 0xffff || _id == MV_MAN_TURN_LU) {
		_framePosOffsets.resize(dynCount + 2);

		for (int i = 0; i < dynCount; i++) {
			DynamicPhase *ph = new DynamicPhase();
			ph->load(file);

			_dynamicPhases.push_back(ph);

			_framePosOffsets[i].x = ph->_x;
			_framePosOffsets[i].y = ph->_y;
		}

		int staticsid = file.readUint16LE();

		_staticsObj1 = ani->getStaticsById(staticsid);

		if (!_staticsObj1 && (staticsid & 0x4000)) {
			Statics *s = ani->getStaticsById(staticsid ^ 0x4000);
			_staticsObj1 = ani->addReverseStatics(s);
		}

		_mx = file.readSint32LE();
		_my = file.readSint32LE();

		int staticsid2 = file.readUint16LE();

		_staticsObj2 = ani->getStaticsById(staticsid2);

		if (!_staticsObj2 && (staticsid2 & 0x4000)) {
			Statics *s = ani->getStaticsById(staticsid2 ^ 0x4000);
			_staticsObj2 = ani->addReverseStatics(s);
		}

		_m2x = file.readSint32LE();
		_m2y = file.readSint32LE();

		if (_staticsObj2) {
			_dynamicPhases.push_back(_staticsObj2);

			_framePosOffsets[_dynamicPhases.size() - 1].x = _m2x;
			_framePosOffsets[_dynamicPhases.size() - 1].y = _m2y;
		}

		debugC(6, kDebugXML, "%% <MOVEMENT %s staticsId=\"%s\" mX=%d my=%d staticsId2=\"%s\" m2x=%d m2y=%d>",
			GameObject::toXML().c_str(), g_nmi->gameIdToStr(staticsid).c_str(), _mx, _my, g_nmi->gameIdToStr(staticsid2).c_str(), _m2x, _m2y);

		for (int i = 0; i < dynCount; i++)
			debugC(6, kDebugXML, "%% <PHASE %s />", _dynamicPhases[i]->toXML().c_str());

		debugC(6, kDebugXML, "%% </MOVEMENT>");

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

	if (g_nmi->_gameProjectVersion >= 8)
		_field_50 = file.readUint32LE();

	if (g_nmi->_gameProjectVersion < 12)
		_counterMax = 83;
	else
		_counterMax = file.readUint32LE();

	_counter = 0;
	updateCurrDynamicPhase();

	return true;
}

Common::Point Movement::getCurrDynamicPhaseXY() const {
	return Common::Point(_currDynamicPhase->_someX, _currDynamicPhase->_someY);
}

Common::Point Movement::calcSomeXY(int idx, int dynidx) {
	int oldox = _ox;
	int oldoy = _oy;
	int oldidx = _currDynamicPhaseIndex;

	int x = 0;
	int y = 0;

	if (!idx) {
		const Common::Point point = _staticsObj1->getSomeXY();
		int x1 = _mx - point.x;
		int y1 = _my - point.y;

		setDynamicPhaseIndex(0);

		x = _currDynamicPhase->_someX + x1;
		y = _currDynamicPhase->_someY + y1;
	}

	setOXY(x, y);

	while (_currDynamicPhaseIndex != dynidx && gotoNextFrame(0, 0))
		;

	Common::Point p(_ox, _oy);

	setDynamicPhaseIndex(oldidx);
	setOXY(oldox, oldoy);

	return p;
}

void Movement::setAlpha(int alpha) {
	if (_currMovement)
		for (uint i = 0; i < _currMovement->_dynamicPhases.size(); i++) {
			_currMovement->_dynamicPhases[i]->setAlpha(alpha);
		}
	else
		for (uint i = 0; i < _dynamicPhases.size(); i++) {
			_dynamicPhases[i]->setAlpha(alpha);
		}
}

Dims Movement::getDimensionsOfPhase(int phaseIndex) const {
	int idx = phaseIndex;

	if (idx == -1)
		idx = _currDynamicPhaseIndex;

	DynamicPhase *dyn;

	if (_currMovement)
		dyn = _currMovement->_dynamicPhases[idx];
	else
		dyn = _dynamicPhases[idx];

	return dyn->getDimensions();
}

void Movement::initStatics(StaticANIObject *ani) {
	if (!_currMovement)
		return;

	debugC(7, kDebugAnimation, "Movement::initStatics()");

	_staticsObj2 = ani->addReverseStatics(_currMovement->_staticsObj2);
	_staticsObj1 = ani->addReverseStatics(_currMovement->_staticsObj1);

	_mx = _currMovement->_mx;
	_my = _currMovement->_my;

	_currMovement->setDynamicPhaseIndex(_currMovement->_updateFlag1 != 0 ? 1 : 0);

	Common::Point point;

	int x1 = _currMovement->_staticsObj1->getDimensions().x - _mx;

	_mx = x1 - _currMovement->_currDynamicPhase->getDimensions().x;

	_currMovement->setDynamicPhaseIndex(_currMovement->_currDynamicPhaseIndex);

	_m2x = _currMovement->_m2x;
	_m2y = _currMovement->_m2y;
	_currMovement->gotoLastFrame();

	x1 = _currMovement->_staticsObj2->getDimensions().x;
	_m2x = _currMovement->_currDynamicPhase->getDimensions().x - _m2x - x1;
}

void Movement::updateCurrDynamicPhase() {
	debugC(7, kDebugAnimation, "Movement::updateCurrDynamicPhase()");

	if (_currMovement) {
		if (_currMovement->_dynamicPhases.size() == 0 || (uint)_currDynamicPhaseIndex >= _currMovement->_dynamicPhases.size())
			return;

		if (_currMovement->_dynamicPhases[_currDynamicPhaseIndex])
			_currDynamicPhase = _currMovement->_dynamicPhases[_currDynamicPhaseIndex];
	} else {
		if (_dynamicPhases.size() == 0 || (uint)_currDynamicPhaseIndex >= _dynamicPhases.size())
			return;

		if (_dynamicPhases[_currDynamicPhaseIndex])
			_currDynamicPhase = _dynamicPhases[_currDynamicPhaseIndex];
	}
}

int Movement::calcDuration() {
	int res = 0;

	if (_currMovement)
		for (uint i = 0; i < _currMovement->_dynamicPhases.size(); i++) {
			res += _currMovement->_dynamicPhases[i]->_initialCountdown + 1;
		}
	else
		for (uint i = 0; i < _dynamicPhases.size(); i++) {
			res += _dynamicPhases[i]->_initialCountdown + 1;
		}

	return res;
}

int Movement::countPhasesWithFlag(int maxidx, int flag) {
	int res = 0;
	int sz;

	if (_currMovement)
		sz = _currMovement->_dynamicPhases.size();
	else
		sz = _dynamicPhases.size();

	if (maxidx < 0)
		maxidx = sz;

	for (int i = 0; i < maxidx && i < sz; i++)
		if (getDynamicPhaseByIndex(i)->_dynFlags & flag)
			res++;

	return res;
}

void Movement::setDynamicPhaseIndex(int index) {
	debugC(7, kDebugAnimation, "Movement::setDynamicPhaseIndex(%d)", index);
	while (_currDynamicPhaseIndex < index)
		gotoNextFrame(0, 0);

	while (_currDynamicPhaseIndex > index)
		gotoPrevFrame();
}

DynamicPhase *Movement::getDynamicPhaseByIndex(int idx) {
	debugC(7, kDebugAnimation, "Movement::updateCurrDynamicPhase()");

	if (_currMovement) {
		if (_currMovement->_dynamicPhases.size() == 0 || (uint)idx >= _currMovement->_dynamicPhases.size())
			return 0;

		return _currMovement->_dynamicPhases[idx];
	} else {
		if (_dynamicPhases.size() == 0 || (uint)idx >= _dynamicPhases.size())
			return 0;

		return _dynamicPhases[idx];
	}
}

void Movement::loadPixelData() {
	Movement *mov = this;
	while (mov->_currMovement)
		mov = mov->_currMovement;

	for (uint i = 0; i < mov->_dynamicPhases.size(); i++) {
		if (mov->_dynamicPhases[i] != mov->_staticsObj2 || !(mov->_staticsObj2->_staticsId & 0x4000))
			mov->_dynamicPhases[i]->getPixelData();
	}

	if (!(mov->_staticsObj1->_staticsId & 0x4000))
		mov->_staticsObj1->getPixelData();
}

void Movement::freePixelData() {
	if (!_currMovement)
		for (uint i = 0; i < _dynamicPhases.size(); i++)
			_dynamicPhases[i]->freePixelData();

	if (_staticsObj1)
		_staticsObj1->freePixelData();
}

void Movement::removeFirstPhase() {
	if (_updateFlag1) {
		if (!_currDynamicPhaseIndex)
			gotoNextFrame(0, 0);

		if (!_currMovement) {
			delete _dynamicPhases.remove_at(0);

			for (uint i = 0; i < _dynamicPhases.size(); i++) {
				_framePosOffsets[i] = _framePosOffsets[i + 1];
			}
			_framePosOffsets.pop_back();
		}
		_currDynamicPhaseIndex--;
	}

	updateCurrDynamicPhase();
	_updateFlag1 = 0;
}

bool Movement::gotoNextFrame(void (*callback1)(int, Common::Point *point, int, int), void (*callback2)(int *)) {
	debugC(8, kDebugAnimation, "Movement::gotoNextFrame()");

	if (!callback2) {
		if (_currMovement) {
			if ((uint)_currDynamicPhaseIndex == _currMovement->_dynamicPhases.size() - 1
				&& !(_currMovement->_dynamicPhases.back()->_countdown)) {
				return false;
			}
		} else if ((uint)_currDynamicPhaseIndex == _dynamicPhases.size() - 1
					&& !(_dynamicPhases.back()->_countdown)) {
			return false;
		}
	}

	if (_currDynamicPhase->_countdown) {
		_currDynamicPhase->_countdown--;
		return true;
	}

	Common::Point point = getCurrDynamicPhaseXY();
	_ox -= point.x;
	_oy -= point.y;

	int deltax = 0;

	if (_currMovement)
		deltax = _currMovement->getDimensionsOfPhase(_currDynamicPhaseIndex).x;

	int oldDynIndex = _currDynamicPhaseIndex;

	if (callback2)
		callback2(&_currDynamicPhaseIndex);
	else
		_currDynamicPhaseIndex++;

	bool result = true;

	if (_currMovement) {
		if (_currMovement->_dynamicPhases.size() <= (uint)_currDynamicPhaseIndex) {
			_currDynamicPhaseIndex = _currMovement->_dynamicPhases.size() - 1;
			result = (callback2 == 0);
		}
		if (_currDynamicPhaseIndex < 0) {
			_currDynamicPhaseIndex = 0;
			result = false;
		}
		if (_currMovement->_framePosOffsets.size()) {
			if (callback1) {
				point = _currMovement->_framePosOffsets[_currDynamicPhaseIndex];
				callback1(_currDynamicPhaseIndex, &point, _ox, _oy);

				_ox += deltax - point.x;
				_oy += point.y;

				_ox -= _currMovement->getDimensionsOfPhase(_currDynamicPhaseIndex).x;
			} else if (oldDynIndex >= _currDynamicPhaseIndex) {
				while (oldDynIndex > _currDynamicPhaseIndex) {
					_ox += deltax;
					deltax = _currMovement->getDimensionsOfPhase(oldDynIndex).x;

					_ox += _currMovement->_framePosOffsets[oldDynIndex].x;
					_oy -= _currMovement->_framePosOffsets[oldDynIndex].y;
					oldDynIndex--;

					_ox -= _currMovement->getDimensionsOfPhase(oldDynIndex).x;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _currDynamicPhaseIndex; i++) {
					_ox += deltax;
					deltax = _currMovement->getDimensionsOfPhase(i).x;
					_ox -= _currMovement->_framePosOffsets[i].x;
					_oy += _currMovement->_framePosOffsets[i].y;
					_ox -= _currMovement->getDimensionsOfPhase(i).x;
				}
			}
		}
	} else {
		if (_dynamicPhases.size() <= (uint)_currDynamicPhaseIndex) {
			_currDynamicPhaseIndex = _dynamicPhases.size() - 1;
			result = (callback2 == 0);
		}
		if (_currDynamicPhaseIndex < 0) {
			_currDynamicPhaseIndex = 0;
			result = false;
		}

		if (_framePosOffsets.size()) {
			if (callback1) {
				point.x = _framePosOffsets[_currDynamicPhaseIndex].x;
				point.y = _framePosOffsets[_currDynamicPhaseIndex].y;

				callback1(_currDynamicPhaseIndex, &point, _ox, _oy);
				_ox += point.x;
				_oy += point.y;
			} else if (oldDynIndex >= _currDynamicPhaseIndex) {
				for (int i = oldDynIndex; i > _currDynamicPhaseIndex; i--) {
					_ox -= _framePosOffsets[i].x;
					_oy -= _framePosOffsets[i].y;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _currDynamicPhaseIndex; i++) {
					_ox += _framePosOffsets[i].x;
					_oy += _framePosOffsets[i].y;
				}
			}
		}
	}

	updateCurrDynamicPhase();
	point = getCurrDynamicPhaseXY();
	_ox += point.x;
	_oy += point.y;

	_currDynamicPhase->_countdown = _currDynamicPhase->_initialCountdown;

	return result;
}

bool Movement::gotoPrevFrame() {
	debugC(8, kDebugAnimation, "Movement::gotoPrevFrame()");

	if (!_currDynamicPhaseIndex) {
		gotoLastFrame();
		return false;
	}

	Common::Point point = getCurrDynamicPhaseXY();

	_ox -= point.x;
	_oy -= point.y;

	if (_currMovement) {
		if (_currMovement->_framePosOffsets.size()) {
			_ox += _currMovement->getDimensionsOfPhase(_currDynamicPhaseIndex).x;
			_ox += _currMovement->_framePosOffsets[_currDynamicPhaseIndex].x;
			_oy -= _currMovement->_framePosOffsets[_currDynamicPhaseIndex].y;
		}

		_currDynamicPhaseIndex--;
		if (_currDynamicPhaseIndex < 0)
			_currDynamicPhaseIndex = _currMovement->_dynamicPhases.size() - 1;

		_ox -= _currMovement->getDimensionsOfPhase(_currDynamicPhaseIndex).x;
	} else {
		if (_framePosOffsets.size()) {
			_ox -= _framePosOffsets[_currDynamicPhaseIndex].x;
			_oy -= _framePosOffsets[_currDynamicPhaseIndex].y;
		}

		_currDynamicPhaseIndex--;
		if (_currDynamicPhaseIndex < 0)
			_currDynamicPhaseIndex = _dynamicPhases.size() - 1;
	}

	updateCurrDynamicPhase();
	point = getCurrDynamicPhaseXY();

	_ox += point.x;
	_oy += point.y;

	return true;
}

void Movement::gotoFirstFrame() {
	while (_currDynamicPhaseIndex)
			gotoPrevFrame();
}

void Movement::gotoLastFrame() {
	if (_currMovement) {
		if ((uint)_currDynamicPhaseIndex != _currMovement->_dynamicPhases.size() - 1) {
			do {
				gotoNextFrame(0, 0);
			} while ((uint)_currDynamicPhaseIndex != _currMovement->_dynamicPhases.size() - 1);
		}
	} else {
		if ((uint)_currDynamicPhaseIndex != _dynamicPhases.size() - 1) {
			do {
				gotoNextFrame(0, 0);
			} while ((uint)_currDynamicPhaseIndex != _dynamicPhases.size() - 1);
		}
	}
}

Common::Point Movement::getCenter() const {
	Common::Rect rect(_currDynamicPhase->_rect);

	if (_currMovement) {
		const Dims dims = _currMovement->getDimensionsOfPhase(_currDynamicPhaseIndex);
		rect.moveTo(dims.x - _currDynamicPhase->_rect.right, _currDynamicPhase->_rect.top);
	}

	return Common::Point(rect.left + _currDynamicPhase->_rect.width() / 2,
						 rect.top + _currDynamicPhase->_rect.height() / 2);
}

DynamicPhase::DynamicPhase() {
	_someX = 0;
	_field_7C = 0;
	_field_7E = 0;
	_dynFlags = 0;
	_someY = 0;
	_data = nullptr;
}

DynamicPhase::DynamicPhase(DynamicPhase *src, bool reverse) {
	_field_7C = src->_field_7C;
	_field_7E = 0;

	debugC(1, kDebugAnimation, "DynamicPhase::DynamicPhase(src, %d)", reverse);

	if (reverse) {
		if (!src->_bitmap)
			src->init();

		_bitmap.reset(src->_bitmap->reverseImage());
		_dataSize = src->_dataSize;

		if (g_nmi->_currArchive) {
			_mfield_14 = 0;
			_libHandle = g_nmi->_currArchive;
		}

		_mflags |= 1;

		_someX = src->_someX;
		_someY = src->_someY;
	} else {
		_mfield_14 = src->_mfield_14;
		_mfield_8 = src->_mfield_8;
		_mflags = src->_mflags;

		_memfilename = src->_memfilename;
		_dataSize = src->_dataSize;
		_mfield_10 = src->_mfield_10;
		_libHandle = src->_libHandle;

		if (src->_bitmap) {
			_field_54 = 1;
			_bitmap.reset(src->_bitmap->reverseImage(false));
		}

		_someX = src->_someX;
		_someY = src->_someY;
	}

	_rect = src->_rect;

	_width = src->_width;
	_height = src->_height;
	_field_7C = src->_field_7C;

	if (src->getExCommand())
		_exCommand.reset(src->getExCommand()->createClone());
	else
		_exCommand.reset();

	_initialCountdown = src->_initialCountdown;
	_field_6A = src->_field_6A;
	_dynFlags = src->_dynFlags;

	debug(8, "DynamicPhase::DynamicPhase(): pal: %p, pal size: %d",
		(const void *)&src->getPaletteData(), src->getPaletteData().size);
	setPaletteData(src->getPaletteData());

	copyMemoryObject2(*src);
}

Common::String DynamicPhase::toXML() {
	return Common::String::format("f7c=%d left=%d top=%d right=%d bottom=%d sX=%d sY=%d dynFlags=%d %s",
			_field_7C, _rect.left, _rect.top, _rect.right, _rect.bottom, _someX, _someY, _dynFlags,
			StaticPhase::toXML().c_str());
}

bool DynamicPhase::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "DynamicPhase::load()");

	StaticPhase::load(file);

	_field_7C = file.readUint16LE();
	_rect.left = file.readSint32LE();
	_rect.top = file.readSint32LE();
	_rect.right = file.readSint32LE();
	_rect.bottom = file.readSint32LE();

	assert(g_nmi->_gameProjectVersion >= 1);

	_someX = file.readSint32LE();
	_someY = file.readSint32LE();

	assert(g_nmi->_gameProjectVersion >= 12);

	_dynFlags = file.readUint32LE();

	return true;
}

StaticPhase::StaticPhase() {
	_field_6A = 1;
	_initialCountdown = 0;
	_countdown = 0;
	_field_68 = 0;
}

Common::String StaticPhase::toXML() {
	return Common::String::format("countdown=%d f6a=%d", _initialCountdown, _field_6A);
}

bool StaticPhase::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "StaticPhase::load()");

	Picture::load(file);

	_initialCountdown = file.readUint16LE();
	_field_6A = file.readUint16LE();

	assert(g_nmi->_gameProjectVersion >= 12);

	_exCommand.reset(file.readClass<ExCommand>());

	return true;
}

} // End of namespace NGI
