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

#include "illusions/illusions.h"
#include "illusions/threads/talkthread.h"
#include "illusions/actor.h"
#include "illusions/dictionary.h"
#include "illusions/input.h"
#include "illusions/resources/talkresource.h"
#include "illusions/screentext.h"
#include "illusions/sound.h"
#include "illusions/time.h"

namespace Illusions {

// TalkThread

TalkThread::TalkThread(IllusionsEngine *vm, uint32 threadId, uint32 callingThreadId, uint notifyFlags,
	int16 duration, uint32 objectId, uint32 talkId, uint32 sequenceId1, uint32 sequenceId2,
	uint32 namedPointId)
	: Thread(vm, threadId, callingThreadId, notifyFlags), _objectId(objectId), _talkId(talkId),
	_sequenceId1(0), _sequenceId2(0) {
	_type = kTTTalkThread;

	if (sequenceId1 && _vm->_dict->getObjectControl(objectId)) {
		_sequenceId1 = sequenceId1;
		_sequenceId2 = sequenceId2;
	}

	if (!callingThreadId)
		_sequenceId2 = 0;

	_namedPointId = namedPointId;

	if (duration)
		_status = 1;
	else if (_vm->checkActiveTalkThreads())
		_status = 2;
	else
		_status = 3;

	_flags = 0x0E;

	_durationMult = _vm->clipTextDuration(_vm->getSubtitleDuration());
	_textDuration = _durationMult;
	_defDurationMult = _vm->clipTextDuration(240);
	_textStartTime = 0;
	_textEndTime = 0;
	_textDurationElapsed = 0;
	_entryText = 0;
	_currEntryText = 0;
	_voiceDurationElapsed = 0;
	_voiceDuration = duration;
	_voiceStartTime = getCurrentTime();
	_voiceEndTime = _voiceStartTime + duration;
	_entryTblPtr = 0;

	if (callingThreadId) {
		Thread *callingThread = _vm->_threads->findThread(callingThreadId);
		if (callingThread)
			_sceneId = callingThread->_sceneId;
	}

}

int TalkThread::onUpdate() {

	TalkEntry *talkEntry;

	switch (_status) {

	case 1:
		if (isTimerExpired(_voiceStartTime, _voiceEndTime)) {
			if (_vm->checkActiveTalkThreads())
				_status = 2;
			else
				_status = 3;
		}
		return kTSYield;

	case 2:
		if (_vm->checkActiveTalkThreads())
			return kTSYield;
		_status = 3;
		// fall through

	case 3:
		talkEntry = getTalkResourceEntry(_talkId);
		_flags = 0;
		_currEntryText = 0;
		_entryText = talkEntry->_text;
		_entryTblPtr = talkEntry->_tblPtr;
		if (_sequenceId1) {
			_pauseCtr = 0;
		} else {
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
		_status = 4;
		// fall through

	case 4:
		if (!(_flags & 4) && !_vm->_soundMan->isVoiceCued())
			return kTSYield;
		_status = 5;
		// fall through

	case 5:
		if (!(_flags & 8))
			refreshText();
		if (!(_flags & 2)) {
			Control *control = _vm->_dict->getObjectControl(_objectId);
			control->startTalkActor(_sequenceId1, _entryTblPtr, _threadId);
		}
		if (!(_flags & 4)) {
			int16 panX = 0;
			if (_namedPointId) {
				Common::Point pt = _vm->getNamedPointPosition(_namedPointId);
				panX = _vm->convertPanXCoord(pt.x);
			}
			_vm->_soundMan->startVoice(255, panX);
		}
		_vm->_input->discardEvent(kEventSkip);
		_status = 6;
		return kTSYield;

	case 6:
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
		if ((_flags & 4) && (_flags & 8)) {
			if (_sequenceId2) {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->startSequenceActor(_sequenceId2, 2, 0);
			}
			if (_sequenceId1) {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->clearNotifyThreadId2();
			}
			_flags |= 2;
		}
//#define DEBUG_SPEEDUP_TALK
#ifdef DEBUG_SPEEDUP_TALK
if (true) {
#else
		if (_objectId && _vm->_input->pollEvent(kEventSkip)) {
#endif
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
					if (_sequenceId2) {
						Control *control = _vm->_dict->getObjectControl(_objectId);
						control->startSequenceActor(_sequenceId2, 2, 0);
					}
					if (_sequenceId1) {
						Control *control = _vm->_dict->getObjectControl(_objectId);
						control->clearNotifyThreadId2();
					}
					_flags |= 2;
				}
			}
		}
		if ((_flags & 8) && (_flags & 2) && (_flags & 4)) {
			_vm->_input->discardEvent(kEventSkip);
			_status = 7;
			return kTSTerminate;
		}
		return kTSYield;

	case 7:
		if (!(_flags & 2)) {
			if (_sequenceId2) {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->startSequenceActor(_sequenceId2, 2, 0);
			}
			if (_sequenceId1) {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->clearNotifyThreadId2();
			}
			_flags |= 2;
		}
		if (!(_flags & 8)) {
			_vm->_screenText->removeText();
			_flags |= 8;
		}
		if (!(_flags & 4)) {
			_vm->_soundMan->stopVoice();
			_flags |= 4;
		}
		return kTSTerminate;

	default:
		break;
	}

	return kTSTerminate;
}

void TalkThread::onSuspend() {
	switch (_status) {
	case 1:
		_voiceDurationElapsed = getDurationElapsed(_voiceStartTime, _voiceEndTime);
		_status = 7;
		break;
	case 4:
		_vm->_soundMan->stopCueingVoice();
		_status = 7;
		break;
	case 6:
	case 7:
		if (!(_flags & 4)) {
			_vm->_soundMan->stopVoice();
			_flags |= 4;
		}
		if (!(_flags & 8)) {
			_vm->_screenText->removeText();
			_flags |= 8;
		}
		_status = 7;
		break;
	default:
		_status = 7;
		break;
	}
}

void TalkThread::onPause() {
	switch (_status) {
	case 1:
		_voiceDurationElapsed = getDurationElapsed(_voiceStartTime, _voiceEndTime);
		break;
	case 4:
		_vm->_soundMan->stopCueingVoice();
		break;
	case 6:
	case 7:
		if (!(_flags & 4)) {
			_vm->_soundMan->pauseVoice();
		}
		if (!(_flags & 8)) {
			_textDurationElapsed = getDurationElapsed(_textStartTime, _textEndTime);
		}
		break;
	default:
		break;
	}
}

void TalkThread::onUnpause() {
	switch (_status) {
	case 1:
		_voiceStartTime = getCurrentTime();
		if (_voiceDuration <= _voiceDurationElapsed) {
			_voiceDurationElapsed = 0;
			_voiceEndTime = _voiceStartTime;
		} else {
			_voiceDurationElapsed = 0;
			_voiceEndTime = _voiceStartTime + _voiceDuration - _voiceDurationElapsed;
		}
		break;
	case 4:
		if (_vm->isSoundActive()) {
			TalkEntry *talkEntry = getTalkResourceEntry(_talkId);
			_vm->_soundMan->cueVoice((char*)talkEntry->_voiceName);
		}
		break;
	case 6:
		if (!(_flags & 4)) {
			_vm->_soundMan->unpauseVoice();
		}
		if (!(_flags & 8)) {
			_textStartTime = getCurrentTime();
			if (_textDuration <= _textDurationElapsed) {
				_textDurationElapsed = 0;
				_textEndTime = _textStartTime;
			} else {
				_textDurationElapsed = 0;
				_textEndTime = _textStartTime + _textDuration - _textDurationElapsed;
			}
		}
		break;
	default:
		break;
	}
}

void TalkThread::onTerminated() {
	if (_status == 4) {
		_vm->_soundMan->stopCueingVoice();
	} else if (_status == 6) {
		if (!(_flags & 4)) {
			_vm->_soundMan->stopVoice();
			_flags |= 4;
		}
		if (!(_flags & 8)) {
			_vm->_screenText->removeText();
			_flags |= 8;
		}
		if (!(_flags & 2)) {
			if (_sequenceId2) {
				Control *control = _vm->_dict->getObjectControl(_objectId);
				control->startSequenceActor(_sequenceId2, 2, 0);
			}
			_flags |= 2;
		}
	}
}

void TalkThread::onKill() {
	_callingThreadId = 0;
	sendMessage(kMsgClearSequenceId1, 0);
	sendMessage(kMsgClearSequenceId2, 0);
}

uint32 TalkThread::sendMessage(int msgNum, uint32 msgValue) {
	switch (msgNum) {
	case kMsgQueryTalkThreadActive:
		if (_status != 1 && _status != 2)
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

void TalkThread::refreshText() {
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

int TalkThread::insertText() {
	debug("%08X %08X [%s]", _threadId, _talkId, debugW2I(_currEntryText));
	WidthHeight dimensions;
	_vm->getDefaultTextDimensions(dimensions);
	uint16 *outTextPtr;
	_vm->_screenText->insertText(_currEntryText, 0x120001, dimensions,
		Common::Point(0, 0), TEXT_FLAG_CENTER_ALIGN, 0, 0, 0, 0, 0, outTextPtr);
	_entryText = outTextPtr;
	Common::Point pt;
	_vm->getDefaultTextPosition(pt);
	_vm->_screenText->updateTextInfoPosition(pt);
	int charCount = (_entryText - _currEntryText) / 2;
	return charCount;
}

TalkEntry *TalkThread::getTalkResourceEntry(uint32 talkId) {
	TalkEntry *talkEntry = _vm->_dict->findTalkEntry(talkId);
	return talkEntry;
}

} // End of namespace Illusions
