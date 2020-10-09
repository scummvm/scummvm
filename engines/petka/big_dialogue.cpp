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

#include "common/debug.h"
#include "common/stream.h"

#include "petka/base.h"
#include "petka/big_dialogue.h"
#include "petka/interfaces/main.h"
#include "petka/interfaces/dialog_interface.h"
#include "petka/objects/heroes.h"
#include "petka/q_system.h"
#include "petka/petka.h"

namespace Petka {

const uint16 kFallback = 0xFFFE;

BigDialogue::BigDialogue(PetkaEngine &vm)
	: _vm(vm) {
	_currOp = nullptr;
	_startOpIndex = 0;

	Common::ScopedPtr<Common::SeekableReadStream> file(vm.openFile("dialogue.fix", true));
	if (!file)
		return;

	_objDialogs.resize(file->readUint32LE());
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		_objDialogs[i].objId = file->readUint32LE();
		_objDialogs[i].handlers.resize(file->readUint32LE());
		file->skip(4); // pointer
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		for (uint j = 0; j < _objDialogs[i].handlers.size(); ++j) {
			_objDialogs[i].handlers[j].opcode = file->readUint32LE();
			_objDialogs[i].handlers[j].dialogs.resize(file->readUint32LE());
			_objDialogs[i].handlers[j].startDialogIndex = file->readUint32LE();
			file->skip(4); // pointer
		}
		for (uint j = 0; j < _objDialogs[i].handlers.size(); ++j) {
			for (uint z = 0; z < _objDialogs[i].handlers[j].dialogs.size(); ++z) {
				_objDialogs[i].handlers[j].dialogs[z].startOpIndex = file->readUint32LE();
				file->skip(4 + 4); // opsCount + pointer
			}
		}
	}

	load(file.get());
}

void BigDialogue::loadSpeechesInfo() {
	if (!_speeches.empty())
		return;

	Common::ScopedPtr<Common::SeekableReadStream> file(_vm.openFile("dialogue.lod", true));
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
		_speeches[i].text = Common::convertToU32String(curr, Common::kWindows1251);
		curr += strlen(curr) + 1;
	}
	delete[] str;
}

const Common::U32String *BigDialogue::getSpeechInfo(int *talkerId, const char **soundName, int choice) {
	if (!_currOp)
		return nullptr;
	switch (_currOp->type) {
	case kOperationMenu: {
		Operation *menuOp = _currOp;
		uint bit = 1;
		if (_currOp->menu.bits <= choice || choice < 0) {
			break;
		}
		while (true) {
			_currOp += 1;
			if (choice == 0 && (bit & menuOp->menu.bitField))
				break;
			if (_currOp->type == kOperationBreak) {
				if (bit & menuOp->menu.bitField)
					choice--;
				bit *= 2;
			}
		}
		if (_currOp->type != kOperationPlay)
			next();
		if (_currOp->type != kOperationPlay) {
			_currOp = menuOp;
			break;
		}

		uint index = _currOp->play.messageIndex;
		_currOp = menuOp;
		if (soundName)
			*soundName = _speeches[index].soundName;
		*talkerId = _speeches[index].speakerId;
		return &_speeches[index].text;
	}
	case kOperationCircle:
		circleMoveTo(_currOp->circle.curr);
		assert(_currOp->type == kOperationPlay);
		// fall through
	case kOperationPlay:
		if (soundName)
			*soundName = _speeches[_currOp->play.messageIndex].soundName;
		*talkerId = _speeches[_currOp->play.messageIndex].speakerId;
		return &_speeches[_currOp->play.messageIndex].text;
	default:
		break;
	}
	return nullptr;
}

const DialogHandler *BigDialogue::findHandler(uint objId, uint opcode, bool *fallback) const {
	if (opcode == kEnd || opcode == kHalf) {
		return nullptr;
	}
	if (fallback) {
		*fallback = false;
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		if (_objDialogs[i].objId != objId) {
			continue;
		}

		for (uint j = 0; j < _objDialogs[i].handlers.size(); ++j) {
			if (_objDialogs[i].handlers[j].opcode == opcode) {
				return &_objDialogs[i].handlers[j];
			}
		}

		if ((uint16)opcode != kObjectUse) {
			continue;
		}

		for (uint j = 0; j < _objDialogs[i].handlers.size(); ++j) {
			if (_objDialogs[i].handlers[j].opcode == kFallback) {
				if (fallback)
					*fallback = true;
				return &_objDialogs[i].handlers[j];
			}
		}
	}
	for (uint i = 0; i < _objDialogs.size(); ++i) {
		if (_objDialogs[i].objId != kFallback)
			continue;
		for (uint j = 0; j < _objDialogs[i].handlers.size(); ++j) {
			if (_objDialogs[i].handlers[j].opcode == opcode) {
				if (fallback)
					*fallback = true;
				return &_objDialogs[i].handlers[j];
			}
		}
	}
	return nullptr;
}

void BigDialogue::setHandler(uint objId, uint opcode) {
	loadSpeechesInfo();
	const DialogHandler *h = findHandler(objId, opcode, nullptr);
	if (h) {
		_startOpIndex = h->dialogs[h->startDialogIndex].startOpIndex;
		_currOp = &_ops[_startOpIndex];
	}
}

uint BigDialogue::opcode() {
	while (_currOp) {
		switch (_currOp->type) {
		case kOperationMenu:
			if (choicesCount() > 1)
				return kOpcodeMenu;
			next(0);
			break;
		case kOperationReturn:
			return kOpcodeEnd;
		case kOperationPlay:
		case kOperationCircle:
			return kOpcodePlay;
		case kOperationUserMessage:
			return kOpcodeUserMessage;
		default:
			next();
			break;
		}
	}
	return kOpcodeEnd;
}

void BigDialogue::load(Common::ReadStream *s) {
	uint32 opsCount = s->readUint32LE();
	_ops.resize(opsCount);
	for (uint i = 0; i < opsCount; ++i) {
		uint op = s->readUint32LE();
		_ops[i].type = (byte)(op >> 24);
		switch (_ops[i].type) {
		case kOperationBreak:
			break;
		case kOperationMenu:
			_ops[i].menu.bits = (byte)op;
			_ops[i].menu.bitField = (uint16)(op >> 8);
			break;
		case kOperationGoTo:
			_ops[i].goTo.opIndex = (uint16)(op);
			break;
		case kOperationDisableMenuItem:
			_ops[i].disableMenuItem.opIndex = (uint16)op;
			_ops[i].disableMenuItem.bit = (byte)(op >> 16);
			break;
		case kOperationEnableMenuItem:
			_ops[i].enableMenuItem.opIndex = (uint16)op;
			_ops[i].enableMenuItem.bit = (byte)(op >> 16);
			break;
		case kOperationReturn:
			break;
		case kOperationPlay:
			_ops[i].play.messageIndex = (uint16)op;
			break;
		case kOperationCircle:
			_ops[i].circle.count = (uint16)op;
			_ops[i].circle.curr = (byte)(op >> 16);
			break;
		case kOperationUserMessage:
			_ops[i].userMsg.arg = (uint16)op;
			break;
		default:
			break;
		}
	}
}

void BigDialogue::save(Common::WriteStream *s) {
	s->writeUint32LE(_ops.size());
	for (uint i = 0; i < _ops.size(); ++i) {
		switch (_ops[i].type) {
		case kOperationBreak:
			s->writeUint32LE(MKTAG(kOperationBreak, 0, 0, 0));
			break;
		case kOperationMenu:
			s->writeByte(_ops[i].menu.bits);
			s->writeUint16LE(_ops[i].menu.bitField);
			s->writeByte(kOperationMenu);
			break;
		case kOperationGoTo:
			s->writeUint16LE(_ops[i].goTo.opIndex);
			s->writeUint16LE(MKTAG16(kOperationGoTo, 0));
			break;
		case kOperationDisableMenuItem:
			s->writeUint16LE(_ops[i].disableMenuItem.opIndex);
			s->writeUint16LE(MKTAG16(kOperationDisableMenuItem, _ops[i].disableMenuItem.bit));
			break;
		case kOperationEnableMenuItem:
			s->writeUint16LE(_ops[i].enableMenuItem.opIndex);
			s->writeUint16LE(MKTAG16(kOperationEnableMenuItem, _ops[i].enableMenuItem.bit));
			break;
		case kOperationReturn:
			s->writeUint32LE(MKTAG(kOperationReturn, 0, 0, 0));
			break;
		case kOperationPlay:
			s->writeUint16LE(_ops[i].play.messageIndex);
			s->writeUint16LE(MKTAG16(kOperationPlay, 0));
			break;
		case kOperationCircle:
			s->writeUint16LE(_ops[i].circle.count);
			s->writeUint16LE(MKTAG16(kOperationCircle, _ops[i].circle.curr));
			break;
		case kOperationUserMessage:
			s->writeUint16LE(_ops[i].userMsg.arg);
			s->writeUint16LE(MKTAG16(kOperationUserMessage, 0));
			break;
		default:
			break;
		}
	}
}

void BigDialogue::next(int choice) {
	bool processed = true;

	if (!_currOp)
		return;
	if (choice != -1 && _currOp->type != kOperationMenu) {
		choice = -1;
	}

	while (true) {
		switch (_currOp->type) {
		case kOperationBreak:
			while (_currOp->type != kOperationMenu && _currOp->type != kOperationCircle) {
				_currOp--;
			}
			next(choice);
			return;
		case kOperationMenu: {
			if (!processed)
				return;
			if (choice == -1)
				choice = 0;
			if (_currOp->menu.bits <= choice)
				choice = _currOp->menu.bits - 1;
			uint16 bitField = _currOp->menu.bitField;
			uint bits = _currOp->menu.bits;
			uint bit = 1;
			uint i = 0;
			while (bits) {
				if (_currOp->type == kOperationBreak) {
					bits--;
					if (!(bit & bitField) && i <= (uint)choice)
						choice++;
					bit *= 2;
					i++;
				}
				_currOp += 1;
			}
			_currOp += choice;
			processed = false;
			break;
		}
		case kOperationGoTo: {
			_currOp = &_ops[_currOp->goTo.opIndex];
			processed = false;
			break;
		}
		case kOperationDisableMenuItem:
			_ops[_currOp->disableMenuItem.opIndex].menu.bitField &= ~(1 << _currOp->disableMenuItem.bit); // disable menu item
			checkMenu(_startOpIndex);
			_currOp += 1;
			processed = false;
			break;
		case kOperationEnableMenuItem:
			_ops[_currOp->enableMenuItem.opIndex].menu.bitField |= (1 << _currOp->enableMenuItem.bit);
			_currOp += 1;
			processed = false;
			break;
		case kOperationReturn:
			return;
		case kOperationPlay:
			if (!processed)
				return;
			_currOp += 1;
			processed = false;
			break;
		case kOperationCircle:
			if (!processed)
				return;
			_currOp->circle.curr = (byte)((_currOp->circle.curr + 1) % _currOp->circle.count);
			circleMoveTo(_currOp->circle.count);
			processed = false;
			break;
		case kOperationUserMessage:
			if (processed)
				_currOp += 1;
			else {
				_vm.getQSystem()->_mainInterface->_dialog.startUserMsg(_currOp->userMsg.arg);
			}
			return;
		default:
			_currOp += 1;
			processed = false;
			break;
		}
	}

}

uint BigDialogue::choicesCount() {
	if (!_currOp || _currOp->type != kOperationMenu)
		return 0;
	uint count = 0;
	uint bit = 1;
	for (uint i = 0; i < _currOp->menu.bits; ++i) {
		if (_currOp->menu.bitField & bit) {
			count++;
		}
		bit <<= 1;
	}
	return count;
}

bool BigDialogue::findOperation(uint index, uint opType, uint *resIndex) {
	while (_ops[index].type != opType) {
		switch(_ops[index].type) {
		case kOperationGoTo:
			if (index >= _ops[index].goTo.opIndex)
				return false;
			index = _ops[index].goTo.opIndex;
			break;
		case kOperationReturn:
			return false;
		default:
			index++;
			break;
		}
	}
	*resIndex = index;
	return true;
}

bool BigDialogue::checkMenu(uint menuIndex) {
	if (_ops[menuIndex].type != kOperationMenu && !findOperation(menuIndex, kOperationMenu, &menuIndex)) {
		return true;
	}

	uint count = 0;
	uint bit = 1;
	uint opIndex = menuIndex + 1;
	for (uint i = 0; i < _ops[menuIndex].menu.bits; ++i) {
		if (_ops[menuIndex].menu.bitField & bit) {
			count++;
		}
		findOperation(opIndex, kOperationBreak, &opIndex);
		opIndex++;
		bit <<= 1;
	}

	if (!count)
		return false;

	bit = 1;
	for (uint i = 0; i < _ops[menuIndex].menu.bits; ++i) {
		uint subMenuIndex;
		if ((_ops[menuIndex].menu.bitField & bit) && findOperation(_ops[opIndex + i].goTo.opIndex, kOperationMenu, &subMenuIndex) && !checkMenu(subMenuIndex)) {
			_ops[menuIndex].menu.bitField &= ~bit;
			count--;
			if (count < 1)
				return false;
		}
		bit <<= 1;
	}
	return true;
}

void BigDialogue::getMenuChoices(Common::Array<Common::U32String> &choices) {
	uint count = choicesCount();
	for (uint i = 0; i < count; ++i) {
		int id;
		choices.push_back(*getSpeechInfo(&id, nullptr, i));
	}
}

void BigDialogue::circleMoveTo(byte index) {
	_currOp += 1;
	for (uint i = 0; i < index; ++i) {
		while (_currOp->type != kOperationBreak)
			_currOp += 1;
		_currOp += 1;
	}
}

} // End of namespace Petka
