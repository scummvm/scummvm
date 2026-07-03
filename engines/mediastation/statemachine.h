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

#ifndef MEDIASTATION_STATEMACHINE_H
#define MEDIASTATION_STATEMACHINE_H

#include "common/textconsole.h"
#include "common/queue.h"

namespace MediaStation {

template<typename EventType>
struct StateMachineEvent {
	EventType eventType;
};

// The original had a very generalized finite state machine supported by state matrix, function pointer,
// and other classes, but that was judged needlessly complex to reimplement here. The state transition logic
// is embedded directly into this class.
template<typename StateType, typename EventType>
class StateMachine {
public:
	virtual ~StateMachine() {};

	void queueEvent(EventType event);
	void runIfNotNested();
	void executeForever();

protected:
	StateType _currentState;
	Common::Queue<EventType> _events;
	bool _handlingEvents = false;
	virtual void executeNextState(EventType eventType) = 0;
	void warnOnInvalidTransition(EventType eventType);
};

template<typename StateType, typename EventType>
void StateMachine<StateType, EventType>::queueEvent(EventType event) {
	_events.push(event);
}

template<typename StateType, typename EventType>
void StateMachine<StateType, EventType>::runIfNotNested() {
	if (!_handlingEvents) {
		_handlingEvents = true;
		executeForever();
		_handlingEvents = false;
	}
}

template<typename StateType, typename EventType>
void StateMachine<StateType, EventType>::executeForever() {
	while (!_events.empty()) {
		EventType eventType = _events.pop();
		executeNextState(eventType);
	}
}

template<typename StateType, typename EventType>
void StateMachine<StateType, EventType>::warnOnInvalidTransition(EventType eventType) {
	warning("Got invalid event %d for state %d", static_cast<uint>(eventType), static_cast<uint>(_currentState));
}

} // End of namespace MediaStation

#endif
