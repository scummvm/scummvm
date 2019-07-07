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

#include "common/ini-file.h"
#include "common/stream.h"
#include "common/system.h"
#include "common/events.h"

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "petka/flc.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/objects/object_star.h"
#include "petka/objects/object_cursor.h"
#include "petka/interfaces/main.h"

namespace Petka {

QVisibleObject::QVisibleObject()
	: _resourceId(-1), _z(240) {}

QMessageObject::QMessageObject() {
	_id = -1;
	_status = 0;
	_time = 0;
	_dialogColor = -1;
	_animate = true;
	_isShown = true;
	_isActive = true;
	_updateZ = 0;
	_field_38 = 0;
	_notLoopedSound = true;
	_startSound = false;
	_hasSound = false;
	_reaction = nullptr;
}

static void processSavedReaction(QReaction **reaction, QMessageObject *obj) {
	QReaction *r = *reaction;
	for (uint i = 0; i < r->messages.size(); ++i) {
		QMessage &msg = r->messages[i];
		if (msg.opcode == kCheck && g_vm->getQSystem()->findObject(msg.objId)->_status != msg.arg1) {
			break;
		}
		g_vm->getQSystem()->addMessage(msg.objId, msg.opcode, msg.arg1, msg.arg2, msg.arg3, 0, obj);
		bool processed = true;
		switch (msg.opcode) {
		case kDialog:
			break;
		case kPlay: {
			QMessageObject *obj = g_vm->getQSystem()->findObject(msg.objId);
			obj->_reaction = new QReaction();
			obj->_reactionResId = msg.arg1;
			for (uint j = i + 1; j < r->messages.size(); ++j) {
				obj->_reaction->messages.push_back(r->messages[j]);
			}
			break;
		}
		case kWalk:
		case kWalkTo:
		case kWalkVich:
			break;
		default:
			processed = false;
			break;
		}
		if (processed)
			break;
	}
	if (*reaction != r) {
		delete r;
		return;
	}
	if (*reaction) {
		delete *reaction;
		*reaction = nullptr;
	}
}

void QMessageObject::processMessage(const QMessage &msg) {
	for (uint i = 0; i < _reactions.size(); ++i) {
		QReaction &r = _reactions[i];
		if (r.opcode != msg.opcode ||
			(r.status != -1 && r.status != _status) ||
			(r.senderId != -1 && r.senderId != msg.sender->_id)) {
			continue;
		}
		for (uint j = 0; j < r.messages.size(); ++j) {
			QMessage &rMsg = r.messages[j];
			if (rMsg.opcode == kCheck && g_vm->getQSystem()->findObject(rMsg.objId)->_status != rMsg.arg1) {
				break;
			}
			if (rMsg.opcode == kIf &&
				(rMsg.arg1 == -1 || rMsg.arg1 != msg.arg1) &&
				(rMsg.arg2 == -1 || rMsg.arg2 != msg.arg2) &&
				(rMsg.arg3 == -1 || rMsg.arg3 != msg.arg3)) {
				break;
			}
			if (rMsg.opcode == kRandom && rMsg.arg2 != -1) {
				rMsg.arg1 = (int16) g_vm->getRnd().getRandomNumber((uint) (rMsg.arg2 - 1));
			}
			g_vm->getQSystem()->addMessage(rMsg.objId, rMsg.opcode, rMsg.arg1, rMsg.arg2, rMsg.arg3, rMsg.unk,
										   rMsg.sender);
			bool processed = true;
			switch (rMsg.opcode) {
			case kPlay: {
				QMessageObject *obj = g_vm->getQSystem()->findObject(rMsg.objId);
				delete obj->_reaction;
				obj->_reaction = new QReaction();
				obj->_reactionResId = rMsg.arg1;
				for (uint z = j + 1; z < r.messages.size(); ++z) {
					obj->_reaction->messages.push_back(r.messages[z]);
				}
				break;
			}
			case kWalk:
			case kWalkTo:
				break;
			case kWalkVich:
				break;
			default:
				processed = false;
			}
			if (processed)
				break;
		}
	}

	switch (msg.opcode) {
	case kAvi: {
		Common::String videoName = g_vm->resMgr()->findResourceName((uint16)msg.arg1);
		g_vm->playVideo(g_vm->openFile(videoName, false));
		break;
	}
	case kSet:
	case kPlay:
		if (dynamic_cast<QObjectBG *>(this)) {
			break;
		}
		if (g_vm->getQSystem()->_isIniting) {
			_resourceId = msg.arg1;
			_notLoopedSound = msg.arg2 != 5;
		} else {
			_sound = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(_resourceId), Audio::Mixer::kSFXSoundType);
			_hasSound = _sound != nullptr;
			_startSound = false;
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
			if (flc) {
				g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
			}

			flc = g_vm->resMgr()->loadFlic(msg.arg1);
			flc->setFrame(1);
			_time = 0;
			if (_notLoopedSound) {
				g_vm->soundMgr()->removeSound(g_vm->resMgr()->findSoundName(_resourceId));
			}
			_resourceId = msg.arg1;
		}
		if (msg.arg2 == 1) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
			flc->setFrame(1);
			g_vm->videoSystem()->makeAllDirty();
			_time = 0;
		} else if (msg.arg2 == 2) {
			g_vm->resMgr()->loadFlic(_resourceId);
		}
		_notLoopedSound = msg.arg2 != 5;
		break;
	case kEnd:
		if (_reaction && _reactionResId == msg.arg1) {
			processSavedReaction(&_reaction, this);
		}
 		break;
	case kStatus:
		_status = (int8)msg.arg1;
		break;
	case kOn:
		_isActive = true;
		show(true);
		break;
	case kOff:
		_isActive = false;
		show(false);
		break;
	case kStop:
		g_vm->getQSystem()->_cursor.get()->show(msg.arg1);
		g_vm->getQSystem()->_star.get()->_isActive = msg.arg1;
	case kShow:
		show(true);
		break;
	case kHide:
		show(false);
		break;
	case kZBuffer:
		_updateZ = msg.arg1;
		_z = (msg.arg2 != -1) ? msg.arg2 : _z;
		break;
	case kActive:
		_isActive = msg.arg1;
		break;
	case kPassive:
		_isActive = false;
		break;
	}

}

void QMessageObject::show(bool v) {
	_isShown = v;
}

QObject::QObject() {
	_animate = true;
	_updateZ = true;
	_field24 = true;
	_field20 = true;
	_sound = nullptr;
	_x = 0;
	_y = 0;
	_field14 = -1;
	_field18 = -1;
	_field28 = 0;
}

bool QObject::isInPoint(int x, int y) {
	if (!_isActive)
		return false;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	Common::Rect rect(_x, _y, _x + flc->getWidth(), _y + flc->getHeight());
	if (!rect.contains(x, y))
		return false;
	return *(byte *) flc->getCurrentFrame()->getBasePtr(x - _x, y - _y) != 0;
}

void QObject::draw() {
	if (!_isShown || _resourceId == -1) {
		return;
	}
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (!flc) {
		return;
	}
	if (_animate && _startSound) {
		if (_sound) {
			_sound->play(!_notLoopedSound);
			_startSound = false;
		}
	}

	Common::Rect screen(640, 480);
	Common::Rect dest(flc->getBounds());
	//flcRect.translate(_x, _y);
	dest.translate(_x, _y);

	Common::Rect intersect(screen.findIntersectingRect(dest));
	if (intersect.isEmpty())
		return;

	const Graphics::Surface *frame = flc->getCurrentFrame();
	Graphics::Surface *s = frame->convertTo(g_system->getScreenFormat(), flc->getPalette());
	const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
	for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
		Common::Rect destRect(intersect.findIntersectingRect(*it));
		if (destRect.isEmpty())
			continue;
		Common::Rect srcRect(destRect);
		srcRect.translate(-_x, -_y);
		g_vm->videoSystem()->screen().transBlitFrom(*s, srcRect, destRect, flc->getTransColor(s->format));
	}
	s->free();
	delete s;
}

void QObject::updateZ() {
	if (!_animate || !_isShown || !_updateZ)
		return;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc) {
		_z = 1;
		const Common::Array<Common::Rect> rects = flc->getMskRects();
		for (uint i = 0; i < rects.size(); ++i) {
			if (_y + rects[i].bottom > _z)
				_z = _y + rects[i].bottom;
		}

	}
}

void QObject::show(bool v) {
	const Common::Array<QVisibleObject*> & objs = g_vm->getQSystem()->_mainInterface->_objs;
	for (uint i = 0; i < objs.size(); ++i) {
		if (objs[i]->_resourceId == _resourceId) {
			FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
			if (flc) {
				g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
			}
			break;
		}
	}
	QMessageObject::show(v);
}

void QObject::update(int time) {
	if (!_animate || !_isShown)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc) {
		while (_time > flc->getDelay()) {
			if (_sound && _hasSound && flc->getCurFrame() == 0) {
				_startSound = true;
				_hasSound = false;
			}
			g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
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
			_time -= flc->getDelay();
		}
	}
}

}