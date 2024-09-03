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

#ifndef MTROPOLIS_COROUTINE_RETURN_VALUE_H
#define MTROPOLIS_COROUTINE_RETURN_VALUE_H

namespace MTropolis {

struct CoroutineReturnValueRefBase {
};

template<class T>
struct CoroutineReturnValueRef : public CoroutineReturnValueRefBase {
	T *_returnValue;

	CoroutineReturnValueRef() : _returnValue(nullptr) {
	}

	explicit CoroutineReturnValueRef(T *returnValue) : _returnValue(returnValue) {
	}

	inline void set(const T &value) const {
		if (_returnValue != nullptr)
			*_returnValue = value;
	}

	inline void set(T &&value) const {
		if (_returnValue != nullptr)
			*_returnValue = static_cast<T &&>(value);
	}

	inline static bool isVoid() {
		return false;
	}
};

template<>
struct CoroutineReturnValueRef<void> : public CoroutineReturnValueRefBase {
	CoroutineReturnValueRef() {
	}

	inline void voidSet() const {
	}

	inline static bool isVoid() {
		return true;
	}
};

} // End of namespace MTropolis

#endif
