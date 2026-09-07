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

#ifndef ULTIMA_SHARED_ENGINE_MESSAGES_H
#define ULTIMA_SHARED_ENGINE_MESSAGES_H

#include "common/array.h"
#include "common/events.h"

namespace Ultima {
namespace Shared {

class UIElement;

namespace Messages {

struct Message {};

struct FocusMessage : public Message {
	UIElement *_priorView = nullptr;
	FocusMessage() : Message() {
	}
	FocusMessage(UIElement *priorView) : Message(),
		_priorView(priorView) {
	}
};

struct UnfocusMessage : public Message {};
struct MouseEnterMessage : public Message {};
struct MouseLeaveMessage : public Message {};

struct KeypressMessage : public Message, public Common::KeyState {
	KeypressMessage() : Message() {
	}
	KeypressMessage(const Common::KeyState &ks) :
		Message(), Common::KeyState(ks) {
	}
};

struct MouseMessage : public Message {
	enum Button {
		MB_LEFT, MB_RIGHT, MB_MIDDLE
	};
	Button _button;
	Common::Point _pos;

	MouseMessage() : Message(), _button(MB_LEFT) {
	}
	MouseMessage(Button btn, const Common::Point &pos) :
		Message(), _button(btn), _pos(pos) {
	}
	MouseMessage(Common::EventType type, const Common::Point &pos);
};
struct MouseDownMessage : public MouseMessage {
	MouseDownMessage() : MouseMessage() {
	}
	MouseDownMessage(Button btn, const Common::Point &pos) :
		MouseMessage(btn, pos) {
	}
	MouseDownMessage(Common::EventType type, const Common::Point &pos) :
		MouseMessage(type, pos) {
	}
};
struct MouseUpMessage : public MouseMessage {
	MouseUpMessage() : MouseMessage() {
	}
	MouseUpMessage(Button btn, const Common::Point &pos) :
		MouseMessage(btn, pos) {
	}
	MouseUpMessage(Common::EventType type, const Common::Point &pos) :
		MouseMessage(type, pos) {
	}
};
typedef MouseMessage MouseMoveMessage;

struct GameMessage : public Message {
	Common::String _name;
	int _value;
	Common::String _stringValue;

	GameMessage() : Message(), _value(-1) {
	}
	GameMessage(const Common::String &name) : Message(),
		_name(name), _value(-1) {
	}
	GameMessage(const Common::String &name, int value) : Message(),
		_name(name), _value(value) {
	}
	GameMessage(const Common::String &name, const Common::String &value) :
		Message(), _name(name), _stringValue(value) {
	}
};

struct ValueMessage : public Message {
	int _value;

	ValueMessage() : Message(), _value(0) {
	}
	ValueMessage(int value) : Message(),
		_value(value) {
	}
};

struct ActionMessage : public Message {
	int _action;
	ActionMessage() : Message(), _action(0) {
	}
	ActionMessage(int action) : Message(),
		_action(action) {
	}
};

/**
 * Sent to a map view to have it briefly overlay a tile directly on-screen
 * at a given map position - e.g. a projectile flying towards the player,
 * or a hit flash - before restoring the view back to normal. (x, y) are
 * in map/world tile coordinates, not screen pixels; it's up to the view
 * receiving the message to convert that to its own on-screen position.
 * The handler is expected to pause briefly before returning, so combat
 * logic that shows several of these in a row (e.g. multiple attackers in
 * a single turn) will naturally have each shown with a delay in turn
 */
struct AttackTileMessage : public Message {
	int _x, _y;
	int _tileId;

	AttackTileMessage() : Message(), _x(0), _y(0), _tileId(0) {
	}
	AttackTileMessage(int x, int y, int tileId) : Message(),
		_x(x), _y(y), _tileId(tileId) {
	}
};

} // namespace Messages
} // namespace Shared
} // namespace Ultima

#endif
