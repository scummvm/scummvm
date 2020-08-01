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
	_heroReaction = nullptr;
	_sender = nullptr;
	_isPetka = true;
	_isWalking = false;
	_x = 574;
	_y = 44;
	_z = 200;
	// _surfId  = -5;
	_surfH = 0;
	_surfW = 0;
	_k = 1.0;
}

void QObjectPetka::processMessage(const QMessage &arg) {
	QMessage msg = arg;
	if (msg.opcode == kImage) {
		msg.opcode = kSet;
		_imageId = msg.arg1;
		// delete _walkObj;
		// _walkObj = new Walk(_imageId + 10);
		// int backgroundId = g_vm->resMgr()->findResourceName(g_vm->getQSystem()->_room->_resourceId)
		// _walkObj->setBackground(backgroundId);

	}
	if (msg.opcode == kSaid || msg.opcode == kStand) {
		msg.opcode = kSet;
		msg.arg1 = _imageId;
		msg.arg2 = 1;
	}
	if (msg.opcode == kSay) {
		msg.opcode = kSet;
		msg.arg1 = _imageId + 1;
		msg.arg2 = 1;
	}
	if (msg.opcode == kSet || msg.opcode == kPlay) {
		_field7C = msg.arg2 == _imageId || msg.opcode == kPlay;
	}
	if (msg.opcode != kWalk) {
		if (msg.opcode == kWalked && _heroReaction) {
			QReaction *reaction = _heroReaction;
			_heroReaction = nullptr;
			_sender->processReaction(reaction);
		}
		QMessageObject::processMessage(msg);
		if (msg.opcode == kSet || msg.opcode == kPlay) {
			initSurface();
			if (!g_vm->getQSystem()->_totalInit) {
				setPos(Common::Point(_x_, _y_), false);
			}
		}
	}
}

void QObjectPetka::initSurface() {
	QManager *resMgr = g_vm->resMgr();
	FlicDecoder *flc = resMgr->loadFlic(_resourceId);
	_surfW = flc->getWidth() * _k;
	_surfH = flc->getHeight() * _k;
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

		//if (currPos.sqrDist(walkPos) >= 25 * 25) {
			//_walkObj->init(currPos, walkPos);
			_destX = x;
			_destY = y;
			_resourceId = _imageId + 10; // + _walkObj->getResId() + 10;
			_isWalking = true;
			_animate = true;

			initSurface();
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
			flc->setFrame(1);

			g_vm->videoSystem()->makeAllDirty();

			_field7C = 0;
			_time = 0;
			_holdMessages = true;
		//}
	} else {
		setPos(Common::Point(x, y), false);
	}
}

void QObjectPetka::draw() {
	if (!_isShown || _resourceId == -1) {
		return;
	}

	if (_animate && _startSound) {
		if (_sound) {
			_sound->play(_loopedSound);
			if (_loopedSound) {
				_sound = nullptr;
			}
		}
		_startSound = false;
	}

	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (!flc) {
		return;
	}
	Graphics::Surface *conv = flc->getCurrentFrame()->convertTo(g_system->getScreenFormat(), flc->getPalette());

	Common::Rect srcRect(0, 0, conv->w, conv->h);
	Common::Rect dstRect(0, 0, _surfW, _surfH);
	dstRect.translate(_x, _y);

	g_vm->videoSystem()->transBlitFrom(*conv, srcRect, dstRect, flc->getTransColor(conv->format));
	conv->free();
	delete conv;
}

void QObjectPetka::setPos(Common::Point p, bool) {
	p.y = MIN<int16>(p.y, 480);
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);

	_k = calcPerspective(p.y);

	_surfH = flc->getHeight() * _k;
	_surfW = flc->getWidth() * _k;

	_x_ = p.x;
	_y_ = p.y;

	_x = p.x - _surfW / 2;
	_y = p.y - _surfH;

	g_vm->videoSystem()->makeAllDirty();
}

double QObjectPetka::calcPerspective(int y) {
	QSystem *qsys = g_vm->getQSystem();

	y = MIN(y, 480);

	const Perspective &pers = qsys->_room->_persp;
	double res = (y - pers.y0) * pers.k / (pers.y1 - pers.y0);
	if (res < 0.0)
		res = 0.0;

	if (res + pers.f0 > pers.f1)
		return pers.f1;
	return res + pers.f0;
}

void QObjectPetka::updateWalk() {
	if (_isWalking) {
		_isWalking = false;
		setPos(Common::Point(_destX, _destY), false);

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

void QObjectPetka::setReactionAfterWalk(uint index, QReaction *reaction, QMessageObject *sender, bool deleteReaction) {
	_heroReaction = nullptr;

	stopWalk();

	QMessage msg(_id, kWalked, 0, 0, 0, sender, 0);
	g_vm->getQSystem()->addMessage(msg);
	_heroReaction = new QReaction();
	_sender = sender;

	for (uint i = index + 1; i < reaction->messages.size(); ++i) {
		_heroReaction->messages.push_back(reaction->messages[i]);
	}

	if (deleteReaction) {
		delete reaction;
	}

}

void QObjectPetka::stopWalk() {
	_isWalking = false;
	_holdMessages = false;

	Common::List<QMessage> &list = g_vm->getQSystem()->_messages;
	for (Common::List<QMessage>::iterator it = list.begin(); it != list.end(); ++it) {
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
		while (_time >= (int)flc->getDelay()) {
			if (_sound && flc->getCurFrame() == 0) {
				_startSound = true;
			}
			flc->setFrame(-1);
			if (flc->getCurFrame() == (int32)flc->getFrameCount() - 1) {
				g_vm->getQSystem()->addMessage(_id, kEnd, _resourceId, 0, 0, 0, 0);
			}
			if (flc->getCurFrame() + 1 == (int32)flc->getFrameCount() / 2) {
				g_vm->getQSystem()->addMessage(_id, kHalf, _resourceId, 0, 0, 0, 0);
			}

			//if (_field7C && flc->getCurFrame() == 0)
			//	_time = -10000;

			updateWalk();
			flc = g_vm->resMgr()->loadFlic(_resourceId);

			_surfH = flc->getHeight() * _k;
			_surfW = flc->getWidth() * _k;

			_time -= flc->getDelay();

			g_vm->videoSystem()->addDirtyRect(Common::Rect(_x, _y, _surfW + _x, _surfH + _y));
		}
	}
}

bool QObjectPetka::isInPoint(Common::Point p) {
	if (!_isActive)
		return false;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	const Graphics::Surface *flcSurface = flc->getCurrentFrame();
	Common::Rect bounds(_surfW, _surfH);
	Graphics::ManagedSurface s(_surfW, _surfH, flcSurface->format);
	s.transBlitFrom(*flcSurface, Common::Rect(0, 0, flcSurface->w, flcSurface->h), bounds);
	p.x -= _x;
	p.y -= _y;
	if (!bounds.contains(p.x, p.y))
		return false;
	return *(uint16 *)s.getBasePtr(p.x, p.y) != 0;
}

void QObjectPetka::updateZ() {
	if (_animate && _isShown && _updateZ) {
		FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
		if (_isWalking) {
			// _z = _walkObj->currPos().y
		} else {
			_z = _y + flc->getHeight() * _k;
		}
	}
}

QObjectChapayev::QObjectChapayev() {
	_x = 477;
	_y = 350;
	// _surfId = -6;
	_isPetka = false;
}

}
