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
	_updateZ = false;
	_holdMessages = false;
	_loopedSound = false;
	_startSound = false;
	_reaction = nullptr;
}

void QMessageObject::processMessage(const QMessage &msg) {
	bool reacted = false;
	int opcode = (msg.opcode == kObjectUse) ? (msg.sender->_id << 16) | kObjectUse : msg.opcode;
	for (uint i = 0; i < _reactions.size(); ++i) {
		QReaction *r = &_reactions[i];
		if (r->opcode != msg.opcode ||
			(r->status != -1 && r->status != _status) ||
			(r->senderId != -1 && r->senderId != msg.sender->_id)) {
			continue;
		}
		bool fallback;
		if (g_vm->getBigDialogue()->findHandler(_id, opcode, &fallback) && !fallback) {
			g_vm->getBigDialogue()->setHandler(_id, opcode);
			g_vm->getQSystem()->_mainInterface->_dialog.setSender(this);
		}
		processReaction(r, &msg);
		reacted = true;
	}

	if (reacted || !g_vm->getBigDialogue()->findHandler(_id, opcode, nullptr)) {
		switch (msg.opcode) {
		case kAddInv:
			g_vm->getQSystem()->getCase()->addItem(msg.objId);
			break;
		case kDelInv:
			g_vm->getQSystem()->getCase()->removeItem(msg.objId);
			break;
		case kSetInv:
			g_vm->getQSystem()->getCase()->transformItem(msg.sender->_id, msg.objId);
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
			g_vm->getQSystem()->getCursor()->setInvItem(this, msg.arg1);
			g_vm->videoSystem()->makeAllDirty();
			break;
		case kDialog:
			g_vm->getQSystem()->_mainInterface->_dialog.start(msg.arg1, this);
			break;
		case kSetPos:
			setPos(Common::Point(msg.arg1, msg.arg2), false);
			break;
		case kSet:
		case kPlay:
			play(msg.arg1, msg.arg2);
			break;
		case kAnimate:
			_animate = msg.arg1;
			break;
		case kEnd:
			if (_reaction && _reactionId == msg.arg1) {
				QReaction *reaction = _reaction;
				_reaction = nullptr;
				processReaction(reaction);
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
			g_vm->getQSystem()->getCursor()->show(msg.arg1);
			g_vm->getQSystem()->getStar()->_isActive = msg.arg1;
			break;
		case kShow:
			show(msg.arg1);
			break;
		case kShake:
			g_vm->videoSystem()->setShake(msg.arg1);
			break;
		case kSystem:
			switch (msg.arg1){
			case 0:
				g_vm->getQSystem()->getStar()->_isActive = false;
				break;
			case 1:
				g_vm->getQSystem()->getStar()->_isActive = true;
				break;
			case 242:
				Engine::quitGame();
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
		case kJump: {
			Common::Point p;
			p.x = (msg.arg1 == 0xffff ? _walkX : msg.arg1);
			p.y = (msg.arg2 == -1 ? _walkY : msg.arg2);
			g_vm->getQSystem()->getPetka()->setPos(p, false);
			break;
		}
		case kJumpVich: {
			Common::Point p;
			p.x = (msg.arg1 == 0xffff ? _walkX : msg.arg1);
			p.y = (msg.arg2 == -1 ? _walkY : msg.arg2);
			g_vm->getQSystem()->getChapay()->setPos(p, false);
			break;
		}
		case kWalk:
			if (!reacted) {
				if (_walkX == -1) {
					g_vm->getQSystem()->getPetka()->walk(msg.arg1, msg.arg2);

				} else {
					g_vm->getQSystem()->getPetka()->walk(_walkX, _walkY);
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
				g_vm->getQSystem()->getPetka()->walk(destX, destY);
				QReaction *r = g_vm->getQSystem()->getPetka()->_heroReaction;
				if (r) {
					for (uint i = 0; i < r->messages.size(); ++i) {
						if (r->messages[i].opcode == kGoTo) {
							g_vm->getQSystem()->getChapay()->walk(destX, destY);
							break;
						}
					}
				}
			}
			break;
		}
		case kWalkVich: {
			int destX = msg.arg1;
			int destY = msg.arg2;
			if (destX == -1 || destY  == -1) {
				destX = _walkX;
				destY = _walkY;
			}
			if (destX != -1)
				g_vm->getQSystem()->getChapay()->walk(destX, destY);
			break;
		}
		case kDescription: {
			Common::ScopedPtr<Common::SeekableReadStream> invStream(g_vm->openFile("invntr.txt", true));
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
		case kPart:
			g_vm->loadPartAtNextFrame(msg.arg1);
			break;
		case kChapter:
			g_vm->loadChapter(msg.arg1);
			break;
		case kToMap:
			g_vm->getQSystem()->toggleMapInterface();
			break;
		default:
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
		g_vm->getBigDialogue()->setHandler(_id, opcode);
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

void QMessageObject::processReaction(QReaction *r, const QMessage *msg) {
	bool deleteReaction = (msg == nullptr);
	for (uint j = 0; j < r->messages.size(); ++j) {
		QMessage &rMsg = r->messages[j];
		if (rMsg.opcode == kCheck && g_vm->getQSystem()->findObject(rMsg.objId)->_status != rMsg.arg1) {
			break;
		}
		if (msg && rMsg.opcode == kIf &&
			((rMsg.arg1 != 0xffff && rMsg.arg1 != msg->arg1) ||
			 (rMsg.arg2 != -1 && rMsg.arg2 != msg->arg2) ||
			 (rMsg.arg3 != -1 && rMsg.arg3 != msg->arg3))) {
			break;
		}
		if (msg && rMsg.opcode == kRandom && rMsg.arg2 != -1) {
			rMsg.arg1 = (int16) g_vm->getRnd().getRandomNumber((uint) (rMsg.arg2 - 1));
		}
		g_vm->getQSystem()->addMessage(rMsg.objId, rMsg.opcode, rMsg.arg1, rMsg.arg2, rMsg.arg3, rMsg.unk, this);
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
			g_vm->getQSystem()->getPetka()->setReactionAfterWalk(j, r, this, deleteReaction);
			return;
		case kWalkVich:
			g_vm->getQSystem()->getChapay()->setReactionAfterWalk(j, r, this, deleteReaction);
			return;
		default:
			processed = false;
			break;
		}
		if (processed)
			break;
	}
	if (deleteReaction)
		delete r;
}

void QMessageObject::play(int id, int type) {
	if (g_vm->getQSystem()->_totalInit) {
		_resourceId = id;
		_loopedSound = (type == 5);
		return;
	}

	if (_loopedSound || g_vm->isDemo()) {
		removeSound();
	}

	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc) {
		g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
	}

	_resourceId = id;

	loadSound();

	flc = g_vm->resMgr()->getFlic(id);
	flc->setFrame(1);
	_time = 0;
	_loopedSound = (type == 5);
}

void QMessageObject::loadSound() {
	Common::String name = g_vm->resMgr()->findSoundName(_resourceId);
	_sound = g_vm->soundMgr()->addSound(name, Audio::Mixer::kSFXSoundType);
	_startSound = false;
}

void QMessageObject::removeSound() {
	Common::String name = g_vm->resMgr()->findSoundName(_resourceId);
	g_vm->soundMgr()->removeSound(name);
	_sound = nullptr;
}

static Common::String readString(Common::ReadStream &readStream) {
	uint32 stringSize = readStream.readUint32LE();
	byte *data = (byte *)malloc(stringSize + 1);
	readStream.read(data, stringSize);
	data[stringSize] = '\0';
	Common::String str((char *)data);
	free(data);
	return str;
}

void QMessageObject::readScriptData(Common::SeekableReadStream &stream) {
	_id = stream.readUint16LE();
	_name = readString(stream);
	_reactions.resize(stream.readUint32LE());

	for (uint i = 0; i < _reactions.size(); ++i) {
		QReaction *reaction = &_reactions[i];
		reaction->opcode = stream.readUint16LE();
		reaction->status = stream.readByte();
		reaction->senderId = stream.readUint16LE();
		reaction->messages.resize(stream.readUint32LE());
		for (uint j = 0; j < reaction->messages.size(); ++j) {
			QMessage *msg = &reaction->messages[j];
			msg->objId = stream.readUint16LE();
			msg->opcode = stream.readUint16LE();
			msg->arg1 = stream.readUint16LE();
			msg->arg2 = stream.readUint16LE();
			msg->arg3 = stream.readUint16LE();
		}
	}
}

void QMessageObject::readInisData(Common::INIFile &names, Common::INIFile &cast, Common::INIFile *bgs) {
	names.getKey(_name, "all", _nameOnScreen);
	Common::String rgbString;
	if (cast.getKey(_name, "all", rgbString)) {
		int r, g, b;
		sscanf(rgbString.c_str(), "%d %d %d", &r, &g, &b);
		_dialogColor = g_vm->_system->getScreenFormat().RGBToColor((byte)r, (byte)g, (byte)b);
	}
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

bool QObject::isInPoint(Common::Point p) {
	if (!_isActive)
		return false;

	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (!flc || !flc->getBounds().contains(p.x - _x, p.y - _y))
		return false;

	const Graphics::Surface *s = flc->getCurrentFrame();
	auto format = g_system->getScreenFormat();

	byte index = *(const byte *)s->getBasePtr(p.x - _x, p.y - _y);
	const byte *pal = flc->getPalette();

	return format.RGBToColor(pal[0], pal[1], pal[2]) != format.RGBToColor(pal[index * 3], pal[index * 3 + 1], pal[index * 3 + 2]);
}

void QObject::draw() {
	if (!_isShown || _resourceId == -1) {
		return;
	}
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (!flc) {
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

	int xOff = g_vm->getQSystem()->_xOffset;
	VideoSystem *videoSys = g_vm->videoSystem();

	Common::Rect screen(640 + xOff, 480);
	Common::Rect flcBounds(flc->getBounds());
	Common::Rect objBounds(flcBounds);

	objBounds.translate(_x, _y);

	Common::Rect intersect(screen.findIntersectingRect(objBounds));
	if (intersect.isEmpty())
		return;

	Graphics::Surface *surface = flc->getCurrentFrame()->getSubArea(flcBounds).convertTo(g_system->getScreenFormat(), flc->getPalette());

	for (Common::Rect dirty : videoSys->rects()) {
		dirty.translate(xOff, 0);

		Common::Rect destRect(intersect.findIntersectingRect(dirty));
		if (destRect.isEmpty())
			continue;

		Common::Rect srcRect(destRect);

		srcRect.translate(-_x, -_y);
		srcRect.translate(-flcBounds.left, -flcBounds.top);

		destRect.translate(-xOff, 0);
		videoSys->transBlitFrom(*surface, srcRect, destRect, flc->getTransColor(surface->format));
	}

	surface->free();
	delete surface;
}

void QObject::updateZ() {
	if (!_animate || !_isShown || !_updateZ)
		return;
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc) {
		_z = 1;
		const Common::Array<Common::Rect> &rects = flc->getMskRects();
		for (uint i = 0; i < rects.size(); ++i) {
			if (_y + rects[i].bottom > _z)
				_z = _y + rects[i].bottom;
		}

	}
}

void QObject::show(bool v) {
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc) {
		g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
	}
	QMessageObject::show(v);
}

void QObject::update(int time) {
	if (!_animate || !_isShown)
		return;
	_time += time;
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc && flc->getFrameCount() != 1) {
		if (_sound) {
			Common::Rect bounds = flc->getBounds();
			_sound->setBalance(bounds.left + bounds.width() / 2 - g_vm->getQSystem()->_xOffset, 640);
		}

		while (_time >= (int32)flc->getDelay()) {
			if (_sound && flc->getCurFrame() == 0) {
				_startSound = true;
			}
			g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
			flc->setFrame(-1);
			if (flc->getCurFrame() == (int32)flc->getFrameCount() - 1) {
				g_vm->getQSystem()->addMessage(_id, kEnd, _resourceId, 0, 0, 0, 0);
			}
			if (flc->getCurFrame() + 1 == (int32)flc->getFrameCount() / 2) {
				g_vm->getQSystem()->addMessage(_id, kHalf, _resourceId, 0, 0, 0, 0);
			}
			g_vm->videoSystem()->addDirtyRect(Common::Point(_x, _y), *flc);
			_time -= flc->getDelay();
		}
	}
}

void QObject::setPos(Common::Point p, bool) {
	FlicDecoder *flc = g_vm->resMgr()->getFlic(_resourceId);
	if (flc) {
		g_vm->videoSystem()->addDirtyMskRects(Common::Point(_x, _y), *flc);
		g_vm->videoSystem()->addDirtyMskRects(p, *flc);
		_x = p.x;
		_y = p.y;
	}
}

void QObject::onClick(Common::Point p) {
	QSystem *sys = g_vm->getQSystem();
	QObjectCursor *cursor = g_vm->getQSystem()->getCursor();

	sys->getPetka()->stopWalk();
	sys->getChapay()->stopWalk();

	switch (cursor->_actionType) {
	case kActionLook:
		g_vm->getQSystem()->addMessage(_id, kLook, 0, 0, 0, 0, this);
		break;
	case kActionWalk:
		g_vm->getQSystem()->addMessage(_id, kWalk, p.x, p.y, 0, 0, this);
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
	case kActionObjUseChapayev:
		g_vm->getQSystem()->addMessage(_id, kObjectUse, p.x, p.y, 0, 0, g_vm->getQSystem()->getChapay());
		break;
	case kActionObjUse:
		g_vm->getQSystem()->addMessage(_id, kObjectUse, 0, 0, 0, 0, cursor->_invObj);
		break;
	default:
		break;
	}
}

void QObject::onMouseMove(Common::Point p) {
	g_vm->getQSystem()->_mainInterface->_objUnderCursor = this;
}

}
