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

#ifndef MM1_MESSAGES_H
#define MM1_MESSAGES_H

#include "common/array.h"
#include "common/events.h"
#include "common/str.h"
#include "mm/mm1/meta_engine.h"

namespace MM {
namespace MM1 {

struct Message {};
struct FocusMessage : public Message {};
struct UnfocusMessage : public Message {};
struct ActionMessage : public Message {
	KeybindingAction _action;
	ActionMessage() : Message(), _action(KEYBIND_NONE) {}
	ActionMessage(KeybindingAction action) : Message(),
		_action(action) {}
};

struct KeypressMessage : public Message, public Common::KeyState {
	KeypressMessage() : Message() {}
	KeypressMessage(const Common::KeyState &ks) :
		Message(), Common::KeyState(ks) {}
};

struct MouseMessage : public Message {
	enum Button { MB_LEFT, MB_RIGHT, MB_MIDDLE };
	Button _button;
	Common::Point _pos;

	MouseMessage() : Message(), _button(MB_LEFT) {}
	MouseMessage(Button btn, const Common::Point &pos) :
		Message(), _button(btn), _pos(pos) {}
	MouseMessage(Common::EventType type, const Common::Point &pos);
};
struct MouseDownMessage : public MouseMessage {
	MouseDownMessage() : MouseMessage() {}
	MouseDownMessage(Button btn, const Common::Point &pos) :
		MouseMessage(btn, pos) {}
	MouseDownMessage(Common::EventType type, const Common::Point &pos) :
		MouseMessage(type, pos) {}
};
struct MouseUpMessage : public MouseMessage {
	MouseUpMessage() : MouseMessage() {}
	MouseUpMessage(Button btn, const Common::Point &pos) :
		MouseMessage(btn, pos) {}
	MouseUpMessage(Common::EventType type, const Common::Point &pos) :
		MouseMessage(type, pos) {}
};

struct GameMessage : public Message {
	Common::String _name;
	GameMessage() : Message() {}
	GameMessage(const Common::String &name) : Message(),
		_name(name) {}
};

struct Line : public Common::Point {
	Common::String _text;

	Line() {
	}
	Line(const Common::String &text) :
		Common::Point(-1, -1), _text(text) {
	}
	Line(int x1, int y1, const Common::String &text) :
		Common::Point(x1, y1), _text(text) {
	}
};
typedef Common::Array<Line> LineArray;

typedef void (*YNCallback)();
typedef void (*KeyCallback)(const Common::KeyState &keyState);
struct InfoMessage : public Message {
	LineArray _lines;
	YNCallback _ynCallback = nullptr;
	KeyCallback _keyCallback = nullptr;
	bool _largeMessage = false;

	InfoMessage();
	InfoMessage(const Common::String &str);
	InfoMessage(int x, int y, const Common::String &str);
	InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2);

	InfoMessage(const Common::String &str,
		YNCallback ynCallback);
	InfoMessage(int x, int y, const Common::String &str,
		YNCallback ynCallback);
	InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		YNCallback ynCallback);

	InfoMessage(const Common::String &str,
		KeyCallback keyCallback);
	InfoMessage(int x, int y, const Common::String &str,
		KeyCallback keyCallback);
	InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		KeyCallback keyCallback);
};

enum LocationType {
	LOC_TRAINING = 0, LOC_MARKET = 1, LOC_TEMPLE = 2,
	LOC_BLACKSMITH = 3, LOC_TAVERN = 4
};

struct ValueMessage : public Message {
	int _value;

	ValueMessage() : Message(), _value(0) {}
	ValueMessage(int value) : Message(),
		_value(value) {}
};

} // namespace MM1
} // namespace MM

#endif
