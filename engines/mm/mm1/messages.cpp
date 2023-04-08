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

#include "mm/mm1/messages.h"
#include "mm/mm1/mm1.h"

namespace MM {
namespace MM1 {

MouseMessage::MouseMessage(Common::EventType type,
		const Common::Point &pos) : Message(), _pos(pos) {
	switch (type) {
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
		_button = MB_RIGHT;
		break;
	case Common::EVENT_MBUTTONDOWN:
	case Common::EVENT_MBUTTONUP:
		_button = MB_MIDDLE;
		break;
	default:
		_button = MB_LEFT;
		break;
	}
}

InfoMessage::InfoMessage() : Message() {}

InfoMessage::InfoMessage(const Common::String &str,
		TextAlign align) : Message() {
	_lines.push_back(Line(0, 0, str, align));
}

InfoMessage::InfoMessage(int x, int y, const Common::String &str,
		TextAlign align) {
	_lines.push_back(Line(x, y, str, align));
}

InfoMessage::InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2) {
	_lines.push_back(Line(x1, y1, str1));
	_lines.push_back(Line(x2, y2, str2));
}

InfoMessage::InfoMessage(const Common::String &str, YNCallback yCallback,
			YNCallback nCallback) :
		Message(), _yCallback(yCallback), _nCallback(nCallback) {
	_lines.push_back(str);
}

InfoMessage::InfoMessage(int x, int y, const Common::String &str,
		YNCallback yCallback, YNCallback nCallback) :
		Message(), _yCallback(yCallback), _nCallback(nCallback) {
	_lines.push_back(Line(x, y, str));
}

InfoMessage::InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2,
		YNCallback yCallback, YNCallback nCallback) :
		Message(), _yCallback(yCallback), _nCallback(nCallback) {
	_lines.push_back(Line(x1, y1, str1));
	_lines.push_back(Line(x2, y2, str2));
}

InfoMessage::InfoMessage(const Common::String &str, KeyCallback keyCallback) :
	Message(), _keyCallback(keyCallback) {
	_lines.push_back(str);
}

InfoMessage::InfoMessage(int x, int y, const Common::String &str, KeyCallback keyCallback) :
		Message(), _keyCallback(keyCallback) {
	_lines.push_back(Line(x, y, str));
}

InfoMessage::InfoMessage(int x1, int y1, const Common::String &str1,
		int x2, int y2, const Common::String &str2, KeyCallback keyCallback) :
		Message(), _keyCallback(keyCallback) {
	_lines.push_back(Line(x1, y1, str1));
	_lines.push_back(Line(x2, y2, str2));
}

InfoMessage &InfoMessage::operator=(const InfoMessage &src) {
	_lines = src._lines;
	_yCallback = src._yCallback;
	_nCallback = src._nCallback;
	_keyCallback = src._keyCallback;
	_largeMessage = src._largeMessage;
	_sound = src._sound;
	_delaySeconds = src._delaySeconds;
	return *this;
}

size_t Line::size() const {
	return _text.size();
}

SoundMessage::SoundMessage(const Common::String &str, TextAlign align) :
	InfoMessage(0, g_engine->isEnhanced() ? 0 : 1, str, align) {
	_sound = true;
}

SoundMessage::SoundMessage(const Common::String &str,
	YNCallback yCallback, YNCallback nCallback) :
	InfoMessage(0, g_engine->isEnhanced() ? 0 : 1, str, yCallback, nCallback) {
	_sound = true;
}

SoundMessage::SoundMessage(const Common::String &str,
	KeyCallback keyCallback) :
	InfoMessage(0, g_engine->isEnhanced() ? 0 : 1, str, keyCallback) {
	_sound = true;
}

} // namespace MM1
} // namespace MM
