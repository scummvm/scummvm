/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "mohawk/livingbooks.h"

#include "common/memstream.h"
#include "common/textconsole.h"

namespace Mohawk {

// Not to be confused with SCRP as "scripts". SCRP is for LBAnimation

LBScriptEntry::LBScriptEntry() {
	state = 0;
	data = nullptr;
	argvParam = nullptr;
	argvTarget = nullptr;
}

LBScriptEntry::~LBScriptEntry() {
	delete[] argvParam;
	delete[] argvTarget;
	delete[] data;

	for (uint i = 0; i < subentries.size(); i++)
		delete subentries[i];
}

void LBItem::readFrom(Common::SeekableReadStreamEndian *stream) {
	_resourceId = stream->readUint16();
	_itemId = stream->readUint16();
	uint16 size = stream->readUint16();
	_desc = _vm->readString(stream);

	debug(2, "Item: size %d, resource %d, id %d", size, _resourceId, _itemId);
	debug(2, "Coords: %d, %d, %d, %d", _rect.left, _rect.top, _rect.right, _rect.bottom);
	debug(2, "String: '%s'", _desc.c_str());

	if (!_itemId)
		error("Item had invalid item id");

	int endPos = stream->pos() + size;
	if (endPos > stream->size())
		error("Item is larger (should end at %d) than stream (size %d)", endPos, (int)stream->size());

	while (true) {
		if (stream->pos() == endPos)
			break;

		uint oldPos = stream->pos();

		uint16 dataType = stream->readUint16();
		uint16 dataSize = stream->readUint16();

		debug(4, "Data type %04x, size %d", dataType, dataSize);
		byte *buf = new byte[dataSize];
		stream->read(buf, dataSize);
		readData(dataType, dataSize, buf);
		delete[] buf;

		if ((uint)stream->pos() != oldPos + 4 + (uint)dataSize)
			error("Failed to read correct number of bytes (off by %d) for data type %04x (size %d)",
				(int)stream->pos() - (int)(oldPos + 4 + (uint)dataSize), dataType, dataSize);

		if (stream->pos() > endPos)
			error("Read off the end (at %d) of data (ends at %d)", (int)stream->pos(), endPos);

		assert(!stream->eos());
	}
}

LBScriptEntry *LBItem::parseScriptEntry(uint16 type, uint16 &size, Common::MemoryReadStreamEndian *stream, bool isSubentry) {
	if (size < 6)
		error("Script entry of type 0x%04x was too small (%d)", type, size);

	uint16 expectedEndSize = 0;

	LBScriptEntry *entry = new LBScriptEntry;
	entry->type = type;
	if (isSubentry) {
		expectedEndSize = size - (stream->readUint16() + 2);
		entry->event = 0xffff;
	} else
		entry->event = stream->readUint16();
	entry->opcode = stream->readUint16();
	entry->param = stream->readUint16();
	debug(4, "Script entry: type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			entry->type, entry->event, entry->opcode, entry->param);
	size -= 6;

	// TODO: read as bytes, if this is correct (but beware endianism)
	byte conditionTag = (entry->event & 0xff00) >> 8;
	entry->event = entry->event & 0xff;

	if (type == kLBMsgListScript && entry->opcode == kLBOpRunSubentries) {
		debug(4, "%d script subentries:", entry->param);
		entry->argc = 0;
		for (uint i = 0; i < entry->param; i++) {
			LBScriptEntry *subentry = parseScriptEntry(type, size, stream, true);
			entry->subentries.push_back(subentry);

			// subentries are aligned
			if (i + 1 < entry->param && size % 2 == 1) {
				stream->skip(1);
				size--;
			}
		}
	} else if (type == kLBMsgListScript) {
		if (size < 2)
			error("Script entry of type 0x%04x was too small (%d)", type, size);

		entry->argc = stream->readUint16();
		size -= 2;

		entry->targetingType = 0;

		uint16 targetingType = entry->argc;
		if (targetingType == kTargetTypeExpression || targetingType == kTargetTypeCode
			|| targetingType == kTargetTypeName) {
			entry->targetingType = targetingType;

			// FIXME
			if (targetingType == kTargetTypeCode)
				error("encountered kTargetTypeCode");

			if (size < 2)
				error("not enough bytes (%d) reading special targeting", size);
			uint16 count = stream->readUint16();
			size -= 2;

			debug(4, "%d targets with targeting type %04x", count, targetingType);

			uint oldAlign = size % 2;
			for (uint i = 0; i < count; i++) {
				Common::String target = _vm->readString(stream);
				debug(4, "target '%s'", target.c_str());
				entry->targets.push_back(target);
				if (target.size() + 1 > size)
					error("failed to read target (ran out of stream)");
				size -= target.size() + 1;
			}
			entry->argc = entry->targets.size();

			if ((uint)(size % 2) != oldAlign) {
				stream->skip(1);
				size--;
			}
		} else if (entry->argc) {
			entry->argvParam = new uint16[entry->argc];
			entry->argvTarget = new uint16[entry->argc];
			debug(4, "With %d targets:", entry->argc);

			if (size < (entry->argc * 4))
				error("Script entry of type 0x%04x was too small (%d)", type, size);

			for (uint i = 0; i < entry->argc; i++) {
				entry->argvParam[i] = stream->readUint16();
				entry->argvTarget[i] = stream->readUint16();
				debug(4, "Target %d, param 0x%04x", entry->argvTarget[i], entry->argvParam[i]);
			}

			size -= (entry->argc * 4);
		}
	}

	if (type == kLBMsgListScript && entry->opcode == kLBOpJumpUnlessExpression) {
		if (size < 6)
			error("not enough bytes (%d) in kLBOpJumpUnlessExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		entry->target = stream->readUint16();
		debug(4, "kLBOpJumpUnlessExpression: offset %08x, target %d", entry->offset, entry->target);
		size -= 6;
	}
	if (type == kLBMsgListScript && entry->opcode == kLBOpJumpToExpression) {
		if (size < 4)
			error("not enough bytes (%d) in kLBOpJumpToExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		debug(4, "kLBOpJumpToExpression: offset %08x", entry->offset);
		size -= 4;
	}

	if (type == kLBNotifyScript && entry->opcode == kLBNotifyChangeMode && _vm->getGameType() != GType_LIVINGBOOKSV1) {
		switch (entry->param) {
		case 1:
			if (size < 8)
				error("%d unknown bytes in notify entry kLBNotifyChangeMode", size);
			entry->newUnknown = stream->readUint16();
			entry->newMode = stream->readUint16();
			entry->newPage = stream->readUint16();
			entry->newSubpage = stream->readUint16();
			debug(4, "kLBNotifyChangeMode: unknown %04x, mode %d, page %d.%d",
				entry->newUnknown, entry->newMode, entry->newPage, entry->newSubpage);
			size -= 8;
			break;
		case 3:
			{
			Common::String newCursor = _vm->readString(stream);
			entry->newCursor = newCursor;
			if (size < newCursor.size() + 1)
				error("failed to read newCursor in notify entry");
			size -= newCursor.size() + 1;
			debug(4, "kLBNotifyChangeMode: new cursor '%s'", newCursor.c_str());
			}
			break;
		default:
			// the original engine also does something when param==2 (but not a notify)
			error("unknown v2 kLBNotifyChangeMode type %d", entry->param);
		}
	}
	if (entry->opcode == kLBOpSendExpression) {
		if (size < 4)
			error("not enough bytes (%d) in kLBOpSendExpression, event 0x%04x", size, entry->event);
		entry->offset = stream->readUint32();
		debug(4, "kLBOpSendExpression: offset %08x", entry->offset);
		size -= 4;
	}
	if (entry->opcode == kLBOpRunData) {
		if (size < 4)
			error("didn't get enough bytes (%d) to read data header in script entry", size);
		entry->dataType = stream->readUint16();
		entry->dataLen = stream->readUint16();
		size -= 4;

		if (size < entry->dataLen)
			error("didn't get enough bytes (%d) to read data in script entry", size);

		if (entry->dataType == kLBCommand) {
			Common::String command = _vm->readString(stream);
			uint commandSize = command.size() + 1;
			if (commandSize > entry->dataLen)
				error("failed to read command in script entry: dataLen %d, command '%s' (%d chars)",
					 entry->dataLen, command.c_str(), commandSize);
			entry->dataLen = commandSize;
			entry->data = new byte[commandSize];
			memcpy(entry->data, command.c_str(), commandSize);
			size -= commandSize;
		} else {
			if (conditionTag)
				error("kLBOpRunData had unexpected conditionTag");
			entry->data = new byte[entry->dataLen];
			stream->read(entry->data, entry->dataLen);
			size -= entry->dataLen;
		}
	}
	if (entry->event == kLBEventNotified) {
		if (size < 4)
			error("not enough bytes (%d) in kLBEventNotified, opcode 0x%04x", size, entry->opcode);
		entry->matchFrom = stream->readUint16();
		entry->matchNotify = stream->readUint16();
		debug(4, "kLBEventNotified: matches %04x (from %04x)",
			entry->matchNotify, entry->matchFrom);
		size -= 4;
	}

	if (isSubentry) {
		// TODO: subentries may be aligned, so this check is a bit too relaxed
		if (size != expectedEndSize && size != expectedEndSize + 1)
			error("expected %d bytes left at end of subentry, but had %d",
				expectedEndSize, size);
		return entry;
	}

	if (conditionTag == 1) {
		if (!size)
			error("failed to read condition (empty stream)");
		Common::String condition = _vm->readString(stream);
		if (condition.size() == 0) {
			size--;
			if (!size)
				error("failed to read condition (null byte, then ran out of stream)");
			condition = _vm->readString(stream);
		}
		if (condition.size() + 1 > size)
			error("failed to read condition (ran out of stream)");
		size -= (condition.size() + 1);

		entry->conditions.push_back(condition);
		debug(4, "script entry condition '%s'", condition.c_str());
	} else if (conditionTag == 2) {
		if (size < 4)
			error("expected more than %d bytes for conditionTag 2", size);
		// FIXME
		stream->skip(4);
		size -= 4;
	}

	if (size == 1) {
		// FIXME: this is alignment, but why?
		stream->skip(1);
		size--;
	} else if (size)
		error("failed to read script entry correctly (%d bytes left): type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			size, entry->type, entry->event, entry->opcode, entry->param);

	return entry;
}

void LBItem::readData(uint16 type, uint16 size, byte *data) {
	Common::MemoryReadStreamEndian stream(data, size, _vm->isBigEndian());
	readData(type, size, &stream);
}

void LBItem::readData(uint16 type, uint16 size, Common::MemoryReadStreamEndian *stream) {
	switch (type) {
	case kLBMsgListScript:
	case kLBNotifyScript:
		_scriptEntries.push_back(parseScriptEntry(type, size, stream));
		break;

	case kLBSetPlayInfo:
		{
		if (size != 20)
			error("kLBSetPlayInfo had wrong size (%d)", size);

		_loopMode = stream->readUint16();
		_delayMin = stream->readUint16();
		_delayMax = stream->readUint16();
		_timingMode = stream->readUint16();
		if (_timingMode > 7)
			error("encountered timing mode %04x", _timingMode);
		_periodMin = stream->readUint16();
		_periodMax = stream->readUint16();
		_relocPoint.x = stream->readSint16();
		_relocPoint.y = stream->readSint16();
		_controlMode = stream->readUint16();
		_soundMode = stream->readUint16();

		debug(2, "kLBSetPlayInfo: loop mode %d (%d to %d), timing mode %d (%d to %d), reloc (%d, %d), control mode %04x, sound mode %04x",
			_loopMode, _delayMin, _delayMax,
			_timingMode, _periodMin, _periodMax,
			_relocPoint.x, _relocPoint.y,
			_controlMode, _soundMode);
		}
		break;

	case kLBSetPlayPhase:
		if (size != 2)
			error("SetPlayPhase had wrong size (%d)", size);
		_phase = stream->readUint16();
		debug(2, "kLBSetPlayPhase: %d", _phase);
		break;

	case kLBSetKeyNotify:
		{
		// FIXME: variable-size notifies, targets
		if (size != 18)
			error("0x6f had wrong size (%d)", size);
		uint event = stream->readUint16();
		LBKey key;
		stream->read(&key, 4);
		uint opcode = stream->readUint16();
		uint param = stream->readUint16();
		uint u6 = stream->readUint16();
		uint u7 = stream->readUint16();
		uint u8 = stream->readUint16();
		uint u9 = stream->readUint16();
		warning("ignoring kLBSetKeyNotify: item %s, key code %02x (modifier mask %d, char %d, repeat %d), event %04x, opcode %04x, param %04x, unknowns %04x, %04x, %04x, %04x",
			_desc.c_str(), key.code, key.modifiers, key.char_, key.repeats, event, opcode, param, u6, u7, u8, u9);
		}
		break;

	case kLBCommand:
		{
			Common::String command = _vm->readString(stream);
			if (size != command.size() + 1)
				error("failed to read command string");

			runCommand(command);
		}
		break;

	case kLBSetNotVisible:
		assert(size == 0);
		_visible = false;
		break;

	case kLBGlobalDisable:
		assert(size == 0);
		_globalEnabled = false;
		break;

	case kLBGlobalSetNotVisible:
		assert(size == 0);
		_globalVisible = false;
		break;

	case kLBSetAmbient:
		assert(size == 0);
		_isAmbient = true;
		break;

	case kLBSetKeyEvent:
		{
		// FIXME: targets
		if (size != 10)
			error("kLBSetKeyEvent had wrong size (%d)", size);
		uint u3 = stream->readUint16();
		LBKey key;
		stream->read(&key, 4);
		uint target = stream->readUint16();
		uint16 event = stream->readUint16();
		// FIXME: this is scripting stuff: what to run when key is pressed
		warning("ignoring kLBSetKeyEvent: item %s, key code %02x (modifier mask %d, char %d, repeat %d) unknown %04x, target %d, event %04x",
			_desc.c_str(), key.code, key.modifiers, key.char_, key.repeats, u3, target, event);
		}
		break;

	case kLBSetHitTest:
		{
		assert(size == 2);
		uint val = stream->readUint16();
		_doHitTest = (bool)val;
		debug(2, "kLBSetHitTest (on %s): value %04x", _desc.c_str(), val);
		}
		break;

	case kLBSetRolloverData:
		{
		assert(size == 2);
		uint16 flag = stream->readUint16();
		warning("ignoring kLBSetRolloverData: item %s, flag %d", _desc.c_str(), flag);
		}
		break;

	case kLBSetParent:
		{
		assert(size == 2);
		uint16 parent = stream->readUint16();
		warning("ignoring kLBSetParent: item %s, parent id %d", _desc.c_str(), parent);
		}
		break;

	case kLBUnknown194:
		{
		assert(size == 4);
		uint offset = stream->readUint32();
		_page->_code->runCode(this, offset);
		}
		break;

	default:
		error("Unknown message %04x (size 0x%04x)", type, size);
		//for (uint i = 0; i < size; i++)
		//	debugN("%02x ", stream->readByte());
		//debugN("\n");
		break;
	}
}

void LBItem::notify(uint16 data, uint16 from) {
	if (_timingMode == kLBAutoSync) {
		// TODO: is this correct?
		if (_periodMin == data && _periodMax == from) {
			debug(2, "Handling notify 0x%04x (from %d)", data, from);
			setNextTime(0, 0);
		}
	}

	runScript(kLBEventNotified, data, from);
}

void LBItem::runScript(uint event, uint16 data, uint16 from) {
	for (uint i = 0; i < _scriptEntries.size(); i++) {
		LBScriptEntry *entry = _scriptEntries[i];

		if (entry->event != event)
			continue;

		if (event == kLBEventNotified) {
			if ((entry->matchFrom && entry->matchFrom != from) || entry->matchNotify != data)
				continue;
		}

		bool conditionsMatch = true;
		for (uint n = 0; n < entry->conditions.size(); n++) {
			if (!checkCondition(entry->conditions[n])) {
				conditionsMatch = false;
				break;
			}
		}
		if (!conditionsMatch)
			continue;

		if (entry->type == kLBNotifyScript) {
			debug(2, "Notify: event 0x%04x, opcode 0x%04x, param 0x%04x",
				entry->event, entry->opcode, entry->param);

			if (entry->opcode == kLBNotifyGUIAction)
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, _itemId));
			else if (entry->opcode == kLBNotifyChangeMode && _vm->getGameType() != GType_LIVINGBOOKSV1) {
				NotifyEvent notifyEvent(entry->opcode, entry->param);
				notifyEvent.newUnknown = entry->newUnknown;
				notifyEvent.newMode = entry->newMode;
				notifyEvent.newPage = entry->newPage;
				notifyEvent.newSubpage = entry->newSubpage;
				notifyEvent.newCursor = entry->newCursor;
				_vm->addNotifyEvent(notifyEvent);
			} else
				_vm->addNotifyEvent(NotifyEvent(entry->opcode, entry->param));
		} else
			runScriptEntry(entry);
	}
}

int LBItem::runScriptEntry(LBScriptEntry *entry) {
	if (entry->state == 0xffff)
		return 0;

	uint start = 0;
	uint count = entry->argc;
	// zero targets = apply to self
	if (!count)
		count = 1;

	if (entry->opcode != kLBOpRunSubentries) switch (entry->param) {
	case 0xfffe:
		// Run once (disable self after run).
		entry->state = 0xffff;
		break;
	case 0xffff:
		break;
	case 0:
	case 1:
	case 2:
		start = entry->state;
		entry->state++;
		if (entry->state >= count) {
			switch (entry->param) {
			case 0:
				// Disable..
				entry->state = 0xffff;
				return 0;
			case 1:
				// Stay at the end.
				entry->state = count - 1;
				break;
			case 2:
				// Loop.
				entry->state = 0;
				break;
			default:
				break;
			}
		}
		count = 1;
		break;
	case 3:
		// Pick random target.
		start = _vm->_rnd->getRandomNumberRng(0, count);
		count = 1;
		break;
	default:
		warning("Weird param for script entry (type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x)",
			entry->type, entry->event, entry->opcode, entry->param);
	}

	for (uint n = start; n < count; n++) {
		LBItem *target;

		debug(2, "Script run: type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x",
			entry->type, entry->event, entry->opcode, entry->param);

		if (entry->argc) {
			switch (entry->targetingType) {
			case kTargetTypeExpression:
				{
				// FIXME: this should be EVALUATED
				LBValue &tgt = _vm->_variables[entry->targets[n]];
				switch (tgt.type) {
				case kLBValueItemPtr:
					target = tgt.item;
					break;
				case kLBValueString:
					// FIXME: handle 'self', at least
					// TODO: correct otherwise? or only self?
					target = _vm->getItemByName(tgt.string);
					break;
				case kLBValueInteger:
					target = _vm->getItemById(tgt.integer);
					break;
				default:
					// FIXME: handle list
					warning("Target '%s' (by expression) resulted in unknown type, skipping", entry->targets[n].c_str());
					continue;
				}
				}
				if (!target) {
					debug(2, "Target '%s' (by expression) doesn't exist, skipping", entry->targets[n].c_str());
					continue;
				}
				debug(2, "Target: '%s' (expression '%s')", target->_desc.c_str(), entry->targets[n].c_str());
				break;
			case kTargetTypeCode:
				// FIXME
				error("encountered kTargetTypeCode");
				break;
			case kTargetTypeName:
				// FIXME: handle 'self'
				target = _vm->getItemByName(entry->targets[n]);
				if (!target) {
					debug(2, "Target '%s' (by name) doesn't exist, skipping", entry->targets[n].c_str());
					continue;
				}
				debug(2, "Target: '%s' (by name)", target->_desc.c_str());
				break;
			default:
				uint16 targetId = entry->argvTarget[n];
				// TODO: is this type, perhaps?
				uint16 param = entry->argvParam[n];
				target = _vm->getItemById(targetId);
				if (!target) {
					debug(2, "Target %04x (%04x) doesn't exist, skipping", targetId, param);
					continue;
				}
				debug(2, "Target: %04x (%04x) '%s'", targetId, param, target->_desc.c_str());
			}
		} else {
			target = this;
			debug(2, "Self-target on '%s'", _desc.c_str());
		}

		// an opcode in the form 0x1xx means to run the script for event 0xx
		if ((entry->opcode & 0xff00) == 0x0100) {
			// FIXME: pass on param
			target->runScript(entry->opcode & 0xff);
			break;
		}

		switch (entry->opcode) {
		case kLBOpNone:
			warning("ignoring kLBOpNone (event 0x%04x, param 0x%04x, target '%s')",
					entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpXShow:
			// TODO: should be setVisible(true) - not a delayed event -
			// when we're doing the param 1/2/3 stuff above?
			// and in modern LB this is perhaps just a direct target->setVisible(true)..
			if (_vm->getGameType() != GType_LIVINGBOOKSV1)
				warning("kLBOpXShow on '%s' is probably broken", target->_desc.c_str());
			_vm->queueDelayedEvent(DelayedEvent(this, kLBDelayedEventSetNotVisible));
			break;

		case kLBOpTogglePlay:
			target->togglePlaying(false, true);
			break;

		case kLBOpSetNotVisible:
			target->setVisible(false);
			break;

		case kLBOpSetVisible:
			target->setVisible(true);
			break;

		case kLBOpDestroy:
			target->destroySelf();
			break;

		case kLBOpRewind:
			target->seek(1);
			break;

		case kLBOpStop:
			target->stop();
			break;

		case kLBOpDisable:
			target->setEnabled(false);
			break;

		case kLBOpEnable:
			target->setEnabled(true);
			break;

		case kLBOpGlobalSetNotVisible:
			target->setGlobalVisible(false);
			break;

		case kLBOpGlobalSetVisible:
			target->setGlobalVisible(true);
			break;

		case kLBOpGlobalDisable:
			target->setGlobalEnabled(false);
			break;

		case kLBOpGlobalEnable:
			target->setGlobalEnabled(true);
			break;

		case kLBOpSeekToEnd:
			target->seek(0xFFFF);
			break;

		case kLBOpMute:
		case kLBOpUnmute:
			// FIXME
			warning("ignoring kLBOpMute/Unmute (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpLoad:
			target->load();
			break;

		case kLBOpPreload:
			// FIXME
			warning("ignoring kLBOpPreload (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpUnload:
			target->unload();
			break;

		case kLBOpSeekToPrev:
		case kLBOpSeekToNext:
			// FIXME
			warning("ignoring kLBOpSeekToPrev/Next (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpDragBegin:
		case kLBOpDragEnd:
			// FIXME
			warning("ignoring kLBOpDragBegin/End (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpScriptDisable:
		case kLBOpScriptEnable:
			// FIXME
			warning("ignoring kLBOpScriptDisable/Enable (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpUnknown1C:
			// FIXME
			warning("ignoring kLBOpUnknown1C (event 0x%04x, param 0x%04x, target '%s')",
				entry->event, entry->param, target->_desc.c_str());
			break;

		case kLBOpSendExpression:
			_page->_code->runCode(this, entry->offset);
			break;

		case kLBOpRunSubentries:
			for (uint i = 0; i < entry->subentries.size(); i++) {
				LBScriptEntry *subentry = entry->subentries[i];

				int e = runScriptEntry(subentry);

				switch (subentry->opcode) {
				case kLBOpJumpUnlessExpression:
					debug(2, "JumpUnless got %d (to %d, on %d, of %d)", e, subentry->target, i, entry->subentries.size());
					if (!e)
						i = subentry->target - 1;
					break;
				case kLBOpBreakExpression:
					debug(2, "BreakExpression");
					i = entry->subentries.size();
					break;
				case kLBOpJumpToExpression:
					debug(2, "JumpToExpression got %d (on %d, of %d)", e, i, entry->subentries.size());
					i = e - 1;
					break;
				default:
					break;
				}
			}
			break;

		case kLBOpRunData:
			readData(entry->dataType, entry->dataLen, entry->data);
			break;

		case kLBOpJumpUnlessExpression:
		case kLBOpBreakExpression:
		case kLBOpJumpToExpression:
			{
			LBValue r = _page->_code->runCode(this, entry->offset);
			// FIXME
			return r.integer;
			}

		default:
			error("Unknown script opcode (type 0x%04x, event 0x%04x, opcode 0x%04x, param 0x%04x, target '%s')",
				entry->type, entry->event, entry->opcode, entry->param, target->_desc.c_str());
		}
	}

	return 0;
}

void LBItem::runCommand(const Common::String &command) {
	LBCode tempCode(_vm, 0);

	debug(2, "running command '%s'", command.c_str());

	uint offset = tempCode.parseCode(command);
	tempCode.runCode(this, offset);
}

bool LBItem::checkCondition(const Common::String &condition) {
	LBCode tempCode(_vm, 0);

	debug(3, "checking condition '%s'", condition.c_str());

	uint offset = tempCode.parseCode(condition);
	LBValue result = tempCode.runCode(this, offset);

	return result.toInt();
}

} // End of namespace Mohawk
