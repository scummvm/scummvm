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

#include "petka/big_dialogue.h"
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
			for (uint z = 0; z < _objDialogs[i].dialogs[j].handlers.size(); ++z) {
				_objDialogs[i].dialogs[j].handlers[z].startOpIndex = file->readUint32LE();
				uint opsCount = file->readUint32LE();
				file->skip(4); // pointer
				file->skip(opsCount * 4); // operations
			}
		}
	}

	_codeSize = file->readUint32LE();
	_code = new int[_codeSize * 4];
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
	}

	char *str = new char[file->size() - file->pos()];
	char *curr = str;
	file->read(str, file->size() - file->pos());
	for (uint i = 0; i < _speeches.size(); ++i) {
		_speeches[i].text = Common::convertToU32String(curr);
		curr += strlen(curr) + 1;
	}
	delete[] str;
}

const SpeechInfo *BigDialogue::getSpeechInfo() {
	if (!_ip)
		return nullptr;
	byte opcode = (byte)(*_ip >> 24);
	switch (opcode) {
	case 2:
		// not implemented
		break;
	case 8:
		_ip += 1;
		for (uint i = 0; i < ((*_ip >> 16) & 0xFF); ++i) {
			while ((*_ip >> 24) != 0x01)
				_ip += 1;
			_ip += 1;
		}
		// fall through
	case 7:
		return &_speeches[(uint16)*_ip];
	default:
		break;
	}
	return nullptr;
}

const Dialog *BigDialogue::findDialog(uint objId, uint opcode, bool *res) const {
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

} // End of namespace Petka