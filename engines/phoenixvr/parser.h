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

#ifndef PHOENIXVR_PARSER_H
#define PHOENIXVR_PARSER_H

#include "common/array.h"
#include "common/str.h"

namespace PhoenixVR {
class Parser {
	const Common::String &_line;
	uint _lineno;
	uint _pos;

public:
	Parser(const Common::String &line, uint lineno) : _line(line), _lineno(lineno), _pos(0) {
		skip();
	}

	void skip() {
		// comment found in amerzone ",*****"
		if (_pos == 0 && _line[_pos] == ',') {
			_pos = _line.size();
			return;
		}
		while (_pos < _line.size() && Common::isSpace(_line[_pos]))
			++_pos;
		if (_pos < _line.size() && _line[_pos] == ';')
			_pos = _line.size();
	}

	bool atEnd() const { return _pos >= _line.size(); }

	int peek() const { return _pos < _line.size() ? _line[_pos] : 0; }
	int next() { return _pos < _line.size() ? _line[_pos++] : 0; }

	void expect(int expected) {
		skip();
		auto ch = next();
		if (ch != expected)
			error("expected '%c' at line %u, got %c in %s", expected, _lineno, ch, _line.c_str());
		skip();
	}

	bool maybe(char ch) {
		skip();
		if (peek() == ch) {
			next();
			return true;
		} else
			return false;
	}

	bool peek(const Common::String &prefix) {
		skip();
		if (_pos + prefix.size() > _line.size())
			return false;

		auto n = prefix.size();
		auto *ch0 = _line.c_str() + _pos;
		auto *ch1 = prefix.c_str();
		while (n--) {
			if (tolower(*ch0++) != tolower(*ch1++))
				return false;
		}
		return true;
	}

	bool maybe(const Common::String &prefix) {
		if (peek(prefix)) {
			_pos += prefix.size();
			skip();
			return true;
		}
		return false;
	}

	bool keyword(const Common::String &prefix) {
		skip();
		bool yes = peek(prefix);
		// keywords ends either on non-alpha or eof
		if (yes) {
			auto end = _pos + prefix.size();
			if (end >= _line.size() || !Common::isAlpha(_line[end])) {
				_pos += prefix.size();
				skip();
				return true;
			}
		}
		return false;
	}

	Common::String nextArg() {
		skip();
		auto begin = _pos;
		while (_pos < _line.size() && _line[_pos] != ',' && _line[_pos] != ')')
			++_pos;
		auto end = _pos;
		while (end > begin && Common::isSpace(_line[end - 1]))
			--end;
		skip();
		return _line.substr(begin, end - begin);
	}

	int nextInt() {
		bool negative = false;
		int value = 0;
		if (maybe('-'))
			negative = true;
		do {
			auto ch = next();
			if (ch < '0' || ch > '9')
				error("expected digit at %d, line: %u, %s", _pos, _lineno, _line.c_str());
			value = value * 10 + (ch - '0');
		} while (Common::isDigit(peek()));
		return negative ? -value : value;
	}

	Common::String nextWord() {
		skip();
		auto begin = _pos;
		while (_pos < _line.size() && !Common::isSpace(_line[_pos]) && _line[_pos] != ',' && _line[_pos] != '(' && _line[_pos] != '=' && _line[_pos] != ')')
			++_pos;
		auto end = _pos;
		skip();
		return _line.substr(begin, end - begin);
	}

	Common::String nextString() {
		skip();
		expect('"');
		Common::String str;
		while (_pos < _line.size()) {
			auto ch = _line[_pos++];
			if (ch == '"')
				break;
			str += ch;
		}
		skip();
		return str;
	}

	Common::Array<Common::String> readStringList() {
		Common::Array<Common::String> list;
		while (!atEnd() && peek() != ')') {
			if (peek() == '"')
				list.push_back(nextString());
			else {
				list.push_back(nextArg());
			}
			if (!atEnd() && peek() != ')')
				expect(',');
		}
		return list;
	}
};
} // namespace PhoenixVR

#endif
