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

#ifndef MEDIASTATION_EVENTS_H
#define MEDIASTATION_EVENTS_H

#include "common/events.h"
#include "common/hash-ptr.h"
#include "common/ptr.h"
#include "common/str.h"
#include "common/queue.h"

#include "mediastation/mediascript/scriptconstants.h"
#include "mediastation/mediascript/scriptvalue.h"

namespace MediaStation {

enum EventClass {
	kEventClassInvalid = 0x00,
	kEventClassSystem = 0x01,
	kEventClassMouse = 0x02,
	kEventClassKeyboard = 0x04,
	kEventClassDisplay = 0x08,
	kEventClassTimerService = 0x10,
	kEventClassScriptTimer = 0x20,
	kEventClassActor = 0x40
};
const char *eventClassToStr(EventClass eventClass);

// This reimplementation doesn't use these values because
// there isn't the indirection of event handler classes. We
// just use the event class directly when deciding where to route an
// event. However, this is still included for completeness .
enum EventClassHandlerType {
	kNoEventClassHandler = 0x00,
	kStageDirectorEventClassHandler = 0x06,
	kDisplayUpdateEventClassHandler = 0x08,
	kTimerServiceClassHandler = 0x10,
	kActorEventClassHandler = 0x40,
};

struct Event {
	EventClass eventClass = kEventClassInvalid;
	EventType type = kEventTypeInvalid;

	Event(EventClass eventClass_, EventType eventType_)
		: eventClass(eventClass_), type(eventType_) {}
	virtual ~Event() {}
	virtual Event *clone() const = 0;
	virtual Common::String debugString() const;
};

struct DisplayEvent : public Event {
	uint disableScreenAutoUpdateToken = 0;

	DisplayEvent(EventType eventType_, uint token_)
		: Event(kEventClassDisplay, eventType_), disableScreenAutoUpdateToken(token_) {}
	Event *clone() const override { return new DisplayEvent(*this); }
	Common::String debugString() const override;
};

// This is an event queued to tell the timer service that a timer has expired. When this event is processed,
// an actual timer event will get sent to the actual timer event handler (actors and such).
class TimerEntry;
struct TimerServiceAlarmEvent : public Event {
	uint32 triggerTime = 0;
	TimerEntry *entry = nullptr;

	TimerServiceAlarmEvent(TimerEntry *entry_, uint32 triggerTime_)
		: Event(kEventClassTimerService, kTimerServiceAlarmEvent), triggerTime(triggerTime_), entry(entry_) {}
	Event *clone() const override { return new TimerServiceAlarmEvent(*this); }
	Common::String debugString() const override;
};

// This is actually sent to timer event handlers (actors and such).
struct TimerEvent : public Event {
	TimerEntry *entry = nullptr;

	TimerEvent(TimerEntry *entry_)
		: Event(kEventClassScriptTimer, kTimerScriptEvent), entry(entry_) {}
	Event *clone() const override { return new TimerEvent(*this); }
	Common::String debugString() const override;
};

struct ActorEvent : public Event {
	uint actorId = 0;
	ScriptValue arg;

	ActorEvent(uint16 actorId_, EventType eventType_)
		: Event(kEventClassActor, eventType_), actorId(actorId_) {}
	ActorEvent(uint16 actorId_, EventType eventType_, ScriptValue arg_)
		: Event(kEventClassActor, eventType_), actorId(actorId_), arg(arg_) {}
	Event *clone() const override { return new ActorEvent(*this); }
	Common::String debugString() const override;
};

struct ScreenBranchEvent : public ActorEvent {
	uint screenId = 0;
	bool disableScreenAutoUpdate = false;

	ScreenBranchEvent(uint16 actorId_, uint16 screenId_, bool disableScreenAutoUpdate_)
		: ActorEvent(actorId_, kScreenBranchEvent), screenId(screenId_), disableScreenAutoUpdate(disableScreenAutoUpdate_) {}
	Event *clone() const override { return new ScreenBranchEvent(*this); }
	Common::String debugString() const override;
};

struct MouseEvent : public Event {
	Common::Point position;

	MouseEvent(EventType eventType_, const Common::Point &position_)
		: Event(kEventClassMouse, eventType_), position(position_) {}
	Event *clone() const override { return new MouseEvent(*this); }
	Common::String debugString() const override;
};

struct KeyboardEvent : public Event {
	uint16 keyCode = 0;

	KeyboardEvent(EventType eventType_, uint16 keyCode_)
		: Event(kEventClassKeyboard, eventType_), keyCode(keyCode_) {}
	Event *clone() const override { return new KeyboardEvent(*this); }
	Common::String debugString() const override;
};

enum PreDisplaySyncState {
	kPreDisplaySyncForceScreenUpdate = 0,
	kPreDisplaySyncNoScreenUpdateRequested = 1,
	kPreDisplaySyncStateBlockScreenUpdate = 2
};

// Interface for clients that need to update during the pre-display sync phase.
// Only stream movie actors seem to actually use this.
class PreDisplaySyncClient {
public:
	virtual ~PreDisplaySyncClient() {}
	virtual PreDisplaySyncState preDisplaySync() = 0;

	void registerForSyncCalls();
	void unregisterForSyncCalls();
};

// The main event loop. In the original, much of the functionality lived in a separate
// EventLoopSupport class (implemented by MAC_App or WIN_App), but in ScummVM there
// is no need for that extra indirection.
class EventLoop {
public:
	void run();
	void queueEvent(const Event &event);

	void registerForPreDisplaySyncCalls(PreDisplaySyncClient *client);
	void unregisterForPreDisplaySyncCalls(PreDisplaySyncClient *client);

private:
	Common::Queue<Common::ScopedPtr<Event>> _incomingQueue;
	Common::Queue<Common::ScopedPtr<Event>> _dispatchQueue;
	Common::Event _queuedSystemEvent;
	Common::HashMap<PreDisplaySyncClient *, PreDisplaySyncClient *> _preDisplaySyncClients;

	void dispatchImtEvents();
	PreDisplaySyncState preDisplaySync();
	void updateDisplay();

	// The original had a separate EventDispatcher, but things are much simpler by not using
	// the polymorphism that required this intermediate class.
	void dispatchImtEvent(const Event &event);
};

class TimerEventReceiver {
public:
	virtual ~TimerEventReceiver() {};

	uint32 currentReceiverTime();
	virtual void timerEvent(const TimerEvent &event) = 0;
};

class TimerEntry {
public:
	TimerEntry() = default;
	TimerEntry(TimerEventReceiver *receiver) : _receiver(receiver) {}
	~TimerEntry();

	void setDuration(uint32 duration) { _duration = duration; }
	uint32 calculateFirstExpirationTime(uint32 currentTime);
	uint32 calculateNextExpirationTime(uint32 currentTime);
	uint32 deltaTimeAtExpiration() const;
	uint32 expirationTime() const { return _expirationTime; }
	bool shouldReschedule() const { return _shouldRepeat; }
	TimerEventReceiver *getReceiver() const { return _receiver; }

	bool operator==(const TimerEntry &other) const;
	bool operator<(const TimerEntry &other) const;
	bool operator>(const TimerEntry &other) const;

private:
	TimerEventReceiver *_receiver = nullptr;
	uint32 _expirationTime = 0;
	uint32 _actualExpirationTime = 0;
	uint32 _duration = 0;
	bool _shouldRepeat = false;
};

class TimerService {
public:
	// This is called to actually dispatch a timer event to the event receiver.
	void handleEvent(const TimerServiceAlarmEvent &event);

	void startTimer(TimerEntry &entry, uint32 duration);
	void startTimer(TimerEntry &entry, double duration);
	void stopTimer(TimerEntry &entry);
	void queueExpiredTimerEvents();

private:
	Common::HashMap<TimerEntry *, TimerEntry *> _timers;

	void rescheduleTimerEntry(TimerEntry &entry);
};

} // End of namespace MediaStation

#endif
