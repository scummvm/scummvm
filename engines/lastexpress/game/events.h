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

#ifndef LASTEXPRESS_EVENTS_H
#define LASTEXPRESS_EVENTS_H

#include "lastexpress/lastexpress.h"

namespace LastExpress {

class LastExpressEngine;
class LogicManager;

struct ConsCallParam {
	ConsCallParam() : intParam(0), stringParam(nullptr) {}

	template<typename T>
	ConsCallParam(T param) : intParam(static_cast<int32>(param)), stringParam(nullptr) {}

	ConsCallParam(const char *param) : intParam(0), stringParam(param) {}
	ConsCallParam(char *param) : intParam(0), stringParam(param) {}

	int32 intParam;
	const char *stringParam;
};

#include "common/pack-start.h"

typedef struct Message {
	int receiver;
	int action;
	int sender;
	ConsCallParam param;

	Message() {
		clear();
	}

	void clear() {
		receiver = 0;
		action = 0;
		sender = 0;
	}
} Message;

#include "common/pack-end.h"

typedef struct Event {
	int channel;
	int x;
	int y;
	int flags;

	Event() {
		channel = 0;
		x = 0;
		y = 0;
		flags = 0;
	}
} Event;

class MessageManager {
	friend class LastExpressEngine;
	friend class LogicManager;
	friend class MemoryManager;

public:
	MessageManager(LastExpressEngine *engine);
	~MessageManager();

	void setMessageHandle(int handleChannel, void (LogicManager::*handle)(Message *));
	void (LogicManager::*getMessageHandle(int handleChannel))(Message *);
	void setEventHandle(int handleChannel, void (LastExpressEngine::*handle)(Event *));
	void (LastExpressEngine::*getEventHandle(int handleChannel))(Event *);
	void addEvent(int channel, int x, int y, int flags);
	Event *getEvent();
	void addMessage(int receiver, int actionId, int sender, ConsCallParam param);
	Message *getMessage();
	bool process();
	void flush();
	void flushTime();
	void forceMessage(Message *msg);
	void clearMessageQueue();
	void clearEventQueue();
	void clearClickEvents();
	void saveMessages(CVCRFile *file);
	void loadMessages(CVCRFile *file);

private:
	LastExpressEngine *_engine = nullptr;

	Message _messages[128];
	Message _emptyMessage;
	void (LogicManager::*_messageHandles[40])(Message *);
	int _curMsgIndex = 0;
	int _numMsgsInQueue = 0;
	int _nextMsgIndex = 0;
	Message *_autoMessages = nullptr;

	Event _events[128];
	void (LastExpressEngine::*_eventHandles[16])(Event *);
	int _doubleClickMaxFrames = 30;
	int _latestTickLeftMousePressed = 0;
	int _lastEventIndex = 128;
	int _curEventIndex = 0;
	int _numEventsInQueue = 0;
	int _nextEventIndex = 0;
	bool _systemEventRightMouseDown = false;
	bool _systemEventLeftMouseDown = false;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_EVENTS_H
