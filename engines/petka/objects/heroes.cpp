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

#include "common/system.h"

#include "petka/petka.h"
#include "petka/q_manager.h"
#include "petka/q_system.h"
#include "petka/flc.h"
#include "petka/video.h"
#include "petka/sound.h"
#include "petka/objects/heroes.h"

namespace Petka {

QObjectPetka::QObjectPetka() {
	_field7C = 1;
	_reaction = nullptr;
	_sender = nullptr;
	_isPetka = true;
	_isWalking = false;
	_x = 574;
	_y = 44;
	_z = 200;
	_surfId  = -5;
	_surfH = 0;
	_surfW = 0;
	_field98 = 1.0;
}

void QObjectPetka::processMessage(const QMessage &arg) {
	QMessage msg = arg;
	if (msg.opcode == kImage) {
		msg.opcode = kSet;
		_imageId = msg.arg1;
	}
	if (msg.opcode == kSaid || msg.opcode == kStand) {
		msg.opcode = kSet;
		msg.arg1 = _imageId + 1;
		msg.arg2 = 1;
	}
	if (msg.opcode == kSet || msg.opcode == kPlay) {
		_field7C = msg.arg2 == _imageId || msg.opcode == kPlay;
	}
	if (msg.opcode != kWalk) {
		if (msg.opcode == kWalked && _heroReaction) {
			processSavedReaction(&_heroReaction, _sender);
		}
		QMessageObject::processMessage(msg);
		if (msg.opcode == kSet || msg.opcode == kPlay) {
			initSurface();
			if (!g_vm->getQSystem()->_isIniting) {
				setPos(_x_, _y_);
			}
		}
	}
}

void QObjectPetka::initSurface() {
	QManager *resMgr = g_vm->resMgr();
	FlicDecoder *flc = resMgr->loadFlic(_resourceId);
	resMgr->removeResource(_surfId);
	resMgr->findOrCreateSurface(_surfId, flc->getWidth(), flc->getHeight());
	_surfW = flc->getWidth() * _field98;
	_surfH = flc->getHeight() * _field98;
}

void QObjectPetka::walk(int x, int y) {
	Common::Point walkPos(x, y);
	if (_isShown) {
		Common::Point currPos;
		if (_isWalking) {
			// currPos = _walkObj->currPos();
		} else {
			currPos.x = _x_;
			currPos.y = _y_;
		}

		if (currPos.sqrDist(walkPos) >= 25 * 25) {
			//_walkObj->init(currPos, walkPos);
			_destX = x;
			_destY = y;
			_resourceId = _imageId; // + _walkObj->getResId() + 10;
			_isWalking = true;
			_animate = true;

			initSurface();
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
			flc->setFrame(1);

			g_vm->videoSystem()->makeAllDirty();

			_field7C = 0;
			_time = 0;
			_holdMessages = true;
		}
	} else {
		setPos(x, y);
	}
}

void QObjectPetka::draw() {
	if (!_isShown || _resourceId == -1) {
		return;
	}

	if (_animate && _startSound) {
		if (_sound) {
			_sound->play(!_notLoopedSound);
		}
		_startSound = false;
	}

	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	Graphics::Surface *surf = g_vm->resMgr()->loadBitmap(_surfId);
	if (!flc || !surf) {
		return;
	}
	Graphics::Surface *conv = flc->getCurrentFrame()->convertTo(g_system->getScreenFormat(), flc->getPalette());
	surf->copyRectToSurface(*conv, 0, 0, Common::Rect(0, 0, flc->getWidth() - 1, flc->getHeight() - 1));

	Common::Rect srcRect(0, 0, _surfW, _surfH);
	Common::Rect dstRect(srcRect);
	dstRect.translate(_x, _y);

	g_vm->videoSystem()->screen().transBlitFrom(*surf, srcRect, dstRect, flc->getTransColor(surf->format));
	conv->free();
	delete conv;
}

void QObjectPetka::setPos(int x, int y) {
	y = MIN(y, 480);
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);

	_field98 = calcSmth(y);

	_surfH = flc->getHeight() * _field98;
	_surfW = flc->getWidth() * _field98;

	_x_ = x;
	_y_ = y;

	_x = x - _surfW / 2;
	_y = y - _surfH;

	g_vm->videoSystem()->makeAllDirty();
}

double QObjectPetka::calcSmth(int y) {
	QSystem *qsys = g_vm->getQSystem();

	y = MIN(y, 480);


	if (!qsys->_unkMap.contains(qsys->_room->_name)) {
		return 1.0;
	}

	const UnkStruct &unk = qsys->_unkMap.getVal(qsys->_room->_name);


	double res = (y - unk.f3) * unk.f2 / (unk.f4 - unk.f3);
	if (res < 0.0)
		res = 0.0;

	if (res + unk.f1 > unk.f5)
		return unk.f5;
	return res + unk.f1;
}

void QObjectPetka::updateWalk() {
	if (_isWalking) {
		_isWalking = false;
		setPos(_destX, _destY);

		QMessage msg(_id, kSet, (uint16)_imageId, 1, 0, nullptr, 0);
		if (_heroReaction) {
			uint i;
			for (i = 0; i < _heroReaction->messages.size(); ++i) {
				if (_heroReaction->messages[i].opcode == kGoTo || _heroReaction->messages[i].opcode == kSetSeq) {
					_resourceId = _imageId; // + _walkObj->getResOffset() + 10;

					FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
					flc->setFrame(1);

					initSurface();

					processMessage(QMessage(_id, kAnimate, 0, 0, 0, nullptr, 0));

					_heroReaction->messages.push_back(msg);
					_heroReaction->messages.push_back(QMessage(_id, kAnimate, 1, 0, 0, nullptr, 0));
					break;
				}
			}
			if (i == _heroReaction->messages.size())
				processMessage(msg);
		} else {
			processMessage(msg);
		}
		_holdMessages = false;
		g_vm->videoSystem()->makeAllDirty();
	}
}

void QObjectPetka::setReactionAfterWalk(uint index, QReaction **reaction, QMessageObject *sender, bool deleteReaction) {
	QReaction *r = *reaction;
	_heroReaction = nullptr;

	stopWalk();

	QMessage msg(_id, kWalked, 0, 0, 0, sender, 0);
	_heroReaction = new QReaction();
	_sender = sender;

	for (uint i = index + 1; i < r->messages.size(); ++i) {
		_heroReaction->messages.push_back(r->messages[i]);
	}

	if (deleteReaction) {
		if (r == *reaction) {
			if (*reaction) {
				delete *reaction;
			}
			*reaction = nullptr;
		} else {
			delete r;
		}
	}

}

void QObjectPetka::stopWalk() {
	_isWalking = false;
	_holdMessages = false;

	Common::List<QMessage> &list = g_vm->getQSystem()->_messages;
	for (Common::List<QMessage>::iterator it = list.begin(); it != list.end();) {
		if (it->opcode == kWalked && it->objId == _id) {
			it->objId = -1;
		}

	}

	delete _heroReaction;
	_heroReaction = nullptr;

	if (!_field7C) {
		QMessage msg(_id, kSet, (uint16)_imageId, 1, 0, nullptr, 0);
		processMessage(msg);
	}
}

void QObjectPetka::update(int time) {
	if (!_animate || !_isShown)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc && flc->getFrameCount() != 1) {
		while (_time >= flc->getDelay()) {
			if (_sound && _hasSound && flc->getCurFrame() == 0) {
				_startSound = true;
				_hasSound = false;
			}
			flc->setFrame(-1);
			if (flc->getCurFrame() == flc->getFrameCount() - 1) {
				if (_notLoopedSound) {
					_hasSound = _sound != nullptr;
				}
				g_vm->getQSystem()->addMessage(_id, kEnd, _resourceId, 0, 0, 0, 0);
			}
			if (flc->getCurFrame() + 1 == flc->getFrameCount() / 2) {
				g_vm->getQSystem()->addMessage(_id, kHalf, _resourceId, 0, 0, 0, 0);
			}

			if (_field7C && flc->getCurFrame() == 0)
				_time = -10000;

			updateWalk();
			flc = g_vm->resMgr()->loadFlic(_resourceId);

			_surfH = flc->getHeight() * _field98;
			_surfW = flc->getWidth() * _field98;

			_time -= flc->getDelay();

			g_vm->videoSystem()->addDirtyRect(Common::Rect(_x, _y, _surfW + _x, _surfH + _y));
		}
	}
}

QObjectChapayev::QObjectChapayev() {
	_x = 477;
	_y = 350;
	_surfId = -6;
	_isPetka = false;
}

}
