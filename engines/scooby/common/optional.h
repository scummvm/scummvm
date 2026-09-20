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

#ifndef SCOOBY_OPTIONAL_H
#define SCOOBY_OPTIONAL_H

// A tiny std::optional<T> replacement for C++11 (std::optional is C++17).
// Used wherever the original C# source declares a nullable value type
// (e.g. "int? offset"). Intentionally minimal: this project only ever
// stores small, trivially-copyable value types (ints, enums) in it.

namespace Scooby {
template<typename T>
class Optional {
public:
	Optional() : _hasValue(false), _value() {
	}

	Optional(const T &value) : _hasValue(true), _value(value) {
	} // NOLINT(*-explicit-constructor)

	Optional &operator=(const T &value) {
		_hasValue = true;
		_value = value;
		return *this;
	}

	void reset() {
		_hasValue = false;
		_value = T();
	}

	bool hasValue() const { return _hasValue; }

	explicit operator bool() const { return _hasValue; }

	const T &value() const { return _value; }

	T &value() { return _value; }

	T valueOr(const T &fallback) const { return _hasValue ? _value : fallback; }

	bool operator==(const Optional &other) const {
		if (_hasValue != other._hasValue) {
			return false;
		}
		return !_hasValue || _value == other._value;
	}

	bool operator!=(const Optional &other) const { return !(*this == other); }

private:
	bool _hasValue;
	T _value;
};
} // namespace Scooby

#endif // SCOOBY_OPTIONAL_H
