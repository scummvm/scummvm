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

#include "lastexpress/lastexpress.h"
#include "lastexpress/game/events.h"

namespace LastExpress {

MessageManager::MessageManager(LastExpressEngine *engine) {
	_engine = engine;

	for (int i = 0; i < 16; i++) {
		_eventHandles[i] = nullptr;
	}

	for (int i = 0; i < 40; i++) {
		_messageHandles[i] = nullptr;
	}

	_autoMessages = nullptr;
}

MessageManager::~MessageManager() {
	free(_autoMessages);
	_autoMessages = nullptr;
}

void MessageManager::setMessageHandle(int handleChannel, void (LogicManager::*handle)(Message *)) {
	_messageHandles[handleChannel] = handle;
}

void (LogicManager::*MessageManager::getMessageHandle(int handleChannel))(Message *) {
	return _messageHandles[handleChannel];
}

void MessageManager::setEventHandle(int handleChannel, void (LastExpressEngine::*handle)(Event *)) {
	_eventHandles[handleChannel] = handle;
}

void (LastExpressEngine::*MessageManager::getEventHandle(int handleChannel))(Event *) {
	return _eventHandles[handleChannel];
}

void MessageManager::addEvent(int channel, int x, int y, int flags) {
	Common::StackLock lock(*_engine->_soundMutex);

	if (_numEventsInQueue >= 127)
		return;

	if (channel == kEventChannelTimer) {
		_engine->setEventTickInternal(true);
	} else if (channel == kEventChannelMouse) {
		if ((flags & kMouseFlagLeftButton) != 0) {
			// Originally _engine->mouseSetLeftClicked(true); was called from here,
			// but it's been moved under the "if" because this lead to fake double
			// clicks when registering mouse movement events (which are re-paired
			// with RMOUSE and LMOUSE flags when sent to the engine via this function).
			if (!_systemEventLeftMouseDown) {
				_engine->mouseSetLeftClicked(true);
				flags |= kMouseFlagLeftDown;
				_systemEventLeftMouseDown = true;
	
				if (_doubleClickMaxFrames + _latestTickLeftMousePressed > _engine->getSoundFrameCounter())
					flags |= (kMouseFlagDoubleClick | kMouseFlagLeftDown);
	
				_latestTickLeftMousePressed = _engine->getSoundFrameCounter();
			}
		} else {
			if (_systemEventLeftMouseDown)
				flags |= kMouseFlagLeftUp;
	
			_systemEventLeftMouseDown = false;
		}
	
		if ((flags & kMouseFlagRightButton) != 0) {
			// Originally _engine->mouseSetRightClicked(true); was called from here,
			// but it's been moved under the "if" because this lead to fake double
			// clicks when registering mouse movement events (which are re-paired
			// with RMOUSE and LMOUSE flags when sent to the engine via this function).
			if (!_systemEventRightMouseDown) {
				_engine->mouseSetRightClicked(true);
				flags |= kMouseFlagRightDown;
				_systemEventRightMouseDown = true;
			}
		} else {
			if (_systemEventRightMouseDown)
				flags |= kMouseFlagRightUp;
	
			_systemEventRightMouseDown = false;
		}

		// Originally "if (!flags)"; this tames slowdowns when dragging the mouse
		// with one of the buttons pressed. Hopefully it doesn't break anything...
		if (!flags || ((flags & kMouseFlagLeftButton) != 0 || (flags & kMouseFlagRightButton) != 0)) {
			if (_lastEventIndex != 128)
				_events[_lastEventIndex].channel = 0;
		
			_lastEventIndex = _curEventIndex;
		}
	}

	_events[_curEventIndex].channel = channel;
	_events[_curEventIndex].x = x;
	_events[_curEventIndex].y = y;
	_events[_curEventIndex].flags = flags;

	_curEventIndex++;
	_numEventsInQueue++;
	_curEventIndex &= 0x7F;
}

Event *MessageManager::getEvent() {
	Event *result = nullptr;

	Common::StackLock lock(*_engine->_soundMutex);

	if (_nextEventIndex == _curEventIndex) {
		// No events in queue...
		return nullptr;
	}

	// Wrap around the circular buffer...
	if (_lastEventIndex == _nextEventIndex) {
		_lastEventIndex = 128;
	}

	result = &_events[_nextEventIndex];

	// Update event queue indexes...
	_numEventsInQueue--;
	_nextEventIndex++;
	_nextEventIndex &= 0x7F; // Wrap around the circular buffer...

	return result;
}

void MessageManager::addMessage(int receiver, int actionId, int sender, ConsCallParam param) {
	if (_numMsgsInQueue < 127) {
		Message *newMsg = &_messages[_curMsgIndex];
		newMsg->receiver = receiver;
		newMsg->action = actionId;
		newMsg->sender = sender;
		newMsg->param = param;

		_curMsgIndex++;
		_numMsgsInQueue++;
		_curMsgIndex &= 0x7F;
	}
}

Message *MessageManager::getMessage() {
	if (_nextMsgIndex == _curMsgIndex)
		return nullptr;
	
	Message *msg = &_messages[_nextMsgIndex];

	_nextMsgIndex = (_nextMsgIndex + 1) & 0x7F;
	_numMsgsInQueue--;

	return msg;
}

bool MessageManager::process() {
	Event *event;

	event = getEvent();

	if (!event || event->channel >= 16)
		return false;

	if (_eventHandles[event->channel])
		(_engine->*_eventHandles[event->channel])(event);

	return true;
}

void MessageManager::flush() {
	Message *message; 

	do {
		message = getMessage();
		if (!message)
			break;

		if (!_engine->getLogicManager()->doAutoMessage(message)) {
			if (_messageHandles[message->receiver])
				(_engine->getLogicManager()->*_messageHandles[message->receiver])(message);
		}

	} while (message && _engine->_navigationEngineIsRunning);
}

void MessageManager::flushTime() {
	for (int i = 1; i < ARRAYSIZE(_messageHandles) && _engine->_navigationEngineIsRunning; i++) {
		if (_messageHandles[i])
			(_engine->getLogicManager()->*_messageHandles[i])(&_emptyMessage);
	}

	if (_engine->_navigationEngineIsRunning)
		flush();
}

void MessageManager::forceMessage(Message *msg) {
	if (_messageHandles[msg->receiver])
		(_engine->getLogicManager()->*_messageHandles[msg->receiver])(msg);
}

void MessageManager::clearMessageQueue() {
	Common::StackLock lock(*_engine->_soundMutex);
	_numMsgsInQueue = 0;
	_nextMsgIndex = 0;
	_curMsgIndex = 0;
}

void MessageManager::clearEventQueue() {
	Common::StackLock lock(*_engine->_soundMutex);
	_numEventsInQueue = 0;
	_nextEventIndex = 0;
	_curEventIndex = 0;
	_engine->setEventTickInternal(false);
	_engine->mouseSetLeftClicked(false);
	_engine->mouseSetRightClicked(false);
}

void MessageManager::clearClickEvents() {
	Common::StackLock lock(*_engine->_soundMutex);
	if (_numEventsInQueue) {
		for (int i = _nextEventIndex; _curEventIndex != i; i = ((i + 1) & 0x7F)) {
			if (_events[i].channel == 1 && _events[i].flags)
				_events[i].flags = 0;
		}
	}
}

void MessageManager::saveMessages(CVCRFile *file) {
	byte *msgData = (byte *)malloc(16 * 128);
	assert(msgData);

	for (int i = 0; i < 128; i++) {
		WRITE_LE_INT32(msgData + (i * 16) + 0, _autoMessages[i].receiver);
		WRITE_LE_INT32(msgData + (i * 16) + 4, _autoMessages[i].action);
		WRITE_LE_INT32(msgData + (i * 16) + 8, _autoMessages[i].sender);
		WRITE_LE_INT32(msgData + (i * 16) + 12, _autoMessages[i].param.intParam);
	}

	file->writeRLE(msgData, 16, 128);
	file->writeRLE(&_numMsgsInQueue, 4, 1);

	free(msgData);
	msgData = nullptr;

	if (_numMsgsInQueue) {
		int nextIdx = _nextMsgIndex;

		while (_curMsgIndex != nextIdx) {
			Message *msgToSave = &_messages[nextIdx];

			msgData = (byte *)malloc(16);
			assert(msgData);

			WRITE_LE_INT32(msgData + 0, msgToSave->receiver);
			WRITE_LE_INT32(msgData + 4, msgToSave->action);
			WRITE_LE_INT32(msgData + 8, msgToSave->sender);
			WRITE_LE_INT32(msgData + 12, msgToSave->param.intParam);

			nextIdx = (nextIdx + 1) & 0x7F;
			file->writeRLE(msgData, 16, 1);

			free(msgData);
			msgData = nullptr;
		}
	}
}

void MessageManager::loadMessages(CVCRFile *file) {
	byte *msgData = (byte *)malloc(16 * 128);
	assert(msgData);

	file->readRLE(msgData, 16, 128);

	for (int i = 0; i < 128; i++) {
		_autoMessages[i].receiver = READ_LE_INT32(msgData + (i * 16) + 0);
		_autoMessages[i].action = READ_LE_INT32(msgData + (i * 16) + 4);
		_autoMessages[i].sender = READ_LE_INT32(msgData + (i * 16) + 8);
		_autoMessages[i].param.intParam = READ_LE_INT32(msgData + (i * 16) + 12);
		_autoMessages[i].param.stringParam = nullptr;
	}

	free(msgData);
	msgData = nullptr;

	int numMsgsInQueue;
	file->readRLE(&numMsgsInQueue, 4, 1);

	Message loadedMsg;
	for (int i = 0; i < numMsgsInQueue; i++) {
		msgData = (byte *)malloc(16);
		assert(msgData);

		file->readRLE(msgData, 16, 1);

		loadedMsg.receiver = READ_LE_INT32(msgData + 0);
		loadedMsg.action = READ_LE_INT32(msgData + 4);
		loadedMsg.sender = READ_LE_INT32(msgData + 8);
		loadedMsg.param.intParam = READ_LE_INT32(msgData + 12);
		loadedMsg.param.stringParam = nullptr;

		addMessage(loadedMsg.receiver, loadedMsg.action, loadedMsg.sender, loadedMsg.param);
		free(msgData);
		msgData = nullptr;
	}
}

} // End of namespace LastExpress
