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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef BURIED_MESSAGE_H
#define BURIED_MESSAGE_H

#include "common/keyboard.h"
#include "common/rect.h"

namespace Buried {

// This is all based on the Windows messaging system
// ...but less stupid

class Window;

enum MessageType {
	// Actual message types
	kMessageTypeKeyUp,
	kMessageTypeKeyDown,
	kMessageTypeTimer,
	kMessageTypeMouseMove,
	kMessageTypeLButtonUp,
	kMessageTypeLButtonDown,
	kMessageTypeMButtonUp,
	kMessageTypeRButtonUp,
	kMessageTypeRButtonDown,
	kMessageTypeSetCursor,
	kMessageTypeEnable,

	// Ranges of messages
	kMessageTypeMouseBegin = kMessageTypeMouseMove,
	kMessageTypeMouseEnd = kMessageTypeRButtonDown,
	kMessageTypeKeyBegin = kMessageTypeKeyUp,
	kMessageTypeKeyEnd = kMessageTypeKeyDown
};


class Message {
public:
	virtual ~Message() {}

	virtual MessageType getMessageType() const = 0;
};

// Templated version to make stuff a bit neater
template <MessageType type>
class MessageTypeIntern : public Message {
public:
	MessageType getMessageType() const { return type; }
};

// Some common template classes
template <MessageType type>
class MouseMessage : public MessageTypeIntern<type> {
public:
	MouseMessage(const Common::Point &point, uint flags) : _point(point), _flags(flags) {}

	Common::Point getPoint() const { return _point; }
	uint getFlags() const { return _flags; }

private:
	Common::Point _point;
	uint _flags;
};

template <MessageType type>
class KeyMessage : public MessageTypeIntern<type> {
public:
	KeyMessage(const Common::KeyState &keyState, uint flags) : _keyState(keyState), _flags(flags) {}

	Common::KeyState getKeyState() const { return _keyState; }
	uint getFlags() const { return _flags; }

private:
	Common::KeyState _keyState;
	uint _flags;
};

// Types for everything that falls under one of the above categories
typedef KeyMessage<kMessageTypeKeyUp>                  KeyUpMessage;
typedef KeyMessage<kMessageTypeKeyDown>                KeyDownMessage;
typedef MouseMessage<kMessageTypeMouseMove>            MouseMoveMessage;
typedef MouseMessage<kMessageTypeLButtonUp>            LButtonUpMessage;
typedef MouseMessage<kMessageTypeLButtonDown>          LButtonDownMessage;
typedef MouseMessage<kMessageTypeMButtonUp>            MButtonUpMessage;
typedef MouseMessage<kMessageTypeRButtonUp>            RButtonUpMessage;
typedef MouseMessage<kMessageTypeRButtonDown>          RButtonDownMessage;

// ...and the rest
class SetCursorMessage : public MessageTypeIntern<kMessageTypeSetCursor> {
public:
	// Hit-test is purposely skipped
	SetCursorMessage(uint message) : _message(message) {}

	uint getMessage() const { return _message; }

private:
	uint _message;
};

class TimerMessage : public MessageTypeIntern<kMessageTypeTimer> {
public:
	TimerMessage(uint timer) : _timer(timer) {}

	uint getTimer() const { return _timer; }

private:
	uint _timer;
};

class EnableMessage : public MessageTypeIntern<kMessageTypeEnable> {
public:
	EnableMessage(bool enable) : _enable(enable) {}

	bool getEnable() const { return _enable; }

private:
	bool _enable;
};

} // End of namespace Buried

#endif
