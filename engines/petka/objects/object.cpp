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

#include "petka/big_dialogue.h"
#include "petka/flc.h"
#include "petka/sound.h"
#include "petka/petka.h"
#include "petka/video.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/objects/object_star.h"
#include "petka/objects/object_cursor.h"
#include "petka/interfaces/main.h"
#include "petka/objects/heroes.h"
#include "petka/objects/object_case.h"

namespace Petka {

QReaction *createReaction(QMessage *messages, QMessage *end) {
	QReaction *reaction = new QReaction();
	while (messages != end) {
		reaction->messages.push_back(*messages++);
	}
	return reaction;
}

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
	_holdMessages = false;
	_notLoopedSound = true;
	_startSound = false;
	_hasSound = false;
	_reaction = nullptr;
}

void processSavedReaction(QReaction *reaction, QMessageObject *sender) {
	for (uint i = 0; i < reaction->messages.size(); ++i) {
		QMessage &msg = reaction->messages[i];
		if (msg.opcode == kCheck && g_vm->getQSystem()->findObject(msg.objId)->_status != msg.arg1) {
			break;
		}
		g_vm->getQSystem()->addMessage(msg.objId, msg.opcode, msg.arg1, msg.arg2, msg.arg3, 0, sender);
		bool processed = true;
		switch (msg.opcode) {
		case kDialog: {
			g_vm->getQSystem()->_mainInterface->_dialog.setReaction(createReaction(reaction->messages.data() + i + 1, reaction->messages.end()));
			break;
		}
		case kPlay: {
			QMessageObject *obj = g_vm->getQSystem()->findObject(msg.objId);
			obj->setReaction(msg.arg1, createReaction(reaction->messages.data() + i + 1, reaction->messages.end()));
			break;
		}
		case kWalk:
		case kWalkTo:
			g_vm->getQSystem()->_petka->setReactionAfterWalk(i, reaction, sender, true);
			return;
		case kWalkVich:
			g_vm->getQSystem()->_chapayev->setReactionAfterWalk(i, reaction, sender, true);
			return;
		default:
			processed = false;
			break;
		}
		if (processed)
			break;
	}
	delete reaction;
}

void QMessageObject::processMessage(const QMessage &msg) {
	bool reacted = false;
	for (uint i = 0; i < _reactions.size(); ++i) {
		QReaction *r = &_reactions[i];
		if (r->opcode != msg.opcode ||
			(r->status != -1 && r->status != _status) ||
			(r->senderId != -1 && r->senderId != msg.sender->_id)) {
			continue;
		}
		bool fallback;
		if (g_vm->getBigDialogue()->findHandler(_id, msg.opcode, &fallback) && !fallback) {
			g_vm->getBigDialogue()->setHandler(_id, msg.opcode);
			g_vm->getQSystem()->_mainInterface->_dialog.setSender(this);
		}
		for (uint j = 0; j < r->messages.size(); ++j) {
			QMessage &rMsg = r->messages[j];
			if (rMsg.opcode == kCheck && g_vm->getQSystem()->findObject(rMsg.objId)->_status != rMsg.arg1) {
				break;
			}
			if (rMsg.opcode == kIf &&
				((rMsg.arg1 != 0xffff && rMsg.arg1 != msg.arg1) ||
			    (rMsg.arg2 != -1 && rMsg.arg2 != msg.arg2) ||
			    (rMsg.arg3 != -1 && rMsg.arg3 != msg.arg3))) {
				break;
			}
			if (rMsg.opcode == kRandom && rMsg.arg2 != -1) {
				rMsg.arg1 = (int16) g_vm->getRnd().getRandomNumber((uint) (rMsg.arg2 - 1));
			}
			g_vm->getQSystem()->addMessage(rMsg.objId, rMsg.opcode, rMsg.arg1, rMsg.arg2, rMsg.arg3, rMsg.unk,
										   rMsg.sender);
			bool processed = true;
			switch (rMsg.opcode) {
			case kDialog: {
				g_vm->getQSystem()->_mainInterface->_dialog.setReaction(createReaction(r->messages.data() + j + 1, r->messages.end()));
				break;
			}
			case kPlay: {
				QMessageObject *obj = g_vm->getQSystem()->findObject(rMsg.objId);
				obj->setReaction(rMsg.arg1, createReaction(r->messages.data() + j + 1, r->messages.end()));
				break;
			}
			case kWalk:
			case kWalkTo:
				g_vm->getQSystem()->_petka->setReactionAfterWalk(j, r, this, false);
				break;
			case kWalkVich:
				g_vm->getQSystem()->_chapayev->setReactionAfterWalk(j, r, this, false);
				break;
			default:
				processed = false;
				break;
			}
			if (processed)
				break;
		}
		reacted = true;
	}

	if (reacted || !g_vm->getBigDialogue()->findHandler(_id, msg.opcode, nullptr)) {
		switch (msg.opcode) {
		case kAddInv:
			g_vm->getQSystem()->_case->addItem(msg.objId);
			break;
		case kDelInv:
			g_vm->getQSystem()->_case->removeItem(msg.objId);
			break;
		case kSetInv:
			g_vm->getQSystem()->_case->transformItem(msg.sender->_id, msg.objId);
			break;
		case kAvi: {
			Common::String videoName = g_vm->resMgr()->findResourceName((uint16) msg.arg1);
			g_vm->playVideo(g_vm->openFile(videoName, false));
			break;
		}
		case kContinue:
			g_vm->getQSystem()->_mainInterface->_dialog.endUserMsg();
			break;
		case kCursor:
			if (msg.arg1 == 0xffff) {
				g_vm->getQSystem()->_cursor->returnInvItem();
				g_vm->getQSystem()->_cursor->_resourceId = 5002;
				g_vm->getQSystem()->_cursor->_actionType = 0;
				g_vm->getQSystem()->_cursor->_invObj = nullptr;
			} else {
				g_vm->getQSystem()->_cursor->returnInvItem();
				g_vm->getQSystem()->_cursor->_resourceId = msg.arg1;
				g_vm->getQSystem()->_cursor->_actionType = kActionObjUse;
				g_vm->getQSystem()->_cursor->_invObj = this;
				_isShown = 0;
				_isActive = 0;
			}
			g_vm->videoSystem()->makeAllDirty();
			break;
		case kDialog:
			g_vm->getQSystem()->_mainInterface->_dialog.start(msg.arg1, this);
			break;
		case kSetPos:
			setPos(msg.arg1, msg.arg2);
			break;
		case kSet:
		case kPlay:
			if (dynamic_cast<QObjectBG *>(this)) {
				break;
			}
			if (g_vm->getQSystem()->_isIniting) {
				_resourceId = msg.arg1;
				_notLoopedSound = msg.arg2 != 5;
			} else {
				_sound = g_vm->soundMgr()->addSound(g_vm->resMgr()->findSoundName(msg.arg1),
													Audio::Mixer::kSFXSoundType);
				_hasSound = _sound != nullptr;
				_startSound = false;
				FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
				if (flc) {
					g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
				}

				flc = g_vm->resMgr()->loadFlic(msg.arg1);
				flc->setFrame(1);
				_time = 0;
				if (!_notLoopedSound) {
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
		case kAnimate:
			_animate = msg.arg1;
			break;
		case kEnd:
			if (_reaction && _reactionId == msg.arg1) {
				QReaction *reaction = _reaction;
				_reaction = nullptr;
				processSavedReaction(reaction, this);
			}
			break;
		case kStatus:
			_status = (int8) msg.arg1;
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
			break;
		case kShow:
			show(true);
			break;
		case kShake:
			g_vm->videoSystem()->setShake(msg.arg1);
			break;
		case kSystem:
			switch (msg.arg1){
			case 0:
				g_vm->getQSystem()->_star->_isActive = 0;
				break;
			case 1:
				g_vm->getQSystem()->_star->_isActive = 1;
				break;
			case 242:
				g_system->quit();
				break;
			default:
				break;
			}
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
		case kJump:
			g_vm->getQSystem()->_petka->setPos((msg.arg1 == 0xffff ? _walkX : msg.arg1), (msg.arg2 == -1 ? _walkY : msg.arg2));
			break;
		case kJumpVich:
			g_vm->getQSystem()->_petka->setPos((msg.arg1 == 0xffff ? _walkX : msg.arg1), (msg.arg2 == -1 ? _walkY : msg.arg2));
			break;
		case kWalk:
			if (!reacted) {
				if (_walkX == -1) {
					g_vm->getQSystem()->_petka->walk(msg.arg1, msg.arg2);

				} else {
					g_vm->getQSystem()->_petka->walk(_walkX, _walkY);
				}
			}
			break;
		case kWalkTo: {
			int destX = msg.arg1;
			int destY = msg.arg2;
			if (destX == -1 || destY  == -1) {
				destX = _walkX;
				destY = _walkY;
			}
			if (destX != -1) {
				g_vm->getQSystem()->_petka->walk(destX, destY);
				QReaction *r = g_vm->getQSystem()->_petka->_heroReaction;
				if (r) {
					for (uint i = 0; i < r->messages.size(); ++i) {
						if (r->messages[i].opcode == kGoTo) {
							g_vm->getQSystem()->_chapayev->walk(_walkX, _walkY);
							break;
						}
					}
				}
			}
			break;
		}
		case kWalkVich:
			if (msg.arg1 == 0xffff || msg.arg2 == -1) {
				g_vm->getQSystem()->_chapayev->walk(msg.arg1, msg.arg2);
			} else if (_walkX != -1) {
				g_vm->getQSystem()->_chapayev->walk(_walkX, _walkY);
			}
			break;
		case kDescription: {
			Common::ScopedPtr<Common::SeekableReadStream> invStream(g_vm->openFile("invntr.txt", false));
			if (invStream) {
				Common::String desc;
				Common::INIFile invIni;

				invIni.allowNonEnglishCharacters();
				invIni.loadFromStream(*invStream);
				invIni.getKey(_name, "ALL", desc);

				g_vm->getQSystem()->_mainInterface->setTextDescription(Common::convertToU32String(desc.c_str(), Common::kWindows1251), msg.arg1);
			}
			break;
		}
		default:
			debug("Opcode %d is not implemented", msg.opcode);
			break;
		}
	} else {
		for (uint i = 0; i < _reactions.size(); ++i) {
			QReaction &r = _reactions[i];
			if (r.opcode != msg.opcode ||
				(r.status != -1 && r.status != _status) ||
				(r.senderId != -1 && r.senderId != msg.sender->_id)) {
				continue;
			}
			g_vm->getQSystem()->_mainInterface->_dialog.setReaction(createReaction(r.messages.data(), r.messages.end()));
		}
		g_vm->getBigDialogue()->setHandler(_id, msg.opcode);
		g_vm->getQSystem()->_mainInterface->_dialog.start(msg.arg1, this);
	}

}

void QMessageObject::show(bool v) {
	_isShown = v;
}

void QMessageObject::setReaction(int16 id, QReaction *reaction) {
	delete _reaction;
	_reaction = reaction;
	_reactionId = id;
}

QObject::QObject() {
	_animate = true;
	_updateZ = true;
	_frame = 1;
	_sound = nullptr;
	_x = 0;
	_y = 0;
	_walkX = -1;
	_walkY = -1;
}

bool QObject::isInPoint(int x, int y) {
	if (!_isActive)
		return false;
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc) {
		if (!flc->getBounds().contains(x - _x, y - _y))
			return false;
		const Graphics::Surface *s = flc->getCurrentFrame();
		if (s->format.bytesPerPixel == 1)
			return *(const byte*)flc->getCurrentFrame()->getBasePtr(x - _x - flc->getPos().x, y - _y - flc->getPos().y) != 0;
		if (s->format.bytesPerPixel == 2)
			return *(const uint16*)flc->getCurrentFrame()->getBasePtr(x - _x - flc->getPos().x, y - _y - flc->getPos().y) != flc->getTransColor(s->format);
	}
	return false;
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
		}
		_startSound = false;
	}

	Common::Rect screen(640, 480);
	Common::Rect dest(flc->getBounds());
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
		srcRect.translate(-_x - flc->getPos().x, -_y - flc->getPos().y);
		g_vm->videoSystem()->transBlitFrom(*s, srcRect, destRect, flc->getTransColor(s->format));
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
	if (g_vm->getQSystem()->_mainInterface->findObject(_resourceId)) {
		FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
		if (flc) {
			g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
		}
	}
	QMessageObject::show(v);
}

void QObject::update(int time) {
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

void QObject::setPos(int x, int y) {
	FlicDecoder *flc = g_vm->resMgr()->loadFlic(_resourceId);
	if (flc) {
		g_vm->videoSystem()->addDirtyMskRects(Common::Point(_x, _y), *flc);
		g_vm->videoSystem()->addDirtyMskRects(Common::Point(x, y), *flc);
		_x = x;
		_y = y;
	}
}

void QObject::onClick(int x, int y) {
	QObjectCursor *cursor = g_vm->getQSystem()->_cursor.get();
	switch (cursor->_actionType) {
	case kActionLook:
		g_vm->getQSystem()->addMessage(_id, kLook, 0, 0, 0, 0, this);
		break;
	case kActionWalk:
		g_vm->getQSystem()->addMessage(_id, kWalk, x, y, 0, 0, this);
		break;
	case kActionUse:
		g_vm->getQSystem()->addMessage(_id, kUse, 0, 0, 0, 0, this);
		break;
	case kActionTake:
		g_vm->getQSystem()->addMessage(_id, kTake, 0, 0, 0, 0, this);
		break;
	case kActionTalk:
		g_vm->getQSystem()->addMessage(_id, kTalk, 0, 0, 0, 0, this);
		break;
	case kActionObjUse:
		g_vm->getQSystem()->addMessage(_id, kObjectUse, x, y, 0, 0, g_vm->getQSystem()->_chapayev.get());
		break;
	case kActionObjUseChapayev:
		g_vm->getQSystem()->addMessage(_id, kObjectUse, 0, 0, 0, 0, cursor->_invObj);
		break;
	default:
		break;
	}
}

void QObject::onMouseMove(int x, int y) {
	g_vm->getQSystem()->_mainInterface->_objUnderCursor = this;
}

}
