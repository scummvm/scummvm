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

#include "common/str.h"

#include "mediastation/debugchannels.h"
#include "mediastation/events.h"
#include "mediastation/actors/sound.h"
#include "mediastation/mediastation.h"

namespace MediaStation {

const char *eventClassToStr(EventClass eventClass) {
	switch (eventClass) {
	case kEventClassInvalid:
		return "Invalid";
	case kEventClassSystem:
		return "System";
	case kEventClassMouse:
		return "Mouse";
	case kEventClassKeyboard:
		return "Keyboard";
	case kEventClassDisplay:
		return "Display";
	case kEventClassTimerService:
		return "TimerServiceAlarm";
	case kEventClassScriptTimer:
		return "ScriptTimer";
	case kEventClassActor:
		return "Actor";
	default:
		return "UNKNOWN";
	}
}

Common::String Event::debugString() const {
	return Common::String::format("%s::%s",
		eventClassToStr(eventClass), eventTypeToStr(type));
}

Common::String DisplayEvent::debugString() const {
	return Common::String::format("%s %s (disableScreenAutoUpdateToken: %u)",
		eventClassToStr(eventClass), eventTypeToStr(type), disableScreenAutoUpdateToken);
}

Common::String TimerServiceAlarmEvent::debugString() const {
	return Common::String::format("%s %s (triggerTime: %u)",
		eventClassToStr(eventClass), eventTypeToStr(type), triggerTime);
}

Common::String TimerEvent::debugString() const {
	return Common::String::format("%s %s",
		eventClassToStr(eventClass), eventTypeToStr(type));
}

Common::String ActorEvent::debugString() const {
	return Common::String::format("%s %s (actorId: %u, arg: %s)",
		eventClassToStr(eventClass), eventTypeToStr(type),
		actorId, arg.getDebugString().c_str());
}

Common::String ScreenBranchEvent::debugString() const {
	return Common::String::format("%s %s (actorId: %u, screenId: %u, disableScreenAutoUpdate: %s)",
		eventClassToStr(eventClass), eventTypeToStr(type),
		actorId, screenId, disableScreenAutoUpdate ? "true" : "false");
}

Common::String MouseEvent::debugString() const {
	return Common::String::format("%s %s (position: %d, %d)",
		eventClassToStr(eventClass), eventTypeToStr(type),
		position.x, position.y);
}

Common::String KeyboardEvent::debugString() const {
	return Common::String::format("%s %s (keyCode: %u)",
		eventClassToStr(eventClass), eventTypeToStr(type), keyCode);
}

void EventLoop::run() {
	while (!g_engine->shouldQuit()) {
		// Do begin-of-frame sync.
		debugC(9, kDebugEvents, "***** START EVENT LOOP ***");
		g_engine->getTimerService()->queueExpiredTimerEvents();

		// Dispatch engine events.
		bool hadImtEvents = !_incomingQueue.empty();
		if (hadImtEvents) {
			dispatchImtEvents();
		}

		// Dispatch system events.
		bool hadSystemEvents = g_system->getEventManager()->pollEvent(_queuedSystemEvent);
		if (hadSystemEvents) {
			g_engine->dispatchOneSystemEvent(_queuedSystemEvent);
		}

		// Do pre-display sync. Only movies seem to use this to update their frame state
		// rather than setting a timer to do it.
		PreDisplaySyncState preDisplaySyncState = preDisplaySync();

		// Do display update.
		bool hadAnyEvents = hadSystemEvents || hadImtEvents;
		bool displayUpdateRequested = hadAnyEvents || preDisplaySyncState == kPreDisplaySyncForceScreenUpdate;
		bool displayUpdateAllowed = preDisplaySyncState != kPreDisplaySyncStateBlockScreenUpdate;
		if (displayUpdateRequested && displayUpdateAllowed) {
			updateDisplay();
		}

		// Yield CPU.
		g_system->delayMillis(15);

		// Do end-of-frame sync. The original only services sounds here.
		g_engine->serviceSounds();
		debugC(9, kDebugEvents, "***** END EVENT LOOP ***");
	}
}

void EventLoop::queueEvent(const Event &event) {
	_incomingQueue.push(Common::ScopedPtr<Event>(event.clone()));
}

void EventLoop::dispatchImtEvents() {
	// Move all incoming events to the dispatch queue so that events
	// generated during dispatch don't create an infinite loop.
	while (!_incomingQueue.empty()) {
		_dispatchQueue.push(_incomingQueue.pop());
	}

	while (!_dispatchQueue.empty()) {
		Common::ScopedPtr<Event> event = _dispatchQueue.pop();
		dispatchImtEvent(*event);
	}
}

void EventLoop::dispatchImtEvent(const Event &event) {
	// The original used polymorphism here that is needlessly complex to understand and
	// reimplement in this engine. So rather than having an EventHandler base class, we
	// will just dispatch the old-fashioned way.
	//
	// Display events are less likely to be interesting (and there will be a lot of them),
	// so they deserve a much higher debug channel.
	uint debugLevel = (event.eventClass == kEventClassDisplay) ? 9 : 6;
	debugC(debugLevel, kDebugEvents, "%s: %s", __func__, event.debugString().c_str());
	switch (event.eventClass) {
	case kEventClassActor: {
		// ActorEventHandler
		const ActorEvent &actorEvent = static_cast<const ActorEvent &>(event);
		Actor *actor = g_engine->getImtGod()->getActorById(actorEvent.actorId);
		if (actor != nullptr) {
			actor->onEvent(actorEvent);
		}
		break;
	}

	case kEventClassDisplay: {
		const DisplayEvent &displayEvent = static_cast<const DisplayEvent &>(event);
		g_engine->getDisplayUpdateManager()->onEvent(displayEvent);
		break;
	}

	case kEventClassTimerService: {
		const TimerServiceAlarmEvent &alarmEvent = static_cast<const TimerServiceAlarmEvent &>(event);
		g_engine->getTimerService()->handleEvent(alarmEvent);
		break;
	}

	case kEventClassMouse: {
		const MouseEvent &mouseEvent = static_cast<const MouseEvent &>(event);
		g_engine->getStageDirector()->handleMouseEvent(mouseEvent);
		break;
	}

	case kEventClassKeyboard: {
		const KeyboardEvent &keyboardEvent = static_cast<const KeyboardEvent &>(event);
		g_engine->getStageDirector()->handleKeyboardEvent(keyboardEvent);
		break;
	}

	default:
		warning("%s: Unhandled engine event: %s", __func__, event.debugString().c_str());
	}
}

void MediaStationEngine::dispatchOneSystemEvent(const Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE: {
		MouseEvent mouseEvent(kMouseMovedEvent, event.mouse);
		g_engine->getEventLoop()->queueEvent(mouseEvent);
		break;
	}

	case Common::EVENT_KEYDOWN: {
		KeyboardEvent keyboardEvent(kKeyDownEvent, event.kbd.ascii);
		_stageDirector->handleKeyboardEvent(keyboardEvent);
		break;
	}

	case Common::EVENT_LBUTTONDOWN: {
		MouseEvent mouseEvent(kMouseDownEvent, event.mouse);
		g_engine->getEventLoop()->queueEvent(mouseEvent);
		break;
	}

	case Common::EVENT_LBUTTONUP: {
		MouseEvent mouseEvent(kMouseUpEvent, event.mouse);
		g_engine->getEventLoop()->queueEvent(mouseEvent);
		break;
	}

	case Common::EVENT_FOCUS_LOST: {
		MouseEvent mouseEvent(kMouseOutOfFocusEvent, event.mouse);
		_stageDirector->handleMouseOutOfFocusEvent(mouseEvent);
		break;
	}

	default:
		// Avoid warnings about unimplemented cases by having an explicit
		// default case.
		break;
	}
}

PreDisplaySyncState EventLoop::preDisplaySync() {
	PreDisplaySyncState state = kPreDisplaySyncNoScreenUpdateRequested;
	for (auto it = _preDisplaySyncClients.begin(); it != _preDisplaySyncClients.end(); ++it) {
		PreDisplaySyncClient *client = it->_value;
		PreDisplaySyncState result = client->preDisplaySync();

		if (result == kPreDisplaySyncForceScreenUpdate) {
			if (state != kPreDisplaySyncStateBlockScreenUpdate) {
				state = kPreDisplaySyncForceScreenUpdate;
			}
		} else if (result == kPreDisplaySyncStateBlockScreenUpdate) {
			state = kPreDisplaySyncStateBlockScreenUpdate;
		}
	}

	return state;
}

void EventLoop::registerForPreDisplaySyncCalls(PreDisplaySyncClient *client) {
	_preDisplaySyncClients[client] = client;
}

void EventLoop::unregisterForPreDisplaySyncCalls(PreDisplaySyncClient *client) {
	_preDisplaySyncClients.erase(client);
}

void EventLoop::updateDisplay() {
	const uint NO_DISABLE_DISPLAY_UPDATE_TOKEN = 0;
	DisplayEvent displayEvent(kDisplayAutoUpdateEvent, NO_DISABLE_DISPLAY_UPDATE_TOKEN);
	dispatchImtEvent(displayEvent);
}

void PreDisplaySyncClient::registerForSyncCalls() {
	g_engine->getEventLoop()->registerForPreDisplaySyncCalls(this);
}

void PreDisplaySyncClient::unregisterForSyncCalls() {
	g_engine->getEventLoop()->unregisterForPreDisplaySyncCalls(this);
}

uint32 TimerEventReceiver::currentReceiverTime() {
	return g_engine->getTotalPlayTime();
}

TimerEntry::~TimerEntry() {
	g_engine->getTimerService()->stopTimer(*this);
}

uint32 TimerEntry::calculateFirstExpirationTime(uint32 currentTime) {
	// Get scheduled expiration time from receiver's current time.
	_expirationTime = currentTime + _duration;
	return _expirationTime;
}

uint32 TimerEntry::calculateNextExpirationTime(uint32 currentTime) {
	// Keep adding duration to the expiration time until it exceeds the current time.
	// This ensures recurring timers "catch up" if execution was delayed.
	while (_expirationTime <= currentTime) {
		_expirationTime += _duration;
	}
	return _expirationTime;
}

uint32 TimerEntry::deltaTimeAtExpiration() const {
	// Get difference between when timer actually fired and when it was scheduled.
	if (_actualExpirationTime < _expirationTime) {
		return 0;
	} else {
		return _actualExpirationTime - _expirationTime;
	}
}

bool TimerEntry::operator==(const TimerEntry &other) const {
	return _receiver == other._receiver &&
		_expirationTime == other._expirationTime &&
		_actualExpirationTime == other._actualExpirationTime &&
		_duration == other._duration &&
		_shouldRepeat == other._shouldRepeat;
}

bool TimerEntry::operator<(const TimerEntry &other) const {
	return deltaTimeAtExpiration() < other.deltaTimeAtExpiration();
}

bool TimerEntry::operator>(const TimerEntry &other) const {
	return deltaTimeAtExpiration() > other.deltaTimeAtExpiration();
}

void TimerService::handleEvent(const TimerServiceAlarmEvent &event) {
	switch (event.type) {
	case kTimerServiceAlarmEvent: {
		if (_timers.contains(event.entry)) {
			stopTimer(*event.entry);
			TimerEvent timerEvent(event.entry);
			event.entry->getReceiver()->timerEvent(timerEvent);

			TimerEntry *entry = _timers.getValOrDefault(event.entry);
			if (entry != nullptr) {
				// Most timers are NOT rescheduled here but are
				// rescheduled from the timer event called above.
				rescheduleTimerEntry(*entry);
			}
		}
		break;
	}

	default:
		warning("%s: Unexpected event: %s", __func__, event.debugString().c_str());
	}
}

void TimerService::startTimer(TimerEntry &entry, uint32 duration) {
	stopTimer(entry);
	entry.setDuration(duration);
	if (entry.getReceiver() == nullptr) {
		warning("%s: Can't start timer without receiver", __func__);
		return;
	}

	uint32 currentReceiverTime = entry.getReceiver()->currentReceiverTime();
	uint32 expirationTime = entry.calculateFirstExpirationTime(currentReceiverTime);
	debugC(5, kDebugEvents, "%s: Starting timer for %d ms (expiration: %d)", __func__, duration, expirationTime);
	_timers.setVal(&entry, &entry);
}

void TimerService::startTimer(TimerEntry &entry, double duration) {
	startTimer(entry, static_cast<uint32>(duration * 1000));
}

void TimerService::stopTimer(TimerEntry &entry) {
	_timers.erase(&entry);
}

void TimerService::rescheduleTimerEntry(TimerEntry &entry) {
	if (entry.shouldReschedule()) {
		// Calculate the next expiration time.
		uint32 currentTime = entry.getReceiver()->currentReceiverTime();
		entry.calculateNextExpirationTime(currentTime);
	}
}

void TimerService::queueExpiredTimerEvents() {
	debugC(7, kDebugEvents, "*** START CHECKING TIMERS (%d timers) ***", _timers.size());
	for (auto it = _timers.begin(); it != _timers.end(); ++it) {
		// Timer has expired if current time is greater than or equal to expiration time.
		TimerEntry *entry = it->_value;
		uint32 currentTime = entry->getReceiver()->currentReceiverTime();
		uint32 expirationTime = entry->expirationTime();
		bool hasExpired = currentTime >= expirationTime;
		debugC(7, kDebugEvents, "%s: Checking timer (current: %d ms, expiration: %d ms)", __func__, currentTime, expirationTime);

		if (hasExpired) {
			// The original put this in a _queueTimerServiceAlarmFor method,
			// but it is only called here, so it is inlined.
			TimerServiceAlarmEvent event(entry, currentTime);
			g_engine->getEventLoop()->queueEvent(event);
		}
	}
	debugC(7, kDebugEvents, "*** END CHECKING TIMERS ***");
}

} // End of namespace MediaStation
