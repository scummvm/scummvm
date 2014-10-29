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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions_duckman.h"
#include "illusions/talkthread_duckman.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/screentext.h"
#include "illusions/scriptman.h"
#include "illusions/talkresource.h"
#include "illusions/time.h"

namespace Illusions {

// TalkThread_Duckman

TalkThread_Duckman::TalkThread_Duckman(IllusionsEngine_Duckman *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	uint32 objectId, uint32 talkId, uint32 sequenceId1, uint32 sequenceId2)
	: Thread(vm, threadId, callingThreadId, notifyFlags), _vm(vm), _objectId(objectId), _talkId(talkId) {
	_type = kTTTalkThread;

	if ((sequenceId1 & 0xFFFF0000) == 0x60000) {
		_sequenceId1 = sequenceId1;
		_sequenceId2 = sequenceId2;
		_namedPointId1 = 0;
		_namedPointId2 = 0;
	} else {
		_sequenceId1 = 0;
		_sequenceId2 = 0;
		_namedPointId1 = sequenceId1;
		_namedPointId2 = sequenceId2;
	}

	if (_vm->checkActiveTalkThreads())
		_status = 1;
	else
		_status = 2;
		
	_durationMult = _vm->clipTextDuration(_vm->_fieldE);
	_textDuration = _durationMult;
	_defDurationMult = _vm->clipTextDuration(240);
	
	_tag = _vm->getCurrentScene();

}

int TalkThread_Duckman::onUpdate() {

	TalkEntry *talkEntry;

	switch (_status) {

	case 1:
		if (_vm->checkActiveTalkThreads())
			return kTSYield;
		_status = 3;
		// Fallthrough to status 2

	case 2:
		talkEntry = getTalkResourceEntry(_talkId);
		_flags = 0;
		_entryText = talkEntry->_text;
		_currEntryText = 0;
		_entryTblPtr = talkEntry->_tblPtr;
		_flags = 0;
		if (_sequenceId1) {
			_pauseCtr = 0;
			_pauseCtrPtr = &_pauseCtr;
		} else {
			_pauseCtrPtr = 0;
			_flags |= 2;
			_flags |= 1;
		}
		if (_vm->isSoundActive()) {
			if (!_vm->cueVoice(talkEntry->_voiceName) && !_durationMult)
				_durationMult = _defDurationMult;
		} else {
			_flags |= 4;
			if (!_durationMult)
				_durationMult = _defDurationMult;
		}
		if (_objectId == 0 || _durationMult == 0)
			_flags |= 8;
		_status = 3;
		// Fallthrough to status 3 

	case 3:
		if (!(_flags & 4) && !_vm->isVoiceCued())
			return kTSYield;
		_status = 4;
		// Fallthrough to status 4
		
	case 4:
		if (!(_flags & 8) ) {
			uint32 actorTypeId = _vm->getObjectActorTypeId(_objectId);
			// TODO getActorTypeColor(actorTypeId, &_colorR, &_colorG, &_colorB);
			refreshText();
		}
		if (!(_flags & 2)) {
			Control *control = _vm->_dict->getObjectControl(_objectId);
			control->startTalkActor(_sequenceId1, _entryTblPtr, _threadId);
		}
		if (!(_flags & 4)) {
			int16 panX = 0;
			if (_flags & 1) {
				if (_namedPointId2) {
					panX = _vm->getNamedPointPosition(_namedPointId2).x;
					panX = _vm->convertPanXCoord(panX);
				}
			} else {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				panX = control->getActorPosition().x;
				panX = _vm->convertPanXCoord(panX);
			}
			_vm->startVoice(255, panX);
		}
		_vm->_input->discardButtons(0x20);
		_status = 5;
		return kTSYield;

	case 5:
		if (!(_flags & 4) && !_vm->isVoicePlaying())
			_flags |= 4;
		if (!(_flags & 8) && isTimerExpired(_textStartTime, _textEndTime)) {
			_vm->_screenText->removeText();
			if (_entryText && *_entryText) {
				refreshText();
				_vm->_input->discardButtons(0x20);
			} else {
				_flags |= 8;
			}
		}
		if (!(_flags & 2)) {
			if (*_pauseCtrPtr < 0) {
				++(*_pauseCtrPtr);
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->startSequenceActor(_sequenceId2, 2, 0);
				_flags |= 2;
			}
		}
		if (_objectId && _vm->_input->pollButton(0x20)) {
			if (!(_flags & 8)) {
				_vm->_screenText->removeText();
				if (_entryText && *_entryText)
					refreshText();
				else
					_flags |= 8;
			}
			if (_flags & 8) {
				if (!(_flags & 4)) {
					_vm->stopVoice();
					_flags |= 4;
				}
				if (!(_flags & 2)) {
					Control *control = _vm->_dict->getObjectControl(_objectId);
					control->clearNotifyThreadId1();
					control->startSequenceActor(_sequenceId2, 2, 0);
					_flags |= 2;
				}
			}
		}
		if ((_flags & 8) && (_flags & 2) && (_flags & 4)) {
			_vm->_input->discardButtons(0x20);
			return kTSTerminate;
		}
		return kTSYield;

	case 6:
		if (!(_flags & 2)) {
			Control *control = _vm->_dict->getObjectControl(_objectId);
			if (*_pauseCtrPtr >= 0) {
				control->clearNotifyThreadId1();
			} else {
				++(*_pauseCtrPtr);
			}
			control->startSequenceActor(_sequenceId2, 2, 0);
			_flags |= 2;
		}
		return kTSTerminate;
		
	}

	return kTSTerminate;

}

void TalkThread_Duckman::onSuspend() {
}

void TalkThread_Duckman::onNotify() {
}

void TalkThread_Duckman::onPause() {
}

void TalkThread_Duckman::onResume() {
}

void TalkThread_Duckman::onTerminated() {
	if (_status == 5) {
		if (!(_flags & 4))
			_vm->stopVoice();
		if (!(_flags & 8)) {
			_vm->_screenText->removeText();
		}
		if (!(_flags & 2)) {
			Control *control = _vm->_dict->getObjectControl(_objectId);
			if (control) {
				control->clearNotifyThreadId1();
				control->startSequenceActor(_sequenceId2, 2, 0);
			}
		}
	}
}

void TalkThread_Duckman::onKill() {
	_callingThreadId = 0;
	sendMessage(kMsgClearSequenceId1, 0);
	sendMessage(kMsgClearSequenceId2, 0);
}

uint32 TalkThread_Duckman::sendMessage(int msgNum, uint32 msgValue) {
	switch (msgNum) {
	case kMsgQueryTalkThreadActive:
		if (_status != 1)
			return 1;
		break;
	case kMsgClearSequenceId1:
		_sequenceId1 = 0;
		_flags |= 3;
		// TODO _pauseCtrPtr = 0;
		break;
	case kMsgClearSequenceId2:
		_sequenceId2 = 0;
		break;
	}
	return 0;
}

void TalkThread_Duckman::refreshText() {
	_currEntryText = _entryText;
	int charCount = insertText();
	uint32 duration = _durationMult;
	if (charCount < 80) {
		duration = _durationMult * charCount / 80;
		if (duration < 25 * _durationMult / 100)
			duration = 25 * _durationMult / 100;
		if (duration < 60)
			duration = 60;
	}
	_textDuration = duration;
	_textStartTime = getCurrentTime();
	_textEndTime = _textStartTime + _textDuration;
}

static char *debugW2I(byte *wstr) {
	static char buf[65];
	char *p = buf;
	while (*wstr != 0) {
		*p++ = *wstr;
		wstr += 2;
	}
	*p = 0;
	return buf;
}

int TalkThread_Duckman::insertText() {
	debug("%08X %08X [%s]", _threadId, _talkId, debugW2I(_currEntryText));
	WidthHeight dimensions;
	_vm->getDefaultTextDimensions(dimensions);
	uint16 *outTextPtr;
	_vm->_screenText->insertText((uint16*)_currEntryText, 0x120001, dimensions, Common::Point(0, 0), 2, 0, 0, _colorR, _colorG, _colorB, outTextPtr);
	_entryText = (byte*)outTextPtr;
	Common::Point pt;
	_vm->getDefaultTextPosition(pt);
	_vm->_screenText->updateTextInfoPosition(pt);
	int charCount = (_entryText - _currEntryText) / 2;
	return charCount;
}

TalkEntry *TalkThread_Duckman::getTalkResourceEntry(uint32 talkId) {
	TalkEntry *talkEntry = _vm->_dict->findTalkEntry(talkId);
	return talkEntry;
}

} // End of namespace Illusions
