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

#ifndef ALCACHOFA_COMMON_H
#define ALCACHOFA_COMMON_H

#include "common/rect.h"
#include "common/serializer.h"
#include "common/stream.h"
#include "common/stack.h"
#include "math/vector2d.h"
#include "math/vector3d.h"

namespace Alcachofa {

enum class CursorType {
	Point,
	LeaveUp,
	LeaveRight,
	LeaveDown,
	LeaveLeft,
	WalkTo
};

enum class Direction {
	Up,
	Right,
	Down,
	Left,

	Invalid = -1
};

enum class MainCharacterKind {
	None,
	Mortadelo,
	Filemon
};

enum class EasingType {
	Linear,
	InOut,
	In,
	Out
};

constexpr const int32 kDirectionCount = 4;
constexpr const int8 kOrderCount = 70;
constexpr const int8 kForegroundOrderCount = 10;

struct Color {
	uint8 r, g, b, a;
};
static constexpr const Color kWhite = { 255, 255, 255, 255 };
static constexpr const Color kBlack = { 0, 0, 0, 255 };
static constexpr const Color kClear = { 0, 0, 0, 0 };
static constexpr const Color kDebugRed = { 250, 0, 0, 70 };
static constexpr const Color kDebugGreen = { 0, 255, 0, 85 };
static constexpr const Color kDebugBlue = { 0, 0, 255, 110 };
static constexpr const Color kDebugLightBlue = { 80, 80, 255, 190 };

/**
 * @brief This *fake* semaphore does not work in multi-threaded scenarios
 * It is used as a safer option for a simple "isBusy" counter
 */
struct FakeSemaphore {
	FakeSemaphore(const char *name, uint initialCount = 0);
	~FakeSemaphore();

	inline bool isReleased() const { return _counter == 0; }
	inline uint counter() const { return _counter; }

	static void sync(Common::Serializer &s, FakeSemaphore &semaphore);
private:
	friend struct FakeLock;
	const char *const _name;
	uint _counter = 0;
};

struct FakeLock {
	FakeLock();
	FakeLock(const char *name, FakeSemaphore &semaphore);
	FakeLock(const FakeLock &other);
	FakeLock(FakeLock &&other) noexcept;
	~FakeLock();
	void operator = (FakeLock &&other) noexcept;
	void release();

	inline bool isReleased() const { return _semaphore == nullptr; }
private:
	void debug(const char *action);

	const char *_name = "<uninitialized>";
	FakeSemaphore *_semaphore = nullptr;
};

float ease(float t, EasingType type);

Math::Vector3d as3D(const Math::Vector2d &v);
Math::Vector3d as3D(Common::Point p);
Math::Vector2d as2D(const Math::Vector3d &v);
Math::Vector2d as2D(Common::Point p);

bool readBool(Common::ReadStream &stream);
Common::Point readPoint(Common::ReadStream &stream);
Common::String readVarString(Common::ReadStream &stream);
void skipVarString(Common::SeekableReadStream &stream);
void syncPoint(Common::Serializer &serializer, Common::Point &point);

template<typename T>
inline void syncArray(Common::Serializer &serializer, Common::Array<T> &array, void (*serializeFunction)(Common::Serializer &, T &)) {
	auto size = array.size();
	serializer.syncAsUint32LE(size);
	array.resize(size);
	serializer.syncArray(array.data(), size, serializeFunction);
}

template<typename T>
inline void syncStack(Common::Serializer &serializer, Common::Stack<T> &stack, void (*serializeFunction)(Common::Serializer &, T &)) {
	auto size = stack.size();
	serializer.syncAsUint32LE(size);
	if (serializer.isLoading()) {
		for (uint i = 0; i < size; i++) {
			T value;
			serializeFunction(serializer, value);
			stack.push(value);
		}
	} else {
		for (uint i = 0; i < size; i++)
			serializeFunction(serializer, stack[i]);
	}
}

template<typename T>
inline void syncEnum(Common::Serializer &serializer, T &enumValue) {
	// syncAs does not have a cast for saving
	int32 intValue = static_cast<int32>(enumValue);
	serializer.syncAsSint32LE(intValue);
	enumValue = static_cast<T>(intValue);
}

}

#endif // ALCACHOFA_COMMON_H
