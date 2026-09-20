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

#ifndef SCOOBY_SPAN_H
#define SCOOBY_SPAN_H

// #include <cstddef>
#include "common/type_traits.h"

#include "common/array.h"

// A tiny, non-owning contiguous-range view standing in for .NET's
// Span<T>/ReadOnlySpan<T> (std::span is C++20, unavailable at C++11).
// ReadOnlySpan<T> becomes Span<const T>; Span<T> stays Span<T>.

namespace Scooby {
template<typename T>
class Span {
public:
	typedef T value_type;
	typedef T *iterator;

	Span() : _data(nullptr), _size(0) {
	}

	Span(T *data, std::size_t size) : _data(data), _size(size) {
	}

	// Allows Span<T> to convert to Span<const T> implicitly, matching the
	// ReadOnlySpan<T> conversion the C# source relies on everywhere.
	template<typename U>
	Span(const Span<U> &other,
		 typename std::enable_if<std::is_same<T, const U>::value>::type * = nullptr)
		: _data(other.data()), _size(other.size()) {
	}

	T *data() const { return _data; }
	std::size_t size() const { return _size; }
	bool empty() const { return _size == 0; }
	bool isEmpty() const { return _size == 0; }

	T &operator[](std::size_t index) const { return _data[index]; }

	iterator begin() const { return _data; }
	iterator end() const { return _data + _size; }

	Span<T> slice(std::size_t start) const { return Span<T>(_data + start, _size - start); }

	Span<T> slice(std::size_t start, std::size_t length) const { return Span<T>(_data + start, length); }

	void fill(const T &value) const {
		for (std::size_t i = 0; i < _size; ++i) {
			_data[i] = value;
		}
	}

	void copyTo(const Span<T> &destination) const {
		for (std::size_t i = 0; i < _size; ++i) {
			destination[i] = _data[i];
		}
	}

	Common::Array<typename Common::remove_const<T>::type> toArray() const {
		return Common::Array<typename Common::remove_const<T>::type>(_data, _size);
	}

private:
	T *_data;
	std::size_t _size;
};

template<typename T>
inline Span<T> MakeSpan(Common::Array<T> &a) {
	return Span<T>(a.data(), a.size());
}

template<typename T>
inline Span<const T> MakeSpan(const Common::Array<T> &a) {
	return Span<const T>(a.data(), a.size());
}

template<typename T, std::size_t N>
inline Span<T> MakeSpan(T (&a)[N]) {
	return Span<T>(a, N);
}
} // namespace Scooby

#endif // SCOOBY_SPAN_H
