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

AniObject::AniObject() {
	_shadowsOn = 1;
	_encodedRotation = 0;
	_field_34 = 1;
	_initialCounter = 0;
	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_movement = nullptr;
	_statics = nullptr;
	_flags = 0;
	_callback1 = nullptr; // Really NULL
	_callback2 = nullptr; // Really NULL
	_sceneId = -1;
	_targetFrame = -1;

	_field_32 = 0;
	_field_96 = 0;
	_messageNum = 0;
	_objtype = kObjTypeStaticANIObject;
}

AniObject::~AniObject() {
	Common::for_each(_staticsList.begin(), _staticsList.end(), Common::DefaultDeleter<Statics>());
	Common::for_each(_movements.begin(), _movements.end(), Common::DefaultDeleter<Movement>());
	g_nmi->_aniHandler->detachAllObjects();
}

AniObject::AniObject(AniObject *src) : GameObject(src) {
	_shadowsOn = src->_shadowsOn;
	_encodedRotation = src->_encodedRotation;
	_field_34 = 1;
	_initialCounter = 0;

	_field_32 = 0;
	_field_96 = 0;
	_messageNum = 0;

	_messageQueueId = 0;
	_animExFlag = 0;
	_counter = 0;
	_targetFrame = -1;
	_sceneId = src->_sceneId;
	_callback1 = src->_callback1;
	_callback2 = src->_callback2;
	_objtype = kObjTypeStaticANIObject;

	for (uint i = 0; i < src->_staticsList.size(); i++)
		_staticsList.push_back(new Statics(src->_staticsList[i], false));

	_movement = nullptr;
	_statics = nullptr;

	for (uint i = 0; i < src->_movements.size(); i++) {
		Movement *newmov;

		if (src->_movements[i]->_currMovement) {
			// This is weird code. Logically it should be
			// newmov = new Movement(src->getMovement(src->getMovementIdById(src->_movements[i]->_id)), this);
			newmov = new Movement(getMovement(src->getMovementIdById(src->_movements[i]->_id)), this);
			newmov->_id = src->_movements[i]->_id;
		} else {
			newmov = new Movement(src->_movements[i], nullptr, -1, this);
		}

		_movements.push_back(newmov);
	}
}

bool AniObject::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "AniObject::load()");

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

		Common::Path movname = genFileName(_id, movNum, "mov");

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

	setPos(pt.x, pt.y);

	return true;
}

void AniObject::setPos(int x, int y) {
	_x = x;
	_y = y;

	if (_movement)
		_movement->setPos(x, y);
}

void AniObject::clearFlags() {
	_flags = 0;

	stopQueue();
	_messageQueueId = 0;
	_movement = nullptr;
	_statics = nullptr;
	_animExFlag = 0;
	_counter = 0;
	_messageNum = 0;
	_stepArray.clear();
}

void AniObject::setFlags40(bool state) {
	if (state) {
		_flags |= 0x40;
	} else {
		if (_flags & 0x40)
			_flags ^= 0x40;
	}
}

void AniObject::stopQueue() {
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

bool AniObject::setQueue(MessageQueue *mq) {
	if (_flags & 0x80)
		return false;

	if (isIdle()) {
		stopQueue();
		_messageQueueId = 0;
		_messageNum = 0;

		if (_flags & 2)
			_flags ^= 2;

		if (mq) {
			_animExFlag = 0;
			if (_movement)
				_messageQueueId = mq->_id;
			else
				mq->next();
		} else {
			_messageQueueId = 0;
		}
	}

	return true;
}

void AniObject::restartMessageQueue(MessageQueue *mq) {
	Command *ex = mq->getCommandByIndex(0);
	if (ex) {
		while (ex->_cmd != 1 || ex->_parentId != _id) {
			ex->_parId = 0;
			ex->_excFlags |= 2;
			ex->handleMessage();

			mq->removeCommand(0, 0);

			ex = mq->getCommandByIndex(0);

			if (!ex)
				return;
		}

		if (ex) {
			startMovement(ex->_messageNum, mq->_id, -1);
			mq->removeCommand(0, 1);
		}
	}
}

MessageQueue *AniObject::getMessageQueue() {
	if (this->_messageQueueId <= 0)
		return nullptr;

	return g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId);
}

bool AniObject::trySetMessageQueue(int msgNum, int qId) {
	if (_messageQueueId || !msgNum) {
		updateGlobalMessageQueue(qId, _id);
		return false;
	}

	_flags |= 2;

	_messageNum = msgNum;
	_messageQueueId = qId;

	return true;
}

void AniObject::startMQIfIdle(int qId, int flag) {
	MessageQueue *msg = g_nmi->_currentScene->findMessageQueue(qId);

	if (msg && isIdle() && !(_flags & 0x100)) {
		MessageQueue *mq = new MessageQueue(msg, 0, 0);

		mq->setFlags(mq->getFlags() | flag);

		Command *ex = mq->getCommandByIndex(0);

		if (ex) {
			while (ex->_cmd != 1 || ex->_parentId != _id) {
				ex->_parId = 0;
				ex->_excFlags |= 2;
				ex->handleMessage();

				mq->removeCommand(0, 0);

				ex = mq->getCommandByIndex(0);

				if (!ex)
					return;
			}

			if (ex) {
				startMovement(ex->_messageNum, mq->_id, -1);
				mq->removeCommand(0, 1);
			}
		}
	}
}

bool AniObject::isIdle() {
	assert(_objtype == kObjTypeStaticANIObject);

	if (_messageQueueId) {
		MessageQueue *m = g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId);

		if (m && m->getFlags() & 1)
			return false;
	}

	return true;
}

Statics *AniObject::getStatic(int itemId) {
	for (uint i = 0; i < _staticsList.size(); i++)
		if (_staticsList[i]->_staticsId == itemId)
			return _staticsList[i];

	return nullptr;
}

Statics *AniObject::getStaticsByName(const Common::String &name) {
	for (uint i = 0; i < _staticsList.size(); i++)
		if (_staticsList[i]->_staticsName == name)
			return _staticsList[i];

	return nullptr;
}

Movement *AniObject::getMovement(int itemId) {
	for (uint i = 0; i < _movements.size(); i++)
		if (_movements[i]->_id == itemId)
			return _movements[i];

	return nullptr;
}

int AniObject::getMovementIdById(int itemId) const {
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

Movement *AniObject::getMovementByName(const Common::String &name) {
	for (uint i = 0; i < _movements.size(); i++)
		if (_movements[i]->_title == name)
			return _movements[i];

	return nullptr;
}

bool AniObject::hit(int x, int y) {
	return getPixelAtPos(x, y, nullptr, true);
}

bool AniObject::getPixelAtPos(int x, int y, uint32 *pixel, bool hitOnly) {
	bool res = false;
	Texture *pic;

	if (_movement)
		pic = _movement->_curPhase;
	else
		pic = _statics;

	if (!pic)
		return false;

	int ongoing;
	int xani, yani;
	int oxani, oyani;

	if (_movement)
		ongoing = _movement->_currMovement != nullptr;
	else
		ongoing = _statics->_staticsId & 0x4000;

	if (_movement) {
		const Common::Point point = _movement->getCurrDynamicPhaseXY();
		xani = point.x;
		yani = point.y;
		oxani = _movement->_x;
		oyani = _movement->_y;
	} else {
		const Common::Point point = _statics->getSomeXY();
		xani = point.x;
		yani = point.y;
		oxani = _x;
		oyani = _y;
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
		return pic->hit(xtarget, ytarget);

	if (pic->hit(xtarget, ytarget)) {
		*pixel = pic->getColor(xtarget, ytarget);

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

	int x = _x - point.x;
	int y = _y - point.y;

	if (_curPhase->getPaletteData().size)
		g_nmi->_globalPalette = &_curPhase->getPaletteData();

	Common::ScopedPtr<Bitmap> bmp;
	if (_currMovement) {
		bmp.reset(_curPhase->getPixelData()->reverseImage());
	} else {
		bmp.reset(_curPhase->getPixelData()->reverseImage(false));
	}

	if (flipFlag) {
		Bitmap *flipped = bmp->flipVertical();
		flipped->drawShaded(1, x, y + 30 + _curPhase->_rect.bottom, _curPhase->getPaletteData(), _curPhase->getAlpha());
		delete flipped;
	} else if (angle) {
		bmp->drawRotated(x, y, angle, _curPhase->getPaletteData(), _curPhase->getAlpha());
	} else {
		bmp->putDib(x, y, _curPhase->getPaletteData(), _curPhase->getAlpha());
	}

	if (_curPhase->_rect.top) {
		if (!_curPhase->getConvertedBitmap()) {
			//v12 = Picture_getPixelData(v5);
			//v13 = Bitmap_convertTo16Bit565(v12, (unsigned int *)&_curPhase->rect);
			//_curPhase->convertedBitmap = v13;
		}

		if (_curPhase->getConvertedBitmap()) {
			if (_currMovement) {
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 1, LOBYTE(_curPhase->rect.top));
				bmp.reset(_curPhase->getConvertedBitmap()->reverseImage());
				bmp->putDib(x, y, _curPhase->getPaletteData(), _curPhase->getAlpha());
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 0, 255);
			} else {
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 1, LOBYTE(_curPhase->rect.top));
				bmp.reset(_curPhase->getConvertedBitmap()->reverseImage(false));
				bmp->putDib(x, y, _curPhase->getPaletteData(), _curPhase->getAlpha());
				//vrtSetAlphaBlendMode(g_vrtDrawHandle, 0, 255);
			}
		}
	}
}

void AniObject::loadMovements() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->load();
}

void AniObject::unloadMovements() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->unload();
}

Statics *AniObject::addReverseStatics(Statics *st) {
	assert(st);
	Statics *res = getStatic(st->_staticsId ^ 0x4000);

	if (!res) {
		res = new Statics(st, true);
		_staticsList.push_back(res);
	}

	return res;
}

void AniObject::render() {
	if ((_flags & 4) == 0)
		return;

	debugC(6, kDebugDrawing, "AniObject::render() (%s) [%d] [%d, %d]", transCyrillic(_title), _id, _x, _y);

	if (_shadowsOn && g_nmi->_currentScene && g_nmi->_currentScene->_shadows
		&& (getFullSize().x != 1 || getFullSize().y != 1)) {

		Phase *dyn;

		if (!_movement || _flags & 0x20)
			dyn = _statics;
		else
			dyn = _movement->_curPhase;

		if (!dyn) {
			warning("HACK: AniObject::render(): dyn is missing");
			return;
		}

		if (dyn->getDynFlags() & 4) {
			const Common::Rect &rect = dyn->_rect;

			Phase *shd = g_nmi->_currentScene->_shadows->findSize(rect.width(), rect.height());
			if (shd) {
				const Dims dims = shd->getDimensions();
				int midx = _x - dims.x / 2 - dyn->_someX;
				int midy = _y - dims.y / 2 - dyn->_someY + rect.bottom - 3;
				int shdw =  dims.y;

				int px;
				if (!_movement || (_flags & 0x20))
					px = _statics->getCenter().x;
				else
					px = _movement->getCenter().x;

				if (_shadowsOn != 1)
					midy = _shadowsOn - shdw / 2;

				shd->render(px + midx, midy, 0, 0);
			}
		}
	}

	int angle = 0;
	if (_encodedRotation & 0xC000) {
		if (_encodedRotation & 0x8000)
			angle = -(_encodedRotation ^ 0x8000);
		else
			angle = _encodedRotation ^ 0x4000;
	}

	if (!_movement || (_flags & 0x20)) {
		const Common::Point point = _statics->getSomeXY();
		_statics->_x = _x - point.x;
		_statics->_y = _y - point.y;
		_statics->render(_statics->_x, _statics->_y, 0, angle);
	} else {
		_movement->draw(0, angle);
	}
}

void AniObject::draw2() {
	debugC(6, kDebugDrawing, "StatciANIObject::draw2(): id: (%s) %d [%d, %d]", transCyrillic(_title), _id, _x, _y);

	if ((_flags & 4) && (_flags & 0x10)) {
		if (_movement) {
			_movement->draw(1, 0);
		} else {
			const Common::Point point = _statics->getSomeXY();
			_statics->render(_x - point.x, _y - point.y, 1, 0);
		}
	}
}

MovTable *AniObject::countMovements() {
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

void AniObject::setSpeed(int speed) {
	GameVar *var = g_nmi->getGameLoaderGameVar()->getSubVarByName(getName())->getSubVarByName("SpeedUp");

	if (!var)
		return;

	for (var = var->_subVars; var; var = var->_nextVarObj) {
		Movement *mov = getMovement(var->_value.intValue);

		if (mov) {
			if (speed) {
				if (mov->_loopDelay == 83)
					mov->_loopDelay = 41;
			} else if (mov->_loopDelay == 41) {
				mov->_loopDelay = 83;
			}
		}
	}

}

void AniObject::setAlpha(int alpha) {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->setAlpha(alpha);

	for (uint i = 0; i < _staticsList.size(); i++)
		_staticsList[i]->setAlpha(alpha);
}

void AniObject::initMovements() {
	for (uint i = 0; i < _movements.size(); i++)
		_movements[i]->removeFirstPhase();
}

void AniObject::preloadMovements(MovTable *mt) {
	if (mt) {
		for (uint i = 0; i < _movements.size(); i++) {
			Movement *mov = _movements[i];

			if ((*mt)[i] == 1)
				mov->load();
			else if ((*mt)[i] == 2)
				mov->unload();
		}
	}
}

Dims AniObject::getFullSize() const {
	Texture *pic;

	if (_movement)
		pic = _movement->_curPhase;
	else
		pic = _statics;

	if (pic) {
		return pic->getDimensions();
	}

	return Dims();
}

Common::Point AniObject::getSomeXY() const {
	if (_movement) {
		return _movement->getCurrDynamicPhaseXY();
	}

	if (_statics)
		return _statics->getSomeXY();

	error("No someXY found");
}

bool AniObject::update(int counterdiff) {
	int mqid;

	debugC(6, kDebugAnimation, "AniObject::update() (%s) [%d] [%d, %d] fl: %x", transCyrillic(_title), _id, _x, _y, _flags);

	if (_flags & 2) {
		_messageNum--;
		if (_messageNum)
			return false;

		mqid = _messageQueueId;
		_messageQueueId = 0;
		_flags ^= 2;

		updateGlobalMessageQueue(mqid, _id);
		return true;
	}

	Command *ex, *newex;

	if (_movement) {
		_movement->_counter += counterdiff;

		const bool android = g_nmi->getPlatform() == Common::kPlatformAndroid;
		if (android) {
			const Phase &phase = *_movement->_curPhase;
			const int delay = _movement->_loopDelay * (1 + phase._delayRelative) + phase._delayMs;
			if (_movement->_counter <= delay)
				return false;
		} else if (_movement->_counter < _movement->_loopDelay) {
			return false;
		}

		_movement->_counter = 0;

		if (_flags & 1) {
			if (_counter) {
				_counter--;

				return false;
			}

			Phase *dyn = _movement->_curPhase;
			if (!android && dyn->_delay == dyn->_countdown) {

				ex = dyn->getCommand();
				if (ex && ex->_cmd != 35) {
					newex = ex->createClone();
					newex->_excFlags |= 2;
					if (newex->_cmd == 17) {
						newex->_parentId = _id;
						newex->_param = _copyId;
					}
					newex->sendMessage();

					if (!_movement)
						return true;
				}
			}

			if (dyn->_delay == dyn->_countdown && dyn->_messageNum != 0) {
				newex = new Command(_id, 17, dyn->_messageNum, 0, 0, 0, 1, 0, 0, 0);
				newex->_excFlags = 2;
				newex->_param = _copyId;
				newex->sendMessage();

				if (!_movement)
					return true;
			}

			// Android ends the movement as soon as the next statics takes over.
			if (!_movement->setNext(_callback1, _callback2, android && (_flags & 8)) || (android && _movement->_curPhase == _movement->_nextStatic)) {
				stopMovement();
			} else {
				setPos(_movement->_x, _movement->_y);
				_counter = _initialCounter;

				if (android) {
					_movement->_curPhase->enter(_id);
				} else if (dyn->_delay == dyn->_countdown) {
					ex = dyn->getCommand();
					if (ex) {
						if (ex->_cmd == 35) {
							newex = ex->createClone();
							newex->_excFlags |= 2;
							newex->sendMessage();
						}
					}
				}
			}
			if (!_movement)
				return true;

			const Common::Point point = _stepArray.getCurrPoint();
			setPos(point.x + _x, point.y + _y);
			_stepArray.gotoNextPoint();
			if (_targetFrame == _movement->_curFrame)
				adjustSomeXY();
		} else if (_flags & 0x20) {
			_flags ^= 0x20;
			_flags |= 1;

			_movement->gotoFirstFrame();

			const Common::Point point = _movement->getCurrDynamicPhaseXY();
			const Common::Point pointS = _statics->getSomeXY();
			_movement->setPos(_x + point.x + _movement->_prevStepX - pointS.x,
							  _y + point.y + _movement->_prevStepY - pointS.y);
		}
		return true;
	}

	if (_statics && _messageQueueId) {
		if (_statics->_countdown) {
			_statics->_countdown--;
			return false;
		}
		mqid = _messageQueueId;
		_messageQueueId = 0;
		updateGlobalMessageQueue(mqid, _id);
		return true;
	}
	return false;
}

void AniObject::updateStepPos() {
	Common::Point point;

	int ox = _movement->_x;
	int oy = _movement->_y;

	point = _movement->getMovementSize(1, _targetFrame);
	int x = point.x;
	int y = point.y;

	point = _stepArray.getPoint(-1, _stepArray.getPointsCount());
	x += point.x;
	y += point.y;

	_statics = _movement->_nextStatic;
	_movement = nullptr;

	setPos(ox + x, oy + y);
}

Common::Point *AniObject::calcNextStep(Common::Point *pRes) {
	if (!_movement) {
		pRes->x = 0;
		pRes->y = 0;

		return pRes;
	}

	Common::Point point = _movement->getMovementSize(1, _targetFrame);

	int resX = point.x;
	int resY = point.y;

	int pointN, offset;

	if (_targetFrame <= 0) {
		pointN = _stepArray.getCurrPointIndex();
		offset = _stepArray.getPointsCount() - _stepArray.getCurrPointIndex();
	} else {
		pointN = _stepArray.getCurrPointIndex();
		offset = 1 - _movement->_curFrame + _targetFrame;
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

void AniObject::stopMovement() {
	debugC(2, kDebugAnimation, "AniObject::stopMovement()");

	if (!(_flags & 1))
		return;

	_flags ^= 1;

	int oid = 0;
	int oldmqid = _messageQueueId;
	Common::Point point;

	if (_movement) {
		setPos(_movement->_x, _movement->_y);

		if (_flags & 0x40) {
			if (!_movement->_currMovement) {
				if (_movement->_curFrame)
					goto L11;
L8:
				_statics = _movement->_prevStatic;
				point = _movement->getCurrDynamicPhaseXY();
				_x -= point.x;
				_y -= point.y;

				_x -= _movement->_prevStepX;
				_y -= _movement->_prevStepY;

				point = _statics->getSomeXY();
				if (_movement->_currMovement) {
					_y += point.y;
					_x -= point.x;
					_x += _statics->getDimensions().x;
				} else {
					_x += point.x;
					_y += point.y;
				}
				goto L12;
			}
			if (!_movement->_curFrame)
				goto L8;
		}
L11:
		_statics = _movement->_nextStatic;
		if (_movement->_mirrorNext)
			_flags ^= 8;
L12:
		point = _statics->getSomeXY();

		_statics->_x = _x - point.x;
		_statics->_y = _y - point.y;
		oid = _movement->_id;
		if (g_nmi->getPlatform() == Common::kPlatformAndroid) {
			_movement->unload();
			for (int i = 0; i < g_nmi->_currSoundListCount; ++i)
				g_nmi->_currSoundList1[i]->stopByCreator(_id);
		}
		_movement = nullptr;

		Command *ex = new Command(_id, 17, 24, 0, 0, 0, 1, 0, 0, 0);
		ex->_param = _copyId;
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

void AniObject::adjustSomeXY() {
	if (_movement) {
		Common::Point point;

		point = _movement->getMovementSize(0, -1);

		int diff = abs(point.y) - abs(point.x);

		point = _movement->getMovementSize(1, -1);

		if (diff > 0)
			_x += point.x;
		else
			_y += point.y;

		_statics = _movement->_nextStatic;
		_movement = nullptr;
		_targetFrame = -1;
	}
}

MessageQueue *AniObject::moveToStatic(int msgNum) {
	g_nmi->_aniHandler->attachObject(_id);

	MessageQueue *mq = g_nmi->_aniHandler->makeQueue(this, msgNum, 0, nullptr, nullptr);

	if (!mq)
		return nullptr;

	if (mq->getCount() <= 0) {
		g_nmi->_globalMessageQueueList->addMessageQueue(mq);

		if (_flags & 1)
			_messageQueueId = mq->_id;
	} else {
		if (!setQueue(mq)) {
			delete mq;

			return nullptr;
		}

		g_nmi->_globalMessageQueueList->addMessageQueue(mq);
	}

	return mq;
}

void AniObject::setStatic(int objId) {
	_animExFlag = 0;

	stopQueue();

	if (_movement || _statics) {
		g_nmi->_aniHandler->attachObject(_id);
		g_nmi->_aniHandler->putObjectToStatic(this, objId);
	} else {
		_statics = getStatic(objId);
	}

	if (_messageQueueId) {
		if (g_nmi->_globalMessageQueueList->getMessageQueueById(_messageQueueId))
			g_nmi->_globalMessageQueueList->deleteQueueById(_messageQueueId);

		_messageQueueId = 0;
	}
}

void AniObject::hide() {
	if (!_messageQueueId) {
		if (_flags & 4)
			_flags ^= 4;
	}
}

void AniObject::show1(int x, int y, int movId, int mqId) {
	debugC(6, kDebugAnimation, "AniObject::show1(%d, %d, %d, %d)", x, y, movId, mqId);

	if (_messageQueueId)
		return;

	if (movId == -1) {
		_flags |= 4u;
		if (x != -1 && y != -1) {
			setPos(x, y);
		}

		return;
	}

	Movement *mov = getMovement(movId);
	if (!mov)
		return;

	if (x != -1 && y != -1) {
		setPos(x, y);
	}

	_statics = mov->_prevStatic;

	const Common::Point point = mov->_prevStatic->getSomeXY();
	_statics->_x = x - point.x;
	_statics->_y = y - point.y;

	_statics->_countdown = _statics->_delay;

	_flags |= 4;
	_x = x;
	_y = y;
	_movement = nullptr;

	if (mov->_currMovement)
		_flags |= 8;
	else if (_flags & 8)
		_flags ^= 8;

	if (_flags & 1)
		_flags ^= 1;

	_messageQueueId = mqId;
}

void AniObject::show2(int x, int y, int movementId, int mqId) {
	if (movementId == -1) {
		_flags |= 4u;
		return;
	}

	if (!_messageQueueId) {
		_messageQueueId = mqId;

		Movement *mov = getMovement(movementId);

		if (mov) {
			_statics = mov->_prevStatic;
			_movement = mov;
			mov->setLast();
			mov->setPos(x, y);
			mov->gotoFirstFrame();

			Common::Point point = mov->getCurrDynamicPhaseXY();
			_statics->_x = mov->_x - point.x - mov->_prevStepX;
			_statics->_y = mov->_y - point.y - mov->_prevStepY;

			point = _statics->getSomeXY();
			_flags |= 4;
			_x = _statics->_x + point.x;
			_y = _statics->_y + point.y;

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

void AniObject::playIdle() {
	if (isIdle())
		adjustSomeXY();
}

void AniObject::startSliding(int movementId, int messageQueueId, int x, int y, const PointList &points, int someDynamicPhaseIndex) {
	Movement *mov = nullptr;

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

	int newx = _x - point.x;
	int newy = _y - point.y;

	_movement = mov;

	if (_flags & 0x40)
		_movement->setLast();
	else
		_movement->gotoFirstFrame();

	_stepArray.clear();
	_stepArray.insertPoints(points);

	if (!(_flags & 0x40)) {
		if (!_movement->_curFrame) {
			point = _stepArray.getCurrPoint();
			newx += point.x + _movement->_prevStepX;
			newy += point.y + _movement->_prevStepY;
			_stepArray.gotoNextPoint();

			Command *ex = _movement->_curPhase->getCommand();

			if (ex) {
				if (ex->_cmd == 35) {
					Command *newEx = ex->createClone();

					newEx->_excFlags |= 2u;
					newEx->sendMessage();
				}
			}
		}
	}

	point = _movement->getCurrDynamicPhaseXY();
	setPos(point.x + newx, point.y + newy);

	if ((_movement->_nextStatic->_staticsId >> 8) & 0x40)
		_flags |= 8;
	else
		_flags &= 0xFFF7;

	_flags |= 1;
	_messageQueueId = messageQueueId;
	_movement->_curPhase->_countdown = _movement->_curPhase->_delay;
	_movement->_counter = 0;
	_counter = _initialCounter;
	_targetFrame = someDynamicPhaseIndex;

	Command *ex = new Command(_id, 17, 23, 0, 0, movementId, 1, 0, 0, 0);

	ex->_param = _copyId;
	ex->_excFlags = 2;
	ex->postMessage();
}

bool AniObject::startAnimEx(int movid, int parId, int flag1, int flag2) {
	bool res = startMovement(movid, parId, -1);
	if (res)
		_animExFlag = 1;

	_targetFrame = -1;
	return res;
}

bool AniObject::startMovement(int movementId, int messageQueueId, int dynPhaseIdx, bool mirror) {
	if (_flags & 0x80)
		return false;

	debugC(4, kDebugAnimation, "AniObject::startMovement(%d, %d, %d) (%s [%d]) [%d, %d]", movementId, messageQueueId, dynPhaseIdx, transCyrillic(_title), _id, _x, _y);

	if (_messageQueueId) {
		updateGlobalMessageQueue(messageQueueId, _id);
		return false;
	}

	const bool android = g_nmi->getPlatform() == Common::kPlatformAndroid;
	Movement *mov = getMovement(movementId);
	// An XML movement without any PHASE elements contains only its end pose.
	if (!mov || (android && !mov->getPhase(1))) {
		updateGlobalMessageQueue(messageQueueId, _id);
		return false;
	}

	if (mov == _movement) {
		_flags |= 1;
		_messageQueueId = messageQueueId;

		return true;
	}

	int newx = _x;
	int newy = _y;
	Common::Point point;

	if (android && !_movement) {
		// SetStatic reflects a change of facing about the old texture's center.
		const bool prevMirror = mirror != mov->_mirrorPrev;
		if (_statics && prevMirror != ((_flags & 8) != 0)) {
			const int shift = _statics->getDimensions().x - 2 * _statics->_someX;
			newx += prevMirror ? shift : -shift;
		}
		_statics = mov->_prevStatic;
	}
	if (mirror)
		newx = -newx;

	if (_movement) {
		point = _movement->getCurrDynamicPhaseXY();

		newx -= point.x;
		newy -= point.y;

	} else if (_statics) {
		point = _statics->getSomeXY();
		if (mov->_mirrorPrev)
			point.x = _statics->getDimensions().x - point.x;

		newx -= point.x;
		newy -= point.y;
	}

	_movement = mov;

	_stepArray.clear();

	if (_flags & 0x40)
		_movement->setLast();
	else
		_movement->gotoFirstFrame();

	if (!(_flags & 0x40)) {
		if (!_movement->_curFrame) {
			point = _stepArray.getCurrPoint();
			newx += point.x + _movement->_prevStepX;
			newy += point.y + _movement->_prevStepY;

			_stepArray.gotoNextPoint();

			Command *ex = _movement->_curPhase->getCommand();
			if (ex && !android) {
				if (ex->_cmd == 35) {
					Command *newex = ex->createClone();
					newex->_excFlags |= 2;
					newex->sendMessage();
				}
			}
		}
	}

	point = _movement->getCurrDynamicPhaseXY();
	newx += point.x;
	setPos(mirror ? -newx : newx, point.y + newy);

	if (mirror || (!android && (_movement->_nextStatic->_staticsId & 0x4000)))
		_flags |= 8;
	else
		_flags &= 0xFFF7;

	_flags |= 1;

	_messageQueueId = messageQueueId;
	_movement->_curPhase->_countdown = _movement->_curPhase->_delay;
	_movement->_counter = 0;

	_counter = _initialCounter;
	_targetFrame = dynPhaseIdx;

	_stepArray.clear();

	Command *newex = new Command(_id, 17, 23, 0, 0, movementId, 1, 0, 0, 0);

	newex->_param = _copyId;
	newex->_excFlags = 2;

	newex->postMessage();

	if (android)
		mov->_curPhase->enter(_id);

	return true;
}

Common::Point *AniObject::calcStepLen(Common::Point *p) {
	if (_movement) {
		Common::Point point;

		point = _movement->getMovementSize(0, _movement->_curFrame);

		p->x = point.x;
		p->y = point.y;

		int idx = _stepArray.getCurrPointIndex() - _movement->_curFrame - 1;

		if (idx >= 0) {
			point = _stepArray.getPoint(idx, _movement->_curFrame + 2);
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

Statics::Statics(Statics *src, bool reverse) : Phase(src, reverse) {
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

	Phase::load(file);

	_staticsId = file.readUint16LE();

	_staticsName = file.readPascalString();
	debugC(6, kDebugXML, "%% <STATICS id=\"%s\" name=\"%s\" %s />",
		g_nmi->gameIdToStr(_staticsId).c_str(), transCyrillic(_staticsName), Phase::toXML().c_str());

	_picture.load(file);

	return true;
}

void Statics::init() {
	Texture::init();

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
	_prevStatic = nullptr;
	_nextStatic = nullptr;
	_prevStepX = 0;
	_prevStepY = 0;
	_nextStepX = 0;
	_nextStepY = 0;
	_link = 1;
	_field_78 = 0;
	_field_84 = 0;
	_curPhase = nullptr;
	_field_8C = 0;
	_curFrame = 0;
	_field_94 = 0;
	_currMovement = nullptr;
	_counter = 0;
	_loopDelay = 83;
}

Movement::~Movement() {
	if (!_currMovement) {
		if (_updateFlag1) {
			_phases[0]->unload();
			delete _phases.remove_at(0);
		}

		// FIXME: At this point, the last entry in _phases is invalid
		for (uint i = 0; i + 1 < _phases.size(); i++) {
			Phase *phase = _phases[i];
			if (phase != _prevStatic && phase != _nextStatic)
				delete phase;
			else
				_phases[i]->unload();
		}
	}
}

Movement::Movement(Movement *src, AniObject *ani) {
	_mirrorPrev = src->_mirrorPrev;
	_mirrorNext = src->_mirrorNext;
	_lastFrameSpecialFlag = 0;
	_flipFlag = src->_flipFlag;
	_updateFlag1 = src->_updateFlag1;
	_prevStatic = nullptr;
	_nextStatic = nullptr;
	_prevStepX = 0;
	_prevStepY = 0;
	_nextStepX = 0;
	_nextStepY = 0;

	_field_78 = 0;
	_field_84 = 0;
	_curPhase = nullptr;
	_field_8C = 0;
	_curFrame = src->_curFrame;
	_field_94 = 0;

	_somePoint.x = 0;
	_somePoint.y = 0;

	_currMovement = src;
	_x = src->_x;
	_y = src->_y;

	initStatics(ani);

	_loopDelay = src->_loopDelay;
	_counter = src->_counter;
	_link = src->_link;

	updateCurrDynamicPhase();
}

Movement::Movement(Movement *src, int *oldIdxs, int newSize, AniObject *ani) : GameObject(src) {
	_mirrorPrev = src->_mirrorPrev;
	_mirrorNext = src->_mirrorNext;
	_lastFrameSpecialFlag = 0;
	_updateFlag1 = 1;
	_prevStatic = nullptr;
	_nextStatic = nullptr;
	_prevStepX = 0;
	_prevStepY = 0;
	_nextStepX = 0;
	_nextStepY = 0;

	_counter = 0;
	_loopDelay = 0;

	_field_78 = 0;
	_field_84 = 0;
	_curPhase = nullptr;
	_field_8C = 0;
	_curFrame = 0;
	_field_94 = 0;

	_somePoint.x = 0;
	_somePoint.y = 0;

	_link = src->_link;
	_flipFlag = src->_flipFlag;
	_currMovement = nullptr;
	_prevStepX = src->_prevStepX;
	_prevStepY = src->_prevStepY;
	_nextStepX = src->_nextStepX;
	_nextStepY = src->_nextStepY;

	if (newSize != -1) {
		if (newSize >= (int)src->_phases.size() + 1)
			newSize = src->_phases.size() + 1;
	} else {
		newSize = src->_phases.size();
	}

	if (!newSize) {
		warning("Movement::Movement: newSize = 0");

		return;
	}

	_framePosOffsets.resize(newSize);

	if (oldIdxs) {
		for (int i = 0; i < newSize - 1; i++, oldIdxs++) {
			if (oldIdxs[i] == -1) {
				_phases.push_back(src->_prevStatic);
			} else {
				src->setCurFrame(oldIdxs[i]);

				_phases.push_back(src->_curPhase);

				_framePosOffsets[i].x = src->_framePosOffsets[oldIdxs[i]].x;
				_framePosOffsets[i].y = src->_framePosOffsets[oldIdxs[i]].y;
			}
		}
		_prevStatic = dynamic_cast<Statics *>(_phases.front());
		_nextStatic = dynamic_cast<Statics *>(_phases.back());
	} else {
		for (int i = 0; i < newSize; i++) {
			src->setCurFrame(i);

			if (i < newSize - 1)
				_phases.push_back(new Phase(src->_curPhase, 0));

			_framePosOffsets[i].x = src->_framePosOffsets[i].x;
			_framePosOffsets[i].y = src->_framePosOffsets[i].y;
		}

		_prevStatic = ani->getStatic(src->_prevStatic->_staticsId);
		_nextStatic = ani->getStatic(src->_nextStatic->_staticsId);

		_phases.push_back(_nextStatic);

		this->_updateFlag1 = src->_updateFlag1;
	}

	updateCurrDynamicPhase();
	removeFirstPhase();

	_loopDelay = src->_loopDelay;
	_counter = src->_counter;
}

bool Movement::load(MfcArchive &file) {
	warning("STUB: Movement::load");
	return true;
}

bool Movement::load(MfcArchive &file, AniObject *ani) {
	GameObject::load(file);

	int dynCount = file.readUint16LE();

	if (dynCount != 0xffff || _id == MV_MAN_TURN_LU) {
		_framePosOffsets.resize(dynCount + 2);

		for (int i = 0; i < dynCount; i++) {
			Phase *ph = new Phase();
			ph->load(file);

			_phases.push_back(ph);

			_framePosOffsets[i].x = ph->_x;
			_framePosOffsets[i].y = ph->_y;
		}

		int staticsid = file.readUint16LE();

		_prevStatic = ani->getStatic(staticsid);

		if (!_prevStatic && (staticsid & 0x4000)) {
			Statics *s = ani->getStatic(staticsid ^ 0x4000);
			_prevStatic = ani->addReverseStatics(s);
		}

		_prevStepX = file.readSint32LE();
		_prevStepY = file.readSint32LE();

		int staticsid2 = file.readUint16LE();

		_nextStatic = ani->getStatic(staticsid2);

		if (!_nextStatic && (staticsid2 & 0x4000)) {
			Statics *s = ani->getStatic(staticsid2 ^ 0x4000);
			_nextStatic = ani->addReverseStatics(s);
		}

		_nextStepX = file.readSint32LE();
		_nextStepY = file.readSint32LE();

		if (_nextStatic) {
			_phases.push_back(_nextStatic);

			_framePosOffsets[_phases.size() - 1].x = _nextStepX;
			_framePosOffsets[_phases.size() - 1].y = _nextStepY;
		}

		debugC(6, kDebugXML, "%% <MOVEMENT %s staticsId=\"%s\" mX=%d my=%d staticsId2=\"%s\" m2x=%d m2y=%d>",
			GameObject::toXML().c_str(), g_nmi->gameIdToStr(staticsid).c_str(), _prevStepX, _prevStepY, g_nmi->gameIdToStr(staticsid2).c_str(), _nextStepX, _nextStepY);

		for (int i = 0; i < dynCount; i++)
			debugC(6, kDebugXML, "%% <PHASE %s />", _phases[i]->toXML().c_str());

		debugC(6, kDebugXML, "%% </MOVEMENT>");

	} else {
		int movid = file.readUint16LE();

		_currMovement = ani->getMovement(movid);
		_prevStatic = nullptr;
		_nextStatic = nullptr;

		initStatics(ani);
	}

	if (_prevStatic && _nextStatic) {
		if ((_prevStatic->_staticsId ^ _nextStatic->_staticsId) & 0x4000)
			_flipFlag = 1;
	}

	if (g_nmi->_gameProjectVersion >= 8)
		_link = file.readUint32LE();

	if (g_nmi->_gameProjectVersion < 12)
		_loopDelay = 83;
	else
		_loopDelay = file.readUint32LE();

	_counter = 0;
	updateCurrDynamicPhase();

	return true;
}

Common::Point Movement::getCurrDynamicPhaseXY() const {
	if (_mirrorNext && _curPhase == _nextStatic)
		return Common::Point(_curPhase->getDimensions().x - _curPhase->_someX, _curPhase->_someY);
	return Common::Point(_curPhase->_someX, _curPhase->_someY);
}

Common::Point Movement::getMovementSize(int idx, int dynidx) {
	int oldox = _x;
	int oldoy = _y;
	int oldidx = _curFrame;

	int x = 0;
	int y = 0;

	if (!idx) {
		const Common::Point point = _prevStatic->getSomeXY();
		int x1 = _prevStepX - point.x;
		int y1 = _prevStepY - point.y;

		setCurFrame(0);

		x = _curPhase->_someX + x1;
		y = _curPhase->_someY + y1;
	}

	setPos(x, y);

	while (_curFrame != dynidx && setNext(nullptr, nullptr))
		;

	Common::Point p(_x, _y);

	setCurFrame(oldidx);
	setPos(oldox, oldoy);

	return p;
}

void Movement::setAlpha(int alpha) {
	if (_currMovement)
		for (uint i = 0; i < _currMovement->_phases.size(); i++) {
			_currMovement->_phases[i]->setAlpha(alpha);
		}
	else
		for (uint i = 0; i < _phases.size(); i++) {
			_phases[i]->setAlpha(alpha);
		}
}

Dims Movement::getPhaseSize(int phaseIndex) const {
	int idx = phaseIndex;

	if (idx == -1)
		idx = _curFrame;

	Phase *dyn;

	if (_currMovement)
		dyn = _currMovement->_phases[idx];
	else
		dyn = _phases[idx];

	return dyn->getDimensions();
}

void Movement::initStatics(AniObject *ani) {
	if (!_currMovement)
		return;

	debugC(7, kDebugAnimation, "Movement::initStatics()");

	_nextStatic = ani->addReverseStatics(_currMovement->_nextStatic);
	_prevStatic = ani->addReverseStatics(_currMovement->_prevStatic);

	_prevStepX = _currMovement->_prevStepX;
	_prevStepY = _currMovement->_prevStepY;

	_currMovement->setCurFrame(_currMovement->_updateFlag1 != 0 ? 1 : 0);

	int x1 = _currMovement->_prevStatic->getDimensions().x - _prevStepX;

	_prevStepX = x1 - _currMovement->_curPhase->getDimensions().x;

	_currMovement->setCurFrame(_currMovement->_curFrame);

	_nextStepX = _currMovement->_nextStepX;
	_nextStepY = _currMovement->_nextStepY;
	_currMovement->setLast();

	x1 = _currMovement->_nextStatic->getDimensions().x;
	_nextStepX = _currMovement->_curPhase->getDimensions().x - _nextStepX - x1;
}

void Movement::updateCurrDynamicPhase() {
	debugC(7, kDebugAnimation, "Movement::updateCurrDynamicPhase()");

	if (_currMovement) {
		if (_currMovement->_phases.size() == 0 || (uint)_curFrame >= _currMovement->_phases.size())
			return;

		if (_currMovement->_phases[_curFrame])
			_curPhase = _currMovement->_phases[_curFrame];
	} else {
		if (_phases.size() == 0 || (uint)_curFrame >= _phases.size())
			return;

		if (_phases[_curFrame])
			_curPhase = _phases[_curFrame];
	}
}

int Movement::calcDuration() {
	int res = 0;

	if (_currMovement)
		for (uint i = 0; i < _currMovement->_phases.size(); i++) {
			res += _currMovement->_phases[i]->_delay + 1;
		}
	else
		for (uint i = 0; i < _phases.size(); i++) {
			res += _phases[i]->_delay + 1;
		}

	return res;
}

int Movement::countPhasesWithFlag(int maxidx, int flag) {
	int res = 0;
	int sz;

	if (_currMovement)
		sz = _currMovement->_phases.size();
	else
		sz = _phases.size();

	if (maxidx < 0)
		maxidx = sz;

	for (int i = 0; i < maxidx && i < sz; i++)
		if (getPhase(i)->_flags & flag)
			res++;

	return res;
}

void Movement::setCurFrame(int index) {
	debugC(7, kDebugAnimation, "Movement::setCurFrame(%d)", index);
	while (_curFrame < index)
		setNext(nullptr, nullptr);

	while (_curFrame > index)
		gotoPrevFrame();
}

Phase *Movement::getPhase(int idx) {
	debugC(7, kDebugAnimation, "Movement::updateCurrDynamicPhase()");

	if (_currMovement) {
		if (_currMovement->_phases.size() == 0 || (uint)idx >= _currMovement->_phases.size())
			return nullptr;

		return _currMovement->_phases[idx];
	} else {
		if (_phases.size() == 0 || (uint)idx >= _phases.size())
			return nullptr;

		return _phases[idx];
	}
}

void Movement::load() {
	Movement *mov = this;
	while (mov->_currMovement)
		mov = mov->_currMovement;

	for (uint i = 0; i < mov->_phases.size(); i++) {
		if (mov->_phases[i] != mov->_nextStatic || !(mov->_nextStatic->_staticsId & 0x4000))
			mov->_phases[i]->getPixelData();
	}

	if (!(mov->_prevStatic->_staticsId & 0x4000))
		mov->_prevStatic->getPixelData();
}

void Movement::unload() {
	if (g_nmi->getPlatform() == Common::kPlatformAndroid) {
		// XML movements borrow their last phase from the actor's statics.
		for (uint i = 0; i + 1 < _phases.size(); ++i)
			_phases[i]->unload();
		return;
	}
	if (!_currMovement)
		for (uint i = 0; i < _phases.size(); i++)
			_phases[i]->unload();

	if (_prevStatic)
		_prevStatic->unload();
}

void Movement::removeFirstPhase() {
	if (_updateFlag1) {
		if (!_curFrame)
			setNext(nullptr, nullptr);

		if (!_currMovement) {
			delete _phases.remove_at(0);

			for (uint i = 0; i < _phases.size(); i++) {
				_framePosOffsets[i] = _framePosOffsets[i + 1];
			}
			_framePosOffsets.pop_back();
		}
		_curFrame--;
	}

	updateCurrDynamicPhase();
	_updateFlag1 = 0;
}

bool Movement::setNext(void (*callback1)(int, Common::Point *point, int, int), void (*callback2)(int *), bool mirror) {
	debugC(8, kDebugAnimation, "Movement::setNext()");

	if (!callback2) {
		if (_currMovement) {
			if ((uint)_curFrame == _currMovement->_phases.size() - 1
				&& !(_currMovement->_phases.back()->_countdown)) {
				return false;
			}
		} else if ((uint)_curFrame == _phases.size() - 1
					&& !(_phases.back()->_countdown)) {
			return false;
		}
	}

	if (_curPhase->_countdown) {
		_curPhase->_countdown--;
		return true;
	}

	const int oldX = _x;
	Common::Point point = getCurrDynamicPhaseXY();
	_x -= point.x;
	_y -= point.y;

	int deltax = 0;

	if (_currMovement)
		deltax = _currMovement->getPhaseSize(_curFrame).x;

	int oldDynIndex = _curFrame;

	if (callback2)
		callback2(&_curFrame);
	else
		_curFrame++;

	bool result = true;

	if (_currMovement) {
		if (_currMovement->_phases.size() <= (uint)_curFrame) {
			_curFrame = _currMovement->_phases.size() - 1;
			result = (callback2 == nullptr);
		}
		if (_curFrame < 0) {
			_curFrame = 0;
			result = false;
		}
		if (_currMovement->_framePosOffsets.size()) {
			if (callback1) {
				point = _currMovement->_framePosOffsets[_curFrame];
				callback1(_curFrame, &point, _x, _y);

				_x += deltax - point.x;
				_y += point.y;

				_x -= _currMovement->getPhaseSize(_curFrame).x;
			} else if (oldDynIndex >= _curFrame) {
				while (oldDynIndex > _curFrame) {
					_x += deltax;
					deltax = _currMovement->getPhaseSize(oldDynIndex).x;

					_x += _currMovement->_framePosOffsets[oldDynIndex].x;
					_y -= _currMovement->_framePosOffsets[oldDynIndex].y;
					oldDynIndex--;

					_x -= _currMovement->getPhaseSize(oldDynIndex).x;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _curFrame; i++) {
					_x += deltax;
					deltax = _currMovement->getPhaseSize(i).x;
					_x -= _currMovement->_framePosOffsets[i].x;
					_y += _currMovement->_framePosOffsets[i].y;
					_x -= _currMovement->getPhaseSize(i).x;
				}
			}
		}
	} else {
		if (_phases.size() <= (uint)_curFrame) {
			_curFrame = _phases.size() - 1;
			result = (callback2 == nullptr);
		}
		if (_curFrame < 0) {
			_curFrame = 0;
			result = false;
		}

		if (_framePosOffsets.size()) {
			if (callback1) {
				point.x = _framePosOffsets[_curFrame].x;
				point.y = _framePosOffsets[_curFrame].y;

				callback1(_curFrame, &point, _x, _y);
				_x += point.x;
				_y += point.y;
			} else if (oldDynIndex >= _curFrame) {
				for (int i = oldDynIndex; i > _curFrame; i--) {
					_x -= _framePosOffsets[i].x;
					_y -= _framePosOffsets[i].y;
				}
			} else {
				for (int i = oldDynIndex + 1; i <= _curFrame; i++) {
					_x += _framePosOffsets[i].x;
					_y += _framePosOffsets[i].y;
				}
			}
		}
	}

	updateCurrDynamicPhase();
	point = getCurrDynamicPhaseXY();
	_x += point.x;
	_y += point.y;

	if (mirror)
		_x = oldX - (_x - oldX);
	_curPhase->_countdown = _curPhase->_delay;

	return result;
}

bool Movement::gotoPrevFrame() {
	debugC(8, kDebugAnimation, "Movement::gotoPrevFrame()");

	if (!_curFrame) {
		setLast();
		return false;
	}

	Common::Point point = getCurrDynamicPhaseXY();

	_x -= point.x;
	_y -= point.y;

	if (_currMovement) {
		if (_currMovement->_framePosOffsets.size()) {
			_x += _currMovement->getPhaseSize(_curFrame).x;
			_x += _currMovement->_framePosOffsets[_curFrame].x;
			_y -= _currMovement->_framePosOffsets[_curFrame].y;
		}

		_curFrame--;
		if (_curFrame < 0)
			_curFrame = _currMovement->_phases.size() - 1;

		_x -= _currMovement->getPhaseSize(_curFrame).x;
	} else {
		if (_framePosOffsets.size()) {
			_x -= _framePosOffsets[_curFrame].x;
			_y -= _framePosOffsets[_curFrame].y;
		}

		_curFrame--;
		if (_curFrame < 0)
			_curFrame = _phases.size() - 1;
	}

	updateCurrDynamicPhase();
	point = getCurrDynamicPhaseXY();

	_x += point.x;
	_y += point.y;

	return true;
}

void Movement::gotoFirstFrame() {
	while (_curFrame)
			gotoPrevFrame();
}

void Movement::setLast() {
	if (_currMovement) {
		if ((uint)_curFrame != _currMovement->_phases.size() - 1) {
			do {
				setNext(nullptr, nullptr);
			} while ((uint)_curFrame != _currMovement->_phases.size() - 1);
		}
	} else {
		if ((uint)_curFrame != _phases.size() - 1) {
			do {
				setNext(nullptr, nullptr);
			} while ((uint)_curFrame != _phases.size() - 1);
		}
	}
}

Common::Point Movement::getCenter() const {
	Common::Rect rect(_curPhase->_rect);

	if (_currMovement) {
		const Dims dims = _currMovement->getPhaseSize(_curFrame);
		rect.moveTo(dims.x - _curPhase->_rect.right, _curPhase->_rect.top);
	}

	return Common::Point(rect.left + _curPhase->_rect.width() / 2,
						 rect.top + _curPhase->_rect.height() / 2);
}

Phase::Phase() {
	_someX = 0;
	_field_7C = 0;
	_field_7E = 0;
	_flags = 0;
	_someY = 0;
	_data = nullptr;
}

Phase::Phase(Phase *src, bool reverse) {
	_field_7C = src->_field_7C;
	_field_7E = 0;

	debugC(1, kDebugAnimation, "Phase::Phase(src, %d)", reverse);

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
			_hasMirroredBitmap = 1;
			_bitmap.reset(src->_bitmap->reverseImage(false));
		}

		_someX = src->_someX;
		_someY = src->_someY;
	}

	_rect = src->_rect;

	_width = src->_width;
	_height = src->_height;
	_field_7C = src->_field_7C;

	if (src->getCommand())
		_command.reset(src->getCommand()->createClone());
	else
		_command.reset();

	_delay = src->_delay;
	_delayMs = src->_delayMs;
	_delayRelative = src->_delayRelative;
	_soundId = src->_soundId;
	_loopSound = src->_loopSound;
	_field_6A = src->_field_6A;
	_flags = src->_flags;

	debug(8, "Phase::Phase(): pal: %p, pal size: %d",
		(const void *)&src->getPaletteData(), src->getPaletteData().size);
	setPaletteData(src->getPaletteData());

	copyMemoryObject2(*src);
}

Common::String Phase::toXML() {
	return Common::String::format("f7c=%d left=%d top=%d right=%d bottom=%d sX=%d sY=%d dynFlags=%d %s",
			_field_7C, _rect.left, _rect.top, _rect.right, _rect.bottom, _someX, _someY, _flags,
			StaticPhase::toXML().c_str());
}

void Phase::setPNG(const Common::Path &path, const Dims &size) {
	_memfilename = path;
	_width = size.x;
	_height = size.y;
}

void Phase::enter(int objectId) {
	if (!_bitmap && !_memfilename.empty())
		loadPNG(_memfilename, getDimensions());
	if (_soundId && g_nmi->_soundEnabled)
		g_nmi->playSound(_soundId, _loopSound, objectId);
	if (_command) {
		Command *cmd = _command->createClone();
		cmd->_excFlags |= 2;
		cmd->sendMessage();
	}
}

bool Phase::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "Phase::load()");

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

	_flags = file.readUint32LE();

	return true;
}

StaticPhase::StaticPhase() {
	_field_6A = 1;
	_delay = 0;
	_countdown = 0;
	_messageNum = 0;
}

Common::String StaticPhase::toXML() {
	return Common::String::format("countdown=%d f6a=%d", _delay, _field_6A);
}

bool StaticPhase::load(MfcArchive &file) {
	debugC(5, kDebugLoading, "StaticPhase::load()");

	Texture::load(file);

	_delay = file.readUint16LE();
	_field_6A = file.readUint16LE();

	assert(g_nmi->_gameProjectVersion >= 12);

	_command.reset(file.readClass<Command>());

	return true;
}

} // End of namespace NGI
