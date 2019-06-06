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

#include "graphics/colormasks.h"

#include "petka/petka.h"
#include "petka/q_system.h"
#include "petka/q_message_object.h"

namespace Petka {

QVisibleObject::QVisibleObject()
	: _resourceId(-1), _z(240) {}

QMessageObject::QMessageObject()
	: _id(-1), _dialogColor(-1) {}

static Common::String readString(Common::ReadStream &readStream) {
	uint32 stringSize = readStream.readUint32LE();
	byte *data = (byte *)malloc(stringSize + 1);
	readStream.read(data, stringSize);
	data[stringSize] = '\0';
	Common::String str((char *)data);
	return str;
}

void QMessageObject::deserialize(Common::SeekableReadStream &stream, const Common::INIFile &namesIni, const Common::INIFile &castIni) {
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

	namesIni.getKey(_name, "all", _nameOnScreen);

	Common::String rgbString;
	if (castIni.getKey(_name, "all", rgbString)) {
		int r, g, b;
		sscanf(rgbString.c_str(), "%d %d %d", &r, &g, &b);
		_dialogColor = Graphics::RGBToColor<Graphics::ColorMasks<888>>((byte)r, (byte)g, (byte)b);
	}
}

uint16 QMessageObject::getId() const {
	return _id;
}

const Common::String &QMessageObject::getName() const {
	return _name;
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

void QMessageObject::readFromBackgrndBg(Common::SeekableReadStream &stream) {
	_x = stream.readSint32LE();
	_y = stream.readSint32LE();
	_z = stream.readSint32LE();
	_field14 = stream.readSint32LE();
	_field18 = stream.readSint32LE();
}

} // End of namespace Petka
