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

#include "graphics/colormasks.h"
#include "graphics/surface.h"

#include "petka/petka.h"
#include "petka/video.h"
#include "petka/q_system.h"
#include "petka/q_manager.h"
#include "petka/obj.h"

namespace Petka {

QVisibleObject::QVisibleObject()
	: _resourceId(-1), _z(240) {}

QMessageObject::QMessageObject()
	: _id(-1), _dialogColor(-1) {}

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
			if (r.opcode == kCheck && g_vm->getQSystem()->findObject(rMsg.objId)->_status != rMsg.arg1) {
				break;
			}
			if (rMsg.opcode == kIf &&
			(rMsg.arg1 == -1 || rMsg.arg1 != msg.arg1) &&
			(rMsg.arg2 == -1 || rMsg.arg2 != msg.arg2) &&
			(rMsg.arg3 == -1 || rMsg.arg3 != msg.arg3)) {
				break;
			}
			if (rMsg.opcode == kRandom && rMsg.arg2 != -1) {
				rMsg.arg1 = (int16)g_vm->getRnd().getRandomNumber((uint)(rMsg.arg2 - 1));
			}
			g_vm->getQSystem()->addMessage(rMsg.objId, rMsg.opcode, rMsg.arg1, rMsg.arg2, rMsg.arg3, rMsg.unk, rMsg.sender);

			switch (rMsg.opcode) {
			case kPlay:
				break;
			case kWalk:
			case kWalkTo:
				break;
			case kWalkVich:
				break;
			}
		}
	}

	switch (msg.opcode) {
	case kSet:
		break;
	case kStatus:
		_status = (int8)msg.arg1;
		break;
	case kHide:
		break;
	case kZBuffer:
		break;
	case kPassive:
		break;
	}

}

void QObjectBG::processMessage(const QMessage &msg) {
	QMessageObject::processMessage(msg);
	switch (msg.opcode) {
	case kSet:
		_resourceId = msg.arg1;
	case kMusic:
		_musicId = msg.arg1;
		break;
	case kBGsFX:
		_fxId = msg.arg1;
		break;
	case kMap:
		_showMap = msg.arg1 != 0;
		break;
	case kNoMap:
		_showMap = 0;
		break;
	case kGoTo:
		break;
	case kSetSeq:
		break;
	case kEndSeq:
		break;
	}

}

void QObjectBG::draw() {
	Graphics::Surface *s = g_vm->resMgr()->loadBitmap(_resourceId);
	if (s) {
		const Common::List<Common::Rect> &dirty = g_vm->videoSystem()->rects();
		for (Common::List<Common::Rect>::const_iterator it = dirty.begin(); it != dirty.end(); ++it) {
			g_vm->videoSystem()->screen().blitFrom(*s, *it, Common::Point(it->top, it->left));
		}
	}
}

} // End of namespace Petka
