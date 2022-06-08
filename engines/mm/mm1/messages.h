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

struct GameMessage : public Message {
	Common::String _name;
	GameMessage() : Message() {}
	GameMessage(const Common::String &name) : Message(),
		_name(name) {}
};

typedef void (*YNCallback)();
struct InfoMessage : public Message {
	struct Line : public Common::Point {
		Common::String _text;

		Line() {}
		Line(const Common::String &text) :
			Common::Point(-1, -1), _text(text) {}
		Line(int x, int y, const Common::String &text) :
			Common::Point(x, y), _text(text) {}
	};
	Common::Array<Line> _lines;
	YNCallback _ynCallback;

	InfoMessage();
	InfoMessage(const Common::String &str,
		YNCallback ynCallback = nullptr);
	InfoMessage(int x, int y, const Common::String &str,
		YNCallback ynCallback = nullptr);
	InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		YNCallback ynCallback = nullptr);
};

struct ValueMessage : public Message {
	int _value;

	ValueMessage() : Message(), _value(0) {}
	ValueMessage(int value) : Message(),
		_value(value) {}
};
typedef ValueMessage BusinessMessage;

} // namespace MM1
} // namespace MM

#endif
