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
#include "mm/mm1/metaengine.h"

namespace MM {
namespace MM1 {

enum TextAlign {
	ALIGN_LEFT, ALIGN_RIGHT, ALIGN_MIDDLE
};

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
	int _value;
	Common::String _stringValue;

	GameMessage() : Message(), _value(-1) {}
	GameMessage(const Common::String &name) : Message(),
		_name(name), _value(-1) {}
	GameMessage(const Common::String &name, int value) : Message(),
		_name(name), _value(value) {}
	GameMessage(const Common::String &name, const Common::String &value) :
		Message(), _name(name), _stringValue(value), _value(-1) {}
};

struct HeaderMessage : public Message {
	Common::String _name;
	HeaderMessage() : Message() {}
	HeaderMessage(const Common::String &name) : Message(),
		_name(name) {}
};

struct Line : public Common::Point {
	Common::String _text;
	TextAlign _align = ALIGN_LEFT;

	Line() {
	}
	Line(const Common::String &text, TextAlign align = ALIGN_LEFT) :
		Common::Point(-1, -1), _text(text), _align(align) {
	}
	Line(int x1, int y1, const Common::String &text,
			TextAlign align = ALIGN_LEFT) :
		Common::Point(x1, y1), _text(text), _align(align) {
	}

	size_t size() const;
};
typedef Common::Array<Line> LineArray;

typedef void (*YNCallback)();
typedef void (*KeyCallback)(const Common::KeyState &keyState);
struct InfoMessage : public Message {
	LineArray _lines;
	YNCallback _ynCallback = nullptr;
	KeyCallback _keyCallback = nullptr;
	YNCallback &_timeoutCallback = _ynCallback;
	bool _largeMessage = false;
	bool _sound = false;
	int _delaySeconds = 0;

	InfoMessage();
	InfoMessage(const Common::String &str, TextAlign align = ALIGN_LEFT);
	InfoMessage(int x, int y, const Common::String &str, TextAlign align = ALIGN_LEFT);
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

	InfoMessage &operator=(const InfoMessage &src);
};

struct SoundMessage : public InfoMessage {
public:
	SoundMessage() : InfoMessage() { _sound = true; }
	SoundMessage(const Common::String &str, TextAlign align = ALIGN_LEFT) :
		InfoMessage(0, 1, str, align) { _sound = true; }
	SoundMessage(int x, int y, const Common::String &str,
		TextAlign align = ALIGN_LEFT) :
		InfoMessage(x, y, str, align) { _sound = true; }
	SoundMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2) :
		InfoMessage(x1, y1, str1, x2, y2, str2) { _sound = true; }

	SoundMessage(const Common::String &str,
		YNCallback ynCallback) :
		InfoMessage(0, 1, str, ynCallback) { _sound = true; }
	SoundMessage(int x, int y, const Common::String &str,
		YNCallback ynCallback) :
		InfoMessage(x, y, str, ynCallback) { _sound = true; }
	SoundMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		YNCallback ynCallback) :
		InfoMessage(x1, y1, str1, x2, y2, str2, ynCallback) { _sound = true; }

	SoundMessage(const Common::String &str,
		KeyCallback keyCallback) :
		InfoMessage(0, 1, str, keyCallback) { _sound = true; }
	SoundMessage(int x, int y, const Common::String &str,
		KeyCallback keyCallback) :
		InfoMessage(x, y, str, keyCallback) { _sound = true; }
	SoundMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		KeyCallback keyCallback) :
		InfoMessage(x1, y1, str1, x2, y2, str2, keyCallback) { _sound = true; }
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

struct DrawGraphicMessage : public Message {
	int _gfxNum;

	DrawGraphicMessage() : Message(), _gfxNum(0) {}
	explicit DrawGraphicMessage(int gfxNum) : Message(),
		_gfxNum(gfxNum) {}
};

} // namespace MM1
} // namespace MM

#endif
