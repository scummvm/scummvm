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

#include "common/stream.h"

#include "petka/base.h"
#include "petka/big_dialogue.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/dialog_interface.h"
#include "petka/objects/heroes.h"
#include "petka/q_system.h"
#include "petka/petka.h"

namespace Petka {

BigDialogue::BigDialogue() {
	_ip = nullptr;
	_code = nullptr;
	_codeSize = 0;
	_startCodeIndex = 0;

	Common::ScopedPtr<Common::SeekableReadStream> file(g_vm->openFile("dialogue.fix", true));
	if (!file)
		return;

	_objDialogs.resize(file->readUint32LE());
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		_objDialogs[i].objId = file->readUint32LE();
		_objDialogs[i].dialogs.resize(file->readUint32LE());
		file->skip(4); // pointer
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
			_objDialogs[i].dialogs[j].opcode = file->readUint16LE();
			_objDialogs[i].dialogs[j].objId = file->readUint16LE();
			_objDialogs[i].dialogs[j].handlers.resize(file->readUint32LE());
			_objDialogs[i].dialogs[j].startHandlerIndex = file->readUint32LE();
			file->skip(4); // pointer
		}
		for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
			for (uint z = 0; z < _objDialogs[i].dialogs[j].handlers.size(); ++z) {
				_objDialogs[i].dialogs[j].handlers[z].startOpIndex = file->readUint32LE();
				_objDialogs[i].dialogs[j].handlers[z].opsCount = file->readUint32LE();
				file->skip(4); // pointer
			}
		}
		for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
			for (uint z = 0; z < _objDialogs[i].dialogs[j].handlers.size(); ++z) {
				file->skip(_objDialogs[i].dialogs[j].handlers[z].opsCount * 4); // operations
			}
		}
	}

	_codeSize = file->readUint32LE();
	_code = new int[_codeSize];
	file->read(_code, _codeSize * 4);
}

void BigDialogue::loadSpeechesInfo() {
	if (!_speeches.empty())
		return;

	Common::ScopedPtr<Common::SeekableReadStream> file(g_vm->openFile("dialogue.lod", true));
	if (!file)
		return;

	_speeches.resize(file->readUint32LE());
	for (uint i = 0; i < _speeches.size(); ++i) {
		_speeches[i].speakerId = file->readUint32LE();
		file->read(_speeches[i].soundName, sizeof(_speeches[i].soundName));
		file->skip(4);
	}

	char *str = new char[file->size() - file->pos()];
	char *curr = str;
	file->read(str, file->size() - file->pos());
	for (uint i = 0; i < _speeches.size(); ++i) {
		_speeches[i].text = Common::convertToU32String(curr, Common::CodePage::kWindows1251);
		curr += strlen(curr) + 1;
	}
	delete[] str;
}

const Common::U32String *BigDialogue::getSpeechInfo(int *talkerId, const char **soundName, int unk) {
	if (!_ip)
		return nullptr;
	int *oldIp = _ip;
	int op = *_ip;
	byte opcode = (byte)(*_ip >> 24);
	switch (opcode) {
	case 2: {
		int unk1 = 1;
		byte arg = (byte)*_ip;
		if (arg <= unk || unk >= 0) {
			break;
		}
		while (true) {
			_ip += 1;
			if (unk == 0 && (unk1 & ((op >> 8) & 0xFFFF)))
				break;
			if ((*_ip >> 24) == 0x1) {
				if (unk1 & ((op >> 8) & 0xFFFF))
					unk--;
				unk1 *= 2;
			}
		}
		if ((*_ip >> 24) != 0x7)
			sub40B670(-1);
		if ((*_ip >> 24) != 0x7) {
			_ip = oldIp;
			break;
		}
		uint index = (uint16)*_ip;
		_ip = oldIp;
		*soundName = _speeches[index].soundName;
		*talkerId = _speeches[index].speakerId;
		return &_speeches[index].text;
	}
	case 8:
		_ip += 1;
		for (uint i = 0; i < ((*_ip >> 16) & 0xFF); ++i) {
			while ((*_ip >> 24) != 0x01)
				_ip += 1;
			_ip += 1;
		}
		// fall through
	case 7:
		*soundName = _speeches[(uint16)*_ip].soundName;
		*talkerId = _speeches[(uint16)*_ip].speakerId;
		return &_speeches[(uint16)*_ip].text;
	default:
		break;
	}
	return nullptr;
}

const Dialog *BigDialogue::findDialog(uint objId, uint opcode, bool *res) const {
	if (opcode == kEnd || opcode == kHalf) {
		return nullptr;
	}
	if (res) {
		*res = 0;
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		if (_objDialogs[i].objId == objId) {
			for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
				if (_objDialogs[i].dialogs[j].opcode == opcode) {
					return &_objDialogs[i].dialogs[j];
				}
			}
			if (opcode != kObjectUse) {
				continue;
			}
			for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
				if (_objDialogs[i].dialogs[j].opcode == 0xFFFE) {
					if (res)
						*res = 1;
					return &_objDialogs[i].dialogs[j];
				}

			}
		}
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		if (_objDialogs[i].objId != 0xFFFE)
			continue;
		for (uint j = 0; j < _objDialogs[i].dialogs.size(); ++j) {
			if (_objDialogs[i].dialogs[j].opcode == opcode) {
				if (res)
					*res = 1;
				return &_objDialogs[i].dialogs[j];
			}
		}
	}
	return nullptr;
}

void BigDialogue::setDialog(uint objId, uint opcode, int index) {
	loadSpeechesInfo();
	const Dialog *d = findDialog(objId, opcode, nullptr);
	if (d) {
		if (index < 0 || index >= d->handlers.size()) {
			_ip = &_code[d->handlers[d->startHandlerIndex].startOpIndex];
			_startCodeIndex = d->handlers[d->startHandlerIndex].startOpIndex;
		} else {
			_ip = &_code[d->handlers[index].startOpIndex];
			_startCodeIndex = d->handlers[index].startOpIndex;
		}
	}
}

uint BigDialogue::opcode() {
	while (_ip) {
		byte op = (*_ip >> 24);
		switch (op) {
		case 2: {
			int unk1 = 0;
			int unk2 = 1;
			for (uint i = 0; i < (*_ip & 0xFF); ++i) {
				if (((*_ip >> 8) & 0xFFFF) & unk2) {
					unk1++;
				}
				unk2 <<= 1;
			}
			if (unk1 > 1)
				return 2;
			sub40B670(0);
			break;
		}
		case 6:
			return 3;
		case 7:
		case 8:
			return 1;
		case 9:
			return 4;
		default:
			sub40B670(-1);
			break;
		}
	}
	return 3;
}

void BigDialogue::sub40B670(int arg) {
	if (!_ip)
		return;
	int unk = 1;
	int unk2;
label:
	unk2 = arg;
	if (arg != -1 && (*_ip >> 24) != 2) {
		arg = -1;
	}
	while (true) {
		byte op = (*_ip >> 24);
		switch (op) {
		case 2: {
			if (!unk)
				return;
			if (unk2 < 0) {
				arg = 0;
				unk2 = 0;
			}
			int n = (byte)*_ip;
			if (n <= unk2)
				arg = (byte) *_ip - 1;
			int a1 = 0;
			int a2 = 1;
			int a3 = (*_ip >> 8) & 0xFFFF;
			for (int i = 0; i < n;) {
				if ((*_ip >> 24) == 10) {
					++i;
					if ((a3 & a2) && a1 <= arg)
						arg++;
					a2 *= 2;
					a1++;
				}
				_ip += 1;
			}
			_ip += arg;
			unk2 = arg;
			unk = 0;
			break;
		}
		case 3:
			_ip = &_code[*_ip & 0xFFFF];
			break;
		case 4:
			break;
		case 5: {
			_code[(*_ip >> 16)] |= (((1 << *(_ip) >> 24) & 0xFFFF) << 8);
			_ip += 1;
			unk = 0;
			break;
		}
		case 6:
			return;
		case 7:
			if (!unk)
				return;
			_ip += 1;
			unk = 0;
			break;
		case 8: {
			if (!unk)
				return;
			*_ip = (*_ip & 0xFF00FFFF) | ((byte)(((byte)(*_ip >> 16) + 1) % (int16)*_ip) << 16);
			int n = *_ip & 0xFF;
			for (int i = 0; i < n; ++i) {
				while ((*_ip >> 24) != 10)
					_ip += 1;
				_ip += 1;
			}
			unk = 0;
			break;
		}
		case 9:
			if (unk)
				_ip += 1;
			else {
				g_vm->getQSystem()->_mainInterface->_dialog.sendMsg(kSaid);
				g_vm->getQSystem()->_mainInterface->_dialog._field4 = 1;
				g_vm->getQSystem()->_mainInterface->_dialog.restoreCursorState();
				g_vm->getQSystem()->addMessage(g_vm->getQSystem()->_chapayev->_id, kUserMsg, *_ip);
			}
			return;
		case 10:
			do {
				if (op == 8) {
					break;
				}
				--_ip;
				op = (*_ip >> 24);
			} while (op != 2);
			unk = 1;
			goto label;
		default:
			_ip += 1;
			unk = 0;
			break;
		}
	}
}

} // End of namespace Petka