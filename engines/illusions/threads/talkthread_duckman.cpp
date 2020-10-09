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

#include "illusions/duckman/illusions_duckman.h"
#include "illusions/threads/talkthread_duckman.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/resources/talkresource.h"
#include "illusions/screentext.h"
#include "illusions/sound.h"
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

	_durationMult = _vm->clipTextDuration(_vm->_subtitleDuration);
	_textDuration = _durationMult;
	_defDurationMult = _vm->clipTextDuration(240);

	_sceneId = _vm->getCurrentScene();

}

int TalkThread_Duckman::onUpdate() {

	TalkEntry *talkEntry;

	switch (_status) {

	case 1:
		if (_vm->checkActiveTalkThreads())
			return kTSYield;
		_status = 3;
		// fall through

	case 2:
		talkEntry = getTalkResourceEntry(_talkId);
		_flags = 0;
		_currEntryText = 0;
		_entryText = talkEntry->_text;
		_entryTblPtr = talkEntry->_tblPtr;
		if (_sequenceId1) {
			_pauseCtrPtr = &_pauseCtr;
			_pauseCtr = 0;
		} else {
			_pauseCtrPtr = 0;
			_flags |= 2;
			_flags |= 1;
		}
		if (_vm->isSoundActive()) {
			if (!_vm->_soundMan->cueVoice((char*)talkEntry->_voiceName) && !_durationMult)
				_durationMult = _defDurationMult;
		} else {
			_flags |= 4;
			if (_durationMult == 0)
				_durationMult = _defDurationMult;
		}
		if (_objectId == 0 || _durationMult == 0)
			_flags |= 8;
		_status = 3;
		// fall through

	case 3:
		if (!(_flags & 4) && !_vm->_soundMan->isVoiceCued())
			return kTSYield;
		_status = 4;
		// fall through

	case 4:
		if (!(_flags & 8) ) {
			uint32 actorTypeId = _vm->getObjectActorTypeId(_objectId);
			getActorTypeColor(actorTypeId, _color);
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
			_vm->_soundMan->startVoice(255, panX);
		}
		_vm->_input->discardEvent(kEventSkip);
		_status = 5;
		return kTSYield;

	case 5:
		if (!(_flags & 4) && !_vm->_soundMan->isVoicePlaying())
			_flags |= 4;
		if (!(_flags & 8) && isTimerExpired(_textStartTime, _textEndTime)) {
			_vm->_screenText->removeText();
			if (_entryText && *_entryText) {
				refreshText();
				_vm->_input->discardEvent(kEventSkip);
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
		if (_objectId && _vm->_input->pollEvent(kEventSkip)) {
			if (!(_flags & 8)) {
				_vm->_screenText->removeText();
				if (_entryText && *_entryText)
					refreshText();
				else
					_flags |= 8;
			}
			if (_flags & 8) {
				if (!(_flags & 4)) {
					_vm->_soundMan->stopVoice();
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
			_vm->_input->discardEvent(kEventSkip);
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

	default:
		break;
	}

	return kTSTerminate;

}

void TalkThread_Duckman::onPause() {
	if (_status == 5) {
		if (!(_flags & 4)) {
			_vm->_soundMan->pauseVoice();
		}
		if (!(_flags & 8))
			_textDurationElapsed = getDurationElapsed(_textStartTime, _textEndTime);
	}
}

void TalkThread_Duckman::onUnpause() {
	if (_status == 3) {
		TalkEntry *talkEntry = getTalkResourceEntry(_talkId);
		if (!_vm->isSoundActive())
			_vm->_soundMan->cueVoice((char*)talkEntry->_voiceName);
	} else if (_status == 5) {
		if (!(_flags & 4)) {
			_vm->_soundMan->unpauseVoice();
		}
		if (!(_flags & 8)) {
			_textStartTime = getCurrentTime();
			if (_textDuration <= _textDurationElapsed)
				_textEndTime = _textStartTime;
			else
				_textEndTime = _textStartTime + _textDuration - _textDurationElapsed;
			_textDurationElapsed = 0;
		}
	}
}

void TalkThread_Duckman::onResume() {
}

void TalkThread_Duckman::onTerminated() {
	if (_status == 5) {
		if (!(_flags & 4))
			_vm->_soundMan->stopVoice();
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
		break;
	case kMsgClearSequenceId2:
		_sequenceId2 = 0;
		break;
	default:
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

int TalkThread_Duckman::insertText() {
	debug(0, "%08X %08X [%s]", _threadId, _talkId, debugW2I(_currEntryText));
	WidthHeight dimensions;
	_vm->getDefaultTextDimensions(dimensions);
	uint16 *outTextPtr;
	_vm->_screenText->insertText(_currEntryText, 0x120001, dimensions,
		Common::Point(0, 0), TEXT_FLAG_CENTER_ALIGN, 0, 0, _color.r, _color.g, _color.b, outTextPtr);
	_entryText = outTextPtr;
	Common::Point pt;
	_vm->getDefaultTextPosition(pt);
	_vm->_screenText->updateTextInfoPosition(pt);
	int charCount = _entryText - _currEntryText;
	return charCount;
}

TalkEntry *TalkThread_Duckman::getTalkResourceEntry(uint32 talkId) {
	TalkEntry *talkEntry = _vm->_dict->findTalkEntry(talkId);
	return talkEntry;
}

void TalkThread_Duckman::getActorTypeColor(uint32 actorTypeId, RGB &color) {
	ActorType *actorType = _vm->_dict->findActorType(actorTypeId);
	color = actorType->_color;
}

} // End of namespace Illusions
